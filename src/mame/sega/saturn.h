// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, Olivier Galibert, Mariusz Wojcieszek, R. Belmont
#ifndef MAME_SEGA_SATURN_H
#define MAME_SEGA_SATURN_H

#pragma once

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "cpu/m68000/m68000.h"
#include "cpu/sh/sh7604.h"

#include "315-5881_crypt.h"
#include "315-5838_317-0229_comp.h"
#include "saturn_scu.h"
#include "smpc.h"

#include "machine/timer.h"
#include "sound/scsp.h"

#include "emupal.h"
#include "screen.h"

class saturn_state : public driver_device
{
public:
	saturn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_rom(*this, "bios"),
			m_workram_l(*this, "workram_l"),
			m_workram_h(*this, "workram_h"),
			m_sound_ram(*this, "sound_ram"),
			m_fake_comms(*this, "fake"),
			m_maincpu(*this, "maincpu"),
			m_slave(*this, "slave"),
			m_audiocpu(*this, "audiocpu"),
			m_scsp(*this, "scsp"),
			m_smpc_hle(*this, "smpc"),
			m_scu(*this, "scu"),
			m_gfxdecode(*this, "gfxdecode"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette")
	{
	}

	void scsp_irq(offs_t offset, uint8_t data);

	// SMPC HLE delegates
	void master_sh2_reset_w(int state);
	void master_sh2_nmi_w(int state);
	void slave_sh2_reset_w(int state);
	void sound_68k_reset_w(int state);
	void system_reset_w(int state);
	void system_halt_w(int state);
	void dot_select_w(int state);

	void m68k_reset_callback(int state);

protected:
	required_region_ptr<uint32_t> m_rom;
	required_shared_ptr<uint32_t> m_workram_l;
	required_shared_ptr<uint32_t> m_workram_h;
	required_shared_ptr<uint16_t> m_sound_ram;
	optional_ioport m_fake_comms;

	memory_region *m_cart_reg[4];
	std::unique_ptr<uint8_t[]>     m_backupram;
	std::unique_ptr<uint16_t[]>    m_vdp2_regs;
	std::unique_ptr<uint32_t[]>    m_vdp2_vram;
	std::unique_ptr<uint32_t[]>    m_vdp2_cram;
	std::unique_ptr<uint32_t[]>    m_vdp1_vram;
	std::unique_ptr<uint16_t[]>    m_vdp1_regs;

	uint8_t     m_en_68k = 0;

	int       m_minit_boost = 0;
	int       m_sinit_boost = 0;
	attotime  m_minit_boost_timeslice;
	attotime  m_sinit_boost_timeslice;

	struct {
		std::unique_ptr<uint16_t * []> framebuffer_display_lines;
		int         framebuffer_mode = 0;
		int         framebuffer_double_interlace = 0;
		int         fbcr_accessed = 0;
		int         framebuffer_width = 0;
		int         framebuffer_height = 0;
		int         framebuffer_current_display = 0;
		int         framebuffer_current_draw = 0;
		int         framebuffer_clear_on_next_frame = 0;
		rectangle system_cliprect;
		rectangle user_cliprect;
		std::unique_ptr<uint16_t []> framebuffer[2];
		std::unique_ptr<uint16_t * []> framebuffer_draw_lines;
		std::unique_ptr<uint8_t []> gfx_decode;
		uint16_t    lopr = 0;
		uint16_t    copr = 0;
		uint16_t    ewdr = 0;

		int         local_x = 0;
		int         local_y = 0;

		emu_timer * draw_end_timer = nullptr;
	}m_vdp1;

	struct {
		std::unique_ptr<uint8_t[]>      gfx_decode;
		bitmap_rgb32 roz_bitmap[2];
		uint8_t     dotsel = 0;
		uint8_t     pal = 0;
		uint8_t     odd = 0;
		uint16_t    h_count = 0;
		uint16_t    v_count = 0;
		uint8_t     exltfg = 0;
		uint8_t     exsyfg = 0;
		int       old_crmd = 0;
		int       old_tvmd = 0;
	}m_vdp2;

	required_device<sh7604_device> m_maincpu;
	required_device<sh7604_device> m_slave;
	required_device<m68000_base_device> m_audiocpu;
	required_device<scsp_device> m_scsp;
	required_device<smpc_hle_device> m_smpc_hle;
	required_device<saturn_scu_device> m_scu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	bitmap_rgb32 m_tmpbitmap;
	DECLARE_VIDEO_START(vdp2_video_start);
	uint32_t screen_update_vdp2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(saturn_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(saturn_slave_scanline);


	TIMER_CALLBACK_MEMBER(vdp1_draw_end);
	void soundram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t soundram_r(offs_t offset);
	void minit_w(uint32_t data);
	void sinit_w(uint32_t data);
	void saturn_minit_w(uint32_t data);
	void saturn_sinit_w(uint32_t data);
	uint8_t backupram_r(offs_t offset);
	void backupram_w(offs_t offset, uint8_t data);

	int m_scsp_last_line = 0;

	uint16_t vdp1_regs_r(offs_t offset);
	uint32_t vdp1_vram_r(offs_t offset);
	uint32_t vdp1_framebuffer0_r(offs_t offset, uint32_t mem_mask = ~0);

	void vdp1_regs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vdp1_vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void vdp1_framebuffer0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t vdp2_vram_r(offs_t offset);
	uint32_t vdp2_cram_r(offs_t offset);
	uint16_t vdp2_regs_r(offs_t offset);

	void vdp2_vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void vdp2_cram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void vdp2_regs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);


	/* VDP1 */
	void vdp1_set_framebuffer_config();
	void vdp1_prepare_framebuffers();
	void vdp1_change_framebuffers();
	void vdp1_video_update();
	void vdp1_process_list();
	void vdp1_set_drawpixel();

	void vdp1_draw_normal_sprite(const rectangle &cliprect, int sprite_type);
	void vdp1_draw_scaled_sprite(const rectangle &cliprect);
	void vdp1_draw_distorted_sprite(const rectangle &cliprect);
	void vdp1_draw_poly_line(const rectangle &cliprect);
	void vdp1_draw_line(const rectangle &cliprect);
	int x2s(int v);
	int y2s(int v);
	void vdp1_fill_quad(const rectangle &cliprect, int patterndata, int xsize, const struct spoint *q);
	void vdp1_fill_line(const rectangle &cliprect, int patterndata, int xsize, int32_t y, int32_t x1, int32_t x2, int32_t u1, int32_t u2, int32_t v1, int32_t v2);
	void (saturn_state::*drawpixel)(int x, int y, int patterndata, int offsetcnt);
	void drawpixel_poly(int x, int y, int patterndata, int offsetcnt);
	void drawpixel_8bpp_trans(int x, int y, int patterndata, int offsetcnt);
	void drawpixel_4bpp_notrans(int x, int y, int patterndata, int offsetcnt);
	void drawpixel_4bpp_trans(int x, int y, int patterndata, int offsetcnt);
	void drawpixel_generic(int x, int y, int patterndata, int offsetcnt);
	void vdp1_fill_slope(const rectangle &cliprect, int patterndata, int xsize,
							int32_t x1, int32_t x2, int32_t sl1, int32_t sl2, int32_t *nx1, int32_t *nx2,
							int32_t u1, int32_t u2, int32_t slu1, int32_t slu2, int32_t *nu1, int32_t *nu2,
							int32_t v1, int32_t v2, int32_t slv1, int32_t slv2, int32_t *nv1, int32_t *nv2,
							int32_t _y1, int32_t y2);
	void vdp1_setup_shading_for_line(int32_t y, int32_t x1, int32_t x2,
												int32_t r1, int32_t g1, int32_t b1,
												int32_t r2, int32_t g2, int32_t b2);
	void vdp1_setup_shading_for_slope(
							int32_t x1, int32_t x2, int32_t sl1, int32_t sl2, int32_t *nx1, int32_t *nx2,
							int32_t r1, int32_t r2, int32_t slr1, int32_t slr2, int32_t *nr1, int32_t *nr2,
							int32_t g1, int32_t g2, int32_t slg1, int32_t slg2, int32_t *ng1, int32_t *ng2,
							int32_t b1, int32_t b2, int32_t slb1, int32_t slb2, int32_t *nb1, int32_t *nb2,
							int32_t _y1, int32_t y2);
	uint16_t vdp1_apply_gouraud_shading(int x, int y, uint16_t pix);
	void vdp1_setup_shading(const struct spoint* q, const rectangle &cliprect);
	uint8_t read_gouraud_table();
	void clear_gouraud_shading();

	void vdp1_clear_framebuffer(int which_framebuffer);
	void vdp1_state_save_postload();
	int vdp1_start();

	struct vdp1_poly_scanline
	{
		int32_t   x[2]{};
		int32_t   b[2]{};
		int32_t   g[2]{};
		int32_t   r[2]{};
		int32_t   db = 0;
		int32_t   dg = 0;
		int32_t   dr = 0;
	};

	struct vdp1_poly_scanline_data
	{
		int32_t   sy = 0, ey = 0;
		struct  vdp1_poly_scanline scanline[512];
	};

	std::unique_ptr<struct vdp1_poly_scanline_data> vdp1_shading_data;

	struct vdp1_sprite_list
	{
		int CMDCTRL = 0, CMDLINK = 0, CMDPMOD = 0, CMDCOLR = 0, CMDSRCA = 0, CMDSIZE = 0, CMDGRDA = 0;
		int CMDXA = 0, CMDYA = 0;
		int CMDXB = 0, CMDYB = 0;
		int CMDXC = 0, CMDYC = 0;
		int CMDXD = 0, CMDYD = 0;

		int ispoly = 0;

	} current_sprite;

	/* Gouraud shading */

	struct _gouraud_shading
	{
		/* Gouraud shading table */
		uint16_t  GA = 0;
		uint16_t  GB = 0;
		uint16_t  GC = 0;
		uint16_t  GD = 0;
	} gouraud_shading;

	uint16_t m_sprite_colorbank = 0;

	/* VDP1 Framebuffer handling */
	int      vdp1_sprite_priorities_used[8]{};
	int      vdp1_sprite_priorities_usage_valid = 0;
	uint8_t    vdp1_sprite_priorities_in_fb_line[512][8]{};


	/* VDP2 */

	uint8_t get_vblank();
	uint8_t get_hblank();
	int get_hcounter();
	int get_vcounter();
	int get_vblank_duration();
	int get_hblank_duration();
	int get_pixel_clock();
	uint8_t get_odd_bit();
	void vdp2_dynamic_res_change();
	int get_vblank_start_position();
	int get_ystep_count();

	void refresh_palette_data();
	inline int vdp2_window_process(int x,int y);
	void vdp2_get_window0_coordinates(int *s_x, int *e_x, int *s_y, int *e_y, int y);
	void vdp2_get_window1_coordinates(int *s_x, int *e_x, int *s_y, int *e_y, int y);
	int get_window_pixel(int s_x,int e_x,int s_y,int e_y,int x, int y,uint8_t win_num);
	int vdp2_apply_window_on_layer(rectangle &cliprect);

	void vdp2_draw_basic_tilemap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vdp2_draw_basic_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_4bpp_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_8bpp_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_11bpp_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_rgb15_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_rgb32_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void vdp2_drawgfxzoom(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx, uint32_t code,uint32_t color,int flipx,int flipy,int sx,int sy,int transparency,int scalex, int scaley,int sprite_screen_width, int sprite_screen_height, int alpha);
	void vdp2_drawgfxzoom_rgb555(bitmap_rgb32 &dest_bmp,const rectangle &clip,uint32_t code,uint32_t color,int flipx,int flipy,int sx,int sy,int transparency,int scalex, int scaley,int sprite_screen_width, int sprite_screen_height, int alpha);
	void vdp2_drawgfx_rgb555(bitmap_rgb32 &dest_bmp, const rectangle &clip, uint32_t code, int flipx, int flipy, int sx, int sy, int transparency, int alpha);
	void vdp2_drawgfx_rgb888(bitmap_rgb32 &dest_bmp, const rectangle &clip, uint32_t code, int flipx, int flipy, int sx, int sy, int transparency, int alpha);

	void vdp2_drawgfx_alpha(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx, uint32_t code,uint32_t color, int flipx,int flipy,int offsx,int offsy, int transparency, int alpha);
	void vdp2_drawgfx_transpen(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx, uint32_t code,uint32_t color, int flipx,int flipy,int offsx,int offsy, int transparency);


	void vdp2_draw_rotation_screen(bitmap_rgb32 &bitmap, const rectangle &cliprect, int iRP);
	void vdp2_check_tilemap_with_linescroll(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vdp2_check_tilemap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vdp2_copy_roz_bitmap(bitmap_rgb32 &bitmap, bitmap_rgb32 &roz_bitmap, const rectangle &cliprect, int iRP, int planesizex, int planesizey, int planerenderedsizex, int planerenderedsizey);
	inline bool vdp2_roz_window(int x, int y);
	inline bool vdp2_roz_mode3_window(int x, int y, int rot_parameter);
	inline int get_roz_window_pixel(int s_x,int e_x,int s_y,int e_y,int x, int y,uint8_t winenable,uint8_t winarea);
	void vdp2_fill_rotation_parameter_table(uint8_t rot_parameter);
	uint8_t vdp2_check_vram_cycle_pattern_registers(uint8_t access_command_pnmdr, uint8_t access_command_cpdr, uint8_t bitmap_enable);
	uint8_t vdp2_is_rotation_applied();
	uint8_t vdp2_are_map_registers_equal();
	void vdp2_get_map_page(int x, int y, int *_map, int *_page);

	void vdp2_draw_mosaic(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint8_t is_roz);
	void vdp2_fade_effects();
	void vdp2_compute_color_offset(int *r, int *g, int *b, int cor);
	void vdp2_compute_color_offset_UINT32(rgb_t *rgb, int cor);
	void vdp2_check_fade_control_for_layer();

	void vdp2_draw_line(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vdp2_draw_back(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vdp2_draw_NBG0(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vdp2_draw_NBG1(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vdp2_draw_NBG2(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vdp2_draw_NBG3(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vdp2_draw_RBG0(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint8_t pri);
	int true_vcount[263][4];

	void vdp2_state_save_postload();
	void vdp2_exit();
	int vdp2_start();

	uint8_t m_vdpdebug_roz = 0;

	struct vdp2_tilemap_capabilities
	{
		uint8_t  enabled = 0;
		uint8_t  transparency = 0;
		uint8_t  colour_calculation_enabled = 0;
		uint8_t  colour_depth = 0;
		uint8_t  alpha = 0;
		uint8_t  tile_size = 0;
		uint8_t  bitmap_enable = 0;
		uint8_t  bitmap_size = 0;
		uint8_t  bitmap_palette_number = 0;
		uint8_t  bitmap_map = 0;
		uint16_t map_offset[16]{};
		uint8_t  map_count = 0;

		uint8_t  pattern_data_size = 0;
		uint8_t  character_number_supplement = 0;
		uint8_t  special_priority_register = 0;
		uint8_t  special_colour_control_register = 0;
		uint8_t  supplementary_palette_bits = 0;
		uint8_t  supplementary_character_bits = 0;

		int16_t scrollx = 0;
		int16_t scrolly = 0;
		uint32_t incx = 0, incy = 0;

		uint8_t   linescroll_enable = 0;
		uint8_t   linescroll_interval = 0;
		uint32_t  linescroll_table_address = 0;
		uint8_t   vertical_linescroll_enable = 0;
		uint8_t   vertical_cell_scroll_enable = 0;
		uint8_t   linezoom_enable = 0;

		uint8_t  plane_size = 0;
		uint8_t  colour_ram_address_offset = 0;
		uint8_t  fade_control = 0;
		struct{
			uint8_t logic = 0;
			uint8_t enabled[2]{};
			uint8_t area[2]{};
		}window_control;

		uint8_t  line_screen_enabled = 0;
		uint8_t  mosaic_screen_enabled = 0;
		bool roz_mode3 = false;

		int layer_name = 0; /* just to keep track */
	} current_tilemap;

	struct rotation_table
	{
		int32_t   xst = 0;
		int32_t   yst = 0;
		int32_t   zst = 0;
		int32_t   dxst = 0;
		int32_t   dyst = 0;
		int32_t   dx = 0;
		int32_t   dy = 0;
		int32_t   A = 0;
		int32_t   B = 0;
		int32_t   C = 0;
		int32_t   D = 0;
		int32_t   E = 0;
		int32_t   F = 0;
		int32_t   px = 0;
		int32_t   py = 0;
		int32_t   pz = 0;
		int32_t   cx = 0;
		int32_t   cy = 0;
		int32_t   cz = 0;
		int32_t   mx = 0;
		int32_t   my = 0;
		int32_t   kx = 0;
		int32_t   ky = 0;
		uint32_t  kast = 0;
		int32_t   dkast = 0;
		int32_t   dkax = 0;

	} current_rotation_table;

	struct _vdp2_layer_data
	{
		uint32_t  map_offset_min = 0;
		uint32_t  map_offset_max = 0;
		uint32_t  tile_offset_min = 0;
		uint32_t  tile_offset_max = 0;
	} vdp2_layer_data;

	struct _RBG0_cache_data
	{
		uint8_t   watch_vdp2_vram_writes = 0;
		uint8_t   is_cache_dirty = 0;

		uint32_t  map_offset_min[2]{ 0, 0 };
		uint32_t  map_offset_max[2]{ 0, 0 };
		uint32_t  tile_offset_min[2]{ 0, 0 };
		uint32_t  tile_offset_max[2]{ 0, 0 };

		struct vdp2_tilemap_capabilities    layer_data[2];

	} RBG0_cache_data;

//  void scudsp_end_w(int state);
//  uint16_t scudsp_dma_r(offs_t offset);
//  void scudsp_dma_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
};


// These two clocks are synthesized by the 315-5746
#define MASTER_CLOCK_352 XTAL(14'318'181)*4
#define MASTER_CLOCK_320 XTAL(14'318'181)*3.75
#define CEF_1   m_vdp1_regs[0x010/2]|=0x0002
#define CEF_0   m_vdp1_regs[0x010/2]&=~0x0002
#define BEF_1   m_vdp1_regs[0x010/2]|=0x0001
#define BEF_0   m_vdp1_regs[0x010/2]&=~0x0001
#define VDP1_TVMR ((m_vdp1_regs[0x000/2])&0xffff)
#define VDP1_VBE  ((VDP1_TVMR & 0x0008) >> 3)
#define VDP1_TVM  ((VDP1_TVMR & 0x0007) >> 0)


extern gfx_decode_entry const gfx_stv[];

#endif // MAME_SEGA_SATURN_H
