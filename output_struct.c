#include "output_struct.h"

/**
 * added to (char *) output string 
 * with (char * ) title
 * and value by specificator
 * with \\n
 * */
#define sprintf_with_value(output,buffer,title,specificator,value)\
    strcat((output),(title)); \
    sprintf((buffer),(specificator),(value)); \
    strcat((output),(buffer)); \
    strcat((output),"\n");

void sprintf_net_device(struct lab_net_device *lnd,char * output)
{
       char buffer_integer[20];
    strcat(output,"<--- NET_DEVICE --->\n");
    sprintf_with_value(output,buffer_integer,"number of device is ","%d", lnd->number);
    sprintf_with_value(output,buffer_integer,"name is  ","%s", lnd->name);
    sprintf_with_value(output,buffer_integer,"state is","%lu", lnd->state);
}

void sprintf_page(struct lab_page *lp,char * output)
{
    char buffer_integer[20];
    strcat(output,"<--- PAGE_STRUCT --->\n");
    sprintf_with_value(output,buffer_integer,"flags of page: ","%x", lp->flags);
    sprintf_with_value(output,buffer_integer,"virtual address is ","%x", lp->virtual_address);
    sprintf_with_value(output,buffer_integer,"page index: ","%x", lp->page_index);
    sprintf_with_value(output,buffer_integer,"page type is : ","%u", lp->page_type);
}

void sprintf_thread(struct lab_thread *lt,char * output)
{
        char buffer_integer[20];
    strcat(output,"<--- THREAD_STRUCT --->\n");
    sprintf_with_value(output,buffer_integer,"th.sp->","%lu",lt->sp);
}