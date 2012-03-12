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

#include "gdivelog.h"
#include "db_generic_callbacks.h"
#include "defines.h"
#include "preferences.h"

extern sqlite3 *logbook_db;

gboolean renumber_db_renumber_dives(gint initial_dive_number)
{
  gint rc;
  glong min_dive_number;
  gchar *sqlErrMsg=NULL,*sqlcmd=NULL;
  gboolean rval=TRUE;

  rc=sqlite3_exec(logbook_db,"SELECT MIN(dive_number) FROM Dive",(gpointer)db_generic_callback_long,&min_dive_number,&sqlErrMsg);  
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in renumber_db_renumber_dives()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else {
    if(min_dive_number!=initial_dive_number) {
      sqlcmd=sqlite3_mprintf("UPDATE Dive SET dive_number=dive_number+%d",initial_dive_number-min_dive_number);
      sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
      if(rc!=SQLITE_OK) {
        g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in renumber_db_renumber_dives()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
        sqlite3_free(sqlErrMsg);
        rval=FALSE;
      }
      else db_not_saved();
    }
  }
  return rval;
}
