#ifndef ACTION_SERVER_H
#define ACTION_SERVER_H
#include "../common/common.h"

void action_parse_frame(context_t * context, gchar * frame);
gint action_execute_script(context_t * context, const gchar * script, gchar ** parameters);
void register_lua_functions(context_t * context);

#endif

