// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    natkeyboard.h

    Natural keyboard input support.

***************************************************************************/
#ifndef MAME_EMU_NATKEYBOARD_H
#define MAME_EMU_NATKEYBOARD_H

#pragma once

#include <iosfwd>
#include <unordered_map>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// keyboard helper function delegates
typedef delegate<int (const char32_t *, size_t)> ioport_queue_chars_delegate;
typedef delegate<bool (char32_t)> ioport_accept_char_delegate;
typedef delegate<bool ()> ioport_charqueue_empty_delegate;


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
	bool can_post() const { return (!m_queue_chars.isnull() || !m_keycode_map.empty()); }
	bool is_posting() const { return (!empty() || (!m_charqueue_empty.isnull() && !m_charqueue_empty())); }
	bool in_use() const { return m_in_use; }

	// configuration
	void configure(ioport_queue_chars_delegate queue_chars, ioport_accept_char_delegate accept_char, ioport_charqueue_empty_delegate charqueue_empty);
	void set_in_use(bool usage);

	// posting
	void post(char32_t ch);
	void post(const char32_t *text, size_t length = 0, const attotime &rate = attotime::zero);
	void post_utf8(const char *text, size_t length = 0, const attotime &rate = attotime::zero);
	void post_coded(const char *text, size_t length = 0, const attotime &rate = attotime::zero);

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
		ioport_field *  field[SHIFT_COUNT + 1];
		unsigned        shift;
	};
	typedef std::unordered_map<char32_t, keycode_map_entry> keycode_map;

	// internal helpers
	void build_codes(ioport_manager &manager);
	bool can_post_directly(char32_t ch);
	bool can_post_alternate(char32_t ch);
	attotime choose_delay(char32_t ch);
	void internal_post(char32_t ch);
	void timer(void *ptr, int param);
	std::string unicode_to_string(char32_t ch) const;
	const keycode_map_entry *find_code(char32_t ch) const;

	// internal state
	running_machine &               m_machine;          // reference to our machine
	bool                            m_in_use;           // is natural keyboard in use?
	u32                             m_bufbegin;         // index of starting character
	u32                             m_bufend;           // index of ending character
	std::vector<char32_t>           m_buffer;           // actual buffer
	unsigned                        m_fieldnum;         // current step in multi-key sequence
	bool                            m_status_keydown;   // current keydown status
	bool                            m_last_cr;          // was the last char a CR?
	emu_timer *                     m_timer;            // timer for posting characters
	attotime                        m_current_rate;     // current rate for posting
	ioport_queue_chars_delegate     m_queue_chars;      // queue characters callback
	ioport_accept_char_delegate     m_accept_char;      // accept character callback
	ioport_charqueue_empty_delegate m_charqueue_empty;  // character queue empty callback
	keycode_map                     m_keycode_map;      // keycode map
};


inline std::ostream &operator<<(std::ostream &str, natural_keyboard const &kbd) { kbd.dump(str); return str; }

#endif // MAME_EMU_NATKEYBOARD_H
