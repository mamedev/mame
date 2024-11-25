// license:GPL-2.0+
// copyright-holders:Raphael Nabet
#ifndef MAME_TI_911_VDT_H
#define MAME_TI_911_VDT_H

#pragma once

#include "sound/beep.h"
#include "emupal.h"
#include "screen.h"

#define vdt911_chr_region ":gfx1"

class vdt911_device : public device_t, public device_gfx_interface
{
public:
	enum
	{
		/* 10 bytes per character definition */
		single_char_len = 10,

		US_chr_offset        = 0,
		UK_chr_offset        = US_chr_offset+128*single_char_len,
		german_chr_offset    = UK_chr_offset+128*single_char_len,
		swedish_chr_offset   = german_chr_offset+128*single_char_len,
		norwegian_chr_offset = swedish_chr_offset+128*single_char_len,
		frenchWP_chr_offset  = norwegian_chr_offset+128*single_char_len,
		japanese_chr_offset  = frenchWP_chr_offset+128*single_char_len,

		chr_region_len   = japanese_chr_offset+256*single_char_len
	};

	enum class screen_size { char_960 = 0, char_1920 };

	enum class model
	{
		US = 0,
		UK,
		French,
		German,
		Swedish,      // Swedish/Finnish
		Norwegian,    // Norwegian/Danish
		Japanese,     // Katakana Japanese
		/*Arabic,*/   // Arabic
		FrenchWP      // French word processing
	};

	vdt911_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t cru_r(offs_t offset);
	void cru_w(offs_t offset, uint8_t data);

	auto keyint_cb() { return m_keyint_line.bind(); }
	auto lineint_cb() { return m_lineint_line.bind(); }

protected:
	// device-level overrides
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	ioport_constructor device_input_ports() const override;

	TIMER_CALLBACK_MEMBER(blink_tick);
	TIMER_CALLBACK_MEMBER(beep_off);
	TIMER_CALLBACK_MEMBER(line_tick);

private:
	void refresh(bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y);
	void check_keyboard();

	void vdt911_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	screen_size    m_screen_size;  // char_960 for 960-char, 12-line model; char_1920 for 1920-char, 24-line model
	model          m_model;        // country code

	uint8_t m_data_reg;                       // dt911 write buffer
	uint8_t m_display_RAM[2048];              // vdt911 char buffer (1kbyte for 960-char model, 2kbytes for 1920-char model)

	unsigned int m_cursor_address;          // current cursor address (controlled by the computer, affects both display and I/O protocol)
	unsigned int m_cursor_address_mask; // 1023 for 960-char model, 2047 for 1920-char model

	emu_timer *m_beep_timer;                // beep clock (beeps ends when timer times out)
	emu_timer *m_blink_timer;               // cursor blink clock
	emu_timer *m_line_timer;                // screen line timer

	uint8_t m_keyboard_data;                  // last code pressed on keyboard
	bool m_keyboard_data_ready;             // true if there is a new code in keyboard_data
	bool m_keyboard_interrupt_enable;       // true when keyboard interrupts are enabled

	bool m_display_enable;                  // screen is black when false
	bool m_dual_intensity_enable;           // if true, MSBit of ASCII codes controls character highlight
	bool m_display_cursor;                  // if true, the current cursor location is displayed on screen
	bool m_blinking_cursor_enable;          // if true, the cursor will blink when displayed
	bool m_blink_state;                     // current cursor blink state

	bool m_word_select;                     // CRU interface mode
	bool m_previous_word_select;            // value of word_select is saved here

	uint8_t m_last_key_pressed;
	int m_last_modifier_state;
	char m_foreign_mode;

	required_device<beep_device> m_beeper;
	required_device<screen_device> m_screen;
	required_ioport_array<6> m_keys;

	devcb_write_line                   m_keyint_line;
	devcb_write_line                   m_lineint_line;
};

DECLARE_DEVICE_TYPE(VDT911, vdt911_device)

#endif // MAME_TI_911_VDT_H
