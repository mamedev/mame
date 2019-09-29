// license:BSD-3-Clause
// copyright-holders:

/*
This skeleton driver hosts 2 unidentified Chang Yu Electronic (CYE) games. The PCBs,
while sharing most components, differ, so they may very well go in different drivers when emulated.

unknown Chang Yu Electronic gambling game 1:

main components:

main PCB (FAN-21 sticker):
1 x R6502P
1 x 12 MHz XTAL
3 x 8-dip banks
1 x XILINK XC2064-33 (originally covered by a black box)
1 x HD46505RP-2
1 x AY-3-8910A
1 x UM5100

small sub PCB (HY-8902):
1 x D8751H
1 x 8 MHz XTAL
1 x TIBPAL16L8


unknown Chang Yu Electronic gambling game 2:

main components:

main PCB (marked 9101):
1 x R6502AP
1 x 12 MHz XTAL
1 x 3.579545 MHz XTAL (near UM3567)
3 x 8-dip banks (2x near AY8910)
1 x HD46505RP-2
1 x AY-3-8910A
1 x UM5100
1 x UM3567 (YM2413 clone)
1 x D8253C
1 x P87C51 MCU
*/

#include "emu.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "cpu/m6502/m6502.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/ay8910.h"
#include "sound/hc55516.h"
#include "sound/ym2413.h"
#include "video/mc6845.h"

class changyu_state : public driver_device
{
public:
	changyu_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	void changyu(machine_config &config);
	void changyu2(machine_config &config);

private:

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);

	virtual void machine_start() override;
};

uint32_t changyu_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void changyu_state::main_map(address_map &map)
{
	map(0x8000, 0xffff).rom().region("maincpu", 0x0000);
}


static INPUT_PORTS_START( changyu )
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

	PORT_START("DSW0") // dips' listing available
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW0:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW0:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW0:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW0:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW0:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW0:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW0:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW0:8")

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END


void changyu_state::machine_start()
{
}

static GFXDECODE_START( gfx_changyu )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_planar, 0, 16 )
GFXDECODE_END

void changyu_state::changyu(machine_config &config)
{
	/* basic machine hardware */
	m6502_device &maincpu(M6502(config, "maincpu", XTAL(12'000'000) / 6)); // R6502P, divisor not verified
	maincpu.set_addrmap(AS_PROGRAM, &changyu_state::main_map);

	I8751(config, "mcu", XTAL(8'000'000));

	// all wrong
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(changyu_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x100);
	GFXDECODE(config, "gfxdecode", "palette", gfx_changyu);

	mc6845_device &crtc(MC6845(config, "crtc", XTAL(12'000'000) / 6));  // HD46505RP-2, divisor not verified
	crtc.set_screen("screen");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay", XTAL(12'000'000 / 6)).add_route(ALL_OUTPUTS, "mono", 1.00); // divisor not verified

	HC55516(config, "voice", XTAL(12'000'000 / 6)).add_route(ALL_OUTPUTS, "mono", 1.00); // UM5100 is a HC55536 with ROM hook-up, divisor not verified
}

void changyu_state::changyu2(machine_config &config)
{
	changyu(config);

	I87C51(config.replace(), "mcu", XTAL(8'000'000));

	YM2413(config, "ymsnd", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.0);
}

ROM_START( changyu )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "23h.u29", 0x00000, 0x8000, CRC(df0a7417) SHA1(9be2be664ed688dc9d5a1803b7f4d9bc2a0b1fae) ) // 27C256

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD( "15a.sub", 0x00000, 0x1000, CRC(c7c24394) SHA1(e290f0b49f3bb536e1bc0cd06e04ed9e99f12e47) )

	ROM_REGION(0x20000, "gfx1", 0)
	ROM_LOAD( "5.u21",   0x00000, 0x08000, CRC(25b23b14) SHA1(cb42442c09475941ffcb9940d130f9b2188ce79e) ) // 27C256
	ROM_LOAD( "6.u20",   0x08000, 0x08000, CRC(35bcfdef) SHA1(5c4173ddf55a3bf4731d819be267f738dfe9fd29) ) // 27C256
	ROM_LOAD( "7.u19",   0x10000, 0x08000, CRC(ed69d69d) SHA1(adbcea3045bec61aef9ee2ee8425f429bf5e0fc8) ) // 27C256
	ROM_LOAD( "8.u42",   0x18000, 0x08000, CRC(4013b219) SHA1(735c64647595285fc0c79c617cd6833c473daa12) ) // 27C256

	ROM_REGION(0x58000, "unsorted", 0)
	ROM_LOAD( "1.u1",    0x00000, 0x08000, CRC(8d60cb76) SHA1(f33e14549ceb6511509be16dd1c238512e6ae758) ) // 27C256
	ROM_LOAD( "2.u2",    0x08000, 0x08000, CRC(b9d78664) SHA1(763876f075f2b5b07e96b36b4e670dc466808f08) ) // 27C256
	ROM_LOAD( "3.u3",    0x10000, 0x08000, CRC(17cc6716) SHA1(df8af0fbe93b8f92219721a35772ef93bca7adb5) ) // 27C256
	ROM_LOAD( "4.u4",    0x18000, 0x08000, CRC(31b76c13) SHA1(c46da02aff8f57c0277e493c82e01970c0acd4fb) ) // 27C256

	ROM_LOAD( "9a.u74",  0x20000, 0x10000, CRC(12f3fd7a) SHA1(e220694b8fa5cfc172bf23149fceaeeb6d0b6230) ) // 27C512
	ROM_LOAD( "10a.u61", 0x30000, 0x10000, CRC(8869968b) SHA1(fbab29436acde19d7d559160ef2394d43a6ebb87) ) // 27C512
	ROM_LOAD( "11a.u62", 0x40000, 0x10000, CRC(d05e6348) SHA1(5b8bd4c94631aed46cbf7cd4db749e4855d4516c) ) // 27C512

	ROM_LOAD( "14.u70",  0x50000, 0x08000, CRC(cdfdfe11) SHA1(b170f9a6e2c77ce3ae01aabc8a963a11eb7fe74e) ) // under the sub board and near an empty socket (u77), program for a removed second CPU? or for the first?

	// u9 and u63 not populated

	ROM_REGION(0x220, "proms", 0)
	ROM_LOAD( "63s281n.u48", 0x000, 0x100, CRC(eb75e89b) SHA1(d3d6843c2cb6fb94e39d51de92205863745efdc1) )
	ROM_LOAD( "63s281n.u49", 0x100, 0x100, CRC(137e2d9c) SHA1(4e498e4fb73cad869789b902fc74d31ee3aa259f) )
	ROM_LOAD( "82s123.u44",  0x200, 0x020, CRC(cbd7e5d4) SHA1(c7d96ee7f6fb0129630fdd4b079c4ed1eabda7c5) )

	ROM_REGION(0x104, "pals", 0)
	ROM_LOAD( "tibpal16l8-25cn.sub", 0x000, 0x104, NO_DUMP )
ROM_END

ROM_START( changyu2 ) // 999 ROM999 II BY HUANGYEH string
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "95.bin", 0x00000, 0x10000, CRC(c3a8061f) SHA1(8e2b2509de32b90c0ac5f3eabb8d256a1fbb393e) ) // 27C512

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD( "99c.bin", 0x00000, 0x1000, CRC(8d52dc7d) SHA1(84ae3d95696aec6a13401ea46f7b13410dc9c31b) ) // decapped

	ROM_REGION(0x20000, "gfx1", 0)
	ROM_LOAD( "91.bin",  0x00000, 0x08000, CRC(747c98e3) SHA1(30926ee500c6ee21b7e73424afc76f34d84cb896) ) // 27256
	ROM_LOAD( "92.bin",  0x08000, 0x08000, CRC(93ac967a) SHA1(41914ccdb6d07e7a74b4db3d5473f8949462ee1c) ) // 27256
	ROM_LOAD( "93.bin",  0x10000, 0x08000, CRC(1d2b75de) SHA1(89b201b75691ee6ac3fc71fb8a998dbf05a1b0b2) ) // 27256
	ROM_LOAD( "94.bin",  0x18000, 0x08000, CRC(f61a8410) SHA1(3c4df3e973322200aa72cf1d1df827c2ba69671b) ) // 27256

	ROM_REGION(0x30000, "unsorted", 0)
	ROM_LOAD( "96c.bin", 0x00000, 0x10000, CRC(06d11350) SHA1(3c65d1d71010a3f10b00c799ede2debc96f6f3cf) ) // 27C512
	ROM_LOAD( "97c.bin", 0x10000, 0x10000, CRC(e242ab79) SHA1(a7b14692556605eb039d1ef98fb3b8b007717c12) ) // 27C512
	ROM_LOAD( "98c.bin", 0x20000, 0x10000, CRC(c8879f76) SHA1(6bcc686720dc63f50509f3f003b1f62ff43fc6b1) ) // 27C512

	ROM_REGION(0x400, "pals", 0)
	ROM_LOAD( "9a", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "9b", 0x200, 0x104, NO_DUMP )
ROM_END

GAME( 1989, changyu,  0, changyu,  changyu, changyu_state, empty_init, ROT0, "Chang Yu Electronic", "unknown Chang Yu Electronic gambling game 1", MACHINE_IS_SKELETON ) // Wing Co. in GFX1, year taken from start of maincpu ROM
GAME( 19??, changyu2, 0, changyu2, changyu, changyu_state, empty_init, ROT0, "Chang Yu Electronic", "unknown Chang Yu Electronic gambling game 2", MACHINE_IS_SKELETON ) // Wing Co. in GFX1
