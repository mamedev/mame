// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    drivers/dgn_beta.c

    Dragon Beta prototype, based on two 68B09E processors, WD2797, 6845.

Project Beta was the second machine that Dragon Data had in development at
the time they ceased trading, the first being Project Alpha (also know as the
Dragon Professional).

The machine uses dual 68B09 CPUs which both sit on the same bus and access
the same memory and IO chips ! The first is the main processor, used to run
user code, the second is uses as a DMA controller to amongst other things
disk data transfers. The first processor controlled the second by having the
halt and nmi lines from the second CPU connected to PIA output lines so
that the could be changed under OS control. The first CPU just passed
instructions for the block to be transferred in 5 low ram addresses and
generated an NMI on the second CPU.

Project Beta like the other Dragons used a WD2797 floppy disk controller
which is memory mapped, and controlled by the second CPU.

Unlike the other Dragon machines, project Beta used a 68b45 to generate video,
and totally did away with the SAM.

The machine has a 6551 ACIA chip, but I have not yet found where this is
memory mapped.

Project Beta, had a custom MMU built from a combination of LSTTL logic, and
PAL programmable logic. This MMU could address 256 blocks of 4K, giving a
total addressable range of 1 megabyte, of this the first 768KB could be RAM,
however the machine by default, came with 256K or ram, and a 16K boot ROM,
which contained an OS-9 Level 2 bootstrap.

A lot of the information required to start work on this driver has been
inferred from disassembly of the boot ROM, and from what little hardware
documentation still exists.

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "includes/dgn_beta.h"
#include "machine/mos6551.h"
#include "formats/vdk_dsk.h"
#include "formats/dmk_dsk.h"
#include "imagedev/flopdrv.h"
#include "machine/ram.h"
#include "video/mc6845.h"


/*
 Colour codes are as below acording to os-9 headers, however the presise values
 may not be quite correct, also this will need changing as the pallate seems to
 be controled by a 4x4bit register file in the video hardware
 The text video ram seems to be aranged of words of character, attribute
 The colour codes are stored in the attribute byte along with :
    Underline bit   $40
    Flash bit   $80

These are yet to be implemented.

*/

static const unsigned char dgnbeta_palette[] =
{
	/*normal brightness */
	0x00,0x00,0x00,     /* black */
	0x80,0x00,0x00,     /* red */
	0x00,0x80,0x00,     /* green */
	0x80,0x80,0x00,     /* yellow */
	0x00,0x00,0x80,     /* blue */
	0x80,0x00,0x80,     /* magenta */
	0x00,0x80,0x80,     /* cyan */
	0x80,0x80,0x80,     /* white */

	/*enhanced brightness*/
	0x00,0x00,0x00,     /* black */
	0xFF,0x00,0x00,     /* red */
	0x00,0xFF,0x00,     /* green */
	0xFF,0xFF,0x00,     /* yellow */
	0x00,0x00,0xFF,     /* blue */
	0xFF,0x00,0xFF,     /* magenta */
	0x00,0xFF,0xFF,     /* cyan */
	0xFF,0xFF,0xFF      /* white */
};

/*
    2005-05-10

    I *THINK* I know how the memory paging works, the 64K memory map is devided
    into 16x 4K pages, what is mapped into each page is controled by the IO at
    FE00-FE0F like so :-

    Location    Memory page     Initialised to
    $FE00       $0000-$0FFF     $00
    $FE01       $1000-$1FFF     $00 (used as ram test page, when sizing memory)
    $FE02       $2000-$2FFF     $00
    $FE03       $3000-$3FFF     $00
    $FE04       $4000-$4FFF     $00
    $FE05       $5000-$5FFF     $00
    $FE06       $6000-$6FFF     $1F ($1F000)
    $FE07       $7000-$7FFF     $00
    $FE08       $8000-$8FFF     $00
    $FE09       $9000-$9FFF     $00
    $FE0A       $A000-$AFFF     $00
    $FE0B       $B000-$BFFF     $00
    $FE0C       $C000-$CFFF     $00
    $FE0D       $D000-$DFFF     $00
    $FE0E       $E000-$EFFF     $FE
    $FE0F       $F000-$FFFF     $FF

    The value stored at each location maps it's page to a 4K page within a 1M address
    space. Acording to the Beta product descriptions released by Dragon Data, the
    machine could have up to 768K of RAM, if this where true then pages $00-$BF could
    potentially be RAM, and pages $C0-$FF would be ROM. The initialisation code maps in
    the memory as described above.

    At reset time the Paging would of course be disabled, as the boot rom needs to be
    mapped in at $C000, the initalisation code would set up the mappings above and then
    enable the paging hardware.

    It appears to be more complicated than this, whilst the above is true, there appear to
    be 16 sets of banking registers, the active set is controled by the bottom 4 bits of
    FCC0, bit 6 has something to do with enabling and disabling banking.

    2005-11-28

    The value $C0 is guaranteed not to have any memory in it acording to the os9 headers,
    quite how the MMU deals with this is still unknown to me.

    Bit 7 of $FCC0, sets maps in the system task which has fixed values for some pages,
    the presise nature of this is yet to be descovered.

*/

static ADDRESS_MAP_START( dgnbeta_map, AS_PROGRAM, 8, dgn_beta_state )
	AM_RANGE(0x0000, 0x0FFF)    AM_RAMBANK("bank1")
	AM_RANGE(0x1000, 0x1FFF)    AM_RAMBANK("bank2")
	AM_RANGE(0x2000, 0x2FFF)    AM_RAMBANK("bank3")
	AM_RANGE(0x3000, 0x3FFF)    AM_RAMBANK("bank4")
	AM_RANGE(0x4000, 0x4FFF)    AM_RAMBANK("bank5")
	AM_RANGE(0x5000, 0x5FFF)    AM_RAMBANK("bank6")
	AM_RANGE(0x6000, 0x6FFF)    AM_RAMBANK("bank7") AM_SHARE("videoram")
	AM_RANGE(0x7000, 0x7FFF)    AM_RAMBANK("bank8")
	AM_RANGE(0x8000, 0x8FFF)    AM_RAMBANK("bank9")
	AM_RANGE(0x9000, 0x9FFF)    AM_RAMBANK("bank10")
	AM_RANGE(0xA000, 0xAFFF)    AM_RAMBANK("bank11")
	AM_RANGE(0xB000, 0xBFFF)    AM_RAMBANK("bank12")
	AM_RANGE(0xC000, 0xCFFF)    AM_RAMBANK("bank13")
	AM_RANGE(0xD000, 0xDFFF)    AM_RAMBANK("bank14")
	AM_RANGE(0xE000, 0xEFFF)    AM_RAMBANK("bank15")
	AM_RANGE(0xF000, 0xFBFF)    AM_RAMBANK("bank16")
	AM_RANGE(0xfC00, 0xfC1F)    AM_NOP
	AM_RANGE(0xFC20, 0xFC23)    AM_DEVREADWRITE(PIA_0_TAG, pia6821_device, read, write)
	AM_RANGE(0xFC24, 0xFC27)    AM_DEVREADWRITE(PIA_1_TAG, pia6821_device, read, write)
	AM_RANGE(0xFC28, 0xfC7F)    AM_NOP
	AM_RANGE(0xfc80, 0xfc80)    AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xfc81, 0xfc81)    AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0xfc82, 0xfC9F)    AM_NOP
	AM_RANGE(0xFCA0, 0xFCA3)    AM_READNOP AM_WRITE(dgnbeta_colour_ram_w)           /* 4x4bit colour ram for graphics modes */
	AM_RANGE(0xFCC0, 0xFCC3)    AM_DEVREADWRITE(PIA_2_TAG, pia6821_device, read, write)
	AM_RANGE(0xfcC4, 0xfcdf)    AM_NOP
	AM_RANGE(0xfce0, 0xfce3)    AM_READWRITE(dgnbeta_wd2797_r   ,dgnbeta_wd2797_w)  /* Onboard disk interface */
	AM_RANGE(0xfce4, 0xfdff)    AM_NOP
	AM_RANGE(0xFE00, 0xFE0F)    AM_READWRITE(dgn_beta_page_r    ,dgn_beta_page_w)
	AM_RANGE(0xfe10, 0xfEff)    AM_NOP
	AM_RANGE(0xFF00, 0xFFFF)    AM_RAMBANK("bank17")

ADDRESS_MAP_END



static INPUT_PORTS_START( dgnbeta )
	PORT_START("KEY0") /* Key ROw 0 */
	/* Return shift if either shift key pressed */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	/* Return FuNction key, if either ALT key pressed */
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ALT") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_HOME) PORT_CHAR(12)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  (") PORT_CODE(KEYCODE_8) PORT_CHAR('8')


	/* Set control on either CTRL key */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)

	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CapsLock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(0x1B)

/*  PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_HOME) PORT_CHAR(12)
    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  (") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(0x1B)
*/

	PORT_START("KEY1") /* Key row 2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b") PORT_CODE(KEYCODE_B) PORT_CHAR('b')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("j") PORT_CODE(KEYCODE_J) PORT_CHAR('j')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("i") PORT_CODE(KEYCODE_I) PORT_CHAR('i')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("u") PORT_CODE(KEYCODE_U) PORT_CHAR('u')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("h") PORT_CODE(KEYCODE_H) PORT_CHAR('h')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("KEY2") /* Key row 3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("n") PORT_CODE(KEYCODE_N) PORT_CHAR('n')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("k") PORT_CODE(KEYCODE_K) PORT_CHAR('k')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("o") PORT_CODE(KEYCODE_O) PORT_CHAR('o')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("g") PORT_CODE(KEYCODE_G) PORT_CHAR('g')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')

	PORT_START("KEY3") /* Key row  4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("m") PORT_CODE(KEYCODE_M) PORT_CHAR('m')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("l") PORT_CODE(KEYCODE_L) PORT_CHAR('l')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("p") PORT_CODE(KEYCODE_P) PORT_CHAR('p')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("t") PORT_CODE(KEYCODE_T) PORT_CHAR('t')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f") PORT_CODE(KEYCODE_F) PORT_CHAR('f')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')

	PORT_START("KEY4") /* Key row  5 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(';')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("v") PORT_CODE(KEYCODE_V) PORT_CHAR('v')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("d") PORT_CODE(KEYCODE_D) PORT_CHAR('d')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')

	PORT_START("KEY5") /* Key row  6 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_COLON) PORT_CHAR(':')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("r") PORT_CODE(KEYCODE_R) PORT_CHAR('r')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("s") PORT_CODE(KEYCODE_S) PORT_CHAR('s')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')

	PORT_START("KEY6") /* Key row  7 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DELETE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("c") PORT_CODE(KEYCODE_C) PORT_CHAR('c')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("x") PORT_CODE(KEYCODE_X) PORT_CHAR('x')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("KEY7") /* Key row  8 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ASTERISK") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PAD_7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PAD_4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PAD_1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("e") PORT_CODE(KEYCODE_E) PORT_CHAR('e')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')

	PORT_START("KEY8") /* Key row  9 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PAD_0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PAD_8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PAD_5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PAD_2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("w") PORT_CODE(KEYCODE_W) PORT_CHAR('w')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("a") PORT_CODE(KEYCODE_A) PORT_CHAR('a')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')

	PORT_START("KEY9") /* Key row  10 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PAD_HASH") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('#')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PAD_9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PAD_6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PAD_3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DOT_PAD") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR('.')

INPUT_PORTS_END


PALETTE_INIT_MEMBER(dgn_beta_state, dgn)
{
	int i;

	for ( i = 0; i < sizeof(dgnbeta_palette) / 3; i++ ) {
		palette.set_pen_color(i, dgnbeta_palette[i*3], dgnbeta_palette[i*3+1], dgnbeta_palette[i*3+2]);
	}
}

/* F4 Character Displayer */
static const gfx_layout dgnbeta_charlayout =
{
	8, 10,                  /* 8 x 10 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( dgnbeta )
	GFXDECODE_ENTRY( "gfx1", 0x0000, dgnbeta_charlayout, 0, 8 )
GFXDECODE_END

FLOPPY_FORMATS_MEMBER( dgn_beta_state::floppy_formats )
	FLOPPY_VDK_FORMAT,
	FLOPPY_DMK_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( dgn_beta_floppies )
	SLOT_INTERFACE("qd", FLOPPY_525_QD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( dgnbeta, dgn_beta_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(MAINCPU_TAG, M6809E, DGNBETA_CPU_SPEED_HZ)        /* 2 MHz */
	MCFG_CPU_PROGRAM_MAP(dgnbeta_map)

	/* both cpus in the beta share the same address/data busses */
	MCFG_CPU_ADD(DMACPU_TAG, M6809E, DGNBETA_CPU_SPEED_HZ)        /* 2 MHz */
	MCFG_CPU_PROGRAM_MAP(dgnbeta_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(DGNBETA_FRAMES_PER_SECOND)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(100))
	MCFG_SCREEN_SIZE(700,550)
	MCFG_SCREEN_VISIBLE_AREA(0, 699, 0, 549)
	MCFG_SCREEN_UPDATE_DEVICE( "crtc", hd6845_device, screen_update )
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dgnbeta)
	MCFG_PALETTE_ADD("palette", ARRAY_LENGTH(dgnbeta_palette) / 3)
	MCFG_PALETTE_INIT_OWNER(dgn_beta_state, dgn)

	/* PIA 0 at $FC20-$FC23 I46 */
	MCFG_DEVICE_ADD(PIA_0_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(dgn_beta_state,d_pia0_pa_r))
	MCFG_PIA_READPB_HANDLER(READ8(dgn_beta_state,d_pia0_pb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(dgn_beta_state,d_pia0_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(dgn_beta_state,d_pia0_pb_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(dgn_beta_state,d_pia0_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(dgn_beta_state,d_pia0_irq_a))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(dgn_beta_state,d_pia0_irq_b))

	/* PIA 1 at $FC24-$FC27 I63 */
	MCFG_DEVICE_ADD(PIA_1_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(dgn_beta_state,d_pia1_pa_r))
	MCFG_PIA_READPB_HANDLER(READ8(dgn_beta_state,d_pia1_pb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(dgn_beta_state,d_pia1_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(dgn_beta_state,d_pia1_pb_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(dgn_beta_state,d_pia1_irq_a))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(dgn_beta_state,d_pia1_irq_b))

	/* PIA 2 at FCC0-FCC3 I28 */
	/* This seems to control the RAM paging system, and have the DRQ */
	/* from the WD2797 */
	MCFG_DEVICE_ADD(PIA_2_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(dgn_beta_state,d_pia2_pa_r))
	MCFG_PIA_READPB_HANDLER(READ8(dgn_beta_state,d_pia2_pb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(dgn_beta_state,d_pia2_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(dgn_beta_state,d_pia2_pb_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(dgn_beta_state,d_pia2_irq_a))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(dgn_beta_state,d_pia2_irq_b))

	MCFG_WD2797_ADD(FDC_TAG, XTAL_1MHz)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(dgn_beta_state, dgnbeta_fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(dgn_beta_state, dgnbeta_fdc_drq_w))

	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG ":0", dgn_beta_floppies, "qd", dgn_beta_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG ":1", dgn_beta_floppies, "qd", dgn_beta_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG ":2", dgn_beta_floppies, "qd", dgn_beta_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG ":3", dgn_beta_floppies, "qd", dgn_beta_state::floppy_formats)

	MCFG_MC6845_ADD("crtc", HD6845, "screen", XTAL_12_288MHz / 16)    //XTAL is guessed
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(16) /*?*/
	MCFG_MC6845_UPDATE_ROW_CB(dgn_beta_state, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(dgn_beta_state, dgnbeta_vsync_changed))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256K")
	MCFG_RAM_EXTRA_OPTIONS("128K,384K,512K,640K,768K")
	/* Ram size can now be configured, since the machine was known as either the Dragon Beta or */
	/* the Dragon 128, I have added a config for 128K, however, the only working machine known  */
	/* to exist was fitted with 256K, so I have made this the default. Also available           */
	/* documentation seems to sugest a maximum of 768K, so I have included configs increasing   */
	/* in blocks of 128K up to this maximum.                                                    */
MACHINE_CONFIG_END

ROM_START(dgnbeta)
	ROM_REGION(0x4000,MAINCPU_TAG,0)
	ROM_SYSTEM_BIOS( 0, "bootrom", "Dragon beta OS9 boot rom (1984)" )
	ROMX_LOAD("beta_bt.rom"     ,0x0000 ,0x4000 ,CRC(4c54c1de) SHA1(141d9fcd2d187c305dff83fce2902a30072aed76), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "testrom", "Dragon beta test rom (1984?)" )
	ROMX_LOAD("beta_tst.rom"    ,0x2000 ,0x2000 ,CRC(01d79d00) SHA1(343e08cf7656b5e8970514868df37ea0af1e2362), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "cfiles", "cfiles rom" )
	ROMX_LOAD("beta_cfi.rom"    ,0x2000 ,0x2000 ,CRC(d312e4c0) SHA1(5c00daac488eaf8d36d66de6ec6c746ab7b78ecf), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "dfiles", "dfiles rom" )
	ROMX_LOAD("beta_dfi.rom"    ,0x2000 ,0x2000 ,CRC(c4ad7f64) SHA1(50aa92a1c383321485d5a1aa41dfe4f90b3beaed), ROM_BIOS(4))

	ROM_REGION (0x2000, "gfx1", 0)
	ROM_LOAD("betachar.rom" ,0x0000 ,0x2000 ,CRC(ca79d66c) SHA1(8e2090d471dd97a53785a7f44a49d3c8c85b41f2))
ROM_END

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT    COMPANY             FULLNAME                    FLAGS */
COMP( 1984, dgnbeta,    0,      0,      dgnbeta,    dgnbeta, driver_device,    0,      "Dragon Data Ltd",  "Dragon Beta Prototype",    MACHINE_NO_SOUND )
