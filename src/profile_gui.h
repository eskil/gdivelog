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

#ifndef PROFILE_GUI_H
#define PROFILE_GUI_H

void profile_init (void);
/* void profile_marker_go_first (void); 
void profile_marker_go_next (void);
void profile_marker_go_previous (void);
void profile_marker_go_last (void);
*/
gdouble profile_get_segment_average_depth (glong time_lwr, glong time_upr);
glong profile_get_max_time (void);
gboolean profile_exists (void);
/*
void profile_set_dive_tank_time_start (void);
void profile_set_dive_tank_time_end (void);
*/
void profile_clear (void);
void profile_set_details (GtkButton *button,gpointer user_data);

#endif /* PROFILE_GUI_H */
