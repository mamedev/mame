/*****************************************************************************
 *
 *   pps4.c
 *
 *   Rockwell PPS-4 CPU
 *
 *   Initial version by Miodrag Milanovic
 *
 *****************************************************************************/
#include "emu.h"
#include "debugger.h"
#include "pps4.h"

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct pps4_state
{
	UINT8	A; // Accumulator
	UINT8	X;

	PAIR	P;
	PAIR    SA;
	PAIR    SB;
	PAIR	B; // BU + BM + BL

	UINT8	C; // Carry flag
	UINT8	FF1; // Flip-flop 1
	UINT8	FF2; // Flip-flop 2

	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *data;
	address_space *io;

	int					icount;
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE pps4_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == PPS4);
	return (pps4_state *)downcast<legacy_cpu_device *>(device)->token();
}

INLINE UINT8 ROP(pps4_state *cpustate)
{
	UINT8 retVal = cpustate->direct->read_decrypted_byte(cpustate->P.w.l);
	cpustate->P.w.l = (cpustate->P.w.l + 1) & 0x0fff;
	return retVal;
}

INLINE UINT8 ARG(pps4_state *cpustate)
{
	UINT8 retVal = cpustate->direct->read_raw_byte(cpustate->P.w.l);
	cpustate->P.w.l = (cpustate->P.w.l + 1) & 0x0fff;
	return retVal;
}

INLINE void DO_SKIP(pps4_state *cpustate)
{
	cpustate->P.w.l = (cpustate->P.w.l + 1) & 0x0fff;
}

static void execute_one(pps4_state *cpustate, int opcode)
{
	cpustate->icount -= 1;
	switch (opcode)
	{
		// Arithmetic instructions
		case 0x0b:	// AD
					break;
		case 0x0a:	// ADC
					break;
		case 0x09:	// ADSK
					break;
		case 0x08:	// ADCSK
					break;
		case 0x60:	case 0x61:	case 0x62:	case 0x63:
		case 0x64:	case 0x66:	case 0x67:	case 0x68:
		case 0x69:	case 0x6a:	case 0x6b:	case 0x6c:
		case 0x6d:	case 0x6e:
					// ADI
					break;
		case 0x65:	//DC
					cpustate->A = (cpustate->A + 10) & 0x0f;
					break;
		// Logical instructions
		case 0x0d:	// AND
					break;
		case 0x0f:	// OR
					break;
		case 0x0c:	// EOR
					break;
		case 0x0e:	// COMP
					cpustate->A ^= 0x0f;
					break;
		// Data transfer instructions
		case 0x20:	// SC
					cpustate->C = 1;
					break;
		case 0x24:	//RC
					cpustate->C = 0;
					break;
		case 0x22:	// SF1
					cpustate->FF1 = 1;
					break;
		case 0x26:	// RF1
					cpustate->FF1 = 0;
					break;
		case 0x21:	// SF2
					cpustate->FF2 = 1;
					break;
		case 0x25:	// RF2
					cpustate->FF2 = 0;
					break;
		case 0x30:	case 0x31:	case 0x32:	case 0x33:
		case 0x34:	case 0x35:	case 0x36:	case 0x37:
					// LD
					break;
		case 0x38:	case 0x39:	case 0x3a:	case 0x3b:
		case 0x3c:	case 0x3d:	case 0x3e:	case 0x3f:
					// EX
					break;
		case 0x28:	case 0x29:	case 0x2a:	case 0x2b:
		case 0x2c:	case 0x2d:	case 0x2e:	case 0x2f:
					// EXD
					break;
		case 0x70:	case 0x71:	case 0x72:	case 0x73:
		case 0x74:	case 0x75:	case 0x76:	case 0x77:
		case 0x78:	case 0x79:	case 0x7a:	case 0x7b:
		case 0x7c:	case 0x7d:	case 0x7e:	case 0x7f:
					// LDI
					cpustate->A = opcode & 0x0f;
					break;
		case 0x12:	// LAX
					cpustate->A = cpustate->X;
					break;
		case 0x1b:	// LXA
					cpustate->X = cpustate->A;
					break;
		case 0x11:	// LABL
					cpustate->A = cpustate->B.w.l & 0x00f;
					break;
		case 0x10:	// LBMX
					cpustate->B.w.l &= 0xf0f;
					cpustate->B.w.l |= (cpustate->X << 4);
					break;
		case 0x04:	// LBUA
					break;
		case 0x19:	// XABL
					{
						UINT8 tmp = cpustate->B.w.l & 0x00f;
						cpustate->B.w.l &= 0xff0;
						cpustate->B.w.l |= cpustate->A;
						cpustate->A = tmp;
					}
					break;
		case 0x18:	// XBMX
					{
						UINT8 tmp = (cpustate->B.w.l & 0x0f0) >> 4;
						cpustate->B.w.l &= 0xf0f;
						cpustate->B.w.l |= (cpustate->X << 4);
						cpustate->X = tmp;
					}
					break;
		case 0x1a:	// XAX
					{
						UINT8 tmp = cpustate->A;
						cpustate->A = cpustate->X;
						cpustate->X = tmp;
					}
					break;
		case 0x06:	// XS
					{
						PAIR tmp = cpustate->SA;
						cpustate->SA = cpustate->SB;
						cpustate->SB = tmp;
					}
					break;
		case 0x6f:	// CYS
					break;
		case 0xc0:	case 0xc1:	case 0xc2:	case 0xc3:
		case 0xc4:	case 0xc5:	case 0xc6:	case 0xc7:
		case 0xc8:	case 0xc9:	case 0xca:	case 0xcb:
		case 0xcc:	case 0xcd:	case 0xce:	case 0xcf:
					// LB
					{
						//UINT8 tmp = ARG(cpustate);
						cpustate->icount -= 1;
					}
					break;
		case 0x00:	// LBL
					{
						UINT8 tmp = ARG(cpustate);
						cpustate->icount -= 1;
						cpustate->B.w.l = tmp;
					}
					break;
		case 0x17:	// INCB
					if ((cpustate->B.w.l & 0x0f) == 0x0f) {
						cpustate->B.w.l &= 0xff0;
						DO_SKIP(cpustate);
					} else {
						cpustate->B.w.l += 1;
					}
					break;
		case 0x1f:	// DECB
					if ((cpustate->B.w.l & 0x0f) == 0x00) {
						cpustate->B.w.l |= 0x00f;
						DO_SKIP(cpustate);
					} else {
						cpustate->B.w.l -= 1;
					}
					break;
		// Control transfer instructions
		case 0x80:	case 0x81:	case 0x82:	case 0x83:
		case 0x84:	case 0x85:	case 0x86:	case 0x87:
		case 0x88:	case 0x89:	case 0x8a:	case 0x8b:
		case 0x8c:	case 0x8d:	case 0x8e:	case 0x8f:
		case 0x90:	case 0x91:	case 0x92:	case 0x93:
		case 0x94:	case 0x95:	case 0x96:	case 0x97:
		case 0x98:	case 0x99:	case 0x9a:	case 0x9b:
		case 0x9c:	case 0x9d:	case 0x9e:	case 0x9f:
		case 0xa0:	case 0xa1:	case 0xa2:	case 0xa3:
		case 0xa4:	case 0xa5:	case 0xa6:	case 0xa7:
		case 0xa8:	case 0xa9:	case 0xaa:	case 0xab:
		case 0xac:	case 0xad:	case 0xae:	case 0xaf:
		case 0xb0:	case 0xb1:	case 0xb2:	case 0xb3:
		case 0xb4:	case 0xb5:	case 0xb6:	case 0xb7:
		case 0xb8:	case 0xb9:	case 0xba:	case 0xbb:
		case 0xbc:	case 0xbd:	case 0xbe:	case 0xbf:
					// T
					cpustate->P.w.l = (cpustate->P.w.l & 0xfc0) | (opcode & 0x3f);
					break;
		case 0xd0:	case 0xd1:	case 0xd2:	case 0xd3:
		case 0xd4:	case 0xd5:	case 0xd6:	case 0xd7:
		case 0xd8:	case 0xd9:	case 0xda:	case 0xdb:
		case 0xdc:	case 0xdd:	case 0xde:	case 0xdf:
		case 0xe0:	case 0xe1:	case 0xe2:	case 0xe3:
		case 0xe4:	case 0xe5:	case 0xe6:	case 0xe7:
		case 0xe8:	case 0xe9:	case 0xea:	case 0xeb:
		case 0xec:	case 0xed:	case 0xee:	case 0xef:
		case 0xf0:	case 0xf1:	case 0xf2:	case 0xf3:
		case 0xf4:	case 0xf5:	case 0xf6:	case 0xf7:
		case 0xf8:	case 0xf9:	case 0xfa:	case 0xfb:
		case 0xfc:	case 0xfd:	case 0xfe:	case 0xff:
					// TM
					break;
		case 0x50:	case 0x51:	case 0x52:	case 0x53:
		case 0x54:	case 0x55:	case 0x56:	case 0x57:
		case 0x58:	case 0x59:	case 0x5a:	case 0x5b:
		case 0x5c:	case 0x5d:	case 0x5e:	case 0x5f:
					// TL
					{
						//UINT8 tmp = ARG(cpustate);
						cpustate->icount -= 1;
					}
					break;
		case 0x01:	case 0x02:	case 0x03:
					// TML
					{
						//UINT8 tmp = ARG(cpustate);
						cpustate->icount -= 1;
					}
					break;
		case 0x15:	// SKC
					break;
		case 0x1e:	// SKZ
					break;
		case 0x40:	case 0x41:	case 0x42:	case 0x43:
		case 0x44:	case 0x45:	case 0x46:	case 0x47:
		case 0x48:	case 0x49:	case 0x4a:	case 0x4b:
		case 0x4c:	case 0x4d:	case 0x4e:	case 0x4f:
					// SKBI
					break;
		case 0x16:	// SKF1
					break;
		case 0x14:	// SKF2
					break;
		case 0x05:	// RTN
					break;
		case 0x07:	// RTNSK
					break;
		// Input/Output instructions
		case 0x1c:	// IOL
					{
						//UINT8 tmp = ARG(cpustate);
						cpustate->icount -= 1;
					}
					break;
		case 0x27:	// DIA
					break;
		case 0x23:	// DIB
					break;
		case 0x1d:	// DOA
					break;
		// Special instructions
		case 0x13:	// SAG
					break;
	}
}


/***************************************************************************
    COMMON EXECUTION
***************************************************************************/
static CPU_EXECUTE( pps4 )
{
	pps4_state *cpustate = get_safe_token(device);

	do
	{
		debugger_instruction_hook(device, cpustate->P.d);
		execute_one(cpustate, ROP(cpustate));

	} while (cpustate->icount > 0);
}

/***************************************************************************
    CORE INITIALIZATION
***************************************************************************/

static CPU_INIT( pps4 )
{
	pps4_state *cpustate = get_safe_token(device);

	cpustate->device = device;

	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = device->space(AS_DATA);
	cpustate->io = device->space(AS_IO);

	device->save_item(NAME(cpustate->A));
	device->save_item(NAME(cpustate->X));
	device->save_item(NAME(cpustate->P));
	device->save_item(NAME(cpustate->SA));
	device->save_item(NAME(cpustate->SB));
	device->save_item(NAME(cpustate->B));
	device->save_item(NAME(cpustate->C));
	device->save_item(NAME(cpustate->FF1));
	device->save_item(NAME(cpustate->FF2));
}

/***************************************************************************
    COMMON RESET
***************************************************************************/

static CPU_RESET( pps4 )
{
	pps4_state *cpustate = get_safe_token(device);

	cpustate->A = cpustate->X = 0;
	cpustate->C = cpustate->FF1 = cpustate->FF2 = 0;

	cpustate->P.d = 0;
	cpustate->SA.d = 0;
	cpustate->SB.d = 0;
	cpustate->B.d = 0;
}

/***************************************************************************
    COMMON STATE IMPORT/EXPORT
***************************************************************************/

static CPU_IMPORT_STATE( pps4 )
{
}

static CPU_EXPORT_STATE( pps4 )
{
}

/***************************************************************************
    COMMON SET INFO
***************************************************************************/
static CPU_SET_INFO( pps4 )
{
}

/***************************************************************************
    PPS4 GET INFO
***************************************************************************/

CPU_GET_INFO( pps4 )
{
	pps4_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(pps4_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 0;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 2;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 8;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:		info->i = 12;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:		info->i = 0;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:			info->i = 8;							break; // 4 bit for RAM
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:			info->i = 12;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:			info->i = 0;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:				info->i = 8;							break; // 4 bit
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:				info->i = 8;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:				info->i = 0;							break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_SET_INFO:		info->setinfo = CPU_SET_INFO_NAME(pps4);				break;
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(pps4);						break;
		case CPUINFO_FCT_RESET:			info->reset = CPU_RESET_NAME(pps4);					break;
		case CPUINFO_FCT_EXECUTE:		info->execute = CPU_EXECUTE_NAME(pps4);				break;
		case CPUINFO_FCT_DISASSEMBLE:	info->disassemble = CPU_DISASSEMBLE_NAME(pps4);		break;
		case CPUINFO_FCT_IMPORT_STATE:	info->import_state = CPU_IMPORT_STATE_NAME(pps4);		break;
		case CPUINFO_FCT_EXPORT_STATE:	info->export_state = CPU_EXPORT_STATE_NAME(pps4);		break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:						strcpy(info->s, "PPS4");				break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "Rockwell PPS-4");			break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_SOURCE_FILE:				strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Miodrag Milanovic"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c",
				cpustate->C ? 'C':'.',
				cpustate->FF1 ? '1':'.',
				cpustate->FF2 ? '2':'.');
			break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(PPS4, pps4);
