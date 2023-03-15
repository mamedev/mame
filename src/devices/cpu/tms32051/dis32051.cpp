// license:BSD-3-Clause
// copyright-holders:Ville Linde

#include "emu.h"
#include "dis32051.h"


const char *const tms32051_disassembler::zl_condition_codes[] =
{
	// bit 3, 2 (ZL), bit 1, 0 (MASK)
	"",             // Z=0, L=0, ZM=0, ZL=0
	"gt",           // Z=0, L=0, ZM=0, ZL=1
	"neq",          // Z=0, L=0, ZM=1, ZL=0
	"gt",           // Z=0, L=0, ZM=1, ZL=1
	"",             // Z=0, L=1, ZM=0, ZL=0
	"lt",           // Z=0, L=1, ZM=0, ZL=1
	"neq",          // Z=0, L=1, ZM=1, ZL=0
	"lt",           // Z=0, L=1, ZM=1, ZL=1
	"",             // Z=1, L=0, ZM=0, ZL=0
	"gt",           // Z=1, L=0, ZM=0, ZL=1
	"eq",           // Z=1, L=0, ZM=1, ZL=0
	"geq",          // Z=1, L=0, ZM=1, ZL=1
	"",             // Z=1, L=1, ZM=0, ZL=0
	"lt",           // Z=1, L=1, ZM=0, ZL=1
	"eq",           // Z=1, L=1, ZM=1, ZL=0
	"leq",          // Z=1, L=1, ZM=1, ZL=1
};

const char *const tms32051_disassembler::cv_condition_codes[16] =
{
	"",             // V=0, C=0, VM=0, CM=0
	"nc",           // V=0, C=0, VM=0, CM=1
	"nov",          // V=0, C=0, VM=1, CM=0
	"nc nov",       // V=0, C=0, VM=1, CM=1
	"",             // V=0, C=1, VM=0, CM=0
	"c",            // V=0, C=1, VM=0, CM=1
	"nov",          // V=0, C=1, VM=1, CM=0
	"c nov",        // V=0, C=1, VM=1, CM=1
	"",             // V=1, C=0, VM=0, CM=0
	"nc",           // V=1, C=0, VM=0, CM=1
	"ov",           // V=1, C=0, VM=1, CM=0
	"nc ov",        // V=1, C=0, VM=1, CM=1
	"",             // V=1, C=1, VM=0, CM=0
	"c",            // V=1, C=1, VM=0, CM=1
	"ov",           // V=1, C=1, VM=1, CM=0
	"c ov",         // V=1, C=1, VM=1, CM=1
};

const char *const tms32051_disassembler::tp_condition_codes[4] =
{
	"bio",
	"tc",
	"ntc",
	""
};

uint16_t tms32051_disassembler::FETCH(offs_t &npc, const data_buffer &opcodes)
{
	return opcodes.r16(npc++);
}

std::string tms32051_disassembler::GET_ADDRESS(int addr_mode, int address)
{
	if (addr_mode)      // Indirect addressing
	{
		int nar = address & 0x7;


		switch ((address >> 3) & 0xf)
		{
			case 0x0:   return util::string_format("*");
			case 0x1:   return util::string_format("*, ar%d", nar);
			case 0x2:   return util::string_format("*-");
			case 0x3:   return util::string_format("*-, ar%d", nar);
			case 0x4:   return util::string_format("*+");
			case 0x5:   return util::string_format("*+, ar%d", nar);
			case 0x8:   return util::string_format("*br0-");
			case 0x9:   return util::string_format("*br0-, ar%d", nar);
			case 0xa:   return util::string_format("*0-");
			case 0xb:   return util::string_format("*0-, ar%d", nar);
			case 0xc:   return util::string_format("*0+");
			case 0xd:   return util::string_format("*0+, ar%d", nar);
			case 0xe:   return util::string_format("*br0+");
			case 0xf:   return util::string_format("*br0+, ar%d", nar);

			default:    return util::string_format("??? (indirect)");
		}
	}
	else            // Direct addressing
	{
		return util::string_format("#%02X", address);
	}
}

std::string tms32051_disassembler::GET_SHIFT(int shift)
{
	if (shift > 0)
	{
		return util::string_format(", %d", shift);
	}
	else
	{
		return "";
	}
}

void tms32051_disassembler::print_condition_codes(bool pp, int zl, int cv, int tp)
{
	if (*(zl_condition_codes[zl]) != 0)
	{
		if (pp)
			util::stream_format(*output, ", ");
		util::stream_format(*output, "%s", zl_condition_codes[zl]);
		pp = true;
	}
	if (*(cv_condition_codes[cv]) != 0)
	{
		if (pp)
			util::stream_format(*output, ", ");
		util::stream_format(*output, "%s", cv_condition_codes[cv]);
		pp = true;
	}
	if (*(tp_condition_codes[tp]) != 0)
	{
		if (pp)
			util::stream_format(*output, ", ");
		util::stream_format(*output, "%s", tp_condition_codes[tp]);
	}
}

uint32_t tms32051_disassembler::dasm_group_be(uint16_t opcode, offs_t &npc, const data_buffer &opcodes)
{
	int subop = opcode & 0xff;
	uint32_t flags = 0;

	switch (subop)
	{
		case 0x00:  util::stream_format(*output, "abs"); break;
		case 0x01:  util::stream_format(*output, "cmpl"); break;
		case 0x02:  util::stream_format(*output, "neg"); break;
		case 0x03:  util::stream_format(*output, "pac"); break;
		case 0x04:  util::stream_format(*output, "apac"); break;
		case 0x05:  util::stream_format(*output, "spac"); break;
		case 0x09:  util::stream_format(*output, "sfl"); break;
		case 0x0a:  util::stream_format(*output, "sfr"); break;
		case 0x0c:  util::stream_format(*output, "rol"); break;
		case 0x0d:  util::stream_format(*output, "ror"); break;
		case 0x10:  util::stream_format(*output, "addb"); break;
		case 0x11:  util::stream_format(*output, "adcb"); break;
		case 0x12:  util::stream_format(*output, "andb"); break;
		case 0x13:  util::stream_format(*output, "orb"); break;
		case 0x14:  util::stream_format(*output, "rolb"); break;
		case 0x15:  util::stream_format(*output, "rorb"); break;
		case 0x16:  util::stream_format(*output, "sflb"); break;
		case 0x17:  util::stream_format(*output, "sfrb"); break;
		case 0x18:  util::stream_format(*output, "sbb"); break;
		case 0x19:  util::stream_format(*output, "sbbb"); break;
		case 0x1a:  util::stream_format(*output, "xorb"); break;
		case 0x1b:  util::stream_format(*output, "crgt"); break;
		case 0x1c:  util::stream_format(*output, "crlt"); break;
		case 0x1d:  util::stream_format(*output, "exar"); break;
		case 0x1e:  util::stream_format(*output, "sacb"); break;
		case 0x1f:  util::stream_format(*output, "lacb"); break;
		case 0x20:  util::stream_format(*output, "bacc"); break;
		case 0x21:  util::stream_format(*output, "baccd"); break;
		case 0x22:  util::stream_format(*output, "idle"); break;
		case 0x23:  util::stream_format(*output, "idle2"); break;
		case 0x30:  util::stream_format(*output, "cala"); break;
		case 0x32:  util::stream_format(*output, "pop"); break;
		case 0x38:  util::stream_format(*output, "reti"); break;
		case 0x3a:  util::stream_format(*output, "rete"); break;
		case 0x3c:  util::stream_format(*output, "push"); break;
		case 0x3d:  util::stream_format(*output, "calad"); break;
		case 0x40:  util::stream_format(*output, "clrc    intm"); break;
		case 0x41:  util::stream_format(*output, "setc    intm"); break;
		case 0x42:  util::stream_format(*output, "clrc    ovm"); break;
		case 0x43:  util::stream_format(*output, "setc    ovm"); break;
		case 0x44:  util::stream_format(*output, "clrc    cnf"); break;
		case 0x45:  util::stream_format(*output, "setc    cnf"); break;
		case 0x46:  util::stream_format(*output, "clrc    sxm"); break;
		case 0x47:  util::stream_format(*output, "setc    sxm"); break;
		case 0x48:  util::stream_format(*output, "clrc    hold"); break;
		case 0x49:  util::stream_format(*output, "setc    hold"); break;
		case 0x4a:  util::stream_format(*output, "clrc    tc"); break;
		case 0x4b:  util::stream_format(*output, "setc    tc"); break;
		case 0x4c:  util::stream_format(*output, "clrc    xf"); break;
		case 0x4d:  util::stream_format(*output, "setc    xf"); break;
		case 0x4e:  util::stream_format(*output, "clrc    carry"); break;
		case 0x4f:  util::stream_format(*output, "setc    carry"); break;
		case 0x51:  util::stream_format(*output, "trap"); flags = STEP_OVER; break;
		case 0x52:  util::stream_format(*output, "nmi"); flags = STEP_OVER; break;
		case 0x58:  util::stream_format(*output, "zpr"); break;
		case 0x59:  util::stream_format(*output, "zap"); break;
		case 0x5a:  util::stream_format(*output, "sath"); break;
		case 0x5b:  util::stream_format(*output, "satl"); break;
		case 0x60: case 0x61: case 0x62: case 0x63:
		case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b:
		case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		case 0x70: case 0x71: case 0x72: case 0x73:
		case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b:
		case 0x7c: case 0x7d: case 0x7e: case 0x7f:
					util::stream_format(*output, "intr    %d", opcode & 0x1f); flags = STEP_OVER; break;

		case 0x80:  util::stream_format(*output, "mpy     #%04X", FETCH(npc, opcodes)); break;
		case 0x81:  util::stream_format(*output, "and     #%04X", FETCH(npc, opcodes) << 16); break;
		case 0x82:  util::stream_format(*output, "or      #%04X", FETCH(npc, opcodes) << 16); break;
		case 0x83:  util::stream_format(*output, "xor     #%04X", FETCH(npc, opcodes) << 16); break;
		case 0xc4:  util::stream_format(*output, "rpt     #%04X", FETCH(npc, opcodes)); break;
		case 0xc5:  util::stream_format(*output, "rptz    #%04X", FETCH(npc, opcodes)); break;
		case 0xc6:  util::stream_format(*output, "rptb    #%04X", FETCH(npc, opcodes)); break;

		default:    util::stream_format(*output, "???     (group be)"); break;
	}
	return flags;
}

void tms32051_disassembler::dasm_group_bf(uint16_t opcode, offs_t &npc, const data_buffer &opcodes)
{
	int subop = (opcode >>  4) & 0xf;
	int shift = opcode & 0xf;

	switch (subop)
	{
		case 0x0:
		{
			if (opcode & 0x8)
			{
				util::stream_format(*output, "lar     ar%d, #%04X", opcode & 0x7, FETCH(npc, opcodes));
			}
			else
			{
				util::stream_format(*output, "spm     #%02X", opcode & 0x3);
			}
			break;
		}

		case 0x4:
		{
			switch (opcode & 0x3)
			{
				case 0: util::stream_format(*output, "cmpr    ar = arcr"); break;
				case 1: util::stream_format(*output, "cmpr    ar < arcr"); break;
				case 2: util::stream_format(*output, "cmpr    ar > arcr"); break;
				case 3: util::stream_format(*output, "cmpr    ar != arcr"); break;
			}
			break;
		}

		case 0x8:   util::stream_format(*output, "lacc    #%04X", FETCH(npc, opcodes) << shift); break;
		case 0x9:   util::stream_format(*output, "add     #%04X", FETCH(npc, opcodes) << shift); break;
		case 0xa:   util::stream_format(*output, "sub     #%04X", FETCH(npc, opcodes) << shift); break;
		case 0xb:   util::stream_format(*output, "and     #%04X", FETCH(npc, opcodes) << shift); break;
		case 0xc:   util::stream_format(*output, "or      #%04X", FETCH(npc, opcodes) << shift); break;
		case 0xd:   util::stream_format(*output, "xor     #%04X", FETCH(npc, opcodes) << shift); break;
		case 0xe:   util::stream_format(*output, "bsar    %d", shift+1); break;

		default:    util::stream_format(*output, "???     (group bf)"); break;
	}
}

u32 tms32051_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t tms32051_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint32_t flags = 0;
	uint16_t opcode;
	int baseop;
	int address, addr_mode;

	offs_t npc = pc;
	output = &stream;

	opcode = FETCH(npc, opcodes);
	baseop = (opcode >> 8) & 0xff;

	addr_mode = (opcode >> 7) & 0x1;
	address = opcode & 0x7f;

	switch (baseop)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
		{
			util::stream_format(*output, "lar     ar%d, %s", (opcode >> 8) & 0x7, GET_ADDRESS(addr_mode, address));
			break;
		}
		case 0x08:  util::stream_format(*output, "lamm    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x09:  util::stream_format(*output, "smmr    %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH(npc, opcodes)); break;
		case 0x0a:  util::stream_format(*output, "subc    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x0b:  util::stream_format(*output, "rpt     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x0c:  util::stream_format(*output, "out     %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH(npc, opcodes)); break;
		case 0x0d:  util::stream_format(*output, "ldp     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x0e:  util::stream_format(*output, "lst     0, %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x0f:  util::stream_format(*output, "lst     1, %s", GET_ADDRESS(addr_mode, address)); break;

		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		{
			util::stream_format(*output, "lacc    %s %s", GET_ADDRESS(addr_mode, address), GET_SHIFT((opcode >> 8) & 0xf));
			break;
		}

		case 0x20: case 0x21: case 0x22: case 0x23:
		case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		{
			util::stream_format(*output, "add     %s %s", GET_ADDRESS(addr_mode, address), GET_SHIFT((opcode >> 8) & 0xf));
			break;
		}

		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b:
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
		{
			util::stream_format(*output, "sub     %s %s", GET_ADDRESS(addr_mode, address), GET_SHIFT((opcode >> 8) & 0xf));
			break;
		}

		case 0x40: case 0x41: case 0x42: case 0x43:
		case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b:
		case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		{
			util::stream_format(*output, "bit     %d, %s", (opcode >> 8) & 0xf, GET_ADDRESS(addr_mode, address));
			break;
		}

		case 0x50:  util::stream_format(*output, "mpya    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x51:  util::stream_format(*output, "mpys    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x52:  util::stream_format(*output, "sqra    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x53:  util::stream_format(*output, "sqrs    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x54:  util::stream_format(*output, "mpy     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x55:  util::stream_format(*output, "mpyu    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x57:  util::stream_format(*output, "bldp    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x58:  util::stream_format(*output, "xpl     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x59:  util::stream_format(*output, "opl     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x5a:  util::stream_format(*output, "apl     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x5b:  util::stream_format(*output, "cpl     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x5c:  util::stream_format(*output, "xpl     %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH(npc, opcodes)); break;
		case 0x5d:  util::stream_format(*output, "opl     %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH(npc, opcodes)); break;
		case 0x5e:  util::stream_format(*output, "apl     %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH(npc, opcodes)); break;
		case 0x5f:  util::stream_format(*output, "cpl     %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH(npc, opcodes)); break;

		case 0x60:  util::stream_format(*output, "addc    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x61:  util::stream_format(*output, "add     %s << 16", GET_ADDRESS(addr_mode, address)); break;
		case 0x62:  util::stream_format(*output, "adds    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x63:  util::stream_format(*output, "addt    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x64:  util::stream_format(*output, "subb    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x65:  util::stream_format(*output, "sub     %s << 16", GET_ADDRESS(addr_mode, address)); break;
		case 0x66:  util::stream_format(*output, "subs    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x67:  util::stream_format(*output, "subt    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x68:  util::stream_format(*output, "zalr    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x69:  util::stream_format(*output, "lacl    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x6a:  util::stream_format(*output, "lacc    %s << 16", GET_ADDRESS(addr_mode, address)); break;
		case 0x6b:  util::stream_format(*output, "lact    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x6c:  util::stream_format(*output, "xor     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x6d:  util::stream_format(*output, "or      %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x6e:  util::stream_format(*output, "and     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x6f:  util::stream_format(*output, "bitt    %s", GET_ADDRESS(addr_mode, address)); break;

		case 0x70:  util::stream_format(*output, "lta     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x71:  util::stream_format(*output, "ltp     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x72:  util::stream_format(*output, "ltd     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x73:  util::stream_format(*output, "lt      %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x74:  util::stream_format(*output, "lts     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x75:  util::stream_format(*output, "lph     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x76:  util::stream_format(*output, "pshd    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x77:  util::stream_format(*output, "dmov    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x78:  util::stream_format(*output, "adrk    #%02X", opcode & 0xff); break;
		case 0x79:  util::stream_format(*output, "b       %04X, %s", FETCH(npc, opcodes), GET_ADDRESS(1, address)); break;
		case 0x7a:  util::stream_format(*output, "call    %04X, %s", FETCH(npc, opcodes), GET_ADDRESS(1, address)); flags = STEP_OVER; break;
		case 0x7b:  util::stream_format(*output, "banz    %04X, %s", FETCH(npc, opcodes), GET_ADDRESS(1, address)); flags = STEP_COND; break;
		case 0x7c:  util::stream_format(*output, "sbrk    #%02X", opcode & 0xff); break;
		case 0x7d:  util::stream_format(*output, "bd      %04X, %s", FETCH(npc, opcodes), GET_ADDRESS(1, address)); break;
		case 0x7e:  util::stream_format(*output, "calld   %04X, %s", FETCH(npc, opcodes), GET_ADDRESS(1, address)); flags = STEP_OVER | step_over_extra(1); break;
		case 0x7f:  util::stream_format(*output, "banzd   %04X, %s", FETCH(npc, opcodes), GET_ADDRESS(1, address)); flags = STEP_COND | step_over_extra(1); break;

		case 0x80: case 0x81: case 0x82: case 0x83:
		case 0x84: case 0x85: case 0x86: case 0x87:
		{
			util::stream_format(*output, "sar     ar%d, %s", (opcode >> 8) & 0x7, GET_ADDRESS(addr_mode, address));
			break;
		}
		case 0x88:  util::stream_format(*output, "samm    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x89:  util::stream_format(*output, "lmmr    %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH(npc, opcodes)); break;
		case 0x8a:  util::stream_format(*output, "popd    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x8b:
		{
			if ((opcode & 0xff) == 0)
			{
				util::stream_format(*output, "nop");
			}
			else
			{
				util::stream_format(*output, "mar     %s", GET_ADDRESS(addr_mode, address));
			}
			break;
		}
		case 0x8c:  util::stream_format(*output, "spl     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x8d:  util::stream_format(*output, "sph     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x8e:  util::stream_format(*output, "sst     0, %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x8f:  util::stream_format(*output, "sst     1, %s", GET_ADDRESS(addr_mode, address)); break;

		case 0x90: case 0x91: case 0x92: case 0x93:
		case 0x94: case 0x95: case 0x96: case 0x97:
		{
			util::stream_format(*output, "sacl    %s %s", GET_ADDRESS(addr_mode, address), GET_SHIFT((opcode >> 8) & 0x7));
			break;
		}
		case 0x98: case 0x99: case 0x9a: case 0x9b:
		case 0x9c: case 0x9d: case 0x9e: case 0x9f:
		{
			util::stream_format(*output, "sach    %s %s", GET_ADDRESS(addr_mode, address), GET_SHIFT((opcode >> 8) & 0x7));
			break;
		}

		case 0xa0:  util::stream_format(*output, "norm    %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH(npc, opcodes)); break;
		case 0xa2:  util::stream_format(*output, "mac     %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH(npc, opcodes)); break;
		case 0xa3:  util::stream_format(*output, "macd    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0xa4:  util::stream_format(*output, "blpd    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0xa5:  util::stream_format(*output, "blpd    %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH(npc, opcodes)); break;
		case 0xa6:  util::stream_format(*output, "tblr    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0xa7:  util::stream_format(*output, "tblw    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0xa8:  util::stream_format(*output, "bldd    %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH(npc, opcodes)); break;
		case 0xa9:  util::stream_format(*output, "bldd    %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH(npc, opcodes)); break;
		case 0xaa:  util::stream_format(*output, "mads    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0xab:  util::stream_format(*output, "madd    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0xac:  util::stream_format(*output, "bldd    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0xad:  util::stream_format(*output, "bldd    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0xae:  util::stream_format(*output, "splk    %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH(npc, opcodes)); break;
		case 0xaf:  util::stream_format(*output, "in      %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH(npc, opcodes)); break;

		case 0xb0: case 0xb1: case 0xb2: case 0xb3:
		case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		{
			util::stream_format(*output, "lar     ar%d, #%02X", (opcode >> 8) & 0x7, opcode & 0xff);
			break;
		}
		case 0xb8:  util::stream_format(*output, "add     #%02X", opcode & 0xff); break;
		case 0xb9:  util::stream_format(*output, "lacl    #%02X", opcode & 0xff); break;
		case 0xba:  util::stream_format(*output, "sub     #%02X", opcode & 0xff); break;
		case 0xbb:  util::stream_format(*output, "rpt     #%02X", opcode & 0xff); break;

		case 0xbc:
		case 0xbd:
		{
			util::stream_format(*output, "ldp     #%03X", opcode & 0x1ff);
			break;
		}
		case 0xbe:  flags = dasm_group_be(opcode, npc, opcodes); break;
		case 0xbf:  dasm_group_bf(opcode, npc, opcodes); break;

		case 0xe0: case 0xe1: case 0xe2: case 0xe3:
		{
			int zlcvmask = opcode & 0xf;
			int zlcv = (opcode >> 4) & 0xf;
			int zl = (zlcv & 0xc) | ((zlcvmask >> 2) & 0x3);
			int cv = ((zlcv << 2) & 0xc) | (zlcvmask & 0x3);
			int tp = (opcode >> 8) & 0x3;

			util::stream_format(*output, "bcnd    %04X", FETCH(npc, opcodes));
			print_condition_codes(true, zl, cv, tp);
			flags = STEP_COND;
			break;
		}

		case 0xe4: case 0xe5: case 0xe6: case 0xe7:
		case 0xf4: case 0xf5: case 0xf6: case 0xf7:
		{
			int zlcvmask = opcode & 0xf;
			int zlcv = (opcode >> 4) & 0xf;
			int zl = (zlcv & 0xc) | ((zlcvmask >> 2) & 0x3);
			int cv = ((zlcv << 2) & 0xc) | (zlcvmask & 0x3);
			int tp = (opcode >> 8) & 0x3;
			int n = ((opcode >> 12) & 0x1) + 1;

			util::stream_format(*output, "xc      %d", n);
			print_condition_codes(true, zl, cv, tp);
			flags = STEP_COND;
			break;
		}

		case 0xe8: case 0xe9: case 0xea: case 0xeb:
		{
			int zlcvmask = opcode & 0xf;
			int zlcv = (opcode >> 4) & 0xf;
			int zl = (zlcv & 0xc) | ((zlcvmask >> 2) & 0x3);
			int cv = ((zlcv << 2) & 0xc) | (zlcvmask & 0x3);
			int tp = (opcode >> 8) & 0x3;

			util::stream_format(*output, "cc      %04X", FETCH(npc, opcodes));
			print_condition_codes(true, zl, cv, tp);
			flags = STEP_OVER | STEP_COND;
			break;
		}

		case 0xec: case 0xed: case 0xee: case 0xef:
		{
			int zlcvmask = opcode & 0xf;
			int zlcv = (opcode >> 4) & 0xf;
			int zl = (zlcv & 0xc) | ((zlcvmask >> 2) & 0x3);
			int cv = ((zlcv << 2) & 0xc) | (zlcvmask & 0x3);
			int tp = (opcode >> 8) & 0x3;

			if (opcode == 0xef00)
			{
				util::stream_format(*output, "ret");
				flags = STEP_OUT;
			}
			else
			{
				util::stream_format(*output, "retc    ");
				print_condition_codes(false, zl, cv, tp);
				flags = STEP_OUT | STEP_COND;
			}
			break;
		}

		case 0xf0: case 0xf1: case 0xf2: case 0xf3:
		{
			int zlcvmask = opcode & 0xf;
			int zlcv = (opcode >> 4) & 0xf;
			int zl = (zlcv & 0xc) | ((zlcvmask >> 2) & 0x3);
			int cv = ((zlcv << 2) & 0xc) | (zlcvmask & 0x3);
			int tp = (opcode >> 8) & 0x3;

			util::stream_format(*output, "bcndd   %04X", FETCH(npc, opcodes));
			print_condition_codes(true, zl, cv, tp);
			flags = STEP_COND | step_over_extra(1);
			break;
		}

		case 0xf8: case 0xf9: case 0xfa: case 0xfb:
		{
			int zlcvmask = opcode & 0xf;
			int zlcv = (opcode >> 4) & 0xf;
			int zl = (zlcv & 0xc) | ((zlcvmask >> 2) & 0x3);
			int cv = ((zlcv << 2) & 0xc) | (zlcvmask & 0x3);
			int tp = (opcode >> 8) & 0x3;

			util::stream_format(*output, "ccd     %04X", FETCH(npc, opcodes));
			print_condition_codes(true, zl, cv, tp);
			flags = STEP_OVER | STEP_COND | step_over_extra(1);
			break;
		}

		case 0xfc: case 0xfd: case 0xfe: case 0xff:
		{
			int zlcvmask = opcode & 0xf;
			int zlcv = (opcode >> 4) & 0xf;
			int zl = (zlcv & 0xc) | ((zlcvmask >> 2) & 0x3);
			int cv = ((zlcv << 2) & 0xc) | (zlcvmask & 0x3);
			int tp = (opcode >> 8) & 0x3;

			if (opcode == 0xff00)
			{
				util::stream_format(*output, "retd");
				flags = STEP_OUT | step_over_extra(1);
			}
			else
			{
				util::stream_format(*output, "retcd   ");
				print_condition_codes(false, zl, cv, tp);
				flags = STEP_OUT | STEP_COND | step_over_extra(1);
			}
			break;
		}

		default:    util::stream_format(*output, "???     ($%04X)", opcode); break;
	}

	return (npc-pc) | flags | SUPPORTED;
}
