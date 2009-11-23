/*****************************************************************************
 *
 *   scmp.c
 *
 *   National Semiconductor SC/MP CPU Disassembly
 *
 *   Initial version by Miodrag Milanovic
 *
 *****************************************************************************/

#include "debugger.h"
#include "scmp.h"

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _scmp_state scmp_state;
struct _scmp_state
{
	scmp_config 		config;
	PAIR	PC;
	PAIR	P1;
	PAIR	P2;
	PAIR	P3;
	UINT8	AC;
	UINT8	ER;
	UINT8	SR;

	const device_config *device;
	const address_space *program;
	const address_space *io;
	cpu_state_table 	state;
	int					icount;

	devcb_resolved_write8		flag_out_func;
	devcb_resolved_write_line	sout_func;
	devcb_resolved_read_line	sin_func;
	devcb_resolved_read_line	sensea_func;
	devcb_resolved_read_line	senseb_func;
	devcb_resolved_write_line	halt_func;
};

/***************************************************************************
    MACROS
***************************************************************************/

/***************************************************************************
    CPU STATE DESCRIPTION
***************************************************************************/

#define SCMP_STATE_ENTRY(_name, _format, _member, _datamask, _flags) \
	CPU_STATE_ENTRY(SCMP_##_name, #_name, _format, scmp_state, _member, _datamask, ~0, _flags)

static const cpu_state_entry state_array[] =
{
	SCMP_STATE_ENTRY(PC,   "%04X", PC.w.l, 0xffff, 0)
	SCMP_STATE_ENTRY(GENPC,"%04X", PC.w.l, 0xffff, CPUSTATE_NOSHOW)
	SCMP_STATE_ENTRY(P1,   "%04X", P1.w.l, 0xffff, 0)
	SCMP_STATE_ENTRY(P2,   "%04X", P2.w.l, 0xffff, 0)
	SCMP_STATE_ENTRY(P3,   "%04X", P3.w.l, 0xffff, 0)
	SCMP_STATE_ENTRY(AC,   "%02X", AC, 0xff, 0)
	SCMP_STATE_ENTRY(ER,   "%02X", ER, 0xff, 0)
	SCMP_STATE_ENTRY(SR,   "%02X", SR, 0xff, 0)

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

INLINE scmp_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == CPU);
	assert(cpu_get_type(device) == CPU_SCMP);
	return (scmp_state *)device->token;
}

INLINE UINT16 ADD12(UINT16 addr, INT8 val)
{
	return ((addr + val) & 0x0fff) | (addr & 0xf000);
}

INLINE UINT8 ROP(scmp_state *cpustate)
{
	UINT16 pc = cpustate->PC.w.l;
	cpustate->PC.w.l = ADD12(cpustate->PC.w.l,1);
	return memory_decrypted_read_byte(cpustate->program,  pc);
}

INLINE UINT8 ARG(scmp_state *cpustate)
{
	UINT16 pc = cpustate->PC.w.l;
	cpustate->PC.w.l = ADD12(cpustate->PC.w.l,1);
	return memory_raw_read_byte(cpustate->program, pc);
}

INLINE UINT8 RM(scmp_state *cpustate,UINT32 a)
{
	return memory_read_byte_8le(cpustate->program, a);
}

INLINE void WM(scmp_state *cpustate,UINT32 a, UINT8 v)
{
	memory_write_byte_8le(cpustate->program, a, v);
}

INLINE void illegal(scmp_state *cpustate,UINT8 opcode)
{
#if VERBOSE
	UINT16 pc = cpustate->PC.w.l;
	LOG(("SC/MP illegal instruction %04X $%02X\n", pc-1, opcode));
#endif
}

INLINE PAIR *GET_PTR_REG(scmp_state *cpustate, int num)
{
	switch(num) {
		case 1: return &cpustate->P1;
		case 2: return &cpustate->P2;
		case 3: return &cpustate->P3;
		default :
				return &cpustate->PC;
	}
}

INLINE void BIN_ADD(scmp_state *cpustate, UINT8 val)
{
	UINT16 tmp = cpustate->AC + val + ((cpustate->SR >> 7) & 1);
	UINT8 ov = (((cpustate->AC & 0x80)==(val & 0x80)) && ((cpustate->AC & 0x80)!=(tmp & 0x80))) ? 0x40 : 0x00;

	cpustate->AC = tmp & 0xff;
	cpustate->SR &= 0x3f; // clear CY/L and OV flag
	cpustate->SR |= (tmp & 0x100) ? 0x80 : 0x00; // set CY/L
	cpustate->SR |= ov;
}

INLINE void DEC_ADD(scmp_state *cpustate, UINT8 val)
{
	UINT16 tmp = cpustate->AC + val + ((cpustate->SR >> 7) & 1);
	if ((tmp & 0x0f) > 9) tmp +=6;
	cpustate->AC = tmp % 0xa0;
	cpustate->SR &= 0x7f; // clear CY/L flag
	cpustate->SR |= (tmp > 0x99) ? 0x80 : 0x00;
}

INLINE UINT16 GET_ADDR(scmp_state *cpustate, UINT8 code)
{
	UINT16 addr = 0;
	INT8 offset = 0;
	UINT16 retVal = 0;
	UINT16 ptr = GET_PTR_REG(cpustate,code & 0x03)->w.l;

	UINT8 arg = ARG(cpustate);
	if (arg == 0x80) {
		offset = cpustate->ER;
	} else {
		if (arg & 0x80) {
			offset = (INT8)arg;
		} else {
			offset = arg;
		}
	}

	addr = ADD12(ptr,offset);

	if (code & 0x04) {
		if (code & 0x03) {
			// Auto-indexed
			if (offset < 0) {
				// pre decrement
				GET_PTR_REG(cpustate,code & 0x03)->w.l = addr;
				retVal = addr;
			} else {
				// post increment
				retVal = ptr;
				GET_PTR_REG(cpustate,code & 0x03)->w.l = addr;
			}
		} else {
			// Immediate
		}
	} else {
		// Indexed
		retVal = addr;
	}
	return retVal;
}

static void execute_one(scmp_state *cpustate, int opcode)
{
	UINT8 tmp;
	UINT8 ptr = opcode & 3;
	if (BIT(opcode,7)) {
		// two bytes instructions
		switch (opcode)
		{
			// Memory Reference Instructions
			case 0xc0 : case 0xc1 : case 0xc2 : case 0xc3 :
			case 0xc5 : case 0xc6 : case 0xc7 :
						//LD
						cpustate->icount -= 18;
						cpustate->AC = RM(cpustate,GET_ADDR(cpustate,opcode));
						break;
			case 0xc8 : case 0xc9 : case 0xca : case 0xcb :
			case 0xcd : case 0xce : case 0xcf :
						// ST
						cpustate->icount -= 18;
						WM(cpustate,GET_ADDR(cpustate,opcode),cpustate->AC);
						break;
			case 0xd0 : case 0xd1 : case 0xd2 : case 0xd3 :
						case 0xd5 : case 0xd6 : case 0xd7 :
						// AND
						cpustate->icount -= 18;
						cpustate->AC &= RM(cpustate,GET_ADDR(cpustate,opcode));
						break;
			case 0xd8 : case 0xd9 : case 0xda : case 0xdb :
						case 0xdd : case 0xde : case 0xdf :
						//OR
						cpustate->icount -= 18;
						cpustate->AC |= RM(cpustate,GET_ADDR(cpustate,opcode));
						break;
			case 0xe0 : case 0xe1 : case 0xe2 : case 0xe3 :
						case 0xe5 : case 0xe6 : case 0xe7 :
						// XOR
						cpustate->icount -= 18;
						cpustate->AC ^= RM(cpustate,GET_ADDR(cpustate,opcode));
						break;
			case 0xe8 : case 0xe9 : case 0xea : case 0xeb :
						case 0xed : case 0xee : case 0xef :
						// DAD
						cpustate->icount -= 23;
						DEC_ADD(cpustate,RM(cpustate,GET_ADDR(cpustate,opcode)));
						break;
			case 0xf0 : case 0xf1 : case 0xf2 : case 0xf3 :
						case 0xf5 : case 0xf6 : case 0xf7 :
						// ADD
						cpustate->icount -= 19;
						BIN_ADD(cpustate,RM(cpustate,GET_ADDR(cpustate,opcode)));
						break;
			case 0xf8 : case 0xf9 : case 0xfa : case 0xfb :
						case 0xfd : case 0xfe : case 0xff :
						// CAD
						cpustate->icount -= 20;
						BIN_ADD(cpustate,~RM(cpustate,GET_ADDR(cpustate,opcode)));
						break;
			// Memory Increment/Decrement Instructions
			case 0xa8 : case 0xa9 : case 0xaa : case 0xab :
						// IDL
						{
							UINT16 addr = GET_ADDR(cpustate,opcode);
							cpustate->icount -= 22;
							cpustate->AC = RM(cpustate,addr) + 1;
							WM(cpustate,addr,cpustate->AC);
						}
						break;
			case 0xb8 : case 0xb9 : case 0xba : case 0xbb :
						// DLD
						{
							UINT16 addr = GET_ADDR(cpustate,opcode);
							cpustate->icount -= 22;
							cpustate->AC = RM(cpustate,addr) - 1;
							WM(cpustate,addr,cpustate->AC);
						}
						break;
			// Immediate Instructions
			case 0xc4 : // LDI
						cpustate->icount -= 10;
						cpustate->AC = ARG(cpustate);
						break;
			case 0xd4 : // ANI
						cpustate->icount -= 10;
						cpustate->AC &= ARG(cpustate);
						break;
			case 0xdc : // ORI
						cpustate->icount -= 10;
						cpustate->AC |= ARG(cpustate);
						break;
			case 0xe4 : // XRI
						cpustate->icount -= 10;
						cpustate->AC ^= ARG(cpustate);
						break;
			case 0xec : // DAI
						cpustate->icount -= 15;
						DEC_ADD(cpustate,ARG(cpustate));
						break;
			case 0xf4 : // ADI
						cpustate->icount -= 11;
						BIN_ADD(cpustate,ARG(cpustate));
						break;
			case 0xfc : // CAI
						cpustate->icount -= 12;
						BIN_ADD(cpustate,~ARG(cpustate));
						break;
			// Transfer Instructions
			case 0x90 : case 0x91 : case 0x92 : case 0x93 :// JMP
						cpustate->icount -= 11;
						cpustate->PC.w.l = ADD12(GET_PTR_REG(cpustate,ptr)->w.l,(INT8)ARG(cpustate));
						break;
			case 0x94 : case 0x95 : case 0x96 : case 0x97 :
						// JP
						cpustate->icount -= 9;
						tmp = ARG(cpustate);
						if (!cpustate->AC & 0x80) {
							cpustate->PC.w.l = ADD12(GET_PTR_REG(cpustate,ptr)->w.l,(INT8)tmp);
							cpustate->icount -= 2;
						}
						break;
			case 0x98 : case 0x99 : case 0x9a : case 0x9b :
						// JZ
						cpustate->icount -= 9;
						tmp = ARG(cpustate);
						if (!cpustate->AC) {
							cpustate->PC.w.l = ADD12(GET_PTR_REG(cpustate,ptr)->w.l,(INT8)tmp);
							cpustate->icount -= 2;
						}
						break;
			case 0x9c : case 0x9d : case 0x9e : case 0x9f :
						// JNZ
						cpustate->icount -= 9;
						tmp = ARG(cpustate);
						if (cpustate->AC) {
							cpustate->PC.w.l = ADD12(GET_PTR_REG(cpustate,ptr)->w.l,(INT8)tmp);
							cpustate->icount -= 2;
						}
						break;
			// Double-Byte Miscellaneous Instructions
			case 0x8f: 	// DLY
						tmp = ARG(cpustate);
						cpustate->icount -= 13 + (cpustate->AC * 2) + (((UINT32)tmp) << 1) + (((UINT32)tmp) << 9);
						cpustate->AC = 0xff;
						break;
			// Others are illegal
			default : 	cpustate->icount -= 1;
						illegal (cpustate,opcode);
						break;
		}
	} else {
		// one byte instructions
		switch (opcode)
		{
			// Extension Register Instructions
			case 0x40: 	// LDE
						cpustate->icount -= 6;
						cpustate->AC = cpustate->ER;
						break;
			case 0x01: 	// XAE
						cpustate->icount -= 7;
						tmp = cpustate->AC;
						cpustate->AC = cpustate->ER;
						cpustate->ER = tmp;
						break;
			case 0x50: 	// ANE
						cpustate->icount -= 6;
						cpustate->AC &= cpustate->ER;
						break;
			case 0x58: 	// ORE
						cpustate->icount -= 6;
						cpustate->AC |= cpustate->ER;
						break;
			case 0x60: 	// XRE
						cpustate->icount -= 6;
						cpustate->AC ^= cpustate->ER;
						break;
			case 0x68: 	// DAE
						cpustate->icount -= 11;
						DEC_ADD(cpustate,cpustate->ER);
						break;
			case 0x70: 	// ADE
						cpustate->icount -= 7;
						BIN_ADD(cpustate,cpustate->ER);
						break;
			case 0x78: 	// CAE
						cpustate->icount -= 8;
						BIN_ADD(cpustate,~cpustate->ER);
						break;
			// Pointer Register Move Instructions
			case 0x30: case 0x31: case 0x32: case 0x33:	// XPAL
						cpustate->icount -= 8;
						tmp = cpustate->AC;
						cpustate->AC = GET_PTR_REG(cpustate,ptr)->b.l;
						GET_PTR_REG(cpustate,ptr)->b.l = tmp;
						break;
			case 0x34: 	case 0x35 :case 0x36: case 0x37:
						// XPAH
						cpustate->icount -= 8;
						tmp = cpustate->AC;
						cpustate->AC = GET_PTR_REG(cpustate,ptr)->b.h;
						GET_PTR_REG(cpustate,ptr)->b.h = tmp;
						break;
			case 0x3c: 	case 0x3d :case 0x3e: case 0x3f:
						// XPPC
						{
							UINT16 tmp = ADD12(cpustate->PC.w.l,-1); // Since PC is incremented we need to fix it
							cpustate->icount -= 7;
							cpustate->PC.w.l = GET_PTR_REG(cpustate,ptr)->w.l;
							GET_PTR_REG(cpustate,ptr)->w.l = tmp;
							// After exchange CPU increment PC
							cpustate->PC.w.l = ADD12(cpustate->PC.w.l,1);
						}
						break;
			// Shift, Rotate, Serial I/O Instructions
			case 0x19: 	// SIO
						cpustate->icount -= 5;
						devcb_call_write_line(&cpustate->sout_func, cpustate->ER & 0x01);
						cpustate->ER >>= 1;
						cpustate->ER |= devcb_call_read_line(&cpustate->sin_func) ? 0x80 : 0x00;
						break;
			case 0x1c: 	// SR
						cpustate->icount -= 5;
						cpustate->AC >>= 1;
						break;
			case 0x1d: 	// SRL
						cpustate->icount -= 5;
						cpustate->AC >>= 1;
						cpustate->AC |= cpustate->SR & 0x80; // add C/L flag
						break;
			case 0x1e: 	// RR
						cpustate->icount -= 5;
						cpustate->AC =  (cpustate->AC >> 1) | ((cpustate->AC & 0x01) << 7);
						break;
			case 0x1f: 	// RRL
						cpustate->icount -= 5;
						tmp = (cpustate->AC & 0x01) << 7;
						cpustate->AC =  (cpustate->AC >> 1) | (cpustate->SR & 0x80);
						cpustate->SR = (cpustate->SR & 0x7f) | tmp;
						break;
			// Single Byte Miscellaneous Instructions
			case 0x00: 	// HALT
						cpustate->icount -= 8;
						devcb_call_write_line(&cpustate->halt_func, 1);
						devcb_call_write_line(&cpustate->halt_func, 0);
						break;
			case 0x02: 	// CCL
						cpustate->icount -= 5;
						cpustate->SR &= 0x7f;
						break;
			case 0x03: 	// SCL
						cpustate->icount -= 5;
						cpustate->SR |= 0x80;
						break;
			case 0x04: 	// DINT
						cpustate->icount -= 6;
						cpustate->SR &= 0xf7;
						break;
			case 0x05: 	// IEN
						cpustate->icount -= 6;
						cpustate->SR |= 0x08;
						break;
			case 0x06: 	// CSA
						cpustate->icount -= 5;
						cpustate->SR &= 0xcf; // clear SA and SB flags
						cpustate->SR |= devcb_call_read_line(&cpustate->sensea_func) ? 0x10 : 0x00;
						cpustate->SR |= devcb_call_read_line(&cpustate->senseb_func) ? 0x20 : 0x00;
						cpustate->AC = cpustate->SR;
						break;
			case 0x07: 	// CAS
						cpustate->icount -= 6;
						cpustate->SR = cpustate->AC;
						devcb_call_write8(&cpustate->flag_out_func, 0, cpustate->SR & 0x07);
						break;
			case 0x08: 	// NOP
						cpustate->icount -= 5;
						break;
			// Others are illegal
			default : 	cpustate->icount -= 1;
						illegal (cpustate,opcode);
						break;
		}
	}
}


/***************************************************************************
    COMMON EXECUTION
***************************************************************************/
static void take_interrupt(scmp_state *cpustate)
{
	UINT16 tmp = ADD12(cpustate->PC.w.l,-1); // We fix PC so at return it goes to current location
	cpustate->SR &= 0xf7; // clear IE flag

	cpustate->icount -= 8; // assumption
	// do XPPC 3
	cpustate->PC.w.l = GET_PTR_REG(cpustate,3)->w.l;
	GET_PTR_REG(cpustate,3)->w.l = tmp;
	// After exchange CPU increment PC
	cpustate->PC.w.l = ADD12(cpustate->PC.w.l,1);
}

static CPU_EXECUTE( scmp )
{
	scmp_state *cpustate = get_safe_token(device);

	cpustate->icount = cycles;

	do
	{
		if ((cpustate->SR & 0x08) && (devcb_call_read_line(&cpustate->sensea_func))) {
			take_interrupt(cpustate);
		}
		debugger_instruction_hook(device, cpustate->PC.d);
		execute_one(cpustate, ROP(cpustate));

	} while (cpustate->icount > 0);

	return cycles - cpustate->icount;
}

/***************************************************************************
    CORE INITIALIZATION
***************************************************************************/

static CPU_INIT( scmp )
{
	scmp_state *cpustate = get_safe_token(device);

	if (device->static_config != NULL)
		cpustate->config = *(scmp_config *)device->static_config;
	/* set up the state table */
	cpustate->state = state_table_template;
	cpustate->state.baseptr = cpustate;
	cpustate->state.subtypemask = 1;

	cpustate->device = device;

	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);

	/* resolve callbacks */
	devcb_resolve_write8(&cpustate->flag_out_func, &cpustate->config.flag_out_func, device);
	devcb_resolve_write_line(&cpustate->sout_func, &cpustate->config.sout_func, device);
	devcb_resolve_read_line(&cpustate->sin_func, &cpustate->config.sin_func, device);
	devcb_resolve_read_line(&cpustate->sensea_func, &cpustate->config.sensea_func, device);
	devcb_resolve_read_line(&cpustate->senseb_func, &cpustate->config.senseb_func, device);
	devcb_resolve_write_line(&cpustate->halt_func, &cpustate->config.halt_func, device);

	state_save_register_device_item(device, 0, cpustate->PC);
	state_save_register_device_item(device, 0, cpustate->P1);
	state_save_register_device_item(device, 0, cpustate->P2);
	state_save_register_device_item(device, 0, cpustate->P3);
	state_save_register_device_item(device, 0, cpustate->AC);
	state_save_register_device_item(device, 0, cpustate->ER);
	state_save_register_device_item(device, 0, cpustate->SR);
}



/***************************************************************************
    COMMON RESET
***************************************************************************/

static CPU_RESET( scmp )
{
	scmp_state *cpustate = get_safe_token(device);

	cpustate->PC.d = 0;
	cpustate->P1.d = 0;
	cpustate->P2.d = 0;
	cpustate->P3.d = 0;
	cpustate->AC = 0;
	cpustate->ER = 0;
	cpustate->SR = 0;
}



/***************************************************************************
    COMMON STATE IMPORT/EXPORT
***************************************************************************/

static CPU_IMPORT_STATE( scmp )
{
}

static CPU_EXPORT_STATE( scmp )
{
}

/***************************************************************************
    COMMON SET INFO
***************************************************************************/
static CPU_SET_INFO( scmp )
{
}

/***************************************************************************
    SCMP GET INFO
***************************************************************************/

CPU_GET_INFO( scmp )
{
	scmp_state *cpustate = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(scmp_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 0;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 5;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 131593;						break; // DLY instruction max time

		case CPUINFO_INT_DATABUS_WIDTH_PROGRAM:			info->i = 8;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH_PROGRAM: 		info->i = 16;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT_PROGRAM: 		info->i = 0;							break;

		case CPUINFO_INT_DATABUS_WIDTH_DATA:			info->i = 0;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH_DATA: 			info->i = 0;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT_DATA: 			info->i = 0;							break;

		case CPUINFO_INT_DATABUS_WIDTH_IO:				info->i = 0;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH_IO: 				info->i = 0;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT_IO: 				info->i = 0;							break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_SET_INFO:		info->setinfo = CPU_SET_INFO_NAME(scmp);				break;
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(scmp);						break;
		case CPUINFO_FCT_RESET:			info->reset = CPU_RESET_NAME(scmp);						break;
		case CPUINFO_FCT_EXECUTE:		info->execute = CPU_EXECUTE_NAME(scmp);					break;
		case CPUINFO_FCT_DISASSEMBLE:	info->disassemble = CPU_DISASSEMBLE_NAME(scmp);			break;
		case CPUINFO_FCT_IMPORT_STATE:	info->import_state = CPU_IMPORT_STATE_NAME(scmp);		break;
		case CPUINFO_FCT_EXPORT_STATE:	info->export_state = CPU_EXPORT_STATE_NAME(scmp);		break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;		break;
		case CPUINFO_PTR_STATE_TABLE:					info->state_table = &cpustate->state;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "SC/MP");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "National Semiconductor SC/MP");			break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:				strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Miodrag Milanovic"); break;

		case CPUINFO_STR_FLAGS:
		 	sprintf(info->s, "%c%c%c%c%c%c%c%c",
			  (cpustate->SR & 0x80) ? 'C' : '.',
			  (cpustate->SR & 0x40) ? 'V' : '.',
			  (cpustate->SR & 0x20) ? 'B' : '.',
			  (cpustate->SR & 0x10) ? 'A' : '.',
			  (cpustate->SR & 0x08) ? 'I' : '.',
			  (cpustate->SR & 0x04) ? '2' : '.',
			  (cpustate->SR & 0x02) ? '1' : '.',
			  (cpustate->SR & 0x01) ? '0' : '.');
			break;
	}
}
