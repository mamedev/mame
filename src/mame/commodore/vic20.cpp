// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*
    TODO:

    - C1540 is not working currently
    - mos6560_port_r/w should respond at 0x1000-0x100f
    - VIC21 (built in 21K ram)

*/

#include "emu.h"

#include "cbm_snqk.h"

#include "bus/cbmiec/cbmiec.h"
#include "bus/pet/cass.h"
#include "bus/vcs_ctrl/ctrl.h"
#include "bus/vic20/exp.h"
#include "bus/vic20/user.h"
#include "cpu/m6502/m6510.h"
#include "imagedev/snapquik.h"
#include "machine/6522via.h"
#include "machine/ram.h"
#include "sound/mos6560.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

#define M6502_TAG       "ue10"
#define M6522_1_TAG     "uab3"
#define M6522_2_TAG     "uab1"
#define VIC_TAG         "ub7"
#define IEC_TAG         "iec"
#define SCREEN_TAG      "screen"
#define CONTROL1_TAG    "joy1"
#define PET_USER_PORT_TAG     "user"

class vic20_state : public driver_device
{
public:
	vic20_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, M6502_TAG),
		m_via1(*this, M6522_1_TAG),
		m_via2(*this, M6522_2_TAG),
		m_vic(*this, VIC_TAG),
		m_iec(*this, CBM_IEC_TAG),
		m_joy(*this, CONTROL1_TAG),
		m_exp(*this, "exp"),
		m_user(*this, PET_USER_PORT_TAG),
		m_cassette(*this, PET_DATASSETTE_PORT_TAG),
		m_ram(*this, RAM_TAG),
		m_screen(*this, SCREEN_TAG),
		m_basic(*this, "basic"),
		m_kernal(*this, "kernal"),
		m_charom(*this, "charom"),
		m_color_ram(*this, "color_ram"),
		m_col(*this, "COL%u", 0),
		m_restore(*this, "RESTORE"),
		m_lock(*this, "LOCK")
	{ }

	void ntsc(machine_config &config);
	void pal(machine_config &config);

protected:
	void vic20(machine_config &config, const char* softlist_filter);
	void add_clocked_devices(machine_config &config, uint32_t clock);

private:
	required_device<m6502_device> m_maincpu;
	required_device<via6522_device> m_via1;
	required_device<via6522_device> m_via2;
	required_device<mos6560_device> m_vic;
	required_device<cbm_iec_device> m_iec;
	required_device<vcs_control_port_device> m_joy;
	required_device<vic20_expansion_slot_device> m_exp;
	required_device<pet_user_port_device> m_user;
	required_device<pet_datassette_port_device> m_cassette;
	required_device<ram_device> m_ram;
	required_device<screen_device> m_screen;
	required_region_ptr<uint8_t> m_basic;
	required_region_ptr<uint8_t> m_kernal;
	required_region_ptr<uint8_t> m_charom;
	required_shared_ptr<uint8_t> m_color_ram;
	required_ioport_array<8> m_col;
	required_ioport m_restore;
	required_ioport m_lock;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t vic_videoram_r(offs_t offset);

	void write_light_pen(int state);
	void write_user_joy0(int state);
	void write_user_joy1(int state);
	void write_user_joy2(int state);
	void write_user_light_pen(int state);
	void write_user_cassette_switch(int state);

	uint8_t via1_pa_r();
	void via1_pa_w(uint8_t data);
	void via1_pb_w(uint8_t data);

	uint8_t via2_pa_r();
	uint8_t via2_pb_r();
	void via2_pa_w(uint8_t data);
	void via2_pb_w(uint8_t data);
	void via2_ca2_w(int state);
	void via2_cb2_w(int state);

	void exp_reset_w(int state);

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_vc20);
	// keyboard state
	int m_key_row;
	int m_key_col;
	int m_light_pen;
	int m_user_joy0;
	int m_user_joy1;
	int m_user_joy2;
	int m_user_light_pen;
	int m_user_cassette_switch;

	enum
	{
		BLK0 = 0,
		BLK1,
		BLK2,
		BLK3,
		BLK4,
		BLK5,
		BLK6,
		BLK7
	};


	enum
	{
		RAM0 = 0,
		RAM1,
		RAM2,
		RAM3,
		RAM4,
		RAM5,
		RAM6,
		RAM7
	};


	enum
	{
		IO0 = 4,
		COLOR = 5,
		IO2 = 6,
		IO3 = 7
	};

	void vic20_mem(address_map &map) ATTR_COLD;
	void vic_colorram_map(address_map &map) ATTR_COLD;
	void vic_videoram_map(address_map &map) ATTR_COLD;
};


QUICKLOAD_LOAD_MEMBER(vic20_state::quickload_vc20)
{
	return general_cbm_loadsnap(image, m_maincpu->space(AS_PROGRAM), 0, cbm_quick_sethiaddress);
}

//**************************************************************************
//  MEMORY MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t vic20_state::read(offs_t offset)
{
	uint8_t data = m_vic->bus_r();

	int ram1 = 1, ram2 = 1, ram3 = 1;
	int blk1 = 1, blk2 = 1, blk3 = 1, blk5 = 1;
	int io2 = 1, io3 = 1;

	switch ((offset >> 13) & 0x07)
	{
	case BLK0:
		switch ((offset >> 10) & 0x07)
		{
		case RAM0:
			data = m_ram->pointer()[offset & 0x3ff];
			break;

		case RAM1: ram1 = 0; break;
		case RAM2: ram2 = 0; break;
		case RAM3: ram3 = 0; break;

		default:
			data = m_ram->pointer()[0x400 + (offset & 0xfff)];
			break;
		}
		break;

	case BLK1: blk1 = 0; break;
	case BLK2: blk2 = 0; break;
	case BLK3: blk3 = 0; break;

	case BLK4:
		switch ((offset >> 10) & 0x07)
		{
		default:
			data = m_charom[offset & 0xfff];
			break;

		case IO0:
			if (BIT(offset, 4))
			{
				data = m_via1->read(offset & 0x0f);
			}
			else if (BIT(offset, 5))
			{
				data = m_via2->read(offset & 0x0f);
			}
			else if (offset >= 0x9000 && offset < 0x9010)
			{
				data = m_vic->read(offset & 0x0f);
			}
			break;

		case COLOR:
			data = m_color_ram[offset & 0x3ff];
			break;

		case IO2: io2 = 0; break;
		case IO3: io3 = 0; break;
		}
		break;

	case BLK5: blk5 = 0; break;

	case BLK6:
		data = m_basic[offset & 0x1fff];
		break;

	case BLK7:
		data = m_kernal[offset & 0x1fff];
		break;
	}

	return m_exp->cd_r(offset & 0x1fff, data, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void vic20_state::write(offs_t offset, uint8_t data)
{
	int ram1 = 1, ram2 = 1, ram3 = 1;
	int blk1 = 1, blk2 = 1, blk3 = 1, blk5 = 1;
	int io2 = 1, io3 = 1;

	switch ((offset >> 13) & 0x07)
	{
	case BLK0:
		switch ((offset >> 10) & 0x07)
		{
		case RAM0:
			m_ram->pointer()[offset] = data;
			break;

		case RAM1: ram1 = 0; break;
		case RAM2: ram2 = 0; break;
		case RAM3: ram3 = 0; break;

		default:
			m_ram->pointer()[0x400 + (offset & 0xfff)] = data;
			break;
		}
		break;

	case BLK1: blk1 = 0; break;
	case BLK2: blk2 = 0; break;
	case BLK3: blk3 = 0; break;

	case BLK4:
		switch ((offset >> 10) & 0x07)
		{
		case IO0:
			if (BIT(offset, 4))
			{
				m_via1->write(offset & 0x0f, data);
			}
			else if (BIT(offset, 5))
			{
				m_via2->write(offset & 0x0f, data);
			}
			else if (offset >= 0x9000 && offset < 0x9010)
			{
				m_vic->write(offset & 0x0f, data);
			}
			break;

		case COLOR:
			m_color_ram[offset & 0x3ff] = data & 0x0f;
			break;

		case IO2: io2 = 0; break;
		case IO3: io3 = 0; break;
		}
		break;

	case BLK5: blk5 = 0; break;
	}

	m_exp->cd_w(offset & 0x1fff, data, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);
}


//-------------------------------------------------
//  vic_videoram_r -
//-------------------------------------------------

uint8_t vic20_state::vic_videoram_r(offs_t offset)
{
	int ram1 = 1, ram2 = 1, ram3 = 1;
	int blk1 = 1, blk2 = 1, blk3 = 1, blk5 = 1;
	int io2 = 1, io3 = 1;

	uint8_t data = 0;

	if (BIT(offset, 13))
	{
		switch ((offset >> 10) & 0x07)
		{
		case RAM0:
			data = m_ram->pointer()[offset & 0x3ff];
			break;

		case RAM1: ram1 = 0; break;
		case RAM2: ram2 = 0; break;
		case RAM3: ram3 = 0; break;

		default:
			data = m_ram->pointer()[0x400 + (offset & 0xfff)];
			break;
		}
	}
	else
	{
		data = m_charom[offset & 0xfff];
	}

	return m_exp->cd_r(offset & 0x1fff, data, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( vic20_mem )
//-------------------------------------------------

void vic20_state::vic20_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(vic20_state::read), FUNC(vic20_state::write));
}


//-------------------------------------------------
//  ADDRESS_MAP( vic_videoram_map )
//-------------------------------------------------

void vic20_state::vic_videoram_map(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(vic20_state::vic_videoram_r));
}


//-------------------------------------------------
//  ADDRESS_MAP( vic_colorram_map )
//-------------------------------------------------

void vic20_state::vic_colorram_map(address_map &map)
{
	map(0x000, 0x3ff).ram().share("color_ram");
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( vic20 )
//-------------------------------------------------

static INPUT_PORTS_START( vic20 )
	PORT_START( "COL0" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Del  Inst") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)     PORT_CHAR(0xA3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)          PORT_CHAR('+')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)              PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)              PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)              PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)              PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)              PORT_CHAR('1') PORT_CHAR('!')

	PORT_START( "COL1" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)     PORT_CHAR('*')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)              PORT_CHAR('P')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)              PORT_CHAR('I')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)              PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)              PORT_CHAR('R')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)              PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x90") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x2190)

	PORT_START( "COL2" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Right Left") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)          PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)              PORT_CHAR('L')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)              PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)              PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)              PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)              PORT_CHAR('A')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)            PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START( "COL3" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Down Up") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)          PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)          PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)              PORT_CHAR('N')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)              PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)              PORT_CHAR('X')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Left)") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Stop Run") PORT_CODE(KEYCODE_HOME)

	PORT_START( "COL4" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)             PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)           PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)              PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)              PORT_CHAR('B')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)              PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)              PORT_CHAR('Z')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)          PORT_CHAR(' ')

	PORT_START( "COL5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)             PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)      PORT_CHAR('=')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)          PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)              PORT_CHAR('K')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)              PORT_CHAR('H')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)              PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)              PORT_CHAR('S')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CBM") PORT_CODE(KEYCODE_LCONTROL)

	PORT_START( "COL6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)             PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi") PORT_CODE(KEYCODE_DEL) PORT_CHAR(0x2191,'^') PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)      PORT_CHAR('@')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)              PORT_CHAR('O')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)              PORT_CHAR('U')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)              PORT_CHAR('T')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)              PORT_CHAR('E')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)              PORT_CHAR('Q')

	PORT_START( "COL7" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4)             PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Home  Clr") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)         PORT_CHAR('-')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)              PORT_CHAR('0')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)              PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)              PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)              PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)              PORT_CHAR('2') PORT_CHAR('"')

	PORT_START( "RESTORE" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESTORE") PORT_CODE(KEYCODE_PRTSCR) PORT_WRITE_LINE_DEVICE_MEMBER(M6522_1_TAG, via6522_device, write_ca1)

	PORT_START( "LOCK" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( vic1001 )
//-------------------------------------------------

static INPUT_PORTS_START( vic1001 )
	PORT_INCLUDE( vic20 )

	PORT_MODIFY( "COL0" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(0xA5)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( vic20s )
//-------------------------------------------------

static INPUT_PORTS_START( vic20s )
	PORT_INCLUDE( vic20 )

	PORT_MODIFY( "COL0" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)     PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)          PORT_CHAR('-')

	PORT_MODIFY( "COL1" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)     PORT_CHAR('@')

	PORT_MODIFY( "COL2" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)          PORT_CHAR(0x00C4)

	PORT_MODIFY( "COL5" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)          PORT_CHAR(0x00D6)

	PORT_MODIFY( "COL6" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)      PORT_CHAR(0x00C5)

	PORT_MODIFY( "COL7" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)         PORT_CHAR('=')
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

uint8_t vic20_state::via1_pa_r()
{
	/*

	    bit     description

	    PA0     SERIAL CLK IN
	    PA1     SERIAL DATA IN
	    PA2     JOY 0 (UP)
	    PA3     JOY 1 (DOWN)
	    PA4     JOY 2 (LEFT)
	    PA5     LITE PEN (FIRE)
	    PA6     CASS SWITCH
	    PA7

	*/

	uint8_t data = 0;

	// serial clock in
	data |= m_iec->clk_r();

	// serial data in
	data |= m_iec->data_r() << 1;

	// joystick / user port
	uint8_t joy = m_joy->read_joy();

	data |= (m_user_joy0 && BIT(joy, 0)) << 2;
	data |= (m_user_joy1 && BIT(joy, 1)) << 3;
	data |= (m_user_joy2 && BIT(joy, 2)) << 4;
	data |= (m_user_light_pen && BIT(joy, 5)) << 5;

	// cassette switch
	data |= (m_user_cassette_switch && m_cassette->sense_r()) << 6;

	return data;
}

void vic20_state::via1_pa_w(uint8_t data)
{
	/*

	    bit     description

	    PA0
	    PA1
	    PA2
	    PA3
	    PA4
	    PA5     LITE PEN (FIRE)
	    PA6
	    PA7     SERIAL ATN OUT

	*/

	// light pen strobe
	m_user->write_7(BIT(data, 5));
	m_vic->lp_w(BIT(data, 5));

	// serial attention out
	m_user->write_9(!BIT(data, 7));
	m_iec->host_atn_w(!BIT(data, 7));
}

void vic20_state::via1_pb_w(uint8_t data)
{
	m_user->write_c((data>>0)&1);
	m_user->write_d((data>>1)&1);
	m_user->write_e((data>>2)&1);
	m_user->write_f((data>>3)&1);
	m_user->write_h((data>>4)&1);
	m_user->write_j((data>>5)&1);
	m_user->write_k((data>>6)&1);
	m_user->write_l((data>>7)&1);
}

uint8_t vic20_state::via2_pa_r()
{
	/*

	    bit     description

	    PA0     ROW 0
	    PA1     ROW 1
	    PA2     ROW 2
	    PA3     ROW 3
	    PA4     ROW 4
	    PA5     ROW 5
	    PA6     ROW 6
	    PA7     ROW 7

	*/

	uint8_t data = 0xff;

	for (int i = 0; i < 8; i++)
	{
		if (!BIT(m_key_col, i)) data &= m_col[i]->read();
	}

	return data;
}

uint8_t vic20_state::via2_pb_r()
{
	/*

	    bit     description

	    PB0     COL 0
	    PB1     COL 1
	    PB2     COL 2
	    PB3     COL 3
	    PB4     COL 4
	    PB5     COL 5
	    PB6     COL 6
	    PB7     COL 7, JOY 3 (RIGHT)

	*/

	uint8_t data = 0xff;

	// joystick
	uint8_t joy = m_joy->read_joy();

	data &= (BIT(joy, 3) << 7) | 0x7f;

	for (int i = 0; i < 8; i++)
	{
		if (!BIT(m_key_row, i))
			for (int c = 0; c < 8; c++)
				if (!BIT(m_col[c]->read(), i))
					data &= ~(1 << c);
	}

	return data;
}

void vic20_state::via2_pa_w(uint8_t data)
{
	/*

	    bit     description

	    PA0     ROW 0
	    PA1     ROW 1
	    PA2     ROW 2
	    PA3     ROW 3
	    PA4     ROW 4
	    PA5     ROW 5
	    PA6     ROW 6
	    PA7     ROW 7

	*/

	// keyboard row
	m_key_row = data;
}

void vic20_state::via2_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0     COL 0
	    PB1     COL 1
	    PB2     COL 2
	    PB3     COL 3, CASS WRITE
	    PB4     COL 4
	    PB5     COL 5
	    PB6     COL 6
	    PB7     COL 7

	*/

	// cassette write
	m_cassette->write(BIT(data, 3));

	// keyboard column
	m_key_col = data;
}

void vic20_state::via2_ca2_w(int state)
{
	// serial clock out
	m_iec->host_clk_w(!state);
}

void vic20_state::via2_cb2_w(int state)
{
	// serial data out
	m_iec->host_data_w(!state);
}


//-------------------------------------------------
//  VIC20_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

void vic20_state::exp_reset_w(int state)
{
	if (!state)
	{
		machine_reset();
	}
}


//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( vic20 )
//-------------------------------------------------

void vic20_state::machine_start()
{
	// initialize memory
	uint8_t data = 0xff;

	for (offs_t offset = 0; offset < m_ram->size(); offset++)
	{
		m_ram->pointer()[offset] = data;
		if (!(offset % 64)) data ^= 0xff;
	}

	m_key_row = 0xff;
	m_key_col = 0xff;

	// state saving
	save_item(NAME(m_key_row));
	save_item(NAME(m_key_col));
	save_item(NAME(m_light_pen));
	save_item(NAME(m_user_joy0));
	save_item(NAME(m_user_joy1));
	save_item(NAME(m_user_joy2));
	save_item(NAME(m_user_light_pen));
	save_item(NAME(m_user_cassette_switch));
}


void vic20_state::machine_reset()
{
	m_maincpu->reset();

	m_vic->reset();
	m_via1->reset();
	m_via2->reset();

	m_iec->reset();
	m_exp->reset();

	m_user->write_3(0);
	m_user->write_3(1);
}

void vic20_state::write_user_joy0(int state)
{
	m_user_joy0 = state;
}

void vic20_state::write_user_joy1(int state)
{
	m_user_joy1 = state;
}

void vic20_state::write_user_joy2(int state)
{
	m_user_joy2 = state;
}

void vic20_state::write_light_pen(int state)
{
	m_light_pen = state;
	m_vic->lp_w(m_light_pen && m_user_light_pen);
}

void vic20_state::write_user_light_pen(int state)
{
	m_user_light_pen = state;
	m_vic->lp_w(m_light_pen && m_user_light_pen);
}

void vic20_state::write_user_cassette_switch(int state)
{
	m_user_cassette_switch = state;
}

//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

void vic20_state::vic20(machine_config &config, const char* softlist_filter)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(VIC_TAG, FUNC(mos6560_device::screen_update));

	m_vic->set_screen(SCREEN_TAG);
	m_vic->set_addrmap(0, &vic20_state::vic_videoram_map);
	m_vic->set_addrmap(1, &vic20_state::vic_colorram_map);

	m_vic->potx_rd_callback().set(m_joy, FUNC(vcs_control_port_device::read_pot_x));
	m_vic->poty_rd_callback().set(m_joy, FUNC(vcs_control_port_device::read_pot_y));
	m_vic->add_route(ALL_OUTPUTS, "mono", 0.25);

	PET_DATASSETTE_PORT(config, m_cassette, 0);
	cbm_datassette_devices(*m_cassette);
	m_cassette->set_default_option("c1530");
	m_cassette->read_handler().set(m_via2, FUNC(via6522_device::write_ca1));

	cbm_iec_slot_device::add(config, m_iec, "c1541");
	m_iec->srq_callback().set(m_via2, FUNC(via6522_device::write_cb1));

	VCS_CONTROL_PORT(config, m_joy, 0);
	vcs_control_port_devices(*m_joy);
	m_joy->set_default_option("joy");
	m_joy->trigger_wr_callback().set(FUNC(vic20_state::write_light_pen));

	PET_USER_PORT(config, m_user, vic20_user_port_cards, nullptr);
	m_user->p3_handler().set(FUNC(vic20_state::exp_reset_w));
	m_user->p4_handler().set(FUNC(vic20_state::write_user_joy0));
	m_user->p5_handler().set(FUNC(vic20_state::write_user_joy1));
	m_user->p6_handler().set(FUNC(vic20_state::write_user_joy2));
	m_user->p7_handler().set(FUNC(vic20_state::write_user_light_pen));
	m_user->p8_handler().set(FUNC(vic20_state::write_user_cassette_switch));
	m_user->pb_handler().set(m_via1, FUNC(via6522_device::write_cb1));
	m_user->pc_handler().set(m_via1, FUNC(via6522_device::write_pb0));
	m_user->pd_handler().set(m_via1, FUNC(via6522_device::write_pb1));
	m_user->pe_handler().set(m_via1, FUNC(via6522_device::write_pb2));
	m_user->pf_handler().set(m_via1, FUNC(via6522_device::write_pb3));
	m_user->ph_handler().set(m_via1, FUNC(via6522_device::write_pb4));
	m_user->pj_handler().set(m_via1, FUNC(via6522_device::write_pb5));
	m_user->pk_handler().set(m_via1, FUNC(via6522_device::write_pb6));
	m_user->pl_handler().set(m_via1, FUNC(via6522_device::write_pb7));
	m_user->pm_handler().set(m_via1, FUNC(via6522_device::write_cb2));

	QUICKLOAD(config, "quickload", "p00,prg", CBM_QUICKLOAD_DELAY).set_load_callback(FUNC(vic20_state::quickload_vc20));

	SOFTWARE_LIST(config, "cart_list").set_original("vic1001_cart").set_filter(softlist_filter);
	SOFTWARE_LIST(config, "cass_list").set_original("vic1001_cass").set_filter(softlist_filter);
	SOFTWARE_LIST(config, "flop_list").set_original("vic1001_flop").set_filter(softlist_filter);

	RAM(config, m_ram);
	m_ram->set_default_size("5K");
}


void vic20_state::add_clocked_devices(machine_config &config, uint32_t clock)
{
	// basic machine hardware
	M6502(config, m_maincpu, clock);
	m_maincpu->set_addrmap(AS_PROGRAM, &vic20_state::vic20_mem);

	MOS6522(config, m_via1, clock);
	m_via1->readpa_handler().set(FUNC(vic20_state::via1_pa_r));
	m_via1->writepa_handler().set(FUNC(vic20_state::via1_pa_w));
	m_via1->writepb_handler().set(FUNC(vic20_state::via1_pb_w));
	m_via1->cb1_handler().set(m_user, FUNC(pet_user_port_device::write_b));
	m_via1->ca2_handler().set(m_cassette, FUNC(pet_datassette_port_device::motor_w));
	m_via1->cb2_handler().set(m_user, FUNC(pet_user_port_device::write_m));
	m_via1->irq_handler().set_inputline(m_maincpu, M6502_NMI_LINE);

	MOS6522(config, m_via2, clock);
	m_via2->readpa_handler().set(FUNC(vic20_state::via2_pa_r));
	m_via2->readpb_handler().set(FUNC(vic20_state::via2_pb_r));
	m_via2->writepa_handler().set(FUNC(vic20_state::via2_pa_w));
	m_via2->writepb_handler().set(FUNC(vic20_state::via2_pb_w));
	m_via2->ca2_handler().set(FUNC(vic20_state::via2_ca2_w));
	m_via2->cb2_handler().set(FUNC(vic20_state::via2_cb2_w));
	m_via2->irq_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	// video/sound hardware
	SPEAKER(config, "mono").front_center();

	// devices
	VIC20_EXPANSION_SLOT(config, m_exp, clock, vic20_expansion_cards, nullptr);
	m_exp->irq_wr_callback().set_inputline(m_maincpu, M6502_IRQ_LINE);
	m_exp->nmi_wr_callback().set_inputline(m_maincpu, M6502_NMI_LINE);
	m_exp->res_wr_callback().set(FUNC(vic20_state::exp_reset_w));
}


void vic20_state::ntsc(machine_config &config)
{
	add_clocked_devices(config, MOS6560_CLOCK);
	MOS6560(config, m_vic, MOS6560_CLOCK);
	vic20(config, "NTSC");

	m_screen->set_refresh_hz(MOS6560_VRETRACERATE);
	m_screen->set_size((MOS6560_XSIZE + 7) & ~7, MOS6560_YSIZE);
	m_screen->set_visarea(MOS6560_MAME_XPOS, MOS6560_MAME_XPOS + MOS6560_MAME_XSIZE - 1, MOS6560_MAME_YPOS, MOS6560_MAME_YPOS + MOS6560_MAME_YSIZE - 1);
}


void vic20_state::pal(machine_config &config)
{
	add_clocked_devices(config, MOS6561_CLOCK);
	MOS6561(config, m_vic, MOS6561_CLOCK);
	vic20(config, "PAL");

	m_screen->set_refresh_hz(MOS6561_VRETRACERATE);
	m_screen->set_size((MOS6561_XSIZE + 7) & ~7, MOS6561_YSIZE);
	m_screen->set_visarea(MOS6561_MAME_XPOS, MOS6561_MAME_XPOS + MOS6561_MAME_XSIZE - 1, MOS6561_MAME_YPOS, MOS6561_MAME_YPOS + MOS6561_MAME_YSIZE - 1);
}



//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

//-------------------------------------------------
//  ROM( vic1001 )
//-------------------------------------------------

ROM_START( vic1001 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901486-01", 0x0000, 0x2000, CRC(db4c43c1) SHA1(587d1e90950675ab6b12d91248a3f0d640d02e8d) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "901486-02", 0x0000, 0x2000, CRC(336900d7) SHA1(c9ead45e6674d1042ca6199160e8583c23aeac22) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901460-02", 0x0000, 0x1000, CRC(fcfd8a4b) SHA1(dae61ac03065aa2904af5c123ce821855898c555) )
ROM_END


//-------------------------------------------------
//  ROM( vic20 )
//-------------------------------------------------

ROM_START( vic20 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901486-01.ue11", 0x0000, 0x2000, CRC(db4c43c1) SHA1(587d1e90950675ab6b12d91248a3f0d640d02e8d) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_SYSTEM_BIOS( 0, "cbm", "Original" )
	ROMX_LOAD( "901486-06.ue12", 0x0000, 0x2000, CRC(e5e7c174) SHA1(06de7ec017a5e78bd6746d89c2ecebb646efeb19), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "jiffydos", "JiffyDOS" )
	ROMX_LOAD( "jiffydos vic-20 ntsc.ue12", 0x0000, 0x2000, CRC(683a757f) SHA1(83fb83e97b5a840311dbf7e1fe56fe828f41936d), ROM_BIOS(1) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901460-03.ud7", 0x0000, 0x1000, CRC(83e032a6) SHA1(4fd85ab6647ee2ac7ba40f729323f2472d35b9b4) )
ROM_END


//-------------------------------------------------
//  ROM( vic20p )
//-------------------------------------------------

ROM_START( vic20p )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901486-01.ue11", 0x0000, 0x2000, CRC(db4c43c1) SHA1(587d1e90950675ab6b12d91248a3f0d640d02e8d) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_SYSTEM_BIOS( 0, "cbm", "Original" )
	ROMX_LOAD( "901486-07.ue12", 0x0000, 0x2000, CRC(4be07cb4) SHA1(ce0137ed69f003a299f43538fa9eee27898e621e), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "jiffydos", "JiffyDOS" )
	ROMX_LOAD( "jiffydos vic-20 pal.ue12", 0x0000, 0x2000, CRC(705e7810) SHA1(5a03623a4b855531b8bffd756f701306f128be2d), ROM_BIOS(1) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901460-03.ud7", 0x0000, 0x1000, CRC(83e032a6) SHA1(4fd85ab6647ee2ac7ba40f729323f2472d35b9b4) )
ROM_END


//-------------------------------------------------
//  ROM( vic20_se )
//-------------------------------------------------

ROM_START( vic20_se )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901486-01.ue11", 0x0000, 0x2000, CRC(db4c43c1) SHA1(587d1e90950675ab6b12d91248a3f0d640d02e8d) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "nec22081.206", 0x0000, 0x2000, CRC(b2a60662) SHA1(cb3e2f6e661ea7f567977751846ce9ad524651a3) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "nec22101.207", 0x0000, 0x1000, CRC(d808551d) SHA1(f403f0b0ce5922bd61bbd768bdd6f0b38e648c9f) )
ROM_END

} // anonymous namespace


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT   COMPAT  MACHINE  INPUT    CLASS        INIT        COMPANY                        FULLNAME                   FLAGS
COMP( 1980, vic1001,  0,       0,      ntsc,    vic1001, vic20_state, empty_init, "Commodore Business Machines", "VIC-1001 (Japan)",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
COMP( 1981, vic20,    vic1001, 0,      ntsc,    vic20,   vic20_state, empty_init, "Commodore Business Machines", "VIC-20 (NTSC)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
COMP( 1981, vic20p,   vic1001, 0,      pal,     vic20,   vic20_state, empty_init, "Commodore Business Machines", "VIC-20 / VC-20 (PAL)",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
COMP( 1981, vic20_se, vic1001, 0,      pal,     vic20s,  vic20_state, empty_init, "Commodore Business Machines", "VIC-20 (Sweden/Finland)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
