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

static gint renumber_callback(GList **list,gint argc,gchar **argv, gchar **azColName)
{
  (*list) = g_list_prepend(*list,GINT_TO_POINTER(atoi(argv[0])));
  return 0;
}

gboolean renumber_db_dive(gint dive, gint number)
{
  gint rc;
  gboolean rval=TRUE;
  gchar *sqlErrMsg=NULL,*sqlcmd=sqlite3_mprintf("UPDATE Dive SET dive_number=%d WHERE dive_id=%d",number,dive);

  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in renumber_db_renumber_dives()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else db_not_saved();
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean renumber_db_renumber_dives(gint initial_dive_number)
{
  gint rc;
  glong min_dive_number;
  gchar *sqlErrMsg=NULL,*sqlcmd=NULL;
  gboolean rval=TRUE;
  GList *list = g_list_alloc();

  rc=sqlite3_exec(logbook_db,"SELECT dive_id FROM Dive ORDER BY dive_datetime DESC;",(gpointer)renumber_callback,&list,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in renumber_db_renumber_dives()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else {
    GList *it=g_list_first(list);
    while(it){
      if(!renumber_db_dive(GPOINTER_TO_INT(it->data),initial_dive_number++)) rval=FALSE;
      it=g_list_next(it);
    }
  }
  return rval;
}
