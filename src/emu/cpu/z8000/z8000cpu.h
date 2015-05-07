// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller,Ernesto Corvi
/*****************************************************************************
 *
 *   z8000cpu.h
 *   Portable Z8000(2) emulator
 *   Macros and types used in z8000.c / z8000ops.inc / z8000tbl.inc
 *
 *****************************************************************************/

/**************************************************************************
 * This is the register file layout:
 *
 * BYTE        WORD         LONG           QUAD
 * msb   lsb       bits           bits           bits
 * RH0 - RL0   R 0 15- 0    RR 0  31-16    RQ 0  63-48
 * RH1 - RL1   R 1 15- 0          15- 0          47-32
 * RH2 - RL2   R 2 15- 0    RR 2  31-16          31-16
 * RH3 - RL3   R 3 15- 0          15- 0          15- 0
 * RH4 - RL4   R 4 15- 0    RR 4  31-16    RQ 4  63-48
 * RH5 - RL5   R 5 15- 0          15- 0          47-32
 * RH6 - RL6   R 6 15- 0    RR 6  31-16          31-16
 * RH7 - RL7   R 7 15- 0          15- 0          15- 0
 *             R 8 15- 0    RR 8  31-16    RQ 8  63-48
 *             R 9 15- 0          15- 0          47-32
 *             R10 15- 0    RR10  31-16          31-16
 *             R11 15- 0          15- 0          15- 0
 *             R12 15- 0    RR12  31-16    RQ12  63-48
 *             R13 15- 0          15- 0          47-32
 *             R14 15- 0    RR14  31-16          31-16
 *             R15 15- 0          15- 0          15- 0
 *
 * Note that for LSB_FIRST machines we have the case that the RR registers
 * use the lower numbered R registers in the higher bit positions.
 * And also the RQ registers use the lower numbered RR registers in the
 * higher bit positions.
 * That's the reason for the ordering in the following pointer table.
 **************************************************************************/
#define RB(n)   m_regs.B[BYTE8_XOR_BE((((n) & 7) << 1) | (((n) & 8) >> 3))]
#define RW(n)   m_regs.W[BYTE4_XOR_BE(n)]
#define RL(n)   m_regs.L[BYTE_XOR_BE((n) >> 1)]
#define RQ(n)   m_regs.Q[(n) >> 2]

/* the register used as stack pointer */
#define SP      (segmented_mode() ? 14 : 15)

/* these vectors are based on m_psap */
#define RST     (PSA_ADDR() + 0)  /* start up m_fcw and m_pc */
#define EPU     (PSA_ADDR() + m_vector_mult * 0x0004)  /* extension processor unit? trap */
#define TRAP    (PSA_ADDR() + m_vector_mult * 0x0008)  /* privilege violation trap */
#define SYSCALL (PSA_ADDR() + m_vector_mult * 0x000c)  /* system call SC */
#define SEGTRAP (PSA_ADDR() + m_vector_mult * 0x0010)  /* segment trap */
#define NMI     (PSA_ADDR() + m_vector_mult * 0x0014)  /* non maskable interrupt */
#define NVI     (PSA_ADDR() + m_vector_mult * 0x0018)  /* non vectored interrupt */
#define VI      (PSA_ADDR() + m_vector_mult * 0x001c)  /* vectored interrupt */
#define VEC00   (PSA_ADDR() + m_vector_mult * 0x001e)  /* vector n m_pc value */

/* bits of the m_fcw */
#define F_SEG   0x8000              /* segmented mode (Z8001 only) */
#define F_S_N   0x4000              /* system / normal mode */
#define F_EPU   0x2000              /* extension processor unit? */
#define F_VIE   0x1000              /* vectored interrupt enable */
#define F_NVIE  0x0800              /* non vectored interrupt enable */
#define F_10    0x0400              /* unused */
#define F_9     0x0200              /* unused */
#define F_8     0x0100              /* unused */
#define F_C     0x0080              /* carry flag */
#define F_Z     0x0040              /* zero flag */
#define F_S     0x0020              /* sign flag */
#define F_PV    0x0010              /* parity/overflow flag */
#define F_DA    0x0008              /* decimal adjust flag (0 add/adc, 1 sub/sbc) */
#define F_H     0x0004              /* half carry flag (byte arithmetic only) */
#define F_1     0x0002              /* unused */
#define F_0     0x0001              /* unused */

/* opcode word numbers in m_op[] array */
#define OP0     0
#define OP1     1
#define OP2     2

/* nibble shift factors for an opcode word */
/* left to right: 0x1340 -> NIB0=1, NIB1=3, NIB2=4, NIB3=0 */
#define NIB0    12
#define NIB1    8
#define NIB2    4
#define NIB3    0

/* sign bit masks for byte, word and long */
#define S08 0x80
#define S16 0x8000
#define S32 0x80000000

/* get a single flag bit 0/1 */
#define GET_C       ((m_fcw >> 7) & 1)
#define GET_Z       ((m_fcw >> 6) & 1)
#define GET_S       ((m_fcw >> 5) & 1)
#define GET_PV      ((m_fcw >> 4) & 1)
#define GET_DA      ((m_fcw >> 3) & 1)
#define GET_H       ((m_fcw >> 2) & 1)

/* clear a single flag bit */
#define CLR_C       m_fcw &= ~F_C
#define CLR_Z       m_fcw &= ~F_Z
#define CLR_S       m_fcw &= ~F_S
#define CLR_P       m_fcw &= ~F_PV
#define CLR_V       m_fcw &= ~F_PV
#define CLR_DA      m_fcw &= ~F_DA
#define CLR_H       m_fcw &= ~F_H

/* clear a flag bit combination */
#define CLR_CZS     m_fcw &= ~(F_C|F_Z|F_S)
#define CLR_CZSP    m_fcw &= ~(F_C|F_Z|F_S|F_PV)
#define CLR_CZSV    m_fcw &= ~(F_C|F_Z|F_S|F_PV)
#define CLR_CZSVH   m_fcw &= ~(F_C|F_Z|F_S|F_PV|F_H)
#define CLR_ZS      m_fcw &= ~(F_Z|F_S)
#define CLR_ZSV     m_fcw &= ~(F_Z|F_S|F_PV)
#define CLR_ZSP     m_fcw &= ~(F_Z|F_S|F_PV)

/* set a single flag bit */
#define SET_C       m_fcw |= F_C
#define SET_Z       m_fcw |= F_Z
#define SET_S       m_fcw |= F_S
#define SET_P       m_fcw |= F_PV
#define SET_V       m_fcw |= F_PV
#define SET_DA      m_fcw |= F_DA
#define SET_H       m_fcw |= F_H

/* set a flag bit combination */
#define SET_SC      m_fcw |= F_C | F_S

/* check condition codes */
#define CC0 (0)                         /* always false */
#define CC1 (GET_PV^GET_S)              /* less than */
#define CC2 (GET_Z|(GET_PV^GET_S))      /* less than or equal */
#define CC3 (GET_Z|GET_C)               /* unsigned less than or equal */
#define CC4 GET_PV                      /* parity even / overflow */
#define CC5 GET_S                       /* minus (signed) */
#define CC6 GET_Z                       /* zero / equal */
#define CC7 GET_C                       /* carry / unsigned less than */

#define CC8 (1)                         /* always true */
#define CC9 !(GET_PV^GET_S)             /* greater than or equal */
#define CCA !(GET_Z|(GET_PV^GET_S))     /* greater than */
#define CCB !(GET_Z|GET_C)              /* unsigned greater than */
#define CCC !GET_PV                     /* parity odd / no overflow */
#define CCD !GET_S                      /* plus (not signed) */
#define CCE !GET_Z                      /* not zero / not equal */
#define CCF !GET_C                      /* not carry / unsigned greater than */

/* get data from the opcode words */
/* o is the opcode word offset    */
/* s is a nibble shift factor     */
#define GET_BIT(o)      UINT16 bit = 1 << (get_operand(o) & 15)
#define GET_CCC(o,s)    UINT8 cc = (get_operand(o) >> (s)) & 15

#define GET_DST(o,s)    UINT8 dst = (get_operand(o) >> (s)) & 15
#define GET_SRC(o,s)    UINT8 src = (get_operand(o) >> (s)) & 15
#define GET_IDX(o,s)    UINT8 idx = (get_operand(o) >> (s)) & 15
#define GET_CNT(o,s)    INT8 cnt = (get_operand(o) >> (s)) & 15
#define GET_IMM4(o,s)   UINT8 imm4 = (get_operand(o) >> (s)) & 15

#define GET_I4M1(o,s)   UINT8 i4p1 = ((get_operand(o) >> (s)) & 15) + 1
#define GET_IMM1(o,s)   UINT8 imm1 = (get_operand(o) >> (s)) & 2
#define GET_IMM2(o,s)   UINT8 imm2 = (get_operand(o) >> (s)) & 3
#define GET_IMM3(o,s)   UINT8 imm3 = (get_operand(o) >> (s)) & 7

#define GET_IMM8(o)     UINT8 imm8 = (UINT8)get_operand(o)

#define GET_IMM16(o)    UINT16 imm16 = get_operand(o)
#define GET_IDX16(o)    UINT32 idx16 = get_operand(o)
#define GET_IMM32       UINT32 imm32 = (get_operand(1) << 16) + get_operand(2)
#define GET_DSP7        UINT8 dsp7 = get_operand(0) & 127
#define GET_DSP8        INT8 dsp8 = (INT8)get_operand(0)
#define GET_DSP16       UINT32 dsp16 = addr_add(m_pc, (INT16)get_operand(1))
#define GET_ADDR(o)     UINT32 addr = (UINT32)get_addr_operand(o)
#define GET_ADDR_RAW(o)     UINT32 addr = (UINT32)get_raw_addr_operand(o)


/* structure for the opcode definition table */
typedef void (z8002_device::*opcode_func)();

struct Z8000_init {
	int     beg, end, step;
	int     size, cycles;
	opcode_func opcode;
	const char  *dasm;
	UINT32 dasmflags;
};

/* structure for the opcode execution table / disassembler */
struct Z8000_exec {
	opcode_func opcode;
	int     cycles;
	int     size;
	const char    *dasm;
	UINT32 dasmflags;
};

/* opcode execution table */
extern Z8000_exec *z8000_exec;

extern void z8000_init_tables(void);
extern void z8000_deinit_tables(void);
