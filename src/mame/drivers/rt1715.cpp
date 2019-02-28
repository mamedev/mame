// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        Robotron PC-1715

        10/06/2008 Preliminary driver.


    Notes:

    - keyboard connected to sio channel a
    - sio channel a clock output connected to ctc trigger 0

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "video/i8275.h"
#include "emupal.h"
#include "screen.h"


class rt1715_state : public driver_device
{
public:
	rt1715_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
	{
	}

	void rt1715(machine_config &config);
	void rt1715w(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(rt1715_floppy_enable);
	DECLARE_READ8_MEMBER(k7658_led1_r);
	DECLARE_READ8_MEMBER(k7658_led2_r);
	DECLARE_READ8_MEMBER(k7658_data_r);
	DECLARE_WRITE8_MEMBER(k7658_data_w);
	DECLARE_WRITE8_MEMBER(rt1715_rom_disable);
	void rt1715_palette(palette_device &palette) const;
	I8275_DRAW_CHARACTER_MEMBER( crtc_display_pixels );

	void k7658_io(address_map &map);
	void k7658_mem(address_map &map);
	void rt1715_io(address_map &map);
	void rt1715_mem(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	int m_led1_val;
	int m_led2_val;
	required_device<z80_device> m_maincpu;
	required_device<ram_device> m_ram;
};


/***************************************************************************
    FLOPPY
***************************************************************************/

WRITE8_MEMBER(rt1715_state::rt1715_floppy_enable)
{
	logerror("%s: rt1715_floppy_enable %02x\n", machine().describe_context(), data);
}


/***************************************************************************
    KEYBOARD
***************************************************************************/

/* si/so led */
READ8_MEMBER(rt1715_state::k7658_led1_r)
{
	m_led1_val ^= 1;
	logerror("%s: k7658_led1_r %02x\n", machine().describe_context(), m_led1_val);
	return 0xff;
}

/* caps led */
READ8_MEMBER(rt1715_state::k7658_led2_r)
{
	m_led2_val ^= 1;
	logerror("%s: k7658_led2_r %02x\n", machine().describe_context(), m_led2_val);
	return 0xff;
}

/* read key state */
READ8_MEMBER(rt1715_state::k7658_data_r)
{
	uint8_t result = 0xff;

	if (BIT(offset,  0)) result &= ioport("row_00")->read();
	if (BIT(offset,  1)) result &= ioport("row_10")->read();
	if (BIT(offset,  2)) result &= ioport("row_20")->read();
	if (BIT(offset,  3)) result &= ioport("row_30")->read();
	if (BIT(offset,  4)) result &= ioport("row_40")->read();
	if (BIT(offset,  5)) result &= ioport("row_50")->read();
	if (BIT(offset,  6)) result &= ioport("row_60")->read();
	if (BIT(offset,  7)) result &= ioport("row_70")->read();
	if (BIT(offset,  8)) result &= ioport("row_08")->read();
	if (BIT(offset,  9)) result &= ioport("row_18")->read();
	if (BIT(offset, 10)) result &= ioport("row_28")->read();
	if (BIT(offset, 11)) result &= ioport("row_38")->read();
	if (BIT(offset, 12)) result &= ioport("row_48")->read();

	return result;
}

/* serial output on D0 */
WRITE8_MEMBER(rt1715_state::k7658_data_w)
{
	logerror("%s: k7658_data_w %02x\n", machine().describe_context(), BIT(data, 0));
}


/***************************************************************************
    MEMORY HANDLING
***************************************************************************/

void rt1715_state::machine_start()
{
	membank("bank2")->set_base(m_ram->pointer() + 0x0800);
	membank("bank3")->set_base(m_ram->pointer());
}

void rt1715_state::machine_reset()
{
	/* on reset, enable ROM */
	membank("bank1")->set_base(memregion("ipl")->base());
}

WRITE8_MEMBER(rt1715_state::rt1715_rom_disable)
{
	logerror("%s: rt1715_set_bank %02x\n", machine().describe_context(), data);

	/* disable ROM, enable RAM */
	membank("bank1")->set_base(m_ram->pointer());
}

/***************************************************************************
    VIDEO EMULATION
***************************************************************************/

I8275_DRAW_CHARACTER_MEMBER( rt1715_state::crtc_display_pixels )
{
}

/* F4 Character Displayer */
static const gfx_layout rt1715_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*128, 1*128*8, 2*128*8, 3*128*8, 4*128*8, 5*128*8, 6*128*8, 7*128*8, 8*128*8, 9*128*8, 10*128*8, 11*128*8, 12*128*8, 13*128*8, 14*128*8, 15*128*8 },
	8                   /* every char takes 1 x 16 bytes */
};

static GFXDECODE_START( gfx_rt1715 )
	GFXDECODE_ENTRY("gfx", 0x0000, rt1715_charlayout, 0, 1)
	GFXDECODE_ENTRY("gfx", 0x0800, rt1715_charlayout, 0, 1)
GFXDECODE_END


/***************************************************************************
    PALETTE
***************************************************************************/

void rt1715_state::rt1715_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0x00)); // black
	palette.set_pen_color(1, rgb_t(0x00, 0x7f, 0x00)); // low intensity
	palette.set_pen_color(2, rgb_t(0x00, 0xff, 0x00)); // high intensitiy
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void rt1715_state::rt1715_mem(address_map &map)
{
	map(0x0000, 0x07ff).bankr("bank1").bankw("bank3");
	map(0x0800, 0xffff).bankrw("bank2");
}

void rt1715_state::rt1715_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).rw("a71", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x04, 0x07).rw("a72", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x08, 0x0b).rw("a30", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x0c, 0x0f).rw("a29", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x18, 0x19).rw("a26", FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0x20, 0x20).w(FUNC(rt1715_state::rt1715_floppy_enable));
	map(0x28, 0x28).w(FUNC(rt1715_state::rt1715_rom_disable));
}

void rt1715_state::k7658_mem(address_map &map)
{
	map(0x0000, 0xffff).w(FUNC(rt1715_state::k7658_data_w));
	map(0x0000, 0x07ff).mirror(0xf800).rom();
}

void rt1715_state::k7658_io(address_map &map)
{
	map(0x2000, 0x2000).mirror(0x8000).r(FUNC(rt1715_state::k7658_led1_r));
	map(0x4000, 0x4000).mirror(0x8000).r(FUNC(rt1715_state::k7658_led2_r));
	map(0x8000, 0x9fff).r(FUNC(rt1715_state::k7658_data_r));
}


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( k7658 )
	PORT_START("row_00")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_20")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_30")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_40")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_50")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_60")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_70")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_08")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_18")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_28")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_38")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_48")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

/* priority unknown */
static const z80_daisy_config rt1715_daisy_chain[] =
{
	{ "a71" },
	{ "a72" },
	{ "a30" },
	{ "a29" },
	{ nullptr }
};

void rt1715_state::rt1715(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 9.832_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &rt1715_state::rt1715_mem);
	m_maincpu->set_addrmap(AS_IO, &rt1715_state::rt1715_io);
	m_maincpu->set_daisy_config(rt1715_daisy_chain);

	/* keyboard */
	z80_device &keyboard(Z80(config, "keyboard", 683000));
	keyboard.set_addrmap(AS_PROGRAM, &rt1715_state::k7658_mem);
	keyboard.set_addrmap(AS_IO, &rt1715_state::k7658_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update("a26", FUNC(i8275_device::screen_update));
	screen.set_raw(13.824_MHz_XTAL, 864, 0, 624, 320, 0, 300); // ?

	GFXDECODE(config, "gfxdecode", "palette", gfx_rt1715);
	PALETTE(config, "palette", FUNC(rt1715_state::rt1715_palette), 3);

	i8275_device &a26(I8275(config, "a26", 13.824_MHz_XTAL / 8));
	a26.set_character_width(8);
	a26.set_display_callback(FUNC(rt1715_state::crtc_display_pixels), this);

	z80ctc_device& ctc(Z80CTC(config, "a30", 9.832_MHz_XTAL / 4));
	ctc.zc_callback<0>().set("a29", FUNC(z80sio_device::txca_w));
	ctc.zc_callback<2>().set("a29", FUNC(z80sio_device::rxtxcb_w));

	Z80SIO(config, "a29", 9.832_MHz_XTAL / 4);

	/* floppy */
	Z80PIO(config, "a71", 9.832_MHz_XTAL / 4);
	Z80PIO(config, "a72", 9.832_MHz_XTAL / 4);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K").set_default_value(0x00);
}

void rt1715_state::rt1715w(machine_config &config)
{
	rt1715(config);
}


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( rt1715 )
	ROM_REGION(0x0800, "ipl", 0)
	ROM_LOAD("s500.a25.3", 0x0000, 0x0800, NO_DUMP) // CCITT 90e7
	ROM_LOAD("s501.a25.3", 0x0000, 0x0800, NO_DUMP) // CCITT 68da
	ROM_LOAD("s502.a25.3", 0x0000, 0x0800, CRC(7b6302e1) SHA1(e8f61763ff8841078a1939aa5e85a17f2af42163))

	ROM_REGION(0x1000, "gfx", 0)
	ROM_LOAD("s619.a25.2", 0x0000, 0x0800, CRC(98647763) SHA1(93fba51ed26392ec3eff1037886576fa12443fe5))
	ROM_LOAD("s602.a25.1", 0x0800, 0x0800, NO_DUMP) // CCITT fd67

	ROM_REGION(0x0800, "keyboard", 0)
	ROM_LOAD("s600.ic8", 0x0000, 0x0800, CRC(b7070122) SHA1(687056b822086ef0eee1e9b27e5b031bdbcade61))

	ROM_REGION(0x0800, "floppy", 0)
	ROM_LOAD("068.a8.2", 0x0000, 0x0400, CRC(5306d57b) SHA1(a12d025717b039a8a760eb9961365402f1f501f5)) // "read rom"
	ROM_LOAD("069.a8.1", 0x0400, 0x0400, CRC(319fa72c) SHA1(5f26af1e36339a934760a63e5975e9db09abeaaf)) // "write rom"
ROM_END

ROM_START( rt1715lc )
	ROM_REGION(0x0800, "ipl", 0)
	ROM_LOAD("s500.a25.3", 0x0000, 0x0800, NO_DUMP) // CCITT 90e7
	ROM_LOAD("s501.a25.3", 0x0000, 0x0800, NO_DUMP) // CCITT 68da
	ROM_LOAD("s502.a25.3", 0x0000, 0x0800, CRC(7b6302e1) SHA1(e8f61763ff8841078a1939aa5e85a17f2af42163))

	ROM_REGION(0x1000, "gfx", 0)
	ROM_LOAD("s643.a25.2", 0x0000, 0x0800, CRC(ea37f0e6) SHA1(357760974d944b9782734504b9820771e7e37645))
	ROM_LOAD("s605.a25.1", 0x0800, 0x0800, CRC(38062024) SHA1(798f62d4adeb7098b7dcbfe6caf28302853ee97d))

	ROM_REGION(0x0800, "keyboard", 0)
	ROM_LOAD("s642.ic8", 0x0000, 0x0800, NO_DUMP) // CCITT 962e

	ROM_REGION(0x0800, "floppy", 0)
	ROM_LOAD("068.a8.2", 0x0000, 0x0400, CRC(5306d57b) SHA1(a12d025717b039a8a760eb9961365402f1f501f5)) // "read rom"
	ROM_LOAD("069.a8.1", 0x0400, 0x0400, CRC(319fa72c) SHA1(5f26af1e36339a934760a63e5975e9db09abeaaf)) // "write rom"
ROM_END

ROM_START( rt1715w )
	ROM_REGION(0x0800, "ipl", 0)
	ROM_LOAD("s550.bin", 0x0000, 0x0800, CRC(0a96c754) SHA1(4d9ad5b877353d91ba355044d2847e1d621e2b01))

	// loaded from floppy on startup
	ROM_REGION(0x1000, "gfx", ROMREGION_ERASEFF)

	ROM_REGION(0x0800, "keyboard", 0)
	ROM_LOAD("s600.ic8", 0x0000, 0x0800, CRC(b7070122) SHA1(687056b822086ef0eee1e9b27e5b031bdbcade61))

	ROM_REGION(0x0100, "prom", 0)
	ROM_LOAD("287.bin", 0x0000, 0x0100, CRC(8508360c) SHA1(d262a8c3cf2d284c67f23b853e0d59ae5cc1d4c8)) // /CAS decoder prom, 74S287
ROM_END


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY     FULLNAME                             FLAGS
COMP( 1986, rt1715,   0,      0,      rt1715,  k7658, rt1715_state, empty_init, "Robotron", "Robotron PC-1715",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1986, rt1715lc, rt1715, 0,      rt1715,  k7658, rt1715_state, empty_init, "Robotron", "Robotron PC-1715 (latin/cyrillic)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1986, rt1715w,  rt1715, 0,      rt1715w, k7658, rt1715_state, empty_init, "Robotron", "Robotron PC-1715W",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
