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

gboolean buddy_db_buddylist_load_store(gint dive_id,gpointer load_list_data,gpointer buddylist_callback)
{
  gint rc;
  gchar *sqlErrMsg = NULL, *sqlcmd;
  gboolean rval = TRUE;

  sqlcmd=sqlite3_mprintf(
    "SELECT buddy_name,T1.buddy_id,dive_id FROM Buddy T1 LEFT JOIN (SELECT * FROM Dive_Buddy WHERE dive_id=%d) T2 ON T2.buddy_id=T1.buddy_id ORDER BY buddy_name",
    dive_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,(gpointer)buddylist_callback,load_list_data,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in buddy_db_buddylist_load_store()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean buddy_db_add_buddy_to_dive(gint dive_id,gint buddy_id)
{
  gint rc;
  gchar *sqlErrMsg=NULL,*sqlcmd=NULL;
  gboolean rval=TRUE;

  sqlcmd=sqlite3_mprintf(
    "INSERT INTO Dive_Buddy (dive_id,buddy_id) VALUES (%d,%d)", dive_id,
    buddy_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in buddy_db_add_buddy_to_dive()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved();
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean buddy_db_remove_buddy_from_dive(gint dive_id,gint buddy_id)
{
  gint rc;
  gchar *sqlErrMsg=NULL, *sqlcmd=NULL;
  gboolean rval=TRUE;

  sqlcmd=sqlite3_mprintf(
    "DELETE FROM Dive_Buddy WHERE dive_id=%d AND buddy_id=%d",
    dive_id,buddy_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in buddy_db_add_buddy_to_dive()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved();
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean buddy_db_load_buddy_detail(gint buddy_id,gpointer buddydetail_callback)
{
  gint rc;
  gchar *sqlcmd,*sqlErrMsg=NULL;
  gboolean rval=TRUE;

  sqlcmd=sqlite3_mprintf("SELECT buddy_notes,dive_id FROM Buddy LEFT JOIN Dive_Buddy ON Dive_Buddy.buddy_id=Buddy.buddy_id WHERE Buddy.buddy_id=%d LIMIT 1",buddy_id);
  rc=sqlite3_exec (logbook_db,sqlcmd,(gpointer)buddydetail_callback,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in buddy_db_load_buddy_detail()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
      sqlite3_free (sqlErrMsg);
      rval = FALSE;
  }
  sqlite3_free (sqlcmd);
  return rval;
}

gint buddy_db_insert(gchar *buddy_name,gchar *buddy_notes)
{
  gint rc;
  gchar *sqlcmd,*sqlErrMsg=NULL;
  gint new_buddy_id=0;

  sqlcmd=sqlite3_mprintf(
    "INSERT INTO Buddy (buddy_name,buddy_notes) VALUES ('%q','%q')",
    buddy_name, buddy_notes
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in buddy_db_insert()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd, sqlErrMsg);
    sqlite3_free (sqlErrMsg);
    new_buddy_id = -1;
  }
  else {
    new_buddy_id=sqlite3_last_insert_rowid (logbook_db);
    db_not_saved();
  }
  sqlite3_free(sqlcmd);
  return new_buddy_id;
}

gboolean buddy_db_update(gint buddy_id,gchar *buddy_name,gchar* buddy_notes)
{
  gint rc;
  gchar *sqlcmd, *sqlErrMsg=NULL;
  gboolean rval = TRUE;

  sqlcmd=sqlite3_mprintf(
    "UPDATE Buddy SET buddy_name='%q',buddy_notes='%q' WHERE buddy_id=%d",
    buddy_name,buddy_notes,buddy_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in buddy_db_update()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved();
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean buddy_db_delete(gint buddy_id)
{
  gint rc;
  gchar *sqlcmd = NULL, *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  sqlcmd=sqlite3_mprintf(
    "DELETE FROM Buddy WHERE buddy_id=%d",
    buddy_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in buddy_db_delete()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved();
  sqlite3_free(sqlcmd);
  return rval;
}
