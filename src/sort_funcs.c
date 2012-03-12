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

/* GtkTreeIterCompareFuncs - user_data is the column id (not a pointer to it, the actual id) */

#include <gtk/gtk.h>

#include "format_fields.h"
#include "support.h"

gint sort_func_double(GtkTreeModel *model,GtkTreeIter *a,GtkTreeIter *b,gpointer user_data)
{
  gint rc=0;
  gchar *str1,*str2;
  double d1,d2;

  gtk_tree_model_get(model,a,(gpointer)user_data,&str1,-1);
  gtk_tree_model_get(model,b,(gpointer)user_data,&str2,-1);
  d1=g_strtod(str1,NULL);
  d2=g_strtod(str2,NULL);
  g_free(str1);
  g_free(str2);
  if(d1<d2) rc=-1;
  else if(d1>d2)rc=1;
  return rc;
}

gint sort_func_time(GtkTreeModel *model,GtkTreeIter *a,GtkTreeIter *b,gpointer user_data)
{
  gchar *str1, *str2;
  glong l1, l2;

  gtk_tree_model_get(model,a,(gpointer)user_data,&str1,-1);
  gtk_tree_model_get(model,b,(gpointer)user_data,&str2,-1);
  l1=format_field_get_time_value(str1);
  l2=format_field_get_time_value(str2);
  g_free(str1);
  g_free(str2);
  return (gint)(l1-l2);
}

gint sort_func_long(GtkTreeModel *model,GtkTreeIter *a,GtkTreeIter *b,gpointer user_data)
{
  glong l1, l2;
  gtk_tree_model_get(model,a,(gpointer)user_data,&l1,-1);
  gtk_tree_model_get(model,b,(gpointer)user_data,&l2,-1);
  return (gint)(l1-l2);
}
