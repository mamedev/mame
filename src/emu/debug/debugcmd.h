/*********************************************************************

    debugcmd.h

    Debugger command interface engine.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#ifndef __DEBUGCMD_H__
#define __DEBUGCMD_H__


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* initialization */
void				debug_command_init(running_machine *machine);

/*  parameter validation */
int					debug_command_parameter_number(const char *param, UINT64 *result);

#endif
