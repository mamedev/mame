// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, Olivier Galibert, Mariusz Wojcieszek, R. Belmont

#include "cdrom.h"
#include "machine/eepromser.h"
#include "cpu/m68000/m68000.h"
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/scudsp/scudsp.h"
#include "cpu/sh2/sh2.h"

#include "bus/sat_ctrl/ctrl.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "machine/315-5881_crypt.h"
#include "machine/315-5838_317-0229_comp.h"

#include "debug/debugcon.h"
#include "debug/debugcmd.h"
#include "debugger.h"

#define MAX_FILTERS (24)
#define MAX_BLOCKS  (200)
#define MAX_DIR_SIZE    (256*1024)

class saturn_state : public driver_device
{
public:
	saturn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_rom(*this, "share6"),
			m_workram_l(*this, "workram_l"),
			m_workram_h(*this, "workram_h"),
			m_sound_ram(*this, "sound_ram"),
			m_fake_comms(*this, "fake"),
			m_maincpu(*this, "maincpu"),
			m_slave(*this, "slave"),
			m_audiocpu(*this, "audiocpu"),
			m_scudsp(*this, "scudsp"),
			m_eeprom(*this, "eeprom"),
			m_cart1(*this, "stv_slot1"),
			m_cart2(*this, "stv_slot2"),
			m_cart3(*this, "stv_slot3"),
			m_cart4(*this, "stv_slot4"),
			m_gfxdecode(*this, "gfxdecode"),
			m_palette(*this, "palette"),
			m_ctrl1(*this, "ctrl1"),
			m_ctrl2(*this, "ctrl2")
	{
	}

	required_shared_ptr<uint32_t> m_rom;
	required_shared_ptr<uint32_t> m_workram_l;
	required_shared_ptr<uint32_t> m_workram_h;
	required_shared_ptr<uint16_t> m_sound_ram;
	optional_ioport m_fake_comms;

	memory_region *m_cart_reg[4];
	std::unique_ptr<uint8_t[]>     m_backupram;
	std::unique_ptr<uint32_t[]>    m_scu_regs;
	std::unique_ptr<uint16_t[]>    m_scsp_regs;
	std::unique_ptr<uint16_t[]>    m_vdp2_regs;
	std::unique_ptr<uint32_t[]>    m_vdp2_vram;
	std::unique_ptr<uint32_t[]>    m_vdp2_cram;
	std::unique_ptr<uint32_t[]>    m_vdp1_vram;
	std::unique_ptr<uint16_t[]>    m_vdp1_regs;

	uint8_t     m_NMI_reset;
	uint8_t     m_en_68k;


	struct {
		uint32_t    src[3];       /* Source DMA lv n address*/
		uint32_t    dst[3];       /* Destination DMA lv n address*/
		uint32_t    src_add[3];   /* Source Addition for DMA lv n*/
		uint32_t    dst_add[3];   /* Destination Addition for DMA lv n*/
		uint32_t    size[3];      /* Transfer DMA size lv n*/
		uint32_t    index[3];
		int       start_factor[3];
		uint8_t     enable_mask[3];
		uint32_t    ist;
		uint32_t    ism;
		uint32_t    illegal_factor[3];
		uint32_t    status;
	}m_scu;

	void scu_reset(void);

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
		uint16_t    h_count;
		uint16_t    v_count;
		uint8_t     exltfg;
		uint8_t     exsyfg;
		int       old_crmd;
		int       old_tvmd;
	}m_vdp2;

	struct {
		uint8_t IOSEL1;
		uint8_t IOSEL2;
		uint8_t EXLE1;
		uint8_t EXLE2;
		uint8_t PDR1;
		uint8_t PDR2;
		uint8_t DDR1;
		uint8_t DDR2;
		uint8_t SF;
		uint8_t SR;
		uint8_t IREG[7];
		uint8_t intback_buf[7];
		uint8_t OREG[32];
		int   intback_stage;
		int   pmode;
		uint8_t SMEM[4];
		uint8_t intback;
		uint8_t rtc_data[7];
		uint8_t slave_on;
	}m_smpc;

	/* Saturn specific*/
	int m_saturn_region;
	uint8_t m_cart_type;
	uint32_t *m_cart_dram;

	/* ST-V specific */
	uint8_t     m_stv_multi_bank;
	uint8_t     m_prev_bankswitch;
	emu_timer *m_stv_rtc_timer;
	uint8_t     m_port_sel,m_mux_data;
	uint8_t     m_system_output;
	uint16_t    m_serial_tx;

	required_device<sh2_device> m_maincpu;
	required_device<sh2_device> m_slave;
	required_device<m68000_base_device> m_audiocpu;
	required_device<scudsp_cpu_device> m_scudsp;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<generic_slot_device> m_cart1;
	optional_device<generic_slot_device> m_cart2;
	optional_device<generic_slot_device> m_cart3;
	optional_device<generic_slot_device> m_cart4;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<saturn_control_port_device> m_ctrl1;
	optional_device<saturn_control_port_device> m_ctrl2;

	bitmap_rgb32 m_tmpbitmap;
	DECLARE_VIDEO_START(stv_vdp2);
	uint32_t screen_update_stv_vdp2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(saturn_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(saturn_slave_scanline);

	void scu_do_transfer(uint8_t event);
	void scu_test_pending_irq();
	DECLARE_READ32_MEMBER(saturn_scu_r);
	DECLARE_WRITE32_MEMBER(saturn_scu_w);
	TIMER_CALLBACK_MEMBER(dma_lv0_ended);
	TIMER_CALLBACK_MEMBER(dma_lv1_ended);
	TIMER_CALLBACK_MEMBER(dma_lv2_ended);
	TIMER_CALLBACK_MEMBER(vdp1_draw_end);
	void scu_single_transfer(address_space &space, uint32_t src, uint32_t dst,uint8_t *src_shift);
	void scu_dma_direct(address_space &space, uint8_t dma_ch);
	void scu_dma_indirect(address_space &space,uint8_t dma_ch);
	DECLARE_WRITE16_MEMBER(saturn_soundram_w);
	DECLARE_READ16_MEMBER(saturn_soundram_r);
	DECLARE_WRITE32_MEMBER(minit_w);
	DECLARE_WRITE32_MEMBER(sinit_w);
	DECLARE_WRITE32_MEMBER(saturn_minit_w);
	DECLARE_WRITE32_MEMBER(saturn_sinit_w);
	DECLARE_READ8_MEMBER(saturn_backupram_r);
	DECLARE_WRITE8_MEMBER(saturn_backupram_w);
	TIMER_CALLBACK_MEMBER(stv_rtc_increment);
	DECLARE_WRITE_LINE_MEMBER(scsp_to_main_irq);
	DECLARE_WRITE8_MEMBER(scsp_irq);
	int m_scsp_last_line;

	uint8_t smpc_direct_mode(uint8_t pad_n);
	uint8_t smpc_th_control_mode(uint8_t pad_n);
	TIMER_CALLBACK_MEMBER( smpc_audio_reset_line_pulse );
	DECLARE_READ8_MEMBER( saturn_SMPC_r );
	DECLARE_WRITE8_MEMBER( saturn_SMPC_w );

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
	void stv_vdp2_get_window0_coordinates(int *s_x, int *e_x, int *s_y, int *e_y);
	void stv_vdp2_get_window1_coordinates(int *s_x, int *e_x, int *s_y, int *e_y);
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

	/* stvcd */
	DECLARE_READ32_MEMBER( stvcd_r );
	DECLARE_WRITE32_MEMBER( stvcd_w );

	TIMER_DEVICE_CALLBACK_MEMBER( stv_sector_cb );
	TIMER_DEVICE_CALLBACK_MEMBER( stv_sh1_sim );

	struct direntryT
	{
		uint8_t record_size;
		uint8_t xa_record_size;
		uint32_t firstfad;        // first sector of file
		uint32_t length;      // length of file
		uint8_t year;
		uint8_t month;
		uint8_t day;
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
		uint8_t gmt_offset;
		uint8_t flags;        // iso9660 flags
		uint8_t file_unit_size;
		uint8_t interleave_gap_size;
		uint16_t volume_sequencer_number;
		uint8_t name[128];
	};

	struct filterT
	{
		uint8_t mode;
		uint8_t chan;
		uint8_t smmask;
		uint8_t cimask;
		uint8_t fid;
		uint8_t smval;
		uint8_t cival;
		uint8_t condtrue;
		uint8_t condfalse;
		uint32_t fad;
		uint32_t range;
	};

	struct blockT
	{
		int32_t size; // size of block
		int32_t FAD;  // FAD on disc
		uint8_t data[CD_MAX_SECTOR_DATA];
		uint8_t chan; // channel
		uint8_t fnum; // file number
		uint8_t subm; // subchannel mode
		uint8_t cinf; // coding information
	};

	struct partitionT
	{
		int32_t size;
		blockT *blocks[MAX_BLOCKS];
		uint8_t bnum[MAX_BLOCKS];
		uint8_t numblks;
	};

	// 16-bit transfer types
	enum transT
	{
		XFERTYPE_INVALID,
		XFERTYPE_TOC,
		XFERTYPE_FILEINFO_1,
		XFERTYPE_FILEINFO_254,
		XFERTYPE_SUBQ,
		XFERTYPE_SUBRW
	};

	// 32-bit transfer types
	enum trans32T
	{
		XFERTYPE32_INVALID,
		XFERTYPE32_GETSECTOR,
		XFERTYPE32_GETDELETESECTOR,
		XFERTYPE32_PUTSECTOR,
		XFERTYPE32_MOVESECTOR
	};


	void stvcd_reset(void);
	void stvcd_exit(void);
	void stvcd_set_tray_open(void);
	void stvcd_set_tray_close(void);

	int get_track_index(uint32_t fad);
	int sega_cdrom_get_adr_control(cdrom_file *file, int track);
	void cr_standard_return(uint16_t cur_status);
	void cd_free_block(blockT *blktofree);
	void cd_defragblocks(partitionT *part);
	void cd_getsectoroffsetnum(uint32_t bufnum, uint32_t *sectoffs, uint32_t *sectnum);

	uint16_t cd_readWord(uint32_t addr);
	void cd_writeWord(uint32_t addr, uint16_t data);
	uint32_t cd_readLong(uint32_t addr);
	void cd_writeLong(uint32_t addr, uint32_t data);

	void cd_readTOC(void);
	void cd_readblock(uint32_t fad, uint8_t *dat);
	void cd_playdata(void);

	void cd_exec_command( void );
	// iso9660 utilities
	void make_dir_current(uint32_t fad);
	void read_new_dir(uint32_t fileno);

	blockT *cd_alloc_block(uint8_t *blknum);
	partitionT *cd_filterdata(filterT *flt, int trktype, uint8_t *p_ok);
	partitionT *cd_read_filtered_sector(int32_t fad, uint8_t *p_ok);

	cdrom_file *cdrom;// = (cdrom_file *)nullptr;

	// local variables
	timer_device *sector_timer;
	timer_device *sh1_timer;
	partitionT partitions[MAX_FILTERS];
	partitionT *transpart;

	blockT blocks[MAX_BLOCKS];
	blockT curblock;

	uint8_t tocbuf[102*4];
	uint8_t subqbuf[5*2];
	uint8_t subrwbuf[12*2];
	uint8_t finfbuf[256];

	int32_t sectlenin, sectlenout;

	uint8_t lastbuf, playtype;

	transT xfertype;
	trans32T xfertype32;
	uint32_t xfercount, calcsize;
	uint32_t xferoffs, xfersect, xfersectpos, xfersectnum, xferdnum;

	filterT filters[MAX_FILTERS];
	filterT *cddevice;
	int cddevicenum;

	uint16_t cr1, cr2, cr3, cr4;
	uint16_t prev_cr1, prev_cr2, prev_cr3, prev_cr4;
	uint8_t status_type;
	uint16_t hirqmask, hirqreg;
	uint16_t cd_stat;
	uint32_t cd_curfad;// = 0;
	uint32_t cd_fad_seek;
	uint32_t fadstoplay;// = 0;
	uint32_t in_buffer;// = 0;    // amount of data in the buffer
	int oddframe;// = 0;
	int buffull, sectorstore, freeblocks;
	int cur_track;
	uint8_t cmd_pending;
	uint8_t cd_speed;
	uint8_t cdda_maxrepeat;
	uint8_t cdda_repeat_count;
	uint8_t tray_is_closed;
	int get_timing_command( void );

	direntryT curroot;       // root entry of current filesystem
	std::vector<direntryT> curdir;       // current directory
	int numfiles;            // # of entries in current directory
	int firstfile;           // first non-directory file

	DECLARE_WRITE_LINE_MEMBER(m68k_reset_callback);
	int DectoBCD(int num);

	DECLARE_WRITE_LINE_MEMBER(scudsp_end_w);
	DECLARE_READ16_MEMBER(scudsp_dma_r);
	DECLARE_WRITE16_MEMBER(scudsp_dma_w);

	// FROM smpc.c
	void stv_select_game(int gameno);
	void smpc_master_on();
	TIMER_CALLBACK_MEMBER( smpc_slave_enable );
	TIMER_CALLBACK_MEMBER( smpc_sound_enable );
	TIMER_CALLBACK_MEMBER( smpc_cd_enable );
	void smpc_system_reset();
	TIMER_CALLBACK_MEMBER( smpc_change_clock );
	TIMER_CALLBACK_MEMBER( stv_intback_peripheral );
	TIMER_CALLBACK_MEMBER( stv_smpc_intback );
	TIMER_CALLBACK_MEMBER( intback_peripheral );
	TIMER_CALLBACK_MEMBER( saturn_smpc_intback );
	void smpc_rtc_write();
	void smpc_memory_setting();
	void smpc_nmi_req();
	TIMER_CALLBACK_MEMBER( smpc_nmi_set );
	void smpc_comreg_exec(address_space &space, uint8_t data, uint8_t is_stv);
	DECLARE_READ8_MEMBER( stv_SMPC_r );
	DECLARE_WRITE8_MEMBER( stv_SMPC_w );

	void debug_scudma_command(int ref, const std::vector<std::string> &params);
	void debug_scuirq_command(int ref, const std::vector<std::string> &params);
	void debug_help_command(int ref, const std::vector<std::string> &params);
	void debug_commands(int ref, const std::vector<std::string> &params);

};


#define MASTER_CLOCK_352 57272720
#define MASTER_CLOCK_320 53693174
#define CEF_1   m_vdp1_regs[0x010/2]|=0x0002
#define CEF_0   m_vdp1_regs[0x010/2]&=~0x0002
#define BEF_1   m_vdp1_regs[0x010/2]|=0x0001
#define BEF_0   m_vdp1_regs[0x010/2]&=~0x0001
#define STV_VDP1_TVMR ((m_vdp1_regs[0x000/2])&0xffff)
#define STV_VDP1_VBE  ((STV_VDP1_TVMR & 0x0008) >> 3)
#define STV_VDP1_TVM  ((STV_VDP1_TVMR & 0x0007) >> 0)

#define IRQ_VBLANK_IN  1 << 0
#define IRQ_VBLANK_OUT 1 << 1
#define IRQ_HBLANK_IN  1 << 2
#define IRQ_TIMER_0    1 << 3
#define IRQ_TIMER_1    1 << 4
#define IRQ_DSP_END    1 << 5
#define IRQ_SOUND_REQ  1 << 6
#define IRQ_SMPC       1 << 7
#define IRQ_PAD        1 << 8
#define IRQ_DMALV2     1 << 9
#define IRQ_DMALV1     1 << 10
#define IRQ_DMALV0     1 << 11
#define IRQ_DMAILL     1 << 12
#define IRQ_VDP1_END   1 << 13
#define IRQ_ABUS       1 << 15

GFXDECODE_EXTERN( stv );
