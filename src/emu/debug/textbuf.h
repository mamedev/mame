// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    textbuf.h

    Debugger text buffering engine.

***************************************************************************/

#ifndef MAME_EMU_DEBUG_TEXTBUF_H
#define MAME_EMU_DEBUG_TEXTBUF_H

#include "emucore.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct text_buffer;

struct text_buffer_line
{
	const char *text;
	size_t length;
};

/* helper class that makes it possible to iterate over the lines of a text_buffer */
class text_buffer_lines
{
private:
	text_buffer& m_buffer;

public:
	text_buffer_lines(text_buffer& buffer) : m_buffer(buffer) { }

	class text_buffer_line_iterator
	{
		text_buffer& m_buffer;
		s32 m_lineptr;
	public:
		text_buffer_line_iterator(text_buffer& buffer, s32 lineptr) :
			m_buffer(buffer),
			m_lineptr(lineptr)
		{
		}

		/* technically this isn't a valid forward iterator, because
		 * operator * doesn't return a reference
		 */
		text_buffer_line operator *() const;
		text_buffer_line_iterator& operator ++();

		bool operator != (const text_buffer_line_iterator& rhs)
		{
			return m_lineptr != rhs.m_lineptr;
		}
		/* according to C++ spec, only != is needed; == is present for completeness. */
		bool operator == (const text_buffer_line_iterator& rhs) { return !(operator !=(rhs)); }
	};

	typedef text_buffer_line_iterator iterator;
	typedef text_buffer_line_iterator const iterator_const;

	iterator begin() const;
	iterator end() const;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* allocate a new text buffer */
text_buffer *text_buffer_alloc(u32 bytes, u32 lines);

/* free a text buffer */
void text_buffer_free(text_buffer *text);

/* clear a text buffer */
void text_buffer_clear(text_buffer *text);

/* "print" data to a text buffer */
void text_buffer_print(text_buffer *text, const char *data);

/* "print" data to a text buffer with word wrapping to a given column */
void text_buffer_print_wrap(text_buffer *text, const char *data, int wrapcol);

/* get the maximum width of lines seen so far */
u32 text_buffer_max_width(text_buffer *text);

/* get the current number of lines in the buffer */
u32 text_buffer_num_lines(text_buffer *text);

/* get an absolute sequence number for a given line */
u32 text_buffer_line_index_to_seqnum(text_buffer *text, u32 index);

/* get a sequenced line from the text buffer */
const char *text_buffer_get_seqnum_line(text_buffer *text, u32 seqnum);

/* get an iterable container of the lines in the buffer */
text_buffer_lines text_buffer_get_lines(text_buffer* text);

#endif  /* MAME_EMU_DEBUG_TEXTBUF_H */
