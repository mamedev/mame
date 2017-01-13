// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************************************************

    Toshiba TLCS-870 Series MCUs

	The TLCS-870/X expands on this instruction set using the same base encoding.

	The TLCS-870/C appears to have a completely different encoding.

	loosely baesd on the tlcs90 core by Luca Elia

*************************************************************************************************************/



#include "emu.h"
#include "debugger.h"
#include "tlcs870.h"

// di, ei, j, and test are just 'alias' opcodes
static const char *const op_names[] = {
	"??",
	"call", "callp", "callv", "clr", "cpl",
	"daa", "das", "dec", /*"di",*/ "div",
	/*"ei",*/
	"inc",
	/*"j",*/ "jp", "jr", "jrs",
	"ld", "ldw",
	"mcmp", "mul",
	"nop",
	"pop", "push",
	"ret", "reti", "retn", "rolc", "rold", "rorc", "rord",
	"set", "shlc", "shrc", "swap", "swi",
	/*"test",*/ "xch",
	// ALU operations
	"addc",
	"add",
	"subb",
	"sub",
	"and",
	"xor",
	"or",
	"cmp",
};




static const char *const reg8[] = {
	"A",
	"W",
	"C",
	"B",
	"E",
	"D",
	"L",
	"H"
};

static const char *const type_x[] = {
	"(x)",
	"(PC+A)",
	"(DE)",
	"(HL)",
	"(HL+d)",
	"(HL+C)",
	"(HL+)",
	"(-HL)",
};

static const char *const conditions[] = {
	"EQ/Z",
	"NE/NZ",
	"LT/CS",
	"GE/CC",
	"LE",
	"GT",
	"T",
	"F",
};

static const char *const reg16[] = {
	"WA",
	"BC",
	"DE",
	"HL"
};

static const char *const reg16p[] = {
	"DE",
	"HL"
};



const device_type TMP87PH40AN = &device_creator<tmp87ph40an_device>;

static ADDRESS_MAP_START(tmp87ph40an_mem, AS_PROGRAM, 8, tlcs870_device)
	AM_RANGE(0x0000, 0x003f) AM_RAM
	AM_RANGE(0x0040, 0x023f) AM_RAM AM_SHARE("intram") // register banks + internal RAM, not, code execution NOT allowed here
	AM_RANGE(0x0f80, 0x0fff) AM_RAM // DBR
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


tlcs870_device::tlcs870_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source, address_map_constructor program_map)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0, program_map)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0)
{
}


tmp87ph40an_device::tmp87ph40an_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tlcs870_device(mconfig, TMP87PH40AN, "TMP87PH40AN", tag, owner, clock, "tmp87ph40an", __FILE__, ADDRESS_MAP_NAME(tmp87ph40an_mem))
{
}

uint8_t  tlcs870_device::RM8 (uint32_t a)    { return m_program->read_byte( a ); }
uint16_t tlcs870_device::RM16(uint32_t a)    { return RM8(a) | (RM8( (a+1) & 0xffff ) << 8); }

void tlcs870_device::WM8 (uint32_t a, uint8_t  v)    { m_program->write_byte( a, v ); }
void tlcs870_device::WM16(uint32_t a, uint16_t v)    { WM8(a,v);    WM8( (a+1) & 0xffff, v >> 8); }

uint8_t  tlcs870_device::RX8 (uint32_t a, uint32_t base)   { return m_program->read_byte( base | a ); }
uint16_t tlcs870_device::RX16(uint32_t a, uint32_t base)   { return RX8(a,base) | (RX8( (a+1) & 0xffff, base ) << 8); }

void tlcs870_device::WX8 (uint32_t a, uint8_t  v, uint32_t base)   { m_program->write_byte( base | a, v ); }
void tlcs870_device::WX16(uint32_t a, uint16_t v, uint32_t base)   { WX8(a,v,base);   WX8( (a+1) & 0xffff, v >> 8, base); }

uint8_t  tlcs870_device::READ8() { uint8_t b0 = RM8( m_addr++ ); m_addr &= 0xffff; return b0; }
uint16_t tlcs870_device::READ16()    { uint8_t b0 = READ8(); return b0 | (READ8() << 8); }

void tlcs870_device::decode()
{
	m_op = 0;
	m_param1_type = 0;
	m_param1 = 0;
	m_param2_type = 0;
	m_param2 = 0;
	m_bitpos = 0;

	uint8_t b0;
	uint8_t b1;
//	uint8_t b2;
	
	b0 = READ8();


	switch (b0)
	{
	case 0x00:
		m_op = NOP;
		// NOP;
		break;

	case 0x01:
		// SWAP A
		m_op = SWAP;

		m_param1_type = REG_8BIT;
		m_param1 = 0; // A
		break;

	case 0x02:
		// MUL W,A
		m_op = MUL;

		m_param1_type = REG_8BIT;
		m_param1 = 1; // W

		m_param2_type = REG_8BIT;
		m_param2 = 0; // A
		break;

	case 0x03:
		// DIV WA,C
		m_op = DIV;

		m_param1_type = REG_16BIT;
		m_param1 = 0; // WA

		m_param2_type = REG_8BIT;
		m_param2 = 2; // C
		break;

	case 0x04:
		// RETI
		m_op = RETI;
		break;

	case 0x05:
		// RET
		m_op = RET;
		break;

	case 0x06:
		// POP PSW
		m_op = POP;
		m_param1_type = PROGRAMSTATUSWORD;
		break;

	case 0x07:
		// PUSH PSW:
		m_op = PUSH;
		m_param1_type = PROGRAMSTATUSWORD;
		break;

	case 0x08:
	case 0x09:
		// unused?
		break;

	case 0x0a:
		// DAA A
		m_op = DAA;

		m_param1_type = REG_8BIT;
		m_param1 = 0; // A
		break;

	case 0x0b:
		// DAS A
		m_op = DAS;

		m_param1_type = REG_8BIT;
		m_param1 = 0; // A
		break;

	case 0x0c:
		// CLR CF
		m_op = CLR;

		m_param1_type = CARRYFLAG; // 16-bit register
		//m_param1 = 0;
		break;

	case 0x0d:
		// SET CF
		m_op = SET;

		m_param1_type = CARRYFLAG; // 16-bit register
		//m_param1 = 0;
		break;

	case 0x0e:
		// CPL CF
		m_op = CPL;

		m_param1_type = CARRYFLAG; // 16-bit register
		//m_param1 = 0;
		break;

	case 0x0f:
		// LD RBS,n
		m_op = LD;

		m_param1_type = REGISTERBANK; // 16-bit register
		//m_param1 = 0;

		m_param2_type = ABSOLUTE_VAL_8;
		m_param2 = READ8();

		break;

	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
		// INC rr
		m_op = INC;

		m_param1_type = REG_16BIT; // 16-bit register
		m_param1 = b0&3;

		break;

	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
		// LD rr,mn
		m_op = LD;

		m_param1_type = REG_16BIT; // 16-bit register
		m_param1 = b0&3;

		m_param2_type = ABSOLUTE_VAL_16; // absolute value
		m_param2 = READ16(); // 16-bit

		break;

	case 0x18:
	case 0x19:
	case 0x1a:
	case 0x1b:
		// DEC rr
		m_op = DEC;

		m_param1_type = REG_16BIT; // 16-bit register
		m_param1 = b0&3;

		break;

	case 0x1c:
		// SHLC A
		m_op = SHLC;

		m_param1_type = REG_8BIT;
		m_param1 = 0; // A
		break;

	case 0x1d:
		// SHRC A
		m_op = SHRC;

		m_param1_type = REG_8BIT;
		m_param1 = 0; // A
		break;

	case 0x1e:
		// ROLC A
		m_op = ROLC;

		m_param1_type = REG_8BIT;
		m_param1 = 0; // A
		break;

	case 0x1f:
		// RORC A
		m_op = RORC;

		m_param1_type = REG_8BIT;
		m_param1 = 0; // A
		break;

	case 0x20:
		// INC (x)
		m_op = INC;
		m_param1_type = ADDR_IN_IMM_X;
		m_param1 = READ8();
		break;

	case 0x21:
		// INC (HL)
		m_op = INC;
		m_param1_type = ADDR_IN_HL;
		//m_param1 = 0;
		break;

	case 0x22:
		// LD A,(x)
		m_op = LD;
		m_param2_type = ADDR_IN_IMM_X;
		m_param2 = READ8();

		m_param1_type = REG_8BIT;
		m_param1 = 0; // A
		break;

	case 0x23:
		// LD A,(HL)
		m_op = LD;
		m_param2_type = ADDR_IN_HL;
		//m_param2 = 0;

		m_param1_type = REG_8BIT;
		m_param1 = 0; // A
		break;

	case 0x24:
		// LDW (x),mn
		m_op = LDW;
		m_param1_type = ADDR_8BIT; // 8-bit memory address
		m_param1 = READ8();

		m_param2_type = ABSOLUTE_VAL_16; // absolute value
		m_param2 = READ16();
		break;

	case 0x25:
		// LDW (HL),mn
		m_op = LDW;
		m_param1_type = ADDR_IN_HL;
		//m_param1 = 0;

		m_param2_type = ABSOLUTE_VAL_16;
		m_param2 = READ16();
		break;


	case 0x26:		
		// LD (x),(y)
		m_op = LD;
		m_param2_type = ADDR_8BIT;
		m_param2 = READ8();

		m_param1_type = ADDR_8BIT;
		m_param1 = READ8();
		break;

	case 0x27:
		// unused
		break;

	case 0x28:
		// DEC (x)
		m_op = DEC;
		m_param1_type = ADDR_IN_IMM_X;
		m_param1 = READ8();
		break;

	case 0x29:
		// DEC (HL)
		m_op = DEC;
		m_param1_type = ADDR_IN_HL;
		//m_param1 = 0;
		break;

	case 0x2a:
		// LD (x),A
		m_op = LD;
		m_param1_type = ADDR_IN_IMM_X;
		m_param1 = READ8();

		m_param2_type = REG_8BIT;
		m_param2 = 0; // A

		break;

	case 0x2b:
		// LD (HL),A
		m_op = LD;
		m_param1_type = ADDR_IN_HL;
		//m_param1 = 0;

		m_param2_type = REG_8BIT;
		m_param2 = 0; // A
		break;

	case 0x2c:
		// LD (x),n
		m_op = LD;
		m_param1_type = ADDR_8BIT; // 8-bit memory address
		m_param1 = READ8();

		m_param2_type = ABSOLUTE_VAL_8; // absolute value
		m_param2 = READ8();

		break;

	case 0x2d:
		// LD (HL),n
		m_op = LD;

		m_param1_type = ADDR_IN_16BITREG; // memory address in 16-bit register
		m_param1 = 3; // (HL)

		m_param2_type = ABSOLUTE_VAL_8; // absolute value
		m_param2 = READ8();

		break;

	case 0x2e:
		// CLR (x)
		m_op = CLR;
		m_param1_type = ADDR_8BIT; // 8-bit memory address
		m_param1 = READ8();

		break;

	case 0x2f:
		// CLR (HL)
		m_op = CLR;
		m_param1_type = ADDR_IN_16BITREG; // memory address in 16-bit register
		m_param1 = 3; // (HL)

		break;

	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
		// LD r,n

		m_op = LD;

		m_param1_type = REG_8BIT; // 8-bit register register
		m_param1 = b0&7;

		m_param2_type = ABSOLUTE_VAL_8; // absolute value
		m_param2 = READ8();


		break;

	case 0x38:
	case 0x39:
	case 0x3a:
	case 0x3b:
	case 0x3c:
	case 0x3d:
	case 0x3e:
	case 0x3f:
		break;

	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	case 0x46:
	case 0x47:
		// SET (x).b
		b1 = READ8();
#if 0
		// this is just an alias
		if ((b0 == 0x40) && (b1 == 0x3a))
		{
			// EI 'Assembler expansion machine instruction'
			break;
		}
#endif
		m_op = SET;

		m_param1_type = ADDR_IN_IMM_X | BITPOS;
		m_param1 = b1;
		m_bitpos = b0 & 7;

		break;

	case 0x48:
	case 0x49:
	case 0x4a:
	case 0x4b:
	case 0x4c:
	case 0x4d:
	case 0x4e:
	case 0x4f:
		// CLR (x).b
		b1 = READ8();
#if 0
		// this is just an alias
		if ((b0 == 0x48) && (b1 == 0x3a))
		{
			// DI 'Assembler expansion machine instruction'
			break;
		}
#endif
		m_op = CLR;

		m_param1_type = ADDR_IN_IMM_X | BITPOS;
		m_param1 = b1;
		m_bitpos = b0 & 7;

		break;

	case 0x50:
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
		// LD A,r  0101 0rrr
		m_op = LD;
		m_param1_type = REG_8BIT;
		m_param1 = 0; // A

		m_param2_type = REG_8BIT;
		m_param2 = b0 & 0x7;	
		
		break;

	case 0x58:
	case 0x59:
	case 0x5a:
	case 0x5b:
	case 0x5c:
	case 0x5d:
	case 0x5e:
	case 0x5f:
		// LD r,A  0101 1rrr
		m_op = LD;
		m_param2_type = REG_8BIT;
		m_param2 = 0; // A

		m_param1_type = REG_8BIT;
		m_param1 = b0 & 0x7;	
		break;

	case 0x60:
	case 0x61:
	case 0x62:
	case 0x63:
	case 0x64:
	case 0x65:
	case 0x66:
	case 0x67:
		// INC r
		m_op = INC;
		m_param1_type = REG_8BIT;
		m_param1 = b0 & 0x7;
		break;

	case 0x68:
	case 0x69:
	case 0x6a:
	case 0x6b:
	case 0x6c:
	case 0x6d:
	case 0x6e:
	case 0x6f:
		// DEC r
		m_op = DEC;
		m_param1_type = REG_8BIT;
		m_param1 = b0 & 0x7;
		break;

	case 0x70:
	case 0x71:
	case 0x72:
	case 0x73:
	case 0x74:
	case 0x75:
	case 0x76:
	case 0x77:
		// (ALU OP) A,n
		m_op = (b0 & 0x7)+ALU_ADDC;
		
		m_param1_type = REG_8BIT;
		m_param1 = 0; // A

		m_param2_type = ABSOLUTE_VAL_8;
		m_param2 = READ8();

		break;

	case 0x78:
	case 0x79:
	case 0x7a:
	case 0x7b:
	case 0x7c:
	case 0x7d:
	case 0x7e:
	case 0x7f:
		// (ALU OP) A,(x)
		m_op = (b0 & 0x7)+ALU_ADDC;
		
		m_param1_type = REG_8BIT;
		m_param1 = 0; // A

		m_param2_type = ADDR_IN_IMM_X;
		m_param2 = READ8();

		break;

	case 0x80:
	case 0x81:
	case 0x82:
	case 0x83:
	case 0x84:
	case 0x85:
	case 0x86:
	case 0x87:
	case 0x88:
	case 0x89:
	case 0x8a:
	case 0x8b:
	case 0x8c:
	case 0x8d:
	case 0x8e:
	case 0x8f:
	case 0x90:
	case 0x91:
	case 0x92:
	case 0x93:
	case 0x94:
	case 0x95:
	case 0x96:
	case 0x97:
	case 0x98:
	case 0x99:
	case 0x9a:
	case 0x9b:
	case 0x9c:
	case 0x9d:
	case 0x9e:
	case 0x9f:
	{
		// JRS T,a
		m_op = JRS;

		m_param1_type = CONDITIONAL;
		m_param1 = 6;

		m_param2_type = ABSOLUTE_VAL_16;

		int val = b0 & 0x1f;
		if (val & 0x10) val -= 0x20;

		m_param2 = m_addr -1 + 2 + val;

		break;
	}
	case 0xa0:
	case 0xa1:
	case 0xa2:
	case 0xa3:
	case 0xa4:
	case 0xa5:
	case 0xa6:
	case 0xa7:
	case 0xa8:
	case 0xa9:
	case 0xaa:
	case 0xab:
	case 0xac:
	case 0xad:
	case 0xae:
	case 0xaf:
	case 0xb0:
	case 0xb1:
	case 0xb2:
	case 0xb3:
	case 0xb4:
	case 0xb5:
	case 0xb6:
	case 0xb7:
	case 0xb8:
	case 0xb9:
	case 0xba:
	case 0xbb:
	case 0xbc:
	case 0xbd:
	case 0xbe:
	case 0xbf:
	{
		// JRS F,a
		m_op = JRS;

		m_param1_type = CONDITIONAL;
		m_param1 = 7;

		m_param2_type = ABSOLUTE_VAL_16;

		int val = b0 & 0x1f;
		if (val & 0x10) val -= 0x20;

		m_param2 = m_addr -1 + 2 + val;

		break;
	}

	case 0xc0:
	case 0xc1:
	case 0xc2:
	case 0xc3:
	case 0xc4:
	case 0xc5:
	case 0xc6:
	case 0xc7:
	case 0xc8:
	case 0xc9:
	case 0xca:
	case 0xcb:
	case 0xcc:
	case 0xcd:
	case 0xce:
	case 0xcf:
		// CALLV n
		m_op = CALLV;

		m_param1_type = MEMVECTOR_16BIT;

		m_param1 = 0xffc0 + ((b0 & 0xf) * 2);

		break;

	case 0xd0:
	case 0xd1:
	case 0xd2:
	case 0xd3:
	case 0xd4:
	case 0xd5:
	case 0xd6:
	case 0xd7:
	{
		// JR cc,a

		m_op = JR;

		m_param1_type = CONDITIONAL;
		m_param1 = b0 & 0x7;

		m_param2_type = ABSOLUTE_VAL_16;

		int val = READ8();
		if (val & 0x80) b0 -= 0x100;

		m_param2 = m_addr - 2 + 2 + val;

		break;
	}
	case 0xd8:
	case 0xd9:
	case 0xda:
	case 0xdb:
	case 0xdc:
	case 0xdd:
	case 0xde:
	case 0xdf:
		// LD CF, (x).b  aka TEST (x).b
		m_op = LD;

		m_param1_type = CARRYFLAG;
		//m_param1 = 0;

		m_param2_type = ADDR_IN_IMM_X | BITPOS;
		m_param2 = READ8();
		m_bitpos = b0 & 0x7;
		break;

	case 0xe0:
		// src prefix
		decode_source(ADDR_IN_BASE+(b0&0x7), READ8());
		break;

	case 0xe1:
	case 0xe2:
	case 0xe3:
		decode_source(ADDR_IN_BASE+(b0&0x7), 0);
		break;

	case 0xe4:
		decode_source(ADDR_IN_BASE+(b0&0x7), READ8());
		break;

	case 0xe5:
	case 0xe6:
	case 0xe7:
		decode_source(ADDR_IN_BASE+(b0&0x7), 0);
		break;

	case 0xe8:
	case 0xe9:
	case 0xea:
	case 0xeb:
	case 0xec:
	case 0xed:
	case 0xee:
	case 0xef:
		// register prefix: g/gg
		decode_register_prefix(b0);
		break;

	case 0xf0: // 1111 0000 xxxx xxxx 0101 0rrr
		// destination memory prefix (dst)
		m_param1_type = ADDR_IN_BASE+(b0&0x7); 
		m_param1 = READ8();
		decode_dest(b0);
		break;

	case 0xf2: // 1111 001p 0101 0rrr
	case 0xf3: // 1111 001p 0101 0rrr
		// destination memory prefix (dst)
		m_param1_type = ADDR_IN_BASE+(b0&0x7); 
		decode_dest(b0);
		break;


	case 0xf4: // 1111 0100 dddd dddd 0101 0rrr
		// destination memory prefix (dst)
		m_param1_type = ADDR_IN_BASE+(b0&0x7); 
		m_param1 = READ8();
		decode_dest(b0);
		break;

	case 0xf6: // 1110 0110 0101 0rrr
	case 0xf7: // 1111 0111 0101 0rrr
		// destination memory prefix (dst)
		m_param1_type = ADDR_IN_BASE+(b0&0x7); 
		decode_dest(b0);
		break;

	case 0xf1:
	case 0xf5:
		// invalid dst memory prefix
		break;


	case 0xf8:
	case 0xf9:
		// unused
		break;

	case 0xfa:
		// LD SP,mn
		m_op = LD;

		m_param1_type = STACKPOINTER;
		//m_param1 = 0;
		
		m_param2_type = ABSOLUTE_VAL_16;
		m_param2 = READ16();

		break;

	case 0xfb:
	{
		// JR a
		m_op = JR;

		m_param2_type = ABSOLUTE_VAL_16;

		int val = READ8();
		if (val & 0x80) b0 -= 0x100;

		m_param2 = m_addr - 2 + 2 + val;

		break;
	}
	
	break;

	case 0xfc:
		// CALL mn
		m_op = CALL;

		m_param1_type = ABSOLUTE_VAL_16;
		m_param1 = READ16();
		break;

	case 0xfd:
		// CALLP n
		m_op = CALLP;

		m_param1_type = ABSOLUTE_VAL_16;
		m_param1 = READ8()+0xff00;

		break;

	case 0xfe:
		// JP mn
		m_op = JP;

		m_param1_type = ABSOLUTE_VAL_16;
		m_param1 = READ16();

		break;

	case 0xff:
		// SWI
		m_op = SWI;

		break;
	}
}

// e8 - ef use this table
void tlcs870_device::decode_register_prefix(uint8_t b0)
{
	uint8_t bx;
	
	bx = READ8();

	switch (bx)
	{
	case 0x00:
		// nothing
		break;

	case 0x01:
		// SWAP g
		m_op = SWAP;
		
		m_param1_type = REG_8BIT;
		m_param1 = b0 & 0x7;
		break;

	case 0x02:
		// MUL ggG, ggL
		m_op = MUL;
		
		m_param1_type = REG_16BIT; // odd syntax
		m_param1 = b0 & 0x3;
		break;

	case 0x03:
		// DIV gg,C
		m_op = DIV;
		m_param1_type = REG_16BIT;
		m_param1 = b0 & 3;
	
		m_param2_type = REG_8BIT;
		m_param2 = 2; // C

		break;

	case 0x04:
		// with E8 only
		// RETN
		if (b0 == 0xe8)
		{
			m_op = RETN;
		}

		break;

	case 0x05:
		break;

	case 0x06:
		// POP gg
		m_op = POP;
		m_param1_type = REG_16BIT;
		m_param1 = b0 & 3;
		// b0 & 4 = invalid?

		break;

	case 0x07:
		// PUSH gg
		m_op = PUSH;
		m_param1_type = REG_16BIT;
		m_param1 = b0 & 3;
		// b0 & 4 = invalid?

		break;

	case 0x08:
	case 0x09:
		break;

	case 0x0a:
		// DAA g
		m_op = DAA;
		
		m_param1_type = REG_8BIT;
		m_param1 = b0 & 0x7;

		break;

	case 0x0b:
		// DAS g
		m_op = DAS;
		
		m_param1_type = REG_8BIT;
		m_param1 = b0 & 0x7;

		break;

	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:
		break;

	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
		// XCH rr,gg
		m_op = XCH;
		
		m_param1_type = REG_16BIT;
		m_param1 = bx & 0x3;

		m_param2_type = REG_16BIT;
		m_param2 = b0 & 0x3;
		break;

	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
		// LD rr,gg
		m_op = LD;
		
		m_param1_type = REG_16BIT;
		m_param1 = bx & 0x3;

		m_param2_type = REG_16BIT;
		m_param2 = b0 & 0x3;

		break;

	case 0x18:
	case 0x19:
	case 0x1a:
	case 0x1b:
		break;

	case 0x1c:
		// SHLC g
		m_op = SHLC;
		
		m_param1_type = REG_8BIT;
		m_param1 = b0 & 0x7;

		break;

	case 0x1d:
		// SHRC g
		m_op = SHRC;
		
		m_param1_type = REG_8BIT;
		m_param1 = b0 & 0x7;

		break;

	case 0x1e:
		// ROLC g
		m_op = ROLC;
		
		m_param1_type = REG_8BIT;
		m_param1 = b0 & 0x7;

		break;

	case 0x1f:
		// RORC g
		m_op = RORC;
		
		m_param1_type = REG_8BIT;
		m_param1 = b0 & 0x7;

		break;

	case 0x20:
	case 0x21:
	case 0x22:
	case 0x23:
	case 0x24:
	case 0x25:
	case 0x26:
	case 0x27:
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2d:
	case 0x2e:
	case 0x2f:
		break;

	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
		// (ALU OP) WA,gg
		m_op = (bx & 0x7)+ALU_ADDC;
		
		m_param1_type = REG_16BIT;
		m_param1 = 0;

		m_param2_type = REG_16BIT;
		m_param2 = b0 & 3;
		// b0 & 4 would be invalid?


		break;

	case 0x38:
	case 0x39:
	case 0x3a:
	case 0x3b:
	case 0x3c:
	case 0x3d:
	case 0x3e:
	case 0x3f:
		// (ALU OP) gg,mn
		m_op = (bx & 0x7)+ALU_ADDC;
		
		m_param1_type = REG_16BIT;
		m_param1 = b0 & 0x3;

		m_param2_type = ABSOLUTE_VAL_16; // absolute value
		m_param2 = READ16(); // 16-bit

		break;

	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	case 0x46:
	case 0x47:
		// SET g.b
		m_op = SET;
		
		m_param1_type = REG_8BIT | BITPOS;
		m_param1 = b0 & 0x7;
		m_bitpos = bx & 0x7;


		break;

	case 0x48:
	case 0x49:
	case 0x4a:
	case 0x4b:
	case 0x4c:
	case 0x4d:
	case 0x4e:
	case 0x4f:
		// CLR g.b
		m_op = CLR;
		
		m_param1_type = REG_8BIT | BITPOS;
		m_param1 = b0 & 0x7;
		m_bitpos = bx & 0x7;

		break;

	case 0x50:
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
		break;

	case 0x58:
	case 0x59:
	case 0x5a:
	case 0x5b:
	case 0x5c:
	case 0x5d:
	case 0x5e:
	case 0x5f:
		// LD r,g
		m_op = LD;
		
		m_param1_type = REG_8BIT;
		m_param1 = bx & 0x7;

		m_param2_type = REG_8BIT;
		m_param2 = b0 & 0x7;
		break;

	case 0x60:
	case 0x61:
	case 0x62:
	case 0x63:
	case 0x64:
	case 0x65:
	case 0x66:
	case 0x67:
		// (ALU OP) A,g
		m_op = (bx & 0x7)+ALU_ADDC;
		
		m_param2_type = REG_8BIT;
		m_param2 = b0 & 0x7;

		m_param1_type = REG_8BIT;
		m_param1 = 0; // A
		break;

	case 0x68:
	case 0x69:
	case 0x6a:
	case 0x6b:
	case 0x6c:
	case 0x6d:
	case 0x6e:
	case 0x6f:
		// (ALU OP) g,A
		m_op = (bx & 0x7)+ALU_ADDC;
		
		m_param1_type = REG_8BIT;
		m_param1 = b0 & 0x7;

		m_param2_type = REG_8BIT;
		m_param2 = 0; // A
		break;

	case 0x70:
	case 0x71:
	case 0x72:
	case 0x73:
	case 0x74:
	case 0x75:
	case 0x76:
	case 0x77:
		// (ALU OP) g,n
		m_op = (bx & 0x7)+ALU_ADDC;
		
		m_param1_type = REG_8BIT;
		m_param1 = b0 & 0x7;

		m_param2_type = ABSOLUTE_VAL_8;
		m_param2 = READ8();


		break;

	case 0x78:
	case 0x79:
	case 0x7a:
	case 0x7b:
	case 0x7c:
	case 0x7d:
	case 0x7e:
	case 0x7f:
		break;


	case 0x80:
	case 0x81:
		break;

	case 0x82:
		// SET (x).g
		m_op = SET;
		m_param1_type = ADDR_IN_IMM_X | BITPOS;
		m_param1 = READ8();
		m_bitpos = b0 & 7;
		break;

	case 0x83:
		// SET (HL).g
		m_op = SET;
		m_param1_type = ADDR_IN_HL | BITPOS;
		//m_param1 = 0;
		m_bitpos = b0 & 7;
		break;

	case 0x84:
	case 0x85:
	case 0x86:
	case 0x87:
		break;

	case 0x88:
	case 0x89:
		break;

	case 0x8a:
		// CLR (x).g
		m_op = CLR;
		m_param1_type = ADDR_IN_IMM_X | BITPOS;
		m_param1 = READ8();
		m_bitpos = b0 & 7;
		break;

	case 0x8b:
		// CLR (HL).g
		m_op = CLR;
		m_param1_type = ADDR_IN_HL | BITPOS;
		//m_param1 = 0;
		m_bitpos = b0 & 7;
		break;

	case 0x8c:
	case 0x8d:
	case 0x8e:
	case 0x8f:
		break;


	case 0x90:
	case 0x91:
		break;

	case 0x92:
		// CPL (x).g
		m_op = CPL;
		m_param1_type = ADDR_IN_IMM_X | BITPOS;
		m_param1 = READ8();
		m_bitpos = b0 & 7;

	case 0x93:
		// CPL (HL).g
		m_op = CPL;
		m_param1_type = ADDR_IN_HL | BITPOS;
		//m_param1 = 0;
		m_bitpos = b0 & 7;
		break;

	case 0x94:
	case 0x95:
	case 0x96:
	case 0x97:
		break;

	case 0x9a:
		// LD (x).g,CF
		m_op = LD;
		m_param1_type = ADDR_IN_IMM_X | BITPOS;
		m_param1 = READ8();
		m_bitpos = b0 & 7;

		m_param2_type = CARRYFLAG;
		//m_param2 = 0;
		break;

	case 0x9b:
		// LD (HL).g,CF
		m_op = LD;
		m_param1_type = ADDR_IN_HL | BITPOS;
		//m_para1 = 0;
		m_bitpos = b0 & 7;

		m_param2_type = CARRYFLAG;
		//m_param2 = 0;
		break;

	case 0x9c:
	case 0x9d:
		break;

	case 0x9e:
		// LD CF,(x).g  aka TEST (x).g
		m_op = LD;
		m_param1_type = CARRYFLAG;
		//m_param1 = 0;

		m_param2_type = ADDR_IN_IMM_X | BITPOS;
		m_param2 = READ8();
		m_bitpos = b0 & 7;

		break;

	case 0x9f:
		// LD CF,(HL).g   aka TEST (HL).g
		m_op = LD;
		m_param1_type = CARRYFLAG;
		//m_param1 = 0;

		m_param2_type = ADDR_IN_HL | BITPOS;
		//m_param2 = 0;
		m_bitpos = b0 & 7;
		break;

	case 0xa0:
	case 0xa1:
	case 0xa2:
	case 0xa3:
	case 0xa4:
	case 0xa5:
	case 0xa6:
	case 0xa7:
		break;

	case 0xb0:
	case 0xb1:
	case 0xb2:
	case 0xb3:
	case 0xb4:
	case 0xb5:
	case 0xb6:
	case 0xb7:
	case 0xb8:
	case 0xb9:
	case 0xba:
	case 0xbb:
	case 0xbc:
	case 0xbd:
	case 0xbe:
	case 0xbf:
		break;

	case 0xc0:
	case 0xc1:
	case 0xc2:
	case 0xc3:
	case 0xc4:
	case 0xc5:
	case 0xc6:
	case 0xc7:
		// CPL g.b
		m_op = CPL;

		m_param1_type = REG_8BIT | BITPOS;
		m_param1 = b0 & 0x7;
		m_bitpos = bx & 0x7;
		break;

	case 0xc8:
	case 0xc9:
	case 0xca:
	case 0xcb:
	case 0xcc:
	case 0xcd:
	case 0xce:
	case 0xcf:
		// LD g.b,CF
		m_op = LD;

		m_param2_type = CARRYFLAG;
		//m_param2 = 0;

		m_param1_type = REG_8BIT | BITPOS;
		m_param1 = b0 & 0x7;
		m_bitpos = bx & 0x7;

		break;

	case 0xd0:
	case 0xd1:
	case 0xd2:
	case 0xd3:
	case 0xd4:
	case 0xd5:
	case 0xd6:
	case 0xd7:
		// XOR CF,g.b
		m_op = ALU_XOR;

		m_param1_type = CARRYFLAG;
		//m_param1 = 0;

		m_param2_type = REG_8BIT | BITPOS;
		m_param2 = b0 & 0x7;
		m_bitpos = bx & 0x7;

		break;

	case 0xd8:
	case 0xd9:
	case 0xda:
	case 0xdb:
	case 0xdc:
	case 0xdd:
	case 0xde:
	case 0xdf:
		// LD CF,g.b   aka TEST g.b
		m_op = LD;

		m_param1_type = CARRYFLAG;
		//m_param1 = 0;

		m_param2_type = REG_8BIT | BITPOS;
		m_param2 = b0 & 0x7;
		m_bitpos = bx & 0x7;

		break;

	case 0xe0:
	case 0xe1:
	case 0xe2:
	case 0xe3:
	case 0xe4:
	case 0xe5:
	case 0xe6:
	case 0xe7:
	case 0xe8:
	case 0xe9:
	case 0xea:
	case 0xeb:
	case 0xec:
	case 0xed:
	case 0xee:
	case 0xef:
		break;

	case 0xf0:
	case 0xf1:
	case 0xf2:
	case 0xf3:
	case 0xf4:
	case 0xf5:
	case 0xf6:
	case 0xf7:
		break;

	case 0xf8:
	case 0xf9:
		break;

	case 0xfa:
		// LD SP,gg
		m_op = LD;

		m_param2_type = REG_16BIT;
		m_param2 = b0 & 3;
		// b0 & 4 would be invalid?

		m_param1_type = STACKPOINTER;
	//	m_param1 = 0;

		break;

	case 0xfb:
		// LD gg,SP
		m_op = LD;
		
		m_param1_type = REG_16BIT;
		m_param1 = b0 & 3;
		// b0 & 4 would be invalid?

		m_param2_type = STACKPOINTER;
	//	m_param2 = 0;
		break;

	case 0xfc:
		// CALL gg
		m_op = CALL;

		m_param2_type = REG_16BIT;
		m_param2 = b0 & 3;
		// b0 & 4 would be invalid?

		break;

	case 0xfd:
		break;

	case 0xfe:
		// JP gg
		m_op = JP;

		m_param2_type = REG_16BIT;
		m_param2 = b0 & 3;
		// b0 & 4 would be invalid?

		break;

	case 0xff:
		break;

	}

}



// e0 - e7 use this table
void tlcs870_device::decode_source(int type, uint16_t val)
{
	uint8_t bx;
	
	bx = READ8();

	switch (bx)
	{
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
		break;

	case 0x08:
		// ROLD A,(src)
		m_op = ROLD;
		m_param2_type = type;
		m_param2 = val;

		m_param1_type = REG_8BIT;
		m_param1 = 0; // A
		break;

	case 0x09:
		// RORD A,(src)
		m_op = RORD;
		m_param2_type = type;
		m_param2 = val;

		m_param1_type = REG_8BIT;
		m_param1 = 0; // A
		break;

	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:

	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
		// see dst
		break;

	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
		// LD rr, (src)
		m_op = LD;
		m_param1_type = REG_16BIT;
		m_param1 = bx & 0x3;

		m_param2_type = type;
		m_param2 = val;
		break;

	case 0x18:
	case 0x19:
	case 0x1a:
	case 0x1b:
	case 0x1c:
	case 0x1d:
	case 0x1e:
	case 0x1f:
		break;

	case 0x20:
		// INC (src)
		m_op = INC;
		m_param1_type = type;
		m_param1 = val;

		break;

	case 0x21:
	case 0x22:
	case 0x23:
	case 0x24:
	case 0x25:
		break;

	case 0x26:
		// LD (x),(src)
		m_op = LD;
		m_param1_type = ADDR_IN_IMM_X;
		m_param1 = READ8();

		m_param2_type = type;
		m_param2 = val;
		break;

	case 0x27:
		// LD (HL),(src)
		m_op = LD;
		m_param1_type = ADDR_IN_HL;
		//m_param1 = 0;
		
		m_param2_type = type;
		m_param2 = val;
		break;


	case 0x28:
		// DEC (src)
		m_op = DEC;
		m_param1_type = type;
		m_param1 = val;

		break;

	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2d:
	case 0x2e:
		break;

	case 0x2f:
		// MCMP (src), n
		m_op = MCMP;
		m_param1_type = type;
		m_param1 = val;

		m_param2_type = ABSOLUTE_VAL_8;
		m_param2 = READ8();
		break;

	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
	case 0x38:
	case 0x39:
	case 0x3a:
	case 0x3b:
	case 0x3c:
	case 0x3d:
	case 0x3e:
	case 0x3f:
		break;

	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	case 0x46:
	case 0x47:
		// SET (src).b
		m_op = SET;
		
		m_param1_type = type | BITPOS;
		m_param1 = val;
		m_bitpos = bx & 0x7;
		break;

	case 0x48:
	case 0x49:
	case 0x4a:
	case 0x4b:
	case 0x4c:
	case 0x4d:
	case 0x4e:
	case 0x4f:
		// CLR (src).b
		m_op = CLR;
		
		m_param1_type = type | BITPOS;
		m_param1 = val;
		m_bitpos = bx & 0x7;
		break;

	case 0x50:
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
		// see dst
		break;

	case 0x58:
	case 0x59:
	case 0x5a:
	case 0x5b:
	case 0x5c:
	case 0x5d:
	case 0x5e:
	case 0x5f:
		// LD r, (src)
		m_op = LD;

		m_param1_type = REG_8BIT;
		m_param1 = bx & 0x7;

		m_param2_type = type;
		m_param2 = val;
		break;

	case 0x60:
	case 0x61:
	case 0x62:
	case 0x63:
	case 0x64:
	case 0x65:
	case 0x66:
	case 0x67:
		// (ALU OP) (src), (HL)
		m_op = (bx & 0x7)+ALU_ADDC;
		m_param1_type = type;
		m_param1 = val;

		m_param2_type = ADDR_IN_HL;
		//m_param2 = 0;
		break;

	case 0x68:
	case 0x69:
	case 0x6a:
	case 0x6b:
	case 0x6c:
	case 0x6d:
	case 0x6e:
	case 0x6f:
		break;

	case 0x70:
	case 0x71:
	case 0x72:
	case 0x73:
	case 0x74:
	case 0x75:
	case 0x76:
	case 0x77:
		// (ALU OP) (src), n
		m_op = (bx & 0x7)+ALU_ADDC;
		m_param1_type = type;
		m_param1 = val;

		m_param2_type = ABSOLUTE_VAL_8;
		m_param2 = READ8();
		break;

	case 0x78:
	case 0x79:
	case 0x7a:
	case 0x7b:
	case 0x7c:
	case 0x7d:
	case 0x7e:
	case 0x7f:
		// (ALU OP) A, (src)

		m_op = (bx & 0x7)+ALU_ADDC;
		m_param2_type = type;
		m_param2 = val;

		m_param1_type = REG_8BIT;
		m_param1 = 0; // A
		break;

	case 0x80:
	case 0x81:
	case 0x82:
	case 0x83:
	case 0x84:
	case 0x85:
	case 0x86:
	case 0x87:
	case 0x88:
	case 0x89:
	case 0x8a:
	case 0x8b:
	case 0x8c:
	case 0x8d:
	case 0x8e:
	case 0x8f:
		break;

	case 0x90:
	case 0x91:
	case 0x92:
	case 0x93:
	case 0x94:
	case 0x95:
	case 0x96:
	case 0x97:
	case 0x98:
	case 0x99:
	case 0x9a:
	case 0x9b:
	case 0x9c:
	case 0x9d:
	case 0x9e:
	case 0x9f:
		break;

	case 0xa0:
	case 0xa1:
	case 0xa2:
	case 0xa3:
	case 0xa4:
	case 0xa5:
	case 0xa6:
	case 0xa7:
		break;

	case 0xa8:
	case 0xa9:
	case 0xaa:
	case 0xab:
	case 0xac:
	case 0xad:
	case 0xae:
	case 0xaf:
		// XCH r,(src)
		m_op = XCH;

		m_param1_type = REG_8BIT;
		m_param1 = bx & 0x7;

		m_param2_type = type;
		m_param2 = val;
		break;


	case 0xb0:
	case 0xb1:
	case 0xb2:
	case 0xb3:
	case 0xb4:
	case 0xb5:
	case 0xb6:
	case 0xb7:
	case 0xb8:
	case 0xb9:
	case 0xba:
	case 0xbb:
	case 0xbc:
	case 0xbd:
	case 0xbe:
	case 0xbf:
		break;

	case 0xc0:
	case 0xc1:
	case 0xc2:
	case 0xc3:
	case 0xc4:
	case 0xc5:
	case 0xc6:
	case 0xc7:
		// CPL (src).b
		m_op = CPL;
		
		m_param1_type = type | BITPOS;
		m_param1 = val;
		m_bitpos = bx & 0x7;
		break;

	case 0xc8:
	case 0xc9:
	case 0xca:
	case 0xcb:
	case 0xcc:
	case 0xcd:
	case 0xce:
	case 0xcf:
		// LD (src).b,CF
		m_op = LD;
		m_param1_type = type | BITPOS;
		m_param1 = val;
		m_bitpos = bx & 0x7;

		m_param2_type = CARRYFLAG;
		//m_param2 = 0;
		break;


	case 0xd0:
	case 0xd1:
	case 0xd2:
	case 0xd3:
	case 0xd4:
	case 0xd5:
	case 0xd6:
	case 0xd7:
		// XOR CF,(src).b
		m_op = ALU_XOR;
		m_param1_type = CARRYFLAG;
		//m_param1 = 0;

		m_param2_type = type | BITPOS;
		m_param2 = val;
		m_bitpos = bx & 0x7;
		break;

	case 0xd8:
	case 0xd9:
	case 0xda:
	case 0xdb:
	case 0xdc:
	case 0xdd:
	case 0xde:
	case 0xdf:
		// LD CF,(src).b  aka  TEST (src).b
		m_op = LD;
		m_param2_type = type | BITPOS;
		m_param2 = val;
		m_bitpos = bx & 0x7;

		m_param1_type = CARRYFLAG;
		//m_param1 = 0;
		break;


	case 0xe0:
	case 0xe1:
	case 0xe2:
	case 0xe3:
	case 0xe4:
	case 0xe5:
	case 0xe6:
	case 0xe7:
	case 0xe8:
	case 0xe9:
	case 0xea:
	case 0xeb:
	case 0xec:
	case 0xed:
	case 0xee:
	case 0xef:
		break;

	case 0xf0:
	case 0xf1:
	case 0xf2:
	case 0xf3:
	case 0xf4:
	case 0xf5:
	case 0xf6:
	case 0xf7:
		break;

	case 0xf8:
	case 0xf9:
	case 0xfa:
	case 0xfb:
	case 0xfc:
		// CALL (src)
		m_op = CALL;
		m_param1_type = type;
		m_param1 = val;
		break;

	case 0xfd:
		break;

	case 0xfe:
		// JP (src)
		m_op = JP;
		m_param1_type = type;
		m_param1 = val;
		break;

	case 0xff:
		break;


	}

}

// f0 - f7 use this table
// note, same table is shown as above in manual, there's no overlap between src/dest, but they're not compatible
void tlcs870_device::decode_dest(uint8_t b0)
{
	uint8_t bx;
	
	bx = READ8();

	switch (bx)
	{
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
		// LD (dst),rr
		m_op = LD;
		m_param2_type = REG_16BIT;
		m_param2 = bx&0x3;
		break;

	case 0x2c:
		// LD (dst),n
		m_op = LD;
		m_param2_type = ABSOLUTE_VAL_8;
		m_param2 = READ8();
		break;

	case 0x50:
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
		// LD (dst),r
		m_op = LD;
		m_param2_type = REG_8BIT;
		m_param2 = bx&0x7;
		break;

	default:
		break;
	}
}

bool tlcs870_device::stream_arg(std::ostream &stream, uint32_t pc, const char *pre, const uint16_t mode, const uint16_t r, const uint16_t rb)
{
	return false;
}

void tlcs870_device::disasm_disassemble_param(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options, int type, uint16_t val)
{
	int basetype = type & MODE_MASK;

	if ((basetype) == ADDR_8BIT)
	{
		if (type&0x80) util::stream_format(stream, " ($%04x)", val);
		else util::stream_format(stream, " ($%02x)", val);
	}

	if (basetype==ADDR_IN_16BITREG) util::stream_format(stream, " %s", type_x[val&7]);

	if (basetype==ADDR_IN_IMM_X) util::stream_format(stream, " ($%02x)", val); // direct
	if (basetype==ADDR_IN_PC_PLUS_REG_A) util::stream_format(stream, " %s", type_x[1]);
	if (basetype==ADDR_IN_DE) util::stream_format(stream, " %s", type_x[2]);
	if (basetype==ADDR_IN_HL) util::stream_format(stream, " %s", type_x[3]);
	if (basetype==ADDR_IN_HL_PLUS_IMM_D) util::stream_format(stream, " (HL+%04x)", val); // todo, sign extend
	if (basetype==ADDR_IN_HL_PLUS_REG_C) util::stream_format(stream, " %s", type_x[5]);
	if (basetype==ADDR_IN_HLINC) util::stream_format(stream, " %s", type_x[6]);
	if (basetype==ADDR_IN_DECHL) util::stream_format(stream, " %s", type_x[7]);

	if (basetype==REG_16BIT) util::stream_format(stream, " %s", reg16[val&3]);
	if (basetype==REG_8BIT) util::stream_format(stream, " %s", reg8[val&3]);
	if (basetype==CONDITIONAL) util::stream_format(stream, " %s", conditions[val]);
	if (basetype==STACKPOINTER) util::stream_format(stream, " SP");
	if (basetype==REGISTERBANK) util::stream_format(stream, " RBS");
	if (basetype==PROGRAMSTATUSWORD) util::stream_format(stream, " PSW");
	if (basetype==CARRYFLAG) util::stream_format(stream, " CF");
	if (basetype==MEMVECTOR_16BIT)  util::stream_format(stream, " (%04x)", val);
	if (basetype==ABSOLUTE_VAL_8)
	{
		if (type&0x80) util::stream_format(stream, "$%04x", val);
		else util::stream_format(stream, "$%02x", val);
	}

	if (type&BITPOS) util::stream_format(stream, ".BIT%d", m_bitpos);


}

offs_t tlcs870_device::disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options)
{
	m_addr = pc;

	decode();
//	m_op &= ~OP_16;

	util::stream_format         (stream, "%-5s",                op_names[ m_op ] );

	if (m_param1_type)
	{
		disasm_disassemble_param(stream, pc, oprom, opram, options, m_param1_type, m_param1);
	}

	if (m_param2_type)
	{
		if (m_param1_type) util::stream_format(stream, ",");

		disasm_disassemble_param(stream, pc, oprom, opram, options, m_param2_type, m_param2);

	}

//	bool streamed = stream_arg  (stream, pc,       " ",         m_mode1, m_r1, m_r1b );
//	stream_arg                  (stream, pc, streamed ?",":"",  m_mode2, m_r2, m_r2b );

	return (m_addr - pc) | DASMFLAG_SUPPORTED;
}

void tlcs870_device::execute_set_input(int inputnum, int state)
{
#if 0
	switch(inputnum) {
		case INPUT_LINE_NMI:
			set_irq_line(INTNMI, state);
			break;
		case INPUT_LINE_IRQ0:
			set_irq_line(INT0, state);
			break;
		case INPUT_LINE_IRQ1:
			set_irq_line(INT1, state);
			break;
		case INPUT_LINE_IRQ2:
			set_irq_line(INT2, state);
			break;
	}
#endif
}

void tlcs870_device::execute_run()
{
	do
	{
		m_prvpc.d = m_pc.d;
		debugger_instruction_hook(this, m_pc.d);

		//check_interrupts();

		m_addr = m_pc.d;
		decode();
		m_pc.d = m_addr;

		switch ( m_op )
		{
			default:
				logerror("%04x: unimplemented opcode, op=%02x\n",pc(),m_op);
		}

		m_icount--;


	} while( m_icount > 0 );
}

void tlcs870_device::device_reset()
{
	// todo, read from top address
	m_pc.d = 0xc030;
}

void tlcs870_device::execute_burn(int32_t cycles)
{
	m_icount -= cycles;
}


void tlcs870_device::device_start()
{
//	int i, p;


	m_program = &space(AS_PROGRAM);
	m_io = &space(AS_IO);

	//state_add( T90_PC, "PC", m_pc.w.l).formatstr("%04X");
	//state_add( T90_SP, "SP", m_sp.w.l).formatstr("%04X");

	state_add(STATE_GENPC, "GENPC", m_pc.w.l).formatstr("%04X").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_prvpc.w.l).formatstr("%04X").noshow();
	state_add(STATE_GENSP, "GENSP", m_sp.w.l).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_F ).formatstr("%8s").noshow();

	m_icountptr = &m_icount;
}


void tlcs870_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	/*
	switch (entry.index())
	{
		
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				F & 0x80 ? 'S':'.',
				F & 0x40 ? 'Z':'.',
				F & 0x20 ? 'I':'.',
				F & 0x10 ? 'H':'.',
				F & 0x08 ? 'X':'.',
				F & 0x04 ? 'P':'.',
				F & 0x02 ? 'N':'.',
				F & 0x01 ? 'C':'.'
			);
			break;
	}
	/*/
}
