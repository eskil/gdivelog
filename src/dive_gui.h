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

#ifndef DIVE_GUI_H
#define DIVE_GUI_H

void dive_init(void);
void dive_clear(void);
void dive_load_list(gint select_dive_id);
void dive_site_set(gint site_id);
glong dive_get_duration(void);
glong dive_get_current_dive_number (void);
void dive_detail_set_dive_duration (glong duration);
void dive_detail_set_dive_max_depth (gdouble max_depth);
void dive_detail_set_dive_max_temp (gdouble max_temp);
void dive_detail_set_dive_min_temp (gdouble min_temp);
void dive_split(glong time_split);

#endif /* DIVE_GUI_H */
