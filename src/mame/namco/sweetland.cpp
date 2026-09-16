// license:BSD-3-Clause
// copyright-holders:x68303

/*
Namco Sweet Land crane machine hardware

SWEET LAND (1986) dumped (ROM labeled "SW 1")
NEW SWEET LAND (1990) not yet dumped
Sweet Land II (1994) not yet dumped (ROM labeled "SW 1B")
SWEET LAND III (1998) not yet dumped (ROM labeled "SWS 1 MPR 0", and at least
this version has a sub pcb with a ROM labeled "SWS 1 SPR 0")
SWEET LAND 4 (1999) no technical information found but sounds different FM sound
and PCM voice. the music is partially from NEW SWEET FACTORY (1994).
Sweet Land 4 Sakura Version/Bright Version (2005) music replaced (Oki M9810B).
other variations and later versions are not researched.

SWEET LAND (1986) ~ Sweet Land II (1994) PCB layout

      -----------------------        ---------------------------
 ----| 40pin peripheral conn |------|   50pin peripheral conn   |-----------
|     |                     |        |                         |            |
|      --|||||||||||||||||--          --|||||||||||||||||||||--             |
|    ------                                                                 |
|1  TD62064P                                                                |
|    ------                                                                 |
|                                                                           |
|                                              ------  ------  ------    ---|
|                                            TD62003P TD62003P TD62003P |+12|
|2  ----------------     ----------------      ------  ------  ------   |+12|
|  | Mitsubishi     |   | Mitsubishi     |    ------    ------          |GND|
|3 | M5L8255AP-5    |   | M5L8255AP-5    |   |LS164P|  |LS164P|         |GND|
|   ----------------     ----------------     ------    ------           ---|
|                                             ----                    Power |
|         ----------     ----------------    YM3014                  Supply |
|      4 | TOSHIBA  |   | YAMAHA         |    ----                          |
|        | TMM2016BP|   | YM2203C        |            -----                 |
| ---     ----------     ----------------            uPC324C                |
||DIP|                                                -----           ----  |
||SW6|   -----------     ----------------                            | VR | |
| --- 5 | FUJITSU   |   | SHARP          |                  -------   ----  |
|       | MBM27128  |   | LH0080A Z80A   |                 | SANYO |        |
|        -----------     ----------------                  | LA4460|        |
|      -----   -----   ----   -----   ---                   -------         |
|   6 |LS139| |LS32 | |8MHz| |LS74A| TL7705CP  1602961100                   |
|      -----   -----   ----   -----   ---     (1602971100)                  |
|        H       G        F     E   D             C      B      A           |
 ---------------------------------------------------------------------------

SWEET LAND III (1998) main PCB layout
      -----------------------        ---------------------------
 ----|    peripheral conn    |------|      peripheral conn      |-----------
|     |                     |        |                         |            |
|      --|||||||||||||||||--          --|||||||||||||||||||||--          ---|
|    ------                                                             |+12|
|J  |      |                                                      Power |+12|
|    ------                                                      Supply |GND|
|                                                                       |GND|
|                                     ------  ------  ------             ---|
|F                                   |      ||      ||      |               |
|                                     ------  ------  ------                |
|  ----------------    -----------------   ------  ------                   |
|E|                |  |                 | |      ||      |                  |
| |                |  |                 |  ------  ------                   |
|  ----------------    -----------------                                    |
|D--    -----          -----------------                                    |
|DIP|  LC35256        | YAMAHA          |      ----                         |
|SW6|   -----         | YM2203          |     YM3014                 ---    |
|---                   -----------------       ---- ------          |VR |   |
|C  --------------     -----------------           |      |          ---    |
|  | STMicro      |   | Zilog           |           ------          ----    |
|  | M27C1001     |   | Z80             |                          |amp |   |
|   --------------     -----------------                           |    |   |
|     -----   -----     ------    ---                              |    |   |
|B   |     | |     |   |      |  |   |           n  a  m  c  o      ----    |
|     -----   -----     ------   ------          Made in Japan              |
|A           |     |   |      | |ClkOsc|         M102 Game PCB              |
|             -----     ------   ------             1632960101              |
|  1  2        3            4  5  6      7    8        9   10 11  12        |
 ---------------------------------------------------------------------------
*/

#include "emu.h"
#include "speaker.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ymopn.h"


namespace {

class sweetland_state : public driver_device
{
public:
	sweetland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		{ }

	void sweetland(machine_config &config);

private:
	void program_map(address_map &map) ATTR_COLD;
};

void sweetland_state::program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x8001).rw("ym", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x9000, 0x9003).rw("8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xa000, 0xa003).rw("8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc000, 0xc000).nopw();
}


static INPUT_PORTS_START( sweetland )
/*
DIPSW pin    YM2203 IOB pin
     1             2
     2             3
     3             4
     4             5
     5             6
     6             7
    */
	PORT_START("YM2203IOB")
	PORT_DIPNAME( 0x01, 0x01, "YM2203 IOB0" ) // coin?
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "YM2203 IOB1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "YM2203 IOB2 - BGM1" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "YM2203 IOB3 - BGM2" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "YM2203 IOB4" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "YM2203 IOB5" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "YM2203 IOB6" ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "YM2203 IOB7" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void sweetland_state::sweetland(machine_config &config)
{
	z80_device &maincpu(Z80(config, "maincpu", XTAL(8'000'000)/2));
	maincpu.set_addrmap(AS_PROGRAM, &sweetland_state::program_map);

	I8255(config, "8255_0");

	I8255(config, "8255_1");

	SPEAKER(config, "mono").front_center();

	ym2203_device &ym(YM2203(config, "ym", XTAL(8'000'000)/2));
	ym.irq_handler().set_inputline("maincpu", 0);
	ym.port_a_read_callback().set([this]() { logerror("%s YM2203IOA read\n", machine().describe_context()); return 0; });
	ym.port_b_read_callback().set_ioport("YM2203IOB");
	ym.add_route(0, "mono", 0.25);
	ym.add_route(1, "mono", 0.25);
	ym.add_route(2, "mono", 0.25);
	ym.add_route(3, "mono", 1.00);
}


ROM_START(sweetlnd)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "sw1.5h", 0x0000, 0x4000, CRC(6e361ae5) SHA1(b12ce39aa8235993d0acdc623ff8dbce9feb146e) )
ROM_END

} // anonymous namespace


GAME( 1986, sweetlnd, 0, sweetland, sweetland, sweetland_state, empty_init, ROT0, "Namco", "Sweet Land", MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
