// license:BSD-3-Clause
// copyright-holders:Curt Coder, F. Ulivi
/*

    HP 9845

    http://www.hp9845.net/

*/
// **************************
// Driver for HP 9845B system
// **************************
//
// What's in:
// - Emulation of both 5061-3001 CPUs
// - LPU & PPU ROMs
// - LPU & PPU RAMs
// - Text mode screen
// - Keyboard (most of keys)
// What's not yet in:
// - Beeper
// - Rest of keyboard
// - Graphic screen
// - Tape drive (this needs some heavy RE of the TACO chip)
// - Better documentation of this file
// - Software list to load optional ROMs
// What's wrong:
// - I'm using character generator from HP64K (another driver of mine): no known dump of the original one
// - Speed, as usual
// - There are a couple of undocumented opcodes that PPU executes at each keyboard interrupt: don't know if ignoring them is a Bad Thing (tm) or not

#include "emu.h"
#include "cpu/z80/z80.h"
#include "softlist.h"
#include "cpu/hphybrid/hphybrid.h"

#define BIT_MASK(n) (1U << (n))

// Macros to clear/set single bits
#define BIT_CLR(w , n)  ((w) &= ~BIT_MASK(n))
#define BIT_SET(w , n)  ((w) |= BIT_MASK(n))

// Base address of video buffer
#define VIDEO_BUFFER_BASE       0x17000

#define MAX_WORD_PER_ROW        600

#define VIDEO_CHAR_WIDTH        9
#define VIDEO_CHAR_HEIGHT       15
#define VIDEO_CHAR_COLUMNS      80
#define VIDEO_CHAR_ROWS         25
#define VIDEO_ACTIVE_SCANLINES  (VIDEO_CHAR_HEIGHT * VIDEO_CHAR_ROWS)

#define KEY_SCAN_OSCILLATOR     327680

class hp9845_state : public driver_device
{
public:
	hp9845_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
	{ }

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

static INPUT_PORTS_START( hp9845 )
INPUT_PORTS_END

class hp9845b_state : public driver_device
{
public:
	hp9845b_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
				m_lpu(*this , "lpu"),
				m_ppu(*this , "ppu"),
				m_palette(*this , "palette"),
		m_io_key0(*this , "KEY0"),
		m_io_key1(*this , "KEY1"),
		m_io_key2(*this , "KEY2"),
		m_io_key3(*this , "KEY3")
	{ }

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

		virtual void machine_start() override;
		virtual void machine_reset() override;

		TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer);

		void vblank_w(screen_device &screen, bool state);

		IRQ_CALLBACK_MEMBER(irq_callback);
	void update_irl(void);

	TIMER_DEVICE_CALLBACK_MEMBER(kb_scan);
	DECLARE_READ16_MEMBER(kb_scancode_r);
	DECLARE_READ16_MEMBER(kb_status_r);
	DECLARE_WRITE16_MEMBER(kb_irq_clear_w);

		DECLARE_WRITE8_MEMBER(pa_w);

private:
		required_device<hp_5061_3001_cpu_device> m_lpu;
		required_device<hp_5061_3001_cpu_device> m_ppu;
	required_device<palette_device> m_palette;
	required_ioport m_io_key0;
	required_ioport m_io_key1;
	required_ioport m_io_key2;
	required_ioport m_io_key3;

		void set_video_mar(UINT16 mar);
		void video_fill_buff(bool buff_idx);
		void video_render_buff(unsigned line_in_row, bool buff_idx);

	// Character generator
	const UINT8 *m_chargen;

		// Text mode video I/F
		typedef struct {
				UINT8 chars[ 80 ];
				UINT8 attrs[ 80 ];
				bool full;
		} video_buffer_t;

	bitmap_rgb32 m_bitmap;
		unsigned m_video_scanline;
		offs_t m_video_mar;
		UINT16 m_video_word;
		bool m_video_load_mar;
		bool m_video_byte_idx;
		UINT8 m_video_attr;
		bool m_video_buff_idx;
		bool m_video_blanked;
		UINT8 m_video_frame;
		video_buffer_t m_video_buff[ 2 ];

		// Interrupt handling
		UINT8 m_irl_pending;

		// State of keyboard
		ioport_value m_kb_state[ 4 ];
		UINT8 m_kb_scancode;
		UINT16 m_kb_status;

};

static INPUT_PORTS_START(hp9845b)
		// Keyboard is arranged in a 8 x 16 matrix. Of the 128 possible positions, 118 are used.
	// Keys are mapped on bit b of KEYn
	// where b = (row & 1) << 4 + column, n = row >> 1
	// column = [0..15]
	// row = [0..7]
	PORT_START("KEY0")
	PORT_BIT(BIT_MASK(0)  , IP_ACTIVE_HIGH , IPT_UNUSED)    // N/U
	PORT_BIT(BIT_MASK(1)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Print All")  // Print All
	PORT_BIT(BIT_MASK(2)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("KP+")        // KP +
	PORT_BIT(BIT_MASK(3)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("KP,")        // KP ,
	PORT_BIT(BIT_MASK(4)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("KP.")        // KP .
        PORT_BIT(BIT_MASK(5)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("KP0")        // KP 0
	PORT_BIT(BIT_MASK(6)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_F12)  PORT_NAME("Execute")    // Execute
	PORT_BIT(BIT_MASK(7)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_F11)  PORT_NAME("Cont")       // Cont
	PORT_BIT(BIT_MASK(8)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))        // Right
	PORT_BIT(BIT_MASK(9)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')  // Space
	PORT_BIT(BIT_MASK(10)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')  // /
	PORT_BIT(BIT_MASK(11)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')  // <
	PORT_BIT(BIT_MASK(12)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_N)     PORT_CHAR('n') PORT_CHAR('N')  // N
	PORT_BIT(BIT_MASK(13)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_V)     PORT_CHAR('v') PORT_CHAR('V')  // V
	PORT_BIT(BIT_MASK(14)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_X)     PORT_CHAR('x') PORT_CHAR('X')  // X
	PORT_BIT(BIT_MASK(15)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)    PORT_CHAR(UCHAR_SHIFT_1)   // Shift
	PORT_BIT(BIT_MASK(16)  , IP_ACTIVE_HIGH , IPT_UNUSED)   // N/U
	PORT_BIT(BIT_MASK(17)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("Auto start") // Auto Start
	PORT_BIT(BIT_MASK(18)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("KP-")        // KP -
	PORT_BIT(BIT_MASK(19)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("KP3")        // KP 3
	PORT_BIT(BIT_MASK(20)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("KP2")        // KP 2
	PORT_BIT(BIT_MASK(21)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("KP1")        // KP 1
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
	PORT_BIT(BIT_MASK(1)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_INSERT)       PORT_NAME("INSCHAR")    // Ins Char
	PORT_BIT(BIT_MASK(2)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("KP*")        // KP *
	PORT_BIT(BIT_MASK(3)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("KP6")        // KP 6
	PORT_BIT(BIT_MASK(4)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("KP5")        // KP 5
	PORT_BIT(BIT_MASK(5)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("KP4")        // KP 4
	PORT_BIT(BIT_MASK(6)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("KP=")        // KP =
	PORT_BIT(BIT_MASK(7)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_F10)  PORT_NAME("Pause")      // Pause
	PORT_BIT(BIT_MASK(8)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_UP)   PORT_CHAR(UCHAR_MAMEKEY(UP))    // Up
	PORT_BIT(BIT_MASK(9)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_ENTER)        PORT_CHAR(13)   // Store
	PORT_BIT(BIT_MASK(10)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(';') PORT_CHAR(':')      // :
	PORT_BIT(BIT_MASK(11)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_K)     PORT_CHAR('k') PORT_CHAR('K')  // K
	PORT_BIT(BIT_MASK(12)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_H)     PORT_CHAR('h') PORT_CHAR('H')  // H
	PORT_BIT(BIT_MASK(13)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F)     PORT_CHAR('f') PORT_CHAR('F')  // F
	PORT_BIT(BIT_MASK(14)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_S)     PORT_CHAR('s') PORT_CHAR('S')  // S
	PORT_BIT(BIT_MASK(15)  , IP_ACTIVE_HIGH , IPT_UNUSED)   // N/U
	PORT_BIT(BIT_MASK(16)  , IP_ACTIVE_HIGH , IPT_UNUSED)   // N/U
	PORT_BIT(BIT_MASK(17)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("INSLN")      // Ins Ln
	PORT_BIT(BIT_MASK(18)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("KP/")        // KP /
	PORT_BIT(BIT_MASK(19)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("KP9")        // KP 9
	PORT_BIT(BIT_MASK(20)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("KP8")        // KP 8
	PORT_BIT(BIT_MASK(21)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("KP7")        // KP 7
	PORT_BIT(BIT_MASK(22)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("Result")     // Result
	PORT_BIT(BIT_MASK(23)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)   PORT_NAME("Run")        // Run
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
	PORT_BIT(BIT_MASK(1)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("DELLN")      // Del Ln
	PORT_BIT(BIT_MASK(2)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("KP^")        // KP ^
	PORT_BIT(BIT_MASK(3)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("KP)")        // KP )
	PORT_BIT(BIT_MASK(4)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("KP(")        // KP (
	PORT_BIT(BIT_MASK(5)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("KPE")        // KP E
	PORT_BIT(BIT_MASK(6)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Clear line") // Clear Line
	PORT_BIT(BIT_MASK(7)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_F8)   PORT_NAME("Stop")       // Stop
	PORT_BIT(BIT_MASK(8)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')      // |
	PORT_BIT(BIT_MASK(9)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(']') PORT_CHAR('}')   // ]
	PORT_BIT(BIT_MASK(10)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_P)     PORT_CHAR('p') PORT_CHAR('P')  // P
	PORT_BIT(BIT_MASK(11)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_I)     PORT_CHAR('i') PORT_CHAR('I')  // I
	PORT_BIT(BIT_MASK(12)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)     PORT_CHAR('y') PORT_CHAR('Y')  // Y
	PORT_BIT(BIT_MASK(13)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_R)     PORT_CHAR('r') PORT_CHAR('R')  // R
	PORT_BIT(BIT_MASK(14)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_W)     PORT_CHAR('w') PORT_CHAR('W')  // W
	PORT_BIT(BIT_MASK(15)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)  PORT_CHAR(UCHAR_SHIFT_2)   // Control
	PORT_BIT(BIT_MASK(16)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("Typwtr")     // Typwtr
	PORT_BIT(BIT_MASK(17)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)  PORT_NAME("DELCHAR")    // Del Char
	PORT_BIT(BIT_MASK(18)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("ROLLDOWN")   // Roll down
	PORT_BIT(BIT_MASK(19)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("ROLLUP")     // Roll up
	PORT_BIT(BIT_MASK(20)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_NAME("HOME")       // Home
	PORT_BIT(BIT_MASK(21)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("Clr to end") // Clr to end
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
	PORT_BIT(BIT_MASK(5)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("K12")        // K12
	PORT_BIT(BIT_MASK(6)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("K11")        // K11
	PORT_BIT(BIT_MASK(7)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("K10")        // K10
        PORT_BIT(BIT_MASK(8)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("K9")         // K9
	PORT_BIT(BIT_MASK(9)  , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("K8")         // K8
	PORT_BIT(BIT_MASK(10)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_0)     PORT_CHAR('0') // 0
	PORT_BIT(BIT_MASK(11)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_8)     PORT_CHAR('8') PORT_CHAR('(')  // 8
	PORT_BIT(BIT_MASK(12)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_6)     PORT_CHAR('6') PORT_CHAR('&')  // 6
	PORT_BIT(BIT_MASK(13)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_4)     PORT_CHAR('4') PORT_CHAR('$')  // 4
	PORT_BIT(BIT_MASK(14)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_2)     PORT_CHAR('2') PORT_CHAR('"')  // 2
	PORT_BIT(BIT_MASK(15)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)   PORT_CHAR('\t')        // Tab
	PORT_BIT(BIT_MASK(16)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("Tab clr")    // Tab clr
        PORT_BIT(BIT_MASK(17)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("Step")  // Step
	PORT_BIT(BIT_MASK(18)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)   PORT_NAME("K7") // K7
	PORT_BIT(BIT_MASK(19)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)   PORT_NAME("K6") // K6
	PORT_BIT(BIT_MASK(20)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)   PORT_NAME("K5") // K5
        PORT_BIT(BIT_MASK(21)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)   PORT_NAME("K4") // K4
        PORT_BIT(BIT_MASK(22)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)   PORT_NAME("K3") // K3
	PORT_BIT(BIT_MASK(23)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)   PORT_NAME("K2") // K2
	PORT_BIT(BIT_MASK(24)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)   PORT_NAME("K1") // K1
	PORT_BIT(BIT_MASK(25)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)  PORT_NAME("K0") // K0
	PORT_BIT(BIT_MASK(26)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('-') PORT_CHAR('_')      // _
	PORT_BIT(BIT_MASK(27)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_9)     PORT_CHAR('9') PORT_CHAR(')')  // 9
	PORT_BIT(BIT_MASK(28)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_7)     PORT_CHAR('7') PORT_CHAR('\'') // 7
	PORT_BIT(BIT_MASK(29)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_5)     PORT_CHAR('5') PORT_CHAR('%')  // 5
	PORT_BIT(BIT_MASK(30)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR('#')  // 3
	PORT_BIT(BIT_MASK(31)  , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_1)     PORT_CHAR('1') PORT_CHAR('!')  // 1

INPUT_PORTS_END

UINT32 hp9845_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

UINT32 hp9845b_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}

void hp9845b_state::machine_start()
{
	machine().first_screen()->register_screen_bitmap(m_bitmap);

		m_chargen = memregion("chargen")->base();
}

void hp9845b_state::machine_reset()
{
		m_lpu->halt_w(1);
		m_ppu->halt_w(0);

		// Some sensible defaults
		m_video_mar = VIDEO_BUFFER_BASE;
		m_video_load_mar = false;
		m_video_byte_idx = false;
		m_video_attr = 0;
		m_video_buff_idx = false;
		m_video_blanked = false;
		m_video_frame = 0;

		m_irl_pending = 0;

		memset(&m_kb_state[ 0 ] , 0 , sizeof(m_kb_state));
		m_kb_scancode = 0x7f;
		m_kb_status = 0;
}

void hp9845b_state::set_video_mar(UINT16 mar)
{
		m_video_mar = (mar & 0xfff) | VIDEO_BUFFER_BASE;
}

void hp9845b_state::video_fill_buff(bool buff_idx)
{
		unsigned char_idx = 0;
		unsigned iters = 0;
		UINT8 byte;
		address_space& prog_space = m_ppu->space(AS_PROGRAM);

		m_video_buff[ buff_idx ].full = false;

		while (1) {
				if (!m_video_byte_idx) {
						if (iters++ >= MAX_WORD_PER_ROW) {
								// Limit on accesses per row reached
								break;
						}
						m_video_word = prog_space.read_word(m_video_mar << 1);
						if (m_video_load_mar) {
								// Load new address into MAR after start of a new frame or NWA instruction
								// TODO: decode graphic/alpha mode bit
								set_video_mar(~m_video_word);
								m_video_load_mar = false;
								continue;
						} else {
								// Read normal word from frame buffer, start parsing at MSB
								set_video_mar(m_video_mar + 1);
								byte = (UINT8)(m_video_word >> 8);
								m_video_byte_idx = true;
						}
				} else {
						// Parse LSB
						byte = (UINT8)(m_video_word & 0xff);
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

void hp9845b_state::video_render_buff(unsigned line_in_row, bool buff_idx)
{
		if (!m_video_buff[ buff_idx ].full) {
				m_video_blanked = true;
		}

		if (m_video_blanked) {
				// TODO: blank scanline
		} else {
		const rgb_t *palette = m_palette->palette()->entry_list_raw();
				bool cursor_line = line_in_row == 12;
				bool ul_line = line_in_row == 14;
				bool cursor_blink = BIT(m_video_frame , 3);
				bool char_blink = BIT(m_video_frame , 4);

				for (unsigned i = 0; i < 80; i++) {
						UINT8 charcode = m_video_buff[ buff_idx ].chars[ i ];
						UINT8 attrs = m_video_buff[ buff_idx ].attrs[ i ];
						UINT8 chargen_byte = m_chargen[ line_in_row  | ((unsigned)charcode << 4) ];
						UINT16 pixels;

						// TODO: Handle selection of 2nd chargen
						// TODO: Check if order of bits in "pixels" is ok

						if ((ul_line && BIT(attrs , 3)) ||
							(cursor_line && cursor_blink && BIT(attrs , 0))) {
								pixels = ~0;
						} else if (char_blink && BIT(attrs , 2)) {
								pixels = 0;
						} else {
								pixels = (UINT16)(chargen_byte & 0x7f) << 2;
						}

						if (BIT(attrs , 1)) {
								pixels = ~pixels;
						}

						for (unsigned j = 0; j < 9; j++) {
								bool pixel = (pixels & (1U << (8 - j))) != 0;

								m_bitmap.pix32(m_video_scanline , i * 9 + j) = palette[ pixel ? 1 : 0 ];
						}
				}
		}
}

TIMER_DEVICE_CALLBACK_MEMBER(hp9845b_state::scanline_timer)
{
		m_video_scanline = param;

		if (m_video_scanline < VIDEO_ACTIVE_SCANLINES) {
				unsigned row = m_video_scanline / 15;
				unsigned line_in_row = m_video_scanline - row * 15;

				if (line_in_row == 0) {
						// Start of new row, swap buffers
						m_video_buff_idx = !m_video_buff_idx;
						video_fill_buff(!m_video_buff_idx);
				}

				video_render_buff(line_in_row , m_video_buff_idx);
		}
}

void hp9845b_state::vblank_w(screen_device &screen, bool state)
{
		// VBlank signal is fed into HALT flag of PPU
		m_ppu->halt_w(state);

		if (state) {
				// Start of V blank
				set_video_mar(0);
				m_video_load_mar = true;
				m_video_byte_idx = false;
				m_video_blanked = false;
				m_video_frame++;
				m_video_buff_idx = !m_video_buff_idx;
				video_fill_buff(!m_video_buff_idx);
		}
}

IRQ_CALLBACK_MEMBER(hp9845b_state::irq_callback)
{
		if (irqline == HPHYBRID_IRL) {
				return m_irl_pending;
		} else {
				return 0;
		}
}

void hp9845b_state::update_irl(void)
{
		m_ppu->set_input_line(HPHYBRID_IRL , m_irl_pending != 0);
}

TIMER_DEVICE_CALLBACK_MEMBER(hp9845b_state::kb_scan)
{
		ioport_value input[ 4 ];
		input[ 0 ] = m_io_key0->read();
		input[ 1 ] = m_io_key1->read();
		input[ 2 ] = m_io_key2->read();
		input[ 3 ] = m_io_key3->read();

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
		if (BIT(input[ 0 ] , 15)) {
				BIT_SET(m_kb_status , 15);
				BIT_CLR(input[ 0 ] , 15);
		} else {
				BIT_CLR(m_kb_status, 15);
		}

		// TODO: handle repeat key
		// TODO: handle ctrl+stop

		for (unsigned i = 0; i < 128; i++) {
				ioport_value mask = BIT_MASK(i & 0x1f);
				unsigned idx = i >> 5;

				if ((input[ idx ] & ~m_kb_state[ idx ]) & mask) {
						// Key pressed, store scancode & generate IRL
						m_kb_scancode = i;
						BIT_SET(m_irl_pending , 0);
						BIT_SET(m_kb_status, 0);
						update_irl();

						// Special case: pressing stop key sets LPU "status" flag
						if (i == 0x47) {
								m_lpu->status_w(1);
						}
				}
		}

		memcpy(&m_kb_state[ 0 ] , &input[ 0 ] , sizeof(m_kb_state));
}

READ16_MEMBER(hp9845b_state::kb_scancode_r)
{
		return ~m_kb_scancode & 0x7f;
}

READ16_MEMBER(hp9845b_state::kb_status_r)
{
		return m_kb_status;
}

WRITE16_MEMBER(hp9845b_state::kb_irq_clear_w)
{
		BIT_CLR(m_irl_pending , 0);
		BIT_CLR(m_kb_status, 0);
		update_irl();
		m_lpu->status_w(0);
		// TODO: beeper start
}

WRITE8_MEMBER(hp9845b_state::pa_w)
{
		// TODO: handle sts & flg
		if (data == 0xf) {
				// RHS tape drive (T15)
				m_ppu->status_w(1);
				m_ppu->flag_w(1);
		} else {
				m_ppu->status_w(0);
				m_ppu->flag_w(0);
		}
}

static MACHINE_CONFIG_START( hp9845a, hp9845_state )
	//MCFG_CPU_ADD("lpu", HP_5061_3010, XTAL_11_4MHz)
	//MCFG_CPU_ADD("ppu", HP_5061_3011, XTAL_11_4MHz)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(hp9845_state, screen_update)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(560, 455)
	MCFG_SCREEN_VISIBLE_AREA(0, 560-1, 0, 455-1)

	MCFG_SOFTWARE_LIST_ADD("optrom_list", "hp9845a_rom")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hp9835a, hp9845_state )
	//MCFG_CPU_ADD("lpu", HP_5061_3001, XTAL_11_4MHz)
	//MCFG_CPU_ADD("ppu", HP_5061_3001, XTAL_11_4MHz)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(hp9845_state, screen_update)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(560, 455)
	MCFG_SCREEN_VISIBLE_AREA(0, 560-1, 0, 455-1)

	MCFG_SOFTWARE_LIST_ADD("optrom_list", "hp9835a_rom")
MACHINE_CONFIG_END

static ADDRESS_MAP_START(global_mem_map , AS_PROGRAM , 16 , hp9845b_state)
        ADDRESS_MAP_GLOBAL_MASK(0x3f7fff)
        ADDRESS_MAP_UNMAP_LOW
        AM_RANGE(0x000000 , 0x007fff) AM_RAM AM_SHARE("lpu_ram")
        AM_RANGE(0x014000 , 0x017fff) AM_RAM AM_SHARE("ppu_ram")
        AM_RANGE(0x030000 , 0x037fff) AM_ROM AM_REGION("lpu" , 0)
        AM_RANGE(0x050000 , 0x057fff) AM_ROM AM_REGION("ppu" , 0)
//AM_RANGE(0x250000 , 0x251fff) AM_ROM AM_REGION("test_rom" , 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(ppu_io_map , AS_IO , 16 , hp9845b_state)
		ADDRESS_MAP_UNMAP_LOW
		// PA = 0, IC = 2
		// Keyboard scancode input
		AM_RANGE(HP_MAKE_IOADDR(0 , 2) , HP_MAKE_IOADDR(0 , 2)) AM_READ(kb_scancode_r)
		// PA = 0, IC = 3
		// Keyboard status input & keyboard interrupt clear
		AM_RANGE(HP_MAKE_IOADDR(0 , 3) , HP_MAKE_IOADDR(0 , 3)) AM_READWRITE(kb_status_r , kb_irq_clear_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( hp9845b, hp9845b_state )
	MCFG_CPU_ADD("lpu", HP_5061_3001, 5700000)
		MCFG_CPU_PROGRAM_MAP(global_mem_map)
		MCFG_HPHYBRID_SET_9845_BOOT(true)
	MCFG_CPU_ADD("ppu", HP_5061_3001, 5700000)
		MCFG_CPU_PROGRAM_MAP(global_mem_map)
	MCFG_CPU_IO_MAP(ppu_io_map)
		MCFG_HPHYBRID_SET_9845_BOOT(true)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(hp9845b_state , irq_callback)
		MCFG_HPHYBRID_PA_CHANGED(WRITE8(hp9845b_state , pa_w))

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(hp9845b_state, screen_update)
		MCFG_SCREEN_RAW_PARAMS(20849400 , 99 * 9 , 0 , 80 * 9 , 26 * 15 , 0 , 25 * 15)
		MCFG_SCREEN_VBLANK_DRIVER(hp9845b_state, vblank_w)
	MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")

	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", hp9845b_state, scanline_timer, "screen", 0, 1)

	// Actual keyboard refresh rate should be KEY_SCAN_OSCILLATOR / 128 (2560 Hz)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("kb_timer" , hp9845b_state , kb_scan , attotime::from_hz(100))

		MCFG_SOFTWARE_LIST_ADD("optrom_list", "hp9845b_rom")
MACHINE_CONFIG_END

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
		ROM_REGION(0x4000 , "test_rom" , ROMREGION_16BIT | ROMREGION_BE)
		ROM_LOAD("09845-66520-45_00-Test_ROM.bin" , 0x0000 , 0x2000 , CRC(95a5b299))
		ROM_LOAD("09845-66520-45_10-Test_ROM.bin" , 0x2000 , 0x2000 , CRC(257e4c66))

	ROM_REGION(0x800 , "chargen" , 0)
		// Don't have the real character generator from HP9845, use the one from HP64000 for now
	ROM_LOAD("1818_2668.bin" , 0 , 0x800 , BAD_DUMP CRC(32a52664) SHA1(8b2a49a32510103ff424e8481d5ed9887f609f2f))

		ROM_REGION(0x10000, "lpu", ROMREGION_16BIT | ROMREGION_BE)
		ROM_LOAD("9845-LPU-Standard-Processor.bin", 0, 0x10000, CRC(dc266c1b) SHA1(1cf3267f13872fbbfc035b70f8b4ec6b5923f182))

		ROM_REGION(0x10000, "ppu", ROMREGION_16BIT | ROMREGION_BE)
		ROM_LOAD("9845-PPU-Standard-Graphics.bin", 0, 0x10000, CRC(f866510f) SHA1(3e22cd2072e3a5f3603a1eb8477b6b4a198d184d))

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

#define rom_hp9845t rom_hp9845b
#define rom_hp9845c rom_hp9845b

COMP( 1978, hp9845a,   0,       0,      hp9845a,       hp9845, driver_device, 0,      "Hewlett-Packard",  "9845A",  MACHINE_IS_SKELETON | MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1978, hp9845s,   hp9845a, 0,      hp9845a,       hp9845, driver_device, 0,      "Hewlett-Packard",  "9845S",  MACHINE_IS_SKELETON | MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1979, hp9835a,   0,       0,      hp9835a,       hp9845, driver_device, 0,      "Hewlett-Packard",  "9835A",  MACHINE_IS_SKELETON | MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1979, hp9835b,   hp9835a, 0,      hp9835a,       hp9845, driver_device, 0,      "Hewlett-Packard",  "9835B",  MACHINE_IS_SKELETON | MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1980, hp9845b,   0,       0,      hp9845b,       hp9845b,driver_device, 0,      "Hewlett-Packard",  "9845B",  MACHINE_NO_SOUND )
COMP( 1980, hp9845t,   hp9845b, 0,      hp9845b,       hp9845b,driver_device, 0,      "Hewlett-Packard",  "9845T",  MACHINE_IS_SKELETON | MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1981, hp9845c,   hp9845b, 0,      hp9845b,       hp9845b,driver_device, 0,      "Hewlett-Packard",  "9845C",  MACHINE_IS_SKELETON | MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
