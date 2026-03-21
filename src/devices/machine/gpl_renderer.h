// license:BSD-3-Clause
// copyright-holders:David Haywood, Ryan Holtz

#ifndef MAME_MACHINE_GPL_RENDERER_H
#define MAME_MACHINE_GPL_RENDERER_H

#pragma once

#include "screen.h"

class gpl_renderer_device : public device_t
{
public:
	gpl_renderer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto space_read_callback() { return m_space_read_cb.bind(); }
	template <typename T> void set_video_space(T &&tag, int no) { m_cpuspace.set_tag(std::forward<T>(tag), no); }
	template <typename T> void set_cs_video_space(T &&tag, int no, uint32_t csbase) { m_cs_space.set_tag(std::forward<T>(tag), no); m_csbase = csbase; }

	void draw_sprites(bool read_from_csspace, int extended_sprites_mode, uint32_t palbank, bool highres, const rectangle &cliprect, uint32_t scanline, int priority, uint32_t spritegfxdata_addr, address_space &spc, uint16_t *paletteram, uint16_t *spriteram);
	void draw_page(bool read_from_csspace, uint32_t palbank, const rectangle &cliprect, uint32_t scanline, int priority, uint16_t tilegfxdata_addr_msb, uint16_t tilegfxdata_addr, uint16_t *scrollregs, uint16_t *tilemapregs, address_space &spc, uint16_t *paletteram, uint16_t *scrollram, uint32_t which);
	void new_line(const rectangle &cliprect);

	void apply_saturation_and_fade(bitmap_rgb32 &bitmap, const rectangle &cliprect, int scanline);

	void set_video_reg_1c(uint16_t val) { m_video_regs_1c = val; update_vcmp_table(); }
	void set_video_reg_1d(uint16_t val) { m_video_regs_1d = val; update_vcmp_table(); }
	void set_video_reg_1e(uint16_t val) { m_video_regs_1e = val; update_vcmp_table(); }
	void set_video_reg_2a(uint16_t val) { m_video_regs_2a = val; }
	void set_video_reg_30(uint16_t val)
	{
		if (m_video_regs_30 != val)
			m_brightness_or_saturation_dirty = true;

		m_video_regs_30 = val;
	}
	void set_video_reg_3c(uint16_t val)
	{
		if (m_video_regs_3c != val)
			m_brightness_or_saturation_dirty = true;

		m_video_regs_3c = val;
	}

	uint16_t get_video_reg_1c() { return m_video_regs_1c; }
	uint16_t get_video_reg_1d() { return m_video_regs_1d; }
	uint16_t get_video_reg_1e() { return m_video_regs_1e; }
	uint16_t get_video_reg_2a() { return m_video_regs_2a; }
	uint16_t get_video_reg_30() { return m_video_regs_30; }
	uint16_t get_video_reg_3c() { return m_video_regs_3c; }

	void set_video_reg_42(uint16_t val) { m_video_regs_42 = val; }
	uint16_t get_video_reg_42() { return m_video_regs_42; }

	void set_video_reg_7f(uint16_t val) { m_video_regs_7f = val; }
	uint16_t get_video_reg_7f() { return m_video_regs_7f; }

protected:
	gpl_renderer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	template<bool Blend, bool FlipX>
	inline void draw_tilestrip(bool read_from_csspace, uint32_t screenwidth, uint32_t drawwidthmask, const rectangle &cliprect, uint32_t tile_h, uint32_t tile_w, uint32_t tilegfxdata_addr, uint32_t tile, uint32_t tile_scanline, int drawx, bool flip_y, uint32_t palette_offset, const uint32_t nc_bpp, const uint32_t bits_per_row, const uint32_t words_per_tile, address_space &spc, uint16_t *palette, uint8_t blendlevel);
	inline void draw_tilestrip(bool read_from_csspace, uint32_t screenwidth, uint32_t drawwidthmask, bool blend, bool flip_x, const rectangle &cliprect, uint32_t tile_h, uint32_t tile_w, uint32_t tilegfxdata_addr, uint32_t tile, uint32_t tile_scanline, int drawx, bool flip_y, uint32_t palette_offset, const uint32_t nc_bpp, const uint32_t bits_per_row, const uint32_t words_per_tile, address_space &spc, uint16_t *paletteram, uint8_t blendlevel);
	inline void draw_sprite(bool read_from_csspace, int extended_sprites_mode, uint32_t palbank, bool highres, const rectangle &cliprect, uint32_t scanline, int priority, uint32_t spritegfxdata_addr, uint32_t base_addr, address_space &spc, uint16_t *paletteram, uint16_t *spriteram);
	virtual void draw_linemap(const rectangle &cliprect, uint32_t scanline, int priority, uint32_t tilegfxdata_addr, uint16_t *scrollregs, uint16_t *tilemapregs, address_space &spc, uint16_t *paletteram);
	inline uint8_t mix_channel(uint8_t a, uint8_t b, uint8_t alpha);
	void update_vcmp_table();
	void update_palette_lookup();

	// config
	devcb_read16 m_space_read_cb;
	required_address_space m_cpuspace;
	optional_address_space m_cs_space;
	uint32_t m_csbase;

	uint8_t m_rgb5_to_rgb8[32];
	std::unique_ptr<uint32_t []> m_rgb555_to_rgb888;
	std::unique_ptr<uint32_t []> m_rgb555_to_rgb888_current;

	// for vcmp
	uint16_t m_video_regs_1c;
	uint16_t m_video_regs_1d;
	uint16_t m_video_regs_1e;

	uint16_t m_video_regs_2a;

	uint16_t m_video_regs_30;
	uint16_t m_video_regs_3c;

	uint16_t m_video_regs_42;

	uint16_t m_video_regs_7f; // new on GPL renderer

	uint32_t m_ycmp_table[480];

	bool m_brightness_or_saturation_dirty;
	uint16_t m_linebuf[640];
};

DECLARE_DEVICE_TYPE(GPL_RENDERER, gpl_renderer_device)

#endif // MAME_MACHINE_GPL_RENDERER_H
