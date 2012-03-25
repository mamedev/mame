/***************************************************************************

  JANKENMAN UNIT

  Driver by Roberto Fresca.
  With further improvements by MAME Team.


  Coin-operated Z80-CTC + DAC system. No screen, just artwork with lamps + LEDs.
  Janken man is coming from Jankenpon, the rock-paper-scissors game.
  There are several versions of this game, the most notable difference being in
  the artwork and bonus lamps. The kid's voice and hand are the same among all.

  Control panel is composed of buttons:
  Guu (rock), Choki (scissors), Paa (paper).
  Some cabs have a Start button and/or Payout button too.

  Info:
  http://dgm.hmc6.net/museum/jyankenman.html
  http://dgm.hmc6.net/museum/jyankenman_kattaraageru.html
  (and many videos on Youtube)


  Janken Man series (working on this hardware):

  * Janken Man (Pretty Carnival) (3 station prize game), (c) 1985, Sunwise.
  * New Janken Man,                                      (c) 1986, Sunwise.
  * Janken Man Fever,                                    (c) 1988, Sunwise.
  * Janken Man Fever JP,                                 (c) 1991, Sunwise.
  * Janken Man JP,                                       (c) 1991, Sunwise.
  * Janken Man Kattara Ageru,                            (c) 1991, Sunwise.
  * Janken Man Bubbles,                                  (c) 1992, Sunwise.
  * Janken Man Big,                                      (c) 19??, Sunwise.
  * Janken Man Lucky!,                                   (c) 199?, Sunwise.


****************************************************************************

  Hardware Notes...

  1x LH0080A          ; Sharp, Z80A CPU.
  1x LH0082A          ; Sharp, Z80 CTC Counter Timer Circuit.
  2x M5L8255AP-5      ; Mitsubishi, PPI 8255 (I/O).
  1x HM6116P-3        ; Hitachi, SRAM.

  1x 27C128           ; Program, labeled 'PCG1'.
  1x 27C020           ; Waveform (8bit mono unsigned 8192Hz), labeled 'PCG2'.

  1x AD7523JN         ; InterSil, D/A Converter, 8-Bit, Multiplying, 6.7MHz.
  1x LA8358           ; Sanyo, ???. (near volume knob)
  1x 386D NJR         ; New Japan Radio, LM386D IC (500mW, 1-Channel Mono Audio AMP).
  3x M54562P          ; 8-Unit 500mA source type Darlington Transistor Array with clamp diode.

  1x 2.500 MHz Xtal.
  1x 8 DIP Switches bank.


****************************************************************************

  Other specs...

  Name of game:       Janken Man.
  Manufacturer:       Sanwaizu Co., Ltd. (Bankruptcy on March 6, 1998)
  Year:               May 1985 (1985)
  Body dimensions:    Depth: 355mm,  Width: 340mm, Height: 855mm (body only)
  Weight:             30kg.
  Power:              AC 100V 50/60Hz.
  Power consumption:  32W
  Capacity:           400 game tokens, 200 commemorative tokens
  Safe capacity:      6000 coins 10 yen, 500 coins 100 yen.
  Coin acceptor:      10 and 100 yens, Manufactured by Asahi Seiko 730-A/BW.
  Coin selector:      KWM/740 made by Asahi Seiko.
  Hopper:             MP04975 made by MAX.
  Solenoid:           AES-112 manufactured by Asahi Seiko.


****************************************************************************

  The waveform is 8bit mono unsigned at 8192Hz.
  Sampleset has sounds, music and voice at approximate rom offsets:

  $00c58-$038a4: "jan ken pon!"                   --> Is the call for rock paper and scissors.
  $04d2e-$05a4b: "zuko"                           --> Is just used for sound effect when player loses.
  $05b2d-$08207: "ai ko desho"                    --> Is the call for rematch when you've drawn.
  $08410-$0a9ec: "ooatari"                        --> "you got it! / perfect!".
  $0a9ec-$0c008: "yappii"                         --> Is just an exclamation of happiness.
  $0c008-$0dac0: "attarii"                        --> "you got it".

  unused PCM data:

  $15db7-$18628: "kakariin o oyobi kudasai"       --> "please call the attendant".
  $18628-$1a4f3: "keihin ga deru yo"              --> "your prize is incoming".
  $3c26d-$3f677: "keihin o sentaku shite kudasai" --> "please select your prize".


****************************************************************************

  About lamps...

  The internal layout has lamps mapped the following way:

  digits:

    0 = Left
    1 = Right

  lamps:

    0 = Multiplier 1 "attarii" (pays x1)
    1 = Multiplier 2 "ooatari" (pays x2)

    2 = Rock button LED
    3 = Scissors button LED
    4 = Paper button LED

    5 = Lose
    6 = Draw
    7 = Win

    8 = Base Hand
    9 = Paper components
    10 = Paper/Scissors components
    11 = Rock components
    12 = Scissors components
    13 = Rock/Scissors components

    14 = Payout error LED

  Not implemented in internal .lay:

    15 = Rotating blue lamp


***************************************************************************/

#define MASTER_CLOCK		XTAL_2_5MHz

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/i8255.h"
#include "sound/dac.h"

#include "jankenmn.lh"


class jankenmn_state : public driver_device
{
public:
	jankenmn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

};


/*********************************************
*            Read/Write Handlers             *
*********************************************/

static const UINT8 led_map[16] = // 7748 IC?
	{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

static WRITE8_DEVICE_HANDLER( jankenmn_lamps1_w )
{
	// hand state: d0: rock, d1: scissors, d2: paper
	output_set_lamp_value(8, (data & 7) != 0);
	output_set_lamp_value(11, data & 1);
	output_set_lamp_value(12, data >> 1 & 1);
	output_set_lamp_value(9, data >> 2 & 1);
	output_set_lamp_value(10, (data & 6) != 0);
	output_set_lamp_value(13, (data & 3) != 0);

	// d4-d7: led7seg (remaining credits) right digit
	output_set_digit_value(1, led_map[data >> 4 & 0xf]);

	// d3: ? (only set if game is over)
}

static WRITE8_DEVICE_HANDLER( jankenmn_lamps2_w )
{
	// button LEDs: d1: paper, d2: scissors, d3: rock
	output_set_lamp_value(2, data >> 3 & 1);
	output_set_lamp_value(3, data >> 2 & 1);
	output_set_lamp_value(4, data >> 1 & 1);

	// lamps: d5: draw, d6: lose, d7: win
	output_set_lamp_value(5, data >> 6 & 1);
	output_set_lamp_value(6, data >> 5 & 1);
	output_set_lamp_value(7, data >> 7 & 1);

	// d4: payout error LED
	output_set_lamp_value(14, data >> 4 & 1);

	// d0: led7seg (remaining credits) left digit
	output_set_digit_value(0, led_map[data & 1]);
}

static WRITE8_DEVICE_HANDLER( jankenmn_lamps3_w )
{
	// d1: blue rotating lamp on top of cab
	output_set_lamp_value(15, data >> 1 & 1);

	// d2: payout (waits for hopper status)
	coin_counter_w(device->machine(), 2, data & 0x04);

	// d3: right multiplier lamp(2), d4: left multiplier lamp(1)
	output_set_lamp_value(0, data >> 4 & 1);
	output_set_lamp_value(1, data >> 3 & 1);

	// d5: assume coin lockout
	coin_lockout_global_w(device->machine(), ~data & 0x20);

	// d0, d6, d7: N/C?
}

static CUSTOM_INPUT( jankenmn_hopper_status_r )
{
	// temp workaround, needs hopper
	return field.machine().rand();
}


/*********************************************
*           Memory Map Definition            *
*********************************************/

static ADDRESS_MAP_START( jankenmn_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( jankenmn_port_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ctc", z80ctc_r, z80ctc_w)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE_MODERN("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE_MODERN("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0x30, 0x30) AM_WRITENOP // ???
ADDRESS_MAP_END


/*********************************************
*          Input Ports Definitions           *
*********************************************/

static INPUT_PORTS_START( jankenmn )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("Guu (Rock)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("Choki (Scissors)")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("Paa (Paper)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN3 ) // 100 yen coin
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(jankenmn_hopper_status_r, NULL)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 ) // 10 yen coin
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) // 10 yen coin

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )	PORT_DIPLOCATION("DSW:7,8")
	PORT_DIPSETTING( 0x00, "Lamp Test" )
	PORT_DIPSETTING( 0x02, DEF_STR( 3C_1C ) )		// or 4 credits on 100 yen
	PORT_DIPSETTING( 0x01, DEF_STR( 2C_1C ) )		// or 6 credits on 100 yen
	PORT_DIPSETTING( 0x03, DEF_STR( 1C_1C ) )		// or 11 credits on 100 yen
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


/*********************************************
*   CTC & Daisy Chain Interrupts Interface   *
*********************************************/

static Z80CTC_INTERFACE( ctc_intf )
{
	0,              	/* timer disables */
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),	/* interrupt handler */
	DEVCB_NULL,			/* ZC/TO0 callback */
	DEVCB_NULL,         /* ZC/TO1 callback */
	DEVCB_NULL          /* ZC/TO2 callback */
};

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ NULL }
};


/*********************************************
*          PPI 8255 (x2) Interface           *
*********************************************/

static I8255_INTERFACE (ppi8255_intf_0)
{
	/* (10-13) Mode 0 - Ports A & B set as input, high C & low C as output. */
	DEVCB_INPUT_PORT("DSW"),
	DEVCB_NULL,
	DEVCB_INPUT_PORT("IN0"),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(jankenmn_lamps3_w)
};

static I8255_INTERFACE (ppi8255_intf_1)
{
	/* (20-23) Mode 0 - Ports A, B, high C & low C set as output. */
	DEVCB_NULL,
	DEVCB_DEVICE_HANDLER("dac", dac_w),
	DEVCB_NULL,
	DEVCB_HANDLER(jankenmn_lamps1_w),
	DEVCB_NULL,
	DEVCB_HANDLER(jankenmn_lamps2_w)
};


/*********************************************
*               Machine Config               *
*********************************************/

static MACHINE_CONFIG_START( jankenmn, jankenmn_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK)	/* 2.5 MHz */
	MCFG_CPU_CONFIG(daisy_chain)
	MCFG_CPU_PROGRAM_MAP(jankenmn_map)
	MCFG_CPU_IO_MAP(jankenmn_port_map)

	MCFG_I8255_ADD( "ppi8255_0", ppi8255_intf_0 )
	MCFG_I8255_ADD( "ppi8255_1", ppi8255_intf_1 )

	MCFG_Z80CTC_ADD("ctc", MASTER_CLOCK, ctc_intf)

	/* NO VIDEO */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END


/*********************************************
*                  Rom Load                  *
*********************************************/

ROM_START( jankenmn )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "pcg2.bin",   0x0000, 0xc000, CRC(48a8f769) SHA1(656346ca0a83fd8ff5c8683152e4c5e1a1c797fa) )
	ROM_CONTINUE(           0xe000, 0x2000 )
	ROM_IGNORE( 0x32000 ) // lots of unused PCM data

	// Z80 code/data in $0000-$2b4f, rest is empty. CTC probably at 58-5b.
	// Can't find any sign of 8255 PPI. Is this an unused leftover or testrom?
	ROM_REGION( 0x4000, "temp", 0 )
	ROM_LOAD( "pcg1.bin",   0x0000, 0x4000,  CRC(a9c5aa2e) SHA1(c3b81eeefa5c442231cd26615aaf6c682063b26f) )
ROM_END


/*********************************************
*                Game Drivers                *
*********************************************/

/*     YEAR  NAME      PARENT  MACHINE   INPUT     INIT  ROT    COMPANY    FULLNAME                   FLAGS...  LAYOUT */
GAMEL( 1991, jankenmn, 0,      jankenmn, jankenmn, 0,    ROT0, "Sunwise", "Janken Man Kattara Ageru", 0,        layout_jankenmn )
