#ifndef ENTRY_H
#define ENTRY_H

#include <glib.h>

int read_int(const gchar * table, const gchar * file, int * res, ...);
int read_string(const gchar * table, const gchar * file, const gchar ** res, ...);
int read_list_index(const gchar * table, const gchar * file, const gchar ** res,gint index, ...);
int read_list(const gchar * table, const gchar * file, gchar *** res, ...);

int write_int(const gchar * table, const gchar * file, int data, ...);
int write_string(const gchar * table, const gchar * file,const char * data, ...);
int write_list_index(const gchar * table, const gchar * file, const char * data,gint index, ...);
int write_list(const gchar * table, const gchar * file, char ** data, ...);

gchar * get_unused_list_entry(const gchar * table, const gchar * file, ...);
gchar * get_unused_group(const gchar * table, const gchar * file, ...);
int get_group_list(const gchar * table, const gchar * file, gchar *** res, ...);

gboolean file_get_contents(const gchar *filename,gchar **contents,gsize *length,GError **error);
gboolean file_set_contents(const gchar *filename,const gchar *contents,gssize length,GError **error);

gboolean add_to_list(const gchar * table, const gchar * file, const gchar * to_be_added, ...);
int remove_group(const gchar * table, const gchar * file, const gchar * group, ...);

void file_dump_all_to_disk(void);

gboolean remove_from_list(const gchar * table, const gchar * file, const gchar * to_be_removed, ...);

gchar * copy_group(const gchar * src_table, const gchar * src_file, const gchar * dst_table, const gchar * dst_file, const gchar * group_name, ...);
gint entry_update(gchar * data);
gint entry_destroy(const gchar * id);
#endif
