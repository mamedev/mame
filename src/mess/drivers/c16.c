/***************************************************************************
    commodore c16 home computer

    PeT mess@utanet.at

    documentation
     www.funet.fi

***************************************************************************/

/*

2008 - Driver Updates
---------------------

(most of the informations are taken from http://www.zimmers.net/cbmpics/ )


[CBM systems which belong to this driver]

* Commodore 116 (1984, Europe only)

  Entry level computer of the Commodore 264 family, it was marketed only
in Europe. It was impressive for the small size of its case, but it didn't
meet commercial success

CPU: MOS Technology 7501 (variable clock rate, with max 1.76 MHz)
RAM: 16 kilobytes (expandable to 64k internally)
ROM: 32 kilobytes
Video: MOS Technology 7360 "TED" (5 Video modes; Max. Resolution 320x200;
    40 columns text; Palette of 16 colors in 8 shades, for 128 colors)
Sound: MOS Technology 7360 "TED" (2 voice tone-generating sound capabilities)
Ports: MOS 7360 (2 Joystick/Mouse ports; CBM Serial port; 'TED' port; "TV"
    Port and switch; CBM Monitor port; Power and reset switches; Power
    connector)
Keyboard: QWERTY 62 key "membrane" (8 programmable function keys; 4 direction
    cursor-pad)


* Commodore 16 (1984)

  Redesigned version of the C116, with a different case and a different
keyboard.

CPU: MOS Technology 7501 (variable clock rate, with max 1.76 MHz)
RAM: 16 kilobytes (expandable to 64k internally)
ROM: 32 kilobytes
Video: MOS Technology 7360 "TED" (5 Video modes; Max. Resolution 320x200;
    40 columns text; Palette of 16 colors in 8 shades, for 128 colors)
Sound: MOS Technology 7360 "TED" (2 voice tone-generating sound capabilities)
Ports: MOS 7360 (2 Joystick/Mouse ports; CBM Serial port; 'TED' port; "TV"
    Port and switch; CBM Monitor port; Power and reset switches; Power
    connector)
Keyboard: QWERTY 66 key typewriter style (8 programmable function keys;
    4 direction cursor-pad)


* Commodore Plus/4 (1984)

  This system became the middle tier of the Commodore 264 family, replacing
the original Commodore 264. The Plus/4 is basically the same as the C264,
but the name refers to the four built-in programs which came with the
machine: Word Processing, Spreadsheet, Database software, Graphing package.

CPU: MOS Technology 7501 (variable clock rate, with max 1.76 MHz)
RAM: 64 kilobytes (expandable to 64k internally)
ROM: 64 kilobytes
Video: MOS Technology 7360 "TED" (5 Video modes; Max. Resolution 320x200;
    40 columns text; Palette of 16 colors in 8 shades, for 128 colors)
Sound: MOS Technology 7360 "TED" (2 voice tone-generating sound capabilities)
Ports: MOS 7360 (2 Joystick/Mouse ports; CBM Serial port; 'TED' port; "TV"
    Port and switch; CBM Monitor port; Power and reset switches; Power
    connector)
Keyboard: Full-sized QWERTY 67 key (8 programmable function keys;
    4 direction cursor-pad)


* Commodore 232 (1984, Prototype)

  This system never reached the production and only few units exist. It is
in between the C16 and the C264, with its 32 kilobytes of RAM.


* Commodore 264 (1984, Prototype)

  Basically the same of a Plus/4 but without the built-in programs.


* Commodore V364 (1984, Prototype)

  This system was supposed to become the high-end system of the family,
featuring 64 kilobytes of RAM, the same technology of the Plus/4, a
keyboard with numeric keypad and built in voice synthesis capabilities.

[TO DO]

* Supported Systems:

- Once we can add / remove devices, we shall only support c16, c116 and plus/4,
removing the separated drivers for different floppy drives

* Other Peripherals:

- Lightpen support is unfinished
- Missing support for (it might or might not be added eventually):
printers and other devices; most expansion modules; userport; rs232/v.24 interface.

* System Specific

- V364 lacks speech hardware emulation

*/


#include "emu.h"
#include "audio/ted7360.h"
#include "audio/t6721.h"
#include "cpu/m6502/m6502.h"
#include "machine/ram.h"
#include "formats/cbm_snqk.h"
#include "includes/cbm.h"
#include "includes/c16.h"
#include "machine/c1551.h"
#include "machine/cbmiec.h"
#include "machine/cbmipt.h"
#include "sound/sid6581.h"
#include "machine/plus4exp.h"
#include "machine/plus4user.h"


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/


/*
 * commodore c16/c116/plus 4
 * 16 KByte (C16/C116) or 32 KByte or 64 KByte (plus4) RAM
 * 32 KByte Rom (C16/C116) 64 KByte Rom (plus4)
 * availability to append additional 64 KByte Rom
 *
 * ports 0xfd00 till 0xff3f are always read/writeable for the cpu
 * for the video interface chip it seams to read from
 * ram or from rom in this  area
 *
 * writes go always to ram
 * only 16 KByte Ram mapped to 0x4000,0x8000,0xc000
 * only 32 KByte Ram mapped to 0x8000
 *
 * rom bank at 0x8000: 16K Byte(low bank)
 * first: basic
 * second(plus 4 only): plus4 rom low
 * third: expansion slot
 * fourth: expansion slot
 * rom bank at 0xc000: 16K Byte(high bank)
 * first: kernal
 * second(plus 4 only): plus4 rom high
 * third: expansion slot
 * fourth: expansion slot
 * writes to 0xfddx select rom banks:
 * address line 0 and 1: rom bank low
 * address line 2 and 3: rom bank high
 *
 * writes to 0xff3e switches to roms (0x8000 till 0xfd00, 0xff40 till 0xffff)
 * writes to 0xff3f switches to rams
 *
 * at 0xfc00 till 0xfcff is ram or rom kernal readable
 */

static ADDRESS_MAP_START(c16_map, AS_PROGRAM, 8, c16_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAMBANK("bank9")
	AM_RANGE(0x4000, 0x7fff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank5")	   /* only ram memory configuration */
	AM_RANGE(0x8000, 0xbfff) AM_READ_BANK("bank2") AM_WRITE_BANK("bank6")
	AM_RANGE(0xc000, 0xfbff) AM_READ_BANK("bank3")
	AM_RANGE(0xfc00, 0xfcff) AM_READ_BANK("bank4")
	AM_RANGE(0xc000, 0xfcff) AM_WRITE_BANK("bank7")
	AM_RANGE(0xfd10, 0xfd1f) AM_READ_LEGACY(c16_fd1x_r)
	AM_RANGE(0xfd30, 0xfd3f) AM_READWRITE_LEGACY(c16_6529_port_r, c16_6529_port_w)		/* 6529 keyboard matrix */
	AM_RANGE(0xfdd0, 0xfddf) AM_WRITE_LEGACY(c16_select_roms) /* rom chips selection */
	AM_RANGE(0xff00, 0xff1f) AM_DEVREADWRITE_LEGACY("ted7360", ted7360_port_r, ted7360_port_w)
	AM_RANGE(0xff20, 0xffff) AM_READ_BANK("bank8")
	AM_RANGE(0xff3e, 0xff3e) AM_WRITE_LEGACY(c16_switch_to_rom)
	AM_RANGE(0xff3f, 0xff3f) AM_WRITE_LEGACY(c16_switch_to_ram)
ADDRESS_MAP_END

static ADDRESS_MAP_START(plus4_map, AS_PROGRAM, 8, c16_state )
	AM_RANGE(0x0000, 0x7fff) AM_READ_BANK("bank9")
	AM_RANGE(0x8000, 0xbfff) AM_READ_BANK("bank2")
	AM_RANGE(0xc000, 0xfbff) AM_READ_BANK("bank3")
	AM_RANGE(0xfc00, 0xfcff) AM_READ_BANK("bank4")
	AM_RANGE(0x0000, 0xfcff) AM_WRITE_BANK("bank9")
	AM_RANGE(0xfd00, 0xfd0f) AM_READWRITE_LEGACY(c16_6551_port_r, c16_6551_port_w)
	AM_RANGE(0xfd10, 0xfd1f) AM_READWRITE_LEGACY(plus4_6529_port_r, plus4_6529_port_w)
	AM_RANGE(0xfd30, 0xfd3f) AM_READWRITE_LEGACY(c16_6529_port_r, c16_6529_port_w) /* 6529 keyboard matrix */
	AM_RANGE(0xfdd0, 0xfddf) AM_WRITE_LEGACY(c16_select_roms) /* rom chips selection */
	AM_RANGE(0xff00, 0xff1f) AM_DEVREADWRITE_LEGACY("ted7360", ted7360_port_r, ted7360_port_w)
	AM_RANGE(0xff20, 0xffff) AM_READ_BANK("bank8")
	AM_RANGE(0xff20, 0xff3d) AM_WRITEONLY
	AM_RANGE(0xff3e, 0xff3e) AM_WRITE_LEGACY(c16_switch_to_rom)
	AM_RANGE(0xff3f, 0xff3f) AM_WRITE_LEGACY(c16_switch_to_ram)
	AM_RANGE(0xff40, 0xffff) AM_WRITEONLY
ADDRESS_MAP_END

static ADDRESS_MAP_START(c364_map , AS_PROGRAM, 8, c16_state )
	AM_RANGE(0x0000, 0x7fff) AM_READ_BANK("bank9")
	AM_RANGE(0x8000, 0xbfff) AM_READ_BANK("bank2")
	AM_RANGE(0xc000, 0xfbff) AM_READ_BANK("bank3")
	AM_RANGE(0xfc00, 0xfcff) AM_READ_BANK("bank4")
	AM_RANGE(0x0000, 0xfcff) AM_WRITE_BANK("bank9")
	AM_RANGE(0xfd00, 0xfd0f) AM_READWRITE_LEGACY(c16_6551_port_r, c16_6551_port_w)
	AM_RANGE(0xfd10, 0xfd1f) AM_READWRITE_LEGACY(plus4_6529_port_r, plus4_6529_port_w)
	AM_RANGE(0xfd20, 0xfd2f) AM_DEVREADWRITE_LEGACY("t6721", t6721_speech_r, t6721_speech_w)
	AM_RANGE(0xfd30, 0xfd3f) AM_READWRITE_LEGACY(c16_6529_port_r, c16_6529_port_w) /* 6529 keyboard matrix */
	AM_RANGE(0xfdd0, 0xfddf) AM_WRITE_LEGACY(c16_select_roms) /* rom chips selection */
	AM_RANGE(0xff00, 0xff1f) AM_DEVREADWRITE_LEGACY("ted7360", ted7360_port_r, ted7360_port_w)
	AM_RANGE(0xff20, 0xffff) AM_READ_BANK("bank8")
	AM_RANGE(0xff20, 0xff3d) AM_WRITEONLY
	AM_RANGE(0xff3e, 0xff3e) AM_WRITE_LEGACY(c16_switch_to_rom)
	AM_RANGE(0xff3f, 0xff3f) AM_WRITE_LEGACY(c16_switch_to_ram)
	AM_RANGE(0xff40, 0xffff) AM_WRITEONLY
ADDRESS_MAP_END


/*************************************
 *
 *  Input Ports
 *
 *************************************/

static INPUT_PORTS_START( c16 )
	PORT_INCLUDE( common_cbm_keyboard )		/* ROW0 -> ROW7 */

	PORT_MODIFY("ROW0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("@") PORT_CODE(KEYCODE_OPENBRACE)				PORT_CHAR('@')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)									PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)									PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)									PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("HELP f7") PORT_CODE(KEYCODE_F4)				PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_INSERT)								PORT_CHAR(0xA3)

	PORT_MODIFY("ROW1")
	/* Both Shift keys were mapped to the same bit */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift (Left & Right)") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)

	PORT_MODIFY("ROW4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0  \xE2\x86\x91") PORT_CODE(KEYCODE_0)		PORT_CHAR('0') PORT_CHAR(0x2191)

	PORT_MODIFY("ROW5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)					PORT_CHAR('-')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)							PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGUP)									PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_MODIFY("ROW6")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)							PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("=  Pi  \xE2\x86\x90") PORT_CODE(KEYCODE_PGDN)	PORT_CHAR('=') PORT_CHAR(0x03C0) PORT_CHAR(0x2190)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)									PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)								PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)								PORT_CHAR('*')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)									PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_MODIFY("ROW7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Home Clear") PORT_CODE(KEYCODE_DEL)			PORT_CHAR(UCHAR_MAMEKEY(HOME))

	PORT_INCLUDE( c16_special )				/* SPECIAL */

	PORT_INCLUDE( c16_controls )			/* CTRLSEL, JOY0, JOY1 */
INPUT_PORTS_END


static INPUT_PORTS_START( plus4 )
	PORT_INCLUDE( c16 )

	PORT_MODIFY( "ROW0" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)					PORT_CHAR(0xA3)
	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)						PORT_CHAR('-')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP)							PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN)							PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)							PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("=  Pi  \xE2\x86\x90") PORT_CODE(KEYCODE_BACKSLASH2)	PORT_CHAR('=') PORT_CHAR(0x03C0) PORT_CHAR(0x2190)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT)							PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_INSERT)						PORT_CHAR('*')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT)							PORT_CHAR(UCHAR_MAMEKEY(LEFT))
INPUT_PORTS_END

static INPUT_PORTS_START( c16sid )
	PORT_INCLUDE( c16 )

	PORT_START("SID")
	PORT_CONFNAME( 0x01, 0x00, "SID Card Address")
	PORT_CONFSETTING(	0x00, "0xfd40" )
	PORT_CONFSETTING(	0x01, "0xfe80" )
#if 0
	PORT_CONFNAME( 0x02, 0x00, "Enable SID writes to 0xd400")
	PORT_CONFSETTING(	0x00, DEF_STR( No ) )
	PORT_CONFSETTING(	0x02, DEF_STR( Yes ) )
#endif
INPUT_PORTS_END


static INPUT_PORTS_START( plus4sid )
	PORT_INCLUDE( plus4 )

	PORT_START("SID")
	PORT_CONFNAME( 0x01, 0x00, "SID Card Address")
	PORT_CONFSETTING(	0x00, "0xfd40" )
	PORT_CONFSETTING(	0x01, "0xfe80" )
#if 0
	PORT_CONFNAME( 0x02, 0x00, "Enable SID writes to 0xd400")
	PORT_CONFSETTING(	0x00, DEF_STR( No ) )
	PORT_CONFSETTING(	0x02, DEF_STR( Yes ) )
#endif
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const unsigned char ted7360_palette[] =
{
/* black, white, red, cyan */
/* purple, green, blue, yellow */
/* orange, light orange, pink, light cyan, */
/* light violett, light green, light blue, light yellow */
/* these 16 colors are 8 times here in different luminance (dark..light) */
/* taken from digitized tv screenshot */
	0x06, 0x01, 0x03, 0x2b, 0x2b, 0x2b, 0x67, 0x0e, 0x0f, 0x00, 0x3f, 0x42,
	0x57, 0x00, 0x6d, 0x00, 0x4e, 0x00, 0x19, 0x1c, 0x94, 0x38, 0x38, 0x00,
	0x56, 0x20, 0x00, 0x4b, 0x28, 0x00, 0x16, 0x48, 0x00, 0x69, 0x07, 0x2f,
	0x00, 0x46, 0x26, 0x06, 0x2a, 0x80, 0x2a, 0x14, 0x9b, 0x0b, 0x49, 0x00,

	0x00, 0x03, 0x02, 0x3d, 0x3d, 0x3d, 0x75, 0x1e, 0x20, 0x00, 0x50, 0x4f,
	0x6a, 0x10, 0x78, 0x04, 0x5c, 0x00, 0x2a, 0x2a, 0xa3, 0x4c, 0x47, 0x00,
	0x69, 0x2f, 0x00, 0x59, 0x38, 0x00, 0x26, 0x56, 0x00, 0x75, 0x15, 0x41,
	0x00, 0x58, 0x3d, 0x15, 0x3d, 0x8f, 0x39, 0x22, 0xae, 0x19, 0x59, 0x00,

	0x00, 0x03, 0x04, 0x42, 0x42, 0x42, 0x7b, 0x28, 0x20, 0x02, 0x56, 0x59,
	0x6f, 0x1a, 0x82, 0x0a, 0x65, 0x09, 0x30, 0x34, 0xa7, 0x50, 0x51, 0x00,
	0x6e, 0x36, 0x00, 0x65, 0x40, 0x00, 0x2c, 0x5c, 0x00, 0x7d, 0x1e, 0x45,
	0x01, 0x61, 0x45, 0x1c, 0x45, 0x99, 0x42, 0x2d, 0xad, 0x1d, 0x62, 0x00,

	0x05, 0x00, 0x02, 0x56, 0x55, 0x5a, 0x90, 0x3c, 0x3b, 0x17, 0x6d, 0x72,
	0x87, 0x2d, 0x99, 0x1f, 0x7b, 0x15, 0x46, 0x49, 0xc1, 0x66, 0x63, 0x00,
	0x84, 0x4c, 0x0d, 0x73, 0x55, 0x00, 0x40, 0x72, 0x00, 0x91, 0x33, 0x5e,
	0x19, 0x74, 0x5c, 0x32, 0x59, 0xae, 0x59, 0x3f, 0xc3, 0x32, 0x76, 0x00,

	0x02, 0x01, 0x06, 0x84, 0x7e, 0x85, 0xbb, 0x67, 0x68, 0x45, 0x96, 0x96,
	0xaf, 0x58, 0xc3, 0x4a, 0xa7, 0x3e, 0x73, 0x73, 0xec, 0x92, 0x8d, 0x11,
	0xaf, 0x78, 0x32, 0xa1, 0x80, 0x20, 0x6c, 0x9e, 0x12, 0xba, 0x5f, 0x89,
	0x46, 0x9f, 0x83, 0x61, 0x85, 0xdd, 0x84, 0x6c, 0xef, 0x5d, 0xa3, 0x29,

	0x02, 0x00, 0x0a, 0xb2, 0xac, 0xb3, 0xe9, 0x92, 0x92, 0x6c, 0xc3, 0xc1,
	0xd9, 0x86, 0xf0, 0x79, 0xd1, 0x76, 0x9d, 0xa1, 0xff, 0xbd, 0xbe, 0x40,
	0xdc, 0xa2, 0x61, 0xd1, 0xa9, 0x4c, 0x93, 0xc8, 0x3d, 0xe9, 0x8a, 0xb1,
	0x6f, 0xcd, 0xab, 0x8a, 0xb4, 0xff, 0xb2, 0x9a, 0xff, 0x88, 0xcb, 0x59,

	0x02, 0x00, 0x0a, 0xc7, 0xca, 0xc9, 0xff, 0xac, 0xac, 0x85, 0xd8, 0xe0,
	0xf3, 0x9c, 0xff, 0x92, 0xea, 0x8a, 0xb7, 0xba, 0xff, 0xd6, 0xd3, 0x5b,
	0xf3, 0xbe, 0x79, 0xe6, 0xc5, 0x65, 0xb0, 0xe0, 0x57, 0xff, 0xa4, 0xcf,
	0x89, 0xe5, 0xc8, 0xa4, 0xca, 0xff, 0xca, 0xb3, 0xff, 0xa2, 0xe5, 0x7a,

	0x01, 0x01, 0x01, 0xff, 0xff, 0xff, 0xff, 0xf6, 0xf2, 0xd1, 0xff, 0xff,
	0xff, 0xe9, 0xff, 0xdb, 0xff, 0xd3, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xa3,
	0xff, 0xff, 0xc1, 0xff, 0xff, 0xb2, 0xfc, 0xff, 0xa2, 0xff, 0xee, 0xff,
	0xd1, 0xff, 0xff, 0xeb, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xed, 0xff, 0xbc
};

static PALETTE_INIT( c16 )
{
	int i;

	for (i = 0; i < sizeof(ted7360_palette) / 3; i++)
		palette_set_color_rgb(machine, i, ted7360_palette[i * 3], ted7360_palette[i * 3 + 1], ted7360_palette[i * 3 + 2]);
}

/*************************************
 *
 *  TED7360 interfaces
 *
 *************************************/

static const ted7360_interface c16_ted7360_intf = {
	"screen",
	TED7360_PAL,
	c16_dma_read,
	c16_dma_read_rom,
	c16_interrupt,
	c16_read_keyboard
};

static const ted7360_interface plus4_ted7360_intf = {
	"screen",
	TED7360_NTSC,
	c16_dma_read,
	c16_dma_read_rom,
	c16_interrupt,
	c16_read_keyboard
};


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static const m6502_interface c16_m7501_interface =
{
	NULL,					/* read_indexed_func */
	NULL,					/* write_indexed_func */
	DEVCB_HANDLER(c16_m7501_port_read),	/* port_read_func */
	DEVCB_HANDLER(c16_m7501_port_write)	/* port_write_func */
};

static CBM_IEC_INTERFACE( cbm_iec_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static PLUS4_EXPANSION_INTERFACE( expansion_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static PLUS4_USER_PORT_INTERFACE( user_intf )
{
	DEVCB_NULL
};

static SCREEN_UPDATE_IND16( c16 )
{
	c16_state *state = screen.machine().driver_data<c16_state>();
	ted7360_video_update(state->m_ted7360, bitmap, cliprect);
	return 0;
}

static INTERRUPT_GEN( c16_raster_interrupt )
{
	c16_state *state = device->machine().driver_data<c16_state>();
	ted7360_raster_interrupt_gen(state->m_ted7360);
}


static MACHINE_START( c16 )
{
	c16_state *state = machine.driver_data<c16_state>();

	state->m_maincpu = machine.device<legacy_cpu_device>("maincpu");
	state->m_ted7360 = machine.device("ted7360");
	state->m_cassette = machine.device<cassette_image_device>(CASSETTE_TAG);
	state->m_messram = machine.device<ram_device>(RAM_TAG);
	state->m_sid = machine.device("sid");

	state->save_item(NAME(state->m_old_level));
	state->save_item(NAME(state->m_lowrom));
	state->save_item(NAME(state->m_highrom));
	state->save_item(NAME(state->m_port6529));
	state->save_item(NAME(state->m_keyline));
}

static MACHINE_CONFIG_START( c16, c16_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M7501, XTAL_17_73447MHz/20)
	MCFG_CPU_PROGRAM_MAP(c16_map)
	MCFG_CPU_CONFIG( c16_m7501_interface )
	MCFG_CPU_VBLANK_INT("screen", c16_frame_interrupt)
	MCFG_CPU_PERIODIC_INT(c16_raster_interrupt, TED7360_HRETRACERATE)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START( c16 )
	MCFG_MACHINE_RESET( c16 )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(TED7360PAL_VRETRACERATE)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(336, 216)
	MCFG_SCREEN_VISIBLE_AREA(0, 336 - 1, 0, 216 - 1)
	MCFG_SCREEN_UPDATE_STATIC( c16 )

	MCFG_PALETTE_LENGTH(ARRAY_LENGTH(ted7360_palette) / 3)
	MCFG_PALETTE_INIT(c16)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_TED7360_ADD("ted7360", c16_ted7360_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_QUICKLOAD_ADD("quickload", cbm_c16, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)

	/* cassette */
	MCFG_CASSETTE_ADD( CASSETTE_TAG, cbm_cassette_interface )

	MCFG_FRAGMENT_ADD(c16_cartslot)

	MCFG_C1551_ADD(C1551_TAG, 8)
	MCFG_SOFTWARE_LIST_ADD("disk_list", "plus4_flop")

	/* IEC serial bus */
	MCFG_CBM_IEC_ADD(cbm_iec_intf, NULL)

	MCFG_PLUS4_EXPANSION_SLOT_ADD(PLUS4_EXPANSION_SLOT_TAG, XTAL_17_73447MHz/20, expansion_intf, plus4_expansion_cards, NULL, NULL)
	MCFG_PLUS4_USER_PORT_ADD(PLUS4_USER_PORT_TAG, user_intf, plus4_user_port_cards, NULL, NULL)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_EXTRA_OPTIONS("16K,32K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( plus4, c16 )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_CLOCK( XTAL_14_31818MHz/16 )
	MCFG_CPU_PROGRAM_MAP(plus4_map)
	MCFG_CPU_CONFIG( c16_m7501_interface )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(TED7360NTSC_VRETRACERATE)

	MCFG_DEVICE_REMOVE("ted7360")
	MCFG_TED7360_ADD("ted7360", plus4_ted7360_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( c364, plus4 )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(c364_map)

	MCFG_T6721_ADD("t6721")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( c264, c16 )
	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( c16sid, c16 )

	MCFG_SOUND_ADD("sid", SID8580, TED7360PAL_CLOCK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( plus4sid, plus4 )

	MCFG_SOUND_ADD("sid", SID8580, TED7360NTSC_CLOCK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/
ROM_START( c232 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "318006-01.bin", 0x10000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )
	ROM_LOAD( "318004-01.bin", 0x14000, 0x4000, CRC(dbdc3319) SHA1(3c77caf72914c1c0a0875b3a7f6935cd30c54201) )
ROM_END

ROM_START( c264 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "basic-264.bin", 0x10000, 0x4000, CRC(6a2fc8e3) SHA1(473fce23afa07000cdca899fbcffd6961b36a8a0) )
	ROM_LOAD( "kernal-264.bin", 0x14000, 0x4000, CRC(8f32abe7) SHA1(d481faf5fcbb331878dc7851c642d04f26a32873) )
ROM_END

ROM_START( c364 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "318006.01", 0x10000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )
	ROM_LOAD( "kern364p.bin", 0x14000, 0x4000, CRC(84fd4f7a) SHA1(b9a5b5dacd57ca117ef0b3af29e91998bf4d7e5f) )
	ROM_LOAD( "317053-01.bin", 0x18000, 0x4000, CRC(4fd1d8cb) SHA1(3b69f6e7cb4c18bb08e203fb18b7dabfa853390f) )
	ROM_LOAD( "317054-01.bin", 0x1c000, 0x4000, CRC(109de2fc) SHA1(0ad7ac2db7da692d972e586ca0dfd747d82c7693) )
	/* at address 0x20000 not so good */
	ROM_LOAD( "spk3cc4.bin", 0x28000, 0x4000, CRC(5227c2ee) SHA1(59af401cbb2194f689898271c6e8aafa28a7af11) )
ROM_END


ROM_START( c16 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("r5")
	ROM_LOAD( "318006-01.u3", 0x10000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )

	ROM_SYSTEM_BIOS( 0, "r3", "rev. 3" )
	ROMX_LOAD( "318004-03.u4", 0x14000, 0x4000, CRC(77bab934) SHA1(97814dab9d757fe5a3a61d357a9a81da588a9783), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r4", "rev. 4" )
	ROMX_LOAD( "318004-04.u4", 0x14000, 0x4000, CRC(be54ed79) SHA1(514ad3c29d01a2c0a3b143d9c1d4143b1912b793), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "r5", "rev. 5" )
	ROMX_LOAD( "318004-05.u4", 0x14000, 0x4000, CRC(71c07bd4) SHA1(7c7e07f016391174a557e790c4ef1cbe33512cdb), ROM_BIOS(3) )
ROM_END

#define rom_c16c rom_c16
#define rom_c16v rom_c16

#define rom_c116		rom_c16
#define rom_c116c		rom_c16c
#define rom_c116v		rom_c16v

ROM_START( c16hun )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "318006-01.u3", 0x10000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )
	ROM_LOAD( "hungary.u4",   0x14000, 0x4000, CRC(775f60c5) SHA1(20cf3c4bf6c54ef09799af41887218933f2e27ee) )
ROM_END

ROM_START( plus4 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("r5")
	ROM_SYSTEM_BIOS( 0, "r4", "rev. 4" )
	ROMX_LOAD( "318005-04.u24", 0x14000, 0x4000, CRC(799a633d) SHA1(5df52c693387c0e2b5d682613a3b5a65477311cf), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r5", "rev. 5" )
	ROMX_LOAD( "318005-05.u24", 0x14000, 0x4000, CRC(70295038) SHA1(a3d9e5be091b98de39a046ab167fb7632d053682), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos plus4.u24", 0x10000, 0x8000, CRC(818d3f45) SHA1(9bc1b1c3da9ca642deae717905f990d8e36e6c3b), ROM_BIOS(3) )

	ROM_LOAD( "318006-01.u23", 0x10000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )

	ROM_LOAD( "317053-01.u25", 0x18000, 0x4000, CRC(4fd1d8cb) SHA1(3b69f6e7cb4c18bb08e203fb18b7dabfa853390f) )
	ROM_LOAD( "317054-01.26", 0x1c000, 0x4000, CRC(109de2fc) SHA1(0ad7ac2db7da692d972e586ca0dfd747d82c7693) )
ROM_END

#define rom_plus4c rom_plus4
#define rom_plus4v rom_plus4

#define rom_c16sid		rom_c16
#define rom_plus4sid		rom_plus4

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME  PARENT COMPAT MACHINE     INPUT      INIT      COMPANY                         FULLNAME            FLAGS */

COMP( 1984, c16,      0,     0,  c16,       c16, c16_state,       c16,      "Commodore Business Machines",  "Commodore 16 (PAL)", 0)
COMP( 1984, c16hun,   c16,   0,  c16,       c16, c16_state,       c16,      "Commodore Business Machines",  "Commodore 16 Novotrade (PAL, Hungary)", 0)

COMP( 1984, c116,     c16,   0,  c16,       c16, c16_state,       c16,      "Commodore Business Machines",  "Commodore 116 (PAL)", 0)

COMP( 1984, plus4,    c16,   0,  plus4,     plus4, c16_state,     plus4,    "Commodore Business Machines",  "Commodore Plus/4 (NTSC)", 0)

COMP( 1984, c232,     c16,   0,  c16,       c16, c16_state,       c16,      "Commodore Business Machines",  "Commodore 232 (Prototype)", 0)
COMP( 1984, c264,     c16,   0,  c264,      plus4, c16_state,     plus4,    "Commodore Business Machines",  "Commodore 264 (Prototype)", 0)
COMP( 1984, c364,     c16,   0,  c364,      plus4, c16_state,     plus4,    "Commodore Business Machines",  "Commodore V364 (Prototype)", GAME_IMPERFECT_SOUND)

COMP( 1984, c16sid,   c16,   0,  c16sid,    c16sid, c16_state,    c16sid,   "Commodore Business Machines",  "Commodore 16 (PAL, SID Card)", GAME_UNOFFICIAL | GAME_IMPERFECT_SOUND)
COMP( 1984, plus4sid, c16,   0,  plus4sid,  plus4sid, c16_state,  plus4sid, "Commodore Business Machines",  "Commodore Plus/4 (NTSC, SID Card)", GAME_UNOFFICIAL | GAME_IMPERFECT_SOUND)
