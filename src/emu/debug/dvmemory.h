/*********************************************************************

    dvmemory.h

    Memory debugger view.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#ifndef __DVMEMORY_H__
#define __DVMEMORY_H__

#include "dvstate.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// a memory view_source
class debug_view_memory_source : public debug_view_source
{
	friend class debug_view_memory;

	debug_view_memory_source(const char *name, address_space &space);
	debug_view_memory_source(const char *name, memory_region &region);
	debug_view_memory_source(const char *name, void *base, int element_size, int num_elements);

public:
	address_space *space() const { return m_space; }

private:
	address_space *m_space;             // address space we reference (if any)
	device_memory_interface *m_memintf;         // pointer to the memory interface of the device
	void *              m_base;                 // pointer to memory base
	offs_t              m_length;               // length of memory
	offs_t              m_offsetxor;            // XOR to apply to offsets
	endianness_t        m_endianness;           // endianness of memory
	UINT8               m_prefsize;             // preferred bytes per chunk
};


// debug view for memory
class debug_view_memory : public debug_view
{
	friend resource_pool_object<debug_view_memory>::~resource_pool_object();
	friend class debug_view_manager;

	// construction/destruction
	debug_view_memory(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate);

public:
	// getters
	const char *expression() { return m_expression.string(); }
	UINT8 bytes_per_chunk() { flush_updates(); return m_bytes_per_chunk; }
	UINT8 chunks_per_row() { flush_updates(); return m_chunks_per_row; }
	bool reverse() const { return m_reverse_view; }
	bool ascii() const { return m_ascii_view; }
	bool physical() const { return m_no_translation; }

	// setters
	void set_expression(const char *expression);
	void set_bytes_per_chunk(UINT8 chunkbytes);
	void set_chunks_per_row(UINT32 rowchunks);
	void set_reverse(bool reverse);
	void set_ascii(bool reverse);
	void set_physical(bool physical);

protected:
	// view overrides
	virtual void view_notify(debug_view_notification type);
	virtual void view_update();
	virtual void view_char(int chval);

private:
	struct cursor_pos
	{
		cursor_pos(offs_t address = 0, UINT8 shift = 0) : m_address(address), m_shift(shift) { }
		offs_t m_address;
		UINT8 m_shift;
	};

	// internal helpers
	void enumerate_sources();
	void recompute();
	bool needs_recompute();

	// cursor position management
	cursor_pos get_cursor_pos();
	void set_cursor_pos(cursor_pos pos);
	cursor_pos begin_update_and_get_cursor_pos() { begin_update(); return get_cursor_pos(); }
	void end_update_and_set_cursor_pos(cursor_pos pos) { set_cursor_pos(pos); end_update(); }

	// memory access
	bool read(UINT8 size, offs_t offs, UINT64 &data);
	void write(UINT8 size, offs_t offs, UINT64 data);

	// internal state
	debug_view_expression m_expression;         // expression describing the start address
	UINT32              m_chunks_per_row;       // number of chunks displayed per line
	UINT8               m_bytes_per_chunk;      // bytes per chunk
	bool                m_reverse_view;         // reverse-endian view?
	bool                m_ascii_view;           // display ASCII characters?
	bool                m_no_translation;       // don't run addresses through the cpu translation hook
	offs_t              m_maxaddr;              // (derived) maximum address to display
	UINT32              m_bytes_per_row;        // (derived) number of bytes displayed per line
	UINT32              m_byte_offset;          // (derived) offset of starting visible byte
	astring             m_addrformat;           // (derived) format string to use to print addresses

	struct section
	{
		bool contains(int x) const { return x >= m_pos && x < m_pos + m_width; }
		INT32           m_pos;                  /* starting position */
		INT32           m_width;                /* width of this section */
	};
	section             m_section[3];           // (derived) 3 sections to manage

	struct memory_view_pos
	{
		UINT8           m_spacing;              /* spacing between each entry */
		UINT8           m_shift[24];            /* shift for each character */
	};
	static const memory_view_pos s_memory_pos_table[9]; // table for rendering at different chunk sizes

	// constants
	static const int MEM_MAX_LINE_WIDTH = 1024;
};


#endif
