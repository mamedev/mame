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

typedef struct
{
	i8086basicregs regs;
	UINT32 pc;
	UINT32 prevpc;
	UINT32 base[4];
	UINT16 sregs[4];
	UINT16 flags;
	cpu_irq_callback irq_callback;
	const device_config *device;
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
}
i8086_Regs;


#include "i86time.c"

/***************************************************************************/
/* cpu state                                                               */
/***************************************************************************/

static int i8086_ICount;

static i8086_Regs I;
static unsigned prefix_base;		   /* base address of the latest prefix segment */
static char seg_prefix;				   /* prefix segment indicator */

static UINT8 parity_table[256];

static struct i80x86_timing timing;

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
	static const char type[] = "I8086";
	state_save_register_item_array(type, device->tag, 0, I.regs.w);
	state_save_register_item(type, device->tag, 0, I.pc);
	state_save_register_item(type, device->tag, 0, I.prevpc);
	state_save_register_item_array(type, device->tag, 0, I.base);
	state_save_register_item_array(type, device->tag, 0, I.sregs);
	state_save_register_item(type, device->tag, 0, I.flags);
	state_save_register_item(type, device->tag, 0, I.AuxVal);
	state_save_register_item(type, device->tag, 0, I.OverVal);
	state_save_register_item(type, device->tag, 0, I.SignVal);
	state_save_register_item(type, device->tag, 0, I.ZeroVal);
	state_save_register_item(type, device->tag, 0, I.CarryVal);
	state_save_register_item(type, device->tag, 0, I.DirVal);
	state_save_register_item(type, device->tag, 0, I.ParityVal);
	state_save_register_item(type, device->tag, 0, I.TF);
	state_save_register_item(type, device->tag, 0, I.IF);
	state_save_register_item(type, device->tag, 0, I.MF);
	state_save_register_item(type, device->tag, 0, I.int_vector);
	state_save_register_item(type, device->tag, 0, I.nmi_state);
	state_save_register_item(type, device->tag, 0, I.irq_state);
	state_save_register_item(type, device->tag, 0, I.extra_cycles);
	state_save_register_item(type, device->tag, 0, I.test_state);	/* PJB 03/05 */
}

static CPU_INIT( i8086 )
{
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

	I.irq_callback = irqcallback;
	I.device = device;

	i8086_state_register(device);
	configure_memory_16bit();
}

#if (HAS_I8088||HAS_I80188)
static CPU_INIT( i8088 )
{
	CPU_INIT_CALL(i8086);
	configure_memory_8bit();
}
#endif

static CPU_RESET( i8086 )
{
	cpu_irq_callback save_irqcallback;
    memory_interface save_mem;

	save_irqcallback = I.irq_callback;
	save_mem = I.mem;
	memset(&I, 0, sizeof (I));
	I.irq_callback = save_irqcallback;
	I.mem = save_mem;
	I.device = device;

	I.sregs[CS] = 0xf000;
	I.base[CS] = SegBase(CS);
	I.pc = 0xffff0 & AMASK;
	ExpandFlags(I.flags);

	change_pc(I.pc);
}

static CPU_EXIT( i8086 )
{
	/* nothing to do ? */
}

/* ASG 971222 -- added these interface functions */

static CPU_GET_CONTEXT( i8086 )
{
	if (dst)
		*(i8086_Regs *) dst = I;
}

static CPU_SET_CONTEXT( i8086 )
{
	if (src)
	{
		I = *(i8086_Regs *)src;
		I.base[CS] = SegBase(CS);
		I.base[DS] = SegBase(DS);
		I.base[ES] = SegBase(ES);
		I.base[SS] = SegBase(SS);
		change_pc(I.pc);
	}
}

static void set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (I.nmi_state == state)
			return;
		I.nmi_state = state;

		/* on a rising edge, signal the NMI */
		if (state != CLEAR_LINE)
			PREFIX(_interrupt)(I8086_NMI_INT_VECTOR);
	}
	else
	{
		I.irq_state = state;

		/* if the IF is set, signal an interrupt */
		if (state != CLEAR_LINE && I.IF)
			PREFIX(_interrupt)(-1);
	}
}

/* PJB 03/05 */
static void set_test_line(int state)
{
        I.test_state = !state;
}

static CPU_EXECUTE( i8086 )
{
	/* copy over the cycle counts if they're not correct */
	if (timing.id != 8086)
		timing = i8086_cycles;

	/* adjust for any interrupts that came in */
	i8086_ICount = cycles;
	i8086_ICount -= I.extra_cycles;
	I.extra_cycles = 0;

	/* run until we're out */
	while (i8086_ICount > 0)
	{
		LOG(("[%04x:%04x]=%02x\tF:%04x\tAX=%04x\tBX=%04x\tCX=%04x\tDX=%04x %d%d%d%d%d%d%d%d%d\n",
				I.sregs[CS], I.pc - I.base[CS], ReadByte(I.pc), I.flags, I.regs.w[AX], I.regs.w[BX], I.regs.w[CX], I.regs.w[DX], I.AuxVal ? 1 : 0, I.OverVal ? 1 : 0,
				I.SignVal ? 1 : 0, I.ZeroVal ? 1 : 0, I.CarryVal ? 1 : 0, I.ParityVal ? 1 : 0, I.TF, I.IF, I.DirVal < 0 ? 1 : 0));
		debugger_instruction_hook(device, I.pc);

		seg_prefix = FALSE;
		I.prevpc = I.pc;
		TABLE86;
	}

	/* adjust for any interrupts that came in */
	i8086_ICount -= I.extra_cycles;
	I.extra_cycles = 0;

	return cycles - i8086_ICount;
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
	/* copy over the cycle counts if they're not correct */
	if (timing.id != 80186)
		timing = i80186_cycles;

	/* adjust for any interrupts that came in */
	i8086_ICount = cycles;
	i8086_ICount -= I.extra_cycles;
	I.extra_cycles = 0;

	/* run until we're out */
	while (i8086_ICount > 0)
	{
		LOG(("[%04x:%04x]=%02x\tAX=%04x\tBX=%04x\tCX=%04x\tDX=%04x\n", I.sregs[CS], I.pc, ReadByte(I.pc), I.regs.w[AX],
			   I.regs.w[BX], I.regs.w[CX], I.regs.w[DX]));
		debugger_instruction_hook(device, I.pc);

		seg_prefix = FALSE;
		I.prevpc = I.pc;
		TABLE186;
	}

	/* adjust for any interrupts that came in */
	i8086_ICount -= I.extra_cycles;
	I.extra_cycles = 0;

	return cycles - i8086_ICount;
}

#endif



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( i8086 )
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:				set_irq_line(0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(INPUT_LINE_NMI, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_TEST:	set_test_line(info->i);					break; /* PJB 03/05 */

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + I8086_PC:
			if (info->i - I.base[CS] >= 0x10000)
			{
				I.base[CS] = info->i & 0xffff0;
				I.sregs[CS] = I.base[CS] >> 4;
			}
			I.pc = info->i;
			break;
		case CPUINFO_INT_REGISTER + I8086_IP:			I.pc = I.base[CS] + info->i;			break;
		case CPUINFO_INT_SP:
			if (info->i - I.base[SS] < 0x10000)
			{
				I.regs.w[SP] = info->i - I.base[SS];
			}
			else
			{
				I.base[SS] = info->i & 0xffff0;
				I.sregs[SS] = I.base[SS] >> 4;
				I.regs.w[SP] = info->i & 0x0000f;
			}
			break;
		case CPUINFO_INT_REGISTER + I8086_SP:			I.regs.w[SP] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I8086_FLAGS: 		I.flags = info->i;	ExpandFlags(info->i); break;
		case CPUINFO_INT_REGISTER + I8086_AX:			I.regs.w[AX] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I8086_CX:			I.regs.w[CX] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I8086_DX:			I.regs.w[DX] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I8086_BX:			I.regs.w[BX] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I8086_BP:			I.regs.w[BP] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I8086_SI:			I.regs.w[SI] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I8086_DI:			I.regs.w[DI] = info->i; 				break;
		case CPUINFO_INT_REGISTER + I8086_ES:			I.sregs[ES] = info->i;	I.base[ES] = SegBase(ES); break;
		case CPUINFO_INT_REGISTER + I8086_CS:			I.sregs[CS] = info->i;	I.base[CS] = SegBase(CS); break;
		case CPUINFO_INT_REGISTER + I8086_SS:			I.sregs[SS] = info->i;	I.base[SS] = SegBase(SS); break;
		case CPUINFO_INT_REGISTER + I8086_DS:			I.sregs[DS] = info->i;	I.base[DS] = SegBase(DS); break;
		case CPUINFO_INT_REGISTER + I8086_VECTOR:		I.int_vector = info->i; 				break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( i8086 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(I);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
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

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = I.irq_state;					break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = I.nmi_state;					break;

		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_TEST:	info->i = I.test_state;					break; /* PJB 03/05 */

		case CPUINFO_INT_PREVIOUSPC:					info->i = I.prevpc;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + I8086_PC:			info->i = I.pc;							break;
		case CPUINFO_INT_REGISTER + I8086_IP:			info->i = I.pc - I.base[CS];			break;
		case CPUINFO_INT_SP:							info->i = I.base[SS] + I.regs.w[SP];	break;
		case CPUINFO_INT_REGISTER + I8086_SP:			info->i = I.regs.w[SP];					break;
		case CPUINFO_INT_REGISTER + I8086_FLAGS: 		I.flags = CompressFlags(); info->i = I.flags; break;
		case CPUINFO_INT_REGISTER + I8086_AX:			info->i = I.regs.w[AX];					break;
		case CPUINFO_INT_REGISTER + I8086_CX:			info->i = I.regs.w[CX];					break;
		case CPUINFO_INT_REGISTER + I8086_DX:			info->i = I.regs.w[DX];					break;
		case CPUINFO_INT_REGISTER + I8086_BX:			info->i = I.regs.w[BX];					break;
		case CPUINFO_INT_REGISTER + I8086_BP:			info->i = I.regs.w[BP];					break;
		case CPUINFO_INT_REGISTER + I8086_SI:			info->i = I.regs.w[SI];					break;
		case CPUINFO_INT_REGISTER + I8086_DI:			info->i = I.regs.w[DI];					break;
		case CPUINFO_INT_REGISTER + I8086_ES:			info->i = I.sregs[ES];					break;
		case CPUINFO_INT_REGISTER + I8086_CS:			info->i = I.sregs[CS];					break;
		case CPUINFO_INT_REGISTER + I8086_SS:			info->i = I.sregs[SS];					break;
		case CPUINFO_INT_REGISTER + I8086_DS:			info->i = I.sregs[DS];					break;
		case CPUINFO_INT_REGISTER + I8086_VECTOR:		info->i = I.int_vector;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(i8086);			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = CPU_GET_CONTEXT_NAME(i8086);	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = CPU_SET_CONTEXT_NAME(i8086);	break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(i8086);				break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(i8086);				break;
		case CPUINFO_PTR_EXIT:							info->exit = CPU_EXIT_NAME(i8086);				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(i8086);			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i8086);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &i8086_ICount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "8086");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Intel 80x86");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.4");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Real mode i286 emulator v1.4 by Fabrice Frances\n(initial work I.based on David Hedley's pcemu)"); break;

		case CPUINFO_STR_FLAGS:
			I.flags = CompressFlags();
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
					I.flags & 0x8000 ? '?' : '.',
					I.flags & 0x4000 ? '?' : '.',
					I.flags & 0x2000 ? '?' : '.',
					I.flags & 0x1000 ? '?' : '.',
					I.flags & 0x0800 ? 'O' : '.',
					I.flags & 0x0400 ? 'D' : '.',
					I.flags & 0x0200 ? 'I' : '.',
					I.flags & 0x0100 ? 'T' : '.',
					I.flags & 0x0080 ? 'S' : '.',
					I.flags & 0x0040 ? 'Z' : '.',
					I.flags & 0x0020 ? '?' : '.',
					I.flags & 0x0010 ? 'A' : '.',
					I.flags & 0x0008 ? '?' : '.',
					I.flags & 0x0004 ? 'P' : '.',
					I.flags & 0x0002 ? 'N' : '.',
					I.flags & 0x0001 ? 'C' : '.');
			break;

		case CPUINFO_STR_REGISTER + I8086_PC: 			sprintf(info->s, "PC:%04X", I.pc); break;
		case CPUINFO_STR_REGISTER + I8086_IP: 			sprintf(info->s, "IP: %04X", I.pc - I.base[CS]); break;
		case CPUINFO_STR_REGISTER + I8086_SP: 			sprintf(info->s, "SP: %04X", I.regs.w[SP]); break;
		case CPUINFO_STR_REGISTER + I8086_FLAGS:		sprintf(info->s, "F:%04X", I.flags); break;
		case CPUINFO_STR_REGISTER + I8086_AX: 			sprintf(info->s, "AX:%04X", I.regs.w[AX]); break;
		case CPUINFO_STR_REGISTER + I8086_CX: 			sprintf(info->s, "CX:%04X", I.regs.w[CX]); break;
		case CPUINFO_STR_REGISTER + I8086_DX: 			sprintf(info->s, "DX:%04X", I.regs.w[DX]); break;
		case CPUINFO_STR_REGISTER + I8086_BX: 			sprintf(info->s, "BX:%04X", I.regs.w[BX]); break;
		case CPUINFO_STR_REGISTER + I8086_BP: 			sprintf(info->s, "BP:%04X", I.regs.w[BP]); break;
		case CPUINFO_STR_REGISTER + I8086_SI: 			sprintf(info->s, "SI: %04X", I.regs.w[SI]); break;
		case CPUINFO_STR_REGISTER + I8086_DI: 			sprintf(info->s, "DI: %04X", I.regs.w[DI]); break;
		case CPUINFO_STR_REGISTER + I8086_ES: 			sprintf(info->s, "ES:%04X", I.sregs[ES]); break;
		case CPUINFO_STR_REGISTER + I8086_CS: 			sprintf(info->s, "CS:%04X", I.sregs[CS]); break;
		case CPUINFO_STR_REGISTER + I8086_SS: 			sprintf(info->s, "SS:%04X", I.sregs[SS]); break;
		case CPUINFO_STR_REGISTER + I8086_DS: 			sprintf(info->s, "DS:%04X", I.sregs[DS]); break;
		case CPUINFO_STR_REGISTER + I8086_VECTOR:		sprintf(info->s, "V:%02X", I.int_vector); break;
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
