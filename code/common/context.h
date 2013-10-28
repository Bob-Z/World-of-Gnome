#include <gtk/gtk.h>
#include <glib.h>
#include <gio/gio.h>
#include <lua.h>

typedef struct selection {
	gchar *		id;	/* a character id */
	gint		map_coord[2];	/* a tile map */
	gchar *		map;
	gchar *		inventory;	/* name of the selected item in inventory */
	gchar *		equipment;	/* name of the selected slot in equipment */
} selection_t;

typedef struct context {
        gchar *         user_name;
        gboolean        connected;
	GSocketConnection * connection;
	GInputStream * 	input_stream;
	GOutputStream * output_stream;
	GSocketConnection * data_connection;
	GInputStream * 	input_data_stream;
	GOutputStream * output_data_stream;
	gchar *		hostname;
	GThreadPool *	send_thread;
	GMutex		send_mutex;

	gchar *		avatar_name;
	gchar *		map;	/* map name */
	gint		map_x;	/* map size */
	gint		map_y;	/* map size */
	gint		tile_x; /* size of a tile for the current map */
	gint		tile_y; /* size of a tile for the current map */
	gint		pos_x;	/* player position */
	gint		pos_y;	/* player position */
	gchar *		type;	/* avatar's type */
	GtkWidget * 	sprite_image;
	selection_t 	selection; /* Selected tile or sprite */
	gchar *		id; /* unique ID of a character (its filename) */
	gchar *		prev_map; /* the map from where this context comes */
	lua_State*	luaVM; /* LUA state */
	GCond*		cond;	/* async condition for npc */
	GMutex*		cond_mutex;/* mutex for async condition for npc */
	
	struct context*	previous;
	struct context*	next;
} context_t;


void context_init(context_t * context);
context_t * context_new(void);
void context_free(context_t * context);
gboolean context_set_username(context_t * context, const gchar * name);
void context_set_connected(context_t * context, gboolean connected);
gboolean context_get_connected(context_t * context);
void context_set_input_stream(context_t * context, GInputStream * stream);
GInputStream * context_get_input_stream(context_t * context);
void context_set_output_stream(context_t * context, GOutputStream * stream);
GOutputStream * context_get_output_stream(context_t * context);
void context_set_connection(context_t * context, GSocketConnection * connection);
GSocketConnection * context_get_connection(context_t * context);
gboolean context_set_avatar_name(context_t * context, const gchar * name);
gboolean context_set_map(context_t * context, const gchar * name);
gboolean context_set_type(context_t * context, const gchar * name);
void context_set_pos_x(context_t * context, guint pos);
void context_set_pos_y(context_t * context, guint pos);
void context_set_tile_x(context_t * context, guint pos);
void context_set_tile_y(context_t * context, guint pos);
void context_set_map_x(context_t * context, guint pos);
void context_set_map_y(context_t * context, guint pos);
void context_new_VM(context_t * context);
gboolean context_set_id(context_t * context, const gchar * name);
gboolean context_update_from_file(context_t * context);
gboolean context_update_from_network_frame(context_t * context, gchar * frame);
void context_update_from_context(context_t * remote_context);
void context_spread(context_t * context);
void context_add_or_update_from_network_frame(context_t * context, gchar * data);
void context_lock_list();
void context_unlock_list();
context_t * context_get_list_first();
gboolean context_write_to_file(context_t * context);
void context_broadcast_file(const gchar * table, const gchar * file, gboolean same_map_only);
void context_request_other_context(context_t * context);
context_t * context_find(const gchar * id);
void context_broadcast_text(const gchar * map, const gchar * text);
gint context_distance(context_t * ctx1, context_t * ctx2);
