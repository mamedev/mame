// license:BSD-3-Clause
// copyright-holders:

/*
Wu Lin Zheng Ba, Huang Yeh, 1999?
Hardware Info by Guru
---------------------

HY-9802
|--------------------------------------------------------------|
| SW5  EPM7032  M5M482128   M5M482128                     T518B|
|                     M5M482128   M5M482128                    |
|                                                              |
|-|  BATT                                                      |
  |                                           EPM7032          |
|-|                            |---------|                     |
|                              |HUANG YEH|                     |
|                      22MHz   |         |        PAL22V10     |
|J   68000       63484         | HY9920  |            PAL16V8  |
|A                             |         |                     |
|M                             |---------|                     |
|M                                                  6116       |
|A                                             Z80       U6612 |
|                                                    U34       |
|  6264      U9   U8   U41   U45   U42   U46                   |
|     6264                                         M6295  U28  |
|-|                                           86171            |
  |                                                            |
|-|                       3.579545MHz                    U6614 |
|                                 8.448MHz                     |
| SW1 SW2 SW3* SW4*                   T518B        LM324  TL084|
|                          ULN2003         1242H               |
|----|       22WAY          |-----------------|   10WAY   |----|
     |----------------------|                 |-----------|
Notes:
      68000 - Motorola MC68000P8 CPU. Clock Input 8.448MHz
      63484 - Hitachi HD63484P8 Advanced CRT Controller (ACRTC). Clock Input Pin 2CLK = 5.5MHz [22/4]
        Z80 - Zilog Z84C0004 Z80 CPU. Clock Input 4.224MHz [8.448/2]
      U6612 - Clone of YM3812 FM Operator Type-LII (OPLII) Sound Chip. Clock Input 3.579545MHz
      U6614 - Clone of YM3014 Serial Input Floating D/A Converter
      M6295 - OKI M6295 4-Channel ADPCM Voice Synthesis LSI. Clock Input 2.112MHz [8.448/4]. Pin 7 LOW
  M5M482128 - Mitsubishi M5M482128AJ-8 1Mbit Dual Port RAM with 128kB x8-bit DRAM and 256 x8-bit Serial Port
       6116 - 6116 2kB x8-bit SRAM
       6264 - 6264 8kB x8-bit SRAM (both chips battery-backed)
     HY9920 - Custom QFP160 Graphics Chip
      86171 - HMC HM86171-80 Color Palette With Triple 6-Bit DAC
      TL084 - Texas Instruments TL084 Quad JFET-Input Operational Amplifier
      LM324 - Texas Instruments LM324 Quad Operational Amplifier
      1242H - NEC uPC1242H Audio Power Amplifier
      SW1-4 - 8-Position DIP Switch. * = SW3 and SW4 Not Populated.
        SW5 - Reset Switch and Clear NVRAM
      T518B - Mitsumi T518B Reset Chip (TO92)
       BATT - 3.6V Ni-Cad Battery. Powers 2x 6264 SRAMs when power is off.
    ULN2003 - ULN2003 7-Channel Darlington Transistor Array
    EPM7032 - Altera EPM7032 CPLD
      U8,U9 - 27C040 EPROM (68000 Program)
        U4x - 27C040 EPROM (Graphics)
        U34 - 27C256 EPROM (Z80 Program)
        U28 - 27C040 EPROM (Oki Samples)
*/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "video/hd63484.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class huangyeh_m68k_state : public driver_device
{
public:
	huangyeh_m68k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void wlzb(machine_config &config) ATTR_COLD;


private:
	required_device<cpu_device> m_maincpu;

	void main_program_map(address_map &map) ATTR_COLD;
	void audio_program_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
	void hd63484_map(address_map &map) ATTR_COLD;
};


void huangyeh_m68k_state::main_program_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x0fffff).rom();
	//map(0x1d0000, 0x1d0003).rw("acrtc", FUNC(hd63484_device::read16), FUNC(hd63484_device::write16));
	//map(0x1d0009, 0x1d0009).w("ramdac", FUNC(ramdac_device::index_w));
	//map(0x1d000b, 0x1d000b).w("ramdac", FUNC(ramdac_device::pal_w));
	//map(0x1d000d, 0x1d000d).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x1f0000, 0x1f3fff).ram();
}

void huangyeh_m68k_state::audio_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf880, 0xf881).w("ymsnd", FUNC(ym3812_device::write));
}

void huangyeh_m68k_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void huangyeh_m68k_state::hd63484_map(address_map &map)
{
	//map(0x00000, 0x7ffff).ram();
}


static INPUT_PORTS_START( wlzb )
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


// TODO
static GFXDECODE_START( gfx )
GFXDECODE_END


void huangyeh_m68k_state::wlzb(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 8.448_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &huangyeh_m68k_state::main_program_map);
	m_maincpu->set_vblank_int("screen", FUNC(huangyeh_m68k_state::irq0_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", 8.448_MHz_XTAL / 2));
	audiocpu.set_addrmap(AS_PROGRAM, &huangyeh_m68k_state::audio_program_map);

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

	RAMDAC(config, "ramdac", 0, "palette").set_addrmap(0, &huangyeh_m68k_state::ramdac_map);

	HD63484(config, "acrtc", 22_MHz_XTAL / 4).set_addrmap(0, &huangyeh_m68k_state::hd63484_map);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM3812(config, "ymsnd", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, "oki", 8.448_MHz_XTAL / 4, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( wlzb )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "w1-a.u9", 0x00000, 0x80000, CRC(2b6cd511) SHA1(0abfe24d83964eb45f348c5cbfd2ad50474db3c6) )
	ROM_LOAD16_BYTE( "w2-a.u8", 0x00001, 0x80000, CRC(df068584) SHA1(de051d56b9d64027f1b6dee609272186c3575bef) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "w7.u34", 0x0000, 0x8000, CRC(c00786b3) SHA1(a8b3ddf3dd1b702d8719eace1b65f42c727b9473) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD( "w3.u41", 0x000000, 0x080000, CRC(58e57d87) SHA1(f870d0729528b2fda495da059f110e466ea58de5) )
	ROM_LOAD( "w4.u45", 0x080000, 0x080000, CRC(5e993a35) SHA1(ed39dbc89cafebc8348f05a6327efa1ea26ff466) )
	ROM_LOAD( "w5.u42", 0x100000, 0x080000, CRC(e728751d) SHA1(00bc65793a65ede318e5412d06eb85259015a5c1) )
	ROM_LOAD( "w6.u46", 0x180000, 0x080000, CRC(a0ea7f31) SHA1(ef985de34485cb65ac59f7938583a0607213c81a) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "w8.u28", 0x00000, 0x80000, CRC(aad9367b) SHA1(e0b20087a8eab9d16e5cb1ed6415ca5373a43da7) )
ROM_END

} // anonymous namespace


GAME( 1999, wlzb, 0, wlzb, wlzb, huangyeh_m68k_state, empty_init, ROT0, "Huang Yeh", "Wu Lin Zheng Ba", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
