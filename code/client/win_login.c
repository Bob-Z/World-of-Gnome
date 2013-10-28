#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gprintf.h>
#include "config.h"
#include "../common/common.h"

static GtkWidget *              login_window;
static GtkWidget *              entry_name;
static GtkWidget *              entry_pass;
static GtkWidget *              entry_ip;

/*************************
 show window
*************************/
void show_login_window(void)
{
	gdk_threads_enter();
	gtk_widget_show_all(login_window);
	gdk_threads_leave();
}

/*************************
 hide window
*************************/
void hide_login_window(void)
{
	gdk_threads_enter();
	gtk_widget_hide_all(login_window);
	gdk_threads_leave();
}

/*************************
 Set entry
*************************/
void win_login_set_entry(gchar * ip, gchar * name, gchar * pass)
{
	gdk_threads_enter();
	if(ip != NULL) {
        	gtk_entry_set_text (GTK_ENTRY(entry_ip),ip);
	}

	if(name != NULL) {
        	gtk_entry_set_text (GTK_ENTRY(entry_name),name);
	}
	if(pass != NULL) {
        	gtk_entry_set_text (GTK_ENTRY(entry_pass),pass);
	}
	gdk_threads_leave();
}


/*************************
 Login button clicked
*************************/
void on_login_clicked (GtkButton *button, gpointer   user_data)
{
        const gchar * name;
        const gchar * pass;
        const gchar * ip;

	context_t * context = (context_t *)user_data;

        name = gtk_entry_get_text (GTK_ENTRY(entry_name));
        pass = gtk_entry_get_text (GTK_ENTRY(entry_pass));
        ip = gtk_entry_get_text (GTK_ENTRY(entry_ip));

        if( network_connect(context,ip) ) {
	        network_login(context, name, pass);
	}
	else {
		GtkWidget * dialog;
		dialog = gtk_message_dialog_new (GTK_WINDOW(login_window),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				"Can't connect to server");
		gtk_message_dialog_format_secondary_text ( GTK_MESSAGE_DIALOG(dialog), "Check server IP address. This error may be due to a service outage on server side. Re-try in a few seconds");
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}

	context->user_name = g_strdup(name);
}

/****************
return FALSE on error
****************/

gboolean win_login_init(context_t * context)
{
        GtkBuilder * builder = NULL;
	gdk_threads_enter();
        builder = gtk_builder_new ();

        gchar tmp[SMALL_BUF];
       	g_snprintf(tmp,SMALL_BUF,"./login.glade");
        if( ! gtk_builder_add_from_file (builder, tmp,NULL)) {
        	g_snprintf(tmp,SMALL_BUF,"%s/wog/client/login.glade",DATADIR);
        	if( ! gtk_builder_add_from_file (builder, tmp,NULL)) {
               		g_print("Error while loading UI file login.glade\n");
			gdk_threads_leave();
			return FALSE;
		}
        }

        login_window = GTK_WIDGET(gtk_builder_get_object(builder,"login_window"));
        entry_name   = GTK_WIDGET(gtk_builder_get_object(builder,"entry_name"));
        entry_pass   = GTK_WIDGET(gtk_builder_get_object(builder,"entry_pass"));
        entry_ip     = GTK_WIDGET(gtk_builder_get_object(builder,"entry_ip"));

        gtk_builder_connect_signals (builder, context);
        g_object_unref (G_OBJECT (builder));

	gtk_widget_show_all(login_window);
	gdk_threads_leave();

	return TRUE;
}

