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

gboolean equipment_db_equipmentlist_load_store(gint dive_id,gpointer load_list_data,gpointer equipmentlist_callback)
{
  gint rc;
  gchar *sqlErrMsg=NULL,*sqlcmd;
  gboolean rval=TRUE;

  sqlcmd=sqlite3_mprintf(
    "SELECT equipment_name,T1.equipment_id,dive_id FROM Equipment T1 LEFT JOIN (SELECT * FROM Dive_Equipment WHERE dive_id=%d) T2 on T2.equipment_id=T1.equipment_id ORDER BY equipment_name",
    dive_id
  );
  rc=sqlite3_exec (logbook_db,sqlcmd,(gpointer)equipmentlist_callback,load_list_data,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in equipment_db_equipmentlist_load_store()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval = FALSE;
  }
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean equipment_db_add_equipment_to_dive (gint dive_id, gint equipment_id)
{
  gint rc;
  gchar *sqlErrMsg=NULL,*sqlcmd=NULL;
  gboolean rval=TRUE;

  sqlcmd=sqlite3_mprintf(
    "INSERT INTO Dive_Equipment (dive_id,equipment_id) VALUES (%d,%d)",
    dive_id, equipment_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in equipment_db_add_equipment_to_dive()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved ();
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean equipment_db_remove_equipment_from_dive(gint dive_id,gint equipment_id)
{
  gint rc;
  gchar *sqlErrMsg = NULL, *sqlcmd = NULL;
  gboolean rval = TRUE;

  sqlcmd=sqlite3_mprintf(
    "DELETE FROM Dive_Equipment WHERE dive_id=%d AND equipment_id=%d",
    dive_id,equipment_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK){
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in equipment_db_add_equipment_to_dive()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved ();
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean equipment_db_load_equipment_detail(gint equipment_id,gpointer equipmentdetail_callback)
{
  gint rc;
  gchar *sqlcmd,*sqlErrMsg=NULL;
  gboolean rval=TRUE;

  sqlcmd=sqlite3_mprintf(
    "SELECT equipment_notes,dive_id FROM Equipment LEFT JOIN Dive_Equipment ON Dive_Equipment.equipment_id=Equipment.equipment_id WHERE Equipment.equipment_id=%d",
    equipment_id
  );
  rc=sqlite3_exec (logbook_db,sqlcmd,(gpointer)equipmentdetail_callback,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log (G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in equipment_db_load_equipment_detail()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  sqlite3_free(sqlcmd);
  return rval;
}

gint
equipment_db_insert (gchar * equipment_name, gchar * equipment_notes)
{
  gint rc;
  gchar *sqlcmd,*sqlErrMsg=NULL;
  gint new_equipment_id=0;

  sqlcmd=sqlite3_mprintf(
    "INSERT INTO Equipment (equipment_name,equipment_notes) VALUES ('%q','%q')",
    equipment_name,equipment_notes
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in equipment_db_insert()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    new_equipment_id = -1;
  }
  else {
    new_equipment_id=sqlite3_last_insert_rowid(logbook_db);
    db_not_saved();
  }
  sqlite3_free(sqlcmd);
  return new_equipment_id;
}

gboolean
equipment_db_update (gint equipment_id, gchar * equipment_name,
		     gchar * equipment_notes)
{
  gint rc;
  gchar *sqlcmd, *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  sqlcmd=sqlite3_mprintf(
    "UPDATE Equipment SET equipment_name='%q',equipment_notes='%q' WHERE equipment_id=%d",
    equipment_name,equipment_notes,equipment_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in equipment_db_update()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved ();
  sqlite3_free (sqlcmd);
  return rval;
}

gboolean
equipment_db_delete (gint equipment_id)
{
  gint rc;
  gchar *sqlcmd=NULL,*sqlErrMsg=NULL;
  gboolean rval=TRUE;

  sqlcmd=sqlite3_mprintf(
    "DELETE FROM Equipment WHERE equipment_id=%d",
		equipment_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in equipment_db_delete()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved ();
  sqlite3_free (sqlcmd);
  return rval;
}
