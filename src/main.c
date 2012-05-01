#include <cairo.h>
#include <gtk/gtk.h>
#include "event.h"
#include "primitives.h"


static gboolean
draw_event(GtkWidget *widget,
    GdkEventExpose *event,
    gpointer data)
{
  cairo_t *cr;

  cr = gdk_cairo_create (widget->window);

  //load events------------------------------------
  event_t *events;
  events=(event_t *)calloc(110,(sizeof(event_t)));

  int n;
  n=read_events_from_file("events.txt",events);

  point p={0,0};
  draw_events(cr,events,n,p);



  cairo_stroke(cr);

  cairo_destroy(cr);

  return FALSE;
}


int main (int argc, char *argv[])
{


  GtkWidget *window;
  GtkWidget *darea;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  darea = gtk_drawing_area_new ();
  gtk_container_add(GTK_CONTAINER (window), darea);

  g_signal_connect(darea, "expose-event",
      G_CALLBACK(draw_event), NULL);
  g_signal_connect(window, "destroy",
      G_CALLBACK(gtk_main_quit), NULL);

  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);

  gtk_widget_show_all(window);

  gtk_main();

  return 0;


}
