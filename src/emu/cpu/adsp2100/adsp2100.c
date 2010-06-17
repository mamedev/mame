/***************************************************************************

        ADSP2100.c
        Core implementation for the portable Analog ADSP-2100 emulator.
        Written by Aaron Giles

****************************************************************************

    For ADSP-2101, ADSP-2111
    ------------------------

        MMAP = 0                                        MMAP = 1

        Automatic boot loading                          No auto boot loading

        Program Space:                                  Program Space:
            0000-07ff = 2k Internal RAM (booted)            0000-37ff = 14k External access
            0800-3fff = 14k External access                 3800-3fff = 2k Internal RAM

        Data Space:                                     Data Space:
            0000-03ff = 1k External DWAIT0                  0000-03ff = 1k External DWAIT0
            0400-07ff = 1k External DWAIT1                  0400-07ff = 1k External DWAIT1
            0800-2fff = 10k External DWAIT2                 0800-2fff = 10k External DWAIT2
            3000-33ff = 1k External DWAIT3                  3000-33ff = 1k External DWAIT3
            3400-37ff = 1k External DWAIT4                  3400-37ff = 1k External DWAIT4
            3800-3bff = 1k Internal RAM                     3800-3bff = 1k Internal RAM
            3c00-3fff = 1k Internal Control regs            3c00-3fff = 1k Internal Control regs


    For ADSP-2105, ADSP-2115
    ------------------------

        MMAP = 0                                        MMAP = 1

        Automatic boot loading                          No auto boot loading

        Program Space:                                  Program Space:
            0000-03ff = 1k Internal RAM (booted)            0000-37ff = 14k External access
            0400-07ff = 1k Reserved                         3800-3bff = 1k Internal RAM
            0800-3fff = 14k External access                 3c00-3fff = 1k Reserved

        Data Space:                                     Data Space:
            0000-03ff = 1k External DWAIT0                  0000-03ff = 1k External DWAIT0
            0400-07ff = 1k External DWAIT1                  0400-07ff = 1k External DWAIT1
            0800-2fff = 10k External DWAIT2                 0800-2fff = 10k External DWAIT2
            3000-33ff = 1k External DWAIT3                  3000-33ff = 1k External DWAIT3
            3400-37ff = 1k External DWAIT4                  3400-37ff = 1k External DWAIT4
            3800-39ff = 512 Internal RAM                    3800-39ff = 512 Internal RAM
            3a00-3bff = 512 Reserved                        3a00-3bff = 512 Reserved
            3c00-3fff = 1k Internal Control regs            3c00-3fff = 1k Internal Control regs


    For ADSP-2104
    -------------

        MMAP = 0                                        MMAP = 1

        Automatic boot loading                          No auto boot loading

        Program Space:                                  Program Space:
            0000-01ff = 512 Internal RAM (booted)           0000-37ff = 14k External access
            0400-07ff = 1k Reserved                         3800-3bff = 1k Internal RAM
            0800-3fff = 14k External access                 3c00-3fff = 1k Reserved

        Data Space:                                     Data Space:
            0000-03ff = 1k External DWAIT0                  0000-03ff = 1k External DWAIT0
            0400-07ff = 1k External DWAIT1                  0400-07ff = 1k External DWAIT1
            0800-2fff = 10k External DWAIT2                 0800-2fff = 10k External DWAIT2
            3000-33ff = 1k External DWAIT3                  3000-33ff = 1k External DWAIT3
            3400-37ff = 1k External DWAIT4                  3400-37ff = 1k External DWAIT4
            3800-38ff = 256 Internal RAM                    3800-38ff = 256 Internal RAM
            3a00-3bff = 512 Reserved                        3a00-3bff = 512 Reserved
            3c00-3fff = 1k Internal Control regs            3c00-3fff = 1k Internal Control regs


    For ADSP-2181
    -------------

        MMAP = 0                                        MMAP = 1

        Program Space:                                  Program Space:
            0000-1fff = 8k Internal RAM                     0000-1fff = 8k External access
            2000-3fff = 8k Internal RAM or Overlay          2000-3fff = 8k Internal

        Data Space:                                     Data Space:
            0000-1fff = 8k Internal RAM or Overlay          0000-1fff = 8k Internal RAM or Overlay
            2000-3fdf = 8k-32 Internal RAM                  2000-3fdf = 8k-32 Internal RAM
            3fe0-3fff = 32 Internal Control regs            3fe0-3fff = 32 Internal Control regs

        I/O Space:                                      I/O Space:
            0000-01ff = 512 External IOWAIT0                0000-01ff = 512 External IOWAIT0
            0200-03ff = 512 External IOWAIT1                0200-03ff = 512 External IOWAIT1
            0400-05ff = 512 External IOWAIT2                0400-05ff = 512 External IOWAIT2
            0600-07ff = 512 External IOWAIT3                0600-07ff = 512 External IOWAIT3

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "adsp2100.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define TRACK_HOTSPOTS		0

/* stack depths */
#define	PC_STACK_DEPTH		16
#define CNTR_STACK_DEPTH	4
#define STAT_STACK_DEPTH	4
#define LOOP_STACK_DEPTH	4

/* chip types */
#define CHIP_TYPE_ADSP2100	0
#define CHIP_TYPE_ADSP2101	1
#define CHIP_TYPE_ADSP2104	2
#define CHIP_TYPE_ADSP2105	3
#define CHIP_TYPE_ADSP2115	4
#define CHIP_TYPE_ADSP2181	5



/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

/* 16-bit registers that can be loaded signed or unsigned */
typedef union
{
	UINT16	u;
	INT16	s;
} ADSPREG16;


/* the SHIFT result register is 32 bits */
typedef union
{
#ifdef LSB_FIRST
	struct { ADSPREG16 sr0, sr1; } srx;
#else
	struct { ADSPREG16 sr1, sr0; } srx;
#endif
	UINT32 sr;
} SHIFTRESULT;


/* the MAC result register is 40 bits */
typedef union
{
#ifdef LSB_FIRST
	struct { ADSPREG16 mr0, mr1, mr2, mrzero; } mrx;
	struct { UINT32 mr0, mr1; } mry;
#else
	struct { ADSPREG16 mrzero, mr2, mr1, mr0; } mrx;
	struct { UINT32 mr1, mr0; } mry;
#endif
	UINT64 mr;
} MACRESULT;

/* there are two banks of "core" registers */
typedef struct ADSPCORE
{
	/* ALU registers */
	ADSPREG16	ax0, ax1;
	ADSPREG16	ay0, ay1;
	ADSPREG16	ar;
	ADSPREG16	af;

	/* MAC registers */
	ADSPREG16	mx0, mx1;
	ADSPREG16	my0, my1;
	MACRESULT	mr;
	ADSPREG16	mf;

	/* SHIFT registers */
	ADSPREG16	si;
	ADSPREG16	se;
	ADSPREG16	sb;
	SHIFTRESULT	sr;

	/* dummy registers */
	ADSPREG16	zero;
} ADSPCORE;


/* ADSP-2100 Registers */
typedef struct
{
	/* Core registers, 2 banks */
	ADSPCORE	core;
	ADSPCORE	alt;

	/* Memory addressing registers */
	UINT32		i[8];
	INT32		m[8];
	UINT32		l[8];
	UINT32		lmask[8];
	UINT32		base[8];
	UINT8		px;

	/* other CPU registers */
	UINT32		pc;
	UINT32		ppc;
	UINT32		loop;
	UINT32		loop_condition;
	UINT32		cntr;

	/* status registers */
	UINT32		astat;
	UINT32		sstat;
	UINT32		mstat;
	UINT32		mstat_prev;
	UINT32		astat_clear;
	UINT32		idle;

	/* stacks */
	UINT32		loop_stack[LOOP_STACK_DEPTH];
	UINT32		cntr_stack[CNTR_STACK_DEPTH];
	UINT32		pc_stack[PC_STACK_DEPTH];
	UINT16		stat_stack[STAT_STACK_DEPTH][3];
	INT32		pc_sp;
	INT32		cntr_sp;
	INT32		stat_sp;
	INT32		loop_sp;

	/* external I/O */
	UINT8		flagout;
	UINT8		flagin;
	UINT8		fl0;
	UINT8		fl1;
	UINT8		fl2;
	UINT16		idma_addr;
	UINT16		idma_cache;
	UINT8		idma_offs;

	/* interrupt handling */
	UINT16		imask;
	UINT8		icntl;
	UINT16		ifc;
    UINT8   	irq_state[9];
    UINT8   	irq_latch[9];
    device_irq_callback irq_callback;
    legacy_cpu_device *device;

	/* other internal states */
    int			icount;
	int			chip_type;
	int			mstat_mask;
	int			imask_mask;

	/* register maps */
	void *		alu_xregs[8];
	void *		alu_yregs[4];
	void *		mac_xregs[8];
	void *		mac_yregs[4];
	void *		shift_xregs[8];

    /* other callbacks */
	adsp21xx_rx_func sport_rx_callback;
	adsp21xx_tx_func sport_tx_callback;
	adsp21xx_timer_func timer_fired;

	/* memory spaces */
    const address_space *program;
    const address_space *data;
    const address_space *io;

} adsp2100_state;



/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

static UINT16 *reverse_table = 0;
static UINT16 *mask_table = 0;
static UINT8 *condition_table = 0;

#if TRACK_HOTSPOTS
static UINT32 pcbucket[0x4000];
#endif



/***************************************************************************
    PRIVATE FUNCTION PROTOTYPES
***************************************************************************/

static int create_tables(void);
static void check_irqs(adsp2100_state *adsp);



/***************************************************************************
    STATE ACCESSORS
***************************************************************************/

INLINE adsp2100_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == CPU);
	assert(cpu_get_type(device) == CPU_ADSP2100 ||
		   cpu_get_type(device) == CPU_ADSP2101 ||
		   cpu_get_type(device) == CPU_ADSP2104 ||
		   cpu_get_type(device) == CPU_ADSP2105 ||
		   cpu_get_type(device) == CPU_ADSP2115 ||
		   cpu_get_type(device) == CPU_ADSP2181);
	return (adsp2100_state *)downcast<legacy_cpu_device *>(device)->token();
}



/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

INLINE UINT16 RWORD_DATA(adsp2100_state *adsp, UINT32 addr)
{
	return memory_read_word_16le(adsp->data, addr << 1);
}

INLINE void WWORD_DATA(adsp2100_state *adsp, UINT32 addr, UINT16 data)
{
	memory_write_word_16le(adsp->data, addr << 1, data);
}

INLINE UINT16 RWORD_IO(adsp2100_state *adsp, UINT32 addr)
{
	return memory_read_word_16le(adsp->io, addr << 1);
}

INLINE void WWORD_IO(adsp2100_state *adsp, UINT32 addr, UINT16 data)
{
	memory_write_word_16le(adsp->io, addr << 1, data);
}

INLINE UINT32 RWORD_PGM(adsp2100_state *adsp, UINT32 addr)
{
	return memory_read_dword_32le(adsp->program, addr << 2);
}

INLINE void WWORD_PGM(adsp2100_state *adsp, UINT32 addr, UINT32 data)
{
	memory_write_dword_32le(adsp->program, addr << 2, data & 0xffffff);
}

#define ROPCODE(a) memory_decrypted_read_dword((a)->program, (a)->pc << 2)


/***************************************************************************
    IMPORT CORE UTILITIES
***************************************************************************/

#include "2100ops.c"



/***************************************************************************
    IRQ HANDLING
***************************************************************************/

INLINE int adsp2100_generate_irq(adsp2100_state *adsp, int which)
{
	/* skip if masked */
	if (!(adsp->imask & (1 << which)))
		return 0;

	/* clear the latch */
	adsp->irq_latch[which] = 0;

	/* push the PC and the status */
	pc_stack_push(adsp);
	stat_stack_push(adsp);

	/* vector to location & stop idling */
	adsp->pc = which;
	adsp->idle = 0;

	/* mask other interrupts based on the nesting bit */
	if (adsp->icntl & 0x10) adsp->imask &= ~((2 << which) - 1);
	else adsp->imask &= ~0xf;

	return 1;
}


INLINE int adsp2101_generate_irq(adsp2100_state *adsp, int which, int indx)
{
	/* skip if masked */
	if (!(adsp->imask & (0x20 >> indx)))
		return 0;

	/* clear the latch */
	adsp->irq_latch[which] = 0;

	/* push the PC and the status */
	pc_stack_push(adsp);
	stat_stack_push(adsp);

	/* vector to location & stop idling */
	adsp->pc = 0x04 + indx * 4;
	adsp->idle = 0;

	/* mask other interrupts based on the nesting bit */
	if (adsp->icntl & 0x10) adsp->imask &= ~(0x3f >> indx);
	else adsp->imask &= ~0x3f;

	return 1;
}


INLINE int adsp2181_generate_irq(adsp2100_state *adsp, int which, int indx)
{
	/* skip if masked */
	if (!(adsp->imask & (0x200 >> indx)))
		return 0;

	/* clear the latch */
	adsp->irq_latch[which] = 0;

	/* push the PC and the status */
	pc_stack_push(adsp);
	stat_stack_push(adsp);

	/* vector to location & stop idling */
	adsp->pc = 0x04 + indx * 4;
	adsp->idle = 0;

	/* mask other interrupts based on the nesting bit */
	if (adsp->icntl & 0x10) adsp->imask &= ~(0x3ff >> indx);
	else adsp->imask &= ~0x3ff;

	return 1;
}


static void check_irqs(adsp2100_state *adsp)
{
	UINT8 check;

	if (adsp->chip_type >= CHIP_TYPE_ADSP2181)
	{
		/* check IRQ2 */
		check = (adsp->icntl & 4) ? adsp->irq_latch[ADSP2181_IRQ2] : adsp->irq_state[ADSP2181_IRQ2];
		if (check && adsp2181_generate_irq(adsp, ADSP2181_IRQ2, 0))
			return;

		/* check IRQL1 */
		check = adsp->irq_state[ADSP2181_IRQL1];
		if (check && adsp2181_generate_irq(adsp, ADSP2181_IRQL1, 1))
			return;

		/* check IRQL2 */
		check = adsp->irq_state[ADSP2181_IRQL2];
		if (check && adsp2181_generate_irq(adsp, ADSP2181_IRQL2, 2))
			return;

		/* check SPORT0 transmit */
		check = adsp->irq_latch[ADSP2181_SPORT0_TX];
		if (check && adsp2181_generate_irq(adsp, ADSP2181_SPORT0_TX, 3))
			return;

		/* check SPORT0 receive */
		check = adsp->irq_latch[ADSP2181_SPORT0_RX];
		if (check && adsp2181_generate_irq(adsp, ADSP2181_SPORT0_RX, 4))
			return;

		/* check IRQE */
		check = adsp->irq_latch[ADSP2181_IRQE];
		if (check && adsp2181_generate_irq(adsp, ADSP2181_IRQE, 5))
			return;

		/* check BDMA interrupt */

		/* check IRQ1/SPORT1 transmit */
		check = (adsp->icntl & 2) ? adsp->irq_latch[ADSP2181_IRQ1] : adsp->irq_state[ADSP2181_IRQ1];
		if (check && adsp2181_generate_irq(adsp, ADSP2181_IRQ1, 7))
			return;

		/* check IRQ0/SPORT1 receive */
		check = (adsp->icntl & 1) ? adsp->irq_latch[ADSP2181_IRQ0] : adsp->irq_state[ADSP2181_IRQ0];
		if (check && adsp2181_generate_irq(adsp, ADSP2181_IRQ0, 8))
			return;

		/* check timer */
		check = adsp->irq_latch[ADSP2181_TIMER];
		if (check && adsp2181_generate_irq(adsp, ADSP2181_TIMER, 9))
			return;
	}
	else if (adsp->chip_type >= CHIP_TYPE_ADSP2101)
	{
		/* check IRQ2 */
		check = (adsp->icntl & 4) ? adsp->irq_latch[ADSP2101_IRQ2] : adsp->irq_state[ADSP2101_IRQ2];
		if (check && adsp2101_generate_irq(adsp, ADSP2101_IRQ2, 0))
			return;

		/* check SPORT0 transmit */
		check = adsp->irq_latch[ADSP2101_SPORT0_TX];
		if (check && adsp2101_generate_irq(adsp, ADSP2101_SPORT0_TX, 1))
			return;

		/* check SPORT0 receive */
		check = adsp->irq_latch[ADSP2101_SPORT0_RX];
		if (check && adsp2101_generate_irq(adsp, ADSP2101_SPORT0_RX, 2))
			return;

		/* check IRQ1/SPORT1 transmit */
		check = (adsp->icntl & 2) ? adsp->irq_latch[ADSP2101_IRQ1] : adsp->irq_state[ADSP2101_IRQ1];
		if (check && adsp2101_generate_irq(adsp, ADSP2101_IRQ1, 3))
			return;

		/* check IRQ0/SPORT1 receive */
		check = (adsp->icntl & 1) ? adsp->irq_latch[ADSP2101_IRQ0] : adsp->irq_state[ADSP2101_IRQ0];
		if (check && adsp2101_generate_irq(adsp, ADSP2101_IRQ0, 4))
			return;

		/* check timer */
		check = adsp->irq_latch[ADSP2101_TIMER];
		if (check && adsp2101_generate_irq(adsp, ADSP2101_TIMER, 5))
			return;
	}
	else
	{
		/* check IRQ3 */
		check = (adsp->icntl & 8) ? adsp->irq_latch[ADSP2100_IRQ3] : adsp->irq_state[ADSP2100_IRQ3];
		if (check && adsp2100_generate_irq(adsp, ADSP2100_IRQ3))
			return;

		/* check IRQ2 */
		check = (adsp->icntl & 4) ? adsp->irq_latch[ADSP2100_IRQ2] : adsp->irq_state[ADSP2100_IRQ2];
		if (check && adsp2100_generate_irq(adsp, ADSP2100_IRQ2))
			return;

		/* check IRQ1 */
		check = (adsp->icntl & 2) ? adsp->irq_latch[ADSP2100_IRQ1] : adsp->irq_state[ADSP2100_IRQ1];
		if (check && adsp2100_generate_irq(adsp, ADSP2100_IRQ1))
			return;

		/* check IRQ0 */
		check = (adsp->icntl & 1) ? adsp->irq_latch[ADSP2100_IRQ0] : adsp->irq_state[ADSP2100_IRQ0];
		if (check && adsp2100_generate_irq(adsp, ADSP2100_IRQ0))
			return;
	}
}


static void set_irq_line(adsp2100_state *adsp, int irqline, int state)
{
	/* update the latched state */
	if (state != CLEAR_LINE && adsp->irq_state[irqline] == CLEAR_LINE)
    	adsp->irq_latch[irqline] = 1;

    /* update the absolute state */
    adsp->irq_state[irqline] = state;
}



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

static adsp2100_state *adsp21xx_init(legacy_cpu_device *device, device_irq_callback irqcallback, int chiptype)
{
	const adsp21xx_config *config = (const adsp21xx_config *)device->baseconfig().static_config();
	adsp2100_state *adsp = get_safe_token(device);

	/* create the tables */
	if (!create_tables())
		fatalerror("creating adsp2100 tables failed");

	/* set the IRQ callback */
	adsp->chip_type = chiptype;
	adsp->irq_callback = irqcallback;

	/* fetch device parameters */
	adsp->device = device;
	adsp->program = device->space(AS_PROGRAM);
	adsp->data = device->space(AS_DATA);
	adsp->io = device->space(AS_IO);

	/* copy function pointers from the config */
	if (config != NULL)
	{
		adsp->sport_rx_callback = config->rx;
		adsp->sport_tx_callback = config->tx;
		adsp->timer_fired = config->timer;
	}

	/* set up ALU register pointers */
	adsp->alu_xregs[0] = &adsp->core.ax0;
	adsp->alu_xregs[1] = &adsp->core.ax1;
	adsp->alu_xregs[2] = &adsp->core.ar;
	adsp->alu_xregs[3] = &adsp->core.mr.mrx.mr0;
	adsp->alu_xregs[4] = &adsp->core.mr.mrx.mr1;
	adsp->alu_xregs[5] = &adsp->core.mr.mrx.mr2;
	adsp->alu_xregs[6] = &adsp->core.sr.srx.sr0;
	adsp->alu_xregs[7] = &adsp->core.sr.srx.sr1;
	adsp->alu_yregs[0] = &adsp->core.ay0;
	adsp->alu_yregs[1] = &adsp->core.ay1;
	adsp->alu_yregs[2] = &adsp->core.af;
	adsp->alu_yregs[3] = &adsp->core.zero;

	/* set up MAC register pointers */
	adsp->mac_xregs[0] = &adsp->core.mx0;
	adsp->mac_xregs[1] = &adsp->core.mx1;
	adsp->mac_xregs[2] = &adsp->core.ar;
	adsp->mac_xregs[3] = &adsp->core.mr.mrx.mr0;
	adsp->mac_xregs[4] = &adsp->core.mr.mrx.mr1;
	adsp->mac_xregs[5] = &adsp->core.mr.mrx.mr2;
	adsp->mac_xregs[6] = &adsp->core.sr.srx.sr0;
	adsp->mac_xregs[7] = &adsp->core.sr.srx.sr1;
	adsp->mac_yregs[0] = &adsp->core.my0;
	adsp->mac_yregs[1] = &adsp->core.my1;
	adsp->mac_yregs[2] = &adsp->core.mf;
	adsp->mac_yregs[3] = &adsp->core.zero;

	/* set up shift register pointers */
	adsp->shift_xregs[0] = &adsp->core.si;
	adsp->shift_xregs[1] = &adsp->core.si;
	adsp->shift_xregs[2] = &adsp->core.ar;
	adsp->shift_xregs[3] = &adsp->core.mr.mrx.mr0;
	adsp->shift_xregs[4] = &adsp->core.mr.mrx.mr1;
	adsp->shift_xregs[5] = &adsp->core.mr.mrx.mr2;
	adsp->shift_xregs[6] = &adsp->core.sr.srx.sr0;
	adsp->shift_xregs[7] = &adsp->core.sr.srx.sr1;

	/* "core" */
	state_save_register_device_item(device, 0, adsp->core.ax0.u);
	state_save_register_device_item(device, 0, adsp->core.ax1.u);
	state_save_register_device_item(device, 0, adsp->core.ay0.u);
	state_save_register_device_item(device, 0, adsp->core.ay1.u);
	state_save_register_device_item(device, 0, adsp->core.ar.u);
	state_save_register_device_item(device, 0, adsp->core.af.u);
	state_save_register_device_item(device, 0, adsp->core.mx0.u);
	state_save_register_device_item(device, 0, adsp->core.mx1.u);
	state_save_register_device_item(device, 0, adsp->core.my0.u);
	state_save_register_device_item(device, 0, adsp->core.my1.u);
	state_save_register_device_item(device, 0, adsp->core.mr.mr);
	state_save_register_device_item(device, 0, adsp->core.mf.u);
	state_save_register_device_item(device, 0, adsp->core.si.u);
	state_save_register_device_item(device, 0, adsp->core.se.u);
	state_save_register_device_item(device, 0, adsp->core.sb.u);
	state_save_register_device_item(device, 0, adsp->core.sr.sr);
	state_save_register_device_item(device, 0, adsp->core.zero.u);

	/* "alt" */
	state_save_register_device_item(device, 0, adsp->alt.ax0.u);
	state_save_register_device_item(device, 0, adsp->alt.ax1.u);
	state_save_register_device_item(device, 0, adsp->alt.ay0.u);
	state_save_register_device_item(device, 0, adsp->alt.ay1.u);
	state_save_register_device_item(device, 0, adsp->alt.ar.u);
	state_save_register_device_item(device, 0, adsp->alt.af.u);
	state_save_register_device_item(device, 0, adsp->alt.mx0.u);
	state_save_register_device_item(device, 0, adsp->alt.mx1.u);
	state_save_register_device_item(device, 0, adsp->alt.my0.u);
	state_save_register_device_item(device, 0, adsp->alt.my1.u);
	state_save_register_device_item(device, 0, adsp->alt.mr.mr);
	state_save_register_device_item(device, 0, adsp->alt.mf.u);
	state_save_register_device_item(device, 0, adsp->alt.si.u);
	state_save_register_device_item(device, 0, adsp->alt.se.u);
	state_save_register_device_item(device, 0, adsp->alt.sb.u);
	state_save_register_device_item(device, 0, adsp->alt.sr.sr);
	state_save_register_device_item(device, 0, adsp->alt.zero.u);

	state_save_register_device_item_array(device, 0, adsp->i);
	state_save_register_device_item_array(device, 0, adsp->m);
	state_save_register_device_item_array(device, 0, adsp->l);
	state_save_register_device_item_array(device, 0, adsp->lmask);
	state_save_register_device_item_array(device, 0, adsp->base);
	state_save_register_device_item(device, 0, adsp->px);

	state_save_register_device_item(device, 0, adsp->pc);
	state_save_register_device_item(device, 0, adsp->ppc);
	state_save_register_device_item(device, 0, adsp->loop);
	state_save_register_device_item(device, 0, adsp->loop_condition);
	state_save_register_device_item(device, 0, adsp->cntr);
	state_save_register_device_item(device, 0, adsp->astat);
	state_save_register_device_item(device, 0, adsp->sstat);
	state_save_register_device_item(device, 0, adsp->mstat);
	state_save_register_device_item(device, 0, adsp->mstat_prev);
	state_save_register_device_item(device, 0, adsp->astat_clear);
	state_save_register_device_item(device, 0, adsp->idle);

	state_save_register_device_item_array(device, 0, adsp->loop_stack);
	state_save_register_device_item_array(device, 0, adsp->cntr_stack);
	state_save_register_device_item_array(device, 0, adsp->pc_stack);
	state_save_register_device_item_2d_array(device, 0, adsp->stat_stack);

	state_save_register_device_item(device, 0, adsp->pc_sp);
	state_save_register_device_item(device, 0, adsp->cntr_sp);
	state_save_register_device_item(device, 0, adsp->stat_sp);
	state_save_register_device_item(device, 0, adsp->loop_sp);

	state_save_register_device_item(device, 0, adsp->flagout);
	state_save_register_device_item(device, 0, adsp->flagin);
	state_save_register_device_item(device, 0, adsp->fl0);
	state_save_register_device_item(device, 0, adsp->fl1);
	state_save_register_device_item(device, 0, adsp->fl2);
	state_save_register_device_item(device, 0, adsp->idma_addr);
	state_save_register_device_item(device, 0, adsp->idma_cache);
	state_save_register_device_item(device, 0, adsp->idma_offs);

	state_save_register_device_item(device, 0, adsp->imask);
	state_save_register_device_item(device, 0, adsp->icntl);
	state_save_register_device_item(device, 0, adsp->ifc);
	state_save_register_device_item_array(device, 0, adsp->irq_state);
	state_save_register_device_item_array(device, 0, adsp->irq_latch);

	// eventually this will be built-in
	device_state_interface *state;
	device->interface(state);
	state->state_add(ADSP2100_PC,      "PC",        adsp->pc);
	state->state_add(STATE_GENPC,      "GENPC",     adsp->pc).noshow();
	state->state_add(STATE_GENPCBASE,  "GENPCBASE", adsp->ppc).noshow();
	state->state_add(STATE_GENFLAGS,   "GENFLAGS",  adsp->astat).mask(0xff).noshow().formatstr("%8s");

	state->state_add(ADSP2100_AX0,     "AX0",       adsp->core.ax0.u);
	state->state_add(ADSP2100_AX1,     "AX1",       adsp->core.ax1.u);
	state->state_add(ADSP2100_AY0,     "AY0",       adsp->core.ay0.u);
	state->state_add(ADSP2100_AY1,     "AY1",       adsp->core.ay1.u);
	state->state_add(ADSP2100_AR,      "AR",        adsp->core.ar.u);
	state->state_add(ADSP2100_AF,      "AF",        adsp->core.af.u);

	state->state_add(ADSP2100_MX0,     "MX0",       adsp->core.mx0.u);
	state->state_add(ADSP2100_MX1,     "MX1",       adsp->core.mx1.u);
	state->state_add(ADSP2100_MY0,     "MY0",       adsp->core.my0.u);
	state->state_add(ADSP2100_MY1,     "MY1",       adsp->core.my1.u);
	state->state_add(ADSP2100_MR0,     "MR0",       adsp->core.mr.mrx.mr0.u);
	state->state_add(ADSP2100_MR1,     "MR1",       adsp->core.mr.mrx.mr1.u);
	state->state_add(ADSP2100_MR2,     "MR2",       adsp->core.mr.mrx.mr2.u).signed_mask(0xff);
	state->state_add(ADSP2100_MF,      "MF",        adsp->core.mf.u);

	state->state_add(ADSP2100_SI,      "SI",        adsp->core.si.u);
	state->state_add(ADSP2100_SE,      "SE",        adsp->core.se.u).signed_mask(0xff);
	state->state_add(ADSP2100_SB,      "SB",        adsp->core.sb.u).signed_mask(0x1f);
	state->state_add(ADSP2100_SR0,     "SR0",       adsp->core.sr.srx.sr0.u);
	state->state_add(ADSP2100_SR1,     "SR1",       adsp->core.sr.srx.sr1.u);

	state->state_add(ADSP2100_AX0_SEC, "AX0_SEC",   adsp->alt.ax0.u);
	state->state_add(ADSP2100_AX1_SEC, "AX1_SEC",   adsp->alt.ax1.u);
	state->state_add(ADSP2100_AY0_SEC, "AY0_SEC",   adsp->alt.ay0.u);
	state->state_add(ADSP2100_AY1_SEC, "AY1_SEC",   adsp->alt.ay1.u);
	state->state_add(ADSP2100_AR_SEC,  "AR_SEC",    adsp->alt.ar.u);
	state->state_add(ADSP2100_AF_SEC,  "AF_SEC",    adsp->alt.af.u);

	state->state_add(ADSP2100_MX0_SEC, "MX0_SEC",   adsp->alt.mx0.u);
	state->state_add(ADSP2100_MX1_SEC, "MX1_SEC",   adsp->alt.mx1.u);
	state->state_add(ADSP2100_MY0_SEC, "MY0_SEC",   adsp->alt.my0.u);
	state->state_add(ADSP2100_MY1_SEC, "MY1_SEC",   adsp->alt.my1.u);
	state->state_add(ADSP2100_MR0_SEC, "MR0_SEC",   adsp->alt.mr.mrx.mr0.u);
	state->state_add(ADSP2100_MR1_SEC, "MR1_SEC",   adsp->alt.mr.mrx.mr1.u);
	state->state_add(ADSP2100_MR2_SEC, "MR2_SEC",   adsp->alt.mr.mrx.mr2.u).signed_mask(0xff);
	state->state_add(ADSP2100_MF_SEC,  "MF_SEC",    adsp->alt.mf.u);

	state->state_add(ADSP2100_SI_SEC,  "SI_SEC",    adsp->alt.si.u);
	state->state_add(ADSP2100_SE_SEC,  "SE_SEC",    adsp->alt.se.u).signed_mask(0xff);
	state->state_add(ADSP2100_SB_SEC,  "SB_SEC",    adsp->alt.sb.u).signed_mask(0x1f);
	state->state_add(ADSP2100_SR0_SEC, "SR0_SEC",   adsp->alt.sr.srx.sr0.u);
	state->state_add(ADSP2100_SR1_SEC, "SR1_SEC",   adsp->alt.sr.srx.sr1.u);

	astring tempstring;
	for (int ireg = 0; ireg < 8; ireg++)
		state->state_add(ADSP2100_I0 + ireg, tempstring.format("I%d", ireg), adsp->i[ireg]).mask(0x3fff).callimport();

	for (int lreg = 0; lreg < 8; lreg++)
		state->state_add(ADSP2100_L0 + lreg, tempstring.format("L%d", lreg), adsp->l[lreg]).mask(0x3fff).callimport();

	for (int mreg = 0; mreg < 8; mreg++)
		state->state_add(ADSP2100_M0 + mreg, tempstring.format("M%d", mreg), adsp->m[mreg]).signed_mask(0x3fff);

	state->state_add(ADSP2100_PX,      "PX",        adsp->px);
	state->state_add(ADSP2100_CNTR,    "CNTR",      adsp->cntr).mask(0x3fff);
	state->state_add(ADSP2100_ASTAT,   "ASTAT",     adsp->astat).mask(0xff);
	state->state_add(ADSP2100_SSTAT,   "SSTAT",     adsp->sstat).mask(0xff);
	state->state_add(ADSP2100_MSTAT,   "MSTAT",     adsp->mstat).mask((chiptype == CHIP_TYPE_ADSP2100) ? 0x0f : 0x7f).callimport();

	state->state_add(ADSP2100_PCSP,    "PCSP",      adsp->pc_sp).mask(0xff);
	state->state_add(STATE_GENSP,      "GENSP",     adsp->pc_sp).mask(0xff).noshow();
	state->state_add(ADSP2100_CNTRSP,  "CNTRSP",    adsp->cntr_sp).mask(0xf);
	state->state_add(ADSP2100_STATSP,  "STATSP",    adsp->stat_sp).mask(0xf);
	state->state_add(ADSP2100_LOOPSP,  "LOOPSP",    adsp->loop_sp).mask(0xf);

	state->state_add(ADSP2100_IMASK,   "IMASK",     adsp->imask).mask((chiptype == CHIP_TYPE_ADSP2100) ? 0x00f : (chiptype == CHIP_TYPE_ADSP2181) ? 0x3ff : 0x07f).callimport();
	state->state_add(ADSP2100_ICNTL,   "ICNTL",     adsp->icntl).mask(0x1f).callimport();

	for (int irqnum = 0; irqnum < 4; irqnum++)
		if (irqnum < 4 || chiptype == CHIP_TYPE_ADSP2100)
			state->state_add(ADSP2100_IRQSTATE0 + irqnum, tempstring.format("IRQ%d", irqnum), adsp->irq_state[irqnum]).mask(1).callimport();

	state->state_add(ADSP2100_FLAGIN,  "FLAGIN",    adsp->flagin).mask(1);
	state->state_add(ADSP2100_FLAGOUT, "FLAGOUT",   adsp->flagout).mask(1);
	state->state_add(ADSP2100_FL0,     "FL0",       adsp->fl0).mask(1);
	state->state_add(ADSP2100_FL1,     "FL1",       adsp->fl1).mask(1);
	state->state_add(ADSP2100_FL2,     "FL2",       adsp->fl2).mask(1);

	return adsp;
}


static CPU_RESET( adsp21xx )
{
	adsp2100_state *adsp = get_safe_token(device);
	int irq;

	/* ensure that zero is zero */
	adsp->core.zero.u = adsp->alt.zero.u = 0;

	/* recompute the memory registers with their current values */
	wr_l0(adsp, adsp->l[0]);  wr_i0(adsp, adsp->i[0]);
	wr_l1(adsp, adsp->l[1]);  wr_i1(adsp, adsp->i[1]);
	wr_l2(adsp, adsp->l[2]);  wr_i2(adsp, adsp->i[2]);
	wr_l3(adsp, adsp->l[3]);  wr_i3(adsp, adsp->i[3]);
	wr_l4(adsp, adsp->l[4]);  wr_i4(adsp, adsp->i[4]);
	wr_l5(adsp, adsp->l[5]);  wr_i5(adsp, adsp->i[5]);
	wr_l6(adsp, adsp->l[6]);  wr_i6(adsp, adsp->i[6]);
	wr_l7(adsp, adsp->l[7]);  wr_i7(adsp, adsp->i[7]);

	/* reset PC and loops */
	switch (adsp->chip_type)
	{
		case CHIP_TYPE_ADSP2100:
			adsp->pc = 4;
			break;

		case CHIP_TYPE_ADSP2101:
		case CHIP_TYPE_ADSP2104:
		case CHIP_TYPE_ADSP2105:
		case CHIP_TYPE_ADSP2115:
		case CHIP_TYPE_ADSP2181:
			adsp->pc = 0;
			break;

		default:
			logerror( "ADSP2100 core: Unknown chip type!. Defaulting to adsp->\n" );
			adsp->pc = 4;
			adsp->chip_type = CHIP_TYPE_ADSP2100;
			break;
	}

	adsp->ppc = -1;
	adsp->loop = 0xffff;
	adsp->loop_condition = 0;

	/* reset status registers */
	adsp->astat_clear = ~(CFLAG | VFLAG | NFLAG | ZFLAG);
	adsp->mstat = 0;
	adsp->sstat = 0x55;
	adsp->idle = 0;
	update_mstat(adsp);

	/* reset stacks */
	adsp->pc_sp = 0;
	adsp->cntr_sp = 0;
	adsp->stat_sp = 0;
	adsp->loop_sp = 0;

	/* reset external I/O */
	adsp->flagout = 0;
	adsp->flagin = 0;
	adsp->fl0 = 0;
	adsp->fl1 = 0;
	adsp->fl2 = 0;

	/* reset interrupts */
	adsp->imask = 0;
	for (irq = 0; irq < 8; irq++)
		adsp->irq_state[irq] = adsp->irq_latch[irq] = CLEAR_LINE;
}


static int create_tables(void)
{
	int i;

	/* allocate the tables */
	if (!reverse_table)
		reverse_table = global_alloc_array(UINT16, 0x4000);
	if (!mask_table)
		mask_table = global_alloc_array(UINT16, 0x4000);
	if (!condition_table)
		condition_table = global_alloc_array(UINT8, 0x1000);

	/* handle errors */
	if (reverse_table == NULL || mask_table == NULL || condition_table == NULL)
		return 0;

	/* initialize the bit reversing table */
	for (i = 0; i < 0x4000; i++)
	{
		UINT16 data = 0;

		data |= (i >> 13) & 0x0001;
		data |= (i >> 11) & 0x0002;
		data |= (i >> 9)  & 0x0004;
		data |= (i >> 7)  & 0x0008;
		data |= (i >> 5)  & 0x0010;
		data |= (i >> 3)  & 0x0020;
		data |= (i >> 1)  & 0x0040;
		data |= (i << 1)  & 0x0080;
		data |= (i << 3)  & 0x0100;
		data |= (i << 5)  & 0x0200;
		data |= (i << 7)  & 0x0400;
		data |= (i << 9)  & 0x0800;
		data |= (i << 11) & 0x1000;
		data |= (i << 13) & 0x2000;

		reverse_table[i] = data;
	}

	/* initialize the mask table */
	for (i = 0; i < 0x4000; i++)
	{
		     if (i > 0x2000) mask_table[i] = 0x0000;
		else if (i > 0x1000) mask_table[i] = 0x2000;
		else if (i > 0x0800) mask_table[i] = 0x3000;
		else if (i > 0x0400) mask_table[i] = 0x3800;
		else if (i > 0x0200) mask_table[i] = 0x3c00;
		else if (i > 0x0100) mask_table[i] = 0x3e00;
		else if (i > 0x0080) mask_table[i] = 0x3f00;
		else if (i > 0x0040) mask_table[i] = 0x3f80;
		else if (i > 0x0020) mask_table[i] = 0x3fc0;
		else if (i > 0x0010) mask_table[i] = 0x3fe0;
		else if (i > 0x0008) mask_table[i] = 0x3ff0;
		else if (i > 0x0004) mask_table[i] = 0x3ff8;
		else if (i > 0x0002) mask_table[i] = 0x3ffc;
		else if (i > 0x0001) mask_table[i] = 0x3ffe;
		else                 mask_table[i] = 0x3fff;
	}

	/* initialize the condition table */
	for (i = 0; i < 0x100; i++)
	{
		int az = ((i & ZFLAG) != 0);
		int an = ((i & NFLAG) != 0);
		int av = ((i & VFLAG) != 0);
		int ac = ((i & CFLAG) != 0);
		int mv = ((i & MVFLAG) != 0);
		int as = ((i & SFLAG) != 0);

		condition_table[i | 0x000] = az;
		condition_table[i | 0x100] = !az;
		condition_table[i | 0x200] = !((an ^ av) | az);
		condition_table[i | 0x300] = (an ^ av) | az;
		condition_table[i | 0x400] = an ^ av;
		condition_table[i | 0x500] = !(an ^ av);
		condition_table[i | 0x600] = av;
		condition_table[i | 0x700] = !av;
		condition_table[i | 0x800] = ac;
		condition_table[i | 0x900] = !ac;
		condition_table[i | 0xa00] = as;
		condition_table[i | 0xb00] = !as;
		condition_table[i | 0xc00] = mv;
		condition_table[i | 0xd00] = !mv;
		condition_table[i | 0xf00] = 1;
	}
	return 1;
}


static CPU_EXIT( adsp21xx )
{
	if (reverse_table != NULL)
		global_free(reverse_table);
	reverse_table = NULL;

	if (mask_table != NULL)
		global_free(mask_table);
	mask_table = NULL;

	if (condition_table != NULL)
		global_free(condition_table);
	condition_table = NULL;

#if TRACK_HOTSPOTS
	{
		FILE *log = fopen("adsp.hot", "w");
		while (1)
		{
			int maxindex = 0, i;
			for (i = 1; i < 0x4000; i++)
				if (pcbucket[i] > pcbucket[maxindex])
					maxindex = i;
			if (pcbucket[maxindex] == 0)
				break;
			fprintf(log, "PC=%04X  (%10d hits)\n", maxindex, pcbucket[maxindex]);
			pcbucket[maxindex] = 0;
		}
		fclose(log);
	}
#endif

}



/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

/* execute instructions on this CPU until icount expires */
static CPU_EXECUTE( adsp21xx )
{
	int check_debugger = ((device->machine->debug_flags & DEBUG_FLAG_ENABLED) != 0);
	adsp2100_state *adsp = get_safe_token(device);

	check_irqs(adsp);

	do
	{
		UINT32 temp;
		UINT32 op;

		/* debugging */
		adsp->ppc = adsp->pc;	/* copy PC to previous PC */
		if (check_debugger)
			debugger_instruction_hook(device, adsp->pc);

#if TRACK_HOTSPOTS
		pcbucket[adsp->pc & 0x3fff]++;
#endif

		/* instruction fetch */
		op = ROPCODE(adsp);

		/* advance to the next instruction */
		if (adsp->pc != adsp->loop)
			adsp->pc++;

		/* handle looping */
		else
		{
			/* condition not met, keep looping */
			if (CONDITION(adsp, adsp->loop_condition))
				adsp->pc = pc_stack_top(adsp);

			/* condition met; pop the PC and loop stacks and fall through */
			else
			{
				loop_stack_pop(adsp);
				pc_stack_pop_val(adsp);
				adsp->pc++;
			}
		}

		/* parse the instruction */
		switch (op >> 16)
		{
			case 0x00:
				/* 00000000 00000000 00000000  NOP */
				break;
			case 0x01:
				/* 00000001 0xxxxxxx xxxxxxxx  dst = IO(x) */
				/* 00000001 1xxxxxxx xxxxxxxx  IO(x) = dst */
				/* ADSP-218x only */
				if (adsp->chip_type >= CHIP_TYPE_ADSP2181)
				{
					if ((op & 0x008000) == 0x000000)
						WRITE_REG(adsp, 0, op & 15, RWORD_IO(adsp, (op >> 4) & 0x7ff));
					else
						WWORD_IO(adsp, (op >> 4) & 0x7ff, READ_REG(adsp, 0, op & 15));
				}
				break;
			case 0x02:
				/* 00000010 0000xxxx xxxxxxxx  modify flag out */
				/* 00000010 10000000 00000000  idle */
				/* 00000010 10000000 0000xxxx  idle (n) */
				if (op & 0x008000)
				{
					adsp->idle = 1;
					adsp->icount = 0;
				}
				else
				{
					if (CONDITION(adsp, op & 15))
					{
						if (op & 0x020) adsp->flagout = 0;
						if (op & 0x010) adsp->flagout ^= 1;
						if (adsp->chip_type >= CHIP_TYPE_ADSP2101)
						{
							if (op & 0x080) adsp->fl0 = 0;
							if (op & 0x040) adsp->fl0 ^= 1;
							if (op & 0x200) adsp->fl1 = 0;
							if (op & 0x100) adsp->fl1 ^= 1;
							if (op & 0x800) adsp->fl2 = 0;
							if (op & 0x400) adsp->fl2 ^= 1;
						}
					}
				}
				break;
			case 0x03:
				/* 00000011 xxxxxxxx xxxxxxxx  call or jump on flag in */
				if (op & 0x000002)
				{
					if (adsp->flagin)
					{
						if (op & 0x000001)
							pc_stack_push(adsp);
						adsp->pc = ((op >> 4) & 0x0fff) | ((op << 10) & 0x3000);
					}
				}
				else
				{
					if (!adsp->flagin)
					{
						if (op & 0x000001)
							pc_stack_push(adsp);
						adsp->pc = ((op >> 4) & 0x0fff) | ((op << 10) & 0x3000);
					}
				}
				break;
			case 0x04:
				/* 00000100 00000000 000xxxxx  stack control */
				if (op & 0x000010) pc_stack_pop_val(adsp);
				if (op & 0x000008) loop_stack_pop(adsp);
				if (op & 0x000004) cntr_stack_pop(adsp);
				if (op & 0x000002)
				{
					if (op & 0x000001) stat_stack_pop(adsp);
					else stat_stack_push(adsp);
				}
				break;
			case 0x05:
				/* 00000101 00000000 00000000  saturate MR */
				if (GET_MV)
				{
					if (adsp->core.mr.mrx.mr2.u & 0x80)
						adsp->core.mr.mrx.mr2.u = 0xffff, adsp->core.mr.mrx.mr1.u = 0x8000, adsp->core.mr.mrx.mr0.u = 0x0000;
					else
						adsp->core.mr.mrx.mr2.u = 0x0000, adsp->core.mr.mrx.mr1.u = 0x7fff, adsp->core.mr.mrx.mr0.u = 0xffff;
				}
				break;
			case 0x06:
				/* 00000110 000xxxxx 00000000  DIVS */
				{
					int xop = (op >> 8) & 7;
					int yop = (op >> 11) & 3;

					xop = ALU_GETXREG_UNSIGNED(adsp, xop);
					yop = ALU_GETYREG_UNSIGNED(adsp, yop);

					temp = xop ^ yop;
					adsp->astat = (adsp->astat & ~QFLAG) | ((temp >> 10) & QFLAG);
					adsp->core.af.u = (yop << 1) | (adsp->core.ay0.u >> 15);
					adsp->core.ay0.u = (adsp->core.ay0.u << 1) | (temp >> 15);
				}
				break;
			case 0x07:
				/* 00000111 00010xxx 00000000  DIVQ */
				{
					int xop = (op >> 8) & 7;
					int res;

					xop = ALU_GETXREG_UNSIGNED(adsp, xop);

					if (GET_Q)
						res = adsp->core.af.u + xop;
					else
						res = adsp->core.af.u - xop;

					temp = res ^ xop;
					adsp->astat = (adsp->astat & ~QFLAG) | ((temp >> 10) & QFLAG);
					adsp->core.af.u = (res << 1) | (adsp->core.ay0.u >> 15);
					adsp->core.ay0.u = (adsp->core.ay0.u << 1) | ((~temp >> 15) & 0x0001);
				}
				break;
			case 0x08:
				/* 00001000 00000000 0000xxxx  reserved */
				break;
			case 0x09:
				/* 00001001 00000000 000xxxxx  modify address register */
				temp = (op >> 2) & 4;
				modify_address(adsp, temp + ((op >> 2) & 3), temp + (op & 3));
				break;
			case 0x0a:
				/* 00001010 00000000 000xxxxx  conditional return */
				if (CONDITION(adsp, op & 15))
				{
					pc_stack_pop(adsp);

					/* RTI case */
					if (op & 0x000010)
						stat_stack_pop(adsp);
				}
				break;
			case 0x0b:
				/* 00001011 00000000 xxxxxxxx  conditional jump (indirect address) */
				if (CONDITION(adsp, op & 15))
				{
					if (op & 0x000010)
						pc_stack_push(adsp);
					adsp->pc = adsp->i[4 + ((op >> 6) & 3)] & 0x3fff;
				}
				break;
			case 0x0c:
				/* 00001100 xxxxxxxx xxxxxxxx  mode control */
				if (adsp->chip_type >= CHIP_TYPE_ADSP2101)
				{
					if (op & 0x000008) adsp->mstat = (adsp->mstat & ~MSTAT_GOMODE) | ((op << 5) & MSTAT_GOMODE);
					if (op & 0x002000) adsp->mstat = (adsp->mstat & ~MSTAT_INTEGER) | ((op >> 8) & MSTAT_INTEGER);
					if (op & 0x008000) adsp->mstat = (adsp->mstat & ~MSTAT_TIMER) | ((op >> 9) & MSTAT_TIMER);
				}
				if (op & 0x000020) adsp->mstat = (adsp->mstat & ~MSTAT_BANK) | ((op >> 4) & MSTAT_BANK);
				if (op & 0x000080) adsp->mstat = (adsp->mstat & ~MSTAT_REVERSE) | ((op >> 5) & MSTAT_REVERSE);
				if (op & 0x000200) adsp->mstat = (adsp->mstat & ~MSTAT_STICKYV) | ((op >> 6) & MSTAT_STICKYV);
				if (op & 0x000800) adsp->mstat = (adsp->mstat & ~MSTAT_SATURATE) | ((op >> 7) & MSTAT_SATURATE);
				update_mstat(adsp);
				break;
			case 0x0d:
				/* 00001101 0000xxxx xxxxxxxx  internal data move */
				WRITE_REG(adsp, (op >> 10) & 3, (op >> 4) & 15, READ_REG(adsp, (op >> 8) & 3, op & 15));
				break;
			case 0x0e:
				/* 00001110 0xxxxxxx xxxxxxxx  conditional shift */
				if (CONDITION(adsp, op & 15)) shift_op(adsp, op);
				break;
			case 0x0f:
				/* 00001111 0xxxxxxx xxxxxxxx  shift immediate */
				shift_op_imm(adsp, op);
				break;
			case 0x10:
				/* 00010000 0xxxxxxx xxxxxxxx  shift with internal data register move */
				shift_op(adsp, op);
				temp = READ_REG(adsp, 0, op & 15);
				WRITE_REG(adsp, 0, (op >> 4) & 15, temp);
				break;
			case 0x11:
				/* 00010001 xxxxxxxx xxxxxxxx  shift with pgm memory read/write */
				if (op & 0x8000)
				{
					pgm_write_dag2(adsp, op, READ_REG(adsp, 0, (op >> 4) & 15));
					shift_op(adsp, op);
				}
				else
				{
					shift_op(adsp, op);
					WRITE_REG(adsp, 0, (op >> 4) & 15, pgm_read_dag2(adsp, op));
				}
				break;
			case 0x12:
				/* 00010010 xxxxxxxx xxxxxxxx  shift with data memory read/write DAG1 */
				if (op & 0x8000)
				{
					data_write_dag1(adsp, op, READ_REG(adsp, 0, (op >> 4) & 15));
					shift_op(adsp, op);
				}
				else
				{
					shift_op(adsp, op);
					WRITE_REG(adsp, 0, (op >> 4) & 15, data_read_dag1(adsp, op));
				}
				break;
			case 0x13:
				/* 00010011 xxxxxxxx xxxxxxxx  shift with data memory read/write DAG2 */
				if (op & 0x8000)
				{
					data_write_dag2(adsp, op, READ_REG(adsp, 0, (op >> 4) & 15));
					shift_op(adsp, op);
				}
				else
				{
					shift_op(adsp, op);
					WRITE_REG(adsp, 0, (op >> 4) & 15, data_read_dag2(adsp, op));
				}
				break;
			case 0x14: case 0x15: case 0x16: case 0x17:
				/* 000101xx xxxxxxxx xxxxxxxx  do until */
				loop_stack_push(adsp, op & 0x3ffff);
				pc_stack_push(adsp);
				break;
			case 0x18: case 0x19: case 0x1a: case 0x1b:
				/* 000110xx xxxxxxxx xxxxxxxx  conditional jump (immediate addr) */
				if (CONDITION(adsp, op & 15))
				{
					adsp->pc = (op >> 4) & 0x3fff;
					/* check for a busy loop */
					if (adsp->pc == adsp->ppc)
						adsp->icount = 0;
				}
				break;
			case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				/* 000111xx xxxxxxxx xxxxxxxx  conditional call (immediate addr) */
				if (CONDITION(adsp, op & 15))
				{
					pc_stack_push(adsp);
					adsp->pc = (op >> 4) & 0x3fff;
				}
				break;
			case 0x20: case 0x21:
				/* 0010000x xxxxxxxx xxxxxxxx  conditional MAC to MR */
				if (CONDITION(adsp, op & 15))
				{
					if (adsp->chip_type >= CHIP_TYPE_ADSP2181 && (op & 0x0018f0) == 0x000010)
						mac_op_mr_xop(adsp, op);
					else
						mac_op_mr(adsp, op);
				}
				break;
			case 0x22: case 0x23:
				/* 0010001x xxxxxxxx xxxxxxxx  conditional ALU to AR */
				if (CONDITION(adsp, op & 15))
				{
					if (adsp->chip_type >= CHIP_TYPE_ADSP2181 && (op & 0x000010) == 0x000010)
						alu_op_ar_const(adsp, op);
					else
						alu_op_ar(adsp, op);
				}
				break;
			case 0x24: case 0x25:
				/* 0010010x xxxxxxxx xxxxxxxx  conditional MAC to MF */
				if (CONDITION(adsp, op & 15))
				{
					if (adsp->chip_type >= CHIP_TYPE_ADSP2181 && (op & 0x0018f0) == 0x000010)
						mac_op_mf_xop(adsp, op);
					else
						mac_op_mf(adsp, op);
				}
				break;
			case 0x26: case 0x27:
				/* 0010011x xxxxxxxx xxxxxxxx  conditional ALU to AF */
				if (CONDITION(adsp, op & 15))
				{
					if (adsp->chip_type >= CHIP_TYPE_ADSP2181 && (op & 0x000010) == 0x000010)
						alu_op_af_const(adsp, op);
					else
						alu_op_af(adsp, op);
				}
				break;
			case 0x28: case 0x29:
				/* 0010100x xxxxxxxx xxxxxxxx  MAC to MR with internal data register move */
				temp = READ_REG(adsp, 0, op & 15);
				mac_op_mr(adsp, op);
				WRITE_REG(adsp, 0, (op >> 4) & 15, temp);
				break;
			case 0x2a: case 0x2b:
				/* 0010101x xxxxxxxx xxxxxxxx  ALU to AR with internal data register move */
				if (adsp->chip_type >= CHIP_TYPE_ADSP2181 && (op & 0x0000ff) == 0x0000aa)
					alu_op_none(adsp, op);
				else
				{
					temp = READ_REG(adsp, 0, op & 15);
					alu_op_ar(adsp, op);
					WRITE_REG(adsp, 0, (op >> 4) & 15, temp);
				}
				break;
			case 0x2c: case 0x2d:
				/* 0010110x xxxxxxxx xxxxxxxx  MAC to MF with internal data register move */
				temp = READ_REG(adsp, 0, op & 15);
				mac_op_mf(adsp, op);
				WRITE_REG(adsp, 0, (op >> 4) & 15, temp);
				break;
			case 0x2e: case 0x2f:
				/* 0010111x xxxxxxxx xxxxxxxx  ALU to AF with internal data register move */
				temp = READ_REG(adsp, 0, op & 15);
				alu_op_af(adsp, op);
				WRITE_REG(adsp, 0, (op >> 4) & 15, temp);
				break;
			case 0x30: case 0x31: case 0x32: case 0x33:
				/* 001100xx xxxxxxxx xxxxxxxx  load non-data register immediate (group 0) */
				WRITE_REG(adsp, 0, op & 15, (INT32)(op << 14) >> 18);
				break;
			case 0x34: case 0x35: case 0x36: case 0x37:
				/* 001101xx xxxxxxxx xxxxxxxx  load non-data register immediate (group 1) */
				WRITE_REG(adsp, 1, op & 15, (INT32)(op << 14) >> 18);
				break;
			case 0x38: case 0x39: case 0x3a: case 0x3b:
				/* 001110xx xxxxxxxx xxxxxxxx  load non-data register immediate (group 2) */
				WRITE_REG(adsp, 2, op & 15, (INT32)(op << 14) >> 18);
				break;
			case 0x3c: case 0x3d: case 0x3e: case 0x3f:
				/* 001111xx xxxxxxxx xxxxxxxx  load non-data register immediate (group 3) */
				WRITE_REG(adsp, 3, op & 15, (INT32)(op << 14) >> 18);
				break;
			case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
			case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
				/* 0100xxxx xxxxxxxx xxxxxxxx  load data register immediate */
				WRITE_REG(adsp, 0, op & 15, (op >> 4) & 0xffff);
				break;
			case 0x50: case 0x51:
				/* 0101000x xxxxxxxx xxxxxxxx  MAC to MR with pgm memory read */
				mac_op_mr(adsp, op);
				WRITE_REG(adsp, 0, (op >> 4) & 15, pgm_read_dag2(adsp, op));
				break;
			case 0x52: case 0x53:
				/* 0101001x xxxxxxxx xxxxxxxx  ALU to AR with pgm memory read */
				alu_op_ar(adsp, op);
				WRITE_REG(adsp, 0, (op >> 4) & 15, pgm_read_dag2(adsp, op));
				break;
			case 0x54: case 0x55:
				/* 0101010x xxxxxxxx xxxxxxxx  MAC to MF with pgm memory read */
				mac_op_mf(adsp, op);
				WRITE_REG(adsp, 0, (op >> 4) & 15, pgm_read_dag2(adsp, op));
				break;
			case 0x56: case 0x57:
				/* 0101011x xxxxxxxx xxxxxxxx  ALU to AF with pgm memory read */
				alu_op_af(adsp, op);
				WRITE_REG(adsp, 0, (op >> 4) & 15, pgm_read_dag2(adsp, op));
				break;
			case 0x58: case 0x59:
				/* 0101100x xxxxxxxx xxxxxxxx  MAC to MR with pgm memory write */
				pgm_write_dag2(adsp, op, READ_REG(adsp, 0, (op >> 4) & 15));
				mac_op_mr(adsp, op);
				break;
			case 0x5a: case 0x5b:
				/* 0101101x xxxxxxxx xxxxxxxx  ALU to AR with pgm memory write */
				pgm_write_dag2(adsp, op, READ_REG(adsp, 0, (op >> 4) & 15));
				alu_op_ar(adsp, op);
				break;
			case 0x5c: case 0x5d:
				/* 0101110x xxxxxxxx xxxxxxxx  ALU to MR with pgm memory write */
				pgm_write_dag2(adsp, op, READ_REG(adsp, 0, (op >> 4) & 15));
				mac_op_mf(adsp, op);
				break;
			case 0x5e: case 0x5f:
				/* 0101111x xxxxxxxx xxxxxxxx  ALU to MF with pgm memory write */
				pgm_write_dag2(adsp, op, READ_REG(adsp, 0, (op >> 4) & 15));
				alu_op_af(adsp, op);
				break;
			case 0x60: case 0x61:
				/* 0110000x xxxxxxxx xxxxxxxx  MAC to MR with data memory read DAG1 */
				mac_op_mr(adsp, op);
				WRITE_REG(adsp, 0, (op >> 4) & 15, data_read_dag1(adsp, op));
				break;
			case 0x62: case 0x63:
				/* 0110001x xxxxxxxx xxxxxxxx  ALU to AR with data memory read DAG1 */
				alu_op_ar(adsp, op);
				WRITE_REG(adsp, 0, (op >> 4) & 15, data_read_dag1(adsp, op));
				break;
			case 0x64: case 0x65:
				/* 0110010x xxxxxxxx xxxxxxxx  MAC to MF with data memory read DAG1 */
				mac_op_mf(adsp, op);
				WRITE_REG(adsp, 0, (op >> 4) & 15, data_read_dag1(adsp, op));
				break;
			case 0x66: case 0x67:
				/* 0110011x xxxxxxxx xxxxxxxx  ALU to AF with data memory read DAG1 */
				alu_op_af(adsp, op);
				WRITE_REG(adsp, 0, (op >> 4) & 15, data_read_dag1(adsp, op));
				break;
			case 0x68: case 0x69:
				/* 0110100x xxxxxxxx xxxxxxxx  MAC to MR with data memory write DAG1 */
				data_write_dag1(adsp, op, READ_REG(adsp, 0, (op >> 4) & 15));
				mac_op_mr(adsp, op);
				break;
			case 0x6a: case 0x6b:
				/* 0110101x xxxxxxxx xxxxxxxx  ALU to AR with data memory write DAG1 */
				data_write_dag1(adsp, op, READ_REG(adsp, 0, (op >> 4) & 15));
				alu_op_ar(adsp, op);
				break;
			case 0x6c: case 0x6d:
				/* 0111110x xxxxxxxx xxxxxxxx  MAC to MF with data memory write DAG1 */
				data_write_dag1(adsp, op, READ_REG(adsp, 0, (op >> 4) & 15));
				mac_op_mf(adsp, op);
				break;
			case 0x6e: case 0x6f:
				/* 0111111x xxxxxxxx xxxxxxxx  ALU to AF with data memory write DAG1 */
				data_write_dag1(adsp, op, READ_REG(adsp, 0, (op >> 4) & 15));
				alu_op_af(adsp, op);
				break;
			case 0x70: case 0x71:
				/* 0111000x xxxxxxxx xxxxxxxx  MAC to MR with data memory read DAG2 */
				mac_op_mr(adsp, op);
				WRITE_REG(adsp, 0, (op >> 4) & 15, data_read_dag2(adsp, op));
				break;
			case 0x72: case 0x73:
				/* 0111001x xxxxxxxx xxxxxxxx  ALU to AR with data memory read DAG2 */
				alu_op_ar(adsp, op);
				WRITE_REG(adsp, 0, (op >> 4) & 15, data_read_dag2(adsp, op));
				break;
			case 0x74: case 0x75:
				/* 0111010x xxxxxxxx xxxxxxxx  MAC to MF with data memory read DAG2 */
				mac_op_mf(adsp, op);
				WRITE_REG(adsp, 0, (op >> 4) & 15, data_read_dag2(adsp, op));
				break;
			case 0x76: case 0x77:
				/* 0111011x xxxxxxxx xxxxxxxx  ALU to AF with data memory read DAG2 */
				alu_op_af(adsp, op);
				WRITE_REG(adsp, 0, (op >> 4) & 15, data_read_dag2(adsp, op));
				break;
			case 0x78: case 0x79:
				/* 0111100x xxxxxxxx xxxxxxxx  MAC to MR with data memory write DAG2 */
				data_write_dag2(adsp, op, READ_REG(adsp, 0, (op >> 4) & 15));
				mac_op_mr(adsp, op);
				break;
			case 0x7a: case 0x7b:
				/* 0111101x xxxxxxxx xxxxxxxx  ALU to AR with data memory write DAG2 */
				data_write_dag2(adsp, op, READ_REG(adsp, 0, (op >> 4) & 15));
				alu_op_ar(adsp, op);
				break;
			case 0x7c: case 0x7d:
				/* 0111110x xxxxxxxx xxxxxxxx  MAC to MF with data memory write DAG2 */
				data_write_dag2(adsp, op, READ_REG(adsp, 0, (op >> 4) & 15));
				mac_op_mf(adsp, op);
				break;
			case 0x7e: case 0x7f:
				/* 0111111x xxxxxxxx xxxxxxxx  ALU to AF with data memory write DAG2 */
				data_write_dag2(adsp, op, READ_REG(adsp, 0, (op >> 4) & 15));
				alu_op_af(adsp, op);
				break;
			case 0x80: case 0x81: case 0x82: case 0x83:
				/* 100000xx xxxxxxxx xxxxxxxx  read data memory (immediate addr) to reg group 0 */
				WRITE_REG(adsp, 0, op & 15, RWORD_DATA(adsp, (op >> 4) & 0x3fff));
				break;
			case 0x84: case 0x85: case 0x86: case 0x87:
				/* 100001xx xxxxxxxx xxxxxxxx  read data memory (immediate addr) to reg group 1 */
				WRITE_REG(adsp, 1, op & 15, RWORD_DATA(adsp, (op >> 4) & 0x3fff));
				break;
			case 0x88: case 0x89: case 0x8a: case 0x8b:
				/* 100010xx xxxxxxxx xxxxxxxx  read data memory (immediate addr) to reg group 2 */
				WRITE_REG(adsp, 2, op & 15, RWORD_DATA(adsp, (op >> 4) & 0x3fff));
				break;
			case 0x8c: case 0x8d: case 0x8e: case 0x8f:
				/* 100011xx xxxxxxxx xxxxxxxx  read data memory (immediate addr) to reg group 3 */
				WRITE_REG(adsp, 3, op & 15, RWORD_DATA(adsp, (op >> 4) & 0x3fff));
				break;
			case 0x90: case 0x91: case 0x92: case 0x93:
				/* 1001xxxx xxxxxxxx xxxxxxxx  write data memory (immediate addr) from reg group 0 */
				WWORD_DATA(adsp, (op >> 4) & 0x3fff, READ_REG(adsp, 0, op & 15));
				break;
			case 0x94: case 0x95: case 0x96: case 0x97:
				/* 1001xxxx xxxxxxxx xxxxxxxx  write data memory (immediate addr) from reg group 1 */
				WWORD_DATA(adsp, (op >> 4) & 0x3fff, READ_REG(adsp, 1, op & 15));
				break;
			case 0x98: case 0x99: case 0x9a: case 0x9b:
				/* 1001xxxx xxxxxxxx xxxxxxxx  write data memory (immediate addr) from reg group 2 */
				WWORD_DATA(adsp, (op >> 4) & 0x3fff, READ_REG(adsp, 2, op & 15));
				break;
			case 0x9c: case 0x9d: case 0x9e: case 0x9f:
				/* 1001xxxx xxxxxxxx xxxxxxxx  write data memory (immediate addr) from reg group 3 */
				WWORD_DATA(adsp, (op >> 4) & 0x3fff, READ_REG(adsp, 3, op & 15));
				break;
			case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
			case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
				/* 1010xxxx xxxxxxxx xxxxxxxx  data memory write (immediate) DAG1 */
				data_write_dag1(adsp, op, (op >> 4) & 0xffff);
				break;
			case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
				/* 1011xxxx xxxxxxxx xxxxxxxx  data memory write (immediate) DAG2 */
				data_write_dag2(adsp, op, (op >> 4) & 0xffff);
				break;
			case 0xc0: case 0xc1:
				/* 1100000x xxxxxxxx xxxxxxxx  MAC to MR with data read to AX0 & pgm read to AY0 */
				mac_op_mr(adsp, op);
				adsp->core.ax0.u = data_read_dag1(adsp, op);
				adsp->core.ay0.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xc2: case 0xc3:
				/* 1100001x xxxxxxxx xxxxxxxx  ALU to AR with data read to AX0 & pgm read to AY0 */
				alu_op_ar(adsp, op);
				adsp->core.ax0.u = data_read_dag1(adsp, op);
				adsp->core.ay0.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xc4: case 0xc5:
				/* 1100010x xxxxxxxx xxxxxxxx  MAC to MR with data read to AX1 & pgm read to AY0 */
				mac_op_mr(adsp, op);
				adsp->core.ax1.u = data_read_dag1(adsp, op);
				adsp->core.ay0.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xc6: case 0xc7:
				/* 1100011x xxxxxxxx xxxxxxxx  ALU to AR with data read to AX1 & pgm read to AY0 */
				alu_op_ar(adsp, op);
				adsp->core.ax1.u = data_read_dag1(adsp, op);
				adsp->core.ay0.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xc8: case 0xc9:
				/* 1100100x xxxxxxxx xxxxxxxx  MAC to MR with data read to MX0 & pgm read to AY0 */
				mac_op_mr(adsp, op);
				adsp->core.mx0.u = data_read_dag1(adsp, op);
				adsp->core.ay0.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xca: case 0xcb:
				/* 1100101x xxxxxxxx xxxxxxxx  ALU to AR with data read to MX0 & pgm read to AY0 */
				alu_op_ar(adsp, op);
				adsp->core.mx0.u = data_read_dag1(adsp, op);
				adsp->core.ay0.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xcc: case 0xcd:
				/* 1100110x xxxxxxxx xxxxxxxx  MAC to MR with data read to MX1 & pgm read to AY0 */
				mac_op_mr(adsp, op);
				adsp->core.mx1.u = data_read_dag1(adsp, op);
				adsp->core.ay0.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xce: case 0xcf:
				/* 1100111x xxxxxxxx xxxxxxxx  ALU to AR with data read to MX1 & pgm read to AY0 */
				alu_op_ar(adsp, op);
				adsp->core.mx1.u = data_read_dag1(adsp, op);
				adsp->core.ay0.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xd0: case 0xd1:
				/* 1101000x xxxxxxxx xxxxxxxx  MAC to MR with data read to AX0 & pgm read to AY1 */
				mac_op_mr(adsp, op);
				adsp->core.ax0.u = data_read_dag1(adsp, op);
				adsp->core.ay1.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xd2: case 0xd3:
				/* 1101001x xxxxxxxx xxxxxxxx  ALU to AR with data read to AX0 & pgm read to AY1 */
				alu_op_ar(adsp, op);
				adsp->core.ax0.u = data_read_dag1(adsp, op);
				adsp->core.ay1.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xd4: case 0xd5:
				/* 1101010x xxxxxxxx xxxxxxxx  MAC to MR with data read to AX1 & pgm read to AY1 */
				mac_op_mr(adsp, op);
				adsp->core.ax1.u = data_read_dag1(adsp, op);
				adsp->core.ay1.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xd6: case 0xd7:
				/* 1101011x xxxxxxxx xxxxxxxx  ALU to AR with data read to AX1 & pgm read to AY1 */
				alu_op_ar(adsp, op);
				adsp->core.ax1.u = data_read_dag1(adsp, op);
				adsp->core.ay1.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xd8: case 0xd9:
				/* 1101100x xxxxxxxx xxxxxxxx  MAC to MR with data read to MX0 & pgm read to AY1 */
				mac_op_mr(adsp, op);
				adsp->core.mx0.u = data_read_dag1(adsp, op);
				adsp->core.ay1.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xda: case 0xdb:
				/* 1101101x xxxxxxxx xxxxxxxx  ALU to AR with data read to MX0 & pgm read to AY1 */
				alu_op_ar(adsp, op);
				adsp->core.mx0.u = data_read_dag1(adsp, op);
				adsp->core.ay1.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xdc: case 0xdd:
				/* 1101110x xxxxxxxx xxxxxxxx  MAC to MR with data read to MX1 & pgm read to AY1 */
				mac_op_mr(adsp, op);
				adsp->core.mx1.u = data_read_dag1(adsp, op);
				adsp->core.ay1.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xde: case 0xdf:
				/* 1101111x xxxxxxxx xxxxxxxx  ALU to AR with data read to MX1 & pgm read to AY1 */
				alu_op_ar(adsp, op);
				adsp->core.mx1.u = data_read_dag1(adsp, op);
				adsp->core.ay1.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xe0: case 0xe1:
				/* 1110000x xxxxxxxx xxxxxxxx  MAC to MR with data read to AX0 & pgm read to MY0 */
				mac_op_mr(adsp, op);
				adsp->core.ax0.u = data_read_dag1(adsp, op);
				adsp->core.my0.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xe2: case 0xe3:
				/* 1110001x xxxxxxxx xxxxxxxx  ALU to AR with data read to AX0 & pgm read to MY0 */
				alu_op_ar(adsp, op);
				adsp->core.ax0.u = data_read_dag1(adsp, op);
				adsp->core.my0.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xe4: case 0xe5:
				/* 1110010x xxxxxxxx xxxxxxxx  MAC to MR with data read to AX1 & pgm read to MY0 */
				mac_op_mr(adsp, op);
				adsp->core.ax1.u = data_read_dag1(adsp, op);
				adsp->core.my0.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xe6: case 0xe7:
				/* 1110011x xxxxxxxx xxxxxxxx  ALU to AR with data read to AX1 & pgm read to MY0 */
				alu_op_ar(adsp, op);
				adsp->core.ax1.u = data_read_dag1(adsp, op);
				adsp->core.my0.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xe8: case 0xe9:
				/* 1110100x xxxxxxxx xxxxxxxx  MAC to MR with data read to MX0 & pgm read to MY0 */
				mac_op_mr(adsp, op);
				adsp->core.mx0.u = data_read_dag1(adsp, op);
				adsp->core.my0.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xea: case 0xeb:
				/* 1110101x xxxxxxxx xxxxxxxx  ALU to AR with data read to MX0 & pgm read to MY0 */
				alu_op_ar(adsp, op);
				adsp->core.mx0.u = data_read_dag1(adsp, op);
				adsp->core.my0.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xec: case 0xed:
				/* 1110110x xxxxxxxx xxxxxxxx  MAC to MR with data read to MX1 & pgm read to MY0 */
				mac_op_mr(adsp, op);
				adsp->core.mx1.u = data_read_dag1(adsp, op);
				adsp->core.my0.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xee: case 0xef:
				/* 1110111x xxxxxxxx xxxxxxxx  ALU to AR with data read to MX1 & pgm read to MY0 */
				alu_op_ar(adsp, op);
				adsp->core.mx1.u = data_read_dag1(adsp, op);
				adsp->core.my0.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xf0: case 0xf1:
				/* 1111000x xxxxxxxx xxxxxxxx  MAC to MR with data read to AX0 & pgm read to MY1 */
				mac_op_mr(adsp, op);
				adsp->core.ax0.u = data_read_dag1(adsp, op);
				adsp->core.my1.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xf2: case 0xf3:
				/* 1111001x xxxxxxxx xxxxxxxx  ALU to AR with data read to AX0 & pgm read to MY1 */
				alu_op_ar(adsp, op);
				adsp->core.ax0.u = data_read_dag1(adsp, op);
				adsp->core.my1.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xf4: case 0xf5:
				/* 1111010x xxxxxxxx xxxxxxxx  MAC to MR with data read to AX1 & pgm read to MY1 */
				mac_op_mr(adsp, op);
				adsp->core.ax1.u = data_read_dag1(adsp, op);
				adsp->core.my1.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xf6: case 0xf7:
				/* 1111011x xxxxxxxx xxxxxxxx  ALU to AR with data read to AX1 & pgm read to MY1 */
				alu_op_ar(adsp, op);
				adsp->core.ax1.u = data_read_dag1(adsp, op);
				adsp->core.my1.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xf8: case 0xf9:
				/* 1111100x xxxxxxxx xxxxxxxx  MAC to MR with data read to MX0 & pgm read to MY1 */
				mac_op_mr(adsp, op);
				adsp->core.mx0.u = data_read_dag1(adsp, op);
				adsp->core.my1.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xfa: case 0xfb:
				/* 1111101x xxxxxxxx xxxxxxxx  ALU to AR with data read to MX0 & pgm read to MY1 */
				alu_op_ar(adsp, op);
				adsp->core.mx0.u = data_read_dag1(adsp, op);
				adsp->core.my1.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xfc: case 0xfd:
				/* 1111110x xxxxxxxx xxxxxxxx  MAC to MR with data read to MX1 & pgm read to MY1 */
				mac_op_mr(adsp, op);
				adsp->core.mx1.u = data_read_dag1(adsp, op);
				adsp->core.my1.u = pgm_read_dag2(adsp, op >> 4);
				break;
			case 0xfe: case 0xff:
				/* 1111111x xxxxxxxx xxxxxxxx  ALU to AR with data read to MX1 & pgm read to MY1 */
				alu_op_ar(adsp, op);
				adsp->core.mx1.u = data_read_dag1(adsp, op);
				adsp->core.my1.u = pgm_read_dag2(adsp, op >> 4);
				break;
		}

		adsp->icount--;
	} while (adsp->icount > 0);
}



/***************************************************************************
    DEBUGGER DEFINITIONS
***************************************************************************/

extern CPU_DISASSEMBLE( adsp21xx );



/***************************************************************************
    STATE HANDLING CALLBACKS
***************************************************************************/

static CPU_IMPORT_STATE( adsp21xx )
{
	adsp2100_state *adsp = get_safe_token(device);
	switch (entry.index())
	{
		case ADSP2100_MSTAT:
			update_mstat(adsp);
			break;

		case ADSP2100_IMASK:
		case ADSP2100_ICNTL:
		case ADSP2100_IRQSTATE0:
		case ADSP2100_IRQSTATE1:
		case ADSP2100_IRQSTATE2:
		case ADSP2100_IRQSTATE3:
			check_irqs(adsp);
			break;

		case ADSP2100_I0:
		case ADSP2100_I1:
		case ADSP2100_I2:
		case ADSP2100_I3:
		case ADSP2100_I4:
		case ADSP2100_I5:
		case ADSP2100_I6:
		case ADSP2100_I7:
			update_i(adsp, entry.index() - ADSP2100_I0);
			break;

		case ADSP2100_L0:
		case ADSP2100_L1:
		case ADSP2100_L2:
		case ADSP2100_L3:
		case ADSP2100_L4:
		case ADSP2100_L5:
		case ADSP2100_L6:
		case ADSP2100_L7:
			update_l(adsp, entry.index() - ADSP2100_L0);
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(adsp21xx) called for unexpected value\n");
			break;
	}
}


static CPU_EXPORT_STRING( adsp21xx )
{
	adsp2100_state *adsp = get_safe_token(device);

	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%c%c%c%c%c%c%c",
				adsp->astat & 0x80 ? 'X':'.',
				adsp->astat & 0x40 ? 'M':'.',
				adsp->astat & 0x20 ? 'Q':'.',
				adsp->astat & 0x10 ? 'S':'.',
				adsp->astat & 0x08 ? 'C':'.',
				adsp->astat & 0x04 ? 'V':'.',
				adsp->astat & 0x02 ? 'N':'.',
				adsp->astat & 0x01 ? 'Z':'.');
			break;
	}
}




/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( adsp21xx )
{
	adsp2100_state *adsp = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:
		case CPUINFO_INT_INPUT_STATE + 1:
		case CPUINFO_INT_INPUT_STATE + 2:
		case CPUINFO_INT_INPUT_STATE + 3:
		case CPUINFO_INT_INPUT_STATE + 4:
		case CPUINFO_INT_INPUT_STATE + 5:
		case CPUINFO_INT_INPUT_STATE + 6:
		case CPUINFO_INT_INPUT_STATE + 7:
		case CPUINFO_INT_INPUT_STATE + 8:
		case CPUINFO_INT_INPUT_STATE + 9:
			set_irq_line(adsp, state - CPUINFO_INT_INPUT_STATE, info->i);
			break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static CPU_GET_INFO( adsp21xx )
{
	adsp2100_state *adsp = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(adsp2100_state);		break;
		case CPUINFO_INT_INPUT_LINES:					/* set per CPU */						break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 1;							break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:			info->i = 32;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: 		info->i = 14;							break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: 		info->i = -2;							break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:			info->i = 16;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:			info->i = 14;							break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:			info->i = -1;							break;

		case CPUINFO_INT_INPUT_STATE + 0:
		case CPUINFO_INT_INPUT_STATE + 1:
		case CPUINFO_INT_INPUT_STATE + 2:
		case CPUINFO_INT_INPUT_STATE + 3:
		case CPUINFO_INT_INPUT_STATE + 4:
		case CPUINFO_INT_INPUT_STATE + 5:
		case CPUINFO_INT_INPUT_STATE + 6:
		case CPUINFO_INT_INPUT_STATE + 7:
		case CPUINFO_INT_INPUT_STATE + 8:
		case CPUINFO_INT_INPUT_STATE + 9:
			info->i = adsp->irq_state[state - CPUINFO_INT_INPUT_STATE];
			break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_SET_INFO:		info->setinfo = CPU_SET_INFO_NAME(adsp21xx);			break;
		case CPUINFO_FCT_INIT:			/* set per CPU */										break;
		case CPUINFO_FCT_RESET:			info->reset = CPU_RESET_NAME(adsp21xx);					break;
		case CPUINFO_FCT_EXIT:			info->exit = CPU_EXIT_NAME(adsp21xx);					break;
		case CPUINFO_FCT_EXECUTE:		info->execute = CPU_EXECUTE_NAME(adsp21xx);				break;
		case CPUINFO_FCT_DISASSEMBLE:	info->disassemble = CPU_DISASSEMBLE_NAME(adsp21xx);		break;
		case CPUINFO_FCT_IMPORT_STATE:	info->import_state = CPU_IMPORT_STATE_NAME(adsp21xx);	break;
		case CPUINFO_FCT_EXPORT_STRING: info->export_string = CPU_EXPORT_STRING_NAME(adsp21xx);	break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &adsp->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							/* set per CPU */						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "ADSP21xx");			break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "2.0");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Aaron Giles"); break;
	}
}

static void adsp21xx_load_boot_data(UINT8 *srcdata, UINT32 *dstdata)
{
	/* see how many words we need to copy */
	int pagelen = (srcdata[(3)] + 1) * 8;
	int i;
	for (i = 0; i < pagelen; i++)
	{
		UINT32 opcode = (srcdata[(i*4+0)] << 16) | (srcdata[(i*4+1)] << 8) | srcdata[(i*4+2)];
		dstdata[i] = opcode;
	}
}


/**************************************************************************
 * ADSP2100 section
 **************************************************************************/

static CPU_INIT( adsp2100 )
{
	adsp2100_state *adsp = adsp21xx_init(device, irqcallback, CHIP_TYPE_ADSP2100);
	adsp->mstat_mask = 0x0f;
	adsp->imask_mask = 0x0f;
}

CPU_GET_INFO( adsp2100 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_LINES:					info->i = 4;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(adsp2100);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "ADSP2100");			break;

		default:
			CPU_GET_INFO_CALL(adsp21xx);
			break;
	}
}

/**************************************************************************
 * ADSP2101 section
 **************************************************************************/

static CPU_INIT( adsp2101 )
{
	adsp2100_state *adsp = adsp21xx_init(device, irqcallback, CHIP_TYPE_ADSP2101);
	adsp->mstat_mask = 0x7f;
	adsp->imask_mask = 0x3f;
}

CPU_GET_INFO( adsp2101 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_LINES:					info->i = 5;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(adsp2101);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "ADSP2101");			break;

		default:
			CPU_GET_INFO_CALL(adsp21xx);
			break;
	}
}

/**************************************************************************
 * ADSP2104 section
 **************************************************************************/

static CPU_INIT( adsp2104 )
{
	adsp2100_state *adsp = adsp21xx_init(device, irqcallback, CHIP_TYPE_ADSP2104);
	adsp->mstat_mask = 0x7f;
	adsp->imask_mask = 0x3f;
}

void adsp2104_load_boot_data(UINT8 *srcdata, UINT32 *dstdata)
{
	adsp21xx_load_boot_data(srcdata, dstdata);
}

CPU_GET_INFO( adsp2104 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_LINES:					info->i = 6;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(adsp2104);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "ADSP2104");			break;

		default:
			CPU_GET_INFO_CALL(adsp21xx);
			break;
	}
}

/**************************************************************************
 * ADSP2105 section
 **************************************************************************/

static CPU_INIT( adsp2105 )
{
	adsp2100_state *adsp = adsp21xx_init(device, irqcallback, CHIP_TYPE_ADSP2105);
	adsp->mstat_mask = 0x7f;
	adsp->imask_mask = 0x3f;
}

void adsp2105_load_boot_data(UINT8 *srcdata, UINT32 *dstdata)
{
	adsp21xx_load_boot_data(srcdata, dstdata);
}

CPU_GET_INFO( adsp2105 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_LINES:					info->i = 3;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(adsp2105);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "ADSP2105");			break;

		default:
			CPU_GET_INFO_CALL(adsp21xx);
			break;
	}
}

/**************************************************************************
 * ADSP2115 section
 **************************************************************************/

static CPU_INIT( adsp2115 )
{
	adsp2100_state *adsp = adsp21xx_init(device, irqcallback, CHIP_TYPE_ADSP2115);
	adsp->mstat_mask = 0x7f;
	adsp->imask_mask = 0x3f;
}

void adsp2115_load_boot_data(UINT8 *srcdata, UINT32 *dstdata)
{
	adsp21xx_load_boot_data(srcdata, dstdata);
}

CPU_GET_INFO( adsp2115 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_LINES:					info->i = 6;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(adsp2115);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "ADSP2115");			break;

		default:
			CPU_GET_INFO_CALL(adsp21xx);
			break;
	}
}

/**************************************************************************
 * ADSP2181 section
 **************************************************************************/

static CPU_INIT( adsp2181 )
{
	adsp2100_state *adsp = adsp21xx_init(device, irqcallback, CHIP_TYPE_ADSP2181);
	adsp->mstat_mask = 0x7f;
	adsp->imask_mask = 0x3ff;
}

void adsp2181_load_boot_data(UINT8 *srcdata, UINT32 *dstdata)
{
	adsp21xx_load_boot_data(srcdata, dstdata);
}

CPU_GET_INFO( adsp2181 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_LINES:					info->i = 9;							break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:				info->i = 16;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:				info->i = 11;							break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:				info->i = -1;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(adsp2181);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "ADSP2181");					break;

		default:
			CPU_GET_INFO_CALL(adsp21xx);
			break;
	}
}

void adsp2181_idma_addr_w(running_device *device, UINT16 data)
{
	adsp2100_state *adsp = get_safe_token(device);
	adsp->idma_addr = data;
	adsp->idma_offs = 0;
}

UINT16 adsp2181_idma_addr_r(running_device *device)
{
	adsp2100_state *adsp = get_safe_token(device);
	return adsp->idma_addr;
}

void adsp2181_idma_data_w(running_device *device, UINT16 data)
{
	adsp2100_state *adsp = get_safe_token(device);

	/* program memory? */
	if (!(adsp->idma_addr & 0x4000))
	{
		/* upper 16 bits */
		if (adsp->idma_offs == 0)
		{
			adsp->idma_cache = data;
			adsp->idma_offs = 1;
		}

		/* lower 8 bits */
		else
		{
			WWORD_PGM(adsp, adsp->idma_addr++ & 0x3fff, (adsp->idma_cache << 8) | (data & 0xff));
			adsp->idma_offs = 0;
		}
	}

	/* data memory */
	else
		WWORD_DATA(adsp, adsp->idma_addr++ & 0x3fff, data);
}

UINT16 adsp2181_idma_data_r(running_device *device)
{
	adsp2100_state *adsp = get_safe_token(device);
	UINT16 result = 0xffff;

	/* program memory? */
	if (!(adsp->idma_addr & 0x4000))
	{
		/* upper 16 bits */
		if (adsp->idma_offs == 0)
		{
			result = RWORD_PGM(adsp, adsp->idma_addr & 0x3fff) >> 8;
			adsp->idma_offs = 1;
		}

		/* lower 8 bits */
		else
		{
			result = RWORD_PGM(adsp, adsp->idma_addr++ & 0x3fff) & 0xff;
			adsp->idma_offs = 0;
		}
	}

	/* data memory */
	else
		result = RWORD_DATA(adsp, adsp->idma_addr++ & 0x3fff);

	return result;
}
