/*****************************************************************************
 *
 *   saturn.c
 *   portable saturn emulator interface
 *   (hp calculators)
 *
 *   Copyright Peter Trauner, all rights reserved.
 *
 *   Modified by Antoine Mine'
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     peter.trauner@jk.uni-linz.ac.at
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"

#include "saturn.h"

#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define A 5
#define B 6
#define C 7
#define D 8
#define I 9 // invalid

typedef UINT32 SaturnAdr;   // 20 bit, packed
typedef UINT8  SaturnNib;   // 4 bit

// 64 bit, unpacked (one nibble per byte)
typedef SaturnNib Saturn64[16];

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)


/****************************************************************************
 * The SATURN registers.
 ****************************************************************************/
typedef struct _saturn_state saturn_state;
struct _saturn_state
{
	saturn_cpu_core *config;

	Saturn64 reg[9]; //r0,r1,r2,r3,r4,a,b,c,d

	SaturnAdr d[2], pc, oldpc, rstk[8]; // 20 bit addresses

	SaturnNib p; // 4 bit pointer

        UINT16 out; // 12 bit (packed)
	UINT8  carry, decimal;
	UINT16 st; // status 16 bit

	SaturnNib hst; // hardware status 4 bit
#define XM 1 // external Modules missing
#define SB 2 // Sticky bit
#define SR 4 // Service Request
#define MP 8 // Module Pulled

	UINT8	nmi_state;
	UINT8   irq_state;
	UINT8   irq_enable;     /* INTON / INTOFF */
	UINT8   in_irq;         /* already servicing IRQ */
	UINT8	pending_irq;	/* IRQ is pending */
	UINT8   sleeping;       /* low-consumption state */
	int 	monitor_id;
	int		monitor_in;
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	int icount;
};

INLINE saturn_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SATURN);
	return (saturn_state *)downcast<legacy_cpu_device *>(device)->token();
}

/***************************************************************
 * include the opcode macros, functions and tables
 ***************************************************************/

#include "satops.c"
#include "sattable.c"

/*****************************************************************************
 *
 *      Saturn CPU interface functions
 *
 *****************************************************************************/

static CPU_INIT( saturn )
{
	saturn_state *cpustate = get_safe_token(device);

	cpustate->config = (saturn_cpu_core *) device->static_config();
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();

	device->save_item(NAME(cpustate->reg[R0]));
	device->save_item(NAME(cpustate->reg[R1]));
	device->save_item(NAME(cpustate->reg[R2]));
	device->save_item(NAME(cpustate->reg[R3]));
	device->save_item(NAME(cpustate->reg[R4]));
	device->save_item(NAME(cpustate->reg[A]));
	device->save_item(NAME(cpustate->reg[B]));
	device->save_item(NAME(cpustate->reg[C]));
	device->save_item(NAME(cpustate->reg[D]));
	device->save_item(NAME(cpustate->d));
	device->save_item(NAME(cpustate->pc));
	device->save_item(NAME(cpustate->oldpc));
	device->save_item(NAME(cpustate->rstk));
	device->save_item(NAME(cpustate->out));
	device->save_item(NAME(cpustate->carry));
	device->save_item(NAME(cpustate->st));
	device->save_item(NAME(cpustate->hst));
	device->save_item(NAME(cpustate->nmi_state));
	device->save_item(NAME(cpustate->irq_state));
	device->save_item(NAME(cpustate->irq_enable));
	device->save_item(NAME(cpustate->in_irq));
	device->save_item(NAME(cpustate->pending_irq));
	device->save_item(NAME(cpustate->sleeping));
}

static CPU_RESET( saturn )
{
	saturn_state *cpustate = get_safe_token(device);

	cpustate->pc=0;
	cpustate->sleeping = 0;
	cpustate->irq_enable = 0;
	cpustate->in_irq = 0;
}


INLINE void saturn_take_irq(saturn_state *cpustate)
{
	cpustate->in_irq = 1;       /* reset by software, using RTI */
	cpustate->pending_irq = 0;
	cpustate->icount -= 7;
	saturn_push(cpustate, cpustate->pc);
	cpustate->pc=IRQ_ADDRESS;

	LOG(("Saturn '%s' takes IRQ ($%04x)\n", cpustate->device->tag(), cpustate->pc));

	if (cpustate->irq_callback) (*cpustate->irq_callback)(cpustate->device, SATURN_IRQ_LINE);
}

static CPU_EXECUTE( saturn )
{
	saturn_state *cpustate = get_safe_token(device);

	do
	{
		cpustate->oldpc = cpustate->pc;

		debugger_instruction_hook(device, cpustate->pc);

		if ( cpustate->sleeping )
		{
			/* advance time when sleeping */
			cpustate->icount -= 100;
		}
		else
		{
			/* takes irq */
			if ( cpustate->pending_irq && (!cpustate->in_irq) )
				saturn_take_irq(cpustate);

			/* execute one instruction */
			saturn_instruction(cpustate);
		}

	} while (cpustate->icount > 0);
}


static void saturn_set_nmi_line(saturn_state *cpustate, int state)
{
	if ( state == cpustate->nmi_state ) return;
	cpustate->nmi_state = state;
	if ( state != CLEAR_LINE )
	{
		LOG(( "SATURN '%s' set_nmi_line(ASSERT)\n", cpustate->device->tag()));
		cpustate->pending_irq = 1;
	}
}

static void saturn_set_irq_line(saturn_state *cpustate, int state)
{
	if ( state == cpustate->irq_state ) return;
	cpustate->irq_state = state;
	if ( state != CLEAR_LINE && cpustate->irq_enable )
	{
		LOG(( "SATURN '%s' set_irq_line(ASSERT)\n", cpustate->device->tag()));
		cpustate->pending_irq = 1;
	}
}

static void saturn_set_wakeup_line(saturn_state *cpustate, int state)
{
	if (cpustate->sleeping && state==1)
	{
		LOG(( "SATURN '%s' set_wakeup_line(ASSERT)\n", cpustate->device->tag()));
		if (cpustate->irq_callback) (*cpustate->irq_callback)(cpustate->device, SATURN_WAKEUP_LINE);
		cpustate->sleeping = 0;
	}
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void IntReg64(Saturn64 r, INT64 d)
{
	int i;
	for (i=0; i<16; i++)
		r[i] = (d >> (4*i)) & 0xf;
}


static CPU_SET_INFO( saturn )
{
	saturn_state *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
	        case CPUINFO_INT_INPUT_STATE + SATURN_NMI_LINE:	        saturn_set_nmi_line(cpustate, info->i);	break;
	        case CPUINFO_INT_INPUT_STATE + SATURN_IRQ_LINE:	        saturn_set_irq_line(cpustate, info->i);	break;
	        case CPUINFO_INT_INPUT_STATE + SATURN_WAKEUP_LINE:	saturn_set_wakeup_line(cpustate, info->i);	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SATURN_PC:			cpustate->pc = info->i; 							break;
		case CPUINFO_INT_REGISTER + SATURN_D0:			cpustate->d[0] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_D1:			cpustate->d[1] = info->i;							break;
	        case CPUINFO_INT_REGISTER + SATURN_A:			IntReg64(cpustate->reg[A], info->i);							break;
	        case CPUINFO_INT_REGISTER + SATURN_B:			IntReg64(cpustate->reg[B], info->i);							break;
	        case CPUINFO_INT_REGISTER + SATURN_C:			IntReg64(cpustate->reg[C], info->i);							break;
	        case CPUINFO_INT_REGISTER + SATURN_D:			IntReg64(cpustate->reg[D], info->i);							break;
	        case CPUINFO_INT_REGISTER + SATURN_R0:			IntReg64(cpustate->reg[R0], info->i);							break;
	        case CPUINFO_INT_REGISTER + SATURN_R1:			IntReg64(cpustate->reg[R1], info->i);							break;
	        case CPUINFO_INT_REGISTER + SATURN_R2:			IntReg64(cpustate->reg[R2], info->i);							break;
	        case CPUINFO_INT_REGISTER + SATURN_R3:			IntReg64(cpustate->reg[R3], info->i);							break;
	        case CPUINFO_INT_REGISTER + SATURN_R4:			IntReg64(cpustate->reg[R4], info->i);							break;
		case CPUINFO_INT_REGISTER + SATURN_P:			cpustate->p = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_OUT:			cpustate->out = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_CARRY:		cpustate->carry = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_ST:			cpustate->st = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_HST:			cpustate->hst = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK0:		cpustate->rstk[0] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK1:		cpustate->rstk[1] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK2:		cpustate->rstk[2] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK3:		cpustate->rstk[3] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK4:		cpustate->rstk[4] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK5:		cpustate->rstk[5] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK6:		cpustate->rstk[6] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK7:		cpustate->rstk[7] = info->i;							break;
                case CPUINFO_INT_REGISTER + SATURN_SLEEPING:		cpustate->sleeping = info->i;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

#define Reg64Data(s) s[15],s[14],s[13],s[12],s[11],s[10],s[9],s[8],s[7],s[6],s[5],s[4],s[3],s[2],s[1],s[0]
#define Reg64Format "%x %x%x%x%x%x%x%x %x%x%x %x%x%x%x%x"

static INT64 Reg64Int(Saturn64 r)
{
	INT64 x = 0;
	int i;
	for (i=0; i<16; i++)
		x |= (INT64) r[i] << (4*i);
	return x;
}

CPU_GET_INFO( saturn )
{
	saturn_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(saturn_state);				break;
		case CPUINFO_INT_INPUT_LINES:						info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 20; /* 20 nibbles max */		break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 21;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 20;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + SATURN_NMI_LINE:	        info->i = cpustate->nmi_state;				break;
		case CPUINFO_INT_INPUT_STATE + SATURN_IRQ_LINE:	        info->i = cpustate->irq_state;				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->oldpc;					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SATURN_PC:			info->i = cpustate->pc;					break;
		case CPUINFO_INT_REGISTER + SATURN_D0:			info->i = cpustate->d[0];					break;
		case CPUINFO_INT_REGISTER + SATURN_D1:			info->i = cpustate->d[1];					break;

    	        case CPUINFO_INT_REGISTER + SATURN_A:			info->i = Reg64Int(cpustate->reg[A]);				break;
                case CPUINFO_INT_REGISTER + SATURN_B:			info->i = Reg64Int(cpustate->reg[B]);				break;
                case CPUINFO_INT_REGISTER + SATURN_C:			info->i = Reg64Int(cpustate->reg[C]);				break;
                case CPUINFO_INT_REGISTER + SATURN_D:			info->i = Reg64Int(cpustate->reg[D]);				break;
                case CPUINFO_INT_REGISTER + SATURN_R0:			info->i = Reg64Int(cpustate->reg[R0]);				break;
	        case CPUINFO_INT_REGISTER + SATURN_R1:			info->i = Reg64Int(cpustate->reg[R1]);				break;
    	        case CPUINFO_INT_REGISTER + SATURN_R2:			info->i = Reg64Int(cpustate->reg[R2]);				break;
	        case CPUINFO_INT_REGISTER + SATURN_R3:			info->i = Reg64Int(cpustate->reg[R3]);				break;
	        case CPUINFO_INT_REGISTER + SATURN_R4:			info->i = Reg64Int(cpustate->reg[R4]);				break;

		case CPUINFO_INT_REGISTER + SATURN_P:			info->i = cpustate->p;						break;
		case CPUINFO_INT_REGISTER + SATURN_OUT:			info->i = cpustate->out;					break;
		case CPUINFO_INT_REGISTER + SATURN_CARRY:		info->i = cpustate->carry;					break;
		case CPUINFO_INT_REGISTER + SATURN_ST:			info->i = cpustate->st;					break;
		case CPUINFO_INT_REGISTER + SATURN_HST:			info->i = cpustate->hst;					break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK0:		info->i = cpustate->rstk[0];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK1:		info->i = cpustate->rstk[1];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK2:		info->i = cpustate->rstk[2];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK3:		info->i = cpustate->rstk[3];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK4:		info->i = cpustate->rstk[4];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK5:		info->i = cpustate->rstk[5];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK6:		info->i = cpustate->rstk[6];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK7:		info->i = cpustate->rstk[7];				break;
	        case CPUINFO_INT_REGISTER + SATURN_SLEEPING:		info->i = cpustate->sleeping;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(saturn);			break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(saturn);					break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(saturn);					break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(saturn);				break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;							break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(saturn);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Saturn");	break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Saturn");	break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0alpha");	break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);	break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Peter Trauner, all rights reserved.");	break;

		case CPUINFO_STR_REGISTER + SATURN_PC:		sprintf(info->s, "PC:   %.5x", cpustate->pc);break;
		case CPUINFO_STR_REGISTER + SATURN_D0:		sprintf(info->s, "D0:   %.5x", cpustate->d[0]);break;
		case CPUINFO_STR_REGISTER + SATURN_D1:		sprintf(info->s, "D1:   %.5x", cpustate->d[1]);break;
    	        case CPUINFO_STR_REGISTER + SATURN_A:		sprintf(info->s, "A: " Reg64Format, Reg64Data(cpustate->reg[A]));break;
    	        case CPUINFO_STR_REGISTER + SATURN_B:		sprintf(info->s, "B: " Reg64Format, Reg64Data(cpustate->reg[B]));break;
    	        case CPUINFO_STR_REGISTER + SATURN_C:		sprintf(info->s, "C: " Reg64Format, Reg64Data(cpustate->reg[C]));break;
    	        case CPUINFO_STR_REGISTER + SATURN_D:		sprintf(info->s, "D: " Reg64Format, Reg64Data(cpustate->reg[D]));break;
    	        case CPUINFO_STR_REGISTER + SATURN_R0:		sprintf(info->s, "R0: " Reg64Format, Reg64Data(cpustate->reg[R0]));break;
    	        case CPUINFO_STR_REGISTER + SATURN_R1:		sprintf(info->s, "R1: " Reg64Format, Reg64Data(cpustate->reg[R1]));break;
    	        case CPUINFO_STR_REGISTER + SATURN_R2:		sprintf(info->s, "R2: " Reg64Format, Reg64Data(cpustate->reg[R2]));break;
    	        case CPUINFO_STR_REGISTER + SATURN_R3:		sprintf(info->s, "R3: " Reg64Format, Reg64Data(cpustate->reg[R3]));break;
    	        case CPUINFO_STR_REGISTER + SATURN_R4:		sprintf(info->s, "R4: " Reg64Format, Reg64Data(cpustate->reg[R4]));break;
		case CPUINFO_STR_REGISTER + SATURN_P:		sprintf(info->s, "P:%x", cpustate->p);break;
		case CPUINFO_STR_REGISTER + SATURN_OUT:		sprintf(info->s, "OUT:%.3x", cpustate->out);break;
		case CPUINFO_STR_REGISTER + SATURN_CARRY:	sprintf(info->s, "Carry: %d", cpustate->carry);break;
		case CPUINFO_STR_REGISTER + SATURN_ST:		sprintf(info->s, "ST:%.4x", cpustate->st);break;
		case CPUINFO_STR_REGISTER + SATURN_HST:		sprintf(info->s, "HST:%x", cpustate->hst);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK0:	sprintf(info->s, "RSTK0:%.5x", cpustate->rstk[0]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK1:	sprintf(info->s, "RSTK1:%.5x", cpustate->rstk[1]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK2:	sprintf(info->s, "RSTK2:%.5x", cpustate->rstk[2]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK3:	sprintf(info->s, "RSTK3:%.5x", cpustate->rstk[3]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK4:	sprintf(info->s, "RSTK4:%.5x", cpustate->rstk[4]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK5:	sprintf(info->s, "RSTK5:%.5x", cpustate->rstk[5]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK6:	sprintf(info->s, "RSTK6:%.5x", cpustate->rstk[6]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK7:	sprintf(info->s, "RSTK7:%.5x", cpustate->rstk[7]);break;
	        case CPUINFO_STR_REGISTER + SATURN_IRQ_STATE:	sprintf(info->s, "IRQ:%c%c%c%i", cpustate->in_irq?'S':'.', cpustate->irq_enable?'e':'.', cpustate->pending_irq?'p':'.', cpustate->irq_state); break;
	        case CPUINFO_STR_FLAGS:				sprintf(info->s, "%c%c", cpustate->decimal?'D':'.', cpustate->carry ? 'C':'.'); break;
	        case CPUINFO_STR_REGISTER + SATURN_SLEEPING:	sprintf(info->s, "sleep:%c", cpustate->sleeping?'S':'.'); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(SATURN, saturn);
