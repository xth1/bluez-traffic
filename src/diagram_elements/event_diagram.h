/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Thiago da Silva Arruda <thiago.xth1@gmail.com>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#ifndef DATA_DUMPED_HEADER

#include "../data_dumped.h"

#endif

#ifndef UTIL_HEADER
#include "../util.h"
#endif

#include <glib.h>
#include <cr-canvas.h>

struct event_diagram{ 
	struct event_t *event;
	struct point position;
};

/*
struct event_diagram_data{
	struct event_t *event;
};
*/ 

enum{
	EVENT_SELECTED = 1,
	EVENT_UNSELECTED,
};

typedef void (*event_diagram_callback) (struct event_diagram *des, int action);

GHashTable *make_all_events(CrItem *group, GArray *events, 
						event_diagram_callback callback, int events_size,
						struct point p, int w, int h);
