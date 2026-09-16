// license:BSD-3-Clause
// copyright-holders:

/*
Mite Shinzeyou - 見てしんぜよう
Capcom 95683-1 + 95684-1 PCBs (LCD PCB is missing)
Slot machine from 1995, featuring three slot reels as well as a bitmapped LCD, and six
illuminated buttons.
https://www.youtube.com/watch?v=QMAC_lRcPvI


Main components for 95683-1:
Z0840008PSC CPU
8.0000 MHz XTAL
2x MB8464A-10L RAM

Main components for 95684-1:
Z0840008PSC CPU
8.0000 MHz XTAL
MB8464A-10L RAM
Oki M6295
Oki M6650
2x bank of 8 DIP switches
*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/okim6376.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class miteshin_state : public driver_device
{
public:
	miteshin_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	{}


	void miteshin(machine_config &config);

private:
	void main_program_map(address_map &map) ATTR_COLD;
	void main_io_map(address_map &map) ATTR_COLD;
	void lcd_program_map(address_map &map) ATTR_COLD;
	void lcd_io_map(address_map &map) ATTR_COLD;
};


void miteshin_state::main_program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0xe000, 0xefff).ram();
}

void miteshin_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
}

void miteshin_state::lcd_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xe000, 0xefff).ram();
}

void miteshin_state::lcd_io_map(address_map &map)
{
	map.global_mask(0xff);
}


static INPUT_PORTS_START( miteshin )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1") // standard settings all OFF according to manual
	PORT_DIPNAME(          0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(             0x04, "300 Yen-1P / 500 Yen-2P" )
	PORT_DIPSETTING(             0x05, "200 Yen-1P / 400 Yen-2P" )
	PORT_DIPSETTING(             0x06, "200 Yen-1P / 300 Yen-2P" )
	PORT_DIPSETTING(             0x07, "100 Yen-1P / 200 Yen-2P" )
	PORT_DIPSETTING(             0x01, "300 Yen-1P * 2P" )
	PORT_DIPSETTING(             0x02, "200 Yen-1P * 2P" )
	PORT_DIPSETTING(             0x03, "100 Yen-1P * 2P" )
	PORT_DIPSETTING(             0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME(          0x18, 0x18, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(             0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(             0x08, "Once every 4 attract cycles" )
	PORT_DIPSETTING(             0x10, "Once every 2 attract cycles" )
	PORT_DIPSETTING(             0x18, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6")
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7")
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8")

	PORT_START("DSW2") // standard settings all OFF according to manual
	PORT_DIPNAME(          0x03, 0x03, "Payout Settings" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(             0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(             0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(             0x03, "1P-1 piece 2P-1 piece" )
	PORT_DIPSETTING(             0x01, "1P-1 piece 2P-2 pieces" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3")
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4")
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5")
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6")
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7")
	PORT_DIPNAME(          0x80, 0x80, "Auto Test Mode" ) PORT_DIPLOCATION("SW2:8") // TODO: could do with better translation
	PORT_DIPSETTING(             0x00, "During Testing" )
	PORT_DIPSETTING(             0x80, "Usually" )
INPUT_PORTS_END


void miteshin_state::miteshin(machine_config &config)
{
	z80_device &maincpu(Z80(config, "maincpu", 8_MHz_XTAL / 2)); // divider not verified
	maincpu.set_addrmap(AS_PROGRAM, &miteshin_state::main_program_map);
	maincpu.set_addrmap(AS_IO, &miteshin_state::main_io_map);
	// maincpu.set_periodic_int(FUNC(miteshin_state::irq0_line_hold), attotime::from_hz(4 * 60));

	z80_device &lcdcpu(Z80(config, "lcdcpu", 8_MHz_XTAL / 2)); // divider not verified
	lcdcpu.set_addrmap(AS_PROGRAM, &miteshin_state::lcd_program_map);
	lcdcpu.set_addrmap(AS_IO, &miteshin_state::lcd_io_map);
	// lcdcpu.set_periodic_int(FUNC(miteshin_state::irq0_line_hold), attotime::from_hz(4 * 60));

	// TODO: one LCD display

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "m6295", 8_MHz_XTAL / 8, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5); // divider & pin 7 not verified

	OKIM6650(config, "m6650", 8_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 0.5); // divider not verified
}


ROM_START( miteshin )
	ROM_REGION( 0x10000, "maincpu", 0 ) // on 95684-1 PCB
	ROM_LOAD( "mts_01a.6j", 0x00000, 0x10000, CRC(4464f04f) SHA1(c38fdcd509a303160617f35a0a0e817621b2cebc) ) // 11xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x280000, "lcdcpu", 0 ) // on 95683-1 PCB
	ROM_LOAD( "mts_05.1d", 0x000000, 0x080000, CRC(a19ce12a) SHA1(021154b6d14ce5da27ec58afe512f4e56beafe1e) ) // 11xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "mts_06.2d", 0x080000, 0x080000, CRC(57f82561) SHA1(b53684ae397e4b211b735f09ca1b2b51eef5f4da) )
	ROM_LOAD( "mts_07.3d", 0x100000, 0x080000, CRC(123367b0) SHA1(7079df1d7fc9762cb2bafdd5a9b0be16577162b2) )
	ROM_LOAD( "mts_08.4d", 0x180000, 0x080000, CRC(32d814f3) SHA1(934dac6cb5d4e5c50cd4e30362af531f7aee5e3d) )
	ROM_LOAD( "mts_09.5d", 0x200000, 0x080000, CRC(2a8107ff) SHA1(1863366a7d8a524d0ea1c568b564210a8f29cd75) ) // 11xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "m6295", 0 ) // on 95684-1 PCB
	ROM_LOAD( "mts_02.8f", 0x00000, 0x80000, CRC(7ebbc259) SHA1(aa8c6066a229a288d2094568ea7f54efa2f8cc70) )

	ROM_REGION( 0x100000, "m6650", 0 ) // on 95684-1 PCB
	ROM_LOAD( "mts_04.5f", 0x00000, 0x80000, CRC(efc2be43) SHA1(c1856d32186fd0187fca2431694c02ef6e885f8f) )
	ROM_LOAD( "mts_03.6f", 0x80000, 0x80000, CRC(332a3855) SHA1(f50413d5fd8f330e4a4ba5021270e65f0824d019) )

	ROM_REGION( 0x800, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "myt1a.8h", 0x000, 0x117, NO_DUMP ) // PAL16L8BCN, on 95684-1 PCB
	ROM_LOAD( "lcd1a.1h", 0x200, 0x117, NO_DUMP ) // PAL16L8BCN, on 95683-1 PCB
	ROM_LOAD( "lcd2a.5c", 0x400, 0x117, NO_DUMP ) // PAL16L8BCN, on 95683-1 PCB
	ROM_LOAD( "lcd3a.2h", 0x600, 0x117, NO_DUMP ) // PAL16L8BCN, on 95683-1 PCB

ROM_END

} // anonymous namespace


GAME( 1995, miteshin, 0, miteshin, miteshin, miteshin_state, empty_init, ROT0, "Capcom", "Mite Shinzeyou", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
