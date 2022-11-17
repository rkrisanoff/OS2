#ifndef OUTPUT_STRUCT_H
#define OUTPUT_STRUCT_H

#include <string.h>
#include <stdio.h>
#include "lab_struct.h"

void sprintf_net_device(struct lab_net_device *lnd,char * output);
void sprintf_page(struct lab_page *lp,char * output);
void sprintf_thread(struct lab_thread *lt,char * output);

#endif