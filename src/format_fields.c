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
#include <time.h>
#include <stdlib.h>
#include <math.h>

#include "globals.h"
#include "gdivelog.h"


gchar *format_field_depth(gdouble depth)
{
  gchar *depthstr;

  if(depth<0) depthstr=g_strdup("");
  else depthstr=g_strdup_printf("%.02f%s",depth,preferences.depth_unit=='m'?_("M"):_("ft"));
  return depthstr;
}

gchar *format_field_visibility(gdouble visibility)
{
  gchar *rvisibility;

  if(visibility>=0) rvisibility=g_strdup_printf("%.02f%s",visibility,preferences.depth_unit=='m'?_("M"):_("ft"));
  else rvisibility=g_strdup("");
  return rvisibility;
}

gchar *format_field_percent(gdouble percent)
{
  gchar *rpercent;

  if(percent<0) rpercent=g_strdup("");
  else rpercent=g_strdup_printf("%.01f%%",percent);
  return rpercent;
}
/* REMOVE ?
gchar *format_field_timer(glong ltime)
{
  gchar *timerstr;

  if (ltime >= 0)
    timerstr = g_strdup_printf ("%d:%02d", ltime / 60, ltime % 60);
  else
    timerstr = g_strdup ("");
  return timerstr;
}
*/

gchar* format_field_pressure(gdouble pressure)
{
  gchar *rpressure;

  if(pressure > 0.0) rpressure=g_strdup_printf("%.0f %s", pressure,preferences.pressure_unit=='b'?_("bar"):_("psi"));
  else rpressure=g_strdup("");
  return rpressure;
}

gchar *format_field_duration(glong duration)
{
  gint h, m;
  gchar *durationstr;

  if(duration<0) durationstr=g_strdup("");
  else {
    h=duration/3600;
    m=(duration-(h*3600))/60;
    durationstr=g_strdup_printf("%d:%02d:%02d",h,m,(duration-(h*3600)-(m*60)));
  }
  return durationstr;
}

gchar *format_field_temperature(gdouble temperature)
{
  gchar *rtemperature;

  if(temperature>NULL_TEMPERATURE) rtemperature=g_strdup_printf("%.01f%s",temperature,preferences.temperature_unit=='c'?_("\302\260C"):_("\302\260F"));
  else rtemperature=g_strdup("");
  return rtemperature;
}

gchar *format_field_weight(gdouble weight)
{
  gchar *rweight;

  if(weight>0.0) rweight=g_strdup_printf("%.01f %s",weight,preferences.weight_unit=='k'?_("kgs"):_("lbs"));
  else rweight=g_strdup("");
  return rweight;
}

gchar *format_field_volume(gdouble volume)
{
  gchar *rvolume;

  if(volume>0.0) rvolume=g_strdup_printf("%.01f %s",volume,preferences.volume_unit=='l'?_("liters"):_("cuft"));
  else rvolume=g_strdup("");
  return rvolume;
}

gchar *format_field_SAC(gdouble SAC)
{
  gchar *rSAC;

  if(SAC>0.0) {
    if(preferences.volume_unit == 'l') rSAC=g_strdup_printf("%.01f %s",SAC,_("L/min"));
    else rSAC=g_strdup_printf("%.04f %s",SAC,_("cuft/min"));
  }
  else rSAC=g_strdup("");
  return rSAC;
}

gdouble format_field_get_numeric_value_with_null(gchar * value,gdouble null_value,gboolean allow_negative)
{
  gchar *str;
  gdouble rval;

  rval=g_strtod(value,NULL);
  if(rval==0.0) {
    str=g_strdup(value);
    g_strchug(str);
    if(!g_ascii_isdigit(str[0]))
      rval=null_value;
      g_free(str);
  }
  if(!allow_negative&&rval<0.0) rval=null_value;
  return rval;
}

glong format_field_get_time_value(gchar *value)
{
  struct tm t;

  g_strstrip(value);
  if(!g_utf8_strlen(value,-1)) return NULL_TIME;
  t.tm_hour=0;
  t.tm_min=0;
  t.tm_sec=0;
  strptime(value,"%H:%M:%S",&t);
  return (t.tm_hour*3600+(t.tm_min*60)+t.tm_sec);
}
