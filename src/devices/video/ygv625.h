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

#include "lrucache.h"


class ygv625_device : public device_t, public device_video_interface
{
public:
	ygv625_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~ygv625_device();

	// configuration: memory region holding the CG (character generator) data
	template <typename T> void set_cg_region(T &&tag) { m_cg.set_tag(std::forward<T>(tag)); }

	// 16-bit CPU bus interface, 0x2000 words
	u16 read(offs_t offset);
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	struct decoded_sprite;
	class bit_reader;
	struct plane_params;

	required_region_ptr<u8> m_cg;
	memory_share_creator<u16> m_ram;       // 0x2000 words: attribute table, quad table, palette RAM

	u16 m_regs[0x200];
	bitmap_rgb32 m_bitmap;
	util::lru_cache_map<u64, decoded_sprite> m_cache;

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
	static void read_overrides(bit_reader &br, plane_params &p, const plane_params &base, u32 flag_layout);
	static bool decode_plane(bit_reader &br, const plane_params &p, int p0, u32 mod, u16 *out);

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
