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
#include <pango/pango.h>

#include "interface.h"
#include "globals.h"
#include "defines.h"
#include "gdivelog.h"
#include "format_fields.h"
#include "support.h"

#define CHART_MARGIN_LEFT 40	/* Where to start drawing profile, not margin of whole chart */
#define CHART_MARGIN_RIGHT 10
#define DEPTH_MARGIN_TOP 10
#define DEPTH_MARGIN_BOTTOM 10
#define TEMPERATURE_MARGIN_TOP 20
#define TEMPERATURE_MARGIN_BOTTOM 10
#define TEMPERATURE_HEIGHT 60
#define CHART_PIP_LENGTH 4
#define TEXT_PADDING_VERTICAL 2

typedef struct {
  gint drawing_area_x;
  gint drawing_area_y;
  gint drawing_area_width;
  gint drawing_area_height;
  gint depth_x;
  gint depth_y;
  gint depth_width;
  gint depth_height;
  gint temperature_x;
  gint temperature_y;
  gint temperature_width;
  gint temperature_height;
  gdouble scale_x;
  gdouble depth_scale_y;
  gdouble temperature_scale_y;
  gint depth_increment;
  gint time_increment;
  gint chart_max_depth;
  glong chart_duration;
  gint chart_max_temperature;
  gint chart_min_temperature;
}ProfileExtents;

typedef struct {
  gdouble max_depth;
  glong duration;
  gboolean has_temperature;
  gdouble max_temperature;
  gdouble min_temperature;
  GArray *segment_array;
  gint segment_array_index;
}ProfileData;

typedef struct
{
  glong time;
  gdouble depth;
  gdouble temp;
} ProfileSegment;

static ProfileData profile_data;
static ProfileExtents profile_extents;
static GdkPixmap *profile_pixmap = NULL;
/* static gdouble x_scale; */

static void profile_set_segment_data_fields(glong time,gdouble depth,gdouble temperature)
{
  gchar *timestr,*depthstr,*temperaturestr;
  GtkWidget *widget_time,*widget_depth,*widget_temperature,*widget_split_dive_btn;

  widget_time=GTK_WIDGET(lookup_widget(main_window,"profile_time"));
  widget_depth=GTK_WIDGET(lookup_widget(main_window, "profile_depth"));
  widget_temperature=GTK_WIDGET(lookup_widget(main_window,"profile_temperature"));
  widget_split_dive_btn=GTK_WIDGET(lookup_widget(main_window,"profile_split_dive_btn"));
  timestr=format_field_duration(time);
  if(depth>preferences.split_dive_limit || !time || time==profile_data.duration) gtk_widget_set_sensitive(GTK_WIDGET(widget_split_dive_btn),FALSE);
  else gtk_widget_set_sensitive(GTK_WIDGET(widget_split_dive_btn),TRUE);
  depthstr=format_field_depth(depth);
  if(preferences.temperature_unit!='c') temperature=convert_celsius_to_farenheit(temperature);
  temperaturestr=format_field_temperature(temperature);
  gtk_label_set_text(GTK_LABEL(widget_time),timestr);
  gtk_label_set_text(GTK_LABEL(widget_depth),depthstr);
  gtk_label_set_text(GTK_LABEL(widget_temperature),temperaturestr);
  g_free(timestr);
  g_free(depthstr);
  g_free(temperaturestr);
}

static void profile_enable(void)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"profile_drawingarea"));
  gtk_widget_show(widget);
  widget=GTK_WIDGET(lookup_widget(main_window,"alignment_profile_controls"));
  gtk_widget_set_sensitive(widget,TRUE);
}

static void profile_disable(void)
{
  GtkWidget *widget;

  profile_set_segment_data_fields(0,0.0,0.0);
  widget=GTK_WIDGET(lookup_widget(main_window,"profile_drawingarea"));
  gtk_widget_hide(GTK_WIDGET(widget));
  widget=GTK_WIDGET(lookup_widget(main_window,"alignment_profile_controls"));
  gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
}

static profile_draw_cursor(void)
{
  GdkGC *gc;
  ProfileSegment *profile_segment;
  gint profile_cursor_x=0;
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"profile_drawingarea"));
  profile_segment=&g_array_index(profile_data.segment_array,ProfileSegment,profile_data.segment_array_index);
  profile_cursor_x=(gint)profile_segment->time*profile_extents.scale_x+profile_extents.depth_x;
  gc=gdk_gc_new(widget->window);
  gdk_gc_set_rgb_fg_color(gc,&preferences.profile_marker_color);
  gdk_gc_set_line_attributes(gc,1,GDK_LINE_SOLID,GDK_CAP_PROJECTING,GDK_JOIN_BEVEL);
  gdk_draw_line(widget->window,gc,profile_cursor_x,0,profile_cursor_x,widget->allocation.height);
  g_object_unref(gc);
}

static void profile_show_segment_data(void)
{
  ProfileSegment *profile_segment;
  gboolean rv;
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"profile_drawingarea"));
  profile_segment=&g_array_index(profile_data.segment_array,ProfileSegment,profile_data.segment_array_index);
  g_signal_emit_by_name(G_OBJECT(widget),"expose-event",0,&rv);
  profile_set_segment_data_fields(profile_segment->time,profile_segment->depth,profile_segment->temp);
}

static gint profile_get_depth_increment(gdouble max_depth)
{
  gint depth_increment;

  if(preferences.depth_unit=='m') {
    if(max_depth<9) depth_increment=1;
    else if(max_depth<18) depth_increment=2;
    else if(max_depth<40) depth_increment=5;
    else depth_increment=10;
  }
  else {
    if(max_depth<18) depth_increment=3;
    else if(max_depth<60) depth_increment=6;
    else if(max_depth<160) depth_increment=15;
    else depth_increment=30;
  }
  return depth_increment;
}

static glong profile_get_time_increment(glong duration)
{
  glong time_increment;

  if(duration<600) time_increment=60;
  else if(duration<1200) time_increment=120;
  else if(duration<3600) time_increment=300;
  else if(duration<7200) time_increment=600;
  else time_increment=1800;
  return time_increment;
}

static void profile_get_extents(GtkWidget *drawing_area,ProfileExtents *extents)
{
  extents->drawing_area_x=0;
  extents->drawing_area_y=0;
  extents->drawing_area_width=drawing_area->allocation.width;
  extents->drawing_area_height=drawing_area->allocation.height;

  extents->depth_increment=profile_get_depth_increment(profile_data.max_depth);
  extents->time_increment=profile_get_time_increment(profile_data.duration);

  extents->chart_max_depth=(profile_data.max_depth+1)/extents->depth_increment;
  extents->chart_max_depth=extents->chart_max_depth*extents->depth_increment+extents->depth_increment;

  extents->chart_duration=profile_data.duration/extents->time_increment;
  extents->chart_duration=extents->chart_duration*extents->time_increment+extents->time_increment;

  extents->depth_scale_y=(double)(extents->drawing_area_height-TEMPERATURE_MARGIN_BOTTOM-TEMPERATURE_HEIGHT-TEMPERATURE_MARGIN_TOP-DEPTH_MARGIN_BOTTOM-DEPTH_MARGIN_TOP)/extents->chart_max_depth;
  extents->scale_x=(double)(extents->drawing_area_width-CHART_MARGIN_RIGHT-CHART_MARGIN_LEFT)/extents->chart_duration;

  extents->depth_x=extents->drawing_area_x+CHART_MARGIN_LEFT;
  extents->depth_y=extents->drawing_area_y+DEPTH_MARGIN_TOP;
  extents->depth_width=extents->chart_duration*extents->scale_x+1;
  extents->depth_height=extents->chart_max_depth*extents->depth_scale_y;

  extents->temperature_x=extents->depth_x;
  extents->temperature_y=extents->drawing_area_height-TEMPERATURE_MARGIN_BOTTOM-TEMPERATURE_HEIGHT;
  extents->temperature_width=extents->depth_width;
  extents->temperature_height=TEMPERATURE_HEIGHT;

  extents->chart_max_temperature=profile_data.max_temperature+0.9;
  extents->chart_min_temperature=profile_data.min_temperature;
  if(extents->chart_max_temperature==extents->chart_min_temperature) { /* Avoid div by 0 when calculating temperature_scale_y */
    extents->chart_max_temperature++;
    extents->chart_min_temperature--;
  }
  extents->temperature_scale_y=extents->temperature_height/(extents->chart_max_temperature-extents->chart_min_temperature);
}

static gboolean profile_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data)
{
  gdouble previous_segment_depth;
  gint i,j,x1,x2,y1,y2,y1_depth,y2_depth,y1_temperature,y2_temperature;
  glong previous_segment_time,chart_gradient,max_ascent_rate;
  ProfileSegment *profile_segment=NULL;
  gchar *str;
  gboolean draw_temperature;
  PangoContext *pc;
  PangoLayout *pl;
  PangoRectangle pr;
  GdkGC *gc;
  GdkColor depth_bg_color;

  if(profile_data.segment_array) {
    if(profile_pixmap) {
      gdk_pixmap_unref(profile_pixmap);
      profile_pixmap=NULL;
    }
    profile_get_extents(widget,&profile_extents);
    if(profile_extents.drawing_area_height>0 && profile_extents.drawing_area_width>0 && widget->window) {
      /* Draw chart */
      if(profile_data.segment_array->len) {
        /* draw background */
        profile_pixmap=gdk_pixmap_new(widget->window,profile_extents.drawing_area_width,profile_extents.drawing_area_height,-1);
        gc=gdk_gc_new(widget->window);
        gdk_gc_set_rgb_fg_color(gc, &preferences.profile_background_color);
        gdk_draw_rectangle(profile_pixmap,gc,TRUE,profile_extents.drawing_area_x,profile_extents.drawing_area_y,profile_extents.drawing_area_width,profile_extents.drawing_area_height);

        max_ascent_rate=preferences.profile_max_ascent_rate;
        if(preferences.depth_unit!='m') max_ascent_rate=convert_meters_to_feet(max_ascent_rate);
       
        /* Set backgound base colour */
        depth_bg_color.red=0xe000;
        depth_bg_color.green=0xf000;
        depth_bg_color.blue=0xffff;
        gdk_gc_set_rgb_fg_color(gc,&depth_bg_color);

        /* draw background for temperature */
        gdk_draw_rectangle(profile_pixmap,gc,TRUE,profile_extents.temperature_x,profile_extents.temperature_y,profile_extents.temperature_width,profile_extents.temperature_height);

        /* draw gradient background */   
        if(preferences.depth_unit=='m') chart_gradient=(0x600*(profile_extents.depth_increment/5+1));
        else chart_gradient=(0x600*(convert_feet_to_meters(profile_extents.depth_increment)/5+1));
        for(i=0;i<profile_extents.chart_max_depth;i+=profile_extents.depth_increment) {
          y1=(i*profile_extents.depth_scale_y)+profile_extents.depth_y;
          gdk_gc_set_rgb_fg_color(gc,&depth_bg_color);
          gdk_draw_rectangle(profile_pixmap,gc,TRUE,profile_extents.depth_x,y1,profile_extents.depth_width,profile_extents.depth_increment*profile_extents.depth_scale_y+1);
          depth_bg_color.red-=chart_gradient;
          depth_bg_color.green-=chart_gradient;
	 }

        /* Text */
        gdk_gc_set_rgb_fg_color(gc,&preferences.profile_text_axis_color);
        pc=gtk_widget_get_pango_context(widget);
        pl=pango_layout_new(pc);

        /* draw depth text */
        j=0;
        for(i=0;i<=profile_extents.chart_max_depth;i+=profile_extents.depth_increment) {  
          str=g_strdup_printf("%d%s",profile_extents.depth_increment*j,preferences.depth_unit=='m'?"M":"ft");
          pango_layout_set_text(pl,str,-1);
          pango_layout_get_pixel_extents(pl,&pr,NULL);
          gdk_draw_layout(profile_pixmap,gc,profile_extents.depth_x-(CHART_MARGIN_LEFT/2)-pr.width/2,(i*profile_extents.depth_scale_y)+profile_extents.depth_y-DEPTH_MARGIN_TOP,pl);
          g_free(str);
          j++;
        }
        /* draw time text & pips */
        j=0;
        for(i=0;i<=profile_extents.chart_duration;i+=profile_extents.time_increment) {
          x1=(i*profile_extents.scale_x)+profile_extents.temperature_x;
          gdk_draw_line(profile_pixmap,gc,x1,profile_extents.depth_height+profile_extents.depth_y,x1,profile_extents.depth_height+profile_extents.depth_y+CHART_PIP_LENGTH);
          gdk_draw_line(profile_pixmap,gc,x1,profile_extents.temperature_y-CHART_PIP_LENGTH,x1,profile_extents.temperature_y);
          str=g_strdup_printf("%d",(profile_extents.time_increment*j)/60);
          pango_layout_set_text(pl,str,-1);
          pango_layout_get_pixel_extents(pl,&pr,NULL);
          gdk_draw_layout(profile_pixmap,gc,x1-(pr.width/2),profile_extents.depth_height+profile_extents.depth_y+CHART_PIP_LENGTH+TEXT_PADDING_VERTICAL, pl);
          g_free(str);
          j++;
        }
        /* draw temperature text */
        if(profile_extents.chart_min_temperature>NULL_TEMPERATURE) {
          str=g_strdup_printf("%d\302\260%c",profile_extents.chart_max_temperature,preferences.temperature_unit=='c'?'C':'F');
          pango_layout_set_text(pl,str,-1);
          pango_layout_get_pixel_extents(pl,&pr,NULL);
          gdk_draw_layout(profile_pixmap,gc,profile_extents.temperature_x-(CHART_MARGIN_LEFT/2)-(pr.width/2),profile_extents.temperature_y-(TEMPERATURE_MARGIN_TOP/2),pl);
          g_free(str);
          str=g_strdup_printf("%d\302\260%c",profile_extents.chart_min_temperature,preferences.temperature_unit=='c'?'C':'F');
          pango_layout_set_text(pl,str,-1);
          pango_layout_get_pixel_extents(pl,&pr,NULL);
          gdk_draw_layout(profile_pixmap,gc,profile_extents.temperature_x-(CHART_MARGIN_LEFT/2)-(pr.width/2),profile_extents.temperature_y+profile_extents.temperature_height-(TEMPERATURE_MARGIN_TOP/2),pl);
          g_free(str);
        }
        g_object_unref(pl);

        profile_segment=&g_array_index(profile_data.segment_array,ProfileSegment,0);

        x1=profile_segment->time*profile_extents.scale_x+profile_extents.depth_x;
        y1_depth=profile_segment->depth*profile_extents.depth_scale_y+profile_extents.depth_y;
        y1_temperature=(profile_extents.chart_max_temperature-profile_segment->temp)*profile_extents.temperature_scale_y+profile_extents.temperature_y;

        gdk_gc_set_line_attributes(gc,1,GDK_LINE_SOLID,GDK_CAP_PROJECTING,GDK_JOIN_BEVEL);
        for(i=1;i<profile_data.segment_array->len; i++) {
          if(profile_segment->temp==NULL_TEMPERATURE) draw_temperature=FALSE;
          else draw_temperature=TRUE;
          previous_segment_depth=profile_segment->depth;
          previous_segment_time=profile_segment->time;
          profile_segment=&g_array_index(profile_data.segment_array,ProfileSegment,i);
        
          /* Depth */
          x2=profile_segment->time*profile_extents.scale_x + profile_extents.depth_x;
          y2_depth=profile_segment->depth*profile_extents.depth_scale_y+profile_extents.depth_y;
          gdk_gc_set_rgb_fg_color(gc,&preferences.profile_depth_color);
          if(max_ascent_rate>0 && profile_segment->time) {
            if(((60/(profile_segment->time-previous_segment_time))*(previous_segment_depth-profile_segment->depth))>max_ascent_rate) {
              gdk_gc_set_rgb_fg_color(gc,&preferences.profile_alarm_color);
            }
          }
          gdk_draw_line(profile_pixmap,gc,x1,y1_depth,x2,y2_depth);
          y1_depth=y2_depth;

          /* Temperature */
          gdk_gc_set_rgb_fg_color(gc,&preferences.profile_temperature_color);
          y2_temperature=(profile_extents.chart_max_temperature-profile_segment->temp)*profile_extents.temperature_scale_y+profile_extents.temperature_y;
          if(draw_temperature) gdk_draw_line(profile_pixmap,gc,x1,y1_temperature,x2,y2_temperature);
          y1_temperature=y2_temperature;
          x1 = x2;
        }
      }
      g_object_unref(gc);
    }
  }
  return FALSE;
}

static gboolean profile_expose_event(GtkWidget *widget,GdkEventExpose *event,gpointer user_data)
{
  if(profile_pixmap) {
    gdk_draw_pixmap(widget->window,widget->style->fg_gc[GTK_WIDGET_STATE(widget)],profile_pixmap,
      0,0,0,0,widget->allocation.width,widget->allocation.height
    );
    profile_draw_cursor();
  }
  return FALSE;
}

static gint profile_segment_array_bin_search(gint idx_hi,gint idx_lo,glong find_time)
{
  gint idx;
  ProfileSegment *profile_segment;
 
  if(idx_lo>idx_hi) return idx_hi;
  idx=(idx_hi+idx_lo)/2;
  profile_segment=&g_array_index(profile_data.segment_array,ProfileSegment,idx);
  if(profile_segment->time<find_time) idx=profile_segment_array_bin_search(idx_hi,idx+1,find_time);
  else if(profile_segment->time>find_time) idx=profile_segment_array_bin_search(idx-1,idx_lo,find_time);
  return idx;
}

static gint profile_get_segment_array_index_from_x(gint x)
{
  gint time;
  ProfileSegment *profile_segment;
  
  
  time=(x-profile_extents.depth_x)/profile_extents.scale_x;
  profile_segment=&g_array_index(profile_data.segment_array,ProfileSegment,0);
  if(time<profile_segment->time) return 0;
  profile_segment=&g_array_index(profile_data.segment_array,ProfileSegment,profile_data.segment_array->len-1);
  if(time>profile_segment->time) return profile_data.segment_array->len-1;
  return profile_segment_array_bin_search(profile_data.segment_array->len,0,time);
}

static gboolean profile_button_press_event(GtkWidget * widget, GdkEventButton * event,gpointer user_data)
{
  if(profile_data.segment_array->len) {
    profile_data.segment_array_index=profile_get_segment_array_index_from_x(event->x);
    profile_show_segment_data();
  }
  return FALSE;
}

static gboolean profile_motion_notify_event(GtkWidget *widget,GdkEventMotion *event,gpointer user_data)
{
  int x, y;
  GdkModifierType state;

  if(profile_data.segment_array->len) {
    gdk_window_get_pointer(event->window,&x,&y,&state);
    if(state&GDK_BUTTON1_MASK) {
      profile_data.segment_array_index=profile_get_segment_array_index_from_x(event->x);
      profile_show_segment_data();
    }
  }
  return FALSE;
}

void profile_init(void)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"profile_drawingarea"));
  g_signal_connect(widget,"configure_event",G_CALLBACK(profile_configure_event),NULL);
  g_signal_connect(widget,"expose_event",G_CALLBACK(profile_expose_event),NULL);
  g_signal_connect(widget,"button_press_event",G_CALLBACK(profile_button_press_event),NULL);
  g_signal_connect(widget,"motion_notify_event",G_CALLBACK(profile_motion_notify_event),NULL);
  gtk_widget_set_events(widget,gtk_widget_get_events(widget)|GDK_BUTTON_PRESS_MASK|GDK_POINTER_MOTION_MASK);
  profile_disable();
}

void profile_marker_go_first(GtkButton *button, gpointer user_data)
{
  profile_data.segment_array_index=0;
  profile_show_segment_data();
}

void profile_marker_go_next(GtkButton *button, gpointer user_data)
{
  if(profile_data.segment_array_index<(profile_data.segment_array->len-1)) {
    profile_data.segment_array_index++;
    profile_show_segment_data();
  }
}

void profile_marker_go_previous(GtkButton *button, gpointer user_data)
{
  if(profile_data.segment_array_index) {
    profile_data.segment_array_index--;
    profile_show_segment_data();
  }
}

void profile_marker_go_last(GtkButton *button, gpointer user_data)
{
  profile_data.segment_array_index=profile_data.segment_array->len-1;
  profile_show_segment_data();
}

static gint profile_load_callback(gpointer dummy, gint argc, gchar ** argv,gchar ** azColName)
{
  ProfileSegment segment;

  segment.time=strtol(argv[0],NULL,0);
  segment.depth=g_strtod(argv[1],NULL); /* REMOVE ? g_ascii_strtod(argv[1],NULL); */
  if(preferences.depth_unit!='m') segment.depth=convert_meters_to_feet(segment.depth);
  segment.temp=g_strtod(argv[2],NULL); /* REMOVE ? g_ascii_strtod(argv[2],NULL); */
  if(preferences.temperature_unit!='c') segment.temp=convert_celsius_to_farenheit(segment.temp);
  if(segment.depth>profile_data.max_depth) profile_data.max_depth=segment.depth;
  if(segment.temp!=NULL_TEMPERATURE) {
    if(segment.temp>profile_data.max_temperature) profile_data.max_temperature=segment.temp;
    if(segment.temp<profile_data.min_temperature) profile_data.min_temperature=segment.temp;
    profile_data.has_temperature=TRUE;
  }
  profile_data.duration=segment.time;
  g_array_append_val(profile_data.segment_array,segment);
  return 0;
}

void profile_clear(void)
{
  if(profile_data.segment_array) g_array_free(profile_data.segment_array,TRUE);
  if(profile_pixmap) gdk_pixmap_unref(profile_pixmap);
  profile_data.segment_array=NULL;
  profile_pixmap=NULL;
  profile_disable();
}

void profile_load(void)
{
  GtkWidget *widget;
  gboolean rv;

  profile_clear();
  widget=GTK_WIDGET(lookup_widget(main_window,"profile_drawingarea")); 

  profile_data.has_temperature=FALSE;
  profile_data.max_depth=0;
  profile_data.duration=0;
  profile_data.max_temperature=G_MINDOUBLE;
  profile_data.min_temperature=G_MAXDOUBLE;
  profile_data.segment_array=g_array_new(FALSE,FALSE,sizeof(ProfileSegment));
  profile_db_load(current_dive_id, profile_load_callback);
  
  if(profile_data.segment_array->len) {
    if(GTK_WIDGET_VISIBLE(widget)) {
      g_signal_emit_by_name(G_OBJECT(widget),"configure-event",0,&rv);
    }
    else profile_enable();
    profile_marker_go_first(NULL,NULL);
  }
  else {
    if(GTK_WIDGET_VISIBLE(widget)) profile_disable();
  } 
}

gdouble profile_get_segment_average_depth(glong time_lwr, glong time_upr)
{
  gint i,count=0;
  gdouble average_depth=-1.0;
  ProfileSegment *profile_segment;

  if(profile_data.segment_array->len) {
    for(i=0;i<profile_data.segment_array->len;i++) {
      profile_segment=&g_array_index(profile_data.segment_array,ProfileSegment,i);
      if(profile_segment->time>=time_lwr) {
        count++;
        average_depth+=profile_segment->depth;
        if(profile_segment->time>=time_upr) break;
      }
    }
    if(count) {
      average_depth=(average_depth+1.0)/count;
      if(preferences.depth_unit!='m') average_depth=convert_meters_to_feet(average_depth);
    }
  }
  return average_depth;
}

glong profile_get_max_time(void)
{
  return profile_data.duration;
}

gboolean profile_exists(void)
{
  gboolean rval=FALSE;
  if(profile_data.segment_array) rval=profile_data.segment_array->len>0?TRUE:FALSE;
  return rval;
}

static glong profile_get_current_time(void) 
{
  GtkWidget *widget;

  widget=GTK_WIDGET(lookup_widget(main_window,"profile_time"));
  return format_field_get_time_value((gchar*)gtk_label_get_text(GTK_LABEL(widget)));
}

void profile_set_dive_tank_time_start(GtkButton *button,gpointer user_data)
{
  dive_tank_detail_set_dive_tank_time_start(profile_get_current_time());
}

void profile_set_dive_tank_time_end(GtkButton *button,gpointer user_data)
{
  dive_tank_detail_set_dive_tank_time_end(profile_get_current_time());
}

void profile_set_dive_details(GtkButton *button,gpointer user_data)
{
  dive_detail_set_dive_max_depth(profile_data.max_depth);
  dive_detail_set_dive_duration(profile_data.duration);
  if(profile_data.has_temperature) {
    dive_detail_set_dive_max_temp(profile_data.max_temperature);
    dive_detail_set_dive_min_temp(profile_data.min_temperature);
  }
}

void profile_split_dive(GtkButton *button,gpointer user_data)
{
  ProfileSegment *profile_segment;
  
  profile_segment=&g_array_index(profile_data.segment_array,ProfileSegment,profile_data.segment_array_index);
  dive_split(profile_segment->time);
}
