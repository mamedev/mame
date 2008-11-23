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


#include "cpuintrf.h"



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


enum
{
	TMS32010_PC=1, TMS32010_SP,   TMS32010_STR,  TMS32010_ACC,
	TMS32010_PREG, TMS32010_TREG, TMS32010_AR0,  TMS32010_AR1,
	TMS32010_STK0, TMS32010_STK1, TMS32010_STK2, TMS32010_STK3
};


/****************************************************************************
 *  Public Functions
 */

CPU_GET_INFO( tms32010 );



/****************************************************************************
 *  Read the state of the BIO pin
 */

#define TMS32010_BIO_In (memory_read_word_16be(R.io, TMS32010_BIO<<1))


/****************************************************************************
 *  Input a word from given I/O port
 */

#define TMS32010_In(Port) (memory_read_word_16be(R.io, (Port)<<1))


/****************************************************************************
 *  Output a word to given I/O port
 */

#define TMS32010_Out(Port,Value) (memory_write_word_16be(R.io, (Port)<<1,Value))



/****************************************************************************
 *  Read a word from given ROM memory location
 */

#define TMS32010_ROM_RDMEM(A) (memory_read_word_16be(R.program, (A)<<1))


/****************************************************************************
 *  Write a word to given ROM memory location
 */

#define TMS32010_ROM_WRMEM(A,V) (memory_write_word_16be(R.program, (A)<<1,V))



/****************************************************************************
 *  Read a word from given RAM memory location
 */

#define TMS32010_RAM_RDMEM(A) (memory_read_word_16be(R.data, (A)<<1))


/****************************************************************************
 *  Write a word to given RAM memory location
 */

#define TMS32010_RAM_WRMEM(A,V) (memory_write_word_16be(R.data, (A)<<1,V))



/****************************************************************************
 *  TMS32010_RDOP() is identical to TMS32010_RDMEM() except it is used for reading
 *  opcodes. In case of system with memory mapped I/O, this function can be
 *  used to greatly speed up emulation
 */

#define TMS32010_RDOP(A) (memory_decrypted_read_word(R.program, (A)<<1))


/****************************************************************************
 *  TMS32010_RDOP_ARG() is identical to TMS32010_RDOP() except it is used
 *  for reading opcode arguments. This difference can be used to support systems
 *  that use different encoding mechanisms for opcodes and opcode arguments
 */

#define TMS32010_RDOP_ARG(A) (memory_raw_read_word(R.program, (A)<<1))



CPU_DISASSEMBLE( tms32010 );

#endif	/* __TMS32010_H__ */
