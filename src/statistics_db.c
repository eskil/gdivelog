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
#include <gtk/gtk.h>

#include "db_main.h"
#include "db_generic_callbacks.h"
#include "defines.h"

extern sqlite3 *logbook_db;

gboolean statistics_db_load_sites_count(gpointer *statistics_load_sites_data,gpointer statistics_load_sites_count_callback)
{
  gint rc;
  gchar *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  rc=sqlite3_exec(logbook_db,"SELECT Site.site_id,site_parent_id,(SELECT COUNT(*) FROM Dive WHERE Dive.site_id=Site.site_id) FROM Site JOIN tSite_Full_Names ON tSite_Full_Names.site_id=Site.site_id ORDER BY site_full_name DESC",(gpointer)statistics_load_sites_count_callback,(gpointer)statistics_load_sites_data,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in statistics_db_load_sites_count()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  return rval;
}

gboolean statistics_db_load_sites(gpointer *statistics_load_sites_data,gpointer statistics_load_sites_callback)
{
  gint rc;
  gchar *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  rc=sqlite3_exec(logbook_db,"SELECT Site.site_id,site_parent_id,site_name FROM Site JOIN tSite_Full_Names ON tSite_Full_Names.site_id=Site.site_id ORDER BY site_full_name",(gpointer)statistics_load_sites_callback,(gpointer)statistics_load_sites_data,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in statistics_db_load_sites()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  return rval;
}

gboolean statistics_db_load_tanks(GtkListStore *list_store,gpointer statistics_load_list_callback)
{
  gint rc;
  gchar *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  rc=sqlite3_exec(logbook_db,"SELECT tank_name,(SELECT COUNT(*) FROM Dive_Tank WHERE tank_id=Tank.tank_id) FROM Tank ORDER BY tank_name",(gpointer)statistics_load_list_callback,(gpointer)list_store,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in statistics_db_load_types()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  return rval;
}

gboolean statistics_db_load_types(GtkListStore *list_store,gpointer statistics_load_list_callback)
{
  gint rc;
  gchar *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  rc=sqlite3_exec(logbook_db,"SELECT type_name,(SELECT COUNT(*) FROM Dive_Type WHERE type_id=Type.type_id) FROM Type ORDER BY type_name",(gpointer)statistics_load_list_callback,(gpointer)list_store,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in statistics_db_load_types()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  return rval;
}

gboolean statistics_db_load_equipment(GtkListStore *list_store,gpointer statistics_load_list_callback)
{
  gint rc;
  gchar *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  rc=sqlite3_exec(logbook_db,"SELECT equipment_name,(SELECT COUNT(*) FROM Dive_Equipment WHERE equipment_id=Equipment.equipment_id) FROM Equipment ORDER BY equipment_name",(gpointer)statistics_load_list_callback,(gpointer)list_store,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in statistics_db_load_equipment()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  return rval;
}

gboolean statistics_db_load_buddies(GtkListStore *list_store,gpointer statistics_load_list_callback)
{
  gint rc;
  gchar *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  rc=sqlite3_exec(logbook_db,"SELECT buddy_name,(SELECT COUNT(*) FROM Dive_Buddy WHERE buddy_id=Buddy.buddy_id) AS dive_count FROM Buddy ORDER BY dive_count DESC",(gpointer)statistics_load_list_callback,(gpointer)list_store,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in statistics_db_load_buddies()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  return rval;
}

gboolean statistics_db_load_totals(GtkListStore *list_store,gpointer statistics_load_totals_callback)
{
  gint rc;
  gchar *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  rc=sqlite3_exec(logbook_db,"SELECT MAX(dive_number),MAX(dive_maxdepth),MAX(dive_duration),SUM(dive_duration),MAX(dive_maxtemp),MIN(dive_mintemp) FROM Dive",(gpointer)statistics_load_totals_callback,(gpointer)list_store,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in statistics_db_load_totals()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  return rval;
}

gboolean statistics_db_load_depth_range(LoadDepthRangeData *load_depth_range_data,gpointer statistics_load_depth_range)
{
  gint rc;
  gchar *sqlErrMsg = NULL,*sqlcmd;
  gboolean rval = TRUE;

  sqlcmd=sqlite3_mprintf("SELECT COUNT(*) FROM Dive WHERE dive_maxdepth > %f and dive_maxdepth <= %f",load_depth_range_data->lwr,load_depth_range_data->upr);
  rc=sqlite3_exec(logbook_db,sqlcmd,(gpointer)statistics_load_depth_range,(gpointer)load_depth_range_data,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in statistics_db_load_depth_range()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  sqlite3_free(sqlcmd);
  return rval;
}
