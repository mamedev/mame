// license:BSD-3-Clause
// copyright-holders:Anthony Kruize, Fabio Priuli
/***************************************************************************

        SNES PPU

***************************************************************************/

#ifndef MAME_VIDEO_SNES_PPU_H
#define MAME_VIDEO_SNES_PPU_H

#pragma once

#include "screen.h"


#define MCLK_NTSC   (21477272)  /* verified */
#define MCLK_PAL    (21218370)  /* verified */

#define DOTCLK_NTSC (MCLK_NTSC/4)
#define DOTCLK_PAL  (MCLK_PAL/4)

#define SNES_SCR_WIDTH        256       /* 32 characters 8 pixels wide */
#define SNES_SCR_HEIGHT_NTSC  225       /* Can be 224 or 240 height */
#define SNES_SCR_HEIGHT_PAL   240       /* ??? */
#define SNES_VTOTAL_NTSC      262       /* Maximum number of lines for NTSC systems */
#define SNES_VTOTAL_PAL       312       /* Maximum number of lines for PAL systems */
#define SNES_HTOTAL           341       /* Maximum number pixels per line (incl. hblank) */

#define SNES_NTSC             0x00
#define SNES_PAL              0x10


#define SNES_LAYER_DEBUG  0


// ======================> snes_ppu_device

class snes_ppu_device :  public device_t,
							public device_video_interface,
							public device_palette_interface
{
public:
	// construction/destruction
	snes_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration helpers
	auto open_bus_callback() { return m_openbus_cb.bind(); }

	void refresh_scanline(bitmap_rgb32 &bitmap, uint16_t curline);

	int16_t current_x() const { return screen().hpos(); }
	int16_t current_y() const { return screen().vpos(); }
	void set_latch_hv(int16_t x, int16_t y);

	uint8_t read(uint32_t offset, uint8_t wrio_bit7);
	void write(uint32_t offset, uint8_t data);

	int vtotal() const { return ((m_stat78 & 0x10) == SNES_NTSC) ? SNES_VTOTAL_NTSC : SNES_VTOTAL_PAL; }
	uint16_t htmult() const { return m_htmult; }
	uint8_t interlace() const { return m_interlace; }
	bool screen_disabled() const { return bool(m_screen_disabled); }
	uint8_t last_visible_line() const { return m_beam.last_visible_line; }
	uint16_t current_vert() const { return m_beam.current_vert; }

	void oam_address_reset();
	void oam_set_first_object();

	void clear_time_range_over() { m_stat77 &= 0x3f; }
	void toggle_field() { m_stat78 ^= 0x80; }
	void reset_interlace()
	{
		m_htmult = 1;
		m_interlace = 1;
		m_oam.interlace = 0;
	}
	void set_current_vert(uint16_t value);

protected:
	/* offset-per-tile modes */
	enum
	{
		SNES_OPT_NONE = 0,
		SNES_OPT_MODE2,
		SNES_OPT_MODE4,
		SNES_OPT_MODE6
	};

	/* layers */
	enum
	{
		SNES_BG1 = 0,
		SNES_BG2,
		SNES_BG3,
		SNES_BG4,
		SNES_OAM,
		SNES_COLOR
	};


	struct SNES_SCANLINE
	{
		int enable, clip;

		uint16_t buffer[SNES_SCR_WIDTH];
		uint8_t  priority[SNES_SCR_WIDTH];
		uint8_t  layer[SNES_SCR_WIDTH];
		uint8_t  blend_exception[SNES_SCR_WIDTH];
	};

	uint8_t m_regs[0x40];

	SNES_SCANLINE m_scanlines[2];

	struct layer_t
	{
		/* clipmasks */
		uint8_t window1_enabled, window1_invert;
		uint8_t window2_enabled, window2_invert;
		uint8_t wlog_mask;
		/* color math enabled */
		uint8_t color_math;

		uint16_t charmap;
		uint16_t tilemap;
		uint8_t tilemap_size;

		uint8_t tile_size;
		uint8_t tile_mode;
		uint8_t mosaic_enabled;   // actually used only for layers 0->3!

		uint8_t main_window_enabled;
		uint8_t sub_window_enabled;
		uint8_t main_bg_enabled;
		uint8_t sub_bg_enabled;

		uint16_t hoffs;
		uint16_t voffs;

		uint8_t priority[2];

		uint16_t mosaic_counter;
		uint16_t mosaic_offset;
	};

	layer_t m_layer[6]; // this is for the BG1 - BG2 - BG3 - BG4 - OBJ - color layers

	struct
	{
		uint16_t address;
		uint16_t base_address;
		uint8_t priority_rotation;
		uint16_t tile_data_address;
		uint8_t name_select;
		uint8_t base_size;
		uint8_t first;
		uint16_t write_latch;
		uint8_t data_latch;
		uint8_t interlace;
		uint8_t priority[4];
	} m_oam;

	struct
	{
		uint16_t latch_horz;
		uint16_t latch_vert;
		uint16_t current_vert;
		uint8_t last_visible_line;
		uint8_t interlace_count;
	} m_beam;

	struct
	{
		uint8_t repeat;
		uint8_t hflip;
		uint8_t vflip;
		int16_t matrix_a;
		int16_t matrix_b;
		int16_t matrix_c;
		int16_t matrix_d;
		int16_t origin_x;
		int16_t origin_y;
		int16_t hor_offset;
		int16_t ver_offset;
		uint8_t extbg;
	} m_mode7;

#if SNES_LAYER_DEBUG
	struct DEBUGOPTS
	{
		uint8_t bg_disabled[5];
		uint8_t mode_disabled[8];
		uint8_t draw_subscreen;
		uint8_t windows_disabled;
		uint8_t mosaic_disabled;
		uint8_t colormath_disabled;
		uint8_t select_pri[5];
	};
	struct DEBUGOPTS m_debug_options;
	uint8_t dbg_video( uint16_t curline );
#endif

	uint8_t m_mosaic_size;
	uint8_t m_clip_to_black;
	uint8_t m_prevent_color_math;
	uint8_t m_sub_add_mode;
	uint8_t m_bg_priority;
	uint8_t m_direct_color;
	uint8_t m_ppu_last_scroll;      /* as per Anomie's doc and Theme Park, all scroll regs shares (but mode 7 ones) the same
	                               'previous' scroll value */
	uint8_t m_mode7_last_scroll;    /* as per Anomie's doc mode 7 scroll regs use a different value, shared with mode 7 matrix! */

	uint8_t m_ppu1_open_bus, m_ppu2_open_bus;
	uint8_t m_ppu1_version, m_ppu2_version;
	uint8_t m_window1_left, m_window1_right, m_window2_left, m_window2_right;

	uint16_t m_mosaic_table[16][4096];
	uint8_t m_clipmasks[6][SNES_SCR_WIDTH];
	uint8_t m_mode;
	uint8_t m_interlace; //doubles the visible resolution
	uint8_t m_screen_brightness;
	uint8_t m_screen_disabled;
	uint8_t m_pseudo_hires;
	uint8_t m_color_modes;
	uint8_t m_stat77;
	uint8_t m_stat78;

	uint16_t                m_htmult;     /* in 512 wide, we run HTOTAL double and halve it on latching */
	uint16_t                m_cgram_address;  /* CGRAM address */
	uint8_t                 m_read_ophct;
	uint8_t                 m_read_opvct;
	uint16_t                m_vram_fgr_high;
	uint16_t                m_vram_fgr_increment;
	uint16_t                m_vram_fgr_count;
	uint16_t                m_vram_fgr_mask;
	uint16_t                m_vram_fgr_shift;
	uint16_t                m_vram_read_buffer;
	uint16_t                m_vmadd;

	struct object_item
	{
		bool valid;
		uint8_t index;
		uint8_t width;
		uint8_t height;
	};

	struct object_tile
	{
		bool valid;
		uint16_t x;
		uint8_t y;
		uint8_t pri;
		uint8_t pal;
		uint8_t hflip;
		uint32_t data;
	};

	struct object
	{
		uint16_t x;
		uint8_t y;
		uint8_t character;
		uint8_t name_select;
		uint8_t vflip;
		uint8_t hflip;
		uint8_t pri;
		uint8_t pal;
		uint8_t size;
	};

	inline uint32_t get_tile(uint8_t layer_idx, uint32_t hoffset, uint32_t voffset);
	void update_line(uint16_t curline, uint8_t layer, uint8_t direct_colors);
	void update_line_mode7(uint16_t curline, uint8_t layer_idx);
	void update_objects(uint16_t curline);
	void update_mode_0(uint16_t curline);
	void update_mode_1(uint16_t curline);
	void update_mode_2(uint16_t curline);
	void update_mode_3(uint16_t curline);
	void update_mode_4(uint16_t curline);
	void update_mode_5(uint16_t curline);
	void update_mode_6(uint16_t curline);
	void update_mode_7(uint16_t curline);
	void draw_screens(uint16_t curline);
	void render_window(uint16_t layer_idx, uint8_t enable, uint8_t *output);
	inline void plot_above(uint16_t x, uint8_t source, uint8_t priority, uint16_t color, int blend_exception = 0);
	inline void plot_below(uint16_t x, uint8_t source, uint8_t priority, uint16_t color, int blend_exception = 0);
	void update_color_windowmasks(uint8_t mask, uint8_t *output);
	void update_video_mode(void);
	void cache_background();
	uint16_t pixel(uint16_t x, SNES_SCANLINE *above, SNES_SCANLINE *below, uint8_t *window_above, uint8_t *window_below);
	uint16_t direct_color(uint16_t palette, uint16_t group);
	inline uint16_t blend(uint16_t x, uint16_t y, bool halve);

	void dynamic_res_change();
	inline uint32_t get_vram_address();

	uint8_t read_oam(uint16_t address);
	void write_oam(uint16_t address, uint8_t data);
	uint8_t read_object(uint16_t address);
	void write_object(uint16_t address, uint8_t data);

	uint8_t cgram_read(offs_t offset);
	void cgram_write(offs_t offset, uint8_t data);
	uint8_t vram_read(offs_t offset);
	void vram_write(offs_t offset, uint8_t data);
	object m_objects[128]; /* Object Attribute Memory (OAM) */
	std::unique_ptr<uint16_t[]> m_cgram;   /* Palette RAM */
	std::unique_ptr<uint8_t[]> m_vram;    /* Video RAM (TODO: Should be 16-bit, but it's easier this way) */
	std::unique_ptr<std::unique_ptr<uint16_t[]>[]> m_light_table; /* Luma ramp */

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_palette_interface overrides
	// 256 word CG RAM data (0x000-0x0ff), 8 group of direct colours (0x100-0x8ff), Fixed color (0x900)
	virtual uint32_t palette_entries() const noexcept override { return 0x100 + (0x100 * 8) + 1; }
	virtual uint32_t palette_indirect_entries() const noexcept override { return 32 * 32 * 32; } // 15 bit BGR

private:
	static constexpr uint16_t DIRECT_COLOUR = 0x100; // Position in palette entry for direct colour
	static constexpr uint16_t FIXED_COLOUR = 0x100 + (0x100 * 8); // Position in palette entry for fixed colour

	devcb_read16  m_openbus_cb;
	optional_ioport m_options;
	optional_ioport m_debug1;
	optional_ioport m_debug2;
	optional_ioport m_debug3;
	optional_ioport m_debug4;
};


// device type definition
DECLARE_DEVICE_TYPE(SNES_PPU, snes_ppu_device)


#endif // MAME_VIDEO_SNES_PPU_H
