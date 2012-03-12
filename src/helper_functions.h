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

#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include <gtk/gtk.h>

gchar *helper_gtk_text_view_get_text (GtkTextView * textview);
gboolean helper_gtk_text_view_is_modified (GtkTextView * textview);
void helper_gtk_text_view_set_text (GtkTextView * textview, gchar * text);
void helper_gtk_text_view_clear_modified (GtkTextView * textview);
gboolean helper_model_has_items (GtkTreeModel * tree_model);

#endif /* HELPER_FUNCTIONS_H */
