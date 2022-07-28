// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff

#include "emu.h"
#include "axc51dasm.h"

// Note: addresses >= 0x100 are bit addresses

const axc51_disassembler::mem_info axc51_disassembler::default_names[] = {
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

axc51_disassembler::axc51_disassembler()
{
}

void axc51_disassembler::add_names(const mem_info *info)
{
	for(unsigned int i=0; info[i].addr >= 0; i++)
		m_names[info[i].addr] = info[i].name;
}

u32 axc51_disassembler::opcode_alignment() const
{
	return 1;
}


std::string axc51_disassembler::get_data_address( uint8_t arg ) const
{
	auto i = m_names.find(arg);
	if (i == m_names.end())
		return util::string_format("$%02X", arg);
	else
		return i->second;
}

std::string axc51_disassembler::get_bit_address( uint8_t arg ) const
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

void axc51_disassembler::disassemble_op_ljmp(std::ostream& stream, unsigned &PC, const data_buffer& params)
{
	uint16_t addr = (params.r8(PC++)<<8) & 0xff00;
	addr|= params.r8(PC++);
	util::stream_format(stream, "ljmp  $%04X", addr);
}

void axc51_disassembler::disassemble_op_lcall(std::ostream& stream, unsigned &PC, const data_buffer& params)
{
	uint16_t addr = (params.r8(PC++)<<8) & 0xff00;
	addr|= params.r8(PC++);
	util::stream_format(stream, "lcall $%04X", addr);
}

offs_t axc51_disassembler::disassemble_op(std::ostream &stream, unsigned PC, offs_t pc, const data_buffer &opcodes, const data_buffer &params, uint8_t op)
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
			disassemble_op_ljmp(stream, PC, params);
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
			flags = STEP_COND;
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
			disassemble_op_lcall(stream, PC, params);
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
			flags = STEP_COND;
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
			flags = STEP_COND;
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
			flags = STEP_COND;
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
			flags = STEP_COND;
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
			flags = STEP_COND;
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
			flags = STEP_COND;
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
			flags = STEP_COND;
			break;

		//CJNE A, data addr, code addr
		case 0xb5:                       /* 1: 1011 0101 */
			sym = get_data_address(params.r8(PC++));
			rel  = params.r8(PC++);
			util::stream_format(stream, "cjne  a,%s,$%04X", sym, PC + rel);
			flags = STEP_COND;
			break;

		//Unable to test
		//CJNE @R0/@R1, #data, code addr /* 1: 1011 011i */
		case 0xb6:
		case 0xb7:
			data = params.r8(PC++);
			rel  = params.r8(PC++);
			util::stream_format(stream, "cjne  @r%d,#$%02X,$%04X", op&1, data, PC + rel);
			flags = STEP_COND;
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
			flags = STEP_COND;
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
			flags = STEP_COND;
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
			flags = STEP_COND;
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

offs_t axc51_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	unsigned PC = pc;
	uint8_t op = opcodes.r8(PC++);
	return disassemble_op(stream, PC, pc,  opcodes, params, op);
}

// SOME of these might be AX208 specific, we do not currently hvae enough information to split it into AXC51 / AX208 however
const axc51core_disassembler::mem_info axc51core_disassembler::axc51core_names[] = {

	// SFR Registers
	{ 0x80, "P0" },
	{ 0x81, "SP" }, // Stack Pointer
	{ 0x82, "DPL0" },
	{ 0x83, "DPH0" },
	{ 0x84, "DPL1" },
	{ 0x85, "DPH1" },
	{ 0x86, "DPCON" }, // Data Pointer Configure Register
	{ 0x87, "PCON0" }, // Power Control 0
	{ 0x88, "SDCON0" },
	{ 0x89, "SDCON1" },
	{ 0x8A, "SDCON2" },
	{ 0x8B, "JPGCON4" },
	{ 0x8C, "JPGCON3" },
	{ 0x8D, "JPGCON2" },
	{ 0x8E, "JPGCON1" },
	{ 0x8F, "TRAP" },
	{ 0x90, "P1" },
	{ 0x91, "SDBAUD" },
	{ 0x92, "SDCPTR" },
	{ 0x93, "SDDCNT" },
	{ 0x94, "SDDPTR" },
	{ 0x95, "IE2" }, // Interrupt Enable 2
	{ 0x96, "UARTBAUDH" }, // UART Baud (high)
	{ 0x97, "PWKEN" }, // Port Wakeup Enable
	{ 0x98, "PWKPND" }, //Port Wakeup Flag
	{ 0x99, "PWKEDGE" }, // Port Wakeup Edge
	{ 0x9A, "PIE0" }, // Port Digital Input Enable Control 0
	{ 0x9B, "DBASE" }, // DRAM Base Address Register
	{ 0x9C, "PCON1" }, // Power Control 1
	{ 0x9D, "PIE1" }, // Port Digital Input Enable Control 1
	{ 0x9E, "IRTDATA" }, // IRTCC Communication Data
	{ 0x9F, "IRTCON" }, // IRTCC Control
	{ 0xA0, "P2" },
	{ 0xA1, "GP0" }, // (General Purpose Register 0)
	{ 0xA2, "GP1" }, // (General Purpose Register 1)
	{ 0xA3, "GP2" }, // (General Purpose Register 2)
	{ 0xA4, "GP3" }, // (General Purpose Register 3)
	{ 0xA5, "DACCON" }, // DAC Control Register
	{ 0xA6, "DACLCH" }, // DAC Left Channel
	{ 0xA7, "DACRCH" }, // DAC Right Channel
	{ 0xA8, "IE0" }, // Interrupt Enable 0
	{ 0xA9, "IE1" }, // Interrupt Enable 1
	{ 0xAA, "KEY0" },
	{ 0xAB, "KEY1" },
	{ 0xAC, "TMR3CON" }, // Timer3 Control
	{ 0xAD, "TMR3CNT" }, // Timer3 Counter
	{ 0xAE, "TMR3PR" }, // Timer3 Period
	{ 0xAF, "TMR3PSR" }, // Timer3 Pre-scalar
	{ 0xB0, "P3" },
	{ 0xB1, "GP4" }, // (General Purpose Register 4)
	{ 0xB2, "GP5" }, // (General Purpose Register 5)
	{ 0xB3, "GP6" }, // (General Purpose Register 6)
	{ 0xB4, "P4" },
	{ 0xB5, "GP7" }, // (General Purpose Register 7)
	{ 0xB6, "LCDCON" }, // LCD Control Register (or C6?)
	{ 0xB7, "PLLCON" }, // PLL Configuration
	{ 0xB8, "IP0" }, // Interrupt Priority 0
	{ 0xB9, "IP1" }, // Interrupt Priority 1
	{ 0xBA, "P0DIR" },
	{ 0xBB, "P1DIR" },
	{ 0xBC, "P2DIR" },
	{ 0xBD, "P3DIR" },
	{ 0xBE, "P4DIR" },
	{ 0xBF, "LVDCON" }, // LVD Control Register
	{ 0xC0, "JPGCON0" },
	{ 0xC1, "TMR2CON" }, // Timer2 Control
	{ 0xC2, "JPGCON9" },
	{ 0xC3, "JPGCON5" },
	{ 0xC4, "JPGCON6" },
	{ 0xC5, "JPGCON7" },
	{ 0xC6, "JPGCON8" },
	{ 0xC7, "LCDPR" }, // LCD CS Pulse Width Register
	{ 0xC8, "LCDTCON" }, // LCD WR Pulse Timing Control Register
	{ 0xC9, "USBCON0" },
	{ 0xCA, "USBCON1" },
	{ 0xCB, "USBCON2" },
	{ 0xCC, "USBDATA" },
	{ 0xCD, "USBADR" },
	{ 0xCE, "illegal" },
	{ 0xCF, "MICCON" }, // MIC Control
	{ 0xD0, "PSW" }, // Processor Status Word
	{ 0xD1, "PGCON" }, // Power Gate Control Register
	{ 0xD2, "ADCCON" }, // SARADC Control
	{ 0xD3, "PCON2" }, // Power Control 2
	{ 0xD4, "ADCDATAL" }, // SARADC Buffer Low Byte Control
	{ 0xD5, "ADCDATAH" }, // SARADC Buffer High Byte Control
	{ 0xD6, "SPIDMAADDR" }, // SPI DMA Start Address
	{ 0xD7, "SPIDMACNT" }, // SPI DMA counter
	{ 0xD8, "SPICON" }, // SPI Control
	{ 0xD9, "SPIBUF" }, // SPI Data Buffer
	{ 0xDA, "SPIBAUD" }, // SPI Baud Rate
	{ 0xDB, "CLKCON" }, // Clock Control
	{ 0xDC, "CLKCON1" },
	{ 0xDD, "USBDPDM" },
	{ 0xDE, "LFSRPOLY0" },
	{ 0xDF, "LFSRPOLY1" },
	{ 0xE0, "ACC" },
	{ 0xE1, "TMR1CON" }, // Timer1 Control
	{ 0xE2, "UID0" },
	{ 0xE3, "UID1" },
	{ 0xE4, "UID2" },
	{ 0xE5, "UID3" },
	{ 0xE6, "ER00" }, // ER00 \- ER0 (16-bit)  Extended Registers (used by 16-bit opcodes)
	{ 0xE7, "ER01" }, // ER01 /
	{ 0xE8, "ER10" }, // ER10 \- ER1 (16-bit)
	{ 0xE9, "ER11" }, // ER11 /
	{ 0xEA, "ER20" }, // ER20 \- ER2 (16-bit)
	{ 0xEB, "ER21" }, // ER21 /
	{ 0xEC, "ER30" }, // ER30 \- ER3 (16-bit)
	{ 0xED, "ER31" }, // ER31 /
	{ 0xEE, "ER8" }, // ER8
	{ 0xEF, "illegal" },
	{ 0xF0, "B" },
	{ 0xF1, "HUFFBUF" },
	{ 0xF2, "HUFFSFT" },
	{ 0xF3, "HUFFDCL" },
	{ 0xF4, "HUFFDCH" },
	{ 0xF5, "CRC" },
	{ 0xF6, "LFSRFIFO" },
	{ 0xF7, "WDTCON" }, // Watchdog Control
	{ 0xF8, "TMR0CON" }, // Timer0 Control
	{ 0xF9, "TMR0CNT" }, // Timer0 Counter
	{ 0xFA, "TMR0PR" }, // Timer0 Period
	{ 0xFB, "TMR0PSR" }, // Timer0 Pre-scalar
	{ 0xFC, "UARTSTA" }, // UART Status
	{ 0xFD, "UARTCON" }, // UART Control
	{ 0xFE, "UARTBAUD" }, // UART Baud (low)
	{ 0xFF, "UARTDATA" }, // UART Communication Data

	// Upper Registers

	{ 0x3010, "PUP0" },
	{ 0x3011, "PUP1" },
	{ 0x3012, "PUP2" },
	{ 0x3013, "PUP3" },
	{ 0x3014, "PUP4" },
	{ 0x3015, "PDN0" },
	{ 0x3016, "PDN1" },
	{ 0x3017, "PDN2" },
	{ 0x3018, "PDN3" },
	{ 0x3019, "PDN4" },

	{ 0x3020, "TMR1CNTL" }, // Timer 1 Counter (low)
	{ 0x3021, "TMR1CNTH" }, // Timer 1 Counter (high)
	{ 0x3022, "TMR1PRL" }, // Timer 1 Period (low)
	{ 0x3023, "TMR1PRH" }, // Timer 1 Period (high)
	{ 0x3024, "TMR1PWML" }, // Timer 1 Duty (low)
	{ 0x3025, "TMR1PWMH" }, // Timer 1 Duty (high)

	{ 0x3030, "TMR2CNTL" }, // Timer 2 Counter (low)
	{ 0x3031, "TMR2CNTH" }, // Timer 2 Counter (high)
	{ 0x3032, "TMR2PRL" }, // Timer 2 Period (low)
	{ 0x3033, "TMR2PRH" }, // Timer 2 Period (high)
	{ 0x3034, "TMR2PWML" }, // Timer 2 Duty (low)
	{ 0x3035, "TMR2PWMH" }, // Timer 2 Duty (high)

	{ 0x3040, "ADCBAUD" }, //S ARADC Baud

	{ 0x3050, "USBEP0ADL" },
	{ 0x3051, "USBEP0ADH" },
	{ 0x3052, "USBEP1RXADL" },
	{ 0x3053, "USBEP1RXADH" },
	{ 0x3054, "USBEP1TXADL" },
	{ 0x3055, "USBEP1TXADH" },
	{ 0x3056, "USBEP2RXADL" },
	{ 0x3057, "USBEP2RXADH" },
	{ 0x3058, "USBEP2TXADL" },
	{ 0x3059, "USBEP2TXADH" },

	{ 0x3060, "SFSCON" },
	{ 0x3061, "SFSPID" },
	{ 0x3062, "SFSCNTH" },
	{ 0x3063, "SFSCNTL" },

	{ 0x3070, "DACPTR" }, // DAC DMA Pointer
	{ 0x3071, "DACCNT" }, // DAC DMA Counter

	{ -1 }
};


// based on extracted symbol table, note 0x8000 - 0x8ca3 is likely boot code, interrupt code, kernel etc.
// this should be the same for all ax208 CPUs as they are thought to all use the same internal ROM
const ax208_disassembler::ax208_bios_info ax208_disassembler::bios_call_names[] = {
	{ 0x8000, "entry point" },

	{ 0x8006, "unknown, used" },
	{ 0x8009, "unknown, used" },

	{ 0x8ca4, "_STRCHR" },
	{ 0x8dd6, "_STRLEN" },
	{ 0x8eb7, "_tolower" },
	{ 0x8ec8, "_toupper" },
	{ 0x900f, "_isalpha" },
	{ 0x902a, "_iscntrl" },
	{ 0x9038, "_isdigit" },
	{ 0x9047, "_isalnum" },
	{ 0x906d, "_isgraph" },
	{ 0x907c, "_isprint" },
	{ 0x908b, "_ispunct" },
	{ 0x90bb, "_islower" },
	{ 0x90ca, "_isupper" },
	{ 0x90d9, "_isspace" },
	{ 0x90ec, "_isxdigit" },
	{ 0x91e2, "COPY" },
	{ 0x9208, "SCDIV" },
	{ 0x922a, "CLDPTR" },
	{ 0x9243, "CLDIPTR" },
	{ 0x9267, "CLDOPTR" },
	{ 0x9294, "CLDIOPTR" },
	{ 0x92cb, "CILDPTR" },
	{ 0x92ed, "CILDOPTR" },
	{ 0x9320, "CSTPTR" },
	{ 0x9332, "CSTOPTR" },
	{ 0x9354, "UIDIV" },
	{ 0x93a9, "SIDIV" },
	{ 0x93df, "IILDX" },
	{ 0x93f5, "ILDIX" },
	{ 0x940b, "ILDPTR" },
	{ 0x9436, "ILDIPTR" },
	{ 0x946b, "ILDOPTR" },
	{ 0x94a3, "ILDIOPTR" },
	{ 0x94ef, "IILDPTR" },
	{ 0x9527, "IILDOPTR" },
	{ 0x9574, "ISTPTR" },
	{ 0x9593, "ISTOPTR" },
	{ 0x95c0, "LADD" },
	{ 0x95cd, "LSUB" },
	{ 0x95db, "LMUL" },
	{ 0x9666, "ULDIV" },
	{ 0x96f8, "LAND" },
	{ 0x9705, "LOR" },
	{ 0x9712, "LXOR" },
	{ 0x971f, "LNOT" },
	{ 0x972c, "LNEG" },
	{ 0x973a, "SLCMP" },
	{ 0x9750, "ULCMP" },
	{ 0x9761, "ULSHR" },
	{ 0x9774, "SLSHR" },
	{ 0x9788, "LSHL" },
	{ 0x979b, "LLDPTR" },
	{ 0x97bb, "LLDOPTR" },
	{ 0x97eb, "LSTPTR" },
	{ 0x9805, "LSTOPTR" },
	{ 0x9829, "LILDPTR" },
	{ 0x9849, "LILDOPTR" },
	{ 0x9879, "LLDIPTR" },
	{ 0x9899, "LLDIOPTR" },
	{ 0x98c9, "LLDIDATA" },
	{ 0x98d5, "LLDXDATA" },
	{ 0x98e1, "LLDPDATA" },
	{ 0x98ed, "LLDCODE" },
	{ 0x98fd, "LLDIDATA0" },
	{ 0x990a, "LLDXDATA0" },
	{ 0x9916, "LLDPDATA0" },
	{ 0x9923, "LLDCODE0" },
	{ 0x9933, "LLDPTR0" },
	{ 0x9953, "LLDOPTR0" },
	{ 0x9983, "LLDIIDATA1" },
	{ 0x9985, "LLDIIDATA8" },
	{ 0x998c, "LLDIIDATA" },
	{ 0x99a3, "LLDIXDATA1" },
	{ 0x99a5, "LLDIXDATA8" },
	{ 0x99ac, "LLDIXDATA" },
	{ 0x99d8, "LLDIPDATA1" },
	{ 0x99da, "LLDIPDATA8" },
	{ 0x99e1, "LLDIPDATA" },
	{ 0x99f8, "LILDIDATA1" },
	{ 0x99fa, "LILDIDATA8" },
	{ 0x9a01, "LILDIDATA" },
	{ 0x9a18, "LILDXDATA1" },
	{ 0x9a1a, "LILDXDATA8" },
	{ 0x9a21, "LILDXDATA" },
	{ 0x9a4d, "LILDPDATA1" },
	{ 0x9a4f, "LILDPDATA8" },
	{ 0x9a56, "LILDPDATA" },
	{ 0x9a6d, "LSTIDATA" },
	{ 0x9a79, "LSTXDATA" },
	{ 0x9a85, "LSTPDATA" },
	{ 0x9a91, "LSTKIDATA" },
	{ 0x9aaa, "LSTKXDATA" },
	{ 0x9adb, "LSTKPDATA" },
	{ 0x9af4, "LSTKPTR" },
	{ 0x9b0e, "LSTKOPTR" },
	{ 0x9b32, "BCAST_L" },
	{ 0x9b3b, "OFFX256" },
	{ 0x9b4c, "OFFXADD" },
	{ 0x9b58, "OFFXADD1" },
	{ 0x9b61, "PLDIDATA" },
	{ 0x9b6a, "PLDIIDATA" },
	{ 0x9b7a, "PILDIDATA" },
	{ 0x9b8a, "PSTIDATA" },
	{ 0x9b93, "PLDXDATA" },
	{ 0x9b9c, "PLDIXDATA" },
	{ 0x9bb3, "PILDXDATA" },
	{ 0x9bca, "PSTXDATA" },
	{ 0x9bd3, "PLDPDATA" },
	{ 0x9bdc, "PLDIPDATA" },
	{ 0x9bec, "PILDPDATA" },
	{ 0x9bfc, "PSTPDATA" },
	{ 0x9c05, "PLDCODE" },
	{ 0x9c11, "PLDPTR" },
	{ 0x9c31, "PLDIPTR" },
	{ 0x9c53, "PILDPTR" },
	{ 0x9c75, "PSTPTR" },
	{ 0x9cc4, "PSTPTRR" },
	{ 0x9cf4, "PLDOPTR" },
	{ 0x9d24, "PLDIOPTR" },
	{ 0x9d56, "PILDOPTR" },
	{ 0x9d88, "PSTOPTR" },
	{ 0x9de1, "CCASE" },
	{ 0x9e07, "ICASE" },
	{ 0x9e34, "LCASE" },
	{ 0x9e6e, "ICALL" },
	{ 0x9e72, "ICALL2" },
	{ 0x9e74, "MEMSET" },
	{ 0x9ea0, "LROL" },
	{ 0x9eb4, "LROR" },
	{ 0x9ec8, "SLDIV" },
	{ 0x9f47, "SPI_ENCRYPT_ON3" },
	{ 0x9f5c, "_lshift9" },
	{ 0x9fdc, "SPI_ENCRYPT_CLOSE" },
	{ -1, "unknown" }
};

axc51core_disassembler::axc51core_disassembler() : axc51_disassembler(axc51core_names)
{
}
/* Extended 16-bit Opcodes

Opcode/params        |    Operation                                        | Flags touched
----------------------------------------------------------------------------------------
INCDPi               |                                                     |
                     |    DPTRi = DPTRi + 1                                |
----------------------------------------------------------------------------------------
DECDPi               |                                                     |
                     |    DPTRi = DPTRi - 1                                |
----------------------------------------------------------------------------------------
ADDDPi               |                                                     |
                     |    DPTRi = DPTRi + {R8, B}                          |
----------------------------------------------------------------------------------------
SUBDPi               |                                                     |
                     |    DPTRi = DPTRi - {R8, B}                          |
----------------------------------------------------------------------------------------
INC2DPi              |                                                     |
                     |    DPTRi = DPTRi + 2                                |
----------------------------------------------------------------------------------------
DEC2DPi              |                                                     |
                     |    DPTRi = DPTRi - 2                                |
----------------------------------------------------------------------------------------
ROTR8                |                                                     |
EACC, ER8            |    Rotate Right ACC by R8 &0x3 bits                 |
----------------------------------------------------------------------------------------
ROTL8                |                                                     |
EACC, ER8            |    Rotate Left ACC by R8 &0x3 bits                  |
----------------------------------------------------------------------------------------
ADD16                |                                                     |
ERp, DPTRi, ERn      |    ERp = XRAM + ERn + EC                            |    EZ, EC
DPTRi, ERn, ERp      |    XRAM = ERn + ERp + EC                            |
ERp, ERn, ERm        |    ERp = ERn + ERm + EC                             |
----------------------------------------------------------------------------------------
SUB16                |                                                     |
ERp, DPTRi, ERn      |    ERp = XRAM - ERn - EC                            |    EZ, EC
DPTRi, ERn, ERp      |    XRAM = ERn - ERp - EC                            |
ERp, ERn, ERm        |    ERp = ERn - ERm - EC                             |
----------------------------------------------------------------------------------------
NOT16                |                                                     |
ERn                  |    ERn = ~ERn                                       |
----------------------------------------------------------------------------------------
CLR16                |                                                     |
ERn                  |    ERn = 0                                          |
----------------------------------------------------------------------------------------
INC16                |                                                     |
ERn                  |    ERn = ERn + 1                                    |    EZ
----------------------------------------------------------------------------------------
DEC16                |                                                     |
ERn                  |    ERn = ERn - 1                                    |    EZ
----------------------------------------------------------------------------------------
ANL16                |                                                     |
ERn, DPTRi           |    ERn = XRAM & ERn                                 |    EZ
DPTRi, ERn           |    XRAM = XRAM & ERn                                |
ERn, ERm             |    ERn = ERn & ERm                                  |
----------------------------------------------------------------------------------------
ORL16                |                                                     |
ERn, DPTRi           |    ERn = XRAM | ERn                                 |    EZ
DPTRi, ERn           |    XRAM = XRAM | ERn                                |
ERn, ERm             |    ERn = ERn | ERm                                  |
----------------------------------------------------------------------------------------
XRL16                |                                                     |
ERn, DPTRi           |    ERn = XRAM ^ ERn                                 |    EZ
DPTRi, ERn           |    XRAM = XRAM ^ ERn                                |
ERn, ERm             |    ERn = ERn ^ ERm                                  |
----------------------------------------------------------------------------------------
MOV16                |                                                     |
ERn, DPTRi           |    ERn = XRAM                                       |    EZ
DPTRi, ERn           |    XRAM = ERn                                       |
ERn, ERm             |    ERn = ERm                                        |
----------------------------------------------------------------------------------------
MUL16 (signed)       |                                                     |
ERn, ERm             |    {ERn, ERm} = ERn * ERm                           |
----------------------------------------------------------------------------------------
MULS16 (sign, satur) |                                                     |
ERn, ERm             |    {ERn, ERm} = ERn * ERm                           |
----------------------------------------------------------------------------------------
ROTR16               |                                                     |
ERn, ER8             |    Rotate Right ERn by ER8 & 0xf bits               |
----------------------------------------------------------------------------------------
ROTL16               |                                                     |
ERn, ER8             |    Rotate Left ERn by ER8 & 0xf bits                |
----------------------------------------------------------------------------------------
SHIFTL   (lsl)       |                                                     |
ERn, ER8             |    ERn = ERn >> (ER8 & 0xf)                         |
----------------------------------------------------------------------------------------
SHIFTR   (asr)       |                                                     |
ERn, ER8             |    ERn = ERn >> (ER8 & 0xf)                         |
----------------------------------------------------------------------------------------
ADD16 (saturate)     |                                                     |
ERp, DPTRi, ERn      |    ERp = XRAM + ERn + EC                            |    EZ, EC
DPTRi, ERn, ERp      |    XRAM = ERn + ERp + EC                            |
ERp, ERn, ERm        |    ERp = ERn + ERm + EC                             |
----------------------------------------------------------------------------------------
SUB16 (saturate)     |                                                     |
ERp, DPTRi, ERn      |    ERp = XRAM - ERn - EC                            |    EZ, EC
DPTRi, ERn, ERp      |    XRAM = ERn - ERp - EC                            |
ERp, ERn, ERm        |    ERp = ERn - ERm - EC                             |
----------------------------------------------------------------------------------------
SWAP16               |                                                     |
ERn                  |    Swap upper and lower 8-bits of ERn               |
----------------------------------------------------------------------------------------

access to 16-bit registers is mapped in SFR space, from 0xE6 (note, changing SFR bank does NOT update the actual registers)

ERn - 16-bit register ER0-ER3 (as data?)
ERm - 16-bit register ER0-ER3 (as data?)
ERp - 16-bit register ER0-ER3 (as pointer?)
EACC = 8-bit accumulator (same as regular opcodes?)
EZ = Zero flag (same as regular opcodes?)
EC = Carry flag  (same as regular opcodes?)

*/

offs_t axc51core_disassembler::disassemble_extended_a5_0e(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params)
{
	uint32_t flags = 0;
	uint8_t prm2 = params.r8(PC++);

	switch (prm2)
	{

	case 0x00: case 0x01: case 0x04: case 0x05: case 0x08: case 0x09: case 0x0c: case 0x0d:
	case 0x10: case 0x11: case 0x14: case 0x15: case 0x18: case 0x19: case 0x1c: case 0x1d:
	case 0x20: case 0x21: case 0x24: case 0x25: case 0x28: case 0x29: case 0x2c: case 0x2d:
	case 0x30: case 0x31: case 0x34: case 0x35: case 0x38: case 0x39: case 0x3c: case 0x3d:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		util::stream_format(stream, "ADD16 ER%01x, EDP%01x, ER%01x", p, i, n);
		break;
	}

	case 0x02: case 0x03: case 0x06: case 0x07: case 0x0a: case 0x0b: case 0x0e: case 0x0f:
	case 0x12: case 0x13: case 0x16: case 0x17: case 0x1a: case 0x1b: case 0x1e: case 0x1f:
	case 0x22: case 0x23: case 0x26: case 0x27: case 0x2a: case 0x2b: case 0x2e: case 0x2f:
	case 0x32: case 0x33: case 0x36: case 0x37: case 0x3a: case 0x3b: case 0x3e: case 0x3f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		util::stream_format(stream, "ADD16 EDP%01x, ER%01x, ER%01x", i, n, p);
		break;
	}

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t m = (prm2 & 0x03) >> 0;
		util::stream_format(stream, "ADD16 ER%01x, ER%01x, ER%01x", p, n, m);
		break;
	}

	default:
		util::stream_format(stream, "illegal ax208 a5 0e $%02X", prm2);
		break;
	}

	return (PC - pc) | flags | SUPPORTED;
}

offs_t axc51core_disassembler::disassemble_extended_a5_0f(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params)
{
	uint32_t flags = 0;
	uint8_t prm2 = params.r8(PC++);

	switch (prm2)
	{

	case 0x00: case 0x01: case 0x04: case 0x05: case 0x08: case 0x09: case 0x0c: case 0x0d:
	case 0x10: case 0x11: case 0x14: case 0x15: case 0x18: case 0x19: case 0x1c: case 0x1d:
	case 0x20: case 0x21: case 0x24: case 0x25: case 0x28: case 0x29: case 0x2c: case 0x2d:
	case 0x30: case 0x31: case 0x34: case 0x35: case 0x38: case 0x39: case 0x3c: case 0x3d:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		util::stream_format(stream, "SUB16 ER%01x, EDP%01x, ER%01x", p, i, n);
		break;
	}

	case 0x02: case 0x03: case 0x06: case 0x07: case 0x0a: case 0x0b: case 0x0e: case 0x0f:
	case 0x12: case 0x13: case 0x16: case 0x17: case 0x1a: case 0x1b: case 0x1e: case 0x1f:
	case 0x22: case 0x23: case 0x26: case 0x27: case 0x2a: case 0x2b: case 0x2e: case 0x2f:
	case 0x32: case 0x33: case 0x36: case 0x37: case 0x3a: case 0x3b: case 0x3e: case 0x3f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		util::stream_format(stream, "SUB16 EDP%01x, ER%01x, ER%01x", i, n, p);
		break;
	}

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t m = (prm2 & 0x03) >> 0;
		util::stream_format(stream, "SUB16 ER%01x, ER%01x, ER%01x", p, n, m);
		break;
	}


	default:
		util::stream_format(stream, "illegal ax208 a5 0f $%02X", prm2);
		break;
	}

	return (PC - pc) | flags | SUPPORTED;
}

offs_t axc51core_disassembler::disassemble_extended_a5_d0(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params)
{
	uint32_t flags = 0;
	uint8_t prm2 = params.r8(PC++);

	switch (prm2)
	{

	case 0x00: case 0x01: case 0x04: case 0x05: case 0x08: case 0x09: case 0x0c: case 0x0d:
	case 0x10: case 0x11: case 0x14: case 0x15: case 0x18: case 0x19: case 0x1c: case 0x1d:
	case 0x20: case 0x21: case 0x24: case 0x25: case 0x28: case 0x29: case 0x2c: case 0x2d:
	case 0x30: case 0x31: case 0x34: case 0x35: case 0x38: case 0x39: case 0x3c: case 0x3d:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		util::stream_format(stream, "ADDS16 ER%01x, EDP%01x, ER%01x", p, i, n);
		break;
	}

	case 0x02: case 0x03: case 0x06: case 0x07: case 0x0a: case 0x0b: case 0x0e: case 0x0f:
	case 0x12: case 0x13: case 0x16: case 0x17: case 0x1a: case 0x1b: case 0x1e: case 0x1f:
	case 0x22: case 0x23: case 0x26: case 0x27: case 0x2a: case 0x2b: case 0x2e: case 0x2f:
	case 0x32: case 0x33: case 0x36: case 0x37: case 0x3a: case 0x3b: case 0x3e: case 0x3f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		util::stream_format(stream, "ADDS16 EDP%01x, ER%01x, ER%01x", i, n, p);
		break;
	}

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t m = (prm2 & 0x03) >> 0;
		util::stream_format(stream, "ADDS16 ER%01x, ER%01x, ER%01x", p, n, m);
		break;
	}

	default:
		util::stream_format(stream, "illegal ax208 a5 d0 $%02X", prm2);
		break;
	}

	return (PC - pc) | flags | SUPPORTED;
}

offs_t axc51core_disassembler::disassemble_extended_a5_d1(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params)
{
	uint32_t flags = 0;
	uint8_t prm2 = params.r8(PC++);

	switch (prm2)
	{

	case 0x00: case 0x01: case 0x04: case 0x05: case 0x08: case 0x09: case 0x0c: case 0x0d:
	case 0x10: case 0x11: case 0x14: case 0x15: case 0x18: case 0x19: case 0x1c: case 0x1d:
	case 0x20: case 0x21: case 0x24: case 0x25: case 0x28: case 0x29: case 0x2c: case 0x2d:
	case 0x30: case 0x31: case 0x34: case 0x35: case 0x38: case 0x39: case 0x3c: case 0x3d:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		util::stream_format(stream, "SUBS16 ER%01x, EDP%01x, ER%01x", p, i, n);
		break;
	}

	case 0x02: case 0x03: case 0x06: case 0x07: case 0x0a: case 0x0b: case 0x0e: case 0x0f:
	case 0x12: case 0x13: case 0x16: case 0x17: case 0x1a: case 0x1b: case 0x1e: case 0x1f:
	case 0x22: case 0x23: case 0x26: case 0x27: case 0x2a: case 0x2b: case 0x2e: case 0x2f:
	case 0x32: case 0x33: case 0x36: case 0x37: case 0x3a: case 0x3b: case 0x3e: case 0x3f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		util::stream_format(stream, "SUBS16 EDP%01x, ER%01x, ER%01x", i, n, p);
		break;
	}

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t m = (prm2 & 0x03) >> 0;
		util::stream_format(stream, "SUBS16 ER%01x, ER%01x, ER%01x", p, n, m);
		break;
	}

	default:
		util::stream_format(stream, "illegal ax208 a5 d1 $%02X", prm2);
		break;
	}

	return (PC - pc) | flags | SUPPORTED;
}


offs_t axc51core_disassembler::disassemble_extended_a5(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params)
{
	uint32_t flags = 0;
	uint8_t prm = params.r8(PC++);

	switch (prm)
	{
	case 0x00:
		util::stream_format(stream, "INCDP0");
		break;

	case 0x01:
		util::stream_format(stream, "INCDP1");
		break;

	case 0x02:
		util::stream_format(stream, "DECDP0");
		break;

	case 0x03:
		util::stream_format(stream, "DECDP1");
		break;

	case 0x04:
		util::stream_format(stream, "ADDDP0");
		break;

	case 0x05:
		util::stream_format(stream, "ADDDP1");
		break;

	case 0x06:
		util::stream_format(stream, "SUBDP0");
		break;

	case 0x07:
		util::stream_format(stream, "SUBDP1");
		break;

	case 0x08:
		util::stream_format(stream, "INC2DP0");
		break;

	case 0x09:
		util::stream_format(stream, "INC2DP1");
		break;

	case 0x0a:
		util::stream_format(stream, "DEC2DP0");
		break;

	case 0x0b:
		util::stream_format(stream, "DEC2DP1");
		break;

	case 0x0c:
		util::stream_format(stream, "ROTR8 EACC, ER8");
		break;

	case 0x0d:
		util::stream_format(stream, "ROTL8 EACC, ER8");
		break;

	case 0x0e: // ADD16
		return disassemble_extended_a5_0e(stream, PC, pc, opcodes, params);

	case 0x0f: // SUB16
		return disassemble_extended_a5_0f(stream, PC, pc, opcodes, params);

	case 0x10: case 0x14: case 0x18: case 0x1c:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "NOT16 ER%01x", n);
		break;
	}

	case 0x11: case 0x15: case 0x19: case 0x1d:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "CLR16 ER%01x", n);
		break;
	}

	case 0x12: case 0x16: case 0x1a: case 0x1e:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "INC16 ER%01x", n);
		break;
	}

	case 0x13: case 0x17: case 0x1b: case 0x1f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "DEC16 ER%01x", n);
		break;
	}

	case 0x20: case 0x21: case 0x24: case 0x25: case 0x28: case 0x29: case 0x2c: case 0x2d:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		util::stream_format(stream, "ANL16 ER%01x, EDP%01x", n, i);
		break;
	}

	case 0x22: case 0x23: case 0x26: case 0x27: case 0x2a: case 0x2b: case 0x2e: case 0x2f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		util::stream_format(stream, "ANL16 EDP%01x, ER%01x", i, n);
		break;
	}

	case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37: case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		util::stream_format(stream, "ANL16 ER%01x, ER%01x", n, m);
		break;
	}

	case 0x40: case 0x41: case 0x44: case 0x45: case 0x48: case 0x49: case 0x4c: case 0x4d:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		util::stream_format(stream, "ORL16 ER%01x, EDP%01x", n, i);
		break;
	}

	case 0x42: case 0x43: case 0x46: case 0x47: case 0x4a: case 0x4b: case 0x4e: case 0x4f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		util::stream_format(stream, "ORL16 EDP%01x, ER%01x", i, n);
		break;
	}

	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		util::stream_format(stream, "ORL16 ER%01x, ER%01x", n, m);
		break;
	}

	case 0x60: case 0x61: case 0x64: case 0x65: case 0x68: case 0x69: case 0x6c: case 0x6d:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		util::stream_format(stream, "XRL16 ER%01x, EDP%01x", n, i);
		break;
	}

	case 0x62: case 0x63: case 0x66: case 0x67: case 0x6a: case 0x6b: case 0x6e: case 0x6f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		util::stream_format(stream, "XRL16 EDP%01x, ER%01x", i, n);
		break;
	}

	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		util::stream_format(stream, "XRL16 ER%01x, ER%01x", n, m);
		break;
	}

	case 0x80: case 0x81: case 0x84: case 0x85: case 0x88: case 0x89: case 0x8c: case 0x8d:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		util::stream_format(stream, "MOV16 ER%01x, EDP%01x", n, i);
		break;
	}

	case 0x82: case 0x83: case 0x86: case 0x87: case 0x8a: case 0x8b: case 0x8e: case 0x8f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		util::stream_format(stream, "MOV16 EDP%01x, ER%01x", i, n);
		break;
	}

	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97: case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		util::stream_format(stream, "MOV16 ER%01x, ER%01x", n, m);
		break;
	}

	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7: case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		util::stream_format(stream, "MUL16 ER%01x, ER%01x", n, m);
		break;
	}

	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7: case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		util::stream_format(stream, "MULS16 ER%01x, ER%01x", n, m);
		break;
	}

	case 0xc0: case 0xc4: case 0xc8: case 0xcc:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "ROTR16 ER%01x, ER8", n);
		break;
	}

	case 0xc1: case 0xc5: case 0xc9: case 0xcd:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "ROTL16 ER%01x, ER8", n);
		break;
	}

	case 0xc2: case 0xc6: case 0xca: case 0xce:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "SHIFTL ER%01x, ER8", n);
		break;
	}

	case 0xc3: case 0xc7: case 0xcb: case 0xcf:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "SHIFTA ER%01x, ER8", n);
		break;
	}

	case 0xd0: // ADDS16
		return disassemble_extended_a5_d0(stream, PC, pc, opcodes, params);

	case 0xd1: // SUBS16
		return disassemble_extended_a5_d1(stream, PC, pc, opcodes, params);

	case 0xd2: case 0xd6: case 0xda: case 0xde:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "SWAP16 ER%01x", n);
		break;
	}

	case 0xd3: case 0xd4: case 0xd5: case 0xd7: case 0xd8: case 0xd9: case 0xdb: case 0xdc: case 0xdd: case 0xdf:
		util::stream_format(stream, "invalid ax208 a5 $%02X", prm);
		break;

	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
		util::stream_format(stream, "invalid ax208 a5 $%02X", prm);
		break;

	case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7: case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
		util::stream_format(stream, "invalid ax208 a5 $%02X", prm);
		break;

	default:
		util::stream_format(stream, "unknown ax208 a5 $%02X", prm);
		break;
	}

	return (PC - pc) | flags | SUPPORTED;
}


offs_t axc51core_disassembler::disassemble_op(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params, uint8_t op)
{
	uint32_t flags = 0;

	switch (op)
	{
	case 0xa5:
		return disassemble_extended_a5(stream, PC, pc, opcodes, params);

	default:
		return axc51_disassembler::disassemble_op(stream, PC, pc, opcodes, params, op);
	}

	return (PC - pc) | flags | SUPPORTED;
}


ax208_disassembler::ax208_disassembler() : axc51core_disassembler(axc51core_names)
{
}

void ax208_disassembler::disassemble_op_ljmp(std::ostream& stream, unsigned &PC, const data_buffer& params)
{
	uint16_t addr = (params.r8(PC++) << 8) & 0xff00;
	addr |= params.r8(PC++);
	if ((addr >= 0x8000) && (addr < 0xa000))
	{
		int i = 0;
		int lookaddr = -1;
		const char* lookname;
		do
		{
			lookaddr = bios_call_names[i].addr;
			lookname = bios_call_names[i].name;

			if (lookaddr == addr)
				break;

			i++;
		} while (lookaddr != -1);

		util::stream_format(stream, "ljmp  $%04X (%s)", addr, lookname);
	}
	else
	{
		util::stream_format(stream, "ljmp  $%04X", addr);
	}
}

void ax208_disassembler::disassemble_op_lcall(std::ostream& stream, unsigned &PC, const data_buffer& params)
{
	uint16_t addr = (params.r8(PC++)<<8) & 0xff00;
	addr|= params.r8(PC++);
	if ((addr >= 0x8000) && (addr < 0xa000))
	{
		int i = 0;
		int lookaddr = -1;
		const char* lookname;
		do
		{
			lookaddr = bios_call_names[i].addr;
			lookname = bios_call_names[i].name;

			if (lookaddr == addr)
				break;

			i++;
		} while (lookaddr != -1);

		util::stream_format(stream, "lcall $%04X (%s)", addr, lookname);
	}
	else
	{
		util::stream_format(stream, "lcall $%04X", addr);
	}
}

