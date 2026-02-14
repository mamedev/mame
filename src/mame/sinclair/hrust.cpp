// license:BSD-3-Clause
// copyright-holders:HalfElf, Andrei I. Holub

#include "hrust.h"


bb_stream::bb_stream(const u8 *src, int src_length)
{
	m_base = m_read = src;

	m_length = src_length;
	m_offset = 0;
	m_eof = false;

	m_buffer = get_byte();
	m_buffer += get_byte() << 8;
}

u8 bb_stream::get_byte()
{
	m_eof |= ((m_read - m_base) == m_length);
	return m_eof ? 0 : *m_read++;
}

u8 bb_stream::get_bit()
{
	u8 bit = BIT(m_buffer, 15 - m_offset);
	if (m_offset == 15)
	{
		m_buffer = get_byte();
		m_buffer += get_byte() << 8;
	}
	m_offset = (m_offset + 1) & 0xf;

	return bit;
}

u8 bb_stream::get_bits(u8 n)
{
	u8 r = 0;
	do
	{
		r = (r << 1) + get_bit();
	} while (--n);

	return r;
}

bool bb_stream::overflow()
{
	return m_eof;
}


hrust_decoder::hrust_decoder()
{
}

void hrust_decoder::decode(address_space &space, const u8 *source, u16 dest, u16 size)
{
	constexpr u8 mask[] = { 0, 0, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0 };
	u8 no_bits = 2;
	bb_stream bitbuf(source, size);

	space.write_byte(dest++, bitbuf.get_byte());

	while (!bitbuf.overflow())
	{
		while (bitbuf.get_bit())
		{
			space.write_byte(dest++, bitbuf.get_byte());
		}

		u16 len = 0;
		u8 bb;
		do
		{
			bb = bitbuf.get_bits(2);
			len += bb;

		} while (bb == 0x03 && len != 0x0f);

		s16 offset = 0;
		if (len == 0)
		{
			offset = 0xfff8 + bitbuf.get_bits(3);
			space.write_byte(dest, space.read_byte(dest + offset));
			++dest;
			continue;
		}
		else if (len == 1)
		{
			const u8 code = bitbuf.get_bits(2);
			if (code == 0 || code == 1)
			{
				offset = bitbuf.get_byte();
				offset += (code ? 0xfe : 0xfd) << 8;
			}
			else if (code == 2)
			{
				u8 b = bitbuf.get_byte();
				if (b >= 0xe0)
				{
					b <<= 1;
					++b;	// rlca
					b ^= 2; // xor c

					if (b == 0xff)
					{
						++no_bits;
						continue;
					}

					offset = 0xff00 + b - 0x0f;
					space.write_byte(dest, space.read_byte(dest + offset));
					++dest;
					space.write_byte(dest++, bitbuf.get_byte());
					space.write_byte(dest, space.read_byte(dest + offset));
					++dest;
					continue;
				}
				offset = 0xff00 + b;
			}
			else if (code == 3)
			{
				offset = 0xffe0 + bitbuf.get_bits(5);
			}

			for (auto i = 0; i < 2; ++i)
			{
				space.write_byte(dest, space.read_byte(dest + offset));
				++dest;
			}
			continue;
		}
		else if (len == 3)
		{
			if (bitbuf.get_bit())
			{
				offset = 0xfff0 + bitbuf.get_bits(4);
				space.write_byte(dest, space.read_byte(dest + offset));
				++dest;
				space.write_byte(dest++, bitbuf.get_byte());
				space.write_byte(dest, space.read_byte(dest + offset));
				++dest;
				continue;
			}

			if (bitbuf.get_bit())
			{
				u8 bytes_no = 6 + bitbuf.get_bits(4);
				for (u8 i = 0; i < (bytes_no << 1); ++i)
					space.write_byte(dest++, bitbuf.get_byte());
				continue;
			}

			len = bitbuf.get_bits(7);
			if (len == 0x0f)
			{
				break; // EOF
			}
			if (len < 0x0f)
			{
				len = (len << 8) + bitbuf.get_byte();
			}
		}

		if (len == 2)
		{
			++len;
		}


		const u8 code = bitbuf.get_bits(2);
		if (code == 0)
		{
			offset = 0xfe00 + bitbuf.get_byte();
		}
		else if (code == 1)
		{
			u8 b = bitbuf.get_byte();

			if (b >= 0xe0)
			{
				if (len > 3)
				{
					// logerror
					break;
				}

				b <<= 1;
				++b;	// rlca
				b ^= 3; // xor c

				offset = 0xff00 + b - 0x0f;

				space.write_byte(dest, space.read_byte(dest + offset));
				++dest;
				space.write_byte(dest++, bitbuf.get_byte());
				space.write_byte(dest, space.read_byte(dest + offset));
				++dest;
				continue;
			}
			offset = 0xff00 + b;
		}
		else if (code == 2)
		{
			offset = 0xffe0 + bitbuf.get_bits(5);
		}
		else if (code == 3)
		{
			offset = (mask[no_bits] + bitbuf.get_bits(no_bits)) << 8;
			offset += bitbuf.get_byte();
		}

		for (u16 i = 0; i < len; ++i)
		{
			space.write_byte(dest, space.read_byte(dest + offset));
			++dest;
		}
	}
}
