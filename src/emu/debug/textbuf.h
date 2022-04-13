// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    textbuf.h

    Debugger text buffering engine.

***************************************************************************/

#ifndef MAME_EMU_DEBUG_TEXTBUF_H
#define MAME_EMU_DEBUG_TEXTBUF_H

#include <memory>
#include <string_view>

#include "emucore.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct text_buffer;

// helper class for iterating over the lines of a text_buffer
class text_buffer_lines
{
private:
	const text_buffer &m_buffer;

public:
	text_buffer_lines(const text_buffer& buffer) : m_buffer(buffer) { }

	class text_buffer_line_iterator
	{
		const text_buffer &m_buffer;
		s32 m_lineptr;
	public:
		text_buffer_line_iterator(const text_buffer &buffer, s32 lineptr) :
			m_buffer(buffer),
			m_lineptr(lineptr)
		{
		}

		// technically this isn't a valid forward iterator, because operator * doesn't return a reference
		std::string_view operator*() const;
		text_buffer_line_iterator &operator++();

		bool operator!=(const text_buffer_line_iterator& rhs)
		{
			return m_lineptr != rhs.m_lineptr;
		}
		// according to C++ spec, only != is needed; == is present for completeness.
		bool operator==(const text_buffer_line_iterator& rhs) { return !operator!=(rhs); }
	};

	typedef text_buffer_line_iterator iterator;
	typedef text_buffer_line_iterator const iterator_const;

	iterator begin() const;
	iterator end() const;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

// free a text buffer
struct text_buffer_deleter { void operator()(text_buffer *text) const; };
using text_buffer_ptr = std::unique_ptr<text_buffer, text_buffer_deleter>;

// allocate a new text buffer
text_buffer_ptr text_buffer_alloc(u32 bytes, u32 lines);

// clear a text buffer
void text_buffer_clear(text_buffer &text);

// "print" data to a text buffer
void text_buffer_print(text_buffer &text, std::string_view data);

// "print" data to a text buffer with word wrapping to a given column
void text_buffer_print_wrap(text_buffer &text, std::string_view data, int wrapcol);

// get the maximum width of lines seen so far
u32 text_buffer_max_width(const text_buffer &text);

// get the current number of lines in the buffer
u32 text_buffer_num_lines(const text_buffer &text);

// get an absolute sequence number for a given line
u32 text_buffer_line_index_to_seqnum(const text_buffer &text, u32 index);

// get a sequenced line from the text buffer
const char *text_buffer_get_seqnum_line(const text_buffer &text, u32 seqnum);

#endif // MAME_EMU_DEBUG_TEXTBUF_H
