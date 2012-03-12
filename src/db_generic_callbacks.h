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

#ifndef DB_GENERIC_CALLBACKS_H
#define DB_GENERIC_CALLBACKS_H

gint db_generic_callback_boolean(gboolean *boolean_value,gint argc,gchar **argv,gchar **azColName);
gint db_generic_callback_long(glong *long_value,gint argc,gchar **argv,gchar **azColName);
gint db_generic_callback_int(gint *int_value,gint argc,gchar **argv, gchar **azColName);
gint db_generic_callback_double(gdouble *double_value,gint argc,gchar **argv, gchar **azColName);
gint db_generic_callback_string(gchar **string, gint argc, gchar **argv,gchar **azColName);

#endif /* DB_GENERIC_CALLBACKS_H */
