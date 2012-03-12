/*
    Copyright 2005 Simon Naunton

    This file is part of gdivelog.

    gdivelog is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    gdivelog is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with gdivelog; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <glib.h>
#include <sqlite3.h>
#include <time.h>

#include "db_main.h"
#include "db_generic_callbacks.h"

extern sqlite3 *logbook_db;

static gboolean site_db_site_name_unique(gint site_id,gint site_parent_id,gchar * site_name)
{
  gint rc;
  gchar *sqlcmd,*sqlErrMsg=NULL;
  gboolean rval=FALSE;

  if(site_id) {
    sqlcmd=sqlite3_mprintf(
      "SELECT site_id FROM Site WHERE site_id!=%d AND site_parent_id=%d AND site_name like '%q'",
      site_id,site_parent_id,site_name
    );
  }
  else {
    sqlcmd=sqlite3_mprintf(
      "SELECT site_id FROM Site WHERE site_parent_id=%d AND site_name like '%q'",
      site_parent_id,site_name
    );
  }
  rc=sqlite3_exec(logbook_db,sqlcmd,(gpointer)db_generic_callback_boolean,&rval,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in site_db_site_name_unique_for parent()\nCode=%d\nQuery=%s\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  sqlite3_free(sqlcmd);
  return !rval;
}

gboolean site_db_sitetree_load_store(gpointer load_sites_callback,gpointer load_sites_data)
{
  gint rc;
  gchar *sqlErrMsg=NULL;
  gboolean rval=TRUE;

  rc=sqlite3_exec(logbook_db,
    "SELECT Site.site_id,site_parent_id,site_name,site_full_name FROM Site JOIN tSite_Full_Names ON tSite_Full_Names.site_id=Site.site_id ORDER BY site_full_name",
    (gpointer)load_sites_callback,load_sites_data,&sqlErrMsg
  );
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in site_db_sitetree_load_store()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  return rval;
}

gboolean site_db_load_detail(gint site_id,gpointer load_detail_callback)
{
  gint rc;
  gchar *sqlcmd,*sqlErrMsg=NULL;
  gboolean rval=TRUE;

  sqlcmd=sqlite3_mprintf("SELECT site_name,site_notes FROM Site WHERE site_id=%d",site_id);
  rc=sqlite3_exec(logbook_db,sqlcmd,(gpointer)load_detail_callback,&site_id,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in site_db_load_detail()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean site_db_update(gint site_id,gint site_parent_id,gchar *site_name,gchar *site_notes)
{
  gint rc;
  gchar *sqlcmd,*sqlErrMsg=NULL,*site_idstr,*site_full_name;
  gboolean rval=TRUE;

  if(!site_db_site_name_unique(site_id,site_parent_id,site_name)) return FALSE;

  sqlcmd=sqlite3_mprintf(
    "UPDATE Site SET site_parent_id=%d,site_name='%q',site_notes='%q' WHERE site_id=%d",
     site_parent_id,site_name,site_notes,site_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in site_db_update()\nCode=%d\nQuery=%s\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  if(rval) {
    db_not_saved();
    site_idstr=g_strdup_printf("%d",site_id);
    site_full_name=db_get_full_site_name(site_idstr);
    sqlite3_free(sqlcmd);
    sqlcmd=sqlite3_mprintf("UPDATE tSite_Full_Names SET site_full_name='%q' WHERE site_id=%d",site_full_name,site_id);
    g_free(site_idstr);
    g_free(site_full_name);
    rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
    if(rc!=SQLITE_OK) {
      g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in site_db_update()\nCode=%d\nQuery=%s\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
      sqlite3_free(sqlErrMsg);
      rval=FALSE;
    }
  }
  sqlite3_free(sqlcmd);
  return rval;
}

gint site_db_new(gint site_parent_id,gchar *site_name, gchar *site_notes)
{
  gint rc;
  gchar *sqlcmd,*sqlErrMsg=NULL,*site_idstr,*site_full_name;
  gint site_id=0;

  if(!site_db_site_name_unique(0,site_parent_id,site_name)) return 0;
  sqlcmd=sqlite3_mprintf(
    "INSERT INTO Site (site_parent_id,site_name,site_notes) VALUES (%d,'%q','%q')",
    site_parent_id, site_name, site_notes
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in site_db_new()\nCode=%d\nQuery=%s\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    site_id=-1;
  }
  else {
    site_id=sqlite3_last_insert_rowid(logbook_db);
    db_not_saved();
    /* Insert full_name into cache */
    site_idstr=g_strdup_printf("%d",site_id);
    site_full_name=db_get_full_site_name(site_idstr);
    sqlite3_free(sqlcmd);
    sqlcmd=sqlite3_mprintf(
      "INSERT INTO tSite_Full_Names (site_id,site_full_name) VALUES (%d,'%q')",
	    site_id, site_full_name
    );
    g_free(site_idstr);
    g_free(site_full_name);
    rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
    if(rc!=SQLITE_OK) {
	    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in site_db_new()\nCode=%d\nQuery=%s\nError Message='%s'\n",rc,sqlcmd, sqlErrMsg);
	    sqlite3_free (sqlErrMsg);
	  }
      /* db_cache_sites(); */
  }
  sqlite3_free(sqlcmd);
  return site_id;
}

gboolean site_db_delete_allowed(gint site_id)
{
  gint rc;
  gchar *sqlcmd,*sqlErrMsg=NULL;
  gint delete_site_id=0;

  /* Check for children - delete not allowed if there are children */
  sqlcmd=sqlite3_mprintf(
    "SELECT site_id FROM Site WHERE site_parent_id=%d LIMIT 1",
    site_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,(gpointer)db_generic_callback_int,&delete_site_id,&sqlErrMsg);
  if(rc==SQLITE_OK&&!delete_site_id) {
    sqlite3_free(sqlcmd);
    /* Check if site has been dived - delete not allowed if true */
    sqlcmd=sqlite3_mprintf(
      "SELECT site_id FROM Dive WHERE site_id=%d LIMIT 1",
      site_id
    );
    rc=sqlite3_exec(logbook_db,sqlcmd,(gpointer)db_generic_callback_int,&delete_site_id,&sqlErrMsg);
  }
  if(rc != SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in site_db_delete_allowed()\nCode=%d\nQuery=%s\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    delete_site_id=0;
  }
  sqlite3_free(sqlcmd);
  return delete_site_id?FALSE:TRUE;
}

gboolean site_db_delete(gint site_id)
{
  gint rc;
  gchar *sqlcmd,*sqlErrMsg=NULL;
  gboolean rval=TRUE;

  sqlcmd=sqlite3_mprintf(
    "DELETE FROM Site WHERE site_id=%d;DELETE FROM tSite_Full_Names WHERE site_id=%d",
    site_id,site_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in site_db_delete()\nCode=%d\nQuery=%s\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved();
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean site_db_get_site_full_name(gint site_id,gpointer get_site_full_name_callback)
{
  gint rc;
  gchar *sqlcmd,*sqlErrMsg=NULL;
  gboolean rval=TRUE;

  sqlcmd=sqlite3_mprintf(
    "SELECT site_full_name FROM tSite_Full_Names WHERE site_id=%d",
     site_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,(gpointer)get_site_full_name_callback,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in site_db_get_site_full_name()\nCode=%d\nQuery=%s\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free (sqlErrMsg);
    rval=FALSE;
  }
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean site_db_set_parent(gint site_id,gint site_parent_id)
{
  gint rc;
  gchar *sqlcmd,*sqlErrMsg=NULL;
  gboolean rval=TRUE;

  sqlcmd=sqlite3_mprintf(
    "UPDATE Site SET site_parent_id=%d WHERE site_id=%d",
    site_parent_id,site_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in site_db_set_parent()\nCode=%d\nQuery=%s\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else {
    db_not_saved();
    db_cache_sites();
  }
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean site_db_isancestor(gint site_id,gint ancestor_id)
{
  gint rc;
  gchar *sqlcmd=NULL,*sqlErrMsg=NULL;
  gboolean rval=FALSE;
  gint current_site_id=site_id;

  while(current_site_id) {
    sqlcmd=sqlite3_mprintf(
      "SELECT site_parent_id FROM Site WHERE site_id=%d",
      current_site_id
    );
    rc=sqlite3_exec(logbook_db,sqlcmd,(gpointer)db_generic_callback_int,&current_site_id,&sqlErrMsg);
    if(rc!=SQLITE_OK) {
      g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in site_db_set_parent()\nCode=%d\nQuery=%s\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
      sqlite3_free(sqlErrMsg);
      sqlite3_free(sqlcmd);
      break;
    }
    if(current_site_id==ancestor_id) {
      rval=TRUE;
      sqlite3_free(sqlcmd);
      break;
    }
    sqlite3_free(sqlcmd);
  }
  return rval;
}
