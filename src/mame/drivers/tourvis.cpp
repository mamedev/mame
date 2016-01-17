// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, Stephh
/****************************************************************************

    TourVision
    Driver by Mariusz Wojcieszek and Stephh

    Bootleg PC-Engine based arcade board from the Spanish company TourVision.
    Two known hardware revisions, one with a sub-board with the PC-Engine chipset
    and the other as an integrated PCB.

    Todo: complete jamma interface emulation.

    By now, three known BIOS versions, U4-52 (dumped from a board with-subboard PCB),
    U4-55 (dumped from an integrated PCB) and U4-60 (dumped from a board with-subboard PCB).

    Known games:

    1943
    Aero Blaster
    After Burner
  * Ankoku Densetu
    Armed-F
    Ballistix
    Be Ball
    Bomberman
    Chōzetsurinjin Beraboh Man (Super Foolish Man)
    Chuka Taisen
    Columns
    Coryoon
  * Cross Wiber
  * Cyber Core
    Daisempuu
    Dead Moon
    Devil Crash
  * Die Hard
    Dodge Ball
  * Doraemon Meikyuu Daisakusen
  * Doramon II
  * Down Load
    Dragon Spirit
    Dungeon Explorer
    Final Blaster
    Final Lap Twin
    Final Match Tennis
    Formation Soccer
    Gomola Speed
    Gunhed
    Hana Taka Daka (Super Long Nose Goblin)
    Jackie Chan
    Jinmu Densho
    Kiki Kaikai
    Legend Of Hero Tomna
    Makyo Densetsu - The Legenary Axe
    Mizubaku Daibouken Liquid Kids
    Mr. Heli
    Ninja Ryukenden
    Operation Wolf
  * Out Run
    Override
    Pac-Land
  * PC Genjin
    PC Genjin 2
    PC Denjin Punkic Cyborg
    Power Drift
    Power Eleven
  * Power Golf
    Power League IV
  * Power Sports
    Pro Yakyuu World Stadium '91
    Psycho Chaser
    Puzzle Boy
    R-Type II
    Rastan Saga II
    Saigo no Nindou
    Salamander
    Shinobi
    Side Arms
    Skweek
  * Soldier Blade
    Son Son II
    Special Criminal Investigation
    Super Star Soldier
    Super Volley ball
    Tatsujin
    Terra Cresta II
    Thunder Blade
  * Tiger Road
    Toy Shop Boys
  * Tricky
  * TV Sports
    USA Pro Basketball
    Veigues
  * Vigilante
  # Volfied
    W-Ring
    Winning Shot
    Xevious

* Denotes Not Dumped
# Denotes Redump Needed

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
                (N.C.) |01      01| +5V
                   +5V |02      02| +5V
                   A18 |03      03| +5V
                   A14 |04      04| A17
                    A8 |05      05| A13
                   A11 |06      06| A9
                   A10 |07      07| OE#
                    D7 |08      08| CE#
(front of           D5 |09      09| D6               (rear of
 cartridge)         D3 |10      10| D4                cartridge)
                    D2 |11      11| GND
                    D0 |12      12| D1
                    A1 |13      13| A0
                    A3 |14      14| A2
                    A5 |15      15| A4
                    A7 |16      16| A6
                   A15 |17      17| A12
                   A19 |18      18| A16
                   GND |19      19| (N.C.)
                   GND |20      20| (N.C.)
                 (KEY) |21------21| (KEY)
                (N.C.) |22      22| (N.C.)
                (N.C.) |23      23| (N.C.)
                (N.C.) |24      24| (N.C.)
                (N.C.) |25      25| (N.C.)
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

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/pcecommn.h"
#include "video/huc6260.h"
#include "video/huc6270.h"
#include "cpu/h6280/h6280.h"
#include "sound/c6280.h"
#include "machine/i8155.h"

class tourvision_state : public pce_common_state
{
public:
	tourvision_state(const machine_config &mconfig, device_type type, std::string tag)
		: pce_common_state(mconfig, type, tag),
		m_subcpu(*this, "subcpu") { }

	DECLARE_WRITE8_MEMBER(tourvision_8085_d000_w);
	DECLARE_WRITE8_MEMBER(tourvision_i8155_a_w);
	DECLARE_WRITE8_MEMBER(tourvision_i8155_b_w);
	DECLARE_WRITE8_MEMBER(tourvision_i8155_c_w);
	DECLARE_WRITE_LINE_MEMBER(tourvision_timer_out);
	DECLARE_WRITE_LINE_MEMBER(pce_irq_changed);
	required_device<cpu_device> m_subcpu;
};


static INPUT_PORTS_START( tourvision )
	PORT_START( "JOY" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* button I */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* button II */
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
	PORT_DIPUNKNOWN( 0x80, 0x00 )
// I can't tell what DSW1 bit 7 is really supposed to do, but it has an effect only when no "Free Play" and [0x8005] = [0x8006] = 0 (code at 0x0a58) :
// Since these conditions seem to be true only in "attract mode" when there is no time left, this bit could enable/disable sounds.

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
	ROMX_LOAD( "u4-43.ic29", 0x0000, 0x8000, CRC(88da23f3) SHA1(9d24faa116129783e55c7f79a4a08902a236d5a6), ROM_BIOS(5) )


ROM_START(tourvis)
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )

	TOURVISION_BIOS /* BIOS rom type is 27C256 */
ROM_END



/* 1943 Kai */ 
ROM_START(tv1943)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_1943_kia.bin", 0x00000, 0x100000, CRC(de4672ab) SHA1(2da1ee082bfb920c632a95014208f11fb48c58e1) )

	TOURVISION_BIOS
ROM_END

/* Aero Blasters - Hudson / Kaneko */ 
ROM_START(tvablast)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_ablast.bin", 0x00000, 0x100000, CRC(9302f6d0) SHA1(76ef27a6d639514ed261b9d65f37217f2989d1c0) )

	TOURVISION_BIOS
ROM_END

/* After Burner */ 
ROM_START(tvaburn)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_afterburner.bin", 0x00000, 0x100000, CRC(5ce31322) SHA1(08918d443891bd70f1b0b0c739522b764b16bc96) )

	TOURVISION_BIOS
ROM_END

/* Armed-F */ 
ROM_START(tvarmedf)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_armed-f.bin", 0x00000, 0x100000, CRC(056617f5) SHA1(d10eb80b8436b8d217170309647104181cca750a) )

	TOURVISION_BIOS
ROM_END

/* Ballistix */ 
ROM_START(tvbalstx)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_ballistix.bin", 0x00000, 0x100000, CRC(9d32ed98) SHA1(404cc3695940a7fdc802ac166ec564a858a894d0) )

	TOURVISION_BIOS
ROM_END

/* Be Ball */ 
ROM_START(tvbeball)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_be_ball.bin", 0x00000, 0x100000, CRC(4b1e2861) SHA1(bea449543284bb6f4b33b1fb4156cd18a782ad6a) )

	TOURVISION_BIOS
ROM_END

/* Bomberman */ 
ROM_START(tvbomber)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_bomberman.bin", 0x00000, 0x100000, CRC(cfcabe78) SHA1(bdd1766fad43c6c76e1b0d6e8b4f0ba3363442d6) )

	TOURVISION_BIOS
ROM_END

/* Chōzetsurinjin Beraboh Man (Super Foolist Man) */ 
ROM_START(tvbrabho)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_chozetsurinjin_beraboh_man.bin", 0x00000, 0x100000, CRC(1f80cf04) SHA1(121bfb9ba4de4d047b08442d900b7f351210dd48) )

	TOURVISION_BIOS
ROM_END

/* Chuka Taisen */ 
ROM_START(tvtaisen)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_chuka_taisen.bin", 0x00000, 0x100000, CRC(3b9e9185) SHA1(96f9f82a9fa6ee2b92c0294e71d47886e27fdc06) )

	TOURVISION_BIOS
ROM_END

/* Columns - Telenet Japan */
ROM_START(tvcolumn)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_column.bin", 0x00000, 0x100000, CRC(bb01dea8) SHA1(24e00aee5117e996becb56b59851e54e3f2fa11f) )

	TOURVISION_BIOS
ROM_END

/* Coryoon */ 
ROM_START(tvcoryon)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_corycoon.bin", 0x00000, 0x100000, CRC(c377db91) SHA1(1585d886f775ed361b2558839e544660533e9297) )

	TOURVISION_BIOS
ROM_END

/* Daisenpu */ 
ROM_START(tvdsenpu)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_daisenpu.bin", 0x00000, 0x100000, CRC(5a8cef75) SHA1(00f27127114e4f5bf69c81212e66948caaec755d) )

	TOURVISION_BIOS
ROM_END

/* Dead Moon */ 
ROM_START(tvdmoon)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_dead_moon.bin", 0x00000, 0x100000, CRC(b54793c1) SHA1(8899947092d9a02f3be61ac9c293642e83a015ec) )

	TOURVISION_BIOS
ROM_END

/* Devil Crash */ 
ROM_START(tvdevilc)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_devil_crash.bin", 0x00000, 0x100000, CRC(c163e5c1) SHA1(2134b3943df87af556694dbe6c77b30723f9175a) )

	TOURVISION_BIOS
ROM_END

/* Dodge Ball */ 
ROM_START(tvdodgeb)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_dodge_ball.bin", 0x00000, 0x100000, CRC(7a12cf72) SHA1(c477bc5dae4e82a89766052f185afb73ca2234f3) )

	TOURVISION_BIOS
ROM_END

/* Dragon Spirit */ 
ROM_START(tvdrgnst)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_dragon_spirit.bin", 0x00000, 0x100000, CRC(5733951f) SHA1(0256b4c343a3ad1ca625c316a470cc91a5254e8e) )

	TOURVISION_BIOS
ROM_END

/*
Dungeon Explorer TourVision cart - Hudson / Atlus

Notes:
 -Cart's A18 line (pin 32) seems not connected to anything.
*/
ROM_START(tvdunexp)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_dungeonexplorer.bin", 0x00000, 0x100000, CRC(6ecc87f4) SHA1(02eb3ae0b336dbcda12166b10c9f19486fb177e0) )

	TOURVISION_BIOS
ROM_END

/* Final Blaster */ 
ROM_START(tvfblast)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_final_blaster.bin", 0x00000, 0x100000, CRC(f5f7483c) SHA1(3933719bdd7a0c73cdad76de78d80463112b475a) )

	TOURVISION_BIOS
ROM_END

/* Final Lap Twin - Namco */
ROM_START(tvflaptw)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_flaptw.bin", 0x00000, 0x100000, CRC(3ca56272) SHA1(9b5417ae9a9400fead170e882d3dae19edfd7157) )

	TOURVISION_BIOS
ROM_END

/* Final Match Tennis */ 
ROM_START(tvftenis)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_final_match_tennis.bin", 0x00000, 0x100000, CRC(f83ed70f) SHA1(f566bd7a806c11f3d33ba0a976e36026a131e6fd) )

	TOURVISION_BIOS
ROM_END

/* Formation Soccer - Human Cup ' 90 - Human */
ROM_START(tvfsoc90)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_fsoc90.bin", 0x00000, 0x100000, CRC(428ffeb1) SHA1(5d12f3ed7f42b2b6da4d8eba95a16e2d34616846) )

	TOURVISION_BIOS
ROM_END

/* Gomola Speed - Human */
ROM_START(tvgomola)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_gomola.bin", 0x00000, 0x100000, CRC(41e8e18f) SHA1(210e511b85056bf216fc0d2540ed379a9dc7c18f) )

	TOURVISION_BIOS
ROM_END

/* Gunhed */ 
ROM_START(tvgunhed)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_gunhed.bin", 0x00000, 0x100000, CRC(9baace99) SHA1(ab676ba72a80314e8cba3810789041d3cc6298f9) )

	TOURVISION_BIOS
ROM_END

/* Hana Taka Daka (Super Long Nose Goblin) */ 
ROM_START(tvhtdaka)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_hana_taka_daka.bin", 0x00000, 0x100000, CRC(0fbfda5c) SHA1(02b2ce93ee5e2aaa11c8640ced15258d0d844e6f) )

	TOURVISION_BIOS
ROM_END

/* Jakie Chan - Hudson */
ROM_START(tvjchan)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_jchan.bin", 0x00000, 0x100000, CRC(7fe2b77c) SHA1(f27251451301dfb800e454c09fbb82d43b518592) )

	TOURVISION_BIOS
ROM_END

/* jinmu Densho */ 
ROM_START(tvdensho)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_dinmu_densho.bin", 0x00000, 0x100000, CRC(411a8643) SHA1(46258042dcf6510404ebccaf47034421928f72a8) )

	TOURVISION_BIOS
ROM_END

/* Kiki Kaikai */ 
ROM_START(tvkaikai)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_kiki_kaikai.bin", 0x00000, 0x100000, CRC(2bdd93f9) SHA1(9b08606865abb8cc8fa17a22becae34b172ff81a) )

	TOURVISION_BIOS
ROM_END

/* Ledgnd of Hero Tonma */ 
ROM_START(tvtonma)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_legend_of_hero_tonma.bin", 0x00000, 0x100000, CRC(e7c2efe3) SHA1(5767bdfa5600b1586e49c17cebd0fd7ef2c5426c) )

	TOURVISION_BIOS
ROM_END

/* Makyo Densetsu - The Legenary Axe - Victor Musical Industries, Inc. */
ROM_START(tvlegaxe)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_makyodensetsuthelegendaryaxe.bin", 0x00000, 0x100000, CRC(50ec3f97) SHA1(d583fa240a4dfd14b0d53ff78762fbac52694dd2) )

	TOURVISION_BIOS
ROM_END

/* Mizubaku Daibouken Liquid Kids */ 
ROM_START(tvlqkids)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_liquid_kids.bin", 0x00000, 0x100000, CRC(23a8636d) SHA1(752e03dcf8617b5a39cd250f4db1fe13cd13b761) )

	TOURVISION_BIOS
ROM_END

/* Mr Heli */ 
ROM_START(tvmrheli)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_mr_heli.bin", 0x00000, 0x100000, CRC(bf197c7a) SHA1(048f91f8ab86220a39ab146e531081950eaf1138) )

	TOURVISION_BIOS
ROM_END

/* Ninja Ryukenden */ 
ROM_START(tvninjar)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_ninja_ryukenden.bin", 0x00000, 0x100000, CRC(d9cc00ca) SHA1(42d914d338d7d0073b5cc98a4e85729e86bbfad1) )

	TOURVISION_BIOS
ROM_END

/* Operation Wolf */ 
ROM_START(tvopwolf)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_operation_wolf.bin", 0x00000, 0x100000, CRC(d4a755a9) SHA1(cd236ba0c3439ba2356cb270f56a41a52e0d6dc6) )

	TOURVISION_BIOS
ROM_END

/* Override */ 
ROM_START(tvovride)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_override.bin", 0x00000, 0x100000, CRC(4dbbf4ef) SHA1(180a68f87a881db1d01ffa3566e0d2e28303d09e) )

	TOURVISION_BIOS
ROM_END

/* Pac-Land */ 
ROM_START(tvpaclnd)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_pac-land.bin", 0x00000, 0x100000, CRC(32aee4e2) SHA1(900a918e73aaa1dc5752f851ebd85217e736109b) )

	TOURVISION_BIOS
ROM_END

/* PC Genjin Punkic Cyborg */ 
ROM_START(tvpcybrg)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_pc_genijin_punkic_cyborg.bin", 0x00000, 0x100000, CRC(5dfdc8fd) SHA1(e4e263cf7c102837c7d669d27894085f3369dd9b) )

	TOURVISION_BIOS
ROM_END

/* PC Genjin 2 - Hudson */
ROM_START(tvpcgen2)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_pckid2.bin", 0x00000, 0x100000, CRC(57fab9ee) SHA1(07c8b18905fceac73c3e18b747e8cf92d8a5f515) )

	TOURVISION_BIOS
ROM_END

/* Power 11 - Hudson */
ROM_START(tvpow11)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_pow11.bin", 0x00000, 0x100000, CRC(375114a3) SHA1(845633345886b335e6c82b3f56ef012d9820e64d) )

	TOURVISION_BIOS
ROM_END

/* Power Drift */ 
ROM_START(tvpdrift)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_power_drift.bin", 0x00000, 0x100000, CRC(eb2fdf0b) SHA1(da2191dd6e9d186c10c1c4d415254b8d7c456159) )

	TOURVISION_BIOS
ROM_END

/*
Power League IV - Hudson

Notes:
 -1st and 2nd halfs are identical, left unsplit for reference.
 -Cart's A19 line seems not connected to anything.
 -CRC of split ROM ("30cc3563") matches the common PC Engine Hu-Card ROM dump.
*/
ROM_START(tvpwlg4)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_powerleague4.bin", 0x00000, 0x100000, CRC(0a6e65f8) SHA1(88adf3f5b9a6d139f216bdb73abf8606bb8e5b16) )

	TOURVISION_BIOS
ROM_END

/* Pro Yakyuu World Stadium '91 */ 
ROM_START(tvpros91)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_pro_yakyuu_world_stadium_91.bin", 0x00000, 0x100000, CRC(2a5f1283) SHA1(e5044e397e6ccbc5c5741fa3f073697b60116325) )

	TOURVISION_BIOS
ROM_END

/* Psycho Chaser */ 
ROM_START(tvpchasr)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_pyscho_chaser.bin", 0x00000, 0x100000, CRC(e0b65280) SHA1(83248975e9bea62e67b5314c663d372c12b08416) )

	TOURVISION_BIOS
ROM_END

/* Puzzle Boy */ 
ROM_START(tvpzlboy)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_puzzle_boy.bin", 0x00000, 0x100000, CRC(0dd96cda) SHA1(652ce8b06f2aef69698d4372ff67b86362655de5) )

	TOURVISION_BIOS
ROM_END

/* Raiden */ 
ROM_START(tvraiden)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_raiden.bin", 0x00000, 0x100000, CRC(b99a85b6) SHA1(5c8b103c5a7bfeba20dcc490204d672b55e36452) )

	TOURVISION_BIOS
ROM_END

/*
Rastan Saga II Tourvision cart - Taito

Notes:
 -Cart's A18 line seems not connected to anything.
*/
ROM_START(tvrs2)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_rastansagaii.bin", 0x00000, 0x100000, CRC(cfe4c2f1) SHA1(1e39276b7d4bdb49421cc1102ad2fbba946127da) )

	TOURVISION_BIOS
ROM_END

/* R-Type II */ 
ROM_START(tvrtype2)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_r-type_ii.bin", 0x00000, 0x100000, CRC(b03bfd7a) SHA1(cc8cec1fc4bae3937d0ed60468ff703d07ce9d0c) )

	TOURVISION_BIOS
ROM_END

/* Saiga No Nindou - Ninja Spirit */ 
ROM_START(tvninjas)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_saiga_no_nindou.bin", 0x00000, 0x100000, CRC(87894514) SHA1(6845c29247f9dd805b7cd8cb046e88526e853a11) )

	TOURVISION_BIOS
ROM_END

/* Salamander */ 
ROM_START(tvslmndr)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_salamander.bin", 0x00000, 0x100000, CRC(ae8bcdf1) SHA1(3cc48fa594ab5ce1573c61861ec8e927163b6abb) )

	TOURVISION_BIOS
ROM_END

/* Shinobi */ 
ROM_START(tvshnobi)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_shinobi.bin", 0x00000, 0x100000, CRC(091a2b01) SHA1(aac2d5fadc74f837b73f662456f8a308413de57a) )

	TOURVISION_BIOS
ROM_END

/* Side arms */ 
ROM_START(tvsdarms)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_side_arms.bin", 0x00000, 0x100000, CRC(04256267) SHA1(a4ff8f19fa528fc8a7aae5ad7e0c574dc52c3388) )

	TOURVISION_BIOS
ROM_END

/* Skweek */ 
ROM_START(tvskweek)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_skweek.bin", 0x00000, 0x100000, CRC(b2a86ecc) SHA1(c1b113132ca6be1b0f3f16f31cc5ba894bee7e91) )

	TOURVISION_BIOS
ROM_END

/* Son Son II */ 
ROM_START(tvsson2)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_son_son_ii.bin", 0x00000, 0x100000, CRC(8fb484cd) SHA1(553838dcb3524fe0b620ea60e926a57cc371068d) )

	TOURVISION_BIOS
ROM_END

/*
Special Criminal Investigation (SCI) - Taito

Notes:
 -1st and 2nd halfs are identical, left unsplit for reference.
 -Cart's A19 line seems not connected to anything.
 -CRC of split ROM ("09a0bfcc") matches the common English language PC Engine Hu-Card ROM dump.
*/
ROM_START(tvsci)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_sci.bin", 0x00000, 0x100000, CRC(4baac6d8) SHA1(4c2431d9553e2bd952cf816e78fc1e3387376ef4) )

	TOURVISION_BIOS
ROM_END

/* Super Star Soldier - Hudson / Kaneko */
ROM_START(tvsssold)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_sssold.bin", 0x00000, 0x100000, CRC(bb2a0b14) SHA1(5380d25b4d5bb3e0048ed857fd36a8206e81a234) )

	TOURVISION_BIOS
ROM_END

/* Super Volley ball - Video System */
ROM_START(tvsvball)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_supervolleyball.bin", 0x00000, 0x100000, CRC(8a32a1ca) SHA1(80144fb4035415eb9b2c67d78d55757ed0d641a1) )

	TOURVISION_BIOS
ROM_END

/* Tatsujin */ 
ROM_START(tvtsujin)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_tatsujin.bin", 0x00000, 0x100000, CRC(023adbcc) SHA1(bef7d03fff2e74970a0747c12d31ec8661703deb) )

	TOURVISION_BIOS
ROM_END

/* Terra Cresta II */ 
ROM_START(tvtcrst2)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_terra_cresta_ii.bin", 0x00000, 0x100000, CRC(8e7bb390) SHA1(af13afe006313b0db1273782c977efdad6100291) )

	TOURVISION_BIOS
ROM_END

/*
Thunder Blade Tourvision cart - Sega / NEC Avenue

Notes:
 -1st and 2nd halfs are identical, left unsplit for reference.
 -Cart's A19 line seems not connected to anything.
 -CRC of split ROM ("DDC3E809") matches the common PC Engine Hu-Card ROM dump.
*/
ROM_START(tvthbld)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_thunderblade.bin", 0x00000, 0x100000, CRC(0b93b85b) SHA1(b7d9fc2f46f95d305aa24326eded13abbe93738c) )

	TOURVISION_BIOS
ROM_END

/* Toy Shop Boys */ 
ROM_START(tvtsboys)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_toy_shop_boys.bin", 0x00000, 0x100000, CRC(a9ed3440) SHA1(c519744cc16dad7a1455e359020ce95f4ac0b51a) )

	TOURVISION_BIOS
ROM_END

/*
USA Pro Basketball - Aicom

Notes:
 -4 identical 256KB parts, left unsplit for reference.
 -Cart's A19 and A18 lines seems not connected to anything.
 -CRC of split ROM ("1CAD4B7F") matches the common PC Engine Hu-Card ROM dump.
*/
ROM_START(tvusapb)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_usaprobasketball.bin", 0x00000, 0x100000, CRC(f9a86270) SHA1(45f33fd80a0fa16a9271d258d8e827c3d5e8c98d) )

	TOURVISION_BIOS
ROM_END

/* Veigues */ 
ROM_START(tveigues)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_veigues.bin", 0x00000, 0x100000, CRC(64ef8be7) SHA1(634191a181cbccbed8cf7a86e4f074691ba9b715) )

	TOURVISION_BIOS
ROM_END

/* Volfied - Taito */
ROM_START(tvvolfd)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_volfd.bin", 0x00000, 0x100000, BAD_DUMP CRC(c33efba5) SHA1(41ad4f85e551321487be61e2adbeae67e65c47de) )

	TOURVISION_BIOS
ROM_END

/* Winning Shot */ 
ROM_START(tvwnshot)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_winning_shot.bin", 0x00000, 0x100000, CRC(7196b2ca) SHA1(a1ae2e875541ad39751a95629d614d2c913b8c02) )

	TOURVISION_BIOS
ROM_END

/* W-Ring */ 
ROM_START(tvwring)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_w-ring.bin", 0x00000, 0x100000, CRC(609dc08d) SHA1(191b8751fc5b8700c7d9dae23d194016fe84586c) )

	TOURVISION_BIOS
ROM_END

/* Xevious */ 
ROM_START(tvxvious)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tourv_xevious.bin", 0x00000, 0x100000, CRC(3c0fb5a9) SHA1(1fd9ff582da83e1b9fee569da4db4de15e912f62) )

	TOURVISION_BIOS
ROM_END


GAME( 19??, tourvis,  0,       tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision)",                                      "Tourvision PCE bootleg", MACHINE_IS_BIOS_ROOT | MACHINE_NOT_WORKING )
GAME( 1988, tvdrgnst, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Namcot",                             "Dragon Spirit (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1988, tvlegaxe, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Victor Musical Industries, Inc.",    "Makyo Densetsu - The Legenary Axe (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

GAME( 1989, tvdunexp, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Atlus Ltd. / Hudson Soft",           "Dungeon Explorer (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1989, tvflaptw, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Namco Ltd. / Namcot",                "Final Lap Twin (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1989, tvgunhed, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Hudson / Toho Sunrise",              "Gunhed (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1989, tvdensho, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Big Club / Wolf Team",               "Jinmu Densho (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1989, tvmrheli, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / IREM Corp",                          "Mr Heli (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1989, tvpaclnd, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Namco / Namcot",                     "Pac-Land (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1989, tvshnobi, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Sega / Asmik Corporation",           "Shinobi (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1989, tvsdarms, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Capcom / Nec Avenue",                "Side Arms (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1989, tvsson2,  tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Capcom / Nec Avenue",                "Son Son II (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1989, tvusapb,  tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Aicom Corporation",                  "USA Pro Basketball (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1989, tvvolfd,  tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Taito Corporation",                  "Volfied (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1989, tvwnshot, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Data East Corp.",                    "Winning Shot (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

GAME( 1990, tvablast, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Inter State / Kaneko / Hudson Soft", "Aero Blasters (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvaburn,  tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Sega / Nec Avenue",                  "After Burner (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvarmedf, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Nichibutsu / Big Don",               "Armed-F (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvbeball, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Hudson Soft",                        "Be Ball (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvbomber, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Hudson Soft",                        "Bomberman (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvbrabho, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Namco / Namcot",                     "Chōzetsurinjin Beraboh Man (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvdsenpu, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Toaplan / Nec Avenue",               "Daisenpu (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvdevilc, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Naxat / Red",                        "Devil Crash (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvdodgeb, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Technos Japan Corp / Naxat Soft",    "Dodge Ball (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvfblast, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Namco / Namcot",                     "Final Blaster (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvfsoc90, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Human",                              "Formation Soccer - Human Cup '90 (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvgomola, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Human",                              "Gomola Speed (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvkaikai, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Taito",                              "Kiki Kaikai (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvopwolf, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Taito / Nec Avenue",                 "Operation Wolf (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvovride, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Sting / Data East Corporation",      "Override (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvpdrift, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Sega / Asmik Corporation",           "Power Drift (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvpchasr, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Naxat Soft",                         "Psycho Chaser (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvrs2,    tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Taito Corporation",                  "Rastan Saga II (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvninjas, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / IREM Corp",                          "Saiga No Nindou - Ninja Spirit (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvsssold, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Inter State / Kaneko / Hudson Soft", "Super Star Soldier (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvsvball, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Video System",                       "Super Volley ball (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvthbld,  tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Sega / NEC Avenue",                  "Thunder Blade (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvtsboys, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Victor Musical Industries, Inc.",    "Toy Shop Boys (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tveigues, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Victor Musical Industries, Inc.",    "Veigues (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvwring,  tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Naxat Soft",                         "W-Ring (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, tvxvious, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Namco Ltd. / Namcot",                "Xevious (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

GAME( 1991, tv1943,   tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Capcom / Naxat Soft",                "1943 Kai (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvbalstx, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Psygnosis / Coconuts Japan",         "Ballistix (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvcolumn, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Telenet Japan",                      "Columns (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvcoryon, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Naxat Soft",                         "Coryoon (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvdmoon,  tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / T.S.S",                              "Dead Moon (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvftenis, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Human",                              "Final Match Tennis (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvhtdaka, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Taito Corporation",                  "Hana Taka Daka (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvjchan,  tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Hudson Soft",                        "Jackie Chan (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvtonma,  tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / IREM Corp",                          "Legend of Hero Tonma (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvpcgen2, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Hudson Soft / Red",                  "PC Genjin 2 - Pithecanthropus Computerurus (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvpow11,  tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Hudson Soft",                        "Power Eleven (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvpwlg4,  tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Hudson Soft",                        "Power League IV (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvpros91, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Namco / Namcot",                     "Pro Yakyuu World Stadium '91 (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvpzlboy, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Atlus / Telenet Japan",              "Puzzle Boy (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvraiden, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Seibu Kaihatsu inc / Hudson Soft",   "Raiden (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvrtype2, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / IREM Corp",                          "R-Type II (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvslmndr, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Konami",                             "Salamander (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvskweek, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Victor Musical Industries, Inc.",    "Skweek (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, tvsci,    tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Taito Corporation",                  "Special Criminal Investigation (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

GAME( 1992, tvtaisen, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Taito Corporation",                  "Chuka Taisen (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1992, tvlqkids, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Taito Corporation",                  "Mizubaku Daibouken Liquid Kids (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1992, tvninjar, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Temco / Hudson Soft",                "Ninja Ryukenden (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1992, tvpcybrg, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Hudson Soft / Red",                  "PC Genjin Punkic Cyborg (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1992, tvtsujin, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Toaplan Co Ltd / Taito Corporation", "Tatsujin (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1992, tvtcrst2, tourvis, tourvision, tourvision, pce_common_state, pce_common, ROT0, "bootleg (Tourvision) / Nichibutsu / Nihon Bussan Co., Ltd", "Terra Cresta II (Tourvision PCE bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
