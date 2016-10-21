// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    natkeyboard.h

    Natural keyboard input support.

***************************************************************************/

#pragma once

#ifndef EMU_NATKEYBOARD_H
#define EMU_NATKEYBOARD_H


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// keyboard helper function delegates
typedef delegate<int (const unicode_char *, size_t)> ioport_queue_chars_delegate;
typedef delegate<bool (unicode_char)> ioport_accept_char_delegate;
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
	void post(unicode_char ch);
	void post(const unicode_char *text, size_t length = 0, const attotime &rate = attotime::zero);
	void post_utf8(const char *text, size_t length = 0, const attotime &rate = attotime::zero);
	void post_coded(const char *text, size_t length = 0, const attotime &rate = attotime::zero);

	// debugging
	std::string dump();

private:
	// internal keyboard code information
	struct keycode_map_entry
	{
		unicode_char    ch;
		ioport_field *  field[UCHAR_SHIFT_END + 1 - UCHAR_SHIFT_BEGIN];
	};

	// internal helpers
	void build_codes(ioport_manager &manager);
	bool can_post_directly(unicode_char ch);
	bool can_post_alternate(unicode_char ch);
	attotime choose_delay(unicode_char ch);
	void internal_post(unicode_char ch);
	void timer(void *ptr, int param);
	std::string unicode_to_string(unicode_char ch);
	const keycode_map_entry *find_code(unicode_char ch) const;

	// internal state
	running_machine &       m_machine;              // reference to our machine
	bool                    m_in_use;               // is natural keyboard in use?
	UINT32                  m_bufbegin;             // index of starting character
	UINT32                  m_bufend;               // index of ending character
	std::vector<unicode_char> m_buffer;           // actual buffer
	bool                    m_status_keydown;       // current keydown status
	bool                    m_last_cr;              // was the last char a CR?
	emu_timer *             m_timer;                // timer for posting characters
	attotime                m_current_rate;         // current rate for posting
	ioport_queue_chars_delegate m_queue_chars;      // queue characters callback
	ioport_accept_char_delegate m_accept_char;      // accept character callback
	ioport_charqueue_empty_delegate m_charqueue_empty; // character queue empty callback
	std::vector<keycode_map_entry> m_keycode_map; // keycode map
};

#endif
