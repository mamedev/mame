// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/*

 Hyperstone disassembler
 written by Pierpaolo Prazzoli

*/

#include "emu.h"
#include "32xsdasm.h"
#include "32xsdefs.h"


const char *const hyperstone_disassembler::L_REG[] =
{
	"L0",  "L1",  "L2",  "L3",  "L4",  "L5",  "L6",  "L7",  "L8",  "L9",
	"L10", "L11", "L12", "L13", "L14", "L15", "L16", "L17", "L18", "L19",
	"L20", "L21", "L22", "L23", "L24", "L25", "L26", "L27", "L28", "L29",
	"L30", "L31", "L32", "L33", "L34", "L35", "L36", "L37", "L38", "L39",
	"L40", "L41", "L42", "L43", "L44", "L45", "L46", "L47", "L48", "L49",
	"L50", "L51", "L52", "L53", "L54", "L55", "L56", "L57", "L58", "L59",
	"L60", "L61", "L62", "L63"
};

const char *const hyperstone_disassembler::G_REG[] =
{
	"PC",  "SR",  "FER", "G03", "G04", "G05", "G06", "G07", "G08", "G09",
	"G10", "G11", "G12", "G13", "G14", "G15", "G16", "G17", "SP",  "UB",
	"BCR", "TPR", "TCR", "TR",  "WCR", "ISR", "FCR", "MCR", "G28", "G29",
	"G30", "G31"
};

const char *const hyperstone_disassembler::SETxx[] =
{
	"SETADR",   "Reserved", "SET1",   "SET0",     "SETLE",  "SETGT",  "SETLT",  "SETGE",
	"SETSE",    "SETHT",    "SETST",  "SETHE",    "SETE",   "SETNE",  "SETV",   "SETNV",
	"Reserved", "Reserved", "SET1M",  "Reserved", "SETLEM", "SETGTM", "SETLTM", "SETGEM",
	"SETSEM",   "SETTHM",   "SETSTM", "SETHEM",   "SETEM",  "SETNEM", "SETVM",  "SETNVM"
};

#define DESTCODE(op)            ((op & 0x00f0) >> 4)
#define SOURCECODE(op)          (op & 0x000f)

#define SOURCEBIT(op)           ((op & 0x100) >> 8)
#define DESTBIT(op)             ((op & 0x200) >> 9)

#define DASM_N_VALUE(op)             ((((op & 0x100) >> 8) << 4 ) | (op & 0x0f))

void hyperstone_disassembler::LL_format(char *source, char *dest, uint16_t op)
{
	strcpy(source, L_REG[(SOURCECODE(op)+global_fp)%64]);
	strcpy(dest, L_REG[(DESTCODE(op)+global_fp)%64]);
}

void hyperstone_disassembler::LR_format(char *source, char *dest, uint16_t op)
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

void hyperstone_disassembler::RR_format(char *source, char *dest, uint16_t op, unsigned h_flag)
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

uint32_t hyperstone_disassembler::LRconst_format(char *source, char *dest, uint16_t op, offs_t &pc, const data_buffer &opcodes)
{
	uint16_t next_op;
	uint32_t const_val;

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

	pc += 2;
	next_op = opcodes.r16(pc);

	if( E_BIT(next_op) )
	{
		uint16_t next_op2;

		size = 6;

		pc += 2;
		next_op2 = opcodes.r16(pc);
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

uint32_t hyperstone_disassembler::RRconst_format(char *source, char *dest, uint16_t op, offs_t &pc, const data_buffer &opcodes)
{
	uint16_t next_op;
	uint32_t const_val;

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

	pc += 2;
	next_op = opcodes.r16(pc);

	if( E_BIT(next_op) )
	{
		uint16_t next_op2;

		size = 6;

		pc += 2;
		next_op2 = opcodes.r16(pc);
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

int32_t hyperstone_disassembler::Rimm_format(char *dest, uint16_t op, offs_t &pc, const data_buffer &opcodes, unsigned h_flag)
{
	uint16_t imm1, imm2;
	int32_t ret;

	int n = DASM_N_VALUE(op);

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
		case 0: case 1:  case 2:  case 3:  case 4:  case 5:  case 6:  case 7: case 8:
		case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16:
			return n;

		case 17:
			pc += 2;
			imm1 = opcodes.r16(pc);
			pc += 2;
			imm2 = opcodes.r16(pc);
			ret = (imm1 << 16) | imm2;

			size = 6;
			return ret;


		case 18:
			pc += 2;
			ret = opcodes.r16(pc);

			size = 4;
			return ret;

		case 19:
			pc += 2;
			ret = (int32_t) (0xffff0000 | opcodes.r16(pc));

			size = 4;
			return ret;

		case 20:
			return 32;  //bit 5 = 1, others = 0

		case 21:
			return 64;  //bit 6 = 1, others = 0

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

uint8_t hyperstone_disassembler::Ln_format(char *dest, uint16_t op)
{
	strcpy(dest, L_REG[(DESTCODE(op)+global_fp)%64]);

	return DASM_N_VALUE(op);
}

uint8_t hyperstone_disassembler::Rn_format(char *dest, uint16_t op)
{
	if( DESTBIT(op) )
	{
		strcpy(dest, L_REG[(DESTCODE(op)+global_fp)%64]);
	}
	else
	{
		strcpy(dest, G_REG[DESTCODE(op)]);
	}

	return DASM_N_VALUE(op);
}

int32_t hyperstone_disassembler::PCrel_format(uint16_t op, offs_t pc, const data_buffer &opcodes)
{
	int32_t ret;

	if( op & 0x80 ) //bit 7 = 1
	{
		uint16_t next;

		size = 4;

		pc += 2;

		next = opcodes.r16(pc);

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

uint32_t hyperstone_disassembler::RRdis_format(char *source, char *dest, uint16_t op, uint16_t next_op, offs_t pc, const data_buffer &opcodes)
{
	uint32_t ret;

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
		uint16_t next;

		size = 6;

		next = opcodes.r16(pc + 4);

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

u32 hyperstone_disassembler::opcode_alignment() const
{
	return 2;
}

hyperstone_disassembler::hyperstone_disassembler(config *conf) : m_config(conf)
{
}

/*****************************/
/* Main disassembly function */
/*****************************/
offs_t hyperstone_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	char source[5] = "\0", dest[5] = "\0";
	uint32_t flags = 0;

	uint16_t op = opcodes.r16(pc);

	size = 2;

	uint8_t source_code = SOURCECODE(op);
	uint8_t dest_code = DESTCODE(op);
	uint8_t source_bit = SOURCEBIT(op);
	uint8_t dest_bit = DESTBIT(op);

	global_fp = m_config->get_fp();
	int h_flag = m_config->get_h();

	uint8_t op_num = (op & 0xff00) >> 8;

	switch( op_num )
	{
		// CHK - CHKZ - NOP
		case 0x00: case 0x01: case 0x02: case 0x03:

			if( source_bit && dest_bit && source_code == 0 && dest_code == 0 )
			{
				util::stream_format(stream, "NOP");
			}
			else
			{
				RR_format(source, dest, op, 0);

				if( !source_bit && source_code == SR_REGISTER )
				{
					util::stream_format(stream, "CHKZ %s, 0", dest);
				}
				else
				{
					util::stream_format(stream, "CHK %s, %s", dest, source);
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
				util::stream_format(stream, "RET PC, %s", source);
				flags = STEP_OUT;
			}
			else if( source_code == SR_REGISTER && !source_bit )
			{
				util::stream_format(stream, "MOVD %s, 0", dest);
			}
			else
			{
				util::stream_format(stream, "MOVD %s, %s", dest, source);
			}

			break;

		// DIVU
		case 0x08: case 0x09: case 0x0a: case 0x0b:

			RR_format(source, dest, op, 0);
			util::stream_format(stream, "DIVU %s, %s", dest, source);

			break;

		// DIVS
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:

			RR_format(source, dest, op, 0);
			util::stream_format(stream, "DIVS %s, %s", dest, source);

			break;

		// XMx - XXx
		case 0x10: case 0x11: case 0x12: case 0x13:
		{
			int xcode;

			RR_format(source, dest, op, 0);

			size = 4;

			pc += 2;
			op = opcodes.r16(pc);

			xcode = X_CODE(op);

			if( xcode < 4 )
			{
				uint16_t lim;

				if( E_BIT(op) )
				{
					uint16_t next_op;

					size = 6;

					pc += 2;
					next_op = opcodes.r16(pc);

					lim = ((op & 0xfff) << 16) | next_op;
				}
				else
				{
					lim = op & 0xfff;
				}

				util::stream_format(stream, "XM%x %s, %s, $%x", (uint8_t)(float) pow(2.0, xcode), dest, source, lim);

			}
			else
			{
				util::stream_format(stream, "XX%x %s, %s, 0", (uint8_t)(float) pow(2.0, (xcode - 4)), dest, source);
			}

			break;
		}

		// MASK
		case 0x14: case 0x15: case 0x16: case 0x17:
		{
			uint32_t const_val = RRconst_format(source, dest, op, pc, opcodes);

			util::stream_format(stream, "MASK %s, %s, $%x", dest, source, const_val);

			break;
		}

		// SUM
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		{
			uint32_t const_val = RRconst_format(source, dest, op, pc, opcodes);

			if( source_code == SR_REGISTER && !source_bit )
			{
				util::stream_format(stream, "SUM %s, C, $%x", dest, const_val);
			}
			else
			{
				util::stream_format(stream, "SUM %s, %s, $%x", dest, source, const_val);
			}

			break;
		}

		// SUMS
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		{
			uint32_t const_val = RRconst_format(source, dest, op, pc, opcodes);

			if( source_code == SR_REGISTER && !source_bit )
			{
				util::stream_format(stream, "SUMS %s, C, $%x", dest, const_val);
			}
			else
			{
				util::stream_format(stream, "SUMS %s, %s, $%x", dest, source, const_val);
			}

			break;
		}

		// CMP
		case 0x20: case 0x21: case 0x22: case 0x23:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				util::stream_format(stream, "CMP %s, C", dest);
			}
			else
			{
				util::stream_format(stream, "CMP %s, %s", dest, source);
			}

			break;

		// MOV
		case 0x24: case 0x25: case 0x26: case 0x27:

			RR_format(source, dest, op, h_flag);
			util::stream_format(stream, "MOV %s, %s", dest, source);

			break;

		// ADD
		case 0x28: case 0x29: case 0x2a: case 0x2b:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				util::stream_format(stream, "ADD %s, C", dest);
			}
			else
			{
				util::stream_format(stream, "ADD %s, %s", dest, source);
			}

			break;

		// ADDS
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				util::stream_format(stream, "ADDS %s, C", dest);
			}
			else
			{
				util::stream_format(stream, "ADDS %s, %s", dest, source);
			}

			break;

		// CMPB
		case 0x30: case 0x31: case 0x32: case 0x33:

			RR_format(source, dest, op, 0);
			util::stream_format(stream, "CMPB %s, %s", dest, source);

			break;

		// ANDN
		case 0x34: case 0x35: case 0x36: case 0x37:

			RR_format(source, dest, op, 0);
			util::stream_format(stream, "ANDN %s, %s", dest, source);

			break;

		// OR
		case 0x38: case 0x39: case 0x3a: case 0x3b:

			RR_format(source, dest, op, 0);
			util::stream_format(stream, "OR %s, %s", dest, source);

			break;

		// XOR
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:

			RR_format(source, dest, op, 0);
			util::stream_format(stream, "XOR %s, %s", dest, source);

			break;

		// SUBC
		case 0x40: case 0x41: case 0x42: case 0x43:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				util::stream_format(stream, "SUBC %s, C", dest);
			}
			else
			{
				util::stream_format(stream, "SUBC %s, %s", dest, source);
			}

			break;

		// NOT
		case 0x44: case 0x45: case 0x46: case 0x47:

			RR_format(source, dest, op, 0);

			util::stream_format(stream, "NOT %s, %s", dest, source);

			break;

		// SUB
		case 0x48: case 0x49: case 0x4a: case 0x4b:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				util::stream_format(stream, "SUB %s, C", dest);
			}
			else
			{
				util::stream_format(stream, "SUB %s, %s", dest, source);
			}

			break;

		// SUBS
		case 0x4c: case 0x4d: case 0x4e: case 0x4f:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				util::stream_format(stream, "SUBS %s, C", dest);
			}
			else
			{
				util::stream_format(stream, "SUBS %s, %s", dest, source);
			}

			break;

		// ADDC
		case 0x50: case 0x51: case 0x52: case 0x53:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				util::stream_format(stream, "ADDC %s, C", dest);
			}
			else
			{
				util::stream_format(stream, "ADDC %s, %s", dest, source);
			}

			break;

		// AND
		case 0x54: case 0x55: case 0x56: case 0x57:

			RR_format(source, dest, op, 0);
			util::stream_format(stream, "AND %s, %s", dest, source);

			break;

		// NEG
		case 0x58: case 0x59: case 0x5a: case 0x5b:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				util::stream_format(stream, "NEG %s, C", dest);
			}
			else
			{
				util::stream_format(stream, "NEG %s, %s", dest, source);
			}

			break;

		// NEGS
		case 0x5c: case 0x5d: case 0x5e: case 0x5f:

			RR_format(source, dest, op, 0);

			if( source_code == SR_REGISTER && !source_bit )
			{
				util::stream_format(stream, "NEGS %s, C", dest);
			}
			else
			{
				util::stream_format(stream, "NEGS %s, %s", dest, source);
			}

			break;

		// CMPI
		case 0x60: case 0x61: case 0x62: case 0x63:
		{
			uint32_t imm = Rimm_format(dest, op, pc, opcodes, 0);

			util::stream_format(stream, "CMPI %s, $%x", dest, imm);

			break;
		}

		// MOVI
		case 0x64: case 0x65: case 0x66: case 0x67:
		{
			uint32_t imm = Rimm_format(dest, op, pc, opcodes, h_flag);

			util::stream_format(stream, "MOVI %s, $%x", dest, imm);

			break;
		}

		// ADDI
		case 0x68: case 0x69: case 0x6a: case 0x6b:
		{
			uint32_t imm = Rimm_format(dest, op, pc, opcodes, 0);

			if( !DASM_N_VALUE(op) )
			{
				util::stream_format(stream, "ADDI %s, CZ", dest);
			}
			else
			{
				util::stream_format(stream, "ADDI %s, $%x", dest, imm);
			}

			break;
		}

		// ADDSI
		case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		{
			uint32_t imm = Rimm_format(dest, op, pc, opcodes, 0);

			if( !DASM_N_VALUE(op) )
			{
				util::stream_format(stream, "ADDSI %s, CZ", dest);
			}
			else
			{
				util::stream_format(stream, "ADDSI %s, $%x", dest, imm);
			}

			break;
		}

		// CMPBI
		case 0x70: case 0x71: case 0x72: case 0x73:
		{
			uint32_t imm = Rimm_format(dest, op, pc, opcodes, 0);

			if( !DASM_N_VALUE(op) )
			{
				util::stream_format(stream, "CMPBI %s, ANYBZ", dest);
			}
			else
			{
				if( DASM_N_VALUE(op) == 31 )
					imm = 0x7fffffff; //bit 31 = 0, others = 1

				util::stream_format(stream, "CMPBI %s, $%x", dest, imm);
			}

			break;
		}

		// ANDNI
		case 0x74: case 0x75: case 0x76: case 0x77:
		{
			uint32_t imm = Rimm_format(dest, op, pc, opcodes, 0);

			if( DASM_N_VALUE(op) == 31 )
				imm = 0x7fffffff; //bit 31 = 0, others = 1

			util::stream_format(stream, "ANDNI %s, $%x", dest, imm);

			break;
		}

		// ORI
		case 0x78: case 0x79: case 0x7a: case 0x7b:
		{
			uint32_t imm = Rimm_format(dest, op, pc, opcodes, 0);

			util::stream_format(stream, "ORI %s, $%x", dest, imm);

			break;
		}

		// XORI
		case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		{
			uint32_t imm = Rimm_format(dest, op, pc, opcodes, 0);

			util::stream_format(stream, "XORI %s, $%x", dest, imm);

			break;
		}

		// SHRDI
		case 0x80: case 0x81:
		{
			uint8_t n = Ln_format(dest, op);

			util::stream_format(stream, "SHRDI %s, $%x", dest, n);

			break;
		}

		// SHRD
		case 0x82:

			LL_format(source, dest, op);

			util::stream_format(stream, "SHRD %s, %s", dest, source);

			break;

		// SHR
		case 0x83:

			LL_format(source, dest, op);

			util::stream_format(stream, "SHR %s, %s", dest, source);

			break;

		// SARDI
		case 0x84: case 0x85:
		{
			uint8_t n = Ln_format(dest, op);

			util::stream_format(stream, "SARDI %s, $%x", dest, n);

			break;
		}

		// SARD
		case 0x86:

			LL_format(source, dest, op);

			util::stream_format(stream, "SARD %s, %s", dest, source);

			break;

		// SAR
		case 0x87:

			LL_format(source, dest, op);

			util::stream_format(stream, "SAR %s, %s", dest, source);

			break;

		// SHLDI
		case 0x88: case 0x89:
		{
			uint8_t n = Ln_format(dest, op);

			util::stream_format(stream, "SHLDI %s, $%x", dest, n);

			break;
		}

		// SHLD
		case 0x8a:

			LL_format(source, dest, op);

			util::stream_format(stream, "SHLD %s, %s", dest, source);

			break;

		// SHL
		case 0x8b:

			LL_format(source, dest, op);

			util::stream_format(stream, "SHL %s, %s", dest, source);

			break;

		// RESERVED
		case 0x8c: case 0x8d:
		case 0xac: case 0xad: case 0xae: case 0xaf:

			util::stream_format(stream, "Reserved");

			break;

		// TESTLZ
		case 0x8e:

			LL_format(source, dest, op);

			util::stream_format(stream, "TESTLZ %s, %s", dest, source);

			break;

		// ROL
		case 0x8f:

			LL_format(source, dest, op);

			util::stream_format(stream, "ROL %s, %s", dest, source);

			break;

		// LDxx.D/A/IOD/IOA
		case 0x90: case 0x91: case 0x92: case 0x93:
		{
			uint16_t next_op = opcodes.r16(pc + 2);
			uint32_t dis = RRdis_format(source, dest, op, next_op, pc, opcodes);

			if( size == 2 )
				size = 4;

			if( dest_code == SR_REGISTER && !dest_bit )
			{
				switch( DD( next_op ) )
				{
					case 0:
						// LDBS.A
						util::stream_format(stream, "LDBS.A 0, %s, $%x", source, dis);
						break;

					case 1:
						// LDBU.A
						util::stream_format(stream, "LDBU.A 0, %s, $%x", source, dis);
						break;

					case 2:
						// LDHS.A
						if( dis & 1 )
						{
							util::stream_format(stream, "LDHS.A 0, %s, $%x", source, dis & ~1);
						}
						// LDHU.A
						else
						{
							util::stream_format(stream, "LDHU.A 0, %s, $%x", source, dis & ~1);
						}

						break;

					case 3:
						// LDD.IOA
						if( (dis & 3) == 3 )
						{
							util::stream_format(stream, "LDD.IOA 0, %s, $%x", source, dis & ~3);
						}
						// LDW.IOA
						else if( (dis & 3) == 2 )
						{
							util::stream_format(stream, "LDW.IOA 0, %s, $%x", source, dis & ~3);
						}
						// LDD.A
						else if( (dis & 3) == 1 )
						{
							util::stream_format(stream, "LDD.A 0, %s, $%x", source, dis & ~1);
						}
						// LDW.A
						else
						{
							util::stream_format(stream, "LDW.A 0, %s, $%x", source, dis & ~1);
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
						util::stream_format(stream, "LDBS.D %s, %s, $%x", dest, source, dis);
						break;

					case 1:
						// LDBU.D
						util::stream_format(stream, "LDBU.D %s, %s, $%x", dest, source, dis);
						break;

					case 2:
						// LDHS.D
						if( dis & 1 )
						{
							util::stream_format(stream, "LDHS.D %s, %s, $%x", dest, source, dis & ~1);
						}
						// LDHU.D
						else
						{
							util::stream_format(stream, "LDHU.D %s, %s, $%x", dest, source, dis & ~1);
						}
						break;

					case 3:
						// LDD.IOD
						if( (dis & 3) == 3 )
						{
							util::stream_format(stream, "LDD.IOD %s, %s, $%x", dest, source, dis & ~3);
						}
						// LDW.IOD
						else if( (dis & 3) == 2 )
						{
							util::stream_format(stream, "LDW.IOD %s, %s, $%x", dest, source, dis & ~3);
						}
						// LDD.D
						else if( (dis & 3) == 1 )
						{
							util::stream_format(stream, "LDD.D %s, %s, $%x", dest, source, dis & ~1);
						}
						// LDW.D
						else
						{
							util::stream_format(stream, "LDW.D %s, %s, $%x", dest, source, dis & ~1);
						}

						break;
				}
			}

			break;
		}

		// LDxx.N/S
		case 0x94: case 0x95: case 0x96: case 0x97:
		{
			uint16_t next_op = opcodes.r16(pc + 2);
			uint32_t dis = RRdis_format(source, dest, op, next_op, pc, opcodes);

			if( size == 2 )
				size = 4;

			if( (dest_code == PC_REGISTER && !dest_bit) || (dest_code == SR_REGISTER && !dest_bit) )
			{
				util::stream_format(stream, "Reserved");
				break;
			}

			switch( DD( next_op ) )
			{
				case 0:
					// LDBS.N
					util::stream_format(stream, "LDBS.N %s, %s, $%x", dest, source, dis);
					break;

				case 1:
					// LDBU.N
					util::stream_format(stream, "LDBU.N %s, %s, $%x", dest, source, dis);
					break;

				case 2:
					// LDHS.N
					if( dis & 1 )
					{
						util::stream_format(stream, "LDHS.N %s, %s, $%x", dest, source, dis & ~1);
					}
					// LDHU.N
					else
					{
						util::stream_format(stream, "LDHU.N %s, %s, $%x", dest, source, dis & ~1);
					}

					break;

				case 3:
					// LDW.S
					if( (dis & 3) == 3 )
					{
						util::stream_format(stream, "LDW.S %s, %s, $%x", dest, source, dis & ~3);
					}
					// Reserved
					else if( (dis & 3) == 2 )
					{
						util::stream_format(stream, "Reserved");
					}
					// LDD.N
					else if( (dis & 3) == 1 )
					{
						util::stream_format(stream, "LDD.N %s, %s, $%x", dest, source, dis & ~1);
					}
					// LDW.N
					else
					{
						util::stream_format(stream, "LDW.N %s, %s, $%x", dest, source, dis & ~1);
					}

					break;
			}

			break;
		}

		// STxx.D/A/IOD/IOA
		case 0x98: case 0x99: case 0x9a: case 0x9b:
		{
			uint16_t next_op = opcodes.r16(pc + 2);
			uint32_t dis = RRdis_format(source, dest, op, next_op, pc, opcodes);

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
						util::stream_format(stream, "STBS.A 0, %s, $%x", source, dis);
						break;

					case 1:
						// STBU.A
						util::stream_format(stream, "STBU.A 0, %s, $%x", source, dis);
						break;

					case 2:
						// STHS.A
						if( dis & 1 )
						{
							util::stream_format(stream, "STHS.A 0, %s, $%x", source, dis & ~1);
						}
						// STHU.A
						else
						{
							util::stream_format(stream, "STHU.A 0, %s, $%x", source, dis & ~1);
						}

						break;

					case 3:
						// STD.IOA
						if( (dis & 3) == 3 )
						{
							util::stream_format(stream, "STD.IOA 0, %s, $%x", source, dis & ~3);
						}
						// STW.IOA
						else if( (dis & 3) == 2 )
						{
							util::stream_format(stream, "STW.IOA 0, %s, $%x", source, dis & ~3);
						}
						// STD.A
						else if( (dis & 3) == 1 )
						{
							util::stream_format(stream, "STD.A 0, %s, $%x", source, dis & ~1);
						}
						// STW.A
						else
						{
							util::stream_format(stream, "STW.A 0, %s, $%x", source, dis & ~1);
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
						util::stream_format(stream, "STBS.D %s, %s, $%x", dest, source, dis);
						break;

					case 1:
						// STBU.D
						util::stream_format(stream, "STBU.D %s, %s, $%x", dest, source, dis);
						break;

					case 2:
						// STHS.D
						if( dis & 1 )
						{
							util::stream_format(stream, "STHS.D %s, %s, $%x", dest, source, dis & ~1);
						}
						// STHU.D
						else
						{
							util::stream_format(stream, "STHU.D %s, %s, $%x", dest, source, dis & ~1);
						}
						break;

					case 3:
						// STD.IOD
						if( (dis & 3) == 3 )
						{
							util::stream_format(stream, "STD.IOD %s, %s, $%x", dest, source, dis & ~3);
						}
						// STW.IOD
						else if( (dis & 3) == 2 )
						{
							util::stream_format(stream, "STW.IOD %s, %s, $%x", dest, source, dis & ~3);
						}
						// STD.D
						else if( (dis & 3) == 1 )
						{
							util::stream_format(stream, "STD.D %s, %s, $%x", dest, source, dis & ~1);
						}
						// STW.D
						else
						{
							util::stream_format(stream, "STW.D %s, %s, $%x", dest, source, dis & ~1);
						}

						break;
				}
			}

			break;
		}

		// STxx.N/S
		case 0x9c: case 0x9d: case 0x9e: case 0x9f:
		{
			uint16_t next_op = opcodes.r16(pc + 2);
			uint32_t dis = RRdis_format(source, dest, op, next_op, pc, opcodes);

			if( size == 2 )
				size = 4;

			if( source_code == SR_REGISTER && !source_bit )
				strcpy(source,"0");

			if( (dest_code == PC_REGISTER && !dest_bit) || (dest_code == SR_REGISTER && !dest_bit) )
			{
				util::stream_format(stream, "Reserved");
				break;
			}

			switch( DD( next_op ) )
			{
				case 0:
					// STBS.N
					util::stream_format(stream, "STBS.N %s, %s, $%x", dest, source, dis);
					break;

				case 1:
					// STBU.N
					util::stream_format(stream, "STBU.N %s, %s, $%x", dest, source, dis);
					break;

				case 2:
					// STHS.N
					if( dis & 1 )
					{
						util::stream_format(stream, "STHS.N %s, %s, $%x", dest, source, dis & ~1);
					}
					// STHU.N
					else
					{
						util::stream_format(stream, "STHU.N %s, %s, $%x", dest, source, dis & ~1);
					}

					break;

				case 3:
					// STW.S
					if( (dis & 3) == 3 )
					{
						util::stream_format(stream, "STW.S %s, %s, $%x", dest, source, dis & ~3);
					}
					// Reserved
					else if( (dis & 3) == 2 )
					{
						util::stream_format(stream, "Reserved");
					}
					// STD.N
					else if( (dis & 3) == 1 )
					{
						util::stream_format(stream, "STD.N %s, %s, $%x", dest, source, dis & ~1);
					}
					// STW.N
					else
					{
						util::stream_format(stream, "STW.N %s, %s, $%x", dest, source, dis & ~1);
					}

					break;
			}

			break;
		}

		// SHRI
		case 0xa0: case 0xa1: case 0xa2: case 0xa3:
		{
			uint8_t n = Rn_format(dest, op);

			util::stream_format(stream, "SHRI %s, $%x", dest, n);

			break;
		}

		// SARI
		case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		{
			uint8_t n = Rn_format(dest, op);

			util::stream_format(stream, "SARI %s, $%x", dest, n);

			break;
		}

		// SHLI
		case 0xa8: case 0xa9: case 0xaa: case 0xab:
		{
			uint8_t n = Rn_format(dest, op);

			util::stream_format(stream, "SHLI %s, $%x", dest, n);

			break;
		}

		// MULU
		case 0xb0: case 0xb1: case 0xb2: case 0xb3:

			RR_format(source, dest, op, 0);

			util::stream_format(stream, "MULU %s, %s", dest, source);

			break;

		// MULS
		case 0xb4: case 0xb5: case 0xb6: case 0xb7:

			RR_format(source, dest, op, 0);

			util::stream_format(stream, "MULS %s, %s", dest, source);

			break;

		// SETxx - SETADR - FETCH
		case 0xb8: case 0xb9: case 0xba: case 0xbb:
		{
			uint8_t n = Rn_format(dest, op);

			if( dest_code == PC_REGISTER && !dest_bit )
			{
				util::stream_format(stream, "Illegal PC: $%x OP: $%x", pc, op);
			}
			else if( dest_code == SR_REGISTER && !dest_bit )
			{
				util::stream_format(stream, "FETCH $%x", (n / 2) + 1);
			}
			else
			{
				util::stream_format(stream, "%s %s", SETxx[n], dest);
			}

			break;
		}

		// MUL
		case 0xbc: case 0xbd: case 0xbe: case 0xbf:

			RR_format(source, dest, op, 0);

			util::stream_format(stream, "MUL %s, %s", dest, source);

			break;

		// FADD
		case 0xc0:

			LL_format(source, dest, op);

			util::stream_format(stream, "FADD %s, %s", dest, source);

			break;

		// FADDD
		case 0xc1:

			LL_format(source, dest, op);

			util::stream_format(stream, "FADDD %s, %s", dest, source);

			break;

		// FSUB
		case 0xc2:

			LL_format(source, dest, op);

			util::stream_format(stream, "FSUB %s, %s", dest, source);

			break;

		// FSUBD
		case 0xc3:

			LL_format(source, dest, op);

			util::stream_format(stream, "FSUBD %s, %s", dest, source);

			break;

		// FMUL
		case 0xc4:

			LL_format(source, dest, op);

			util::stream_format(stream, "FMUL %s, %s", dest, source);

			break;

		// FMULD
		case 0xc5:

			LL_format(source, dest, op);

			util::stream_format(stream, "FMULD %s, %s", dest, source);

			break;

		// FDIV
		case 0xc6:

			LL_format(source, dest, op);

			util::stream_format(stream, "FDIV %s, %s", dest, source);

			break;

		// FDIVD
		case 0xc7:

			LL_format(source, dest, op);

			util::stream_format(stream, "FDIVD %s, %s", dest, source);

			break;

		// FCMP
		case 0xc8:

			LL_format(source, dest, op);

			util::stream_format(stream, "FCMP %s, %s", dest, source);

			break;

		// FCMPD
		case 0xc9:

			LL_format(source, dest, op);

			util::stream_format(stream, "FCMPD %s, %s", dest, source);

			break;

		// FCMPU
		case 0xca:

			LL_format(source, dest, op);

			util::stream_format(stream, "FCMPU %s, %s", dest, source);

			break;

		// FCMPUD
		case 0xcb:

			LL_format(source, dest, op);

			util::stream_format(stream, "FCMPUD %s, %s", dest, source);

			break;

		// FCVT
		case 0xcc:

			LL_format(source, dest, op);

			util::stream_format(stream, "FCVT %s, %s", dest, source);

			break;

		// FCVTD
		case 0xcd:

			LL_format(source, dest, op);

			util::stream_format(stream, "FCVTD %s, %s", dest, source);

			break;

		// EXTEND
		case 0xce:
		{
			uint16_t extended_op;

			LL_format(source, dest, op);

			pc += 2;
			extended_op = opcodes.r16(pc);

			size = 4;

			switch( extended_op )
			{
			case 0x100:
			case EMUL:
				util::stream_format(stream, "EMUL %s, %s", dest, source);
				break;

			case EMULU:
				util::stream_format(stream, "EMULU %s, %s", dest, source);
				break;

			case EMULS:
				util::stream_format(stream, "EMULS %s, %s", dest, source);
				break;

			case EMAC:
				util::stream_format(stream, "EMAC %s, %s", dest, source);
				break;

			case EMACD:
				util::stream_format(stream, "EMACD %s, %s", dest, source);
				break;

			case EMSUB:
				util::stream_format(stream, "EMSUB %s, %s", dest, source);
				break;

			case EMSUBD:
				util::stream_format(stream, "EMSUBD %s, %s", dest, source);
				break;

			case EHMAC:
				util::stream_format(stream, "EHMAC %s, %s", dest, source);
				break;

			case EHMACD:
				util::stream_format(stream, "EHMACD %s, %s", dest, source);
				break;

			case EHCMULD:
				util::stream_format(stream, "EHCMULD %s, %s", dest, source);
				break;

			case EHCMACD:
				util::stream_format(stream, "EHCMACD %s, %s", dest, source);
				break;

			case EHCSUMD:
				util::stream_format(stream, "EHCSUMD %s, %s", dest, source);
				break;

			case EHCFFTD:
				util::stream_format(stream, "EHCFFTD %s, %s", dest, source);
				break;

			case EHCFFTSD:
				util::stream_format(stream, "EHCFFTSD %s, %s", dest, source);
				break;

			default:
				util::stream_format(stream, "Ext. OP $%X @ %X\n", extended_op, pc);
				osd_printf_verbose("Illegal Extended Opcode: %X @ %X\n", extended_op, pc);
				break;
			}

			break;
		}

		// DO
		case 0xcf:

			LL_format(source, dest, op);

			util::stream_format(stream, "DO %s, %s", dest, source);

			break;

		// LDW.R
		case 0xd0: case 0xd1:

			LR_format(source, dest, op);

			util::stream_format(stream, "LDW.R %s, %s", dest, source);

			break;

		// LDD.R
		case 0xd2: case 0xd3:

			LR_format(source, dest, op);

			util::stream_format(stream, "LDD.R %s, %s", dest, source);

			break;

		// LDW.P
		case 0xd4: case 0xd5:

			LR_format(source, dest, op);

			util::stream_format(stream, "LDW.P %s, %s", dest, source);

			break;

		// LDD.P
		case 0xd6: case 0xd7:

			LR_format(source, dest, op);

			util::stream_format(stream, "LDD.P %s, %s", dest, source);

			break;

		// STW.R
		case 0xd8: case 0xd9:

			LR_format(source, dest, op);

			if( source_code == SR_REGISTER && !source_bit )
				strcpy(source,"0");

			util::stream_format(stream, "STW.R %s, %s", dest, source);

			break;

		// STD.R
		case 0xda: case 0xdb:

			LR_format(source, dest, op);

			if( source_code == SR_REGISTER && !source_bit )
				strcpy(source,"0");

			util::stream_format(stream, "STD.R %s, %s", dest, source);

			break;

		// STW.P
		case 0xdc: case 0xdd:

			LR_format(source, dest, op);

			if( source_code == SR_REGISTER && !source_bit )
				strcpy(source,"0");

			util::stream_format(stream, "STW.P %s, %s", dest, source);

			break;

		// STD.P
		case 0xde: case 0xdf:

			LR_format(source, dest, op);

			if( source_code == SR_REGISTER && !source_bit )
				strcpy(source,"0");

			util::stream_format(stream, "STD.P %s, %s", dest, source);

			break;

		// DBV
		case 0xe0:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "DBV $%x", rel);
			flags = STEP_OVER | step_over_extra(1);

			break;
		}

		// DBNV
		case 0xe1:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "DBNV $%x", rel);
			flags = STEP_OVER | step_over_extra(1);

			break;
		}

		// DBE
		case 0xe2:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "DBE $%x", rel);
			flags = STEP_OVER | step_over_extra(1);

			break;
		}

		// DBNE
		case 0xe3:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "DBNE $%x", rel);
			flags = STEP_OVER | step_over_extra(1);

			break;
		}

		// DBC
		case 0xe4:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "DBC $%x", rel);
			flags = STEP_OVER | step_over_extra(1);

			break;
		}

		// DBNC
		case 0xe5:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "DBNC $%x", rel);
			flags = STEP_OVER | step_over_extra(1);

			break;
		}

		// DBSE
		case 0xe6:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "DBSE $%x", rel);
			flags = STEP_OVER | step_over_extra(1);

			break;
		}

		// DBHT
		case 0xe7:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "DBHT $%x", rel);
			flags = STEP_OVER | step_over_extra(1);

			break;
		}

		// DBN
		case 0xe8:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "DBN $%x", rel);
			flags = STEP_OVER | step_over_extra(1);

			break;
		}

		// DBNN
		case 0xe9:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "DBNN $%x", rel);
			flags = STEP_OVER | step_over_extra(1);

			break;
		}

		// DBLE
		case 0xea:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "DBLE $%x", rel);
			flags = STEP_OVER | step_over_extra(1);

			break;
		}

		// DBGT
		case 0xeb:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "DBGT $%x", rel);
			flags = STEP_OVER | step_over_extra(1);

			break;
		}

		// DBR
		case 0xec:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "DBR $%x", rel);
			flags = STEP_OVER | step_over_extra(1);

			break;
		}

		// FRAME
		case 0xed:

			global_fp = 0;
			LL_format(source, dest, op);

			util::stream_format(stream, "FRAME %s, %s", dest, source);

			break;

		// CALL
		case 0xee: case 0xef:
		{
			uint32_t const_val = LRconst_format(source, dest, op, pc, opcodes);

			if( source_code == SR_REGISTER && !source_bit )
			{
				util::stream_format(stream, "CALL %s, 0, $%x", dest, const_val);
				flags = STEP_OVER;
			}
			else
			{
				util::stream_format(stream, "CALL %s, %s, $%x", dest, source, const_val);
				flags = STEP_OVER;
			}

			break;
		}

		// BV
		case 0xf0:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "BV $%x", rel);

			break;
		}

		// BNV
		case 0xf1:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "BNV $%x", rel);

			break;
		}

		// BE
		case 0xf2:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "BE $%x", rel);

			break;
		}

		// BNE
		case 0xf3:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "BNE $%x", rel);

			break;
		}

		// BC
		case 0xf4:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "BC $%x", rel);

			break;
		}

		// BNC
		case 0xf5:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "BNC $%x", rel);

			break;
		}

		// BSE
		case 0xf6:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "BSE $%x", rel);

			break;
		}

		// BHT
		case 0xf7:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "BHT $%x", rel);

			break;
		}

		// BN
		case 0xf8:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "BN $%x", rel);

			break;
		}

		// BNN
		case 0xf9:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "BNN $%x", rel);

			break;
		}

		// BLE
		case 0xfa:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "BLE $%x", rel);

			break;
		}

		// BGT
		case 0xfb:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "BGT $%x", rel);

			break;
		}

		// BR
		case 0xfc:
		{
			int32_t rel = PCrel_format(op, pc, opcodes) + 2;

			util::stream_format(stream, "BR $%x", rel);

			break;
		}

		// TRAPxx - TRAP
		case 0xfd: case 0xfe: case 0xff:
		{
			uint8_t code = ((op & 0x300) >> 6) | (op & 0x03);
			uint8_t trapno = (op & 0xfc) >> 2;

			switch( code )
			{
				case TRAPLE:
					util::stream_format(stream, "TRAPLE %d", trapno);
					flags = STEP_OVER;

					break;

				case TRAPGT:
					util::stream_format(stream, "TRAPGT %d", trapno);
					flags = STEP_OVER;

					break;

				case TRAPLT:
					util::stream_format(stream, "TRAPLT %d", trapno);
					flags = STEP_OVER;

					break;

				case TRAPGE:
					util::stream_format(stream, "TRAPGE %d", trapno);
					flags = STEP_OVER;

					break;

				case TRAPSE:
					util::stream_format(stream, "TRAPSE %d", trapno);
					flags = STEP_OVER;

					break;

				case TRAPHT:
					util::stream_format(stream, "TRAPHT %d", trapno);
					flags = STEP_OVER;

					break;

				case TRAPST:
					util::stream_format(stream, "TRAPST %d", trapno);
					flags = STEP_OVER;

					break;

				case TRAPHE:
					util::stream_format(stream, "TRAPHE %d", trapno);
					flags = STEP_OVER;

					break;

				case TRAPE:
					util::stream_format(stream, "TRAPE %d", trapno);
					flags = STEP_OVER;

					break;

				case TRAPNE:
					util::stream_format(stream, "TRAPNE %d", trapno);
					flags = STEP_OVER;

					break;

				case TRAPV:
					util::stream_format(stream, "TRAPV %d", trapno);
					flags = STEP_OVER;

					break;

				case TRAP:
					util::stream_format(stream, "TRAP %d", trapno);
					flags = STEP_OVER;

					break;
			}

			break;
		}
	}

	return size | flags | SUPPORTED;
}
