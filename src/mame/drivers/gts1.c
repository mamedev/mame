/****************************************************************************************************

PINBALL
Gottlieb System 1

Gottlieb's first foray into computerised pinball.

Typical of Gottlieb's golden period, these machines are physically well-designed and made.
However, the computer side was another story, an attempt to catch up to its competitors who
were way ahead in the technology race. Instead of each board being solidly grounded to the
chassis, the only connections were through flaky edge connectors. Voltage differences would
then cause solenoids and lights to switch on at random and destroy transistors. Further, the
CPU chips chosen were an unusual 4-bit design that was already old.

The first games had chimes. Then, this was replaced by 3 NE555 tone oscillators. The last
machines had a real sound board which had more computing power than the main cpu.

Game numbering:
Each Gottlieb game had the model number printed on the instruction card, so it was very
easy to gather information. Gottlieb either made a single-player game, or a 2-player and
a 4-player game. For example, Centigrade 37 (#407) was a single-player game, while Bronco
(4-player)(#396) was exactly the same as Mustang (2-player)(#397). Cleopatra (#409) was
originally a 4-player EM game (with Pyramid #410 being the 2-player version). Then, the SS
version was made, and it kept the same number. After that, the SS versions were suffixed
with 'SS' up to The Incredible Hulk (#433), and then the 'SS' was dropped.


Game List:
Number  ROM  Name
409     A    Cleopatra
412SS   B    Sinbad
417SS   C    Joker Poker
419SS   D    Dragon
421SS   E    Solar Ride
422SS   F    Countdown
424SS   G    Close Encounters of the third kind
425SS   H    Charlie's Angels
427SS   I    Pinball Pool
429SS   J    Totem
433SS   K    The Incredible Hulk
435     L    Genie
437     N    Buck Rogers
438     P    Torch
440     R    Roller Disco
442     S    Asteroid Annie and the Aliens

Chips used:
U1 11660     CPU
U2 10696EE   5101L RAM interface (device#6)
U3 10696EE   General purpose I/O (dipswitches, lamps, misc) (device#3)
U4 A1753CX   Custom 2kx8 ROM, 128x4 RAM, 16x1 I/O (solenoid control)
U5 A1752CX   Custom 2kx8 ROM, 128x4 RAM, 16x1 I/O (switch matrix)
U6 10788     Display driver
   5101L     4-bit static RAM
   MM6351-IJ ROM


ToDo:
- Everything
- Hard to debug because no errors are logged; also the program flow seems odd.
- 5101L RAM (battery-backed) is driven from the 10696.
- MM6351 ROM is driven from the CPU I/O ports and has 4 banks.

*****************************************************************************************************/


#include "machine/genpin.h"
#include "cpu/pps4/pps4.h"
//#include "gts1.lh"

class gts1_state : public genpin_class
{
public:
	gts1_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_DRIVER_INIT(gts1);
private:
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START( gts1_map, AS_PROGRAM, 8, gts1_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( gts1_data, AS_DATA, 8, gts1_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM // not correct
ADDRESS_MAP_END

static ADDRESS_MAP_START( gts1_io, AS_IO, 8, gts1_state )
	AM_RANGE(0x0000, 0x00ff) AM_RAM // connects to all the other chips
ADDRESS_MAP_END

static INPUT_PORTS_START( gts1 )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "S01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x20, "S06")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ))
	PORT_DIPNAME( 0x40, 0x40, "S07")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ))
	PORT_DIPNAME( 0x80, 0x80, "S08")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "S09")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S14")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ))
	PORT_DIPSETTING(    0x20, DEF_STR( No ))
	PORT_DIPNAME( 0x40, 0x40, "S15")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ))
	PORT_DIPNAME( 0x80, 0x00, "S16")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "S17")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S18")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S19")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S20")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S21")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S22")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S23")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "S24")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))
INPUT_PORTS_END

void gts1_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(gts1_state,gts1)
{
}

static MACHINE_CONFIG_START( gts1, gts1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPS4, XTAL_3_579545MHz / 18)  // divided in the CPU
	MCFG_CPU_PROGRAM_MAP(gts1_map)
	MCFG_CPU_DATA_MAP(gts1_data)
	MCFG_CPU_IO_MAP(gts1_io)

	//MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	//MCFG_DEFAULT_LAYOUT(layout_gts1)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
MACHINE_CONFIG_END


ROM_START( gts1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
ROM_END

ROM_START( gts1s )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
ROM_END

/*-------------------------------------------------------------------
/ Asteroid Annie and the Aliens (12/1980) #442
/-------------------------------------------------------------------*/
ROM_START(astannie)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("442.cpu", 0x2000, 0x0400, CRC(579521e0) SHA1(b1b19473e1ca3373955ee96104b87f586c4c311c))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("442.snd", 0x0400, 0x0400, CRC(c70195b4) SHA1(ff06197f07111d6a4b8942dcfe8d2279bda6f281))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Buck Rogers (01/1980) #437
/-------------------------------------------------------------------*/
ROM_START(buckrgrs)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("437.cpu", 0x2000, 0x0400, CRC(e57d9278) SHA1(dfc4ebff1e14b9a074468671a8e5ac7948d5b352))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("437.snd", 0x0400, 0x0400, CRC(732b5a27) SHA1(7860ea54e75152246c3ac3205122d750b243b40c))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Charlie's Angels (11/1978) #425
/-------------------------------------------------------------------*/
ROM_START(charlies)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("425.cpu", 0x2000, 0x0400, CRC(928b4279) SHA1(51096d45e880d6a8263eaeaa0cdab0f61ad2f58d))
ROM_END
/*-------------------------------------------------------------------
/ Cleopatra (11/1977) #409
/-------------------------------------------------------------------*/
ROM_START(cleoptra)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("409.cpu", 0x2000, 0x0400, CRC(8063ff71) SHA1(205f09f067bf79544d2ce2a48d23259901f935dd))
ROM_END

/*-------------------------------------------------------------------
/ Close Encounters of the Third Kind (10/1978) #424
/-------------------------------------------------------------------*/
ROM_START(closeenc)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("424.cpu", 0x2000, 0x0400, CRC(a7a5dd13) SHA1(223c67b9484baa719c91de52b363ff22813db160))
ROM_END

/*-------------------------------------------------------------------
/ Count-Down (05/1979) #422
/-------------------------------------------------------------------*/
ROM_START(countdwn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("422.cpu", 0x2000, 0x0400, CRC(51bc2df0) SHA1(d4b555d106c6b4e420b0fcd1df8871f869476c22))
ROM_END

/*-------------------------------------------------------------------
/ Dragon (10/1978) #419
/-------------------------------------------------------------------*/
ROM_START(dragon)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("419.cpu", 0x2000, 0x0400, CRC(018d9b3a) SHA1(da37ef5017c71bc41bdb1f30d3fd7ac3b7e1ee7e))
ROM_END

/*-------------------------------------------------------------------
/ Genie (11/1979) #435
/-------------------------------------------------------------------*/
ROM_START(geniep)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("435.cpu", 0x2000, 0x0400, CRC(7749fd92) SHA1(9cd3e799842392e3939877bf295759c27f199e58))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("435.snd", 0x0400, 0x0400, CRC(4a98ceed) SHA1(f1d7548e03107033c39953ee04b043b5301dbb47))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Joker Poker (08/1978) #417
/-------------------------------------------------------------------*/
ROM_START(jokrpokr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("417.cpu", 0x2000, 0x0400, CRC(33dade08) SHA1(23b8dbd7b6c84b806fc0d2da95478235cbf9f80a))
ROM_END

/*-------------------------------------------------------------------
/ Jungle Queen (1985)
/-------------------------------------------------------------------*/
/*-------------------------------------------------------------------
/ L'Hexagone (04/1986)
/-------------------------------------------------------------------*/
ROM_START(hexagone)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("435.cpu", 0x2000, 0x0400, CRC(7749fd92) SHA1(9cd3e799842392e3939877bf295759c27f199e58))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("hexagone.bin", 0, 0x4000, CRC(002b5464) SHA1(e2d971c4e85b4fb6580c2d3945c9946ea0cebc2e))
ROM_END
/*-------------------------------------------------------------------
/ Movie
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Pinball Pool (08/1979) #427
/-------------------------------------------------------------------*/
ROM_START(pinpool)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("427.cpu", 0x2000, 0x0400, CRC(c496393d) SHA1(e91d9596aacdb4277fa200a3f8f9da099c278f32))
ROM_END

/*-------------------------------------------------------------------
/ Roller Disco (02/1980) #440
/-------------------------------------------------------------------*/
ROM_START(roldisco)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("440.cpu", 0x2000, 0x0400, CRC(bc50631f) SHA1(6aa3124d09fc4e369d087a5ad6dd1737ace55e41))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("440.snd", 0x0400, 0x0400, CRC(4a0a05ae) SHA1(88f21b5638494d8e78dc0b6b7d69873b76b5f75d))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Sahara Love (1984)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Sinbad (05/1978) #412
/-------------------------------------------------------------------*/
ROM_START(sinbad)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("412.cpu", 0x2000, 0x0400, CRC(84a86b83) SHA1(f331f2ffd7d1b279b4ffbb939aa8649e723f5fac))
ROM_END

ROM_START(sinbadn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("412no1.cpu", 0x2000, 0x0400, CRC(f5373f5f) SHA1(027840501416ff01b2adf07188c7d667adf3ad5f))
ROM_END

/*-------------------------------------------------------------------
/ Sky Warrior (1983)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Solar Ride (02/1979) #421
/-------------------------------------------------------------------*/
ROM_START(solaride)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("421.cpu", 0x2000, 0x0400, CRC(6b5c5da6) SHA1(a09b7009473be53586f53f48b7bfed9a0c5ecd55))
ROM_END

/*-------------------------------------------------------------------
/ The Incredible Hulk (10/1979) #433
/-------------------------------------------------------------------*/
ROM_START(hulk)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("433.cpu", 0x2000, 0x0400, CRC(c05d2b52) SHA1(393fe063b029246317c90ee384db95a84d61dbb7))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("433.snd", 0x0400, 0x0400, CRC(20cd1dff) SHA1(93e7c47ff7051c3c0dc9f8f95aa33ba094e7cf25))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Torch (02/1980) #438
/-------------------------------------------------------------------*/
ROM_START(torch)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("438.cpu", 0x2000, 0x0400, CRC(2d396a64) SHA1(38a1862771500faa471071db08dfbadc6e8759e8))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("438.snd", 0x0400, 0x0400, CRC(a9619b48) SHA1(1906bc1b059bf31082e3b4546f5a30159479ad3c))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Totem (10/1979) #429
/-------------------------------------------------------------------*/
ROM_START(totem)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("429.cpu", 0x2000, 0x0400, CRC(7885a384) SHA1(1770662af7d48ad8297097a9877c5c497119978d))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("429.snd", 0x0400, 0x0400, CRC(5d1b7ed4) SHA1(4a584f880e907fb21da78f3b3a0617f20599688f))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ System 1 Test prom
/-------------------------------------------------------------------*/
ROM_START(sys1test)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("test.cpu", 0x2000, 0x0400, CRC(8b0704bb) SHA1(5f0eb8d5af867b815b6012c9d078927398efe6d8))
ROM_END


GAME(1977,  gts1,       0,          gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "System 1", GAME_IS_BIOS_ROOT)

//Exact same roms as gts1 with added hardware we'll likely need roms for to emulate properly
GAME(1979,  gts1s,      gts1,       gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "System 1 with sound board", GAME_IS_BIOS_ROOT)
GAME(19??,  sys1test,   gts1,       gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "System 1 Test prom",                   GAME_IS_SKELETON_MECHANICAL)

// chimes
GAME(1977,  cleoptra,   gts1,       gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "Cleopatra",                            GAME_IS_SKELETON_MECHANICAL)
GAME(1978,  sinbad,     gts1,       gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "Sinbad",                               GAME_IS_SKELETON_MECHANICAL)
GAME(1978,  sinbadn,    sinbad,     gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "Sinbad (Norway)",                      GAME_IS_SKELETON_MECHANICAL)
GAME(1978,  jokrpokr,   gts1,       gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "Joker Poker",                          GAME_IS_SKELETON_MECHANICAL)
GAME(1978,  dragon,     gts1,       gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "Dragon",                               GAME_IS_SKELETON_MECHANICAL)
GAME(1979,  solaride,   gts1,       gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "Solar Ride",                           GAME_IS_SKELETON_MECHANICAL)
GAME(1979,  countdwn,   gts1,       gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "Count-Down",                           GAME_IS_SKELETON_MECHANICAL)

// NE555 beeper
GAME(1978,  closeenc,   gts1,       gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "Close Encounters of the Third Kind",   GAME_IS_SKELETON_MECHANICAL)
GAME(1978,  charlies,   gts1,       gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "Charlie's Angels",                     GAME_IS_SKELETON_MECHANICAL)
GAME(1979,  pinpool,    gts1,       gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "Pinball Pool",                         GAME_IS_SKELETON_MECHANICAL)

// sound card
GAME(1979,  totem,      gts1s,      gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "Totem",                                GAME_IS_SKELETON_MECHANICAL)
GAME(1979,  hulk,       gts1s,      gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "Incredible Hulk,The",                  GAME_IS_SKELETON_MECHANICAL)
GAME(1979,  geniep,     gts1s,      gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "Genie (Pinball)",                              GAME_IS_SKELETON_MECHANICAL)
GAME(1980,  buckrgrs,   gts1s,      gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "Buck Rogers",                          GAME_IS_SKELETON_MECHANICAL)
GAME(1980,  torch,      gts1s,      gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "Torch",                                GAME_IS_SKELETON_MECHANICAL)
GAME(1980,  roldisco,   gts1s,      gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "Roller Disco",                         GAME_IS_SKELETON_MECHANICAL)
GAME(1980,  astannie,   gts1s,      gts1,   gts1, gts1_state,   gts1,   ROT0,   "Gottlieb",     "Asteroid Annie and the Aliens",        GAME_IS_SKELETON_MECHANICAL)

// homebrew
GAME(1986,  hexagone,   gts1s,      gts1,   gts1, gts1_state,   gts1,   ROT0,   "Christian Tabart (France)",        "L'Hexagone",       GAME_IS_SKELETON_MECHANICAL)
