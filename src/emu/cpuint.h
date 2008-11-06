/***************************************************************************

    cpuint.h

    Core multi-CPU interrupt engine.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __CPUINT_H__
#define __CPUINT_H__

#include "memory.h"

#define INTERRUPT_GEN(func)		void func(running_machine *machine, int cpunum)
#define IRQ_CALLBACK(func)		int func(running_machine *machine, int irqline)



/*************************************
 *
 *  Startup/shutdown
 *
 *************************************/

void cpuint_init(running_machine *machine);

void cpuint_reset(void);



/*************************************
 *
 *  CPU lines
 *
 *************************************/

/* Set the logical state (ASSERT_LINE/CLEAR_LINE) of the an input line on a CPU */
void cpunum_set_input_line(running_machine *machine, int cpunum, int line, int state);
void cputag_set_input_line(running_machine *machine, const char *tag, int line, int state);

/* Set the vector to be returned during a CPU's interrupt acknowledge cycle */
void cpunum_set_input_line_vector(int cpunum, int irqline, int vector);

/* Set the logical state (ASSERT_LINE/CLEAR_LINE) of the an input line on a CPU and its associated vector */
void cpunum_set_input_line_and_vector(running_machine *machine, int cpunum, int line, int state, int vector);
void cputag_set_input_line_and_vector(running_machine *machine, const char *tag, int line, int state, int vector);

/* Install a driver callback for IRQ acknowledge */
void cpunum_set_irq_callback(int cpunum, int (*callback)(running_machine *machine, int irqline));


#endif	/* __CPUINT_H__ */
