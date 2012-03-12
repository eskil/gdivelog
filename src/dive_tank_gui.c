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
#include <time.h>

#include "interface.h"
#include "globals.h"
#include "defines.h"
#include "gdivelog.h"
#include "format_fields.h"
#include "sort_funcs.h"
#include "helper_functions.h"
#include "dive_tank_db.h"
#include "dive_gui.h"
#include "profile_gui.h"

enum
{
  DIVE_TANKLIST_COL_TANK_NAME = 0,
  DIVE_TANKLIST_COL_O2_PER,
  DIVE_TANKLIST_COL_HE_PER,
  DIVE_TANKLIST_COL_PRESSURE_START,
  DIVE_TANKLIST_COL_PRESSURE_END,
  DIVE_TANKLIST_COL_TIME_START,
  DIVE_TANKLIST_COL_TIME_END,
  DIVE_TANKLIST_COL_AVERAGE_DEPTH,
  DIVE_TANKLIST_COL_SAC,
  DIVE_TANKLIST_COL_DIVE_TANK_ID,
  DIVE_TANKLIST_COL_TANK_ID,
  DIVE_TANKLIST_COL_DIVE_ID,
  DIVE_TANKLIST_COL_TANK_VOLUME,
  DIVE_TANKLIST_COL_TANK_WP,
  DIVE_TANKLIST_NUM_COLS
};

static gint current_dive_tank_id = 0;
static gint current_tank_id = 0;
static gdouble current_tank_volume = 0;	/* in liters regardless of user perference */
static gdouble current_tank_wp = 0;	/* in bar regardless of user perference */

static const gchar *dive_tank_detail_get_tank_name(void) 
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_name"));
  return gtk_label_get_text(GTK_LABEL(widget));
}

#define dive_tank_detail_set_tank_name_text(x) dive_tank_detail_set_tank_name(x)

static void dive_tank_detail_set_tank_name(gchar *tank_name)  
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_name"));
  gtk_label_set_text(GTK_LABEL(widget),tank_name);
}

static double dive_tank_detail_get_dive_tank_O2_per(void) 
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_O2_per"));
  return format_field_get_numeric_value_with_null((gchar*)gtk_entry_get_text(GTK_ENTRY(widget)),NULL_PERCENT,FALSE);
}

static void dive_tank_detail_set_dive_tank_O2_per_text(gchar *O2perstr)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_O2_per"));
  gtk_entry_set_text (GTK_ENTRY(widget),O2perstr);
}

static void dive_tank_detail_set_dive_tank_O2_per(gdouble O2per)
{
  gchar *perstr;

  perstr=format_field_percent(O2per);
  dive_tank_detail_set_dive_tank_O2_per_text(perstr);
  g_free(perstr);
}

static gdouble dive_tank_detail_get_dive_tank_He_per(void) 
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_He_per"));
  return format_field_get_numeric_value_with_null((gchar*)gtk_entry_get_text(GTK_ENTRY(widget)),NULL_PERCENT,FALSE);
}

static void dive_tank_detail_set_dive_tank_He_per_text(gchar *Heperstr)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_He_per"));
  gtk_entry_set_text (GTK_ENTRY(widget),Heperstr);
}

static void dive_tank_detail_set_dive_tank_He_per(gdouble Heper)
{
  gchar *perstr;

  perstr=format_field_percent(Heper);
  dive_tank_detail_set_dive_tank_He_per_text(perstr);
  g_free(perstr);
}

static gdouble dive_tank_detail_get_dive_tank_pressure_start(void) 
{ 
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_pressure_start"));
  return format_field_get_numeric_value_with_null((gchar*)gtk_entry_get_text(GTK_ENTRY(widget)),NULL_PRESSURE,FALSE);
}

static void dive_tank_detail_set_dive_tank_pressure_start_text(gchar *pressurestr)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_pressure_start"));
  gtk_entry_set_text(GTK_ENTRY(widget),pressurestr);
}

static void dive_tank_detail_set_dive_tank_pressure_start(gdouble pressure)
{
  gchar *pressurestr;

  pressurestr=format_field_pressure(pressure);
  dive_tank_detail_set_dive_tank_pressure_start_text(pressurestr);
  g_free(pressurestr);
}

static gdouble dive_tank_detail_get_dive_tank_pressure_end(void) 
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_pressure_end"));
  return format_field_get_numeric_value_with_null((gchar*)gtk_entry_get_text(GTK_ENTRY(widget)),NULL_PRESSURE,FALSE);
}

static void dive_tank_detail_set_dive_tank_pressure_end_text(gchar *pressurestr)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_pressure_end"));
  gtk_entry_set_text(GTK_ENTRY(widget), pressurestr);
}

static void dive_tank_detail_set_dive_tank_pressure_end(gdouble pressure)
{
  gchar *pressurestr;

  pressurestr=format_field_pressure(pressure);
  dive_tank_detail_set_dive_tank_pressure_end_text(pressurestr);
  g_free(pressurestr);
}

static gdouble dive_tank_detail_get_dive_tank_average_depth(void) 
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_average_depth"));
  return format_field_get_numeric_value_with_null((gchar*)gtk_entry_get_text(GTK_ENTRY(widget)),NULL_DEPTH,FALSE);
}

static void dive_tank_detail_set_dive_tank_average_depth_text(gchar *depthstr)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_average_depth"));
  gtk_entry_set_text(GTK_ENTRY(widget), depthstr);
}

static void dive_tank_detail_set_dive_tank_average_depth(gdouble depth)
{
  gchar *depthstr;

  depthstr = format_field_depth(depth);
  dive_tank_detail_set_dive_tank_average_depth_text(depthstr);
  g_free(depthstr);
}

static glong dive_tank_detail_get_dive_tank_time_start(void) 
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_time_start"));
  return format_field_get_time_value((gchar*)gtk_entry_get_text(GTK_ENTRY(widget)));
}

static void dive_tank_detail_set_dive_tank_time_start_text(gchar *timestr)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_time_start"));
  gtk_entry_set_text(GTK_ENTRY(widget),timestr);
}

void dive_tank_detail_set_dive_tank_time_start(glong time)
{
  gchar *timestr;

  timestr=format_field_time(time);
  dive_tank_detail_set_dive_tank_time_start_text(timestr);
  g_free(timestr);
}

static glong dive_tank_detail_get_dive_tank_time_end(void) 
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_time_end"));
  return format_field_get_time_value((gchar*)gtk_entry_get_text(GTK_ENTRY(widget)));
}

static void dive_tank_detail_set_dive_tank_time_end_text(gchar *timestr)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_time_end"));
  gtk_entry_set_text(GTK_ENTRY(widget),timestr);
}

void dive_tank_detail_set_dive_tank_time_end(glong time)
{
  gchar *timestr;

  timestr=format_field_time(time);
  dive_tank_detail_set_dive_tank_time_end_text(timestr);
  g_free(timestr);
}

static gdouble dive_tank_calc_SAC(gdouble pressure_start,gdouble pressure_end,glong time_start,glong time_end,gdouble average_depth,gdouble tank_volume)
{
  gdouble SAC=-1;

  if(pressure_start>0.0 && pressure_end>0.0 && time_start>=0 && time_end>0 && average_depth>0.0) {
    if(preferences.depth_unit!='m') average_depth=convert_feet_to_ata(average_depth);
    else average_depth=convert_meters_to_ata(average_depth);
    if(preferences.pressure_unit!='b') {
      pressure_start=convert_psi_to_bar(pressure_start);
      pressure_end=convert_psi_to_bar(pressure_end);
    }
    SAC=((pressure_start-pressure_end)*tank_volume/average_depth)/((time_end-time_start)/60);
    if(preferences.volume_unit!='l') SAC=convert_liters_to_cuft(SAC);
  }
  return SAC;
}

static const gchar *dive_tank_detail_get_SAC(void) 
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_SAC"));
  return gtk_label_get_text(GTK_LABEL(widget));
}

void dive_tank_detail_set_SAC(GtkEditable *editable,gpointer user_data)
{
  gdouble SAC, tank_volume;
  gchar *SACstr;
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_SAC"));
  SAC=dive_tank_calc_SAC(
    dive_tank_detail_get_dive_tank_pressure_start(),
    dive_tank_detail_get_dive_tank_pressure_end(),
    dive_tank_detail_get_dive_tank_time_start(),
    dive_tank_detail_get_dive_tank_time_end(),
    dive_tank_detail_get_dive_tank_average_depth(),
    current_tank_volume
  );
  SACstr=format_field_SAC(SAC);
  gtk_label_set_text(GTK_LABEL(widget),SACstr);
  g_free (SACstr);
}

gboolean dive_tank_detail_O2_per_validate(GtkWidget *widget,GdkEventFocus *event,gpointer user_data)
{
  dive_tank_detail_set_dive_tank_O2_per(dive_tank_detail_get_dive_tank_O2_per());
  return FALSE;
}

gboolean dive_tank_detail_He_per_validate(GtkWidget *widget,GdkEventFocus *event,gpointer user_data)
{
  dive_tank_detail_set_dive_tank_He_per (dive_tank_detail_get_dive_tank_He_per());
  return FALSE;
}

gboolean dive_tank_detail_pressure_start_validate(GtkWidget *widget,GdkEventFocus *event,gpointer user_data)
{
  dive_tank_detail_set_dive_tank_pressure_start(dive_tank_detail_get_dive_tank_pressure_start());
  dive_tank_detail_set_SAC(GTK_EDITABLE(widget),user_data);
  return FALSE;
}

gboolean dive_tank_detail_pressure_end_validate(GtkWidget *widget,GdkEventFocus *event,gpointer user_data)
{
  dive_tank_detail_set_dive_tank_pressure_end(dive_tank_detail_get_dive_tank_pressure_end());
  dive_tank_detail_set_SAC(GTK_EDITABLE(widget),user_data);
  return FALSE;
}

gboolean dive_tank_detail_average_depth_validate(GtkWidget *widget,GdkEventFocus *event,gpointer user_data)
{
  dive_tank_detail_set_dive_tank_average_depth(dive_tank_detail_get_dive_tank_average_depth());
  dive_tank_detail_set_SAC(GTK_EDITABLE(widget),user_data);
  return FALSE;
}

static void dive_tank_set_average_depth_calc_sensitive(gboolean sensitive)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"button_average_depth_calc"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static void dive_tank_set_average_depth_calc_btn_sensitivity(void)
{
  gdouble ts, te;

  if(profile_exists()) {
    ts=dive_tank_detail_get_dive_tank_time_start();
    te=dive_tank_detail_get_dive_tank_time_end();
    if((ts>=0)&&(te>0)) dive_tank_set_average_depth_calc_sensitive(TRUE);
    else dive_tank_set_average_depth_calc_sensitive(FALSE);
  }
}

gboolean dive_tank_detail_time_start_validate(GtkWidget *widget,GdkEventFocus *event,gpointer user_data)
{
  dive_tank_detail_set_dive_tank_time_start(dive_tank_detail_get_dive_tank_time_start());
  dive_tank_set_average_depth_calc_btn_sensitivity();
  return FALSE;
}

gboolean dive_tank_detail_time_end_validate(GtkWidget *widget,GdkEventFocus *event,gpointer user_data)
{
  dive_tank_detail_set_dive_tank_time_end(dive_tank_detail_get_dive_tank_time_end());
  dive_tank_set_average_depth_calc_btn_sensitivity();  
  return FALSE;
}

static void dive_tank_set_new_sensitive(gboolean sensitive)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_new_btn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static void dive_tank_set_save_sensitive(gboolean sensitive)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_save_btn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static void dive_tank_set_delete_sensitive(gboolean sensitive)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_delete_btn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

void dive_tank_init(void)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkWidget *dive_tank_list;
  GtkListStore *dive_tank_list_store;
  
  dive_tank_list=GTK_WIDGET(lookup_widget(main_window, "dive_tanklist"));

  dive_tank_list_store=gtk_list_store_new(
    DIVE_TANKLIST_NUM_COLS,
    G_TYPE_STRING,
    G_TYPE_STRING,
    G_TYPE_STRING,
    G_TYPE_STRING,
    G_TYPE_STRING,
    G_TYPE_STRING,
    G_TYPE_STRING,
    G_TYPE_STRING,
    G_TYPE_STRING,
    G_TYPE_INT,
    G_TYPE_INT,
    G_TYPE_INT,
    G_TYPE_DOUBLE,
    G_TYPE_DOUBLE
  );

  column=gtk_tree_view_column_new_with_attributes(_("Tank"),gtk_cell_renderer_text_new(),"text",DIVE_TANKLIST_COL_TANK_NAME,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(dive_tank_list),column);
  gtk_tree_view_column_set_sort_column_id(column,DIVE_TANKLIST_COL_TANK_NAME);

  column=gtk_tree_view_column_new_with_attributes(_("O2%"),gtk_cell_renderer_text_new(),"text",DIVE_TANKLIST_COL_O2_PER,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(dive_tank_list),column);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(dive_tank_list_store),DIVE_TANKLIST_COL_O2_PER,sort_func_double,(gpointer)DIVE_TANKLIST_COL_O2_PER,NULL);
  gtk_tree_view_column_set_sort_column_id(column,DIVE_TANKLIST_COL_O2_PER);

  column=gtk_tree_view_column_new_with_attributes(_("He%"),gtk_cell_renderer_text_new(),"text",DIVE_TANKLIST_COL_HE_PER,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(dive_tank_list),column);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(dive_tank_list_store),DIVE_TANKLIST_COL_HE_PER,sort_func_double,(gpointer)DIVE_TANKLIST_COL_HE_PER,NULL);
  gtk_tree_view_column_set_sort_column_id(column, DIVE_TANKLIST_COL_HE_PER);

  column=gtk_tree_view_column_new_with_attributes(_("Start Pressure"),gtk_cell_renderer_text_new(),"text",DIVE_TANKLIST_COL_PRESSURE_START,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(dive_tank_list),column);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE (dive_tank_list_store),DIVE_TANKLIST_COL_PRESSURE_START,sort_func_double,(gpointer)DIVE_TANKLIST_COL_PRESSURE_START,NULL);
  gtk_tree_view_column_set_sort_column_id(column,DIVE_TANKLIST_COL_PRESSURE_START);

  column=gtk_tree_view_column_new_with_attributes(_("End Pressure"),gtk_cell_renderer_text_new(),"text",DIVE_TANKLIST_COL_PRESSURE_END,NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW(dive_tank_list),column);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(dive_tank_list_store),DIVE_TANKLIST_COL_PRESSURE_END,sort_func_double,(gpointer)DIVE_TANKLIST_COL_PRESSURE_END,NULL);
  gtk_tree_view_column_set_sort_column_id(column,DIVE_TANKLIST_COL_PRESSURE_END);

  column=gtk_tree_view_column_new_with_attributes(_("Start Time"),gtk_cell_renderer_text_new(),"text",DIVE_TANKLIST_COL_TIME_START,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(dive_tank_list),column);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(dive_tank_list_store),DIVE_TANKLIST_COL_TIME_START,sort_func_time,(gpointer)DIVE_TANKLIST_COL_TIME_START,NULL);
  gtk_tree_view_column_set_sort_column_id(column,DIVE_TANKLIST_COL_TIME_START);

  column=gtk_tree_view_column_new_with_attributes(_("End Time"),gtk_cell_renderer_text_new(),"text",DIVE_TANKLIST_COL_TIME_END,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(dive_tank_list),column);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(dive_tank_list_store),DIVE_TANKLIST_COL_TIME_END,sort_func_time,(gpointer)DIVE_TANKLIST_COL_TIME_END,NULL);
  gtk_tree_view_column_set_sort_column_id(column,DIVE_TANKLIST_COL_TIME_END);

  column=gtk_tree_view_column_new_with_attributes(_("Avg Depth"),gtk_cell_renderer_text_new(),"text",DIVE_TANKLIST_COL_AVERAGE_DEPTH,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(dive_tank_list),column);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(dive_tank_list_store),DIVE_TANKLIST_COL_AVERAGE_DEPTH,sort_func_double,(gpointer)DIVE_TANKLIST_COL_AVERAGE_DEPTH,NULL);
  gtk_tree_view_column_set_sort_column_id(column,DIVE_TANKLIST_COL_AVERAGE_DEPTH);

  column=gtk_tree_view_column_new_with_attributes(_("SAC"),gtk_cell_renderer_text_new(),"text",DIVE_TANKLIST_COL_SAC,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(dive_tank_list),column);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(dive_tank_list_store),DIVE_TANKLIST_COL_SAC,sort_func_double,(gpointer)DIVE_TANKLIST_COL_SAC,NULL);
  gtk_tree_view_column_set_sort_column_id(column, DIVE_TANKLIST_COL_SAC);

  gtk_tree_view_set_model(GTK_TREE_VIEW(dive_tank_list),GTK_TREE_MODEL(dive_tank_list_store));
  g_object_unref(GTK_TREE_MODEL(dive_tank_list_store));
}

static void dive_tank_clear_list(void)
{
  gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(lookup_widget(main_window, "dive_tanklist")))));
}

void dive_tank_clear(void)
{
  dive_tank_clear_list();
  dive_tank_detail_set_tank_name("");
  dive_tank_detail_set_dive_tank_O2_per(NULL_PERCENT);
  dive_tank_detail_set_dive_tank_He_per(NULL_PERCENT);
  dive_tank_detail_set_dive_tank_pressure_start(NULL_PRESSURE);
  dive_tank_detail_set_dive_tank_pressure_end(NULL_PRESSURE);
  dive_tank_detail_set_dive_tank_time_start(NULL_TIME);
  dive_tank_detail_set_dive_tank_time_end(NULL_TIME);
  dive_tank_detail_set_dive_tank_average_depth(NULL_DEPTH);
  /* REMOVE ? dive_tank_detail_set_SAC(void) - should be handled via events */
  current_tank_id = 0;
  current_dive_tank_id = 0;
  current_tank_volume = 0.0;
  current_tank_wp = 0.0;
  dive_tank_set_new_sensitive(FALSE);
  dive_tank_set_save_sensitive(FALSE);
  dive_tank_set_delete_sensitive(FALSE);
  dive_tank_set_average_depth_calc_sensitive(FALSE);
}

static gint dive_tank_load_list_callback(LoadListData *load_list_data,gint argc, gchar **argv,gchar **azColName)
{
  gint dive_tank_id;
  gdouble tank_volume,tank_wp,SAC,end_pressure,start_pressure,average_depth;
  glong start_time,end_time;
  gchar *tank_name,*O2_perstr,*He_perstr,*start_pressurestr,*end_pressurestr,*start_timestr,*end_timestr,*average_depthstr,*SACstr;
  GtkTreeIter iter;
  tank_volume=argv[11]?g_strtod(argv[11],NULL):NULL_VOLUME;
  tank_wp=argv[12]?g_strtod(argv[12],NULL):NULL_PRESSURE;
  dive_tank_id=strtol(argv[8],NULL,0);
  start_pressure=argv[3]?g_strtod(argv[3],NULL):NULL_PRESSURE;
  end_pressure=argv[4]?g_strtod(argv[4],NULL):NULL_PRESSURE;
  average_depth=argv[7]?g_strtod(argv[7],NULL):NULL_DEPTH;
  start_time=argv[5]?strtol(argv[5],NULL,0):NULL_TIME;
  end_time=argv[6]?strtol(argv[6],NULL,0):NULL_TIME;
  SAC=dive_tank_calc_SAC(start_pressure,end_pressure,start_time,end_time,average_depth,tank_volume);
  if(preferences.depth_unit!='m' && average_depth!=NULL_DEPTH) average_depth=convert_meters_to_feet(average_depth);
  if(preferences.pressure_unit!='b') {
    if(start_pressure!=NULL_PRESSURE) start_pressure=convert_bar_to_psi(start_pressure);
    if(end_pressure!=NULL_PRESSURE) end_pressure=convert_bar_to_psi(end_pressure);
  }
  if(preferences.volume_unit!='l') SAC=convert_liters_to_cuft(SAC);
  O2_perstr=format_field_percent(argv[1]?g_strtod(argv[1],NULL):NULL_PERCENT);
  He_perstr=format_field_percent(argv[2]?g_strtod(argv[2],NULL):NULL_PERCENT);
  start_pressurestr=format_field_pressure(start_pressure);
  end_pressurestr=format_field_pressure(end_pressure);
  start_timestr=format_field_time(start_time);
  end_timestr=format_field_time(end_time);
  average_depthstr=format_field_depth (average_depth);
  SACstr=format_field_SAC(SAC);

  gtk_list_store_append(GTK_LIST_STORE(load_list_data->model),&iter);
  gtk_list_store_set(GTK_LIST_STORE(load_list_data->model),&iter,
    DIVE_TANKLIST_COL_TANK_NAME,argv[0],
    DIVE_TANKLIST_COL_O2_PER,O2_perstr,
    DIVE_TANKLIST_COL_HE_PER,He_perstr,
    DIVE_TANKLIST_COL_PRESSURE_START,start_pressurestr,
    DIVE_TANKLIST_COL_PRESSURE_END,end_pressurestr,
    DIVE_TANKLIST_COL_TIME_START,start_timestr,
    DIVE_TANKLIST_COL_TIME_END,end_timestr,
    DIVE_TANKLIST_COL_AVERAGE_DEPTH,average_depthstr,
    DIVE_TANKLIST_COL_SAC,SACstr,
    DIVE_TANKLIST_COL_DIVE_TANK_ID,dive_tank_id,
    DIVE_TANKLIST_COL_TANK_ID,strtol(argv[9],NULL,0),
    DIVE_TANKLIST_COL_DIVE_ID,strtol(argv[10],NULL,0),
    DIVE_TANKLIST_COL_TANK_VOLUME,tank_volume,
    DIVE_TANKLIST_COL_TANK_WP,tank_wp, 
    -1
  );
  g_free(O2_perstr);
  g_free(He_perstr);
  g_free(start_pressurestr);
  g_free(end_pressurestr);
  g_free(start_timestr);
  g_free(end_timestr);
  g_free(average_depthstr);
  g_free(SACstr);

  if(dive_tank_id==load_list_data->select_id) load_list_data->select_path=gtk_tree_model_get_path(load_list_data->model,&iter);

  return 0;
}

void dive_tank_load_list(gint select_id)
{
  GtkTreePath *path;
  LoadListData load_list_data={0,NULL,NULL};
  GtkWidget *dive_tank_list;
  GtkTreeModel *dive_tank_list_model;

  dive_tank_clear();
  dive_tank_list=GTK_WIDGET(lookup_widget(main_window,"dive_tanklist"));
  load_list_data.model=gtk_tree_view_get_model(GTK_TREE_VIEW(dive_tank_list));
  load_list_data.select_id=select_id;
  dive_tank_db_dive_tanklist_load_store(current_dive_id,&load_list_data,dive_tank_load_list_callback);
  if(!select_id) {
    if(helper_model_has_items(load_list_data.model)) load_list_data.select_path=gtk_tree_path_new_first();	
  }
  if(load_list_data.select_path) {
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(dive_tank_list),load_list_data.select_path,NULL,FALSE);
    gtk_tree_path_free(load_list_data.select_path);
  }
  dive_tank_set_new_sensitive(TRUE);
}

void dive_tank_list_cursor_changed(GtkTreeView *dive_tank_list,gpointer user_data)
{
  gchar *name,*O2,*He,*ps,*pe,*ts,*te,*ad,*SAC;
  GtkTreeIter iter;
  GtkTreeModel *dive_tank_list_model;
  
  dive_tank_list_model=gtk_tree_view_get_model(GTK_TREE_VIEW(dive_tank_list));
  gtk_tree_selection_get_selected(gtk_tree_view_get_selection(dive_tank_list),&dive_tank_list_model,&iter);
  gtk_tree_model_get(GTK_TREE_MODEL(dive_tank_list_model),&iter,
    DIVE_TANKLIST_COL_TANK_NAME,&name,
    DIVE_TANKLIST_COL_O2_PER,&O2,
    DIVE_TANKLIST_COL_HE_PER,&He,
    DIVE_TANKLIST_COL_PRESSURE_START,&ps,
    DIVE_TANKLIST_COL_PRESSURE_END,&pe,
    DIVE_TANKLIST_COL_TIME_START,&ts,
    DIVE_TANKLIST_COL_TIME_END,&te,
    DIVE_TANKLIST_COL_AVERAGE_DEPTH,&ad,
    DIVE_TANKLIST_COL_SAC,&SAC,
    DIVE_TANKLIST_COL_DIVE_TANK_ID,&current_dive_tank_id,
    DIVE_TANKLIST_COL_TANK_ID,&current_tank_id,
    DIVE_TANKLIST_COL_TANK_VOLUME,&current_tank_volume,
    DIVE_TANKLIST_COL_TANK_WP,&current_tank_wp,
    -1
  );
  dive_tank_detail_set_tank_name(name);
  dive_tank_detail_set_dive_tank_O2_per_text(O2);
  dive_tank_detail_set_dive_tank_He_per_text(He);
  dive_tank_detail_set_dive_tank_pressure_start_text(ps);
  dive_tank_detail_set_dive_tank_pressure_end_text(pe);
  dive_tank_detail_set_dive_tank_time_start_text(ts);
  dive_tank_detail_set_dive_tank_time_end_text(te);
  dive_tank_detail_set_dive_tank_average_depth_text(ad);
  
  dive_tank_set_save_sensitive(TRUE);
  dive_tank_set_delete_sensitive(TRUE);
  dive_tank_set_average_depth_calc_btn_sensitivity();

  g_free(name);
  g_free(O2);
  g_free(He);
  g_free(ps);
  g_free(pe);
  g_free(ts);
  g_free(te);
  g_free(ad);
  g_free(SAC);
}

void dive_tank_show_tank_window(GtkButton *button,gpointer user_data)
{
  tank_show_window(current_tank_id);
}

void dive_tank_tank_set(gint tank_id,gchar *tank_name,gdouble tank_volume,gdouble tank_wp)
{
  gchar *tank_full_name, *tank_volumestr, *tank_wpstr;

  current_tank_id=tank_id;
  dive_tank_detail_set_tank_name(tank_name);
  current_tank_wp=tank_wp;
  if(preferences.pressure_unit!='b') current_tank_wp=convert_psi_to_bar(current_tank_wp);
  current_tank_volume=tank_volume;
  if(preferences.volume_unit!='l') current_tank_volume=convert_cuft_to_liters(current_tank_volume)/current_tank_wp;
  dive_tank_detail_set_SAC(NULL,NULL);
  dive_tank_set_new_sensitive(TRUE);
}

void dive_tank_new(GtkButton *button, gpointer user_data)
{
  gint new_dive_tank_id;
  gdouble ad, ps, pe;

  ad=dive_tank_detail_get_dive_tank_average_depth();
  if(preferences.depth_unit!='m') ad=convert_feet_to_meters(ad);
  ps=dive_tank_detail_get_dive_tank_pressure_start();
  pe=dive_tank_detail_get_dive_tank_pressure_end();
  if(preferences.pressure_unit!='b') {
    ps=convert_psi_to_bar(ps);
    pe=convert_psi_to_bar (pe);
  }
  new_dive_tank_id=dive_tank_db_insert(
    current_dive_id,
    current_tank_id,
    ad,
    dive_tank_detail_get_dive_tank_O2_per(),
    dive_tank_detail_get_dive_tank_He_per(),
    dive_tank_detail_get_dive_tank_time_start(),
    dive_tank_detail_get_dive_tank_time_end(),
    ps,
    pe
  );
  if(new_dive_tank_id>0) dive_tank_load_list(new_dive_tank_id);
}

void dive_tank_save(GtkButton *button, gpointer user_data)
{
  gchar *namestr, *adstr, *O2str, *Hestr, *psstr, *pestr, *tsstr, *testr;
  gdouble ad, O2, He, ps, pe, wad, wps, wpe;
  glong ts, te;
  GtkWidget *dive_tank_list;
  GtkTreeModel *dive_tank_list_model;
  GtkTreeIter iter;

  ad=dive_tank_detail_get_dive_tank_average_depth();
  if(preferences.depth_unit != 'm') wad=convert_feet_to_meters (ad);
  else wad=ad;
  O2=dive_tank_detail_get_dive_tank_O2_per();
  He=dive_tank_detail_get_dive_tank_He_per();
  ts=dive_tank_detail_get_dive_tank_time_start();
  te=dive_tank_detail_get_dive_tank_time_end();
  ps=dive_tank_detail_get_dive_tank_pressure_start();
  pe=dive_tank_detail_get_dive_tank_pressure_end();
  if(preferences.pressure_unit != 'b') {
    wps=convert_psi_to_bar(ps);
    wpe=convert_psi_to_bar(pe);
  }
  else {
    wps=ps;
    wpe=pe;
  }
  if(dive_tank_db_update(current_dive_tank_id,current_tank_id,wad,O2,He,ts,te,wps,wpe)) {
    O2str=format_field_percent(O2);
    Hestr=format_field_percent(He);
    psstr=format_field_pressure(ps);
    pestr=format_field_pressure(pe);
    tsstr=format_field_time(ts);
    testr=format_field_time(te);
    adstr=format_field_depth(ad);
    
    dive_tank_list=GTK_WIDGET(lookup_widget(main_window,"dive_tanklist"));
    dive_tank_list_model=gtk_tree_view_get_model(GTK_TREE_VIEW(dive_tank_list));
    gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(dive_tank_list)),&dive_tank_list_model,&iter);
    gtk_list_store_set(GTK_LIST_STORE(dive_tank_list_model),&iter,
      DIVE_TANKLIST_COL_TANK_NAME,dive_tank_detail_get_tank_name(),
      DIVE_TANKLIST_COL_O2_PER,O2str,
      DIVE_TANKLIST_COL_HE_PER,Hestr,
      DIVE_TANKLIST_COL_PRESSURE_START,psstr,
      DIVE_TANKLIST_COL_PRESSURE_END,pestr,
      DIVE_TANKLIST_COL_TIME_START,tsstr,
      DIVE_TANKLIST_COL_TIME_END,testr,
      DIVE_TANKLIST_COL_AVERAGE_DEPTH,adstr,
      DIVE_TANKLIST_COL_SAC,dive_tank_detail_get_SAC(),
      DIVE_TANKLIST_COL_TANK_ID,current_tank_id,
      DIVE_TANKLIST_COL_TANK_VOLUME,current_tank_volume,
      DIVE_TANKLIST_COL_TANK_WP,current_tank_wp, 
      -1
    );
    g_free(adstr);
    g_free(O2str);
    g_free(Hestr);
    g_free(psstr);
    g_free(pestr);
    g_free(tsstr);
    g_free(testr);
  }
}

void dive_tank_delete(GtkButton *button, gpointer user_data)
{
  if(dive_tank_db_delete(current_dive_tank_id)) dive_tank_load_list(0);
}

void dive_tank_set_time_to_all(GtkButton * button, gpointer user_data)
{
  glong te=0;
  dive_tank_detail_set_dive_tank_time_start(0);
  if(profile_exists()) {
    te=profile_get_max_time();
    dive_tank_set_average_depth_calc_sensitive(TRUE);
  }
  else te=dive_get_duration();
  dive_tank_detail_set_dive_tank_time_end(te);
}

void dive_tank_get_segment_average_depth_from_profile(GtkButton *button, gpointer user_data)
{
  dive_tank_detail_set_dive_tank_average_depth(profile_get_segment_average_depth(dive_tank_detail_get_dive_tank_time_start(),dive_tank_detail_get_dive_tank_time_end()));
}
