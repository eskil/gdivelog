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

#include <gtk/gtk.h>

gchar *
helper_gtk_text_view_get_text (GtkTextView * textview)
{
  /* returned char* must be freed after use */
  GtkTextBuffer *textbuffer;
  GtkTextIter start, end;

  textbuffer = gtk_text_view_get_buffer (textview);
  gtk_text_buffer_get_iter_at_offset (textbuffer, &start, 0);
  gtk_text_buffer_get_iter_at_offset (textbuffer, &end, -1);
  return gtk_text_buffer_get_text (textbuffer, &start, &end, TRUE);
}

void
helper_gtk_text_view_set_text (GtkTextView * textview, gchar * text)
{
  GtkTextBuffer *textbuffer;

  textbuffer = gtk_text_view_get_buffer (textview);
  gtk_text_buffer_set_text (textbuffer, text, -1);
}

void
helper_gtk_text_view_clear_modified (GtkTextView * textview)
{
  GtkTextBuffer *textbuffer;

  textbuffer = gtk_text_view_get_buffer (textview);
  gtk_text_buffer_set_modified (textbuffer, FALSE);
}

gboolean
helper_gtk_text_view_is_modified (GtkTextView * textview)
{
  GtkTextBuffer *textbuffer;

  textbuffer = gtk_text_view_get_buffer (textview);
  return gtk_text_buffer_get_modified (textbuffer);
}

gboolean
helper_model_has_items (GtkTreeModel * tree_model)
{
  GtkTreeIter iter;

  if (!tree_model)
    return FALSE;
  return gtk_tree_model_get_iter_first (tree_model, &iter);
}
