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

   - many copy-protected cassettes that work in DCMOTO fail to load correctly
     (mostly by Infogrames and Loriciels)
   - floppy issues still remain (such as many recent TO8 slideshows giving I/O errors)
   - MO6/MO5NR cartridge is completely broken (garbage screen/hangs)
   - TO8/TO9+ cassette is completely broken (I/O error)
   - add several clones that are emulated in DCMOTO
   - internal, keyboard-attached TO9 mouse port (untested)
   - floppy: 2-sided or 4-sided .fd images, modernization
   - printer post-processing => postscript
   - RS232 serial port extensions: CC 90-232, RF 57-932
   - modem, teltel extension: MD 90-120 / MD 90-333 (need controller ROM?)
   - IEEE extension
   - TV overlay (IN 57-001) (@)
   - digitisation extension (DI 90-011) (@)
   - barcode reader (@)

   (@) means MAME is lacking support for this kind of device / feature anyway

*/

#include "emu.h"
#include "thomson.h"

#include "bus/centronics/ctronics.h"
#include "bus/thomson/cc90_232.h"
#include "bus/thomson/cd90_015.h"
#include "bus/thomson/cq90_028.h"
#include "bus/thomson/cd90_351.h"
#include "bus/thomson/cd90_640.h"
#include "bus/thomson/md90_120.h"
#include "bus/thomson/midipak.h"
#include "bus/thomson/nanoreseau.h"
#include "bus/thomson/rf57_932.h"
#include "bus/thomson/speech.h"
#include "machine/clock.h"
#include "machine/ram.h"
#include "machine/thmfc1.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/sap_dsk.h"
#include "formats/thom_cas.h"
#include "formats/thom_dsk.h"

#include "utf8.h"


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
  Two generations of game port extensions were developed

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

#define TO7_LIGHTPEN_DECAL 17 /* horizontal lightpen shift, stored in $60D2 */
#define MO5_LIGHTPEN_DECAL 12
#define TO9_LIGHTPEN_DECAL 8
#define TO8_LIGHTPEN_DECAL 16
#define MO6_LIGHTPEN_DECAL 12

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
    the MAME cartridge device is named -cart
  - cassette 900 bauds (frequency signals: 0=4.5 kHz, 1=6.3 kHz)
    the MAME cassette device is named -cass
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
    . MAME floppy devices are named -flop0 to -flop3
  - alternate 5"1/2 floppy drive extension
    . CD 90-015 floppy controller, based on a HD 46503 S
    . UD 90-070 5"1/4 single-sided single density floppy drive
  - alternate 3"1/2 floppy drive extension
    . CD 90-351 floppy controller, based on a custom Thomson gate-array
    . DD 90-352 3"1/2 floppy drives
  - alternate QDD floppy drive extension
    . CQ 90-028 floppy controller, based on a Motorola 6852 SSDA
    . QD 90-028 quickdrive 2"8 (QDD), only one drive, single side
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
Actually, the two computers are indistinguishable, except for the different
startup screen, and a couple BIOS addresses.
They can run the same software and accept the same devices and extensions.


**********************************************************************/

/* ------------ address maps ------------ */

void thomson_state::to7_map(address_map &map)
{

	map(0x0000, 0x3fff).bankr(THOM_CART_BANK).w(FUNC(thomson_state::to7_cartridge_w)); /* 4 * 16 KB */
	map(0x4000, 0x5fff).bankr(THOM_VRAM_BANK).w(FUNC(thomson_state::to7_vram_w));
	map(0x6000, 0x7fff).bankrw(THOM_BASE_BANK); /* 1 * 8 KB */
	map(0x8000, 0xdfff).bankrw(THOM_RAM_BANK);  /* 16 or 24 KB (for extension) */
	map(0xe7c0, 0xe7c7).rw(m_mc6846, FUNC(mc6846_device::read), FUNC(mc6846_device::write));
	map(0xe7c8, 0xe7cb).rw("pia_0", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xe7cc, 0xe7cf).rw("pia_1", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xe800, 0xefff).rom().region("mc6846", 0);
	map(0xf000, 0xffff).rom().region("monitor", 0);

/* 0x10000 - 0x1ffff: 64 KB external ROM cartridge */
/* 18 KB floppy / network ROM controllers */

/* RAM mapping:
   0x0000 - 0x3fff: 16 KB video RAM (actually 8 K x 8 bits + 8 K x 6 bits)
   0x4000 - 0x5fff:  8 KB base RAM
   0x6000 - 0x9fff: 16 KB extended RAM
   0xa000 - 0xbfff:  8 KB more extended RAM
 */
}



/* ------------ ROMS ------------ */

ROM_START ( to7 )
	ROM_REGION ( 0x1000, "monitor", 0 )
	ROM_LOAD ( "to7.u3", 0x0000, 0x1000, CRC(99f73da8) SHA1(416981860b44934b2ebf0192080b2cdd79c2c8d5) )

	ROM_REGION ( 0x800, "mc6846", 0 )
	ROM_LOAD ( "tha010_ef6846p.u1", 0x000, 0x800, CRC(39d74cec) SHA1(6428fe9439a1f09c2864697d40da9c0b72a52ca1) )

	ROM_REGION ( 0x20, "proms", 0 )
	ROM_LOAD ( "6331-1.u11", 0x00, 0x20, NO_DUMP ) // address decode

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL ( 0x00000, 0x10000, 0x39 )
ROM_END

ROM_START ( t9000 )
	ROM_REGION ( 0x1000, "monitor", 0 )
	ROM_LOAD ( "t9000.bin", 0x0000, 0x1000, CRC(5cd41431) SHA1(8bf1a40964b76584ee8c83e62be6635de8d1ccd0) )

	ROM_REGION ( 0x800, "mc6846", 0 )
	ROM_LOAD ( "t9000_mc6846.bin", 0x000, 0x800, CRC(8987b838) SHA1(9fee6bb31dc3b39265e7ebc73b44943d4115a516) )

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL ( 0x00000, 0x10000, 0x39 )
ROM_END


/* ------------ inputs   ------------ */

static INPUT_PORTS_START ( to7_config )
	PORT_START ( "config" )

	PORT_CONFNAME ( 0x01, 0x00, "Game Port" )
	PORT_CONFSETTING ( 0x00, DEF_STR( Joystick ) )
	PORT_CONFSETTING ( 0x01, "Mouse" )

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

INPUT_PORTS_END

static INPUT_PORTS_START ( to7 )
	PORT_INCLUDE ( thom_lightpen )
	PORT_INCLUDE ( thom_game_port )
	PORT_INCLUDE ( to7_keyboard )
	PORT_INCLUDE ( to7_config )
INPUT_PORTS_END

static INPUT_PORTS_START ( t9000 )
	PORT_INCLUDE ( to7 )
INPUT_PORTS_END

static void to9_floppy_drives(device_slot_interface &device)
{
	device.option_add("dd90_352", FLOPPY_35_DD);
}

static void to8_floppy_drives(device_slot_interface &device)
{
	device.option_add("dd90_352", FLOPPY_35_DD);
	//  device.option_add("qd90_280", FLOPPY_28_QDD);
}

static void to35_floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_THOMSON_35_FORMAT);
	fr.add(FLOPPY_SAP_FORMAT);
}

/* ------------ driver ------------ */

void thomson_state::to7_base(machine_config &config, bool is_mo)
{
	MCFG_MACHINE_START_OVERRIDE( thomson_state, to7 )
	MCFG_MACHINE_RESET_OVERRIDE( thomson_state, to7 )

/* cpu */
	MC6809E(config, m_maincpu, 16_MHz_XTAL / 16);
	m_maincpu->set_addrmap(AS_PROGRAM, &thomson_state::to7_map);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	INPUT_MERGER_ANY_HIGH(config, "mainfirq").output_handler().set_inputline(m_maincpu, M6809_FIRQ_LINE);

	if (!is_mo)
	{
		/* timer */
		MC6846(config, m_mc6846, 16_MHz_XTAL / 16);
		m_mc6846->out_port().set(FUNC(thomson_state::to7_timer_port_out));
		m_mc6846->in_port().set(FUNC(thomson_state::to7_timer_port_in));
		m_mc6846->cp2().set("buzzer", FUNC(dac_bit_interface::write));
		m_mc6846->cto().set(FUNC(thomson_state::to7_set_cassette));
		m_mc6846->irq().set("mainirq", FUNC(input_merger_device::in_w<0>));
	}

/* video */
	SCREEN(config, "screen", SCREEN_TYPE_RASTER).set_palette("palette");

	PALETTE(config, "palette", FUNC(thomson_state::thom_palette), 4097); // 12-bit color + transparency

/* sound */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "buzzer", 0).add_route(ALL_OUTPUTS, "speaker", 0.5);
	DAC_6BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // 6-bit game extension R-2R DAC (R=10K)

/* cassette */
	CASSETTE(config, m_cassette);
	m_cassette->set_formats(to7_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->set_interface("to_cass");

/* extension port */
	THOMSON_EXTENSION(config, m_extension, 16_MHz_XTAL / 16);
	m_extension->firq_callback().set("mainfirq", FUNC(input_merger_device::in_w<3>));
	m_extension->irq_callback().set("mainirq", FUNC(input_merger_device::in_w<3>));
	m_extension->option_add("cc90_232", CC90_232);
	m_extension->option_add("cd90_015", CD90_015);
	m_extension->option_add("cq90_028", CQ90_028);
	m_extension->option_add("cd90_351", CD90_351);
	m_extension->option_add("cd90_640", CD90_640);
	m_extension->option_add("md90_120", MD90_120);
	m_extension->option_add("midipak", LOGIMUS_MIDIPAK);
	m_extension->option_add("rf57_932", RF57_932);
	m_extension->option_add("speech", THOMSON_SPEECH);
	if(is_mo)
		m_extension->option_add("nanoreseau", NANORESEAU_MO);
	else
		m_extension->option_add("nanoreseau", NANORESEAU_TO);

/* pia */
	PIA6821(config, m_pia_sys);
	m_pia_sys->readpa_handler().set(FUNC(thomson_state::to7_sys_porta_in));
	m_pia_sys->readpb_handler().set(FUNC(thomson_state::to7_sys_portb_in));
	m_pia_sys->writepb_handler().set(FUNC(thomson_state::to7_sys_portb_out));
	m_pia_sys->ca2_handler().set(FUNC(thomson_state::to7_set_cassette_motor));
	m_pia_sys->cb2_handler().set(FUNC(thomson_state::to7_sys_cb2_out));
	m_pia_sys->irqa_handler().set("mainfirq", FUNC(input_merger_device::in_w<0>));
	m_pia_sys->irqb_handler().set("mainfirq", FUNC(input_merger_device::in_w<1>));

	PIA6821(config, m_pia_game);
	m_pia_game->readpa_handler().set(FUNC(thomson_state::to7_game_porta_in));
	m_pia_game->readpb_handler().set(FUNC(thomson_state::to7_game_portb_in));
	m_pia_game->writepb_handler().set(FUNC(thomson_state::to7_game_portb_out));
	m_pia_game->cb2_handler().set(FUNC(thomson_state::to7_game_cb2_out));
	m_pia_game->irqa_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));
	m_pia_game->irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<2>));

/* cartridge */
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "to_cart", "m7,rom").set_device_load(FUNC(thomson_state::to7_cartridge));

/* internal ram */
	RAM(config, m_ram).set_default_size("40K").set_extra_options("24K,48K");

/* software lists */
	SOFTWARE_LIST(config, "to7_cart_list").set_original("to7_cart");
	SOFTWARE_LIST(config, "to7_cass_list").set_original("to7_cass");
	SOFTWARE_LIST(config, "to_flop_list").set_original("to_flop");
	SOFTWARE_LIST(config, "to7_qd_list").set_original("to7_qd");
}

void thomson_state::to7(machine_config &config)
{
	to7_base(config, false);

	MC6809(config.replace(), m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &thomson_state::to7_map);

	TO7_VIDEO(config, m_video, 16_MHz_XTAL);
	m_video->set_screen("screen");
	m_video->set_lightpen_decal(TO7_LIGHTPEN_DECAL);
	m_video->set_lightpen_steps(3);
	m_video->set_vram_page_cb(FUNC(thomson_state::get_vram_page));
	m_video->set_lightpen_step_cb(FUNC(thomson_state::to7_lightpen_cb));
	m_video->init_cb().set(m_pia_sys, FUNC(pia6821_device::ca1_w));
}

void thomson_state::t9000(machine_config &config)
{
	to7(config);
}


COMP( 1982, to7, 0, 0, to7, to7, thomson_state, empty_init, "Thomson", "TO7", 0 )

COMP( 1980, t9000, to7, 0, t9000, t9000, thomson_state, empty_init, "Thomson", "T9000", 0 )


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

void thomson_state::to770_map(address_map &map)
{
	map(0x0000, 0x3fff).bankr(THOM_CART_BANK).w(FUNC(thomson_state::to7_cartridge_w)); /* 4 * 16 KB */
	map(0x4000, 0x5fff).bankr(THOM_VRAM_BANK).w(FUNC(thomson_state::to770_vram_w));
	map(0x6000, 0x9fff).bankrw(THOM_BASE_BANK); /* 16 KB */
	map(0xa000, 0xdfff).bankrw(THOM_RAM_BANK);  /* 6 * 16 KB */
	map(0xe7c0, 0xe7c7).rw(m_mc6846, FUNC(mc6846_device::read), FUNC(mc6846_device::write));
	map(0xe7c8, 0xe7cb).rw("pia_0", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xe7cc, 0xe7cf).rw("pia_1", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xe7e4, 0xe7e7).rw(m_video, FUNC(to770_video_device::gatearray_r), FUNC(to770_video_device::gatearray_w));
	map(0xe800, 0xefff).rom().region("mc6846", 0);
	map(0xf000, 0xffff).rom().region("monitor", 0);

/* 0x10000 - 0x1ffff: 64 KB external ROM cartridge */
/* 18 KB floppy / network ROM controllers */

/* RAM mapping:
   0x00000 - 0x03fff: 16 KB video RAM
   0x04000 - 0x07fff: 16 KB unbanked base RAM
   0x08000 - 0x1ffff: 6 * 16 KB banked extended RAM
 */

}



/* ------------ ROMS ------------ */

ROM_START ( to770 )
	ROM_REGION ( 0x1000, "monitor", 0 )
	ROM_LOAD ( "to770.i29", 0x0000, 0x1000, CRC(1ede9310) SHA1(264f0167b3e64a894f347ae5e9123f38b993ead1) )

	ROM_REGION ( 0x800, "mc6846", 0 )
	ROM_LOAD ( "tha010_ef6846p.i33", 0x000, 0x800, CRC(39d74cec) SHA1(6428fe9439a1f09c2864697d40da9c0b72a52ca1) )

	ROM_REGION ( 0x20, "proms", 0 )
	ROM_LOAD ( "a2.i23", 0x00, 0x20, NO_DUMP ) // palette

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL ( 0x00000, 0x10000, 0x39 )
ROM_END

ROM_START ( to770a )
	ROM_REGION ( 0x1000, "monitor", 0 )
	ROM_LOAD ( "to770a.bin", 0x0000, 0x1000, CRC(de30bee8) SHA1(5f9bf37979d35a0fa7fa36538a0e2633065b1639) )

	ROM_REGION ( 0x800, "mc6846", 0 )
	ROM_LOAD ( "to770a_mc6846.bin", 0x000, 0x800, CRC(2bf67c9c) SHA1(6bb97045b591bd279c7b93616e703c85e0a5c9b5) )

	ROM_REGION ( 0x20, "proms", 0 )
	ROM_LOAD ( "a2.i23", 0x00, 0x20, NO_DUMP ) // palette

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL ( 0x00000, 0x10000, 0x39 )
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

void thomson_state::to770(machine_config &config)
{
	to7_base(config, false);
	MCFG_MACHINE_START_OVERRIDE( thomson_state, to770 )
	MCFG_MACHINE_RESET_OVERRIDE( thomson_state, to770 )

	m_maincpu->set_addrmap(AS_PROGRAM, &thomson_state::to770_map);

	m_pia_sys->readpa_handler().set(FUNC(thomson_state::to770_sys_porta_in));
	m_pia_sys->readpb_handler().set_constant(0);
	m_pia_sys->writepb_handler().set(FUNC(thomson_state::to770_sys_portb_out));
	m_pia_sys->cb2_handler().set(FUNC(thomson_state::to770_sys_cb2_out));

	m_mc6846->out_port().set(FUNC(thomson_state::to770_timer_port_out));

	/* internal ram */
	m_ram->set_default_size("128K").set_extra_options("64K");

	TO770_VIDEO(config, m_video, 16_MHz_XTAL);
	m_video->set_screen("screen");
	m_video->set_lightpen_decal(TO7_LIGHTPEN_DECAL);
	m_video->set_lightpen_steps(3);
	m_video->set_vram_page_cb(FUNC(thomson_state::get_vram_page));
	m_video->set_lightpen_step_cb(FUNC(thomson_state::to7_lightpen_cb));
	m_video->init_cb().set(m_pia_sys, FUNC(pia6821_device::ca1_w));

	SOFTWARE_LIST(config, "t770_cart_list").set_original("to770_cart");
	SOFTWARE_LIST(config.replace(), "to7_cart_list").set_compatible("to7_cart");
}

void thomson_state::to770a(machine_config &config)
{
	to770(config);
	config.device_remove("t770_cart_list");
	SOFTWARE_LIST(config, "t770a_cart_list").set_original("to770a_cart");
}

COMP( 1984, to770, 0, 0, to770, to770, thomson_state, empty_init, "Thomson", "TO7/70", 0 )

COMP( 1984, to770a, to770, 0, to770a, to770a, thomson_state, empty_init, "Thomson", "TO7/70 (Arabic)", 0 )


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

void mo5_state::mo5_map(address_map &map)
{

	map(0x0000, 0x1fff).bankr(THOM_VRAM_BANK).w(FUNC(mo5_state::to770_vram_w));
	map(0x2000, 0x9fff).bankrw(THOM_BASE_BANK);
	map(0xa7c0, 0xa7c3).rw("pia_0", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xa7cb, 0xa7cb).w(FUNC(mo5_state::mo5_ext_w));
	map(0xa7cc, 0xa7cf).rw("pia_1", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xa7e4, 0xa7e7).rw(m_video, FUNC(to770_video_device::gatearray_r), FUNC(to770_video_device::gatearray_w));
	map(0xb000, 0xefff).bankr(THOM_CART_BANK).w(FUNC(mo5_state::mo5_cartridge_w));
	map(0xf000, 0xffff).rom().region("basic", 0x4000);

/* 0x10000 - 0x1ffff: 16 KB integrated BASIC / 64 KB external cartridge */
/* 18 KB floppy / network ROM controllers */

/* RAM mapping:
   0x00000 - 0x03fff: 16 KB video RAM
   0x04000 - 0x0bfff: 32 KB unbanked base RAM
   0x0c000 - 0x1bfff: 4 * 16 KB bank extended RAM
 */

}



/* ------------ ROMS ------------ */

ROM_START ( mo5 )
	ROM_REGION ( 0x5000, "basic", 0 )
	ROM_LOAD ( "mo5.i04", 0x1000, 0x4000, CRC(237c60bf) SHA1(8d2865996a1a8d8a13fc9965c1bcf490f9621399) )

	ROM_REGION ( 0x20, "proms", 0 )
	ROM_LOAD ( "7603-5.i03", 0x00, 0x20, NO_DUMP ) // palette

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL( 0x00000, 0x10000, 0x39 )
ROM_END

ROM_START ( mo5e )
	ROM_REGION ( 0x5000, "basic", 0 )
	ROM_LOAD ( "mo5e.bin", 0x1000, 0x4000, CRC(56f11cf3) SHA1(0f60c8ad391c48b2e7d02b646509586ad34b7417) )

	ROM_REGION ( 0x20, "proms", 0 )
	ROM_LOAD ( "7603-5.i03", 0x00, 0x20, NO_DUMP ) // palette

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL( 0x00000, 0x10000, 0x39 )
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

void mo5_state::mo5(machine_config &config)
{
	to7_base(config, true);
	MCFG_MACHINE_START_OVERRIDE( mo5_state, mo5 )
	MCFG_MACHINE_RESET_OVERRIDE( mo5_state, mo5 )

	m_maincpu->set_addrmap(AS_PROGRAM, &mo5_state::mo5_map);

	m_cassette->set_formats(mo5_cassette_formats);
	m_cassette->set_interface("mo_cass");

	subdevice<palette_device>("palette")->set_init(FUNC(mo5_state::mo5_palette));

	m_pia_sys->readpa_handler().set(FUNC(mo5_state::mo5_sys_porta_in));
	m_pia_sys->readpb_handler().set(FUNC(mo5_state::mo5_sys_portb_in));
	m_pia_sys->writepa_handler().set(FUNC(mo5_state::mo5_sys_porta_out));
	m_pia_sys->writepb_handler().set("buzzer", FUNC(dac_bit_interface::data_w));
	m_pia_sys->ca2_handler().set(FUNC(mo5_state::mo5_set_cassette_motor));
	m_pia_sys->cb2_handler().set_nop();
	m_pia_sys->irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<0>)); // WARNING: differs from TO7 !

	GENERIC_CARTSLOT(config.replace(), "cartslot", generic_plain_slot, "mo_cart", "m5,rom").set_device_load(FUNC(mo5_state::mo5_cartridge));

	config.device_remove("to7_cart_list");
	config.device_remove("to7_cass_list");
	config.device_remove("to_flop_list");
	config.device_remove("to7_qd_list");

	SOFTWARE_LIST(config, "mo5_cart_list").set_original("mo5_cart");
	SOFTWARE_LIST(config, "mo5_cass_list").set_original("mo5_cass");
	SOFTWARE_LIST(config, "mo5_flop_list").set_original("mo5_flop");
	SOFTWARE_LIST(config, "mo5_qd_list").set_original("mo5_qd");

	/* internal ram */
	m_ram->set_default_size("112K");

	TO770_VIDEO(config, m_video, 16_MHz_XTAL);
	m_video->set_screen("screen");
	m_video->set_is_mo(true);
	m_video->set_lightpen_decal(MO5_LIGHTPEN_DECAL);
	m_video->set_lightpen_steps(3);
	m_video->set_vram_page_cb(FUNC(mo5_state::get_vram_page));
	m_video->set_lightpen_step_cb(FUNC(mo5_state::mo5_lightpen_cb));
	m_video->int_50hz_cb().set(m_pia_sys, FUNC(pia6821_device::cb1_w));
}

void mo5_state::mo5e(machine_config &config)
{
	mo5(config);
}


COMP( 1984, mo5, 0, 0, mo5, mo5, mo5_state, empty_init, "Thomson", "MO5", 0 )

COMP( 1986, mo5e, mo5, 0, mo5e, mo5e, mo5_state, empty_init, "Thomson", "MO5E", 0 )


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
    . integrated floppy controller, based on WD1770 or WD2793
    . integrated one-sided double-density 3''1/2
    . external two-sided double-density 3''1/2, 5''1/4 or QDD (extension)
    . floppies are TO7 and MO5 compatible
  - speech synthesis extension: identical to TO7
  - MIDIPAK MIDI extension: identical to TO7

**********************************************************************/

void to9_state::to9_map(address_map &map)
{

	map(0x0000, 0x3fff).bankr(THOM_CART_BANK).w(FUNC(to9_state::to9_cartridge_w));/* 4 * 16 KB */
	map(0x4000, 0x5fff).bankr(THOM_VRAM_BANK).w(FUNC(to9_state::to770_vram_w));
	map(0x6000, 0x9fff).bankrw(THOM_BASE_BANK); /* 16 KB */
	map(0xa000, 0xdfff).bankrw(THOM_RAM_BANK);  /* 10 * 16 KB */
	map(0xe000, 0xe7af).rom().region("monitor", 0);
	map(0xe7c0, 0xe7c7).rw(m_mc6846, FUNC(mc6846_device::read), FUNC(mc6846_device::write));
	map(0xe7c8, 0xe7cb).rw("pia_0", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xe7cc, 0xe7cf).rw("pia_1", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xe7d0, 0xe7d3).mirror(4).rw(m_fdc, FUNC(wd_fdc_device_base::read), FUNC(wd_fdc_device_base::write));
	map(0xe7d8, 0xe7d8).rw(FUNC(to9_state::to9_floppy_control_r), FUNC(to9_state::to9_floppy_control_w));
	map(0xe7da, 0xe7db).rw(FUNC(to9_state::to9_vreg_r), FUNC(to9_state::to9_vreg_w));
	map(0xe7dc, 0xe7dc).w(m_video, FUNC(to9_video_device::video_mode_w));
	map(0xe7dd, 0xe7dd).w(m_video, FUNC(to9_video_device::border_color_w));
	map(0xe7de, 0xe7df).rw(m_to9_kbd, FUNC(to9_keyboard_device::kbd_acia_r), FUNC(to9_keyboard_device::kbd_acia_w));
	map(0xe7e4, 0xe7e7).rw(m_video, FUNC(to9_video_device::gatearray_r), FUNC(to9_video_device::gatearray_w));
/*  map(0xe7f0, 0xe7f7).rw(FUNC(to9_state::to9_ieee_r), FUNC(to9_state::to9_ieee_w )); */
	map(0xe800, 0xffff).rom().region("monitor", 0x800);

/* 0x10000 - 0x1ffff:  64 KB external ROM cartridge */
/* 0x20000 - 0x3ffff: 128 KB internal software ROM */
/* 18 KB external floppy / network ROM controllers */

/* RAM mapping:
   0x00000 - 0x03fff: 16 KB video RAM
   0x04000 - 0x07fff: 16 KB unbanked base RAM
   0x08000 - 0x2ffff: 10 * 16 KB banked extended RAM
 */

}



/* ------------ ROMS ------------ */

/* NOT WORKING
   these bios seem heavily patched (probably to work with specific emulators
   that trap some bios calls)
 */

ROM_START ( to9 )
	ROM_REGION ( 0x2000, "monitor", 0 )
	ROM_LOAD ( "monitor.i42", 0x0000, 0x2000, /* BIOS & floppy controller */
		CRC(f9278bf7)
		SHA1(9e99e6ae0285950f007b19161de642a4031fe46e) )

		/* BASIC & software */
	ROM_REGION ( 0x20000, "basic", 0 )
	ROM_LOAD ( "basic128.i39",   0x00000, 0x8000, CRC(c9bc204f) SHA1(e4c2a684e9186f49c8092d16f0f74764f51ad86c) )
	ROM_LOAD ( "basic1.i56",     0x08000, 0x8000, CRC(b1469ffc) SHA1(548c631d1272dfa25e3e925adc08f6eeb8e4448e) )
	ROM_LOAD ( "fiches.i38",     0x10000, 0x8000, CRC(3eba1a1a) SHA1(e8ed04d30fb70fda37ac31dd5c2c2e59248cd395) )
	ROM_LOAD ( "paragraphe.i40", 0x18000, 0x8000, CRC(1ff9e47e) SHA1(381c493c07271e3259be13bf9edfbf2b2c81a059) )

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL( 0x00000, 0x10000, 0x39 )
ROM_END


/* ------------ inputs   ------------ */

static INPUT_PORTS_START ( to9 )
	PORT_INCLUDE ( thom_lightpen )
	PORT_INCLUDE ( thom_game_port )

	PORT_START ( "config" )
	PORT_BIT ( 0x01, 0x00, IPT_UNUSED )

	PORT_MODIFY ( "mouse_x" )
	PORT_BIT ( 0xffff, 0x00, IPT_UNUSED )

	PORT_MODIFY ( "mouse_y" )
	PORT_BIT ( 0xffff, 0x00, IPT_UNUSED )

	PORT_MODIFY ( "mouse_button" )
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* ------------ driver ------------ */

void to9_state::to9(machine_config &config)
{
	to7_base(config, false);
	MCFG_MACHINE_START_OVERRIDE( to9_state, to9 )
	MCFG_MACHINE_RESET_OVERRIDE( to9_state, to9 )

	m_maincpu->set_addrmap(AS_PROGRAM, &to9_state::to9_map);

	TO9_KEYBOARD(config, m_to9_kbd);
	m_to9_kbd->irq_cb().set(m_mainirq, FUNC(input_merger_device::in_w<4>));

	m_pia_sys->readpa_handler().set(FUNC(to9_state::to9_sys_porta_in));
	m_pia_sys->readpb_handler().set_constant(0);
	m_pia_sys->writepa_handler().set(FUNC(to9_state::to9_sys_porta_out));
	m_pia_sys->writepb_handler().set(FUNC(to9_state::to9_sys_portb_out));
	m_pia_sys->cb2_handler().set_nop();
	m_pia_sys->irqa_handler().set_nop();

	m_mc6846->out_port().set(FUNC(to9_state::to9_timer_port_out));

	WD1770(config, m_fdc, 16_MHz_XTAL / 2);
	FLOPPY_CONNECTOR(config, m_floppy[0], to9_floppy_drives, "dd90_352", to35_floppy_formats, true).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], to9_floppy_drives, nullptr,    to35_floppy_formats, false).enable_sound(true);

	m_extension->option_remove("cd90_015");
	m_extension->option_remove("cq90_028");
	m_extension->option_remove("cd90_351");
	m_extension->option_remove("cd90_640");
	m_extension->option_remove("nanoreseau");

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(to9_state::write_centronics_busy));

	/* internal ram */
	m_ram->set_default_size("192K").set_extra_options("128K");

	TO9_VIDEO(config, m_video, 16_MHz_XTAL);
	m_video->set_screen("screen");
	m_video->set_lightpen_decal(TO9_LIGHTPEN_DECAL);
	m_video->set_lightpen_steps(3);
	m_video->set_vram_page_cb(FUNC(to9_state::get_vram_page));
	m_video->set_lightpen_step_cb(FUNC(to9_state::to7_lightpen_cb));
}


COMP( 1985, to9, 0, 0, to9, to9, to9_state, empty_init, "Thomson", "TO9", MACHINE_IMPERFECT_COLORS )


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


void to8_state::to8_map(address_map &map)
{
	map(0x0000, 0x3fff).bankr(THOM_CART_BANK).w(FUNC(to8_state::to8_cartridge_w)); /* 4 * 16 KB */
	map(0x4000, 0x5fff).bankr(THOM_VRAM_BANK).w(FUNC(to8_state::to770_vram_w));
	map(0x6000, 0x7fff).bankr(TO8_SYS_LO).w(FUNC(to8_state::to8_sys_lo_w));
	map(0x8000, 0x9fff).bankr(TO8_SYS_HI).w(FUNC(to8_state::to8_sys_hi_w));
	map(0xa000, 0xbfff).bankr(TO8_DATA_LO).w(FUNC(to8_state::to8_data_lo_w));
	map(0xc000, 0xdfff).bankr(TO8_DATA_HI).w(FUNC(to8_state::to8_data_hi_w));
	map(0xe000, 0xffff).bankr(TO8_BIOS_BANK);
	map(0xe7c0, 0xe7ff).unmaprw();
	map(0xe7c0, 0xe7c7).rw(m_mc6846, FUNC(mc6846_device::read), FUNC(mc6846_device::write));
	map(0xe7c8, 0xe7cb).rw("pia_0", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xe7cc, 0xe7cf).rw("pia_1", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xe7d0, 0xe7d7).m("thmfc1", FUNC(thmfc1_device::map));
	map(0xe7da, 0xe7dd).rw(FUNC(to8_state::to8_vreg_r), FUNC(to8_state::to8_vreg_w));
	map(0xe7e4, 0xe7e7).rw(m_video, FUNC(to8_video_device::gatearray_r), FUNC(to8_video_device::gatearray_w));
/*  map(0xe7f0, 0xe7f7).rw(FUNC(to8_state::to9_ieee_r), FUNC(to8_state::to9_ieee_w )); */

/* 0x10000 - 0x1ffff: 64 KB external ROM cartridge */
/* 0x20000 - 0x2ffff: 64 KB internal software ROM */
/* 0x30000 - 0x33fff: 16 KB BIOS ROM */
/* 18 KB external floppy / network ROM controllers */

/* RAM mapping: 512 KB flat (including video) */

}


/* ------------ ROMS ------------ */

ROM_START ( to8 )
		/* BIOS & floppy */
	ROM_REGION ( 0x4000, "monitor", 0 )
	ROM_LOAD ( "to8.iw17", 0x0000, 0x4000, CRC(c2610c13) SHA1(75fffd10494d1ebb78e9068e1b232ede6641ad8c) )

		/* BASIC */
	ROM_REGION ( 0x10000, "basic", 0 )
	ROM_LOAD ( "basic512.iw16", 0x0000, 0x8000, CRC(f45e3592) SHA1(8fd98973bd33f88fb63278a7fba86329076b473f) )
	ROM_LOAD ( "basic1.iw15",   0x8000, 0x8000, CRC(2f4f61fc) SHA1(e0eea9c941113c550ba0c55a5c15f55a64c39060) )

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL( 0x00000, 0x10000, 0x39 )
ROM_END

ROM_START ( to8d )
		/* BIOS & floppy */
	ROM_REGION ( 0x4000, "monitor", 0 )
	ROM_LOAD ( "to8d.iw17", 0x0000, 0x4000, CRC(15fd82d5) SHA1(dd90c326abfec1d28ffc4fe974615870e33a597d) )

		/* BASIC */
	ROM_REGION ( 0x10000, "basic", 0 )
	ROM_LOAD ( "basic.iw15", 0x00000, 0x10000, CRC(ffff0512) SHA1(c474d74a1e315d61e21c74c6a1b26af499b385ea) )

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL( 0x00000, 0x10000, 0x39 )
ROM_END


/* ------------ inputs   ------------ */

static INPUT_PORTS_START ( to8 )
	PORT_INCLUDE ( thom_lightpen )
	PORT_INCLUDE ( thom_game_port )
	PORT_INCLUDE ( to7_config )
INPUT_PORTS_END


static INPUT_PORTS_START ( to8d )
	PORT_INCLUDE ( to8 )
INPUT_PORTS_END

/* ------------ driver ------------ */

void to8_state::to8(machine_config &config)
{
	to7_base(config, false);
	MCFG_MACHINE_START_OVERRIDE( to8_state, to8 )
	MCFG_MACHINE_RESET_OVERRIDE( to8_state, to8 )

	m_maincpu->set_addrmap(AS_PROGRAM, &to8_state::to8_map);

	TO8_KEYBOARD(config, m_to8_kbd);
	m_to8_kbd->data_cb().set(m_mc6846, FUNC(mc6846_device::set_input_cp1));

	m_pia_sys->readpa_handler().set(FUNC(to8_state::to8_sys_porta_in));
	m_pia_sys->readpb_handler().set_constant(0);
	m_pia_sys->writepa_handler().set(FUNC(to8_state::to8_sys_porta_out));
	m_pia_sys->writepb_handler().set(FUNC(to8_state::to8_sys_portb_out));
	m_pia_sys->cb2_handler().set_nop();
	m_pia_sys->irqa_handler().set_nop();

	THMFC1(config, "thmfc1", 16_MHz_XTAL);
	FLOPPY_CONNECTOR(config, "thmfc1:0", to8_floppy_drives, "dd90_352", to35_floppy_formats, false).enable_sound(true);
	FLOPPY_CONNECTOR(config, "thmfc1:1", to8_floppy_drives, nullptr,    to35_floppy_formats, false).enable_sound(true);

	m_extension->option_remove("cd90_015");
	m_extension->option_remove("cq90_028");
	m_extension->option_remove("cd90_351");
	m_extension->option_remove("cd90_640");
	m_extension->option_remove("nanoreseau");

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(to8_state::write_centronics_busy));

	m_mc6846->out_port().set(FUNC(to8_state::to8_timer_port_out));
	m_mc6846->in_port().set(FUNC(to8_state::to8_timer_port_in));
	m_mc6846->cp2().set(FUNC(to8_state::to8_timer_cp2_out));

	/* internal ram */
	m_ram->set_default_size("512K").set_extra_options("256K");

	TO8_VIDEO(config, m_video, 16_MHz_XTAL);
	m_video->set_screen("screen");
	m_video->set_lightpen_decal(TO8_LIGHTPEN_DECAL);
	m_video->set_lightpen_steps(4);
	m_video->set_vram_page_cb(FUNC(to8_state::get_vram_page));
	downcast<to8_video_device &>(*m_video).set_update_ram_bank_cb(FUNC(to8_state::to8_update_ram_bank));
	downcast<to8_video_device &>(*m_video).set_update_cart_bank_cb(FUNC(to8_state::to8_update_cart_bank));
	downcast<to8_video_device &>(*m_video).lightpen_intr_cb().set(m_mainfirq, FUNC(input_merger_device::in_w<2>));

	SOFTWARE_LIST(config, "to8_cass_list").set_original("to8_cass");
	SOFTWARE_LIST(config, "to8_qd_list").set_original("to8_qd");
	SOFTWARE_LIST(config.replace(), "to7_cass_list").set_compatible("to7_cass");
	SOFTWARE_LIST(config.replace(), "to7_qd_list").set_compatible("to7_qd");
}

void to8_state::to8d(machine_config &config)
{
	to8(config);
	subdevice<floppy_connector>("thmfc1:0")->set_fixed(true);
}


COMP( 1986, to8, 0, 0, to8, to8, to8_state, empty_init, "Thomson", "TO8", 0 )

COMP( 1987, to8d, to8, 0, to8d, to8d, to8_state, empty_init, "Thomson", "TO8D", 0 )


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

void to8_state::to9p_map(address_map &map)
{

	map(0x0000, 0x3fff).bankr(THOM_CART_BANK).w(FUNC(to8_state::to8_cartridge_w)); /* 4 * 16 KB */
	map(0x4000, 0x5fff).bankr(THOM_VRAM_BANK).w(FUNC(to8_state::to770_vram_w));
	map(0x6000, 0x7fff).bankr(TO8_SYS_LO).w(FUNC(to8_state::to8_sys_lo_w));
	map(0x8000, 0x9fff).bankr(TO8_SYS_HI).w(FUNC(to8_state::to8_sys_hi_w));
	map(0xa000, 0xbfff).bankr(TO8_DATA_LO).w(FUNC(to8_state::to8_data_lo_w));
	map(0xc000, 0xdfff).bankr(TO8_DATA_HI).w(FUNC(to8_state::to8_data_hi_w));
	map(0xe000, 0xffff).bankr(TO8_BIOS_BANK);
	map(0xe7c0, 0xe7ff).unmaprw();
	map(0xe7c0, 0xe7c7).rw(m_mc6846, FUNC(mc6846_device::read), FUNC(mc6846_device::write));
	map(0xe7c8, 0xe7cb).rw("pia_0", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xe7cc, 0xe7cf).rw("pia_1", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xe7d0, 0xe7d7).m("thmfc1", FUNC(thmfc1_device::map));
	map(0xe7da, 0xe7dd).rw(FUNC(to8_state::to8_vreg_r), FUNC(to8_state::to8_vreg_w));
	map(0xe7de, 0xe7df).rw(m_to9_kbd, FUNC(to9_keyboard_device::kbd_acia_r), FUNC(to9_keyboard_device::kbd_acia_w));
	map(0xe7e4, 0xe7e7).rw(m_video, FUNC(to8_video_device::gatearray_r), FUNC(to8_video_device::gatearray_w));
/*  map(0xe7f0, 0xe7f7).rw(FUNC(to8_state::to9_ieee_r), FUNC(to8_state::to9_ieee_w )); */

/* 0x10000 - 0x1ffff: 64 KB external ROM cartridge */
/* 0x20000 - 0x2ffff: 64 KB internal software ROM */
/* 0x30000 - 0x33fff: 16 KB BIOS ROM */
/* 18 KB external floppy / network ROM controllers */

/* RAM mapping: 512 KB flat (including video) */

}


/* ------------ ROMS ------------ */

ROM_START ( to9p )
		/* BIOS & floppy */
	ROM_REGION ( 0x4000, "monitor", 0 )
	ROM_LOAD ( "monitor.iw12", 0x0000, 0x4000, CRC(9e007126) SHA1(bd37a8099f5015c27fb49682559e68fddd532ddc) )

		/* BASIC */
	ROM_REGION ( 0x10000, "basic", 0 )
	ROM_LOAD ( "basic512.iw13", 0x0000, 0x8000, CRC(f45e3592) SHA1(8fd98973bd33f88fb63278a7fba86329076b473f) )
	ROM_LOAD ( "basic1.iw14",   0x8000, 0x8000, CRC(31f44ec6) SHA1(f7cf04d6560ea207672a6b611a5af5bac8ba3e13) )

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL( 0x00000, 0x10000, 0x39 )
ROM_END


/* ------------ inputs   ------------ */

static INPUT_PORTS_START ( to9p )
	PORT_INCLUDE ( thom_lightpen )
	PORT_INCLUDE ( thom_game_port )
	PORT_INCLUDE ( to7_config )
INPUT_PORTS_END

/* ------------ driver ------------ */

void to8_state::to9p(machine_config &config)
{
	to7_base(config, false);
	MCFG_MACHINE_START_OVERRIDE( to8_state, to9p )
	MCFG_MACHINE_RESET_OVERRIDE( to8_state, to8 )

	m_maincpu->set_addrmap(AS_PROGRAM, &to8_state::to9p_map);

	TO9P_KEYBOARD(config, m_to9_kbd);
	m_to9_kbd->irq_cb().set(m_mainirq, FUNC(input_merger_device::in_w<4>));

	m_pia_sys->readpa_handler().set(FUNC(to8_state::to9p_sys_porta_in));
	m_pia_sys->readpb_handler().set_constant(0);
	m_pia_sys->writepa_handler().set(FUNC(to8_state::to8_sys_porta_out));
	m_pia_sys->writepb_handler().set(FUNC(to8_state::to8_sys_portb_out));
	m_pia_sys->cb2_handler().set_nop();
	m_pia_sys->irqa_handler().set_nop();
	m_pia_sys->irqb_handler().set("mainfirq", FUNC(input_merger_device::in_w<1>));

	THMFC1(config, "thmfc1", 16_MHz_XTAL);
	FLOPPY_CONNECTOR(config, "thmfc1:0", to8_floppy_drives, "dd90_352", to35_floppy_formats, true).enable_sound(true);
	FLOPPY_CONNECTOR(config, "thmfc1:1", to8_floppy_drives, nullptr,    to35_floppy_formats, false).enable_sound(true);

	m_extension->option_remove("cd90_015");
	m_extension->option_remove("cq90_028");
	m_extension->option_remove("cd90_351");
	m_extension->option_remove("cd90_640");
	m_extension->option_remove("nanoreseau");

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(to8_state::write_centronics_busy));

	m_mc6846->out_port().set(FUNC(to8_state::to9p_timer_port_out));
	m_mc6846->in_port().set(FUNC(to8_state::to9p_timer_port_in));
	m_mc6846->cp2().set(FUNC(to8_state::to8_timer_cp2_out));

	/* internal ram */
	m_ram->set_default_size("512K");

	TO8_VIDEO(config, m_video, 16_MHz_XTAL);
	m_video->set_screen("screen");
	m_video->set_lightpen_decal(TO8_LIGHTPEN_DECAL);
	m_video->set_lightpen_steps(4);
	m_video->set_vram_page_cb(FUNC(to8_state::get_vram_page));
	downcast<to8_video_device &>(*m_video).set_update_ram_bank_cb(FUNC(to8_state::to8_update_ram_bank));
	downcast<to8_video_device &>(*m_video).set_update_cart_bank_cb(FUNC(to8_state::to8_update_cart_bank));
	downcast<to8_video_device &>(*m_video).lightpen_intr_cb().set(m_mainfirq, FUNC(input_merger_device::in_w<2>));

	SOFTWARE_LIST(config, "to8_cass_list").set_original("to8_cass");
	SOFTWARE_LIST(config, "to8_qd_list").set_original("to8_qd");
	SOFTWARE_LIST(config.replace(), "to7_cass_list").set_compatible("to7_cass");
	SOFTWARE_LIST(config.replace(), "to7_qd_list").set_compatible("to7_qd");
}

COMP( 1986, to9p, 0, 0, to9p, to9p, to8_state, empty_init, "Thomson", "TO9+", 0 )



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

void mo6_state::mo6_map(address_map &map)
{
	map(0x0000, 0x1fff).bankr(THOM_VRAM_BANK).w(FUNC(mo6_state::to770_vram_w));
	map(0x2000, 0x3fff).bankr(TO8_SYS_LO).w(FUNC(mo6_state::to8_sys_lo_w));
	map(0x4000, 0x5fff).bankr(TO8_SYS_HI).w(FUNC(mo6_state::to8_sys_hi_w));
	map(0x6000, 0x7fff).bankr(TO8_DATA_LO).w(FUNC(mo6_state::to8_data_lo_w));
	map(0x8000, 0x9fff).bankr(TO8_DATA_HI).w(FUNC(mo6_state::to8_data_hi_w));
	map(0xa7c0, 0xa7c3).rw("pia_0", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xa7cb, 0xa7cb).w(FUNC(mo6_state::mo6_ext_w));
	map(0xa7cc, 0xa7cf).rw("pia_1", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xa7da, 0xa7dd).rw(FUNC(mo6_state::mo6_vreg_r), FUNC(mo6_state::mo6_vreg_w));
	map(0xa7e4, 0xa7e7).rw(m_video, FUNC(to8_video_device::gatearray_r), FUNC(to8_video_device::gatearray_w));
/*  map(0xa7f0, 0xa7f7).rw(FUNC(mo6_state::to9_ieee_r), FUNC(homson_state::to9_ieee_w));*/
	map(0xb000, 0xbfff).bankr(MO6_CART_LO).w(FUNC(mo6_state::mo6_cartridge_w));
	map(0xc000, 0xefff).bankr(MO6_CART_HI).w(FUNC(mo6_state::mo6_cartridge_w));
	map(0xf000, 0xffff).bankr(TO8_BIOS_BANK);

/* 0x10000 - 0x1ffff: 64 KB external ROM cartridge */
/* 0x20000 - 0x2ffff: 64 KB BIOS ROM */
/* 16 KB floppy / network ROM controllers */

/* RAM mapping: 128 KB flat (including video) */

}


/* ------------ ROMS ------------ */

ROM_START ( mo6 )
		/* BASIC & BIOS */
	ROM_REGION ( 0x10000, "basic", 0 )
	ROM_LOAD ( "basic1.iw01",   0x0000, 0x8000, CRC(e04c98fc) SHA1(55a9c91a4da0ce455bf0402e6a86e8abdb3c93a0) )
	ROM_LOAD ( "basic128.iw02", 0x8000, 0x8000, CRC(f523ba0e) SHA1(e747a5310d5c137918e033e67b2dd83d12ec75c1) )

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL ( 0x00000, 0x10000, 0x39 )
ROM_END

ROM_START ( pro128 )
		/* BASIC & BIOS */
	ROM_REGION ( 0x10000, "basic", 0 )
	ROM_LOAD ( "pro128.iw01", 0x0000, 0x8000, CRC(c5896603) SHA1(f0410456de778e650db7130f45e05fcd5bfd2024) )
	ROM_LOAD ( "basico.iw02", 0x8000, 0x8000, CRC(7c9a0174) SHA1(65f85edece4a88f3b5d5ed1f83df180705fa3d20) )

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL ( 0x00000, 0x10000, 0x39 )
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
INPUT_PORTS_END

static INPUT_PORTS_START ( pro128 )
	PORT_INCLUDE ( thom_lightpen )
	PORT_INCLUDE ( thom_game_port )
	PORT_INCLUDE ( pro128_keyboard )
	PORT_INCLUDE ( to7_config )
INPUT_PORTS_END


/* ------------ driver ------------ */

void mo6_state::mo6(machine_config &config)
{
	to7_base(config, true);
	MCFG_MACHINE_START_OVERRIDE( mo6_state, mo6 )
	MCFG_MACHINE_RESET_OVERRIDE( mo6_state, mo6 )

	m_maincpu->set_addrmap(AS_PROGRAM, &mo6_state::mo6_map);

	m_cassette->set_formats(mo5_cassette_formats);
	m_cassette->set_interface("mo_cass");

	m_pia_sys->readpa_handler().set(FUNC(mo6_state::mo6_sys_porta_in));
	m_pia_sys->readpb_handler().set(FUNC(mo6_state::mo6_sys_portb_in));
	m_pia_sys->writepa_handler().set(FUNC(mo6_state::mo6_sys_porta_out));
	m_pia_sys->writepb_handler().set("buzzer", FUNC(dac_bit_interface::data_w));
	m_pia_sys->ca2_handler().set(FUNC(mo6_state::mo5_set_cassette_motor));
	m_pia_sys->cb2_handler().set(FUNC(mo6_state::mo6_sys_cb2_out));
	m_pia_sys->irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<0>)); // differs from TO

	m_pia_game->writepa_handler().set(FUNC(mo6_state::mo6_game_porta_out));
	m_pia_game->cb2_handler().set(FUNC(mo6_state::mo6_game_cb2_out));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(mo6_state::write_centronics_busy));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	GENERIC_CARTSLOT(config.replace(), "cartslot", generic_plain_slot, "mo_cart", "m5,rom").set_device_load(FUNC(mo6_state::mo5_cartridge));

	/* internal ram */
	m_ram->set_default_size("128K");

	TO8_VIDEO(config, m_video, 16_MHz_XTAL);
	m_video->set_is_mo(true);
	m_video->set_screen("screen");
	m_video->set_lightpen_decal(MO6_LIGHTPEN_DECAL);
	m_video->set_lightpen_steps(3);
	m_video->set_vram_page_cb(FUNC(mo6_state::get_vram_page));
	downcast<to8_video_device &>(*m_video).set_update_ram_bank_cb(FUNC(mo6_state::mo6_update_ram_bank));
	downcast<to8_video_device &>(*m_video).set_update_cart_bank_cb(FUNC(mo6_state::mo6_update_cart_bank));
	downcast<to8_video_device &>(*m_video).lightpen_intr_cb().set(m_mainfirq, FUNC(input_merger_device::in_w<2>));
	m_video->int_50hz_cb().set(m_pia_sys, FUNC(pia6821_device::cb1_w));

	config.device_remove("to7_cart_list");
	config.device_remove("to7_cass_list");
	config.device_remove("to_flop_list");
	config.device_remove("to7_qd_list");

	SOFTWARE_LIST(config, "mo6_cass_list").set_original("mo6_cass");
	SOFTWARE_LIST(config, "mo6_flop_list").set_original("mo6_flop");

	SOFTWARE_LIST(config, "mo5_cart_list").set_compatible("mo5_cart");
	SOFTWARE_LIST(config, "mo5_cass_list").set_compatible("mo5_cass");
	SOFTWARE_LIST(config, "mo5_flop_list").set_compatible("mo5_flop");
	SOFTWARE_LIST(config, "mo5_qd_list").set_compatible("mo5_qd");
}

void mo6_state::pro128(machine_config &config)
{
	mo6(config);
	config.device_remove("mo6_cass_list");
	config.device_remove("mo6_flop_list");

	config.device_remove("mo5_cart_list");
	config.device_remove("mo5_cass_list");
	config.device_remove("mo5_flop_list");
	config.device_remove("mo5_qd_list");

	SOFTWARE_LIST(config, "p128_cart_list").set_original("pro128_cart");
	SOFTWARE_LIST(config, "p128_cass_list").set_original("pro128_cass");
	SOFTWARE_LIST(config, "p128_flop_list").set_original("pro128_flop");
}

COMP( 1986, mo6, 0, 0, mo6, mo6, mo6_state, empty_init, "Thomson", "MO6", 0 )

COMP( 1986, pro128, mo6, 0, pro128, pro128, mo6_state, empty_init, "Olivetti / Thomson", "Prodest PC 128", 0 )




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
  - CENTRONICS printer interface not built in (requires CC 90-232 extension)
  - MO5-compatible network (probably identical to NR 07-005 extension)
  - extern floppy controller & drive possible, masks the network

**********************************************************************/

void mo5nr_state::mo5nr_map(address_map &map)
{
	map(0x0000, 0x1fff).bankr(THOM_VRAM_BANK).w(FUNC(mo5nr_state::to770_vram_w));
	map(0x2000, 0x3fff).bankr(TO8_SYS_LO).w(FUNC(mo5nr_state::to8_sys_lo_w));
	map(0x4000, 0x5fff).bankr(TO8_SYS_HI).w(FUNC(mo5nr_state::to8_sys_hi_w));
	map(0x6000, 0x7fff).bankr(TO8_DATA_LO).w(FUNC(mo5nr_state::to8_data_lo_w));
	map(0x8000, 0x9fff).bankr(TO8_DATA_HI).w(FUNC(mo5nr_state::to8_data_hi_w));
	map(0xa000, 0xa7ff).view(m_extension_view);
	map(0xa7c0, 0xa7c3).rw("pia_0", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xa7cb, 0xa7cb).w(FUNC(mo5nr_state::mo6_ext_w));
	map(0xa7cc, 0xa7cf).rw("pia_1", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	m_extension_view[1](0xa7d8, 0xa7d9).r(FUNC(mo5nr_state::id_r));
	map(0xa7da, 0xa7dd).rw(FUNC(mo5nr_state::mo6_vreg_r), FUNC(mo5nr_state::mo6_vreg_w));
	map(0xa7e4, 0xa7e7).rw(m_video, FUNC(to8_video_device::gatearray_r), FUNC(to8_video_device::gatearray_w));
/*  map(0xa7f0, 0xa7f7).rw(FUNC(mo5nr_state::to9_ieee_r), FUNC(homson_state::to9_ieee_w));*/
	map(0xb000, 0xbfff).bankr(MO6_CART_LO).w(FUNC(mo5nr_state::mo6_cartridge_w));
	map(0xc000, 0xefff).bankr(MO6_CART_HI).w(FUNC(mo5nr_state::mo6_cartridge_w));
	map(0xf000, 0xffff).bankr(TO8_BIOS_BANK);

/* 0x10000 - 0x1ffff: 64 KB external ROM cartridge */
/* 0x20000 - 0x2ffff: 64 KB BIOS ROM */
/* 16 KB floppy / network ROM controllers */

/* RAM mapping: 128 KB flat (including video) */

}


/* ------------ ROMS ------------ */

ROM_START ( mo5nr )
		/* BASIC & BIOS */
	ROM_REGION ( 0x10000, "basic", 0 )
	ROM_LOAD ( "mo5nr.iw01",  0x0000, 0x8000, CRC(ade3c46d) SHA1(64bede6ecb58ad7409b2c546259773af097f162d) )
	ROM_LOAD ( "basicn.iw02", 0x8000, 0x8000, CRC(3a6981c3) SHA1(6d22e3f2ff2a19383401f950c5ace72e1560816c) )

	ROM_REGION ( 0x2000, "nr", 0 ) /* TODO: network ROM */
	ROM_LOAD ( "nr.iw20", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION ( 0x100, "proms", 0 )
	ROM_LOAD ( "an-r.iw21", 0x000, 0x100, NO_DUMP ) // unknown purpose (N82S129AN)

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL ( 0x00000, 0x10000, 0x39 )
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

INPUT_PORTS_END

static INPUT_PORTS_START ( mo5nr )
	PORT_INCLUDE ( thom_lightpen )
	PORT_INCLUDE ( thom_game_port )
	PORT_INCLUDE ( mo5nr_keyboard )
	PORT_INCLUDE ( to7_config )

	PORT_START ( "nanoreseau_config" )
	PORT_DIPNAME(0x01, 0x01, "Extension selection") PORT_DIPLOCATION("SW03:1")
	PORT_DIPSETTING(0x00, "Extension port")
	PORT_DIPSETTING(0x01, "Internal networking")

	PORT_DIPNAME(0x3e, 0x02, "Network ID")          PORT_DIPLOCATION("SW03:2,3,4,5,6")
	PORT_DIPSETTING(0x00, "0 (Master)")
	PORT_DIPSETTING(0x02, "1")
	PORT_DIPSETTING(0x04, "2")
	PORT_DIPSETTING(0x06, "3")
	PORT_DIPSETTING(0x08, "4")
	PORT_DIPSETTING(0x0a, "5")
	PORT_DIPSETTING(0x0c, "6")
	PORT_DIPSETTING(0x0e, "7")
	PORT_DIPSETTING(0x10, "8")
	PORT_DIPSETTING(0x12, "9")
	PORT_DIPSETTING(0x14, "10")
	PORT_DIPSETTING(0x16, "11")
	PORT_DIPSETTING(0x18, "12")
	PORT_DIPSETTING(0x1a, "13")
	PORT_DIPSETTING(0x1c, "14")
	PORT_DIPSETTING(0x1e, "15")
	PORT_DIPSETTING(0x20, "16")
	PORT_DIPSETTING(0x22, "17")
	PORT_DIPSETTING(0x24, "18")
	PORT_DIPSETTING(0x26, "19")
	PORT_DIPSETTING(0x28, "20")
	PORT_DIPSETTING(0x2a, "21")
	PORT_DIPSETTING(0x2c, "22")
	PORT_DIPSETTING(0x2e, "23")
	PORT_DIPSETTING(0x30, "24")
	PORT_DIPSETTING(0x32, "25")
	PORT_DIPSETTING(0x34, "26")
	PORT_DIPSETTING(0x36, "27")
	PORT_DIPSETTING(0x38, "28")
	PORT_DIPSETTING(0x3a, "29")
	PORT_DIPSETTING(0x3c, "30")
	PORT_DIPSETTING(0x3e, "31")
INPUT_PORTS_END


/* ------------ driver ------------ */

void mo5nr_state::mo5nr(machine_config &config)
{
	to7_base(config, true);
	MCFG_MACHINE_START_OVERRIDE( mo5nr_state, mo5nr )
	MCFG_MACHINE_RESET_OVERRIDE( mo5nr_state, mo5nr )

	m_maincpu->set_addrmap(AS_PROGRAM, &mo5nr_state::mo5nr_map);

	m_cassette->set_formats(mo5_cassette_formats);
	m_cassette->set_interface("mo_cass");

	m_pia_sys->readpa_handler().set(FUNC(mo5nr_state::mo6_sys_porta_in));
	m_pia_sys->readpb_handler().set(FUNC(mo5nr_state::mo5nr_sys_portb_in));
	m_pia_sys->writepa_handler().set(FUNC(mo5nr_state::mo5nr_sys_porta_out));
	m_pia_sys->writepb_handler().set("buzzer", FUNC(dac_bit_interface::data_w));
	m_pia_sys->ca2_handler().set(FUNC(mo5nr_state::mo5_set_cassette_motor));
	m_pia_sys->cb2_handler().set(FUNC(mo5nr_state::mo6_sys_cb2_out));
	m_pia_sys->irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<0>)); // differs from TO

	GENERIC_CARTSLOT(config.replace(), "cartslot", generic_plain_slot, "mo_cart", "m5,rom").set_device_load(FUNC(mo5nr_state::mo5_cartridge));

	NANORESEAU_MO(config, m_nanoreseau, 0, true);

	m_extension->option_remove("nanoreseau");

	/* internal ram */
	m_ram->set_default_size("128K");

	TO8_VIDEO(config, m_video, 16_MHz_XTAL);
	m_video->set_is_mo(true);
	m_video->set_screen("screen");
	m_video->set_lightpen_decal(MO6_LIGHTPEN_DECAL);
	m_video->set_lightpen_steps(3);
	m_video->set_vram_page_cb(FUNC(mo5nr_state::get_vram_page));
	downcast<to8_video_device &>(*m_video).set_update_ram_bank_cb(FUNC(mo5nr_state::mo6_update_ram_bank));
	downcast<to8_video_device &>(*m_video).set_update_cart_bank_cb(FUNC(mo5nr_state::mo6_update_cart_bank));
	downcast<to8_video_device &>(*m_video).lightpen_intr_cb().set(m_mainfirq, FUNC(input_merger_device::in_w<2>));
	m_video->int_50hz_cb().set(m_pia_sys, FUNC(pia6821_device::cb1_w));

	config.device_remove("to7_cart_list");
	config.device_remove("to7_cass_list");
	config.device_remove("to_flop_list");
	config.device_remove("to7_qd_list");

	SOFTWARE_LIST(config, "mo6_cass_list").set_original("mo6_cass");
	SOFTWARE_LIST(config, "mo6_flop_list").set_original("mo6_flop");

	SOFTWARE_LIST(config, "mo5_cart_list").set_compatible("mo5_cart");
	SOFTWARE_LIST(config, "mo5_cass_list").set_compatible("mo5_cass");
	SOFTWARE_LIST(config, "mo5_flop_list").set_compatible("mo5_flop");
	SOFTWARE_LIST(config, "mo5_qd_list").set_compatible("mo5_qd");
}

COMP( 1986, mo5nr, 0, 0, mo5nr, mo5nr, mo5nr_state, empty_init, "Thomson", "MO5 NR", 0 )
