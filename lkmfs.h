#ifndef LKFS_H
#define LKFS_H

#include "character_dev.h"
#include "lab_struct.h"

#include <linux/if_tun.h>
#include <linux/if_macvlan.h>
#include <linux/signal.h>
#include <linux/pagemap.h>
#include <linux/page-flags.h>
#include <linux/mm_types.h>
#include "linux/mm.h"
#include <asm/pgtable.h>
#include <asm/page.h>

#include <asm/uaccess.h>
#include <linux/netdevice.h>
#include <linux/list.h>
#include <asm/siginfo.h>    //siginfo
#include <linux/rcupdate.h> //rcu_read_lock
#include <linux/sched.h>    //find_task_by_pid_type

int get_multiprocess_signals_info(int pid);
int get_page_struct_info(struct lab_page *lp,int pid,int page_number);
int get_net_device_struct_info(struct lab_net_device *lnd);
int get_thread_struct_info(struct lab_thread *lts, int pid);
#endif