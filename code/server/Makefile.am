CC=clang

bin_PROGRAMS=wog_server
wog_server_SOURCES=main_server.c parser_server.c character.c server.c npc.c action.c equipment.c inventory.c attribute.c network_server.c
wog_server_LDADD=../common/libcommon.a $(ADD_LIB)

dist_noinst_HEADERS = parser.h character.h npc.h action.h equipment.h inventory.h attribute.h network_server.h
