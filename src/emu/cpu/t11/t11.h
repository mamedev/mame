/*** T-11: Portable DEC T-11 emulator ******************************************/

#pragma once

#ifndef __T11_H__
#define __T11_H__


enum
{
	T11_R0=1, T11_R1, T11_R2, T11_R3, T11_R4, T11_R5, T11_SP, T11_PC, T11_PSW
};

#define T11_IRQ0        0      /* IRQ0 */
#define T11_IRQ1        1      /* IRQ1 */
#define T11_IRQ2        2      /* IRQ2 */
#define T11_IRQ3        3      /* IRQ3 */

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
	UINT16  mode;           /* initial processor mode */
};


DECLARE_LEGACY_CPU_DEVICE(T11, t11);

CPU_DISASSEMBLE( t11 );

#endif /* __T11_H__ */
