bin_PROGRAMS=wog_client
wog_client_SOURCES=main.c parser.c file.c imageDB.c client.c textview.c sdl.c item.c anim.c scr_select.c screen.c scr_play.c scr_inventory.c

AV_LIB=-lavcodec -lavformat -lswscale
SDL_LIB=`sdl2-config --libs`

wog_client_LDADD=../common/libcommon.a $(ADD_LIB)

wog_clientdir = $(datadir)/wog/client
wog_client_DATA = login.glade select_character.glade game.glade

dist_noinst_HEADERS = parser.h file.h imageDB.h sdl.h item.h anim.h screen.h scr_select.h scr_play.h scr_inventory.h

#  uncomment the following if tools requires the math library
#tools_LDADD=-lm

#  if you write a self-test script named `chk', uncomment the
#  following and add `chk' to the EXTRA_DIST list
#TESTS=chk
