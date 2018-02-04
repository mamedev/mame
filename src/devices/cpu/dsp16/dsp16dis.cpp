// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include "emu.h"
#include "dsp16dis.h"

std::string dsp16a_disassembler::disasmF1Field(const uint8_t& F1, const uint8_t& D, const uint8_t& S)
{
	switch (F1)
	{
		case 0x00: return string_format("a%u = p, p = x*y", D); break;
		case 0x01: return string_format("a%u = a%u + p, p = x*y", D, S); break;
		case 0x02: return string_format("p = x*y"); break;
		case 0x03: return string_format("a%u = a%u - p, p = x*y", D, S); break;
		case 0x04: return string_format("a%u = p", D); break;
		case 0x05: return string_format("a%u = a%u + p", D, S); break;
		case 0x06: return string_format("NOP"); break;
		case 0x07: return string_format("a%u = a%u - p", D, S); break;
		case 0x08: return string_format("a%u = a%u | y", D, S); break;
		case 0x09: return string_format("a%u = a%u ^ y", D, S); break;
		case 0x0a: return string_format("a%u & y", S); break;
		case 0x0b: return string_format("a%u - y", S); break;
		case 0x0c: return string_format("a%u = y", D); break;
		case 0x0d: return string_format("a%u = a%u + y", D, S); break;
		case 0x0e: return string_format("a%u = a%u & y", D, S); break;
		case 0x0f: return string_format("a%u = a%u - y", D, S); break;

		default: return "UNKNOWN";
	}
}

std::string dsp16a_disassembler::disasmYField(const uint8_t& Y)
{
	switch (Y)
	{
		case 0x00: return "*r0";
		case 0x01: return "*r0++";
		case 0x02: return "*r0--";
		case 0x03: return "*r0++j";

		case 0x04: return "*r1";
		case 0x05: return "*r1++";
		case 0x06: return "*r1--";
		case 0x07: return "*r1++j";

		case 0x08: return "*r2";
		case 0x09: return "*r2++";
		case 0x0a: return "*r2--";
		case 0x0b: return "*r2++j";

		case 0x0c: return "*r3";
		case 0x0d: return "*r3++";
		case 0x0e: return "*r3--";
		case 0x0f: return "*r3++j";

		default: return "UNKNOWN";
	}
	// never executed
	//return "";
}

std::string dsp16a_disassembler::disasmZField(const uint8_t& Z)
{
	switch (Z)
	{
		case 0x00: return "*r0zp";
		case 0x01: return "*r0pz";
		case 0x02: return "*r0m2";
		case 0x03: return "*r0jk";

		case 0x04: return "*r1zp";
		case 0x05: return "*r1pz";
		case 0x06: return "*r1m2";
		case 0x07: return "*r1jk";

		case 0x08: return "*r2zp";
		case 0x09: return "*r2pz";
		case 0x0a: return "*r2m2";
		case 0x0b: return "*r2jk";

		case 0x0c: return "*r3zp";
		case 0x0d: return "*r3pz";
		case 0x0e: return "*r3m2";
		case 0x0f: return "*r3jk";

		default: return "UNKNOWN";
	}
	// never executed
	//return "";
}

std::string dsp16a_disassembler::disasmF2Field(const uint8_t& F2, const uint8_t& D, const uint8_t& S)
{
	std::string ret = "";
	switch (F2)
	{
		case 0x00: return string_format("a%u = a%u >> 1", D, S); break;
		case 0x01: return string_format("a%u = a%u << 1", D, S); break;
		case 0x02: return string_format("a%u = a%u >> 4", D, S); break;
		case 0x03: return string_format("a%u = a%u << 4", D, S); break;
		case 0x04: return string_format("a%u = a%u >> 8", D, S); break;
		case 0x05: return string_format("a%u = a%u << 8", D, S); break;
		case 0x06: return string_format("a%u = a%u >> 16", D, S); break;
		case 0x07: return string_format("a%u = a%u << 16", D, S); break;

		case 0x08: return string_format("a%u = p", D); break;
		case 0x09: return string_format("a%uh = a%uh + 1", D, S); break;
		case 0x0a: return string_format("RESERVED"); break;
		case 0x0b: return string_format("a%u = rnd(a%u)", D, S); break;
		case 0x0c: return string_format("a%u = y", D); break;
		case 0x0d: return string_format("a%u = a%u + 1", D, S); break;
		case 0x0e: return string_format("a%u = a%u", D, S); break;
		case 0x0f: return string_format("a%u = -a%u", D, S); break;

		default: return "UNKNOWN";
	}
	return ret;
}

std::string dsp16a_disassembler::disasmCONField(const uint8_t& CON)
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
	// never executed
	//return "";
}

std::string dsp16a_disassembler::disasmBField(const uint8_t& B)
{
	switch (B)
	{
		case 0x00: return "return";
		case 0x01: return "ireturn";
		case 0x02: return "goto pt";
		case 0x03: return "call pt";
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07: return "RESERVED";

		default: return "UNKNOWN";
	}
	// never executed
	//return "";
}

std::string dsp16a_disassembler::disasmRImmediateField(const uint8_t& R)
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

		default: return "UNKNOWN";
	}
	// never executed
	//return "";
}

std::string dsp16a_disassembler::disasmRField(const uint8_t& R)
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
	// never executed
	//return "";
}

std::string dsp16a_disassembler::disasmIField(const uint8_t& I)
{
	switch (I)
	{
		case 0x00: return "r0/j";
		case 0x01: return "r1/k";
		case 0x02: return "r2/rb";
		case 0x03: return "r3/re";

		default: return "UNKNOWN";
	}
	// never executed
	//return "";
}

bool dsp16a_disassembler::disasmSIField(const uint8_t& SI)
{
	switch (SI)
	{
		case 0x00: return 0;    // Not a software interrupt
		case 0x01: return 1;    // Software Interrupt
	}
	return false;
}


u32 dsp16a_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t dsp16a_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint8_t opSize = 1;
	uint32_t dasmflags = 0;
	uint16_t op  = opcodes.r16(pc);
	uint16_t op2 = opcodes.r16(pc+1);

	// TODO: Test for previous "if CON" instruction and tab the next instruction in?

	const uint8_t opcode = (op >> 11) & 0x1f;
	switch(opcode)
	{
		// Format 1: Multiply/ALU Read/Write Group
		case 0x06:
		{
			// F1, Y
			const uint8_t Y = (op & 0x000f);
			const uint8_t S = (op & 0x0200) >> 9;
			const uint8_t D = (op & 0x0400) >> 10;
			const uint8_t F1 = (op & 0x01e0) >> 5;
			std::string yString = disasmYField(Y);
			std::string fString = disasmF1Field(F1, D, S);
			util::stream_format(stream, "%s, %s", fString, yString);
			break;
		}
		case 0x04: case 0x1c:
		{
			// F1 Y=a0[1] | F1 Y=a1[1]
			const uint8_t Y = (op & 0x000f);
			const uint8_t X = (op & 0x0010) >> 4;
			const uint8_t S = (op & 0x0200) >> 9;
			const uint8_t D = (op & 0x0400) >> 10;
			const uint8_t F1 = (op & 0x01e0) >> 5;
			std::string yString = disasmYField(Y);
			std::string fString = disasmF1Field(F1, D, S);
			std::string aString = (opcode == 0x1c) ? "a0" : "a1";
			std::string xString = (X) ? "" : "l";
			util::stream_format(stream, "%s = %s%s, %s", yString, aString, xString, fString);
			break;
		}
		case 0x16:
		{
			// F1, x = Y
			const uint8_t Y = (op & 0x000f);
			const uint8_t S = (op & 0x0200) >> 9;
			const uint8_t D = (op & 0x0400) >> 10;
			const uint8_t F1 = (op & 0x01e0) >> 5;
			std::string yString = disasmYField(Y);
			std::string fString = disasmF1Field(F1, D, S);
			util::stream_format(stream, "%s, x = %s", fString, yString);
			break;
		}
		case 0x17:
		{
			// F1, y[l] = Y
			const uint8_t Y = (op & 0x000f);
			const uint8_t X = (op & 0x0010) >> 4;
			const uint8_t S = (op & 0x0200) >> 9;
			const uint8_t D = (op & 0x0400) >> 10;
			const uint8_t F1 = (op & 0x01e0) >> 5;
			std::string yString = disasmYField(Y);
			std::string fString = disasmF1Field(F1, D, S);
			std::string xString = (X ? "y" : "y1");
			util::stream_format(stream, "%s, %s = %s", fString, xString, yString);
			break;
		}
		case 0x1f:
		{
			// F1, y = Y, x = *pt++[i]
			const uint8_t Y = (op & 0x000f);
			const uint8_t X = (op & 0x0010) >> 4;
			const uint8_t S = (op & 0x0200) >> 9;
			const uint8_t D = (op & 0x0400) >> 10;
			const uint8_t F1 = (op & 0x01e0) >> 5;
			std::string yString = disasmYField(Y);
			std::string fString = disasmF1Field(F1, D, S);
			std::string xString = (X ? "*pt++i" : "*pt++");
			util::stream_format(stream, "%s, y = %s, x = %s", fString, yString, xString);
			break;
		}
		case 0x19: case 0x1b:
		{
			// F1, y = a0|1, x = *pt++[i]
			const uint8_t Y = (op & 0x000f);
			const uint8_t X = (op & 0x0010) >> 4;
			const uint8_t S = (op & 0x0200) >> 9;
			const uint8_t D = (op & 0x0400) >> 10;
			const uint8_t F1 = (op & 0x01e0) >> 5;
			std::string fString = disasmF1Field(F1, D, S);
			std::string xString = (X ? "*pt++i" : "*pt++");
			std::string aString = (opcode == 0x19) ? "a0" : "a1";

			if (Y != 0x00)
				util::stream_format(stream, "UNKNOWN");
			else
				util::stream_format(stream, "%s, y = %s, x = %s", fString, aString, xString);
			break;
		}
		case 0x14:
		{
			// F1, Y = y[1]
			const uint8_t Y = (op & 0x000f);
			const uint8_t X = (op & 0x0010) >> 4;
			const uint8_t S = (op & 0x0200) >> 9;
			const uint8_t D = (op & 0x0400) >> 10;
			const uint8_t F1 = (op & 0x01e0) >> 5;
			std::string yString = disasmYField(Y);
			std::string xString = (X ? "y" : "y1");
			std::string fString = disasmF1Field(F1, D, S);
			util::stream_format(stream, "%s, %s = %s", fString, yString, xString);
			break;
		}

		// Format 1a: Multiply/ALU Read/Write Group (major typo in docs on p3-51)
		case 0x07:
		{
			// F1, At[1] = Y
			const uint8_t Y = (op & 0x000f);
			const uint8_t S = (op & 0x0200) >> 9;
			const uint8_t aT = (op & 0x0400) >> 10;
			const uint8_t F1 = (op & 0x01e0) >> 5;
			std::string yString = disasmYField(Y);
			std::string atString = (aT ? "a0" : "a1");
			std::string fString = disasmF1Field(F1, aT, S);
			util::stream_format(stream, "%s, %s = %s", fString, atString, yString);
			break;
		}

		// Format 2: Multiply/ALU Read/Write Group
		case 0x15:
		{
			// F1, Z : y[1]
			const uint8_t Z = (op & 0x000f);
			const uint8_t X = (op & 0x0010) >> 4;
			const uint8_t S = (op & 0x0200) >> 9;
			const uint8_t D = (op & 0x0400) >> 10;
			const uint8_t F1 = (op & 0x01e0) >> 5;
			std::string zString = disasmZField(Z);
			std::string xString = (X ? "y" : "y1");
			std::string fString = disasmF1Field(F1, D, S);
			util::stream_format(stream, "%s, %s <=> %s", fString, xString, zString);
			break;
		}
		case 0x1d:
		{
			// F1, Z : y, x=*pt++[i]
			const uint8_t Z = (op & 0x000f);
			const uint8_t X = (op & 0x0010) >> 4;
			const uint8_t S = (op & 0x0200) >> 9;
			const uint8_t D = (op & 0x0400) >> 10;
			const uint8_t F1 = (op & 0x01e0) >> 5;
			std::string zString = disasmZField(Z);
			std::string xString = (X ? "*pt++i" : "*pt++");
			std::string fString = disasmF1Field(F1, D, S);
			util::stream_format(stream, "%s, %s <=> y, x = %s", fString, zString, xString);
			break;
		}

		// Format 2a: Multiply/ALU Read/Write Group
		case 0x05:
		{
			// F1, Z : aT[1]
			const uint8_t Z = (op & 0x000f);
			const uint8_t X = (op & 0x0010) >> 4;
			const uint8_t S = (op & 0x0200) >> 9;
			const uint8_t aT = (op & 0x0400) >> 10;
			const uint8_t F1 = (op & 0x01e0) >> 5;
			std::string zString = disasmZField(Z);
			std::string atString = (aT ? "a0" : "a1");
			atString += X ? "" : "1";   // TODO: Figure out unclear wording.
			std::string fString = disasmF1Field(F1, aT, S);
			util::stream_format(stream, "%s, %s <=> %s", fString, zString, atString);
			break;
		}

		// Format 3: Special Functions
		case 0x12:
		case 0x13:
		{
			// if|ifc CON F2
			const uint8_t CON = (op & 0x001f);
			const uint8_t S = (op & 0x0200) >> 9;
			const uint8_t D = (op & 0x0400) >> 10;
			const uint8_t F2 = (op & 0x01e0) >> 5;
			std::string fString = disasmF2Field(F2, D, S);
			std::string conString = disasmCONField(CON);
			if (op & 0x0800) util::stream_format(stream,  "if %s : %s", conString, fString);
			else             util::stream_format(stream, "ifc %s : %s", conString, fString);
			break;
		}

		// Format 4: Branch Direct Group
		case 0x00: case 0x01:
		{
			// goto JA
			const uint16_t JA = (op & 0x0fff) | (pc & 0xf000);
			util::stream_format(stream, "goto 0x%04x", JA);
			break;
		}
		case 0x10: case 0x11:
		{
			// call JA
			const uint16_t JA = (op & 0x0fff) | (pc & 0xf000);
			util::stream_format(stream, "call 0x%04x", JA);
			break;
		}

		// Format 5: Branch Indirect Group
		case 0x18:
		{
			// goto B
			const uint8_t B = (op & 0x0700) >> 8;
			std::string bString = disasmBField(B);
			util::stream_format(stream, "%s", bString);
			break;
		}

		// Format 6: Contitional Branch Qualifier/Software Interrupt (icall)
		case 0x1a:
		{
			// if CON [goto/call/return]
			const uint8_t CON = (op & 0x001f);
			std::string conString = disasmCONField(CON);
			util::stream_format(stream, "if %s:", conString);
			// TODO: Test for invalid ops
			// icall
			if (op == 0xd40e) util::stream_format(stream, "icall");
			break;
		}

		// Format 7: Data Move Group
		case 0x09: case 0x0b:
		{
			// R = aS
			const uint8_t R = (op & 0x03f0) >> 4;
			const uint8_t S = (op & 0x1000) >> 12;
			std::string rString = disasmRField(R);
			util::stream_format(stream, "%s = %s", rString, (S ? "a1" : "a0"));
			break;
		}
		case 0x08:
		{
			// aT = R
			const uint8_t R  = (op & 0x03f0) >> 4;
			const uint8_t aT = (op & 0x0400) >> 10;
			std::string rString = disasmRField(R);
			util::stream_format(stream, "%s = %s", (aT ? "a0" : "a1"), rString);
			break;
		}
		case 0x0f:
		{
			// R = Y
			const uint8_t Y = (op & 0x000f);
			const uint8_t R = (op & 0x03f0) >> 4;
			std::string yString = disasmYField(Y);
			std::string rString = disasmRField(R);
			util::stream_format(stream, "%s = %s", rString, yString);
			// TODO: Special case the R == [y, y1, or x] case
			break;
		}
		case 0x0c:
		{
			// Y = R
			const uint8_t Y = (op & 0x000f);
			const uint8_t R = (op & 0x03f0) >> 4;
			std::string yString = disasmYField(Y);
			std::string rString = disasmRField(R);
			// TODO: page 3-31 "special function encoding"
			util::stream_format(stream, "%s = %s", yString, rString);
			break;
		}
		case 0x0d:
		{
			// Z : R
			const uint8_t Z = (op & 0x000f);
			const uint8_t R = (op & 0x03f0) >> 4;
			std::string zString = disasmZField(Z);
			std::string rString = disasmRField(R);
			util::stream_format(stream, "%s <=> %s", zString, rString);
			break;
		}

		// Format 8: Data Move (immediate operand - 2 words)
		case 0x0a:
		{
			// R = N
			const uint8_t R = (op & 0x03f0) >> 4;
			std::string rString = disasmRField(R);
			util::stream_format(stream, "%s = 0x%04x", rString, op2);
			opSize = 2;
			break;
		}

		// Format 9: Short Immediate Group
		case 0x02: case 0x03:
		{
			// R = M
			const uint16_t M = (op & 0x01ff);
			const uint8_t  R = (op & 0x0e00) >> 9;
			std::string rString = disasmRImmediateField(R);
			util::stream_format(stream, "%s = 0x%04x", rString, M);
			break;
		}

		// Format 10: do - redo
		case 0x0e:
		{
			// do|redo K
			const uint8_t K = (op & 0x007f);
			const uint8_t NI = (op & 0x0780) >> 7;

			// TODO: Limits on K & NI
			if (NI == 0x00)
				util::stream_format(stream, "redo %d", K);
			else
				util::stream_format(stream, "do (next %d inst) %d times", NI, K);
			break;
		}

		// RESERVED
		case 0x1e:
		{
			util::stream_format(stream, "RESERVED");
			break;
		}

		// UNKNOWN
		default:
		{
			util::stream_format(stream, "UNKNOWN");
			break;
		}
	}

	return opSize | dasmflags | SUPPORTED;
}
