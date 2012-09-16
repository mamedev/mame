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

extern int i386_dasm_one(char *buffer, UINT32 eip, const UINT8 *oprom, int mode);

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) mame_printf_debug x; } while (0)


/* All pre-i286 CPUs have a 1MB address space */
#define AMASK	0xfffff


/* I86 registers */
union i8086basicregs
{									   /* eight general registers */
	UINT16 w[8];					   /* viewed as 16 bits registers */
	UINT8 b[16];					   /* or as 8 bit registers */
};

struct i8086_state
{
	i8086basicregs regs;
	offs_t fetch_xor;
	UINT32 pc;
	UINT32 prevpc;
	UINT32 base[4];
	UINT16 sregs[4];
	UINT16 flags;
	device_irq_acknowledge_callback irq_callback;
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

	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *io;
	int icount;

	char seg_prefix;				   /* prefix segment indicator */
	UINT8	prefix_seg;					/* The prefixed segment */
	unsigned ea;
	UINT16 eo; /* HJB 12/13/98 effective offset of the address (before segment is added) */
	UINT8 ea_seg;	/* effective segment of the address */

	devcb_resolved_write_line	out_tmrout0_func;
	devcb_resolved_write_line	out_tmrout1_func;
};

INLINE i8086_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == I8086 ||
		   device->type() == I8088 ||
		   device->type() == I80186 ||
		   device->type() == I80188);
	return (i8086_state *)downcast<legacy_cpu_device *>(device)->token();
}


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
#undef I8086


/***************************************************************************/
static void i8086_state_register(device_t *device)
{
	i8086_state *cpustate = get_safe_token(device);
	device->save_item(NAME(cpustate->regs.w));
	device->save_item(NAME(cpustate->pc));
	device->save_item(NAME(cpustate->prevpc));
	device->save_item(NAME(cpustate->base));
	device->save_item(NAME(cpustate->sregs));
	device->save_item(NAME(cpustate->flags));
	device->save_item(NAME(cpustate->AuxVal));
	device->save_item(NAME(cpustate->OverVal));
	device->save_item(NAME(cpustate->SignVal));
	device->save_item(NAME(cpustate->ZeroVal));
	device->save_item(NAME(cpustate->CarryVal));
	device->save_item(NAME(cpustate->DirVal));
	device->save_item(NAME(cpustate->ParityVal));
	device->save_item(NAME(cpustate->TF));
	device->save_item(NAME(cpustate->IF));
	device->save_item(NAME(cpustate->MF));
	device->save_item(NAME(cpustate->int_vector));
	device->save_item(NAME(cpustate->nmi_state));
	device->save_item(NAME(cpustate->irq_state));
	device->save_item(NAME(cpustate->extra_cycles));
	device->save_item(NAME(cpustate->halted));
	device->save_item(NAME(cpustate->test_state));	/* PJB 03/05 */
	device->save_item(NAME(cpustate->rep_in_progress));	/* PJB 03/05 */
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
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = device->space(AS_IO);

	/* set up the state table */
	{
		device_state_interface *state;
		device->interface(state);
		state->state_add(STATE_GENPC, "GENPC", cpustate->pc).mask(0xfffff).formatstr("%9s").callimport();
		state->state_add(I8086_IP,    "IP",    cpustate->ip).callimport().callexport();
		state->state_add(I8086_FLAGS, "FLAGS", cpustate->flags).callimport().callexport().noshow();
		state->state_add(STATE_GENFLAGS, "GENFLAGS", cpustate->flags).callimport().callexport().noshow().formatstr("%16s");
		state->state_add(I8086_AX,    "AX",    cpustate->regs.w[AX]);
		state->state_add(I8086_BX,    "BX",    cpustate->regs.w[BX]);
		state->state_add(I8086_CX,    "CX",    cpustate->regs.w[CX]);
		state->state_add(I8086_DX,    "DX",    cpustate->regs.w[DX]);
		state->state_add(I8086_SI,    "SI",    cpustate->regs.w[SI]);
		state->state_add(I8086_DI,    "DI",    cpustate->regs.w[DI]);
		state->state_add(I8086_BP,    "BP",    cpustate->regs.w[BP]);
		state->state_add(I8086_SP,    "SP",    cpustate->regs.w[SP]);
		state->state_add(STATE_GENSP, "GENSP", cpustate->sp).mask(0xfffff).formatstr("%9s").callimport().callexport();
		state->state_add(I8086_AL,    "AL",    cpustate->regs.b[AL]).noshow();
		state->state_add(I8086_BL,    "BL",    cpustate->regs.b[BL]).noshow();
		state->state_add(I8086_CL,    "CL",    cpustate->regs.b[CL]).noshow();
		state->state_add(I8086_DL,    "DL",    cpustate->regs.b[DL]).noshow();
		state->state_add(I8086_AH,    "AH",    cpustate->regs.b[AH]).noshow();
		state->state_add(I8086_BH,    "BH",    cpustate->regs.b[BH]).noshow();
		state->state_add(I8086_CH,    "CH",    cpustate->regs.b[CH]).noshow();
		state->state_add(I8086_DH,    "DH",    cpustate->regs.b[DH]).noshow();
		state->state_add(I8086_CS,    "CS",    cpustate->sregs[CS]).callimport();
		state->state_add(I8086_DS,    "DS",    cpustate->sregs[DS]).callimport();
		state->state_add(I8086_ES,    "ES",    cpustate->sregs[ES]).callimport();
		state->state_add(I8086_SS,    "SS",    cpustate->sregs[SS]).callimport();
	}

	i8086_state_register(device);
	cpustate->fetch_xor = BYTE_XOR_LE(0);
}

static CPU_INIT( i8088 )
{
	i8086_state *cpustate = get_safe_token(device);
	CPU_INIT_CALL(i8086);
	cpustate->fetch_xor = 0;
}

static CPU_INIT( i80186 )
{
	i8086_state *cpustate = get_safe_token(device);

	CPU_INIT_CALL(i8086);

	/* resolve callbacks */
	i80186_interface *intf = (i80186_interface *) device->static_config();

	if (intf != NULL)
	{
		cpustate->out_tmrout0_func.resolve(intf->out_tmrout0_func, *device);
		cpustate->out_tmrout1_func.resolve(intf->out_tmrout1_func, *device);
	}
}

static CPU_RESET( i8086 )
{
	i8086_state *cpustate = get_safe_token(device);
	device_irq_acknowledge_callback save_irqcallback;

	save_irqcallback = cpustate->irq_callback;
	memset(cpustate, 0, sizeof(*cpustate));
	cpustate->irq_callback = save_irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = device->space(AS_IO);

	cpustate->sregs[CS] = 0xffff;
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

static void set_drq_line(i8086_state *cpustate, int irqline, int state)
{
	// TODO implement me
}

static void set_tmrin_line(i8086_state *cpustate, int irqline, int state)
{
	// TODO implement me
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
	{
		cpustate->icount = 0;
		return;
	}

	/* copy over the cycle counts if they're not correct */
	if (timing.id != 8086)
		timing = i8086_cycles;

	/* adjust for any interrupts that came in */
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
}


static CPU_DISASSEMBLE( i8086 )
{
	return i386_dasm_one(buffer, pc, oprom, 1);
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
}




/**************************************************************************
 * STATE IMPORT/EXPORT
 **************************************************************************/

static CPU_IMPORT_STATE( i8086 )
{
	i8086_state *cpustate = get_safe_token(device);

	switch (entry.index())
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
		case STATE_GENFLAGS:
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

	switch (entry.index())
	{
		case I8086_IP:
			cpustate->ip = cpustate->pc - cpustate->base[CS];
			break;

		case I8086_FLAGS:
		case STATE_GENFLAGS:
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

	switch (entry.index())
	{
		case I8086_GENPC:
			string.printf("%04X:%04X", cpustate->sregs[CS] & 0xffff, (cpustate->pc - cpustate->base[CS]) & 0xffff);
			break;

		case I8086_GENSP:
			string.printf("%04X:%04X", cpustate->sregs[SS] & 0xffff, cpustate->regs.w[SP] & 0xffff);
			break;

		case STATE_GENFLAGS:
			cpustate->flags = CompressFlags();
			string.printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
					cpustate->flags & 0x8000 ? '1' : '.',
					cpustate->flags & 0x4000 ? '1' : '.',
					cpustate->flags & 0x2000 ? '1' : '.',
					cpustate->flags & 0x1000 ? '1' : '.',
					cpustate->flags & 0x0800 ? 'O' : '.',
					cpustate->flags & 0x0400 ? 'D' : '.',
					cpustate->flags & 0x0200 ? 'I' : '.',
					cpustate->flags & 0x0100 ? 'T' : '.',
					cpustate->flags & 0x0080 ? 'S' : '.',
					cpustate->flags & 0x0040 ? 'Z' : '.',
					cpustate->flags & 0x0020 ? '0' : '.',
					cpustate->flags & 0x0010 ? 'A' : '.',
					cpustate->flags & 0x0008 ? '0' : '.',
					cpustate->flags & 0x0004 ? 'P' : '.',
					cpustate->flags & 0x0002 ? '1' : '.',
					cpustate->flags & 0x0001 ? 'C' : '.');
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
		case CPUINFO_INT_INPUT_STATE + 0:				set_irq_line(cpustate, 0, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(cpustate, INPUT_LINE_NMI, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_TEST:	set_test_line(cpustate, info->i);					break; /* PJB 03/05 */
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( i8086 )
{
	i8086_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(i8086_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 8;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 50;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 20;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

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
		case CPUINFO_FCT_IMPORT_STATE:					info->import_state = CPU_IMPORT_STATE_NAME(i8086);	break;
		case CPUINFO_FCT_EXPORT_STATE:					info->export_state = CPU_EXPORT_STATE_NAME(i8086);	break;
		case CPUINFO_FCT_EXPORT_STRING:					info->export_string = CPU_EXPORT_STRING_NAME(i8086);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "8086");				break;
		case CPUINFO_STR_FAMILY:						strcpy(info->s, "Intel 80x86");			break;
		case CPUINFO_STR_VERSION:						strcpy(info->s, "1.4");					break;
		case CPUINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:						strcpy(info->s, "Real mode i286 emulator v1.4 by Fabrice Frances\n(initial work cpustate->based on David Hedley's pcemu)"); break;
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
		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i8088);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "8088");				break;

		default:										CPU_GET_INFO_CALL(i8086);				break;
	}
}


/**************************************************************************
 * CPU-specific get_info/set_info
 **************************************************************************/

static CPU_SET_INFO( i80186 )
{
	i8086_state *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_INT0:		set_irq_line(cpustate, 0, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_INT1:		set_irq_line(cpustate, 1, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_INT2:		set_irq_line(cpustate, 2, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_INT3:		set_irq_line(cpustate, 3, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_DRQ0:		set_drq_line(cpustate, 0, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_DRQ1:		set_drq_line(cpustate, 1, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_TMRIN0:	set_tmrin_line(cpustate, 0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_TMRIN1:	set_tmrin_line(cpustate, 1, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		set_irq_line(cpustate, INPUT_LINE_NMI, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_TEST:		set_test_line(cpustate, info->i);					break; /* PJB 03/05 */
	}
}

CPU_GET_INFO( i80186 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;								break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 2;								break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(i80186);	break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i80186);			break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(i80186);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "80186");					break;

		default:										CPU_GET_INFO_CALL(i8086);					break;
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
		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i8088);		break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(i80186);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "80188");				break;

		default:										CPU_GET_INFO_CALL(i8086);				break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(I8086, i8086);
DEFINE_LEGACY_CPU_DEVICE(I8088, i8088);
DEFINE_LEGACY_CPU_DEVICE(I80186, i80186);
DEFINE_LEGACY_CPU_DEVICE(I80188, i80188);
