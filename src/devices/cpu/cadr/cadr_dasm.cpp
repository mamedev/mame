// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    MIT CADR microcode disassembler

Disassembler reverse engineered from cadrlp source.

***************************************************************************/

#include "emu.h"
#include "cadr_dasm.h"


namespace {

static const char *const jump_rp[0x04] =
{
	"JUMP", "CALL", "POPJ", "CALL-POPJ-??"
};

static const char *const jump_cond[0x08] =
{
	"T",              "-LESS-THAN",                  "-LESS-OR-EQUAL",     "-EQUAL",
	"-IF-PAGE-FAULT", "-IF-PAGE-FAULT-OR-INTERRUPT", "-IF-SEQUENCE-BREAK", "NIL"
};

static const char *const jump_cond_invert[0x08] =
{
	"T",                 "-GREATER-OR-EQUAL",              "-GREATER-THAN",         "-NOT-EQUAL",
	"-IF-NO-PAGE-FAULT", "-IF-NO-PAGE-FAULT-OR-INTERRUPT", "-IF-NO-SEQUENCE-BREAK", "-NEVER"
};

static const char *const alu_op[0x40] =
{
	"SETZ",            "AND",             "ANDCA",           "SETM",
	"ANDCM",           "",                "XOR",             "IOR",
	"ANDCB",           "EQV",             "SETCA",           "ORCA",
	"SETCM",           "ORCM",            "ORCB",            "SETO",

	"ALU-FUNCTION-20", "ALU-FUNCTION-21", "ALU-FUNCTION-22", "ALU-FUNCTION-23",
	"ALU-FUNCTION-24", "ALU-FUNCTION-25", "SUB",             "ALU-FUNCTION-27",
	"ALU-FUNCTION-30", "ADD",             "ALU-FUNCTION-32", "ALU-FUNCTION-33",
	"INCM",            "ALU-FUNCTION-35", "ALU-FUNCTION-36", "LSHM",

	"MUL",             "DIV",             "ALU-FUNCTION-42", "ALU-FUNCTION-43",
	"ALU-FUNCTION-44", "DIVRC",           "ALU-FUNCTION-46", "ALU-FUNCTION-47",
	"ALU-FUNCTION-50", "DIVFS",           "ALU-FUNCTION-52", "ALU-FUNCTION-53",
	"ALU-FUNCTION-54", "ALU-FUNCTION-55", "ALU-FUNCTION-56", "ALU-FUNCTION-57",

	"ALU-FUNCTION-60", "ALU-FUNCTION-61", "ALU-FUNCTION-62", "ALU-FUNCTION-63",
	"ALU-FUNCTION-64", "ALU-FUNCTION-65", "ALU-FUNCTION-66", "ALU-FUNCTION-67",
	"ALU-FUNCTION-70", "ALU-FUNCTION-71", "ALU-FUNCTION-72", "ALU-FUNCTION-73",
	"ALU-FUNCTION-74", "ALU-FUNCTION-75", "ALU-FUNCTION-76", "ALU-FUNCTION-77",
};

static const char *const output_selector[0x04] =
{
	"OUTPUT-SELECTOR-0", "", "OUTPUT-SELECTOR-RIGHTSHIFT-1", "OUTPUT-SELECTOR-LEFTSHIFT-1"
};

static const char *const q_shift[0x04] =
{
	"", "SHIFT-Q-LEFT", "SHIFT-Q-RIGHT", ""
};

static const char *const byte_operation[0x04] =
{
	"BYTE-OPERATION-0", "LDB", "SELECTIVE-DEPOSIT", "DPB"
};

static const char *const map_dispatch[0x04] =
{
	"", " MAP-14", " MAP-15", " MAP-BOTH-14-AND-15"
};


void output(std::ostream &stream, bool &need_sp, const char * const string)
{
	if (need_sp) stream << ' ';
	stream << string;
	need_sp = true;
}


template <typename Stream, typename Format, typename... Params>
typename Stream::off_type output(Stream &stream, bool &need_sp, Format const &fmt, Params &&... args)
{
	if (need_sp) stream << ' ';
	need_sp = true;
	return util::stream_format(stream, fmt, std::forward<Params>(args)...);
}


void m_source(std::ostream &stream, bool &need_sp, u64 op)
{
	if (BIT(op, 31))
	{
		switch ((op >> 26) & 0x1f)
		{
		case 0x00: output(stream, need_sp, "READ-I-ARG"); break;
		case 0x01: output(stream, need_sp, "MICRO-STACK-PNTR-AND-DATA"); break;
		case 0x02: output(stream, need_sp, "PDL-BUFFER-POINTER-%o", op & 0x3ff); break;
		case 0x03: output(stream, need_sp, "PDL-BUFFER-INDEX-%o", op & 0x3ff); break;
//		case 0x04: output(stream, need_sp, "C-PDL-BUFFER-INDEX--"); break; // not official
		case 0x05: output(stream, need_sp, "C-PDL-BUFFER-INDEX"); break;
		case 0x06: output(stream, need_sp, "C-OPC-BUFFER-%05o", op & 0x1fff); break;
		case 0x07: output(stream, need_sp, "Q-R"); break;
		case 0x08: output(stream, need_sp, "VMA"); break;
		case 0x09: output(stream, need_sp, "MEMORY-MAP-DATA"); break;
		case 0x0a: output(stream, need_sp, "MD"); break;
		case 0x0b: output(stream, need_sp, "LOCATION-COUNTER"); break;
		case 0x0c: output(stream, need_sp, "MICRO-STACK-PNTR-AND-DATA-POP"); break;
//		case 0x0d: output(stream, need_sp, "reserved-0d"); break;
//		case 0x0e: output(stream, need_sp, "reserved-0e"); break;
//		case 0x0f: output(stream, need_sp, "reserved-0f"); break;
		case 0x14: output(stream, need_sp, "C-PDL-BUFFER-POINTER-POP"); break;
		case 0x15: output(stream, need_sp, "C-PDL-BUFFER-POINTER"); break;
		default:   output(stream, need_sp, "FSOURCE-%o", (op >> 26) & 0x1f); break;
		}
	}
	else
	{
		if ((op >> 26) & 0x1f)
		{
			output(stream, need_sp, "M-%o", (op >> 26) & 0x1f);
		}
	}
}


void disassemble_alu_op(std::ostream &stream, bool &need_sp, u64 op)
{
	if (((op >> 3) & 0x3f) != 0x05)
		output(stream, need_sp, "%s", alu_op[(op >> 3) & 0x3f]);
	if (((op >> 3) & 0x3f) == 22)
	{
		if (!BIT(op, 2))
			output(stream, need_sp, "ALU-CARRY-IN-ZERO");
	}
	else
	{
		if (BIT(op, 2))
			output(stream, need_sp, "ALU-CARRY-IN-ONE");
	}
}


void disassemble_destination(std::ostream &stream, bool &need_sp, u64 op)
{
	if (!((op >> 14) & 0x7ff))
	{
		if (((op >> 43) & 0x03) == 0x00 && (op & 0x03) == 0x03)
			output(stream, need_sp, "(Q-R)");
		return;
	}

	output(stream, need_sp, "(");
	need_sp = false; // We do not want a separator after an opening (
	if (BIT(op, 25))
	{
		output(stream, need_sp, "A-%o", (op >> 14) & 0x3ff);
	}
	else
	{
		if ((op >> 14) & 0x1f)
			output(stream, need_sp, "M-%o", (op >> 14) & 0x1f);

		switch ((op >> 19) & 0x1f)
		{
		case 0x00: break;
		case 0x01: output(stream, need_sp, "LOCATION-COUNTER"); break;
		case 0x02: output(stream, need_sp, "INTERRUPT-CONTROL"); break;
		case 0x08: output(stream, need_sp, "C-PDL-BUFFER-POINTER"); break;
		case 0x09: output(stream, need_sp, "C-PDL-BUFFER-POINTER-PUSH"); break;
		case 0x0a: output(stream, need_sp, "C-PDL-BUFFER-INDEX"); break;
		case 0x0b: output(stream, need_sp, "PDL-BUFFER-INDEX"); break;
		case 0x0c: output(stream, need_sp, "PDL-BUFFER-POINTER"); break;
		case 0x0d: output(stream, need_sp, "MICRO-STACK-DATA-PUSH"); break;
		case 0x0e: output(stream, need_sp, "OA-REG-LOW"); break;
		case 0x0f: output(stream, need_sp, "OA-REG-HI"); break;
		case 0x10: output(stream, need_sp, "VMA"); break;
		case 0x11: output(stream, need_sp, "VMA-START-READ"); break;
		case 0x12: output(stream, need_sp, "VMA-START-WRITE"); break;
		case 0x13: output(stream, need_sp, "VMA-WRITE-MAP"); break;
		case 0x18: output(stream, need_sp, "MD"); break;
		case 0x19: output(stream, need_sp, "MD-START-READ"); break;
		case 0x1a: output(stream, need_sp, "MD-START-WRITE"); break;
		case 0x1b: output(stream, need_sp, "MD-WRITE-MAP"); break;
		default:   output(stream, need_sp, "FDEST-%o", (op >> 19) & 0x1f); break;
		}
	}
	if (((op >> 43) & 0x03) == 0x00 && (op & 0x03) == 0x03)
	{
		output(stream, need_sp, "Q-R");
	}
	stream << ')';
	need_sp = true;
}


void disassemble_jump_condition(std::ostream &stream, bool &need_sp, u64 op)
{
	output(stream, need_sp, jump_rp[(op >>8) & 0x03]);
	if (!BIT(op, 5))
	{
		stream << (BIT(op, 6) ? "-IF-BIT-SET" : "-IF-BIT-CLEAR");
		if (BIT(op, 7))
			stream << "-XCT-NEXT";
		output(stream, need_sp, "(BYTE-FIELD 1 %o)", 32 - (op & 0x1f));
	}
	else
	{
		if (op & 0x07)
		{
			if (BIT(op, 6) || (op & 0x07) != 0x07)
				stream << (BIT(op, 6) ? jump_cond_invert[op & 0x07] : jump_cond[op & 0x07]);
			if (!BIT(op, 7))
				stream << "-XCT-NEXT";
		}
		else
		{
			if (!BIT(op, 7))
				stream << "-XCT-NEXT";
			stream << " JUMP-CONDITION 0";
			if (!BIT(op, 6))
				stream << " (INVERTED)";
		}
	}
}


} // anonymous namespace


u32 cadr_disassembler::opcode_alignment() const
{
	return 1;
}


offs_t cadr_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	offs_t cpc = pc;
	offs_t flags = 0;
	u64 op = opcodes.r64(cpc++);
	bool need_sp = false;

	if (BIT(op, 42))
		output(stream, need_sp, "(POPJ-AFTER-NEXT");
	else
		stream << '(';

	switch ((op >> 43) & 0x03)
	{
	case 0x00: // ALU
		disassemble_destination(stream, need_sp, op);
		disassemble_alu_op(stream, need_sp, op);
		if (((op >> 12) & 0x03) != 0x01)
			output(stream, need_sp, "%s", output_selector[(op >> 12) & 0x03]);
		if (((op & 0x03) == 0x01) || ((op & 0x03) == 0x02))
			output(stream, need_sp, "%s", q_shift[op & 0x03]);
		m_source(stream, need_sp, op);
		if ((op >> 32) & 0x3ff)
			output(stream, need_sp, "A-%o", (op >> 32) & 0x3ff);
		break;
	case 0x01: // JUMP
		disassemble_jump_condition(stream, need_sp, op);
		m_source(stream, need_sp, op);
		if ((op >> 32) & 0x3ff)
			output(stream, need_sp, "A-%o", (op >> 32) & 0x3ff);
		output(stream, need_sp, "%o", (op >> 12) & 0x3fff);
		break;
	case 0x02: // DISPATCH
		output(stream, need_sp, "DISPATCH");
		if ((op >> 32) & 0x3ff)
			output(stream, need_sp, "(%o)", (op >> 12) & 0x3ff);
		output(stream, need_sp, "(BYTE-FIELD %o %o)", ((op >> 5) & 0x07), 32 - (op & 0x1f));
		m_source(stream, need_sp, op);
		output(stream, need_sp, "%o", (op >> 12) & 0x7ff);
		if (BIT(op, 25))
			output(stream, need_sp, "PUSH-OWN-ADDRESS");
		if (BIT(op, 24))
			output(stream, need_sp, "IFETCH");
		stream << map_dispatch[(op >> 8) & 0x03];
		break;
	case 0x03: // BYTE
		disassemble_destination(stream, need_sp, op);
		output(stream, need_sp, byte_operation[(op >> 12) & 0x03]);
		output(stream, need_sp, "(BYTE-FIELD %o %o)", ((op >> 5) & 0x07) + 1, ((op >> 10) & 0x03) == 1 ? 32 - (op & 0x1f) : (op & 0x1f));
		m_source(stream, need_sp, op);
		if ((op >> 32) & 0x3ff)
			output(stream, need_sp, "A-%o", (op >> 32) & 0x3ff);
		break;
	}
	if ((op >> 10) & 0x03)
		output(stream, need_sp, "MF-%o", (op >> 10) & 0x03);
	if (BIT(op, 45))
		output(stream, need_sp, "ILONG");
	stream << ')';

	return (cpc - pc) | flags | SUPPORTED;
}
