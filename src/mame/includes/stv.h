// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, Olivier Galibert, Mariusz Wojcieszek, R. Belmont
/*----------- defined in drivers/stv.c -----------*/
#include "cdrom.h"
#include "machine/eepromser.h"
#include "cpu/m68000/m68000.h"
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/scudsp/scudsp.h"
#include "cpu/sh2/sh2.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "machine/315-5881_crypt.h"
#include "machine/315-5838_317-0229_comp.h"

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
			m_palette(*this, "palette")
	{
	}

	required_shared_ptr<UINT32> m_rom;
	required_shared_ptr<UINT32> m_workram_l;
	required_shared_ptr<UINT32> m_workram_h;
	required_shared_ptr<UINT16> m_sound_ram;
	optional_ioport m_fake_comms;

	memory_region *m_cart_reg[4];
	std::unique_ptr<UINT8[]>     m_backupram;
	std::unique_ptr<UINT32[]>    m_scu_regs;
	std::unique_ptr<UINT16[]>    m_scsp_regs;
	std::unique_ptr<UINT16[]>    m_vdp2_regs;
	std::unique_ptr<UINT32[]>    m_vdp2_vram;
	std::unique_ptr<UINT32[]>    m_vdp2_cram;
	std::unique_ptr<UINT32[]>    m_vdp1_vram;
	std::unique_ptr<UINT16[]>    m_vdp1_regs;

	UINT8     m_NMI_reset;
	UINT8     m_en_68k;


	struct {
		UINT32    src[3];       /* Source DMA lv n address*/
		UINT32    dst[3];       /* Destination DMA lv n address*/
		UINT32    src_add[3];   /* Source Addition for DMA lv n*/
		UINT32    dst_add[3];   /* Destination Addition for DMA lv n*/
		UINT32    size[3];      /* Transfer DMA size lv n*/
		UINT32    index[3];
		int       start_factor[3];
		UINT8     enable_mask[3];
		UINT32    ist;
		UINT32    ism;
		UINT32    illegal_factor[3];
		UINT32    status;
	}m_scu;

	void scu_reset(void);

	int       m_minit_boost;
	int       m_sinit_boost;
	attotime  m_minit_boost_timeslice;
	attotime  m_sinit_boost_timeslice;

	struct {
		UINT16    **framebuffer_display_lines;
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
		std::unique_ptr<UINT16[]>   framebuffer[2];
		UINT16    **framebuffer_draw_lines;
		std::unique_ptr<UINT8[]>     gfx_decode;
		UINT16    lopr;
		UINT16    copr;
		UINT16    ewdr;

		int       local_x;
		int       local_y;
	}m_vdp1;

	struct {
		std::unique_ptr<UINT8[]>      gfx_decode;
		bitmap_rgb32 roz_bitmap[2];
		UINT8     dotsel;
		UINT8     pal;
		UINT16    h_count;
		UINT16    v_count;
		UINT8     exltfg;
		UINT8     exsyfg;
		int       old_crmd;
		int       old_tvmd;
	}m_vdp2;

	struct {
		UINT8 IOSEL1;
		UINT8 IOSEL2;
		UINT8 EXLE1;
		UINT8 EXLE2;
		UINT8 PDR1;
		UINT8 PDR2;
		UINT8 DDR1;
		UINT8 DDR2;
		UINT8 SF;
		UINT8 SR;
		UINT8 IREG[7];
		UINT8 intback_buf[7];
		UINT8 OREG[32];
		int   intback_stage;
		int   pmode;
		UINT8 SMEM[4];
		UINT8 intback;
		UINT8 rtc_data[7];
		UINT8 slave_on;
	}m_smpc;

	struct {
		UINT8 status;
		UINT8 data;
		UINT8 prev_data;
		UINT16 repeat_count;
	}m_keyb;

	/* Saturn specific*/
	int m_saturn_region;
	UINT8 m_cart_type;
	UINT32 *m_cart_dram;

	/* ST-V specific */
	UINT8     m_stv_multi_bank;
	UINT8     m_prev_bankswitch;
	emu_timer *m_stv_rtc_timer;
	UINT8     m_port_sel,m_mux_data;
	UINT8     m_system_output;
	UINT16    m_serial_tx;

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



	bitmap_rgb32 m_tmpbitmap;
	DECLARE_VIDEO_START(stv_vdp2);
	UINT32 screen_update_saturn(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_stv_vdp2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(saturn_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(saturn_slave_scanline);

	void scu_do_transfer(UINT8 event);
	void scu_test_pending_irq();
	DECLARE_READ32_MEMBER(saturn_scu_r);
	DECLARE_WRITE32_MEMBER(saturn_scu_w);
	TIMER_CALLBACK_MEMBER(dma_lv0_ended);
	TIMER_CALLBACK_MEMBER(dma_lv1_ended);
	TIMER_CALLBACK_MEMBER(dma_lv2_ended);
	TIMER_CALLBACK_MEMBER(vdp1_draw_end);
	void scu_single_transfer(address_space &space, UINT32 src, UINT32 dst,UINT8 *src_shift);
	void scu_dma_direct(address_space &space, UINT8 dma_ch);
	void scu_dma_indirect(address_space &space,UINT8 dma_ch);
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

	UINT8 smpc_direct_mode(UINT8 pad_n);
	UINT8 smpc_th_control_mode(UINT8 pad_n);
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
	void vdp1_fill_line(const rectangle &cliprect, int patterndata, int xsize, INT32 y, INT32 x1, INT32 x2, INT32 u1, INT32 u2, INT32 v1, INT32 v2);
	void (saturn_state::*drawpixel)(int x, int y, int patterndata, int offsetcnt);
	void drawpixel_poly(int x, int y, int patterndata, int offsetcnt);
	void drawpixel_8bpp_trans(int x, int y, int patterndata, int offsetcnt);
	void drawpixel_4bpp_notrans(int x, int y, int patterndata, int offsetcnt);
	void drawpixel_4bpp_trans(int x, int y, int patterndata, int offsetcnt);
	void drawpixel_generic(int x, int y, int patterndata, int offsetcnt);
	void vdp1_fill_slope(const rectangle &cliprect, int patterndata, int xsize,
							INT32 x1, INT32 x2, INT32 sl1, INT32 sl2, INT32 *nx1, INT32 *nx2,
							INT32 u1, INT32 u2, INT32 slu1, INT32 slu2, INT32 *nu1, INT32 *nu2,
							INT32 v1, INT32 v2, INT32 slv1, INT32 slv2, INT32 *nv1, INT32 *nv2,
							INT32 _y1, INT32 y2);
	void stv_vdp1_setup_shading_for_line(INT32 y, INT32 x1, INT32 x2,
												INT32 r1, INT32 g1, INT32 b1,
												INT32 r2, INT32 g2, INT32 b2);
	void stv_vdp1_setup_shading_for_slope(
							INT32 x1, INT32 x2, INT32 sl1, INT32 sl2, INT32 *nx1, INT32 *nx2,
							INT32 r1, INT32 r2, INT32 slr1, INT32 slr2, INT32 *nr1, INT32 *nr2,
							INT32 g1, INT32 g2, INT32 slg1, INT32 slg2, INT32 *ng1, INT32 *ng2,
							INT32 b1, INT32 b2, INT32 slb1, INT32 slb2, INT32 *nb1, INT32 *nb2,
							INT32 _y1, INT32 y2);
	UINT16 stv_vdp1_apply_gouraud_shading( int x, int y, UINT16 pix );
	void stv_vdp1_setup_shading(const struct spoint* q, const rectangle &cliprect);
	UINT8 stv_read_gouraud_table( void );
	void stv_clear_gouraud_shading(void);

	void stv_clear_framebuffer( int which_framebuffer );
	void stv_vdp1_state_save_postload( void );
	int stv_vdp1_start ( void );

	struct stv_vdp1_poly_scanline
	{
		INT32   x[2];
		INT32   b[2];
		INT32   g[2];
		INT32   r[2];
		INT32   db;
		INT32   dg;
		INT32   dr;
	};

	struct stv_vdp1_poly_scanline_data
	{
		INT32   sy, ey;
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
		UINT16  GA;
		UINT16  GB;
		UINT16  GC;
		UINT16  GD;
	} stv_gouraud_shading;

	UINT16 m_sprite_colorbank;

	/* VDP1 Framebuffer handling */
	int      stv_sprite_priorities_used[8];
	int      stv_sprite_priorities_usage_valid;
	UINT8    stv_sprite_priorities_in_fb_line[512][8];


	/* VDP2 */

	UINT8 get_vblank( void );
	UINT8 get_hblank( void );
	int get_hcounter( void );
	int get_vcounter( void );
	int get_vblank_duration( void );
	int get_hblank_duration( void );
	int get_pixel_clock( void );
	UINT8 get_odd_bit( void );
	void stv_vdp2_dynamic_res_change( void );
	int get_vblank_start_position( void );
	int get_ystep_count( void );

	void refresh_palette_data( void );
	inline int stv_vdp2_window_process(int x,int y);
	void stv_vdp2_get_window0_coordinates(int *s_x, int *e_x, int *s_y, int *e_y);
	void stv_vdp2_get_window1_coordinates(int *s_x, int *e_x, int *s_y, int *e_y);
	int get_window_pixel(int s_x,int e_x,int s_y,int e_y,int x, int y,UINT8 win_num);
	int stv_vdp2_apply_window_on_layer(rectangle &cliprect);

	void stv_vdp2_draw_basic_tilemap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_draw_basic_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_4bpp_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_8bpp_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_11bpp_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_rgb15_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_rgb32_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void stv_vdp2_drawgfxzoom(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx, UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,int transparency,int transparent_color,int scalex, int scaley,int sprite_screen_width, int sprite_screen_height, int alpha);
	void stv_vdp2_drawgfxzoom_rgb555(bitmap_rgb32 &dest_bmp,const rectangle &clip,UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,int transparency,int transparent_color,int scalex, int scaley,int sprite_screen_width, int sprite_screen_height, int alpha);
	void stv_vdp2_drawgfx_rgb555( bitmap_rgb32 &dest_bmp, const rectangle &clip, UINT32 code, int flipx, int flipy, int sx, int sy, int transparency, int alpha);
	void stv_vdp2_drawgfx_rgb888( bitmap_rgb32 &dest_bmp, const rectangle &clip, UINT32 code, int flipx, int flipy, int sx, int sy, int transparency, int alpha);

	void stv_vdp2_drawgfx_alpha(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx, UINT32 code,UINT32 color, int flipx,int flipy,int offsx,int offsy, int transparent_color, int alpha);
	void stv_vdp2_drawgfx_transpen(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx, UINT32 code,UINT32 color, int flipx,int flipy,int offsx,int offsy, int transparent_color);


	void stv_vdp2_draw_rotation_screen(bitmap_rgb32 &bitmap, const rectangle &cliprect, int iRP);
	void stv_vdp2_check_tilemap_with_linescroll(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_check_tilemap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_copy_roz_bitmap(bitmap_rgb32 &bitmap, bitmap_rgb32 &roz_bitmap, const rectangle &cliprect, int iRP, int planesizex, int planesizey, int planerenderedsizex, int planerenderedsizey);
	void stv_vdp2_fill_rotation_parameter_table( UINT8 rot_parameter );
	UINT8 stv_vdp2_check_vram_cycle_pattern_registers( UINT8 access_command_pnmdr, UINT8 access_command_cpdr, UINT8 bitmap_enable );
	UINT8 stv_vdp2_is_rotation_applied(void);
	UINT8 stv_vdp2_are_map_registers_equal(void);
	void stv_vdp2_get_map_page( int x, int y, int *_map, int *_page );

	void stv_vdp2_draw_mosaic(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT8 is_roz);
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
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT8 pri);
	int true_vcount[263][4];

	void stv_vdp2_state_save_postload( void );
	void stv_vdp2_exit ( void );
	int stv_vdp2_start ( void );

	UINT8 m_vdpdebug_roz;

	struct stv_vdp2_tilemap_capabilities
	{
		UINT8  enabled;
		UINT8  transparency;
		UINT8  colour_calculation_enabled;
		UINT8  colour_depth;
		UINT8  alpha;
		UINT8  tile_size;
		UINT8  bitmap_enable;
		UINT8  bitmap_size;
		UINT8  bitmap_palette_number;
		UINT8  bitmap_map;
		UINT16 map_offset[16];
		UINT8  map_count;

		UINT8  pattern_data_size;
		UINT8  character_number_supplement;
		UINT8  special_priority_register;
		UINT8  special_colour_control_register;
		UINT8  supplementary_palette_bits;
		UINT8  supplementary_character_bits;

		INT16 scrollx;
		INT16 scrolly;
		UINT32 incx, incy;

		UINT8   linescroll_enable;
		UINT8   linescroll_interval;
		UINT32  linescroll_table_address;
		UINT8   vertical_linescroll_enable;
		UINT8   linezoom_enable;

		UINT8  plane_size;
		UINT8  colour_ram_address_offset;
		UINT8  fade_control;
		struct{
			UINT8 logic;
			UINT8 enabled[2];
			UINT8 area[2];
		}window_control;

		UINT8  line_screen_enabled;
		UINT8  mosaic_screen_enabled;

		int layer_name; /* just to keep track */
	} stv2_current_tilemap;

	struct rotation_table
	{
		INT32   xst;
		INT32   yst;
		INT32   zst;
		INT32   dxst;
		INT32   dyst;
		INT32   dx;
		INT32   dy;
		INT32   A;
		INT32   B;
		INT32   C;
		INT32   D;
		INT32   E;
		INT32   F;
		INT32   px;
		INT32   py;
		INT32   pz;
		INT32   cx;
		INT32   cy;
		INT32   cz;
		INT32   mx;
		INT32   my;
		INT32   kx;
		INT32   ky;
		UINT32  kast;
		INT32   dkast;
		INT32   dkax;

	} stv_current_rotation_parameter_table;

	struct _stv_vdp2_layer_data_placement
	{
		UINT32  map_offset_min;
		UINT32  map_offset_max;
		UINT32  tile_offset_min;
		UINT32  tile_offset_max;
	} stv_vdp2_layer_data_placement;

	struct _stv_rbg_cache_data
	{
		UINT8   watch_vdp2_vram_writes;
		UINT8   is_cache_dirty;

		UINT32  map_offset_min[2];
		UINT32  map_offset_max[2];
		UINT32  tile_offset_min[2];
		UINT32  tile_offset_max[2];

		struct stv_vdp2_tilemap_capabilities    layer_data[2];

	} stv_rbg_cache_data;

	/* stvcd */
	DECLARE_READ32_MEMBER( stvcd_r );
	DECLARE_WRITE32_MEMBER( stvcd_w );

	TIMER_DEVICE_CALLBACK_MEMBER( stv_sector_cb );
	TIMER_DEVICE_CALLBACK_MEMBER( stv_sh1_sim );

	struct direntryT
	{
		UINT8 record_size;
		UINT8 xa_record_size;
		UINT32 firstfad;        // first sector of file
		UINT32 length;      // length of file
		UINT8 year;
		UINT8 month;
		UINT8 day;
		UINT8 hour;
		UINT8 minute;
		UINT8 second;
		UINT8 gmt_offset;
		UINT8 flags;        // iso9660 flags
		UINT8 file_unit_size;
		UINT8 interleave_gap_size;
		UINT16 volume_sequencer_number;
		UINT8 name[128];
	};

	struct filterT
	{
		UINT8 mode;
		UINT8 chan;
		UINT8 smmask;
		UINT8 cimask;
		UINT8 fid;
		UINT8 smval;
		UINT8 cival;
		UINT8 condtrue;
		UINT8 condfalse;
		UINT32 fad;
		UINT32 range;
	};

	struct blockT
	{
		INT32 size; // size of block
		INT32 FAD;  // FAD on disc
		UINT8 data[CD_MAX_SECTOR_DATA];
		UINT8 chan; // channel
		UINT8 fnum; // file number
		UINT8 subm; // subchannel mode
		UINT8 cinf; // coding information
	};

	struct partitionT
	{
		INT32 size;
		blockT *blocks[MAX_BLOCKS];
		UINT8 bnum[MAX_BLOCKS];
		UINT8 numblks;
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

	int get_track_index(UINT32 fad);
	int sega_cdrom_get_adr_control(cdrom_file *file, int track);
	void cr_standard_return(UINT16 cur_status);
	void cd_free_block(blockT *blktofree);
	void cd_defragblocks(partitionT *part);
	void cd_getsectoroffsetnum(UINT32 bufnum, UINT32 *sectoffs, UINT32 *sectnum);

	UINT16 cd_readWord(UINT32 addr);
	void cd_writeWord(UINT32 addr, UINT16 data);
	UINT32 cd_readLong(UINT32 addr);
	void cd_writeLong(UINT32 addr, UINT32 data);

	void cd_readTOC(void);
	void cd_readblock(UINT32 fad, UINT8 *dat);
	void cd_playdata(void);

	void cd_exec_command( void );
	// iso9660 utilities
	void make_dir_current(UINT32 fad);
	void read_new_dir(UINT32 fileno);

	blockT *cd_alloc_block(UINT8 *blknum);
	partitionT *cd_filterdata(filterT *flt, int trktype, UINT8 *p_ok);
	partitionT *cd_read_filtered_sector(INT32 fad, UINT8 *p_ok);

	cdrom_file *cdrom;// = (cdrom_file *)NULL;

	// local variables
	timer_device *sector_timer;
	timer_device *sh1_timer;
	partitionT partitions[MAX_FILTERS];
	partitionT *transpart;

	blockT blocks[MAX_BLOCKS];
	blockT curblock;

	UINT8 tocbuf[102*4];
	UINT8 subqbuf[5*2];
	UINT8 subrwbuf[12*2];
	UINT8 finfbuf[256];

	INT32 sectlenin, sectlenout;

	UINT8 lastbuf, playtype;

	transT xfertype;
	trans32T xfertype32;
	UINT32 xfercount, calcsize;
	UINT32 xferoffs, xfersect, xfersectpos, xfersectnum, xferdnum;

	filterT filters[MAX_FILTERS];
	filterT *cddevice;
	int cddevicenum;

	UINT16 cr1, cr2, cr3, cr4;
	UINT16 prev_cr1, prev_cr2, prev_cr3, prev_cr4;
	UINT8 status_type;
	UINT16 hirqmask, hirqreg;
	UINT16 cd_stat;
	UINT32 cd_curfad;// = 0;
	UINT32 cd_fad_seek;
	UINT32 fadstoplay;// = 0;
	UINT32 in_buffer;// = 0;    // amount of data in the buffer
	int oddframe;// = 0;
	int buffull, sectorstore, freeblocks;
	int cur_track;
	UINT8 cmd_pending;
	UINT8 cd_speed;
	UINT8 cdda_maxrepeat;
	UINT8 cdda_repeat_count;
	UINT8 tray_is_closed;
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
	void smpc_digital_pad(UINT8 pad_num, UINT8 offset);
	void smpc_analog_pad(UINT8 pad_num, UINT8 offset, UINT8 id);
	void smpc_keyboard(UINT8 pad_num, UINT8 offset);
	void smpc_mouse(UINT8 pad_num, UINT8 offset, UINT8 id);
	void smpc_md_pad(UINT8 pad_num, UINT8 offset, UINT8 id);
	void smpc_unconnected(UINT8 pad_num, UINT8 offset);
	TIMER_CALLBACK_MEMBER( intback_peripheral );
	TIMER_CALLBACK_MEMBER( saturn_smpc_intback );
	void smpc_rtc_write();
	void smpc_memory_setting();
	void smpc_nmi_req();
	TIMER_CALLBACK_MEMBER( smpc_nmi_set );
	void smpc_comreg_exec(address_space &space, UINT8 data, UINT8 is_stv);
	DECLARE_READ8_MEMBER( stv_SMPC_r );
	DECLARE_WRITE8_MEMBER( stv_SMPC_w );

};

class stv_state : public saturn_state
{
public:
	stv_state(const machine_config &mconfig, device_type type, const char *tag)
		: saturn_state(mconfig, type, tag),
		m_adsp(*this, "adsp"),
		m_adsp_pram(*this, "adsp_pram"),
		m_cryptdevice(*this, "315_5881"),
		m_5838crypt(*this, "315_5838")
	{
	}

	DECLARE_DRIVER_INIT(astrass);
	DECLARE_DRIVER_INIT(batmanfr);
	DECLARE_DRIVER_INIT(finlarch);
	DECLARE_DRIVER_INIT(decathlt);
	DECLARE_DRIVER_INIT(sanjeon);
	DECLARE_DRIVER_INIT(puyosun);
	DECLARE_DRIVER_INIT(winterht);
	DECLARE_DRIVER_INIT(gaxeduel);
	DECLARE_DRIVER_INIT(rsgun);
	DECLARE_DRIVER_INIT(groovef);
	DECLARE_DRIVER_INIT(sandor);
	DECLARE_DRIVER_INIT(cottonbm);
	DECLARE_DRIVER_INIT(smleague);
	DECLARE_DRIVER_INIT(nameclv3);
	DECLARE_DRIVER_INIT(danchiq);
	DECLARE_DRIVER_INIT(hanagumi);
	DECLARE_DRIVER_INIT(cotton2);
	DECLARE_DRIVER_INIT(seabass);
	DECLARE_DRIVER_INIT(stv);
	DECLARE_DRIVER_INIT(thunt);
	DECLARE_DRIVER_INIT(critcrsh);
	DECLARE_DRIVER_INIT(stvmp);
	DECLARE_DRIVER_INIT(sasissu);
	DECLARE_DRIVER_INIT(dnmtdeka);
	DECLARE_DRIVER_INIT(ffreveng);
	DECLARE_DRIVER_INIT(fhboxers);
	DECLARE_DRIVER_INIT(pblbeach);
	DECLARE_DRIVER_INIT(sss);
	DECLARE_DRIVER_INIT(diehard);
	DECLARE_DRIVER_INIT(danchih);
	DECLARE_DRIVER_INIT(shienryu);
	DECLARE_DRIVER_INIT(elandore);
	DECLARE_DRIVER_INIT(prikura);
	DECLARE_DRIVER_INIT(maruchan);
	DECLARE_DRIVER_INIT(colmns97);
	DECLARE_DRIVER_INIT(grdforce);
	DECLARE_DRIVER_INIT(suikoenb);
	DECLARE_DRIVER_INIT(magzun);
	DECLARE_DRIVER_INIT(shanhigw);
	DECLARE_DRIVER_INIT(sokyugrt);
	DECLARE_DRIVER_INIT(vfremix);
	DECLARE_DRIVER_INIT(twcup98);
	DECLARE_DRIVER_INIT(znpwfv);
	DECLARE_DRIVER_INIT(othellos);
	DECLARE_DRIVER_INIT(mausuke);

	DECLARE_READ8_MEMBER(stv_ioga_r);
	DECLARE_WRITE8_MEMBER(stv_ioga_w);
	DECLARE_READ8_MEMBER(critcrsh_ioga_r);
	DECLARE_READ8_MEMBER(magzun_ioga_r);
	DECLARE_WRITE8_MEMBER(magzun_ioga_w);
	DECLARE_READ8_MEMBER(stvmp_ioga_r);
	DECLARE_WRITE8_MEMBER(stvmp_ioga_w);
	DECLARE_READ32_MEMBER(stv_ioga_r32);
	DECLARE_WRITE32_MEMBER(stv_ioga_w32);
	DECLARE_READ32_MEMBER(critcrsh_ioga_r32);
	DECLARE_READ32_MEMBER(stvmp_ioga_r32);
	DECLARE_WRITE32_MEMBER(stvmp_ioga_w32);
	DECLARE_READ32_MEMBER(magzun_ioga_r32);
	DECLARE_WRITE32_MEMBER(magzun_ioga_w32);
	DECLARE_READ32_MEMBER(magzun_hef_hack_r);
	DECLARE_READ32_MEMBER(magzun_rx_hack_r);

	int load_cart(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( stv_cart1 ) { return load_cart(image, m_cart1); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( stv_cart2 ) { return load_cart(image, m_cart2); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( stv_cart3 ) { return load_cart(image, m_cart3); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( stv_cart4 ) { return load_cart(image, m_cart4); }

	void install_stvbios_speedups( void );

	DECLARE_MACHINE_START(stv);
	DECLARE_MACHINE_RESET(stv);

	/* Batman Forever specifics */
	optional_device<adsp2181_device>    m_adsp;
	optional_shared_ptr<UINT32> m_adsp_pram;

	struct
	{
		UINT16 bdma_internal_addr;
		UINT16 bdma_external_addr;
		UINT16 bdma_control;
		UINT16 bdma_word_count;
	} m_adsp_regs;

	DECLARE_MACHINE_RESET(batmanfr);
	DECLARE_READ16_MEMBER( adsp_control_r );
	DECLARE_WRITE16_MEMBER( adsp_control_w );
	DECLARE_WRITE32_MEMBER(batmanfr_sound_comms_w);

	// protection specific variables and functions (see machine/stvprot.c)
	UINT32 m_abus_protenable;
	UINT32 m_abus_protkey;

	UINT32 m_a_bus[4];

	DECLARE_READ32_MEMBER( common_prot_r );
	DECLARE_WRITE32_MEMBER( common_prot_w );

	void install_common_protection();
	void stv_register_protection_savestates();



	optional_device<sega_315_5881_crypt_device> m_cryptdevice;
	optional_device<sega_315_5838_comp_device> m_5838crypt;
	UINT16 crypt_read_callback(UINT32 addr);
	UINT16 crypt_read_callback_ch1(UINT32 addr);
	UINT16 crypt_read_callback_ch2(UINT32 addr);
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
