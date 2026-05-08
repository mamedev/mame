// license:BSD-3-Clause
// copyright-holders:

/*
BW5 PCB

Main components:

HD647180X0CP6 (undumped internal ROM, sanded off but serigraphed under the socket)
24 MHz XTAL
HM6716P-30 (near obj ROM)
2x HM6264ALSP-12 (near chr back ROM and data ROM)
Oki M6295
4.220 resonator
empty space for a potentiometer (SW1)
bank of 4 switches (SW2)
bank of 8 switches (SW3)

TODO:
* find a way to dump internal ROM;
* everything else.
*/

#include "emu.h"

#include "cpu/z180/hd647180x.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class probowl_state : public driver_device
{
public:
	probowl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void probowl(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


uint32_t probowl_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	return 0;
}


void probowl_state::program_map(address_map &map)
{
	map(0x00000, 0x03fff).rom();
}

void probowl_state::io_map(address_map &map)
{
}


static INPUT_PORTS_START( probowl )
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

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW3:8" )
INPUT_PORTS_END


static GFXDECODE_START( gfx_probowl )
	GFXDECODE_ENTRY( "fgtiles", 0, gfx_8x8x8_raw, 0, 16 )
	// GFXDECODE_ENTRY( "bgtiles", 0, x, 0, 16 ) // TODO
	// GFXDECODE_ENTRY( "sprites", 0, x, 0, 16 ) // TODO
GFXDECODE_END


void probowl_state::probowl(machine_config &config)
{
	HD647180X(config, m_maincpu, 24_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &probowl_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &probowl_state::io_map);

	// TODO: everything
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(probowl_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_probowl);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 4.220_MHz_XTAL, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // TODO: pin 7 not verified
}


ROM_START( probowl ) // all labels also have (C) 1994 NAC
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "probowl_cpu_version_5.15.cpu", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x4000, "data", 0 ) // maybe data for the CPU?
	ROM_LOAD( "probowl_u25_version_5.15.u25", 0x0000, 0x4000, CRC(160524af) SHA1(a9e7be06f25bb521899ebc02f41f4209f1d3dd14) ) // 2ND HALF = FFxx

	ROM_REGION( 0xa0000, "fgtiles", 0 ) // chars, title screen, ecc
	ROM_LOAD( "probowl_u67_chr-f1_v5.15.u67", 0x00000, 0x80000, CRC(85b72740) SHA1(a50a988aafcc18550df5e57a7f1df9c008eb2255) )
	ROM_LOAD( "probowl_u75_chr-f2_v5.15.u75", 0x80000, 0x20000, CRC(19ad1313) SHA1(e79150afed302bd299841f78564625e0336aaa23) )

	ROM_REGION( 0x20000, "bgtiles", 0 ) // seems pin GFX
	ROM_LOAD( "probowl_u40_chr-back_v5.15.u40", 0x00000, 0x20000, CRC(b0cc405e) SHA1(4910c54c73399ce509180ae124234ace71b57bc9) )

	ROM_REGION( 0x10000, "sprites", 0 ) // seems bowling ball GFX
	ROM_LOAD( "probowl_u43_chr-obj_v5.15.u43", 0x00000, 0x10000, CRC(9351bbdc) SHA1(c4dadce3d610ca3a1221c26da0e7ac46d466be7f) ) // FIXED BITS (xx1xxxxx)

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "probowl_u2_adpcm_v5.15.u2", 0x00000, 0x80000, CRC(6272092c) SHA1(cd8142e4cca090fd06c4f0c5135ab15bb0ea5433) )
ROM_END

} // anonymous namespace


// manufacturer and year taken from GFX assets
GAME( 1995, probowl, 0, probowl, probowl, probowl_state, empty_init, ROT0, "Zuck / Able Corp", "Pro Bowl (v5.15)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
