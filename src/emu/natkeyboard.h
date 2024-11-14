// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Vas Crabb
/***************************************************************************

    natkeyboard.h

    Natural keyboard input support.

***************************************************************************/
#ifndef MAME_EMU_NATKEYBOARD_H
#define MAME_EMU_NATKEYBOARD_H

#pragma once

#include <array>
#include <functional>
#include <iosfwd>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// keyboard helper function delegates
using ioport_queue_chars_delegate = delegate<int (const char32_t *, size_t)>;
using ioport_accept_char_delegate = delegate<bool (char32_t)>;
using ioport_charqueue_empty_delegate = delegate<bool ()>;


// ======================> natural_keyboard

// buffer to handle copy/paste/insert of keys
class natural_keyboard
{
	DISABLE_COPYING(natural_keyboard);

public:
	// construction/destruction
	natural_keyboard(running_machine &machine);

	// getters and queries
	running_machine &machine() const { return m_machine; }
	bool empty() const { return (m_bufbegin == m_bufend); }
	bool full() const { return ((m_bufend + 1) % m_buffer.size()) == m_bufbegin; }
	bool can_post() const { return m_have_charkeys || !m_queue_chars.isnull(); }
	bool is_posting() const { return (!empty() || (!m_charqueue_empty.isnull() && !m_charqueue_empty())); }
	bool in_use() const { return m_in_use; }

	// configuration
	void configure(ioport_queue_chars_delegate queue_chars, ioport_accept_char_delegate accept_char, ioport_charqueue_empty_delegate charqueue_empty);
	void set_in_use(bool usage);
	size_t keyboard_count() const { return m_keyboards.size(); }
	device_t &keyboard_device(size_t n) const { return m_keyboards[n].device; }
	bool keyboard_is_keypad(size_t n) const { return !m_keyboards[n].keyboard; }
	bool keyboard_enabled(size_t n) const { return m_keyboards[n].enabled; }
	void enable_keyboard(size_t n) { set_keyboard_enabled(n, true); }
	void disable_keyboard(size_t n) { set_keyboard_enabled(n, false); }

	// posting
	void post_char(char32_t ch, bool normalize_crlf = false);
	void post(std::u32string_view text, const attotime &rate = attotime::zero);
	void post_utf8(std::string_view text, const attotime &rate = attotime::zero);
	void post_coded(std::string_view text, const attotime &rate = attotime::zero);
	void paste();

	// debugging
	void dump(std::ostream &str) const;
	std::string dump() const;

private:
	enum
	{
		SHIFT_COUNT = UCHAR_SHIFT_END - UCHAR_SHIFT_BEGIN + 1,
		SHIFT_STATES = 1 << SHIFT_COUNT
	};

	// internal keyboard code information
	struct keycode_map_entry
	{
		std::array<ioport_field *, SHIFT_COUNT + 1> field;
		unsigned                                    shift;
		ioport_condition                            condition;
	};
	typedef std::vector<keycode_map_entry> keycode_map_entries;
	typedef std::unordered_map<char32_t, keycode_map_entries> keycode_map;

	// per-device character-to-key mapping
	struct kbd_dev_info
	{
		kbd_dev_info(device_t &dev) : device(dev) { }

		std::reference_wrapper<device_t>                    device;
		std::vector<std::reference_wrapper<ioport_field> >  keyfields;
		keycode_map                                         codemap;
		bool                                                keyboard = false;
		bool                                                keypad = false;
		bool                                                enabled = false;
	};

	// internal helpers
	void build_codes();
	void set_keyboard_enabled(size_t n, bool enable);
	bool can_post_directly(char32_t ch);
	bool can_post_alternate(char32_t ch);
	attotime choose_delay(char32_t ch);
	void internal_post(char32_t ch);
	void timer(s32 param);
	std::string unicode_to_string(char32_t ch) const;
	const keycode_map_entry *find_code(char32_t ch) const;

	// internal state
	std::vector<kbd_dev_info>       m_keyboards;        // info on keyboard devices in system
	running_machine &               m_machine;          // reference to our machine
	bool                            m_have_charkeys;    // are there keys with character?
	bool                            m_in_use;           // is natural keyboard in use?
	u32                             m_bufbegin;         // index of starting character
	u32                             m_bufend;           // index of ending character
	std::vector<char32_t>           m_buffer;           // actual buffer
	keycode_map_entry const *       m_current_code;     // current code being typed
	unsigned                        m_fieldnum;         // current step in multi-key sequence
	bool                            m_status_keydown;   // current keydown status
	bool                            m_last_cr;          // was the last char a CR?
	emu_timer *                     m_timer;            // timer for posting characters
	attotime                        m_current_rate;     // current rate for posting
	ioport_queue_chars_delegate     m_queue_chars;      // queue characters callback
	ioport_accept_char_delegate     m_accept_char;      // accept character callback
	ioport_charqueue_empty_delegate m_charqueue_empty;  // character queue empty callback
};


inline std::ostream &operator<<(std::ostream &str, natural_keyboard const &kbd) { kbd.dump(str); return str; }

#endif // MAME_EMU_NATKEYBOARD_H
