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
#include <stdio.h>

#include "gdivelog.h"
#include "defines.h"
#include "globals.h"
#include "plugins.h"
#include "import_dive.h"
#include "dive_gui.h"
#include "dive_db.h"
#include "db_main.h"
#include "format_fields.h"
#include "interface.h"
#include "support.h"

enum
{
  PLUGIN_COL_DESCRIPTION = 0,
  PLUGIN_COL_FUNCPTR,
  PLUGIN_NUM_COLS
};

typedef struct
{
  GModule *module;
  gchar *name;
} PluginsDL;

/* yeah yeah, static variables... */
static GtkWidget *plugins_window;
static GArray *plugin_dls;
static GtkWidget *widget_plugins_list;
static GtkListStore *plugins_liststore;
static GtkListStore *plugins_import_liststore;
static GtkListStore *plugins_download_liststore;
static GtkListStore *plugins_export_liststore;
static GtkListStore *plugins_general_liststore;
static GDiveLogPluginInterface plugin_interface;
static GModule *current_module;

static gboolean plugins_is_depth_metric(void)
{
  return (preferences.depth_unit=='m');
}

static gboolean plugins_is_temperature_metric (void)
{
  return (preferences.temperature_unit=='c');
}

static gboolean plugins_is_weight_metric(void)
{
  return (preferences.weight_unit=='k');
}

static gboolean plugins_is_pressure_metric (void)
{
  return (preferences.pressure_unit=='b');
}

static gboolean plugins_is_volume_metric (void)
{
  return (preferences.volume_unit=='v');
}

static gint plugin_get_currently_selected_dive_id(void)
{
  return current_dive_id;
}

static gchar *plugin_get_site_name_seperator(void)
{
  return preferences.site_name_seperator;
}

static void plugin_register(gchar *dl_sym,PluginType plugin_type,gchar *description)
{
  GtkTreeIter iter;
  gpointer dl_func_ptr;
  
  if(g_module_symbol(current_module,dl_sym,&dl_func_ptr)) {
    switch(plugin_type) {
      case PLUGIN_TYPE_IMPORT :
        plugins_liststore=plugins_import_liststore;
        break;
      case PLUGIN_TYPE_EXPORT :
        plugins_liststore=plugins_export_liststore;
        break;
      case PLUGIN_TYPE_DOWNLOAD :
        plugins_liststore=plugins_download_liststore;
        break;
      default:
        plugins_liststore=plugins_general_liststore;
        break;
    }
    gtk_list_store_append(plugins_liststore,&iter);
    gtk_list_store_set(plugins_liststore,&iter,
      PLUGIN_COL_DESCRIPTION,description,
      PLUGIN_COL_FUNCPTR,dl_func_ptr,
     -1
    );
  }
}

static void plugins_init_interface(void)
{
  plugin_interface.main_window=main_window;
  plugin_interface.plugin_register=plugin_register;
  plugin_interface.begin_transaction=db_begin_transaction;
  plugin_interface.commit_transaction=db_commit_transaction;
  plugin_interface.rollback_transaction=db_rollback_transaction;
  plugin_interface.import_dive=import_dive;
  plugin_interface.import_profile=import_profile;
  plugin_interface.match_import=dive_db_match_import;
  plugin_interface.refresh_dive_list=dive_load_list;
  plugin_interface.get_currently_selected_dive_id=plugin_get_currently_selected_dive_id;
  plugin_interface.get_currently_selected_dive_number=dive_get_current_dive_number;
  plugin_interface.get_last_dive_datetime=db_get_last_dive_datetime;
  plugin_interface.get_site_name_seperator=plugin_get_site_name_seperator;
  plugin_interface.is_depth_metric=plugins_is_depth_metric;
  plugin_interface.is_temperature_metric=plugins_is_temperature_metric;
  plugin_interface.is_weight_metric=plugins_is_weight_metric;
  plugin_interface.is_pressure_metric=plugins_is_pressure_metric;
  plugin_interface.is_volume_metric=plugins_is_volume_metric;
  plugin_interface.format_field_depth=format_field_depth;
  plugin_interface.format_field_visibility=format_field_visibility;
  plugin_interface.format_field_percent=format_field_percent;
  plugin_interface.format_field_pressure=format_field_pressure;
  plugin_interface.format_field_time=format_field_duration;
  plugin_interface.format_field_temperature=format_field_temperature;
  plugin_interface.format_field_weight=format_field_weight;
  plugin_interface.format_field_volume=format_field_volume;
  plugin_interface.sql_query=db_execute_query;
}

static GModule *plugins_dl_get_module (gchar *dl)
{
  gint i;
  PluginsDL plugin_dl;
  GModule *rval = NULL;

  /* Check if library is already loaded and if so return it's handle */
  for(i=0;i<plugin_dls->len; i++) {
    plugin_dl = g_array_index (plugin_dls, PluginsDL, i);
    if(!g_utf8_collate(dl,plugin_dl.name)) {
      rval=plugin_dl.module;
      break;
    }
  }
  if(!rval) {
    /* library is not loaded, so load it and return it's handle */
    rval=g_module_open(dl,G_MODULE_BIND_LAZY);
    if(rval) {
      /* g_log(G_LOG_DOMAIN,G_LOG_LEVEL_INFO,"Loading plugin library: %s",dl); */
      plugin_dl.module=rval;
      plugin_dl.name=g_strdup(dl);
      g_array_append_val(plugin_dls,plugin_dl);
    }
    else g_log(G_LOG_DOMAIN,G_LOG_LEVEL_WARNING,"Failed to load plugin library %s. Error = %s.",dl,g_module_error());
  }
  return rval;
}

void plugins_show_window(PluginType plugin_type)
{
  gchar *heading;

  GtkTreeViewColumn *column;
  switch(plugin_type) {
    case PLUGIN_TYPE_IMPORT :
      heading=g_strdup((gchar *)_("Import Plugins"));
      plugins_liststore=plugins_import_liststore;
      break;
    case PLUGIN_TYPE_EXPORT :
      heading=g_strdup((gchar *)_("Export Plugins"));
      plugins_liststore=plugins_export_liststore;
      break;
    case PLUGIN_TYPE_DOWNLOAD :
      heading=g_strdup((gchar *)_("Download Plugins"));
      plugins_liststore=plugins_download_liststore;
      break;
    case PLUGIN_TYPE_GENERAL :
      heading=g_strdup((gchar *)_("General Plugins"));
      plugins_liststore=plugins_general_liststore;
      break;
    }
  plugins_window=GTK_WIDGET(create_plugins_window());
  column=gtk_tree_view_column_new_with_attributes(heading,gtk_cell_renderer_text_new(),"text",PLUGIN_COL_DESCRIPTION,NULL);
  g_free(heading);
  widget_plugins_list=GTK_WIDGET(lookup_widget(GTK_WIDGET(plugins_window),"pluginslist"));
  gtk_tree_view_append_column(GTK_TREE_VIEW(widget_plugins_list),column);
  gtk_tree_view_column_set_sort_column_id(column, PLUGIN_COL_DESCRIPTION);
  gtk_tree_view_set_model(GTK_TREE_VIEW(widget_plugins_list),GTK_TREE_MODEL(plugins_liststore));
  gtk_tree_view_set_cursor(GTK_TREE_VIEW(widget_plugins_list),gtk_tree_path_new_first(),NULL,FALSE);
  gtk_window_set_transient_for(GTK_WINDOW(plugins_window),GTK_WINDOW(main_window));
  gtk_widget_show(plugins_window);
  if(!gtk_tree_model_iter_n_children(GTK_TREE_MODEL(plugins_liststore),NULL))
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(plugins_window),"plugins_ok_btn")),FALSE);
}


static void plugins_load_dir(char *path)
{
  GError *error=NULL;
  GDir *dir;
  gchar *dl;
  GDiveLogPluginInit dl_init_func;
  
  if(g_file_test(path,G_FILE_TEST_EXISTS|G_FILE_TEST_IS_DIR)) {
    dir=g_dir_open(path,0,&error);
    if(dir) {
      do {
        dl=(gchar*)g_dir_read_name(dir);
        if(dl) {
          dl=g_strconcat(path,"/",dl,NULL);
          current_module=plugins_dl_get_module(dl);        
          if(g_module_symbol(current_module,"gdivelog_plugin",(gpointer)&dl_init_func)) dl_init_func(&plugin_interface);
          else g_log(G_LOG_DOMAIN,G_LOG_LEVEL_WARNING,"Failed to initialize plugin library '%s'. Is this actually a plugin library?",dl);        
          g_free(dl);          
        }  
      } while(dl);
      g_dir_close(dir);  
    }
    else {
      g_log(G_LOG_DOMAIN,G_LOG_LEVEL_WARNING,"Failed to open plugin dir '%s'\nError: %d\nMessage: '%s'",path,error->code,error->message);
      g_error_free(error);
    }
  }
}

void plugins_load(void)
{
  gchar *plugin_path,**plugin_dir;
  gint i;
  
  plugins_init_interface();
  plugins_import_liststore=gtk_list_store_new(PLUGIN_NUM_COLS,G_TYPE_STRING,G_TYPE_POINTER);
  plugins_export_liststore=gtk_list_store_new(PLUGIN_NUM_COLS, G_TYPE_STRING,G_TYPE_POINTER);
  plugins_download_liststore=gtk_list_store_new(PLUGIN_NUM_COLS,G_TYPE_STRING,G_TYPE_POINTER);
  plugins_general_liststore=gtk_list_store_new (PLUGIN_NUM_COLS, G_TYPE_STRING,G_TYPE_POINTER);
  
  plugin_dls=g_array_new(FALSE, FALSE, sizeof (PluginsDL));
  plugins_load_dir(PLUGIN_DIR);
  plugin_path=(gchar*)g_getenv("GDIVELOG_PLUGIN_PATH");
  if(plugin_path) {
    plugin_dir=g_strsplit(plugin_path,":",0);
    i=0;
    while(plugin_dir[i]) {
      plugins_load_dir(plugin_dir[i]);
      i++;
    }
    g_strfreev(plugin_dir);
  }

}

void plugins_unload(void)
{
  gint i;
  PluginsDL *plugin_dl;

  for(i=0;i<plugin_dls->len;i++) {
    plugin_dl=&g_array_index(plugin_dls,PluginsDL,i);
    /* g_log(G_LOG_DOMAIN,G_LOG_LEVEL_INFO,"Unloading plugin library: %s",plugin_dl->name); */
    if(!g_module_close(plugin_dl->module)) g_log(G_LOG_DOMAIN,G_LOG_LEVEL_WARNING,"Failed to unload plugin '%s'. Error: '%s'",plugin_dl->name,g_module_error());
    g_free (plugin_dl->name);
  }
  g_array_free(plugin_dls,TRUE);
  g_object_unref(GTK_TREE_MODEL (plugins_import_liststore));
  g_object_unref(GTK_TREE_MODEL (plugins_export_liststore));
  g_object_unref(GTK_TREE_MODEL (plugins_download_liststore));
  g_object_unref(GTK_TREE_MODEL (plugins_general_liststore));
}

void plugins_cancel(GtkButton* button,gpointer user_data)
{
  gtk_widget_destroy(plugins_window);
}

void plugins_ok(GtkButton* button,gpointer user_data)
{
  GtkTreeIter iter;
  GDiveLogPluginInCall plugin_call;

  gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW (widget_plugins_list)),(GtkTreeModel **) & plugins_liststore,&iter);
  gtk_tree_model_get(GTK_TREE_MODEL(plugins_liststore),&iter,PLUGIN_COL_FUNCPTR,&plugin_call,-1);
  gtk_widget_destroy(plugins_window);
  plugin_call();
}
