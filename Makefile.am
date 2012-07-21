bin_PROGRAMS = bluez_traffic
bluez_traffic_SOURCES = src/main.c src/control.c src/packet.c \
			src/diagram.c src/UI.c src/util.c src/event.c \
			src/control.h src/packet.h src/diagram.h\
			src/UI.h src/util.h src/event.h
AM_CPPFLAGS = $(GLIB_CFLAGS) $(GTK_CFLAGS) $(BLUEZ_CFLAGS) $(CRCANVAS_CFLAGS) -w
bluez_traffic_LDADD = $(GLIB_LIBS) $(GTK_LIBS) $(BLUEZ_LIBS) $(CRCANVAS_LIBS)

