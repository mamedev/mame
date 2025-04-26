// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/*

 Hyperstone disassembler
 written by Pierpaolo Prazzoli

*/

#include "emu.h"
#include "32xsdasm.h"
#include "32xsdefs.h"


namespace {

const char *const L_REG[] =
{
	"L0",  "L1",  "L2",  "L3",  "L4",  "L5",  "L6",  "L7",
	"L8",  "L9",  "L10", "L11", "L12", "L13", "L14", "L15"
};

const char *const G_REG[] =
{
	"PC",  "SR",  "FER", "G03", "G04", "G05", "G06", "G07",
	"G08", "G09", "G10", "G11", "G12", "G13", "G14", "G15",
	"G16", "G17", "SP",  "UB",  "BCR", "TPR", "TCR", "TR",
	"WCR", "ISR", "FCR", "MCR", "G28", "G29", "G30", "G31"
};

const char *const SETxx[] =
{
	"SETADR",   nullptr,    "SET1",   "SET0",     "SETLE",  "SETGT",  "SETLT",  "SETGE",
	"SETSE",    "SETHT",    "SETST",  "SETHE",    "SETE",   "SETNE",  "SETV",   "SETNV",
	nullptr,    nullptr,    "SET1M",  nullptr,    "SETLEM", "SETGTM", "SETLTM", "SETGEM",
	"SETSEM",   "SETTHM",   "SETSTM", "SETHEM",   "SETEM",  "SETNEM", "SETVM",  "SETNVM"
};

const char *const Fxxx[] =
{
	"FADD",   "FADDD",  "FSUB",   "FSUBD",  "FMUL",   "FMULD", "FDIV", "FDIVD",
	"FCMP",   "FCMPD",  "FCMPU",  "FCMPUD", "FCVT",   "FCVTD"
};

const char *const Bxx[] =
{
	"BV",  "BNV", "BE",  "BNE", "BC",  "BNC", "BSE", "BHT",
	"BN",  "BNN", "BLE", "BGT", "BR"
};

const char *const TRAPxx[] =
{
	nullptr,  nullptr,  nullptr,  nullptr,  "TRAPLE", "TRAPGT", "TRAPLT", "TRAPGE",
	"TRAPSE", "TRAPHT", "TRAPST", "TRAPHE", "TRAPE",  "TRAPNE", "TRAPV",  "TRAP"
};

#define DESTCODE(op)            ((op & 0x00f0) >> 4)
#define SOURCECODE(op)          (op & 0x000f)

#define SOURCEBIT(op)           ((op & 0x100) >> 8)
#define DESTBIT(op)             ((op & 0x200) >> 9)

#define DASM_N_VALUE(op)        ((((op & 0x100) >> 8) << 4 ) | (op & 0x0f))


void LL_format(std::string_view &source, std::string_view &dest, uint16_t op)
{
	source = L_REG[SOURCECODE(op)];
	dest = L_REG[DESTCODE(op)];
}

void LR_format(std::string_view &source, std::string_view &dest, uint16_t op)
{
	if (SOURCEBIT(op))
		source = L_REG[SOURCECODE(op)];
	else
		source = G_REG[SOURCECODE(op)];

	dest = L_REG[DESTCODE(op)];
}

void RR_format(std::string_view &source, std::string_view &dest, uint16_t op, unsigned h_flag)
{
	if (SOURCEBIT(op))
		source = L_REG[SOURCECODE(op)];
	else
		source = G_REG[SOURCECODE(op) + h_flag * 16];

	if (DESTBIT(op))
		dest = L_REG[DESTCODE(op)];
	else
		dest = G_REG[DESTCODE(op) + h_flag * 16];
}

uint8_t Ln_format(std::string_view &dest, uint16_t op)
{
	dest = L_REG[DESTCODE(op)];

	return DASM_N_VALUE(op);
}

uint8_t Rn_format(std::string_view &dest, uint16_t op)
{
	if (DESTBIT(op))
		dest = L_REG[DESTCODE(op)];
	else
		dest = G_REG[DESTCODE(op)];

	return DASM_N_VALUE(op);
}


void format_addsub(
		std::ostream &stream,
		std::string_view inst,
		std::string_view dest,
		std::string_view source,
		uint8_t source_bit,
		uint8_t source_code)
{
	util::stream_format(stream, "%-7s %s, %s",
			inst,
			dest,
			(!source_bit && (source_code == SR_REGISTER)) ? "C" : source);
}

} // anonymous namespace



uint32_t hyperstone_disassembler::LRconst_format(std::string_view &source, std::string_view &dest, uint16_t op, offs_t pc, const data_buffer &opcodes)
{
	uint16_t next_op;
	uint32_t const_val;

	if (SOURCEBIT(op))
		source = L_REG[SOURCECODE(op)];
	else
		source = G_REG[SOURCECODE(op)];

	dest = L_REG[DESTCODE(op)];

	size = 4;

	pc += 2;
	next_op = opcodes.r16(pc);

	if (E_BIT(next_op))
	{
		uint16_t next_op2;

		size = 6;

		pc += 2;
		next_op2 = opcodes.r16(pc);
		const_val = next_op2;
		const_val |= ((next_op & 0x3fff) << 16 );

		if (S_BIT_CONST(next_op))
			const_val |= 0xc0000000;
	}
	else
	{
		const_val = next_op & 0x3fff;

		if (S_BIT_CONST(next_op))
			const_val |= 0xffffc000;
	}

	return const_val;
}

uint32_t hyperstone_disassembler::RRconst_format(std::string_view &source, std::string_view &dest, uint16_t op, offs_t pc, const data_buffer &opcodes)
{
	uint16_t next_op;
	uint32_t const_val;

	if (SOURCEBIT(op))
		source = L_REG[SOURCECODE(op)];
	else
		source = G_REG[SOURCECODE(op)];

	if (DESTBIT(op))
		dest = L_REG[DESTCODE(op)];
	else
		dest = G_REG[DESTCODE(op)];

	size = 4;

	pc += 2;
	next_op = opcodes.r16(pc);

	if (E_BIT(next_op))
	{
		uint16_t next_op2;

		size = 6;

		pc += 2;
		next_op2 = opcodes.r16(pc);
		const_val = next_op2;
		const_val |= ((next_op & 0x3fff) << 16 );

		if (S_BIT_CONST(next_op))
		{
			const_val |= 0xc0000000;
		}
	}
	else
	{
		const_val = next_op & 0x3fff;

		if (S_BIT_CONST(next_op))
			const_val |= 0xffffc000;
	}

	return const_val;
}

int32_t hyperstone_disassembler::Rimm_format(std::string_view &dest, uint16_t op, offs_t pc, const data_buffer &opcodes, unsigned h_flag)
{
	uint16_t imm1, imm2;
	int32_t ret;

	int n = DASM_N_VALUE(op);

	if (DESTBIT(op))
		dest = L_REG[DESTCODE(op)];
	else
		dest = G_REG[DESTCODE(op) + h_flag * 16];

	switch (n)
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

uint32_t hyperstone_disassembler::RRdis_format(std::string_view &source, std::string_view &dest, uint16_t op, uint16_t next_op, offs_t pc, const data_buffer &opcodes)
{
	uint32_t ret;

	if (SOURCEBIT(op))
		source = L_REG[SOURCECODE(op)];
	else
		source = G_REG[SOURCECODE(op)];

	if (DESTBIT(op))
		dest = L_REG[DESTCODE(op)];
	else
		dest = G_REG[DESTCODE(op)];

	if (E_BIT(next_op))
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
	std::string_view source, dest;
	uint32_t flags = 0;

	const uint16_t op = opcodes.r16(pc);

	size = 2;

	const uint8_t source_code = SOURCECODE(op);
	const uint8_t dest_code = DESTCODE(op);
	const uint8_t source_bit = SOURCEBIT(op);
	const uint8_t dest_bit = DESTBIT(op);

	const int h_flag = m_config->get_h();

	const uint8_t op_num = (op & 0xff00) >> 8;

	switch (op_num)
	{
		// CHK - CHKZ - NOP
		case 0x00: case 0x01: case 0x02: case 0x03:
			if (source_bit && dest_bit && !source_code && !dest_code)
			{
				util::stream_format(stream, "NOP");
			}
			else
			{
				RR_format(source, dest, op, 0);
				if (!source_bit && (source_code == SR_REGISTER))
					util::stream_format(stream, "CHKZ    %s, 0", dest);
				else
					util::stream_format(stream, "CHK     %s, %s", dest, source);
			}
			break;

		// MOVD - RET
		case 0x04: case 0x05: case 0x06: case 0x07:
			RR_format(source, dest, op, 0);

			if (!dest_bit && (dest_code == PC_REGISTER))
			{
				if (!source_bit && ((source_code == PC_REGISTER) || (source_code == SR_REGISTER)))
				{
					// RET with PC or SR as source is reserved for future expansion
					util::stream_format(stream, "D.HU    $%04x", op);
				}
				else
				{
					RR_format(source, dest, op, 0);
					util::stream_format(stream, "RET     PC, %s", source);
					flags = STEP_OUT;
				}
			}
			else if (!source_bit && (source_code == SR_REGISTER))
			{
				util::stream_format(stream, "MOVD    %s, 0", dest);
			}
			else
			{
				util::stream_format(stream, "MOVD    %s, %s", dest, source);
			}
			break;

		case 0x08: case 0x09: case 0x0a: case 0x0b: // DIVU
		case 0x0c: case 0x0d: case 0x0e: case 0x0f: // DIVS
			RR_format(source, dest, op, 0);
			util::stream_format(stream, "DIV%c    %s, %s", (op_num & 0x4) ? 'S' : 'U', dest, source);
			break;

		// XMx - XXx
		case 0x10: case 0x11: case 0x12: case 0x13:
			{
				RR_format(source, dest, op, 0);

				size = 4;

				pc += 2;
				const uint16_t ilm1 = opcodes.r16(pc);

				const int xcode = X_CODE(ilm1);

				if (xcode < 4)
				{
					uint16_t lim;

					if (E_BIT(ilm1))
					{
						size = 6;

						pc += 2;
						const uint16_t ilm2 = opcodes.r16(pc);

						lim = ((ilm1 & 0xfff) << 16) | ilm2;
					}
					else
					{
						lim = ilm1 & 0xfff;
					}

					util::stream_format(stream, "XM%x     %s, %s, $%x", 1 << xcode, dest, source, lim);
				}
				else
				{
					util::stream_format(stream, "XX%x     %s, %s, 0", 1 << (xcode  & 0x3), dest, source);
				}
			}
			break;

		// MASK
		case 0x14: case 0x15: case 0x16: case 0x17:
			{
				const uint32_t const_val = RRconst_format(source, dest, op, pc, opcodes);
				util::stream_format(stream, "MASK    %s, %s, $%x", dest, source, const_val);
				if (!source_bit && (source_code == PC_REGISTER))
					util::stream_format(stream, " ; $%x", uint32_t((pc + size) & const_val));
			}
			break;

		case 0x18: case 0x19: case 0x1a: case 0x1b: // SUM
		case 0x1c: case 0x1d: case 0x1e: case 0x1f: // SUMS
			{
				const uint32_t const_val = RRconst_format(source, dest, op, pc, opcodes);
				util::stream_format(stream, "SUM%c    %s, %s, $%x",
						(op_num & 0x4) ? 'S' : ' ',
						dest,
						(!source_bit && (source_code == SR_REGISTER)) ? "C" : source,
						const_val);
				if (!source_bit && (source_code == PC_REGISTER))
					util::stream_format(stream, " ; $%x", uint32_t(pc + size + const_val));
			}
			break;

		// CMP
		case 0x20: case 0x21: case 0x22: case 0x23:
			RR_format(source, dest, op, 0);
			format_addsub(stream, "CMP", dest, source, source_bit, source_code);
			break;

		// MOV
		case 0x24: case 0x25: case 0x26: case 0x27:
			RR_format(source, dest, op, h_flag);
			util::stream_format(stream, "MOV     %s, %s", dest, source);
			break;

		case 0x28: case 0x29: case 0x2a: case 0x2b: // ADD
		case 0x2c: case 0x2d: case 0x2e: case 0x2f: // ADDS
			RR_format(source, dest, op, 0);
			format_addsub(stream, (op_num & 0x4) ? "ADDS" : "ADD", dest, source, source_bit, source_code);
			break;

		// CMPB
		case 0x30: case 0x31: case 0x32: case 0x33:
			RR_format(source, dest, op, 0);
			util::stream_format(stream, "CMPB    %s, %s", dest, source);
			break;

		// ANDN
		case 0x34: case 0x35: case 0x36: case 0x37:
			RR_format(source, dest, op, 0);
			util::stream_format(stream, "ANDN    %s, %s", dest, source);
			break;

		// OR
		case 0x38: case 0x39: case 0x3a: case 0x3b:
			RR_format(source, dest, op, 0);
			util::stream_format(stream, "OR      %s, %s", dest, source);
			break;

		// XOR
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			RR_format(source, dest, op, 0);
			util::stream_format(stream, "XOR     %s, %s", dest, source);
			break;

		// SUBC
		case 0x40: case 0x41: case 0x42: case 0x43:
			RR_format(source, dest, op, 0);
			format_addsub(stream, "SUBC", dest, source, source_bit, source_code);
			break;

		// NOT
		case 0x44: case 0x45: case 0x46: case 0x47:
			RR_format(source, dest, op, 0);
			util::stream_format(stream, "NOT     %s, %s", dest, source);
			break;

		case 0x48: case 0x49: case 0x4a: case 0x4b: // SUB
		case 0x4c: case 0x4d: case 0x4e: case 0x4f: // SUBS
			RR_format(source, dest, op, 0);
			format_addsub(stream, (op_num & 0x4) ? "SUBS" : "SUB", dest, source, source_bit, source_code);
			break;

		// ADDC
		case 0x50: case 0x51: case 0x52: case 0x53:
			RR_format(source, dest, op, 0);
			format_addsub(stream, "ADDC", dest, source, source_bit, source_code);
			break;

		// AND
		case 0x54: case 0x55: case 0x56: case 0x57:
			RR_format(source, dest, op, 0);
			util::stream_format(stream, "AND     %s, %s", dest, source);
			break;

		case 0x58: case 0x59: case 0x5a: case 0x5b: // NEG
		case 0x5c: case 0x5d: case 0x5e: case 0x5f: // NEGS
			RR_format(source, dest, op, 0);
			format_addsub(stream, (op_num & 0x4) ? "NEGS" : "NEG", dest, source, source_bit, source_code);
			break;

		// CMPI
		case 0x60: case 0x61: case 0x62: case 0x63:
			{
				const uint32_t imm = Rimm_format(dest, op, pc, opcodes, 0);
				util::stream_format(stream, "CMPI    %s, $%x", dest, imm);
			}
			break;

		// MOVI
		case 0x64: case 0x65: case 0x66: case 0x67:
			{
				const uint32_t imm = Rimm_format(dest, op, pc, opcodes, h_flag);
				util::stream_format(stream, "MOVI    %s, $%x", dest, imm);
			}
			break;

		case 0x68: case 0x69: case 0x6a: case 0x6b: // ADDI
		case 0x6c: case 0x6d: case 0x6e: case 0x6f: // ADDSI
			if (!DASM_N_VALUE(op))
			{
				util::stream_format(stream, "%-7s %s, CZ", (op_num & 0x4) ? "ADDSI" : "ADDI", dest);
			}
			else
			{
				const uint32_t imm = Rimm_format(dest, op, pc, opcodes, 0);
				util::stream_format(stream, "%-7s %s, $%x", (op_num & 0x4) ? "ADDSI" : "ADDI", dest, imm);
				if (!dest_bit && (dest_code == PC_REGISTER))
					util::stream_format(stream, " ; $%x", uint32_t(pc + size + imm));
			}
			break;

		// CMPBI
		case 0x70: case 0x71: case 0x72: case 0x73:
			if (!DASM_N_VALUE(op))
			{
				util::stream_format(stream, "CMPBI   %s, ANYBZ", dest);
			}
			else
			{
				uint32_t imm = Rimm_format(dest, op, pc, opcodes, 0);
				if (DASM_N_VALUE(op) == 31)
					imm = 0x7fffffff; //bit 31 = 0, others = 1

				util::stream_format(stream, "CMPBI   %s, $%x", dest, imm);
			}
			break;

		// ANDNI
		case 0x74: case 0x75: case 0x76: case 0x77:
			{
				uint32_t imm = Rimm_format(dest, op, pc, opcodes, 0);
				if (DASM_N_VALUE(op) == 31)
					imm = 0x7fffffff; //bit 31 = 0, others = 1

				util::stream_format(stream, "ANDNI   %s, $%x", dest, imm);
				if (!dest_bit && (dest_code == PC_REGISTER))
					util::stream_format(stream, " ; $%x", uint32_t((pc + size) & ~imm));
			}
			break;

		// ORI
		case 0x78: case 0x79: case 0x7a: case 0x7b:
			{
				const uint32_t imm = Rimm_format(dest, op, pc, opcodes, 0);

				util::stream_format(stream, "ORI     %s, $%x", dest, imm);
				if (!dest_bit && (dest_code == PC_REGISTER))
					util::stream_format(stream, " ; $%x", uint32_t((pc + size) | imm));
			}
			break;

		// XORI
		case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			{
				const uint32_t imm = Rimm_format(dest, op, pc, opcodes, 0);

				util::stream_format(stream, "XORI    %s, $%x", dest, imm);
				if (!dest_bit && (dest_code == PC_REGISTER))
					util::stream_format(stream, " ; $%x", uint32_t((pc + size) ^ imm));
			}
			break;

		case 0x80: case 0x81: // SHRDI
		case 0x84: case 0x85: // SARDI
			{
				const uint8_t n = Ln_format(dest, op);
				util::stream_format(stream, "S%cRDI   %s, $%x", (op_num & 0x4) ? 'A' : 'H', dest, n);
			}
			break;

		case 0x82: // SHRD
		case 0x83: // SHR
		case 0x86: // SARD
		case 0x87: // SAR
			LL_format(source, dest, op);
			util::stream_format(stream, "S%cR%c    %s, %s",
					(op_num & 0x4) ? 'A' : 'H',
					(op_num & 0x1) ? ' ' : 'D',
					dest,
					source);
			break;

		// SHLDI
		case 0x88: case 0x89:
			{
				const uint8_t n = Ln_format(dest, op);
				util::stream_format(stream, "SHLDI   %s, $%x", dest, n);
			}
			break;

		case 0x8a: // SHLD
		case 0x8b: // SHL
			LL_format(source, dest, op);
			util::stream_format(stream, "SHL%c    %s, %s", (op_num & 0x1) ? ' ' : 'D', dest, source);
			break;

		// RESERVED
		case 0x8c: case 0x8d:
		case 0xac: case 0xad: case 0xae: case 0xaf:
			util::stream_format(stream, "D.HU    $%04x", op);
			break;

		// TESTLZ
		case 0x8e:
			LL_format(source, dest, op);
			util::stream_format(stream, "TESTLZ  %s, %s", dest, source);
			break;

		// ROL
		case 0x8f:
			LL_format(source, dest, op);
			util::stream_format(stream, "ROL     %s, %s", dest, source);
			break;

		case 0x90: case 0x91: case 0x92: case 0x93: // LDxx.D/A/IOD/IOA
		case 0x98: case 0x99: case 0x9a: case 0x9b: // STxx.D/A/IOD/IOA
			{
				const uint16_t next_op = opcodes.r16(pc + 2);
				const uint32_t dis = RRdis_format(source, dest, op, next_op, pc, opcodes);

				if (size == 2)
					size = 4;

				if ((op_num & 0x8) && !source_bit && (source_code == SR_REGISTER))
					source = "0";

				if (!dest_bit && (dest_code == SR_REGISTER))
					dest = "0";

				const std::string_view inst = (op_num & 0x8) ? "ST" : "LD";
				const char mode = (!dest_bit && (dest_code == SR_REGISTER)) ? 'A' : 'D';

				switch (DD(next_op))
				{
					case 0:
						// LD/STBS.D/A
						util::stream_format(stream, "%sBS.%c  %s, %s, $%x", inst, mode, dest, source, dis);
						if (!dest_bit && (dest_code == PC_REGISTER))
							util::stream_format(stream, " ; $%x", uint32_t(pc + size + dis));
						break;

					case 1:
						// LD/STBU.D/A
						util::stream_format(stream, "%sBU.%c  %s, %s, $%x", inst, mode, dest, source, dis);
						if (!dest_bit && (dest_code == PC_REGISTER))
							util::stream_format(stream, " ; $%x", uint32_t(pc + size + dis));
						break;

					case 2:
						// LD/STHU.D/A
						// LD/STHS.D/A
						util::stream_format(stream, "%sH%c.%c  %s, %s, $%x", inst, (dis & 1) ? 'S' : 'U', mode, dest, source, dis & ~1);
						if (!dest_bit && (dest_code == PC_REGISTER))
							util::stream_format(stream, " ; $%x", uint32_t(pc + size + (dis & ~1)) & ~1);
						break;

					case 3:
						if (dis & 2)
						{
							// LD/STW.IOD/A
							// LD/STD.IOD/A
							util::stream_format(stream, "%s%c.IO%c %s, %s, $%x", inst, (dis & 1) ? 'D' : 'W', mode, dest, source, dis & ~3);
						}
						else
						{
							// LD/STW.D/A
							// LD/STD.D/A
							util::stream_format(stream, "%s%c.%c   %s, %s, $%x", inst, (dis & 1) ? 'D' : 'W', mode, dest, source, dis & ~3);
						}
						if (!dest_bit && (dest_code == PC_REGISTER))
							util::stream_format(stream, " ; $%x", uint32_t(pc + size + (dis & ~3)) & ~3);
						break;
				}
			}
			break;

		case 0x94: case 0x95: case 0x96: case 0x97: // LDxx.N/S
		case 0x9c: case 0x9d: case 0x9e: case 0x9f: // STxx.N/S
			{
				const uint16_t next_op = opcodes.r16(pc + 2);
				const uint32_t dis = RRdis_format(source, dest, op, next_op, pc, opcodes);

				if (size == 2)
					size = 4;

				if (!dest_bit && ((dest_code == PC_REGISTER) || (dest_code == SR_REGISTER)))
				{
					util::stream_format(stream, "Reserved");
					break;
				}

				if ((op_num & 0x8) && !source_bit && (source_code == SR_REGISTER))
					source = "0";

				const std::string_view inst = (op_num & 0x8) ? "ST" : "LD";

				switch (DD(next_op))
				{
					case 0:
						// LD/STBS.N
						util::stream_format(stream, "%sBS.N  %s, %s, $%x", inst, dest, source, dis);
						break;

					case 1:
						// LD/STBU.N
						util::stream_format(stream, "%sBU.N  %s, %s, $%x", inst, dest, source, dis);
						break;

					case 2:
						// LD/STHU.N
						// LD/STHS.N
						util::stream_format(stream, "%sH%c.N  %s, %s, $%x", inst, (dis & 1) ? 'S' : 'U', dest, source, dis & ~1);
						break;

					case 3:
						if ((dis & 3) == 3)
						{
							// LD/STW.S
							util::stream_format(stream, "%sW.S   %s, %s, $%x", inst, dest, source, dis & ~3);
						}
						else if ((dis & 3) == 2)
						{
							// Reserved
							util::stream_format(stream, "Reserved");
						}
						else
						{
							// LD/STW.N
							// LD/STD.N
							util::stream_format(stream, "%s%c.N   %s, %s, $%x", inst, (dis & 1) ? 'D' : 'W', dest, source, dis & ~3);
						}
						break;
				}
			}
			break;

		case 0xa0: case 0xa1: case 0xa2: case 0xa3: // SHRI
		case 0xa4: case 0xa5: case 0xa6: case 0xa7: // SARI
			{
				const uint8_t n = Rn_format(dest, op);
				util::stream_format(stream, "S%cRI    %s, $%x", (op_num & 0x4) ? 'A' : 'H', dest, n);
			}
			break;

		// SHLI
		case 0xa8: case 0xa9: case 0xaa: case 0xab:
			{
				const uint8_t n = Rn_format(dest, op);
				util::stream_format(stream, "SHLI    %s, $%x", dest, n);
			}
			break;

		case 0xb0: case 0xb1: case 0xb2: case 0xb3: // MULU
		case 0xb4: case 0xb5: case 0xb6: case 0xb7: // MULS
			RR_format(source, dest, op, 0);
			util::stream_format(stream, "MUL%c    %s, %s", (op_num & 0x4) ? 'S' : 'U', dest, source);
			break;

		// SETxx - SETADR - FETCH
		case 0xb8: case 0xb9: case 0xba: case 0xbb:
			{
				const uint8_t n = Rn_format(dest, op);

				if (!dest_bit && (dest_code == PC_REGISTER))
				{
					// SETxx with PC as destination is reserved for future use
					util::stream_format(stream, "D.HU    $%04x", op);
				}
				else if (!dest_bit && (dest_code == SR_REGISTER))
				{
					util::stream_format(stream, "FETCH   $%x", (n / 2) + 1);
				}
				else
				{
					char const *const inst = SETxx[n];
					if (inst)
						util::stream_format(stream, "%-7s %s", inst, dest);
					else
						util::stream_format(stream, "D.HU    $%04x", op);
				}
			}
			break;

		// MUL
		case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			RR_format(source, dest, op, 0);
			util::stream_format(stream, "MUL     %s, %s", dest, source);
			break;

		case 0xc0: // FADD
		case 0xc1: // FADDD
		case 0xc2: // FSUB
		case 0xc3: // FSUBD
		case 0xc4: // FMUL
		case 0xc5: // FMULD
		case 0xc6: // FDIV
		case 0xc7: // FDIVD
		case 0xc8: // FCMP
		case 0xc9: // FCMPD
		case 0xca: // FCMPU
		case 0xcb: // FCMPUD
		case 0xcc: // FCVT
		case 0xcd: // FCVTD
			LL_format(source, dest, op);
			util::stream_format(stream, "%-7s %s, %s", Fxxx[op_num & 0xf], dest, source);
			break;

		// EXTEND
		case 0xce:
			{
				LL_format(source, dest, op);

				pc += 2;
				const uint16_t extended_op = opcodes.r16(pc);

				size = 4;

				switch (extended_op)
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
						util::stream_format(stream, "Ext. OP $%X @ %X", extended_op, pc);
						osd_printf_verbose("Illegal Extended Opcode: %X @ %X\n", extended_op, pc);
						break;
				}
			}
			break;

		// DO
		case 0xcf:
			LL_format(source, dest, op);
			util::stream_format(stream, "DO      %s, %s", dest, source);
			break;

		case 0xd0: case 0xd1: // LDW.R
		case 0xd2: case 0xd3: // LDD.R
		case 0xd4: case 0xd5: // LDW.P
		case 0xd6: case 0xd7: // LDD.P
		case 0xd8: case 0xd9: // STW.R
		case 0xda: case 0xdb: // STD.R
		case 0xdc: case 0xdd: // STW.P
		case 0xde: case 0xdf: // STD.P
			LR_format(source, dest, op);
			util::stream_format(stream, "%s%c.%c   %s, %s",
					(op_num & 0x8) ? "ST" : "LD",
					(op_num & 0x2) ? 'D' : 'W',
					(op_num & 0x4) ? 'P' : 'R',
					dest,
					((op_num & 0x8) && !source_bit && (source_code == SR_REGISTER)) ? "0" : source);
			break;

		case 0xe0: // DBV
		case 0xe1: // DBNV
		case 0xe2: // DBE
		case 0xe3: // DBNE
		case 0xe4: // DBC
		case 0xe5: // DBNC
		case 0xe6: // DBSE
		case 0xe7: // DBHT
		case 0xe8: // DBN
		case 0xe9: // DBNN
		case 0xea: // DBLE
		case 0xeb: // DBGT
		case 0xec: // DBR
			{
				const int32_t rel = PCrel_format(op, pc, opcodes) + 2;
				util::stream_format(stream, "D%-6s $%x", Bxx[op_num & 0xf], rel);
				if (op_num != 0xec)
					flags = STEP_COND | step_over_extra(1);
			}
			break;

		// FRAME
		case 0xed:
			LL_format(source, dest, op);
			util::stream_format(stream, "FRAME   %s, %s", dest, source);
			break;

		// CALL
		case 0xee: case 0xef:
			{
				const uint32_t const_val = LRconst_format(source, dest, op, pc, opcodes);

				util::stream_format(stream, "CALL    %s, %s, $%x", dest, (!source_bit && (source_code == SR_REGISTER)) ? "0" : source, const_val);
				if (!source_bit && (source_code == PC_REGISTER))
					util::stream_format(stream, " ; $%x", uint32_t(pc + size + const_val));
				flags = STEP_OVER;
			}
			break;

		case 0xf0: // BV
		case 0xf1: // BNV
		case 0xf2: // BE
		case 0xf3: // BNE
		case 0xf4: // BC
		case 0xf5: // BNC
		case 0xf6: // BSE
		case 0xf7: // BHT
		case 0xf8: // BN
		case 0xf9: // BNN
		case 0xfa: // BLE
		case 0xfb: // BGT
		case 0xfc: // BR
			{
				const int32_t rel = PCrel_format(op, pc, opcodes) + 2;
				util::stream_format(stream, "%-7s $%x", Bxx[op_num & 0xf], rel);
				if (op_num != 0xfc)
					flags = STEP_COND;
			}
			break;

		// TRAPxx - TRAP
		case 0xfd: case 0xfe: case 0xff:
			{
				const uint8_t code = ((op & 0x300) >> 6) | (op & 0x03);
				const uint8_t trapno = (op & 0xfc) >> 2;

				util::stream_format(stream, "%-7s %d", TRAPxx[code], trapno);
				if (code == TRAP)
					flags = STEP_OVER;
				else
					flags = STEP_OVER | STEP_COND;
			}
			break;
	}

	return size | flags | SUPPORTED;
}
