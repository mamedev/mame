/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/

#include "debugger.h"
#include "host.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) mame_printf_debug x; } while (0)

/* All post-i286 CPUs have a 16MB address space */
#define AMASK	cpustate->amask


#define INPUT_LINE_A20		1

#include "i86priv.h"
#include "i286.h"


#include "i86time.c"
#include "i86mem.h"

/***************************************************************************/
/* cpu state                                                               */
/***************************************************************************/
/* I86 registers */
typedef union
{                   /* eight general registers */
    UINT16 w[8];    /* viewed as 16 bits registers */
    UINT8  b[16];   /* or as 8 bit registers */
} i80286basicregs;

typedef struct _i80286_state i80286_state;
struct _i80286_state
{
    i80286basicregs regs;
	UINT32 	amask;			/* address mask */
    UINT32  pc;
    UINT32  prevpc;
	UINT16	flags;
	UINT16	msw;
	UINT32	base[4];
	UINT16	sregs[4];
	UINT16	limit[4];
	UINT8 rights[4];
	struct {
		UINT32 base;
		UINT16 limit;
	} gdtr, idtr;
	struct {
		UINT16 sel;
		UINT32 base;
		UINT16 limit;
		UINT8 rights;
	} ldtr, tr;
	cpu_irq_callback irq_callback;
	const device_config *device;
	const address_space *program;
	const address_space *io;
    INT32     AuxVal, OverVal, SignVal, ZeroVal, CarryVal, DirVal; /* 0 or non-0 valued flags */
    UINT8	ParityVal;
	UINT8	TF, IF; 	/* 0 or 1 valued flags */
	UINT8	int_vector;
	INT8	nmi_state;
	INT8	irq_state;
	INT8	test_state;		/* PJB 03/05 */
	UINT8 rep_in_progress;
	INT32 	extra_cycles;       /* extra cycles for interrupts */

	memory_interface	mem;
	int icount;
	unsigned prefix_base;
	char seg_prefix;
	unsigned ea;
	UINT16 eo; /* HJB 12/13/98 effective offset of the address (before segment is added) */
};

INLINE i80286_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == CPU);
	assert(cpu_get_type(device) == CPU_I80286);
	return (i80286_state *)device->token;
}

#define INT_IRQ 0x01
#define NMI_IRQ 0x02

static UINT8 parity_table[256];

static struct i80x86_timing timing;

/***************************************************************************/

#define I80286
#define PREFIX(fname) i80286##fname
#define PREFIX86(fname) i80286##fname
#define PREFIX186(fname) i80286##fname
#define PREFIX286(fname) i80286##fname
#define i8086_state i80286_state

#include "ea.h"
#include "modrm.h"
#include "instr86.h"
#include "instr186.h"
#include "instr286.h"
#include "table286.h"
#include "instr86.c"
#include "instr186.c"
#include "instr286.c"
#include "i86mem.c"

static void i80286_urinit(void)
{
	unsigned int i,j,c;
	static const BREGS reg_name[8]={ AL, CL, DL, BL, AH, CH, DH, BH };

	for (i = 0;i < 256; i++)
	{
		for (j = i, c = 0; j > 0; j >>= 1)
			if (j & 1) c++;

		parity_table[i] = !(c & 1);
	}

	for (i = 0; i < 256; i++)
	{
		Mod_RM.reg.b[i] = reg_name[(i & 0x38) >> 3];
		Mod_RM.reg.w[i] = (WREGS) ( (i & 0x38) >> 3) ;
	}

	for (i = 0xc0; i < 0x100; i++)
	{
		Mod_RM.RM.w[i] = (WREGS)( i & 7 );
		Mod_RM.RM.b[i] = (BREGS)reg_name[i & 7];
	}
}

static void i80286_set_a20_line(i80286_state *cpustate, int state)
{
	cpustate->amask = state ? 0x00ffffff : 0x000fffff;
}

static CPU_RESET( i80286 )
{
	i80286_state *cpustate = get_safe_token(device);
	static int urinit=1;

	/* in my docu not all registers are initialized! */
	if (urinit) {
		i80286_urinit();
		urinit=0;

	}

	cpustate->sregs[CS] = 0xf000;
	cpustate->base[CS] = 0xff0000;
	/* temporary, until I have the right reset vector working */
	cpustate->base[CS] = cpustate->sregs[CS] << 4;
	cpustate->pc = 0xffff0;
	cpustate->limit[CS]=cpustate->limit[SS]=cpustate->limit[DS]=cpustate->limit[ES]=0xffff;
	cpustate->sregs[DS]=cpustate->sregs[SS]=cpustate->sregs[ES]=0;
	cpustate->base[DS]=cpustate->base[SS]=cpustate->base[ES]=0;
	cpustate->msw=0xfff0;
	cpustate->flags=2;
	ExpandFlags(cpustate->flags);
	cpustate->idtr.base=0;cpustate->idtr.limit=0x3ff;
	cpustate->rep_in_progress = FALSE;

	CHANGE_PC(cpustate->pc);

}

/****************************************************************************/

/* ASG 971222 -- added these interface functions */

static void set_irq_line(i80286_state *cpustate, int irqline, int state)
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

static CPU_EXECUTE( i80286 )
{
	i80286_state *cpustate = get_safe_token(device);

	/* copy over the cycle counts if they're not correct */
	if (timing.id != 80286)
		timing = i80286_cycles;

	/* adjust for any interrupts that came in */
	cpustate->icount = cycles;
	cpustate->icount -= cpustate->extra_cycles;
	cpustate->extra_cycles = 0;

	/* run until we're out */
	while(cpustate->icount>0)
	{
		LOG(("[%04x:%04x]=%02x\tF:%04x\tAX=%04x\tBX=%04x\tCX=%04x\tDX=%04x %d%d%d%d%d%d%d%d%d\n",cpustate->sregs[CS],cpustate->pc - cpustate->base[CS],ReadByte(cpustate->pc),cpustate->flags,cpustate->regs.w[AX],cpustate->regs.w[BX],cpustate->regs.w[CX],cpustate->regs.w[DX], cpustate->AuxVal?1:0, cpustate->OverVal?1:0, cpustate->SignVal?1:0, cpustate->ZeroVal?1:0, cpustate->CarryVal?1:0, cpustate->ParityVal?1:0,cpustate->TF, cpustate->IF, cpustate->DirVal<0?1:0));
		debugger_instruction_hook(device, cpustate->pc);

		cpustate->seg_prefix=FALSE;
		cpustate->prevpc = cpustate->pc;

		TABLE286 // call instruction
    }

	/* adjust for any interrupts that came in */
	cpustate->icount -= cpustate->extra_cycles;
	cpustate->extra_cycles = 0;

	return cycles - cpustate->icount;
}

extern int i386_dasm_one(char *buffer, UINT32 eip, const UINT8 *oprom, int mode);

static CPU_DISASSEMBLE( i80286 )
{
	return i386_dasm_one(buffer, pc, oprom, 16);
}

static CPU_INIT( i80286 )
{
	i80286_state *cpustate = get_safe_token(device);

	state_save_register_device_item_array(device, 0, cpustate->regs.w);
	state_save_register_device_item(device, 0, cpustate->amask);
	state_save_register_device_item(device, 0, cpustate->pc);
	state_save_register_device_item(device, 0, cpustate->prevpc);
	state_save_register_device_item(device, 0, cpustate->msw);
	state_save_register_device_item_array(device, 0, cpustate->base);
	state_save_register_device_item_array(device, 0, cpustate->sregs);
	state_save_register_device_item_array(device, 0, cpustate->limit);
	state_save_register_device_item_array(device, 0, cpustate->rights);
	state_save_register_device_item(device, 0, cpustate->gdtr.base);
	state_save_register_device_item(device, 0, cpustate->gdtr.limit);
	state_save_register_device_item(device, 0, cpustate->idtr.base);
	state_save_register_device_item(device, 0, cpustate->idtr.limit);
	state_save_register_device_item(device, 0, cpustate->ldtr.sel);
	state_save_register_device_item(device, 0, cpustate->ldtr.base);
	state_save_register_device_item(device, 0, cpustate->ldtr.limit);
	state_save_register_device_item(device, 0, cpustate->ldtr.rights);
	state_save_register_device_item(device, 0, cpustate->tr.sel);
	state_save_register_device_item(device, 0, cpustate->tr.base);
	state_save_register_device_item(device, 0, cpustate->tr.limit);
	state_save_register_device_item(device, 0, cpustate->tr.rights);
	state_save_register_device_item(device, 0, cpustate->AuxVal);
	state_save_register_device_item(device, 0, cpustate->OverVal);
	state_save_register_device_item(device, 0, cpustate->SignVal);
	state_save_register_device_item(device, 0, cpustate->ZeroVal);
	state_save_register_device_item(device, 0, cpustate->CarryVal);
	state_save_register_device_item(device, 0, cpustate->DirVal);
	state_save_register_device_item(device, 0, cpustate->ParityVal);
	state_save_register_device_item(device, 0, cpustate->TF);
	state_save_register_device_item(device, 0, cpustate->IF);
	state_save_register_device_item(device, 0, cpustate->int_vector);
	state_save_register_device_item(device, 0, cpustate->nmi_state);
	state_save_register_device_item(device, 0, cpustate->irq_state);
	state_save_register_device_item(device, 0, cpustate->extra_cycles);
	state_save_register_device_item(device, 0, cpustate->rep_in_progress);

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cpustate->io = memory_find_address_space(device, ADDRESS_SPACE_IO);

	/* If a reset parameter is given, take it as pointer to an address mask */
	if( device->static_config )
		cpustate->amask = *(unsigned*)device->static_config;
	else
		cpustate->amask = 0x00ffff;

	configure_memory_16bit(cpustate);
}





/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( i80286 )
{
	i80286_state *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:				set_irq_line(cpustate, 0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(cpustate, INPUT_LINE_NMI, info->i);	break;

		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_A20:	i80286_set_a20_line(cpustate, info->i);			break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + I80286_PC:
			if (PM)
			{
				/* protected mode NYI */
			}
			else
			{
				if (info->i - cpustate->base[CS] >= 0x10000)
				{
					cpustate->base[CS] = info->i & 0xffff0;
					cpustate->sregs[CS] = cpustate->base[CS] >> 4;
				}
				cpustate->pc = info->i;
			}
			break;

		case CPUINFO_INT_REGISTER + I80286_IP:			cpustate->pc = cpustate->base[CS] + info->i;			break;
		case CPUINFO_INT_SP:
			if (PM)
			{
				/* protected mode NYI */
			}
			else
			{
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
			}
			break;

		case CPUINFO_INT_REGISTER + I80286_SP:			cpustate->regs.w[SP] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I80286_FLAGS: 		cpustate->flags = info->i;	ExpandFlags(info->i); break;
		case CPUINFO_INT_REGISTER + I80286_AX:			cpustate->regs.w[AX] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I80286_CX:			cpustate->regs.w[CX] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I80286_DX:			cpustate->regs.w[DX] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I80286_BX:			cpustate->regs.w[BX] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I80286_BP:			cpustate->regs.w[BP] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I80286_SI:			cpustate->regs.w[SI] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I80286_DI:			cpustate->regs.w[DI] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I80286_ES:			cpustate->sregs[ES] = info->i;	cpustate->base[ES] = SegBase(ES); break;
		case CPUINFO_INT_REGISTER + I80286_CS:			cpustate->sregs[CS] = info->i;	cpustate->base[CS] = SegBase(CS); break;
		case CPUINFO_INT_REGISTER + I80286_SS:			cpustate->sregs[SS] = info->i;	cpustate->base[SS] = SegBase(SS); break;
		case CPUINFO_INT_REGISTER + I80286_DS:			cpustate->sregs[DS] = info->i;	cpustate->base[DS] = SegBase(DS); break;
		case CPUINFO_INT_REGISTER + I80286_VECTOR:		cpustate->int_vector = info->i; 				break;
	}
}



/****************************************************************************
 * Generic get_info
 ****************************************************************************/

CPU_GET_INFO( i80286 )
{
	i80286_state *cpustate = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(i80286_state);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 15;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 50;							break;

		case CPUINFO_INT_DATABUS_WIDTH_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_PROGRAM: info->i = 24;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH_IO:		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_IO: 		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = cpustate->irq_state;					break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = cpustate->nmi_state;					break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->prevpc;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + I80286_PC:			info->i = cpustate->pc;							break;
		case CPUINFO_INT_REGISTER + I80286_IP:			info->i = cpustate->pc - cpustate->base[CS];			break;
		case CPUINFO_INT_SP:							info->i = cpustate->base[SS] + cpustate->regs.w[SP];	break;
		case CPUINFO_INT_REGISTER + I80286_SP:			info->i = cpustate->regs.w[SP];					break;
		case CPUINFO_INT_REGISTER + I80286_FLAGS: 		cpustate->flags = CompressFlags(); info->i = cpustate->flags; break;
		case CPUINFO_INT_REGISTER + I80286_AX:			info->i = cpustate->regs.w[AX];					break;
		case CPUINFO_INT_REGISTER + I80286_CX:			info->i = cpustate->regs.w[CX];					break;
		case CPUINFO_INT_REGISTER + I80286_DX:			info->i = cpustate->regs.w[DX];					break;
		case CPUINFO_INT_REGISTER + I80286_BX:			info->i = cpustate->regs.w[BX];					break;
		case CPUINFO_INT_REGISTER + I80286_BP:			info->i = cpustate->regs.w[BP];					break;
		case CPUINFO_INT_REGISTER + I80286_SI:			info->i = cpustate->regs.w[SI];					break;
		case CPUINFO_INT_REGISTER + I80286_DI:			info->i = cpustate->regs.w[DI];					break;
		case CPUINFO_INT_REGISTER + I80286_ES:			info->i = cpustate->sregs[ES];					break;
		case CPUINFO_INT_REGISTER + I80286_CS:			info->i = cpustate->sregs[CS];					break;
		case CPUINFO_INT_REGISTER + I80286_SS:			info->i = cpustate->sregs[SS];					break;
		case CPUINFO_INT_REGISTER + I80286_DS:			info->i = cpustate->sregs[DS];					break;
		case CPUINFO_INT_REGISTER + I80286_VECTOR:		info->i = cpustate->int_vector;					break;
		case CPUINFO_INT_REGISTER + I80286_MSW:			info->i = cpustate->msw;						break;
		case CPUINFO_INT_REGISTER + I80286_GDTR_BASE:	info->i = cpustate->gdtr.base;					break;
		case CPUINFO_INT_REGISTER + I80286_GDTR_LIMIT:	info->i = cpustate->gdtr.limit;					break;
		case CPUINFO_INT_REGISTER + I80286_IDTR_BASE:	info->i = cpustate->idtr.base;					break;
		case CPUINFO_INT_REGISTER + I80286_IDTR_LIMIT:	info->i = cpustate->idtr.limit;					break;
		case CPUINFO_INT_REGISTER + I80286_LDTR_BASE:	info->i = cpustate->ldtr.base;					break;
		case CPUINFO_INT_REGISTER + I80286_LDTR_LIMIT:	info->i = cpustate->ldtr.limit;					break;
		case CPUINFO_INT_REGISTER + I80286_TR_BASE:		info->i = cpustate->tr.base;					break;
		case CPUINFO_INT_REGISTER + I80286_TR_LIMIT:	info->i = cpustate->tr.limit;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(i80286);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i80286);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(i80286);				break;
		case CPUINFO_FCT_EXIT:							info->exit = NULL;						break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(i80286);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i80286);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "80286");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Intel 80286");			break;
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

		case CPUINFO_STR_REGISTER + I80286_PC: 			sprintf(info->s, "PC:%04X", cpustate->pc); break;
		case CPUINFO_STR_REGISTER + I80286_IP: 			sprintf(info->s, "IP: %04X", cpustate->pc - cpustate->base[CS]); break;
		case CPUINFO_STR_REGISTER + I80286_SP: 			sprintf(info->s, "SP: %04X", cpustate->regs.w[SP]); break;
		case CPUINFO_STR_REGISTER + I80286_FLAGS:		sprintf(info->s, "F:%04X", cpustate->flags); break;
		case CPUINFO_STR_REGISTER + I80286_AX: 			sprintf(info->s, "AX:%04X", cpustate->regs.w[AX]); break;
		case CPUINFO_STR_REGISTER + I80286_CX: 			sprintf(info->s, "CX:%04X", cpustate->regs.w[CX]); break;
		case CPUINFO_STR_REGISTER + I80286_DX: 			sprintf(info->s, "DX:%04X", cpustate->regs.w[DX]); break;
		case CPUINFO_STR_REGISTER + I80286_BX: 			sprintf(info->s, "BX:%04X", cpustate->regs.w[BX]); break;
		case CPUINFO_STR_REGISTER + I80286_BP: 			sprintf(info->s, "BP:%04X", cpustate->regs.w[BP]); break;
		case CPUINFO_STR_REGISTER + I80286_SI: 			sprintf(info->s, "SI: %04X", cpustate->regs.w[SI]); break;
		case CPUINFO_STR_REGISTER + I80286_DI: 			sprintf(info->s, "DI: %04X", cpustate->regs.w[DI]); break;
		case CPUINFO_STR_REGISTER + I80286_CS: 			sprintf(info->s, "CS:  %04X %02X", cpustate->sregs[CS], cpustate->rights[CS]); break;
		case CPUINFO_STR_REGISTER + I80286_CS_2: 		sprintf(info->s, "%06X %04X", cpustate->base[CS], cpustate->limit[CS]); break;
		case CPUINFO_STR_REGISTER + I80286_SS: 			sprintf(info->s, "SS:  %04X %02X", cpustate->sregs[SS], cpustate->rights[SS]); break;
		case CPUINFO_STR_REGISTER + I80286_SS_2: 		sprintf(info->s, "%06X %04X", cpustate->base[SS], cpustate->limit[SS]); break;
		case CPUINFO_STR_REGISTER + I80286_DS: 			sprintf(info->s, "DS:  %04X %02X", cpustate->sregs[DS], cpustate->rights[DS]); break;
		case CPUINFO_STR_REGISTER + I80286_DS_2: 		sprintf(info->s, "%06X %04X", cpustate->base[DS], cpustate->limit[DS]); break;
		case CPUINFO_STR_REGISTER + I80286_ES: 			sprintf(info->s, "ES:  %04X %02X", cpustate->sregs[ES], cpustate->rights[ES]); break;
		case CPUINFO_STR_REGISTER + I80286_ES_2: 		sprintf(info->s, "%06X %04X", cpustate->base[ES], cpustate->limit[ES]); break;
		case CPUINFO_STR_REGISTER + I80286_VECTOR:		sprintf(info->s, "V:%02X", cpustate->int_vector); break;
		case CPUINFO_STR_REGISTER + I80286_MSW:			sprintf(info->s, "MSW:%04X", cpustate->msw); break;
		case CPUINFO_STR_REGISTER + I80286_TR_BASE:		sprintf(info->s, "GDTR: %06X", cpustate->gdtr.base); break;
		case CPUINFO_STR_REGISTER + I80286_TR_LIMIT:	sprintf(info->s, "%04X", cpustate->gdtr.limit); break;
		case CPUINFO_STR_REGISTER + I80286_GDTR_BASE:	sprintf(info->s, "IDTR: %06X", cpustate->idtr.base); break;
		case CPUINFO_STR_REGISTER + I80286_GDTR_LIMIT:	sprintf(info->s, "%04X", cpustate->idtr.limit); break;
		case CPUINFO_STR_REGISTER + I80286_LDTR_BASE:	sprintf(info->s, "LDTR:%04X %02X", cpustate->ldtr.sel, cpustate->ldtr.rights); break;
		case CPUINFO_STR_REGISTER + I80286_LDTR_LIMIT:	sprintf(info->s, "%06X %04X", cpustate->ldtr.base, cpustate->ldtr.limit); break;
		case CPUINFO_STR_REGISTER + I80286_IDTR_BASE:	sprintf(info->s, "IDTR: %06X", cpustate->idtr.base); break;
		case CPUINFO_STR_REGISTER + I80286_IDTR_LIMIT:	sprintf(info->s, "%04X", cpustate->idtr.limit); break;
	}
}
