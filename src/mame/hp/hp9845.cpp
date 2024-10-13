// license:BSD-3-Clause
// copyright-holders:Curt Coder, F. Ulivi, Ansgar Kueckes
/*

    HP 9845

    http://www.hp9845.net/

*/
// *******************************
// Driver for HP 9845B/C/T systems
// *******************************
//
// What's in:
// - Emulation of 45B, 45C and 45T systems
// - Emulation of both 5061-3001 CPUs
// - LPU & PPU ROMs
// - LPU & PPU RAMs
// - Text mode screen
// - Graphic screen
// - Keyboard (US & German layouts)
// - T14 and T15 tape drive
// - Software list to load optional ROMs
// - Beeper
// - Correct character generator ROMs (a huge "thank you" to Ansgar Kueckes for the dumps!)
// - 98775 light pen controller
// - Display softkeys on 45C & 45T
// - HLE of integral printer
// What's not yet in:
// - Better naming of tape drive image (it's now "magt1" and "magt2", should be "t15" and "t14")
// - Better documentation of this file
// What's wrong:
// - Speed, as usual
// - Light pen tracing sometimes behaves erratically in 45C and 45T
// What will probably never be in:
// - Fast LPU processor (dump of microcode PROMs is not available)

#include "emu.h"
#include "hp9845.h"

#include "hp9845_optrom.h"
#include "bus/hp9845_io/hp9845_io.h"
#include "machine/timer.h"

#include "render.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "hp9845b.lh"

#include "hp9845_printer.h"

// Debugging
#define VERBOSE 0
#include "logmacro.h"

#define BIT_MASK(n) (1U << (n))

// Macros to clear/set single bits
#define BIT_CLR(w , n)  ((w) &= ~BIT_MASK(n))
#define BIT_SET(w , n)  ((w) |= BIT_MASK(n))

/*

 The 9845 has three possible display options:

 98750A: Standard monochrome (alpha with graphics option)
 98780A: Enhanced monochrome (alpha with hardware accelerated monochrome graphics)
 98770A: Color (color alpha with hardware accelerated color graphics with three planes)

 All displays use a 560x455 graphics raster. Alpha and graphics share the
 same dots within a 720x455 super matrix. All pixels have a 1:1 ratio (square
 pixels).

 The 98750A uses a 25x80 alpha area, either alpha or graphics can be enabled,
 but not both at the same time. In fact, both modes use different video circuits.

 Timing and pixel size for real 98750A are slightly different between
 alpha and graphics raster (dual raster):

                                  alpha       graphics
 ------------------------------------------------------
 Matrix:                          720x375     560x455
 Clock frequency:                 20.85 MHz   20.85 MHz
 Horizontal scan frequency:       23.4 kHz    28.7 kHz
 Horizontal retrace time:         8.2 us      8.0 us
 Frame frequency:                 60 Hz       60 Hz
 Vertical retrace time:           641 us      800 us
 Size on screen:                  9.3"x4.84"  7.9"x6.4"


 The 98770A and 98780A both use a 720x455 raster, implemented with a single video
 circuit, which again is shared by the alpha and graphics logic, with alpha
 dominant over graphics. So, nominally the alpha area for those systems can
 hold up to 30 rows with full size characters plus some lines for one row with
 cropped characters:

                                  98770A       98780A
 ------------------------------------------------------
 Matrix:                          720x455      720x455
 Clock frequency:                 29.7984 MHz  28.224 MHz
 Horizontal scan frequency:       29.1 kHz     31.5 kHz
 Horizontal retrace time:         10.02 us     4.145 us
 Frame frequency:                 60 Hz        60 Hz
 Vertical retrace time:           1.03 ms      2.22 ms
 Size on screen:                  247x154 mm   236x149 mm
 Dot size:                        0.343 mm     0.33 mm

*/

// Base address of video buffer
#define VIDEO_BUFFER_BASE_LOW       0x16000         // for 98770A and 98780A
#define VIDEO_BUFFER_BASE_HIGH      0x17000         // for 98750A

// For test "B" of alpha video to succeed this must be < 234
// Basically "B" test is designed to intentionally prevent line buffer to be filled so that display is blanked
// from 2nd row on. This in turn prevents "BAD" text to be visible on screen.
#define MAX_WORD_PER_ROW        220

// Constants of alpha video
#define VIDEO_PIXEL_CLOCK       20849400
#define VIDEO_CHAR_WIDTH        9
#define VIDEO_CHAR_HEIGHT       15
#define VIDEO_CHAR_COLUMNS      80
#define VIDEO_CHAR_TOTAL        99
#define VIDEO_CHAR_ROWS         25
#define VIDEO_ROWS_TOTAL        26
#define VIDEO_HBSTART           (VIDEO_CHAR_WIDTH * VIDEO_CHAR_COLUMNS)
#define VIDEO_HTOTAL            (VIDEO_CHAR_WIDTH * VIDEO_CHAR_TOTAL)
#define VIDEO_VTOTAL            (VIDEO_CHAR_HEIGHT * VIDEO_ROWS_TOTAL)
#define VIDEO_ACTIVE_SCANLINES  (VIDEO_CHAR_HEIGHT * VIDEO_CHAR_ROWS)
#define VIDEO_TOT_HPIXELS       (VIDEO_CHAR_WIDTH * VIDEO_CHAR_COLUMNS)

// Constants of graphic video
// Pixel clock is 20.8494 MHz (the same as alpha video)
// Horizontal counter counts in [1..727] range
// Vertical counter counts in [34..511] range
#define GVIDEO_HTOTAL           727
#define GVIDEO_HCNT_OFF         1       // Actual start value of h counter
#define GVIDEO_HBEND            (69 - GVIDEO_HCNT_OFF)
#define GVIDEO_HPIXELS          560
#define GVIDEO_HBSTART          (GVIDEO_HBEND + GVIDEO_HPIXELS)
#define GVIDEO_VTOTAL           478
#define GVIDEO_VCNT_OFF         34      // Actual start value of v counter
#define GVIDEO_VBEND            (50 - GVIDEO_VCNT_OFF)
#define GVIDEO_VPIXELS          455
#define GVIDEO_VBSTART          (GVIDEO_VBEND + GVIDEO_VPIXELS)
#define GVIDEO_MEM_SIZE         16384
#define GVIDEO_ADDR_MASK        (GVIDEO_MEM_SIZE - 1)

// Constants of 98770A video
// HBEND & VBEND probably are not really 0
#define VIDEO_770_PIXEL_CLOCK   29798400
#define VIDEO_770_HTOTAL        1024
#define VIDEO_770_HBEND         0
#define VIDEO_770_HBSTART       (VIDEO_CHAR_COLUMNS * VIDEO_CHAR_WIDTH)
#define VIDEO_770_VTOTAL        485
#define VIDEO_770_VBEND         0
#define VIDEO_770_VBSTART       (VIDEO_770_VBEND + GVIDEO_VPIXELS)
#define VIDEO_770_ALPHA_L_LIM   80  // Left-side limit of alpha-only horizontal part
#define VIDEO_770_ALPHA_R_LIM   640 // Right-side limit of alpha-only horizontal part

// Constants of 98780A video
#define VIDEO_780_PIXEL_CLOCK   28224000
#define VIDEO_780_HTOTAL        896
#define VIDEO_780_VTOTAL        525
#define VIDEO_780_HBEND         0
#define VIDEO_780_HBSTART       (VIDEO_CHAR_COLUMNS * VIDEO_CHAR_WIDTH)
#define VIDEO_780_VBEND         0
#define VIDEO_780_VBSTART       (VIDEO_780_VBEND + GVIDEO_VPIXELS)
#define VIDEO_780_ALPHA_L_LIM   80  // Left-side limit of alpha-only horizontal part
#define VIDEO_780_ALPHA_R_LIM   640 // Right-side limit of alpha-only horizontal part

#define I_GR    0xb0    // graphics intensity
#define I_AL    0xd0    // alpha intensity
#define I_CU    0xf0    // graphics cursor intensity
#define I_LP    0xff    // light pen cursor intensity

// Palette indexes (for monochromatic screens)
#define PEN_BLACK   0   // Black
#define PEN_GRAPHIC 1   // Graphics
#define PEN_ALPHA   2   // Text
#define PEN_CURSOR  3   // Graphic cursor
#define PEN_LP      4   // Light pen cursor

// Light pen constants
constexpr unsigned LP_FOV = 9;  // Field of view
constexpr unsigned LP_XOFFSET = 5;  // x-offset of LP (due to delay in hit recognition)

// Peripheral Addresses (PA)
#define PRINTER_PA          0
#define IO_SLOT_FIRST_PA    1
#define IO_SLOT_LAST_PA     12
#define GVIDEO_PA           13
#define T14_PA              14
#define T15_PA              15

#define KEY_SCAN_OSCILLATOR     327680

class hp9845_state : public driver_device
{
public:
	hp9845_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void hp9845a(machine_config &config);
	void hp9835a(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

static INPUT_PORTS_START( hp9845 )
INPUT_PORTS_END

uint32_t hp9845_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START(hp9845_base)
	// Keyboard is arranged in a 8 x 16 matrix. Of the 128 possible positions, 118 are used.
	// Keys are mapped on bit b of KEYn
	// where b = (row & 1) << 4 + column, n = row >> 1
	// column = [0..15]
	// row = [0..7]
	PORT_START("KEY0")
	PORT_BIT(BIT_MASK(0)  , IP_ACTIVE_HIGH , IPT_UNUSED)    // N/U
	PORT_BIT(BIT_MASK(1)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_TOGGLE PORT_NAME("Prt all") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845_base_state, togglekey_changed, 1) // Print All
	PORT_BIT(BIT_MASK(2)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))                           // KP +
	PORT_BIT(BIT_MASK(3)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_COMMA_PAD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))                         // KP ,
	PORT_BIT(BIT_MASK(4)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))                             // KP .
	PORT_BIT(BIT_MASK(5)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))                                 // KP 0
	PORT_BIT(BIT_MASK(6)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_NAME("Execute")    // Execute
	PORT_BIT(BIT_MASK(7)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_ENTER) PORT_NAME("Cont") PORT_CHAR(13)    // Cont
	PORT_BIT(BIT_MASK(8)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))        // Right
	PORT_BIT(BIT_MASK(9)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')  // Space
	PORT_BIT(BIT_MASK(10)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')  // /
	PORT_BIT(BIT_MASK(11)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')  // <
	PORT_BIT(BIT_MASK(12)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_N)     PORT_CHAR('n') PORT_CHAR('N')  // N
	PORT_BIT(BIT_MASK(13)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_V)     PORT_CHAR('v') PORT_CHAR('V')  // V
	PORT_BIT(BIT_MASK(14)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_X)     PORT_CHAR('x') PORT_CHAR('X')  // X
	PORT_BIT(BIT_MASK(15)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)    PORT_CHAR(UCHAR_SHIFT_1)   // Shift
	PORT_BIT(BIT_MASK(16)  , IP_ACTIVE_HIGH , IPT_UNUSED)   // N/U
	PORT_BIT(BIT_MASK(17)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_TOGGLE PORT_NAME("Auto st") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845_base_state, togglekey_changed, 2) // Auto Start
	PORT_BIT(BIT_MASK(18)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) // KP -
	PORT_BIT(BIT_MASK(19)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))         // KP 3
	PORT_BIT(BIT_MASK(20)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))         // KP 2
	PORT_BIT(BIT_MASK(21)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))         // KP 1
	PORT_BIT(BIT_MASK(22)  , IP_ACTIVE_HIGH , IPT_UNUSED)   // N/U
	PORT_BIT(BIT_MASK(23)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))  // Left
	PORT_BIT(BIT_MASK(24)  , IP_ACTIVE_HIGH , IPT_UNUSED)   // Repeat
	PORT_BIT(BIT_MASK(25)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))  // Down
	PORT_BIT(BIT_MASK(26)  , IP_ACTIVE_HIGH , IPT_UNUSED)   // N/U
	PORT_BIT(BIT_MASK(27)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')   // >
	PORT_BIT(BIT_MASK(28)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_M)     PORT_CHAR('m') PORT_CHAR('M')  // M
	PORT_BIT(BIT_MASK(29)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_B)     PORT_CHAR('b') PORT_CHAR('B')  // B
	PORT_BIT(BIT_MASK(30)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_C)     PORT_CHAR('c') PORT_CHAR('C')  // C
	PORT_BIT(BIT_MASK(31)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)     PORT_CHAR('z') PORT_CHAR('Z')  // Z

	PORT_START("KEY1")
	PORT_BIT(BIT_MASK(0)  , IP_ACTIVE_HIGH , IPT_UNUSED)    // N/U
	PORT_BIT(BIT_MASK(1)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_INSERT) PORT_NAME("Ins chr")    // Ins Char
	PORT_BIT(BIT_MASK(2)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))     // KP *
	PORT_BIT(BIT_MASK(3)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))           // KP 6
	PORT_BIT(BIT_MASK(4)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))           // KP 5
	PORT_BIT(BIT_MASK(5)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))           // KP 4
	PORT_BIT(BIT_MASK(6)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_EQUALS_PAD) PORT_CHAR(UCHAR_MAMEKEY(EQUALS_PAD)) // KP =
	PORT_BIT(BIT_MASK(7)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Pause")      // Pause
	PORT_BIT(BIT_MASK(8)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_UP)   PORT_CHAR(UCHAR_MAMEKEY(UP))    // Up
	PORT_BIT(BIT_MASK(9)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Store")  // Store
	PORT_BIT(BIT_MASK(10)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(';') PORT_CHAR(':')      // :
	PORT_BIT(BIT_MASK(11)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_K)     PORT_CHAR('k') PORT_CHAR('K')  // K
	PORT_BIT(BIT_MASK(12)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_H)     PORT_CHAR('h') PORT_CHAR('H')  // H
	PORT_BIT(BIT_MASK(13)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F)     PORT_CHAR('f') PORT_CHAR('F')  // F
	PORT_BIT(BIT_MASK(14)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_S)     PORT_CHAR('s') PORT_CHAR('S')  // S
	PORT_BIT(BIT_MASK(15)  , IP_ACTIVE_HIGH , IPT_UNUSED)   // N/U
	PORT_BIT(BIT_MASK(16)  , IP_ACTIVE_HIGH , IPT_UNUSED)   // N/U
	PORT_BIT(BIT_MASK(17)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("Ins ln")      // Ins Ln
	PORT_BIT(BIT_MASK(18)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) // KP /
	PORT_BIT(BIT_MASK(19)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))         // KP 9
	PORT_BIT(BIT_MASK(20)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))         // KP 8
	PORT_BIT(BIT_MASK(21)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))         // KP 7
	PORT_BIT(BIT_MASK(22)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("Result")     // Result
	PORT_BIT(BIT_MASK(23)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("Run")        // Run
	PORT_BIT(BIT_MASK(24)  , IP_ACTIVE_HIGH , IPT_UNUSED)   // N/U
	PORT_BIT(BIT_MASK(25)  , IP_ACTIVE_HIGH , IPT_UNUSED)   // N/U
	PORT_BIT(BIT_MASK(26)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"') // "
	PORT_BIT(BIT_MASK(27)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_L)     PORT_CHAR('l') PORT_CHAR('L')  // L
	PORT_BIT(BIT_MASK(28)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_J)     PORT_CHAR('j') PORT_CHAR('J')  // J
	PORT_BIT(BIT_MASK(29)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_G)     PORT_CHAR('g') PORT_CHAR('G')  // G
	PORT_BIT(BIT_MASK(30)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_D)     PORT_CHAR('d') PORT_CHAR('D')  // D
	PORT_BIT(BIT_MASK(31)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_A)     PORT_CHAR('a') PORT_CHAR('A')  // A

	PORT_START("KEY2")
	PORT_BIT(BIT_MASK(0)  , IP_ACTIVE_HIGH , IPT_UNUSED)    // N/U
	PORT_BIT(BIT_MASK(1)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Del ln")          // Del Ln
	PORT_BIT(BIT_MASK(2)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Keypad ^")        // KP ^
	PORT_BIT(BIT_MASK(3)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Keypad )")        // KP )
	PORT_BIT(BIT_MASK(4)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Keypad (")        // KP (
	PORT_BIT(BIT_MASK(5)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Keypad E")        // KP E
	PORT_BIT(BIT_MASK(6)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Clear line")      // Clear Line
	PORT_BIT(BIT_MASK(7)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC)) PORT_NAME("Stop")       // Stop
	PORT_BIT(BIT_MASK(8)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')      // |
	PORT_BIT(BIT_MASK(9)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(']') PORT_CHAR('}')   // ]
	PORT_BIT(BIT_MASK(10)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_P)     PORT_CHAR('p') PORT_CHAR('P')  // P
	PORT_BIT(BIT_MASK(11)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_I)     PORT_CHAR('i') PORT_CHAR('I')  // I
	PORT_BIT(BIT_MASK(12)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)     PORT_CHAR('y') PORT_CHAR('Y')  // Y
	PORT_BIT(BIT_MASK(13)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_R)     PORT_CHAR('r') PORT_CHAR('R')  // R
	PORT_BIT(BIT_MASK(14)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_W)     PORT_CHAR('w') PORT_CHAR('W')  // W
	PORT_BIT(BIT_MASK(15)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)  PORT_CHAR(UCHAR_SHIFT_2)   // Control
	PORT_BIT(BIT_MASK(16)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("Typwtr")     // Typwtr
	PORT_BIT(BIT_MASK(17)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)  PORT_NAME("Del chr")    // Del Char
	PORT_BIT(BIT_MASK(18)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Roll down")   // Roll down
	PORT_BIT(BIT_MASK(19)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Roll up")     // Roll up
	PORT_BIT(BIT_MASK(20)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Home")       // Home
	PORT_BIT(BIT_MASK(21)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_NAME("Clr to end") // Clr to end
	PORT_BIT(BIT_MASK(22)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("Clear")      // Clear
	PORT_BIT(BIT_MASK(23)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)     PORT_CHAR('`') PORT_CHAR('~')      // ~
	PORT_BIT(BIT_MASK(24)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)       // BS
	PORT_BIT(BIT_MASK(25)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('=') PORT_CHAR('+')      // +
	PORT_BIT(BIT_MASK(26)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')      // [
	PORT_BIT(BIT_MASK(27)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_O)     PORT_CHAR('o') PORT_CHAR('O')  // O
	PORT_BIT(BIT_MASK(28)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_U)     PORT_CHAR('u') PORT_CHAR('U')  // U
	PORT_BIT(BIT_MASK(29)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_T)     PORT_CHAR('t') PORT_CHAR('T')  // T
	PORT_BIT(BIT_MASK(30)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_E)     PORT_CHAR('e') PORT_CHAR('E')  // E
	PORT_BIT(BIT_MASK(31)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)     PORT_CHAR('q') PORT_CHAR('Q')  // Q

	PORT_START("KEY3")
	PORT_BIT(BIT_MASK(0)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Tab set")    // Tab set
	PORT_BIT(BIT_MASK(1)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Recall")     // Recall
	PORT_BIT(BIT_MASK(2)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("K15")        // K15
	PORT_BIT(BIT_MASK(3)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("K14")        // K14
	PORT_BIT(BIT_MASK(4)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("K13")        // K13
	PORT_BIT(BIT_MASK(5)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_NAME("K12")      // K12
	PORT_BIT(BIT_MASK(6)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11)) PORT_NAME("K11")      // K11
	PORT_BIT(BIT_MASK(7)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10)) PORT_NAME("K10")      // K10
	PORT_BIT(BIT_MASK(8)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9)) PORT_NAME("K9")         // K9
	PORT_BIT(BIT_MASK(9)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_NAME("K8")         // K8
	PORT_BIT(BIT_MASK(10)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_0)     PORT_CHAR('0') PORT_CHAR(')') // 0
	PORT_BIT(BIT_MASK(11)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_8)     PORT_CHAR('8') PORT_CHAR('*')  // 8
	PORT_BIT(BIT_MASK(12)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_6)     PORT_CHAR('6') PORT_CHAR('^')  // 6
	PORT_BIT(BIT_MASK(13)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_4)     PORT_CHAR('4') PORT_CHAR('$')  // 4
	PORT_BIT(BIT_MASK(14)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_2)     PORT_CHAR('2') PORT_CHAR('@')  // 2
	PORT_BIT(BIT_MASK(15)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)   PORT_CHAR('\t')        // Tab
	PORT_BIT(BIT_MASK(16)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("Tab clr")    // Tab clr
	PORT_BIT(BIT_MASK(17)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("Step")  // Step
	PORT_BIT(BIT_MASK(18)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_NAME("K7") // K7
	PORT_BIT(BIT_MASK(19)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_NAME("K6") // K6
	PORT_BIT(BIT_MASK(20)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_NAME("K5") // K5
	PORT_BIT(BIT_MASK(21)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_NAME("K4") // K4
	PORT_BIT(BIT_MASK(22)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_NAME("K3") // K3
	PORT_BIT(BIT_MASK(23)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_NAME("K2") // K2
	PORT_BIT(BIT_MASK(24)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_NAME("K1") // K1
	PORT_BIT(BIT_MASK(25)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("K0") // K0
	PORT_BIT(BIT_MASK(26)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('-') PORT_CHAR('_')      // _
	PORT_BIT(BIT_MASK(27)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_9)     PORT_CHAR('9') PORT_CHAR('(')  // 9
	PORT_BIT(BIT_MASK(28)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_7)     PORT_CHAR('7') PORT_CHAR('&') // 7
	PORT_BIT(BIT_MASK(29)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_5)     PORT_CHAR('5') PORT_CHAR('%')  // 5
	PORT_BIT(BIT_MASK(30)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR('#')  // 3
	PORT_BIT(BIT_MASK(31)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_1)     PORT_CHAR('1') PORT_CHAR('!')  // 1

	PORT_START("SHIFTLOCK");
	PORT_BIT(BIT_MASK(0)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_NAME("Shift lock") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845_base_state, togglekey_changed, 0) // Shift lock

INPUT_PORTS_END

/*
    German keyboard layout

    Remarks:
    - Most keys including umlauts map correctly to the German keyboard layout of the 9845 without special configuration,
      provided that the German keyboard firmware ROM is used on the 9845
    - '#' maps positionally correct to Shift+3
    - AltGr modifier on the Germany PC keyboard for 9845 shifted keycodes 23=| and 5=@ need to get assigned dynamically
    - ~{}\'` are not available on the German 9845 keyboard, ^ is available via keypad only
*/
static INPUT_PORTS_START(hp9845_base_de)
	PORT_INCLUDE(hp9845_base)

	PORT_MODIFY("KEY0")
	PORT_BIT(BIT_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('-') PORT_CHAR('_')           // - _
	PORT_BIT(BIT_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')           // , ;
	PORT_BIT(BIT_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')            // . :
	PORT_BIT(BIT_MASK(31) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('y') PORT_CHAR('Y')               // Y

	PORT_MODIFY("KEY1")
	PORT_BIT(BIT_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(0x00f6) PORT_CHAR(0x00d6) // Ö
	PORT_BIT(BIT_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00e4) PORT_CHAR(0x00c4) // Ä

	PORT_MODIFY("KEY2")
	PORT_BIT(BIT_MASK(8)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<') PORT_CHAR('>')      // < >
	PORT_BIT(BIT_MASK(9)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('+') PORT_CHAR('*')      // + *
	PORT_BIT(BIT_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('z') PORT_CHAR('Z')               // Z
	PORT_BIT(BIT_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)                        // Backspace
	PORT_BIT(BIT_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR(']') PORT_CHAR('@')           // ] @
	PORT_BIT(BIT_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('[') PORT_CHAR('|')          // [ |
	PORT_BIT(BIT_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00fc) PORT_CHAR(0x00dc) // Ü

	PORT_MODIFY("KEY3")
	PORT_BIT(BIT_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')               // 0 =
	PORT_BIT(BIT_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')               // 8 (
	PORT_BIT(BIT_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')               // 6 &
	PORT_BIT(BIT_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')               // 2 "
	PORT_BIT(BIT_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(0x00df) PORT_CHAR('?')    // ß ?
	PORT_BIT(BIT_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')               // 9 )
	PORT_BIT(BIT_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')               // 7 /
INPUT_PORTS_END

// *******************
//  hp9845_base_state
// *******************
hp9845_base_state::hp9845_base_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_lpu(*this, "lpu"),
	m_ppu(*this, "ppu"),
	m_io_sys(*this , "io_sys"),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_gv_timer(*this, "gv_timer"),
	m_io_key(*this, "KEY%u", 0U),
	m_io_shiftlock(*this, "SHIFTLOCK"),
	m_t14(*this, "t14"),
	m_t15(*this, "t15"),
	m_beeper(*this, "beeper"),
	m_beep_timer(*this, "beep_timer"),
	m_io_slot(*this, "slot%u", 0U),
	m_ram(*this, RAM_TAG),
	m_softkeys(*this, "Softkey%u", 0U),
	m_shift_lock_led(*this, "shift_lock_led"),
	m_prt_all_led(*this, "prt_all_led"),
	m_auto_st_led(*this, "auto_st_led"),
	m_chargen(*this, "chargen")
{
}

void hp9845_base_state::setup_ram_block(unsigned block , unsigned offset)
{
	unsigned block_addr = block << 16;
	m_lpu->space(AS_PROGRAM).install_ram(block_addr , block_addr + 0x7fff , m_ram->pointer() + offset);
	m_ppu->space(AS_PROGRAM).install_ram(block_addr , block_addr + 0x7fff , m_ram->pointer() + offset);
}

void hp9845_base_state::machine_start()
{
	m_softkeys.resolve();
	m_shift_lock_led.resolve();
	m_prt_all_led.resolve();
	m_auto_st_led.resolve();

	m_screen->register_screen_bitmap(m_bitmap);

	m_t15->set_name("T15");
	m_t14->set_name("T14");

	// setup RAM dynamically for -ramsize
	// 0K..64K
	setup_ram_block(0 , 0);
	if (m_ram->size() >= 192 * 1024) {
		// 64K..192K
		setup_ram_block(004 , 0x10000);
		setup_ram_block(006 , 0x20000);
	}
	if (m_ram->size() >= 320 * 1024) {
		// 192K..320K
		setup_ram_block(010 , 0x30000);
		setup_ram_block(012 , 0x40000);
	}
	if (m_ram->size() >= 448 * 1024) {
		// 320K..448K
		setup_ram_block(014 , 0x50000);
		setup_ram_block(016 , 0x60000);
	}
}

void hp9845_base_state::device_reset()
{
	// First, unmap every r/w handler in 1..12 select codes
	for (unsigned sc = IO_SLOT_FIRST_PA; sc < (IO_SLOT_LAST_PA + 1); sc++) {
		m_ppu->space(AS_IO).unmap_readwrite(sc * 4 , sc * 4 + 3);
	}

	// Then, set r/w handlers of all installed I/O cards
	int sc;
	read16m_delegate rhandler(*this);
	write16m_delegate whandler(*this);
	for (unsigned i = 0; 4 > i; ++i) {
		if ((sc = m_io_slot[i]->get_rw_handlers(rhandler , whandler)) >= 0) {
			logerror("Install R/W handlers for slot %u @ SC = %d\n", i, sc);
			m_ppu->space(AS_IO).install_readwrite_handler(sc * 4 , sc * 4 + 3 , rhandler , whandler);
			if (m_io_slot[ i ]->has_dual_sc()) {
				logerror("Installing dual SC\n");
				m_ppu->space(AS_IO).install_readwrite_handler(sc * 4 + 4 , sc * 4 + 7 , rhandler , whandler);
			}
		}
		m_slot_sc[ i ] = sc;
	}
}

void hp9845_base_state::machine_reset()
{
	m_lpu->halt_w(1);
	m_ppu->halt_w(0);

	// Some sensible defaults
	m_video_load_mar = false;
	m_video_first_mar = false;
	m_video_byte_idx = false;
	m_video_buff_idx = false;
	m_video_blanked = false;
	m_graphic_sel = false;
	m_gv_fsm_state = GV_STAT_RESET;
	m_gv_int_en = false;
	m_gv_dma_en = false;

	m_io_sys->set_sts(GVIDEO_PA , true);

	memset(&m_kb_state[ 0 ] , 0 , sizeof(m_kb_state));
	m_kb_scancode = 0x7f;
	m_kb_status = 0;

	m_beeper->set_state(0);

	m_prt_irl = false;
}

TIMER_DEVICE_CALLBACK_MEMBER(hp9845_base_state::gv_timer)
{
		advance_gv_fsm(false , false);
}

attotime hp9845_base_state::time_to_gv_mem_availability() const
{
		if (m_graphic_sel) {
				int hpos = m_screen->hpos();
				if (hpos < (34 - GVIDEO_HCNT_OFF) || hpos >= (628 - GVIDEO_HCNT_OFF)) {
						// Access to graphic memory available now
						return attotime::zero;
				} else {
						// Wait until start of hblank
						return m_screen->time_until_pos(m_screen->vpos() , 628);
				}
		} else {
				// TODO:
				return attotime::zero;
		}
}

void hp9845_base_state::kb_scan_ioport(ioport_value pressed , ioport_port &port , unsigned idx_base , int& max_seq_len , unsigned& max_seq_idx)
{
	while (pressed) {
		unsigned bit_no = 31 - count_leading_zeros_32(pressed);
		ioport_value mask = BIT_MASK(bit_no);
		int seq_len = port.field(mask)->seq().length();
		if (seq_len > max_seq_len) {
			max_seq_len = seq_len;
			max_seq_idx = bit_no + idx_base;
		}
		pressed &= ~mask;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(hp9845_base_state::kb_scan)
{
		ioport_value input[ 4 ]{
				m_io_key[0]->read(),
				m_io_key[1]->read(),
				m_io_key[2]->read(),
				m_io_key[3]->read() };

		// Shift lock
		ioport_value shiftlock = m_io_shiftlock->read();

		// Set status bits for "shift", "control", "auto start" & "print all" keys
		// ** Print all **
		// (R,C) = (0,1)
		// Bit 12 in kb status
		if (BIT(input[ 0 ] , 1)) {
				BIT_SET(m_kb_status , 12);
				BIT_CLR(input[ 0 ] , 1);
		} else {
				BIT_CLR(m_kb_status, 12);
		}
		// ** Auto start **
		// (R,C) = (1,1)
		// Bit 13 in kb status
		if (BIT(input[ 0 ] , 17)) {
				BIT_SET(m_kb_status , 13);
				BIT_CLR(input[ 0 ] , 17);
		} else {
				BIT_CLR(m_kb_status, 13);
		}
		// ** Control **
		// (R,C) = (4,15)
		// Bit 14 in kb status
		if (BIT(input[ 2 ] , 15)) {
				BIT_SET(m_kb_status , 14);
				BIT_CLR(input[ 2 ] , 15);
		} else {
				BIT_CLR(m_kb_status, 14);
		}
		// ** Shift **
		// (R,C) = (0,15)
		// Bit 15 in kb status
		if (BIT(input[ 0 ] , 15) || shiftlock) {
				BIT_SET(m_kb_status , 15);
				BIT_CLR(input[ 0 ] , 15);
		} else {
				BIT_CLR(m_kb_status, 15);
		}

		int max_seq_len = 0;
		unsigned max_seq_idx = 0;
		for (unsigned i = 0; 4 > i; ++i)
			kb_scan_ioport(input[i] & ~m_kb_state[i] , *m_io_key[i] , i << 5 , max_seq_len , max_seq_idx);
		// TODO: handle repeat key
		// TODO: handle ctrl+stop

		if (max_seq_len) {
			// Key pressed, store scancode & generate IRL
			m_kb_scancode = max_seq_idx;
			BIT_SET(m_kb_status, 0);
			update_kb_prt_irq();

			// Special case: pressing stop key sets LPU "status" flag
			if (max_seq_idx == 0x47) {
				m_lpu->status_w(1);
			}
		}

		memcpy(&m_kb_state[ 0 ] , &input[ 0 ] , sizeof(m_kb_state));
}

uint16_t hp9845_base_state::kb_scancode_r()
{
		return ~m_kb_scancode & 0x7f;
}

uint16_t hp9845_base_state::kb_status_r()
{
		return m_kb_status;
}

void hp9845_base_state::kb_irq_clear_w(uint16_t data)
{
		BIT_CLR(m_kb_status, 0);
		update_kb_prt_irq();
		m_lpu->status_w(0);

		if (BIT(data , 15)) {
			// Start beeper
			m_beep_timer->adjust(attotime::from_ticks(64, KEY_SCAN_OSCILLATOR / 512));
			m_beeper->set_state(1);
		}
}

void hp9845_base_state::update_kb_prt_irq()
{
	bool state = BIT(m_kb_status , 0) || m_prt_irl;
	m_io_sys->set_irq(0 , state);
}

void hp9845_base_state::set_irq_slot(unsigned slot , int state)
{
	int sc = m_slot_sc[ slot ];
	assert(sc >= 0);
	m_io_sys->set_irq(uint8_t(sc) , state);
}

void hp9845_base_state::set_sts_slot(unsigned slot , int state)
{
	int sc = m_slot_sc[ slot ];
	assert(sc >= 0);
	m_io_sys->set_sts(uint8_t(sc) , state);
}

void hp9845_base_state::set_flg_slot(unsigned slot , int state)
{
	int sc = m_slot_sc[ slot ];
	assert(sc >= 0);
	m_io_sys->set_flg(uint8_t(sc) , state);
}

void hp9845_base_state::set_irq_nextsc_slot(unsigned slot , int state)
{
	int sc = m_slot_sc[ slot ];
	assert(sc >= 0);
	m_io_sys->set_irq(uint8_t(sc + 1) , state);
}

void hp9845_base_state::set_sts_nextsc_slot(unsigned slot , int state)
{
	int sc = m_slot_sc[ slot ];
	assert(sc >= 0);
	m_io_sys->set_sts(uint8_t(sc + 1) , state);
}

void hp9845_base_state::set_flg_nextsc_slot(unsigned slot , int state)
{
	int sc = m_slot_sc[ slot ];
	assert(sc >= 0);
	m_io_sys->set_flg(uint8_t(sc + 1) , state);
}

void hp9845_base_state::set_dmar_slot(unsigned slot , int state)
{
	int sc = m_slot_sc[ slot ];
	assert(sc >= 0);
	m_io_sys->set_dmar(uint8_t(sc) , state);
}

TIMER_DEVICE_CALLBACK_MEMBER(hp9845_base_state::beeper_off)
{
	m_beeper->set_state(0);
}

void hp9845_base_state::prt_irl_w(int state)
{
	m_prt_irl = state;
	update_kb_prt_irq();
}

INPUT_CHANGED_MEMBER(hp9845_base_state::togglekey_changed)
{
	uint32_t togglekey = param;
	switch (togglekey) {
	case 0: // Shift lock
		{
			bool state = m_io_shiftlock->read();
			popmessage("SHIFT LOCK %s", state ? "ON" : "OFF");
			m_shift_lock_led = state;
		}
		break;
	case 1: // Prt all
		{
			bool state = BIT(m_io_key[0]->read(), 1);
			popmessage("PRT ALL %s", state ? "ON" : "OFF");
			m_prt_all_led = state;
		}
		break;
	case 2: // Auto st
		{
			bool state = BIT(m_io_key[0]->read(), 17);
			popmessage("AUTO ST %s", state ? "ON" : "OFF");
			m_auto_st_led = state;
		}
		break;
	}
}

// ***************
//  hp9845b_state
// ***************
class hp9845b_state : public hp9845_base_state
{
public:
	hp9845b_state(const machine_config &mconfig, device_type type, const char *tag);

	void hp9845b(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual uint16_t graphic_r(offs_t offset) override;
	virtual void graphic_w(offs_t offset, uint16_t data) override;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer);

	void vblank_w(int state);

	void set_graphic_mode(bool graphic);
	void set_video_mar(uint16_t mar);
	void video_fill_buff(bool buff_idx);
	void video_render_buff(unsigned video_scanline , unsigned line_in_row, bool buff_idx);
	void graphic_video_render(unsigned video_scanline);

	virtual void advance_gv_fsm(bool ds , bool trigger) override;
	void update_graphic_bits();

	// Optional character generator
	required_region_ptr<uint8_t> m_optional_chargen;

	uint8_t m_video_attr = 0;
	uint16_t m_gv_cursor_w = 0;   // U38 & U39 (GS)
	std::vector<uint16_t> m_graphic_mem;
};

hp9845b_state::hp9845b_state(const machine_config &mconfig, device_type type, const char *tag)
	: hp9845_base_state(mconfig , type , tag),
	  m_optional_chargen(*this , "optional_chargen")
{
}

uint32_t hp9845b_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_graphic_sel) {
		copybitmap(bitmap, m_bitmap, 0, 0, GVIDEO_HBEND, GVIDEO_VBEND, cliprect);
	} else {
		copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	}

	return 0;
}

void hp9845b_state::machine_start()
{
	// Common part first
	hp9845_base_state::machine_start();

	m_graphic_mem.resize(GVIDEO_MEM_SIZE);

	// initialize palette
	m_palette->set_pen_color(PEN_BLACK  , 0x00, 0x00, 0x00);    // black
	m_palette->set_pen_color(PEN_GRAPHIC, 0x00, I_GR, 0x00);    // graphics
	m_palette->set_pen_color(PEN_ALPHA  , 0x00, I_AL, 0x00);    // alpha
	m_palette->set_pen_color(PEN_CURSOR , 0x00, I_CU, 0x00);    // graphics cursor
}

void hp9845b_state::machine_reset()
{
	// Common part first
	hp9845_base_state::machine_reset();

	set_video_mar(0);
	m_video_attr = 0;
	update_graphic_bits();
}

uint16_t hp9845b_state::graphic_r(offs_t offset)
{
	uint16_t res = 0;

	switch (offset) {
	case 0:
		// R4: data register
		res = m_gv_data_r;
		advance_gv_fsm(true , false);
		break;

	case 1:
		// R5: status register
		if (m_gv_int_en) {
			BIT_SET(res, 7);
		}
		if (m_gv_dma_en) {
			BIT_SET(res, 6);
		}
		BIT_SET(res, 5);    // ID
		break;

	case 2:
		// R6: data register with DMA TC
		m_gv_dma_en = false;
		res = m_gv_data_r;
		advance_gv_fsm(true , false);
		break;

	case 3:
		// R7: not mapped
		break;
	}

	//logerror("rd gv R%u = %04x\n", 4 + offset , res);

	return res;
}

void hp9845b_state::graphic_w(offs_t offset, uint16_t data)
{
		//logerror("wr gv R%u = %04x\n", 4 + offset , data);

		switch (offset) {
		case 0:
				// R4: data register
				m_gv_data_w = data;
				m_gv_cursor_w = data;
				advance_gv_fsm(true , false);
				break;

		case 1:
				// R5: command register
				m_gv_cmd = (uint8_t)(data & 0xf);
				m_gv_dma_en = BIT(data , 6) != 0;
				m_gv_int_en = BIT(data , 7) != 0;
				if (BIT(data , 5)) {
					m_gv_fsm_state = GV_STAT_RESET;
				}
				advance_gv_fsm(false , false);
				break;

		case 2:
				// R6: data register with DMA TC
				m_gv_dma_en = false;
				m_gv_data_w = data;
				m_gv_cursor_w = data;
				advance_gv_fsm(true , false);
				break;

		case 3:
				// R7: trigger
				advance_gv_fsm(false , true);
				break;
		}
}

TIMER_DEVICE_CALLBACK_MEMBER(hp9845b_state::scanline_timer)
{
	unsigned video_scanline = param;

	if (m_graphic_sel) {
		if (video_scanline >= GVIDEO_VBEND && video_scanline < GVIDEO_VBSTART) {
			graphic_video_render(video_scanline);
		}
	} else if (video_scanline < VIDEO_ACTIVE_SCANLINES) {
		unsigned row = video_scanline / VIDEO_CHAR_HEIGHT;
		unsigned line_in_row = video_scanline - row * VIDEO_CHAR_HEIGHT;

		if (line_in_row == 0) {
			// Start of new row, swap buffers
			m_video_buff_idx = !m_video_buff_idx;
			video_fill_buff(!m_video_buff_idx);
		}

		video_render_buff(video_scanline , line_in_row , m_video_buff_idx);
	}
}

void hp9845b_state::vblank_w(int state)
{
	// VBlank signal is fed into HALT flag of PPU
	m_ppu->halt_w(state);

	if (state) {
		// Start of V blank
		set_video_mar(0);
		m_video_load_mar = true;
		m_video_first_mar = true;
		m_video_byte_idx = false;
		m_video_blanked = false;
		m_video_buff_idx = !m_video_buff_idx;
		video_fill_buff(!m_video_buff_idx);
	}
}

void hp9845b_state::set_graphic_mode(bool graphic)
{
	if (graphic != m_graphic_sel) {
		m_graphic_sel = graphic;
		logerror("GS=%d\n" , graphic);
		if (m_graphic_sel) {
			m_screen->configure(GVIDEO_HTOTAL , GVIDEO_VTOTAL , rectangle(GVIDEO_HBEND , GVIDEO_HBSTART - 1 , GVIDEO_VBEND , GVIDEO_VBSTART - 1) , HZ_TO_ATTOSECONDS(VIDEO_PIXEL_CLOCK) * GVIDEO_HTOTAL * GVIDEO_VTOTAL);
			// Set graphic mode view (1.23:1 aspect ratio)
			machine().render().first_target()->set_view(1);
		} else {
			m_screen->configure(VIDEO_HTOTAL , VIDEO_VTOTAL , rectangle(0 , VIDEO_HBSTART - 1 , 0 , VIDEO_ACTIVE_SCANLINES - 1) , HZ_TO_ATTOSECONDS(VIDEO_PIXEL_CLOCK) * VIDEO_HTOTAL * VIDEO_VTOTAL);
			// Set alpha mode view (1.92:1 aspect ratio)
			machine().render().first_target()->set_view(0);
		}
	}
}

void hp9845b_state::set_video_mar(uint16_t mar)
{
	m_video_mar = (mar & 0xfff) | VIDEO_BUFFER_BASE_HIGH;
}

void hp9845b_state::video_fill_buff(bool buff_idx)
{
	unsigned char_idx = 0;
	unsigned iters = 0;
	uint8_t byte;
	address_space& prog_space = m_ppu->space(AS_PROGRAM);

	m_video_buff[ buff_idx ].full = false;

	while (1) {
		if (!m_video_byte_idx) {
			if (iters++ >= MAX_WORD_PER_ROW) {
				// Limit on accesses per row reached
				break;
			}
			m_video_word = prog_space.read_word(m_video_mar);
			if (m_video_load_mar) {
				// Load new address into MAR after start of a new frame or NWA instruction
				if (m_video_first_mar) {
					set_graphic_mode(!BIT(m_video_word , 15));
					m_video_first_mar = false;
				}
				set_video_mar(~m_video_word);
				m_video_load_mar = false;
				continue;
			} else {
				// Read normal word from frame buffer, start parsing at MSB
				set_video_mar(m_video_mar + 1);
				byte = (uint8_t)(m_video_word >> 8);
				m_video_byte_idx = true;
			}
		} else {
			// Parse LSB
			byte = (uint8_t)(m_video_word & 0xff);
			m_video_byte_idx = false;
		}
		if ((byte & 0xc0) == 0x80) {
			// Attribute command
			m_video_attr = byte & 0x1f;
		} else if ((byte & 0xc1) == 0xc0) {
			// New Word Address (NWA)
			m_video_load_mar = true;
			m_video_byte_idx = false;
		} else if ((byte & 0xc1) == 0xc1) {
			// End of line (EOL)
			// Fill rest of buffer with spaces
			memset(&m_video_buff[ buff_idx ].chars[ char_idx ] , 0x20 , 80 - char_idx);
			memset(&m_video_buff[ buff_idx ].attrs[ char_idx ] , m_video_attr , 80 - char_idx);
			m_video_buff[ buff_idx ].full = true;
			break;
		} else {
			// Normal character
			m_video_buff[ buff_idx ].chars[ char_idx ] = byte;
			m_video_buff[ buff_idx ].attrs[ char_idx ] = m_video_attr;
			char_idx++;
			if (char_idx == 80) {
				m_video_buff[ buff_idx ].full = true;
				break;
			}
		}
	}
}

void hp9845b_state::video_render_buff(unsigned video_scanline , unsigned line_in_row, bool buff_idx)
{
	if (!m_video_buff[ buff_idx ].full) {
		m_video_blanked = true;
	}

	const pen_t *pen = m_palette->pens();

	if (m_video_blanked) {
		// Blank scanline
		for (unsigned i = 0; i < VIDEO_HBSTART; i++) {
			m_bitmap.pix(video_scanline , i) = pen[ PEN_BLACK ];
		}
	} else {
		bool cursor_line = line_in_row == 12;
		bool ul_line = line_in_row == 14;
		unsigned video_frame = (unsigned)m_screen->frame_number();
		bool cursor_blink = BIT(video_frame , 3);
		bool char_blink = BIT(video_frame , 4);

		for (unsigned i = 0; i < 80; i++) {
			uint8_t charcode = m_video_buff[ buff_idx ].chars[ i ];
			uint8_t attrs = m_video_buff[ buff_idx ].attrs[ i ];
			uint16_t chrgen_addr = ((uint16_t)(charcode ^ 0x7f) << 4) | line_in_row;
			uint16_t pixels;

			if ((ul_line && BIT(attrs , 3)) ||
				(cursor_line && cursor_blink && BIT(attrs , 0))) {
				pixels = ~0;
			} else if (char_blink && BIT(attrs , 2)) {
				pixels = 0;
			} else if (BIT(attrs , 4)) {
				pixels = (uint16_t)(m_optional_chargen[ chrgen_addr ] & 0x7f) << 1;
			} else {
				pixels = (uint16_t)(m_chargen[ chrgen_addr ] & 0x7f) << 1;
			}

			if (BIT(attrs , 1)) {
				pixels = ~pixels;
			}

			for (unsigned j = 0; j < 9; j++) {
				bool pixel = (pixels & (1U << j)) != 0;

				m_bitmap.pix(video_scanline , i * 9 + j) = pen[ pixel ? PEN_ALPHA : PEN_BLACK ];
			}
		}
	}
}

void hp9845b_state::graphic_video_render(unsigned video_scanline)
{
	const pen_t *pen = m_palette->pens();
	bool yc = (video_scanline + GVIDEO_VCNT_OFF) == (m_gv_cursor_y + 6);
	bool yw;
	bool blink;

	if (m_gv_cursor_fs) {
		yw = true;
		// Steady cursor
		blink = true;
	} else {
		yw = (video_scanline + GVIDEO_VCNT_OFF) >= (m_gv_cursor_y + 2) &&
			(video_scanline + GVIDEO_VCNT_OFF) <= (m_gv_cursor_y + 10);
		// Blinking cursor (frame freq. / 16)
		blink = BIT(m_screen->frame_number() , 3) != 0;
	}

	unsigned mem_idx = 36 * (video_scanline - GVIDEO_VBEND);
	for (unsigned i = 0; i < GVIDEO_HPIXELS; i += 16) {
		uint16_t word = m_graphic_mem[ mem_idx++ ];
		unsigned x = i;
		for (uint16_t mask = 0x8000; mask != 0; mask >>= 1) {
			unsigned cnt_h = x + GVIDEO_HBEND + GVIDEO_HCNT_OFF;
			bool xc = cnt_h == (m_gv_cursor_x + 6);
			bool xw = m_gv_cursor_fs || (cnt_h >= (m_gv_cursor_x + 2) && cnt_h <= (m_gv_cursor_x + 10));
			unsigned pixel;
			if (blink && ((xw && yc) || (yw && xc && m_gv_cursor_gc))) {
				// Cursor
				pixel = PEN_CURSOR;
			} else {
				// Normal pixel
				pixel = (word & mask) != 0 ? PEN_GRAPHIC : PEN_BLACK;
			}
			m_bitmap.pix(video_scanline - GVIDEO_VBEND , x++) = pen[ pixel ];
		}
	}
}

void hp9845b_state::advance_gv_fsm(bool ds , bool trigger)
{
	bool get_out = false;

	attotime time_mem_av;

	do {
		bool act_trig = trigger || m_gv_dma_en || !BIT(m_gv_cmd , 2);

		switch (m_gv_fsm_state) {
		case GV_STAT_WAIT_DS_0:
			if ((m_gv_cmd & 0xc) == 0xc) {
				// Read command (11xx)
				m_gv_fsm_state = GV_STAT_WAIT_MEM_0;
			} else if (ds) {
				// Wait for data strobe (r/w on r4 or r6)
				m_gv_fsm_state = GV_STAT_WAIT_TRIG_0;
			} else {
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_TRIG_0:
			// Wait for trigger
			if (act_trig) {
				if (BIT(m_gv_cmd , 3)) {
					// Not a cursor command
					// Load memory address
					m_gv_io_counter = ~m_gv_data_w & GVIDEO_ADDR_MASK;
					// Write commands (10xx)
					m_gv_fsm_state = GV_STAT_WAIT_DS_2;
				} else {
					// Cursor command (0xxx)
					if (BIT(m_gv_cmd , 2)) {
						// Write X cursor position (01xx)
						m_gv_cursor_x = (~m_gv_cursor_w >> 6) & 0x3ff;
					} else {
						// Write Y cursor position and type (00xx)
						m_gv_cursor_y = (~m_gv_cursor_w >> 6) & 0x1ff;
						m_gv_cursor_gc = BIT(m_gv_cmd , 1) == 0;
						m_gv_cursor_fs = BIT(m_gv_cmd , 0) != 0;
					}
					m_gv_fsm_state = GV_STAT_WAIT_DS_0;
				}
			} else {
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_MEM_0:
			time_mem_av = time_to_gv_mem_availability();
			if (time_mem_av.is_zero()) {
				// Read a word from graphic memory
				m_gv_data_r = m_graphic_mem[ m_gv_io_counter ];
				m_gv_io_counter = (m_gv_io_counter + 1) & GVIDEO_ADDR_MASK;
				m_gv_fsm_state = GV_STAT_WAIT_DS_1;
			} else {
				m_gv_timer->adjust(time_mem_av);
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_DS_1:
			if (ds) {
				m_gv_fsm_state = GV_STAT_WAIT_MEM_0;
			} else {
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_DS_2:
			// Wait for data word to be written
			if (ds) {
				m_gv_fsm_state = GV_STAT_WAIT_TRIG_1;
			} else {
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_TRIG_1:
			// Wait for trigger
			if (act_trig) {
				if (BIT(m_gv_cmd , 1)) {
					// Clear words (101x)
					m_gv_data_w = 0;
					m_gv_fsm_state = GV_STAT_WAIT_MEM_1;
				} else if (BIT(m_gv_cmd , 0)) {
					// Write a single pixel (1001)
					m_gv_fsm_state = GV_STAT_WAIT_MEM_2;
				} else {
					// Write words (1000)
					m_gv_fsm_state = GV_STAT_WAIT_MEM_1;
				}
			} else {
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_MEM_1:
			time_mem_av = time_to_gv_mem_availability();
			if (time_mem_av.is_zero()) {
				// Write a full word to graphic memory
				m_graphic_mem[ m_gv_io_counter ] = m_gv_data_w;
				m_gv_io_counter = (m_gv_io_counter + 1) & GVIDEO_ADDR_MASK;
				m_gv_fsm_state = GV_STAT_WAIT_DS_2;
			} else {
				m_gv_timer->adjust(time_mem_av);
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_MEM_2:
			time_mem_av = time_to_gv_mem_availability();
			if (time_mem_av.is_zero()) {
				// Write a single pixel to graphic memory
				uint16_t mask = 0x8000 >> (m_gv_data_w & 0xf);
				if (BIT(m_gv_data_w , 15)) {
					// Set pixel
					m_graphic_mem[ m_gv_io_counter ] |= mask;
				} else {
					// Clear pixel
					m_graphic_mem[ m_gv_io_counter ] &= ~mask;
				}
				// Not really needed
				m_gv_io_counter = (m_gv_io_counter + 1) & GVIDEO_ADDR_MASK;
				m_gv_fsm_state = GV_STAT_WAIT_DS_0;
			} else {
				m_gv_timer->adjust(time_mem_av);
				get_out = true;
			}
			break;

		default:
			logerror("Invalid state reached %d\n" , m_gv_fsm_state);
			m_gv_fsm_state = GV_STAT_RESET;
		}

		ds = false;
		trigger = false;
	} while (!get_out);

	update_graphic_bits();
}

void hp9845b_state::update_graphic_bits()
{
		bool gv_ready = m_gv_fsm_state == GV_STAT_WAIT_DS_0 ||
			m_gv_fsm_state == GV_STAT_WAIT_DS_1 ||
			m_gv_fsm_state == GV_STAT_WAIT_DS_2;

		m_io_sys->set_flg(GVIDEO_PA , gv_ready);

		bool irq = m_gv_int_en && !m_gv_dma_en && gv_ready;

		m_io_sys->set_irq(GVIDEO_PA , irq);

		bool dmar = gv_ready && m_gv_dma_en;

		m_io_sys->set_dmar(GVIDEO_PA , dmar);
}

// ***************
//  hp9845ct_base_state
// ***************

class hp9845ct_base_state : public hp9845_base_state
{
public:
	hp9845ct_base_state(const machine_config &mconfig, device_type type, const char *tag);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void vblank_w(int state);
	DECLARE_INPUT_CHANGED_MEMBER(softkey_changed);

protected:
	required_ioport m_io_softkeys;
	required_ioport m_lightpen_x;
	required_ioport m_lightpen_y;
	required_ioport m_lightpen_sw;

	virtual void set_graphic_mode(bool graphic , bool alpha) = 0;
	void set_video_mar(uint16_t mar);
	void video_fill_buff(bool buff_idx);
	virtual void plot(uint16_t x, uint16_t y, bool draw_erase) = 0;
	void draw_line(unsigned x0 , unsigned y0 , unsigned x1 , unsigned y1);
	void update_line_pattern();
	void pattern_fill(uint16_t x0 , uint16_t y0 , uint16_t x1 , uint16_t y1 , unsigned fill_idx);
	static uint16_t get_gv_mem_addr(unsigned x , unsigned y);
	virtual void update_graphic_bits() = 0;
	static int get_wrapped_scanline(unsigned scanline);
	void render_lp_cursor(unsigned video_scanline , unsigned pen_idx);

	void lp_r4_w(uint16_t data);
	uint16_t lp_r4_r();
	void lp_r5_w(uint16_t data);
	bool lp_segment_intersect(unsigned yline) const;
	void compute_lp_data();
	void lp_scanline_update(unsigned video_scanline);

	virtual void update_gcursor() = 0;

	bool m_alpha_sel;
	bool m_gv_sk_en;
	bool m_gv_gr_en;
	bool m_gv_opt_en;
	bool m_gv_dsa_en;
	bool m_gv_lp_status;
	bool m_gv_sk_status;
	uint16_t m_gv_lp_cursor_x;
	uint16_t m_gv_lp_cursor_y;
	bool m_gv_lp_cursor_fs;
	bool m_gv_lp_en;
	uint8_t m_gv_last_cmd;
	uint16_t m_gv_word_x_position;
	uint16_t m_gv_word_y_position;
	uint16_t m_gv_memory_control;
	uint16_t m_gv_line_type_area_fill;
	uint16_t m_gv_line_type_mask;
	uint8_t m_gv_repeat_count;
	uint16_t m_gv_xpt;
	uint16_t m_gv_ypt;
	uint16_t m_gv_last_xpt;
	uint16_t m_gv_last_ypt;
	uint16_t m_gv_lp_data[ 3 ];
	uint16_t m_gv_next_lp_data[ 3 ];
	unsigned m_gv_next_lp_scanline[ 3 ];
	bool m_gv_lp_selftest;
	bool m_gv_lp_interlace;
	bool m_gv_lp_vblank;
	bool m_gv_lp_1sthit;
	bool m_gv_lp_vbint;
	bool m_gv_lp_fullbright;
	bool m_gv_lp_threshold;
	uint16_t m_gv_lp_x;
	uint16_t m_gv_lp_y;
	bool m_gv_lp_sw;
	uint8_t m_gv_lp_reg_cnt;
	bool m_gv_lp_int_en;
	bool m_gv_lp_hit_lt192;
	bool m_gv_lp_int_256;
	uint16_t m_gv_lxc;
	uint16_t m_gv_lyc;
	uint8_t m_gv_softkey;

	static const uint16_t m_line_type[];
	static const uint16_t m_area_fill[];
};

/*
   For 9845C and 9845T we just add the light pen support via MAME's lightgun device.

   Note that the LIGHTGUN device needs '-lightgun' and '-lightgun_device mouse' for light gun emulation if no real light gun device is installed.
 */
static INPUT_PORTS_START(hp9845ct)
	PORT_INCLUDE(hp9845_base)
	PORT_START("SOFTKEYS")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Softkey0") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845ct_base_state, softkey_changed, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Softkey1") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845ct_base_state, softkey_changed, 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Softkey2") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845ct_base_state, softkey_changed, 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Softkey3") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845ct_base_state, softkey_changed, 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Softkey4") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845ct_base_state, softkey_changed, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Softkey5") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845ct_base_state, softkey_changed, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Softkey6") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845ct_base_state, softkey_changed, 0)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Softkey7") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845ct_base_state, softkey_changed, 0)

	PORT_START("LIGHTPENX")
	PORT_BIT( 0x3ff, 0x000, IPT_LIGHTGUN_X ) PORT_SENSITIVITY(20) PORT_MINMAX(0, VIDEO_TOT_HPIXELS - 1) PORT_CROSSHAIR(X, 1.0, 0.0, 0)

	PORT_START("LIGHTPENY")
	PORT_BIT( 0x3ff, 0x000, IPT_LIGHTGUN_Y ) PORT_SENSITIVITY(20) PORT_MINMAX(0, GVIDEO_VPIXELS - 1) PORT_CROSSHAIR(Y, 1.0, 0.0, 0)

	PORT_START("GKEY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Gkey")
INPUT_PORTS_END

static INPUT_PORTS_START(hp9845ct_de)
	PORT_INCLUDE(hp9845_base_de)
	PORT_START("SOFTKEYS")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Softkey0") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845ct_base_state, softkey_changed, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Softkey1") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845ct_base_state, softkey_changed, 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Softkey2") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845ct_base_state, softkey_changed, 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Softkey3") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845ct_base_state, softkey_changed, 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Softkey4") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845ct_base_state, softkey_changed, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Softkey5") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845ct_base_state, softkey_changed, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Softkey6") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845ct_base_state, softkey_changed, 0)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Softkey7") PORT_CHANGED_MEMBER(DEVICE_SELF, hp9845ct_base_state, softkey_changed, 0)

	PORT_START("LIGHTPENX")
	PORT_BIT( 0x3ff, 0x000, IPT_LIGHTGUN_X ) PORT_SENSITIVITY(20) PORT_MINMAX(0, VIDEO_TOT_HPIXELS - 1) PORT_CROSSHAIR(X, 1.0, 0.0, 0)

	PORT_START("LIGHTPENY")
	PORT_BIT( 0x3ff, 0x000, IPT_LIGHTGUN_Y ) PORT_SENSITIVITY(20) PORT_MINMAX(0, GVIDEO_VPIXELS - 1) PORT_CROSSHAIR(Y, 1.0, 0.0, 0)

	PORT_START("GKEY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Gkey")
INPUT_PORTS_END

hp9845ct_base_state::hp9845ct_base_state(const machine_config &mconfig, device_type type, const char *tag)
	: hp9845_base_state(mconfig , type , tag),
	  m_io_softkeys(*this, "SOFTKEYS"),
	  m_lightpen_x(*this, "LIGHTPENX"),
	  m_lightpen_y(*this, "LIGHTPENY"),
	  m_lightpen_sw(*this, "GKEY")
{
}

void hp9845ct_base_state::machine_start()
{
	// Common part first
	hp9845_base_state::machine_start();
}

void hp9845ct_base_state::machine_reset()
{
	// Common part first
	hp9845_base_state::machine_reset();

	m_alpha_sel = true;
	m_gv_sk_en = false;
	m_gv_gr_en = false;
	m_gv_opt_en = false;
	m_gv_dsa_en = false;
	m_gv_lp_status = false;
	m_gv_sk_status = false;
	m_gv_lp_cursor_x = 944;
	m_gv_lp_cursor_y = 50;
	m_gv_lp_cursor_fs = false;
	m_gv_lp_en = false;
	m_gv_last_cmd = 0;
	m_gv_word_x_position = 0;
	m_gv_word_y_position = 0;
	m_gv_memory_control = 0;
	m_gv_line_type_area_fill = 0;
	m_gv_line_type_mask = 0xffff;
	m_gv_repeat_count = 0;
	m_gv_xpt = 0;
	m_gv_ypt = 0;
	m_gv_last_xpt = 0;
	m_gv_last_ypt = 0;
	m_gv_lp_selftest = false;
	m_gv_lp_interlace = false;
	m_gv_lp_vblank = false;
	m_gv_lp_1sthit = false;
	m_gv_lp_vbint = false;
	m_gv_lp_fullbright = false;
	m_gv_lp_threshold = false;
	m_gv_lp_x = 0;
	m_gv_lp_y = 0;
	m_gv_lp_sw = false;
	m_gv_lp_int_en = false;

	update_graphic_bits();
}

uint32_t hp9845ct_base_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}

void hp9845ct_base_state::vblank_w(int state)
{
	// VBlank signal is fed into HALT flag of PPU
	m_ppu->halt_w(state);

	if (state) {
		// Start of V blank
		set_video_mar(0);
		m_video_load_mar = true;
		m_video_first_mar = true;
		m_video_blanked = false;
		m_video_buff_idx = !m_video_buff_idx;
		video_fill_buff(!m_video_buff_idx);

		// lightpen
		m_gv_lp_vblank = true;
		m_gv_lp_sw = m_lightpen_sw->read();
		m_gv_lp_x = m_lightpen_x->read();
		if (m_gv_lp_x > (VIDEO_TOT_HPIXELS - 1)) {
			m_gv_lp_x = VIDEO_TOT_HPIXELS - 1;
		}
		m_gv_lp_y = m_lightpen_y->read();
		if (m_gv_lp_y > (GVIDEO_VPIXELS - 1)) {
			m_gv_lp_y = GVIDEO_VPIXELS - 1;
		}
	} else {
		m_gv_lp_vblank = false;
		update_gcursor();
	}
}

INPUT_CHANGED_MEMBER(hp9845ct_base_state::softkey_changed)
{
	uint8_t softkey_data = m_io_softkeys->read();
	unsigned softkey;
	for (softkey = 0; softkey < 8; softkey++) {
		m_softkeys[softkey] = !BIT(softkey_data , 7 - softkey);
	}
	for (softkey = 0; softkey < 8 && BIT(softkey_data , 7 - softkey); softkey++) {
	}
	LOG("SK %02x => %u\n" , softkey_data , softkey);
	if (softkey < 8 && !m_gv_sk_status) {
		// softkey pressed
		m_gv_softkey = softkey;
		m_gv_sk_status = true;
		update_graphic_bits();
	}
}

void hp9845ct_base_state::set_video_mar(uint16_t mar)
{
	m_video_mar = (mar & 0x1fff) | VIDEO_BUFFER_BASE_LOW;
}

void hp9845ct_base_state::video_fill_buff(bool buff_idx)
{
	unsigned char_idx = 0;
	unsigned iters = 0;
	address_space& prog_space = m_ppu->space(AS_PROGRAM);

	m_video_buff[ buff_idx ].full = false;

	while (1) {
		if ((m_video_mar & 0x1fff) > 0x1dff) {
			// CRT buffer ends at 0x7dff
			break;
		}
		// Get video word
		if (iters++ >= MAX_WORD_PER_ROW) {
			// Limit on accesses per row reached
			break;
		}
		m_video_word = prog_space.read_word(m_video_mar);
		if (m_video_load_mar) {
			// Load new address into MAR after start of a new frame or NWA instruction
			if (m_video_first_mar) {
				set_graphic_mode(BIT(m_video_word , 15), BIT(m_video_word , 14));
				m_video_first_mar = false;
			}
			set_video_mar(~m_video_word);
			m_video_load_mar = false;
			continue;
		} else {
			// Update counter for next word fetch
			set_video_mar(m_video_mar + 1);
		}
		// Parse video word
		if (m_video_word == 0x8020) {
			// End-of-line (EOL)
			// Fill rest of buffer with spaces
			memset(&m_video_buff[ buff_idx ].chars[ char_idx ] , 0x20 , 80 - char_idx);
			memset(&m_video_buff[ buff_idx ].attrs[ char_idx ] , 0 , 80 - char_idx);
			m_video_buff[ buff_idx ].full = true;
			break;
		} else if ((m_video_word & 0xc020) == 0x8000) {
			// New word address (NWA)
			m_video_load_mar = true;
		} else if ((m_video_word & 0xc000) == 0xc000) {
			// NOP
		} else {
			// fill line buffer
			m_video_buff[ buff_idx ].chars[ char_idx ] = (uint8_t)(m_video_word & 0xff);
			m_video_buff[ buff_idx ].attrs[ char_idx ] = (uint8_t)(m_video_word >> 8);
			char_idx++;
			if (char_idx == 80) {
				m_video_buff[ buff_idx ].full = true;
				break;
			}
		}
	}
}

void hp9845ct_base_state::draw_line(unsigned x0 , unsigned y0 , unsigned x1 , unsigned y1)
{
	int dx, dy, sx, sy, x, y, err, e2;

	// draw line, vector generator uses Bresenham's algorithm
	x = x0;
	y = y0;
	dx = abs((int) (x1 - x));
	sx = x < x1 ? 1 : -1;   // actually always 1 because of normalization
	dy = abs((int) (y1 - y));
	sy = y < y1 ? 1 : -1;
	err = (dx > dy ? dx : -dy) / 2;

	for(;;){
		plot(x, y, BIT(m_gv_line_type_mask, 15));
		update_line_pattern();

		if (x == x1 && y == y1) break;

		e2 = err;
		if (e2 > -dx) {
			err -= dy;
			x += sx;
		}
		if (e2 < dy) {
			err += dx;
			y += sy;
		}
	}
}

void hp9845ct_base_state::update_line_pattern()
{
	// update line pattern
	m_gv_repeat_count++;
	if (m_gv_repeat_count > ((m_gv_line_type_area_fill >> 5) & 0xf)) {
		// Rotate m_gv_line_type_mask 1 bit to the left
		bool save_bit = BIT(m_gv_line_type_mask , 15);
		m_gv_line_type_mask = save_bit | (m_gv_line_type_mask << 1);
		m_gv_repeat_count = 0;
	}
}

void hp9845ct_base_state::pattern_fill(uint16_t x0 , uint16_t y0 , uint16_t x1 , uint16_t y1 , unsigned fill_idx)
{
	uint16_t x,y,xmax,ymax;
	uint16_t pixel_mask, fill_mask;

	x = std::min(x0 , x1);
	xmax = std::max(x0 , x1);
	y = std::min(y0 , y1);
	ymax = std::max(y0 , y1);

	for (;y <= ymax; y++) {
		fill_mask = (m_area_fill[ fill_idx ] << (y % 4) * 4) & 0xf000;
		fill_mask |= (fill_mask >> 4) | (fill_mask >> 8) | (fill_mask >> 12);
		for (;x <= xmax; x++) {
			pixel_mask = (0x8000 >> (x % 16));
			plot(x , y , (pixel_mask & fill_mask) != 0);
		}
	}
}

uint16_t hp9845ct_base_state::get_gv_mem_addr(unsigned x , unsigned y)
{
	return (uint16_t)((x + y * 35) & GVIDEO_ADDR_MASK);
}

int hp9845ct_base_state::get_wrapped_scanline(unsigned scanline)
{
	// The 770's VTOTAL applies to 780, too.
	// The 780 hw generates a line clock (GVclk in Duell's schematics) that's suppressed
	// for lines in [485..524] range so that the total number of lines per frame counted
	// by this clock matches the total count of lines in 770 (i.e. 485).
	int wrapped_cursor_y = (int)scanline;
	if (wrapped_cursor_y >= GVIDEO_VPIXELS && wrapped_cursor_y < VIDEO_770_VTOTAL) {
		wrapped_cursor_y -= VIDEO_770_VTOTAL;
	}

	return wrapped_cursor_y;
}

void hp9845ct_base_state::render_lp_cursor(unsigned video_scanline , unsigned pen_idx)
{
	int cursor_y_top = get_wrapped_scanline(m_gv_lp_cursor_y);

	bool yw;
	if (m_gv_lp_cursor_fs) {
		yw = true;
	} else {
		yw = (int)video_scanline >= cursor_y_top &&
			(int)video_scanline <= (cursor_y_top + 48);
	}
	if (!yw) {
		return;
	}

	if (!m_gv_lp_cursor_fs && m_gv_lp_cursor_x >= (VIDEO_TOT_HPIXELS + 24)) {
		return;
	}

	bool yc = video_scanline == (cursor_y_top + 24);

	const pen_t &pen = m_palette->pen(pen_idx);
	if (!yc) {
		if (m_gv_lp_cursor_x < VIDEO_TOT_HPIXELS) {
			m_bitmap.pix(video_scanline , m_gv_lp_cursor_x) = pen;
		}
	} else if (m_gv_lp_cursor_fs) {
		for (unsigned x = 0; x < VIDEO_TOT_HPIXELS; x++) {
			m_bitmap.pix(video_scanline , x) = pen;
		}
	} else {
		for (unsigned x = std::max(0 , (int)m_gv_lp_cursor_x - 24); x <= (m_gv_lp_cursor_x + 25) && x < VIDEO_TOT_HPIXELS; x++) {
			m_bitmap.pix(video_scanline , x) = pen;
		}
	}
}

void hp9845ct_base_state::lp_r4_w(uint16_t data)
{
	if (m_gv_lp_en) {
		switch (m_gv_lp_reg_cnt) {
		case 2:
			// LP Y cursor + threshold + interlace + vertical blank interrupt
			m_gv_lp_cursor_y = ((~data >> 6) & 0x1ff);
			m_gv_lp_fullbright = BIT(data, 1);
			m_gv_lp_threshold = BIT(data, 3);
			m_gv_lp_interlace = !BIT(data, 4);
			m_gv_lp_vbint = BIT(data, 5);
			LOG("LP Y cursor y = %d, threshold = %d, interlace = %d, vbint = %d\n",
				 m_gv_lp_cursor_y, m_gv_lp_threshold, m_gv_lp_interlace, m_gv_lp_vbint);
			m_gv_lp_reg_cnt--;
			break;

		case 3:
			// LP X cursor + cursor type
			m_gv_lp_cursor_x = ((data >> 6) & 0x3ff) + 1;
			m_gv_lp_cursor_fs = !BIT(data, 0);
			LOG("LP X cursor x = %d, fs = %d\n", m_gv_lp_cursor_x, m_gv_lp_cursor_fs);
			m_gv_lp_reg_cnt--;
			break;

		default:
			logerror("Writing to unmapped LP register %u\n" , m_gv_lp_reg_cnt);
		}
	}
}

uint16_t hp9845ct_base_state::lp_r4_r()
{
	uint16_t res = 0;

	if (m_gv_lp_en) {
		switch (m_gv_lp_reg_cnt) {
		case 4:
			// YLO
			res = m_gv_lp_data[ 2 ];
			m_gv_lp_reg_cnt--;
			m_gv_lp_status = false;
			m_gv_lp_1sthit = false;
			update_graphic_bits();
			break;

		case 5:
			// XLEFT
			res = m_gv_lp_data[ 1 ];
			m_gv_lp_reg_cnt--;
			break;

		case 6:
			// YHI
			res = m_gv_lp_data[ 0 ];
			if (!m_gv_lp_vblank) {
				BIT_SET(res, 12);
			}
			if (m_gv_lp_sw) {
				BIT_SET(res, 14);
			}
			if (m_gv_lp_1sthit) {
				BIT_SET(res, 15);
			}
			m_gv_lp_reg_cnt--;
			break;

		default:
			logerror("Reading from unmapped LP register %u\n" , m_gv_lp_reg_cnt);
		}
	}
	return res;
}

void hp9845ct_base_state::lp_r5_w(uint16_t data)
{
	m_gv_lp_reg_cnt = data & 7;
	m_gv_lp_en = (data & 0x700) == 0x400;   // enables writes on R4 to set LP data (actually FB bit), also enables LP command processing and LP IRQs
	m_gv_lp_int_en = (data & 0x500) == 0x400;
	m_gv_lp_selftest = m_gv_lp_en && m_gv_lp_reg_cnt == 7;
	update_graphic_bits();
}

bool hp9845ct_base_state::lp_segment_intersect(unsigned yline) const
{
	int xp = m_gv_lp_x;
	int yp = m_gv_lp_y;
	int xc = (int)m_gv_lp_cursor_x + 1 - LP_XOFFSET;

	unsigned h = (unsigned)abs((int)yline - yp);

	if (h > LP_FOV) {
		return false;
	}

	int dt = (int)sqrt(LP_FOV * LP_FOV - h * h);
	int ex = xp - dt;
	int fx = xp + dt;

	// Consider [xc..xc+24] segment (i.e. the right-hand side of cursor interlace window)
	return fx >= xc && ex <= (xc + 24);
}

void hp9845ct_base_state::compute_lp_data()
{
	// get LP hit data, returns three words for cmd=6 and one word for cmd=4
	// actually simulating the 9845 lightpen is a bit more complex, since YHI, XLEFT and YLO
	// depend on an circular field of view, moving on the screen
	// bit 0..10 x bzw y
	// bit 11 = IRQ (YHI + XLEFT + YLO)
	// bit 12 = vblank (YHI)
	// bit 13 = xwindow (YHI + XLEFT + YLO) = X is in [xcursor-24, xcursor+24] and Y in [ycursor-8,ycursor+8]
	// bit 14 = sw (YHI) bzw. ywindow (XLEFT + YLO)
	// bit 15 = 1st hit (YHI) = valid hit

	bool xwindow[ 3 ] = { false , false , false };
	bool ywindow[ 3 ] = { false , false , false };
	// hit coordinates
	uint16_t yhi = 0;
	uint16_t xleft = 0;
	uint16_t yleft = 0;
	uint16_t ylo = 0;
	uint16_t xp = m_gv_lp_x;                    // light gun pointer
	uint16_t yp = m_gv_lp_y;
	int yc = get_wrapped_scanline(m_gv_lp_cursor_y) + 24;

	if (m_gv_lp_selftest) {
		constexpr int offset = 57 - VIDEO_770_ALPHA_L_LIM;
		xwindow[ 0 ] = xwindow[ 1 ] = xwindow[ 2 ] = true;
		ywindow[ 0 ] = ywindow[ 1 ] = ywindow[ 2 ] = true;
		yhi = m_gv_lp_cursor_y + 16;    // YHI
		xleft = m_gv_lp_cursor_x + offset;  // XLEFT
		yleft = yhi;
		ylo = m_gv_lp_cursor_y + 32;    // YLO
	} else {
		// Hit in a cursor-only part.
		bool yhi_hit = false;
		uint16_t y_top = std::max(0 , (int)yp - (int)LP_FOV);

		// XLEFT: x coordinate of 1st hit in the frame or hit on cursor line
		unsigned dy = abs((int)yp - yc);
		if (dy <= LP_FOV) {
			// Hit on horizontal cursor line
			xleft = (uint16_t)std::max(0 , (int)xp - (int)sqrt(LP_FOV * LP_FOV - dy * dy));
			yleft = yc;
		} else {
			// 1st hit in the frame
			dy = abs((int)yp - (int)y_top);
			xleft = (uint16_t)std::max(0 , (int)xp - (int)sqrt(LP_FOV * LP_FOV - dy * dy));
			yleft = y_top;
		}
		xleft += LP_XOFFSET;

		if (m_gv_lp_interlace) {
			// **** Interlaced mode ****
			unsigned ywd_top = (unsigned)std::max(yc - 24 , 0);
			unsigned ywd_bot = std::min((unsigned)(GVIDEO_VPIXELS - 1) , (unsigned)(yc + 24));
			unsigned even_odd = (unsigned)m_screen->frame_number() & 1;

			// Scan the cursor window [yc-24..yc+24]
			// Only consider each other line. LSB of frame number selects either even-numbered
			// (0) or odd-numbered lines (1).
			for (unsigned line = ywd_top; line <= ywd_bot; line++) {
				if ((line & 1) == even_odd) {
					// YHI: y coordinate of 1st hit in the frame or 1st hit in cursor-only part
					bool curs_hit = lp_segment_intersect(line);
					if (!yhi_hit && curs_hit) {
						yhi = line;
						yhi_hit = true;
					}
					// YLO: y coordinate of last hit in cursor-only part
					if (curs_hit) {
						ylo = line;
					}
				}
			}
		}

		if (!m_gv_lp_interlace || !yhi_hit) {
			// **** Non-interlaced mode ****
			// YHI: y coordinate of 1st hit in the frame
			yhi = y_top;

			// YLO: y coordinate of last hit in the frame
			ylo = std::min((unsigned)(GVIDEO_VPIXELS - 1) , yp + LP_FOV);
		}

		xwindow[ 0 ] = yhi_hit;
		xwindow[ 1 ] = yhi_hit && yleft >= yhi;
		xwindow[ 2 ] = yhi_hit;

		ywindow[ 1 ] = yleft == yc;
		ywindow[ 2 ] = yleft == yc && ylo >= yleft;
	}
	m_gv_next_lp_data[ 0 ] = ~yhi & 0x1ff;  // YHI
	m_gv_next_lp_data[ 1 ] = ~xleft & 0x3ff;    // XLEFT
	m_gv_next_lp_data[ 2 ] = ~ylo & 0x1ff;  // YLO

	if (!xwindow[ 0 ]) {
		BIT_SET(m_gv_next_lp_data[ 0 ], 13);
	}
	if (!xwindow[ 1 ]) {
		BIT_SET(m_gv_next_lp_data[ 1 ], 13);
	}
	if (!xwindow[ 2 ]) {
		BIT_SET(m_gv_next_lp_data[ 2 ], 13);
	}
	if (!ywindow[ 1 ]) {
		BIT_SET(m_gv_next_lp_data[ 1 ], 14);
	}
	if (!ywindow[ 2 ]) {
		BIT_SET(m_gv_next_lp_data[ 2 ], 14);
	}

	m_gv_next_lp_scanline[ 0 ] = yhi;
	m_gv_next_lp_scanline[ 1 ] = yleft;
	m_gv_next_lp_scanline[ 2 ] = ylo;

	m_gv_lp_hit_lt192 = yhi < 192;
	LOG("LP data %d %d %d %d (%u;%d) (%u;%u) %u %u %u %04x %04x %04x\n" , m_gv_lp_selftest , m_gv_lp_interlace , m_gv_lp_sw , m_gv_lp_hit_lt192 , m_gv_lp_cursor_x , yc , m_gv_lp_x , m_gv_lp_y , yhi , xleft , ylo , m_gv_next_lp_data[ 0 ] , m_gv_next_lp_data[ 1 ] , m_gv_next_lp_data[ 2 ]);
}

void hp9845ct_base_state::lp_scanline_update(unsigned video_scanline)
{
	if (video_scanline == 0) {
		compute_lp_data();
	}

	if (video_scanline == 256 && !m_gv_lp_status && m_gv_lp_hit_lt192) {
		LOG("Hit < 192 @%d\n" , m_screen->vpos());
		m_gv_lp_status = true;
		m_gv_lp_int_256 = true;
		update_graphic_bits();
	}

	if (video_scanline == 456) {
		// VB interrupt
		if (m_gv_lp_vbint || !m_gv_lp_int_256) {
			m_gv_lp_status = true;
		}
		m_gv_lp_int_256 = false;
		update_graphic_bits();
	}

	for (unsigned i = 0; i < 3; i++) {
		if (m_gv_next_lp_scanline[ i ] == video_scanline) {
			m_gv_lp_data[ i ] = m_gv_next_lp_data[ i ];
			if (!m_gv_lp_status) {
				BIT_SET(m_gv_lp_data[ i ], 11);
			}
			m_gv_lp_1sthit = true;
		}
	}
}

const uint16_t hp9845ct_base_state::m_line_type[] = {
	0xffff, 0xaaaa, 0xff00, 0xfff0, 0xfffa, 0xfff6, 0xffb6, 0x0000
};

const uint16_t hp9845ct_base_state::m_area_fill[] = {
	0xffff, 0xefff, 0xefbf, 0xefaf, 0xafaf, 0xadaf, 0xada7, 0xada5,
	0xa5a5, 0xa4a5, 0xa4a1, 0xa4a0, 0xa0a0, 0x80a0, 0x8020, 0x8000
};

// ***************
//  hp9845c_state
// ***************
class hp9845c_state : public hp9845ct_base_state
{
public:
	hp9845c_state(const machine_config &mconfig, device_type type, const char *tag);

	void hp9845c(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual uint16_t graphic_r(offs_t offset) override;
	virtual void graphic_w(offs_t offset, uint16_t data) override;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer);

	virtual void set_graphic_mode(bool graphic , bool alpha) override;
	void video_render_buff(unsigned video_scanline , unsigned line_in_row, bool buff_idx);
	void graphic_video_render(unsigned video_scanline);
	virtual void plot(uint16_t x, uint16_t y, bool draw_erase) override;

	void advance_io_counter();
	virtual void advance_gv_fsm(bool ds , bool trigger) override;
	virtual void update_graphic_bits() override;

	virtual void update_gcursor() override;

	// Palette indexes
	static constexpr unsigned pen_graphic(unsigned rgb) { return rgb; }
	static constexpr unsigned pen_alpha(unsigned rgb) { return 8 + rgb; }
	static constexpr unsigned pen_cursor(unsigned rgb) { return 16 + rgb; }

	// Optional character generator
	required_region_ptr<uint8_t> m_optional_chargen;

	std::vector<uint16_t> m_graphic_mem[ 3 ];
	uint16_t m_gv_music_memory = 0;
	uint8_t m_gv_cursor_color = 0;
	uint8_t m_gv_plane = 0;
	bool m_gv_lp_int_latched = false;
	bool m_gv_sk_int_latched = false;
};

hp9845c_state::hp9845c_state(const machine_config &mconfig, device_type type, const char *tag)
	: hp9845ct_base_state(mconfig , type , tag),
	  m_optional_chargen(*this , "optional_chargen")
{
}

void hp9845c_state::machine_start()
{
	// Common part first
	hp9845ct_base_state::machine_start();

	m_graphic_mem[ 0 ].resize(GVIDEO_MEM_SIZE);
	m_graphic_mem[ 1 ].resize(GVIDEO_MEM_SIZE);
	m_graphic_mem[ 2 ].resize(GVIDEO_MEM_SIZE);

	// initialize palette
	// graphics colors
	m_palette->set_pen_color(0,  0x00, 0x00, 0x00); // black
	m_palette->set_pen_color(1,  I_GR, 0x00, 0x00); // red
	m_palette->set_pen_color(2,  0x00, I_GR, 0x00); // green
	m_palette->set_pen_color(3,  I_GR, I_GR, 0x00); // yellow
	m_palette->set_pen_color(4,  0x00, 0x00, I_GR); // blue
	m_palette->set_pen_color(5,  I_GR, 0x00, I_GR); // magenta
	m_palette->set_pen_color(6,  0x00, I_GR, I_GR); // cyan
	m_palette->set_pen_color(7,  I_GR, I_GR, I_GR); // white

	// alpha colors
	m_palette->set_pen_color(8,  0x00, 0x00, 0x00); // black
	m_palette->set_pen_color(9,  I_AL, 0x00, 0x00); // red
	m_palette->set_pen_color(10, 0x00, I_AL, 0x00); // green
	m_palette->set_pen_color(11, I_AL, I_AL, 0x00); // yellow
	m_palette->set_pen_color(12, 0x00, 0x00, I_AL); // blue
	m_palette->set_pen_color(13, I_AL, 0x00, I_AL); // magenta
	m_palette->set_pen_color(14, 0x00, I_AL, I_AL); // cyan
	m_palette->set_pen_color(15, I_AL, I_AL, I_AL); // white

	// cursor colors
	m_palette->set_pen_color(16, 0x80, 0x80, 0x80); // grey
	m_palette->set_pen_color(17, I_CU, 0x00, 0x00); // red
	m_palette->set_pen_color(18, 0x00, I_CU, 0x00); // green
	m_palette->set_pen_color(19, I_CU, I_CU, 0x00); // yellow
	m_palette->set_pen_color(20, 0x00, 0x00, I_CU); // blue
	m_palette->set_pen_color(21, I_CU, 0x00, I_CU); // magenta
	m_palette->set_pen_color(22, 0x00, I_CU, I_CU); // cyan
	m_palette->set_pen_color(23, I_CU, I_CU, I_CU); // white
}

void hp9845c_state::machine_reset()
{
	// Common part first
	hp9845ct_base_state::machine_reset();

	set_video_mar(0);

	// red -> plane #1, green -> plane #2, blue -> plane #3
	m_gv_music_memory = 0x1 | (0x2 << 3) | (0x4 << 6);
	// TODO: correct?
	m_gv_cursor_color = 7;
	m_gv_plane = 0;
	m_gv_lp_int_latched = false;
	m_gv_sk_int_latched = false;
}

uint16_t hp9845c_state::graphic_r(offs_t offset)
{
	uint16_t res = 0;

	switch (offset) {
	case 2:
		// R6: data register with DMA TC
		m_gv_dma_en = false;
		[[fallthrough]];

	case 0:
		// R4: data register
		if (m_gv_lp_en) {
			res = lp_r4_r();
		} else if (m_gv_sk_int_latched) {
			res = m_gv_softkey;
			m_gv_sk_status = false;
		} else {
			res = m_gv_data_r;
		}
		advance_gv_fsm(true , false);
		update_graphic_bits();
		break;

	case 1:
		// R5: status register
		if (m_gv_int_en) {
			BIT_SET(res, 7);
		}
		if (m_gv_dma_en) {
			BIT_SET(res, 6);
		}
		if (m_gv_lp_int_latched) {
			BIT_SET(res, 0);    // Lightpen service request
		}
		if (m_gv_sk_int_latched) {
			BIT_SET(res, 1);    // Softkey service request
		}
		// TODO: check! Should it be 10 instead?
		BIT_SET(res, 11);   // ID
		break;

	case 3:
		// R7: not mapped
		break;
	}

	LOG("rd gv R%u = %04x\n", 4 + offset , res);

	return res;
}

void hp9845c_state::graphic_w(offs_t offset, uint16_t data)
{
	LOG("wr gv R%u = %04x\n", 4 + offset , data);

	switch (offset) {
	case 0:
		// R4: data register
		m_gv_data_w = data;
		advance_gv_fsm(true , false);
		lp_r4_w(data);
		break;

	case 1:
		// R5: command register
		m_gv_cmd = (uint8_t)(data & 0xf);
		m_gv_dma_en = BIT(data , 6) != 0;
		m_gv_int_en = BIT(data , 7) != 0;
		m_gv_gr_en = BIT(data , 8); // enables graphics controller & vector generator command processing and IRQs
		m_gv_sk_en = BIT(data , 9); // enables reads on R4 to return SK keycode, also enables SK IRQs
		m_gv_opt_en = BIT(data , 11);   // not really used
		m_gv_dsa_en = BIT(data , 12);   // for factory use only (unknown)
		if (BIT(data, 5)) {
			m_gv_fsm_state = GV_STAT_RESET;     // command/reset state machine
		}
		advance_gv_fsm(false , false);
		lp_r5_w(data);
		update_graphic_bits();
		break;

	case 2:
		// R6: data register with DMA TC
		m_gv_dma_en = false;
		m_gv_data_w = data;
		advance_gv_fsm(true , false);
		lp_r4_w(data);
		break;

	case 3:
		// R7: trigger
		advance_gv_fsm(false , true);
		break;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(hp9845c_state::scanline_timer)
{
	unsigned video_scanline = param;

	if (video_scanline >= VIDEO_770_VBEND && video_scanline < VIDEO_770_VBSTART) {
		if (m_graphic_sel) {
			graphic_video_render(video_scanline - VIDEO_770_VBEND);
		}
		unsigned row = (video_scanline - VIDEO_770_VBEND) / VIDEO_CHAR_HEIGHT;
		unsigned line_in_row = (video_scanline - VIDEO_770_VBEND) - row * VIDEO_CHAR_HEIGHT;

		if (line_in_row == 0) {
			// Start of new row, swap buffers
			m_video_buff_idx = !m_video_buff_idx;
			video_fill_buff(!m_video_buff_idx);
		}
		video_render_buff(video_scanline , line_in_row , m_video_buff_idx);
		// Lightpen cursor
		if (m_graphic_sel) {
			render_lp_cursor(video_scanline - VIDEO_770_VBEND , pen_cursor(7));
		}
	}
	lp_scanline_update(video_scanline - VIDEO_770_VBEND);
}

void hp9845c_state::set_graphic_mode(bool graphic , bool alpha)
{
	m_graphic_sel = graphic;
	m_alpha_sel = alpha;
}

void hp9845c_state::video_render_buff(unsigned video_scanline , unsigned line_in_row, bool buff_idx)
{
	if (!m_video_buff[ buff_idx ].full) {
		m_video_blanked = true;
	}

	const pen_t *pen = m_palette->pens();

	if (m_video_blanked || !m_alpha_sel) {
		// Blank scanline
		for (unsigned i = 0; i < VIDEO_770_ALPHA_L_LIM; i++) {
			m_bitmap.pix(video_scanline , i) = pen[ pen_alpha(0) ];
		}
		if (!m_graphic_sel) {
			for (unsigned i = VIDEO_770_ALPHA_L_LIM; i < VIDEO_770_ALPHA_R_LIM; i++) {
				m_bitmap.pix(video_scanline , i) = pen[ pen_alpha(0) ];
			}
		}
		for (unsigned i = VIDEO_770_ALPHA_R_LIM; i < VIDEO_TOT_HPIXELS; i++) {
			m_bitmap.pix(video_scanline , i) = pen[ pen_alpha(0) ];
		}
	} else {
		bool cursor_line = line_in_row == 12;
		bool ul_line = line_in_row == 14;
		unsigned video_frame = (unsigned)m_screen->frame_number();
		bool cursor_blink = BIT(video_frame , 4);
		bool char_blink = !BIT(video_frame , 4);

		for (unsigned i = 0; i < 80; i++) {
			uint8_t charcode = m_video_buff[ buff_idx ].chars[ i ] & 0x7f;
			uint8_t attrs = m_video_buff[ buff_idx ].attrs[ i ];
			uint16_t chrgen_addr = ((uint16_t)(charcode ^ 0x7f) << 4) | line_in_row;
			uint16_t pixels;
			uint8_t color = (attrs >> 4) & 7;

			if (ul_line && BIT(attrs , 3)) {
				// Color of underline: same as character
				pixels = ~0;
			} else if (cursor_line && cursor_blink && BIT(attrs , 0)) {
				// Color of cursor: white
				color = 7;
				pixels = ~0;
			} else if (char_blink && BIT(attrs , 2)) {
				pixels = 0;
			} else if (BIT(m_video_buff[ buff_idx ].chars[ i ] , 7)) {
				// 98770A has hw support to fill the 1st and the 9th column of character matrix
				// with pixels in 2nd and 8th columns, respectively. This feature is used in
				// 98780A to make horizontal lines of line-drawing characters appear continuous
				// (see hp9845t_state::video_render_buff).
				// Apparently, though, HP did not use this feature at all in real
				// machines (i.e. horizontal lines are broken by gaps)
				pixels = (uint16_t)(m_optional_chargen[ chrgen_addr ] & 0x7f) << 1;
			} else {
				pixels = (uint16_t)(m_chargen[ chrgen_addr ] & 0x7f) << 1;
			}

			if (BIT(attrs , 1)) {
				pixels = ~pixels;
			}

			for (unsigned j = 0; j < 9; j++) {
				bool pixel = (pixels & (1U << j)) != 0;
				unsigned x = i * 9 + j;

				if (m_graphic_sel && x >= VIDEO_770_ALPHA_L_LIM && x < VIDEO_770_ALPHA_R_LIM) {
					// alpha overlays graphics (non-dominating)
					if (pixel) {
						m_bitmap.pix(video_scanline , x) = pen[ pen_alpha(color) ];
					}
				} else {
					// Graphics disabled or alpha-only zone
					m_bitmap.pix(video_scanline , x) = pen[ pixel ? pen_alpha(color) : pen_alpha(0) ];
				}
			}
		}
	}
}

void hp9845c_state::graphic_video_render(unsigned video_scanline)
{
	// video_scanline is 0-based, i.e. the topmost visible line of graphic screen is 0
	const pen_t *pen = m_palette->pens();
	bool yc, yw;
	uint16_t word0, word1, word2;
	uint8_t pen0, pen1, pen2;

	// apply music memory
	pen0 = (m_gv_music_memory & 0x001) | ((m_gv_music_memory & 0x008) >> 2) | ((m_gv_music_memory & 0x040) >> 4);
	pen1 = ((m_gv_music_memory & 0x002) >> 1) | ((m_gv_music_memory & 0x010) >> 3) | ((m_gv_music_memory & 0x080) >> 5);
	pen2 = ((m_gv_music_memory & 0x004) >> 2) | ((m_gv_music_memory & 0x020) >> 4) | ((m_gv_music_memory & 0x100) >> 6);

	if (m_gv_cursor_fs) {
		// Full-screen cursor
		yw = false;
		yc = video_scanline == m_gv_cursor_y;
	} else if (m_gv_cursor_gc) {
		// 15 x 15 crosshair
		int cursor_y_top = get_wrapped_scanline(m_gv_cursor_y);
		int int_scanline = (int)video_scanline;
		yw = (int_scanline >= (cursor_y_top + 1) && int_scanline <= (cursor_y_top + 6)) ||
			(int_scanline >= (cursor_y_top + 10) && int_scanline <= (cursor_y_top + 15));
		yc = int_scanline == cursor_y_top + 8;
	} else {
		// 9-pixel blinking line
		int cursor_y_top = get_wrapped_scanline(m_gv_cursor_y);
		int int_scanline = (int)video_scanline;
		yw = false;
		yc = int_scanline == cursor_y_top + 8;
	}

	unsigned mem_idx = get_gv_mem_addr(0 , video_scanline);
	for (unsigned i = 0; i < GVIDEO_HPIXELS; i += 16) {
		word0 = m_graphic_mem[ 0 ][ mem_idx ];
		word1 = m_graphic_mem[ 1 ][ mem_idx ];
		word2 = m_graphic_mem[ 2 ][ mem_idx ];
		mem_idx++;
		unsigned x = i;
		for (uint16_t mask = 0x8000; mask != 0; mask >>= 1) {
			bool cursor = false;
			unsigned pixel;

			if (m_gv_cursor_fs) {
				// Full-screen cursor
				cursor = yc || (x + 111) == m_gv_cursor_x;
			} else if (m_gv_cursor_gc) {
				bool xc = (x + 103) == m_gv_cursor_x;
				bool xw = ((x + 96) <= m_gv_cursor_x && (x + 101) >= m_gv_cursor_x) ||
					((x + 105) <= m_gv_cursor_x && (x + 110) >= m_gv_cursor_x);

				// 15 x 15 crosshair
				cursor = (yc && xw) || (yw && xc);
			} else if (BIT(m_screen->frame_number() , 3)) {
				// 9-pixel blinking line
				cursor = yc && (x + 107) >= m_gv_cursor_x && (x + 99) <= m_gv_cursor_x;
			}
			if (cursor) {
				// Cursor
				pixel = pen_cursor(m_gv_cursor_color);
			} else {
				// Normal pixel
				pixel = pen_graphic(((word0 & mask) ? pen0 : 0) | ((word1 & mask) ? pen1 : 0) | ((word2 & mask) ? pen2 : 0));
			}
			m_bitmap.pix(video_scanline , VIDEO_770_ALPHA_L_LIM + x++) = pen[ pixel ];
		}
	}
}

void hp9845c_state::plot(uint16_t x, uint16_t y, bool draw_erase)
{
	uint16_t addr, pixel_mask;
	bool do_draw, do_erase, dominance;

	pixel_mask = 0x8000 >> (x & 0xf);

	addr = get_gv_mem_addr(x >> 4 , y);
	dominance = BIT(m_gv_memory_control, 6);
	if (BIT(m_gv_memory_control, 0)) {
		do_erase = dominance;
		do_draw = draw_erase;
		if (!BIT(m_gv_memory_control, 3) && draw_erase) {
			do_draw = false;
			do_erase = true;
		}
		if (do_draw)
			m_graphic_mem[0][ addr ] |= pixel_mask;
		else if (do_erase)
			m_graphic_mem[0][ addr ] &= ~pixel_mask;
	}
	if (BIT(m_gv_memory_control, 1)) {
		do_erase = dominance;
		do_draw = draw_erase;
		if (!BIT(m_gv_memory_control, 4) && draw_erase) {
			do_draw = false;
			do_erase = true;
		}
		if (do_draw)
			m_graphic_mem[1][ addr ] |= pixel_mask;
		else if (do_erase)
			m_graphic_mem[1][ addr ] &= ~pixel_mask;
	}
	if (BIT(m_gv_memory_control, 2)) {
		do_erase = dominance;
		do_draw = draw_erase;
		if (!BIT(m_gv_memory_control, 5) && draw_erase) {
			do_draw = false;
			do_erase = true;
		}
		if (do_draw)
			m_graphic_mem[2][ addr ] |= pixel_mask;
		else if (do_erase)
			m_graphic_mem[2][ addr ] &= ~pixel_mask;
	}
}

void hp9845c_state::advance_io_counter()
{
	m_gv_plane++;
	if (m_gv_plane > 2) {
		if (m_gv_io_counter < GVIDEO_ADDR_MASK) {
			m_gv_plane = 0;
			m_gv_io_counter++;
		} else {
			m_gv_plane = 2;
		}
	}
}

void hp9845c_state::advance_gv_fsm(bool ds , bool trigger)
{
	if (!m_gv_gr_en) {
		return;
	}

	bool get_out = false;

	attotime time_mem_av;

	do {
		// U73 on vector generator board
		bool act_trig = trigger || m_gv_int_en || !BIT(m_gv_cmd , 0);

		switch (m_gv_fsm_state) {
		case GV_STAT_WAIT_DS_0:
			// inital state (same as GV_STAT_RESET), command received
			if (m_gv_cmd == 0x1) {
				// read words command
				LOG("read words\n");
				m_gv_fsm_state = GV_STAT_WAIT_TRIG_0;
			} else if (ds) {
				if ((m_gv_cmd == 0x0) || (m_gv_cmd == 0x2)) {
					// write words & clear/set words commands
					if (m_gv_cmd == 0x2) LOG("clear/set words\n");
					else LOG("write words\n");
					m_gv_fsm_state = GV_STAT_WAIT_TRIG_1;   // -> write stream
				} else {
					// any other command
					m_gv_fsm_state = GV_STAT_WAIT_TRIG_0;   // -> wait for trigger
				}
			} else {
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_TRIG_0:
			// process data on R4 or R6
			if (act_trig) {
				switch (m_gv_cmd) {
				case 1: // read words command
					break;
				case 0x8:   // load X I/O address
					m_gv_word_x_position = ~m_gv_data_w & 0x3f;     // 0..34
					LOG("load X I/O adress = %04x\n", m_gv_word_x_position);
					m_gv_io_counter = get_gv_mem_addr(m_gv_word_x_position , m_gv_word_y_position);
					m_gv_plane = 0;
					break;
				case 0x9:   // load Y I/O address
					m_gv_word_y_position = ~m_gv_data_w & 0x1ff;    // 0..454
					LOG("load Y I/O adress = %04x\n", m_gv_word_y_position);
					m_gv_io_counter = get_gv_mem_addr(m_gv_word_x_position , m_gv_word_y_position);
					break;
				case 0xa:   // load memory control
					m_gv_memory_control = m_gv_data_w & 0x7f;
					LOG("load memory control = %04x\n", m_gv_memory_control);
					break;
				case 0xb:   // set line type/area fill
					m_gv_line_type_area_fill =  m_gv_data_w & 0x1ff;
					if (BIT(m_gv_line_type_area_fill, 4)) {
						m_gv_line_type_mask = m_line_type[ m_gv_line_type_area_fill & 0x7 ];
						m_gv_repeat_count = 0;
					}
					LOG("set line type = %04x\n", m_gv_line_type_area_fill);
					break;
				case 0xc:   // load color mask
					m_gv_music_memory = m_gv_data_w & 0x1ff;
					LOG("load color mask = %04x\n", m_gv_music_memory);
					break;
				case 0xd:   // load end points
					m_gv_ypt = ~m_gv_data_w & 0x1ff;
					LOG("load end points y = %d\n", m_gv_ypt);
					break;
				case 0xe:   // Y cursor position & color
					m_gv_lyc = m_gv_data_w;
					break;
				case 0xf:   // X cursor position & type
					m_gv_lxc = m_gv_data_w;
					break;
				default:
					logerror("unknown 98770A command = %d, parm = 0x%04x\n", m_gv_cmd, m_gv_data_w);
				}
				if (m_gv_cmd == 1) {    // Read words
					m_gv_fsm_state = GV_STAT_WAIT_MEM_0;
				} else if (m_gv_cmd == 0xd) {
					m_gv_fsm_state = GV_STAT_WAIT_DS_2;     // -> get second data word
				} else {
					get_out = true;
					m_gv_fsm_state = GV_STAT_WAIT_DS_0;     // -> done
				}
			} else {
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_MEM_0:
			// process data during read transfer
			time_mem_av = time_to_gv_mem_availability();
			if (time_mem_av.is_zero()) {
				// Read a word from graphic memory
				m_gv_data_r = m_graphic_mem[ m_gv_plane ][ m_gv_io_counter ];
				LOG("read words @%04x = %04x, plane #%d\n" , m_gv_io_counter , m_gv_data_r, m_gv_plane + 1);
				advance_io_counter();
				m_gv_fsm_state = GV_STAT_WAIT_DS_1;     // -> proceed with read stream
			} else {
				m_gv_timer->adjust(time_mem_av);
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_DS_1:
			// wait for data word to be read
			if (ds) {
				// -- next word
				m_gv_fsm_state = GV_STAT_WAIT_TRIG_0;    // -> process data word
			} else {
				// -- done
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_DS_2:
			// wait for data word to be written
			if (ds) {
				// -- next word
				m_gv_fsm_state = GV_STAT_WAIT_TRIG_1;   // -> process data word
			} else {
				// done
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_TRIG_1:
			// process multi-word parameters & data during write transfer
			if (act_trig) {
				if (m_gv_cmd == 0xd) {
					// load endpoints command
					m_gv_xpt = ~m_gv_data_w & 0x3ff;
					if (BIT(m_gv_data_w, 10)) {
						// draw vector
						LOG("load end points x = %d (draw)\n", m_gv_xpt);
						m_gv_fsm_state = GV_STAT_WAIT_MEM_2;    // -> proceed with draw vector
					} else {
						LOG("load end points x = %d (move)\n", m_gv_xpt);
						m_gv_last_xpt = m_gv_xpt;
						m_gv_last_ypt = m_gv_ypt;
						m_gv_fsm_state = GV_STAT_WAIT_DS_0;     // -> proceed with next word pair
					}
				} else if (m_gv_cmd == 0x2) {
					// clear/set words command
					m_gv_data_w = BIT(m_gv_memory_control, m_gv_plane + 3) ? 0xffff : 0;
					m_gv_fsm_state = GV_STAT_WAIT_MEM_1;        // -> proceed with next word
				} else if (m_gv_cmd == 0x0) {
					// write words command
					m_gv_fsm_state = GV_STAT_WAIT_MEM_1;        // -> proceed with next word
				}
			} else {
				// done
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_MEM_1:
			// -- transfer from bus to graphics memory to bus within write transfer
			time_mem_av = time_to_gv_mem_availability();
			if (time_mem_av.is_zero()) {
				// Write a full word to graphic memory
				LOG("write words @%04x = %04x, plane #%d\n" , m_gv_io_counter , m_gv_data_w, m_gv_plane + 1);
				if ((m_gv_cmd == 0x0) || BIT(m_gv_memory_control, m_gv_plane)) {
					m_graphic_mem[ m_gv_plane ][ m_gv_io_counter ] = m_gv_data_w;
				}
				advance_io_counter();
				m_gv_fsm_state = GV_STAT_WAIT_DS_2;             // -> proceed with write stream
			} else {
				m_gv_timer->adjust(time_mem_av);
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_MEM_2:
			// vector generator
			time_mem_av = time_to_gv_mem_availability();
			if (time_mem_av.is_zero()) {
				if (BIT (m_gv_line_type_area_fill, 4)) {
					unsigned x0;
					unsigned x1;
					unsigned y0;
					unsigned y1;

					// vector generator uses normalization
					if (m_gv_xpt > m_gv_last_xpt) {
						x0 = m_gv_last_xpt;
						y0 = m_gv_last_ypt;
						x1 = m_gv_xpt;
						y1 = m_gv_ypt;
					} else {
						x0 = m_gv_xpt;
						y0 = m_gv_ypt;
						x1 = m_gv_last_xpt;
						y1 = m_gv_last_ypt;
					}
					draw_line(x0 , y0 , x1 , y1);
				} else {
					// fill area with pattern
					LOG("area fill (%d,%d) -> (%d,%d) pattern=%04x\n", m_gv_last_xpt, m_gv_last_ypt, m_gv_xpt, m_gv_ypt, m_gv_line_type_area_fill);

					pattern_fill(m_gv_xpt , m_gv_ypt , m_gv_last_xpt , m_gv_last_ypt , m_gv_line_type_area_fill & 0xf);
				}
				m_gv_last_xpt = m_gv_xpt;
				m_gv_last_ypt = m_gv_ypt;
				m_gv_fsm_state = GV_STAT_WAIT_DS_0;     // -> proceed with next word pair
			} else {
				m_gv_timer->adjust(time_mem_av);
				get_out = true;
			}
			break;

		default:
			logerror("Invalid state reached %d\n" , m_gv_fsm_state);
			m_gv_fsm_state = GV_STAT_RESET;
		}

		ds = false;
		trigger = false;
	} while (!get_out);

	update_graphic_bits();
}

void hp9845c_state::update_graphic_bits()
{
	bool lp_int = m_gv_lp_int_en && m_gv_lp_status;
	bool sk_int = m_gv_sk_status && m_gv_sk_en;

	if (lp_int && !m_gv_sk_int_latched) {
		m_gv_lp_int_latched = true;
	} else if (sk_int && !m_gv_lp_int_latched) {
		m_gv_sk_int_latched = true;
	} else if (!lp_int && !sk_int) {
		m_gv_lp_int_latched = false;
		m_gv_sk_int_latched = false;
	}

	bool gv_ready = m_gv_lp_int_latched || m_gv_sk_int_latched;

	if (m_gv_gr_en && !gv_ready) {
		gv_ready = m_gv_fsm_state == GV_STAT_WAIT_DS_0 ||
			m_gv_fsm_state == GV_STAT_WAIT_TRIG_0 ||
			m_gv_fsm_state == GV_STAT_WAIT_DS_1 ||
			m_gv_fsm_state == GV_STAT_WAIT_DS_2 ||
			m_gv_fsm_state == GV_STAT_WAIT_TRIG_1;
	}

	m_io_sys->set_flg(GVIDEO_PA , gv_ready);

	bool irq = m_gv_int_en && !m_gv_dma_en && gv_ready;

	m_io_sys->set_irq(GVIDEO_PA , irq);

	bool dmar = gv_ready && m_gv_dma_en;

	m_io_sys->set_dmar(GVIDEO_PA , dmar);
}

void hp9845c_state::update_gcursor()
{
	m_gv_cursor_color = ~m_gv_lyc & 0x7;
	m_gv_cursor_y = (~m_gv_lyc >> 6) & 0x1ff;
	m_gv_cursor_fs = BIT(m_gv_lxc, 0);
	m_gv_cursor_gc = BIT(m_gv_lxc, 1);
	m_gv_cursor_x = (m_gv_lxc >> 6) & 0x3ff;
}

// ***************
//  hp9845t_state
// ***************
class hp9845t_state : public hp9845ct_base_state
{
public:
	hp9845t_state(const machine_config &mconfig, device_type type, const char *tag);

	void hp9845t(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual uint16_t graphic_r(offs_t offset) override;
	virtual void graphic_w(offs_t offset, uint16_t data) override;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer);

	virtual void set_graphic_mode(bool graphic , bool alpha) override;
	void video_render_buff(unsigned video_scanline , unsigned line_in_row, bool buff_idx);
	void graphic_video_render(unsigned video_scanline);
	virtual void plot(uint16_t x, uint16_t y, bool draw_erase) override;
	void draw_arc(uint16_t x0, uint16_t y0, int xstart, int ystart, uint16_t radius, int quadrant, int& draw_counter);
	void draw_full_arc(int x0 , int y0 , int dx , int dy , int draw_counter);

	virtual void advance_gv_fsm(bool ds , bool trigger) override;
	virtual void update_graphic_bits() override;

	virtual void update_gcursor() override;

	std::vector<uint16_t> m_graphic_mem;

	bool m_gv_stat = false;
	bool m_gv_increment_to_next_row = false;

	uint16_t m_gv_scan_start_x = 0;
	uint16_t m_gv_scan_start_y = 0;
	uint8_t m_gv_rb_control = 0;
	uint16_t m_gv_rb_counter = 0;
	uint16_t m_gv_rb_memory[ 256 ]{};
	uint16_t m_gv_arc[ 4 ]{};
	uint8_t m_gv_arc_parm = 0;
	bool m_back_arrow_cursor = false;

	static const uint8_t m_back_arrow_shape[];
};

hp9845t_state::hp9845t_state(const machine_config &mconfig, device_type type, const char *tag)
	: hp9845ct_base_state(mconfig , type , tag)
{
}

void hp9845t_state::machine_start()
{
	// Common part first
	hp9845ct_base_state::machine_start();

	m_graphic_mem.resize(GVIDEO_MEM_SIZE);

	// initialize palette
	m_palette->set_pen_color(PEN_BLACK  , 0x00, 0x00, 0x00);    // black
	m_palette->set_pen_color(PEN_GRAPHIC, 0x00, I_GR, 0x00);    // graphics
	m_palette->set_pen_color(PEN_ALPHA  , 0x00, I_AL, 0x00);    // alpha
	m_palette->set_pen_color(PEN_CURSOR , 0x00, I_CU, 0x00);    // graphics cursor
	m_palette->set_pen_color(PEN_LP     , 0x00, I_LP, 0x00);    // lightpen cursor
}

void hp9845t_state::machine_reset()
{
	// Common part first
	hp9845ct_base_state::machine_reset();

	m_gv_stat = false;
	m_gv_increment_to_next_row = false;
	m_gv_scan_start_x = 0;
	m_gv_scan_start_y = 0;
	m_gv_rb_control = 0;
	m_gv_rb_counter = 0;
	memset(m_gv_rb_memory, 0, sizeof(m_gv_rb_memory));
	m_back_arrow_cursor = false;

	set_video_mar(0);
}

uint16_t hp9845t_state::graphic_r(offs_t offset)
{
	uint16_t res = 0;

	switch (offset) {
	case 2:
		// R6: data register with DMA TC
		m_gv_dma_en = false;
		[[fallthrough]];

	case 0:
		// R4: data register
		if (m_gv_lp_en) {
			res = lp_r4_r();
		} else if (m_gv_sk_en) {
			res = m_gv_softkey;
			m_gv_sk_status = false;
		} else {
			res = m_gv_data_r;
		}
		advance_gv_fsm(true , false);
		update_graphic_bits();
		break;

	case 1:
		// R5: status register
		if (m_gv_int_en) {
			BIT_SET(res, 7);
		}
		if (m_gv_dma_en) {
			BIT_SET(res, 6);
		}
		if (m_gv_lp_status && m_gv_lp_int_en) {
			BIT_SET(res, 0);    // Lightpen service request
		}
		// TODO: gsr/
		if (m_gv_sk_status) {
			BIT_SET(res, 1);    // Softkey service request
		}
		BIT_SET(res, 9);        // ID
		BIT_SET(res, 11);       // ID
		if (m_gv_stat) {
			BIT_SET(res, 13);   // error indication
		}
		break;

	case 3:
		// R7: not mapped
		break;
	}

	LOG("rd gv R%u = %04x\n", 4 + offset , res);

	return res;
}

void hp9845t_state::graphic_w(offs_t offset, uint16_t data)
{
	LOG("wr gv R%u = %04x\n", 4 + offset , data);

	switch (offset) {
	case 0:
		// R4: data register
		m_gv_data_w = data;
		advance_gv_fsm(true , false);
		lp_r4_w(data);
		if (m_gv_lp_int_en) {
			m_gv_lp_fullbright = BIT(data , 1);
		}
		break;

	case 1:
		// R5: command register
		m_gv_cmd = (uint8_t)(data & 0xf);
		m_gv_dma_en = BIT(data , 6) != 0;
		m_gv_int_en = BIT(data , 7) != 0;
		m_gv_gr_en = BIT(data , 8);       // enables graphics controller & vector generator command processing and IRQs
		m_gv_sk_en = (data & 0xb00) == 0x200;       // enables reads on R4 to return SK keycode, also enables SK IRQs
		m_gv_opt_en = BIT(data , 11);     // not really used
		m_gv_dsa_en = BIT(data , 12);     // for factory use only (function unknown)
		m_gv_fsm_state = GV_STAT_RESET;     // command/reset state machine
		lp_r5_w(data);
		advance_gv_fsm(false , false);
		break;

	case 2:
		// R6: data register with DMA TC
		m_gv_dma_en = false;
		m_gv_data_w = data;
		lp_r4_w(data);
		if (m_gv_lp_int_en) {
			m_gv_lp_fullbright = BIT(data , 1);
		}
		advance_gv_fsm(true , false);
		break;

	case 3:
		// R7: not mapped
		break;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(hp9845t_state::scanline_timer)
{
	unsigned video_scanline = param;

	if (video_scanline >= VIDEO_780_VBEND && video_scanline < VIDEO_780_VBSTART) {
		if (m_graphic_sel) {
			graphic_video_render(video_scanline - VIDEO_780_VBEND);
		}
		unsigned row = (video_scanline - VIDEO_780_VBEND) / VIDEO_CHAR_HEIGHT;
		unsigned line_in_row = (video_scanline - VIDEO_780_VBEND) - row * VIDEO_CHAR_HEIGHT;

		if (line_in_row == 0) {
			// Start of new row, swap buffers
			m_video_buff_idx = !m_video_buff_idx;
			video_fill_buff(!m_video_buff_idx);
		}
		video_render_buff(video_scanline , line_in_row , m_video_buff_idx);
		// Lightpen cursor
		if (m_graphic_sel) {
			render_lp_cursor(video_scanline - VIDEO_780_VBEND , PEN_LP);
		}
	}
	lp_scanline_update(video_scanline - VIDEO_780_VBEND);
}

void hp9845t_state::set_graphic_mode(bool graphic , bool alpha)
{
	m_back_arrow_cursor = graphic;      // triggers back arrow cursor, 98780A uses video on/off command for enabling/disabling graphics
	m_alpha_sel = alpha;
}

void hp9845t_state::video_render_buff(unsigned video_scanline , unsigned line_in_row, bool buff_idx)
{
	if (!m_video_buff[ buff_idx ].full) {
		m_video_blanked = true;
	}

	const pen_t *pen = m_palette->pens();

	if (m_video_blanked || !m_alpha_sel) {
		// Blank scanline
		for (unsigned i = 0; i < VIDEO_780_ALPHA_L_LIM; i++) {
			m_bitmap.pix(video_scanline , i) = pen[ PEN_BLACK ];
		}
		if (!m_graphic_sel) {
			for (unsigned i = VIDEO_780_ALPHA_L_LIM; i < VIDEO_780_ALPHA_R_LIM; i++) {
				m_bitmap.pix(video_scanline , i) = pen[ PEN_BLACK ];
			}
		}
		for (unsigned i = VIDEO_780_ALPHA_R_LIM; i < VIDEO_TOT_HPIXELS; i++) {
			m_bitmap.pix(video_scanline , i) = pen[ PEN_BLACK ];
		}
	} else {
		bool cursor_line = line_in_row == 12;
		bool ul_line = line_in_row == 14;
		unsigned video_frame = (unsigned)m_screen->frame_number();
		bool cursor_blink = BIT(video_frame , 3);
		bool char_blink = BIT(video_frame , 4);

		for (unsigned i = 0; i < 80; i++) {
			uint8_t attrs = m_video_buff[ buff_idx ].attrs[ i ];
			uint16_t pixels;

			if (!BIT(attrs , 2) || char_blink) {
				if (ul_line && BIT(attrs , 3)) {
					pixels = ~0;
				} else {
					// The 98780A uses two identical 4KB ROMs interlaced to keep up with the speed of
					// the video circuit. Each of the 4K ROMs contains the full character set.
					// The 98780A fills row 0 (space between characters) controlled by row 1 and 2 from
					// the character ROM, thereby providing line drawing characters for continuous lines
					uint8_t charcode = m_video_buff[ buff_idx ].chars[ i ];
					uint16_t chrgen_addr = ((uint16_t)(charcode ^ 0xff) << 4) | (line_in_row + 1);
					pixels = (uint16_t)m_chargen[ chrgen_addr ] << 1;
					if ((charcode & 0xe0) == 0xe0 && (pixels & 0x6) == 0x6) {
						pixels |= 1;
					}
				}
			} else {
				pixels = 0;
			}

			if (cursor_blink && BIT(attrs , 0)) {
				if (m_back_arrow_cursor) {
					// back arrow cursor (HP's hardware easter egg)
					pixels |= m_back_arrow_shape[ line_in_row ];
				} else if (cursor_line) {
					pixels = ~0;
				}
			}

			if (BIT(attrs , 1)) {
				pixels = ~pixels;
			}

			for (unsigned j = 0; j < 9; j++) {
				bool pixel = (pixels & (1U << j)) != 0;
				unsigned x = i * 9 + j;

				if (m_graphic_sel && x >= VIDEO_780_ALPHA_L_LIM && x < VIDEO_780_ALPHA_R_LIM) {
					// alpha overlays graphics (non-dominating)
					if (pixel) {
						m_bitmap.pix(video_scanline , x) = pen[ PEN_ALPHA ];
					}
				} else {
					// Graphics disabled or alpha-only zone
					m_bitmap.pix(video_scanline , x) = pen[ pixel ? PEN_ALPHA : PEN_BLACK ];
				}
			}
		}
	}
}

void hp9845t_state::graphic_video_render(unsigned video_scanline)
{
	// video_scanline is 0-based, i.e. the topmost visible line of graphic screen is 0
	const pen_t *pen = m_palette->pens();
	bool yc, yw;
	uint16_t word;

	if (m_gv_cursor_fs) {
		// Full-screen cursor
		yw = false;
		yc = video_scanline == m_gv_cursor_y;
	} else if (m_gv_cursor_gc) {
		// 9 x 9 crosshair
		int cursor_y_top = get_wrapped_scanline(m_gv_cursor_y);
		int int_scanline = (int)video_scanline;
		yw = int_scanline >= cursor_y_top && int_scanline <= (cursor_y_top + 8);
		yc = int_scanline == cursor_y_top + 4;
	} else {
		// 9-pixel blinking line
		int cursor_y_top = get_wrapped_scanline(m_gv_cursor_y);
		int int_scanline = (int)video_scanline;
		yw = false;
		yc = int_scanline == cursor_y_top + 4;
	}

	unsigned mem_idx = get_gv_mem_addr(m_gv_scan_start_x >> 4, video_scanline + m_gv_scan_start_y);
	for (unsigned i = 0; i < GVIDEO_HPIXELS; i += 16) {
		word = m_graphic_mem[ mem_idx ];
		mem_idx++;
		// Is wraparound better?
		if (mem_idx > GVIDEO_ADDR_MASK) return;
		unsigned x = i;
		for (uint16_t mask = 0x8000; mask != 0; mask >>= 1) {
			bool cursor = false;
			unsigned pixel;
			if (m_gv_cursor_fs) {
				// Full-screen cursor
				cursor = yc || (x + 184) == m_gv_cursor_x;
			} else if (m_gv_cursor_gc) {
				bool xc = (x + 184) == m_gv_cursor_x;
				bool xw = (x + 180) <= m_gv_cursor_x && (x + 188) >= m_gv_cursor_x;

				// 9 x 9 crosshair
				cursor = (yc && xw) || (yw && xc);
			} else if (BIT(m_screen->frame_number() , 3)) {
				// 9-pixel blinking line
				cursor = yc && (x + 188) >= m_gv_cursor_x && (x + 180) <= m_gv_cursor_x;
			}
			if (cursor) {
				pixel = PEN_CURSOR;
			} else {
				// Normal pixel
				if (m_gv_lp_fullbright)
					pixel = word & mask ? PEN_LP : PEN_BLACK;
				else
					pixel = word & mask ? PEN_GRAPHIC : PEN_BLACK;
			}
			m_bitmap.pix(video_scanline , VIDEO_780_ALPHA_L_LIM + x++) = pen[ pixel ];
		}
	}
}

void hp9845t_state::plot(uint16_t x, uint16_t y, bool draw_erase)
{
	uint16_t addr, pixel_mask;
	bool do_draw;

	pixel_mask = 0x8000 >> (x & 0xf);
	addr = get_gv_mem_addr(x >> 4 , y);
	if (BIT(m_gv_rb_control, 1)) {
		// save graphics memory to rubber band memory
		if (m_graphic_mem[ addr ] & pixel_mask)
			m_gv_rb_memory[m_gv_rb_counter/16] |= 0x1 << (m_gv_rb_counter % 16);        // set
		else
			m_gv_rb_memory[m_gv_rb_counter/16] &= ~(0x1 << (m_gv_rb_counter % 16));     // clear
		m_gv_rb_counter++;
		if (m_gv_rb_counter > 4095) {
			m_gv_stat = true;   // we might prevent data corruption here, but the original hardware doesn't
			m_gv_rb_counter = 0;
		}
	} else if (BIT(m_gv_rb_control, 0)) {
		// restore graphics memory from rubber band memory
		if (BIT(m_gv_rb_memory[m_gv_rb_counter / 16], m_gv_rb_counter % 16))
			m_graphic_mem[ addr ] |= pixel_mask;        // set
		else
			m_graphic_mem[ addr ] &= ~pixel_mask;   // clear
		m_gv_rb_counter++;
		if (m_gv_rb_counter > 4095) {
			m_gv_stat = true;
			m_gv_rb_counter = 0;
		}
	} else {
		// draw/erase pixel
		do_draw = m_gv_memory_control ? draw_erase : !draw_erase;
		if (do_draw)
			m_graphic_mem[ addr ] |= pixel_mask;
		else
			m_graphic_mem[ addr ] &= ~pixel_mask;
	}
}

void hp9845t_state::draw_arc(uint16_t x0, uint16_t y0, int xstart, int ystart, uint16_t radius, int quadrant, int& draw_counter)
{
	int i, x1, y1, d;
	bool draw_erase;
	bool do_plot = (xstart < 0) || (ystart < 0);
	int save_x1[560];

	/* quadrants:
	 *
	 *    1 | 0
	 *   ---+---
	 *    2 | 3
	 *
	 *  Note: we are using Horn's algorithm here, however it is not 100% pixel
	 *  accurate with the original vector generator
	 */

	d = -radius;
	x1 = radius;
	y1 = 0;
	i = 0;
	while ((x1 > 0) && (draw_counter > 0)) {
		// check for plot start
		if  (!do_plot) {
			if ((quadrant % 2) == 0) {
				if ((x1 <= xstart) && (y1 >= ystart)) do_plot = true;
			} else {
				if ((y1 >= xstart) && (x1 <= ystart)) do_plot = true;
			}
		}

		// draw pixels in quadrants
		draw_erase = BIT(m_gv_line_type_mask, 15);
		if (do_plot) {
			switch (quadrant) {
			case 0:
				plot(x0 + x1, y0 - y1, draw_erase);     // quadrant 0
				break;
			case 1:
				plot(x0 - y1, y0 - x1, draw_erase);     // quadrant 1
				break;
			case 2:
				plot(x0 - x1, y0 + y1, draw_erase);     // quadrant 2
				break;
			case 3:
				plot(x0 + y1, y0 + x1, draw_erase);     // quadrant 3
				break;
			}
		}

		// update coordinates
		if (x1 > y1) {
			save_x1[i++] = x1;
			d += 2 * y1 + 1;
			y1++;
			if (do_plot) draw_counter--;
			if (d > 0) {
				x1--;
				if (do_plot) draw_counter--;
				d -= 2 * x1;
			}
			if (x1 <= y1) draw_counter++;
		}
		else {
			x1--;
			y1 = save_x1[--i];
			if (do_plot) {
				draw_counter--;
				if (save_x1[i] != save_x1[i+1])
					draw_counter--;
			}
		}
		update_line_pattern();
	}
}

void hp9845t_state::draw_full_arc(int x0 , int y0 , int dx , int dy , int draw_counter)
{
	// radius
	int radius = sqrt(dx * dx + dy * dy);

	LOG("midpoint = (%d,%d) radius = %d ctrl = %d count = %d\n", x0, y0, radius, m_gv_memory_control, draw_counter);

	/* quadrants:
	 *
	 *    1 | 0
	 *   ---+---
	 *    2 | 3
	 */

	int quadrant = 0;

	// determine the start quadrant
	if (dx > 0)
		quadrant = (dy < 0) ? 1 : 2;
	else
		quadrant = (dy <= 0) ? 0 : 3;

	draw_arc(x0, y0, abs(dx), abs(dy), radius, quadrant, draw_counter);
	while (draw_counter > 0) {
		quadrant++;
		if (quadrant > 3) {
			quadrant = 0;
		}
		draw_arc(x0, y0, -1, -1, radius, quadrant, draw_counter);
	}
}

void hp9845t_state::advance_gv_fsm(bool ds , bool trigger)
{
	if (!m_gv_gr_en) {
		return;
	}

	bool get_out = false;

	attotime time_mem_av;

	do {
		switch (m_gv_fsm_state) {
		case GV_STAT_WAIT_DS_0:
			// inital state (same as GV_STAT_RESET), command received
			if (m_gv_cmd == 0x9) {
				// read words command
				if (m_gv_last_cmd != m_gv_cmd) {
					m_gv_io_counter = get_gv_mem_addr(m_gv_word_x_position , m_gv_word_y_position);
				}
				LOG("read words, last = %x\n", m_gv_last_cmd);
				m_gv_fsm_state = GV_STAT_WAIT_MEM_0;    // -> read stream
				m_gv_last_cmd = m_gv_cmd;
			} else if (m_gv_cmd == 0xd) {
				// fast clear/set command
				m_gv_fsm_state = GV_STAT_WAIT_MEM_2;
				m_gv_last_cmd = m_gv_cmd;
			} else if (ds) {
				if (m_gv_cmd == 0x8) {
					// write words command
					if (m_gv_last_cmd != m_gv_cmd) {
						m_gv_io_counter = get_gv_mem_addr(m_gv_word_x_position , m_gv_word_y_position);
					}
					LOG("write words\n");
					m_gv_fsm_state = GV_STAT_WAIT_TRIG_1;   // -> write stream
				} else {
					// any other command
					m_gv_fsm_state = GV_STAT_WAIT_TRIG_0;   // -> wait for trigger
				}
				m_gv_last_cmd = m_gv_cmd;
			} else {
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_TRIG_0:
			// process data on R4 or R6
			switch (m_gv_cmd) {
			case 0x1:   // load end points
				m_gv_ypt = m_gv_data_w & 0x3ff;
				LOG("load end points y = %d\n", m_gv_ypt);
				break;
			case 0x3:   // load arc
				m_gv_arc_parm = 0;
				m_gv_arc[ m_gv_arc_parm ] = m_gv_data_w;
				LOG("load arc parm%d = %04x\n", m_gv_arc_parm, m_gv_arc[m_gv_arc_parm]);
				m_gv_arc_parm++;
				break;
			case 0x5:   // load scan
				m_gv_scan_start_x = m_gv_data_w & 0x3ff;    // 0..559
				LOG("load scan x = %d\n", m_gv_scan_start_x);
				break;
			case 0x6:   // set line type/area fill
				m_gv_line_type_area_fill = m_gv_data_w & 0x1ff;
				if (BIT(m_gv_line_type_area_fill, 4)) {
					m_gv_line_type_mask = m_line_type[ m_gv_line_type_area_fill & 0x7 ];
					m_gv_repeat_count = 0;
				}
				LOG("set line type = %04x\n", m_gv_line_type_area_fill);
				break;
			case 0x7:   // load X/Y I/O address
				m_gv_word_y_position = m_gv_data_w & 0x1ff; // 0..454
				LOG("load X/Y I/O adress y = %04x\n", m_gv_word_y_position);
				break;
			case 0xa:   // load memory control
				// A single bit is saved (InvBit)
				m_gv_memory_control = (m_gv_data_w & 0x9) == 9 || (m_gv_data_w & 0x12) == 0x12 || (m_gv_data_w & 0x24) == 0x24;
				LOG("load memory control = %04x\n", m_gv_memory_control);
				break;
			case 0xb:   // video on/off - enable graphics video output (1=on 2=off)
				m_graphic_sel = BIT(m_gv_data_w, 0);
				LOG("video on/off parm = %d\n", m_gv_data_w & 0x3);
				break;
			case 0xc:   // load color mask (no effect, just for compatibility with 9845c), takes a single word as parameter
				break;
			case 0xe:   // Y cursor position
				m_gv_lyc = m_gv_data_w;
				break;
			case 0xf:   // X cursor position
				m_gv_lxc = m_gv_data_w;
				break;
			default:
				LOG("unknown 98780A command = %d, parm = 0x%04x\n", m_gv_cmd, m_gv_data_w);
			}
			if ((m_gv_cmd == 0x1) || (m_gv_cmd == 0x3) || (m_gv_cmd == 0x5) || (m_gv_cmd == 0x7)) {
				m_gv_fsm_state = GV_STAT_WAIT_DS_2;     // -> get second data word
			} else {
				get_out = true;
				m_gv_fsm_state = GV_STAT_WAIT_DS_0;     // -> done
			}
			break;

		case GV_STAT_WAIT_MEM_0:
			// process data during read transfer
			time_mem_av = time_to_gv_mem_availability();
			if (time_mem_av.is_zero()) {
				// Read a word from graphic memory
				m_gv_data_r = m_graphic_mem[ m_gv_io_counter ];
				LOG("read words @%04x = %04x\n" , m_gv_io_counter , m_gv_data_r);
				m_gv_io_counter = (m_gv_io_counter + 1) & GVIDEO_ADDR_MASK;
				m_gv_fsm_state = GV_STAT_WAIT_DS_1;     // -> proceed with read stream
			} else {
				m_gv_timer->adjust(time_mem_av);
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_DS_1:
			// wait for data word to be read
			if (ds) {
				// -- next word
				m_gv_fsm_state = GV_STAT_WAIT_MEM_0;    // -> process data word
			} else {
				// -- done
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_DS_2:
			// wait for data word to be written
			if (ds) {
				// -- next word
				m_gv_fsm_state = GV_STAT_WAIT_TRIG_1;   // -> process data word
			} else {
				// done
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_TRIG_1:
			// process multi-word parameters & data during write transfer
			switch (m_gv_cmd) {
			case 0x1:
				// load endpoints command
				m_gv_xpt = m_gv_data_w & 0x3ff;
				// RB control is actually set whenever a data word is written into R4/R6 register, not just here
				m_gv_rb_control = (m_gv_data_w >> 13) & 0x7;
				if (BIT(m_gv_rb_control, 2)) {
					m_gv_rb_counter = 0;
					m_gv_stat = false;
				}
				if (BIT(m_gv_data_w, 11)) {
					// draw vector
					LOG("load end points x = %d, rb = %d (draw)\n", m_gv_xpt, m_gv_rb_control);
					m_gv_fsm_state = GV_STAT_WAIT_MEM_2;    // -> proceed with draw vector
				} else {
					LOG("load end points x = %d, rb = %d (move)\n", m_gv_xpt, m_gv_rb_control);
					m_gv_last_xpt = m_gv_xpt;
					m_gv_last_ypt = m_gv_ypt;
					m_gv_fsm_state = GV_STAT_WAIT_DS_0;     // -> proceed with next word pair
				}
				break;

			case 0x3:
				// load arc
				m_gv_arc[ m_gv_arc_parm ] = m_gv_data_w;
				LOG("load arc parm%d = %04x\n", m_gv_arc_parm, m_gv_arc[m_gv_arc_parm]);
				m_gv_arc_parm++;
				if (m_gv_arc_parm < 4) {
					m_gv_fsm_state = GV_STAT_WAIT_DS_2;     // -> proceed with next word
				} else {
					m_gv_fsm_state = GV_STAT_WAIT_MEM_2;    // -> proceed with draw vector
				}
				break;

			case 0x5:
				// load scan
				m_gv_scan_start_y = m_gv_data_w & 0x3ff;    // 0..454
				LOG("load scan y = %d\n", m_gv_scan_start_y);
				m_gv_fsm_state = GV_STAT_WAIT_DS_0;
				break;

			case 0x7:
				// load X/Y I/O address
				m_gv_word_x_position = (m_gv_data_w & 0x3f0) >> 4;  // 0..34
				m_gv_increment_to_next_row = BIT(m_gv_data_w, 11);
				m_gv_io_counter = get_gv_mem_addr(m_gv_word_x_position , m_gv_word_y_position);
				LOG("load X/Y I/O adress x = %04x increment = %d\n", m_gv_word_x_position, m_gv_increment_to_next_row);
				m_gv_fsm_state = GV_STAT_WAIT_DS_0;
				break;

			case 0x8:
				// write words command
				m_gv_fsm_state = GV_STAT_WAIT_MEM_1;        // -> proceed with next word
				break;
			}
			break;

		case GV_STAT_WAIT_MEM_1:
			// -- transfer from bus to graphics memory to bus within write transfer
			time_mem_av = time_to_gv_mem_availability();
			if (time_mem_av.is_zero()) {
				// Write a full word to graphic memory
				LOG("write words @%04x = %04x\n" , m_gv_io_counter , m_gv_data_w);
				m_graphic_mem[ m_gv_io_counter ] = m_gv_data_w;
				if (!m_gv_increment_to_next_row || (m_gv_word_x_position < 34)) {
					m_gv_io_counter = (m_gv_io_counter + 1) & GVIDEO_ADDR_MASK;
				}
				m_gv_fsm_state = GV_STAT_WAIT_DS_2;             // -> proceed with write stream
			} else {
				m_gv_timer->adjust(time_mem_av);
				get_out = true;
			}
			break;

		case GV_STAT_WAIT_MEM_2:
			// vector generator
			time_mem_av = time_to_gv_mem_availability();
			if (time_mem_av.is_zero()) {
				if (m_gv_cmd == 0xd) {
					// fast clear/set command
					if (m_gv_memory_control) {
						LOG("fast clear/set (set)\n");
						for (auto& el : m_graphic_mem) {
							el = 0xffff;
						}
					} else {
						LOG("fast clear/set (clear)\n");
						for (auto& el : m_graphic_mem) {
							el = 0;
						}
					}
				} else {
					if (m_gv_cmd == 0x3) {
						// draw arc/circle
						// drawing is performed counter-clockwise by using Horn's algorithm
						// m_gv_arc[0] is the delta last load endpoint y coordinate minus the midpoint y coordinate
						// m_gv_arc[1] is the delta last load endpoint x coordinate minus the midpoint x coordinate
						// m_gv_arc[2] is the (probably) the start count (?), actually ignored
						// m_gv_arc[3] is the total horizontal + vertical count for the 4 quadrants counter-clockwise, starting at the last load endpoint (equals 4 times radius for full circles)
						LOG("arc draw\n");

						// midpoint
						int dx = BIT(m_gv_arc[ 1 ] , 15) ? (int)m_gv_arc[ 1 ] - 65536 : m_gv_arc[ 1 ] & 0x7ff;
						int dy = BIT(m_gv_arc[ 0 ] , 15) ? (int)m_gv_arc[ 0 ] - 65536 : m_gv_arc[ 0 ];
						int x0 = m_gv_xpt + dx;
						int y0 = m_gv_ypt - dy;

						draw_full_arc(x0 , y0 , dx , dy , m_gv_arc[ 3 ]);
					} else if (BIT (m_gv_line_type_area_fill, 4)) {
						unsigned x0;
						unsigned x1;
						unsigned y0;
						unsigned y1;

						LOG("line draw (%d,%d)->(%d,%d)\n", m_gv_last_xpt, m_gv_last_ypt, m_gv_xpt, m_gv_ypt);

						// vector generator uses normalization
						if (m_gv_xpt > m_gv_last_xpt) {
							x0 = m_gv_last_xpt;
							y0 = m_gv_last_ypt;
							x1 = m_gv_xpt;
							y1 = m_gv_ypt;
						} else {
							x0 = m_gv_xpt;
							y0 = m_gv_ypt;
							x1 = m_gv_last_xpt;
							y1 = m_gv_last_ypt;
						}
						draw_line(x0 , y0 , x1 , y1);
					} else {
						// fill area with pattern
						LOG("area fill (%d,%d) -> (%d,%d) pattern=%04x\n", m_gv_last_xpt, m_gv_last_ypt, m_gv_xpt, m_gv_ypt, m_gv_line_type_area_fill);

						pattern_fill(m_gv_xpt , m_gv_ypt , m_gv_last_xpt , m_gv_last_ypt , 15 - (m_gv_line_type_area_fill & 0xf));
					}
					m_gv_last_xpt = m_gv_xpt;
					m_gv_last_ypt = m_gv_ypt;
				}
				m_gv_fsm_state = GV_STAT_WAIT_DS_0;
			} else {
				m_gv_timer->adjust(time_mem_av);
			}
			get_out = true;
			break;

		default:
			logerror("Invalid state reached %d\n" , m_gv_fsm_state);
			m_gv_fsm_state = GV_STAT_RESET;
		}

		ds = false;
	} while (!get_out);

	update_graphic_bits();
}

void hp9845t_state::update_graphic_bits()
{
	bool gv_ready = (m_gv_lp_int_en && m_gv_lp_status) || (m_gv_sk_en && m_gv_sk_status);

	if (m_gv_gr_en && !gv_ready) {
		gv_ready = m_gv_fsm_state == GV_STAT_WAIT_DS_0 ||
			m_gv_fsm_state == GV_STAT_WAIT_DS_1 ||
			m_gv_fsm_state == GV_STAT_WAIT_DS_2;
	}

	// WARNING! Race conditions here!
	// FLG and IRQ are raised together. In enhgfxb ROM a SFC instruction
	// that spins on itself waiting for FLG to be true was getting
	// stuck because the interrupt always took precedence (and the ISR
	// cleared the FLG bit).
	// In real hw there was a non-zero chance that SFC exited the loop before
	// interrupt was serviced. In case SFC stayed in the loop, it got another
	// chance at the next interrupt.
	// Fix for this problem is in commit 27004d00
	// My apologies to Tony Duell for doubting at one point the correctness
	// of his 98780A schematics.. :)
	m_io_sys->set_flg(GVIDEO_PA , gv_ready);

	bool irq = m_gv_int_en && !m_gv_dma_en && gv_ready;

	m_io_sys->set_irq(GVIDEO_PA , irq);

	bool dmar = gv_ready && m_gv_dma_en;

	m_io_sys->set_dmar(GVIDEO_PA , dmar);
}

void hp9845t_state::update_gcursor()
{
	m_gv_cursor_fs = (m_gv_lyc & 0x3) == 0;
	m_gv_cursor_gc = !BIT(m_gv_lyc , 1);
	m_gv_cursor_y = (~m_gv_lyc >> 7) & 0x1ff;
	m_gv_cursor_x = (m_gv_lxc >> 6) & 0x3ff;
}

const uint8_t hp9845t_state::m_back_arrow_shape[] = {
	0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xfc,
	0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00, 0x00
};

void hp9845_state::hp9845a(machine_config &config)
{
	//HP_5061_3010(config, m_lpu, XTAL(11'400'000));
	//HP_5061_3011(config, m_ppu, XTAL(11'400'000));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(hp9845_state::screen_update));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(560, 455);
	screen.set_visarea(0, 560-1, 0, 455-1);

	SOFTWARE_LIST(config, "optrom_list").set_original("hp9845a_rom");
}

void hp9845_state::hp9835a(machine_config &config)
{
	//HP_5061_3001(config, m_lpu, XTAL(11'400'000));
	//HP_5061_301(1config, m_ppu, XTAL(11'400'000));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(hp9845_state::screen_update));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(560, 455);
	screen.set_visarea(0, 560-1, 0, 455-1);

	SOFTWARE_LIST(config, "optrom_list").set_original("hp9835a_rom");
}

/*
    Global memory map in blocks of 32 kwords / 64 kbytes each:

    block  0: 0x000000 - 0x007fff (LPU RAM)
    block  1: 0x008000 - 0x00ffff (PPU RAM, only 0x00c000 - 0x00ffff used)
    block  2: 0x010000 - 0x017fff (unused)
    block  3: 0x018000 - 0x01ffff (LPU system ROM)
    block  4: 0x020000 - 0x027fff (LPU RAM)
    block  5: 0x028000 - 0x02ffff (PPU system ROM)
    block  6: 0x030000 - 0x037fff (LPU RAM)
    block  7: 0x038000 - 0x03ffff (LPU option ROM)
    block 10: 0x040000 - 0x047fff (LPU RAM)
    block 11: 0x048000 - 0x04ffff (PPU option ROM)
    block 12: 0x050000 - 0x057fff (LPU RAM)
    block 13: 0x058000 - 0x05ffff (LPU option ROM)
    block 14: 0x060000 - 0x067fff (LPU RAM)
    block 15: 0x068000 - 0x06ffff (PPU option ROM)
    block 16: 0x070000 - 0x077fff (LPU RAM)
    block 17: 0x078000 - 0x07ffff (unused)

    notes:
    - all block numbers are octal
    - blocks 20 to 76 are reserved for 512 kbyte RAM boards (p/n 09845-66590)
    - block 45 is reserved for the Test ROM
    - memory addresses are continuous (for convenience, the mapping below uses block numbers as
      address part above 0xffff, so there are gaps between 0x8000 and 0xffff which are masked out).
    - all LPU RAM is dynamically mapped at machine start according to -ramsize option
*/

void hp9845_base_state::global_mem_map(address_map &map)
{
	map.global_mask(0x3f7fff);
	map.unmap_value_low();
	map(0x014000, 0x017fff).ram().share("ppu_ram");
	map(0x030000, 0x037fff).rom().region("lpu", 0);
	map(0x050000, 0x057fff).rom().region("ppu", 0);
}

void hp9845_base_state::ppu_io_map(address_map &map)
{
	map.unmap_value_low();
	// PA = 0, IC = 0..1
	// Internal printer
	map(HP_MAKE_IOADDR(PRINTER_PA, 0), HP_MAKE_IOADDR(PRINTER_PA, 1)).rw("printer", FUNC(hp9845_printer_device::printer_r), FUNC(hp9845_printer_device::printer_w));
	// PA = 0, IC = 2
	// Keyboard scancode input
	map(HP_MAKE_IOADDR(0, 2), HP_MAKE_IOADDR(0, 2)).r(FUNC(hp9845_base_state::kb_scancode_r));
	// PA = 0, IC = 3
	// Keyboard status input & keyboard interrupt clear
	map(HP_MAKE_IOADDR(0, 3), HP_MAKE_IOADDR(0, 3)).rw(FUNC(hp9845_base_state::kb_status_r), FUNC(hp9845_base_state::kb_irq_clear_w));
	// PA = 13, IC = 0..3
	// Graphic video
	map(HP_MAKE_IOADDR(GVIDEO_PA, 0), HP_MAKE_IOADDR(GVIDEO_PA, 3)).rw(FUNC(hp9845_base_state::graphic_r), FUNC(hp9845_base_state::graphic_w));
	// PA = 14, IC = 0..3
	// Left-hand side tape drive (T14)
	map(HP_MAKE_IOADDR(T14_PA, 0), HP_MAKE_IOADDR(T14_PA, 3)).rw(m_t14, FUNC(hp_taco_device::reg_r), FUNC(hp_taco_device::reg_w));
	// PA = 15, IC = 0..3
	// Right-hand side tape drive (T15)
	map(HP_MAKE_IOADDR(T15_PA, 0), HP_MAKE_IOADDR(T15_PA, 3)).rw(m_t15, FUNC(hp_taco_device::reg_r), FUNC(hp_taco_device::reg_w));
}

void hp9845_base_state::hp9845_base(machine_config &config)
{
	HP_5061_3001(config , m_lpu , 5700000);
	// Clock scaling takes into account the slowdown caused by DRAM refresh
	m_lpu->set_clock_scale(0.93);
	m_lpu->set_addrmap(AS_PROGRAM , &hp9845_base_state::global_mem_map);
	m_lpu->set_9845_boot_mode(true);
	m_lpu->set_rw_cycles(6 , 6);
	m_lpu->set_relative_mode(true);
	HP_5061_3001(config , m_ppu , 5700000);
	m_ppu->set_clock_scale(0.93);
	m_ppu->set_addrmap(AS_PROGRAM , &hp9845_base_state::global_mem_map);
	m_ppu->set_addrmap(AS_IO , &hp9845_base_state::ppu_io_map);
	m_ppu->set_9845_boot_mode(true);
	m_ppu->set_rw_cycles(6 , 6);
	m_ppu->set_relative_mode(true);
	m_ppu->set_int_cb(m_io_sys , FUNC(hp98x5_io_sys_device::int_r));
	m_ppu->pa_changed_cb().set(m_io_sys , FUNC(hp98x5_io_sys_device::pa_w));

	HP98X5_IO_SYS(config , m_io_sys , 0);
	m_io_sys->irl().set_inputline(m_ppu, HPHYBRID_IRL);
	m_io_sys->irh().set_inputline(m_ppu, HPHYBRID_IRH);
	m_io_sys->sts().set(m_ppu , FUNC(hp_5061_3001_cpu_device::status_w));
	m_io_sys->flg().set(m_ppu , FUNC(hp_5061_3001_cpu_device::flag_w));
	m_io_sys->dmar().set(m_ppu , FUNC(hp_5061_3001_cpu_device::dmar_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);

	TIMER(config, m_gv_timer).configure_generic(FUNC(hp9845_base_state::gv_timer));

	// Actual keyboard refresh rate should be KEY_SCAN_OSCILLATOR / 128 (2560 Hz)
	TIMER(config, "kb_timer").configure_periodic(FUNC(hp9845_base_state::kb_scan), attotime::from_hz(100));

	// Beeper
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper , KEY_SCAN_OSCILLATOR / 512).add_route(ALL_OUTPUTS , "mono" , 0.50);

	TIMER(config, m_beep_timer).configure_generic(FUNC(hp9845_base_state::beeper_off));

	// Tape drives
	HP_TACO(config , m_t15 , 4000000);
	m_t15->irq().set([this](int state) { m_io_sys->set_irq(T15_PA , state); });
	m_t15->flg().set([this](int state) { m_io_sys->set_flg(T15_PA , state); });
	m_t15->sts().set([this](int state) { m_io_sys->set_sts(T15_PA , state); });
	HP_TACO(config , m_t14 , 4000000);
	m_t14->irq().set([this](int state) { m_io_sys->set_irq(T14_PA , state); });
	m_t14->flg().set([this](int state) { m_io_sys->set_flg(T14_PA , state); });
	m_t14->sts().set([this](int state) { m_io_sys->set_sts(T14_PA , state); });

	// In real machine there were 8 slots for LPU ROMs and 8 slots for PPU ROMs in
	// right-hand side and left-hand side drawers, respectively.
	// Here we do away with the distinction between LPU & PPU ROMs: in the end they
	// are visible to both CPUs at the same addresses.
	HP9845_OPTROM(config, "drawer1", 0);
	HP9845_OPTROM(config, "drawer2", 0);
	HP9845_OPTROM(config, "drawer3", 0);
	HP9845_OPTROM(config, "drawer4", 0);
	HP9845_OPTROM(config, "drawer5", 0);
	HP9845_OPTROM(config, "drawer6", 0);
	HP9845_OPTROM(config, "drawer7", 0);
	HP9845_OPTROM(config, "drawer8", 0);

	// I/O slots
	for (unsigned slot = 0; slot < 4; slot++) {
		auto& finder = m_io_slot[ slot ];
		hp9845_io_slot_device& tmp( HP9845_IO_SLOT(config , finder , 0) );
		tmp.irq().set([this , slot](int state) { set_irq_slot(slot , state); });
		tmp.sts().set([this , slot](int state) { set_sts_slot(slot , state); });
		tmp.flg().set([this , slot](int state) { set_flg_slot(slot , state); });
		tmp.irq_nextsc().set([this , slot](int state) { set_irq_nextsc_slot(slot , state); });
		tmp.sts_nextsc().set([this , slot](int state) { set_sts_nextsc_slot(slot , state); });
		tmp.flg_nextsc().set([this , slot](int state) { set_flg_nextsc_slot(slot , state); });
		tmp.dmar().set([this , slot](int state) { set_dmar_slot(slot , state); });
	}

	// LPU memory options
	RAM(config, RAM_TAG).set_default_size("192K").set_extra_options("64K, 320K, 448K");

	// Internal printer
	hp9845_printer_device& prt{ HP9845_PRINTER(config , "printer" , 0) };
	prt.irq().set(FUNC(hp9845_base_state::prt_irl_w));
	prt.flg().set([this](int state) { m_io_sys->set_flg(PRINTER_PA , state); });
	prt.sts().set([this](int state) { m_io_sys->set_sts(PRINTER_PA , state); });
}

void hp9845b_state::hp9845b(machine_config &config)
{
	hp9845_base(config);
	// video hardware
	m_screen->set_screen_update(FUNC(hp9845b_state::screen_update));
	m_screen->screen_vblank().set(FUNC(hp9845b_state::vblank_w));
	m_screen->set_color(rgb_t::green());
	// These parameters are for alpha video
	m_screen->set_raw(VIDEO_PIXEL_CLOCK , VIDEO_HTOTAL , 0 , VIDEO_HBSTART , VIDEO_VTOTAL , 0 , VIDEO_ACTIVE_SCANLINES);
	PALETTE(config, m_palette).set_entries(4);
	TIMER(config, "scantimer").configure_scanline(FUNC(hp9845b_state::scanline_timer), "screen", 0, 1);

	config.set_default_layout(layout_hp9845b);

	SOFTWARE_LIST(config, "optrom_list").set_original("hp9845b_rom");
}

void hp9845c_state::hp9845c(machine_config &config)
{
	hp9845_base(config);
	// video hardware
	m_screen->set_screen_update(FUNC(hp9845c_state::screen_update));
	m_screen->screen_vblank().set(FUNC(hp9845c_state::vblank_w));
	m_screen->set_raw(VIDEO_770_PIXEL_CLOCK , VIDEO_770_HTOTAL , VIDEO_770_HBEND , VIDEO_770_HBSTART , VIDEO_770_VTOTAL , VIDEO_770_VBEND , VIDEO_770_VBSTART);
	PALETTE(config, m_palette).set_entries(24);
	TIMER(config, "scantimer").configure_scanline(FUNC(hp9845c_state::scanline_timer), "screen", 0, 1);

	SOFTWARE_LIST(config, "optrom_list").set_original("hp9845b_rom");
}

void hp9845t_state::hp9845t(machine_config &config)
{
	hp9845_base(config);
	// video hardware
	m_screen->set_screen_update(FUNC(hp9845t_state::screen_update));
	m_screen->screen_vblank().set(FUNC(hp9845t_state::vblank_w));
	m_screen->set_color(rgb_t::green());
	m_screen->set_raw(VIDEO_780_PIXEL_CLOCK , VIDEO_780_HTOTAL , VIDEO_780_HBEND , VIDEO_780_HBSTART , VIDEO_780_VTOTAL , VIDEO_780_VBEND , VIDEO_780_VBSTART);
	PALETTE(config, m_palette).set_entries(5);
	TIMER(config, "scantimer").configure_scanline(FUNC(hp9845t_state::scanline_timer), "screen", 0, 1);

	SOFTWARE_LIST(config, "optrom_list").set_original("hp9845b_rom");
}

ROM_START( hp9845a )
	ROM_REGION( 0200000, "lpu", ROMREGION_16BIT | ROMREGION_BE )
	ROM_LOAD( "09845-65544-65547-03-system_lpu.bin", 0000000, 0200000, CRC(47beb87f) SHA1(456caefacafcf19435e1e7e68b1c1e4010841664) )

	ROM_REGION( 0200000, "ppu", ROMREGION_16BIT | ROMREGION_BE )
	ROM_LOAD( "09845-65540-65543-01-system_ppu.bin", 0000000, 0160000, CRC(bc0a34cc) SHA1(9ff215f4ba32ad85f144845d15f762a71e35588b) )
ROM_END

#define rom_hp9845s rom_hp9845a

ROM_START( hp9835a )
	ROM_REGION( 0200000, "lpu", ROMREGION_16BIT | ROMREGION_BE )
	ROM_LOAD( "1818-2800-03_00-system-lpu.bin", 0000000, 020000, CRC(e0b0977a) SHA1(5afdc6c725abff70b674e46688d8ab38ccf8f3c1) )
	ROM_LOAD( "1818-2801-03_10-system-lpu.bin", 0020000, 020000, CRC(c51c1e3a) SHA1(798964fa2e7a1fc149ce4400b694630049293119) )
	ROM_LOAD( "1818-2802-03_20-system-lpu.bin", 0040000, 020000, CRC(bba70a7e) SHA1(2d488594493f8dfcd753e462414cc51c24596a2c) )
	ROM_LOAD( "1818-2803-03_30-system-lpu.bin", 0060000, 020000, CRC(65e9eba6) SHA1(a11f5d37e8ed14a428335c43e785d635b02d1129) )
	ROM_LOAD( "1818-2804-03_40-system-lpu.bin", 0100000, 020000, CRC(ef83b695) SHA1(8ca2914609ece2c9c59ebba6ece3fcbc8929aeaf) )
	ROM_LOAD( "1818-2805-03_50-system-lpu.bin", 0120000, 020000, CRC(401d539f) SHA1(00bda59f71632c4d4fc3268c04262bb81ef0eeba) )
	ROM_LOAD( "1818-2806-03_60-system-lpu.bin", 0140000, 020000, CRC(fe353db5) SHA1(0fb52d82d3743008cdebebb20c488e34ce2fca4b) )
	ROM_LOAD( "1818-2807-03_70-system-lpu.bin", 0160000, 020000, CRC(45a3cc5e) SHA1(35c9959331acf7c98ab6a880915b03e3e783a656) )

	ROM_REGION( 0200000, "ppu", ROMREGION_16BIT | ROMREGION_BE )
	ROM_LOAD( "1818-2808-05_00-system-ppu.bin", 0000000, 020000, CRC(d0c96276) SHA1(cc578d586c4eda81469f29eb7cab7f667e0d5977) )
	ROM_LOAD( "1818-2809-05_30-system-ppu.bin", 0060000, 020000, CRC(ccdb7171) SHA1(1d24596bc1219983e7cb81f6987af094f2ca7d81) )
	ROM_LOAD( "1818-2810-05_40-system-ppu.bin", 0100000, 020000, CRC(97487d24) SHA1(823cd16671de8e6ff2c245060c99778acb6ff79c) )
	ROM_LOAD( "1818-2811-05_50-system-ppu.bin", 0120000, 020000, CRC(18aee6fd) SHA1(388d3b2a063ea2cfdfe9fb9f864fa5f08af817b0) )
	ROM_LOAD( "1818-2812-05_60-system-ppu.bin", 0140000, 020000, CRC(c0beeeae) SHA1(a5db36a7f7bad84c1013bd3ec4813c355f72427d) )
	ROM_LOAD( "1818-2813-05_70-system-ppu.bin", 0160000, 020000, CRC(75361bbf) SHA1(40f499c597da5c8c9a55a2a891976d946a54926b) )
ROM_END

#define rom_hp9835b rom_hp9835a

ROM_START( hp9845b )
	ROM_REGION(0x800 , "chargen" , 0)
	ROM_LOAD("chrgen.bin" , 0 , 0x800 , CRC(fe9e844f) SHA1(0c45ae00766ceba94a19bd5e154bd6d23e208cca))

	ROM_REGION(0x800 , "optional_chargen" , 0)
	ROM_LOAD("optional_chrgen.bin" , 0 , 0x800 , CRC(0ecfa63b) SHA1(c295e6393d1503d903c1d2ce576fa597df9746bf))

	ROM_REGION(0x10000, "lpu", ROMREGION_16BIT | ROMREGION_BE)
	ROM_LOAD("9845-lpu-standard-processor.bin", 0, 0x10000, CRC(dc266c1b) SHA1(1cf3267f13872fbbfc035b70f8b4ec6b5923f182))

	ROM_REGION(0x10000, "ppu", ROMREGION_16BIT | ROMREGION_BE)
	ROM_LOAD("9845-ppu-standard-graphics.bin", 0, 0x10000, CRC(f866510f) SHA1(3e22cd2072e3a5f3603a1eb8477b6b4a198d184d))

#if 0
	ROM_REGION( 0200000, "lpu", ROMREGION_16BIT | ROMREGION_BE )
	ROM_LOAD( "1818-0823-0827-03_00-revb-system_lpu.bin", 0000000, 020000, CRC(7e49c781) SHA1(866c9ebd98d94bb6f99692e29d2d83f55b38c4b6) )
	ROM_LOAD( "1818-0823-0827-03_10-revb-system_lpu.bin", 0020000, 020000, CRC(2f819e3d) SHA1(250886378c3ce2253229997007c7bf0be80a8d1d) )
	ROM_LOAD( "1818-0824-0828-03_20-reva-system_lpu.bin", 0040000, 020000, CRC(834f7063) SHA1(5c390ed74671e4663cc80d899d07b69fd1fb4be6) )
	ROM_LOAD( "1818-0824-0828-03_20-revb-system_lpu.bin", 0040000, 020000, CRC(aa221deb) SHA1(7878643405ee45405dc5269c3b6dc9459f39437b) )
	ROM_LOAD( "1818-0824-0828-03_30-reva-system_lpu.bin", 0060000, 020000, CRC(0ebafdb2) SHA1(80733bfb7026d39a294841221d80ec40eafffe34) )
	ROM_LOAD( "1818-0824-0828-03_30-revb-system_lpu.bin", 0060000, 020000, CRC(0ebafdb2) SHA1(80733bfb7026d39a294841221d80ec40eafffe34) )
	ROM_LOAD( "1818-0825-0829-03_40-revb-system_lpu.bin", 0100000, 020000, CRC(beb09a57) SHA1(b832b995fa21c219673f0c7cf215dee70698f4f1) )
	ROM_LOAD( "1818-0825-0829-03_50-revb-system_lpu.bin", 0120000, 020000, CRC(bbb06222) SHA1(b0bfe1b48fac61eb955e27e0ddfbea020e09e0eb) )
	ROM_LOAD( "1818-0826-0830-03_60-revc-system_lpu.bin", 0140000, 020000, CRC(5c1c3abe) SHA1(fa9f99bf7c8a6df5c71e9fd8c807f0a2ff06640d) )
	ROM_LOAD( "1818-0826-0830-03_70-revc-system_lpu.bin", 0160000, 020000, CRC(0c61a266) SHA1(0cfbf482e7f8e99c87b97c77cf178682cd7af7d6) )

	ROM_REGION( 0200000, "lpu_fast", ROMREGION_16BIT | ROMREGION_BE )
	ROM_LOAD( "1818-1506-1502-03_00-reva-system_fast_lpu.bin", 0000000, 020000, CRC(b77194d8) SHA1(6feec8605331783e6f5a2ab6d6cbd9285036e863) )
	ROM_LOAD( "1818-1506-1502-03_10-reva-system_fast_lpu.bin", 0020000, 020000, CRC(bc5557a5) SHA1(282237e561c3f2304cdeb45efa2432748581af45) )
	ROM_LOAD( "1818-1507-1503-03_20-reva-system_fast_lpu.bin", 0040000, 020000, CRC(2ebc71e2) SHA1(a2d39fb24d565465304833dfd0ff87dd5ef26fb3) )
	ROM_LOAD( "1818-1507-1503-03_30-reva-system_fast_lpu.bin", 0060000, 020000, CRC(82e56bc4) SHA1(36201f343382e533c248ddd123507a2e195cca39) )
	ROM_LOAD( "1818-1508-1504-03_40-reva-system_fast_lpu.bin", 0100000, 020000, CRC(70b0fcb0) SHA1(3f7ce60cad0ffec8344f33d584869492c7f73026) )
	ROM_LOAD( "1818-1508-1504-03_50-reva-system_fast_lpu.bin", 0120000, 020000, CRC(935fab96) SHA1(ecb1da2a0bd46e8c0da2875a1af8cf71d8f4bb56) )
	ROM_LOAD( "1818-1509-1505-03_60-reva-system_fast_lpu.bin", 0140000, 020000, CRC(f4119af7) SHA1(72a3e8b8d7d306e55f8adf0e23225bb81bc2b4ba) )
	ROM_LOAD( "1818-1509-1505-03_70-reva-system_fast_lpu.bin", 0160000, 020000, CRC(22fb0864) SHA1(4e1dce32e84ba216dbbd4116f3b22ca7f254f529) )

	ROM_REGION( 0200000, "ppu", ROMREGION_16BIT | ROMREGION_BE )
	ROM_LOAD( "1818-0833-0837-05_40-revc-system_ppu.bin", 0100000, 020000, CRC(d790795c) SHA1(7ba1e245a98379a34833a780898a784049e33b86) )
	ROM_LOAD( "1818-0833-0837-05_40-revd-system_ppu.bin", 0100000, 020000, CRC(49897e40) SHA1(780a9973ff26d40f470e2004fccceb1019f8ba7f) )
	ROM_LOAD( "1818-0833-0837-05_50-revc-system_ppu.bin", 0120000, 020000, CRC(ef8acde4) SHA1(e68648543aac2b841b08d7758949ba1339a83701) )
	ROM_LOAD( "1818-0833-0837-05_50-revd-system_ppu.bin", 0120000, 020000, CRC(54f61d07) SHA1(f807fb8a59cd9cd221f63907e6a86948a0bf7c1d) )
	ROM_LOAD( "1818-0834-0838-05_60-revc-system_ppu.bin", 0140000, 020000, CRC(20f2100a) SHA1(9304f0b069de9233d697588328f9657dbeabc254) )
	ROM_LOAD( "1818-0834-0838-05_60-revd-system_ppu.bin", 0140000, 020000, CRC(454af601) SHA1(54b56e67e855fd2d699a0dbef0b4d2e8c150c39b) )
	ROM_LOAD( "1818-0834-0838-05_70-revc-system_ppu.bin", 0160000, 020000, CRC(43f62491) SHA1(a9489b37b3fa8768ca6e503f346bd023833ae3ac) )
	ROM_LOAD( "1818-0834-0838-05_70-revd-system_ppu.bin", 0160000, 020000, CRC(43f62491) SHA1(a9489b37b3fa8768ca6e503f346bd023833ae3ac) )
	ROM_LOAD( "1818-1899-1898-05_60-reva-system_ppu.bin", 0140000, 020000, CRC(454af601) SHA1(54b56e67e855fd2d699a0dbef0b4d2e8c150c39b) )
	ROM_LOAD( "1818-1899-1898-05_70-reva-system_ppu.bin", 0160000, 020000, CRC(049604f2) SHA1(89bfd8e086bc9365f156966b0a62c3ac720fc627) )

	ROM_REGION( 0200000, "ppu_tops", ROMREGION_16BIT | ROMREGION_BE )
	ROM_LOAD( "1818-0831-0835-05_00-reva-tops_ppu.bin", 0000000, 020000, CRC(7ddce706) SHA1(746e34d3de52a17372af9a9eb1ed4974a4eae656) )
	ROM_LOAD( "1818-0831-0835-05_10-reva-tops_ppu.bin", 0020000, 020000, CRC(d7fc3d47) SHA1(a3d723fe62f047cb0c17d405d07bb0b08d08e830) )
	ROM_LOAD( "1818-1209-1208-05_00-revb-tops_ppu.bin", 0000000, 020000, CRC(0dc90614) SHA1(94c07553a62b2c86414bc95314601f90eb4e4022) )
	ROM_LOAD( "1818-1209-1208-05_10-revb-tops_ppu.bin", 0020000, 020000, CRC(4e362657) SHA1(b09098c0acd56b11ec3b72ff3e8b5a1e14ef3ae8) )
	ROM_LOAD( "1818-1592-1591-05_00-revb-tops_ppu.bin", 0000000, 020000, CRC(8cfe29a8) SHA1(f1007b6b1d3f2b603653880c44cec48b23701263) )
	ROM_LOAD( "1818-1592-1591-05_10-revb-tops_ppu.bin", 0020000, 020000, CRC(95048264) SHA1(36cfddef9d1289fdaf69596e10d95f88a520feae) )

	ROM_REGION( 0200000, "ppu_kbd_us", ROMREGION_16BIT | ROMREGION_BE )
	ROM_LOAD( "1818-0832-0836-05_20-revc-keyboard_us.bin", 0040000, 020000, CRC(3bf6268a) SHA1(65d7dfeaf34c74dbc86ebe5d3bb65c6bd10163cb) )
	ROM_LOAD( "1818-0832-0836-05_30-revc-keyboard_us.bin", 0060000, 020000, CRC(2dfc619c) SHA1(5c54ff502d1344907817210bfdfcab7f8d6b61bd) )

	ROM_REGION( 0200000, "ppu_kbd_de", ROMREGION_16BIT | ROMREGION_BE )
	ROM_LOAD( "1818-0841-0846-05_20-revc-keyboard_german.bin", 0040000, 020000, CRC(76667eca) SHA1(ac63e5d584d1f2da5668d8a9560f927f48e25e03) )
	ROM_LOAD( "1818-0841-0846-05_20-revd-keyboard_german.bin", 0060000, 020000, CRC(3bf6268a) SHA1(65d7dfeaf34c74dbc86ebe5d3bb65c6bd10163cb) )
	ROM_LOAD( "1818-0841-0846-05_30-revc-keyboard_german.bin", 0040000, 020000, CRC(2b83db22) SHA1(6eda714ce05d2d75f4c041e36b6b6df40697d94a) )
	ROM_LOAD( "1818-0841-0846-05_30-revd-keyboard_german.bin", 0060000, 020000, CRC(b4006959) SHA1(584a85f746a3b0c262fdf9e4be8e696c80cfd429) )
#endif
ROM_END

ROM_START( hp9845c )
	ROM_REGION(0x800 , "chargen" , 0)
	ROM_LOAD("chrgen.bin" , 0 , 0x800 , CRC(fe9e844f) SHA1(0c45ae00766ceba94a19bd5e154bd6d23e208cca))

	ROM_REGION(0x800 , "optional_chargen" , 0)
	ROM_LOAD("optional_chrgen.bin" , 0 , 0x800 , CRC(0ecfa63b) SHA1(c295e6393d1503d903c1d2ce576fa597df9746bf))

	ROM_REGION(0x10000, "lpu", ROMREGION_16BIT | ROMREGION_BE)
	ROM_LOAD("9845-lpu-standard-processor.bin", 0, 0x10000, CRC(dc266c1b) SHA1(1cf3267f13872fbbfc035b70f8b4ec6b5923f182))

	ROM_REGION(0x10000, "ppu", ROMREGION_16BIT | ROMREGION_BE)
	ROM_LOAD("9845-ppu-color-enhanced-graphics.bin", 0, 0x10000, CRC(96e11edc) SHA1(3f1da50edb35dfc57ec2ecfd816a8c8230e110bd))
ROM_END

ROM_START( hp9845t )
	ROM_REGION(0x1000 , "chargen" , 0)
	ROM_LOAD("1818-1395.bin" , 0 , 0x1000 , CRC(7b555edf) SHA1(3b08e094635ef02aef9a2e37b049c61bcf1ec037))

	ROM_REGION(0x10000, "lpu", ROMREGION_16BIT | ROMREGION_BE)
	ROM_LOAD("9845-lpu-standard-processor.bin", 0, 0x10000, CRC(dc266c1b) SHA1(1cf3267f13872fbbfc035b70f8b4ec6b5923f182))

	ROM_REGION(0x10000, "ppu", ROMREGION_16BIT | ROMREGION_BE)
	ROM_LOAD("9845-ppu-color-enhanced-graphics.bin", 0, 0x10000, CRC(96e11edc) SHA1(3f1da50edb35dfc57ec2ecfd816a8c8230e110bd))
ROM_END

ROM_START( hp9845b_de )
	ROM_REGION(0x800 , "chargen" , 0)
	ROM_LOAD("chrgen.bin" , 0 , 0x800 , CRC(fe9e844f) SHA1(0c45ae00766ceba94a19bd5e154bd6d23e208cca))

	ROM_REGION(0x800 , "optional_chargen" , 0)
	ROM_LOAD("optional_chrgen.bin" , 0 , 0x800 , CRC(0ecfa63b) SHA1(c295e6393d1503d903c1d2ce576fa597df9746bf))

	ROM_REGION(0x10000, "lpu", ROMREGION_16BIT | ROMREGION_BE)
	ROM_LOAD("9845-lpu-standard-processor.bin", 0, 0x10000, CRC(dc266c1b) SHA1(1cf3267f13872fbbfc035b70f8b4ec6b5923f182))

	ROM_REGION(0x10000, "ppu", ROMREGION_16BIT | ROMREGION_BE)
	ROM_LOAD("9845-ppu-standard-graphics-ger.bin", 0, 0x10000, CRC(c968363d) SHA1(bc6805403371ca49d1a137f22cd254e3b0e0dbb4))
ROM_END

ROM_START( hp9845c_de )
	ROM_REGION(0x800 , "chargen" , 0)
	ROM_LOAD("chrgen.bin" , 0 , 0x800 , CRC(fe9e844f) SHA1(0c45ae00766ceba94a19bd5e154bd6d23e208cca))

	ROM_REGION(0x800 , "optional_chargen" , 0)
	ROM_LOAD("optional_chrgen.bin" , 0 , 0x800 , CRC(0ecfa63b) SHA1(c295e6393d1503d903c1d2ce576fa597df9746bf))

	ROM_REGION(0x10000, "lpu", ROMREGION_16BIT | ROMREGION_BE)
	ROM_LOAD("9845-lpu-standard-processor.bin", 0, 0x10000, CRC(dc266c1b) SHA1(1cf3267f13872fbbfc035b70f8b4ec6b5923f182))

	ROM_REGION(0x10000, "ppu", ROMREGION_16BIT | ROMREGION_BE)
	ROM_LOAD("9845-ppu-color-enhanced-graphics-ger.bin", 0, 0x10000, CRC(a7ef79ee) SHA1(637742ed8fc8201a8e7bac62654f21c5409dfb76))
ROM_END

ROM_START( hp9845t_de )
	ROM_REGION(0x1000 , "chargen" , 0)
	ROM_LOAD("1818-1395.bin" , 0 , 0x1000 , CRC(7b555edf) SHA1(3b08e094635ef02aef9a2e37b049c61bcf1ec037))

	ROM_REGION(0x10000, "lpu", ROMREGION_16BIT | ROMREGION_BE)
	ROM_LOAD("9845-lpu-standard-processor.bin", 0, 0x10000, CRC(dc266c1b) SHA1(1cf3267f13872fbbfc035b70f8b4ec6b5923f182))

	ROM_REGION(0x10000, "ppu", ROMREGION_16BIT | ROMREGION_BE)
	ROM_LOAD("9845-ppu-color-enhanced-graphics-ger.bin", 0, 0x10000, CRC(a7ef79ee) SHA1(637742ed8fc8201a8e7bac62654f21c5409dfb76))
ROM_END

//    YEAR  NAME        PARENT   COMPAT  MACHINE  INPUT           CLASS          INIT        COMPANY            FULLNAME  FLAGS
COMP( 1977, hp9845a,    0,       0,      hp9845a, hp9845,         hp9845_state,  empty_init, "Hewlett-Packard", "9845A",  MACHINE_IS_SKELETON )
COMP( 1977, hp9845s,    hp9845a, 0,      hp9845a, hp9845,         hp9845_state,  empty_init, "Hewlett-Packard", "9845S",  MACHINE_IS_SKELETON )
COMP( 1979, hp9835a,    0,       0,      hp9835a, hp9845,         hp9845_state,  empty_init, "Hewlett-Packard", "9835A",  MACHINE_IS_SKELETON )
COMP( 1979, hp9835b,    hp9835a, 0,      hp9835a, hp9845,         hp9845_state,  empty_init, "Hewlett-Packard", "9835B",  MACHINE_IS_SKELETON )
COMP( 1979, hp9845b,    0,       0,      hp9845b, hp9845_base,    hp9845b_state, empty_init, "Hewlett-Packard", "9845B",  0 )
COMP( 1982, hp9845t,    0,       0,      hp9845t, hp9845ct,       hp9845t_state, empty_init, "Hewlett-Packard", "9845T",  0 )
COMP( 1980, hp9845c,    0,       0,      hp9845c, hp9845ct,       hp9845c_state, empty_init, "Hewlett-Packard", "9845C",  0 )
COMP( 1979, hp9845b_de, hp9845b, 0,      hp9845b, hp9845_base_de, hp9845b_state, empty_init, "Hewlett-Packard", "9845B (Germany)", 0 )
COMP( 1982, hp9845t_de, hp9845t, 0,      hp9845t, hp9845ct_de,    hp9845t_state, empty_init, "Hewlett-Packard", "9845T (Germany)", 0 )
COMP( 1980, hp9845c_de, hp9845c, 0,      hp9845c, hp9845ct_de,    hp9845c_state, empty_init, "Hewlett-Packard", "9845C (Germany)", 0 )
