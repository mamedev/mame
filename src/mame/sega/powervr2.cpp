// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*
  Dreamcast video emulation

  TODO:
  - Needs break-down into sub-devices, as a generic rule of thumb
    TA is Holly/CLX implementation specific while CORE is ISP/TSP only.
    This will be particularly useful for PVR PMX implementations
    (atvtrack.cpp and aristmk6.cpp);
  - Remove dc_state readbacks from here;
  - Better abstraction of timers;

*/

#include "emu.h"
#include "powervr2.h"
#include "dc.h"

#include "cpu/sh/sh4.h"
#include "video/rgbutil.h"
#include "rendutil.h"

#define LOG_WARN        (1U << 1) // Show warnings
#define LOG_TA_CMD      (1U << 2) // Show TA CORE commands
#define LOG_TA_FIFO     (1U << 3) // Show TA FIFO polygon entries
#define LOG_TA_TILE     (1U << 4) // Show TA tile entries
#define LOG_PVR_DMA     (1U << 5) // Show PVR-DMA access
#define LOG_IRQ         (1U << 6) // Show irq triggers

#define VERBOSE (LOG_WARN | LOG_PVR_DMA)

#include "logmacro.h"

#define LOGWARN(...)       LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGTACMD(...)      LOGMASKED(LOG_TA_CMD, __VA_ARGS__)
#define LOGTAFIFO(...)     LOGMASKED(LOG_TA_FIFO, __VA_ARGS__)
#define LOGTATILE(...)     LOGMASKED(LOG_TA_TILE, __VA_ARGS__)
#define LOGPVRDMA(...)     LOGMASKED(LOG_PVR_DMA, __VA_ARGS__)
#define LOGIRQ(...)        LOGMASKED(LOG_IRQ, __VA_ARGS__)

#define LIVE_YUV_VIEW 0


DEFINE_DEVICE_TYPE(POWERVR2, powervr2_device, "powervr2", "PowerVR 2")

void powervr2_device::ta_map(address_map &map)
{
	map(0x0000, 0x0003).r(FUNC(powervr2_device::id_r));
	map(0x0004, 0x0007).r(FUNC(powervr2_device::revision_r));
	map(0x0008, 0x000b).rw(FUNC(powervr2_device::softreset_r), FUNC(powervr2_device::softreset_w));
	map(0x0014, 0x0017).w(FUNC(powervr2_device::startrender_w));
// 18 = test select
	map(0x0020, 0x0023).rw(FUNC(powervr2_device::param_base_r), FUNC(powervr2_device::param_base_w));
	map(0x002c, 0x002f).rw(FUNC(powervr2_device::region_base_r), FUNC(powervr2_device::region_base_w));
// 30 = span sort cfg
	map(0x0040, 0x0043).rw(FUNC(powervr2_device::vo_border_col_r), FUNC(powervr2_device::vo_border_col_w));
	map(0x0044, 0x0047).rw(FUNC(powervr2_device::fb_r_ctrl_r), FUNC(powervr2_device::fb_r_ctrl_w));
	map(0x0048, 0x004b).rw(FUNC(powervr2_device::fb_w_ctrl_r), FUNC(powervr2_device::fb_w_ctrl_w));
	map(0x004c, 0x004f).rw(FUNC(powervr2_device::fb_w_linestride_r), FUNC(powervr2_device::fb_w_linestride_w));
	map(0x0050, 0x0053).rw(FUNC(powervr2_device::fb_r_sof1_r), FUNC(powervr2_device::fb_r_sof1_w));
	map(0x0054, 0x0057).rw(FUNC(powervr2_device::fb_r_sof2_r), FUNC(powervr2_device::fb_r_sof2_w));
	map(0x005c, 0x005f).rw(FUNC(powervr2_device::fb_r_size_r), FUNC(powervr2_device::fb_r_size_w));
	map(0x0060, 0x0063).rw(FUNC(powervr2_device::fb_w_sof1_r), FUNC(powervr2_device::fb_w_sof1_w));
	map(0x0064, 0x0067).rw(FUNC(powervr2_device::fb_w_sof2_r), FUNC(powervr2_device::fb_w_sof2_w));
	map(0x0068, 0x006b).rw(FUNC(powervr2_device::fb_x_clip_r), FUNC(powervr2_device::fb_x_clip_w));
	map(0x006c, 0x006f).rw(FUNC(powervr2_device::fb_y_clip_r), FUNC(powervr2_device::fb_y_clip_w));
// 74 = fpu_shad_scale
// 78 = fpu_cull_val
	map(0x007c, 0x007f).rw(FUNC(powervr2_device::fpu_param_cfg_r), FUNC(powervr2_device::fpu_param_cfg_w));
// 80 = half_offset
// 84 = fpu_perp_val
// 88 = isp_backgnd_d
	map(0x008c, 0x008f).rw(FUNC(powervr2_device::isp_backgnd_t_r), FUNC(powervr2_device::isp_backgnd_t_w));
// 98 = isp_feed_cfg
// a0 = sdram_refresh
// a4 = sdram_arb_cfg
// a8 = sdram_cfg
// b0 = fog_col_ram
// b4 = fog_col_vert
// b8 = fog_density
// bc = fog_clamp_max
// c0 = fog_clamp_min
// c4 = spg_trigger_pos
	map(0x00c8, 0x00cb).rw(FUNC(powervr2_device::spg_hblank_int_r), FUNC(powervr2_device::spg_hblank_int_w));
	map(0x00cc, 0x00cf).rw(FUNC(powervr2_device::spg_vblank_int_r), FUNC(powervr2_device::spg_vblank_int_w));
	map(0x00d0, 0x00d3).rw(FUNC(powervr2_device::spg_control_r), FUNC(powervr2_device::spg_control_w));
	map(0x00d4, 0x00d7).rw(FUNC(powervr2_device::spg_hblank_r), FUNC(powervr2_device::spg_hblank_w));
	map(0x00d8, 0x00db).rw(FUNC(powervr2_device::spg_load_r), FUNC(powervr2_device::spg_load_w));
	map(0x00dc, 0x00df).rw(FUNC(powervr2_device::spg_vblank_r), FUNC(powervr2_device::spg_vblank_w));
	map(0x00e0, 0x00e3).rw(FUNC(powervr2_device::spg_width_r), FUNC(powervr2_device::spg_width_w));
	map(0x00e4, 0x00e7).rw(FUNC(powervr2_device::text_control_r), FUNC(powervr2_device::text_control_w));
	map(0x00e8, 0x00eb).rw(FUNC(powervr2_device::vo_control_r), FUNC(powervr2_device::vo_control_w));
	map(0x00ec, 0x00ef).rw(FUNC(powervr2_device::vo_startx_r), FUNC(powervr2_device::vo_startx_w));
	map(0x00f0, 0x00f3).rw(FUNC(powervr2_device::vo_starty_r), FUNC(powervr2_device::vo_starty_w));
// f4 = scaler_ctl
	map(0x0108, 0x010b).rw(FUNC(powervr2_device::pal_ram_ctrl_r), FUNC(powervr2_device::pal_ram_ctrl_w));
	map(0x010c, 0x010f).r(FUNC(powervr2_device::spg_status_r));
// 110 = fb_burstctrl
// 118 = y_coeff
// 11c = pt_alpha_ref

	map(0x0124, 0x0127).rw(FUNC(powervr2_device::ta_ol_base_r), FUNC(powervr2_device::ta_ol_base_w));
	map(0x0128, 0x012b).rw(FUNC(powervr2_device::ta_isp_base_r), FUNC(powervr2_device::ta_isp_base_w));
	map(0x012c, 0x012f).rw(FUNC(powervr2_device::ta_ol_limit_r), FUNC(powervr2_device::ta_ol_limit_w));
	map(0x0130, 0x0133).rw(FUNC(powervr2_device::ta_isp_limit_r), FUNC(powervr2_device::ta_isp_limit_w));
	map(0x0134, 0x0137).r(FUNC(powervr2_device::ta_next_opb_r));
	map(0x0138, 0x013b).r(FUNC(powervr2_device::ta_itp_current_r));
// 13c = ta_glob_tile_clip
	map(0x0140, 0x0143).rw(FUNC(powervr2_device::ta_alloc_ctrl_r), FUNC(powervr2_device::ta_alloc_ctrl_w));
	map(0x0144, 0x0147).rw(FUNC(powervr2_device::ta_list_init_r), FUNC(powervr2_device::ta_list_init_w));
	map(0x0148, 0x014b).rw(FUNC(powervr2_device::ta_yuv_tex_base_r), FUNC(powervr2_device::ta_yuv_tex_base_w));
	map(0x014c, 0x014f).rw(FUNC(powervr2_device::ta_yuv_tex_ctrl_r), FUNC(powervr2_device::ta_yuv_tex_ctrl_w));
	map(0x0150, 0x0153).r(FUNC(powervr2_device::ta_yuv_tex_cnt_r));
	map(0x0160, 0x0163).w(FUNC(powervr2_device::ta_list_cont_w));
	map(0x0164, 0x0167).rw(FUNC(powervr2_device::ta_next_opb_init_r), FUNC(powervr2_device::ta_next_opb_init_w));

	map(0x0200, 0x03ff).rw(FUNC(powervr2_device::fog_table_r), FUNC(powervr2_device::fog_table_w));
	map(0x1000, 0x1fff).rw(FUNC(powervr2_device::palette_r), FUNC(powervr2_device::palette_w));
}

void powervr2_device::pd_dma_map(address_map &map)
{
	map(0x00, 0x03).rw(FUNC(powervr2_device::sb_pdstap_r), FUNC(powervr2_device::sb_pdstap_w));
	map(0x04, 0x07).rw(FUNC(powervr2_device::sb_pdstar_r), FUNC(powervr2_device::sb_pdstar_w));
	map(0x08, 0x0b).rw(FUNC(powervr2_device::sb_pdlen_r), FUNC(powervr2_device::sb_pdlen_w));
	map(0x0c, 0x0f).rw(FUNC(powervr2_device::sb_pddir_r), FUNC(powervr2_device::sb_pddir_w));
	map(0x10, 0x13).rw(FUNC(powervr2_device::sb_pdtsel_r), FUNC(powervr2_device::sb_pdtsel_w));
	map(0x14, 0x17).rw(FUNC(powervr2_device::sb_pden_r), FUNC(powervr2_device::sb_pden_w));
	map(0x18, 0x1b).rw(FUNC(powervr2_device::sb_pdst_r), FUNC(powervr2_device::sb_pdst_w));
	map(0x80, 0x83).rw(FUNC(powervr2_device::sb_pdapro_r), FUNC(powervr2_device::sb_pdapro_w));
}

const int powervr2_device::pvr_parconfseq[] = {1,2,3,2,3,4,5,6,5,6,7,8,9,10,11,12,13,14,13,14,15,16,17,16,17,0,0,0,0,0,18,19,20,19,20,21,22,23,22,23};
const int powervr2_device::pvr_wordsvertex[24]  = {8,8,8,8,8,16,16,8,8,8, 8, 8,8,8,8,8,16,16, 8,16,16,8,16,16};
const int powervr2_device::pvr_wordspolygon[24] = {8,8,8,8,8, 8, 8,8,8,8,16,16,8,8,8,8, 8, 8,16,16,16,8, 8, 8};

inline int32_t powervr2_device::clamp(int32_t in, int32_t min, int32_t max)
{
	if(in < min) return min;
	if(in > max) return max;
	return in;
}

// Perform a standard bilinear filter across four pixels
inline uint32_t powervr2_device::bilinear_filter(uint32_t c0, uint32_t c1, uint32_t c2, uint32_t c3, float u, float v)
{
	const uint32_t ui = (u * 256.0f);
	const uint32_t vi = (v * 256.0f);
	return rgbaint_t::bilinear_filter(c0, c1, c3, c2, ui, vi);
}

// Multiply with alpha value in bits 31-24
inline uint32_t powervr2_device::bla(uint32_t c, uint32_t a)
{
	a = a >> 24;
	return ((((c & 0xff00ff)*a) & 0xff00ff00) >> 8) | ((((c >> 8) & 0xff00ff)*a) & 0xff00ff00);
}

// Multiply with 1-alpha value in bits 31-24
inline uint32_t powervr2_device::blia(uint32_t c, uint32_t a)
{
	a = 0x100 - (a >> 24);
	return ((((c & 0xff00ff)*a) & 0xff00ff00) >> 8) | ((((c >> 8) & 0xff00ff)*a) & 0xff00ff00);
}

// Per-component multiply with color value
inline uint32_t powervr2_device::blc(uint32_t c1, uint32_t c2)
{
	uint32_t cr =
		(((c1 & 0x000000ff)*(c2 & 0x000000ff) & 0x0000ff00) >> 8)  |
		(((c1 & 0x0000ff00)*(c2 & 0x0000ff00) & 0xff000000) >> 16);
	c1 >>= 16;
	c2 >>= 16;
	cr |=
		(((c1 & 0x000000ff)*(c2 & 0x000000ff) & 0x0000ff00) << 8)  |
		(((c1 & 0x0000ff00)*(c2 & 0x0000ff00) & 0xff000000));
	return cr;
}

// Per-component multiply with 1-color value
inline uint32_t powervr2_device::blic(uint32_t c1, uint32_t c2)
{
	uint32_t cr =
		(((c1 & 0x000000ff)*(0x00100-(c2 & 0x000000ff)) & 0x0000ff00) >> 8)  |
		(((c1 & 0x0000ff00)*(0x10000-(c2 & 0x0000ff00)) & 0xff000000) >> 16);
	c1 >>= 16;
	c2 >>= 16;
	cr |=
		(((c1 & 0x000000ff)*(0x00100-(c2 & 0x000000ff)) & 0x0000ff00) << 8)  |
		(((c1 & 0x0000ff00)*(0x10000-(c2 & 0x0000ff00)) & 0xff000000));
	return cr;
}

// Add two colors with saturation
inline uint32_t powervr2_device::bls(uint32_t c1, uint32_t c2)
{
	uint32_t cr1, cr2;
	cr1 = (c1 & 0x00ff00ff) + (c2 & 0x00ff00ff);
	if(cr1 & 0x0000ff00)
		cr1 = (cr1 & 0xffff00ff) | 0x000000ff;
	if(cr1 & 0xff000000)
		cr1 = (cr1 & 0x00ffffff) | 0x00ff0000;

	cr2 = ((c1 >> 8) & 0x00ff00ff) + ((c2 >> 8) & 0x00ff00ff);
	if(cr2 & 0x0000ff00)
		cr2 = (cr2 & 0xffff00ff) | 0x000000ff;
	if(cr2 & 0xff000000)
		cr2 = (cr2 & 0x00ffffff) | 0x00ff0000;
	return cr1|(cr2 << 8);
}

/*
 * Add two colors with saturation, not including the alpha channel
 * The only difference between this function and bls is that bls does not
 * ignore alpha.  The alpha will be cleared to zero by this instruction
 */
inline uint32_t powervr2_device::bls24(uint32_t c1, uint32_t c2)
{
	uint32_t cr1, cr2;
	cr1 = (c1 & 0x00ff00ff) + (c2 & 0x00ff00ff);
	if(cr1 & 0x0000ff00)
		cr1 = (cr1 & 0xffff00ff) | 0x000000ff;
	if(cr1 & 0xff000000)
		cr1 = (cr1 & 0x00ffffff) | 0x00ff0000;

	cr2 = ((c1 >> 8) & 0x000000ff) + ((c2 >> 8) & 0x000000ff);
	if(cr2 & 0x0000ff00)
		cr2 = (cr2 & 0xffff00ff) | 0x000000ff;
	return cr1|(cr2 << 8);
}

inline uint32_t powervr2_device::float_argb_to_packed_argb(float argb[4]) {
	int argb_int[4] = {
		(int)(argb[0] * 256.0f),
		(int)(argb[1] * 256.0f),
		(int)(argb[2] * 256.0f),
		(int)(argb[3] * 256.0f)
	};

	// clamp to [0, 255]
	int idx;
	for (idx = 0; idx < 4; idx++) {
		if (argb_int[idx] < 0)
			argb_int[idx] = 0;
		else if (argb_int[idx] > 255)
			argb_int[idx] = 255;
	}

	return (argb_int[0] << 24) | (argb_int[1] << 16) |
		(argb_int[2] << 8) | argb_int[3];
}

inline void powervr2_device::packed_argb_to_float_argb(float dst[4], uint32_t in) {
	dst[0] = (in >> 24) / 256.0f;
	dst[1] = ((in >> 16) & 0xff) / 256.0f;
	dst[2] = ((in >> 8) & 0xff) / 256.0f;
	dst[3] = (in & 0xff) / 256.0f;
}

// All 64 blending modes, 3 top bits are source mode, 3 bottom bits are destination mode
uint32_t powervr2_device::bl00(uint32_t s, uint32_t d) { return 0; }
uint32_t powervr2_device::bl01(uint32_t s, uint32_t d) { return d; }
uint32_t powervr2_device::bl02(uint32_t s, uint32_t d) { return blc(d, s); }
uint32_t powervr2_device::bl03(uint32_t s, uint32_t d) { return blic(d, s); }
uint32_t powervr2_device::bl04(uint32_t s, uint32_t d) { return bla(d, s); }
uint32_t powervr2_device::bl05(uint32_t s, uint32_t d) { return blia(d, s); }
uint32_t powervr2_device::bl06(uint32_t s, uint32_t d) { return bla(d, d); }
uint32_t powervr2_device::bl07(uint32_t s, uint32_t d) { return blia(d, d); }
uint32_t powervr2_device::bl10(uint32_t s, uint32_t d) { return s; }
uint32_t powervr2_device::bl11(uint32_t s, uint32_t d) { return bls(s, d); }
uint32_t powervr2_device::bl12(uint32_t s, uint32_t d) { return bls(s, blc(s, d)); }
uint32_t powervr2_device::bl13(uint32_t s, uint32_t d) { return bls(s, blic(s, d)); }
uint32_t powervr2_device::bl14(uint32_t s, uint32_t d) { return bls(s, bla(d, s)); }
uint32_t powervr2_device::bl15(uint32_t s, uint32_t d) { return bls(s, blia(d, s)); }
uint32_t powervr2_device::bl16(uint32_t s, uint32_t d) { return bls(s, bla(d, d)); }
uint32_t powervr2_device::bl17(uint32_t s, uint32_t d) { return bls(s, blia(d, d)); }
uint32_t powervr2_device::bl20(uint32_t s, uint32_t d) { return blc(d, s); }
uint32_t powervr2_device::bl21(uint32_t s, uint32_t d) { return bls(blc(d, s), d); }
uint32_t powervr2_device::bl22(uint32_t s, uint32_t d) { return bls(blc(d, s), blc(s, d)); }
uint32_t powervr2_device::bl23(uint32_t s, uint32_t d) { return bls(blc(d, s), blic(s, d)); }
uint32_t powervr2_device::bl24(uint32_t s, uint32_t d) { return bls(blc(d, s), bla(d, s)); }
uint32_t powervr2_device::bl25(uint32_t s, uint32_t d) { return bls(blc(d, s), blia(d, s)); }
uint32_t powervr2_device::bl26(uint32_t s, uint32_t d) { return bls(blc(d, s), bla(d, d)); }
uint32_t powervr2_device::bl27(uint32_t s, uint32_t d) { return bls(blc(d, s), blia(d, d)); }
uint32_t powervr2_device::bl30(uint32_t s, uint32_t d) { return blic(d, s); }
uint32_t powervr2_device::bl31(uint32_t s, uint32_t d) { return bls(blic(d, s), d); }
uint32_t powervr2_device::bl32(uint32_t s, uint32_t d) { return bls(blic(d, s), blc(s, d)); }
uint32_t powervr2_device::bl33(uint32_t s, uint32_t d) { return bls(blic(d, s), blic(s, d)); }
uint32_t powervr2_device::bl34(uint32_t s, uint32_t d) { return bls(blic(d, s), bla(d, s)); }
uint32_t powervr2_device::bl35(uint32_t s, uint32_t d) { return bls(blic(d, s), blia(d, s)); }
uint32_t powervr2_device::bl36(uint32_t s, uint32_t d) { return bls(blic(d, s), bla(d, d)); }
uint32_t powervr2_device::bl37(uint32_t s, uint32_t d) { return bls(blic(d, s), blia(d, d)); }
uint32_t powervr2_device::bl40(uint32_t s, uint32_t d) { return bla(s, s); }
uint32_t powervr2_device::bl41(uint32_t s, uint32_t d) { return bls(bla(s, s), d); }
uint32_t powervr2_device::bl42(uint32_t s, uint32_t d) { return bls(bla(s, s), blc(s, d)); }
uint32_t powervr2_device::bl43(uint32_t s, uint32_t d) { return bls(bla(s, s), blic(s, d)); }
uint32_t powervr2_device::bl44(uint32_t s, uint32_t d) { return bls(bla(s, s), bla(d, s)); }
uint32_t powervr2_device::bl45(uint32_t s, uint32_t d) { return bls(bla(s, s), blia(d, s)); }
uint32_t powervr2_device::bl46(uint32_t s, uint32_t d) { return bls(bla(s, s), bla(d, d)); }
uint32_t powervr2_device::bl47(uint32_t s, uint32_t d) { return bls(bla(s, s), blia(d, d)); }
uint32_t powervr2_device::bl50(uint32_t s, uint32_t d) { return blia(s, s); }
uint32_t powervr2_device::bl51(uint32_t s, uint32_t d) { return bls(blia(s, s), d); }
uint32_t powervr2_device::bl52(uint32_t s, uint32_t d) { return bls(blia(s, s), blc(s, d)); }
uint32_t powervr2_device::bl53(uint32_t s, uint32_t d) { return bls(blia(s, s), blic(s, d)); }
uint32_t powervr2_device::bl54(uint32_t s, uint32_t d) { return bls(blia(s, s), bla(d, s)); }
uint32_t powervr2_device::bl55(uint32_t s, uint32_t d) { return bls(blia(s, s), blia(d, s)); }
uint32_t powervr2_device::bl56(uint32_t s, uint32_t d) { return bls(blia(s, s), bla(d, d)); }
uint32_t powervr2_device::bl57(uint32_t s, uint32_t d) { return bls(blia(s, s), blia(d, d)); }
uint32_t powervr2_device::bl60(uint32_t s, uint32_t d) { return bla(s, d); }
uint32_t powervr2_device::bl61(uint32_t s, uint32_t d) { return bls(bla(s, d), d); }
uint32_t powervr2_device::bl62(uint32_t s, uint32_t d) { return bls(bla(s, d), blc(s, d)); }
uint32_t powervr2_device::bl63(uint32_t s, uint32_t d) { return bls(bla(s, d), blic(s, d)); }
uint32_t powervr2_device::bl64(uint32_t s, uint32_t d) { return bls(bla(s, d), bla(d, s)); }
uint32_t powervr2_device::bl65(uint32_t s, uint32_t d) { return bls(bla(s, d), blia(d, s)); }
uint32_t powervr2_device::bl66(uint32_t s, uint32_t d) { return bls(bla(s, d), bla(d, d)); }
uint32_t powervr2_device::bl67(uint32_t s, uint32_t d) { return bls(bla(s, d), blia(d, d)); }
uint32_t powervr2_device::bl70(uint32_t s, uint32_t d) { return blia(s, d); }
uint32_t powervr2_device::bl71(uint32_t s, uint32_t d) { return bls(blia(s, d), d); }
uint32_t powervr2_device::bl72(uint32_t s, uint32_t d) { return bls(blia(s, d), blc(s, d)); }
uint32_t powervr2_device::bl73(uint32_t s, uint32_t d) { return bls(blia(s, d), blic(s, d)); }
uint32_t powervr2_device::bl74(uint32_t s, uint32_t d) { return bls(blia(s, d), bla(d, s)); }
uint32_t powervr2_device::bl75(uint32_t s, uint32_t d) { return bls(blia(s, d), blia(d, s)); }
uint32_t powervr2_device::bl76(uint32_t s, uint32_t d) { return bls(blia(s, d), bla(d, d)); }
uint32_t powervr2_device::bl77(uint32_t s, uint32_t d) { return bls(blia(s, d), blia(d, d)); }

uint32_t (*const powervr2_device::blend_functions[64])(uint32_t s, uint32_t d) = {
	bl00, bl01, bl02, bl03, bl04, bl05, bl06, bl07,
	bl10, bl11, bl12, bl13, bl14, bl15, bl16, bl17,
	bl20, bl21, bl22, bl23, bl24, bl25, bl26, bl27,
	bl30, bl31, bl32, bl33, bl34, bl35, bl36, bl37,
	bl40, bl41, bl42, bl43, bl44, bl45, bl46, bl47,
	bl50, bl51, bl52, bl53, bl54, bl55, bl56, bl57,
	bl60, bl61, bl62, bl63, bl64, bl65, bl66, bl67,
	bl70, bl71, bl72, bl73, bl74, bl75, bl76, bl77,
};

inline uint32_t powervr2_device::cv_1555(uint16_t c)
{
	return
		(c & 0x8000 ? 0xff000000 : 0) |
		((c << 9) & 0x00f80000) | ((c << 4) & 0x00070000) |
		((c << 6) & 0x0000f800) | ((c << 1) & 0x00000700) |
		((c << 3) & 0x000000f8) | ((c >> 2) & 0x00000007);
}

inline uint32_t powervr2_device::cv_1555z(uint16_t c)
{
	return
		(c & 0x8000 ? 0xff000000 : 0) |
		((c << 9) & 0x00f80000) |
		((c << 6) & 0x0000f800) |
		((c << 3) & 0x000000f8);
}

inline uint32_t powervr2_device::cv_565(uint16_t c)
{
	return
		0xff000000 |
		((c << 8) & 0x00f80000) | ((c << 3) & 0x00070000) |
		((c << 5) & 0x0000fc00) | ((c >> 1) & 0x00000300) |
		((c << 3) & 0x000000f8) | ((c >> 2) & 0x00000007);
}

inline uint32_t powervr2_device::cv_565z(uint16_t c)
{
	return
		0xff000000 |
		((c << 8) & 0x00f80000) |
		((c << 5) & 0x0000fc00) |
		((c << 3) & 0x000000f8);
}

inline uint32_t powervr2_device::cv_4444(uint16_t c)
{
	return
		((c << 16) & 0xf0000000) | ((c << 12) & 0x0f000000) |
		((c << 12) & 0x00f00000) | ((c <<  8) & 0x000f0000) |
		((c <<  8) & 0x0000f000) | ((c <<  4) & 0x00000f00) |
		((c <<  4) & 0x000000f0) | ((c      ) & 0x0000000f);
}

inline uint32_t powervr2_device::cv_4444z(uint16_t c)
{
	return
		((c << 16) & 0xf0000000) |
		((c << 12) & 0x00f00000) |
		((c <<  8) & 0x0000f000) |
		((c <<  4) & 0x000000f0);
}

inline uint32_t powervr2_device::cv_yuv(uint16_t c1, uint16_t c2, int x)
{
	int u = 11*((c1 & 0xff) - 128);
	int v = 11*((c2 & 0xff) - 128);
	int y = (x & 1 ? c2 : c1) >> 8;
	int r = y + v/8;
	int g = y - u/32 - v/16;
	int b = y + (3*u)/16;
	r = r < 0 ? 0 : r > 255 ? 255 : r;
	g = g < 0 ? 0 : g > 255 ? 255 : g;
	b = b < 0 ? 0 : b > 255 ? 255 : b;
	return 0xff000000 | (r << 16) | (g << 8) | b;
}

int powervr2_device::uv_wrap(float uv, int size)
{
	int iuv = (int)uv;
	return iuv & (size - 1);
}
int powervr2_device::uv_flip(float uv, int size)
{
	int iuv = (int)uv;
	return ((iuv & size) ? ~iuv : iuv) & (size - 1);
}
int powervr2_device::uv_clamp(float uv, int size)
{
	int iuv = (int)uv;
	return (iuv > 0) ? (iuv < size) ? iuv : (size - 1) : 0;
}

uint32_t powervr2_device::tex_r_yuv_n(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int addrp = t->address + (t->stride*yt + (xt & ~1))*2;
	uint16_t c1 = *(uint16_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + WORD_XOR_LE(addrp));
	uint16_t c2 = *(uint16_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + WORD_XOR_LE(addrp+2));
	return cv_yuv(c1, c2, xt);
}

uint32_t powervr2_device::tex_r_yuv_tw(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int addrp = t->address + (dilated1[t->cd][xt & ~1] + dilated0[t->cd][yt]) * 2;
	uint16_t c1 = *(uint16_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + WORD_XOR_LE(addrp));
	uint16_t c2 = *(uint16_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + WORD_XOR_LE(addrp+4));
	return cv_yuv(c1, c2, xt);
}

#if 0
uint32_t powervr2_device::tex_r_yuv_vq(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int idx = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + (dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 1])*2;
	uint16_t c1 = *(uint16_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + WORD_XOR_LE(addrp));
	uint16_t c2 = *(uint16_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + WORD_XOR_LE(addrp+4));
	return cv_yuv(c1, c2, xt);
}
#endif

uint32_t powervr2_device::tex_r_1555_n(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int addrp = t->address + (t->stride*yt + xt) * 2;
	return cv_1555z(*(uint16_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

uint32_t powervr2_device::tex_r_1555_tw(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int addrp = t->address + (dilated1[t->cd][xt] + dilated0[t->cd][yt]) * 2;
	return cv_1555(*(uint16_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

uint32_t powervr2_device::tex_r_1555_vq(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int idx = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + (dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 1])*2;
	return cv_1555(*(uint16_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

uint32_t powervr2_device::tex_r_565_n(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int addrp = t->address + (t->stride*yt + xt) * 2;
	return cv_565z(*(uint16_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

uint32_t powervr2_device::tex_r_565_tw(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int addrp = t->address + (dilated1[t->cd][xt] + dilated0[t->cd][yt]) * 2;
	return cv_565(*(uint16_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

uint32_t powervr2_device::tex_r_565_vq(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int idx = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + (dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 1])*2;
	return cv_565(*(uint16_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

uint32_t powervr2_device::tex_r_4444_n(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int addrp = t->address + (t->stride*yt + xt) * 2;
	return cv_4444z(*(uint16_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

uint32_t powervr2_device::tex_r_4444_tw(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int addrp = t->address + (dilated1[t->cd][xt] + dilated0[t->cd][yt]) * 2;
	return cv_4444(*(uint16_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

uint32_t powervr2_device::tex_r_4444_vq(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int idx = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + (dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 1])*2;
	return cv_4444(*(uint16_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

uint32_t powervr2_device::tex_r_p4_1555_tw(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int off = dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int addrp = t->address + (off >> 1);
	int c = ((reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] >> ((off & 1) << 2)) & 0xf;
	return cv_1555(palette[t->palbase + c]);
}

uint32_t powervr2_device::tex_r_p4_1555_vq(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int idx = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] & 0xf;
	return cv_1555(palette[t->palbase + c]);
}

uint32_t powervr2_device::tex_r_p4_565_tw(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int off = dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int addrp = t->address + (off >> 1);
	int c = ((reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] >> ((off & 1) << 2)) & 0xf;
	return cv_565(palette[t->palbase + c]);
}

uint32_t powervr2_device::tex_r_p4_565_vq(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int idx = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] & 0xf;
	return cv_565(palette[t->palbase + c]);
}

uint32_t powervr2_device::tex_r_p4_4444_tw(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int off = dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int addrp = t->address + (off >> 1);
	int c = ((reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] >> ((off & 1) << 2)) & 0xf;
	return cv_4444(palette[t->palbase + c]);
}

uint32_t powervr2_device::tex_r_p4_4444_vq(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int idx = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] & 0xf;
	return cv_4444(palette[t->palbase + c]);
}

uint32_t powervr2_device::tex_r_p4_8888_tw(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int off = dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int addrp = t->address + (off >> 1);
	int c = ((reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] >> ((off & 1) << 2)) & 0xf;
	return palette[t->palbase + c];
}

uint32_t powervr2_device::tex_r_p4_8888_vq(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int idx = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] & 0xf;
	return palette[t->palbase + c];
}

uint32_t powervr2_device::tex_r_p8_1555_tw(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int addrp = t->address + dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int c = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_1555(palette[t->palbase + c]);
}

uint32_t powervr2_device::tex_r_p8_1555_vq(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int idx = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_1555(palette[t->palbase + c]);
}

uint32_t powervr2_device::tex_r_p8_565_tw(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int addrp = t->address + dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int c = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_565(palette[t->palbase + c]);
}

uint32_t powervr2_device::tex_r_p8_565_vq(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int idx = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_565(palette[t->palbase + c]);
}

uint32_t powervr2_device::tex_r_p8_4444_tw(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int addrp = t->address + dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int c = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_4444(palette[t->palbase + c]);
}

uint32_t powervr2_device::tex_r_p8_4444_vq(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int idx = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_4444(palette[t->palbase + c]);
}

uint32_t powervr2_device::tex_r_p8_8888_tw(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int addrp = t->address + dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int c = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return palette[t->palbase + c];
}

uint32_t powervr2_device::tex_r_p8_8888_vq(texinfo *t, float x, float y)
{
	int xt = t->u_func(x, t->sizex);
	int yt = t->v_func(y, t->sizey);
	int idx = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<uint8_t *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return palette[t->palbase + c];
}


uint32_t powervr2_device::tex_r_default(texinfo *t, float x, float y)
{
	return ((int)x ^ (int)y) & 4 ? 0xffffff00 : 0xff0000ff;
}

void powervr2_device::tex_get_info(texinfo *t)
{
	int miptype = 0;

	t->textured    = texture;
	t->address     = textureaddress;
	t->pf          = pixelformat;
	t->palette     = 0;
	t->mode        = (vqcompressed<<1);

	// scanorder is ignored for palettized textures (palettized textures are ALWAYS twiddled)
	// (the same bits are used for palette select instead)
	if ((t->pf == 5) || (t->pf == 6))
	{
		t->palette = paletteselector;
	}
	else
	{
		t->mode |= scanorder;
	}

	/* When scan order is 1 (non-twiddled) mipmap is ignored */
	t->mipmapped = t->mode & 1 ? 0 : mipmapped;

	// Mipmapped textures are always square, ignore v size
	if (t->mipmapped)
	{
		t->sizes = (texturesizes & 0x38) | ((texturesizes & 0x38) >> 3);
	}
	else
	{
		t->sizes = texturesizes;
	}

	t->sizex = 1 << (3+((t->sizes >> 3) & 7));
	t->sizey = 1 << (3+(t->sizes & 7));


	/* Stride select is used only in the non-twiddled case */
	t->stride = (t->mode & 1) && strideselect ? (text_control & 0x1f) << 5 : t->sizex;

	t->blend_mode  = blend_mode;
	t->filter_mode = filtermode;

	t->u_func = (clampuv & 2) ? &powervr2_device::uv_clamp : ((flipuv & 2) ? &powervr2_device::uv_flip : &powervr2_device::uv_wrap);
	t->v_func = (clampuv & 1) ? &powervr2_device::uv_clamp : ((flipuv & 1) ? &powervr2_device::uv_flip : &powervr2_device::uv_wrap);

	t->r = &powervr2_device::tex_r_default;
	t->cd = dilatechose[t->sizes];
	t->palbase = 0;
	t->vqbase = t->address;
	t->blend = ignoretexalpha ? bl10 : blend_functions[t->blend_mode];

	t->coltype = coltype;
	t->tsinstruction = tsinstruction;

//  fprintf(stderr, "tex %d %d %d %d\n", t->pf, t->mode, pal_ram_ctrl, t->mipmapped);
	if(!t->textured)
	{
		t->r = nullptr;
	}
	else
	{
	switch(t->pf) {
	case 0: // 1555
		switch(t->mode) {
		case 0:  t->r = &powervr2_device::tex_r_1555_tw; miptype = 2; break;
		case 1:  t->r = &powervr2_device::tex_r_1555_n;  miptype = 2; break;
		case 2:
		case 3:  t->r = &powervr2_device::tex_r_1555_vq; miptype = 3; t->address += 0x800; break;

		default:
			//
			break;
		}
		break;

	case 1: // 565
		switch(t->mode) {
		case 0:  t->r = &powervr2_device::tex_r_565_tw; miptype = 2; break;
		case 1:  t->r = &powervr2_device::tex_r_565_n;  miptype = 2; break;
		case 2:
		case 3:  t->r = &powervr2_device::tex_r_565_vq; miptype = 3; t->address += 0x800; break;

		default:
			//
			break;
		}
		break;

	case 2: // 4444
		switch(t->mode) {
		case 0:  t->r = &powervr2_device::tex_r_4444_tw; miptype = 2; break;
		case 1:  t->r = &powervr2_device::tex_r_4444_n;  miptype = 2; break;
		case 2:
		case 3:  t->r = &powervr2_device::tex_r_4444_vq; miptype = 3; t->address += 0x800; break;

		default:
			//
			break;
		}
		break;

	case 3: // yuv422
		switch(t->mode) {
		case 0:  t->r = &powervr2_device::tex_r_yuv_tw; miptype = -1; break;
		case 1:  t->r = &powervr2_device::tex_r_yuv_n; miptype = -1; break;
		//default: t->r = &powervr2_device::tex_r_yuv_vq; miptype = -1; break;
		}
		break;

	case 4: // bumpmap
		break;

	case 5: // 4bpp palette
		t->palbase = (t->palette & 0x3f) << 4;
		switch(t->mode) {
		case 0: case 1:
			miptype = 0;

			switch(pal_ram_ctrl) {
			case 0: t->r = &powervr2_device::tex_r_p4_1555_tw; break;
			case 1: t->r = &powervr2_device::tex_r_p4_565_tw;  break;
			case 2: t->r = &powervr2_device::tex_r_p4_4444_tw; break;
			case 3: t->r = &powervr2_device::tex_r_p4_8888_tw; break;
			}
			break;
		case 2: case 3:
			miptype = 3; // ?
			switch(pal_ram_ctrl) {
			case 0: t->r = &powervr2_device::tex_r_p4_1555_vq; t->address += 0x800; break;
			case 1: t->r = &powervr2_device::tex_r_p4_565_vq;  t->address += 0x800; break;
			case 2: t->r = &powervr2_device::tex_r_p4_4444_vq; t->address += 0x800; break;
			case 3: t->r = &powervr2_device::tex_r_p4_8888_vq; t->address += 0x800; break;
			}
			break;

		default:
			//
			break;
		}
		break;

	case 6: // 8bpp palette
		t->palbase = (t->palette & 0x30) << 4;
		switch(t->mode) {
		case 0: case 1:
			miptype = 1;

			switch(pal_ram_ctrl) {
			case 0: t->r = &powervr2_device::tex_r_p8_1555_tw; break;
			case 1: t->r = &powervr2_device::tex_r_p8_565_tw; break;
			case 2: t->r = &powervr2_device::tex_r_p8_4444_tw; break;
			case 3: t->r = &powervr2_device::tex_r_p8_8888_tw; break;
			}
			break;
		case 2: case 3:
			miptype = 3; // ?
			switch(pal_ram_ctrl) {
			case 0: t->r = &powervr2_device::tex_r_p8_1555_vq; t->address += 0x800; break;
			case 1: t->r = &powervr2_device::tex_r_p8_565_vq;  t->address += 0x800; break;
			case 2: t->r = &powervr2_device::tex_r_p8_4444_vq; t->address += 0x800; break;
			case 3: t->r = &powervr2_device::tex_r_p8_8888_vq; t->address += 0x800; break;
			}
			break;

		default:
			//
			break;
		}
		break;

	case 9: // reserved
		break;
	}
	}

	if (t->mipmapped)
	{
		// full offset tables for reference,
		// we don't do mipmapping, so don't use anything < 8x8
		// first table is half-bytes

		// 4BPP palette textures
		// Texture size _4-bit_ offset value for starting address
		// 1x1          0x00003
		// 2x2          0x00004
		// 4x4          0x00008
		// 8x8          0x00018
		// 16x16        0x00058
		// 32x32        0x00158
		// 64x64        0x00558
		// 128x128      0x01558
		// 256x256      0x05558
		// 512x512      0x15558
		// 1024x1024    0x55558

		// 8BPP palette textures
		// Texture size Byte offset value for starting address
		// 1x1          0x00003
		// 2x2          0x00004
		// 4x4          0x00008
		// 8x8          0x00018
		// 16x16        0x00058
		// 32x32        0x00158
		// 64x64        0x00558
		// 128x128      0x01558
		// 256x256      0x05558
		// 512x512      0x15558
		// 1024x1024    0x55558

		// Non-palette textures
		// Texture size Byte offset value for starting address
		// 1x1          0x00006
		// 2x2          0x00008
		// 4x4          0x00010
		// 8x8          0x00030
		// 16x16        0x000B0
		// 32x32        0x002B0
		// 64x64        0x00AB0
		// 128x128      0x02AB0
		// 256x256      0x0AAB0
		// 512x512      0x2AAB0
		// 1024x1024    0xAAAB0

		// VQ textures
		// Texture size Byte offset value for starting address
		// 1x1          0x00000
		// 2x2          0x00001
		// 4x4          0x00002
		// 8x8          0x00006
		// 16x16        0x00016
		// 32x32        0x00056
		// 64x64        0x00156
		// 128x128      0x00556
		// 256x256      0x01556
		// 512x512      0x05556
		// 1024x1024    0x15556

		static const int mipmap_4_8_offset[8] = { 0x00018, 0x00058, 0x00158, 0x00558, 0x01558, 0x05558, 0x15558, 0x55558 };  // 4bpp (4bit offset) / 8bpp (8bit offset)
		static const int mipmap_np_offset[8] =  { 0x00030, 0x000B0, 0x002B0, 0x00AB0, 0x02AB0, 0x0AAB0, 0x2AAB0, 0xAAAB0 };  // nonpalette textures
		static const int mipmap_vq_offset[8] =  { 0x00006, 0x00016, 0x00056, 0x00156, 0x00556, 0x01556, 0x05556, 0x15556 };  // vq textures

		switch (miptype)
		{
			case 0: // 4bpp
				//printf("4bpp\n");
				t->address += mipmap_4_8_offset[(t->sizes)&7]>>1;
				break;

			case 1: // 8bpp
				//printf("8bpp\n");
				t->address += mipmap_4_8_offset[(t->sizes)&7];
				break;

			case 2: // nonpalette
				//printf("np\n");
				t->address += mipmap_np_offset[(t->sizes)&7];
				break;

			case 3: // vq
				//printf("vq\n");
				t->address += mipmap_vq_offset[(t->sizes)&7];
				break;
		}
	}

}

uint32_t powervr2_device::id_r()
{
	return 0x17fd11db;
}

uint32_t powervr2_device::revision_r()
{
	return 0x00000011;
}

uint32_t powervr2_device::softreset_r()
{
	return softreset;
}

void powervr2_device::softreset_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&softreset);
	if (softreset & 1) {
		LOGTACMD("TA soft reset\n");
		listtype_used=0;
	}
	if (softreset & 2) {
		LOGTACMD("Core Pipeline soft reset\n");

		if (start_render_received == 1) {
			for (int a=0;a < NUM_BUFFERS;a++)
				if (grab[a].busy == 1)
					grab[a].busy = 0;

			start_render_received = 0;
		}
	}
	if (softreset & 4) {
		LOGTACMD("sdram I/F soft reset\n");
	}
}

void powervr2_device::startrender_w(address_space& space, uint32_t data)
{
	if (m_render_request)
	{
		int result;
		do
		{
			result = osd_work_item_wait(m_render_request, 1000);
			//printf("waiting\n");

		} while (result == 0);
		osd_work_item_release(m_render_request);
	}

	m_render_request = osd_work_item_queue(m_work_queue, blit_request_callback, (void*)this, 0);

	dc_state *state = machine().driver_data<dc_state>();

	// hacky end of render delay for Capcom games, otherwise they works at ~1/10 speed
	int sanitycount = 1500;
    endofrender_timer_isp->adjust(state->m_maincpu->cycles_to_attotime(sanitycount*25 + 2000000));   // hacky end of render delay for Capcom games, otherwise they works at ~1/10 speed
}


void powervr2_device::startrender_real_w(address_space &space)
{
	//dc_state *state = machine().driver_data<dc_state>();
	auto profile = g_profiler.start(PROFILER_USER1);

	LOGTACMD("Start render, region=%08x, params=%08x\n", region_base, param_base);

	// select buffer to draw using param_base
	for (int a=0;a < NUM_BUFFERS;a++) {
		if ((grab[a].ispbase == param_base) && (grab[a].valid == 1) && (grab[a].busy == 0)) {
			grab[a].busy = 1;
			renderselect = a;
			start_render_received=1;


			grab[a].fbwsof1 = fb_w_sof1;
			grab[a].fbwsof2 = fb_w_sof2;

			rectangle clip(0, 640, 0, 480);

			// we've got a request to draw, so, draw to the accumulation buffer!
			// this should really be done for each tile!
			render_to_accumulation_buffer(*fake_accumulationbuffer_bitmap,clip);

			/* copy the tiles to the framebuffer (really the rendering should be in this loop too) */
			int sizera = fpu_param_cfg & 0x200000 ? 6 : 5;
			int offsetra=region_base;

			//printf("base is %08x\n", offsetra);

			// sanity
			int sanitycount = 0;
			for (;;) {
				uint32_t st[6];

				st[0]=space.read_dword((0x05000000+offsetra));
				st[1]=space.read_dword((0x05000004+offsetra)); // Opaque List Pointer
				st[2]=space.read_dword((0x05000008+offsetra)); // Opaque Modifier Volume List Pointer
				st[3]=space.read_dword((0x0500000c+offsetra)); // Translucent List Pointer
				st[4]=space.read_dword((0x05000010+offsetra)); // Translucent Modifier Volume List Pointer

				if (sizera == 6) {
					st[5] = space.read_dword((0x05000014+offsetra)); // Punch Through List Pointer
					offsetra+=0x18;
				} else  {
					st[5] = 0;
					offsetra+=0x14;
				}

				{
					int x = ((st[0]&0x000000fc)>>2)*32;
					int y = ((st[0]&0x00003f00)>>8)*32;
					//printf("tiledata %08x %d %d - %08x %08x %08x %08x %08x\n",st[0],x,y,st[1],st[2],st[3],st[4],st[5]);

					// should render to the accumulation buffer here using pointers we filled in when processing the data
					// sent to the TA.  HOWEVER, we don't process the TA data and create the real format object lists, so
					// instead just use these co-ordinates to copy data from our fake full-screnen accumnulation buffer into
					// the framebuffer

					pvr_accumulationbuffer_to_framebuffer(space, x,y);
				}

				if (st[0] & 0x80000000)
					break;

				sanitycount++;
				// prevent infinite loop if asked to process invalid data
				//if(sanitycount>2000)
				//  break;
			}

			/* Fire ISP irq after a set amount of time */
			// TODO: exact timing of this
			// hacky end of render delay for Capcom games, otherwise they works at ~1/10 speed.
			// 500k is definitely too slow for them: it fires the isp/tsp completion at
			// vbl-in and expect that it completes after some time that vbl-out kicks in,
			// in order to have enough time to execute logic stuff in the meantime.
//          const u64 isp_completion = sanitycount * 25 + 500000;
			//const u64 isp_completion = sanitycount * 25 + 2000000;
			//LOGIRQ("[%d] ISP end of render start %d in %d cycles\n",
			//	screen().frame_number(), screen().vpos(), isp_completion
			//);
			//endofrender_timer_isp->adjust(state->m_maincpu->cycles_to_attotime(isp_completion));
			break;
		}
	}
}

void *powervr2_device::blit_request_callback(void *param, int threadid)
{
	powervr2_device *object = reinterpret_cast<powervr2_device *>(param);

	dc_state *state = object->machine().driver_data<dc_state>();
	address_space &space = state->m_maincpu->space(AS_PROGRAM);

	object->startrender_real_w(space);
	return nullptr;
}

uint32_t powervr2_device::param_base_r()
{
	return param_base;
}

void powervr2_device::param_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&param_base);
}

uint32_t powervr2_device::region_base_r()
{
	return region_base;
}

void powervr2_device::region_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&region_base);
}

uint32_t powervr2_device::vo_border_col_r()
{
	return vo_border_col;
}

void powervr2_device::vo_border_col_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&vo_border_col);
}

uint32_t powervr2_device::fb_r_ctrl_r()
{
	return fb_r_ctrl;
}

void powervr2_device::fb_r_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&fb_r_ctrl);
}

uint32_t powervr2_device::fb_w_ctrl_r()
{
	return fb_w_ctrl;
}

void powervr2_device::fb_w_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&fb_w_ctrl);
}

uint32_t powervr2_device::fb_w_linestride_r()
{
	return fb_w_linestride;
}

void powervr2_device::fb_w_linestride_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&fb_w_linestride);
}

uint32_t powervr2_device::fb_r_sof1_r()
{
	return fb_r_sof1;
}

void powervr2_device::fb_r_sof1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&fb_r_sof1);
}

uint32_t powervr2_device::fb_r_sof2_r()
{
	return fb_r_sof2;
}

void powervr2_device::fb_r_sof2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&fb_r_sof2);
}

uint32_t powervr2_device::fb_r_size_r()
{
	return fb_r_size;
}

void powervr2_device::fb_r_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&fb_r_size);
}

uint32_t powervr2_device::fb_w_sof1_r()
{
	return fb_w_sof1;
}

void powervr2_device::fb_w_sof1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&fb_w_sof1);
}

uint32_t powervr2_device::fb_w_sof2_r()
{
	return fb_w_sof2;
}

void powervr2_device::fb_w_sof2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&fb_w_sof2);
}

uint32_t powervr2_device::fb_x_clip_r()
{
	return fb_x_clip;
}

void powervr2_device::fb_x_clip_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&fb_x_clip);
}

uint32_t powervr2_device::fb_y_clip_r()
{
	return fb_y_clip;
}

void powervr2_device::fb_y_clip_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&fb_y_clip);
}

uint32_t powervr2_device::fpu_param_cfg_r()
{
	return fpu_param_cfg;
}

void powervr2_device::fpu_param_cfg_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&fpu_param_cfg);
}

uint32_t powervr2_device::isp_backgnd_t_r()
{
	return isp_backgnd_t;
}

void powervr2_device::isp_backgnd_t_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&isp_backgnd_t);
}

uint32_t powervr2_device::spg_hblank_int_r()
{
	return spg_hblank_int;
}

void powervr2_device::spg_hblank_int_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&spg_hblank_int);
	// TODO: timer adjust
}

uint32_t powervr2_device::spg_vblank_int_r()
{
	return spg_vblank_int;
}

void powervr2_device::spg_vblank_int_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&spg_vblank_int);

	/* clear pending irqs and modify them with the updated ones */
//  vbin_timer->adjust(attotime::never);
//  vbout_timer->adjust(attotime::never);

//  vbin_timer->adjust(screen().time_until_pos(spg_vblank_int & 0x3ff));
//  vbout_timer->adjust(screen().time_until_pos((spg_vblank_int >> 16) & 0x3ff));
}

uint32_t powervr2_device::spg_control_r()
{
	return spg_control;
}

void powervr2_device::spg_control_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&spg_control);
	update_screen_format();

	if((spg_control & 0xc0) == 0xc0)
		popmessage("SPG undocumented pixel clock mode 11, contact MAME/MESSdev");

	if((spg_control & 0xd0) == 0x10)
		popmessage("SPG enabled VGA mode with interlace, contact MAME/MESSdev");
}

uint32_t powervr2_device::spg_hblank_r()
{
	return spg_hblank;
}

void powervr2_device::spg_hblank_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&spg_hblank);
	update_screen_format();
}

uint32_t powervr2_device::spg_load_r()
{
	return spg_load;
}

void powervr2_device::spg_load_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&spg_load);
	update_screen_format();
}

uint32_t powervr2_device::spg_vblank_r()
{
	return spg_vblank;
}

void powervr2_device::spg_vblank_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&spg_vblank);
	update_screen_format();
}

uint32_t powervr2_device::spg_width_r()
{
	return spg_width;
}

void powervr2_device::spg_width_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&spg_width);
	update_screen_format();
}

uint32_t powervr2_device::text_control_r()
{
	return text_control;
}

void powervr2_device::text_control_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&text_control);
}

uint32_t powervr2_device::vo_control_r()
{
	return vo_control;
}

void powervr2_device::vo_control_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&vo_control);
}

uint32_t powervr2_device::vo_startx_r()
{
	return vo_startx;
}

void powervr2_device::vo_startx_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&vo_startx);
	update_screen_format();
}

uint32_t powervr2_device::vo_starty_r()
{
	return vo_starty;
}

void powervr2_device::vo_starty_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&vo_starty);
	update_screen_format();
}

uint32_t powervr2_device::pal_ram_ctrl_r()
{
	return pal_ram_ctrl;
}

void powervr2_device::pal_ram_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&pal_ram_ctrl);
}

uint32_t powervr2_device::spg_status_r()
{
	uint32_t fieldnum = (screen().frame_number() & 1) ? 1 : 0;
	int32_t spg_hbstart = spg_hblank & 0x3ff;
	int32_t spg_hbend = (spg_hblank >> 16) & 0x3ff;
	int32_t spg_vbstart = spg_vblank & 0x3ff;
	int32_t spg_vbend = (spg_vblank >> 16) & 0x3ff;

	uint32_t vsync = ((screen().vpos() >= spg_vbstart) || (screen().vpos() < spg_vbend)) ? 0 : 1;
	uint32_t hsync = ((screen().hpos() >= spg_hbstart) || (screen().hpos() < spg_hbend)) ? 0 : 1;
	// FIXME: following is just a wild guess
	uint32_t blank = ((screen().vpos() >= spg_vbstart) || (screen().vpos() < spg_vbend) ||
					(screen().hpos() >= spg_hbstart) || (screen().hpos() < spg_hbend)) ? 0 : 1;
	if(vo_control & 4) { blank^=1; }
	if(vo_control & 2) { vsync^=1; }
	if(vo_control & 1) { hsync^=1; }

	return (vsync << 13) | (hsync << 12) | (blank << 11) | (fieldnum << 10) | (screen().vpos() & 0x3ff);
}


uint32_t powervr2_device::ta_ol_base_r()
{
	return ta_ol_base;
}

void powervr2_device::ta_ol_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&ta_ol_base);
}

uint32_t powervr2_device::ta_isp_base_r()
{
	return ta_isp_base;
}

void powervr2_device::ta_isp_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&ta_isp_base);
}

uint32_t powervr2_device::ta_ol_limit_r()
{
	return ta_ol_limit;
}

void powervr2_device::ta_ol_limit_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&ta_ol_limit);
}

uint32_t powervr2_device::ta_isp_limit_r()
{
	return ta_isp_limit;
}

void powervr2_device::ta_isp_limit_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&ta_isp_limit);
}

uint32_t powervr2_device::ta_next_opb_r()
{
	return ta_next_opb;
}

uint32_t powervr2_device::ta_itp_current_r()
{
	return ta_itp_current;
}

uint32_t powervr2_device::ta_alloc_ctrl_r()
{
	return ta_alloc_ctrl;
}

void powervr2_device::ta_alloc_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&ta_alloc_ctrl);
}

uint32_t powervr2_device::ta_list_init_r()
{
	return 0; //bit 31 always return 0, a probable left-over in Crazy Taxi reads this and discards the read (?)
}

void powervr2_device::ta_list_init_w(uint32_t data)
{
	if(data & 0x80000000) {
		auto profile = g_profiler.start(PROFILER_USER2);
		tafifo_pos=0;
		tafifo_mask=7;
		tafifo_vertexwords=8;
		tafifo_listtype= DISPLAY_LIST_NONE;

		LOGTACMD("list init ol=(%08x, %08x) isp=(%08x, %08x), alloc=%08x obp=%08x\n",
			ta_ol_base, ta_ol_limit,
			ta_isp_base, ta_isp_limit,
			ta_alloc_ctrl,
			ta_next_opb_init
		);

		ta_next_opb = ta_next_opb_init;
		ta_itp_current = ta_isp_base;
		alloc_ctrl_OPB_Mode = ta_alloc_ctrl & 0x100000; // 0 up 1 down
		alloc_ctrl_PT_OPB = (4 << ((ta_alloc_ctrl >> 16) & 3)) & 0x38; // number of 32 bit words (0,8,16,32)
		alloc_ctrl_TM_OPB = (4 << ((ta_alloc_ctrl >> 12) & 3)) & 0x38;
		alloc_ctrl_T_OPB = (4 << ((ta_alloc_ctrl >> 8) & 3)) & 0x38;
		alloc_ctrl_OM_OPB = (4 << ((ta_alloc_ctrl >> 4) & 3)) & 0x38;
		alloc_ctrl_O_OPB = (4 << ((ta_alloc_ctrl >> 0) & 3)) & 0x38;
		listtype_used |= (1+4);
		// use ta_isp_base and select buffer for grab data
		grabsel = -1;
		// try to find already used buffer but not busy
		for (int a=0;a < NUM_BUFFERS;a++)
			if ((grab[a].ispbase == ta_isp_base) && (grab[a].busy == 0) && (grab[a].valid == 1)) {
				grabsel=a;
				break;
			}

		// try a buffer not used yet
		if (grabsel < 0)
			for (int a=0;a < NUM_BUFFERS;a++)
				if (grab[a].valid == 0) {
					grabsel=a;
					break;
				}

		// find a non busy buffer starting from the last one used
		if (grabsel < 0)
			for (int a=0;a < 3;a++)
				if (grab[(grabsellast+1+a) & 3].busy == 0) {
					grabsel=a;
					break;
				}

		if (grabsel < 0)
			throw emu_fatalerror("powervr2_device::ta_list_init_w: TA grabber error B!");
		grabsellast=grabsel;
		grab[grabsel].ispbase=ta_isp_base;
		grab[grabsel].busy=0;
		grab[grabsel].valid=1;
		grab[grabsel].verts_size=0;
		for (int group = 0; group < DISPLAY_LIST_COUNT; group++) {
			grab[grabsel].groups[group].strips_size=0;
		}
	}
}


uint32_t powervr2_device::ta_yuv_tex_base_r()
{
	return ta_yuv_tex_base;
}

void powervr2_device::ta_yuv_tex_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&ta_yuv_tex_base);

	// Writes causes a reset in YUV FIFO pipeline
	ta_yuv_index = 0;
	ta_yuv_u_ptr = 0;
	ta_yuv_v_ptr = 0;
}

uint32_t powervr2_device::ta_yuv_tex_ctrl_r()
{
	return ta_yuv_tex_ctrl;
}

void powervr2_device::ta_yuv_tex_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&ta_yuv_tex_ctrl);

	ta_yuv_v_size = (((ta_yuv_tex_ctrl >> 8) & 0x3f) + 1) * 16;
	ta_yuv_u_size = ((ta_yuv_tex_ctrl & 0x3f) + 1) * 16;

	if (ta_yuv_tex_ctrl & 0x01010000)
		throw emu_fatalerror("YUV422 mode selected %08x", ta_yuv_tex_ctrl);
}

// TODO: unemulated YUV macroblock live count, nothing seems to use it?
uint32_t powervr2_device::ta_yuv_tex_cnt_r()
{
	if (!machine().side_effects_disabled())
		LOGWARN("%s yuv_tex_cnt read!", machine().describe_context());
	return ta_yuv_tex_cnt;
}

void powervr2_device::ta_list_cont_w(uint32_t data)
{
	if(data & 0x80000000) {
		tafifo_listtype= DISPLAY_LIST_NONE; // no list being received
		listtype_used |= (1+4);
	}
}

uint32_t powervr2_device::ta_next_opb_init_r()
{
	return ta_next_opb_init;
}

void powervr2_device::ta_next_opb_init_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&ta_next_opb_init);
}


uint32_t powervr2_device::fog_table_r(offs_t offset)
{
	return fog_table[offset];
}

void powervr2_device::fog_table_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(fog_table+offset);
}

uint32_t powervr2_device::palette_r(offs_t offset)
{
	return palette[offset];
}

void powervr2_device::palette_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(palette+offset);
}

void powervr2_device::update_screen_format()
{
	/*                        00=VGA    01=NTSC   10=PAL,   11=illegal/undocumented */
	const int spg_clks[4] = { 26944080, 13458568, 13462800, 26944080 };
	int32_t spg_hsize = spg_load & 0x3ff;
	int32_t spg_vsize = (spg_load >> 16) & 0x3ff;
	int32_t spg_hbstart = spg_hblank & 0x3ff;
	int32_t spg_hbend = (spg_hblank >> 16) & 0x3ff;
	int32_t spg_vbstart = spg_vblank & 0x3ff;
	int32_t spg_vbend = (spg_vblank >> 16) & 0x3ff;
	//int32_t vo_horz_start_pos = vo_startx & 0x3ff;
	//int32_t vo_vert_start_pos_f1 = vo_starty & 0x3ff;
	int pclk = spg_clks[(spg_control >> 6) & 3] * (((spg_control & 0x10) >> 4)+1);

	attoseconds_t refresh = HZ_TO_ATTOSECONDS(pclk) * spg_hsize * spg_vsize;

	rectangle visarea = screen().visible_area();

	visarea.min_x = spg_hbend;
	visarea.max_x = spg_hbstart - 1;
	visarea.min_y = spg_vbend;
	visarea.max_y = spg_vbstart - 1;

	// Sanitize
	if(visarea.max_x >= spg_hsize)
		visarea.max_x = spg_hsize-1;
	if(visarea.max_y >= spg_vsize)
		visarea.max_y = spg_vsize-1;
	if(visarea.min_x > visarea.max_x)
		visarea.min_x = visarea.max_x;
	if(visarea.min_y > visarea.max_y)
		visarea.min_y = visarea.max_y;

	screen().configure(spg_hsize, spg_vsize, visarea, refresh );
}


uint32_t powervr2_device::sb_pdstap_r()
{
	return sb_pdstap;
}

void powervr2_device::sb_pdstap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&sb_pdstap);
	m_pvr_dma.pvr_addr = sb_pdstap;
}

uint32_t powervr2_device::sb_pdstar_r()
{
	return sb_pdstar;
}

void powervr2_device::sb_pdstar_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&sb_pdstar);
	m_pvr_dma.sys_addr = sb_pdstar;
}

uint32_t powervr2_device::sb_pdlen_r()
{
	return sb_pdlen;
}

void powervr2_device::sb_pdlen_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&sb_pdlen);
	m_pvr_dma.size = sb_pdlen;
}

uint32_t powervr2_device::sb_pddir_r()
{
	return sb_pddir;
}

void powervr2_device::sb_pddir_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&sb_pddir);
	m_pvr_dma.dir = sb_pddir;
}

uint32_t powervr2_device::sb_pdtsel_r()
{
	return sb_pdtsel;
}

void powervr2_device::sb_pdtsel_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&sb_pdtsel);
	m_pvr_dma.sel = sb_pdtsel & 1;
}

uint32_t powervr2_device::sb_pden_r()
{
	return sb_pden;
}

void powervr2_device::sb_pden_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&sb_pden);
	m_pvr_dma.flag = sb_pden & 1;
}

uint32_t powervr2_device::sb_pdst_r()
{
	return sb_pdst;
}

void powervr2_device::sb_pdst_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&sb_pdst);

	uint32_t old = m_pvr_dma.start & 1;
	m_pvr_dma.start = sb_pdst & 1;

	// 0 -> 1 transition starts the DMA
	if(((old & 1) == 0) && m_pvr_dma.flag && m_pvr_dma.start && ((m_pvr_dma.sel & 1) == 0))
		pvr_dma_execute(space);
}

uint32_t powervr2_device::sb_pdapro_r()
{
	return sb_pdapro;
}

void powervr2_device::sb_pdapro_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&sb_pdapro);
}


TIMER_CALLBACK_MEMBER(powervr2_device::transfer_opaque_list_irq)
{
	LOGIRQ("[%d] EOXFER OPLST %d\n", screen().frame_number(), screen().vpos());

	irq_cb(EOXFER_OPLST_IRQ);
}

TIMER_CALLBACK_MEMBER(powervr2_device::transfer_opaque_modifier_volume_list_irq)
{
	LOGIRQ("[%d] EOXFER OPMV %d\n", screen().frame_number(), screen().vpos());

	irq_cb(EOXFER_OPMV_IRQ);
}

TIMER_CALLBACK_MEMBER(powervr2_device::transfer_translucent_list_irq)
{
	LOGIRQ("[%d] EOXFER TRLST %d\n", screen().frame_number(), screen().vpos());

	irq_cb(EOXFER_TRLST_IRQ);
}

TIMER_CALLBACK_MEMBER(powervr2_device::transfer_translucent_modifier_volume_list_irq)
{
	LOGIRQ("[%d] EOXFER TRMV %d\n", screen().frame_number(), screen().vpos());

	irq_cb(EOXFER_TRMV_IRQ);
}

TIMER_CALLBACK_MEMBER(powervr2_device::transfer_punch_through_list_irq)
{
	LOGIRQ("[%d] EOXFER PTLST %d\n", screen().frame_number(), screen().vpos());

	irq_cb(EOXFER_PTLST_IRQ);
}

void powervr2_device::process_ta_fifo()
{
	/* first byte in the buffer is the Parameter Control Word

	 pppp pppp gggg gggg oooo oooo oooo oooo

	 p = para control
	 g = group control
	 o = object control

	*/

	receiveddata *rd = &grab[grabsel];

	// Para Control
	paracontrol=(tafifo_buff[0] >> 24) & 0xff;
	// 0 end of list
	// 1 user tile clip
	// 2 object list set
	// 3 reserved
	// 4 polygon/modifier volume
	// 5 sprite
	// 6 reserved
	// 7 vertex
	paratype=(paracontrol >> 5) & 7;
	endofstrip=(paracontrol >> 4) & 1;
	listtype=(paracontrol >> 0) & 7;
	if ((paratype >= 4) && (paratype <= 6))
	{
		global_paratype = paratype;
		// Group Control
		groupcontrol=(tafifo_buff[0] >> 16) & 0xff;
		groupen=(groupcontrol >> 7) & 1;
		striplen=(groupcontrol >> 2) & 3;
		userclip=(groupcontrol >> 0) & 3;
		// Obj Control
		objcontrol=(tafifo_buff[0] >> 0) & 0xffff;
		shadow=(objcontrol >> 7) & 1;
		volume=(objcontrol >> 6) & 1;
		coltype=(objcontrol >> 4) & 3;
		texture=(objcontrol >> 3) & 1;
		offset_color_enable=texture ? (objcontrol >> 2) & 1 : 0;
		gouraud=(objcontrol >> 1) & 1;
		uv16bit=(objcontrol >> 0) & 1;
	}

	// check if we need 8 words more
	if (tafifo_mask == 7)
	{
		parameterconfig = pvr_parameterconfig[objcontrol & 0x3d];
		// decide number of words per vertex
		if (paratype == 7)
		{
			if ((global_paratype == 5) ||
				(tafifo_listtype == DISPLAY_LIST_OPAQUE_MOD) ||
				(tafifo_listtype == DISPLAY_LIST_TRANS_MOD))
			{
				tafifo_vertexwords = 16;
			}
			if (tafifo_vertexwords == 16)
			{
				tafifo_mask = 15;
				tafifo_pos = 8;
				return;
			}
		}
		// decide number of words when not a vertex
		tafifo_vertexwords=pvr_wordsvertex[parameterconfig];
		if ((paratype == 4) && ((listtype != 1) && (listtype != 3)))
			if (pvr_wordspolygon[parameterconfig] == 16)
			{
				tafifo_mask = 15;
				tafifo_pos = 8;
				return;
			}
	}
	bool have_16_byte_header = tafifo_mask != 7;
	tafifo_mask = 7;

	// now we heve all the needed words

	/*
	 * load per-polygon colors if color type is 2 or 3 or parameter type is
	 * 5 (quad).  For color types 0 and 1, color is determined entirely on a
	 * per-vertex basis.
	 */
	if (paratype == 4)
	{
		switch (coltype) {
		case 2:
			if (offset_color_enable) {
				memcpy(poly_base_color, tafifo_buff + 8, 4 * sizeof(float));
				memcpy(poly_offs_color, tafifo_buff + 12, 4 * sizeof(float));
			} else {
				memcpy(poly_base_color, tafifo_buff + 4, 4 * sizeof(float));
				memset(poly_offs_color, 0, sizeof(poly_offs_color));
			}
			memcpy(poly_last_mode_2_base_color, poly_base_color, sizeof(poly_last_mode_2_base_color));
			break;
		case 3:
			memcpy(poly_base_color, poly_last_mode_2_base_color, sizeof(poly_base_color));
			memset(poly_offs_color, 0, sizeof(poly_offs_color));
			break;
		default:
			memset(poly_base_color, 0, sizeof(poly_base_color));
			memset(poly_offs_color, 0, sizeof(poly_offs_color));
			break;
		}
	} else if (paratype == 5) {
		packed_argb_to_float_argb(poly_base_color, tafifo_buff[4]);
		if (offset_color_enable) {
			packed_argb_to_float_argb(poly_offs_color, tafifo_buff[5]);
		} else {
			memset(poly_offs_color, 0, sizeof(poly_offs_color));
		}
	}

	// here we should generate the data for the various tiles
	// for now, just interpret their meaning
	if (paratype == 0)
	{
		LOGTATILE("Para Type 0 End of List\n");

		/* Process transfer FIFO done irqs here */
		//printf("%d %d\n",tafifo_listtype,screen().vpos());
		// FIXME: timing of these
		switch (tafifo_listtype)
		{
			case DISPLAY_LIST_OPAQUE:
				opaque_irq_timer->adjust(attotime::from_usec(100));
				break;
			case DISPLAY_LIST_OPAQUE_MOD:
				opaque_modifier_volume_irq_timer->adjust(attotime::from_usec(100));
				break;
			case DISPLAY_LIST_TRANS:
				translucent_irq_timer->adjust(attotime::from_usec(100));
				break;
			case DISPLAY_LIST_TRANS_MOD:
				translucent_modifier_volume_irq_timer->adjust(attotime::from_usec(100));
				break;
			case DISPLAY_LIST_PUNCH_THROUGH:
				punch_through_irq_timer->adjust(attotime::from_usec(100));
				break;
		}
		tafifo_listtype = DISPLAY_LIST_NONE; // no list being received
		listtype_used |= (2+8);
	}
	else if (paratype == 1)
	{
		LOGTATILE("Para Type 1 User Tile Clip\n");
		LOGTATILE(" (%d , %d)-(%d , %d)\n", tafifo_buff[4], tafifo_buff[5], tafifo_buff[6], tafifo_buff[7]);
	}
	else if (paratype == 2)
	{
		LOGTATILE("Para Type 2 Object List Set at %08x\n", tafifo_buff[1]);
		LOGTATILE(" (%d , %d)-(%d , %d)\n", tafifo_buff[4], tafifo_buff[5], tafifo_buff[6], tafifo_buff[7]);
	}
	else if (paratype == 3)
	{
		LOGWARN("Para Type %x Unknown!\n", tafifo_buff[0]);
	}
	else
	{ // global parameter or vertex parameter
		LOGTATILE("Para Type %d", paratype);
		if (paratype == 7)
			LOGTATILE(" End of Strip %d", endofstrip);
		if (listtype_used & 3)
			LOGTATILE(" List Type %d", listtype);
		LOGTATILE("\n");

		// set type of list currently being received
		if ((paratype == 4) || (paratype == 5) || (paratype == 6))
		{
			if (tafifo_listtype < 0)
			{
				tafifo_listtype = listtype;
			}
		}
		listtype_used = listtype_used ^ (listtype_used & 3);

		if ((paratype == 4) || (paratype == 5))
		{ // quad or polygon
			depthcomparemode=(tafifo_buff[1] >> 29) & 7;
			cullingmode=(tafifo_buff[1] >> 27) & 3;
			zwritedisable=(tafifo_buff[1] >> 26) & 1;
			cachebypass=(tafifo_buff[1] >> 21) & 1;
			dcalcctrl=(tafifo_buff[1] >> 20) & 1;
			volumeinstruction=(tafifo_buff[1] >> 29) & 7;

			//textureusize=1 << (3+((tafifo_buff[2] >> 3) & 7));
			//texturevsize=1 << (3+(tafifo_buff[2] & 7));
			texturesizes=tafifo_buff[2] & 0x3f;
			blend_mode = tafifo_buff[2] >> 26;
			srcselect=(tafifo_buff[2] >> 25) & 1;
			dstselect=(tafifo_buff[2] >> 24) & 1;
			fogcontrol=(tafifo_buff[2] >> 22) & 3;
			colorclamp=(tafifo_buff[2] >> 21) & 1;
			use_alpha = (tafifo_buff[2] >> 20) & 1;
			ignoretexalpha=(tafifo_buff[2] >> 19) & 1;
			flipuv=(tafifo_buff[2] >> 17) & 3;
			clampuv=(tafifo_buff[2] >> 15) & 3;
			filtermode=(tafifo_buff[2] >> 13) & 3;
			sstexture=(tafifo_buff[2] >> 12) & 1;
			mmdadjust=(tafifo_buff[2] >> 8) & 1;
			tsinstruction=(tafifo_buff[2] >> 6) & 3;
			if (texture == 1)
			{
				textureaddress=(tafifo_buff[3] & 0x1FFFFF) << 3;
				scanorder=(tafifo_buff[3] >> 26) & 1;
				pixelformat=(tafifo_buff[3] >> 27) & 7;
				mipmapped=(tafifo_buff[3] >> 31) & 1;
				vqcompressed=(tafifo_buff[3] >> 30) & 1;
				strideselect=(tafifo_buff[3] >> 25) & 1;
				paletteselector=(tafifo_buff[3] >> 21) & 0x3F;

				LOGTATILE(" Texture at %08x format %d\n", (tafifo_buff[3] & 0x1FFFFF) << 3, pixelformat);
			}
			if (use_alpha == 0)
			{
				// Alpha value in base/offset color is to be treated as 1.0/0xFF.
				poly_base_color[0] = 1.0;
				if (offset_color_enable)
					poly_offs_color[0] = 1.0;
			}
			if (paratype == 4)
			{
				LOGTATILE(" %s\n",
					((tafifo_listtype == DISPLAY_LIST_OPAQUE_MOD) ||
					 (tafifo_listtype == DISPLAY_LIST_TRANS_MOD)) ? "Modifier Volume" : "Polygon"
				);
			}
			if (paratype == 5)
			{ // quad
				LOGTATILE(" Sprite\n");
			}
		}

		if (paratype == 7)
		{ // vertex
			if (tafifo_listtype < 0 || tafifo_listtype >= DISPLAY_LIST_COUNT) {
				LOGWARN("unrecognized list type %d\n", tafifo_listtype);
				return;
			}
			struct poly_group *grp = rd->groups + tafifo_listtype;
			if ((tafifo_listtype == DISPLAY_LIST_OPAQUE_MOD) ||
				(tafifo_listtype == DISPLAY_LIST_TRANS_MOD))
			{
				LOGTATILE(" Vertex modifier volume");
				LOGTATILE(" A(%f,%f,%f) B(%f,%f,%f) C(%f,%f,%f)\n",
					u2f(tafifo_buff[1]), u2f(tafifo_buff[2]), u2f(tafifo_buff[3]),
					u2f(tafifo_buff[4]), u2f(tafifo_buff[5]), u2f(tafifo_buff[6]),
					u2f(tafifo_buff[7]), u2f(tafifo_buff[8]), u2f(tafifo_buff[9])
				);
			}
			else if (global_paratype == 5)
			{
				LOGTATILE(" Vertex sprite");
				LOGTATILE(" A(%f,%f,%f) B(%f,%f,%f) C(%f,%f,%f) D(%f,%f,)\n",
					u2f(tafifo_buff[1]), u2f(tafifo_buff[2]), u2f(tafifo_buff[3]),
					u2f(tafifo_buff[4]), u2f(tafifo_buff[5]), u2f(tafifo_buff[6]),
					u2f(tafifo_buff[7]), u2f(tafifo_buff[8]), u2f(tafifo_buff[9]),
					u2f(tafifo_buff[10]), u2f(tafifo_buff[11])
				);

				if (rd->verts_size <= (MAX_VERTS - 6) && grp->strips_size < MAX_STRIPS)
				{
					vert * const tv = &rd->verts[rd->verts_size];
					tv[0].x = u2f(tafifo_buff[0x1]);
					tv[0].y = u2f(tafifo_buff[0x2]);
					tv[0].w = u2f(tafifo_buff[0x3]);
					tv[1].x = u2f(tafifo_buff[0x4]);
					tv[1].y = u2f(tafifo_buff[0x5]);
					tv[1].w = u2f(tafifo_buff[0x6]);
					tv[3].x = u2f(tafifo_buff[0x7]);
					tv[3].y = u2f(tafifo_buff[0x8]);
					tv[3].w = u2f(tafifo_buff[0x9]);
					tv[2].x = u2f(tafifo_buff[0xa]);
					tv[2].y = u2f(tafifo_buff[0xb]);
					tv[2].w = tv[0].w+tv[3].w-tv[1].w;

					if (texture == 1)
					{
						tv[0].u = u2f(tafifo_buff[0xd] & 0xffff0000);
						tv[0].v = u2f(tafifo_buff[0xd] << 16);
						tv[1].u = u2f(tafifo_buff[0xe] & 0xffff0000);
						tv[1].v = u2f(tafifo_buff[0xe] << 16);
						tv[3].u = u2f(tafifo_buff[0xf] & 0xffff0000);
						tv[3].v = u2f(tafifo_buff[0xf] << 16);
						tv[2].u = tv[0].u+tv[3].u-tv[1].u;
						tv[2].v = tv[0].v+tv[3].v-tv[1].v;
					}

					for (int idx = 0; idx < 4; idx++)
					{
						memcpy(tv[idx].b, poly_base_color, sizeof(tv[idx].b));
						memcpy(tv[idx].o, poly_offs_color, sizeof(tv[idx].o));
					}

					strip * const ts = &grp->strips[grp->strips_size++];
					tex_get_info(&ts->ti);
					ts->svert = rd->verts_size;
					ts->evert = rd->verts_size + 3;

					rd->verts_size += 4;
				}
			}
			else if (global_paratype == 4)
			{
				LOGTATILE(" Vertex polygon");
				LOGTATILE(" V(%f,%f,%f) T(%f,%f)\n",
					u2f(tafifo_buff[1]), u2f(tafifo_buff[2]), u2f(tafifo_buff[3]),
					u2f(tafifo_buff[4]), u2f(tafifo_buff[5])
				);

				if (rd->verts_size <= (MAX_VERTS - 6))
				{
					float vert_offset_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
					float vert_base_color[4];

					float base_intensity, offs_intensity;

					switch (coltype) {
					case 0:
						// packed color
						packed_argb_to_float_argb(vert_base_color, tafifo_buff[6]);
						break;
					case 1:
						// floating-point color
						if (have_16_byte_header) {
							memcpy(vert_base_color, tafifo_buff + 8,
								   sizeof(vert_base_color));
							memcpy(vert_offset_color, tafifo_buff + 12,
								   sizeof(vert_offset_color));
						} else {
							memcpy(vert_base_color, tafifo_buff + 4,
								   sizeof(vert_base_color));
						}
						break;
					case 2:
					case 3:
						/*
						* base/offset color were previously
						* specified on a per-polygon basis.
						* To get the per-vertex base and
						* offset colors, they are scaled by
						* per-vertex scalar values.
						*/
						memcpy(&base_intensity, tafifo_buff + 6, sizeof(base_intensity));
						memcpy(&offs_intensity, tafifo_buff + 7, sizeof(offs_intensity));
						vert_base_color[0] = poly_base_color[0] * base_intensity;
						vert_base_color[1] = poly_base_color[1] * base_intensity;
						vert_base_color[2] = poly_base_color[2] * base_intensity;
						vert_base_color[3] = poly_base_color[3] * base_intensity;
						if (offset_color_enable) {
							vert_offset_color[0] = poly_offs_color[0] * offs_intensity;
							vert_offset_color[1] = poly_offs_color[1] * offs_intensity;
							vert_offset_color[2] = poly_offs_color[2] * offs_intensity;
							vert_offset_color[3] = poly_offs_color[3] * offs_intensity;
						}
						break;
					default:
						// This will never actually happen, coltype is 2-bits.
						logerror("line %d of %s - coltype is %d\n", coltype);
						memset(vert_base_color, 0, sizeof(vert_base_color));
					}

					/* add a vertex to our list */
					/* this is used for 3d stuff, ie most of the graphics (see guilty gear, confidential mission, maze of the kings etc.) */
					/* -- this is also wildly inaccurate! */
					vert *tv = &rd->verts[rd->verts_size];

					tv->x=u2f(tafifo_buff[1]);
					tv->y=u2f(tafifo_buff[2]);
					tv->w=u2f(tafifo_buff[3]);
					tv->u=u2f(tafifo_buff[4]);
					tv->v=u2f(tafifo_buff[5]);
					memcpy(tv->b, vert_base_color, sizeof(tv->b));
					memcpy(tv->o, vert_offset_color, sizeof(tv->o));

					if(grp->strips_size < MAX_STRIPS &&
						((!grp->strips_size) ||
						grp->strips[grp->strips_size-1].evert != -1))
					{
						strip *ts = &grp->strips[grp->strips_size++];
						tex_get_info(&ts->ti);
						ts->svert = rd->verts_size;
						ts->evert = -1;
					}
					if(endofstrip)
						grp->strips[grp->strips_size-1].evert = rd->verts_size;
					rd->verts_size++;
				}
			}
		}
	}
}

void powervr2_device::ta_fifo_poly_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	if (mem_mask == 0xffffffffffffffffU)    // 64 bit
	{
		tafifo_buff[tafifo_pos]=(uint32_t)data;
		tafifo_buff[tafifo_pos+1]=(uint32_t)(data >> 32);
		LOGTAFIFO("ta_fifo_poly_w: write64 %08x = %x -> %08x %08x\n",
			0x10000000 + offset * 8,
			data,
			tafifo_buff[tafifo_pos],
			tafifo_buff[tafifo_pos + 1]
		);
		tafifo_pos += 2;
	}
	else
	{
		LOGWARN("ta_fifo_poly_w:  Unmapped write64 %08x = %x mask %x\n", 0x10000000+offset*8, data, mem_mask);
	}

	tafifo_pos &= tafifo_mask;

	// if the command is complete, process it
	if (tafifo_pos == 0)
		process_ta_fifo();

}

TIMER_CALLBACK_MEMBER(powervr2_device::yuv_convert_end)
{
	irq_cb(EOXFER_YUV_IRQ);
	yuv_timer_end->adjust(attotime::never);
}

void powervr2_device::ta_fifo_yuv_w(uint8_t data)
{
	dc_state *state = machine().driver_data<dc_state>();

	yuv_fifo[ta_yuv_index] = data;
	ta_yuv_index++;

	if(ta_yuv_index == 0x180)
	{
#if LIVE_YUV_VIEW
		popmessage("YUV fifo write base=%08x ctrl=%08x (%dx%d)",
			ta_yuv_tex_base, ta_yuv_tex_ctrl,
			ta_yuv_u_size, ta_yuv_v_size
		);
#endif

		ta_yuv_index = 0;
		for(int y = 0; y < 16; y++)
		{
			for(int x = 0; x < 16; x+=2)
			{
				int dst_addr;
				int u, v, y0, y1;

				dst_addr = ta_yuv_tex_base;
				dst_addr+= (ta_yuv_u_ptr + x) * 2;
				// pitch is given by U size (320 for luptype, 512 for ss2005/kurucham)
				dst_addr+= ((ta_yuv_v_ptr + y) * ta_yuv_u_size * 2);

				u = yuv_fifo[0x00+(x>>1)+((y>>1)*8)];
				v = yuv_fifo[0x40+(x>>1)+((y>>1)*8)];
				y0 = yuv_fifo[0x80+((x&8) ? 0x40 : 0x00)+((y&8) ? 0x80 : 0x00)+(x&6)+((y&7)*8)];
				y1 = yuv_fifo[0x80+((x&8) ? 0x40 : 0x00)+((y&8) ? 0x80 : 0x00)+(x&6)+((y&7)*8)+1];

				*(uint8_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + BYTE8_XOR_LE(dst_addr)) = u;
				*(uint8_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + BYTE8_XOR_LE(dst_addr+1)) = y0;
				*(uint8_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + BYTE8_XOR_LE(dst_addr+2)) = v;
				*(uint8_t *)((reinterpret_cast<uint8_t *>(dc_texture_ram)) + BYTE8_XOR_LE(dst_addr+3)) = y1;
			}
		}

		ta_yuv_u_ptr += 16;
		if(ta_yuv_u_ptr == ta_yuv_u_size)
		{
			ta_yuv_u_ptr = 0;
			ta_yuv_v_ptr += 16;
			if(ta_yuv_v_ptr == ta_yuv_v_size)
			{
				ta_yuv_v_ptr = 0;
				// TODO: actual timings
				yuv_timer_end->adjust(state->m_maincpu->cycles_to_attotime((ta_yuv_u_size/16)*(ta_yuv_v_size/16)*0x180));
			}
		}
	}
}

// SB_LMMODE0
void powervr2_device::ta_texture_directpath0_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	// That's not in the pvr control address space, it's in g2's
	//  int mode = pvrctrl_regs[SB_LMMODE0]&1;
	int mode = 0;
	if (mode&1)
	{
		printf("ta_texture_directpath0_w 32-bit access!\n");
		COMBINE_DATA(&dc_framebuffer_ram[offset]);
	}
	else
	{
		COMBINE_DATA(&dc_texture_ram[offset]);
	}
}

// SB_LMMODE1
void powervr2_device::ta_texture_directpath1_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	// That's not in the pvr control address space, it's in g2's
	//  int mode = pvrctrl_regs[SB_LMMODE1]&1;
	int mode = 0;
	if (mode&1)
	{
		printf("ta_texture_directpath1_w 32-bit access!\n");
		COMBINE_DATA(&dc_framebuffer_ram[offset]);
	}
	else
	{
		COMBINE_DATA(&dc_texture_ram[offset]);
	}
}

/* test video start */
uint32_t powervr2_device::dilate0(uint32_t value,int bits) // dilate first "bits" bits in "value"
{
	uint32_t x,m1,m2,m3;
	int a;

	x = value;
	for (a=0;a < bits;a++)
	{
		m2 = 1 << (a << 1);
		m1 = m2 - 1;
		m3 = (~m1) << 1;
		x = (x & m1) + (x & m2) + ((x & m3) << 1);
	}
	return x;
}

uint32_t powervr2_device::dilate1(uint32_t value,int bits) // dilate first "bits" bits in "value"
{
	uint32_t x,m1,m2,m3;
	int a;

	x = value;
	for (a=0;a < bits;a++)
	{
		m2 = 1 << (a << 1);
		m1 = m2 - 1;
		m3 = (~m1) << 1;
		x = (x & m1) + ((x & m2) << 1) + ((x & m3) << 1);
	}
	return x;
}

void powervr2_device::computedilated()
{
	int a,b;

	for (b=0;b <= 14;b++)
		for (a=0;a < 1024;a++) {
			dilated0[b][a]=dilate0(a,b);
			dilated1[b][a]=dilate1(a,b);
		}
	for (b=0;b <= 7;b++)
		for (a=0;a <= 7;a++)
			dilatechose[(b << 3) + a]=3+(a < b ? a : b);
}

inline uint32_t powervr2_device::sample_nontextured(texinfo *ti, float u, float v, uint32_t offset_color, uint32_t base_color)
{
	return bls24(base_color, offset_color) | (base_color & 0xff000000);
}

template <int tsinst, bool bilinear>
inline uint32_t powervr2_device::sample_textured(texinfo *ti, float u, float v, uint32_t offset_color, uint32_t base_color)
{
	uint32_t tmp;
	uint32_t c = (this->*(ti->r))(ti, u, v);
	if (bilinear)
	{
		uint32_t c1 = (this->*(ti->r))(ti, u+1.0f, v);
		uint32_t c2 = (this->*(ti->r))(ti, u+1.0f, v+1.0f);
		uint32_t c3 = (this->*(ti->r))(ti, u, v+1.0f);
		c = bilinear_filter(c, c1, c2, c3, u, v);
	}

	switch (tsinst) {
		case 0:
			// decal
			c = bls24(c, offset_color) | (c & 0xff000000);
			break;
		case 1:
			// modulate
			tmp = blc(c, base_color);
			tmp = bls24(tmp, offset_color);
			tmp |= c & 0xff000000;
			c = tmp;
			break;
		case 2:
			// decal with alpha
			tmp = bls24(bla(c, c), blia(base_color, c));
			c = bls24(tmp, offset_color) | (base_color & 0xff000000);
			break;
		case 3:
			// modulate with alpha
			tmp = blc(c, base_color);
			tmp = bls24(tmp, offset_color);
			tmp |= (((c >> 24) * (base_color >> 24)) >> 8) << 24;
			c = tmp;
			break;
	}
	return c;
}

template <powervr2_device::pix_sample_fn sample_fn, int group_no>
inline void powervr2_device::render_hline(bitmap_rgb32 &bitmap, texinfo *ti, int y, float xl, float xr, float ul, float ur, float vl, float vr, float wl, float wr, float const bl_in[4], float const br_in[4], float const offl_in[4], float const offr_in[4])
{
	int idx;
	int xxl, xxr;
	float dx, ddx, dudx, dvdx, dwdx;
	uint32_t *tdata;
	float *wbufline;

	float bl[4], offl[4];

	if(xr < 0 || xl >= 640)
		return;

	xxl = round(xl);
	xxr = round(xr);

	if(xxl == xxr)
		return;

	memcpy(bl, bl_in, sizeof(bl));
	memcpy(offl, offl_in, sizeof(offl));

	dx = xr-xl;
	float dx_recip = 1.0f / dx;

	dudx = (ur-ul) * dx_recip;
	dvdx = (vr-vl) * dx_recip;
	dwdx = (wr-wl) * dx_recip;

	float dbdx[4] = {
		(br_in[0] - bl[0]) * dx_recip,
		(br_in[1] - bl[1]) * dx_recip,
		(br_in[2] - bl[2]) * dx_recip,
		(br_in[3] - bl[3]) * dx_recip
	};

	float dodx[4] = {
		(offr_in[0] - offl[0]) * dx_recip,
		(offr_in[1] - offl[1]) * dx_recip,
		(offr_in[2] - offl[2]) * dx_recip,
		(offr_in[3] - offl[3]) * dx_recip
	};

	if(xxl < 0)
		xxl = 0;
	if(xxr > 640)
		xxr = 640;

	// Target the pixel center
	ddx = xxl + 0.5f - xl;
	ul += ddx*dudx;
	vl += ddx*dvdx;
	wl += ddx*dwdx;
	for (idx = 0; idx < 4; idx++) {
		bl[idx] += ddx * dbdx[idx];
		offl[idx] += ddx * dodx[idx];
	}

	tdata = &bitmap.pix(y, xxl);
	wbufline = &wbuffer[y][xxl];

	while(xxl < xxr) {
		if((wl >= *wbufline)) {
			uint32_t c;
			float u = ul/wl;
			float v = vl/wl;
			uint32_t offset_color = float_argb_to_packed_argb(offl);
			uint32_t base_color = float_argb_to_packed_argb(bl);
			c = (this->*sample_fn)(ti, u, v, offset_color, base_color);
			if (group_no == DISPLAY_LIST_OPAQUE) {
				*tdata = c;
				*wbufline = wl;
			} else {
				if(c & 0xff000000) {
					*wbufline = wl;
					*tdata = ti->blend(c, *tdata);
				}
			}
		}
		wbufline++;
		tdata++;

		ul += dudx;
		vl += dvdx;
		wl += dwdx;
		for (idx = 0; idx < 4; idx++) {
			bl[idx] += dbdx[idx];
			offl[idx] += dodx[idx];
		}
		xxl ++;
	}
}

template <powervr2_device::pix_sample_fn sample_fn, int group_no>
inline void powervr2_device::render_span(bitmap_rgb32 &bitmap, texinfo *ti,
									float y0, float y1,
									float xl, float xr,
									float ul, float ur,
									float vl, float vr,
									float wl, float wr,
									float const bl_in[4], float const br_in[4],
									float const offl_in[4], float const offr_in[4],
									float dxldy, float dxrdy,
									float duldy, float durdy,
									float dvldy, float dvrdy,
									float dwldy, float dwrdy,
									float const dbldy[4], float const dbrdy[4],
									float const doldy[4], float const dordy[4])
{
	int idx;
	float dy;
	int yy0, yy1;

	// demofist, chocomk (hardlocks with -drc, MT#8088)
	// TODO: should throw an error?
	if (std::isnan(y0) || std::isnan(y1))
		return;

	if(y1 <= 0)
		return;
	if(y1 > 480)
		y1 = 480;

	float bl[4], br[4], offl[4], offr[4];
	memcpy(bl, bl_in, sizeof(bl));
	memcpy(br, br_in, sizeof(br));
	memcpy(offl, offl_in, sizeof(offl));
	memcpy(offr, offr_in, sizeof(offr));

	if(y0 < 0) {
		xl += -dxldy*y0;
		xr += -dxrdy*y0;
		ul += -duldy*y0;
		ur += -durdy*y0;
		vl += -dvldy*y0;
		vr += -dvrdy*y0;
		wl += -dwldy*y0;
		wr += -dwrdy*y0;

		for (idx = 0; idx < 4; idx++) {
			bl[idx] += -dbldy[idx] * y0;
			br[idx] += -dbrdy[idx] * y0;
			offl[idx] += -doldy[idx] * y0;
			offr[idx] += -dordy[idx] * y0;
		}
		y0 = 0;
	}

	yy0 = round(y0);
	yy1 = round(y1);

	if((yy0 < 0 && y0 > 0) || (yy1 < 0 && y1 > 0)) //temp handling of int32 overflow, needed by hotd2/totd
		return;

	dy = yy0+0.5f-y0;

	if(0)
		fprintf(stderr, "%f %f %f %f -> %f %f | %f %f -> %f %f\n",
				(double) y0,
				(double) dy, (double) dxldy, (double) dxrdy, (double) (dy*dxldy), (double) (dy*dxrdy),
				(double) xl, (double) xr, (double) (xl + dy*dxldy), (double) (xr + dy*dxrdy));
	xl += dy*dxldy;
	xr += dy*dxrdy;
	ul += dy*duldy;
	ur += dy*durdy;
	vl += dy*dvldy;
	vr += dy*dvrdy;
	wl += dy*dwldy;
	wr += dy*dwrdy;
	for (idx = 0; idx < 4; idx++) {
		bl[idx]   += dy * dbldy[idx];
		br[idx]   += dy * dbrdy[idx];
		offl[idx] += dy * doldy[idx];
		offr[idx] += dy * dordy[idx];
	}

	while(yy0 < yy1) {
		render_hline<sample_fn, group_no>(bitmap, ti, yy0, xl, xr, ul, ur, vl, vr, wl, wr, bl, br, offl, offr);

		xl += dxldy;
		xr += dxrdy;
		ul += duldy;
		ur += durdy;
		vl += dvldy;
		vr += dvrdy;
		wl += dwldy;
		wr += dwrdy;
		for (idx = 0; idx < 4; idx++) {
			bl[idx] += dbldy[idx];
			br[idx] += dbrdy[idx];
			offl[idx] += doldy[idx];
			offr[idx] += dordy[idx];
		}

		yy0 ++;
	}
}

void powervr2_device::sort_vertices(const vert *v, int *i0, int *i1, int *i2)
{
	float miny, maxy;
	int imin, imax, imid;
	miny = maxy = v[0].y;
	imin = imax = 0;

	if(miny > v[1].y) {
		miny = v[1].y;
		imin = 1;
	} else if(maxy < v[1].y) {
		maxy = v[1].y;
		imax = 1;
	}

	if(miny > v[2].y) {
		miny = v[2].y;
		imin = 2;
	} else if(maxy < v[2].y) {
		maxy = v[2].y;
		imax = 2;
	}

	imid = (imin == 0 || imax == 0) ? (imin == 1 || imax == 1) ? 2 : 1 : 0;

	*i0 = imin;
	*i1 = imid;
	*i2 = imax;
}


template <powervr2_device::pix_sample_fn sample_fn, int group_no>
inline void powervr2_device::render_tri_sorted(bitmap_rgb32 &bitmap, texinfo *ti, const vert *v0, const vert *v1, const vert *v2)
{
	float dy01, dy02, dy12;

	float dx01dy, dx02dy, dx12dy, du01dy, du02dy, du12dy, dv01dy, dv02dy, dv12dy, dw01dy, dw02dy, dw12dy;

	if(v0->y >= 480 || v2->y < 0)
		return;

	float db01[4] = {
		v1->b[0] - v0->b[0],
		v1->b[1] - v0->b[1],
		v1->b[2] - v0->b[2],
		v1->b[3] - v0->b[3]
	};

	float db02[4] = {
		v2->b[0] - v0->b[0],
		v2->b[1] - v0->b[1],
		v2->b[2] - v0->b[2],
		v2->b[3] - v0->b[3]
	};

	float db12[4] = {
		v2->b[0] - v1->b[0],
		v2->b[1] - v1->b[1],
		v2->b[2] - v1->b[2],
		v2->b[3] - v1->b[3]
	};

	float do01[4] = {
		v1->o[0] - v0->o[0],
		v1->o[1] - v0->o[1],
		v1->o[2] - v0->o[2],
		v1->o[3] - v0->o[3]
	};

	float do02[4] = {
		v2->o[0] - v0->o[0],
		v2->o[1] - v0->o[1],
		v2->o[2] - v0->o[2],
		v2->o[3] - v0->o[3]
	};

	float do12[4] = {
		v2->o[0] - v1->o[0],
		v2->o[1] - v1->o[1],
		v2->o[2] - v1->o[2],
		v2->o[3] - v1->o[3]
	};

	dy01 = v1->y - v0->y;
	dy02 = v2->y - v0->y;
	dy12 = v2->y - v1->y;

	float db01dy[4] = {
		dy01 ? db01[0]/dy01 : 0,
		dy01 ? db01[1]/dy01 : 0,
		dy01 ? db01[2]/dy01 : 0,
		dy01 ? db01[3]/dy01 : 0
	};

	float db02dy[4] = {
		dy01 ? db02[0]/dy02 : 0,
		dy01 ? db02[1]/dy02 : 0,
		dy01 ? db02[2]/dy02 : 0,
		dy01 ? db02[3]/dy02 : 0
	};

	float db12dy[4] = {
		dy01 ? db12[0]/dy12 : 0,
		dy01 ? db12[1]/dy12 : 0,
		dy01 ? db12[2]/dy12 : 0,
		dy01 ? db12[3]/dy12 : 0
	};

	float do01dy[4] = {
		dy01 ? do01[0]/dy01 : 0,
		dy01 ? do01[1]/dy01 : 0,
		dy01 ? do01[2]/dy01 : 0,
		dy01 ? do01[3]/dy01 : 0
	};

	float do02dy[4] = {
		dy01 ? do02[0]/dy02 : 0,
		dy01 ? do02[1]/dy02 : 0,
		dy01 ? do02[2]/dy02 : 0,
		dy01 ? do02[3]/dy02 : 0
	};

	float do12dy[4] = {
		dy01 ? do12[0]/dy12 : 0,
		dy01 ? do12[1]/dy12 : 0,
		dy01 ? do12[2]/dy12 : 0,
		dy01 ? do12[3]/dy12 : 0
	};

	dx01dy = dy01 ? (v1->x-v0->x)/dy01 : 0;
	dx02dy = dy02 ? (v2->x-v0->x)/dy02 : 0;
	dx12dy = dy12 ? (v2->x-v1->x)/dy12 : 0;

	du01dy = dy01 ? (v1->u-v0->u)/dy01 : 0;
	du02dy = dy02 ? (v2->u-v0->u)/dy02 : 0;
	du12dy = dy12 ? (v2->u-v1->u)/dy12 : 0;

	dv01dy = dy01 ? (v1->v-v0->v)/dy01 : 0;
	dv02dy = dy02 ? (v2->v-v0->v)/dy02 : 0;
	dv12dy = dy12 ? (v2->v-v1->v)/dy12 : 0;

	dw01dy = dy01 ? (v1->w-v0->w)/dy01 : 0;
	dw02dy = dy02 ? (v2->w-v0->w)/dy02 : 0;
	dw12dy = dy12 ? (v2->w-v1->w)/dy12 : 0;

	if(!dy01) {
		if(!dy12)
			return;

		if(v1->x > v0->x)
			render_span<sample_fn, group_no>(bitmap, ti, v1->y, v2->y, v0->x, v1->x, v0->u, v1->u, v0->v, v1->v, v0->w, v1->w, v0->b, v1->b, v0->o, v1->o, dx02dy, dx12dy, du02dy, du12dy, dv02dy, dv12dy, dw02dy, dw12dy, db02dy, db12dy, do02dy, do12dy);
		else
			render_span<sample_fn, group_no>(bitmap, ti, v1->y, v2->y, v1->x, v0->x, v1->u, v0->u, v1->v, v0->v, v1->w, v0->w, v1->b, v0->b, v1->o, v0->o, dx12dy, dx02dy, du12dy, du02dy, dv12dy, dv02dy, dw12dy, dw02dy, db12dy, db02dy, do12dy, do02dy);

	} else if(!dy12) {
		if(v2->x > v1->x)
			render_span<sample_fn, group_no>(bitmap, ti, v0->y, v1->y, v0->x, v0->x, v0->u, v0->u, v0->v, v0->v, v0->w, v0->w, v0->b, v0->b, v0->o, v0->o, dx01dy, dx02dy, du01dy, du02dy, dv01dy, dv02dy, dw01dy, dw02dy, db01dy, db02dy, do01dy, do02dy);
		else
			render_span<sample_fn, group_no>(bitmap, ti, v0->y, v1->y, v0->x, v0->x, v0->u, v0->u, v0->v, v0->v, v0->w, v0->w, v0->b, v0->b, v0->o, v0->o, dx02dy, dx01dy, du02dy, du01dy, dv02dy, dv01dy, dw02dy, dw01dy, db02dy, db01dy, do02dy, do01dy);

	} else {
			float idk_b[4] = {
				v0->b[0] + db02dy[0] * dy01,
				v0->b[1] + db02dy[1] * dy01,
				v0->b[2] + db02dy[2] * dy01,
				v0->b[3] + db02dy[3] * dy01
			};
			float idk_o[4] = {
				v0->o[0] + do02dy[0] * dy01,
				v0->o[1] + do02dy[1] * dy01,
				v0->o[2] + do02dy[2] * dy01,
				v0->o[3] + do02dy[3] * dy01
			};
		if(dx01dy < dx02dy) {
			render_span<sample_fn, group_no>(bitmap, ti, v0->y, v1->y,
						v0->x, v0->x, v0->u, v0->u, v0->v, v0->v, v0->w, v0->w, v0->b, v0->b, v0->o, v0->o,
						dx01dy, dx02dy, du01dy, du02dy, dv01dy, dv02dy, dw01dy, dw02dy, db01dy, db02dy, do01dy, do02dy);
			render_span<sample_fn, group_no>(bitmap, ti, v1->y, v2->y,
						v1->x, v0->x + dx02dy*dy01, v1->u, v0->u + du02dy*dy01, v1->v, v0->v + dv02dy*dy01, v1->w, v0->w + dw02dy*dy01, v1->b, idk_b, v1->o, idk_o,
						dx12dy, dx02dy, du12dy, du02dy, dv12dy, dv02dy, dw12dy, dw02dy, db12dy, db02dy, do12dy, do02dy);
		} else {
			render_span<sample_fn, group_no>(bitmap, ti, v0->y, v1->y,
						v0->x, v0->x, v0->u, v0->u, v0->v, v0->v, v0->w, v0->w, v0->b, v0->b, v0->o, v0->o,
						dx02dy, dx01dy, du02dy, du01dy, dv02dy, dv01dy, dw02dy, dw01dy, db02dy, db01dy, do02dy, do01dy);
			render_span<sample_fn, group_no>(bitmap, ti, v1->y, v2->y,
						v0->x + dx02dy*dy01, v1->x, v0->u + du02dy*dy01, v1->u, v0->v + dv02dy*dy01, v1->v, v0->w + dw02dy*dy01, v1->w, idk_b, v1->b, idk_o, v1->o,
						dx02dy, dx12dy, du02dy, du12dy, dv02dy, dv12dy, dw02dy, dw12dy, db02dy, db12dy, do02dy, do12dy);
		}
	}
}

template <int group_no>
void powervr2_device::render_tri(bitmap_rgb32 &bitmap, texinfo *ti, const vert *v)
{
	int i0, i1, i2;

	sort_vertices(v, &i0, &i1, &i2);

	bool textured = ti->textured;
	if (textured) {
		bool bilinear = (debug_dip_status & 1) &&
			(ti->filter_mode >= TEX_FILTER_BILINEAR);
		if (bilinear) {
			switch (ti->tsinstruction) {
			case 0:
				render_tri_sorted<&powervr2_device::sample_textured<0,true>, group_no>(bitmap, ti, v+i0, v+i1, v+i2);
				break;
			case 1:
				render_tri_sorted<&powervr2_device::sample_textured<1,true>, group_no>(bitmap, ti, v+i0, v+i1, v+i2);
				break;
			case 2:
				render_tri_sorted<&powervr2_device::sample_textured<2,true>, group_no>(bitmap, ti, v+i0, v+i1, v+i2);
				break;
			case 3:
				render_tri_sorted<&powervr2_device::sample_textured<3,true>, group_no>(bitmap, ti, v+i0, v+i1, v+i2);
				break;
			default:
				/*
				 * This should be impossible because tsinstruction was previously
				 * AND'd with 3
				 */
				logerror("%s - tsinstruction is 0x%08x\n", (unsigned)ti->tsinstruction);
				render_tri_sorted<&powervr2_device::sample_nontextured, group_no>(bitmap, ti, v+i0, v+i1, v+i2);
			}
		} else {
			switch (ti->tsinstruction) {
			case 0:
				render_tri_sorted<&powervr2_device::sample_textured<0,false>, group_no>(bitmap, ti, v+i0, v+i1, v+i2);
				break;
			case 1:
				render_tri_sorted<&powervr2_device::sample_textured<1,false>, group_no>(bitmap, ti, v+i0, v+i1, v+i2);
				break;
			case 2:
				render_tri_sorted<&powervr2_device::sample_textured<2,false>, group_no>(bitmap, ti, v+i0, v+i1, v+i2);
				break;
			case 3:
				render_tri_sorted<&powervr2_device::sample_textured<3,false>, group_no>(bitmap, ti, v+i0, v+i1, v+i2);
				break;
			default:
				/*
				 * This should be impossible because tsinstruction was previously
				 * AND'd with 3
				 */
				logerror("%s - tsinstruction is 0x%08x\n", (unsigned)ti->tsinstruction);
				render_tri_sorted<&powervr2_device::sample_nontextured, group_no>(bitmap, ti, v+i0, v+i1, v+i2);
			}
		}
	} else {
			render_tri_sorted<&powervr2_device::sample_nontextured, group_no>(bitmap, ti, v+i0, v+i1, v+i2);
	}
}

template <int group_no>
void powervr2_device::render_group_to_accumulation_buffer(bitmap_rgb32 &bitmap,const rectangle &cliprect)
{
#if 0
	int stride;
	uint16_t *bmpaddr16;
	uint32_t k;
#endif


	if (renderselect < 0)
		return;

	//printf("drawtest!\n");

	int rs=renderselect;

	struct poly_group *grp = grab[rs].groups + group_no;

	int ns=grp->strips_size;

	for (int cs=0;cs < ns;cs++)
	{
		strip *ts = &grp->strips[cs];
		int sv = ts->svert;
		int ev = ts->evert;
		int i;
		if(ev == -1)
			continue;

		for(i=sv; i <= ev; i++)
		{
			vert *tv = grab[rs].verts + i;
			tv->u = tv->u * ts->ti.sizex * tv->w;
			tv->v = tv->v * ts->ti.sizey * tv->w;
		}

		for(i=sv; i <= ev-2; i++)
		{
			if (!(debug_dip_status&0x2))
				render_tri<group_no>(bitmap, &ts->ti, grab[rs].verts + i);

		}
	}
}

void powervr2_device::render_to_accumulation_buffer(bitmap_rgb32 &bitmap, const rectangle &cliprect) {
	if (renderselect < 0)
		return;

	memset(wbuffer, 0x00, sizeof(wbuffer));

	dc_state *state = machine().driver_data<dc_state>();
	address_space &space = state->m_maincpu->space(AS_PROGRAM);

	// TODO: read ISP/TSP command from isp_background_t instead of assuming Gourad-shaded
	// full-screen polygon.
	uint32_t c=space.read_dword(0x05000000+(param_base&0xf00000)+((isp_backgnd_t&0xfffff8)>>1)+(3+3)*4);
	bitmap.fill(c, cliprect);

	// TODO: modifier volumes
	render_group_to_accumulation_buffer<DISPLAY_LIST_OPAQUE>(bitmap, cliprect);
	render_group_to_accumulation_buffer<DISPLAY_LIST_TRANS>(bitmap, cliprect);
	render_group_to_accumulation_buffer<DISPLAY_LIST_PUNCH_THROUGH>(bitmap, cliprect);

	grab[renderselect].busy=0;
}

// copies the accumulation buffer into the framebuffer, converting to the specified format
// not accurate, ignores field stuff and just uses SOF1 for now
// also ignores scale effects (can scale accumulation buffer to half size with filtering etc.)
// also can specify dither etc.
// basically, just a crude implementation!

/* 0555KRGB = 0 */
void powervr2_device::fb_convert_0555krgb_to_555rgb(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint16_t newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000f800) >> 11)) << 5)  |
							((((data & 0x00f80000) >> 19)) << 10);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_0555krgb_to_565rgb(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint16_t newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000f800) >> 11)) << 5)  |
							((((data & 0x00f80000) >> 19)) << 11);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_0555krgb_to_888rgb24(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*3);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint32_t newdat = (data & 0xf8f8f8);

			space.write_byte(realwriteoffs+xcnt*3+0, newdat >> 0);
			space.write_byte(realwriteoffs+xcnt*3+1, newdat >> 8);
			space.write_byte(realwriteoffs+xcnt*3+2, newdat >> 16);
		}
	}
}

void powervr2_device::fb_convert_0555krgb_to_888rgb32(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*4);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint32_t newdat = (data & 0xf8f8f8);

			space.write_dword(realwriteoffs+xcnt*4, newdat);
		}
	}
}

/* 0565RGB = 1 */
void powervr2_device::fb_convert_0565rgb_to_555rgb(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint16_t newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000fc00) >> 10)) << 5)  |
							((((data & 0x00f80000) >> 19)) << 10);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_0565rgb_to_565rgb(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint16_t newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000fc00) >> 10)) << 5)  |
							((((data & 0x00f80000) >> 19)) << 11);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_0565rgb_to_888rgb24(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*3);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint32_t newdat = (data & 0xf8fcf8);

			space.write_byte(realwriteoffs+xcnt*3+0, newdat >> 0);
			space.write_byte(realwriteoffs+xcnt*3+1, newdat >> 8);
			space.write_byte(realwriteoffs+xcnt*3+2, newdat >> 16);
		}
	}
}

void powervr2_device::fb_convert_0565rgb_to_888rgb32(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*4);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint32_t newdat = (data & 0xf8fcf8);

			space.write_dword(realwriteoffs+xcnt*4, newdat);
		}
	}
}

/* 1555ARGB = 3 */
void powervr2_device::fb_convert_1555argb_to_555rgb(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint16_t newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000f800) >> 11)) << 5)  |
							((((data & 0x00f80000) >> 19)) << 10);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_1555argb_to_565rgb(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint16_t newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000f800) >> 11)) << 5)  |
							((((data & 0x00f80000) >> 19)) << 11);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_1555argb_to_888rgb24(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*3);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint32_t newdat = (data & 0xf8f8f8);

			space.write_byte(realwriteoffs+xcnt*3+0, newdat >> 0);
			space.write_byte(realwriteoffs+xcnt*3+1, newdat >> 8);
			space.write_byte(realwriteoffs+xcnt*3+2, newdat >> 16);
		}
	}
}

void powervr2_device::fb_convert_1555argb_to_888rgb32(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*4);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint32_t newdat = (data & 0xf8f8f8);

			space.write_dword(realwriteoffs+xcnt*4, newdat);
		}
	}
}

/* 888RGB = 4 */
void powervr2_device::fb_convert_888rgb_to_555rgb(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint16_t newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000f800) >> 11)) << 5)  |
							((((data & 0x00f80000) >> 16)) << 10);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_888rgb_to_565rgb(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint16_t newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000fc00) >> 11)) << 5)  |
							((((data & 0x00f80000) >> 16)) << 11);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_888rgb_to_888rgb24(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*3);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint32_t newdat = (data & 0xffffff);

			space.write_byte(realwriteoffs+xcnt*3+0, newdat >> 0);
			space.write_byte(realwriteoffs+xcnt*3+1, newdat >> 8);
			space.write_byte(realwriteoffs+xcnt*3+2, newdat >> 16);
		}
	}
}

void powervr2_device::fb_convert_888rgb_to_888rgb32(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*4);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint32_t newdat = (data & 0xffffff);

			space.write_dword(realwriteoffs+xcnt*4, newdat);
		}
	}
}


/* 8888ARGB = 6 */
void powervr2_device::fb_convert_8888argb_to_555rgb(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint16_t newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000f800) >> 11)) << 5)  |
							((((data & 0x00f80000) >> 16)) << 10);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_8888argb_to_565rgb(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint16_t newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000fc00) >> 11)) << 5)  |
							((((data & 0x00f80000) >> 16)) << 11);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_8888argb_to_888rgb24(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*3);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint32_t newdat = (data & 0xffffff);

			space.write_byte(realwriteoffs+xcnt*3+0, newdat >> 0);
			space.write_byte(realwriteoffs+xcnt*3+1, newdat >> 8);
			space.write_byte(realwriteoffs+xcnt*3+2, newdat >> 16);
		}
	}
}

void powervr2_device::fb_convert_8888argb_to_888rgb32(address_space &space, int x,int y)
{
	for (int ycnt=0;ycnt<32;ycnt++)
	{
		uint32_t realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*4);
		uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y+ycnt, x);

		for (int xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			uint32_t data = src[xcnt];
			uint32_t newdat = (data & 0xffffff);

			space.write_dword(realwriteoffs+xcnt*4, newdat);
		}
	}
}


/*

0x0 0555 KRGB 16 bit (default) Bit 15 is the value of fb_kval 7.
0x1 565 RGB 16 bit
0x2 4444 ARGB 16 bit
0x3 1555 ARGB 16 bit The alpha value is determined by comparison with the value of fb_alpha_threshold.
0x4 888 RGB 24 bit packed
0x5 0888 KRGB 32 bit K is the value of fk_kval.
0x6 8888 ARGB 32 bit
0x7 Setting prohibited.

*/

void powervr2_device::pvr_accumulationbuffer_to_framebuffer(address_space &space, int x,int y)
{
	// the accumulation buffer is always 8888
	//
	// the standard format for the framebuffer appears to be 565
	// yes, this means colour data is lost in the conversion

	uint8_t packmode = fb_w_ctrl & 0x7;
	uint8_t unpackmode = (fb_r_ctrl & 0x0000000c) >>2;  // aka fb_depth

//  popmessage("%02x %02x",packmode,unpackmode);

	switch (packmode)
	{
		// used by ringout
		case 0x00: //0555 KRGB
		{
			switch(unpackmode)
			{
				case 0x00: fb_convert_0555krgb_to_555rgb(space,x,y); break;
				case 0x01: fb_convert_0555krgb_to_565rgb(space,x,y); break;
				case 0x02: fb_convert_0555krgb_to_888rgb24(space,x,y); break;
				case 0x03: fb_convert_0555krgb_to_888rgb32(space,x,y); break;
			}
		}
		break;

		// used by cleoftp
		case 0x01: //565 RGB 16 bit
		{
			switch(unpackmode)
			{
				case 0x00: fb_convert_0565rgb_to_555rgb(space,x,y); break;
				case 0x01: fb_convert_0565rgb_to_565rgb(space,x,y); break;
				case 0x02: fb_convert_0565rgb_to_888rgb24(space,x,y); break;
				case 0x03: fb_convert_0565rgb_to_888rgb32(space,x,y); break;
			}
		}
		break;

		case 0x02:
			// TODO: demofist character select
			//printf("pvr_accumulationbuffer_to_framebuffer buffer to tile at %d,%d - unsupported pack mode %02x (4444 ARGB)\n",x,y,packmode);
			break;

		case 0x03: // 1555 ARGB 16 bit
		{
			switch(unpackmode)
			{
				case 0x00: fb_convert_1555argb_to_555rgb(space,x,y); break;
				case 0x01: fb_convert_1555argb_to_565rgb(space,x,y); break;
				case 0x02: fb_convert_1555argb_to_888rgb24(space,x,y); break;
				case 0x03: fb_convert_1555argb_to_888rgb32(space,x,y); break;
			}
		}
		break;

		// used by Suchie3
		case 0x04: // 888 RGB 24-bit
		{
			switch(unpackmode)
			{
				case 0x00: fb_convert_888rgb_to_555rgb(space,x,y); break;
				case 0x01: fb_convert_888rgb_to_565rgb(space,x,y); break;
				case 0x02: fb_convert_888rgb_to_888rgb24(space,x,y); break;
				case 0x03: fb_convert_888rgb_to_888rgb32(space,x,y); break;
			}
		}
		break;

		case 0x05: // 0888 KRGB 32 bit
		{
			switch(unpackmode)
			{
				case 0x00: fb_convert_8888argb_to_555rgb(space,x,y); break;
				case 0x01: fb_convert_8888argb_to_565rgb(space,x,y); break;
				case 0x02: fb_convert_8888argb_to_888rgb24(space,x,y); break;
				case 0x03: fb_convert_8888argb_to_888rgb32(space,x,y); break;
			}
		}
		break;

		case 0x06: // 8888 ARGB 32 bit
		{
			switch(unpackmode)
			{
				case 0x00: fb_convert_8888argb_to_555rgb(space,x,y); break;
				case 0x01: fb_convert_8888argb_to_565rgb(space,x,y); break;
				case 0x02: fb_convert_8888argb_to_888rgb24(space,x,y); break;
				case 0x03: fb_convert_8888argb_to_888rgb32(space,x,y); break;
			}
		}
		break;

		case 0x07:
			throw emu_fatalerror("pvr_accumulationbuffer_to_framebuffer buffer to tile at %d,%d - unsupported pack mode %02x (Reserved! Don't Use!)\n",x,y,packmode);
			break;
	}
}

void powervr2_device::pvr_drawframebuffer(bitmap_rgb32 &bitmap,const rectangle &cliprect)
{
	int dy,xi;
	uint8_t interlace_on = ((spg_control & 0x10) >> 4);
	int32_t ystart_f1 = (vo_starty & 0x3ff) << interlace_on;
	//int32_t ystart_f2 = (vo_starty >> 16) & 0x3ff;
	int32_t hstart = (vo_startx & 0x3ff);
//  rectangle fbclip;

	uint8_t unpackmode = (fb_r_ctrl & 0x0000000c) >>2;  // aka fb_depth
	uint8_t enable = (fb_r_ctrl & 0x00000001);

	// ??
	if (!enable) return;

	// only for rgb565 framebuffer
	xi=((fb_r_size & 0x3ff)+1) << 1;
	dy=((fb_r_size >> 10) & 0x3ff)+1;

	dy++;
	dy*=2; // probably depends on interlace mode, fields etc...

//  popmessage("%d %d %d %d %d",ystart_f1,ystart_f2,interlace_on,spg_vblank & 0x3ff,(spg_vblank >> 16) & 0x3ff);

#if 0
	fbclip.min_x = hstart;
	fbclip.min_y = ystart_f1;
	fbclip.max_x = hstart + (xi << ((vo_control & 0x100) >> 8));
	fbclip.max_y = ystart_f1 + dy;

	popmessage("%d %d %d %d",fbclip.min_x,fbclip.min_y,fbclip.max_x,fbclip.max_y);
#endif

	switch (unpackmode)
	{
		case 0x00: // 0555 RGB 16-bit, Cleo Fortune Plus
			// should upsample back to 8-bit output using fb_concat
			for (int y=0;y <= dy;y++)
			{
				uint32_t addrp = fb_r_sof1+y*xi*2;
				if(vo_control & 0x100)
				{
					for (int x=0;x < xi;x++)
					{
						int res_x = x*2+0 + hstart;
						int res_y = y + ystart_f1;

						uint32_t *fbaddr=&bitmap.pix(res_y, res_x);
						uint32_t c=*((reinterpret_cast<uint16_t *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 1));

						uint32_t b = (c & 0x001f) << 3;
						uint32_t g = (c & 0x03e0) >> 2;
						uint32_t r = (c & 0x7c00) >> 7;

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						res_x = x*2+1 + hstart;

						fbaddr=&bitmap.pix(res_y, res_x);
						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);
						addrp+=2;
					}
				}
				else
				{
					for (int x=0;x < xi;x++)
					{
						int res_x = x + hstart;
						int res_y = y + ystart_f1;

						uint32_t *fbaddr=&bitmap.pix(res_y, res_x);
						uint32_t c=*((reinterpret_cast<uint16_t *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 1));

						uint32_t b = (c & 0x001f) << 3;
						uint32_t g = (c & 0x03e0) >> 2;
						uint32_t r = (c & 0x7c00) >> 7;

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);
						addrp+=2;
					}
				}
			}
			break;

		case 0x01: // 0565 RGB 16-bit
			// should upsample back to 8-bit output using fb_concat
			for (int y=0;y <= dy;y++)
			{
				uint32_t addrp = fb_r_sof1+y*xi*2;
				if(vo_control & 0x100)
				{
					for (int x=0;x < xi;x++)
					{
						int res_x = x*2+0 + hstart;
						int res_y = y + ystart_f1;
						uint32_t *fbaddr=&bitmap.pix(res_y, res_x);
						uint32_t c=*((reinterpret_cast<uint16_t *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 1));

						uint32_t b = (c & 0x001f) << 3;
						uint32_t g = (c & 0x07e0) >> 3;
						uint32_t r = (c & 0xf800) >> 8;

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						res_x = x*2+1 + hstart;
						//res_y = y + ystart_f1;
						fbaddr=&bitmap.pix(res_y, res_x);

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						addrp+=2;
					}
				}
				else
				{
					for (int x=0;x < xi;x++)
					{
						int res_x = x + hstart;
						int res_y = y + ystart_f1;
						uint32_t *fbaddr=&bitmap.pix(res_y, res_x);
						uint32_t c=*((reinterpret_cast<uint16_t *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 1));

						uint32_t b = (c & 0x001f) << 3;
						uint32_t g = (c & 0x07e0) >> 3;
						uint32_t r = (c & 0xf800) >> 8;

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);
						addrp+=2;
					}
				}
			}
			break;

		case 0x02: ; // 888 RGB 24-bit - suchie3, Soul Calibur
			for (int y=0;y <= dy;y++)
			{
				uint32_t addrp = fb_r_sof1+y*xi*2;

				if(vo_control & 0x100)
				{
					for (int x=0;x < xi;x++)
					{
						int res_x = x*2+0 + hstart;
						int res_y = y + ystart_f1;

						uint32_t *fbaddr=&bitmap.pix(res_y, res_x);

						uint32_t b =*(uint8_t *)((reinterpret_cast<uint8_t *>(dc_framebuffer_ram)) + BYTE8_XOR_LE(addrp));

						uint32_t g =*(uint8_t *)((reinterpret_cast<uint8_t *>(dc_framebuffer_ram)) + BYTE8_XOR_LE(addrp+1));

						uint32_t r =*(uint8_t *)((reinterpret_cast<uint8_t *>(dc_framebuffer_ram)) + BYTE8_XOR_LE(addrp+2));

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						res_x = x*2+1 + hstart;

						fbaddr=&bitmap.pix(res_y, res_x);
						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						addrp+=3;
					}
				}
				else
				{
					for (int x=0;x < xi;x++)
					{
						int res_x = x + hstart;
						int res_y = y + ystart_f1;
						uint32_t *fbaddr=&bitmap.pix(res_y, res_x);

						uint32_t b =*(uint8_t *)((reinterpret_cast<uint8_t *>(dc_framebuffer_ram)) + BYTE8_XOR_LE(addrp));

						uint32_t g =*(uint8_t *)((reinterpret_cast<uint8_t *>(dc_framebuffer_ram)) + BYTE8_XOR_LE(addrp+1));

						uint32_t r =*(uint8_t *)((reinterpret_cast<uint8_t *>(dc_framebuffer_ram)) + BYTE8_XOR_LE(addrp+2));

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						addrp+=3;
					}
				}
			}
			break;

		case 0x03:        // 0888 ARGB 32-bit
			for (int y=0;y <= dy;y++)
			{
				uint32_t addrp = fb_r_sof1+y*xi*2;

				if(vo_control & 0x100)
				{
					for (int x=0;x < xi;x++)
					{
						int res_x = x*2+0 + hstart;
						int res_y = y + ystart_f1;

						uint32_t *fbaddr=&bitmap.pix(res_y, res_x);
						uint32_t c =*((reinterpret_cast<uint32_t *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 2));

						uint32_t b = (c & 0x0000ff) >> 0;
						uint32_t g = (c & 0x00ff00) >> 8;
						uint32_t r = (c & 0xff0000) >> 16;

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						res_x = x*2+1 + hstart;

						fbaddr=&bitmap.pix(res_y, res_x);
						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						addrp+=4;
					}
				}
				else
				{
					for (int x=0;x < xi;x++)
					{
						int res_x = x + hstart;
						int res_y = y + ystart_f1;
						uint32_t *fbaddr=&bitmap.pix(res_y, res_x);
						uint32_t c =*((reinterpret_cast<uint32_t *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 2));

						uint32_t b = (c & 0x0000ff) >> 0;
						uint32_t g = (c & 0x00ff00) >> 8;
						uint32_t r = (c & 0xff0000) >> 16;

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						addrp+=4;
					}
				}
			}
			break;
	}
}

/* test video end */

void powervr2_device::pvr_build_parameterconfig()
{
	int a,b,c,d,e,p;

	for (a = 0;a <= 63;a++)
		pvr_parameterconfig[a] = -1;
	p=0;
	// volume,col_type,texture,offset,16bit_uv
	for (a = 0;a <= 1;a++)
		for (b = 0;b <= 3;b++)
			for (c = 0;c <= 1;c++)
				if (c == 0)
				{
					for (d = 0;d <= 1;d++)
						for (e = 0;e <= 1;e++)
							pvr_parameterconfig[(a << 6) | (b << 4) | (c << 3) | (d << 2) | (e << 0)] = pvr_parconfseq[p];
					p++;
				}
				else
					for (d = 0;d <= 1;d++)
						for (e = 0;e <= 1;e++)
						{
							pvr_parameterconfig[(a << 6) | (b << 4) | (c << 3) | (d << 2) | (e << 0)] = pvr_parconfseq[p];
							p++;
						}
	for (a = 1;a <= 63;a++)
		if (pvr_parameterconfig[a] < 0)
			pvr_parameterconfig[a] = pvr_parameterconfig[a-1];
}

// TODO: these two are currently unused
TIMER_CALLBACK_MEMBER(powervr2_device::vbin)
{
	LOGIRQ("[%d] VBL IN %d\n", screen().frame_number(), screen().vpos());
	LOGIRQ("    VII %d VOI %d VI %d VO %d VS %d\n",
		spg_vblank_int & 0x3ff, (spg_vblank_int >> 16) & 0x3ff,
		spg_vblank & 0x3ff, (spg_vblank >> 16) & 0x3ff, (spg_load >> 16) & 0x3ff
	);

	irq_cb(VBL_IN_IRQ);

//  vbin_timer->adjust(screen().time_until_pos(spg_vblank_int & 0x3ff));
}

TIMER_CALLBACK_MEMBER(powervr2_device::vbout)
{
	LOGIRQ("[%d] VBL OUT %d\n", screen().frame_number(), screen().vpos());

	irq_cb(VBL_OUT_IRQ);

//  vbout_timer->adjust(screen().time_until_pos((spg_vblank_int >> 16) & 0x3ff));
}

TIMER_CALLBACK_MEMBER(powervr2_device::hbin)
{
	if(spg_hblank_int & 0x1000)
	{
		if(scanline == next_y)
		{
			LOGIRQ("[%d] HBL IN %d - (%08x)\n",
				screen().frame_number(), screen().vpos(), scanline, spg_hblank_int
			);
			irq_cb(HBL_IN_IRQ);
			next_y += spg_hblank_int & 0x3ff;
		}
	}
	else if((scanline == (spg_hblank_int & 0x3ff)) || (spg_hblank_int & 0x2000))
	{
		LOGIRQ("[%d] HBL IN %d - (%08x)\n",
			screen().frame_number(), screen().vpos(), scanline, spg_hblank_int
		);
		irq_cb(HBL_IN_IRQ);
	}

	scanline++;

	if(scanline >= ((spg_load >> 16) & 0x3ff))
	{
		scanline = 0;
		next_y = spg_hblank_int & 0x3ff;
	}

	hbin_timer->adjust(screen().time_until_pos(scanline, ((spg_hblank_int >> 16) & 0x3ff)-1));
}

// TODO: these two aren't really called atm
TIMER_CALLBACK_MEMBER(powervr2_device::endofrender_video)
{
	LOGIRQ("[%d] VIDEO END %d\n", screen().frame_number(), screen().vpos());
//  endofrender_timer_video->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(powervr2_device::endofrender_tsp)
{
	LOGIRQ("[%d] TSP END %d\n", screen().frame_number(), screen().vpos());

//  endofrender_timer_tsp->adjust(attotime::never);
//  endofrender_timer_video->adjust(attotime::from_usec(500) );
}

TIMER_CALLBACK_MEMBER(powervr2_device::endofrender_isp)
{
	irq_cb(EOR_ISP_IRQ); // ISP end of render
	irq_cb(EOR_TSP_IRQ); // TSP end of render
	irq_cb(EOR_VIDEO_IRQ); // VIDEO end of render

	LOGIRQ("[%d] ISP END %d\n", screen().frame_number(), screen().vpos());

	endofrender_timer_isp->adjust(attotime::never);
//  endofrender_timer_tsp->adjust(attotime::from_usec(500) );
}

uint32_t powervr2_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/******************
	  MAME note
	*******************

	The video update function should NOT be generating interrupts, setting timers or doing _anything_ the game might be able to detect
	as it will be called at different times depending on frameskip etc.

	Rendering should happen when the hardware requests it, to the framebuffer(s)

	Everything else should depend on timers.

	******************/

//  static int useframebuffer=1;
//  const rectangle &visarea = screen.visible_area();

	// copy our fake framebuffer bitmap (where things have been rendered) to the screen
#if 0
	for (int y = visarea->min_y ; y <= visarea->max_y ; y++)
	{
		for (int x = visarea->min_x ; x <= visarea->max_x ; x++)
		{
			uint32_t const *const src = &fake_accumulationbuffer_bitmap->pix(y, x);
			uint32_t *const dst = &bitmap.pix(y, x);
			dst[0] = src[0];
		}
	}
#endif

	//FIXME: additional Chroma bit
	bitmap.fill(rgb_t(0xff, (vo_border_col >> 16) & 0xff,
							(vo_border_col >> 8 ) & 0xff,
							(vo_border_col      ) & 0xff), cliprect);

	if(!(vo_control & 8))
		pvr_drawframebuffer(bitmap, cliprect);

	// update this here so we only do string lookup once per frame
	debug_dip_status = m_mamedebug->read();

	return 0;
}

TIMER_CALLBACK_MEMBER(powervr2_device::pvr_dma_irq)
{
	LOGIRQ("[%d] PVR DMA %d\n", screen().frame_number(), screen().vpos());

	m_pvr_dma.start = sb_pdst = 0;
	irq_cb(DMA_PVR_IRQ);
}

/* used so far by usagiym and sprtjam */
// TODO: very inaccurate
void powervr2_device::pvr_dma_execute(address_space &space)
{
	dc_state *state = machine().driver_data<dc_state>();
	uint32_t src,dst,size;
	dst = m_pvr_dma.pvr_addr;
	src = m_pvr_dma.sys_addr;
	size = 0;

	LOGPVRDMA("PVR-DMA start\n");
	LOGPVRDMA("%08x %08x %08x\n",m_pvr_dma.pvr_addr, m_pvr_dma.sys_addr, m_pvr_dma.size);
	LOGPVRDMA("src %s dst %08x\n",m_pvr_dma.dir ? "->" : "<-",m_pvr_dma.sel);

	/* Throw illegal address set */
	// TODO: unimplemented for now, checkable from DCLP
#if 0
	if((m_pvr_dma.sys_addr & 0x1c000000) != 0x0c000000)
	{
		irq_cb(ERR_PVRIF_ILL_ADDR_IRQ);
		m_pvr_dma.start = sb_pdst = 0;
		printf("Illegal PVR DMA set\n");
		return;
	}
#endif

	/* 0 rounding size = 16 Mbytes */
	if(m_pvr_dma.size == 0) { m_pvr_dma.size = 0x100000; }

	if(m_pvr_dma.dir == 0)
	{
		for(;size < m_pvr_dma.size; size+=4)
		{
			space.write_dword(dst,space.read_dword(src));
			src+=4;
			dst+=4;
		}
	}
	else
	{
		for(;size < m_pvr_dma.size; size+=4)
		{
			space.write_dword(src,space.read_dword(dst));
			src+=4;
			dst+=4;
		}
	}

	/* Note: do not update the params, since this DMA type doesn't support it. */
	// TODO: accurate timing
	dma_irq_timer->adjust(state->m_maincpu->cycles_to_attotime(m_pvr_dma.size/4));
}

INPUT_PORTS_START( powervr2 )
	PORT_START("PVR_DEBUG")
	PORT_CONFNAME( 0x01, 0x00, "Bilinear Filtering" )
	PORT_CONFSETTING(    0x00, DEF_STR( No ) )
	PORT_CONFSETTING(    0x01, DEF_STR( Yes ) )
	PORT_CONFNAME( 0x02, 0x00, "Disable Render Calls" )
	PORT_CONFSETTING(    0x00, DEF_STR( No ) )
	PORT_CONFSETTING(    0x02, DEF_STR( Yes ) )
INPUT_PORTS_END

ioport_constructor powervr2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( powervr2 );
}

powervr2_device::powervr2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, POWERVR2, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, irq_cb(*this)
	, m_mamedebug(*this, "PVR_DEBUG")
{
}

void powervr2_device::device_start()
{
	grab = std::make_unique<receiveddata[]>(NUM_BUFFERS);

	pvr_build_parameterconfig();

	computedilated();

//  vbout_timer = timer_alloc(FUNC(powervr2_device::vbout), this);
//  vbin_timer = timer_alloc(FUNC(powervr2_device::vbin), this);
	hbin_timer = timer_alloc(FUNC(powervr2_device::hbin), this);
	yuv_timer_end = timer_alloc(FUNC(powervr2_device::yuv_convert_end), this);

	endofrender_timer_isp = timer_alloc(FUNC(powervr2_device::endofrender_isp), this);
	endofrender_timer_tsp = timer_alloc(FUNC(powervr2_device::endofrender_tsp), this);
	endofrender_timer_video = timer_alloc(FUNC(powervr2_device::endofrender_video), this);

	opaque_irq_timer = timer_alloc(FUNC(powervr2_device::transfer_opaque_list_irq), this);
	opaque_modifier_volume_irq_timer = timer_alloc(FUNC(powervr2_device::transfer_opaque_modifier_volume_list_irq), this);
	translucent_irq_timer = timer_alloc(FUNC(powervr2_device::transfer_translucent_list_irq), this);
	translucent_modifier_volume_irq_timer = timer_alloc(FUNC(powervr2_device::transfer_translucent_modifier_volume_list_irq), this);
	punch_through_irq_timer = timer_alloc(FUNC(powervr2_device::transfer_punch_through_list_irq), this);
	dma_irq_timer = timer_alloc(FUNC(powervr2_device::pvr_dma_irq), this);

	fake_accumulationbuffer_bitmap = std::make_unique<bitmap_rgb32>(2048,2048);

	softreset = 0;
	param_base = 0;
	region_base = 0;
	vo_border_col = 0;
	fb_r_ctrl = 0;
	fb_w_ctrl = 0;
	fb_w_linestride = 0;
	fb_r_sof1 = 0;
	fb_r_sof2 = 0;
	fb_r_size = 0;
	fb_w_sof1 = 0;
	fb_w_sof2 = 0;
	fb_x_clip = 0;
	fb_y_clip = 0;
	fpu_param_cfg = 0;
	isp_backgnd_t = 0;
	spg_hblank_int = 0;
	spg_vblank_int = 0;
	spg_hblank = 0;
	spg_load = 0;
	spg_vblank = 0;
	spg_width = 0;
	spg_control = 0;
	vo_control = 0;
	vo_startx = 0;
	vo_starty = 0;
	text_control = 0;
	pal_ram_ctrl = 0;
	ta_ol_base = 0;
	ta_ol_limit = 0;
	ta_isp_base = 0;
	ta_isp_limit = 0;
	ta_next_opb = 0;
	ta_itp_current = 0;
	ta_alloc_ctrl = 0;
	ta_next_opb_init = 0;
	ta_yuv_tex_base = 0;
	ta_yuv_tex_ctrl = 0;
	ta_yuv_tex_cnt = 0;
	memset(fog_table, 0, sizeof(fog_table));
	memset(palette, 0, sizeof(palette));
	memset(&m_pvr_dma, 0x00, sizeof(m_pvr_dma));

	sb_pdstap = 0;
	sb_pdstar = 0;
	sb_pdlen = 0;
	sb_pddir = 0;
	sb_pdtsel = 0;
	sb_pden = 0;
	sb_pdst = 0;
	sb_pdapro = 0;

	save_item(NAME(softreset));
	save_item(NAME(param_base));
	save_item(NAME(region_base));
	save_item(NAME(vo_border_col));
	save_item(NAME(fb_r_ctrl));
	save_item(NAME(fb_w_ctrl));
	save_item(NAME(fb_w_linestride));
	save_item(NAME(fb_r_sof1));
	save_item(NAME(fb_r_sof2));
	save_item(NAME(fb_r_size));
	save_item(NAME(fb_w_sof1));
	save_item(NAME(fb_w_sof2));
	save_item(NAME(fb_x_clip));
	save_item(NAME(fb_y_clip));
	save_item(NAME(fpu_param_cfg));
	save_item(NAME(isp_backgnd_t));
	save_item(NAME(spg_hblank_int));
	save_item(NAME(spg_vblank_int));
	save_item(NAME(spg_hblank));
	save_item(NAME(spg_load));
	save_item(NAME(spg_vblank));
	save_item(NAME(spg_width));
	save_item(NAME(vo_control));
	save_item(NAME(vo_startx));
	save_item(NAME(vo_starty));
	save_item(NAME(text_control));
	save_item(NAME(pal_ram_ctrl));
	save_item(NAME(ta_ol_base));
	save_item(NAME(ta_ol_limit));
	save_item(NAME(ta_isp_base));
	save_item(NAME(ta_isp_limit));
	save_item(NAME(ta_next_opb));
	save_item(NAME(ta_itp_current));
	save_item(NAME(ta_alloc_ctrl));
	save_item(NAME(ta_next_opb_init));
	save_item(NAME(ta_yuv_tex_base));
	save_item(NAME(ta_yuv_tex_ctrl));
	save_item(NAME(ta_yuv_tex_cnt));
	save_pointer(NAME(fog_table), 0x80);
	save_pointer(NAME(palette), 0x400);

	save_item(NAME(sb_pdstap));
	save_item(NAME(sb_pdstar));
	save_item(NAME(sb_pdlen));
	save_item(NAME(sb_pddir));
	save_item(NAME(sb_pdtsel));
	save_item(NAME(sb_pden));
	save_item(NAME(sb_pdst));
	save_item(NAME(sb_pdapro));

	save_item(NAME(m_pvr_dma.pvr_addr));
	save_item(NAME(m_pvr_dma.sys_addr));
	save_item(NAME(m_pvr_dma.size));
	save_item(NAME(m_pvr_dma.sel));
	save_item(NAME(m_pvr_dma.dir));
	save_item(NAME(m_pvr_dma.flag));
	save_item(NAME(m_pvr_dma.start));
	save_item(NAME(debug_dip_status));
	save_pointer(NAME(tafifo_buff),32);
	save_item(NAME(scanline));
	save_item(NAME(next_y));

	m_work_queue = nullptr;
	m_render_request = nullptr;
}

void powervr2_device::device_reset()
{
	softreset =                 0x00000007;
	vo_control =                0x00000108;
	vo_startx =                 0x0000009d;
	vo_starty =                 0x00150015;
	spg_hblank =                0x007e0345;
	spg_load =                  0x01060359;
	spg_vblank =                0x01500104;
	spg_hblank_int =            0x031d0000;
	spg_vblank_int =            0x01500104;

	tafifo_pos = 0;
	tafifo_mask = 7;
	tafifo_vertexwords = 8;
	tafifo_listtype = DISPLAY_LIST_NONE;
	start_render_received = 0;
	renderselect = -1;
	grabsel = 0;

//  vbout_timer->adjust(screen().time_until_pos((spg_vblank_int >> 16) & 0x3ff));
//  vbin_timer->adjust(screen().time_until_pos(spg_vblank_int & 0x3ff));
	hbin_timer->adjust(screen().time_until_pos(0, ((spg_hblank_int >> 16) & 0x3ff)-1));

	scanline = 0;
	next_y = 0;

	endofrender_timer_isp->adjust(attotime::never);
	endofrender_timer_tsp->adjust(attotime::never);
	endofrender_timer_video->adjust(attotime::never);
	yuv_timer_end->adjust(attotime::never);

	dc_state *state = machine().driver_data<dc_state>();
	dc_texture_ram = state->dc_texture_ram.target();
	dc_framebuffer_ram = state->dc_framebuffer_ram.target();

	m_work_queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_HIGH_FREQ);
}

/* called by TIMER_ADD_PERIODIC, in driver sections (controlled by SPG, that's a PVR sub-device) */
void powervr2_device::pvr_scanline_timer(int vpos)
{
	int vbin_line = spg_vblank_int & 0x3ff;
	int vbout_line = (spg_vblank_int >> 16) & 0x3ff;
	uint8_t interlace_on = ((spg_control & 0x10) >> 4);
	dc_state *state = machine().driver_data<dc_state>();

	vbin_line <<= interlace_on;
	vbout_line <<= interlace_on;

	if(vbin_line-(1+interlace_on) == vpos)
		state->m_maple->maple_hw_trigger();

	if(vbin_line == vpos)
	{
		LOGIRQ("[%d] VBL IN %d\n", screen().frame_number(), screen().vpos());
		LOGIRQ("    VII %d VOI %d VI %d VO %d VS %d\n",
			spg_vblank_int & 0x3ff, (spg_vblank_int >> 16) & 0x3ff,
			spg_vblank & 0x3ff, (spg_vblank >> 16) & 0x3ff, (spg_load >> 16) & 0x3ff
		);
		irq_cb(VBL_IN_IRQ);
	}

	if(vbout_line == vpos)
	{
		LOGIRQ("[%d] VBL OUT %d\n", screen().frame_number(), screen().vpos());
		irq_cb(VBL_OUT_IRQ);
	}
}
