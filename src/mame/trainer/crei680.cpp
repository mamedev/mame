// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************************

CREI680 trainer.

This has 2 boards, the CPU board with the buttons, LEDs etc,
and the video board.

Like many machines of this era, the cassette circuit is grossly overcomplicated,
using a 3301 op-amp, a 75140 line receiver and a bunch of CMOS logic ICs. Since
the first of these (a 14040B) divides the E clock down to 300 * 16 to generate
the ACIA's TXC input, the format can be assumed to be Kansas City.

The schematic is missing some information, so we've guessed a few things.
We don't have any instructions, user manual or anything - just the schematic.

TODO:
- Cassette to test when we find out how to use it.
- Single-step circuit
- Add paste facility, although it will be difficult with having to set each
   individual bit of each byte. If the byte already is non-zero, it won't
   know what to do.


**********************************************************************************/


#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "imagedev/cassette.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/timer.h"
#include "video/mc6845.h"
#include "sound/beep.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "crei680.lh"


namespace {

class crei680_state : public driver_device
{
public:
	crei680_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_pia0(*this, "pia0")
		, m_pia1(*this, "pia1")
		, m_uart(*this, "uart")
		, m_crtc(*this, "crtc")
		, m_vram(*this, "videoram")
		, m_cgen(*this, "chargen")
		, m_palette(*this, "palette")
		, m_beep(*this, "beeper")
		, m_leds(*this, "led%u", 0U)
		{ }


	void crei680(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

private:
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_w);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	MC6845_UPDATE_ROW(crtc_update_row);

	void mem_map(address_map &map) ATTR_COLD;

	void pia0b_w(u8);
	void pia1b_w(u8);
	void piaset_w(offs_t, u8);
	bool m_cassbit = 0;
	bool m_cassold = 0;
	u8 m_cass_data[4]{};
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<pia6821_device> m_pia0;
	required_device<pia6821_device> m_pia1;
	required_device<acia6850_device> m_uart;
	required_device<mc6845_device> m_crtc;
	required_shared_ptr<u8> m_vram;
	required_region_ptr<u8> m_cgen;
	required_device<palette_device> m_palette;
	required_device<beep_device> m_beep;
	output_finder<8> m_leds;
};


/* Memory Maps */

void crei680_state::mem_map(address_map &map)
{
	map(0x0080, 0x017f).ram();    // mc6810
	map(0x3000, 0x3fff).ram();    // 8x 2114
	map(0xc004, 0xc007).rw(m_pia0, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc008, 0xc009).rw(m_uart, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xc010, 0xc010).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0xc011, 0xc011).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xc020, 0xc023).rw(m_pia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc024, 0xc027).w(FUNC(crei680_state::piaset_w));
	map(0xd000, 0xd3ff).ram().share("videoram");    // 6x 2102
	map(0xe000, 0xefff).rom().region("basic", 0);   // 68A332
	map(0xf000, 0xffff).rom().region("maincpu", 0);  // 68A332
}

/* Input Ports */

static INPUT_PORTS_START( crei680 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_NAME("bit0")    // toggle bit0 of data
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_NAME("bit1")    // ...etc...
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("bit2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("bit3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("bit4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("bit5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("bit6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_NAME("bit7")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_NAME("REG")   // Examine or set register
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_NAME("LAD")   // Set low byte of address
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_NAME("HAD")   // Set high byte of address
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_NAME("RUN")    // Execute
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("DEC")   // Previous address or register
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INC")    // Next address or register
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("DATA")   // Switch between address and data modes
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RST") PORT_CODE(KEYCODE_LALT) PORT_CHANGED_MEMBER(DEVICE_SELF, crei680_state, reset_button, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(crei680_state::reset_button)
{
	if (newval)
	{
		m_pia0->reset();
		m_pia1->reset();
	}
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}

/* Video */
/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 8,                  /* 8 x 8 characters */
	128,                    /* 64 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_crei680 )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END

MC6845_UPDATE_ROW( crei680_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32 *p = &bitmap.pix(y);

	for (u16 x = 0; x < x_count; x++)
	{
		u16 mem = (ma + x) & 0x3ff;
		u8 chr = m_vram[mem] & 0x3f;  // video ram is 6 bit.
		if (chr < 0x20)
			chr |= 0x40;
		u8 gfx = (ra < 8) ? m_cgen[(chr<<3) | ra] : 0;

		/* Display a scanline of a character (6 pixels) */
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(crei680_state::kansas_w)
{
	m_cass_data[3]++;

	if (m_cassbit != m_cassold)
	{
		m_cass_data[3] = 0;
		m_cassold = m_cassbit;
	}

	if (m_cassbit)
		m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz
}

TIMER_DEVICE_CALLBACK_MEMBER(crei680_state::kansas_r)
{
	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	u8 cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_uart->write_rxd((m_cass_data[1] < 12) ? 1 : 0);
		m_cass_data[1] = 0;
	}
}

void crei680_state::pia0b_w(u8 data)
{
	/* d0/1 = both low for beep
	   d4 = cassette relay
	   others are mystery circuits */
	m_beep->set_state(BIT(data, 0, 2) ? 0 : 1);
	m_cass->change_state((BIT(data,4)) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

void crei680_state::pia1b_w(u8 data)
{
	for (u8 i = 0; i < 8; i++)
		m_leds[i] = !BIT(data, i);
}

// The various devices in the cxxx range are selected by an address line. This means things clash
// if 2 or more are selected at the same time. Normally this could be ignored, except that the
// bootup sequence writes to the 2 PIAs at the same time in one particular instruction.
void crei680_state::piaset_w(offs_t offset, u8 data)
{
	m_pia0->write(offset, data);
	m_pia1->write(offset, data);
}

/* Machine Initialization */

void crei680_state::machine_start()
{
	m_leds.resolve();
	save_item(NAME(m_cassold));
	save_item(NAME(m_cass_data));
	save_item(NAME(m_cassbit));
}

void crei680_state::machine_reset()
{
	m_beep->set_state(0);
	m_cass_data[0] = 0;
	m_cass_data[1] = 0;
	m_cass_data[3] = 0;
}

/* Machine Drivers */

void crei680_state::crei680(machine_config &config)
{
	/* basic machine hardware */
	M6802(config, m_maincpu, 2.4576_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &crei680_state::mem_map);

	/* video hardware */
	config.set_default_layout(layout_crei680);
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	PALETTE(config, m_palette, palette_device::MONOCHROME);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_crei680);

	MC6845(config, m_crtc, 5281920/6);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(6);
	m_crtc->set_update_row_callback(FUNC(crei680_state::crtc_update_row));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 900).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	PIA6821(config, m_pia0);
	m_pia0->readpa_handler().set_ioport("X1");
	m_pia0->writepb_handler().set(FUNC(crei680_state::pia0b_w));
	//m_pia0->cb2_handler().set(FUNC(crei680_state::screen_w));
	m_pia0->irqa_handler().set_inputline("maincpu", M6800_IRQ_LINE);
	m_pia0->irqb_handler().set_inputline("maincpu", M6800_IRQ_LINE);

	PIA6821(config, m_pia1);
	m_pia1->readpa_handler().set_ioport("X0");
	m_pia1->writepb_handler().set(FUNC(crei680_state::pia1b_w));
	//m_pia1->cb2_handler().set(FUNC(crei680_state::screen_w));
	m_pia1->irqa_handler().set_inputline("maincpu", M6800_IRQ_LINE);
	m_pia1->irqb_handler().set_inputline("maincpu", M6800_IRQ_LINE);

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	ACIA6850(config, m_uart, 0);
	m_uart->txd_handler().set([this] (bool state) { m_cassbit = state; });
	//m_uart->rts_handler().set(FUNC(crei680_state::acia_rts_w));
	m_uart->irq_handler().set_inputline("maincpu", M6800_IRQ_LINE);

	clock_device &acia_clock(CLOCK(config, "acia_clock", 2.4576_MHz_XTAL / 4 / 128));
	acia_clock.signal_handler().set("uart", FUNC(acia6850_device::write_txc)); // connected directly
	acia_clock.signal_handler().append("uart", FUNC(acia6850_device::write_rxc)); // FIXME: modulated by RXD

	TIMER(config, "kansas_w").configure_periodic(FUNC(crei680_state::kansas_w), attotime::from_hz(4800));
	TIMER(config, "kansas_r").configure_periodic(FUNC(crei680_state::kansas_r), attotime::from_hz(40000));
}

/* ROMs */

ROM_START( crei680 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "monitor.u19", 0x0000, 0x1000, CRC(0f95918e) SHA1(8cd675cc3a7b38ccd0975ef44f3e954d21711e0a) )  // cpu board

	ROM_REGION( 0x1000, "basic", 0 )
	ROM_LOAD( "basic.u13",   0x0000, 0x1000, CRC(339d5557) SHA1(74763e6416a124dda5c18e6cfa71c4c6848c0ac9) )  // video board

	ROM_REGION( 0x0400, "chargen", 0 )
	ROM_LOAD( "mcm6674p.u9", 0x0000, 0x0400, CRC(1c22088a) SHA1(b5f0bd0cfdec0cd5c1cb764506bef3c17d6af0eb) )  // video board
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  COMPAT  MACHINE    INPUT    CLASS          INIT          COMPANY                               FULLNAME      FLAGS
COMP( 19??, crei680, 0,      0,      crei680,   crei680, crei680_state, empty_init, "Capital Radio Engineering Institute", "CREI 680", MACHINE_SUPPORTS_SAVE )
