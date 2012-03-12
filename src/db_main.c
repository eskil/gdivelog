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
#include <glib/gi18n.h>
#include <sqlite3.h>

#include "gdivelog.h"
#include "defines.h"
#include "db_generic_callbacks.h"
#include "preferences.h"

typedef struct
{
  gint parent_id;
  GString *site_name;
} SiteData;

extern Preferences preferences;
sqlite3 *logbook_db = NULL;
gboolean handle_transactions=TRUE;
static gboolean db_saved_value = TRUE;

static gint db_get_full_site_name_callback(SiteData *site_data,gint argc, gchar **argv,gchar **azColName);
static gint db_cache_sites_callback(gchar **site_id,gint argc,gchar **argv, gchar **azColName);

void db_not_saved(void)
{
  db_saved_value=FALSE;
}

void db_saved(void)
{
  db_saved_value=TRUE;
}

gboolean db_is_saved(void)
{
  return db_saved_value;
}

gboolean db_create_schema(sqlite3 *db)
{
  gint rc;
  gchar *sqlErrMsg = NULL;
  gchar sqlcmd[] =
    "PRAGMA auto_vacuum=1;"
    "CREATE TABLE Dive (dive_id INTEGER NOT NULL UNIQUE PRIMARY KEY AUTOINCREMENT,dive_number INTEGER,dive_datetime TEXT,dive_duration INTEGER,dive_maxdepth REAL,dive_mintemp REAL,dive_maxtemp REAL,dive_notes TEXT,site_id INTEGER,dive_visibility REAL,dive_weight REAL);"
    "CREATE TABLE Site (site_id INTEGER NOT NULL UNIQUE PRIMARY KEY AUTOINCREMENT,site_parent_id INTEGER,site_name TEXT,site_notes TEXT);"
    "CREATE TABLE Buddy (buddy_id INTEGER NOT NULL UNIQUE PRIMARY KEY AUTOINCREMENT,buddy_name TEXT,buddy_notes TEXT);"
    "CREATE TABLE Dive_Buddy (dive_id INTEGER,buddy_id INTEGER);"
    "CREATE TABLE Equipment (equipment_id INTEGER NOT NULL UNIQUE PRIMARY KEY AUTOINCREMENT,equipment_name TEXT,equipment_notes TEXT);"
    "CREATE TABLE Dive_Equipment (dive_id INTEGER,equipment_id INTEGER);"
    "CREATE TABLE Type (type_id INTEGER NOT NULL UNIQUE PRIMARY KEY AUTOINCREMENT,type_name TEXT INTEGER,type_notes TEXT);"
    "CREATE TABLE Dive_Type (dive_id INTEGER,type_id INTEGER);"
    "CREATE TABLE Tank (tank_id INTEGER NOT NULL UNIQUE PRIMARY KEY AUTOINCREMENT,tank_name TEXT,tank_volume REAL,tank_wp REAL,tank_notes TEXT);"
    "CREATE TABLE Dive_Tank (dive_tank_id INTEGER NOT NULL UNIQUE PRIMARY KEY AUTOINCREMENT,dive_id INTEGER,tank_id INTEGER,dive_tank_avg_depth REAL,dive_tank_O2 REAL,dive_tank_He REAL,dive_tank_stime INTEGER,dive_tank_etime INTEGER,dive_tank_spressure REAL,dive_tank_epressure REAL);"
    "CREATE TABLE Profile (dive_id INTEGER, profile_time INTEGER, profile_depth REAL, profile_temperature REAL);"
    "CREATE TABLE Preferences (preferences_template_dive_number INTEGER,gdivelog_schema_version TEXT);"
    "CREATE TRIGGER Dive_Insert AFTER INSERT ON Dive BEGIN UPDATE Dive SET dive_number=dive_number+1 WHERE dive_datetime > NEW.dive_datetime AND (SELECT CASE WHEN (SELECT dive_number FROM Dive WHERE dive_datetime < NEW.dive_datetime ORDER BY dive_datetime DESC LIMIT 1) IS NULL THEN (SELECT CASE WHEN (SELECT MIN(dive_number) FROM Dive WHERE dive_number > 0) > 1 THEN dive_number < 0 ELSE dive_number > 0 END) ELSE dive_number > 0 END);UPDATE Dive SET dive_number=(SELECT CASE WHEN (SELECT dive_number FROM Dive WHERE dive_datetime < NEW.dive_datetime ORDER BY dive_datetime DESC LIMIT 1) IS NULL THEN (SELECT CASE WHEN (SELECT MIN(dive_number) FROM Dive WHERE dive_number > 0) > 1 THEN (SELECT MIN(dive_number)-1 FROM Dive WHERE dive_number > 1) ELSE 1 END) ELSE (SELECT dive_number+1 FROM Dive WHERE dive_datetime <= NEW.dive_datetime AND dive_number > 0 ORDER BY dive_datetime DESC LIMIT 1) END) WHERE dive_id=NEW.dive_id;END;"
    "CREATE TRIGGER Dive_Update AFTER UPDATE OF dive_datetime ON Dive BEGIN UPDATE Dive SET dive_number=(SELECT CASE WHEN dive_datetime > OLD.dive_datetime AND dive_datetime < NEW.dive_datetime THEN dive_number-1 WHEN dive_datetime < OLD.dive_datetime AND dive_datetime > NEW.dive_datetime THEN dive_number+1 WHEN dive_datetime = NEW.dive_datetime THEN (SELECT CASE WHEN dive_datetime < OLD.dive_datetime THEN dive_number-(SELECT COUNT(*) FROM Dive WHERE dive_datetime < OLD.dive_datetime AND dive_datetime > NEW.dive_datetime) ELSE dive_number+(SELECT COUNT(*) FROM Dive WHERE dive_datetime > OLD.dive_datetime AND dive_datetime < NEW.dive_datetime) END) END) WHERE (SELECT CASE WHEN dive_datetime < OLD.dive_datetime THEN dive_datetime <= OLD.dive_datetime AND dive_datetime >= NEW.dive_datetime ELSE dive_datetime >= OLD.dive_datetime AND dive_datetime <= NEW.dive_datetime END); END;"
    "CREATE TRIGGER Dive_Delete AFTER DELETE ON Dive BEGIN UPDATE Dive SET dive_number=dive_number-1 WHERE dive_number > OLD.dive_number;DELETE FROM Dive_Buddy WHERE dive_id=OLD.dive_id;DELETE FROM Dive_Equipment WHERE dive_id=OLD.dive_id;DELETE FROM Dive_Tank WHERE dive_id=OLD.dive_id;DELETE FROM Dive_Type WHERE dive_id=OLD.dive_id;DELETE FROM Profile WHERE dive_id=OLD.dive_id;END;"
    "CREATE TRIGGER Dive_Update_Preferences AFTER UPDATE OF dive_number ON Dive BEGIN UPDATE Preferences SET preferences_template_dive_number=NEW.dive_number WHERE preferences_template_dive_number=OLD.dive_number; END;"
    "CREATE TRIGGER Dive_Delete_Preferences BEFORE DELETE ON Dive BEGIN UPDATE Preferences SET preferences_template_dive_number=-1 WHERE preferences_template_dive_number=OLD.dive_number; END;"
    "CREATE TRIGGER Buddy_Delete AFTER DELETE ON Buddy BEGIN DELETE FROM Dive_Buddy WHERE buddy_id=OLD.buddy_id; END;"
    "CREATE TRIGGER Equipment_Delete AFTER DELETE ON Equipment BEGIN DELETE FROM Dive_Equipment WHERE equipment_id=OLD.equipment_id; END;"
    "CREATE TRIGGER Type_Delete AFTER DELETE ON Type BEGIN DELETE FROM Dive_Type WHERE type_id=OLD.type_id; END;"
    "CREATE TRIGGER Tank_Delete AFTER DELETE ON Tank BEGIN DELETE FROM Dive_Tank WHERE tank_id=OLD.tank_id; END;"
    "INSERT INTO Preferences (preferences_template_dive_number,gdivelog_schema_version) VALUES (-1,'" GDIVELOG_SCHEMA_VERSION "');"
  ;
  rc=sqlite3_exec(db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_CRITICAL,"Error in db_create_schema()\nCode=%d\nQuery='%s'\nError message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    return FALSE;
  }
  return TRUE;
}

static gint db_get_full_site_name_callback(SiteData *site_data,gint argc,gchar **argv, gchar **azColName)
{
  site_data->parent_id=strtol(argv[0],NULL,0);
  if(site_data->site_name->len) g_string_prepend(site_data->site_name,preferences.site_name_seperator);
  g_string_prepend(site_data->site_name,argv[1]);
  return 0;
}

gchar *db_get_full_site_name(gchar *site_id)
{
  gint current_id;
  gint rc;
  gchar *sqlcmd, *sqlErrMsg = NULL, *site_name;
  SiteData site_data;
  
  current_id=strtol(site_id,NULL,0);
  site_data.site_name=g_string_new(NULL);
  do {
    sqlcmd=sqlite3_mprintf(
      "SELECT site_parent_id,site_name FROM Site WHERE site_id=%ld",
      current_id
    );
    rc=sqlite3_exec(logbook_db,sqlcmd,(gpointer)db_get_full_site_name_callback,&site_data,&sqlErrMsg);
    if(rc!=SQLITE_OK) {
      g_log(G_LOG_DOMAIN,G_LOG_LEVEL_CRITICAL,"Error in db_get_full_site_name()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
      sqlite3_free (sqlErrMsg);
      return NULL;
    }
    else current_id=site_data.parent_id;
    sqlite3_free(sqlcmd);
  } while(current_id);
  site_name=g_strdup(site_data.site_name->str);
  g_string_free(site_data.site_name,TRUE);
  return site_name;
}

static gint db_cache_sites_callback(gchar **site_id,gint argc, gchar **argv, gchar **azColName)
{
  gchar *site_full_name, *sqlcmd, *sqlErrMsg = NULL;
  gint rc;

  site_full_name=db_get_full_site_name(argv[0]);
  sqlcmd=sqlite3_mprintf(
    "INSERT INTO tSite_Full_Names (site_id,site_full_name) VALUES (%s,'%q')",
    argv[0],site_full_name
  );
  g_free(site_full_name);
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_CRITICAL,"Error in db_cache_sites_callback()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free (sqlErrMsg);
    rc=1;
  }
  else rc=0;
  sqlite3_free (sqlcmd);
  return rc;
}

gboolean db_cache_sites(void)
{
  /* Creates a temp table for cacheing the full names (i.e. paths) of sites */

  gint rc;
  gchar *sqlErrMsg = NULL;

  /* ignoring errors on the following one line of code is intentional because  */
  /* tSite_Full_Names will not exist when this function is first called             */
  sqlite3_exec(logbook_db, "DROP TABLE tSite_Full_Names",NULL,0,NULL);
  rc=sqlite3_exec(logbook_db,"CREATE TEMP TABLE tSite_Full_Names (site_id INTEGER, site_full_name TEXT);",NULL,0,&sqlErrMsg);
  if(rc==SQLITE_OK) rc=sqlite3_exec(logbook_db,"SELECT site_id FROM Site",(gpointer)db_cache_sites_callback,0,&sqlErrMsg);
  if(rc != SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in db_cache_sites()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    return FALSE;
  }
  return TRUE;
}

gboolean db_close(void)
{
  gint rc;

  rc=sqlite3_close(logbook_db);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_CRITICAL,"Error in db_close(). Code=%d\n",rc);
    return FALSE;
  }
  logbook_db=NULL;
  return TRUE;
}

gboolean db_open(gchar * fname)
{
  gint rc=0;
  gchar *sqlErrMsg=NULL;
  gboolean rval=TRUE;

  if(sqlite3_open(fname, &logbook_db)!=SQLITE_OK) {
    g_log (G_LOG_DOMAIN,G_LOG_LEVEL_CRITICAL,"Cannot open %s.\nError: %s\n",fname,sqlite3_errmsg (logbook_db));
    sqlite3_close(logbook_db);
    logbook_db=NULL;
    rval=FALSE;
  }
  else {
    db_cache_sites ();
    preferences_load_template_dive_number();
  }
  return rval;
}

gboolean db_new(gchar *fname)
{
  gint rc=0;
  gchar *sqlErrMsg=NULL;

  if(logbook_db) {
    if(!db_close())
    return FALSE;
  }
  if(sqlite3_open(fname,&logbook_db)!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_CRITICAL,"Cannot create %s.\nError: %s\n",fname,sqlite3_errmsg(logbook_db));
    sqlite3_close(logbook_db);
    logbook_db=NULL;
    return FALSE;
  }
  if(!db_create_schema(logbook_db)) return FALSE;
  return TRUE;
}

gint db_execute_query(gchar *statement,gpointer callback,gpointer user_data,gchar **error_message)
{
  return sqlite3_exec (logbook_db, statement, callback, user_data, error_message);
}

gchar *db_get_last_dive_datetime(void)
{
  gint rc;
  gchar *sqlErrMsg=NULL,*rval=NULL;

  rc=sqlite3_exec(logbook_db,"SELECT dive_datetime FROM Dive ORDER BY dive_datetime DESC LIMIT 1",(gpointer)db_generic_callback_string,&rval,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_CRITICAL,"Error in db_get_last_dive_datetime()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free (sqlErrMsg);
  }
  return rval;
}

gboolean db_begin_transaction(void)
{
  gint rc;
  gchar *sqlErrMsg = NULL;
  gboolean rval=TRUE;
  
  rc= sqlite3_exec(logbook_db,"BEGIN",NULL,0,&sqlErrMsg);
  if(rc != SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_CRITICAL,"Error in db_begin()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free (sqlErrMsg);
    rval=FALSE;
  }
  return rval;
}

gboolean db_commit_transaction(void)
{
  gint rc;
  gchar *sqlErrMsg = NULL;
  gboolean rval=TRUE;

  handle_transactions=TRUE;
  rc=sqlite3_exec(logbook_db,"COMMIT",NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_CRITICAL,"Error in db_commit()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  return rval;
}

gboolean db_rollback_transaction(void)
{
  gint rc;
  gchar *sqlErrMsg = NULL;
  gboolean rval=TRUE;

  handle_transactions=TRUE;
  rc=sqlite3_exec(logbook_db,"ROLLBACK",NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_CRITICAL,"Error in db_rollback()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  return rval;
}
