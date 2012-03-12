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

#include "gdivelog.h"
#include "db_generic_callbacks.h"
#include "defines.h"
#include "preferences.h"
#include "profile_gui.h"

static glong dive_db_template_dive_number;
extern sqlite3 *logbook_db;
extern handle_transactions;
extern Preferences preferences;

gboolean dive_db_divelist_load_store(gpointer load_list_data,gpointer divelist_callback)
{
  gint rc;
  gchar *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  rc=sqlite3_exec(logbook_db,
    "SELECT dive_number,dive_datetime,dive_maxdepth,dive_duration,tSite_Full_Names.site_full_name,dive_id FROM Dive LEFT JOIN tSite_Full_Names ON tSite_Full_Names.site_id=Dive.site_id ORDER BY dive_number DESC",
    (gpointer)divelist_callback,load_list_data,&sqlErrMsg
  );
  if(rc!=SQLITE_OK) {
    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,"Error in dive_db_divelist_load_store()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  return rval;
}

gboolean dive_db_load_dive_detail(gint dive_id, gpointer divedetail_callback)
{
  gint rc;
  gchar *sqlcmd, *sqlErrMsg = NULL;
  gboolean rval = TRUE;
  sqlcmd=sqlite3_mprintf(
    /* REMOVE "SELECT T1.dive_datetime,T1.dive_mintemp,T1.dive_maxtemp,T1.dive_notes,T1.dive_visibility,T1.dive_weight,T1.site_id,T2.dive_datetime,T2.dive_duration FROM Dive T1 LEFT JOIN Dive T2 on T2.dive_number=T1.dive_number-1 WHERE T1.dive_id=%d",*/
    "SELECT T1.dive_datetime,T1.dive_mintemp,T1.dive_maxtemp,T1.dive_notes,T1.dive_visibility,T1.dive_weight,T1.site_id,T2.dive_datetime,T2.dive_duration,T3.dive_datetime FROM Dive T1 LEFT JOIN Dive T2 on T2.dive_number=T1.dive_number-1 LEFT JOIN Dive T3 on T3.dive_number=T1.dive_number+1 WHERE T1.dive_id=%d",
    dive_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,(gpointer)divedetail_callback,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in dive_db_load_dive_detail()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free (sqlErrMsg);
    rval=FALSE;
  }
  sqlite3_free (sqlcmd);
  return rval;
}

gboolean dive_db_update(gint dive_id,gchar *dive_datetime,gulong dive_duration,gdouble dive_maxdepth,gdouble dive_mintemp,gdouble dive_maxtemp,gchar *dive_notes,gint site_id,gdouble dive_visibility,gdouble dive_weight)
{
  gint rc;
  gchar *sqlcmd,*sqlErrMsg=NULL;
  gboolean rval = TRUE; 

  sqlcmd=sqlite3_mprintf(
    "UPDATE Dive SET dive_datetime='%s',dive_duration=%lu,dive_maxdepth=%f,dive_mintemp=%f,dive_maxtemp=%f,"
    "dive_notes='%q',site_id=%d,dive_visibility=%f,dive_weight=%f WHERE dive_id=%d;",                
    dive_datetime, dive_duration, dive_maxdepth, dive_mintemp, dive_maxtemp,
    dive_notes,site_id, dive_visibility, dive_weight, dive_id
  );
  if(handle_transactions) db_begin_transaction();
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,"Error in dive_db_update()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
    if(handle_transactions) db_rollback_transaction();
  }
  else {
    db_not_saved();
    if(handle_transactions) db_commit_transaction();
    preferences_load_template_dive_number();
  }
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean dive_db_delete(gint dive_id,glong dive_number)
{
  gint rc;
  gchar *sqlcmd, *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  sqlcmd=sqlite3_mprintf("DELETE FROM Dive WHERE dive_id=%d",dive_id); 
  if(handle_transactions)  db_begin_transaction();
  rc=sqlite3_exec (logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in dive_db_delete()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
    if(handle_transactions)  db_rollback_transaction();
  }
  else {
    db_not_saved();
    if(handle_transactions) db_commit_transaction();
    preferences_load_template_dive_number();
  }
  sqlite3_free(sqlcmd);
  return rval;
}

gint dive_db_insert(gchar *dive_datetime,gulong dive_duration,gdouble dive_maxdepth,gdouble dive_mintemp,gdouble dive_maxtemp, gchar *dive_notes,gint site_id,gdouble dive_visibility, gdouble dive_weight)
{
  gint rc;
  gchar *sqlcmd,*sqlErrMsg=NULL;
  gint new_dive_id=0;

  sqlcmd=sqlite3_mprintf(
    "INSERT INTO Dive (dive_datetime,dive_duration,dive_maxdepth,dive_mintemp,dive_maxtemp,dive_notes,site_id,dive_visibility,dive_weight) VALUES ('%s',%lu,%f,%f,%f,'%q',%d,%f,%f)",
    dive_datetime, dive_duration,dive_maxdepth,dive_mintemp,
    dive_maxtemp,dive_notes,site_id,dive_visibility,dive_weight
  );   
  if(handle_transactions) db_begin_transaction();
  rc=sqlite3_exec (logbook_db, sqlcmd, NULL, 0, &sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,"Error in dive_db_insert()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
  }
  else {
    new_dive_id=sqlite3_last_insert_rowid(logbook_db);
    db_not_saved();    

    /* insert child entities for template */
    if(dive_db_template_dive_number>0) {
      sqlite3_free (sqlcmd);
      sqlcmd =sqlite3_mprintf(
        "INSERT INTO Dive_Buddy (dive_id,buddy_id) SELECT %d,buddy_id FROM Dive_Buddy JOIN Dive ON Dive_Buddy.dive_id=Dive.dive_id WHERE dive_number=%ld;"
        "INSERT INTO Dive_Equipment (dive_id,equipment_id) SELECT %d,equipment_id FROM Dive_Equipment JOIN Dive ON Dive_Equipment.dive_id=Dive.dive_id WHERE dive_number=%ld;"
        "INSERT INTO Dive_Type (dive_id,type_id) SELECT %d,type_id FROM Dive_Type JOIN Dive ON Dive_Type.dive_id=Dive.dive_id WHERE dive_number=%ld;"
        "INSERT INTO Dive_Tank (dive_id,tank_id,dive_tank_avg_depth,dive_tank_O2,dive_tank_He,dive_tank_stime,dive_tank_etime,dive_tank_spressure,dive_tank_epressure) SELECT %d,tank_id,%f,dive_tank_O2,dive_tank_He,%d,%d,dive_tank_spressure,dive_tank_epressure FROM Dive_Tank JOIN Dive ON Dive_Tank.dive_id=Dive.dive_id WHERE dive_number=%ld",
        new_dive_id, dive_db_template_dive_number, new_dive_id,
        dive_db_template_dive_number, new_dive_id,
        dive_db_template_dive_number, new_dive_id, NULL_DEPTH,
        NULL_TIME, NULL_TIME, dive_db_template_dive_number
      );
      rc=sqlite3_exec (logbook_db, sqlcmd, NULL, 0, &sqlErrMsg);
      if(rc != SQLITE_OK) {
        g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in dive_db_insert()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
        sqlite3_free(sqlErrMsg);
        new_dive_id=0;
      }
    }
  }
  if(handle_transactions) {
    if(new_dive_id) db_commit_transaction();
    else db_rollback_transaction();
    preferences_load_template_dive_number();
  }
  sqlite3_free(sqlcmd);
  return new_dive_id;
}

gboolean dive_db_get_min_max_dive_number(glong *min_dive_number,glong *max_dive_number)
{
  gint rc;
  gchar *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  rc=sqlite3_exec(logbook_db,"SELECT MIN(dive_number) FROM Dive",
		(gpointer) db_generic_callback_long,min_dive_number,&sqlErrMsg
  );
  if(rc!=SQLITE_OK) {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in dive_db_get_min_max_dive_number()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else {
    rc=sqlite3_exec(logbook_db,"SELECT MAX(dive_number) FROM Dive",
      (gpointer)db_generic_callback_long, max_dive_number,&sqlErrMsg
    );
    if(rc!=SQLITE_OK) {
	    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in dive_db_get_min_max_dive_number()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
	    sqlite3_free (sqlErrMsg);
	    rval=FALSE;
	  }
  }
  return rval;
}

void dive_db_set_current_template(glong template_dive_number)
{
  dive_db_template_dive_number=template_dive_number;
}

gint dive_db_match_import(gchar *datetime,gdouble max_depth,glong duration,gdouble max_temperature,gdouble min_temperature,GArray *profile)
{
  
  gint rc;
  gchar *sqlErrMsg = NULL,*sqlcmd;
  gchar datetime_upr[50], datetime_lwr[50];
  struct tm t1;
  time_t tt;
  gint dive_id=0;

  strptime(datetime,DATETIME_STR_FMT,&t1);
  tt=mktime(&t1);
  tt+=preferences.match_variance;
  strftime(datetime_upr,50,DATETIME_STR_FMT,localtime(&tt));
  tt-=(preferences.match_variance*2);
  strftime(datetime_lwr,50,DATETIME_STR_FMT,localtime (&tt));

	sqlcmd=sqlite3_mprintf(
    "SELECT dive_id FROM Dive WHERE dive_datetime > '%q' AND dive_datetime < '%q'",
	  datetime_lwr,datetime_upr
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,(gpointer) db_generic_callback_int,&dive_id,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,
	    "Error in dive_db_match_import()\nCode=%d\nError Message='%s'\n",
	    rc, sqlErrMsg
    );
    sqlite3_free(sqlErrMsg);
    dive_id =-1;
  }
  else if(dive_id>0){
    sqlite3_free(sqlcmd);
    sqlcmd=sqlite3_mprintf(
      "UPDATE Dive SET dive_datetime='%q',dive_maxdepth=%f,dive_duration=%ld,dive_maxtemp=%f,dive_mintemp=%f WHERE dive_id=%d",
	     datetime,max_depth,duration,max_temperature,min_temperature,dive_id
    );
    rc=sqlite3_exec(logbook_db, sqlcmd, NULL,0, &sqlErrMsg);
    if(rc!=SQLITE_OK) {
      g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in dive_db_match_import()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
      sqlite3_free (sqlErrMsg);
      dive_id=-1;
    }
    else import_profile(dive_id,profile);
  }
  sqlite3_free(sqlcmd);
  return dive_id;
}

static gint dive_db_merge_next_callback(MergeDiveData * merge_data, gint argc,gchar ** argv, gchar ** azColName)
{
  struct tm t;
  time_t dive1_start,dive1_end,dive2_start,dive2_end;
  gdouble max_depth, max_temperature, min_temperature;
  
  strptime(merge_data->datetime,DATETIME_STR_FMT,&t);
  dive1_start=mktime(&t);
  dive1_end=dive1_start + merge_data->duration;
  
  strptime(argv[2],DATETIME_STR_FMT,&t);
  dive2_start=mktime(&t);
  dive2_end=dive2_start+strtol(argv[4],NULL,0);
  
  merge_data->dive_id=strtol(argv[0],NULL,0);
  merge_data->dive_number=strtol(argv[1],NULL,0);
  merge_data->duration=dive2_end-dive1_start;
  merge_data->duration_mod=dive2_start-dive1_start;
  merge_data->SI=dive2_start-dive1_end;
  max_depth = g_strtod(argv[3],NULL);
  if(max_depth > merge_data->max_depth) merge_data->max_depth = max_depth;
  max_temperature=g_strtod(argv[5],NULL);
  if(max_temperature>merge_data->max_temperature) merge_data->max_temperature=max_temperature;
  min_temperature=g_strtod(argv[6],NULL);
  if(min_temperature<merge_data->min_temperature) merge_data->min_temperature=min_temperature;
  merge_data->has_profile=argv[7]?TRUE:FALSE;
  
  return 0;
}  
  
gboolean dive_db_merge_next(gint dive_id,glong dive_number,MergeDiveData *merge_data)
{
  gint rc;
  gchar *sqlErrMsg=NULL,*sqlcmd,*tptr;
  gboolean rval=TRUE;
  glong sample_interval;
  gdouble profile_temperature;
  
  sqlcmd=sqlite3_mprintf(
    "SELECT Dive.dive_id,dive_number,dive_datetime,dive_maxdepth,dive_duration,dive_maxtemp,dive_mintemp,Profile.dive_id FROM Dive LEFT JOIN Profile ON Profile.dive_id=Dive.dive_id WHERE dive_number=%d LIMIT 1",
	  dive_number+1
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,(gpointer)dive_db_merge_next_callback,merge_data,&sqlErrMsg);
  if(rc!=SQLITE_OK){
    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,"Error in dive_db_merge_next()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else {
    sqlite3_free(sqlcmd);
    /* do merge
    /* 1 - If both dives have profiles and the SI < 1 min, insert a profile segment   */
    /* one interval before second dive with same depth and temperature as last        */
    /* profile segment of dive 1                                                      */
    /* 2 - Update duration maxdepth and temperatures of dive one                      */
    /* 3 - If dive 2 has profile update the profile segments for dive 2 to dive 1 by  */
    /* setting to id of dive one and adding time to profile_time                      */
    /* 4 - delete dive two by calling dive_db_delete()                                */

    sqlcmd=sqlite3_mprintf(
      "UPDATE Dive SET dive_maxdepth=%f,dive_duration=%d,dive_maxtemp=%f,dive_mintemp=%f WHERE dive_id=%d",
	    merge_data->max_depth,merge_data->duration,merge_data->max_temperature,merge_data->min_temperature,dive_id
    );    
    if(merge_data->has_profile) {
      tptr=sqlcmd;
      sqlcmd=sqlite3_mprintf(
        "%s;UPDATE Profile SET profile_time=profile_time+%d,dive_id=%d WHERE dive_id=%d",
        sqlcmd,merge_data->duration_mod,dive_id,merge_data->dive_id
      );
      sqlite3_free(tptr);
      if(profile_exists() && merge_data->SI>60) {
        tptr=sqlcmd;
        sqlcmd=sqlite3_mprintf(
          "INSERT INTO Profile (dive_id,profile_time,profile_depth,profile_temperature) "
            "SELECT %d, %d-(SELECT MAX(profile_time)/COUNT(*) FROM Profile WHERE dive_id=%d),"
              "profile_depth,profile_temperature FROM Profile WHERE dive_id=%d "
              "ORDER BY profile_time DESC LIMIT 1;%s",
          dive_id,merge_data->duration_mod,merge_data->dive_id,dive_id,sqlcmd
        );
        sqlite3_free(tptr);
      }
    }
    rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
    if(rc!=SQLITE_OK) {
      g_log (G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in dive_db_merge_next()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
      sqlite3_free(sqlErrMsg);
      rval=FALSE;
    }
    else dive_db_delete(merge_data->dive_id,merge_data->dive_number);
  }
  sqlite3_free(sqlcmd);
  return rval;
}

gboolean dive_db_split(gint dive_id,glong time_split,gchar *new_dive_datetime)
{
  gint rc,new_dive_id=0;
  gchar *sqlErrMsg=NULL,*sqlcmd;
  gboolean rval=TRUE;

  sqlcmd=sqlite3_mprintf(
    "INSERT INTO Dive (dive_datetime,dive_notes,site_id,dive_visibility,dive_weight) SELECT '%q',dive_notes,site_id,dive_visibility,dive_weight FROM Dive WHERE dive_id=%d",
    new_dive_datetime,dive_id
  );
  rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
  if(rc!=SQLITE_OK) {
    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,"Error in dive_db_split()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
    sqlite3_free(sqlErrMsg);
    rval=FALSE;
  }
  else {
    new_dive_id=sqlite3_last_insert_rowid(logbook_db);
    sqlite3_free(sqlcmd);   
    sqlcmd=sqlite3_mprintf(
      "UPDATE Profile SET dive_id=%d,profile_time=profile_time-%d WHERE dive_id=%d AND profile_time>=%d;" 
      "UPDATE Dive SET " 
        "dive_maxdepth=(SELECT MAX(profile_depth) FROM Profile WHERE dive_id=%d),"
        "dive_duration=(SELECT profile_time FROM Profile WHERE dive_id=%d ORDER BY profile_time DESC LIMIT 1),"
        "dive_maxtemp=(SELECT MAX(profile_temperature) FROM Profile WHERE dive_id=%d),"
        "dive_mintemp=(SELECT MIN(profile_temperature) FROM Profile WHERE dive_id=%d) "
      "WHERE dive_id=%d;"
      "UPDATE Dive SET " 
        "dive_maxdepth=(SELECT MAX(profile_depth) FROM Profile WHERE dive_id=%d),"
        "dive_duration=(SELECT profile_time FROM Profile WHERE dive_id=%d ORDER BY profile_time DESC LIMIT 1),"
        "dive_maxtemp=(SELECT MAX(profile_temperature) FROM Profile WHERE dive_id=%d),"
        "dive_mintemp=(SELECT MIN(profile_temperature) FROM Profile WHERE dive_id=%d) "
      "WHERE dive_id=%d;"
      "INSERT INTO Dive_Buddy (dive_id,buddy_id) SELECT %d,buddy_id FROM Dive_Buddy WHERE dive_id=%d;"
      "INSERT INTO Dive_Equipment (dive_id,equipment_id) SELECT %d,equipment_id FROM Dive_Equipment WHERE dive_id=%d;"
      "INSERT INTO Dive_Type (dive_id,type_id) SELECT %d,type_id FROM Dive_Type WHERE dive_id=%d;"
      "INSERT INTO Dive_Tank (dive_id,tank_id,dive_tank_O2,dive_tank_He,dive_tank_stime,dive_tank_etime,dive_tank_spressure,dive_tank_epressure) SELECT %d,tank_id,dive_tank_O2,dive_tank_He,%f,%f,%f,%f FROM Dive_Tank WHERE dive_id=%d",
      new_dive_id,time_split,dive_id,time_split,
      dive_id,dive_id,dive_id,dive_id,dive_id,
      new_dive_id,new_dive_id,new_dive_id,new_dive_id,new_dive_id,
      new_dive_id,dive_id,
      new_dive_id,dive_id,
      new_dive_id,NULL_TIME,NULL_TIME,NULL_PRESSURE,NULL_PRESSURE,dive_id,
      new_dive_id,dive_id
    );
    rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
    if(rc!=SQLITE_OK) {
      g_log (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,"Error in dive_db_split()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
      sqlite3_free(sqlErrMsg);
      rval=FALSE;
    }
  }
  sqlite3_free(sqlcmd);
  return rval;
}
