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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "interface.h"
#include "globals.h"
#include "defines.h"
#include "gdivelog.h"
#include "format_fields.h"
#include "helper_functions.h"
#include "buddy_db.h"

enum
{
  BUDDYLIST_COL_ON_DIVE = 0,
  BUDDYLIST_COL_NAME,
  BUDDYLIST_COL_BUDDY_ID,
  BUDDYLIST_NUM_COLS
};

static gint current_buddy_id = 0;
static gboolean buddy_has_dives=FALSE;

static void buddy_set_new_sensitive(gboolean sensitive)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"buddy_new_btn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static void buddy_set_save_sensitive(gboolean sensitive)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"buddy_save_btn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static void buddy_set_delete_sensitive(gboolean sensitive)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"buddy_delete_btn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static const gchar *buddy_detail_get_buddy_name(void)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"buddy_name"));
  return gtk_entry_get_text(GTK_ENTRY(widget));
}

static void buddy_detail_set_buddy_name(gchar *buddy_name)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"buddy_name"));
  gtk_entry_set_text(GTK_ENTRY(widget),buddy_name);
}

static gchar *buddy_detail_get_buddy_notes(void)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"buddy_notes"));
  return helper_gtk_text_view_get_text(GTK_TEXT_VIEW(widget));
}

static void buddy_detail_set_buddy_notes(gchar *buddy_notes)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"buddy_notes"));
  helper_gtk_text_view_set_text(GTK_TEXT_VIEW(widget),buddy_notes); 
}

static buddy_on_dive_toggled(GtkCellRendererToggle *cell,gchar *path_str,gpointer user_data)
{
  GtkTreePath *path;
  GtkTreeIter iter;
  gboolean on_dive;
  gint buddy_id;

  path=gtk_tree_path_new_from_string(path_str);
  gtk_tree_model_get_iter(GTK_TREE_MODEL(user_data),&iter,path);
  gtk_tree_model_get(GTK_TREE_MODEL(user_data),&iter,
    BUDDYLIST_COL_ON_DIVE,&on_dive,
    BUDDYLIST_COL_BUDDY_ID,&buddy_id,
    -1
  );
  if(on_dive) buddy_db_remove_buddy_from_dive(current_dive_id,buddy_id);
  else buddy_db_add_buddy_to_dive(current_dive_id, buddy_id);
  on_dive^=1;
  gtk_list_store_set(GTK_LIST_STORE(user_data),&iter,BUDDYLIST_COL_ON_DIVE,on_dive,-1);
}

static void buddy_clear_list (void)
{
  gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(lookup_widget(main_window,"buddylist")))));
}

void buddy_clear(void)
{
  buddy_clear_list();
  buddy_detail_set_buddy_name("");
  buddy_detail_set_buddy_notes("");
  current_buddy_id=0;
  buddy_set_new_sensitive(FALSE);
  buddy_set_save_sensitive(FALSE);
  buddy_set_delete_sensitive(FALSE);
}

void buddy_init(void)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkWidget *buddy_list;
  GtkListStore *buddy_list_store;

  buddy_list=GTK_WIDGET(lookup_widget(main_window,"buddylist"));
  buddy_list_store=gtk_list_store_new(BUDDYLIST_NUM_COLS,G_TYPE_BOOLEAN,G_TYPE_STRING,G_TYPE_INT);

  renderer=gtk_cell_renderer_toggle_new();
  g_signal_connect(renderer,"toggled",G_CALLBACK(buddy_on_dive_toggled),(gpointer)buddy_list_store);
  column=gtk_tree_view_column_new_with_attributes(_("On Dive?"),renderer,"active",BUDDYLIST_COL_ON_DIVE,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(buddy_list),column);
  gtk_tree_view_column_set_sort_column_id (column, BUDDYLIST_COL_ON_DIVE);

  column=gtk_tree_view_column_new_with_attributes(_("Buddy"),gtk_cell_renderer_text_new(),"text",BUDDYLIST_COL_NAME,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(buddy_list),column);
  gtk_tree_view_column_set_sort_column_id(column,BUDDYLIST_COL_NAME);

  gtk_tree_view_set_model(GTK_TREE_VIEW(buddy_list),GTK_TREE_MODEL(buddy_list_store));
  g_object_unref(GTK_TREE_MODEL(buddy_list_store));
}

static gint buddy_load_list_callback(LoadListData *load_list_data, gint argc,gchar **argv, gchar **azColName)
{
  gint buddy_id;
  GtkTreeIter iter;

  buddy_id=strtol(argv[1],NULL,0);
  gtk_list_store_append(GTK_LIST_STORE(load_list_data->model),&iter);
  gtk_list_store_set(GTK_LIST_STORE(load_list_data->model), &iter,
    BUDDYLIST_COL_ON_DIVE,argv[2]?TRUE:FALSE,
    BUDDYLIST_COL_NAME,argv[0],
    BUDDYLIST_COL_BUDDY_ID,buddy_id,
    -1
  );
  if(buddy_id==load_list_data->select_id) load_list_data->select_path=gtk_tree_model_get_path(load_list_data->model,&iter);
  return 0;
}

void buddy_load_list(gint select_id)
{
  GtkTreePath *path;
  LoadListData load_list_data={0,NULL,NULL};
  GtkWidget *buddy_list;

  buddy_list=GTK_WIDGET(lookup_widget(main_window,"buddylist"));
  load_list_data.model=gtk_tree_view_get_model(GTK_TREE_VIEW(buddy_list));
  buddy_clear_list();
  load_list_data.select_id=select_id;
  buddy_db_buddylist_load_store(current_dive_id,&load_list_data,buddy_load_list_callback);
  if(!select_id) {
    if(helper_model_has_items(load_list_data.model)) load_list_data.select_path=gtk_tree_path_new_first();
  }
  if(load_list_data.select_path) {
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(buddy_list),load_list_data.select_path,NULL,FALSE);
    gtk_tree_path_free(load_list_data.select_path);
  }
  buddy_set_new_sensitive(TRUE);
}

static gint buddy_detail_callback(gpointer dummy,gint argc,gchar **argv,gchar **azColName)
{
  buddy_detail_set_buddy_notes(argv[0]);
  if(argv[1]) {
    if(!preferences.allow_deletes) buddy_set_delete_sensitive(FALSE);    
    else buddy_set_delete_sensitive(TRUE);
    buddy_has_dives=TRUE;
  }
  else {
    buddy_set_delete_sensitive(TRUE);
    buddy_has_dives=FALSE;
  }
  return 0;
}

void buddy_list_cursor_changed(GtkTreeView *buddy_list,gpointer user_data)
{
  gchar *buddy_name;
  GtkTreeIter iter;
  GtkTreeModel *buddy_list_model;

  buddy_list_model=gtk_tree_view_get_model(GTK_TREE_VIEW(buddy_list));
  gtk_tree_selection_get_selected(gtk_tree_view_get_selection(buddy_list),&buddy_list_model,&iter);
  gtk_tree_model_get(buddy_list_model,&iter,
    BUDDYLIST_COL_BUDDY_ID,&current_buddy_id,
    BUDDYLIST_COL_NAME,&buddy_name,
    -1
  );
  buddy_detail_set_buddy_name(buddy_name);
  buddy_db_load_buddy_detail(current_buddy_id,buddy_detail_callback);
  buddy_set_save_sensitive(TRUE);
  g_free(buddy_name);
}

void buddy_new(GtkButton *button,gpointer user_data)
{
  gchar *buddy_notes;
  gint new_buddy_id;

  buddy_notes=buddy_detail_get_buddy_notes();
  new_buddy_id=buddy_db_insert((gchar*)buddy_detail_get_buddy_name(), buddy_notes);
  if(new_buddy_id>0) buddy_load_list(new_buddy_id);
  g_free(buddy_notes);
}

void buddy_save(GtkButton *button,gpointer user_data)
{
  gchar *buddy_notes,*buddy_name;
  GtkTreeIter iter;
  GtkWidget *buddy_list;
  GtkTreeModel *buddy_list_model;

  buddy_name=(gchar*)buddy_detail_get_buddy_name();
  buddy_notes=buddy_detail_get_buddy_notes();
  if(buddy_db_update(current_buddy_id,buddy_name,buddy_notes)) {
    buddy_list=GTK_WIDGET(lookup_widget(main_window,"buddylist"));
    buddy_list_model=gtk_tree_view_get_model(GTK_TREE_VIEW(buddy_list));
    gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(buddy_list)),&buddy_list_model,&iter);
    gtk_list_store_set(GTK_LIST_STORE(buddy_list_model),&iter,BUDDYLIST_COL_NAME,buddy_name,-1);
  }
  g_free(buddy_notes);
}

void buddy_delete(GtkButton *button,gpointer user_data)
{
  gchar *message,*buddy_name;
  gboolean delete=TRUE;
  
  if(preferences.allow_deletes && buddy_has_dives) {
    buddy_name=(gchar*)buddy_detail_get_buddy_name();
    message=g_strdup_printf (_("Are you sure you want to delete '%s'?\n\n'%s' has been a buddy on some dives."),buddy_name,buddy_name);
    delete=prompt_message_question(message);
  }
  if(delete) {
    if(buddy_db_delete(current_buddy_id)) buddy_load_list(0);
  }
}
