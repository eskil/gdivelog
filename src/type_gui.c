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
#include "type_db.h"
#include "support.h"

enum
{
  TYPELIST_COL_ON_DIVE = 0,
  TYPELIST_COL_NAME,
  TYPELIST_COL_TYPE_ID,
  TYPELIST_NUM_COLS
};

static gint current_type_id = 0;
static gboolean type_has_dives=FALSE;

static void type_set_new_sensitive(gboolean sensitive)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"type_new_btn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static void type_set_save_sensitive(gboolean sensitive)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"type_save_btn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static void type_set_delete_sensitive(gboolean sensitive)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"type_delete_btn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static const gchar *type_detail_get_type_name(void)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"type_name"));
  return gtk_entry_get_text(GTK_ENTRY(widget));
}

static void type_detail_set_type_name(gchar *type_name)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"type_name"));
  gtk_entry_set_text(GTK_ENTRY(widget),type_name);
}

static gchar *type_detail_get_type_notes(void)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"type_notes"));
  return helper_gtk_text_view_get_text(GTK_TEXT_VIEW(widget));
}

static void type_detail_set_type_notes(gchar *type_notes)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"type_notes"));
  helper_gtk_text_view_set_text(GTK_TEXT_VIEW(widget),type_notes); 
}

static type_on_dive_toggled(GtkCellRendererToggle *cell,gchar *path_str,gpointer user_data)
{
  GtkTreePath *path;
  GtkTreeIter iter;
  gboolean on_dive;
  gint type_id;

  path=gtk_tree_path_new_from_string(path_str);
  gtk_tree_model_get_iter(GTK_TREE_MODEL(user_data),&iter,path);
  gtk_tree_model_get(GTK_TREE_MODEL(user_data),&iter,
    TYPELIST_COL_ON_DIVE,&on_dive,
    TYPELIST_COL_TYPE_ID,&type_id,
    -1
  );
  if(on_dive) type_db_remove_type_from_dive(current_dive_id,type_id);
  else type_db_add_type_to_dive(current_dive_id, type_id);
  on_dive^=1;
  gtk_list_store_set(GTK_LIST_STORE(user_data),&iter,TYPELIST_COL_ON_DIVE,on_dive,-1);
}

static void type_clear_list (void)
{
  gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(lookup_widget(main_window,"typelist")))));
}

void type_clear(void)
{
  type_clear_list();
  type_detail_set_type_name("");
  type_detail_set_type_notes("");
  current_type_id=0;
  type_set_new_sensitive(FALSE);
  type_set_save_sensitive(FALSE);
  type_set_delete_sensitive(FALSE);
}

void type_init(void)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkWidget *type_list;
  GtkListStore *type_list_store;

  type_list=GTK_WIDGET(lookup_widget(main_window,"typelist"));
  type_list_store=gtk_list_store_new(TYPELIST_NUM_COLS,G_TYPE_BOOLEAN,G_TYPE_STRING,G_TYPE_INT);

  renderer=gtk_cell_renderer_toggle_new();
  g_signal_connect(renderer,"toggled",G_CALLBACK(type_on_dive_toggled),(gpointer)type_list_store);
  column=gtk_tree_view_column_new_with_attributes(_("On Dive?"),renderer,"active",TYPELIST_COL_ON_DIVE,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(type_list),column);
  gtk_tree_view_column_set_sort_column_id (column, TYPELIST_COL_ON_DIVE);

  
column=gtk_tree_view_column_new_with_attributes(_("Type"),gtk_cell_renderer_text_new(),"text",TYPELIST_COL_NAME,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(type_list),column);
  gtk_tree_view_column_set_sort_column_id(column,TYPELIST_COL_NAME);

  gtk_tree_view_set_model(GTK_TREE_VIEW(type_list),GTK_TREE_MODEL(type_list_store));
  g_object_unref(GTK_TREE_MODEL(type_list_store));
}

static gint type_load_list_callback(LoadListData *load_list_data,gint argc,gchar **argv,gchar **azColName)
{
  gint type_id;
  GtkTreeIter iter;

  type_id=strtol(argv[1],NULL,0);
  gtk_list_store_append(GTK_LIST_STORE(load_list_data->model),&iter);
  gtk_list_store_set(GTK_LIST_STORE(load_list_data->model), &iter,
    TYPELIST_COL_ON_DIVE,argv[2]?TRUE:FALSE,
    TYPELIST_COL_NAME,argv[0],
    TYPELIST_COL_TYPE_ID,type_id,
    -1
  );
  if(type_id==load_list_data->select_id) load_list_data->select_path=gtk_tree_model_get_path(load_list_data->model,&iter);
  return 0;
}

void type_load_list(gint select_id)
{
  GtkTreePath *path;
  LoadListData load_list_data={0,NULL,NULL};
  GtkWidget *type_list;

  type_list=GTK_WIDGET(lookup_widget(main_window,"typelist"));
  load_list_data.model=gtk_tree_view_get_model(GTK_TREE_VIEW(type_list));
  type_clear_list();
  load_list_data.select_id=select_id;
  type_db_typelist_load_store(current_dive_id,&load_list_data,type_load_list_callback);
  if(!select_id) {
    if(helper_model_has_items(load_list_data.model)) load_list_data.select_path=gtk_tree_path_new_first();
  }
  if(load_list_data.select_path) {
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(type_list),load_list_data.select_path,NULL,FALSE);
    gtk_tree_path_free(load_list_data.select_path);
  }
  type_set_new_sensitive(TRUE);
}

static gint type_detail_callback(gpointer dummy, gint argc, gchar **argv,gchar **azColName)
{
  type_detail_set_type_notes(argv[0]);
  if(argv[1]) {
    if(!preferences.allow_deletes) type_set_delete_sensitive(FALSE);    
    else type_set_delete_sensitive(TRUE);
    type_has_dives=TRUE;
  }
  else {
    type_set_delete_sensitive(TRUE);
    type_has_dives=FALSE;
  }
  return 0;
}

void type_list_cursor_changed(GtkTreeView *type_list,gpointer user_data)
{
  gchar *type_name;
  GtkTreeIter iter;
  GtkTreeModel *type_list_model;

  type_list_model=gtk_tree_view_get_model(GTK_TREE_VIEW(type_list));
  gtk_tree_selection_get_selected(gtk_tree_view_get_selection(type_list),&type_list_model,&iter);
  gtk_tree_model_get(type_list_model,&iter,
    TYPELIST_COL_TYPE_ID,&current_type_id,
    TYPELIST_COL_NAME,&type_name,
    -1
  );
  type_detail_set_type_name(type_name);
  type_db_load_type_detail(current_type_id,type_detail_callback);
  type_set_save_sensitive(TRUE);
  g_free(type_name);
}

void type_new(GtkButton *button,gpointer user_data)
{
  gchar *type_notes;
  gint new_type_id;

  type_notes=type_detail_get_type_notes();
  new_type_id=type_db_insert((gchar*)type_detail_get_type_name(), type_notes);
  if(new_type_id>0) type_load_list(new_type_id);
  g_free(type_notes);
}

void type_save(GtkButton *button,gpointer user_data)
{
  gchar *type_notes,*type_name;
  GtkTreeIter iter;
  GtkWidget *type_list;
  GtkTreeModel *type_list_model;

  type_name=(gchar*)type_detail_get_type_name();
  type_notes=type_detail_get_type_notes();
  if(type_db_update(current_type_id,type_name,type_notes)) {
    type_list=GTK_WIDGET(lookup_widget(main_window,"typelist"));
    type_list_model=gtk_tree_view_get_model(GTK_TREE_VIEW(type_list));
    gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(type_list)),&type_list_model,&iter);
    gtk_list_store_set(GTK_LIST_STORE(type_list_model),&iter,TYPELIST_COL_NAME,type_name,-1);
  }
  g_free(type_notes);
}

void type_delete(GtkButton *button,gpointer user_data)
{
  gchar *message,*type_name;
  gboolean delete=TRUE;
  
  if(preferences.allow_deletes && type_has_dives) {
    type_name=(gchar*)type_detail_get_type_name();
    message=g_strdup_printf (_("Are you sure you want to delete '%s'?\n\n'%s' has been a type on some dives."),type_name,type_name);
    delete=prompt_message_question(message);
  }
  if(delete) {
    if(type_db_delete(current_type_id)) type_load_list(0);
  }
}
