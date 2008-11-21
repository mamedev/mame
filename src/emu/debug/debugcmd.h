/*********************************************************************

    debugcmd.h

    Debugger command interface engine.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#ifndef __DEBUGCMD_H__
#define __DEBUGCMD_H__


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* initialization */
void debug_command_init(running_machine *machine);

/*  parameter validation */
int	debug_command_parameter_number(running_machine *machine, const char *param, UINT64 *result);
int debug_command_parameter_cpu(running_machine *machine, const char *param, const device_config **result);

#endif
