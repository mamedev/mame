// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
	TS-Configuration Copper disassembler

    Instruction format (16-bit words, cf. pentevo/fpga/current/common/copper.v):
      WREG #rr, #vv  [15:12] != 0xF
      F5nn SIGNAL (bit0 SIG_INT, bit1 SIG_RDY)
      F6nn SETA
      F7nn WAITX
      F8/F9nn SETB (9-bit)
      FA/FBnn WAITY (9-bit; FBFn = WAIT evt)
      FBFn WAIT evt (0 EVT_NONE, 1 EVT_DMA, 2 EVT_XY0, 3 EVT_X0)
      FCnn DJNZA
      FDnn DJNZB
      FEnn CALL
      FFnn JUMP (FF = RET)
    Labels (WREG register names, EVT_*, SIG_*) are adopted from
    pentevo/tools/copper/copper_asm.py.
**********************************************************************/

#include "emu.h"
#include "tsconf_copper_dasm.h"

namespace {

struct reg_name { u8 num; const char *name; };

const reg_name reg_names[] {
	{0x00, "VCONFIG"}, {0x01, "VPAGE"},
	{0x02, "GXOFFSL"}, {0x03, "GXOFFSH"}, {0x04, "GYOFFSL"}, {0x05, "GYOFFSH"},
	{0x06, "TSCONFIG"}, {0x07, "PALSEL"}, {0x0f, "BORDER"},
	{0x10, "PAGE0"}, {0x11, "PAGE1"}, {0x12, "PAGE2"}, {0x13, "PAGE3"},
	{0x15, "FMADDR"}, {0x16, "TMPAGE"}, {0x17, "T0GPAGE"}, {0x18, "T1GPAGE"}, {0x19, "SGPAGE"},
	{0x1a, "DMASADDRL"}, {0x1b, "DMASADDRH"}, {0x1c, "DMASADDRX"},
	{0x1d, "DMADADDRL"}, {0x1e, "DMADADDRH"}, {0x1f, "DMADADDRX"},
	{0x20, "SYSCONFIG"}, {0x21, "MEMCONFIG"}, {0x22, "HSINT"}, {0x23, "VSINTL"}, {0x24, "VSINTH"},
	{0x25, "DMAWPD"}, {0x26, "DMALEN"}, {0x27, "DMACTR"}, {0x28, "DMANUM"},
	{0x29, "FDDVIRT"}, {0x2a, "INTMASK"}, {0x2b, "CACHECONF"}, {0x2c, "DMANUMH"}, {0x2d, "DMAWPA"},
	{0x40, "T0XOFFSL"}, {0x41, "T0XOFFSH"}, {0x42, "T0YOFFSL"}, {0x43, "T0YOFFSH"},
	{0x44, "T1XOFFSL"}, {0x45, "T1XOFFSH"}, {0x46, "T1YOFFSL"}, {0x47, "T1YOFFSH"},
};

const char *wreg_name(u8 num)
{
	for (const auto &r : reg_names)
		if (r.num == num)
			return r.name;
	return nullptr;
}

} // anonymous namespace

u32 tsconf_copper_disassembler::opcode_alignment() const
{
	return 1;
}

#define P std::ostream &stream, const data_buffer &opcodes, u16 opcode, u16 pc
const tsconf_copper_disassembler::instruction tsconf_copper_disassembler::instructions[] {
	// SIGNAL: bit0 = SIG_INT, bit1 = SIG_RDY
	{ 0xf500, 0xff00, [](P) -> u32 {
			std::string sig;
			if (BIT(opcode, 0)) sig += "SIG_INT";
			if (BIT(opcode, 1)) sig += sig.empty() ? "SIG_RDY" : "|SIG_RDY";
			util::stream_format(stream, "SIGNAL %s", sig.empty() ? "0" : sig);
			return 1; } },

	// SETA a
	{ 0xf600, 0xff00, [](P) -> u32 { util::stream_format(stream, "SETA %d", opcode & 0xff); return 1; } },

	// WAITX x
	{ 0xf700, 0xff00, [](P) -> u32 { util::stream_format(stream, "WAITX %d", opcode & 0xff); return 1; } },

	// WAIT evt (must precede WAITY: FBFn overlaps its opcode space)
	{ 0xfbf0, 0xfff0, [](P) -> u32 {
			switch (opcode & 0x0f)
			{
			case 0:  util::stream_format(stream, "NOP"); break; // WAIT EVT_NONE
			case 1:  util::stream_format(stream, "WAIT EVT_DMA"); break;
			case 2:  util::stream_format(stream, "WAIT EVT_XY0"); break;
			case 3:  util::stream_format(stream, "WAIT EVT_X0"); break;
			default: util::stream_format(stream, "WAIT %d", opcode & 0x0f); break;
			}
			return 1; } },

	// SETB b (9-bit)
	{ 0xf800, 0xfe00, [](P) -> u32 { util::stream_format(stream, "SETB %d", opcode & 0x1ff); return 1; } },

	// WAITY y (9-bit)
	{ 0xfa00, 0xfe00, [](P) -> u32 { util::stream_format(stream, "WAITY %d", opcode & 0x1ff); return 1; } },

	// DJNZA / DJNZB
	{ 0xfc00, 0xff00, [](P) -> u32 { util::stream_format(stream, "DJNZA $%02x", opcode & 0xff); return 1; } },
	{ 0xfd00, 0xff00, [](P) -> u32 { util::stream_format(stream, "DJNZB $%02x", opcode & 0xff); return 1; } },

	// CALL
	{ 0xfe00, 0xff00, [](P) -> u32 { util::stream_format(stream, "CALL $%02x", opcode & 0xff); return 1; } },

	// JUMP (FF = RET, self = HALT)
	{ 0xff00, 0xff00, [](P) -> u32 {
			const u8 to = opcode & 0xff;
			if (to == 0xff)
				util::stream_format(stream, "RET");
			else if (to == pc)
				util::stream_format(stream, "HALT");
			else
				util::stream_format(stream, "JUMP $%02x", to);
			return 1; } },

	// Reserved 0xFxxx
	{ 0xf000, 0xf000, [](P) -> u32 { util::stream_format(stream, "?%04x", opcode); return 1; } },

	// Fallback: WREG #rr, #vv
	{ 0x0000, 0x0000, [](P) -> u32 {
			const u8 reg = (opcode >> 8) & 0xff;
			const char *name = wreg_name(reg);
			util::stream_format(stream, "WREG %s, $%02x", name ? name : util::string_format("$%02x", reg), opcode & 0xff);
			return 1; } },
};
#undef P

offs_t tsconf_copper_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 opcode = opcodes.r16(pc);

	for(u32 i=0;; i++)
		if((opcode & instructions[i].mask) == instructions[i].value)
			return instructions[i].cb(stream, opcodes, opcode, pc) | SUPPORTED;
	return 0;
}
