/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/
/* 26.March 2000 PeT changed set_irq_line */

#include "debugger.h"
#include "cpuintrf.h"

#include "host.h"
#include "i86.h"
#include "i86intf.h"

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
	INT8 test_state;	/* PJB 03/05 */
	INT32 extra_cycles;       /* extra cycles for interrupts */

	memory_interface	mem;

	const device_config *device;
	const address_space *program;
	const address_space *io;
	int icount;

	unsigned prefix_base;		   /* base address of the latest prefix segment */
	char seg_prefix;				   /* prefix segment indicator */
	unsigned ea;
	UINT16 eo; /* HJB 12/13/98 effective offset of the address (before segment is added) */
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
static void i8086_state_register(const device_config *device)
{
	i8086_state *cpustate = device->token;
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
	state_save_register_device_item(device, 0, cpustate->test_state);	/* PJB 03/05 */
}

static CPU_INIT( i8086 )
{
	i8086_state *cpustate = device->token;
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
	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cpustate->io = memory_find_address_space(device, ADDRESS_SPACE_IO);

	i8086_state_register(device);
	configure_memory_16bit(cpustate);
}

#if (HAS_I8088||HAS_I80188)
static CPU_INIT( i8088 )
{
	i8086_state *cpustate = device->token;
	CPU_INIT_CALL(i8086);
	configure_memory_8bit(cpustate);
}
#endif

static CPU_RESET( i8086 )
{
	i8086_state *cpustate = device->token;
	cpu_irq_callback save_irqcallback;
    memory_interface save_mem;

	save_irqcallback = cpustate->irq_callback;
	save_mem = cpustate->mem;
	memset(cpustate, 0, sizeof(*cpustate));
	cpustate->irq_callback = save_irqcallback;
	cpustate->mem = save_mem;
	cpustate->device = device;
	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cpustate->io = memory_find_address_space(device, ADDRESS_SPACE_IO);

	cpustate->sregs[CS] = 0xf000;
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = 0xffff0 & AMASK;
	ExpandFlags(cpustate->flags);
}

static CPU_EXIT( i8086 )
{
	/* nothing to do ? */
}

/* ASG 971222 -- added these interface functions */

static void set_irq_line(i8086_state *cpustate, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (cpustate->nmi_state == state)
			return;
		cpustate->nmi_state = state;

		/* on a rising edge, signal the NMI */
		if (state != CLEAR_LINE)
			PREFIX(_interrupt)(cpustate, I8086_NMI_INT_VECTOR);
	}
	else
	{
		cpustate->irq_state = state;

		/* if the IF is set, signal an interrupt */
		if (state != CLEAR_LINE && cpustate->IF)
			PREFIX(_interrupt)(cpustate, -1);
	}
}

/* PJB 03/05 */
static void set_test_line(i8086_state *cpustate, int state)
{
        cpustate->test_state = !state;
}

static CPU_EXECUTE( i8086 )
{
	i8086_state *cpustate = device->token;

	cpustate->base[CS] = SegBase(CS);
	cpustate->base[DS] = SegBase(DS);
	cpustate->base[ES] = SegBase(ES);
	cpustate->base[SS] = SegBase(SS);

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


#if (HAS_I80186 || HAS_I80188)

#include "i186intf.h"

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
	i8086_state *cpustate = device->token;

	cpustate->base[CS] = SegBase(CS);
	cpustate->base[DS] = SegBase(DS);
	cpustate->base[ES] = SegBase(ES);
	cpustate->base[SS] = SegBase(SS);

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

#endif



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( i8086 )
{
	i8086_state *cpustate = device->token;

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:				set_irq_line(cpustate, 0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(cpustate, INPUT_LINE_NMI, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_TEST:	set_test_line(cpustate, info->i);					break; /* PJB 03/05 */

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + I8086_PC:
			if (info->i - cpustate->base[CS] >= 0x10000)
			{
				cpustate->base[CS] = info->i & 0xffff0;
				cpustate->sregs[CS] = cpustate->base[CS] >> 4;
			}
			cpustate->pc = info->i;
			break;
		case CPUINFO_INT_REGISTER + I8086_IP:			cpustate->pc = cpustate->base[CS] + info->i;			break;
		case CPUINFO_INT_SP:
			if (info->i - cpustate->base[SS] < 0x10000)
			{
				cpustate->regs.w[SP] = info->i - cpustate->base[SS];
			}
			else
			{
				cpustate->base[SS] = info->i & 0xffff0;
				cpustate->sregs[SS] = cpustate->base[SS] >> 4;
				cpustate->regs.w[SP] = info->i & 0x0000f;
			}
			break;
		case CPUINFO_INT_REGISTER + I8086_SP:			cpustate->regs.w[SP] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I8086_FLAGS: 		cpustate->flags = info->i;	ExpandFlags(info->i); break;
		case CPUINFO_INT_REGISTER + I8086_AX:			cpustate->regs.w[AX] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I8086_CX:			cpustate->regs.w[CX] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I8086_DX:			cpustate->regs.w[DX] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I8086_BX:			cpustate->regs.w[BX] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I8086_BP:			cpustate->regs.w[BP] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I8086_SI:			cpustate->regs.w[SI] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I8086_DI:			cpustate->regs.w[DI] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I8086_ES:			cpustate->sregs[ES] = info->i;	cpustate->base[ES] = SegBase(ES); break;
		case CPUINFO_INT_REGISTER + I8086_CS:			cpustate->sregs[CS] = info->i;	cpustate->base[CS] = SegBase(CS); break;
		case CPUINFO_INT_REGISTER + I8086_SS:			cpustate->sregs[SS] = info->i;	cpustate->base[SS] = SegBase(SS); break;
		case CPUINFO_INT_REGISTER + I8086_DS:			cpustate->sregs[DS] = info->i;	cpustate->base[DS] = SegBase(DS); break;
		case CPUINFO_INT_REGISTER + I8086_VECTOR:		cpustate->int_vector = info->i; 				break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( i8086 )
{
	i8086_state *cpustate = (device != NULL) ? device->token : NULL;

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
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 15;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 50;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 20;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = cpustate->irq_state;					break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = cpustate->nmi_state;					break;

		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_TEST:	info->i = cpustate->test_state;					break; /* PJB 03/05 */

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->prevpc;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + I8086_PC:			info->i = cpustate->pc;							break;
		case CPUINFO_INT_REGISTER + I8086_IP:			info->i = cpustate->pc - cpustate->base[CS];			break;
		case CPUINFO_INT_SP:							info->i = cpustate->base[SS] + cpustate->regs.w[SP];	break;
		case CPUINFO_INT_REGISTER + I8086_SP:			info->i = cpustate->regs.w[SP];					break;
		case CPUINFO_INT_REGISTER + I8086_FLAGS: 		cpustate->flags = CompressFlags(); info->i = cpustate->flags; break;
		case CPUINFO_INT_REGISTER + I8086_AX:			info->i = cpustate->regs.w[AX];					break;
		case CPUINFO_INT_REGISTER + I8086_CX:			info->i = cpustate->regs.w[CX];					break;
		case CPUINFO_INT_REGISTER + I8086_DX:			info->i = cpustate->regs.w[DX];					break;
		case CPUINFO_INT_REGISTER + I8086_BX:			info->i = cpustate->regs.w[BX];					break;
		case CPUINFO_INT_REGISTER + I8086_BP:			info->i = cpustate->regs.w[BP];					break;
		case CPUINFO_INT_REGISTER + I8086_SI:			info->i = cpustate->regs.w[SI];					break;
		case CPUINFO_INT_REGISTER + I8086_DI:			info->i = cpustate->regs.w[DI];					break;
		case CPUINFO_INT_REGISTER + I8086_ES:			info->i = cpustate->sregs[ES];					break;
		case CPUINFO_INT_REGISTER + I8086_CS:			info->i = cpustate->sregs[CS];					break;
		case CPUINFO_INT_REGISTER + I8086_SS:			info->i = cpustate->sregs[SS];					break;
		case CPUINFO_INT_REGISTER + I8086_DS:			info->i = cpustate->sregs[DS];					break;
		case CPUINFO_INT_REGISTER + I8086_VECTOR:		info->i = cpustate->int_vector;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(i8086);			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = CPU_GET_CONTEXT_NAME(dummy);	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = CPU_SET_CONTEXT_NAME(dummy);	break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(i8086);				break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(i8086);				break;
		case CPUINFO_PTR_EXIT:							info->exit = CPU_EXIT_NAME(i8086);				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(i8086);			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i8086);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "8086");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Intel 80x86");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.4");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Real mode i286 emulator v1.4 by Fabrice Frances\n(initial work cpustate->based on David Hedley's pcemu)"); break;

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

		case CPUINFO_STR_REGISTER + I8086_PC: 			sprintf(info->s, "PC:%04X", cpustate->pc); break;
		case CPUINFO_STR_REGISTER + I8086_IP: 			sprintf(info->s, "IP: %04X", cpustate->pc - cpustate->base[CS]); break;
		case CPUINFO_STR_REGISTER + I8086_SP: 			sprintf(info->s, "SP: %04X", cpustate->regs.w[SP]); break;
		case CPUINFO_STR_REGISTER + I8086_FLAGS:		sprintf(info->s, "F:%04X", cpustate->flags); break;
		case CPUINFO_STR_REGISTER + I8086_AX: 			sprintf(info->s, "AX:%04X", cpustate->regs.w[AX]); break;
		case CPUINFO_STR_REGISTER + I8086_CX: 			sprintf(info->s, "CX:%04X", cpustate->regs.w[CX]); break;
		case CPUINFO_STR_REGISTER + I8086_DX: 			sprintf(info->s, "DX:%04X", cpustate->regs.w[DX]); break;
		case CPUINFO_STR_REGISTER + I8086_BX: 			sprintf(info->s, "BX:%04X", cpustate->regs.w[BX]); break;
		case CPUINFO_STR_REGISTER + I8086_BP: 			sprintf(info->s, "BP:%04X", cpustate->regs.w[BP]); break;
		case CPUINFO_STR_REGISTER + I8086_SI: 			sprintf(info->s, "SI: %04X", cpustate->regs.w[SI]); break;
		case CPUINFO_STR_REGISTER + I8086_DI: 			sprintf(info->s, "DI: %04X", cpustate->regs.w[DI]); break;
		case CPUINFO_STR_REGISTER + I8086_ES: 			sprintf(info->s, "ES:%04X", cpustate->sregs[ES]); break;
		case CPUINFO_STR_REGISTER + I8086_CS: 			sprintf(info->s, "CS:%04X", cpustate->sregs[CS]); break;
		case CPUINFO_STR_REGISTER + I8086_SS: 			sprintf(info->s, "SS:%04X", cpustate->sregs[SS]); break;
		case CPUINFO_STR_REGISTER + I8086_DS: 			sprintf(info->s, "DS:%04X", cpustate->sregs[DS]); break;
		case CPUINFO_STR_REGISTER + I8086_VECTOR:		sprintf(info->s, "V:%02X", cpustate->int_vector); break;
	}
}


#if (HAS_I8088)
/**************************************************************************
 * CPU-specific get_info/set_info
 **************************************************************************/

CPU_GET_INFO( i8088 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(i8088);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "8088");				break;

		default:										CPU_GET_INFO_CALL(i8086);				break;
	}
}
#endif


#if (HAS_I80186)
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
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(i80186);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "80186");				break;

		default:										CPU_GET_INFO_CALL(i8086);				break;
	}
}
#endif


#if (HAS_I80188)
/**************************************************************************
 * CPU-specific get_info/set_info
 **************************************************************************/

CPU_GET_INFO( i80188 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(i8088);		break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(i80186);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "80188");				break;

		default:										CPU_GET_INFO_CALL(i8086);				break;
	}
}
#endif
