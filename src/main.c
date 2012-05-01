#include <cairo.h>
#include <gtk/gtk.h>


static gboolean
on_expose_event(GtkWidget *widget,
    GdkEventExpose *event,
    gpointer data)
{
  cairo_t *cr;

  cr = gdk_cairo_create (widget->window);

  cairo_set_source_rgba(cr, 0, 0, 0, 1);
  cairo_set_line_width(cr, 10);

  cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
  cairo_move_to(cr, 30, 50);
  cairo_line_to(cr, 150, 50);
  cairo_stroke(cr);

  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  cairo_move_to(cr, 30, 90);
  cairo_line_to(cr, 150, 90);
  cairo_stroke(cr);

  cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
  cairo_move_to(cr, 30, 130);
  cairo_line_to(cr, 150, 130);
  cairo_stroke(cr);

  cairo_set_line_width(cr, 1.5);

  cairo_move_to(cr, 30, 40);
  cairo_line_to(cr, 30, 140);
  cairo_stroke(cr);

  cairo_move_to(cr, 150, 40);
  cairo_line_to(cr, 150, 140);
  cairo_stroke(cr);

  cairo_move_to(cr, 155, 40);
  cairo_line_to(cr, 155, 140);
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
      G_CALLBACK(on_expose_event), NULL);
  g_signal_connect(window, "destroy",
      G_CALLBACK(gtk_main_quit), NULL);

  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}
