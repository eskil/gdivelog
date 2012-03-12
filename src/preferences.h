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

#ifndef PREFERENCES_H
#define PREFERENCES_H

gboolean preferences_load_do(void);
void preferences_show_window(void);
void preferences_set_ascent_rate(gint ascent_rate_meters);
gint preferences_get_ascent_rate(void);
void preferences_load_template_dive_number(void);
void preferences_save_template_dive_number(void);

#define preferences_load() preferences_load_do();

#endif /* PREFERENCES_H */
