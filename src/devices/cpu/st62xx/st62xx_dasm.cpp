// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    STmicro ST6-series microcontroller disassembler

    To Do:
        - Named registers/bits

**********************************************************************/

#include "emu.h"
#include "st62xx_dasm.h"

uint32_t st62xx_disassembler::opcode_alignment() const
{
	return 1;
}

std::string st62xx_disassembler::reg_name(const uint8_t reg)
{
	static char const *const REG_NAMES[256] =
	{
		nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,

		nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,

		"X",     "Y",     "V",     "W",      nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,

		"DRA",   "DRB",   "DRC",   "DRD",    "DDRA",  "DDRB",  "DDRC",  "DDRD",   "IOR",   "DWR",   "PRPR",  "DRBR",   "ORA",   "ORB",   "ORC",   "ORD",
		"ADR",   "ADCR",  "PSC",   "TCR",    "TSCR",  nullptr, "UARTDR","UARTCR", "DWDR",  nullptr, "IPR",   nullptr,  "SIDR",  "SDSR",  nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr,  nullptr, "ARMC",  "ARSC0", "ARSC1",  nullptr, "ARRC",  "ARCP",  "ARLR",   nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, "A",
	};

	if (REG_NAMES[reg])
		return std::string(REG_NAMES[reg]);
	else
		return util::string_format("$%02Xh", reg);
}

offs_t st62xx_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	offs_t base_pc = pc;
	offs_t flags = 0;
	const uint8_t op = opcodes.r8(pc);
	pc++;

	switch (op)
	{
		case 0x00: case 0x10: case 0x20: case 0x30: case 0x40: case 0x50: case 0x60: case 0x70:
		case 0x80:            case 0xa0: case 0xb0: case 0xc0: case 0xd0: case 0xe0: case 0xf0:
		case 0x08: case 0x18: case 0x28: case 0x38: case 0x48: case 0x58: case 0x68: case 0x78:
		case 0x88:            case 0xa8: case 0xb8: case 0xc8: case 0xd8: case 0xe8: case 0xf8:
		{
			const int8_t e = ((int8_t)op) >> 3;
			util::stream_format(stream, "JRNZ $%d", e);
			flags = STEP_COND;
			break;
		}
		case 0x01: case 0x11: case 0x21: case 0x31: case 0x41: case 0x51: case 0x61: case 0x71:
		case 0x81: case 0x91: case 0xa1: case 0xb1: case 0xc1: case 0xd1: case 0xe1: case 0xf1:
		{
			const uint8_t ab = opcodes.r8(pc);
			pc++;
			const uint16_t abc = ((op & 0xf0) >> 4) | (ab << 4);
			util::stream_format(stream, "CALL %Xh", abc);
			flags = STEP_OVER;
			break;
		}
		case 0x09: case 0x19: case 0x29: case 0x39: case 0x49: case 0x59: case 0x69: case 0x79:
		case 0x89: case 0x99: case 0xa9: case 0xb9: case 0xc9: case 0xd9: case 0xe9: case 0xf9:
		{
			const uint8_t ab = opcodes.r8(pc);
			pc++;
			const uint16_t abc = ((op & 0xf0) >> 4) | (ab << 4);
			util::stream_format(stream, "JP   %Xh", abc);
			break;
		}
		case 0x02: case 0x12: case 0x22: case 0x32: case 0x42: case 0x52: case 0x62: case 0x72:
		case 0x82: case 0x92: case 0xa2: case 0xb2: case 0xc2: case 0xd2: case 0xe2: case 0xf2:
		case 0x0a: case 0x1a: case 0x2a: case 0x3a: case 0x4a: case 0x5a: case 0x6a: case 0x7a:
		case 0x8a: case 0x9a: case 0xaa: case 0xba: case 0xca: case 0xda: case 0xea: case 0xfa:
		{
			const int8_t e = ((int8_t)op) >> 3;
			util::stream_format(stream, "JRNC $%d", e);
			flags = STEP_COND;
			break;
		}
		case 0x03: case 0x23: case 0x43: case 0x63: case 0x83: case 0xa3: case 0xc3: case 0xe3:
		{
			const uint8_t b = (op >> 5) & 7;
			const uint8_t rr = opcodes.r8(pc);
			pc++;
			const int8_t ee = (int8_t)opcodes.r8(pc);
			pc++;
			util::stream_format(stream, "JRR  %d,%s,$%Xh", b, reg_name(rr), ee);
			flags = STEP_COND;
			break;
		}
		case 0x13: case 0x33: case 0x53: case 0x73: case 0x93: case 0xb3: case 0xd3: case 0xf3:
		{
			const uint8_t b = (op >> 5) & 7;
			const uint8_t rr = opcodes.r8(pc);
			pc++;
			const int8_t ee = (int8_t)opcodes.r8(pc);
			pc++;
			util::stream_format(stream, "JRS  %d,%s,$%Xh", b, reg_name(rr), ee);
			flags = STEP_COND;
			break;
		}
		case 0x0b: case 0x2b: case 0x4b: case 0x6b: case 0x8b: case 0xab: case 0xcb: case 0xeb:
		{
			const uint8_t b = (op >> 5) & 7;
			const uint8_t rr = opcodes.r8(pc);
			pc++;
			util::stream_format(stream, "RES  %d,%s", b, reg_name(rr));
			break;
		}
		case 0x1b: case 0x3b: case 0x5b: case 0x7b: case 0x9b: case 0xbb: case 0xdb: case 0xfb:
		{
			const uint8_t b = (op >> 5) & 7;
			const uint8_t rr = opcodes.r8(pc);
			pc++;
			util::stream_format(stream, "SET  %d,%s", b, reg_name(rr));
			break;
		}
		case 0x04: case 0x14: case 0x24: case 0x34: case 0x44: case 0x54: case 0x64: case 0x74:
		case 0x84: case 0x94: case 0xa4: case 0xb4: case 0xc4: case 0xd4: case 0xe4: case 0xf4:
		case 0x0c: case 0x1c: case 0x2c: case 0x3c: case 0x4c: case 0x5c: case 0x6c: case 0x7c:
		case 0x8c: case 0x9c: case 0xac: case 0xbc: case 0xcc: case 0xdc: case 0xec: case 0xfc:
		{
			const int8_t e = ((int8_t)op) >> 3;
			util::stream_format(stream, "JRZ  $%d", e);
			flags = STEP_COND;
			break;
		}
		case 0x06: case 0x16: case 0x26: case 0x36: case 0x46: case 0x56: case 0x66: case 0x76:
		case 0x86: case 0x96: case 0xa6: case 0xb6: case 0xc6: case 0xd6: case 0xe6: case 0xf6:
		case 0x0e: case 0x1e: case 0x2e: case 0x3e: case 0x4e: case 0x5e: case 0x6e: case 0x7e:
		case 0x8e: case 0x9e: case 0xae: case 0xbe: case 0xce: case 0xde: case 0xee: case 0xfe:
		{
			const int8_t e = ((int8_t)op) >> 3;
			util::stream_format(stream, "JRC  $%d", e);
			flags = STEP_COND;
			break;
		}
		case 0x15:
			util::stream_format(stream, "INC  X"); // x
			break;
		case 0x35:
			util::stream_format(stream, "LD   A,X"); // a,x
			break;
		case 0x55:
			util::stream_format(stream, "INC  Y"); // y
			break;
		case 0x75:
			util::stream_format(stream, "LD   A,Y"); // a,y
			break;
		case 0x95:
			util::stream_format(stream, "INC  V"); // v
			break;
		case 0xb5:
			util::stream_format(stream, "LD   A,V"); // a,v
			break;
		case 0xd5:
			util::stream_format(stream, "INC  W"); // w
			break;
		case 0xf5:
			util::stream_format(stream, "LD   A,W"); // a,w
			break;
		case 0x0d:
		{
			const uint8_t rr = opcodes.r8(pc);
			pc++;
			const uint8_t nn = opcodes.r8(pc);
			pc++;
			if (nn == 0)
				util::stream_format(stream, "CLR  %s", reg_name(rr));
			else
				util::stream_format(stream, "LDI  %s,%Xh", reg_name(rr), nn);
			break;
		}
		case 0x1d:
			util::stream_format(stream, "DEC  X"); // x
			break;
		case 0x2d:
			util::stream_format(stream, "COM  A"); // a
			break;
		case 0x3d:
			util::stream_format(stream, "LD   X,A"); // x,a
			break;
		case 0x4d:
			util::stream_format(stream, "RETI");
			flags = STEP_OUT;
			break;
		case 0x5d:
			util::stream_format(stream, "DEC  Y"); // y
			break;
		case 0x6d:
			util::stream_format(stream, "STOP");
			break;
		case 0x7d:
			util::stream_format(stream, "LD   Y,A"); // y,a
			break;
		case 0x9d:
			util::stream_format(stream, "DEC  V"); // v
			break;
		case 0xad:
			util::stream_format(stream, "RLC"); // a
			break;
		case 0xbd:
			util::stream_format(stream, "LD   V,A"); // v,a
			break;
		case 0xcd:
			util::stream_format(stream, "RET");
			flags = STEP_OUT;
			break;
		case 0xdd:
			util::stream_format(stream, "DEC  W"); // w
			break;
		case 0xed:
			util::stream_format(stream, "WAIT");
			break;
		case 0xfd:
			util::stream_format(stream, "LD   W,A"); // w,a
			break;
		case 0x07:
			util::stream_format(stream, "LD   A,(X)"); // a,(x)
			break;
		case 0x17:
		{
			const uint8_t nn = opcodes.r8(pc);
			pc++;
			util::stream_format(stream, "LDI  A,%Xh", nn);
			break;
		}
		case 0x27:
			util::stream_format(stream, "CP   A,(X)"); // a,(x)
			break;
		case 0x37:
		{
			const uint8_t nn = opcodes.r8(pc);
			pc++;
			util::stream_format(stream, "CPI  A,%Xh", nn);
			break;
		}
		case 0x47:
			util::stream_format(stream, "ADD  A,(X)"); // a,(x)
			break;
		case 0x57:
		{
			const uint8_t nn = opcodes.r8(pc);
			pc++;
			util::stream_format(stream, "ADDI A,%Xh", nn);
			break;
		}
		case 0x67:
			util::stream_format(stream, "INC  (X)"); // (x)
			break;
		case 0x87:
			util::stream_format(stream, "LD   (X),A"); // (x),a
			break;
		case 0xa7:
			util::stream_format(stream, "AND  A,(X)"); // a,(x)
			break;
		case 0xb7:
		{
			const uint8_t nn = opcodes.r8(pc);
			pc++;
			util::stream_format(stream, "ANDI A,%Xh", nn);
			break;
		}
		case 0xc7:
			util::stream_format(stream, "SUB  A,(X)");
			break;
		case 0xd7:
		{
			const uint8_t nn = opcodes.r8(pc);
			pc++;
			util::stream_format(stream, "SUBI A,%Xh", nn);
			break;
		}
		case 0xe7:
			util::stream_format(stream, "DEC  (X)"); // (x)
			break;
		case 0x0f:
			util::stream_format(stream, "LD   A,(Y)"); // a,(y)
			break;
		case 0x1f:
		{
			const uint8_t rr = opcodes.r8(pc);
			pc++;
			util::stream_format(stream, "LD   A,%s", reg_name(rr));
			break;
		}
		case 0x2f:
			util::stream_format(stream, "CP   A,(Y)"); // a,(y)
			break;
		case 0x3f:
		{
			const uint8_t rr = opcodes.r8(pc);
			pc++;
			util::stream_format(stream, "CP   A,%s", reg_name(rr)); // rr
			break;
		}
		case 0x4f:
			util::stream_format(stream, "ADD  A,(Y)"); // a,(y)
			break;
		case 0x5f:
		{
			const uint8_t rr = opcodes.r8(pc);
			pc++;
			if (rr == 0xff)
				util::stream_format(stream, "SLA  A");
			else
				util::stream_format(stream, "ADD  A,%s", reg_name(rr));
			break;
		}
		case 0x6f:
			util::stream_format(stream, "INC  (Y)"); // (y)
			break;
		case 0x7f:
		{
			const uint8_t rr = opcodes.r8(pc);
			pc++;
			util::stream_format(stream, "INC  %s", reg_name(rr)); // rr
			break;
		}
		case 0x8f:
			util::stream_format(stream, "LD   (Y),A"); // (y),a
			break;
		case 0x9f:
		{
			const uint8_t rr = opcodes.r8(pc);
			pc++;
			util::stream_format(stream, "LD   %s,A", reg_name(rr)); // rr
			break;
		}
		case 0xaf:
			util::stream_format(stream, "AND  A,(Y)"); // a,(y)
			break;
		case 0xbf:
		{
			const uint8_t rr = opcodes.r8(pc);
			pc++;
			util::stream_format(stream, "AND  A,%s", reg_name(rr)); // rr
			break;
		}
		case 0xcf:
			util::stream_format(stream, "SUB  A,(Y)"); // a,(y)
			break;
		case 0xdf:
		{
			const uint8_t rr = opcodes.r8(pc);
			pc++;
			if (rr == 0xff)
				util::stream_format(stream, "CLR  A", rr);
			else
				util::stream_format(stream, "SUB  A,%s", reg_name(rr));
			break;
		}
		case 0xef:
			util::stream_format(stream, "DEC  (Y)"); // (y)
			break;
		case 0xff:
		{
			const uint8_t rr = opcodes.r8(pc);
			pc++;
			util::stream_format(stream, "DEC  %s", reg_name(rr)); // rr
			break;
		}
		default:
			util::stream_format(stream, "undefined");
			break;
	}

	return (pc - base_pc) | flags | SUPPORTED;
}
