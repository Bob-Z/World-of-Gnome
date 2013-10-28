#ifndef MAP_H
#define MAP_H

gchar * map_delete_item(const gchar * map, gint x, gint y);
gint map_add_item(const gchar * map, const gchar * item, gint x, gint y);
gboolean map_check_tile(context_t * context,gchar * map,gint x,gint y);
int map_set_tile(const gchar * map,const gchar * tile,gint x, gint y);
gchar * map_get_tile(const gchar * map,gint x, gint y);
const gchar ** map_get_event(const gchar * map,gint x, gint y);
gint map_add_event(const gchar * map, const gchar * script, gint x, gint y);
gint map_delete_event(const gchar * map, const gchar * script, gint x, gint y);
#endif

