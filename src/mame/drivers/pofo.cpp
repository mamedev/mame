// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Atari Portfolio

    http://portfolio.wz.cz/
    http://www.pofowiki.de/doku.php
    http://www.best-electronics-ca.com/portfoli.htm
    http://www.atari-portfolio.co.uk/pfnews/pf9.txt

    Command line for dual RAM expansion with A: File Manager ROM card and B: RAM card
    ./mess64 pofo -exp ram -exp:ram:exp ram2 -cart1 fileman -exp:ram:ccmb ram

*/

/*

    TODO:

    - cursor is missing
    - where do CDET and NMD1 connect to ??
    - i/o port 8051
    - screen contrast
    - system tick frequency selection (1 or 128 Hz)
    - soft power off
    - LCD board
        - HD61830A00
        - 5816 2Kx8 RAM
        - 27C256 32Kx8 EPROM
        - PCD3311T DTMF generator @ 3.578640MHz

*/

#include "emu.h"

#include "cpu/i86/i86.h"
#include "bus/pofo/ccm.h"
#include "bus/pofo/exp.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/pcd3311.h"
#include "video/hd61830.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define M80C88A_TAG     "u1"
#define HD61830_TAG     "hd61830"
#define PCD3311T_TAG    "pcd3311t"
#define TIMER_TICK_TAG  "tick"
#define SCREEN_TAG      "screen"

static const uint8_t INTERRUPT_VECTOR[] = { 0x08, 0x09, 0x00 };



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class portfolio_state : public driver_device
{
public:
	portfolio_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, M80C88A_TAG),
		m_lcdc(*this, HD61830_TAG),
		m_dtmf(*this, PCD3311T_TAG),
		m_ccm(*this, PORTFOLIO_MEMORY_CARD_SLOT_A_TAG),
		m_exp(*this, PORTFOLIO_EXPANSION_SLOT_TAG),
		m_timer_tick(*this, TIMER_TICK_TAG),
		m_nvram(*this, "nvram"),
		m_ram(*this, "ram"),
		m_rom(*this, M80C88A_TAG),
		m_char_rom(*this, HD61830_TAG),
		m_y(*this, "Y%01u", 0),
		m_battery(*this, "BATTERY"),
		m_keylatch(0xff)
	{ }

	void portfolio(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void portfolio_io(address_map &map);
	void portfolio_lcdc(address_map &map);
	void portfolio_mem(address_map &map);

	void check_interrupt();
	void trigger_interrupt(int level);
	void scan_keyboard();

	enum
	{
		INT_TICK = 0,
		INT_KEYBOARD,
		INT_ERROR,
		INT_EXTERNAL
	};

	enum
	{
		ROM_APP,
		CCM_A,
		CCM_B,
		ROM_EXT
	};

	DECLARE_READ8_MEMBER( mem_r );
	DECLARE_WRITE8_MEMBER( mem_w );

	DECLARE_READ8_MEMBER( io_r );
	DECLARE_WRITE8_MEMBER( io_w );

	DECLARE_READ8_MEMBER( irq_status_r );
	DECLARE_READ8_MEMBER( keyboard_r );
	DECLARE_READ8_MEMBER( battery_r );
	DECLARE_READ8_MEMBER( counter_r );

	DECLARE_WRITE8_MEMBER( irq_mask_w );
	DECLARE_WRITE8_MEMBER( dtmf_w );
	DECLARE_WRITE8_MEMBER( power_w );
	DECLARE_WRITE8_MEMBER( select_w );
	DECLARE_WRITE8_MEMBER( counter_w );
	DECLARE_WRITE8_MEMBER( contrast_w );

	DECLARE_WRITE_LINE_MEMBER( eint_w );
	DECLARE_WRITE_LINE_MEMBER( wake_w );

	void portfolio_palette(palette_device &palette) const;
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(system_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(counter_tick);
	DECLARE_READ8_MEMBER(hd61830_rd_r);
	IRQ_CALLBACK_MEMBER(portfolio_int_ack);

	required_device<cpu_device> m_maincpu;
	required_device<hd61830_device> m_lcdc;
	required_device<pcd3311_device> m_dtmf;
	required_device<portfolio_memory_card_slot_device> m_ccm;
	required_device<portfolio_expansion_slot_device> m_exp;
	required_device<timer_device> m_timer_tick;
	required_device<nvram_device> m_nvram;
	required_device<ram_device> m_ram;
	required_region_ptr<uint8_t> m_rom;
	required_region_ptr<uint8_t> m_char_rom;
	required_ioport_array<8> m_y;
	required_ioport m_battery;

	uint8_t m_ip;
	uint8_t m_ie;
	uint16_t m_counter;
	uint8_t m_keylatch;
	int m_rom_b;
};



//**************************************************************************
//  INTERRUPTS
//**************************************************************************

//-------------------------------------------------
//  check_interrupt - check interrupt status
//-------------------------------------------------

void portfolio_state::check_interrupt()
{
	int level = (m_ip & m_ie) ? ASSERT_LINE : CLEAR_LINE;

	m_maincpu->set_input_line(INPUT_LINE_INT0, level);
	m_exp->iint_w(level);
}


//-------------------------------------------------
//  trigger_interrupt - trigger interrupt request
//-------------------------------------------------

void portfolio_state::trigger_interrupt(int level)
{
	// set interrupt pending bit
	m_ip |= 1 << level;

	check_interrupt();
}


//-------------------------------------------------
//  eint_w - external interrupt
//-------------------------------------------------

WRITE_LINE_MEMBER( portfolio_state::eint_w )
{
	if (state)
	{
		trigger_interrupt(INT_EXTERNAL);
	}
}


//-------------------------------------------------
//  wake_w - wake
//-------------------------------------------------

WRITE_LINE_MEMBER( portfolio_state::wake_w )
{
	// TODO
}


//-------------------------------------------------
//  irq_status_r - interrupt status read
//-------------------------------------------------

READ8_MEMBER( portfolio_state::irq_status_r )
{
	return m_ip;
}


//-------------------------------------------------
//  irq_mask_w - interrupt enable mask
//-------------------------------------------------

WRITE8_MEMBER( portfolio_state::irq_mask_w )
{
	m_ie = data;

	if (LOG) logerror("%s %s IE %01x\n", machine().time().as_string(), machine().describe_context(), data);

	check_interrupt();
}


//-------------------------------------------------
//  IRQ_CALLBACK_MEMBER( portfolio_int_ack )
//-------------------------------------------------

IRQ_CALLBACK_MEMBER(portfolio_state::portfolio_int_ack)
{
	uint8_t vector = 0;

	for (int i = 0; i < 4; i++)
	{
		if (BIT(m_ip, i))
		{
			// clear interrupt pending bit
			m_ip &= ~(1 << i);

			if (LOG) logerror("%s %s IP %01x\n", machine().time().as_string(), machine().describe_context(), m_ip);

			if (i == 3)
				vector = m_exp->eack_r();
			else
				vector = INTERRUPT_VECTOR[i];

			break;
		}
	}

	check_interrupt();

	return vector;
}



//**************************************************************************
//  KEYBOARD
//**************************************************************************

//-------------------------------------------------
//  scan_keyboard - scan keyboard
//-------------------------------------------------

void portfolio_state::scan_keyboard()
{
	uint8_t keycode = 0xff;

	for (int row = 0; row < 8; row++)
	{
		uint8_t data = static_cast<int>(m_y[row]->read());

		if (data != 0xff)
		{
			for (int col = 0; col < 8; col++)
			{
				if (!BIT(data, col))
				{
					keycode = (row * 8) + col;
				}
			}
		}
	}

	if (keycode != 0xff)
	{
		// key pressed
		if (keycode != m_keylatch)
		{
			m_keylatch = keycode;

			trigger_interrupt(INT_KEYBOARD);
		}
	}
	else
	{
		// key released
		if (!(m_keylatch & 0x80))
		{
			m_keylatch |= 0x80;

			trigger_interrupt(INT_KEYBOARD);
		}
	}
}


//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK_MEMBER( keyboard_tick )
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER(portfolio_state::keyboard_tick)
{
	scan_keyboard();
}


//-------------------------------------------------
//  keyboard_r - keyboard scan code register
//-------------------------------------------------

READ8_MEMBER( portfolio_state::keyboard_r )
{
	return m_keylatch;
}



//**************************************************************************
//  SOUND
//**************************************************************************

//-------------------------------------------------
//  dtmf_w -
//-------------------------------------------------

WRITE8_MEMBER( portfolio_state::dtmf_w )
{
	/*

	    bit     description

	    0       PCD3311T D0
	    1       PCD3311T D1
	    2       PCD3311T D2
	    3       PCD3311T D3
	    4       PCD3311T D4
	    5       PCD3311T D5
	    6       PCD3311T STROBE
	    7       PCD3311T VDD,MODE,A0

	*/

	if (LOG) logerror("%s %s DTMF %02x\n", machine().time().as_string(), machine().describe_context(), data);

	m_dtmf->mode_w(!BIT(data, 7));
	m_dtmf->a0_w(!BIT(data, 7));
	m_dtmf->write(space, 0, data & 0x3f);
	m_dtmf->strobe_w(BIT(data, 6));
}



//**************************************************************************
//  POWER MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  power_w - power management
//-------------------------------------------------

WRITE8_MEMBER( portfolio_state::power_w )
{
	/*

	    bit     description

	    0
	    1       1=power off
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	if (LOG) logerror("%s %s POWER %02x\n", machine().time().as_string(), machine().describe_context(), data);

	if (BIT(data, 1))
	{
		// TODO power off
	}
}


//-------------------------------------------------
//  battery_r - battery status
//-------------------------------------------------

READ8_MEMBER( portfolio_state::battery_r )
{
	/*

	    bit     signal      description

	    0       ?           1=boots from B:
	    1       ?           1=boots from external ROM
	    2       ?           1=boots from B:
	    3       ?           1=boots from ???
	    4       ?
	    5       PDET        1=peripheral connected
	    6       LOWB        0=battery low
	    7       BDET?       1=cold boot

	*/

	uint8_t data = 0;

	// peripheral detect
	data |= m_exp->pdet_r() << 5;

	// battery status
	data |= (m_battery->read() & 0x03) << 6;

	return data;
}


//-------------------------------------------------
//  select_w -
//-------------------------------------------------

WRITE8_MEMBER( portfolio_state::select_w )
{
	/*

	    bit     description

	    0       ?
	    1       ?
	    2       ?
	    3       ?
	    4
	    5
	    6       ?
	    7       ?

	*/

	if (LOG) logerror("%s %s SELECT %02x\n", machine().time().as_string(), machine().describe_context(), data);

	switch (data & 0x0f)
	{
	case 3: m_rom_b = CCM_A; break;
	case 7: m_rom_b = CCM_B; break;
	case 0: m_rom_b = ROM_APP; break;
	case 2: m_rom_b = ROM_EXT; break;
	}
}



//**************************************************************************
//  SYSTEM TIMERS
//**************************************************************************

//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK_MEMBER( system_tick )
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER(portfolio_state::system_tick)
{
	//trigger_interrupt(INT_TICK);
}


//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK_MEMBER( counter_tick )
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER(portfolio_state::counter_tick)
{
	m_counter++;
}


//-------------------------------------------------
//  counter_r - counter register read
//-------------------------------------------------

READ8_MEMBER( portfolio_state::counter_r )
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_counter & 0xff;
		break;

	case 1:
		data = m_counter >> 8;
		break;
	}

	return data;
}


//-------------------------------------------------
//  counter_w - counter register write
//-------------------------------------------------

WRITE8_MEMBER( portfolio_state::counter_w )
{
	switch (offset)
	{
	case 0:
		m_counter = (m_counter & 0xff00) | data;
		break;

	case 1:
		m_counter = (data << 8) | (m_counter & 0xff);
		break;
	}
}



//**************************************************************************
//  MEMORY MAPPING
//**************************************************************************

//-------------------------------------------------
//  mem_r -
//-------------------------------------------------

READ8_MEMBER( portfolio_state::mem_r )
{
	uint8_t data = 0;

	int iom = 0;
	int bcom = 1;
	int ncc1 = 1;

	if (offset < 0x1f000)
	{
		data = m_ram->read(offset);
	}
	else if (offset >= 0xb0000 && offset < 0xc0000)
	{
		data = m_ram->read(0x1f000 + (offset & 0xfff));
	}
	else if (offset >= 0xc0000 && offset < 0xe0000)
	{
		switch (m_rom_b)
		{
		case ROM_APP:
			data = m_rom[offset & 0x3ffff];
			break;

		case CCM_A:
			if (LOG) logerror("%s %s CCM0 read %05x\n", machine().time().as_string(), machine().describe_context(), offset & 0x1ffff);

			data = m_ccm->nrdi_r(space, offset & 0x1ffff);
			break;

		case CCM_B:
			ncc1 = 0;
			break;

		case ROM_EXT:
			// TODO
			break;
		}
	}
	else if (offset >= 0xe0000)
	{
		data = m_rom[offset & 0x3ffff];
	}

	data = m_exp->nrdi_r(space, offset, data, iom, bcom, ncc1);

	return data;
}


//-------------------------------------------------
//  mem_w -
//-------------------------------------------------

WRITE8_MEMBER( portfolio_state::mem_w )
{
	int iom = 0;
	int bcom = 1;
	int ncc1 = 1;

	if (offset < 0x1f000)
	{
		m_ram->write(offset, data);
	}
	else if (offset >= 0xb0000 && offset < 0xc0000)
	{
		m_ram->write(0x1f000 + (offset & 0xfff), data);
	}
	else if (offset >= 0xc0000 && offset < 0xe0000)
	{
		switch (m_rom_b)
		{
		case CCM_A:
			if (LOG) logerror("%s %s CCM0 write %05x:%02x\n", machine().time().as_string(), machine().describe_context(), offset & 0x1ffff, data);

			m_ccm->nwri_w(space, offset & 0x1ffff, data);
			break;

		case CCM_B:
			ncc1 = 0;
			break;
		}
	}

	m_exp->nwri_w(space, offset, data, iom, bcom, ncc1);
}


//-------------------------------------------------
//  io_r -
//-------------------------------------------------

READ8_MEMBER( portfolio_state::io_r )
{
	uint8_t data = 0;

	int iom = 1;
	int bcom = 1;
	int ncc1 = 0;

	if ((offset & 0xff00) == 0x8000)
	{
		switch ((offset >> 4) & 0x0f)
		{
		case 0:
			data = keyboard_r(space, 0);
			break;

		case 1:
			if (offset & 0x01)
			{
				data = m_lcdc->status_r(space, 0);
			}
			else
			{
				data = m_lcdc->data_r(space, 0);
			}
			break;

		case 4:
			data = counter_r(space, offset & 0x01);
			break;

		case 5:
			if (offset & 0x01)
			{
				data = battery_r(space, 0);
			}
			else
			{
				data = irq_status_r(space, 0);
			}
			break;

		case 7:
			bcom = 0;
			break;
		}
	}

	data = m_exp->nrdi_r(space, offset, data, iom, bcom, ncc1);

	return data;
}


//-------------------------------------------------
//  io_w -
//-------------------------------------------------

WRITE8_MEMBER( portfolio_state::io_w )
{
	int iom = 1;
	int bcom = 1;
	int ncc1 = 0;

	if ((offset & 0xff00) == 0x8000)
	{
		switch ((offset >> 4) & 0x0f)
		{
		case 1:
			if (offset & 0x01)
			{
				m_lcdc->control_w(space, 0, data);
			}
			else
			{
				m_lcdc->data_w(space, 0, data);
			}
			break;

		case 2:
			dtmf_w(space, 0, data);
			break;

		case 3:
			power_w(space, 0, data);
			break;

		case 4:
			counter_w(space, offset & 0x01, data);
			break;

		case 5:
			if (offset & 0x01)
			{
				select_w(space, 0, data);
			}
			else
			{
				irq_mask_w(space, 0, data);
			}
			break;

		case 6:
			contrast_w(space, 0, data);
			break;

		case 7:
			bcom = 0;
			break;
		}
	}

	m_exp->nwri_w(space, offset, data, iom, bcom, ncc1);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( portfolio_mem )
//-------------------------------------------------

void portfolio_state::portfolio_mem(address_map &map)
{
	map(0x00000, 0xfffff).rw(FUNC(portfolio_state::mem_r), FUNC(portfolio_state::mem_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( portfolio_io )
//-------------------------------------------------

void portfolio_state::portfolio_io(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(portfolio_state::io_r), FUNC(portfolio_state::io_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( portfolio_lcdc )
//-------------------------------------------------

void portfolio_state::portfolio_lcdc(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x0000, 0x07ff).ram();
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( portfolio )
//-------------------------------------------------

static INPUT_PORTS_START( portfolio )
	PORT_START("Y0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Atari") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')

	PORT_START("Y1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del Ins") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')

	PORT_START("Y2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')

	PORT_START("Y3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('"') PORT_CHAR('`')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("Y4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')

	PORT_START("Y5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('8')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')

	PORT_START("Y6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Fn") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("Y7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("BATTERY")
	PORT_CONFNAME( 0x01, 0x01, "Battery Status" )
	PORT_CONFSETTING( 0x01, DEF_STR( Normal ) )
	PORT_CONFSETTING( 0x00, "Low Battery" )
	PORT_CONFNAME( 0x02, 0x00, "Boot" )
	PORT_CONFSETTING( 0x02, "Cold" )
	PORT_CONFSETTING( 0x00, "Warm" )
INPUT_PORTS_END



//**************************************************************************
//  VIDEO
//**************************************************************************

//-------------------------------------------------
//  contrast_w -
//-------------------------------------------------

WRITE8_MEMBER( portfolio_state::contrast_w )
{
	if (LOG) logerror("%s %s CONTRAST %02x\n", machine().time().as_string(), machine().describe_context(), data);
}


//-------------------------------------------------
//  PALETTE_INIT( portfolio )
//-------------------------------------------------

void portfolio_state::portfolio_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(142, 193, 172));
	palette.set_pen_color(1, rgb_t(67, 71, 151));
}


//-------------------------------------------------
//  HD61830_INTERFACE( lcdc_intf )
//-------------------------------------------------

READ8_MEMBER( portfolio_state::hd61830_rd_r )
{
	offs_t address = ((offset & 0xff) << 4) | ((offset >> 12) & 0x0f);
	uint8_t data = m_char_rom[address];

	return data;
}


//-------------------------------------------------
//  gfx_layout charlayout
//-------------------------------------------------

static const gfx_layout charlayout =
{
	6, 8,
	256,
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2 },
	{ STEP8(0,8) },
	8*8
};


//-------------------------------------------------
//  GFXDECODE( portfolio )
//-------------------------------------------------

static GFXDECODE_START( gfx_portfolio )
	GFXDECODE_ENTRY( HD61830_TAG, 0, charlayout, 0, 2 )
GFXDECODE_END



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void portfolio_state::machine_start()
{
	m_nvram->set_base(m_ram->pointer(), m_ram->size());

	// state saving
	save_item(NAME(m_ip));
	save_item(NAME(m_ie));
	save_item(NAME(m_counter));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_rom_b));
}


//-------------------------------------------------
//  machine_reset
//-------------------------------------------------

void portfolio_state::machine_reset()
{
	m_lcdc->reset();

	m_exp->reset();
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( portfolio )
//-------------------------------------------------

MACHINE_CONFIG_START(portfolio_state::portfolio)
	// basic machine hardware
	MCFG_DEVICE_ADD(M80C88A_TAG, I8088, XTAL(4'915'200))
	MCFG_DEVICE_PROGRAM_MAP(portfolio_mem)
	MCFG_DEVICE_IO_MAP(portfolio_io)
	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DRIVER(portfolio_state,portfolio_int_ack)

	// video hardware
	MCFG_SCREEN_ADD(SCREEN_TAG, LCD)
	MCFG_SCREEN_REFRESH_RATE(72)
	MCFG_SCREEN_UPDATE_DEVICE(HD61830_TAG, hd61830_device, screen_update)
	MCFG_SCREEN_SIZE(240, 64)
	MCFG_SCREEN_VISIBLE_AREA(0, 240-1, 0, 64-1)
	MCFG_SCREEN_PALETTE("palette")

	PALETTE(config, "palette", FUNC(portfolio_state::portfolio_palette), 2);

	GFXDECODE(config, "gfxdecode", "palette", gfx_portfolio);

	HD61830(config, m_lcdc, XTAL(4'915'200)/2/2);
	m_lcdc->set_addrmap(0, &portfolio_state::portfolio_lcdc);
	m_lcdc->rd_rd_callback().set(FUNC(portfolio_state::hd61830_rd_r));
	m_lcdc->set_screen(SCREEN_TAG);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD(PCD3311T_TAG, PCD3311, XTAL(3'578'640))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	PORTFOLIO_MEMORY_CARD_SLOT(config, m_ccm, portfolio_memory_cards, nullptr);

	PORTFOLIO_EXPANSION_SLOT(config, m_exp, XTAL(4'915'200), portfolio_expansion_cards, nullptr);
	m_exp->eint_wr_callback().set(FUNC(portfolio_state::eint_w));
	m_exp->nmio_wr_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_exp->wake_wr_callback().set(FUNC(portfolio_state::wake_w));

	TIMER(config, "counter").configure_periodic(FUNC(portfolio_state::counter_tick), attotime::from_hz(XTAL(32'768)/16384));
	TIMER(config, TIMER_TICK_TAG).configure_periodic(FUNC(portfolio_state::system_tick), attotime::from_hz(XTAL(32'768)/32768));

	// fake keyboard
	TIMER(config, "keyboard").configure_periodic(FUNC(portfolio_state::keyboard_tick), attotime::from_usec(2500));

	// software list
	SOFTWARE_LIST(config, "cart_list").set_original("pofo");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("128K");

	NVRAM(config, "nvram", nvram_device::DEFAULT_RANDOM);
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( pofo )
//-------------------------------------------------

ROM_START( pofo )
	ROM_REGION( 0x40000, M80C88A_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "dip1072", "DIP DOS 1.072" )
	ROMX_LOAD( "c101782-007.u4", 0x00000, 0x20000, CRC(c9852766) SHA1(c74430281bc717bd36fd9b5baec1cc0f4489fe82), ROM_BIOS(0) )
	ROMX_LOAD( "c101781-007.u3", 0x20000, 0x20000, CRC(b8fb730d) SHA1(1b9d82b824cab830256d34912a643a7d048cd401), ROM_BIOS(0) )

	ROM_REGION( 0x8000, HD61830_TAG, 0 )
	ROM_LOAD( "c101783-001a-01.u3", 0x0000, 0x8000, CRC(61fdaff1) SHA1(5eb99e7a19af7b8d77ea8a2f1f554e6e3d382fa2) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME  PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY  FULLNAME     FLAGS
COMP( 1989, pofo, 0,      0,      portfolio, portfolio, portfolio_state, empty_init, "Atari", "Portfolio", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
