// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    This is the base for several NEC 78K family disassemblers. These have
    more or less incompatible instruction decodings, so this file mostly
    contains common helpers.

***************************************************************************/

#include "emu.h"
#include "upd78kd.h"

upd78k_family_disassembler::upd78k_family_disassembler(const char *const sfr_names[], const char *const sfrp_names[], u16 saddr_ram_base)
	: util::disasm_interface()
	, m_sfr_names(sfr_names)
	, m_sfrp_names(sfrp_names)
	, m_saddr_ram_base(saddr_ram_base)
{
}

u32 upd78k_family_disassembler::opcode_alignment() const
{
	return 1;
}

void upd78k_family_disassembler::format_imm8(std::ostream &stream, u8 d)
{
	if (d < 0xa0)
		util::stream_format(stream, "#%02XH", d);
	else
		util::stream_format(stream, "#0%02XH", d);
}

void upd78k_family_disassembler::format_imm16(std::ostream &stream, u16 d)
{
	if (d < 0xa000)
		util::stream_format(stream, "#%04XH", d);
	else
		util::stream_format(stream, "#0%04XH", d);
}

void upd78k_family_disassembler::format_ix_disp8(std::ostream &stream, const char *r, u8 d)
{
	if (d < 0xa0)
		util::stream_format(stream, "[%s+%02XH]", r, d);
	else
		util::stream_format(stream, "[%s+0%02XH]", r, d);
}

void upd78k_family_disassembler::format_ix_disp16(std::ostream &stream, const char *r, u16 d)
{
	if (d > 0xff00) // assume these are effectively small negative offsets
	{
		stream << "-";
		d = 0x10000 - d;
	}
	if (d < 0x000a)
		util::stream_format(stream, "%d[%s]", d, r);
	else
	{
		if (d < 0x0010 || d >= (d < 0x0100 ? 0x00a0 : d < 0x1000 ? 0x0a00 : 0xa000))
			stream << "0";
		util::stream_format(stream, "%XH[%s]", d, r);
	}
}

void upd78k_family_disassembler::format_ix_base16(std::ostream &stream, const char *r, u16 d)
{
	if (d >= 0xa000)
		stream << "0";
	util::stream_format(stream, "%04XH[%s]", d, r);
}

void upd78k_family_disassembler::format_abs16(std::ostream &stream, u16 addr)
{
	if (addr < 0xa000)
		util::stream_format(stream, "!%04XH", addr);
	else
		util::stream_format(stream, "!0%04XH", addr);
}

void upd78k_family_disassembler::format_jdisp8(std::ostream &stream, offs_t pc, u8 disp)
{
	u16 addr = pc + s8(disp);
	if (addr < 0xa000)
		util::stream_format(stream, "$%04XH", addr);
	else
		util::stream_format(stream, "$0%04XH", addr);
}

void upd78k_family_disassembler::format_sfr(std::ostream &stream, u8 addr)
{
	if (m_sfr_names[addr] != nullptr)
		stream << m_sfr_names[addr];
	else
		util::stream_format(stream, "0%04XH", 0xff00 + addr);
}

void upd78k_family_disassembler::format_saddr(std::ostream &stream, u8 addr)
{
	if (addr < 0x20)
		format_sfr(stream, addr);
	else
		util::stream_format(stream, "0%04XH", m_saddr_ram_base + addr);
}

void upd78k_family_disassembler::format_sfrp(std::ostream &stream, u8 addr)
{
	if (!BIT(addr, 0) && m_sfrp_names[addr >> 1] != nullptr)
		stream << m_sfrp_names[addr >> 1];
	else
		util::stream_format(stream, "0%04XH", 0xff00 + addr);
}

void upd78k_family_disassembler::format_count(std::ostream &stream, u8 n)
{
	if (n < 0x0a)
		util::stream_format(stream, "%d", n);
	else
	{
		if (n >= 0x0a)
			stream << "0";
		util::stream_format(stream, "%02XH", n);
	}
}

void upd78k_family_disassembler::format_saddrp(std::ostream &stream, u8 addr)
{
	if (addr < 0x20)
		format_sfrp(stream, addr);
	else
		util::stream_format(stream, "0%04XH", 0xfe00 + addr);
}

offs_t upd78k_family_disassembler::dasm_illegal(std::ostream &stream, u8 op)
{
	if (op < 0xa0)
		util::stream_format(stream, "%-8s%02XH", "DB", op);
	else
		util::stream_format(stream, "%-8s0%02XH", "DB", op);
	return 1 | SUPPORTED;
}

offs_t upd78k_family_disassembler::dasm_illegal2(std::ostream &stream, u8 op1, u8 op2)
{
	if (op2 < 0xa0)
		util::stream_format(stream, "%-8s%02XH,%02XH", "DB", op1, op2);
	else
		util::stream_format(stream, "%-8s%02XH,0%02XH", "DB", op1, op2);
	return 2 | SUPPORTED;
}

offs_t upd78k_family_disassembler::dasm_illegal3(std::ostream &stream, u8 op1, u8 op2, u8 op3)
{
	if (op3 < 0xa0)
		util::stream_format(stream, "%-8s%02XH,%02XH,%02XH", "DB", op1, op2, op3);
	else
		util::stream_format(stream, "%-8s%02XH,%02XH,0%02XH", "DB", op1, op2, op3);
	return 3 | SUPPORTED;
}

// For families with 4 banks of 8 registers and only one PSW byte
upd78k_8reg_disassembler::upd78k_8reg_disassembler(const char *const sfr_names[], const char *const sfrp_names[])
	: upd78k_family_disassembler(sfr_names, sfrp_names, 0xfe00)
{
}

const char *const upd78k_8reg_disassembler::s_r_names[8] =
{
	"X",
	"A",
	"C",
	"B",
	"E",
	"D",
	"L",
	"H"
};

const char *const upd78k_8reg_disassembler::s_rp_names[4] =
{
	"AX",
	"BC",
	"DE",
	"HL"
};

const char *const upd78k_8reg_disassembler::s_psw_bits[8] =
{
	"CY",
	"ISP",
	"PSW.2",
	"RBS0",
	"AC",
	"RBS1",
	"Z",
	"IE"
};
