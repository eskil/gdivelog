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

#ifndef FORMAT_FIELDS_H
#define FORMAT_FIELDS_H

gchar *format_field_depth(gdouble depth);
gchar *format_field_visibility(gdouble visibility);
gchar *format_field_percent(gdouble percent);
/* gchar *format_field_timer(glong ltime); */
gchar *format_field_pressure(gdouble pressure);
gchar *format_field_duration(glong duration);
gchar *format_field_temperature(gdouble temperature);
gchar *format_field_weight(gdouble weight);
gchar *format_field_volume(gdouble volume);
gchar *format_field_SAC(gdouble SAC);
gdouble format_field_get_numeric_value_with_null(gchar *value,gdouble null_value,gboolean allow_negative);
glong format_field_get_time_value(gchar* value);

#define format_field_time(t) format_field_duration(t)
#define format_field_get_duration_value(x) format_field_get_time_value(x)

#endif /* FORMAT_FIELDS_H */
