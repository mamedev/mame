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
#include "machine/wd_fdc.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "formats/basicdsk.h"
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
	map(0xe800, 0xffff).rom();       /* system bios  */

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
	ROM_REGION ( 0x10000, "maincpu", 0 )
	ROM_LOAD ( "to7.rom", 0xe800, 0x1800,
		CRC(0e7826da)
		SHA1(23a2f84b03c01d385cc1923c8ece95c43756297a) )

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL ( 0x00000, 0x10000, 0x39 )
ROM_END

ROM_START ( t9000 )
	ROM_REGION ( 0x10000, "maincpu", 0 )
	ROM_LOAD ( "t9000.rom", 0xe800, 0x1800,
		CRC(daa8cfbf)
		SHA1(a5735db1ad4e529804fc46603f838d3f4ccaf5cf) )

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
	PORT_INCLUDE ( to7_vconfig )
INPUT_PORTS_END

static INPUT_PORTS_START ( t9000 )
	PORT_INCLUDE ( to7 )
INPUT_PORTS_END

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
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(/*50*/ 1./0.019968);
	m_screen->set_size(THOM_TOTAL_WIDTH * 2, THOM_TOTAL_HEIGHT);
	m_screen->set_visarea(0, THOM_TOTAL_WIDTH * 2 - 1, 0, THOM_TOTAL_HEIGHT - 1);
	m_screen->set_screen_update(FUNC(thomson_state::screen_update_thom));
	m_screen->screen_vblank().set(FUNC(thomson_state::thom_vblank));
	m_screen->set_palette("palette");

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
	map(0xe7e4, 0xe7e7).rw(FUNC(thomson_state::to770_gatearray_r), FUNC(thomson_state::to770_gatearray_w));
	map(0xe800, 0xffff).rom();       /* system bios  */

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
	ROM_REGION ( 0x10000, "maincpu", 0 )
	ROM_LOAD ( "to770.rom", 0xe800, 0x1800, /* BIOS */
		CRC(89518862)
		SHA1(cd34474c0bcc758f6d71c90fbd40cef379d61374) )

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL ( 0x00000, 0x10000, 0x39 )
ROM_END

ROM_START ( to770a )
	ROM_REGION ( 0x10000, "maincpu", 0 )
	ROM_LOAD ( "to770a.rom", 0xe800, 0x1800,
		CRC(378ea808)
		SHA1(f4575b537dfdb46ff2a0e7cbe8dfe4ba63161b8e) )

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
	map(0xa7e4, 0xa7e7).rw(FUNC(mo5_state::mo5_gatearray_r), FUNC(mo5_state::mo5_gatearray_w));
	map(0xb000, 0xefff).bankr(THOM_CART_BANK).w(FUNC(mo5_state::mo5_cartridge_w));
	map(0xf000, 0xffff).rom();       /* system bios */

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
	ROM_REGION ( 0x14000, "maincpu", 0 )
	ROM_LOAD ( "mo5.rom", 0xf000, 0x1000,
		CRC(f0ea9140)
		SHA1(36ce2d3df1866ec2fe368c1c28757e2f5401cf44) )
	ROM_LOAD ( "basic5.rom", 0x11000, 0x3000,
		CRC(c2c11b9d)
		SHA1(512dd40fb45bc2b51a24c84b3723a32bc8e80c06) )

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL( 0x00000, 0x10000, 0x39 )
ROM_END

ROM_START ( mo5e )
	ROM_REGION ( 0x14000, "maincpu", 0 )
	ROM_LOAD ( "mo5e.rom", 0xf000, 0x1000,
		CRC(6520213a)
		SHA1(f17a7a59baf2819ec80991b34b204795536a5e01) )
	ROM_LOAD ( "basic5e.rom", 0x11000, 0x3000,
		CRC(934a72b2)
		SHA1(b37e2b1afbfba368c19be87b3bf61dfe6ad8b0bb) )

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
    . integrated floppy controller, based on WD2793
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
	map(0xe000, 0xe7bf).rom();
	map(0xe7c0, 0xe7c7).rw(m_mc6846, FUNC(mc6846_device::read), FUNC(mc6846_device::write));
	map(0xe7c8, 0xe7cb).rw("pia_0", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xe7cc, 0xe7cf).rw("pia_1", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xe7da, 0xe7dd).rw(FUNC(to9_state::to9_vreg_r), FUNC(to9_state::to9_vreg_w));
	map(0xe7de, 0xe7df).rw(m_to9_kbd, FUNC(to9_keyboard_device::kbd_acia_r), FUNC(to9_keyboard_device::kbd_acia_w));
	map(0xe7e4, 0xe7e7).rw(FUNC(to9_state::to9_gatearray_r), FUNC(to9_state::to9_gatearray_w));
/*  map(0xe7f0, 0xe7f7).rw(FUNC(to9_state::to9_ieee_r), FUNC(to9_state::to9_ieee_w )); */
	map(0xe800, 0xffff).rom();       /* system bios  */

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
	ROM_REGION ( 0x30000, "maincpu", 0 )
	ROM_LOAD ( "to9.rom", 0xe000, 0x2000, /* BIOS & floppy controller */
		CRC(f9278bf7)
		SHA1(9e99e6ae0285950f007b19161de642a4031fe46e) )

		/* BASIC & software */
	ROM_LOAD ( "basic9-0.rom", 0x10000, 0x4000,
		CRC(c7bac620)
		SHA1(4b2a8b30cf437858ce978ba7b0dfa2bbd57eb38a) )
	ROM_LOAD ( "basic9-1.rom", 0x14000, 0x4000,
		CRC(ea5f3e43)
		SHA1(5e58a29c2d117fcdb1f5e7ca31dbfffa0f9218f2) )
	ROM_LOAD ( "basic9-2.rom", 0x18000, 0x4000,
		CRC(0f5581b3)
		SHA1(93815ca78d3532192aaa56cbf65b68b0f10f1b8a) )
	ROM_LOAD ( "basic9-3.rom", 0x1c000, 0x4000,
		CRC(6b5b19e3)
		SHA1(0e832670c185694d9abbcebcc3ad90e94eed585d) )
	ROM_LOAD ( "soft9-0a.rom", 0x20000, 0x4000,
		CRC(8cee157e)
		SHA1(f32fc39b95890c00571e9f3fbcc2d8e0596fc4a1) )
	ROM_LOAD ( "soft9-1a.rom", 0x24000, 0x4000,
		CRC(cf39ac93)
		SHA1(b97e6b7389398e5706624973c11ee7ddba323ce1) )
	ROM_LOAD ( "soft9-0b.rom", 0x28000, 0x4000,
		CRC(033aee3f)
		SHA1(f3604e500329ec0489b05dbab05530322e9463c5) )
	ROM_LOAD ( "soft9-1b.rom", 0x2c000, 0x4000,
		CRC(214fe527)
		SHA1(0d8e3f1ca347026e906c3d00a0371e8238c44a60) )

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL( 0x00000, 0x10000, 0x39 )
ROM_END


/* ------------ inputs   ------------ */

static INPUT_PORTS_START ( to9 )
	PORT_INCLUDE ( thom_lightpen )
	PORT_INCLUDE ( thom_game_port )
	PORT_INCLUDE ( to7_vconfig )

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

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(to9_state::write_centronics_busy));

	/* internal ram */
	m_ram->set_default_size("192K").set_extra_options("128K");
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


void to9_state::to8_map(address_map &map)
{

	map(0x0000, 0x3fff).bankr(THOM_CART_BANK).w(FUNC(to9_state::to8_cartridge_w)); /* 4 * 16 KB */
	map(0x4000, 0x5fff).bankr(THOM_VRAM_BANK).w(FUNC(to9_state::to770_vram_w));
	map(0x6000, 0x7fff).bankr(TO8_SYS_LO).w(FUNC(to9_state::to8_sys_lo_w));
	map(0x8000, 0x9fff).bankr(TO8_SYS_HI).w(FUNC(to9_state::to8_sys_hi_w));
	map(0xa000, 0xbfff).bankr(TO8_DATA_LO).w(FUNC(to9_state::to8_data_lo_w));
	map(0xc000, 0xdfff).bankr(TO8_DATA_HI).w(FUNC(to9_state::to8_data_hi_w));
	map(0xe000, 0xffff).bankr(TO8_BIOS_BANK);
	map(0xe7c0, 0xe7ff).unmaprw();
	map(0xe7c0, 0xe7c7).rw(m_mc6846, FUNC(mc6846_device::read), FUNC(mc6846_device::write));
	map(0xe7c8, 0xe7cb).rw("pia_0", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xe7cc, 0xe7cf).rw("pia_1", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xe7da, 0xe7dd).rw(FUNC(to9_state::to8_vreg_r), FUNC(to9_state::to8_vreg_w));
	map(0xe7e4, 0xe7e7).rw(FUNC(to9_state::to8_gatearray_r), FUNC(to9_state::to8_gatearray_w));
/*  map(0xe7f0, 0xe7f7).rw(FUNC(to9_state::to9_ieee_r), FUNC(to9_state::to9_ieee_w )); */

/* 0x10000 - 0x1ffff: 64 KB external ROM cartridge */
/* 0x20000 - 0x2ffff: 64 KB internal software ROM */
/* 0x30000 - 0x33fff: 16 KB BIOS ROM */
/* 18 KB external floppy / network ROM controllers */

/* RAM mapping: 512 KB flat (including video) */

}


/* ------------ ROMS ------------ */

ROM_START ( to8 )
	ROM_REGION ( 0x24000, "maincpu", 0 )

		/* BIOS & floppy */
	ROM_LOAD ( "to8-0.rom", 0x20000, 0x2000,
		CRC(3c4a640a)
		SHA1(0a4952f0ca002d82ac83755e1f694d56399413b2) )
	ROM_LOAD ( "to8-1.rom", 0x22000, 0x2000,
		CRC(cb9bae2d)
		SHA1(a4a55a6e2c74bca15951158c5164970e922fc1c1) )

		/* BASIC */
	ROM_LOAD ( "basic8-0.rom", 0x10000, 0x4000,
		CRC(e5a00fb3)
		SHA1(281e535ed9b0f76e620253e9103292b8ff623d02) )
	ROM_LOAD ( "basic8-1.rom", 0x14000, 0x4000,
		CRC(4b241e63)
		SHA1(ca8941a10db6cc069bf84c773f5e7d7d2c18449e) )
	ROM_LOAD ( "basic8-2.rom", 0x18000, 0x4000,
		CRC(0f5581b3)
		SHA1(93815ca78d3532192aaa56cbf65b68b0f10f1b8a) )
	ROM_LOAD ( "basic8-3.rom", 0x1c000, 0x4000,
		CRC(f552e7e3)
		SHA1(3208e0d7d90241a327ed24e4921303f16e167bd5) )

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL( 0x00000, 0x10000, 0x39 )
ROM_END

ROM_START ( to8d )
	ROM_REGION ( 0x24000, "maincpu", 0 )

		/* BIOS & floppy */
	ROM_LOAD ( "to8d-0.rom", 0x20000, 0x2000,
		CRC(30ea4950)
		SHA1(6705100cd337fffb26ce999302b55fb71557b128) )
	ROM_LOAD ( "to8d-1.rom", 0x22000, 0x2000,
		CRC(926cf0ca)
		SHA1(8521613ac00e04dd94b69e771aeaefbf4fe97bf7) )

		/* BASIC */
	ROM_LOAD ( "basic8-0.rom", 0x10000, 0x4000,
		CRC(e5a00fb3)
		SHA1(281e535ed9b0f76e620253e9103292b8ff623d02) )
	ROM_LOAD ( "basic8-1.rom", 0x14000, 0x4000,
		CRC(4b241e63)
		SHA1(ca8941a10db6cc069bf84c773f5e7d7d2c18449e) )
	ROM_LOAD ( "basic8-2.rom", 0x18000, 0x4000,
		CRC(0f5581b3)
		SHA1(93815ca78d3532192aaa56cbf65b68b0f10f1b8a) )
	ROM_LOAD ( "basic8-3.rom", 0x1c000, 0x4000,
		CRC(f552e7e3)
		SHA1(3208e0d7d90241a327ed24e4921303f16e167bd5) )

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL( 0x00000, 0x10000, 0x39 )
ROM_END


/* ------------ inputs   ------------ */

static INPUT_PORTS_START ( to8 )
	PORT_INCLUDE ( thom_lightpen )
	PORT_INCLUDE ( thom_game_port )
	PORT_INCLUDE ( to7_config )
	PORT_INCLUDE ( to7_vconfig )
INPUT_PORTS_END


static INPUT_PORTS_START ( to8d )
	PORT_INCLUDE ( to8 )
INPUT_PORTS_END

/* ------------ driver ------------ */

void to9_state::to8(machine_config &config)
{
	to7_base(config, false);
	MCFG_MACHINE_START_OVERRIDE( to9_state, to8 )
	MCFG_MACHINE_RESET_OVERRIDE( to9_state, to8 )

	m_maincpu->set_addrmap(AS_PROGRAM, &to9_state::to8_map);

	TO8_KEYBOARD(config, m_to8_kbd);
	m_to8_kbd->data_cb().set(m_mc6846, FUNC(mc6846_device::set_input_cp1));

	m_pia_sys->readpa_handler().set(FUNC(to9_state::to8_sys_porta_in));
	m_pia_sys->readpb_handler().set_constant(0);
	m_pia_sys->writepa_handler().set(FUNC(to9_state::to9_sys_porta_out));
	m_pia_sys->writepb_handler().set(FUNC(to9_state::to8_sys_portb_out));
	m_pia_sys->cb2_handler().set_nop();
	m_pia_sys->irqa_handler().set_nop();

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(to9_state::write_centronics_busy));

	m_mc6846->out_port().set(FUNC(to9_state::to8_timer_port_out));
	m_mc6846->in_port().set(FUNC(to9_state::to8_timer_port_in));
	m_mc6846->cp2().set(FUNC(to9_state::to8_timer_cp2_out));

	/* internal ram */
	m_ram->set_default_size("512K").set_extra_options("256K");

	SOFTWARE_LIST(config, "to8_cass_list").set_original("to8_cass");
	SOFTWARE_LIST(config, "to8_qd_list").set_original("to8_qd");
	SOFTWARE_LIST(config.replace(), "to7_cass_list").set_compatible("to7_cass");
	SOFTWARE_LIST(config.replace(), "to7_qd_list").set_compatible("to7_qd");
}

void to9_state::to8d(machine_config &config)
{
	to8(config);
}


COMP( 1986, to8, 0, 0, to8, to8, to9_state, empty_init, "Thomson", "TO8", 0 )

COMP( 1987, to8d, to8, 0, to8d, to8d, to9_state, empty_init, "Thomson", "TO8D", 0 )


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

void to9_state::to9p_map(address_map &map)
{

	map(0x0000, 0x3fff).bankr(THOM_CART_BANK).w(FUNC(to9_state::to8_cartridge_w)); /* 4 * 16 KB */
	map(0x4000, 0x5fff).bankr(THOM_VRAM_BANK).w(FUNC(to9_state::to770_vram_w));
	map(0x6000, 0x7fff).bankr(TO8_SYS_LO).w(FUNC(to9_state::to8_sys_lo_w));
	map(0x8000, 0x9fff).bankr(TO8_SYS_HI).w(FUNC(to9_state::to8_sys_hi_w));
	map(0xa000, 0xbfff).bankr(TO8_DATA_LO).w(FUNC(to9_state::to8_data_lo_w));
	map(0xc000, 0xdfff).bankr(TO8_DATA_HI).w(FUNC(to9_state::to8_data_hi_w));
	map(0xe000, 0xffff).bankr(TO8_BIOS_BANK);
	map(0xe7c0, 0xe7ff).unmaprw();
	map(0xe7c0, 0xe7c7).rw(m_mc6846, FUNC(mc6846_device::read), FUNC(mc6846_device::write));
	map(0xe7c8, 0xe7cb).rw("pia_0", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xe7cc, 0xe7cf).rw("pia_1", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xe7da, 0xe7dd).rw(FUNC(to9_state::to8_vreg_r), FUNC(to9_state::to8_vreg_w));
	map(0xe7de, 0xe7df).rw(m_to9_kbd, FUNC(to9_keyboard_device::kbd_acia_r), FUNC(to9_keyboard_device::kbd_acia_w));
	map(0xe7e4, 0xe7e7).rw(FUNC(to9_state::to8_gatearray_r), FUNC(to9_state::to8_gatearray_w));
/*  map(0xe7f0, 0xe7f7).rw(FUNC(to9_state::to9_ieee_r), FUNC(to9_state::to9_ieee_w )); */

/* 0x10000 - 0x1ffff: 64 KB external ROM cartridge */
/* 0x20000 - 0x2ffff: 64 KB internal software ROM */
/* 0x30000 - 0x33fff: 16 KB BIOS ROM */
/* 18 KB external floppy / network ROM controllers */

/* RAM mapping: 512 KB flat (including video) */

}


/* ------------ ROMS ------------ */

ROM_START ( to9p )
	ROM_REGION ( 0x24000, "maincpu", 0 )

		/* BIOS & floppy */
	ROM_LOAD ( "to9p-0.rom", 0x20000, 0x2000,
		CRC(a2731296)
		SHA1(b30e06127d6e99d4ac5a5bb67881df27bbd9a7e5) )
	ROM_LOAD ( "to9p-1.rom", 0x22000, 0x2000,
		CRC(c52ce315)
		SHA1(7eacbd796e76bc72b872f9700c9b90414899ea0f) )

		/* BASIC */
	ROM_LOAD ( "basicp-0.rom", 0x10000, 0x4000,
		CRC(e5a00fb3)
		SHA1(281e535ed9b0f76e620253e9103292b8ff623d02) )
	ROM_LOAD ( "basicp-1.rom", 0x14000, 0x4000,
		CRC(4b241e63)
		SHA1(ca8941a10db6cc069bf84c773f5e7d7d2c18449e) )
	ROM_LOAD ( "basicp-2.rom", 0x18000, 0x4000,
		CRC(0f5581b3)
		SHA1(93815ca78d3532192aaa56cbf65b68b0f10f1b8a) )
	ROM_LOAD ( "basicp-3.rom", 0x1c000, 0x4000,
		CRC(ebe9c8d9)
		SHA1(b667ad09a1181f65059a2cbb4c95421bc544a334) )

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL( 0x00000, 0x10000, 0x39 )
ROM_END


/* ------------ inputs   ------------ */

static INPUT_PORTS_START ( to9p )
	PORT_INCLUDE ( thom_lightpen )
	PORT_INCLUDE ( thom_game_port )
	PORT_INCLUDE ( to7_config )
	PORT_INCLUDE ( to7_vconfig )
INPUT_PORTS_END

/* ------------ driver ------------ */

void to9_state::to9p(machine_config &config)
{
	to7_base(config, false);
	MCFG_MACHINE_START_OVERRIDE( to9_state, to9p )
	MCFG_MACHINE_RESET_OVERRIDE( to9_state, to9p )

	m_maincpu->set_addrmap(AS_PROGRAM, &to9_state::to9p_map);

	TO9P_KEYBOARD(config, m_to9_kbd);
	m_to9_kbd->irq_cb().set(m_mainirq, FUNC(input_merger_device::in_w<4>));

	m_pia_sys->readpa_handler().set(FUNC(to9_state::to9_sys_porta_in));
	m_pia_sys->readpb_handler().set_constant(0);
	m_pia_sys->writepa_handler().set(FUNC(to9_state::to9_sys_porta_out));
	m_pia_sys->writepb_handler().set(FUNC(to9_state::to8_sys_portb_out));
	m_pia_sys->cb2_handler().set_nop();
	m_pia_sys->irqa_handler().set_nop();
	m_pia_sys->irqb_handler().set("mainfirq", FUNC(input_merger_device::in_w<1>));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(to9_state::write_centronics_busy));

	m_mc6846->out_port().set(FUNC(to9_state::to9p_timer_port_out));
	m_mc6846->in_port().set(FUNC(to9_state::to9p_timer_port_in));
	m_mc6846->cp2().set(FUNC(to9_state::to8_timer_cp2_out));

	/* internal ram */
	m_ram->set_default_size("512K");

	SOFTWARE_LIST(config, "to8_cass_list").set_original("to8_cass");
	SOFTWARE_LIST(config, "to8_qd_list").set_original("to8_qd");
	SOFTWARE_LIST(config.replace(), "to7_cass_list").set_compatible("to7_cass");
	SOFTWARE_LIST(config.replace(), "to7_qd_list").set_compatible("to7_qd");
}

COMP( 1986, to9p, 0, 0, to9p, to9p, to9_state, empty_init, "Thomson", "TO9+", 0 )



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
	map(0xa7e4, 0xa7e7).rw(FUNC(mo6_state::mo6_gatearray_r), FUNC(mo6_state::mo6_gatearray_w));
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
	ROM_REGION ( 0x20000, "maincpu", 0 )

		/* BIOS */
	ROM_LOAD ( "mo6-0.rom", 0x13000, 0x1000,
		CRC(0446eef6)
		SHA1(b57fcda69c95f0c97c5cb0605d17c49a0c630300) )
	ROM_LOAD ( "mo6-1.rom", 0x17000, 0x1000,
		CRC(eb6df8d4)
		SHA1(24e2232f582ce04f260acd8e9ec710468a81505c) )

		/* BASIC */
	ROM_LOAD ( "basic6-0.rom", 0x10000, 0x3000,
		CRC(18789833)
		SHA1(fccbf69cbc6deba45a767a26cd6454cf0eedfc2b) )
	ROM_LOAD ( "basic6-1.rom", 0x14000, 0x3000,
		CRC(c9b4d6f4)
		SHA1(47487d2bc4c9a9c09c733bd89c49693c52e262de) )
	ROM_LOAD ( "basic6-2.rom", 0x18000, 0x4000,
		CRC(08eac9bb)
		SHA1(c0231fdb3bcccbbb10c1f93cc529fc3b96dd3f4d) )
	ROM_LOAD ( "basic6-3.rom", 0x1c000, 0x4000,
		CRC(19d66dc4)
		SHA1(301b6366269181b74cb5d7ccdf5455b7290ae99b) )

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL ( 0x00000, 0x10000, 0x39 )
ROM_END

ROM_START ( pro128 )
	ROM_REGION ( 0x20000, "maincpu", 0 )

		/* BIOS */
	ROM_LOAD ( "pro128-0.rom", 0x13000, 0x1000,
		CRC(a8aef291)
		SHA1(2685cca841f405a37ef48b0115f90c865ce79d0f) )
	ROM_LOAD ( "pro128-1.rom", 0x17000, 0x1000,
		CRC(5b3340ec)
		SHA1(269f2eb3e3452014b8d1f0f9e1c63fe56375a863) )

		/* BASIC */
	ROM_LOAD ( "basico-0.rom", 0x10000, 0x3000,
		CRC(98b10d5e)
		SHA1(d6b77e694fa85e1114293448e5a64f6e2cf46c22) )
	ROM_LOAD ( "basico-1.rom", 0x14000, 0x3000,
		CRC(721d2124)
		SHA1(51db1cd03b3891e212a24aa6563b09968930d897) )
	ROM_LOAD ( "basico-2.rom", 0x18000, 0x4000,
		CRC(135438ab)
		SHA1(617d4e4979842bea2c21ef7f8c50f3b08b15239a) )
	ROM_LOAD ( "basico-3.rom", 0x1c000, 0x4000,
		CRC(2c2befa6)
		SHA1(3e94e182bacbb55bb07be2af4c76c0b0df47b3bf) )

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
	PORT_INCLUDE ( to7_vconfig )
INPUT_PORTS_END

static INPUT_PORTS_START ( pro128 )
	PORT_INCLUDE ( thom_lightpen )
	PORT_INCLUDE ( thom_game_port )
	PORT_INCLUDE ( pro128_keyboard )
	PORT_INCLUDE ( to7_config )
	PORT_INCLUDE ( to7_vconfig )
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
	map(0xa7e4, 0xa7e7).rw(FUNC(mo5nr_state::mo6_gatearray_r), FUNC(mo5nr_state::mo6_gatearray_w));
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
	ROM_REGION ( 0x20000, "maincpu", 0 )

		/* BIOS */
	ROM_LOAD ( "mo5nr-0.rom", 0x13000, 0x1000,
		CRC(06e31115)
		SHA1(7429cc0c15475398b5ab514cb3d3efdc71cf082f) )
	ROM_LOAD ( "mo5nr-1.rom", 0x17000, 0x1000,
		CRC(7cda17c9)
		SHA1(2ff6480ce9e30acc4c89b6113d7c8ea6095d90a5) )

		/* BASIC */
	ROM_LOAD ( "basicn-0.rom", 0x10000, 0x3000,
		CRC(fae9e691)
		SHA1(62fbfd6d4ca837f6cb8ed37f828eca97f80e6200) )
	ROM_LOAD ( "basicn-1.rom", 0x14000, 0x3000,
		CRC(cf134dd7)
		SHA1(1bd961314e16e460d37a65f5e7f4acf5604fbb17) )
	ROM_LOAD ( "basicn-2.rom", 0x18000, 0x4000,
		CRC(b69d2e0d)
		SHA1(ea3220bbae991e08259d38a7ea24533b2bb86418) )
	ROM_LOAD ( "basicn-3.rom", 0x1c000, 0x4000,
		CRC(7785610f)
		SHA1(c38b0be404d8af6f409a1b52cb79a4e10fc33177) )

	ROM_REGION ( 0x10000, "cartridge", 0 )
	ROM_FILL ( 0x00000, 0x10000, 0x39 ) /* TODO: network ROM */
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
	PORT_INCLUDE ( to7_vconfig )

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
