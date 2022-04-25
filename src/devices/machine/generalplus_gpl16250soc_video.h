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
	void set_legacy_video_mode() { m_use_legacy_mode = true; }

	void set_video_spaces(address_space& cpuspace, address_space& cs_space, int csbase) { m_cpuspace = &cpuspace; m_cs_space = &cs_space; m_csbase = csbase; }

	//void set_pal_sprites(int pal_sprites) { m_pal_sprites = pal_sprites; }
	//void set_pal_back(int pal_back) { m_pal_back = pal_back; }

	uint16_t tmap0_regs_r(offs_t offset);
	void tmap0_regs_w(offs_t offset, uint16_t data);
	uint16_t tmap0_tilebase_lsb_r();
	uint16_t tmap0_tilebase_msb_r();
	void tmap0_tilebase_lsb_w(uint16_t data);
	void tmap0_tilebase_msb_w(uint16_t data);

	uint16_t tmap1_regs_r(offs_t offset);
	void tmap1_regs_w(offs_t offset, uint16_t data);
	uint16_t tmap1_tilebase_lsb_r();
	uint16_t tmap1_tilebase_msb_r();
	void tmap1_tilebase_lsb_w(uint16_t data);
	void tmap1_tilebase_msb_w(uint16_t data);

	uint16_t tmap2_regs_r(offs_t offset);
	void tmap2_regs_w(offs_t offset, uint16_t data);
	uint16_t tmap2_tilebase_lsb_r();
	uint16_t tmap2_tilebase_msb_r();
	void tmap2_tilebase_lsb_w(uint16_t data);
	void tmap2_tilebase_msb_w(uint16_t data);

	uint16_t tmap3_regs_r(offs_t offset);
	void tmap3_regs_w(offs_t offset, uint16_t data);
	uint16_t tmap3_tilebase_lsb_r();
	uint16_t tmap3_tilebase_msb_r();
	void tmap3_tilebase_lsb_w(uint16_t data);
	void tmap3_tilebase_msb_w(uint16_t data);

	void video_701c_w(uint16_t data);
	void video_701d_w(uint16_t data);
	void video_701e_w(uint16_t data);

	uint16_t sprite_7022_gfxbase_lsb_r();
	uint16_t sprite_702d_gfxbase_msb_r();

	void sprite_7022_gfxbase_lsb_w(uint16_t data);
	void sprite_702d_gfxbase_msb_w(uint16_t data);
	uint16_t sprite_7042_extra_r();
	void sprite_7042_extra_w(uint16_t data);

	void video_dma_source_w(uint16_t data);
	void video_dma_dest_w(uint16_t data);
	uint16_t video_dma_size_busy_r();
	void video_dma_size_trigger_w(address_space &space, uint16_t data);
	void video_707e_spritebank_w(uint16_t data);

	uint16_t video_703a_palettebank_r();
	void video_703a_palettebank_w(uint16_t data);

	void update_raster_split_position();
	void split_irq_xpos_w(uint16_t data);
	void split_irq_ypos_w(uint16_t data);

	uint16_t videoirq_source_enable_r();
	void videoirq_source_enable_w(uint16_t data);

	uint16_t video_7063_videoirq_source_r();
	void video_7063_videoirq_source_ack_w(uint16_t data);

	void video_702a_w(uint16_t data);
	uint16_t video_7030_brightness_r();
	void video_7030_brightness_w(uint16_t data);
	uint16_t video_curline_r();

	uint16_t video_703c_tvcontrol1_r();
	void video_703c_tvcontrol1_w(uint16_t data);

	uint16_t video_707c_r();

	uint16_t video_707f_r();
	void video_707f_w(uint16_t data);

	void video_7080_w(uint16_t data);
	void video_7081_w(uint16_t data);
	void video_7082_w(uint16_t data);
	void video_7083_w(uint16_t data);
	void video_7084_w(uint16_t data);
	void video_7085_w(uint16_t data);
	void video_7086_w(uint16_t data);
	void video_7087_w(uint16_t data);
	void video_7088_w(uint16_t data);

	uint16_t video_7083_r();

	uint16_t palette_r(offs_t offset);
	void palette_w(offs_t offset, uint16_t data);

	void spriteram_w(offs_t offset, uint16_t data);
	uint16_t spriteram_r(offs_t offset);

	uint16_t video_7051_r();
	uint16_t video_70e0_prng_r();

	auto write_video_irq_callback() { return m_video_irq_cb.bind(); }

	virtual void device_add_mconfig(machine_config& config) override;

protected:

	static const device_timer_id TIMER_SCREENPOS = 2;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	inline void check_video_irq();


	virtual void device_start() override;
	virtual void device_reset() override;

	required_device<unsp_device> m_cpu;
	required_device<screen_device> m_screen;

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

	uint16_t m_spriteram[0x400*2];
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
	bool m_use_legacy_mode; // could be related to the 'unused' bits in the palete bank select being set, but uncertain

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
