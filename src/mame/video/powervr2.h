// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef __POWERVR2_H__
#define __POWERVR2_H__

#define MCFG_POWERVR2_ADD(_tag, _irq_cb)                                \
	MCFG_DEVICE_ADD(_tag, POWERVR2, 0)                                  \
	downcast<powervr2_device *>(device)->set_irq_cb(DEVCB_ ## _irq_cb);

class powervr2_device : public device_t,
						public device_video_interface
{
public:
	enum { NUM_BUFFERS = 4 };
	enum {
		EOXFER_YUV_IRQ,
		EOXFER_OPLST_IRQ,
		EOXFER_OPMV_IRQ,
		EOXFER_TRLST_IRQ,
		EOXFER_TRMV_IRQ,
		EOXFER_PTLST_IRQ,
		VBL_IN_IRQ,
		VBL_OUT_IRQ,
		HBL_IN_IRQ,
		EOR_VIDEO_IRQ,
		EOR_TSP_IRQ,
		EOR_ISP_IRQ,
		DMA_PVR_IRQ,
		ERR_ISP_LIMIT_IRQ,
		ERR_PVRIF_ILL_ADDR_IRQ
	};

	DECLARE_ADDRESS_MAP(ta_map, 32);
	DECLARE_ADDRESS_MAP(pd_dma_map, 32);

	struct {
		UINT32 pvr_addr;
		UINT32 sys_addr;
		UINT32 size;
		UINT8 sel;
		UINT8 dir;
		UINT8 flag;
		UINT8 start;
	} m_pvr_dma;

	static const int pvr_parconfseq[];
	static const int pvr_wordsvertex[24];
	static const int pvr_wordspolygon[24];
	int pvr_parameterconfig[128];
	UINT32 dilated0[15][1024];
	UINT32 dilated1[15][1024];
	int dilatechose[64];
	float wbuffer[480][640];


	// the real accumulation buffer is a 32x32x8bpp buffer into which tiles get rendered before they get copied to the framebuffer
	//  our implementation is not currently tile based, and thus the accumulation buffer is screen sized
	std::unique_ptr<bitmap_rgb32> fake_accumulationbuffer_bitmap;

	struct texinfo  {
		UINT32 address, vqbase;
		UINT32 nontextured_pal_int;
		UINT8 nontextured_fpal_a,nontextured_fpal_r,nontextured_fpal_g,nontextured_fpal_b;
		int textured, sizex, sizey, stride, sizes, pf, palette, mode, mipmapped, blend_mode, filter_mode, flip_u, flip_v;
		int coltype;

		UINT32 (powervr2_device::*r)(struct texinfo *t, float x, float y);
		UINT32 (*blend)(UINT32 s, UINT32 d);
		int palbase, cd;
	};

	typedef struct
	{
		float x, y, w, u, v;
	} vert;

	struct strip
	{
		int svert, evert;
		texinfo ti;
	};

	struct receiveddata {
		vert verts[65536];
		strip strips[65536];

		int verts_size, strips_size;
		UINT32 ispbase;
		UINT32 fbwsof1;
		UINT32 fbwsof2;
		int busy;
		int valid;
	};

	enum {
		TEX_FILTER_NEAREST = 0,
		TEX_FILTER_BILINEAR,
		TEX_FILTER_TRILINEAR_A,
		TEX_FILTER_TRILINEAR_B
	};

	int tafifo_pos, tafifo_mask, tafifo_vertexwords, tafifo_listtype;
	int start_render_received;
	int renderselect;
	int listtype_used;
	int alloc_ctrl_OPB_Mode, alloc_ctrl_PT_OPB, alloc_ctrl_TM_OPB, alloc_ctrl_T_OPB, alloc_ctrl_OM_OPB, alloc_ctrl_O_OPB;
	receiveddata grab[NUM_BUFFERS];
	int grabsel;
	int grabsellast;
	UINT32 paracontrol,paratype,endofstrip,listtype,global_paratype,parameterconfig;
	UINT32 groupcontrol,groupen,striplen,userclip;
	UINT32 objcontrol,shadow,volume,coltype,texture,offfset,gouraud,uv16bit;
	UINT32 texturesizes,textureaddress,scanorder,pixelformat;
	UINT32 blend_mode, srcselect,dstselect,fogcontrol,colorclamp, use_alpha;
	UINT32 ignoretexalpha,flipuv,clampuv,filtermode,sstexture,mmdadjust,tsinstruction;
	UINT32 depthcomparemode,cullingmode,zwritedisable,cachebypass,dcalcctrl,volumeinstruction,mipmapped,vqcompressed,strideselect,paletteselector;
	UINT32 nontextured_pal_int;
	float nontextured_fpal_a,nontextured_fpal_r,nontextured_fpal_g,nontextured_fpal_b;

	UINT64 *dc_texture_ram;
	UINT64 *dc_framebuffer_ram;

	UINT64 *pvr2_texture_ram;
	UINT64 *pvr2_framebuffer_ram;
	UINT64 *elan_ram;

	UINT32 debug_dip_status;
	emu_timer *vbout_timer;
	emu_timer *vbin_timer;
	emu_timer *hbin_timer;
	emu_timer *endofrender_timer_isp;
	emu_timer *endofrender_timer_tsp;
	emu_timer *endofrender_timer_video;
	emu_timer *yuv_timer_end;
	UINT32 tafifo_buff[32];
	int scanline;
	int next_y;

	powervr2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	template<class _cb> void set_irq_cb(_cb cb) { irq_cb.set_callback(cb); }

	DECLARE_READ32_MEMBER(  id_r );
	DECLARE_READ32_MEMBER(  revision_r );
	DECLARE_READ32_MEMBER(  softreset_r );
	DECLARE_WRITE32_MEMBER( softreset_w );
	DECLARE_WRITE32_MEMBER( startrender_w );
	DECLARE_READ32_MEMBER(  param_base_r );
	DECLARE_WRITE32_MEMBER( param_base_w );
	DECLARE_READ32_MEMBER(  region_base_r );
	DECLARE_WRITE32_MEMBER( region_base_w );
	DECLARE_READ32_MEMBER(  vo_border_col_r );
	DECLARE_WRITE32_MEMBER( vo_border_col_w );
	DECLARE_READ32_MEMBER(  fb_r_ctrl_r );
	DECLARE_WRITE32_MEMBER( fb_r_ctrl_w );
	DECLARE_READ32_MEMBER(  fb_w_ctrl_r );
	DECLARE_WRITE32_MEMBER( fb_w_ctrl_w );
	DECLARE_READ32_MEMBER(  fb_w_linestride_r );
	DECLARE_WRITE32_MEMBER( fb_w_linestride_w );
	DECLARE_READ32_MEMBER(  fb_r_sof1_r );
	DECLARE_WRITE32_MEMBER( fb_r_sof1_w );
	DECLARE_READ32_MEMBER(  fb_r_sof2_r );
	DECLARE_WRITE32_MEMBER( fb_r_sof2_w );
	DECLARE_READ32_MEMBER(  fb_r_size_r );
	DECLARE_WRITE32_MEMBER( fb_r_size_w );
	DECLARE_READ32_MEMBER(  fb_w_sof1_r );
	DECLARE_WRITE32_MEMBER( fb_w_sof1_w );
	DECLARE_READ32_MEMBER(  fb_w_sof2_r );
	DECLARE_WRITE32_MEMBER( fb_w_sof2_w );
	DECLARE_READ32_MEMBER(  fb_x_clip_r );
	DECLARE_WRITE32_MEMBER( fb_x_clip_w );
	DECLARE_READ32_MEMBER(  fb_y_clip_r );
	DECLARE_WRITE32_MEMBER( fb_y_clip_w );
	DECLARE_READ32_MEMBER(  fpu_param_cfg_r );
	DECLARE_WRITE32_MEMBER( fpu_param_cfg_w );
	DECLARE_READ32_MEMBER(  isp_backgnd_t_r );
	DECLARE_WRITE32_MEMBER( isp_backgnd_t_w );
	DECLARE_READ32_MEMBER(  spg_hblank_int_r );
	DECLARE_WRITE32_MEMBER( spg_hblank_int_w );
	DECLARE_READ32_MEMBER(  spg_vblank_int_r );
	DECLARE_WRITE32_MEMBER( spg_vblank_int_w );
	DECLARE_READ32_MEMBER(  spg_control_r );
	DECLARE_WRITE32_MEMBER( spg_control_w );
	DECLARE_READ32_MEMBER(  spg_hblank_r );
	DECLARE_WRITE32_MEMBER( spg_hblank_w );
	DECLARE_READ32_MEMBER(  spg_load_r );
	DECLARE_WRITE32_MEMBER( spg_load_w );
	DECLARE_READ32_MEMBER(  spg_vblank_r );
	DECLARE_WRITE32_MEMBER( spg_vblank_w );
	DECLARE_READ32_MEMBER(  spg_width_r );
	DECLARE_WRITE32_MEMBER( spg_width_w );
	DECLARE_READ32_MEMBER(  text_control_r );
	DECLARE_WRITE32_MEMBER( text_control_w );
	DECLARE_READ32_MEMBER(  vo_control_r );
	DECLARE_WRITE32_MEMBER( vo_control_w );
	DECLARE_READ32_MEMBER(  vo_startx_r );
	DECLARE_WRITE32_MEMBER( vo_startx_w );
	DECLARE_READ32_MEMBER(  vo_starty_r );
	DECLARE_WRITE32_MEMBER( vo_starty_w );
	DECLARE_READ32_MEMBER(  pal_ram_ctrl_r );
	DECLARE_WRITE32_MEMBER( pal_ram_ctrl_w );
	DECLARE_READ32_MEMBER(  spg_status_r );

	DECLARE_READ32_MEMBER(  ta_ol_base_r );
	DECLARE_WRITE32_MEMBER( ta_ol_base_w );
	DECLARE_READ32_MEMBER(  ta_isp_base_r );
	DECLARE_WRITE32_MEMBER( ta_isp_base_w );
	DECLARE_READ32_MEMBER(  ta_ol_limit_r );
	DECLARE_WRITE32_MEMBER( ta_ol_limit_w );
	DECLARE_READ32_MEMBER(  ta_isp_limit_r );
	DECLARE_WRITE32_MEMBER( ta_isp_limit_w );
	DECLARE_READ32_MEMBER(  ta_next_opb_r );
	DECLARE_READ32_MEMBER(  ta_itp_current_r );
	DECLARE_READ32_MEMBER(  ta_alloc_ctrl_r );
	DECLARE_WRITE32_MEMBER( ta_alloc_ctrl_w );
	DECLARE_READ32_MEMBER(  ta_list_init_r );
	DECLARE_WRITE32_MEMBER( ta_list_init_w );
	DECLARE_READ32_MEMBER(  ta_yuv_tex_base_r );
	DECLARE_WRITE32_MEMBER( ta_yuv_tex_base_w );
	DECLARE_READ32_MEMBER(  ta_yuv_tex_ctrl_r );
	DECLARE_WRITE32_MEMBER( ta_yuv_tex_ctrl_w );
	DECLARE_READ32_MEMBER(  ta_yuv_tex_cnt_r );
	DECLARE_WRITE32_MEMBER( ta_yuv_tex_cnt_w );
	DECLARE_WRITE32_MEMBER( ta_list_cont_w );
	DECLARE_READ32_MEMBER(  ta_next_opb_init_r );
	DECLARE_WRITE32_MEMBER( ta_next_opb_init_w );


	DECLARE_READ32_MEMBER(  fog_table_r );
	DECLARE_WRITE32_MEMBER( fog_table_w );
	DECLARE_READ32_MEMBER(  palette_r );
	DECLARE_WRITE32_MEMBER( palette_w );

	DECLARE_READ32_MEMBER(  sb_pdstap_r );
	DECLARE_WRITE32_MEMBER( sb_pdstap_w );
	DECLARE_READ32_MEMBER(  sb_pdstar_r );
	DECLARE_WRITE32_MEMBER( sb_pdstar_w );
	DECLARE_READ32_MEMBER(  sb_pdlen_r );
	DECLARE_WRITE32_MEMBER( sb_pdlen_w );
	DECLARE_READ32_MEMBER(  sb_pddir_r );
	DECLARE_WRITE32_MEMBER( sb_pddir_w );
	DECLARE_READ32_MEMBER(  sb_pdtsel_r );
	DECLARE_WRITE32_MEMBER( sb_pdtsel_w );
	DECLARE_READ32_MEMBER(  sb_pden_r );
	DECLARE_WRITE32_MEMBER( sb_pden_w );
	DECLARE_READ32_MEMBER(  sb_pdst_r );
	DECLARE_WRITE32_MEMBER( sb_pdst_w );
	DECLARE_READ32_MEMBER(  sb_pdapro_r );
	DECLARE_WRITE32_MEMBER( sb_pdapro_w );

	DECLARE_READ32_MEMBER(  pvr2_ta_r );
	DECLARE_WRITE32_MEMBER( pvr2_ta_w );
	DECLARE_WRITE32_MEMBER( pvrs_ta_w );
	DECLARE_READ32_MEMBER(  elan_regs_r );
	DECLARE_WRITE32_MEMBER( elan_regs_w );
	DECLARE_WRITE64_MEMBER( ta_fifo_poly_w );
	DECLARE_WRITE8_MEMBER( ta_fifo_yuv_w );
	DECLARE_WRITE64_MEMBER( ta_texture_directpath0_w );
	DECLARE_WRITE64_MEMBER( ta_texture_directpath1_w );

	TIMER_CALLBACK_MEMBER(vbin);
	TIMER_CALLBACK_MEMBER(vbout);
	TIMER_CALLBACK_MEMBER(hbin);
	TIMER_CALLBACK_MEMBER(yuv_convert_end);
	TIMER_CALLBACK_MEMBER(endofrender_video);
	TIMER_CALLBACK_MEMBER(endofrender_tsp);
	TIMER_CALLBACK_MEMBER(endofrender_isp);
	TIMER_CALLBACK_MEMBER(transfer_opaque_list_irq);
	TIMER_CALLBACK_MEMBER(transfer_opaque_modifier_volume_list_irq);
	TIMER_CALLBACK_MEMBER(transfer_translucent_list_irq);
	TIMER_CALLBACK_MEMBER(transfer_translucent_modifier_volume_list_irq);
	TIMER_CALLBACK_MEMBER(transfer_punch_through_list_irq);
	TIMER_CALLBACK_MEMBER(pvr_dma_irq);

	void pvr_dma_execute(address_space &space);
	void pvr_scanline_timer(int vpos);
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_write8 irq_cb;
	required_ioport m_mamedebug;

	// Core registers
	UINT32 softreset;
	UINT32 param_base, region_base;
	UINT32 vo_border_col;
	UINT32 fb_r_ctrl, fb_w_ctrl, fb_w_linestride, fb_r_sof1, fb_r_sof2, fb_r_size, fb_w_sof1, fb_w_sof2, fb_x_clip, fb_y_clip;
	UINT32 fpu_param_cfg;
	UINT32 isp_backgnd_t;
	UINT32 spg_hblank_int, spg_vblank_int, spg_control, spg_hblank, spg_load, spg_vblank, spg_width;
	UINT32 vo_control, vo_startx, vo_starty;
	UINT32 text_control;
	UINT32 pal_ram_ctrl;

	// TA registers
	UINT32 ta_ol_base, ta_ol_limit, ta_isp_base, ta_isp_limit;
	UINT32 ta_next_opb, ta_itp_current, ta_alloc_ctrl, ta_next_opb_init;
	UINT32 ta_yuv_tex_base, ta_yuv_tex_ctrl, ta_yuv_tex_cnt;
	UINT32 ta_yuv_index;
	int ta_yuv_x,ta_yuv_y;
	int ta_yuv_x_size,ta_yuv_y_size;
	UINT8 yuv_fifo[384];

	// Other registers
	UINT32 fog_table[0x80];
	UINT32 palette[0x400];

	// PD DMA registers
	UINT32 sb_pdstap, sb_pdstar, sb_pdlen, sb_pddir, sb_pdtsel, sb_pden, sb_pdst, sb_pdapro;

	static UINT32 (*const blend_functions[64])(UINT32 s, UINT32 d);

	static inline INT32 clamp(INT32 in, INT32 min, INT32 max);
	static inline UINT32 bilinear_filter(UINT32 c0, UINT32 c1, UINT32 c2, UINT32 c3, float u, float v);
	static inline UINT32 bla(UINT32 c, UINT32 a);
	static inline UINT32 blia(UINT32 c, UINT32 a);
	static inline UINT32 blc(UINT32 c1, UINT32 c2);
	static inline UINT32 blic(UINT32 c1, UINT32 c2);
	static inline UINT32 bls(UINT32 c1, UINT32 c2);
	static UINT32 bl00(UINT32 s, UINT32 d);
	static UINT32 bl01(UINT32 s, UINT32 d);
	static UINT32 bl02(UINT32 s, UINT32 d);
	static UINT32 bl03(UINT32 s, UINT32 d);
	static UINT32 bl04(UINT32 s, UINT32 d);
	static UINT32 bl05(UINT32 s, UINT32 d);
	static UINT32 bl06(UINT32 s, UINT32 d);
	static UINT32 bl07(UINT32 s, UINT32 d);
	static UINT32 bl10(UINT32 s, UINT32 d);
	static UINT32 bl11(UINT32 s, UINT32 d);
	static UINT32 bl12(UINT32 s, UINT32 d);
	static UINT32 bl13(UINT32 s, UINT32 d);
	static UINT32 bl14(UINT32 s, UINT32 d);
	static UINT32 bl15(UINT32 s, UINT32 d);
	static UINT32 bl16(UINT32 s, UINT32 d);
	static UINT32 bl17(UINT32 s, UINT32 d);
	static UINT32 bl20(UINT32 s, UINT32 d);
	static UINT32 bl21(UINT32 s, UINT32 d);
	static UINT32 bl22(UINT32 s, UINT32 d);
	static UINT32 bl23(UINT32 s, UINT32 d);
	static UINT32 bl24(UINT32 s, UINT32 d);
	static UINT32 bl25(UINT32 s, UINT32 d);
	static UINT32 bl26(UINT32 s, UINT32 d);
	static UINT32 bl27(UINT32 s, UINT32 d);
	static UINT32 bl30(UINT32 s, UINT32 d);
	static UINT32 bl31(UINT32 s, UINT32 d);
	static UINT32 bl32(UINT32 s, UINT32 d);
	static UINT32 bl33(UINT32 s, UINT32 d);
	static UINT32 bl34(UINT32 s, UINT32 d);
	static UINT32 bl35(UINT32 s, UINT32 d);
	static UINT32 bl36(UINT32 s, UINT32 d);
	static UINT32 bl37(UINT32 s, UINT32 d);
	static UINT32 bl40(UINT32 s, UINT32 d);
	static UINT32 bl41(UINT32 s, UINT32 d);
	static UINT32 bl42(UINT32 s, UINT32 d);
	static UINT32 bl43(UINT32 s, UINT32 d);
	static UINT32 bl44(UINT32 s, UINT32 d);
	static UINT32 bl45(UINT32 s, UINT32 d);
	static UINT32 bl46(UINT32 s, UINT32 d);
	static UINT32 bl47(UINT32 s, UINT32 d);
	static UINT32 bl50(UINT32 s, UINT32 d);
	static UINT32 bl51(UINT32 s, UINT32 d);
	static UINT32 bl52(UINT32 s, UINT32 d);
	static UINT32 bl53(UINT32 s, UINT32 d);
	static UINT32 bl54(UINT32 s, UINT32 d);
	static UINT32 bl55(UINT32 s, UINT32 d);
	static UINT32 bl56(UINT32 s, UINT32 d);
	static UINT32 bl57(UINT32 s, UINT32 d);
	static UINT32 bl60(UINT32 s, UINT32 d);
	static UINT32 bl61(UINT32 s, UINT32 d);
	static UINT32 bl62(UINT32 s, UINT32 d);
	static UINT32 bl63(UINT32 s, UINT32 d);
	static UINT32 bl64(UINT32 s, UINT32 d);
	static UINT32 bl65(UINT32 s, UINT32 d);
	static UINT32 bl66(UINT32 s, UINT32 d);
	static UINT32 bl67(UINT32 s, UINT32 d);
	static UINT32 bl70(UINT32 s, UINT32 d);
	static UINT32 bl71(UINT32 s, UINT32 d);
	static UINT32 bl72(UINT32 s, UINT32 d);
	static UINT32 bl73(UINT32 s, UINT32 d);
	static UINT32 bl74(UINT32 s, UINT32 d);
	static UINT32 bl75(UINT32 s, UINT32 d);
	static UINT32 bl76(UINT32 s, UINT32 d);
	static UINT32 bl77(UINT32 s, UINT32 d);
	static inline UINT32 cv_1555(UINT16 c);
	static inline UINT32 cv_1555z(UINT16 c);
	static inline UINT32 cv_565(UINT16 c);
	static inline UINT32 cv_565z(UINT16 c);
	static inline UINT32 cv_4444(UINT16 c);
	static inline UINT32 cv_4444z(UINT16 c);
	static inline UINT32 cv_yuv(UINT16 c1, UINT16 c2, int x);
	UINT32 tex_r_yuv_n(texinfo *t, float x, float y);
	UINT32 tex_r_yuv_tw(texinfo *t, float x, float y);
//  UINT32 tex_r_yuv_vq(texinfo *t, float x, float y);
	UINT32 tex_r_1555_n(texinfo *t, float x, float y);
	UINT32 tex_r_1555_tw(texinfo *t, float x, float y);
	UINT32 tex_r_1555_vq(texinfo *t, float x, float y);
	UINT32 tex_r_565_n(texinfo *t, float x, float y);
	UINT32 tex_r_565_tw(texinfo *t, float x, float y);
	UINT32 tex_r_565_vq(texinfo *t, float x, float y);
	UINT32 tex_r_4444_n(texinfo *t, float x, float y);
	UINT32 tex_r_4444_tw(texinfo *t, float x, float y);
	UINT32 tex_r_4444_vq(texinfo *t, float x, float y);
	UINT32 tex_r_p4_1555_tw(texinfo *t, float x, float y);
	UINT32 tex_r_p4_1555_vq(texinfo *t, float x, float y);
	UINT32 tex_r_p4_565_tw(texinfo *t, float x, float y);
	UINT32 tex_r_p4_565_vq(texinfo *t, float x, float y);
	UINT32 tex_r_p4_4444_tw(texinfo *t, float x, float y);
	UINT32 tex_r_p4_4444_vq(texinfo *t, float x, float y);
	UINT32 tex_r_p4_8888_tw(texinfo *t, float x, float y);
	UINT32 tex_r_p4_8888_vq(texinfo *t, float x, float y);
	UINT32 tex_r_p8_1555_tw(texinfo *t, float x, float y);
	UINT32 tex_r_p8_1555_vq(texinfo *t, float x, float y);
	UINT32 tex_r_p8_565_tw(texinfo *t, float x, float y);
	UINT32 tex_r_p8_565_vq(texinfo *t, float x, float y);
	UINT32 tex_r_p8_4444_tw(texinfo *t, float x, float y);
	UINT32 tex_r_p8_4444_vq(texinfo *t, float x, float y);
	UINT32 tex_r_p8_8888_tw(texinfo *t, float x, float y);
	UINT32 tex_r_p8_8888_vq(texinfo *t, float x, float y);

	UINT32 tex_r_nt_palint(texinfo *t, float x, float y);
	UINT32 tex_r_nt_palfloat(texinfo *t, float x, float y);

	UINT32 tex_r_default(texinfo *t, float x, float y);
	void tex_get_info(texinfo *t);

	void render_hline(bitmap_rgb32 &bitmap, texinfo *ti, int y, float xl, float xr, float ul, float ur, float vl, float vr, float wl, float wr);
	void render_span(bitmap_rgb32 &bitmap, texinfo *ti,
						float y0, float y1,
						float xl, float xr,
						float ul, float ur,
						float vl, float vr,
						float wl, float wr,
						float dxldy, float dxrdy,
						float duldy, float durdy,
						float dvldy, float dvrdy,
						float dwldy, float dwrdy);
	void sort_vertices(const vert *v, int *i0, int *i1, int *i2);
	void render_tri_sorted(bitmap_rgb32 &bitmap, texinfo *ti, const vert *v0, const vert *v1, const vert *v2);
	void render_tri(bitmap_rgb32 &bitmap, texinfo *ti, const vert *v);
	void render_to_accumulation_buffer(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void pvr_accumulationbuffer_to_framebuffer(address_space &space, int x, int y);
	void pvr_drawframebuffer(bitmap_rgb32 &bitmap,const rectangle &cliprect);
	static UINT32 dilate0(UINT32 value,int bits);
	static UINT32 dilate1(UINT32 value,int bits);
	void computedilated();
	void pvr_build_parameterconfig();
	void process_ta_fifo();
	void update_screen_format();

	void fb_convert_0555krgb_to_555rgb(address_space &space, int x, int y);
	void fb_convert_0555krgb_to_565rgb(address_space &space, int x, int y);
	void fb_convert_0555krgb_to_888rgb24(address_space &space, int x, int y);
	void fb_convert_0555krgb_to_888rgb32(address_space &space, int x, int y);

	void fb_convert_0565rgb_to_555rgb(address_space &space, int x, int y);
	void fb_convert_0565rgb_to_565rgb(address_space &space, int x, int y);
	void fb_convert_0565rgb_to_888rgb24(address_space &space, int x, int y);
	void fb_convert_0565rgb_to_888rgb32(address_space &space, int x, int y);

	void fb_convert_1555argb_to_555rgb(address_space &space, int x, int y);
	void fb_convert_1555argb_to_565rgb(address_space &space, int x, int y);
	void fb_convert_1555argb_to_888rgb24(address_space &space, int x, int y);
	void fb_convert_1555argb_to_888rgb32(address_space &space, int x, int y);

	void fb_convert_888rgb_to_555rgb(address_space &space, int x, int y);
	void fb_convert_888rgb_to_565rgb(address_space &space, int x, int y);
	void fb_convert_888rgb_to_888rgb24(address_space &space, int x, int y);
	void fb_convert_888rgb_to_888rgb32(address_space &space, int x, int y);

	void fb_convert_8888argb_to_555rgb(address_space &space, int x, int y);
	void fb_convert_8888argb_to_565rgb(address_space &space, int x, int y);
	void fb_convert_8888argb_to_888rgb24(address_space &space, int x, int y);
	void fb_convert_8888argb_to_888rgb32(address_space &space, int x, int y);

};

extern const device_type POWERVR2;

#endif
