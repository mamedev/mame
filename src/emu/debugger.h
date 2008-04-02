/****************************************************************************

    debugger.h

    General debugging interfaces

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************/

#pragma once

#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#include "mame.h"
#include "memory.h"
#include "deprecat.h"


/***************************************************************************
    MACROS
***************************************************************************/

/* handy macro for hard-coding debugger breaks */
#ifdef ENABLE_DEBUGGER
#define DEBUGGER_BREAK			if (Machine->debug_mode) mame_debug_break();
#else
#define DEBUGGER_BREAK
#endif


/* handy macro for CPU cores */
#ifdef ENABLE_DEBUGGER
#define CALL_DEBUGGER(p)		if (Machine->debug_mode) mame_debug_hook(p);
#else
#define CALL_DEBUGGER(p)
#endif



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* initialize the debugger */
void mame_debug_init(running_machine *machine);

/* call this once per instruction from CPU cores */
void mame_debug_hook(offs_t curpc);

/* call this to break into the debugger as soon as possible */
void mame_debug_break(void);

/* call this to determine if the debugger is currently active (broken) */
int mame_debug_is_active(void);



/***************************************************************************
    STUBS
***************************************************************************/

#ifndef ENABLE_DEBUGGER
#define mame_debug_init(m) do { } while (0)
#define mame_debug_hook() do { } while (0)
#define mame_debug_break() do { } while (0)
#define mame_debug_is_active() FALSE
#endif


#endif	/* __DEBUGGER_H__ */
