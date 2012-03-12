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

#ifndef DIVE_DB_H
#define DIVE_DB_H

gboolean dive_dblist_load_store(gpointer load_list_data,gpointer divelist_callback);
gboolean dive_db_load_dive_detail(gint dive_id,gpointer divedetail_callback);
gboolean dive_db_update(gint dive_id,gchar *dive_datetime,gulong dive_duration,gdouble dive_maxdepth,gdouble dive_mintemp,gdouble dive_maxtemp,gchar *dive_notes, gint site_id,gdouble dive_visibility,gdouble dive_weight);
gboolean dive_db_delete(gint dive_id,glong dive_number);
gint dive_db_insert(gchar *dive_datetime,gulong dive_duration,gdouble dive_maxdepth,gdouble dive_mintemp,gdouble dive_maxtemp,gchar *dive_notes,gint site_id,gdouble dive_visibility,gdouble dive_weight);
gboolean dive_db_get_min_max_dive_number(glong *min_dive_number,glong *max_dive_number);
void dive_db_set_current_template(glong template_dive_number);
gint dive_db_match_import(gchar *datetime,gdouble max_depth,glong duration,gdouble max_temperature,gdouble min_temperature,GArray *profile);
gboolean dive_db_merge_next (gint dive_id,glong dive_number,MergeDiveData *dive_data);

#endif /* DIVE_DB_H */
