// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic , Robbbert
/*************************************************************************************

Robotron Z9001 (KC85/1)

2009-05-12 Skeleton driver.
2011-07-13 Notes added. You can enter text via terminal input.
           Colour and flashing added.
2019-06-13 Basic enabled

All input should be in UPPER case.

For KC87_10/11/20/21, type BASIC to start Basic.

The only other kind of acceptable input is a filename that is in 8.3 format and
begins with a letter. It will say 'start tape'. You can press ^C here to
escape, or any key to continue.

Some other control keys:
^B clear input line
^C break
^F toggle flashing attribute
^H backspace
^L clear screen


ToDo:
- cassette in - interrupt-driven via PIO1
    via astb should cause interrupt but nothing happens.
- proper keyboard - interrupt-driven via PIO2
    pressing any key should program ctc/2 to a debounce delay and this then causes
    another interrupt which reads keyboard and places ascii character at 0x0025.
- get rid of temporary code

**************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "imagedev/cassette.h"
#include "machine/timer.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "sound/beep.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

// temporary
#include "machine/keyboard.h"

class z9001_state : public driver_device
{
public:
	z9001_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_framecnt(0)
		, m_maincpu(*this, "maincpu")
		, m_beeper(*this, "beeper")
		, m_cass(*this, "cassette")
		, m_p_colorram(*this, "colorram")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
	{
	}

	void z9001(machine_config &config);

private:
	void kbd_put(u8 data);
	DECLARE_WRITE8_MEMBER(port88_w);
	DECLARE_WRITE_LINE_MEMBER(cass_w);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_callback);
	uint32_t screen_update_z9001(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void z9001_io(address_map &map);
	void z9001_mem(address_map &map);

	uint8_t m_framecnt;
	bool m_cassbit;
	virtual void machine_reset() override;
	//virtual void machine_start();
	required_device<z80_device> m_maincpu;
	required_device<beep_device> m_beeper;
	required_device<cassette_image_device> m_cass;
	required_shared_ptr<uint8_t> m_p_colorram;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
};

void z9001_state::z9001_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xe7ff).ram();
	map(0xe800, 0xebff).ram().share("colorram");
	map(0xc000, 0xe7ff).rom();
	map(0xec00, 0xefff).ram().share("videoram");
	map(0xf000, 0xffff).rom();
}

void z9001_state::z9001_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x83).mirror(4).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x88, 0x8B).mirror(4).rw("pio1", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x90, 0x93).mirror(4).rw("pio2", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}

/* Input ports */
static INPUT_PORTS_START( z9001 )
INPUT_PORTS_END

static const z80_daisy_config z9001_daisy_chain[] =
{
	{ "pio2" },
	{ "pio1" },
	{ "ctc" },
	{ nullptr }
};

//Bits0,1 not connected; 2,3,4,5 go to a connector; 6 goes to 'graphics' LED; 7 goes to speaker.
WRITE8_MEMBER( z9001_state::port88_w )
{
	m_beeper->set_state(BIT(data, 7));
}

WRITE_LINE_MEMBER( z9001_state::cass_w )
{
	if (state)
	{
		m_cassbit ^= 1;
		m_cass->output( m_cassbit ? -1.0 : +1.0);
	}
}


// temporary (prevent freezing when you type an invalid filename)
TIMER_DEVICE_CALLBACK_MEMBER(z9001_state::timer_callback)
{
	m_maincpu->space(AS_PROGRAM).write_byte(0x006a, 0);
}

void z9001_state::machine_reset()
{
	m_maincpu->set_state_int(Z80_PC, 0xf000);
}

uint32_t z9001_state::screen_update_z9001(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,chr,gfx,col,fg,bg;
	uint16_t sy=0,ma=0,x;
	m_framecnt++;

	for(y = 0; y < 24; y++ )
	{
		for (ra = 0; ra < 8; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 40; x++)
			{
				chr = m_p_videoram[x]; // get char in videoram
				gfx = m_p_chargen[(chr<<3) | ra]; // get dot pattern in chargen
				col = m_p_colorram[x];
				fg = col>>4;
				bg = col&15;

				/* Check for flashing - swap bg & fg */
				if ((BIT(col, 7)) && (m_framecnt & 0x10))
				{
					bg = fg;
					fg = col&15;
				}

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7) ? fg : bg;
				*p++ = BIT(gfx, 6) ? fg : bg;
				*p++ = BIT(gfx, 5) ? fg : bg;
				*p++ = BIT(gfx, 4) ? fg : bg;
				*p++ = BIT(gfx, 3) ? fg : bg;
				*p++ = BIT(gfx, 2) ? fg : bg;
				*p++ = BIT(gfx, 1) ? fg : bg;
				*p++ = BIT(gfx, 0) ? fg : bg;
			}
		}
		ma+=40;
	}
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout z9001_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	1024,                   /* 4 x 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

void z9001_state::kbd_put(u8 data)
{
	m_maincpu->space(AS_PROGRAM).write_byte(0x0025, data);
}

static GFXDECODE_START( gfx_z9001 )
	GFXDECODE_ENTRY( "chargen", 0x0000, z9001_charlayout, 0, 1 )
GFXDECODE_END


void z9001_state::z9001(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(9'830'400) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &z9001_state::z9001_mem);
	m_maincpu->set_addrmap(AS_IO, &z9001_state::z9001_io);
	m_maincpu->set_daisy_config(z9001_daisy_chain);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(40*8, 24*8);
	screen.set_visarea(0, 40*8-1, 0, 24*8-1);
	screen.set_screen_update(FUNC(z9001_state::screen_update_z9001));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_z9001);
	PALETTE(config, "palette").set_entries(16);

	/* Sound */
	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 800).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* Devices */
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(z9001_state::kbd_put));
	TIMER(config, "z9001_timer").configure_periodic(FUNC(z9001_state::timer_callback), attotime::from_msec(10));

	z80pio_device& pio1(Z80PIO(config, "pio1", XTAL(9'830'400) / 4));
	pio1.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	pio1.out_pa_callback().set(FUNC(z9001_state::port88_w));

	z80pio_device& pio2(Z80PIO(config, "pio2", XTAL(9'830'400) / 4)); // keyboard PIO
	pio2.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80ctc_device& ctc(Z80CTC(config, "ctc", XTAL(9'830'400) / 4));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.zc_callback<0>().set(FUNC(z9001_state::cass_w));
	ctc.zc_callback<2>().set("ctc", FUNC(z80ctc_device::trg3));

	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

/* ROM definition */
ROM_START( z9001 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "orig", "Original" )
	ROMX_LOAD( "os____f0.851", 0xf000, 0x1000, CRC(9fe60a92) SHA1(553609631f5eaa7d6758a73f56c613e280a5b310), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "rb20", "ROM-Bank System without menu" )
	ROMX_LOAD( "os_rb20.rom",  0xf000, 0x1000, CRC(c783124d) SHA1(c2893ce5bb23b280ba4e982e860586d21de2469b), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "rb21", "ROM-Bank System with menu" )
	ROMX_LOAD( "os_rb21.rom",  0xf000, 0x1000, CRC(11eec2dd) SHA1(5dbb661bdf4daf92d6c4ffbbdec674e57917e9eb), ROM_BIOS(2))

	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "chargen.851", 0x0000, 0x0800, CRC(dd9c0f4e) SHA1(2e4928ba7161f5cce7173b7d2ded3d6596ae2aa2))
	ROM_LOAD( "zg_cga.rom",  0x0800, 0x0800, CRC(697cefb1) SHA1(f57a78a928fe1151b2fedb7f1a93a141195422ff))
	ROM_LOAD( "zg_cgai.rom", 0x1000, 0x0800, CRC(ecadf355) SHA1(4d36fefd335903680c45a5e3f38b969d2e9bb621))
	ROM_LOAD( "zg_de.rom",   0x1800, 0x0800, CRC(71854b0a) SHA1(912bb7d1f8b4582894125e82da080bd9c3b88f34))
ROM_END

#define rom_kc85_111 rom_z9001

ROM_START( kc87_10 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "orig", "Original" )
	ROMX_LOAD( "os____f0.851", 0xf000, 0x1000, CRC(9fe60a92) SHA1(553609631f5eaa7d6758a73f56c613e280a5b310), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "rb20", "ROM-Bank System without menu" )
	ROMX_LOAD( "os_rb20.rom",  0xf000, 0x1000, CRC(c783124d) SHA1(c2893ce5bb23b280ba4e982e860586d21de2469b), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "rb21", "ROM-Bank System with menu" )
	ROMX_LOAD( "os_rb21.rom",  0xf000, 0x1000, CRC(11eec2dd) SHA1(5dbb661bdf4daf92d6c4ffbbdec674e57917e9eb), ROM_BIOS(2))

	ROM_LOAD( "basic_c0.87a", 0xc000, 0x2800, CRC(c508d45e) SHA1(ea85b53e21429c4cb85cdb81b92f278a8f4eb574))

	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "chargen.851", 0x0000, 0x0800, CRC(dd9c0f4e) SHA1(2e4928ba7161f5cce7173b7d2ded3d6596ae2aa2))
	ROM_LOAD( "zg_cga.rom",  0x0800, 0x0800, CRC(697cefb1) SHA1(f57a78a928fe1151b2fedb7f1a93a141195422ff))
	ROM_LOAD( "zg_cgai.rom", 0x1000, 0x0800, CRC(ecadf355) SHA1(4d36fefd335903680c45a5e3f38b969d2e9bb621))
	ROM_LOAD( "zg_de.rom",   0x1800, 0x0800, CRC(71854b0a) SHA1(912bb7d1f8b4582894125e82da080bd9c3b88f34))
ROM_END

#define rom_kc87_11 rom_kc87_10

ROM_START( kc87_20 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "orig", "Original" )
	ROMX_LOAD( "os____f0.87b", 0xf000, 0x1000, CRC(a357d093) SHA1(b1df6b499517c8366a0795030ee800e8a258e938), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "rb20", "ROM-Bank System without menu" )
	ROMX_LOAD( "os_rb20.rom",  0xf000, 0x1000, CRC(c783124d) SHA1(c2893ce5bb23b280ba4e982e860586d21de2469b), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "rb21", "ROM-Bank System with menu" )
	ROMX_LOAD( "os_rb21.rom",  0xf000, 0x1000, CRC(11eec2dd) SHA1(5dbb661bdf4daf92d6c4ffbbdec674e57917e9eb), ROM_BIOS(2))

	ROM_LOAD( "basic_c0.87b", 0xc000, 0x2800, CRC(9e8f6380) SHA1(8ffecc64ba35c953c93738f8568c83dc6af1ae72))

	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "chargen.851", 0x0000, 0x0800, CRC(dd9c0f4e) SHA1(2e4928ba7161f5cce7173b7d2ded3d6596ae2aa2))
	ROM_LOAD( "zg_cga.rom",  0x0800, 0x0800, CRC(697cefb1) SHA1(f57a78a928fe1151b2fedb7f1a93a141195422ff))
	ROM_LOAD( "zg_cgai.rom", 0x1000, 0x0800, CRC(ecadf355) SHA1(4d36fefd335903680c45a5e3f38b969d2e9bb621))
	ROM_LOAD( "zg_de.rom",   0x1800, 0x0800, CRC(71854b0a) SHA1(912bb7d1f8b4582894125e82da080bd9c3b88f34))
ROM_END

#define rom_kc87_21 rom_kc87_20

/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY     FULLNAME              FLAGS
COMP( 1984, z9001,    0,      0,      z9001,   z9001, z9001_state, empty_init, "Robotron", "Z9001 (KC 85/1.10)", MACHINE_NOT_WORKING )
COMP( 1986, kc85_111, z9001,  0,      z9001,   z9001, z9001_state, empty_init, "Robotron", "KC 85/1.11",         MACHINE_NOT_WORKING )
COMP( 1987, kc87_10,  z9001,  0,      z9001,   z9001, z9001_state, empty_init, "Robotron", "KC 87.10",           MACHINE_NOT_WORKING )
COMP( 1987, kc87_11,  z9001,  0,      z9001,   z9001, z9001_state, empty_init, "Robotron", "KC 87.11",           MACHINE_NOT_WORKING )
COMP( 1987, kc87_20,  z9001,  0,      z9001,   z9001, z9001_state, empty_init, "Robotron", "KC 87.20",           MACHINE_NOT_WORKING )
COMP( 1987, kc87_21,  z9001,  0,      z9001,   z9001, z9001_state, empty_init, "Robotron", "KC 87.21",           MACHINE_NOT_WORKING )
