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
#include "pofo_kbd.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/pcd3311.h"
#include "video/hd61830.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

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
		m_keyboard(*this, "keyboard"),
		m_dtmf(*this, PCD3311T_TAG),
		m_ccm(*this, PORTFOLIO_MEMORY_CARD_SLOT_A_TAG),
		m_exp(*this, "exp"),
		m_timer_tick(*this, TIMER_TICK_TAG),
		m_nvram(*this, "nvram"),
		m_ram(*this, "ram"),
		m_rom(*this, M80C88A_TAG),
		m_char_rom(*this, HD61830_TAG),
		m_battery(*this, "BATTERY")
	{ }

	void portfolio(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void portfolio_io(address_map &map) ATTR_COLD;
	void portfolio_lcdc(address_map &map) ATTR_COLD;
	void portfolio_mem(address_map &map) ATTR_COLD;

	void check_interrupt();
	void trigger_interrupt(int level);

	enum
	{
		INT_TICK = 0,
		INT_KEYBOARD,
		INT_ERROR,
		INT_EXTERNAL
	};

	enum
	{
		ROM_APP = 0b000, // 0
		CCM_A   = 0b011, // 3
		CCM_B   = 0b111, // 7
		ROM_EXT = 0b010, // 2
	};

	uint8_t mem_r(offs_t offset);
	void mem_w(offs_t offset, uint8_t data);

	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);

	uint8_t irq_status_r();
	uint8_t battery_r();
	uint8_t counter_r(offs_t offset);

	void irq_mask_w(uint8_t data);
	void dtmf_w(uint8_t data);
	void power_w(uint8_t data);
	void select_w(uint8_t data);
	void counter_w(offs_t offset, uint8_t data);
	void contrast_w(uint8_t data);

	void eint_w(int state);
	void wake_w(int state);
	void keyboard_int_w(int state);

	void portfolio_palette(palette_device &palette) const;
	TIMER_DEVICE_CALLBACK_MEMBER(system_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(counter_tick);
	uint8_t hd61830_rd_r(offs_t offset);
	IRQ_CALLBACK_MEMBER(portfolio_int_ack);

	required_device<cpu_device> m_maincpu;
	required_device<hd61830_device> m_lcdc;
	required_device<pofo_keyboard_device> m_keyboard;
	required_device<pcd3311_device> m_dtmf;
	required_device<portfolio_memory_card_slot_device> m_ccm;
	required_device<portfolio_expansion_slot_device> m_exp;
	required_device<timer_device> m_timer_tick;
	required_device<nvram_device> m_nvram;
	required_device<ram_device> m_ram;
	required_region_ptr<uint8_t> m_rom;
	required_region_ptr<uint8_t> m_char_rom;
	required_ioport m_battery;

	uint8_t m_ip = 0;
	uint8_t m_ie = 0;
	uint16_t m_counter = 0;
	int m_rom_b = 0;
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

void portfolio_state::eint_w(int state)
{
	if (state)
	{
		trigger_interrupt(INT_EXTERNAL);
	}
}


//-------------------------------------------------
//  wake_w - wake
//-------------------------------------------------

void portfolio_state::wake_w(int state)
{
	// TODO
}

void portfolio_state::keyboard_int_w(int state)
{
	if (state)
	{
		trigger_interrupt(INT_KEYBOARD);
	}
}

//-------------------------------------------------
//  irq_status_r - interrupt status read
//-------------------------------------------------

uint8_t portfolio_state::irq_status_r()
{
	uint8_t data = m_ip;
	/*
	    The BIOS interrupt 11h (Equipment list) reports that the second floppy drive (B:) is
	    installed if the 3rd bit is set (which is also the external interrupt line).
	    It is not clear if the ~NMD1 line is OR or XORed or muxed with the interrupt line,
	    but this way seems to work.
	*/
	data |= !m_exp->nmd1_r() << 3;

	return data;
}

//-------------------------------------------------
//  irq_mask_w - interrupt enable mask
//-------------------------------------------------

void portfolio_state::irq_mask_w(uint8_t data)
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
//  SOUND
//**************************************************************************

//-------------------------------------------------
//  dtmf_w -
//-------------------------------------------------

void portfolio_state::dtmf_w(uint8_t data)
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
	m_dtmf->write(data & 0x3f);
	m_dtmf->strobe_w(BIT(data, 6));
}



//**************************************************************************
//  POWER MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  power_w - power management
//-------------------------------------------------

void portfolio_state::power_w(uint8_t data)
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

uint8_t portfolio_state::battery_r()
{
	/*

	    bit     signal      description

	    0       ?           bit 0 from bus select (m_rom_b)
	    1       ?
	    2       ?           bit 2 from bus select (m_rom_b)
	    3       ?
	    4       ?
	    5       PDET        1=peripheral connected
	    6       LOWB        0=battery low
	    7       BDET?       1=cold boot

	*/

	uint8_t data = 0;

	/*
	    Partially stores what has been written into this port.
	    Used by interrupt 61h service 24h (Get ROM/CCM state).
	    Setting bit 1 here causes the BIOS to permanently wedge the external ROM
	    select on, so mask it out as a workaround.
	*/
	data |= (m_rom_b & 0b101);

	// peripheral detect
	data |= m_exp->pdet_r() << 5;

	// battery status
	data |= (m_battery->read() & 0x03) << 6;

	return data;
}


//-------------------------------------------------
//  select_w -
//-------------------------------------------------

void portfolio_state::select_w(uint8_t data)
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

	m_rom_b = data & 0x0f;
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

uint8_t portfolio_state::counter_r(offs_t offset)
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

void portfolio_state::counter_w(offs_t offset, uint8_t data)
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

uint8_t portfolio_state::mem_r(offs_t offset)
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

			data = m_ccm->nrdi_r(offset & 0x1ffff);
			break;

		case CCM_B:
			ncc1 = 0;
			break;

		case ROM_EXT:
			// TODO
			break;

		default:
			logerror("%s %s Invalid bus read %05x\n", machine().time().as_string(), machine().describe_context(), offset & 0x1ffff);
			break;
		}
	}
	else if (offset >= 0xe0000)
	{
		data = m_rom[offset & 0x3ffff];
	}

	data = m_exp->nrdi_r(offset, data, iom, bcom, ncc1);

	return data;
}


//-------------------------------------------------
//  mem_w -
//-------------------------------------------------

void portfolio_state::mem_w(offs_t offset, uint8_t data)
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

			m_ccm->nwri_w(offset & 0x1ffff, data);
			break;

		case CCM_B:
			ncc1 = 0;
			break;

		case ROM_EXT:
		case ROM_APP:
			break;

		default:
			logerror("%s %s Invalid bus write %05x\n", machine().time().as_string(), machine().describe_context(), offset & 0x1ffff);
			break;
		}
	}

	m_exp->nwri_w(offset, data, iom, bcom, ncc1);
}


//-------------------------------------------------
//  io_r -
//-------------------------------------------------

uint8_t portfolio_state::io_r(offs_t offset)
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
			data = m_keyboard->read();
			break;

		case 1:
			if (offset & 0x01)
			{
				data = m_lcdc->status_r();
			}
			else
			{
				data = m_lcdc->data_r();
			}
			break;

		case 4:
			data = counter_r(offset & 0x01);
			break;

		case 5:
			if (offset & 0x01)
			{
				data = battery_r();
			}
			else
			{
				data = irq_status_r();
			}
			break;

		case 7:
			bcom = 0;
			break;
		}
	}
	else if (offset == 0x61)
	{
		// Magic port to detect the Pofo
		data = 0x61;
	}

	data = m_exp->nrdi_r(offset, data, iom, bcom, ncc1);

	return data;
}


//-------------------------------------------------
//  io_w -
//-------------------------------------------------

void portfolio_state::io_w(offs_t offset, uint8_t data)
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
				m_lcdc->control_w(data);
			}
			else
			{
				m_lcdc->data_w(data);
			}
			break;

		case 2:
			dtmf_w(data);
			break;

		case 3:
			power_w(data);
			break;

		case 4:
			counter_w(offset & 0x01, data);
			break;

		case 5:
			if (offset & 0x01)
			{
				select_w(data);
			}
			else
			{
				irq_mask_w(data);
			}
			break;

		case 6:
			contrast_w(data);
			break;

		case 7:
			bcom = 0;
			break;
		}
	}

	m_exp->nwri_w(offset, data, iom, bcom, ncc1);
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

void portfolio_state::contrast_w(uint8_t data)
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

uint8_t portfolio_state::hd61830_rd_r(offs_t offset)
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
	save_item(NAME(m_rom_b));

	m_ip = 0;
}

//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  machine_config( portfolio )
//-------------------------------------------------

void portfolio_state::portfolio(machine_config &config)
{
	// basic machine hardware
	I8088(config, m_maincpu, XTAL(4'915'200));
	m_maincpu->set_addrmap(AS_PROGRAM, &portfolio_state::portfolio_mem);
	m_maincpu->set_addrmap(AS_IO, &portfolio_state::portfolio_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(portfolio_state::portfolio_int_ack));

	// video hardware
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_LCD));
	screen.set_refresh_hz(72);
	screen.set_screen_update(HD61830_TAG, FUNC(hd61830_device::screen_update));
	screen.set_size(240, 64);
	screen.set_visarea(0, 240-1, 0, 64-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(portfolio_state::portfolio_palette), 2);

	GFXDECODE(config, "gfxdecode", "palette", gfx_portfolio);

	HD61830(config, m_lcdc, XTAL(4'915'200)/2/2);
	m_lcdc->set_addrmap(0, &portfolio_state::portfolio_lcdc);
	m_lcdc->rd_rd_callback().set(FUNC(portfolio_state::hd61830_rd_r));
	m_lcdc->set_screen(SCREEN_TAG);

	POFO_KEYBOARD(config, m_keyboard);
	m_keyboard->int_handler().set(FUNC(portfolio_state::keyboard_int_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	PCD3311(config, m_dtmf, XTAL(3'578'640)).add_route(ALL_OUTPUTS, "mono", 0.25);

	// devices
	PORTFOLIO_MEMORY_CARD_SLOT(config, m_ccm, portfolio_memory_cards, nullptr);

	PORTFOLIO_EXPANSION_SLOT(config, m_exp, XTAL(4'915'200), portfolio_expansion_cards, nullptr);
	m_exp->eint_wr_callback().set(FUNC(portfolio_state::eint_w));
	m_exp->nmio_wr_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_exp->wake_wr_callback().set(FUNC(portfolio_state::wake_w));

	TIMER(config, "counter").configure_periodic(FUNC(portfolio_state::counter_tick), attotime::from_hz(XTAL(32'768)/16384));
	TIMER(config, TIMER_TICK_TAG).configure_periodic(FUNC(portfolio_state::system_tick), attotime::from_hz(XTAL(32'768)/32768));

	// software list
	SOFTWARE_LIST(config, "cart_list").set_original("pofo");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("128K");

	NVRAM(config, "nvram", nvram_device::DEFAULT_RANDOM);
}



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

} // Anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME  PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY  FULLNAME     FLAGS
COMP( 1989, pofo, 0,      0,      portfolio, portfolio, portfolio_state, empty_init, "Atari", "Portfolio", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
