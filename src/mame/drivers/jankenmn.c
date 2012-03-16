/***************************************************************************

  JANKENMAN UNIT

  Preliminary driver by Roberto Fresca.


  Coin-operated Z80-CTC + DAC system. No screen, just artwork with lamps + LEDs.
  Janken man is coming from Jankenpon, the rock-paper-scissors game.
  There are several versions of this game, the most notable difference being in
  the artwork and bonus lamps. The kid's voice and hand are the same among all.

  info: http://dgm.hmc6.net/museum/jyankenman.html
  (and many videos on Youtube)

  It's all a challenge. Even once emulated, the game will need a lot of
  artwork and lamps work...


  Games working on this hardware:
  
  * Janken Man (Pretty Carnival) (3 station prize game),  (c) 1985, Sunwise.


****************************************************************************

  Hardware Notes...


  1x LH0080A          ; Sharp, Z80A CPU.
  1x LH0082A          ; Sharp, Z80 CTC Counter Timer Circuit.
  2x M5L8255AP-5      ; Mitsubishi, PPI 8255 (I/O).
  1x HM6116P-3        ; Hitachi, SRAM.

  1x 27C128           ; Program, labeled 'PCG1'.
  1x 27C020           ; Waveform (8bit mono unsigned 8192Hz), labeled 'PCG2'.

  1x AD7523JN         ; InterSil, D/A Converter, 8-Bit, Multiplying, 6.7MHz.
  1x LA8358           ; Sanyo, ???.
  1x 386D NJR         ; New Japan Radio, LM386D IC (500mW, 1-Channel Mono Audio AMP).
  3x M54562P          ; 8-Unit 500mA source type Darlington Transistor Array with clamp diode.

  1x 2.500 MHz Xtal.
  1x 8 DIP Switches bank.


****************************************************************************

  Other specs...

  Name of game:       Janken Man.
  Manufacturer:	      Sanwaizu Co., Ltd. (Bankruptcy on March 6, 1998)
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

  Dev Notes...

  Run the second program, then BP to 0x97b.
  the instruction 'ld a,($c000)' throw the code flow to $f3c0 (IM2?)  ---> YES. The CTC implementation fixed it.


  Later, the code flow (pc=9f7) is branch to 0xfc1d, where there is no code...

****************************************************************************

  The waveform is 8bit mono unsigned at 8192Hz.
  Sampleset has sounds, music and...

  "jan ken pon!"                   --> Is the call for rock paper and scissors.
  "zuko"                           --> Is just used for sound effect.
  "ai ko desho"                    --> Is the call for rematch when you've drawn.
  "ooatari"                        --> "you got it! / perfect!".
  "yappii"                         --> Is just an exclamation of happiness.
  "attarii"                        --> "you got it".
  "kakariin o oyobi kudasai"       --> "please call the attendant".
  "keihin ga deru yo"              --> "your prize is incoming".
  "keihin o sentaku shite kudasai" --> "please select your prize".


  Control panel is composed of 5 buttons:
  Start, Guu (rock), Choki (scissors), Paa (paper) and Payout.


  5 lamps for numbers 2, 4, 8, 16 and 32.
  3 states lamps for  LOSE (left), DRAW (middle) and WIN (right).

  Unknown lamps for hands (seems 3 wires, so up to 7 components)

  3 LEDs in the control panel. One for each hand button.
  
  
****************************************************************************

  About lamps...

  The internal layout has lamps mapped the following way:

  Lamp0 = Multiplier 2
  Lamp1 = Multiplier 4
  Lamp2 = Multiplier 8
  Lamp3 = Multiplier 16
  Lamp4 = Multiplier 32

  Lamp5 = Win
  Lamp6 = Draw
  Lamp7 = Lose

  Lamp8 = Base Hand
  Lamp9 = Paper components
  Lamp10 = Scissors components
  Lamp11 = Rock components

  The Hand lamps should be splitted into more segments/components, due to
  some of them are shared between states.


****************************************************************************

  Memory Map:
  -----------


  PCG1, Main Z80(?)
  
  0000-3fff  ; ROM Space.
  4000-47ff  ; Work RAM
  
  58-5b  : must be CTC
  ..


  PCG2, Audio Z80?... or main program?

  0000-bfff  ; ROM Space.
  c000-c7ff  ; Work RAM

  00-03  : CTC? or is CTC mapped at 30-33?
  10-13  : PPI1: A & B input, high C & low C output.
  20-23  : PPI2: A, B, high C & low C output.


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

static WRITE8_DEVICE_HANDLER( ppi0_portc_w )
{
/*  PPI-0 (10h-13h); PortC OUT.
    unknown:

  7654 3210
  xxxx xxxx  * unknown.

*/
	logerror("ppi0_portc: %02x\n", data);
}

//static WRITE8_DEVICE_HANDLER( ppi1_porta_w )
//{
/*  PPI-1 (20h-23h); PortA OUT.
    unknown:

  7654 3210
  xxxx xxxx  * unknown.

*/
	// DAC??
//	logerror("ppi1_porta: %02x\n", data);
//}

static WRITE8_DEVICE_HANDLER( ppi1_portb_w )
{
/*  PPI-1 (20h-23h); PortB OUT.
    unknown:

  7654 3210
  xxxx xxxx  * unknown.

*/
	logerror("ppi1_portb: %02x\n", data);
}

static WRITE8_DEVICE_HANDLER( ppi1_portc_w )
{
/*  PPI-1 (20h-23h); PortC OUT.
    unknown:

  7654 3210
  xxxx xxxx  * unknown.

*/
	logerror("ppi1_portc: %02x\n", data);
}


/*********************************************
*           Memory Map Definition            *
*********************************************/


static ADDRESS_MAP_START( jankenmn_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
//	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( jankenmn_port_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ctc", z80ctc_r, z80ctc_w)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE_MODERN("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE_MODERN("ppi8255_1", i8255_device, read, write)
ADDRESS_MAP_END

/*

  RW

  10-13  : PPI1 --> ctrl 0x92 : A & B input, high C & low C output.
  20-23  : PPI2 --> ctrl 0x90 : A, B, high C & low C output.

':audiocpu' (000C): unmapped i/o memory write to 0030 = 00 & FF

':audiocpu' (0018): unmapped i/o memory write to 0000 = 57 & FF \
':audiocpu' (001C): unmapped i/o memory write to 0000 = 01 & FF  |
':audiocpu' (0020): unmapped i/o memory write to 0001 = 57 & FF  |
':audiocpu' (0024): unmapped i/o memory write to 0001 = 01 & FF  |
':audiocpu' (0028): unmapped i/o memory write to 0002 = 97 & FF  |> Seems CTC mapped 00-03.
':audiocpu' (002C): unmapped i/o memory write to 0002 = 14 & FF  |
':audiocpu' (0030): unmapped i/o memory write to 0003 = 97 & FF  |
':audiocpu' (0034): unmapped i/o memory write to 0003 = FF & FF  |
':audiocpu' (0038): unmapped i/o memory write to 0000 = 48 & FF /

':audiocpu' (003A): unmapped i/o memory write to 0030 = 48 & FF --> CTC defined vector (48) is written here.
':audiocpu' (0978): unmapped i/o memory write to 0030 = 00 & FF
':audiocpu' (0978): unmapped i/o memory write to 0030 = 00 & FF
':audiocpu' (0978): unmapped i/o memory write to 0030 = 00 & FF
':audiocpu' (0978): unmapped i/o memory write to 0030 = 00 & FF

  A lot of info is passing through port 30h, when a sound is triggered (press key T)

*/

/*********************************************
*          Input Ports Definitions           *
*********************************************/

static INPUT_PORTS_START( jankenmn )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1-5")	// trigger a sample
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW:8")	// trigger a sample
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW:7")	// trigger a sample
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW:6")	// seems to mute the sound system...
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW:3")	// change the sample
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW:1")	// change the sample
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
	DEVCB_INPUT_PORT("DSW"),		/* Port A read */
	DEVCB_NULL,						/* Port A write */
	DEVCB_INPUT_PORT("IN0"),		/* Port B read */
	DEVCB_NULL,						/* Port B write */
	DEVCB_NULL,						/* Port C read */
	DEVCB_HANDLER(ppi0_portc_w)		/* Port C write */
};

static I8255_INTERFACE (ppi8255_intf_1)
{
	/* (20-23) Mode 0 - Ports A, B, high C & low C set as output. */
	DEVCB_NULL,									/* Port A read */
	DEVCB_DEVICE_HANDLER("dac", dac_signed_w),	/* Port A write */	// is really the DAC here?
//	DEVCB_HANDLER(ppi1_porta_w),				/* Port A write */
	DEVCB_NULL,									/* Port B read */
	DEVCB_HANDLER(ppi1_portb_w),				/* Port B write */
	DEVCB_NULL,									/* Port C read */
	DEVCB_HANDLER(ppi1_portc_w)					/* Port C write */
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
	ROM_REGION( 0x40000, "maincpu", 0 )		// this one has valid z80 code, plus the PPI inits
	ROM_LOAD( "pcg2.bin",   0x0000, 0x10000, CRC(48a8f769) SHA1(656346ca0a83fd8ff5c8683152e4c5e1a1c797fa) )
    ROM_IGNORE(                     0x30000)	// need a banking

	ROM_REGION( 0x10000, "temp", 0 )	// this should be the program ROM, but there's not PPI code and seems odd
	ROM_LOAD( "pcg1.bin",   0x0000, 0x4000,  CRC(a9c5aa2e) SHA1(c3b81eeefa5c442231cd26615aaf6c682063b26f) )
ROM_END


static DRIVER_INIT( jankenmn )
{
}


/*********************************************
*                Game Drivers                *
*********************************************/

/*     YEAR  NAME      PARENT  MACHINE   INPUT     INIT      ROT    COMPANY    FULLNAME                       FLAGS...          LAYOUT */
GAMEL( 1985, jankenmn, 0,      jankenmn, jankenmn, jankenmn, ROT0, "Sunwise", "Janken Man (Pretty Carnival)", GAME_NOT_WORKING, layout_jankenmn )
