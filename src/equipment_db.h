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

#ifndef EQUIPMENT_DB_H
#define EQUIPMENT_DB_H

gboolean equipment_db_equipmentlist_load_store (gint dive_id,
						gpointer load_list_data,
						gpointer
						equipmentlist_callback);
gboolean equipment_db_add_equipment_to_dive (gint dive_id,
					     gint equipment_id);
gboolean equipment_db_remove_equipment_from_dive (gint dive_id,
						  gint equipment_id);
gboolean equipment_db_load_equipment_detail (gint equipment_id,
					     gpointer
					     equipmentdetail_callback);
gint equipment_db_insert (gchar * equipment_name, gchar * equipment_notes);
gboolean equipment_db_update (gint equipment_id, gchar * equipment_name,
			      gchar * equipment_notes);
gboolean equipment_db_delete (gint equipment_id);

#endif /* EQUIPMENT_DB_H */
