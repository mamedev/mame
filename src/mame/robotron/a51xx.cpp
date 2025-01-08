// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    A51xx

    12/05/2009 Skeleton driver.

    http://www.robotrontechnik.de/index.htm?/html/computer/a5120.htm
    http://www.robotrontechnik.de/index.htm?/html/computer/a5130.htm

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"

#include "emupal.h"
#include "screen.h"


namespace {

class a51xx_state : public driver_device
{
public:
	a51xx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void a5130(machine_config &config);
	void a5120(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	DECLARE_MACHINE_RESET(a5130);
	DECLARE_VIDEO_START(a5130);
	uint32_t screen_update_a5120(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_a5130(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	void a5120_io(address_map &map) ATTR_COLD;
	void a5120_mem(address_map &map) ATTR_COLD;
	void a5130_io(address_map &map) ATTR_COLD;
	void a5130_mem(address_map &map) ATTR_COLD;
};


void a51xx_state::a5120_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).rom();
	map(0x0400, 0xffff).ram();
}

void a51xx_state::a5120_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
}

void a51xx_state::a5130_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0xffff).ram();
}

void a51xx_state::a5130_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
}


/* Input ports */
static INPUT_PORTS_START( a5120 )
INPUT_PORTS_END


void a51xx_state::machine_reset()
{
}

void a51xx_state::machine_start()
{
}

uint32_t a51xx_state::screen_update_a5120(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


/* Input ports */
static INPUT_PORTS_START( a5130 )
INPUT_PORTS_END


MACHINE_RESET_MEMBER(a51xx_state,a5130)
{
}

VIDEO_START_MEMBER(a51xx_state,a5130)
{
}

uint32_t a51xx_state::screen_update_a5130(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout a51xx_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 1024*8, 1025*8, 1026*8, 1027*8, 1028*8, 1029*8, 1030*8, 1031*8 },
	8*8                 /* every char takes 2 x 8 bytes */
};

static GFXDECODE_START( gfx_a51xx )
	GFXDECODE_ENTRY( "chargen", 0x0000, a51xx_charlayout, 0, 1 )
GFXDECODE_END

void a51xx_state::a5120(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &a51xx_state::a5120_mem);
	m_maincpu->set_addrmap(AS_IO, &a51xx_state::a5120_io);


	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(a51xx_state::screen_update_a5120));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_a51xx);

	PALETTE(config, "palette", palette_device::MONOCHROME);
}

void a51xx_state::a5130(machine_config &config)
{
	a5120(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &a51xx_state::a5130_mem);
	m_maincpu->set_addrmap(AS_IO, &a51xx_state::a5130_io);

	MCFG_MACHINE_RESET_OVERRIDE(a51xx_state,a5130)

	/* video hardware */
	subdevice<screen_device>("screen")->set_screen_update(FUNC(a51xx_state::screen_update_a5130));

	MCFG_VIDEO_START_OVERRIDE(a51xx_state,a5130)
}

/* ROM definition */
ROM_START( a5120 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "v1", "v1" )
	ROMX_LOAD( "a5120_v1.rom", 0x0000, 0x0400, CRC(b2b3fee0) SHA1(6198513b263d8a7a867f1dda368b415bb37fcdae), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v2", "v2" )
	ROMX_LOAD( "a5120_v2.rom", 0x0000, 0x0400, CRC(052386c2) SHA1(e545d30a0882cb7ee7acdbea842b57440552e4a6), ROM_BIOS(1))

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD( "bab47_1_lat.bin", 0x0000, 0x0400, CRC(93220886) SHA1(a5a1ab4e2e06eabc58c84991caa6a1cf55f1462d))
	ROM_LOAD( "bab46_2_lat.bin", 0x0400, 0x0400, CRC(7a578ec8) SHA1(d17d3f1c182c23e9e9fd4dd56f3ac3de4c18fb1a))
ROM_END

/* ROM definition */
ROM_START( a5130 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dzr5130.rom", 0x0000, 0x1000, CRC(4719beb7) SHA1(09295a658b8c5b75b20faea57ad925f69f07a9b5))

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD( "bab47_1_lat.bin", 0x0000, 0x0400, CRC(93220886) SHA1(a5a1ab4e2e06eabc58c84991caa6a1cf55f1462d))
	ROM_LOAD( "bab46_2_lat.bin", 0x0400, 0x0400, CRC(7a578ec8) SHA1(d17d3f1c182c23e9e9fd4dd56f3ac3de4c18fb1a))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY         FULLNAME  FLAGS
COMP( 1982, a5120, 0,      0,      a5120,   a5120, a51xx_state, empty_init, "VEB Robotron", "A5120",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
COMP( 1983, a5130, a5120,  0,      a5130,   a5130, a51xx_state, empty_init, "VEB Robotron", "A5130",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
