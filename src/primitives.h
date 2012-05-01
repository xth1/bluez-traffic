#ifndef PRIMITIVES_H
#define PRIMITIVES_H 1

#include <cairo.h>
#include <gtk/gtk.h>
#include "event.h"
#include <stdlib.h>
#include <stdio.h>
#define EVENT_HEIGHT 50
#define EVENT_WIDTH 100
#define EVENT_GAP 10

typedef struct{
    int x,y;

}point;

int draw_sequencial_lines(cairo_t *cr,int *coordx,int *coordy,int n,int closed);

int draw_events(cairo_t *cr,event_t *events,int n,point begin);

#endif
