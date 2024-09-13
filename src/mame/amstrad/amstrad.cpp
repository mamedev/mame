// license:GPL-2.0+
// copyright-holders:Kevin Thacker, Barry Rodewald
/******************************************************************************

    amstrad.cpp
    system driver

        Amstrad Hardware:
            - 8255 connected to AY-3-8912 sound generator,
                keyboard, cassette, printer, crtc vsync output,
                and some PCB links
            - 6845 (either HD6845S, UM6845R or M6845) crtc graphics display
              controller
            - UPD765 floppy disc controller (CPC664,CPC6128)
            - Z80 CPU running at 4 MHz (slowed by wait states on memory
              access)
            - custom ASIC "Gate Array" controlling rom paging, ram paging,
                current display mode and colour palette

    Kevin Thacker [MESS driver]

  May-June 2004 - Yoann Courtois (aka Papagayo/ex GMC/ex PARADOX) - rewriting some code with hardware documentation from http://www.cepece.info/amstrad/

  June 2006  - Very preliminary CPC+ support.  CPR cart image handling, secondary ROM register, ASIC unlock detection
               Supported:
               12-bit palette,
               12-bit hardware sprites (largely works from what I've seen, some games have display issues),
               Programmable Raster Interrupt (seems to work),
               Split screen registers,
               Soft scroll registers (only byte-by-byre horizontally for now),
               Analogue controls (may well be completely wrong, I have no idea on how these should work),
               Vectored interrupts for Z80 interrupt mode 2 (used by Pang),
               DMA sound channels (may still be some issues, noticeable in Navy Seals and Copter 271)
               04/07/06:  Added interrupt vector support for IM 2.
                          Added soft scroll register implementation.  Vertical adjustments are a bit shaky.
               05/07/06:  Fixed hardware sprite offsets
               14/07/06:  Added basic analogue control support.
               04/08/06:  Fixed PRI and Split screen scanline offsets (based on code in Arnold ;))
                          Implemented DMA sound channels
               06/08/06:  Fixed CRTC palette if the ASIC was re-locked after already being unlocked and used.
                          This fixes Klax, which is now playable.
               08/08/06:  Fixed up vertical soft scroll, now we just need to get a finer detail on horizontal soft scroll
                          (Only works on a byte level for now)
                          Fixed DMA pause function when the prescaler is set to 0.

               Tested with the Arnold 5 Diagnostic Cartridge.  Mostly works fine, but the soft scroll test is
               noticeably wrong.

               Known issues with some games (as at 12/01/08):
               Robocop 2:  playable, but sprites should be cut off outside the playing area (partial updates should be able to fix this).
               Navy Seals:  Playable, but sound effects cut out at times (probably DMA related).
               Dick Tracy:  Sprite visibility issues
               Switchblade:  has some slowdown when numerous enemies are on screen (normal?)
               Epyx World of Sports: doesn't start at all.
               Tennis Cup II:  controls don't seem to work.
               Fire and Forget II:  playable, but the top half of the screen flickers
               Crazy Cars II:  playable, with slight shaking of horizon
               No Exit:  Display is wrong, but usable, uses demo-like techniques.


   January 2008 - added preliminary Aleste 520EX support
               The Aleste 520EX is a Russian clone of the CPC6128, that expands on existing video modes, and can run MSX-DOS.
               It also adds an MC146818 RTC/NVRAM, an Intel 8253 timer, and the "Magic Sound" board, a 4-channel DMA-based
               sample player.  Also includes a software emulation of the MSX2 VDP, used in the ports of MSX games.

               Known issues:
                - The RTC doesn't always update in setup.  It expects bit 4 of register C (Update End Interrupt) to be high.
                - Title screens appear then disappear quickly (probably because the 8253 hasn't been plugged in yet).
                - Some video modes display wrong (still needs some work here).
                - Vampire Killer crashes after collecting a certain key part way through stage 1.
                - Magic Sound isn't emulated.


   January 2009 - changed drivers to use the mc6845 device implementation
               To get rid of duplicated code the drivers have been changed to use the new mc6845 device
               implementation. As a result the (runtime) selection of CRTC type has been removed.

    July 2011 - added basic expansion port interface, with support for both the Amstrad SSA-1 and DK'Tronics
                speech synthesisers.


Some bugs left :
----------------
    - CRTC all type support (0,1,2,3,4) ?
    - Gate Array and CRTC aren't synchronised. (The Gate Array can change the color every microseconds?) So the multi-rasters in one line aren't supported (see yao demo p007's part)!
    - Implement full Asic for CPC+ emulation.  Soft scroll is rather dodgy.
    - The KC Compact should not reuse the gate array functionality. Instead z8536 support should be added. (bug #42)

******************************************************************************/

/* Core includes */
#include "emu.h"
#include "amstrad.h"

/* Components */
#include "machine/i8255.h"     // for 8255 ppi
#include "cpu/z80/z80.h"       // for cycle tables
#include "video/mc6845.h"      // CRTC
#include "machine/upd765.h"    // for floppy disc controller
#include "sound/ay8910.h"
#include "machine/mc146818.h"  // Aleste RTC
#include "bus/centronics/ctronics.h"

/* Devices */
#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "formats/tzx_cas.h"
#include "formats/msx_dsk.h"
#include "formats/ipf_dsk.h"

#include "machine/ram.h"
#include "softlist.h"
#include "speaker.h"
#include "utf8.h"

#define MANUFACTURER_NAME 0x07
#define TV_REFRESH_RATE 0x10

#define SYSTEM_CPC    0
#define SYSTEM_PLUS   1
#define SYSTEM_GX4000 2


/* Memory is banked in 16k blocks. However, the multiface pages the memory in 8k blocks!
   The ROM can be paged into bank 0 and bank 3. */
void amstrad_state::amstrad_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(amstrad_state::amstrad_cpc_mem_r), FUNC(amstrad_state::amstrad_cpc_mem_w));
}

/* I've handled the I/O ports in this way, because the ports
are not fully decoded by the CPC h/w. Doing it this way means
I can decode it myself and a lot of  software should work */
void amstrad_state::amstrad_io(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(amstrad_state::amstrad_cpc_io_r), FUNC(amstrad_state::amstrad_cpc_io_w));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( amstrad_keyboard )
	PORT_START("kbrow.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP)                 PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)              PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)               PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 9")              PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 6")              PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 3")              PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad Enter")          PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad .")              PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_START("kbrow.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)               PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Copy")                  PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 7")              PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 8")              PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 5")              PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 1")              PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 2")              PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 0")              PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))

	PORT_START("kbrow.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Clr")                   PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[")                     PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter")                 PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]")                     PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 4")              PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift")                 PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\")                    PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR('\\') PORT_CHAR('`')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl")                  PORT_CODE(KEYCODE_RALT)       PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START("kbrow.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2191 £")            PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('^') PORT_CHAR(U'£') // U+2191 = ↑
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('@') PORT_CHAR(U'¦','|')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COLON)      PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("kbrow.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR('_')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("kbrow.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space")                 PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(32)

	PORT_START("kbrow.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("kbrow.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("kbrow.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('\"') PORT_CHAR('~')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc")                   PORT_CODE(KEYCODE_TILDE)      PORT_CHAR(27)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab")                   PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock")             PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) // No physical toggle
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("kbrow.9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(1) PORT_8WAY PORT_CONDITION("controller_type", 0x01, EQUALS, 0x01)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(1) PORT_8WAY PORT_CONDITION("controller_type", 0x01, EQUALS, 0x01)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(1) PORT_8WAY PORT_CONDITION("controller_type", 0x01, EQUALS, 0x01)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1) PORT_8WAY PORT_CONDITION("controller_type", 0x01, EQUALS, 0x01)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(1) PORT_CONDITION("controller_type", 0x01, EQUALS, 0x01)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2)        PORT_PLAYER(1) PORT_CONDITION("controller_type", 0x01, EQUALS, 0x01)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
/* The bit for the third button is officially undocumented: Amstrad joysticks actually
   use only two buttons. The only device that reads this bit is the AMX mouse, which uses
   dedicated hardware to connect to the joystick port.
*/
//  PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3)        PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del")                   PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(8)

/* Second joystick port need to be defined: the second joystick is daisy-chained on the back of the first one
   Joystick 2 is mapped to keyboard line 6.  */

/* The CPC supports at least two different brands of lightpens, probably connected to the expansion port */

INPUT_PORTS_END


/* Steph 2000-10-27 I remapped the 'Machine Name' Dip Switches (easier to understand) */
INPUT_CHANGED_MEMBER(amstrad_state::cpc_monitor_changed)
{
	if ((m_io_green_display->read()) & 0x01)
		amstrad_cpc_green_palette(*m_palette);
	else
		amstrad_cpc_palette(*m_palette);
}


static INPUT_PORTS_START( crtc_links )

/* the following are solder links on the circuit board. The links are open or closed when the
 * PCB is made, and are set depending on which country the Amstrad system was to go to
 * (reference: http://amstrad.cpc.free.fr/article.php?sid=26)

lk1 lk2 lk3 Manufacturer Name (CPC and CPC+ only):

0   0   0   Isp
0   0   1   Triumph (UK?)
0   1   0   Saisho (UK: Saisho is Dixons brand name for their electronic goods)
0   1   1   Solavox
1   0   0   Awa (Australia)
1   0   1   Schneider (Germany)
1   1   0   Orion (Japan?)
1   1   1   Amstrad (UK)

lk4     Frequency
0       60 Hz
1       50 Hz
*/
	PORT_START("solder_links")
	PORT_CONFNAME(0x07, 0x07, "Manufacturer Name")
	PORT_CONFSETTING(0x00, "Isp")
	PORT_CONFSETTING(0x01, "Triumph")
	PORT_CONFSETTING(0x02, "Saisho")
	PORT_CONFSETTING(0x03, "Solavox")
	PORT_CONFSETTING(0x04, "Awa")
	PORT_CONFSETTING(0x05, "Schneider")
	PORT_CONFSETTING(0x06, "Orion")
	PORT_CONFSETTING(0x07, "Amstrad")
	PORT_CONFNAME(0x10, 0x10, "TV Refresh Rate")
	PORT_CONFSETTING(0x00, "60 Hz")
	PORT_CONFSETTING(0x10, "50 Hz")

/* Part number Manufacturer Type number
   UM6845      UMC          0
   HD6845S     Hitachi      0
   UM6845R     UMC          1
   MC6845      Motorola     2
   AMS40489    Amstrad      3
   Pre-ASIC??? Amstrad?     4 In the "cost-down" CPC6128, the CRTC functionality is integrated into a single ASIC IC. This ASIC is often referred to as the "Pre-ASIC" because it preceded the CPC+ ASIC
As far as I know, the KC compact used HD6845S only.
*/
//  PORT_START("crtc")
//  PORT_CONFNAME( 0xFF, M6845_PERSONALITY_UM6845R, "CRTC Type")
//  PORT_CONFSETTING(M6845_PERSONALITY_UM6845, "Type 0 - UM6845")
//  PORT_CONFSETTING(M6845_PERSONALITY_HD6845S, "Type 0 - HD6845S")
//  PORT_CONFSETTING(M6845_PERSONALITY_UM6845R, "Type 1 - UM6845R")
//  PORT_CONFSETTING(M6845_PERSONALITY_GENUINE, "Type 2 - MC6845")
//  PORT_CONFSETTING(M6845_PERSONALITY_AMS40489, "Type 3 - AMS40489")
//  PORT_CONFSETTING(M6845_PERSONALITY_PREASIC, "Type 4 - Pre-ASIC")

	PORT_START("green_display")
	PORT_CONFNAME( 0x01, 0x00, "Monitor" ) PORT_CHANGED_MEMBER(DEVICE_SELF, amstrad_state,  cpc_monitor_changed, 0 )
	PORT_CONFSETTING(0x00, "CTM640 Colour Monitor" )
	PORT_CONFSETTING(0x01, "GT64 Green Monitor" )

INPUT_PORTS_END

static INPUT_PORTS_START( amx_mouse )
	PORT_START("mouse_input1")
	PORT_BIT(0xff , 0, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1) PORT_CONDITION("controller_type", 0x02, EQUALS, 0x02)

	PORT_START("mouse_input2")
	PORT_BIT(0xff , 0, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1) PORT_CONDITION("controller_type", 0x02, EQUALS, 0x02)

	PORT_START("mouse_input3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("Left mouse button")   PORT_CODE(MOUSECODE_BUTTON1) PORT_CONDITION("controller_type", 0x02, EQUALS, 0x02)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("Right mouse button")  PORT_CODE(MOUSECODE_BUTTON2) PORT_CONDITION("controller_type", 0x02, EQUALS, 0x02)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("Middle mouse button") PORT_CODE(MOUSECODE_BUTTON3) PORT_CONDITION("controller_type", 0x02, EQUALS, 0x02)

	PORT_START("controller_type")
	PORT_CONFNAME( 0x07, 0x01, "Joystick port device" )
	PORT_CONFSETTING(0x00, "Nothing" )
	PORT_CONFSETTING(0x01, "2-button joystick" )
	PORT_CONFSETTING(0x02, "AMX mouse interface" )
	PORT_CONFSETTING(0x04, "Cheetah 125 Special rotational joystick" )

INPUT_PORTS_END

static INPUT_PORTS_START( cheetah_125_special )
	PORT_START("cheetah")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(1) PORT_8WAY PORT_NAME("Cheetah 125 Up") PORT_CONDITION("controller_type", 0x04, EQUALS, 0x04)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(1) PORT_8WAY PORT_NAME("Cheetah 125 Down") PORT_CONDITION("controller_type", 0x04, EQUALS, 0x04)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(1) PORT_8WAY PORT_NAME("Cheetah 125 Left") PORT_CONDITION("controller_type", 0x04, EQUALS, 0x04)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1) PORT_8WAY PORT_NAME("Cheetah 125 Right") PORT_CONDITION("controller_type", 0x04, EQUALS, 0x04)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(1) PORT_NAME("Cheetah 125 Fire") PORT_CONDITION("controller_type", 0x04, EQUALS, 0x04)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2)        PORT_PLAYER(1) PORT_NAME("Cheetah 125 Rotate Left") PORT_CONDITION("controller_type", 0x04, EQUALS, 0x04)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3)        PORT_PLAYER(1) PORT_NAME("Cheetah 125 Rotate Right") PORT_CONDITION("controller_type", 0x04, EQUALS, 0x04)
INPUT_PORTS_END

static INPUT_PORTS_START( cpc464 )
	PORT_INCLUDE(amstrad_keyboard)
	PORT_INCLUDE(crtc_links)
	PORT_INCLUDE(amx_mouse)
	PORT_INCLUDE(cheetah_125_special)
INPUT_PORTS_END


static INPUT_PORTS_START( cpc664 )
	PORT_INCLUDE(amstrad_keyboard)

	PORT_MODIFY("kbrow.0")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad f9")             PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad f6")             PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad f3")             PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))

	PORT_MODIFY("kbrow.1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad f7")             PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad f8")             PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad f5")             PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad f1")             PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad f2")             PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad f0")             PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))

	PORT_MODIFY("kbrow.2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad f4")             PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))

	PORT_INCLUDE(crtc_links)
	PORT_INCLUDE(amx_mouse)
	PORT_INCLUDE(cheetah_125_special)
INPUT_PORTS_END


static INPUT_PORTS_START( cpc6128 )
	PORT_INCLUDE(amstrad_keyboard)

	PORT_MODIFY("kbrow.0")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f9")                    PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f6")                    PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f3")                    PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter")                 PORT_CODE(KEYCODE_RALT)       PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".")                     PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_MODIFY("kbrow.1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Copy")                  PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f7")                    PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f8")                    PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f5")                    PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f1")                    PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f2")                    PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f0")                    PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))

	PORT_MODIFY("kbrow.2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return")                PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f4")                    PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR('\\') PORT_CHAR('`')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Control")               PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_SHIFT_2)

	PORT_INCLUDE(crtc_links)
	PORT_INCLUDE(amx_mouse)
	PORT_INCLUDE(cheetah_125_special)
INPUT_PORTS_END


/* This CPC6128 sold in France has the AZERTY layout. Reference: http://amstrad.cpc.free.fr/amstrad/cpc6128.htm */
static INPUT_PORTS_START( cpc6128f )
	PORT_INCLUDE(amstrad_keyboard)

	PORT_MODIFY("kbrow.0")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f9")                    PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f6")                    PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f3")                    PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter")                 PORT_CODE(KEYCODE_RALT)       PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".")                     PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_MODIFY("kbrow.1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Copy")                  PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f7")                    PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f8")                    PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f5")                    PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f1")                    PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f2")                    PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f0")                    PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))

	PORT_MODIFY("kbrow.2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('*') PORT_CHAR('<')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return")                PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('#') PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f4")                    PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR('$') PORT_CHAR('@') PORT_CHAR('\\')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Control")               PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_SHIFT_2)

	PORT_MODIFY("kbrow.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(')') PORT_CHAR('[')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('^') PORT_CHAR(U'¦','|')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(U'ù') PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COLON)      PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_STOP)       PORT_CHAR(':') PORT_CHAR('/')

	PORT_MODIFY("kbrow.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_0)          PORT_CHAR(U'à') PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_9)          PORT_CHAR(U'ç') PORT_CHAR('9')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_M)          PORT_CHAR(',') PORT_CHAR('?')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(';') PORT_CHAR('.')

	PORT_MODIFY("kbrow.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_8)          PORT_CHAR('!') PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_7)          PORT_CHAR(U'è') PORT_CHAR('7')

	PORT_MODIFY("kbrow.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_6)          PORT_CHAR(']') PORT_CHAR('6')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_5)          PORT_CHAR('(') PORT_CHAR('5')

	PORT_MODIFY("kbrow.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_4)          PORT_CHAR('\'') PORT_CHAR('4')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_3)          PORT_CHAR('\"') PORT_CHAR('3')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_W)          PORT_CHAR('z') PORT_CHAR('Z')

	PORT_MODIFY("kbrow.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_1)          PORT_CHAR('&') PORT_CHAR('1')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_2)          PORT_CHAR(U'é') PORT_CHAR('2') PORT_CHAR('~')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Q)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_A)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Z)          PORT_CHAR('w') PORT_CHAR('W')

	PORT_INCLUDE(crtc_links)
	PORT_INCLUDE(amx_mouse)
	PORT_INCLUDE(cheetah_125_special)
INPUT_PORTS_END

static INPUT_PORTS_START( cpc6128s )
	PORT_INCLUDE(cpc6128)

	PORT_MODIFY("kbrow.2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(U'Ü')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(U'É')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR('/') PORT_CHAR('?')

	PORT_MODIFY("kbrow.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('+') PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(U'Å')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(U'Ä')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COLON)      PORT_CHAR(U'Ö')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR(':')

	PORT_MODIFY("kbrow.4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR(';')
INPUT_PORTS_END

static INPUT_PORTS_START( cpc6128sp )
	PORT_INCLUDE(cpc6128)

	PORT_MODIFY("kbrow.2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[")                     PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]")                     PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(']') PORT_CHAR('+')

	PORT_MODIFY("kbrow.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('^') PORT_CHAR(U'\u20a7') // U+20A7 = ₧ (Peseta)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COLON)      PORT_CHAR(U'ñ') PORT_CHAR(U'Ñ')
INPUT_PORTS_END

/*
 * The BIOS of the KC Compact would be able to recognize the keypresses
 * generated by F5-F9. Unfortunately these keys are not present on the
 * keyboard! Reference: http://www.cepece.info/amstrad/docs/kcc/kcc01.jpg
 *
 */
static INPUT_PORTS_START( kccomp )
	PORT_INCLUDE(amstrad_keyboard)

	PORT_MODIFY("kbrow.0")
	PORT_BIT(0x08, 0x08, IPT_UNUSED)
	PORT_BIT(0x10, 0x10, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f3")                    PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter")                 PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".")                     PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_MODIFY("kbrow.1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Copy")                  PORT_CODE(KEYCODE_RALT)       PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x04, 0x04, IPT_UNUSED)
	PORT_BIT(0x08, 0x08, IPT_UNUSED)
	PORT_BIT(0x10, 0x10, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f1")                    PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f2")                    PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f0")                    PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))

	PORT_MODIFY("kbrow.2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return")                PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f4")                    PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR('\\') PORT_CHAR('`')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Control")               PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_SHIFT_2)

	PORT_INCLUDE(crtc_links)
	PORT_INCLUDE(amx_mouse)
	PORT_INCLUDE(cheetah_125_special)
INPUT_PORTS_END


static INPUT_PORTS_START( plus )
	PORT_INCLUDE(cpc664)

	/* The CPC+ and GX4000 adds support for analogue controllers.
	   Up to two joysticks or four paddles can be used, although the ASIC supports twice that.
	   Read at &6808-&680f in ASIC RAM
	   I am unsure if these are even close to correct.

	UPDATE: the analog port supports (probably with an Y-cable) up to two analog joysticks
	with two buttons each or four paddles with one button each. The CPC+/GX4000 have also an
	AUX port which supports a lightgun/lightpen with two buttons.
	Since all these devices have their dedicated connector, two digital joystick, two analog joysticks
	and one lightgun can be connected at the same moment.

	The connectors' description for both CPCs and CPC+'s can be found at http://www.hardwarebook.info/Category:Computer */

	PORT_START("analog.0")
	PORT_BIT(0x3f , 0, IPT_AD_STICK_X)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_PLAYER(1)

	PORT_START("analog.1")
	PORT_BIT(0x3f , 0, IPT_AD_STICK_Y)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_PLAYER(1)

	PORT_START("analog.2")
	PORT_BIT(0x3f , 0, IPT_AD_STICK_X)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_PLAYER(2)

	PORT_START("analog.3")
	PORT_BIT(0x3f , 0, IPT_AD_STICK_Y)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_PLAYER(2)

// Not used, but are here for completeness
	PORT_START("analog.4")
	PORT_BIT(0x3f , 0, IPT_TRACKBALL_X)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_PLAYER(3)

	PORT_START("analog.5")
	PORT_BIT(0x3f , 0, IPT_TRACKBALL_Y)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_PLAYER(3)

	PORT_START("analog.6")
	PORT_BIT(0x3f , 0, IPT_TRACKBALL_X)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_PLAYER(4)

	PORT_START("analog.7")
	PORT_BIT(0x3f , 0, IPT_TRACKBALL_Y)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_PLAYER(4)
INPUT_PORTS_END


static INPUT_PORTS_START( gx4000 )

	// The GX4000 is a console, so no keyboard access other than the joysticks.
	PORT_START("kbrow.0")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kbrow.1")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kbrow.2")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kbrow.3")
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME(DEF_STR(Pause)) PORT_CODE(KEYCODE_1)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kbrow.4")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kbrow.5")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kbrow.6")  // Joystick 2
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2)        PORT_PLAYER(2)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kbrow.7")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kbrow.8")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kbrow.9")  // Joystick 1
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2)        PORT_PLAYER(1)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(crtc_links)  // included to keep the driver happy

	PORT_START("analog.0")
	PORT_BIT(0x3f , 0, IPT_TRACKBALL_X)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_PLAYER(1)

	PORT_START("analog.1")
	PORT_BIT(0x3f , 0, IPT_TRACKBALL_Y)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_PLAYER(1)

	PORT_START("analog.2")
	PORT_BIT(0x3f , 0, IPT_TRACKBALL_X)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_PLAYER(2)

	PORT_START("analog.3")
	PORT_BIT(0x3f , 0, IPT_TRACKBALL_Y)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_PLAYER(2)

// Not used, but are here for completeness
	PORT_START("analog.4")
	PORT_BIT(0x3f , 0, IPT_TRACKBALL_X)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_PLAYER(3)

	PORT_START("analog.5")
	PORT_BIT(0x3f , 0, IPT_TRACKBALL_Y)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_PLAYER(3)

	PORT_START("analog.6")
	PORT_BIT(0x3f , 0, IPT_TRACKBALL_X)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_PLAYER(4)

	PORT_START("analog.7")
	PORT_BIT(0x3f , 0, IPT_TRACKBALL_Y)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_PLAYER(4)
INPUT_PORTS_END


static INPUT_PORTS_START( aleste )
	PORT_INCLUDE(amstrad_keyboard)

	PORT_MODIFY( "kbrow.9" )
	/* Documentation marks this input as "R/L", it's purpose is unknown - I can't even find it on the keyboard */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R  L")      PORT_CODE(KEYCODE_PGUP)

	PORT_START( "kbrow.10" )
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1  F6")    PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2  F7")    PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3  F8")    PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4  F9")    PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5  F10")   PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("HELP")      PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INS")       PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ъ  _")    PORT_CODE(KEYCODE_HOME)

	PORT_INCLUDE(crtc_links)
	PORT_INCLUDE(amx_mouse)
	PORT_INCLUDE(cheetah_125_special)
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

/* actual clock to CPU is 4 MHz, but it is slowed by memory
accessess. A HALT is used for every memory access by the CPU.
This stretches the timing for opcodes, and gives an effective
speed of 3.8 MHz */

/* Info about structures below:

    The Amstrad has a CPU running at 4 MHz, slowed with wait states.
    I have measured 19968 NOP instructions per frame, which gives,
    50.08 fps as the tv refresh rate.

  There are 312 lines on a PAL screen, giving 64us per line.

  There is only 50us visible per line, and 35*8 lines visible on the screen.

  This is the reason why the displayed area is not the same as the visible area.
 */


static void amstrad_floppies(device_slot_interface &device)
{
	device.option_add("3ssdd", FLOPPY_3_SSDD);
	device.option_add("35ssdd", FLOPPY_35_DD);
}

void amstrad_state::amstrad_floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_IPF_FORMAT);
}

static void aleste_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

void amstrad_state::aleste_floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_MSX_FORMAT);
}

void amstrad_state::cpcplus_cartslot(machine_config &config)
{
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "gx4000_cart", "bin,cpr"));
	cartslot.set_must_be_loaded(true);
	cartslot.set_device_load(FUNC(amstrad_state::amstrad_plus_cartridge));

	SOFTWARE_LIST(config, "cart_list").set_original("gx4000");
}

void cpc464_exp_cards(device_slot_interface &device)
{
	device.option_add("ddi1", CPC_DDI1);
	device.option_add("ssa1", CPC_SSA1);
	device.option_add("dkspeech", CPC_DKSPEECH);
	device.option_add("rom", CPC_ROM);
	device.option_add("multiface2", CPC_MFACE2);
	device.option_add("pds", CPC_PDS);
	device.option_add("rs232", CPC_RS232);
	device.option_add("amsrs232", CPC_RS232_AMS);
	device.option_add("sf2", CPC_SYMBIFACE2);
	device.option_add("amdrum", CPC_AMDRUM);
	device.option_add("playcity", CPC_PLAYCITY);
	device.option_add("smartwatch", CPC_SMARTWATCH);
	device.option_add("brunword4", CPC_BRUNWORD_MK4);
	device.option_add("hd20", CPC_HD20);
	device.option_add("doubler", CPC_DOUBLER);
	device.option_add("transtape", CPC_TRANSTAPE);
	device.option_add("musicmachine", CPC_MUSICMACHINE);
}

void cpc_exp_cards(device_slot_interface &device)
{
	device.option_add("ssa1", CPC_SSA1);
	device.option_add("dkspeech", CPC_DKSPEECH);
	device.option_add("rom", CPC_ROM);
	device.option_add("multiface2", CPC_MFACE2);
	device.option_add("pds", CPC_PDS);
	device.option_add("rs232", CPC_RS232);
	device.option_add("amsrs232", CPC_RS232_AMS);
	device.option_add("sf2", CPC_SYMBIFACE2);
	device.option_add("amdrum", CPC_AMDRUM);
	device.option_add("playcity", CPC_PLAYCITY);
	device.option_add("smartwatch", CPC_SMARTWATCH);
	device.option_add("brunword4", CPC_BRUNWORD_MK4);
	device.option_add("hd20", CPC_HD20);
	device.option_add("doubler", CPC_DOUBLER);
	device.option_add("transtape", CPC_TRANSTAPE);
	device.option_add("musicmachine", CPC_MUSICMACHINE);
}

void cpcplus_exp_cards(device_slot_interface &device)
{
	device.option_add("ssa1", CPC_SSA1);
	device.option_add("dkspeech", CPC_DKSPEECH);
	device.option_add("rom", CPC_ROM);
	device.option_add("pds", CPC_PDS);
	device.option_add("rs232", CPC_RS232);
	device.option_add("amsrs232", CPC_RS232_AMS);
	device.option_add("sf2", CPC_SYMBIFACE2);
	device.option_add("amdrum", CPC_AMDRUM);
	device.option_add("playcity", CPC_PLAYCITY);
	device.option_add("smartwatch", CPC_SMARTWATCH);
	device.option_add("hd20", CPC_HD20);
	device.option_add("doubler", CPC_DOUBLER);
	device.option_add("transtape", CPC_TRANSTAPE);  // Plus compatible?
	device.option_add("musicmachine", CPC_MUSICMACHINE);
}

void aleste_exp_cards(device_slot_interface &device)
{
	device.option_add("ssa1", CPC_SSA1);
	device.option_add("dkspeech", CPC_DKSPEECH);
	device.option_add("rom", CPC_ROM);
	device.option_add("multiface2", CPC_MFACE2);
	device.option_add("pds", CPC_PDS);
	device.option_add("rs232", CPC_RS232);
	device.option_add("amsrs232", CPC_RS232_AMS);
	device.option_add("sf2", CPC_SYMBIFACE2);
	device.option_add("amdrum", CPC_AMDRUM);
	device.option_add("playcity", CPC_PLAYCITY);
	device.option_add("smartwatch", CPC_SMARTWATCH);
	device.option_add("brunword4", CPC_BRUNWORD_MK4);
	device.option_add("hd20", CPC_HD20);
	device.option_add("doubler", CPC_DOUBLER);
	device.option_add("transtape", CPC_TRANSTAPE);
	device.option_add("musicmachine", CPC_MUSICMACHINE);
	device.option_add("magicsound", AL_MAGICSOUND);
}

void amstrad_state::amstrad_base(machine_config &config)
{
	/* Machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &amstrad_state::amstrad_mem);
	m_maincpu->set_addrmap(AS_IO, &amstrad_state::amstrad_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(amstrad_state::amstrad_cpu_acknowledge_int));

	config.set_maximum_quantum(attotime::from_hz(60));

	MCFG_MACHINE_START_OVERRIDE(amstrad_state, amstrad )
	MCFG_MACHINE_RESET_OVERRIDE(amstrad_state, amstrad )

	i8255_device &ppi(I8255(config, "ppi8255"));
	ppi.in_pa_callback().set(FUNC(amstrad_state::amstrad_ppi_porta_r));
	ppi.out_pa_callback().set(FUNC(amstrad_state::amstrad_ppi_porta_w));
	ppi.in_pb_callback().set(FUNC(amstrad_state::amstrad_ppi_portb_r));
	ppi.out_pc_callback().set(FUNC(amstrad_state::amstrad_ppi_portc_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16_MHz_XTAL, 1024, 32, 32 + 640 + 64, 312, 56 + 15, 200 + 15);
	m_screen->set_screen_update(FUNC(amstrad_state::screen_update_amstrad));
	m_screen->screen_vblank().set(FUNC(amstrad_state::screen_vblank_amstrad));
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(amstrad_state::amstrad_cpc_palette), 32);

	HD6845S(config, m_crtc, 16_MHz_XTAL / 16);
	m_crtc->set_screen(nullptr);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(16);
	m_crtc->out_de_callback().set(FUNC(amstrad_state::amstrad_de_changed));
	m_crtc->out_hsync_callback().set(FUNC(amstrad_state::amstrad_hsync_changed));
	m_crtc->out_vsync_callback().set(FUNC(amstrad_state::amstrad_vsync_changed));
	m_crtc->out_cur_callback().set("exp", FUNC(cpc_expansion_slot_device::cursor_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8912(config, m_ay, 16_MHz_XTAL / 16);
	m_ay->port_a_read_callback().set(FUNC(amstrad_state::amstrad_psg_porta_read));
	m_ay->add_route(ALL_OUTPUTS, "mono", 0.25);

	/* printer */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(amstrad_state::write_centronics_busy));

	/* snapshot */
	SNAPSHOT(config, "snapshot", "sna").set_load_callback(FUNC(amstrad_state::snapshot_cb));

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(cdt_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.10);
	m_cassette->set_interface("cpc_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("cpc_cass");
}

void amstrad_state::cpc464(machine_config &config)
{
	amstrad_base(config);

	cpc_expansion_slot_device &exp(CPC_EXPANSION_SLOT(config, "exp", 16_MHz_XTAL / 4, cpc464_exp_cards, nullptr));
	exp.set_cpu_tag(m_maincpu);
	exp.irq_callback().set_inputline("maincpu", 0);
	exp.nmi_callback().set_inputline("maincpu", INPUT_LINE_NMI);
	exp.romdis_callback().set(FUNC(amstrad_state::cpc_romdis));  // ROMDIS
	exp.rom_select_callback().set(FUNC(amstrad_state::rom_select));

	/* internal ram */
	RAM(config, m_ram).set_default_size("64K").set_extra_options("128K,320K,576K");
}

void amstrad_state::cpc664(machine_config &config)
{
	amstrad_base(config);
	UPD765A(config, m_fdc, 16_MHz_XTAL / 4, true, true);
	FLOPPY_CONNECTOR(config, "upd765:0", amstrad_floppies, "3ssdd", amstrad_state::amstrad_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "upd765:1", amstrad_floppies, "35ssdd", amstrad_state::amstrad_floppy_formats).enable_sound(true);
	SOFTWARE_LIST(config, "flop_list").set_original("cpc_flop");

	cpc_expansion_slot_device &exp(CPC_EXPANSION_SLOT(config, "exp", 16_MHz_XTAL / 4, cpc_exp_cards, nullptr));
	exp.set_cpu_tag(m_maincpu);
	exp.irq_callback().set_inputline("maincpu", 0);
	exp.nmi_callback().set_inputline("maincpu", INPUT_LINE_NMI);
	exp.romdis_callback().set(FUNC(amstrad_state::cpc_romdis));  // ROMDIS
	exp.rom_select_callback().set(FUNC(amstrad_state::rom_select));

	/* internal ram */
	RAM(config, m_ram).set_default_size("64K").set_extra_options("128K,320K,576K");
}

void amstrad_state::cpc6128(machine_config &config)
{
	amstrad_base(config);
	UPD765A(config, m_fdc, 16_MHz_XTAL / 4, true, true);
	FLOPPY_CONNECTOR(config, "upd765:0", amstrad_floppies, "3ssdd", amstrad_state::amstrad_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "upd765:1", amstrad_floppies, "35ssdd", amstrad_state::amstrad_floppy_formats).enable_sound(true);
	SOFTWARE_LIST(config, "flop_list").set_original("cpc_flop");

	cpc_expansion_slot_device &exp(CPC_EXPANSION_SLOT(config, "exp", 16_MHz_XTAL / 4, cpc_exp_cards, nullptr));
	exp.set_cpu_tag(m_maincpu);
	exp.irq_callback().set_inputline("maincpu", 0);
	exp.nmi_callback().set_inputline("maincpu", INPUT_LINE_NMI);
	exp.romdis_callback().set(FUNC(amstrad_state::cpc_romdis));  // ROMDIS
	exp.rom_select_callback().set(FUNC(amstrad_state::rom_select));

	/* internal ram */
	RAM(config, m_ram).set_default_size("128K").set_extra_options("320K,576K");
}


void amstrad_state::kccomp(machine_config &config)
{
	cpc6128(config);
	MCFG_MACHINE_START_OVERRIDE(amstrad_state,kccomp)
	MCFG_MACHINE_RESET_OVERRIDE(amstrad_state,kccomp)

	m_palette->set_init(FUNC(amstrad_state::kccomp_palette));
}


void amstrad_state::cpcplus(machine_config &config)
{
	/* Machine hardware */
	Z80(config, m_maincpu, 40_MHz_XTAL / 10);
	m_maincpu->set_addrmap(AS_PROGRAM, &amstrad_state::amstrad_mem);
	m_maincpu->set_addrmap(AS_IO, &amstrad_state::amstrad_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(amstrad_state::amstrad_cpu_acknowledge_int));

	config.set_maximum_quantum(attotime::from_hz(60));

	MCFG_MACHINE_START_OVERRIDE(amstrad_state, plus )
	MCFG_MACHINE_RESET_OVERRIDE(amstrad_state, plus )

	ams40489_ppi_device &ppi(AMS40489_PPI(config, "ppi8255"));
	ppi.in_pa_callback().set(FUNC(amstrad_state::amstrad_ppi_porta_r));
	ppi.out_pa_callback().set(FUNC(amstrad_state::amstrad_ppi_porta_w));
	ppi.in_pb_callback().set(FUNC(amstrad_state::amstrad_ppi_portb_r));
	ppi.out_pc_callback().set(FUNC(amstrad_state::amstrad_ppi_portc_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw((40_MHz_XTAL * 2) / 5, 1024, 32, 32 + 640 + 64, 312, 56 + 15, 200 + 15);
	m_screen->set_screen_update(FUNC(amstrad_state::screen_update_amstrad));
	m_screen->screen_vblank().set(FUNC(amstrad_state::screen_vblank_amstrad));
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(amstrad_state::amstrad_plus_palette), 4096);

	AMS40489(config, m_crtc, 40_MHz_XTAL / 40);
	m_crtc->set_screen(nullptr);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(16);
	m_crtc->out_de_callback().set(FUNC(amstrad_state::amstrad_plus_de_changed));
	m_crtc->out_hsync_callback().set(FUNC(amstrad_state::amstrad_plus_hsync_changed));
	m_crtc->out_vsync_callback().set(FUNC(amstrad_state::amstrad_plus_vsync_changed));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8912(config, m_ay, 40_MHz_XTAL / 40);
	m_ay->port_a_read_callback().set(FUNC(amstrad_state::amstrad_psg_porta_read));
	m_ay->add_route(ALL_OUTPUTS, "mono", 0.25);

	/* printer */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(amstrad_state::write_centronics_busy));

	/* snapshot */
	SNAPSHOT(config, "snapshot", "sna").set_load_callback(FUNC(amstrad_state::snapshot_cb));

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(cdt_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.10);
	m_cassette->set_interface("cpc_cass");
	SOFTWARE_LIST(config, "cass_list").set_original("cpc_cass");

	UPD765A(config, m_fdc, 40_MHz_XTAL / 10, true, true);

	cpcplus_cartslot(config);

	FLOPPY_CONNECTOR(config, "upd765:0", amstrad_floppies, "3ssdd", amstrad_state::amstrad_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "upd765:1", amstrad_floppies, "35ssdd", amstrad_state::amstrad_floppy_formats).enable_sound(true);
	SOFTWARE_LIST(config, "flop_list").set_original("cpc_flop");

	cpc_expansion_slot_device &exp(CPC_EXPANSION_SLOT(config, "exp", 40_MHz_XTAL / 10, cpcplus_exp_cards, nullptr));
	exp.set_cpu_tag(m_maincpu);
	exp.irq_callback().set_inputline("maincpu", 0);
	exp.nmi_callback().set_inputline("maincpu", INPUT_LINE_NMI);
	exp.romdis_callback().set(FUNC(amstrad_state::cpc_romdis));  // ROMDIS
	exp.rom_select_callback().set(FUNC(amstrad_state::rom_select));

	/* internal ram */
	RAM(config, m_ram).set_default_size("128K").set_extra_options("64K,320K,576K");
}


void amstrad_state::gx4000(machine_config &config)
{
	/* Machine hardware */
	Z80(config, m_maincpu, 40_MHz_XTAL / 10);
	m_maincpu->set_addrmap(AS_PROGRAM, &amstrad_state::amstrad_mem);
	m_maincpu->set_addrmap(AS_IO, &amstrad_state::amstrad_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(amstrad_state::amstrad_cpu_acknowledge_int));

	config.set_maximum_quantum(attotime::from_hz(60));

	MCFG_MACHINE_START_OVERRIDE(amstrad_state, gx4000 )
	MCFG_MACHINE_RESET_OVERRIDE(amstrad_state, gx4000 )

	ams40489_ppi_device &ppi(AMS40489_PPI(config, "ppi8255"));
	ppi.in_pa_callback().set(FUNC(amstrad_state::amstrad_ppi_porta_r));
	ppi.out_pa_callback().set(FUNC(amstrad_state::amstrad_ppi_porta_w));
	ppi.in_pb_callback().set(FUNC(amstrad_state::amstrad_ppi_portb_r));
	ppi.out_pc_callback().set(FUNC(amstrad_state::amstrad_ppi_portc_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw((40_MHz_XTAL * 2) / 5, 1024, 32, 32 + 640 + 64, 312, 56 + 15, 200 + 15);
	m_screen->set_screen_update(FUNC(amstrad_state::screen_update_amstrad));
	m_screen->screen_vblank().set(FUNC(amstrad_state::screen_vblank_amstrad));
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(amstrad_state::amstrad_plus_palette), 4096);

	AMS40489(config, m_crtc, 40_MHz_XTAL / 40);
	m_crtc->set_screen(nullptr);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(16);
	m_crtc->out_de_callback().set(FUNC(amstrad_state::amstrad_plus_de_changed));
	m_crtc->out_hsync_callback().set(FUNC(amstrad_state::amstrad_plus_hsync_changed));
	m_crtc->out_vsync_callback().set(FUNC(amstrad_state::amstrad_plus_vsync_changed));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8912(config, m_ay, 40_MHz_XTAL / 40);
	m_ay->port_a_read_callback().set(FUNC(amstrad_state::amstrad_psg_porta_read));
	m_ay->add_route(ALL_OUTPUTS, "mono", 0.25);

	cpcplus_cartslot(config);

	/* internal ram */
	RAM(config, m_ram).set_default_size("64K");
}


void amstrad_state::aleste(machine_config &config)
{
	cpc6128(config);
	MCFG_MACHINE_START_OVERRIDE(amstrad_state,aleste)
	MCFG_MACHINE_RESET_OVERRIDE(amstrad_state,aleste)

	AY8912(config.replace(), m_ay, 16_MHz_XTAL / 16);
	m_ay->port_a_read_callback().set(FUNC(amstrad_state::amstrad_psg_porta_read));
	m_ay->add_route(ALL_OUTPUTS, "mono", 0.25);

	m_palette->set_entries(32+64);
	m_palette->set_init(FUNC(amstrad_state::aleste_palette));

	MC146818(config, m_rtc, 4.194304_MHz_XTAL);

	I8272A(config.replace(), m_fdc, 16_MHz_XTAL / 4, true);

	cpc_expansion_slot_device &exp(CPC_EXPANSION_SLOT(config.replace(), "exp", 16_MHz_XTAL / 4, aleste_exp_cards, nullptr));
	exp.set_cpu_tag(m_maincpu);
	exp.irq_callback().set_inputline("maincpu", 0);
	exp.nmi_callback().set_inputline("maincpu", INPUT_LINE_NMI);
	exp.romdis_callback().set(FUNC(amstrad_state::cpc_romdis));  // ROMDIS
	exp.rom_select_callback().set(FUNC(amstrad_state::rom_select));

	FLOPPY_CONNECTOR(config, "upd765:0", aleste_floppies, "35dd", amstrad_state::aleste_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "upd765:1", aleste_floppies, "35dd", amstrad_state::aleste_floppy_formats).enable_sound(true);

	SOFTWARE_LIST(config.replace(), "flop_list").set_original("aleste");
	SOFTWARE_LIST(config, "cpc_list").set_compatible("cpc_flop");

	/* internal ram */
	m_ram->set_default_size("2M");
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

// cpc6128.rom contains OS in first 16k, BASIC in second 16k
// cpcados.rom contains Amstrad DOS

// I am loading the roms outside of the Z80 memory area, because they are banked.
ROM_START( cpc6128 )
	// this defines the total memory size - 64k ram, 16k OS, 16k BASIC, 16k DOS
	ROM_REGION(0x020000, "maincpu", 0)
	/* load the os to offset 0x01000 from memory base */
	ROM_LOAD("cpc6128.rom", 0x10000, 0x8000, CRC(9e827fe1) SHA1(5977adbad3f7c1e0e082cd02fe76a700d9860c30))
	ROM_LOAD("cpcados.rom", 0x18000, 0x4000, CRC(1fe22ecd) SHA1(39102c8e9cb55fcc0b9b62098780ed4a3cb6a4bb))
ROM_END


ROM_START( cpc6128f )
	// this defines the total memory size (128kb))- 64k ram, 16k OS, 16k BASIC, 16k DOS +16k
	ROM_REGION(0x020000, "maincpu", 0)

	// load the os to offset 0x01000 from memory base
	ROM_LOAD("cpc6128f.rom", 0x10000, 0x8000, CRC(1574923b) SHA1(200d59076dfef36db061d6d7d21d80021cab1237))
	ROM_LOAD("cpcados.rom",  0x18000, 0x4000, CRC(1fe22ecd) SHA1(39102c8e9cb55fcc0b9b62098780ed4a3cb6a4bb))
ROM_END


ROM_START( cpc6128s )
	// this defines the total memory size (128kb))- 64k ram, 16k OS, 16k BASIC, 16k DOS +16k
	ROM_REGION(0x020000, "maincpu", 0)

	/* load the os to offset 0x01000 from memory base */
	ROM_LOAD("cpc6128s.rom", 0x10000, 0x8000, CRC(588b5540) SHA1(6765a91a42fed68a807325bf62a728e5ac5d622f))
	ROM_LOAD("cpcados.rom",  0x18000, 0x4000, CRC(1fe22ecd) SHA1(39102c8e9cb55fcc0b9b62098780ed4a3cb6a4bb))
ROM_END

ROM_START( cpc6128sp )
	// this defines the total memory size (128kb))- 64k ram, 16k OS, 16k BASIC, 16k DOS +16k
	ROM_REGION(0x020000, "maincpu", 0)

	/* load the os to offset 0x01000 from memory base */
	ROM_LOAD("amstrad_40038.ic103", 0x10000, 0x8000, CRC(2fa2e7d6) SHA1(1acd01c2c03424a78b32c9440f4d890fb1104815))
	ROM_LOAD("amstrad_40015.ic204",  0x18000, 0x4000, CRC(1fe22ecd) SHA1(39102c8e9cb55fcc0b9b62098780ed4a3cb6a4bb))

	ROM_REGION(0x200, "pals", 0)
	ROM_LOAD("mmi_hal16l8.ic118", 0x000, 0x104, NO_DUMP)
ROM_END

ROM_START( cpc464 )
	/* this defines the total memory size - 64k ram, 16k OS, 16k BASIC, 16k DOS */
	ROM_REGION(0x01c000, "maincpu", 0)
	/* load the os to offset 0x01000 from memory base */
	ROM_LOAD("cpc464.rom",  0x10000, 0x8000, CRC(40852f25) SHA1(56d39c463da60968d93e58b4ba0e675829412a20))
ROM_END


ROM_START( cpc664 )
	/* this defines the total memory size - 64k ram, 16k OS, 16k BASIC, 16k DOS */
	ROM_REGION(0x01c000, "maincpu", 0)
	/* load the os to offset 0x01000 from memory base */
	ROM_LOAD("cpc664.rom",  0x10000, 0x8000, CRC(9ab5a036) SHA1(073a7665527b5bd8a148747a3947dbd3328682c8))
	ROM_LOAD("cpcados.rom", 0x18000, 0x4000, CRC(1fe22ecd) SHA1(39102c8e9cb55fcc0b9b62098780ed4a3cb6a4bb))
ROM_END


ROM_START( kccomp )
	ROM_REGION(0x01c000, "maincpu", 0)
	ROM_LOAD("kccos.rom",  0x10000, 0x4000, CRC(7f9ab3f7) SHA1(f828045e98e767f737fd93df0af03917f936ad08))
	ROM_LOAD("kccbas.rom", 0x14000, 0x4000, CRC(ca6af63d) SHA1(d7d03755099d0aff501fa5fffc9c0b14c0825448))
	ROM_RELOAD(0x18000, 0x4000)  // has no AMSDOS ROM, so just reload BASIC in it's place (BASIC appears in any unused ROM slot)

	ROM_REGION(0x018000+0x0800, "proms", 0)
	ROM_LOAD("farben.rom", 0x18000, 0x0800, CRC(a50fa3cf) SHA1(2f229ac9f62d56973139dad9992c208421bc0f51))

	/* fake region - required by graphics decode structure */
	/*ROM_REGION(0x0c00, "gfx1") */
ROM_END


/* this system must have a cartridge installed to run */
ROM_START(cpc6128p)
	ROM_REGION(0x80000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x04000, "user1", ROMREGION_ERASEFF)
ROM_END

#define rom_cpc464p  rom_cpc6128p
#define rom_gx4000  rom_cpc6128p


ROM_START( al520ex )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("al512.bin", 0x10000, 0x10000, CRC(e8c2a9a1) SHA1(ad5827582cb19eaaae1b76e67df62d96da6ad96b))

	ROM_REGION(0x20, "user2", 0)
	ROM_LOAD("af.bin", 0x00, 0x20, CRC(c81fb524) SHA1(17738d0603915a67ec1fddc4cbf7d6b98cdeb8f6))

	ROM_REGION(0x100, "user3", 0)  // RAM bank mappings
	ROM_LOAD("mapper.bin", 0x00, 0x100, CRC(0daebd80) SHA1(8633073cba752c38c5dc912ff9f6a3c89357539b))

	ROM_REGION(0x800, "user4", 0)  // Colour data
	ROM_LOAD("rfcoldat.bin", 0x00, 0x800, CRC(c6ace0e6) SHA1(2f4c51fcfaacb8deed68f6ae9388b870bc962cef))

	ROM_REGION(0x800, "user5", 0)  // Keyboard / Video
	ROM_LOAD("rfvdkey.bin", 0x00, 0x800, CRC(cf2aa4b0) SHA1(20f37da3bc3c377b1c47ae4d9ab8d150faae19a0))

	ROM_REGION(0x100, "user6", 0)
	ROM_LOAD("romram.bin", 0x00, 0x100, CRC(b3ea95d7) SHA1(1252390737a7ead4ecec988c873181798fbc291b))
ROM_END

/*************************************
 *
 *  Driver definitions
 *
 *************************************/

/*    YEAR  NAME       PARENT  COMPAT MACHINE  INPUT      CLASS          INIT        COMPANY        FULLNAME                                     FLAGS */
COMP( 1984, cpc464,    0,      0,     cpc464,  cpc464,    amstrad_state, empty_init, "Amstrad plc", "Amstrad CPC464",                            0 )
COMP( 1985, cpc664,    cpc464, 0,     cpc664,  cpc664,    amstrad_state, empty_init, "Amstrad plc", "Amstrad CPC664",                            0 )
COMP( 1985, cpc6128,   cpc464, 0,     cpc6128, cpc6128,   amstrad_state, empty_init, "Amstrad plc", "Amstrad CPC6128",                           0 )
COMP( 1985, cpc6128f,  cpc464, 0,     cpc6128, cpc6128f,  amstrad_state, empty_init, "Amstrad plc", "Amstrad CPC6128 (France, AZERTY Keyboard)", 0 )
COMP( 1985, cpc6128s,  cpc464, 0,     cpc6128, cpc6128s,  amstrad_state, empty_init, "Amstrad plc", "Amstrad CPC6128 (Sweden/Finland)",          0 )
COMP( 1985, cpc6128sp, cpc464, 0,     cpc6128, cpc6128sp, amstrad_state, empty_init, "Amstrad plc", "Amstrad CPC6128 (Spain)",                   0 )
COMP( 1990, cpc464p,   0,      0,     cpcplus, plus,      amstrad_state, empty_init, "Amstrad plc", "Amstrad CPC464+",                           0 )
COMP( 1990, cpc6128p,  0,      0,     cpcplus, plus,      amstrad_state, empty_init, "Amstrad plc", "Amstrad CPC6128+",                          0 )
CONS( 1990, gx4000,    0,      0,     gx4000,  gx4000,    amstrad_state, empty_init, "Amstrad plc", "Amstrad GX4000",                            0 )
COMP( 1989, kccomp,    cpc464, 0,     kccomp,  kccomp,    amstrad_state, empty_init, u8"VEB Mikroelektronik \"Wilhelm Pieck\" Mühlhausen",
																									"KC Compact",                                0 )
COMP( 1993, al520ex,   cpc464, 0,     aleste,  aleste,    amstrad_state, empty_init, "Patisonic",   "Aleste 520EX",                              MACHINE_IMPERFECT_SOUND )
