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
typedef struct
{
	SATURN_CONFIG *config;

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
	int 	(*irq_callback)(int irqline);	/* IRQ callback */
}	Saturn_Regs;

static int saturn_ICount = 0;

static Saturn_Regs saturn;

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

static void saturn_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	saturn.config = (SATURN_CONFIG *) config;
	saturn.irq_callback = irqcallback;

	state_save_register_item_array("saturn",index,saturn.reg[R0]);
	state_save_register_item_array("saturn",index,saturn.reg[R1]);
	state_save_register_item_array("saturn",index,saturn.reg[R2]);
	state_save_register_item_array("saturn",index,saturn.reg[R3]);
	state_save_register_item_array("saturn",index,saturn.reg[R4]);
	state_save_register_item_array("saturn",index,saturn.reg[A]);
	state_save_register_item_array("saturn",index,saturn.reg[B]);
	state_save_register_item_array("saturn",index,saturn.reg[C]);
	state_save_register_item_array("saturn",index,saturn.reg[D]);
	state_save_register_item_array("saturn",index,saturn.d);
	state_save_register_item("saturn",index,saturn.pc);
	state_save_register_item("saturn",index,saturn.oldpc);
	state_save_register_item_array("saturn",index,saturn.rstk);
	state_save_register_item("saturn",index,saturn.out);
	state_save_register_item("saturn",index,saturn.carry);
	state_save_register_item("saturn",index,saturn.st);
	state_save_register_item("saturn",index,saturn.hst);
	state_save_register_item("saturn",index,saturn.nmi_state);
	state_save_register_item("saturn",index,saturn.irq_state);
	state_save_register_item("saturn",index,saturn.irq_enable);
	state_save_register_item("saturn",index,saturn.in_irq);
	state_save_register_item("saturn",index,saturn.pending_irq);
	state_save_register_item("saturn",index,saturn.sleeping);
}

static void saturn_reset(void)
{
	saturn.pc=0;
	saturn.sleeping = 0;
	saturn.irq_enable = 0;
	saturn.in_irq = 0;
	change_pc(saturn.pc);
}

static void saturn_get_context (void *dst)
{
	if( dst )
		*(Saturn_Regs*)dst = saturn;
}

static void saturn_set_context (void *src)
{
	if( src )
	{
		saturn = *(Saturn_Regs*)src;
		change_pc(saturn.pc);
	}
}



INLINE void saturn_take_irq(void)
{
	saturn.in_irq = 1;       /* reset by software, using RTI */
	saturn.pending_irq = 0;
	saturn_ICount -= 7;
	saturn_push(saturn.pc);
	saturn.pc=IRQ_ADDRESS;

	LOG(("Saturn#%d takes IRQ ($%04x)\n", cpu_getactivecpu(), saturn.pc));

	if (saturn.irq_callback) (*saturn.irq_callback)(SATURN_IRQ_LINE);
	change_pc(saturn.pc);
}

static int saturn_execute(int cycles)
{
	saturn_ICount = cycles;

	change_pc(saturn.pc);

	do
	{
		saturn.oldpc = saturn.pc;

		debugger_instruction_hook(Machine, saturn.pc);

		if ( saturn.sleeping )
		{
			/* advance time when sleeping */
			saturn_ICount -= 100;
		}
		else
		{
			/* takes irq */
			if ( saturn.pending_irq && (!saturn.in_irq) )
				saturn_take_irq();

			/* execute one instruction */
			saturn_instruction();
		}

	} while (saturn_ICount > 0);

	return cycles - saturn_ICount;
}


static void saturn_set_nmi_line(int state)
{
	if ( state == saturn.nmi_state ) return;
	saturn.nmi_state = state;
	if ( state != CLEAR_LINE )
	{
		LOG(( "SATURN#%d set_nmi_line(ASSERT)\n", cpu_getactivecpu()));
		saturn.pending_irq = 1;
	}
}

static void saturn_set_irq_line(int state)
{
	if ( state == saturn.irq_state ) return;
	saturn.irq_state = state;
	if ( state != CLEAR_LINE && saturn.irq_enable )
	{
		LOG(( "SATURN#%d set_irq_line(ASSERT)\n", cpu_getactivecpu()));
		saturn.pending_irq = 1;
	}
}

static void saturn_set_wakeup_line(int state)
{
	if (saturn.sleeping && state==1)
	{
		LOG(( "SATURN#%d set_wakeup_line(ASSERT)\n", cpu_getactivecpu()));
		if (saturn.irq_callback) (*saturn.irq_callback)(SATURN_WAKEUP_LINE);
		saturn.sleeping = 0;
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


static void saturn_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
	        case CPUINFO_INT_INPUT_STATE + SATURN_NMI_LINE:	        saturn_set_nmi_line(info->i);	break;
 	        case CPUINFO_INT_INPUT_STATE + SATURN_IRQ_LINE:	        saturn_set_irq_line(info->i);	break;
 	        case CPUINFO_INT_INPUT_STATE + SATURN_WAKEUP_LINE:	saturn_set_wakeup_line(info->i);	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SATURN_PC:			saturn.pc = info->i; change_pc(saturn.pc);				break;
		case CPUINFO_INT_REGISTER + SATURN_D0:			saturn.d[0] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_D1:			saturn.d[1] = info->i;							break;
  	        case CPUINFO_INT_REGISTER + SATURN_A:			IntReg64(saturn.reg[A], info->i);							break;
  	        case CPUINFO_INT_REGISTER + SATURN_B:			IntReg64(saturn.reg[B], info->i);							break;
  	        case CPUINFO_INT_REGISTER + SATURN_C:			IntReg64(saturn.reg[C], info->i);							break;
  	        case CPUINFO_INT_REGISTER + SATURN_D:			IntReg64(saturn.reg[D], info->i);							break;
  	        case CPUINFO_INT_REGISTER + SATURN_R0:			IntReg64(saturn.reg[R0], info->i);							break;
  	        case CPUINFO_INT_REGISTER + SATURN_R1:			IntReg64(saturn.reg[R1], info->i);							break;
  	        case CPUINFO_INT_REGISTER + SATURN_R2:			IntReg64(saturn.reg[R2], info->i);							break;
  	        case CPUINFO_INT_REGISTER + SATURN_R3:			IntReg64(saturn.reg[R3], info->i);							break;
  	        case CPUINFO_INT_REGISTER + SATURN_R4:			IntReg64(saturn.reg[R4], info->i);							break;
		case CPUINFO_INT_REGISTER + SATURN_P:			saturn.p = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_OUT:			saturn.out = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_CARRY:		saturn.carry = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_ST:			saturn.st = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_HST:			saturn.hst = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK0:		saturn.rstk[0] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK1:		saturn.rstk[1] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK2:		saturn.rstk[2] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK3:		saturn.rstk[3] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK4:		saturn.rstk[4] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK5:		saturn.rstk[5] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK6:		saturn.rstk[6] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK7:		saturn.rstk[7] = info->i;							break;
                case CPUINFO_INT_REGISTER + SATURN_SLEEPING:		saturn.sleeping = info->i;
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

void saturn_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(saturn);				break;
		case CPUINFO_INT_INPUT_LINES:						info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 20; /* 20 nibbles max */		break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 21;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 20;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + SATURN_NMI_LINE:	        info->i = saturn.nmi_state;				break;
		case CPUINFO_INT_INPUT_STATE + SATURN_IRQ_LINE:	        info->i = saturn.irq_state;				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = saturn.oldpc;					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SATURN_PC:			info->i = saturn.pc;					break;
		case CPUINFO_INT_REGISTER + SATURN_D0:			info->i = saturn.d[0];					break;
		case CPUINFO_INT_REGISTER + SATURN_D1:			info->i = saturn.d[1];					break;

     	        case CPUINFO_INT_REGISTER + SATURN_A:			info->i = Reg64Int(saturn.reg[A]);				break;
                case CPUINFO_INT_REGISTER + SATURN_B:			info->i = Reg64Int(saturn.reg[B]);				break;
                case CPUINFO_INT_REGISTER + SATURN_C:			info->i = Reg64Int(saturn.reg[C]);				break;
                case CPUINFO_INT_REGISTER + SATURN_D:			info->i = Reg64Int(saturn.reg[D]);				break;
                case CPUINFO_INT_REGISTER + SATURN_R0:			info->i = Reg64Int(saturn.reg[R0]);				break;
	        case CPUINFO_INT_REGISTER + SATURN_R1:			info->i = Reg64Int(saturn.reg[R1]);				break;
       	        case CPUINFO_INT_REGISTER + SATURN_R2:			info->i = Reg64Int(saturn.reg[R2]);				break;
	        case CPUINFO_INT_REGISTER + SATURN_R3:			info->i = Reg64Int(saturn.reg[R3]);				break;
	        case CPUINFO_INT_REGISTER + SATURN_R4:			info->i = Reg64Int(saturn.reg[R4]);				break;

		case CPUINFO_INT_REGISTER + SATURN_P:			info->i = saturn.p;						break;
		case CPUINFO_INT_REGISTER + SATURN_OUT:			info->i = saturn.out;					break;
		case CPUINFO_INT_REGISTER + SATURN_CARRY:		info->i = saturn.carry;					break;
		case CPUINFO_INT_REGISTER + SATURN_ST:			info->i = saturn.st;					break;
		case CPUINFO_INT_REGISTER + SATURN_HST:			info->i = saturn.hst;					break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK0:		info->i = saturn.rstk[0];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK1:		info->i = saturn.rstk[1];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK2:		info->i = saturn.rstk[2];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK3:		info->i = saturn.rstk[3];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK4:		info->i = saturn.rstk[4];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK5:		info->i = saturn.rstk[5];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK6:		info->i = saturn.rstk[6];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK7:		info->i = saturn.rstk[7];				break;
	        case CPUINFO_INT_REGISTER + SATURN_SLEEPING:		info->i = saturn.sleeping;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = saturn_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = saturn_get_context;		break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = saturn_set_context;		break;
		case CPUINFO_PTR_INIT:							info->init = saturn_init;					break;
		case CPUINFO_PTR_RESET:							info->reset = saturn_reset;					break;
		case CPUINFO_PTR_EXECUTE:						info->execute = saturn_execute;				break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;							break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = saturn_dasm;		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &saturn_ICount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s = cpuintrf_temp_str(), "Saturn");	break;
		case CPUINFO_STR_CORE_FAMILY: 					strcpy(info->s = cpuintrf_temp_str(), "Saturn");	break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s = cpuintrf_temp_str(), "1.0alpha");	break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s = cpuintrf_temp_str(), __FILE__);	break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s = cpuintrf_temp_str(), "Copyright Peter Trauner, all rights reserved.");	break;

		case CPUINFO_STR_REGISTER + SATURN_PC:		sprintf(info->s = cpuintrf_temp_str(), "PC:   %.5x", saturn.pc);break;
		case CPUINFO_STR_REGISTER + SATURN_D0:		sprintf(info->s = cpuintrf_temp_str(), "D0:   %.5x", saturn.d[0]);break;
		case CPUINFO_STR_REGISTER + SATURN_D1:		sprintf(info->s = cpuintrf_temp_str(), "D1:   %.5x", saturn.d[1]);break;
       	        case CPUINFO_STR_REGISTER + SATURN_A:		sprintf(info->s = cpuintrf_temp_str(), "A: " Reg64Format, Reg64Data(saturn.reg[A]));break;
       	        case CPUINFO_STR_REGISTER + SATURN_B:		sprintf(info->s = cpuintrf_temp_str(), "B: " Reg64Format, Reg64Data(saturn.reg[B]));break;
       	        case CPUINFO_STR_REGISTER + SATURN_C:		sprintf(info->s = cpuintrf_temp_str(), "C: " Reg64Format, Reg64Data(saturn.reg[C]));break;
       	        case CPUINFO_STR_REGISTER + SATURN_D:		sprintf(info->s = cpuintrf_temp_str(), "D: " Reg64Format, Reg64Data(saturn.reg[D]));break;
       	        case CPUINFO_STR_REGISTER + SATURN_R0:		sprintf(info->s = cpuintrf_temp_str(), "R0: " Reg64Format, Reg64Data(saturn.reg[R0]));break;
       	        case CPUINFO_STR_REGISTER + SATURN_R1:		sprintf(info->s = cpuintrf_temp_str(), "R1: " Reg64Format, Reg64Data(saturn.reg[R1]));break;
       	        case CPUINFO_STR_REGISTER + SATURN_R2:		sprintf(info->s = cpuintrf_temp_str(), "R2: " Reg64Format, Reg64Data(saturn.reg[R2]));break;
       	        case CPUINFO_STR_REGISTER + SATURN_R3:		sprintf(info->s = cpuintrf_temp_str(), "R3: " Reg64Format, Reg64Data(saturn.reg[R3]));break;
       	        case CPUINFO_STR_REGISTER + SATURN_R4:		sprintf(info->s = cpuintrf_temp_str(), "R4: " Reg64Format, Reg64Data(saturn.reg[R4]));break;
		case CPUINFO_STR_REGISTER + SATURN_P:		sprintf(info->s = cpuintrf_temp_str(), "P:%x", saturn.p);break;
		case CPUINFO_STR_REGISTER + SATURN_OUT:		sprintf(info->s = cpuintrf_temp_str(), "OUT:%.3x", saturn.out);break;
		case CPUINFO_STR_REGISTER + SATURN_CARRY:	sprintf(info->s = cpuintrf_temp_str(), "Carry: %d", saturn.carry);break;
		case CPUINFO_STR_REGISTER + SATURN_ST:		sprintf(info->s = cpuintrf_temp_str(), "ST:%.4x", saturn.st);break;
		case CPUINFO_STR_REGISTER + SATURN_HST:		sprintf(info->s = cpuintrf_temp_str(), "HST:%x", saturn.hst);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK0:	sprintf(info->s = cpuintrf_temp_str(), "RSTK0:%.5x", saturn.rstk[0]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK1:	sprintf(info->s = cpuintrf_temp_str(), "RSTK1:%.5x", saturn.rstk[1]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK2:	sprintf(info->s = cpuintrf_temp_str(), "RSTK2:%.5x", saturn.rstk[2]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK3:	sprintf(info->s = cpuintrf_temp_str(), "RSTK3:%.5x", saturn.rstk[3]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK4:	sprintf(info->s = cpuintrf_temp_str(), "RSTK4:%.5x", saturn.rstk[4]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK5:	sprintf(info->s = cpuintrf_temp_str(), "RSTK5:%.5x", saturn.rstk[5]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK6:	sprintf(info->s = cpuintrf_temp_str(), "RSTK6:%.5x", saturn.rstk[6]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK7:	sprintf(info->s = cpuintrf_temp_str(), "RSTK7:%.5x", saturn.rstk[7]);break;
 	        case CPUINFO_STR_REGISTER + SATURN_IRQ_STATE:	sprintf(info->s = cpuintrf_temp_str(), "IRQ:%c%c%c%i", saturn.in_irq?'S':'.', saturn.irq_enable?'e':'.', saturn.pending_irq?'p':'.', saturn.irq_state); break;
 	        case CPUINFO_STR_FLAGS:				sprintf(info->s = cpuintrf_temp_str(), "%c%c", saturn.decimal?'D':'.', saturn.carry ? 'C':'.'); break;
 	        case CPUINFO_STR_REGISTER + SATURN_SLEEPING:	sprintf(info->s = cpuintrf_temp_str(), "sleep:%c", saturn.sleeping?'S':'.'); break;
	}
}
