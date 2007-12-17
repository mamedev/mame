/*****************************************************************************
 *
 *   z8000cpu.h
 *   Portable Z8000(2) emulator
 *   Macros and types used in z8000.c / z8000ops.c / z8000tbl.c
 *
 *   Copyright (c) 1998,1999 Juergen Buchmueller, all rights reserved.
 *   Bug fixes and MSB_FIRST compliance Ernesto Corvi.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

/* pointers to the registers inside the Z8000_Regs struct Z */
#define RB(n)   (*pRB[n])
#define RW(n)   (*pRW[n])
#define RL(n)   (*pRL[n])
#define RQ(n)   (*pRQ[n])

/* the register used as stack pointer */
#define SP      15

/* programm status */
#define PPC     Z.ppc
#define PC		Z.pc
#define PSAP    Z.psap
#define FCW     Z.fcw
#define REFRESH Z.refresh
#define NSP 	Z.nsp
#define IRQ_REQ Z.irq_req
#define IRQ_SRV Z.irq_srv
#define IRQ_VEC Z.irq_vec

/* these vectors are based on PSAP */
#define RST 	(Z.psap + 0x0000)	/* start up FCW and PC */
#define EPU 	(Z.psap + 0x0004)	/* extension processor unit? trap */
#define TRAP	(Z.psap + 0x0008)	/* privilege violation trap */
#define SYSCALL (Z.psap + 0x000c)	/* system call SC */
#define SEGTRAP (Z.psap + 0x0010)	/* segment trap */
#define NMI 	(Z.psap + 0x0014)	/* non maskable interrupt */
#define NVI 	(Z.psap + 0x0018)	/* non vectored interrupt */
#define VI		(Z.psap + 0x001c)	/* vectored interrupt */
#define VEC00	(Z.psap + 0x001e)	/* vector n PC value */

/* bits of the FCW */
#define F_SEG	0x8000				/* segmented mode (Z8001 only) */
#define F_S_N	0x4000				/* system / normal mode */
#define F_EPU	0x2000				/* extension processor unit? */
#define F_NVIE	0x1000				/* non vectored interrupt enable */
#define F_VIE	0x0800				/* vectored interrupt enable */
#define F_10	0x0400				/* unused */
#define F_9 	0x0200				/* unused */
#define F_8 	0x0100				/* unused */
#define F_C 	0x0080				/* carry flag */
#define F_Z 	0x0040				/* zero flag */
#define F_S 	0x0020				/* sign flag */
#define F_PV	0x0010				/* parity/overflow flag */
#define F_DA	0x0008				/* decimal adjust flag (0 add/adc, 1 sub/sbc) */
#define F_H 	0x0004				/* half carry flag (byte arithmetic only) */
#define F_1 	0x0002				/* unused */
#define F_0 	0x0001				/* unused */

/* opcode word numbers in Z.op[] array */
#define OP0 	0
#define OP1     1
#define OP2     2

/* nibble shift factors for an opcode word */
/* left to right: 0x1340 -> NIB0=1, NIB1=3, NIB2=4, NIB3=0 */
#define NIB0    12
#define NIB1	8
#define NIB2	4
#define NIB3	0

/* sign bit masks for byte, word and long */
#define S08 0x80
#define S16 0x8000
#define S32 0x80000000

/* get a single flag bit 0/1 */
#define GET_C       ((FCW >> 7) & 1)
#define GET_Z		((FCW >> 6) & 1)
#define GET_S		((FCW >> 5) & 1)
#define GET_PV		((FCW >> 4) & 1)
#define GET_DA		((FCW >> 3) & 1)
#define GET_H		((FCW >> 2) & 1)

/* clear a single flag bit */
#define CLR_C       FCW &= ~F_C
#define CLR_Z		FCW &= ~F_Z
#define CLR_S		FCW &= ~F_S
#define CLR_P		FCW &= ~F_PV
#define CLR_V		FCW &= ~F_PV
#define CLR_DA		FCW &= ~F_DA
#define CLR_H		FCW &= ~F_H

/* clear a flag bit combination */
#define CLR_CZS     FCW &= ~(F_C|F_Z|F_S)
#define CLR_CZSP	FCW &= ~(F_C|F_Z|F_S|F_PV)
#define CLR_CZSV	FCW &= ~(F_C|F_Z|F_S|F_PV)
#define CLR_CZSVH	FCW &= ~(F_C|F_Z|F_S|F_PV|F_H)
#define CLR_ZS		FCW &= ~(F_Z|F_S)
#define CLR_ZSV 	FCW &= ~(F_Z|F_S|F_PV)
#define CLR_ZSP 	FCW &= ~(F_Z|F_S|F_PV)

/* set a single flag bit */
#define SET_C       FCW |= F_C
#define SET_Z		FCW |= F_Z
#define SET_S		FCW |= F_S
#define SET_P		FCW |= F_PV
#define SET_V		FCW |= F_PV
#define SET_DA		FCW |= F_DA
#define SET_H		FCW |= F_H

/* set a flag bit combination */
#define SET_SC      FCW |= F_C | F_S

/* check condition codes */
#define CC0 (0) 						/* always false */
#define CC1 (GET_PV^GET_S)				/* less than */
#define CC2 (GET_Z|(GET_PV^GET_S))		/* less than or equal */
#define CC3 (GET_Z|GET_C)				/* unsigned less than or equal */
#define CC4 GET_PV						/* parity even / overflow */
#define CC5 GET_S						/* minus (signed) */
#define CC6 GET_Z						/* zero / equal */
#define CC7 GET_C						/* carry / unsigned less than */

#define CC8 (1) 						/* always true */
#define CC9 !(GET_PV^GET_S) 			/* greater than or equal */
#define CCA !(GET_Z|(GET_PV^GET_S)) 	/* greater than */
#define CCB !(GET_Z|GET_C)				/* unsigned greater than */
#define CCC !GET_PV 					/* parity odd / no overflow */
#define CCD !GET_S						/* plus (not signed) */
#define CCE !GET_Z						/* not zero / not equal */
#define CCF !GET_C						/* not carry / unsigned greater than */

/* get data from the opcode words */
/* o is the opcode word offset    */
/* s is a nibble shift factor     */
#define GET_BIT(o)      UINT16 bit = 1 << (Z.op[o] & 15)
#define GET_CCC(o,s)	UINT8 cc = (Z.op[o] >> (s)) & 15

#define GET_DST(o,s)	UINT8 dst = (Z.op[o] >> (s)) & 15
#define GET_SRC(o,s)	UINT8 src = (Z.op[o] >> (s)) & 15
#define GET_IDX(o,s)	UINT8 idx = (Z.op[o] >> (s)) & 15
#define GET_CNT(o,s)	INT8 cnt = (Z.op[o] >> (s)) & 15
#define GET_IMM4(o,s)	UINT8 imm4 = (Z.op[o] >> (s)) & 15

#define GET_I4M1(o,s)	UINT8 i4p1 = ((Z.op[o] >> (s)) & 15) + 1
#define GET_IMM1(o,s)	UINT8 imm1 = (Z.op[o] >> (s)) & 2
#define GET_IMM2(o,s)	UINT8 imm2 = (Z.op[o] >> (s)) & 3
#define GET_IMM3(o,s)	UINT8 imm3 = (Z.op[o] >> (s)) & 7

#define GET_IMM8(o) 	UINT8 imm8 = (UINT8)Z.op[o]

#define GET_IMM16(o)	UINT16 imm16 = Z.op[o]
#define GET_IMM32		UINT32 imm32 = Z.op[2] + (Z.op[1] << 16)
#define GET_DSP7		UINT8 dsp7 = Z.op[0] & 127
#define GET_DSP8		INT8 dsp8 = (INT8)Z.op[0]
#define GET_DSP16		UINT16 dsp16 = PC + (INT16)Z.op[1]
#define GET_ADDR(o) 	UINT16 addr = (UINT16)Z.op[o]

/* structure for the opcode definition table */
typedef struct {
	int 	beg, end, step;
	int 	size, cycles;
	void	(*opcode)(void);
	const char	*dasm;
    UINT32 dasmflags;
}	Z8000_init;

/* structure for the opcode execution table / disassembler */
typedef struct {
    void    (*opcode)(void);
    int     cycles;
	int 	size;
    const char    *dasm;
    UINT32 dasmflags;
}	Z8000_exec;

/* opcode execution table */
extern Z8000_exec *z8000_exec;

extern void z8000_init(int index, int clock, const void *config, int (*irqcallback)(int));
extern void z8000_deinit(void);

