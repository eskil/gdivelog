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
#include <string.h>

#include "interface.h"
#include "globals.h"
#include "defines.h"
#include "gdivelog.h"
#include "helper_functions.h"
#include "dive_gui.h"
#include "site_db.h"

enum
{
  SITETREE_COL_SITE_NAME = 0,
  SITETREE_COL_SITE_ID,
  SITETREE_COL_SITE_PARENT_ID,
  SITETREE_COL_SEQ_NUMBER,
  SITETREE_NUM_COLS
};

enum
{
  SITEPARENT_COL_SITE_FULL_NAME = 0,
  SITEPARENT_COL_SITE_ID,
  SITEPARENT_NUM_COLS
};

typedef struct
{
  gint select_id;
  GHashTable *hash_table;
  GtkTreePath *select_path;
  GtkTreeModel *site_tree_model;
  GtkTreeModel *site_parent_combo_model;
  gint seq_number;
}SiteLoadData;

static GtkWidget *site_window = NULL;
static gint current_site_id = 0;

static void site_load(gint select_id);

/* DND */
static gboolean(*default_row_drag_possible)();
static gboolean(*default_row_drop_possible)();
static GtkTreePath *dnd_parent_path=NULL;

static gboolean site_row_drag_possible(GtkTreeDragSource *drag_source, GtkTreePath *path)
{
  gboolean rval;

  rval=(*default_row_drag_possible)(drag_source, path);
  if(gtk_tree_path_get_depth(path)<2) rval=FALSE;
  return rval;
}

static gboolean site_row_drop_possible(GtkTreeDragDest *drag_dest,GtkTreePath *path,GtkSelectionData *selection_data)
{
  GtkTreePath *new_parent_path;
  gboolean rval;

  rval=(*default_row_drop_possible)(drag_dest, path, selection_data);
  new_parent_path=gtk_tree_path_copy(path);
  if(gtk_tree_path_get_depth(new_parent_path)>1) {
    gtk_tree_path_up(new_parent_path);
    if(!gtk_tree_path_compare(dnd_parent_path, new_parent_path)) rval=FALSE;  /* Same parent */
  }
  else rval = FALSE; /* No parent */
  gtk_tree_path_free(new_parent_path);
  return rval;
}

void site_tree_begin_drag(GtkWidget *widget,GdkDragContext *drag_context,gpointer user_data)
{
  GtkTreeIter iter;
  GtkTreeModel *model;

  model=gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(widget)),&model,&iter);
  dnd_parent_path=gtk_tree_model_get_path(model,&iter);
  gtk_tree_path_up(dnd_parent_path);
}

gboolean site_tree_drag_drop(GtkWidget * widget,GdkDragContext *drag_context,gint x,gint y,guint time,gpointer user_data)
{
  GtkTreePath *path = NULL;
  GtkTreeViewDropPosition pos;
  GtkTreeIter iter;
  GtkTreePath *current_site_path;
  GtkTreeModel *model;
  gint new_parent_id;
  
  gtk_tree_view_get_dest_row_at_pos(GTK_TREE_VIEW(widget),x,y,&path,&pos);
  model=gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  gtk_tree_model_get_iter(model,&iter,path);
  gtk_tree_model_get(model,&iter,
    SITETREE_COL_SITE_ID,&new_parent_id,
    -1
  );
  gtk_tree_path_free(dnd_parent_path);
  site_db_set_parent(current_site_id, new_parent_id);
  site_load(current_site_id);
  if(path) gtk_tree_path_free (path);
  return FALSE;
}

gboolean site_delete_event(GtkWidget *widget,GdkEvent * event, gpointer user_data)
{
  /* reset DND */
  GtkTreeDragSourceIface *src_iface;
  GtkTreeDragDestIface *dest_iface;
  GtkTreeModel *model;
  
  model=gtk_tree_view_get_model(GTK_TREE_VIEW(lookup_widget(site_window,"sitetree")));
  src_iface=GTK_TREE_DRAG_SOURCE_GET_IFACE(GTK_TREE_DRAG_SOURCE(model));
  src_iface->row_draggable=default_row_drag_possible;
  dest_iface=GTK_TREE_DRAG_DEST_GET_IFACE(GTK_TREE_DRAG_DEST(model));
  dest_iface->row_drop_possible=default_row_drop_possible;
  return FALSE;
}

/* End DND */

static const gchar *site_detail_get_site_name(void)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(site_window,"site_name"));
  return gtk_entry_get_text(GTK_ENTRY(widget));
}

static void site_detail_set_site_name(gchar *site_name)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(site_window,"site_name"));
  gtk_entry_set_text(GTK_ENTRY(widget),site_name);
}

static gchar *site_detail_get_site_notes(void)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(site_window,"site_notes"));
  return helper_gtk_text_view_get_text(GTK_TEXT_VIEW(widget));
}

static void site_detail_set_site_notes(gchar *site_notes)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(site_window,"site_notes"));
  helper_gtk_text_view_set_text(GTK_TEXT_VIEW(widget),site_notes);
}

static void site_set_new_sensitive(gboolean sensitive)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(site_window,"site_new_btn"));  
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static void site_set_save_sensitive(gboolean sensitive)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(site_window,"site_save_btn"));  
    gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static void site_set_delete_sensitive(gboolean sensitive)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(site_window,"site_delete_btn"));  
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static void site_set_select_sensitive(gboolean sensitive)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(site_window,"site_select_btn"));  
  gtk_widget_set_sensitive(GTK_WIDGET(widget),sensitive);
}

static site_load_detail_callback(gint *site_id, gint argc, gchar **argv,gchar **azColName)
{
  site_detail_set_site_name(argv[0]);
  site_detail_set_site_notes(argv[1]);
  if (site_db_delete_allowed(current_site_id)) site_set_delete_sensitive(TRUE);
  else site_set_delete_sensitive(FALSE);
  return 0;
}

static site_clear_tree(void)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(site_window,"sitetree"));
  gtk_tree_store_clear(GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget))));
}

static site_clear_parent_combo(void)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(site_window,"site_parent"));
  gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(widget))));
}

static void site_clear(void)
{

  site_clear_tree();
  site_clear_parent_combo();
  site_detail_set_site_name("");
  site_detail_set_site_notes("");
  site_set_new_sensitive(FALSE);
  site_set_save_sensitive(FALSE);
  site_set_delete_sensitive(FALSE);
  current_site_id=0;
}

static gint site_load_callback(SiteLoadData *site_load_data,gint argc,gchar **argv, gchar **azColName)
{
  gint site_id,site_parent_id,*key;
  GtkTreeIter *iter,*parent_iter,site_parent_combo_iter;
  
  site_id=strtol(argv[0],NULL,0);
  site_parent_id=strtol(argv[1],NULL,0);

  key=g_malloc(sizeof(gint));
  *key=site_id;
  iter=g_malloc(sizeof(GtkTreeIter*));
  g_hash_table_insert(site_load_data->hash_table,key,iter);

  if(!site_parent_id) parent_iter=NULL;
  else parent_iter=g_hash_table_lookup(site_load_data->hash_table,&site_parent_id);
  
  gtk_tree_store_append(GTK_TREE_STORE(site_load_data->site_tree_model),iter,parent_iter);
  gtk_tree_store_set(GTK_TREE_STORE(site_load_data->site_tree_model),iter,
    SITETREE_COL_SITE_NAME,argv[2],
    SITETREE_COL_SITE_ID,site_id,
    SITETREE_COL_SITE_PARENT_ID,site_parent_id,
    SITETREE_COL_SEQ_NUMBER,site_load_data->seq_number, /* Effectively the site parent combo index for same site */
    -1
  );
  site_load_data->seq_number++;
  if(site_id==site_load_data->select_id) {
    site_load_data->select_path=gtk_tree_model_get_path(site_load_data->site_tree_model,iter);
  }
  
  gtk_list_store_append(GTK_LIST_STORE(site_load_data->site_parent_combo_model),&site_parent_combo_iter);
  gtk_list_store_set(GTK_LIST_STORE(site_load_data->site_parent_combo_model),&site_parent_combo_iter,
    SITEPARENT_COL_SITE_FULL_NAME,argv[3],
    SITEPARENT_COL_SITE_ID,site_id,
    -1
  );
  return 0;
}

static void site_load(gint select_id)
{
  gint max_site_id, min_site_id, i;
  GtkWidget *widget,*sitetree;
  GtkTreeIter iter;
  SiteLoadData site_load_data;

  /* clear stores and fields */
  site_clear();

  /* Prep site_parent store */
  widget=GTK_WIDGET(lookup_widget(site_window,"site_parent"));
  site_load_data.site_parent_combo_model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
  gtk_list_store_append(GTK_LIST_STORE(site_load_data.site_parent_combo_model),&iter);
  gtk_list_store_set(GTK_LIST_STORE(site_load_data.site_parent_combo_model),&iter,
    SITEPARENT_COL_SITE_FULL_NAME, "",
    SITEPARENT_COL_SITE_ID,0, /* must have qualifier or will cause exception */
    -1
  );
  
  widget=GTK_WIDGET(lookup_widget(site_window,"sitetree"));
  site_load_data.site_tree_model=gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  site_load_data.select_id=select_id;
  site_load_data.hash_table=g_hash_table_new_full(g_int_hash,g_int_equal,g_free,g_free);
  site_load_data.select_path=NULL;
  site_load_data.seq_number=1;    
  site_db_sitetree_load_store(site_load_callback,(gpointer)&site_load_data);
  g_hash_table_destroy(site_load_data.hash_table);
  
  if(gtk_tree_model_iter_n_children(site_load_data.site_tree_model,NULL)) {
    if(!select_id) {
      site_load_data.select_path=gtk_tree_path_new_first();
      gtk_tree_view_expand_row(GTK_TREE_VIEW(widget),site_load_data.select_path,FALSE);
    }
    else {
      gtk_tree_view_expand_to_path(GTK_TREE_VIEW(widget),site_load_data.select_path);
    }
    if(site_load_data.select_path) {
      gtk_tree_view_set_cursor(GTK_TREE_VIEW(widget),site_load_data.select_path,NULL,FALSE);
      gtk_tree_path_free(site_load_data.select_path);
    }
  }
}

static gint site_get_site_parent_id(void)
{
  gint site_parent_id=0;
  GtkWidget *widget;
  GtkTreeModel *model;
  GtkTreeIter iter;

  widget=GTK_WIDGET(lookup_widget(site_window,"site_parent"));
  model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget),&iter)) {
    gtk_tree_model_get(model,&iter,
      SITEPARENT_COL_SITE_ID,
      &site_parent_id,
      -1
    );
  }
  return site_parent_id;
}

static const gchar *site_get_site_parent_name(void)
{
  GtkWidget *widget;
  
  widget=GTK_WIDGET(lookup_widget(site_window,"site_parent"));
  return gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget));
}

void site_show_window(gint select_id)
{
  GtkWidget *widget;
  GtkTreeModel *model;

  /* DND */
  GtkTreeDragSourceIface *src_iface;
  GtkTreeDragDestIface *dest_iface;
  static const GtkTargetEntry target_table[] = {
    {"GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0}
  };
  /* End DND */

  site_window=GTK_WIDGET(create_site_window());
  
  /* Create treeview and tree model */
  widget=GTK_WIDGET(lookup_widget(site_window,"sitetree"));
  gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(widget),-1,_("Site"),gtk_cell_renderer_text_new(),"text",SITETREE_COL_SITE_NAME,NULL);
  model=GTK_TREE_MODEL(gtk_tree_store_new(SITETREE_NUM_COLS,G_TYPE_STRING,G_TYPE_INT,G_TYPE_INT,G_TYPE_INT));
  gtk_tree_view_set_model(GTK_TREE_VIEW(widget),model);
  g_object_unref(model);

  /* DND */
  src_iface=GTK_TREE_DRAG_SOURCE_GET_IFACE(GTK_TREE_DRAG_SOURCE(model));
  default_row_drag_possible=src_iface->row_draggable;
  src_iface->row_draggable=site_row_drag_possible;
  dest_iface=GTK_TREE_DRAG_DEST_GET_IFACE(GTK_TREE_DRAG_DEST(model));
  default_row_drop_possible=dest_iface->row_drop_possible;
  dest_iface->row_drop_possible=site_row_drop_possible;
  gtk_tree_view_enable_model_drag_source(
    GTK_TREE_VIEW(widget),
    GDK_BUTTON1_MASK,
    target_table,
    G_N_ELEMENTS(target_table),
    GDK_ACTION_MOVE
  );
  gtk_tree_view_enable_model_drag_dest(
    GTK_TREE_VIEW(widget),
    target_table,
    G_N_ELEMENTS(target_table),
    GDK_ACTION_MOVE
  );
  /* End DND */
  
  /* Create combo model etc */
  widget=GTK_WIDGET(lookup_widget(site_window,"site_parent"));
  model=GTK_TREE_MODEL(gtk_list_store_new(SITEPARENT_NUM_COLS,G_TYPE_STRING,G_TYPE_INT));
  gtk_combo_box_set_model(GTK_COMBO_BOX(widget),model);
  g_object_unref(model);
  gtk_window_set_transient_for(GTK_WINDOW(site_window),GTK_WINDOW(main_window));
  gtk_widget_show(site_window);
  site_load(select_id);
}

void site_tree_cursor_changed(GtkTreeView *treeview,gpointer user_data)
{
  gint seq_number=0;
  GtkTreeModel *model;
  GtkTreeIter iter,parent_iter;
  GtkWidget *widget;
  
  model=gtk_tree_view_get_model(treeview);
  gtk_tree_selection_get_selected(gtk_tree_view_get_selection(treeview),&model,&iter);
  gtk_tree_model_get(model,&iter,SITETREE_COL_SITE_ID,&current_site_id,-1);
  if(current_site_id) {
    site_db_load_detail(current_site_id,(gpointer)site_load_detail_callback);
    site_set_save_sensitive(TRUE);
    site_set_select_sensitive(TRUE);
  }
  if(gtk_tree_model_iter_parent(model,&parent_iter,&iter)) gtk_tree_model_get(model,&parent_iter,SITETREE_COL_SEQ_NUMBER,&seq_number,-1);
  widget=GTK_WIDGET(lookup_widget(site_window, "site_parent"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(widget),seq_number);
}

void site_save(GtkButton *button, gpointer user_data)
{
  gchar *site_name,*site_notes;
  
  site_name=(gchar*)site_detail_get_site_name();
  site_notes=site_detail_get_site_notes();
  if(site_db_update(current_site_id,site_get_site_parent_id(),(gchar*)site_detail_get_site_name(),site_notes)) site_load(current_site_id);
  else g_printerr (_("'%s/%s' already exists."),site_get_site_parent_name(),site_name);
  g_free (site_notes);
}

void site_new(GtkButton *button, gpointer user_data)
{
  gchar *site_name,*site_notes;
  gint site_id;
  GtkTreeIter iter;

  site_name=(gchar*)site_detail_get_site_name();
  site_notes=site_detail_get_site_notes();
  site_id=site_db_new(
    site_get_site_parent_id(),
    site_name,
    site_notes
  );
  if(site_id) {
    site_load(site_id);
  }
  else g_printerr (_("'%s/%s' already exists."),site_get_site_parent_name(),site_name);
  g_free(site_notes);
}

void site_delete(GtkButton *button, gpointer user_data)
{
  gint site_parent_id;

  site_parent_id=site_get_site_parent_id();
  site_db_delete(current_site_id);
  site_load(site_parent_id);
}

void site_select(GtkButton *button, gpointer user_data)
{
  dive_site_set(current_site_id);
  site_delete_event(NULL,NULL,NULL);
  gtk_widget_destroy(site_window);
}

void site_cancel(GtkButton *button, gpointer user_data)
{
  site_delete_event(NULL,NULL,NULL);
  gtk_widget_destroy(site_window);
}

void site_name_changed(GtkEditable *editable,gpointer user_data)
{
  site_set_save_sensitive(FALSE);
  site_set_new_sensitive(FALSE);
  if(strlen(gtk_entry_get_text(GTK_ENTRY(editable)))) {
    if(current_site_id) site_set_save_sensitive(TRUE);
    site_set_new_sensitive(TRUE);
  }
}

void site_parent_changed(GtkComboBox *combobox,gpointer user_data)
{
  gint site_parent_id;
  GtkTreeModel *model;
  GtkTreeIter iter;
  
  model=gtk_combo_box_get_model(combobox);
  gtk_combo_box_get_active_iter(combobox,&iter);
  gtk_tree_model_get(model,&iter,
    SITEPARENT_COL_SITE_ID,
    &site_parent_id,
    -1
  );
  if((site_parent_id==current_site_id)||site_db_isancestor(site_parent_id,current_site_id))
    site_set_save_sensitive(FALSE);
}
