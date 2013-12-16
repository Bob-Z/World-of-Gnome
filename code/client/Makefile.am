bin_PROGRAMS=wog_client
wog_client_SOURCES=main.c parser.c win_login.c file.c imageDB.c client.c action.c inventory.c textview.c equipment.c sdl.c item.c anim.c scr_select.c screen.c

wog_client_LDADD=../common/libcommon.a /usr/lib/libconfig.a $(ADD_LIB)

wog_clientdir = $(datadir)/wog/client
wog_client_DATA = login.glade select_character.glade game.glade

dist_noinst_HEADERS = parser.h win_login.h file.h imageDB.h action.h inventory.h equipment.h sdl.h item.h anim.h screen.h scr_select.h

#  uncomment the following if tools requires the math library
#tools_LDADD=-lm

#  if you write a self-test script named `chk', uncomment the
#  following and add `chk' to the EXTRA_DIST list
#TESTS=chk
