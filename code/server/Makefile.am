bin_PROGRAMS=wog_server
wog_server_SOURCES=main.c parser.c character.c server.c npc.c action.c equipment.c inventory.c attribute.c item.c
wog_server_LDADD=../common/libcommon.a $(ADD_LIB)

dist_noinst_HEADERS = parser.h character.h npc.h action.h equipment.h inventory.h attribute.h item.h

#  uncomment the following if mgeftl requires the math library
#mgeftl_LDADD=-lm
mgeftl_CFLAGS = $(AM_CFLAGS) $(LIBUSB_CFLAGS)

#  if you write a self-test script named `chk', uncomment the
#  following and add `chk' to the EXTRA_DIST list
#TESTS=chk
