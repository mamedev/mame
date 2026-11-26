// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen, Morten Shearman Kirkegaard
/**********************************************************************

    Axell AG-1 / AG-2 character data decompressor

**********************************************************************/

#include "emu.h"

#include "axell_decomp.h"

#include <algorithm>
#include <cstring>
#include <iterator>


namespace {

enum class huffman_op : u8
{
	NEW_COLOUR,     // repeat an explicit colour
	PREV_COLOUR,    // repeat one of the 16 most recently used colours
	COPY,           // copy from the neighbouring row of the strip
	COPY_PLUS_ONE,  // likewise, one pixel to the right
	COPY_MINUS_ONE  // likewise, one pixel to the left
};

struct huffman_node
{
	const char *bits;
	huffman_op op;
	u8 arg_bits;
	u8 count;
};

constexpr huffman_node HUFFMAN_NODES[] = {
	{ "11",                 huffman_op::NEW_COLOUR,     8,  1 },
	{ "10111",              huffman_op::NEW_COLOUR,     8,  2 },
	{ "10110011",           huffman_op::NEW_COLOUR,     8,  3 },
	{ "1010011000",         huffman_op::NEW_COLOUR,     8,  4 },
	{ "1010101010000",      huffman_op::NEW_COLOUR,     8,  5 },
	{ "101010001011110",    huffman_op::NEW_COLOUR,     8,  6 },
	{ "101010010101110",    huffman_op::NEW_COLOUR,     8,  7 },
	{ "101010010101111",    huffman_op::NEW_COLOUR,     8,  8 },
	{ "101010010101100",    huffman_op::NEW_COLOUR,     8,  9 },
	{ "10101000101100",     huffman_op::NEW_COLOUR,     8, 10 },
	{ "101010010101101",    huffman_op::NEW_COLOUR,     8, 11 },
	{ "10101000101101",     huffman_op::NEW_COLOUR,     8, 12 },
	{ "0",                  huffman_op::PREV_COLOUR,    4,  1 },
	{ "100",                huffman_op::PREV_COLOUR,    4,  2 },
	{ "101011",             huffman_op::PREV_COLOUR,    4,  3 },
	{ "1010010",            huffman_op::PREV_COLOUR,    4,  4 },
	{ "101010100",          huffman_op::PREV_COLOUR,    4,  5 },
	{ "1010100100",         huffman_op::PREV_COLOUR,    4,  6 },
	{ "10101010110",        huffman_op::PREV_COLOUR,    4,  7 },
	{ "10100111000",        huffman_op::PREV_COLOUR,    4,  8 },
	{ "101010101001",       huffman_op::PREV_COLOUR,    4,  9 },
	{ "101001111000",       huffman_op::PREV_COLOUR,    4, 10 },
	{ "101010010100",       huffman_op::PREV_COLOUR,    4, 11 },
	{ "1010011001",         huffman_op::PREV_COLOUR,    4, 12 },
	{ "101101",             huffman_op::COPY,           0,  2 },
	{ "10101011",           huffman_op::COPY,           0,  3 },
	{ "101010011",          huffman_op::COPY,           0,  4 },
	{ "101001101",          huffman_op::COPY,           0,  5 },
	{ "1010011111",         huffman_op::COPY,           0,  6 },
	{ "1010100011",         huffman_op::COPY,           0,  7 },
	{ "10101000100",        huffman_op::COPY,           0,  8 },
	{ "101010101111",       huffman_op::COPY,           0,  9 },
	{ "101001110010",       huffman_op::COPY,           0, 10 },
	{ "1010011100111",      huffman_op::COPY,           0, 11 },
	{ "101100101",          huffman_op::COPY,           0, 12 },
	{ "1011000",            huffman_op::COPY_PLUS_ONE,  0,  2 },
	{ "101010000",          huffman_op::COPY_PLUS_ONE,  0,  3 },
	{ "10101001011",        huffman_op::COPY_PLUS_ONE,  0,  4 },
	{ "101010001010",       huffman_op::COPY_PLUS_ONE,  0,  5 },
	{ "1010101011101",      huffman_op::COPY_PLUS_ONE,  0,  6 },
	{ "1010101011100",      huffman_op::COPY_PLUS_ONE,  0,  7 },
	{ "1010011110010",      huffman_op::COPY_PLUS_ONE,  0,  8 },
	{ "10101000101110",     huffman_op::COPY_PLUS_ONE,  0,  9 },
	{ "101010001011111",    huffman_op::COPY_PLUS_ONE,  0, 10 },
	{ "1010011110011",      huffman_op::COPY_PLUS_ONE,  0, 11 },
	{ "101000",             huffman_op::COPY_MINUS_ONE, 0,  2 },
	{ "101100100",          huffman_op::COPY_MINUS_ONE, 0,  3 },
	{ "1010011101",         huffman_op::COPY_MINUS_ONE, 0,  4 },
	{ "10101010101",        huffman_op::COPY_MINUS_ONE, 0,  5 },
	{ "101001111011",       huffman_op::COPY_MINUS_ONE, 0,  6 },
	{ "101001111010",       huffman_op::COPY_MINUS_ONE, 0,  7 },
	{ "1010101010001",      huffman_op::COPY_MINUS_ONE, 0,  8 },
	{ "1010011100110",      huffman_op::COPY_MINUS_ONE, 0,  9 },
	{ "10101001010101",     huffman_op::COPY_MINUS_ONE, 0, 10 },
	{ "10101001010100",     huffman_op::COPY_MINUS_ONE, 0, 11 }
};

} // anonymous namespace


axell_decompressor::axell_decompressor(u8 *dest, u32 x_bits, u32 x_mask, u32 y_mask, bool check_runs) :
	m_error(false),
	m_dest(dest),
	m_x_bits(x_bits),
	m_x_mask(x_mask),
	m_y_mask(y_mask),
	m_check_runs(check_runs),
	m_bits(0),
	m_num_bits(0),
	m_colour{},
	m_dx(0),
	m_dy(0),
	m_ow(0),
	m_oh(0),
	m_ox(0),
	m_oy(0)
{
}


axell_decompressor::~axell_decompressor() = default;


int axell_decompressor::get_next_bit()
{
	if (!m_num_bits)
	{
		m_bits = fetch_byte();
		m_num_bits = 8;
	}

	const int bit = m_bits & 1;
	m_bits >>= 1;
	m_num_bits--;
	return bit;
}


u32 axell_decompressor::get_bits(u32 count)
{
	u32 result = 0;
	for (u32 i = 0; i < count; i++)
		result |= get_next_bit() << i;
	return result;
}


int axell_decompressor::next_node()
{
	char code[128]{};

	for (u32 i = 0; i < std::size(code) - 1; i++)
	{
		code[i] = '0' + get_next_bit();

		const auto node = std::find_if(std::begin(HUFFMAN_NODES), std::end(HUFFMAN_NODES),
				[&code, i] (const huffman_node &n) { return !strncmp(code, n.bits, i + 1); });
		if (node == std::end(HUFFMAN_NODES))
			return -1;
		if (!node->bits[i + 1])
			return int(node - std::begin(HUFFMAN_NODES));
	}

	return -1;
}


void axell_decompressor::set_o(u8 v)
{
	u8 a = v;
	for (u32 i = 0; i < std::size(m_colour); i++)
	{
		const u8 b = m_colour[i];
		m_colour[i] = a;
		a = b;
		if (a == v)
			break;
	}

	if (m_ox >= 0 && m_ox < m_ow && m_oy >= 0 && m_oy < m_oh)
		m_dest[addr(m_dx + m_ox, m_dy + m_oy)] = v;
	m_ox++;
}


u8 axell_decompressor::get_o(int x, int y) const
{
	if (x < 0 || x >= m_ow || y < 0 || y >= m_oh)
		return 0;

	return m_dest[addr(m_dx + x, m_dy + y)];
}


void axell_decompressor::copy(int dx, u32 count)
{
	for (u32 i = 0; i < count; i++)
	{
		if ((m_ox / 12) & 1)
			set_o(get_o(m_ox + dx, m_oy + 1));
		else
			set_o(get_o(m_ox + dx, m_oy - 1));
	}
}


bool axell_decompressor::decompress(int dx, int dy, int width, int height)
{
	m_dx = dx;
	m_dy = dy;
	m_ow = width;
	m_oh = height;
	m_ox = 0;
	m_oy = 0;

	while (m_ox < m_ow)
	{
		if (m_error)
			return true;

		const int index = next_node();
		if (index < 0)
			return true;
		const huffman_node &n = HUFFMAN_NODES[index];

		const u8 v = get_bits(n.arg_bits);
		if (m_error)
			return true;
		if (m_check_runs && (n.count > std::min(12 - (m_ox % 12), m_ow - m_ox)))
			return true;

		switch (n.op)
		{
		case huffman_op::NEW_COLOUR:
			for (u32 i = 0; i < n.count; i++)
				set_o(v);
			break;
		case huffman_op::PREV_COLOUR:
		{
			const u8 c = m_colour[v];
			for (u32 i = 0; i < n.count; i++)
				set_o(c);
			break;
		}
		case huffman_op::COPY:
			copy(0, n.count);
			break;
		case huffman_op::COPY_PLUS_ONE:
			copy(1, n.count);
			break;
		case huffman_op::COPY_MINUS_ONE:
			copy(-1, n.count);
			break;
		}

		bool eol;
		if (!(m_ox % 12))
		{
			m_ox -= 12;
			eol = true;
		}
		else if (m_ox == m_ow)
		{
			m_ox -= m_ow % 12;
			eol = true;
		}
		else
		{
			eol = false;
		}

		if (eol)
		{
			if ((m_ox / 12) & 1)
				m_oy -= 1;
			else
				m_oy += 1;

			if ((m_oy == m_oh) || (m_oy == -1))
			{
				m_ox += 12;
				if ((m_ox / 12) & 1)
					m_oy -= 1;
				else
					m_oy += 1;
			}
		}
	}

	return false;
}
