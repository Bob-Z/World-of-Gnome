CC=clang

bin_PROGRAMS=wog_client
wog_client_SOURCES=main_client.c parser_client.c file_client.c imageDB.c client.c textview.c scr_select.c screen.c scr_play.c network_client.c ui_play.c option_client.c

wog_client_LDADD=../common/libcommon.a ../sdl_item/libsdl_item.a $(ADD_LIB)

wog_clientdir = $(datadir)/wog/client

dist_noinst_HEADERS = parser.h file.h imageDB.h screen.h scr_select.h scr_play.h network_client.h ui_play.h option_client.h
