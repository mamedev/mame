/****************************************************************************

    TourVision
    Driver by Mariusz Wojcieszek

	Bootleg PC-Engine based arcade board from the Spanish company TourVision. 
	Two known hardware revisions, one with a sub-board with the PC-Engine chipset 
	and the other as an integrated PCB.

	Todo: complete jamma interface emulation.
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

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/pcecommn.h"
#include "video/vdc.h"
#include "cpu/h6280/h6280.h"
#include "sound/c6280.h"
#include "machine/i8155.h"


class tourvision_state : public driver_device
{
public:
	tourvision_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

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
INPUT_PORTS_END

static ADDRESS_MAP_START( pce_mem , AS_PROGRAM, 8)
	AM_RANGE( 0x000000, 0x07FFFF) AM_ROM
	AM_RANGE( 0x1F0000, 0x1F1FFF) AM_RAM AM_MIRROR(0x6000) AM_BASE( &pce_user_ram )
	AM_RANGE( 0x1FE000, 0x1FE3FF) AM_READWRITE( vdc_0_r, vdc_0_w )
	AM_RANGE( 0x1FE400, 0x1FE7FF) AM_READWRITE( vce_r, vce_w )
	AM_RANGE( 0x1FE800, 0x1FEBFF) AM_DEVREADWRITE( "c6280", c6280_r, c6280_w )
	AM_RANGE( 0x1FEC00, 0x1FEFFF) AM_READWRITE( h6280_timer_r, h6280_timer_w )
	AM_RANGE( 0x1FF000, 0x1FF3FF) AM_READWRITE( pce_joystick_r, pce_joystick_w )
	AM_RANGE( 0x1FF400, 0x1FF7FF) AM_READWRITE( h6280_irq_status_r, h6280_irq_status_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( pce_io , AS_IO, 8)
	AM_RANGE( 0x00, 0x03) AM_READWRITE( vdc_0_r, vdc_0_w )
ADDRESS_MAP_END

static WRITE8_HANDLER( tourvision_8085_d000_w )
{
	//logerror( "D000 (8085) write %02x\n", data );
}

static ADDRESS_MAP_START(tourvision_8085_map, AS_PROGRAM, 8)
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x80ff) AM_DEVREADWRITE_MODERN("i8155", i8155_device, memory_r, memory_w)
	AM_RANGE(0x8100, 0x8107) AM_DEVREADWRITE_MODERN("i8155", i8155_device, io_r, io_w)
	AM_RANGE(0xd000, 0xd000) AM_WRITE( tourvision_8085_d000_w )
	AM_RANGE(0xe000, 0xe1ff) AM_RAM
ADDRESS_MAP_END

static WRITE8_DEVICE_HANDLER(tourvision_i8155_a_w)
{
	//logerror("i8155 Port A: %02X\n", data);
}

static WRITE8_DEVICE_HANDLER(tourvision_i8155_b_w)
{
	//logerror("i8155 Port B: %02X\n", data);
}

static WRITE8_DEVICE_HANDLER(tourvision_i8155_c_w)
{
	//logerror("i8155 Port C: %02X\n", data);
}

static WRITE_LINE_DEVICE_HANDLER(tourvision_timer_out)
{
	cputag_set_input_line(device->machine(), "sub", I8085_RST55_LINE, state ? CLEAR_LINE : ASSERT_LINE );
	//logerror("Timer out %d\n", state);
}

static I8155_INTERFACE(i8155_intf)
{
	DEVCB_NULL,
	DEVCB_HANDLER(tourvision_i8155_a_w),
	DEVCB_NULL,
	DEVCB_HANDLER(tourvision_i8155_b_w),
	DEVCB_NULL,
	DEVCB_HANDLER(tourvision_i8155_c_w),
	DEVCB_LINE(tourvision_timer_out)
};

static const c6280_interface c6280_config =
{
	"maincpu"
};

static MACHINE_CONFIG_START( tourvision, tourvision_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", H6280, PCE_MAIN_CLOCK/3)
	MCFG_CPU_PROGRAM_MAP(pce_mem)
	MCFG_CPU_IO_MAP(pce_io)
	MCFG_TIMER_ADD_SCANLINE("scantimer", pce_interrupt, "screen", 0, 1)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_CPU_ADD("sub", I8085A, 18000000/3 /*?*/)
	MCFG_CPU_PROGRAM_MAP(tourvision_8085_map)

    /* video hardware */

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_RAW_PARAMS(PCE_MAIN_CLOCK/2, VDC_WPF, 70, 70 + 512 + 32, VDC_LPF, 14, 14+242)
	MCFG_SCREEN_UPDATE( pce )

	/* MCFG_GFXDECODE( pce_gfxdecodeinfo ) */
	MCFG_PALETTE_LENGTH(1024)
	MCFG_PALETTE_INIT( vce )

	MCFG_VIDEO_START( pce )

	MCFG_I8155_ADD("i8155", 1000000 /*?*/, i8155_intf)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker","rspeaker")
	MCFG_SOUND_ADD("c6280", C6280, PCE_MAIN_CLOCK/6)
	MCFG_SOUND_CONFIG(c6280_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)

MACHINE_CONFIG_END

/*
Taito Scene Crime Investigation (SCI) Tourvision cart.

Notes:
 -1st and 2nd halfs are identical, left unsplit for reference.
 -Cart's A19 line seems not connected to anything.
 -CRC of split ROM ("09a0bfcc") matches the common English language PC Engine Hu-Card ROM dump.

Dumped directly from the cartridge edge connector using the following adapter:

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
*/

ROM_START(scitvpce)
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tourv_sci.bin", 0x00000, 0x80000, CRC(4baac6d8) SHA1(4c2431d9553e2bd952cf816e78fc1e3387376ef4) )
	ROM_CONTINUE(0x00000, 0x80000)

	ROM_REGION( 0x8000, "sub", 0 )
	ROM_LOAD( "tourvision.bin", 0x0000, 0x8000, CRC(87cf66c1) SHA1(d6b42137be7a07a0e299c2d922328a6a9a2b7b8f) )
ROM_END

static DRIVER_INIT(tourvision)
{
	DRIVER_INIT_CALL(pce);
}

GAME( 1991, scitvpce, 0, tourvision, tourvision, tourvision, ROT0, "bootleg (Tourvision) / Taito", "Special Criminal Investigation (Tourvision PCE bootleg)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
