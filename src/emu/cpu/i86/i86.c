/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/
/* 26.March 2000 PeT changed set_irq_line */

#include "emu.h"
#include "debugger.h"

#include "host.h"
#include "i86priv.h"
#include "i86.h"

#include "i86mem.h"

extern int i386_dasm_one(char *buffer, UINT32 eip, const UINT8 *oprom, int mode);

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) mame_printf_debug x; } while (0)


/* All pre-i286 CPUs have a 1MB address space */
#define AMASK	0xfffff


/* I86 registers */
typedef union
{									   /* eight general registers */
	UINT16 w[8];					   /* viewed as 16 bits registers */
	UINT8 b[16];					   /* or as 8 bit registers */
}
i8086basicregs;

typedef struct _i8086_state i8086_state;
struct _i8086_state
{
	i8086basicregs regs;
	UINT32 pc;
	UINT32 prevpc;
	UINT32 base[4];
	UINT16 sregs[4];
	UINT16 flags;
	cpu_irq_callback irq_callback;
	INT32 AuxVal, OverVal, SignVal, ZeroVal, CarryVal, DirVal;		/* 0 or non-0 valued flags */
	UINT8 ParityVal;
	UINT8 TF, IF;				   /* 0 or 1 valued flags */
	UINT8 MF;						   /* V30 mode flag */
	UINT8 int_vector;
	INT8 nmi_state;
	INT8 irq_state;
	INT8 test_state;
	UINT8 rep_in_progress;
	INT32 extra_cycles;       /* extra cycles for interrupts */

	int halted;         /* Is the CPU halted ? */

	UINT16 ip;
	UINT32 sp;

	memory_interface	mem;

	running_device *device;
	const address_space *program;
	const address_space *io;
	int icount;
	cpu_state_table state;

	unsigned prefix_base;		   /* base address of the latest prefix segment */
	char seg_prefix;				   /* prefix segment indicator */
	unsigned ea;
	UINT16 eo; /* HJB 12/13/98 effective offset of the address (before segment is added) */
};

INLINE i8086_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == CPU);
	assert(cpu_get_type(device) == CPU_I8086 ||
		   cpu_get_type(device) == CPU_I8088 ||
		   cpu_get_type(device) == CPU_I80186 ||
		   cpu_get_type(device) == CPU_I80188);
	return (i8086_state *)device->token;
}

/***************************************************************************
    CPU STATE DESCRIPTION
***************************************************************************/

#define I86_STATE_ENTRY(_name, _format, _member, _datamask, _flags) \
	CPU_STATE_ENTRY(I8086_##_name, #_name, _format, i8086_state, _member, _datamask, ~0, _flags)

static const cpu_state_entry state_array[] =
{
	I86_STATE_ENTRY(GENPC, "%9s", pc, 0xfffff, CPUSTATE_IMPORT)
	I86_STATE_ENTRY(GENPCBASE, "%08X", pc, 0xfffff, CPUSTATE_NOSHOW)	/* not implemented */
	I86_STATE_ENTRY(IP,    "%04X", ip, 0xffff, CPUSTATE_IMPORT | CPUSTATE_EXPORT)

	I86_STATE_ENTRY(FLAGS, "%04X", flags, 0xffff, CPUSTATE_NOSHOW | CPUSTATE_IMPORT | CPUSTATE_EXPORT)

	I86_STATE_ENTRY(AX,    "%04X", regs.w[AX], 0xffff, 0)
	I86_STATE_ENTRY(BX,    "%04X", regs.w[BX], 0xffff, 0)
	I86_STATE_ENTRY(CX,    "%04X", regs.w[CX], 0xffff, 0)
	I86_STATE_ENTRY(DX,    "%04X", regs.w[DX], 0xffff, 0)
	I86_STATE_ENTRY(SI,    "%04X", regs.w[SI], 0xffff, 0)
	I86_STATE_ENTRY(DI,    "%04X", regs.w[DI], 0xffff, 0)
	I86_STATE_ENTRY(BP,    "%04X", regs.w[BP], 0xffff, 0)
	I86_STATE_ENTRY(SP,    "%04X", regs.w[SP], 0xffff, 0)
	I86_STATE_ENTRY(GENSP, "%9s", sp, 0xfffff, CPUSTATE_IMPORT | CPUSTATE_EXPORT)

	I86_STATE_ENTRY(AL,    "%02X", regs.b[AL], 0xff, CPUSTATE_NOSHOW)
	I86_STATE_ENTRY(BL,    "%02X", regs.b[BL], 0xff, CPUSTATE_NOSHOW)
	I86_STATE_ENTRY(CL,    "%02X", regs.b[CL], 0xff, CPUSTATE_NOSHOW)
	I86_STATE_ENTRY(DL,    "%02X", regs.b[DL], 0xff, CPUSTATE_NOSHOW)
	I86_STATE_ENTRY(AH,    "%02X", regs.b[AH], 0xff, CPUSTATE_NOSHOW)
	I86_STATE_ENTRY(BH,    "%02X", regs.b[BH], 0xff, CPUSTATE_NOSHOW)
	I86_STATE_ENTRY(CH,    "%02X", regs.b[CH], 0xff, CPUSTATE_NOSHOW)
	I86_STATE_ENTRY(DH,    "%02X", regs.b[DH], 0xff, CPUSTATE_NOSHOW)

	I86_STATE_ENTRY(CS,    "%04X", sregs[CS], 0xffff, CPUSTATE_IMPORT)
	I86_STATE_ENTRY(DS,    "%04X", sregs[DS], 0xffff, CPUSTATE_IMPORT)
	I86_STATE_ENTRY(ES,    "%04X", sregs[ES], 0xffff, CPUSTATE_IMPORT)
	I86_STATE_ENTRY(SS,    "%04X", sregs[SS], 0xffff, CPUSTATE_IMPORT)
};

static const cpu_state_table state_table_template =
{
	NULL,						/* pointer to the base of state (offsets are relative to this) */
	0,							/* subtype this table refers to */
	ARRAY_LENGTH(state_array),	/* number of entries */
	state_array					/* array of entries */
};



#include "i86time.c"

/***************************************************************************/
/* cpu state                                                               */
/***************************************************************************/


static struct i80x86_timing timing;

static UINT8 parity_table[256];

/* The interrupt number of a pending external interrupt pending NMI is 2.   */
/* For INTR interrupts, the level is caught on the bus during an INTA cycle */

#define PREFIX(name) i8086##name
#define PREFIX86(name) i8086##name

#define I8086
#include "instr86.h"
#include "ea.h"
#include "modrm.h"
#include "table86.h"

#include "instr86.c"
#include "i86mem.c"
#undef I8086


/***************************************************************************/
static void i8086_state_register(running_device *device)
{
	i8086_state *cpustate = get_safe_token(device);
	state_save_register_device_item_array(device, 0, cpustate->regs.w);
	state_save_register_device_item(device, 0, cpustate->pc);
	state_save_register_device_item(device, 0, cpustate->prevpc);
	state_save_register_device_item_array(device, 0, cpustate->base);
	state_save_register_device_item_array(device, 0, cpustate->sregs);
	state_save_register_device_item(device, 0, cpustate->flags);
	state_save_register_device_item(device, 0, cpustate->AuxVal);
	state_save_register_device_item(device, 0, cpustate->OverVal);
	state_save_register_device_item(device, 0, cpustate->SignVal);
	state_save_register_device_item(device, 0, cpustate->ZeroVal);
	state_save_register_device_item(device, 0, cpustate->CarryVal);
	state_save_register_device_item(device, 0, cpustate->DirVal);
	state_save_register_device_item(device, 0, cpustate->ParityVal);
	state_save_register_device_item(device, 0, cpustate->TF);
	state_save_register_device_item(device, 0, cpustate->IF);
	state_save_register_device_item(device, 0, cpustate->MF);
	state_save_register_device_item(device, 0, cpustate->int_vector);
	state_save_register_device_item(device, 0, cpustate->nmi_state);
	state_save_register_device_item(device, 0, cpustate->irq_state);
	state_save_register_device_item(device, 0, cpustate->extra_cycles);
	state_save_register_device_item(device, 0, cpustate->halted);
	state_save_register_device_item(device, 0, cpustate->test_state);	/* PJB 03/05 */
	state_save_register_device_item(device, 0, cpustate->rep_in_progress);	/* PJB 03/05 */
}

static CPU_INIT( i8086 )
{
	i8086_state *cpustate = get_safe_token(device);
	unsigned int i, j, c;
	static const BREGS reg_name[8] = {AL, CL, DL, BL, AH, CH, DH, BH};
	for (i = 0; i < 256; i++)
	{
		for (j = i, c = 0; j > 0; j >>= 1)
			if (j & 1)
				c++;

		parity_table[i] = !(c & 1);
	}

	for (i = 0; i < 256; i++)
	{
		Mod_RM.reg.b[i] = reg_name[(i & 0x38) >> 3];
		Mod_RM.reg.w[i] = (WREGS) ((i & 0x38) >> 3);
	}

	for (i = 0xc0; i < 0x100; i++)
	{
		Mod_RM.RM.w[i] = (WREGS) (i & 7);
		Mod_RM.RM.b[i] = (BREGS) reg_name[i & 7];
	}

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->io = device->space(AS_IO);

	/* set up the state table */
	cpustate->state = state_table_template;
	cpustate->state.baseptr = cpustate;
	cpustate->state.subtypemask = 1;

	i8086_state_register(device);
	configure_memory_16bit(cpustate);
}

static CPU_INIT( i8088 )
{
	i8086_state *cpustate = get_safe_token(device);
	CPU_INIT_CALL(i8086);
	configure_memory_8bit(cpustate);
}

static CPU_RESET( i8086 )
{
	i8086_state *cpustate = get_safe_token(device);
	cpu_irq_callback save_irqcallback;
	memory_interface save_mem;
	cpu_state_table save_state;

	save_irqcallback = cpustate->irq_callback;
	save_mem = cpustate->mem;
	save_state = cpustate->state;
	memset(cpustate, 0, sizeof(*cpustate));
	cpustate->irq_callback = save_irqcallback;
	cpustate->mem = save_mem;
	cpustate->state = save_state;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->io = device->space(AS_IO);

	cpustate->sregs[CS] = 0xf000;
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = 0xffff0 & AMASK;
	ExpandFlags(cpustate->flags);

	cpustate->halted = 0;
}

static CPU_EXIT( i8086 )
{
	/* nothing to do ? */
}

/* ASG 971222 -- added these interface functions */

static void set_irq_line(i8086_state *cpustate, int irqline, int state)
{
	if (state != CLEAR_LINE && cpustate->halted)
	{
		cpustate->halted = 0;
	}

	if (irqline == INPUT_LINE_NMI)
	{
		if (cpustate->nmi_state == state)
			return;
		cpustate->nmi_state = state;

		/* on a rising edge, signal the NMI */
		if (state != CLEAR_LINE)
		{
			PREFIX(_interrupt)(cpustate, I8086_NMI_INT_VECTOR);
		}
	}
	else
	{
		cpustate->irq_state = state;

		/* if the IF is set, signal an interrupt */
		if (state != CLEAR_LINE && cpustate->IF)
			PREFIX(_interrupt)(cpustate, (UINT32)-1);
	}
}

/* PJB 03/05 */
static void set_test_line(i8086_state *cpustate, int state)
{
	cpustate->test_state = !state;
}

static CPU_EXECUTE( i8086 )
{
	i8086_state *cpustate = get_safe_token(device);


	if (cpustate->halted)
		return cycles;

	/* copy over the cycle counts if they're not correct */
	if (timing.id != 8086)
		timing = i8086_cycles;

	/* adjust for any interrupts that came in */
	cpustate->icount = cycles;
	cpustate->icount -= cpustate->extra_cycles;
	cpustate->extra_cycles = 0;

	/* run until we're out */
	while (cpustate->icount > 0)
	{
		LOG(("[%04x:%04x]=%02x\tF:%04x\tAX=%04x\tBX=%04x\tCX=%04x\tDX=%04x %d%d%d%d%d%d%d%d%d\n",
				cpustate->sregs[CS], cpustate->pc - cpustate->base[CS], ReadByte(cpustate->pc), cpustate->flags, cpustate->regs.w[AX], cpustate->regs.w[BX], cpustate->regs.w[CX], cpustate->regs.w[DX], cpustate->AuxVal ? 1 : 0, cpustate->OverVal ? 1 : 0,
				cpustate->SignVal ? 1 : 0, cpustate->ZeroVal ? 1 : 0, cpustate->CarryVal ? 1 : 0, cpustate->ParityVal ? 1 : 0, cpustate->TF, cpustate->IF, cpustate->DirVal < 0 ? 1 : 0));
		debugger_instruction_hook(device, cpustate->pc);

		cpustate->seg_prefix = FALSE;
		cpustate->prevpc = cpustate->pc;
		TABLE86;
	}

	/* adjust for any interrupts that came in */
	cpustate->icount -= cpustate->extra_cycles;
	cpustate->extra_cycles = 0;

	return cycles - cpustate->icount;
}


static CPU_DISASSEMBLE( i8086 )
{
	return i386_dasm_one(buffer, pc, oprom, 16);
}



#include "i86.h"

#undef PREFIX
#define PREFIX(name) i80186##name
#define PREFIX186(name) i80186##name

#define I80186
#include "instr186.h"
#include "table186.h"

#include "instr86.c"
#include "instr186.c"
#undef I80186

static CPU_EXECUTE( i80186 )
{
	i8086_state *cpustate = get_safe_token(device);

	/* copy over the cycle counts if they're not correct */
	if (timing.id != 80186)
		timing = i80186_cycles;

	/* adjust for any interrupts that came in */
	cpustate->icount = cycles;
	cpustate->icount -= cpustate->extra_cycles;
	cpustate->extra_cycles = 0;

	/* run until we're out */
	while (cpustate->icount > 0)
	{
		LOG(("[%04x:%04x]=%02x\tAX=%04x\tBX=%04x\tCX=%04x\tDX=%04x\n", cpustate->sregs[CS], cpustate->pc, ReadByte(cpustate->pc), cpustate->regs.w[AX],
			   cpustate->regs.w[BX], cpustate->regs.w[CX], cpustate->regs.w[DX]));
		debugger_instruction_hook(device, cpustate->pc);

		cpustate->seg_prefix = FALSE;
		cpustate->prevpc = cpustate->pc;
		TABLE186;
	}

	/* adjust for any interrupts that came in */
	cpustate->icount -= cpustate->extra_cycles;
	cpustate->extra_cycles = 0;

	return cycles - cpustate->icount;
}




/**************************************************************************
 * STATE IMPORT/EXPORT
 **************************************************************************/

static CPU_IMPORT_STATE( i8086 )
{
	i8086_state *cpustate = get_safe_token(device);

	switch (entry->index)
	{
		case I8086_GENPC:
			if (cpustate->pc - cpustate->base[CS] >= 0x10000)
			{
				cpustate->base[CS] = cpustate->pc & 0xffff0;
				cpustate->sregs[CS] = cpustate->base[CS] >> 4;
			}
			break;

		case I8086_IP:
			cpustate->pc = cpustate->base[CS] + cpustate->ip;
			break;

		case I8086_GENSP:
			if (cpustate->sp - cpustate->base[SS] >= 0x10000)
			{
				cpustate->base[SS] = cpustate->sp & 0xffff0;
				cpustate->sregs[SS] = cpustate->base[SS] >> 4;
			}
			cpustate->regs.w[SP] = cpustate->sp - cpustate->base[SS];
			break;

		case I8086_FLAGS:
			ExpandFlags(cpustate->flags);
			break;

		case I8086_ES:
			cpustate->base[ES] = SegBase(ES);
			break;

		case I8086_CS:
			cpustate->base[CS] = SegBase(CS);
			break;

		case I8086_SS:
			cpustate->base[SS] = SegBase(SS);
			break;

		case I8086_DS:
			cpustate->base[DS] = SegBase(DS);
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(i8086) called for unexpected value\n");
			break;
	}
}


static CPU_EXPORT_STATE( i8086 )
{
	i8086_state *cpustate = get_safe_token(device);

	switch (entry->index)
	{
		case I8086_IP:
			cpustate->ip = cpustate->pc - cpustate->base[CS];
			break;

		case I8086_FLAGS:
			cpustate->flags = CompressFlags();
			break;

		case I8086_GENSP:
			cpustate->sp = cpustate->base[SS] + cpustate->regs.w[SP];
			break;

		default:
			fatalerror("CPU_EXPORT_STATE(i8086) called for unexpected value\n");
			break;
	}
}


static CPU_EXPORT_STRING( i8086 )
{
	i8086_state *cpustate = get_safe_token(device);

	switch (entry->index)
	{
		case I8086_GENPC:
			sprintf(string, "%04X:%04X", cpustate->sregs[CS] & 0xffff, (cpustate->pc - cpustate->base[CS]) & 0xffff);
			break;

		case I8086_GENSP:
			sprintf(string, "%04X:%04X", cpustate->sregs[SS] & 0xffff, cpustate->regs.w[SP] & 0xffff);
			break;

		default:
			fatalerror("CPU_EXPORT_STRING(i8086) called for unexpected value\n");
			break;
	}
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( i8086 )
{
	i8086_state *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:				set_irq_line(cpustate, 0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(cpustate, INPUT_LINE_NMI, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_TEST:	set_test_line(cpustate, info->i);					break; /* PJB 03/05 */
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( i8086 )
{
	i8086_state *cpustate = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(i8086_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 15;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 50;							break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 20;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = cpustate->irq_state;					break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = cpustate->nmi_state;					break;

		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_TEST:	info->i = cpustate->test_state;					break; /* PJB 03/05 */

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->prevpc;						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(i8086);			break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i8086);					break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(i8086);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(i8086);					break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(i8086);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;									break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i8086);	break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;					break;
		case CPUINFO_PTR_STATE_TABLE:					info->state_table = &cpustate->state;				break;
		case CPUINFO_FCT_IMPORT_STATE:					info->import_state = CPU_IMPORT_STATE_NAME(i8086);	break;
		case CPUINFO_FCT_EXPORT_STATE:					info->export_state = CPU_EXPORT_STATE_NAME(i8086);	break;
		case CPUINFO_FCT_EXPORT_STRING:					info->export_string = CPU_EXPORT_STRING_NAME(i8086);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "8086");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Intel 80x86");			break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.4");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Real mode i286 emulator v1.4 by Fabrice Frances\n(initial work cpustate->based on David Hedley's pcemu)"); break;

		case CPUINFO_STR_FLAGS:
			cpustate->flags = CompressFlags();
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
					cpustate->flags & 0x8000 ? '?' : '.',
					cpustate->flags & 0x4000 ? '?' : '.',
					cpustate->flags & 0x2000 ? '?' : '.',
					cpustate->flags & 0x1000 ? '?' : '.',
					cpustate->flags & 0x0800 ? 'O' : '.',
					cpustate->flags & 0x0400 ? 'D' : '.',
					cpustate->flags & 0x0200 ? 'I' : '.',
					cpustate->flags & 0x0100 ? 'T' : '.',
					cpustate->flags & 0x0080 ? 'S' : '.',
					cpustate->flags & 0x0040 ? 'Z' : '.',
					cpustate->flags & 0x0020 ? '?' : '.',
					cpustate->flags & 0x0010 ? 'A' : '.',
					cpustate->flags & 0x0008 ? '?' : '.',
					cpustate->flags & 0x0004 ? 'P' : '.',
					cpustate->flags & 0x0002 ? 'N' : '.',
					cpustate->flags & 0x0001 ? 'C' : '.');
			break;
	}
}


/**************************************************************************
 * CPU-specific get_info/set_info
 **************************************************************************/

CPU_GET_INFO( i8088 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i8088);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "8088");				break;

		default:										CPU_GET_INFO_CALL(i8086);				break;
	}
}


/**************************************************************************
 * CPU-specific get_info/set_info
 **************************************************************************/

CPU_GET_INFO( i80186 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 2;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(i80186);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "80186");				break;

		default:										CPU_GET_INFO_CALL(i8086);				break;
	}
}


/**************************************************************************
 * CPU-specific get_info/set_info
 **************************************************************************/

CPU_GET_INFO( i80188 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i8088);		break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(i80186);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "80188");				break;

		default:										CPU_GET_INFO_CALL(i8086);				break;
	}
}
