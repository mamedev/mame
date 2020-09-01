// license:BSD-3-Clause
// copyright-holders:

/*
Skeleton driver for Cirsa "Mini Super Fruits" and clones.
Known games on this hardware:
 __________________________________________________
 | Dumped | Game              | Manufacturer       |
 +--------+-------------------+--------------------+
 | YES    | Mini Super Fruits | Cirsa              |
 | YES    | Lucky Player      | Cirsa              |
 | YES    | Miss Bamby        | Automatics Pasqual |
 | YES    | Golden Winer      | Reben              |
 | NO     | Golden Fruits     | unknown            |
 | NO     | St.-Tropez        | unknown            |
 +-------------------------------------------------+
*/
/*
Miss Bamby - Automatics Pasqual
   _____________________________________________________________
   |                             _______                        |
   |                             |_PROM_|      ____________     |
   |                  __________________       | EMPTY     |    |
   |                  | M5L8085AP       |      |_SOCKET____|    |
   |                  |_________________|      ____________     |
   |                       ______________      | ROM1      |    |
   |         XTAL          |M5L8212P    |      |___________|    |
   |   __   6.144          |____________|      ____________     |
   |   |R|                                     | ROM0      |    |
 __|                                           |___________|    |
|__| ________  ________  ___________________   ____________     |
|__| |ULN2003A |ULN2003A | M5L8155P         |  |_D5101LC__|     |
|__|                     |__________________|  ____________     |
|__|                     ________   _________  |_D5101LC__|     |
|__|           _______   |74LS393|  |74LS74B1                   |
|__|           |7407N |                        ________         |
|__|                     ________   ________   |GD4001B|        |
|__|                     |74LS14_|  |74LS153|        _________  |
|__|                                          ____  | BATT    | |
|__|   _______           ___________________  |D  | | 3.6V    | |
|__|   |LM380N|          |    AY-3-8910     | |I  | |_________| |
|__|                     |__________________| |P  | ______      |
|__|                                          |S__| LM311N      |
   |____________________________________________________________|

Golden Winner - Reben
   _____________________________________________________________
   |                             _______                        |
   |                            DM74S188N      ____________     |
   |                  __________________       | EMPTY     |    |
   |                  | NEC D8085AC     |      |_SOCKET____|    |
   |                  |_________________|      ____________     |
   |                       ______________      | ROM-B     |    |
   |         XTAL          |INS/DP8212N |      |___________|    |
   |   __   6.000          |____________|      ____________     |
   |   |R|                                     | ROM-A     |    |
 __|                                           |___________|    |
|__| ________  ________  ___________________   ____________     |
|__| |_L203B_| |_L203B_| | NEC D8155HC      |  |MCM51L01P45|    |
|__|                     |__________________|  ____________     |
|__|                     ________   _________  |SCL 5101E-1|    |
|__|           _______   |74LS393N  |DM7474N|                   |
|__|           |7407N |                        ________         |
|__|                     ________   ________   |GD4001B|        |
|__|                     SN74LS14N  DM74LS153N                  |
|__|                                          ____              |
|__|   _______           ___________________  |D  |             |
|__|   |LM380N|          |    AY-3-8910     | |I  |             |
|__|                     |__________________| |P  | ______      |
|__|                                          |S__| CA311E      |
   |____________________________________________________________|
*/

#include "emu.h"
#include "emupal.h"
#include "speaker.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
//#include "machine/nvram.h"
#include "sound/ay8910.h"

class missbamby_state : public driver_device
{
public:
	missbamby_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void missbamby(machine_config &config);
	void gldwinner(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void io_map(address_map &map);
	void prg_map(address_map &map);

	virtual void machine_start() override;
};

void missbamby_state::prg_map(address_map &map) // preliminary, everything to be taken with a grain of salt
{
	map(0x0000, 0x3fff).rom();
	map(0x5000, 0x50ff).ram();
	//map(0x6000, 0x6000).r(); // only read once at start-up?
	map(0x8000, 0x80ff).ram();
	map(0x8800, 0x88ff).rw("i8155", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x8900, 0x8907).rw("i8155", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0x9000, 0x9000).r("psg", FUNC(ay8910_device::data_r));
	map(0x9001, 0x9001).w("psg", FUNC(ay8910_device::address_w));
	map(0x9002, 0x9002).w("psg", FUNC(ay8910_device::data_w));
}

void missbamby_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
}


static INPUT_PORTS_START( missbamby )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

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


void missbamby_state::machine_start()
{
}


void missbamby_state::missbamby(machine_config &config)
{
	I8085A(config, m_maincpu, 6.144_MHz_XTAL); // M5L8085AP
	m_maincpu->set_addrmap(AS_PROGRAM, &missbamby_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &missbamby_state::io_map);

	I8155(config, "i8155", 6.144_MHz_XTAL/2); // Guessed divisor

	SPEAKER(config, "mono").front_center();
	ay8910_device &psg(AY8910(config, "psg", 6.144_MHz_XTAL / 4));
	psg.port_a_read_callback().set_ioport("DSW1");
	psg.add_route(ALL_OUTPUTS, "mono", 1.0); // guess
}

void missbamby_state::gldwinner(machine_config &config)
{
	I8085A(config, m_maincpu, 6_MHz_XTAL); // NEC D8085AC
	m_maincpu->set_addrmap(AS_PROGRAM, &missbamby_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &missbamby_state::io_map);

	I8155(config, "i8155", 6_MHz_XTAL/2); // NEC D8155HC, guessed divisor

	SPEAKER(config, "mono").front_center();
	ay8910_device &psg(AY8910(config, "psg", 6_MHz_XTAL / 4));
	psg.port_a_read_callback().set_ioport("DSW1");
	psg.port_b_read_callback().set_ioport("IN0");
	psg.add_route(ALL_OUTPUTS, "mono", 1.0); // guess
}

ROM_START( minisupf )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "mini_av_1.4.a", 0x0000, 0x0800, CRC(d29a6468) SHA1(9a6d25a6d5602aff226340e8b4a87aa8a55e7c51) )
	ROM_LOAD( "mini_av_1.4.b", 0x0800, 0x0800, CRC(0ebf0bd8) SHA1(d4a1a551dbeb56f16f17d7c02f926364132fb4a5) )
	ROM_LOAD( "mini_av_1.4.c", 0x1000, 0x0800, CRC(5f976549) SHA1(5b81e71967719a913c51fe2e027f4477d507c5b6) )

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD( "mini_18sa030n.bin", 0x00, 0x20, CRC(fa7822eb) SHA1(586705f64a5fb95e5dd1c7bfc929dccfebc3ec49) )
ROM_END

// The 8155 was missing on this PCB, but probably it was just removed for reusing it elsewhere.
ROM_START( luckyplr )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "lucky_player_24.a", 0x0000, 0x1000, CRC(11a3daf2) SHA1(239d2e53f05eecfcbc0cf5e037df21e3851e4d69) )
	ROM_LOAD( "lucky_player.b",    0x1000, 0x1000, CRC(5578abba) SHA1(1daf3de6c12792043fbea533d619e81b092a0a7d) )

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD( "lucky_player_18sa030n.bin", 0x00, 0x20, CRC(cf80f260) SHA1(b965017aa871454c54f6175fee486eea810a9c2e) )
ROM_END

ROM_START( msbamby )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "1.bin", 0x0000, 0x2000, CRC(7b5efbd9) SHA1(abb4b4432021945aee474c4bdd83979f6460c671) )
	ROM_LOAD( "2.bin", 0x2000, 0x2000, CRC(6048d5cd) SHA1(a3bbf43b1474de75aef9957b967ead96b9a18fc5) )

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD( "prom.bin", 0x00, 0x20, CRC(f7013c11) SHA1(6e4e6d7f2a041d44359a7f5662bb4302da234ace) ) // Unknown manufacturer, dumped as 82s123
ROM_END

ROM_START( gwinner )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "reben_sa_gw-a_chk_8811_crc_2677.bin", 0x0000, 0x1000, CRC(ffcb4ba0) SHA1(2bc0dfc2b35a3a6cc3addf69a4c8916cc54347e4) )
	ROM_LOAD( "reben_sa_gw-b_chk_f884_crc_7822.bin", 0x2000, 0x1000, CRC(0146a4ff) SHA1(d23048ba0f23daf5caba07affc57e88ab09ca91e) )

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD( "dm74s188n.bin", 0x00, 0x20, NO_DUMP )
ROM_END

GAME( 1981, minisupf, 0, missbamby, missbamby, missbamby_state, empty_init, ROT0, "Cirsa",              "Mini Super Fruits", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1981, luckyplr, 0, missbamby, missbamby, missbamby_state, empty_init, ROT0, "Cirsa",              "Lucky Player",      MACHINE_IS_SKELETON_MECHANICAL )
GAME( 198?, msbamby,  0, missbamby, missbamby, missbamby_state, empty_init, ROT0, "Automatics Pasqual", "Miss Bamby",        MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1983, gwinner,  0, gldwinner, missbamby, missbamby_state, empty_init, ROT0, "Reben SA",           "Golden Winner",     MACHINE_IS_SKELETON_MECHANICAL )
