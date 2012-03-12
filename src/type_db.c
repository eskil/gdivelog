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

#include "db_generic_callbacks.h"

extern sqlite3 *logbook_db;

gboolean type_db_typelist_load_store(gint dive_id,gpointer load_list_data,gpointer typelist_callback)
{
  gint rc;
  gchar *sqlErrMsg = NULL, *sqlcmd;
  gboolean rval = TRUE;

  sqlcmd=sqlite3_mprintf(
    "SELECT type_name,T1.type_id,dive_id FROM Type T1 LEFT JOIN (SELECT * FROM Dive_Type WHERE dive_id=%d) T2 on T2.type_id=T1.type_id ORDER BY type_name",
    dive_id
  );
  rc=sqlite3_exec(logbook_db, sqlcmd,(gpointer)typelist_callback,load_list_data,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in type_db_typelist_load_store()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean type_db_add_type_to_dive(gint dive_id,gint type_id)
{
  gint rc;
  gchar *sqlErrMsg = NULL, *sqlcmd = NULL;
  gboolean rval = TRUE;

  sqlcmd=sqlite3_mprintf(
    "INSERT INTO Dive_Type (dive_id,type_id) VALUES (%d,%d)", dive_id,
    type_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in type_db_add_type_to_dive()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved();
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean type_db_remove_type_from_dive(gint dive_id,gint type_id)
{
  gint rc;
  gchar *sqlErrMsg = NULL, *sqlcmd = NULL;
  gboolean rval = TRUE;

  sqlcmd=sqlite3_mprintf(
    "DELETE FROM Dive_Type WHERE dive_id=%d AND type_id=%d",
    dive_id,type_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in type_db_add_type_to_dive()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved();
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean type_db_load_type_detail(gint type_id,gpointer typedetail_callback)
{
  gint rc;
  gchar *sqlcmd, *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  sqlcmd=sqlite3_mprintf(
    "SELECT type_notes,dive_id FROM Type LEFT JOIN Dive_Type ON Dive_Type.type_id=Type.type_id WHERE Type.type_id=%d",
		type_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,(gpointer)typedetail_callback,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in type_db_load_type_detail()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  sqlite3_free(sqlcmd);
  return rval;
}

gint type_db_insert(gchar *type_name, gchar *type_notes)
{
  gint rc;
  gchar *sqlcmd, *sqlErrMsg = NULL;
  gint new_type_id = 0;

  sqlcmd=sqlite3_mprintf(
    "INSERT INTO Type (type_name,type_notes) VALUES ('%q','%q')",
    type_name,type_notes
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in type_db_insert()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    new_type_id=-1;
  }
  else {
    new_type_id=sqlite3_last_insert_rowid(logbook_db);
    db_not_saved();
  }
  sqlite3_free(sqlcmd);
  return new_type_id;
}

gboolean type_db_update(gint type_id,gchar *type_name,gchar *type_notes)
{
  gint rc;
  gchar *sqlcmd, *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  sqlcmd=sqlite3_mprintf(
    "UPDATE Type SET type_name='%q',type_notes='%q' WHERE type_id=%d",
    type_name,type_notes,type_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in type_db_update()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved();
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean type_db_delete(gint type_id)
{
  gint rc;
  gchar *sqlcmd = NULL, *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  sqlcmd=sqlite3_mprintf(
    "DELETE FROM Type WHERE type_id=%d",
    type_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!= SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in type_db_delete()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved();
  sqlite3_free(sqlcmd);
  return rval;
}
