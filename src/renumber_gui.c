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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "interface.h"
#include "globals.h"
#include "defines.h"
#include "gdivelog.h"
#include "helper_functions.h"
#include "renumber_db.h"
#include "support.h"

static GtkWidget *renumber_window;

void on_renumber_dives_apply_btn_clicked(GtkButton *button,gpointer user_data)
{ 
  GtkWidget *widget_initial_dive_number;
  gint initial_dive_number;
  
  widget_initial_dive_number=GTK_WIDGET(lookup_widget(renumber_window,"initial_dive_number_spinbutton"));
  initial_dive_number=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget_initial_dive_number));
  renumber_db_renumber_dives(initial_dive_number);
  dive_load_list(0);
  gtk_widget_destroy(renumber_window);
}

void on_renumber_dives_cancel_btn_clicked(GtkButton *button,gpointer user_data)
{
  gtk_widget_destroy(renumber_window);
}

void renumber_show_window(void)
{
  renumber_window=GTK_WIDGET(create_renumber_window());
  gtk_window_set_transient_for(GTK_WINDOW(renumber_window),GTK_WINDOW(main_window));
  gtk_widget_show(renumber_window);
}
