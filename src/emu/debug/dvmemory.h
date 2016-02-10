// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    dvmemory.h

    Memory debugger view.

***************************************************************************/

#ifndef __DVMEMORY_H__
#define __DVMEMORY_H__

#include "softfloat/mamesf.h"
#include "softfloat/softfloat.h"

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
	address_space *m_space;                     // address space we reference (if any)
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
	int get_data_format() { flush_updates(); return m_data_format; }
	UINT32 chunks_per_row() { flush_updates(); return m_chunks_per_row; }
	bool reverse() const { return m_reverse_view; }
	bool ascii() const { return m_ascii_view; }
	bool physical() const { return m_no_translation; }
	offs_t addressAtCursorPosition(const debug_view_xy& pos) { return get_cursor_pos(pos).m_address; }

	// setters
	void set_expression(const char *expression);
	void set_chunks_per_row(UINT32 rowchunks);
	void set_data_format(int format); // 1-8 current values 9 32bit floating point
	void set_reverse(bool reverse);
	void set_ascii(bool reverse);
	void set_physical(bool physical);

protected:
	// view overrides
	virtual void view_notify(debug_view_notification type) override;
	virtual void view_update() override;
	virtual void view_char(int chval) override;
	virtual void view_click(const int button, const debug_view_xy& pos) override;

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
	cursor_pos get_cursor_pos(const debug_view_xy& cursor);
	void set_cursor_pos(cursor_pos pos);
	cursor_pos begin_update_and_get_cursor_pos() { begin_update(); return get_cursor_pos(m_cursor); }
	void end_update_and_set_cursor_pos(cursor_pos pos) { set_cursor_pos(pos); end_update(); }

	// memory access
	bool read(UINT8 size, offs_t offs, UINT64 &data);
	void write(UINT8 size, offs_t offs, UINT64 data);
	bool read(UINT8 size, offs_t offs, floatx80 &data);

	// internal state
	debug_view_expression m_expression;         // expression describing the start address
	UINT32              m_chunks_per_row;       // number of chunks displayed per line
	UINT8               m_bytes_per_chunk;      // bytes per chunk
	int                 m_data_format;          // 1-8 current values 9 32bit floating point
	bool                m_reverse_view;         // reverse-endian view?
	bool                m_ascii_view;           // display ASCII characters?
	bool                m_no_translation;       // don't run addresses through the cpu translation hook
	bool                m_edit_enabled;         // can modify contents ?
	offs_t              m_maxaddr;              // (derived) maximum address to display
	UINT32              m_bytes_per_row;        // (derived) number of bytes displayed per line
	UINT32              m_byte_offset;          // (derived) offset of starting visible byte
	std::string         m_addrformat;           // (derived) format string to use to print addresses

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
	static const memory_view_pos s_memory_pos_table[12]; // table for rendering at different data formats

	// constants
	static const int MEM_MAX_LINE_WIDTH = 1024;
};


#endif
