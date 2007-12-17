/*

 Hyperstone disassembler
 written by Pierpaolo Prazzoli

*/

#include <math.h>
#include "debugger.h"
#include "e132xs.h"

static const char *L_REG[] =
{
  "L0",  "L1",  "L2",  "L3",  "L4",  "L5",  "L6",  "L7",  "L8",  "L9",
  "L10", "L11", "L12", "L13", "L14", "L15", "L16", "L17", "L18", "L19",
  "L20", "L21", "L22", "L23", "L24", "L25", "L26", "L27", "L28", "L29",
  "L30", "L31", "L32", "L33", "L34", "L35", "L36", "L37", "L38", "L39",
  "L40", "L41", "L42", "L43", "L44", "L45", "L46", "L47", "L48", "L49",
  "L50", "L51", "L52", "L53", "L54", "L55", "L56", "L57", "L58", "L59",
  "L60", "L61", "L62", "L63"
};

static const char *G_REG[] =
{
  "PC",  "SR",  "FER", "G03", "G04", "G05", "G06", "G07", "G08", "G09",
  "G10", "G11", "G12", "G13", "G14", "G15", "G16", "G17", "SP",  "UB",
  "BCR", "TPR", "TCR", "TR",  "WCR", "ISR", "FCR", "MCR", "G28", "G29",
  "G30", "G31"
};

static const char *SETxx[] =
{
  "SETADR",   "Reserved", "SET1",   "SET0",     "SETLE",  "SETGT",  "SETLT",  "SETGE",
  "SETSE",    "SETHT",    "SETST",  "SETHE",    "SETE",   "SETNE",  "SETV",   "SETNV",
  "Reserved", "Reserved", "SET1M",  "Reserved", "SETLEM", "SETGTM", "SETLTM", "SETGEM",
  "SETSEM",   "SETTHM",   "SETSTM", "SETHEM",   "SETEM",  "SETNEM", "SETVM",  "SETNVM"
};

#define DESTCODE(op)			((op & 0x00f0) >> 4)
#define SOURCECODE(op)			(op & 0x000f)

#define SOURCEBIT(op)			((op & 0x100) >> 8)
#define DESTBIT(op)				((op & 0x200) >> 9)

#define N_VALUE(op)				((((op & 0x100) >> 8) << 4 ) | (op & 0x0f))

static int size, global_fp;

static offs_t base_pc;
static const UINT8 *base_oprom;
#define READ_OP_DASM(p)			((base_oprom[(p) - base_pc] << 8) | base_oprom[(p) + 1 - base_pc])


static void LL_format(char *source, char *dest, UINT16 op)
{
	strcpy(source, L_REG[(SOURCECODE(op)+global_fp)%64]);
	strcpy(dest, L_REG[(DESTCODE(op)+global_fp)%64]);
}

static void LR_format(char *source, char *dest, UINT16 op)
{
	if( SOURCEBIT(op) )
	{
		strcpy(source, L_REG[(SOURCECODE(op)+global_fp)%64]);
	}
	else
	{
		strcpy(source, G_REG[SOURCECODE(op)]);
	}

	strcpy(dest, L_REG[(DESTCODE(op)+global_fp)%64]);
}

static void RR_format(char *source, char *dest, UINT16 op, unsigned h_flag)
{
	if( SOURCEBIT(op) )
	{
		strcpy(source, L_REG[(SOURCECODE(op)+global_fp)%64]);
	}
	else
	{
		strcpy(source, G_REG[SOURCECODE(op) + h_flag * 16]);
	}

	if( DESTBIT(op) )
	{
		strcpy(dest, L_REG[(DESTCODE(op)+global_fp)%64]);
	}
	else
	{
		strcpy(dest, G_REG[DESTCODE(op) + h_flag * 16]);
	}
}

static UINT32 LRconst_format(char *source, char *dest, UINT16 op, unsigned *pc)
{
	UINT16 next_op;
	UINT32 const_val;

	if( SOURCEBIT(op) )
	{
		strcpy(source, L_REG[(SOURCECODE(op)+global_fp)%64]);
	}
	else
	{
		strcpy(source, G_REG[SOURCECODE(op)]);
	}

	strcpy(dest, L_REG[(DESTCODE(op)+global_fp)%64]);

	size = 4;

	*pc += 2;
	next_op = READ_OP_DASM(*pc);

	if( E_BIT(next_op) )
	{
		UINT16 next_op2;

		size = 6;

		*pc += 2;
		next_op2 = READ_OP_DASM(*pc);
		const_val = next_op2;
		const_val |= ((next_op & 0x3fff) << 16 );

		if( S_BIT_CONST(next_op) )
		{
			const_val |= 0xc0000000;
		}
	}
	else
	{
		const_val = next_op & 0x3fff;

		if( S_BIT_CONST(next_op) )
		{
			const_val |= 0xffffc000;
		}
	}

	return const_val;
}

static UINT32 RRconst_format(char *source, char *dest, UINT16 op, unsigned *pc)
{
	UINT16 next_op;
	UINT32 const_val;

	if( SOURCEBIT(op) )
	{
		strcpy(source, L_REG[(SOURCECODE(op)+global_fp)%64]);
	}
	else
	{
		strcpy(source, G_REG[SOURCECODE(op)]);
	}

	if( DESTBIT(op) )
	{
		strcpy(dest, L_REG[(DESTCODE(op)+global_fp)%64]);
	}
	else
	{
		strcpy(dest, G_REG[DESTCODE(op)]);
	}

	size = 4;

	*pc += 2;
	next_op = READ_OP_DASM(*pc);

	if( E_BIT(next_op) )
	{
		UINT16 next_op2;

		size = 6;

		*pc += 2;
		next_op2 = READ_OP_DASM(*pc);
		const_val = next_op2;
		const_val |= ((next_op & 0x3fff) << 16 );

		if( S_BIT_CONST(next_op) )
		{
			const_val |= 0xc0000000;
		}
	}
	else
	{
		const_val = next_op & 0x3fff;

		if( S_BIT_CONST(next_op) )
		{
			const_val |= 0xffffc000;
		}
	}

	return const_val;
}

static INT32 Rimm_format(char *dest, UINT16 op, unsigned *pc, unsigned h_flag)
{
	UINT16 imm1, imm2;
	INT32 ret;

	int n = N_VALUE(op);

	if( DESTBIT(op) )
	{
		strcpy(dest, L_REG[(DESTCODE(op)+global_fp)%64]);
	}
	else
	{
		strcpy(dest, G_REG[DESTCODE(op) + h_flag * 16]);
	}

	switch( n )
	{
		case 0:	case 1:  case 2:  case 3:  case 4:  case 5:  case 6:  case 7: case 8:
		case 9:	case 10: case 11: case 12: case 13: case 14: case 15: case 16:
			return n;

		case 17:
			*pc += 2;
			imm1 = READ_OP_DASM(*pc);
			*pc += 2;
			imm2 = READ_OP_DASM(*pc);
			ret = (imm1 << 16) | imm2;

			size = 6;
			return ret;


		case 18:
			*pc += 2;
			ret = READ_OP_DASM(*pc);

			size = 4;
			return ret;

		case 19:
			*pc += 2;
			ret = (INT32) (0xffff0000 | READ_OP_DASM(*pc));

			size = 4;
			return ret;

		case 20:
			return 32;	//bit 5 = 1, others = 0

		case 21:
			return 64;	//bit 6 = 1, others = 0

		case 22:
			return 128; //bit 7 = 1, others = 0

		case 23:
			return 0x80000000; //bit 31 = 1, others = 0 (2 at the power of 31)

		case 24:
			return -8;

		case 25:
			return -7;

		case 26:
			return -6;

		case 27:
			return -5;

		case 28:
			return -4;

		case 29:
			return -3;

		case 30:
			return -2;

		case 31:
			return -1;

		default:
			return 0; //should never goes here
	}
}

static UINT8 Ln_format(char *dest, UINT16 op)
{
	strcpy(dest, L_REG[(DESTCODE(op)+global_fp)%64]);

	return N_VALUE(op);
}

static UINT8 Rn_format(char *dest, UINT16 op)
{
	if( DESTBIT(op) )
	{
		strcpy(dest, L_REG[(DESTCODE(op)+global_fp)%64]);
	}
	else
	{
		strcpy(dest, G_REG[DESTCODE(op)]);
	}

	return N_VALUE(op);
}

static INT32 PCrel_format(UINT16 op, unsigned pc)
{
	INT32 ret;

	if( op & 0x80 ) //bit 7 = 1
	{
		UINT16 next;

		size = 4;

		pc += 2;

		next = READ_OP_DASM(pc);

		ret = (op & 0x7f) << 16;

		ret |= (next & 0xfffe);

		if( next & 1 )
			ret |= 0xff800000; //ok?
	}
	else
	{
		ret = op & 0x7e;

		if( op & 1 )
			ret |= 0xffffff80; //ok?
	}

	return (pc + ret);
}

static UINT32 RRdis_format(char *source, char *dest, UINT16 op, UINT16 next_op, unsigned pc)
{
	UINT32 ret;

	if( SOURCEBIT(op) )
	{
		strcpy(source, L_REG[(SOURCECODE(op)+global_fp)%64]);
	}
	else
	{
		strcpy(source, G_REG[SOURCECODE(op)]);
	}

	if( DESTBIT(op) )
	{
		strcpy(dest, L_REG[(DESTCODE(op)+global_fp)%64]);
	}
	else
	{
		strcpy(dest, G_REG[DESTCODE(op)]);
	}

	if( E_BIT(next_op) )
	{
		UINT16 next;

		size = 6;

		next = READ_OP_DASM(pc + 4);

		ret = next;
		ret |= ( ( next_op & 0xfff ) << 16 );

		if( S_BIT_CONST(next_op) )
		{
			ret |= 0xf0000000;
		}
	}
	else
	{
		ret = next_op & 0xfff;
		if( S_BIT_CONST(next_op) )
		{
			ret |= 0xfffff000;
		}
	}

	return ret;
}

unsigned dasm_hyperstone(char *buffer, unsigned pc, const UINT8 *oprom, unsigned h_flag, int private_fp)
{
	UINT16 op = 0;
	UINT8 op_num;

	UINT8 source_code, dest_code, source_bit, dest_bit;

	char source[5] = "\0", dest[5] = "\0";
	UINT32 flags = 0;

	base_pc = pc;
	base_oprom = oprom;

	op = READ_OP_DASM(pc);

	size = 2;

	source_code = SOURCECODE(op);
	dest_code = DESTCODE(op);
	source_bit = SOURCEBIT(op);
	dest_bit = DESTBIT(op);

	global_fp = private_fp;

	op_num = (op & 0xff00) >> 8;

	switch( op_num )
	{
		// CHK - CHKZ - NOP
		case 0x00: case 0x01: case 0x02: case 0x03:

			if( source_bit && dest_bit && source_code == 0 && dest_code == 0 )
			{
				sprintf(buffer, "NOP");
			}
			else
			{
				RR_format(source, dest, op, 0);

				if( !source_bit && source_code == SR_REGISTER )
				{
					sprintf(buffer, "CHKZ %s, 0", dest);
				}
				else
				{
					sprintf(buffer, "CHK %s, %s", dest, source);
				}
			}

			break;

		// MOVD - RET
		case 0x04: case 0x05: case 0x06: case 0x07:

			RR_format(source, dest, op, 0);

			if( dest_code == PC_REGISTER && !dest_bit )
			{
				global_fp = 0;
				RR_format(source, dest, op, 0);
				sprintf(buffer, "RET PC, %s", source);
				flags = DASMFLAG_STEP_OUT;
			}
			else if( source_code == SR_REGISTER && !source_bit )
			{
				sprintf(buffer, "MOVD %s, 0", dest);
			}
			else
			{
				sprintf(buffer, "MOVD %s, %s", dest, source);
			}

			break;

		// DIVU
		case 0x08: case 0x09: case 0x0a: case 0x0b:

			RR_format(source, dest, op, 0);
			sprintf(buffer, "DIVU %s, %s", dest, source);

			break;

		// DIVS
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:

			RR_format(source, dest, op, 0);
			sprintf(buffer, "DIVS %s, %s", dest, source);

			break;

		// XMx - XXx
		case 0x10: case 0x11: case 0x12: case 0x13:
		{
			int xcode;

			RR_format(source, dest, op, 0);

			size = 4;

			pc += 2;
			op = READ_OP_DASM(pc);

			xcode = X_CODE(op);

			if( xcode < 4 )
			{
				UINT16 lim;

				if( E_BIT(op) )
				{
					UINT16 next_op;

					size = 6;

					pc += 2;
					next_op = READ_OP_DASM(pc);

					lim = ((op & 0xfff) << 16) | next_op;
				}
				else
				{
					lim = op & 0xfff;
				}

				sprintf(buffer, "XM%x %s, %s, $%x", (UINT8)(float) pow(2, xcode), dest, source, lim);

			}
			else
			{
				sprintf(buffer, "XX%x %s, %s, 0", (UINT8)(float) pow(2, (xcode - 4)), dest, source);
			}

			break;
		}

		// MASK
		case 0x14: case 0x15: case 0x16: case 0x17:
		{
			UINT32 const_val = RRconst_format(source, dest, op, &pc);

			sprintf(buffer, "MASK %s, %s, $%x", dest, source, const_val);

			break;
		}

		// SUM
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		{
			UINT32 const_val = RRconst_format(source, dest, op, &pc);

			if( source_code == SR_REGISTER && !source_bit )
			{
				sprintf(buffer, "SUM %s, C, $%x", dest, const_val);
			}
			else
			{
				sprintf(buffer, "SUM %s, %s, $%x", dest, source, const_val);
			}

			break;
		}

		// SUMS
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		{
			UINT32 const_val = RRconst_format(source, dest, op, &pc);

			if( source_code == SR_REGISTER && !source_bit )
			{
				sprintf(buffer, "SUMS %s, C, $%x", dest, const_val);
			}
			else
			{
				sprintf(buffer, "SUMS %s, %s, $%x", dest, source, const_val);
			}

			break;
		}

		// CMP
		case 0x20: case 0x21: case 0x22: case 0x23:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				sprintf(buffer, "CMP %s, C", dest);
			}
			else
			{
				sprintf(buffer, "CMP %s, %s", dest, source);
			}

			break;

		// MOV
		case 0x24: case 0x25: case 0x26: case 0x27:

			RR_format(source, dest, op, h_flag);
			sprintf(buffer, "MOV %s, %s", dest, source);

			break;

		// ADD
		case 0x28: case 0x29: case 0x2a: case 0x2b:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				sprintf(buffer, "ADD %s, C", dest);
			}
			else
			{
				sprintf(buffer, "ADD %s, %s", dest, source);
			}

			break;

		// ADDS
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				sprintf(buffer, "ADDS %s, C", dest);
			}
			else
			{
				sprintf(buffer, "ADDS %s, %s", dest, source);
			}

			break;

		// CMPB
		case 0x30: case 0x31: case 0x32: case 0x33:

			RR_format(source, dest, op, 0);
			sprintf(buffer, "CMPB %s, %s", dest, source);

			break;

		// ANDN
		case 0x34: case 0x35: case 0x36: case 0x37:

			RR_format(source, dest, op, 0);
			sprintf(buffer, "ANDN %s, %s", dest, source);

			break;

		// OR
		case 0x38: case 0x39: case 0x3a: case 0x3b:

			RR_format(source, dest, op, 0);
			sprintf(buffer, "OR %s, %s", dest, source);

			break;

		// XOR
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:

			RR_format(source, dest, op, 0);
			sprintf(buffer, "XOR %s, %s", dest, source);

			break;

		// SUBC
		case 0x40: case 0x41: case 0x42: case 0x43:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				sprintf(buffer, "SUBC %s, C", dest);
			}
			else
			{
				sprintf(buffer, "SUBC %s, %s", dest, source);
			}

			break;

		// NOT
		case 0x44: case 0x45: case 0x46: case 0x47:

			RR_format(source, dest, op, 0);

			sprintf(buffer, "NOT %s, %s", dest, source);

			break;

		// SUB
		case 0x48: case 0x49: case 0x4a: case 0x4b:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				sprintf(buffer, "SUB %s, C", dest);
			}
			else
			{
				sprintf(buffer, "SUB %s, %s", dest, source);
			}

			break;

		// SUBS
		case 0x4c: case 0x4d: case 0x4e: case 0x4f:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				sprintf(buffer, "SUBS %s, C", dest);
			}
			else
			{
				sprintf(buffer, "SUBS %s, %s", dest, source);
			}

			break;

		// ADDC
		case 0x50: case 0x51: case 0x52: case 0x53:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				sprintf(buffer, "ADDC %s, C", dest);
			}
			else
			{
				sprintf(buffer, "ADDC %s, %s", dest, source);
			}

			break;

		// AND
		case 0x54: case 0x55: case 0x56: case 0x57:

			RR_format(source, dest, op, 0);
			sprintf(buffer, "AND %s, %s", dest, source);

			break;

		// NEG
		case 0x58: case 0x59: case 0x5a: case 0x5b:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				sprintf(buffer, "NEG %s, C", dest);
			}
			else
			{
				sprintf(buffer, "NEG %s, %s", dest, source);
			}

			break;

		// NEGS
		case 0x5c: case 0x5d: case 0x5e: case 0x5f:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				sprintf(buffer, "NEGS %s, C", dest);
			}
			else
			{
				sprintf(buffer, "NEGS %s, %s", dest, source);
			}

			break;

		// CMPI
		case 0x60: case 0x61: case 0x62: case 0x63:
		{
			UINT32 imm = Rimm_format(dest, op, &pc, 0);

			sprintf(buffer, "CMPI %s, $%x", dest, imm);

			break;
		}

		// MOVI
		case 0x64: case 0x65: case 0x66: case 0x67:
		{
			UINT32 imm = Rimm_format(dest, op, &pc, h_flag);

			sprintf(buffer, "MOVI %s, $%x", dest, imm);

			break;
		}

		// ADDI
		case 0x68: case 0x69: case 0x6a: case 0x6b:
		{
			UINT32 imm = Rimm_format(dest, op, &pc, 0);

			if( !N_VALUE(op) )
			{
				sprintf(buffer, "ADDI %s, CZ", dest);
			}
			else
			{
				sprintf(buffer, "ADDI %s, $%x", dest, imm);
			}

			break;
		}

		// ADDSI
		case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		{
			UINT32 imm = Rimm_format(dest, op, &pc, 0);

			if( !N_VALUE(op) )
			{
				sprintf(buffer, "ADDSI %s, CZ", dest);
			}
			else
			{
				sprintf(buffer, "ADDSI %s, $%x", dest, imm);
			}

			break;
		}

		// CMPBI
		case 0x70: case 0x71: case 0x72: case 0x73:
		{
			UINT32 imm = Rimm_format(dest, op, &pc, 0);

			if( !N_VALUE(op) )
			{
				sprintf(buffer, "CMPBI %s, ANYBZ", dest);
			}
			else
			{
				if( N_VALUE(op) == 31 )
					imm = 0x7fffffff; //bit 31 = 0, others = 1

				sprintf(buffer, "CMPBI %s, $%x", dest, imm);
			}

			break;
		}

		// ANDNI
		case 0x74: case 0x75: case 0x76: case 0x77:
		{
			UINT32 imm = Rimm_format(dest, op, &pc, 0);

			if( N_VALUE(op) == 31 )
				imm = 0x7fffffff; //bit 31 = 0, others = 1

			sprintf(buffer, "ANDNI %s, $%x", dest, imm);

			break;
		}

		// ORI
		case 0x78: case 0x79: case 0x7a: case 0x7b:
		{
			UINT32 imm = Rimm_format(dest, op, &pc, 0);

			sprintf(buffer, "ORI %s, $%x", dest, imm);

			break;
		}

		// XORI
		case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		{
			UINT32 imm = Rimm_format(dest, op, &pc, 0);

			sprintf(buffer, "XORI %s, $%x", dest, imm);

			break;
		}

		// SHRDI
		case 0x80: case 0x81:
		{
			UINT8 n = Ln_format(dest, op);

			sprintf(buffer, "SHRDI %s, $%x", dest, n);

			break;
		}

		// SHRD
		case 0x82:

			LL_format(source, dest, op);

			sprintf(buffer, "SHRD %s, %s", dest, source);

			break;

		// SHR
		case 0x83:

			LL_format(source, dest, op);

			sprintf(buffer, "SHR %s, %s", dest, source);

			break;

		// SARDI
		case 0x84: case 0x85:
		{
			UINT8 n = Ln_format(dest, op);

			sprintf(buffer, "SARDI %s, $%x", dest, n);

			break;
		}

		// SARD
		case 0x86:

			LL_format(source, dest, op);

			sprintf(buffer, "SARD %s, %s", dest, source);

			break;

		// SAR
		case 0x87:

			LL_format(source, dest, op);

			sprintf(buffer, "SAR %s, %s", dest, source);

			break;

		// SHLDI
		case 0x88: case 0x89:
		{
			UINT8 n = Ln_format(dest, op);

			sprintf(buffer, "SHLDI %s, $%x", dest, n);

			break;
		}

		// SHLD
		case 0x8a:

			LL_format(source, dest, op);

			sprintf(buffer, "SHLD %s, %s", dest, source);

			break;

		// SHL
		case 0x8b:

			LL_format(source, dest, op);

			sprintf(buffer, "SHL %s, %s", dest, source);

			break;

		// RESERVED
		case 0x8c: case 0x8d:
		case 0xac: case 0xad: case 0xae: case 0xaf:

			sprintf(buffer, "Reserved");

			break;

		// TESTLZ
		case 0x8e:

			LL_format(source, dest, op);

			sprintf(buffer, "TESTLZ %s, %s", dest, source);

			break;

		// ROL
		case 0x8f:

			LL_format(source, dest, op);

			sprintf(buffer, "ROL %s, %s", dest, source);

			break;

		// LDxx.D/A/IOD/IOA
		case 0x90: case 0x91: case 0x92: case 0x93:
		{
			UINT16 next_op = READ_OP_DASM(pc + 2);
			UINT32 dis = RRdis_format(source, dest, op, next_op, pc);

			if( size == 2 )
				size = 4;

			if( dest_code == SR_REGISTER && !dest_bit )
			{
				switch( DD( next_op ) )
				{
					case 0:
						// LDBS.A
						sprintf(buffer, "LDBS.A 0, %s, $%x", source, dis);
						break;

					case 1:
						// LDBU.A
						sprintf(buffer, "LDBU.A 0, %s, $%x", source, dis);
						break;

					case 2:
						// LDHS.A
						if( dis & 1 )
						{
							sprintf(buffer, "LDHS.A 0, %s, $%x", source, dis & ~1);
						}
						// LDHU.A
						else
						{
							sprintf(buffer, "LDHU.A 0, %s, $%x", source, dis & ~1);
						}

						break;

					case 3:
						// LDD.IOA
						if( (dis & 3) == 3 )
						{
							sprintf(buffer, "LDD.IOA 0, %s, $%x", source, dis & ~3);
						}
						// LDW.IOA
						else if( (dis & 3) == 2 )
						{
							sprintf(buffer, "LDW.IOA 0, %s, $%x", source, dis & ~3);
						}
						// LDD.A
						else if( (dis & 3) == 1 )
						{
							sprintf(buffer, "LDD.A 0, %s, $%x", source, dis & ~1);
						}
						// LDW.A
						else
						{
							sprintf(buffer, "LDW.A 0, %s, $%x", source, dis & ~1);
						}

						break;
				}
			}
			else
			{
				switch( DD( next_op ) )
				{
					case 0:
						// LDBS.D
						sprintf(buffer, "LDBS.D %s, %s, $%x", dest, source, dis);
						break;

					case 1:
						// LDBU.D
						sprintf(buffer, "LDBU.D %s, %s, $%x", dest, source, dis);
						break;

					case 2:
						// LDHS.D
						if( dis & 1 )
						{
							sprintf(buffer, "LDHS.D %s, %s, $%x", dest, source, dis & ~1);
						}
						// LDHU.D
						else
						{
							sprintf(buffer, "LDHU.D %s, %s, $%x", dest, source, dis & ~1);
						}
						break;

					case 3:
						// LDD.IOD
						if( (dis & 3) == 3 )
						{
							sprintf(buffer, "LDD.IOD %s, %s, $%x", dest, source, dis & ~3);
						}
						// LDW.IOD
						else if( (dis & 3) == 2 )
						{
							sprintf(buffer, "LDW.IOD %s, %s, $%x", dest, source, dis & ~3);
						}
						// LDD.D
						else if( (dis & 3) == 1 )
						{
							sprintf(buffer, "LDD.D %s, %s, $%x", dest, source, dis & ~1);
						}
						// LDW.D
						else
						{
							sprintf(buffer, "LDW.D %s, %s, $%x", dest, source, dis & ~1);
						}

						break;
				}
			}

			break;
			}

		// LDxx.N/S
		case 0x94: case 0x95: case 0x96: case 0x97:
		{
			UINT16 next_op = READ_OP_DASM(pc + 2);
			UINT32 dis = RRdis_format(source, dest, op, next_op, pc);

			if( size == 2 )
				size = 4;

			if( (dest_code == PC_REGISTER && !dest_bit) || (dest_code == SR_REGISTER && !dest_bit) )
			{
				sprintf(buffer, "Reserved");
				break;
			}

			switch( DD( next_op ) )
			{
				case 0:
					// LDBS.N
					sprintf(buffer, "LDBS.N %s, %s, $%x", dest, source, dis);
					break;

				case 1:
					// LDBU.N
					sprintf(buffer, "LDBU.N %s, %s, $%x", dest, source, dis);
					break;

				case 2:
					// LDHS.N
					if( dis & 1 )
					{
						sprintf(buffer, "LDHS.N %s, %s, $%x", dest, source, dis & ~1);
					}
					// LDHU.N
					else
					{
						sprintf(buffer, "LDHU.N %s, %s, $%x", dest, source, dis & ~1);
					}

					break;

				case 3:
					// LDW.S
					if( (dis & 3) == 3 )
					{
						sprintf(buffer, "LDW.S %s, %s, $%x", dest, source, dis & ~3);
					}
					// Reserved
					else if( (dis & 3) == 2 )
					{
						sprintf(buffer, "Reserved");
					}
					// LDD.N
					else if( (dis & 3) == 1 )
					{
						sprintf(buffer, "LDD.N %s, %s, $%x", dest, source, dis & ~1);
					}
					// LDW.N
					else
					{
						sprintf(buffer, "LDW.N %s, %s, $%x", dest, source, dis & ~1);
					}

					break;
			}

			break;
		}

		// STxx.D/A/IOD/IOA
		case 0x98: case 0x99: case 0x9a: case 0x9b:
		{
			UINT16 next_op = READ_OP_DASM(pc + 2);
			UINT32 dis = RRdis_format(source, dest, op, next_op, pc);

			if( size == 2 )
				size = 4;

			if( source_code == SR_REGISTER && !source_bit )
				strcpy(source,"0");

			if( dest_code == SR_REGISTER && !dest_bit )
			{
				switch( DD( next_op ) )
				{
					case 0:
						// STBS.A
						sprintf(buffer, "STBS.A 0, %s, $%x", source, dis);
						break;

					case 1:
						// STBU.A
						sprintf(buffer, "STBU.A 0, %s, $%x", source, dis);
						break;

					case 2:
						// STHS.A
						if( dis & 1 )
						{
							sprintf(buffer, "STHS.A 0, %s, $%x", source, dis & ~1);
						}
						// STHU.A
						else
						{
							sprintf(buffer, "STHU.A 0, %s, $%x", source, dis & ~1);
						}

						break;

					case 3:
						// STD.IOA
						if( (dis & 3) == 3 )
						{
							sprintf(buffer, "STD.IOA 0, %s, $%x", source, dis & ~3);
						}
						// STW.IOA
						else if( (dis & 3) == 2 )
						{
							sprintf(buffer, "STW.IOA 0, %s, $%x", source, dis & ~3);
						}
						// STD.A
						else if( (dis & 3) == 1 )
						{
							sprintf(buffer, "STD.A 0, %s, $%x", source, dis & ~1);
						}
						// STW.A
						else
						{
							sprintf(buffer, "STW.A 0, %s, $%x", source, dis & ~1);
						}

						break;
				}
			}
			else
			{
				switch( DD( next_op ) )
				{
					case 0:
						// STBS.D
						sprintf(buffer, "STBS.D %s, %s, $%x", dest, source, dis);
						break;

					case 1:
						// STBU.D
						sprintf(buffer, "STBU.D %s, %s, $%x", dest, source, dis);
						break;

					case 2:
						// STHS.D
						if( dis & 1 )
						{
							sprintf(buffer, "STHS.D %s, %s, $%x", dest, source, dis & ~1);
						}
						// STHU.D
						else
						{
							sprintf(buffer, "STHU.D %s, %s, $%x", dest, source, dis & ~1);
						}
						break;

					case 3:
						// STD.IOD
						if( (dis & 3) == 3 )
						{
							sprintf(buffer, "STD.IOD %s, %s, $%x", dest, source, dis & ~3);
						}
						// STW.IOD
						else if( (dis & 3) == 2 )
						{
							sprintf(buffer, "STW.IOD %s, %s, $%x", dest, source, dis & ~3);
						}
						// STD.D
						else if( (dis & 3) == 1 )
						{
							sprintf(buffer, "STD.D %s, %s, $%x", dest, source, dis & ~1);
						}
						// STW.D
						else
						{
							sprintf(buffer, "STW.D %s, %s, $%x", dest, source, dis & ~1);
						}

						break;
				}
			}

			break;
			}

		// STxx.N/S
		case 0x9c: case 0x9d: case 0x9e: case 0x9f:
		{
			UINT16 next_op = READ_OP_DASM(pc + 2);
			UINT32 dis = RRdis_format(source, dest, op, next_op, pc);

			if( size == 2 )
				size = 4;

			if( source_code == SR_REGISTER && !source_bit )
				strcpy(source,"0");

			if( (dest_code == PC_REGISTER && !dest_bit) || (dest_code == SR_REGISTER && !dest_bit) )
			{
				sprintf(buffer, "Reserved");
				break;
			}

			switch( DD( next_op ) )
			{
				case 0:
					// STBS.N
					sprintf(buffer, "STBS.N %s, %s, $%x", dest, source, dis);
					break;

				case 1:
					// STBU.N
					sprintf(buffer, "STBU.N %s, %s, $%x", dest, source, dis);
					break;

				case 2:
					// STHS.N
					if( dis & 1 )
					{
						sprintf(buffer, "STHS.N %s, %s, $%x", dest, source, dis & ~1);
					}
					// STHU.N
					else
					{
						sprintf(buffer, "STHU.N %s, %s, $%x", dest, source, dis & ~1);
					}

					break;

				case 3:
					// STW.S
					if( (dis & 3) == 3 )
					{
						sprintf(buffer, "STW.S %s, %s, $%x", dest, source, dis & ~3);
					}
					// Reserved
					else if( (dis & 3) == 2 )
					{
						sprintf(buffer, "Reserved");
					}
					// STD.N
					else if( (dis & 3) == 1 )
					{
						sprintf(buffer, "STD.N %s, %s, $%x", dest, source, dis & ~1);
					}
					// STW.N
					else
					{
						sprintf(buffer, "STW.N %s, %s, $%x", dest, source, dis & ~1);
					}

					break;
			}

			break;
		}

		// SHRI
		case 0xa0: case 0xa1: case 0xa2: case 0xa3:
		{
			UINT8 n = Rn_format(dest, op);

			sprintf(buffer, "SHRI %s, $%x", dest, n);

			break;
		}

		// SARI
		case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		{
			UINT8 n = Rn_format(dest, op);

			sprintf(buffer, "SARI %s, $%x", dest, n);

			break;
		}

		// SHLI
		case 0xa8: case 0xa9: case 0xaa: case 0xab:
		{
			UINT8 n = Rn_format(dest, op);

			sprintf(buffer, "SHLI %s, $%x", dest, n);

			break;
		}

		// MULU
		case 0xb0: case 0xb1: case 0xb2: case 0xb3:

			RR_format(source, dest, op, 0);

			sprintf(buffer, "MULU %s, %s", dest, source);

			break;

		// MULS
		case 0xb4: case 0xb5: case 0xb6: case 0xb7:

			RR_format(source, dest, op, 0);

			sprintf(buffer, "MULS %s, %s", dest, source);

			break;

		// SETxx - SETADR - FETCH
		case 0xb8: case 0xb9: case 0xba: case 0xbb:
		{
			UINT8 n = Rn_format(dest, op);

			if( dest_code == PC_REGISTER && !dest_bit )
			{
				sprintf(buffer, "Illegal PC: $%x OP: $%x", pc, op);
			}
			else if( dest_code == SR_REGISTER && !dest_bit )
			{
				sprintf(buffer, "FETCH $%x", (n / 2) + 1);
			}
			else
			{
				sprintf(buffer, "%s %s", SETxx[n], dest);
			}

			break;
		}

		// MUL
		case 0xbc: case 0xbd: case 0xbe: case 0xbf:

			RR_format(source, dest, op, 0);

			sprintf(buffer, "MUL %s, %s", dest, source);

			break;

		// FADD
		case 0xc0:

			LL_format(source, dest, op);

			sprintf(buffer, "FADD %s, %s", dest, source);

			break;

		// FADDD
		case 0xc1:

			LL_format(source, dest, op);

			sprintf(buffer, "FADDD %s, %s", dest, source);

			break;

		// FSUB
		case 0xc2:

			LL_format(source, dest, op);

			sprintf(buffer, "FSUB %s, %s", dest, source);

			break;

		// FSUBD
		case 0xc3:

			LL_format(source, dest, op);

			sprintf(buffer, "FSUBD %s, %s", dest, source);

			break;

		// FMUL
		case 0xc4:

			LL_format(source, dest, op);

			sprintf(buffer, "FMUL %s, %s", dest, source);

			break;

		// FMULD
		case 0xc5:

			LL_format(source, dest, op);

			sprintf(buffer, "FMULD %s, %s", dest, source);

			break;

		// FDIV
		case 0xc6:

			LL_format(source, dest, op);

			sprintf(buffer, "FDIV %s, %s", dest, source);

			break;

		// FDIVD
		case 0xc7:

			LL_format(source, dest, op);

			sprintf(buffer, "FDIVD %s, %s", dest, source);

			break;

		// FCMP
		case 0xc8:

			LL_format(source, dest, op);

			sprintf(buffer, "FCMP %s, %s", dest, source);

			break;

		// FCMPD
		case 0xc9:

			LL_format(source, dest, op);

			sprintf(buffer, "FCMPD %s, %s", dest, source);

			break;

		// FCMPU
		case 0xca:

			LL_format(source, dest, op);

			sprintf(buffer, "FCMPU %s, %s", dest, source);

			break;

		// FCMPUD
		case 0xcb:

			LL_format(source, dest, op);

			sprintf(buffer, "FCMPUD %s, %s", dest, source);

			break;

		// FCVT
		case 0xcc:

			LL_format(source, dest, op);

			sprintf(buffer, "FCVT %s, %s", dest, source);

			break;

		// FCVTD
		case 0xcd:

			LL_format(source, dest, op);

			sprintf(buffer, "FCVTD %s, %s", dest, source);

			break;

		// EXTEND
		case 0xce:
		{
			UINT16 extended_op;

			LL_format(source, dest, op);

			pc += 2;
			extended_op = READ_OP_DASM(pc);

			size = 4;

			switch( extended_op )
			{
			case 0x100:
			case EMUL:
				sprintf(buffer, "EMUL %s, %s", dest, source);
				break;

			case EMULU:
				sprintf(buffer, "EMULU %s, %s", dest, source);
				break;

			case EMULS:
				sprintf(buffer, "EMULS %s, %s", dest, source);
				break;

			case EMAC:
				sprintf(buffer, "EMAC %s, %s", dest, source);
				break;

			case EMACD:
				sprintf(buffer, "EMACD %s, %s", dest, source);
				break;

			case EMSUB:
				sprintf(buffer, "EMSUB %s, %s", dest, source);
				break;

			case EMSUBD:
				sprintf(buffer, "EMSUBD %s, %s", dest, source);
				break;

			case EHMAC:
				sprintf(buffer, "EHMAC %s, %s", dest, source);
				break;

			case EHMACD:
				sprintf(buffer, "EHMACD %s, %s", dest, source);
				break;

			case EHCMULD:
				sprintf(buffer, "EHCMULD %s, %s", dest, source);
				break;

			case EHCMACD:
				sprintf(buffer, "EHCMACD %s, %s", dest, source);
				break;

			case EHCSUMD:
				sprintf(buffer, "EHCSUMD %s, %s", dest, source);
				break;

			case EHCFFTD:
				sprintf(buffer, "EHCFFTD %s, %s", dest, source);
				break;

			case EHCFFTSD:
				sprintf(buffer, "EHCFFTSD %s, %s", dest, source);
				break;

			default:
				sprintf(buffer, "Ext. OP $%x", extended_op);
				logerror(buffer, "Illegal Extended Opcode: %x\n", extended_op);
				break;
			}

			break;
		}

		// DO
		case 0xcf:

			LL_format(source, dest, op);

			sprintf(buffer, "DO %s, %s", dest, source);

			break;

		// LDW.R
		case 0xd0: case 0xd1:

			LR_format(source, dest, op);

			sprintf(buffer, "LDW.R %s, %s", dest, source);

			break;

		// LDD.R
		case 0xd2: case 0xd3:

			LR_format(source, dest, op);

			sprintf(buffer, "LDD.R %s, %s", dest, source);

			break;

		// LDW.P
		case 0xd4: case 0xd5:

			LR_format(source, dest, op);

			sprintf(buffer, "LDW.P %s, %s", dest, source);

			break;

		// LDD.P
		case 0xd6: case 0xd7:

			LR_format(source, dest, op);

			sprintf(buffer, "LDD.P %s, %s", dest, source);

			break;

		// STW.R
		case 0xd8: case 0xd9:

			LR_format(source, dest, op);

			if( source_code == SR_REGISTER && !source_bit )
				strcpy(source,"0");

			sprintf(buffer, "STW.R %s, %s", dest, source);

			break;

		// STD.R
		case 0xda: case 0xdb:

			LR_format(source, dest, op);

			if( source_code == SR_REGISTER && !source_bit )
				strcpy(source,"0");

			sprintf(buffer, "STD.R %s, %s", dest, source);

			break;

		// STW.P
		case 0xdc: case 0xdd:

			LR_format(source, dest, op);

			if( source_code == SR_REGISTER && !source_bit )
				strcpy(source,"0");

			sprintf(buffer, "STW.P %s, %s", dest, source);

			break;

		// STD.P
		case 0xde: case 0xdf:

			LR_format(source, dest, op);

			if( source_code == SR_REGISTER && !source_bit )
				strcpy(source,"0");

			sprintf(buffer, "STD.P %s, %s", dest, source);

			break;

		// DBV
		case 0xe0:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "DBV $%x", rel);
			flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);

			break;
		}

		// DBNV
		case 0xe1:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "DBNV $%x", rel);
			flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);

			break;
		}

		// DBE
		case 0xe2:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "DBE $%x", rel);
			flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);

			break;
		}

		// DBNE
		case 0xe3:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "DBNE $%x", rel);
			flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);

			break;
		}

		// DBC
		case 0xe4:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "DBC $%x", rel);
			flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);

			break;
		}

		// DBNC
		case 0xe5:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "DBNC $%x", rel);
			flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);

			break;
		}

		// DBSE
		case 0xe6:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "DBSE $%x", rel);
			flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);

			break;
		}

		// DBHT
		case 0xe7:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "DBHT $%x", rel);
			flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);

			break;
		}

		// DBN
		case 0xe8:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "DBN $%x", rel);
			flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);

			break;
		}

		// DBNN
		case 0xe9:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "DBNN $%x", rel);
			flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);

			break;
		}

		// DBLE
		case 0xea:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "DBLE $%x", rel);
			flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);

			break;
		}

		// DBGT
		case 0xeb:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "DBGT $%x", rel);
			flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);

			break;
		}

		// DBR
		case 0xec:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "DBR $%x", rel);
			flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);

			break;
		}

		// FRAME
		case 0xed:

			global_fp = 0;
			LL_format(source, dest, op);

			sprintf(buffer, "FRAME %s, %s", dest, source);

			break;

		// CALL
		case 0xee: case 0xef:
		{
			UINT32 const_val = LRconst_format(source, dest, op, &pc);

			if( source_code == SR_REGISTER && !source_bit )
			{
				sprintf(buffer, "CALL %s, 0, $%x", dest, const_val);
				flags = DASMFLAG_STEP_OVER;
			}
			else
			{
				sprintf(buffer, "CALL %s, %s, $%x", dest, source, const_val);
				flags = DASMFLAG_STEP_OVER;
			}

			break;
		}

		// BV
		case 0xf0:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "BV $%x", rel);

			break;
		}

		// BNV
		case 0xf1:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "BNV $%x", rel);

			break;
		}

		// BE
		case 0xf2:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "BE $%x", rel);

			break;
		}

		// BNE
		case 0xf3:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "BNE $%x", rel);

			break;
		}

		// BC
		case 0xf4:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "BC $%x", rel);

			break;
		}

		// BNC
		case 0xf5:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "BNC $%x", rel);

			break;
		}

		// BSE
		case 0xf6:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "BSE $%x", rel);

			break;
		}

		// BHT
		case 0xf7:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "BHT $%x", rel);

			break;
		}

		// BN
		case 0xf8:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "BN $%x", rel);

			break;
		}

		// BNN
		case 0xf9:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "BNN $%x", rel);

			break;
		}

		// BLE
		case 0xfa:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "BLE $%x", rel);

			break;
		}

		// BGT
		case 0xfb:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "BGT $%x", rel);

			break;
		}

		// BR
		case 0xfc:
		{
			INT32 rel = PCrel_format(op, pc) + 2;

			sprintf(buffer, "BR $%x", rel);

			break;
		}

		// TRAPxx - TRAP
		case 0xfd: case 0xfe: case 0xff:
		{
			UINT8 code = ((op & 0x300) >> 6) | (op & 0x03);
			UINT8 trapno = (op & 0xfc) >> 2;

			switch( code )
			{
				case TRAPLE:
					sprintf(buffer, "TRAPLE %d", trapno);
					flags = DASMFLAG_STEP_OVER;

					break;

				case TRAPGT:
					sprintf(buffer, "TRAPGT %d", trapno);
					flags = DASMFLAG_STEP_OVER;

					break;

				case TRAPLT:
					sprintf(buffer, "TRAPLT %d", trapno);
					flags = DASMFLAG_STEP_OVER;

					break;

				case TRAPGE:
					sprintf(buffer, "TRAPGE %d", trapno);
					flags = DASMFLAG_STEP_OVER;

					break;

				case TRAPSE:
					sprintf(buffer, "TRAPSE %d", trapno);
					flags = DASMFLAG_STEP_OVER;

					break;

				case TRAPHT:
					sprintf(buffer, "TRAPHT %d", trapno);
					flags = DASMFLAG_STEP_OVER;

					break;

				case TRAPST:
					sprintf(buffer, "TRAPST %d", trapno);
					flags = DASMFLAG_STEP_OVER;

					break;

				case TRAPHE:
					sprintf(buffer, "TRAPHE %d", trapno);
					flags = DASMFLAG_STEP_OVER;

					break;

				case TRAPE:
					sprintf(buffer, "TRAPE %d", trapno);
					flags = DASMFLAG_STEP_OVER;

					break;

				case TRAPNE:
					sprintf(buffer, "TRAPNE %d", trapno);
					flags = DASMFLAG_STEP_OVER;

					break;

				case TRAPV:
					sprintf(buffer, "TRAPV %d", trapno);
					flags = DASMFLAG_STEP_OVER;

					break;

				case TRAP:
					sprintf(buffer, "TRAP %d", trapno);
					flags = DASMFLAG_STEP_OVER;

					break;
			}

			break;
		}
	}

	return size | flags | DASMFLAG_SUPPORTED;
}
