// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff
/*****************************************************************************
 *
 *   i8051dasm.c
 *   Portable MCS-51 Family Emulator
 *
 *   Chips in the family:
 *   8051 Product Line (8031,8051,8751)
 *   8052 Product Line (8032,8052,8752)
 *   8054 Product Line (8054)
 *   8058 Product Line (8058)
 *
 *   Copyright Steve Ellenoff, all rights reserved.
 *
 *  This work is based on:
 *  #1) 'Intel(tm) MC51 Microcontroller Family Users Manual' and
 *  #2) 8051 simulator by Travis Marlatte
 *  #3) Portable UPI-41/8041/8741/8042/8742 emulator V0.1 by Juergen Buchmueller (MAME CORE)
 *
 *****************************************************************************
 * Symbol Memory Name Tables borrowed from:
 * D52 8052 Disassembler - Copyright Jeffery L. Post
 *****************************************************************************/

#include "emu.h"
#include "mcs51dasm.h"

// Note: addresses >= 0x100 are bit addresses

const mcs51_disassembler::mem_info mcs51_disassembler::default_names[] = {
	{  0x00, "rb0r0" },
	{  0x01, "rb0r1" },
	{  0x02, "rb0r2" },
	{  0x03, "rb0r3" },
	{  0x04, "rb0r4" },
	{  0x05, "rb0r5" },
	{  0x06, "rb0r6" },
	{  0x07, "rb0r7" },
	{  0x08, "rb1r0" },
	{  0x09, "rb1r1" },
	{  0x0a, "rb1r2" },
	{  0x0b, "rb1r3" },
	{  0x0c, "rb1r4" },
	{  0x0d, "rb1r5" },
	{  0x0e, "rb1r6" },
	{  0x0f, "rb1r7" },
	{  0x10, "rb2r0" },
	{  0x11, "rb2r1" },
	{  0x12, "rb2r2" },
	{  0x13, "rb2r3" },
	{  0x14, "rb2r4" },
	{  0x15, "rb2r5" },
	{  0x16, "rb2r6" },
	{  0x17, "rb2r7" },
	{  0x18, "rb3r0" },
	{  0x19, "rb3r1" },
	{  0x1a, "rb3r2" },
	{  0x1b, "rb3r3" },
	{  0x1c, "rb3r4" },
	{  0x1d, "rb3r5" },
	{  0x1e, "rb3r6" },
	{  0x1f, "rb3r7" },

	{  0x80, "p0"    },
	{  0x81, "sp"    },
	{  0x82, "dpl"   },
	{  0x83, "dph"   },
	{  0x87, "pcon"  },
	{  0x88, "tcon"  },
	{  0x89, "tmod"  },
	{  0x8a, "tl0"   },
	{  0x8b, "tl1"   },
	{  0x8c, "th0"   },
	{  0x8d, "th1"   },
	{  0x90, "p1"    },
	{  0x98, "scon"  },
	{  0x99, "sbuf"  },
	{  0xa0, "p2"    },
	{  0xa8, "ie"    },
	{  0xb0, "p3"    },
	{  0xb8, "ip"    },
	{  0xd0, "psw"   },
	{  0xe0, "acc"   },
	{  0xf0, "b"     },

	{ 0x188, "it0"   },
	{ 0x189, "ie0"   },
	{ 0x18a, "it1"   },
	{ 0x18b, "ie1"   },
	{ 0x18c, "tr0"   },
	{ 0x18d, "tf0"   },
	{ 0x18e, "tr1"   },
	{ 0x18f, "tf1"   },

	{ 0x198, "ri"    },
	{ 0x199, "ti"    },
	{ 0x19a, "rb8"   },
	{ 0x19b, "tb8"   },
	{ 0x19c, "ren"   },
	{ 0x19d, "sm2"   },
	{ 0x19e, "sm1"   },
	{ 0x19f, "sm0"   },

	{ 0x1a8, "ex0"   },
	{ 0x1a9, "et0"   },
	{ 0x1aa, "ex1"   },
	{ 0x1ab, "et1"   },
	{ 0x1ac, "es"    },
	{ 0x1ad, "ie.5"  },
	{ 0x1ae, "ie.6"  },
	{ 0x1af, "ea"    },

	/* FIXME: port 3 - depends on external circuits and not really
	 * implemented in the core. TBD */
	{ 0x1b0, "rxd"   },
	{ 0x1b1, "txd"   },
	{ 0x1b2, "int0"  },
	{ 0x1b3, "int1"  },
	{ 0x1b4, "t0"    },
	{ 0x1b5, "t1"    },
	{ 0x1b6, "wr"    },
	{ 0x1b7, "rd"    },

	{ 0x1b8, "px0"   },
	{ 0x1b9, "pt0"   },
	{ 0x1ba, "px1"   },
	{ 0x1bb, "pt1"   },
	{ 0x1bc, "ps"    },
	{ 0x1bd, "ip.5"  },
	{ 0x1be, "ip.6"  },
	{ 0x1bf, "ip.7"  },

	{ 0x1d0, "p"     },
	{ 0x1d1, "psw.1" },
	{ 0x1d2, "ov"    },
	{ 0x1d3, "rs0"   },
	{ 0x1d4, "rs1"   },
	{ 0x1d5, "f0"    },
	{ 0x1d6, "ac"    },
	{ 0x1d7, "cy"    },

	{ -1 }
};

const mcs51_disassembler::mem_info mcs51_disassembler::i8052_names[] = {
	{  0xc8, "t2con"  },
	{  0xca, "rcap2l" },
	{  0xcb, "rcap2h" },
	{  0xcc, "tl2"    },
	{  0xcd, "th2"    },

	{ 0x1ad, "et2"    },
	{ 0x1bd, "pt2"    },

	{ 0x190, "t2"     },
	{ 0x191, "t2ex"   },

	{ 0x1c8, "cprl2"  },
	{ 0x1c9, "ct2"    },
	{ 0x1ca, "tr2"    },
	{ 0x1cb, "exen2"  },
	{ 0x1cc, "tclk"   },
	{ 0x1cd, "rclk"   },
	{ 0x1ce, "exf2"   },
	{ 0x1cf, "tf2"    },

	{ -1 }
};

const mcs51_disassembler::mem_info mcs51_disassembler::i80c52_names[] = {
	{  0xb7, "iph"    },
	{  0xa9, "saddr"  },
	{  0xb9, "saden"  },
	{  0xc9, "t2mod"  },

	{ -1 }
};

const mcs51_disassembler::mem_info mcs51_disassembler::i8xc51fx_names[] = {
	{  0xd8, "ccon"   },
	{  0xd9, "cmod"   },
	{  0xe9, "cl"     },
	{  0xf9, "ch"     },
	{  0xda, "ccapm0" },
	{  0xdb, "ccapm1" },
	{  0xdc, "ccapm2" },
	{  0xdd, "ccapm3" },
	{  0xde, "ccapm4" },
	{  0xea, "ccap0l" },
	{  0xfa, "ccap0h" },
	{  0xeb, "ccap1l" },
	{  0xfb, "ccap1h" },
	{  0xec, "ccap2l" },
	{  0xfc, "ccap2h" },
	{  0xed, "ccap3l" },
	{  0xfd, "ccap3h" },
	{  0xee, "ccap4l" },
	{  0xfe, "ccap4h" },

	{ 0x192, "eci"    },
	{ 0x193, "cex0"   },
	{ 0x194, "cex1"   },
	{ 0x195, "cex2"   },
	{ 0x196, "cex3"   },
	{ 0x197, "cex4"   },

	{ 0x1ae, "ec"     },
	{ 0x1be, "ppc"    },

	{ 0x1d8, "ccf0"   },
	{ 0x1d9, "ccf1"   },
	{ 0x1da, "ccf2"   },
	{ 0x1db, "ccf3"   },
	{ 0x1dc, "ccf4"   },
	{ 0x1de, "cr"     },
	{ 0x1df, "cf"     },
};

const mcs51_disassembler::mem_info mcs51_disassembler::i8xc51gb_names[] = {
	{  0x84, "ad0"    },
	{  0x94, "ad1"    },
	{  0xa4, "ad2"    },
	{  0xb4, "ad3"    },
	{  0xc4, "ad4"    },
	{  0xd4, "ad5"    },
	{  0xe4, "ad6"    },
	{  0xf4, "ad7"    },
	{  0x85, "acon"   },
	{  0xc7, "acmp"   },

	{  0xe8, "c1con"  },
	{  0x9f, "c1mod"  },
	{  0xaf, "cl1"    },
	{  0xbf, "ch1"    },
	{  0x9a, "c1capm0"},
	{  0x9b, "c1capm1"},
	{  0x9c, "c1capm2"},
	{  0x9d, "c1capm3"},
	{  0x9e, "c1capm4"},
	{  0xaa, "c1cap0l"},
	{  0xba, "c1cap0h"},
	{  0xab, "c1cap1l"},
	{  0xbb, "c1cap1h"},
	{  0xac, "c1cap2l"},
	{  0xbc, "c1cap2h"},
	{  0xad, "c1cap3l"},
	{  0xbd, "c1cap3h"},
	{  0xae, "c1cap4l"},
	{  0xbe, "c1cap4h"},

	{  0xc6, "exicon" },
	{  0xa7, "iea"    },
	{  0xb6, "ipa"    },
	{  0xb5, "ipah"   },

	{  0xa5, "oscr"   },
	{  0xa6, "wdtrst" },

	{  0xc0, "p4"     },
	{  0xf8, "p5"     },

	{  0xd7, "sepcon" },
	{  0xe7, "sepdat" },
	{  0xf7, "sepstat"},

	{ 0x1c0, "sepclk" },
	{ 0x1c1, "sepdat" },
	{ 0x1c2, "eci1"   },
	{ 0x1c3, "c1ex0"  },
	{ 0x1c4, "c1ex1"  },
	{ 0x1c5, "c1ex2"  },
	{ 0x1c6, "c1ex3"  },
	{ 0x1c7, "c1ex4"  },

	{ 0x1e8, "c1cf0"  },
	{ 0x1e9, "c1cf1"  },
	{ 0x1ea, "c1cf2"  },
	{ 0x1eb, "c1cf3"  },
	{ 0x1ec, "c1cf4"  },
	{ 0x1ed, "cre"    },
	{ 0x1ee, "cr1"    },
	{ 0x1ef, "cf1"    },

	{ 0x1fa, "int2"   },
	{ 0x1fb, "int3"   },
	{ 0x1fc, "int4"   },
	{ 0x1fd, "int5"   },
	{ 0x1fe, "int6"   },

	{ -1 }
};

const mcs51_disassembler::mem_info mcs51_disassembler::ds5002fp_names[] = {
	{  0x8e, "pwcm"   },
	{  0x8f, "pwmp"   },
	{  0xc1, "crcr"   },
	{  0xc2, "crcl"   },
	{  0xc3, "crch"   },
	{  0xc6, "mcon"   },
	{  0xc7, "ta"     },
	{  0xcf, "rnr"    },
	{  0xd8, "rpctl"  },
	{  0xd9, "rps"    },

	{ 0x1d8, "rg0"    },
	{ 0x1d9, "rpc"    },
	{ 0x1da, "dma"    },
	{ 0x1db, "ibi"    },
	{ 0x1dc, "ae"     },
	{ 0x1dd, "exbs"   },
	{ 0x1de, "d8.6"   },
	{ 0x1df, "rnr"    },

	{ -1 }
};

const mcs51_disassembler::mem_info mcs51_disassembler::i8xc751_names[] = {
	{  0x98, "i2con"   },
	{  0x99, "i2dat"   },
	{  0xd8, "i2cfg"   },
	{  0xf8, "i2sta"   },  /* read only */

	{ 0x198, "xstp"    }, /* read: no function */
	{ 0x199, "xstr"    }, /* read: MASTER */
	{ 0x19a, "cstp"    }, /* read: STP */
	{ 0x19b, "cstr"    }, /* read: STR */
	{ 0x19c, "carl"    }, /* read: ARL */
	{ 0x19d, "cdr"     }, /* read: DRDY */
	{ 0x19e, "idle"    }, /* read: ATN */
	{ 0x19f, "cxa"     }, /* read: RDAT */

	{ 0x1ac, "ei2"     },

	{ 0x1bc, "pi2"     },

	{ 0x1d8, "ct0"     },
	{ 0x1d9, "ct1"     },
	{ 0x1da, "i2cfg.2" },
	{ 0x1db, "i2cfg.3" },
	{ 0x1dc, "tirun"   },
	{ 0x1dd, "clrti"   },
	{ 0x1de, "masterq" },
	{ 0x1df, "slaven"  },

	{ 0x1f8, "xstp"    },
	{ 0x1f9, "xstr"    },
	{ 0x1fa, "makstp"  },
	{ 0x1fb, "makstr"  },
	{ 0x1fc, "xactv"   },
	{ 0x1fd, "xdata"   },
	{ 0x1fe, "idle"    },
	{ 0x1ff, "i2sta.7" },

	/* unknown
	 * "ibf",    "obf",    "idsm",   "obfc",    e8 - eb
	 * "ma0",    "ma1",    "mb0",    "mb1",     ec - ef
	 */

	{ -1 }
};

mcs51_disassembler::mcs51_disassembler()
{
}

void mcs51_disassembler::add_names(const mem_info *info)
{
	for(unsigned int i=0; info[i].addr >= 0; i++)
		m_names[info[i].addr] = info[i].name;
}

u32 mcs51_disassembler::opcode_alignment() const
{
	return 1;
}


std::string mcs51_disassembler::get_data_address( uint8_t arg ) const
{
	auto i = m_names.find(arg);
	if (i == m_names.end())
		return util::string_format("$%02X", arg);
	else
		return i->second;
}

std::string mcs51_disassembler::get_bit_address( uint8_t arg ) const
{
	if(arg < 0x80)
	{
		//Bit address 0-7F can be referred to as 20.0, 20.1, to 20.7 for address 0, and 2f.0,2f.1 to 2f.7 for address 7f
		return util::string_format("$%02X.%d", (arg >> 3) | 0x20, arg & 0x07);
	}
	else
	{
		auto i = m_names.find(arg | 0x100);
		if (i == m_names.end())
		{
			i = m_names.find(arg & 0xf8);
			if (i == m_names.end())
				return util::string_format("$%02X.%d", arg & 0xf8, arg & 0x07);
			else
				return util::string_format("%s.%d", i->second, arg & 0x07);
		}
		else
			return i->second;
	}
}

offs_t mcs51_disassembler::disassemble_op(std::ostream &stream, unsigned PC, offs_t pc, const data_buffer &opcodes, const data_buffer &params, uint8_t op)
{
	uint32_t flags = 0;
	std::string sym, sym2;
	uint8_t data;
	uint16_t addr;
	int8_t rel;

	switch( op )
	{
		//NOP
		case 0x00:              /* 1: 0000 0000 */
			util::stream_format(stream, "nop");
			break;

		//AJMP code addr        /* 1: aaa0 0001 */
		case 0x01:
		case 0x21:
		case 0x41:
		case 0x61:
		case 0x81:
		case 0xa1:
		case 0xc1:
		case 0xe1:
			addr = params.r8(PC++);
			addr|= (PC & 0xf800) | ((op & 0xe0) << 3);
			util::stream_format(stream, "ajmp  $%04X", addr);
			break;

		//LJMP code addr
		case 0x02:              /* 1: 0000 0010 */
			addr = (params.r8(PC++)<<8) & 0xff00;
			addr|= params.r8(PC++);
			util::stream_format(stream, "ljmp  $%04X", addr);
			break;

		//RR A
		case 0x03:              /* 1: 0000 0011 */
			util::stream_format(stream, "rr    a");
			break;

		//INC A
		case 0x04:              /* 1: 0000 0100 */
			util::stream_format(stream, "inc   a");
			break;

		//INC data addr
		case 0x05:              /* 1: 0000 0101 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "inc   %s", sym);
			break;

		//INC @R0/@R1           /* 1: 0000 011i */
		case 0x06:
		case 0x07:
			util::stream_format(stream, "inc   @r%d", op&1);
			break;

		//INC R0 to R7          /* 1: 0000 1rrr */
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
			util::stream_format(stream, "inc   r%d", op&7);
			break;

		//JBC bit addr, code addr
		case 0x10:              /* 1: 0001 0000 */
			sym = get_bit_address(params.r8(PC++));
			rel  = params.r8(PC++);
			util::stream_format(stream, "jbc   %s,$%04X", sym, PC + rel);
			break;

		//ACALL code addr       /* 1: aaa1 0001 */
		case 0x11:
		case 0x31:
		case 0x51:
		case 0x71:
		case 0x91:
		case 0xb1:
		case 0xd1:
		case 0xf1:
			util::stream_format(stream, "acall $%04X", (PC & 0xf800) | ((op & 0xe0) << 3) | params.r8(PC));
			PC++;
			flags = STEP_OVER;
			break;

		//LCALL code addr
		case 0x12:              /* 1: 0001 0010 */
			addr = (params.r8(PC++)<<8) & 0xff00;
			addr|= params.r8(PC++);
			util::stream_format(stream, "lcall $%04X", addr);
			flags = STEP_OVER;
			break;

		//RRC A
		case 0x13:              /* 1: 0001 0011 */
			util::stream_format(stream, "rrc   a");
			break;

		//DEC A
		case 0x14:              /* 1: 0001 0100 */
			util::stream_format(stream, "dec   a");
			break;

		//DEC data addr
		case 0x15:              /* 1: 0001 0101 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "dec   %s", sym);
			break;

		//Unable to test
		//DEC @R0/@R1           /* 1: 0001 011i */
		case 0x16:
		case 0x17:
			util::stream_format(stream, "dec   @r%d", op&1);
			break;

		//DEC R0 to R7          /* 1: 0001 1rrr */
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			util::stream_format(stream, "dec   r%d", op&7);
			break;

		//JB  bit addr, code addr
		case 0x20:              /* 1: 0010 0000 */
			sym = get_bit_address(params.r8(PC++));
			rel  = params.r8(PC++);
			util::stream_format(stream, "jb    %s,$%04X", sym, (PC + rel));
			break;

		//RET
		case 0x22:              /* 1: 0010 0010 */
			util::stream_format(stream, "ret");
			flags = STEP_OUT;
			break;

		//RL A
		case 0x23:              /* 1: 0010 0011 */
			util::stream_format(stream, "rl    a");
			break;

		//ADD A, #data
		case 0x24:              /* 1: 0010 0100 */
			util::stream_format(stream, "add   a,#$%02X", params.r8(PC++));
			break;

		//ADD A, data addr
		case 0x25:              /* 1: 0010 0101 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "add   a,%s", sym);
			break;

		//Unable to Test
		//ADD A, @R0/@R1        /* 1: 0010 011i */
		case 0x26:
		case 0x27:
			util::stream_format(stream, "add   a,@r%d", op&1);
			break;

		//ADD A, R0 to R7       /* 1: 0010 1rrr */
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:
			util::stream_format(stream, "add   a,r%d", op&7);
			break;

		//JNB bit addr, code addr
		case 0x30:              /* 1: 0011 0000 */
			sym = get_bit_address(params.r8(PC++));
			rel  = params.r8(PC++);
			util::stream_format(stream, "jnb   %s,$%04X", sym, (PC + rel));
			break;

		//RETI
		case 0x32:              /* 1: 0011 0010 */
			util::stream_format(stream, "reti");
			flags = STEP_OUT;
			break;

		//RLC A
		case 0x33:              /* 1: 0011 0011 */
			util::stream_format(stream, "rlc   a");
			break;

		//ADDC A, #data
		case 0x34:              /* 1: 0011 0100 */
			util::stream_format(stream, "addc  a,#$%02X", params.r8(PC++));
			break;

		//ADDC A, data addr
		case 0x35:              /* 1: 0011 0101 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "addc  a,%s", sym);
			break;

		//ADDC A, @R0/@R1       /* 1: 0011 011i */
		case 0x36:
		case 0x37:
			util::stream_format(stream, "addc  a,@r%d", op&1);
			break;

		//ADDC A, R0 to R7      /* 1: 0011 1rrr */
		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
			util::stream_format(stream, "addc  a,r%d", op&7);
			break;

		//JC code addr
		case 0x40:              /* 1: 0100 0000 */
			rel = params.r8(PC++);
			util::stream_format(stream, "jc    $%04X", PC + rel);
			break;

		//ORL data addr, A
		case 0x42:              /* 1: 0100 0010 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "orl   %s,a", sym);
			break;

		//ORL data addr, #data
		case 0x43:              /* 1: 0100 0011 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "orl   %s,#$%02X", sym, params.r8(PC++));
			break;

		//Unable to Test
		//ORL A, #data
		case 0x44:              /* 1: 0100 0100 */
			util::stream_format(stream, "orl   a,#$%02X", params.r8(PC++));
			break;

		//ORL A, data addr
		case 0x45:              /* 1: 0100 0101 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "orl   a,%s", sym);
			break;

		//ORL A, @RO/@R1        /* 1: 0100 011i */
		case 0x46:
		case 0x47:
			util::stream_format(stream, "orl   a,@r%d", op&1);
			break;

		//ORL A, RO to R7       /* 1: 0100 1rrr */
		case 0x48:
		case 0x49:
		case 0x4a:
		case 0x4b:
		case 0x4c:
		case 0x4d:
		case 0x4e:
		case 0x4f:
			util::stream_format(stream, "orl   a,r%d", op&7);
			break;

		//JNC code addr
		case 0x50:              /* 1: 0101 0000 */
			rel = params.r8(PC++);
			util::stream_format(stream, "jnc   $%04X", PC + rel);
			break;

		//Unable to test
		//ANL data addr, A
		case 0x52:              /* 1: 0101 0010 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "anl   %s,a", sym);
			break;

		//Unable to test
		//ANL data addr, #data
		case 0x53:              /* 1: 0101 0011 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "anl   %s,#$%02X", sym, params.r8(PC++));
			break;

		//ANL A, #data
		case 0x54:              /* 1: 0101 0100 */
			util::stream_format(stream, "anl   a,#$%02X", params.r8(PC++));
			break;

		//ANL A, data addr
		case 0x55:              /* 1: 0101 0101 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "anl   a,%s", sym);
			break;

		//Unable to test
		//ANL A, @RO/@R1        /* 1: 0101 011i */
		case 0x56:
		case 0x57:
			util::stream_format(stream, "anl   a,@r%d", op&1);
			break;

		//ANL A, RO to R7       /* 1: 0101 1rrr */
		case 0x58:
		case 0x59:
		case 0x5a:
		case 0x5b:
		case 0x5c:
		case 0x5d:
		case 0x5e:
		case 0x5f:
			util::stream_format(stream, "anl   a,r%d", op&7);
			break;

		//JZ code addr
		case 0x60:              /* 1: 0110 0000 */
			rel = params.r8(PC++);
			util::stream_format(stream, "jz    $%04X", PC + rel);
			break;

		//Unable to test
		//XRL data addr, A
		case 0x62:              /* 1: 0110 0010 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "xrl   %s,a", sym);
			break;

		//XRL data addr, #data
		case 0x63:              /* 1: 0110 0011 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "xrl   %s,#$%02X", sym, params.r8(PC++));
			break;

		//XRL A, #data
		case 0x64:              /* 1: 0110 0100 */
			util::stream_format(stream, "xrl   a,#$%02X", params.r8(PC++));
			break;

		//XRL A, data addr
		case 0x65:              /* 1: 0110 0101 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "xrl   a,%s", sym);
			break;

		//Unable to test
		//XRL A, @R0/@R1        /* 1: 0110 011i */
		case 0x66:
		case 0x67:
			util::stream_format(stream, "xrl   a,@r%d", op&1);
			break;

		//XRL A, R0 to R7       /* 1: 0110 1rrr */
		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
		case 0x6c:
		case 0x6d:
		case 0x6e:
		case 0x6f:
			util::stream_format(stream, "xrl   a,r%d", op&7);
			break;

		//JNZ code addr
		case 0x70:              /* 1: 0111 0000 */
			rel = params.r8(PC++);
			util::stream_format(stream, "jnz   $%04X", PC + rel);
			break;

		//Unable to test
		//ORL C, bit addr
		case 0x72:              /* 1: 0111 0010 */
			sym = get_bit_address(params.r8(PC++));
			util::stream_format(stream, "orl   c,%s", sym);
			break;

		//Unable to test
		//JMP @A+DPTR
		case 0x73:              /* 1: 0111 0011 */
			util::stream_format(stream, "jmp   @a+dptr");
			break;

		//MOV A, #data
		case 0x74:              /* 1: 0111 0100 */
			util::stream_format(stream, "mov   a,#$%02X", params.r8(PC++));
			break;

		//MOV data addr, #data
		case 0x75:              /* 1: 0111 0101 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "mov   %s,#$%02X", sym, params.r8(PC++));
			break;

		//Unable to test
		//MOV @R0/@R1, #data    /* 1: 0111 011i */
		case 0x76:
		case 0x77:
			util::stream_format(stream, "mov   @r%d,#$%02X", op&1, params.r8(PC++));
			break;

		//MOV R0 to R7, #data   /* 1: 0111 1rrr */
		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
		case 0x7c:
		case 0x7d:
		case 0x7e:
		case 0x7f:
			util::stream_format(stream, "mov   r%d,#$%02X", (op & 7), params.r8(PC++));
			break;

		//SJMP code addr
		case 0x80:              /* 1: 1000 0000 */
			rel = params.r8(PC++);
			util::stream_format(stream, "sjmp  $%04X", PC + rel);
			break;

		//ANL C, bit addr
		case 0x82:              /* 1: 1000 0010 */
			sym = get_bit_address(params.r8(PC++));
			util::stream_format(stream, "anl   c,%s", sym);
			break;

		//MOVC A, @A + PC
		case 0x83:              /* 1: 1000 0011 */
			util::stream_format(stream, "movc  a,@a+pc");
			break;

		//DIV AB
		case 0x84:              /* 1: 1000 0100 */
			util::stream_format(stream, "div   ab");
			break;

		//MOV data addr, data addr  (Note: 1st address is src, 2nd is dst, but the mov command works as mov dst,src)
		case 0x85:              /* 1: 1000 0101 */
			sym  = get_data_address(params.r8(PC++));
			sym2 = get_data_address(params.r8(PC++));
			util::stream_format(stream, "mov   %s,%s", sym2, sym);
			break;

		//Unable to test
		//MOV data addr, @R0/@R1/* 1: 1000 011i */
		case 0x86:
		case 0x87:
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "mov   %s,@r%d", sym, op&1);
			break;

		//MOV data addr,R0 to R7/* 1: 1000 1rrr */
		case 0x88:
		case 0x89:
		case 0x8a:
		case 0x8b:
		case 0x8c:
		case 0x8d:
		case 0x8e:
		case 0x8f:
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "mov   %s,r%d", sym, op&7);
			break;

		//MOV DPTR, #data16
		case 0x90:              /* 1: 1001 0000 */
			addr = (params.r8(PC++)<<8) & 0xff00;
			addr|= params.r8(PC++);
			util::stream_format(stream, "mov   dptr,#$%04X", addr);
			break;

		//MOV bit addr, C
		case 0x92:              /* 1: 1001 0010 */
			sym = get_bit_address(params.r8(PC++));
			util::stream_format(stream, "mov   %s,c", sym);
			break;

		//MOVC A, @A + DPTR
		case 0x93:              /* 1: 1001 0011 */
			util::stream_format(stream, "movc  a,@a+dptr");
			break;

		//SUBB A, #data
		case 0x94:              /* 1: 1001 0100 */
			util::stream_format(stream, "subb  a,#$%02X", params.r8(PC++));
			break;

		//SUBB A, data addr
		case 0x95:              /* 1: 1001 0101 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "subb  a,%s", sym);
			break;

		//Unable to test
		//SUBB A, @R0/@R1       /* 1: 1001 011i */
		case 0x96:
		case 0x97:
			util::stream_format(stream, "subb  a,@r%d", op&1);
			break;

		//SUBB A, R0 to R7      /* 1: 1001 1rrr */
		case 0x98:
		case 0x99:
		case 0x9a:
		case 0x9b:
		case 0x9c:
		case 0x9d:
		case 0x9e:
		case 0x9f:
			util::stream_format(stream, "subb  a,r%d", op&7);
			break;

		//Unable to test
		//ORL C, /bit addr
		case 0xa0:                /* 1: 1010 0000 */
			sym = get_bit_address(params.r8(PC++));
			util::stream_format(stream, "orl   c,/%s", sym);
			break;

		//MOV C, bit addr
		case 0xa2:                /* 1: 1010 0010 */
			sym = get_bit_address(params.r8(PC++));
			util::stream_format(stream, "mov   c,%s", sym);
			break;

		//INC DPTR
		case 0xa3:                /* 1: 1010 0011 */
			util::stream_format(stream, "inc   dptr");
			break;

		//MUL AB
		case 0xa4:                /* 1: 1010 0100 */
			util::stream_format(stream, "mul   ab");
			break;

		//reserved
		case 0xa5:                /* 1: 1010 0101 */
			util::stream_format(stream, "ill/rsv");
			break;

		//Unable to test
		//MOV @R0/@R1, data addr  /* 1: 1010 011i */
		case 0xa6:
		case 0xa7:
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "mov   @r%d,%s", op&1, sym);
			break;

		//MOV R0 to R7, data addr /* 1: 1010 1rrr */
		case 0xa8:
		case 0xa9:
		case 0xaa:
		case 0xab:
		case 0xac:
		case 0xad:
		case 0xae:
		case 0xaf:
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "mov   r%d,%s", op&7, sym);
			break;

		//ANL C,/bit addr
		case 0xb0:                       /* 1: 1011 0000 */
			sym = get_bit_address(params.r8(PC++));
			util::stream_format(stream, "anl   c,/%s", sym);
			break;

		//CPL bit addr
		case 0xb2:                       /* 1: 1011 0010 */
			sym = get_bit_address(params.r8(PC++));
			util::stream_format(stream, "cpl   %s", sym);
			break;

		//Unable to test
		//CPL C
		case 0xb3:                       /* 1: 1011 0011 */
			util::stream_format(stream, "cpl   c");
			break;

		//CJNE A, #data, code addr
		case 0xb4:                       /* 1: 1011 0100 */
			data = params.r8(PC++);
			rel  = params.r8(PC++);
			util::stream_format(stream, "cjne  a,#$%02X,$%04X", data, PC + rel);
			break;

		//CJNE A, data addr, code addr
		case 0xb5:                       /* 1: 1011 0101 */
			sym = get_data_address(params.r8(PC++));
			rel  = params.r8(PC++);
			util::stream_format(stream, "cjne  a,%s,$%04X", sym, PC + rel);
			break;

		//Unable to test
		//CJNE @R0/@R1, #data, code addr /* 1: 1011 011i */
		case 0xb6:
		case 0xb7:
			data = params.r8(PC++);
			rel  = params.r8(PC++);
			util::stream_format(stream, "cjne  @r%d,#$%02X,$%04X", op&1, data, PC + rel);
			break;

		//CJNE R0 to R7, #data, code addr/* 1: 1011 1rrr */
		case 0xb8:
		case 0xb9:
		case 0xba:
		case 0xbb:
		case 0xbc:
		case 0xbd:
		case 0xbe:
		case 0xbf:
			data = params.r8(PC++);
			rel  = params.r8(PC++);
			util::stream_format(stream, "cjne  r%d,#$%02X,$%04X", op&7, data, PC + rel);
			break;

		//PUSH data addr
		case 0xc0:                      /* 1: 1100 0000 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "push  %s", sym);
			break;

		//CLR bit addr
		case 0xc2:                      /* 1: 1100 0010 */
			sym = get_bit_address(params.r8(PC++));
			util::stream_format(stream, "clr   %s", sym);
			break;

		//CLR C
		case 0xc3:                      /* 1: 1100 0011 */
			util::stream_format(stream, "clr   c");
			break;

		//SWAP A
		case 0xc4:                      /* 1: 1100 0100 */
			util::stream_format(stream, "swap  a");
			break;

		//XCH A, data addr
		case 0xc5:                      /* 1: 1100 0101 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "xch   a,%s", sym);
			break;

		//XCH A, @RO/@R1                /* 1: 1100 011i */
		case 0xc6:
		case 0xc7:
			util::stream_format(stream, "xch   a,@r%d", op&1);
			break;

		//XCH A, RO to R7               /* 1: 1100 1rrr */
		case 0xc8:
		case 0xc9:
		case 0xca:
		case 0xcb:
		case 0xcc:
		case 0xcd:
		case 0xce:
		case 0xcf:
			util::stream_format(stream, "xch   a,r%d", op&7);
			break;

		//POP data addr
		case 0xd0:                      /* 1: 1101 0000 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "pop   %s", sym);
			break;

		//SETB bit addr
		case 0xd2:                      /* 1: 1101 0010 */
			sym = get_bit_address(params.r8(PC++));
			util::stream_format(stream, "setb  %s", sym);
			break;

		//SETB C
		case 0xd3:                      /* 1: 1101 0011 */
			util::stream_format(stream, "setb  c");
			break;

		//Unable to test
		//DA A
		case 0xd4:                      /* 1: 1101 0100 */
			util::stream_format(stream, "da    a");
			break;

		//DJNZ data addr, code addr
		case 0xd5:                      /* 1: 1101 0101 */
			sym = get_data_address(params.r8(PC++));
			rel  = params.r8(PC++);
			util::stream_format(stream, "djnz  %s,$%04X", sym, PC + rel);
			flags = STEP_OVER;
			break;

		//XCHD A, @R0/@R1               /* 1: 1101 011i */
		case 0xd6:
		case 0xd7:
			util::stream_format(stream, "xchd  a,@r%d", op&1);
			break;

		//DJNZ R0 to R7,code addr       /* 1: 1101 1rrr */
		case 0xd8:
		case 0xd9:
		case 0xda:
		case 0xdb:
		case 0xdc:
		case 0xdd:
		case 0xde:
		case 0xdf:
			rel = params.r8(PC++);
			util::stream_format(stream, "djnz  r%d,$%04X", op&7, (PC + rel));
			flags = STEP_OVER;
			break;

		//MOVX A,@DPTR
		case 0xe0:                      /* 1: 1110 0000 */
			util::stream_format(stream, "movx  a,@dptr");
			break;

		//Unable to test
		//MOVX A, @R0/@R1               /* 1: 1110 001i */
		case 0xe2:
		case 0xe3:
			util::stream_format(stream, "movx  a,@r%d", op&1);
			break;

		//CLR A
		case 0xe4:                      /* 1: 1110 0100 */
			util::stream_format(stream, "clr   a");
			break;

		//MOV A, data addr
		case 0xe5:                      /* 1: 1110 0101 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "mov   a,%s", sym);
			break;

		//Unable to test
		//MOV A,@RO/@R1                 /* 1: 1110 011i */
		case 0xe6:
		case 0xe7:
			util::stream_format(stream, "mov   a,@r%d", op&1);
			break;

		//MOV A,R0 to R7                /* 1: 1110 1rrr */
		case 0xe8:
		case 0xe9:
		case 0xea:
		case 0xeb:
		case 0xec:
		case 0xed:
		case 0xee:
		case 0xef:
			util::stream_format(stream, "mov   a,r%d", op&7);
			break;

		//MOVX @DPTR,A
		case 0xf0:                      /* 1: 1111 0000 */
			util::stream_format(stream, "movx  @dptr,a");
			break;

		//Unable to test
		//MOVX @R0/@R1,A                /* 1: 1111 001i */
		case 0xf2:
		case 0xf3:
			util::stream_format(stream, "movx  @r%d,a", op&1);
			break;

		//CPL A
		case 0xf4:                      /* 1: 1111 0100 */
			util::stream_format(stream, "cpl   a");
			break;

		//MOV data addr, A
		case 0xf5:                      /* 1: 1111 0101 */
			sym = get_data_address(params.r8(PC++));
			util::stream_format(stream, "mov   %s,a", sym);
			break;

		//MOV @R0/@R1, A                /* 1: 1111 011i */
		case 0xf6:
		case 0xf7:
			util::stream_format(stream, "mov   @r%d,a", op&1);
			break;

		//MOV R0 to R7, A               /* 1: 1111 1rrr */
		case 0xf8:
		case 0xf9:
		case 0xfa:
		case 0xfb:
		case 0xfc:
		case 0xfd:
		case 0xfe:
		case 0xff:
			util::stream_format(stream, "mov   r%d,a", op&7);
			break;

		default:
			util::stream_format(stream, "illegal");
	}

	return (PC - pc) | flags | SUPPORTED;
}

offs_t mcs51_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	unsigned PC = pc;
	uint8_t op = opcodes.r8(PC++);
	return disassemble_op(stream, PC, pc,  opcodes, params, op);
}

i8051_disassembler::i8051_disassembler() : mcs51_disassembler(default_names)
{
}

i8052_disassembler::i8052_disassembler() : mcs51_disassembler(default_names, i8052_names)
{
}

i80c51_disassembler::i80c51_disassembler() : mcs51_disassembler(default_names)
{
}

i80c52_disassembler::i80c52_disassembler() : mcs51_disassembler(default_names, i8052_names, i80c52_names)
{
}

i8xc51fx_disassembler::i8xc51fx_disassembler() : mcs51_disassembler(default_names, i8052_names, i80c52_names, i8xc51fx_names)
{
}

i8xc51gb_disassembler::i8xc51gb_disassembler() : mcs51_disassembler(default_names, i8052_names, i80c52_names, i8xc51fx_names, i8xc51gb_names)
{
}

ds5002fp_disassembler::ds5002fp_disassembler() : mcs51_disassembler(default_names, i8052_names, i80c52_names, ds5002fp_names, i8xc751_names)
{
}
