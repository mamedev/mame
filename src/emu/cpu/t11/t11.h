/*** T-11: Portable DEC T-11 emulator ******************************************/

#ifndef _T11_H
#define _T11_H

#include "cpuintrf.h"

enum {
	T11_R0=1, T11_R1, T11_R2, T11_R3, T11_R4, T11_R5, T11_SP, T11_PC, T11_PSW };

#define T11_IRQ0        0      /* IRQ0 */
#define T11_IRQ1		1	   /* IRQ1 */
#define T11_IRQ2		2	   /* IRQ2 */
#define T11_IRQ3		3	   /* IRQ3 */

#define T11_RESERVED    0x000   /* Reserved vector */
#define T11_TIMEOUT     0x004   /* Time-out/system error vector */
#define T11_ILLINST     0x008   /* Illegal and reserved instruction vector */
#define T11_BPT         0x00C   /* BPT instruction vector */
#define T11_IOT         0x010   /* IOT instruction vector */
#define T11_PWRFAIL     0x014   /* Power fail vector */
#define T11_EMT         0x018   /* EMT instruction vector */
#define T11_TRAP        0x01C   /* TRAP instruction vector */


struct t11_setup
{
	UINT16	mode;			/* initial processor mode */
};


extern void t11_get_info(UINT32 state, cpuinfo *info);

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
#define T11_RDMEM(A) ((unsigned)program_read_byte_16le(A))
#define T11_RDMEM_WORD(A) ((unsigned)program_read_word_16le(A))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define T11_WRMEM(A,V) (program_write_byte_16le(A,V))
#define T11_WRMEM_WORD(A,V) (program_write_word_16le(A,V))

#ifdef MAME_DEBUG
offs_t t11_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif

#endif /* _T11_H */
