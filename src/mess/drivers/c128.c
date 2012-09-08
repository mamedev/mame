/***************************************************************************
    commodore c128 home computer

    PeT mess@utanet.at

    documentation:
     iDOC (http://www.softwolves.pp.se/idoc)
           Christian Janoff  mepk@c64.org
***************************************************************************/

/*

2008 - Driver Updates
---------------------

(most of the informations are taken from http://www.zimmers.net/cbmpics/ )


[CBM systems which belong to this driver]

* Commodore 128 (1985)

CPU: CSG 8502 (1 or 2 MHz), Z80 (~3 MHz)
RAM: 128 kilobytes
ROM: 72 kilobytes expandable
Video: MOS 8564 "VIC-IIE", MOS 8563 "VDC" CTRC (40/80 columns text; Palette of 16
    colors; Hires modes 320 x 200, 640 x 200, 16k of dedicated VDC RAM)
Sound: MOS 8580 "SID" (3 voice stereo synthesizer/digital sound
    capabilities)
Ports: MOS 6526 CIA x2 (2 Joystick/Mouse ports; CBM Serial port; CBM
    Datasette port; parallel programmable "User" port; CBM Monitor port;
    C64 expansion port; Warm reset switch; Keyboard port; Power switch)
Keyboard: Full-sized 93 key QWERTY (14 key numeric keypad; 8 programmable
    function keys + HELP; 4 direction 4-key cursor-pad)

Upgrade kits were sold to upgrade the VDC RAM to 64k using a passthrough board
that the VDC sat in.


* Commodore 128CR (prototype from June, 1986)

  Basically, a C128 in a redesigned board to reduce production costs. It's
not clear when it's been produced, nor if it has ever been produced on
large scale. Its BIOS is an intermediate revision between rev. 0 and rev. 1
in the main C128.
A picture of the PCB can be found here:
http://www.zimmers.net/anonftp/pub/cbm/schematics/computers/c128/c128cr.jpg

Appears to be functionally identical to the original c128, has 16k VDC ram, and a
?prototype? VDC labeled 2568R1X. Has 6526A-1 CIAs, 8721R3 and 8722R2 gate
arrays, 8580R5 sid, and 8502R0 processor.


* Commodore 128D (1985)

  Designed in the US, but only sold in Europe, it is a C128 in a desktop
case, with built-in 1571 disk drive (upgraded with a special software to
discourage pirating software). Some NTSC prototypes exist.

CPU: CSG 8502 (1 or 2 MHz), Z80 (~3 MHz), 6502 (co-processor for disk
    drive)
RAM: 128 kilobytes
ROM: 72 kilobytes expandable
Video: MOS 8564 "VIC-IIE", MOS 8563 "VDC" CTRC (40/80 columns text; Palette of 16
    colors; Hires modes 320 x 200, 640 x 200, 64k of dedicated VDC RAM)
Sound: MOS 8580 "SID" (3 voice stereo synthesizer/digital sound
    capabilities)
Ports: MOS 6526 CIA x2 (2 Joystick/Mouse ports; CBM Serial port; CBM
    Datasette port; parallel programmable "User" port; CBM Monitor port;
    C64 expansion port; Warm reset switch; Keyboard port; Power switch)
Keyboard: Full-sized 93 key QWERTY (14 key numeric keypad; 8 programmable
    function keys + HELP; 4 direction 4-key cursor-pad)
Additional Hardware: Internal 1571 disk drive (Double sided/Double Density
    360k; capable of reading GCR and MFM formats)


* Commodore 128DCR (1986)

  Basically, a C128D in a redesigned board to reduce production costs. It's
the only model sold in the US, but it's quite possible that it came later
in Europe as well (being cheaper to produce).


* Commodore "128D/81" (198?)

  NTSC prototype for an improved version of C128D, featuring a built-in 1581
disk drive in place of the 1571 used in C128D / C128DCR. The prototype has no
given name, so C128D/81 is just a reasonable way to indicate it. The case is
from a PAL C128D and the board is a heavily modified PAL board with hand
soldered connections to make it NTSC.


[TO DO]

* C64 Mode

  See [TO DO] in drivers/c64.c for the missing features.


* C/PM Mode

  It should work if you put the CP/M disk in drive 8 and enter BOOT. Better
disk emulation would be of help, anyway.

* C128 Mode

  Various missing features (e.g. no cpu clock doubling; no internal function
rom; serial bus doesn't support printer or other devices; no C128 cart
expansions are supported; no userport; no rs232/v.24 interface)

* Informations / BIOS / Supported Sets:

- Was C128D using rev. 1 BIOS in 4 ROMs? I guessed so because the board has the
same desing as a C128, and later C128DCR still used rev. 1 BIOS (only contained
in two ROMs)

- Is it possible to track down and dump properly C128 PAL BIOSes? Current sets
are mostly tagged as bad dumps because obtained by extracting the content in
pieces. I'd like to have confirmation that the common parts are really the same
before removing the flag.

- PAL BIOSes are from C128? C128D? or C128DCR? Were there differences in the
contents between them, except for being splitted in 2 or 4 parts? Were all
versions sold in each country? Right now we choose to support the following sets
(more to be added if BIOS content confirmed):

+ German, Italian and Swedish dumps are known to come from a C128DCR. Therefore,
we support c128drde, c128drit and c128drsw.

+ We also have a dump of the German C128, therefore we support the c128ger, even
if it's not clear which BASIC version it was shipped with. We assumed the older
kernal to be shipped with rev. 0 and the newer with rev. 1.

+ The Finnish, French and Norwegian dumps came with no notes (or these have been
lost). Therefore we support only the c128 for these, i.e. c128fin, c128fre and
c128nor.

+ Character ROM for Belgium, Italy and French was the same (I/F/B on the label,
and indeed it turned out to be the same on both the Italian and French C128)

- Also, the italian C128DCR was found with a rev. 0 BASIC on it. Were both rev. 0
and rev. 1 used in the CR version? When did Commodore switch between the two?

[Notes about dumping BIOS]

Dumping roms with eeprom reader
-------------------------------

c128 / c128d

    U18       (read compatible 2764?) 8kB c64 character rom, c128 character rom
    U32 23128 (read compatible 27128?) 16kB c64 Basic, c64 Kernel
    U33 23128 (read compatible 27128?) 16kB c128 Basic at 0x4000
    U34 23128 (read compatible 27128?) 16kB c128 Basic at 0x8000
    U35 23128 (read compatible 27128?) 16kB c128 Editor, Z80BIOS, c128 Kernel

c128cr / c128dcr

    U18       (read compatible 2764?) 8kB c64 character rom, c128 character rom
    U32 23256 (read compatible 27256?) 32kB c64 Basic + Kernel, c128 Editor, Z80BIOS, c128 Kernel
    U34 23256 (read compatible 27256?) 32kB c128 Basic

c128d / c128dcr also need:

    U102 23256 (read compatible 27256?) 32kB 1571 system rom


It would be also possible to dump the BIOS in monitor, but it would be preferable
to use an EEPROM reader, in order to obtain a dump of the whole content.
*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6502/m6502.h"
#include "sound/sid6581.h"
#include "sound/dac.h"
#include "machine/6526cia.h"

#include "machine/cbmipt.h"
#include "video/mos6566.h"
#include "video/mc6845.h"


/* devices config */
#include "includes/cbm.h"
#include "formats/cbm_snqk.h"
#include "machine/cbmiec.h"

#include "includes/c128.h"
#include "includes/c64_legacy.h"


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/


/* shares ram with m8502
 * how to bankswitch ?
 * bank 0
 * 0x0000, 0x03ff bios rom
 * 0x1400, 0x1bff vdc videoram
 * 0x1c00, 0x23ff vdc colorram
 * 0x2c00, 0x2fff vic2e videoram
 * 0xff00, 0xff04 mmu registers
 * else ram (dram bank 0?)
 * bank 1
 * 0x0000-0xedff ram (dram bank 1?)
 * 0xe000-0xffff ram as bank 0
 */
//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( z80_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( z80_mem, AS_PROGRAM, 8, c128_state )
	AM_RANGE(0x0000, 0x0fff) AM_READ_BANK("bank10") AM_WRITE(write_0000)
	AM_RANGE(0x1000, 0xbfff) AM_READ_BANK("bank11") AM_WRITE(write_1000)
	AM_RANGE(0xc000, 0xffff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( z80_io )
//-------------------------------------------------

static ADDRESS_MAP_START( z80_io, AS_IO, 8, c128_state )
	AM_RANGE(0x1000, 0x13ff) AM_READWRITE_LEGACY(c64_colorram_read, c64_colorram_write)
	AM_RANGE(0xd000, 0xd3ff) AM_DEVREADWRITE(MOS8564_TAG, mos6566_device, read, write)
	AM_RANGE(0xd400, 0xd4ff) AM_DEVREADWRITE_LEGACY(MOS6581_TAG, sid6581_r, sid6581_w)
	AM_RANGE(0xd500, 0xd5ff) AM_READWRITE(mmu8722_port_r, mmu8722_port_w)
	//AM_RANGE(0xd600, 0xd7ff) AM_DEVREADWRITE_LEGACY(MOS8563_TAG, vdc8563_port_r, vdc8563_port_w)
	AM_RANGE(0xd600, 0xd600) AM_MIRROR(0x1fe) AM_DEVREADWRITE(MOS8563_TAG, mos8563_device, status_r, address_w)
	AM_RANGE(0xd601, 0xd601) AM_MIRROR(0x1fe) AM_DEVREADWRITE(MOS8563_TAG, mos8563_device, register_r, register_w)
	AM_RANGE(0xdc00, 0xdcff) AM_DEVREADWRITE_LEGACY(MOS6526_1_TAG, mos6526_r, mos6526_w)
	AM_RANGE(0xdd00, 0xddff) AM_DEVREADWRITE_LEGACY(MOS6526_2_TAG, mos6526_r, mos6526_w)
/*  AM_RANGE(0xdf00, 0xdfff) AM_READWRITE_LEGACY(dma_port_r, dma_port_w) */
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( m8502_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( m8502_mem, AS_PROGRAM, 8, c128_state )
	AM_RANGE(0x0000, 0x00ff) AM_RAMBANK("bank1")
	AM_RANGE(0x0100, 0x01ff) AM_RAMBANK("bank2")
	AM_RANGE(0x0200, 0x03ff) AM_RAMBANK("bank3")
	AM_RANGE(0x0400, 0x0fff) AM_RAMBANK("bank4")
	AM_RANGE(0x1000, 0x1fff) AM_RAMBANK("bank5")
	AM_RANGE(0x2000, 0x3fff) AM_RAMBANK("bank6")

	AM_RANGE(0x4000, 0x7fff) AM_READ_BANK( "bank7") AM_WRITE(write_4000 )
	AM_RANGE(0x8000, 0x9fff) AM_READ_BANK( "bank8") AM_WRITE(write_8000 )
	AM_RANGE(0xa000, 0xbfff) AM_READ_BANK( "bank9") AM_WRITE(write_a000 )

	AM_RANGE(0xc000, 0xcfff) AM_READ_BANK( "bank12") AM_WRITE(write_c000 )
	AM_RANGE(0xd000, 0xdfff) AM_READ_BANK( "bank13") AM_WRITE(write_d000)
	AM_RANGE(0xe000, 0xfeff) AM_READ_BANK( "bank14") AM_WRITE(write_e000 )
	AM_RANGE(0xff00, 0xff04) AM_READ_BANK( "bank15") AM_WRITE(write_ff00 )	   /* mmu c128 modus */
	AM_RANGE(0xff05, 0xffff) AM_READ_BANK( "bank16") AM_WRITE(write_ff05 )
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vic_videoram_map )
//-------------------------------------------------

static ADDRESS_MAP_START( vic_videoram_map, AS_0, 8, c128_state )
	AM_RANGE(0x0000, 0x3fff) AM_READ(vic_dma_read)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vic_colorram_map )
//-------------------------------------------------

static ADDRESS_MAP_START( vic_colorram_map, AS_1, 8, c128_state )
	AM_RANGE(0x000, 0x3ff) AM_READ(vic_dma_read_color)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vdc_videoram_map )
//-------------------------------------------------

static ADDRESS_MAP_START( vdc_videoram_map, AS_0, 8, c128_state )
	AM_RANGE(0x0000, 0xffff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( c128 )
//-------------------------------------------------

static INPUT_PORTS_START( c128 )
	PORT_INCLUDE( common_cbm_keyboard )		/* ROW0 -> ROW7 */

	PORT_START( "KP0" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD)				PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD) 			PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD) 			PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD)				PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5)				PORT_CHAR('\t')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD) 			PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD) 			PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Help") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(PGUP))

	PORT_START( "KP1" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD) 			PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD) 			PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD) 			PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER_PAD) 		PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Line Feed") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD)			PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) 		PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)				PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START( "KP2" )
	PORT_CONFNAME( 0x80, 0x00, "No Scroll (switch)") PORT_CODE(KEYCODE_F9)
	PORT_CONFSETTING(	0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(	0x80, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) 			PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT)				PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN)				PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP)				PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD)			PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD) 			PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Alt") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(LALT))

	PORT_INCLUDE( c128_special )			/* SPECIAL */

	PORT_INCLUDE( c64_controls )			/* CTRLSEL, JOY0, JOY1, PADDLE0 -> PADDLE3, TRACKX, TRACKY, LIGHTX, LIGHTY, OTHER */
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c128ger )
//-------------------------------------------------

static INPUT_PORTS_START( c128ger )
	PORT_INCLUDE( c128 )

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Z  { Y }") PORT_CODE(KEYCODE_Z)					PORT_CHAR('Z')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3  #  { 3  Paragraph }") PORT_CODE(KEYCODE_3)		PORT_CHAR('3') PORT_CHAR('#')

	PORT_MODIFY( "ROW3" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Y  { Z }") PORT_CODE(KEYCODE_Y)					PORT_CHAR('Y')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7  '  { 7  / }") PORT_CODE(KEYCODE_7)				PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY( "ROW4" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0  { = }") PORT_CODE(KEYCODE_0)					PORT_CHAR('0')

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(",  <  { ; }") PORT_CODE(KEYCODE_COMMA)			PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Paragraph  \xE2\x86\x91  { \xc3\xbc }") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00A7) PORT_CHAR(0x2191)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(":  [  { \xc3\xa4 }") PORT_CODE(KEYCODE_COLON)		PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(".  >  { : }") PORT_CODE(KEYCODE_STOP)				PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("-  { '  ` }") PORT_CODE(KEYCODE_EQUALS)			PORT_CHAR('-')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("+  { \xc3\x9f ? }") PORT_CODE(KEYCODE_MINUS)		PORT_CHAR('+')

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/  ?  { -  _ }") PORT_CODE(KEYCODE_SLASH)					PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Sum  Pi  { ] \\ }") PORT_CODE(KEYCODE_DEL)				PORT_CHAR(0x03A3) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("=  { # ' }") PORT_CODE(KEYCODE_BACKSLASH)					PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(";  ]  { \xc3\xb6 }") PORT_CODE(KEYCODE_QUOTE)				PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("*  `  { +  * }") PORT_CODE(KEYCODE_CLOSEBRACE)			PORT_CHAR('*') PORT_CHAR('`')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\\  { [  \xE2\x86\x91 }") PORT_CODE(KEYCODE_BACKSLASH2)	PORT_CHAR('\\')

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("_  { <  > }") PORT_CODE(KEYCODE_TILDE)					PORT_CHAR('_')

	PORT_MODIFY( "SPECIAL" )
	PORT_CONFNAME( 0x20, 0x00, "ASCII DIN (switch)" )
	PORT_CONFSETTING(	0x00, "ASCII" )
	PORT_CONFSETTING(	0x20, "DIN" )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c128fra )
//-------------------------------------------------

static INPUT_PORTS_START( c128fra )
	PORT_INCLUDE( c128 )

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Z  { W }") PORT_CODE(KEYCODE_Z)				PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4  $  { '  4 }") PORT_CODE(KEYCODE_4)			PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("A  { Q }") PORT_CODE(KEYCODE_A)				PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("W  { Z }") PORT_CODE(KEYCODE_W)				PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3  #  { \"  3 }") PORT_CODE(KEYCODE_3)		PORT_CHAR('3') PORT_CHAR('#')

	PORT_MODIFY( "ROW2" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6  &  { Paragraph  6 }") PORT_CODE(KEYCODE_6)	PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5  %  { (  5 }") PORT_CODE(KEYCODE_5)			PORT_CHAR('5') PORT_CHAR('%')

	PORT_MODIFY( "ROW3" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8  (  { !  8 }") PORT_CODE(KEYCODE_8)			PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7  '  { \xc3\xa8  7 }") PORT_CODE(KEYCODE_7)	PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY( "ROW4" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("K  Large-  { \\ }") PORT_CODE(KEYCODE_K)		PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("M  Large-/  { ,  ? }") PORT_CODE(KEYCODE_M)	PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0  { \xc3\xa0  0 }") PORT_CODE(KEYCODE_0)		PORT_CHAR('0')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9  )  { \xc3\xa7  9 }") PORT_CODE(KEYCODE_9)	PORT_CHAR('9') PORT_CHAR(')')

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(",  <  { ;  . }") PORT_CODE(KEYCODE_COMMA)						PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("@  \xc3\xbb  { ^  \xc2\xa8 }") PORT_CODE(KEYCODE_OPENBRACE)	PORT_CHAR('@') PORT_CHAR(0x00FB)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(":  [  { \xc3\xb9  % }") PORT_CODE(KEYCODE_COLON)				PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(".  >  { :  / }") PORT_CODE(KEYCODE_STOP)						PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("-  \xc2\xb0  { -  _ }") PORT_CODE(KEYCODE_EQUALS)				PORT_CHAR('-') PORT_CHAR('\xB0')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("+  \xc3\xab  { )  \xc2\xb0 }") PORT_CODE(KEYCODE_MINUS)		PORT_CHAR('+') PORT_CHAR(0x00EB)

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/  ?  { =  + }") PORT_CODE(KEYCODE_SLASH)						PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi  { *  ] }") PORT_CODE(KEYCODE_DEL)			PORT_CHAR(0x2191) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("=  {\xE2\x86\x91  \\ }") PORT_CODE(KEYCODE_BACKSLASH)			PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(";  ]  { M  Large-/ }") PORT_CODE(KEYCODE_QUOTE)				PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("*  `  { $  [ }") PORT_CODE(KEYCODE_CLOSEBRACE)				PORT_CHAR('*') PORT_CHAR('`')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\\  { @  # }") PORT_CODE(KEYCODE_BACKSLASH)					PORT_CHAR('\\')

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Q  { A }") PORT_CODE(KEYCODE_Q)				PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2  \"  { \xc3\xa9  2 }") PORT_CODE(KEYCODE_2)	PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("_   { <  > }") PORT_CODE(KEYCODE_TILDE)		PORT_CHAR('_')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1  !  { &  1 }") PORT_CODE(KEYCODE_1)			PORT_CHAR('1') PORT_CHAR('!')

	PORT_MODIFY( "SPECIAL" )
	PORT_CONFNAME( 0x20, 0x00, "ASCII ?French? (switch)" )
	PORT_CONFSETTING(	0x00, "ASCII" )
	PORT_CONFSETTING(	0x20, "?French?" )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c128ita )
//-------------------------------------------------

static INPUT_PORTS_START( c128ita )
	PORT_INCLUDE( c128 )

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Z  { W }") PORT_CODE(KEYCODE_Z)						PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4  $  { '  4 }") PORT_CODE(KEYCODE_4)					PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("W  { Z }") PORT_CODE(KEYCODE_W)						PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3  #  { \"  3 }") PORT_CODE(KEYCODE_3)				PORT_CHAR('3') PORT_CHAR('#')

	PORT_MODIFY( "ROW2" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6  &  { _  6 }") PORT_CODE(KEYCODE_6)					PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5  %  { (  5 }") PORT_CODE(KEYCODE_5)					PORT_CHAR('5') PORT_CHAR('%')

	PORT_MODIFY( "ROW3" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8  (  { &  8 }") PORT_CODE(KEYCODE_8)					PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7  '  { \xc3\xa8  7 }") PORT_CODE(KEYCODE_7)			PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY( "ROW4" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("M  Large-/  { ,  ? }") PORT_CODE(KEYCODE_M)			PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0  { \xc3\xa0  0 }") PORT_CODE(KEYCODE_0)				PORT_CHAR('0')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9  )  { \xc3\xa7  9 }") PORT_CODE(KEYCODE_9)			PORT_CHAR('9') PORT_CHAR(')')

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(",  <   { ;  . }") PORT_CODE(KEYCODE_COMMA)			PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("@  \xc3\xbb  { \xc3\xac  = }") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR(0x00FB)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(":  [  { \xc3\xb9  % }") PORT_CODE(KEYCODE_COLON)		PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(".  >  { :  / }") PORT_CODE(KEYCODE_STOP)				PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("-  \xc2\xb0  { -  + }") PORT_CODE(KEYCODE_EQUALS)		PORT_CHAR('-') PORT_CHAR('\xb0')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("+  \xc3\xab  { )  \xc2\xb0 }") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('+') PORT_CHAR(0x00EB)

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/  ?  { \xc3\xb2  ! }") PORT_CODE(KEYCODE_SLASH)		PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi  { *  ] }") PORT_CODE(KEYCODE_DEL)	PORT_CHAR(0x2191) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("=  { \xE2\x86\x91  \\ }") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(";  ]  { M }") PORT_CODE(KEYCODE_QUOTE)				PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("*  `  { $  [ }") PORT_CODE(KEYCODE_CLOSEBRACE)		PORT_CHAR('*') PORT_CHAR('`')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\\  { @  # }") PORT_CODE(KEYCODE_BACKSLASH2)			PORT_CHAR('\\')

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2  \"  { \xc3\xa9  2 }") PORT_CODE(KEYCODE_2)			PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("_  { <  > }") PORT_CODE(KEYCODE_TILDE)				PORT_CHAR('_')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1  !  { \xc2\xa3  1 }") PORT_CODE(KEYCODE_1)			PORT_CHAR('1') PORT_CHAR('!')

	PORT_MODIFY( "SPECIAL" )
	PORT_CONFNAME( 0x20, 0x00, "ASCII Italian (switch)" )
	PORT_CONFSETTING( 0x00, "ASCII" )
	PORT_CONFSETTING( 0x20, DEF_STR( Italian ) )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c128swe )
//-------------------------------------------------

static INPUT_PORTS_START( c128swe )
	PORT_INCLUDE( c128 )

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3  #  { 3  Paragraph }") PORT_CODE(KEYCODE_3)		PORT_CHAR('3') PORT_CHAR('#')

	PORT_MODIFY( "ROW3" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7  '  { 7  / }") PORT_CODE(KEYCODE_7)				PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("]  { \xc3\xa2 }") PORT_CODE(KEYCODE_OPENBRACE)	PORT_CHAR(']')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("[  { \xc3\xa4 }") PORT_CODE(KEYCODE_COLON)		PORT_CHAR('[')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)									PORT_CHAR('=')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)										PORT_CHAR('-')

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(";  +") PORT_CODE(KEYCODE_BACKSLASH)				PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xc2\xa3  { \xc3\xb6 }") PORT_CODE(KEYCODE_QUOTE)	PORT_CHAR('\xA3')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("@") PORT_CODE(KEYCODE_CLOSEBRACE)					PORT_CHAR('@')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(":  *") PORT_CODE(KEYCODE_BACKSLASH2)				PORT_CHAR(':') PORT_CHAR('*')

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("_  { <  > }") PORT_CODE(KEYCODE_TILDE)			PORT_CHAR('_')

	PORT_MODIFY( "SPECIAL" )
	PORT_CONFNAME( 0x20, 0x00, "ASCII Swedish/Finnish (switch)" )
	PORT_CONFSETTING( 0x00, "ASCII" )
	PORT_CONFSETTING( 0x20, "Swedish/Finnish" )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MOS8564_INTERFACE( vic_intf )
//-------------------------------------------------

READ8_MEMBER( c128_state::vic_lightpen_x_cb )
{
	return ioport("LIGHTX")->read() & ~0x01;
}

READ8_MEMBER( c128_state::vic_lightpen_y_cb )
{
	return ioport("LIGHTY")->read() & ~0x01;
}

READ8_MEMBER( c128_state::vic_lightpen_button_cb )
{
	return ioport("OTHER")->read() & 0x04;
}

READ8_MEMBER( c128_state::vic_rdy_cb )
{
	return ioport("CTRLSEL")->read() & 0x08;
}

static MOS8564_INTERFACE( vic_intf )
{
	SCREEN_VIC_TAG,
	Z80A_TAG,
	DEVCB_DRIVER_LINE_MEMBER(c128_state, vic_interrupt),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(c128_state, vic_lightpen_x_cb),
	DEVCB_DRIVER_MEMBER(c128_state, vic_lightpen_y_cb),
	DEVCB_DRIVER_MEMBER(c128_state, vic_lightpen_button_cb),
	DEVCB_DRIVER_MEMBER(c128_state, vic_rdy_cb)
};


//-------------------------------------------------
//  mc6845_interface vdc_intf
//-------------------------------------------------

static MC6845_UPDATE_ROW( vdc_update_row )
{
    mos8563_device *mos8563 = static_cast<mos8563_device *>(device);

    mos8563->update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, param);
}

static const mc6845_interface vdc_intf =
{
	SCREEN_VDC_TAG,
	8,
	NULL,
	vdc_update_row,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};


//-------------------------------------------------
//  sid6581_interface sid_intf
//-------------------------------------------------

READ8_MEMBER( c128_state::sid_potx_r )
{
	return c64_paddle_read(m_sid, 0);
}

READ8_MEMBER( c128_state::sid_poty_r )
{
	return c64_paddle_read(m_sid, 1);
}

static const sid6581_interface sid_intf =
{
	DEVCB_DRIVER_MEMBER(c128_state, sid_potx_r),
	DEVCB_DRIVER_MEMBER(c128_state, sid_poty_r)
};


//-------------------------------------------------
//  M6510_INTERFACE( cpu_intf )
//-------------------------------------------------

static M6510_INTERFACE( cpu_intf )
{
	DEVCB_NULL,					/* read_indexed_func */
	DEVCB_NULL,					/* write_indexed_func */
	DEVCB_DRIVER_MEMBER(c128_state, cpu_r),	/* port_read_func */
	DEVCB_DRIVER_MEMBER(c128_state, cpu_w),	/* port_write_func */
	0x07,
	0x20
};

//-------------------------------------------------
//  CBM_IEC_INTERFACE( cbm_iec_intf )
//-------------------------------------------------

static CBM_IEC_INTERFACE( cbm_iec_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(c128_state, iec_srq_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(c128_state, iec_data_w),
	DEVCB_NULL
};


//-------------------------------------------------
//  PET_DATASSETTE_PORT_INTERFACE( datassette_intf )
//-------------------------------------------------

static PET_DATASSETTE_PORT_INTERFACE( datassette_intf )
{
	DEVCB_DEVICE_LINE(MOS6526_1_TAG, mos6526_flag_w)
};



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( ntsc )
//-------------------------------------------------

static MACHINE_CONFIG_START( ntsc, c128_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80A_TAG, Z80, VIC6567_CLOCK)
	MCFG_CPU_PROGRAM_MAP( z80_mem)
	MCFG_CPU_IO_MAP( z80_io)
	MCFG_CPU_VBLANK_INT(SCREEN_VIC_TAG, c128_frame_interrupt)
	//MCFG_CPU_PERIODIC_INT(vic2_raster_irq, VIC6567_HRETRACERATE)

	MCFG_CPU_ADD(M8502_TAG, M8502, VIC6567_CLOCK)
	MCFG_CPU_PROGRAM_MAP( m8502_mem)
	MCFG_CPU_CONFIG( cpu_intf )
	MCFG_CPU_VBLANK_INT(SCREEN_VIC_TAG, c128_frame_interrupt)
	// MCFG_CPU_PERIODIC_INT(vic2_raster_irq, VIC6567_HRETRACERATE)

	/* video hardware */
	MCFG_MOS8564_ADD(MOS8564_TAG, SCREEN_VIC_TAG, VIC6567_CLOCK, vic_intf, vic_videoram_map, vic_colorram_map)
	MCFG_MOS8563_ADD(MOS8563_TAG, SCREEN_VDC_TAG, 2000000, vdc_intf, vdc_videoram_map)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6581_TAG, SID6581, VIC6567_CLOCK)
	MCFG_SOUND_CONFIG(sid_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", cbm_c64, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)

	/* cassette */
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, datassette_intf, cbm_datassette_devices, "c1530", NULL)

	/* cia */
	MCFG_MOS6526R1_ADD(MOS6526_1_TAG, VIC6567_CLOCK, c128_ntsc_cia0)
	MCFG_MOS6526R1_ADD(MOS6526_2_TAG, VIC6567_CLOCK, c128_ntsc_cia1)

	MCFG_FRAGMENT_ADD(c64_cartslot)
	MCFG_SOFTWARE_LIST_ADD("c64_disk_list", "c64_flop")
	MCFG_SOFTWARE_LIST_ADD("c128_disk_list", "c128_flop")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( c128 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( c128, ntsc )
	MCFG_CBM_IEC_ADD(cbm_iec_intf, "c1571")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( c128d )
//-------------------------------------------------
static MACHINE_CONFIG_DERIVED( c128d, ntsc )
	MCFG_CBM_IEC_ADD(cbm_iec_intf, "c1571")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( c128dcr )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( c128dcr, ntsc )
	MCFG_CBM_IEC_BUS_ADD(cbm_iec_intf)
	MCFG_CBM_IEC_SLOT_ADD("iec4", 4, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec8", 8, c128dcr_iec_devices, "c1571cr", NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec9", 9, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec10", 10, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec11", 11, cbm_iec_devices, NULL, NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( c128d81 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( c128d81, ntsc )
	MCFG_CBM_IEC_BUS_ADD(cbm_iec_intf)
	MCFG_CBM_IEC_SLOT_ADD("iec4", 4, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec8", 8, c128d81_iec_devices, "c1563", NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec9", 9, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec10", 10, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec11", 11, cbm_iec_devices, NULL, NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( ntsc )
//-------------------------------------------------

static MACHINE_CONFIG_START( pal, c128_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80A_TAG, Z80, VIC6569_CLOCK)
	MCFG_CPU_PROGRAM_MAP( z80_mem)
	MCFG_CPU_IO_MAP( z80_io)
	MCFG_CPU_VBLANK_INT(SCREEN_VIC_TAG, c128_frame_interrupt)
	//MCFG_CPU_PERIODIC_INT(vic2_raster_irq, VIC6569_HRETRACERATE)

	MCFG_CPU_ADD(M8502_TAG, M8502, VIC6569_CLOCK)
	MCFG_CPU_PROGRAM_MAP( m8502_mem)
	MCFG_CPU_CONFIG( cpu_intf )
	MCFG_CPU_VBLANK_INT(SCREEN_VIC_TAG, c128_frame_interrupt)
	// MCFG_CPU_PERIODIC_INT(vic2_raster_irq, VIC6569_HRETRACERATE)

	/* video hardware */
	MCFG_MOS8566_ADD(MOS8566_TAG, SCREEN_VIC_TAG, VIC6569_CLOCK, vic_intf, vic_videoram_map, vic_colorram_map)
	MCFG_MOS8563_ADD(MOS8563_TAG, SCREEN_VDC_TAG, 2000000, vdc_intf, vdc_videoram_map)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6581_TAG, SID6581, VIC6569_CLOCK)
	MCFG_SOUND_CONFIG(sid_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", cbm_c64, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)

	/* cassette */
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, datassette_intf, cbm_datassette_devices, "c1530", NULL)

	/* cia */
	MCFG_MOS6526R1_ADD(MOS6526_1_TAG, VIC6569_CLOCK, c128_ntsc_cia0)
	MCFG_MOS6526R1_ADD(MOS6526_2_TAG, VIC6569_CLOCK, c128_ntsc_cia1)

	MCFG_FRAGMENT_ADD(c64_cartslot)
	MCFG_SOFTWARE_LIST_ADD("c64_disk_list", "c64_flop")
	MCFG_SOFTWARE_LIST_ADD("c128_disk_list", "c128_flop")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( c128pal )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( c128pal, pal )
	MCFG_CBM_IEC_ADD(cbm_iec_intf, "c1571")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( c128dpal )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( c128dpal, pal )
	MCFG_CBM_IEC_ADD(cbm_iec_intf, "c1571")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( c128dcrp )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( c128dcrp, pal )
	MCFG_CBM_IEC_BUS_ADD(cbm_iec_intf)
	MCFG_CBM_IEC_SLOT_ADD("iec4", 4, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec8", 8, c128dcr_iec_devices, "c1571cr", NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec9", 9, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec10", 10, cbm_iec_devices, NULL, NULL)
	MCFG_CBM_IEC_SLOT_ADD("iec11", 11, cbm_iec_devices, NULL, NULL)
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( c128 )
//-------------------------------------------------

ROM_START( c128 )
	ROM_REGION( 0x132800, Z80A_TAG, 0 )
	ROM_DEFAULT_BIOS("r1")
	ROM_SYSTEM_BIOS( 0, "r0", "rev. 0" )
	ROMX_LOAD( "318018-02.bin", 0x100000, 0x4000, CRC(2ee6e2fa) SHA1(60e1491e1d5782e3cf109f518eb73427609badc6), ROM_BIOS(1) )			// BASIC lo
	ROMX_LOAD( "318019-02.bin", 0x104000, 0x4000, CRC(d551fce0) SHA1(4d223883e866645328f86a904b221464682edc4f), ROM_BIOS(1) )			// BASIC hi
	ROMX_LOAD( "318020-03.bin", 0x10c000, 0x4000, CRC(1e94bb02) SHA1(e80ffbafae068cc0e42698ec5c5c39af46ac612a), ROM_BIOS(1) )			// Kernal
	ROM_SYSTEM_BIOS( 1, "r1", "rev. 1" )
	ROMX_LOAD( "318018-04.bin", 0x100000, 0x4000, CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441), ROM_BIOS(2) )			// BASIC lo
	ROMX_LOAD( "318019-04.bin", 0x104000, 0x4000, CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0), ROM_BIOS(2) )			// BASIC hi
	ROMX_LOAD( "318020-05.bin", 0x10c000, 0x4000, CRC(ba456b8e) SHA1(ceb6e1a1bf7e08eb9cbc651afa29e26adccf38ab), ROM_BIOS(2) )			// Kernal
	ROM_SYSTEM_BIOS( 2, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "318018-04.bin", 0x100000, 0x4000, CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441), ROM_BIOS(3) )			// BASIC lo
	ROMX_LOAD( "318019-04.bin", 0x104000, 0x4000, CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0), ROM_BIOS(3) )			// BASIC hi
	ROMX_LOAD( "jiffydos c128.bin", 0x10c000, 0x4000, CRC(4b7964de) SHA1(7d1898f32beae4b2ae610d469ce578a588efaa7c), ROM_BIOS(3) )			// Kernal

	ROM_LOAD( "251913-01.bin", 0x108000, 0x4000, CRC(0010ec31) SHA1(765372a0e16cbb0adf23a07b80f6b682b39fbf88) )		// C64 OS ROM
	ROM_LOAD( "390059-01.bin", 0x120000, 0x2000, CRC(6aaaafe6) SHA1(29ed066d513f2d5c09ff26d9166ba23c2afb2b3f) )		// Character

	ROM_REGION( 0x10000, M8502_TAG, ROMREGION_ERASEFF )
ROM_END


//-------------------------------------------------
//  ROM( c128cr )
//-------------------------------------------------

ROM_START( c128cr )
	/* C128CR prototype, owned by Bo Zimmers
       PCB markings: "COMMODORE 128CR REV.3 // PCB NO.252270" and "PCB ASSY NO.250783"
           Sticker on rom cart shield: "C128CR  No.2 // ENG. SAMPLE // Jun/9/'86   KNT"
       3 ROMs (combined basic, combined c64/kernal, plain character rom)
       6526A-1 CIAs
       ?prototype? 2568R1X VDC w/ 1186 datecode
    */
	ROM_REGION( 0x132800, Z80A_TAG, 0 )
	ROM_LOAD( "252343-03.u34", 0x100000, 0x8000, CRC(bc07ed87) SHA1(0eec437994a3f2212343a712847213a8a39f4a7b) )			// BASIC lo + hi, "252343-03 // U34"
	ROM_LOAD( "252343-04.u32", 0x108000, 0x8000, CRC(cc6bdb69) SHA1(36286b2e8bea79f7767639fd85e12c5447c7041b) )			// C64 OS ROM + Kernal, "252343-04 // US // U32"
	ROM_LOAD( "390059-01.u18", 0x120000, 0x2000, CRC(6aaaafe6) SHA1(29ed066d513f2d5c09ff26d9166ba23c2afb2b3f) )			// Character, "MOS // (C)1985 CBM // 390059-01 // M468613 8547H"

	ROM_REGION( 0x10000, M8502_TAG, ROMREGION_ERASEFF )
ROM_END


//-------------------------------------------------
//  ROM( c128ger )
//-------------------------------------------------

ROM_START( c128ger )
	ROM_REGION( 0x132800, Z80A_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "default", "rev. 1" )
	ROMX_LOAD( "318018-04.bin", 0x100000, 0x4000, CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441), ROM_BIOS(1) )			// BASIC lo
	ROMX_LOAD( "318019-04.bin", 0x104000, 0x4000, CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0), ROM_BIOS(1) )			// BASIC hi
	ROMX_LOAD( "315078-02.bin", 0x10c000, 0x4000, CRC(b275bb2e) SHA1(78ac5dcdd840b092ba1ee6d19b33af079613291f), ROM_BIOS(1) )			// Kernal
	ROM_SYSTEM_BIOS( 1, "rev0", "rev. 0" )
	ROMX_LOAD( "318018-02.bin", 0x100000, 0x4000, CRC(2ee6e2fa) SHA1(60e1491e1d5782e3cf109f518eb73427609badc6), ROM_BIOS(2) )			// BASIC lo
	ROMX_LOAD( "318019-02.bin", 0x104000, 0x4000, CRC(d551fce0) SHA1(4d223883e866645328f86a904b221464682edc4f), ROM_BIOS(2) )			// BASIC hi
	ROMX_LOAD( "315078-01.bin", 0x10c000, 0x4000, CRC(a51e2168) SHA1(bcf82a89a8fc5d086bec2ff3bcbdecc8af2be3af), ROM_BIOS(2) )			// Kernal

	ROM_LOAD( "251913-01.bin", 0x108000, 0x4000, CRC(0010ec31) SHA1(765372a0e16cbb0adf23a07b80f6b682b39fbf88) )		// C64 OS ROM
	ROM_LOAD( "390059-01.bin", 0x120000, 0x2000, CRC(6aaaafe6) SHA1(29ed066d513f2d5c09ff26d9166ba23c2afb2b3f) )		// Character

	ROM_REGION( 0x10000, M8502_TAG, ROMREGION_ERASEFF )
ROM_END


//-------------------------------------------------
//  ROM( c128sfi )
//-------------------------------------------------

ROM_START( c128sfi )
	ROM_REGION( 0x132800, Z80A_TAG, 0 )
	ROM_LOAD( "318018-02.u33", 0x100000, 0x4000, CRC(2ee6e2fa) SHA1(60e1491e1d5782e3cf109f518eb73427609badc6) )
	ROM_LOAD( "318019-02.u34", 0x104000, 0x4000, CRC(d551fce0) SHA1(4d223883e866645328f86a904b221464682edc4f) )
	ROM_LOAD( "325182-01.u32", 0x108000, 0x4000, CRC(2aff27d3) SHA1(267654823c4fdf2167050f41faa118218d2569ce) ) // C128 64 Sw/Fi
	ROM_LOAD( "325189-01.u35", 0x10c000, 0x4000, CRC(9526fac4) SHA1(a01dd871241c801db51e8ebc30fedfafd8cc506b) ) // C128 Ker Sw/Fi

	/* This was not included in the submission, unfortunately */
	ROM_LOAD( "325181-02.u18", 0x120000, 0x2000, BAD_DUMP CRC(7a70d9b8) SHA1(aca3f7321ee7e6152f1f0afad646ae41964de4fb) ) // C128 Char Sw/Fi

	ROM_REGION( 0x10000, M8502_TAG, ROMREGION_ERASEFF )
ROM_END


//-------------------------------------------------
//  ROM( c128fra )
//-------------------------------------------------

ROM_START( c128fra )
	ROM_REGION( 0x132800, Z80A_TAG, 0 )
	ROM_LOAD( "318018-04.bin", 0x100000, 0x4000, BAD_DUMP CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441) )			// BASIC lo
	ROM_LOAD( "318019-04.bin", 0x104000, 0x4000, BAD_DUMP CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0) )			// BASIC hi
	ROM_LOAD( "251913-01.bin", 0x108000, 0x4000, BAD_DUMP CRC(0010ec31) SHA1(765372a0e16cbb0adf23a07b80f6b682b39fbf88) )			// C64 OS ROM
	ROM_LOAD( "editor.french.bin", 0x10c000, 0x1000, BAD_DUMP CRC(3e086a24) SHA1(0a2f67455166f8dac101f8f8564a1c0364cb456a) )
	ROM_LOAD( "z80bios.bin", 0x10d000, 0x1000, BAD_DUMP CRC(c38d83c6) SHA1(38662a024f1de2f4417a5f9df4898a9985503e06) )
	ROM_LOAD( "kernalpart.french.bin", 0x10e000, 0x2000, BAD_DUMP CRC(ca5e1179) SHA1(d234a031caf59a0f66871f2babe1644783e66cf7) )
	ROM_LOAD( "325167-01.bin", 0x120000, 0x2000, BAD_DUMP CRC(bad36b88) SHA1(9119b27a1bf885fa4c76fff5d858c74c194dd2b8) )

	ROM_REGION( 0x10000, M8502_TAG, ROMREGION_ERASEFF )
ROM_END


//-------------------------------------------------
//  ROM( c128nor )
//-------------------------------------------------

ROM_START( c128nor )
	ROM_REGION( 0x132800, Z80A_TAG, 0 )
	ROM_LOAD( "318018-04.bin", 0x100000, 0x4000, BAD_DUMP CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441) )			// BASIC lo
	ROM_LOAD( "318019-04.bin", 0x104000, 0x4000, BAD_DUMP CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0) )			// BASIC hi
	ROM_LOAD( "251913-01.bin", 0x108000, 0x4000, BAD_DUMP CRC(0010ec31) SHA1(765372a0e16cbb0adf23a07b80f6b682b39fbf88) )			// C64 OS ROM
	ROM_LOAD( "editor.norwegian.bin", 0x10c000, 0x1000, BAD_DUMP CRC(84c55911) SHA1(22f6c5f40d4e895ea51e8432b30c219eacb75778) )
	ROM_LOAD( "z80bios.bin", 0x10d000, 0x1000, BAD_DUMP CRC(c38d83c6) SHA1(38662a024f1de2f4417a5f9df4898a9985503e06) )
	ROM_LOAD( "kernalpart.norwegian.bin", 0x10e000, 0x2000, BAD_DUMP CRC(3ba48012) SHA1(21a90256a3572a890f8027a085d851bf818b0dda) )
	/* standard vic20 based norwegian */
	ROM_LOAD( "char.nor", 0x120000, 0x2000, BAD_DUMP CRC(ba95c625) SHA1(5a87faa457979e7b6f434251a9e32f4483b337b3) )

	ROM_REGION( 0x10000, M8502_TAG, ROMREGION_ERASEFF )
ROM_END


//-------------------------------------------------
//  ROM( c128d )
//-------------------------------------------------

/* C128D Board is basically the same as C128 + a second board for the disk drive */
ROM_START( c128d )
	ROM_REGION( 0x132800, Z80A_TAG, 0 )
	ROM_LOAD( "318018-04.bin", 0x100000, 0x4000, CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441) )			// BASIC lo
	ROM_LOAD( "318019-04.bin", 0x104000, 0x4000, CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0) )			// BASIC hi
	ROM_LOAD( "251913-01.bin", 0x108000, 0x4000, CRC(0010ec31) SHA1(765372a0e16cbb0adf23a07b80f6b682b39fbf88) )			// C64 OS ROM
	ROM_LOAD( "318020-05.bin", 0x10c000, 0x4000, CRC(ba456b8e) SHA1(ceb6e1a1bf7e08eb9cbc651afa29e26adccf38ab) )			// Kernal
	ROM_LOAD( "390059-01.bin", 0x120000, 0x2000, CRC(6aaaafe6) SHA1(29ed066d513f2d5c09ff26d9166ba23c2afb2b3f) )

	ROM_REGION( 0x10000, M8502_TAG, ROMREGION_ERASEFF )
ROM_END


//-------------------------------------------------
//  ROM( c128dpr )
//-------------------------------------------------

#define rom_c128dpr		rom_c128d


//-------------------------------------------------
//  ROM( c128dcr )
//-------------------------------------------------

/* This BIOS is exactly the same as C128 rev. 1, but on two ROMs only */
ROM_START( c128dcr )
	ROM_REGION( 0x132800, Z80A_TAG, 0 )
	ROM_LOAD( "318022-02.bin", 0x100000, 0x8000, CRC(af1ae1e8) SHA1(953dcdf5784a6b39ef84dd6fd968c7a03d8d6816) )			// BASIC lo + hi
	ROM_LOAD( "318023-02.bin", 0x108000, 0x8000, CRC(eedc120a) SHA1(f98c5a986b532c78bb68df9ec6dbcf876913b99f) )			// C64 OS ROM + Kernal
	ROM_LOAD( "390059-01.bin", 0x120000, 0x2000, CRC(6aaaafe6) SHA1(29ed066d513f2d5c09ff26d9166ba23c2afb2b3f) )			// Character

	ROM_REGION( 0x10000, M8502_TAG, ROMREGION_ERASEFF )
ROM_END


//-------------------------------------------------
//  ROM( c128drde )
//-------------------------------------------------

ROM_START( c128drde )
	ROM_REGION( 0x132800, Z80A_TAG, 0 )
	ROM_LOAD( "318022-02.bin", 0x100000, 0x8000, CRC(af1ae1e8) SHA1(953dcdf5784a6b39ef84dd6fd968c7a03d8d6816) )			// BASIC lo + hi
	ROM_LOAD( "318077-01.bin", 0x108000, 0x8000, CRC(eb6e2c8f) SHA1(6b3d891fedabb5335f388a5d2a71378472ea60f4) )			// C64 OS ROM + Kernal Ger
	ROM_LOAD( "315079-01.bin", 0x120000, 0x2000, CRC(fe5a2db1) SHA1(638f8aff51c2ac4f99a55b12c4f8c985ef4bebd3) )

	ROM_REGION( 0x10000, M8502_TAG, ROMREGION_ERASEFF )
ROM_END


//-------------------------------------------------
//  ROM( c128drsw )
//-------------------------------------------------

ROM_START( c128drsw )
	ROM_REGION( 0x132800, Z80A_TAG, 0 )
	ROM_LOAD( "318022-02.bin", 0x100000, 0x8000, CRC(af1ae1e8) SHA1(953dcdf5784a6b39ef84dd6fd968c7a03d8d6816) )			// BASIC lo + hi
	ROM_LOAD( "318034-01.bin", 0x108000, 0x8000, CRC(cb4e1719) SHA1(9b0a0cef56d00035c611e07170f051ee5e63aa3a) )			// C64 OS ROM + Kernal Sw/Fi
	ROM_LOAD( "325181-01.bin", 0x120000, 0x2000, CRC(7a70d9b8) SHA1(aca3f7321ee7e6152f1f0afad646ae41964de4fb) )

	ROM_REGION( 0x10000, M8502_TAG, ROMREGION_ERASEFF )
ROM_END


//-------------------------------------------------
//  ROM( c128drit )
//-------------------------------------------------

ROM_START( c128drit )
	ROM_REGION( 0x132800, Z80A_TAG, 0 )
	ROM_LOAD( "318022-01.bin", 0x100000, 0x8000, CRC(e857df90) SHA1(5c2d7bbda2c3f9a926bd76ad19dc0c8c733c41cd) )			// BASIC lo + hi - based on BASIC rev.0
	ROM_LOAD( "251913-01.bin", 0x108000, 0x4000, BAD_DUMP CRC(0010ec31) SHA1(765372a0e16cbb0adf23a07b80f6b682b39fbf88) )			// C64 OS ROM
	ROM_LOAD( "editor.italian.bin", 0x10c000, 0x1000, BAD_DUMP CRC(8df58148) SHA1(39add4c0adda7a64f68a09ae8742599091228017) )
	ROM_LOAD( "z80bios.bin", 0x10d000, 0x1000, BAD_DUMP CRC(c38d83c6) SHA1(38662a024f1de2f4417a5f9df4898a9985503e06) )
	ROM_LOAD( "kernalpart.italian.bin", 0x10e000, 0x2000, BAD_DUMP CRC(7b0d2140) SHA1(f5d604d89daedb47a1abe4b0aa41ea762829e71e) )
	ROM_LOAD( "325167-01.bin", 0x120000, 0x2000, BAD_DUMP CRC(bad36b88) SHA1(9119b27a1bf885fa4c76fff5d858c74c194dd2b8) )

	ROM_REGION( 0x10000, M8502_TAG, ROMREGION_ERASEFF )
ROM_END


//-------------------------------------------------
//  ROM( c128d81 )
//-------------------------------------------------

ROM_START( c128d81 )
	ROM_REGION( 0x132800, Z80A_TAG, 0 )
	ROM_LOAD( "318018-04.bin", 0x100000, 0x4000, CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441) )			// BASIC lo
	ROM_LOAD( "318019-04.bin", 0x104000, 0x4000, CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0) )			// BASIC hi
	ROM_LOAD( "251913-01.bin", 0x108000, 0x4000, CRC(0010ec31) SHA1(765372a0e16cbb0adf23a07b80f6b682b39fbf88) )			// C64 OS ROM
	ROM_LOAD( "318020-05.bin", 0x10c000, 0x4000, CRC(ba456b8e) SHA1(ceb6e1a1bf7e08eb9cbc651afa29e26adccf38ab) )			// Kernal
	ROM_LOAD( "390059-01.bin", 0x120000, 0x2000, CRC(6aaaafe6) SHA1(29ed066d513f2d5c09ff26d9166ba23c2afb2b3f) )

	ROM_REGION( 0x10000, M8502_TAG, ROMREGION_ERASEFF )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE     INPUT   INIT                        COMPANY                        FULLNAME                                     FLAGS
COMP( 1985, c128,      0,     0,   c128,     c128, c128_state,    c128,     "Commodore Business Machines", "Commodore 128 (NTSC)", 0)
COMP( 1985, c128cr,    c128,  0,   c128,     c128, c128_state,    c128,     "Commodore Business Machines", "Commodore 128CR (NTSC, prototype?)", 0)

COMP( 1985, c128sfi,   c128,  0,   c128pal,  c128swe, c128_state, c128pal,  "Commodore Business Machines", "Commodore 128 (Sweden/Finland)", 0)
COMP( 1985, c128fra,   c128,  0,   c128pal,  c128fra, c128_state, c128pal,  "Commodore Business Machines", "Commodore 128 (France)", 0)
COMP( 1985, c128ger,   c128,  0,   c128pal,  c128ger, c128_state, c128pal,  "Commodore Business Machines", "Commodore 128 (Germany)", 0)
COMP( 1985, c128nor,   c128,  0,   c128pal,  c128ita, c128_state, c128pal,  "Commodore Business Machines", "Commodore 128 (Norway)", 0)
// we miss other countries: Spain, Belgium, etc.

// the following drivers use a 1571 floppy drive
COMP( 1985, c128dpr,   c128,  0,   c128d,    c128, c128_state,    c128d,   "Commodore Business Machines", "Commodore 128D (NTSC, prototype)", GAME_NOT_WORKING )
COMP( 1985, c128d,     c128,  0,   c128dpal, c128, c128_state,    c128dpal,"Commodore Business Machines", "Commodore 128D (PAL)", GAME_NOT_WORKING )

// the following drivers use a 1571CR floppy drive
COMP( 1986, c128dcr,   c128,  0,   c128dcr,  c128, c128_state,    c128dcr, "Commodore Business Machines", "Commodore 128DCR (NTSC)", GAME_NOT_WORKING)
COMP( 1986, c128drde,  c128,  0,   c128dcrp, c128ger, c128_state, c128dcrp,"Commodore Business Machines", "Commodore 128DCR (Germany)", GAME_NOT_WORKING)
COMP( 1986, c128drit,  c128,  0,   c128dcrp, c128ita, c128_state, c128dcrp,"Commodore Business Machines", "Commodore 128DCR (Italy)", GAME_NOT_WORKING)
COMP( 1986, c128drsw,  c128,  0,   c128dcrp, c128swe, c128_state, c128dcrp,"Commodore Business Machines", "Commodore 128DCR (Sweden/Finland)", GAME_NOT_WORKING)

// the following driver is a c128 with 1581 floppy drive. it allows us to document 1581 firmware dumps, but it does not do much more
COMP( 1986, c128d81,   c128,  0,   c128d81,  c128, c128_state,    c128d81, "Commodore Business Machines", "Commodore 128D/81 (NTSC, prototype)", GAME_NOT_WORKING)
