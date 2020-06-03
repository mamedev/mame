// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood
/*****************************************************************************

    SunPlus GCM394-series SoC peripheral emulation (Video)

**********************************************************************/

#ifndef MAME_MACHINE_GENERALPLUS_GPL16250SOC_VIDEO_H
#define MAME_MACHINE_GENERALPLUS_GPL16250SOC_VIDEO_H

#pragma once

#include "spg_renderer.h"
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

	void write_tmap_scroll(int tmap, uint16_t* regs, int offset, uint16_t data);
	void write_tmap_extrascroll(int tmap, uint16_t* regs, int offset, uint16_t data);

	void write_tmap_regs(int tmap, uint16_t* regs, int offset, uint16_t data);

	//void set_paldisplaybank_high(int pal_displaybank_high) { m_pal_displaybank_high = pal_displaybank_high; }
	void set_alt_tile_addressing(int alt_tile_addressing) { m_alt_tile_addressing = alt_tile_addressing; }
	void set_alt_extrasprite(int alt_extrasprite_hack) { m_alt_extrasprite_hack = alt_extrasprite_hack; }


	void set_video_spacees(address_space& cpuspace, address_space& cs_space, int csbase) { m_cpuspace = &cpuspace; m_cs_space = &cs_space; m_csbase = csbase; }

	//void set_pal_sprites(int pal_sprites) { m_pal_sprites = pal_sprites; }
	//void set_pal_back(int pal_back) { m_pal_back = pal_back; }

	DECLARE_READ16_MEMBER(tmap0_regs_r);
	DECLARE_WRITE16_MEMBER(tmap0_regs_w);
	DECLARE_READ16_MEMBER(tmap0_tilebase_lsb_r);
	DECLARE_READ16_MEMBER(tmap0_tilebase_msb_r);
	DECLARE_WRITE16_MEMBER(tmap0_tilebase_lsb_w);
	DECLARE_WRITE16_MEMBER(tmap0_tilebase_msb_w);

	DECLARE_READ16_MEMBER(tmap1_regs_r);
	DECLARE_WRITE16_MEMBER(tmap1_regs_w);
	DECLARE_READ16_MEMBER(tmap1_tilebase_lsb_r);
	DECLARE_READ16_MEMBER(tmap1_tilebase_msb_r);
	DECLARE_WRITE16_MEMBER(tmap1_tilebase_lsb_w);
	DECLARE_WRITE16_MEMBER(tmap1_tilebase_msb_w);

	DECLARE_READ16_MEMBER(tmap2_regs_r);
	DECLARE_WRITE16_MEMBER(tmap2_regs_w);
	DECLARE_READ16_MEMBER(tmap2_tilebase_lsb_r);
	DECLARE_READ16_MEMBER(tmap2_tilebase_msb_r);
	DECLARE_WRITE16_MEMBER(tmap2_tilebase_lsb_w);
	DECLARE_WRITE16_MEMBER(tmap2_tilebase_msb_w);

	DECLARE_READ16_MEMBER(tmap3_regs_r);
	DECLARE_WRITE16_MEMBER(tmap3_regs_w);
	DECLARE_READ16_MEMBER(tmap3_tilebase_lsb_r);
	DECLARE_READ16_MEMBER(tmap3_tilebase_msb_r);
	DECLARE_WRITE16_MEMBER(tmap3_tilebase_lsb_w);
	DECLARE_WRITE16_MEMBER(tmap3_tilebase_msb_w);

	DECLARE_WRITE16_MEMBER(video_701c_w);
	DECLARE_WRITE16_MEMBER(video_701d_w);
	DECLARE_WRITE16_MEMBER(video_701e_w);

	DECLARE_READ16_MEMBER(sprite_7022_gfxbase_lsb_r);
	DECLARE_READ16_MEMBER(sprite_702d_gfxbase_msb_r);

	DECLARE_WRITE16_MEMBER(sprite_7022_gfxbase_lsb_w);
	DECLARE_WRITE16_MEMBER(sprite_702d_gfxbase_msb_w);
	DECLARE_READ16_MEMBER(sprite_7042_extra_r);
	DECLARE_WRITE16_MEMBER(sprite_7042_extra_w);

	DECLARE_WRITE16_MEMBER(video_dma_source_w);
	DECLARE_WRITE16_MEMBER(video_dma_dest_w);
	DECLARE_READ16_MEMBER(video_dma_size_busy_r);
	DECLARE_WRITE16_MEMBER(video_dma_size_trigger_w);
	DECLARE_WRITE16_MEMBER(video_707e_spritebank_w);

	DECLARE_READ16_MEMBER(video_703a_palettebank_r);
	DECLARE_WRITE16_MEMBER(video_703a_palettebank_w);

	void update_raster_split_position();
	DECLARE_WRITE16_MEMBER(split_irq_xpos_w);
	DECLARE_WRITE16_MEMBER(split_irq_ypos_w);

	DECLARE_READ16_MEMBER(videoirq_source_enable_r);
	DECLARE_WRITE16_MEMBER(videoirq_source_enable_w);

	DECLARE_READ16_MEMBER(video_7063_videoirq_source_r);
	DECLARE_WRITE16_MEMBER(video_7063_videoirq_source_ack_w);

	DECLARE_WRITE16_MEMBER(video_702a_w);
	DECLARE_READ16_MEMBER(video_7030_brightness_r);
	DECLARE_WRITE16_MEMBER(video_7030_brightness_w);
	DECLARE_READ16_MEMBER(video_curline_r);

	DECLARE_READ16_MEMBER(video_703c_tvcontrol1_r);
	DECLARE_WRITE16_MEMBER(video_703c_tvcontrol1_w);

	DECLARE_READ16_MEMBER(video_707c_r);

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

	DECLARE_WRITE16_MEMBER(spriteram_w);
	DECLARE_READ16_MEMBER(spriteram_r);

	DECLARE_READ16_MEMBER(video_7051_r);
	DECLARE_READ16_MEMBER(video_70e0_r);

	auto write_video_irq_callback() { return m_video_irq_cb.bind(); };

	virtual void device_add_mconfig(machine_config& config) override;

protected:

	static const device_timer_id TIMER_SCREENPOS = 2;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

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
	void draw(const rectangle &cliprect, uint32_t line, uint32_t xoff, uint32_t yoff, uint32_t bitmap_addr, uint32_t tile, int32_t h, int32_t w, uint8_t bpp, uint32_t yflipmask, uint32_t palette_offset, int addressing_mode);
	void draw_page(const rectangle &cliprect, uint32_t scanline, int priority, uint32_t bitmap_addr, uint16_t *regs, uint16_t *scroll, int which);
	void draw_sprites(const rectangle& cliprect, uint32_t scanline, int priority);
	void draw_sprite(const rectangle& cliprect, uint32_t scanline, int priority, uint32_t base_addr);

	uint32_t m_screenbuf[640 * 480];
	uint8_t m_rgb5_to_rgb8[32];
	uint32_t m_rgb555_to_rgb888[0x8000];

	required_device<unsp_device> m_cpu;
	required_device<screen_device> m_screen;
 // required_shared_ptr<uint16_t> m_scrollram;

	uint16_t m_page0_addr_lsb;
	uint16_t m_page0_addr_msb;

	uint16_t m_page1_addr_lsb;
	uint16_t m_page1_addr_msb;

	uint16_t m_707e_spritebank;
	uint16_t m_videodma_size;
	uint16_t m_videodma_dest;
	uint16_t m_videodma_source;

	devcb_write_line m_video_irq_cb;


	// video 70xx
	uint16_t m_tmap0_regs[0x4];
	uint16_t m_tmap1_regs[0x4];

	uint16_t m_tmap2_regs[0x4];
	uint16_t m_tmap3_regs[0x4];


	uint16_t m_tmap0_scroll[0x2];
	uint16_t m_tmap1_scroll[0x2];

	uint16_t m_tmap2_scroll[0x4];
	uint16_t m_tmap3_scroll[0x4];


	uint16_t m_707f;
	uint16_t m_703a_palettebank;
	uint16_t m_video_irq_enable;
	uint16_t m_video_irq_status;

	uint16_t m_702a;
	uint16_t m_7030_brightness;
	uint16_t m_xirqpos;
	uint16_t m_yirqpos;
	uint16_t m_703c_tvcontrol1;

	uint16_t m_7042_sprite;

	uint16_t m_7080;
	uint16_t m_7081;
	uint16_t m_7082;
	uint16_t m_7083;
	uint16_t m_7084;
	uint16_t m_7085;
	uint16_t m_7086;
	uint16_t m_7087;
	uint16_t m_7088;

	uint16_t m_sprite_7022_gfxbase_lsb;
	uint16_t m_sprite_702d_gfxbase_msb;
	uint16_t m_page2_addr_lsb;
	uint16_t m_page2_addr_msb;
	uint16_t m_page3_addr_lsb;
	uint16_t m_page3_addr_msb;

	void unk_vid_regs_w(int which, int offset, uint16_t data);

	emu_timer *m_screenpos_timer;

	uint16_t m_spriteram[0x400];
	uint16_t m_spriteextra[0x400];
	uint16_t m_paletteram[0x100*0x10];

	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	devcb_read16 m_space_read_cb;

	required_shared_ptr<uint16_t> m_rowscroll;
	required_shared_ptr<uint16_t> m_rowzoom;

	int m_maxgfxelement;
	void decodegfx(const char* tag);

	//int m_pal_displaybank_high;
	//int m_pal_sprites;
	//int m_pal_back;
	int m_alt_extrasprite_hack;
	int m_alt_tile_addressing;

	required_device<spg_renderer_device> m_renderer;

	address_space* m_cpuspace;
	address_space* m_cs_space;
	int m_csbase;
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

#endif // MAME_MACHINE_GENERALPLUS_GPL16250SOC_VIDEO_H
