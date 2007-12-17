 /**************************************************************************\
 *                 Texas Instruments TMS32010 DSP Emulator                  *
 *                                                                          *
 *                  Copyright (C) 1999-2002+ Tony La Porta                  *
 *      You are not allowed to distribute this software commercially.       *
 *                      Written for the MAME project.                       *
 *                                                                          *
 *                                                                          *
 *      Note :  This is a word based microcontroller, with addressing       *
 *              architecture based on the Harvard addressing scheme.        *
 *                                                                          *
 \**************************************************************************/

#ifndef _TMS32010_H
#define _TMS32010_H


#include "cpuintrf.h"



/**************************************************************************
 *  Internal Clock divisor
 *
 *  External Clock is divided internally by 4, to produce the states
 *  used in carrying out an instruction (machine) cycle.
 */

#define TMS32010_CLOCK_DIVIDER		4


/****************************************************************************
 * Use this in the I/O port address fields of your driver for the BIO pin
 * i.e,
 *  AM_RANGE(TMS32010_BIO, TMS32010_BIO) AM_READ(twincobr_bio_line_r)
 */

#define TMS32010_BIO			0x10		/* BIO input */


#define TMS32010_INT_PENDING	0x80000000
#define TMS32010_INT_NONE		0

#define  TMS32010_ADDR_MASK  0x0fff		/* TMS32010 can only address 0x0fff */
										/* however other TMS3201x devices   */
										/* can address up to 0xffff (incase */
										/* their support is ever added).    */


enum {
	TMS32010_PC=1, TMS32010_SP,   TMS32010_STR,  TMS32010_ACC,
	TMS32010_PREG, TMS32010_TREG, TMS32010_AR0,  TMS32010_AR1,
	TMS32010_STK0, TMS32010_STK1, TMS32010_STK2, TMS32010_STK3
};


/****************************************************************************
 *  Public Functions
 */

void tms32010_get_info(UINT32 state, cpuinfo *info);



/****************************************************************************
 *  Read the state of the BIO pin
 */

#define TMS32010_BIO_In (io_read_word_16be(TMS32010_BIO<<1))


/****************************************************************************
 *  Input a word from given I/O port
 */

#define TMS32010_In(Port) (io_read_word_16be((Port)<<1))


/****************************************************************************
 *  Output a word to given I/O port
 */

#define TMS32010_Out(Port,Value) (io_write_word_16be((Port)<<1,Value))



/****************************************************************************
 *  Read a word from given ROM memory location
 */

#define TMS32010_ROM_RDMEM(A) (program_read_word_16be((A)<<1))


/****************************************************************************
 *  Write a word to given ROM memory location
 */

#define TMS32010_ROM_WRMEM(A,V) (program_write_word_16be((A)<<1,V))



/****************************************************************************
 *  Read a word from given RAM memory location
 */

#define TMS32010_RAM_RDMEM(A) (data_read_word_16be((A)<<1))


/****************************************************************************
 *  Write a word to given RAM memory location
 */

#define TMS32010_RAM_WRMEM(A,V) (data_write_word_16be((A)<<1,V))



/****************************************************************************
 *  TMS32010_RDOP() is identical to TMS32010_RDMEM() except it is used for reading
 *  opcodes. In case of system with memory mapped I/O, this function can be
 *  used to greatly speed up emulation
 */

#define TMS32010_RDOP(A) (cpu_readop16((A)<<1))


/****************************************************************************
 *  TMS32010_RDOP_ARG() is identical to TMS32010_RDOP() except it is used
 *  for reading opcode arguments. This difference can be used to support systems
 *  that use different encoding mechanisms for opcodes and opcode arguments
 */

#define TMS32010_RDOP_ARG(A) (cpu_readop_arg16((A)<<1))



#ifdef	MAME_DEBUG
offs_t tms32010_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif

#endif	/* _TMS32010_H */
