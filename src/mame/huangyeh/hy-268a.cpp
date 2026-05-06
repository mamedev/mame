// license:BSD-3-Clause
// copyright-holders:

/*
Huang Yeh HY-268A PCB

seems a more compact / cost reduced version of the HY-9802 PCB in huangyeh/hy-9802.cpp

Main components are:
JX-1689HP CPU (M68000 clone)
HD63484P8 ACRTC
22.000 MHx XTAL
4x M5M482128AJ SDRAM
2x GM76C88ALK-15 SRAM
HM86171-80 RAMDAC
HUANG YEH HY9826 160-pin custom (GFX related?)
2x Altera EPM70325LC44-10 FPGA
U6295 ADPCM (Oki M6295 clone)
UM3567 OPL (YM2413 clone)
ATMEL 93C46 EEPROM


TODO:
- crashes strong with ACRTC, plenty of unsupported features (starting with COMMAND_DWT);
- EEPROM;
- sound;
- I/Os;
*/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "video/hd63484.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class hy268a_state : public driver_device
{
public:
	hy268a_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void xycs(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
	void hd63484_map(address_map &map) ATTR_COLD;
};


void hy268a_state::program_map(address_map &map)
{
	map.unmap_value_high();

	map(0x000000, 0x07ffff).rom();
	map(0x110020, 0x110021).portr("IN0");
	map(0x170000, 0x170003).rw("acrtc", FUNC(hd63484_device::read16), FUNC(hd63484_device::write16));
	map(0x170009, 0x170009).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x17000b, 0x17000b).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x17000d, 0x17000d).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x1f0000, 0x1f7fff).ram();
}

void hy268a_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void hy268a_state::hd63484_map(address_map &map)
{
	// TODO: likely banked, also writes
	map(0x00000, 0xfffff).rom().region("tiles", 0);
}


static INPUT_PORTS_START( xycs )
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

	// PCB has no DIP switches
INPUT_PORTS_END


// TODO
static GFXDECODE_START( gfx )
GFXDECODE_END


void hy268a_state::xycs(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 22_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &hy268a_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(hy268a_state::irq3_line_hold));

	EEPROM_93C46_16BIT(config, "eeprom");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: verify everything once emulation works
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 2*8, 30*8-1);
	screen.set_screen_update("acrtc", FUNC(hd63484_device::update_screen));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	RAMDAC(config, "ramdac", 0, "palette").set_addrmap(0, &hy268a_state::ramdac_map);

	HD63484(config, "acrtc", 22_MHz_XTAL / 4).set_addrmap(0, &hy268a_state::hd63484_map); // divider not verified

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2413(config, "ymsnd", 22_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 1.00); // divider not verified

	OKIM6295(config, "oki", 22_MHz_XTAL / 22, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 1.0); // divider and pin 7 not verified
}


// 幸运财神 (Xìngyùn Cáishén)
ROM_START( xycs )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "hy_566.u1", 0x00000, 0x80000, CRC(22a3b218) SHA1(fc0e4e41b77c53ad95b6344f24da767f03bc6ed3) )

	ROM_REGION16_BE( 0x400000, "tiles", 0 )
	ROM_LOAD16_WORD_SWAP( "p_2a.u26", 0x000000, 0x200000, CRC(d6bdc1b6) SHA1(abca4ecaa8abe2d632df901822abdfb5c24e79f3) ) // 1xxxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_WORD_SWAP( "p_3b.u27", 0x200000, 0x200000, CRC(ea1259a8) SHA1(e258258b0a40f8dc399ac6c8cfa78ca150d59b23) ) // 1xxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "p_4.u35", 0x00000, 0x80000, CRC(86ee70de) SHA1(29fb637a88a22542d453890bad675b5e8bfef623) )
ROM_END

} // anonymous namespace


GAME( 2001, xycs, 0, xycs, xycs, hy268a_state, empty_init, ROT0, "Huang Yeh", "Xingyun Caishen", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
