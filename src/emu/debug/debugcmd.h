// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    debugcmd.h

    Debugger command interface engine.

*********************************************************************/

#ifndef __DEBUGCMD_H__
#define __DEBUGCMD_H__


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- initialization ----- */

/* initializes the command system */
void debug_command_init(running_machine &machine);



/* ----- parameter validation ----- */

/* validates a number parameter */
int debug_command_parameter_number(running_machine &machine, const char *param, UINT64 *result);

/* validates a parameter as a cpu */
int debug_command_parameter_cpu(running_machine &machine, const char *param, device_t **result);

/* validates a parameter as a cpu and retrieves the given address space */
int debug_command_parameter_cpu_space(running_machine &machine, const char *param, int spacenum, address_space **result);

#endif
