// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************************************************

    Toshiba TLCS-870 Series MCUs

    The TLCS-870/X expands on this instruction set using the same base encoding.

    The TLCS-870/C appears to have a completely different encoding.

    loosely baesd on the tlcs90 core by Luca Elia

*************************************************************************************************************/



#include "emu.h"
#include "tlcs870.h"
#include "tlcs870d.h"
#include "debugger.h"

#define IS16BIT 0x80
#define BITPOS 0x40
#define BITPOS_INDIRECT 0x20

#define ABSOLUTE_VAL_8 0x01
#define REG_8BIT 0x02

// special
#define CONDITIONAL 0x03
#define STACKPOINTER (0x04 | IS16BIT) // this is a 16-bit reg
#define CARRYFLAG (0x5 | BITPOS) // also flag as BITPOS since it's a bit operation?
#define MEMVECTOR_16BIT 0x6
#define REGISTERBANK 0x7
#define PROGRAMSTATUSWORD 0x8

#define ABSOLUTE_VAL_16 (ABSOLUTE_VAL_8|IS16BIT)
#define REG_16BIT (REG_8BIT|IS16BIT)

#define ADDR_IN_BASE 0x10
#define ADDR_IN_IMM_X (ADDR_IN_BASE+0x0)
#define ADDR_IN_PC_PLUS_REG_A (ADDR_IN_BASE+0x1)
#define ADDR_IN_DE (ADDR_IN_BASE+0x2)
#define ADDR_IN_HL (ADDR_IN_BASE+0x3)
#define ADDR_IN_HL_PLUS_IMM_D (ADDR_IN_BASE+0x4)
#define ADDR_IN_HL_PLUS_REG_C (ADDR_IN_BASE+0x5)
#define ADDR_IN_HLINC (ADDR_IN_BASE+0x6)
#define ADDR_IN_DECHL (ADDR_IN_BASE+0x7)

#define MODE_MASK 0x1f


#define FLAG_J (0x80)
#define FLAG_Z (0x40)
#define FLAG_C (0x20)
#define FLAG_H (0x10)


#define IS_JF ((m_F & FLAG_J) ? 1 : 0)
#define IS_ZF ((m_F & FLAG_Z) ? 1 : 0)
#define IS_CF ((m_F & FLAG_C) ? 1 : 0)
#define IS_HF ((m_F & FLAG_H) ? 1 : 0)

#define SET_JF (m_F |= FLAG_J)
#define SET_ZF (m_F |= FLAG_Z)
#define SET_CF (m_F |= FLAG_C)
#define SET_HF (m_F |= FLAG_H)

#define CLEAR_JF (m_F &= ~FLAG_J)
#define CLEAR_ZF (m_F &= ~FLAG_Z)
#define CLEAR_CF (m_F &= ~FLAG_C)
#define CLEAR_HF (m_F &= ~FLAG_H)



DEFINE_DEVICE_TYPE(TMP87PH40AN, tmp87ph40an_device, "tmp87ph40an", "Toshiba TMP87PH40AN")

void tlcs870_device::tmp87ph40an_mem(address_map &map)
{
#if 0
	map(0x0000, 0x0000).rw(this, FUNC(tlcs870_device::port0_r), FUNC(tlcs870_device::port0_w));
	map(0x0001, 0x0001).rw(this, FUNC(tlcs870_device::port1_r), FUNC(tlcs870_device::port1_w));
	map(0x0002, 0x0002).rw(this, FUNC(tlcs870_device::port2_r), FUNC(tlcs870_device::port2_w));
	map(0x0003, 0x0003).rw(this, FUNC(tlcs870_device::port3_r), FUNC(tlcs870_device::port3_w));
	map(0x0004, 0x0004).rw(this, FUNC(tlcs870_device::port4_r), FUNC(tlcs870_device::port4_w));
	map(0x0005, 0x0005).rw(this, FUNC(tlcs870_device::port5_r), FUNC(tlcs870_device::port5_w));
	map(0x0006, 0x0006).rw(this, FUNC(tlcs870_device::port6_r), FUNC(tlcs870_device::port6_w));
	map(0x0007, 0x0007).rw(this, FUNC(tlcs870_device::port7_r), FUNC(tlcs870_device::port7_w));
	// 0x8 reserved
	// 0x9 reserved
	map(0x000a, 0x000a).w(this, FUNC(tlcs870_device::p0cr_w)); // Port 0 I/O control
	map(0x000b, 0x000b).w(this, FUNC(tlcs870_device::p1cr_w)); // Port 1 I/O control
	map(0x000c, 0x000c).w(this, FUNC(tlcs870_device::p6cr_w)); // Port 6 I/O control
	map(0x000d, 0x000d).w(this, FUNC(tlcs870_device::p7cr_w)); // Port 7 I/O control
	map(0x000e, 0x000e).rw(this, FUNC(tlcs870_device::adccr_r), FUNC(tlcs870_device::adccr_w)); // A/D converter control
	map(0x000f, 0x000f).r(this, FUNC(tlcs870_device::adcdr_r)); // A/D converter result

	map(0x0010, 0x0010).w(this, FUNC(tlcs870_device::treg1a_l_w)); // Timer register 1A
	map(0x0011, 0x0011).w(this, FUNC(tlcs870_device::treg1a_h_w)); //
	map(0x0012, 0x0012).rw(this, FUNC(tlcs870_device::treg1b_l_r), FUNC(tlcs870_device::treg1b_l_w)); // Timer register 1B
	map(0x0013, 0x0013).rw(this, FUNC(tlcs870_device::treg1b_h_r), FUNC(tlcs870_device::treg1b_h_w)); //
	map(0x0014, 0x0014).w(this, FUNC(tlcs870_device::tc1cr_w)); // TC1 control
	map(0x0015, 0x0015).w(this, FUNC(tlcs870_device::tc2cr_w)); // TC2 control
	map(0x0016, 0x0016).w(this, FUNC(tlcs870_device::treg2_l_w)); // Timer register 2
	map(0x0017, 0x0017).w(this, FUNC(tlcs870_device::treg2_h_w)); //
	map(0x0018, 0x0018).rw(this, FUNC(tlcs870_device::treg3a_r), FUNC(tlcs870_device::treg3a_w)); // Timer register 3A
	map(0x0019, 0x0019).r(this, FUNC(tlcs870_device::treg3b_r)); // Timer register 3B
	map(0x001a, 0x001a).w(this, FUNC(tlcs870_device::tc3cr_w)); // TC3 control
	map(0x001b, 0x001b).r(this, FUNC(tlcs870_device::treg4_r)); // Timer register 4
	map(0x001c, 0x001c).w(this, FUNC(tlcs870_device::tc4cr_w)); // TC4 control
	// 0x1d reserved
	// 0x1e reserved
	// 0x1f reserved

	map(0x0020, 0x0020).rw(this, FUNC(tlcs870_device::sio1sr_r), FUNC(tlcs870_device::sio1cr1_w)); // SIO1 status / SIO1 control
	map(0x0021, 0x0021).w(this, FUNC(tlcs870_device::sio1cr2_w)); // SIO1 control
	map(0x0022, 0x0022).rw(this, FUNC(tlcs870_device::sio2sr_r), FUNC(tlcs870_device::sio2cr1_w)); // SIO2 status / SIO2 control
	map(0x0023, 0x0023).w(this, FUNC(tlcs870_device::sio2cr2_w)); // SIO2 control
	// 0x24 reserved
	// 0x25 reserved
	// 0x26 reserved
	// 0x27 reserved
	// 0x28 reserved
	// 0x29 reserved
	// 0x2a reserved
	// 0x2b reserved
	// 0x2c reserved
	// 0x2d reserved
	// 0x2e reserved
	// 0x2f reserved

	// 0x30 reserved
	// 0x31 reserved
	// 0x32 reserved
	// 0x33 reserved
	map(0x0034, 0x0034).w(this, FUNC(tlcs870_device::wdtcr1_w)); // WDT control
	map(0x0035, 0x0035).w(this, FUNC(tlcs870_device::wdtcr2_w)); //

	map(0x0036, 0x0036).rw(this, FUNC(tlcs870_device::tbtcr_r), FUNC(tlcs870_device::tbtcr_w)); // TBT / TG / DVO control
	map(0x0037, 0x0037).rw(this, FUNC(tlcs870_device::eintcr_r), FUNC(tlcs870_device::eintcr_w)); // External interrupt control

	map(0x0038, 0x0038).rw(this, FUNC(tlcs870_device::syscr1_r), FUNC(tlcs870_device::syscr1_w)); // System Control
	map(0x0039, 0x0039).rw(this, FUNC(tlcs870_device::syscr2_r), FUNC(tlcs870_device::syscr2_w)); //

	map(0x003a, 0x003a).rw(this, FUNC(tlcs870_device::eir_l_r), FUNC(tlcs870_device::eir_l_w)); // Interrupt enable register
	map(0x003b, 0x003b).rw(this, FUNC(tlcs870_device::eir_h_r), FUNC(tlcs870_device::eir_h_w)); //

	map(0x003c, 0x003c).rw(this, FUNC(tlcs870_device::il_l_r), FUNC(tlcs870_device::il_l_w)); // Interrupt latch
	map(0x003d, 0x003d).rw(this, FUNC(tlcs870_device::il_h_r), FUNC(tlcs870_device::il_h_w)); //
	// 0x3e reserved
	map(0x003f, 0x003f).rw(this, FUNC(tlcs870_device::psw_r), FUNC(tlcs870_device::rbs_w)); // Program status word / Register bank selector
#endif

	map(0x0040, 0x023f).ram().share("intram"); // register banks + internal RAM, not, code execution NOT allowed here
	map(0x0f80, 0x0fff).ram(); // DBR
	map(0xc000, 0xffff).rom();
}


tlcs870_device::tlcs870_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor program_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0, program_map)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_intram(*this, "intram")
{
}


tmp87ph40an_device::tmp87ph40an_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tlcs870_device(mconfig, TMP87PH40AN, tag, owner, clock, address_map_constructor(FUNC(tmp87ph40an_device::tmp87ph40an_mem), this))
{
}

device_memory_interface::space_config_vector tlcs870_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
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
	m_cycles = 1;
	m_flagsaffected = 0; // needed to signal which flags to change and which to leave alone in some cases (LD operations at least)

	uint8_t b0;
	uint8_t b1;

	int tmppc = m_addr;

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
		m_op = LD;      // Flags / Cycles  1--- / 2
		m_flagsaffected |= FLAG_J;

		m_param1_type = REGISTERBANK; // 4-bit register
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
		m_op = LD;   // Flags / Cycles  1--- / 3
		m_flagsaffected |= FLAG_J;

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
		m_op = LD;   // Flags / Cycles  1Z-- / 3
		m_flagsaffected |= FLAG_J | FLAG_Z;

		m_param1_type = REG_8BIT;
		m_param1 = 0; // A

		m_param2_type = ADDR_IN_IMM_X;
		m_param2 = READ8();
		break;

	case 0x23:
		// LD A,(HL)
		m_op = LD;  // Flags / Cycles  1Z-- / 2
		m_flagsaffected |= FLAG_J | FLAG_Z;

		m_param1_type = REG_8BIT;
		m_param1 = 0; // A

		m_param2_type = ADDR_IN_HL;
		//m_param2 = 0;
		break;

	case 0x24:
		// LDW (x),mn
		m_op = LDW;
		m_param1_type = ADDR_IN_IMM_X; // 8-bit memory address
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
		// LD (x),(y)  // Flags / Cycles  1Z-- / 5
		m_op = LD;
		m_flagsaffected |= FLAG_J | FLAG_Z;

		m_param2_type = ADDR_IN_IMM_X;
		m_param2 = READ8();

		m_param1_type = ADDR_IN_IMM_X;
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
		// LD (x),A  // Flags / Cycles  1Z-- / 3
		m_op = LD;
		m_flagsaffected |= FLAG_J | FLAG_Z;

		m_param1_type = ADDR_IN_IMM_X;
		m_param1 = READ8();

		m_param2_type = REG_8BIT;
		m_param2 = 0; // A

		break;

	case 0x2b:
		// LD (HL),A  // Flags / Cycles  1--- / 2
		m_op = LD;
		m_flagsaffected |= FLAG_J;

		m_param1_type = ADDR_IN_HL;
		//m_param1 = 0;

		m_param2_type = REG_8BIT;
		m_param2 = 0; // A
		break;

	case 0x2c:
		// LD (x),n
		m_op = LD;   // Flags / Cycles  1--- / 4
		m_flagsaffected |= FLAG_J;

		m_param1_type = ADDR_IN_IMM_X; // 8-bit memory address
		m_param1 = READ8();

		m_param2_type = ABSOLUTE_VAL_8; // absolute value
		m_param2 = READ8();

		break;

	case 0x2d:
		// LD (HL),n
		m_op = LD;  // Flags / Cycles  1--- / 3
		m_flagsaffected |= FLAG_J;

		m_param1_type = ADDR_IN_HL; // memory address in 16-bit register
		//m_param1 = 3; // (HL)

		m_param2_type = ABSOLUTE_VAL_8; // absolute value
		m_param2 = READ8();

		break;

	case 0x2e:
		// CLR (x)
		m_op = CLR;
		m_param1_type = ADDR_IN_IMM_X; // 8-bit memory address
		m_param1 = READ8();

		break;

	case 0x2f:
		// CLR (HL)
		m_op = CLR;
		m_param1_type = ADDR_IN_HL; // memory address in 16-bit register
		//m_param1 = 3; // (HL)

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

		m_op = LD;  // Flags / Cycles  1--- / 2
		m_flagsaffected |= FLAG_J;

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
		m_op = LD;   // Flags / Cycles  1Z-- / 1
		m_flagsaffected |= FLAG_J | FLAG_Z;

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
		m_op = LD;  // Flags / Cycles  1Z-- / 1
		m_flagsaffected |= FLAG_J | FLAG_Z;

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

		m_param2 = tmppc + 2 + val;

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

		m_param2 = tmppc + 2 + val;

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
		if (val & 0x80) val -= 0x100;

		m_param2 = tmppc+ 2 + val;

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
		m_op = LD;  // Flags / Cycles  %-*- / 4
		m_flagsaffected |= FLAG_J | FLAG_C;

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
		m_op = LD;  // Flags / Cycles  1--- / 3
		m_flagsaffected |= FLAG_J;

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
		if (val & 0x80) val -= 0x100;

		m_param2 = tmppc + 2 + val;

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

		m_param2_type = ABSOLUTE_VAL_16;
		m_param2 = READ16();

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
		m_op = LD;  // Flags / Cycles  1--- / 2
		m_flagsaffected |= FLAG_J;

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
		m_op = LD;   // Flags / Cycles  1Z-- / 2
		m_flagsaffected |= FLAG_J | FLAG_Z;

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
	case 0x83:
		// SET (pp).g
		m_op = SET;
		m_param1_type = (ADDR_IN_DE+(bx&1)) | BITPOS | BITPOS_INDIRECT;
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
	case 0x8b:
		// CLR (pp).g
		m_op = CLR;
		m_param1_type = (ADDR_IN_DE+(bx&1)) | BITPOS | BITPOS_INDIRECT;
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
	case 0x93:
		// CPL (pp).g
		m_op = CPL;
		m_param1_type = (ADDR_IN_DE+(bx&1)) | BITPOS | BITPOS_INDIRECT;
		//m_param1 = 0;
		m_bitpos = b0 & 7;
		break;

	case 0x94:
	case 0x95:
	case 0x96:
	case 0x97:
		break;

	case 0x9a:
	case 0x9b:
		// LD (pp).g,CF
		m_op = LD;  // Flags / Cycles  1--- / 5
		m_flagsaffected |= FLAG_J;

		m_param1_type = (ADDR_IN_DE+(bx&1)) | BITPOS | BITPOS_INDIRECT;
		//m_para1 = 0;
		m_bitpos = b0 & 7;

		m_param2_type = CARRYFLAG;
		//m_param2 = 0;
		break;

	case 0x9c:
	case 0x9d:
		break;

	case 0x9e:
	case 0x9f:
		// LD CF,(pp).g   aka TEST (pp).g
		m_op = LD;   // Flags / Cycles  %-*- / 4
		m_flagsaffected |= FLAG_J | FLAG_C;

		m_param1_type = CARRYFLAG;
		//m_param1 = 0;

		m_param2_type = (ADDR_IN_DE+(bx&1)) | BITPOS | BITPOS_INDIRECT;
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
		m_op = LD;    // Flags / Cycles  1--- / 2
		m_flagsaffected |= FLAG_J;

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
		m_op = LD;  // Flags / Cycles  %-*- / 2
		m_flagsaffected |= FLAG_J | FLAG_C;

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
		m_op = LD;  // Flags / Cycles  1--- / 3
		m_flagsaffected |= FLAG_J;

		m_param2_type = REG_16BIT;
		m_param2 = b0 & 3;
		// b0 & 4 would be invalid?

		m_param1_type = STACKPOINTER;
	//  m_param1 = 0;

		break;

	case 0xfb:
		// LD gg,SP
		m_op = LD;  // Flags / Cycles  1--- / 3
		m_flagsaffected |= FLAG_J;

		m_param1_type = REG_16BIT;
		m_param1 = b0 & 3;
		// b0 & 4 would be invalid?

		m_param2_type = STACKPOINTER;
	//  m_param2 = 0;
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
		m_op = LD;  // Flags / Cycles  1--- / x
		m_flagsaffected |= FLAG_J;

		m_param1_type = REG_16BIT;
		m_param1 = bx & 0x3;

		m_param2_type = type | IS16BIT;
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

	case 0x26:  // invalid if (src) is also (x) ? (not specified)
		// LD (x),(src)
		m_op = LD;  // Flags / Cycles  1U-- / x
		m_flagsaffected |= FLAG_J /*| FLAG_Z*/; // Z is undefined!

		m_param1_type = ADDR_IN_IMM_X;
		m_param1 = READ8();

		m_param2_type = type;
		m_param2 = val;
		break;

	case 0x27:
		// LD (HL),(src)
		m_op = LD;   // Flags / Cycles  1Z-- / x
		m_flagsaffected |= FLAG_J | FLAG_Z;

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
		m_op = LD;  // Flags / Cycles  1Z-- / x
		m_flagsaffected |= FLAG_J | FLAG_Z;

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
		m_op = LD;  // Flags / Cycles  1--- / x
		m_flagsaffected |= FLAG_J;

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
		m_op = LD;   // Flags / Cycles  %-*- / x
		m_flagsaffected |= FLAG_J | FLAG_C;

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
		m_param2_type = type | IS16BIT;
		m_param2 = val;
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
		// LD (dst),rr    // (dst) can only be  (x) (pp) or (HL+d) ?  not (HL+) or (-HL) ?
		m_op = LD;  // Flags / Cycles  1--- / x
		m_flagsaffected |= FLAG_J;

		m_param1_type |= IS16BIT;


		m_param2_type = REG_16BIT;
		m_param2 = bx&0x3;
		break;

	case 0x2c:
		// LD (dst),n   // (dst) can only be (DE), (HL+), (-HL), or (HL+d)  because (x) and (HL) are redundant encodings?
		m_op = LD;  // Flags / Cycles  1--- / x
		m_flagsaffected |= FLAG_J;

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
		m_op = LD;   // Flags / Cycles  1--- / x
		m_flagsaffected |= FLAG_J;

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

uint16_t tlcs870_device::get_addr(uint16_t param_type, uint16_t param_val)
{
	uint16_t addr = 0x0000;

	switch (param_type&MODE_MASK)
	{
	case ADDR_IN_IMM_X:
		addr = param_val;
		break;
	case ADDR_IN_PC_PLUS_REG_A:
		addr = m_temppc + 2 + get_reg8(REG_A);
		break;
	case ADDR_IN_DE:
		addr = get_reg16(REG_DE);
		break;
	case ADDR_IN_HL:
		addr = get_reg16(REG_HL);
		break;
	case ADDR_IN_HL_PLUS_IMM_D:
		addr = get_reg16(REG_HL) + param_val;
		break;
	case ADDR_IN_HL_PLUS_REG_C:
		addr = get_reg16(REG_HL) + get_reg16(REG_C);
		break;
	case ADDR_IN_HLINC:
	{
		uint16_t tmpHL = get_reg16(REG_HL);
		addr = tmpHL;
		tmpHL++;
		set_reg16(REG_HL, tmpHL);
		break;
	}
	case ADDR_IN_DECHL:
	{
		uint16_t tmpHL = get_reg16(REG_HL);
		tmpHL--;
		set_reg16(REG_HL, tmpHL);
		addr = tmpHL;
		break;
	}
	}

	return addr;
}

void tlcs870_device::set_dest_val(uint16_t param_type, uint16_t param_val, uint16_t dest_val)
{
	if (param_type & IS16BIT)
	{
		switch (param_type)
		{
		case ABSOLUTE_VAL_16:
			logerror("illegal dest ABSOLUTE_VAL_16\n");
			break;

		case REG_16BIT:
			set_reg16(param_val, dest_val);
			break;

		case (STACKPOINTER):
			m_sp.d = dest_val;
			break;
		}
	}
	else
	{
		switch (param_type & MODE_MASK)
		{
		case ABSOLUTE_VAL_8:
			logerror("illegal dest ABSOLUTE_VAL_8\n");
			break;
		case REG_8BIT:
			set_reg8(param_val, dest_val);
			break;
		}
	}
}

uint16_t tlcs870_device::get_source_val(uint16_t param_type, uint16_t param_val)
{
	uint16_t ret_val = 0x0000;

	if (param_type & IS16BIT)
	{
		switch (param_type)
		{
		case ABSOLUTE_VAL_16:
			ret_val = param_val;
			break;

		case REG_16BIT:
			ret_val = get_reg16(param_val);
			break;

		case (STACKPOINTER):
			ret_val = m_sp.d;
			break;
		}
	}
	else
	{
		switch (param_type & MODE_MASK)
		{
		case ABSOLUTE_VAL_8:
			ret_val = param_val;
			break;
		case REG_8BIT:
			ret_val = get_reg8(param_val);
			break;
		}
	}
	return ret_val;
}

void tlcs870_device::setbit_param(uint16_t param_type, uint16_t param, uint8_t bit, bool do_flag)
{
	if (param_type & BITPOS)
	{
		uint8_t bitpos = 0;

		// need to read param 1
		uint16_t addr = 0x0000;
		uint16_t val = 0;

		// READ
		if (param_type & ADDR_IN_BASE)
		{
			addr = get_addr(param_type, param); // any pre/post HL address adjustments happen here
			if (param_type & IS16BIT)
				val = RM16(addr);
			else
				val = RM8(addr);
		}
		else
		{
			val = get_source_val(param_type, param);
		}

		if (param_type & BITPOS_INDIRECT)
		{
			bitpos = get_reg8(m_bitpos & 7) & 0x7;
		}
		else
		{
			bitpos = m_bitpos;
		}

		// MODIFY
		int bitused = (1 << bitpos);

		// this is the flag behavior for the set/clr bit opcodes
		if (do_flag)
		{
			int existingbit = (val & bitused);
			if (existingbit)
			{
				CLEAR_ZF;
				CLEAR_JF; // 'Z' (so copy Z flag?)
			}
			else
			{
				SET_ZF;
				SET_JF;  // 'Z'
			}
		}


		bit ? (val |= bitused) : (val &= ~bitused);

		//printf("bit to set is %d, value is %02x", bitused, val);

		// WRITE
		if (param_type & ADDR_IN_BASE)
		{
			//addr = get_addr(param_type,param); // already have addr, don't want to cause any further HL decrements/increments.
			if (param_type & IS16BIT)
				WM16(addr, val);
			else
				WM8(addr, val);
		}
		else
		{
			set_dest_val(param_type, param, val);
		}
	}
	else
	{
		fatalerror("not a bit op? 0\n");
	}
}

uint8_t tlcs870_device::getbit_param(uint16_t param_type, uint16_t param)
{
	uint8_t bit = 0;

	if (param_type & BITPOS)
	{
		uint8_t bitpos;

		// need to read param 2
		uint16_t addr = 0x0000;
		uint16_t val = 0;
		if (param_type & ADDR_IN_BASE)
		{
			addr = get_addr(param_type, param);
			if (param_type & IS16BIT)
				val = RM16(addr);
			else
				val = RM8(addr);
		}
		else
		{
			val = get_source_val(param_type, param);
		}

		if (param_type & BITPOS_INDIRECT)
		{
			bitpos = get_reg8(m_bitpos & 7) & 0x7;
		}
		else
		{
			bitpos = m_bitpos;
		}

		bit = (val >> bitpos) & 1;
	}
	else
	{
		fatalerror("not a bit op? 0\n");
	}

	return bit;
}

void tlcs870_device::execute_run()
{
	do
	{
		m_prvpc.d = m_pc.d;
		debugger_instruction_hook(m_pc.d);

		//check_interrupts();
		m_temppc = m_pc.d;

		m_addr = m_pc.d;
		decode();
		m_pc.d = m_addr;

		switch (m_op)
		{
		case UNKNOWN:
			// invalid instruction
			break;

		case CALL:
			break;
		case CALLP:
			break;
		case CALLV:
			break;

		case CLR:
			if ((m_param1_type & BITPOS))
			{
				//printf("clr on bit\n");

				if (m_param1_type == CARRYFLAG)
				{
					CLEAR_CF;
					SET_JF;
				}
				else
				{
					setbit_param(m_param1_type, m_param1, 0, true);
				}
			}
			else
			{
				// grouped with the LD operations in the manual but with CLR name, so probably internally just calling LD with 0 value

				uint16_t addr = 0x0000;
				uint16_t val = 0;

				if (m_param1_type & ADDR_IN_BASE)
				{
					addr = get_addr(m_param1_type,m_param1);
					if (m_param1_type & IS16BIT)
						WM16(addr, val);
					else
						WM8(addr, val);
				}
				else
				{
					set_dest_val(m_param1_type,m_param1, val);
				}
				SET_JF;
			}
			break;


		case CPL:
			break;
		case DAA:
			break;
		case DAS:
			break;
		case DEC:
			break;
		/*
		case DI:
		    break;
		*/
		case DIV:
			break;
		/*
		case EI:
		    break;
		*/
		case INC:
		{
			// READ
			uint16_t addr = 0x0000;
			uint16_t val = 0;
			if (m_param1_type & ADDR_IN_BASE)
			{
				addr = get_addr(m_param1_type, m_param1);
				if (m_param1_type & IS16BIT)
					val = RM16(addr);
				else
					val = RM8(addr);
			}
			else
			{
				val = get_source_val(m_param1_type, m_param1);
			}

			// MODIFY
			val++;

			if (!(m_param1_type & IS16BIT))
				val &= 0xff;

			if (val == 0)
			{
				SET_ZF;
				SET_JF;
			}
			else
			{
				CLEAR_ZF;
				CLEAR_JF;
			}

			// WRITE
			if (m_param1_type & ADDR_IN_BASE)
			{
				//addr = get_addr(m_param1_type,m_param1); // already have
				if (m_param1_type & IS16BIT)
					WM16(addr, val);
				else
					WM8(addr, val);
			}
			else
			{
				set_dest_val(m_param1_type,m_param1, val);
			}

			break;
		}
		/*
		case J:
		    break;
		*/
		case JP:
		case JR:
		case JRS:
		{
			bool takejump = true;

			if (m_param1_type == CONDITIONAL)
			{
				switch (m_param1)
				{
				case COND_EQ_Z:
					if (IS_ZF == 1) takejump = true;
					else takejump = false;
					break;

				case COND_NE_NZ:
					if (IS_ZF == 0) takejump = true;
					else takejump = false;
					break;

				case COND_LT_CS:
					if (IS_CF == 1) takejump = true;
					else takejump = false;
					break;

				case COND_GE_CC:
					if (IS_CF == 0) takejump = true;
					else takejump = false;
					break;

				case COND_LE:
					if ((IS_CF || IS_ZF) == 1) takejump = true;
					else takejump = false;
					break;

				case COND_GT:
					if ((IS_CF || IS_ZF) == 0) takejump = true;
					else takejump = false;
					break;

				case COND_T:
					if (IS_JF == 1) takejump = true;
					else takejump = false;
					break;

				case COND_F:
					if (IS_JF == 0) takejump = true;
					else takejump = false;
					break;

				}
			}

			if (takejump)
			{
				// would the address / register be read even if the jump isn't going to be taken?
				uint16_t addr = 0x0000;
				uint16_t val = 0;
				if (m_param2_type & ADDR_IN_BASE)
				{
					addr = get_addr(m_param2_type,m_param2);
					if (m_param2_type & IS16BIT)
						val = RM16(addr);
					else
					{
						fatalerror("8-bit jump destination?");
						//  val = RM8(addr);
					}
				}
				else
				{
					val = get_source_val(m_param2_type,m_param2);
				}

				m_pc.d = val;
			}

			SET_JF;

			break;
		}
		case LD:
		{
			if ((m_param1_type & BITPOS) || (m_param2_type & BITPOS))
			{
				// bit operations, including the 'TEST' style bit instruction
				uint8_t bit = 0;

				if (m_param2_type == CARRYFLAG)
				{
					bit = IS_CF;

					setbit_param(m_param1_type,m_param1,bit, false);

					// for this type of operation ( LD *.b, CF ) the Jump Flag always ends up being 1
					SET_JF;

				}
				else if (m_param1_type == CARRYFLAG)
				{
					getbit_param(m_param2_type,m_param2);

					bit ? SET_CF : CLEAR_CF;
					// for this type of operation ( LD CF, *.b ) the Jump Flag always ends up the inverse of the Carry Flag
					bit ? CLEAR_JF : SET_JF;
				}
				else
				{
					fatalerror("not a bit op?! 2");
				}
			}
			else
			{
				uint16_t addr = 0x0000;
				uint16_t val = 0;
				if (m_param2_type & ADDR_IN_BASE)
				{
					addr = get_addr(m_param2_type,m_param2);
					if (m_param2_type & IS16BIT)
						val = RM16(addr);
					else
						val = RM8(addr);
				}
				else
				{
					val = get_source_val(m_param2_type,m_param2);
				}

				SET_JF; // Jump Flag always gets set

				// some (but not all) LD operations change the Zero Flag, some leave it undefined (for those we don't change it)
				if (m_flagsaffected & FLAG_Z)
				{
					if (val == 0x00) SET_ZF;
					else CLEAR_ZF;
				}

				if (m_param1_type & ADDR_IN_BASE)
				{
					addr = get_addr(m_param1_type,m_param1);
					if (m_param1_type & IS16BIT)
						WM16(addr, val);
					else
						WM8(addr, val);
				}
				else
				{
					set_dest_val(m_param1_type,m_param1, val);
				}
			}
			break;
		}
		case LDW:
			break;
		case MCMP:
			break;
		case MUL:
			break;
		case NOP:
			break;
		case POP:
			break;
		case PUSH:
			break;
		case RET:
			break;
		case RETI:
			break;
		case RETN:
			break;
		case ROLC:
			break;
		case ROLD:
			break;
		case RORC:
			break;
		case RORD:
			break;


		case SET:
			if ((m_param1_type & BITPOS))
			{
				//printf("set on bit\n");

				if (m_param1_type == CARRYFLAG)
				{
					SET_CF;
					CLEAR_JF;
				}
				else
				{
					//printf("set with setbit_param\n");

					setbit_param(m_param1_type, m_param1, 1, true);
				}
			}
			else
			{
				fatalerror("all SET opcode should be bit operations\n");
				/*
				uint16_t addr = 0x0000;
				uint16_t val = 1;

				if (m_param1_type & ADDR_IN_BASE)
				{
				    addr = get_addr(m_param1_type,m_param1);
				    if (m_param1_type & IS16BIT)
				        WM16(addr, val);
				    else
				        WM8(addr, val);
				}
				else
				{
				    set_dest_val(m_param1_type,m_param1, val);
				}
				*/

			}
			break;


		case SHLC:
			break;
		case SHRC:
			break;
		case SWAP:
			break;
		case SWI:
			break;
		/*
		case TEST:
		    break;
		*/
		case XCH:
			break;
		case ALU_ADDC:
			break;
		case ALU_ADD:
			break;
		case ALU_SUBB:
			break;
		case ALU_SUB:
			break;
		case ALU_AND:
			break;
		case ALU_XOR:
			break;
		case ALU_OR:
			break;
		case ALU_CMP:

				uint16_t addr = 0x0000;
				uint16_t val = 0;
				if (m_param2_type & ADDR_IN_BASE)
				{
					addr = get_addr(m_param2_type,m_param2);
					if (m_param2_type & IS16BIT)
						val = RM16(addr);
					else
						val = RM8(addr);
				}
				else
				{
					val = get_source_val(m_param2_type,m_param2);
				}

				uint16_t val2 = val;

				if (m_param1_type & ADDR_IN_BASE)
				{
					addr = get_addr(m_param1_type,m_param1);
					if (m_param1_type & IS16BIT)
						val = RM16(addr);
					else
						val = RM8(addr);
				}
				else
				{
					val = get_source_val(m_param1_type,m_param1);
				}

				if (val < val2)
				{
					SET_CF;
				}
				else
				{
					CLEAR_CF;
				}

				if (val == val2)
				{
					SET_ZF;
					SET_JF;
				}
				else
				{
					CLEAR_ZF;
					CLEAR_JF;
				}

				// TODO: HF (how to calculate it?)


			break;
		}

		m_icount-=m_cycles*4; // 1 machine cycle = 4 clock cycles?


	} while( m_icount > 0 );
}

void tlcs870_device::device_reset()
{
	// todo, read from top address
	m_pc.d = 0xc030;

	m_RBS = 0;

}

uint8_t tlcs870_device::get_reg8(int reg)
{
	return m_intram[((m_RBS & 0xf) * 8) + (reg & 0x7)];
}

void tlcs870_device::set_reg8(int reg, uint8_t val)
{
	m_intram[((m_RBS & 0xf) * 8) + (reg & 0x7)] = val;
}

uint16_t tlcs870_device::get_reg16(int reg)
{
	uint16_t res = 0;
	res |= get_reg8(((reg & 0x3) * 2) + 1) << 8;
	res |= get_reg8(((reg & 0x3) * 2) + 0) << 0;
	return res;
}

void tlcs870_device::set_reg16(int reg, uint16_t val)
{
	set_reg8(((reg & 0x3) * 2) + 1, (val & 0xff00) >> 8);
	set_reg8(((reg & 0x3) * 2) + 0, (val & 0x00ff) >> 0);
}



void tlcs870_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case DEBUGGER_REG_A:
			set_reg8(REG_A, m_debugger_temp);
			break;

		case DEBUGGER_REG_W:
			set_reg8(REG_W, m_debugger_temp);
			break;

		case DEBUGGER_REG_C:
			set_reg8(REG_C, m_debugger_temp);
			break;

		case DEBUGGER_REG_B:
			set_reg8(REG_B, m_debugger_temp);
			break;

		case DEBUGGER_REG_E:
			set_reg8(REG_E, m_debugger_temp);
			break;

		case DEBUGGER_REG_D:
			set_reg8(REG_D, m_debugger_temp);
			break;

		case DEBUGGER_REG_L:
			set_reg8(REG_L, m_debugger_temp);
			break;

		case DEBUGGER_REG_H:
			set_reg8(REG_H, m_debugger_temp);
			break;

		case DEBUGGER_REG_WA:
			set_reg16(REG_WA, m_debugger_temp);
			break;

		case DEBUGGER_REG_BC:
			set_reg16(REG_BC, m_debugger_temp);
			break;

		case DEBUGGER_REG_DE:
			set_reg16(REG_DE, m_debugger_temp);
			break;

		case DEBUGGER_REG_HL:
			set_reg16(REG_HL, m_debugger_temp);
			break;
	}
}


void tlcs870_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case DEBUGGER_REG_A:
			m_debugger_temp = get_reg8(REG_A);
			break;

		case DEBUGGER_REG_W:
			m_debugger_temp = get_reg8(REG_W);
			break;

		case DEBUGGER_REG_C:
			m_debugger_temp = get_reg8(REG_C);
			break;

		case DEBUGGER_REG_B:
			m_debugger_temp = get_reg8(REG_B);
			break;

		case DEBUGGER_REG_E:
			m_debugger_temp = get_reg8(REG_E);
			break;

		case DEBUGGER_REG_D:
			m_debugger_temp = get_reg8(REG_D);
			break;

		case DEBUGGER_REG_L:
			m_debugger_temp = get_reg8(REG_L);
			break;

		case DEBUGGER_REG_H:
			m_debugger_temp = get_reg8(REG_H);
			break;

		case DEBUGGER_REG_WA:
			m_debugger_temp = get_reg16(REG_WA);
			break;

		case DEBUGGER_REG_BC:
			m_debugger_temp = get_reg16(REG_BC);
			break;

		case DEBUGGER_REG_DE:
			m_debugger_temp = get_reg16(REG_DE);
			break;

		case DEBUGGER_REG_HL:
			m_debugger_temp = get_reg16(REG_HL);
			break;

	}
}


void tlcs870_device::device_start()
{
//  int i, p;
	m_sp.d = 0x0000;
	m_F = 0;

	m_program = &space(AS_PROGRAM);
	m_io = &space(AS_IO);

	state_add(DEBUGGER_REG_A, "A", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_W, "W", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_C, "C", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_B, "B", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_E, "E", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_D, "D", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_L, "L", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_H, "H", m_debugger_temp).callimport().callexport().formatstr("%02X");

	state_add(DEBUGGER_REG_WA, "WA", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add(DEBUGGER_REG_BC, "BC", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add(DEBUGGER_REG_DE, "DE", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add(DEBUGGER_REG_HL, "HL", m_debugger_temp).callimport().callexport().formatstr("%04X");


	state_add(STATE_GENPC, "GENPC", m_pc.w.l).formatstr("%04X");
	state_add(STATE_GENPCBASE, "CURPC", m_prvpc.w.l).formatstr("%04X").noshow();
	state_add(STATE_GENSP, "GENSP", m_sp.w.l).formatstr("%04X");
	state_add(STATE_GENFLAGS, "GENFLAGS", m_F ).formatstr("%8s").noshow();

	set_icountptr(m_icount);
}


void tlcs870_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	int F = m_F;

	switch (entry.index())
	{

		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c",
				F & 0x80 ? 'J':'.',
				F & 0x40 ? 'Z':'.',
				F & 0x20 ? 'C':'.',
				F & 0x10 ? 'H':'.'
			);
			break;
	}

}

std::unique_ptr<util::disasm_interface> tlcs870_device::create_disassembler()
{
	return std::make_unique<tlcs870_disassembler>();
}
