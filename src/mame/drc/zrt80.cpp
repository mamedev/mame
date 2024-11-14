// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        DEC ZRT-80

        12/05/2009 Skeleton driver.

        16/02/2011 Working.

        The beeper is external, frequency not known. I've made a reasonable
        assumption of frequency and lengths.

        Make sure 'mode' dipswitch is set to 'local' so you can see your
        typing.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "machine/ins8250.h"
#include "machine/keyboard.h"
#include "sound/beep.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class zrt80_state : public driver_device
{
public:
	zrt80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_8250(*this, "ins8250")
		, m_beep(*this, "beeper")
		, m_palette(*this, "palette")
		, m_p_chargen(*this, "chargen")
		, m_beep_timer(*this, "beep_timer")
	{ }

	void zrt80(machine_config &config);

private:
	uint8_t zrt80_10_r();
	void zrt80_30_w(uint8_t data);
	void zrt80_38_w(uint8_t data);
	void kbd_put(u8 data);
	MC6845_UPDATE_ROW(crtc_update_row);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(beep_timer);
	uint8_t m_term_data = 0U;
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<ins8250_device> m_8250;
	required_device<beep_device> m_beep;
	required_device<palette_device> m_palette;
	required_region_ptr<u8> m_p_chargen;
	required_device<timer_device> m_beep_timer;
};


uint8_t zrt80_state::zrt80_10_r()
{
	uint8_t ret = m_term_data;
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return ret;
}

TIMER_DEVICE_CALLBACK_MEMBER(zrt80_state::beep_timer)
{
	m_beep->set_state(0);
}


void zrt80_state::zrt80_30_w(uint8_t data)
{
	m_beep_timer->adjust(attotime::from_msec(100));
	m_beep->set_state(1);
}

void zrt80_state::zrt80_38_w(uint8_t data)
{
	m_beep_timer->adjust(attotime::from_msec(400));
	m_beep->set_state(1);
}

void zrt80_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom(); // Z25 - Main firmware
	map(0x1000, 0x1fff).rom(); // Z24 - Expansion
	map(0x4000, 0x43ff).ram(); // Board RAM
	// Normally video RAM is 0x800 but could be expanded up to 8K
	map(0xc000, 0xdfff).ram().share("videoram"); // Video RAM

}

void zrt80_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x07).rw(m_8250, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	map(0x08, 0x08).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x09, 0x09).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x10, 0x17).r(FUNC(zrt80_state::zrt80_10_r));
	map(0x18, 0x1F).portr("DIPSW2");
	map(0x20, 0x27).portr("DIPSW3");
	map(0x30, 0x37).w(FUNC(zrt80_state::zrt80_30_w));
	map(0x38, 0x3F).w(FUNC(zrt80_state::zrt80_38_w));
}

/* Input ports */
static INPUT_PORTS_START( zrt80 )
	PORT_START("DIPSW1")
		PORT_DIPNAME( 0x01, 0x01, "Composite Sync" )
		PORT_DIPSETTING(    0x01, "Negative" )
		PORT_DIPSETTING(    0x00, "Positive" )
		PORT_DIPNAME( 0x02, 0x02, "Vertical Sync" )
		PORT_DIPSETTING(    0x02, "Negative" )
		PORT_DIPSETTING(    0x00, "Positive" )
		PORT_DIPNAME( 0x04, 0x00, "Video" )
		PORT_DIPSETTING(    0x04, "Negative" )
		PORT_DIPSETTING(    0x00, "Positive" )
		PORT_DIPNAME( 0x08, 0x08, "Keypad" )
		PORT_DIPSETTING(    0x08, "Numeric" )
		PORT_DIPSETTING(    0x00, "Alternate Keyboard" )
		PORT_DIPNAME( 0x10, 0x10, "Horizontal Sync" )
		PORT_DIPSETTING(    0x10, "Negative" )
		PORT_DIPSETTING(    0x00, "Positive" )
		PORT_DIPNAME( 0x20, 0x20, "CPU" )
		PORT_DIPSETTING(    0x20, "Operating" )
		PORT_DIPSETTING(    0x00, "Reset" )
		PORT_DIPNAME( 0x40, 0x40, "Keyboard strobe" )
		PORT_DIPSETTING(    0x40, "Negative" )
		PORT_DIPSETTING(    0x00, "Positive" )
		PORT_DIPNAME( 0x80, 0x00, "Beeper" )
		PORT_DIPSETTING(    0x80, "Silent" )
		PORT_DIPSETTING(    0x00, "Enable" )
	PORT_START("DIPSW2")
		PORT_DIPNAME( 0x0f, 0x05, "Baud rate" )
		PORT_DIPSETTING(    0x00, "50" )
		PORT_DIPSETTING(    0x01, "75" )
		PORT_DIPSETTING(    0x02, "110" )
		PORT_DIPSETTING(    0x03, "134.5" )
		PORT_DIPSETTING(    0x04, "150" )
		PORT_DIPSETTING(    0x05, "300" )
		PORT_DIPSETTING(    0x06, "600" )
		PORT_DIPSETTING(    0x07, "1200" )
		PORT_DIPSETTING(    0x08, "1800" )
		PORT_DIPSETTING(    0x09, "2000" )
		PORT_DIPSETTING(    0x0a, "2400" )
		PORT_DIPSETTING(    0x0b, "3600" )
		PORT_DIPSETTING(    0x0c, "4800" )
		PORT_DIPSETTING(    0x0d, "7200" )
		PORT_DIPSETTING(    0x0e, "9600" )
		PORT_DIPSETTING(    0x0f, "19200" )
		PORT_DIPNAME( 0x30, 0x20, "Parity" )
		PORT_DIPSETTING(    0x00, "Odd" )
		PORT_DIPSETTING(    0x10, "Even" )
		PORT_DIPSETTING(    0x20, "Marking" )
		PORT_DIPSETTING(    0x30, "Spacing" )
		PORT_DIPNAME( 0x40, 0x40, "Handshake" )
		PORT_DIPSETTING(    0x40, "CTS" )
		PORT_DIPSETTING(    0x00, "XON/XOFF" )
		PORT_DIPNAME( 0x80, 0x80, "Line Feed" )
		PORT_DIPSETTING(    0x80, "No LF on CR" )
		PORT_DIPSETTING(    0x00, "Auto" )
	PORT_START("DIPSW3")
		PORT_DIPNAME( 0x07, 0x07, "Display" )
		PORT_DIPSETTING(    0x00, "96 x 24 15750Hz, 50Hz" )
		PORT_DIPSETTING(    0x01, "80 x 48 15750Hz, 50Hz" )
		PORT_DIPSETTING(    0x02, "80 x 24 15750Hz, 50Hz" )
		PORT_DIPSETTING(    0x03, "96 x 24 15750Hz, 60Hz" )
		PORT_DIPSETTING(    0x04, "80 x 48 18700Hz, 50Hz" )
		PORT_DIPSETTING(    0x05, "80 x 24 17540Hz, 60Hz" )
		PORT_DIPSETTING(    0x06, "80 x 48 15750Hz, 60Hz" )
		PORT_DIPSETTING(    0x07, "80 x 24 15750Hz, 60Hz" )
		PORT_DIPNAME( 0x18, 0x18, "Emulation" )
		PORT_DIPSETTING(    0x00, "Adds" )
		PORT_DIPSETTING(    0x08, "Beehive" )
		PORT_DIPSETTING(    0x10, "LSI ADM-3" )
		PORT_DIPSETTING(    0x18, "Heath H-19" )
		PORT_DIPNAME( 0x20, 0x00, "Mode" )
		PORT_DIPSETTING(    0x20, "Line" )
		PORT_DIPSETTING(    0x00, "Local" )
		PORT_DIPNAME( 0x40, 0x40, "Duplex" )
		PORT_DIPSETTING(    0x40, "Full" )
		PORT_DIPSETTING(    0x00, "Half" )
		PORT_DIPNAME( 0x80, 0x80, "Wraparound" )
		PORT_DIPSETTING(    0x80, "Disabled" )
		PORT_DIPSETTING(    0x00, "Enabled" )
INPUT_PORTS_END


void zrt80_state::machine_start()
{
	save_item(NAME(m_term_data));
}

void zrt80_state::machine_reset()
{
	m_term_data = 0;
}

MC6845_UPDATE_ROW( zrt80_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);
	uint8_t const polarity = ioport("DIPSW1")->read() & 4 ? 0xff : 0;

	for (uint16_t x = 0; x < x_count; x++)
	{
		uint8_t inv = polarity;
		if (x == cursor_x) inv ^= 0xff;
		uint16_t const mem = (ma + x) & 0x1fff;
		uint8_t chr = m_p_videoram[mem];

		if (BIT(chr, 7))
		{
			inv ^= 0xff;
			chr &= 0x7f;
		}

		uint8_t const gfx = m_p_chargen[(chr<<4) | ra] ^ inv;

		/* Display a scanline of a character */
		*p++ = palette[BIT(gfx, 7)];
		*p++ = palette[BIT(gfx, 6)];
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
	}
}

void zrt80_state::kbd_put(u8 data)
{
	m_term_data = data;
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

/* F4 Character Displayer */
static const gfx_layout zrt80_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_zrt80 )
	GFXDECODE_ENTRY( "chargen", 0x0000, zrt80_charlayout, 0, 1 )
GFXDECODE_END

void zrt80_state::zrt80(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(2'457'600));
	m_maincpu->set_addrmap(AS_PROGRAM, &zrt80_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &zrt80_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 200);
	screen.set_visarea(0, 640-1, 0, 200-1);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_zrt80);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 800).add_route(ALL_OUTPUTS, "mono", 0.50);
	TIMER(config, m_beep_timer).configure_generic(FUNC(zrt80_state::beep_timer));

	/* Devices */
	MC6845(config, m_crtc, XTAL(20'000'000) / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8); /*?*/
	m_crtc->set_update_row_callback(FUNC(zrt80_state::crtc_update_row));

	INS8250(config, m_8250, 2457600);
	m_8250->out_int_callback().set_inputline("maincpu", INPUT_LINE_IRQ0);

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(zrt80_state::kbd_put));
}

/* ROM definition */
ROM_START( zrt80 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("zrt80mon.z25", 0x0000, 0x1000, CRC(e6ea96dc) SHA1(e3075e30bb2b85f9288d0b8b8cdf1d2b4f7586fd) )
	//z24 is 2nd chip, used as expansion

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD("zrt80chr.z30", 0x0000, 0x0800, CRC(4dbdc60f) SHA1(20e393f7207a8440029c8290cdf2f121d317a37e) )
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                       FULLNAME   FLAGS */
COMP( 1982, zrt80, 0,      0,      zrt80,   zrt80, zrt80_state, empty_init, "Digital Research Computers", "ZRT-80",  MACHINE_SUPPORTS_SAVE )
