#ifndef LAB_STRUCT_H
#define LAB_STRUCT_H

/**
 * lab network device struct
 * */
struct lab_net_device
{
    int number;
    char name[16];
    unsigned long state;
};
/**
 * lab page struct
 * */
struct lab_page
{
    unsigned long flags;
    unsigned long virtual_address;
    unsigned long page_index;
    unsigned int  page_type;
};
/**
 * lab thread struct
 * */
struct lab_thread {
    unsigned long sp;
};
/**
 * request we sent to device
 * */
struct lab_request
{
    int pid;
    int page_number;
};
/**
 * request we get from device
 * */
struct lab_response
{
    struct lab_net_device lnd;
    struct lab_page lp;
    struct lab_thread lt;
};

#endif