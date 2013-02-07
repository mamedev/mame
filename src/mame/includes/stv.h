/*----------- defined in drivers/stv.c -----------*/

class saturn_state : public driver_device
{
public:
	saturn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_workram_l(*this, "workram_l"),
			m_workram_h(*this, "workram_h"),
			m_sound_ram(*this, "sound_ram"),
			m_fake_comms(*this, "fake")
			{ }

	required_shared_ptr<UINT32> m_workram_l;
	required_shared_ptr<UINT32> m_workram_h;
	required_shared_ptr<UINT16> m_sound_ram;
	optional_ioport m_fake_comms;

	UINT8     *m_backupram;
	UINT8     *m_cart_backupram;
	UINT32    *m_scu_regs;
	UINT16    *m_scsp_regs;
	UINT16    *m_vdp2_regs;
	UINT32    *m_vdp2_vram;
	UINT32    *m_vdp2_cram;
	UINT32    *m_vdp1_vram;
	UINT16    *m_vdp1_regs;

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
	}m_scu;

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
		UINT16    *framebuffer[2];
		UINT16    **framebuffer_draw_lines;
		UINT8     *gfx_decode;
		UINT16    lopr;
		UINT16    copr;
		UINT16    ewdr;

		int       local_x;
		int       local_y;
	}m_vdp1;

	struct {
		UINT8     *gfx_decode;
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

	legacy_cpu_device* m_maincpu;
	legacy_cpu_device* m_slave;
	legacy_cpu_device* m_audiocpu;

	bitmap_rgb32 m_tmpbitmap;
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
	DECLARE_READ32_MEMBER(astrass_hack_r);
	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);
	DECLARE_INPUT_CHANGED_MEMBER(nmi_reset);
	DECLARE_INPUT_CHANGED_MEMBER(tray_open);
	DECLARE_INPUT_CHANGED_MEMBER(tray_close);
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

	DECLARE_DRIVER_INIT(saturnus);
	DECLARE_DRIVER_INIT(saturneu);
	DECLARE_DRIVER_INIT(saturnjp);
	DECLARE_MACHINE_START(saturn);
	DECLARE_MACHINE_RESET(saturn);
	DECLARE_VIDEO_START(stv_vdp2);
	DECLARE_MACHINE_START(stv);
	DECLARE_MACHINE_RESET(stv);
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
	DECLARE_READ8_MEMBER(saturn_cart_type_r);
	TIMER_CALLBACK_MEMBER(stv_rtc_increment);
	DECLARE_READ32_MEMBER(saturn_null_ram_r);
	DECLARE_WRITE32_MEMBER(saturn_null_ram_w);
	DECLARE_READ32_MEMBER(saturn_cart_dram0_r);
	DECLARE_WRITE32_MEMBER(saturn_cart_dram0_w);
	DECLARE_READ32_MEMBER(saturn_cart_dram1_r);
	DECLARE_WRITE32_MEMBER(saturn_cart_dram1_w);
	DECLARE_READ32_MEMBER(saturn_cs1_r);
	DECLARE_WRITE32_MEMBER(saturn_cs1_w);
	WRITE_LINE_MEMBER(scsp_to_main_irq);
	void saturn_init_driver(int rgn);

	int m_scsp_last_line;

	UINT8 smpc_direct_mode(UINT8 pad_n);
	UINT8 smpc_th_control_mode(UINT8 pad_n);
	DECLARE_READ8_MEMBER( saturn_SMPC_r );
	DECLARE_WRITE8_MEMBER( saturn_SMPC_w );

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( sat_cart );
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( stv_cart );

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
//	void (*drawpixel)(int x, int y, int patterndata, int offsetcnt);
//	void drawpixel_poly(int x, int y, int patterndata, int offsetcnt);
//	void drawpixel_8bpp_trans(int x, int y, int patterndata, int offsetcnt);
//	void drawpixel_4bpp_notrans(int x, int y, int patterndata, int offsetcnt);
//	void drawpixel_4bpp_trans(int x, int y, int patterndata, int offsetcnt);
//	void drawpixel_generic(int x, int y, int patterndata, int offsetcnt);
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

	struct stv_vdp1_poly_scanline_data* stv_vdp1_shading_data;

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
	int get_vblank_duration( void );
	int get_hblank_duration( void );
	int get_pixel_clock( void );
	UINT8 get_odd_bit( void );
	void stv_vdp2_dynamic_res_change( void );

	void refresh_palette_data( void );
	int stv_vdp2_window_process(int x,int y);
	void stv_vdp2_get_window0_coordinates(UINT16 *s_x, UINT16 *e_x, UINT16 *s_y, UINT16 *e_y);
	void stv_vdp2_get_window1_coordinates(UINT16 *s_x, UINT16 *e_x, UINT16 *s_y, UINT16 *e_y);
	int get_window_pixel(UINT16 s_x,UINT16 e_x,UINT16 s_y,UINT16 e_y,int x, int y,UINT8 win_num);
	int stv_vdp2_apply_window_on_layer(rectangle &cliprect);

	void stv_vdp2_draw_basic_tilemap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_draw_basic_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void stv_vdp2_drawgfxzoom(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx, UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,int transparency,int transparent_color,int scalex, int scaley,int sprite_screen_width, int sprite_screen_height, int alpha);
	void stv_vdp2_drawgfxzoom_rgb555(bitmap_rgb32 &dest_bmp,const rectangle &clip,UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,int transparency,int transparent_color,int scalex, int scaley,int sprite_screen_width, int sprite_screen_height, int alpha);
	void stv_vdp2_drawgfx_rgb555( bitmap_rgb32 &dest_bmp, const rectangle &clip, UINT32 code, int flipx, int flipy, int sx, int sy, int transparency, int alpha);
	void stv_vdp2_drawgfx_rgb888( bitmap_rgb32 &dest_bmp, const rectangle &clip, UINT32 code, int flipx, int flipy, int sx, int sy, int transparency, int alpha);

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
	void stv_vdp2_compute_color_offset_UINT32(UINT32 *rgb, int cor);
	void stv_vdp2_check_fade_control_for_layer( void );

	void stv_vdp2_draw_line(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_draw_back(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_draw_NBG0(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_draw_NBG1(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_draw_NBG2(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_draw_NBG3(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void stv_vdp2_draw_RBG0(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT8 pri);

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
		UINT8  window_control;

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



/*----------- defined in drivers/stv.c -----------*/

void install_stvbios_speedups(running_machine &machine);

/*----------- defined in video/stvvdp1.c -----------*/

extern UINT16   **stv_framebuffer_display_lines;
extern int stv_framebuffer_double_interlace;
extern int stv_framebuffer_mode;
extern UINT8* stv_vdp1_gfx_decode;

int stv_vdp1_start ( running_machine &machine );
void video_update_vdp1(running_machine &machine);
void stv_vdp2_dynamic_res_change(running_machine &machine);

