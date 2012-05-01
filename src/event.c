#include "event.h"
#include <stdlib.h>
#include <stdio.h>
int read_events_from_file(char *filename,event_t *events){

    int n;

    FILE *file=fopen(filename,"r");

    fscanf(file,"%d",&n);

    printf("N : %d, %d, %d\n",n,sizeof(event_t),n*(sizeof(event_t)) );

    printf("after\n");

    fflush(stdin);
    int i;

    for(i=0;i<n;i++){
        fscanf(file,"%d",&events[i].time);
        fscanf(file,"%s",events[i].name);
        fscanf(file,"%d",&events[i].id_current_device);
        fscanf(file,"%d",&events[i].id_rel_device);
        fscanf(file,"%d",&events[i].type);
        printf("%d %s %d %d %d\n",events[i].time,events[i].name,events[i].id_current_device,events[i].id_rel_device,events[i].type);
    }

    return n;

}
