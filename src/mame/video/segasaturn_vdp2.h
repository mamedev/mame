// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, Olivier Galibert, Mariusz Wojcieszek, R. Belmont
/**************************************************************************************************

Sega Saturn / ST-V - VDP2

**************************************************************************************************/

#ifndef MAME_VIDEO_SATURN_VDP2_H
#define MAME_VIDEO_SATURN_VDP2_H

#pragma once

#include "screen.h"
#include "emupal.h"
#include "segasaturn_vdp1.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class saturn_vdp2_device : public device_t, 
						   public device_video_interface
{
public:
	// construction/destruction
	saturn_vdp2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class T> void set_vdp1(T &&tag) { m_vdp1.set_tag(std::forward<T>(tag)); }

	// I/O operations
	void vram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 vram_r(offs_t offset);
	void cram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cram_r(offs_t offset);
	void regs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 regs_r(offs_t offset);

	void set_pal_system(bool state);
	void flip_odd_bit();
	void set_dotsel(bool state);
	int get_vblank_start_position( void );
	int get_ystep_count( void );
	void vram_clear();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device <saturn_vdp1_device> m_vdp1;
	required_device <palette_device> m_palette;
	required_device <gfxdecode_device> m_gfxdecode;
	
	/* VDP1 Framebuffer/mixing handling */
	int      stv_sprite_priorities_used[8];
	int      stv_sprite_priorities_usage_valid;
	uint8_t  stv_sprite_priorities_in_fb_line[512][8];
	void draw_vdp1_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint8_t pri);

	std::unique_ptr<uint8_t[]>      gfx_decode_buffer;
	std::unique_ptr<uint16_t[]>    m_vdp2_regs;
	std::unique_ptr<uint32_t[]>    m_vdp2_vram;
	std::unique_ptr<uint32_t[]>    m_vdp2_cram;

	bitmap_rgb32 roz_bitmap[2];
	bitmap_rgb32 m_tmpbitmap;
	uint8_t     dotsel;
	bool        is_pal_system;
	uint8_t     odd_bit;
	uint16_t    h_count;
	uint16_t    v_count;
	uint8_t     exltfg;
	uint8_t     exsyfg;
	int       old_crmd;
	int       old_tvmd;

	uint8_t get_vblank( void );
	uint8_t get_hblank( void );
	int get_hcounter( void );
	int get_vcounter( void );
	int get_vblank_duration( void );
	int get_hblank_duration( void );
	XTAL get_pixel_clock( void );
	uint8_t get_odd_bit( void );
	void stv_vdp2_dynamic_res_change( void );

	void refresh_palette_data( void );
	inline int stv_vdp2_window_process(int x,int y);
	void stv_vdp2_get_window0_coordinates(int *s_x, int *e_x, int *s_y, int *e_y, int y);
	void stv_vdp2_get_window1_coordinates(int *s_x, int *e_x, int *s_y, int *e_y, int y);
	int get_window_pixel(int s_x,int e_x,int s_y,int e_y,int x, int y,uint8_t win_num);
	int stv_vdp2_apply_window_on_layer(rectangle &cliprect);

	void stv_vdp2_draw_basic_tilemap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_draw_basic_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_4bpp_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_8bpp_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_11bpp_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_rgb15_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_rgb32_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void stv_vdp2_drawgfxzoom(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx, uint32_t code,uint32_t color,int flipx,int flipy,int sx,int sy,int transparency,int scalex, int scaley,int sprite_screen_width, int sprite_screen_height, int alpha);
	void stv_vdp2_drawgfxzoom_rgb555(bitmap_rgb32 &dest_bmp,const rectangle &clip,uint32_t code,uint32_t color,int flipx,int flipy,int sx,int sy,int transparency,int scalex, int scaley,int sprite_screen_width, int sprite_screen_height, int alpha);
	void stv_vdp2_drawgfx_rgb555( bitmap_rgb32 &dest_bmp, const rectangle &clip, uint32_t code, int flipx, int flipy, int sx, int sy, int transparency, int alpha);
	void stv_vdp2_drawgfx_rgb888( bitmap_rgb32 &dest_bmp, const rectangle &clip, uint32_t code, int flipx, int flipy, int sx, int sy, int transparency, int alpha);

	void stv_vdp2_drawgfx_alpha(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx, uint32_t code,uint32_t color, int flipx,int flipy,int offsx,int offsy, int transparency, int alpha);
	void stv_vdp2_drawgfx_transpen(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx, uint32_t code,uint32_t color, int flipx,int flipy,int offsx,int offsy, int transparency);


	void stv_vdp2_draw_rotation_screen(bitmap_rgb32 &bitmap, const rectangle &cliprect, int iRP);
	void stv_vdp2_check_tilemap_with_linescroll(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_check_tilemap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_copy_roz_bitmap(bitmap_rgb32 &bitmap, bitmap_rgb32 &roz_bitmap, const rectangle &cliprect, int iRP, int planesizex, int planesizey, int planerenderedsizex, int planerenderedsizey);
	inline bool stv_vdp2_roz_window(int x, int y);
	inline bool stv_vdp2_roz_mode3_window(int x, int y, int rot_parameter);
	inline int get_roz_window_pixel(int s_x,int e_x,int s_y,int e_y,int x, int y,uint8_t winenable,uint8_t winarea);
	void stv_vdp2_fill_rotation_parameter_table( uint8_t rot_parameter );
	uint8_t stv_vdp2_check_vram_cycle_pattern_registers( uint8_t access_command_pnmdr, uint8_t access_command_cpdr, uint8_t bitmap_enable );
	uint8_t stv_vdp2_is_rotation_applied(void);
	uint8_t stv_vdp2_are_map_registers_equal(void);
	void stv_vdp2_get_map_page( int x, int y, int *_map, int *_page );

	void stv_vdp2_draw_mosaic(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint8_t is_roz);
	void stv_vdp2_fade_effects( void );
	void stv_vdp2_compute_color_offset( int *r, int *g, int *b, int cor );
	void stv_vdp2_compute_color_offset_UINT32(rgb_t *rgb, int cor);
	void stv_vdp2_check_fade_control_for_layer( void );

	void stv_vdp2_draw_line(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_draw_back(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_draw_NBG0(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_draw_NBG1(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_draw_NBG2(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_draw_NBG3(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_draw_RBG0(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	int true_vcount[263][4];

	void stv_vdp2_state_save_postload( void );
	void stv_vdp2_exit ( void );
	int stv_vdp2_start ( void );

	uint8_t m_vdpdebug_roz;

	struct stv_vdp2_tilemap_capabilities
	{
		uint8_t  enabled;
		uint8_t  transparency;
		uint8_t  colour_calculation_enabled;
		uint8_t  colour_depth;
		uint8_t  alpha;
		uint8_t  tile_size;
		uint8_t  bitmap_enable;
		uint8_t  bitmap_size;
		uint8_t  bitmap_palette_number;
		uint8_t  bitmap_map;
		uint16_t map_offset[16];
		uint8_t  map_count;

		uint8_t  pattern_data_size;
		uint8_t  character_number_supplement;
		uint8_t  special_priority_register;
		uint8_t  special_colour_control_register;
		uint8_t  supplementary_palette_bits;
		uint8_t  supplementary_character_bits;

		int16_t scrollx;
		int16_t scrolly;
		uint32_t incx, incy;

		uint8_t   linescroll_enable;
		uint8_t   linescroll_interval;
		uint32_t  linescroll_table_address;
		uint8_t   vertical_linescroll_enable;
		uint8_t   vertical_cell_scroll_enable;
		uint8_t   linezoom_enable;

		uint8_t  plane_size;
		uint8_t  colour_ram_address_offset;
		uint8_t  fade_control;
		struct{
			uint8_t logic;
			uint8_t enabled[2];
			uint8_t area[2];
		}window_control;

		uint8_t  line_screen_enabled;
		uint8_t  mosaic_screen_enabled;
		bool roz_mode3;

		int layer_name; /* just to keep track */
	} stv2_current_tilemap;

	struct rotation_table
	{
		int32_t   xst;
		int32_t   yst;
		int32_t   zst;
		int32_t   dxst;
		int32_t   dyst;
		int32_t   dx;
		int32_t   dy;
		int32_t   A;
		int32_t   B;
		int32_t   C;
		int32_t   D;
		int32_t   E;
		int32_t   F;
		int32_t   px;
		int32_t   py;
		int32_t   pz;
		int32_t   cx;
		int32_t   cy;
		int32_t   cz;
		int32_t   mx;
		int32_t   my;
		int32_t   kx;
		int32_t   ky;
		uint32_t  kast;
		int32_t   dkast;
		int32_t   dkax;

	} stv_current_rotation_parameter_table;

	struct _stv_vdp2_layer_data_placement
	{
		uint32_t  map_offset_min;
		uint32_t  map_offset_max;
		uint32_t  tile_offset_min;
		uint32_t  tile_offset_max;
	} stv_vdp2_layer_data_placement;

	struct _stv_rbg_cache_data
	{
		uint8_t   watch_vdp2_vram_writes;
		uint8_t   is_cache_dirty;

		uint32_t  map_offset_min[2];
		uint32_t  map_offset_max[2];
		uint32_t  tile_offset_min[2];
		uint32_t  tile_offset_max[2];

		struct stv_vdp2_tilemap_capabilities    layer_data[2];

	} stv_rbg_cache_data;
};


// device type definition
DECLARE_DEVICE_TYPE(SATURN_VDP2, saturn_vdp2_device)

#endif // MAME_VIDEO_SATURN_VDP2_H
