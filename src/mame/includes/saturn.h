// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, Olivier Galibert, Mariusz Wojcieszek, R. Belmont
#ifndef MAME_INCLUDES_SATURN_H
#define MAME_INCLUDES_SATURN_H

#pragma once

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "cpu/m68000/m68000.h"
#include "cpu/sh/sh2.h"

#include "debug/debugcon.h"
#include "debug/debugcmd.h"

#include "machine/315-5881_crypt.h"
#include "machine/315-5838_317-0229_comp.h"
#include "machine/sega_scu.h"
#include "machine/smpc.h"
#include "machine/timer.h"

#include "sound/scsp.h"

#include "debugger.h"
#include "emupal.h"
#include "screen.h"

class saturn_state : public driver_device
{
public:
	saturn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_rom(*this, "bios", 0x20000),
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

	DECLARE_WRITE8_MEMBER(scsp_irq);

	// SMPC HLE delegates
	DECLARE_WRITE_LINE_MEMBER(master_sh2_reset_w);
	DECLARE_WRITE_LINE_MEMBER(master_sh2_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(slave_sh2_reset_w);
	DECLARE_WRITE_LINE_MEMBER(sound_68k_reset_w);
	DECLARE_WRITE_LINE_MEMBER(system_reset_w);
	DECLARE_WRITE_LINE_MEMBER(system_halt_w);
	DECLARE_WRITE_LINE_MEMBER(dot_select_w);

	DECLARE_WRITE_LINE_MEMBER(m68k_reset_callback);

protected:
	required_region_ptr<uint32_t> m_rom;
	required_shared_ptr<uint32_t> m_workram_l;
	required_shared_ptr<uint32_t> m_workram_h;
	required_shared_ptr<uint16_t> m_sound_ram;
	optional_ioport m_fake_comms;

	memory_region *m_cart_reg[4];
	std::unique_ptr<uint8_t[]>     m_backupram;
//  std::unique_ptr<uint32_t[]>    m_scu_regs;
	std::unique_ptr<uint16_t[]>    m_vdp2_regs;
	std::unique_ptr<uint32_t[]>    m_vdp2_vram;
	std::unique_ptr<uint32_t[]>    m_vdp2_cram;
	std::unique_ptr<uint32_t[]>    m_vdp1_vram;
	std::unique_ptr<uint16_t[]>    m_vdp1_regs;

	uint8_t     m_en_68k;

	int       m_minit_boost;
	int       m_sinit_boost;
	attotime  m_minit_boost_timeslice;
	attotime  m_sinit_boost_timeslice;

	struct {
		uint16_t    **framebuffer_display_lines;
		int       framebuffer_mode;
		int       framebuffer_double_interlace;
		int       fbcr_accessed;
		int       framebuffer_width;
		int       framebuffer_height;
		int       framebuffer_current_display;
		int       framebuffer_current_draw;
		int       framebuffer_clear_on_next_frame;
		rectangle system_cliprect;
		rectangle user_cliprect;
		std::unique_ptr<uint16_t[]>   framebuffer[2];
		uint16_t    **framebuffer_draw_lines;
		std::unique_ptr<uint8_t[]>     gfx_decode;
		uint16_t    lopr;
		uint16_t    copr;
		uint16_t    ewdr;

		int       local_x;
		int       local_y;
	}m_vdp1;

	struct {
		std::unique_ptr<uint8_t[]>      gfx_decode;
		bitmap_rgb32 roz_bitmap[2];
		uint8_t     dotsel;
		uint8_t     pal;
		uint8_t     odd;
		uint16_t    h_count;
		uint16_t    v_count;
		uint8_t     exltfg;
		uint8_t     exsyfg;
		int       old_crmd;
		int       old_tvmd;
	}m_vdp2;

	required_device<sh2_device> m_maincpu;
	required_device<sh2_device> m_slave;
	required_device<m68000_base_device> m_audiocpu;
	required_device<scsp_device> m_scsp;
	required_device<smpc_hle_device> m_smpc_hle;
	required_device<sega_scu_device> m_scu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	bitmap_rgb32 m_tmpbitmap;
	DECLARE_VIDEO_START(stv_vdp2);
	uint32_t screen_update_stv_vdp2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(saturn_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(saturn_slave_scanline);


	TIMER_CALLBACK_MEMBER(vdp1_draw_end);
	DECLARE_WRITE16_MEMBER(saturn_soundram_w);
	DECLARE_READ16_MEMBER(saturn_soundram_r);
	DECLARE_WRITE32_MEMBER(minit_w);
	DECLARE_WRITE32_MEMBER(sinit_w);
	DECLARE_WRITE32_MEMBER(saturn_minit_w);
	DECLARE_WRITE32_MEMBER(saturn_sinit_w);
	DECLARE_READ8_MEMBER(saturn_backupram_r);
	DECLARE_WRITE8_MEMBER(saturn_backupram_w);

	int m_scsp_last_line;

	DECLARE_READ16_MEMBER ( saturn_vdp1_regs_r );
	DECLARE_READ32_MEMBER ( saturn_vdp1_vram_r );
	DECLARE_READ32_MEMBER ( saturn_vdp1_framebuffer0_r );

	DECLARE_WRITE16_MEMBER ( saturn_vdp1_regs_w );
	DECLARE_WRITE32_MEMBER ( saturn_vdp1_vram_w );
	DECLARE_WRITE32_MEMBER ( saturn_vdp1_framebuffer0_w );

	DECLARE_READ32_MEMBER ( saturn_vdp2_vram_r );
	DECLARE_READ32_MEMBER ( saturn_vdp2_cram_r );
	DECLARE_READ16_MEMBER ( saturn_vdp2_regs_r );

	DECLARE_WRITE32_MEMBER ( saturn_vdp2_vram_w );
	DECLARE_WRITE32_MEMBER ( saturn_vdp2_cram_w );
	DECLARE_WRITE16_MEMBER ( saturn_vdp2_regs_w );


	/* VDP1 */
	void stv_set_framebuffer_config( void );
	void stv_prepare_framebuffers( void );
	void stv_vdp1_change_framebuffers( void );
	void video_update_vdp1( void );
	void stv_vdp1_process_list( void );
	void stv_vdp1_set_drawpixel( void );

	void stv_vdp1_draw_normal_sprite(const rectangle &cliprect, int sprite_type);
	void stv_vdp1_draw_scaled_sprite(const rectangle &cliprect);
	void stv_vdp1_draw_distorted_sprite(const rectangle &cliprect);
	void stv_vdp1_draw_poly_line(const rectangle &cliprect);
	void stv_vdp1_draw_line(const rectangle &cliprect);
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
	void stv_vdp1_setup_shading_for_line(int32_t y, int32_t x1, int32_t x2,
												int32_t r1, int32_t g1, int32_t b1,
												int32_t r2, int32_t g2, int32_t b2);
	void stv_vdp1_setup_shading_for_slope(
							int32_t x1, int32_t x2, int32_t sl1, int32_t sl2, int32_t *nx1, int32_t *nx2,
							int32_t r1, int32_t r2, int32_t slr1, int32_t slr2, int32_t *nr1, int32_t *nr2,
							int32_t g1, int32_t g2, int32_t slg1, int32_t slg2, int32_t *ng1, int32_t *ng2,
							int32_t b1, int32_t b2, int32_t slb1, int32_t slb2, int32_t *nb1, int32_t *nb2,
							int32_t _y1, int32_t y2);
	uint16_t stv_vdp1_apply_gouraud_shading( int x, int y, uint16_t pix );
	void stv_vdp1_setup_shading(const struct spoint* q, const rectangle &cliprect);
	uint8_t stv_read_gouraud_table( void );
	void stv_clear_gouraud_shading(void);

	void stv_clear_framebuffer( int which_framebuffer );
	void stv_vdp1_state_save_postload( void );
	int stv_vdp1_start ( void );

	struct stv_vdp1_poly_scanline
	{
		int32_t   x[2];
		int32_t   b[2];
		int32_t   g[2];
		int32_t   r[2];
		int32_t   db;
		int32_t   dg;
		int32_t   dr;
	};

	struct stv_vdp1_poly_scanline_data
	{
		int32_t   sy, ey;
		struct  stv_vdp1_poly_scanline scanline[512];
	};

	std::unique_ptr<struct stv_vdp1_poly_scanline_data> stv_vdp1_shading_data;

	struct stv_vdp2_sprite_list
	{
		int CMDCTRL, CMDLINK, CMDPMOD, CMDCOLR, CMDSRCA, CMDSIZE, CMDGRDA;
		int CMDXA, CMDYA;
		int CMDXB, CMDYB;
		int CMDXC, CMDYC;
		int CMDXD, CMDYD;

		int ispoly;

	} stv2_current_sprite;

	/* Gouraud shading */

	struct _stv_gouraud_shading
	{
		/* Gouraud shading table */
		uint16_t  GA;
		uint16_t  GB;
		uint16_t  GC;
		uint16_t  GD;
	} stv_gouraud_shading;

	uint16_t m_sprite_colorbank;

	/* VDP1 Framebuffer handling */
	int      stv_sprite_priorities_used[8];
	int      stv_sprite_priorities_usage_valid;
	uint8_t    stv_sprite_priorities_in_fb_line[512][8];


	/* VDP2 */

	uint8_t get_vblank( void );
	uint8_t get_hblank( void );
	int get_hcounter( void );
	int get_vcounter( void );
	int get_vblank_duration( void );
	int get_hblank_duration( void );
	int get_pixel_clock( void );
	uint8_t get_odd_bit( void );
	void stv_vdp2_dynamic_res_change( void );
	int get_vblank_start_position( void );
	int get_ystep_count( void );

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

	void stv_vdp2_drawgfxzoom(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx, uint32_t code,uint32_t color,int flipx,int flipy,int sx,int sy,int transparency,int transparent_color,int scalex, int scaley,int sprite_screen_width, int sprite_screen_height, int alpha);
	void stv_vdp2_drawgfxzoom_rgb555(bitmap_rgb32 &dest_bmp,const rectangle &clip,uint32_t code,uint32_t color,int flipx,int flipy,int sx,int sy,int transparency,int transparent_color,int scalex, int scaley,int sprite_screen_width, int sprite_screen_height, int alpha);
	void stv_vdp2_drawgfx_rgb555( bitmap_rgb32 &dest_bmp, const rectangle &clip, uint32_t code, int flipx, int flipy, int sx, int sy, int transparency, int alpha);
	void stv_vdp2_drawgfx_rgb888( bitmap_rgb32 &dest_bmp, const rectangle &clip, uint32_t code, int flipx, int flipy, int sx, int sy, int transparency, int alpha);

	void stv_vdp2_drawgfx_alpha(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx, uint32_t code,uint32_t color, int flipx,int flipy,int offsx,int offsy, int transparent_color, int alpha);
	void stv_vdp2_drawgfx_transpen(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx, uint32_t code,uint32_t color, int flipx,int flipy,int offsx,int offsy, int transparent_color);


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
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint8_t pri);
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

//  DECLARE_WRITE_LINE_MEMBER(scudsp_end_w);
//  DECLARE_READ16_MEMBER(scudsp_dma_r);
//  DECLARE_WRITE16_MEMBER(scudsp_dma_w);

//  void debug_scudma_command(int ref, const std::vector<std::string> &params);
//  void debug_scuirq_command(int ref, const std::vector<std::string> &params);
//  void debug_help_command(int ref, const std::vector<std::string> &params);
//  void debug_commands(int ref, const std::vector<std::string> &params);
};


// These two clocks are synthesized by the 315-5746
#define MASTER_CLOCK_352 XTAL(14'318'181)*4
#define MASTER_CLOCK_320 XTAL(14'318'181)*3.75
#define CEF_1   m_vdp1_regs[0x010/2]|=0x0002
#define CEF_0   m_vdp1_regs[0x010/2]&=~0x0002
#define BEF_1   m_vdp1_regs[0x010/2]|=0x0001
#define BEF_0   m_vdp1_regs[0x010/2]&=~0x0001
#define STV_VDP1_TVMR ((m_vdp1_regs[0x000/2])&0xffff)
#define STV_VDP1_VBE  ((STV_VDP1_TVMR & 0x0008) >> 3)
#define STV_VDP1_TVM  ((STV_VDP1_TVMR & 0x0007) >> 0)


extern gfx_decode_entry const gfx_stv[];

#endif // MAME_INCLUDES_SATURN_H
