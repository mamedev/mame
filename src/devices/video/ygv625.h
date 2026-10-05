// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/***************************************************************************

    Yamaha YGV625 "PVDC6R" sprite processor / CRTC

    Sprite display processor used by the Amuzy cartridge arcade/medal
    system.  Sprites are stored in external CG memory, either raw or
    compressed with Yamaha's SPINET lossless scheme, and are decoded on the
    fly by the chip while it renders a display list of up to 512 sprite
    attribute entries into a frame buffer.

    CPU interface (16-bit, 16 KiB window):

      0x0000-0x1fff  sprite attribute table, 512 entries of 16 bytes
      0x2000-0x227f  quadrilateral (deformation) table, 40 entries of 8 words
      0x3800-0x38ff  colour table (128 colours, used by palettes 0x100 and up)
      0x3c00-0x3fff  registers

    Sprite palettes normally live in CG memory, addressed through five
    palette base registers (one per colour depth).

    See ygv625.cpp for the attribute, register and CG data formats.

***************************************************************************/

#ifndef MAME_VIDEO_YGV625_H
#define MAME_VIDEO_YGV625_H

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>


class ygv625_device : public device_t, public device_video_interface
{
public:
	ygv625_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration: memory region holding the CG (character generator) data
	template <typename T> void set_cg_region(T &&tag) { m_cg.set_tag(std::forward<T>(tag)); }

	// 16-bit CPU bus interface, 0x2000 words
	u16 read(offs_t offset, u16 mem_mask = ~0);
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// a decoded sprite, pixels are palette indices (indexed formats) or
	// RGB565 words (direct colour)
	struct decoded_sprite
	{
		u16 width = 0;
		u16 height = 0;
		u8 format = 0;
		u8 depth = 0;           // bits per pixel for indexed formats, 16 for direct colour
		u8 transparent = 0;     // transparent colour index from the sprite header
		bool has_transparent = false;
		bool valid = false;
		std::vector<u16> pixels;
	};

	// bit reader over CG memory (MSB first)
	class bit_reader
	{
	public:
		bit_reader(const u8 *base, u32 size, u32 byte_offset) : m_base(base), m_size(size), m_pos(u64(byte_offset) * 8) { }
		u32 read(u32 bits);
		u64 pos() const { return m_pos; }
		bool overrun() const { return m_pos > u64(m_size) * 8; }
	private:
		const u8 *m_base;
		u32 m_size;
		u64 m_pos;
	};

	struct plane_params
	{
		u8 cmax, cth, esc, wn;
	};

	required_region_ptr<u8> m_cg;

	std::unique_ptr<u16[]> m_ram;          // 0x2000 words: attribute table, quad table, palette RAM
	u16 m_regs[0x200];
	bitmap_rgb32 m_bitmap;
	std::unordered_map<u64, decoded_sprite> m_cache;

	u16 m_cg_read_data;
	bool m_cg_read_ready;
	u8 m_scan_tables[4][256][2];

	// decoding
	u8 cg_byte(u32 addr) const { return (addr < m_cg.length()) ? m_cg[addr] : 0; }
	u16 cg_word(u32 addr) const { return (u16(cg_byte(addr)) << 8) | cg_byte(addr + 1); }
	const decoded_sprite &get_sprite(u32 addr, u16 width, u16 height);
	bool decode_sprite(decoded_sprite &spr, u32 addr);
	bool decode_raw(decoded_sprite &spr, u32 addr, u32 bpp);
	bool decode_indexed(decoded_sprite &spr, u32 addr);
	bool decode_rgb(decoded_sprite &spr, u32 addr);
	bool read_overrides(bit_reader &br, plane_params &p, const plane_params &base, u32 flag_layout);
	bool decode_plane(bit_reader &br, const plane_params &p, int p0, u32 mod, u16 *out);
	static void scan_position(u32 scan, u32 index, u32 &row, u32 &col);

	// palette lookup
	rgb_t lookup_colour(u32 palette, u32 depth, u32 index) const;
	static rgb_t rgb565(u16 c) { return rgb_t(pal5bit(c >> 11), pal6bit(c >> 5), pal5bit(c)); }

	// rendering
	void render_frame();
	void draw_sprite(const decoded_sprite &spr, int cx, int cy, u32 palette, u32 zoomx, u32 zoomy, bool flipx, bool flipy, bool transparency);
	void draw_sprite_quad(const decoded_sprite &spr, const double (&qx)[4], const double (&qy)[4], u32 palette, bool flipx, bool flipy, bool transparency);
};


DECLARE_DEVICE_TYPE(YGV625, ygv625_device)

#endif // MAME_VIDEO_YGV625_H
