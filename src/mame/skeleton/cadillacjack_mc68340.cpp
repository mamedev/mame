// license:BSD-3-Clause
// copyright-holders:

/*
MGP340Y PCB
Seems to be designed to also run updated releases of previous Cadillac Jack games emulated
in misc/blitz68k.cpp, so it's probably not too dissimilar.

Main components are:
MC68340AB25E CPU
11.0592 MHz XTAL
3.6864 MHz XTAL near CPU (for serial)
2x program ROM sockets (ROM size selectable via jumpers)
PIC17C44-33/P MCU
33 MHz XTAL
PIC16LC62B MCU
Lattice ispLSI 2064A HDPLD
DS1100-25 5-tap economy timing element
DS1302 RTC
93LC66B EEPROM
Xilinx XC9572
Philips / NXP SCC2698BC1A84 8-channel UART
BT476KPJ35 RAMDAC or compatible (i.e. ADV476KP50)
2x NEC D431000ACZ-70LL SRAM
8x GFX ROM sockets (ROM size selectable via jumpers)
YMF721-S OPL
YMF715C-S OPL
bank of 8 switches
*probably more, but pictures aren't super high quality*

There doesn't appear to be a MC6845-derived CRTC on PCB but software seems to program one?
*/

#include "emu.h"

#include "cpu/pic17/pic17c4x.h"
#include "machine/68340.h"
#include "machine/ds1302.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "video/bt47x.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace
{

class mgp340y_state : public driver_device
{
public:
	mgp340y_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void mgp340y(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


void mgp340y_state::video_start()
{
}

uint32_t mgp340y_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void mgp340y_state::program_map(address_map &map)
{
	map(0x00000000, 0x000fffff).rom();
	map(0x01000000, 0x0103ffff).ram();
}


static INPUT_PORTS_START( ffruit )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END


void mgp340y_state::mgp340y(machine_config &config)
{
	M68340(config, m_maincpu, 11.0592_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mgp340y_state::program_map);

	// NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: verify all once it works
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-8-1);
	screen.set_screen_update(FUNC(mgp340y_state::screen_update));

	PALETTE(config, "palette").set_entries(0x100);

	SPEAKER(config, "mono").front_center();
}


ROM_START( ffruit )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "funny_fruit_1.05_a.u97", 0x00000, 0x80000, CRC(5ce5df7b) SHA1(dc61f518e040abe606f045a3f908d9314211645f) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "funny_fruit_1.05_b.u98", 0x00001, 0x80000, CRC(e2ec3263) SHA1(716dd5dffb6ad40576ab946d30c8a6184998b013) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x8000, "pic17c44", 0 )
	ROM_LOAD( "cj_funny_fruit_2.30.u70", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0x1000, "pic16lc62b", 0 )
	ROM_LOAD( "pic_1.4.u69", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x200000, "blitter", 0 ) // TODO: check ROM loading
	ROM_LOAD16_BYTE( "funny_fruit_1.05_c.u81", 0x000000, 0x80000, CRC(6ec55018) SHA1(451297dc399a0cdf92dffa0709309ecd634bc51d) )
	ROM_LOAD16_BYTE( "funny_fruit_1.05_d.u71", 0x000001, 0x80000, CRC(5a4491d5) SHA1(7d6b8d0db502bf53390d67cbd898445c158aa4b4) )
	ROM_LOAD16_BYTE( "funny_fruit_1.05_e.u61", 0x100000, 0x80000, CRC(0c88040f) SHA1(12904af0f5c2e3f4d61dc9efa8fff0540a0468f8) )
	ROM_LOAD16_BYTE( "funny_fruit_1.05_f.u51", 0x100001, 0x80000, CRC(79e0fab8) SHA1(707385acb909c7d891d7acbfbd82711ff87f6241) )
	// 4 empty sockets

	ROM_REGION( 0x100000, "ymf", 0 )
	ROM_LOAD( "funny_fruit_1.05_k.u40", 0x00000, 0x80000, CRC(51b01b6f) SHA1(5a60798024029a089aa2b812d8913aca17425b4e) )
	ROM_LOAD( "funny_fruit_1.05_l.u30", 0x80000, 0x80000, CRC(5989354d) SHA1(202a5fec247f42557c348b7c5f1121e1df7c59cd) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0xa00, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.u18", 0x000, 0x117, CRC(630e5eb9) SHA1(a9e99d435c14ce660c39f517437e456af369cd0a) )
	ROM_LOAD( "gal16v8b.u32", 0x200, 0x117, CRC(630e5eb9) SHA1(a9e99d435c14ce660c39f517437e456af369cd0a) ) // yes, same contents as u18
	ROM_LOAD( "gal16v8b.u38", 0x400, 0x117, CRC(e7c88f2b) SHA1(6410b1502de3064d617c036b3127fb38977c7ab3) )
	ROM_LOAD( "gal16v8b.u49", 0x600, 0x117, CRC(99045a13) SHA1(42eb8d39049ed8094b13161c6540fe345fe4cb3f) )
	ROM_LOAD( "gal16v8b.u79", 0x800, 0x117, CRC(25ca7e57) SHA1(d55445153252c4ef21561071505f83ec17981914) )
ROM_END

} // anonymous namespace


GAME( 2004, ffruit, 0, mgp340y, ffruit, mgp340y_state, empty_init, ROT0, "Cadillac Jack", "Funny Fruit (Ver. 1.05, newer hardware)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
