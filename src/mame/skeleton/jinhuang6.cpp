// license:BSD-3-Clause
// copyright-holders:

/*
Jin Huangguan 6th by Yuntong Electronics

PCB etched JHG6th 运通电子

Main components:
- PLCC-84 scratched off chip. On the underside 4J6518 19766NH 1-C 0609.
  For lack of alternatives, this is probably the CPU core with internal ROM.
- 11.0592 MHz XTAL. This is often seen with MCS51-family devices, so it may
  point to what core it is.
- M74HC08B1 quad 2-input AND gate (immediately near XTAL)
- W24257-70L RAM
- DS12C887 RTC
- U6295 (Oki M6295 clone)
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/ds128x.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class jinhuang6_state : public driver_device
{
public:
	jinhuang6_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void jinhuang6(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


void jinhuang6_state::video_start()
{
}

uint32_t jinhuang6_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	return 0;
}


static INPUT_PORTS_START( jinhuang6 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// no switches on PCB
INPUT_PORTS_END


static const gfx_layout gfx_8x8x16 =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0, 2) },
	{ STEP8(0, 8*2) },
	{ STEP8(0, 8*8*2) },
	8*8*16
};

// TODO
static GFXDECODE_START( gfx_jinhuang6 )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x16, 0, 16 )
GFXDECODE_END


void jinhuang6_state::jinhuang6(machine_config &config)
{
	Z80(config, m_maincpu, 11.0592_MHz_XTAL); // wrong! CPU not identified

	DS12885(config, "rtc", 32.768_kHz_XTAL); // should be DS12C887

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(jinhuang6_state::screen_update));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_jinhuang6);
	PALETTE(config, "palette").set_entries(0x100); // TODO

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 11.0592_MHz_XTAL / 11, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // pin 7 and clock not verified
}

ROM_START( jinhuang6 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "internal_rom", 0x00000, 0x10000, NO_DUMP ) // size unknown

	ROM_REGION( 0x400000, "tiles", 0 )
	ROM_LOAD( "jhg6th.u5", 0x000000, 0x400000, CRC(c185c9e5) SHA1(dc0518427c33e6e0f2d43fab644f02f4653b9c80) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "jhg6th.u9", 0x00000, 0x40000, CRC(e6aac74d) SHA1(b8f88b894383cd3eab4b00a20b74ee84dea61672) )

	ROM_REGION( 0x400, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "u6", 0x000, 0x117, NO_DUMP ) // read protected
	ROM_LOAD( "u7", 0x200, 0x117, NO_DUMP ) // read protected
ROM_END

} // anonymous namespace


GAME( 199?, jinhuang6, 0, jinhuang6, jinhuang6, jinhuang6_state, empty_init, ROT0, "Yuntong Electronics", "Jin Huangguan 6th", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
