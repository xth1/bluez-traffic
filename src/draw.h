#include <gtk/gtk.h>
#include <sys/time.h>
#define MAX_SIZE_EVENT_NAME 64

typedef struct{
  char socket_name[MAX_SIZE_EVENT_NAME];
  struct timeval *time;
  int index;
  int address_device;
  int type;
  char *data;
}event_t;

typedef struct{
    int x,y;

}point;

int draw_init(int argc,char **argv,GMainLoop *loop);
void add_event(event_t e);
