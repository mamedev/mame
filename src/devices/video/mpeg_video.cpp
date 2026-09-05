// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    ISO/IEC 11172-2 MPEG-1 video support.

    This is a clean-room implementation written directly from the syntax,
    semantics and decoding process specified by the following standards.

    References:

      ISO/IEC 11172-2:1993
        Information technology -- Coding of moving pictures and associated
        audio for digital storage media at up to about 1,5 Mbit/s --
        Part 2: Video

      ISO/IEC 11172-2:1993/Cor 1:1996
      ISO/IEC 11172-2:1993/Cor 2:1999
      ISO/IEC 11172-2:1993/Cor 3:2003
      ISO/IEC 11172-2:1993/Cor 4:2006

    The implementation primarily follows clause 2.4 and normative Annexes
    A and B.  Technical Corrigendum 2 supplies the corrected intra
    quantizer-matrix rule, and Technical Corrigendum 4 updates the IDCT
    conformance reference.

***************************************************************************/

#include "emu.h"
#include "mpeg_video.h"

#include <cmath>
#include <numbers>

struct vlc_entry
{
	u32 code;
	u8 bits;
	int value;
};

struct dct_vlc_entry
{
	u32 code;
	u8 bits;
	u8 run;
	u8 level;
};

template <typename T, std::size_t N>
constexpr T make_vlc(const char (&text)[N], int value)
{
	u32 code = 0;
	for (unsigned i = 0; i != (N - 1); i++)
		code = (code << 1) | (text[i] == '1');
	return T{ code, u8(N - 1), value };
}

template <std::size_t N>
constexpr dct_vlc_entry make_dct_vlc(const char (&text)[N], unsigned run, unsigned level)
{
	u32 code = 0;
	for (unsigned i = 0; i != (N - 1); i++)
		code = (code << 1) | (text[i] == '1');
	return dct_vlc_entry{ code, u8(N - 1), u8(run), u8(level) };
}

constexpr vlc_entry s_macroblock_address_increment[] =
{
	make_vlc<vlc_entry>("1", 1),
	make_vlc<vlc_entry>("011", 2),
	make_vlc<vlc_entry>("010", 3),
	make_vlc<vlc_entry>("0011", 4),
	make_vlc<vlc_entry>("0010", 5),
	make_vlc<vlc_entry>("00011", 6),
	make_vlc<vlc_entry>("00010", 7),
	make_vlc<vlc_entry>("0000111", 8),
	make_vlc<vlc_entry>("0000110", 9),
	make_vlc<vlc_entry>("00001011", 10),
	make_vlc<vlc_entry>("00001010", 11),
	make_vlc<vlc_entry>("00001001", 12),
	make_vlc<vlc_entry>("00001000", 13),
	make_vlc<vlc_entry>("00000111", 14),
	make_vlc<vlc_entry>("00000110", 15),
	make_vlc<vlc_entry>("0000010111", 16),
	make_vlc<vlc_entry>("0000010110", 17),
	make_vlc<vlc_entry>("0000010101", 18),
	make_vlc<vlc_entry>("0000010100", 19),
	make_vlc<vlc_entry>("0000010011", 20),
	make_vlc<vlc_entry>("0000010010", 21),
	make_vlc<vlc_entry>("00000100011", 22),
	make_vlc<vlc_entry>("00000100010", 23),
	make_vlc<vlc_entry>("00000100001", 24),
	make_vlc<vlc_entry>("00000100000", 25),
	make_vlc<vlc_entry>("00000011111", 26),
	make_vlc<vlc_entry>("00000011110", 27),
	make_vlc<vlc_entry>("00000011101", 28),
	make_vlc<vlc_entry>("00000011100", 29),
	make_vlc<vlc_entry>("00000011011", 30),
	make_vlc<vlc_entry>("00000011010", 31),
	make_vlc<vlc_entry>("00000011001", 32),
	make_vlc<vlc_entry>("00000011000", 33)
};

constexpr vlc_entry s_coded_block_pattern[] =
{
	make_vlc<vlc_entry>("111", 60),
	make_vlc<vlc_entry>("1101", 4), make_vlc<vlc_entry>("1100", 8),
	make_vlc<vlc_entry>("1011", 16), make_vlc<vlc_entry>("1010", 32),
	make_vlc<vlc_entry>("10011", 12), make_vlc<vlc_entry>("10010", 48),
	make_vlc<vlc_entry>("10001", 20), make_vlc<vlc_entry>("10000", 40),
	make_vlc<vlc_entry>("01111", 28), make_vlc<vlc_entry>("01110", 44),
	make_vlc<vlc_entry>("01101", 52), make_vlc<vlc_entry>("01100", 56),
	make_vlc<vlc_entry>("01011", 1), make_vlc<vlc_entry>("01010", 61),
	make_vlc<vlc_entry>("01001", 2), make_vlc<vlc_entry>("01000", 62),
	make_vlc<vlc_entry>("001111", 24), make_vlc<vlc_entry>("001110", 36),
	make_vlc<vlc_entry>("001101", 3), make_vlc<vlc_entry>("001100", 63),
	make_vlc<vlc_entry>("0010111", 5), make_vlc<vlc_entry>("0010110", 9),
	make_vlc<vlc_entry>("0010101", 17), make_vlc<vlc_entry>("0010100", 33),
	make_vlc<vlc_entry>("0010011", 6), make_vlc<vlc_entry>("0010010", 10),
	make_vlc<vlc_entry>("0010001", 18), make_vlc<vlc_entry>("0010000", 34),
	make_vlc<vlc_entry>("00011111", 7), make_vlc<vlc_entry>("00011110", 11),
	make_vlc<vlc_entry>("00011101", 19), make_vlc<vlc_entry>("00011100", 35),
	make_vlc<vlc_entry>("00011011", 13), make_vlc<vlc_entry>("00011010", 49),
	make_vlc<vlc_entry>("00011001", 21), make_vlc<vlc_entry>("00011000", 41),
	make_vlc<vlc_entry>("00010111", 14), make_vlc<vlc_entry>("00010110", 50),
	make_vlc<vlc_entry>("00010101", 22), make_vlc<vlc_entry>("00010100", 42),
	make_vlc<vlc_entry>("00010011", 15), make_vlc<vlc_entry>("00010010", 51),
	make_vlc<vlc_entry>("00010001", 23), make_vlc<vlc_entry>("00010000", 43),
	make_vlc<vlc_entry>("00001111", 25), make_vlc<vlc_entry>("00001110", 37),
	make_vlc<vlc_entry>("00001101", 26), make_vlc<vlc_entry>("00001100", 38),
	make_vlc<vlc_entry>("00001011", 29), make_vlc<vlc_entry>("00001010", 45),
	make_vlc<vlc_entry>("00001001", 53), make_vlc<vlc_entry>("00001000", 57),
	make_vlc<vlc_entry>("00000111", 30), make_vlc<vlc_entry>("00000110", 46),
	make_vlc<vlc_entry>("00000101", 54), make_vlc<vlc_entry>("00000100", 58),
	make_vlc<vlc_entry>("000000111", 31), make_vlc<vlc_entry>("000000110", 47),
	make_vlc<vlc_entry>("000000101", 55), make_vlc<vlc_entry>("000000100", 59),
	make_vlc<vlc_entry>("000000011", 27), make_vlc<vlc_entry>("000000010", 39)
};

constexpr vlc_entry s_motion_code[] =
{
	make_vlc<vlc_entry>("1", 0),
	make_vlc<vlc_entry>("011", -1), make_vlc<vlc_entry>("010", 1),
	make_vlc<vlc_entry>("0011", -2), make_vlc<vlc_entry>("0010", 2),
	make_vlc<vlc_entry>("00011", -3), make_vlc<vlc_entry>("00010", 3),
	make_vlc<vlc_entry>("0000111", -4), make_vlc<vlc_entry>("0000110", 4),
	make_vlc<vlc_entry>("00001011", -5), make_vlc<vlc_entry>("00001010", 5),
	make_vlc<vlc_entry>("00001001", -6), make_vlc<vlc_entry>("00001000", 6),
	make_vlc<vlc_entry>("00000111", -7), make_vlc<vlc_entry>("00000110", 7),
	make_vlc<vlc_entry>("0000010111", -8), make_vlc<vlc_entry>("0000010110", 8),
	make_vlc<vlc_entry>("0000010101", -9), make_vlc<vlc_entry>("0000010100", 9),
	make_vlc<vlc_entry>("0000010011", -10), make_vlc<vlc_entry>("0000010010", 10),
	make_vlc<vlc_entry>("00000100011", -11), make_vlc<vlc_entry>("00000100010", 11),
	make_vlc<vlc_entry>("00000100001", -12), make_vlc<vlc_entry>("00000100000", 12),
	make_vlc<vlc_entry>("00000011111", -13), make_vlc<vlc_entry>("00000011110", 13),
	make_vlc<vlc_entry>("00000011101", -14), make_vlc<vlc_entry>("00000011100", 14),
	make_vlc<vlc_entry>("00000011011", -15), make_vlc<vlc_entry>("00000011010", 15),
	make_vlc<vlc_entry>("00000011001", -16), make_vlc<vlc_entry>("00000011000", 16)
};

constexpr dct_vlc_entry s_dct_coefficient[] =
{
	make_dct_vlc("011", 1, 1), make_dct_vlc("0100", 0, 2), make_dct_vlc("0101", 2, 1),
	make_dct_vlc("00101", 0, 3), make_dct_vlc("00111", 3, 1), make_dct_vlc("00110", 4, 1),
	make_dct_vlc("000110", 1, 2), make_dct_vlc("000111", 5, 1),
	make_dct_vlc("000101", 6, 1), make_dct_vlc("000100", 7, 1),
	make_dct_vlc("0000110", 0, 4), make_dct_vlc("0000100", 2, 2),
	make_dct_vlc("0000111", 8, 1), make_dct_vlc("0000101", 9, 1),
	make_dct_vlc("00100110", 0, 5), make_dct_vlc("00100001", 0, 6),
	make_dct_vlc("00100101", 1, 3), make_dct_vlc("00100100", 3, 2),
	make_dct_vlc("00100111", 10, 1), make_dct_vlc("00100011", 11, 1),
	make_dct_vlc("00100010", 12, 1), make_dct_vlc("00100000", 13, 1),
	make_dct_vlc("0000001010", 0, 7), make_dct_vlc("0000001100", 1, 4),
	make_dct_vlc("0000001011", 2, 3), make_dct_vlc("0000001111", 4, 2),
	make_dct_vlc("0000001001", 5, 2), make_dct_vlc("0000001110", 14, 1),
	make_dct_vlc("0000001101", 15, 1), make_dct_vlc("0000001000", 16, 1),
	make_dct_vlc("000000011101", 0, 8), make_dct_vlc("000000011000", 0, 9),
	make_dct_vlc("000000010011", 0, 10), make_dct_vlc("000000010000", 0, 11),
	make_dct_vlc("000000011011", 1, 5), make_dct_vlc("000000010100", 2, 4),
	make_dct_vlc("000000011100", 3, 3), make_dct_vlc("000000010010", 4, 3),
	make_dct_vlc("000000011110", 6, 2), make_dct_vlc("000000010101", 7, 2),
	make_dct_vlc("000000010001", 8, 2), make_dct_vlc("000000011111", 17, 1),
	make_dct_vlc("000000011010", 18, 1), make_dct_vlc("000000011001", 19, 1),
	make_dct_vlc("000000010111", 20, 1), make_dct_vlc("000000010110", 21, 1),
	make_dct_vlc("0000000011010", 0, 12), make_dct_vlc("0000000011001", 0, 13),
	make_dct_vlc("0000000011000", 0, 14), make_dct_vlc("0000000010111", 0, 15),
	make_dct_vlc("0000000010110", 1, 6), make_dct_vlc("0000000010101", 1, 7),
	make_dct_vlc("0000000010100", 2, 5), make_dct_vlc("0000000010011", 3, 4),
	make_dct_vlc("0000000010010", 5, 3), make_dct_vlc("0000000010001", 9, 2),
	make_dct_vlc("0000000010000", 10, 2), make_dct_vlc("0000000011111", 22, 1),
	make_dct_vlc("0000000011110", 23, 1), make_dct_vlc("0000000011101", 24, 1),
	make_dct_vlc("0000000011100", 25, 1), make_dct_vlc("0000000011011", 26, 1),
	make_dct_vlc("00000000011111", 0, 16), make_dct_vlc("00000000011110", 0, 17),
	make_dct_vlc("00000000011101", 0, 18), make_dct_vlc("00000000011100", 0, 19),
	make_dct_vlc("00000000011011", 0, 20), make_dct_vlc("00000000011010", 0, 21),
	make_dct_vlc("00000000011001", 0, 22), make_dct_vlc("00000000011000", 0, 23),
	make_dct_vlc("00000000010111", 0, 24), make_dct_vlc("00000000010110", 0, 25),
	make_dct_vlc("00000000010101", 0, 26), make_dct_vlc("00000000010100", 0, 27),
	make_dct_vlc("00000000010011", 0, 28), make_dct_vlc("00000000010010", 0, 29),
	make_dct_vlc("00000000010001", 0, 30), make_dct_vlc("00000000010000", 0, 31),
	make_dct_vlc("000000000011000", 0, 32), make_dct_vlc("000000000010111", 0, 33),
	make_dct_vlc("000000000010110", 0, 34), make_dct_vlc("000000000010101", 0, 35),
	make_dct_vlc("000000000010100", 0, 36), make_dct_vlc("000000000010011", 0, 37),
	make_dct_vlc("000000000010010", 0, 38), make_dct_vlc("000000000010001", 0, 39),
	make_dct_vlc("000000000010000", 0, 40), make_dct_vlc("000000000011111", 1, 8),
	make_dct_vlc("000000000011110", 1, 9), make_dct_vlc("000000000011101", 1, 10),
	make_dct_vlc("000000000011100", 1, 11), make_dct_vlc("000000000011011", 1, 12),
	make_dct_vlc("000000000011010", 1, 13), make_dct_vlc("000000000011001", 1, 14),
	make_dct_vlc("0000000000010011", 1, 15), make_dct_vlc("0000000000010010", 1, 16),
	make_dct_vlc("0000000000010001", 1, 17), make_dct_vlc("0000000000010000", 1, 18),
	make_dct_vlc("0000000000010100", 6, 3), make_dct_vlc("0000000000011010", 11, 2),
	make_dct_vlc("0000000000011001", 12, 2), make_dct_vlc("0000000000011000", 13, 2),
	make_dct_vlc("0000000000010111", 14, 2), make_dct_vlc("0000000000010110", 15, 2),
	make_dct_vlc("0000000000010101", 16, 2), make_dct_vlc("0000000000011111", 27, 1),
	make_dct_vlc("0000000000011110", 28, 1), make_dct_vlc("0000000000011101", 29, 1),
	make_dct_vlc("0000000000011100", 30, 1), make_dct_vlc("0000000000011011", 31, 1)
};

constexpr u8 TYPE_QUANT = 0x01;
constexpr u8 TYPE_FORWARD = 0x02;
constexpr u8 TYPE_BACKWARD = 0x04;
constexpr u8 TYPE_PATTERN = 0x08;
constexpr u8 TYPE_INTRA = 0x10;

constexpr vlc_entry s_i_macroblock_type[] =
{
	make_vlc<vlc_entry>("1", TYPE_INTRA),
	make_vlc<vlc_entry>("01", TYPE_QUANT | TYPE_INTRA)
};

constexpr vlc_entry s_p_macroblock_type[] =
{
	make_vlc<vlc_entry>("1", TYPE_FORWARD | TYPE_PATTERN),
	make_vlc<vlc_entry>("01", TYPE_PATTERN),
	make_vlc<vlc_entry>("001", TYPE_FORWARD),
	make_vlc<vlc_entry>("00011", TYPE_INTRA),
	make_vlc<vlc_entry>("00010", TYPE_QUANT | TYPE_FORWARD | TYPE_PATTERN),
	make_vlc<vlc_entry>("00001", TYPE_QUANT | TYPE_PATTERN),
	make_vlc<vlc_entry>("000001", TYPE_QUANT | TYPE_INTRA)
};

constexpr vlc_entry s_b_macroblock_type[] =
{
	make_vlc<vlc_entry>("10", TYPE_FORWARD | TYPE_BACKWARD),
	make_vlc<vlc_entry>("11", TYPE_FORWARD | TYPE_BACKWARD | TYPE_PATTERN),
	make_vlc<vlc_entry>("010", TYPE_BACKWARD),
	make_vlc<vlc_entry>("011", TYPE_BACKWARD | TYPE_PATTERN),
	make_vlc<vlc_entry>("0010", TYPE_FORWARD),
	make_vlc<vlc_entry>("0011", TYPE_FORWARD | TYPE_PATTERN),
	make_vlc<vlc_entry>("00011", TYPE_INTRA),
	make_vlc<vlc_entry>("00010", TYPE_QUANT | TYPE_FORWARD | TYPE_BACKWARD | TYPE_PATTERN),
	make_vlc<vlc_entry>("000011", TYPE_QUANT | TYPE_FORWARD | TYPE_PATTERN),
	make_vlc<vlc_entry>("000010", TYPE_QUANT | TYPE_BACKWARD | TYPE_PATTERN),
	make_vlc<vlc_entry>("000001", TYPE_QUANT | TYPE_INTRA)
};

constexpr vlc_entry s_d_macroblock_type[] =
{
	make_vlc<vlc_entry>("1", TYPE_INTRA)
};

constexpr vlc_entry s_dc_size_luminance[] =
{
	make_vlc<vlc_entry>("100", 0), make_vlc<vlc_entry>("00", 1),
	make_vlc<vlc_entry>("01", 2), make_vlc<vlc_entry>("101", 3),
	make_vlc<vlc_entry>("110", 4), make_vlc<vlc_entry>("1110", 5),
	make_vlc<vlc_entry>("11110", 6), make_vlc<vlc_entry>("111110", 7),
	make_vlc<vlc_entry>("1111110", 8)
};

constexpr vlc_entry s_dc_size_chrominance[] =
{
	make_vlc<vlc_entry>("00", 0), make_vlc<vlc_entry>("01", 1),
	make_vlc<vlc_entry>("10", 2), make_vlc<vlc_entry>("110", 3),
	make_vlc<vlc_entry>("1110", 4), make_vlc<vlc_entry>("11110", 5),
	make_vlc<vlc_entry>("111110", 6), make_vlc<vlc_entry>("1111110", 7),
	make_vlc<vlc_entry>("11111110", 8)
};

template <typename T, std::size_t N, unsigned MaxBits>
class vlc_decoder
{
public:
	struct match
	{
		s16 entry;
		u8 bits;
	};

	constexpr vlc_decoder(const T (&table)[N]) :
		m_nodes{}
	{
		for (node &item : m_nodes)
			item = node{ { -1, -1 }, -1 };

		s16 node_count = 1;
		for (unsigned entry = 0; entry != N; entry++)
		{
			s16 current = 0;
			for (unsigned bit = table[entry].bits; bit; bit--)
			{
				s16 &next = m_nodes[current].child[BIT(table[entry].code, bit - 1)];
				if (next < 0)
				{
					next = node_count++;
					m_nodes[next] = node{ { -1, -1 }, -1 };
				}
				current = next;
			}
			m_nodes[current].entry = entry;
		}
	}

	match lookup(u32 code, unsigned bits) const
	{
		s16 current = 0;
		for (unsigned consumed = 0; consumed != bits; consumed++)
		{
			current = m_nodes[current].child[BIT(code, bits - consumed - 1)];
			if (current < 0)
				break;
			if (m_nodes[current].entry >= 0)
				return match{ m_nodes[current].entry, u8(consumed + 1) };
		}
		return match{ -1, 0 };
	}

	static constexpr unsigned max_bits() { return MaxBits; }

private:
	struct node
	{
		s16 child[2];
		s16 entry;
	};

	node m_nodes[N * MaxBits + 1];
};

template <unsigned MaxBits, typename T, std::size_t N>
constexpr auto make_vlc_decoder(const T (&table)[N])
{
	return vlc_decoder<T, N, MaxBits>(table);
}

constexpr auto s_macroblock_address_increment_decoder = make_vlc_decoder<11>(s_macroblock_address_increment);
constexpr auto s_coded_block_pattern_decoder = make_vlc_decoder<9>(s_coded_block_pattern);
constexpr auto s_motion_code_decoder = make_vlc_decoder<11>(s_motion_code);
constexpr auto s_dct_coefficient_decoder = make_vlc_decoder<16>(s_dct_coefficient);
constexpr auto s_i_macroblock_type_decoder = make_vlc_decoder<2>(s_i_macroblock_type);
constexpr auto s_p_macroblock_type_decoder = make_vlc_decoder<6>(s_p_macroblock_type);
constexpr auto s_b_macroblock_type_decoder = make_vlc_decoder<6>(s_b_macroblock_type);
constexpr auto s_d_macroblock_type_decoder = make_vlc_decoder<1>(s_d_macroblock_type);
constexpr auto s_dc_size_luminance_decoder = make_vlc_decoder<7>(s_dc_size_luminance);
constexpr auto s_dc_size_chrominance_decoder = make_vlc_decoder<8>(s_dc_size_chrominance);

template <typename T, std::size_t N, unsigned MaxBits, typename P, typename S>
const T *decode_vlc(const T (&table)[N], const vlc_decoder<T, N, MaxBits> &decoder,
		int available, P &&peek, S &&skip)
{
	const unsigned lookahead = std::min<unsigned>(decoder.max_bits(), std::max(available, 0));
	const auto match = decoder.lookup(peek(lookahead), lookahead);
	if (match.entry >= 0)
	{
		skip(match.bits);
		return &table[match.entry];
	}

	// Preserve streaming semantics: a truncated prefix needs more data rather than being invalid.
	if (lookahead < decoder.max_bits())
		peek(lookahead + 1);
	return nullptr;
}

const u8 mpeg_video::s_default_intra_quantizer_matrix[64] =
{
	8, 16, 19, 22, 26, 27, 29, 34,
	16, 16, 22, 24, 27, 29, 34, 37,
	19, 22, 26, 27, 29, 34, 34, 38,
	22, 22, 26, 27, 29, 34, 37, 40,
	22, 26, 27, 29, 32, 35, 40, 48,
	26, 27, 29, 32, 35, 40, 48, 58,
	26, 27, 29, 34, 38, 46, 56, 69,
	27, 29, 35, 38, 46, 56, 69, 83
};

// ISO/IEC 11172-2 Table 2.4.4.1, indexed by natural coefficient position.
const u8 mpeg_video::s_scan[64] =
{
	0, 1, 5, 6, 14, 15, 27, 28,
	2, 4, 7, 13, 16, 26, 29, 42,
	3, 8, 12, 17, 25, 30, 41, 43,
	9, 11, 18, 24, 31, 40, 44, 53,
	10, 19, 23, 32, 39, 45, 52, 54,
	20, 22, 33, 38, 46, 51, 55, 60,
	21, 34, 37, 47, 50, 56, 59, 61,
	35, 36, 48, 49, 57, 58, 62, 63
};

const double mpeg_video::s_picture_rates[16] =
{
	0.0, 24'000.0 / 1'001.0, 24.0, 25.0,
	30'000.0 / 1'001.0, 30.0, 50.0, 60'000.0 / 1'001.0,
	60.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
};

mpeg_video::mpeg_video(const void *base, int maximum_width, int maximum_height) :
	m_base(reinterpret_cast<const u8 *>(base)),
	m_maximum_width(maximum_width),
	m_maximum_height(maximum_height)
{
	assert(m_maximum_width > 0);
	assert(m_maximum_height > 0);

	const unsigned maximum_mb_width = (m_maximum_width + 15) / 16;
	const unsigned maximum_mb_height = (m_maximum_height + 15) / 16;
	const unsigned luma_size = maximum_mb_width * 16 * maximum_mb_height * 16;
	const unsigned chroma_size = maximum_mb_width * 8 * maximum_mb_height * 8;
	for (frame *const item : { &m_current_frame, &m_forward_reference, &m_backward_reference })
	{
		item->y.resize(luma_size);
		item->cb.resize(chroma_size);
		item->cr.resize(chroma_size);
	}

	for (int x = 0; x != 8; x++)
	{
		for (int u = 0; u != 8; u++)
		{
			// Fold the IDCT's quarter scaling and C(u) normalization into its two passes.
			const double scale = u ? 0.5 : (0.5 / std::numbers::sqrt2);
			m_idct_basis[x][u] = scale * std::cos((2 * x + 1) * u * std::numbers::pi / 16.0);
		}
	}

	clear();
}

void mpeg_video::clear()
{
	m_current_pos = 0;
	m_current_limit = 0;
	m_horizontal_size = 0;
	m_vertical_size = 0;
	m_mb_width = 0;
	m_mb_height = 0;
	m_luma_pitch = 0;
	m_chroma_pitch = 0;
	m_frame_rate = 0.0;
	std::copy(
			std::begin(s_default_intra_quantizer_matrix),
			std::end(s_default_intra_quantizer_matrix),
			m_intra_quantizer_matrix);
	std::fill(std::begin(m_non_intra_quantizer_matrix), std::end(m_non_intra_quantizer_matrix), 16);
	m_picture_coding_type = 0;
	m_full_pel_forward_vector = false;
	m_full_pel_backward_vector = false;
	m_forward_f = 0;
	m_backward_f = 0;
	m_quantizer_scale = 0;
	m_macroblock_address = -1;
	reset_dc_predictors();
	m_forward_horizontal_previous = 0;
	m_forward_vertical_previous = 0;
	m_backward_horizontal_previous = 0;
	m_backward_vertical_previous = 0;
	m_previous_b_forward = false;
	m_previous_b_backward = false;
}

void mpeg_video::register_save_state(device_t &device, int index)
{
	device.save_item(m_horizontal_size, "mpeg_video_horizontal_size", index);
	device.save_item(m_vertical_size, "mpeg_video_vertical_size", index);
	device.save_item(m_mb_width, "mpeg_video_mb_width", index);
	device.save_item(m_mb_height, "mpeg_video_mb_height", index);
	device.save_item(m_luma_pitch, "mpeg_video_luma_pitch", index);
	device.save_item(m_chroma_pitch, "mpeg_video_chroma_pitch", index);
	device.save_item(m_frame_rate, "mpeg_video_frame_rate", index);
	device.save_item(m_intra_quantizer_matrix, "mpeg_video_intra_quantizer_matrix", index);
	device.save_item(m_non_intra_quantizer_matrix, "mpeg_video_non_intra_quantizer_matrix", index);
}

mpeg_video::decode_result mpeg_video::decode_buffer(int &pos, int limit, const picture_buffers &buffers,
		int &width, int &height, double &frame_rate)
{
	if ((limit - pos) < 32)
		return decode_result::NEED_DATA;

	int scan_position = pos;
	for (;;)
	{
		m_current_pos = scan_position;
		m_current_limit = limit;
		int syntax_position = scan_position;

		try
		{
			next_start_code();
			syntax_position = m_current_pos;
			const u32 code = peek(32);

			switch (code)
			{
			case SEQUENCE_HEADER_CODE:
				sequence_header();
				break;

			case GROUP_START_CODE:
				group_of_pictures();
				break;

			case PICTURE_START_CODE:
				picture(buffers);
				// A sequence-end code terminates the preceding coded sequence rather than
				// describing another decoding task.
				if (peek(32) == SEQUENCE_END_CODE)
					gb(32);
				pos = m_current_pos;
				width = m_horizontal_size;
				height = m_vertical_size;
				frame_rate = m_frame_rate;
				return decode_result::PICTURE;

			case SEQUENCE_END_CODE:
				gb(32);
				pos = m_current_pos;
				width = m_horizontal_size;
				height = m_vertical_size;
				frame_rate = m_frame_rate;
				return decode_result::SEQUENCE_END;

			default:
				// Extension, user and system data are delimited by the next start code.
				gb(32);
				break;
			}

			scan_position = m_current_pos;
		}
		catch (limit_hit const &)
		{
			pos = syntax_position;
			return decode_result::NEED_DATA;
		}
		catch (invalid_stream const &)
		{
			// Skip the failed syntax element and resume at the next start code.
			int recovery_position = std::max(syntax_position + 8, (m_current_pos + 7) & ~7);
			while ((recovery_position + 24) <= limit)
			{
				const unsigned byte_position = recovery_position / 8;
				if (!m_base[byte_position] && !m_base[byte_position + 1] && (m_base[byte_position + 2] == 1))
					break;
				recovery_position += 8;
			}
			pos = std::min(recovery_position, limit);
			return decode_result::INVALID_DATA;
		}
	}
}

void mpeg_video::sequence_header()
{
	gb(32);
	const int horizontal_size = gb(12);
	const int vertical_size = gb(12);
	gb(4); // pel aspect ratio
	const unsigned picture_rate = gb(4);
	gb(18); // bit rate
	if (gb(1) != 1)
		throw invalid_stream();
	gb(10); // VBV buffer size
	gb(1); // constrained parameters flag

	u8 intra_quantizer_matrix[64];
	u8 non_intra_quantizer_matrix[64];
	std::copy(
			std::begin(s_default_intra_quantizer_matrix),
			std::end(s_default_intra_quantizer_matrix),
			intra_quantizer_matrix);
	if (gb(1))
	{
		for (unsigned zigzag = 0; zigzag != 64; zigzag++)
		{
			for (unsigned natural = 0; natural != 64; natural++)
			{
				if (s_scan[natural] == zigzag)
				{
					intra_quantizer_matrix[natural] = gb(8);
					break;
				}
			}
		}
		// ISO/IEC 11172-2 Technical Corrigendum 2 requires this value to be eight.
		intra_quantizer_matrix[0] = 8;
	}

	std::fill(std::begin(non_intra_quantizer_matrix), std::end(non_intra_quantizer_matrix), 16);
	if (gb(1))
	{
		for (unsigned zigzag = 0; zigzag != 64; zigzag++)
		{
			for (unsigned natural = 0; natural != 64; natural++)
			{
				if (s_scan[natural] == zigzag)
				{
					non_intra_quantizer_matrix[natural] = gb(8);
					break;
				}
			}
		}
	}

	if (!horizontal_size || !vertical_size ||
		(horizontal_size > m_maximum_width) || (vertical_size > m_maximum_height) ||
		!s_picture_rates[picture_rate])
		throw invalid_stream();

	next_start_code();
	while ((peek(32) == EXTENSION_START_CODE) || (peek(32) == USER_DATA_START_CODE))
	{
		gb(32);
		next_start_code();
	}

	m_horizontal_size = horizontal_size;
	m_vertical_size = vertical_size;
	m_frame_rate = s_picture_rates[picture_rate];
	m_mb_width = (m_horizontal_size + 15) / 16;
	m_mb_height = (m_vertical_size + 15) / 16;
	m_luma_pitch = m_mb_width * 16;
	m_chroma_pitch = m_mb_width * 8;
	std::copy(std::begin(intra_quantizer_matrix), std::end(intra_quantizer_matrix), m_intra_quantizer_matrix);
	std::copy(std::begin(non_intra_quantizer_matrix), std::end(non_intra_quantizer_matrix), m_non_intra_quantizer_matrix);
}

void mpeg_video::group_of_pictures()
{
	gb(32);
	gb(1); // drop frame flag
	const unsigned hours = gb(5);
	const unsigned minutes = gb(6);
	if (gb(1) != 1) // marker bit
		throw invalid_stream();
	const unsigned seconds = gb(6);
	const unsigned pictures = gb(6);
	if ((hours > 23) || (minutes > 59) || (seconds > 59) || (pictures > 59))
		throw invalid_stream();
	gb(1); // closed GOP
	gb(1); // broken link
	next_start_code();

	while ((peek(32) == EXTENSION_START_CODE) || (peek(32) == USER_DATA_START_CODE))
	{
		gb(32);
		next_start_code();
	}
}

void mpeg_video::picture(const picture_buffers &buffers)
{
	if (!m_horizontal_size || !m_vertical_size)
		throw invalid_stream();

	gb(32);
	gb(10); // temporal reference
	m_picture_coding_type = gb(3);
	gb(16); // VBV delay

	m_full_pel_forward_vector = false;
	m_full_pel_backward_vector = false;
	m_forward_f = 0;
	m_backward_f = 0;
	if ((m_picture_coding_type == 2) || (m_picture_coding_type == 3))
	{
		m_full_pel_forward_vector = gb(1);
		const int forward_f_code = gb(3);
		if (!forward_f_code)
			throw invalid_stream();
		m_forward_f = 1 << (forward_f_code - 1);
	}
	if (m_picture_coding_type == 3)
	{
		m_full_pel_backward_vector = gb(1);
		const int backward_f_code = gb(3);
		if (!backward_f_code)
			throw invalid_stream();
		m_backward_f = 1 << (backward_f_code - 1);
	}
	if ((m_picture_coding_type < 1) || (m_picture_coding_type > 4))
		throw invalid_stream();

	if ((m_picture_coding_type == 2) || (m_picture_coding_type == 3))
		read_frame(m_forward_reference, buffers.forward.data, buffers.forward.bytes);
	if (m_picture_coding_type == 3)
		read_frame(m_backward_reference, buffers.backward.data, buffers.backward.bytes);

	while (gb(1))
		gb(8);
	next_start_code();

	while ((peek(32) == EXTENSION_START_CODE) || (peek(32) == USER_DATA_START_CODE))
	{
		gb(32);
		next_start_code();
	}

	// Macroblocks not reconstructed by the task retain their previous contents in RFP.
	read_frame(m_current_frame, buffers.reconstructed.data, buffers.reconstructed.bytes);

	bool have_slice = false;
	while ((peek(32) >= 0x00000101) && (peek(32) <= 0x000001af))
	{
		const unsigned vertical_position = peek(32) & 0xff;
		slice(vertical_position);
		have_slice = true;
	}
	if (!have_slice)
		throw invalid_stream();

	write_frame(m_current_frame, buffers.reconstructed.data, buffers.reconstructed.bytes);
}

void mpeg_video::reset_dc_predictors()
{
	std::fill(std::begin(m_dc_predictor), std::end(m_dc_predictor), 128 * 8);
}

void mpeg_video::slice(unsigned vertical_position)
{
	gb(32);
	m_quantizer_scale = gb(5);
	if (!m_quantizer_scale)
		throw invalid_stream();
	while (gb(1))
		gb(8);

	m_macroblock_address = (vertical_position - 1) * m_mb_width - 1;
	reset_dc_predictors();
	m_forward_horizontal_previous = 0;
	m_forward_vertical_previous = 0;
	m_backward_horizontal_previous = 0;
	m_backward_vertical_previous = 0;
	m_previous_b_forward = false;
	m_previous_b_backward = false;

	bool first_in_slice = true;
	do
	{
		macroblock(first_in_slice);
		first_in_slice = false;
	}
	while (peek(23) != 0);

	next_start_code();
}

void mpeg_video::macroblock(bool first_in_slice)
{
	const int increment = macroblock_address_increment();
	const int address = m_macroblock_address + increment;
	if ((address < 0) || (address >= (m_mb_width * m_mb_height)))
		throw invalid_stream();

	if (!first_in_slice)
	{
		for (int skipped = m_macroblock_address + 1; skipped < address; skipped++)
			skipped_macroblock(skipped);
	}
	m_macroblock_address = address;

	const macroblock_type type = macroblock_type_code();
	if (type.quant)
	{
		m_quantizer_scale = gb(5);
		if (!m_quantizer_scale)
			throw invalid_stream();
	}

	motion_vector forward_vector{ 0, 0 };
	motion_vector backward_vector{ 0, 0 };

	if (m_picture_coding_type == 2)
	{
		if (type.forward)
			decode_motion_vector(true, forward_vector);
		else
		{
			m_forward_horizontal_previous = 0;
			m_forward_vertical_previous = 0;
		}
	}
	else if (m_picture_coding_type == 3)
	{
		if (type.forward)
			decode_motion_vector(true, forward_vector);
		else
		{
			forward_vector.horizontal = m_forward_horizontal_previous * (m_full_pel_forward_vector ? 2 : 1);
			forward_vector.vertical = m_forward_vertical_previous * (m_full_pel_forward_vector ? 2 : 1);
		}

		if (type.backward)
			decode_motion_vector(false, backward_vector);
		else
		{
			backward_vector.horizontal = m_backward_horizontal_previous * (m_full_pel_backward_vector ? 2 : 1);
			backward_vector.vertical = m_backward_vertical_previous * (m_full_pel_backward_vector ? 2 : 1);
		}
	}

	if (!type.intra)
	{
		reset_dc_predictors();
		const bool predict_forward = (m_picture_coding_type == 2) || type.forward;
		predict_macroblock(predict_forward, type.backward, forward_vector, backward_vector);
	}

	const int pattern = type.intra ? 0x3f : (type.pattern ? coded_block_pattern() : 0);
	for (unsigned index = 0; index != 6; index++)
	{
		if (BIT(pattern, 5 - index))
			block(index, type.intra);
	}

	if (m_picture_coding_type == 4)
	{
		if (gb(1) != 1)
			throw invalid_stream();
	}

	if (type.intra)
	{
		m_forward_horizontal_previous = 0;
		m_forward_vertical_previous = 0;
		m_backward_horizontal_previous = 0;
		m_backward_vertical_previous = 0;
		m_previous_b_forward = false;
		m_previous_b_backward = false;
	}
	else if (m_picture_coding_type == 3)
	{
		m_previous_b_forward = type.forward;
		m_previous_b_backward = type.backward;
	}
}

void mpeg_video::skipped_macroblock(int address)
{
	m_macroblock_address = address;
	reset_dc_predictors();

	if (m_picture_coding_type == 2)
	{
		m_forward_horizontal_previous = 0;
		m_forward_vertical_previous = 0;
		predict_macroblock(true, false, motion_vector{ 0, 0 }, motion_vector{ 0, 0 });
	}
	else if (m_picture_coding_type == 3)
	{
		if (!m_previous_b_forward && !m_previous_b_backward)
			throw invalid_stream();
		const motion_vector forward_vector
		{
			m_forward_horizontal_previous * (m_full_pel_forward_vector ? 2 : 1),
			m_forward_vertical_previous * (m_full_pel_forward_vector ? 2 : 1)
		};
		const motion_vector backward_vector
		{
			m_backward_horizontal_previous * (m_full_pel_backward_vector ? 2 : 1),
			m_backward_vertical_previous * (m_full_pel_backward_vector ? 2 : 1)
		};
		predict_macroblock(m_previous_b_forward, m_previous_b_backward, forward_vector, backward_vector);
	}
	else
	{
		throw invalid_stream();
	}
}

void mpeg_video::decode_motion_vector(bool forward, motion_vector &vector)
{
	const int f = forward ? m_forward_f : m_backward_f;
	const bool full_pel = forward ? m_full_pel_forward_vector : m_full_pel_backward_vector;
	int &horizontal_previous = forward ? m_forward_horizontal_previous : m_backward_horizontal_previous;
	int &vertical_previous = forward ? m_forward_vertical_previous : m_backward_vertical_previous;

	const int horizontal_code = motion_code();
	const int horizontal_residual = ((f != 1) && horizontal_code) ? int(gb(std::countr_zero(unsigned(f)))) : 0;
	const int vertical_code = motion_code();
	const int vertical_residual = ((f != 1) && vertical_code) ? int(gb(std::countr_zero(unsigned(f)))) : 0;

	vector.horizontal = decode_motion_component(horizontal_code, horizontal_residual, f, horizontal_previous, full_pel);
	vector.vertical = decode_motion_component(vertical_code, vertical_residual, f, vertical_previous, full_pel);
}

int mpeg_video::decode_motion_component(int code, int residual, int f, int &previous, bool full_pel)
{
	const int complement = ((f == 1) || !code) ? 0 : (f - 1 - residual);
	int little = code * f;
	int big = 0;
	if (little > 0)
	{
		little -= complement;
		big = little - 32 * f;
	}
	else if (little < 0)
	{
		little += complement;
		big = little + 32 * f;
	}

	const int maximum = 16 * f - 1;
	const int minimum = -16 * f;
	const int candidate = previous + little;
	previous += ((candidate >= minimum) && (candidate <= maximum)) ? little : big;
	return previous * (full_pel ? 2 : 1);
}

void mpeg_video::predict_macroblock(
		bool forward,
		bool backward,
		motion_vector forward_vector,
		motion_vector backward_vector)
{
	const int macroblock_x = (m_macroblock_address % m_mb_width) * 16;
	const int macroblock_y = (m_macroblock_address / m_mb_width) * 16;
	bool have_prediction = false;

	if (forward)
	{
		predict_plane(m_current_frame.y.data(), m_luma_pitch, m_forward_reference.y.data(), m_luma_pitch,
				macroblock_x, macroblock_y, 16, 16, forward_vector, false, false);
		predict_plane(m_current_frame.cb.data(), m_chroma_pitch, m_forward_reference.cb.data(), m_chroma_pitch,
				macroblock_x / 2, macroblock_y / 2, 8, 8, forward_vector, true, false);
		predict_plane(m_current_frame.cr.data(), m_chroma_pitch, m_forward_reference.cr.data(), m_chroma_pitch,
				macroblock_x / 2, macroblock_y / 2, 8, 8, forward_vector, true, false);
		have_prediction = true;
	}

	if (backward)
	{
		predict_plane(m_current_frame.y.data(), m_luma_pitch, m_backward_reference.y.data(), m_luma_pitch,
				macroblock_x, macroblock_y, 16, 16, backward_vector, false, have_prediction);
		predict_plane(m_current_frame.cb.data(), m_chroma_pitch, m_backward_reference.cb.data(), m_chroma_pitch,
				macroblock_x / 2, macroblock_y / 2, 8, 8, backward_vector, true, have_prediction);
		predict_plane(m_current_frame.cr.data(), m_chroma_pitch, m_backward_reference.cr.data(), m_chroma_pitch,
				macroblock_x / 2, macroblock_y / 2, 8, 8, backward_vector, true, have_prediction);
	}
}

void mpeg_video::predict_plane(u8 *destination, int destination_pitch, const u8 *reference, int reference_pitch,
		int x, int y, int width, int height, motion_vector vector, bool chroma, bool average) const
{
	auto split_half_pel = [] (int value, int &whole, int &half)
	{
		whole = value / 2;
		if ((value < 0) && (value % 2))
			whole--;
		half = value - 2 * whole;
	};

	const int horizontal_vector = chroma ? (vector.horizontal / 2) : vector.horizontal;
	const int vertical_vector = chroma ? (vector.vertical / 2) : vector.vertical;
	int horizontal_whole, horizontal_half;
	int vertical_whole, vertical_half;
	split_half_pel(horizontal_vector, horizontal_whole, horizontal_half);
	split_half_pel(vertical_vector, vertical_whole, vertical_half);
	const int source_x = x + horizontal_whole;
	const int source_y = y + vertical_whole;
	const int reference_height = m_mb_height * (chroma ? 8 : 16);
	if ((source_x < 0) || (source_y < 0) ||
		((source_x + width + horizontal_half) > reference_pitch) ||
		((source_y + height + vertical_half) > reference_height))
	{
		throw invalid_stream();
	}

	for (int row = 0; row != height; row++)
	{
		for (int column = 0; column != width; column++)
		{
			const int reference_x = source_x + column;
			const int reference_y = source_y + row;
			int prediction = reference[reference_y * reference_pitch + reference_x];
			if (horizontal_half)
				prediction += reference[reference_y * reference_pitch + reference_x + 1];
			if (vertical_half)
			{
				prediction += reference[(reference_y + 1) * reference_pitch + reference_x];
				if (horizontal_half)
					prediction += reference[(reference_y + 1) * reference_pitch + reference_x + 1];
			}

			const int divisor = (horizontal_half + 1) * (vertical_half + 1);
			prediction = (prediction + divisor / 2) / divisor;
			u8 &target = destination[(y + row) * destination_pitch + x + column];
			target = average ? ((int(target) + prediction + 1) / 2) : prediction;
		}
	}
}

void mpeg_video::block(unsigned index, bool intra)
{
	int quantized[64]{};
	int scan_position = intra ? 0 : -1;

	if (intra)
	{
		const int size = dc_size(index < 4);
		int differential = size ? int(gb(size)) : 0;
		if (size && !(differential & (1 << (size - 1))))
			differential = differential + 1 - (1 << size);
		quantized[0] = differential;
	}
	else
	{
		int run, level;
		dct_coefficient(true, run, level);
		scan_position = run;
		if (scan_position > 63)
			throw invalid_stream();
		quantized[scan_position] = level;
	}

	if (m_picture_coding_type != 4)
	{
		while (peek(2) != 2)
		{
			int run, level;
			dct_coefficient(false, run, level);
			scan_position += run + 1;
			if (scan_position > 63)
				throw invalid_stream();
			quantized[scan_position] = level;
		}
		gb(2); // end of block
	}

	int coefficients[64];
	bool dc_only = true;
	for (unsigned natural = 0; natural != 64; natural++)
	{
		const int level = quantized[s_scan[natural]];
		dc_only = dc_only && (!natural || !level);
		const int sign = (level > 0) - (level < 0);
		int reconstructed;
		if (intra)
		{
			reconstructed = (2 * level * m_quantizer_scale * m_intra_quantizer_matrix[natural]) / 16;
		}
		else
		{
			reconstructed = ((2 * level + sign) * m_quantizer_scale * m_non_intra_quantizer_matrix[natural]) / 16;
		}
		if (!(reconstructed & 1))
			reconstructed -= (reconstructed > 0) - (reconstructed < 0);
		coefficients[natural] = std::clamp(reconstructed, -2048, 2047);
		if (!level)
			coefficients[natural] = 0;
	}

	if (intra)
	{
		const unsigned component = (index < 4) ? 0 : (index - 3);
		m_dc_predictor[component] += quantized[0] * 8;
		coefficients[0] = m_dc_predictor[component];
	}

	int values[64];
	inverse_dct(coefficients, values, dc_only);
	put_block(index, values, intra);
}

void mpeg_video::put_block(unsigned index, const int *values, bool intra)
{
	const int macroblock_x = (m_macroblock_address % m_mb_width) * 16;
	const int macroblock_y = (m_macroblock_address / m_mb_width) * 16;
	u8 *destination;
	int pitch;
	int x;
	int y;

	if (index < 4)
	{
		destination = m_current_frame.y.data();
		pitch = m_luma_pitch;
		x = macroblock_x + ((index & 1) * 8);
		y = macroblock_y + ((index >> 1) * 8);
	}
	else
	{
		destination = (index == 4) ? m_current_frame.cb.data() : m_current_frame.cr.data();
		pitch = m_chroma_pitch;
		x = macroblock_x / 2;
		y = macroblock_y / 2;
	}

	for (int row = 0; row != 8; row++)
	{
		for (int column = 0; column != 8; column++)
		{
			u8 &target = destination[(y + row) * pitch + x + column];
			const int value = intra ? values[row * 8 + column] : (int(target) + values[row * 8 + column]);
			target = std::clamp(value, 0, 255);
		}
	}
}

void mpeg_video::inverse_dct(const int *coefficients, int *values, bool dc_only) const
{
	if (dc_only)
	{
		// A block with no AC coefficients has the same value in every sample.
		std::fill_n(values, 64, std::clamp<int>(std::lround(double(coefficients[0]) / 8.0), -256, 255));
		return;
	}

	double intermediate[8][8];
	for (int v = 0; v != 8; v++)
	{
		for (int x = 0; x != 8; x++)
		{
			double sum = 0.0;
			for (int u = 0; u != 8; u++)
				sum += coefficients[v * 8 + u] * m_idct_basis[x][u];
			intermediate[v][x] = sum;
		}
	}

	for (int y = 0; y != 8; y++)
	{
		for (int x = 0; x != 8; x++)
		{
			double sum = 0.0;
			for (int v = 0; v != 8; v++)
				sum += intermediate[v][x] * m_idct_basis[y][v];
			values[y * 8 + x] = std::clamp<int>(std::lround(sum), -256, 255);
		}
	}
}

void mpeg_video::read_frame(frame &destination, const u8 *source, unsigned source_bytes) const
{
	const unsigned luma_bytes = m_luma_pitch * m_mb_height * 16;
	const unsigned chroma_bytes = m_chroma_pitch * m_mb_height * 8;
	if (!source || (source_bytes < (luma_bytes + 2 * chroma_bytes)))
		throw invalid_stream();

	std::copy_n(source, luma_bytes, destination.y.begin());
	std::copy_n(source + luma_bytes, chroma_bytes, destination.cb.begin());
	std::copy_n(source + luma_bytes + chroma_bytes, chroma_bytes, destination.cr.begin());
}

void mpeg_video::write_frame(const frame &source, u8 *output, unsigned output_bytes) const
{
	const unsigned luma_bytes = m_luma_pitch * m_mb_height * 16;
	const unsigned chroma_bytes = m_chroma_pitch * m_mb_height * 8;
	if (!output || (output_bytes < (luma_bytes + 2 * chroma_bytes)))
		throw invalid_stream();

	std::copy_n(source.y.begin(), luma_bytes, output);
	std::copy_n(source.cb.begin(), chroma_bytes, output + luma_bytes);
	std::copy_n(source.cr.begin(), chroma_bytes, output + luma_bytes + chroma_bytes);
}

int mpeg_video::macroblock_address_increment()
{
	int increment = 0;
	while (peek(11) == 0x00f)
		gb(11); // macroblock stuffing
	while (peek(11) == 0x008)
	{
		gb(11); // macroblock escape
		increment += 33;
	}

	const vlc_entry *const entry = decode_vlc(
			s_macroblock_address_increment,
			s_macroblock_address_increment_decoder,
			m_current_limit - m_current_pos,
			[this] (int bits) { return peek(bits); },
			[this] (int bits) { m_current_pos += bits; });
	if (!entry)
		throw invalid_stream();
	return increment + entry->value;
}

mpeg_video::macroblock_type mpeg_video::macroblock_type_code()
{
	const vlc_entry *entry = nullptr;
	auto const read = [this, &entry] (auto const &table, auto const &decoder)
	{
		entry = decode_vlc(table, decoder, m_current_limit - m_current_pos,
				[this] (int bits) { return peek(bits); },
				[this] (int bits) { m_current_pos += bits; });
	};

	switch (m_picture_coding_type)
	{
	case 1: read(s_i_macroblock_type, s_i_macroblock_type_decoder); break;
	case 2: read(s_p_macroblock_type, s_p_macroblock_type_decoder); break;
	case 3: read(s_b_macroblock_type, s_b_macroblock_type_decoder); break;
	case 4: read(s_d_macroblock_type, s_d_macroblock_type_decoder); break;
	}
	if (!entry)
		throw invalid_stream();
	const int value = entry->value;

	return macroblock_type
	{
		bool(value & TYPE_QUANT),
		bool(value & TYPE_FORWARD),
		bool(value & TYPE_BACKWARD),
		bool(value & TYPE_PATTERN),
		bool(value & TYPE_INTRA)
	};
}

int mpeg_video::coded_block_pattern()
{
	const vlc_entry *const entry = decode_vlc(
			s_coded_block_pattern,
			s_coded_block_pattern_decoder,
			m_current_limit - m_current_pos,
			[this] (int bits) { return peek(bits); },
			[this] (int bits) { m_current_pos += bits; });
	if (!entry)
		throw invalid_stream();
	return entry->value;
}

int mpeg_video::motion_code()
{
	const vlc_entry *const entry = decode_vlc(
			s_motion_code,
			s_motion_code_decoder,
			m_current_limit - m_current_pos,
			[this] (int bits) { return peek(bits); },
			[this] (int bits) { m_current_pos += bits; });
	if (!entry)
		throw invalid_stream();
	return entry->value;
}

int mpeg_video::dc_size(bool luminance)
{
	const vlc_entry *const entry = luminance
		? decode_vlc(s_dc_size_luminance, s_dc_size_luminance_decoder,
				m_current_limit - m_current_pos,
				[this] (int bits) { return peek(bits); },
				[this] (int bits) { m_current_pos += bits; })
		: decode_vlc(s_dc_size_chrominance, s_dc_size_chrominance_decoder,
				m_current_limit - m_current_pos,
				[this] (int bits) { return peek(bits); },
				[this] (int bits) { m_current_pos += bits; });
	if (!entry)
		throw invalid_stream();
	return entry->value;
}

void mpeg_video::dct_coefficient(bool first, int &run, int &level)
{
	if (first && (peek(1) == 1))
	{
		gb(1);
		run = 0;
		level = gb(1) ? -1 : 1;
		return;
	}
	if (!first && (peek(2) == 3))
	{
		gb(2);
		run = 0;
		level = gb(1) ? -1 : 1;
		return;
	}
	if (peek(6) == 1)
	{
		gb(6);
		run = gb(6);
		const int level_byte = gb(8);
		if (!level_byte)
		{
			const int extension = gb(8);
			if (extension < 0x80)
				throw invalid_stream();
			level = extension;
		}
		else if (level_byte == 0x80)
		{
			const int extension = gb(8);
			if (!extension || (extension > 0x80))
				throw invalid_stream();
			level = extension - 256;
		}
		else
		{
			level = util::sext(level_byte, 8);
		}
		return;
	}

	const dct_vlc_entry *const entry = decode_vlc(
			s_dct_coefficient,
			s_dct_coefficient_decoder,
			m_current_limit - m_current_pos,
			[this] (int bits) { return peek(bits); },
			[this] (int bits) { m_current_pos += bits; });
	if (entry)
	{
		run = entry->run;
		level = gb(1) ? -entry->level : entry->level;
		return;
	}
	throw invalid_stream();
}

void mpeg_video::next_start_code()
{
	if (m_current_pos & 7)
	{
		if (gb(8 - (m_current_pos & 7)))
			throw invalid_stream();
	}
	while (peek(24) != START_CODE_PREFIX)
		gb(8);
}

u32 mpeg_video::peek(int count) const
{
	if ((count < 0) || (count > 32) || ((m_current_pos + count) > m_current_limit))
		throw limit_hit();
	if (!count)
		return 0;

	const unsigned byte_position = m_current_pos / 8;
	const unsigned bit_offset = m_current_pos & 7;
	const unsigned bytes = (bit_offset + count + 7) / 8;
	u64 source = 0;
	for (unsigned byte = 0; byte != bytes; byte++)
		source = (source << 8) | m_base[byte_position + byte];
	return (source >> (bytes * 8 - bit_offset - count)) & make_bitmask<u32>(count);
}

u32 mpeg_video::gb(int count)
{
	const u32 value = peek(count);
	m_current_pos += count;
	return value;
}
