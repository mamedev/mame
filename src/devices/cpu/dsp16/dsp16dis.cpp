// license:BSD-3-Clause
// copyright-holders:Andrew Gardner, Vas Crabb
#include "emu.h"
#include "dsp16dis.h"

std::string dsp16a_disassembler::disasmF1Field(u8 F1, u8 D, u8 S)
{
	switch (F1)
	{
	case 0x00: return util::string_format("a%u = p, p = x*y", D);
	case 0x01: return util::string_format("a%u = a%u + p, p = x*y", D, S);
	case 0x02: return util::string_format("p = x*y");
	case 0x03: return util::string_format("a%u = a%u - p, p = x*y", D, S);
	case 0x04: return util::string_format("a%u = p", D);
	case 0x05: return util::string_format("a%u = a%u + p", D, S);
	case 0x06: return util::string_format("nop");
	case 0x07: return util::string_format("a%u = a%u - p", D, S);
	case 0x08: return util::string_format("a%u = a%u | y", D, S);
	case 0x09: return util::string_format("a%u = a%u ^ y", D, S);
	case 0x0a: return util::string_format("a%u & y", S);
	case 0x0b: return util::string_format("a%u - y", S);
	case 0x0c: return util::string_format("a%u = y", D);
	case 0x0d: return util::string_format("a%u = a%u + y", D, S);
	case 0x0e: return util::string_format("a%u = a%u & y", D, S);
	case 0x0f: return util::string_format("a%u = a%u - y", D, S);
	}
	return "UNKNOWN";
}

std::string dsp16a_disassembler::disasmYField(u8 Y)
{
	switch (Y & 0x03U)
	{
	case 0x00: return util::string_format("*r%u", Y >> 2);
	case 0x01: return util::string_format("*r%u++", Y >> 2);
	case 0x02: return util::string_format("*r%u--", Y >> 2);
	case 0x03: return util::string_format("*r%u++j", Y >> 2);
	}
	return "UNKNOWN";
}

std::string dsp16a_disassembler::disasmZField(u8 Z)
{
	switch (Z & 0x03U)
	{
	case 0x00: return util::string_format("*r%uzp", Z >> 2);
	case 0x01: return util::string_format("*r%upz", Z >> 2);
	case 0x02: return util::string_format("*r%um2", Z >> 2);
	case 0x03: return util::string_format("*r%ujk", Z >> 2);
	}
	return "UNKNOWN";
}

std::string dsp16a_disassembler::disasmF2Field(u8 F2, u8 D, u8 S)
{
	switch (F2)
	{
	case 0x00: return util::string_format("a%u = a%u >> 1", D, S);
	case 0x01: return util::string_format("a%u = a%u << 1", D, S);
	case 0x02: return util::string_format("a%u = a%u >> 4", D, S);
	case 0x03: return util::string_format("a%u = a%u << 4", D, S);
	case 0x04: return util::string_format("a%u = a%u >> 8", D, S);
	case 0x05: return util::string_format("a%u = a%u << 8", D, S);
	case 0x06: return util::string_format("a%u = a%u >> 16", D, S);
	case 0x07: return util::string_format("a%u = a%u << 16", D, S);
	case 0x08: return util::string_format("a%u = p", D);
	case 0x09: return util::string_format("a%uh = a%uh + 1", D, S);
	case 0x0a: return util::string_format("RESERVED");
	case 0x0b: return util::string_format("a%u = rnd(a%u)", D, S);
	case 0x0c: return util::string_format("a%u = y", D);
	case 0x0d: return util::string_format("a%u = a%u + 1", D, S);
	case 0x0e: return util::string_format("a%u = a%u", D, S);
	case 0x0f: return util::string_format("a%u = -a%u", D, S);
	}
	return "UNKNOWN";
}

std::string dsp16a_disassembler::disasmCONField(u8 CON)
{
	switch (CON)
	{
	case 0x00: return "mi";
	case 0x01: return "pl";
	case 0x02: return "eq";
	case 0x03: return "ne";
	case 0x04: return "lvs";
	case 0x05: return "lvc";
	case 0x06: return "mvs";
	case 0x07: return "mvc";
	case 0x08: return "heads";
	case 0x09: return "tails";
	case 0x0a: return "c0ge";
	case 0x0b: return "c0lt";
	case 0x0c: return "c1ge";
	case 0x0d: return "c1lt";
	case 0x0e: return "true";
	case 0x0f: return "false";
	case 0x10: return "gt";
	case 0x11: return "le";

	default: return "RESERVED";
	}
}

std::string dsp16a_disassembler::disasmBField(u8 B, uint32_t &dasmflags)
{
	switch (B)
	{
	case 0x00: dasmflags |= STEP_OUT;  return "return"; // TODO: can be predicated
	case 0x01: dasmflags |= STEP_OUT;  return "ireturn";
	case 0x02:                         return "goto pt";
	case 0x03: dasmflags |= STEP_OVER; return "call pt";
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07: return "RESERVED";
	}
	return "UNKNOWN";
}

std::string dsp16a_disassembler::disasmRImmediateField(u8 R)
{
	switch (R)
	{
	case 0x00: return "j";
	case 0x01: return "k";
	case 0x02: return "rb";
	case 0x03: return "re";
	case 0x04: return "r0";
	case 0x05: return "r1";
	case 0x06: return "r2";
	case 0x07: return "r3";
	}
	return "UNKNOWN";
}

std::string dsp16a_disassembler::disasmRField(u8 R)
{
	switch (R)
	{
	case 0x00: return "r0";
	case 0x01: return "r1";
	case 0x02: return "r2";
	case 0x03: return "r3";
	case 0x04: return "j";
	case 0x05: return "k";
	case 0x06: return "rb";
	case 0x07: return "re";
	case 0x08: return "pt";
	case 0x09: return "pr";
	case 0x0a: return "pi";
	case 0x0b: return "i";
	case 0x10: return "x";
	case 0x11: return "y";
	case 0x12: return "yl";
	case 0x13: return "auc";
	case 0x14: return "psw";
	case 0x15: return "c0";
	case 0x16: return "c1";
	case 0x17: return "c2";
	case 0x18: return "sioc";
	case 0x19: return "srta";
	case 0x1a: return "sdx";
	case 0x1b: return "tdms";
	case 0x1c: return "pioc";
	case 0x1d: return "pdx0";
	case 0x1e: return "pdx1";

	default: return "RESERVED";
	}
}

std::string dsp16a_disassembler::disasmIField(u8 I)
{
	switch (I)
	{
	case 0x00: return "r0/j";
	case 0x01: return "r1/k";
	case 0x02: return "r2/rb";
	case 0x03: return "r3/re";

	default: return "UNKNOWN";
	}
}

bool dsp16a_disassembler::disasmSIField(u8 SI)
{
	switch (SI)
	{
	case 0x00: return false;    // Not a software interrupt
	case 0x01: return true;     // Software Interrupt
	}
	return false;
}


u32 dsp16a_disassembler::interface_flags() const
{
	return PAGED;
}

u32 dsp16a_disassembler::page_address_bits() const
{
	return 12U;
}

u32 dsp16a_disassembler::opcode_alignment() const
{
	return 1U;
}

offs_t dsp16a_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u8 opSize = 1;
	uint32_t dasmflags = 0;
	uint16_t op  = opcodes.r16(pc);
	uint16_t op2 = opcodes.r16(pc+1);

	// TODO: Test for previous "if CON" instruction and tab the next instruction in?

	const u8 opcode = (op >> 11) & 0x1f;
	switch(opcode)
	{
	// Format 1: Multiply/ALU Read/Write Group
	case 0x06: // F1, Y
		{
			const u8 Y = (op & 0x000f);
			const u8 S = (op & 0x0200) >> 9;
			const u8 D = (op & 0x0400) >> 10;
			const u8 F1 = (op & 0x01e0) >> 5;
			std::string yString = disasmYField(Y);
			std::string fString = disasmF1Field(F1, D, S);
			util::stream_format(stream, "%s, %s", fString, yString);
		}
		break;
	case 0x04: // F1, Y=a1[l]
	case 0x1c: // F1, Y=a0[l]
		{
			const u8 Y = (op & 0x000f);
			const u8 X = (op & 0x0010) >> 4;
			const u8 S = (op & 0x0200) >> 9;
			const u8 D = (op & 0x0400) >> 10;
			const u8 F1 = (op & 0x01e0) >> 5;
			std::string yString = disasmYField(Y);
			std::string fString = disasmF1Field(F1, D, S);
			std::string aString = (opcode == 0x1c) ? "a0" : "a1";
			std::string xString = X ? "" : "l";
			util::stream_format(stream, "%s, %s = %s%s", fString, yString, aString, xString);
		}
		break;
	case 0x16: // F1, x = Y
		{
			const u8 Y = (op & 0x000f);
			const u8 S = (op & 0x0200) >> 9;
			const u8 D = (op & 0x0400) >> 10;
			const u8 F1 = (op & 0x01e0) >> 5;
			std::string yString = disasmYField(Y);
			std::string fString = disasmF1Field(F1, D, S);
			util::stream_format(stream, "%s, x = %s", fString, yString);
		}
		break;
	case 0x17: // F1, y[l] = Y
		{
			const u8 Y = (op & 0x000f);
			const u8 X = (op & 0x0010) >> 4;
			const u8 S = (op & 0x0200) >> 9;
			const u8 D = (op & 0x0400) >> 10;
			const u8 F1 = (op & 0x01e0) >> 5;
			std::string yString = disasmYField(Y);
			std::string fString = disasmF1Field(F1, D, S);
			std::string xString = X ? "y" : "yl";
			util::stream_format(stream, "%s, %s = %s", fString, xString, yString);
		}
		break;
	case 0x1f: // F1, y = Y, x = *pt++[i]
		{
			const u8 Y = (op & 0x000f);
			const u8 X = (op & 0x0010) >> 4;
			const u8 S = (op & 0x0200) >> 9;
			const u8 D = (op & 0x0400) >> 10;
			const u8 F1 = (op & 0x01e0) >> 5;
			std::string yString = disasmYField(Y);
			std::string fString = disasmF1Field(F1, D, S);
			std::string xString = X ? "*pt++i" : "*pt++";
			util::stream_format(stream, "%s, y = %s, x = %s", fString, yString, xString);
		}
		break;
	case 0x19: // F1, y = a0, x = *pt++[i]
	case 0x1b: // F1, y = a0, x = *pt++[i]
		{
			const u8 Y = (op & 0x000f);
			const u8 X = (op & 0x0010) >> 4;
			const u8 S = (op & 0x0200) >> 9;
			const u8 D = (op & 0x0400) >> 10;
			const u8 F1 = (op & 0x01e0) >> 5;
			std::string fString = disasmF1Field(F1, D, S);
			std::string xString = X ? "*pt++i" : "*pt++";
			std::string aString = (opcode == 0x19) ? "a0" : "a1";

			if (Y != 0x00)
				util::stream_format(stream, "UNKNOWN");
			else
				util::stream_format(stream, "%s, y = %s, x = %s", fString, aString, xString);
		}
		break;
	case 0x14: // F1, Y = y[l]
		{
			const u8 Y = (op & 0x000f);
			const u8 X = (op & 0x0010) >> 4;
			const u8 S = (op & 0x0200) >> 9;
			const u8 D = (op & 0x0400) >> 10;
			const u8 F1 = (op & 0x01e0) >> 5;
			std::string yString = disasmYField(Y);
			std::string xString = X ? "y" : "yl";
			std::string fString = disasmF1Field(F1, D, S);
			util::stream_format(stream, "%s, %s = %s", fString, yString, xString);
		}
		break;

	// Format 1a: Multiply/ALU Read/Write Group (major typo in docs on p3-51)
	case 0x07: // F1, aT[l] = Y
		{
			const u8 Y = (op & 0x000f);
			const u8 S = (op & 0x0200) >> 9;
			const u8 aT = (op & 0x0400) >> 10;
			const u8 F1 = (op & 0x01e0) >> 5;
			std::string yString = disasmYField(Y);
			std::string atString = aT ? "a0" : "a1";
			std::string fString = disasmF1Field(F1, aT, S);
			util::stream_format(stream, "%s, %s = %s", fString, atString, yString);
		}
		break;

	// Format 2: Multiply/ALU Read/Write Group
	case 0x15: // F1, Z : y[l]
		{
			const u8 Z = (op & 0x000f);
			const u8 X = (op & 0x0010) >> 4;
			const u8 S = (op & 0x0200) >> 9;
			const u8 D = (op & 0x0400) >> 10;
			const u8 F1 = (op & 0x01e0) >> 5;
			std::string zString = disasmZField(Z);
			std::string xString = X ? "y" : "yl";
			std::string fString = disasmF1Field(F1, D, S);
			util::stream_format(stream, "%s, %s : %s", fString, zString, xString);
			break;
		}
	case 0x1d: // F1, Z : y, x=*pt++[i]
		{
			const u8 Z = (op & 0x000f);
			const u8 X = (op & 0x0010) >> 4;
			const u8 S = (op & 0x0200) >> 9;
			const u8 D = (op & 0x0400) >> 10;
			const u8 F1 = (op & 0x01e0) >> 5;
			std::string zString = disasmZField(Z);
			std::string xString = X ? "*pt++i" : "*pt++";
			std::string fString = disasmF1Field(F1, D, S);
			util::stream_format(stream, "%s, %s : y, x = %s", fString, zString, xString);
		}
		break;

	// Format 2a: Multiply/ALU Read/Write Group
	case 0x05: // F1, Z : aT[l]
		{
			const u8 Z = (op & 0x000f);
			const u8 X = (op & 0x0010) >> 4;
			const u8 S = (op & 0x0200) >> 9;
			const u8 aT = (op & 0x0400) >> 10;
			const u8 F1 = (op & 0x01e0) >> 5;
			std::string zString = disasmZField(Z);
			std::string atString = (aT ? "a0" : "a1");
			atString += X ? "" : "l";
			std::string fString = disasmF1Field(F1, aT, S);
			util::stream_format(stream, "%s, %s : %s", fString, zString, atString);
			break;
		}

	// Format 3: Special Functions
	case 0x12: // if CON F2
	case 0x13: // ifc CON F2
		{
			// if|ifc CON F2
			const u8 CON = (op & 0x001f);
			const u8 S = (op & 0x0200) >> 9;
			const u8 D = (op & 0x0400) >> 10;
			const u8 F2 = (op & 0x01e0) >> 5;
			std::string fString = disasmF2Field(F2, D, S);
			std::string conString = disasmCONField(CON);
			if (op & 0x0800) util::stream_format(stream,  "if %s %s", conString, fString);
			else             util::stream_format(stream, "ifc %s %s", conString, fString);
		}
		break;

	// Format 4: Branch Direct Group
	case 0x00: case 0x01: // goto JA
		{
			const uint16_t JA = (op & 0x0fff) | (pc & 0xf000);
			util::stream_format(stream, "goto 0x%04x", JA);
		}
		break;
	case 0x10: case 0x11: // call JA
		{
			const uint16_t JA = (op & 0x0fff) | (pc & 0xf000);
			util::stream_format(stream, "call 0x%04x", JA);
			dasmflags |= STEP_OVER;
		}
		break;

	// Format 5: Branch Indirect Group
	case 0x18: // goto B
		{
			const u8 B = (op & 0x0700) >> 8;
			std::string bString = disasmBField(B, dasmflags);
			util::stream_format(stream, "%s", bString);
		}
		break;

	// Format 6: Contitional Branch Qualifier/Software Interrupt (icall)
	case 0x1a: // if CON [goto/call/return]
		{
			const u8 CON = (op & 0x001f);
			std::string conString = disasmCONField(CON);
			// TODO: Test for invalid ops
			if (op == 0xd40e)
			{
				util::stream_format(stream, "icall");
				dasmflags |= STEP_OVER;
			}
			else
			{
				util::stream_format(stream, "if %s", conString);
			}
		}
		break;

	// Format 7: Data Move Group
	case 0x09: case 0x0b: // R = aS
		{
			const u8 R = (op & 0x03f0) >> 4;
			const u8 S = (op & 0x1000) >> 12;
			std::string rString = disasmRField(R);
			util::stream_format(stream, "move %s = %s", rString, S ? "a1" : "a0");
		}
		break;
	case 0x08: // aT = R
		{
			const u8 R  = (op & 0x03f0) >> 4;
			const u8 aT = (op & 0x0400) >> 10;
			std::string rString = disasmRField(R);
			util::stream_format(stream, "move %s = %s", aT ? "a0" : "a1", rString);
		}
		break;
	case 0x0f: // R = Y
		{
			const u8 Y = (op & 0x000f);
			const u8 R = (op & 0x03f0) >> 4;
			std::string yString = disasmYField(Y);
			std::string rString = disasmRField(R);
			util::stream_format(stream, "move %s = %s", rString, yString);
			// TODO: Special case the R == [y, yl, or x] case
		}
		break;
	case 0x0c: // Y = R
		{
			const u8 Y = (op & 0x000f);
			const u8 R = (op & 0x03f0) >> 4;
			std::string yString = disasmYField(Y);
			std::string rString = disasmRField(R);
			util::stream_format(stream, "move %s = %s", yString, rString);
		}
		break;
	case 0x0d: // Z : R
		{
			const u8 Z = (op & 0x000f);
			const u8 R = (op & 0x03f0) >> 4;
			std::string zString = disasmZField(Z);
			std::string rString = disasmRField(R);
			util::stream_format(stream, "move %s : %s", zString, rString);
		}
		break;

	// Format 8: Data Move (immediate operand - 2 words)
	case 0x0a: // R = N
		{
			const u8 R = (op & 0x03f0) >> 4;
			std::string rString = disasmRField(R);
			util::stream_format(stream, "%s = 0x%04x", rString, op2);
			opSize = 2;
		}
		break;

	// Format 9: Short Immediate Group
	case 0x02: case 0x03: // R = M
		{
			const uint16_t M = (op & 0x01ff);
			const u8  R = (op & 0x0e00) >> 9;
			std::string rString = disasmRImmediateField(R);
			util::stream_format(stream, "set %s = 0x%04x", rString, M);
		}
		break;

	// Format 10: do - redo
	case 0x0e: // do|redo K
		{
			const u8 K = (op & 0x007f);
			const u8 NI = (op & 0x0780) >> 7;

			// TODO: Limits on K & NI
			if (NI == 0x00)
			{
				util::stream_format(stream, "redo %u", K);
				dasmflags |= STEP_OVER;
			}
			else
				util::stream_format(stream, "do %u { %u instructions }", K, NI);
		}
		break;

	// RESERVED
	case 0x1e:
		util::stream_format(stream, "RESERVED");
		break;

	// UNKNOWN
	default:
		util::stream_format(stream, "UNKNOWN");
	}

	return opSize | dasmflags | SUPPORTED;
}
