	/**************************************************************************\
	*                 Texas Instruments TMS32010 DSP Emulator                  *
	*                                                                          *
	*                  Copyright Tony La Porta                                 *
	*      You are not allowed to distribute this software commercially.       *
	*                      Written for the MAME project.                       *
	*                                                                          *
	*                                                                          *
	*      Note :  This is a word based microcontroller, with addressing       *
	*              architecture based on the Harvard addressing scheme.        *
	*                                                                          *
	\**************************************************************************/

#pragma once

#ifndef __TMS32010_H__
#define __TMS32010_H__




/****************************************************************************
 * Use this in the I/O port address fields of your driver for the BIO pin
 * i.e,
 *  AM_RANGE(TMS32010_BIO, TMS32010_BIO) AM_READ(twincobr_bio_line_r)
 */

#define TMS32010_BIO            0x10        /* BIO input */


#define TMS32010_INT_PENDING    0x80000000
#define TMS32010_INT_NONE       0


enum
{
	TMS32010_PC=1, TMS32010_SP,   TMS32010_STR,  TMS32010_ACC,
	TMS32010_PREG, TMS32010_TREG, TMS32010_AR0,  TMS32010_AR1,
	TMS32010_STK0, TMS32010_STK1, TMS32010_STK2, TMS32010_STK3
};


/****************************************************************************
 *  Public Functions
 */

DECLARE_LEGACY_CPU_DEVICE(TMS32010, tms32010);
DECLARE_LEGACY_CPU_DEVICE(TMS32015, tms32015);
DECLARE_LEGACY_CPU_DEVICE(TMS32016, tms32016);


CPU_DISASSEMBLE( tms32010 );

#endif  /* __TMS32010_H__ */
