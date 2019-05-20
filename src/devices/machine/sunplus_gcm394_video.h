// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood
/*****************************************************************************

    SunPlus GCM394-series SoC peripheral emulation (Video)

**********************************************************************/

#ifndef MAME_MACHINE_GCM394_VIDEO_H
#define MAME_MACHINE_GCM394_VIDEO_H

#pragma once

#include "cpu/unsp/unsp.h"
#include "screen.h"
#include "emupal.h"

class gcm394_base_video_device : public device_t, public device_video_interface
{
public:
	gcm394_base_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank);

	auto space_read_callback() { return m_space_read_cb.bind(); }

	void write_tmap_regs(int tmap, uint16_t* regs, int offset, uint16_t data);

	DECLARE_READ16_MEMBER(tmap0_regs_r);
	DECLARE_WRITE16_MEMBER(tmap0_regs_w);
	DECLARE_WRITE16_MEMBER(tmap0_unk0_w);
	DECLARE_WRITE16_MEMBER(tmap0_unk1_w);

	DECLARE_READ16_MEMBER(tmap1_regs_r);
	DECLARE_WRITE16_MEMBER(tmap1_regs_w);
	DECLARE_WRITE16_MEMBER(tmap1_unk0_w);
	DECLARE_WRITE16_MEMBER(tmap1_unk1_w);

	DECLARE_WRITE16_MEMBER(unknown_video_device0_regs_w);
	DECLARE_WRITE16_MEMBER(unknown_video_device0_unk0_w);
	DECLARE_WRITE16_MEMBER(unknown_video_device0_unk1_w);

	DECLARE_WRITE16_MEMBER(unknown_video_device1_regs_w);
	DECLARE_WRITE16_MEMBER(unknown_video_device1_unk0_w);
	DECLARE_WRITE16_MEMBER(unknown_video_device1_unk1_w);

	DECLARE_WRITE16_MEMBER(unknown_video_device2_unk0_w);
	DECLARE_WRITE16_MEMBER(unknown_video_device2_unk1_w);
	DECLARE_WRITE16_MEMBER(unknown_video_device2_unk2_w);

	DECLARE_WRITE16_MEMBER(video_dma_source_w);
	DECLARE_WRITE16_MEMBER(video_dma_dest_w);
	DECLARE_READ16_MEMBER(video_dma_size_r);
	DECLARE_WRITE16_MEMBER(video_dma_size_w);
	DECLARE_WRITE16_MEMBER(video_dma_unk_w);

	DECLARE_READ16_MEMBER(video_703a_r);
	DECLARE_WRITE16_MEMBER(video_703a_w);

	DECLARE_READ16_MEMBER(video_7062_r);
	DECLARE_WRITE16_MEMBER(video_7062_w);

	DECLARE_WRITE16_MEMBER(video_7063_w);

	DECLARE_WRITE16_MEMBER(video_702a_w);
	DECLARE_READ16_MEMBER(video_7030_r);
	DECLARE_WRITE16_MEMBER(video_7030_w);
	DECLARE_WRITE16_MEMBER(video_703c_w);


	DECLARE_READ16_MEMBER(video_707f_r);
	DECLARE_WRITE16_MEMBER(video_707f_w);

	DECLARE_WRITE16_MEMBER(video_7080_w);
	DECLARE_WRITE16_MEMBER(video_7081_w);
	DECLARE_WRITE16_MEMBER(video_7082_w);
	DECLARE_WRITE16_MEMBER(video_7083_w);
	DECLARE_WRITE16_MEMBER(video_7084_w);
	DECLARE_WRITE16_MEMBER(video_7085_w);
	DECLARE_WRITE16_MEMBER(video_7086_w);
	DECLARE_WRITE16_MEMBER(video_7087_w);
	DECLARE_WRITE16_MEMBER(video_7088_w);

	DECLARE_READ16_MEMBER(video_7083_r);
	
	DECLARE_READ16_MEMBER(palette_r);
	DECLARE_WRITE16_MEMBER(palette_w);

	auto write_video_irq_callback() { return m_video_irq_cb.bind(); };

	uint8_t* m_gfxregion;
	uint32_t m_gfxregionsize;

	virtual void device_add_mconfig(machine_config& config) override;

protected:

	enum
	{
		PAGE_ENABLE_MASK        = 0x0008,
		PAGE_WALLPAPER_MASK     = 0x0004,

		SPRITE_ENABLE_MASK      = 0x0001,
		SPRITE_COORD_TL_MASK    = 0x0002,

		PAGE_PRIORITY_FLAG_MASK    = 0x3000,
		PAGE_PRIORITY_FLAG_SHIFT   = 12,
		PAGE_TILE_HEIGHT_MASK   = 0x00c0,
		PAGE_TILE_HEIGHT_SHIFT  = 6,
		PAGE_TILE_WIDTH_MASK    = 0x0030,
		PAGE_TILE_WIDTH_SHIFT   = 4,
		TILE_X_FLIP             = 0x0004,
		TILE_Y_FLIP             = 0x0008
	};


	inline void check_video_irq();


	virtual void device_start() override;
	virtual void device_reset() override;

	enum blend_enable_t : bool
	{
		BlendOff = false,
		BlendOn = true
	};

	enum rowscroll_enable_t : bool
	{
		RowScrollOff = false,
		RowScrollOn = true
	};

	enum flipx_t : bool
	{
		FlipXOff = false,
		FlipXOn = true
	};

	template<blend_enable_t Blend, rowscroll_enable_t RowScroll, flipx_t FlipX>
	void draw(const rectangle &cliprect, uint32_t line, uint32_t xoff, uint32_t yoff, uint32_t bitmap_addr, uint32_t tile, int32_t h, int32_t w, uint8_t bpp, uint32_t yflipmask, uint32_t palette_offset);
	void draw_page(const rectangle &cliprect, uint32_t scanline, int priority, uint32_t bitmap_addr, uint16_t *regs);
	void draw_sprites(const rectangle& cliprect, uint32_t scanline, int priority);
	void draw_sprite(const rectangle& cliprect, uint32_t scanline, int priority, uint32_t base_addr);

	uint32_t m_screenbuf[320 * 240];
	uint8_t m_rgb5_to_rgb8[32];
	uint32_t m_rgb555_to_rgb888[0x8000];

	required_device<unsp_device> m_cpu;
	required_device<screen_device> m_screen;
//	required_shared_ptr<uint16_t> m_scrollram;
	required_shared_ptr<uint16_t> m_spriteram;

	uint16_t m_page1_addr;
	uint16_t m_page2_addr;

	uint16_t m_videodma_bank;
	uint16_t m_videodma_size;
	uint16_t m_videodma_dest;
	uint16_t m_videodma_source;

	devcb_write_line m_video_irq_cb;


	// video 70xx
	uint16_t m_tmap0_regs[0x6];
	uint16_t m_tmap1_regs[0x6];

	uint16_t m_707f;
	uint16_t m_703a;
	uint16_t m_7062;
	uint16_t m_7063;

	uint16_t m_702a;
	uint16_t m_7030;
	uint16_t m_703c;


	uint16_t m_7080;
	uint16_t m_7081;
	uint16_t m_7082;
	uint16_t m_7083;
	uint16_t m_7084;
	uint16_t m_7085;
	uint16_t m_7086;
	uint16_t m_7087;
	uint16_t m_7088;

	uint16_t m_video_irq_status;

	uint16_t m_spriteextra[0x100];
	uint16_t m_paletteram[0x100*0x10];

	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	devcb_read16 m_space_read_cb;
};

class gcm394_video_device : public gcm394_base_video_device
{
public:
	template <typename T, typename U>
	gcm394_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&screen_tag)
		: gcm394_video_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
		m_screen.set_tag(std::forward<U>(screen_tag));
	}

	gcm394_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(GCM394_VIDEO, gcm394_video_device)

#endif // MAME_MACHINE_GCM394_VIDEO_H
