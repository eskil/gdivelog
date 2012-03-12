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

#ifndef GLOBALS_H
#define GLOBALS_H

#include "defines.h"

#ifdef LOCAL
#define EXTERN
#else
#define EXTERN extern
#endif

gboolean prompt_message(gchar *message, GtkMessageType message_type);
#define prompt_message_info(x) prompt_message(x,GTK_MESSAGE_INFO)
#define prompt_message_warning(x) prompt_message(x,GTK_MESSAGE_WARNING)
#define prompt_message_question(x) prompt_message(x,GTK_MESSAGE_QUESTION)
#define prompt_message_error(x) prompt_message(x,GTK_MESSAGE_ERROR)
gboolean check_create_data_dir(void);
gboolean logbook_open_prompt(void);
gboolean logbook_new(void);
gboolean logbook_save_do(gboolean for_close);
gboolean logbook_close_do(gboolean prompt_save);

#define logbook_open(x) logbook_open_do(x,TRUE)
#define logbook_open_last_opened(x) logbook_open_do(x,FALSE)
#define logbook_save() logbook_save_do(FALSE)
#define logbook_save_for_close() logbook_save_do(TRUE)
#define logbook_close() logbook_close_do(TRUE)

EXTERN GtkWidget *main_window;
EXTERN Preferences preferences;

gint current_dive_id;

#endif /* GLOBALS_H */
