// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/****************************************************************************

    TourVision
    Driver by Mariusz Wojcieszek and Stephh

    Bootleg PC-Engine based arcade board from the Spanish company TourVision.
    Two known hardware revisions, one with a sub-board with the PC-Engine chipset
    and the other as an integrated PCB.

    Todo: complete jamma interface emulation.

    By now, six known BIOS versions, U4-52 (dumped from a board with-subboard PCB),
    U4-55 (dumped from an integrated PCB) and U4-60 (dumped from a board with-subboard PCB).

    Known games (followed by game ID, some are duplicate):

    1943 Kai (65)
    Aero Blaster (32)
    After Burner II (46)
  * Ankoku Densetu (Legendary Axe II)
    Armed-F (?)
    Ballistix (186)
  * Batman
    Be Ball (93)
  * Blodia
    Bomberman (71)
    Bull Fight (185)
    Chozetsurinjin Beraboh Man (Super Foolish Man) (27)
    Chuka Taisen (37)
    Columns (90)
    Coryoon (43)
  * Cross Wiber
    Cyber Core (13)
    Daisempuu (3)
    Dead Moon (?)
    Devil Crash (47)
  * Die Hard
    Dodge Ball (194)
    Doraemon Meikyuu Daisakusen (20)
  * Doraemon II
    Down Load (43)
  * Dragon Egg!
    Dragon Saber (65)
    Dragon Spirit (?)
    Dungeon Explorer (?)
  * F1 Triple Battle
    Final Blaster (29)
    Final Lap Twin (79)
    Final Match Tennis (62)
    Formation Soccer (1)
    Gomola Speed (27)
    Gunhed (148)
    Hana Taka Daka (Super Long Nose Goblin) (6)
  * Hatris
    Jackie Chan (54)
    Jinmu Densho (19)
    Kiki Kaikai (120)
    Legend Of Hero Tomna (56)
    Makyo Densetsu - The Legenary Axe (?)
    Mizubaku Daibouken Liquid Kids (10) (marketed as "Parasol Stars II")
    Mr. Heli (23)
    Ninja Ryukenden (10)
    Operation Wolf (26)
  * Out Run
    Override (53)
    Pac-Land (16)
  * Paranoia (18)
  * PC Genjin
    PC Genjin 2 (84)
    PC Denjin Punkic Cyborg (201)
    Power Drift (200)
    Power Eleven (83)
  * Power Golf
    Power League IV (?)
  * Power Sports
    Pro Yakyuu World Stadium '91 (192)
    Psycho Chaser (14)
    Puzzle Boy (57)
  * Puzznic
    R-Type II (61)
  * Rabio Lepus Special
    Raiden (111)
    Rastan Saga II (?)
    Saigo no Nindou (44)
    Salamander (184)
    Shinobi (5)
    Side Arms (2)
    Skweek (89)
  * Soldier Blade
    Son Son II (80)
    Special Criminal Investigation (?)
    Super Star Soldier (42)
    Super Volley ball (?)
    Tatsujin (31)
    Terra Cresta II (27)
    Thunder Blade (?)
  * Tiger Road
  * Titan
    Toy Shop Boys (51)
  * Tricky
  * TV Sports
    USA Pro Basketball (?)
    Veigues (40)
    Vigilante (8)
    Volfied (68)
    W-Ring (21)
    Winning Shot (28)
    Xevious (?)

* Denotes Not Dumped

 _______________________________________________________________________________________________________________________________________________
|                                                                                                                                               |
|                                           ____________               ____________               ____________               ____________       |
|                                          |T74LS125AB1|  ____        |T74LS125AB1|  ____        |T74LS125AB1|  ____        |T74LS125AB1|  ____ |
|                                          -------------  |  |        -------------  |  |        -------------  |  |        -------------  |  | |
|                                           ____________  |  |         ____________  |  |         ____________  |  |         ____________  |  | |
|                                          | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  | |
|                                          -------------  |  |        -------------  |  |        -------------  |  |        -------------  |  | |
|                                           ____________  |  |         ____________  |  |         ____________  |  |         ____________  |  | |
|       ___________   ____________         | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  | |
|      |4116R-001  | |X74LS32B1  |         -------------  |  |        -------------  |  |        -------------  |  |        -------------  |  | |
|      ------------- -------------                        |JP|                       |JP|                       |JP|                       |JP| |
|       ___________                                       | 4|                       | 3|                       | 2|                       | 1| |
|__     ___________                         ____________  |  |         ____________  |  |         ____________  |  |         ____________  |  | |
   |    ____________  ____________         | 74LS244N  |  |  |        | 74LS244N  |  |  |        | 74LS244N  |  |  |        | 74LS244N  |  |  | |
 __|   |SN74LS257SN| |4116R-001  |         -------------  |  |        -------------  |  |        -------------  |  |        -------------  |  | |
|__    ------------- -------------          ____________  |  |         ____________  |  |         ____________  |  |         ____________  |  | |
|__     ____________  ____________         | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  | |
|__    | 74LS157N  | | 74LS157N  |         -------------  |  |        -------------  |  |        -------------  |  |        -------------  |  | |
|__    ------------- -------------          ____________  |  |         ____________  |  |         ____________  |  |         ____________  |  | |
|__     ____________  ____________         | 74LS244N  |  ----        | 74LS244N  |  ----        | 74LS244N  |  ----        | 74LS244N  |  ---- |
|__    | SN74LS08N | | SN74LS08N |         -------------              -------------              -------------              -------------       |
|__    ------------- -------------                                                                                                              |
|__     ____________  ____________                                                                                            ____________      |
|__    | SN74LS08N | | SN74LS08N |                                                                                            |HSRM2264LM10     |
|__    ------------- -------------                                                                                            |__________|      |
|__     ____________  ____________                                                                                                              |
|__    | 74LS138N  | | 74LS138N  |                                                                                                              |
|__    ------------- -------------                                                                                                              |
|__     ____________  ____________  ____________   ____________   ____________  ____________  ____________                                      |
|__    |  74LS244N | |  74LS244N |  |T74LS32B1 |   |MC14017BCP|  | T74LS14B1 | | GD74LS393 | |T74LS125AB1|                                      |
|__    ------------- -------------  ------------   ------------  ------------- ------------  -------------                                      |
|__      .........     .........    ____________                                                                                                |
|__      .........     .........    | 74LS138N |                     _________                                                                  |
|__     ___________   ___________   ------------                     |       |                                      ______                      |
|__    |4116R-001  | |4116R-001  |  _________________                |       |                                     | HU  |        HSRM20256LM12 |
|__    ------------- ------------- |                |                |  BT1  |                                     |C6270|          ___         |
|__      __________  ____________  | NEC D4465C     |                |       |                                     |     |         |  |         |
|__     | TC4011BP|  |SN74LS373N | |________________|                |_______|                                     |_____|         |  |         |
|__     -----------  -------------  _________________                                                                              |__|         |
|__    ..... ___________     ____  | TOURVISION BIOS|                                                                                           |
|__         | 74LS138N |     |XT1| |                |                                                     ______     ______          ___        |
   |        ------------     |___| |________________|               ____                                 | HU  |    | HU  |         |  |        |
 __|    IC_  _____________                   ________              |    |                                |C6280A    |C6260A         |  |        |
|      |36| |  74LS244N  |  _________________________              | PT |                                |     |    |     |         |__|        |
|      ---- -------------  |                        |              |____|                                |_____|    |_____|                     |
|  ________  _____________ | NEC D8085AHC           |    ___________   ___________   _______                                       HSRM20256LM12|
| | DIP 2 | |  74LS244N  | |________________________|   | T74LS14B1|  |MC14001BCP|  |LM393N|                          ________                  |
| --------- --------------  _________________________   ------------  ------------  --------                         |D74HCU04C                 |
|  ________  _____________ |                        |    ___________                                                 ---------                  |
|  ________ |  74LS244N  | | NEC D8155HC            |   |  7407N   |                                             ____                           |
| | DIP 1 | -------------- |________________________|   ------------                                             |XT2|                          |
| ---------          ____                _____________         _____________                                     |___|                          |
|                    JP107              | JP 106     |        | JP 105     |                                                                    |
|                    ----               --------------        --------------                                                                    |
|_______________________________________________________________________________________________________________________________________________|

IC36  = ST NE 555N 99201
XT1   = 6144 KSS1H
JP107 = 2-pin connector
JP106 = 14-pin connector to 2-digit 7 segments display
JP105 = 16-pin connector (unknown functionality)
PT    = Push-type switch
BT1   = 3.6 V battery
XT2   = 21.32825 MHz UNI 90-H
JP1-4 = Carts slots

Games are dumped directly from the cartridge edge connector using the following adapter:

 ----------------------------------------------------------------------------
 Cartridge pinout
 ----------------------------------------------------------------------------

                       +----------+
                (N.C.) |50      49| +5V
                   +5V |48      47| +5V
                   A18 |46      45| +5V
                   A14 |44      43| A17
                    A8 |42      41| A13
                   A11 |40      39| A9
                   A10 |38      37| OE#
                    D7 |36      35| CE#
(front of           D5 |34      33| D6               (rear of
 cartridge)         D3 |32      31| D4                cartridge)
                    D2 |30      29| GND
                    D0 |28      27| D1
                    A1 |26      25| A0
                    A3 |24      23| A2
                    A5 |22      21| A4
                    A7 |20      19| A6
                   A15 |18      17| A12
                   A19 |16      15| A16
                   GND |14      13| (N.C.)
                   GND |12      11| GND)
                 (KEY) |10------09| (KEY)
                   ID7 |08      07| ID6
                   ID5 |06      05| ID4
                   ID3 |04      03| ID2
                   ID1 |02      01| ID0
                       +----------+

 ----------------------------------------------------------------------------
 27C080 pinout
 ----------------------------------------------------------------------------
                        +----v----+
                    A19 | 1     32| +5V
                    A16 | 2     31| A18
                    A15 | 3     30| A17
                    A12 | 4     29| A14
                     A7 | 5     28| A13
                     A6 | 6     27| A8
                     A5 | 7     26| A9
                     A4 | 8     25| A11
                     A3 | 9     24| OE#
                     A2 |10     23| A10
                     A1 |11     22| CE#
                     A0 |12     21| D7
                     D0 |13     20| D6
                     D1 |14     19| D5
                     D2 |15     18| D4
                    GND |16     17| D3
                        +---------+


Stephh notes for 8085 code:
0xe01d : game slot number (range 0-3) - sometimes inc'ed/dec'ed/zeroed, but also filled based on games slot status (code at 0x01e8) :
0x8009 and 0x800a : main timer (BCD, LSB first)

0xe054 to 0xe057 : display timer (main, LSdigit first, stored in 4 lower bits)
0xe058 to 0xe05b : display timer (game slot 1, LSdigit first, stored in 4 upper bits)
0xe05c to 0xe05f : display timer (game slot 2, LSdigit first, stored in 4 lower bits)
0xe060 to 0xe063 : display timer (game slot 3, LSdigit first, stored in 4 upper bits)
0xe064 to 0xe067 : display timer (game slot 4, LSdigit first, stored in 4 lower bits)

display timer (main) "filled" with code at 0x054e
display timer (game slot n) "filled" with code at 0x04e3

coin insertion routine at 0x0273
coin 1 triggers code at 0x02d7
coin 2 triggers code at 0x028f

in each coin insertion routine, you need to insert n coins (based on DSW settings) then you are awarded u units of time (also based on DSW settings)
I can't tell ATM if units are seconds (even if values in tables seem very related to them)

****************************************************************************
Notes from system11:
Game ID is configured on carts using pins 1 -> 8, these form a single byte integer, known IDs are tagged above.  IDs can be shared between games.  Game handling is defined based on data tables in the BIOS, using offset + ID*data block size.  Data block starts in 6.0 BIOS as follows:

1184   1284   1384   1484   1584   3584   5d84   5684

For full information see:
http://blog.system11.org/?p=1943

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/pcecommn.h"
#include "video/huc6260.h"
#include "video/huc6270.h"
#include "cpu/h6280/h6280.h"
#include "sound/c6280.h"
#include "machine/i8155.h"
#include "softlist.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

class tourvision_state : public pce_common_state
{
public:
	tourvision_state(const machine_config &mconfig, device_type type, const char *tag)
		: pce_common_state(mconfig, type, tag),
		m_subcpu(*this, "subcpu"),
		m_cart(*this, "cartslot")
		{ }

	DECLARE_WRITE8_MEMBER(tourvision_8085_d000_w);
	DECLARE_WRITE8_MEMBER(tourvision_i8155_a_w);
	DECLARE_WRITE8_MEMBER(tourvision_i8155_b_w);
	DECLARE_WRITE8_MEMBER(tourvision_i8155_c_w);
	DECLARE_WRITE_LINE_MEMBER(tourvision_timer_out);
	DECLARE_WRITE_LINE_MEMBER(pce_irq_changed);
	required_device<cpu_device> m_subcpu;
	required_device<generic_slot_device> m_cart;
	UINT32  m_rom_size;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(tourvision_cart);
};

DEVICE_IMAGE_LOAD_MEMBER( tourvision_state, tourvision_cart )
{
	m_rom_size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(m_rom_size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), m_rom_size, "rom");

	UINT8* rgn = memregion("maincpu")->base();
	UINT8* base = m_cart->get_rom_base();

	if (m_rom_size == 0x0c0000)
	{
		memcpy(rgn+0x000000, base+0x000000, 0x0c0000 );
		memcpy(rgn+0x0c0000, base+0x080000, 0x040000 );
	}
	else
	if (m_rom_size == 0x060000)
	{
		memcpy(rgn+0x000000, base+0x000000, 0x040000 );
		memcpy(rgn+0x040000, base+0x000000, 0x040000 );
		memcpy(rgn+0x080000, base+0x040000, 0x020000 );
		memcpy(rgn+0x0a0000, base+0x040000, 0x020000 );
		memcpy(rgn+0x0c0000, base+0x040000, 0x020000 );
		memcpy(rgn+0x0e0000, base+0x040000, 0x020000 );
	}
	else
	{
		for (int i=0;i<0x100000;i+=m_rom_size)
			memcpy(rgn+i, base+0x000000, m_rom_size );
	}

#if 0
	{
		FILE *fp;
		fp=fopen("tourvision.bin", "w+b");
		if (fp)
		{
			fwrite(rgn, 0x100000, 1, fp);
			fclose(fp);
		}
	}
#endif

	return IMAGE_INIT_PASS;
}

/* note from system11 - this system actually supports 2 players */

static INPUT_PORTS_START( tourvision )
	PORT_START( "JOY" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* button I */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* button II */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) /* select */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) /* run */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )

	PORT_START( "DSW1" )
	PORT_DIPNAME( 0x07, 0x07, "Coins needed 1" )
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0x78, 0x78, "Time units 1" )
	PORT_DIPSETTING(    0x78, "900" )
	PORT_DIPSETTING(    0x70, "720" )
	PORT_DIPSETTING(    0x68, "600" )
	PORT_DIPSETTING(    0x60, "540" )
	PORT_DIPSETTING(    0x58, "480" )
	PORT_DIPSETTING(    0x50, "420" )
	PORT_DIPSETTING(    0x48, "360" )
	PORT_DIPSETTING(    0x40, "300" )
	PORT_DIPSETTING(    0x38, "270" )
	PORT_DIPSETTING(    0x30, "240" )
	PORT_DIPSETTING(    0x28, "210" )
	PORT_DIPSETTING(    0x20, "180" )
	PORT_DIPSETTING(    0x18, "150" )
	PORT_DIPSETTING(    0x10, "120" )
	PORT_DIPSETTING(    0x08, "90" )
	PORT_DIPSETTING(    0x00, "60" )
		PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )

	PORT_START( "DSW2" )
	PORT_DIPNAME( 0x03, 0x03, "Coins needed 2" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x78, 0x78, "Time units 2" )
	PORT_DIPSETTING(    0x78, "1500" )
	PORT_DIPSETTING(    0x70, "1200" )
	PORT_DIPSETTING(    0x68, "1080" )
	PORT_DIPSETTING(    0x60, "960" )
	PORT_DIPSETTING(    0x58, "900" )
	PORT_DIPSETTING(    0x50, "840" )
	PORT_DIPSETTING(    0x48, "780" )
	PORT_DIPSETTING(    0x40, "720" )
	PORT_DIPSETTING(    0x38, "660" )
	PORT_DIPSETTING(    0x30, "600" )
	PORT_DIPSETTING(    0x28, "540" )
	PORT_DIPSETTING(    0x20, "480" )
	PORT_DIPSETTING(    0x18, "420" )
	PORT_DIPSETTING(    0x10, "360" )
	PORT_DIPSETTING(    0x08, "300" )
	PORT_DIPSETTING(    0x00, "240" )
	PORT_DIPUNKNOWN( 0x80, 0x00 )
// SW2 bit 7 might be "free play" HIGH and when "Coins needed 2" is set to "1" (multiple comparisons with 0x83) in BIOS0 and BIOS1.
// In BIOS2, "Coins needed 2" can be set to anything (multiple comparisons with 0x80) instead.
// Of course, it can also be sort of "Test mode" or "Debug mode" ...

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) // games slot status in bits 3 to 7
INPUT_PORTS_END

static ADDRESS_MAP_START( pce_mem , AS_PROGRAM, 8, tourvision_state )
	AM_RANGE( 0x000000, 0x0FFFFF) AM_ROM
	AM_RANGE( 0x1F0000, 0x1F1FFF) AM_RAM AM_MIRROR(0x6000)
	AM_RANGE( 0x1FE000, 0x1FE3FF) AM_DEVREADWRITE( "huc6270", huc6270_device, read, write )
	AM_RANGE( 0x1FE400, 0x1FE7FF) AM_DEVREADWRITE( "huc6260", huc6260_device, read, write )
	AM_RANGE( 0x1FE800, 0x1FEBFF) AM_DEVREADWRITE("c6280", c6280_device, c6280_r, c6280_w )
	AM_RANGE( 0x1FEC00, 0x1FEFFF) AM_DEVREADWRITE("maincpu", h6280_device, timer_r, timer_w )
	AM_RANGE( 0x1FF000, 0x1FF3FF) AM_READWRITE(pce_joystick_r, pce_joystick_w )
	AM_RANGE( 0x1FF400, 0x1FF7FF) AM_DEVREADWRITE("maincpu", h6280_device, irq_status_r, irq_status_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( pce_io , AS_IO, 8, tourvision_state )
	AM_RANGE( 0x00, 0x03) AM_DEVREADWRITE( "huc6270", huc6270_device, read, write )
ADDRESS_MAP_END

WRITE8_MEMBER(tourvision_state::tourvision_8085_d000_w)
{
	//logerror( "D000 (8085) write %02x\n", data );
}

static ADDRESS_MAP_START(tourvision_8085_map, AS_PROGRAM, 8, tourvision_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x80ff) AM_DEVREADWRITE("i8155", i8155_device, memory_r, memory_w)
	AM_RANGE(0x8100, 0x8107) AM_DEVREADWRITE("i8155", i8155_device, io_r, io_w)
	AM_RANGE(0x9000, 0x9000) AM_READ_PORT("DSW1")
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("DSW2")
	AM_RANGE(0xb000, 0xb000) AM_READNOP // unknown (must NOT be == 0x03 ? code at 0x1154)
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xd000, 0xd000) AM_WRITE(tourvision_8085_d000_w )
	AM_RANGE(0xe000, 0xe1ff) AM_RAM
	AM_RANGE(0xf000, 0xf000) AM_READNOP // protection or internal counter ? there is sometimes some data in BIOS0 which is replaced by 0xff in BIOS1
ADDRESS_MAP_END

WRITE8_MEMBER(tourvision_state::tourvision_i8155_a_w)
{
	//logerror("i8155 Port A: %02X\n", data);
}

WRITE8_MEMBER(tourvision_state::tourvision_i8155_b_w)
{
	// Selects game slot in bits 0 - 1
	//logerror("i8155 Port B: %02X\n", data);
}

WRITE8_MEMBER(tourvision_state::tourvision_i8155_c_w)
{
	//logerror("i8155 Port C: %02X\n", data);
}

WRITE_LINE_MEMBER(tourvision_state::tourvision_timer_out)
{
	m_subcpu->set_input_line(I8085_RST55_LINE, state ? CLEAR_LINE : ASSERT_LINE );
	//logerror("Timer out %d\n", state);
}

WRITE_LINE_MEMBER(tourvision_state::pce_irq_changed)
{
	m_maincpu->set_input_line(0, state);
}

static MACHINE_CONFIG_START( tourvision, tourvision_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", H6280, PCE_MAIN_CLOCK/3)
	MCFG_CPU_PROGRAM_MAP(pce_mem)
	MCFG_CPU_IO_MAP(pce_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_CPU_ADD("subcpu", I8085A, 18000000/3 /*?*/)
	MCFG_CPU_PROGRAM_MAP(tourvision_8085_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PCE_MAIN_CLOCK, HUC6260_WPF, 64, 64 + 1024 + 64, HUC6260_LPF, 18, 18 + 242)
	MCFG_SCREEN_UPDATE_DRIVER( pce_common_state, screen_update )
	MCFG_SCREEN_PALETTE("huc6260:palette")

	MCFG_DEVICE_ADD( "huc6260", HUC6260, PCE_MAIN_CLOCK )
	MCFG_HUC6260_NEXT_PIXEL_DATA_CB(DEVREAD16("huc6270", huc6270_device, next_pixel))
	MCFG_HUC6260_TIME_TIL_NEXT_EVENT_CB(DEVREAD16("huc6270", huc6270_device, time_until_next_event))
	MCFG_HUC6260_VSYNC_CHANGED_CB(DEVWRITELINE("huc6270", huc6270_device, vsync_changed))
	MCFG_HUC6260_HSYNC_CHANGED_CB(DEVWRITELINE("huc6270", huc6270_device, hsync_changed))
	MCFG_DEVICE_ADD( "huc6270", HUC6270, 0 )
	MCFG_HUC6270_VRAM_SIZE(0x10000)
	MCFG_HUC6270_IRQ_CHANGED_CB(WRITELINE(tourvision_state, pce_irq_changed))

	MCFG_DEVICE_ADD("i8155", I8155, 1000000 /*?*/)
	MCFG_I8155_OUT_PORTA_CB(WRITE8(tourvision_state, tourvision_i8155_a_w))
	MCFG_I8155_OUT_PORTB_CB(WRITE8(tourvision_state, tourvision_i8155_b_w))
	MCFG_I8155_OUT_PORTC_CB(WRITE8(tourvision_state, tourvision_i8155_c_w))
	MCFG_I8155_OUT_TIMEROUT_CB(WRITELINE(tourvision_state, tourvision_timer_out))

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker","rspeaker")
	MCFG_SOUND_ADD("c6280", C6280, PCE_MAIN_CLOCK/6)
	MCFG_C6280_CPU("maincpu")
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "tourvision_cart")
	MCFG_GENERIC_EXTENSIONS("bin")
	MCFG_GENERIC_LOAD(tourvision_state, tourvision_cart)
	MCFG_GENERIC_MANDATORY

	MCFG_SOFTWARE_LIST_ADD("tv_list","pce_tourvision")


MACHINE_CONFIG_END

#define TOURVISION_BIOS \
	ROM_REGION( 0x8000, "subcpu", 0 ) \
	ROM_SYSTEM_BIOS( 0, "60", "U4-60" ) \
	ROMX_LOAD( "u4-60.ic29", 0x0000, 0x8000, CRC(1fd27e22) SHA1(b103d365eac3fa447c2e9addddf6974b4403ed41), ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS( 1, "55", "U4-55" ) \
	ROMX_LOAD( "u4-55.ic29", 0x0000, 0x8000, CRC(87cf66c1) SHA1(d6b42137be7a07a0e299c2d922328a6a9a2b7b8f), ROM_BIOS(2) ) \
	ROM_SYSTEM_BIOS( 2, "53", "U4-53" ) \
	ROMX_LOAD( "u4-53.ic29", 0x0000, 0x8000, CRC(bccb53c9) SHA1(a27113d70cf348c7eafa39fc7a76f55f63723ad7), ROM_BIOS(3) ) \
	ROM_SYSTEM_BIOS( 3, "52", "U4-52" ) \
	ROMX_LOAD( "u4-52.ic29", 0x0000, 0x8000, CRC(ffd7b0fe) SHA1(d1804865c91e925a01b05cf441e8458a3db23f50), ROM_BIOS(4) ) \
	ROM_SYSTEM_BIOS( 4, "43", "U4-43" ) \
	ROMX_LOAD( "u4-43.ic29", 0x0000, 0x8000, CRC(88da23f3) SHA1(9d24faa116129783e55c7f79a4a08902a236d5a6), ROM_BIOS(5) ) \
	ROM_SYSTEM_BIOS( 5, "40", "U4-40" ) \
	ROMX_LOAD( "u4-40.ic29", 0x0000, 0x8000, CRC(ba6290cc) SHA1(92b0e9f55791e892ec209de4fadd80faef370622), ROM_BIOS(6) )


ROM_START(tourvis)
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )

	TOURVISION_BIOS /* BIOS rom type is 27C256 */
ROM_END


GAME( 19??, tourvis,  0,       tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision)",                                      "Tourvision PCE bootleg", MACHINE_IS_BIOS_ROOT | MACHINE_NOT_WORKING )
