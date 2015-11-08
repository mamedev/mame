// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*********************************************************************

    drivers/lisa.c

    Experimental LISA driver

    Raphael Nabet, 2000

*********************************************************************/

#include "emu.h"
#include "cpu/m6502/m6504.h"
#include "cpu/cop400/cop400.h"
#include "includes/lisa.h"
#include "formats/ap_dsk35.h"
#include "softlist.h"

/***************************************************************************
    ADDRESS MAP
***************************************************************************/

static ADDRESS_MAP_START(lisa_map, AS_PROGRAM, 16, lisa_state )
	AM_RANGE(0x000000, 0xffffff) AM_READWRITE(lisa_r, lisa_w)           /* no fixed map, we use an MMU */
ADDRESS_MAP_END

static ADDRESS_MAP_START(lisa_fdc_map, AS_PROGRAM, 8, lisa_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff) // only 8k of address space
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE("fdc_ram")             /* RAM (shared with 68000) */
	AM_RANGE(0x0400, 0x07ff) AM_READWRITE(lisa_fdc_io_r, lisa_fdc_io_w) /* disk controller (IWM and TTL logic) */
	AM_RANGE(0x0800, 0x0fff) AM_NOP
	AM_RANGE(0x1000, 0x1fff) AM_ROM AM_REGION("fdccpu", 0x1000) AM_SHARE("fdc_rom")     /* ROM */
	AM_RANGE(0x2000, 0xffff) AM_READWRITE(lisa_fdc_r, lisa_fdc_w)       /* handler for wrap-around */
ADDRESS_MAP_END

static ADDRESS_MAP_START(lisa210_fdc_map, AS_PROGRAM, 8, lisa_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff) // only 8k of address space
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE("fdc_ram")             /* RAM (shared with 68000) */
	AM_RANGE(0x0400, 0x07ff) AM_NOP                                     /* nothing, or RAM wrap-around ??? */
	AM_RANGE(0x0800, 0x0bff) AM_READWRITE(lisa_fdc_io_r, lisa_fdc_io_w) /* disk controller (IWM and TTL logic) */
	AM_RANGE(0x0c00, 0x0fff) AM_NOP                                     /* nothing, or IO port wrap-around ??? */
	AM_RANGE(0x1000, 0x1fff) AM_ROM AM_REGION("fdccpu", 0x1000) AM_SHARE("fdc_rom")         /* ROM */
	AM_RANGE(0x2000, 0xffff) AM_READWRITE(lisa_fdc_r, lisa_fdc_w)       /* handler for wrap-around */
ADDRESS_MAP_END



/***************************************************************************
    DEVICE CONFIG
***************************************************************************/

static void lisa2_set_iwm_enable_lines(device_t *device,int enable_mask)
{
	/* E1 & E2 is connected to the Sony SEL line (?) */
	/*logerror("new sel line state %d\n", (enable_mask) ? 0 : 1);*/
	sony_set_sel_line(device,(enable_mask) ? 0 : 1);
}

static void lisa210_set_iwm_enable_lines(device_t *device,int enable_mask)
{
	/* E2 is connected to the Sony enable line (?) */
	sony_set_enable_lines(device,enable_mask >> 1);
}

static const applefdc_interface lisa2_fdc_interface =
{
	sony_set_lines,
	lisa2_set_iwm_enable_lines,

	sony_read_data,
	sony_write_data,
	sony_read_status
};

static const applefdc_interface lisa210_fdc_interface =
{
	sony_set_lines,
	lisa210_set_iwm_enable_lines,

	sony_read_data,
	sony_write_data,
	sony_read_status
};

static const floppy_interface lisa_floppy_interface =
{
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(apple35_mac),
	"floppy_5_25"
};

/***************************************************************************
    MACHINE DRIVER
***************************************************************************/

/* Lisa1 and Lisa 2 machine */
static MACHINE_CONFIG_START( lisa, lisa_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 5093760)        /* 20.37504 MHz / 4 */
	MCFG_CPU_PROGRAM_MAP(lisa_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", lisa_state,  lisa_interrupt)

	MCFG_CPU_ADD(COP421_TAG, COP421, 3900000)
	MCFG_COP400_CONFIG( COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, COP400_MICROBUS_ENABLED )

	MCFG_CPU_ADD(KB_COP421_TAG, COP421, 3900000) // ?
	MCFG_COP400_CONFIG( COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, COP400_MICROBUS_ENABLED )

	MCFG_CPU_ADD("fdccpu", M6504, 2000000)        /* 16.000 MHz / 8 in when DIS asserted, 16.000 MHz / 9 otherwise (?) */
	MCFG_CPU_PROGRAM_MAP(lisa_fdc_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(880, 380)
	MCFG_SCREEN_VISIBLE_AREA(0, 720-1, 0, 364-1)
	MCFG_SCREEN_UPDATE_DRIVER(lisa_state, screen_update_lisa)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* nvram */
	MCFG_NVRAM_ADD_CUSTOM_DRIVER("nvram", lisa_state, nvram_init)

	/* devices */
	MCFG_IWM_ADD("fdc", lisa2_fdc_interface)
	MCFG_LEGACY_FLOPPY_SONY_2_DRIVES_ADD(lisa_floppy_interface)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","lisa")

	/* via */
	MCFG_DEVICE_ADD("via6522_0", VIA6522, 500000)
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(lisa_state, COPS_via_out_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(lisa_state, COPS_via_out_b))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(lisa_state, COPS_via_out_ca2))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(lisa_state, COPS_via_out_cb2))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(lisa_state, COPS_via_irq_func))

	MCFG_DEVICE_ADD("via6522_1", VIA6522, 500000)

	MCFG_DEVICE_ADD("scc", SCC8530, 7833600)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( lisa210, lisa )
	MCFG_CPU_MODIFY( "fdccpu" )
	MCFG_CPU_PROGRAM_MAP(lisa210_fdc_map)

	/* Lisa 2/10 and MacXL had a slightly different FDC interface */
	MCFG_IWM_MODIFY("fdc", lisa210_fdc_interface)

	/* via */
	MCFG_DEVICE_MODIFY("via6522_0")
	MCFG_DEVICE_CLOCK(1250000)
	MCFG_DEVICE_MODIFY("via6522_1")
	MCFG_DEVICE_CLOCK(1250000)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( macxl, lisa210 )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(   768/* ???? */, 447/* ???? */)
	MCFG_SCREEN_VISIBLE_AREA(0, 608-1, 0, 431-1)
MACHINE_CONFIG_END

/* 2008-05 FP:
Small note about natural keyboard support: currently,
- "Clear" (on the Keypad) is mapped to 'F1'
- "Enter" (different from Return) is mapped to 'F2'
- "Keypad ," is currently unmapped                   */

static INPUT_PORTS_START( lisa )
	PORT_START("MOUSE_X") /* Mouse - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("MOUSE_Y") /* Mouse - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	/* pseudo-input ports with keyboard layout (based on pictures) */
	PORT_START("LINE0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Button")

	PORT_START("LINE1")
	PORT_BIT(0xFFFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)         PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)          PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)         PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)             PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)             PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)             PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)          PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)             PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)             PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)             PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad ,")    // this one would be between '+' and 'Enter' on a modern keypad.
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)           PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)             PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)             PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)         PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))

	PORT_START("LINE3")
	PORT_BIT(0xFFFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)             PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)            PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)         PORT_CHAR('\\') PORT_CHAR('|') // this one would be 2nd row, 3rd key from 'P'
#if 1
	/* US layout */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNUSED)
#else
	/* European layout */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("~  `") PORT_CODE(KEYCODE_BACKSLASH2)
	/* 2008-05 FP: Differences in European Layout (based on a couple of pictures found in the web):
	- at KEYCODE_ESC, "`  ~" is replaced by "?  #"
	- Shift + 3  gives the pound symbol (\xC2\xA3)
	- There is no "\  |" key after "]  }"
	- There is an additional key at 3rd row, 3rd key from 'L', and it's  "`  ~"
	- Between Left Shift and Z there is another key, but the image is not clear on that key
	*/
#endif
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)             PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)     PORT_CHAR(8)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_MENU) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)         PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x0C00, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)         PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)         PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Option") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)             PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)             PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)             PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)             PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)             PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)             PORT_CHAR('l') PORT_CHAR('M')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)             PORT_CHAR('m') PORT_CHAR('L')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)         PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)         PORT_CHAR(' ')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)         PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)          PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)             PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("LINE6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)             PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)             PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)             PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)             PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)             PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)             PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)           PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)             PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)             PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)             PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)             PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)             PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)             PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)             PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("LINE7")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)             PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)             PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)             PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)             PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)             PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)             PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)             PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)             PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)           PORT_CHAR('\t')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)             PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)             PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)             PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Option") PORT_CODE(KEYCODE_LALT)                 PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Alpha Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE  PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)                     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Command") PORT_CODE(KEYCODE_LCONTROL)                 PORT_CHAR(UCHAR_SHIFT_2)
INPUT_PORTS_END

	/* Note we're missing a whole bunch of lisa bootrom revisions; based on http://www.cs.dartmouth.edu/~woz/bootrom.pdf :
	?A?(10/12/82) ?B?(11/19/82) ?C?(1/28/83) D(5/12/83) 3B(9/8/83) E(10/20/83) F(12/21/83) G(2/8/84) and H(2/24/84) are known to exist. Earlier prototypes existed as well. Only F and H are dumped. */
	/* Based on http://www.cs.dartmouth.edu/~woz/bootrom.pdf and other information, at least the following systems existed:
	 * Lisa: two 890K twiggy drives, old MB (slow 500khz-clock parallel port via), old i/o board w/io40 disk rom, supports profile hdd, all bootrom revs (practically speaking, it only appeared with bootroms A, B, C, D, E or F)
	 * Lisa2 (aka Lisa2/5): one 400K SSDD drive, old MB (slow 500khz-clock parallel port via), old i/o board w/ioa8 disk rom, supports profile hdd, bootrom revs "3B", E, F, G, H
	 * Lisa2/10: one 400K SSDD drive, new MB (fast 1.25MHz-clock parallel port via), new i/o board w/ioa8 disk rom, internal widget 10mb drive (no profile hdd ports), bootrom revs F, G, H
	 * MacXL: one 400K SSDD drive, new MB (fast 1.25MHz-clock parallel port via), new i/o board w/io88 disk rom, internal widget 10mb drive (no profile hdd ports), bootrom rev 3A, different screen aspect, no serial number in video state rom so lisa office would not run
	 * Sun-remanufactured MacXL: one 800K DSDD drive, new MB (fast 1.25MHz-clock parallel port via), sun-made custom disk rom (3 revisions exist), internal custom 10mb,20mb, or 40mb drive (?no profile hdd ports?), bootrom rev 3A, different screen aspect, no serial number in video state rom so lisa apps would not run
	 */
		/* the old i/o board has a battery pack on it which often leaks and destroys the board, and has a socket for an amd 2915 math co-procesor; the new i/o board lacks the battery pack and the socket for the coprocessor, and integrates some of the function of the old twiggy-to-400k drive convertor board (which all lisa2/5s had) on it, requiring a mod to be done to the old convertor if used with a new i/o board.*/
	/* Twiggy disk format notes: twiggy disks seem to have wide '48tpi' heads, but cram 62.5tpi on the media by closely spacing the tracks! Twiggy media is DSHD-grade (needing strong magnetic field to set due to high data rate, see http://www.folklore.org/StoryView.py?project=Macintosh&story=Hide_Under_This_Desk.txt). The twiggy format is *PROBABLY* GCR encoding similar to apple2 and mac800k. The disks are 5.25" disks with TWO holes for the drive heads on both sides of the media, and the write protect notch in an unusual place (the index hole is in its normal position, though). By using variable motor speed similar to the later apple 3.5" disks, double sided disks, and tight track spacing, 871,424 bytes are stored per disk. see http://www.brouhaha.com/~eric/retrocomputing/lisa/twiggy.html
	The drives were notoriously unreliable and were replaced by a single SSDD Sony-made varialble speed 400k drive in the lisa2, which was also used on the original macintosh. */
	/* which systems used the 341-0193-A parallel interface card rom? */


ROM_START( lisa ) /* with twiggy drives, io40 i/o rom; technically any of the bootroms will work on this. */
	ROM_REGION16_BE(0x204000,"maincpu",0)   /* 68k rom and ram */
	ROM_DEFAULT_BIOS( "revh" )

	ROM_SYSTEM_BIOS( 0, "revh", "LISA Bootrom Rev H (2/24/84)")
	ROMX_LOAD( "341-0175-h", 0x000000, 0x2000, CRC(adfd4516) SHA1(97a89ce1218b8aa38f69f92f6f363f435c887914), ROM_SKIP(1) | ROM_BIOS(1)) /* 341-0175-H LISA Bootrom Rev H (2/24/84) (High) */
	ROMX_LOAD( "341-0176-h", 0x000001, 0x2000, CRC(546d6603) SHA1(2a81e4d483f50ae8a2519621daeb7feb440a3e4d), ROM_SKIP(1) | ROM_BIOS(1)) /* 341-0176-H LISA Bootrom Rev H (2/24/84) (Low) */

	ROM_SYSTEM_BIOS( 1, "revg", "LISA Bootrom Rev G (2/08/84)") /* limited test release before release of rom rev H */
	ROMX_LOAD( "341-0175-g", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(2)) /* 341-0175-G LISA Bootrom Rev G (2/08/84) (High) */
	ROMX_LOAD( "341-0176-g", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(2)) /* 341-0176-G LISA Bootrom Rev G (2/08/84) (Low) */

	ROM_SYSTEM_BIOS( 2, "revf", "LISA Bootrom Rev F (12/21/83)")
	ROMX_LOAD( "341-0175-f", 0x000000, 0x2000, CRC(701b9dab) SHA1(b116e5fada7b9a51f1b6e25757b2814d1b2737a5), ROM_SKIP(1) | ROM_BIOS(3)) /* 341-0175-F LISA Bootrom Rev F (12/21/83) (High) */
	ROMX_LOAD( "341-0176-f", 0x000001, 0x2000, CRC(036010b6) SHA1(ac93e6dbe4ce59396d7d191ee3e3e79a504e518f), ROM_SKIP(1) | ROM_BIOS(3)) /* 341-0176-F LISA Bootrom Rev F (12/21/83) (Low) */

	ROM_SYSTEM_BIOS( 3, "reve", "LISA Bootrom Rev E (10/20/83)")
	ROMX_LOAD( "341-0175-e", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(4)) /* 341-0175-E LISA Bootrom Rev E (10/20/83) (High) */
	ROMX_LOAD( "341-0176-e", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(4)) /* 341-0176-E LISA Bootrom Rev E (10/20/83) (Low) */

	ROM_SYSTEM_BIOS( 4, "revd", "LISA Bootrom Rev D (5/12/83)")
	ROMX_LOAD( "341-0175-d", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(5)) /* 341-0175-D LISA Bootrom Rev D (5/12/83) (High) */
	ROMX_LOAD( "341-0176-d", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(5)) /* 341-0176-D LISA Bootrom Rev D (5/12/83) (Low) */

	ROM_SYSTEM_BIOS( 5, "revc", "LISA Bootrom Rev C (1/28/83?)")
	ROMX_LOAD( "341-0175-c", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(6)) /* 341-0175-C LISA Bootrom Rev C (1/28/83) (High) */
	ROMX_LOAD( "341-0176-c", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(6)) /* 341-0176-C LISA Bootrom Rev C (1/28/83) (Low) */

	ROM_SYSTEM_BIOS( 6, "revb", "LISA Bootrom Rev B (11/19/82?)")
	ROMX_LOAD( "341-0175-b", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(7)) /* 341-0175-B LISA Bootrom Rev B (11/19/82?) (High) */
	ROMX_LOAD( "341-0176-b", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(7)) /* 341-0176-B LISA Bootrom Rev B (11/19/82?) (Low) */

	ROM_SYSTEM_BIOS( 7, "reva", "LISA Bootrom Rev A (10/12/82?)") /* possibly only prototypes */
	ROMX_LOAD( "341-0175-a", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(8)) /* 341-0175-A LISA Bootrom Rev A (10/12/82?) (High) */
	ROMX_LOAD( "341-0176-a", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(8)) /* 341-0176-A LISA Bootrom Rev A (10/12/82?) (Low) */

	ROM_REGION( 0x400, COP421_TAG, 0 )
	ROM_LOAD( "341-0064a.u9f", 0x000, 0x400, CRC(e6849910) SHA1(d46e67df75c9e3e773d20542fb9d5b1d2ac0fb9b) )

	ROM_REGION( 0x400, KB_COP421_TAG, 0 )
	ROM_LOAD( "341-0064a.u9f", 0x000, 0x400, CRC(e6849910) SHA1(d46e67df75c9e3e773d20542fb9d5b1d2ac0fb9b) )

	ROM_REGION(0x2000,"fdccpu",0)       /* 6504 RAM and ROM */
	/* note: other ?prototype? revisions of this rom for the lisa probably exist as well */
	ROM_LOAD( "341-0138f.bin", 0x001000, 0x001000, CRC(edd8d560) SHA1(872211d21386cd9625b3735d7682e2b2ecff05b4) )

	ROM_REGION(0x4000,"profile", 0)     /* Profile/5 HDD */
	ROM_LOAD_OPTIONAL( "341-0080-b", 0x0000, 0x800, CRC(26df0b8d) SHA1(08f6689afb517e0a2bdaa48433003e62a66ae3c7)) /* 341-0080-B z8 mcu piggyback rom */

	/* TODO: the 341-0193-A parallel interface card rom should be loaded here as well for the lisa 1 and 2/5? */

	ROM_REGION(0x100,"gfx1",0)      /* video ROM (includes S/N) */
	ROM_LOAD( "vidstate.rom", 0x00, 0x100, CRC(75904783) SHA1(3b0023bd90f2ca1be0b099160a566b044856885d))
ROM_END

ROM_START( lisa2 ) /* internal apple codename was 'pepsi'; has one SSDD 400K drive, ioa8 i/o rom */
	ROM_REGION16_BE(0x204000,"maincpu",0)   /* 68k rom and ram */
	ROM_DEFAULT_BIOS( "revh" )

	ROM_SYSTEM_BIOS( 0, "revh", "LISA Bootrom Rev H (2/24/84)")
	ROMX_LOAD( "341-0175-h", 0x000000, 0x2000, CRC(adfd4516) SHA1(97a89ce1218b8aa38f69f92f6f363f435c887914), ROM_SKIP(1) | ROM_BIOS(1)) /* 341-0175-H LISA Bootrom Rev H (2/24/84) (High) */
	ROMX_LOAD( "341-0176-h", 0x000001, 0x2000, CRC(546d6603) SHA1(2a81e4d483f50ae8a2519621daeb7feb440a3e4d), ROM_SKIP(1) | ROM_BIOS(1)) /* 341-0176-H LISA Bootrom Rev H (2/24/84) (Low) */

	ROM_SYSTEM_BIOS( 1, "revg", "LISA Bootrom Rev G (2/08/84)") /* limited test release before release of rom rev H */
	ROMX_LOAD( "341-0175-g", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(2)) /* 341-0175-G LISA Bootrom Rev G (2/08/84) (High) */
	ROMX_LOAD( "341-0176-g", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(2)) /* 341-0176-G LISA Bootrom Rev G (2/08/84) (Low) */

	ROM_SYSTEM_BIOS( 2, "revf", "LISA Bootrom Rev F (12/21/83)")
	ROMX_LOAD( "341-0175-f", 0x000000, 0x2000, CRC(701b9dab) SHA1(b116e5fada7b9a51f1b6e25757b2814d1b2737a5), ROM_SKIP(1) | ROM_BIOS(3)) /* 341-0175-F LISA Bootrom Rev F (12/21/83) (High) */
	ROMX_LOAD( "341-0176-f", 0x000001, 0x2000, CRC(036010b6) SHA1(ac93e6dbe4ce59396d7d191ee3e3e79a504e518f), ROM_SKIP(1) | ROM_BIOS(3)) /* 341-0176-F LISA Bootrom Rev F (12/21/83) (Low) */

	ROM_SYSTEM_BIOS( 3, "reve", "LISA Bootrom Rev E (10/20/83)")
	ROMX_LOAD( "341-0175-e", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(4)) /* 341-0175-E LISA Bootrom Rev E (10/20/83) (High) */
	ROMX_LOAD( "341-0176-e", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(4)) /* 341-0176-E LISA Bootrom Rev E (10/20/83) (Low) */

	ROM_SYSTEM_BIOS( 4, "rev3b", "LISA Bootrom Rev 3B (9/8/83)") /* Earliest lisa2 rom, prototype. */
	ROMX_LOAD( "341-0175-3b", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(5)) /* ?label? 341-0175-3b LISA Bootrom Rev 3B (9/8/83) (High) */
	ROMX_LOAD( "341-0176-3b", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(5)) /* ?label? 341-0176-3b LISA Bootrom Rev 3B (9/8/83) (Low) */

	ROM_REGION( 0x400, COP421_TAG, 0 )
	ROM_LOAD( "341-0064a.u9f", 0x000, 0x400, CRC(e6849910) SHA1(d46e67df75c9e3e773d20542fb9d5b1d2ac0fb9b) )

	ROM_REGION( 0x400, KB_COP421_TAG, 0 )
	ROM_LOAD( "341-0064a.u9f", 0x000, 0x400, CRC(e6849910) SHA1(d46e67df75c9e3e773d20542fb9d5b1d2ac0fb9b) )

	ROM_REGION(0x2000,"fdccpu",0)       /* 6504 RAM and ROM */
	ROM_LOAD( "341-0290-b", 0x1000, 0x1000, CRC(bc6364f1) SHA1(f3164923330a51366a06d9d8a4a01ec7b0d3a8aa)) /* 341-0290-B LISA 2/5 Disk Rom (ioa8), supports profile on external port */

	ROM_REGION(0x4000,"profile", 0)     /* Profile/5 HDD */
	ROM_LOAD_OPTIONAL( "341-0080-b", 0x0000, 0x800, CRC(26df0b8d) SHA1(08f6689afb517e0a2bdaa48433003e62a66ae3c7)) /* 341-0080-B z8 mcu piggyback rom */

	/* TODO: the 341-0193-A parallel interface card rom should be loaded here as well for the lisa 1 and 2/5? */

	ROM_REGION(0x100,"gfx1",0)      /* video ROM (includes S/N) */
	ROM_LOAD( "vidstate.rom", 0x00, 0x100, CRC(75904783) SHA1(3b0023bd90f2ca1be0b099160a566b044856885d))
ROM_END

ROM_START( lisa210 ) /* newer motherboard and i/o board; has io88 i/o rom, built in widget hdd */
	ROM_REGION16_BE(0x204000,"maincpu", 0)  /* 68k rom and ram */
	ROM_DEFAULT_BIOS( "revh" )
	ROM_SYSTEM_BIOS( 0, "revh", "LISA Bootrom Rev H (2/24/84)")
	ROMX_LOAD( "341-0175-h", 0x000000, 0x2000, CRC(adfd4516) SHA1(97a89ce1218b8aa38f69f92f6f363f435c887914), ROM_SKIP(1) | ROM_BIOS(1)) /* 341-0175-H LISA Bootrom Rev H (2/24/84) (High) */
	ROMX_LOAD( "341-0176-h", 0x000001, 0x2000, CRC(546d6603) SHA1(2a81e4d483f50ae8a2519621daeb7feb440a3e4d), ROM_SKIP(1) | ROM_BIOS(1)) /* 341-0176-H LISA Bootrom Rev H (2/24/84) (Low) */

	ROM_SYSTEM_BIOS( 1, "revg", "LISA Bootrom Rev G (2/08/84)") /* limited test release before release of rom rev H */
	ROMX_LOAD( "341-0175-g", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(2)) /* 341-0175-G LISA Bootrom Rev G (2/08/84) (High) */
	ROMX_LOAD( "341-0176-g", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(2)) /* 341-0176-G LISA Bootrom Rev G (2/08/84) (Low) */

	ROM_SYSTEM_BIOS( 2, "revf", "LISA Bootrom Rev F (12/21/83)")
	ROMX_LOAD( "341-0175-f", 0x000000, 0x2000, CRC(701b9dab) SHA1(b116e5fada7b9a51f1b6e25757b2814d1b2737a5), ROM_SKIP(1) | ROM_BIOS(3)) /* 341-0175-F LISA Bootrom Rev F (12/21/83) (High) */
	ROMX_LOAD( "341-0176-f", 0x000001, 0x2000, CRC(036010b6) SHA1(ac93e6dbe4ce59396d7d191ee3e3e79a504e518f), ROM_SKIP(1) | ROM_BIOS(3)) /* 341-0176-F LISA Bootrom Rev F (12/21/83) (Low) */

	ROM_REGION( 0x400, COP421_TAG, 0 )
	ROM_LOAD( "341-0064a.u9f", 0x000, 0x400, CRC(e6849910) SHA1(d46e67df75c9e3e773d20542fb9d5b1d2ac0fb9b) )

	ROM_REGION( 0x400, KB_COP421_TAG, 0 )
	ROM_LOAD( "341-0064a.u9f", 0x000, 0x400, CRC(e6849910) SHA1(d46e67df75c9e3e773d20542fb9d5b1d2ac0fb9b) )

#if 1
	ROM_REGION(0x2000,"fdccpu", 0)      /* 6504 RAM and ROM */
	ROM_LOAD( "341-0281-d", 0x1000, 0x1000, CRC(e343fe74) SHA1(a0e484ead2d2315fca261f39fff2f211ff61b0ef)) /* 341-0281-D LISA 2/10 Disk Rom (io88), supports widget on internal port */
#else
	ROM_REGION(0x2000,"fdccpu", 0)      /* 6504 RAM and ROM */
	ROM_LOAD( "341-8003-c", 0x1000, 0x1000, CRC(8c67959a) SHA1(aa446b0c4acb4cb6c9d0adfbbea900fb8c04c1e9)) /* 341-8003-C Sun Mac XL Disk rom for 800k drives (Rev C, from Goodwill XL) (io88800k) */
	/* Note: there are two earlier/alternate versions of this rom as well which are dumped */
#endif

	ROM_REGION(0x4000,"widget", 0)      /* Widget HDD controller */
	ROM_LOAD( "341-0288-a", 0x0000, 0x800, CRC(a26ef1c6) SHA1(5aaeb6ff7f7d4f7ce7c70402f75e82533635dda4)) /* 341-0288-A z8 mcu piggyback rom */
	ROM_LOAD( "341-0289-d", 0x2000, 0x2000, CRC(25e86e95) SHA1(72a346c2074d2256adde491b930023ebdcb5f51a)) /* 341-0289-D external rom on widget board */

	ROM_REGION(0x100,"gfx1", 0)     /* video ROM (includes S/N) */
	ROM_LOAD( "vidstate.rom", 0x00, 0x100, CRC(75904783) SHA1(3b0023bd90f2ca1be0b099160a566b044856885d))
ROM_END

ROM_START( macxl )
	ROM_REGION16_BE(0x204000,"maincpu", 0)  /* 68k rom and ram */
	ROM_LOAD16_BYTE( "341-0347-a", 0x000000, 0x2000, CRC(80add605) SHA1(82215688b778d8c712a8186235f7981e3dc4dd7f)) /* 341-0347-A Mac XL '3A' Bootrom Hi (boot3a.hi)*/
	ROM_LOAD16_BYTE( "341-0346-a", 0x000001, 0x2000, CRC(edf5222f) SHA1(b0388ee8dbbc51a2d628473dc29b65ce913fcd76)) /* 341-0346-A Mac XL '3A' Bootrom Lo (boot3a.lo)*/

	ROM_REGION( 0x400, COP421_TAG, 0 )
	ROM_LOAD( "341-0064a.u9f", 0x000, 0x400, CRC(e6849910) SHA1(d46e67df75c9e3e773d20542fb9d5b1d2ac0fb9b) )

	ROM_REGION( 0x400, KB_COP421_TAG, 0 )
	ROM_LOAD( "341-0064a.u9f", 0x000, 0x400, CRC(e6849910) SHA1(d46e67df75c9e3e773d20542fb9d5b1d2ac0fb9b) )

#if 1
	ROM_REGION(0x2000,"fdccpu", 0)      /* 6504 RAM and ROM */
	ROM_LOAD( "341-0281-d", 0x1000, 0x1000, CRC(e343fe74) SHA1(a0e484ead2d2315fca261f39fff2f211ff61b0ef)) /* 341-0281-D LISA 2/10 Disk Rom (io88), supports widget on internal port */
#else
	ROM_REGION(0x2000,"fdccpu", 0)      /* 6504 RAM and ROM */
	ROM_LOAD( "341-8003-c", 0x1000, 0x1000, CRC(8c67959a) SHA1(aa446b0c4acb4cb6c9d0adfbbea900fb8c04c1e9)) /* 341-8003-C Sun Mac XL Disk rom for 800k drives (Rev C, from Goodwill XL) (io88800k) */
	/* Note: there are two earlier/alternate versions of this rom as well which are dumped */
#endif

	ROM_REGION(0x100,"gfx1", 0)     /* video ROM (includes S/N) ; no dump known, although Lisa ROM works fine at our level of emulation */
	ROM_LOAD( "vidstatem.rom", 0x00, 0x100, BAD_DUMP CRC(75904783) SHA1(3b0023bd90f2ca1be0b099160a566b044856885d))
ROM_END

/*
    Lisa drivers boot MacWorks, but do not boot the Lisa OS, which is why we set
    the MACHINE_NOT_WORKING flag...
*/
/*     YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT  INIT   COMPANY  FULLNAME */
COMP( 1983, lisa,     0,    0,  lisa,     lisa, lisa_state,  lisa2,         "Apple Computer",  "Lisa", MACHINE_NOT_WORKING )
COMP( 1984, lisa2,    0,    0,  lisa,     lisa, lisa_state,  lisa2,         "Apple Computer",  "Lisa2", MACHINE_NOT_WORKING )
COMP( 1984, lisa210,  lisa2,    0,  lisa210,  lisa, lisa_state,  lisa210,   "Apple Computer",  "Lisa2/10", MACHINE_NOT_WORKING )
COMP( 1985, macxl,    lisa2,    0,  macxl,    lisa, lisa_state,  mac_xl,    "Apple Computer",  "Macintosh XL", /*MACHINE_NOT_WORKING*/0 )
