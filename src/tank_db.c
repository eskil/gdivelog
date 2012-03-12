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

gboolean tank_db_tanklist_load_store(gpointer load_list_data,gpointer tanklist_callback)
{
  gint rc;
  gchar *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  rc=sqlite3_exec(logbook_db,"SELECT tank_name,tank_volume,tank_wp,tank_id FROM Tank ORDER BY tank_name",(gpointer)tanklist_callback,load_list_data,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in tank_db_tanklist_load_store()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  return rval;
}

gboolean tank_db_load_tank_detail(gint tank_id,gpointer tankdetail_callback)
{
  gint rc;
  gchar *sqlcmd, *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  sqlcmd=sqlite3_mprintf (
    "SELECT tank_notes,dive_id FROM Tank LEFT JOIN Dive_Tank ON Dive_Tank.tank_id=Tank.tank_id WHERE Tank.tank_id=%d",
		tank_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,(gpointer)tankdetail_callback,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in tank_db_load_tank_detail()\nCode=%d\nQuery='%s'\nError Message='%s'\n", rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  sqlite3_free(sqlcmd);
  return rval;
}

gint tank_db_insert(gchar *tank_name,gdouble tank_volume,gdouble tank_wp,gchar *tank_notes)
{
  gint rc;
  gchar *sqlcmd,*sqlErrMsg=NULL;
  gint new_tank_id=0;

  sqlcmd=sqlite3_mprintf(
    "INSERT INTO Tank (tank_name,tank_volume,tank_wp,tank_notes) VALUES ('%q',%f,%f,'%q')",
    tank_name, tank_volume, tank_wp, tank_notes
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in tank_db_insert()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free (sqlErrMsg);
    new_tank_id = -1;
  }
  else {
    new_tank_id=sqlite3_last_insert_rowid(logbook_db);
    db_not_saved();
  }
  sqlite3_free(sqlcmd);
  return new_tank_id;
}

gboolean tank_db_update(gint tank_id, gchar *tank_name,gdouble tank_volume,gdouble tank_wp,gchar *tank_notes)
{
  gint rc;
  gchar *sqlcmd, *sqlErrMsg=NULL;
  gboolean rval=TRUE;

  sqlcmd=sqlite3_mprintf(
    "UPDATE Tank SET tank_name='%q',tank_volume=%f,tank_wp=%f,tank_notes='%q' WHERE tank_id=%d",
    tank_name,tank_volume,tank_wp,tank_notes,tank_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in tank_db_update()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved();
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean tank_db_delete(gint tank_id)
{
  gint rc;
  gchar *sqlcmd = NULL, *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  sqlcmd=sqlite3_mprintf(
    "DELETE FROM Tank WHERE tank_id=%d",
    tank_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in tank_db_delete()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved ();
  sqlite3_free (sqlcmd);
  return rval;
}
