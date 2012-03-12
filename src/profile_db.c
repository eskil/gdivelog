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

gboolean profile_db_load(gint dive_id,gpointer profile_load_callback)
{
  gint rc;
  gchar *sqlErrMsg=NULL,*sqlcmd=NULL;
  gboolean rval=TRUE;

  sqlcmd=sqlite3_mprintf(
    "SELECT profile_time,profile_depth,profile_temperature FROM Profile WHERE dive_id=%d ORDER BY profile_time",
    dive_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,profile_load_callback,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in profile_db_load()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean profile_db_insert_segment(gint dive_id,glong profile_time,gdouble profile_depth,gdouble profile_temperature)
{
  gint rc;
  gchar *sqlcmd, *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  sqlcmd=sqlite3_mprintf(
    "INSERT INTO Profile (dive_id,profile_time,profile_depth,profile_temperature) VALUES (%d,%d,%f,%f)",
    dive_id,profile_time,profile_depth,profile_temperature
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in profile_db_insert_segment()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved();
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean profile_db_delete(gint dive_id)
{
  gint rc;
  gchar *sqlcmd,*sqlErrMsg=NULL;
  gboolean rval=TRUE;

  sqlcmd=sqlite3_mprintf(
    "DELETE FROM Profile WHERE dive_id=%d",
    dive_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in profile_db_delete()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd, sqlErrMsg);
    sqlite3_free (sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved();
  sqlite3_free(sqlcmd);
  return rval;
}
/*
gboolean profile_db_exists(glong dive_number)
{
  gint rc;
  gchar *sqlErrMsg=NULL,*sqlcmd=NULL;
  gboolean rval=FALSE;

  sqlcmd=sqlite3_mprintf(
    "SELECT profile.dive_id FROM Dive INNER JOIN Profile ON Profile.dive_id=Dive.dive_id WHERE dive_number=%ld LIMIT 1",
    dive_number
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,(gpointer)db_generic_callback_boolean,&rval,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in profile_db_load()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  sqlite3_free(sqlcmd);
  return rval;
}
*/
