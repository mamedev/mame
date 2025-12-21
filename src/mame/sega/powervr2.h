// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_SEGA_POWERVR2_H
#define MAME_SEGA_POWERVR2_H

#pragma once

class powervr2_device : public device_t,
						public device_video_interface
{
public:
	powervr2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	auto irq_callback() { return irq_cb.bind(); }
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

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

	void ta_map(address_map &map) ATTR_COLD;
	void pd_dma_map(address_map &map) ATTR_COLD;

	struct {
		uint32_t pvr_addr;
		uint32_t sys_addr;
		uint32_t size;
		uint8_t sel;
		uint8_t dir;
		uint8_t flag;
		uint8_t start;
	} m_pvr_dma;

	static const int pvr_parconfseq[];
	static const int pvr_wordsvertex[24];
	static const int pvr_wordspolygon[24];
	int pvr_parameterconfig[128];
	uint32_t dilated0[15][1024];
	uint32_t dilated1[15][1024];
	int dilatechose[64];
	float wbuffer[480][640];


	// the real accumulation buffer is a 32x32x8bpp buffer into which tiles get rendered before they get copied to the framebuffer
	//  our implementation is not currently tile based, and thus the accumulation buffer is screen sized
	std::unique_ptr<bitmap_rgb32> fake_accumulationbuffer_bitmap;

	/*
	 * Per-polygon base and offset colors.  These are scaled by per-vertex
	 * weights.
	 *
	 * These are only used if the colortype in the polygon header is 2
	 * or 3.  If it is 0 or 1, then each vertex's base and offset colors are
	 * specified completely independently of one another in the per-vertex
	 * parameters.
	 *
	 * The base color is combined with the texture sample (if any) according
	 * to one of four fixed functions.  The offset color is then added to
	 * the combined texture sample and base color with the exception of
	 * alpha.
	 *
	 * poly_offs_color is not always used.  Not specifying a poly_offs_color
	 * is equivalent to using a poly_offs_color of 0.
	 *
	 * poly_last_mode_2_base_color is used to hold the last base color
	 * specified using color type 2.  Color type 3 will always use the last
	 * base color specified using color type 2.
	 */
	float poly_base_color[4], poly_offs_color[4],
		poly_last_mode_2_base_color[4];

	struct texinfo
	{
		uint32_t address = 0, vqbase = 0;

		uint32_t tsinstruction = 0;

		int textured = 0, sizex = 0, sizey = 0, stride = 0, sizes = 0, pf = 0, palette = 0, mode = 0, mipmapped = 0, blend_mode = 0, filter_mode = 0;
		int coltype = 0;

		uint32_t (powervr2_device::*r)(texinfo *t, float x, float y) = nullptr;
		uint32_t (*blend)(uint32_t s, uint32_t d) = nullptr;
		int (*u_func)(float uv, int size) = nullptr;
		int (*v_func)(float uv, int size) = nullptr;
		int palbase = 0, cd = 0;
	};

	struct vert
	{
		float x = 0, y = 0, w = 0, u = 0, v = 0;

		// base and offset colors
		float b[4] = { 0, 0, 0, 0 }, o[4] = { 0, 0, 0, 0 };
	};

	struct strip
	{
		int svert = 0, evert = 0;
		texinfo ti;
	};

	static const unsigned MAX_VERTS = 65536;
	static const unsigned MAX_STRIPS = 65536;

	/*
	 * There are five polygon lists:
	 *
	 * Opaque
	 * Punch-through polygon
	 * Opaque/punch-through modifier volume
	 * Translucent
	 * Translucent modifier volume
	 *
	 * They are rendered in that order.  List indices are are three bits, so
	 * technically there are 8 polygon lists, but only the first 5 are valid.
	 */
	enum {
		DISPLAY_LIST_OPAQUE,
		DISPLAY_LIST_OPAQUE_MOD,
		DISPLAY_LIST_TRANS,
		DISPLAY_LIST_TRANS_MOD,
		DISPLAY_LIST_PUNCH_THROUGH,
		DISPLAY_LIST_LAST,

		DISPLAY_LIST_COUNT,

		DISPLAY_LIST_NONE = -1
	};

	struct poly_group {
		strip strips[MAX_STRIPS];
		int strips_size = 0;
	};

	struct receiveddata {
		vert verts[MAX_VERTS];
		struct poly_group groups[DISPLAY_LIST_COUNT];
		uint32_t ispbase = 0;
		uint32_t fbwsof1 = 0;
		uint32_t fbwsof2 = 0;
		int busy = 0;
		int valid = 0;
		int verts_size = 0;
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
	std::unique_ptr<receiveddata[]> grab;
	int grabsel;
	int grabsellast;
	uint32_t paracontrol,paratype,endofstrip,listtype,global_paratype,parameterconfig;
	uint32_t groupcontrol,groupen,striplen,userclip;
	uint32_t objcontrol,shadow,volume,coltype,texture,offset_color_enable,gouraud,uv16bit;
	uint32_t texturesizes,textureaddress,scanorder,pixelformat;
	uint32_t blend_mode, srcselect,dstselect,fogcontrol,colorclamp, use_alpha;
	uint32_t ignoretexalpha,flipuv,clampuv,filtermode,sstexture,mmdadjust,tsinstruction;
	uint32_t depthcomparemode,cullingmode,zwritedisable,cachebypass,dcalcctrl,volumeinstruction,mipmapped,vqcompressed,strideselect,paletteselector;

	uint64_t *dc_texture_ram;
	uint64_t *dc_framebuffer_ram;

	uint64_t *pvr2_texture_ram;
	uint64_t *pvr2_framebuffer_ram;

	uint32_t debug_dip_status;
	emu_timer *vbout_timer;
	emu_timer *vbin_timer;
	emu_timer *hbin_timer;
	emu_timer *endofrender_timer_isp;
	emu_timer *endofrender_timer_tsp;
	emu_timer *endofrender_timer_video;
	emu_timer *yuv_timer_end;
	uint32_t tafifo_buff[32];
	int scanline;
	int next_y;

	uint32_t id_r();
	uint32_t revision_r();
	uint32_t softreset_r();
	void softreset_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void startrender_w(address_space &space, uint32_t data);
	uint32_t param_base_r();
	void param_base_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t region_base_r();
	void region_base_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t vo_border_col_r();
	void vo_border_col_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t fb_r_ctrl_r();
	void fb_r_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t fb_w_ctrl_r();
	void fb_w_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t fb_w_linestride_r();
	void fb_w_linestride_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t fb_r_sof1_r();
	void fb_r_sof1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t fb_r_sof2_r();
	void fb_r_sof2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t fb_r_size_r();
	void fb_r_size_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t fb_w_sof1_r();
	void fb_w_sof1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t fb_w_sof2_r();
	void fb_w_sof2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t fb_x_clip_r();
	void fb_x_clip_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t fb_y_clip_r();
	void fb_y_clip_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t fpu_param_cfg_r();
	void fpu_param_cfg_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t isp_backgnd_t_r();
	void isp_backgnd_t_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t spg_hblank_int_r();
	void spg_hblank_int_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t spg_vblank_int_r();
	void spg_vblank_int_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t spg_control_r();
	void spg_control_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t spg_hblank_r();
	void spg_hblank_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t spg_load_r();
	void spg_load_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t spg_vblank_r();
	void spg_vblank_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t spg_width_r();
	void spg_width_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t text_control_r();
	void text_control_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t vo_control_r();
	void vo_control_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t vo_startx_r();
	void vo_startx_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t vo_starty_r();
	void vo_starty_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t pal_ram_ctrl_r();
	void pal_ram_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t spg_status_r();

	uint32_t ta_ol_base_r();
	void ta_ol_base_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ta_isp_base_r();
	void ta_isp_base_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ta_ol_limit_r();
	void ta_ol_limit_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ta_isp_limit_r();
	void ta_isp_limit_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ta_next_opb_r();
	uint32_t ta_itp_current_r();
	uint32_t ta_alloc_ctrl_r();
	void ta_alloc_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ta_list_init_r();
	void ta_list_init_w(uint32_t data);
	uint32_t ta_yuv_tex_base_r();
	void ta_yuv_tex_base_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ta_yuv_tex_ctrl_r();
	void ta_yuv_tex_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ta_yuv_tex_cnt_r();
	void ta_list_cont_w(uint32_t data);
	uint32_t ta_next_opb_init_r();
	void ta_next_opb_init_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);


	uint32_t fog_table_r(offs_t offset);
	void fog_table_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t palette_r(offs_t offset);
	void palette_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t sb_pdstap_r();
	void sb_pdstap_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sb_pdstar_r();
	void sb_pdstar_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sb_pdlen_r();
	void sb_pdlen_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sb_pddir_r();
	void sb_pddir_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sb_pdtsel_r();
	void sb_pdtsel_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sb_pden_r();
	void sb_pden_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sb_pdst_r();
	void sb_pdst_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sb_pdapro_r();
	void sb_pdapro_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t pvr2_ta_r(offs_t offset);

	void ta_fifo_poly_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void ta_fifo_yuv_w(uint8_t data);
	void ta_texture_directpath0_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void ta_texture_directpath1_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);

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
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	typedef uint32_t(powervr2_device::*pix_sample_fn)(texinfo*,float,float,uint32_t,uint32_t);

	inline uint32_t sample_nontextured(texinfo *ti, float u, float v, uint32_t offset_color, uint32_t base_color);

	template <int tsinst, bool bilinear>
		inline uint32_t sample_textured(texinfo *ti, float u, float v, uint32_t offset_color, uint32_t base_color);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	ioport_constructor device_input_ports() const override;

private:
	devcb_write8 irq_cb;
	required_ioport m_mamedebug;

	// Core registers
	uint32_t softreset;
	uint32_t param_base, region_base;
	uint32_t vo_border_col;
	uint32_t fb_r_ctrl, fb_w_ctrl, fb_w_linestride, fb_r_sof1, fb_r_sof2, fb_r_size, fb_w_sof1, fb_w_sof2, fb_x_clip, fb_y_clip;
	uint32_t fpu_param_cfg;
	uint32_t isp_backgnd_t;
	uint32_t spg_hblank_int, spg_vblank_int, spg_control, spg_hblank, spg_load, spg_vblank, spg_width;
	uint32_t vo_control, vo_startx, vo_starty;
	uint32_t text_control;
	uint32_t pal_ram_ctrl;

	// TA registers
	uint32_t ta_ol_base, ta_ol_limit, ta_isp_base, ta_isp_limit;
	uint32_t ta_next_opb, ta_itp_current, ta_alloc_ctrl, ta_next_opb_init;
	uint32_t ta_yuv_tex_base, ta_yuv_tex_ctrl, ta_yuv_tex_cnt;
	uint32_t ta_yuv_index;
	int ta_yuv_u_ptr, ta_yuv_v_ptr;
	int ta_yuv_u_size, ta_yuv_v_size;
	uint8_t yuv_fifo[384];

	// Other registers
	uint32_t fog_table[0x80];
	uint32_t palette[0x400];

	// PD DMA registers
	uint32_t sb_pdstap, sb_pdstar, sb_pdlen, sb_pddir, sb_pdtsel, sb_pden, sb_pdst, sb_pdapro;

	// Timer callbacks
	emu_timer *opaque_irq_timer = nullptr;
	emu_timer *opaque_modifier_volume_irq_timer = nullptr;
	emu_timer *translucent_irq_timer = nullptr;
	emu_timer *translucent_modifier_volume_irq_timer = nullptr;
	emu_timer *punch_through_irq_timer = nullptr;
	emu_timer *dma_irq_timer = nullptr;

	static uint32_t (*const blend_functions[64])(uint32_t s, uint32_t d);

	static int uv_wrap(float uv, int size);
	static int uv_flip(float uv, int size);
	static int uv_clamp(float uv, int size);

	static inline uint32_t float_argb_to_packed_argb(float argb[4]);
	static inline void packed_argb_to_float_argb(float dst[4], uint32_t in);

	static inline int32_t clamp(int32_t in, int32_t min, int32_t max);
	static inline uint32_t bilinear_filter(uint32_t c0, uint32_t c1, uint32_t c2, uint32_t c3, float u, float v);
	static inline uint32_t bla(uint32_t c, uint32_t a);
	static inline uint32_t blia(uint32_t c, uint32_t a);
	static inline uint32_t blc(uint32_t c1, uint32_t c2);
	static inline uint32_t blic(uint32_t c1, uint32_t c2);
	static inline uint32_t bls(uint32_t c1, uint32_t c2);
	static inline uint32_t bls24(uint32_t c1, uint32_t c2);

	static uint32_t bl00(uint32_t s, uint32_t d);
	static uint32_t bl01(uint32_t s, uint32_t d);
	static uint32_t bl02(uint32_t s, uint32_t d);
	static uint32_t bl03(uint32_t s, uint32_t d);
	static uint32_t bl04(uint32_t s, uint32_t d);
	static uint32_t bl05(uint32_t s, uint32_t d);
	static uint32_t bl06(uint32_t s, uint32_t d);
	static uint32_t bl07(uint32_t s, uint32_t d);
	static uint32_t bl10(uint32_t s, uint32_t d);
	static uint32_t bl11(uint32_t s, uint32_t d);
	static uint32_t bl12(uint32_t s, uint32_t d);
	static uint32_t bl13(uint32_t s, uint32_t d);
	static uint32_t bl14(uint32_t s, uint32_t d);
	static uint32_t bl15(uint32_t s, uint32_t d);
	static uint32_t bl16(uint32_t s, uint32_t d);
	static uint32_t bl17(uint32_t s, uint32_t d);
	static uint32_t bl20(uint32_t s, uint32_t d);
	static uint32_t bl21(uint32_t s, uint32_t d);
	static uint32_t bl22(uint32_t s, uint32_t d);
	static uint32_t bl23(uint32_t s, uint32_t d);
	static uint32_t bl24(uint32_t s, uint32_t d);
	static uint32_t bl25(uint32_t s, uint32_t d);
	static uint32_t bl26(uint32_t s, uint32_t d);
	static uint32_t bl27(uint32_t s, uint32_t d);
	static uint32_t bl30(uint32_t s, uint32_t d);
	static uint32_t bl31(uint32_t s, uint32_t d);
	static uint32_t bl32(uint32_t s, uint32_t d);
	static uint32_t bl33(uint32_t s, uint32_t d);
	static uint32_t bl34(uint32_t s, uint32_t d);
	static uint32_t bl35(uint32_t s, uint32_t d);
	static uint32_t bl36(uint32_t s, uint32_t d);
	static uint32_t bl37(uint32_t s, uint32_t d);
	static uint32_t bl40(uint32_t s, uint32_t d);
	static uint32_t bl41(uint32_t s, uint32_t d);
	static uint32_t bl42(uint32_t s, uint32_t d);
	static uint32_t bl43(uint32_t s, uint32_t d);
	static uint32_t bl44(uint32_t s, uint32_t d);
	static uint32_t bl45(uint32_t s, uint32_t d);
	static uint32_t bl46(uint32_t s, uint32_t d);
	static uint32_t bl47(uint32_t s, uint32_t d);
	static uint32_t bl50(uint32_t s, uint32_t d);
	static uint32_t bl51(uint32_t s, uint32_t d);
	static uint32_t bl52(uint32_t s, uint32_t d);
	static uint32_t bl53(uint32_t s, uint32_t d);
	static uint32_t bl54(uint32_t s, uint32_t d);
	static uint32_t bl55(uint32_t s, uint32_t d);
	static uint32_t bl56(uint32_t s, uint32_t d);
	static uint32_t bl57(uint32_t s, uint32_t d);
	static uint32_t bl60(uint32_t s, uint32_t d);
	static uint32_t bl61(uint32_t s, uint32_t d);
	static uint32_t bl62(uint32_t s, uint32_t d);
	static uint32_t bl63(uint32_t s, uint32_t d);
	static uint32_t bl64(uint32_t s, uint32_t d);
	static uint32_t bl65(uint32_t s, uint32_t d);
	static uint32_t bl66(uint32_t s, uint32_t d);
	static uint32_t bl67(uint32_t s, uint32_t d);
	static uint32_t bl70(uint32_t s, uint32_t d);
	static uint32_t bl71(uint32_t s, uint32_t d);
	static uint32_t bl72(uint32_t s, uint32_t d);
	static uint32_t bl73(uint32_t s, uint32_t d);
	static uint32_t bl74(uint32_t s, uint32_t d);
	static uint32_t bl75(uint32_t s, uint32_t d);
	static uint32_t bl76(uint32_t s, uint32_t d);
	static uint32_t bl77(uint32_t s, uint32_t d);
	static inline uint32_t cv_1555(uint16_t c);
	static inline uint32_t cv_1555z(uint16_t c);
	static inline uint32_t cv_565(uint16_t c);
	static inline uint32_t cv_565z(uint16_t c);
	static inline uint32_t cv_4444(uint16_t c);
	static inline uint32_t cv_4444z(uint16_t c);
	static inline uint32_t cv_yuv(uint16_t c1, uint16_t c2, int x);
	uint32_t tex_r_yuv_n(texinfo *t, float x, float y);
	uint32_t tex_r_yuv_tw(texinfo *t, float x, float y);
//  uint32_t tex_r_yuv_vq(texinfo *t, float x, float y);
	uint32_t tex_r_1555_n(texinfo *t, float x, float y);
	uint32_t tex_r_1555_tw(texinfo *t, float x, float y);
	uint32_t tex_r_1555_vq(texinfo *t, float x, float y);
	uint32_t tex_r_565_n(texinfo *t, float x, float y);
	uint32_t tex_r_565_tw(texinfo *t, float x, float y);
	uint32_t tex_r_565_vq(texinfo *t, float x, float y);
	uint32_t tex_r_4444_n(texinfo *t, float x, float y);
	uint32_t tex_r_4444_tw(texinfo *t, float x, float y);
	uint32_t tex_r_4444_vq(texinfo *t, float x, float y);
	uint32_t tex_r_p4_1555_tw(texinfo *t, float x, float y);
	uint32_t tex_r_p4_1555_vq(texinfo *t, float x, float y);
	uint32_t tex_r_p4_565_tw(texinfo *t, float x, float y);
	uint32_t tex_r_p4_565_vq(texinfo *t, float x, float y);
	uint32_t tex_r_p4_4444_tw(texinfo *t, float x, float y);
	uint32_t tex_r_p4_4444_vq(texinfo *t, float x, float y);
	uint32_t tex_r_p4_8888_tw(texinfo *t, float x, float y);
	uint32_t tex_r_p4_8888_vq(texinfo *t, float x, float y);
	uint32_t tex_r_p8_1555_tw(texinfo *t, float x, float y);
	uint32_t tex_r_p8_1555_vq(texinfo *t, float x, float y);
	uint32_t tex_r_p8_565_tw(texinfo *t, float x, float y);
	uint32_t tex_r_p8_565_vq(texinfo *t, float x, float y);
	uint32_t tex_r_p8_4444_tw(texinfo *t, float x, float y);
	uint32_t tex_r_p8_4444_vq(texinfo *t, float x, float y);
	uint32_t tex_r_p8_8888_tw(texinfo *t, float x, float y);
	uint32_t tex_r_p8_8888_vq(texinfo *t, float x, float y);

	uint32_t tex_r_default(texinfo *t, float x, float y);
	void tex_get_info(texinfo *t);

	template <pix_sample_fn sample_fn, int group_no>
		inline void render_hline(bitmap_rgb32 &bitmap, texinfo *ti,
									int y, float xl, float xr,
									float ul, float ur, float vl, float vr,
									float wl, float wr,
									float const bl[4], float const br[4],
									float const offl[4], float const offr[4]);

	template <pix_sample_fn sample_fn, int group_no>
		inline void render_span(bitmap_rgb32 &bitmap, texinfo *ti,
								float y0, float y1,
								float xl, float xr,
								float ul, float ur,
								float vl, float vr,
								float wl, float wr,
								float const bl[4], float const br[4],
								float const offl[4], float const offr[4],
								float dxldy, float dxrdy,
								float duldy, float durdy,
								float dvldy, float dvrdy,
								float dwldy, float dwrdy,
								float const dbldy[4], float const dbrdy[4],
								float const doldy[4], float const dordy[4]);

	template <pix_sample_fn sample_fn, int group_no>
		inline void render_tri_sorted(bitmap_rgb32 &bitmap, texinfo *ti,
										const vert *v0,
										const vert *v1, const vert *v2);

	template <int group_no>
		void render_tri(bitmap_rgb32 &bitmap, texinfo *ti, const vert *v);

	template <int group_no>
		void render_group_to_accumulation_buffer(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void sort_vertices(const vert *v, int *i0, int *i1, int *i2);
	void render_to_accumulation_buffer(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void pvr_accumulationbuffer_to_framebuffer(address_space &space, int x, int y);
	void pvr_drawframebuffer(bitmap_rgb32 &bitmap,const rectangle &cliprect);
	static uint32_t dilate0(uint32_t value,int bits);
	static uint32_t dilate1(uint32_t value,int bits);
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

DECLARE_DEVICE_TYPE(POWERVR2, powervr2_device)

#endif // MAME_SEGA_POWERVR2_H
