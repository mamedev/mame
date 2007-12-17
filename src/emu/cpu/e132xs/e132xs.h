#ifndef E132XS_H
#define E132XS_H

#include "cpuintrf.h"

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

#ifdef MAME_DEBUG
extern unsigned dasm_hyperstone(char *buffer, unsigned pc, const UINT8 *oprom, unsigned h_flag, int private_fp);
#endif

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
#define DELAY_TAKEN		2

/* Trap numbers */
#define IO2					48
#define IO1					49
#define INT4				50
#define INT3				51
#define INT2				52
#define INT1				53
#define IO3					54
#define TIMER				55
#define RESERVED1			56
#define TRACE_EXCEPTION		57
#define PARITY_ERROR		58
#define EXTENDED_OVERFLOW	59
#define RANGE_ERROR			60
#define PRIVILEGE_ERROR		RANGE_ERROR
#define FRAME_ERROR			RANGE_ERROR
#define RESERVED2			61
#define RESET				62	// reserved if not mapped @ MEM3
#define ERROR_ENTRY			63	// for instruction code of all ones

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
