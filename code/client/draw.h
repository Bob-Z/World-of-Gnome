#include <gtk/gtk.h>

extern gint main_window_width;
extern gint main_window_height;

void draw_sprite(context_t * context, GtkWidget * tile_set, gint tile_x, gint tile_y);
void draw_all_sprite(GtkWidget * tile_set);
void draw_cleanup(context_t * context);
void draw_map(context_t * context, GtkWidget * tile_set);
void draw_item(context_t * context, GtkWidget * tile_set);
void draw_cursor(context_t * context, GtkWidget * tile_set);
void update_selected_character(context_t * context);
