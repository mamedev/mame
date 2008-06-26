#ifndef E132XS_H
#define E132XS_H

#include "cpuintrf.h"

/*
    A note about clock multipliers and dividers:

    E1-16[T] and E1-32[T] accept a straight clock

    E1-16X[T|N] and E1-32X[T|N] accept a clock and multiply it
        internally by 4; in the emulator, you MUST specify 4 * XTAL
        to achieve the correct speed

    E1-16XS[R] and E1-32XS[R] accept a clock and multiply it
        internally by 8; in the emulator, you MUST specify 8 * XTAL
        to achieve the correct speed
*/



/* Functions */

#if (HAS_E116T)
void e116t_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_E116XT)
void e116xt_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_E116XS)
void e116xs_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_E116XSR)
void e116xsr_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_E132N)
void e132n_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_E132T)
void e132t_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_E132XN)
void e132xn_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_E132XT)
void e132xt_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_E132XS)
void e132xs_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_E132XSR)
void e132xsr_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_GMS30C2116)
void gms30c2116_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_GMS30C2132)
void gms30c2132_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_GMS30C2216)
void gms30c2216_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_GMS30C2232)
void gms30c2232_get_info(UINT32 state, cpuinfo *info);
#endif

extern unsigned dasm_hyperstone(char *buffer, unsigned pc, const UINT8 *oprom, unsigned h_flag, int private_fp);

extern int hyp_type_16bit;

/* Memory access */
/* read byte */
#define READ_B(addr)           ((*hyp_cpu_read_byte)(addr))
/* read half-word */
#define READ_HW(addr)          ((*hyp_cpu_read_half_word)((addr) & ~1))
/* read word */
#define READ_W(addr)           ((*hyp_cpu_read_word)((addr) & ~3))

/* write byte */
#define WRITE_B(addr, data)    ((*hyp_cpu_write_byte)(addr, data))
/* write half-word */
#define WRITE_HW(addr, data)   ((*hyp_cpu_write_half_word)((addr) & ~1, data))
/* write word */
#define WRITE_W(addr, data)    ((*hyp_cpu_write_word)((addr) & ~3, data))


/* I/O access */
/* read word */
#define IO_READ_W(addr)        ((*hyp_cpu_read_io_word)(((addr) >> 11) & 0x7ffc))
/* write word */
#define IO_WRITE_W(addr, data) ((*hyp_cpu_write_io_word)(((addr) >> 11) & 0x7ffc, data))


#define READ_OP(addr)	       (cpu_readop16(hyp_type_16bit ? addr: WORD_XOR_BE(addr)))


/* Registers Number */
#define PC_REGISTER			 0
#define SR_REGISTER			 1
#define BCR_REGISTER		20
#define TPR_REGISTER		21
#define TCR_REGISTER		22
#define TR_REGISTER			23
#define ISR_REGISTER		25
#define FCR_REGISTER		26
#define MCR_REGISTER		27

#define X_CODE(val)		 ((val & 0x7000) >> 12)
#define E_BIT(val)		 ((val & 0x8000) >> 15)
#define S_BIT_CONST(val) ((val & 0x4000) >> 14)
#define DD(val)			 ((val & 0x3000) >> 12)


/* Extended DSP instructions */
#define EMUL			0x102
#define EMULU			0x104
#define EMULS			0x106
#define EMAC			0x10a
#define EMACD			0x10e
#define EMSUB			0x11a
#define EMSUBD			0x11e
#define EHMAC			0x02a
#define EHMACD			0x02e
#define EHCMULD			0x046
#define EHCMACD			0x04e
#define EHCSUMD			0x086
#define EHCFFTD			0x096
#define EHCFFTSD		0x296

/* Delay values */
#define NO_DELAY		0
#define DELAY_EXECUTE	1

/* IRQ numbers */
#define IRQ_INT1		0
#define IRQ_INT2		1
#define IRQ_INT3		2
#define IRQ_INT4		3
#define IRQ_IO1			4
#define IRQ_IO2			5
#define IRQ_IO3			6

/* Trap numbers */
#define TRAPNO_IO2					48
#define TRAPNO_IO1					49
#define TRAPNO_INT4				50
#define TRAPNO_INT3				51
#define TRAPNO_INT2				52
#define TRAPNO_INT1				53
#define TRAPNO_IO3					54
#define TRAPNO_TIMER				55
#define TRAPNO_RESERVED1			56
#define TRAPNO_TRACE_EXCEPTION		57
#define TRAPNO_PARITY_ERROR		58
#define TRAPNO_EXTENDED_OVERFLOW	59
#define TRAPNO_RANGE_ERROR			60
#define TRAPNO_PRIVILEGE_ERROR		TRAPNO_RANGE_ERROR
#define TRAPNO_FRAME_ERROR			TRAPNO_RANGE_ERROR
#define TRAPNO_RESERVED2			61
#define TRAPNO_RESET				62	// reserved if not mapped @ MEM3
#define TRAPNO_ERROR_ENTRY			63	// for instruction code of all ones

/* Traps code */
#define	TRAPLE		4
#define	TRAPGT		5
#define	TRAPLT		6
#define	TRAPGE		7
#define	TRAPSE		8
#define	TRAPHT		9
#define	TRAPST		10
#define	TRAPHE		11
#define	TRAPE		12
#define	TRAPNE		13
#define	TRAPV		14
#define	TRAP		15

/* Entry point to get trap locations or emulated code associated */
#define	E132XS_ENTRY_MEM0	0
#define	E132XS_ENTRY_MEM1	1
#define	E132XS_ENTRY_MEM2	2
#define	E132XS_ENTRY_IRAM	3
#define	E132XS_ENTRY_MEM3	7

#endif /* E132XS_H */
