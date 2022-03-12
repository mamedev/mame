// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"

#include "ns32000dasm.h"

char const *const cond_code[] = { "EQ", "NE", "CS", "CC", "HI", "LS", "GT", "LE", "FS", "FC", "LO", "HS", "LT", "GE", "R", "N" };
char const size_char[] = { 'B','W',' ','D' };

s32 ns32000_disassembler::displacement(offs_t pc, data_buffer const &opcodes, unsigned &bytes)
{
	u32 const byte0 = opcodes.r8(pc + bytes++);

	if (BIT(byte0, 7))
	{
		if (BIT(byte0, 6))
		{
			// double word displacement
			u32 const byte1 = opcodes.r8(pc + bytes++);
			u32 const byte2 = opcodes.r8(pc + bytes++);
			u32 const byte3 = opcodes.r8(pc + bytes++);

			return (s32((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3) << 2) >> 2;
		}
		else
		{
			// word displacement
			u8 const byte1 = opcodes.r8(pc + bytes++);

			return s16(((byte0 << 8) | byte1) << 2) >> 2;
		}
	}
	else
		// byte displacement
		return s8(byte0 << 1) >> 1;
}

std::string ns32000_disassembler::displacement_string(offs_t pc, data_buffer const &opcodes, unsigned &bytes, std::string const zero)
{
	s32 const d = displacement(pc, opcodes, bytes);

	if (d < 0)
		return util::string_format("-0x%X", -d);
	else if (d > 0)
		return util::string_format("0x%X", d);
	else
		return zero;
}

void ns32000_disassembler::decode(addr_mode *mode, offs_t pc, data_buffer const &opcodes, unsigned &bytes)
{
	char const scale_size[] = { 'B', 'W', 'D', 'Q' };
	std::string scale[2];

	// scaled mode
	for (unsigned i = 0; i < 2; i++)
	{
		if (mode[i].gen > 0x1b)
		{
			u8 const index = opcodes.r8(pc + bytes++);

			scale[i] = util::string_format("[R%d:%c]", index & 7, scale_size[mode[i].gen & 3]);
			mode[i].gen = index >> 3;
		}
	}

	// base mode
	for (unsigned i = 0; i < 2; i++)
	{
		switch (mode[i].gen)
		{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
			// register
			mode[i].mode = util::string_format("%c%d", mode[i].fpu ? 'F' : 'R', mode[i].gen);
			break;
		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			// register relative
			mode[i].mode = util::string_format("%s(R%d)", displacement_string(pc, opcodes, bytes), mode[i].gen & 7);
			break;
		case 0x10:
			// frame memory relative disp2(disp1(FP))
			{
				std::string const disp1 = displacement_string(pc, opcodes, bytes);
				std::string const disp2 = displacement_string(pc, opcodes, bytes);
				mode[i].mode = util::string_format("%s(%s(FP))", disp2, disp1);
			}
			break;
		case 0x11:
			// stack memory relative disp2(disp1(SP))
			{
				std::string const disp1 = displacement_string(pc, opcodes, bytes);
				std::string const disp2 = displacement_string(pc, opcodes, bytes);
				mode[i].mode = util::string_format("%s(%s(SP))", disp2, disp1);
			}
			break;
		case 0x12:
			// static memory relative disp2(disp1(SB))
			{
				std::string const disp1 = displacement_string(pc, opcodes, bytes);
				std::string const disp2 = displacement_string(pc, opcodes, bytes);
				mode[i].mode = util::string_format("%s(%s(SB))", disp2, disp1);
			}
			break;
		case 0x13:
			// reserved
			break;
		case 0x14:
			// immediate
			switch (mode[i].size)
			{
			case SIZE_B: mode[i].mode = util::string_format("0x%X", opcodes.r8(pc + bytes)); bytes += 1; break;
			case SIZE_W: mode[i].mode = util::string_format("0x%X", swapendian_int16(opcodes.r16(pc + bytes))); bytes += 2; break;
			case SIZE_D: mode[i].mode = util::string_format("0x%X", swapendian_int32(opcodes.r32(pc + bytes))); bytes += 4; break;
			case SIZE_Q: mode[i].mode = util::string_format("0x%X", swapendian_int64(opcodes.r64(pc + bytes))); bytes += 8; break;
			}
			break;
		case 0x15:
			// absolute @disp
			mode[i].mode = util::string_format("@0x%X", displacement(pc, opcodes, bytes));
			break;
		case 0x16:
			// external EXT(disp1) + disp2
			{
				std::string const disp1 = displacement_string(pc, opcodes, bytes, "0");
				s32 const disp2 = displacement(pc, opcodes, bytes);
				if (disp2 < 0)
					mode[i].mode = util::string_format("EXT(%s) - 0x%X", disp1, -disp2);
				else if (disp2 > 0)
					mode[i].mode = util::string_format("EXT(%s) + 0x%X", disp1, disp2);
				else
					mode[i].mode = util::string_format("EXT(%s)", disp1);
			}
			break;
		case 0x17:
			// top of stack TOS
			mode[i].mode = "TOS";
			break;
		case 0x18:
			// frame memory disp(FP)
			mode[i].mode = util::string_format("%s(FP)", displacement_string(pc, opcodes, bytes));
			break;
		case 0x19:
			// stack memory disp(SP)
			mode[i].mode = util::string_format("%s(SP)", displacement_string(pc, opcodes, bytes));
			break;
		case 0x1a:
			// static memory disp(SB)
			mode[i].mode = util::string_format("%s(SB)", displacement_string(pc, opcodes, bytes));
			break;
		case 0x1b:
			// program memory *+disp
			mode[i].mode = util::string_format("0x%X", pc + displacement(pc, opcodes, bytes));
			break;
		}

		if (!scale[i].empty())
			mode[i].mode.append(scale[i]);
	}
}

std::string ns32000_disassembler::reglist(u8 imm)
{
	std::string result;

	for (unsigned i = 0; i < 8; i++)
		if (BIT(imm, i))
		{
			if (result.empty())
				result.append(util::string_format("R%d", i));
			else
				result.append(util::string_format(",R%d", i));
		}

	return result;
}

std::string ns32000_disassembler::config(u8 imm)
{
	static char const *const cfg[] = { "I", "F", "M", "C", "FF", "FM", "FC", "P" };
	std::string result;

	for (unsigned i = 0; i < 8; i++)
	{
		if (BIT(imm, i))
		{
			if (!result.empty())
				result.append(",");

			result.append(cfg[i]);
		}
	}

	return result;
}

offs_t ns32000_disassembler::disassemble(std::ostream &stream, offs_t pc, data_buffer const &opcodes, data_buffer const &params)
{
	uint32_t flags = SUPPORTED;
	unsigned bytes = 0;

	u8 const opbyte = opcodes.r8(pc + bytes++);

	if ((opbyte & 15) == 10)
	{
		// format 0: cccc 1010
		util::stream_format(stream, "B%-2s     0x%X", cond_code[BIT(opbyte, 4, 4)], pc + displacement(pc, opcodes, bytes));
	}
	else if ((opbyte & 15) == 2)
	{
		// format 1: oooo 0010

		// reglist immediate for save/restore/enter/exit
		u8 const imm = (BIT(opbyte, 4, 4) >= 6 && BIT(opbyte, 4, 4) <= 9) ?
			opcodes.r8(pc + bytes++) : 0;

		switch (BIT(opbyte, 4, 4))
		{
		case 0x0: util::stream_format(stream, "BSR     0x%X", pc + displacement(pc, opcodes, bytes)); flags |= STEP_OVER; break;
		case 0x1: util::stream_format(stream, "RET     %s", displacement_string(pc, opcodes, bytes, "0")); flags |= STEP_OUT; break;
		case 0x2: util::stream_format(stream, "CXP     %s", displacement_string(pc, opcodes, bytes, "0")); flags |= STEP_OVER; break;
		case 0x3: util::stream_format(stream, "RXP     %s", displacement_string(pc, opcodes, bytes, "0")); flags |= STEP_OUT; break;
		case 0x4: util::stream_format(stream, "RETT    %s", displacement_string(pc, opcodes, bytes, "0")); flags |= STEP_OUT; break;
		case 0x5: util::stream_format(stream, "RETI"); flags |= STEP_OUT; break;
		case 0x6: util::stream_format(stream, "SAVE    [%s]", reglist(imm)); break;
		case 0x7: util::stream_format(stream, "RESTORE [%s]", reglist(bitswap(imm, 0, 1, 2, 3, 4, 5, 6, 7))); break;
		case 0x8: util::stream_format(stream, "ENTER   [%s], %s", reglist(imm), displacement_string(pc, opcodes, bytes, "0")); break;
		case 0x9: util::stream_format(stream, "EXIT    [%s]", reglist(bitswap(imm, 0, 1, 2, 3, 4, 5, 6, 7))); break;
		case 0xa: util::stream_format(stream, "NOP"); break;
		case 0xb: util::stream_format(stream, "WAIT"); break;
		case 0xc: util::stream_format(stream, "DIA"); break;
		case 0xd: util::stream_format(stream, "FLAG"); break;
		case 0xe: util::stream_format(stream, "SVC"); break;
		case 0xf: util::stream_format(stream, "BPT"); break;
		}
	}
	else if ((opbyte & 15) == 12 || (opbyte & 15) == 13 || (opbyte & 15) == 15)
	{
		// format 2: gggg gsss sooo 11ii
		u16 const opword = (u16(opcodes.r8(pc + bytes++)) << 8) | opbyte;

		// HACK: use reserved mode for second unused type
		addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(0x13) };

		unsigned const quick = BIT(opword, 7, 4);
		size_code const size = size_code(opbyte & 3);

		char const *const procreg[] = { "UPSR", "DCR", "BPC", "DSR", "CAR", "", "", "", "FP", "SP", "SB", "USP", "CFG", "PSR", "INTBASE", "MOD" };

		mode[0].size_i(size);
		decode(mode, pc, opcodes, bytes);

		switch (BIT(opbyte, 4, 3))
		{
		case 0: util::stream_format(stream, "ADDQ%c   %d, %s", size_char[size], s32(quick << 28) >> 28, mode[0].mode); break;
		case 1: util::stream_format(stream, "CMPQ%c   %d, %s", size_char[size], s32(quick << 28) >> 28, mode[0].mode); break;
		case 2: util::stream_format(stream, "SPR%c    %s, %s", size_char[size], procreg[quick], mode[0].mode); break;
		case 3: util::stream_format(stream, "S%s%c    %s", cond_code[quick], size_char[size], mode[0].mode); break;
		case 4: util::stream_format(stream, "ACB%c    %d, %s, 0x%X", size_char[size], s32(quick << 28) >> 28, mode[0].mode, pc + displacement(pc, opcodes, bytes)); break;
		case 5: util::stream_format(stream, "MOVQ%c   %d, %s", size_char[size], s32(quick << 28) >> 28, mode[0].mode); break;
		case 6: util::stream_format(stream, "LPR%c    %s, %s", size_char[size], procreg[quick], mode[0].mode); break;
		case 7:
			// format 3: gggg gooo o111 11ii
			switch (BIT(opword, 7, 4))
			{
			case 0x0: util::stream_format(stream, "CXPD    %s", mode[0].mode); flags |= STEP_OVER; break;
			case 0x2: util::stream_format(stream, "BICPSR%c %s", size_char[size], mode[0].mode); break;
			case 0x4: util::stream_format(stream, "JUMP    %s", mode[0].mode); break;
			case 0x6: util::stream_format(stream, "BISPSR%c %s", size_char[size], mode[0].mode); break;
			case 0xa: util::stream_format(stream, "ADJSP%c  %s", size_char[size], mode[0].mode); break;
			case 0xc: util::stream_format(stream, "JSR     %s", mode[0].mode); flags |= STEP_OVER; break;
			case 0xe: util::stream_format(stream, "CASE%c   %s", size_char[size], mode[0].mode); break;
			default: bytes = 1; break;
			}
			break;
		}
	}
	else if ((opbyte & 3) != 2)
	{
		// format 4: xxxx xyyy yyoo ooii
		u16 const opword = (u16(opcodes.r8(pc + bytes++)) << 8) | opbyte;

		addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(BIT(opword, 6, 5)) };
		size_code const size = size_code(opbyte & 3);

		mode[0].size_i(size);
		mode[1].size_i(size);
		decode(mode, pc, opcodes, bytes);

		switch (BIT(opbyte, 2, 4))
		{
		case 0x0: util::stream_format(stream, "ADD%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
		case 0x1: util::stream_format(stream, "CMP%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
		case 0x2: util::stream_format(stream, "BIC%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
		case 0x4: util::stream_format(stream, "ADDC%c   %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
		case 0x5: util::stream_format(stream, "MOV%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
		case 0x6: util::stream_format(stream, "OR%c     %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
		case 0x8: util::stream_format(stream, "SUB%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
		case 0x9: util::stream_format(stream, "ADDR    %s, %s", mode[0].mode, mode[1].mode); break;
		case 0xa: util::stream_format(stream, "AND%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
		case 0xc: util::stream_format(stream, "SUBC%c   %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
		case 0xd: util::stream_format(stream, "TBIT%c   %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
		case 0xe: util::stream_format(stream, "XOR%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
		default: bytes = 1; break;
		}
	}
	else switch (opbyte)
	{
	case 0x0e:
		// format 5: 0000 0sss s0oo ooii 0000 1110
		{
			u16 const opword = opcodes.r16(pc + bytes); bytes += 2;

			char const *const options[] = { "", "B", "W", "W,B", "", "", "U", "U,B" };

			size_code const size = size_code(opword & 3);

			switch (BIT(opword, 2, 4))
			{
			case 0:
				if (BIT(opword, 7))
					util::stream_format(stream, "MOVST%c  %s", size_char[size], options[BIT(opword, 8, 3)]);
				else
					util::stream_format(stream, "MOVS%c   %s", size_char[size], options[BIT(opword, 8, 3)]);
				break;
			case 1:
				if (BIT(opword, 7))
					util::stream_format(stream, "CMPST%c  %s", size_char[size], options[BIT(opword, 8, 3)]);
				else
					util::stream_format(stream, "CMPS%c   %s", size_char[size], options[BIT(opword, 8, 3)]);
				break;
			case 2: util::stream_format(stream, "SETCFG  [%s]", config(BIT(opword, 7, 8))); break;
			case 3:
				if (BIT(opword, 7))
					util::stream_format(stream, "SKPST%c  %s", size_char[size], options[BIT(opword, 8, 3)]);
				else
					util::stream_format(stream, "SKPS%c   %s", size_char[size], options[BIT(opword, 8, 3)]);
				break;
			default: bytes = 1; break;
			}
		}
		break;
	case 0x4e:
		// format 6: xxxx xyyy yyoo ooii 0100 1110
		{
			u16 const opword = opcodes.r16(pc + bytes); bytes += 2;

			addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(BIT(opword, 6, 5)) };
			size_code const size = size_code(opword & 3);

			switch (BIT(opword, 2, 4))
			{
			case 0: case 1: case 5:
				mode[0].size_i(SIZE_B);
				break;
			default:
				mode[0].size_i(size);
				break;
			}
			mode[1].size_i(size);
			decode(mode, pc, opcodes, bytes);

			switch (BIT(opword, 2, 4))
			{
			case 0x0: util::stream_format(stream, "ROT%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0x1: util::stream_format(stream, "ASH%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0x2: util::stream_format(stream, "CBIT%c   %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0x3: util::stream_format(stream, "CBITI%c  %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0x5: util::stream_format(stream, "LSH%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0x6: util::stream_format(stream, "SBIT%c   %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0x7: util::stream_format(stream, "SBITI%c  %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0x8: util::stream_format(stream, "NEG%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0x9: util::stream_format(stream, "NOT%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0xb: util::stream_format(stream, "SUBP%c   %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0xc: util::stream_format(stream, "ABS%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0xd: util::stream_format(stream, "COM%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0xe: util::stream_format(stream, "IBIT%c   %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0xf: util::stream_format(stream, "ADDP%c   %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			default: bytes = 1; break;
			}
		}
		break;
	case 0xce:
		// format 7: xxxx xyyy yyoo ooii 1100 1110
		{
			u16 const opword = opcodes.r16(pc + bytes); bytes += 2;

			addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(BIT(opword, 6, 5)) };
			size_code const size = size_code(opword & 3);

			mode[0].size_i(size);
			mode[1].size_i(size);
			decode(mode, pc, opcodes, bytes);

			u8 const imm = (BIT(opword, 2, 4) == 2 || BIT(opword, 2, 4) == 3) ? opcodes.r8(pc + bytes++) : 0;

			switch (BIT(opword, 2, 4))
			{
			case 0x0: util::stream_format(stream, "MOVM%c   %s, %s, %d", size_char[size], mode[0].mode, mode[1].mode, displacement(pc, opcodes, bytes) / (size + 1) + 1); break;
			case 0x1: util::stream_format(stream, "CMPM%c   %s, %s, %d", size_char[size], mode[0].mode, mode[1].mode, displacement(pc, opcodes, bytes) / (size + 1) + 1); break;
			case 0x2: util::stream_format(stream, "INSS%c   %s, %s, %d, %d", size_char[size], mode[0].mode, mode[1].mode, imm >> 5, (imm & 31) + 1); break;
			case 0x3: util::stream_format(stream, "EXTS%c   %s, %s, %d, %d", size_char[size], mode[0].mode, mode[1].mode, imm >> 5, (imm & 31) + 1); break;
			case 0x4: util::stream_format(stream, "MOVXBW  %s, %s", mode[0].mode, mode[1].mode); break;
			case 0x5: util::stream_format(stream, "MOVZBW  %s, %s", mode[0].mode, mode[1].mode); break;
			case 0x6: util::stream_format(stream, "MOVZ%cD  %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0x7: util::stream_format(stream, "MOVX%cD  %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0x8: util::stream_format(stream, "MUL%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0x9: util::stream_format(stream, "MEI%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0xb: util::stream_format(stream, "DEI%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0xc: util::stream_format(stream, "QUO%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0xd: util::stream_format(stream, "REM%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0xe: util::stream_format(stream, "MOD%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 0xf: util::stream_format(stream, "DIV%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			default: bytes = 1; break;
			}
		}
		break;
	case 0x2e:
	case 0x6e:
	case 0xae:
	case 0xee:
		// format 8: xxxx xyyy yyrr roii oo10 1110
		{
			u16 const opword = opcodes.r16(pc + bytes); bytes += 2;

			addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(BIT(opword, 6, 5)) };
			unsigned const reg = BIT(opword, 3, 3);
			size_code const size = size_code(opword & 3);

			mode[0].size_i(size);
			mode[1].size_i(size);
			decode(mode, pc, opcodes, bytes);

			switch ((opword & 4) | BIT(opbyte, 6, 2))
			{
			case 0: util::stream_format(stream, "EXT%c    R%d, %s, %s, %d", size_char[size], reg, mode[0].mode, mode[1].mode, displacement(pc, opcodes, bytes)); break;
			case 1: util::stream_format(stream, "CVTP    R%d, %s, %s", reg, mode[0].mode, mode[1].mode); break;
			case 2: util::stream_format(stream, "INS%c    R%d, %s, %s, %d", size_char[size], reg, mode[0].mode, mode[1].mode, displacement(pc, opcodes, bytes)); break;
			case 3: util::stream_format(stream, "CHECK%c  R%d, %s, %s", size_char[size], reg, mode[0].mode, mode[1].mode); break;
			case 4: util::stream_format(stream, "INDEX%c  R%d, %s, %s", size_char[size], reg, mode[0].mode, mode[1].mode); break;
			case 5: util::stream_format(stream, "FFS%c    %s, %s", size_char[size], mode[0].mode, mode[1].mode); break;
			case 6:
				if (reg == 1)
					util::stream_format(stream, "MOVSU%c  %s, %s", size_char[size], mode[0].mode, mode[1].mode);
				else if (reg == 3)
					util::stream_format(stream, "MOVUS%c  %s, %s", size_char[size], mode[0].mode, mode[1].mode);
				else
					bytes = 1;
				break;
			default: bytes = 1; break;
			}
		}
		break;
	case 0x3e:
		// format 9: xxxx xyyy yyoo ofii 0011 1110
		{
			u16 const opword = opcodes.r16(pc + bytes); bytes += 2;

			addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(BIT(opword, 6, 5)) };
			size_code const size_f = BIT(opword, 0) ? SIZE_D : SIZE_Q;
			size_code const size = size_code(opword & 3);

			switch (BIT(opword, 3, 3))
			{
			case 0:
				// MOVif src,dst
				//       gen,gen
				//       read.i,write.f
				mode[0].size_i(size);
				mode[1].size_f(size_f);
				decode(mode, pc, opcodes, bytes);
				util::stream_format(stream, "MOV%c%c   %s, %s", size_char[size], BIT(opword, 0) ? 'F' : 'L', mode[0].mode, mode[1].mode);
				break;
			case 1:
				// LFSR src
				//      gen
				//      read.D
				mode[0].size_i(size);
				decode(mode, pc, opcodes, bytes);
				util::stream_format(stream, "LFSR    %s", mode[0].mode);
				break;
			case 2:
				// MOVLF src,dst
				//       gen,gen
				//       read.L,write.F
				mode[0].size_f(SIZE_Q);
				mode[1].size_f(size_f);
				decode(mode, pc, opcodes, bytes);
				util::stream_format(stream, "MOVLF   %s, %s", mode[0].mode, mode[1].mode);
				break;
			case 3:
				// MOVFL src,dst
				//       gen,gen
				//       read.F,write.L
				mode[0].size_f(SIZE_D);
				mode[1].size_f(size_f);
				decode(mode, pc, opcodes, bytes);
				util::stream_format(stream, "MOVFL   %s, %s", mode[0].mode, mode[1].mode);
				break;
			case 4:
				// ROUNDfi src,dst
				//         gen,gen
				//         read.f,write.i
				mode[0].size_f(size_f);
				mode[1].size_i(size);
				decode(mode, pc, opcodes, bytes);
				util::stream_format(stream, "ROUND%c%c %s, %s", BIT(opword, 0) ? 'F' : 'L', size_char[size], mode[0].mode, mode[1].mode);
				break;
			case 5:
				// TRUNCfi src,dst
				//         gen,gen
				//         read.f,write.i
				mode[0].size_f(size_f);
				mode[1].size_i(size);
				decode(mode, pc, opcodes, bytes);
				util::stream_format(stream, "TRUNC%c%c %s, %s", BIT(opword, 0) ? 'F' : 'L', size_char[size], mode[0].mode, mode[1].mode);
				break;
			case 6:
				// SFSR dst
				//      gen
				//      write.D
				mode[0].size_i(size);
				decode(mode, pc, opcodes, bytes);
				util::stream_format(stream, "SFSR    %s", mode[0].mode);
				break;
			case 7:
				// FLOORfi src,dst
				//         gen,gen
				//         read.f,write.i
				mode[0].size_f(size_f);
				mode[1].size_i(size);
				decode(mode, pc, opcodes, bytes);
				util::stream_format(stream, "FLOOR%c%c %s, %s", BIT(opword, 0) ? 'F' : 'L', size_char[size], mode[0].mode, mode[1].mode);
				break;
			}
		}
		break;
	case 0x7e: // format 10
		bytes = 1;
		break;
	case 0xbe:
		// format 11: xxxx xyyy yyoo oo0f 1011 1110
		{
			u16 const opword = opcodes.r16(pc + bytes); bytes += 2;

			addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(BIT(opword, 6, 5)) };
			size_code const size_f = BIT(opword, 0) ? SIZE_D : SIZE_Q;

			mode[0].size_f(size_f);
			mode[1].size_f(size_f);
			decode(mode, pc, opcodes, bytes);

			switch (BIT(opword, 2, 4))
			{
			case 0x0: util::stream_format(stream, "ADD%c    %s, %s", BIT(opword, 0) ? 'F' : 'L', mode[0].mode, mode[1].mode); break;
			case 0x1: util::stream_format(stream, "MOV%c    %s, %s", BIT(opword, 0) ? 'F' : 'L', mode[0].mode, mode[1].mode); break;
			case 0x2: util::stream_format(stream, "CMP%c    %s, %s", BIT(opword, 0) ? 'F' : 'L', mode[0].mode, mode[1].mode); break;
			case 0x4: util::stream_format(stream, "SUB%c    %s, %s", BIT(opword, 0) ? 'F' : 'L', mode[0].mode, mode[1].mode); break;
			case 0x5: util::stream_format(stream, "NEG%c    %s, %s", BIT(opword, 0) ? 'F' : 'L', mode[0].mode, mode[1].mode); break;
			case 0x8: util::stream_format(stream, "DIV%c    %s, %s", BIT(opword, 0) ? 'F' : 'L', mode[0].mode, mode[1].mode); break;
			case 0xc: util::stream_format(stream, "MUL%c    %s, %s", BIT(opword, 0) ? 'F' : 'L', mode[0].mode, mode[1].mode); break;
			case 0xd: util::stream_format(stream, "ABS%c    %s, %s", BIT(opword, 0) ? 'F' : 'L', mode[0].mode, mode[1].mode); break;
			default: bytes = 1; break;
			}
		}
		break;
	case 0xfe:
		// format 12: xxxx xyyy yyoo oo0f 1111 1110
		{
			u16 const opword = opcodes.r16(pc + bytes); bytes += 2;

			addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(BIT(opword, 6, 5)) };
			size_code const size_f = BIT(opword, 0) ? SIZE_D : SIZE_Q;

			mode[0].size_f(size_f);
			mode[1].size_f(size_f);
			decode(mode, pc, opcodes, bytes);

			switch (BIT(opword, 2, 4))
			{
			case 0x2: util::stream_format(stream, "POLY%c   %s, %s", BIT(opword, 0) ? 'F' : 'L', mode[0].mode, mode[1].mode); break;
			case 0x3: util::stream_format(stream, "DOT%c    %s, %s", BIT(opword, 0) ? 'F' : 'L', mode[0].mode, mode[1].mode); break;
			case 0x4: util::stream_format(stream, "SCALB%c  %s, %s", BIT(opword, 0) ? 'F' : 'L', mode[0].mode, mode[1].mode); break;
			case 0x5: util::stream_format(stream, "LOGB%c   %s, %s", BIT(opword, 0) ? 'F' : 'L', mode[0].mode, mode[1].mode); break;
			default: bytes = 1; break;
			}
		}
		break;
	case 0x9e: // format 13
		bytes = 1;
		break;
	case 0x1e:
		// format 14: xxxx xsss s0oo ooii 0001 1110
		{
			// TODO: different mmu registers for 32332 and 32532
			char const *const mmureg[] = { "BPR0", "BPR1", "", "", "PF0", "PF1", "", "", "SC", "", "MSR", "BCNT", "PTB0", "PTB1", "", "EIA" };

			u16 const opword = opcodes.r16(pc + bytes); bytes += 2;

			addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(0x13) };

			unsigned const quick = BIT(opword, 7, 4);
			size_code const size = size_code(opword & 3);

			mode[0].size_i(size);
			decode(mode, pc, opcodes, bytes);

			switch (BIT(opword, 2, 4))
			{
			case 0: util::stream_format(stream, "RDVAL   %s", mode[0].mode); break;
			case 1: util::stream_format(stream, "WRVAL   %s", mode[0].mode); break;
			case 2: util::stream_format(stream, "LMR     %s, %s", mmureg[quick], mode[0].mode); break;
			case 3: util::stream_format(stream, "SMR     %s, %s", mmureg[quick], mode[0].mode); break;
			default: bytes = 1; break;
			}
		}
		break;
	case 0x16: // format 15.0
	case 0x36: // format 15.1
	case 0xb6: // format 15.5
	case 0x5e: // format 16
	case 0xde: // format 17
	case 0x8e: // format 18
	case 0x06:
	case 0x26:
	case 0x46:
	case 0x66:
	case 0x86:
	case 0xa6:
	case 0xc6:
	case 0xe6:
		// format 19
	default:
		bytes = 1;
		break;
	}

	return bytes | flags;
}
