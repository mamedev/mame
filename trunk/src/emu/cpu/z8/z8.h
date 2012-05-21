/**********************************************************************

    Zilog Z8 Single-Chip MCU emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __Z8_H__
#define __Z8_H__


enum
{
	Z8_PC, Z8_SP, Z8_RP, Z8_T0, Z8_T1,

	Z8_R0, Z8_R1, Z8_R2, Z8_R3, Z8_R4, Z8_R5, Z8_R6, Z8_R7, Z8_R8, Z8_R9, Z8_R10, Z8_R11, Z8_R12, Z8_R13, Z8_R14, Z8_R15,

	Z8_GENPC = STATE_GENPC,
	Z8_GENSP = STATE_GENSP
};

/* Zilog Z8601 */
DECLARE_LEGACY_CPU_DEVICE(Z8601, z8601);

/* VEB Mikroelektronik Erfurt UB8830D MME */
DECLARE_LEGACY_CPU_DEVICE(UB8830D, ub8830d);

/* Zilog Z8611 */
DECLARE_LEGACY_CPU_DEVICE(Z8611, z8611);

CPU_DISASSEMBLE( z8 );

#endif
