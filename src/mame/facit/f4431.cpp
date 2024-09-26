// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Facit 4431

    VT100 compatible terminal

    Hardware:
    - Z80
    - TMS9927 CRTC
    - Z80A-DART
    - Z80A-CTC
    - ER1400
    - 6116
    - 6116 x2
    - 2114 x4
    - XTAL 9.828 MHz (B1), 14.976 MHz (B2), 4 MHz (B3)
    - AY-5-1013A UART

    TODO:
    - Character attributes (RAM at 0xc000)
    - Cursor
    - 132 column mode
    - Smooth scrolling
    - Timings
    - Printer
    - Figure out why the EAROM hack is needed
    - Move ergo201 driver here? The hardware is very similar

    Notes:

***************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/ay31015.h"
#include "machine/er1400.h"
#include "machine/ripple_counter.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "video/tms9927.h"
#include "f4431_kbd.h"
#include "emupal.h"
#include "screen.h"

#include "f4431.lh"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class f4431_state : public driver_device
{
public:
	f4431_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_earom(*this, "earom"),
		m_ctc(*this, "ctc"),
		m_dart(*this, "dart"),
		m_vtc(*this, "vtc"),
		m_screen(*this, "screen"),
		m_uart(*this, "uart"),
		m_ascii(*this, "ascii"),
		m_chargen(*this, "chargen"),
		m_switches(*this, "switches"),
		m_display_enabled(false),
		m_nmi_disabled(true)
	{ }

	void f4431(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<er1400_device> m_earom;
	required_device<z80ctc_device> m_ctc;
	required_device<z80dart_device> m_dart;
	required_device<tms9927_device> m_vtc;
	required_device<screen_device> m_screen;
	required_device<ay31015_device> m_uart;
	required_shared_ptr<uint8_t> m_ascii;
	required_region_ptr<uint8_t> m_chargen;
	required_ioport m_switches;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint8_t latch_r();
	void latch_w(uint8_t data);

	void scanline_cb(uint32_t data);
	void vsync_cb(int state);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void row_w(offs_t offset, uint8_t data);
	void brightness_w(uint8_t data);

	uint8_t m_row_address;
	uint8_t m_row_attr;

	bool m_display_enabled;
	bool m_nmi_disabled;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void f4431_state::mem_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x1a00, 0x1a00).nopw(); // spurious write
	map(0x2000, 0x2000).w(FUNC(f4431_state::brightness_w));
	map(0x3000, 0x300f).w(m_vtc, FUNC(tms9927_device::write));
	map(0x4000, 0x4000).w(FUNC(f4431_state::latch_w));
	map(0x5000, 0x5000).w(m_uart, FUNC(ay31015_device::transmit));
	map(0x6000, 0x6000).r(m_uart, FUNC(ay31015_device::receive));
	map(0x6000, 0x6fff).w(FUNC(f4431_state::row_w));
	map(0x6001, 0x6001).r(FUNC(f4431_state::latch_r));
	map(0x7000, 0x77ff).ram(); // scratchpad ram
	map(0x8000, 0x8fff).ram().share(m_ascii);
	map(0xc000, 0xcfff).ram(); // attribute ram
}

void f4431_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xf4, 0xf7).rw(m_dart, FUNC(z80dart_device::cd_ba_r), FUNC(z80dart_device::cd_ba_w));
	map(0xf8, 0xfb).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( f4431 )
	PORT_START("switches")
	PORT_DIPNAME(0x01, 0x01, "Production Test")
	PORT_DIPLOCATION("W:4")
	PORT_DIPSETTING(   0x01, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPNAME(0x02, 0x00, "Enable EAROM Save")
	PORT_DIPLOCATION("W:5")
	PORT_DIPSETTING(   0x02, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void f4431_state::scanline_cb(uint32_t data)
{
	// in the actual system this would be generated
	// by the r0 and r3 output of the vtc

	if (!m_nmi_disabled && ((data % 10) == 9))
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
}

void f4431_state::vsync_cb(int state)
{
	if (!m_nmi_disabled && state)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
}

uint32_t f4431_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_display_enabled && BIT(m_row_attr, 6))
	{
		for (int i = cliprect.min_y; i <= cliprect.max_y; i++)
		{
			int line = i % 10;

			// double height, bottom
			if ((m_row_attr & 0x30) == 0x10)
				line = line / 2 + 5;

			// double height, top
			if ((m_row_attr & 0x30) == 0x20)
				line = line / 2;

			if (BIT(m_row_attr, 4) || BIT(m_row_attr, 5) )
			{
				// double width
				for (int x = 0; x < 40; x++)
				{
					uint8_t code = m_ascii[(m_row_address << 4) + x * 2];
					uint8_t data = m_chargen[(line << 7) | code];

					bitmap.pix(i, x * 20 + 0) = BIT(data, 7) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 1) = BIT(data, 7) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 2) = (BIT(data, 7) || BIT(data, 6)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 3) = (BIT(data, 7) || BIT(data, 6)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 4) = (BIT(data, 6) || BIT(data, 5)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 5) = (BIT(data, 6) || BIT(data, 5)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 6) = (BIT(data, 5) || BIT(data, 4)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 7) = (BIT(data, 5) || BIT(data, 4)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 8) = (BIT(data, 4) || BIT(data, 3)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 9) = (BIT(data, 4) || BIT(data, 3)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 10) = (BIT(data, 3) || BIT(data, 2)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 11) = (BIT(data, 3) || BIT(data, 2)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 12) = (BIT(data, 2) || BIT(data, 1)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 13) = (BIT(data, 2) || BIT(data, 1)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 14) = (BIT(data, 1) || BIT(data, 0)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 15) = (BIT(data, 1) || BIT(data, 0)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 16) = BIT(data, 0) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 17) = BIT(data, 0) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 20 + 18) = rgb_t::black();
					bitmap.pix(i, x * 20 + 19) = rgb_t::black();
				}
			}
			else
			{
				for (int x = 0; x < 80; x++)
				{
					uint8_t code = m_ascii[(m_row_address << 4) + x];
					uint8_t data = m_chargen[(line << 7) | code];

					bitmap.pix(i, x * 10 + 0) = BIT(data, 7) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 10 + 1) = (BIT(data, 7) || BIT(data, 6)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 10 + 2) = (BIT(data, 6) || BIT(data, 5)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 10 + 3) = (BIT(data, 5) || BIT(data, 4)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 10 + 4) = (BIT(data, 4) || BIT(data, 3)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 10 + 5) = (BIT(data, 3) || BIT(data, 2)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 10 + 6) = (BIT(data, 2) || BIT(data, 1)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 10 + 7) = (BIT(data, 1) || BIT(data, 0)) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 10 + 8) = BIT(data, 0) ? rgb_t::white() : rgb_t::black();
					bitmap.pix(i, x * 10 + 9) = rgb_t::black();
				}
			}
		}
	}
	else
	{
		bitmap.fill(rgb_t::black(), cliprect);
	}

	return 0;
}

void f4431_state::row_w(offs_t offset, uint8_t data)
{
	if (0)
		logerror("row_w: %02x %02x line %d %d\n", offset, data, m_screen->vpos(), m_screen->hpos());

	m_row_address = offset >> 4;
	m_row_attr = data;
}

void f4431_state::brightness_w(uint8_t data)
{
	if (0)
		logerror("brightness_w: %02x\n", data);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

uint8_t f4431_state::latch_r()
{
	// 7-------  vsync
	// -6------  not connected
	// --5-----  earom data
	// ---4----  not connected
	// ----3---  switch w5 (disable earom save)
	// -----2--  switch w4 (production test)
	// ------1-  uart dav
	// -------0  uart tbe/boc

	uint8_t data = 0;

	data |= (m_uart->tbmt_r() | m_uart->eoc_r()) << 0;
	data |= m_uart->dav_r() << 1;
	data |= m_switches->read() << 2;
	data |= m_earom->data_r() << 5;
	data |= (m_screen->vblank() ? 0 : 1) << 7;

	return data;
}

void f4431_state::latch_w(uint8_t data)
{
	// 7-------  80/132 columns
	// -6------  interrupt disable
	// --5-----  screen blank
	// ---4----  earom clock
	// ----3---  earom c2
	// -----2--  earom c3
	// ------1-  earom data
	// -------0  earom c1

	m_earom->c1_w(BIT(data, 0));
	m_earom->data_w(BIT(data, 1));
	m_earom->c3_w(BIT(data, 2));
	m_earom->c2_w(BIT(data, 3));
	m_earom->clock_w(BIT(data, 4));

	m_display_enabled = bool(BIT(data, 5));
	m_nmi_disabled = bool(BIT(data, 6));
}

void f4431_state::machine_start()
{
	// set uart control lines
	m_uart->write_cs(1);
	m_uart->write_np(1);
	m_uart->write_tsb(1);
	m_uart->write_nb2(1);
	m_uart->write_nb1(1);
	m_uart->write_eps(1);
	m_uart->write_swe(0);

	// register for save states
	save_item(NAME(m_row_address));
	save_item(NAME(m_row_attr));
	save_item(NAME(m_display_enabled));
	save_item(NAME(m_nmi_disabled));
}

void f4431_state::machine_reset()
{
	m_display_enabled = false;
	m_nmi_disabled = true;
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

static const z80_daisy_config daisy_chain[] =
{
	{ "dart" },
	{ "ctc" },
	{ nullptr }
};

void f4431_state::f4431(machine_config &config)
{
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &f4431_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &f4431_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	ER1400(config, m_earom);

	Z80CTC(config, m_ctc, 4_MHz_XTAL);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->set_clk<0>(4_MHz_XTAL / 13);
	m_ctc->zc_callback<0>().set(m_dart, FUNC(z80dart_device::rxca_w));
	m_ctc->set_clk<1>(4_MHz_XTAL / 13);
	m_ctc->zc_callback<1>().set(m_dart, FUNC(z80dart_device::txca_w));
	m_ctc->set_clk<2>(4_MHz_XTAL / 13);
	m_ctc->zc_callback<2>().set(m_dart, FUNC(z80dart_device::rxtxcb_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_raw(9.828_MHz_XTAL * 2, 1020, 0, 800, 268, 0, 250); // probably wrong
	m_screen->set_screen_update(FUNC(f4431_state::screen_update));
	m_screen->scanline().set(FUNC(f4431_state::scanline_cb));

	PALETTE(config, "palette", palette_device::MONOCHROME);

	TMS9927(config, m_vtc, 9.828_MHz_XTAL / 6); // ?
	m_vtc->set_char_width(10); // renders in half-dots?
	m_vtc->set_screen("screen");
	m_vtc->hsyn_callback().set(m_ctc, FUNC(z80ctc_device::trg3));
	m_vtc->vsyn_callback().set(FUNC(f4431_state::vsync_cb));

	Z80DART(config, m_dart, 4_MHz_XTAL);
	m_dart->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_dart->out_txda_callback().set("comm", FUNC(rs232_port_device::write_txd));
	m_dart->out_rtsa_callback().set("comm", FUNC(rs232_port_device::write_rts));
	// port b: printer

	rs232_port_device &porta(RS232_PORT(config, "comm", default_rs232_devices, nullptr));
	porta.rxd_handler().set(m_dart, FUNC(z80dart_device::rxa_w));
	porta.cts_handler().set(m_dart, FUNC(z80dart_device::ctsa_w));

	AY31015(config, m_uart);
	m_uart->set_auto_rdav(true);
	m_uart->write_so_callback().set("kbd", FUNC(f4431_kbd_device::rx_w));

	ripple_counter_device &uart_clk(RIPPLE_COUNTER(config, "uart_clk", 4_MHz_XTAL / 13));
	uart_clk.set_stages(12);
	uart_clk.count_out_cb().set(m_uart, FUNC(ay31015_device::write_rcp)).bit(4); // Q4
	uart_clk.count_out_cb().append(m_uart, FUNC(ay31015_device::write_tcp)).bit(4); // Q4

	f4431_kbd_device &kbd(F4431_KBD(config, "kbd"));
	kbd.tx_handler().set(m_uart, FUNC(ay31015_device::write_si));

	config.set_default_layout(layout_f4431);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( f4431 )
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD("11420050-00_4431.d73", 0x0000, 0x1000, CRC(e7a9c982) SHA1(60c2eb769bd6051b4acaf38750ef40b9d02c568c))
	ROM_COPY("maincpu", 0x0000, 0x1000, 0x1000)
	ROM_LOAD("11420060-00_4431.d74", 0x2000, 0x1000, CRC(e9703cf0) SHA1(9eef6d4f0b57a6430a034f9405d4d8279723e2a0))
	ROM_COPY("maincpu", 0x2000, 0x3000, 0x1000)
	ROM_LOAD("11420070-00_4431.d75", 0x4000, 0x1000, CRC(a0d5b59e) SHA1(fca5b25163d942ec688d82004253850a3e08b0af))
	ROM_COPY("maincpu", 0x4000, 0x5000, 0x1000)

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD("11419840-00_cg30.d40", 0x000, 0x800, CRC(d2988792) SHA1(bc3b59882c351fb503371e358e396e7d683c9467))
	// National variants (not dumped):
	// 1141 89 50-00/0 UK
	// 1141 98 60-00/9 FRA
	// 1141 90 70-00/8 GER
	// 1141 98 80-00/7 NOR
	// 1141 98 90-00/6 SWE/FIN
	// 1141 99 00-00/3 SPA
	// 1141 99 10-00/2 DEN

	ROM_REGION(0x800, "attr", 0)
	ROM_LOAD("11419950-00_4431.d57", 0x000, 0x800, CRC(066dc6bd) SHA1(1ad17c9ec96544278d9f8494c19b4b3bce8e3a8e))

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD("11419960-00_4431.d19", 0x00, 0x20, CRC(daae0c28) SHA1(58c55b8b9d4161a9d38259a4375cf19799ea0b7a))

	// factory default settings
	ROM_REGION16_LE(200, "earom", 0)
	ROM_LOAD("earom.d63", 0, 200, CRC(f14db754) SHA1(904e26974fbe7fe9166b731850bf414d8ffbe75d))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME  FLAGS
COMP( 1981, f4431, 0,       0,     f4431,   f4431, f4431_state, empty_init, "Facit", "4431",   MACHINE_NOT_WORKING )
