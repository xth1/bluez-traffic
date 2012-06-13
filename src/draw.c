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

#define WINDOW_H 600
#define WINDOW_W 400

#define DAREA_H WINDOW_H
#define DAREA_W (WINDOW_H/2)
#define DAREA_HEIGHT_GROW 200



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
	int new_height;
	point p;
	GtkRequisition size;
	cr = gdk_cairo_create (widget->window);
	gtk_widget_size_request(widget, &size);

	new_height=events_size * R_H;
	if(new_height != size.height)
		gtk_widget_set_size_request(darea, size.width,
			new_height);
	

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

}


int draw_init(int argc,char **argv,GMainLoop *loop)
{
	GtkWidget* sw;
	
	GtkRequisition size;
    GtkWidget* viewport;
    
    GtkWidget *vbox;
    
	GtkWidget *menubar;
	GtkWidget *filemenu;
	GtkWidget *file;
	GtkWidget *quit;
	
	gtk_init(&argc,&argv);

	/* set variables */
	mainloop=loop;
	window = gtk_window_new(0);
	darea = gtk_drawing_area_new ();
	sw = gtk_scrolled_window_new(NULL, NULL);
	events=g_array_new(FALSE,FALSE,sizeof(event_t));
	events_size=0;
	
	/* add layout manager */
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	
	/*add menubar*/
	menubar = gtk_menu_bar_new();
	filemenu = gtk_menu_new();

	file = gtk_menu_item_new_with_mnemonic("_File");

	quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
	g_signal_connect(quit, "activate",
        G_CALLBACK(on_destroy_event), (gpointer) quit);
	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
	
	/* add scrollable drawing area*/
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);	
	gtk_box_pack_end(GTK_BOX(vbox), sw, TRUE, TRUE, 0);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), darea);
	
	/*set events handlers */
	g_signal_connect(window, "destroy",
		G_CALLBACK(on_destroy_event), NULL);
	g_signal_connect(darea, "expose-event",G_CALLBACK(on_expose_event), NULL);
	
	/*set positions and dimensions*/
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_W, WINDOW_H);
	
	viewport = gtk_widget_get_ancestor(darea, GTK_TYPE_VIEWPORT);
    gtk_widget_size_request(darea, &size);
    gtk_widget_set_size_request(darea, DAREA_W, DAREA_H);
    gtk_widget_set_size_request(sw, DAREA_W, DAREA_H);
    
    gtk_widget_show_all(window);

	return 0;
}


