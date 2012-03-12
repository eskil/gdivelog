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
#include <string.h>

#include "interface.h"
#include "globals.h"
#include "defines.h"
#include "gdivelog.h"
#include "helper_functions.h"
#include "statistics_db.h"
#include "format_fields.h"
#include "site_db.h"

enum {
  STATISTICS_COL_NAME=0,
  STATISTICS_COL_VALUE,
  STATISTICS_NUM_COLS
};

static GtkWidget *statistics_window=NULL;

typedef struct
{
  GtkTreeIter *iter;
  gint site_parent_id;
  gint dive_count;
}StatisticsSiteData;

typedef struct
{
  GHashTable *hash_table;
  GtkTreeModel *model;
}StatisticsLoadSitesData;

static void statistics_site_hash_value_destroy(gpointer data)
{
  g_free(((StatisticsSiteData*)data)->iter);
  g_free(data);
}

static statistics_load_sites_show_dive_count(gpointer key,gpointer value,gpointer user_data)
{
  gtk_tree_store_set(GTK_TREE_STORE(user_data),((StatisticsSiteData*)value)->iter,
    STATISTICS_COL_VALUE,((StatisticsSiteData*)value)->dive_count,
    -1
  );
}

static gint statistics_load_sites_count_callback(StatisticsLoadSitesData *statistics_load_site_data,gint argc,gchar **argv, gchar **azColName)
{
  gint dive_count,id;
  StatisticsSiteData *statistics_site_data;
  
  dive_count=strtol(argv[2],NULL,0);  
  id=strtol(argv[0],NULL,0);
  statistics_site_data=g_hash_table_lookup(statistics_load_site_data->hash_table,&id);
  statistics_site_data->dive_count=statistics_site_data->dive_count+dive_count;
  dive_count=statistics_site_data->dive_count;
  id=strtol(argv[1],NULL,0);
  if(id) {
    statistics_site_data=g_hash_table_lookup(statistics_load_site_data->hash_table,&id);
    statistics_site_data->dive_count=statistics_site_data->dive_count+dive_count;
  }
  return 0;
}

static gint statistics_load_sites_callback(StatisticsLoadSitesData *statistics_load_site_data,gint argc,gchar **argv, gchar **azColName)
{
  gint *key;
  GtkTreeIter *iter,*parent_iter;
  StatisticsSiteData *statistics_site_data,*parent_statistics_site_data;
  
  key=g_malloc(sizeof(gint));
  *key=strtol(argv[0],NULL,0);
  statistics_site_data=g_malloc(sizeof(StatisticsSiteData*));
  statistics_site_data->iter=g_malloc(sizeof(GtkTreeIter*));
  statistics_site_data->site_parent_id=strtol(argv[1],NULL,0);
  statistics_site_data->dive_count=0;
  g_hash_table_insert(statistics_load_site_data->hash_table,key,statistics_site_data);
  if(!statistics_site_data->site_parent_id) parent_iter=NULL;
  else {
    parent_statistics_site_data=g_hash_table_lookup(statistics_load_site_data->hash_table,&statistics_site_data->site_parent_id);
    parent_iter=parent_statistics_site_data->iter;
  }
  gtk_tree_store_append(GTK_TREE_STORE(statistics_load_site_data->model),statistics_site_data->iter,parent_iter);
  gtk_tree_store_set(GTK_TREE_STORE(statistics_load_site_data->model),statistics_site_data->iter,
    STATISTICS_COL_NAME,argv[2],
    -1
  );
  return 0;
}

static void statistics_load_sites(void)
{
  gint max_site_id, min_site_id, i;
  GtkWidget *widget,*sitetree;
  GtkTreeIter iter;
  GtkTreeViewColumn *column;
  StatisticsLoadSitesData statistics_load_site_data;

  widget=GTK_WIDGET(lookup_widget(statistics_window,"statistics_sites_tree"));

  column=gtk_tree_view_column_new_with_attributes(_("site"),gtk_cell_renderer_text_new(),"text",STATISTICS_COL_NAME,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(widget),column);  
  gtk_tree_view_column_set_sort_column_id(column,STATISTICS_COL_NAME);
  
  column=gtk_tree_view_column_new_with_attributes(_("Number Of Dives"),gtk_cell_renderer_text_new(),"text",STATISTICS_COL_VALUE,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(widget),column);
  gtk_tree_view_column_set_sort_column_id(column,STATISTICS_COL_VALUE);
  
  statistics_load_site_data.model=GTK_TREE_MODEL(gtk_tree_store_new(STATISTICS_NUM_COLS,G_TYPE_STRING,G_TYPE_LONG));
  statistics_load_site_data.hash_table=g_hash_table_new_full(g_int_hash,g_int_equal,g_free,statistics_site_hash_value_destroy);    
  
  statistics_db_load_sites((gpointer)&statistics_load_site_data,(gpointer)statistics_load_sites_callback);
  statistics_db_load_sites_count((gpointer)&statistics_load_site_data,(gpointer)statistics_load_sites_count_callback);
 
  g_hash_table_foreach(statistics_load_site_data.hash_table,(GHFunc)statistics_load_sites_show_dive_count,(gpointer)statistics_load_site_data.model);
  g_hash_table_destroy(statistics_load_site_data.hash_table);

  gtk_tree_view_set_model(GTK_TREE_VIEW(widget),statistics_load_site_data.model);
  g_object_unref(statistics_load_site_data.model);
}

static gint statistics_load_list_callback(GtkListStore *list_store,gint argc,gchar **argv,gchar **azColName)
{
  GtkTreeIter iter;
  
  gtk_list_store_append(list_store,&iter);
  gtk_list_store_set(list_store,&iter,
    STATISTICS_COL_NAME,argv[0],
    STATISTICS_COL_VALUE,strtol(argv[1],NULL,0),
    -1
  ); 
  return 0;
}

static void statistics_load_types(void)
{
  GtkWidget *widget;
  GtkListStore *list_store;
  GtkTreeIter iter;
  GtkTreeViewColumn *column;
  
  widget=GTK_WIDGET(lookup_widget(statistics_window,"statistics_types_list"));
  
  list_store=gtk_list_store_new(STATISTICS_NUM_COLS,G_TYPE_STRING,G_TYPE_LONG);  
  
  column=gtk_tree_view_column_new_with_attributes(_("Type"),gtk_cell_renderer_text_new(),"text",STATISTICS_COL_NAME,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(widget),column);
  gtk_tree_view_column_set_sort_column_id(column,STATISTICS_COL_NAME);

  column=gtk_tree_view_column_new_with_attributes(_("Number of Dives"),gtk_cell_renderer_text_new(),"text",STATISTICS_COL_VALUE,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(widget),column);
  gtk_tree_view_column_set_sort_column_id(column,STATISTICS_COL_VALUE);

  statistics_db_load_types(list_store,statistics_load_list_callback);

  gtk_tree_view_set_model(GTK_TREE_VIEW(widget),GTK_TREE_MODEL(list_store));
  g_object_unref(GTK_TREE_MODEL(list_store));
}

static void statistics_load_tanks(void)
{
  GtkWidget *widget;
  GtkListStore *list_store;
  GtkTreeIter iter;
  GtkTreeViewColumn *column;
  
  widget=GTK_WIDGET(lookup_widget(statistics_window,"statistics_tanks_list"));
  
  list_store=gtk_list_store_new(STATISTICS_NUM_COLS,G_TYPE_STRING,G_TYPE_LONG);  
  
  column=gtk_tree_view_column_new_with_attributes(_("Type"),gtk_cell_renderer_text_new(),"text",STATISTICS_COL_NAME,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(widget),column);
  gtk_tree_view_column_set_sort_column_id(column,STATISTICS_COL_NAME);

  column=gtk_tree_view_column_new_with_attributes(_("Number of Dives"),gtk_cell_renderer_text_new(),"text",STATISTICS_COL_VALUE,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(widget),column);
  gtk_tree_view_column_set_sort_column_id(column,STATISTICS_COL_VALUE);

  statistics_db_load_tanks(list_store,statistics_load_list_callback);

  gtk_tree_view_set_model(GTK_TREE_VIEW(widget),GTK_TREE_MODEL(list_store));
  g_object_unref(GTK_TREE_MODEL(list_store));
}

static void statistics_load_equipment(void)
{
  GtkWidget *widget;
  GtkListStore *list_store;
  GtkTreeIter iter;
  GtkTreeViewColumn *column;
  
  widget=GTK_WIDGET(lookup_widget(statistics_window,"statistics_equipment_list"));
  
  list_store=gtk_list_store_new(STATISTICS_NUM_COLS,G_TYPE_STRING,G_TYPE_LONG);  
  
  column=gtk_tree_view_column_new_with_attributes(_("Equipment"),gtk_cell_renderer_text_new(),"text",STATISTICS_COL_NAME,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(widget),column);
  gtk_tree_view_column_set_sort_column_id(column,STATISTICS_COL_NAME);

  column=gtk_tree_view_column_new_with_attributes(_("Number of Dives"),gtk_cell_renderer_text_new(),"text",STATISTICS_COL_VALUE,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(widget),column);
  gtk_tree_view_column_set_sort_column_id(column,STATISTICS_COL_VALUE);
  
  statistics_db_load_equipment(list_store,statistics_load_list_callback);

  gtk_tree_view_set_model(GTK_TREE_VIEW(widget),GTK_TREE_MODEL(list_store));
  g_object_unref(GTK_TREE_MODEL(list_store));
}

static void statistics_load_buddies(void)
{
  GtkWidget *widget;
  GtkListStore *list_store;
  GtkTreeIter iter;
  GtkTreeViewColumn *column;
  
  widget=GTK_WIDGET(lookup_widget(statistics_window,"statistics_buddies_list"));
  
  list_store=gtk_list_store_new(STATISTICS_NUM_COLS,G_TYPE_STRING,G_TYPE_LONG);  
  
  column=gtk_tree_view_column_new_with_attributes(_("Buddy"),gtk_cell_renderer_text_new(),"text",STATISTICS_COL_NAME,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(widget),column);
  gtk_tree_view_column_set_sort_column_id(column,STATISTICS_COL_NAME);

  column=gtk_tree_view_column_new_with_attributes(_("Number of Dives"),gtk_cell_renderer_text_new(),"text",STATISTICS_COL_VALUE,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(widget),column);
  gtk_tree_view_column_set_sort_column_id(column,STATISTICS_COL_VALUE);
  
  statistics_db_load_buddies(list_store,statistics_load_list_callback);

  gtk_tree_view_set_model(GTK_TREE_VIEW(widget),GTK_TREE_MODEL(list_store));
  g_object_unref(GTK_TREE_MODEL(list_store));
}

static gint statistics_load_depth_range_callback(LoadDepthRangeData *load_depth_range_data,gint argc,gchar **argv,gchar **azColName)
{
  GtkTreeIter iter;
  
  gtk_list_store_append(load_depth_range_data->list_store,&iter);
  gtk_list_store_set(load_depth_range_data->list_store,&iter,
	  STATISTICS_COL_NAME,load_depth_range_data->str,
    STATISTICS_COL_VALUE,strtol(argv[0],NULL,0),
    -1
  ); 
  return 0;
}

static void statistics_load_depth_range(gdouble max_depth)
{
  LoadDepthRangeData load_depth_range_data;
  GtkListStore *depth_range_list_store;
  GtkWidget *widget;
  GtkTreeViewColumn *column;
  gint i,max_depth_int,depth_increment;
  gchar *lwrstr,*uprstr;
  
  widget=GTK_WIDGET(lookup_widget(statistics_window,"statistics_depth_range_list"));
  depth_range_list_store=gtk_list_store_new(STATISTICS_NUM_COLS,G_TYPE_STRING,G_TYPE_INT);  
  
  column=gtk_tree_view_column_new_with_attributes(_("Depth Range"),gtk_cell_renderer_text_new(),"text",STATISTICS_COL_NAME,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(widget),column);
  gtk_tree_view_column_set_sort_column_id(column,STATISTICS_COL_NAME);

  column=gtk_tree_view_column_new_with_attributes(_("Number of Dives"),gtk_cell_renderer_text_new(),"text",STATISTICS_COL_VALUE,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(widget),column);
  gtk_tree_view_column_set_sort_column_id(column,STATISTICS_COL_VALUE);

  load_depth_range_data.list_store=depth_range_list_store;  

  if(preferences.depth_unit=='m') depth_increment=10;
  else depth_increment=30;
  max_depth_int=((gint)max_depth/depth_increment)*depth_increment+depth_increment;
  
  for(i=0;i<max_depth_int;i+=depth_increment) {
    load_depth_range_data.lwr=(gdouble)i;
    load_depth_range_data.upr=(gdouble)i+depth_increment;
    lwrstr=format_field_depth(load_depth_range_data.lwr);
    uprstr=format_field_depth(load_depth_range_data.upr);
    load_depth_range_data.str=g_strdup_printf("%s to %s",lwrstr,uprstr);
    g_free(lwrstr);
    g_free(uprstr);
    if(preferences.depth_unit!='m') {
      load_depth_range_data.lwr=convert_feet_to_meters((gdouble)i);
      load_depth_range_data.upr=convert_feet_to_meters((gdouble)i+depth_increment);
    }
    statistics_db_load_depth_range(&load_depth_range_data,statistics_load_depth_range_callback);
    g_free(load_depth_range_data.str);
  }
  
  gtk_tree_view_set_model(GTK_TREE_VIEW(widget),GTK_TREE_MODEL(depth_range_list_store));
  g_object_unref(GTK_TREE_MODEL(depth_range_list_store));
}

static gint statistics_load_totals_callback(GtkListStore *list_store,gint argc,gchar **argv,gchar **azColName)
{
  GtkTreeIter iter;
  gchar *str,*names[]={"Number of dives","Deepest","Longest","Total duration","Warmest","Coldest"};
  double dbl;
  glong l;
  
  gtk_list_store_append(list_store,&iter);
  gtk_list_store_set(list_store,&iter,
	  STATISTICS_COL_NAME,_("Total Number of dives"),
    STATISTICS_COL_VALUE,argv[0],
    -1
  );
  dbl=g_strtod(argv[1],NULL);
  if(preferences.depth_unit!='m') dbl=convert_meters_to_feet(dbl);
  str=format_field_depth(dbl);
  gtk_list_store_append(list_store,&iter);
  gtk_list_store_set(list_store,&iter,
	  STATISTICS_COL_NAME,_("Deepest"),
    STATISTICS_COL_VALUE,str,
    -1
  ); 
  g_free(str);
  statistics_load_depth_range(dbl);
  
  l=strtol(argv[2],NULL,NULL,0);
  str=format_field_duration(l);
  gtk_list_store_append(list_store,&iter);
  gtk_list_store_set(list_store,&iter,
	  STATISTICS_COL_NAME,_("Longest"),
    STATISTICS_COL_VALUE,str,
    -1
  );  
  g_free(str);
  l=strtol(argv[3],NULL,NULL,0);
  str=format_field_duration(l);
  gtk_list_store_append(list_store,&iter);
  gtk_list_store_set(list_store,&iter,
	  STATISTICS_COL_NAME,_("Total Duration"),
    STATISTICS_COL_VALUE,str,
    -1
  );  
  g_free(str);
  dbl=g_strtod(argv[4],NULL);
  if(preferences.temperature_unit!='c') dbl=convert_celsius_to_farenheit(dbl);
  str=format_field_temperature(dbl);
  gtk_list_store_append(list_store,&iter);
  gtk_list_store_set(list_store,&iter,
	  STATISTICS_COL_NAME,_("Warmest"),
    STATISTICS_COL_VALUE,str,
    -1
  );    
  g_free(str);
  dbl=g_strtod(argv[5],NULL);
  if(preferences.temperature_unit!='c') dbl=convert_celsius_to_farenheit(dbl);
  str=format_field_temperature(dbl);
  gtk_list_store_append(list_store,&iter);
  gtk_list_store_set(list_store,&iter,
    STATISTICS_COL_NAME,_("Coldest"),
    STATISTICS_COL_VALUE,str,
    -1
  );    
  g_free(str);
  return 0;
}

static void statistics_load_totals(void)
{
  GtkWidget *widget;
  GtkListStore *list_store;
  GtkTreeIter iter;
  GtkTreeViewColumn *column;

  widget=GTK_WIDGET(lookup_widget(statistics_window,"statistics_totals_list"));
  
  list_store=gtk_list_store_new(STATISTICS_NUM_COLS,G_TYPE_STRING,G_TYPE_STRING);  
  
  column=gtk_tree_view_column_new_with_attributes(_("Statistic"),gtk_cell_renderer_text_new(),"text",STATISTICS_COL_NAME,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(widget),column);
  gtk_tree_view_column_set_sort_column_id(column,STATISTICS_COL_NAME);

  column=gtk_tree_view_column_new_with_attributes(_("Value"),gtk_cell_renderer_text_new(),"text",STATISTICS_COL_VALUE,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(widget),column);
  gtk_tree_view_column_set_sort_column_id(column,STATISTICS_COL_VALUE);
  
  statistics_db_load_totals(list_store,statistics_load_totals_callback);
  
  gtk_tree_view_set_model(GTK_TREE_VIEW(widget),GTK_TREE_MODEL(list_store));
  g_object_unref(GTK_TREE_MODEL(list_store));
}

void statistics_show_window(GtkMenuItem *menuitem,gpointer user_data)
{
  statistics_window=GTK_WIDGET(create_statistics_window());
  
  statistics_load_totals();
  statistics_load_sites();
  statistics_load_buddies();
  statistics_load_equipment();
  statistics_load_types();
  statistics_load_tanks();
  gtk_window_set_transient_for(GTK_WINDOW(statistics_window),GTK_WINDOW(main_window));
  gtk_widget_show(statistics_window);
}

void statistics_ok_btn_clicked(GtkButton *button,gpointer user_data)
{
  gtk_widget_destroy(statistics_window);
}
