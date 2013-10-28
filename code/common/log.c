#ifndef LOG_C
#define LOG_C

#include <glib.h>
#include <glib/gprintf.h>

static gint level;

void my_log_handler(const gchar *log_domain,
                    GLogLevelFlags log_level,
                    const gchar *message,
                    gpointer user_data) {

	if( level == 0 ) {
		if( log_level <= G_LOG_LEVEL_CRITICAL ) {
//			g_log_default_handler(log_domain,log_level,message,user_data);
			printf("%04X | %s\n",log_level,message);
		}
		return;
	}
	if( level == 1 ) {
		if( log_level <=  G_LOG_LEVEL_WARNING ) {
//			g_log_default_handler(log_domain,log_level,message,user_data);
			printf("%04X | %s\n",log_level,message);
		}
		return;
	}

	if( level == 2 ) {
		if( log_level <=  G_LOG_LEVEL_MESSAGE ) {
//			g_log_default_handler(log_domain,log_level,message,user_data);
			printf("%04X | %s\n",log_level,message);
		}
		return;
	}

	if( level >= 3 ) {
		if( log_level <=  G_LOG_LEVEL_DEBUG ) {
//			g_log_default_handler(log_domain,log_level,message,user_data);
			printf("%04X | %s\n",log_level,message);
		}
		return;
	}

	g_printf("LOG ERROR\n");
}

void init_log(gint log_level)
{
	level = log_level;
	g_log_set_handler (NULL, G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL
                   | G_LOG_FLAG_RECURSION, my_log_handler, NULL);
}
#endif
