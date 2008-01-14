/*****************************************************************************
 *
 *   sc61860.c
 *   portable sharp 61860 emulator interface
 *   (sharp pocket computers)
 *
 *   Copyright Peter Trauner, all rights reserved.
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
 * History of changes:
 * 29.7.2001 Several changes listed below taken by Mario Konegger
 *           (konegger@itp.tu-graz.ac.at)
 *           Added 0x7f to set_reg, to prevent p,q,r, overflow.
 *         Changed 512ms timerinterval from 256 to 128, thus the
 *         duration of one period is 512ms.
 *         Extended execute procudure with HLT-mode of CPU.
 *****************************************************************************/

#include "cpuintrf.h"
#include "debugger.h"

#include "sc61860.h"
#include "sc.h"

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

/****************************************************************************
 * The 61860 registers.
 ****************************************************************************/
typedef struct
{
    SC61860_CONFIG *config;
    UINT8 ram[0x60]; // internal special ram
    UINT8 p, q, r; //7 bits only?

    UINT8 c;        // port c, used for HLT.
    UINT8 d, h;
    UINT16 oldpc, pc, dp;

    int carry, zero;

    struct { int t2ms, t512ms; int count;} timer;
}   SC61860_Regs;

static int sc61860_ICount = 0;

static SC61860_Regs sc61860;

UINT8 *sc61860_internal_ram(void) { return sc61860.ram; }

static TIMER_CALLBACK(sc61860_2ms_tick)
{
	if (--sc61860.timer.count == 0)
	{
		sc61860.timer.count = 128;
		sc61860.timer.t512ms = !sc61860.timer.t512ms;
	}
	sc61860.timer.t2ms = !sc61860.timer.t2ms;
}

/***************************************************************
 * include the opcode macros, functions and tables
 ***************************************************************/
#include "scops.c"
#include "sctable.c"

static void sc61860_reset(void)
{
	sc61860.timer.t2ms=0;
	sc61860.timer.t512ms=0;
	sc61860.timer.count=256;
	sc61860.pc=0;
	change_pc(sc61860.pc);
}

static void sc61860_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	sc61860.config = (SC61860_CONFIG *) config;
	timer_pulse(ATTOTIME_IN_HZ(500), NULL, 0, sc61860_2ms_tick);
}

static void sc61860_get_context (void *dst)
{
	if( dst )
		*(SC61860_Regs*)dst = sc61860;
}

static void sc61860_set_context (void *src)
{
	if( src )
	{
		sc61860 = *(SC61860_Regs*)src;
		change_pc(sc61860.pc);
	}
}

static int sc61860_execute(int cycles)
{
	sc61860_ICount = cycles;

	change_pc(sc61860.pc);

	do
	{
		sc61860.oldpc = sc61860.pc;

		CALL_MAME_DEBUG;

		sc61860_instruction();

               /* Are we in HLT-mode? */
               /*if (sc61860.c & 4)
         {
         if ((sc61860.config && sc61860.config->ina && (sc61860.config->ina()!=0)) || sc61860.timer.t512ms)
         {
                 sc61860.c&=0xfb;
                 if (sc61860.config->outc) sc61860.config->outc(sc61860.c);
         }
         sc61860_ICount-=4;
         }
         else if(sc61860.c & 8) {}

         else sc61860_instruction();*/

	} while (sc61860_ICount > 0);

	return cycles - sc61860_ICount;
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void sc61860_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SC61860_PC:			sc61860.pc = info->i; change_pc(sc61860.pc); break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + SC61860_R:			sc61860.r = info->i & 0x7F;						break;
		case CPUINFO_INT_REGISTER + SC61860_DP:			sc61860.dp = info->i;			break;
		case CPUINFO_INT_REGISTER + SC61860_P:			sc61860.p = info->i & 0x7F;		break;
		case CPUINFO_INT_REGISTER + SC61860_Q:			sc61860.q = info->i & 0x7F;		break;
		case CPUINFO_INT_REGISTER + SC61860_CARRY:		sc61860.carry = info->i;			break;
		case CPUINFO_INT_REGISTER + SC61860_ZERO:		sc61860.zero = info->i;			break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void sc61860_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(sc61860);				break;
		case CPUINFO_INT_INPUT_LINES:						info->i = 0;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 4;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = sc61860.oldpc;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SC61860_PC:				info->i =  sc61860.pc;						break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + SC61860_R:				info->i =  sc61860.r;						break;
		case CPUINFO_INT_REGISTER + SC61860_DP:				info->i =  sc61860.dp;						break;
		case CPUINFO_INT_REGISTER + SC61860_P:				info->i =  sc61860.p;						break;
		case CPUINFO_INT_REGISTER + SC61860_Q:				info->i =  sc61860.q;						break;
		case CPUINFO_INT_REGISTER + SC61860_CARRY:			info->i =  sc61860.carry;						break;
		case CPUINFO_INT_REGISTER + SC61860_ZERO:			info->i =  sc61860.zero;						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = sc61860_set_info;				break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = sc61860_get_context;			break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = sc61860_set_context;			break;
		case CPUINFO_PTR_INIT:							info->init = sc61860_init;					break;
		case CPUINFO_PTR_RESET:							info->reset = sc61860_reset;					break;
		case CPUINFO_PTR_EXECUTE:						info->execute = sc61860_execute;				break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = sc61860_dasm;					break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &sc61860_ICount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s = cpuintrf_temp_str(), "SC61860"); break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s = cpuintrf_temp_str(), "SC61860"); break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s = cpuintrf_temp_str(), "1.0beta"); break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s = cpuintrf_temp_str(), __FILE__); break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s = cpuintrf_temp_str(), "Copyright Peter Trauner, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s = cpuintrf_temp_str(), "%c%c", sc61860.zero?'Z':'.', sc61860.carry ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + SC61860_PC:		sprintf(info->s = cpuintrf_temp_str(), "PC:%.4x", sc61860.pc);break;
		case CPUINFO_STR_REGISTER + SC61860_DP:		sprintf(info->s = cpuintrf_temp_str(), "DP:%.4x", sc61860.dp);break;
		case CPUINFO_STR_REGISTER + SC61860_P:		sprintf(info->s = cpuintrf_temp_str(), "P:%.2x", sc61860.p);break;
		case CPUINFO_STR_REGISTER + SC61860_Q:		sprintf(info->s = cpuintrf_temp_str(), "Q:%.2x", sc61860.q);break;
		case CPUINFO_STR_REGISTER + SC61860_R:		sprintf(info->s = cpuintrf_temp_str(), "R:%.2x", sc61860.r);break;
		case CPUINFO_STR_REGISTER + SC61860_I:		sprintf(info->s = cpuintrf_temp_str(), "I:%.2x", sc61860.ram[I]);break;
		case CPUINFO_STR_REGISTER + SC61860_J:		sprintf(info->s = cpuintrf_temp_str(), "J:%.2x", sc61860.ram[J]);break;
		case CPUINFO_STR_REGISTER + SC61860_K:		sprintf(info->s = cpuintrf_temp_str(), "K:%.2x", sc61860.ram[K]);break;
		case CPUINFO_STR_REGISTER + SC61860_L:		sprintf(info->s = cpuintrf_temp_str(), "L:%.2x", sc61860.ram[L]);break;
		case CPUINFO_STR_REGISTER + SC61860_V:		sprintf(info->s = cpuintrf_temp_str(), "V:%.2x", sc61860.ram[V]);break;
		case CPUINFO_STR_REGISTER + SC61860_W:		sprintf(info->s = cpuintrf_temp_str(), "W:%.2x", sc61860.ram[W]);break;
		case CPUINFO_STR_REGISTER + SC61860_H:		sprintf(info->s = cpuintrf_temp_str(), "W:%.2x", sc61860.h);break;
		case CPUINFO_STR_REGISTER + SC61860_BA:		sprintf(info->s = cpuintrf_temp_str(), "BA:%.2x%.2x", sc61860.ram[B], sc61860.ram[A]);break;
		case CPUINFO_STR_REGISTER + SC61860_X:		sprintf(info->s = cpuintrf_temp_str(), "X: %.2x%.2x", sc61860.ram[XH], sc61860.ram[XL]);break;
		case CPUINFO_STR_REGISTER + SC61860_Y:		sprintf(info->s = cpuintrf_temp_str(), "Y: %.2x%.2x", sc61860.ram[YH], sc61860.ram[YL]);break;
		case CPUINFO_STR_REGISTER + SC61860_CARRY:	sprintf(info->s = cpuintrf_temp_str(), "Carry: %d", sc61860.carry);break;
		case CPUINFO_STR_REGISTER + SC61860_ZERO:	sprintf(info->s = cpuintrf_temp_str(), "Zero: %d", sc61860.zero);break;
	}
}
