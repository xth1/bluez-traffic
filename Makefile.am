bin_PROGRAMS = bluez_traffic
bluez_traffic_SOURCES = src/main.c src/control.c src/packet.c \
			src/diagram.c src/UI.c src/util.c src/data_dumped.c \
			src/filter.c src/parser.c \
			src/diagram_elements/event_diagram.c \
			src/diagram_elements/device_diagram.c \
			src/diagram_elements/link_diagram.c \
			src/UI_components/device_filter_dialog.c \
			src/UI_components/event_detail.c\
			src/control.h src/packet.h src/diagram.h \
			src/UI.h src/util.h src/data_dumped.h \
			src/filter.h src/parser.h \
			src/diagram_elements/event_diagram.h \
			src/diagram_elements/device_diagram.h \
			src/diagram_elements/link_diagram.h \
			src/UI_components/device_filter_dialog.h\
			src/UI_components/event_detail.h

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GTK_CFLAGS) $(BLUEZ_CFLAGS) $(CRCANVAS_CFLAGS) -w
bluez_traffic_LDADD = $(GLIB_LIBS) $(GTK_LIBS) $(BLUEZ_LIBS) $(CRCANVAS_LIBS)

