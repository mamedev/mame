// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************
 *
 * video/gb_lcd.h
 *
 ****************************************************************************/

#ifndef MAME_VIDEO_GB_LCD_H
#define MAME_VIDEO_GB_LCD_H

#pragma once

#include "cpu/lr35902/lr35902.h"



class dmg_ppu_device :  public device_t, public device_video_interface
{
public:
	template <typename T>
	dmg_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag)
		: dmg_ppu_device(mconfig, tag, owner, u32(0))
	{
		set_lr35902_tag(std::forward<T>(cpu_tag));
	}
	dmg_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_lr35902_tag(T &&tag) { m_lr35902.set_tag(std::forward<T>(tag)); }

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	uint8_t oam_r(offs_t offset);
	void oam_w(offs_t offset, uint8_t data);
	virtual uint8_t video_r(offs_t offset);
	virtual void video_w(offs_t offset, uint8_t data);

	virtual void update_state();

protected:
	enum {
		// bits in LCD Control register
		ENABLED = 0x80,
		WINDOW_ENABLED = 0x20,
		LARGE_SPRITES = 0x04,
		SPRITES_ENABLED = 0x02,
		BACKGROUND_ENABLED = 0x01,
		// bits in LCD Status register
		LY_LYC_INT_ENABLED = 0x40,
		MODE_2_INT_ENABLED = 0x20,
		MODE_1_INT_ENABLED = 0x10,
		MODE_0_INT_ENABLED = 0x08,
		LY_LYC_FLAG = 0x04
	};

	struct layer_struct {
		uint8_t  enabled;
		uint8_t  *bg_tiles;
		uint8_t  *bg_map;
		uint8_t  xindex;
		uint8_t  xshift;
		uint8_t  xstart;
		uint8_t  xend;
		/* GBC specific */
		uint8_t  *gbc_map;
		int16_t  bgline;
	};

	inline void plot_pixel(int x, int y, uint16_t color);

	void select_sprites();
	void calculate_window_cycles();
	virtual void update_sprites();
	virtual void update_scanline(uint32_t cycles_to_go);

	dmg_ppu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t vram_size);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_tick);

	void common_start();
	void common_reset();

	required_device<lr35902_cpu_device> m_lr35902;

	address_space *m_program_space;

	// state variables
	bitmap_ind16 m_bitmap;

	uint8_t m_sgb_atf_data[4096];       /* (SGB) Attributes files 4050 bytes, but it's in WRAM, because 4k is transferred */
	uint32_t m_sgb_atf;
	uint16_t m_sgb_pal_data[4096];
	uint8_t m_sgb_pal_map[20][18];
	uint16_t m_sgb_pal[128];
	std::unique_ptr<uint8_t[]> m_sgb_tile_data;
	uint8_t m_sgb_tile_map[2048];
	uint8_t m_sgb_window_mask;

	int m_window_lines_drawn;

	static constexpr unsigned NR_GB_VID_REGS = 0x40;

	uint8_t   m_vid_regs[NR_GB_VID_REGS];
	uint8_t   m_bg_zbuf[160];

	uint16_t  m_cgb_bpal[32];   /* CGB current background palette table */
	uint16_t  m_cgb_spal[32];   /* CGB current sprite palette table */

	uint16_t  m_gb_bpal[4];     /* Background palette */
	uint16_t  m_gb_spal0[4];    /* Sprite 0 palette */
	uint16_t  m_gb_spal1[4];    /* Sprite 1 palette */

	/* WIP Things used to render current line */
	struct {
		// Background/window data
		uint8_t tile_cycle;
		uint8_t tile_count;
		uint8_t y;
		uint16_t pattern_address;
		uint8_t pattern;
		uint16_t tile_address;
		uint8_t plane0;
		uint8_t plane1;
		uint16_t shift_register;

		// Sprite data
		struct {
			bool enabled;
			uint8_t x;
			uint8_t y;
			uint8_t pattern;
			uint8_t flags;
			uint8_t tile_plane_0;
			uint8_t tile_plane_1;
		} sprite[10];
		uint8_t sprite_delay_cycles;
		// other internal data
		bool starting;       // Initial fetches when (re)starting the rendering engine.
		uint8_t sequence_counter;
		bool drawing;
		bool start_drawing;
		uint8_t scrollx_delay;
		uint8_t scrollx_to_apply;
		uint8_t pixels_drawn;
		uint16_t window_compare_position;
		bool window_active;
		uint8_t scrollx;
		// To keep track of when changes to WNDPOSY/WNDPOSX should kick in
		uint8_t window_start_y[16];
		uint8_t window_start_x[16];
		int window_start_y_index;
		// To keep track of when changes to LCDCONT should kick in for window
		uint8_t window_enable[16];
		int window_enable_index;
		bool window_should_trigger;
	} m_line;
	bool m_frame_window_active;

	int m_current_line;       /* Current line */
	int m_cmp_line;           /* Compare line */
	int m_sprCount;           /* Number of sprites on current line */
	int m_sprite[10];         /* References to sprites to draw on current line */
	int m_previous_line;      /* Previous line we've drawn in */
	int m_start_x;            /* Pixel to start drawing from (inclusive) */
	int m_end_x;              /* Pixel to end drawing (exclusive) */
	int m_mode;               /* Keep track of internal STAT mode */
	int m_state;              /* Current state of the video state machine */
	int m_sprite_cycles;
	int m_window_cycles;
	int m_scrollx_adjust;
	int m_oam_locked;
	int m_oam_locked_reading;
	int m_vram_locked;
	int m_pal_locked;
	int m_hdma_enabled;
	int m_hdma_possible;
	int m_hdma_cycles_to_start;
	uint16_t m_hdma_length;
	struct layer_struct m_layer[2];
	emu_timer *m_lcd_timer;
	int m_oam_dma_start_cycles;
	int m_oam_dma_cycles_left;
	uint16_t m_oam_dma_source_address;
	int m_gbc_mode;
	uint8_t m_window_x;
	uint8_t m_window_y;
	uint8_t m_old_curline;
	// Interrupt related
	bool m_stat_mode0_int;
	bool m_stat_mode1_int;
	bool m_stat_mode2_int;
	bool m_stat_lyc_int;
	bool m_stat_lyc_int_prev;
	bool m_stat_write_int;
	bool m_stat_int;

	std::unique_ptr<uint8_t[]>   m_vram;     // Pointer to VRAM
	std::unique_ptr<uint8_t[]>   m_oam;      // Pointer to OAM memory
	bool    m_oam_dma_processing;
	uint8_t   m_gb_tile_no_mod;
	uint32_t  m_gb_chrgen_offs;     // GB Character generator
	uint32_t  m_gb_bgdtab_offs;     // GB Background character table
	uint32_t  m_gb_wndtab_offs;     // GB Window character table
	uint32_t  m_gbc_chrgen_offs;    // CGB Character generator
	uint32_t  m_gbc_bgdtab_offs;    // CGB Background character table
	uint32_t  m_gbc_wndtab_offs;    // CGB Window character table
	int     m_vram_bank;

	attotime m_last_updated;
	uint64_t m_cycles_left;
	int m_next_state;
	bool m_updating_state;
	bool m_enable_experimental_engine;

	virtual void videoptr_restore();
	virtual bool stat_write(uint8_t new_data);
	void increment_scanline();
	void lcd_switch_on(uint8_t new_data);
	void update_oam_dma_state(uint64_t cycles);
	void check_stat_irq();
	void clear_line_state();
	void update_line_state(uint64_t cycles);
	void check_start_of_window();

private:
	const uint32_t m_oam_size;
	const uint32_t m_vram_size;
};


class mgb_ppu_device : public dmg_ppu_device
{
public:
	template <typename T>
	mgb_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag)
		: mgb_ppu_device(mconfig, tag, owner, u32(0))
	{
		set_lr35902_tag(std::forward<T>(cpu_tag));
	}
	mgb_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};


class sgb_ppu_device : public dmg_ppu_device
{
public:
	template <typename T>
	sgb_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag)
		: sgb_ppu_device(mconfig, tag, owner, u32(0))
	{
		set_lr35902_tag(std::forward<T>(cpu_tag));
	}
	sgb_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void sgb_io_write_pal(int offs, uint8_t *data);

protected:

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void update_sprites() override;
	virtual void update_scanline(uint32_t cycles_to_go) override;
	void refresh_border();

	void sgb_vram_memcpy(uint8_t *dst, uint8_t start, size_t num_tiles);
};


class cgb_ppu_device : public dmg_ppu_device
{
public:
	template <typename T>
	cgb_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag)
		: cgb_ppu_device(mconfig, tag, owner, u32(0))
	{
		set_lr35902_tag(std::forward<T>(cpu_tag));
	}
	cgb_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t video_r(offs_t offset) override;
	virtual void video_w(offs_t offset, uint8_t data) override;

protected:

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void update_sprites() override;
	virtual void update_scanline(uint32_t cycles_to_go) override;

	virtual void update_state() override;
	virtual void videoptr_restore() override;
	virtual bool stat_write(uint8_t new_data) override;
	void update_hdma_state(uint64_t cycles);
	void hdma_trans(uint16_t length);
	void hdma_trans_execute();
};


DECLARE_DEVICE_TYPE(DMG_PPU, dmg_ppu_device)
DECLARE_DEVICE_TYPE(MGB_PPU, mgb_ppu_device)
DECLARE_DEVICE_TYPE(SGB_PPU, sgb_ppu_device)
DECLARE_DEVICE_TYPE(CGB_PPU, cgb_ppu_device)

#endif // MAME_VIDEO_GB_LCD_H
