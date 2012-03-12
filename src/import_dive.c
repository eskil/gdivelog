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
#include <string.h>

#include "globals.h"
#include "gdivelog.h"
#include "buddy_db.h"
#include "dive_db.h"
#include "dive_tank_db.h"
#include "equipment_db.h"
#include "import_dive.h"
#include "profile_db.h"
#include "site_db.h"
#include "tank_db.h"
#include "type_db.h"
#include "support.h"

extern sqlite3 *logbook_db;
extern handle_transactions;

typedef struct
{
  gint id;
  gint parent_id;
  ImportSite site;
  gchar *name_updated;
  gchar *notes_updated;
} SiteData;


typedef struct
{
  gint id;
  ImportBuddies buddy;
  gchar *name_updated;
  gchar *notes_updated;
} BuddyData;

typedef struct
{
  gint id;
  ImportEquipment equipment;
  gchar *name_updated;
  gchar *notes_updated;
} EquipmentData;

typedef struct
{
  gint id;
  ImportTypes type;
  gchar *name_updated;
  gchar *notes_updated;
} TypeData;

typedef struct
{
  gint id;
  ImportTanks tank;
  gchar *name_updated;
  gchar *notes_updated;
} TankData;

static gchar *import_set_notes(gchar *original_notes, gchar *import_notes)
{
  gchar *rval = NULL, *start, *temp;
  gint num_chars;

  if(!import_notes) rval = strdup (original_notes);
  else
    {
      if (g_utf8_collate (original_notes, import_notes))
	{
	  start = g_strrstr (original_notes, "<import>\n");
	  if (start)
	    {
	      start += 9;
	      num_chars = g_strrstr (start, "\n</import>") - start;
	      temp = g_strndup (start, num_chars);
	      if (!g_utf8_collate (import_notes, temp))
		rval = g_strdup (original_notes);
	      g_free (temp);
	    }
	  if (!rval)
	    rval =
             g_strconcat( original_notes, "\n\n<import>\n" , 
                          import_notes, "\n</import>\n", NULL);
	}
      else
	rval = g_strdup (original_notes);
    }
  return rval;
}

static gint
import_site_callback (SiteData * site_data, gint argc, gchar ** argv,
		      gchar ** azColName)
{
  gchar *notes;

  site_data->id = strtol(argv[0],NULL,0);
  site_data->parent_id = strtol(argv[1],NULL,0);
  site_data->name_updated = g_strdup (argv[2]);
  site_data->notes_updated =
    import_set_notes (argv[3], site_data->site.notes);
  return 0;
}

static gint import_sites(GArray *sites)
{
  gint i;
  ImportSite site;
  SiteData site_data;
  gint rc;
  gchar *sqlcmd,*sqlErrMsg=NULL;
  gboolean rval=TRUE;

  site_data.parent_id = 0;
  for (i=0;i<sites->len;i++) {
    site_data.site = g_array_index (sites, ImportSite, i);
    site_data.id = 0;
    sqlcmd=sqlite3_mprintf(
      "SELECT site_id,site_parent_id,site_name,site_notes FROM Site WHERE site_name like '%q' AND site_parent_id=%d",
	    site_data.site.name, site_data.parent_id
    );
    rc=sqlite3_exec(logbook_db,sqlcmd,(gpointer)import_site_callback,&site_data,&sqlErrMsg);
    if(rc!=SQLITE_OK) {
	    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in import_sites()\nCode=%d\nQuery='%s'\nError Message='%s'\n",rc,sqlcmd,sqlErrMsg);
	    sqlite3_free(sqlErrMsg);
	    rval=FALSE;
	  }
    else {
	    if(!site_data.id) {
	      site_data.id=site_db_new(site_data.parent_id,site_data.site.name,site_data.site.notes);
	      if(site_data.id<=0) rval=FALSE;           
	    }
	    else {
	      if(!site_db_update(site_data.id, site_data.parent_id, site_data.name_updated,site_data.notes_updated)) rval=FALSE;
	      g_free(site_data.name_updated);
	      g_free(site_data.notes_updated);
	    }
	  }
    sqlite3_free (sqlcmd);
    if(!rval)	break;
    site_data.parent_id = site_data.id;
  }
  if(!rval) return 0;
  else return site_data.id;
}

static gint
import_buddy_callback (BuddyData * buddy_data, gint argc, gchar ** argv,
		       gchar ** azColName)
{
  gchar *notes;

  buddy_data->id = strtol(argv[0],NULL,0);
  buddy_data->name_updated = g_strdup (argv[1]);
  buddy_data->notes_updated =
    import_set_notes (argv[2], buddy_data->buddy.notes);
  return 0;
}

static gboolean
import_buddies (gint dive_id, GArray * buddies)
{
  gint i;
  BuddyData buddy_data;
  gint rc;
  gchar *sqlcmd, *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  for (i = 0; i < buddies->len; i++)
    {
      buddy_data.buddy = g_array_index (buddies, ImportBuddies, i);
      buddy_data.id = 0;
      sqlcmd =
	sqlite3_mprintf
	("SELECT buddy_id,buddy_name,buddy_notes FROM Buddy WHERE buddy_name like '%q'",
	 buddy_data.buddy.name);
      rc =
	sqlite3_exec (logbook_db, sqlcmd, (gpointer) import_buddy_callback,
		      &buddy_data, &sqlErrMsg);
      if (rc != SQLITE_OK)
	{
	  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,
		 "Error in import_buddies()\nCode=%d\nQuery='%s'\nError Message='%s'\n",
		 rc, sqlcmd, sqlErrMsg);
	  sqlite3_free (sqlErrMsg);
	  rval = FALSE;
	}
      else
	{
	  if (!buddy_data.id)
	    {
	      buddy_data.id =
		buddy_db_insert (buddy_data.buddy.name,
				 buddy_data.buddy.notes);
	      if (buddy_data.id <= 0)
		rval = FALSE;
	    }
	  else
	    {
	      if (!buddy_db_update
		  (buddy_data.id, buddy_data.name_updated,
		   buddy_data.notes_updated))
		rval = FALSE;
	      g_free (buddy_data.name_updated);
	      g_free (buddy_data.notes_updated);
	    }
	  if (rval)
	    {
	      if (!buddy_db_add_buddy_to_dive (dive_id, buddy_data.id))
		rval = FALSE;
	    }
	}
      sqlite3_free (sqlcmd);
      if (!rval)
	{
	  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,
		 "Error in import_buddies()\nbuddy_id=%d\nbuddy_name=%s\nbuddy_notes=%s\n",
		 buddy_data.id, buddy_data.buddy.name,
		 buddy_data.buddy.notes);
	  break;
	}
    }
}

static gint
import_equipment_callback (EquipmentData * equipment_data, gint argc,
			   gchar ** argv, gchar ** azColName)
{
  equipment_data->id = strtol(argv[0],NULL,0);
  equipment_data->name_updated = g_strdup (argv[1]);
  equipment_data->notes_updated =
    import_set_notes (argv[2], equipment_data->equipment.notes);
  return 0;
}

static gboolean
import_equipment (gint dive_id, GArray * equipment)
{
  gint i;
  EquipmentData equipment_data;
  gint rc;
  gchar *sqlcmd, *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  for (i = 0; i < equipment->len; i++)
    {
      equipment_data.equipment =
	g_array_index (equipment, ImportEquipment, i);
      equipment_data.id = 0;
      sqlcmd =
	sqlite3_mprintf
	("SELECT equipment_id,equipment_name,equipment_notes FROM equipment WHERE equipment_name like '%q'",
	 equipment_data.equipment.name);
      rc =
	sqlite3_exec (logbook_db, sqlcmd,
		      (gpointer) import_equipment_callback, &equipment_data,
		      &sqlErrMsg);
      if (rc != SQLITE_OK)
	{
	  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,
		 "Error in import_equipment()\nCode=%d\nQuery='%s'\nError Message='%s'\n",
		 rc, sqlcmd, sqlErrMsg);
	  sqlite3_free (sqlErrMsg);
	  rval = FALSE;
	}
      else
	{
	  if (!equipment_data.id)
	    {
	      equipment_data.id =
		equipment_db_insert (equipment_data.equipment.name,
				     equipment_data.equipment.notes);
	      if (equipment_data.id <= 0)
		rval = FALSE;
	    }
	  else
	    {
	      if (!equipment_db_update
		  (equipment_data.id, equipment_data.name_updated,
		   equipment_data.notes_updated))
		rval = FALSE;
	      g_free (equipment_data.name_updated);
	      g_free (equipment_data.notes_updated);

	    }
	  if (rval)
	    {
	      if (!equipment_db_add_equipment_to_dive
		  (dive_id, equipment_data.id))
		rval = FALSE;
	    }
	}
      sqlite3_free (sqlcmd);
      if (!rval)
	{
	  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,
		 "Error in import_equipment()\nequipment_id=%d\nequipment_name=%s\nequipment_notes=%s\n",
		 equipment_data.id, equipment_data.equipment.name,
		 equipment_data.equipment.notes);
	  break;
	}
    }
}

static gint
import_type_callback (TypeData * type_data, gint argc, gchar ** argv,
		      gchar ** azColName)
{
  type_data->id = strtol(argv[0],NULL,0);
  type_data->name_updated = g_strdup (argv[1]);
  type_data->notes_updated =
    import_set_notes (argv[2], type_data->type.notes);
  return 0;
}

static gboolean
import_types (gint dive_id, GArray * types)
{
  gint i;
  TypeData type_data;
  gint rc;
  gchar *sqlcmd, *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  for (i = 0; i < types->len; i++)
    {
      type_data.type = g_array_index (types, ImportTypes, i);
      type_data.id = 0;
      sqlcmd =
	sqlite3_mprintf
	("SELECT type_id,type_name,type_notes FROM type WHERE type_name like '%q'",
	 type_data.type.name);
      rc =
	sqlite3_exec (logbook_db, sqlcmd, (gpointer) import_type_callback,
		      &type_data, &sqlErrMsg);
      if (rc != SQLITE_OK)
	{
	  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,
		 "Error in import_types()\nCode=%d\nQuery='%s'\nError Message='%s'\n",
		 rc, sqlcmd, sqlErrMsg);
	  sqlite3_free (sqlErrMsg);
	  rval = FALSE;
	}
      else
	{
	  if (!type_data.id)
	    {
	      type_data.id =
		type_db_insert (type_data.type.name, type_data.type.notes);
	      if (type_data.id <= 0)
		rval = FALSE;
	    }
	  else
	    {
	      if (!type_db_update
		  (type_data.id, type_data.name_updated,
		   type_data.notes_updated))
		rval = FALSE;
	      g_free (type_data.name_updated);
	      g_free (type_data.notes_updated);
	    }
	  if (rval)
	    {
	      if (!type_db_add_type_to_dive (dive_id, type_data.id))
		rval = FALSE;
	    }
	}
      sqlite3_free (sqlcmd);
      if (!rval)
	{
	  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,
		 "Error in import_types()\ntype_id=%d\ntype_name=%s\ntype_notes=%s\n",
		 type_data.id, type_data.type.name, type_data.type.notes);
	  break;
	}
    }
}

static gint
import_tank_callback (TankData * tank_data, gint argc, gchar ** argv,
		      gchar ** azColName)
{
  tank_data->id = strtol(argv[0],NULL,0);
  tank_data->name_updated = g_strdup (argv[1]);
  tank_data->notes_updated =
    import_set_notes (argv[4], tank_data->tank.notes);
  return 0;
}

static gboolean
import_tanks (gint dive_id, GArray * tanks)
{
  gint i;
  TankData tank_data;
  gint rc;
  gchar *sqlcmd, *sqlErrMsg = NULL;
  gboolean rval = TRUE;

  for (i = 0; i < tanks->len; i++)
    {
      tank_data.tank = g_array_index (tanks, ImportTanks, i);
      tank_data.id = 0;
      sqlcmd =
	sqlite3_mprintf
	("SELECT tank_id,tank_name,tank_volume,tank_wp,tank_notes FROM tank WHERE tank_name like '%q' AND tank_volume=%f AND tank_wp=%f",
	 tank_data.tank.name, tank_data.tank.volume, tank_data.tank.wp);
      rc =
	sqlite3_exec (logbook_db, sqlcmd, (gpointer) import_tank_callback,
		      &tank_data, &sqlErrMsg);
      if (rc != SQLITE_OK)
	{
	  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,
		 "Error in import_tanks()\nCode=%d\nQuery='%s'\nError Message='%s'\n",
		 rc, sqlcmd, sqlErrMsg);
	  sqlite3_free (sqlErrMsg);
	  rval = FALSE;
	}
      else
	{
	  if (!tank_data.id)
	    {
	      tank_data.id =
		tank_db_insert (tank_data.tank.name, tank_data.tank.volume,
				tank_data.tank.wp, tank_data.tank.notes);
	      if (tank_data.id <= 0)
		rval = FALSE;
	    }
	  else
	    {
	      if (!tank_db_update
		  (tank_data.id, tank_data.name_updated,
		   tank_data.tank.volume, tank_data.tank.wp,
		   tank_data.notes_updated))
		{
		  rval = FALSE;
		}
	      g_free (tank_data.name_updated);
	      g_free (tank_data.notes_updated);
	    }
	  if (rval)
	    {
	      if (!dive_tank_db_insert
		  (dive_id, tank_data.id, tank_data.tank.average_depth,
		   tank_data.tank.O2, tank_data.tank.He,
		   tank_data.tank.time_start, tank_data.tank.time_end,
		   tank_data.tank.pressure_start,
		   tank_data.tank.pressure_end))
		rval = FALSE;
	    }
	}
      sqlite3_free (sqlcmd);
      if (!rval)
	{
	  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,
		 "Error in import_tanks()\ntank_id=%d\ntank_name=%s\ntank_notes=%s\n",
		 tank_data.id, tank_data.tank.name, tank_data.tank.notes);
	  break;
	}
    }
}

gint import_profile(gint dive_id,GArray *profile)
{
  gint i;
  ImportProfile *profile_segment;

  profile_db_delete(dive_id);
  for(i=0;i<profile->len;i++) {
    profile_segment=&g_array_index(profile,ImportProfile,i);
    if(!profile_db_insert_segment(dive_id, profile_segment->time, profile_segment->depth,profile_segment->temperature))	break;
  }
  return i;
}

gint import_dive(gchar * dive_datetime, glong dive_duration,gdouble dive_maxdepth,gdouble dive_mintemp,
	     gdouble dive_maxtemp,gchar *dive_notes,
	     gdouble dive_visibility, gdouble dive_weight, GArray * sites,
	     GArray * buddies, GArray * equipment, GArray * types,
	     GArray * tanks, GArray * profile, gboolean use_template)
{
  gint site_id=0, dive_id=0;

  handle_transactions=FALSE;
  if(sites) site_id=import_sites(sites);
  if(!use_template) dive_db_set_current_template(-1);
  dive_id=dive_db_insert(dive_datetime,dive_duration,dive_maxdepth,dive_mintemp,dive_maxtemp,dive_notes,site_id,dive_visibility,dive_weight);
  if(!use_template) dive_db_set_current_template(preferences.template_dive_number);
  if(dive_id){
    if(buddies) import_buddies(dive_id,buddies);
    if(equipment) import_equipment(dive_id,equipment);
    if(types) import_types(dive_id,types);
    if(tanks) import_tanks(dive_id,tanks);
    if(profile) import_profile(dive_id,profile);
  }
  return dive_id;
}
