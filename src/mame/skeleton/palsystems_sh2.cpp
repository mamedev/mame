// license:BSD-3-Clause
// copyright-holders:

/*
SH2 based card games by PAL Systems.

The following markings have been seen on different PCBs:
PAL PCB
PAL POKER 3
SH-POKER V3.0

The main components for PAL PCB are:
HD64F7044F28 (SH-2) with internal ROM
D720J8 XTAL (near SH2)
DS2404S RTC
KOS9F XTAL (near RTC)
square 144 pin chip with no marking (probably YGV617B-S like board below)
12.000 MHz XTAL (near 144 pin chip)
2x LATTICE Mach211SP-15JC - 18JI 8010OPY3 PK3_CTRL chips
2x GM71C18163CJ6 RAM
BR62256P RAM
Oki M6295 sound chip
2x bank of 8 DIP switches

The main components for PAL POKER 3 are:
HD64F7044F28 (SH-2) with internal ROM
D720J8 XTAL (near SH2)
DS2404S RTC
KOS9F XTAL (near RTC)
YGV617B-S Advanced Video Processor
12.000 MHz XTAL (near 144 pin chip)
LATTICE Mach211SP-15JC - 18JI 9950APA B PK2_CTRL chip
2x GM71C18163CJ6 RAM
BR62256P RAM
Oki M6295 sound chip
2x bank of 8 DIP switches

The main components for SH-POKER V3.0 are:
scratched square 112 pin chip (probably HD64F7044F28 as above)
14.318 MHz XTAL
DS2404S RTC
DS7A XTAL (near RTC)
scratched square 144 pin chip (probably YGV617B-S as above)
12.000 MHz XTAL (near 144 pin chip)
scratched square 40 pin chip (probably LATTICE Mach211SP as above)
scratched square 44 pin chip (probably Oki M6295 as above)
6x bank of 8 DIP switches (on base board)
bank of 8 DIP switches (on CPU board)

TODO: everything. Needs internal ROM dumps.
*/


#include "emu.h"

#include "cpu/sh/sh7042.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class palsystems_sh2_state : public driver_device
{
public:
	palsystems_sh2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void palsystems_sh2(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t palsystems_sh2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{

	return 0;
}

void palsystems_sh2_state::video_start()
{
}


void palsystems_sh2_state::program_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).rom();
}


static INPUT_PORTS_START( mpoker2 )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x0001, 0x0001, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x0002, 0x0002, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x0004, 0x0004, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x0008, 0x0008, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x0010, 0x0010, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x0020, 0x0020, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x0040, 0x0040, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x0080, 0x0080, "SW1:8")
	PORT_DIPUNKNOWN_DIPLOC(0x0100, 0x0100, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x0200, 0x0200, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x0400, 0x0400, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x0800, 0x0800, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x1000, 0x1000, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x2000, 0x2000, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x4000, 0x4000, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x8000, 0x8000, "SW2:8")
INPUT_PORTS_END


// TODO
static GFXDECODE_START( gfx_palsystems_sh2 )
GFXDECODE_END


void palsystems_sh2_state::palsystems_sh2(machine_config &config)
{
	SH7042(config, m_maincpu, 12'000'000); // TODO: wrong, actually SH7044. Also wrong clock.
	m_maincpu->set_addrmap(AS_PROGRAM, &palsystems_sh2_state::program_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(palsystems_sh2_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_palsystems_sh2);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 12'000'000 / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // pin 7 and clock not verified
}


ROM_START( mpoker2 ) // this was on the PAL PCB
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "internal_rom", 0x00000, 0x40000, NO_DUMP)

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "lh28f160s5t.u0222", 0x000000, 0x200000, CRC(12dc71c7) SHA1(e0db76e05752f475e554ccd24244ea8bc60f2042) )

	ROM_REGION( 0x4934, "mach", 0 )
	ROM_LOAD( "pk3_ctrl.jed", 0x0000, 0x4934, NO_DUMP )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "msm27c401.u066", 0x00000, 0x80000, CRC(2aba63a3) SHA1(c3d0a5a37cfa309cb94d6218c068fe90272d1721) ) // 1xxxxxxxxxxxxxxxxxx = 0x00
ROM_END

ROM_START( mpoker2a ) // this was on the PAL POKER 3 PCB
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "internal_rom", 0x00000, 0x40000, NO_DUMP)

	ROM_REGION( 0x4934, "mach", 0 )
	ROM_LOAD( "pk2_ctrl.u0114.jed", 0x0000, 0x4934, CRC(5f897e0e) SHA1(018eadeeb43846fb23b2665758a108e0c1a79d72) ) // JED as jedutil can't handle MACHs yet

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "flash56.bin", 0x000000, 0x200000, CRC(fbca21df) SHA1(78ab98468bb4d5563d14a9a5be4be73112e8b0a2) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "msm27c401cz.bin", 0x00000, 0x80000, CRC(c55fd486) SHA1(e8ba3686b1f88a70863da7dd6dcfa39e80c569f5) )
ROM_END

ROM_START( mpoker2b ) // this was on the SH-POKER V3.0
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "internal_rom", 0x00000, 0x40000, NO_DUMP)

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "flash56.bin", 0x000000, 0x200000, CRC(1d65901c) SHA1(f8f42ff2dc858a5970ecfbdfb08154047833676b) )

	ROM_REGION( 0x4934, "mach", 0 )
	ROM_LOAD( "mach.u0114.jed", 0x0000, 0x4934, CRC(2f9b8fdc) SHA1(ab5ddac49db6d8a8a5a59cf6fefe145031add487) ) // JED as jedutil can't handle MACHs yet

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "msm27c401.bin", 0x00000, 0x80000, CRC(2aba63a3) SHA1(c3d0a5a37cfa309cb94d6218c068fe90272d1721) ) // 1xxxxxxxxxxxxxxxxxx = 0x00
ROM_END

} // anonymous namespace


GAME( 199?, mpoker2,  0,       palsystems_sh2, mpoker2, palsystems_sh2_state, empty_init, ROT0, "PAL Systems", "Major Poker 2 (set 1)", MACHINE_IS_SKELETON )
GAME( 199?, mpoker2a, mpoker2, palsystems_sh2, mpoker2, palsystems_sh2_state, empty_init, ROT0, "PAL Systems", "Major Poker 2 (set 2)", MACHINE_IS_SKELETON )
GAME( 199?, mpoker2b, mpoker2, palsystems_sh2, mpoker2, palsystems_sh2_state, empty_init, ROT0, "PAL Systems", "Major Poker 2 (set 3)", MACHINE_IS_SKELETON )
