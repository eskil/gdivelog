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

#ifndef TANK_DB_H
#define TANK_DB_H

gboolean tank_db_tanklist_load_store (gpointer load_list_data,
				      gpointer tanklist_callback);
gboolean tank_db_tank_on_dive (gint tank_id, gint dive_id);
gboolean tank_db_add_tank_to_dive (gint dive_id, gint tank_id);
gboolean tank_db_remove_tank_from_dive (gint dive_id, gint tank_id);
gboolean tank_db_load_tank_detail (gint tank_id,
				   gpointer tankdetail_callback);
gint tank_db_insert (gchar * tank_name, gdouble tank_volume,
		       gdouble tank_wp, gchar * tank_notes);
gboolean tank_db_update (gint tank_id, gchar * tank_name,
			 gdouble tank_volume, gdouble tank_wp,
			 gchar * tank_notes);
gboolean tank_db_delete (gint tank_id);

#endif /* TANK_DB_H */
