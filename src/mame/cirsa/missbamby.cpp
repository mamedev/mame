// license:BSD-3-Clause
// copyright-holders:

/*
Skeleton driver for Cirsa "Mini Super Fruits" slot machines and clones.
Known games on this hardware:
 ________________________________________________________________________
 | Dumped | Game              | Manufacturer       | Notes              |
 +--------+-------------------+--------------------+--------------------|
 | YES    | Mini Super Fruits | Cirsa              | Cirsa PCB 810601 A |
 | YES    | Lucky Player      | Cirsa              | Cirsa PCB 810702 A |
 | YES    | Miss Bamby        | Automatics Pasqual |                    |
 | YES    | Mini Azar D       | Arfyc              |                    |
 | YES    | Golden Winner     | Reben              |                    |
 | YES    | Golden Fruits     | Video Game         | 4 dipsw, battery   |
 | NO     | St.-Tropez        | Video Game         |                    |
 | NO     | Cocktails         | Video Game         |                    |
 | NO     | Gold Mini II      | Video Game         |                    |
 +--------+-------------------+--------------------+--------------------+

Unrelated games with similar hardware also on this driver:
 _________________________________________________________
 |Name   |Company | Description                          |
 +-------+--------+--------------------------------------|
 |Trebol | Regama | Pinball                              |
 |Basket | Igoa   | Electromechanical basketball machine |
 |Multy  | Deta   | Slot / bingo                         |
 +-------+--------+--------------------------------------+

*/

#include "emu.h"
#include "speaker.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
//#include "machine/nvram.h"
#include "sound/ay8910.h"


namespace {

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
	void igoabasket(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void io_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
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

// 4 dipswitches on Cirsa 810601 A PCB
static INPUT_PORTS_START( c_810601a )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
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

void missbamby_state::igoabasket(machine_config &config)
{
	missbamby(config);

	// Two 8155 on the Basket PCB
	I8155(config, "i8155b", 6_MHz_XTAL/2); // NEC D8155HC, guessed divisor
}


// *****************
// Games from Cirsa
// *****************

// Cirsa PCB 810702 A
ROM_START( luckyplr )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "lucky_player_24.a", 0x0000, 0x1000, CRC(11a3daf2) SHA1(239d2e53f05eecfcbc0cf5e037df21e3851e4d69) )
	ROM_LOAD( "lucky_player.b",    0x1000, 0x1000, CRC(5578abba) SHA1(1daf3de6c12792043fbea533d619e81b092a0a7d) )

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD( "lucky_player_18sa030n.bin", 0x00, 0x20, CRC(cf80f260) SHA1(b965017aa871454c54f6175fee486eea810a9c2e) )
ROM_END

// Cirsa PCB 810601 A
ROM_START( minisupf )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "mini_av_1.4.a", 0x0000, 0x0800, CRC(d29a6468) SHA1(9a6d25a6d5602aff226340e8b4a87aa8a55e7c51) )
	ROM_LOAD( "mini_av_1.4.b", 0x0800, 0x0800, CRC(0ebf0bd8) SHA1(d4a1a551dbeb56f16f17d7c02f926364132fb4a5) )
	ROM_LOAD( "mini_av_1.4.c", 0x1000, 0x0800, CRC(5f976549) SHA1(5b81e71967719a913c51fe2e027f4477d507c5b6) )

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD( "mini_18sa030n.bin", 0x00, 0x20, CRC(fa7822eb) SHA1(586705f64a5fb95e5dd1c7bfc929dccfebc3ec49) )
ROM_END


// *****************
// **** Clones *****
// *****************

// 4 dipswitches, battery on PCB, exact clone of Mini Super Fruits PCB
ROM_START( goldfrts )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "a.bin", 0x0000, 0x0800, CRC(4dc662b9) SHA1(f63cde167f8266c511c64903d6f694a6cf5885fc) )
	ROM_LOAD( "b.bin", 0x0800, 0x0800, CRC(681c2c62) SHA1(c56d2f0ecabbc3e9d2aa7e5ec07a51c5905128ff) )
	ROM_LOAD( "c.bin", 0x1000, 0x0800, CRC(0083d794) SHA1(d7d1f67fddab1560f5d42f4354b32a2e6ce0224d) )

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD( "golden_fruits_18sa030n.bin", 0x00, 0x20, CRC(fa7822eb) SHA1(586705f64a5fb95e5dd1c7bfc929dccfebc3ec49) ) // Same as Mini Super Fruits
ROM_END

/* Golden Winner - Reben. PCB without battery, 6 MHz xtal
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
|__|                                  8 dips->|S__| CA311E      |
   |____________________________________________________________|
*/
ROM_START( gwinner )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "reben_sa_gw-a_chk_8811_crc_2677.bin", 0x0000, 0x1000, CRC(ffcb4ba0) SHA1(2bc0dfc2b35a3a6cc3addf69a4c8916cc54347e4) )
	ROM_LOAD( "reben_sa_gw-b_chk_f884_crc_7822.bin", 0x2000, 0x1000, CRC(0146a4ff) SHA1(d23048ba0f23daf5caba07affc57e88ab09ca91e) )

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD( "dm74s188n.bin", 0x00, 0x20, CRC(27ed5f29) SHA1(2d5f190fac6be15d3c5b344647333bbf47a04fc3) )
ROM_END

ROM_START( miniazard )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "mini_azar_rom1.2.bin", 0x0000, 0x2000, CRC(4368a70a) SHA1(fc09b0bfb9c7777b144b909ff306098b77ef1b82) )
	ROM_LOAD( "mini_azar_rom1.1.bin", 0x2000, 0x2000, CRC(5228f8c8) SHA1(1979905e5728e388f49f9fbc0be604f37455beae) ) // 11xxxxxxxxxxx = 0xFF

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD( "prom.bin", 0x00, 0x20, NO_DUMP )
ROM_END

/* Miss Bamby - Automatics Pasqual
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
|__|                                  8 dips->|S__| LM311N      |
   |____________________________________________________________|
*/
ROM_START( msbamby )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "1.bin", 0x0000, 0x2000, CRC(7b5efbd9) SHA1(abb4b4432021945aee474c4bdd83979f6460c671) )
	ROM_LOAD( "2.bin", 0x2000, 0x2000, CRC(6048d5cd) SHA1(a3bbf43b1474de75aef9957b967ead96b9a18fc5) )

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD( "prom.bin", 0x00, 0x20, CRC(f7013c11) SHA1(6e4e6d7f2a041d44359a7f5662bb4302da234ace) ) // Unknown manufacturer, dumped as 82s123
ROM_END


// *****************
// Similar hardware
// *****************

/*  "Basket" electromechanical machine from the Spanish company IGOA S.A.
    Similar hardware, with two 8155. PCB silkscreened as "CPU 687-1".
    Two empty EPROM sockets on the dumped PCB.
   ____________________________________________________
  |                          _________                 |
  |                         | 8xDips  |                |__
  |  _____________   _________________                  __|
  | | EMPTY       | | GI AY-3-8910A   |                 __|
  | |_____________| |_________________|                 __|
  |  _____________   _________________                  __|
  | | EMPTY       | | NEC D8155HC     |                 __|
  | |_____________| |_________________|                 __|
  |  _____________   _______  _________      ________   __|
  | | EPROM       | |DM7417N  SN74LS373N    |ULN2003R|  __|
  | |_____________|  _________________       ________   __|
  |   ____________  | M5L8085AP       |     |ULN2003A|  __|
  |  | HM6116LP-3 | |_________________|                |
  |  |____________|           ________                 |__
  |   ________         Xtal   SN74LS14N                 __|
  |   82S123AN     6.144 MHz  _________________         __|
  |   ________               | NEC D8155HC     |        __|
  |   74LS393N               |_________________|        __|
  |   ________    ________                              __|
  |  PAL16R4ACN  |TC4011BP|                             __|
  |                  _____                              __|
  |  __________     NE555N             ________         __|
  |  | BATT 5V |                      |ULN2803A         __|
  |  |_________|                                        __|
  |     __                                             |
  |    (__) <- Button                                  |
  |____________________________________________________|
*/
ROM_START( basket )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "igoa_sa_ba2.81_chk_69d2_mod_basket.bin", 0x0000, 0x8000, CRC(3d52419d) SHA1(5f097391bcad72c8d0f029ef21ca38d903cef140) )

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD( "n82s123an.bin", 0x00, 0x20, CRC(1ad9b60b) SHA1(936b9f7a6d8f2dde71e86205f6c37634e4119cde) )

	ROM_REGION(0x104, "plds", 0)
	ROM_LOAD( "pal16r4acn.bin", 0x000, 0x104, NO_DUMP )
ROM_END

/* Multy - Deta. The CPU was missing on the dumped PCB (so the exact type is unknown).
   _______________________________________________________________________
  |            _______                                                   |
  |  ________  | BATT |  ________        ___     ________     ________   |
  |MCM51L01P45 |______| |N82S23N|       |__|     CD4001BE    |_EMPTY_|   |
  |            ____________  ____________                                |
  |  ________  | EPROM     || EPROM      |  ________  ________  ________ |
  |MCM51L01P45 |___________||____________|  74LS74PC  74LS393N |_______| |
  |            ________________    ________________    ________________  |
  |  ________  | GI AY-3-8910 |   | Intel P8155H  |   | CPU            | |
  | 74LS373PC  |______________|   |_______________|   |________________| |
  |                     ________   ________              (_) <- Switch   |
  |                     |8xDIPS | |_7417N_|                              |
  |                                  ________  ________                  |
  |                                  ULN2003A  ULN2003A                  |
  |                ____                               ____               |
  |_______________|    |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|    |______________|
*/
ROM_START( multy )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "tms2564jl.bin", 0x0000, 0x2000, CRC(43fa54fb) SHA1(2e0d0883deb20d68e46d3c5c5c98508c9b5d5411) )
	ROM_LOAD( "d2764d.bin",    0x2000, 0x1000, CRC(b7c55fc2) SHA1(2dfcc35821971c27178522afe9db1d18e12b3a42) ) // 1xxxxxxxxxxx = 0xFF

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD( "n83s23n.bin",   0x00, 0x20, CRC(ea598b2c) SHA1(c0d6367ed2381a4a0f22780773ff4777569e88ab) )
ROM_END

/* Pinball from the Spanish company "Regama S.A."
   CPU:   1 x I8085A
   IO:    1 x I8155
   Sound: 1 x AY8910
*/
ROM_START( trebol )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "m69.bin", 0x0000, 0x2000, CRC(8fb8cd39) SHA1(4ed505d06b489ce83316fdaa39f7ce128011fb4b) )
ROM_END


} // Anonymous namespace

// From Cirsa
GAME( 1981,  luckyplr,  0, missbamby,  missbamby, missbamby_state, empty_init, ROT0, "Cirsa",              "Lucky Player",      MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1981,  minisupf,  0, missbamby,  c_810601a, missbamby_state, empty_init, ROT0, "Cirsa",              "Mini Super Fruits", MACHINE_IS_SKELETON_MECHANICAL )

// Clones
GAME( 198?,  goldfrts,  0, missbamby,  c_810601a, missbamby_state, empty_init, ROT0, "Video Game",         "Golden Fruits",     MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1983,  gwinner,   0, gldwinner,  missbamby, missbamby_state, empty_init, ROT0, "Reben SA",           "Golden Winner",     MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1989?, miniazard, 0, gldwinner,  missbamby, missbamby_state, empty_init, ROT0, "Arfyc",              "Mini Azar D",       MACHINE_IS_SKELETON_MECHANICAL )
GAME( 198?,  msbamby,   0, missbamby,  missbamby, missbamby_state, empty_init, ROT0, "Automatics Pasqual", "Miss Bamby",        MACHINE_IS_SKELETON_MECHANICAL )

// Similar hardware
GAME( 1987?, basket,    0, igoabasket, missbamby, missbamby_state, empty_init, ROT0, "Igoa S.A.",          "Basket",            MACHINE_IS_SKELETON_MECHANICAL ) // v2.0 on ROM string, v2.81 on EPROM label
GAME( 1987,  multy,     0, missbamby,  missbamby, missbamby_state, empty_init, ROT0, "Deta",               "Multy",             MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1985,  trebol,    0, missbamby,  missbamby, missbamby_state, empty_init, ROT0, "Regama",             "Trebol",            MACHINE_IS_SKELETON_MECHANICAL )
