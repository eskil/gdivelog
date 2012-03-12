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

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <sqlite3.h>

#include "interface.h"
#include "globals.h"
#include "defines.h"
#include "gdivelog.h"
#include "dive_db.h"
#include "dive_gui.h"
#include "db_generic_callbacks.h"
#include "support.h"

#define SPLIT_DIVE_RANGE_METERS_UPR 10.0
#define SPLIT_DIVE_RANGE_FEET_UPR 33


extern sqlite3 *logbook_db;
static GtkWidget *preferences_window;

void preferences_load_template_dive_number(void)
{
  gchar *sqlErrMsg=NULL;
  gint rc;

  if(logbook_db) {
    rc=sqlite3_exec(logbook_db,"SELECT preferences_template_dive_number FROM Preferences LIMIT 1",(gpointer)db_generic_callback_long,&preferences.template_dive_number,&sqlErrMsg);
    if(rc!=SQLITE_OK) {
      g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in preferences_load_do()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
      sqlite3_free(sqlErrMsg);
      preferences.template_dive_number=-1;
    }
  }
}

void preferences_save_template_dive_number(void)
{
  gchar *sqlcmd,*sqlErrMsg=NULL;
  gint rc;

  if(logbook_db) {
    sqlcmd=sqlite3_mprintf("UPDATE Preferences SET preferences_template_dive_number=%d",preferences.template_dive_number);
    rc=sqlite3_exec(logbook_db,sqlcmd,NULL,0,&sqlErrMsg);
    if(rc!=SQLITE_OK) {
      g_log(G_LOG_DOMAIN,G_LOG_LEVEL_ERROR,"Error in preferences_save_do()\nCode=%d\nError Message='%s'\n",rc,sqlErrMsg);
      sqlite3_free(sqlErrMsg);
    }
    sqlite3_free(sqlcmd);
  }
  else if(preferences.template_dive_number>0)  preferences.template_dive_number=-1;
}

static void preferences_set_depth_unit(gchar depth_unit)
{
  GtkWidget *widget_meters;
  GtkWidget *widget_feet;

  widget_meters=GTK_WIDGET(lookup_widget(preferences_window,"preferences_meters"));
  widget_feet=GTK_WIDGET(lookup_widget(preferences_window,"preferences_feet"));
  if(depth_unit=='m') {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_meters),TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_feet),FALSE);
  }
  else {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_meters),FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_feet),TRUE);
  }
}

static gchar preferences_get_depth_unit(void)
{
  GtkWidget *widget_meters;

  widget_meters=GTK_WIDGET(lookup_widget(preferences_window,"preferences_meters"));
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget_meters))) return 'm';
  return 'f';
}

static void preferences_set_pressure_unit(gchar pressure_unit)
{
  GtkWidget *widget_bar;
  GtkWidget *widget_psi;

  widget_bar=GTK_WIDGET(lookup_widget(preferences_window,"preferences_bar"));
  widget_psi=GTK_WIDGET(lookup_widget(preferences_window,"preferences_psi"));
  if(pressure_unit=='b') {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(widget_bar),TRUE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(widget_psi),FALSE);
  }
  else {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_bar),FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_psi),TRUE);
  }
}

static gchar preferences_get_pressure_unit(void)
{
  GtkWidget *widget_bar;

  widget_bar=GTK_WIDGET(lookup_widget(preferences_window,"preferences_bar"));
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget_bar))) return 'b';
  return 'p';
}

static void preferences_set_temperature_unit(gchar temperature_unit)
{
  GtkWidget *widget_celsius;
  GtkWidget *widget_farenheit;

  widget_celsius=GTK_WIDGET(lookup_widget(preferences_window,"preferences_celsius"));
  widget_farenheit=GTK_WIDGET(lookup_widget(preferences_window,"preferences_farenheit"));
  if(temperature_unit == 'c') {
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_celsius),TRUE);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_farenheit),FALSE);
  }
  else {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_celsius),FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_farenheit),TRUE);
  }
}

static gchar preferences_get_temperature_unit(void)
{
  GtkWidget *widget_celsius;

  widget_celsius=GTK_WIDGET(lookup_widget(preferences_window,"preferences_celsius"));  
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget_celsius))) return 'c';
  return 'f';
}

static void preferences_set_volume_unit(gchar volume_unit)
{
  GtkWidget *widget_liters;
  GtkWidget *widget_cuft;

  widget_liters=GTK_WIDGET(lookup_widget(preferences_window,"preferences_liters"));
  widget_cuft=GTK_WIDGET(lookup_widget(preferences_window,"preferences_cuft"));
  if(volume_unit=='l') {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_liters),TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_cuft),FALSE);
  }
  else {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_liters),FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_cuft),TRUE);
  }
}

static gchar preferences_get_volume_unit(void)
{
  GtkWidget *widget_liters;

  widget_liters=GTK_WIDGET(lookup_widget(preferences_window,"preferences_liters"));
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget_liters))) return 'l';
  return 'c';
}

static void preferences_set_weight_unit(gchar weight_unit)
{
  GtkWidget *widget_kgs;
  GtkWidget *widget_lbs;
  
  widget_kgs=GTK_WIDGET(lookup_widget(preferences_window,"preferences_kgs"));
  widget_lbs=GTK_WIDGET(lookup_widget(preferences_window,"preferences_lbs"));
  if(weight_unit=='k') {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_kgs),TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_lbs),FALSE);
  }
  else {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(widget_kgs),FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(widget_lbs),TRUE);
  }
}

static gchar preferences_get_weight_unit(void)
{
  GtkWidget *widget_kgs;
  
  widget_kgs=GTK_WIDGET(lookup_widget(preferences_window,"preferences_kgs"));  
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget_kgs))) return 'k';
  return 'l';
}

void preferences_ascent_rate_sensitive(gboolean sensitive)
{
  GtkWidget *widget_hbox_ascent_rate;
  GtkWidget *widget_alarm_colorbutton;
  GtkWidget *widget_ascent_warning_check_btn;
  
  widget_hbox_ascent_rate=GTK_WIDGET(lookup_widget(preferences_window,"hbox_ascent_rate"));
  widget_alarm_colorbutton=GTK_WIDGET(lookup_widget(preferences_window,"preferences_alarm_colorbutton"));
  widget_ascent_warning_check_btn=GTK_WIDGET(lookup_widget(preferences_window,"preferences_ascent_warning_check_btn"));  
  gtk_widget_set_sensitive (GTK_WIDGET (widget_hbox_ascent_rate), sensitive);
  gtk_widget_set_sensitive (GTK_WIDGET (widget_alarm_colorbutton), sensitive);
  if(sensitive) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_ascent_warning_check_btn),TRUE);
  else gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_ascent_warning_check_btn),FALSE);
}

void preferences_set_ascent_rate(gint ascent_rate_meters)
{
  gint rate;
  GtkWidget *widget_ascent_rate;
  GtkWidget *widget_ascent_rate_label;

  widget_ascent_rate=GTK_WIDGET(lookup_widget(preferences_window,"preferences_ascent_rate"));
  widget_ascent_rate_label=GTK_WIDGET(lookup_widget(preferences_window,"preferences_ascent_rate_label"));  
  if(ascent_rate_meters > 0) {
    rate=ascent_rate_meters;
    preferences_ascent_rate_sensitive(TRUE);
  }
  else {
    rate=9;
    preferences_ascent_rate_sensitive(FALSE);
  }
  if(preferences.depth_unit!='m') {
    rate=rate*3.3+0.5;
    gtk_label_set_text(GTK_LABEL(widget_ascent_rate_label),_("ft/min"));
  }
  else gtk_label_set_text(GTK_LABEL(widget_ascent_rate_label),_("M/min"));
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget_ascent_rate),(gdouble)rate);
}

static gint preferences_get_ascent_rate(void)
{
  gint rate=0;
  GtkWidget *widget_ascent_rate; 
  GtkWidget *widget_ascent_warning_check_btn;
  GtkWidget *widget_feet;
  
  widget_ascent_rate=GTK_WIDGET(lookup_widget(preferences_window,"preferences_ascent_rate"));
  widget_ascent_warning_check_btn=GTK_WIDGET(lookup_widget(preferences_window,"preferences_ascent_warning_check_btn"));
  widget_feet=GTK_WIDGET(lookup_widget(preferences_window,"preferences_feet"));
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget_ascent_warning_check_btn))) {
    rate = gtk_spin_button_get_value (GTK_SPIN_BUTTON(widget_ascent_rate));
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget_feet))) rate=rate/3.3+0.5;
  }
  return rate;
}

static void preferences_set_template_dive_number(glong template_dive_number)
{
  glong min_dive_number, max_dive_number;
  GtkWidget *widget_template_specific_dive_number;
  GtkWidget *widget_template_specific_dive;
  GtkWidget *widget_template_specific_none;
  GtkWidget *widget_template_selected_dive;
  
  widget_template_specific_dive_number=GTK_WIDGET(lookup_widget(preferences_window,"preferences_template_specific_dive_number"));
  widget_template_specific_dive=GTK_WIDGET(lookup_widget(preferences_window,"preferences_template_specific_dive"));
  widget_template_specific_none=GTK_WIDGET(lookup_widget(preferences_window,"preferences_template_none"));
  widget_template_selected_dive=GTK_WIDGET(lookup_widget(preferences_window, "preferences_template_selected_dive"));
    
  dive_db_get_min_max_dive_number(&min_dive_number,&max_dive_number);
  gtk_spin_button_set_range(GTK_SPIN_BUTTON(widget_template_specific_dive_number),(gdouble)min_dive_number,(gdouble)max_dive_number);
  if(template_dive_number<=0) {
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget_template_specific_dive_number),(gdouble)max_dive_number);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_template_specific_dive),FALSE);
    if(!template_dive_number) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_template_specific_none),FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_template_selected_dive),TRUE);
    }
    else {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_template_specific_none),TRUE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_template_selected_dive),FALSE);
    }
  }
  else {
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget_template_specific_dive_number),(gdouble)template_dive_number);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_template_selected_dive),FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_template_specific_dive),TRUE);
  }
}

static glong preferences_get_template_dive_number(void)
{
  glong template_dive_number=-1;
  GtkWidget *widget_template_specific_dive;
  GtkWidget *widget_template_specific_dive_number;

  widget_template_specific_dive=GTK_WIDGET(lookup_widget(preferences_window,"preferences_template_specific_dive"));
  widget_template_specific_dive_number=GTK_WIDGET(lookup_widget(preferences_window,"preferences_template_specific_dive_number")); 
  if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget_template_specific_dive))) template_dive_number = 0;
  else template_dive_number=(glong)gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget_template_specific_dive_number));
  return template_dive_number;
}

static void preferences_set_site_name_seperator(gchar *seperatorstr)
{
  GtkWidget *widget_site_name_seperator;

  widget_site_name_seperator=GTK_WIDGET(lookup_widget(preferences_window,"preferences_site_name_seperator"));
  gtk_entry_set_text(GTK_ENTRY(widget_site_name_seperator),seperatorstr);
}

static gboolean preferences_get_site_name_seperator(gchar *seperatorstr)
{
  gchar *new_site_seperatorstr;
  gboolean recache_sites=FALSE;
  GtkWidget *widget_site_name_seperator;

  widget_site_name_seperator=GTK_WIDGET(lookup_widget(preferences_window,"preferences_site_name_seperator"));  
  new_site_seperatorstr=(gchar*)gtk_entry_get_text(GTK_ENTRY(widget_site_name_seperator));
  if(g_utf8_collate(preferences.site_name_seperator,new_site_seperatorstr)) recache_sites=TRUE;
  g_stpcpy(seperatorstr,new_site_seperatorstr);
  return recache_sites;
}

static void preferences_set_merge_variance(gint merge_variance)
{
  GtkWidget *widget_merge_dive_duration_variance;
  
  widget_merge_dive_duration_variance=GTK_WIDGET(lookup_widget(preferences_window,"preferences_merge_dive_duration_variance"));
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget_merge_dive_duration_variance),(gdouble)merge_variance/60);

}

static gint preferences_get_merge_variance(void)
{
  GtkWidget *widget_merge_dive_duration_variance;
  widget_merge_dive_duration_variance=GTK_WIDGET(lookup_widget(preferences_window,"preferences_merge_dive_duration_variance"));
  return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget_merge_dive_duration_variance))*60;
}

static void preferences_set_match_variance(gint match_variance)
{
  GtkWidget *widget_match_variance;
  
  widget_match_variance=GTK_WIDGET(lookup_widget(preferences_window,"preferences_match_variance"));
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget_match_variance),(gdouble)match_variance/60);

}

static gint preferences_get_match_variance(void)
{
  GtkWidget *widget_match_variance;
  widget_match_variance=GTK_WIDGET(lookup_widget(preferences_window,"preferences_match_variance"));
  return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget_match_variance))*60;
}

static void preferences_set_allow_deletes(gboolean allow_deletes)
{
  static GtkWidget *widget_allow_deletes;
 
  widget_allow_deletes=GTK_WIDGET(lookup_widget(preferences_window,"preferences_allow_deletes"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_allow_deletes),allow_deletes);
}

static gboolean preferences_get_allow_deletes(void)
{
  static GtkWidget *widget_allow_deletes;
  
  widget_allow_deletes=GTK_WIDGET(lookup_widget(preferences_window,"preferences_allow_deletes"));
  return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget_allow_deletes));
}

static void preferences_set_split_dive_limit(gdouble split_dive_limit_meters)
{
  gdouble limit;
  GtkWidget *widget_split_dive_limit;
  GtkWidget *widget_split_dive_limit_label;

  widget_split_dive_limit=GTK_WIDGET(lookup_widget(preferences_window,"preferences_split_dive_limit"));
  widget_split_dive_limit_label=GTK_WIDGET(lookup_widget(preferences_window,"preferences_split_dive_limit_label"));  
  if(split_dive_limit_meters > 0) limit=split_dive_limit_meters;
  else limit=1.5;
  if(preferences.depth_unit!='m') {
    limit=convert_meters_to_feet(limit);
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(widget_split_dive_limit),0,SPLIT_DIVE_RANGE_FEET_UPR);
    gtk_spin_button_set_increments(GTK_SPIN_BUTTON(widget_split_dive_limit),1,1);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget_split_dive_limit),0);
    gtk_label_set_text(GTK_LABEL(widget_split_dive_limit_label),_("ft"));
  }
  else {
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(widget_split_dive_limit),0.0,SPLIT_DIVE_RANGE_METERS_UPR);
    gtk_spin_button_set_increments(GTK_SPIN_BUTTON(widget_split_dive_limit),0.1,0.1);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget_split_dive_limit),1);
    gtk_label_set_text(GTK_LABEL(widget_split_dive_limit_label),_("M"));
  }
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget_split_dive_limit),limit);
}

static gdouble preferences_get_split_dive_limit(void)
{
  gdouble limit;
  GtkWidget *widget_split_dive_limit;
  GtkWidget *widget_feet;
  
  widget_split_dive_limit=GTK_WIDGET(lookup_widget(preferences_window,"preferences_split_dive_limit"));
  widget_feet=GTK_WIDGET(lookup_widget(preferences_window,"preferences_feet"));
  limit=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget_split_dive_limit));
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget_feet))) limit=convert_feet_to_meters(limit);
  return limit;
}

gboolean preferences_load_do(void)
{
  FILE *fp;
  gchar *filename=NULL;
  gboolean rval=FALSE;

  filename=g_strconcat( g_get_home_dir(),LOGBOOK_APP_DATA_PREFERENCES,NULL);
  fp=fopen(filename,"rb");
  if(fp) {
    if(fread(&preferences, sizeof(Preferences),1,fp)==1) {
      rval=TRUE;
      preferences_load_template_dive_number();
    }
    fclose(fp);
  }
  g_free(filename);
  if(!rval) {
    preferences.depth_unit = 'm';
    preferences.temperature_unit = 'c';
    preferences.weight_unit = 'l';
    preferences.pressure_unit = 'b';
    preferences.volume_unit = 'l';
    preferences.template_dive_number = 0;
    preferences.profile_max_ascent_rate = 10;
    preferences.profile_depth_color.red = 0;
    preferences.profile_depth_color.green = 0;
    preferences.profile_depth_color.blue = 0xffff;
    preferences.profile_temperature_color.red = 0;
    preferences.profile_temperature_color.green = 0xffff;
    preferences.profile_temperature_color.blue = 0;
    preferences.profile_marker_color.red = 0;
    preferences.profile_marker_color.green = 0;
    preferences.profile_marker_color.blue = 0;
    preferences.profile_background_color.red = 0xffff;
    preferences.profile_background_color.green = 0xffff;
    preferences.profile_background_color.blue = 0xffff;
    preferences.profile_alarm_color.red = 0xffff;
    preferences.profile_alarm_color.green = 0;
    preferences.profile_alarm_color.blue = 0;
    preferences.profile_text_axis_color.red = 0;
    preferences.profile_text_axis_color.green = 0;
    preferences.profile_text_axis_color.blue = 0;
    preferences.merge_variance=600;
    preferences.match_variance=300;
    preferences.split_dive_limit=1.5;
    g_stpcpy(preferences.site_name_seperator,"/");
    preferences.allow_deletes=TRUE;
  }
  return rval;
}

static gboolean preferences_save_do()
{
  FILE *fp;
  gchar *filename=NULL;
  gboolean rval=FALSE;

  if(check_create_data_dir()) {
    filename=g_strconcat(g_get_home_dir(),LOGBOOK_APP_DATA_PREFERENCES,NULL);
    fp=fopen(filename,"wb");
    if(fp) {
      if(fwrite(&preferences,sizeof(Preferences),1,fp)==1) {
        rval=TRUE;
        preferences_save_template_dive_number();
      }
      fclose(fp);
    }
    g_free(filename);
  }

  return rval;
}

void preferences_show_window(void)
{
  GtkWidget *widget_dive_number;
  GtkWidget *widget_depth_colorbutton;
  GtkWidget *widget_temperature_colorbutton;
  GtkWidget *widget_marker_colorbutton;
  GtkWidget *widget_background_colorbutton;
  GtkWidget *widget_alarm_colorbutton;
  GtkWidget *widget_text_axis_colorbutton;

  preferences_window=GTK_WIDGET(create_preferences_window());

  preferences_set_depth_unit(preferences.depth_unit);
  preferences_set_pressure_unit(preferences.pressure_unit);
  preferences_set_temperature_unit(preferences.temperature_unit);
  preferences_set_volume_unit(preferences.volume_unit);
  preferences_set_weight_unit(preferences.weight_unit);
  preferences_set_ascent_rate(preferences.profile_max_ascent_rate);
  preferences_set_template_dive_number(preferences.template_dive_number);

  widget_depth_colorbutton=GTK_WIDGET(lookup_widget(preferences_window,"preferences_depth_colorbutton"));
  widget_temperature_colorbutton=GTK_WIDGET(lookup_widget(preferences_window,"preferences_temperature_colorbutton"));
  widget_marker_colorbutton=GTK_WIDGET(lookup_widget(preferences_window,"preferences_marker_colorbutton"));
  widget_background_colorbutton=GTK_WIDGET(lookup_widget(preferences_window,"preferences_background_colorbutton"));
  widget_alarm_colorbutton=GTK_WIDGET(lookup_widget(preferences_window,"preferences_alarm_colorbutton"));
  widget_text_axis_colorbutton=GTK_WIDGET(lookup_widget(preferences_window,"preferences_text_axis_colorbutton"));
  gtk_color_button_set_color(GTK_COLOR_BUTTON(widget_depth_colorbutton),&preferences.profile_depth_color);
  gtk_color_button_set_color(GTK_COLOR_BUTTON(widget_temperature_colorbutton),&preferences.profile_temperature_color);
  gtk_color_button_set_color(GTK_COLOR_BUTTON(widget_marker_colorbutton),&preferences.profile_marker_color);
  gtk_color_button_set_color(GTK_COLOR_BUTTON(widget_background_colorbutton),&preferences.profile_background_color);
  gtk_color_button_set_color(GTK_COLOR_BUTTON(widget_alarm_colorbutton),&preferences.profile_alarm_color);
  gtk_color_button_set_color(GTK_COLOR_BUTTON (widget_text_axis_colorbutton),&preferences.profile_text_axis_color);

  preferences_set_site_name_seperator(preferences.site_name_seperator);
  preferences_set_merge_variance(preferences.merge_variance);
  preferences_set_match_variance(preferences.match_variance);
  preferences_set_split_dive_limit(preferences.split_dive_limit);
  preferences_set_allow_deletes(preferences.allow_deletes);
  
  gtk_window_set_transient_for(GTK_WINDOW(preferences_window),GTK_WINDOW(main_window));
  gtk_widget_show(preferences_window);
}

void preferences_save(GtkButton *button,gpointer user_data)
{
  gboolean recache_sites=FALSE;
  GtkWidget *widget_depth_colorbutton;
  GtkWidget *widget_temperature_colorbutton;
  GtkWidget *widget_marker_colorbutton;
  GtkWidget *widget_background_colorbutton;
  GtkWidget *widget_alarm_colorbutton;
  GtkWidget *widget_text_axis_colorbutton;
  
  
  preferences.depth_unit=preferences_get_depth_unit();
  preferences.pressure_unit=preferences_get_pressure_unit();
  preferences.temperature_unit=preferences_get_temperature_unit();
  preferences.volume_unit=preferences_get_volume_unit();
  preferences.weight_unit=preferences_get_weight_unit();
  preferences.profile_max_ascent_rate=preferences_get_ascent_rate();
  preferences.template_dive_number=preferences_get_template_dive_number();

  widget_depth_colorbutton=GTK_WIDGET(lookup_widget(preferences_window,"preferences_depth_colorbutton"));
  widget_temperature_colorbutton=GTK_WIDGET(lookup_widget(preferences_window,"preferences_temperature_colorbutton"));
  widget_marker_colorbutton=GTK_WIDGET(lookup_widget(preferences_window,"preferences_marker_colorbutton"));
  widget_background_colorbutton=GTK_WIDGET(lookup_widget(preferences_window,"preferences_background_colorbutton"));
  widget_alarm_colorbutton=GTK_WIDGET(lookup_widget(preferences_window,"preferences_alarm_colorbutton"));
  widget_text_axis_colorbutton=GTK_WIDGET(lookup_widget(preferences_window,"preferences_text_axis_colorbutton"));
 
  gtk_color_button_get_color(GTK_COLOR_BUTTON(widget_depth_colorbutton),&preferences.profile_depth_color);
  gtk_color_button_get_color(GTK_COLOR_BUTTON(widget_temperature_colorbutton),&preferences.profile_temperature_color);
  gtk_color_button_get_color(GTK_COLOR_BUTTON(widget_marker_colorbutton),&preferences.profile_marker_color);
  gtk_color_button_get_color(GTK_COLOR_BUTTON(widget_background_colorbutton),&preferences.profile_background_color);
  gtk_color_button_get_color(GTK_COLOR_BUTTON (widget_alarm_colorbutton),&preferences.profile_alarm_color);
  gtk_color_button_get_color(GTK_COLOR_BUTTON(widget_text_axis_colorbutton),&preferences.profile_text_axis_color);
  
  recache_sites=preferences_get_site_name_seperator(preferences.site_name_seperator);
  preferences.merge_variance=preferences_get_merge_variance();
  preferences.match_variance=preferences_get_match_variance();
  preferences.split_dive_limit=preferences_get_split_dive_limit();
  preferences.allow_deletes=preferences_get_allow_deletes();

  preferences_save_do();
  if(recache_sites) db_cache_sites();
  gtk_widget_destroy(preferences_window);
  dive_load_list(current_dive_id);
}

void preferences_cancel(GtkButton *button,gpointer user_data)
{
  gtk_widget_destroy(preferences_window);
}

void preferences_template_specific_dive_toggled(GtkToggleButton *togglebutton,gpointer user_data)
{
  GtkWidget *widget_template_specific_dive_number;
  
  widget_template_specific_dive_number=GTK_WIDGET(lookup_widget(preferences_window,"preferences_template_specific_dive_number"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget_template_specific_dive_number),gtk_toggle_button_get_active(togglebutton));
}

void preferences_ascent_warning_check_btn_toggled(GtkToggleButton *togglebutton,gpointer user_data)
{
  preferences_ascent_rate_sensitive(gtk_toggle_button_get_active(togglebutton));
}

void preferences_meters_toggled(GtkToggleButton *togglebutton,gpointer user_data)
{
  gint rate;
  gdouble limit;
  GtkWidget *widget_ascent_rate; 
  GtkWidget *widget_meters;
  GtkWidget *widget_ascent_rate_label;
  GtkWidget *widget_split_dive_limit;
  GtkWidget *widget_split_dive_limit_label;

  widget_ascent_rate=GTK_WIDGET(lookup_widget(preferences_window,"preferences_ascent_rate"));
  widget_meters=GTK_WIDGET(lookup_widget(preferences_window,"preferences_meters"));
  widget_ascent_rate_label=GTK_WIDGET(lookup_widget(preferences_window,"preferences_ascent_rate_label"));    
  rate=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget_ascent_rate));
  widget_split_dive_limit=GTK_WIDGET(lookup_widget(preferences_window,"preferences_split_dive_limit"));
  widget_split_dive_limit_label=GTK_WIDGET(lookup_widget(preferences_window,"preferences_split_dive_limit_label"));  
  limit=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget_split_dive_limit));
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget_meters))) {
    gtk_label_set_text(GTK_LABEL (widget_ascent_rate_label),_("M/min"));
    rate=rate/3.3;
    limit=convert_feet_to_meters(limit);
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(widget_split_dive_limit),0.0,SPLIT_DIVE_RANGE_METERS_UPR);
    gtk_spin_button_set_increments(GTK_SPIN_BUTTON(widget_split_dive_limit),0.1,0.1);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget_split_dive_limit),1);
    gtk_label_set_text(GTK_LABEL(widget_split_dive_limit_label),_("M"));
  }
  else {
    gtk_label_set_text(GTK_LABEL (widget_ascent_rate_label),_("ft/min"));
    rate=rate*3.3+ 0.5;
    limit=convert_meters_to_feet(limit);
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(widget_split_dive_limit),0,SPLIT_DIVE_RANGE_FEET_UPR);
    gtk_spin_button_set_increments(GTK_SPIN_BUTTON(widget_split_dive_limit),1,1);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget_split_dive_limit),0);
    gtk_label_set_text(GTK_LABEL(widget_split_dive_limit_label),_("ft"));
  }
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget_ascent_rate),(gdouble)rate);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget_split_dive_limit),limit);
}
