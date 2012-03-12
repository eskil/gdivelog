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
#include <bzlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "interface.h"
#include "callbacks.h"
#include "support.h"
#include "defines.h"
#include "gdivelog.h"
#include "preferences.h"

#define LOCAL
#include "globals.h"
#undef LOCAL

#include "db_main.h"
#include "dive_gui.h"
#include "buddy_gui.h"
#include "equipment_gui.h"
#include "dive_tank_gui.h"
#include "type_gui.h"
#include "profile_gui.h"
#include "plugins.h"

static gchar *logbook_filename=NULL;
static gchar *logbook_tmpfname=NULL;
static FILE *logbook_fp=NULL;

#define BUFSIZE 2048
#define GDIVELOG_TMPFILE_TEMPLATE "gdivelog.XXXXXX"

static void display_error_message(gchar *message)
{
  GtkWidget *dialog;

  dialog=gtk_message_dialog_new(
    GTK_WINDOW (main_window),
    GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
    GTK_BUTTONS_CLOSE, message
  );
  g_signal_connect_swapped(dialog,"response",G_CALLBACK(gtk_widget_destroy),dialog);
  gtk_dialog_run(GTK_DIALOG(dialog));
}

static void display_message(gchar *message)
{
  GtkWidget *dialog;

  dialog=gtk_message_dialog_new(
    GTK_WINDOW (main_window),
    GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO,
    GTK_BUTTONS_CLOSE, message
  );
  g_signal_connect_swapped(dialog,"response",G_CALLBACK (gtk_widget_destroy),dialog);
  gtk_dialog_run(GTK_DIALOG (dialog));
}

gboolean prompt_message(gchar *message, GtkMessageType message_type)
{
  GtkWidget *dialog;
  gint rc;

  dialog=gtk_message_dialog_new(
    GTK_WINDOW (main_window),
    GTK_DIALOG_DESTROY_WITH_PARENT,message_type,
    GTK_BUTTONS_YES_NO,message
  );
  rc=gtk_dialog_run(GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);
  return rc==GTK_RESPONSE_YES?TRUE:FALSE;
}

gboolean check_create_data_dir(void)
{
  gchar *dir;
  gboolean rc = TRUE;

  dir=g_strconcat(g_get_home_dir(),LOGBOOK_APP_DATA_DIR,NULL);
  if(!g_file_test(dir,G_FILE_TEST_EXISTS)) {
    if(mkdir(dir,S_IRWXU)!=0) rc=FALSE;
  }
  else if(!g_file_test(dir,G_FILE_TEST_IS_DIR)) rc=FALSE;
  g_free(dir);
  return rc;
}

static gchar *logbook_last_opened_get(void)
{
  FILE *fp;
  gchar buf[4096], *filepath = NULL, *filename = NULL;

  filename=g_strconcat(g_get_home_dir(),LOGBOOK_APP_DATA_LASTOPENED,NULL);
  fp=fopen(filename,"r");
  if(fp) {
    fgets(buf,4096,fp);
    fclose(fp);
    if(g_file_test(buf,G_FILE_TEST_EXISTS)) filepath=g_strdup(buf);
  }
  g_free (filename);
  return filepath;
}

static gboolean logbook_last_opened_set(char *filepath)
{
  FILE *fp;
  gchar *filename = NULL;
  gboolean rc = FALSE;

  if(check_create_data_dir()) {
    filename=g_strconcat(g_get_home_dir(), LOGBOOK_APP_DATA_LASTOPENED,NULL);
    fp=fopen(filename,"w");
    if(fp) {
      if(fputs(filepath,fp)!=EOF) rc=TRUE;
      fclose(fp);
    }
    g_free (filename);
  }
  return rc;
}

static gboolean logbook_decompress(void)
{

  /* decompresses open file with pointer logbook_fp (ptr to the bzip2ed sqlite3 db file) */
  /* to file with name logbook_tmpfname (the sqlite3 db file)                            */

  FILE *fp_out;
  BZFILE *bz_in;
  gint bzerror,bytes_read,bytes_written;
  guint8 buf[BUFSIZE];
  gboolean rval=FALSE;

  fp_out=fopen(logbook_tmpfname,"w");
  if(fp_out) {
    bz_in=BZ2_bzReadOpen(&bzerror,logbook_fp,0,0,NULL,0);
    if(bzerror==BZ_OK) {
      do {       
        bytes_read=BZ2_bzRead(&bzerror,bz_in,buf,BUFSIZE);
        if((bzerror==BZ_OK | bzerror==BZ_STREAM_END) && bytes_read) {
          bytes_written=fwrite(buf,sizeof(guint8),bytes_read,fp_out);
          if(bytes_written!=bytes_read) {
            rval=FALSE;
            break;
          }
          else rval=TRUE;
        }
        else {
          rval=FALSE;
          break;
        }
      } while(bzerror!=BZ_STREAM_END);
      BZ2_bzReadClose(&bzerror,bz_in);
      if(bzerror != BZ_OK) {
        rval=FALSE;
      }        
    }
    fclose(fp_out);
  }
  return rval;
}

static gboolean logbook_compress(void)
{

  /* compresses file with name logbook_tmpfname (the sqlite3 db file)          */ 
  /* to open file with pointer logbook_fp (ptr to the bzip2ed sqlite3 db file) */

  FILE *fp_in;
  BZFILE *bz_out;
  gint bzerror,bytes_read;
  guint8 buf[BUFSIZE];
  gboolean rval=FALSE;

  fp_in=fopen(logbook_tmpfname,"r");
  if(fp_in) {
    bz_out=BZ2_bzWriteOpen(&bzerror,logbook_fp,3,0,0);
    if(bzerror==BZ_OK) {
      do {
        bytes_read=fread(buf,sizeof(guint8),BUFSIZE,fp_in);
        rval=TRUE;
        if(bytes_read) {
          BZ2_bzWrite(&bzerror,bz_out,buf,bytes_read);
          if(bzerror!=BZ_OK) {
            rval=FALSE;
            break;
          }
        } 
        else if(!feof(fp_in)) rval=FALSE;
      } while(bytes_read == BUFSIZE);
      BZ2_bzWriteClose(&bzerror,bz_out,0,NULL,NULL);
      if(bzerror!=BZ_OK) rval=FALSE;
    }
    fclose(fp_in);
  }
  return rval;
}

static void logbook_set_window_title(gchar *filename)
{
  gchar *title;

  if(filename) {
    title=g_strconcat(LOGBOOK_APP_NAME," - ", g_strrstr(filename,"/")+1,NULL);
    logbook_last_opened_set(filename);
  }
  else title=g_strdup(LOGBOOK_APP_NAME);
  gtk_window_set_title((GtkWindow *)main_window,title);
  g_free(title);
}

static void logbook_init_gui(gchar *filename)
{
  logbook_set_window_title(filename);
  dive_clear();
  buddy_clear();
  type_clear();
  equipment_clear();
  dive_tank_clear();
  profile_clear();
  dive_load_list(0);
}

static gboolean logbook_lock(gboolean verbose)
{
  gboolean rval=TRUE;
  struct flock fl;

  fl.l_type=F_WRLCK;
  fl.l_whence=SEEK_SET;
  fl.l_start=0;
  fl.l_len=0;
  if(fcntl(fileno(logbook_fp),F_SETLK,&fl)==-1) {
    switch(errno) {
      case EAGAIN :
      case EACCES :
        if(verbose) g_printerr(_("Another user has '%s' open.\n\nPlease try again later"),g_strrstr(logbook_filename,"/")+1);
        rval=FALSE;
        break;
      case ENOLCK : 
        if(verbose) g_printerr(_("The system has run out of locks.\n\nIt will be possible for another user to overwrite any changes you make while you are making them and visa versa.\n\nIt maybe safer to close this logbook and try again later or 'save as' the logbook with a different name."));
    }
  }
  return rval;
}

gboolean logbook_save_do(gboolean for_close)
{
  gboolean rval=FALSE;
 
  if(logbook_filename) {
    if(!fseek(logbook_fp,0,SEEK_SET)) {
      if(db_close()) {                             /* close connection to db         */
        if(logbook_compress()) {                   /* compress to logbook file       */
          if(for_close) rval=TRUE;                 /* If saving for close, exit now  */
          else {
            db_saved();
            rval=db_open(logbook_tmpfname);        /* If just saving reconnect to db */
          }
        }
      }
    }
    if(!rval) g_printerr("Unable to save logbook.");
  }
  else {
    logbook_save_as_prompt(NULL,NULL);
    rval=TRUE;
  }
  return rval;
}

gboolean logbook_save_as(gchar *filename)
{
  gboolean rval=TRUE;
  
  if(logbook_fp) {
    if(fclose(logbook_fp)) {
      g_printerr("Unable to close %s",logbook_filename);
      rval=FALSE;
    }
  }
  if(rval) {
    g_free(logbook_filename);
    logbook_filename=g_strdup(filename);
    logbook_fp=fopen(filename,"w+");
    if(logbook_fp) {
      logbook_lock(TRUE);
      logbook_save();
      logbook_set_window_title(filename);
      logbook_last_opened_set(filename);
      rval=TRUE;
    }
    else rval=FALSE;
  }
  return rval;
}

void logbook_save_as_prompt(GtkMenuItem *menuitem,gpointer user_data)
{
  GtkWidget *logbook_save_as_file_dialog;
  GtkFileFilter *file_filter;
  gchar *filename, *tmpfname;

  file_filter=gtk_file_filter_new();
  gtk_file_filter_set_name(file_filter,"gdivelogs");
  gtk_file_filter_add_pattern(file_filter,LOGBOOK_APP_FNAME_PATTERN);
  logbook_save_as_file_dialog=gtk_file_chooser_dialog_new("Save Logbook",
    GTK_WINDOW
    (main_window),
    GTK_FILE_CHOOSER_ACTION_SAVE,
    GTK_STOCK_CANCEL,
    GTK_RESPONSE_CANCEL,
    GTK_STOCK_SAVE,
    GTK_RESPONSE_ACCEPT,
    NULL
  );
  gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(logbook_save_as_file_dialog),file_filter);
  if(gtk_dialog_run(GTK_DIALOG(logbook_save_as_file_dialog))==GTK_RESPONSE_ACCEPT) {
    filename=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(logbook_save_as_file_dialog));
    if(!g_str_has_suffix(filename,LOGBOOK_APP_FNAME_EXT)) {
	    tmpfname=filename;
	    filename=g_strconcat(tmpfname,LOGBOOK_APP_FNAME_EXT, NULL);
	    g_free(tmpfname);
    }
    if(g_file_test(filename,G_FILE_TEST_EXISTS)) g_printerr(_("%s already exists."),filename);
    else logbook_save_as(filename);
    g_free(filename);
  }
  gtk_widget_destroy(logbook_save_as_file_dialog);
}

gboolean logbook_close_do(gboolean prompt_save)
{
  gboolean rval = TRUE;
  
  if(logbook_fp) {
    if(!db_is_saved() && prompt_save) {
      if(prompt_message_question(_("Save log book?"))) rval=logbook_save_for_close();
    }
    else {
      rval=db_close();
    }
    if(rval) {
      if(fclose(logbook_fp)) rval=FALSE;
    }
  }
  if(rval) {  
    if(logbook_tmpfname) {
      g_unlink(logbook_tmpfname);
      g_free(logbook_tmpfname);
      g_free(logbook_filename);
      logbook_fp=NULL;        
      logbook_tmpfname=NULL;
      logbook_filename=NULL;
    }
  }
  else g_printerr(_("Unable to close logbook file\n\nAre you out of disk space or something?"));
  return rval;
}

static gboolean logbook_open_do(gchar *filename,gboolean verbose)
{
  gboolean rval=FALSE;
  gchar template[]=GDIVELOG_TMPFILE_TEMPLATE;
  gint tf;

  logbook_filename=g_strdup(filename);
  tf=g_file_open_tmp(template,&logbook_tmpfname,NULL);
  if(tf>=0) {
    close(tf);
    logbook_fp=fopen(logbook_filename,"r+");
    if(logbook_fp) {
      if(logbook_lock(verbose)) {   
        if(logbook_decompress()) {
          rval=db_open(logbook_tmpfname);
          if(rval) logbook_init_gui(filename);
        }
      }
    }
  }
  if(!rval && verbose) g_printerr (_("Unable to open %s."), filename);
  return rval;
}

gboolean logbook_open_prompt(void)
{
  gboolean rval=FALSE;
  GtkWidget *logbook_open_file_dialog;
  GtkFileFilter *file_filter;
  gchar *filename;

  if(logbook_close()) {
    file_filter=gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter,"gdivelogs");
    gtk_file_filter_add_pattern(file_filter,LOGBOOK_APP_FNAME_PATTERN);
    logbook_open_file_dialog=gtk_file_chooser_dialog_new("Open Logbook",
      GTK_WINDOW
      (main_window),
      GTK_FILE_CHOOSER_ACTION_OPEN,
      GTK_STOCK_CANCEL,
      GTK_RESPONSE_CANCEL,
      GTK_STOCK_OPEN,
      GTK_RESPONSE_ACCEPT,
      NULL
    );
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(logbook_open_file_dialog),file_filter);
    if(gtk_dialog_run(GTK_DIALOG(logbook_open_file_dialog))==GTK_RESPONSE_ACCEPT) {
      filename=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(logbook_open_file_dialog));
      rval=logbook_open(filename);
      g_free(filename);
      
    }
    gtk_widget_destroy(logbook_open_file_dialog);
  }
  return rval;
}

gboolean logbook_new(void)
{
  gchar template[]=GDIVELOG_TMPFILE_TEMPLATE;
  gint tf;
  struct flock fl;
  gboolean rval=FALSE;
  
  if(logbook_close()) {
    tf=g_file_open_tmp(template,&logbook_tmpfname,NULL);
    if(tf>=0) {      
      close(tf);
      if(db_new(logbook_tmpfname)) {      
        if(db_open(logbook_tmpfname)) {
          logbook_init_gui(NULL);
          rval=TRUE;
        }
      }
      else g_printerr(_("Unable to create new logbook"));
    }
  }
  return rval;
}

int main(int argc,char *argv[])
{
  gchar *lastopenedlogbook;
  gboolean open_prompt=TRUE;

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE,
		      argc, argv,
		      GNOME_PARAM_APP_DATADIR, PACKAGE_DATA_DIR, NULL);

  preferences_load();
  main_window=create_main_window();
  plugins_load();
  dive_init();
  g_set_print_handler((gpointer)display_message);
  g_set_printerr_handler((gpointer)display_error_message);
  gtk_widget_show(main_window);
  lastopenedlogbook=logbook_last_opened_get();
  if(lastopenedlogbook) {
    if(logbook_open_last_opened(lastopenedlogbook)) open_prompt=FALSE;
    g_free(lastopenedlogbook);
  }
  if(open_prompt) {
    if(!logbook_open_prompt()) logbook_new();
  }
  gtk_main();
  plugins_unload();
  logbook_close_do(FALSE);

  return 0;
}
