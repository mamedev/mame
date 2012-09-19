/*** t11: Portable DEC T-11 emulator ******************************************

    Copyright Aaron Giles

    System dependencies:    long must be at least 32 bits
                            word must be 16 bit unsigned int
                            byte must be 8 bit unsigned int
                            long must be more than 16 bits
                            arrays up to 65536 bytes must be supported
                            machine must be twos complement

*****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "t11.h"


/*************************************
 *
 *  Internal state representation
 *
 *************************************/

struct t11_state
{
	PAIR				ppc;	/* previous program counter */
    PAIR				reg[8];
    PAIR				psw;
    UINT16				initial_pc;
    UINT8				wait_state;
    UINT8				irq_state;
    int					icount;
	device_irq_acknowledge_callback	irq_callback;
	legacy_cpu_device *		device;
	address_space *program;
	direct_read_data *direct;
};


INLINE t11_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == T11);
	return (t11_state *)downcast<legacy_cpu_device *>(device)->token();
}



/*************************************
 *
 *  Macro shortcuts
 *
 *************************************/

/* registers of various sizes */
#define REGD(x) reg[x].d
#define REGW(x) reg[x].w.l
#define REGB(x) reg[x].b.l

/* PC, SP, and PSW definitions */
#define SP		REGW(6)
#define PC		REGW(7)
#define SPD 	REGD(6)
#define PCD 	REGD(7)
#define PSW 	psw.b.l


/*************************************
 *
 *  Low-level memory operations
 *
 *************************************/

INLINE int ROPCODE(t11_state *cpustate)
{
	cpustate->PC &= 0xfffe;
	int val = cpustate->direct->read_decrypted_word(cpustate->PC);
	cpustate->PC += 2;
	return val;
}


INLINE int RBYTE(t11_state *cpustate, int addr)
{
	return cpustate->program->read_byte(addr);
}


INLINE void WBYTE(t11_state *cpustate, int addr, int data)
{
	cpustate->program->write_byte(addr, data);
}


INLINE int RWORD(t11_state *cpustate, int addr)
{
	return cpustate->program->read_word(addr & 0xfffe);
}


INLINE void WWORD(t11_state *cpustate, int addr, int data)
{
	cpustate->program->write_word(addr & 0xfffe, data);
}



/*************************************
 *
 *  Low-level stack operations
 *
 *************************************/

INLINE void PUSH(t11_state *cpustate, int val)
{
	cpustate->SP -= 2;
	WWORD(cpustate, cpustate->SPD, val);
}


INLINE int POP(t11_state *cpustate)
{
	int result = RWORD(cpustate, cpustate->SPD);
	cpustate->SP += 2;
	return result;
}



/*************************************
 *
 *  Flag definitions and operations
 *
 *************************************/

/* flag definitions */
#define CFLAG 1
#define VFLAG 2
#define ZFLAG 4
#define NFLAG 8

/* extracts flags */
#define GET_C (cpustate->PSW & CFLAG)
#define GET_V (cpustate->PSW & VFLAG)
#define GET_Z (cpustate->PSW & ZFLAG)
#define GET_N (cpustate->PSW & NFLAG)

/* clears flags */
#define CLR_C (cpustate->PSW &= ~CFLAG)
#define CLR_V (cpustate->PSW &= ~VFLAG)
#define CLR_Z (cpustate->PSW &= ~ZFLAG)
#define CLR_N (cpustate->PSW &= ~NFLAG)

/* sets flags */
#define SET_C (cpustate->PSW |= CFLAG)
#define SET_V (cpustate->PSW |= VFLAG)
#define SET_Z (cpustate->PSW |= ZFLAG)
#define SET_N (cpustate->PSW |= NFLAG)



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

struct irq_table_entry
{
	UINT8	priority;
	UINT8	vector;
};

static const struct irq_table_entry irq_table[] =
{
	{ 0<<5, 0x00 },
	{ 4<<5, 0x38 },
	{ 4<<5, 0x34 },
	{ 4<<5, 0x30 },
	{ 5<<5, 0x5c },
	{ 5<<5, 0x58 },
	{ 5<<5, 0x54 },
	{ 5<<5, 0x50 },
	{ 6<<5, 0x4c },
	{ 6<<5, 0x48 },
	{ 6<<5, 0x44 },
	{ 6<<5, 0x40 },
	{ 7<<5, 0x6c },
	{ 7<<5, 0x68 },
	{ 7<<5, 0x64 },
	{ 7<<5, 0x60 }
};

static void t11_check_irqs(t11_state *cpustate)
{
	const struct irq_table_entry *irq = &irq_table[cpustate->irq_state & 15];
	int priority = cpustate->PSW & 0xe0;

	/* compare the priority of the interrupt to the PSW */
	if (irq->priority > priority)
	{
		int vector = irq->vector;
		int new_pc, new_psw;

		/* call the callback; if we don't get -1 back, use the return value as our vector */
		if (cpustate->irq_callback != NULL)
		{
			int new_vector = (*cpustate->irq_callback)(cpustate->device, cpustate->irq_state & 15);
			if (new_vector != -1)
				vector = new_vector;
		}

		/* fetch the new PC and PSW from that vector */
		assert((vector & 3) == 0);
		new_pc = RWORD(cpustate, vector);
		new_psw = RWORD(cpustate, vector + 2);

		/* push the old state, set the new one */
		PUSH(cpustate, cpustate->PSW);
		PUSH(cpustate, cpustate->PC);
		cpustate->PCD = new_pc;
		cpustate->PSW = new_psw;
		t11_check_irqs(cpustate);

		/* count cycles and clear the WAIT flag */
		cpustate->icount -= 114;
		cpustate->wait_state = 0;
	}
}



/*************************************
 *
 *  Core opcodes
 *
 *************************************/

/* includes the static function prototypes and the master opcode table */
#include "t11table.c"

/* includes the actual opcode implementations */
#include "t11ops.c"



/*************************************
 *
 *  Low-level initialization/cleanup
 *
 *************************************/

static CPU_INIT( t11 )
{
	static const UINT16 initial_pc[] =
	{
		0xc000, 0x8000, 0x4000, 0x2000,
		0x1000, 0x0000, 0xf600, 0xf400
	};
	const struct t11_setup *setup = (const struct t11_setup *)device->static_config();
	t11_state *cpustate = get_safe_token(device);

	cpustate->initial_pc = initial_pc[setup->mode >> 13];
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();

	device->save_item(NAME(cpustate->ppc.w.l));
	device->save_item(NAME(cpustate->reg[0].w.l));
	device->save_item(NAME(cpustate->reg[1].w.l));
	device->save_item(NAME(cpustate->reg[2].w.l));
	device->save_item(NAME(cpustate->reg[3].w.l));
	device->save_item(NAME(cpustate->reg[4].w.l));
	device->save_item(NAME(cpustate->reg[5].w.l));
	device->save_item(NAME(cpustate->reg[6].w.l));
	device->save_item(NAME(cpustate->reg[7].w.l));
	device->save_item(NAME(cpustate->psw.w.l));
	device->save_item(NAME(cpustate->initial_pc));
	device->save_item(NAME(cpustate->wait_state));
	device->save_item(NAME(cpustate->irq_state));
}



/*************************************
 *
 *  CPU reset
 *
 *************************************/

static CPU_RESET( t11 )
{
	t11_state *cpustate = get_safe_token(device);

	/* initial SP is 376 octal, or 0xfe */
	cpustate->SP = 0x00fe;

	/* initial PC comes from the setup word */
	cpustate->PC = cpustate->initial_pc;

	/* PSW starts off at highest priority */
	cpustate->PSW = 0xe0;

	/* initialize the IRQ state */
	cpustate->irq_state = 0;

	/* reset the remaining state */
	cpustate->REGD(0) = 0;
	cpustate->REGD(1) = 0;
	cpustate->REGD(2) = 0;
	cpustate->REGD(3) = 0;
	cpustate->REGD(4) = 0;
	cpustate->REGD(5) = 0;
	cpustate->ppc.d = 0;
	cpustate->wait_state = 0;
}



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

static void set_irq_line(t11_state *cpustate, int irqline, int state)
{
	/* set the appropriate bit */
	if (state == CLEAR_LINE)
		cpustate->irq_state &= ~(1 << irqline);
	else
		cpustate->irq_state |= 1 << irqline;
}



/*************************************
 *
 *  Core execution
 *
 *************************************/

static CPU_EXECUTE( t11 )
{
	t11_state *cpustate = get_safe_token(device);

	t11_check_irqs(cpustate);

	if (cpustate->wait_state)
	{
		cpustate->icount = 0;
		goto getout;
	}

	do
	{
		UINT16 op;

		cpustate->ppc = cpustate->reg[7];	/* copy PC to previous PC */

		debugger_instruction_hook(device, cpustate->PCD);

		op = ROPCODE(cpustate);
		(*opcode_table[op >> 3])(cpustate, op);

	} while (cpustate->icount > 0);

getout:
	;
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( t11 )
{
	t11_state *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + T11_IRQ0:		set_irq_line(cpustate, T11_IRQ0, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + T11_IRQ1:		set_irq_line(cpustate, T11_IRQ1, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + T11_IRQ2:		set_irq_line(cpustate, T11_IRQ2, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + T11_IRQ3:		set_irq_line(cpustate, T11_IRQ3, info->i);		break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + T11_PC:				cpustate->PC = info->i; 						break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + T11_SP:				cpustate->SP = info->i;							break;
		case CPUINFO_INT_REGISTER + T11_PSW:			cpustate->PSW = info->i;						break;
		case CPUINFO_INT_REGISTER + T11_R0:				cpustate->REGW(0) = info->i;					break;
		case CPUINFO_INT_REGISTER + T11_R1:				cpustate->REGW(1) = info->i;					break;
		case CPUINFO_INT_REGISTER + T11_R2:				cpustate->REGW(2) = info->i;					break;
		case CPUINFO_INT_REGISTER + T11_R3:				cpustate->REGW(3) = info->i;					break;
		case CPUINFO_INT_REGISTER + T11_R4:				cpustate->REGW(4) = info->i;					break;
		case CPUINFO_INT_REGISTER + T11_R5:				cpustate->REGW(5) = info->i;					break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( t11 )
{
	t11_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(t11_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 4;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = -1;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 6;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 12;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 110;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + T11_IRQ0:		info->i = (cpustate->irq_state & 1) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + T11_IRQ1:		info->i = (cpustate->irq_state & 2) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + T11_IRQ2:		info->i = (cpustate->irq_state & 4) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + T11_IRQ3:		info->i = (cpustate->irq_state & 8) ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->ppc.w.l;			break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + T11_PC:				info->i = cpustate->PCD;				break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + T11_SP:				info->i = cpustate->SPD;				break;
		case CPUINFO_INT_REGISTER + T11_PSW:			info->i = cpustate->PSW;				break;
		case CPUINFO_INT_REGISTER + T11_R0:				info->i = cpustate->REGD(0);			break;
		case CPUINFO_INT_REGISTER + T11_R1:				info->i = cpustate->REGD(1);			break;
		case CPUINFO_INT_REGISTER + T11_R2:				info->i = cpustate->REGD(2);			break;
		case CPUINFO_INT_REGISTER + T11_R3:				info->i = cpustate->REGD(3);			break;
		case CPUINFO_INT_REGISTER + T11_R4:				info->i = cpustate->REGD(4);			break;
		case CPUINFO_INT_REGISTER + T11_R5:				info->i = cpustate->REGD(5);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(t11);			break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(t11);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(t11);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(t11);			break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(t11);	break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "T11");					break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "DEC T-11");			break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Aaron Giles"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				cpustate->psw.b.l & 0x80 ? '?':'.',
				cpustate->psw.b.l & 0x40 ? 'I':'.',
				cpustate->psw.b.l & 0x20 ? 'I':'.',
				cpustate->psw.b.l & 0x10 ? 'T':'.',
				cpustate->psw.b.l & 0x08 ? 'N':'.',
				cpustate->psw.b.l & 0x04 ? 'Z':'.',
				cpustate->psw.b.l & 0x02 ? 'V':'.',
				cpustate->psw.b.l & 0x01 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + T11_PC:				sprintf(info->s, "PC:%04X", cpustate->reg[7].w.l); break;
		case CPUINFO_STR_REGISTER + T11_SP:				sprintf(info->s, "SP:%04X", cpustate->reg[6].w.l); break;
		case CPUINFO_STR_REGISTER + T11_PSW:			sprintf(info->s, "PSW:%02X", cpustate->psw.b.l);   break;
		case CPUINFO_STR_REGISTER + T11_R0:				sprintf(info->s, "R0:%04X", cpustate->reg[0].w.l); break;
		case CPUINFO_STR_REGISTER + T11_R1:				sprintf(info->s, "R1:%04X", cpustate->reg[1].w.l); break;
		case CPUINFO_STR_REGISTER + T11_R2:				sprintf(info->s, "R2:%04X", cpustate->reg[2].w.l); break;
		case CPUINFO_STR_REGISTER + T11_R3:				sprintf(info->s, "R3:%04X", cpustate->reg[3].w.l); break;
		case CPUINFO_STR_REGISTER + T11_R4:				sprintf(info->s, "R4:%04X", cpustate->reg[4].w.l); break;
		case CPUINFO_STR_REGISTER + T11_R5:				sprintf(info->s, "R5:%04X", cpustate->reg[5].w.l); break;

		case CPUINFO_IS_OCTAL:							info->i = true;							break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(T11, t11);
