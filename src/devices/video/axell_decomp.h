// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen, Morten Shearman Kirkegaard
/**********************************************************************

    Axell AG-1 / AG-2 character data decompressor

    Huffman/RLE scheme used by the blitters of the AG-1 (AX51101,
    AX51102) and AG-2 (AX51201) graphics display controllers.  The
    output is produced in vertical strips of 12 pixels, going down in
    even strips and up in odd ones.  The order in which the source
    bytes are fetched is chip specific and supplied by a derived class.

**********************************************************************/

#ifndef MAME_VIDEO_AXELL_DECOMP_H
#define MAME_VIDEO_AXELL_DECOMP_H

#pragma once


class axell_decompressor
{
public:
	virtual ~axell_decompressor();

	// read bits from the stream, least significant first
	u32 get_bits(u32 count);

	// decompress a width x height block to (dx, dy) of the destination;
	// returns true if the stream is malformed
	bool decompress(int dx, int dy, int width, int height);

protected:
	// with check_runs set, a run crossing a strip or the end of the block
	// is treated as a malformed stream
	axell_decompressor(u8 *dest, u32 x_bits, u32 x_mask, u32 y_mask, bool check_runs);

	// next byte of the compressed stream; sets m_error when the source is exhausted
	virtual u8 fetch_byte() = 0;

	bool m_error;

private:
	u32 addr(int x, int y) const { return (x & m_x_mask) | ((y & m_y_mask) << m_x_bits); }
	int get_next_bit();
	int next_node();
	void set_o(u8 v);
	u8 get_o(int x, int y) const;
	void copy(int dx, u32 count);

	u8 *const m_dest;
	const u32 m_x_bits;
	const u32 m_x_mask;
	const u32 m_y_mask;
	const bool m_check_runs;

	u8 m_bits;
	int m_num_bits;
	u8 m_colour[16];
	int m_dx, m_dy;
	int m_ow, m_oh;
	int m_ox, m_oy;
};

#endif // MAME_VIDEO_AXELL_DECOMP_H
