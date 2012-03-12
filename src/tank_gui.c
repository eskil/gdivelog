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
#include "tank_db.h"
#include "sort_funcs.h"
#include "support.h"

enum
{
  TANKLIST_COL_NAME = 0,
  TANKLIST_COL_VOLUME,
  TANKLIST_COL_WP,
  TANKLIST_COL_TANK_ID,
  TANKLIST_NUM_COLS
};

static gint current_tank_id = 0;
static gboolean tank_has_dives=FALSE;
static GtkWidget *tank_window = NULL;

static const gchar *tank_detail_get_tank_name(void) 
{
   GtkWidget *widget;
  
   widget=GTK_WIDGET(lookup_widget(tank_window,"tank_name"));
   return gtk_entry_get_text(GTK_ENTRY(widget));
}

static void tank_detail_set_tank_name(gchar *tank_name) 
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(tank_window,"tank_name"));
  gtk_entry_set_text(GTK_ENTRY(widget),tank_name);
}

#define tank_detail_set_tank_name_text(x) tank_detail_set_tank_name(x) 

static gdouble tank_detail_get_tank_volume(void) 
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(tank_window,"tank_volume"));
  return format_field_get_numeric_value_with_null((gchar*)gtk_entry_get_text(GTK_ENTRY(widget)),NULL_WP,FALSE);
}

static void tank_detail_set_tank_volume_text(gchar *volumestr)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(tank_window,"tank_volume"));
  gtk_entry_set_text(GTK_ENTRY(widget),volumestr);
}

static void tank_detail_set_tank_volume(gdouble volume)
{
  gchar *volumestr;

  volumestr=format_field_volume(volume);
  tank_detail_set_tank_volume_text(volumestr);
  g_free(volumestr);
}

static gdouble tank_detail_get_tank_wp(void) 
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(tank_window,"tank_wp"));
  return format_field_get_numeric_value_with_null((gchar*)gtk_entry_get_text(GTK_ENTRY(widget)),NULL_VOLUME,FALSE);
}

static void tank_detail_set_tank_wp_text(gchar *wpstr)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(tank_window,"tank_wp"));
  gtk_entry_set_text(GTK_ENTRY(widget),wpstr);
}

static void tank_detail_set_tank_wp(gdouble wp)
{
  gchar *wpstr;

  wpstr=format_field_pressure(wp);
  tank_detail_set_tank_wp_text(wpstr);
  g_free(wpstr);
}

static gchar *tank_detail_get_notes(void) 
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(tank_window,"tank_notes"));
  return helper_gtk_text_view_get_text(GTK_TEXT_VIEW(widget));
}

static void tank_detail_set_notes(gchar *tank_notes) 
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(tank_window,"tank_notes"));
  helper_gtk_text_view_set_text(GTK_TEXT_VIEW(widget),tank_notes);
}

#define tank_detail_set_tank_notes_text(x) tank_detail_set_tank_notes(x) 

gboolean tank_detail_volume_validate(GtkWidget *widget,GdkEventFocus *event,gpointer user_data)
{
  tank_detail_set_tank_volume(tank_detail_get_tank_volume());
  return FALSE;
}

gboolean tank_detail_wp_validate(GtkWidget *widget,GdkEventFocus *event,gpointer user_data)
{
  tank_detail_set_tank_wp(tank_detail_get_tank_wp());
  return FALSE;
}

static void tank_set_save_sensitive(gboolean sensitive)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(tank_window,"tank_save_btn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static void tank_set_delete_sensitive(gboolean sensitive)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(tank_window,"tank_delete_btn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static void tank_set_select_sensitive(gboolean sensitive)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(tank_window,"tank_select_btn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static tank_clear_list(void)
{
  gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(lookup_widget(tank_window,"tanklist")))));
}

static gint tank_load_list_callback(LoadListData *load_list_data,gint argc,gchar **argv,gchar **azColName)
{
  gint tank_id;
  gdouble dvolume, dwp;
  gchar *volumestr, *wpstr;
  GtkTreeIter iter;

  tank_id=strtol(argv[3],NULL,0);
  dvolume=argv[1]?g_strtod(argv[1],NULL):NULL_VOLUME;
  dwp=argv[2]?g_strtod(argv[2],NULL):NULL_PRESSURE;
  if(preferences.volume_unit!='l' && dvolume!=NULL_VOLUME) dvolume=convert_liters_to_cuft(dvolume *dwp);
  if(preferences.pressure_unit!='b' && dwp!=NULL_PRESSURE) dwp=convert_bar_to_psi(dwp);
  volumestr=format_field_volume(dvolume);
  wpstr=format_field_pressure(dwp);
  gtk_list_store_append(GTK_LIST_STORE(load_list_data->model),&iter);
  gtk_list_store_set(GTK_LIST_STORE(load_list_data->model),&iter,
    TANKLIST_COL_NAME,argv[0],
    TANKLIST_COL_VOLUME,volumestr,
    TANKLIST_COL_WP,wpstr,
    TANKLIST_COL_TANK_ID,tank_id,
    -1
  );
  g_free(volumestr);
  g_free(wpstr);
  if(tank_id==load_list_data->select_id) {
    load_list_data->select_path=gtk_tree_model_get_path(load_list_data->model,&iter);
  }
  return 0;
}

static void tank_load_list(gint select_id)
{
  GtkTreePath *path;
  LoadListData load_list_data={0,NULL,NULL};
  GtkWidget *tank_list;
  GtkTreeModel *tank_list_model;

  tank_clear_list();
  tank_list=GTK_WIDGET(lookup_widget(tank_window,"tanklist"));
  load_list_data.model=gtk_tree_view_get_model(GTK_TREE_VIEW(tank_list));  
  load_list_data.select_id=select_id;
  tank_db_tanklist_load_store(&load_list_data,tank_load_list_callback);

  if(!select_id) {
    if(helper_model_has_items(load_list_data.model)) {
      load_list_data.select_path=gtk_tree_path_new_first();
    }
  }
  if(load_list_data.select_path) {
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(tank_list),load_list_data.select_path,NULL,FALSE);
    gtk_tree_path_free(load_list_data.select_path);
  }
}

void tank_show_window(gint tank_id)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkWidget *tank_list;
  GtkListStore *tank_list_store;

  tank_window=GTK_WIDGET(create_tank_window());
 
  tank_list=GTK_WIDGET(lookup_widget(tank_window, "tanklist"));
  tank_list_store=gtk_list_store_new(TANKLIST_NUM_COLS,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_INT);

  column=gtk_tree_view_column_new_with_attributes(_("Tank"),gtk_cell_renderer_text_new(),"text",TANKLIST_COL_NAME,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tank_list),column);
  gtk_tree_view_column_set_sort_column_id(column,TANKLIST_COL_NAME);

  column=gtk_tree_view_column_new_with_attributes(_("Volume"),gtk_cell_renderer_text_new(),"text",TANKLIST_COL_VOLUME,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tank_list),column);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tank_list_store),TANKLIST_COL_VOLUME,sort_func_double,(gpointer)TANKLIST_COL_VOLUME,NULL);
  gtk_tree_view_column_set_sort_column_id (column, TANKLIST_COL_VOLUME);

  column=gtk_tree_view_column_new_with_attributes(_("WP"),gtk_cell_renderer_text_new(),"text",TANKLIST_COL_WP,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tank_list),column);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tank_list_store),TANKLIST_COL_WP,sort_func_double,(gpointer)TANKLIST_COL_WP,NULL);
  gtk_tree_view_column_set_sort_column_id(column,TANKLIST_COL_WP);

  gtk_tree_view_set_model(GTK_TREE_VIEW(tank_list),GTK_TREE_MODEL(tank_list_store));
  g_object_unref(GTK_TREE_MODEL(tank_list_store));

  tank_set_save_sensitive(FALSE);
  tank_set_delete_sensitive(FALSE);
  tank_set_select_sensitive(FALSE);
  gtk_window_set_transient_for(GTK_WINDOW(tank_window),GTK_WINDOW(main_window));
  gtk_widget_show(tank_window);
  tank_load_list(tank_id);
}

static gint tank_detail_callback(gpointer dummy,gint argc,gchar **argv,gchar **azColName)
{
  tank_detail_set_notes(argv[0]);
  if(argv[1]) {
    if(!preferences.allow_deletes) tank_set_delete_sensitive(FALSE);
    else tank_set_delete_sensitive(TRUE);
    tank_has_dives=TRUE;
  }
  else {
    tank_set_delete_sensitive(TRUE);
    tank_has_dives=FALSE;
  } 
  return 0;
}

void tank_list_cursor_changed(GtkTreeView *tank_list,gpointer user_data)
{
  gchar *name,*volume,*wp;
  GtkTreeIter iter;
  GtkTreeModel *tank_list_model;
  
  tank_list_model=gtk_tree_view_get_model(tank_list);
  gtk_tree_selection_get_selected(gtk_tree_view_get_selection(tank_list),&tank_list_model,&iter);
  gtk_tree_model_get(GTK_TREE_MODEL(tank_list_model),&iter,
    TANKLIST_COL_NAME,&name,
    TANKLIST_COL_VOLUME,&volume,
    TANKLIST_COL_WP,&wp,
    TANKLIST_COL_TANK_ID,&current_tank_id,
    -1
  );
  tank_db_load_tank_detail(current_tank_id,tank_detail_callback);  
  tank_detail_set_tank_name_text(name);
  tank_detail_set_tank_volume_text(volume);
  tank_detail_set_tank_wp_text(wp);
  g_free(name);
  g_free(volume);
  g_free(wp);
  tank_set_save_sensitive(TRUE);
  tank_set_select_sensitive(TRUE);
}

void tank_new(GtkButton *button, gpointer user_data)
{
  gchar *tank_notes;
  gdouble tank_volume,tank_wp;
  gint new_tank_id;

  tank_volume=tank_detail_get_tank_volume();
  tank_wp=tank_detail_get_tank_wp();
  tank_notes=tank_detail_get_notes();
  if(preferences.pressure_unit!='b') tank_wp=convert_bar_to_psi(tank_wp);
  if(preferences.volume_unit != 'l') tank_volume=convert_cuft_to_liters(tank_volume)/tank_wp;
  new_tank_id=tank_db_insert(
    (gchar*)tank_detail_get_tank_name(),
    tank_volume,
    tank_wp,
    tank_notes
  );
  g_free(tank_notes);
  if(new_tank_id>0) tank_load_list(new_tank_id);
}

void tank_save(GtkButton *button, gpointer user_data)
{
  gchar *tank_name, *tank_notes, *volumestr, *wpstr;
  gdouble tank_volume, tank_wp;
  GtkWidget *tank_list;
  GtkTreeModel *tank_list_model;
  GtkTreeIter iter;
  
  tank_name=(gchar*)tank_detail_get_tank_name();
  tank_volume=tank_detail_get_tank_volume();
  tank_wp=tank_detail_get_tank_wp();
  tank_notes=tank_detail_get_notes();
  if(preferences.pressure_unit!='b') tank_wp=convert_psi_to_bar(tank_wp);
  if(preferences.volume_unit!='l') tank_volume=convert_cuft_to_liters(tank_volume)/tank_wp;
  if(tank_db_update(current_tank_id,tank_name,tank_volume,tank_wp,tank_notes)) {
    tank_list=GTK_WIDGET(lookup_widget(tank_window,"tanklist"));
    tank_list_model=gtk_tree_view_get_model(GTK_TREE_VIEW(tank_list));
    gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tank_list)),&tank_list_model,&iter);
    if(preferences.volume_unit!='l') tank_volume=convert_liters_to_cuft(tank_volume*tank_wp);
    if(preferences.pressure_unit!='b') tank_wp=convert_bar_to_psi(tank_wp);
    volumestr=format_field_volume(tank_volume);
    wpstr=format_field_pressure(tank_wp);
    gtk_list_store_set(GTK_LIST_STORE(tank_list_model),&iter,
		  TANKLIST_COL_NAME,tank_name,
		  TANKLIST_COL_VOLUME,volumestr,
		  TANKLIST_COL_WP,wpstr,
      -1
    );
    g_free (volumestr);
    g_free (wpstr);
  }
}

void tank_delete(GtkButton *button, gpointer user_data)
{
  gchar *message,*tank_name;
  gboolean delete=TRUE;
  
  if(preferences.allow_deletes && tank_has_dives) {
    tank_name=(gchar*)tank_detail_get_tank_name();
    message=g_strdup_printf(_("Are you sure you want to delete '%s'?\n\n'%s' has been used on some dives."),tank_name,tank_name);
    delete=prompt_message_question(message);
  }
  if(delete) {
    if(tank_db_delete(current_tank_id)) tank_load_list(0);
  }
}

void tank_select(GtkButton *button, gpointer user_data)
{
  dive_tank_tank_set(
    current_tank_id,
    tank_detail_get_tank_name(),
    tank_detail_get_tank_volume(),
    tank_detail_get_tank_wp()
  );
  gtk_widget_destroy(tank_window);
}

void tank_cancel(GtkButton *button, gpointer user_data)
{
  gtk_widget_destroy(tank_window);
}
