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

#include <gnome.h>

void on_main_window_destroy (GtkObject * object, gpointer user_data);

void on_new1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_open1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_save1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_quit1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_cut1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_copy1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_paste1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_clear1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_properties1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_preferences1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_about1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_toolbar_new_clicked (GtkToolButton * toolbutton, gpointer user_data);

void on_toolbar_open_clicked (GtkToolButton * toolbutton, gpointer user_data);

void on_toolbar_save_clicked (GtkToolButton * toolbutton, gpointer user_data);

void on_dive_new_tbbtn_clicked (GtkToolButton * toolbutton, gpointer user_data);

void
on_dive_save_tbbtn_clicked (GtkToolButton * toolbutton, gpointer user_data);

void
on_dive_delete_tbbtn_clicked (GtkToolButton * toolbutton, gpointer user_data);

gboolean
  on_profile_drawingarea_button_press_event
  (GtkWidget * widget, GdkEventButton * event, gpointer user_data);


void on_plugins_ok_btn_clicked (GtkButton * button, gpointer user_data);

void on_plugins_cancel_btn_clicked (GtkButton * button, gpointer user_data);

void on_plugins1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_download_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_import_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_export_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_general_activate (GtkMenuItem * menuitem, gpointer user_data);

void
on_ascent_warning_check_btn_toggled (GtkToggleButton * togglebutton,
				     gpointer user_data);

void
on_dive2_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_menu_new_dive_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_menu_save_dive_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_menu_delete_dive_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_menu_merge_next_dive_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);                                   
gboolean
on_main_window_delete                  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
dive_merge_next                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
dive_show_site_window                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_new1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_toolbar_new_clicked                 (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
profile_set_dive_details               (GtkButton       *button,
                                        gpointer         user_data);

void
profile_marker_go_first                (GtkButton       *button,
                                        gpointer         user_data);

void
profile_marker_go_previous             (GtkButton       *button,
                                        gpointer         user_data);

void
profile_marker_go_next                 (GtkButton       *button,
                                        gpointer         user_data);

void
profile_marker_go_last                 (GtkButton       *button,
                                        gpointer         user_data);

void
profile_set_dive_tank_time_start       (GtkButton       *button,
                                        gpointer         user_data);

void
profile_set_dive_tank_time_end         (GtkButton       *button,
                                        gpointer         user_data);

void
preferences_meters_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

gboolean
dive_tank_detail_time_start_validate   (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
dive_tank_detail_average_depth_validate
                                        (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
dive_tank_detail_time_end_validate     (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
dive_tank_detail_pressure_end_validate (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

void
dive_tank_detail_set_SAC               (GtkEditable     *editable,
                                        gpointer         user_data);

gboolean
dive_tank_detail_time_end_validate     (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
dive_tank_detail_O2_per_validate       (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
dive_tank_detail_pressure_start_validate
                                        (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
dive_tank_detail_time_start_validate   (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
dive_tank_detail_He_per_validate       (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

void
dive_tank_list_cursor_changed          (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
dive_tank_set_time_to_all              (GtkButton       *button,
                                        gpointer         user_data);

void
dive_tank_get_segment_average_depth_from_profile
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
dive_tank_show_tank_window             (GtkButton       *button,
                                        gpointer         user_data);

void
dive_tank_new                          (GtkButton       *button,
                                        gpointer         user_data);

void
dive_tank_save                         (GtkButton       *button,
                                        gpointer         user_data);

void
dive_tank_delete                       (GtkButton       *button,
                                        gpointer         user_data);

void
buddy_list_cursor_changed              (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
buddy_new                              (GtkButton       *button,
                                        gpointer         user_data);

void
buddy_save                             (GtkButton       *button,
                                        gpointer         user_data);

void
buddy_delete                           (GtkButton       *button,
                                        gpointer         user_data);

void
equipment_list_cursor_changed          (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
equipment_new                          (GtkButton       *button,
                                        gpointer         user_data);

void
equipment_save                         (GtkButton       *button,
                                        gpointer         user_data);

void
equipment_delete                       (GtkButton       *button,
                                        gpointer         user_data);

void
type_list_cursor_changed               (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
type_new                               (GtkButton       *button,
                                        gpointer         user_data);

void
type_save                              (GtkButton       *button,
                                        gpointer         user_data);

void
type_delete                            (GtkButton       *button,
                                        gpointer         user_data);

void
tank_list_cursor_changed               (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
tank_new                               (GtkButton       *button,
                                        gpointer         user_data);

void
tank_save                              (GtkButton       *button,
                                        gpointer         user_data);

void
tank_delete                            (GtkButton       *button,
                                        gpointer         user_data);

gboolean
tank_detail_volume_validate            (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
tank_detail_wp_validate                (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

void
tank_select                            (GtkButton       *button,
                                        gpointer         user_data);

void
tank_cancel                            (GtkButton       *button,
                                        gpointer         user_data);

void
preferences_ascent_warning_check_btn_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
preferences_template_specific_dive_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
preferences_save                       (GtkButton       *button,
                                        gpointer         user_data);

void
preferences_cancel                     (GtkButton       *button,
                                        gpointer         user_data);

void
site_tree_cursor_changed               (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
site_tree_begin_drag                   (GtkWidget       *widget,
                                        GdkDragContext  *drag_context,
                                        gpointer         user_data);

gboolean
site_tree_drag_drop                    (GtkWidget       *widget,
                                        GdkDragContext  *drag_context,
                                        gint             x,
                                        gint             y,
                                        guint            time,
                                        gpointer         user_data);

void
site_name_changed                      (GtkEditable     *editable,
                                        gpointer         user_data);

void
site_parent_changed                    (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
site_new                               (GtkButton       *button,
                                        gpointer         user_data);

void
site_save                              (GtkButton       *button,
                                        gpointer         user_data);

void
site_delete                            (GtkButton       *button,
                                        gpointer         user_data);

void
site_select                            (GtkButton       *button,
                                        gpointer         user_data);

void
site_cancel                            (GtkButton       *button,
                                        gpointer         user_data);

gboolean
site_delete_event                      (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
dive_list_cursor_changed               (GtkTreeView     *treeview,
                                        gpointer         user_data);

gboolean
dive_detail_time_validate(GtkWidget *widget,GdkEventFocus *event, gpointer user_data);

gboolean 
dive_detail_duration_validate(GtkWidget *widget,GdkEventFocus *event, gpointer user_data);

gboolean 
dive_detail_max_depth_validate(GtkWidget *widget,GdkEventFocus *event, gpointer user_data);

gboolean 
dive_detail_max_temp_validate(GtkWidget *widget,GdkEventFocus *event, gpointer user_data);

gboolean 
dive_detail_min_temp_validate(GtkWidget *widget,GdkEventFocus *event, gpointer user_data);

gboolean 
dive_detail_visibility_validate(GtkWidget *widget,GdkEventFocus *event, gpointer user_data);

gboolean 
dive_detail_weight_validate(GtkWidget *widget,GdkEventFocus *event, gpointer user_data);

gboolean 
dive_detail_date_validate(GtkWidget *widget,GtkDirectionType direction,gpointer user_data);


void
statistics_show_window                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
statistics_ok_btn_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
plugins_cancel                         (GtkButton       *button,
                                        gpointer         user_data);
void
logbook_save_as_prompt                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
profile_split_dive                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_about_ok_button_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_renumber_dives_apply_btn_clicked    (GtkButton       *button,
                                        gpointer         user_data);

void
on_renumber_dives_cancel_btn_clicked   (GtkButton       *button,
                                        gpointer         user_data);

void
on_renumber_dives_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
