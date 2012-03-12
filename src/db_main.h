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

#ifndef DB_MAIN_H
#define DB_MAIN_H

#include <sqlite3.h>

gboolean db_create_schema(sqlite3 *db);

gboolean db_cache_sites(void);
gchar *db_get_full_site_name(gchar *site_id);

gboolean db_is_saved(void);
void db_not_saved(void);
void db_saved(void);

gboolean db_open(gchar * fname);
gboolean db_new(gchar * fname);
gboolean db_save(void);
gboolean db_close(void);
gboolean db_begin_transaction(void);
gboolean db_commit_transaction(void);
gboolean db_rollback_transaction(void);

gint db_execute_query(gchar *statement,gpointer callback,gpointer user_data,gchar *error_message);
gchar *db_get_last_dive_datetime(void);
#endif /* DB_MAIN_H */
