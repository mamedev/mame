// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Hudson/NEC HuC6271 "Rainbow" device

***************************************************************************/

#ifndef MAME_VIDEO_HUC6271_H
#define MAME_VIDEO_HUC6271_H

#pragma once

#include <array>
#include <functional>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> huc6271_device

class huc6271_device : public device_t
{
public:
	using fetch_delegate = std::function<u8 ()>;

	enum
	{
		FORMAT_NONE = -1,
		FORMAT_PALETTE = 0,
		FORMAT_YUV = 1
	};

	// construction/destruction
	huc6271_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void amap(address_map &map) ATTR_COLD;

	// interface to the HuC6272
	void hsync();
	void decode_block(bool first, const fetch_delegate &fetch);
	void swap_buffers();
	int fetch_line(u32 *dest);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	struct lut_entry { u8 value; u8 length; };

	u16 m_hscroll;
	u16 m_control;          // R01 as currently applied
	u16 m_control_reg;      // R01 as written
	u16 m_back_y, m_back_u, m_back_v;
	u16 m_hsr;

	// two 256x16 buffers, YUV888 or palette indices
	u32 m_buffer[2][256 * 16];
	s32 m_format[2];
	u8 m_read_buffer;
	u8 m_read_line;
	u8 m_quant[2][64], m_quant_base[2][64];
	bool m_garbage;

	// bit reader state, only used while decoding
	fetch_delegate m_fetch;
	s32 m_bytes_left;
	u32 m_bit_buffer;
	int m_bit_count;

	// Huffman decoding tables, indexed by the next 9/8/12/12 bits of the stream
	std::array<lut_entry, 512> m_dc_y_lut;
	std::array<lut_entry, 256> m_dc_uv_lut;
	std::array<lut_entry, 4096> m_ac_y_lut, m_ac_uv_lut;
	double m_idct_cos[8][8];

	u8 stream_byte();
	u32 peek_bits(int count);
	s32 get_signed(int count);
	u8 get_code(const lut_entry *lut, int bits);
	s32 get_ac(const lut_entry *lut, int &zeroes);
	s32 get_dc_y(int &null_run);
	s32 get_dc_uv();
	void decode_coefficients(s32 *dct, const u8 *quant, s32 dc, const lut_entry *lut);
	void idct(const s32 *in, u8 *out);
	void decode_jpeg(int buffer, int size);
	void decode_rle(int buffer, int type, int size);
};

// device type definition
DECLARE_DEVICE_TYPE(HUC6271, huc6271_device)

#endif // MAME_VIDEO_HUC6271_H
