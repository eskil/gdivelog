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

#ifndef DIVE_TANK_GUI_H
#define DIVE_TANK_GUI_H

void dive_tank_init(void);
void dive_tank_load_list(gint select_id);
void dive_tank_tank_set(gint tank_id,gchar *tank_name,gdouble tank_volume,gdouble tank_wp);
void dive_tank_detail_set_dive_tank_time_start(glong time);
void dive_tank_detail_set_dive_tank_time_end(glong time);
void dive_tank_clear(void);

#endif /* DIVE_TANK_GUI_H */
