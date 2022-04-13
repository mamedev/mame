// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    National Semiconductor HPC disassembler

    Note that though all 16-bit fields in instructions have the MSB first,
    the HPC's memory organization is in fact little-endian (including
    vector and JIDW tables). This is why r16 is always swapped here.

***************************************************************************/

#include "emu.h"
#include "hpcdasm.h"

#include <cctype>

const char *const hpc16083_disassembler::s_regs[128] =
{
	"psw", nullptr, "SP", "PC", "A", "K", "B", "X",
	"enir", "irpd", "ircd", "sio", "porti", nullptr, "halten", nullptr,
	"porta", "portb", nullptr, "upic", nullptr, nullptr, nullptr, nullptr,
	"dira", "dirb", "bfun", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, "portd", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"enu", "enui", "rbuf", "tbuf", "enur", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"t4", "r4", "t5", "r5", "t6", "r6", "t7", "r7",
	"pwmode", "portp", nullptr, nullptr, nullptr, nullptr, "eicon", "eicr",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"i4cr", "i3cr", "i2cr", "r2", "t2", "r3", "t3", "divby",
	"tmmode", "t0con", "watchdog", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const hpc16164_disassembler::s_regs[128] =
{
	"psw", nullptr, "SP", "PC", "A", "K", "B", "X",
	"enir", "irpd", "ircd", "sio", "porti", nullptr, "halten", "romdump",
	"porta", "portb", nullptr, "upic", nullptr, nullptr, nullptr, nullptr,
	"dira", "dirb", "bfun", nullptr, nullptr, nullptr, nullptr, nullptr,
	"adcr1", "adcr2", "portd", "adcr3", nullptr, nullptr, nullptr, nullptr,
	"ad0", "ad1", "ad2", "ad3", "ad4", "ad5", "ad6", "ad7",
	"enu", "enui", "rbuf", "tbuf", "enur", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"t4", "r4", "t5", "r5", "t6", "r6", "t7", "r7",
	"pwmode", "portp", nullptr, nullptr, nullptr, nullptr, "eicon", "eicr",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"i4cr", "i3cr", "i2cr", "r2", "t2", "r3", "t3", "divby",
	"tmmode", "t0con", "watchdog", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

hpc_disassembler::hpc_disassembler(const char *const regs[])
	: util::disasm_interface()
	, m_regs(regs)
{
}

u32 hpc_disassembler::opcode_alignment() const
{
	return 1;
}

void hpc_disassembler::format_register(std::ostream &stream, u16 reg) const
{
	if (reg >= 0x00c0 && reg < 0x01c0)
	{
		const char *name = m_regs[(reg - 0x00c0) >> 1];
		if (name != nullptr)
		{
			stream << name;
			if (BIT(reg, 0))
				stream << "+1";
			return;
		}
	}

	util::stream_format(stream, "0%X", reg);
}

void hpc_disassembler::format_immediate_byte(std::ostream &stream, u8 data) const
{
	stream << "#";
	if (data >= 0x10)
		stream << "0";
	util::stream_format(stream, "%02X", data);
}

void hpc_disassembler::format_immediate_word(std::ostream &stream, u16 data) const
{
	stream << "#";
	if (data >= 0x1000)
		stream << "0";
	util::stream_format(stream, "%04X", data);
}

void hpc_disassembler::disassemble_op(std::ostream &stream, const char *op, u16 reg, u16 src, bool imm, bool indir, bool idx, bool w) const
{
	util::stream_format(stream, "%-8s", op);
	if (idx)
		stream << "A";
	else
		format_register(stream, reg);
	stream << ",";
	if (imm)
	{
		if (w)
			format_immediate_word(stream, src);
		else
			format_immediate_byte(stream, src);
	}
	else
	{
		if (idx)
			util::stream_format(stream, "0%X", u16(reg));

		if (indir)
			stream << "[";
		format_register(stream, src);
		if (indir)
			stream << "]";

		if (w)
			stream << ".w";
		else
			stream << ".b";
	}
}

void hpc_disassembler::disassemble_unary_op(std::ostream &stream, const char *op, u16 offset, u16 src, bool indir, bool idx, bool w) const
{
	util::stream_format(stream, "%-8s", op);
	if (idx)
		util::stream_format(stream, "0%X", offset);

	if (indir)
		stream << "[";
	format_register(stream, src);
	if (indir)
		stream << "]";

	if (w)
	{
		if (src < 0x00c4 || src >= 0x00d0 || indir)
			stream << ".w";
	}
	else
		stream << ".b";
}

void hpc_disassembler::disassemble_bit_op(std::ostream &stream, const char *op, u8 bit, u16 offset, u16 src, bool indir, bool idx) const
{
	if (src >= 0x00c0 && src < 0x01c0 && BIT(src, 0) && !indir && m_regs[(src - 0x00c0) >> 1] != nullptr)
	{
		src &= 0xfffe;
		bit += 8;
	}

	util::stream_format(stream, "%-8s%d,", op, bit);

	if (idx)
		util::stream_format(stream, "0%X", offset);

	if (indir)
		stream << "[";
	format_register(stream, src);
	if (indir)
		stream << "].b";
}

offs_t hpc_disassembler::disassemble(std::ostream &stream, offs_t pc, const hpc_disassembler::data_buffer &opcodes, const hpc_disassembler::data_buffer &params)
{
	u8 opcode = opcodes.r8(pc);
	u16 reg = REGISTER_A;
	u16 src = REGISTER_B;
	bool imm = false;
	bool dmode = false;
	bool indir = true;
	bool idx = false;
	bool jmp = false;
	offs_t bytes = 1;

	switch (opcode)
	{
	case 0x20: case 0x21: case 0x22: case 0x23:
	case 0x24: case 0x25: case 0x26: case 0x27:
	case 0x28: case 0x29: case 0x2a: case 0x2b:
	case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		jmp = true;
		src = 0xffd0 + (opcode & 0x0f) * 2;
		bytes = 1;
		break;

	case 0x30: case 0x31: case 0x32: case 0x33:
		jmp = true;
		src = pc + 2 + ((opcode & 0x03) << 8 | opcodes.r8(pc + 1));
		bytes = 2;
		break;

	case 0x34: case 0x35: case 0x36: case 0x37:
		jmp = true;
		src = pc - ((opcode & 0x03) << 8 | opcodes.r8(pc + 1));
		bytes = 2;
		break;

	case 0x38: case 0x39: case 0x3a:
		reg = REGISTER_X;
		break;

	case 0x3f:
	case 0x89: case 0x8a:
	case 0xa9: case 0xaa: case 0xaf:
		indir = false;
		src = opcodes.r8(pc + 1);
		bytes = 2;
		break;

	case 0x40: case 0x41: case 0x42: case 0x43:
	case 0x44: case 0x45: case 0x46: case 0x47:
	case 0x48: case 0x49: case 0x4a: case 0x4b:
	case 0x4c: case 0x4d: case 0x4e: case 0x4f:
	case 0x50: case 0x51: case 0x52: case 0x53:
	case 0x54: case 0x55: case 0x56: case 0x57:
	case 0x58: case 0x59: case 0x5a: case 0x5b:
	case 0x5c: case 0x5d: case 0x5e: case 0x5f:
		jmp = true;
		src = pc + 1 + (opcode & 0x1f);
		break;

	case 0x60: case 0x61: case 0x62: case 0x63:
	case 0x64: case 0x65: case 0x66: case 0x67:
	case 0x68: case 0x69: case 0x6a: case 0x6b:
	case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	case 0x70: case 0x71: case 0x72: case 0x73:
	case 0x74: case 0x75: case 0x76: case 0x77:
	case 0x78: case 0x79: case 0x7a: case 0x7b:
	case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		jmp = true;
		src = pc - (opcode & 0x1f);
		break;

	case 0x80: case 0x82:
	case 0xa0:
		dmode = true;
		indir = false;
		if (BIT(opcode, 1))
			imm = true;

		src = opcodes.r8(pc + 1);
		reg = opcodes.r8(pc + 2);
		opcode = opcodes.r8(pc + 3);
		bytes = 4;
		break;

	case 0x84: case 0x86:
	case 0xa4:
		dmode = true;
		indir = false;
		if (BIT(opcode, 1))
			imm = true;

		src = swapendian_int16(opcodes.r16(pc + 1));
		reg = opcodes.r8(pc + 3);
		opcode = opcodes.r8(pc + 4);
		bytes = 5;
		break;

	case 0x81: case 0x83:
	case 0xa1:
		dmode = true;
		indir = false;
		if (BIT(opcode, 1))
			imm = true;

		src = opcodes.r8(pc + 1);
		reg = swapendian_int16(opcodes.r16(pc + 2));
		opcode = opcodes.r8(pc + 4);
		bytes = 5;
		break;

	case 0xa2:
		idx = true;
		reg = opcodes.r8(pc + 1);
		src = opcodes.r8(pc + 2);
		opcode = opcodes.r8(pc + 3);
		bytes = 4;
		break;

	case 0x85: case 0x87:
	case 0xa5:
		dmode = true;
		indir = false;
		if (BIT(opcode, 1))
			imm = true;

		src = swapendian_int16(opcodes.r16(pc + 1));
		reg = swapendian_int16(opcodes.r16(pc + 3));
		opcode = opcodes.r8(pc + 5);
		bytes = 6;
		break;

	case 0xa6:
		idx = true;
		reg = swapendian_int16(opcodes.r16(pc + 1));
		src = opcodes.r8(pc + 3);
		opcode = opcodes.r8(pc + 4);
		bytes = 5;
		break;

	case 0x88: case 0x8b: case 0x8e:
	case 0xa8: case 0xab: case 0xae:
		indir = false;
		src = opcodes.r8(pc + 1);
		bytes = 2;
		break;

	case 0x8c:
	case 0xac:
		indir = false;
		src = opcodes.r8(pc + 1);
		reg = opcodes.r8(pc + 2);
		bytes = 3;
		break;

	case 0x8d:
		imm = true;
		src = opcodes.r8(pc + 1);
		reg = opcodes.r8(pc + 2);
		bytes = 3;
		break;

	case 0x8f:
		src = REGISTER_X;
		opcode = opcodes.r8(pc + 1);
		bytes = 2;
		break;

	case 0x90: case 0x91: case 0x92: case 0x93:
		imm = true;
		reg = 0x00c8 | ((opcode & 0x03) << 1);
		src = opcodes.r8(pc + 1);
		bytes = 2;
		break;

	case 0x98: case 0x99: case 0x9a: case 0x9b:
	case 0x9c: case 0x9d: case 0x9e: case 0x9f:
		imm = true;
		src = opcodes.r8(pc + 1);
		bytes = 2;
		break;

	case 0x94:
		jmp = true;
		src = pc + 2 + opcodes.r8(pc + 1);
		bytes = 2;
		break;

	case 0x95:
		jmp = true;
		src = pc - opcodes.r8(pc + 1);
		bytes = 2;
		break;

	case 0x96:
		indir = false;
		src = opcodes.r8(pc + 1);
		opcode = opcodes.r8(pc + 2);
		bytes = 3;
		break;

	case 0x97:
		indir = false;
		imm = true;
		src = opcodes.r8(pc + 1);
		reg = opcodes.r8(pc + 2);
		bytes = 3;
		break;

	case 0xa7:
		imm = true;
		src = swapendian_int16(opcodes.r16(pc + 1));
		reg = swapendian_int16(opcodes.r16(pc + 3));
		bytes = 5;
		break;

	case 0xad:
		src = opcodes.r8(pc + 1);
		opcode = opcodes.r8(pc + 2);
		bytes = 3;
		break;

	case 0xb0: case 0xb1: case 0xb2: case 0xb3:
		imm = true;
		reg = 0x00c8 | ((opcode & 0x03) << 1);
		src = swapendian_int16(opcodes.r16(pc + 1));
		bytes = 3;
		break;

	case 0xb8: case 0xb9: case 0xba: case 0xbb:
	case 0xbc: case 0xbd: case 0xbe: case 0xbf:
		imm = true;
		src = swapendian_int16(opcodes.r16(pc + 1));
		bytes = 3;
		break;

	case 0xb4: case 0xb5:
		jmp = true;
		src = pc + 3 + swapendian_int16(opcodes.r16(pc + 1));
		bytes = 3;
		break;

	case 0xb6:
		indir = false;
		src = swapendian_int16(opcodes.r16(pc + 1));
		opcode = opcodes.r8(pc + 3);
		bytes = 4;
		break;

	case 0xb7:
		imm = true;
		src = swapendian_int16(opcodes.r16(pc + 1));
		reg = opcodes.r8(pc + 3);
		bytes = 4;
		break;

	case 0xd4: case 0xd5: case 0xd6:
	case 0xf4: case 0xf5: case 0xf6:
		src = REGISTER_X;
		break;
	}

	switch (opcode)
	{
	case 0x00:
		util::stream_format(stream, "%-8sA", "clr");
		break;

	case 0x01:
		util::stream_format(stream, "%-8sA", "comp");
		break;

	case 0x02:
		stream << "sc";
		break;

	case 0x03:
		stream << "rc";
		break;

	case 0x04:
		util::stream_format(stream, "%-8sA", "inc");
		break;

	case 0x05:
		util::stream_format(stream, "%-8sA", "dec");
		break;

	case 0x06:
		stream << "ifnc";
		bytes |= STEP_COND;
		break;

	case 0x07:
		stream << "ifc";
		bytes |= STEP_COND;
		break;

	case 0x08: case 0x09: case 0x0a: case 0x0b:
	case 0x0c: case 0x0d: case 0x0e: case 0x0f:
		disassemble_bit_op(stream, "sbit", opcode & 0x07, reg, src, indir, idx);
		break;

	case 0x10: case 0x11: case 0x12: case 0x13:
	case 0x14: case 0x15: case 0x16: case 0x17:
		disassemble_bit_op(stream, "ifbit", opcode & 0x07, reg, src, indir, idx);
		bytes |= STEP_COND;
		break;

	case 0x18: case 0x19: case 0x1a: case 0x1b:
	case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		disassemble_bit_op(stream, "rbit", opcode & 0x07, reg, src, indir, idx);
		break;

	case 0x20: case 0x21: case 0x22: case 0x23:
	case 0x24: case 0x25: case 0x26: case 0x27:
	case 0x28: case 0x29: case 0x2a: case 0x2b:
	case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		util::stream_format(stream, "%-8s", "jsrp");
		if (jmp)
			util::stream_format(stream, "[0%X]", src);
		else
			stream << "???";
		bytes |= STEP_OVER;
		break;

	case 0x30: case 0x31: case 0x32: case 0x33:
	case 0x34: case 0x35: case 0x36: case 0x37:
		util::stream_format(stream, "%-8s", "jsr");
		if (jmp)
			util::stream_format(stream, "0%X", src);
		else
			stream << "???";
		bytes |= STEP_OVER;
		break;

	case 0x38:
		disassemble_op(stream, "rbit", reg, src, imm, indir, idx, false);
		break;

	case 0x39:
		disassemble_op(stream, "sbit", reg, src, imm, indir, idx, false);
		break;

	case 0x3a:
		disassemble_op(stream, "ifbit", reg, src, imm, indir, idx, false);
		bytes |= STEP_COND;
		break;

	case 0x3b:
		util::stream_format(stream, "%-8sA", "swap");
		break;

	case 0x3c:
		stream << "ret";
		bytes |= STEP_OUT;
		break;

	case 0x3d:
		stream << "retsk";
		bytes |= STEP_OUT;
		break;

	case 0x3e:
		stream << "reti";
		bytes |= STEP_OUT;
		break;

	case 0x3f:
		util::stream_format(stream, "%-8s", "pop");
		format_register(stream, src);
		break;

	case 0x40:
		stream << "nop";
		break;

	case 0x41: case 0x42: case 0x43:
	case 0x44: case 0x45: case 0x46: case 0x47:
	case 0x48: case 0x49: case 0x4a: case 0x4b:
	case 0x4c: case 0x4d: case 0x4e: case 0x4f:
	case 0x50: case 0x51: case 0x52: case 0x53:
	case 0x54: case 0x55: case 0x56: case 0x57:
	case 0x58: case 0x59: case 0x5a: case 0x5b:
	case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	case 0x60: case 0x61: case 0x62: case 0x63:
	case 0x64: case 0x65: case 0x66: case 0x67:
	case 0x68: case 0x69: case 0x6a: case 0x6b:
	case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	case 0x70: case 0x71: case 0x72: case 0x73:
	case 0x74: case 0x75: case 0x76: case 0x77:
	case 0x78: case 0x79: case 0x7a: case 0x7b:
	case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		util::stream_format(stream, "%-8s", "jp");
		if (jmp)
			util::stream_format(stream, "0%X", src);
		else
			stream << "???";
		break;

	case 0x88: case 0x8c:
	case 0x90: case 0x91: case 0x92: case 0x93:
	case 0x97:
	case 0xa8: case 0xac:
	case 0xb0: case 0xb1: case 0xb2: case 0xb3:
	case 0xb7:
	case 0xc4:
	case 0xd4:
	case 0xe4:
	case 0xf4:
		disassemble_op(stream, "ld", reg, src, imm, indir, idx, BIT(opcode, 5));
		break;

	case 0x89:
	case 0xa9:
		disassemble_unary_op(stream, "inc", reg, src, indir, idx, BIT(opcode, 5));
		break;

	case 0x8a:
	case 0xaa:
		disassemble_unary_op(stream, "decsz", reg, src, indir, idx, BIT(opcode, 5));
		bytes |= STEP_COND;
		break;

	case 0x8b:
	case 0xab:
	case 0xc6:
	case 0xd6:
	case 0xe6:
	case 0xf6:
		disassemble_op(stream, dmode ? "ld" : "st", reg, src, imm, indir, idx, BIT(opcode, 5));
		break;

	case 0x8d:
		util::stream_format(stream, "%-8sBK,", "ld");
		format_immediate_byte(stream, src);
		stream << ",";
		format_immediate_byte(stream, reg);
		break;

	case 0x8e:
	case 0xae:
	case 0xc5:
	case 0xd5:
	case 0xe5:
	case 0xf5:
		disassemble_op(stream, "x", reg, src, imm, indir, idx, BIT(opcode, 5));
		break;

	case 0x94: case 0x95:
		util::stream_format(stream, "%-8s", "jp");
		if (jmp)
			util::stream_format(stream, "0%X", src);
		else
			stream << "???";
		break;

	case 0x98: case 0xb8: case 0xd8: case 0xf8:
		disassemble_op(stream, "add", reg, src, imm, indir, idx, BIT(opcode, 5));
		break;

	case 0x99: case 0xb9: case 0xd9: case 0xf9:
		disassemble_op(stream, "and", reg, src, imm, indir, idx, BIT(opcode, 5));
		break;

	case 0x9a: case 0xba: case 0xda: case 0xfa:
		disassemble_op(stream, "or", reg, src, imm, indir, idx, BIT(opcode, 5));
		break;

	case 0x9b: case 0xbb: case 0xdb: case 0xfb:
		disassemble_op(stream, "xor", reg, src, imm, indir, idx, BIT(opcode, 5));
		break;

	case 0x9c: case 0xbc: case 0xdc: case 0xfc:
		disassemble_op(stream, "ifeq", reg, src, imm, indir, idx, BIT(opcode, 5));
		bytes |= STEP_COND;
		break;

	case 0x9d: case 0xbd: case 0xdd: case 0xfd:
		disassemble_op(stream, "ifgt", reg, src, imm, indir, idx, BIT(opcode, 5));
		bytes |= STEP_COND;
		break;

	case 0x9e: case 0xbe: case 0xde: case 0xfe:
		disassemble_op(stream, "mult", reg, src, imm, indir, idx, BIT(opcode, 5));
		break;

	case 0x9f: case 0xbf: case 0xdf: case 0xff:
		disassemble_op(stream, "div", reg, src, imm, indir, idx, BIT(opcode, 5));
		break;

	case 0xa7:
		util::stream_format(stream, "%-8sBK,", "ld");
		format_immediate_word(stream, src);
		stream << ",";
		format_immediate_word(stream, reg);
		break;

	case 0xaf:
		util::stream_format(stream, "%-8s", "push");
		format_register(stream, src);
		break;

	case 0xb4:
		util::stream_format(stream, "%-8s", "jmpl");
		if (jmp)
			util::stream_format(stream, "0%X", src);
		else
			stream << "???";
		break;

	case 0xb5:
		util::stream_format(stream, "%-8s", "jsrl");
		if (jmp)
			util::stream_format(stream, "0%X", src);
		else
			stream << "???";
		bytes |= STEP_OVER;
		break;

	case 0xc0: case 0xc2:
	case 0xe0: case 0xe2:
		util::stream_format(stream, "%-8sA,[B%c].%c", "lds",
			BIT(opcode, 1) ? '-' : '+',
			BIT(opcode, 5) ? 'w' : 'b');
		bytes |= STEP_COND;
		break;

	case 0xd0: case 0xd2:
	case 0xf0: case 0xf2:
		util::stream_format(stream, "%-8sA,[X%c].%c", "ld",
			BIT(opcode, 1) ? '-' : '+',
			BIT(opcode, 5) ? 'w' : 'b');
		break;

	case 0xc1: case 0xc3:
	case 0xe1: case 0xe3:
		util::stream_format(stream, "%-8sA,[B%c].%c", "xs",
			BIT(opcode, 1) ? '-' : '+',
			BIT(opcode, 5) ? 'w' : 'b');
		bytes |= STEP_COND;
		break;

	case 0xd1: case 0xd3:
	case 0xf1: case 0xf3:
		util::stream_format(stream, "%-8sA,[X%c].%c", "x",
			BIT(opcode, 1) ? '-' : '+',
			BIT(opcode, 5) ? 'w' : 'b');
		break;

	case 0xc7: case 0xe7:
		util::stream_format(stream, "%-8sA", BIT(opcode, 5) ? "shl" : "shr");
		break;

	case 0xd7: case 0xf7:
		util::stream_format(stream, "%-8sA", BIT(opcode, 5) ? "rlc" : "rrc");
		break;

	case 0xc8: case 0xe8:
		disassemble_op(stream, "adc", reg, src, imm, indir, idx, BIT(opcode, 5));
		break;

	case 0xc9: case 0xe9:
		disassemble_op(stream, "dadc", reg, src, imm, indir, idx, BIT(opcode, 5));
		break;

	case 0xca: case 0xea:
		disassemble_op(stream, "dsubc", reg, src, imm, indir, idx, BIT(opcode, 5));
		break;

	case 0xcb: case 0xeb:
		disassemble_op(stream, "subc", reg, src, imm, indir, idx, BIT(opcode, 5));
		break;

	case 0xcc: case 0xec:
		stream << "jid";
		if (BIT(opcode, 5))
			stream << "w";
		break;

	case 0xcf: case 0xef:
		disassemble_op(stream, "divd", reg, src, imm, indir, idx, BIT(opcode, 5));
		break;

	default:
		stream << "???";
		break;
	}

	return bytes | SUPPORTED;
}
