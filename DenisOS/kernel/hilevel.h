/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#ifndef __HILEVEL_H
#define __HILEVEL_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Include functionality relating to the platform.

#include   "GIC.h"
#include "PL011.h"
#include "PL050.h"
#include "PL111.h"
#include "SP804.h"
#include   "SYS.h"

// Include functionality relating to the kernel.

#include "lolevel.h"
#include "int.h"
#include "libc.h"

typedef int pid_t;
typedef int pipeid_t;
typedef int epid_t;

typedef enum {
    STATUS_TERMINATED,
    STATUS_CREATED,
    STATUS_READY,
    STATUS_EXECUTING,
    STATUS_WAITING
} status_t;


const int pLow = 0;
const int pNormal = 500;


//difference between each priority level [default 250].

const int priorityWeight = 250;

typedef enum {
    PRIORITY_LOW,
    PRIORITY_NORMAL,
    PRIORITY_HIGH,
} priority_t;

typedef enum { WRITE, READ} type_t;
typedef enum { EMPTY, FULL} pipestatus_t;
typedef enum { YES, NO} inUse;

typedef struct {
    uint32_t cpsr, pc, gpr[ 13 ], sp, lr;
} ctx_t;

typedef struct {
    pid_t      pid;
    status_t   status;
    priority_t priority;
    ctx_t      ctx;
    int        age;
} pcb_t;

typedef struct {
    pipeid_t     pipeid; //pipe ID.
    pipestatus_t status; //EMPTY OR FULL
    inUse        used;   //YES OR NO.
    char         data;   //DATA BEING PASSED.
} pipe_t;

typedef struct {
    epid_t     eid;    //endpoint ID [fd].
    pipeid_t   pipeid; //owner of this endpoint.
    pid_t        pid;  //process owning endpoint.
    inUse      used;   //YES OR NO.
    type_t     type;   //is WRITE or READ.
} endpoint_t;

typedef int sem_t;


typedef pcb_t item;

typedef struct{
    uint16_t x;
    uint16_t y;
    uint16_t clr;
} pixel_t;



#endif
