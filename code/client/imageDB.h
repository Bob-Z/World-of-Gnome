void imageDB_init();
void imageDB_add_file(context_t * context, gchar * filename, GtkWidget * widget);
GtkWidget * imageDB_get_widget(context_t * context, const gchar * image_name);
GtkWidget * imageDB_check_widget(gchar * file_name);
gboolean image_DB_update(context_t * context,gchar * filename);
extern gboolean updated_media;
