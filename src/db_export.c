/*
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

gboolean
db_export (char *fname)
{
  gchar export_sql[] =
    "CREATE TEMP TABLE teDive (dive_id INTEGER NOT NULL UNIQUE PRIMARY KEY AUTOINCREMENT, dive_number INTEGER, dive_datetime TEXT, dive_duration INTEGER, dive_maxdepth REAL, dive_mintemp REAL, dive_maxtemp REAL, dive_notes TEXT, site_id INTEGER, dive_visibility REAL, dive_weight REAL, odive_id INTEGER, osite_id INTEGER);"
    "INSERT INTO teDive (odive_id,dive_number,dive_datetime,dive_duration,dive_maxdepth,dive_mintemp,dive_maxtemp,dive_notes TEXT,osite_id,dive_visibility,dive_weight) SELECT * FROM Dive;"
    "CREATE TEMP TABLE teSite (site_id INTEGER PRIMARY KEY AUTOINCREMENT,site_parent_id INTEGER,osite_id INTEGER,osite_parent_id INTEGER);"
    "CREATE VIEW tveSite AS SELECT teSite.site_id,teSite.site_parent_id,Site.site_name,Site.site_notes FROM teSite INNER JOIN Site on Site.site_id=teSite.osite_id;"
    "INSERT INTO teSite (osite_id,osite_parent_id,site_parent_id) SELECT site_id,site_parent_id,0 FROM Site;"
    "UPDATE teDive SET site_id=(SELECT teSite.site_id FROM teSite WHERE teSite.osite_id=teDive.osite_id);"
    "CREATE TEMP VIEW tveSiteParentList AS SELECT DISTINCT t2.site_id AS site_id,t2.osite_id AS osite_id FROM teSite t1 JOIN teSite t2 ON t1.osite_parent_id=t2.osite_id;"
    "UPDATE teSite SET site_parent_id=(SELECT site_id FROM tveSiteParentList WHERE tveSiteParentList.osite_id=teSite.oparent_id);"
    "DROP VIEW tveSiteParentList;"
    "INSERT INTO export.Site (site_id,site_parent_id,site_name,site_notes) SELECT * FROM tveSite;"
    "DROP VIEW tveSite;"
    "DROP TABLE teSite;"
    "CREATE TEMP TABLE teBuddy (buddy_id INTEGER PRIMARY KEY AUTOINCREMENT,buddy_name TEXT,buddy_notes TEXT,obuddy_id INTEGER);"
    "INSERT INTO teBuddy (obuddy_id,buddy_name,buddy_notes) SELECT * FROM Buddy;"
    "CREATE TEMP TABLE teDive_Buddy (dive_id INTEGER, buddy_id INTEGER,odive_id INTEGER, obuddy_id INTEGER);"
    "INSERT INTO teDive_Buddy (odive_id,obuddy_id) SELECT * FROM Dive_Buddy;"
    "UPDATE teDive_Buddy SET teDive_Buddy.buddy_id=(SELECT teBuddy.buddy_id FROM teBuddy WHERE teBuddy.obuddy_id=teDive_Buddy.obuddy_id);"
    "UPDATE teDive_Buddy SET teDive_Buddy.dive_id=(SELECT teDive.dive_id FROM teDive WHERE teDive.odive_id=teDive_Buddy.odive_id);"
    "INSERT INTO export.Buddy (buddy_id,buddy_name,buddy_notes) SELECT buddy_id,buddy_name,buddy_notes FROM teBuddy;"
    "DROP TABLE teBuddy;"
    "INSERT INTO export.Dive_Buddy (dive_id,buddy_id) SELECT dive_id,buddy_id FROM teDive_Buddy;"
    "DROP TABLE teDive_Buddy;"
    "CREATE TEMP TABLE teEquipment (equipment_id INTEGER PRIMARY KEY AUTOINCREMENT,equipment_name TEXT,equipment_notes TEXT,oequipment_id INTEGER);"
    "INSERT INTO teEquipment (oequipment_id,equipment_name,equipment_notes) SELECT * FROM Equipment;"
    "CREATE TEMP TABLE teDive_Equipment (dive_id INTEGER, equipment_id INTEGER,odive_id INTEGER, oequipment_id INTEGER);"
    "INSERT INTO teDive_Equipment (odive_id,oequipment_id) SELECT * FROM Dive_Equipment;"
    "UPDATE teDive_Equipment SET teDive_Equipment.equipment_id=(SELECT teEquipment.equipment_id FROM teEquipment WHERE teEquipment.oequipment_id=teDive_Equipment.oequipment_id);"
    "UPDATE teDive_Equipment SET teDive_Equipment.dive_id=(SELECT teDive.dive_id FROM teDive WHERE teDive.odive_id=teDive_Equipment.odive_id);"
    "INSERT INTO export.Equipment (equipment_id,equipment_name,equipment_notes) SELECT equipment_id,equipment_name,equipment_notes FROM teEquipment;"
    "DROP TABLE teEquipment;"
    "INSERT INTO export.Dive_Equipment (dive_id,equipment_id) SELECT dive_id,equipment_id FROM teDive_Equipment;"
    "DROP TABLE teDive_Equipment;"
    "CREATE TEMP TABLE teType (type_id INTEGER PRIMARY KEY AUTOINCREMENT,type_name TEXT,type_notes TEXT,otype_id INTEGER);"
    "INSERT INTO teType (otype_id,type_name,type_notes) SELECT * FROM Type;"
    "CREATE TEMP TABLE teDive_Type (dive_id INTEGER, type_id INTEGER,odive_id INTEGER, otype_id INTEGER);"
    "INSERT INTO teDive_Type (odive_id,otype_id) SELECT * FROM Dive_Type;"
    "UPDATE teDive_Type SET teDive_Type.type_id=(SELECT teType.type_id FROM teType WHERE teType.otype_id=teDive_Type.otype_id);"
    "UPDATE teDive_Type SET teDive_Type.dive_id=(SELECT teDive.dive_id FROM teDive WHERE teDive.odive_id=teDive_Type.odive_id);"
    "INSERT INTO export.Type (type_id,type_name,type_notes) SELECT type_id,type_name,type_notes FROM teType;"
    "DROP TABLE teType;"
    "INSERT INTO export.Dive_Type (dive_id,type_id) SELECT dive_id,type_id FROM teDive_Type;"
    "DROP TABLE teDive_Type;"
    /* TODO - finish SQL */
    ;
  sqlite3 *export_db;
  gchar *sqlcmd;
  gboolean rval = FALSE;
  gint rc;

  if (sqlite3_open (fname, &export_db) != SQLITE_OK)
    {
      g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
	     "Cannot create %s.\nError: %s\n", fname,
	     sqlite3_errmsg (logbook_db));
      sqlite3_close (export_db);
    }
  else
    {
      if (db_create_schema (export_db))
	{
	  sqlite3_close (export_db);
	  sqlcmd = sqlite3_mprintf ("ATTACH '%s' AS export", fname);
	  rc = sqlite3_exec (logbook_db, sqlcmd, NULL, 0, &sqlErrMsg);
	  if (rc != SQLITE_OK)
	    {
	      g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
		     "Error in db_export()\nCode=%d\nQuery='%s'\nError Message='%s'\n",
		     rc, sqlcmd, sqlErrMsg);
	      sqlite3_free (sqlErrMsg);
	    }
	  else
	    {
	      rc = sqlite3_exec (logbook_db, export_sql, NULL, 0, &sqlErrMsg);
	      if (rc != SQLITE_OK)
		{
		  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
			 "Error in db_export()\nCode=%d\nQuery='%s'\nError Message='%s'\n",
			 rc, sqlcmd, sqlErrMsg);
		  sqlite3_free (sqlErrMsg);
		}
	      else
		{
		  rc =
		    sqlite3_exec (logbook_db, "DETACH export", NULL, 0,
				  &sqlErrMsg);
		  if (rc != SQLITE_OK)
		    {
		      g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
			     "Error in db_export()\nCode=%d\nQuery='%s'\nError Message='%s'\n",
			     rc, sqlcmd, sqlErrMsg);
		      sqlite3_free (sqlErrMsg);
		    }
		  else
		    rval = TRUE;
		}
	    }
	  sqlite3_free (sqlcmd);
	}
    }
  return rval;
}
