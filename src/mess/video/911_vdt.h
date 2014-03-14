
#include "sound/beep.h"

#define vdt911_chr_region ":gfx1"
enum
{
	/* 10 bytes per character definition */
	vdt911_single_char_len = 10,

	vdt911_US_chr_offset        = 0,
	vdt911_UK_chr_offset        = vdt911_US_chr_offset+128*vdt911_single_char_len,
	vdt911_german_chr_offset    = vdt911_UK_chr_offset+128*vdt911_single_char_len,
	vdt911_swedish_chr_offset   = vdt911_german_chr_offset+128*vdt911_single_char_len,
	vdt911_norwegian_chr_offset = vdt911_swedish_chr_offset+128*vdt911_single_char_len,
	vdt911_frenchWP_chr_offset  = vdt911_norwegian_chr_offset+128*vdt911_single_char_len,
	vdt911_japanese_chr_offset  = vdt911_frenchWP_chr_offset+128*vdt911_single_char_len,

	vdt911_chr_region_len   = vdt911_japanese_chr_offset+256*vdt911_single_char_len
};

enum vdt911_screen_size_t { char_960 = 0, char_1920 };
enum vdt911_model_t
{
	vdt911_model_US = 0,
	vdt911_model_UK,
	vdt911_model_French,
	vdt911_model_German,
	vdt911_model_Swedish,       // Swedish/Finnish
	vdt911_model_Norwegian,     // Norwegian/Danish
	vdt911_model_FrenchWP,      // French word processing
	vdt911_model_Japanese       // Katakana Japanese
	/*vdt911_model_Arabic,*/    // Arabic
};

class vdt911_device : public device_t
{
public:
	vdt911_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(cru_r);
	DECLARE_WRITE8_MEMBER(cru_w);

	DECLARE_PALETTE_INIT(vdt911);

	template<class _Object> static devcb2_base &static_set_int_callback(device_t &device, _Object object)
	{
		return downcast<vdt911_device &>(device).m_int_line.set_callback(object);
	}

	void refresh(bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y);
	void keyboard();

protected:
	// device-level overrides
	void device_config_complete();
	void device_start();
	void device_reset();

	machine_config_constructor device_mconfig_additions() const;
	ioport_constructor device_input_ports() const;

	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:

	// internal state

	vdt911_screen_size_t    m_screen_size;  // char_960 for 960-char, 12-line model; char_1920 for 1920-char, 24-line model
	vdt911_model_t          m_model;        // country code

	UINT8 m_data_reg;                       // dt911 write buffer
	UINT8 m_display_RAM[2048];              // vdt911 char buffer (1kbyte for 960-char model, 2kbytes for 1920-char model)

	unsigned int m_cursor_address;          // current cursor address (controlled by the computer, affects both display and I/O protocol)
	unsigned int m_cursor_address_mask; // 1023 for 960-char model, 2047 for 1920-char model

	emu_timer *m_beep_timer;                // beep clock (beeps ends when timer times out)
	emu_timer *m_blink_timer;               // cursor blink clock

	UINT8 m_keyboard_data;                  // last code pressed on keyboard
	bool m_keyboard_data_ready;             // true if there is a new code in keyboard_data
	bool m_keyboard_interrupt_enable;       // true when keybord interrupts are enabled

	bool m_display_enable;                  // screen is black when false
	bool m_dual_intensity_enable;           // if true, MSBit of ASCII codes controls character highlight
	bool m_display_cursor;                  // if true, the current cursor location is displayed on screen
	bool m_blinking_cursor_enable;          // if true, the cursor will blink when displayed
	bool m_blink_state;                     // current cursor blink state

	bool m_word_select;                     // CRU interface mode
	bool m_previous_word_select;            // value of word_select is saved here

	UINT8 m_last_key_pressed;
	int m_last_modifier_state;
	char m_foreign_mode;

	required_device<beep_device>        m_beeper;
	required_device<gfxdecode_device>   m_gfxdecode;
	required_device<palette_device>     m_palette;
	devcb2_write_line                   m_int_line;
};

extern const device_type VDT911;

#define MCFG_VDT911_INT_HANDLER( _intcallb ) \
	devcb = &vdt911_device::static_set_int_callback( *device, DEVCB2_##_intcallb );

#define VDT911_KEY_PORTS                                                                        \
	PORT_START("KEY0")  /* keys 1-16 */                                                                 \
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)        \
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)        \
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)        \
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)        \
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)        \
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)        \
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7)        \
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8)        \
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CMD") PORT_CODE(KEYCODE_F9)       \
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(red)") PORT_CODE(KEYCODE_F10)        \
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ERASE FIELD") PORT_CODE(KEYCODE_END)  \
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ERASE INPUT") PORT_CODE(KEYCODE_PGDN) \
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(grey)") PORT_CODE(KEYCODE_F11)       \
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UPPER CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK)  PORT_TOGGLE\
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1)        \
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 @") PORT_CODE(KEYCODE_2)        \
																								\
	PORT_START("KEY1")  /* keys 17-32 */                                                                \
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3)        \
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4)        \
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5)        \
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 ^") PORT_CODE(KEYCODE_6)        \
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 &") PORT_CODE(KEYCODE_7)        \
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 *") PORT_CODE(KEYCODE_8)        \
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 (") PORT_CODE(KEYCODE_9)        \
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0)        \
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+ [") PORT_CODE(KEYCODE_MINUS)    \
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- ]") PORT_CODE(KEYCODE_EQUALS)   \
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("_ =") PORT_CODE(KEYCODE_BACKSPACE)    \
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)      \
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 (numpad)") PORT_CODE(KEYCODE_7_PAD) \
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 (numpad)") PORT_CODE(KEYCODE_8_PAD) \
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 (numpad)") PORT_CODE(KEYCODE_9_PAD) \
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PRINT") PORT_CODE(KEYCODE_PRTSCR) \
																								\
	PORT_START("KEY2")  /* keys 33-48 */                                                                \
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(up)") PORT_CODE(KEYCODE_UP)      \
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("REPEAT") PORT_CODE(KEYCODE_LALT)  \
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER_PAD)  \
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)      \
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)      \
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)      \
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)      \
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)      \
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)      \
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)      \
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)      \
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)      \
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)      \
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CHAR (left/right)") PORT_CODE(KEYCODE_OPENBRACE)  \
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("FIELD (left/right)") PORT_CODE(KEYCODE_CLOSEBRACE)    \
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) \
																								\
	PORT_START("KEY3")  /* keys 49-64 */                                                                \
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 (numpad)") PORT_CODE(KEYCODE_4_PAD) \
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 (numpad)") PORT_CODE(KEYCODE_5_PAD) \
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 (numpad)") PORT_CODE(KEYCODE_6_PAD) \
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(left)") PORT_CODE(KEYCODE_LEFT)  \
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME)    \
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(right)") PORT_CODE(KEYCODE_RIGHT)    \
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CONTROL") PORT_CODE(KEYCODE_LCONTROL) \
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)      \
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)      \
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)      \
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)      \
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)      \
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)      \
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)      \
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)      \
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)      \
																								\
	PORT_START("KEY4")  /* keys 65-80 */                                                                \
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON)    \
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("' \"") PORT_CODE(KEYCODE_QUOTE)   \
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(not on US keyboard)") PORT_CODE(KEYCODE_BACKSLASH)   \
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SKIP TAB") PORT_CODE(KEYCODE_TAB) \
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 (numpad)") PORT_CODE(KEYCODE_1_PAD) \
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 (numpad)") PORT_CODE(KEYCODE_2_PAD) \
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 (numpad)") PORT_CODE(KEYCODE_3_PAD) \
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INS CHAR") PORT_CODE(KEYCODE_INSERT)  \
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(down)") PORT_CODE(KEYCODE_DOWN)  \
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL CHAR") PORT_CODE(KEYCODE_DEL) \
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) \
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)      \
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)      \
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)      \
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)      \
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)      \
																								\
	PORT_START("KEY5")  /* keys 81-91 */                                                                \
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)      \
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)      \
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)    \
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) \
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH)    \
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_RSHIFT) \
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 (numpad)") PORT_CODE(KEYCODE_0_PAD) \
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". (numpad)") PORT_CODE(KEYCODE_DEL_PAD)   \
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(not on US keyboard)") PORT_CODE(KEYCODE_MINUS_PAD)   \
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE)  \
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(not on US keyboard)") PORT_CODE(KEYCODE_PLUS_PAD)
