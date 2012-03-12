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

#ifndef BUDDY_DB_H
#define BUDDY_DB_H

gboolean buddy_db_buddylist_load_store (gint dive_id,
					gpointer load_list_data,
					gpointer buddylist_callback);
gboolean buddy_db_add_buddy_to_dive (gint dive_id, gint buddy_id);
gboolean buddy_db_remove_buddy_from_dive (gint dive_id, gint buddy_id);
gboolean buddy_db_load_buddy_detail (gint buddy_id,
				     gpointer buddydetail_callback);
gint buddy_db_insert (gchar * buddy_name, gchar * buddy_notes);
gboolean buddy_db_update (gint buddy_id, gchar * buddy_name,
			  gchar * buddy_notes);
gboolean buddy_db_delete (gint buddy_id);

#endif /* BUDDY_DB_H */
