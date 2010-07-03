/*****************************************************************************
 *
 *   sc61860.h
 *   portable sharp 61860 emulator interface
 *   (sharp pocket computers)
 *
 *   Copyright Peter Trauner, all rights reserved.
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

#pragma once

#ifndef __SC61860_H__
#define __SC61860_H__

/*
  official names seam to be
  ESR-H, ESR-J
  (ESR-L SC62015 ist complete different)
 */

/* unsolved problems
   the processor has 8 kbyte internal rom
   only readable with special instructions and program execution
   64 kb external ram (first 8kbyte not seen for program execution?) */


typedef struct _sc61860_cpu_core sc61860_cpu_core;
struct _sc61860_cpu_core
{
    int (*reset)(running_device *device);
    int (*brk)(running_device *device);
    int (*x)(running_device *device);
    int (*ina)(running_device *device);
    void (*outa)(running_device *device, int);
    int (*inb)(running_device *device);
    void (*outb)(running_device *device, int);
    void (*outc)(running_device *device, int);
};

CPU_DISASSEMBLE( sc61860 );

/* this is though for power on/off of the sharps */
UINT8 *sc61860_internal_ram(running_device *device);

DECLARE_LEGACY_CPU_DEVICE(SC61860, sc61860);

#endif /* __SC61860_H__ */
