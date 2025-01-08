// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    TODO:

    - memory mapping with PLA
    - PLA dump

*/

#include "emu.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "bus/pet/cass.h"
#include "bus/vic10/exp.h"
#include "bus/vcs_ctrl/ctrl.h"
#include "cpu/m6502/m6510.h"
#include "machine/input_merger.h"
#include "machine/mos6526.h"
#include "machine/ram.h"
#include "sound/mos6581.h"
#include "video/mos6566.h"


namespace {

#define MOS6566_TAG     "u2"
#define MOS6581_TAG     "u6"
#define MOS6526_TAG     "u9"
#define SCREEN_TAG      "screen"
#define TIMER_C1531_TAG "c1531"
#define CONTROL1_TAG    "joy1"
#define CONTROL2_TAG    "joy2"

class vic10_state : public driver_device
{
public:
	vic10_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "u3"),
		m_vic(*this, MOS6566_TAG),
		m_sid(*this, MOS6581_TAG),
		m_cia(*this, MOS6526_TAG),
		m_joy1(*this, CONTROL1_TAG),
		m_joy2(*this, CONTROL2_TAG),
		m_exp(*this, "exp"),
		m_ram(*this, RAM_TAG),
		m_cassette(*this, PET_DATASSETTE_PORT_TAG),
		m_color_ram(*this, "color_ram", 0x400, ENDIANNESS_LITTLE),
		m_row(*this, "ROW%u", 0),
		m_restore(*this, "RESTORE"),
		m_lock(*this, "LOCK")
	{ }

	void vic10(machine_config &config);

private:
	required_device<m6510_device> m_maincpu;
	required_device<mos6566_device> m_vic;
	required_device<mos6581_device> m_sid;
	required_device<mos6526_device> m_cia;
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	required_device<vic10_expansion_slot_device> m_exp;
	required_device<ram_device> m_ram;
	optional_device<pet_datassette_port_device> m_cassette;
	memory_share_creator<uint8_t> m_color_ram;
	required_ioport_array<8> m_row;
	required_ioport m_restore;
	required_ioport m_lock;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t vic_videoram_r(offs_t offset);
	uint8_t vic_colorram_r(offs_t offset);

	uint8_t sid_potx_r();
	uint8_t sid_poty_r();

	uint8_t cia_pa_r();
	uint8_t cia_pb_r();
	void cia_pb_w(uint8_t data);

	uint8_t cpu_r();
	void cpu_w(uint8_t data);

	void exp_reset_w(int state);

	void vic10_mem(address_map &map) ATTR_COLD;
	void vic_colorram_map(address_map &map) ATTR_COLD;
	void vic_videoram_map(address_map &map) ATTR_COLD;
};


//**************************************************************************
//  MEMORY MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t vic10_state::read(offs_t offset)
{
	// TODO this is really handled by the PLA

	uint8_t data = m_vic->bus_r();
	int lorom = 1, uprom = 1, exram = 1;

	if (offset < 0x800)
	{
		data = m_ram->pointer()[offset];
	}
	else if (offset < 0x1000)
	{
		exram = 0;
	}
	else if (offset >= 0x8000 && offset < 0xa000)
	{
		lorom = 0;
	}
	else if (offset >= 0xd000 && offset < 0xd400)
	{
		data = m_vic->read(offset & 0x3f);
	}
	else if (offset >= 0xd400 && offset < 0xd800)
	{
		data = m_sid->read(offset & 0x1f);
	}
	else if (offset >= 0xd800 && offset < 0xdc00)
	{
		data = m_color_ram[offset & 0x3ff];
	}
	else if (offset >= 0xdc00 && offset < 0xe000)
	{
		data = m_cia->read(offset & 0x0f);
	}
	else if (offset >= 0xe000)
	{
		uprom = 0;
	}

	return m_exp->cd_r(offset, data, lorom, uprom, exram);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void vic10_state::write(offs_t offset, uint8_t data)
{
	// TODO this is really handled by the PLA

	int lorom = 1, uprom = 1, exram = 1;

	if (offset < 0x800)
	{
		m_ram->pointer()[offset] = data;
	}
	else if (offset < 0x1000)
	{
		exram = 0;
	}
	else if (offset >= 0xd000 && offset < 0xd400)
	{
		m_vic->write(offset & 0x3f, data);
	}
	else if (offset >= 0xd400 && offset < 0xd800)
	{
		m_sid->write(offset & 0x1f, data);
	}
	else if (offset >= 0xd800 && offset < 0xdc00)
	{
		m_color_ram[offset & 0x3ff] = data & 0x0f;
	}
	else if (offset >= 0xdc00 && offset < 0xe000)
	{
		m_cia->write(offset & 0x0f, data);
	}

	m_exp->cd_w(offset, data, lorom, uprom, exram);
}


//-------------------------------------------------
//  vic_videoram_r -
//-------------------------------------------------

uint8_t vic10_state::vic_videoram_r(offs_t offset)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	if (offset < 0x3000)
		return program.read_byte(offset);

	return program.read_byte(0xe000 + (offset & 0x1fff));
}


//-------------------------------------------------
//  vic_colorram_r -
//-------------------------------------------------

uint8_t vic10_state::vic_colorram_r(offs_t offset)
{
	return m_color_ram[offset];
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( vic10_mem )
//-------------------------------------------------

void vic10_state::vic10_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(vic10_state::read), FUNC(vic10_state::write));
}


//-------------------------------------------------
//  ADDRESS_MAP( vic_videoram_map )
//-------------------------------------------------

void vic10_state::vic_videoram_map(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(vic10_state::vic_videoram_r));
}


//-------------------------------------------------
//  ADDRESS_MAP( vic_colorram_map )
//-------------------------------------------------

void vic10_state::vic_colorram_map(address_map &map)
{
	map(0x000, 0x3ff).r(FUNC(vic10_state::vic_colorram_r));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( vic10 )
//-------------------------------------------------

static INPUT_PORTS_START( vic10 )
	PORT_START( "ROW0" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Down Up") PORT_CODE(KEYCODE_RALT)        PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)                                    PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)                                    PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)                                    PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4)                                    PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Right Left") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)             PORT_CHAR(13)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INST DEL") PORT_CODE(KEYCODE_BACKSPACE)       PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START( "ROW1" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Left)") PORT_CODE(KEYCODE_LSHIFT)      PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)         PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)         PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)         PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_CHAR('3') PORT_CHAR('#')

	PORT_START( "ROW2" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)         PORT_CHAR('X')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)         PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)         PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)         PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)         PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_CHAR('R')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%')

	PORT_START( "ROW3" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)         PORT_CHAR('V')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)         PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)         PORT_CHAR('H')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)         PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)         PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)         PORT_CHAR('Y')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START( "ROW4" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)         PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)         PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)         PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)         PORT_CHAR('0')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)         PORT_CHAR('J')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)         PORT_CHAR('I')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR(')')

	PORT_START( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('-')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_CHAR('L')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)         PORT_CHAR('P')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('+')

	PORT_START( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)                             PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi") PORT_CODE(KEYCODE_DEL) PORT_CHAR(0x2191) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)                         PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CLR HOME") PORT_CODE(KEYCODE_INSERT)      PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)                             PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)                        PORT_CHAR('*')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)                        PORT_CHAR(0xA3)

	PORT_START( "ROW7" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUN STOP") PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)                                 PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CBM") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)                             PORT_CHAR(' ')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)                                 PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)                               PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x90") PORT_CODE(KEYCODE_TILDE)   PORT_CHAR(0x2190)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)                                 PORT_CHAR('1') PORT_CHAR('!')

	PORT_START( "RESTORE" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESTORE") PORT_CODE(KEYCODE_PRTSCR)

	PORT_START( "LOCK" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  sid6581_interface sid_intf
//-------------------------------------------------

uint8_t vic10_state::sid_potx_r()
{
	uint8_t data = 0xff;

	switch (m_cia->pa_r() >> 6)
	{
	case 1: data = m_joy1->read_pot_x(); break;
	case 2: data = m_joy2->read_pot_x(); break;
	case 3:
		if (m_joy1->has_pot_x() && m_joy2->has_pot_x())
		{
			data = 1 / (1 / m_joy1->read_pot_x() + 1 / m_joy2->read_pot_x());
		}
		else if (m_joy1->has_pot_x())
		{
			data = m_joy1->read_pot_x();
		}
		else if (m_joy2->has_pot_x())
		{
			data = m_joy2->read_pot_x();
		}
		break;
	}

	return data;
}

uint8_t vic10_state::sid_poty_r()
{
	uint8_t data = 0xff;

	switch (m_cia->pa_r() >> 6)
	{
	case 1: data = m_joy1->read_pot_y(); break;
	case 2: data = m_joy2->read_pot_y(); break;
	case 3:
		if (m_joy1->has_pot_y() && m_joy2->has_pot_y())
		{
			data = 1 / (1 / m_joy1->read_pot_y() + 1 / m_joy2->read_pot_y());
		}
		else if (m_joy1->has_pot_y())
		{
			data = m_joy1->read_pot_y();
		}
		else if (m_joy2->has_pot_y())
		{
			data = m_joy2->read_pot_y();
		}
		break;
	}

	return data;
}


//-------------------------------------------------
//  MOS6526_INTERFACE( cia_intf )
//-------------------------------------------------

uint8_t vic10_state::cia_pa_r()
{
	/*

	    bit     description

	    PA0     COL0, JOY B0
	    PA1     COL1, JOY B1
	    PA2     COL2, JOY B2
	    PA3     COL3, JOY B3
	    PA4     COL4, BTNB
	    PA5     COL5
	    PA6     COL6
	    PA7     COL7

	*/

	uint8_t data = 0xff;

	// joystick
	uint8_t joy_b = m_joy2->read_joy();

	data &= (0xf0 | (joy_b & 0x0f));
	data &= ~(!BIT(joy_b, 5) << 4);

	// keyboard
	uint8_t cia_pb = m_cia->pb_r();
	uint32_t row[8] = { m_row[0]->read(), m_row[1]->read() & m_lock->read(), m_row[2]->read(), m_row[3]->read(),
						m_row[4]->read(), m_row[5]->read(), m_row[6]->read(), m_row[7]->read() };

	for (int i = 0; i < 8; i++)
	{
		if (!BIT(cia_pb, i))
		{
			if (!BIT(row[7], i)) data &= ~0x80;
			if (!BIT(row[6], i)) data &= ~0x40;
			if (!BIT(row[5], i)) data &= ~0x20;
			if (!BIT(row[4], i)) data &= ~0x10;
			if (!BIT(row[3], i)) data &= ~0x08;
			if (!BIT(row[2], i)) data &= ~0x04;
			if (!BIT(row[1], i)) data &= ~0x02;
			if (!BIT(row[0], i)) data &= ~0x01;
		}
	}

	return data;
}

uint8_t vic10_state::cia_pb_r()
{
	/*

	    bit     description

	    PB0     JOY A0
	    PB1     JOY A1
	    PB2     JOY A2
	    PB3     JOY A3
	    PB4     BTNA/_LP
	    PB5
	    PB6
	    PB7

	*/

	uint8_t data = 0xff;

	// joystick
	uint8_t joy_a = m_joy1->read_joy();

	data &= (0xf0 | (joy_a & 0x0f));
	data &= ~(!BIT(joy_a, 5) << 4);

	// keyboard
	uint8_t cia_pa = m_cia->pa_r();

	if (!BIT(cia_pa, 7)) data &= m_row[7]->read();
	if (!BIT(cia_pa, 6)) data &= m_row[6]->read();
	if (!BIT(cia_pa, 5)) data &= m_row[5]->read();
	if (!BIT(cia_pa, 4)) data &= m_row[4]->read();
	if (!BIT(cia_pa, 3)) data &= m_row[3]->read();
	if (!BIT(cia_pa, 2)) data &= m_row[2]->read();
	if (!BIT(cia_pa, 1)) data &= m_row[1]->read() & m_lock->read();
	if (!BIT(cia_pa, 0)) data &= m_row[0]->read();

	return data;
}

void vic10_state::cia_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0     ROW0
	    PB1     ROW1
	    PB2     ROW2
	    PB3     ROW3
	    PB4     ROW4
	    PB5     ROW5
	    PB6     ROW6
	    PB7     ROW7

	*/

	m_vic->lp_w(BIT(data, 4));
}


//-------------------------------------------------
//  M6510_INTERFACE( cpu_intf )
//-------------------------------------------------

uint8_t vic10_state::cpu_r()
{
	/*

	    bit     description

	    P0      EXPANSION PORT
	    P1
	    P2
	    P3
	    P4      CASS SENS
	    P5      0

	*/

	uint8_t data = 0;

	// expansion port
	data |= m_exp->p0_r();

	// cassette sense
	data |= m_cassette->sense_r() << 4;

	return data;
}

void vic10_state::cpu_w(uint8_t data)
{
	/*

	    bit     description

	    P0      EXPANSION PORT
	    P1
	    P2
	    P3      CASS WRT
	    P4
	    P5      CASS MOTOR

	*/

	if (0 /*BIT(offset, 0)*/) // what offset?
	{
		m_exp->p0_w(BIT(data, 0));
	}

	// cassette write
	m_cassette->write(BIT(data, 3));

	// cassette motor
	m_cassette->motor_w(BIT(data, 5));
}


//-------------------------------------------------
//  VIC10_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

void vic10_state::exp_reset_w(int state)
{
	if (state == ASSERT_LINE)
	{
		machine_reset();
	}
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( vic10 )
//-------------------------------------------------

void vic10_state::machine_start()
{
	// initialize memory
	uint8_t data = 0xff;

	for (offs_t offset = 0; offset < m_ram->size(); offset++)
	{
		m_ram->pointer()[offset] = data;
		if (!(offset % 64)) data ^= 0xff;
	}
}


void vic10_state::machine_reset()
{
	m_maincpu->reset();

	m_vic->reset();
	m_sid->reset();
	m_cia->reset();

	m_exp->reset();
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  machine_config( vic10 )
//-------------------------------------------------

void vic10_state::vic10(machine_config &config)
{
	// basic hardware
	M6510(config, m_maincpu, XTAL(8'000'000)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &vic10_state::vic10_mem);
	m_maincpu->read_callback().set(FUNC(vic10_state::cpu_r));
	m_maincpu->write_callback().set(FUNC(vic10_state::cpu_w));
	m_maincpu->set_pulls(0x10, 0x20);
	config.set_perfect_quantum(m_maincpu);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, m6510_device::IRQ_LINE);

	// video hardware
	mos8566_device &mos8566(MOS8566(config, MOS6566_TAG, XTAL(8'000'000)/8));
	mos8566.set_cpu(m_maincpu);
	mos8566.irq_callback().set("mainirq", FUNC(input_merger_device::in_w<1>));
	mos8566.set_screen(SCREEN_TAG);
	mos8566.set_addrmap(0, &vic10_state::vic_videoram_map);
	mos8566.set_addrmap(1, &vic10_state::vic_colorram_map);

	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(VIC6566_VRETRACERATE);
	screen.set_size(VIC6567_COLUMNS, VIC6567_LINES);
	screen.set_visarea(0, VIC6567_VISIBLECOLUMNS - 1, 0, VIC6567_VISIBLELINES - 1);
	screen.set_screen_update(MOS6566_TAG, FUNC(mos6566_device::screen_update));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	MOS6581(config, m_sid, XTAL(8'000'000)/8);
	m_sid->potx().set(FUNC(vic10_state::sid_potx_r));
	m_sid->poty().set(FUNC(vic10_state::sid_poty_r));
	m_sid->add_route(ALL_OUTPUTS, "mono", 1.00);

	// devices
	MOS6526(config, m_cia, XTAL(8'000'000)/8);
	m_cia->set_tod_clock(60);
	m_cia->irq_wr_callback().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_cia->cnt_wr_callback().set(m_exp, FUNC(vic10_expansion_slot_device::cnt_w));
	m_cia->sp_wr_callback().set(m_exp, FUNC(vic10_expansion_slot_device::sp_w));
	m_cia->pa_rd_callback().set(FUNC(vic10_state::cia_pa_r));
	m_cia->pb_rd_callback().set(FUNC(vic10_state::cia_pb_r));
	m_cia->pb_wr_callback().set(FUNC(vic10_state::cia_pb_w));

	PET_DATASSETTE_PORT(config, m_cassette, cbm_datassette_devices, "c1530");
	m_cassette->read_handler().set(m_cia, FUNC(mos6526_device::flag_w));

	VCS_CONTROL_PORT(config, m_joy1, vcs_control_port_devices, nullptr);
	m_joy1->trigger_wr_callback().set(MOS6566_TAG, FUNC(mos6566_device::lp_w));
	VCS_CONTROL_PORT(config, m_joy2, vcs_control_port_devices, "joy");

	VIC10_EXPANSION_SLOT(config, m_exp, XTAL(8'000'000)/8, vic10_expansion_cards, nullptr);
	m_exp->set_must_be_loaded(true);
	m_exp->irq_callback().set("mainirq", FUNC(input_merger_device::in_w<2>));
	m_exp->res_callback().set(FUNC(vic10_state::exp_reset_w));
	m_exp->cnt_callback().set(m_cia, FUNC(mos6526_device::cnt_w));
	m_exp->sp_callback().set(m_cia, FUNC(mos6526_device::sp_w));

	// software list
	SOFTWARE_LIST(config, "cart_list").set_original("vic10");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("4K");
}



//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( vic10 )
	ROM_REGION( 0x100, "pla", 0 )
	ROM_LOAD( "6703.u4", 0x000, 0x100, NO_DUMP )
ROM_END

} // anonymous namespace


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

COMP( 1982, vic10, 0, 0, vic10, vic10, vic10_state, empty_init, "Commodore Business Machines", "VIC-10 / Max Machine / UltiMax (NTSC)", MACHINE_SUPPORTS_SAVE )
