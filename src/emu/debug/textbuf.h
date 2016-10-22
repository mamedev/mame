// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    textbuf.h

    Debugger text buffering engine.

***************************************************************************/

#ifndef __TEXTBUF_H__
#define __TEXTBUF_H__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct text_buffer;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* allocate a new text buffer */
text_buffer *text_buffer_alloc(uint32_t bytes, uint32_t lines);

/* free a text buffer */
void text_buffer_free(text_buffer *text);

/* clear a text buffer */
void text_buffer_clear(text_buffer *text);

/* "print" data to a text buffer */
void text_buffer_print(text_buffer *text, const char *data);

/* "print" data to a text buffer with word wrapping to a given column */
void text_buffer_print_wrap(text_buffer *text, const char *data, int wrapcol);

/* get the maximum width of lines seen so far */
uint32_t text_buffer_max_width(text_buffer *text);

/* get the current number of lines in the buffer */
uint32_t text_buffer_num_lines(text_buffer *text);

/* get an absolute sequence number for a given line */
uint32_t text_buffer_line_index_to_seqnum(text_buffer *text, uint32_t index);

/* get a sequenced line from the text buffer */
const char *text_buffer_get_seqnum_line(text_buffer *text, uint32_t seqnum);


#endif  /* __TEXTBUF_H__ */
