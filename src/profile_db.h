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

#ifndef PROFILE_DB_H
#define PROFILE_DB_H

gboolean profile_db_load (gint dive_id, gpointer profile_load_callback);
gboolean profile_db_get_average_depth (gint dive_id, glong time_lwr, glong time_upr, gdouble * average_depth);	/* returning average_depth did not work so using pointer instead */
gboolean profile_db_insert_segment (gint dive_id, glong profile_time,
				    gdouble profile_depth,
				    gdouble profile_temperature);

#endif /* PROFILE_DB_H */
