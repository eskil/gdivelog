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

#ifndef STATISTICS_DB_H
#define STATISTICS_DB_H

gboolean statistics_db_load_totals(GtkListStore *list_store,gpointer statistics_load_totals_callback);
gboolean statistics_db_load_equipment(GtkListStore *list_store,gpointer statistics_load_list_callback);
gboolean statistics_db_load_buddies(GtkListStore *list_store,gpointer statistics_load_list_callback);
gboolean statistics_db_load_sites(gpointer *statistics_load_sites_data,gpointer statistics_load_sites_callback);
gboolean statistics_db_load_sites_count(gpointer *statistics_load_sites_data,gpointer statistics_load_sites_count_callback);
gboolean statistics_db_load_tanks(GtkListStore *list_store,gpointer statistics_load_list_callback);
gboolean statistics_db_load_types(GtkListStore *list_store,gpointer statistics_load_list_callback);

#endif /* STATISTICS_DB_H */
