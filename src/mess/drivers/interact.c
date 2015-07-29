// license:BSD-3-Clause
// copyright-holders:JJ Stacino
/***************************************************************************

        Interact Family Computer

        12/05/2009 Skeleton driver - Micko
    31/05/2009 Added notes - Robbbert

    This was made by Interact Company of Ann Arbor, Michigan. However,
    just after launch, the company collapsed. The liquidator, Protecto,
    sold some and MicroVideo sold the rest. MicroVideo continued to
    develop but went under 2 years later. Meanwhile, the French company
    Lambda Systems sold a clone called the Victor Lambda. But, like the
    Americans, Lambda Systems also collapsed. Another French company,
    Micronique, purchased all remaining stock and intellectual rights
    from Lambda Systems, Microvideo and Interact, and the computer becomes
    wholly French. The computer has a name change, becoming the Hector.
    This in turn gets upgraded (2HR, HRX, MX). The line is finally
    retired in about 1985.

        Hector 2HR+
        Victor
        Hector 2HR
        Hector HRX
        Hector MX40c
        Hector MX80c
        Hector 1
        Interact

        12/05/2009 Skeleton driver - Micko : mmicko@gmail.com
        31/06/2009 Video - Robbbert

        29/10/2009 Update skeleton to functional machine
                    by yo_fr            (jj.stac @ aliceadsl.fr)

                => add Keyboard,
                => add color,
                => add cassette,
                => add sn76477 sound and 1bit sound,
                => add joysticks (stick, pot, fire)
                => add BR/HR switching
                => add bank switch for HRX
                => add device MX80c and bank switching for the ROM
    Importante note : the keyboard function add been piked from
                    DChector project : http://dchector.free.fr/ made by DanielCoulom
                    (thank's Daniel)
    TODO :  Add the cartridge function,
            Adjust the one shot and A/D timing (sn76477)
****************************************************************************/
/* Mapping for joystick see hec2hrp.c*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "formats/hect_tap.h"
#include "imagedev/printer.h"
#include "sound/wave.h"      /* for K7 sound*/
#include "sound/discrete.h"  /* for 1 Bit sound*/
#include "machine/upd765.h" /* for floppy disc controller */

#include "includes/hec2hrp.h"


class interact_state : public hec2hrp_state
{
public:
	interact_state(const machine_config &mconfig, device_type type, const char *tag)
		: hec2hrp_state(mconfig, type, tag),
			m_videoram(*this, "videoram") { }

	required_shared_ptr<UINT8> m_videoram;
	DECLARE_MACHINE_START(interact);
	DECLARE_MACHINE_RESET(interact);
	UINT32 screen_update_interact(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


static ADDRESS_MAP_START(interact_mem, AS_PROGRAM, 8, interact_state )
	ADDRESS_MAP_UNMAP_HIGH
	/* Hardware address mapping*/
/*  AM_RANGE(0x0800,0x0808) AM_WRITE(hector_switch_bank_w)// Bank management not udsed in BR machine*/
	AM_RANGE(0x1000,0x1000) AM_WRITE(hector_color_a_w)  /* Color c0/c1*/
	AM_RANGE(0x1800,0x1800) AM_WRITE(hector_color_b_w)  /* Color c2/c3*/
	AM_RANGE(0x2000,0x2003) AM_WRITE(hector_sn_2000_w)  /* Sound*/
	AM_RANGE(0x2800,0x2803) AM_WRITE(hector_sn_2800_w)  /* Sound*/
	AM_RANGE(0x3000,0x3000) AM_READWRITE(hector_cassette_r, hector_sn_3000_w)/* Write necessary*/
	AM_RANGE(0x3800,0x3807) AM_READWRITE(hector_keyboard_r, hector_keyboard_w)  /* Keyboard*/

	/* Main ROM page*/
	AM_RANGE(0x0000,0x3fff) AM_ROM  /*BANK(2)*/
	/*   AM_RANGE(0x1000,0x3fff) AM_RAM*/

	/* Video br mapping*/
	AM_RANGE(0x4000,0x49ff) AM_RAM AM_SHARE("videoram")
	/* continous RAM*/
	AM_RANGE(0x4A00,0xffff) AM_RAM

ADDRESS_MAP_END


MACHINE_RESET_MEMBER(interact_state,interact)
{
	hector_reset(0, 0);
}

MACHINE_START_MEMBER(interact_state,interact)
{
	hector_init();
}

UINT32 interact_state::screen_update_interact(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram;
	screen.set_visible_area(0, 113, 0, 75);
	hector_hr(bitmap, videoram,  77, 32);
	return 0;
}

static MACHINE_CONFIG_START( interact, interact_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(interact_mem)
	MCFG_CPU_PERIODIC_INT_DRIVER(interact_state, irq0_line_hold, 50) /*  put on the I8080 irq in Hz*/

	MCFG_MACHINE_RESET_OVERRIDE(interact_state,interact)
	MCFG_MACHINE_START_OVERRIDE(interact_state,interact)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(256, 79)
	MCFG_SCREEN_VISIBLE_AREA(0, 112, 0, 77)
	MCFG_SCREEN_UPDATE_DRIVER(interact_state, screen_update_interact)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)             /* 8 colours, but only 4 at a time*/

	MCFG_VIDEO_START_OVERRIDE(interact_state,hec2hrp)

	MCFG_FRAGMENT_ADD(hector_audio)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(hector_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MASK_SPEAKER)
	MCFG_CASSETTE_INTERFACE("interact_cass")

	MCFG_SOFTWARE_LIST_ADD("cass_list","interact")

	/* printer */
	MCFG_DEVICE_ADD("printer", PRINTER, 0)

MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hector1, interact_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_1_75MHz)
	MCFG_CPU_PROGRAM_MAP(interact_mem)
	MCFG_CPU_PERIODIC_INT_DRIVER(interact_state, irq0_line_hold, 50) /*  put on the I8080 irq in Hz*/

	MCFG_MACHINE_RESET_OVERRIDE(interact_state,interact)
	MCFG_MACHINE_START_OVERRIDE(interact_state,interact)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(256, 79)
	MCFG_SCREEN_VISIBLE_AREA(0, 112, 0, 77)
	MCFG_SCREEN_UPDATE_DRIVER(interact_state, screen_update_interact)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)             /* 8 colours, but only 4 at a time*/

	MCFG_VIDEO_START_OVERRIDE(interact_state,hec2hrp)

	MCFG_FRAGMENT_ADD(hector_audio)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(hector_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MASK_SPEAKER)
	MCFG_CASSETTE_INTERFACE("interact_cass")

	/* printer */
	MCFG_DEVICE_ADD("printer", PRINTER, 0)

MACHINE_CONFIG_END

/* Input ports */
static INPUT_PORTS_START( interact )
	/* keyboard input */
	PORT_START("KEY.0") /* [0] - port 3000 @ 0 */
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR('\\') PORT_CHAR('|')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space")          PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return")         PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab")            PORT_CODE(KEYCODE_TAB)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("<--")            PORT_CODE(KEYCODE_BACKSPACE)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock")      PORT_CODE(KEYCODE_CAPSLOCK)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl")           PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift")          PORT_CODE(KEYCODE_LSHIFT)     PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_START("KEY.1") /* [1] - port 3000 @ 1 */    /* touches => 2  1  0  /  .  -  ,  +     */
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 \"")           PORT_CODE(KEYCODE_2)    PORT_CHAR('2')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 >")            PORT_CODE(KEYCODE_1)    PORT_CHAR('1')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 <")            PORT_CODE(KEYCODE_0)    PORT_CHAR('0')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)     PORT_CHAR('.') PORT_CHAR('>')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)    PORT_CHAR(',') PORT_CHAR('<')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)    PORT_CHAR('-') PORT_CHAR('_')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)   PORT_CHAR('=') PORT_CHAR('+')

	PORT_START("KEY.2") /* [1] - port 3000 @ 2 */     /* touches => .. 9  8  7  6  5  4  3  */
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 )")            PORT_CODE(KEYCODE_9)    PORT_CHAR('9')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 (")            PORT_CODE(KEYCODE_8)    PORT_CHAR('8')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 :")            PORT_CODE(KEYCODE_7)    PORT_CHAR('7')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 !")            PORT_CODE(KEYCODE_6)    PORT_CHAR('6')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 %")            PORT_CODE(KEYCODE_5)    PORT_CHAR('5')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $")            PORT_CODE(KEYCODE_4)    PORT_CHAR('4')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 /")            PORT_CODE(KEYCODE_3)    PORT_CHAR('3')
	PORT_START("KEY.3") /* [1] - port 3000 @ 3 */    /* touches =>  B  A  ..  ? .. =   ..  ;       */
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)     PORT_CHAR('B')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")           PORT_CODE(KEYCODE_Q)   PORT_CHAR('Q')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('[') PORT_CHAR('{')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(']') PORT_CHAR('}')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('/') PORT_CHAR('?')
	PORT_START("KEY.4") /* [1] - port 3000 @ 4 */
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)            PORT_CHAR('J')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)            PORT_CHAR('I')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)            PORT_CHAR('H')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)            PORT_CHAR('G')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)            PORT_CHAR('F')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)            PORT_CHAR('E')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)            PORT_CHAR('C')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)            PORT_CHAR('D')

	PORT_START("KEY.5") /* [1] - port 3000 @ 5 */
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)            PORT_CHAR('R')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")         PORT_CODE(KEYCODE_A)         PORT_CHAR('A')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)            PORT_CHAR('P')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)            PORT_CHAR('O')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)            PORT_CHAR('N')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)            PORT_CHAR('L')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)            PORT_CHAR('K')

	PORT_START("KEY.6") /* [1] - port 3000 @ 6 */
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")          PORT_CODE(KEYCODE_W)            PORT_CHAR('W')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)            PORT_CHAR('Y')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X")          PORT_CODE(KEYCODE_X)            PORT_CHAR('X')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")          PORT_CODE(KEYCODE_Z)            PORT_CHAR('Z')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)            PORT_CHAR('V')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)            PORT_CHAR('U')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)            PORT_CHAR('T')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)            PORT_CHAR('S')

	PORT_START("KEY.7") /* [1] - port 3000 @ 7  JOYSTICK */
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Joy(0) LEFT")        PORT_CODE(KEYCODE_LEFT)     PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Joy(0) RIGHT")       PORT_CODE(KEYCODE_RIGHT)    PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Joy(0) UP")          PORT_CODE(KEYCODE_UP)       PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Joy(0) DOWN")        PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Joy(1) LEFT")        PORT_CODE(KEYCODE_1_PAD)     /* Joy(1) on numpad*/
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Joy(1) RIGHT")       PORT_CODE(KEYCODE_3_PAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Joy(1) UP")          PORT_CODE(KEYCODE_5_PAD)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Joy(1) DOWN")        PORT_CODE(KEYCODE_2_PAD)

	PORT_START("KEY.8") /* [1] - port 3000 @ 8  not for the real machine, but to emulate the analog signal of the joystick */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")             PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
		PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYBOARD) PORT_NAME("Joy(0) FIRE")       PORT_CODE(KEYCODE_0_PAD)
		PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYBOARD) PORT_NAME("Joy(1) FIRE")       PORT_CODE(KEYCODE_PLUS_PAD)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Pot(0)+") PORT_CODE(KEYCODE_INSERT)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Pot(0)-") PORT_CODE(KEYCODE_DEL)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Pot(1)+") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Pot(1)-") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

/* ROM definition */
ROM_START( interact )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "interact.rom", 0x0000, 0x0800, CRC(1aa50444) SHA1(405806c97378abcf7c7b0d549430c78c7fc60ba2))
ROM_END

ROM_START( hector1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "hector1.rom",  0x0000, 0x1000, CRC(3be6628b) SHA1(1c106d6732bed743d8283d39e5b8248271f18c42))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP(1979, interact, 0,         0,  interact,   interact, driver_device, 0,  "Interact",   "Interact Family Computer", MACHINE_IMPERFECT_SOUND)
COMP(1983, hector1,  interact,  0,   hector1,   interact, driver_device, 0,  "Micronique", "Hector 1",  MACHINE_IMPERFECT_SOUND)
