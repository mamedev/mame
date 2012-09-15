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

#include "emu.h"
#include "debugger.h"

#include "sc61860.h"
#include "sc.h"

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

/****************************************************************************
 * The 61860 registers.
 ****************************************************************************/
struct sc61860_state
{
	sc61860_cpu_core *config;
	UINT8 p, q, r; //7 bits only?

	UINT8 c;        // port c, used for HLT.
	UINT8 d, h;
	UINT16 oldpc, pc, dp;

	int carry, zero;

	struct { int t2ms, t512ms; int count; } timer;

	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	int icount;
	UINT8 ram[0x100]; // internal special ram, should be 0x60, 0x100 to avoid memory corruption for now
};

INLINE sc61860_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SC61860);
	return (sc61860_state *)downcast<legacy_cpu_device *>(device)->token();
}

UINT8 *sc61860_internal_ram(device_t *device)
{
	sc61860_state *cpustate = get_safe_token(device);
	return cpustate->ram;
}

static TIMER_CALLBACK(sc61860_2ms_tick)
{
	sc61860_state *cpustate = (sc61860_state *)ptr;
	if (--cpustate->timer.count == 0)
	{
		cpustate->timer.count = 128;
		cpustate->timer.t512ms = !cpustate->timer.t512ms;
	}
	cpustate->timer.t2ms = !cpustate->timer.t2ms;
}

/***************************************************************
 * include the opcode macros, functions and tables
 ***************************************************************/
#include "scops.c"
#include "sctable.c"

static CPU_RESET( sc61860 )
{
	sc61860_state *cpustate = get_safe_token(device);
	cpustate->timer.t2ms=0;
	cpustate->timer.t512ms=0;
	cpustate->timer.count=256;
	cpustate->pc=0;
}

static CPU_INIT( sc61860 )
{
	sc61860_state *cpustate = get_safe_token(device);
	cpustate->config = (sc61860_cpu_core *) device->static_config();
	device->machine().scheduler().timer_pulse(attotime::from_hz(500), FUNC(sc61860_2ms_tick), 0, cpustate);
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
}

static CPU_EXECUTE( sc61860 )
{
	sc61860_state *cpustate = get_safe_token(device);

	do
	{
		cpustate->oldpc = cpustate->pc;

		debugger_instruction_hook(device, cpustate->pc);

		sc61860_instruction(cpustate);

#if 0
		/* Are we in HLT-mode? */
		if (cpustate->c & 4)
		{
			if ((cpustate->config && cpustate->config->ina && (cpustate->config->ina(cpustate)!=0)) || cpustate->timer.t512ms)
			{
				cpustate->c&=0xfb;
				if (cpustate->config->outc) cpustate->config->outc(cpustate->c);
			}
			cpustate->icount-=4;
		}
		else if(cpustate->c & 8) {}

		else sc61860_instruction(cpustate);
#endif

	} while (cpustate->icount > 0);
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( sc61860 )
{
	sc61860_state *cpustate = get_safe_token(device);
	switch (state)
	{

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SC61860_PC:			cpustate->pc = info->i; break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + SC61860_R:			cpustate->r = info->i & 0x7F;						break;
		case CPUINFO_INT_REGISTER + SC61860_DP:			cpustate->dp = info->i;			break;
		case CPUINFO_INT_REGISTER + SC61860_P:			cpustate->p = info->i & 0x7F;		break;
		case CPUINFO_INT_REGISTER + SC61860_Q:			cpustate->q = info->i & 0x7F;		break;
		case CPUINFO_INT_REGISTER + SC61860_CARRY:		cpustate->carry = info->i;			break;
		case CPUINFO_INT_REGISTER + SC61860_ZERO:		cpustate->zero = info->i;			break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( sc61860 )
{
	sc61860_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(sc61860_state);				break;
		case CPUINFO_INT_INPUT_LINES:						info->i = 0;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 4;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->oldpc;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SC61860_PC:				info->i =  cpustate->pc;						break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + SC61860_R:				info->i =  cpustate->r;						break;
		case CPUINFO_INT_REGISTER + SC61860_DP:				info->i =  cpustate->dp;						break;
		case CPUINFO_INT_REGISTER + SC61860_P:				info->i =  cpustate->p;						break;
		case CPUINFO_INT_REGISTER + SC61860_Q:				info->i =  cpustate->q;						break;
		case CPUINFO_INT_REGISTER + SC61860_CARRY:			info->i =  cpustate->carry;					break;
		case CPUINFO_INT_REGISTER + SC61860_ZERO:			info->i =  cpustate->zero;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(sc61860);				break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(sc61860);						break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(sc61860);					break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(sc61860);				break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;								break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(sc61860);				break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "SC61860"); break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "SC61860"); break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.0beta"); break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__); break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Peter Trauner, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c", cpustate->zero?'Z':'.', cpustate->carry ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + SC61860_PC:		sprintf(info->s, "PC:%.4x", cpustate->pc);break;
		case CPUINFO_STR_REGISTER + SC61860_DP:		sprintf(info->s, "DP:%.4x", cpustate->dp);break;
		case CPUINFO_STR_REGISTER + SC61860_P:		sprintf(info->s, "P:%.2x", cpustate->p);break;
		case CPUINFO_STR_REGISTER + SC61860_Q:		sprintf(info->s, "Q:%.2x", cpustate->q);break;
		case CPUINFO_STR_REGISTER + SC61860_R:		sprintf(info->s, "R:%.2x", cpustate->r);break;
		case CPUINFO_STR_REGISTER + SC61860_I:		sprintf(info->s, "I:%.2x", cpustate->ram[I]);break;
		case CPUINFO_STR_REGISTER + SC61860_J:		sprintf(info->s, "J:%.2x", cpustate->ram[J]);break;
		case CPUINFO_STR_REGISTER + SC61860_K:		sprintf(info->s, "K:%.2x", cpustate->ram[K]);break;
		case CPUINFO_STR_REGISTER + SC61860_L:		sprintf(info->s, "L:%.2x", cpustate->ram[L]);break;
		case CPUINFO_STR_REGISTER + SC61860_V:		sprintf(info->s, "V:%.2x", cpustate->ram[V]);break;
		case CPUINFO_STR_REGISTER + SC61860_W:		sprintf(info->s, "W:%.2x", cpustate->ram[W]);break;
		case CPUINFO_STR_REGISTER + SC61860_H:		sprintf(info->s, "W:%.2x", cpustate->h);break;
		case CPUINFO_STR_REGISTER + SC61860_BA:		sprintf(info->s, "BA:%.2x%.2x", cpustate->ram[B], cpustate->ram[A]);break;
		case CPUINFO_STR_REGISTER + SC61860_X:		sprintf(info->s, "X: %.2x%.2x", cpustate->ram[XH], cpustate->ram[XL]);break;
		case CPUINFO_STR_REGISTER + SC61860_Y:		sprintf(info->s, "Y: %.2x%.2x", cpustate->ram[YH], cpustate->ram[YL]);break;
		case CPUINFO_STR_REGISTER + SC61860_CARRY:	sprintf(info->s, "Carry: %d", cpustate->carry);break;
		case CPUINFO_STR_REGISTER + SC61860_ZERO:	sprintf(info->s, "Zero: %d", cpustate->zero);break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(SC61860, sc61860);
