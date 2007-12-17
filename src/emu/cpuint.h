/***************************************************************************

    cpuint.h

    Core multi-CPU interrupt engine.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __CPUINT_H__
#define __CPUINT_H__

#include "memory.h"

#define INTERRUPT_GEN(func)		void func(void)



/*************************************
 *
 *  Startup/shutdown
 *
 *************************************/

void cpuint_init(running_machine *machine);

void cpuint_reset(void);

extern int (*cpu_irq_callbacks[])(int);



/*************************************
 *
 *  CPU lines
 *
 *************************************/

/* Set the logical state (ASSERT_LINE/CLEAR_LINE) of the an input line on a CPU */
void cpunum_set_input_line(int cpunum, int line, int state);

/* Set the vector to be returned during a CPU's interrupt acknowledge cycle */
void cpunum_set_input_line_vector(int cpunum, int irqline, int vector);

/* Set the logical state (ASSERT_LINE/CLEAR_LINE) of the an input line on a CPU and its associated vector */
void cpunum_set_input_line_and_vector(int cpunum, int line, int state, int vector);

/* Install a driver callback for IRQ acknowledge */
void cpunum_set_irq_callback(int cpunum, int (*callback)(int irqline));


#endif	/* __CPUINT_H__ */
