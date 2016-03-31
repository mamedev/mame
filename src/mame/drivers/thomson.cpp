// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Thomson 8-bit micro-computers.

  A 6809E-based French family of personal computers developed in the 80's.
  Despite their rather high price and poor design , they were quite popular
  in France, probably due to the government plan "Informatique pour tous"
  (Computer Science for Everyone) to put a network of MO5 computers in every
  school during 1984-1986. And maybe their very popular light-pen.

  Drivers
  - t9000  Thomson T9000           (early TO7 prototype, oct 1980)
  - to7    Thomson TO7             (nov 1982)
  - to770  Thomson TO7/70          (scarcely enhanced TO7, 1984)
  - mo5    Thomson MO5             (entry-game, TO7-incompatible, jun 1984)
  - to9    Thomson TO9             (high-end TO7/70 successor, sep 1985)
  - mo5e   Thomson MO5E            (export MO5 version, 1986)
  - to8    Thomson TO8             (next generation of TO7/70, sep 1986)
  - to9p   Thomson TO9+            (improved TO9 with TO8 technology, sep 1986)
  - mo6    Thomson MO6             (improved MO5 with TO8 technology, sep 1986)
  - mo5nr  Thomson MO5NR           (network-enhanced MO6, 1986)
  - to8d   Thomson TO8D            (TO8 with integrated floppy, dec 1987)
  - pro128 Olivetti Prodest PC 128 (Italian MO6, built by Thomson, 1986)

  We do not consider here the few 16-bit computers built by Thomson
  (68000-based Micromega 32, 8088-based Micromega 16 or the TO16: Thomson's own
   8088-based PC).

  You may distinguish three families:
  * TO7,TO7/70,TO8,TO8D (family computers)
  * TO9,TO9+ (professional desktop look with separate keyboard and monitor)
  * MO5,MO5E,MO5NR,MO6 (cheaper, less extensible family)
  Computers in both TO families are compatible. Computers in the MO family are
  compatible. However, the TO and MO families are incompatible
  (different memory-mapping).

  Note that the TO8, TO9+ and MO6 were produced at the same time, using very
  similar technologies, but with different cost/feature trade-offs and
  different compatibility (MO6 is MO5-compatible, TO9+ is TO9-compatible and
  TO8 and TO9+ are TO7-compatible).
  Also note that the MO5NR is actually based on MO6 design, not MO5
  (although both MO5NR and MO6 are MO5-compatible)

  Also of interest are the Platini and Hinault versions of the MO5
  (plain MO5 inside, but with custom, signed box-case).
  There were several versions of TO7/70 and MO5 with alternate keyboards.

  Thomson stopped producing micro-computers in jan 1990.

**********************************************************************/

/* TODO (roughly in decreasing priority order):
   ----

   - internal, keyboard-attached TO9 mouse port (untested)
   - floppy: 2-sided or 4-sided .fd images
   - printer post-processing => postscript
   - RS232 serial port extensions: CC 90-232, RF 57-932
   - modem, teltel extension: MD 90-120 / MD 90-333 (need controller ROM?)
   - IEEE extension
   - TV overlay (IN 57-001) (@)
   - digitisation extension (DI 90-011) (@)
   - barcode reader (@)

   (@) means MESS is lacking support for this kind of device / feature anyway

*/

#include "includes/thomson.h"
#include "bus/rs232/rs232.h"
#include "machine/6821pia.h"
#include "machine/wd_fdc.h"
#include "machine/clock.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/flopdrv.h"
#include "formats/cd90_640_dsk.h"
#include "formats/basicdsk.h"
#include "machine/ram.h"
#include "softlist.h"


/**************************** common *******************************/

#define KEY(pos,name,key)                   \
	PORT_BIT  ( 1<<(pos), IP_ACTIVE_LOW, IPT_KEYBOARD ) \
	PORT_NAME ( name )                  \
	PORT_CODE ( KEYCODE_##key )

#define PAD(mask,player,name,port,dir,key)              \
	PORT_BIT    ( mask, IP_ACTIVE_LOW, IPT_##port )         \
	PORT_NAME   ( "P" #player " " name )                \
	PORT_CODE( KEYCODE_##key )                  \
	PORT_PLAYER ( player )


/* ------------- game port ------------- */

/*
  Two generations of game port extensions were developped

  - CM 90-112 (model 1)
    connect up to two 8-position 1-button game pads

  - SX 90-018 (model 2)
    connect either two 8-position 2-button game pads
    or a 2-button mouse (not both at the same time!)

  We emulate the SX 90-018 as it is fully compatible with the CM 90-112.

  Notes:
  * all extensions are compatible with all Thomson computers.
  * the SX 90-018 extension is integrated within the TO8(D)
  * the TO9 has its own, different mouse port
  * all extensions are based on a Motorola 6821 PIA
  * all extensions include a 6-bit sound DAC
  * most pre-TO8 software (including TO9) do not recognise the mouse nor the
  second button of each pad
  * the mouse cannot be used at the same time as the pads: they use the same
  6821 input ports & physical port; we use a config switch to tell MESS
  whether pads or a mouse is connected
  * the mouse should not be used at the same time as the sound DAC: they use
  the same 6821 ports, either as input or output; starting from the TO8,
  there is a 'mute' signal to cut the DAC output and avoid producing an
  audible buzz whenever the mouse is moved; unfortunately, mute is not
  available on the TO7(/70), TO9 and MO5.
*/

static INPUT_PORTS_START( thom_game_port )

/* joysticks, common to CM 90-112 & SX 90-018 */
	PORT_START ( "game_port_directions" )
		PAD ( 0x01, 1, UTF8_UP, JOYSTICK_UP,    UP,    UP)
		PAD ( 0x02, 1, UTF8_DOWN, JOYSTICK_DOWN,  DOWN,  DOWN )
		PAD ( 0x04, 1, UTF8_LEFT, JOYSTICK_LEFT,  LEFT,  LEFT )
		PAD ( 0x08, 1, UTF8_RIGHT, JOYSTICK_RIGHT, RIGHT, RIGHT )
		PAD ( 0x10, 2, UTF8_UP, JOYSTICK_UP,    UP,    8_PAD )
		PAD ( 0x20, 2, UTF8_DOWN, JOYSTICK_DOWN,  DOWN,  2_PAD )
		PAD ( 0x40, 2, UTF8_LEFT, JOYSTICK_LEFT,  LEFT,  4_PAD )
		PAD ( 0x80, 2, UTF8_RIGHT, JOYSTICK_RIGHT, RIGHT, 6_PAD )

	PORT_START ( "game_port_buttons" )
		PAD ( 0x40, 1, "Action A", BUTTON1, BUTTON1, LCONTROL )
		PAD ( 0x80, 2, "Action A", BUTTON1, BUTTON1, RCONTROL )

/* joysticks, SX 90-018 specific */
		PAD ( 0x04, 1, "Action B", BUTTON2, BUTTON2, LALT )
		PAD ( 0x08, 2, "Action B", BUTTON2, BUTTON2, RALT )
	PORT_BIT  ( 0x30, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT  ( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED ) /* ? */

/* mouse, SX 90-018 specific */
	PORT_START ( "mouse_x" )
	PORT_BIT ( 0xffff, 0x00, IPT_MOUSE_X )
	PORT_NAME ( "Mouse X" )
	PORT_SENSITIVITY ( 150 )
	PORT_PLAYER (1)

	PORT_START ( "mouse_y" )
	PORT_BIT ( 0xffff, 0x00, IPT_MOUSE_Y )
	PORT_NAME ( "Mouse Y" )
	PORT_SENSITIVITY ( 150 )
	PORT_PLAYER (1)

	PORT_START ( "mouse_button" )
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_NAME ( "Left Mouse Button" )
	PORT_CODE( MOUSECODE_BUTTON1 )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_NAME ( "Right Mouse Button" )

INPUT_PORTS_END


/* ------------ lightpen ------------ */

static INPUT_PORTS_START( thom_lightpen )

	PORT_START ( "lightpen_x" )
	PORT_BIT ( 0xffff, THOM_TOTAL_WIDTH/2, IPT_LIGHTGUN_X )
	PORT_NAME ( "Lightpen X" )
	PORT_MINMAX( 0, THOM_TOTAL_WIDTH )
	PORT_SENSITIVITY( 50 )
	PORT_CROSSHAIR(X, 1.0, 0.0, 0)

	PORT_START ( "lightpen_y" )
	PORT_BIT ( 0xffff, THOM_TOTAL_HEIGHT/2, IPT_LIGHTGUN_Y )
	PORT_NAME ( "Lightpen Y" )
	PORT_MINMAX ( 0, THOM_TOTAL_HEIGHT )
	PORT_SENSITIVITY( 50 )
	PORT_CROSSHAIR(Y, 1.0, 0.0, 0)

	PORT_START ( "lightpen_button" )
	PORT_BIT ( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_NAME ( "Lightpen Button" )
	PORT_CODE( MOUSECODE_BUTTON1 )

INPUT_PORTS_END

/************************** T9000 / TO7 *******************************

TO7 (1982)
---

First computer by Thomson.
Note that the computer comes with only a minimal BIOS and requires an
external cartridge to be usable.
Most software are distributed on cassette and require the BASIC 1.0 cartridge
to be present (-cart basic.m7), as only it provides the necessary OS
capabilities (e.g., a cassette loader).
To use disks, you will need both a BASIC 1.0 cartridge and a BASIC DOS
boot floppy.

* chips:
  - 1 MHz Motorola 6809E CPU
  - 1 Motorola 6821 PIA (+3 for I/O, game, and modem extensions)
  - 1 Motorola 6846 timer, I/O, ROM

* memory:
  - 8 KB base user RAM
    + 16 KB extended user RAM (EM 90-016) = 24 KB total user RAM emulated
    + homebrew 8 KB RAM extension (Theophile magazine 6, sept 1984)
  - 6 KB BIOS ROM
  - 6-bit x 8 K color RAM + 8-bit x 8 K point RAM, bank switched
  - 2 to 8 KB ROM comes with the floppy drive / network controller

* video:
    320x200 pixels with color constraints (2 colors per horizontal
    8-pixel span), 8-color pixel palette,
    50 Hz (tweaked SECAM)

* devices:
  - AZERTY keyboard, 58-keys, French with accents
  - cartridge 16 KB (up to 64 KB using bank-switching),
    the MESS cartridge device is named -cart
  - cassette 900 bauds (frequency signals: 0=4.5 kHz, 1=6.3 kHz)
    the MESS cassette device is named -cass
  - 1-bit internal buzzer
  - lightpen, with 8-pixel horizontal resolution, 1-pixel vertical
  - SX 90-018 game & music extension
    . 6-bit DAC sound
    . two 8-position 2-button game pads
    . 2-button mouse
    . based on a Motorola 6821 PIA
  - CC 90-232 I/O extension:
    . CENTRONICS (printer)
    . RS232 (unemulated)
    . based on a Motorola 6821 PIA
    . NOTE: you cannot use the CENTRONICS and RS232 at the same time
  - RF 57-932: RS232 extension, based on a SY 6551 ACIA (unemulated)
  - MD 90-120: MODEM, TELETEL extension (unemulated)
    . 1 Motorola 6850 ACIA
    . 1 Motorola 6821 PIA
    . 1 EFB 7513 MODEM FSK V13, full duplex
    . PTT-, VELI7Y-, and V23-compatible MODEM (up to 1200 bauds)
    . seems to come with an extra ROM
  - 5"1/2 floppy drive extension
    . CD 90-640 floppy controller, based on a Western Digital 2793
    . DD 90-320 double-density double-sided 5"1/4 floppy
      (2 drives considered as 4 simple-face drives: 0/1 for the first drive,
       2/3 for the second drive, 1 and 3 for upper sides, 0 and 2 for lower
       sides)
    . floppies are 40 tracks/side, 16 sectors/track, 128 or 256 bytes/sector
      = from 80 KB one-sided single-density, to 320 KB two-sided double-density
    . MESS floppy devices are named -flop0 to -flop3
  - alternate 5"1/2 floppy drive extension
    . CD 90-015 floppy controller, based on a HD 46503 S
    . UD 90-070 5"1/4 single-sided single density floppy drive
  - alternate 3"1/2 floppy drive extension
    . CD 90-351 floppy controller, based on a custom Thomson gate-array
    . DD 90-352 3"1/2 floppy drives
  - alternate QDD floppy drive extension
    . CQ 90-028 floppy controller, based on a Motorola 6852 SSDA
    . QD 90-028 quickdrive 2"8 (QDD), only one drive, signe side
  - speech synthesis extension: based on a Philips / Signetics MEA 8000
    (cannot be used with the MODEM)
  - MIDIPAK MIDI extension, uses a EF 6850 ACIA
  - NR 07-005: network extension, MC 6854 based, 2 KB ROM & 64 KB RAM
    (build by the French Leanord company)


T9000 (1980)
-----

Early TO7 prototype.
The hardware seems to be the exactly same. Only the BIOS is different.
It has some bug that were corrected later for the TO7.
Actually, the two computers are undistinguishable, except for the different
startup screen, and a couple BIOS addresses.
They can run the same software and accept the same devices and extensions.


**********************************************************************/

/* ------------ address maps ------------ */

static ADDRESS_MAP_START ( to7, AS_PROGRAM, 8, thomson_state )

	AM_RANGE ( 0x0000, 0x3fff ) AM_READ_BANK ( THOM_CART_BANK ) AM_WRITE(to7_cartridge_w ) /* 4 * 16 KB */
	AM_RANGE ( 0x4000, 0x5fff ) AM_READ_BANK ( THOM_VRAM_BANK ) AM_WRITE(to7_vram_w )
	AM_RANGE ( 0x6000, 0x7fff ) AM_RAMBANK   ( THOM_BASE_BANK ) /* 1 * 8 KB */
	AM_RANGE ( 0x8000, 0xdfff ) AM_RAMBANK   ( THOM_RAM_BANK )  /* 16 or 24 KB (for extension) */
	AM_RANGE ( 0xe000, 0xe7bf ) AM_ROMBANK   ( THOM_FLOP_BANK )
	AM_RANGE ( 0xe7c0, 0xe7c7 ) AM_DEVREADWRITE("mc6846", mc6846_device, read, write)
	AM_RANGE ( 0xe7c8, 0xe7cb ) AM_DEVREADWRITE( "pia_0", pia6821_device, read_alt, write_alt )
	AM_RANGE ( 0xe7cc, 0xe7cf ) AM_DEVREADWRITE( "pia_1", pia6821_device, read_alt, write_alt )
	AM_RANGE ( 0xe7d0, 0xe7df ) AM_READWRITE(to7_floppy_r, to7_floppy_w )
	AM_RANGE ( 0xe7e0, 0xe7e3 ) AM_DEVREADWRITE( "to7_io:pia_2", pia6821_device, read_alt, write_alt )
	AM_RANGE ( 0xe7e8, 0xe7eb ) AM_DEVREADWRITE( "acia",  mos6551_device, read, write )
	AM_RANGE ( 0xe7f2, 0xe7f3 ) AM_READWRITE(to7_midi_r, to7_midi_w )
	AM_RANGE ( 0xe7f8, 0xe7fb ) AM_DEVREADWRITE( "pia_3", pia6821_device, read_alt, write_alt )
	AM_RANGE ( 0xe7fe, 0xe7ff ) AM_READWRITE(to7_modem_mea8000_r, to7_modem_mea8000_w )
	AM_RANGE ( 0xe800, 0xffff ) AM_ROM       /* system bios  */

/* 0x10000 - 0x1ffff: 64 KB external ROM cartridge */
/* 0x20000 - 0x247ff: 18 KB floppy / network ROM controllers */

/* RAM mapping:
   0x0000 - 0x3fff: 16 KB video RAM (actually 8 K x 8 bits + 8 K x 6 bits)
   0x4000 - 0x5fff:  8 KB base RAM
   0x6000 - 0x9fff: 16 KB extended RAM
   0xa000 - 0xbfff:  8 KB more extended RAM
 */
ADDRESS_MAP_END



/* ------------ ROMS ------------ */

/* external floppy controllers */
#define ROM_FLOPPY( base )                      \
		/* no controller */                     \
	ROM_FILL( base, 0x800, 0x39 )                   \
		/* CD 90-015 (5"1/4) */                     \
	ROM_LOAD ( "cd90-015.rom", base+0x800, 0x7c0,           \
		CRC(821d34c1)                       \
		SHA1(31a6bb81baaeec5fc8de457c97264f9dfa92c18b) )    \
		/* CD 90-640 (5"1/4) */                     \
	ROM_LOAD ( "cd90-640.rom", base+0x1000, 0x7c0,          \
		CRC(5114c0a5)                       \
		SHA1(5c72566c22d8160ef0c75959e1863a1309bbbe49) )    \
		/* CD 90-351 (3"1/2) */                     \
	ROM_LOAD ( "cd-351-0.rom", base+0x1800, 0x7c0,          \
		CRC(2c0159fd)                       \
		SHA1(bab5395ed8bc7c06f9897897f836054e6546e8e8) )    \
	ROM_LOAD ( "cd-351-1.rom", base+0x2000, 0x7c0,          \
		CRC(8e58d159)                       \
		SHA1(dcf992c96e7556b2faee6bacd3f744e56998e6ea) )    \
	ROM_LOAD ( "cd-351-2.rom", base+0x2800, 0x7c0,          \
		CRC(c9228b60)                       \
		SHA1(179e10107d5be91e684069dee80f94847b83201f) )    \
	ROM_LOAD ( "cd-351-3.rom", base+0x3000, 0x7c0,          \
		CRC(3ca8e5dc)                       \
		SHA1(7118636fb5c597c78c2fce17b02aed5e4ba38635) )    \
		/* CQ 90-028 (2"8, aka QDD) */                  \
	ROM_LOAD ( "cq90-028.rom", base+0x3800, 0x7c0,          \
		CRC(ca4dba3d)                       \
		SHA1(949c1f777c892da62c242215d79757d61e71e62b) )

/* external floppy / network controller: 9 banks */
#define ROM_FLOPPY5( base )             \
	ROM_FLOPPY( base )                  \
	ROM_LOAD ( "nano5.rom", base+0x4000, 0x7c0, \
			CRC(2f756868)               \
			SHA1(b5b7cb6d12493d849330b6b5628efd1a83a4bbf5) )

#define ROM_FLOPPY7( base )             \
	ROM_FLOPPY( base )                  \
	ROM_LOAD ( "nano7.rom", base+0x4000, 0x7c0, \
			CRC(42a1d1a6)               \
			SHA1(973209f4baa5e81bf7885c0602949e064bac7862) )


ROM_START ( to7 )
	ROM_REGION ( 0x24800, "maincpu", 0 )
	ROM_LOAD ( "to7.rom", 0xe800, 0x1800,
		CRC(0e7826da)
		SHA1(23a2f84b03c01d385cc1923c8ece95c43756297a) )
	ROM_FILL ( 0x10000, 0x10000, 0x39 )
	ROM_FLOPPY7 ( 0x20000 )
ROM_END

ROM_START ( t9000 )
	ROM_REGION ( 0x24800, "maincpu", 0 )
	ROM_LOAD ( "t9000.rom", 0xe800, 0x1800,
		CRC(daa8cfbf)
		SHA1(a5735db1ad4e529804fc46603f838d3f4ccaf5cf) )
	ROM_FILL ( 0x10000, 0x10000, 0x39 )
	ROM_FLOPPY7 ( 0x20000 )
ROM_END


/* ------------ inputs   ------------ */

static INPUT_PORTS_START ( to7_config )
	PORT_START ( "config" )

	PORT_CONFNAME ( 0x01, 0x00, "Game Port" )
	PORT_CONFSETTING ( 0x00, DEF_STR( Joystick ) )
	PORT_CONFSETTING ( 0x01, "Mouse" )

INPUT_PORTS_END

static INPUT_PORTS_START ( to7_vconfig )
	PORT_START ( "vconfig" )

	PORT_CONFNAME ( 0x03, 0x00, "Border" )
	PORT_CONFSETTING ( 0x00, "Normal (56x47)" )
	PORT_CONFSETTING ( 0x01, "Small (16x16)" )
	PORT_CONFSETTING ( 0x02, DEF_STR ( None ) )

	PORT_CONFNAME ( 0x0c, 0x08, "Resolution" )
	PORT_CONFSETTING ( 0x00, DEF_STR ( Low ) )
	PORT_CONFSETTING ( 0x04, DEF_STR ( High  ) )
	PORT_CONFSETTING ( 0x08, "Auto"  )

INPUT_PORTS_END

static INPUT_PORTS_START ( to7_mconfig )
	PORT_START ( "mconfig" )

	PORT_CONFNAME ( 0x01, 0x01, "E7FE-F port" )
	PORT_CONFSETTING ( 0x00, "Modem (unemulated)" )
	PORT_CONFSETTING ( 0x01, "Speech" )

INPUT_PORTS_END

static INPUT_PORTS_START ( to7_fconfig )
	PORT_START ( "fconfig" )

	PORT_CONFNAME ( 0x07, 0x03, "Floppy (reset)" )
	PORT_CONFSETTING ( 0x00, DEF_STR ( None ) )
	PORT_CONFSETTING ( 0x01, "CD 90-015 (5\"1/4 SD)" )
	PORT_CONFSETTING ( 0x02, "CD 90-640 (5\"1/4 DD)" )
	PORT_CONFSETTING ( 0x03, "CD 90-351 (3\"1/2)" )
	PORT_CONFSETTING ( 0x04, "CQ 90-028 (2\"8 QDD)" )
	PORT_CONFSETTING ( 0x05, "Network" )

	PORT_CONFNAME ( 0xf8, 0x08, "Network ID" )
	PORT_CONFSETTING ( 0x00, "0 (Master)" )
	PORT_CONFSETTING ( 0x08, "1" )
	PORT_CONFSETTING ( 0x10, "2" )
	PORT_CONFSETTING ( 0x18, "3" )
	PORT_CONFSETTING ( 0x20, "4" )
	PORT_CONFSETTING ( 0x28, "5" )
	PORT_CONFSETTING ( 0x30, "6" )
	PORT_CONFSETTING ( 0x38, "7" )
	PORT_CONFSETTING ( 0x40, "8" )
	PORT_CONFSETTING ( 0x48, "9" )
	PORT_CONFSETTING ( 0x50, "10" )
	PORT_CONFSETTING ( 0x58, "11" )
	PORT_CONFSETTING ( 0x60, "12" )
	PORT_CONFSETTING ( 0x68, "13" )
	PORT_CONFSETTING ( 0x70, "14" )
	PORT_CONFSETTING ( 0x78, "15" )
	PORT_CONFSETTING ( 0x80, "16" )
	PORT_CONFSETTING ( 0x88, "17" )
	PORT_CONFSETTING ( 0x90, "18" )
	PORT_CONFSETTING ( 0x98, "19" )
	PORT_CONFSETTING ( 0xa0, "20" )
	PORT_CONFSETTING ( 0xa8, "21" )
	PORT_CONFSETTING ( 0xb0, "22" )
	PORT_CONFSETTING ( 0xb8, "23" )
	PORT_CONFSETTING ( 0xc0, "24" )
	PORT_CONFSETTING ( 0xc8, "25" )
	PORT_CONFSETTING ( 0xd0, "26" )
	PORT_CONFSETTING ( 0xd8, "27" )
	PORT_CONFSETTING ( 0xe0, "28" )
	PORT_CONFSETTING ( 0xe8, "29" )
	PORT_CONFSETTING ( 0xf0, "30" )
	PORT_CONFSETTING ( 0xf8, "31" )

INPUT_PORTS_END


static INPUT_PORTS_START ( to7_keyboard )
	PORT_START ( "keyboard.0" )
		KEY ( 0, "Shift", LSHIFT ) PORT_CODE ( KEYCODE_RSHIFT ) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT  ( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START ( "keyboard.1" )
		KEY ( 0, "W", W )                PORT_CHAR('W')
		KEY ( 1, UTF8_UP, UP )    PORT_CHAR(UCHAR_MAMEKEY(UP))
		KEY ( 2, "C \303\247", C )       PORT_CHAR('C')
		KEY ( 3, "Clear", ESC )          PORT_CHAR(UCHAR_MAMEKEY(ESC))
		KEY ( 4, "Enter", ENTER )        PORT_CHAR(13)
		KEY ( 5, "Control", LCONTROL )   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
		KEY ( 6, "Accent", END )         PORT_CHAR(UCHAR_MAMEKEY(END))
		KEY ( 7, "Stop", TAB )           PORT_CHAR(27)
	PORT_START ( "keyboard.2" )
		KEY ( 0, "X", X )                PORT_CHAR('X')
		KEY ( 1, UTF8_LEFT, LEFT )  PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		KEY ( 2, "V", V )                PORT_CHAR('V')
		KEY ( 3, "Q", Q )                PORT_CHAR('Q')
		KEY ( 4, "* :", QUOTE )          PORT_CHAR('*') PORT_CHAR(':')
		KEY ( 5, "A", A )                PORT_CHAR('A')
		KEY ( 6, "+ ;", EQUALS )         PORT_CHAR('+') PORT_CHAR(';')
		KEY ( 7, "1 !", 1 )              PORT_CHAR('1') PORT_CHAR('!')
	PORT_START ( "keyboard.3" )
		KEY ( 0, "Space Caps-Lock", SPACE ) PORT_CHAR(' ') PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
		KEY ( 1, UTF8_DOWN, DOWN )  PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		KEY ( 2, "B", B )                PORT_CHAR('B')
		KEY ( 3, "S", S )                PORT_CHAR('S')
		KEY ( 4, "/ ?", SLASH )          PORT_CHAR('/') PORT_CHAR('?')
		KEY ( 5, "Z \305\223", Z)        PORT_CHAR('Z')
		KEY ( 6, "- =", MINUS )          PORT_CHAR('-') PORT_CHAR('=')
		KEY ( 7, "2 \" \302\250", 2 )    PORT_CHAR('2') PORT_CHAR('"')
	PORT_START ( "keyboard.4" )
		KEY ( 0, "@ \342\206\221", TILDE ) PORT_CHAR('@')
		KEY ( 1, UTF8_RIGHT, RIGHT ) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		KEY ( 2, "M", M )                PORT_CHAR('M')
		KEY ( 3, "D", D )                PORT_CHAR('D')
		KEY ( 4, "P", P )                PORT_CHAR('P')
		KEY ( 5, "E", E )                PORT_CHAR('E')
		KEY ( 6, "0 \140", 0 )           PORT_CHAR('0') PORT_CHAR( 0140 )
		KEY ( 7, "3 #", 3 )              PORT_CHAR('3') PORT_CHAR('#')
	PORT_START ( "keyboard.5" )
		KEY ( 0, ". >", STOP )           PORT_CHAR('.') PORT_CHAR('>')
		KEY ( 1, "Home", HOME )          PORT_CHAR(UCHAR_MAMEKEY(HOME))
		KEY ( 2, "L", L )                PORT_CHAR('L')
		KEY ( 3, "F", F )                PORT_CHAR('F')
		KEY ( 4, "O", O )                PORT_CHAR('O')
		KEY ( 5, "R", R )                PORT_CHAR('R')
		KEY ( 6, "9 )", 9 )              PORT_CHAR('9') PORT_CHAR(')')
		KEY ( 7, "4 $", 4 )              PORT_CHAR('4') PORT_CHAR('$')
	PORT_START ( "keyboard.6" )
		KEY ( 0, ", <", COMMA )          PORT_CHAR(',') PORT_CHAR('<')
		KEY ( 1, "Insert", INSERT )      PORT_CHAR(UCHAR_MAMEKEY(INSERT))
		KEY ( 2, "K", K )                PORT_CHAR('K')
		KEY ( 3, "G", G )                PORT_CHAR('G')
		KEY ( 4, "I", I )                PORT_CHAR('I')
		KEY ( 5, "T", T )                PORT_CHAR('T')
		KEY ( 6, "8 (", 8 )              PORT_CHAR('8') PORT_CHAR('(')
		KEY ( 7, "5 %", 5 )              PORT_CHAR('5') PORT_CHAR('%')
	PORT_START ( "keyboard.7" )
		KEY ( 0, "N", N )                PORT_CHAR('N')
		KEY ( 1, "Delete", DEL )         PORT_CHAR(8)
		KEY ( 2, "J \305\222", J )       PORT_CHAR('J')
		KEY ( 3, "H \302\250", H )       PORT_CHAR('H')
		KEY ( 4, "U", U )                PORT_CHAR('U')
		KEY ( 5, "Y", Y )                PORT_CHAR('Y')
		KEY ( 6, "7 ' \302\264", 7 )     PORT_CHAR('7') PORT_CHAR('\'')
		KEY ( 7, "6 &", 6 )              PORT_CHAR('6') PORT_CHAR('&')

		/* unused */
	PORT_START ( "keyboard.8" )
	PORT_START ( "keyboard.9" )

INPUT_PORTS_END

static INPUT_PORTS_START ( to7 )
	PORT_INCLUDE ( thom_lightpen )
	PORT_INCLUDE ( thom_game_port )
	PORT_INCLUDE ( to7_keyboard )
	PORT_INCLUDE ( to7_config )
	PORT_INCLUDE ( to7_fconfig )
	PORT_INCLUDE ( to7_vconfig )
	PORT_INCLUDE ( to7_mconfig )
INPUT_PORTS_END

static INPUT_PORTS_START ( t9000 )
	PORT_INCLUDE ( to7 )
INPUT_PORTS_END

WRITE_LINE_MEMBER( thomson_state::fdc_index_0_w )
{
	thomson_index_callback(machine().device<legacy_floppy_image_device>(FLOPPY_0), state);
}

WRITE_LINE_MEMBER( thomson_state::fdc_index_1_w )
{
	thomson_index_callback(machine().device<legacy_floppy_image_device>(FLOPPY_1), state);
}

WRITE_LINE_MEMBER( thomson_state::fdc_index_2_w )
{
	thomson_index_callback(machine().device<legacy_floppy_image_device>(FLOPPY_2), state);
}

WRITE_LINE_MEMBER( thomson_state::fdc_index_3_w )
{
	thomson_index_callback(machine().device<legacy_floppy_image_device>(FLOPPY_3), state);
}

static const floppy_interface thomson_floppy_interface =
{
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(thomson),
	nullptr
};

FLOPPY_FORMATS_MEMBER( thomson_state::cd90_640_formats )
	FLOPPY_CD90_640_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( cd90_640_floppies )
	SLOT_INTERFACE("sssd", FLOPPY_525_SSSD)
	SLOT_INTERFACE("sd",   FLOPPY_525_SD)
	SLOT_INTERFACE("ssdd", FLOPPY_525_SSDD)
	SLOT_INTERFACE("dd",   FLOPPY_525_DD)
SLOT_INTERFACE_END


/* ------------ driver ------------ */

static MACHINE_CONFIG_START( to7, thomson_state )

	MCFG_MACHINE_START_OVERRIDE( thomson_state, to7 )
	MCFG_MACHINE_RESET_OVERRIDE( thomson_state, to7 )

/* cpu */
	MCFG_CPU_ADD ( "maincpu", M6809, 1000000 )
	MCFG_CPU_PROGRAM_MAP ( to7)

/* video */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE ( /*50*/ 1./0.019968 )
	MCFG_SCREEN_SIZE ( THOM_TOTAL_WIDTH * 2, THOM_TOTAL_HEIGHT )
	MCFG_SCREEN_VISIBLE_AREA ( 0, THOM_TOTAL_WIDTH * 2 - 1,
				0, THOM_TOTAL_HEIGHT - 1 )
	MCFG_SCREEN_UPDATE_DRIVER( thomson_state, screen_update_thom )
	MCFG_SCREEN_VBLANK_DRIVER( thomson_state, thom_vblank )
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD ( "palette", 4097 ) /* 12-bit color + transparency */
	MCFG_PALETTE_INIT_OWNER(thomson_state, thom)
	MCFG_VIDEO_START_OVERRIDE( thomson_state, thom )

/* sound */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD ( "buzzer", DAC, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.) /* 1-bit buzzer */
	MCFG_SOUND_ADD ( "dac", DAC, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.) /* 6-bit game extention DAC */
	MCFG_SOUND_ADD ( "speech", DAC, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.) /* speech synthesis */

/* cassette */
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(to7_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)

/* timer */
	MCFG_DEVICE_ADD("mc6846", MC6846, 0)
	MCFG_MC6846_OUT_PORT_CB(WRITE8(thomson_state, to7_timer_port_out))
	MCFG_MC6846_OUT_CP2_CB(WRITE8(thomson_state, to7_timer_cp2_out))
	MCFG_MC6846_IN_PORT_CB(READ8(thomson_state, to7_timer_port_in))
	MCFG_MC6846_OUT_CTO_CB(WRITE8(thomson_state, to7_timer_tco_out))
	MCFG_MC6846_IRQ_CB(WRITELINE(thomson_state, thom_dev_irq_0))

/* speech synthesis */
	MCFG_DEVICE_ADD("mea8000", MEA8000, 0)
	MCFG_MEA8000_DAC("speech")

/* floppy */
	MCFG_DEVICE_ADD("mc6843", MC6843, 0)

	MCFG_WD2793_ADD("wd2793", XTAL_1MHz)

	MCFG_FLOPPY_DRIVE_ADD("wd2793:0", cd90_640_floppies, "dd", thomson_state::cd90_640_formats)
	MCFG_FLOPPY_DRIVE_ADD("wd2793:1", cd90_640_floppies, "dd", thomson_state::cd90_640_formats)

	MCFG_DEVICE_ADD(FLOPPY_0, LEGACY_FLOPPY, 0)
	MCFG_DEVICE_CONFIG(thomson_floppy_interface)
	MCFG_LEGACY_FLOPPY_IDX_CB(WRITELINE(thomson_state, fdc_index_0_w))
	MCFG_DEVICE_ADD(FLOPPY_1, LEGACY_FLOPPY, 0)
	MCFG_DEVICE_CONFIG(thomson_floppy_interface)
	MCFG_LEGACY_FLOPPY_IDX_CB(WRITELINE(thomson_state, fdc_index_1_w))
	MCFG_DEVICE_ADD(FLOPPY_2, LEGACY_FLOPPY, 0)
	MCFG_DEVICE_CONFIG(thomson_floppy_interface)
	MCFG_LEGACY_FLOPPY_IDX_CB(WRITELINE(thomson_state, fdc_index_2_w))
	MCFG_DEVICE_ADD(FLOPPY_3, LEGACY_FLOPPY, 0)
	MCFG_DEVICE_CONFIG(thomson_floppy_interface)
	MCFG_LEGACY_FLOPPY_IDX_CB(WRITELINE(thomson_state, fdc_index_3_w))

/* network */
	MCFG_DEVICE_ADD( "mc6854", MC6854, 0 )
	MCFG_MC6854_OUT_FRAME_CB(thomson_state, to7_network_got_frame)


/* pia */
	MCFG_DEVICE_ADD(THOM_PIA_SYS, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(thomson_state, to7_sys_porta_in))
	MCFG_PIA_READPB_HANDLER(READ8(thomson_state, to7_sys_portb_in))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(thomson_state, to7_sys_portb_out))
	MCFG_PIA_CA2_HANDLER(WRITELINE(thomson_state, to7_set_cassette_motor))
	MCFG_PIA_CB2_HANDLER(WRITELINE(thomson_state, to7_sys_cb2_out))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(thomson_state, thom_firq_1))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(thomson_state, thom_firq_1))

	MCFG_DEVICE_ADD(THOM_PIA_GAME, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(thomson_state, to7_game_porta_in))
	MCFG_PIA_READPB_HANDLER(READ8(thomson_state, to7_game_portb_in))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(thomson_state, to7_game_portb_out))
	MCFG_PIA_CB2_HANDLER(WRITELINE(thomson_state, to7_game_cb2_out))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(thomson_state, thom_irq_1))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(thomson_state, thom_irq_1))

/* TODO: CONVERT THIS TO A SLOT DEVICE (RF 57-932) */
	MCFG_DEVICE_ADD("acia", MOS6551, 0)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
	MCFG_MOS6551_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))

	/// 2400 7N2
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia", mos6551_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("acia", mos6551_device, write_dcd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("acia", mos6551_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("acia", mos6551_device, write_cts))


/* TODO: CONVERT THIS TO A SLOT DEVICE (CC 90-232) */
	MCFG_TO7_IO_LINE_ADD("to7_io")


/* TODO: CONVERT THIS TO A SLOT DEVICE (MD 90-120) */
	MCFG_DEVICE_ADD(THOM_PIA_MODEM, PIA6821, 0)

	MCFG_DEVICE_ADD("acia6850", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(thomson_state, to7_modem_tx_w))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(thomson_state, to7_modem_cb))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 1200) /* 1200 bauds, might be divided by 16 */
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(thomson_state, write_acia_clock))


/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "to7_cart")
	MCFG_GENERIC_EXTENSIONS("m7,rom")
	MCFG_GENERIC_LOAD(thomson_state, to7_cartridge)

	MCFG_SOFTWARE_LIST_ADD("cart_list","to7_cart")

/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("40K")
	MCFG_RAM_EXTRA_OPTIONS("24K,48K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( t9000, to7 )
MACHINE_CONFIG_END


COMP ( 1982, to7, 0, 0, to7, to7, driver_device, 0,  "Thomson", "TO7", 0 )

COMP ( 1980, t9000, to7, 0, t9000, t9000, driver_device,  0, "Thomson", "T9000", 0 )


/***************************** TO7/70 *********************************

TO7/70 ( 1984)
------

Enhanced TO7.
The TO7/70 supports virtually all TO7 software and most TO7 devices and
extensions (floppy, game, communucation, etc.).
As the TO7, it is only usable with a cartridge, and most software require
the BASIC 1.0 cartridge to be present.
Though, you may also use the more advanced BASIC 128 (-cart basic128.m7):
it allows BASIC programs to access all the memory and the video capabilities,
includes its own DOS (no need for a boot disk), but may not be compatible
with all games.

It has the following modifications:

* chips:
  - custom logics for video, lightpen, address map has been replaced with an
    integrated Gate-Array (Motorola MC 1300 ALS)

* memory:
  - 48 KB user base RAM (16 KB unswitchable + 2 switchable banks of 16 KB)
    + 64 KB user extended RAM (EM 97-064, as 4 extra 16 KB banks)
    = 112 KB total user RAM emulated
  - now 8-bit x 8 K color RAM (instead of 6-bit x 8 K)

* video:
  - 16-color fixed palette instead of 8-color (but same constraints)
  - IN 57-001: TV overlay extension, not implemented
    (black becomes transparent and shows the TV image)

* devices:
  - lightpen management has changed, it now has 1-pixel horizontal resolution
  - keyboard management has changed (but the keys are the same)


TO7/70 arabic (198?)
-------------

TO7/70 with an alternate ROM.
Together with a special (64 KB) BASIC 128 cartridge (-cart basic128a.m7),
it allows typing in arabic.
Use Ctrl+W to switch to arabic, and Ctrl+F to switch back to latin.
In latin mode, Ctrl+U / Ctrl+X to start / stop typing in-line arabic.
In arabic mode, Ctrl+E / Ctrl+X to start / stop typing in-line latin.

**********************************************************************/

static ADDRESS_MAP_START ( to770, AS_PROGRAM, 8, thomson_state )

	AM_RANGE ( 0x0000, 0x3fff ) AM_READ_BANK ( THOM_CART_BANK) AM_WRITE(to7_cartridge_w ) /* 4 * 16 KB */
	AM_RANGE ( 0x4000, 0x5fff ) AM_READ_BANK ( THOM_VRAM_BANK) AM_WRITE(to770_vram_w )
	AM_RANGE ( 0x6000, 0x9fff ) AM_RAMBANK   ( THOM_BASE_BANK ) /* 16 KB */
	AM_RANGE ( 0xa000, 0xdfff ) AM_RAMBANK   ( THOM_RAM_BANK )  /* 6 * 16 KB */
	AM_RANGE ( 0xe000, 0xe7bf ) AM_ROMBANK   ( THOM_FLOP_BANK )
	AM_RANGE ( 0xe7c0, 0xe7c7 ) AM_DEVREADWRITE("mc6846", mc6846_device, read, write)
	AM_RANGE ( 0xe7c8, 0xe7cb ) AM_DEVREADWRITE( "pia_0", pia6821_device, read_alt, write_alt )
	AM_RANGE ( 0xe7cc, 0xe7cf ) AM_DEVREADWRITE( "pia_1", pia6821_device, read_alt, write_alt )
	AM_RANGE ( 0xe7d0, 0xe7df ) AM_READWRITE(to7_floppy_r, to7_floppy_w )
	AM_RANGE ( 0xe7e0, 0xe7e3 ) AM_DEVREADWRITE( "to7_io:pia_2", pia6821_device, read_alt, write_alt )
	AM_RANGE ( 0xe7e4, 0xe7e7 ) AM_READWRITE(to770_gatearray_r, to770_gatearray_w )
	AM_RANGE ( 0xe7e8, 0xe7eb ) AM_DEVREADWRITE( "acia",  mos6551_device, read, write )
	AM_RANGE ( 0xe7f2, 0xe7f3 ) AM_READWRITE(to7_midi_r, to7_midi_w )
	AM_RANGE ( 0xe7f8, 0xe7fb ) AM_DEVREADWRITE( "pia_3", pia6821_device, read_alt, write_alt )
	AM_RANGE ( 0xe7fe, 0xe7ff ) AM_READWRITE(to7_modem_mea8000_r, to7_modem_mea8000_w )
	AM_RANGE ( 0xe800, 0xffff ) AM_ROM       /* system bios  */

/* 0x10000 - 0x1ffff: 64 KB external ROM cartridge */
/* 0x20000 - 0x247ff: 18 KB floppy / network ROM controllers */

/* RAM mapping:
   0x00000 - 0x03fff: 16 KB video RAM
   0x04000 - 0x07fff: 16 KB unbanked base RAM
   0x08000 - 0x1ffff: 6 * 16 KB banked extended RAM
 */

ADDRESS_MAP_END



/* ------------ ROMS ------------ */

ROM_START ( to770 )
	ROM_REGION ( 0x24800, "maincpu", 0 )
	ROM_LOAD ( "to770.rom", 0xe800, 0x1800, /* BIOS */
		CRC(89518862)
		SHA1(cd34474c0bcc758f6d71c90fbd40cef379d61374) )
	ROM_FLOPPY7 ( 0x20000 )
	ROM_FILL ( 0x10000, 0x10000, 0x39 )
ROM_END

ROM_START ( to770a )
	ROM_REGION ( 0x24800, "maincpu", 0 )
	ROM_LOAD ( "to770a.rom", 0xe800, 0x1800,
		CRC(378ea808)
		SHA1(f4575b537dfdb46ff2a0e7cbe8dfe4ba63161b8e) )
	ROM_FLOPPY7 ( 0x20000 )
	ROM_FILL ( 0x10000, 0x10000, 0x39 )
ROM_END


/* ------------ inputs   ------------ */

static INPUT_PORTS_START ( to770 )
	PORT_INCLUDE ( to7 )

	PORT_MODIFY ( "keyboard.1" )
		KEY ( 2, "C \302\250 \303\247", C )   PORT_CHAR('C')
	PORT_MODIFY ( "keyboard.4" )
		KEY ( 6, "0 \140 \303\240", 0 )       PORT_CHAR('0') PORT_CHAR( 0140 )
	PORT_MODIFY ( "keyboard.5" )
		KEY ( 6, "9 ) \303\247", 9 )          PORT_CHAR('9') PORT_CHAR(')')
	PORT_MODIFY ( "keyboard.6" )
		KEY ( 6, "8 ( \303\271", 8 )          PORT_CHAR('8') PORT_CHAR('(')
	PORT_MODIFY ( "keyboard.7" )
		KEY ( 6, "7 ' \303\250 \302\264", 7 ) PORT_CHAR('7') PORT_CHAR('\'')
		KEY ( 7, "6 & \303\251", 6 )          PORT_CHAR('6') PORT_CHAR('&')

INPUT_PORTS_END

/* arabic version (QWERTY keyboard) */
static INPUT_PORTS_START ( to770a )
	PORT_INCLUDE ( to770 )

	PORT_MODIFY ( "keyboard.1" )
		KEY ( 0, "Z", Z )                     PORT_CHAR('Z')
	PORT_MODIFY ( "keyboard.2" )
		KEY ( 3, "A", A )                     PORT_CHAR('A')
		KEY ( 4, "/ ?", QUOTE )               PORT_CHAR('/') PORT_CHAR('?')
		KEY ( 5, "Q", Q )                     PORT_CHAR('Q')
	PORT_MODIFY ( "keyboard.3" )
		KEY ( 4, "* :", SLASH )               PORT_CHAR('*') PORT_CHAR(':')
		KEY ( 5, "W", W)                      PORT_CHAR('W')
	PORT_MODIFY ( "keyboard.4" )
		KEY ( 0, ". >", STOP )                PORT_CHAR('.') PORT_CHAR('>')
		KEY ( 2, "@ \342\206\221", TILDE )    PORT_CHAR('@') PORT_CHAR('^')
		KEY ( 6, "0 \302\243 \302\260 \140", 0 )   PORT_CHAR('0') PORT_CHAR( 0140 )
	PORT_MODIFY ( "keyboard.5" )
		KEY ( 0, ", <", COMMA )               PORT_CHAR(',') PORT_CHAR('<')
		KEY ( 6, "9 ) \303\261", 9 )          PORT_CHAR('9') PORT_CHAR(')')
	PORT_MODIFY ( "keyboard.6" )
		KEY ( 0, "M", M )                     PORT_CHAR('M')
		KEY ( 6, "8 ( \303\274", 8 )          PORT_CHAR('8') PORT_CHAR('(')
	PORT_MODIFY ( "keyboard.7" )
		KEY ( 6, "7 ' \303\266 \302\264", 7 ) PORT_CHAR('7') PORT_CHAR('\'')
		KEY ( 7, "6 & \303\244", 6 )          PORT_CHAR('6') PORT_CHAR('&')

INPUT_PORTS_END


/* ------------ driver ------------ */

static MACHINE_CONFIG_DERIVED( to770, to7 )
	MCFG_MACHINE_START_OVERRIDE( thomson_state, to770 )
	MCFG_MACHINE_RESET_OVERRIDE( thomson_state, to770 )

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP ( to770)

	MCFG_DEVICE_MODIFY(THOM_PIA_SYS)
	MCFG_PIA_READPA_HANDLER(READ8(thomson_state, to770_sys_porta_in))
	MCFG_PIA_READPB_HANDLER(NULL)
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(thomson_state, to770_sys_portb_out))
	MCFG_PIA_CB2_HANDLER(WRITELINE(thomson_state, to770_sys_cb2_out))

	MCFG_DEVICE_MODIFY("mc6846")
	MCFG_MC6846_OUT_PORT_CB(WRITE8(thomson_state, to770_timer_port_out))

	MCFG_DEVICE_REMOVE("cartslot")
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "to770_cart")
	MCFG_GENERIC_EXTENSIONS("m7,rom")
	MCFG_GENERIC_LOAD(thomson_state, to7_cartridge)

	MCFG_DEVICE_REMOVE("cart_list")
	MCFG_SOFTWARE_LIST_ADD("cart_list","to770_cart")

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("64K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( to770a, to770 )
MACHINE_CONFIG_END

COMP ( 1984, to770, 0, 0, to770, to770, driver_device, 0, "Thomson", "TO7/70", 0 )

COMP ( 1984, to770a, to770, 0, to770a, to770a, driver_device, 0, "Thomson", "TO7/70 (Arabic)", 0 )


/************************* MO5 / MO5E *********************************

MO5 (1984)
---

The MO5 is Thomson's attempt to provide a less costly micro-computer, using
the same technology as the TO7/70.
It has less memory and is less expandable. The MC6846 timer has disappeared.
The BIOS has been throughly rewritten and uses a more compact call scheme.
This, and the fact that the address map has changed, makes the MO5 completely
TO7 software incompatible (except for pure BASIC programs).
Moreover, the MO5 has incompatible cassette and cartridge formats.
Unlike the TO7, the BASIC 1.0 is integrated and the MO5 can be used "as-is".

* chips:
  - 1 MHz Motorola 6809E CPU
  - 1 Motorola 6821 PIA (+3 for I/O, game, and modem extensions)
  - Motorola 1300 ALS Gate-Array

* memory:
  - 32 KB base user RAM
  - 64 KB extended user RAM (4 x 16 KB banks) with the network extension
    (no available to BASIC programs)
  - 16 KB combined BASIC and BIOS ROM
  - 8 KB color RAM + 8 KB point RAM, bank switched
  - 2 to 8 KB floppy ROM comes with the floppy drive / network extension

* video:
  - as the TO7/70 but with different color encoding,
    320x200 pixels with color constraints, 16-color fixed palette
  - IN 57-001: TV overlay extension (not implemented)

* devices:
  - AZERTY keyboard, 58-keys, slightlty different from the TO7
    . the right SHIFT key has been replaced with a BASIC key
    . no caps-lock led
  - the famous lightpen is optional
  - cassette 1200 bauds (frequency signals: 0=4.5kHz, 1=6.3kHz),
    TO7-incompatible
  - optional cartridge, up to 64 KB, incompatible with TO7,
    masks the integrated BASIC ROM
  - game & music, I/O, floppy, network extensions: identical to TO7
  - speech synthesis extension: identical to TO7
  - MIDIPAK MIDI extension: identical to TO7

MO5E (1986)
----

This is a special MO5 version for the export market (mainly Germany).
Although coming in a different (nicer) case, it is internally similar to
the MO5 and is fully compatible.
Differences include:
 - much better keyboard; some are QWERTY instead of AZERTY (we emulate QWERTY)
 - a different BIOS and integrated BASIC
 - the game extension is integrated


**********************************************************************/

static ADDRESS_MAP_START ( mo5, AS_PROGRAM, 8, thomson_state )

	AM_RANGE ( 0x0000, 0x1fff ) AM_READ_BANK ( THOM_VRAM_BANK ) AM_WRITE(to770_vram_w )
	AM_RANGE ( 0x2000, 0x9fff ) AM_RAMBANK   ( THOM_BASE_BANK )
	AM_RANGE ( 0xa000, 0xa7bf ) AM_ROMBANK   ( THOM_FLOP_BANK )
	AM_RANGE ( 0xa7c0, 0xa7c3 ) AM_DEVREADWRITE( "pia_0", pia6821_device, read_alt, write_alt )
	AM_RANGE ( 0xa7cb, 0xa7cb ) AM_WRITE(mo5_ext_w )
	AM_RANGE ( 0xa7cc, 0xa7cf ) AM_DEVREADWRITE( "pia_1", pia6821_device, read_alt, write_alt )
	AM_RANGE ( 0xa7d0, 0xa7df ) AM_READWRITE(to7_floppy_r, to7_floppy_w )
	AM_RANGE ( 0xa7e0, 0xa7e3 ) AM_DEVREADWRITE( "to7_io:pia_2", pia6821_device, read_alt, write_alt )
	AM_RANGE ( 0xa7e4, 0xa7e7 ) AM_READWRITE(mo5_gatearray_r, mo5_gatearray_w )
	AM_RANGE ( 0xa7e8, 0xa7eb ) AM_DEVREADWRITE( "acia",  mos6551_device, read, write )
	AM_RANGE ( 0xa7f2, 0xa7f3 ) AM_READWRITE(to7_midi_r, to7_midi_w )
	AM_RANGE ( 0xa7fe, 0xa7ff ) AM_DEVREADWRITE("mea8000", mea8000_device, read, write)
	AM_RANGE ( 0xb000, 0xefff ) AM_READ_BANK ( THOM_CART_BANK) AM_WRITE(mo5_cartridge_w )
	AM_RANGE ( 0xf000, 0xffff ) AM_ROM       /* system bios */

/* 0x10000 - 0x1ffff: 16 KB integrated BASIC / 64 KB external cartridge */
/* 0x20000 - 0x247ff: 18 KB floppy / network ROM controllers */

/* RAM mapping:
   0x00000 - 0x03fff: 16 KB video RAM
   0x04000 - 0x0bfff: 32 KB unbanked base RAM
   0x0c000 - 0x1bfff: 4 * 16 KB bank extended RAM
 */

ADDRESS_MAP_END



/* ------------ ROMS ------------ */

ROM_START ( mo5 )
	ROM_REGION ( 0x24800, "maincpu", 0 )
	ROM_LOAD ( "mo5.rom", 0xf000, 0x1000,
		CRC(f0ea9140)
		SHA1(36ce2d3df1866ec2fe368c1c28757e2f5401cf44) )
	ROM_LOAD ( "basic5.rom", 0x11000, 0x3000,
		CRC(c2c11b9d)
		SHA1(512dd40fb45bc2b51a24c84b3723a32bc8e80c06) )
	ROM_FLOPPY5 ( 0x20000 )
ROM_END

ROM_START ( mo5e )
	ROM_REGION ( 0x24800, "maincpu", 0 )
	ROM_LOAD ( "mo5e.rom", 0xf000, 0x1000,
		CRC(6520213a)
		SHA1(f17a7a59baf2819ec80991b34b204795536a5e01) )
	ROM_LOAD ( "basic5e.rom", 0x11000, 0x3000,
		CRC(934a72b2)
		SHA1(b37e2b1afbfba368c19be87b3bf61dfe6ad8b0bb) )
	ROM_FLOPPY5 ( 0x20000 )
ROM_END


/* ------------ inputs  ------------ */

static INPUT_PORTS_START ( mo5 )
	PORT_INCLUDE ( to770 )

	PORT_MODIFY ( "keyboard.0" )
		KEY ( 1, "BASIC", RCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT  ( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END

/* QWERTY version */
static INPUT_PORTS_START ( mo5e )
	PORT_INCLUDE ( mo5 )

	PORT_MODIFY ( "keyboard.1" )
		KEY ( 0, "Z", Z )                     PORT_CHAR('Z')
	PORT_MODIFY ( "keyboard.2" )
		KEY ( 3, "A", A )                     PORT_CHAR('A')
		KEY ( 5, "Q", Q )                     PORT_CHAR('Q')
	PORT_MODIFY ( "keyboard.3" )
		KEY ( 5, "W", W)                      PORT_CHAR('W')
	PORT_MODIFY ( "keyboard.4" )
		KEY ( 0, ". >", STOP )                PORT_CHAR('.') PORT_CHAR('>')
		KEY ( 2, "@ \342\206\221", TILDE )    PORT_CHAR('@') PORT_CHAR('^')
		KEY ( 6, "0 \302\243 \302\260 \140", 0 )   PORT_CHAR('0') PORT_CHAR( 0140 )
	PORT_MODIFY ( "keyboard.5" )
		KEY ( 0, ", <", COMMA )               PORT_CHAR(',') PORT_CHAR('<')
		KEY ( 6, "9 ) \303\261", 9 )          PORT_CHAR('9') PORT_CHAR(')')
	PORT_MODIFY ( "keyboard.6" )
		KEY ( 0, "M", M )                     PORT_CHAR('M')
		KEY ( 6, "8 ( \303\274", 8 )          PORT_CHAR('8') PORT_CHAR('(')
	PORT_MODIFY ( "keyboard.7" )
		KEY ( 6, "7 ' \303\266 \302\264", 7 ) PORT_CHAR('7') PORT_CHAR('\'')
		KEY ( 7, "6 & \303\244", 6 )          PORT_CHAR('6') PORT_CHAR('&')

INPUT_PORTS_END

/* ------------ driver ------------ */

static MACHINE_CONFIG_DERIVED( mo5, to7 )
	MCFG_MACHINE_START_OVERRIDE( thomson_state, mo5 )
	MCFG_MACHINE_RESET_OVERRIDE( thomson_state, mo5 )

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP ( mo5)

	MCFG_CASSETTE_MODIFY( "cassette" )
	MCFG_CASSETTE_FORMATS(mo5_cassette_formats)

	MCFG_DEVICE_REMOVE( "mc6846" )

		MCFG_PALETTE_MODIFY( "palette" )
	MCFG_PALETTE_INIT_OWNER(thomson_state, mo5)

	MCFG_DEVICE_MODIFY(THOM_PIA_SYS)
	MCFG_PIA_READPA_HANDLER(READ8(thomson_state, mo5_sys_porta_in))
	MCFG_PIA_READPB_HANDLER(READ8(thomson_state, mo5_sys_portb_in))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(thomson_state, mo5_sys_porta_out))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(thomson_state, mo5_sys_portb_out))
	MCFG_PIA_CA2_HANDLER(WRITELINE(thomson_state, mo5_set_cassette_motor))
	MCFG_PIA_CB2_HANDLER(NULL)
	MCFG_PIA_IRQB_HANDLER(WRITELINE(thomson_state, thom_irq_1)) /* WARNING: differs from TO7 ! */

	MCFG_DEVICE_REMOVE("cartslot")
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "mo5_cart")
	MCFG_GENERIC_EXTENSIONS("m5,rom")
	MCFG_GENERIC_LOAD(thomson_state, mo5_cartridge)

	MCFG_DEVICE_REMOVE("cart_list")
	MCFG_SOFTWARE_LIST_ADD("cart_list","mo5_cart")

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("112K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mo5e, mo5 )
MACHINE_CONFIG_END


COMP ( 1984, mo5, 0, 0, mo5, mo5, driver_device, 0, "Thomson", "MO5", 0 )

COMP ( 1986, mo5e, mo5, 0, mo5e, mo5e, driver_device, 0, "Thomson", "MO5E", 0 )


/********************************* TO9 *******************************

TO9 (1985)
---

The TO9 is the successor of the TO7/70.
It is a high-end product: it integrates 96 KB of base RAM, 128 KB of
software in ROM, a floppy drive. It has improved graphics capabilities
(several video modes, a palette of 4096 colors, thanks to the use of
a dedicated video gate-array).
The ROM contains the old BASIC 1.0 for compatibility and the newer BASIC 128.
It has a more professional, desktop look, with a separate keyboard, and an
optional mouse.
It is also quite compatible with the TO7 and TO7/70 (but not the MO5).
However, it also has many problems. The integrated BASIC 128 can only access
128 KB of memory, which forces the 64 KB extension to be managed as a virtual
disk. The early versions of the software ROM has many bugs. The integrated
floppy drive is one-sided.
It was replaced quickly with the improved TO9+.

* chips:
  - 1 MHz Motorola 6809E CPU
  - 1 Motorola 6821 PIA (+2 for game, modem extensions)
  - 1 Motorola 6846 timer, PIA
  - 1 Motorola 6805 + 1 Motorola 6850 (keyboard & mouse control)
  - 1 Western Digital 2793 (disk controller)
  - 3 gate-arrays (address decoding, system, video)

* memory:
  - 112 KB base RAM
  - 64 KB extension RAM (as virtual disk)
  - 6 KB BIOS ROM + 2 KB floppy BIOS
  - 128 KB software ROM (BASIC 1, BASIC 128, extended BIOS,
    DOS and configuration GUI, two software: "Paragraphe" and
    "Fiches et dossiers")
  - 16 KB video RAM

* video:
  - 8 video modes:
    o 320x200, 16 colors with constraints (TO7/70 compatible)
    o 320x200, 4 colors without constraints
    o 160x200, 16 colors without constraints
    o 640x200, 2 colors
    o 320x200, 2 colors, two pages
       . page one
       . page two
       . pages overlaid
    o 160x200, 2 colors, four pages overlaid
  - palette: 16 colors can be chosen among 4096

* devices:
  - AZERTY keyboard, 81-keys, French with accents, keypad & function keys
  - cartridge, up to 64 KB, TO7 compatible
  - two-button mouse connected to the keyboard (not working yet)
  - lightpen, with 1-pixel vertical and horizontal resolution
  - 1-bit internal buzzer
  - cassette 900 bauds, TO7 compatible
  - integrated parallel CENTRONICS (printer emulated)
  - SX 90-018: game extension (identical to the TO7)
  - RF 57-932: RS232 extension (identical to the TO7)
  - MD 90-120: MODEM extension (identical to the TO7)
  - IEEE extension ? (unemulated)
  - floppy:
    . integrated floppy controller, based on WD2793
    . integrated one-sided double-density 3''1/2
    . external two-sided double-density 3''1/2, 5''1/4 or QDD (extension)
    . floppies are TO7 and MO5 compatible
  - speech synthesis extension: identical to TO7
  - MIDIPAK MIDI extension: identical to TO7

**********************************************************************/

static ADDRESS_MAP_START ( to9, AS_PROGRAM, 8, thomson_state )

	AM_RANGE ( 0x0000, 0x3fff ) AM_READ_BANK ( THOM_CART_BANK ) AM_WRITE(to9_cartridge_w )/* 4 * 16 KB */
	AM_RANGE ( 0x4000, 0x5fff ) AM_READ_BANK ( THOM_VRAM_BANK ) AM_WRITE(to770_vram_w )
	AM_RANGE ( 0x6000, 0x9fff ) AM_RAMBANK   ( THOM_BASE_BANK ) /* 16 KB */
	AM_RANGE ( 0xa000, 0xdfff ) AM_RAMBANK   ( THOM_RAM_BANK )  /* 10 * 16 KB */
	AM_RANGE ( 0xe000, 0xe7bf ) AM_ROMBANK   ( THOM_FLOP_BANK )
	AM_RANGE ( 0xe7c0, 0xe7c7 ) AM_DEVREADWRITE("mc6846", mc6846_device, read, write)
	AM_RANGE ( 0xe7c8, 0xe7cb ) AM_DEVREADWRITE( "pia_0", pia6821_device, read_alt, write_alt)
	AM_RANGE ( 0xe7cc, 0xe7cf ) AM_DEVREADWRITE( "pia_1", pia6821_device, read_alt, write_alt)
	AM_RANGE ( 0xe7d0, 0xe7d9 ) AM_READWRITE(to9_floppy_r, to9_floppy_w )
	AM_RANGE ( 0xe7da, 0xe7dd ) AM_READWRITE(to9_vreg_r, to9_vreg_w )
	AM_RANGE ( 0xe7de, 0xe7df ) AM_READWRITE(to9_kbd_r, to9_kbd_w )
	AM_RANGE ( 0xe7e4, 0xe7e7 ) AM_READWRITE(to9_gatearray_r, to9_gatearray_w )
	AM_RANGE ( 0xe7e8, 0xe7eb ) AM_DEVREADWRITE( "acia",  mos6551_device, read, write )
/*   AM_RANGE ( 0xe7f0, 0xe7f7 ) AM_READWRITE(to9_ieee_r, to9_ieee_w ) */
	AM_RANGE ( 0xe7f2, 0xe7f3 ) AM_READWRITE(to7_midi_r, to7_midi_w )
	AM_RANGE ( 0xe7f8, 0xe7fb ) AM_DEVREADWRITE( "pia_3", pia6821_device, read_alt, write_alt)
	AM_RANGE ( 0xe7fe, 0xe7ff ) AM_READWRITE(to7_modem_mea8000_r, to7_modem_mea8000_w )
	AM_RANGE ( 0xe800, 0xffff ) AM_ROM       /* system bios  */

/* 0x10000 - 0x1ffff:  64 KB external ROM cartridge */
/* 0x20000 - 0x3ffff: 128 KB internal software ROM */
/* 0x40000 - 0x447ff: 18  KB external floppy / network ROM controllers */

/* RAM mapping:
   0x00000 - 0x03fff: 16 KB video RAM
   0x04000 - 0x07fff: 16 KB unbanked base RAM
   0x08000 - 0x2ffff: 10 * 16 KB banked extended RAM
 */

ADDRESS_MAP_END



/* ------------ ROMS ------------ */

/* NOT WORKING
   these bios seem heavily patched (probably to work with specific emulators
   that trap some bios calls)
 */

ROM_START ( to9 )
	ROM_REGION ( 0x44800, "maincpu", 0 )
	ROM_LOAD ( "to9.rom", 0xe000, 0x2000, /* BIOS & floppy controller */
		CRC(f9278bf7)
		SHA1(9e99e6ae0285950f007b19161de642a4031fe46e) )

		/* BASIC & software */
	ROM_LOAD ( "basic9-0.rom", 0x20000, 0x4000,
		CRC(c7bac620)
		SHA1(4b2a8b30cf437858ce978ba7b0dfa2bbd57eb38a) )
	ROM_LOAD ( "basic9-1.rom", 0x24000, 0x4000,
		CRC(ea5f3e43)
		SHA1(5e58a29c2d117fcdb1f5e7ca31dbfffa0f9218f2) )
	ROM_LOAD ( "basic9-2.rom", 0x28000, 0x4000,
		CRC(0f5581b3)
		SHA1(93815ca78d3532192aaa56cbf65b68b0f10f1b8a) )
	ROM_LOAD ( "basic9-3.rom", 0x2c000, 0x4000,
		CRC(6b5b19e3)
		SHA1(0e832670c185694d9abbcebcc3ad90e94eed585d) )
	ROM_LOAD ( "soft9-0a.rom", 0x30000, 0x4000,
		CRC(8cee157e)
		SHA1(f32fc39b95890c00571e9f3fbcc2d8e0596fc4a1) )
	ROM_LOAD ( "soft9-1a.rom", 0x34000, 0x4000,
		CRC(cf39ac93)
		SHA1(b97e6b7389398e5706624973c11ee7ddba323ce1) )
	ROM_LOAD ( "soft9-0b.rom", 0x38000, 0x4000,
		CRC(033aee3f)
		SHA1(f3604e500329ec0489b05dbab05530322e9463c5) )
	ROM_LOAD ( "soft9-1b.rom", 0x3c000, 0x4000,
		CRC(214fe527)
		SHA1(0d8e3f1ca347026e906c3d00a0371e8238c44a60) )

	ROM_FLOPPY7( 0x40000 )

	ROM_FILL( 0x10000, 0x10000, 0x39 )
ROM_END


/* ------------ inputs   ------------ */

static INPUT_PORTS_START ( to9_keyboard )
	PORT_START ( "keyboard.0" )
		KEY ( 0, "F2 F7", F2 )           PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR(UCHAR_MAMEKEY(F7))
		KEY ( 1, "_ 6", 6 )              PORT_CHAR('_') PORT_CHAR('6')
		KEY ( 2, "Y", Y )                PORT_CHAR('Y')
		KEY ( 3, "H \302\250", H )       PORT_CHAR('H')
		KEY ( 4, UTF8_UP, UP )    PORT_CHAR(UCHAR_MAMEKEY(UP))
		KEY ( 5, UTF8_RIGHT, RIGHT ) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		KEY ( 6, "Home Clear", HOME )    PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CHAR(UCHAR_MAMEKEY(ESC))
		KEY ( 7, "N", N )                PORT_CHAR('N')
	PORT_START ( "keyboard.1" )
		KEY ( 0, "F3 F8", F3 )           PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F8))
		KEY ( 1, "( 5", 5 )              PORT_CHAR('(') PORT_CHAR('5')
		KEY ( 2, "T", T )                PORT_CHAR('T')
		KEY ( 3, "G", G )                PORT_CHAR('G')
		KEY ( 4, "= +", EQUALS )         PORT_CHAR('=') PORT_CHAR('+')
		KEY ( 5, UTF8_LEFT, LEFT )  PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		KEY ( 6, "Insert", INSERT )      PORT_CHAR(UCHAR_MAMEKEY(INSERT))
		KEY ( 7, "B \302\264", B )       PORT_CHAR('B')
	PORT_START ( "keyboard.2" )
		KEY ( 0, "F4 F9", F4 )           PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHAR(UCHAR_MAMEKEY(F9))
		KEY ( 1, "' 4", 4 )              PORT_CHAR('\'') PORT_CHAR('4')
		KEY ( 2, "R", R )                PORT_CHAR('R')
		KEY ( 3, "F", F )                PORT_CHAR('F')
		KEY ( 4, "Accent", END )         PORT_CHAR(UCHAR_MAMEKEY(END))
		KEY ( 5, "Keypad 1", 1_PAD )     PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
		KEY ( 6, "Delete Backspace", DEL ) PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE))
		KEY ( 7, "V", V )                PORT_CHAR('V')
	PORT_START ( "keyboard.3" )
		KEY ( 0, "F5 F10", F5 )          PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHAR(UCHAR_MAMEKEY(F10))
		KEY ( 1, "\" 3", 3 )             PORT_CHAR('"') PORT_CHAR('3')
		KEY ( 2, "E", E )                PORT_CHAR('E')
		KEY ( 3, "D", D )                PORT_CHAR('D')
		KEY ( 4, "Keypad 7", 7_PAD )     PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
		KEY ( 5, "Keypad 4", 4_PAD )     PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
		KEY ( 6, "Keypad 0", 0_PAD )     PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
		KEY ( 7, "C \136", C )           PORT_CHAR('C')
	PORT_START ( "keyboard.4" )
		KEY ( 0, "F1 F6", F1 )           PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F6))
		KEY ( 1, "\303\251 2", 2 )       PORT_CHAR( 0xe9 ) PORT_CHAR('2')
		KEY ( 2, "Z", Z )                PORT_CHAR('Z')
		KEY ( 3, "S", S )                PORT_CHAR('S')
		KEY ( 4, "Keypad 8", 8_PAD )     PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
		KEY ( 5, "Keypad 2", 2_PAD )     PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
		KEY ( 6, "Keypad .", DEL_PAD )   PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
		KEY ( 7, "X", X )                PORT_CHAR('X')
	PORT_START ( "keyboard.5" )
		KEY ( 0, "# @", TILDE )          PORT_CHAR('#') PORT_CHAR('@')
		KEY ( 1, "* 1", 1 )              PORT_CHAR('*') PORT_CHAR('1')
		KEY ( 2, "A \140", A )           PORT_CHAR('A')
		KEY ( 3, "Q", Q )                PORT_CHAR('Q')
		KEY ( 4, "[ {", QUOTE )          PORT_CHAR('[') PORT_CHAR('{')
		KEY ( 5, "Keypad 5", 5_PAD )     PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
		KEY ( 6, "Keypad 6", 6_PAD )     PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
		KEY ( 7, "W", W )                PORT_CHAR('W')
	PORT_START ( "keyboard.6" )
		KEY ( 0, "Stop", TAB )           PORT_CHAR(27)
		KEY ( 1, "\303\250 7", 7 )       PORT_CHAR( 0xe8 ) PORT_CHAR('7')
		KEY ( 2, "U", U )                PORT_CHAR('U')
		KEY ( 3, "J", J )                PORT_CHAR('J')
		KEY ( 4, "Space", SPACE )        PORT_CHAR(' ')
		KEY ( 5, "Keypad 9", 9_PAD )     PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
		KEY ( 6, "Keypad Enter", ENTER_PAD ) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
		KEY ( 7, ", ?", COMMA )          PORT_CHAR(',') PORT_CHAR('?')
	PORT_START ( "keyboard.7" )
		KEY ( 0, "Control", LCONTROL )   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
		KEY ( 1, "! 8", 8 )              PORT_CHAR('!') PORT_CHAR('8')
		KEY ( 2, "I", I )                PORT_CHAR('I')
		KEY ( 3, "K", K )                PORT_CHAR('K')
		KEY ( 4, "$ &", CLOSEBRACE )     PORT_CHAR('$') PORT_CHAR('&')
		KEY ( 5, UTF8_DOWN, DOWN )  PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		KEY ( 6, "] }", BACKSLASH )      PORT_CHAR(']') PORT_CHAR('}')
		KEY ( 7, "; .", STOP )           PORT_CHAR(';') PORT_CHAR('.')
	PORT_START ( "keyboard.8" )
		KEY ( 0, "Caps-Lock", CAPSLOCK ) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
		KEY ( 1, "\303\247 9", 9 )       PORT_CHAR( 0xe7 ) PORT_CHAR('9')
		KEY ( 2, "O", O )                PORT_CHAR('O')
		KEY ( 3, "L", L )                PORT_CHAR('L')
		KEY ( 4, "- \\", BACKSPACE )     PORT_CHAR('-') PORT_CHAR('\\')
		KEY ( 5, "\303\271 %", COLON )   PORT_CHAR( 0xf9 ) PORT_CHAR('%')
		KEY ( 6, "Enter", ENTER )        PORT_CHAR(13)
		KEY ( 7, ": /", SLASH )          PORT_CHAR(':') PORT_CHAR('/')
	PORT_START ( "keyboard.9" )
		KEY ( 0, "Shift", LSHIFT )  PORT_CODE ( KEYCODE_RSHIFT ) PORT_CHAR(UCHAR_SHIFT_1)
		KEY ( 1, "\303\240 0", 0 )       PORT_CHAR( 0xe0 ) PORT_CHAR('0')
		KEY ( 2, "P", P )                PORT_CHAR('P')
		KEY ( 3, "M", M )                PORT_CHAR('M')
		KEY ( 4, ") \302\260", MINUS )   PORT_CHAR(')') PORT_CHAR( 0xb0 )
		KEY ( 5, "\342\206\221 \302\250", OPENBRACE )  PORT_CHAR('^') PORT_CHAR( 0xa8 )
		KEY ( 6, "Keypad 3", 3_PAD )     PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
		KEY ( 7, "> <", BACKSLASH2 )     PORT_CHAR('>') PORT_CHAR('<')
INPUT_PORTS_END

static INPUT_PORTS_START ( to9_fconfig )
	PORT_START ( "fconfig" )

	PORT_CONFNAME ( 0x07, 0x00, "External floppy (reset)" )
	PORT_CONFSETTING ( 0x00, "No external" )
	PORT_CONFSETTING ( 0x01, "CD 90-015 (5\"1/4 SD)" )
	PORT_CONFSETTING ( 0x02, "CD 90-640 (5\"1/4 DD)" )
	PORT_CONFSETTING ( 0x03, "CD 90-351 (3\"1/2)" )
	PORT_CONFSETTING ( 0x04, "CQ 90-028 (2\"8 QDD)" )
	PORT_CONFSETTING ( 0x05, "Network" )

	PORT_CONFNAME ( 0xf8, 0x08, "Network ID" )
	PORT_CONFSETTING ( 0x00, "0 (Master)" )
	PORT_CONFSETTING ( 0x08, "1" )
	PORT_CONFSETTING ( 0x10, "2" )
	PORT_CONFSETTING ( 0x18, "3" )
	PORT_CONFSETTING ( 0x20, "4" )
	PORT_CONFSETTING ( 0x28, "5" )
	PORT_CONFSETTING ( 0x30, "6" )
	PORT_CONFSETTING ( 0x38, "7" )
	PORT_CONFSETTING ( 0x40, "8" )
	PORT_CONFSETTING ( 0x48, "9" )
	PORT_CONFSETTING ( 0x50, "10" )
	PORT_CONFSETTING ( 0x58, "11" )
	PORT_CONFSETTING ( 0x60, "12" )
	PORT_CONFSETTING ( 0x68, "13" )
	PORT_CONFSETTING ( 0x70, "14" )
	PORT_CONFSETTING ( 0x78, "15" )
	PORT_CONFSETTING ( 0x80, "16" )
	PORT_CONFSETTING ( 0x88, "17" )
	PORT_CONFSETTING ( 0x90, "18" )
	PORT_CONFSETTING ( 0x98, "19" )
	PORT_CONFSETTING ( 0xa0, "20" )
	PORT_CONFSETTING ( 0xa8, "21" )
	PORT_CONFSETTING ( 0xb0, "22" )
	PORT_CONFSETTING ( 0xb8, "23" )
	PORT_CONFSETTING ( 0xc0, "24" )
	PORT_CONFSETTING ( 0xc8, "25" )
	PORT_CONFSETTING ( 0xd0, "26" )
	PORT_CONFSETTING ( 0xd8, "27" )
	PORT_CONFSETTING ( 0xe0, "28" )
	PORT_CONFSETTING ( 0xe8, "29" )
	PORT_CONFSETTING ( 0xf0, "30" )
	PORT_CONFSETTING ( 0xf8, "31" )

INPUT_PORTS_END

static INPUT_PORTS_START ( to9 )
	PORT_INCLUDE ( thom_lightpen )
	PORT_INCLUDE ( thom_game_port )
	PORT_INCLUDE ( to9_keyboard )
	PORT_INCLUDE ( to7_config )
	PORT_INCLUDE ( to9_fconfig )
	PORT_INCLUDE ( to7_vconfig )
	PORT_INCLUDE ( to7_mconfig )
INPUT_PORTS_END

/* ------------ driver ------------ */

static MACHINE_CONFIG_DERIVED( to9, to7 )
	MCFG_MACHINE_START_OVERRIDE( thomson_state, to9 )
	MCFG_MACHINE_RESET_OVERRIDE( thomson_state, to9 )

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP ( to9)

	MCFG_DEVICE_MODIFY(THOM_PIA_SYS)
	MCFG_PIA_READPA_HANDLER(READ8(thomson_state, to9_sys_porta_in))
	MCFG_PIA_READPB_HANDLER(NULL)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(thomson_state, to9_sys_porta_out))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(thomson_state, to9_sys_portb_out))
	MCFG_PIA_CB2_HANDLER(NULL)
	MCFG_PIA_IRQA_HANDLER(NULL)

	MCFG_DEVICE_MODIFY("mc6846")
	MCFG_MC6846_OUT_PORT_CB(WRITE8(thomson_state, to9_timer_port_out))

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(thomson_state, write_centronics_busy))

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("192K")
	MCFG_RAM_EXTRA_OPTIONS("128K")
MACHINE_CONFIG_END


COMP ( 1985, to9, 0, 0, to9, to9, driver_device, 0, "Thomson", "TO9", MACHINE_IMPERFECT_COLORS )


/******************************** TO8 ********************************

TO8 (1986)
---

The TO8 was meant to replace the TO7/70 as a home-computer.
It includes and improves on the technology from the TO9 (improved video,
256 KB of RAM fully managed by the new BASIC 512, more integrated gate-array).
It has a more compact Amiga-like look, no separate keyboard, no integrated
floppy drive (although the controller is integrated), no software in ROM,
less extension slots. Also, the game & music extension is now integrated.
It is quite compatible with the TO7 and TO7/70, and with the TO9 to some
extent.
The TO8 was quite popular and became the de-facto gamming computer in the
Thomson family.

* chips:
  - 1 MHz Motorola 6809E CPU
  - 2 Motorola 6821 PIAs (system, game, +1 in modem extension)
  - 1 Motorola 6846 timer, PIA
  - 1 Motorola 6804 (keyboard)
  - 2 gate-arrays (system & video, floppy controller)

* memory:
  - 256 KB base RAM
    + 256 KB extended RAM (EM 88-256) = 512 KB total RAM emulated
  - 16 KB BIOS ROM
  - 64 KB software ROM (BASIC 1, BASIC 512, extended BIOS)
  - unified memory view via improved bank switching

* video:
  improved wrt TO9: a 9-th video mode, 4 video pages (shared with main RAM)
  border color has its 4-th index bit inverted

* devices:
  - same keyboard as T09: AZERTY 81-keys
    (but no 6850 controller, the 6804 is directly connected to the 6821 & 6846)
  - cartridge, up to 64 KB, TO7 compatible
  - two-button serial mouse (TO9-incompatible)
  - lightpen, with 1-pixel vertical and horizontal resolution
  - two 8-position 2-button game pads (SX 90-018 extension integrated)
  - 6-bit DAC sound (NOTE: 1-bit buzzer is gone)
  - cassette 900 bauds, TO7 compatible
  - integrated parallel CENTRONICS (printer emulated)
  - RF 57-932: RS232 extension (identical to the TO7)
  - MD 90-120: MODEM extension (identical to the TO7?)
  - IEEE extension ?
  - floppy:
   . integrated floppy controller, based on custom Thomson gate-array
   . no integrated drive
   . up to two external two-sided double-density 3"1/2, 5"1/4 or QDD drives
   . floppies are TO7 and MO5 compatible
  - speech synthesis extension: identical to TO7
  - MIDIPAK MIDI extension: identical to TO7

TO8D (1987)
----

The TO8D is simply a TO8 with an integrated 3"1/2 floppy drive.

**********************************************************************/


static ADDRESS_MAP_START ( to8, AS_PROGRAM, 8, thomson_state )

	AM_RANGE ( 0x0000, 0x3fff ) AM_READ_BANK ( THOM_CART_BANK) AM_WRITE(to8_cartridge_w ) /* 4 * 16 KB */
	AM_RANGE ( 0x4000, 0x5fff ) AM_READ_BANK ( THOM_VRAM_BANK) AM_WRITE(to770_vram_w )
	AM_RANGE ( 0x6000, 0x7fff ) AM_READ_BANK ( TO8_SYS_LO) AM_WRITE(to8_sys_lo_w )
	AM_RANGE ( 0x8000, 0x9fff ) AM_READ_BANK ( TO8_SYS_HI) AM_WRITE(to8_sys_hi_w )
	AM_RANGE ( 0xa000, 0xbfff ) AM_READ_BANK ( TO8_DATA_LO) AM_WRITE(to8_data_lo_w )
	AM_RANGE ( 0xc000, 0xdfff ) AM_READ_BANK ( TO8_DATA_HI) AM_WRITE(to8_data_hi_w )
	AM_RANGE ( 0xe000, 0xe7bf ) AM_ROMBANK   ( THOM_FLOP_BANK ) /* 2 * 2 KB */
	AM_RANGE ( 0xe7c0, 0xe7c7 ) AM_DEVREADWRITE("mc6846", mc6846_device, read, write)
	AM_RANGE ( 0xe7c8, 0xe7cb ) AM_DEVREADWRITE( "pia_0", pia6821_device, read_alt, write_alt)
	AM_RANGE ( 0xe7cc, 0xe7cf ) AM_DEVREADWRITE( "pia_1", pia6821_device, read_alt, write_alt)
	AM_RANGE ( 0xe7d0, 0xe7d9 ) AM_READWRITE(to8_floppy_r, to8_floppy_w )
	AM_RANGE ( 0xe7da, 0xe7dd ) AM_READWRITE(to8_vreg_r, to8_vreg_w )
	AM_RANGE ( 0xe7e4, 0xe7e7 ) AM_READWRITE(to8_gatearray_r, to8_gatearray_w )
	AM_RANGE ( 0xe7e8, 0xe7eb ) AM_DEVREADWRITE( "acia",  mos6551_device, read, write )
/*   AM_RANGE ( 0xe7f0, 0xe7f7 ) AM_READWRITE(to9_ieee_r, to9_ieee_w ) */
	AM_RANGE ( 0xe7f2, 0xe7f3 ) AM_READWRITE(to7_midi_r, to7_midi_w )
	AM_RANGE ( 0xe7f8, 0xe7fb ) AM_DEVREADWRITE( "pia_3", pia6821_device, read_alt, write_alt)
	AM_RANGE ( 0xe7fe, 0xe7ff ) AM_READWRITE(to7_modem_mea8000_r, to7_modem_mea8000_w )
	AM_RANGE ( 0xe800, 0xffff ) AM_ROMBANK   ( TO8_BIOS_BANK ) /* 2 * 6 KB */

/* 0x10000 - 0x1ffff: 64 KB external ROM cartridge */
/* 0x20000 - 0x2ffff: 64 KB internal software ROM */
/* 0x30000 - 0x33fff: 16 KB BIOS ROM */
/* 0x34000 - 0x387ff: 18 KB external floppy / network ROM controllers */

/* RAM mapping: 512 KB flat (including video) */

ADDRESS_MAP_END


/* ------------ ROMS ------------ */

ROM_START ( to8 )
	ROM_REGION ( 0x38800, "maincpu", 0 )

		/* BIOS & floppy */
	ROM_LOAD ( "to8-0.rom", 0x30000, 0x2000,
		CRC(3c4a640a)
		SHA1(0a4952f0ca002d82ac83755e1f694d56399413b2) )
	ROM_LOAD ( "to8-1.rom", 0x32000, 0x2000,
		CRC(cb9bae2d)
		SHA1(a4a55a6e2c74bca15951158c5164970e922fc1c1) )

		/* BASIC */
	ROM_LOAD ( "basic8-0.rom", 0x20000, 0x4000,
		CRC(e5a00fb3)
		SHA1(281e535ed9b0f76e620253e9103292b8ff623d02) )
	ROM_LOAD ( "basic8-1.rom", 0x24000, 0x4000,
		CRC(4b241e63)
		SHA1(ca8941a10db6cc069bf84c773f5e7d7d2c18449e) )
	ROM_LOAD ( "basic8-2.rom", 0x28000, 0x4000,
		CRC(0f5581b3)
		SHA1(93815ca78d3532192aaa56cbf65b68b0f10f1b8a) )
	ROM_LOAD ( "basic8-3.rom", 0x2c000, 0x4000,
		CRC(f552e7e3)
		SHA1(3208e0d7d90241a327ed24e4921303f16e167bd5) )

	ROM_FLOPPY7( 0x34000 )

	ROM_FILL( 0x10000, 0x10000, 0x39 )
ROM_END

ROM_START ( to8d )
	ROM_REGION ( 0x38800, "maincpu", 0 )

		/* BIOS & floppy */
	ROM_LOAD ( "to8d-0.rom", 0x30000, 0x2000,
		CRC(30ea4950)
		SHA1(6705100cd337fffb26ce999302b55fb71557b128) )
	ROM_LOAD ( "to8d-1.rom", 0x32000, 0x2000,
		CRC(926cf0ca)
		SHA1(8521613ac00e04dd94b69e771aeaefbf4fe97bf7) )

		/* BASIC */
	ROM_LOAD ( "basic8-0.rom", 0x20000, 0x4000,
		CRC(e5a00fb3)
		SHA1(281e535ed9b0f76e620253e9103292b8ff623d02) )
	ROM_LOAD ( "basic8-1.rom", 0x24000, 0x4000,
		CRC(4b241e63)
		SHA1(ca8941a10db6cc069bf84c773f5e7d7d2c18449e) )
	ROM_LOAD ( "basic8-2.rom", 0x28000, 0x4000,
		CRC(0f5581b3)
		SHA1(93815ca78d3532192aaa56cbf65b68b0f10f1b8a) )
	ROM_LOAD ( "basic8-3.rom", 0x2c000, 0x4000,
		CRC(f552e7e3)
		SHA1(3208e0d7d90241a327ed24e4921303f16e167bd5) )

	ROM_FLOPPY7( 0x34000 )

	ROM_FILL( 0x10000, 0x10000, 0x39 )
ROM_END


/* ------------ inputs   ------------ */

static INPUT_PORTS_START ( to8_config )
	PORT_START ( "config" )

	PORT_CONFNAME ( 0x01, 0x00, "Game Port" )
	PORT_CONFSETTING ( 0x00, DEF_STR( Joystick ) )
	PORT_CONFSETTING ( 0x01, "Mouse" )

	PORT_CONFNAME ( 0x02, 0x00, "Keyboard" )
	PORT_CONFSETTING ( 0x00, "Enabled" )
	PORT_CONFSETTING ( 0x02, "Disabled" )

INPUT_PORTS_END


static INPUT_PORTS_START ( to8 )
	PORT_INCLUDE ( thom_lightpen )
	PORT_INCLUDE ( thom_game_port )
	PORT_INCLUDE ( to9_keyboard )
	PORT_INCLUDE ( to8_config )
	PORT_INCLUDE ( to9_fconfig )
	PORT_INCLUDE ( to7_vconfig )
	PORT_INCLUDE ( to7_mconfig )
INPUT_PORTS_END


static INPUT_PORTS_START ( to8d )
	PORT_INCLUDE ( to8 )
INPUT_PORTS_END

/* ------------ driver ------------ */

static MACHINE_CONFIG_DERIVED( to8, to7 )
	MCFG_MACHINE_START_OVERRIDE( thomson_state, to8 )
	MCFG_MACHINE_RESET_OVERRIDE( thomson_state, to8 )

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP ( to8)

	MCFG_DEVICE_MODIFY(THOM_PIA_SYS)
	MCFG_PIA_READPA_HANDLER(READ8(thomson_state, to8_sys_porta_in))
	MCFG_PIA_READPB_HANDLER(NULL)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(thomson_state, to9_sys_porta_out))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(thomson_state, to8_sys_portb_out))
	MCFG_PIA_CB2_HANDLER(NULL)
	MCFG_PIA_IRQA_HANDLER(NULL)

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(thomson_state, write_centronics_busy))

	MCFG_DEVICE_MODIFY("mc6846")
	MCFG_MC6846_OUT_PORT_CB(WRITE8(thomson_state, to8_timer_port_out))
	MCFG_MC6846_OUT_CP2_CB(WRITE8(thomson_state, to8_timer_cp2_out))
	MCFG_MC6846_IN_PORT_CB(READ8(thomson_state, to8_timer_port_in))

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")
	MCFG_RAM_EXTRA_OPTIONS("256K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( to8d, to8 )
MACHINE_CONFIG_END


COMP ( 1986, to8, 0, 0, to8, to8, driver_device, 0, "Thomson", "TO8", 0 )

COMP ( 1987, to8d, to8, 0, to8d, to8d, driver_device, 0, "Thomson", "TO8D", 0 )


/******************************** TO9+ *******************************

TO9+ (1986)
----

The TO9+ is the direct successor of the T09 as Thomson's high-end
product: desktop look, 512 KB of RAM, integrated floppy drive and
modem. Some software integrated in ROM on the TO9 are now supplied on
floppies.
Internally, the TO9+ is based more on TO8 technology than T09
(same gate-arrays).
It has enhanced communication capabilities by integrating either the
MODEM or the RS232 extension.
It should be compatible with the TO9 and, to some extent, with the TO7, TO7/70
and TO8.
It uses the same video gate-array and floppy controller.

The differences with the TO8 are:

* chips:
  - 1 Motorola 6805 + 1 Motorola 6850 (keyboard)
  - 3 Motorola 6821 PIAs (system, game, modem)

* memory:
  - 512 KB RAM (not extendable)

* devices:
  - same keyboard as T08/TO9 (AZERTY 81-keys) but different controller
  - RF 57-932: RS232 (identical to the TO7) sometimes integrated
  - MD 90-120: MODEM (identical to the TO7?) sometimes integrated
  - IEEE extension ?
  - floppy: one two-sided double-density 3''1/2 floppy drive is integrated
  - RS 52-932 RS232 extension ?
  - digitisation extension

**********************************************************************/

static ADDRESS_MAP_START ( to9p, AS_PROGRAM, 8, thomson_state )

	AM_RANGE ( 0x0000, 0x3fff ) AM_READ_BANK ( THOM_CART_BANK) AM_WRITE(to8_cartridge_w ) /* 4 * 16 KB */
	AM_RANGE ( 0x4000, 0x5fff ) AM_READ_BANK ( THOM_VRAM_BANK) AM_WRITE(to770_vram_w )
	AM_RANGE ( 0x6000, 0x7fff ) AM_READ_BANK ( TO8_SYS_LO) AM_WRITE(to8_sys_lo_w )
	AM_RANGE ( 0x8000, 0x9fff ) AM_READ_BANK ( TO8_SYS_HI) AM_WRITE(to8_sys_hi_w )
	AM_RANGE ( 0xa000, 0xbfff ) AM_READ_BANK ( TO8_DATA_LO) AM_WRITE(to8_data_lo_w )
	AM_RANGE ( 0xc000, 0xdfff ) AM_READ_BANK ( TO8_DATA_HI) AM_WRITE(to8_data_hi_w )
	AM_RANGE ( 0xe000, 0xe7bf ) AM_ROMBANK   ( THOM_FLOP_BANK ) /* 2 * 2 KB */
	AM_RANGE ( 0xe7c0, 0xe7c7 ) AM_DEVREADWRITE("mc6846", mc6846_device, read, write)
	AM_RANGE ( 0xe7c8, 0xe7cb ) AM_DEVREADWRITE( "pia_0", pia6821_device, read_alt, write_alt)
	AM_RANGE ( 0xe7cc, 0xe7cf ) AM_DEVREADWRITE( "pia_1", pia6821_device, read_alt, write_alt)
	AM_RANGE ( 0xe7d0, 0xe7d9 ) AM_READWRITE(to8_floppy_r, to8_floppy_w )
	AM_RANGE ( 0xe7da, 0xe7dd ) AM_READWRITE(to8_vreg_r, to8_vreg_w )
	AM_RANGE ( 0xe7de, 0xe7df ) AM_READWRITE(to9_kbd_r, to9_kbd_w )
	AM_RANGE ( 0xe7e4, 0xe7e7 ) AM_READWRITE(to8_gatearray_r, to8_gatearray_w )
	AM_RANGE ( 0xe7e8, 0xe7eb ) AM_DEVREADWRITE( "acia",  mos6551_device, read, write )
/*   AM_RANGE ( 0xe7f0, 0xe7f7 ) AM_READWRITE(to9_ieee_r, to9_ieee_w ) */
	AM_RANGE ( 0xe7f2, 0xe7f3 ) AM_READWRITE(to7_midi_r, to7_midi_w )
	AM_RANGE ( 0xe7f8, 0xe7fb ) AM_DEVREADWRITE( "pia_3", pia6821_device, read_alt, write_alt)
	AM_RANGE ( 0xe7fe, 0xe7ff ) AM_READWRITE(to7_modem_mea8000_r, to7_modem_mea8000_w )
	AM_RANGE ( 0xe800, 0xffff ) AM_ROMBANK   ( TO8_BIOS_BANK ) /* 2 * 6 KB */

/* 0x10000 - 0x1ffff: 64 KB external ROM cartridge */
/* 0x20000 - 0x2ffff: 64 KB internal software ROM */
/* 0x30000 - 0x33fff: 16 KB BIOS ROM */
/* 0x34000 - 0x387ff: 18 KB external floppy / network ROM controllers */

/* RAM mapping: 512 KB flat (including video) */

ADDRESS_MAP_END


/* ------------ ROMS ------------ */

ROM_START ( to9p )
	ROM_REGION ( 0x38800, "maincpu", 0 )

		/* BIOS & floppy */
	ROM_LOAD ( "to9p-0.rom", 0x30000, 0x2000,
		CRC(a2731296)
		SHA1(b30e06127d6e99d4ac5a5bb67881df27bbd9a7e5) )
	ROM_LOAD ( "to9p-1.rom", 0x32000, 0x2000,
		CRC(c52ce315)
		SHA1(7eacbd796e76bc72b872f9700c9b90414899ea0f) )

		/* BASIC */
	ROM_LOAD ( "basicp-0.rom", 0x20000, 0x4000,
		CRC(e5a00fb3)
		SHA1(281e535ed9b0f76e620253e9103292b8ff623d02) )
	ROM_LOAD ( "basicp-1.rom", 0x24000, 0x4000,
		CRC(4b241e63)
		SHA1(ca8941a10db6cc069bf84c773f5e7d7d2c18449e) )
	ROM_LOAD ( "basicp-2.rom", 0x28000, 0x4000,
		CRC(0f5581b3)
		SHA1(93815ca78d3532192aaa56cbf65b68b0f10f1b8a) )
	ROM_LOAD ( "basicp-3.rom", 0x2c000, 0x4000,
		CRC(ebe9c8d9)
		SHA1(b667ad09a1181f65059a2cbb4c95421bc544a334) )

	ROM_FLOPPY7( 0x34000 )

	ROM_FILL( 0x10000, 0x10000, 0x39 )
ROM_END


/* ------------ inputs   ------------ */

static INPUT_PORTS_START ( to9p )
	PORT_INCLUDE ( thom_lightpen )
	PORT_INCLUDE ( thom_game_port )
	PORT_INCLUDE ( to9_keyboard )
	PORT_INCLUDE ( to7_config )
	PORT_INCLUDE ( to9_fconfig )
	PORT_INCLUDE ( to7_vconfig )
	PORT_INCLUDE ( to7_mconfig )
INPUT_PORTS_END

/* ------------ driver ------------ */

static MACHINE_CONFIG_DERIVED( to9p, to7 )
	MCFG_MACHINE_START_OVERRIDE( thomson_state, to9p )
	MCFG_MACHINE_RESET_OVERRIDE( thomson_state, to9p )

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP ( to9p)

	MCFG_DEVICE_MODIFY(THOM_PIA_SYS)
	MCFG_PIA_READPA_HANDLER(READ8(thomson_state, to9_sys_porta_in))
	MCFG_PIA_READPB_HANDLER(NULL)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(thomson_state, to9_sys_porta_out))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(thomson_state, to8_sys_portb_out))
	MCFG_PIA_CB2_HANDLER(NULL)
	MCFG_PIA_IRQA_HANDLER(NULL)
	MCFG_PIA_IRQB_HANDLER(WRITELINE(thomson_state, thom_firq_1))

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(thomson_state, write_centronics_busy))

	MCFG_DEVICE_MODIFY("mc6846")
	MCFG_MC6846_OUT_PORT_CB(WRITE8(thomson_state, to9p_timer_port_out))
	MCFG_MC6846_OUT_CP2_CB(WRITE8(thomson_state, to8_timer_cp2_out))
	MCFG_MC6846_IN_PORT_CB(READ8(thomson_state, to9p_timer_port_in))

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")
MACHINE_CONFIG_END

COMP ( 1986, to9p, 0, 0, to9p, to9p, driver_device, 0, "Thomson", "TO9+", 0 )



/******************************** MO6 ********************************

MO6 (1986)
---

The MO6 is the (long awaited) successor to the MO5.
It is based on TO8 technology (same system & video gate-array).
However, it is lower-end and cheaper: less memory (128 KB RAM, not
extensible), no floppy controller but an integrated cassette recorder.
The MO6 is MO5 compatible, but not compatible with the TO family.

* chips:
  - 1 MHz Motorola 6809E CPU
  - 2 Motorola 6821 PIAs (system, game)
  - 1 gate-array (system & video, identical to the TO8)

* memory:
  - 128 KB RAM (not extendable)
  - 8 KB BIOS ROM
  - 24 KB BASIC 1 ROM
  - 32 KB BASIC 128 & extended BIOS ROM

* video:
  all modes from the TO8, but the TO7/70-compatible mode is replaced with
  an MO5-compatible one

* devices:
  - AZERTY keyboard, 69 keys, no keyboard controller (scanning is done
    periodically by the 6809)
  - MO5-compatible cartridge
  - two-button mouse (TO8-like)
  - optional lightpen
  - integrated game port (similar to SX 90-018)
    . 6-bit DAC sound
    . two 8-position 2-button game pads
    . two-button mouse
  - integrated cassette reader 1200 bauds (MO5 compatible) and 2400 bauds
  - parallel CENTRONICS (printer emulated)
  - RF 57-932: RS232 extension (identical to the TO7), or RF 90-932 (???)
  - IEEE extension ?
  - no integrated controller, but external TO7 floppy controllers & drives
    are possible
  - speech synthesis extension: identical to TO7 ?
  - MIDIPAK MIDI extension: identical to TO7 ?


Olivetti Prodest PC 128 (1986)
-----------------------

Italian version of the MO6, built by Thomson and sold by Olivetti.
Except from the ROM, it is very similar to the MO6.
Do not confuse with the Olivetti Prodest PC 128 Systema (or 128s) which is
based on the Acorn BBC Master Compact. Or with the Olivetti PC 1, which is
a PC XT.


**********************************************************************/

static ADDRESS_MAP_START ( mo6, AS_PROGRAM, 8, thomson_state )

	AM_RANGE ( 0x0000, 0x1fff ) AM_READ_BANK ( THOM_VRAM_BANK) AM_WRITE(to770_vram_w )
	AM_RANGE ( 0x2000, 0x3fff ) AM_READ_BANK ( TO8_SYS_LO) AM_WRITE(to8_sys_lo_w )
	AM_RANGE ( 0x4000, 0x5fff ) AM_READ_BANK ( TO8_SYS_HI) AM_WRITE(to8_sys_hi_w )
	AM_RANGE ( 0x6000, 0x7fff ) AM_READ_BANK ( TO8_DATA_LO) AM_WRITE(to8_data_lo_w )
	AM_RANGE ( 0x8000, 0x9fff ) AM_READ_BANK ( TO8_DATA_HI) AM_WRITE(to8_data_hi_w )
	AM_RANGE ( 0xa000, 0xa7bf ) AM_ROMBANK   ( THOM_FLOP_BANK )
	AM_RANGE ( 0xa7c0, 0xa7c3 ) AM_DEVREADWRITE( "pia_0", pia6821_device, read_alt, write_alt)
	AM_RANGE ( 0xa7cb, 0xa7cb ) AM_WRITE(mo6_ext_w )
	AM_RANGE ( 0xa7cc, 0xa7cf ) AM_DEVREADWRITE( "pia_1", pia6821_device, read_alt, write_alt)
	AM_RANGE ( 0xa7d0, 0xa7d9 ) AM_READWRITE(to7_floppy_r, to7_floppy_w )
	AM_RANGE ( 0xa7da, 0xa7dd ) AM_READWRITE(mo6_vreg_r, mo6_vreg_w )
	AM_RANGE ( 0xa7e4, 0xa7e7 ) AM_READWRITE(mo6_gatearray_r, mo6_gatearray_w )
	AM_RANGE ( 0xa7e8, 0xa7eb ) AM_DEVREADWRITE( "acia",  mos6551_device, read, write )
/*   AM_RANGE ( 0xa7f0, 0xa7f7 ) AM_READWRITE(to9_ieee_r, to9_ieee_w )*/
	AM_RANGE ( 0xa7f2, 0xa7f3 ) AM_READWRITE(to7_midi_r, to7_midi_w )
	AM_RANGE ( 0xa7fe, 0xa7ff ) AM_DEVREADWRITE("mea8000", mea8000_device, read, write)
	AM_RANGE ( 0xb000, 0xbfff ) AM_ROMBANK   ( MO6_CART_LO )
					AM_WRITE     ( mo6_cartridge_w )
	AM_RANGE ( 0xc000, 0xefff ) AM_ROMBANK   ( MO6_CART_HI )
					AM_WRITE     ( mo6_cartridge_w )
	AM_RANGE ( 0xf000, 0xffff ) AM_ROMBANK   ( TO8_BIOS_BANK )

/* 0x10000 - 0x1ffff: 64 KB external ROM cartridge */
/* 0x20000 - 0x2ffff: 64 KB BIOS ROM */
/* 0x30000 - 0x347ff: 16 KB floppy / network ROM controllers */

/* RAM mapping: 128 KB flat (including video) */

ADDRESS_MAP_END


/* ------------ ROMS ------------ */

ROM_START ( mo6 )
	ROM_REGION ( 0x34800, "maincpu", 0 )

		/* BIOS */
	ROM_LOAD ( "mo6-0.rom", 0x23000, 0x1000,
		CRC(0446eef6)
		SHA1(b57fcda69c95f0c97c5cb0605d17c49a0c630300) )
	ROM_LOAD ( "mo6-1.rom", 0x27000, 0x1000,
		CRC(eb6df8d4)
		SHA1(24e2232f582ce04f260acd8e9ec710468a81505c) )

		/* BASIC */
	ROM_LOAD ( "basic6-0.rom", 0x20000, 0x3000,
		CRC(18789833)
		SHA1(fccbf69cbc6deba45a767a26cd6454cf0eedfc2b) )
	ROM_LOAD ( "basic6-1.rom", 0x24000, 0x3000,
		CRC(c9b4d6f4)
		SHA1(47487d2bc4c9a9c09c733bd89c49693c52e262de) )
	ROM_LOAD ( "basic6-2.rom", 0x28000, 0x4000,
		CRC(08eac9bb)
		SHA1(c0231fdb3bcccbbb10c1f93cc529fc3b96dd3f4d) )
	ROM_LOAD ( "basic6-3.rom", 0x2c000, 0x4000,
		CRC(19d66dc4)
		SHA1(301b6366269181b74cb5d7ccdf5455b7290ae99b) )

	ROM_FLOPPY5 ( 0x30000 )
	ROM_FILL ( 0x10000, 0x10000, 0x39 )
ROM_END

ROM_START ( pro128 )
	ROM_REGION ( 0x34800, "maincpu", 0 )

		/* BIOS */
	ROM_LOAD ( "pro128-0.rom", 0x23000, 0x1000,
		CRC(a8aef291)
		SHA1(2685cca841f405a37ef48b0115f90c865ce79d0f) )
	ROM_LOAD ( "pro128-1.rom", 0x27000, 0x1000,
		CRC(5b3340ec)
		SHA1(269f2eb3e3452014b8d1f0f9e1c63fe56375a863) )

		/* BASIC */
	ROM_LOAD ( "basico-0.rom", 0x20000, 0x3000,
		CRC(98b10d5e)
		SHA1(d6b77e694fa85e1114293448e5a64f6e2cf46c22) )
	ROM_LOAD ( "basico-1.rom", 0x24000, 0x3000,
		CRC(721d2124)
		SHA1(51db1cd03b3891e212a24aa6563b09968930d897) )
	ROM_LOAD ( "basico-2.rom", 0x28000, 0x4000,
		CRC(135438ab)
		SHA1(617d4e4979842bea2c21ef7f8c50f3b08b15239a) )
	ROM_LOAD ( "basico-3.rom", 0x2c000, 0x4000,
		CRC(2c2befa6)
		SHA1(3e94e182bacbb55bb07be2af4c76c0b0df47b3bf) )

	ROM_FLOPPY5 ( 0x30000 )
	ROM_FILL ( 0x10000, 0x10000, 0x39 )
ROM_END


/* ------------ inputs   ------------ */

static INPUT_PORTS_START ( mo6_keyboard )

	PORT_START ( "keyboard.0" )
		KEY ( 0, "N", N )                   PORT_CHAR('N')
		KEY ( 1, ", ?", COMMA )             PORT_CHAR(',') PORT_CHAR('?')
		KEY ( 2, "; .", STOP )              PORT_CHAR(';') PORT_CHAR('.')
		KEY ( 3, "# @", TILDE )             PORT_CHAR('#') PORT_CHAR('@')
		KEY ( 4, "Space", SPACE )           PORT_CHAR(' ')
		KEY ( 5, "X", X )                   PORT_CHAR('X')
		KEY ( 6, "W", W )                   PORT_CHAR('W')
		KEY ( 7, "Shift", LSHIFT ) PORT_CODE ( KEYCODE_RSHIFT ) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_START ( "keyboard.1" )
		KEY ( 0, "Delete Backspace", DEL )  PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE))
		KEY ( 1, "Insert", INSERT )         PORT_CHAR(UCHAR_MAMEKEY(INSERT))
		KEY ( 2, "> <", BACKSLASH2 )        PORT_CHAR('>') PORT_CHAR('<')
		KEY ( 3, UTF8_RIGHT, RIGHT )    PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		KEY ( 4, UTF8_DOWN, DOWN )     PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		KEY ( 5, UTF8_LEFT, LEFT )     PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		KEY ( 6, UTF8_UP, UP )       PORT_CHAR(UCHAR_MAMEKEY(UP))
		KEY ( 7, "BASIC", RCONTROL )        PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_START ( "keyboard.2" )
		KEY ( 0, "J", J )                   PORT_CHAR('J')
		KEY ( 1, "K", K )                   PORT_CHAR('K')
		KEY ( 2, "L", L )                   PORT_CHAR('L')
		KEY ( 3, "M", M )                   PORT_CHAR('M')
		KEY ( 4, "B \302\264", B )          PORT_CHAR('B')
		KEY ( 5, "V", V )                   PORT_CHAR('V')
		KEY ( 6, "C \136", C )              PORT_CHAR('C')
		KEY ( 7, "Caps-Lock", CAPSLOCK )    PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_START ( "keyboard.3" )
		KEY ( 0, "H \302\250", H )          PORT_CHAR('H')
		KEY ( 1, "G", G )                   PORT_CHAR('G')
		KEY ( 2, "F", F )                   PORT_CHAR('F')
		KEY ( 3, "D", D )                   PORT_CHAR('D')
		KEY ( 4, "S", S )                   PORT_CHAR('S')
		KEY ( 5, "Q", Q )                   PORT_CHAR('Q')
		KEY ( 6, "Home Clear", HOME )       PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CHAR(UCHAR_MAMEKEY(ESC))
		KEY ( 7, "F1 F6", F1 )              PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_START ( "keyboard.4" )
		KEY ( 0, "U", U )                   PORT_CHAR('U')
		KEY ( 1, "I", I )                   PORT_CHAR('I')
		KEY ( 2, "O", O )                   PORT_CHAR('O')
		KEY ( 3, "P", P )                   PORT_CHAR('P')
		KEY ( 4, ": /", SLASH )             PORT_CHAR(':') PORT_CHAR('/')
		KEY ( 5, "$ &", CLOSEBRACE )        PORT_CHAR('$') PORT_CHAR('&')
		KEY ( 6, "Enter", ENTER )           PORT_CHAR(13)
		KEY ( 7, "F2 F7", F2 )              PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_START ( "keyboard.5" )
		KEY ( 0, "Y", Y )                   PORT_CHAR('Y')
		KEY ( 1, "T", T )                   PORT_CHAR('T')
		KEY ( 2, "R", R )                   PORT_CHAR('R')
		KEY ( 3, "E", E )                   PORT_CHAR('E')
		KEY ( 4, "Z", Z )                   PORT_CHAR('Z')
		KEY ( 5, "A \140", A )              PORT_CHAR('A')
		KEY ( 6, "Control", LCONTROL )      PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
		KEY ( 7, "F3 F8", F3 )              PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_START ( "keyboard.6" )
		KEY ( 0, "7 \303\250", 7 )          PORT_CHAR('7') PORT_CHAR( 0xe8 )
		KEY ( 1, "8 !", 8 )                 PORT_CHAR('8') PORT_CHAR('!')
		KEY ( 2, "9 \303\247", 9 )          PORT_CHAR('9') PORT_CHAR( 0xe7 )
		KEY ( 3, "0 \303\240", 0 )          PORT_CHAR('0') PORT_CHAR( 0xe0 )
		KEY ( 4, "- \\", BACKSPACE )        PORT_CHAR('-') PORT_CHAR('\\')
		KEY ( 5, "= +", EQUALS )            PORT_CHAR('=') PORT_CHAR('+')
		KEY ( 6, "Accent", END )            PORT_CHAR(UCHAR_MAMEKEY(END))
		KEY ( 7, "F4 F9", F4 )              PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_START ( "keyboard.7" )
		KEY ( 0, "6 _", 6 )                 PORT_CHAR('6') PORT_CHAR('_')
		KEY ( 1, "5 (", 5 )                 PORT_CHAR('5') PORT_CHAR('(')
		KEY ( 2, "4 '", 4 )                 PORT_CHAR('4') PORT_CHAR('\'')
		KEY ( 3, "3 \"", 3 )                PORT_CHAR('3') PORT_CHAR('"')
		KEY ( 4, "2 \303\251", 2 )          PORT_CHAR('2')  PORT_CHAR( 0xe9 )
		KEY ( 5, "1 *", 1 )                 PORT_CHAR('1') PORT_CHAR('*')
		KEY ( 6, "Stop", TAB )              PORT_CHAR(27)
		KEY ( 7, "F5 F10", F5 )             PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_START ( "keyboard.8" )
		KEY ( 0, "[ {", QUOTE )             PORT_CHAR('[') PORT_CHAR('{')
		KEY ( 1, "] }", BACKSLASH )         PORT_CHAR(']') PORT_CHAR('}')
		KEY ( 2, ") \302\260", MINUS )      PORT_CHAR(')') PORT_CHAR( 0xb0 )
		KEY ( 3, "\342\206\221 \302\250", OPENBRACE ) PORT_CHAR('^') PORT_CHAR( 0xa8 )
		KEY ( 4, "\303\271 %", COLON )      PORT_CHAR( 0xf9 ) PORT_CHAR('%')
	PORT_BIT  ( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

		/* unused */
	PORT_START ( "keyboard.9" )

INPUT_PORTS_END

/* QWERTY version */
static INPUT_PORTS_START ( pro128_keyboard )
	PORT_INCLUDE ( mo6_keyboard )

	PORT_MODIFY ( "keyboard.0" )
		KEY ( 1, "M", M )                     PORT_CHAR('M')
		KEY ( 2, ", ;", COMMA )               PORT_CHAR(',') PORT_CHAR(';')
		KEY ( 3, "[ {", QUOTE  )              PORT_CHAR('[') PORT_CHAR('{')
		KEY ( 6, "Z", Z )                     PORT_CHAR('Z')
		KEY ( 7, "Shift", LSHIFT ) PORT_CODE ( KEYCODE_RSHIFT ) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_MODIFY ( "keyboard.1" )
		KEY ( 2, "- _", MINUS )               PORT_CHAR('-') PORT_CHAR('_')
	PORT_MODIFY ( "keyboard.2" )
		KEY ( 3, "\303\221", TILDE )          PORT_CHAR( 0xd1 )
	PORT_MODIFY ( "keyboard.3" )
		KEY ( 5, "A \140", A )                PORT_CHAR('A')
	PORT_MODIFY ( "keyboard.4" )
		KEY ( 4, ". :", STOP )                PORT_CHAR('.') PORT_CHAR(':')
		KEY ( 5, "+ *", BACKSPACE )           PORT_CHAR('+') PORT_CHAR('*')
	PORT_MODIFY ( "keyboard.5" )
		KEY ( 4, "W", W )                     PORT_CHAR('W')
		KEY ( 5, "Q", Q )                     PORT_CHAR('Q')
	PORT_MODIFY ( "keyboard.6" )
		KEY ( 0, "7 /", 7 )                   PORT_CHAR('7') PORT_CHAR('/')
		KEY ( 1, "8 (", 8 )                   PORT_CHAR('8') PORT_CHAR('(')
		KEY ( 2, "9 )", 9 )                   PORT_CHAR('9') PORT_CHAR(')')
		KEY ( 3, "0 =", 0 )                   PORT_CHAR('0') PORT_CHAR('=')
		KEY ( 4, "' \302\243", CLOSEBRACE )   PORT_CHAR('\'') PORT_CHAR( 0xa3 )
		KEY ( 5, "] }", BACKSLASH )           PORT_CHAR(']') PORT_CHAR('}')
	PORT_MODIFY ( "keyboard.7" )
		KEY ( 0, "6 &", 6 )                   PORT_CHAR('6') PORT_CHAR('&')
		KEY ( 1, "5 %", 5 )                   PORT_CHAR('5') PORT_CHAR('%')
		KEY ( 2, "4 $", 4 )                   PORT_CHAR('4') PORT_CHAR('$')
		KEY ( 3, "3 \302\247", 3 )            PORT_CHAR('3') PORT_CHAR( 0xa7 )
		KEY ( 4, "2 \"", 2 )                  PORT_CHAR('2') PORT_CHAR('"')
		KEY ( 5, "1 !", 1 )                   PORT_CHAR('1') PORT_CHAR('!')
	PORT_MODIFY ( "keyboard.8" )
		KEY ( 0, "> <", BACKSLASH2 )          PORT_CHAR('>') PORT_CHAR('<')
		KEY ( 1, "# \342\206\221", EQUALS )   PORT_CHAR('#') PORT_CHAR('^')
		KEY ( 2, "\303\247 ?", COLON )        PORT_CHAR( 0xe7 ) PORT_CHAR('?')
		KEY ( 3, "\302\277 @", SLASH )        PORT_CHAR( 0xbf ) PORT_CHAR('@')
		KEY ( 4, "\302\241 \302\250", OPENBRACE )  PORT_CHAR( 0xa1 ) PORT_CHAR( 0xa8 )

INPUT_PORTS_END


static INPUT_PORTS_START ( mo6 )
	PORT_INCLUDE ( thom_lightpen )
	PORT_INCLUDE ( thom_game_port )
	PORT_INCLUDE ( mo6_keyboard )
	PORT_INCLUDE ( to7_config )
	PORT_INCLUDE ( to7_fconfig )
	PORT_INCLUDE ( to7_vconfig )
INPUT_PORTS_END

static INPUT_PORTS_START ( pro128 )
	PORT_INCLUDE ( thom_lightpen )
	PORT_INCLUDE ( thom_game_port )
	PORT_INCLUDE ( pro128_keyboard )
	PORT_INCLUDE ( to7_config )
	PORT_INCLUDE ( to7_fconfig )
	PORT_INCLUDE ( to7_vconfig )
INPUT_PORTS_END


/* ------------ driver ------------ */

static MACHINE_CONFIG_DERIVED( mo6, to7 )
	MCFG_MACHINE_START_OVERRIDE( thomson_state, mo6 )
	MCFG_MACHINE_RESET_OVERRIDE( thomson_state, mo6 )

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP ( mo6)

	MCFG_CASSETTE_MODIFY( "cassette" )
	MCFG_CASSETTE_FORMATS(mo5_cassette_formats)

	MCFG_DEVICE_REMOVE( "mc6846" )

	MCFG_DEVICE_MODIFY(THOM_PIA_SYS)
	MCFG_PIA_READPA_HANDLER(READ8(thomson_state, mo6_sys_porta_in))
	MCFG_PIA_READPB_HANDLER(READ8(thomson_state, mo6_sys_portb_in))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(thomson_state, mo6_sys_porta_out))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(thomson_state, mo6_sys_portb_out))
	MCFG_PIA_CA2_HANDLER(WRITELINE(thomson_state, mo5_set_cassette_motor))
	MCFG_PIA_CB2_HANDLER(WRITELINE(thomson_state, mo6_sys_cb2_out))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(thomson_state, thom_irq_1)) /* differs from TO */

	MCFG_DEVICE_MODIFY(THOM_PIA_GAME)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(thomson_state, mo6_game_porta_out))
	MCFG_PIA_CB2_HANDLER(WRITELINE(thomson_state, mo6_game_cb2_out))

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(thomson_state, write_centronics_busy))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_DEVICE_REMOVE("cartslot")
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "mo5_cart")
	MCFG_GENERIC_EXTENSIONS("m5,rom")
	MCFG_GENERIC_LOAD(thomson_state, mo5_cartridge)

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pro128, mo6 )
MACHINE_CONFIG_END

COMP ( 1986, mo6, 0, 0, mo6, mo6, driver_device, 0, "Thomson", "MO6", 0 )

COMP ( 1986, pro128, mo6, 0, pro128, pro128, driver_device, 0, "Olivetti / Thomson", "Prodest PC 128", 0 )




/****************************** MO5NR ********************************

MO5 NR (1986)
------

Despite its name, the MO5 NR is much more related to the MO6 than to the MO5.
It can be though as the network-enhanced version of the MO6.
It is both MO5 and MO6 compatible (but not TO-compatible).

Here are the differences between the MO6 and MO5NR:

* chips:
  - integrated MC 6854 network controller

* memory:
  - extra 2 KB ROM for the integrated network controller,
    can be masked by the ROM from the external floppy controller

* video: identical

* devices:
  - AZERTY keyboard has only 58 keys, and no caps-lock led
  - CENTRONICS printer handled differently
  - MO5-compatible network (probably identical to NR 07-005 extension)
  - extern floppy controller & drive possible, masks the network

**********************************************************************/

static ADDRESS_MAP_START ( mo5nr, AS_PROGRAM, 8, thomson_state )

	AM_RANGE ( 0x0000, 0x1fff ) AM_READ_BANK ( THOM_VRAM_BANK) AM_WRITE(to770_vram_w )
	AM_RANGE ( 0x2000, 0x3fff ) AM_READ_BANK ( TO8_SYS_LO) AM_WRITE(to8_sys_lo_w )
	AM_RANGE ( 0x4000, 0x5fff ) AM_READ_BANK ( TO8_SYS_HI) AM_WRITE(to8_sys_hi_w )
	AM_RANGE ( 0x6000, 0x7fff ) AM_READ_BANK ( TO8_DATA_LO) AM_WRITE(to8_data_lo_w )
	AM_RANGE ( 0x8000, 0x9fff ) AM_READ_BANK ( TO8_DATA_HI) AM_WRITE(to8_data_hi_w )
	AM_RANGE ( 0xa000, 0xa7bf ) AM_ROMBANK   ( THOM_FLOP_BANK )
	AM_RANGE ( 0xa7c0, 0xa7c3 ) AM_DEVREADWRITE( "pia_0", pia6821_device, read_alt, write_alt)
	AM_RANGE ( 0xa7cb, 0xa7cb ) AM_WRITE(mo6_ext_w )
	AM_RANGE ( 0xa7cc, 0xa7cf ) AM_DEVREADWRITE( "pia_1", pia6821_device, read_alt, write_alt)
	AM_RANGE ( 0xa7d0, 0xa7d9 ) AM_READWRITE(mo5nr_net_r, mo5nr_net_w )
	AM_RANGE ( 0xa7da, 0xa7dd ) AM_READWRITE(mo6_vreg_r, mo6_vreg_w )
	AM_RANGE ( 0xa7e1, 0xa7e1 ) AM_DEVREAD("cent_data_in", input_buffer_device, read)
	AM_RANGE ( 0xa7e1, 0xa7e1 ) AM_DEVWRITE("cent_data_out", output_latch_device, write)
	AM_RANGE ( 0xa7e3, 0xa7e3 ) AM_READWRITE(mo5nr_prn_r, mo5nr_prn_w )
	AM_RANGE ( 0xa7e4, 0xa7e7 ) AM_READWRITE(mo6_gatearray_r, mo6_gatearray_w )
	AM_RANGE ( 0xa7e8, 0xa7eb ) AM_DEVREADWRITE( "acia",  mos6551_device, read, write )
/*   AM_RANGE ( 0xa7f0, 0xa7f7 ) AM_READWRITE(to9_ieee_r, to9_ieee_w ) */
	AM_RANGE ( 0xa7f2, 0xa7f3 ) AM_READWRITE(to7_midi_r, to7_midi_w )
	AM_RANGE ( 0xa7f8, 0xa7fb ) AM_DEVREADWRITE( "pia_3", pia6821_device, read_alt, write_alt)
	AM_RANGE ( 0xa7fe, 0xa7ff ) AM_DEVREADWRITE("mea8000", mea8000_device, read, write)
	AM_RANGE ( 0xb000, 0xbfff ) AM_ROMBANK   ( MO6_CART_LO )
					AM_WRITE     ( mo6_cartridge_w )
	AM_RANGE ( 0xc000, 0xefff ) AM_ROMBANK   ( MO6_CART_HI )
					AM_WRITE     ( mo6_cartridge_w )
	AM_RANGE ( 0xf000, 0xffff ) AM_ROMBANK   ( TO8_BIOS_BANK )

/* 0x10000 - 0x1ffff: 64 KB external ROM cartridge */
/* 0x20000 - 0x2ffff: 64 KB BIOS ROM */
/* 0x30000 - 0x347ff: 16 KB floppy / network ROM controllers */

/* RAM mapping: 128 KB flat (including video) */

ADDRESS_MAP_END


/* ------------ ROMS ------------ */

ROM_START ( mo5nr )
	ROM_REGION ( 0x34800, "maincpu", 0 )

		/* BIOS */
	ROM_LOAD ( "mo5nr-0.rom", 0x23000, 0x1000,
		CRC(06e31115)
		SHA1(7429cc0c15475398b5ab514cb3d3efdc71cf082f) )
	ROM_LOAD ( "mo5nr-1.rom", 0x27000, 0x1000,
		CRC(7cda17c9)
		SHA1(2ff6480ce9e30acc4c89b6113d7c8ea6095d90a5) )

		/* BASIC */
	ROM_LOAD ( "basicn-0.rom", 0x20000, 0x3000,
		CRC(fae9e691)
		SHA1(62fbfd6d4ca837f6cb8ed37f828eca97f80e6200) )
	ROM_LOAD ( "basicn-1.rom", 0x24000, 0x3000,
		CRC(cf134dd7)
		SHA1(1bd961314e16e460d37a65f5e7f4acf5604fbb17) )
	ROM_LOAD ( "basicn-2.rom", 0x28000, 0x4000,
		CRC(b69d2e0d)
		SHA1(ea3220bbae991e08259d38a7ea24533b2bb86418) )
	ROM_LOAD ( "basicn-3.rom", 0x2c000, 0x4000,
		CRC(7785610f)
		SHA1(c38b0be404d8af6f409a1b52cb79a4e10fc33177) )

	ROM_FLOPPY5 ( 0x30000 )
	ROM_FILL ( 0x10000, 0x10000, 0x39 ) /* TODO: network ROM */
ROM_END




/* ------------ inputs   ------------ */

static INPUT_PORTS_START ( mo5nr_keyboard )

	PORT_START ( "keyboard.0" )
		KEY ( 0, "N", N )                   PORT_CHAR('N')
		KEY ( 1, ", <", COMMA )             PORT_CHAR(',') PORT_CHAR('<')
		KEY ( 2, ". >", STOP )              PORT_CHAR('.') PORT_CHAR('>')
		KEY ( 3, "@ \342\206\221", TILDE )  PORT_CHAR('@') PORT_CHAR('^')
		KEY ( 4, "Space Caps-Lock", SPACE ) PORT_CHAR(' ') PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
		KEY ( 5, "X", X )                   PORT_CHAR('X')
		KEY ( 6, "W", W )                   PORT_CHAR('W')
		KEY ( 7, "Shift", LSHIFT ) PORT_CODE ( KEYCODE_RSHIFT ) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_START ( "keyboard.1" )
		KEY ( 0, "Delete Backspace", DEL )  PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE))
		KEY ( 1, "Insert", INSERT )         PORT_CHAR(UCHAR_MAMEKEY(INSERT))
		KEY ( 2, "Home", HOME )             PORT_CHAR(UCHAR_MAMEKEY(HOME))
		KEY ( 3, UTF8_RIGHT, RIGHT )    PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		KEY ( 4, UTF8_DOWN, DOWN )     PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		KEY ( 5, UTF8_LEFT, LEFT )     PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		KEY ( 6, UTF8_UP, UP )       PORT_CHAR(UCHAR_MAMEKEY(UP))
		KEY ( 7, "BASIC", RCONTROL )
	PORT_START ( "keyboard.2" )
		KEY ( 0, "J", J )                   PORT_CHAR('J')
		KEY ( 1, "K", K )                   PORT_CHAR('K')
		KEY ( 2, "L", L )                   PORT_CHAR('L')
		KEY ( 3, "M", M )                   PORT_CHAR('M')
		KEY ( 4, "B \140", B )              PORT_CHAR('B')
		KEY ( 5, "V", V )                   PORT_CHAR('V')
		KEY ( 6, "C \136", C )              PORT_CHAR('C')
	PORT_BIT  ( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START ( "keyboard.3" )
		KEY ( 0, "H \302\250", H )          PORT_CHAR('H')
		KEY ( 1, "G", G )                   PORT_CHAR('G')
		KEY ( 2, "F", F )                   PORT_CHAR('F')
		KEY ( 3, "D", D )                   PORT_CHAR('D')
		KEY ( 4, "S", S )                   PORT_CHAR('S')
		KEY ( 5, "Q", Q )                   PORT_CHAR('Q')
		KEY ( 6, "Clear", ESC )             PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT  ( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START ( "keyboard.4" )
		KEY ( 0, "U", U )                   PORT_CHAR('U')
		KEY ( 1, "I", I )                   PORT_CHAR('I')
		KEY ( 2, "O", O )                   PORT_CHAR('O')
		KEY ( 3, "P", P )                   PORT_CHAR('P')
		KEY ( 4, "/ ?", SLASH )             PORT_CHAR('/') PORT_CHAR('?')
		KEY ( 5, "* :", QUOTE )             PORT_CHAR('*') PORT_CHAR(':')
		KEY ( 6, "Enter", ENTER )           PORT_CHAR(13)
	PORT_BIT  ( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START ( "keyboard.5" )
		KEY ( 0, "Y", Y )                   PORT_CHAR('Y')
		KEY ( 1, "T", T )                   PORT_CHAR('T')
		KEY ( 2, "R", R )                   PORT_CHAR('R')
		KEY ( 3, "E", E )                   PORT_CHAR('E')
		KEY ( 4, "Z", Z )                   PORT_CHAR('Z')
		KEY ( 5, "A \140", A )              PORT_CHAR('A')
		KEY ( 6, "Control", LCONTROL )      PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT  ( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START ( "keyboard.6" )
		KEY ( 0, "7 ' \303\250", 7 )          PORT_CHAR('7') PORT_CHAR('\'' )
		KEY ( 1, "8 ( \303\271", 8 )          PORT_CHAR('8') PORT_CHAR('(')
		KEY ( 2, "9 ) \303\247", 9 )          PORT_CHAR('9') PORT_CHAR(')')
		KEY ( 3, "0 \303\240", 0 )            PORT_CHAR('0') PORT_CHAR( 0xe0 )
		KEY ( 4, "- =", MINUS )               PORT_CHAR('-') PORT_CHAR('=')
		KEY ( 5, "+ ;", EQUALS )              PORT_CHAR('+') PORT_CHAR(';')
		KEY ( 6, "Accent", END )              PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT  ( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START ( "keyboard.7" )
		KEY ( 0, "6 & \303\251", 6 )          PORT_CHAR('6') PORT_CHAR('&')
		KEY ( 1, "5 %", 5 )                   PORT_CHAR('5') PORT_CHAR('%')
		KEY ( 2, "4 $", 4 )                   PORT_CHAR('4') PORT_CHAR('$')
		KEY ( 3, "3 #", 3 )                   PORT_CHAR('3') PORT_CHAR('#')
		KEY ( 4, "2 \"", 2 )                  PORT_CHAR('2') PORT_CHAR('"')
		KEY ( 5, "1 !", 1 )                   PORT_CHAR('1') PORT_CHAR('!')
		KEY ( 6, "Stop", TAB )                PORT_CHAR(27)
	PORT_BIT  ( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

		/* unused */
	PORT_START ( "keyboard.8" )
	PORT_START ( "keyboard.9" )

INPUT_PORTS_END

static INPUT_PORTS_START ( mo5nr )
	PORT_INCLUDE ( thom_lightpen )
	PORT_INCLUDE ( thom_game_port )
	PORT_INCLUDE ( mo5nr_keyboard )
	PORT_INCLUDE ( to7_config )
	PORT_INCLUDE ( to7_fconfig )
	PORT_INCLUDE ( to7_vconfig )
INPUT_PORTS_END


/* ------------ driver ------------ */

static MACHINE_CONFIG_DERIVED( mo5nr, to7 )
	MCFG_MACHINE_START_OVERRIDE( thomson_state, mo5nr )
	MCFG_MACHINE_RESET_OVERRIDE( thomson_state, mo5nr )

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP ( mo5nr)

	MCFG_DEVICE_REMOVE( "mc6846" )

	MCFG_DEVICE_MODIFY(THOM_PIA_SYS)
	MCFG_PIA_READPA_HANDLER(READ8(thomson_state, mo6_sys_porta_in))
	MCFG_PIA_READPB_HANDLER(READ8(thomson_state, mo5nr_sys_portb_in))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(thomson_state, mo5nr_sys_porta_out))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(thomson_state, mo6_sys_portb_out))
	MCFG_PIA_CA2_HANDLER(WRITELINE(thomson_state, mo5_set_cassette_motor))
	MCFG_PIA_CB2_HANDLER(WRITELINE(thomson_state, mo6_sys_cb2_out))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(thomson_state, thom_irq_1)) /* differs from TO */

	MCFG_DEVICE_MODIFY(THOM_PIA_GAME)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(thomson_state, mo6_game_porta_out))

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_DATA_INPUT_BUFFER("cent_data_in")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(thomson_state, write_centronics_busy))

	MCFG_DEVICE_ADD("cent_data_in", INPUT_BUFFER, 0)
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_DEVICE_REMOVE("cartslot")
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "mo5_cart")
	MCFG_GENERIC_EXTENSIONS("m5,rom")
	MCFG_GENERIC_LOAD(thomson_state, mo5_cartridge)

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
MACHINE_CONFIG_END

COMP ( 1986, mo5nr, 0, 0, mo5nr, mo5nr, driver_device, 0, "Thomson", "MO5 NR", 0 )
