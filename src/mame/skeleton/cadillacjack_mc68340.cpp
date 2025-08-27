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
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
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

ROM_START( silverb5 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a.u97", 0x00000, 0x80000, CRC(8e0e4068) SHA1(8bee162452e2f9bf76e3a8f9365a9a2c949d2fd3) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "b.u98", 0x00001, 0x80000, CRC(2d1b27c4) SHA1(026fdd1797867c7415b88abfdfe00e71665e5b58) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x8000, "pic17c44", 0 )
	ROM_LOAD( "u70", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0x1000, "pic16lc62b", 0 )
	ROM_LOAD( "u69", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x100000, "blitter", 0 ) // TODO: check ROM loading
	ROM_LOAD16_BYTE( "c.u81", 0x000000, 0x80000, CRC(bd65867d) SHA1(b0f508abfff7c5e89e93eb425a35446f87789485) )
	ROM_LOAD16_BYTE( "d.u71", 0x000001, 0x80000, CRC(ced9df7f) SHA1(bb8e7f95c8ddac4adb8881c0a663d5d3857e12ca) )
	// 6 empty sockets

	ROM_REGION( 0x100000, "ymf", 0 )
	ROM_LOAD( "k.u40", 0x00000, 0x80000, CRC(173d35ff) SHA1(6c61ec5fe0559d9756ea7081a22b911b6cf0e623) )
	// 1 empty socket

	ROM_REGION( 0xa00, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.u18", 0x000, 0x117, CRC(630e5eb9) SHA1(a9e99d435c14ce660c39f517437e456af369cd0a) )
	ROM_LOAD( "gal16v8b.u32", 0x200, 0x117, CRC(630e5eb9) SHA1(a9e99d435c14ce660c39f517437e456af369cd0a) ) // yes, same contents as u18
	ROM_LOAD( "gal16v8b.u38", 0x400, 0x117, CRC(e7c88f2b) SHA1(6410b1502de3064d617c036b3127fb38977c7ab3) )
	ROM_LOAD( "gal16v8b.u49", 0x600, 0x117, CRC(99045a13) SHA1(42eb8d39049ed8094b13161c6540fe345fe4cb3f) )
	ROM_LOAD( "gal16v8b.u79", 0x800, 0x117, CRC(25ca7e57) SHA1(d55445153252c4ef21561071505f83ec17981914) )
ROM_END

ROM_START( bestnudg )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a.u97", 0x00000, 0x80000, CRC(64bb3943) SHA1(5a8178b6acdc66e36ee0068d1378d23b4eadc013) )
	ROM_LOAD16_BYTE( "b.u98", 0x00001, 0x80000, CRC(e388a127) SHA1(eaaf960e24d771bdcd11d0e3da1a4a13bfdf221b) )

	ROM_REGION( 0x8000, "pic17c44", 0 )
	ROM_LOAD( "u70", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0x1000, "pic16lc62b", 0 )
	ROM_LOAD( "u69", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x300000, "blitter", 0 ) // TODO: check ROM loading
	ROM_LOAD16_BYTE( "c.u81", 0x000000, 0x80000, CRC(2a263fbc) SHA1(c0687590a63f50d1ce1d6a6fe309ae99ce76d4f8) )
	ROM_LOAD16_BYTE( "d.u71", 0x000001, 0x80000, CRC(7cde5b82) SHA1(878bc2ec9d14d8a426b705d0ea442b06914c69d1) )
	ROM_LOAD16_BYTE( "e.u61", 0x100000, 0x80000, CRC(f583f9cf) SHA1(c2bd725d1eef2b519c4639cc9174108777c7edad) )
	ROM_LOAD16_BYTE( "f.u51", 0x100001, 0x80000, CRC(4eefec64) SHA1(451cc90428ff82db2721216f8ef52c089b30cd01) )
	ROM_LOAD16_BYTE( "g.u41", 0x200000, 0x80000, CRC(c211671e) SHA1(fbdeb957f9d5356aa7c42383a7ade3cd666ae7b0) )
	ROM_LOAD16_BYTE( "h.u31", 0x200001, 0x80000, CRC(7d678b5c) SHA1(97e183f04f83f7c221cdfefb4d78688f9a2b1863) )
	// 2 empty sockets

	ROM_REGION( 0x100000, "ymf", 0 )
	ROM_LOAD( "k.u40", 0x00000, 0x80000, CRC(58a5f712) SHA1(721b06587ea02d73a2fe6c442e89e6e576f7bd39) )
	ROM_LOAD( "l.u30", 0x80000, 0x80000, CRC(2d2ab896) SHA1(d0e17cdef5656c5cee1cd160f751898e9b41ed0f) )

	ROM_REGION( 0xa00, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.u18", 0x000, 0x117, CRC(630e5eb9) SHA1(a9e99d435c14ce660c39f517437e456af369cd0a) )
	ROM_LOAD( "gal16v8b.u32", 0x200, 0x117, CRC(630e5eb9) SHA1(a9e99d435c14ce660c39f517437e456af369cd0a) ) // yes, same contents as u18
	ROM_LOAD( "gal16v8b.u38", 0x400, 0x117, CRC(e7c88f2b) SHA1(6410b1502de3064d617c036b3127fb38977c7ab3) )
	ROM_LOAD( "gal16v8b.u49", 0x600, 0x117, CRC(99045a13) SHA1(42eb8d39049ed8094b13161c6540fe345fe4cb3f) )
	ROM_LOAD( "gal16v8b.u79", 0x800, 0x117, CRC(25ca7e57) SHA1(d55445153252c4ef21561071505f83ec17981914) )
ROM_END

ROM_START( gldtouch )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a.u97", 0x00000, 0x80000, CRC(ed3811fd) SHA1(550de98f2edab45cb34b71fc24e04835d84ce2ea) )
	ROM_LOAD16_BYTE( "b.u98", 0x00001, 0x80000, CRC(e6e67b02) SHA1(202f7918f74511c389dcb30be980070e48c9e9ba) )

	ROM_REGION( 0x8000, "pic17c44", 0 )
	ROM_LOAD( "u70", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0x1000, "pic16lc62b", 0 )
	ROM_LOAD( "u69", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x400000, "blitter", 0 ) // TODO: check ROM loading
	ROM_LOAD16_BYTE( "c.u81", 0x000000, 0x80000, CRC(33b3c6f7) SHA1(4ecb14dd81a72e7cc0d5ca8048249c99a5365411) )
	ROM_LOAD16_BYTE( "d.u71", 0x000001, 0x80000, CRC(792825e5) SHA1(47c044bee902f3c8bf697c41d6dd855aeacb0851) )
	ROM_LOAD16_BYTE( "e.u61", 0x100000, 0x80000, CRC(7858fb4b) SHA1(ee99cef459892d8bef1d10fb2196b047fca24aac) )
	ROM_LOAD16_BYTE( "f.u51", 0x100001, 0x80000, CRC(844c444b) SHA1(a42daa96c214ba93e33592a7ef7c919c21ce9c19) )
	ROM_LOAD16_BYTE( "g.u41", 0x200000, 0x80000, CRC(940e65ba) SHA1(9d87edcf757c8835fdfdefa3af9acd265d5e13f1) )
	ROM_LOAD16_BYTE( "h.u31", 0x200001, 0x80000, CRC(f61d1f1d) SHA1(aa7a83b74caf1c53e42604b18537877cd2d954df) )
	ROM_LOAD16_BYTE( "i.u21", 0x300000, 0x80000, CRC(370279f0) SHA1(85d1aea2cb1b5b9566308311f4e0bc8f42fd77bf) )
	ROM_LOAD16_BYTE( "j.u11", 0x300001, 0x80000, CRC(36f7b578) SHA1(958f8f7dad7a1f13d5ff0fdf9319b1e0af5ea029) )

	ROM_REGION( 0x100000, "ymf", 0 )
	ROM_LOAD( "k.u40", 0x00000, 0x80000, CRC(207bba31) SHA1(b010475badbf9a62b44dae2e90561acb5eb7064f) )
	ROM_LOAD( "l.u30", 0x80000, 0x80000, CRC(c82754c3) SHA1(0c29e3ed09b80d026deb760c5df4196cb418252f) )

	ROM_REGION( 0xa00, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.u18", 0x000, 0x117, CRC(630e5eb9) SHA1(a9e99d435c14ce660c39f517437e456af369cd0a) )
	ROM_LOAD( "gal16v8b.u32", 0x200, 0x117, CRC(630e5eb9) SHA1(a9e99d435c14ce660c39f517437e456af369cd0a) ) // yes, same contents as u18
	ROM_LOAD( "gal16v8b.u38", 0x400, 0x117, CRC(e7c88f2b) SHA1(6410b1502de3064d617c036b3127fb38977c7ab3) )
	ROM_LOAD( "gal16v8b.u49", 0x600, 0x117, CRC(99045a13) SHA1(42eb8d39049ed8094b13161c6540fe345fe4cb3f) )
	ROM_LOAD( "gal16v8b.u79", 0x800, 0x117, CRC(25ca7e57) SHA1(d55445153252c4ef21561071505f83ec17981914) )
ROM_END

// this is dated 1998 in ROM but seems way early for this hardware?
ROM_START( gldtouch239 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a.u97", 0x00000, 0x80000, CRC(545b3540) SHA1(8dcc214a1c8e9ad7fab3ce2c7f9887a85f4304e2) )
	ROM_LOAD16_BYTE( "b.u98", 0x00001, 0x80000, CRC(6e0ae501) SHA1(8534794c30fb6c8e6cf9187046091c9de41cb8ff) )

	ROM_REGION( 0x8000, "pic17c44", 0 )
	ROM_LOAD( "u70", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0x1000, "pic16lc62b", 0 )
	ROM_LOAD( "u69", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x400000, "blitter", 0 ) // TODO: check ROM loading
	ROM_LOAD16_BYTE( "c.u81", 0x000000, 0x80000, CRC(33b3c6f7) SHA1(4ecb14dd81a72e7cc0d5ca8048249c99a5365411) )
	ROM_LOAD16_BYTE( "d.u71", 0x000001, 0x80000, CRC(792825e5) SHA1(47c044bee902f3c8bf697c41d6dd855aeacb0851) )
	ROM_LOAD16_BYTE( "e.u61", 0x100000, 0x80000, CRC(7858fb4b) SHA1(ee99cef459892d8bef1d10fb2196b047fca24aac) )
	ROM_LOAD16_BYTE( "f.u51", 0x100001, 0x80000, CRC(844c444b) SHA1(a42daa96c214ba93e33592a7ef7c919c21ce9c19) )
	ROM_LOAD16_BYTE( "g.u41", 0x200000, 0x80000, CRC(940e65ba) SHA1(9d87edcf757c8835fdfdefa3af9acd265d5e13f1) )
	ROM_LOAD16_BYTE( "h.u31", 0x200001, 0x80000, CRC(f61d1f1d) SHA1(aa7a83b74caf1c53e42604b18537877cd2d954df) )
	ROM_LOAD16_BYTE( "i.u21", 0x300000, 0x80000, CRC(370279f0) SHA1(85d1aea2cb1b5b9566308311f4e0bc8f42fd77bf) )
	ROM_LOAD16_BYTE( "j.u11", 0x300001, 0x80000, CRC(36f7b578) SHA1(958f8f7dad7a1f13d5ff0fdf9319b1e0af5ea029) )

	ROM_REGION( 0x100000, "ymf", 0 )
	ROM_LOAD( "k.u40", 0x00000, 0x80000, CRC(207bba31) SHA1(b010475badbf9a62b44dae2e90561acb5eb7064f) )
	ROM_LOAD( "l.u30", 0x80000, 0x80000, CRC(c82754c3) SHA1(0c29e3ed09b80d026deb760c5df4196cb418252f) )

	ROM_REGION( 0xa00, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.u18", 0x000, 0x117, CRC(630e5eb9) SHA1(a9e99d435c14ce660c39f517437e456af369cd0a) )
	ROM_LOAD( "gal16v8b.u32", 0x200, 0x117, CRC(630e5eb9) SHA1(a9e99d435c14ce660c39f517437e456af369cd0a) ) // yes, same contents as u18
	ROM_LOAD( "gal16v8b.u38", 0x400, 0x117, CRC(e7c88f2b) SHA1(6410b1502de3064d617c036b3127fb38977c7ab3) )
	ROM_LOAD( "gal16v8b.u49", 0x600, 0x117, CRC(99045a13) SHA1(42eb8d39049ed8094b13161c6540fe345fe4cb3f) )
	ROM_LOAD( "gal16v8b.u79", 0x800, 0x117, CRC(25ca7e57) SHA1(d55445153252c4ef21561071505f83ec17981914) )
ROM_END

ROM_START( southgld )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a.u97", 0x00000, 0x80000, CRC(7b860560) SHA1(4fda42b8660773fbf9a1af0075e31b14935da5a4) )
	ROM_IGNORE(                        0x80000 ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD16_BYTE( "b.u98", 0x00001, 0x80000, CRC(f1c2f165) SHA1(511e7b7dcae00f1c2b44b0e1a8a0ed37c215047d) )
	ROM_IGNORE(                        0x80000 ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x8000, "pic17c44", 0 )
	ROM_LOAD( "u70", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0x1000, "pic16lc62b", 0 )
	ROM_LOAD( "u69", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x600000, "blitter", 0 ) // TODO: check ROM loading
	ROM_LOAD16_BYTE( "c.u81", 0x000000, 0x100000, CRC(9a5d9777) SHA1(3eaf20cce406e2d80d039f32e2d44d2c2aa6c1fb) )
	ROM_LOAD16_BYTE( "d.u71", 0x000001, 0x100000, CRC(fa530ab7) SHA1(92e77392351056c607b402f532e706f959bac630) )
	ROM_LOAD16_BYTE( "e.u61", 0x200000, 0x100000, CRC(82cc5a6b) SHA1(80b101df2589205cf53796d6c1161cacb3a73872) )
	ROM_LOAD16_BYTE( "f.u51", 0x200001, 0x100000, CRC(2c28af4c) SHA1(2ba4a731d422ed943a1a5c1c1fc916dfe98dd44e) )
	ROM_LOAD16_BYTE( "g.u41", 0x400000, 0x100000, CRC(3a781c78) SHA1(190437f94920ad4a49a4ec8f80ad6c3809dee958) )
	ROM_LOAD16_BYTE( "h.u31", 0x400001, 0x100000, CRC(42e54c06) SHA1(c24be9d18dbfeb3fb4184852a867863c6265d6f5) )
	// 2 empty sockets

	ROM_REGION( 0x100000, "ymf", 0 )
	ROM_LOAD( "k.u40", 0x00000, 0x80000, CRC(a77bf3e4) SHA1(07ace3ac729062ec4be122b146eef276e532c881) )
	ROM_LOAD( "l.u30", 0x80000, 0x80000, CRC(6136f8bd) SHA1(3c2310bd02d8306c267fd41e63c9694cb22be6b1) )

	ROM_REGION( 0xa00, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.u18", 0x000, 0x117, CRC(630e5eb9) SHA1(a9e99d435c14ce660c39f517437e456af369cd0a) )
	ROM_LOAD( "gal16v8b.u32", 0x200, 0x117, CRC(630e5eb9) SHA1(a9e99d435c14ce660c39f517437e456af369cd0a) ) // yes, same contents as u18
	ROM_LOAD( "gal16v8b.u38", 0x400, 0x117, CRC(e7c88f2b) SHA1(6410b1502de3064d617c036b3127fb38977c7ab3) )
	ROM_LOAD( "gal16v8b.u49", 0x600, 0x117, CRC(99045a13) SHA1(42eb8d39049ed8094b13161c6540fe345fe4cb3f) )
	ROM_LOAD( "gal16v8b.u79", 0x800, 0x117, CRC(25ca7e57) SHA1(d55445153252c4ef21561071505f83ec17981914) )
ROM_END

ROM_START( southgld114 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a.u97", 0x00000, 0x80000, CRC(0f02ca6a) SHA1(a0aedcbb768a1848069e5d64fab1bfb4b9a4e318) )
	ROM_LOAD16_BYTE( "b.u98", 0x00001, 0x80000, CRC(738ef7da) SHA1(4658f26c6e7eadf09cc668cef016ed77b20733b0) )

	ROM_REGION( 0x8000, "pic17c44", 0 )
	ROM_LOAD( "u70", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0x1000, "pic16lc62b", 0 )
	ROM_LOAD( "u69", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x600000, "blitter", 0 ) // TODO: check ROM loading
	ROM_LOAD16_BYTE( "c.u81", 0x000000, 0x100000, CRC(9a5d9777) SHA1(3eaf20cce406e2d80d039f32e2d44d2c2aa6c1fb) )
	ROM_LOAD16_BYTE( "d.u71", 0x000001, 0x100000, CRC(fa530ab7) SHA1(92e77392351056c607b402f532e706f959bac630) )
	ROM_LOAD16_BYTE( "e.u61", 0x200000, 0x100000, CRC(82cc5a6b) SHA1(80b101df2589205cf53796d6c1161cacb3a73872) )
	ROM_LOAD16_BYTE( "f.u51", 0x200001, 0x100000, CRC(2c28af4c) SHA1(2ba4a731d422ed943a1a5c1c1fc916dfe98dd44e) )
	ROM_LOAD16_BYTE( "g.u41", 0x400000, 0x100000, CRC(3a781c78) SHA1(190437f94920ad4a49a4ec8f80ad6c3809dee958) )
	ROM_LOAD16_BYTE( "h.u31", 0x400001, 0x100000, CRC(42e54c06) SHA1(c24be9d18dbfeb3fb4184852a867863c6265d6f5) )
	// 2 empty sockets

	ROM_REGION( 0x100000, "ymf", 0 )
	ROM_LOAD( "k.u40", 0x00000, 0x80000, CRC(a77bf3e4) SHA1(07ace3ac729062ec4be122b146eef276e532c881) )
	ROM_LOAD( "l.u30", 0x80000, 0x80000, CRC(6136f8bd) SHA1(3c2310bd02d8306c267fd41e63c9694cb22be6b1) )

	ROM_REGION( 0xa00, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.u18", 0x000, 0x117, CRC(630e5eb9) SHA1(a9e99d435c14ce660c39f517437e456af369cd0a) )
	ROM_LOAD( "gal16v8b.u32", 0x200, 0x117, CRC(630e5eb9) SHA1(a9e99d435c14ce660c39f517437e456af369cd0a) ) // yes, same contents as u18
	ROM_LOAD( "gal16v8b.u38", 0x400, 0x117, CRC(e7c88f2b) SHA1(6410b1502de3064d617c036b3127fb38977c7ab3) )
	ROM_LOAD( "gal16v8b.u49", 0x600, 0x117, CRC(99045a13) SHA1(42eb8d39049ed8094b13161c6540fe345fe4cb3f) )
	ROM_LOAD( "gal16v8b.u79", 0x800, 0x117, CRC(25ca7e57) SHA1(d55445153252c4ef21561071505f83ec17981914) )
ROM_END

} // anonymous namespace


GAME( 2004, ffruit,      0,        mgp340y, ffruit, mgp340y_state, empty_init, ROT0, "Cadillac Jack", "Funny Fruit (Ver. 1.05, newer hardware)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2004, silverb5,    0,        mgp340y, ffruit, mgp340y_state, empty_init, ROT0, "Cadillac Jack", "Silver Bar (Ver. 1.07)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2007, bestnudg,    0,        mgp340y, ffruit, mgp340y_state, empty_init, ROT0, "Cadillac Jack", "Best of Nudge (Ver. 1.01)",               MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2004, gldtouch,    0,        mgp340y, ffruit, mgp340y_state, empty_init, ROT0, "Cadillac Jack", "Gold Touch (Ver. 2.45)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1998, gldtouch239, gldtouch, mgp340y, ffruit, mgp340y_state, empty_init, ROT0, "Cadillac Jack", "Gold Touch (Ver. 2.39)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2004, southgld,    0,        mgp340y, ffruit, mgp340y_state, empty_init, ROT0, "Cadillac Jack", "Southern Gold (Ver. 1.16)",               MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2001, southgld114, southgld, mgp340y, ffruit, mgp340y_state, empty_init, ROT0, "Cadillac Jack", "Southern Gold (Ver. 1.14)",               MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
