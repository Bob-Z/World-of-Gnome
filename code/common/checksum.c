#include <glib.h>
#include <gio/gio.h>
#include <common.h>


/* Return a string representing the checksum of the file or NULL on error */
/* filename is the directory + name */
/* The returned string MUST be FREED */

gchar * checksum_file(const gchar * filename)
{
        gchar * file_data = NULL;
        gsize file_length = 0;
        gboolean res = file_get_contents(filename,&file_data,&file_length,NULL);

        if( res == FALSE) {
                return NULL;
        }

	gchar * checksum = NULL;

	checksum = g_compute_checksum_for_data ( G_CHECKSUM_MD5,(guchar *)file_data,file_length);

	g_free(file_data);

	return checksum;
}
