/***************************************************************************

    tms->c
    Core implementation for the portable TMS32C031 emulator.
    Written by Aaron Giles

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "tms32031.h"

CPU_DISASSEMBLE( tms32031 );

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

#define IREG(T,rnum)	((T)->r[rnum].i32[0])



/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

typedef union _int_double int_double;
union _int_double
{
	double d;
	float f[2];
	UINT32 i[2];
};

typedef union _tmsreg tmsreg;
union _tmsreg
{
	UINT32		i32[2];
	UINT16		i16[4];
	UINT8		i8[8];
};

/* TMS34031 Registers */
typedef struct _tms32031_state tms32031_state;
struct _tms32031_state
{
	/* core registers */
	UINT32				pc;
	tmsreg				r[36];
	UINT32				bkmask;

	/* internal stuff */
	UINT16				irq_state;
	UINT8				delayed;
	UINT8				irq_pending;
	UINT8				mcu_mode;
	UINT8				is_32032;
	UINT8				is_idling;
	int					icount;

	UINT32				bootoffset;
	tms32031_xf_func	xf0_w;
	tms32031_xf_func	xf1_w;
	tms32031_iack_func	iack_w;
	cpu_irq_callback	irq_callback;
	running_device *device;
	const address_space *program;
};

INLINE tms32031_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == CPU);
	assert(cpu_get_type(device) == CPU_TMS32031 ||
		   cpu_get_type(device) == CPU_TMS32032);
	return (tms32031_state *)device->token;
}



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void trap(tms32031_state *tms, int trapnum);
static UINT32 boot_loader(tms32031_state *tms, UINT32 boot_rom_addr);



/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

#define ROPCODE(T,pc)		memory_decrypted_read_dword((T)->program, (pc) << 2)

#define RMEM(T,addr)		memory_read_dword_32le((T)->program, (addr) << 2)
#define WMEM(T,addr,data)	memory_write_dword_32le((T)->program, (addr) << 2, data)




/***************************************************************************
    HELPER MACROS
***************************************************************************/

#define MANTISSA(r)			((INT32)(r)->i32[0])
#define EXPONENT(r)			((INT8)(r)->i32[1])
#define SET_MANTISSA(r,v)	((r)->i32[0] = (v))
#define SET_EXPONENT(r,v)	((r)->i32[1] = (v))


/***************************************************************************
    CONVERSION FUNCTIONS
***************************************************************************/

static float dsp_to_float(tmsreg *fp)
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


static double dsp_to_double(tmsreg *fp)
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


static void double_to_dsp(double val, tmsreg *result)
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
	tmsreg gen;

	SET_MANTISSA(&gen, floatdata << 8);
	SET_EXPONENT(&gen, (INT32)floatdata >> 24);

	return dsp_to_float(&gen);
}


double convert_tms3203x_fp_to_double(UINT32 floatdata)
{
	tmsreg gen;

	SET_MANTISSA(&gen, floatdata << 8);
	SET_EXPONENT(&gen, (INT32)floatdata >> 24);

	return dsp_to_double(&gen);
}


UINT32 convert_float_to_tms3203x_fp(float fval)
{
	tmsreg gen;

	double_to_dsp(fval, &gen);
	return (EXPONENT(&gen) << 24) | ((UINT32)MANTISSA(&gen) >> 8);
}


UINT32 convert_double_to_tms3203x_fp(double dval)
{
	tmsreg gen;

	double_to_dsp(dval, &gen);
	return (EXPONENT(&gen) << 24) | ((UINT32)MANTISSA(&gen) >> 8);
}



/***************************************************************************
    IRQ HANDLING
***************************************************************************/

static void check_irqs(tms32031_state *tms)
{
	int whichtrap = 0;
	UINT16 validints;
	int i;

	/* determine if we have any live interrupts */
	validints = IREG(tms, TMR_IF) & IREG(tms, TMR_IE) & 0x0fff;
	if (validints == 0 || (IREG(tms, TMR_ST) & GIEFLAG) == 0)
		return;

	/* find the lowest signalled value */
	for (i = 0; i < 12; i++)
		if (validints & (1 << i))
		{
			whichtrap = i + 1;
			break;
		}

	/* no longer idling if we get here */
	tms->is_idling = FALSE;
	if (!tms->delayed)
	{
		UINT16 intmask = 1 << (whichtrap - 1);

		/* bit in IF is cleared when interrupt is taken */
		IREG(tms, TMR_IF) &= ~intmask;
		trap(tms, whichtrap);

		/* after auto-clearing the interrupt bit, we need to re-trigger
           level-sensitive interrupts */
		if (!tms->is_32032 || (IREG(tms, TMR_ST) & 0x4000) == 0)
			IREG(tms, TMR_IF) |= tms->irq_state & 0x0f;
	}
	else
		tms->irq_pending = TRUE;
}


static void set_irq_line(tms32031_state *tms, int irqline, int state)
{
	UINT16 intmask = 1 << irqline;

	/* ignore anything out of range */
	if (irqline >= 12)
		return;

	/* update the external state */
    if (state == ASSERT_LINE)
    {
		tms->irq_state |= intmask;
	    IREG(tms, TMR_IF) |= intmask;
	}
	else
		tms->irq_state &= ~intmask;

	/* external interrupts are level-sensitive on the '31 and can be
       configured as such on the '32; in that case, if the external
       signal is high, we need to update the value in IF accordingly */
	if (!tms->is_32032 || (IREG(tms, TMR_ST) & 0x4000) == 0)
		IREG(tms, TMR_IF) |= tms->irq_state & 0x0f;
}



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

static CPU_INIT( tms32031 )
{
	const tms32031_config *configdata = (const tms32031_config *)device->baseconfig().static_config;
	tms32031_state *tms = get_safe_token(device);
	int i;

	tms->irq_callback = irqcallback;
	tms->device = device;
	tms->program = device->space(AS_PROGRAM);

	/* copy in the xf write routines */
	tms->bootoffset = (configdata != NULL) ? configdata->bootoffset : 0;
	if (configdata != NULL)
	{
		tms->xf0_w = configdata->xf0_w;
		tms->xf1_w = configdata->xf1_w;
		tms->iack_w = configdata->iack_w;
	}

	state_save_register_device_item(device, 0, tms->pc);
	for (i = 0; i < 36; i++)
		state_save_register_generic(device->machine, "tms32031", device->tag, i, "reg", tms->r[i].i8, UINT8, 8);
	state_save_register_device_item(device, 0, tms->bkmask);
	state_save_register_device_item(device, 0, tms->irq_state);
	state_save_register_device_item(device, 0, tms->delayed);
	state_save_register_device_item(device, 0, tms->irq_pending);
	state_save_register_device_item(device, 0, tms->mcu_mode);
	state_save_register_device_item(device, 0, tms->is_idling);
}

static CPU_RESET( tms32031 )
{
	tms32031_state *tms = get_safe_token(device);

	/* if we have a config struct, get the boot ROM address */
	if (tms->bootoffset)
	{
		tms->mcu_mode = TRUE;
		tms->pc = boot_loader(tms, tms->bootoffset);
	}
	else
	{
		tms->mcu_mode = FALSE;
		tms->pc = RMEM(tms, 0);
	}
	tms->is_32032 = FALSE;

	/* reset some registers */
	IREG(tms, TMR_IE) = 0;
	IREG(tms, TMR_IF) = 0;
	IREG(tms, TMR_ST) = 0;
	IREG(tms, TMR_IOF) = 0;

	/* reset internal stuff */
	tms->delayed = tms->irq_pending = FALSE;
	tms->is_idling = FALSE;
}

static CPU_RESET( tms32032 )
{
	tms32031_state *tms = get_safe_token(device);
	CPU_RESET_CALL(tms32031);
	tms->is_32032 = TRUE;
}


#if (LOG_OPCODE_USAGE)
static UINT32 hits[0x200*4];
#endif

static CPU_EXIT( tms32031 )
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

static CPU_EXECUTE( tms32031 )
{
	tms32031_state *tms = get_safe_token(device);

	/* check IRQs up front */
	tms->icount = cycles;
	check_irqs(tms);

	/* if we're idling, just eat the cycles */
	if (tms->is_idling)
		return tms->icount;

	if ((device->machine->debug_flags & DEBUG_FLAG_ENABLED) == 0)
	{
		while (tms->icount > 0)
		{
			if ((IREG(tms, TMR_ST) & RMFLAG) && tms->pc == IREG(tms, TMR_RE) + 1)
			{
				if ((INT32)--IREG(tms, TMR_RC) >= 0)
					tms->pc = IREG(tms, TMR_RS);
				else
				{
					IREG(tms, TMR_ST) &= ~RMFLAG;
					if (tms->delayed)
					{
						tms->delayed = FALSE;
						if (tms->irq_pending)
						{
							tms->irq_pending = FALSE;
							check_irqs(tms);
						}
					}
				}
				continue;
			}

			execute_one(tms);
		}
	}
	else
	{
		while (tms->icount > 0)
		{
			if (IREG(tms, TMR_SP) & 0xff000000)
				debugger_break(device->machine);
			if ((IREG(tms, TMR_ST) & RMFLAG) && tms->pc == IREG(tms, TMR_RE) + 1)
			{
				if ((INT32)--IREG(tms, TMR_RC) >= 0)
					tms->pc = IREG(tms, TMR_RS);
				else
				{
					IREG(tms, TMR_ST) &= ~RMFLAG;
					if (tms->delayed)
					{
						tms->delayed = FALSE;
						if (tms->irq_pending)
						{
							tms->irq_pending = FALSE;
							check_irqs(tms);
						}
					}
				}
				continue;
			}

			debugger_instruction_hook(device, tms->pc);
			execute_one(tms);
		}
	}

	return cycles - tms->icount;
}



/***************************************************************************
    BOOT LOADER
***************************************************************************/

static UINT32 boot_loader(tms32031_state *tms, UINT32 boot_rom_addr)
{
	UINT32 bits, control, advance;
	UINT32 start_offset = 0;
	UINT32 datamask;
	int first = 1, i;

	/* read the size of the data */
	bits = RMEM(tms, boot_rom_addr);
	if (bits != 8 && bits != 16 && bits != 32)
		return 0;
	datamask = 0xffffffffUL >> (32 - bits);
	advance = 32 / bits;
	boot_rom_addr += advance;

	/* read the control register */
	control = RMEM(tms, boot_rom_addr++) & datamask;
	for (i = 1; i < advance; i++)
		control |= (RMEM(tms, boot_rom_addr++) & datamask) << (bits * i);

	/* now parse the data */
	while (1)
	{
		UINT32 offs, len;

		/* read the length of this section */
		len = RMEM(tms, boot_rom_addr++) & datamask;
		for (i = 1; i < advance; i++)
			len |= (RMEM(tms, boot_rom_addr++) & datamask) << (bits * i);

		/* stop at 0 */
		if (len == 0)
			return start_offset;

		/* read the destination offset of this section */
		offs = RMEM(tms, boot_rom_addr++) & datamask;
		for (i = 1; i < advance; i++)
			offs |= (RMEM(tms, boot_rom_addr++) & datamask) << (bits * i);

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
			data = RMEM(tms, boot_rom_addr++) & datamask;
			for (i = 1; i < advance; i++)
				data |= (RMEM(tms, boot_rom_addr++) & datamask) << (bits * i);

			/* write it out */
			WMEM(tms, offs++, data);
		}
	}
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( tms32031 )
{
	tms32031_state *tms = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + TMS32031_IRQ0:	set_irq_line(tms, TMS32031_IRQ0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_IRQ1:	set_irq_line(tms, TMS32031_IRQ1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_IRQ2:	set_irq_line(tms, TMS32031_IRQ2, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_IRQ3:	set_irq_line(tms, TMS32031_IRQ3, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_XINT0:	set_irq_line(tms, TMS32031_XINT0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_RINT0:	set_irq_line(tms, TMS32031_RINT0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_XINT1:	set_irq_line(tms, TMS32031_XINT1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_RINT1:	set_irq_line(tms, TMS32031_RINT1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_TINT0:	set_irq_line(tms, TMS32031_TINT0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_TINT1:	set_irq_line(tms, TMS32031_TINT1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_DINT:	set_irq_line(tms, TMS32031_DINT, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_DINT1:	set_irq_line(tms, TMS32031_DINT1, info->i);	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32031_PC:		tms->pc = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32031_R0:		IREG(tms, TMR_R0) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_R1:		IREG(tms, TMR_R1) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_R2:		IREG(tms, TMR_R2) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_R3:		IREG(tms, TMR_R3) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_R4:		IREG(tms, TMR_R4) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_R5:		IREG(tms, TMR_R5) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_R6:		IREG(tms, TMR_R6) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_R7:		IREG(tms, TMR_R7) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_R0F:		double_to_dsp(*(float *)&info->i, &tms->r[TMR_R0]); break;
		case CPUINFO_INT_REGISTER + TMS32031_R1F:		double_to_dsp(*(float *)&info->i, &tms->r[TMR_R1]); break;
		case CPUINFO_INT_REGISTER + TMS32031_R2F:		double_to_dsp(*(float *)&info->i, &tms->r[TMR_R2]); break;
		case CPUINFO_INT_REGISTER + TMS32031_R3F:		double_to_dsp(*(float *)&info->i, &tms->r[TMR_R3]); break;
		case CPUINFO_INT_REGISTER + TMS32031_R4F:		double_to_dsp(*(float *)&info->i, &tms->r[TMR_R4]); break;
		case CPUINFO_INT_REGISTER + TMS32031_R5F:		double_to_dsp(*(float *)&info->i, &tms->r[TMR_R5]); break;
		case CPUINFO_INT_REGISTER + TMS32031_R6F:		double_to_dsp(*(float *)&info->i, &tms->r[TMR_R6]); break;
		case CPUINFO_INT_REGISTER + TMS32031_R7F:		double_to_dsp(*(float *)&info->i, &tms->r[TMR_R7]); break;
		case CPUINFO_INT_REGISTER + TMS32031_AR0:		IREG(tms, TMR_AR0) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR1:		IREG(tms, TMR_AR1) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR2:		IREG(tms, TMR_AR2) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR3:		IREG(tms, TMR_AR3) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR4:		IREG(tms, TMR_AR4) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR5:		IREG(tms, TMR_AR5) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR6:		IREG(tms, TMR_AR6) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR7:		IREG(tms, TMR_AR7) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_DP:		IREG(tms, TMR_DP) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_IR0:		IREG(tms, TMR_IR0) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_IR1:		IREG(tms, TMR_IR1) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_BK:		IREG(tms, TMR_BK) = info->i;				break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + TMS32031_SP:		IREG(tms, TMR_SP) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_ST:		IREG(tms, TMR_ST) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_IE:		IREG(tms, TMR_IE) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_IF:		IREG(tms, TMR_IF) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_IOF:		IREG(tms, TMR_IOF) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_RS:		IREG(tms, TMR_RS) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_RE:		IREG(tms, TMR_RE) = info->i;				break;
		case CPUINFO_INT_REGISTER + TMS32031_RC:		IREG(tms, TMR_RC) = info->i;				break;
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

CPU_GET_INFO( tms32031 )
{
	tms32031_state *tms = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;
	float ftemp;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(tms32031_state);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 11;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 4;							break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 24;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = -2;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + TMS32031_IRQ0:	info->i = (IREG(tms, TMR_IF) & (1 << TMS32031_IRQ0)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_IRQ1:	info->i = (IREG(tms, TMR_IF) & (1 << TMS32031_IRQ1)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_IRQ2:	info->i = (IREG(tms, TMR_IF) & (1 << TMS32031_IRQ2)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_IRQ3:	info->i = (IREG(tms, TMR_IF) & (1 << TMS32031_IRQ3)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_XINT0:	info->i = (IREG(tms, TMR_IF) & (1 << TMS32031_XINT0)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_RINT0:	info->i = (IREG(tms, TMR_IF) & (1 << TMS32031_RINT0)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_XINT1:	info->i = (IREG(tms, TMR_IF) & (1 << TMS32031_XINT1)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_RINT1:	info->i = (IREG(tms, TMR_IF) & (1 << TMS32031_RINT1)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_TINT0:	info->i = (IREG(tms, TMR_IF) & (1 << TMS32031_TINT0)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_TINT1:	info->i = (IREG(tms, TMR_IF) & (1 << TMS32031_TINT1)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_DINT:	info->i = (IREG(tms, TMR_IF) & (1 << TMS32031_DINT)) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32031_DINT1:	info->i = (IREG(tms, TMR_IF) & (1 << TMS32031_DINT1)) ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					/* not implemented */					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32031_PC:		info->i = tms->pc;					break;

		case CPUINFO_INT_REGISTER + TMS32031_R0:		info->i = IREG(tms, TMR_R0);					break;
		case CPUINFO_INT_REGISTER + TMS32031_R1:		info->i = IREG(tms, TMR_R1);					break;
		case CPUINFO_INT_REGISTER + TMS32031_R2:		info->i = IREG(tms, TMR_R2);					break;
		case CPUINFO_INT_REGISTER + TMS32031_R3:		info->i = IREG(tms, TMR_R3);					break;
		case CPUINFO_INT_REGISTER + TMS32031_R4:		info->i = IREG(tms, TMR_R4);					break;
		case CPUINFO_INT_REGISTER + TMS32031_R5:		info->i = IREG(tms, TMR_R5);					break;
		case CPUINFO_INT_REGISTER + TMS32031_R6:		info->i = IREG(tms, TMR_R6);					break;
		case CPUINFO_INT_REGISTER + TMS32031_R7:		info->i = IREG(tms, TMR_R7);					break;
		case CPUINFO_INT_REGISTER + TMS32031_R0F:		ftemp = dsp_to_double(&tms->r[TMR_R0]); info->i = f2u(ftemp); break;
		case CPUINFO_INT_REGISTER + TMS32031_R1F:		ftemp = dsp_to_double(&tms->r[TMR_R1]); info->i = f2u(ftemp); break;
		case CPUINFO_INT_REGISTER + TMS32031_R2F:		ftemp = dsp_to_double(&tms->r[TMR_R2]); info->i = f2u(ftemp); break;
		case CPUINFO_INT_REGISTER + TMS32031_R3F:		ftemp = dsp_to_double(&tms->r[TMR_R3]); info->i = f2u(ftemp); break;
		case CPUINFO_INT_REGISTER + TMS32031_R4F:		ftemp = dsp_to_double(&tms->r[TMR_R4]); info->i = f2u(ftemp); break;
		case CPUINFO_INT_REGISTER + TMS32031_R5F:		ftemp = dsp_to_double(&tms->r[TMR_R5]); info->i = f2u(ftemp); break;
		case CPUINFO_INT_REGISTER + TMS32031_R6F:		ftemp = dsp_to_double(&tms->r[TMR_R6]); info->i = f2u(ftemp); break;
		case CPUINFO_INT_REGISTER + TMS32031_R7F:		ftemp = dsp_to_double(&tms->r[TMR_R7]); info->i = f2u(ftemp); break;
		case CPUINFO_INT_REGISTER + TMS32031_AR0:		info->i = IREG(tms, TMR_AR0);				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR1:		info->i = IREG(tms, TMR_AR1);				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR2:		info->i = IREG(tms, TMR_AR2);				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR3:		info->i = IREG(tms, TMR_AR3);				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR4:		info->i = IREG(tms, TMR_AR4);				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR5:		info->i = IREG(tms, TMR_AR5);				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR6:		info->i = IREG(tms, TMR_AR6);				break;
		case CPUINFO_INT_REGISTER + TMS32031_AR7:		info->i = IREG(tms, TMR_AR7);				break;
		case CPUINFO_INT_REGISTER + TMS32031_DP:		info->i = IREG(tms, TMR_DP);					break;
		case CPUINFO_INT_REGISTER + TMS32031_IR0:		info->i = IREG(tms, TMR_IR0);				break;
		case CPUINFO_INT_REGISTER + TMS32031_IR1:		info->i = IREG(tms, TMR_IR1);				break;
		case CPUINFO_INT_REGISTER + TMS32031_BK:		info->i = IREG(tms, TMR_BK);					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + TMS32031_SP:		info->i = IREG(tms, TMR_SP);					break;
		case CPUINFO_INT_REGISTER + TMS32031_ST:		info->i = IREG(tms, TMR_ST);					break;
		case CPUINFO_INT_REGISTER + TMS32031_IE:		info->i = IREG(tms, TMR_IE);					break;
		case CPUINFO_INT_REGISTER + TMS32031_IF:		info->i = IREG(tms, TMR_IF);					break;
		case CPUINFO_INT_REGISTER + TMS32031_IOF:		info->i = IREG(tms, TMR_IOF);				break;
		case CPUINFO_INT_REGISTER + TMS32031_RS:		info->i = IREG(tms, TMR_RS);					break;
		case CPUINFO_INT_REGISTER + TMS32031_RE:		info->i = IREG(tms, TMR_RE);					break;
		case CPUINFO_INT_REGISTER + TMS32031_RC:		info->i = IREG(tms, TMR_RC);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(tms32031);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(tms32031);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(tms32031);			break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(tms32031);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(tms32031);		break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(tms32031);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &tms->icount;		break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: info->internal_map32 = ADDRESS_MAP_NAME(internal_32031); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "TMS32031");			break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Texas Instruments TMS32031"); break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Aaron Giles"); break;

		case CPUINFO_STR_FLAGS:
		{
			UINT32 temp = tms->r[TMR_ST].i32[0];
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

		case CPUINFO_STR_REGISTER + TMS32031_PC:		sprintf(info->s, "PC: %08X", tms->pc); break;

		case CPUINFO_STR_REGISTER + TMS32031_R0:		sprintf(info->s, " R0:%08X", tms->r[TMR_R0].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_R1:		sprintf(info->s, " R1:%08X", tms->r[TMR_R1].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_R2:		sprintf(info->s, " R2:%08X", tms->r[TMR_R2].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_R3:		sprintf(info->s, " R3:%08X", tms->r[TMR_R3].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_R4:		sprintf(info->s, " R4:%08X", tms->r[TMR_R4].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_R5:		sprintf(info->s, " R5:%08X", tms->r[TMR_R5].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_R6:		sprintf(info->s, " R6:%08X", tms->r[TMR_R6].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_R7:		sprintf(info->s, " R7:%08X", tms->r[TMR_R7].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_R0F:		sprintf(info->s, "R0F:!%12g", dsp_to_double(&tms->r[TMR_R0])); break;
		case CPUINFO_STR_REGISTER + TMS32031_R1F:		sprintf(info->s, "R1F:!%12g", dsp_to_double(&tms->r[TMR_R1])); break;
		case CPUINFO_STR_REGISTER + TMS32031_R2F:		sprintf(info->s, "R2F:!%12g", dsp_to_double(&tms->r[TMR_R2])); break;
		case CPUINFO_STR_REGISTER + TMS32031_R3F:		sprintf(info->s, "R3F:!%12g", dsp_to_double(&tms->r[TMR_R3])); break;
		case CPUINFO_STR_REGISTER + TMS32031_R4F:		sprintf(info->s, "R4F:!%12g", dsp_to_double(&tms->r[TMR_R4])); break;
		case CPUINFO_STR_REGISTER + TMS32031_R5F:		sprintf(info->s, "R5F:!%12g", dsp_to_double(&tms->r[TMR_R5])); break;
		case CPUINFO_STR_REGISTER + TMS32031_R6F:		sprintf(info->s, "R6F:!%12g", dsp_to_double(&tms->r[TMR_R6])); break;
		case CPUINFO_STR_REGISTER + TMS32031_R7F:		sprintf(info->s, "R7F:!%12g", dsp_to_double(&tms->r[TMR_R7])); break;
		case CPUINFO_STR_REGISTER + TMS32031_AR0:		sprintf(info->s, "AR0:%08X", tms->r[TMR_AR0].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_AR1:		sprintf(info->s, "AR1:%08X", tms->r[TMR_AR1].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_AR2:		sprintf(info->s, "AR2:%08X", tms->r[TMR_AR2].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_AR3:		sprintf(info->s, "AR3:%08X", tms->r[TMR_AR3].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_AR4:		sprintf(info->s, "AR4:%08X", tms->r[TMR_AR4].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_AR5:		sprintf(info->s, "AR5:%08X", tms->r[TMR_AR5].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_AR6:		sprintf(info->s, "AR6:%08X", tms->r[TMR_AR6].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_AR7:		sprintf(info->s, "AR7:%08X", tms->r[TMR_AR7].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_DP:		sprintf(info->s, " DP:%02X", tms->r[TMR_DP].i8[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_IR0:		sprintf(info->s, "IR0:%08X", tms->r[TMR_IR0].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_IR1:		sprintf(info->s, "IR1:%08X", tms->r[TMR_IR1].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_BK:		sprintf(info->s, " BK:%08X", tms->r[TMR_BK].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_SP:		sprintf(info->s, " SP:%08X", tms->r[TMR_SP].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_ST:		sprintf(info->s, " ST:%08X", tms->r[TMR_ST].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_IE:		sprintf(info->s, " IE:%08X", tms->r[TMR_IE].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_IF:		sprintf(info->s, " IF:%08X", tms->r[TMR_IF].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_IOF:		sprintf(info->s, "IOF:%08X", tms->r[TMR_IOF].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_RS:		sprintf(info->s, " RS:%08X", tms->r[TMR_RS].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_RE:		sprintf(info->s, " RE:%08X", tms->r[TMR_RE].i32[0]); break;
		case CPUINFO_STR_REGISTER + TMS32031_RC:		sprintf(info->s, " RC:%08X", tms->r[TMR_RC].i32[0]); break;
	}
}

CPU_GET_INFO( tms32032 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(tms32032);			break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: info->internal_map32 = ADDRESS_MAP_NAME(internal_32032); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "TMS32032");			break;

		default:										CPU_GET_INFO_CALL(tms32031);			break;
	}
}
