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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "statistics_gui.h"
#include "gdivelog.h"
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "globals.h"
#include "dive_gui.h"
#include "tank_gui.h"
#include "renumber_gui.h"
#include "plugins.h"
#include "db_main.h"

static GtkWidget *about_window=NULL;

gboolean on_main_window_delete(GtkWidget *widget,GdkEvent *event,gpointer user_data)
{
  gboolean rval=FALSE;
  gchar *message;
  
  if(!db_is_saved()) {
    if(!prompt_message_question(_("You have unsaved data.\n\nAre you sure you want to quit?"))) rval=TRUE;
  }
  if(!rval) {
    profile_clear();
    gtk_main_quit();
  }
  return rval;
}

void on_open1_activate(GtkMenuItem *menuitem, gpointer user_data)
{
  logbook_open_prompt();
}

void on_save1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  logbook_save();
}

void on_quit1_activate(GtkMenuItem *menuitem, gpointer user_data)
{
  on_main_window_delete(NULL,NULL,NULL);
}

void
on_cut1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_copy1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_paste1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_clear1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_properties1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_preferences1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  preferences_show_window ();
}


void
on_about1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  GtkAboutDialog *about_dialog;
  GdkPixbuf *about_dialog_logo_pixbuf;
  
  about_dialog=GTK_ABOUT_DIALOG(gtk_about_dialog_new());
  gtk_about_dialog_set_name(about_dialog,"gdivelog");
  gtk_about_dialog_set_version(about_dialog,VERSION);
  gtk_about_dialog_set_copyright(about_dialog,"Copyright (c) Simon Naunton");
  gtk_about_dialog_set_website(about_dialog,"http://gdivelog.sourceforge.net");
  about_dialog_logo_pixbuf=create_pixbuf("gdivelog/gdivelog-about.png");
  if(about_dialog_logo_pixbuf) {
    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about_dialog),about_dialog_logo_pixbuf);
    gdk_pixbuf_unref(about_dialog_logo_pixbuf);
  }
  gtk_window_set_transient_for(GTK_WINDOW(about_dialog),GTK_WINDOW(main_window));
  gtk_widget_show(GTK_WIDGET(about_dialog));
}

void on_toolbar_open_clicked (GtkToolButton * toolbutton, gpointer user_data)
{
  logbook_open_prompt();
}

void on_toolbar_save_clicked (GtkToolButton * toolbutton, gpointer user_data)
{
  logbook_save();
}

void on_dive_new_tbbtn_clicked (GtkToolButton * toolbutton, gpointer user_data)
{
  dive_new();
}


void on_dive_save_tbbtn_clicked (GtkToolButton * toolbutton, gpointer user_data)
{
  dive_save();
}


void on_dive_delete_tbbtn_clicked (GtkToolButton * toolbutton, gpointer user_data)
{
  dive_delete();
}

void on_dive2_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_menu_new_dive_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  dive_new();
}


void
on_menu_save_dive_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  dive_save();
}

void on_menu_delete_dive_activate(GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  dive_delete();
}

void
on_new1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  logbook_new();
}


void
on_toolbar_new_clicked                 (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
  logbook_new();
}



/********** plugins ***********/


void on_download_activate(GtkMenuItem *menuitem, gpointer user_data)
{
  plugins_show_window(PLUGIN_TYPE_DOWNLOAD);
}


void on_import_activate(GtkMenuItem *menuitem, gpointer user_data)
{
  plugins_show_window(PLUGIN_TYPE_IMPORT);
}


void on_export_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  plugins_show_window(PLUGIN_TYPE_EXPORT);
}


void on_general_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  plugins_show_window(PLUGIN_TYPE_GENERAL);
}

void on_plugins_ok_btn_clicked (GtkButton * button, gpointer user_data)
{
  plugins_ok(button,user_data);
}

void on_renumber_dives_activate(GtkMenuItem *menuitem,gpointer user_data)
{
  renumber_show_window();
}
