// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Hudson/NEC HuC6271 "Rainbow" device

    Decodes a compressed image stream fed by the HuC6272 (KING) from KRAM,
    16 lines at a time, into one of two line buffers while the other one is displayed.

    Block types (FF xx marker, then a 16-bit big endian size):
    - F0-F3: run-length encoded palette indices, 4/3/2/1 bits of colour per byte;
    - F8: JPEG-like 16x16 macroblocks (Y 4x 8x8, U and V 8x8 at half resolution);
    - FF: as F8, preceded by 2x 64 byte Y/UV quantization tables.
    An FF byte inside the data is followed by a stuffing byte, which the
    block size counts.

    The Huffman codes are the encoder tables of Hudson's MPCONV2.EXE (GMAKER PC-FXGA
    SDK, 1995).

    TODO:
    - Chroma interpolation is not done, chroma is repeated over 2x2 dots;
    - Decode timing is approximate (one block per 16 lines);

***************************************************************************/

#include "emu.h"
#include "huc6271.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <numbers>


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(HUC6271, huc6271_device, "huc6271", "Hudson HuC6271 \"Rainbow\"")

namespace {

// Huffman codes: { code length, code, value }
struct huffman_code { u8 length; u16 code; u8 value; };

// DC luminance: 0-e = number of difference bits, f = run of null macroblocks follows, 10+ = new quantization scale
constexpr huffman_code DC_Y_CODES[] =
{
	{ 3, 0x000, 0x00 }, { 3, 0x001, 0x02 }, { 3, 0x002, 0x03 }, { 3, 0x003, 0x04 }, { 3, 0x004, 0x05 },
	{ 3, 0x005, 0x06 }, { 3, 0x006, 0x07 }, { 4, 0x00e, 0x01 }, { 6, 0x03c, 0x08 }, { 7, 0x07a, 0x09 },
	{ 7, 0x07b, 0x0f }, { 9, 0x1f0, 0x10 }, { 9, 0x1f1, 0x11 }, { 9, 0x1f2, 0x12 }, { 9, 0x1f3, 0x13 },
	{ 9, 0x1f4, 0x14 }, { 9, 0x1f5, 0x15 }, { 9, 0x1f6, 0x16 }, { 9, 0x1f7, 0x17 }, { 9, 0x1f8, 0x18 },
	{ 9, 0x1f9, 0x19 }, { 9, 0x1fa, 0x1a }, { 9, 0x1fb, 0x1b }, { 9, 0x1fc, 0x1c }, { 9, 0x1fd, 0x1d },
	{ 9, 0x1fe, 0x1e }, { 9, 0x1ff, 0x1f }
};

// DC chrominance: number of difference bits
constexpr huffman_code DC_UV_CODES[] =
{
	{ 2, 0x000, 0x00 }, { 2, 0x001, 0x01 }, { 2, 0x002, 0x02 }, { 3, 0x006, 0x03 }, { 4, 0x00e, 0x04 },
	{ 5, 0x01e, 0x05 }, { 6, 0x03e, 0x06 }, { 7, 0x07e, 0x07 }, { 8, 0x0fe, 0x08 }, { 8, 0x0ff, 0x09 }
};

// AC: upper nibble = zero run, lower nibble = number of coefficient bits
constexpr huffman_code AC_Y_CODES[] =
{
	{ 2, 0x000, 0x01 }, { 2, 0x001, 0x02 }, { 3, 0x004, 0x03 }, { 4, 0x00a, 0x04 }, { 4, 0x00b, 0x11 },
	{ 5, 0x018, 0x05 }, { 5, 0x019, 0x12 }, { 5, 0x01a, 0x21 }, { 5, 0x01f, 0x00 }, { 6, 0x036, 0x06 },
	{ 6, 0x037, 0x31 }, { 6, 0x038, 0x41 }, { 6, 0x039, 0x51 }, { 7, 0x074, 0x13 }, { 7, 0x075, 0x22 },
	{ 7, 0x076, 0x61 }, { 9, 0x1dc, 0x07 }, { 9, 0x1dd, 0x71 }, { 11, 0x778, 0x09 }, { 11, 0x779, 0x19 },
	{ 11, 0x77a, 0x29 }, { 11, 0x77b, 0x39 }, { 11, 0x77c, 0x49 }, { 11, 0x77d, 0x59 }, { 11, 0x77e, 0x69 },
	{ 11, 0x77f, 0x79 }, { 11, 0x780, 0x89 }, { 11, 0x781, 0x99 }, { 11, 0x782, 0xa9 }, { 11, 0x783, 0xb9 },
	{ 11, 0x784, 0xc9 }, { 11, 0x785, 0xd9 }, { 11, 0x786, 0xe9 }, { 11, 0x787, 0xf9 }, { 12, 0xf10, 0x08 },
	{ 12, 0xf11, 0x14 }, { 12, 0xf12, 0x15 }, { 12, 0xf13, 0x16 }, { 12, 0xf14, 0x17 }, { 12, 0xf15, 0x18 },
	{ 12, 0xf16, 0x23 }, { 12, 0xf17, 0x24 }, { 12, 0xf18, 0x25 }, { 12, 0xf19, 0x26 }, { 12, 0xf1a, 0x27 },
	{ 12, 0xf1b, 0x28 }, { 12, 0xf1c, 0x32 }, { 12, 0xf1d, 0x33 }, { 12, 0xf1e, 0x34 }, { 12, 0xf1f, 0x35 },
	{ 12, 0xf20, 0x36 }, { 12, 0xf21, 0x37 }, { 12, 0xf22, 0x38 }, { 12, 0xf23, 0x42 }, { 12, 0xf24, 0x43 },
	{ 12, 0xf25, 0x44 }, { 12, 0xf26, 0x45 }, { 12, 0xf27, 0x46 }, { 12, 0xf28, 0x47 }, { 12, 0xf29, 0x48 },
	{ 12, 0xf2a, 0x52 }, { 12, 0xf2b, 0x53 }, { 12, 0xf2c, 0x54 }, { 12, 0xf2d, 0x55 }, { 12, 0xf2e, 0x56 },
	{ 12, 0xf2f, 0x57 }, { 12, 0xf30, 0x58 }, { 12, 0xf31, 0x62 }, { 12, 0xf32, 0x63 }, { 12, 0xf33, 0x64 },
	{ 12, 0xf34, 0x65 }, { 12, 0xf35, 0x66 }, { 12, 0xf36, 0x67 }, { 12, 0xf37, 0x68 }, { 12, 0xf38, 0x72 },
	{ 12, 0xf39, 0x73 }, { 12, 0xf3a, 0x74 }, { 12, 0xf3b, 0x75 }, { 12, 0xf3c, 0x76 }, { 12, 0xf3d, 0x77 },
	{ 12, 0xf3e, 0x78 }, { 12, 0xf3f, 0x81 }, { 12, 0xf40, 0x82 }, { 12, 0xf41, 0x83 }, { 12, 0xf42, 0x84 },
	{ 12, 0xf43, 0x85 }, { 12, 0xf44, 0x86 }, { 12, 0xf45, 0x87 }, { 12, 0xf46, 0x88 }, { 12, 0xf47, 0x91 },
	{ 12, 0xf48, 0x92 }, { 12, 0xf49, 0x93 }, { 12, 0xf4a, 0x94 }, { 12, 0xf4b, 0x95 }, { 12, 0xf4c, 0x96 },
	{ 12, 0xf4d, 0x97 }, { 12, 0xf4e, 0x98 }, { 12, 0xf4f, 0xa1 }, { 12, 0xf50, 0xa2 }, { 12, 0xf51, 0xa3 },
	{ 12, 0xf52, 0xa4 }, { 12, 0xf53, 0xa5 }, { 12, 0xf54, 0xa6 }, { 12, 0xf55, 0xa7 }, { 12, 0xf56, 0xa8 },
	{ 12, 0xf57, 0xb1 }, { 12, 0xf58, 0xb2 }, { 12, 0xf59, 0xb3 }, { 12, 0xf5a, 0xb4 }, { 12, 0xf5b, 0xb5 },
	{ 12, 0xf5c, 0xb6 }, { 12, 0xf5d, 0xb7 }, { 12, 0xf5e, 0xb8 }, { 12, 0xf5f, 0xc1 }, { 12, 0xf60, 0xc2 },
	{ 12, 0xf61, 0xc3 }, { 12, 0xf62, 0xc4 }, { 12, 0xf63, 0xc5 }, { 12, 0xf64, 0xc6 }, { 12, 0xf65, 0xc7 },
	{ 12, 0xf66, 0xc8 }, { 12, 0xf67, 0xd1 }, { 12, 0xf68, 0xd2 }, { 12, 0xf69, 0xd3 }, { 12, 0xf6a, 0xd4 },
	{ 12, 0xf6b, 0xd5 }, { 12, 0xf6c, 0xd6 }, { 12, 0xf6d, 0xd7 }, { 12, 0xf6e, 0xd8 }, { 12, 0xf6f, 0xe1 },
	{ 12, 0xf70, 0xe2 }, { 12, 0xf71, 0xe3 }, { 12, 0xf72, 0xe4 }, { 12, 0xf73, 0xe5 }, { 12, 0xf74, 0xe6 },
	{ 12, 0xf75, 0xe7 }, { 12, 0xf76, 0xe8 }, { 12, 0xf77, 0xf1 }, { 12, 0xf78, 0xf2 }, { 12, 0xf79, 0xf3 },
	{ 12, 0xf7a, 0xf4 }, { 12, 0xf7b, 0xf5 }, { 12, 0xf7c, 0xf6 }, { 12, 0xf7d, 0xf7 }, { 12, 0xf7e, 0xf8 },
	{ 12, 0xf7f, 0x10 }
};

constexpr huffman_code AC_UV_CODES[] =
{
	{ 2, 0x000, 0x01 }, { 2, 0x001, 0x02 }, { 3, 0x004, 0x11 }, { 4, 0x00a, 0x03 }, { 4, 0x00b, 0x21 },
	{ 5, 0x018, 0x04 }, { 5, 0x019, 0x12 }, { 5, 0x01a, 0x31 }, { 5, 0x01b, 0x41 }, { 5, 0x01f, 0x00 },
	{ 6, 0x038, 0x51 }, { 7, 0x072, 0x05 }, { 7, 0x073, 0x13 }, { 7, 0x074, 0x22 }, { 7, 0x075, 0x61 },
	{ 7, 0x076, 0x71 }, { 9, 0x1dc, 0x32 }, { 9, 0x1dd, 0x81 }, { 11, 0x778, 0x09 }, { 11, 0x779, 0x19 },
	{ 11, 0x77a, 0x29 }, { 11, 0x77b, 0x39 }, { 11, 0x77c, 0x49 }, { 11, 0x77d, 0x59 }, { 11, 0x77e, 0x69 },
	{ 11, 0x77f, 0x79 }, { 11, 0x780, 0x89 }, { 11, 0x781, 0x99 }, { 11, 0x782, 0xa9 }, { 11, 0x783, 0xb9 },
	{ 11, 0x784, 0xc9 }, { 11, 0x785, 0xd9 }, { 11, 0x786, 0xe9 }, { 11, 0x787, 0xf9 }, { 12, 0xf10, 0x06 },
	{ 12, 0xf11, 0x07 }, { 12, 0xf12, 0x08 }, { 12, 0xf13, 0x14 }, { 12, 0xf14, 0x15 }, { 12, 0xf15, 0x16 },
	{ 12, 0xf16, 0x17 }, { 12, 0xf17, 0x18 }, { 12, 0xf18, 0x23 }, { 12, 0xf19, 0x24 }, { 12, 0xf1a, 0x25 },
	{ 12, 0xf1b, 0x26 }, { 12, 0xf1c, 0x27 }, { 12, 0xf1d, 0x28 }, { 12, 0xf1e, 0x33 }, { 12, 0xf1f, 0x34 },
	{ 12, 0xf20, 0x35 }, { 12, 0xf21, 0x36 }, { 12, 0xf22, 0x37 }, { 12, 0xf23, 0x38 }, { 12, 0xf24, 0x42 },
	{ 12, 0xf25, 0x43 }, { 12, 0xf26, 0x44 }, { 12, 0xf27, 0x45 }, { 12, 0xf28, 0x46 }, { 12, 0xf29, 0x47 },
	{ 12, 0xf2a, 0x48 }, { 12, 0xf2b, 0x52 }, { 12, 0xf2c, 0x53 }, { 12, 0xf2d, 0x54 }, { 12, 0xf2e, 0x55 },
	{ 12, 0xf2f, 0x56 }, { 12, 0xf30, 0x57 }, { 12, 0xf31, 0x58 }, { 12, 0xf32, 0x62 }, { 12, 0xf33, 0x63 },
	{ 12, 0xf34, 0x64 }, { 12, 0xf35, 0x65 }, { 12, 0xf36, 0x66 }, { 12, 0xf37, 0x67 }, { 12, 0xf38, 0x68 },
	{ 12, 0xf39, 0x72 }, { 12, 0xf3a, 0x73 }, { 12, 0xf3b, 0x74 }, { 12, 0xf3c, 0x75 }, { 12, 0xf3d, 0x76 },
	{ 12, 0xf3e, 0x77 }, { 12, 0xf3f, 0x78 }, { 12, 0xf40, 0x82 }, { 12, 0xf41, 0x83 }, { 12, 0xf42, 0x84 },
	{ 12, 0xf43, 0x85 }, { 12, 0xf44, 0x86 }, { 12, 0xf45, 0x87 }, { 12, 0xf46, 0x88 }, { 12, 0xf47, 0x91 },
	{ 12, 0xf48, 0x92 }, { 12, 0xf49, 0x93 }, { 12, 0xf4a, 0x94 }, { 12, 0xf4b, 0x95 }, { 12, 0xf4c, 0x96 },
	{ 12, 0xf4d, 0x97 }, { 12, 0xf4e, 0x98 }, { 12, 0xf4f, 0xa1 }, { 12, 0xf50, 0xa2 }, { 12, 0xf51, 0xa3 },
	{ 12, 0xf52, 0xa4 }, { 12, 0xf53, 0xa5 }, { 12, 0xf54, 0xa6 }, { 12, 0xf55, 0xa7 }, { 12, 0xf56, 0xa8 },
	{ 12, 0xf57, 0xb1 }, { 12, 0xf58, 0xb2 }, { 12, 0xf59, 0xb3 }, { 12, 0xf5a, 0xb4 }, { 12, 0xf5b, 0xb5 },
	{ 12, 0xf5c, 0xb6 }, { 12, 0xf5d, 0xb7 }, { 12, 0xf5e, 0xb8 }, { 12, 0xf5f, 0xc1 }, { 12, 0xf60, 0xc2 },
	{ 12, 0xf61, 0xc3 }, { 12, 0xf62, 0xc4 }, { 12, 0xf63, 0xc5 }, { 12, 0xf64, 0xc6 }, { 12, 0xf65, 0xc7 },
	{ 12, 0xf66, 0xc8 }, { 12, 0xf67, 0xd1 }, { 12, 0xf68, 0xd2 }, { 12, 0xf69, 0xd3 }, { 12, 0xf6a, 0xd4 },
	{ 12, 0xf6b, 0xd5 }, { 12, 0xf6c, 0xd6 }, { 12, 0xf6d, 0xd7 }, { 12, 0xf6e, 0xd8 }, { 12, 0xf6f, 0xe1 },
	{ 12, 0xf70, 0xe2 }, { 12, 0xf71, 0xe3 }, { 12, 0xf72, 0xe4 }, { 12, 0xf73, 0xe5 }, { 12, 0xf74, 0xe6 },
	{ 12, 0xf75, 0xe7 }, { 12, 0xf76, 0xe8 }, { 12, 0xf77, 0xf1 }, { 12, 0xf78, 0xf2 }, { 12, 0xf79, 0xf3 },
	{ 12, 0xf7a, 0xf4 }, { 12, 0xf7b, 0xf5 }, { 12, 0xf7c, 0xf6 }, { 12, 0xf7d, 0xf7 }, { 12, 0xf7e, 0xf8 },
	{ 12, 0xf7f, 0x10 }
};

constexpr u8 ZIGZAG[63] =
{
	0x01, 0x08, 0x10, 0x09, 0x02, 0x03, 0x0a, 0x11,
	0x18, 0x20, 0x19, 0x12, 0x0b, 0x04, 0x05, 0x0c,
	0x13, 0x1a, 0x21, 0x28, 0x30, 0x29, 0x22, 0x1b,
	0x14, 0x0d, 0x06, 0x07, 0x0e, 0x15, 0x1c, 0x23,
	0x2a, 0x31, 0x38, 0x39, 0x32, 0x2b, 0x24, 0x1d,
	0x16, 0x0f, 0x17, 0x1e, 0x25, 0x2c, 0x33, 0x3a,
	0x3b, 0x34, 0x2d, 0x26, 0x1f, 0x27, 0x2e, 0x35,
	0x3c, 0x3d, 0x36, 0x2f, 0x37, 0x3e, 0x3f
};

// fill a decoding table indexed by the next log2(N) bits of the stream
template <typename T, size_t N, size_t M>
void build_lut(const huffman_code (&codes)[M], std::array<T, N> &lut)
{
	static_assert(std::has_single_bit(N));
	constexpr int BITS = std::bit_width(N) - 1;
	lut.fill(T{ 0, 0 });
	for (const huffman_code &c : codes)
	{
		const int span = BITS - c.length;
		for (int j = 0; j < (1 << span); j++)
			lut[(c.code << span) | j] = T{ c.value, c.length };
	}
}

} // anonymous namespace


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

huc6271_device::huc6271_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HUC6271, tag, owner, clock)
	, m_hscroll(0)
	, m_control(0)
	, m_back_y(0)
	, m_back_u(0)
	, m_back_v(0)
	, m_hsync(0)
{
}

void huc6271_device::amap(address_map &map)
{
	// D0-D7 of $00 set the low byte, D0-D7 of $02 the high byte
	map(0x00, 0x03).lw32(NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
		if (ACCESSING_BITS_0_7)
			m_hscroll = (m_hscroll & 0xff00) | (data & 0xff);
		if (ACCESSING_BITS_16_23)
			m_hscroll = (m_hscroll & 0x00ff) | ((data >> 8) & 0xff00);
		m_hscroll &= 0x1ff;
	}));
	// ---- -?-- <unknown>
	// ---- --x- endless scroll mode
	// ---- ---x enable Rainbow transfers
	map(0x04, 0x05).lw16(NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_control); }));
	// colour of null (skipped) macroblocks, signed
	map(0x08, 0x09).lw16(NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_back_y); }));
	map(0x0c, 0x0d).lw16(NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_back_u); }));
	map(0x10, 0x11).lw16(NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_back_v); }));
	map(0x14, 0x15).lw16(NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_hsync); }));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void huc6271_device::device_start()
{
	build_lut(DC_Y_CODES, m_dc_y_lut);
	build_lut(DC_UV_CODES, m_dc_uv_lut);
	build_lut(AC_Y_CODES, m_ac_y_lut);
	build_lut(AC_UV_CODES, m_ac_uv_lut);

	for (int x = 0; x < 8; x++)
	{
		for (int u = 0; u < 8; u++)
			m_idct_cos[x][u] = (u ? 1.0 : 1.0 / std::numbers::sqrt2) * std::cos((2 * x + 1) * u * std::numbers::pi / 16);
	}

	save_item(NAME(m_control));
	save_item(NAME(m_hscroll));
	save_item(NAME(m_hsync));
	save_item(NAME(m_back_y));
	save_item(NAME(m_back_u));
	save_item(NAME(m_back_v));
	save_item(NAME(m_buffer));
	save_item(NAME(m_format));
	save_item(NAME(m_read_buffer));
	save_item(NAME(m_read_line));
	save_item(NAME(m_quant));
	save_item(NAME(m_quant_base));
	save_item(NAME(m_garbage));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void huc6271_device::device_reset()
{
	m_control = 0;
	m_back_y = m_back_u = m_back_v = 0;
	m_hscroll = 0;
	m_read_line = 0;
	m_read_buffer = 0;
	m_format[0] = m_format[1] = FORMAT_NONE;
	m_garbage = false;
	for (auto &q : m_quant)
		std::fill(std::begin(q), std::end(q), 0);
	for (auto &q : m_quant_base)
		std::fill(std::begin(q), std::end(q), 0);
}


//**************************************************************************
//  DECODER
//**************************************************************************

// the block size counts the stuffing byte after an FF too
u8 huc6271_device::stream_byte()
{
	if (m_bytes_left <= 0)
		return 0;
	const u8 data = m_fetch();
	m_bytes_left--;
	if (data == 0xff)
	{
		m_fetch();
		m_bytes_left--;
	}
	return data;
}

u32 huc6271_device::peek_bits(int count)
{
	while (m_bit_count < count)
	{
		m_bit_buffer = (m_bit_buffer << 8) | stream_byte();
		m_bit_count += 8;
	}
	return (m_bit_buffer >> (m_bit_count - count)) & util::make_bitmask<u32>(count);
}

// read a JPEG style magnitude: values below half the range are negative
s32 huc6271_device::get_signed(int count)
{
	if (!count)
		return 0;
	s32 value = peek_bits(count);
	m_bit_count -= count;
	if (value < (1 << (count - 1)))
		value += 1 - (1 << count);
	return value;
}

u8 huc6271_device::get_code(const lut_entry *lut, int bits)
{
	const lut_entry &entry = lut[peek_bits(bits)];
	m_bit_count -= entry.length;
	return entry.value;
}

s32 huc6271_device::get_ac(const lut_entry *lut, int &zeroes)
{
	const u8 code = get_code(lut, 12);
	zeroes = code >> 4;
	return get_signed(code & 0xf);
}

// returns the DC difference, or sets null_run to a count of null macroblocks
s32 huc6271_device::get_dc_y(int &null_run)
{
	null_run = 0;
	for (;;)
	{
		const u8 code = get_code(m_dc_y_lut.data(), 9);
		if (code < 0xf)
		{
			return get_signed(code);
		}
		else if (code == 0xf)
		{
			get_ac(m_ac_y_lut.data(), null_run);
			null_run++;
			return 0;
		}
		else
		{
			// new quantization scale, the UV DC entry is not scaled
			const u32 scale = code - 0x10;
			for (int i = 0; i < 64; i++)
			{
				m_quant[0][i] = std::clamp<u32>((m_quant_base[0][i] * scale) >> 2, 1, 0xfe);
				m_quant[1][i] = std::clamp<u32>(i ? ((m_quant_base[1][i] * scale) >> 2) : (m_quant_base[1][i] >> 2), 1, 0xfe);
			}
		}
	}
}

s32 huc6271_device::get_dc_uv()
{
	return get_signed(get_code(m_dc_uv_lut.data(), 8));
}

void huc6271_device::decode_coefficients(s32 *dct, const u8 *quant, s32 dc, const lut_entry *lut)
{
	dct[0] = s16(quant[0] * dc);
	int count = 0;
	while (count < 63)
	{
		int zeroes;
		const s32 coeff = get_ac(lut, zeroes);
		if (!coeff)
		{
			if (!zeroes)
			{
				// end of block
				while (count < 63)
					dct[ZIGZAG[count++]] = 0;
				break;
			}
			else if (zeroes == 1)
			{
				zeroes = 0xf;
			}
		}
		while (zeroes-- && count < 63)
			dct[ZIGZAG[count++]] = 0;
		if (count < 63)
		{
			const int index = ZIGZAG[count++];
			dct[index] = s16(quant[index] * coeff);
		}
	}
}

// 8x8 inverse DCT (4x the gain of the JPEG normalization), result level shifted to 0-255
void huc6271_device::idct(const s32 *in, u8 *out)
{
	double tmp[64];
	for (int y = 0; y < 8; y++)
	{
		for (int u = 0; u < 8; u++)
		{
			double sum = 0;
			for (int v = 0; v < 8; v++)
				sum += m_idct_cos[y][v] * in[v * 8 + u];
			tmp[y * 8 + u] = sum;
		}
	}
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			double sum = 0;
			for (int u = 0; u < 8; u++)
				sum += m_idct_cos[x][u] * tmp[y * 8 + u];
			out[y * 8 + x] = std::clamp<int>(int(std::lround(sum)) + 0x80, 0, 255);
		}
	}
}

void huc6271_device::decode_jpeg(int buffer, int size)
{
	m_bytes_left = size;
	m_bit_buffer = 0;
	m_bit_count = 0;

	const u32 null_colour = (u8(m_back_y + 0x80) << 16) | (u8(m_back_u + 0x80) << 8) | u8(m_back_v + 0x80);
	u32 *const dest = &m_buffer[buffer][0];
	s32 dc_y = 0, dc_u = 0, dc_v = 0;

	for (int column = 0; column < 16; column++)
	{
		int null_run;
		dc_y += get_dc_y(null_run);
		if (null_run)
		{
			for ( ; null_run && column < 16; null_run--, column++)
			{
				for (int y = 0; y < 16; y++)
					std::fill_n(&dest[y * 256 + column * 16], 16, null_colour);
			}
			column--;
			dc_y = dc_u = dc_v = 0;
			continue;
		}

		s32 dct[6][64];
		decode_coefficients(dct[0], m_quant[0], dc_y, m_ac_y_lut.data());
		dc_y += get_dc_y(null_run);
		decode_coefficients(dct[1], m_quant[0], dc_y, m_ac_y_lut.data());
		dc_y += get_dc_y(null_run);
		decode_coefficients(dct[2], m_quant[0], dc_y, m_ac_y_lut.data());
		dc_y += get_dc_y(null_run);
		decode_coefficients(dct[3], m_quant[0], dc_y, m_ac_y_lut.data());
		dc_u += get_dc_uv();
		decode_coefficients(dct[4], m_quant[1], dc_u, m_ac_uv_lut.data());
		dc_v += get_dc_uv();
		decode_coefficients(dct[5], m_quant[1], dc_v, m_ac_uv_lut.data());

		u8 pix[6][64];
		for (int i = 0; i < 6; i++)
			idct(dct[i], pix[i]);

		// luminance blocks: top left, bottom left, top right, bottom right
		for (int y = 0; y < 16; y++)
		{
			for (int x = 0; x < 16; x++)
			{
				const int block = ((x >> 3) << 1) | (y >> 3);
				const int cy = y >> 1, cx = x >> 1;
				dest[y * 256 + column * 16 + x] = (pix[block][(y & 7) * 8 + (x & 7)] << 16) | (pix[4][cy * 8 + cx] << 8) | pix[5][cy * 8 + cx];
			}
		}
	}
}

void huc6271_device::decode_rle(int buffer, int type, int size)
{
	// F0-F3 = 4-1 bits of colour, the rest of the byte is the run length (0 = in the next byte)
	const int colour_shift = 4 - (type & 3);
	const u8 run_mask = util::make_bitmask<u8>(colour_shift);
	u32 *const dest = &m_buffer[buffer][0];
	int x = 0;

	while (size > 0)
	{
		const u8 data = m_fetch();
		size--;
		if (data == 0xff)
		{
			m_fetch();
			size--;
		}

		int run = data & run_mask;
		if (!run)
		{
			const u8 count = m_fetch();
			size--;
			if (count == 0xff)
			{
				m_fetch();
				size--;
			}
			run = count + 1;
		}

		for ( ; run && x < 256 * 16; run--)
			dest[x++] = data >> colour_shift;
	}
}

// decode the next 16 lines into the back buffer, fetching the stream through "fetch"
void huc6271_device::decode_block(bool first, const fetch_delegate &fetch)
{
	if (!BIT(m_control, 0))
		return;

	m_fetch = fetch;
	const int buffer = m_read_buffer ^ 1;

	if (first)
		m_garbage = false;

	// search for a block marker
	int budget = m_garbage ? 0 : 0x200;
	u8 type;
	int size;
	do
	{
		do
		{
			while (m_fetch() != 0xff && budget > 0)
				budget--;
			type = m_fetch();
			budget--;
		} while (type != 0xf0 && type != 0xf1 && type != 0xf2 && type != 0xf3 && type != 0xf8 && type != 0xff && budget > 0);

		const u8 size_high = m_fetch();
		size = s16((size_high << 8) | m_fetch()) - 2;

		if (type == 0xff && size <= 0)
		{
			for (int i = 0; i < 128; i++, budget--)
				m_fetch();
		}
	} while (size <= 0 && budget > 0);

	if (budget <= 0)
	{
		m_garbage = true;
		m_format[buffer] = FORMAT_PALETTE;
		std::fill(std::begin(m_buffer[buffer]), std::end(m_buffer[buffer]), 0);
		return;
	}

	if (type == 0xf8 || type == 0xff)
	{
		m_format[buffer] = FORMAT_YUV;
		if (type == 0xff)
		{
			for (int q = 0; q < 2; q++)
			{
				for (int i = 0; i < 64; i++)
					m_quant[q][i] = m_quant_base[q][i] = m_fetch();
			}
			size -= 128;
		}
		decode_jpeg(buffer, size);
	}
	else
	{
		m_format[buffer] = FORMAT_PALETTE;
		decode_rle(buffer, type, size);
	}
}

void huc6271_device::swap_buffers()
{
	m_read_buffer ^= 1;
	m_read_line = 0;
}

// fetch the next decoded line: YUV888 for FORMAT_YUV, palette index for FORMAT_PALETTE, 0 = transparent
int huc6271_device::fetch_line(u32 *dest)
{
	const int format = m_format[m_read_buffer];
	const bool endless = BIT(m_control, 1);

	if (format == FORMAT_NONE)
	{
		std::fill_n(dest, 256, 0);
	}
	else if (format == FORMAT_YUV)
	{
		u32 *const src = &m_buffer[m_read_buffer][m_read_line * 256];
		for (int x = 0; x < 256; x++)
		{
			const u16 pos = endless ? ((m_hscroll + x) & 0xff) : ((m_hscroll + x) & 0x1ff);
			dest[x] = (pos < 256) ? src[pos] : 0;
		}
		std::fill_n(src, 256, 0);
	}
	else
	{
		const u32 *const src = &m_buffer[m_read_buffer][m_read_line * 256];
		for (int x = 0; x < 256; x++)
		{
			const u16 pos = endless ? ((m_hscroll + x) & 0xff) : ((m_hscroll + x) & 0x1ff);
			dest[x] = (pos < 256) ? src[pos] : 0;
		}
	}

	m_read_line = (m_read_line + 1) & 0xf;
	if (!m_read_line)
		m_format[m_read_buffer] = FORMAT_NONE;

	return format;
}
