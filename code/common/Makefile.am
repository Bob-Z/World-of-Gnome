CC=clang

noinst_LIBRARIES=libcommon.a
libcommon_a_SOURCES=util.c context.c network.c map.c entry.c log.c mutex.c item.c file.c list.c fifo.c
libcommon_a_LIBADD=$(ADD_LIB)
libcommon_a_CFLAGS = $(AM_CFLAGS)

dist_noinst_HEADERS = util.h const.h syntax.h protocol.h context.h common.h network.h map.h entry.h log.h mutex.h item.h file.h list.h syntax.h fifo.h
