#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include "../common/common.h"
#include <dirent.h>
#include <string.h>
#include "action.h"

static GStaticMutex item_mutex = G_STATIC_MUTEX_INIT;

/* return the name of an available empty file */
/* if success the file is created on disk */
/* the returned string must be freed by caller */
static gchar * get_new_file()
{
        GDir * dir;
        const gchar * name;
        gchar * dirname;
        gchar * filename;
	gchar tag[7];
	gint index = 0;
	GFile * file;
	GFileOutputStream * file_stream;

	g_static_mutex_lock(&item_mutex);
	g_sprintf(tag,"A%05x",index);

        dirname = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", ITEM_TABLE,  NULL);
        dir = g_dir_open(dirname,0,NULL);

        while(( name = g_dir_read_name(dir)) != NULL ) {
		if( g_strcmp0(name,".") == 0 ) continue;
		if( g_strcmp0(name,"..") == 0 ) continue;
		if( g_strcmp0(name,tag) == 0 ) {
			index++;
			g_sprintf(tag,"A%05x",index);
			g_dir_rewind (dir);
			continue;
		}
	}

	filename = g_strconcat(dirname,"/",tag,NULL);
	file = g_file_new_for_path(filename);
	file_stream = g_file_create(file,G_FILE_CREATE_NONE,NULL,NULL);

	g_static_mutex_unlock(&item_mutex);

	g_object_unref(file_stream);
	g_object_unref(file);

	g_free(filename);
	g_free(dirname);

	return g_strdup(tag);
}

/*****************************/
/* Create an empty new item */
/* return the id of the newly created item */
/* the returned string must be freed by caller */
/* return NULL if fails */
gchar * item_create_empty()
{
	return get_new_file();
}

/*****************************/
/* Create a new item based on the specified template */
/* return the id of the newly created item */
/* the returned string must be freed by caller */
/* return NULL if fails */
gchar * item_create_from_template(const gchar * template)
{
	gchar * new_name;
        gchar * templatename;
        gchar * newfilename;
        GFile * templatefile;
        GFile * newfile;

	new_name = get_new_file();

        templatename = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", ITEM_TEMPLATE_TABLE, "/", template,  NULL);
	templatefile = g_file_new_for_path(templatename);

        newfilename = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", ITEM_TABLE, "/", new_name,  NULL);
	newfile = g_file_new_for_path(newfilename);

	if( g_file_copy(templatefile,newfile, G_FILE_COPY_OVERWRITE,NULL,NULL,NULL,NULL) == FALSE ) {
		g_free(new_name);
		return NULL;
	}

	return new_name;
}

/* Remove an item file */
/* return -1 if fails */
gint item_destroy(const gchar * item_id)
{
        gchar * filename;

        filename = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", ITEM_TABLE, "/", item_id, NULL);

	g_remove(filename);

        return 0;
}
