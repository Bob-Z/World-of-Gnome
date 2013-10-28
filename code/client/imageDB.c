/* This manage an image data base */
/* When a thread need an image, it create a GtkWidget containing the image, then it adds an entry in the image data base (a hash table) with the name of the file containing the image on the disk (relative to $HOME/.config/wog/client/data) as the key and the pointer to the GtkWidget as the value.
When a file is received from the server, the parser's thread check the image data base and if the received file is an entry in the image data base, it update the corresponding GtkImage*/

#include <gtk/gtk.h>
#include "../common/common.h"

gboolean updated_media = FALSE;

static GHashTable* imageDB;

static void free_key(gpointer data)
{
	g_free(data);
}

static void free_value(gpointer data)
{
	g_free(data);
}

void imageDB_init()
{
	/* key is a string : it is the filename of the image */
	imageDB = g_hash_table_new_full(g_str_hash,g_str_equal, free_key, free_value);
}

void imageDB_add_file(context_t * context, gchar * filename, GtkWidget * widget)
{
	if( g_hash_table_lookup(imageDB, filename) ) {
		return;
	}

	gchar * name = g_strdup(filename);
	GtkWidget ** w = g_malloc(sizeof(GtkWidget *));
	*w = widget;
	g_hash_table_replace(imageDB, name, w);

	/* request an update to the server */
	network_send_req_file(context,filename);
}

/* Return a copy of the image which must be destroyed (gtk_widget_destroy) */
/* image_name is the image file name without any path (e.g. : marquee.jpg) */
GtkWidget * imageDB_get_widget(context_t * context, const gchar * image_name)
{
	GtkWidget ** w;
	gchar * tmp;
	gchar * filename;
	GdkPixbufAnimation * pixbuf;
	GtkWidget *new_widget;

	filename = g_strconcat( IMAGE_TABLE, "/", image_name , NULL);
	w = g_hash_table_lookup(imageDB, filename);
	if( w != NULL ) {
		g_free(filename);
		return gtk_image_new_from_animation(gtk_image_get_animation(GTK_IMAGE(*w)));
	}

	/* The image was not in the imageDB */
	/* try to read the file currently available */
	tmp = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", IMAGE_TABLE, "/",  image_name , NULL);
	pixbuf = gdk_pixbuf_animation_new_from_file(tmp,NULL);
	if( pixbuf == NULL ) {
		/* Use the default image */
		g_free(tmp);
		tmp = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", IMAGE_TABLE, "/",  DEFAULT_IMAGE_FILE , NULL);
		pixbuf = gdk_pixbuf_animation_new_from_file(tmp,NULL);
	}
	g_free(tmp);
	
	new_widget = gtk_image_new_from_animation(pixbuf);

	/* Set image_name in the imageDB to be updated when ready */
	imageDB_add_file(context, filename, new_widget);

	/* return a copy of the default image file */
	return gtk_image_new_from_animation(gtk_image_get_animation(GTK_IMAGE(new_widget)));
}

/* return NULL if filename is NOT in the DB 
   else return the widget coresponding to this
   file (not of copy of it !) */
/* image_name is the image file name without any path (e.g. : marquee.jpg) */
GtkWidget * imageDB_check_widget(gchar * image_name)
{
        GtkWidget ** w;
	gchar * name;

	name =  g_strconcat( IMAGE_TABLE, "/", image_name , NULL);

        w = g_hash_table_lookup(imageDB, name);
	g_free(name);
        if( w != NULL ) {
                return *w;
        }

	return NULL;
}

/* Update the database with a new file */
/* filename is the path relative to base directory ( e.g.: images/marquee.jpg ) */
/* return true if there is an update */
gboolean image_DB_update(context_t * context,gchar * filename) {
        gchar * full_name;
        gchar * image_name = filename;
	gboolean updated = FALSE;

        /* search the / chararcter */
        while( image_name[0] != '/' && image_name[0] != 0) {
                image_name++;
        }

        if( image_name[0] == '/' ) {
                image_name++;
        }
        else {
                g_warning("%s: invalid filename ( %s )",__func__,filename);
                return FALSE;
        }

        full_name = g_strconcat( g_getenv("HOME"),"/", base_directory, "/",filename, NULL);

        GdkPixbufAnimation * pixbuf = gdk_pixbuf_animation_new_from_file(full_name,NULL);
        if( pixbuf ) {
		updated = TRUE;
                /* It is actually an image, so try to update the widget DB */
                GtkWidget * image = imageDB_check_widget(image_name);
                if( image != NULL ) {
                        /* The image exists in the DB, update the pixbuf */
                        gdk_threads_enter();
                        gtk_image_set_from_animation(GTK_IMAGE(image),pixbuf);
                        gdk_threads_leave();
                }
                g_object_unref(pixbuf);
        }

        g_free(full_name);

	return updated;
}

