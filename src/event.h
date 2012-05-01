#ifndef EVENT_H
#define EVENT_H 1
#define MAX_SIZE_EVENT_NAME 256

typedef struct{
  char name[MAX_SIZE_EVENT_NAME];
  int time;
  int id_current_device;
  int id_rel_device;
  int type;
}event_t;

int read_events_from_file(char *filename,event_t *events);

#endif
