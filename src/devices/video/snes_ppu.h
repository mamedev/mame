// license:BSD-3-Clause
// copyright-holders:Anthony Kruize, Fabio Priuli
/***************************************************************************

        SNES PPU

***************************************************************************/

#pragma once

#ifndef __SNES_PPU_H__
#define __SNES_PPU_H__


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

	UINT16 buffer[SNES_SCR_WIDTH];
	UINT8  priority[SNES_SCR_WIDTH];
	UINT8  layer[SNES_SCR_WIDTH];
	UINT8  blend_exception[SNES_SCR_WIDTH];
};

// ======================> snes_ppu_device

class snes_ppu_device :  public device_t,
							public device_video_interface
{
public:
	// construction/destruction
	snes_ppu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	template<class _Object> static devcb_base &static_set_open_bus_callback(device_t &device, _Object object) { return downcast<snes_ppu_device &>(device).m_openbus_cb.set_callback(object); }

	UINT8 m_regs[0x40];

	SNES_SCANLINE m_scanlines[2];

	struct
	{
		/* clipmasks */
		UINT8 window1_enabled, window1_invert;
		UINT8 window2_enabled, window2_invert;
		UINT8 wlog_mask;
		/* color math enabled */
		UINT8 color_math;

		UINT8 charmap;
		UINT8 tilemap;
		UINT8 tilemap_size;

		UINT8 tile_size;
		UINT8 mosaic_enabled;   // actually used only for layers 0->3!

		UINT8 main_window_enabled;
		UINT8 sub_window_enabled;
		UINT8 main_bg_enabled;
		UINT8 sub_bg_enabled;

		UINT16 hoffs;
		UINT16 voffs;
	} m_layer[6]; // this is for the BG1 - BG2 - BG3 - BG4 - OBJ - color layers

	struct
	{
		UINT8 address_low;
		UINT8 address_high;
		UINT8 saved_address_low;
		UINT8 saved_address_high;
		UINT16 address;
		UINT16 priority_rotation;
		UINT8 next_charmap;
		UINT8 next_size;
		UINT8 size;
		UINT32 next_name_select;
		UINT32 name_select;
		UINT8 first_sprite;
		UINT8 flip;
		UINT16 write_latch;
	} m_oam;

	struct
	{
		UINT16 latch_horz;
		UINT16 latch_vert;
		UINT16 current_vert;
		UINT8 last_visible_line;
		UINT8 interlace_count;
	} m_beam;

	struct
	{
		UINT8 repeat;
		UINT8 hflip;
		UINT8 vflip;
		INT16 matrix_a;
		INT16 matrix_b;
		INT16 matrix_c;
		INT16 matrix_d;
		INT16 origin_x;
		INT16 origin_y;
		UINT16 hor_offset;
		UINT16 ver_offset;
		UINT8 extbg;
	} m_mode7;

	struct OAM
	{
		UINT16 tile;
		INT16 x, y;
		UINT8 size, vflip, hflip, priority_bits, pal;
		int height, width;
	};

	struct OAM m_oam_spritelist[SNES_SCR_WIDTH / 2];

	UINT8 m_oam_itemlist[32];

	struct TILELIST {
		INT16 x;
		UINT16 priority, pal, tileaddr;
		int hflip;
	};

	struct TILELIST m_oam_tilelist[34];

#if SNES_LAYER_DEBUG
	struct DEBUGOPTS
	{
		UINT8 bg_disabled[5];
		UINT8 mode_disabled[8];
		UINT8 draw_subscreen;
		UINT8 windows_disabled;
		UINT8 mosaic_disabled;
		UINT8 colormath_disabled;
		UINT8 sprite_reversed;
		UINT8 select_pri[5];
	};
	struct DEBUGOPTS m_debug_options;
#endif

	UINT8 m_mosaic_size;
	UINT8 m_clip_to_black;
	UINT8 m_prevent_color_math;
	UINT8 m_sub_add_mode;
	UINT8 m_bg3_priority_bit;
	UINT8 m_direct_color;
	UINT8 m_ppu_last_scroll;      /* as per Anomie's doc and Theme Park, all scroll regs shares (but mode 7 ones) the same
                                   'previous' scroll value */
	UINT8 m_mode7_last_scroll;    /* as per Anomie's doc mode 7 scroll regs use a different value, shared with mode 7 matrix! */

	UINT8 m_ppu1_open_bus, m_ppu2_open_bus;
	UINT8 m_ppu1_version, m_ppu2_version;
	UINT8 m_window1_left, m_window1_right, m_window2_left, m_window2_right;

	UINT16 m_mosaic_table[16][4096];
	UINT8 m_clipmasks[6][SNES_SCR_WIDTH];
	UINT8 m_update_windows;
	UINT8 m_update_offsets;
	UINT8 m_update_oam_list;
	UINT8 m_mode;
	UINT8 m_interlace; //doubles the visible resolution
	UINT8 m_obj_interlace;
	UINT8 m_screen_brightness;
	UINT8 m_screen_disabled;
	UINT8 m_pseudo_hires;
	UINT8 m_color_modes;
	UINT8 m_stat77;
	UINT8 m_stat78;

	UINT16                m_htmult;     /* in 512 wide, we run HTOTAL double and halve it on latching */
	UINT16                m_cgram_address;  /* CGRAM address */
	UINT8                 m_read_ophct;
	UINT8                 m_read_opvct;
	UINT16                m_vram_fgr_high;
	UINT16                m_vram_fgr_increment;
	UINT16                m_vram_fgr_count;
	UINT16                m_vram_fgr_mask;
	UINT16                m_vram_fgr_shift;
	UINT16                m_vram_read_buffer;
	UINT16                m_vmadd;

	inline UINT16 get_bgcolor(UINT8 direct_colors, UINT16 palette, UINT8 color);
	inline void set_scanline_pixel(int screen, INT16 x, UINT16 color, UINT8 priority, UINT8 layer, int blend);
	inline void draw_bgtile_lores(UINT8 layer, INT16 ii, UINT8 colour, UINT16 pal, UINT8 direct_colors, UINT8 priority);
	inline void draw_bgtile_hires(UINT8 layer, INT16 ii, UINT8 colour, UINT16 pal, UINT8 direct_colors, UINT8 priority);
	inline void draw_oamtile(INT16 ii, UINT8 colour, UINT16 pal, UINT8 priority);
	inline void draw_tile(UINT8 planes, UINT8 layer, UINT32 tileaddr, INT16 x, UINT8 priority, UINT8 flip, UINT8 direct_colors, UINT16 pal, UINT8 hires);
	inline UINT32 get_tmap_addr(UINT8 layer, UINT8 tile_size, UINT32 base, UINT32 x, UINT32 y);
	inline void update_line(UINT16 curline, UINT8 layer, UINT8 priority_b, UINT8 priority_a, UINT8 color_depth, UINT8 hires, UINT8 offset_per_tile, UINT8 direct_colors);
	void update_line_mode7(UINT16 curline, UINT8 layer, UINT8 priority_b, UINT8 priority_a);
	void update_obsel(void);
	void oam_list_build(void);
	int is_sprite_on_scanline(UINT16 curline, UINT8 sprite);
	void update_objects_rto(UINT16 curline);
	void update_objects(UINT8 priority_oam0, UINT8 priority_oam1, UINT8 priority_oam2, UINT8 priority_oam3);
	void update_mode_0(UINT16 curline);
	void update_mode_1(UINT16 curline);
	void update_mode_2(UINT16 curline);
	void update_mode_3(UINT16 curline);
	void update_mode_4(UINT16 curline);
	void update_mode_5(UINT16 curline);
	void update_mode_6(UINT16 curline);
	void update_mode_7(UINT16 curline);
	void draw_screens(UINT16 curline);
	void update_windowmasks(void);
	void update_offsets(void);
	inline void draw_blend(UINT16 offset, UINT16 *colour, UINT8 prevent_color_math, UINT8 black_pen_clip, int switch_screens);
	void refresh_scanline(bitmap_rgb32 &bitmap, UINT16 curline);

	inline INT16 current_x() { return m_screen->hpos() / m_htmult; }
	inline INT16 current_y() { return m_screen->vpos(); }
	void set_latch_hv(INT16 x, INT16 y);
	void dynamic_res_change();
	inline UINT32 get_vram_address();

	UINT8 read(address_space &space, UINT32 offset, UINT8 wrio_bit7);
	void write(address_space &space, UINT32 offset, UINT8 data);

	DECLARE_READ8_MEMBER( oam_read );
	DECLARE_WRITE8_MEMBER( oam_write );
	DECLARE_READ8_MEMBER( cgram_read );
	DECLARE_WRITE8_MEMBER( cgram_write );
	DECLARE_READ8_MEMBER( vram_read );
	DECLARE_WRITE8_MEMBER( vram_write );
	std::unique_ptr<UINT16[]> m_oam_ram;     /* Object Attribute Memory */
	std::unique_ptr<UINT16[]> m_cgram;   /* Palette RAM */
	std::unique_ptr<UINT8[]> m_vram;    /* Video RAM (TODO: Should be 16-bit, but it's easier this way) */

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_read16  m_openbus_cb;
};


// device type definition
extern const device_type SNES_PPU;


/***************************************************************************
 INTERFACE CONFIGURATION MACROS
 ***************************************************************************/

#define MCFG_SNES_PPU_OPENBUS_CB(_read) \
	devcb = &snes_ppu_device::static_set_open_bus_callback(*device, DEVCB_##_read);

#endif
