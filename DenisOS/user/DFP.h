/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#ifndef __DFP_H
#define __DFP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "libc.h"

typedef int fid_t;

typedef enum {
    TAKEN,
    AVAILABLE
} fstatus_t;

typedef struct {
    fid_t     fid;
    pid_t     owner;
    fstatus_t status;
} fork_t;


#endif
