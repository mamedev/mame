/***************************************************************************

    tms32031.c
    Core implementation for the portable TMS32C031 emulator.
    Written by Aaron Giles

***************************************************************************/

#include "debugger.h"
#include "tms32031.h"
#include "eminline.h"


#define LOG_OPCODE_USAGE	(0)


/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	TMR_R0 = 0,
	TMR_R1,
	TMR_R2,
	TMR_R3,
	TMR_R4,
	TMR_R5,
	TMR_R6,
	TMR_R7,
	TMR_AR0,
	TMR_AR1,
	TMR_AR2,
	TMR_AR3,
	TMR_AR4,
	TMR_AR5,
	TMR_AR6,
	TMR_AR7,
	TMR_DP,
	TMR_IR0,
	TMR_IR1,
	TMR_BK,
	TMR_SP,
	TMR_ST,
	TMR_IE,
	TMR_IF,
	TMR_IOF,
	TMR_RS,
	TMR_RE,
	TMR_RC,
	TMR_R8,		/* 3204x only */
	TMR_R9,		/* 3204x only */
	TMR_R10,	/* 3204x only */
	TMR_R11,	/* 3204x only */
	TMR_TEMP1,	/* used by the interpreter */
	TMR_TEMP2,	/* used by the interpreter */
	TMR_TEMP3	/* used by the interpreter */
};

#define CFLAG		0x0001
#define VFLAG		0x0002
#define ZFLAG		0x0004
#define NFLAG		0x0008
#define UFFLAG		0x0010
#define LVFLAG		0x0020
#define LUFFLAG		0x0040
#define OVMFLAG		0x0080
#define RMFLAG		0x0100
#define CFFLAG		0x0400
#define CEFLAG		0x0800
#define CCFLAG		0x1000
#define GIEFLAG		0x2000

#define IREG(rnum)			(tms32031.r[rnum].i32[0])



/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

union genreg
{
	UINT32		i32[2];
	UINT16		i16[4];
	UINT8		i8[8];
};

/* TMS34031 Registers */
typedef struct
{
	/* core registers */
	UINT32			pc;
	union genreg	r[36];
	UINT32			bkmask;

	/* internal stuff */
	UINT32			op;
	UINT16			irq_state;
	UINT8			delayed;
	UINT8			irq_pending;
	UINT8			mcu_mode;
	UINT8			is_32032;
	UINT8			is_idling;
	int				interrupt_cycles;

	UINT32			bootoffset;
	void			(*xf0_w)(UINT8 val);
	void			(*xf1_w)(UINT8 val);
	void			(*iack_w)(UINT8 val, offs_t addr);
	int				(*irq_callback)(int state);
} tms32031_regs;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void trap(int trapnum);
static UINT32 boot_loader(UINT32 boot_rom_addr);



/***************************************************************************
    PUBLIC GLOBAL VARIABLES
***************************************************************************/

static int	tms32031_icount;



/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

static tms32031_regs tms32031;



/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

#define ROPCODE(pc)		cpu_readop32((pc) << 2)
#define OP				tms32031.op

#define RMEM(addr)		program_read_dword_32le((addr) << 2)
#define WMEM(addr,data)	program_write_dword_32le((addr) << 2, data)
#define UPDATEPC(addr)	change_pc((addr) << 2)



/***************************************************************************
    HELPER MACROS
***************************************************************************/

#define MANTISSA(r)			((INT32)(r)->i32[0])
#define EXPONENT(r)			((INT8)(r)->i32[1])
#define SET_MANTISSA(r,v)	((r)->i32[0] = (v))
#define SET_EXPONENT(r,v)	((r)->i32[1] = (v))

typedef union int_double
{
	double d;
	float f[2];
	UINT32 i[2];
} int_double;


static float dsp_to_float(union genreg *fp)
{
	int_double id;

	if (MANTISSA(fp) == 0 && EXPONENT(fp) == -128)
		return 0;
	else if (MANTISSA(fp) >= 0)
	{
		int exponent = (EXPONENT(fp) + 127) << 23;
		id.i[0] = exponent + (MANTISSA(fp) >> 8);
	}
	else
	{
		int exponent = (EXPONENT(fp) + 127) << 23;
		INT32 man = -MANTISSA(fp);
		id.i[0] = 0x80000000 + exponent + ((man >> 8) & 0x00ffffff);
	}
	return id.f[0];
}


static double dsp_to_double(union genreg *fp)
{
	int_double id;

	if (MANTISSA(fp) == 0 && EXPONENT(fp) == -128)
		return 0;
	else if (MANTISSA(fp) >= 0)
	{
		int exponent = (EXPONENT(fp) + 1023) << 20;
		id.i[BYTE_XOR_BE(0)] = exponent + (MANTISSA(fp) >> 11);
		id.i[BYTE_XOR_BE(1)] = (MANTISSA(fp) << 21) & 0xffe00000;
	}
	else
	{
		int exponent = (EXPONENT(fp) + 1023) << 20;
		INT32 man = -MANTISSA(fp);
		id.i[BYTE_XOR_BE(0)] = 0x80000000 + exponent + ((man >> 11) & 0x001fffff);
		id.i[BYTE_XOR_BE(1)] = (man << 21) & 0xffe00000;
	}
	return id.d;
}


static void double_to_dsp(double val, union genreg *result)
{
	int mantissa, exponent;
	int_double id;
	id.d = val;

	mantissa = ((id.i[BYTE_XOR_BE(0)] & 0x000fffff) << 11) | ((id.i[BYTE_XOR_BE(1)] & 0xffe00000) >> 21);
	exponent = ((id.i[BYTE_XOR_BE(0)] & 0x7ff00000) >> 20) - 1023;
	if (exponent < -128)
	{
		SET_MANTISSA(result, 0);
		SET_EXPONENT(result, -128);
	}
	else if (exponent > 127)
	{
		if ((INT32)id.i[BYTE_XOR_BE(0)] >= 0)
			SET_MANTISSA(result, 0x7fffffff);
		else
			SET_MANTISSA(result, 0x80000001);
		SET_EXPONENT(result, 127);
	}
	else if ((INT32)id.i[BYTE_XOR_BE(0)] >= 0)
	{
		SET_MANTISSA(result, mantissa);
		SET_EXPONENT(result, exponent);
	}
	else if (mantissa != 0)
	{
		SET_MANTISSA(result, 0x80000000 | -mantissa);
		SET_EXPONENT(result, exponent);
	}
	else
	{
		SET_MANTISSA(result, 0x80000000);
		SET_EXPONENT(result, exponent - 1);
	}
}


float convert_tms3203x_fp_to_float(UINT32 floatdata)
{
	union genreg gen;

	SET_MANTISSA(&gen, floatdata << 8);
	SET_EXPONENT(&gen, (INT32)floatdata >> 24);

	return dsp_to_float(&gen);
}


double convert_tms3203x_fp_to_double(UINT32 floatdata)
{
	union genreg gen;

	SET_MANTISSA(&gen, floatdata << 8);
	SET_EXPONENT(&gen, (INT32)floatdata >> 24);

	return dsp_to_double(&gen);
}


UINT32 convert_float_to_tms3203x_fp(float fval)
{
	union genreg gen;

	double_to_dsp(fval, &gen);
	return (EXPONENT(&gen) << 24) | ((UINT32)MANTISSA(&gen) >> 8);
}


UINT32 convert_double_to_tms3203x_fp(double dval)
{
	union genreg gen;

	double_to_dsp(dval, &gen);
	return (EXPONENT(&gen) << 24) | ((UINT32)MANTISSA(&gen) >> 8);
}



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
	int whichtrap = 0;
	UINT16 validints;
	int i;

	/* external interrupts are level-sensitive on the '31 and can be
       configured as such on the '32; in that case, if the external
       signal is high, we need to update the value in IF accordingly */
	if (!tms32031.is_32032 || (IREG(TMR_ST) & 0x4000) == 0)
		IREG(TMR_IF) |= tms32031.irq_state & 0x0f;

	/* determine if we have any live interrupts */
	validints = IREG(TMR_IF) & IREG(TMR_IE) & 0x0fff;
	if (validints == 0 || (IREG(TMR_ST) & GIEFLAG) == 0)
		return;

	/* find the lowest signalled value */
	for (i = 0; i < 12; i++)
		if (validints & (1 << i))
		{
			whichtrap = i + 1;
			break;
		}

	/* no longer idling if we get here */
	tms32031.is_idling = FALSE;
	if (!tms32031.delayed)
	{
		UINT16 intmask = 1 << (whichtrap - 1);

		/* bit in IF is cleared when interrupt is taken */
		IREG(TMR_IF) &= ~intmask;
		trap(whichtrap);

		/* after auto-clearing the interrupt bit, we need to re-trigger
           level-sensitive interrupts */
		if (!tms32031.is_32032 || (IREG(TMR_ST) & 0x4000) == 0)
			IREG(TMR_IF) |= tms32031.irq_state & 0x0f;
	}
	else
		tms32031.irq_pending = TRUE;
}


static void set_irq_line(int irqline, int state)
{
	UINT16 intmask = 1 << irqline;

	/* ignore anything out of range */
	if (irqline >= 12)
		return;

	/* update the external state */
    if (state == ASSERT_LINE)
    {
		tms32031.irq_state |= intmask;
	    IREG(TMR_IF) |= intmask;
	}
	else
		tms32031.irq_state &= ~intmask;

	/* check for IRQs */
	check_irqs();
}



/***************************************************************************
    CONTEXT SWITCHING
***************************************************************************/

static void tms32031_get_context(void *dst)
{
	/* copy the context */
	if (dst)
		*(tms32031_regs *)dst = tms32031;
}


static void tms32031_set_context(void *src)
{
	/* copy the context */
	if (src)
		tms32031 = *(tms32031_regs *)src;
	UPDATEPC(tms32031.pc);

	/* check for IRQs */
	check_irqs();
}



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

static void tms32031_init(int index, int clock, const void *_config, int (*irqcallback)(int))
{
	const struct tms32031_config *config = _config;
	int i;
	char namebuf[30];

	tms32031.irq_callback = irqcallback;

	/* copy in the xf write routines */
	tms32031.bootoffset = config ? config->bootoffset : 0;
	if (config)
	{
		tms32031.xf0_w = config->xf0_w;
		tms32031.xf1_w = config->xf1_w;
		tms32031.iack_w = config->iack_w;
	}

	state_save_register_item("tms32031", index, tms32031.pc);
	for (i=0;i<36;i++)
	{
		sprintf(namebuf,"tms32031.r[%d]",i);
		state_save_register_generic("tms32031", index, namebuf, tms32031.r[i].i8, UINT8, 8);
	}
	state_save_register_item("tms32031", index, tms32031.bkmask);
	state_save_register_item("tms32031", index, tms32031.op);
	state_save_register_item("tms32031", index, tms32031.irq_state);
	state_save_register_item("tms32031", index, tms32031.delayed);
	state_save_register_item("tms32031", index, tms32031.irq_pending);
	state_save_register_item("tms32031", index, tms32031.mcu_mode);
	state_save_register_item("tms32031", index, tms32031.is_idling);
	state_save_register_item("tms32031", index, tms32031.interrupt_cycles);

}

static void tms32031_reset(void)
{
	/* if we have a config struct, get the boot ROM address */
	if (tms32031.bootoffset)
	{
		tms32031.mcu_mode = TRUE;
		tms32031.pc = boot_loader(tms32031.bootoffset);
	}
	else
	{
		tms32031.mcu_mode = FALSE;
		tms32031.pc = RMEM(0);
	}
	tms32031.is_32032 = FALSE;

	/* reset some registers */
	IREG(TMR_IE) = 0;
	IREG(TMR_IF) = 0;
	IREG(TMR_ST) = 0;
	IREG(TMR_IOF) = 0;

	/* reset internal stuff */
	tms32031.delayed = tms32031.irq_pending = FALSE;
	tms32031.is_idling = FALSE;
}

static void tms32032_reset(void)
{
	tms32031_reset();
	tms32031.is_32032 = TRUE;
}


#if (LOG_OPCODE_USAGE)
static UINT32 hits[0x200*4];
#endif

static void tms32031_exit(void)
{
#if (LOG_OPCODE_USAGE)
	int i;
	for (i = 0; i < 0x200*4; i++)
		if (hits[i])
			mame_printf_debug("%10d - %03X.%X\n", hits[i], i / 4, i % 4);
#endif
}



/***************************************************************************
    CORE OPCODES
***************************************************************************/

#include "32031ops.c"



/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

static int tms32031_execute(int cycles)
{
	/* count cycles and interrupt cycles */
	tms32031_icount = cycles;
	tms32031_icount -= tms32031.interrupt_cycles;
	tms32031.interrupt_cycles = 0;

	/* check IRQs up front */
	check_irqs();

	/* if we're idling, just eat the cycles */
	if (tms32031.is_idling)
		return tms32031_icount;

	while (tms32031_icount > 0)
	{
#ifdef MAME_DEBUG
	if (IREG(TMR_SP) & 0xff000000)
		DEBUGGER_BREAK;
#endif
		if ((IREG(TMR_ST) & RMFLAG) && tms32031.pc == IREG(TMR_RE) + 1)
		{
			if ((INT32)--IREG(TMR_RC) >= 0)
				tms32031.pc = IREG(TMR_RS);
			else
			{
				IREG(TMR_ST) &= ~RMFLAG;
				if (tms32031.delayed)
				{
					tms32031.delayed = FALSE;
					if (tms32031.irq_pending)
					{
						tms32031.irq_pending = FALSE;
						check_irqs();
					}
				}
			}
			continue;
		}

		execute_one();
	}

	tms32031_icount -= tms32031.interrupt_cycles;
	tms32031.interrupt_cycles = 0;

	return cycles - tms32031_icount;
}



/***************************************************************************
    DISASSEMBLY HOOK
***************************************************************************/

#ifdef MAME_DEBUG
static offs_t tms32031_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	UINT32 op = oprom[0] | (oprom[1] << 8) | (oprom[2] << 16) | (oprom[3] << 24);
	extern unsigned dasm_tms32031(char *, unsigned, UINT32);
    return dasm_tms32031(buffer, pc, op);
}
#endif /* MAME_DEBUG */



/***************************************************************************
    BOOT LOADER
***************************************************************************/

static UINT32 boot_loader(UINT32 boot_rom_addr)
{
	UINT32 bits, control, advance;
	UINT32 start_offset = 0;
	UINT32 datamask;
	int first = 1, i;

	/* read the size of the data */
	bits = RMEM(boot_rom_addr);
	if (bits != 8 && bits != 16 && bits != 32)
		return 0;
	datamask = 0xffffffffUL >> (32 - bits);
	advance = 32 / bits;
	boot_rom_addr += advance;

	/* read the control register */
	control = RMEM(boot_rom_addr++) & datamask;
	for (i = 1; i < advance; i++)
		control |= (RMEM(boot_rom_addr++) & datamask) << (bits * i);

	/* now parse the data */
	while (1)
	{
		UINT32 offs, len;

		/* read the length of this section */
		len = RMEM(boot_rom_addr++) & datamask;
		for (i = 1; i < advance; i++)
			len |= (RMEM(boot_rom_addr++) & datamask) << (bits * i);

		/* stop at 0 */
		if (len == 0)
			return start_offset;

		/* read the destination offset of this section */
		offs = RMEM(boot_rom_addr++) & datamask;
		for (i = 1; i < advance; i++)
			offs |= (RMEM(boot_rom_addr++) & datamask) << (bits * i);

		/* if this is the first block, that's where we boot to */
		if (first)
		{
			start_offset = offs;
			first = 0;
		}

		/* now copy the data */
		while (len--)
		{
			UINT32 data;

			/* extract the 32-bit word */
			data = RMEM(boot_rom_addr++) & datamask;
			for (i = 1; i < advance; i++)
				data |= (RMEM(boot_rom_addr++) & datamask) << (bits * i);

			/* write it out */
			WMEM(offs++, data);
		}
	}

	/* keep the compiler happy */
	return 0;
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void tms32031_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + TMS32031_IRQ0:	set_irq_line(TMS32031_IRQ0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_IRQ1:	set_irq_line(TMS32031_IRQ1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_IRQ2:	set_irq_line(TMS32031_IRQ2, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_IRQ3:	set_irq_line(TMS32031_IRQ3, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_XINT0:	set_irq_line(TMS32031_XINT0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_RINT0:	set_irq_line(TMS32031_RINT0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_XINT1:	set_irq_line(TMS32031_XINT1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_RINT1:	set_irq_line(TMS32031_RINT1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_TINT0:	set_irq_line(TMS32031_TINT0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_TINT1:	set_irq_line(TMS32031_TINT1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_DINT:	set_irq_line(TMS32031_DINT, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_DINT1:	set_irq_line(TMS32031_DINT1, info->i);	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32031_PC:		tms32031.pc = info->i; 					break;
		case CPUINFO_INT_REGISTER + TMS32031_R0:		IREG(TMR_R0) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_R1:		IREG(TMR_R1) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_R2:		IREG(TMR_R2) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_R3:		IREG(TMR_R3) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_R4:		IREG(TMR_R4) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_R5:		IREG(TMR_R5) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_R6:		IREG(TMR_R6) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_R7:		IREG(TMR_R7) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_R0F:		double_to_dsp(*(float *)&info->i, &tms32031.r[TMR_R0]); break;
		case CPUINFO_INT_REGISTER + TMS32031_R1F:		double_to_dsp(*(float *)&info->i, &tms32031.r[TMR_R1]); break;
		case CPUINFO_INT_REGISTER + TMS32031_R2F:		double_to_dsp(*(float *)&info->i, &tms32031.r[TMR_R2]); break;
		case CPUINFO_INT_REGISTER + TMS32031_R3F:		double_to_dsp(*(float *)&info->i, &tms32031.r[TMR_R3]); break;
		case CPUINFO_INT_REGISTER + TMS32031_R4F:		double_to_dsp(*(float *)&info->i, &tms32031.r[TMR_R4]); break;
		case CPUINFO_INT_REGISTER + TMS32031_R5F:		double_to_dsp(*(float *)&info->i, &tms32031.r[TMR_R5]); break;
		case CPUINFO_INT_REGISTER + TMS32031_R6F:		double_to_dsp(*(float *)&info->i, &tms32031.r[TMR_R6]); break;
		case CPUINFO_INT_REGISTER + TMS32031_R7F:		double_to_dsp(*(float *)&info->i, &tms32031.r[TMR_R7]); break;
		case CPUINFO_INT_REGISTER + TMS32031_AR0:		IREG(TMR_AR0) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR1:		IREG(TMR_AR1) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR2:		IREG(TMR_AR2) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR3:		IREG(TMR_AR3) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR4:		IREG(TMR_AR4) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR5:		IREG(TMR_AR5) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR6:		IREG(TMR_AR6) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR7:		IREG(TMR_AR7) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_DP:		IREG(TMR_DP) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_IR0:		IREG(TMR_IR0) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_IR1:		IREG(TMR_IR1) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_BK:		IREG(TMR_BK) = info->i; 				break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + TMS32031_SP:		IREG(TMR_SP) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_ST:		IREG(TMR_ST) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_IE:		IREG(TMR_IE) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_IF:		IREG(TMR_IF) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_IOF:		IREG(TMR_IOF) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_RS:		IREG(TMR_RS) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_RE:		IREG(TMR_RE) = info->i; 				break;
		case CPUINFO_INT_REGISTER + TMS32031_RC:		IREG(TMR_RC) = info->i; 				break;
	}
}


/**************************************************************************
 * Internal memory map
 **************************************************************************/

static ADDRESS_MAP_START( internal_32031, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x809800, 0x809fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( internal_32032, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x87fe00, 0x87ffff) AM_RAM
ADDRESS_MAP_END


/**************************************************************************
 * Generic get_info
 **************************************************************************/

void tms32031_get_info(UINT32 state, cpuinfo *info)
{
	float ftemp;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(tms32031);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 11;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 4;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 24;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = -2;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + TMS32031_IRQ0:	info->i = (IREG(TMR_IF) & (1 << TMS32031_IRQ0)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_IRQ1:	info->i = (IREG(TMR_IF) & (1 << TMS32031_IRQ1)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_IRQ2:	info->i = (IREG(TMR_IF) & (1 << TMS32031_IRQ2)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_IRQ3:	info->i = (IREG(TMR_IF) & (1 << TMS32031_IRQ3)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_XINT0:	info->i = (IREG(TMR_IF) & (1 << TMS32031_XINT0)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_RINT0:	info->i = (IREG(TMR_IF) & (1 << TMS32031_RINT0)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_XINT1:	info->i = (IREG(TMR_IF) & (1 << TMS32031_XINT1)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_RINT1:	info->i = (IREG(TMR_IF) & (1 << TMS32031_RINT1)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_TINT0:	info->i = (IREG(TMR_IF) & (1 << TMS32031_TINT0)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_TINT1:	info->i = (IREG(TMR_IF) & (1 << TMS32031_TINT1)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_DINT:	info->i = (IREG(TMR_IF) & (1 << TMS32031_DINT)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_DINT1:	info->i = (IREG(TMR_IF) & (1 << TMS32031_DINT1)) ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					/* not implemented */					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32031_PC:		info->i = tms32031.pc;					break;

		case CPUINFO_INT_REGISTER + TMS32031_R0:		info->i = IREG(TMR_R0);					break;
		case CPUINFO_INT_REGISTER + TMS32031_R1:		info->i = IREG(TMR_R1);					break;
		case CPUINFO_INT_REGISTER + TMS32031_R2:		info->i = IREG(TMR_R2);					break;
		case CPUINFO_INT_REGISTER + TMS32031_R3:		info->i = IREG(TMR_R3);					break;
		case CPUINFO_INT_REGISTER + TMS32031_R4:		info->i = IREG(TMR_R4);					break;
		case CPUINFO_INT_REGISTER + TMS32031_R5:		info->i = IREG(TMR_R5);					break;
		case CPUINFO_INT_REGISTER + TMS32031_R6:		info->i = IREG(TMR_R6);					break;
		case CPUINFO_INT_REGISTER + TMS32031_R7:		info->i = IREG(TMR_R7);					break;
		case CPUINFO_INT_REGISTER + TMS32031_R0F:		ftemp = dsp_to_double(&tms32031.r[TMR_R0]); info->i = f2u(ftemp); break;
		case CPUINFO_INT_REGISTER + TMS32031_R1F:		ftemp = dsp_to_double(&tms32031.r[TMR_R1]); info->i = f2u(ftemp); break;
		case CPUINFO_INT_REGISTER + TMS32031_R2F:		ftemp = dsp_to_double(&tms32031.r[TMR_R2]); info->i = f2u(ftemp); break;
		case CPUINFO_INT_REGISTER + TMS32031_R3F:		ftemp = dsp_to_double(&tms32031.r[TMR_R3]); info->i = f2u(ftemp); break;
		case CPUINFO_INT_REGISTER + TMS32031_R4F:		ftemp = dsp_to_double(&tms32031.r[TMR_R4]); info->i = f2u(ftemp); break;
		case CPUINFO_INT_REGISTER + TMS32031_R5F:		ftemp = dsp_to_double(&tms32031.r[TMR_R5]); info->i = f2u(ftemp); break;
		case CPUINFO_INT_REGISTER + TMS32031_R6F:		ftemp = dsp_to_double(&tms32031.r[TMR_R6]); info->i = f2u(ftemp); break;
		case CPUINFO_INT_REGISTER + TMS32031_R7F:		ftemp = dsp_to_double(&tms32031.r[TMR_R7]); info->i = f2u(ftemp); break;
		case CPUINFO_INT_REGISTER + TMS32031_AR0:		info->i = IREG(TMR_AR0);				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR1:		info->i = IREG(TMR_AR1);				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR2:		info->i = IREG(TMR_AR2);				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR3:		info->i = IREG(TMR_AR3);				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR4:		info->i = IREG(TMR_AR4);				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR5:		info->i = IREG(TMR_AR5);				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR6:		info->i = IREG(TMR_AR6);				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR7:		info->i = IREG(TMR_AR7);				break;
		case CPUINFO_INT_REGISTER + TMS32031_DP:		info->i = IREG(TMR_DP);					break;
		case CPUINFO_INT_REGISTER + TMS32031_IR0:		info->i = IREG(TMR_IR0);				break;
		case CPUINFO_INT_REGISTER + TMS32031_IR1:		info->i = IREG(TMR_IR1);				break;
		case CPUINFO_INT_REGISTER + TMS32031_BK:		info->i = IREG(TMR_BK);					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + TMS32031_SP:		info->i = IREG(TMR_SP);					break;
		case CPUINFO_INT_REGISTER + TMS32031_ST:		info->i = IREG(TMR_ST);					break;
		case CPUINFO_INT_REGISTER + TMS32031_IE:		info->i = IREG(TMR_IE);					break;
		case CPUINFO_INT_REGISTER + TMS32031_IF:		info->i = IREG(TMR_IF);					break;
		case CPUINFO_INT_REGISTER + TMS32031_IOF:		info->i = IREG(TMR_IOF);				break;
		case CPUINFO_INT_REGISTER + TMS32031_RS:		info->i = IREG(TMR_RS);					break;
		case CPUINFO_INT_REGISTER + TMS32031_RE:		info->i = IREG(TMR_RE);					break;
		case CPUINFO_INT_REGISTER + TMS32031_RC:		info->i = IREG(TMR_RC);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = tms32031_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = tms32031_get_context; break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = tms32031_set_context; break;
		case CPUINFO_PTR_INIT:							info->init = tms32031_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = tms32031_reset;			break;
		case CPUINFO_PTR_EXIT:							info->exit = tms32031_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = tms32031_execute;		break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = tms32031_dasm;		break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &tms32031_icount;		break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: info->internal_map = construct_map_internal_32031; break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "TMS32031");			break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Texas Instruments TMS32031"); break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (C) Aaron Giles 2002"); break;

		case CPUINFO_STR_FLAGS:
		{
			UINT32 temp = tms32031.r[TMR_ST].i32[0];
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				(temp & 0x80) ? 'O':'.',
				(temp & 0x40) ? 'U':'.',
                (temp & 0x20) ? 'V':'.',
                (temp & 0x10) ? 'u':'.',
                (temp & 0x08) ? 'n':'.',
                (temp & 0x04) ? 'z':'.',
                (temp & 0x02) ? 'v':'.',
                (temp & 0x01) ? 'c':'.');
            break;
        }

		case CPUINFO_STR_REGISTER + TMS32031_PC:  		sprintf(info->s, "PC: %08X", tms32031.pc); break;

		case CPUINFO_STR_REGISTER + TMS32031_R0:		sprintf(info->s, " R0:%08X", tms32031.r[TMR_R0].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_R1:		sprintf(info->s, " R1:%08X", tms32031.r[TMR_R1].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_R2:		sprintf(info->s, " R2:%08X", tms32031.r[TMR_R2].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_R3:		sprintf(info->s, " R3:%08X", tms32031.r[TMR_R3].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_R4:		sprintf(info->s, " R4:%08X", tms32031.r[TMR_R4].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_R5:		sprintf(info->s, " R5:%08X", tms32031.r[TMR_R5].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_R6:		sprintf(info->s, " R6:%08X", tms32031.r[TMR_R6].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_R7:		sprintf(info->s, " R7:%08X", tms32031.r[TMR_R7].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_R0F:		sprintf(info->s, "R0F:!%12g", dsp_to_double(&tms32031.r[TMR_R0])); break;
		case CPUINFO_STR_REGISTER + TMS32031_R1F:		sprintf(info->s, "R1F:!%12g", dsp_to_double(&tms32031.r[TMR_R1])); break;
		case CPUINFO_STR_REGISTER + TMS32031_R2F:		sprintf(info->s, "R2F:!%12g", dsp_to_double(&tms32031.r[TMR_R2])); break;
		case CPUINFO_STR_REGISTER + TMS32031_R3F:		sprintf(info->s, "R3F:!%12g", dsp_to_double(&tms32031.r[TMR_R3])); break;
		case CPUINFO_STR_REGISTER + TMS32031_R4F:		sprintf(info->s, "R4F:!%12g", dsp_to_double(&tms32031.r[TMR_R4])); break;
		case CPUINFO_STR_REGISTER + TMS32031_R5F:		sprintf(info->s, "R5F:!%12g", dsp_to_double(&tms32031.r[TMR_R5])); break;
		case CPUINFO_STR_REGISTER + TMS32031_R6F:		sprintf(info->s, "R6F:!%12g", dsp_to_double(&tms32031.r[TMR_R6])); break;
		case CPUINFO_STR_REGISTER + TMS32031_R7F:		sprintf(info->s, "R7F:!%12g", dsp_to_double(&tms32031.r[TMR_R7])); break;
		case CPUINFO_STR_REGISTER + TMS32031_AR0:		sprintf(info->s, "AR0:%08X", tms32031.r[TMR_AR0].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_AR1:		sprintf(info->s, "AR1:%08X", tms32031.r[TMR_AR1].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_AR2:		sprintf(info->s, "AR2:%08X", tms32031.r[TMR_AR2].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_AR3:		sprintf(info->s, "AR3:%08X", tms32031.r[TMR_AR3].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_AR4:		sprintf(info->s, "AR4:%08X", tms32031.r[TMR_AR4].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_AR5:		sprintf(info->s, "AR5:%08X", tms32031.r[TMR_AR5].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_AR6:		sprintf(info->s, "AR6:%08X", tms32031.r[TMR_AR6].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_AR7:		sprintf(info->s, "AR7:%08X", tms32031.r[TMR_AR7].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_DP:		sprintf(info->s, " DP:%02X", tms32031.r[TMR_DP].i8[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_IR0:		sprintf(info->s, "IR0:%08X", tms32031.r[TMR_IR0].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_IR1:		sprintf(info->s, "IR1:%08X", tms32031.r[TMR_IR1].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_BK:		sprintf(info->s, " BK:%08X", tms32031.r[TMR_BK].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_SP:		sprintf(info->s, " SP:%08X", tms32031.r[TMR_SP].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_ST:		sprintf(info->s, " ST:%08X", tms32031.r[TMR_ST].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_IE:		sprintf(info->s, " IE:%08X", tms32031.r[TMR_IE].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_IF:		sprintf(info->s, " IF:%08X", tms32031.r[TMR_IF].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_IOF:		sprintf(info->s, "IOF:%08X", tms32031.r[TMR_IOF].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_RS:		sprintf(info->s, " RS:%08X", tms32031.r[TMR_RS].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_RE:		sprintf(info->s, " RE:%08X", tms32031.r[TMR_RE].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_RC:		sprintf(info->s, " RC:%08X", tms32031.r[TMR_RC].i32[0]); break;
	}
}

void tms32032_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_RESET:							info->reset = tms32032_reset;			break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: info->internal_map = construct_map_internal_32032; break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "TMS32032");			break;

		default:										tms32031_get_info(state, info);			break;
	}
}
