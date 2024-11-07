// license:BSD-3-Clause
// copyright-holders:

/*
Video slots by Extrema running on the Extrema M27-SV PCB

The main components (at least for M27-SV rev.1.0) are:
Z84C0010PEC Z80 CPU
DALLAS DS5000T
scratched off rectangular 100-pin chip
scratched off square 200-pin chip, Extrema Ukraine 2006
36 MHz XTAL
2x 11.0592S MHz XTAL
Winbond WF19054
ADV7120 RAMDAC
2x CY7C199 RAM (1 near Z80, 1 near GFX ROMs)

TODO: everything.
*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class extrema_m27sv_state : public driver_device
{
public:
	extrema_m27sv_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void extrema_m27sv(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t extrema_m27sv_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{

	return 0;
}

void extrema_m27sv_state::video_start()
{
}


void extrema_m27sv_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}


static INPUT_PORTS_START( multgmxl )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

// no DSW on PCB
INPUT_PORTS_END


// TODO
static GFXDECODE_START( gfx_extrema_m27sv )
GFXDECODE_END


void extrema_m27sv_state::extrema_m27sv(machine_config &config)
{
	Z80(config, m_maincpu, 36_MHz_XTAL / 8 ); // divider not verified. Or maybe uses one of the 11.0592 XTALs?
	m_maincpu->set_addrmap(AS_PROGRAM, &extrema_m27sv_state::program_map);
	// m_maincpu->set_vblank_int("screen", FUNC(extrema_m27sv_state::irq0_line_hold));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(extrema_m27sv_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_extrema_m27sv);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	RAMDAC(config, "ramdac", 0, "palette");

	SPEAKER(config, "mono").front_center();

	AY8910(config, "aysnd", 36_MHz_XTAL / 8); // divider not verified. Or maybe uses one of the 11.0592 XTALs?
}


ROM_START( multgmxl )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "u8_multigamexl.u8", 0x00000, 0x40000, CRC(735a8b87) SHA1(974a3a6c4de41131021b74d280b409d668c87de9) )

	ROM_REGION( 0x8000, "ds5000t", ROMREGION_ERASE00 )
	ROM_LOAD( "internal_nvram", 0x0000, 0x8000, NO_DUMP )
	// 32K bytes internal NVRAM

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "u10_multigamexl.u10", 0x000000, 0x100000, CRC(d745758e) SHA1(9aeb8fd8e46b2ee0663f436628c95e73057dfc51) )
	ROM_LOAD( "u11_multigamexl.u11", 0x100000, 0x100000, CRC(ca59894e) SHA1(a97afe937d29e4261cc46e41283e6977b20eafc5) )
ROM_END

} // anonymous namespace


GAME( 2006?, multgmxl, 0, extrema_m27sv, multgmxl, extrema_m27sv_state, empty_init, ROT0, "Extrema", "Multi GAMEe XL", MACHINE_IS_SKELETON )
