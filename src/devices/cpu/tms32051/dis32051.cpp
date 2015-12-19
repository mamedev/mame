// license:BSD-3-Clause
// copyright-holders:Ville Linde
#include "emu.h"



static const char *const zl_condition_codes[] =
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

static const char *const cv_condition_codes[16] =
{
	"",             // C=0, V=0, CM=0, VM=0
	"nov",          // C=0, V=0, CM=0, VM=1
	"nc",           // C=0, V=0, CM=1, VM=0
	"nc nov",       // C=0, V=0, CM=1, VM=1
	"",             // C=0, V=1, CM=0, VM=0
	"ov",           // C=0, V=1, CM=0, VM=1
	"nc",           // C=0, V=1, CM=1, VM=0
	"nc ov",        // C=0, V=1, CM=1, VM=1
	"",             // C=1, V=0, CM=0, VM=0
	"nov",          // C=1, V=0, CM=0, VM=1
	"c",            // C=1, V=0, CM=1, VM=0
	"c nov",        // C=1, V=0, CM=1, VM=1
	"",             // C=1, V=1, CM=0, VM=0
	"ov",           // C=1, V=1, CM=0, VM=1
	"c",            // C=1, V=1, CM=1, VM=0
	"c ov",         // C=1, V=1, CM=1, VM=1
};

static const char *const tp_condition_codes[4] =
{
	"bio",
	"tc",
	"ntc",
	""
};


static offs_t npc;
static const UINT8 *rombase;
static offs_t pcbase;

static char *output;

static void ATTR_PRINTF(1,2) print(const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	output += vsprintf(output, fmt, vl);
	va_end(vl);
}

static UINT16 FETCH(void)
{
	UINT16 result = rombase[(npc - pcbase) * 2 + 0] | (rombase[(npc - pcbase) * 2 + 1] << 8);
	npc++;
	return result;
}

static char *GET_ADDRESS(int addr_mode, int address)
{
	static char buffer[100];

	if (addr_mode)      // Indirect addressing
	{
		int nar = address & 0x7;


		switch ((address >> 3) & 0xf)
		{
			case 0x0:   sprintf(buffer, "*"); break;
			case 0x1:   sprintf(buffer, "*, ar%d", nar); break;
			case 0x2:   sprintf(buffer, "*-"); break;
			case 0x3:   sprintf(buffer, "*-, ar%d", nar); break;
			case 0x4:   sprintf(buffer, "*+"); break;
			case 0x5:   sprintf(buffer, "*+, ar%d", nar); break;
			case 0x8:   sprintf(buffer, "*br0-"); break;
			case 0x9:   sprintf(buffer, "*br0-, ar%d", nar); break;
			case 0xa:   sprintf(buffer, "*0-"); break;
			case 0xb:   sprintf(buffer, "*0-, ar%d", nar); break;
			case 0xc:   sprintf(buffer, "*0+"); break;
			case 0xd:   sprintf(buffer, "*0+, ar%d", nar); break;
			case 0xe:   sprintf(buffer, "*br0+"); break;
			case 0xf:   sprintf(buffer, "*br0+, ar%d", nar); break;

			default:    sprintf(buffer, "??? (indirect)"); break;
		}
	}
	else            // Direct addressing
	{
		sprintf(buffer, "#%02X", address);
	}
	return buffer;
}

static char *GET_SHIFT(int shift)
{
	static char buffer[100];

	if (shift > 0)
	{
		sprintf(buffer, ", %d", shift);
	}
	else
	{
		memset(buffer, 0, sizeof(buffer));
	}

	return buffer;
}

static void print_condition_codes(bool pp, int zl, int cv, int tp)
{
	if (*(zl_condition_codes[zl]) != 0)
	{
		if (pp)
			print(", ");
		print("%s", zl_condition_codes[zl]);
		pp = true;
	}
	if (*(cv_condition_codes[cv]) != 0)
	{
		if (pp)
			print(", ");
		print("%s", cv_condition_codes[cv]);
		pp = true;
	}
	if (*(tp_condition_codes[tp]) != 0)
	{
		if (pp)
			print(", ");
		print("%s", tp_condition_codes[tp]);
	}
}

static void dasm_group_be(UINT16 opcode)
{
	int subop = opcode & 0xff;

	switch (subop)
	{
		case 0x00:  print("abs"); break;
		case 0x01:  print("cmpl"); break;
		case 0x02:  print("neg"); break;
		case 0x03:  print("pac"); break;
		case 0x04:  print("apac"); break;
		case 0x05:  print("spac"); break;
		case 0x09:  print("sfl"); break;
		case 0x0a:  print("sfr"); break;
		case 0x0c:  print("rol"); break;
		case 0x0d:  print("ror"); break;
		case 0x10:  print("addb"); break;
		case 0x11:  print("adcb"); break;
		case 0x12:  print("andb"); break;
		case 0x13:  print("orb"); break;
		case 0x14:  print("rolb"); break;
		case 0x15:  print("rorb"); break;
		case 0x16:  print("sflb"); break;
		case 0x17:  print("sfrb"); break;
		case 0x18:  print("sbb"); break;
		case 0x19:  print("sbbb"); break;
		case 0x1a:  print("xorb"); break;
		case 0x1b:  print("crgt"); break;
		case 0x1c:  print("crlt"); break;
		case 0x1d:  print("exar"); break;
		case 0x1e:  print("sacb"); break;
		case 0x1f:  print("lacb"); break;
		case 0x20:  print("bacc"); break;
		case 0x21:  print("baccd"); break;
		case 0x22:  print("idle"); break;
		case 0x23:  print("idle2"); break;
		case 0x30:  print("cala"); break;
		case 0x32:  print("pop"); break;
		case 0x38:  print("reti"); break;
		case 0x3a:  print("rete"); break;
		case 0x3c:  print("push"); break;
		case 0x3d:  print("calad"); break;
		case 0x40:  print("clrc    intm"); break;
		case 0x41:  print("setc    intm"); break;
		case 0x42:  print("clrc    ovm"); break;
		case 0x43:  print("setc    ovm"); break;
		case 0x44:  print("clrc    cnf"); break;
		case 0x45:  print("setc    cnf"); break;
		case 0x46:  print("clrc    sxm"); break;
		case 0x47:  print("setc    sxm"); break;
		case 0x48:  print("clrc    hold"); break;
		case 0x49:  print("setc    hold"); break;
		case 0x4a:  print("clrc    tc"); break;
		case 0x4b:  print("setc    tc"); break;
		case 0x4c:  print("clrc    xf"); break;
		case 0x4d:  print("setc    xf"); break;
		case 0x4e:  print("clrc    carry"); break;
		case 0x4f:  print("setc    carry"); break;
		case 0x51:  print("trap"); break;
		case 0x52:  print("nmi"); break;
		case 0x58:  print("zpr"); break;
		case 0x59:  print("zap"); break;
		case 0x5a:  print("sath"); break;
		case 0x5b:  print("satl"); break;
		case 0x60: case 0x61: case 0x62: case 0x63:
		case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b:
		case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		case 0x70: case 0x71: case 0x72: case 0x73:
		case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b:
		case 0x7c: case 0x7d: case 0x7e: case 0x7f:
					print("intr    %d", opcode & 0x1f); break;

		case 0x80:  print("mpy     #%04X", FETCH()); break;
		case 0x81:  print("and     #%04X", FETCH() << 16); break;
		case 0x82:  print("or      #%04X", FETCH() << 16); break;
		case 0x83:  print("xor     #%04X", FETCH() << 16); break;
		case 0xc4:  print("rpt     #%04X", FETCH()); break;
		case 0xc5:  print("rpt     #%04X", FETCH()); break;
		case 0xc6:  print("rpt     #%04X", FETCH()); break;

		default:    print("???     (group be)"); break;
	}
}

static void dasm_group_bf(UINT16 opcode)
{
	int subop = (opcode >>  4) & 0xf;
	int shift = opcode & 0xf;

	switch (subop)
	{
		case 0x0:
		{
			if (opcode & 0x8)
			{
				print("lar     ar%d, #%04X", opcode & 0x7, FETCH());
			}
			else
			{
				print("spm     #%02X", opcode & 0x3);
			}
			break;
		}

		case 0x4:
		{
			switch (opcode & 0x3)
			{
				case 0: print("cmpr    ar = arcr"); break;
				case 1: print("cmpr    ar < arcr"); break;
				case 2: print("cmpr    ar > arcr"); break;
				case 3: print("cmpr    ar != arcr"); break;
			}
			break;
		}

		case 0x8:   print("lacc    #%04X", FETCH() << shift); break;
		case 0x9:   print("add     #%04X", FETCH() << shift); break;
		case 0xa:   print("sub     #%04X", FETCH() << shift); break;
		case 0xb:   print("and     #%04X", FETCH() << shift); break;
		case 0xc:   print("or      #%04X", FETCH() << shift); break;
		case 0xd:   print("xor     #%04X", FETCH() << shift); break;
		case 0xe:   print("bsar    %d", shift+1); break;

		default:    print("???     (group bf)"); break;
	}
}

CPU_DISASSEMBLE( tms32051 )
{
	UINT32 flags = 0;
	UINT16 opcode;
	int baseop;
	int address, addr_mode;

	pcbase = pc;
	rombase = oprom;
	npc = pc;
	output = buffer;

	opcode = FETCH();
	baseop = (opcode >> 8) & 0xff;

	addr_mode = (opcode >> 7) & 0x1;
	address = opcode & 0x7f;

	switch (baseop)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
		{
			print("lar     ar%d, %s", (opcode >> 8) & 0x7, GET_ADDRESS(addr_mode, address));
			break;
		}
		case 0x08:  print("lamm    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x09:  print("smmr    %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH()); break;
		case 0x0a:  print("subc    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x0b:  print("rpt     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x0c:  print("out     %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH()); break;
		case 0x0d:  print("ldp     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x0e:  print("lst     0, %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x0f:  print("lst     1, %s", GET_ADDRESS(addr_mode, address)); break;

		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		{
			print("lacc    %s %s", GET_ADDRESS(addr_mode, address), GET_SHIFT((opcode >> 8) & 0xf));
			break;
		}

		case 0x20: case 0x21: case 0x22: case 0x23:
		case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		{
			print("add     %s %s", GET_ADDRESS(addr_mode, address), GET_SHIFT((opcode >> 8) & 0xf));
			break;
		}

		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b:
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
		{
			print("sub     %s %s", GET_ADDRESS(addr_mode, address), GET_SHIFT((opcode >> 8) & 0xf));
			break;
		}

		case 0x40: case 0x41: case 0x42: case 0x43:
		case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b:
		case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		{
			print("bit     %d, %s", (opcode >> 8) & 0xf, GET_ADDRESS(addr_mode, address));
			break;
		}

		case 0x50:  print("mpya    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x51:  print("mpys    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x52:  print("sqra    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x53:  print("sqrs    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x54:  print("mpy     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x55:  print("mpyu    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x57:  print("bldp    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x58:  print("xpl     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x59:  print("opl     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x5a:  print("apl     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x5b:  print("cpl     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x5c:  print("xpl     %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH()); break;
		case 0x5d:  print("opl     %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH()); break;
		case 0x5e:  print("apl     %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH()); break;
		case 0x5f:  print("cpl     %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH()); break;

		case 0x60:  print("addc    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x61:  print("add     %s << 16", GET_ADDRESS(addr_mode, address)); break;
		case 0x62:  print("adds    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x63:  print("addt    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x64:  print("subb    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x65:  print("sub     %s << 16", GET_ADDRESS(addr_mode, address)); break;
		case 0x66:  print("subs    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x67:  print("subt    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x68:  print("zalr    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x69:  print("lacl    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x6a:  print("lacc    %s << 16", GET_ADDRESS(addr_mode, address)); break;
		case 0x6b:  print("lact    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x6c:  print("xor     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x6d:  print("or      %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x6e:  print("and     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x6f:  print("bitt    %s", GET_ADDRESS(addr_mode, address)); break;

		case 0x70:  print("lta     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x71:  print("ltp     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x72:  print("ltd     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x73:  print("lt      %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x74:  print("lts     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x75:  print("lph     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x76:  print("pshd    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x77:  print("dmov    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x78:  print("adrk    #%02X", opcode & 0xff); break;
		case 0x79:  print("b       %04X, %s", FETCH(), GET_ADDRESS(1, address)); break;
		case 0x7a:  print("call    %04X, %s", FETCH(), GET_ADDRESS(1, address)); flags = DASMFLAG_STEP_OVER; break;
		case 0x7b:  print("banz    %04X, %s", FETCH(), GET_ADDRESS(1, address)); break;
		case 0x7c:  print("sbrk    #%02X", opcode & 0xff); break;
		case 0x7d:  print("bd      %04X, %s", FETCH(), GET_ADDRESS(1, address)); break;
		case 0x7e:  print("calld   %04X, %s", FETCH(), GET_ADDRESS(1, address)); flags = DASMFLAG_STEP_OVER; break;
		case 0x7f:  print("banzd   %04X, %s", FETCH(), GET_ADDRESS(1, address)); break;

		case 0x80: case 0x81: case 0x82: case 0x83:
		case 0x84: case 0x85: case 0x86: case 0x87:
		{
			print("sar     ar%d, %s", (opcode >> 8) & 0x7, GET_ADDRESS(addr_mode, address));
			break;
		}
		case 0x88:  print("samm    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x89:  print("lmmr    %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH()); break;
		case 0x8a:  print("popd    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x8b:
		{
			if ((opcode & 0xff) == 0)
			{
				print("nop");
			}
			else
			{
				print("mar     %s", GET_ADDRESS(addr_mode, address));
			}
			break;
		}
		case 0x8c:  print("spl     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x8d:  print("sph     %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x8e:  print("sst     0, %s", GET_ADDRESS(addr_mode, address)); break;
		case 0x8f:  print("sst     1, %s", GET_ADDRESS(addr_mode, address)); break;

		case 0x90: case 0x91: case 0x92: case 0x93:
		case 0x94: case 0x95: case 0x96: case 0x97:
		{
			print("sacl    %s %s", GET_ADDRESS(addr_mode, address), GET_SHIFT((opcode >> 8) & 0x7));
			break;
		}
		case 0x98: case 0x99: case 0x9a: case 0x9b:
		case 0x9c: case 0x9d: case 0x9e: case 0x9f:
		{
			print("sach    %s %s", GET_ADDRESS(addr_mode, address), GET_SHIFT((opcode >> 8) & 0x7));
			break;
		}

		case 0xa0:  print("norm    %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH()); break;
		case 0xa2:  print("mac     %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH()); break;
		case 0xa3:  print("macd    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0xa4:  print("blpd    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0xa5:  print("blpd    %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH()); break;
		case 0xa6:  print("tblr    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0xa7:  print("tblw    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0xa8:  print("bldd    %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH()); break;
		case 0xa9:  print("bldd    %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH()); break;
		case 0xaa:  print("mads    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0xab:  print("madd    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0xac:  print("bldd    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0xad:  print("bldd    %s", GET_ADDRESS(addr_mode, address)); break;
		case 0xae:  print("splk    %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH()); break;
		case 0xaf:  print("in      %s, #%04X", GET_ADDRESS(addr_mode, address), FETCH()); break;

		case 0xb0: case 0xb1: case 0xb2: case 0xb3:
		case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		{
			print("lar     ar%d, #%02X", (opcode >> 8) & 0x7, opcode & 0xff);
			break;
		}
		case 0xb8:  print("add     #%02X", opcode & 0xff); break;
		case 0xb9:  print("lacl    #%02X", opcode & 0xff); break;
		case 0xba:  print("sub     #%02X", opcode & 0xff); break;
		case 0xbb:  print("rpt     #%02X", opcode & 0xff); break;

		case 0xbc:
		case 0xbd:
		{
			print("ldp     #%03X", opcode & 0x1ff);
			break;
		}
		case 0xbe:  dasm_group_be(opcode); break;
		case 0xbf:  dasm_group_bf(opcode); break;

		case 0xe0: case 0xe1: case 0xe2: case 0xe3:
		{
			int zlcvmask = opcode & 0xf;
			int zlcv = (opcode >> 4) & 0xf;
			int zl = (zlcv & 0xc) | ((zlcvmask >> 2) & 0x3);
			int cv = ((zlcv << 2) & 0xc) | (zlcvmask & 0x3);
			int tp = (opcode >> 8) & 0x3;

			print("bcnd    %04X", FETCH());
			print_condition_codes(true, zl, cv, tp);
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

			print("xc      %d", n);
			print_condition_codes(true, zl, cv, tp);
			break;
		}

		case 0xe8: case 0xe9: case 0xea: case 0xeb:
		{
			int zlcvmask = opcode & 0xf;
			int zlcv = (opcode >> 4) & 0xf;
			int zl = (zlcv & 0xc) | ((zlcvmask >> 2) & 0x3);
			int cv = ((zlcv << 2) & 0xc) | (zlcvmask & 0x3);
			int tp = (opcode >> 8) & 0x3;

			print("cc      %04X", FETCH());
			print_condition_codes(true, zl, cv, tp);
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
				print("ret");
			}
			else
			{
				print("retc    ");
				print_condition_codes(false, zl, cv, tp);
			}
			flags = DASMFLAG_STEP_OUT;
			break;
		}

		case 0xf0: case 0xf1: case 0xf2: case 0xf3:
		{
			int zlcvmask = opcode & 0xf;
			int zlcv = (opcode >> 4) & 0xf;
			int zl = (zlcv & 0xc) | ((zlcvmask >> 2) & 0x3);
			int cv = ((zlcv << 2) & 0xc) | (zlcvmask & 0x3);
			int tp = (opcode >> 8) & 0x3;

			print("bcndd   %04X", FETCH());
			print_condition_codes(true, zl, cv, tp);
			break;
		}

		case 0xf8: case 0xf9: case 0xfa: case 0xfb:
		{
			int zlcvmask = opcode & 0xf;
			int zlcv = (opcode >> 4) & 0xf;
			int zl = (zlcv & 0xc) | ((zlcvmask >> 2) & 0x3);
			int cv = ((zlcv << 2) & 0xc) | (zlcvmask & 0x3);
			int tp = (opcode >> 8) & 0x3;

			print("ccd     %04X", FETCH());
			print_condition_codes(true, zl, cv, tp);
			break;
		}

		case 0xfc: case 0xfd: case 0xfe:
		{
			int zlcvmask = opcode & 0xf;
			int zlcv = (opcode >> 4) & 0xf;
			int zl = (zlcv & 0xc) | ((zlcvmask >> 2) & 0x3);
			int cv = ((zlcv << 2) & 0xc) | (zlcvmask & 0x3);
			int tp = (opcode >> 8) & 0x3;

			print("retcd   ");
			print_condition_codes(false, zl, cv, tp);
			flags = DASMFLAG_STEP_OUT;
			break;
		}

		case 0xff:  print("retd"); flags = DASMFLAG_STEP_OUT; break;

		default:    print("???     ($%04X)", opcode); break;
	}

	return (npc-pc) | flags | DASMFLAG_SUPPORTED;
}
