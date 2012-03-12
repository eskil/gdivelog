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

extern sqlite3 *logbook_db;

gboolean dive_tank_db_dive_tanklist_load_store(gint dive_id,gpointer load_list_data,gpointer dive_tanklist_callback)
{
  gint rc;
  gchar *sqlErrMsg=NULL,*sqlcmd=NULL;
  gboolean rval=TRUE;

  sqlcmd=sqlite3_mprintf(
    "SELECT tank_name,dive_tank_O2,dive_tank_He,dive_tank_spressure,dive_tank_epressure,dive_tank_stime,dive_tank_etime,dive_tank_avg_depth,dive_tank_id,Dive_Tank.tank_id,dive_id,tank_volume,tank_wp FROM Dive_Tank LEFT JOIN Tank on Tank.tank_id=Dive_Tank.tank_id WHERE dive_id=%d",
    dive_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,(gpointer)dive_tanklist_callback,load_list_data,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,"Error in dive_tank_db_tanklist_load_store()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  sqlite3_free(sqlcmd);
  return rval;
}

gint dive_tank_db_insert(gint dive_id, gint tank_id,gdouble dive_tank_avg_depth,gdouble dive_tank_O2,gdouble dive_tank_He,
  glong dive_tank_stime,glong dive_tank_etime, gdouble dive_tank_spressure,gdouble dive_tank_epressure)
{
  gint rc;
  gchar *sqlcmd, *sqlErrMsg = NULL;
  gint new_dive_tank_id = 0;

  sqlcmd=sqlite3_mprintf(
    "INSERT INTO Dive_Tank (dive_id,tank_id,dive_tank_avg_depth,dive_tank_O2,dive_tank_He,dive_tank_stime,dive_tank_etime,dive_tank_spressure,dive_tank_epressure) VALUES (%d,%d,%f,%f,%f,%d,%d,%f,%f)",
    dive_id, tank_id,dive_tank_avg_depth,dive_tank_O2,dive_tank_He,dive_tank_stime,dive_tank_etime,dive_tank_spressure,dive_tank_epressure
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in dive_tank_db_insert()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    new_dive_tank_id=-1;
  }
  else {
    new_dive_tank_id=sqlite3_last_insert_rowid(logbook_db);
    db_not_saved();
  }
  sqlite3_free(sqlcmd);
  return new_dive_tank_id;
}

gboolean dive_tank_db_update(gint dive_tank_id,gint tank_id,gdouble dive_tank_avg_depth,gdouble dive_tank_O2,gdouble dive_tank_He,
  glong dive_tank_stime,glong dive_tank_etime,gdouble dive_tank_spressure,gdouble dive_tank_epressure)
{
  gint rc;
  gchar *sqlcmd, *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  sqlcmd=sqlite3_mprintf(
    "UPDATE Dive_Tank SET tank_id=%d,dive_tank_avg_depth=%f,dive_tank_O2=%f,dive_tank_He=%f,dive_tank_stime=%d,dive_tank_etime=%d,dive_tank_spressure=%f,dive_tank_epressure=%f WHERE dive_tank_id=%d",
    tank_id,dive_tank_avg_depth,dive_tank_O2,dive_tank_He,dive_tank_stime,dive_tank_etime,dive_tank_spressure,dive_tank_epressure,dive_tank_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in dive_tank_db_update()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved();
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean dive_tank_db_delete(gint dive_tank_id)
{
  gint rc;
  gchar *sqlcmd=NULL,*sqlErrMsg=NULL;
  gboolean rval=TRUE;

  sqlcmd=sqlite3_mprintf(
    "DELETE FROM Dive_Tank WHERE dive_tank_id=%d",
    dive_tank_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc != SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in dive_tank_db_delete()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved();
  sqlite3_free(sqlcmd);
  return rval;
}
