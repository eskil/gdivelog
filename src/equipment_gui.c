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
#include "equipment_db.h"

enum
{
  EQUIPMENTLIST_COL_ON_DIVE = 0,
  EQUIPMENTLIST_COL_NAME,
  EQUIPMENTLIST_COL_EQUIPMENT_ID,
  EQUIPMENTLIST_NUM_COLS
};

static gint current_equipment_id = 0;
static gboolean equipment_has_dives=FALSE;

static void equipment_set_new_sensitive(gboolean sensitive)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"equipment_new_btn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static void equipment_set_save_sensitive(gboolean sensitive)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"equipment_save_btn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static void equipment_set_delete_sensitive(gboolean sensitive)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"equipment_delete_btn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static const gchar *equipment_detail_get_equipment_name(void)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"equipment_name"));
  return gtk_entry_get_text(GTK_ENTRY(widget));
}

static void equipment_detail_set_equipment_name(gchar *equipment_name)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"equipment_name"));
  gtk_entry_set_text(GTK_ENTRY(widget),equipment_name);
}

static gchar *equipment_detail_get_equipment_notes(void)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"equipment_notes"));
  return helper_gtk_text_view_get_text(GTK_TEXT_VIEW(widget));
}

static void equipment_detail_set_equipment_notes(gchar *equipment_notes)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"equipment_notes"));
  helper_gtk_text_view_set_text(GTK_TEXT_VIEW(widget),equipment_notes); 
}

static equipment_on_dive_toggled(GtkCellRendererToggle *cell,gchar *path_str,gpointer user_data)
{
  GtkTreePath *path;
  GtkTreeIter iter;
  gboolean on_dive;
  gint equipment_id;

  path=gtk_tree_path_new_from_string(path_str);
  gtk_tree_model_get_iter(GTK_TREE_MODEL(user_data),&iter,path);
  gtk_tree_model_get(GTK_TREE_MODEL(user_data),&iter,
    EQUIPMENTLIST_COL_ON_DIVE,&on_dive,
    EQUIPMENTLIST_COL_EQUIPMENT_ID,&equipment_id,
    -1
  );
  if(on_dive) equipment_db_remove_equipment_from_dive(current_dive_id,equipment_id);
  else equipment_db_add_equipment_to_dive(current_dive_id, equipment_id);
  on_dive^=1;
  gtk_list_store_set(GTK_LIST_STORE(user_data),&iter,EQUIPMENTLIST_COL_ON_DIVE,on_dive,-1);
}

static void equipment_clear_list(void)
{
  gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(lookup_widget(main_window,"equipmentlist")))));
}

void equipment_clear(void)
{
  equipment_clear_list();
  equipment_detail_set_equipment_name("");
  equipment_detail_set_equipment_notes("");
  current_equipment_id=0;
  equipment_set_new_sensitive(FALSE);
  equipment_set_save_sensitive(FALSE);
  equipment_set_delete_sensitive(FALSE);
}

void equipment_init(void)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkWidget *equipment_list;
  GtkListStore *equipment_list_store;

  equipment_list=GTK_WIDGET(lookup_widget(main_window,"equipmentlist"));
  equipment_list_store=gtk_list_store_new(EQUIPMENTLIST_NUM_COLS,G_TYPE_BOOLEAN,G_TYPE_STRING,G_TYPE_INT);

  renderer=gtk_cell_renderer_toggle_new();
  g_signal_connect(renderer,"toggled",G_CALLBACK(equipment_on_dive_toggled),(gpointer)equipment_list_store);
  column=gtk_tree_view_column_new_with_attributes(_("On Dive?"),renderer,"active",EQUIPMENTLIST_COL_ON_DIVE,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(equipment_list),column);
  gtk_tree_view_column_set_sort_column_id (column, EQUIPMENTLIST_COL_ON_DIVE);

  
column=gtk_tree_view_column_new_with_attributes(_("Equipment"),gtk_cell_renderer_text_new(),"text",EQUIPMENTLIST_COL_NAME,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(equipment_list),column);
  gtk_tree_view_column_set_sort_column_id(column,EQUIPMENTLIST_COL_NAME);

  gtk_tree_view_set_model(GTK_TREE_VIEW(equipment_list),GTK_TREE_MODEL(equipment_list_store));
  g_object_unref(GTK_TREE_MODEL(equipment_list_store));
}

static gint equipment_load_list_callback (LoadListData *load_list_data,gint argc, gchar **argv,gchar **azColName)
{
  gint equipment_id;
  GtkTreeIter iter;

  equipment_id=strtol(argv[1],NULL,0);
  gtk_list_store_append(GTK_LIST_STORE(load_list_data->model),&iter);
  gtk_list_store_set(GTK_LIST_STORE(load_list_data->model), &iter,
    EQUIPMENTLIST_COL_ON_DIVE,argv[2]?TRUE:FALSE,
    EQUIPMENTLIST_COL_NAME,argv[0],
    EQUIPMENTLIST_COL_EQUIPMENT_ID,equipment_id,
    -1
  );
  if(equipment_id==load_list_data->select_id) load_list_data->select_path=gtk_tree_model_get_path(load_list_data->model,&iter);
  return 0;  
}

void
equipment_load_list (gint select_id)
{ 
  GtkTreePath *path;
  LoadListData load_list_data={0,NULL,NULL};
  GtkWidget *equipment_list;

  equipment_list=GTK_WIDGET(lookup_widget(main_window,"equipmentlist"));
  load_list_data.model=gtk_tree_view_get_model(GTK_TREE_VIEW(equipment_list));
  equipment_clear_list();
  load_list_data.select_id=select_id;
  equipment_db_equipmentlist_load_store(current_dive_id,&load_list_data,equipment_load_list_callback);
  if(!select_id) {
    if(helper_model_has_items(load_list_data.model)) load_list_data.select_path=gtk_tree_path_new_first();
  }
  if(load_list_data.select_path) {
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(equipment_list),load_list_data.select_path,NULL,FALSE);
    gtk_tree_path_free(load_list_data.select_path);
  }
  equipment_set_new_sensitive(TRUE);
}

static gint equipment_detail_callback(gpointer dummy,gint argc,gchar ** argv,gchar ** azColName)
{
  equipment_detail_set_equipment_notes(argv[0]);
  if(argv[1]) {
    if(!preferences.allow_deletes) equipment_set_delete_sensitive(FALSE);    
    else equipment_set_delete_sensitive(TRUE);
    equipment_has_dives=TRUE;
  }
  else {
    equipment_set_delete_sensitive(TRUE);
    equipment_has_dives=FALSE;
  }
  return 0;
}

void equipment_list_cursor_changed (GtkTreeView *equipment_list,gpointer user_data)
{
  gchar *equipment_name;
  GtkTreeIter iter;
  GtkTreeModel *equipment_list_model;

  equipment_list_model=gtk_tree_view_get_model(GTK_TREE_VIEW(equipment_list));
  gtk_tree_selection_get_selected(gtk_tree_view_get_selection(equipment_list),&equipment_list_model,&iter);
  gtk_tree_model_get(equipment_list_model,&iter,
    EQUIPMENTLIST_COL_EQUIPMENT_ID,&current_equipment_id,
    EQUIPMENTLIST_COL_NAME,&equipment_name,
    -1
  );
  equipment_detail_set_equipment_name(equipment_name);
  equipment_db_load_equipment_detail(current_equipment_id,equipment_detail_callback);
  equipment_set_save_sensitive(TRUE);
  g_free(equipment_name);
}

void equipment_new(GtkButton *button,gpointer user_data)
{
  gchar *equipment_notes;
  gint new_equipment_id;

  equipment_notes=equipment_detail_get_equipment_notes();
  new_equipment_id=equipment_db_insert((gchar*)equipment_detail_get_equipment_name(), equipment_notes);
  if(new_equipment_id>0) equipment_load_list(new_equipment_id);
  g_free(equipment_notes);
}

void equipment_save(GtkButton *button,gpointer user_data)
{

  gchar *equipment_name, *equipment_notes;
  GtkTreeIter iter;
  GtkWidget *equipment_list;
  GtkTreeModel *equipment_list_model;  
  
  equipment_name=(gchar*)equipment_detail_get_equipment_name();
  equipment_notes=equipment_detail_get_equipment_notes();
  if(equipment_db_update(current_equipment_id,equipment_name,equipment_notes)) {
    equipment_list=GTK_WIDGET(lookup_widget(main_window,"equipmentlist"));
    equipment_list_model=gtk_tree_view_get_model(GTK_TREE_VIEW(equipment_list));
    gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(equipment_list)),&equipment_list_model,&iter);
    gtk_list_store_set(GTK_LIST_STORE(equipment_list_model),&iter,EQUIPMENTLIST_COL_NAME,equipment_name,-1);
  }
  g_free(equipment_notes);
}

void equipment_delete(GtkButton *button,gpointer user_data)
{
  gchar *message,*equipment_name;
  gboolean delete=TRUE;
  
  if(preferences.allow_deletes && equipment_has_dives) {
    equipment_name=(gchar*)equipment_detail_get_equipment_name();
    message=g_strdup_printf (_("Are you sure you want to delete '%s'?\n\n'%s' has been a equipment on some dives."),equipment_name,equipment_name);
    delete=prompt_message_question(message);
  }
  if(delete) {
    if(equipment_db_delete(current_equipment_id)) equipment_load_list(0);
  }
}
