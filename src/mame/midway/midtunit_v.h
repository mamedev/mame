// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles
// thanks-to:Kurt Mahan
/*************************************************************************

    Video Emulation for Midway T-unit, W-unit, and X-unit games.

**************************************************************************/

#ifndef MAME_MIDWAY_MIDTUNIT_V_H
#define MAME_MIDWAY_MIDTUNIT_V_H

#pragma once

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "emupal.h"

#define DEBUG_MIDTUNIT_BLITTER      (0)

class midtunit_video_device : public device_t
{
public:
	// construction/destruction
	template <typename T>
	midtunit_video_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&palette_tag)
		: midtunit_video_device(mconfig, tag, owner, (uint32_t)0)
	{
		m_palette.set_tag(std::forward<T>(palette_tag));
	}

	midtunit_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration
	auto dma_irq_cb() { return m_dma_irq_cb.bind(); }
	void set_gfx_rom_large(bool gfx_rom_large) { m_gfx_rom_large = gfx_rom_large; }

	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);

	uint16_t midtunit_vram_r(offs_t offset);
	void midtunit_vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t midtunit_gfxrom_r(offs_t offset);
	void midtunit_vram_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void midtunit_vram_color_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t midtunit_vram_data_r(offs_t offset);
	uint16_t midtunit_vram_color_r(offs_t offset);

	uint16_t dma_r(offs_t offset);
	void dma_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void midtunit_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	enum op_type_t
	{
		PIXEL_SKIP  = 0,
		PIXEL_COLOR = 1,
		PIXEL_COPY  = 2
	};

protected:
	// construction/destruction
	midtunit_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(dma_done);

	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_gfxrom;

	emu_timer *m_dma_timer;

	// constants for the DMA chip
	static constexpr uint32_t XPOSMASK = 0x3ff;
	static constexpr uint32_t YPOSMASK = 0x1ff;

	template <int BitsPerPixel, bool XFlip, bool Skip, bool Scale, op_type_t Zero, op_type_t NonZero> void dma_draw();
	void dma_draw_none() {}

	typedef void (midtunit_video_device::*draw_func)();
	draw_func m_dma_draw_skip_scale[8*32];
	draw_func m_dma_draw_noskip_scale[8*32];
	draw_func m_dma_draw_skip_noscale[8*32];
	draw_func m_dma_draw_noskip_noscale[8*32];

	enum
	{
		DMA_LRSKIP = 0,
		DMA_COMMAND,
		DMA_OFFSETLO,
		DMA_OFFSETHI,
		DMA_XSTART,
		DMA_YSTART,
		DMA_WIDTH,
		DMA_HEIGHT,
		DMA_PALETTE,
		DMA_COLOR,
		DMA_SCALE_X,
		DMA_SCALE_Y,
		DMA_TOPCLIP,
		DMA_BOTCLIP,
		DMA_UNKNOWN_E,  // MK1/2 never write here; NBA only writes 0
		DMA_CONFIG,
		DMA_LEFTCLIP,   // pseudo-register
		DMA_RIGHTCLIP   // pseudo-register
	};

	// graphics-related variables
	uint16_t    m_midtunit_control;
	bool        m_gfx_rom_large;

	// videoram-related variables
	uint32_t    m_gfxbank_offset[2];
	std::unique_ptr<uint16_t[]> m_local_videoram;
	uint8_t     m_videobank_select;

	// DMA-related variables
	uint16_t    m_dma_register[18];
	struct dma_state
	{
		uint8_t *     gfxrom;

		uint32_t      offset;         // source offset, in bits
		int32_t       rowbits;        // source bits to skip each row
		int32_t       xpos;           // x position, clipped
		int32_t       ypos;           // y position, clipped
		int32_t       width;          // horizontal pixel count
		int32_t       height;         // vertical pixel count
		uint16_t      palette;        // palette base
		uint16_t      color;          // current foreground color with palette

		uint8_t       yflip;          // yflip?
		uint8_t       preskip;        // preskip scale
		uint8_t       postskip;       // postskip scale
		int32_t       topclip;        // top clipping scanline
		int32_t       botclip;        // bottom clipping scanline
		int32_t       leftclip;       // left clipping column
		int32_t       rightclip;      // right clipping column
		int32_t       startskip;      // pixels to skip at start
		int32_t       endskip;        // pixels to skip at end
		uint16_t      xstep;          // 8.8 fixed number scale x factor
		uint16_t      ystep;          // 8.8 fixed number scale y factor
	};
	dma_state   m_dma_state;

	devcb_write_line m_dma_irq_cb;

#if DEBUG_MIDTUNIT_BLITTER
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	uint32_t debug_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void do_debug_blit();
	void do_dma_debug_inputs();

	required_device<palette_device> m_debug_palette;
	std::unique_ptr<uint16_t[]> m_debug_videoram;
	bool m_dma_debug;
	bool m_doing_debug_dma;
	dma_state m_debug_dma_state;
	int32_t m_debug_dma_bpp;
	int32_t m_debug_dma_mode;
	int32_t m_debug_dma_command;
#endif

	std::string m_log_path;
	bool m_log_png;
	bool m_log_json;
	std::unique_ptr<uint64_t[]> m_logged_rom;
	bitmap_argb32 m_log_bitmap;

	void debug_init();
	void debug_commands(const std::vector<std::string_view> &params);
	void debug_help_command(const std::vector<std::string_view> &params);
	void debug_png_dma_command(const std::vector<std::string_view> &params);
	void log_bitmap(int command, int bpp, bool skip);
};

class midwunit_video_device : public midtunit_video_device
{
public:
	// construction/destruction
	template <typename T>
	midwunit_video_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&palette_tag)
		: midwunit_video_device(mconfig, tag, owner, (uint32_t)0)
	{
		m_palette.set_tag(std::forward<T>(palette_tag));
	}
	midwunit_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint16_t midwunit_gfxrom_r(offs_t offset);

	void midwunit_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t midwunit_control_r();

protected:
	midwunit_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

#if DEBUG_MIDTUNIT_BLITTER
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
#endif
};

class midxunit_video_device : public midwunit_video_device
{
public:
	// construction/destruction
	template <typename T>
	midxunit_video_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&palette_tag)
		: midwunit_video_device(mconfig, tag, owner, (uint32_t)0)
	{
		m_palette.set_tag(std::forward<T>(palette_tag));
	}

	midxunit_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void midxunit_paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t midxunit_paletteram_r(offs_t offset);

	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);

protected:
	virtual void device_start() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(MIDTUNIT_VIDEO, midtunit_video_device)
DECLARE_DEVICE_TYPE(MIDWUNIT_VIDEO, midwunit_video_device)
DECLARE_DEVICE_TYPE(MIDXUNIT_VIDEO, midxunit_video_device)

#endif // MAME_MIDWAY_MIDTUNIT_V_H
