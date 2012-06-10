#include <cairo.h>
#include <gtk/gtk.h>
#include <string.h>
#include <glib.h>
#include <sys/time.h>
#include "draw.h"
#define MAX_BUFF 512

/* drawing contants */
#define R_W 375
#define R_H 30
#define SPACE 10
#define FONT_SIZE 9
#define GAP_SIZE 9

gboolean quit(GtkWidget *widget,GdkEventExpose *event,
    gpointer data);
gboolean on_expose_event(GtkWidget *widget,GdkEventExpose *event,
	gpointer data);
gboolean on_destroy_event(GtkWidget *widget,GdkEventExpose *event,
    gpointer data);
void draw_event(cairo_t *cr,event_t e,point p);

event_t get_event(int p);

static GtkWidget *window;
static GtkWidget *darea;
static GMainLoop *mainloop;
static cairo_t *cairo_draw;


/*Array of events*/
static GArray *events;
static int events_size;
void add_event(event_t e)
{
    g_array_append_val (events, e);
    events_size++;
    
    /*generate event to update darea*/
    gtk_widget_queue_draw(darea);
    
}

event_t get_event(int p)
{
	return g_array_index(events,event_t,p);
}

gboolean on_destroy_event(GtkWidget *widget,GdkEventExpose *event,
    gpointer data)
{
	gtk_main_quit();
	g_main_quit (mainloop);
	
	return FALSE;
}


gboolean on_expose_event(GtkWidget *widget,GdkEventExpose *event,gpointer data)
{
	cairo_t *cr;
	event_t e;
	int i;
	point p;
	cr = gdk_cairo_create (widget->window);
	
	p.x=p.y=0;
	for(i=0;i<events_size;i++){
		e=get_event(i);
		draw_event(cr,e,p);
		p.y+=R_H+SPACE;
	}
   

    return FALSE;
}

void draw_event(cairo_t *cr,event_t e,point p)
{
	
	char buff[MAX_BUFF];
	int size;
	time_t t;
	struct tm tm;
	/*draw retangle */
	p.x+= SPACE;
	p.y+= SPACE;
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_set_line_width(cr, 1);
	
	cairo_rectangle(cr, p.x,p.y, p.x + R_W, p.y + R_H);
	cairo_stroke_preserve(cr);
	
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_fill(cr);
	
	p.x+= SPACE;
	p.y+= SPACE;
	
	cairo_select_font_face(cr, "Courier",
	CAIRO_FONT_SLANT_NORMAL,
	CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr,FONT_SIZE);
	cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
	
	/*print date*/
	t = (e.time)->tv_sec;
	localtime_r(&t, &tm);

	sprintf(buff,"%04d-%02d-%02d", 
	tm.tm_year + 1900,tm.tm_mon + 1, tm.tm_mday);
	cairo_move_to(cr,p.x, p.y);
	cairo_show_text(cr,buff);
	size=strlen(buff);
	
	/*print time*/
	sprintf(buff,"%02d:%02d:%02d.%06lu ", tm.tm_hour,tm.tm_min,
						 tm.tm_sec, (e.time)->tv_usec);
	cairo_move_to(cr,p.x, p.y+ 2*SPACE);
	cairo_show_text(cr,buff);
	
	/*print adapter index*/

	cairo_set_source_rgb(cr, 0.3, 0.0, 0.0);
	cairo_set_font_size(cr,FONT_SIZE+2);
	p.x+= size*GAP_SIZE + SPACE;
	cairo_move_to(cr, p.x, p.y);
	sprintf(buff,"HCI%d",e.index);
	cairo_show_text(cr,buff);
	size=strlen(buff);
		
	/*print socket type*/
	cairo_set_source_rgb(cr, 0.0, 0.5, 0.0);
	cairo_set_font_size(cr,FONT_SIZE+1);
	
	p.x+= size*GAP_SIZE + SPACE;
	cairo_move_to(cr,p.x , p.y);
	cairo_show_text(cr,e.socket_name);
	
	/*print socket type*/
	cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
	cairo_set_font_size(cr,FONT_SIZE+1);
	
	sprintf(buff,"OPCODE: %d",e.type);
	
	p.x+= size*GAP_SIZE + SPACE;
	cairo_move_to(cr,p.x , p.y);
	cairo_show_text(cr,buff);
	
	//size=strlen(buff);
	
}


int draw_init(int argc,char **argv,GMainLoop *loop)
{
	gtk_init(&argc,&argv);

	/* set variables */
	mainloop=loop;
	window = gtk_window_new(0);
	
	darea = gtk_drawing_area_new ();
	cairo_draw = gdk_cairo_create (darea->window);
	
	events=g_array_new(FALSE,FALSE,sizeof(event_t));
	events_size=0;
	
	gtk_container_add(GTK_CONTAINER (window), darea);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 700);

	gtk_widget_show_all(window);
 
	/*set events handlers */
	g_signal_connect(window, "destroy",
		G_CALLBACK(on_destroy_event), NULL);

	g_signal_connect(darea, "expose-event",G_CALLBACK(on_expose_event), NULL);

	return 0;
}


