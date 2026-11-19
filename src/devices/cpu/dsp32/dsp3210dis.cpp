// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    dsp3210dis.cpp
    Disassembler for the AT&T DSP3210.

    Bit fields follow the DSP3210 Information Manual (Sept 1991), ch. 10
    Tables 10-1 (formats), 10-2 (CA fields) and 10-3 (DA fields), with the
    corrections established by disassembling the Macintosh Quadra 840AV
    ROM's DSP code:

      * conditional gotos / nop / ireturn live at bit 31 = 1 (nop is
        0x80000000); the bit-31-clear encodings are reserved;
      * register-indirect moves are format 7c at op[31:25] = 1001110 with
        bit 10 = 0 and rP at 15:11; bit 10 = 1 selects the I/O-register
        moves (7b); op[31:25] = 1001111 is the I/O-register <-> memory
        form (7d);
      * float32 / int32 are special functions G = 8 / 9;
      * a Z field with P = 1111 stores through the Y operand's effective
        address and post-modifies Y's pointer with Z's I bits.

    The output syntax is the manual's assembler syntax, as also produced
    by the dsp3210-sdk reference disassembler.

***************************************************************************/

#include "emu.h"
#include "dsp3210dis.h"

#include <optional>


namespace {

//**************************************************************************
//  NAME TABLES
//**************************************************************************

// 5-bit CAU register codes (Table 10-2); the space is deliberately
// discontinuous: pc sits between r14 and r15, 22/23 are the -n/+n
// pseudo-operands, 16, 27-29 and 31 are reserved
constexpr const char *const REG_NAMES[32] =
{
	"r0",   "r1",   "r2",   "r3",   "r4",   "r5",   "r6",   "r7",
	"r8",   "r9",   "r10",  "r11",  "r12",  "r13",  "r14",  "pc",
	"?r16", "r15",  "r16",  "r17",  "r18",  "r19",  "-n",   "+n",
	"r20",  "r21",  "r22",  "?r27", "?r28", "?r29", "pcsh", "?r31"
};

// 6-bit condition codes (shared by branches and conditional ALU ops)
constexpr const char *const COND_NAMES[64] =
{
	// 00xxxx - CAU
	"false", "true", "pl",  "mi",  "ne",  "eq",  "vc",  "vs",
	"cc",    "cs",   "ge",  "lt",  "gt",  "le",  "hi",  "ls",
	// 01xxxx - DAU
	"auc",   "aus",  "age", "alt", "ane", "aeq", "avc", "avs",
	"agt",   "ale",  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	// 10xxxx - I/O
	"ibe",   "ibf",  "obf", "obe", nullptr, nullptr, nullptr, nullptr,
	"syc",   "sys",  "fbc", "fbs", "ir0c", "ir0s", "ir1c", "ir1s",
	// 11xxxx - reserved
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

// DA special functions (format 5, G field)
constexpr const char *const G_FUNCS[16] =
{
	"ic",      "oc",    "float16", "int16", "round", "ifalt", "ifaeq", "ifagt",
	"float32", "int32", nullptr,   nullptr, "ieee",  "dsp",   "seed",  nullptr
};

// CA ALU functions (F field)
constexpr const char *const F_SYMS[16] =
{
	"+", "<<", nullptr /* N - rD */, "#", "-", nullptr /* reserved */, "&~", "-",
	"^", ">>>1", "|", "<<<1", ">>", "$>>", "&", "&"
};

enum : u32
{
	F_ADD = 0, F_SHL_N, F_RSUB, F_CRADD, F_SUB, F_RES5, F_ANDC, F_CMP,
	F_XOR, F_ROR, F_OR, F_ROL, F_SHR_N, F_ASR_N, F_AND, F_BTST
};

enum : u32
{
	RC_R0 = 0,
	RC_PC = 15,
	RC_R18 = 20,
	RC_MINUS = 22,      // -n pseudo-operand
	RC_PLUS = 23,       // +n pseudo-operand
	RC_SP = 25,         // r21
	RC_PCSH = 30
};

// Z-field classification (see z_kind)
enum class zk
{
	NONE,               // no memory write
	NORMAL,             // ordinary *rP Z operand
	THRU_Y,             // p = 1111: store through Y's effective address
	BAD                 // p = 1111 with an unresolvable Y
};


//**************************************************************************
//  SMALL FORMATTERS
//**************************************************************************

// I/O registers reachable by format 7b/7d moves (everything else is MMIO)
std::string ior_name(u32 code)
{
	switch (code)
	{
		case 0:  return "ps";
		case 8:  return "emr";
		case 10: return "spc";
		case 12: return "pcw";
		case 14: return "dauc";
		case 15: return "ctr";
		default: return util::string_format("?ior%u", code);
	}
}

std::string cond_name(u32 code)
{
	const char *const n = COND_NAMES[code & 63];
	return n ? std::string(n) : util::string_format("?cond%02x", code & 63);
}

// move size keyword from the 3-bit W field; (long), the default, prints as nothing
const char *w_keyword(u32 w)
{
	switch (w & 7)
	{
		case 0:  return "(byte) ";
		case 1:  return "(char) ";
		case 2:  return "(ushort) ";
		case 3:  return "(short) ";
		case 4:  return "(hbyte) ";
		case 7:  return "";
		default: return "(?size) ";     // 101/110 reserved
	}
}

// signed displacement: "+0x12" / "-0x12"
std::string disp(s32 n)
{
	return (n < 0) ? util::string_format("-0x%x", u32(-n)) : util::string_format("+0x%x", u32(n));
}

// signed immediate as a standalone value
std::string imm(s32 n)
{
	return (n < 0) ? util::string_format("-0x%x", u32(-n)) : util::string_format("0x%x", u32(n));
}

// branch/call target {N, rB, rB+N}: rB = r0 is absolute, rB = pc is relative
// to the instruction after the delay slot (address + 8)
void ca_target(std::ostream &s, offs_t pc, u32 rb, s32 n, std::optional<u32> &target)
{
	if (rb == RC_R0)
	{
		util::stream_format(s, "0x%x", u32(n));
		target = u32(n);
	}
	else if (rb == RC_PC)
	{
		s << "pc";
		if (n != 0)
		{
			s << disp(n);
		}
		target = u32(pc) + 8 + u32(n);
	}
	else
	{
		s << REG_NAMES[rb];
		if (n != 0)
		{
			s << disp(n);
		}
	}
}

// register-indirect memory operand of the format 7c/7d moves
void ca_mem(std::ostream &s, u32 rp, u32 ri)
{
	util::stream_format(s, "*%s", REG_NAMES[rp]);
	if (ri == RC_R0)
	{
		return;                         // post-modify by zero: plain *rP
	}
	if (ri == RC_PLUS)
	{
		s << "++";
	}
	else if (ri == RC_MINUS)
	{
		s << "--";
	}
	else
	{
		util::stream_format(s, "++%s", REG_NAMES[ri]);
	}
}

// 7-bit DA X/Y/Z field: p (bits 6-3) selects r1-r14, i (bits 2-0) the
// post-modification (0 none, 1-5 r15-r19, 6/7 -/+ operand size); p = 0 is
// register-direct (a0-a3 in X/Y, "no write" in Z); p = 15 is not allowed
// here (it is the store-through-Y encoding in Z, handled by z_print).
// Returns false for a reserved combination.
bool da_operand(std::ostream &s, u32 f, bool is_z)
{
	const u32 p = BIT(f, 3, 4);
	const u32 i = f & 7;

	if (p == 0)
	{
		if (!is_z && (i <= 3))
		{
			util::stream_format(s, "a%u", i);
			return true;
		}
		s << "?";
		return false;
	}
	if (p == 15)
	{
		s << "?";
		return false;
	}
	util::stream_format(s, "*r%u", p);
	switch (i)
	{
		case 0:  break;
		case 6:  s << "--"; break;
		case 7:  s << "++"; break;
		default: util::stream_format(s, "++r%u", 14 + i); break;
	}
	return true;
}

// Z-field classification.  Table 10-3 allows "no write" only as p = 0000,
// i = 111 and calls p = 1111 "not allowed", but Apple's assembler emits
// p = 1111 for every in-place operation: the result is stored through the
// Y operand's own (pre-post-modify) address and the Z field's I bits
// post-modify Y's pointer register.  With an accumulator Y there is
// nothing to store through and no write happens.
zk z_kind(u32 z, u32 y)
{
	const u32 p = BIT(z, 3, 4);
	const u32 yp = BIT(y, 3, 4);

	if (p == 0)
	{
		return zk::NONE;
	}
	if (p != 15)
	{
		return zk::NORMAL;
	}
	if ((yp >= 1) && (yp <= 14))
	{
		return zk::THRU_Y;
	}
	if ((yp == 0) && ((y & 7) <= 3))
	{
		return zk::NONE;                // accumulator Y: nothing to store
	}
	return zk::BAD;
}

bool z_print(std::ostream &s, u32 z, u32 y, zk kind)
{
	if (kind == zk::NORMAL)
	{
		return da_operand(s, z, true);
	}
	if (kind != zk::THRU_Y)
	{
		s << "?";
		return false;
	}
	util::stream_format(s, "*r%u", BIT(y, 3, 4));
	switch (z & 7)
	{
		case 0:  break;
		case 6:  s << "--"; break;
		case 7:  s << "++"; break;
		default: util::stream_format(s, "++r%u", 14 + (z & 7)); break;
	}
	return true;
}

// the tap forms print the Z write inside the expression: (Z = Y)
void tap_operand(std::ostream &s, u32 z, u32 y, zk kind)
{
	if (kind != zk::NONE)
	{
		s << "(";
		z_print(s, z, y, kind);
		s << " = ";
		da_operand(s, y, false);
		s << ")";
	}
	else
	{
		da_operand(s, y, false);
	}
}


//**************************************************************************
//  DA (FLOATING POINT) FORMATS 1-5
//**************************************************************************

void dasm_da(std::ostream &s, u32 op)
{
	const u32 fmt = BIT(op, 29, 3);         // 1, 2 or 3
	const u32 m = BIT(op, 26, 3);
	const u32 n = BIT(op, 21, 2);
	const u32 x = BIT(op, 14, 7);
	const u32 y = BIT(op, 7, 7);
	const u32 z = op & 0x7f;
	const bool fs = BIT(op, 24);            // adder-input sign (0 = +, 1 = -)
	const bool ss = BIT(op, 23);            // product sign
	const zk kind = z_kind(z, y);
	const bool zw = (kind != zk::NONE);

	// format 5 - special functions: [Z =] aN = g(Y)
	if ((fmt == 3) && (m >= 6))
	{
		const u32 g = BIT(op, 23, 4);

		if (zw)
		{
			z_print(s, z, y, kind);
			s << " = ";
		}
		util::stream_format(s, "a%u = ", n);
		if (G_FUNCS[g])
		{
			s << G_FUNCS[g];
		}
		else
		{
			util::stream_format(s, "?gfunc%x", g);
		}
		s << "(";
		da_operand(s, y, false);
		s << ")";
		return;
	}

	// formats 1-4 have bit 25 fixed at 0; the core executes the others as
	// reserved no-ops
	if (BIT(op, 25))
	{
		util::stream_format(s, ".word 0x%08x ; reserved (DA bit 25 set)", op);
		return;
	}

	// multiply/accumulate formats:
	//   fmt 1, M = aM :  [Z =] aN = [-]Y {+,-} aM * X    FMULT-ADD-STORE
	//   fmt 1, M = 1.0:  [Z =] aN = [-]Y {+,-} X         FADD-STORE
	//   fmt 1, M = 0.0:  [Z =] aN = [-]Y                 FLOAD-STORE
	//   fmt 1, M = 110:  aN = [-](Z = Y) {+,-} X         FADD-TAP (format 4)
	//   fmt 2, M = aM :  aN = [-]aM {+,-} (Z = Y) * X    FMULT-ACC-TAP
	//   fmt 2, M = 0.0:  aN = [-](Z = Y) * X             FMULT-TAP
	//   fmt 3, M = aM :  [Z =] aN = [-]aM {+,-} Y * X    FMULT-ACC-STORE
	//   fmt 3, M = 0.0:  [Z =] aN = [-]Y * X             FMULT-STORE
	const bool tap = (fmt == 2) || ((fmt == 1) && (m == 6));

	if (zw && !tap)
	{
		z_print(s, z, y, kind);
		s << " = ";
	}
	util::stream_format(s, "a%u = ", n);

	if ((fmt == 1) && (m == 6))
	{
		// FADD-TAP: aN = [-](Z = Y) {+,-} X
		if (fs)
		{
			s << "-";
		}
		tap_operand(s, z, y, kind);
		s << (ss ? " - " : " + ");
		da_operand(s, x, false);
	}
	else if (fmt == 1)
	{
		// adder input = Y; multiplier = {aM, 0.0, 1.0} * X
		if (fs)
		{
			s << "-";
		}
		da_operand(s, y, false);
		if (m == 4)
		{
			// 0.0 * X: the product vanishes; show it only if X != a0
			if (x != 0)
			{
				s << (ss ? " - " : " + ");
				s << "0.0 * ";
				da_operand(s, x, false);
			}
		}
		else
		{
			s << (ss ? " - " : " + ");
			if (m != 5)                 // 1.0 * X prints as just X
			{
				util::stream_format(s, "a%u * ", m);
			}
			da_operand(s, x, false);
		}
	}
	else
	{
		// fmt 2 (tap) and fmt 3 (store): product = Y * X, adder input = {aM, 0.0, 1.0}
		if (m != 4)
		{
			if (fs)
			{
				s << "-";
			}
			if (m == 5)
			{
				s << "1.0";
			}
			else
			{
				util::stream_format(s, "a%u", m);
			}
			s << (ss ? " - " : " + ");
		}
		else
		{
			if (fs)
			{
				s << (ss ? "-0.0 - " : "-0.0 + ");   // -0.0 as adder input: keep it visible
			}
			else if (ss)
			{
				s << "-";
			}
		}
		if (fmt == 2)
		{
			tap_operand(s, z, y, kind);
		}
		else
		{
			da_operand(s, y, false);
		}
		s << " * ";
		da_operand(s, x, false);
	}
}


//**************************************************************************
//  CA ALU (FORMATS 6a-6d)
//**************************************************************************

void dasm_alu_reg(std::ostream &s, u32 op)
{
	const bool e = BIT(op, 31);             // 0 = short, 1 = long
	const u32 f = BIT(op, 21, 4);
	const u32 rd = BIT(op, 16, 5);
	const u32 rs1 = BIT(op, 11, 5);
	const u32 c = BIT(op, 5, 6);
	const u32 rs2 = op & 31;
	const char *const size = e ? "" : "(short) ";

	if (f == F_RES5)
	{
		util::stream_format(s, ".word 0x%08x ; reserved ALU function", op);
		return;
	}

	// condition prefix (C = 000001 "true" prints nothing)
	if (c != 1)
	{
		util::stream_format(s, "if (%s) ", cond_name(c));
	}

	// no-store forms
	if ((f == F_CMP) || (f == F_BTST))
	{
		util::stream_format(s, "%s%s %s %s", size, REG_NAMES[rs1], (f == F_CMP) ? "-" : "&", REG_NAMES[rs2]);
		return;
	}

	// stack-pointer bump: sp = sp{++,--} moves by 4, not 1
	if ((f == F_ADD) && (rd == RC_SP) && (rs1 == RC_SP) && ((rs2 == RC_PLUS) || (rs2 == RC_MINUS)))
	{
		util::stream_format(s, "sp = %ssp%s", size, (rs2 == RC_PLUS) ? "++" : "--");
		return;
	}

	util::stream_format(s, "%s = %s", REG_NAMES[rd], size);

	switch (f)
	{
		case F_ADD:
			if ((rs2 == RC_PLUS) || (rs2 == RC_MINUS))
			{
				util::stream_format(s, "%s %c 1", REG_NAMES[rs1], (rs2 == RC_PLUS) ? '+' : '-');
			}
			else if ((rs1 == rs2) && (rs1 != RC_R0))
			{
				util::stream_format(s, "%s * 2", REG_NAMES[rs1]);
			}
			else if (rs2 == RC_R0)
			{
				s << REG_NAMES[rs1];                    // assignment
			}
			else if (rs1 == RC_R0)
			{
				s << REG_NAMES[rs2];                    // assignment
			}
			else
			{
				util::stream_format(s, "%s + %s", REG_NAMES[rs1], REG_NAMES[rs2]);
			}
			break;

		case F_SUB:
			if (rs1 == RC_R0)
			{
				util::stream_format(s, "-%s", REG_NAMES[rs2]);      // negate
			}
			else if (rs2 == RC_PLUS)
			{
				util::stream_format(s, "%s - 1", REG_NAMES[rs1]);
			}
			else if (rs2 == RC_MINUS)
			{
				util::stream_format(s, "%s + 1", REG_NAMES[rs1]);
			}
			else
			{
				util::stream_format(s, "%s - %s", REG_NAMES[rs1], REG_NAMES[rs2]);
			}
			break;

		case F_RSUB:
			// the immediate form is N - rD; the register form is not in the
			// assembler's repertoire - render the raw meaning
			util::stream_format(s, "%s - %s", REG_NAMES[rs2], REG_NAMES[rs1]);
			break;

		case F_ROR:
		case F_ROL:
			util::stream_format(s, "%s %s", REG_NAMES[rs1], F_SYMS[f]);
			break;

		case F_SHL_N:
		case F_SHR_N:
		case F_ASR_N:
			if (rs2 == RC_PLUS)
			{
				util::stream_format(s, "%s %s 1", REG_NAMES[rs1], F_SYMS[f]);
			}
			else
			{
				util::stream_format(s, "%s %s %s", REG_NAMES[rs1], F_SYMS[f], REG_NAMES[rs2]);
			}
			break;

		default:    // #, &~, ^, |, &
			util::stream_format(s, "%s %s %s", REG_NAMES[rs1], F_SYMS[f], REG_NAMES[rs2]);
			break;
	}
}

void dasm_alu_imm(std::ostream &s, u32 op)
{
	const bool e = BIT(op, 31);
	const u32 f = BIT(op, 21, 4);
	const u32 rd = BIT(op, 16, 5);
	const s32 n = s16(op);
	const char *const size = e ? "" : "(short) ";

	if (f == F_RES5)
	{
		util::stream_format(s, ".word 0x%08x ; reserved ALU function", op);
		return;
	}

	// no-store forms
	if ((f == F_CMP) || (f == F_BTST))
	{
		util::stream_format(s, "%s%s %s %s", size, REG_NAMES[rd], (f == F_CMP) ? "-" : "&", imm(n));
		return;
	}

	util::stream_format(s, "%s = %s", REG_NAMES[rd], size);

	switch (f)
	{
		case F_RSUB:                                        // rD = N - rD
			util::stream_format(s, "%s - %s", imm(n), REG_NAMES[rd]);
			break;

		case F_SHL_N:
		case F_SHR_N:
		case F_ASR_N:                                       // 5 LSBs of N used
			util::stream_format(s, "%s %s %u", REG_NAMES[rd], F_SYMS[f], u32(n & 31));
			break;

		case F_ROR:
		case F_ROL:                                         // not documented with an immediate
			util::stream_format(s, "%s %s", REG_NAMES[rd], F_SYMS[f]);
			break;

		default:                                            // + - # &~ ^ | &
			util::stream_format(s, "%s %s %s", REG_NAMES[rd], F_SYMS[f], imm(n));
			break;
	}
}


//**************************************************************************
//  CA MOVES (FORMATS 7a-7d)
//**************************************************************************

void dasm_move(std::ostream &s, u32 op)
{
	const bool direct = !BIT(op, 31);       // format 7a: memory-direct
	const bool io = BIT(op, 25);            // format 7d: memory <-> I/O register
	const bool t = BIT(op, 24);             // 0 = load register, 1 = store register
	const u32 wsz = BIT(op, 21, 3);
	const u32 rh = BIT(op, 16, 5);
	const char *const kw = w_keyword(wsz);

	if (direct)
	{
		// 7a:  rH = (w) *L  /  *L = (w) rH   (L: 16-bit on-chip address)
		const u32 l = op & 0xffff;
		if (io)
		{
			util::stream_format(s, ".word 0x%08x ; reserved (format 7a, bit 25 set)", op);
			return;
		}
		if (!t)
		{
			util::stream_format(s, "%s = %s*0x%x", REG_NAMES[rh], kw, l);
		}
		else
		{
			util::stream_format(s, "*0x%x = %s%s", l, kw, REG_NAMES[rh]);
		}
		return;
	}

	if (!io && BIT(op, 10))
	{
		// 7b:  rH = (w) ior  /  ior = (w) rH
		const u32 ior = op & 31;
		const std::string in = ior_name(ior);

		// spc pseudo-instructions: stores of r0 to spc (ior10), by size
		if (t && (rh == RC_R0) && (ior == 10))
		{
			switch (wsz)
			{
				case 7:
					s << "waiti";
					return;
				case 3:
					s << "bkpt";
					return;
				case 0:
					s << "sftrst";
					return;
			}
		}
		if (!t)
		{
			util::stream_format(s, "%s = %s%s", REG_NAMES[rh], kw, in);
		}
		else
		{
			util::stream_format(s, "%s = %s%s", in, kw, REG_NAMES[rh]);
		}
		return;
	}

	// 7c: rH <-> MEM  |  7d: ior <-> MEM
	const u32 rp = BIT(op, 11, 5);
	const u32 ri = op & 31;
	const std::string regn = io ? ior_name(rh) : std::string(REG_NAMES[rh]);

	if (!t)
	{
		util::stream_format(s, "%s = %s", regn, kw);
		ca_mem(s, rp, ri);
	}
	else
	{
		ca_mem(s, rp, ri);
		util::stream_format(s, " = %s%s", kw, regn);
	}
}

} // anonymous namespace


//**************************************************************************
//  MAIN DECODER
//**************************************************************************

u32 dsp3210_disassembler::opcode_alignment() const
{
	return 4;
}

offs_t dsp3210_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const u32 op = opcodes.r32(pc);
	const u32 op6 = op >> 26;
	std::optional<u32> target;
	u32 flags = 0;

	// the seven architected illegal opcodes (manual 7.5.3.2)
	switch (op6)
	{
		case 0x00: case 0x01: case 0x02:    // reserved formats 0a/1a/2a
		case 0x0f:                          // DA format 1, M = 111
		case 0x16: case 0x17:               // DA format 2, M = 11x
		case 0x22:                          // reserved format 2b
			util::stream_format(stream, ".word 0x%08x ; illegal opcode 0x%02x", op, op6);
			return 4 | SUPPORTED;
	}

	// DA class: top three bits 001/010/011 (illegal M values filtered above)
	if (((op >> 29) >= 1) && ((op >> 29) <= 3))
	{
		dasm_da(stream, op);
		return 4 | SUPPORTED;
	}

	// every goto, call and return has one delay slot
	switch (op6)
	{
		case 0x03:                          // 3a: if (rM-- >= 0) goto
		{
			const u32 rm = BIT(op, 21, 5);
			const u32 rb = BIT(op, 16, 5);
			util::stream_format(stream, "if (%s-- >= 0) goto ", REG_NAMES[rm]);
			ca_target(stream, pc, rb, s16(op), target);
			flags = STEP_COND | step_over_extra(1);
			break;
		}

		case 0x04:                          // 4a: call {N, rB, rB+N} (rM)
		{
			const u32 rm = BIT(op, 21, 5);
			const u32 rb = BIT(op, 16, 5);
			stream << "call ";
			ca_target(stream, pc, rb, s16(op), target);
			util::stream_format(stream, " (%s)", REG_NAMES[rm]);
			flags = STEP_OVER | step_over_extra(1);
			break;
		}

		case 0x05: case 0x25:               // 5a/5b: rD = rS3 + N
		{
			const u32 rd = BIT(op, 21, 5);
			const u32 rs3 = BIT(op, 16, 5);
			const char *const size = (op6 & 0x20) ? "" : "(short) ";
			const s32 n = s16(op);
			util::stream_format(stream, "%s = %s", REG_NAMES[rd], size);
			if (rs3 == RC_R0)               // SET: rD = (short) N
			{
				stream << imm(n);
			}
			else if (n == 0)
			{
				stream << REG_NAMES[rs3];
			}
			else
			{
				util::stream_format(stream, "%s %c 0x%x", REG_NAMES[rs3], (n < 0) ? '-' : '+', u32((n < 0) ? -n : n));
			}
			break;
		}

		case 0x06: case 0x26:               // 6a-6d: ALU
			if (BIT(op, 25))
			{
				dasm_alu_imm(stream, op);
			}
			else
			{
				dasm_alu_reg(stream, op);
			}
			break;

		case 0x07: case 0x27:               // 7a-7d: moves
			dasm_move(stream, op);
			break;

		case 0x20: case 0x21:               // 0b/1b: if (COND) goto
		{
			const u32 c = BIT(op, 21, 6);
			const u32 rb = BIT(op, 16, 5);
			const s32 n = s16(op);
			if ((c == 0) && (rb == RC_R0) && (n == 0))
			{
				stream << "nop";
				break;
			}
			if ((c == 1) && (rb == RC_PCSH) && (n == 0))
			{
				// ireturn: a control transfer with NO delay slot
				stream << "ireturn";
				flags = STEP_OUT;
				break;
			}
			const bool ret = (rb == RC_R18) && (n == 0);    // goto r18: the conventional return
			if (c == 1)
			{
				stream << "goto ";
				if (ret)
				{
					flags = STEP_OUT | step_over_extra(1);
				}
			}
			else
			{
				util::stream_format(stream, "if (%s) goto ", cond_name(c));
				flags = (ret ? (STEP_OUT | STEP_COND) : STEP_COND) | step_over_extra(1);
			}
			ca_target(stream, pc, rb, n, target);
			break;
		}

		case 0x23:                          // 3b/3c: do / dolock / doblock
		{
			const bool isreg = BIT(op, 25);
			const bool b = BIT(op, 24);
			const bool m = BIT(op, 23);
			const u32 k = BIT(op, 11, 7);
			const char *const mn = m ? (b ? "?do" : "doblock") : (b ? "dolock" : "do");
			stream << mn << " ";
			if (!m || k)                    // doblock has an implicit K = 0
			{
				util::stream_format(stream, "%u, ", k);
			}
			if (isreg)
			{
				stream << REG_NAMES[op & 31];
			}
			else
			{
				util::stream_format(stream, "%u", op & 0x7ff);
			}
			break;
		}

		case 0x24:                          // 4b: rD = rS <<| N
		{
			const u32 rd = BIT(op, 21, 5);
			const u32 rs = BIT(op, 16, 5);
			util::stream_format(stream, "%s = %s <<| 0x%x", REG_NAMES[rd], REG_NAMES[rs], op & 0xffff);
			break;
		}

		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:     // 8a: goto {M, rB+M}
		{
			const u32 rb = BIT(op, 16, 5);
			const u32 m = (BIT(op, 21, 8) << 16) | (op & 0xffff);
			stream << "goto ";
			if (rb == RC_R0)
			{
				util::stream_format(stream, "0x%x", m);
				target = m;
			}
			else if (rb == RC_PC)
			{
				util::stream_format(stream, "pc+0x%x", m);
				target = u32(pc) + 8 + m;
			}
			else
			{
				util::stream_format(stream, "%s+0x%x", REG_NAMES[rb], m);
			}
			break;
		}

		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x34: case 0x35: case 0x36: case 0x37:     // 8b: rD = (ushort24) M
		{
			const u32 rd = BIT(op, 16, 5);
			const u32 m = (BIT(op, 21, 8) << 16) | (op & 0xffff);
			util::stream_format(stream, "%s = (ushort24) 0x%x", REG_NAMES[rd], m);
			break;
		}

		case 0x38: case 0x39: case 0x3a: case 0x3b:
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:     // 8c: call M (rM)
		{
			const u32 rm = BIT(op, 16, 5);
			const u32 m = (BIT(op, 21, 8) << 16) | (op & 0xffff);
			util::stream_format(stream, "call 0x%x (%s)", m, REG_NAMES[rm]);
			target = m;
			flags = STEP_OVER | step_over_extra(1);
			break;
		}
	}

	if (target)
	{
		util::stream_format(stream, " ; -> 0x%08x", *target);
	}

	return 4 | flags | SUPPORTED;
}
