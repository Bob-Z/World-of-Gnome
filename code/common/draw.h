#ifndef DRAW_H
#define DRAW_H

#include <gtk/gtk.h>

extern gint main_window_width;
extern gint main_window_height;

void draw_sprite(context_t * context, GtkWidget * tile_set, gint tile_x, gint tile_y);
#endif
