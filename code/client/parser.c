/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013 carabobz@gmail.com

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include <glib.h>
#include <gio/gio.h>
#include "../common/common.h"
#include <gtk/gtk.h>
#include "win_login.h"
#include "win_select_character.h"
#include "file.h"
#include "imageDB.h"
#include "win_game.h"
#include "textview.h"
#include "action.h"
#include "draw.h"

/* parse_incoming_data
Return FALSE on error, TRUE if OK */
gboolean parse_incoming_data(context_t * context, guint32 command, guint32 command_size, gchar * data)
{
        gchar * filename = NULL;

	if(context == NULL) {
		hide_select_character_window();
		hide_game_window();
		show_login_window();
	}

        switch(command) {
                case CMD_LOGIN_OK :
                        wlog(LOGDEBUG,"Received CMD_LOGIN_OK");
			if(!network_open_data_connection(context)) {
				return FALSE;
			}
			context_set_connected(context, TRUE);
                        wlog(LOGUSER,"Successfully connected");
			hide_login_window();
			show_select_character_window(context);
			network_request_user_character_list(context);
			wlog(LOGDEBUG,"Character list requested");
                        break;
                case CMD_LOGIN_NOK :
                        wlog(LOGDEBUG,"Received CMD_LOGIN_NOK");
			context_set_connected(context, FALSE);
			/* message box */
			gdk_threads_enter();
			GtkWidget * dialog;
			dialog = gtk_message_dialog_new (NULL,
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_CLOSE,
					"Authentication failure");
			gtk_message_dialog_format_secondary_text ( GTK_MESSAGE_DIALOG(dialog), "Check your login and password (they are case sensitive)");
			gtk_dialog_run (GTK_DIALOG (dialog));
	                gtk_widget_destroy (dialog);
			gdk_threads_leave();

                        break;
                case CMD_SEND_CHARACTER :
                        wlog(LOGDEBUG,"Received CMD_SEND_CHARACTER");
			wlog(LOGUSER,"New character : %s", data);
                        break;
                case CMD_SEND_FILE :
                        wlog(LOGDEBUG,"Received CMD_SEND_FILE");
			file_add(data,command_size,&filename);

			/* Special case for image file: try to update the widget data base */
			/* try to read the file as if it was an image */
			updated_media = image_DB_update(context,filename);
			g_free(filename);

			/* Asynchronous update of the select character window */
			update_select_character_window(context);
			/* Asynchronous update of the game window */
			redraw_window();

			updated_media = FALSE;
                        break;
                case CMD_SEND_USER_CHARACTER :
                        wlog(LOGDEBUG,"Received CMD_SEND_USER_CHARACTER");
			add_user_character(context,data);
			update_select_character_window(context);
                        break;
                case CMD_SEND_CONTEXT :
                        wlog(LOGDEBUG,"Received CMD_SEND_CONTEXT");
			context_add_or_update_from_network_frame(context,data);
			redraw_window();
                        break;
		case CMD_SEND_TEXT :
                        wlog(LOGDEBUG,"Received CMD_SEND_TEXT");
			textview_add_line(data);
			break;
		case CMD_SEND_ENTRY :
                        wlog(LOGDEBUG,"Received CMD_SEND_ENTRY");
			if( entry_update(data) != -1 ) {
				redraw_window();
			}
			break;
                default:
                        werr(LOGDEV,"Unknown request from server");
			return FALSE;
			break;
        }

        return TRUE;
}

