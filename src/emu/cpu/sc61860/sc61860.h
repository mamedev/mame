/*****************************************************************************
 *
 *   sc61860.h
 *   portable sharp 61860 emulator interface
 *   (sharp pocket computers)
 *
 *   Copyright (c) 2000 Peter Trauner, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     peter.trauner@jk.uni-linz.ac.at
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/
#ifndef _SC61860_H
#define _SC61860_H

/*
  official names seam to be
  ESR-H, ESR-J
  (ESR-L SC62015 ist complete different)
 */

/* unsolved problems
   the processor has 8 kbyte internal rom
   only readable with special instructions and program execution
   64 kb external ram (first 8kbyte not seen for program execution?) */

#include "cpuintrf.h"

typedef struct {
    int (*reset)(void);
    int (*brk)(void);
    int (*x)(void);
    int (*ina)(void);
    void (*outa)(int);
    int (*inb)(void);
    void (*outb)(int);
    void (*outc)(int);
} SC61860_CONFIG;

#ifdef MAME_DEBUG
unsigned sc61860_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif /* MAME_DEBUG */

/* this is though for power on/off of the sharps */
UINT8 *sc61860_internal_ram(void);

void sc61860_get_info(UINT32 state, cpuinfo *info);

#endif
