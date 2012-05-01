#include "primitives.h"

int draw_sequencial_lines(cairo_t *cr,int *coordx,int *coordy,int n,int closed){
	int i;
	if(n <2)
		return FALSE;
	for(i=0;i<n;i++){
		cairo_move_to(cr, coordx[i], coordy[i]);
          	cairo_line_to(cr, coordx[i+1], coordy[i+1]);
	}
	return TRUE;
}

int draw_events(cairo_t *cr,event_t *events,int n,point begin){
    int i;
    char buff[500];
    point p=begin;
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1);
    p.x+=EVENT_GAP;
    for(i=0;i<n;i++){
         cairo_rectangle(cr,p.x,p.y,EVENT_WIDTH, EVENT_HEIGHT);




         sprintf(buff,"%d ms",events[i].time);

         cairo_move_to(cr, p.x + EVENT_GAP, p.y + EVENT_GAP);
         cairo_show_text(cr, buff);

         cairo_move_to(cr, p.x + 4*EVENT_GAP, p.y + EVENT_GAP);
         cairo_show_text(cr, events[i].name);

          p.y+=EVENT_HEIGHT+EVENT_GAP;
    }

    return TRUE;


}
