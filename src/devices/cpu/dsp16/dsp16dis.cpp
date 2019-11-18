// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "emu.h"
#include "dsp16dis.h"
/***************************************************************************

    WE|AT&T DSP16 series disassembler

***************************************************************************/

/***********************************************************************
    construction/destruction
***********************************************************************/

dsp16_disassembler::dsp16_disassembler() : m_host(nullptr)
{
}

dsp16_disassembler::dsp16_disassembler(cpu const &host) : m_host(&host)
{
}

/***********************************************************************
    util::disasm_interface implementation
***********************************************************************/

u32 dsp16_disassembler::interface_flags() const
{
	return PAGED;
}

u32 dsp16_disassembler::page_address_bits() const
{
	return 12U;
}

u32 dsp16_disassembler::opcode_alignment() const
{
	return 1U;
}

offs_t dsp16_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u8 length(1U);
	u32 flags(0U);
	u16 const op(opcodes.r16(pc));
	switch (op >> 11)
	{
	// control group (p3-14)
	case 0x00: case 0x01: // goto JA (p3-20)
	case 0x10: case 0x11: // call JA (p3-23)
		stream << dasm_ja(op, pc, flags).text << check_branch_predicate(pc);
		break;
	case 0x18: // goto B (p3-21)
		stream << ((op & 0x00ffU) ? dasm_int(op) : dasm_b(op, flags)).text;
		break;
	case 0x1a: // if CON goto/call/return # icall (p3-22 # p3-24)
		if (0xd40e == op)
		{
			util::stream_format(stream, "icall");
			flags |= STEP_OVER;
		}
		else if ((op >> 5) & 0x003fU)
		{
			stream << dasm_int(op).text;
		}
		else
		{
			result const con(dasm_con(op));
			u16 const op2(opcodes.r16(inc_pc(pc, 1)));
			result controlled{ true, false, { } };
			switch (op2 >> 11)
			{
			case 0x00: case 0x01: // goto JA (p3-20)
			case 0x10: case 0x11: // call JA (p3-23)
				controlled = dasm_ja(op2, pc, flags);
				length = 2U;
				break;
			case 0x18: // goto B (p3-21)
				switch ((op >> 8) & 0x0007U)
				{
				case 0x0: // return
				case 0x2: // goto pt
				case 0x3: // call pt
					controlled = dasm_b(op2, flags);
					length = 2U;
					break;
				}
				break;
			}
			char const *const comment(controlled.nop ? "" : check_branch_con(pc, op));
			util::stream_format(
					stream, "if %1$s%2$s%3$s%4$s",
					con.text, controlled.nop ? "" : " ", controlled.text, comment);
		}
		break;

	// cache instructions (p3-18)
	case 0x0e: // do K { instr1...instrNI } # redo K (p3-25
		{
			u16 const ni((op & 0x0780U) >> 7);
			u16 const k(op & 0x007fU);
			if (2U > k)
			{
				stream << dasm_int(op).text;
			}
			else if (ni)
			{
				offs_t const instr1(inc_pc(pc, 1));
				offs_t const instrni(inc_pc(pc, ni));
				util::stream_format(
						stream, (1U < ni) ? "do %1$u { 0x%2$04x...0x%3$04x }" : "do %1$u { 0x%2$04x }",
						k, instr1, instrni);
				// TODO: increase skip field size for step over to support this instruction
			}
			else
			{
				util::stream_format(stream, "redo %1$u", k);
				flags |= STEP_OVER;
			}
		}
		break;

	// data move instructions (p3-16)
	case 0x02: case 0x03: // R = M (p3-27)
		{
			u16 const m(op & 0x01ffU);
			switch ((op >> 9) & 0x0007U)
			{
			case 0x0: util::stream_format(stream, "set j = 0x%03x", m); break;
			case 0x1: util::stream_format(stream, "set k = 0x%03x", m); break;
			case 0x2: util::stream_format(stream, "set rb = 0x%03x", m); break;
			case 0x3: util::stream_format(stream, "set re = 0x%03x", m); break;
			default: util::stream_format(stream, "set r%u = 0x%03x", (op >> 9) & 0x0003U, m); break;
			}
		}
		break;
	case 0x0a: // R = N (p3-28)
		if (op & 0x000fU)
		{
			stream << dasm_int(op).text;
		}
		else
		{
			length = 2U;
			bool const low(!((op >> 8) & 0x0003U));
			result const r(dasm_r(op));
			u16 const n(opcodes.r16(inc_pc(pc, 1)));
			util::stream_format(
					stream, (low && r.ambiguous) ? "move %1$s = 0x%2$04x" : "%1$s = 0x%2$04x",
					r.text, n);
		}
		break;
	case 0x09: case 0x0b: // R = aS (p3-29)
		{
			bool const high((op >> 8) & 0x0003U);
			result const r(dasm_r(op));
			util::stream_format(
					stream, (high && r.ambiguous) ? "move %1$s = a%2$u" : "%1$s = a%2$u",
					r.text, (op >> 12) & 0x0001U);
		}
		break;
	case 0x08: // aT = R (p3-30)
		{
			bool const high((op >> 8) & 0x0003U);
			result const r(dasm_r(op));
			util::stream_format(
					stream, (high && r.ambiguous) ? "move a%1$u = %2$s" : "a%1$u = %2$s",
					(~op >> 10) & 0x0001U, r.text);
		}
		break;
	case 0x0f: // R = Y (p3-32)
		if ((op >> 10) & 0x0001U)
		{
			stream << dasm_int(op).text;
		}
		else
		{
			bool const high((op >> 8) & 0x0003U);
			result const r(dasm_r(op)), y(dasm_y(op));
			util::stream_format(
					stream, (high && r.ambiguous) ? "move %1$u = %2$s" : "%1$u = %2$s",
					r.text, y.text);
		}
		break;
	case 0x0c: // Y = R (p3-33)
		{
			bool const high((op >> 8) & 0x0003U);
			result const r(dasm_r(op)), y(dasm_y(op));
			util::stream_format(
					stream, (high && r.ambiguous) ? "move %1$u = %2$s" : "%1$u = %2$s",
					y.text, r.text);
		}
		break;
	case 0x0d: // Z : R (p3-34)
		{
			bool const high((op >> 8) & 0x0003U);
			result const r(dasm_r(op)), z(dasm_z(op));
			util::stream_format(
					stream, (high && r.ambiguous) ? "move %1$u : %2$s" : "%1$u : %2$s",
					z.text, r.text);
		}
		break;

	// special function group (p3-12)
	case 0x13: // if CON F2 (p3-36)
	case 0x12: // ifc CON F2 (p3-37)
		{
			result const f2(dasm_f2(op)), con(dasm_con(op));
			bool const conditional((op & 0x0001U) || f2.ambiguous || !con.nop);
			char const *const comment(conditional ? check_special_con(pc, op) : "");
			util::stream_format(
					stream, conditional ? "if%1$s %2$s %3$s%4$s" : "%3$s%4$s",
					(op & 0x0800U) ? "" : "c", con.text, f2.text, comment);
		}
		break;

	// multiply/ALU group (p3-6)
	case 0x06: // F1 ; Y (p3-38)
		{
			result const f1(dasm_f1(op)), y(dasm_y(op));
			util::stream_format(
					stream, "%1$s ; %2$s",
					f1.text, y.text);
		}
		break;
	case 0x1c: // F1 ; Y=a0[l] (p3-40)
	case 0x04: // F1 ; Y=a1[l] (p3-40)
		{
			result const f1(dasm_f1(op)), y(dasm_y(op));
			util::stream_format(
					stream, f1.nop ? "%2$s = a%3$u%4$s" : "%1$s ; %2$s = a%3$u%4$s",
					f1.text, y.text, (~op >> 14) & 0x0001U, ((op >> 4) & 0x0001U) ? "" : "l");
		}
		break;
	case 0x16: // F1 ; x = Y (p3-42)
		{
			result const f1(dasm_f1(op)), y(dasm_y(op));
			util::stream_format(
					stream, f1.nop ? "au x = %2$s" : "%1$s ; x = %2$s",
					f1.text, y.text);
		}
		break;
	case 0x17: // F1 ; y[l] = Y (p3-44)
		{
			result const f1(dasm_f1(op)), y(dasm_y(op));
			util::stream_format(
					stream, f1.nop ? "au y%2$s = %3$s" : "%1$s ; y%2$s = %3$s",
					f1.text, ((op >> 4) & 0x0001U) ? "" : "l", y.text);
		}
		break;
	case 0x1f: // F1 ; y = Y ; x = *pt++[i] (p3-46)
		{
			result const f1(dasm_f1(op)), x(dasm_x(op)), y(dasm_y(op));
			util::stream_format(
					stream, f1.nop ? "y = %2$s ; %3$s" : "%1$s ; y = %2$s ; %3$s",
					f1.text, y.text, x.text);
		}
		break;
	case 0x19: // F1 ; y = a0 ; x = *pt++[i] (p3-48)
	case 0x1b: // F1 ; y = a1 ; x = *pt++[i] (p3-48)
		if (op & 0x000fU)
		{
			stream << dasm_int(op).text;
		}
		else
		{
			result const f1(dasm_f1(op)), x(dasm_x(op));
			util::stream_format(
					stream, f1.nop ? "y = a%2$u ; %3$s" : "%1$s ; y = a%2$u ; %3$s",
					f1.text, (op >> 12) & 0x0001U, x.text);
		}
		break;
	case 0x07: // F1 ; aT[l] = Y (p3-50)
		{
			result const f1(dasm_f1(op)), y(dasm_y(op));
			util::stream_format(
					stream, f1.nop ? "a%2$u%3$s = %4$s" : "%1$s ; a%2$u%3$s = %4$s",
					f1.text, (~op >> 10) & 0x0001U, ((op >> 4) & 0x0001U) ? "" : "l", y.text);
		}
		break;
	case 0x14: // F1 ; Y = y[l] (p3-52)
		{
			result const f1(dasm_f1(op)), y(dasm_y(op));
			util::stream_format(
					stream, f1.nop ? "au %2$s = y%3$s" : "%1$s ; %2$s = y%3$s",
					f1.text, y.text, ((op >> 4) & 0x0001U) ? "" : "l");
		}
		break;
	case 0x15: // F1 ; Z : y[l] (p3-54)
		{
			result const f1(dasm_f1(op)), z(dasm_z(op));
			util::stream_format(
					stream, f1.nop ? "au %2$s : y%3$s" : "%1$s ; %2$s : y%3$s",
					f1.text, z.text, ((op >> 4) & 0x0001U) ? "" : "l");
		}
		break;
	case 0x05: // F1 ; Z : aT[l] (p3-56)
		{
			result const f1(dasm_f1(op)), z(dasm_z(op));
			util::stream_format(
					stream, f1.nop ? "%2$s : a%3$u4$s" : "%1$s ; %2$s : a%3$u4$s",
					f1.text, z.text, (~op >> 10) & 0x0001U, ((op >> 4) & 0x0001U) ? "" : "l");
		}
		break;
	case 0x1d: // F1 ; Z : y ; x=*pt++[i] (p3-58)
		{
			result const f1(dasm_f1(op)), x(dasm_x(op)), z(dasm_z(op));
			util::stream_format(
					stream, f1.nop ? "%2$s : y ; %3$s" : "%1$s ; %2$s : y ; %3$s",
					f1.text, z.text, x.text);
		}
		break;

	case 0x1e: // Reserved
		stream << dasm_int(op).text;
		break;

	default:
		throw false;
	}
	return length | flags | SUPPORTED;
}

/***********************************************************************
    sub-instruction helpers
***********************************************************************/

dsp16_disassembler::result dsp16_disassembler::dasm_int(u16 op)
{
	return result{ false, false, util::string_format("int 0x%04x", op) };
}

dsp16_disassembler::result dsp16_disassembler::dasm_con(u16 op)
{
	u16 const con(op & 0x001fU);
	switch (con)
	{
	case 0x00: return result{ false, false, "mi" };
	case 0x01: return result{ false, false, "pl" };
	case 0x02: return result{ false, false, "eq" };
	case 0x03: return result{ false, false, "ne" };
	case 0x04: return result{ false, false, "lvs" };
	case 0x05: return result{ false, false, "lvc" };
	case 0x06: return result{ false, false, "mvs" };
	case 0x07: return result{ false, false, "mvc" };
	case 0x08: return result{ false, false, "heads" };
	case 0x09: return result{ false, false, "tails" };
	case 0x0a: return result{ false, false, "c0ge" };
	case 0x0b: return result{ false, false, "c0lt" };
	case 0x0c: return result{ false, false, "c1ge" };
	case 0x0d: return result{ false, false, "c1lt" };
	case 0x0e: return result{ true,  false, "true" };
	case 0x0f: return result{ false, false, "false" };
	case 0x10: return result{ false, false, "gt" };
	case 0x11: return result{ false, false, "le" };
	}
	return result{ false, false, "if Reserved(0x%01x)" };
}

dsp16_disassembler::result dsp16_disassembler::dasm_b(u16 op, u32 &flags)
{
	u16 const b((op >> 8) & 0x0007U);
	switch (b)
	{
	case 0x0: flags |= STEP_OUT; return result{ false, false, "return" };
	case 0x1: flags |= STEP_OUT; return result{ false, false, "ireturn" };
	case 0x2: return result{ false, false, "goto pt" };
	case 0x3: flags |= STEP_OVER; return result{ false, false, "call pt" };
	}
	return result{ false, false, util::string_format("goto Reserved(0x%01x)", b) };
}

dsp16_disassembler::result dsp16_disassembler::dasm_ja(u16 op, offs_t pc, u32 &flags)
{
	if ((op >> 15) & 0x0001U)
		flags |= STEP_OVER;
	u16 const ja(op & 0x0fffU);
	return result{
			false,
			false,
			util::string_format(
				(op >> 15) & 0x0001U ? "call 0x%1$04x" : "goto 0x%1$04x",
				(pc & ~((offs_t(1) << 12) - 1)) | ja) };
}

dsp16_disassembler::result dsp16_disassembler::dasm_f1(u16 op)
{
	u16 const d((op >> 10) & 0x0001U);
	u16 const s((op >> 9) & 0x0001U);
	switch ((op >> 5) & 0x000fU)
	{
	case 0x00: return result{ false, false, util::string_format("a%u = p ; p = x*y", d) };
	case 0x01: return result{ false, false, util::string_format("a%u = a%u + p ; p = x*y", d, s) };
	case 0x02: return result{ false, false, util::string_format("p = x*y") };
	case 0x03: return result{ false, false, util::string_format("a%u = a%u-p ; p = x*y", d, s) };
	case 0x04: return result{ false, true,  util::string_format("a%u = p", d) };
	case 0x05: return result{ false, false, util::string_format("a%u = a%u+p", d, s) };
	case 0x06: return result{ true,  false, util::string_format("nop") };
	case 0x07: return result{ false, false, util::string_format("a%u = a%u-p", d, s) };
	case 0x08: return result{ false, false, util::string_format("a%u = a%u|y", d, s) };
	case 0x09: return result{ false, false, util::string_format("a%u = a%u^y", d, s) };
	case 0x0a: return result{ false, false, util::string_format("a%u&y", s) };
	case 0x0b: return result{ false, false, util::string_format("a%u-y", s) };
	case 0x0c: return result{ false, true,  util::string_format("a%u = y", d) };
	case 0x0d: return result{ false, false, util::string_format("a%u = a%u+y", d, s) };
	case 0x0e: return result{ false, false, util::string_format("a%u = a%u&y", d, s) };
	case 0x0f: return result{ false, false, util::string_format("a%u = a%u-y", d, s) };
	}
	throw false;
}

dsp16_disassembler::result dsp16_disassembler::dasm_f2(u16 op)
{
	u16 const d((op >> 10) & 0x0001U);
	u16 const s((op >> 9) & 0x0001U);
	u16 const f2((op >> 5) & 0x000fU);
	switch (f2)
	{
	case 0x00: return result{ false, false, util::string_format("a%u = a%u>>1", d, s) };
	case 0x01: return result{ false, false, util::string_format("a%u = a%u<<1", d, s) };
	case 0x02: return result{ false, false, util::string_format("a%u = a%u>>4", d, s) };
	case 0x03: return result{ false, false, util::string_format("a%u = a%u<<4", d, s) };
	case 0x04: return result{ false, false, util::string_format("a%u = a%u>>8", d, s) };
	case 0x05: return result{ false, false, util::string_format("a%u = a%u<<8", d, s) };
	case 0x06: return result{ false, false, util::string_format("a%u = a%u>>16", d, s) };
	case 0x07: return result{ false, false, util::string_format("a%u = a%u<<16", d, s) };
	case 0x08: return result{ false, true,  util::string_format("a%u = p", d) };
	case 0x09: return result{ false, false, util::string_format("a%uh = a%uh+1", d, s) };
	case 0x0a: return result{ false, false, util::string_format("Reserved(0x%01x)", f2) };
	case 0x0b: return result{ false, false, util::string_format("a%u = rnd(a%u)", d, s) };
	case 0x0c: return result{ false, true,  util::string_format("a%u = y", d) };
	case 0x0d: return result{ false, false, util::string_format("a%u = a%u+1", d, s) };
	case 0x0e: return result{ false, false, util::string_format("a%u = a%u", d, s) };
	case 0x0f: return result{ false, false, util::string_format("a%u = -a%u", d, s) };
	}
	throw false;
}

dsp16_disassembler::result dsp16_disassembler::dasm_r(u16 op)
{
	u16 const r((op >> 4) & 0x001fU);
	switch (r)
	{
	case 0x00: return result{ false, true,  "r0" };
	case 0x01: return result{ false, true,  "r1" };
	case 0x02: return result{ false, true,  "r2" };
	case 0x03: return result{ false, true,  "r3" };
	case 0x04: return result{ false, true,  "j" };
	case 0x05: return result{ false, true,  "k" };
	case 0x06: return result{ false, true,  "rb" };
	case 0x07: return result{ false, true,  "re" };
	case 0x08: return result{ false, false, "pt" };
	case 0x09: return result{ false, false, "pr" };
	case 0x0a: return result{ false, false, "pi" };
	case 0x0b: return result{ false, false, "i" };
	case 0x10: return result{ false, true,  "x" };
	case 0x11: return result{ false, true,  "y" };
	case 0x12: return result{ false, true,  "yl" };
	case 0x13: return result{ false, false, "auc" };
	case 0x14: return result{ false, false, "psw" };
	case 0x15: return result{ false, false, "c0" };
	case 0x16: return result{ false, false, "c1" };
	case 0x17: return result{ false, false, "c2" };
	case 0x18: return result{ false, false, "sioc" };
	case 0x19: return result{ false, false, "srta" };
	case 0x1a: return result{ false, false, "sdx" };
	case 0x1b: return result{ false, false, "tdms" };
	case 0x1c: return result{ false, false, "pioc" };
	case 0x1d: return result{ false, false, "pdx0" };
	case 0x1e: return result{ false, false, "pdx1" };
	}
	return result{ false, false, util::string_format("Reserved(0x%02x)", r) };
}

dsp16_disassembler::result dsp16_disassembler::dasm_x(u16 op)
{
	u16 const x((op >> 4) & 0x0001U);
	return result{ false, false, x ? "x = *pt++i" : "x = *pt++" };
}

dsp16_disassembler::result dsp16_disassembler::dasm_y(u16 op)
{
	u16 const r((op >> 2) & 0x0003U);
	switch (op & 0x0003U)
	{
	case 0x00: return result{ false, false, util::string_format("*r%u", r) };
	case 0x01: return result{ false, false, util::string_format("*r%u++", r) };
	case 0x02: return result{ false, false, util::string_format("*r%u--", r) };
	case 0x03: return result{ false, false, util::string_format("*r%u++j", r) };
	}
	throw false;
}

dsp16_disassembler::result dsp16_disassembler::dasm_z(u16 op)
{
	u16 const r((op >> 2) & 0x0003U);
	switch (op & 0x0003U)
	{
	case 0x00: return result{ false, false, util::string_format("*r%uzp", r) };
	case 0x01: return result{ false, false, util::string_format("*r%upz", r) };
	case 0x02: return result{ false, false, util::string_format("*r%um2", r) };
	case 0x03: return result{ false, false, util::string_format("*r%ujk", r) };
	}
	throw false;
}

/***********************************************************************
    common maths
***********************************************************************/

constexpr offs_t dsp16_disassembler::inc_pc(offs_t pc, offs_t inc)
{
	return (pc & ~((offs_t(1) << 12) - 1)) | ((pc + inc) & ((offs_t(1) << 12) - 1));
}

/***********************************************************************
    live state checks
***********************************************************************/

char const *dsp16_disassembler::check_branch_predicate(offs_t pc) const
{
	if (m_host)
	{
		switch (m_host->check_branch(pc))
		{
		case cpu::predicate::INDETERMINATE: return "";
		case cpu::predicate::TAKEN: return " // will branch";
		case cpu::predicate::SKIPPED: return " // will fall through";
		}
		throw false;
	}
	else
	{
		return "";
	}
}

char const *dsp16_disassembler::check_branch_con(offs_t pc, u16 op) const
{
	if (m_host)
	{
		switch (m_host->check_con(pc, op))
		{
		case cpu::predicate::INDETERMINATE: return "";
		case cpu::predicate::TAKEN: return " // will branch";
		case cpu::predicate::SKIPPED: return " // will fall through";
		}
		throw false;
	}
	else
	{
		return "";
	}
}

char const *dsp16_disassembler::check_special_con(offs_t pc, u16 op) const
{
	if (m_host)
	{
		switch (m_host->check_con(pc, op))
		{
		case cpu::predicate::INDETERMINATE: return "";
		case cpu::predicate::TAKEN: return " // will execute";
		case cpu::predicate::SKIPPED: return " // will skip";
		}
		throw false;
	}
	else
	{
		return "";
	}
}
