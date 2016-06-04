// license:GPL-2.0+
// copyright-holders:Raphael Nabet

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
	vdt911_model_Japanese,       // Katakana Japanese
	/*vdt911_model_Arabic,*/    // Arabic
	vdt911_model_FrenchWP      // French word processing
};

class vdt911_device : public device_t, public device_gfx_interface
{
public:
	vdt911_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(cru_r);
	DECLARE_WRITE8_MEMBER(cru_w);

	DECLARE_PALETTE_INIT(vdt911);

	template<class _Object> static devcb_base &static_set_keyint_callback(device_t &device, _Object object)
	{
		return downcast<vdt911_device &>(device).m_keyint_line.set_callback(object);
	}

	template<class _Object> static devcb_base &static_set_lineint_callback(device_t &device, _Object object)
	{
		return downcast<vdt911_device &>(device).m_lineint_line.set_callback(object);
	}

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	void device_start() override;
	void device_reset() override;

	machine_config_constructor device_mconfig_additions() const override;
	ioport_constructor device_input_ports() const override;

	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	void refresh(bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y);
	int get_refresh_rate();
	void check_keyboard();

	vdt911_screen_size_t    m_screen_size;  // char_960 for 960-char, 12-line model; char_1920 for 1920-char, 24-line model
	vdt911_model_t          m_model;        // country code

	UINT8 m_data_reg;                       // dt911 write buffer
	UINT8 m_display_RAM[2048];              // vdt911 char buffer (1kbyte for 960-char model, 2kbytes for 1920-char model)

	unsigned int m_cursor_address;          // current cursor address (controlled by the computer, affects both display and I/O protocol)
	unsigned int m_cursor_address_mask; // 1023 for 960-char model, 2047 for 1920-char model

	emu_timer *m_beep_timer;                // beep clock (beeps ends when timer times out)
	emu_timer *m_blink_timer;               // cursor blink clock
	emu_timer *m_line_timer;                // screen line timer

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
	devcb_write_line                   m_keyint_line;
	devcb_write_line                   m_lineint_line;
};

extern const device_type VDT911;

#define MCFG_VDT911_KEYINT_HANDLER( _intcallb ) \
	devcb = &vdt911_device::static_set_keyint_callback( *device, DEVCB_##_intcallb );

#define MCFG_VDT911_LINEINT_HANDLER( _intcallb ) \
	devcb = &vdt911_device::static_set_lineint_callback( *device, DEVCB_##_intcallb );
