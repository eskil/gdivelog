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

#ifndef PLUGINS_H
#define PLUGINS_H

#include "gdivelog.h"

void plugins_show_window(PluginType plugin_type);
void plugins_load(void);
void plugins_unload(void);
void plugins_cancel(GtkButton* button,gpointer user_data);
void plugins_ok(GtkButton* button,gpointer user_data);

#endif /* PLUGINS_H */
