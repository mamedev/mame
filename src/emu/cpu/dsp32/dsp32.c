/***************************************************************************

    dsp32.c
    Core implementation for the portable DSP32 emulator.
    Written by Aaron Giles

****************************************************************************

    Important note:

    At this time, the emulator is rather incomplete. However, it is
    sufficiently complete to run both Race Drivin' and Hard Drivin's
    Airborne, which is all I was after.

    Things that still need to be implemented:

        * interrupts
        * carry-reverse add operations
        * do loops
        * ieee/dsp conversions
        * input/output conversion
        * serial I/O

    In addition, there are several optimizations enabled which make
    assumptions about the code which may not be valid for other
    applications. Check dsp32ops.c for details.

***************************************************************************/

#include <math.h>
#include "debugger.h"
#include "dsp32.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define DETECT_MISALIGNED_MEMORY	0



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* internal register numbering for PIO registers */
#define PIO_PAR			0
#define PIO_PDR			1
#define PIO_EMR			2
#define PIO_ESR			3
#define PIO_PCR			4
#define PIO_PIR			5
#define PIO_PARE		6
#define PIO_PDR2		7
#define PIO_RESERVED	8

#define UPPER			(0x00ff << 8)
#define LOWER			(0xff00 << 8)

/* bits in the PCR register */
#define PCR_RESET		0x001
#define PCR_REGMAP		0x002
#define PCR_ENI			0x004
#define PCR_DMA			0x008
#define PCR_AUTO		0x010
#define PCR_PDFs		0x020
#define PCR_PIFs		0x040
#define PCR_RES			0x080
#define PCR_DMA32		0x100
#define PCR_PIO16		0x200
#define PCR_FLG			0x400

/* internal flag bits */
#define UFLAGBIT		1
#define VFLAGBIT		2



/***************************************************************************
    MACROS
***************************************************************************/

/* register mapping */
#define R0				r[0]
#define R1				r[1]
#define R2				r[2]
#define R3				r[3]
#define R4				r[4]
#define R5				r[5]
#define R6				r[6]
#define R7				r[7]
#define R8				r[8]
#define R9				r[9]
#define R10				r[10]
#define R11				r[11]
#define R12				r[12]
#define R13				r[13]
#define R14				r[14]
#define PC				r[15]
#define R0_ALT			r[16]
#define R15				r[17]
#define R16				r[18]
#define R17				r[19]
#define R18				r[20]
#define R19				r[21]
#define RMM				r[22]
#define RPP				r[23]
#define R20				r[24]
#define R21				r[25]
#define DAUC			r[26]
#define IOC				r[27]
#define R22				r[29]
#define PCSH			r[30]

#define A0				a[0]
#define A1				a[1]
#define A2				a[2]
#define A3				a[3]
#define A_0				a[4]
#define A_1				a[5]

#define OP				dsp32.op

#define zFLAG			((dsp32.nzcflags & 0xffffff) == 0)
#define nFLAG			((dsp32.nzcflags & 0x800000) != 0)
#define cFLAG			((dsp32.nzcflags & 0x1000000) != 0)
#define vFLAG			((dsp32.vflags & 0x800000) != 0)
#define ZFLAG			(dsp32.NZflags == 0)
#define NFLAG			(dsp32.NZflags < 0)
#define UFLAG			(dsp32.VUflags & UFLAGBIT)
#define VFLAG			(dsp32.VUflags & VFLAGBIT)



/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

/* DSP32 Registers */
typedef struct
{
	/* core registers */
	UINT32			r[32];
	UINT32			pin, pout;
	UINT32			ivtp;
	UINT32			nzcflags;
	UINT32			vflags;

	double			a[6];
	double			NZflags;
	UINT8			VUflags;

	double			abuf[4];
	UINT8			abufreg[4];
	UINT8			abufVUflags[4];
	UINT8			abufNZflags[4];
	int				abufcycle[4];
	int				abuf_index;

	INT32			mbufaddr[4];
	UINT32			mbufdata[4];
	int				mbuf_index;

	UINT16			par;
	UINT8			pare;
	UINT16			pdr;
	UINT16			pdr2;
	UINT16			pir;
	UINT16			pcr;
	UINT16			emr;
	UINT8			esr;
	UINT16			pcw;
	UINT8			piop;

	UINT32			ibuf;
	UINT32			isr;
	UINT32			obuf;
	UINT32			osr;

	/* internal stuff */
	UINT8			lastpins;
	UINT32			ppc;
	UINT32			op;
	int				interrupt_cycles;
	void			(*output_pins_changed)(UINT32 pins);
} dsp32_regs;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void dsp32c_reset(void);



/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

static dsp32_regs dsp32;
static int dsp32_icount;



/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

#define ROPCODE(pc)			cpu_readop32(pc)

#define RBYTE(addr)			program_read_byte_32le(addr)
#define WBYTE(addr,data)	program_write_byte_32le((addr), data)

#if (!DETECT_MISALIGNED_MEMORY)

#define RWORD(addr)			program_read_word_32le(addr)
#define WWORD(addr,data)	program_write_word_32le((addr), data)
#define RLONG(addr)			program_read_dword_32le(addr)
#define WLONG(addr,data)	program_write_dword_32le((addr), data)

#else

INLINE UINT16 RWORD(offs_t addr)
{
	UINT16 data;
	if (addr & 1) fprintf(stderr, "Unaligned word read @ %06X, PC=%06X\n", addr, dsp32.PC);
	data = program_read_word_32le(addr);
	return data;
}

INLINE UINT32 RLONG(offs_t addr)
{
	UINT32 data;
	if (addr & 3) fprintf(stderr, "Unaligned long read @ %06X, PC=%06X\n", addr, dsp32.PC);
	data = program_write_word_32le(addr);
	return data;
}

INLINE void WWORD(offs_t addr, UINT16 data)
{
	if (addr & 1) fprintf(stderr, "Unaligned word write @ %06X, PC=%06X\n", addr, dsp32.PC);
	program_read_dword_32le((addr), data);
}

INLINE void WLONG(offs_t addr, UINT32 data)
{
	if (addr & 3) fprintf(stderr, "Unaligned long write @ %06X, PC=%06X\n", addr, dsp32.PC);
	program_write_dword_32le((addr), data);
}

#endif



/***************************************************************************
    EXECEPTION HANDLING
***************************************************************************/

INLINE void generate_exception(int exception)
{
}


INLINE void invalid_instruction(UINT32 op)
{
}



/***************************************************************************
    IRQ HANDLING
***************************************************************************/

static void check_irqs(void)
{
	/* finish me! */
}


static void set_irq_line(int irqline, int state)
{
	/* finish me! */
}



/***************************************************************************
    REGISTER HANDLING
***************************************************************************/

static void update_pcr(UINT16 newval)
{
	UINT16 oldval = dsp32.pcr;
	dsp32.pcr = newval;

	/* reset the chip if we get a reset */
	if ((oldval & PCR_RESET) == 0 && (newval & PCR_RESET) != 0)
		dsp32c_reset();

	/* track the state of the output pins */
	if (dsp32.output_pins_changed)
	{
		UINT16 newoutput = ((newval & (PCR_PIFs | PCR_ENI)) == (PCR_PIFs | PCR_ENI)) ? DSP32_OUTPUT_PIF : 0;
		if (newoutput != dsp32.lastpins)
		{
			dsp32.lastpins = newoutput;
			(*dsp32.output_pins_changed)(newoutput);
		}
	}
}



/***************************************************************************
    CONTEXT SWITCHING
***************************************************************************/

static void dsp32c_get_context(void *dst)
{
	/* copy the context */
	if (dst)
		*(dsp32_regs *)dst = dsp32;
}


static void dsp32c_set_context(void *src)
{
	/* copy the context */
	if (src)
		dsp32 = *(dsp32_regs *)src;
	memory_set_opbase(dsp32.PC);

	/* check for IRQs */
	check_irqs();
}



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

static void dsp32c_init(int index, int clock, const void *_config, int (*irqcallback)(int))
{
	const struct dsp32_config *config = _config;

	/* copy in config data */
	if (config)
		dsp32.output_pins_changed = config->output_pins_changed;
}


static void dsp32c_reset(void)
{
	/* reset goes to 0 */
	dsp32.PC = 0;
	memory_set_opbase(dsp32.PC);

	/* clear some registers */
	dsp32.pcw &= 0x03ff;
	update_pcr(dsp32.pcr & PCR_RESET);
	dsp32.esr = 0;
	dsp32.emr = 0xffff;

	/* initialize fixed registers */
	dsp32.R0 = dsp32.R0_ALT = 0;
	dsp32.RMM = -1;
	dsp32.RPP = 1;
	dsp32.A_0 = 0.0;
	dsp32.A_1 = 1.0;

	/* init internal stuff */
	dsp32.abufcycle[0] = dsp32.abufcycle[1] = dsp32.abufcycle[2] = dsp32.abufcycle[3] = 12345678;
	dsp32.mbufaddr[0] = dsp32.mbufaddr[1] = dsp32.mbufaddr[2] = dsp32.mbufaddr[3] = 1;
}


static void dsp32c_exit(void)
{
}



/***************************************************************************
    CORE INCLUDE
***************************************************************************/

#include "dsp32ops.c"



/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

static int dsp32c_execute(int cycles)
{
	/* skip if halted */
	if ((dsp32.pcr & PCR_RESET) == 0)
		return cycles;

	/* count cycles and interrupt cycles */
	dsp32_icount = cycles;
	dsp32_icount -= dsp32.interrupt_cycles;
	dsp32.interrupt_cycles = 0;

	/* update buffered accumulator values */
	dsp32.abufcycle[0] += dsp32_icount;
	dsp32.abufcycle[1] += dsp32_icount;
	dsp32.abufcycle[2] += dsp32_icount;
	dsp32.abufcycle[3] += dsp32_icount;

	while (dsp32_icount > 0)
		execute_one();

	dsp32_icount -= dsp32.interrupt_cycles;
	dsp32.interrupt_cycles = 0;

	/* normalize buffered accumulator values */
	dsp32.abufcycle[0] -= dsp32_icount;
	dsp32.abufcycle[1] -= dsp32_icount;
	dsp32.abufcycle[2] -= dsp32_icount;
	dsp32.abufcycle[3] -= dsp32_icount;

	return cycles - dsp32_icount;
}



/***************************************************************************
    DISASSEMBLY HOOK
***************************************************************************/

#ifdef MAME_DEBUG
static offs_t dsp32c_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	extern unsigned dasm_dsp32(char *, unsigned, UINT32);
	return dasm_dsp32(buffer, pc, oprom[0] | (oprom[1] << 8) | (oprom[2] << 16) | (oprom[3] << 24));
}
#endif /* MAME_DEBUG */



/***************************************************************************
    PARALLEL INTERFACE WRITES
***************************************************************************/

/* context finder */
INLINE dsp32_regs *FINDCONTEXT(int cpu)
{
	dsp32_regs *context = cpunum_get_context_ptr(cpu);
	if (!context)
		context = &dsp32;
	return context;
}

static UINT32 regmap[4][16] =
{
	{	/* DSP32 compatible mode */
		PIO_PAR|LOWER, PIO_PAR|UPPER, PIO_PDR|LOWER, PIO_PDR|UPPER,
		PIO_EMR|LOWER, PIO_EMR|UPPER, PIO_ESR|LOWER, PIO_PCR|LOWER,
		PIO_PIR|UPPER, PIO_PIR|UPPER, PIO_PIR|UPPER, PIO_PIR|UPPER,
		PIO_PIR|UPPER, PIO_PIR|UPPER, PIO_PIR|UPPER, PIO_PIR|UPPER
	},
	{	/* DSP32C 8-bit mode */
		PIO_PAR|LOWER, PIO_PAR|UPPER, PIO_PDR|LOWER, PIO_PDR|UPPER,
		PIO_EMR|LOWER, PIO_EMR|UPPER, PIO_ESR|LOWER, PIO_PCR|LOWER,
		PIO_PIR|LOWER, PIO_PIR|UPPER, PIO_PCR|UPPER, PIO_PARE|LOWER,
		PIO_PDR2|LOWER,PIO_PDR2|UPPER,PIO_RESERVED,  PIO_RESERVED
	},
	{	/* DSP32C illegal mode */
		PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,
		PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,
		PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,
		PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED
	},
	{	/* DSP32C 16-bit mode */
		PIO_PAR,       PIO_RESERVED,  PIO_PDR,       PIO_RESERVED,
		PIO_EMR,       PIO_RESERVED,  PIO_ESR|LOWER, PIO_PCR,
		PIO_PIR,       PIO_RESERVED,  PIO_RESERVED,  PIO_PARE|LOWER,
		PIO_PDR2,      PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED
	}
};



/***************************************************************************
    PARALLEL INTERFACE WRITES
***************************************************************************/

INLINE void dma_increment(void)
{
	if (dsp32.pcr & PCR_AUTO)
	{
		int amount = (dsp32.pcr & PCR_DMA32) ? 4 : 2;
		dsp32.par += amount;
		if (dsp32.par < amount)
			dsp32.pare++;
	}
}


INLINE void dma_load(void)
{
	/* only process if DMA is enabled */
	if (dsp32.pcr & PCR_DMA)
	{
		UINT32 addr = dsp32.par | (dsp32.pare << 16);

		/* 16-bit case */
		if (!(dsp32.pcr & PCR_DMA32))
			dsp32.pdr = RWORD(addr & 0xfffffe);

		/* 32-bit case */
		else
		{
			UINT32 temp = RLONG(addr & 0xfffffc);
			dsp32.pdr = temp >> 16;
			dsp32.pdr2 = temp & 0xffff;
		}

		/* set the PDF flag to indicate we have data ready */
		update_pcr(dsp32.pcr | PCR_PDFs);
	}
}


INLINE void dma_store(void)
{
	/* only process if DMA is enabled */
	if (dsp32.pcr & PCR_DMA)
	{
		UINT32 addr = dsp32.par | (dsp32.pare << 16);

		/* 16-bit case */
		if (!(dsp32.pcr & PCR_DMA32))
			WWORD(addr & 0xfffffe, dsp32.pdr);

		/* 32-bit case */
		else
			WLONG(addr & 0xfffffc, (dsp32.pdr << 16) | dsp32.pdr2);

		/* clear the PDF flag to indicate we have taken the data */
		update_pcr(dsp32.pcr & ~PCR_PDFs);
	}
}


void dsp32c_pio_w(int cpunum, int reg, int data)
{
	UINT16 mask;
	UINT8 mode;

	cpuintrf_push_context(cpunum);

	/* look up register and mask */
	mode = ((dsp32.pcr >> 8) & 2) | ((dsp32.pcr >> 1) & 1);
	reg = regmap[mode][reg];
	mask = reg >> 8;
	if (mask == 0x00ff) data <<= 8;
	data &= ~mask;
	reg &= 0xff;

	/* switch off the register */
	switch (reg)
	{
		case PIO_PAR:
			dsp32.par = (dsp32.par & mask) | data;

			/* trigger a load on the upper half */
			if (!(mask & 0xff00))
				dma_load();
			break;

		case PIO_PARE:
			dsp32.pare = (dsp32.pare & mask) | data;
			break;

		case PIO_PDR:
			dsp32.pdr = (dsp32.pdr & mask) | data;

			/* trigger a write and PDF setting on the upper half */
			if (!(mask & 0xff00))
			{
				dma_store();
				dma_increment();
			}
			break;

		case PIO_PDR2:
			dsp32.pdr2 = (dsp32.pdr2 & mask) | data;
			break;

		case PIO_EMR:
			dsp32.emr = (dsp32.emr & mask) | data;
			break;

		case PIO_ESR:
			dsp32.esr = (dsp32.esr & mask) | data;
			break;

		case PIO_PCR:
			mask |= 0x0060;
			data &= ~mask;
			update_pcr((dsp32.pcr & mask) | data);
			break;

		case PIO_PIR:
			dsp32.pir = (dsp32.pir & mask) | data;

			/* set PIF on upper half */
			if (!(mask & 0xff00))
				update_pcr(dsp32.pcr | PCR_PIFs);
			break;

		/* error case */
		default:
			logerror("dsp32_pio_w called on invalid register %d\n", reg);
			break;
	}

	cpuintrf_pop_context();
}



/***************************************************************************
    PARALLEL INTERFACE READS
***************************************************************************/

int dsp32c_pio_r(int cpunum, int reg)
{
	UINT16 mask, result = 0xffff;
	UINT8 mode, shift = 0;

	cpuintrf_push_context(cpunum);

	/* look up register and mask */
	mode = ((dsp32.pcr >> 8) & 2) | ((dsp32.pcr >> 1) & 1);
	reg = regmap[mode][reg];
	mask = reg >> 8;
	if (mask == 0x00ff) mask = 0xff00, shift = 8;
	reg &= 0xff;

	/* switch off the register */
	switch (reg)
	{
		case PIO_PAR:
			result = dsp32.par | 1;
			break;

		case PIO_PARE:
			result = dsp32.pare;
			break;

		case PIO_PDR:
			result = dsp32.pdr;

			/* trigger an increment on the lower half */
			if (shift != 8)
				dma_increment();

			/* trigger a fetch on the upper half */
			if (!(mask & 0xff00))
				dma_load();
			break;

		case PIO_PDR2:
			result = dsp32.pdr2;
			break;

		case PIO_EMR:
			result = dsp32.emr;
			break;

		case PIO_ESR:
			result = dsp32.esr;
			break;

		case PIO_PCR:
			result = dsp32.pcr;
			break;

		case PIO_PIR:
			if (!(mask & 0xff00))
				update_pcr(dsp32.pcr & ~PCR_PIFs);	/* clear PIFs */
			result = dsp32.pir;
			break;

		/* error case */
		default:
			logerror("dsp32_pio_w called on invalid register %d\n", reg);
			break;
	}

	cpuintrf_pop_context();
	return (result >> shift) & ~mask;
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void dsp32c_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + DSP32_IRQ0:	set_irq_line(DSP32_IRQ0, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + DSP32_IRQ1:	set_irq_line(DSP32_IRQ1, info->i);			break;

		/* CAU */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + DSP32_PC:		dsp32.PC = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R0:		dsp32.R0 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R1:		dsp32.R1 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R2:		dsp32.R2 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R3:		dsp32.R3 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R4:		dsp32.R4 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R5:		dsp32.R5 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R6:		dsp32.R6 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R7:		dsp32.R7 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R8:		dsp32.R8 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R9:		dsp32.R9 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R10:		dsp32.R10 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R11:		dsp32.R11 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R12:		dsp32.R12 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R13:		dsp32.R13 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R14:		dsp32.R14 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R15:		dsp32.R15 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R16:		dsp32.R16 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R17:		dsp32.R17 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R18:		dsp32.R18 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R19:		dsp32.R19 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R20:		dsp32.R20 = info->i & 0xffffff;				break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + DSP32_R21:		dsp32.R21 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R22:		dsp32.R22 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_PIN:		dsp32.pin = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_POUT:		dsp32.pout = info->i & 0xffffff;			break;
		case CPUINFO_INT_REGISTER + DSP32_IVTP:		dsp32.ivtp = info->i & 0xffffff;			break;

		/* DAU */
		case CPUINFO_INT_REGISTER + DSP32_A0:		dsp32.A0 = info->i;	/* fix me -- very wrong */ break;
		case CPUINFO_INT_REGISTER + DSP32_A1:		dsp32.A1 = info->i;	/* fix me -- very wrong */ break;
		case CPUINFO_INT_REGISTER + DSP32_A2:		dsp32.A2 = info->i;	/* fix me -- very wrong */ break;
		case CPUINFO_INT_REGISTER + DSP32_A3:		dsp32.A3 = info->i;	/* fix me -- very wrong */ break;
		case CPUINFO_INT_REGISTER + DSP32_DAUC:		dsp32.DAUC = info->i;						break;

		/* PIO */
		case CPUINFO_INT_REGISTER + DSP32_PAR:		dsp32.par = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_PDR:		dsp32.pdr = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_PIR:		dsp32.pir = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_PCR:		update_pcr(info->i & 0x3ff);				break;
		case CPUINFO_INT_REGISTER + DSP32_EMR:		dsp32.emr = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_ESR:		dsp32.esr = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_PCW:		dsp32.pcw = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_PIOP:		dsp32.piop = info->i;						break;

		/* SIO */
		case CPUINFO_INT_REGISTER + DSP32_IBUF:		dsp32.ibuf = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_ISR:		dsp32.isr = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_OBUF:		dsp32.obuf = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_OSR:		dsp32.osr = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_IOC:		dsp32.IOC = info->i & 0xfffff;				break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void dsp32c_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(dsp32);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 4;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 4;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 24;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + DSP32_IRQ0:		info->i = 0;							break;
		case CPUINFO_INT_INPUT_STATE + DSP32_IRQ1:		info->i = 0;							break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = dsp32.ppc;					break;

		/* CAU */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + DSP32_PC:			info->i = dsp32.PC;						break;
		case CPUINFO_INT_REGISTER + DSP32_R0:			info->i = dsp32.R0;						break;
		case CPUINFO_INT_REGISTER + DSP32_R1:			info->i = dsp32.R1;						break;
		case CPUINFO_INT_REGISTER + DSP32_R2:			info->i = dsp32.R2;						break;
		case CPUINFO_INT_REGISTER + DSP32_R3:			info->i = dsp32.R3;						break;
		case CPUINFO_INT_REGISTER + DSP32_R4:			info->i = dsp32.R4;						break;
		case CPUINFO_INT_REGISTER + DSP32_R5:			info->i = dsp32.R5;						break;
		case CPUINFO_INT_REGISTER + DSP32_R6:			info->i = dsp32.R6;						break;
		case CPUINFO_INT_REGISTER + DSP32_R7:			info->i = dsp32.R7;						break;
		case CPUINFO_INT_REGISTER + DSP32_R8:			info->i = dsp32.R8;						break;
		case CPUINFO_INT_REGISTER + DSP32_R9:			info->i = dsp32.R9;						break;
		case CPUINFO_INT_REGISTER + DSP32_R10:			info->i = dsp32.R10;					break;
		case CPUINFO_INT_REGISTER + DSP32_R11:			info->i = dsp32.R11;					break;
		case CPUINFO_INT_REGISTER + DSP32_R12:			info->i = dsp32.R12;					break;
		case CPUINFO_INT_REGISTER + DSP32_R13:			info->i = dsp32.R13;					break;
		case CPUINFO_INT_REGISTER + DSP32_R14:			info->i = dsp32.R14;					break;
		case CPUINFO_INT_REGISTER + DSP32_R15:			info->i = dsp32.R15;					break;
		case CPUINFO_INT_REGISTER + DSP32_R16:			info->i = dsp32.R16;					break;
		case CPUINFO_INT_REGISTER + DSP32_R17:			info->i = dsp32.R17;					break;
		case CPUINFO_INT_REGISTER + DSP32_R18:			info->i = dsp32.R18;					break;
		case CPUINFO_INT_REGISTER + DSP32_R19:			info->i = dsp32.R19;					break;
		case CPUINFO_INT_REGISTER + DSP32_R20:			info->i = dsp32.R20;					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + DSP32_R21:			info->i = dsp32.R21;					break;
		case CPUINFO_INT_REGISTER + DSP32_R22:			info->i = dsp32.R22;					break;
		case CPUINFO_INT_REGISTER + DSP32_PIN:			info->i = dsp32.pin;					break;
		case CPUINFO_INT_REGISTER + DSP32_POUT:			info->i = dsp32.pout;					break;
		case CPUINFO_INT_REGISTER + DSP32_IVTP:			info->i = dsp32.ivtp;					break;

		/* DAU */
		case CPUINFO_INT_REGISTER + DSP32_A0:			info->i = dsp32.A0;	/* fix me -- very wrong */ break;
		case CPUINFO_INT_REGISTER + DSP32_A1:			info->i = dsp32.A1;	/* fix me -- very wrong */ break;
		case CPUINFO_INT_REGISTER + DSP32_A2:			info->i = dsp32.A2;	/* fix me -- very wrong */ break;
		case CPUINFO_INT_REGISTER + DSP32_A3:			info->i = dsp32.A3;	/* fix me -- very wrong */ break;
		case CPUINFO_INT_REGISTER + DSP32_DAUC:			info->i = dsp32.DAUC;					break;

		/* PIO */
		case CPUINFO_INT_REGISTER + DSP32_PAR:			info->i = dsp32.par;					break;
		case CPUINFO_INT_REGISTER + DSP32_PDR:			info->i = dsp32.pdr;					break;
		case CPUINFO_INT_REGISTER + DSP32_PIR:			info->i = dsp32.pir;					break;
		case CPUINFO_INT_REGISTER + DSP32_PCR:			info->i = dsp32.pcr;					break;
		case CPUINFO_INT_REGISTER + DSP32_EMR:			info->i = dsp32.emr;					break;
		case CPUINFO_INT_REGISTER + DSP32_ESR:			info->i = dsp32.esr;					break;
		case CPUINFO_INT_REGISTER + DSP32_PCW:			info->i = dsp32.pcw;					break;
		case CPUINFO_INT_REGISTER + DSP32_PIOP:			info->i = dsp32.piop;					break;

		/* SIO */
		case CPUINFO_INT_REGISTER + DSP32_IBUF:			info->i = dsp32.ibuf;					break;
		case CPUINFO_INT_REGISTER + DSP32_ISR:			info->i = dsp32.isr;					break;
		case CPUINFO_INT_REGISTER + DSP32_OBUF:			info->i = dsp32.obuf;					break;
		case CPUINFO_INT_REGISTER + DSP32_OSR:			info->i = dsp32.osr;					break;
		case CPUINFO_INT_REGISTER + DSP32_IOC:			info->i = dsp32.IOC;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = dsp32c_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = dsp32c_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = dsp32c_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = dsp32c_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = dsp32c_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = dsp32c_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = dsp32c_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = dsp32c_dasm;		break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &dsp32_icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "DSP32C");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Lucent DSP32");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Aaron Giles");			break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				NFLAG ? 'N':'.',
				ZFLAG ? 'Z':'.',
                UFLAG ? 'U':'.',
                VFLAG ? 'V':'.',
                nFLAG ? 'n':'.',
                zFLAG ? 'z':'.',
                cFLAG ? 'c':'.',
                vFLAG ? 'v':'.');
            break;

		/* CAU */
		case CPUINFO_STR_REGISTER + DSP32_PC:			sprintf(info->s, "PC: %06X", dsp32.PC); break;
		case CPUINFO_STR_REGISTER + DSP32_R0:			sprintf(info->s, "R0: %06X", dsp32.R0); break;
		case CPUINFO_STR_REGISTER + DSP32_R1:			sprintf(info->s, "R1: %06X", dsp32.R1); break;
		case CPUINFO_STR_REGISTER + DSP32_R2:			sprintf(info->s, "R2: %06X", dsp32.R2); break;
		case CPUINFO_STR_REGISTER + DSP32_R3:			sprintf(info->s, "R3: %06X", dsp32.R3); break;
		case CPUINFO_STR_REGISTER + DSP32_R4:			sprintf(info->s, "R4: %06X", dsp32.R4); break;
		case CPUINFO_STR_REGISTER + DSP32_R5:			sprintf(info->s, "R5: %06X", dsp32.R5); break;
		case CPUINFO_STR_REGISTER + DSP32_R6:			sprintf(info->s, "R6: %06X", dsp32.R6); break;
		case CPUINFO_STR_REGISTER + DSP32_R7:			sprintf(info->s, "R7: %06X", dsp32.R7); break;
		case CPUINFO_STR_REGISTER + DSP32_R8:			sprintf(info->s, "R8: %06X", dsp32.R8); break;
		case CPUINFO_STR_REGISTER + DSP32_R9:			sprintf(info->s, "R9: %06X", dsp32.R9); break;
		case CPUINFO_STR_REGISTER + DSP32_R10:			sprintf(info->s, "R10:%06X", dsp32.R10); break;
		case CPUINFO_STR_REGISTER + DSP32_R11:			sprintf(info->s, "R11:%06X", dsp32.R11); break;
		case CPUINFO_STR_REGISTER + DSP32_R12:			sprintf(info->s, "R12:%06X", dsp32.R12); break;
		case CPUINFO_STR_REGISTER + DSP32_R13:			sprintf(info->s, "R13:%06X", dsp32.R13); break;
		case CPUINFO_STR_REGISTER + DSP32_R14:			sprintf(info->s, "R14:%06X", dsp32.R14); break;
		case CPUINFO_STR_REGISTER + DSP32_R15:			sprintf(info->s, "R15:%06X", dsp32.R15); break;
		case CPUINFO_STR_REGISTER + DSP32_R16:			sprintf(info->s, "R16:%06X", dsp32.R16); break;
		case CPUINFO_STR_REGISTER + DSP32_R17:			sprintf(info->s, "R17:%06X", dsp32.R17); break;
		case CPUINFO_STR_REGISTER + DSP32_R18:			sprintf(info->s, "R18:%06X", dsp32.R18); break;
		case CPUINFO_STR_REGISTER + DSP32_R19:			sprintf(info->s, "R19:%06X", dsp32.R19); break;
		case CPUINFO_STR_REGISTER + DSP32_R20:			sprintf(info->s, "R20:%06X", dsp32.R20); break;
		case CPUINFO_STR_REGISTER + DSP32_R21:			sprintf(info->s, "R21:%06X", dsp32.R21); break;
		case CPUINFO_STR_REGISTER + DSP32_R22:			sprintf(info->s, "R22:%06X", dsp32.R22); break;
		case CPUINFO_STR_REGISTER + DSP32_PIN:			sprintf(info->s, "PIN:%06X", dsp32.pin); break;
		case CPUINFO_STR_REGISTER + DSP32_POUT:			sprintf(info->s, "POUT:%06X", dsp32.pout); break;
		case CPUINFO_STR_REGISTER + DSP32_IVTP:			sprintf(info->s, "IVTP:%06X", dsp32.ivtp); break;

		/* DAU */
		case CPUINFO_STR_REGISTER + DSP32_A0:			sprintf(info->s, "A0:%8g", dsp32.A0); break;
		case CPUINFO_STR_REGISTER + DSP32_A1:			sprintf(info->s, "A1:%8g", dsp32.A1); break;
		case CPUINFO_STR_REGISTER + DSP32_A2:			sprintf(info->s, "A2:%8g", dsp32.A2); break;
		case CPUINFO_STR_REGISTER + DSP32_A3:			sprintf(info->s, "A3:%8g", dsp32.A3); break;
		case CPUINFO_STR_REGISTER + DSP32_DAUC:			sprintf(info->s, "DAUC:%02X", dsp32.DAUC); break;

		/* PIO */
		case CPUINFO_STR_REGISTER + DSP32_PAR:			sprintf(info->s, "PAR:%08X", dsp32.par); break;
		case CPUINFO_STR_REGISTER + DSP32_PDR:			sprintf(info->s, "PDR:%08X", dsp32.pdr); break;
		case CPUINFO_STR_REGISTER + DSP32_PIR:			sprintf(info->s, "PIR:%04X", dsp32.pir); break;
		case CPUINFO_STR_REGISTER + DSP32_PCR:			sprintf(info->s, "PCR:%03X", dsp32.pcr); break;
		case CPUINFO_STR_REGISTER + DSP32_EMR:			sprintf(info->s, "EMR:%04X", dsp32.emr); break;
		case CPUINFO_STR_REGISTER + DSP32_ESR:			sprintf(info->s, "ESR:%02X", dsp32.esr); break;
		case CPUINFO_STR_REGISTER + DSP32_PCW:			sprintf(info->s, "PCW:%04X", dsp32.pcw); break;
		case CPUINFO_STR_REGISTER + DSP32_PIOP:			sprintf(info->s, "PIOP:%02X", dsp32.piop); break;

		/* SIO */
		case CPUINFO_STR_REGISTER + DSP32_IBUF:			sprintf(info->s, "IBUF:%08X", dsp32.ibuf); break;
		case CPUINFO_STR_REGISTER + DSP32_ISR:			sprintf(info->s, "ISR:%08X", dsp32.isr); break;
		case CPUINFO_STR_REGISTER + DSP32_OBUF:			sprintf(info->s, "OBUF:%08X", dsp32.obuf); break;
		case CPUINFO_STR_REGISTER + DSP32_OSR:			sprintf(info->s, "OSR:%08X", dsp32.osr); break;
		case CPUINFO_STR_REGISTER + DSP32_IOC:			sprintf(info->s, "IOC:%05X", dsp32.IOC); break;
	}
}
