// license:BSD-3-Clause
// copyright-holders:

/*******************************************************************
Bao Qing Tian, TIC, 1995
Hardware Info by Guru
---------------------

no number (bootleg-like so probably a rip-off of something else)
|-------------------------------------------------|
| uPC1241H   VOL1    VOL2          6606    TIC01  |
|            4558    4558                         |
|                           95123     3.579545MHz |
|-|             2018                             8|
  |                         95101          SCAP  L|
|-|             2018                             I|
|                                          6264  N|
|1                          Z8400A               E|
|8          |---------|                          R|
|W          |         |       |------|    TIC2-2  |
|A          |  95124  |       |PLCC68|            |
|Y          |         |       |ALTERA|    TIC03   |
|-|  SW4    |         |       |------|          |-|
  |         |---------|  21.47727MHz  T518B     |
|-|  SW3                                  TIC04 |-|
|          HM53462 HM53462    |----|              |
|10  SW2   HM53462 HM53462    |TK101      TIC05 10|
|WAY       HM53462 HM53462    |----|           WAY|
|                                                 |
|-|  SW1      JAMMA        |--|           TIC06   |
  |------------------------|  |-------------------|
Notes:
     Z8400A - Z80A CPU. Clock 5.3693175MHz [21.47727/4]
       6606 - Equivalent to OKI M6295 4-Channel ADPCM Voice Synthesis LSI. Clock input 1.342329375MHz [21.47727/16]. Pin 7 HIGH
      95123 - DIP18 IC. Could be equivalent to Yamaha YM2413 OPLL FM Synthesis Sound Chip. Clock input 3.579545MHz
      95101 - Equivalent to AY-3-8910. Clock input 1.342329375MHz [21.47727/16]
      95124 - Custom Chip (video)
      TK101 - Custom Chip (I/O?)
       2018 - 2kB x8-bit SRAM
       6264 - 8kB x8-bit SRAM (battery-backed)
    HM53462 - Hitachi HM53462 Multi-Port RAM; 64k-word x4-bit DRAM and 256-word x4-bit Serial Access RAM
              Seems to be the same type of Multi-Port RAM used on Mortal Kombat.
      SW1-4 - 8-position DIP Switch
   uPC1241H - NEC uPC1241H Audio Power Amp
       VOL1 - Music Volume Pot
       VOL2 - Voice Volume Pot
       4558 - Dual Operational Amplifier
     PLCC68 - Unknown PLCC68 IC. Altera logo partially visible so some kind of Altera CPLD
      T518B - Mitsumi PST518B Master Reset IC (TO92)
       SCAP - 5.5V 0.1F Supercap
      TIC01 - 27C2001 EPROM (oki samples)
     TIC2-2 - 27C2001 EPROM (main program)
   TIC03-06 - 27C4001 OTP EPROM (gfx)


TODO: This is almost surely bootlegged from hardware in dynax/dynax.cpp
      mjelctrn I/O map is very similar.
*******************************************************************/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class ticz80_state : public driver_device
{
public:
	ticz80_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void baoqingt(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


uint32_t ticz80_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	return 0;
}


void ticz80_state::program_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x6fff).ram();
}

void ticz80_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
}


static INPUT_PORTS_START( baoqingt )
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

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW3:8")

	PORT_START("DSW4")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW4:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW4:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW4:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW4:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW4:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW4:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW4:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW4:8")
INPUT_PORTS_END


void ticz80_state::baoqingt(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 21.477272_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &ticz80_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &ticz80_state::io_map);
	// m_maincpu->set_vblank_int("screen", FUNC(ticz80_state::irq0_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: verify everything once emulation works
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(ticz80_state::screen_update));

	PALETTE(config, "palette").set_entries(0x100); // TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2413(config, "ymsnd", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.5);

	AY8910(config, "ay", 21.477272_MHz_XTAL / 16).add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, "oki", 21.477272_MHz_XTAL / 16, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5);
}


ROM_START( baoqingt )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "tic2-2.u4", 0x00000, 0x40000, CRC(a4d16608) SHA1(254aae41284fae0eeda6ab7f72ec907cd9a5c7e2) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "tic03.u41", 0x000000, 0x80000, CRC(00c9faf2) SHA1(4eaebfc9506d7e3925e43f44c0d396c1ba38a214) )
	ROM_LOAD( "tic04.u42", 0x080000, 0x80000, CRC(cb8f0831) SHA1(122da594df6b025f96eb30eb0edcdef2c0f59556) )
	ROM_LOAD( "tic05.u43", 0x100000, 0x80000, CRC(5736a700) SHA1(b61e011858a3ee91fd69d49450b48e1d157cb11d) )
	ROM_LOAD( "tic06.u44", 0x180000, 0x80000, CRC(a093594e) SHA1(56931c1014a862fa4db2d32eb97deda20e41d92f) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "tic01.u9", 0x00000, 0x40000, CRC(b16b5dbf) SHA1(0896bb8a32c9a2d9645ce40653549b4ec9ce01a4) )
ROM_END

} // anonymous namespace


GAME( 1995, baoqingt, 0, baoqingt, baoqingt, ticz80_state, empty_init, ROT0, "TIC", "Bao Qing Tian", MACHINE_IS_SKELETON )
