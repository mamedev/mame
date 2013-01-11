#pragma once

#ifndef __Z180_H__
#define __Z180_H__


enum
{
	Z180_PC,
	Z180_SP,
	Z180_AF,
	Z180_BC,
	Z180_DE,
	Z180_HL,
	Z180_IX,
	Z180_IY,
	Z180_A,
	Z180_B,
	Z180_C,
	Z180_D,
	Z180_E,
	Z180_H,
	Z180_L,
	Z180_AF2,
	Z180_BC2,
	Z180_DE2,
	Z180_HL2,
	Z180_R,
	Z180_I,
	Z180_IM,
	Z180_IFF1,
	Z180_IFF2,
	Z180_HALT,
	Z180_DC0,
	Z180_DC1,
	Z180_DC2,
	Z180_DC3,
	Z180_CNTLA0,    /* 00 ASCI control register A ch 0 */
	Z180_CNTLA1,    /* 01 ASCI control register A ch 1 */
	Z180_CNTLB0,    /* 02 ASCI control register B ch 0 */
	Z180_CNTLB1,    /* 03 ASCI control register B ch 1 */
	Z180_STAT0,     /* 04 ASCI status register 0 */
	Z180_STAT1,     /* 05 ASCI status register 1 */
	Z180_TDR0,      /* 06 ASCI transmit data register 0 */
	Z180_TDR1,      /* 07 ASCI transmit data register 1 */
	Z180_RDR0,      /* 08 ASCI receive data register 0 */
	Z180_RDR1,      /* 09 ASCI receive data register 1 */
	Z180_CNTR,      /* 0a CSI/O control/status register */
	Z180_TRDR,      /* 0b CSI/O transmit/receive register */
	Z180_TMDR0L,    /* 0c TIMER data register ch 0 L */
	Z180_TMDR0H,    /* 0d TIMER data register ch 0 H */
	Z180_RLDR0L,    /* 0e TIMER reload register ch 0 L */
	Z180_RLDR0H,    /* 0f TIMER reload register ch 0 H */
	Z180_TCR,       /* 10 TIMER control register */
	Z180_IO11,      /* 11 reserved */
	Z180_ASEXT0,    /* 12 (Z8S180/Z8L180) ASCI extension control register 0 */
	Z180_ASEXT1,    /* 13 (Z8S180/Z8L180) ASCI extension control register 0 */
	Z180_TMDR1L,    /* 14 TIMER data register ch 1 L */
	Z180_TMDR1H,    /* 15 TIMER data register ch 1 H */
	Z180_RLDR1L,    /* 16 TIMER reload register ch 1 L */
	Z180_RLDR1H,    /* 17 TIMER reload register ch 1 H */
	Z180_FRC,       /* 18 free running counter */
	Z180_IO19,      /* 19 reserved */
	Z180_ASTC0L,    /* 1a ASCI time constant ch 0 L */
	Z180_ASTC0H,    /* 1b ASCI time constant ch 0 H */
	Z180_ASTC1L,    /* 1c ASCI time constant ch 1 L */
	Z180_ASTC1H,    /* 1d ASCI time constant ch 1 H */
	Z180_CMR,       /* 1e clock multiplier */
	Z180_CCR,       /* 1f chip control register */
	Z180_SAR0L,     /* 20 DMA source address register ch 0 L */
	Z180_SAR0H,     /* 21 DMA source address register ch 0 H */
	Z180_SAR0B,     /* 22 DMA source address register ch 0 B */
	Z180_DAR0L,     /* 23 DMA destination address register ch 0 L */
	Z180_DAR0H,     /* 24 DMA destination address register ch 0 H */
	Z180_DAR0B,     /* 25 DMA destination address register ch 0 B */
	Z180_BCR0L,     /* 26 DMA byte count register ch 0 L */
	Z180_BCR0H,     /* 27 DMA byte count register ch 0 H */
	Z180_MAR1L,     /* 28 DMA memory address register ch 1 L */
	Z180_MAR1H,     /* 29 DMA memory address register ch 1 H */
	Z180_MAR1B,     /* 2a DMA memory address register ch 1 B */
	Z180_IAR1L,     /* 2b DMA I/O address register ch 1 L */
	Z180_IAR1H,     /* 2c DMA I/O address register ch 1 H */
	Z180_IAR1B,     /* 2d (Z8S180/Z8L180) DMA I/O address register ch 1 B */
	Z180_BCR1L,     /* 2e DMA byte count register ch 1 L */
	Z180_BCR1H,     /* 2f DMA byte count register ch 1 H */
	Z180_DSTAT,     /* 30 DMA status register */
	Z180_DMODE,     /* 31 DMA mode register */
	Z180_DCNTL,     /* 32 DMA/WAIT control register */
	Z180_IL,        /* 33 INT vector low register */
	Z180_ITC,       /* 34 INT/TRAP control register */
	Z180_IO35,      /* 35 reserved */
	Z180_RCR,       /* 36 refresh control register */
	Z180_IO37,      /* 37 reserved */
	Z180_CBR,       /* 38 MMU common base register */
	Z180_BBR,       /* 39 MMU bank base register */
	Z180_CBAR,      /* 3a MMU common/bank area register */
	Z180_IO3B,      /* 3b reserved */
	Z180_IO3C,      /* 3c reserved */
	Z180_IO3D,      /* 3d reserved */
	Z180_OMCR,      /* 3e operation mode control register */
	Z180_IOCR,      /* 3f I/O control register */
	Z180_IOLINES,   /* read/write I/O lines */

	Z180_GENPC = STATE_GENPC,
	Z180_GENSP = STATE_GENSP,
	Z180_GENPCBASE = STATE_GENPCBASE
};

enum
{
	Z180_TABLE_op,
	Z180_TABLE_cb,
	Z180_TABLE_ed,
	Z180_TABLE_xy,
	Z180_TABLE_xycb,
	Z180_TABLE_ex    /* cycles counts for taken jr/jp/call and interrupt latency (rst opcodes) */
};

enum
{
	CPUINFO_PTR_Z180_CYCLE_TABLE = CPUINFO_PTR_CPU_SPECIFIC,
	CPUINFO_PTR_Z180_CYCLE_TABLE_LAST = CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_ex
};


#define Z180_IRQ0       0           /* Execute IRQ1 */
#define Z180_IRQ1       1           /* Execute IRQ1 */
#define Z180_IRQ2       2           /* Execute IRQ2 */


#ifdef UNUSED_DEFINITION
/* MMU mapped memory lookup */
extern UINT8 z180_readmem(device_t *device, offs_t offset);
extern void z180_writemem(device_t *device, offs_t offset, UINT8 data);
#endif

DECLARE_LEGACY_CPU_DEVICE(Z180, z180);

CPU_DISASSEMBLE( z180 );

#endif /* __Z180_H__ */
