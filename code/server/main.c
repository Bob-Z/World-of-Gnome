#include <glib.h>
#include <gio/gio.h>
#include "../common/common.h"
#include <signal.h>
#include <stdlib.h>
#include <npc.h>

void sigint_handler(int sig)
{
	file_dump_all_to_disk();
	printf("Exiting\n");
	exit(0);
}

/**************************
  main
 **************************/

int main (int argc, char **argv)
{
	g_type_init();
	g_thread_init(NULL);

	//init the main loop
	GMainLoop * mainLoop = NULL;
	mainLoop = g_main_loop_new(NULL,FALSE);

        if(argc == 2) {
                gint log_level = g_ascii_strtoll(argv[1],NULL,10);
                init_log(log_level);
        }

	//init non playing character
	init_npc();
	//init network server
	network_init();

	signal(SIGINT,sigint_handler);

	/* Run main loop */
	g_main_loop_run(mainLoop);

	return 0;
}
