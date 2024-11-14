// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/********************************************************************************

EC-65 (also known as Octopus)

2009-07-16 Initial driver.

No sound. Storage is floppy disk. No software has been found.
Modules are constructed on separate eurocards and plug into a common backplane.

EC65 -  Disk controller module uses 6821 and 6850. Needs to be emulated.
        When done, .b to boot disk

EC65K - To be developed from scratch. Similar design to EC65, but 6522
        replaced with 6821. Contains extra RTC. Particulars of video and
        universal FDC not known.
      - Extra 256KB card
      - Extra Z80 card with ROM, 2x PIA, 64 or 256KB RAM, for CP/M use.
      - Currently runs into the weeds soon after start, at F070


********************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/g65816/g65816.h"
#include "video/mc6845.h"
#include "machine/6821pia.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "machine/6850acia.h"
#include "machine/keyboard.h"
#include "machine/mc146818.h"
#include "emupal.h"
#include "screen.h"

namespace {

class ec65_common : public driver_device
{
public:
	ec65_common(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_vram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
	{ }

	MC6845_UPDATE_ROW(crtc_update_row);

protected:
	required_shared_ptr<u8> m_vram;
	required_region_ptr<u8> m_p_chargen;
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;

};

class ec65_state : public ec65_common
{
public:
	ec65_state(const machine_config &mconfig, device_type type, const char *tag)
		: ec65_common(mconfig, type, tag)
		, m_via0(*this, "via0")
		, m_via1(*this, "via1")
	{ }

	void ec65(machine_config &config);

private:
	void machine_reset() override ATTR_COLD;
	void ec65_mem(address_map &map) ATTR_COLD;
	void kbd_put(u8 data);
	required_device<via6522_device> m_via0;
	required_device<via6522_device> m_via1;
};

class ec65k_state : public ec65_common
{
public:
	ec65k_state(const machine_config &mconfig, device_type type, const char *tag)
		: ec65_common(mconfig, type, tag)
		, m_rtc(*this, "rtc")
		, m_pia0(*this, "pia0")
		, m_pia1(*this, "pia1")
	{ }

	void ec65k(machine_config &config);

private:
	void ec65k_mem(address_map &map) ATTR_COLD;
	void kbd_put(u8 data);
	required_device<mc146818_device> m_rtc;
	required_device<pia6821_device> m_pia0;
	required_device<pia6821_device> m_pia1;
};

void ec65_state::ec65_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xdfff).ram();
	map(0xe000, 0xe003).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // FDC card
	map(0xe010, 0xe011).rw("fdc", FUNC(acia6850_device::read), FUNC(acia6850_device::write)); // FDC card
	map(0xe100, 0xe10f).m(m_via0, FUNC(via6522_device::map));
	map(0xe110, 0xe11f).m(m_via1, FUNC(via6522_device::map));
	map(0xe130, 0xe133).rw("uart", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0xe140, 0xe140).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0xe141, 0xe141).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xe400, 0xe7ff).ram(); // 1KB on-board RAM
	map(0xe800, 0xefff).ram().share("videoram");
	map(0xf000, 0xffff).rom().region("maincpu",0);
}

void ec65k_state::ec65k_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x00dfff).ram();   // D000-DFFF = System RAM
	map(0x00e000, 0x00e003).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // old FDC card
	map(0x00e010, 0x00e011).rw("fdc", FUNC(acia6850_device::read), FUNC(acia6850_device::write)); // old FDC card
	//map(0x00e150,             colorator CRTC (no info)
	//map(0x00e280,             basicode interface (no info)
	//map(0x00e300,             Z80 board
	map(0x00e400, 0x00e403).rw(m_pia0, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); //  PIA0 porta=keyboard; portb=parallel port
	map(0x00e410, 0x00e413).rw("uart", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x00e420, 0x00e423).rw(m_pia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); //  PIA1 porta=centronics control; portb=centronics data
	map(0x00e430, 0x00e430).w(m_rtc, FUNC(mc146818_device::address_w));  //  RTC 146818 - has battery backup
	map(0x00e431, 0x00e431).rw(m_rtc, FUNC(mc146818_device::data_r), FUNC(mc146818_device::data_w));
	//map(0x00e500, 0x00e5ff)   universal disk controller (no info)
	map(0x00e800, 0x00efff).ram().share("videoram");
	map(0x00f000, 0x00ffff).rom().region("maincpu",0);
	map(0x010000, 0x0fffff).ram();
}

/* Input ports */
static INPUT_PORTS_START( ec65 )
INPUT_PORTS_END

void ec65_state::kbd_put(u8 data)
{
	if (data)
	{
		m_via0->write_pa(data);
		m_via0->write_ca1(1);
		m_via0->write_ca1(0);
	}
}

void ec65k_state::kbd_put(u8 data)
{
	if (data)
	{
		m_pia0->porta_w(data);
		m_pia0->ca1_w(1);
		m_pia0->ca1_w(0);
	}
}

void ec65_state::machine_reset()
{
	m_via1->write_pb(0xff);
}

MC6845_UPDATE_ROW( ec65_common::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32 *p = &bitmap.pix(y);

	for (u16 x = 0; x < x_count; x++)
	{
		u8 const inv = (x == cursor_x) ? 0xff : 0;
		u16 const mem = (ma + x) & 0x7ff;
		u8 const chr = m_vram[mem];

		/* get pattern of pixels for that character scanline */
		u8 const gfx = m_p_chargen[(chr<<4) | (ra & 0x0f)] ^ inv;

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

/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_ec65 )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END

void ec65_state::ec65(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, XTAL(4'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &ec65_state::ec65_mem);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 200);
	screen.set_visarea_full();
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_ec65);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	mc6845_device &crtc(MC6845(config, "crtc", XTAL(16'000'000) / 8));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8); /*?*/
	crtc.set_update_row_callback(FUNC(ec65_state::crtc_update_row));

	/* devices */
	ACIA6850(config, "fdc", 0); // used as a FDC on separate card
	PIA6821(config, "pia"); // assists 6850

	MOS6522(config, m_via0, XTAL(4'000'000) / 4);

	MOS6522(config, m_via1, XTAL(4'000'000) / 4);

	mos6551_device &uart(MOS6551(config, "uart", 0));
	uart.set_xtal(XTAL(1'843'200));

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(ec65_state::kbd_put));
}

void ec65k_state::ec65k(machine_config &config)
{
	/* basic machine hardware */
	g65816_device &maincpu(G65816(config, "maincpu", XTAL(4'000'000))); // can use 4,2 or 1 MHz
	maincpu.set_addrmap(AS_PROGRAM, &ec65k_state::ec65k_mem);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 200);
	screen.set_visarea_full();
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_ec65);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	mc6845_device &crtc(MC6845(config, "crtc", XTAL(16'000'000) / 8));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8); /*?*/
	crtc.set_update_row_callback(FUNC(ec65k_state::crtc_update_row));

	/* devices */
	ACIA6850(config, "fdc", 0); // used as a FDC on separate card
	PIA6821(config, "pia"); // assists 6850

	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	//m_rtc->irq().set(FUNC(micronic_state::mc146818_irq));   Connects to common irq line used by below PIAs and UART

	PIA6821(config, m_pia0);
	PIA6821(config, m_pia1);

	mos6551_device &uart(MOS6551(config, "uart", 0));
	uart.set_xtal(XTAL(1'843'200));

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(ec65k_state::kbd_put));
}

/* ROM definition */
ROM_START( ec65 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ec65.ic6", 0x0000, 0x1000, CRC(acd928ed) SHA1(e02a688a057ff77294717cf7b887425fed0b1153))

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "chargen.ic19", 0x0000, 0x1000, CRC(9b56a28d) SHA1(41c04fd9fb542c50287bc0e366358a61fc4b0cd4)) // Located on VDU card
ROM_END

ROM_START( ec65k )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ec65k.ic19",  0x0000, 0x1000, CRC(5e5a890a) SHA1(daa006f2179fd156833e11c73b37881cafe5dede))

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "chargen.ic19", 0x0000, 0x1000, CRC(9b56a28d) SHA1(41c04fd9fb542c50287bc0e366358a61fc4b0cd4)) // Located on VDU card
ROM_END

} // Anonymous namespace

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY    FULLNAME  FLAGS */
COMP( 1985, ec65,  0,      0,      ec65,    ec65,  ec65_state,  empty_init, "Elektor", "EC-65",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1986, ec65k, ec65,   0,      ec65k,   ec65,  ec65k_state, empty_init, "Elektor", "EC-65K", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
