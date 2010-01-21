/*****************************************************************************
 *
 *   i4004.c
 *
 *   Intel 4004 CPU
 *
 *   Initial version by Miodrag Milanovic
 *
 *****************************************************************************/
#include "emu.h"
#include "debugger.h"
#include "i4004.h"

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

static const UINT8 kbp_table[] = { 0x00,0x01,0x02,0x0f,0x03,0x0f,0x0f,0x0f,0x04,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f };

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _i4004_state i4004_state;
struct _i4004_state
{
	UINT8	A; // Accumulator
	UINT8	R[8];
	PAIR	ADDR[4]; // Address registers
	PAIR	RAM;
	UINT8	C; // Carry flag
	UINT8	TEST; // Test PIN status
	PAIR	PC; // It is in fact one of ADDR regs

	running_device *device;
	const address_space *program;
	const address_space *data;
	const address_space *io;
	cpu_state_table 	state;
	int					icount;
	int 				pc_pos; // PC possition in ADDR
	int					addr_mask;
};

/***************************************************************************
    MACROS
***************************************************************************/
#define GET_PC					(cpustate->ADDR[cpustate->pc_pos])

/***************************************************************************
    CPU STATE DESCRIPTION
***************************************************************************/

#define I4004_STATE_ENTRY(_name, _format, _member, _datamask, _flags) \
	CPU_STATE_ENTRY(I4004_##_name, #_name, _format, i4004_state, _member, _datamask, ~0, _flags)

static const cpu_state_entry state_array[] =
{
	I4004_STATE_ENTRY(PC,   "%03X", PC.w.l, 0x0fff, 0)
	I4004_STATE_ENTRY(GENPC,"%03X", PC.w.l, 0x0fff, CPUSTATE_NOSHOW)
	I4004_STATE_ENTRY(A,   "%01X", A, 0x0f, 0)
	I4004_STATE_ENTRY(R01, "%02X", R[0], 0xff, 0)
	I4004_STATE_ENTRY(R23, "%02X", R[1], 0xff, 0)
	I4004_STATE_ENTRY(R45, "%02X", R[2], 0xff, 0)
	I4004_STATE_ENTRY(R67, "%02X", R[3], 0xff, 0)
	I4004_STATE_ENTRY(R89, "%02X", R[4], 0xff, 0)
	I4004_STATE_ENTRY(RAB, "%02X", R[5], 0xff, 0)
	I4004_STATE_ENTRY(RCD, "%02X", R[6], 0xff, 0)
	I4004_STATE_ENTRY(REF, "%02X", R[7], 0xff, 0)
	I4004_STATE_ENTRY(ADDR1, "%03X", ADDR[0].w.l, 0x0fff, 0)
	I4004_STATE_ENTRY(ADDR2, "%03X", ADDR[1].w.l, 0x0fff, 0)
	I4004_STATE_ENTRY(ADDR3, "%03X", ADDR[2].w.l, 0x0fff, 0)
	I4004_STATE_ENTRY(ADDR4, "%03X", ADDR[3].w.l, 0x0fff, 0)
	I4004_STATE_ENTRY(RAM, "%03X", RAM.w.l, 0x0fff, 0)
};

static const cpu_state_table state_table_template =
{
	NULL,						/* pointer to the base of state (offsets are relative to this) */
	0,							/* subtype this table refers to */
	ARRAY_LENGTH(state_array),	/* number of entries */
	state_array					/* array of entries */
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE i4004_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == CPU);
	assert(cpu_get_type(device) == CPU_I4004);
	return (i4004_state *)device->token;
}

INLINE UINT8 ROP(i4004_state *cpustate)
{
	UINT8 retVal = memory_decrypted_read_byte(cpustate->program, GET_PC.w.l);
	GET_PC.w.l = (GET_PC.w.l + 1) & 0x0fff;
	cpustate->PC = GET_PC;
	return retVal;
}

INLINE UINT8 READ_ROM(i4004_state *cpustate)
{
	return memory_decrypted_read_byte(cpustate->program, (GET_PC.w.l & 0x0f00) | cpustate->R[0]);
}

INLINE void WPM(i4004_state *cpustate)
{
	UINT8 t =  (memory_read_byte_8le(cpustate->program, cpustate->RAM.d) << 4) | cpustate->A;
	memory_write_byte_8le(cpustate->program, (GET_PC.w.l & 0x0f00) | cpustate->RAM.d, t);
}


INLINE UINT8 ARG(i4004_state *cpustate)
{
	UINT8 retVal = memory_raw_read_byte(cpustate->program, GET_PC.w.l);
	GET_PC.w.l = (GET_PC.w.l + 1) & 0x0fff;
	cpustate->PC = GET_PC;
	return retVal;
}

INLINE UINT8 RM(i4004_state *cpustate)
{
	return memory_read_byte_8le(cpustate->data, cpustate->RAM.d) & 0x0f;
}

INLINE UINT8 RMS(i4004_state *cpustate, UINT32 a)
{
	return memory_read_byte_8le(cpustate->data, (cpustate->RAM.d & 0xff0) + a) >> 4;
}

INLINE void WM(i4004_state *cpustate, UINT8 v)
{
	UINT8 t =  memory_read_byte_8le(cpustate->data, cpustate->RAM.d);
	memory_write_byte_8le(cpustate->data, cpustate->RAM.d, (t & 0xf0) | v);
}


INLINE void WMP(i4004_state *cpustate, UINT8 v)
{
	memory_write_byte_8le(cpustate->io, (cpustate->RAM.d >> 6) | 0x10, v & 0x0f);
}

INLINE void WMS(i4004_state *cpustate, UINT32 a, UINT8 v)
{
	UINT8 t =  memory_read_byte_8le(cpustate->data, (cpustate->RAM.d & 0xff0) + a);
	memory_write_byte_8le(cpustate->data,(cpustate->RAM.d & 0xff0) + a, (t & 0x0f) | (v<<4));
}

INLINE UINT8 RIO(i4004_state *cpustate)
{
	return memory_read_byte_8le(cpustate->io, cpustate->RAM.b.l >> 4) & 0x0f;
}

INLINE void WIO(i4004_state *cpustate, UINT8 v)
{
	memory_write_byte_8le(cpustate->io, cpustate->RAM.b.l >> 4, v & 0x0f);
}

INLINE UINT8 GET_REG(i4004_state *cpustate, UINT8 num)
{
	UINT8 r = cpustate->R[num>>1];
	if (num & 1) {
		return r & 0x0f;
	} else {
		return (r >> 4) & 0x0f;
	}
}

INLINE void SET_REG(i4004_state *cpustate, UINT8 num, UINT8 val)
{
	if (num & 1) {
		cpustate->R[num>>1] = (cpustate->R[num>>1] & 0xf0) + (val & 0x0f);
	} else {
		cpustate->R[num>>1] = (cpustate->R[num>>1] & 0x0f) + ((val & 0x0f) << 4);
	}
}

INLINE void PUSH_STACK(i4004_state *cpustate)
{
	cpustate->pc_pos = (cpustate->pc_pos + 1) & cpustate->addr_mask;
}

INLINE void POP_STACK(i4004_state *cpustate)
{
	cpustate->ADDR[cpustate->pc_pos].d = 0;
	cpustate->pc_pos = (cpustate->pc_pos - 1) & cpustate->addr_mask;
}

void i4004_set_test(running_device *device, UINT8 val)
{
	i4004_state *cpustate = get_safe_token(device);
	cpustate->TEST = val;
}

static void execute_one(i4004_state *cpustate, int opcode)
{
	cpustate->icount -= 8;
	switch (opcode)
	{
		case 0x00:	/* NOP  */
			/* no op */
			break;
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c: case 0x1d: case 0x1e: case 0x1f: /* JCN */
			{
				UINT8 arg =  ARG(cpustate);

				UINT8 C1 = BIT(opcode,3);
				UINT8 C2 = BIT(opcode,2);
				UINT8 C3 = BIT(opcode,1);
				UINT8 C4 = BIT(opcode,0);
				UINT8 JUMP = (((cpustate->A == 0) ? 1 : 0) & C2) | ((cpustate->C) & C3) | ((cpustate->TEST ^ 1) & C4);
				cpustate->icount -= 8;

				if(((C1 ^ 1) &  JUMP) | (C1 & (JUMP ^ 1))) {
					GET_PC.w.l = (GET_PC.w.l & 0x0f00) | arg;
					cpustate->PC = GET_PC;
				}
			}
			break;
		case 0x20: case 0x22: case 0x24: case 0x26:
		case 0x28: case 0x2a: case 0x2c: case 0x2e: /* FIM */
			cpustate->icount -= 8;
			cpustate->R[(opcode & 0x0f)>>1] = ROP(cpustate);
			break;
		case 0x21: case 0x23: case 0x25: case 0x27:
		case 0x29: case 0x2b: case 0x2d: case 0x2f: /* SRC */
			cpustate->RAM.b.l = cpustate->R[(opcode & 0x0f)>>1];
			break;
		case 0x30: case 0x32: case 0x34: case 0x36:
		case 0x38: case 0x3a: case 0x3c: case 0x3e: /* FIN */
			cpustate->icount -= 8;
			cpustate->R[(opcode & 0x0f)>>1] = READ_ROM(cpustate);
			break;
		case 0x31: case 0x33: case 0x35: case 0x37:
		case 0x39: case 0x3b: case 0x3d: case 0x3f: /* JIN */
			GET_PC.w.l = (GET_PC.w.l & 0x0f00) | cpustate->R[(opcode & 0x0f)>>1];
			cpustate->PC = GET_PC;
			break;
		case 0x40: case 0x41: case 0x42: case 0x43:
		case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b:
		case 0x4c: case 0x4d: case 0x4e: case 0x4f: /* JUN */
			cpustate->icount -= 8;
			GET_PC.w.l = ((opcode & 0x0f) << 8) | ARG(cpustate);
			cpustate->PC = GET_PC;
			break;
		case 0x50: case 0x51: case 0x52: case 0x53:
		case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b:
		case 0x5c: case 0x5d: case 0x5e: case 0x5f: /* JMS */
			{
				UINT16 newPC = ((opcode & 0x0f) << 8) | ARG(cpustate);
				cpustate->icount -= 8;
				PUSH_STACK(cpustate);
				GET_PC.w.l = newPC;
				cpustate->PC = GET_PC;
			}
			break;
		case 0x60: case 0x61: case 0x62: case 0x63:
		case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b:
		case 0x6c: case 0x6d: case 0x6e: case 0x6f: /* INC */
			SET_REG(cpustate, opcode & 0x0f, GET_REG(cpustate, opcode & 0x0f) + 1);
			break;
		case 0x70: case 0x71: case 0x72: case 0x73:
		case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b:
		case 0x7c: case 0x7d: case 0x7e: case 0x7f: /* ISZ */
			{
				UINT8 val = (GET_REG(cpustate, opcode & 0x0f) + 1) & 0xf;
				UINT16 addr = ARG(cpustate);
				cpustate->icount -= 8;
				SET_REG(cpustate, opcode & 0x0f, val);
				if (val!=0) {
					GET_PC.w.l = (GET_PC.w.l & 0x0f00) | addr;
				}
				cpustate->PC = GET_PC;
			}
			break;
		case 0x80: case 0x81: case 0x82: case 0x83:
		case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b:
		case 0x8c: case 0x8d: case 0x8e: case 0x8f: /* ADD */
			{
				UINT8 acc = cpustate->A + GET_REG(cpustate, opcode & 0x0f) + cpustate->C;
				cpustate->A = acc & 0x0f;
				cpustate->C = (acc >> 4) & 1;
			}
			break;
		case 0x90: case 0x91: case 0x92: case 0x93:
		case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b:
		case 0x9c: case 0x9d: case 0x9e: case 0x9f: /* SUB */
			{
				UINT8 acc = cpustate->A + (GET_REG(cpustate, opcode & 0x0f) ^ 0x0f) + (cpustate->C ^ 1);
				cpustate->A = acc & 0x0f;
				cpustate->C = (acc >> 4) & 1;
			}
			break;
		case 0xa0: case 0xa1: case 0xa2: case 0xa3:
		case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab:
		case 0xac: case 0xad: case 0xae: case 0xaf: /* LD */
			cpustate->A = GET_REG(cpustate, opcode & 0x0f);
			break;
		case 0xb0: case 0xb1: case 0xb2: case 0xb3:
		case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb:
		case 0xbc: case 0xbd: case 0xbe: case 0xbf: /* XCH */
			{
				UINT8 temp = cpustate->A;
				cpustate->A = GET_REG(cpustate, opcode & 0x0f);
				SET_REG(cpustate, opcode & 0x0f, temp);
			}
			break;
		case 0xc0: case 0xc1: case 0xc2: case 0xc3:
		case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		case 0xc8: case 0xc9: case 0xca: case 0xcb:
		case 0xcc: case 0xcd: case 0xce: case 0xcf: /*  BBL */
		    POP_STACK(cpustate);
		    cpustate->A = opcode & 0x0f;
		    cpustate->PC = GET_PC;
			break;
		case 0xd0: case 0xd1: case 0xd2: case 0xd3:
		case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		case 0xd8: case 0xd9: case 0xda: case 0xdb:
		case 0xdc: case 0xdd: case 0xde: case 0xdf: /* LDM */
			cpustate->A = opcode & 0x0f;
			break;
		case 0xe0: /* WRM */
			WM(cpustate,cpustate->A);
			break;
		case 0xe1: /* WMP */
			WMP(cpustate,cpustate->A);
			break;
		case 0xe2: /* WRR */
			WIO(cpustate,cpustate->A);
			break;
		case 0xe3: /* WPM */
			WPM(cpustate);
			break;
		case 0xe4: /* WR0 */
			WMS(cpustate,0,cpustate->A);
			break;
		case 0xe5: /* WR1 */
			WMS(cpustate,1,cpustate->A);
			break;
		case 0xe6: /* WR2 */
			WMS(cpustate,2,cpustate->A);
			break;
		case 0xe7: /* WR3 */
			WMS(cpustate,3,cpustate->A);
			break;
		case 0xe8: /* SBM */
			cpustate->A = cpustate->A + (RM(cpustate) ^ 0x0f) + (cpustate->C ^ 1);
			cpustate->C = cpustate->A >> 4;
			cpustate->A &= 0x0f;
			break;
		case 0xe9: /* RDM */
			cpustate->A = RM(cpustate);
			break;
		case 0xea: /* RDR */
			cpustate->A = RIO(cpustate);
			break;
		case 0xeb: /* ADM */
			cpustate->A += RM(cpustate) + cpustate->C;
			cpustate->C = cpustate->A >> 4;
			cpustate->A &= 0x0f;
			break;
		case 0xec: /* RD0 */
			cpustate->A = RMS(cpustate,0);
			break;
		case 0xed: /* RD1 */
			cpustate->A = RMS(cpustate,1);
			break;
		case 0xee: /* RD2 */
			cpustate->A = RMS(cpustate,2);
			break;
		case 0xef: /* RD3 */
			cpustate->A = RMS(cpustate,3);
			break;

		case 0xf0: /* CLB */
			cpustate->A = 0;
			cpustate->C = 0;
			break;
		case 0xf1: /* CLC */
			cpustate->C = 0;
			break;
		case 0xf2: /* IAC */
			cpustate->A += 1;
			cpustate->C = cpustate->A >> 4;
			cpustate->A &= 0x0f;
			break;
		case 0xf3: /* CMC */
			cpustate->C ^= 1;
			break;
		case 0xf4: /* CMA */
			cpustate->A ^= 0x0f;
			break;
		case 0xf5: /* RAL */
			cpustate->A = (cpustate->A << 1) | cpustate->C;
			cpustate->C = cpustate->A >> 4;
			cpustate->A &= 0x0f;
			break;
		case 0xf6: /* RAR */
			{
				UINT8 c = cpustate->A & 1;
				cpustate->A = (cpustate->A >> 1) | (cpustate->C  << 3);
				cpustate->C = c;
			}
			break;
		case 0xf7: /* TCC */
			cpustate->A = cpustate->C;
			cpustate->C = 0;
			break;
		case 0xf8: /* DAC */
			cpustate->A = cpustate->A + 0x0f;
			cpustate->C = cpustate->A >> 4;
			cpustate->A &= 0x0f;
			break;
		case 0xf9: /* TCS */
			cpustate->A = cpustate->C ? 10 : 9;
			cpustate->C = 0;
			break;
		case 0xfa: /* STC */
			cpustate->C = 1;
			break;
		case 0xfb: /* DAA */
			if (cpustate->C || (cpustate->A > 9)) {
				cpustate->A += 6;
			}
			if (cpustate->A > 0x0f) {
				// it is unaffected if it is in range
				cpustate->C = 1;
			}
			cpustate->A &= 0x0f;
			break;
		case 0xfc: /* KBP */
			cpustate->A = kbp_table[cpustate->A];
			break;
		case 0xfd: /* DCL */
			cpustate->RAM.b.h = cpustate->A;
			break;
	}
}


/***************************************************************************
    COMMON EXECUTION
***************************************************************************/

static CPU_EXECUTE( i4004 )
{
	i4004_state *cpustate = get_safe_token(device);

	cpustate->icount = cycles;


	do
	{
		debugger_instruction_hook(device, GET_PC.d);
		execute_one(cpustate, ROP(cpustate));

	} while (cpustate->icount > 0);

	return cycles - cpustate->icount;
}

/***************************************************************************
    CORE INITIALIZATION
***************************************************************************/

static CPU_INIT( i4004 )
{
	i4004_state *cpustate = get_safe_token(device);

	/* set up the state table */
	cpustate->state = state_table_template;
	cpustate->state.baseptr = cpustate;
	cpustate->state.subtypemask = 1;

	cpustate->device = device;

	cpustate->program = device->space(AS_PROGRAM);
	cpustate->data = device->space(AS_DATA);
	cpustate->io = device->space(AS_IO);

	state_save_register_device_item(device, 0, cpustate->PC);
	state_save_register_device_item(device, 0, cpustate->A);
	state_save_register_device_item(device, 0, cpustate->C);
	state_save_register_device_item(device, 0, cpustate->TEST);
	state_save_register_device_item(device, 0, cpustate->pc_pos);
	state_save_register_device_item(device, 0, cpustate->ADDR[0]);
	state_save_register_device_item(device, 0, cpustate->ADDR[1]);
	state_save_register_device_item(device, 0, cpustate->ADDR[2]);
	state_save_register_device_item(device, 0, cpustate->ADDR[3]);
	state_save_register_device_item(device, 0, cpustate->R[0]);
	state_save_register_device_item(device, 0, cpustate->R[1]);
	state_save_register_device_item(device, 0, cpustate->R[2]);
	state_save_register_device_item(device, 0, cpustate->R[3]);
	state_save_register_device_item(device, 0, cpustate->R[4]);
	state_save_register_device_item(device, 0, cpustate->R[5]);
	state_save_register_device_item(device, 0, cpustate->R[6]);
	state_save_register_device_item(device, 0, cpustate->R[7]);
	state_save_register_device_item(device, 0, cpustate->RAM);
}



/***************************************************************************
    COMMON RESET
***************************************************************************/

static CPU_RESET( i4004 )
{
	i4004_state *cpustate = get_safe_token(device);

	cpustate->addr_mask = 3;
	cpustate->C = 0;
	cpustate->pc_pos = 0;
	cpustate->A = 0;
	memset(cpustate->R,0,8);
	memset(cpustate->ADDR,0,sizeof(cpustate->ADDR));
	cpustate->RAM.d = 0;
	cpustate->PC = GET_PC;

}



/***************************************************************************
    COMMON STATE IMPORT/EXPORT
***************************************************************************/

static CPU_IMPORT_STATE( i4004 )
{
}

static CPU_EXPORT_STATE( i4004 )
{
}

/***************************************************************************
    COMMON SET INFO
***************************************************************************/
static CPU_SET_INFO( i4004 )
{
}

/***************************************************************************
    4004 GET INFO
***************************************************************************/

CPU_GET_INFO( i4004 )
{
	i4004_state *cpustate = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(i4004_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 0;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 8;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 16;							break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:			info->i = 8;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: 		info->i = 12;							break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: 		info->i = 0;							break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:			info->i = 8;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:			info->i = 12;							break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:			info->i = 0;							break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:				info->i = 8;							break; // Only lower 4 bits used
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:				info->i = 6;							break; // 4 I/O for each ROM chip and 4 OUT for each RAM
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:				info->i = 0;							break; // There could be 4 chips in 16 banks for RAM

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_SET_INFO:		info->setinfo = CPU_SET_INFO_NAME(i4004);				break;
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(i4004);						break;
		case CPUINFO_FCT_RESET:			info->reset = CPU_RESET_NAME(i4004);					break;
		case CPUINFO_FCT_EXECUTE:		info->execute = CPU_EXECUTE_NAME(i4004);				break;
		case CPUINFO_FCT_DISASSEMBLE:	info->disassemble = CPU_DISASSEMBLE_NAME(i4004);		break;
		case CPUINFO_FCT_IMPORT_STATE:	info->import_state = CPU_IMPORT_STATE_NAME(i4004);		break;
		case CPUINFO_FCT_EXPORT_STATE:	info->export_state = CPU_EXPORT_STATE_NAME(i4004);		break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;		break;
		case CPUINFO_PTR_STATE_TABLE:					info->state_table = &cpustate->state;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "4004");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Intel 4004");			break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:				strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Miodrag Milanovic"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, ".%c%c%c",
				(cpustate->A==0) ? 'Z':'.',
				cpustate->C      ? 'C':'.',
				cpustate->TEST   ? 'T':'.');
			break;
	}
}
