// license:BSD-3-Clause
// copyright-holders:David Haywood, Ryan Holtz

#ifndef MAME_MACHINE_SPG_RENDERER_H
#define MAME_MACHINE_SPG_RENDERER_H

#pragma once

#include "screen.h"

class spg_renderer_device : public device_t
{
public:
	spg_renderer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	spg_renderer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void draw_sprites(const rectangle& cliprect, uint32_t* dst, uint32_t scanline, int priority, address_space& spc, uint16_t* palette, uint16_t* spriteram, int sprlimit);

	void set_video_reg_22(uint16_t val) { m_video_regs_22 = val; }
	void set_video_reg_2a(uint16_t val) { m_video_regs_2a = val; }
	void set_video_reg_42(uint16_t val) { m_video_regs_42 = val; }

	uint16_t get_video_reg_22(void) { return m_video_regs_22; }
	uint16_t get_video_reg_2a(void) { return m_video_regs_2a; }
	uint16_t get_video_reg_42(void) { return m_video_regs_42; }



protected:

	virtual void device_start() override;
	virtual void device_reset() override;

	enum blend_enable_t : const bool
	{
		BlendOff = false,
		BlendOn = true
	};

	enum rowscroll_enable_t : const bool
	{
		RowScrollOff = false,
		RowScrollOn = true
	};

	enum flipx_t : const bool
	{
		FlipXOff = false,
		FlipXOn = true
	};

	template<spg_renderer_device::blend_enable_t Blend, spg_renderer_device::flipx_t FlipX>
	inline void draw_tilestrip(const rectangle& cliprect, uint32_t* dst, uint32_t tile_h, uint32_t tile_w, uint32_t tilegfxdata_addr, uint16_t tile, uint32_t tile_scanline, int drawx, bool flip_y, uint32_t palette_offset, const uint32_t nc_bpp, const uint32_t bits_per_row, const uint32_t words_per_tile, address_space &spc, uint16_t* palette);

	void draw_sprite(const rectangle& cliprect, uint32_t* dst, uint32_t scanline, int priority, uint32_t base_addr, address_space& spc, uint16_t* palette, uint16_t* spriteram);

	uint8_t mix_channel(uint8_t a, uint8_t b);

	uint8_t m_rgb5_to_rgb8[32];
	uint32_t m_rgb555_to_rgb888[0x8000];

private:
	uint16_t m_video_regs_2a;
	uint16_t m_video_regs_22;
	uint16_t m_video_regs_42;
};

DECLARE_DEVICE_TYPE(SPG_RENDERER, spg_renderer_device)


#endif // MAME_MACHINE_SPG_RENDERER_H
