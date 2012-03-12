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

#ifndef TYPE_DB_H
#define TYPE_DB_H

gboolean type_db_typelist_load_store (gint dive_id, gpointer load_list_data,
				      gpointer typelist_callback);
gboolean type_db_add_type_to_dive (gint dive_id, gint type_id);
gboolean type_db_remove_type_from_dive (gint dive_id, gint type_id);
gboolean type_db_load_type_detail (gint type_id,
				   gpointer typedetail_callback);
gint type_db_insert (gchar * type_name, gchar * type_notes);
gboolean type_db_update (gint type_id, gchar * type_name,
			 gchar * type_notes);
gboolean type_db_delete (gint type_id);

#endif /* TYPE_DB_H */
