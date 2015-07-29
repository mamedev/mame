// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/**********************************************************************************

    FOUR ROSES.

    Driver by Roberto Fresca.


***********************************************************************************


    The hardware is composed by:

    CPU:    1x R65C02P2 (main) at 2MHz.
            1x EP87C750EBPN (8-bit microcontroller family 1K/64 OTP ROM, low pin count)

    Sound:  1x WF19054 (AY-3-8910 compatible) at 2MHz.
            1x TDA2003 (audio amp).

    Video:  1x HD46505 (CRT controller)
    I/O:    Custom logic.

    RAM:    1x Elite MT LP62256E-70LL (32Kx8 SRAM, 70ns)
    NVRAM:  1x Elite MT LP6264D-70LL (8Kx8 SRAM, 70ns)
    ROMs:   1x TMS27C512 (10).
            1x TMS27C256 (20).
            1x M27C4001 (30).
            1x M27C512 (4.10).

    PLDs:   1x PALCE22V10H (read protected)

    Clock:  1x 16MHz Crystal.

    Other:  1x JAMMA edge connector.
            1x trimmer (volume).
            1x 8 DIP switches bank (SW1).
            1x 4 DIP switches bank (SW2).
            3x pushbutton (SW3-statistic, SW4-management, SW5-ricarica).
            1x battery.


    GENERAL NOTES:

    - The game is based on Funworld/Tab/CMC games, but the hardware is completely different.
      It has more complex improvements (encryption, MCU, banks, etc...)
    - The program ROM is encrypted.
    - The color palettes are stored in a normal ROM.


***********************************************************************************


    Memory Map
    ----------

    $0000 - $7FFF   Still unknown...
    $8000 - $FFFF   ROM Space.


***********************************************************************************


    Connectors, DIPs and instructions,
    from original datasheet...

    ---------------------|--|-----------------
              Components |  | solder
    ---------------------|--|-----------------
      GND                |01| GND
      GND                |02| GND
      Ricarica           |03| +12V. Lamp.
      Remote             |04| +5V. Lamp.
      Cancel / Play Take |05| Lamp Cancel (Play Take)
      Start              |06| Lamp Start
      Hold 5 / Half Take |07| Lamp Hold 5 / Half Take
      Hold 1             |08| Lamp Hold 1
      Hold 2 / Small     |09| Lamp Hold 2 / Small
      Hold 4 / Big       |10| Lamp Hold 4 / Big
      Hold 3             |11| Lamp Hold 3
      Ticket sensor      |12| Ticket drive
      Coin 1 A           |13| Coin 2 B
      Management         |14| (n/c)
      GND                |15| Statistic
      Blue               |16| Sync
      Red                |17| Green
      Hopper sensor      |18| GND
      Speaker (+)        |19| Speaker (-)
      Hopper drive       |20| Charge     Counter
      Factory  Counter   |21| Discharge  Counter
      +5V.     Counter   |22| +12V.      Counter
      +12V.              |23| +12V.
      (n/c)              |24| (n/c)
      +5V.               |25| +5V.
      +5V.               |26| +5V.
      GND                |27| GND
      GND                |28| GND


    ------------------------------------------------------------
    DIP SW.#1                     1   2   3   4   5   6   7   8
    ------------------------------------------------------------
    Remote 10                    OFF
    Remote 100                   ON
    Hopper                           OFF OFF
    Ticket                           ON  OFF
    Ticket + hopper                  OFF ON
    Ticket + hopper                  ON  ON
    CoinA: 1 coin 1 credit                   OFF OFF
    CoinA: 1 coin 5 credits                  OFF ON
    CoinA: 1 coin 2 credits                  ON  OFF
    CoinA: 1 coin 10 credits                 ON  ON
    CoinB: A x1                                      OFF
    CoinB: A x10                                     ON
    Skills without consuming 1 coin                      OFF
    Skills consuming 1 coin                              ON
    Select with cards                                       OFF
    Select without cards                                    ON
    ------------------------------------------------------------

    --------------------------------------------
    DIP SW.#2                     1   2   3   4
    --------------------------------------------
    Game with cards              OFF OFF
    Game with balls              ON  OFF
    Game with roses              OFF ON
    Game allow choose            ON  ON
    View YES                             OFF
    View NO                              ON
    Normal Points Table                      OFF
    Points Table in Super Game               ON
    --------------------------------------------


    Instructions:
    =============

    Turn on the machine getting STATISTIC & MANAGEMENT buttons pressed.

    STATISTICS = Show credits in/out.
    To cancel statistics press CANCEL for some seconds.

    MANAGEMENT = Settings.
    The following menu will show up:

    HOLD3 = Verify In/Out, even when the stats were cleared.
    HOLD2 + HOLD4 = Modify maximum bet, pressing HOLD1.
                    Modify minimum bet, pressing HOLD2.
                    Modify percentage (1, 2, 3, 4), pressing HOLD3.

    Press START to exit from programming mode.


***********************************************************************************

    [2008/12/12]
    - Initial release.
    - Added technical notes.


    *** TO DO ***

    - Decrypt the program ROMs.
    - Fix memory map.
    - Fix GFX decode.
    - Fix color decode routines.
    - Proper inputs.
    - MCU simulation.
    - Dump/decap the MCU.


***********************************************************************************/


#define MASTER_CLOCK    XTAL_16MHz

#include "emu.h"
#include "cpu/m6502/m65c02.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"
#include "includes/funworld.h"


class _4roses_state : public funworld_state
{
public:
	_4roses_state(const machine_config &mconfig, device_type type, const char *tag)
		: funworld_state(mconfig, type, tag) { }

	DECLARE_DRIVER_INIT(4roses);
};


/**********************
* Read/Write Handlers *
**********************/



/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( 4roses_map, AS_PROGRAM, 8, _4roses_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM // AM_SHARE("nvram")
	AM_RANGE(0x6000, 0x6fff) AM_RAM_WRITE(funworld_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x7000, 0x7fff) AM_RAM_WRITE(funworld_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

/*
    Unknown R/W
    -----------


*/


/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( 4roses )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-3") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-4") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-5") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-6") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-7") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-8") PORT_CODE(KEYCODE_8)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-1") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-2") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-3") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-4") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-5") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-6") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-7") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-8") PORT_CODE(KEYCODE_I)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-7") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-8") PORT_CODE(KEYCODE_K)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-6") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-7") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-8") PORT_CODE(KEYCODE_L)

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")   /* this bank is x4 DIP switches */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
{
/* WRONG... Must be changed */
	4,
	8,
//  0x1000,
	RGN_FRAC(1,2),
	4,
//  { 0, 4, 0x8000*8, 0x8000*8+4 },
	{ RGN_FRAC(0,2), RGN_FRAC(0,2) + 4, RGN_FRAC(1,2), RGN_FRAC(1,2) + 4 },
	{ 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*4*2
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( 4roses )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout, 0, 16 )
GFXDECODE_END


/**************************
*     Machine Drivers     *
**************************/

static MACHINE_CONFIG_START( 4roses, _4roses_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M65C02, MASTER_CLOCK/8) /* 2MHz, guess */
	MCFG_CPU_PROGRAM_MAP(4roses_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", _4roses_state,  nmi_line_pulse)

//  MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE((124+1)*4, (30+1)*8)               /* guess. taken from funworld games */
	MCFG_SCREEN_VISIBLE_AREA(0*4, 96*4-1, 0*8, 29*8-1)  /* guess. taken from funworld games */
	MCFG_SCREEN_UPDATE_DRIVER(_4roses_state, screen_update_funworld)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 4roses)

	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_INIT_OWNER(_4roses_state,funworld)
	MCFG_VIDEO_START_OVERRIDE(_4roses_state,funworld)

//  MCFG_MC6845_ADD("crtc", MC6845, "screen", MASTER_CLOCK/8) /* 2MHz, guess */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay8910", AY8910, MASTER_CLOCK/8)    /* 2MHz, guess */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.5)
MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( 4roses )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* encrypted program ROM...*/
	ROM_LOAD( "4.10.u32", 0x00000, 0x10000, CRC(e94440e9) SHA1(b2f81ba79f1f40ed35e45fd80c17eb8529ccdb4c) )

	ROM_REGION( 0x0400,  "mcu", 0 ) /* protected... no dump available */
	ROM_LOAD( "ep87c750ebpn_no_dump.u41", 0x0000, 0x0400, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "30.u17", 0x00000, 0x80000, CRC(daefacc2) SHA1(5896e9da06fde39770fcdc585881b8c689b34369) )

	ROM_REGION( 0x18000, "proms", 0 )
	ROM_LOAD( "20.u43", 0x00000, 0x08000, CRC(f206b4d3) SHA1(dfee226a9e01ddacf09995ec4e027b0ed4dffe7e) )
	ROM_LOAD( "10.u39", 0x08000, 0x10000, CRC(87dcf9c5) SHA1(b289527b8d9db1e91adf85b53233415c6969f4d4) )

	ROM_REGION( 0x02dd, "plds", 0 )
	ROM_LOAD( "palce22v10h.u29", 0x0000, 0x02dd, BAD_DUMP CRC(5c4e9024) SHA1(e9d1e4df3d79c21f4ce053a84bb7b7a43d650f91) )
ROM_END

ROM_START( 4rosesa )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* encrypted program ROM...*/
	ROM_LOAD( "4.u15", 0x00000, 0x10000, CRC(66bb5b67) SHA1(438371c3918f0a285cb19caa650739df9fb24800) )

	ROM_REGION( 0x0400,  "mcu", 0 ) /* protected... no dump available */
	ROM_LOAD( "ep87c750ebpn_no_dump.u41", 0x0000, 0x0400, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "30.u17", 0x00000, 0x80000, CRC(daefacc2) SHA1(5896e9da06fde39770fcdc585881b8c689b34369) )

	ROM_REGION( 0x20000, "proms", 0 )
	ROM_LOAD( "20.ub5", 0x00000, 0x10000, CRC(01cc8b15) SHA1(f8e1fa7c0a4ae35debf8eecde31471049308cd60) )
	ROM_LOAD( "10.ua5", 0x10000, 0x10000, CRC(87dcf9c5) SHA1(b289527b8d9db1e91adf85b53233415c6969f4d4) )

	ROM_REGION( 0x02dd, "plds", 0 )
	ROM_LOAD( "palce22v10h.u29", 0x0000, 0x02dd, BAD_DUMP CRC(5c4e9024) SHA1(e9d1e4df3d79c21f4ce053a84bb7b7a43d650f91) )
ROM_END

/*
2x 6821
6845
AY-3-8910

no cpu... cpu seem a 44 plcc chip with name scratched off...

pcb is almost the same as "Four Roses"
*/
ROM_START( rugby )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* encrypted program ROM...*/
	ROM_LOAD( "rugby1.u15", 0x00000, 0x10000, CRC(6ac45fa7) SHA1(dba936d236d57172e56143a9858e5052009e4346) )

	ROM_REGION( 0x0400,  "mcu", 0 ) /* protected... no dump available */
	ROM_LOAD( "ep87c750ebpn_no_dump.u41", 0x0000, 0x0400, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "rugby2.u17", 0x00000, 0x40000, CRC(822eb316) SHA1(8568f5a67f6a54858841e6832dc987f72dd911e2) )
	ROM_LOAD( "rugby3.u18", 0x40000, 0x40000, CRC(536c56c1) SHA1(a7812f0a854c5138fb7412d656de128ff094010f) )

	ROM_REGION( 0x10000, "proms", 0 )
	ROM_LOAD( "rugby4.u5", 0x00000, 0x10000, CRC(dc1eb4cd) SHA1(fb7b933a6e4307ee693c4f4bb3630b98a0c60f16) )
ROM_END


/**************************
*  Driver Initialization  *
**************************/

DRIVER_INIT_MEMBER(_4roses_state,4roses)
{
}


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME     PARENT  MACHINE  INPUT    STATE         INIT    ROT    COMPANY      FULLNAME                        FLAGS  */
GAME( 1999, 4roses,  0,      4roses,  4roses, _4roses_state, 4roses, ROT0, "<unknown>", "Four Roses (encrypted, set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, 4rosesa, 4roses, 4roses,  4roses, _4roses_state, 4roses, ROT0, "<unknown>", "Four Roses (encrypted, set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, rugby,   0,      4roses,  4roses, _4roses_state, 4roses, ROT0, "C.M.C.",    "Rugby? (four roses hardware)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
