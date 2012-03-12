/*
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

#ifndef GDIVELOG_H
#define GDIVELOG_H

#include <gtk/gtk.h>

#define DATETIME_STR_FMT "%Y-%m-%d %H:%M:%S"

#define NULL_TEMPERATURE -274.0
#define NULL_DEPTH -1.0
#define NULL_VISIBILITY -1.0
#define NULL_VOLUME -1.0
#define NULL_WEIGHT -1.0
#define NULL_WP -1.0
#define NULL_PERCENT -1.0
#define NULL_DURATION -1
#define NULL_SAC 0.0
#define NULL_PRESSURE -1
#define NULL_TIME -1
#define NULL_NOTES ""

#define convert_meters_to_feet(x)       ((x)*3.2808399)
#define convert_feet_to_meters(x)       ((x)/3.2808399)
#define convert_kgs_to_lbs(x)           ((x)*2.20462262)
#define convert_lbs_to_kgs(x)           ((x)/2.20462262)
#define convert_liters_to_cuft(x)       ((x)*0.0353146667)
#define convert_cuft_to_liters(x)       ((x)/0.0353146667)
#define convert_bar_to_psi(x)           ((x)*14.5037738)
#define convert_psi_to_bar(x)           ((x)/14.5037738)
#define convert_celsius_to_farenheit(x) (((x)*1.8)+32)
#define convert_farenheit_to_celcius(x) (((x)-32)/1.8)
#define convert_feet_to_ata(x)          (((x)/33)+1)
#define convert_meters_to_ata(x)        (((x)/10)+1)

typedef enum {
  PLUGIN_TYPE_IMPORT=0,
  PLUGIN_TYPE_EXPORT,
  PLUGIN_TYPE_DOWNLOAD,
  PLUGIN_TYPE_GENERAL
}PluginType;

typedef struct
{
  gchar *name;
  gchar *notes;
} ImportSite;

typedef struct
{
  gchar *name;
  gchar *notes;
} ImportBuddies;

typedef struct
{
  gchar *name;
  gchar *notes;
} ImportEquipment;

typedef struct
{
  gchar *name;
  gchar *notes;
} ImportTypes;

typedef struct
{
  gchar *name;
  gdouble volume;
  gdouble wp;
  gchar *notes;
  gdouble O2;
  gdouble He;
  gdouble time_start;
  gdouble time_end;
  gdouble pressure_start;
  gdouble pressure_end;
  gdouble average_depth;
} ImportTanks;

typedef struct
{
  glong time;
  gdouble depth;
  gdouble temperature;
} ImportProfile;

typedef struct
{
  GtkWidget *main_window;
  gint merge_variance;
  void (*plugin_register)(gchar *dl_sym,PluginType plugin_type,gchar *description);
  gboolean (*begin_transaction)(void);
  gboolean (*commit_transaction)(void);
  gboolean (*rollback_transaction)(void);
  gint   (*import_dive)(gchar *dive_datetime,glong dive_duration,
	           gdouble dive_maxdepth,gdouble dive_mintemp,
			       gdouble dive_maxtemp,gchar *dive_notes,
			       gdouble dive_visibility,gdouble dive_weight,
			       GArray *sites, GArray *buddies, GArray *equipment,
			       GArray *types, GArray *tanks, GArray *profile,
			       gboolean use_template);
  gboolean (*import_profile)(gint dive_id,GArray * profile);
  gint   (*match_import)(gchar *datetime, 
             gdouble max_depth, glong duration, 
             gdouble max_temperature, gdouble min_temperature, 
             GArray *profile);
  gint   (*get_currently_selected_dive_id)(void);
  glong    (*get_currently_selected_dive_number)(void);
  gchar    *(*get_last_dive_datetime)(void);
  void     (*refresh_dive_list)(gint select_dive_id);
  gint     (*sql_query)(gchar *statement,gpointer callback,gpointer user_data,gchar **error_message);
  gboolean (*is_depth_metric)(void);	/* use this for visibility as well */
  gboolean (*is_temperature_metric)(void);
  gboolean (*is_weight_metric)(void);
  gboolean (*is_pressure_metric)(void);
  gboolean (*is_volume_metric)(void);
  gchar    *(*format_field_depth)(gdouble depth);
  gchar    *(*format_field_visibility)(gdouble visibility);
  gchar    *(*format_field_percent)(gdouble percent);
  gchar    *(*format_field_pressure)(gdouble pressure);
  gchar    *(*format_field_time)(glong time);
  gchar    *(*format_field_temperature)(gdouble temperature);
  gchar    *(*format_field_weight)(gdouble weight);
  gchar    *(*format_field_volume)(gdouble volume);
  gchar    *(*get_site_name_seperator)(void);
} GDiveLogPluginInterface;

typedef void (*GDiveLogPluginInCall)();
typedef void (*GDiveLogPluginInit)(GDiveLogPluginInterface *gdivelog_plugin_interface);


#endif /* GDIVELOG_H */
