// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************************************

Skeleton driver for Memorex 2178

Chips: Z80A, N8X305N, 2x B8452A, 4x D4016C-3, 2x HD468A50P, HD46505SP-1
Crystal: 18.8696MHz
There is a large piezo-beeper.

TODO:
- Connect up the beeper
- Unknown memory i/o C000, 4000
- Need schematic / tech manual
- Doesn't seem to be any dips, looks like all settings and modes are controlled by keystrokes.
- 8X305 is a 16-bit bipolar processor which appears to use four external PROMs (undumped). It
  would communicate with the Z80 via a common 8-bit I/O bus. No idea what it is used for here,
  but in another system it acts as the floppy disk controller.
- Debug trick: set pc=809 to see the test menu.
- It shows the status line but keystrokes are ignored. After 20 minutes of inactivity, the screen
  goes blank. Pressing a key will restore it.

***************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
//#include "cpu/8x300/8x300.h" // device = N8X300
#include "machine/6850acia.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"

namespace {

class mx2178_state : public driver_device
{
public:
	mx2178_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
	{ }

	void mx2178(machine_config &config);

private:
	MC6845_UPDATE_ROW(crtc_update_row);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	virtual void machine_reset() override ATTR_COLD;
	required_device<palette_device> m_palette;
	required_shared_ptr<u8> m_p_videoram;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
};

void mx2178_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom().region("roms", 0);
	map(0x2000, 0x27ff).ram().share("videoram");
	map(0x6000, 0x6fff).ram();
	map(0xe000, 0xe7ff).ram();
}

void mx2178_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x01, 0x01).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x80, 0x81).rw("acia1", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xa0, 0xa1).rw("acia2", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
}


/* Input ports */
static INPUT_PORTS_START( mx2178 )
INPUT_PORTS_END

MC6845_UPDATE_ROW( mx2178_state::crtc_update_row )
{
	rgb_t const *const pens = m_palette->palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);

	for (uint16_t x = 0; x < x_count; x++)
	{
		uint16_t mem = (ma + x) & 0x7ff;
		uint8_t chr = m_p_videoram[mem];

		/* get pattern of pixels for that character scanline */
		uint8_t gfx = m_p_chargen[(chr<<4) | ra] ^ ((x == cursor_x) ? 0xff : 0);

		/* Display a scanline of a character (8 pixels) */
		*p++ = pens[BIT(gfx, 7)];
		*p++ = pens[BIT(gfx, 6)];
		*p++ = pens[BIT(gfx, 5)];
		*p++ = pens[BIT(gfx, 4)];
		*p++ = pens[BIT(gfx, 3)];
		*p++ = pens[BIT(gfx, 2)];
		*p++ = pens[BIT(gfx, 1)];
		*p++ = pens[BIT(gfx, 0)];
	}
}

/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8 },
	8*16                    /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_mx2178 )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END

void mx2178_state::machine_reset()
{
}

void mx2178_state::mx2178(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(18'869'600) / 5); // guess
	m_maincpu->set_addrmap(AS_PROGRAM, &mx2178_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mx2178_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not correct
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_mx2178);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* Devices */
	mc6845_device &crtc(MC6845(config, "crtc", XTAL(18'869'600) / 8)); // clk unknown
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(mx2178_state::crtc_update_row));
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	clock_device &acia_clock(CLOCK(config, "acia_clock", XTAL(18'869'600) / 30));
	acia_clock.signal_handler().set("acia1", FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append("acia1", FUNC(acia6850_device::write_rxc));
	acia_clock.signal_handler().append("acia2", FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append("acia2", FUNC(acia6850_device::write_rxc));

	acia6850_device &acia1(ACIA6850(config, "acia1", 0));
	acia1.txd_handler().set("rs232a", FUNC(rs232_port_device::write_txd));
	acia1.rts_handler().set("rs232a", FUNC(rs232_port_device::write_rts));
	acia1.irq_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set("acia1", FUNC(acia6850_device::write_rxd));
	rs232a.cts_handler().set("acia1", FUNC(acia6850_device::write_cts));

	acia6850_device &acia2(ACIA6850(config, "acia2", 0));
	acia2.txd_handler().set("rs232b", FUNC(rs232_port_device::write_txd));
	acia2.rts_handler().set("rs232b", FUNC(rs232_port_device::write_rts));
	acia2.irq_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, "keyboard"));
	rs232b.rxd_handler().set("acia2", FUNC(acia6850_device::write_rxd));
	rs232b.cts_handler().set("acia2", FUNC(acia6850_device::write_cts));
}

/* ROM definition */
ROM_START( mx2178 )
	ROM_REGION(0x2000, "roms", 0) // MBM2764-25
	ROM_LOAD( "96274139.u9", 0x000000, 0x002000, CRC(eb471a27) SHA1(433abefd1a72653d0bf35bcaaeccf9943b96260b) )

	ROM_REGION(0x800, "proms", 0) // MB7122E - actual mapping not known
	ROMX_LOAD( "96270350.q2", 0x0000, 0x0400, NO_DUMP, ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )
	ROMX_LOAD( "96270368.r2", 0x0000, 0x0400, NO_DUMP, ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) )
	ROMX_LOAD( "96270376.s2", 0x0001, 0x0400, NO_DUMP, ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )
	ROMX_LOAD( "96270384.t2", 0x0001, 0x0400, NO_DUMP, ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) )

	ROM_REGION(0x1000, "chargen", 0) // D2732A-3
	ROM_LOAD( "96273883.c7", 0x000000, 0x001000, CRC(8311fadd) SHA1(573bbad23e893ad9374edc929642dc1cba3452d2) )
ROM_END

} // Anonymous namespace

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY    FULLNAME        FLAGS
COMP( 1984, mx2178, 0,      0,      mx2178,  mx2178, mx2178_state, empty_init, "Memorex", "Memorex 2178", MACHINE_IS_SKELETON | MACHINE_SUPPORTS_SAVE )
