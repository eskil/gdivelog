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

gint db_generic_callback_boolean(gboolean *boolean_value,gint argc,gchar **argv,gchar **azColName)
{
  *boolean_value=TRUE;
  return 0;
}

gint db_generic_callback_long(glong *long_value,gint argc,gchar **argv,gchar **azColName)
{
  *long_value=argv[0]?strtol(argv[0],NULL,0):0;
  return 0;
}

gint db_generic_callback_int(gint *int_value,gint argc,gchar **argv,gchar **azColName)
{
  *int_value=argv[0]?strtol(argv[0],NULL,0):0;
  return 0;
}

gint db_generic_callback_double(gdouble *double_value,gint argc,gchar **argv,gchar **azColName)
{
  *double_value=argv[0]?g_strtod(argv[0],NULL):0.0;
  return 0;
}

gint db_generic_callback_string(gchar **string,gint argc,gchar **argv,gchar **azColName)
{
  *string=argv[0]?g_strdup(argv[0]):NULL;
  return 0;
}
