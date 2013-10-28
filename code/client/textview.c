#include <gtk/gtk.h>
#include "../common/common.h"

extern GtkWidget *             textview;
extern GtkTextBuffer *         textbuffer;

void textview_add_line(gchar * string) {

	GtkTextIter iter;

	gtk_text_buffer_get_start_iter(textbuffer,&iter);
	gtk_text_buffer_insert(textbuffer,&iter,string,-1);
}
