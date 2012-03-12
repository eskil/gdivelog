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

#ifndef GLOGBOOK_DEFINES_H
#define GLOGBOOK_DEFINES_H

#include <gtk/gtk.h>

#undef DB_DEBUG

#define LOGBOOK_APP_FNAME_EXT ".glg"
#define LOGBOOK_APP_FNAME_PATTERN "*.glg"
#define LOGBOOK_APP_NAME "gdivelog"
#define LOGBOOK_APP_DATA_LASTOPENED "/.gdivelog/lastopened"
#define LOGBOOK_APP_DATA_PREFERENCES "/.gdivelog/preferences"
#define LOGBOOK_APP_DATA_DIR "/.gdivelog"

#define GDIVELOG_SCHEMA_VERSION "1.0"

typedef struct
{
  gchar depth_unit;		/* m = meters, anything else feet is assumed          */
  gchar temperature_unit;	/* c = centigrade, anything else farenheit is assumed */
  gchar weight_unit;		/* l = lbs, anything else Kgs is assumed              */
  gchar pressure_unit;		/* b = bar, anything else psi is assumed              */
  gchar volume_unit;		/* l = liter, anything else cuft is assumed           */
  gchar profile_max_ascent_rate;	    /* In meters. Do not show alarms  <=0                 */
  GdkColor profile_depth_color;
  GdkColor profile_temperature_color;
  GdkColor profile_marker_color;
  GdkColor profile_background_color;
  GdkColor profile_alarm_color;
  GdkColor profile_text_axis_color;
  gint merge_variance;
  gint match_variance;
  gdouble split_dive_limit;
  gchar site_name_seperator[4];
  gboolean allow_deletes;
  glong template_dive_number;	    /* -1 = none, 0 = use currently selected else dive_number */
} Preferences;

typedef struct
{
  gint select_id;
  GtkTreePath *select_path;
  GtkTreeModel *model;
} LoadListData;

typedef struct
{
  gint dive_id;
  glong dive_number;
  gchar *datetime;
  glong duration;
  glong duration_mod;
  glong SI;
  gdouble max_depth;
  gdouble max_temperature;
  gdouble min_temperature;
  gboolean has_profile;
} MergeDiveData;


typedef struct {
  gdouble lwr;
  gdouble upr;
  gchar *str;
  GtkListStore *list_store;
}LoadDepthRangeData;

#endif /* GLOGBOOK_DEFINES_H */
