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
#include <libgnomeui/libgnomeui.h>
#include <time.h>

#include "interface.h"
#include "globals.h"
#include "defines.h"
#include "gdivelog.h"
#include "format_fields.h"
#include "helper_functions.h"
#include "buddy_gui.h"
#include "dive_db.h"
#include "site_gui.h"
#include "profile_gui.h"
#include "sort_funcs.h"
#include "support.h"

enum
{
  DIVELIST_COL_DIVENUMBER = 0,
  DIVELIST_COL_DATE,
  DIVELIST_COL_TIME,
  DIVELIST_COL_SITE,
  DIVELIST_COL_MAXDEPTH,
  DIVELIST_COL_DURATION,
  DIVELIST_COL_ID,
  DIVELIST_NUM_COLS
};

static gint current_dive_number;
static gint current_site_id;
static glong current_template_dive_number;

static time_t dive_parse_date_time(gchar *datetimestr)
{
  struct tm t;

  strptime(datetimestr,DATETIME_STR_FMT,&t);
  return(mktime(&t));
}

gchar *dive_make_date_time_string(time_t datetime)
{
  struct tm *t;
  gchar buf[50];

  t = localtime (&datetime);
  strftime (buf, 50, DATETIME_STR_FMT, t);
  return g_strdup (buf);
}

static void dive_set_date_and_time_strs(gchar *datetimestr, gchar **datestr,gchar **timestr)
{
  struct tm t;
  gchar buf[50];

  strptime (datetimestr, DATETIME_STR_FMT, &t);
  strftime (buf, 50, "%x", &t);
  *datestr = g_strdup (buf);
  strftime (buf, 50, "%X", &t);
  *timestr = g_strdup (buf);
}

static time_t dive_detail_get_dive_date(void) 
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_date"));
  return gnome_date_edit_get_time(GNOME_DATE_EDIT(widget));
}

static void dive_detail_set_dive_date(time_t date) 
{
  GtkWidget *widget;
 
   widget=GTK_WIDGET(lookup_widget(main_window,"dive_date"));
   gnome_date_edit_set_time(GNOME_DATE_EDIT(widget),date);
}

static glong dive_detail_get_dive_time(void) 
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_time"));
  return format_field_get_time_value((gchar*)gtk_entry_get_text(GTK_ENTRY(widget)));
}

static void dive_detail_set_dive_time_text(gchar *timestr)
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_time"));
  gtk_entry_set_text(GTK_ENTRY(widget),timestr);
}

static void dive_detail_set_dive_time(glong time)
{
  gchar *timestr;

  timestr=format_field_duration(time);
  dive_detail_set_dive_time_text(timestr);
  g_free (timestr);
}

static gdouble dive_detail_get_dive_max_depth(void) 
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_max_depth"));
  return format_field_get_numeric_value_with_null((gchar*)gtk_entry_get_text(GTK_ENTRY(widget)),NULL_DEPTH,FALSE);
}

void dive_detail_set_dive_max_depth_text(gchar *max_depthstr)
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_max_depth"));
  gtk_entry_set_text(GTK_ENTRY(widget),max_depthstr);
}

void dive_detail_set_dive_max_depth(gdouble max_depth)
{
  gchar *max_depthstr;

  max_depthstr=format_field_depth(max_depth);
  dive_detail_set_dive_max_depth_text(max_depthstr);
  g_free(max_depthstr);
}
  
static gdouble dive_detail_get_dive_visibility(void) 
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_visibility"));
  return format_field_get_numeric_value_with_null((gchar*)gtk_entry_get_text(GTK_ENTRY(widget)),NULL_VISIBILITY,FALSE);
}

static void dive_detail_set_dive_visibility_text(gchar *visibilitystr)
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_visibility"));
  gtk_entry_set_text(GTK_ENTRY(widget),visibilitystr);
}

static void dive_detail_set_dive_visibility(gdouble visibility)
{
  gchar *visibilitystr;

  visibilitystr=format_field_visibility(visibility);
  dive_detail_set_dive_visibility_text(visibilitystr);
  g_free(visibilitystr);
}

static gdouble dive_detail_get_dive_min_temp(void) 
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_min_temp"));
  return format_field_get_numeric_value_with_null((gchar*)gtk_entry_get_text(GTK_ENTRY(widget)),NULL_TEMPERATURE,TRUE);
}

void dive_detail_set_dive_min_temp_text(gchar *min_tempstr)
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_min_temp"));
  gtk_entry_set_text(GTK_ENTRY(widget),min_tempstr);
}

void dive_detail_set_dive_min_temp(gdouble min_temp)
{
  gchar *min_tempstr;

  min_tempstr=format_field_temperature(min_temp);
  dive_detail_set_dive_min_temp_text(min_tempstr);
  g_free(min_tempstr);
}

static gdouble dive_detail_get_dive_max_temp(void) 
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_max_temp"));
  return format_field_get_numeric_value_with_null((gchar*)gtk_entry_get_text(GTK_ENTRY(widget)),NULL_TEMPERATURE,TRUE);
}

void dive_detail_set_dive_max_temp_text(gchar *max_tempstr)
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_max_temp"));
  gtk_entry_set_text(GTK_ENTRY(widget),max_tempstr);
}

void dive_detail_set_dive_max_temp(gdouble max_temp)
{
  gchar *max_tempstr;

  max_tempstr=format_field_temperature(max_temp);
  dive_detail_set_dive_max_temp_text(max_tempstr);
  g_free(max_tempstr);
}

static gchar *dive_detail_get_dive_notes(void) 
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_notes"));
  return helper_gtk_text_view_get_text(GTK_TEXT_VIEW(widget));
}

static void dive_detail_set_dive_notes(gchar *dive_notes) 
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_notes"));
  helper_gtk_text_view_set_text(GTK_TEXT_VIEW(widget),dive_notes);
}


static gdouble dive_detail_get_dive_weight(void)
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_weight"));
  return format_field_get_numeric_value_with_null((gchar*)gtk_entry_get_text(GTK_ENTRY(widget)),NULL_WEIGHT,TRUE);
}

static void dive_detail_set_dive_weight_text(gchar *weightstr)
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_weight"));
  gtk_entry_set_text(GTK_ENTRY(widget),weightstr);
}

static void dive_detail_set_dive_weight (gdouble weight)
{
  gchar *weightstr;

  weightstr=format_field_weight(weight);
  dive_detail_set_dive_weight_text(weightstr);
  g_free(weightstr);
}

static void dive_detail_set_dive_site(gchar *site_full_name) 
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_site"));
  gtk_label_set_text(GTK_LABEL(widget),site_full_name);
}

static glong dive_detail_get_dive_duration(void) 
{  
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_duration")); 
  return format_field_get_duration_value((gchar*)gtk_entry_get_text(GTK_ENTRY(widget)));
}

void dive_detail_set_dive_duration_text(gchar *durationstr)
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_duration"));
  gtk_entry_set_text(GTK_ENTRY (widget), durationstr);
}

void dive_detail_set_dive_duration(glong duration)
{
  gchar *durationstr;

  durationstr=format_field_duration(duration);
  dive_detail_set_dive_duration_text(durationstr);
  g_free(durationstr);
}

static void dive_detail_set_dive_number(gchar *dive_numberstr) 
{
  GtkWidget *widget;
 
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_number")); 
  gtk_label_set_text(GTK_LABEL(widget),dive_numberstr);
}

void dive_detail_set_dive_SI_text(gchar *SIstr)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"dive_SI"));
  gtk_label_set_text(GTK_LABEL(widget),SIstr);
}


static void dive_set_dive_SI(time_t dive_start,time_t previous_dive_end)
{
  glong SI;
  guchar h, m;
  gchar *SIstr;

  /* A complete and utter mystery, but these lines fix a mysterious bug 
     for no apparent reason and discovered by completely by accident */
  struct tm *t;
  t = localtime (&dive_start);
  /* end mystery bug fix */

  SI=dive_start-previous_dive_end;
  if(SI>=0) {
    if(SI<86401) {
      h=SI/3600;
      m=(SI-(h*3600))/60;
      SIstr=g_strdup_printf("%d:%02d:%02ld",h,m,SI-(h*3600)-(m*60));
    }
    else SIstr = g_strdup ("> 24 hours");
  }
  else SIstr=g_strdup("Error: SI < 0");
  dive_detail_set_dive_SI_text(SIstr);
  g_free(SIstr);
}

static void dive_set_select_site_sensitive(gboolean sensitive)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_site_select_btn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);  
}

static void dive_set_select_tank_sensitive(gboolean sensitive)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_tank_select_tank_btn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);  
}

static void dive_set_merge_next_dive_menuitem_sensitive(gboolean sensitive)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(main_window,"menu_merge_next_dive"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);  
}

static void dive_set_dive_save_sensitive(gboolean sensitive)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(main_window,"menu_save_dive"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);  
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_save_tbbtn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);  
}

static void dive_set_dive_delete_sensitive(gboolean sensitive)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(main_window,"menu_delete_dive"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
  widget=GTK_WIDGET(lookup_widget(main_window,"dive_delete_tbbtn"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

gboolean dive_detail_date_validate(GtkWidget *widget,GtkDirectionType direction,gpointer user_data)
{
  time_t changed_time;

  changed_time=gnome_date_edit_get_time(GNOME_DATE_EDIT(widget));
  if((glong)changed_time<0) dive_detail_set_dive_date(gnome_date_edit_get_initial_time(GNOME_DATE_EDIT(widget)));
  else dive_detail_set_dive_date(changed_time);
  return FALSE;
}

gboolean dive_detail_time_validate(GtkWidget *widget,GdkEventFocus *event, gpointer user_data)
{
  dive_detail_set_dive_time(dive_detail_get_dive_time());
  return FALSE;
}

gboolean dive_detail_duration_validate(GtkWidget *widget,GdkEventFocus *event, gpointer user_data)
{
  dive_detail_set_dive_duration (dive_detail_get_dive_duration ());
   return FALSE;
}

gboolean dive_detail_max_depth_validate(GtkWidget *widget,GdkEventFocus *event, gpointer user_data)
{
  dive_detail_set_dive_max_depth(dive_detail_get_dive_max_depth());
  return FALSE;
}

gboolean dive_detail_max_temp_validate(GtkWidget *widget,GdkEventFocus *event, gpointer user_data)
{
  dive_detail_set_dive_max_temp(dive_detail_get_dive_max_temp());
  return FALSE;
}

gboolean dive_detail_min_temp_validate(GtkWidget *widget,GdkEventFocus *event, gpointer user_data)
{
  dive_detail_set_dive_min_temp(dive_detail_get_dive_min_temp());
  return FALSE;
}

gboolean dive_detail_visibility_validate(GtkWidget *widget,GdkEventFocus *event, gpointer user_data)
{
  dive_detail_set_dive_visibility(dive_detail_get_dive_visibility());
  return FALSE;
}

gboolean dive_detail_weight_validate(GtkWidget *widget,GdkEventFocus *event, gpointer user_data)
{
  dive_detail_set_dive_weight(dive_detail_get_dive_weight());
  return FALSE;
}

static gint dive_list_callback(LoadListData *load_list_data,gint argc,gchar **argv, gchar **azColName)
{
  gchar *duration,*maxdepth,*divetime,*divedate;
  gdouble dbl;
  struct tm t;
  gint dive_id;
  GtkTreeIter iter;

  dive_id=strtol(argv[5],NULL,0);
  dive_set_date_and_time_strs(argv[1],&divedate,&divetime);
  dbl=argv[2]?g_strtod(argv[2],NULL):NULL_DEPTH;
  if(preferences.depth_unit!='m' && dbl!=NULL_DEPTH) dbl=convert_meters_to_feet (dbl);
  maxdepth=format_field_depth(dbl);
  duration=format_field_duration(argv[3]?g_strtod(argv[3],NULL):NULL_DURATION);
  gtk_list_store_append(GTK_LIST_STORE(load_list_data->model),&iter);
  gtk_list_store_set(GTK_LIST_STORE(load_list_data->model),&iter,
    DIVELIST_COL_DIVENUMBER,atoi(argv[0]),
    DIVELIST_COL_DATE,divedate,
    DIVELIST_COL_TIME,divetime,
    DIVELIST_COL_MAXDEPTH,maxdepth,
    DIVELIST_COL_DURATION,duration,
    DIVELIST_COL_SITE,argv[4],
    DIVELIST_COL_ID,dive_id,
    -1
  );
  g_free(duration);
  g_free(maxdepth);
  g_free(divetime);
  g_free(divedate);
  if(dive_id==load_list_data->select_id) load_list_data->select_path=gtk_tree_model_get_path(load_list_data->model,&iter);
  return 0;
}

static gint dive_detail_callback(gpointer dummy,gint argc,gchar **argv,gchar **azColName)
{
  gdouble dbl;
  struct tm t;
  time_t dive_start,previous_dive_end;

  dive_start=dive_parse_date_time(argv[0]);
  dive_detail_set_dive_date(dive_start);
  dbl=argv[1]?g_strtod(argv[1],NULL):NULL_TEMPERATURE;
  if(preferences.temperature_unit!='c' && dbl!=NULL_TEMPERATURE) dbl=convert_celsius_to_farenheit(dbl);
  dive_detail_set_dive_min_temp(dbl);
  dbl=argv[2]?g_strtod(argv[2],NULL):NULL_TEMPERATURE;
  if(preferences.temperature_unit!='c' && dbl!=NULL_TEMPERATURE) dbl=convert_celsius_to_farenheit(dbl);
  dive_detail_set_dive_max_temp(dbl);
  dive_detail_set_dive_notes(argv[3]);
  dbl=argv[4]?g_strtod(argv[4],NULL):NULL_VISIBILITY;
  if(preferences.depth_unit!='m' && dbl!=NULL_VISIBILITY) dbl=convert_meters_to_feet(dbl);
  dive_detail_set_dive_visibility(dbl);
  dbl=argv[5]?g_strtod(argv[5],NULL):NULL_WEIGHT;
  if(preferences.weight_unit!='k' && dbl!=NULL_WEIGHT) dbl=convert_kgs_to_lbs(dbl);
  dive_detail_set_dive_weight(dbl);
  current_site_id=strtol(argv[6],NULL,0);

  if(argv[7]) {
    strptime(argv[7],DATETIME_STR_FMT,&t);
    previous_dive_end=mktime(&t);
    previous_dive_end=previous_dive_end+strtol(argv[8],NULL,0);
  }
  else previous_dive_end=0;
  dive_set_dive_SI(dive_start,previous_dive_end);

  dive_set_merge_next_dive_menuitem_sensitive(FALSE);
  if(argv[9]) {
    if(dive_parse_date_time(argv[9])-dive_start-dive_detail_get_dive_duration()<=preferences.merge_variance)
      dive_set_merge_next_dive_menuitem_sensitive(TRUE);
  }

  dive_set_dive_save_sensitive(TRUE);
  dive_set_dive_delete_sensitive(TRUE);
  dive_set_select_site_sensitive(TRUE);
  dive_set_select_tank_sensitive(TRUE);

  buddy_load_list(0);
  equipment_load_list(0);
  type_load_list(0);
  profile_load(); /* must be loaded before dive_tank */
  dive_tank_load_list(0);

  return 0;
}

static void dive_clear_list(void)
{
  gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(lookup_widget(main_window,"divelist")))));
}

void dive_clear(void)
{
  dive_clear_list();
  dive_detail_set_dive_number("");
  dive_detail_set_dive_site("");
  dive_detail_set_dive_date(0);
  dive_detail_set_dive_time_text("");
  dive_detail_set_dive_duration_text("");
  dive_detail_set_dive_max_depth_text("");
  dive_detail_set_dive_SI_text("");
  dive_detail_set_dive_max_temp_text("");
  dive_detail_set_dive_min_temp_text("");
  dive_detail_set_dive_visibility_text("");
  dive_detail_set_dive_notes("");
  dive_detail_set_dive_weight_text("");
  current_dive_id=0;
  current_dive_number=0;
  current_site_id=0;
  dive_set_dive_save_sensitive(FALSE);
  dive_set_dive_delete_sensitive(FALSE);
  dive_set_select_site_sensitive(FALSE);
  dive_set_select_tank_sensitive(FALSE);
  dive_set_merge_next_dive_menuitem_sensitive(FALSE);
}

void dive_init(void)
{
  GtkTreeViewColumn *column;
  GtkWidget *dive_list;
  GtkListStore *dive_list_store;

  dive_list=GTK_WIDGET(lookup_widget(main_window,"divelist"));
  dive_list_store=gtk_list_store_new(DIVELIST_NUM_COLS,G_TYPE_INT,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_INT);

  column=gtk_tree_view_column_new_with_attributes(_("Num"),gtk_cell_renderer_text_new(),"text",DIVELIST_COL_DIVENUMBER,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(dive_list),column);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(dive_list_store),DIVELIST_COL_DIVENUMBER,sort_func_long,(gpointer)DIVELIST_COL_DIVENUMBER,NULL);
  gtk_tree_view_column_set_sort_column_id(column,DIVELIST_COL_DIVENUMBER);

  column=gtk_tree_view_column_new_with_attributes(_("Date"),gtk_cell_renderer_text_new(),"text",DIVELIST_COL_DATE,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(dive_list),column);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(dive_list_store),DIVELIST_COL_DATE,sort_func_long,(gpointer)DIVELIST_COL_DIVENUMBER,NULL);
  gtk_tree_view_column_set_sort_column_id(column,DIVELIST_COL_DATE);

  column=gtk_tree_view_column_new_with_attributes(_("Time"),gtk_cell_renderer_text_new(),"text",DIVELIST_COL_TIME,NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW(dive_list),column);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(dive_list_store),DIVELIST_COL_TIME,sort_func_time,(gpointer)DIVELIST_COL_TIME,NULL);
  gtk_tree_view_column_set_sort_column_id(column,DIVELIST_COL_TIME);

  column=gtk_tree_view_column_new_with_attributes(_("Site"),gtk_cell_renderer_text_new(),"text",DIVELIST_COL_SITE,NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW(dive_list),column);
  gtk_tree_view_column_set_sort_column_id(column, DIVELIST_COL_SITE);

  column=gtk_tree_view_column_new_with_attributes(_("Maxdepth"),gtk_cell_renderer_text_new(),"text",DIVELIST_COL_MAXDEPTH,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(dive_list),column);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE (dive_list_store),DIVELIST_COL_MAXDEPTH,sort_func_double,(gpointer)DIVELIST_COL_MAXDEPTH, NULL);
  gtk_tree_view_column_set_sort_column_id(column,DIVELIST_COL_MAXDEPTH);

  column=gtk_tree_view_column_new_with_attributes(_("Duration"),gtk_cell_renderer_text_new(),"text",DIVELIST_COL_DURATION,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(dive_list),column);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(dive_list_store),DIVELIST_COL_DURATION,sort_func_time,(gpointer)DIVELIST_COL_DURATION,NULL);
  gtk_tree_view_column_set_sort_column_id(column, DIVELIST_COL_DURATION);

  gtk_tree_view_set_model(GTK_TREE_VIEW(dive_list),GTK_TREE_MODEL(dive_list_store));
  g_object_unref (GTK_TREE_MODEL(dive_list_store));

  buddy_init();
  equipment_init();
  type_init();
  dive_tank_init();   
  profile_init(); 
}

void dive_list_cursor_changed(GtkTreeView * treeview, gpointer user_data)
{
  gchar *dive_numberstr,*site_full_name,*timestr,*max_depthstr,*durationstr;
  GtkTreeModel *model;
  GtkTreeIter iter;

  model=gtk_tree_view_get_model(treeview);
  gtk_tree_selection_get_selected(gtk_tree_view_get_selection(treeview),&model,&iter); 
  gtk_tree_model_get(model,&iter,
    DIVELIST_COL_DIVENUMBER,&current_dive_number,
    DIVELIST_COL_TIME,&timestr,
    DIVELIST_COL_SITE,&site_full_name,
    DIVELIST_COL_MAXDEPTH,&max_depthstr,
		DIVELIST_COL_DURATION,&durationstr,
    DIVELIST_COL_ID,&current_dive_id,
    -1
  );
  dive_numberstr=g_strdup_printf("%d",current_dive_number);
  dive_detail_set_dive_number(dive_numberstr);
  dive_detail_set_dive_site(site_full_name);
  dive_detail_set_dive_time_text(timestr);
  dive_detail_set_dive_max_depth_text(max_depthstr);
  dive_detail_set_dive_duration_text(durationstr);
    
  g_free(dive_numberstr);
  g_free(site_full_name); 
  g_free(timestr);
  g_free(max_depthstr);
  g_free(durationstr);
  dive_db_load_dive_detail(current_dive_id,dive_detail_callback);  
  dive_db_set_current_template(preferences.template_dive_number?preferences.template_dive_number:current_dive_number);
}

void dive_load_list(gint select_id)
{
  GtkWidget *dive_list;
  GtkTreePath *path;
  LoadListData load_list_data={0,NULL,NULL};

  dive_clear_list();
  dive_list=GTK_WIDGET(lookup_widget(main_window,"divelist"));
  load_list_data.model=gtk_tree_view_get_model(GTK_TREE_VIEW(dive_list));
  load_list_data.select_id=select_id;
  dive_db_divelist_load_store(&load_list_data,dive_list_callback);
  if(!select_id) {
    if(helper_model_has_items(load_list_data.model)) {
	    load_list_data.select_path=gtk_tree_path_new_first();
	  }
  }
  if(load_list_data.select_path) {
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(dive_list),load_list_data.select_path,NULL,FALSE);
    gtk_tree_path_free(load_list_data.select_path);
  }
}

void dive_save(void)
{
  gchar *datestr,*notes;
  gdouble maxdepth,mintemp,maxtemp,visibility,weight;
  glong duration;

  datestr=dive_make_date_time_string(dive_detail_get_dive_date()+dive_detail_get_dive_time());
  maxdepth=dive_detail_get_dive_max_depth();
  visibility=dive_detail_get_dive_visibility();
  if(preferences.depth_unit != 'm') {
    maxdepth=convert_feet_to_meters(maxdepth);
    visibility=convert_feet_to_meters(visibility);
  }
  mintemp=dive_detail_get_dive_min_temp();
  maxtemp=dive_detail_get_dive_max_temp();
  if(preferences.temperature_unit!='c') {
    mintemp = convert_farenheit_to_celcius (mintemp);
    maxtemp = convert_farenheit_to_celcius (maxtemp);
  }
  weight=dive_detail_get_dive_weight();
  if(preferences.weight_unit != 'k') {
    weight=convert_lbs_to_kgs(weight);
  }
  duration=dive_detail_get_dive_duration();
  notes=dive_detail_get_dive_notes();
  if(dive_db_update(current_dive_id,datestr,duration,maxdepth,mintemp,maxtemp,notes,current_site_id,visibility,weight))
    dive_load_list(current_dive_id);
  g_free (datestr);
  g_free (notes);
}

void dive_delete(void)
{
  gchar *message;

  message=g_strdup_printf (_("Are you sure you want to delete dive # %d?"),current_dive_number);
  if(prompt_message_question (message)) {
    if(dive_db_delete(current_dive_id,current_dive_number)) {
	    dive_clear_list();
	    dive_load_list(0);
	  }
  }
  g_free (message);
}

void dive_new(void)
{
  gchar *datestr,*notes;
  gdouble maxdepth,mintemp,maxtemp,visibility,weight;
  glong duration;
  gint new_dive_id;

  datestr=dive_make_date_time_string(dive_detail_get_dive_date()+dive_detail_get_dive_time());
  maxdepth=dive_detail_get_dive_max_depth();
  visibility=dive_detail_get_dive_visibility();
  if(preferences.depth_unit!='m') {
    maxdepth = convert_feet_to_meters (maxdepth);
    visibility = convert_feet_to_meters (visibility);
  }
  mintemp=dive_detail_get_dive_min_temp();
  maxtemp=dive_detail_get_dive_max_temp();
  if(preferences.temperature_unit!='c') {
    mintemp = convert_farenheit_to_celcius (mintemp);
    maxtemp = convert_farenheit_to_celcius (maxtemp);
  }
  weight=dive_detail_get_dive_weight();
  if(preferences.weight_unit != 'k') {
    weight = convert_lbs_to_kgs (weight);
  }
  duration=dive_detail_get_dive_duration();
  notes=dive_detail_get_dive_notes();
  new_dive_id=dive_db_insert(datestr,duration,maxdepth,mintemp,maxtemp,notes,current_site_id,visibility,weight);  
  if(new_dive_id) {
    dive_clear_list();
    dive_load_list(new_dive_id);
  }
  g_free(datestr);
  g_free(notes);
}

void dive_show_site_window(GtkButton * button,gpointer user_data)
{
  site_show_window(current_site_id);
}

static gint dive_site_set_callback(gpointer dummy,gint argc,gchar ** argv,gchar ** azColName)
{
  dive_detail_set_dive_site (argv[0]);
  return 0;
}

void dive_site_set(gint site_id)
{
  current_site_id = site_id;
  site_db_get_site_full_name(site_id,(gpointer)dive_site_set_callback);
}

glong dive_get_duration(void)
{
  return dive_detail_get_dive_duration();
}

glong dive_get_current_dive_number (void)
{
  return current_dive_number;
}

void dive_merge_next(GtkMenuItem *menuitem,gpointer user_data)
{
  MergeDiveData merge_data;
  
  merge_data.datetime=dive_make_date_time_string(dive_detail_get_dive_date()+dive_detail_get_dive_time());
  merge_data.duration=dive_detail_get_dive_duration();  
  merge_data.max_depth=dive_detail_get_dive_max_depth();
  if(preferences.depth_unit != 'm') merge_data.max_depth=convert_feet_to_meters(merge_data.max_depth);
  merge_data.max_temperature=dive_detail_get_dive_max_temp();
  merge_data.min_temperature=dive_detail_get_dive_min_temp();
  if(preferences.temperature_unit != 'c') {
    merge_data.max_temperature=convert_farenheit_to_celcius(merge_data.max_temperature);
    merge_data.min_temperature=convert_farenheit_to_celcius(merge_data.min_temperature);
  }
  if(dive_db_merge_next(current_dive_id,current_dive_number,&merge_data)) dive_load_list(current_dive_id);
  g_free(merge_data.datetime);
}

void dive_split(glong time_split)
{
  gchar *new_datetimestr;

  new_datetimestr=dive_make_date_time_string(dive_detail_get_dive_date()+dive_detail_get_dive_time()+time_split);
  if(dive_db_split(current_dive_id,time_split,new_datetimestr)) dive_load_list(current_dive_id);
}
