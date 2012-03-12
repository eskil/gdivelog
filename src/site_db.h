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

#ifndef SITE_DB_H
#define SITE_DB_H

gint site_db_get_max_id(void);
gboolean site_db_sitetree_load_store(gpointer load_sites_callback,gpointer load_sites_data);
gchar *site_db_get_notes(gint site_id);
gboolean site_db_update(gint site_id,gint site_parent_id,gchar *site_name, gchar *site_notes);
gint site_db_new(gint site_parent_id,gchar *site_name,gchar *site_notes);
gboolean site_db_can_delete(gint site_id);
gboolean site_db_delete(gint site_id);
gboolean site_db_get_site_full_name(gint site_id,gpointer get_site_full_name_callback);
gboolean site_db_parent_combo_load(gpointer load_parents_callback);
gboolean site_db_isancestor(gint site_id, gint ancestor_id);

#endif /* SITE_DB_H */
