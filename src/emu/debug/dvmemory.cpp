// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    dvmemory.cpp

    Memory debugger view.

***************************************************************************/

#include "emu.h"
#include "dvmemory.h"

#include "debugcpu.h"

#include <algorithm>
#include <cctype>
#include <tuple>


//**************************************************************************
//  HELPER FUNCTIONS
//**************************************************************************

namespace {

constexpr u8 sanitise_character(u8 ch)
{
	// assume ISO-8859-1 (low 256 Unicode codepoints) - tab, soft hyphen, C0 and C1 cause problems
	return ('\t' == ch) ? ' ' : (0xadU == ch) ? '-' : ((' ' > ch) || (('~' < ch) && (0xa0U > ch))) ? '.' : ch;
}

} // anonymous namespace

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const debug_view_memory::memory_view_pos debug_view_memory::s_memory_pos_table[16] =
{
	/* 0 bytes per chunk:                         */ {  0,  0, { 0 } },
	/* 1 byte  per chunk: 00 11 22 33 44 55 66 77 */ {  1,  3, { 0x04, 0x00, 0x80 } },
	/* 2 bytes per chunk:  0011  2233  4455  6677 */ {  2,  6, { 0x8c, 0x0c, 0x08, 0x04, 0x00, 0x80 } },
	/* 3 bytes per chunk:                         */ {  0,  0, { 0 } },
	/* 4 bytes per chunk:   00112233    44556677  */ {  4, 12, { 0x9c, 0x9c, 0x1c, 0x18, 0x14, 0x10, 0x0c, 0x08, 0x04, 0x00, 0x80, 0x80 } },
	/* 5 bytes per chunk:                         */ {  0,  0, { 0 } },
	/* 6 bytes per chunk:                         */ {  0,  0, { 0 } },
	/* 7 bytes per chunk:                         */ {  0,  0, { 0 } },
	/* 8 bytes per chunk:     0011223344556677    */ {  8, 24, { 0xbc, 0xbc, 0xbc, 0xbc, 0x3c, 0x38, 0x34, 0x30, 0x2c, 0x28, 0x24, 0x20, 0x1c, 0x18, 0x14, 0x10, 0x0c, 0x08, 0x04, 0x00, 0x80, 0x80, 0x80, 0x80 } },
	/* 32 bit floating point:                     */ {  4, 16, { 0 } },
	/* 64 bit floating point:                     */ {  8, 32, { 0 } },
	/* 80 bit floating point:                     */ { 10, 32, { 0 } },
	/* 8 bit octal:                               */ {  1,  4, { 0x06, 0x03, 0x00, 0x80 } },
	/* 16 bit octal:                              */ {  2,  8, { 0x8f, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x00, 0x80 } },
	/* 32 bit octal:                              */ {  4, 15, { 0x9e, 0x9e, 0x1e, 0x1b, 0x18, 0x15, 0x12, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x00, 0x80, 0x80 } },
	/* 64 bit octal:                              */ {  8, 28, { 0xbf, 0xbf, 0xbf, 0x3f, 0x3c, 0x39, 0x36, 0x33, 0x30, 0x2d, 0x2a, 0x27, 0x24, 0x21, 0x1e, 0x1b, 0x18, 0x15, 0x12, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x00, 0x80, 0x80, 0x80 } },
};


//**************************************************************************
//  DEBUG VIEW MEMORY SOURCE
//**************************************************************************

//-------------------------------------------------
//  debug_view_memory_source - constructors
//-------------------------------------------------

debug_view_memory_source::debug_view_memory_source(std::string &&name, address_space &space)
	: debug_view_source(std::move(name), &space.device())
	, m_space(&space)
	, m_memintf(dynamic_cast<device_memory_interface *>(&space.device()))
	, m_base(nullptr)
	, m_blocklength(0)
	, m_numblocks(0)
	, m_blockstride(0)
	, m_offsetxor(0)
	, m_endianness(space.endianness())
	, m_prefsize(space.data_width() / 8)
{
}

debug_view_memory_source::debug_view_memory_source(std::string &&name, memory_region &region)
	: debug_view_source(std::move(name))
	, m_space(nullptr)
	, m_memintf(nullptr)
	, m_base(region.base())
	, m_blocklength(region.bytes())
	, m_numblocks(1)
	, m_blockstride(0)
	, m_offsetxor(region.endianness() == ENDIANNESS_NATIVE ? 0 : region.bytewidth() - 1)
	, m_endianness(region.endianness())
	, m_prefsize(std::min<u8>(region.bytewidth(), 8))
{
}

debug_view_memory_source::debug_view_memory_source(std::string &&name, void *base, int element_size, int num_elements, int num_blocks, int block_stride)
	: debug_view_source(std::move(name))
	, m_space(nullptr)
	, m_memintf(nullptr)
	, m_base(base)
	, m_blocklength(element_size * num_elements)
	, m_numblocks(num_blocks)
	, m_blockstride(block_stride)
	, m_offsetxor(0)
	, m_endianness(ENDIANNESS_NATIVE)
	, m_prefsize(std::min(element_size, 8))
{
}



//**************************************************************************
//  DEBUG VIEW MEMORY
//**************************************************************************

//-------------------------------------------------
//  debug_view_memory - constructor
//-------------------------------------------------

debug_view_memory::debug_view_memory(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate)
	: debug_view(machine, DVT_MEMORY, osdupdate, osdprivate),
		m_expression(machine),
		m_chunks_per_row(16),
		m_bytes_per_chunk(1),
		m_steps_per_chunk(1),
		m_data_format(data_format::HEX_8BIT),
		m_reverse_view(false),
		m_ascii_view(true),
		m_no_translation(false),
		m_edit_enabled(true),
		m_shift_bits(4),
		m_address_radix(16),
		m_maxaddr(0),
		m_bytes_per_row(16),
		m_byte_offset(0)
{
	// hack: define some sane init values
	// that don't hurt the initial computation of top_left
	// in set_cursor_pos()
	m_section[0].m_pos = 0;
	m_section[0].m_width = 1 + 8 + 1;
	m_section[1].m_pos = m_section[0].m_pos + m_section[0].m_width;

	// fail if no available sources
	enumerate_sources();
	if (m_source_list.empty())
		throw std::bad_alloc();

	// configure the view
	m_supports_cursor = true;
}


//-------------------------------------------------
//  enumerate_sources - enumerate all possible
//  sources for a memory view
//-------------------------------------------------

void debug_view_memory::enumerate_sources()
{
	// start with an empty list
	m_source_list.clear();
	m_source_list.reserve(machine().save().registration_count());

	// first add all the devices' address spaces
	for (device_memory_interface &memintf : memory_interface_enumerator(machine().root_device()))
	{
		for (int spacenum = 0; spacenum < memintf.max_space_count(); ++spacenum)
		{
			if (memintf.has_space(spacenum))
			{
				address_space &space(memintf.space(spacenum));
				m_source_list.emplace_back(
						std::make_unique<debug_view_memory_source>(
							util::string_format("%s '%s' %s space memory", memintf.device().name(), memintf.device().tag(), space.name()),
							space));
			}
		}
	}

	// then add all the memory regions
	for (auto &region : machine().memory().regions())
	{
		m_source_list.emplace_back(
				std::make_unique<debug_view_memory_source>(
					util::string_format("Region '%s'", region.second->name()),
					*region.second.get()));
	}

	// finally add all global array symbols in ASCII order
	std::string name;
	std::size_t const firstsave = m_source_list.size();
	for (int itemnum = 0; itemnum < machine().save().registration_count(); itemnum++)
	{
		u32 valsize, valcount, blockcount, stride;
		void *base;
		name = machine().save().indexed_item(itemnum, base, valsize, valcount, blockcount, stride);

		// add pretty much anything that's not a timer (we may wish to cull other items later)
		// also, don't trim the front of the name, it's important to know which VIA6522 we're looking at, e.g.
		if (strncmp(name.c_str(), "timer/", 6))
			m_source_list.emplace_back(std::make_unique<debug_view_memory_source>(std::move(name), base, valsize, valcount, blockcount, stride));
	}
	std::sort(
			std::next(m_source_list.begin(), firstsave),
			m_source_list.end(),
			[] (auto const &x, auto const &y) { return 0 > std::strcmp(x->name(), y->name()); });

	// reset the source to a known good entry
	if (!m_source_list.empty())
		set_source(*m_source_list[0]);
}


//-------------------------------------------------
//  view_notify - handle notification of updates
//  to cursor changes
//-------------------------------------------------

void debug_view_memory::view_notify(debug_view_notification type)
{
	if (type == VIEW_NOTIFY_CURSOR_CHANGED)
	{
		// normalize the cursor
		set_cursor_pos(get_cursor_pos(m_cursor));
	}
	else if (type == VIEW_NOTIFY_SOURCE_CHANGED)
	{
		// update for the new source
		const debug_view_memory_source &source = downcast<const debug_view_memory_source &>(*m_source);
		m_chunks_per_row = m_bytes_per_chunk * m_chunks_per_row / source.m_prefsize;
		m_bytes_per_chunk = source.m_prefsize;
		if (m_bytes_per_chunk > 8)
			m_bytes_per_chunk = 8;
		bool octal = source.m_space != nullptr && source.m_space->is_octal();
		switch (m_bytes_per_chunk)
		{
		case 1:
			m_data_format = octal ? data_format::OCTAL_8BIT : data_format::HEX_8BIT;
			break;

		case 2:
			m_data_format = octal ? data_format::OCTAL_16BIT : data_format::HEX_16BIT;
			break;

		case 4:
			m_data_format = octal ? data_format::OCTAL_32BIT : data_format::HEX_32BIT;
			break;

		case 8:
			m_data_format = octal ? data_format::OCTAL_64BIT : data_format::HEX_64BIT;
			break;
		}
		m_shift_bits = octal ? 3 : 4;
		m_steps_per_chunk = source.m_space ? source.m_space->byte_to_address(m_bytes_per_chunk) : m_bytes_per_chunk;
		if (source.m_space != nullptr)
			m_expression.set_context(&source.m_space->device().debug()->symtable());
		else
			m_expression.set_context(nullptr);
		m_address_radix = octal ? 8 : 16;
		m_expression.set_default_base(m_address_radix);
	}
}


//-------------------------------------------------
//  u32_to_float - return a floating point number
//  whose 32 bit representation is value
//-------------------------------------------------

static inline float u32_to_float(u32 value)
{
	union {
		float f;
		u32 i;
	} v;

	v.i = value;
	return v.f;
}

//-------------------------------------------------
//  u64_to_double - return a floating point number
//  whose 64 bit representation is value
//-------------------------------------------------

static inline float u64_to_double(u64 value)
{
	union {
		double f;
		u64 i;
	} v;

	v.i = value;
	return v.f;
}

//-------------------------------------------------
//  generate_row - read one row of data and make a
//  text representation of the chunks
//-------------------------------------------------

void debug_view_memory::generate_row(debug_view_char *destmin, debug_view_char *destmax, debug_view_char *destrow, offs_t address)
{
	// get positional data
	const memory_view_pos &posdata = get_posdata(m_data_format);
	int spacing = posdata.m_spacing;

	// generate the address
	char addrtext[20];
	snprintf(addrtext, 20, m_addrformat.c_str(), address);
	debug_view_char *dest = destrow + m_section[0].m_pos + 1;
	for (int ch = 0; addrtext[ch] != 0 && ch < m_section[0].m_width - 1; ch++, dest++)
		if (dest >= destmin && dest < destmax)
			dest->byte = addrtext[ch];

	// generate the data and the ASCII string
	std::string chunkascii;
	if (m_shift_bits != 0)
	{
		for (int chunknum = 0; chunknum < m_chunks_per_row; chunknum++)
		{
			u64 chunkdata;
			bool ismapped = read_chunk(address, chunknum, chunkdata);

			int chunkindex = m_reverse_view ? (m_chunks_per_row - 1 - chunknum) : chunknum;
			dest = destrow + m_section[1].m_pos + 1 + chunkindex * spacing;
			for (int ch = 0; ch < spacing; ch++, dest++)
				if (dest >= destmin && dest < destmax)
				{
					u8 shift = posdata.m_shift[ch];
					if (shift < 64)
						dest->byte = ismapped ? "0123456789ABCDEF"[BIT(chunkdata, shift, m_shift_bits)] : '*';
				}

			for (int i = 0; i < m_bytes_per_chunk; i++)
			{
				u8 chval = chunkdata >> (8 * (m_bytes_per_chunk - i - 1));
				chunkascii += char(ismapped ? sanitise_character(chval) : '.');
			}
		}
	}
	else
	{
		for (int chunknum = 0; chunknum < m_chunks_per_row; chunknum++)
		{
			char valuetext[64];
			u64 chunkdata = 0;
			extFloat80_t chunkdata80 = { 0, 0 };
			bool ismapped;

			if (m_data_format != data_format::FLOAT_80BIT)
				ismapped = read(m_bytes_per_chunk, address + chunknum * m_steps_per_chunk, chunkdata);
			else
				ismapped = read(m_bytes_per_chunk, address + chunknum * m_steps_per_chunk, chunkdata80);

			if (ismapped)
				switch (m_data_format)
				{
				case data_format::FLOAT_32BIT:
					snprintf(valuetext, 64, "%.8g", u32_to_float(u32(chunkdata)));
					break;
				case data_format::FLOAT_64BIT:
					snprintf(valuetext, 64, "%.24g", u64_to_double(chunkdata));
					break;
				case data_format::FLOAT_80BIT:
				{
					float64_t f64 = extF80M_to_f64(&chunkdata80);
					snprintf(valuetext, 64, "%.24g", u64_to_double(f64.v));
					break;
				}
				default:
					break;
				}
			else
			{
				valuetext[0] = '*';
				valuetext[1] = 0;
			}

			int ch;
			int chunkindex = m_reverse_view ? (m_chunks_per_row - 1 - chunknum) : chunknum;
			dest = destrow + m_section[1].m_pos + 1 + chunkindex * spacing;
			// first copy the text
			for (ch = 0; (ch < spacing) && (valuetext[ch] != 0); ch++, dest++)
				if (dest >= destmin && dest < destmax)
					dest->byte = valuetext[ch];
			// then fill with spaces
			for (; ch < spacing; ch++, dest++)
				if (dest >= destmin && dest < destmax)
					dest->byte = ' ';

			for (int i = 0; i < m_bytes_per_chunk; i++)
			{
				u8 chval = chunkdata >> (8 * (m_bytes_per_chunk - i - 1));
				chunkascii += char(ismapped ? sanitise_character(chval) : '.');
			}
		}
	}

	// generate the ASCII data, but follow the chunks
	if (m_section[2].m_width > 0)
	{
		dest = destrow + m_section[2].m_pos + 1;
		for (size_t i = 0; i != chunkascii.size(); i++)
		{
			if (dest >= destmin && dest < destmax)
				dest->byte = chunkascii[i];
			dest++;
		}
	}
}

//-------------------------------------------------
//  view_update - update the contents of the
//  memory view
//-------------------------------------------------

void debug_view_memory::view_update()
{
	const debug_view_memory_source &source = downcast<const debug_view_memory_source &>(*m_source);

	// if we need to recompute, do it now
	if (needs_recompute())
		recompute();

	// loop over visible rows
	for (u32 row = 0; row < m_visible.y; row++)
	{
		debug_view_char *destmin = &m_viewdata[row * m_visible.x];
		debug_view_char *destmax = destmin + m_visible.x;
		debug_view_char *destrow = destmin - m_topleft.x;
		u32 effrow = m_topleft.y + row;

		// reset the line of data; section 1 is normal, others are ancillary, cursor is selected
		u32 effcol = m_topleft.x;
		for (debug_view_char *dest = destmin; dest != destmax; dest++, effcol++)
		{
			dest->byte = ' ';
			dest->attrib = DCA_ANCILLARY;
			if (m_section[1].contains(effcol))
			{
				dest->attrib = DCA_NORMAL;
				if (m_cursor_visible && effrow == m_cursor.y && effcol == m_cursor.x)
					dest->attrib |= DCA_SELECTED;
			}
		}

		// if this visible row is valid, add it to the buffer
		if (effrow < m_total.y)
		{
			offs_t addrbyte = m_byte_offset + effrow * m_bytes_per_row;
			offs_t address = (source.m_space != nullptr) ? source.m_space->byte_to_address(addrbyte) : addrbyte;
			generate_row(destmin, destmax, destrow, address);
		}
	}
}


//-------------------------------------------------
//  view_char - handle a character typed within
//  the current view
//-------------------------------------------------

void debug_view_memory::view_char(int chval)
{
	// get the position
	cursor_pos pos = get_cursor_pos(m_cursor);

	// editing is not supported when showing floating point values
	if (m_edit_enabled == false)
		return;

	// handle the incoming key
	switch (chval)
	{
		case DCH_UP:
			if (pos.m_address >= m_byte_offset + m_bytes_per_row)
				pos.m_address -= m_bytes_per_row;
			break;

		case DCH_DOWN:
			if (pos.m_address <= m_maxaddr - m_bytes_per_row)
				pos.m_address += m_bytes_per_row;
			break;

		case DCH_PUP:
			for (u32 delta = (m_visible.y - 2) * m_bytes_per_row; delta > 0; delta -= m_bytes_per_row)
				if (pos.m_address >= m_byte_offset + delta)
				{
					pos.m_address -= delta;
					break;
				}
			break;

		case DCH_PDOWN:
			for (u32 delta = (m_visible.y - 2) * m_bytes_per_row; delta > 0; delta -= m_bytes_per_row)
				if (pos.m_address <= m_maxaddr - delta)
				{
					pos.m_address += delta;
					break;
				}
			break;

		case DCH_HOME:
			pos.m_address -= pos.m_address % m_bytes_per_row;
			pos.m_shift = get_posdata(m_data_format).m_shift[0] & 0x7f;
			break;

		case DCH_CTRLHOME:
			pos.m_address = m_byte_offset;
			pos.m_shift = get_posdata(m_data_format).m_shift[0] & 0x7f;
			break;

		case DCH_END:
			pos.m_address += (m_bytes_per_row - (pos.m_address % m_bytes_per_row) - 1);
			pos.m_shift = 0;
			break;

		case DCH_CTRLEND:
			pos.m_address = m_maxaddr;
			pos.m_shift = 0;
			break;

		case DCH_CTRLLEFT:
			if (pos.m_address >= m_byte_offset + m_bytes_per_chunk)
				pos.m_address -= m_bytes_per_chunk;
			break;

		case DCH_CTRLRIGHT:
			if (pos.m_address <= m_maxaddr - m_bytes_per_chunk)
				pos.m_address += m_bytes_per_chunk;
			break;

		default:
			{
				static const char hexvals[] = "0123456789abcdef";
				char *hexchar = (char *)strchr(hexvals, tolower(chval));
				if (hexchar == nullptr || (m_shift_bits == 3 && chval >= '8'))
					break;

				if (!write_digit(pos.m_address, pos.m_shift, hexchar - hexvals))
					break; // TODO: alert OSD?
			}
			// fall through to the right-arrow press
			[[fallthrough]];
		case DCH_RIGHT:
			if (pos.m_shift != 0)
				pos.m_shift -= m_shift_bits;
			else if (pos.m_address != m_maxaddr)
			{
				pos.m_shift = get_posdata(m_data_format).m_shift[0] & 0x7f;
				pos.m_address += m_bytes_per_chunk;
			}
			break;

		case DCH_LEFT:
			if (pos.m_shift != (get_posdata(m_data_format).m_shift[0] & 0x7f))
				pos.m_shift += m_shift_bits;
			else if (pos.m_address != m_byte_offset)
			{
				pos.m_shift = 0;
				pos.m_address -= m_bytes_per_chunk;
			}
			break;
	}

	// set a new position
	begin_update();
	set_cursor_pos(pos);
	m_update_pending = true;
	end_update();
}


//-------------------------------------------------
//  view_click - handle a mouse click within the
//  current view
//-------------------------------------------------

void debug_view_memory::view_click(const int button, const debug_view_xy& pos)
{
	const debug_view_xy origcursor = m_cursor;
	m_cursor = pos;

	/* cursor popup|toggle */
	bool cursorVisible = true;
	if (m_cursor.y == origcursor.y && m_cursor.x == origcursor.x)
	{
		cursorVisible = !m_cursor_visible;
	}

	/* send a cursor changed notification */
	begin_update();
	m_cursor_visible = cursorVisible;
	view_notify(VIEW_NOTIFY_CURSOR_CHANGED);
	m_update_pending = true;
	end_update();
}


//-------------------------------------------------
//  recompute - recompute the internal data and
//  structure of the memory view
//-------------------------------------------------

void debug_view_memory::recompute()
{
	const debug_view_memory_source &source = downcast<const debug_view_memory_source &>(*m_source);

	// get the current cursor position
	cursor_pos pos = get_cursor_pos(m_cursor);

	// determine the maximum address and address format string from the raw information
	int addrchars;
	u64 maxbyte;
	if (source.m_space != nullptr)
	{
		m_maxaddr = m_no_translation ? source.m_space->addrmask() : source.m_space->logaddrmask();
		maxbyte = source.m_space->address_to_byte_end(m_maxaddr);
		if (m_address_radix == 8)
			addrchars = ((m_no_translation ? source.m_space->addr_width() : source.m_space->logaddr_width()) + 2) / 3;
		else
			addrchars = m_no_translation ? source.m_space->addrchars() : source.m_space->logaddrchars();
	}
	else
	{
		maxbyte = m_maxaddr = (source.m_blocklength * source.m_numblocks) - 1;
		if (m_address_radix == 8)
			addrchars = string_format("%o", m_maxaddr).size();
		else
			addrchars = string_format("%X", m_maxaddr).size();
	}

	// generate an 8-byte aligned format for the address
	switch (m_address_radix)
	{
	case 8:
		if (!m_reverse_view)
			m_addrformat = string_format("%*s%%0%do", 11 - addrchars, "", addrchars);
		else
			m_addrformat = string_format("%%0%do%*s", addrchars, 11 - addrchars, "");
		break;

	case 10:
		// omit leading zeros for decimal addresses
		m_addrformat = m_reverse_view ? "%-10d" : "%10d";
		break;

	case 16:
		if (!m_reverse_view)
			m_addrformat = string_format("%*s%%0%dX", 8 - addrchars, "", addrchars);
		else
			m_addrformat = string_format("%%0%dX%*s", addrchars, 8 - addrchars, "");
		break;
	}

	// if we are viewing a space with a minimum chunk size, clamp the bytes per chunk
	// BAD
#if 0
	if (source.m_space != nullptr && source.m_space->byte_to_address(1) > 1)
	{
		u32 min_bytes_per_chunk = source.m_space->byte_to_address(1);
		while (m_bytes_per_chunk < min_bytes_per_chunk)
		{
			m_bytes_per_chunk *= 2;
			m_chunks_per_row /= 2;
		}
		m_chunks_per_row = std::max(1U, m_chunks_per_row);
	}
#endif

	// recompute the byte offset based on the most recent expression result
	m_bytes_per_row = m_bytes_per_chunk * m_chunks_per_row;
	offs_t val = m_expression.value();
	if (source.m_space)
		val = source.m_space->address_to_byte(val);
	m_byte_offset = val % m_bytes_per_row;

	// compute the section widths
	switch (m_address_radix)
	{
	case 8:
		m_section[0].m_width = 1 + 11 + 1;
		break;

	case 10:
		m_section[0].m_width = 1 + 10 + 1;
		break;

	case 16:
		m_section[0].m_width = 1 + 8 + 1;
		break;
	}
	m_section[1].m_width = 1 + get_posdata(m_data_format).m_spacing * m_chunks_per_row + 1;
	m_section[2].m_width = m_ascii_view ? (1 + m_bytes_per_row + 1) : 0;

	// compute the section positions
	if (!m_reverse_view)
	{
		m_section[0].m_pos = 0;
		m_section[1].m_pos = m_section[0].m_pos + m_section[0].m_width;
		m_section[2].m_pos = m_section[1].m_pos + m_section[1].m_width;
		m_total.x = m_section[2].m_pos + m_section[2].m_width;
	}
	else
	{
		m_section[2].m_pos = 0;
		m_section[1].m_pos = m_section[2].m_pos + m_section[2].m_width;
		m_section[0].m_pos = m_section[1].m_pos + m_section[1].m_width;
		m_total.x = m_section[0].m_pos + m_section[0].m_width;
	}

	// derive total sizes from that
	m_total.y = (maxbyte - u64(m_byte_offset) + u64(m_bytes_per_row) /*- 1*/) / m_bytes_per_row;

	// reset the current cursor position
	set_cursor_pos(pos);
}


//-------------------------------------------------
//  needs_recompute - determine if anything has
//  changed that requires a recomputation
//-------------------------------------------------

bool debug_view_memory::needs_recompute()
{
	bool recompute = m_recompute;

	// handle expression changes
	if (m_expression.dirty())
	{
		const debug_view_memory_source &source = downcast<const debug_view_memory_source &>(*m_source);
		offs_t val = m_expression.value();
		if (source.m_space)
			val = source.m_space->address_to_byte(val & (m_no_translation ? source.m_space->addrmask() : source.m_space->logaddrmask()));
		recompute = true;

		m_byte_offset = val % m_bytes_per_row;
		m_topleft.y = std::min(s32(val / m_bytes_per_row), m_total.y - 1);

		set_cursor_pos(cursor_pos(val, m_bytes_per_chunk * 8 - 4));
	}

	// expression is clean at this point, and future recomputation is not necessary
	m_recompute = false;
	return recompute;
}


//-------------------------------------------------
//  get_cursor_pos - return the cursor position as
//  an address and a shift value
//-------------------------------------------------

debug_view_memory::cursor_pos debug_view_memory::get_cursor_pos(const debug_view_xy& cursor)
{
	// start with the base address for this row
	cursor_pos pos;
	const memory_view_pos &posdata = get_posdata(m_data_format);
	pos.m_address = m_byte_offset + cursor.y * m_bytes_per_chunk * m_chunks_per_row;

	// determine the X position within the middle section, clamping as necessary
	if (posdata.m_shift[0] != 0) {
		int xposition = cursor.x - m_section[1].m_pos - 1;
		if (xposition < 0)
			xposition = 0;
		else if (xposition >= posdata.m_spacing * m_chunks_per_row)
			xposition = posdata.m_spacing * m_chunks_per_row - 1;

		// compute chunk number and offset within that chunk
		int chunknum = xposition / posdata.m_spacing;
		int chunkoffs = xposition % posdata.m_spacing;

		// reverse the chunknum if we're reversed
		if (m_reverse_view)
			chunknum = m_chunks_per_row - 1 - chunknum;

		// compute the address and shift
		pos.m_address += chunknum * m_bytes_per_chunk;
		pos.m_shift = posdata.m_shift[chunkoffs] & 0x7f;
	}
	else {
		int xposition = cursor.x - m_section[1].m_pos - 1;
		// check for lower limit
		if (xposition < 0)
			xposition = 0;
		int chunknum = xposition / posdata.m_spacing;
		// check for upper limit
		if (chunknum >= m_chunks_per_row)
			chunknum = m_chunks_per_row - 1;
		// reverse the chunknum if we're reversed
		if (m_reverse_view)
			chunknum = m_chunks_per_row - 1 - chunknum;
		// compute the address
		pos.m_address += chunknum * m_bytes_per_chunk;
		pos.m_shift = 0;
	}

	return pos;
}


//-------------------------------------------------
//  set_cursor_pos - set the cursor position as a
//  function of an address and a shift value
//-------------------------------------------------

void debug_view_memory::set_cursor_pos(cursor_pos pos)
{
	const memory_view_pos &posdata = get_posdata(m_data_format);

	// offset the address by the byte offset
	if (pos.m_address < m_byte_offset)
		pos.m_address = m_byte_offset;
	pos.m_address -= m_byte_offset;

	// compute the Y coordinate and chunk index
	m_cursor.y = pos.m_address / m_bytes_per_row;
	int chunknum = (pos.m_address % m_bytes_per_row) / m_bytes_per_chunk;

	// reverse the chunknum if we're reversed
	if (m_reverse_view)
		chunknum = m_chunks_per_row - 1 - chunknum;

	if (posdata.m_shift[0] != 0) {
		// scan within the chunk to find the shift
		for (m_cursor.x = 0; m_cursor.x < posdata.m_spacing; m_cursor.x++)
			if (posdata.m_shift[m_cursor.x] == pos.m_shift)
				break;

		// add in the chunk offset and shift to the right of divider1
		m_cursor.x += m_section[1].m_pos + 1 + posdata.m_spacing * chunknum;
	}
	else {
		m_cursor.x = m_section[1].m_pos + 1 + posdata.m_spacing * chunknum;
	}

	// clamp to the window bounds
	m_cursor.x = std::min(m_cursor.x, m_total.x);
	m_cursor.y = std::min(m_cursor.y, m_total.y);

	// scroll if out of range
	adjust_visible_x_for_cursor();
	adjust_visible_y_for_cursor();
}


//-------------------------------------------------
//  read - generic memory view data reader
//-------------------------------------------------

bool debug_view_memory::read(u8 size, offs_t offs, u64 &data)
{
	const debug_view_memory_source &source = downcast<const debug_view_memory_source &>(*m_source);

	// if no raw data, just use the standard debug routines
	if (source.m_space)
	{
		auto dis = machine().disable_side_effects();

		bool ismapped = offs <= m_maxaddr;
		address_space *tspace;
		if (ismapped && !m_no_translation)
		{
			offs_t dummyaddr = offs;
			ismapped = source.m_memintf->translate(source.m_space->spacenum(), device_memory_interface::TR_READ, dummyaddr, tspace);
		}
		else
			tspace = source.m_space;
		data = ~u64(0);
		if (ismapped)
			data = m_expression.context().read_memory(*tspace, offs, size, !m_no_translation);
		return ismapped;
	}

	// if larger than a byte, reduce by half and recurse
	if (size > 1)
	{
		size /= 2;

		u64 data0, data1;
		bool ismapped = read(size, offs + 0 * size, data0);
		ismapped |= read(size, offs + 1 * size, data1);

		if (source.m_endianness == ENDIANNESS_LITTLE)
			data = data0 | (data1 << (size * 8));
		else
			data = data1 | (data0 << (size * 8));
		return ismapped;
	}

	// all 0xff if out of bounds
	offs ^= source.m_offsetxor;
	if (offs >= (source.m_blocklength * source.m_numblocks))
		return false;
	data = *(reinterpret_cast<const u8 *>(source.m_base) + (offs / source.m_blocklength * source.m_blockstride) + (offs % source.m_blocklength));
	return true;
}


//-------------------------------------------------
//  read - read a 80 bit value
//-------------------------------------------------

bool debug_view_memory::read(u8 size, offs_t offs, extFloat80_t &data)
{
	u64 t;
	bool mappedhi, mappedlo;
	const debug_view_memory_source &source = downcast<const debug_view_memory_source &>(*m_source);

	if (source.m_endianness == ENDIANNESS_LITTLE) {
		mappedlo = read(8, offs, data.signif);
		mappedhi = read(2, offs+8, t);
		data.signExp = u16(t);
	}
	else {
		mappedhi = read(2, offs, t);
		data.signExp = u16(t);
		mappedlo = read(8, offs + 2, data.signif);
	}

	return mappedhi && mappedlo;
}


//-------------------------------------------------
//  read_chunk - memory view data reader helper
//-------------------------------------------------

bool debug_view_memory::read_chunk(offs_t address, int chunknum, u64 &chunkdata)
{
	const debug_view_memory_source &source = downcast<const debug_view_memory_source &>(*m_source);
	if (source.m_space) {
		address += source.m_space->byte_to_address(chunknum * m_bytes_per_chunk);
		if (!source.m_space->byte_to_address(m_bytes_per_chunk)) {
			// if chunks are too small to be addressable, read a minimal chunk and split it up
			u8 minbytes = 1 << -source.m_space->addr_shift();
			bool ismapped = read(minbytes, address, chunkdata);
			u8 suboffset = (chunknum * m_bytes_per_chunk) & (minbytes - 1);
			chunkdata >>= 8 * (source.m_space->endianness() == ENDIANNESS_LITTLE ? suboffset : minbytes - m_bytes_per_chunk - suboffset);
			chunkdata &= ~u64(0) >> (64 - 8 * m_bytes_per_chunk);
			return ismapped;
		}
	}
	else
		address += chunknum * m_bytes_per_chunk;
	return read(m_bytes_per_chunk, address, chunkdata);
}


//-------------------------------------------------
//  write - generic memory view data writer
//-------------------------------------------------

void debug_view_memory::write(u8 size, offs_t offs, u64 data)
{
	const debug_view_memory_source &source = downcast<const debug_view_memory_source &>(*m_source);

	// if no raw data, just use the standard debug routines
	if (source.m_space)
	{
		auto dis = machine().disable_side_effects();
		m_expression.context().write_memory(*source.m_space, offs, data, size, !m_no_translation);
		return;
	}

	// if larger than a byte, reduce by half and recurse
	if (size > 1)
	{
		size /= 2;
		if (source.m_endianness == ENDIANNESS_LITTLE)
		{
			write(size, offs + 0 * size, data);
			write(size, offs + 1 * size, data >> (8 * size));
		}
		else
		{
			write(size, offs + 1 * size, data);
			write(size, offs + 0 * size, data >> (8 * size));
		}
		return;
	}

	// ignore if out of bounds
	offs ^= source.m_offsetxor;
	if (offs >= (source.m_blocklength * source.m_numblocks))
		return;
	*(reinterpret_cast<u8 *>(source.m_base) + (offs / source.m_blocklength * source.m_blockstride) + (offs % source.m_blocklength)) = data;
}


//-------------------------------------------------
//  write_digit - write one hex or octal digit
//  at the given address and bit position
//-------------------------------------------------

bool debug_view_memory::write_digit(offs_t offs, u8 pos, u8 digit)
{
	const debug_view_memory_source &source = downcast<const debug_view_memory_source &>(*m_source);
	offs_t address = (source.m_space != nullptr) ? source.m_space->byte_to_address(offs) : offs;
	u64 data;
	bool ismapped = read(m_bytes_per_chunk, address, data);
	if (!ismapped)
		return false;

	// clamp to chunk size
	if (m_bytes_per_chunk * 8 < pos + m_shift_bits)
	{
		assert(m_bytes_per_chunk * 8 > pos);
		digit &= util::make_bitmask<u8>(m_bytes_per_chunk * 8 - pos);
	}

	u64 write_data = (data & ~(util::make_bitmask<u64>(m_shift_bits) << pos)) | (u64(digit) << pos);
	write(m_bytes_per_chunk, address, write_data);

	// verify that data reads back as it was written
	if (source.m_space != nullptr)
	{
		read(m_bytes_per_chunk, address, data);
		return data == write_data;
	}
	else
		return true;
}


//-------------------------------------------------
//  set_expression - set the expression string
//  describing the home address
//-------------------------------------------------

void debug_view_memory::set_expression(const std::string &expression)
{
	begin_update();
	m_expression.set_string(expression);
	m_recompute = m_update_pending = true;
	end_update();
}


//-------------------------------------------------
//  set_chunks_per_row - specify the number of
//  chunks displayed across a row
//-------------------------------------------------

void debug_view_memory::set_chunks_per_row(u32 rowchunks)
{
	if (rowchunks < 1)
		return;

	cursor_pos pos = begin_update_and_get_cursor_pos();
	m_chunks_per_row = rowchunks;
	m_recompute = m_update_pending = true;
	end_update_and_set_cursor_pos(pos);
}


//-------------------------------------------------
//  set_data_format - specify what kind of values
//  are shown
//-------------------------------------------------

void debug_view_memory::set_data_format(data_format format)
{
	cursor_pos pos;

	// should never be
	if (!is_valid_format(format))
		return;
	// no need to change
	if (format == m_data_format)
		return;

	pos = begin_update_and_get_cursor_pos();
	const debug_view_memory_source &source = downcast<const debug_view_memory_source &>(*m_source);
	if (is_hex_format(format) && is_hex_format(m_data_format)) {
		pos.m_address += (pos.m_shift / 8) ^ ((source.m_endianness == ENDIANNESS_LITTLE) ? 0 : (m_bytes_per_chunk - 1));
		pos.m_shift %= 8;

		m_bytes_per_chunk = get_posdata(format).m_bytes;
		m_steps_per_chunk = source.m_space ? source.m_space->byte_to_address(m_bytes_per_chunk) : m_bytes_per_chunk;
		m_chunks_per_row = m_bytes_per_row / m_bytes_per_chunk;
		if (m_chunks_per_row < 1)
			m_chunks_per_row = 1;

		pos.m_shift += 8 * ((pos.m_address % m_bytes_per_chunk) ^ ((source.m_endianness == ENDIANNESS_LITTLE) ? 0 : (m_bytes_per_chunk - 1)));
		pos.m_address -= pos.m_address % m_bytes_per_chunk;
	} else {
		if (is_hex_format(format)) {
			m_supports_cursor = true;
			m_edit_enabled = true;
			m_shift_bits = 4;
		}
		else if (is_octal_format(format)) {
			m_supports_cursor = true;
			m_edit_enabled = true;
			m_shift_bits = 3;
		}
		else {
			m_supports_cursor = false;
			m_edit_enabled = false;
			m_cursor_visible = false;
			m_shift_bits = 0;
		}

		m_bytes_per_chunk = get_posdata(format).m_bytes;
		m_chunks_per_row = m_bytes_per_row / m_bytes_per_chunk;
		if (m_chunks_per_row < 1)
			m_chunks_per_row = 1;
		m_steps_per_chunk = source.m_space ? source.m_space->byte_to_address(m_bytes_per_chunk) : m_bytes_per_chunk;
		pos.m_shift = get_posdata(format).m_shift[0] & 0x7f;
		pos.m_address -= pos.m_address % m_bytes_per_chunk;
	}
	m_recompute = m_update_pending = true;
	m_data_format = format;
	end_update_and_set_cursor_pos(pos);
}

//-------------------------------------------------
//  set_reverse - specify true if the memory view
//  is displayed reverse
//-------------------------------------------------

void debug_view_memory::set_reverse(bool reverse)
{
	cursor_pos pos = begin_update_and_get_cursor_pos();
	m_reverse_view = reverse;
	m_recompute = m_update_pending = true;
	end_update_and_set_cursor_pos(pos);
}


//-------------------------------------------------
//  set_ascii - specify true if the memory view
//  should display an ASCII representation
//-------------------------------------------------

void debug_view_memory::set_ascii(bool ascii)
{
	cursor_pos pos = begin_update_and_get_cursor_pos();
	m_ascii_view = ascii;
	m_recompute = m_update_pending = true;
	end_update_and_set_cursor_pos(pos);
}


//-------------------------------------------------
//  set_physical - specify true if the memory view
//  should display physical addresses versus
//  logical addresses
//-------------------------------------------------

void debug_view_memory::set_physical(bool physical)
{
	cursor_pos pos = begin_update_and_get_cursor_pos();
	m_no_translation = physical;
	m_recompute = m_update_pending = true;
	end_update_and_set_cursor_pos(pos);
}


//-------------------------------------------------
//  set_address_radix - specify whether the memory
//  view should display addresses in base 8, base
//  10 or base 16
//-------------------------------------------------

void debug_view_memory::set_address_radix(int radix)
{
	if (radix != 8 && radix != 10 && radix != 16)
		return;

	cursor_pos pos = begin_update_and_get_cursor_pos();
	m_address_radix = radix;
	m_expression.set_default_base(radix);
	m_recompute = m_update_pending = true;
	end_update_and_set_cursor_pos(pos);
}
