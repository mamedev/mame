// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*
  Dreamcast video emulation
*/

#include "emu.h"
#include "powervr2.h"
#include "includes/dc.h"
#include "cpu/sh4/sh4.h"
#include "rendutil.h"
#include "video/rgbutil.h"

const device_type POWERVR2 = &device_creator<powervr2_device>;

DEVICE_ADDRESS_MAP_START(ta_map, 32, powervr2_device)
	AM_RANGE(0x0000, 0x0003) AM_READ(     id_r)
	AM_RANGE(0x0004, 0x0007) AM_READ(     revision_r)
	AM_RANGE(0x0008, 0x000b) AM_READWRITE(softreset_r,        softreset_w)
	AM_RANGE(0x0014, 0x0017) AM_WRITE(    startrender_w)
// 18 = test select
	AM_RANGE(0x0020, 0x0023) AM_READWRITE(param_base_r,       param_base_w)
	AM_RANGE(0x002c, 0x002f) AM_READWRITE(region_base_r,      region_base_w)
// 30 = span sort cfg
	AM_RANGE(0x0040, 0x0043) AM_READWRITE(vo_border_col_r,    vo_border_col_w)
	AM_RANGE(0x0044, 0x0047) AM_READWRITE(fb_r_ctrl_r,        fb_r_ctrl_w)
	AM_RANGE(0x0048, 0x004b) AM_READWRITE(fb_w_ctrl_r,        fb_w_ctrl_w)
	AM_RANGE(0x004c, 0x004f) AM_READWRITE(fb_w_linestride_r,  fb_w_linestride_w)
	AM_RANGE(0x0050, 0x0053) AM_READWRITE(fb_r_sof1_r,        fb_r_sof1_w)
	AM_RANGE(0x0054, 0x0057) AM_READWRITE(fb_r_sof2_r,        fb_r_sof2_w)
	AM_RANGE(0x005c, 0x005f) AM_READWRITE(fb_r_size_r,        fb_r_size_w)
	AM_RANGE(0x0060, 0x0063) AM_READWRITE(fb_w_sof1_r,        fb_w_sof1_w)
	AM_RANGE(0x0064, 0x0067) AM_READWRITE(fb_w_sof2_r,        fb_w_sof2_w)
	AM_RANGE(0x0068, 0x006b) AM_READWRITE(fb_x_clip_r,        fb_x_clip_w)
	AM_RANGE(0x006c, 0x006f) AM_READWRITE(fb_y_clip_r,        fb_y_clip_w)
// 74 = fpu_shad_scale
// 78 = fpu_cull_val
	AM_RANGE(0x007c, 0x007f) AM_READWRITE(fpu_param_cfg_r,    fpu_param_cfg_w)
// 80 = half_offset
// 84 = fpu_perp_val
// 88 = isp_backgnd_d
	AM_RANGE(0x008c, 0x008f) AM_READWRITE(isp_backgnd_t_r,    isp_backgnd_t_w)
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
	AM_RANGE(0x00c8, 0x00cb) AM_READWRITE(spg_hblank_int_r,   spg_hblank_int_w)
	AM_RANGE(0x00cc, 0x00cf) AM_READWRITE(spg_vblank_int_r,   spg_vblank_int_w)
	AM_RANGE(0x00d0, 0x00d3) AM_READWRITE(spg_control_r,      spg_control_w)
	AM_RANGE(0x00d4, 0x00d7) AM_READWRITE(spg_hblank_r,       spg_hblank_w)
	AM_RANGE(0x00d8, 0x00db) AM_READWRITE(spg_load_r,         spg_load_w)
	AM_RANGE(0x00dc, 0x00df) AM_READWRITE(spg_vblank_r,       spg_vblank_w)
	AM_RANGE(0x00e0, 0x00e3) AM_READWRITE(spg_width_r,        spg_width_w)
	AM_RANGE(0x00e4, 0x00e7) AM_READWRITE(text_control_r,     text_control_w)
	AM_RANGE(0x00e8, 0x00eb) AM_READWRITE(vo_control_r,       vo_control_w)
	AM_RANGE(0x00ec, 0x00ef) AM_READWRITE(vo_startx_r,        vo_startx_w)
	AM_RANGE(0x00f0, 0x00f3) AM_READWRITE(vo_starty_r,        vo_starty_w)
// f4 = scaler_ctl
	AM_RANGE(0x0108, 0x010b) AM_READWRITE(pal_ram_ctrl_r,     pal_ram_ctrl_w)
	AM_RANGE(0x010c, 0x010f) AM_READ(     spg_status_r)
// 110 = fb_burstctrl
// 118 = y_coeff
// 11c = pt_alpha_ref

	AM_RANGE(0x0124, 0x0127) AM_READWRITE(ta_ol_base_r,       ta_ol_base_w)
	AM_RANGE(0x0128, 0x012b) AM_READWRITE(ta_isp_base_r,      ta_isp_base_w)
	AM_RANGE(0x012c, 0x012f) AM_READWRITE(ta_ol_limit_r,      ta_ol_limit_w)
	AM_RANGE(0x0130, 0x0133) AM_READWRITE(ta_isp_limit_r,     ta_isp_limit_w)
	AM_RANGE(0x0134, 0x0137) AM_READ(     ta_next_opb_r)
	AM_RANGE(0x0138, 0x013b) AM_READ(     ta_itp_current_r)
// 13c = ta_glob_tile_clip
	AM_RANGE(0x0140, 0x0143) AM_READWRITE(ta_alloc_ctrl_r,    ta_alloc_ctrl_w)
	AM_RANGE(0x0144, 0x0147) AM_READWRITE(ta_list_init_r,     ta_list_init_w)
	AM_RANGE(0x0148, 0x014b) AM_READWRITE(ta_yuv_tex_base_r,  ta_yuv_tex_base_w)
	AM_RANGE(0x014c, 0x014f) AM_READWRITE(ta_yuv_tex_ctrl_r,  ta_yuv_tex_ctrl_w)
	AM_RANGE(0x0150, 0x0153) AM_READWRITE(ta_yuv_tex_cnt_r,   ta_yuv_tex_cnt_w)
	AM_RANGE(0x0160, 0x0163) AM_WRITE(    ta_list_cont_w)
	AM_RANGE(0x0164, 0x0167) AM_READWRITE(ta_next_opb_init_r, ta_next_opb_init_w)

	AM_RANGE(0x0200, 0x03ff) AM_READWRITE(fog_table_r,        fog_table_w)
	AM_RANGE(0x1000, 0x1fff) AM_READWRITE(palette_r,          palette_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(pd_dma_map, 32, powervr2_device)
	AM_RANGE(0x00,   0x03)   AM_READWRITE(sb_pdstap_r,        sb_pdstap_w)
	AM_RANGE(0x04,   0x07)   AM_READWRITE(sb_pdstar_r,        sb_pdstar_w)
	AM_RANGE(0x08,   0x0b)   AM_READWRITE(sb_pdlen_r,         sb_pdlen_w)
	AM_RANGE(0x0c,   0x0f)   AM_READWRITE(sb_pddir_r,         sb_pddir_w)
	AM_RANGE(0x10,   0x13)   AM_READWRITE(sb_pdtsel_r,        sb_pdtsel_w)
	AM_RANGE(0x14,   0x17)   AM_READWRITE(sb_pden_r,          sb_pden_w)
	AM_RANGE(0x18,   0x1b)   AM_READWRITE(sb_pdst_r,          sb_pdst_w)
	AM_RANGE(0x80,   0x83)   AM_READWRITE(sb_pdapro_r,        sb_pdapro_w)
ADDRESS_MAP_END

const int powervr2_device::pvr_parconfseq[] = {1,2,3,2,3,4,5,6,5,6,7,8,9,10,11,12,13,14,13,14,15,16,17,16,17,0,0,0,0,0,18,19,20,19,20,21,22,23,22,23};
const int powervr2_device::pvr_wordsvertex[24]  = {8,8,8,8,8,16,16,8,8,8, 8, 8,8,8,8,8,16,16, 8,16,16,8,16,16};
const int powervr2_device::pvr_wordspolygon[24] = {8,8,8,8,8, 8, 8,8,8,8,16,16,8,8,8,8, 8, 8,16,16,16,8, 8, 8};

#define DEBUG_FIFO_POLY (0)
#define DEBUG_PVRTA 0
#define DEBUG_PVRDLIST  (0)
#define DEBUG_PALRAM (0)
#define DEBUG_PVRCTRL   (0)

inline INT32 powervr2_device::clamp(INT32 in, INT32 min, INT32 max)
{
	if(in < min) return min;
	if(in > max) return max;
	return in;
}

// Perform a standard bilinear filter across four pixels
inline UINT32 powervr2_device::bilinear_filter(UINT32 c0, UINT32 c1, UINT32 c2, UINT32 c3, float u, float v)
{
	const UINT32 ui = (u * 256.0f);
	const UINT32 vi = (v * 256.0f);
	return rgbaint_t::bilinear_filter(c0, c1, c3, c2, ui, vi);
}

// Multiply with alpha value in bits 31-24
inline UINT32 powervr2_device::bla(UINT32 c, UINT32 a)
{
	a = a >> 24;
	return ((((c & 0xff00ff)*a) & 0xff00ff00) >> 8) | ((((c >> 8) & 0xff00ff)*a) & 0xff00ff00);
}

// Multiply with 1-alpha value in bits 31-24
inline UINT32 powervr2_device::blia(UINT32 c, UINT32 a)
{
	a = 0x100 - (a >> 24);
	return ((((c & 0xff00ff)*a) & 0xff00ff00) >> 8) | ((((c >> 8) & 0xff00ff)*a) & 0xff00ff00);
}

// Per-component multiply with color value
inline UINT32 powervr2_device::blc(UINT32 c1, UINT32 c2)
{
	UINT32 cr =
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
inline UINT32 powervr2_device::blic(UINT32 c1, UINT32 c2)
{
	UINT32 cr =
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
inline UINT32 powervr2_device::bls(UINT32 c1, UINT32 c2)
{
	UINT32 cr1, cr2;
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

// All 64 blending modes, 3 top bits are source mode, 3 bottom bits are destination mode
UINT32 powervr2_device::bl00(UINT32 s, UINT32 d) { return 0; }
UINT32 powervr2_device::bl01(UINT32 s, UINT32 d) { return d; }
UINT32 powervr2_device::bl02(UINT32 s, UINT32 d) { return blc(d, s); }
UINT32 powervr2_device::bl03(UINT32 s, UINT32 d) { return blic(d, s); }
UINT32 powervr2_device::bl04(UINT32 s, UINT32 d) { return bla(d, s); }
UINT32 powervr2_device::bl05(UINT32 s, UINT32 d) { return blia(d, s); }
UINT32 powervr2_device::bl06(UINT32 s, UINT32 d) { return bla(d, d); }
UINT32 powervr2_device::bl07(UINT32 s, UINT32 d) { return blia(d, d); }
UINT32 powervr2_device::bl10(UINT32 s, UINT32 d) { return s; }
UINT32 powervr2_device::bl11(UINT32 s, UINT32 d) { return bls(s, d); }
UINT32 powervr2_device::bl12(UINT32 s, UINT32 d) { return bls(s, blc(s, d)); }
UINT32 powervr2_device::bl13(UINT32 s, UINT32 d) { return bls(s, blic(s, d)); }
UINT32 powervr2_device::bl14(UINT32 s, UINT32 d) { return bls(s, bla(d, s)); }
UINT32 powervr2_device::bl15(UINT32 s, UINT32 d) { return bls(s, blia(d, s)); }
UINT32 powervr2_device::bl16(UINT32 s, UINT32 d) { return bls(s, bla(d, d)); }
UINT32 powervr2_device::bl17(UINT32 s, UINT32 d) { return bls(s, blia(d, d)); }
UINT32 powervr2_device::bl20(UINT32 s, UINT32 d) { return blc(d, s); }
UINT32 powervr2_device::bl21(UINT32 s, UINT32 d) { return bls(blc(d, s), d); }
UINT32 powervr2_device::bl22(UINT32 s, UINT32 d) { return bls(blc(d, s), blc(s, d)); }
UINT32 powervr2_device::bl23(UINT32 s, UINT32 d) { return bls(blc(d, s), blic(s, d)); }
UINT32 powervr2_device::bl24(UINT32 s, UINT32 d) { return bls(blc(d, s), bla(d, s)); }
UINT32 powervr2_device::bl25(UINT32 s, UINT32 d) { return bls(blc(d, s), blia(d, s)); }
UINT32 powervr2_device::bl26(UINT32 s, UINT32 d) { return bls(blc(d, s), bla(d, d)); }
UINT32 powervr2_device::bl27(UINT32 s, UINT32 d) { return bls(blc(d, s), blia(d, d)); }
UINT32 powervr2_device::bl30(UINT32 s, UINT32 d) { return blic(d, s); }
UINT32 powervr2_device::bl31(UINT32 s, UINT32 d) { return bls(blic(d, s), d); }
UINT32 powervr2_device::bl32(UINT32 s, UINT32 d) { return bls(blic(d, s), blc(s, d)); }
UINT32 powervr2_device::bl33(UINT32 s, UINT32 d) { return bls(blic(d, s), blic(s, d)); }
UINT32 powervr2_device::bl34(UINT32 s, UINT32 d) { return bls(blic(d, s), bla(d, s)); }
UINT32 powervr2_device::bl35(UINT32 s, UINT32 d) { return bls(blic(d, s), blia(d, s)); }
UINT32 powervr2_device::bl36(UINT32 s, UINT32 d) { return bls(blic(d, s), bla(d, d)); }
UINT32 powervr2_device::bl37(UINT32 s, UINT32 d) { return bls(blic(d, s), blia(d, d)); }
UINT32 powervr2_device::bl40(UINT32 s, UINT32 d) { return bla(s, s); }
UINT32 powervr2_device::bl41(UINT32 s, UINT32 d) { return bls(bla(s, s), d); }
UINT32 powervr2_device::bl42(UINT32 s, UINT32 d) { return bls(bla(s, s), blc(s, d)); }
UINT32 powervr2_device::bl43(UINT32 s, UINT32 d) { return bls(bla(s, s), blic(s, d)); }
UINT32 powervr2_device::bl44(UINT32 s, UINT32 d) { return bls(bla(s, s), bla(d, s)); }
UINT32 powervr2_device::bl45(UINT32 s, UINT32 d) { return bls(bla(s, s), blia(d, s)); }
UINT32 powervr2_device::bl46(UINT32 s, UINT32 d) { return bls(bla(s, s), bla(d, d)); }
UINT32 powervr2_device::bl47(UINT32 s, UINT32 d) { return bls(bla(s, s), blia(d, d)); }
UINT32 powervr2_device::bl50(UINT32 s, UINT32 d) { return blia(s, s); }
UINT32 powervr2_device::bl51(UINT32 s, UINT32 d) { return bls(blia(s, s), d); }
UINT32 powervr2_device::bl52(UINT32 s, UINT32 d) { return bls(blia(s, s), blc(s, d)); }
UINT32 powervr2_device::bl53(UINT32 s, UINT32 d) { return bls(blia(s, s), blic(s, d)); }
UINT32 powervr2_device::bl54(UINT32 s, UINT32 d) { return bls(blia(s, s), bla(d, s)); }
UINT32 powervr2_device::bl55(UINT32 s, UINT32 d) { return bls(blia(s, s), blia(d, s)); }
UINT32 powervr2_device::bl56(UINT32 s, UINT32 d) { return bls(blia(s, s), bla(d, d)); }
UINT32 powervr2_device::bl57(UINT32 s, UINT32 d) { return bls(blia(s, s), blia(d, d)); }
UINT32 powervr2_device::bl60(UINT32 s, UINT32 d) { return bla(s, d); }
UINT32 powervr2_device::bl61(UINT32 s, UINT32 d) { return bls(bla(s, d), d); }
UINT32 powervr2_device::bl62(UINT32 s, UINT32 d) { return bls(bla(s, d), blc(s, d)); }
UINT32 powervr2_device::bl63(UINT32 s, UINT32 d) { return bls(bla(s, d), blic(s, d)); }
UINT32 powervr2_device::bl64(UINT32 s, UINT32 d) { return bls(bla(s, d), bla(d, s)); }
UINT32 powervr2_device::bl65(UINT32 s, UINT32 d) { return bls(bla(s, d), blia(d, s)); }
UINT32 powervr2_device::bl66(UINT32 s, UINT32 d) { return bls(bla(s, d), bla(d, d)); }
UINT32 powervr2_device::bl67(UINT32 s, UINT32 d) { return bls(bla(s, d), blia(d, d)); }
UINT32 powervr2_device::bl70(UINT32 s, UINT32 d) { return blia(s, d); }
UINT32 powervr2_device::bl71(UINT32 s, UINT32 d) { return bls(blia(s, d), d); }
UINT32 powervr2_device::bl72(UINT32 s, UINT32 d) { return bls(blia(s, d), blc(s, d)); }
UINT32 powervr2_device::bl73(UINT32 s, UINT32 d) { return bls(blia(s, d), blic(s, d)); }
UINT32 powervr2_device::bl74(UINT32 s, UINT32 d) { return bls(blia(s, d), bla(d, s)); }
UINT32 powervr2_device::bl75(UINT32 s, UINT32 d) { return bls(blia(s, d), blia(d, s)); }
UINT32 powervr2_device::bl76(UINT32 s, UINT32 d) { return bls(blia(s, d), bla(d, d)); }
UINT32 powervr2_device::bl77(UINT32 s, UINT32 d) { return bls(blia(s, d), blia(d, d)); }

UINT32 (*const powervr2_device::blend_functions[64])(UINT32 s, UINT32 d) = {
	bl00, bl01, bl02, bl03, bl04, bl05, bl06, bl07,
	bl10, bl11, bl12, bl13, bl14, bl15, bl16, bl17,
	bl20, bl21, bl22, bl23, bl24, bl25, bl26, bl27,
	bl30, bl31, bl32, bl33, bl34, bl35, bl36, bl37,
	bl40, bl41, bl42, bl43, bl44, bl45, bl46, bl47,
	bl50, bl51, bl52, bl53, bl54, bl55, bl56, bl57,
	bl60, bl61, bl62, bl63, bl64, bl65, bl66, bl67,
	bl70, bl71, bl72, bl73, bl74, bl75, bl76, bl77,
};

inline UINT32 powervr2_device::cv_1555(UINT16 c)
{
	return
		(c & 0x8000 ? 0xff000000 : 0) |
		((c << 9) & 0x00f80000) | ((c << 4) & 0x00070000) |
		((c << 6) & 0x0000f800) | ((c << 1) & 0x00000700) |
		((c << 3) & 0x000000f8) | ((c >> 2) & 0x00000007);
}

inline UINT32 powervr2_device::cv_1555z(UINT16 c)
{
	return
		(c & 0x8000 ? 0xff000000 : 0) |
		((c << 9) & 0x00f80000) |
		((c << 6) & 0x0000f800) |
		((c << 3) & 0x000000f8);
}

inline UINT32 powervr2_device::cv_565(UINT16 c)
{
	return
		0xff000000 |
		((c << 8) & 0x00f80000) | ((c << 3) & 0x00070000) |
		((c << 5) & 0x0000fc00) | ((c >> 1) & 0x00000300) |
		((c << 3) & 0x000000f8) | ((c >> 2) & 0x00000007);
}

inline UINT32 powervr2_device::cv_565z(UINT16 c)
{
	return
		0xff000000 |
		((c << 8) & 0x00f80000) |
		((c << 5) & 0x0000fc00) |
		((c << 3) & 0x000000f8);
}

inline UINT32 powervr2_device::cv_4444(UINT16 c)
{
	return
		((c << 16) & 0xf0000000) | ((c << 12) & 0x0f000000) |
		((c << 12) & 0x00f00000) | ((c <<  8) & 0x000f0000) |
		((c <<  8) & 0x0000f000) | ((c <<  4) & 0x00000f00) |
		((c <<  4) & 0x000000f0) | ((c      ) & 0x0000000f);
}

inline UINT32 powervr2_device::cv_4444z(UINT16 c)
{
	return
		((c << 16) & 0xf0000000) |
		((c << 12) & 0x00f00000) |
		((c <<  8) & 0x0000f000) |
		((c <<  4) & 0x000000f0);
}

inline UINT32 powervr2_device::cv_yuv(UINT16 c1, UINT16 c2, int x)
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

UINT32 powervr2_device::tex_r_yuv_n(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int addrp = t->address + (t->stride*yt + (xt & ~1))*2;
	UINT16 c1 = *(UINT16 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + WORD_XOR_LE(addrp));
	UINT16 c2 = *(UINT16 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + WORD_XOR_LE(addrp+2));
	return cv_yuv(c1, c2, xt);
}

UINT32 powervr2_device::tex_r_yuv_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int addrp = t->address + (dilated1[t->cd][xt & ~1] + dilated0[t->cd][yt])*2;
	UINT16 c1 = *(UINT16 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + WORD_XOR_LE(addrp));
	UINT16 c2 = *(UINT16 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + WORD_XOR_LE(addrp+4));
	return cv_yuv(c1, c2, xt);
}

#if 0
UINT32 powervr2_device::tex_r_yuv_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + (dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 1])*2;
	UINT16 c1 = *(UINT16 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + WORD_XOR_LE(addrp));
	UINT16 c2 = *(UINT16 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + WORD_XOR_LE(addrp+4));
	return cv_yuv(c1, c2, xt);
}
#endif

UINT32 powervr2_device::tex_r_1555_n(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int addrp = t->address + (t->stride*yt + xt)*2;
	return cv_1555z(*(UINT16 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

UINT32 powervr2_device::tex_r_1555_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int addrp = t->address + (dilated1[t->cd][xt] + dilated0[t->cd][yt])*2;
	return cv_1555(*(UINT16 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

UINT32 powervr2_device::tex_r_1555_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + (dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 1])*2;
	return cv_1555(*(UINT16 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

UINT32 powervr2_device::tex_r_565_n(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int addrp = t->address + (t->stride*yt + xt)*2;
	return cv_565z(*(UINT16 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

UINT32 powervr2_device::tex_r_565_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int addrp = t->address + (dilated1[t->cd][xt] + dilated0[t->cd][yt])*2;
	return cv_565(*(UINT16 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

UINT32 powervr2_device::tex_r_565_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + (dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 1])*2;
	return cv_565(*(UINT16 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

UINT32 powervr2_device::tex_r_4444_n(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int addrp = t->address + (t->stride*yt + xt)*2;
	return cv_4444z(*(UINT16 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

UINT32 powervr2_device::tex_r_4444_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int addrp = t->address + (dilated1[t->cd][xt] + dilated0[t->cd][yt])*2;
	return cv_4444(*(UINT16 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

UINT32 powervr2_device::tex_r_4444_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + (dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 1])*2;
	return cv_4444(*(UINT16 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + WORD_XOR_LE(addrp)));
}

UINT32 powervr2_device::tex_r_nt_palint(texinfo *t, float x, float y)
{
	return t->nontextured_pal_int;
}

UINT32 powervr2_device::tex_r_nt_palfloat(texinfo *t, float x, float y)
{
	return (t->nontextured_fpal_a << 24) | (t->nontextured_fpal_r << 16) | (t->nontextured_fpal_g << 8) | (t->nontextured_fpal_b);
}

UINT32 powervr2_device::tex_r_p4_1555_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int off = dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int addrp = t->address + (off >> 1);
	int c = ((reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] >> ((off & 1) << 2)) & 0xf;
	return cv_1555(palette[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p4_1555_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] & 0xf;
	return cv_1555(palette[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p4_565_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int off = dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int addrp = t->address + (off >> 1);
	int c = ((reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] >> ((off & 1) << 2)) & 0xf;
	return cv_565(palette[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p4_565_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] & 0xf;
	return cv_565(palette[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p4_4444_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int off = dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int addrp = t->address + (off >> 1);
	int c = ((reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] >> ((off & 1) << 2)) & 0xf;
	return cv_4444(palette[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p4_4444_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] & 0xf;
	return cv_4444(palette[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p4_8888_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int off = dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int addrp = t->address + (off >> 1);
	int c = ((reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] >> ((off & 1) << 2)) & 0xf;
	return palette[t->palbase + c];
}

UINT32 powervr2_device::tex_r_p4_8888_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] & 0xf;
	return palette[t->palbase + c];
}

UINT32 powervr2_device::tex_r_p8_1555_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int addrp = t->address + dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_1555(palette[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p8_1555_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_1555(palette[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p8_565_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int addrp = t->address + dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_565(palette[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p8_565_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_565(palette[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p8_4444_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int addrp = t->address + dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_4444(palette[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p8_4444_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_4444(palette[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p8_8888_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int addrp = t->address + dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return palette[t->palbase + c];
}

UINT32 powervr2_device::tex_r_p8_8888_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return palette[t->palbase + c];
}


UINT32 powervr2_device::tex_r_default(texinfo *t, float x, float y)
{
	return ((int)x ^ (int)y) & 4 ? 0xffffff00 : 0xff0000ff;
}

void powervr2_device::tex_get_info(texinfo *t)
{
	int miptype = 0;

	t->textured    = texture;

	// not textured, abort.
//  if (!t->textured) return;

	t->address     = textureaddress;
	t->pf          = pixelformat;
	t->palette     = 0;

	t->mode = (vqcompressed<<1);

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
	t->mipmapped  = t->mode & 1 ? 0 : mipmapped;

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
	t->flip_u      = (flipuv >> 1) & 1;
	t->flip_v      = flipuv & 1;

	t->r = &powervr2_device::tex_r_default;
	t->cd = dilatechose[t->sizes];
	t->palbase = 0;
	t->vqbase = t->address;
	t->blend = use_alpha ? blend_functions[t->blend_mode] : bl10;

//  fprintf(stderr, "tex %d %d %d %d\n", t->pf, t->mode, pal_ram_ctrl, t->mipmapped);
	if(!t->textured)
	{
		t->coltype = coltype;
		switch(t->coltype) {
			case 0: // packed color
				t->nontextured_pal_int = nontextured_pal_int;
				t->r = &powervr2_device::tex_r_nt_palint;
				break;
			case 1: // floating color
				/* TODO: might be converted even earlier I believe */
				t->nontextured_fpal_a = (UINT8)(nontextured_fpal_a * 255.0f);
				t->nontextured_fpal_r = (UINT8)(nontextured_fpal_r * 255.0f);
				t->nontextured_fpal_g = (UINT8)(nontextured_fpal_g * 255.0f);
				t->nontextured_fpal_b = (UINT8)(nontextured_fpal_b * 255.0f);
				t->r = &powervr2_device::tex_r_nt_palfloat;
				break;
		}
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

READ32_MEMBER( powervr2_device::id_r )
{
	return 0x17fd11db;
}

READ32_MEMBER( powervr2_device::revision_r )
{
	return 0x00000011;
}

READ32_MEMBER( powervr2_device::softreset_r )
{
	return softreset;
}

WRITE32_MEMBER( powervr2_device::softreset_w )
{
	COMBINE_DATA(&softreset);
	if (softreset & 1) {
#if DEBUG_PVRTA
		logerror("%s: TA soft reset\n", tag());
#endif
		listtype_used=0;
	}
	if (softreset & 2) {
#if DEBUG_PVRTA
		logerror("%s: Core Pipeline soft reset\n", tag());
#endif
		if (start_render_received == 1) {
			for (auto & elem : grab)
				if (elem.busy == 1)
					elem.busy = 0;
			start_render_received = 0;
		}
	}
	if (softreset & 4) {
#if DEBUG_PVRTA
		logerror("%s: sdram I/F soft reset\n", tag());
#endif
	}
}

WRITE32_MEMBER( powervr2_device::startrender_w )
{
	dc_state *state = machine().driver_data<dc_state>();
	g_profiler.start(PROFILER_USER1);
#if DEBUG_PVRTA
	logerror("%s: Start render, region=%08x, params=%08x\n", tag(), region_base, param_base);
#endif

	// select buffer to draw using param_base
	for (int a=0;a < NUM_BUFFERS;a++) {
		if ((grab[a].ispbase == param_base) && (grab[a].valid == 1) && (grab[a].busy == 0)) {
			grab[a].busy = 1;
			renderselect = a;
			start_render_received=1;


			grab[a].fbwsof1 = fb_w_sof1;
			grab[a].fbwsof2 = fb_w_sof2;

			rectangle clip(0, 1023, 0, 1023);

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
				UINT32 st[6];

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
//          printf("ISP START %d %d\n",sanitycount,m_screen->vpos());
			/* Fire ISP irq after a set amount of time TODO: timing of this */
			endofrender_timer_isp->adjust(state->m_maincpu->cycles_to_attotime(sanitycount*25));
			break;
		}
	}
}


READ32_MEMBER( powervr2_device::param_base_r )
{
	return param_base;
}

WRITE32_MEMBER( powervr2_device::param_base_w )
{
	COMBINE_DATA(&param_base);
}

READ32_MEMBER( powervr2_device::region_base_r )
{
	return region_base;
}

WRITE32_MEMBER( powervr2_device::region_base_w )
{
	COMBINE_DATA(&region_base);
}

READ32_MEMBER( powervr2_device::vo_border_col_r )
{
	return vo_border_col;
}

WRITE32_MEMBER( powervr2_device::vo_border_col_w )
{
	COMBINE_DATA(&vo_border_col);
}

READ32_MEMBER( powervr2_device::fb_r_ctrl_r )
{
	return fb_r_ctrl;
}

WRITE32_MEMBER( powervr2_device::fb_r_ctrl_w )
{
	COMBINE_DATA(&fb_r_ctrl);
}

READ32_MEMBER( powervr2_device::fb_w_ctrl_r )
{
	return fb_w_ctrl;
}

WRITE32_MEMBER( powervr2_device::fb_w_ctrl_w )
{
	COMBINE_DATA(&fb_w_ctrl);
}

READ32_MEMBER( powervr2_device::fb_w_linestride_r )
{
	return fb_w_linestride;
}

WRITE32_MEMBER( powervr2_device::fb_w_linestride_w )
{
	COMBINE_DATA(&fb_w_linestride);
}

READ32_MEMBER( powervr2_device::fb_r_sof1_r )
{
	return fb_r_sof1;
}

WRITE32_MEMBER( powervr2_device::fb_r_sof1_w )
{
	COMBINE_DATA(&fb_r_sof1);
}

READ32_MEMBER( powervr2_device::fb_r_sof2_r )
{
	return fb_r_sof2;
}

WRITE32_MEMBER( powervr2_device::fb_r_sof2_w )
{
	COMBINE_DATA(&fb_r_sof2);
}

READ32_MEMBER( powervr2_device::fb_r_size_r )
{
	return fb_r_size;
}

WRITE32_MEMBER( powervr2_device::fb_r_size_w )
{
	COMBINE_DATA(&fb_r_size);
}

READ32_MEMBER( powervr2_device::fb_w_sof1_r )
{
	return fb_w_sof1;
}

WRITE32_MEMBER( powervr2_device::fb_w_sof1_w )
{
	COMBINE_DATA(&fb_w_sof1);
}

READ32_MEMBER( powervr2_device::fb_w_sof2_r )
{
	return fb_w_sof2;
}

WRITE32_MEMBER( powervr2_device::fb_w_sof2_w )
{
	COMBINE_DATA(&fb_w_sof2);
}

READ32_MEMBER( powervr2_device::fb_x_clip_r )
{
	return fb_x_clip;
}

WRITE32_MEMBER( powervr2_device::fb_x_clip_w )
{
	COMBINE_DATA(&fb_x_clip);
}

READ32_MEMBER( powervr2_device::fb_y_clip_r )
{
	return fb_y_clip;
}

WRITE32_MEMBER( powervr2_device::fb_y_clip_w )
{
	COMBINE_DATA(&fb_y_clip);
}

READ32_MEMBER( powervr2_device::fpu_param_cfg_r )
{
	return fpu_param_cfg;
}

WRITE32_MEMBER( powervr2_device::fpu_param_cfg_w )
{
	COMBINE_DATA(&fpu_param_cfg);
}

READ32_MEMBER( powervr2_device::isp_backgnd_t_r )
{
	return isp_backgnd_t;
}

WRITE32_MEMBER( powervr2_device::isp_backgnd_t_w )
{
	COMBINE_DATA(&isp_backgnd_t);
}

READ32_MEMBER( powervr2_device::spg_hblank_int_r )
{
	return spg_hblank_int;
}

WRITE32_MEMBER( powervr2_device::spg_hblank_int_w )
{
	COMBINE_DATA(&spg_hblank_int);
	/* TODO: timer adjust */
}

READ32_MEMBER( powervr2_device::spg_vblank_int_r )
{
	return spg_vblank_int;
}

WRITE32_MEMBER( powervr2_device::spg_vblank_int_w )
{
	COMBINE_DATA(&spg_vblank_int);

	/* clear pending irqs and modify them with the updated ones */
//  vbin_timer->adjust(attotime::never);
//  vbout_timer->adjust(attotime::never);

//  vbin_timer->adjust(m_screen->time_until_pos(spg_vblank_int & 0x3ff));
//  vbout_timer->adjust(m_screen->time_until_pos((spg_vblank_int >> 16) & 0x3ff));
}

READ32_MEMBER( powervr2_device::spg_control_r )
{
	return spg_control;
}

WRITE32_MEMBER( powervr2_device::spg_control_w )
{
	COMBINE_DATA(&spg_control);
	update_screen_format();

	if((spg_control & 0xc0) == 0xc0)
		popmessage("SPG undocumented pixel clock mode 11, contact MAME/MESSdev");

	if((spg_control & 0xd0) == 0x10)
		popmessage("SPG enabled VGA mode with interlace, contact MAME/MESSdev");
}

READ32_MEMBER( powervr2_device::spg_hblank_r )
{
	return spg_hblank;
}

WRITE32_MEMBER( powervr2_device::spg_hblank_w )
{
	COMBINE_DATA(&spg_hblank);
	update_screen_format();
}

READ32_MEMBER( powervr2_device::spg_load_r )
{
	return spg_load;
}

WRITE32_MEMBER( powervr2_device::spg_load_w )
{
	COMBINE_DATA(&spg_load);
	update_screen_format();
}

READ32_MEMBER( powervr2_device::spg_vblank_r )
{
	return spg_vblank;
}

WRITE32_MEMBER( powervr2_device::spg_vblank_w )
{
	COMBINE_DATA(&spg_vblank);
	update_screen_format();
}

READ32_MEMBER( powervr2_device::spg_width_r )
{
	return spg_width;
}

WRITE32_MEMBER( powervr2_device::spg_width_w )
{
	COMBINE_DATA(&spg_width);
	update_screen_format();
}

READ32_MEMBER( powervr2_device::text_control_r )
{
	return text_control;
}

WRITE32_MEMBER( powervr2_device::text_control_w )
{
	COMBINE_DATA(&text_control);
}

READ32_MEMBER( powervr2_device::vo_control_r )
{
	return vo_control;
}

WRITE32_MEMBER( powervr2_device::vo_control_w )
{
	COMBINE_DATA(&vo_control);
}

READ32_MEMBER( powervr2_device::vo_startx_r )
{
	return vo_startx;
}

WRITE32_MEMBER( powervr2_device::vo_startx_w )
{
	COMBINE_DATA(&vo_startx);
	update_screen_format();
}

READ32_MEMBER( powervr2_device::vo_starty_r )
{
	return vo_starty;
}

WRITE32_MEMBER( powervr2_device::vo_starty_w )
{
	COMBINE_DATA(&vo_starty);
	update_screen_format();
}

READ32_MEMBER( powervr2_device::pal_ram_ctrl_r )
{
	return pal_ram_ctrl;
}

WRITE32_MEMBER( powervr2_device::pal_ram_ctrl_w )
{
	COMBINE_DATA(&pal_ram_ctrl);
}

READ32_MEMBER( powervr2_device::spg_status_r )
{
	UINT32 fieldnum = (m_screen->frame_number() & 1) ? 1 : 0;
	INT32 spg_hbstart = spg_hblank & 0x3ff;
	INT32 spg_hbend = (spg_hblank >> 16) & 0x3ff;
	INT32 spg_vbstart = spg_vblank & 0x3ff;
	INT32 spg_vbend = (spg_vblank >> 16) & 0x3ff;

	UINT32 vsync = ((m_screen->vpos() >= spg_vbstart) || (m_screen->vpos() < spg_vbend)) ? 0 : 1;
	UINT32 hsync = ((m_screen->hpos() >= spg_hbstart) || (m_screen->hpos() < spg_hbend)) ? 0 : 1;
	/* FIXME: following is just a wild guess */
	UINT32 blank = ((m_screen->vpos() >= spg_vbstart) || (m_screen->vpos() < spg_vbend) |
					(m_screen->hpos() >= spg_hbstart) || (m_screen->hpos() < spg_hbend)) ? 0 : 1;
	if(vo_control & 4) { blank^=1; }
	if(vo_control & 2) { vsync^=1; }
	if(vo_control & 1) { hsync^=1; }

	return (vsync << 13) | (hsync << 12) | (blank << 11) | (fieldnum << 10) | (m_screen->vpos() & 0x3ff);
}


READ32_MEMBER( powervr2_device::ta_ol_base_r )
{
	return ta_ol_base;
}

WRITE32_MEMBER( powervr2_device::ta_ol_base_w )
{
	COMBINE_DATA(&ta_ol_base);
}

READ32_MEMBER( powervr2_device::ta_isp_base_r )
{
	return ta_isp_base;
}

WRITE32_MEMBER( powervr2_device::ta_isp_base_w )
{
	COMBINE_DATA(&ta_isp_base);
}

READ32_MEMBER( powervr2_device::ta_ol_limit_r )
{
	return ta_ol_limit;
}

WRITE32_MEMBER( powervr2_device::ta_ol_limit_w )
{
	COMBINE_DATA(&ta_ol_limit);
}

READ32_MEMBER( powervr2_device::ta_isp_limit_r )
{
	return ta_isp_limit;
}

WRITE32_MEMBER( powervr2_device::ta_isp_limit_w )
{
	COMBINE_DATA(&ta_isp_limit);
}

READ32_MEMBER( powervr2_device::ta_next_opb_r )
{
	return ta_next_opb;
}

READ32_MEMBER( powervr2_device::ta_itp_current_r )
{
	return ta_itp_current;
}

READ32_MEMBER( powervr2_device::ta_alloc_ctrl_r )
{
	return ta_alloc_ctrl;
}

WRITE32_MEMBER( powervr2_device::ta_alloc_ctrl_w )
{
	COMBINE_DATA(&ta_alloc_ctrl);
}

READ32_MEMBER( powervr2_device::ta_list_init_r )
{
	return 0; //bit 31 always return 0, a probable left-over in Crazy Taxi reads this and discards the read (?)
}

WRITE32_MEMBER( powervr2_device::ta_list_init_w )
{
	if(data & 0x80000000) {
		tafifo_pos=0;
		tafifo_mask=7;
		tafifo_vertexwords=8;
		tafifo_listtype= -1;
#if DEBUG_PVRTA
		logerror("%s: list init ol=(%08x, %08x) isp=(%08x, %08x), alloc=%08x obp=%08x\n",
					tag(), ta_ol_base, ta_ol_limit, ta_isp_base, ta_isp_limit, ta_alloc_ctrl, ta_next_opb_init);
#endif
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
			assert_always(0, "TA grabber error B!\n");
		grabsellast=grabsel;
		grab[grabsel].ispbase=ta_isp_base;
		grab[grabsel].busy=0;
		grab[grabsel].valid=1;
		grab[grabsel].verts_size=0;
		grab[grabsel].strips_size=0;

		g_profiler.stop();
	}
}


READ32_MEMBER( powervr2_device::ta_yuv_tex_base_r )
{
	return ta_yuv_tex_base;
}

WRITE32_MEMBER( powervr2_device::ta_yuv_tex_base_w )
{
	COMBINE_DATA(&ta_yuv_tex_base);
	logerror("%s: ta_yuv_tex_base = %08x\n", tag(), ta_yuv_tex_base);

	ta_yuv_index = 0;
	ta_yuv_x = 0;
	ta_yuv_y = 0;

}

READ32_MEMBER( powervr2_device::ta_yuv_tex_ctrl_r )
{
	return ta_yuv_tex_ctrl;
}

WRITE32_MEMBER( powervr2_device::ta_yuv_tex_ctrl_w )
{
	COMBINE_DATA(&ta_yuv_tex_ctrl);
	ta_yuv_x_size = ((ta_yuv_tex_ctrl & 0x3f)+1)*16;
	ta_yuv_y_size = (((ta_yuv_tex_ctrl>>8) & 0x3f)+1)*16;
	logerror("%s: ta_yuv_tex_ctrl = %08x\n", tag(), ta_yuv_tex_ctrl);
	if(ta_yuv_tex_ctrl & 0x01010000)
		fatalerror("YUV with setting %08x",ta_yuv_tex_ctrl);
}

#include "debugger.h"
/* TODO */
READ32_MEMBER( powervr2_device::ta_yuv_tex_cnt_r )
{
	debugger_break(machine());
	return ta_yuv_tex_cnt;
}

WRITE32_MEMBER( powervr2_device::ta_yuv_tex_cnt_w )
{
	debugger_break(machine());
	COMBINE_DATA(&ta_yuv_tex_cnt);
}

WRITE32_MEMBER( powervr2_device::ta_list_cont_w )
{
	if(data & 0x80000000) {
		tafifo_listtype= -1; // no list being received
		listtype_used |= (1+4);
	}
}

READ32_MEMBER( powervr2_device::ta_next_opb_init_r )
{
	return ta_next_opb_init;
}

WRITE32_MEMBER( powervr2_device::ta_next_opb_init_w )
{
	COMBINE_DATA(&ta_next_opb_init);
}


READ32_MEMBER( powervr2_device::fog_table_r )
{
	return fog_table[offset];
}

WRITE32_MEMBER( powervr2_device::fog_table_w )
{
	COMBINE_DATA(fog_table+offset);
}

READ32_MEMBER( powervr2_device::palette_r )
{
	return palette[offset];
}

WRITE32_MEMBER( powervr2_device::palette_w )
{
	COMBINE_DATA(palette+offset);
}

void powervr2_device::update_screen_format()
{
	/*                        00=VGA    01=NTSC   10=PAL,   11=illegal/undocumented */
	const int spg_clks[4] = { 26944080, 13458568, 13462800, 26944080 };
	INT32 spg_hsize = spg_load & 0x3ff;
	INT32 spg_vsize = (spg_load >> 16) & 0x3ff;
	INT32 spg_hbstart = spg_hblank & 0x3ff;
	INT32 spg_hbend = (spg_hblank >> 16) & 0x3ff;
	INT32 spg_vbstart = spg_vblank & 0x3ff;
	INT32 spg_vbend = (spg_vblank >> 16) & 0x3ff;
	//INT32 vo_horz_start_pos = vo_startx & 0x3ff;
	//INT32 vo_vert_start_pos_f1 = vo_starty & 0x3ff;
	int pclk = spg_clks[(spg_control >> 6) & 3] * (((spg_control & 0x10) >> 4)+1);

	attoseconds_t refresh = HZ_TO_ATTOSECONDS(pclk) * spg_hsize * spg_vsize;

	rectangle visarea = m_screen->visible_area();

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

	m_screen->configure(spg_hsize, spg_vsize, visarea, refresh );
}


READ32_MEMBER( powervr2_device::sb_pdstap_r )
{
	return sb_pdstap;
}

WRITE32_MEMBER( powervr2_device::sb_pdstap_w )
{
	COMBINE_DATA(&sb_pdstap);
	m_pvr_dma.pvr_addr = sb_pdstap;
}

READ32_MEMBER( powervr2_device::sb_pdstar_r )
{
	return sb_pdstar;
}

WRITE32_MEMBER( powervr2_device::sb_pdstar_w )
{
	COMBINE_DATA(&sb_pdstar);
	m_pvr_dma.sys_addr = sb_pdstar;
}

READ32_MEMBER( powervr2_device::sb_pdlen_r )
{
	return sb_pdlen;
}

WRITE32_MEMBER( powervr2_device::sb_pdlen_w )
{
	COMBINE_DATA(&sb_pdlen);
	m_pvr_dma.size = sb_pdlen;
}

READ32_MEMBER( powervr2_device::sb_pddir_r )
{
	return sb_pddir;
}

WRITE32_MEMBER( powervr2_device::sb_pddir_w )
{
	COMBINE_DATA(&sb_pddir);
	m_pvr_dma.dir = sb_pddir;
}

READ32_MEMBER( powervr2_device::sb_pdtsel_r )
{
	return sb_pdtsel;
}

WRITE32_MEMBER( powervr2_device::sb_pdtsel_w )
{
	COMBINE_DATA(&sb_pdtsel);
	m_pvr_dma.sel = sb_pdtsel & 1;
}

READ32_MEMBER( powervr2_device::sb_pden_r )
{
	return sb_pden;
}

WRITE32_MEMBER( powervr2_device::sb_pden_w )
{
	COMBINE_DATA(&sb_pden);
	m_pvr_dma.flag = sb_pden & 1;
}

READ32_MEMBER( powervr2_device::sb_pdst_r )
{
	return sb_pdst;
}

WRITE32_MEMBER( powervr2_device::sb_pdst_w )
{
	COMBINE_DATA(&sb_pdst);

	UINT32 old = m_pvr_dma.start & 1;
	m_pvr_dma.start = sb_pdst & 1;

	if(((old & 1) == 0) && m_pvr_dma.flag && m_pvr_dma.start && ((m_pvr_dma.sel & 1) == 0)) // 0 -> 1
		pvr_dma_execute(space);
}

READ32_MEMBER( powervr2_device::sb_pdapro_r )
{
	return sb_pdapro;
}

WRITE32_MEMBER( powervr2_device::sb_pdapro_w )
{
	COMBINE_DATA(&sb_pdapro);
}


TIMER_CALLBACK_MEMBER(powervr2_device::transfer_opaque_list_irq)
{
//  printf("OPLST %d\n",m_screen->vpos());

	irq_cb(EOXFER_OPLST_IRQ);
}

TIMER_CALLBACK_MEMBER(powervr2_device::transfer_opaque_modifier_volume_list_irq)
{
//  printf("OPMV %d\n",m_screen->vpos());

	irq_cb(EOXFER_OPMV_IRQ);
}

TIMER_CALLBACK_MEMBER(powervr2_device::transfer_translucent_list_irq)
{
//  printf("TRLST %d\n",m_screen->vpos());

	irq_cb(EOXFER_TRLST_IRQ);
}

TIMER_CALLBACK_MEMBER(powervr2_device::transfer_translucent_modifier_volume_list_irq)
{
//  printf("TRMV %d\n",m_screen->vpos());

	irq_cb(EOXFER_TRMV_IRQ);
}

TIMER_CALLBACK_MEMBER(powervr2_device::transfer_punch_through_list_irq)
{
//  printf("PTLST %d\n",m_screen->vpos());

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
		offfset=(objcontrol >> 2) & 1;
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
			if ((global_paratype == 5) || (tafifo_listtype == 1) || (tafifo_listtype == 3))
				tafifo_vertexwords = 16;
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
	tafifo_mask = 7;

	// now we heve all the needed words
	// here we should generate the data for the various tiles
	// for now, just interpret their meaning
	if (paratype == 0)
	{ // end of list
		#if DEBUG_PVRDLIST
		osd_printf_verbose("Para Type 0 End of List\n");
		#endif
		/* Process transfer FIFO done irqs here */
		/* FIXME: timing of these */
		//printf("%d %d\n",tafifo_listtype,m_screen->vpos());
		switch (tafifo_listtype)
		{
		case 0: machine().scheduler().timer_set(attotime::from_usec(100), timer_expired_delegate(FUNC(powervr2_device::transfer_opaque_list_irq), this)); break;
		case 1: machine().scheduler().timer_set(attotime::from_usec(100), timer_expired_delegate(FUNC(powervr2_device::transfer_opaque_modifier_volume_list_irq), this)); break;
		case 2: machine().scheduler().timer_set(attotime::from_usec(100), timer_expired_delegate(FUNC(powervr2_device::transfer_translucent_list_irq), this)); break;
		case 3: machine().scheduler().timer_set(attotime::from_usec(100), timer_expired_delegate(FUNC(powervr2_device::transfer_translucent_modifier_volume_list_irq), this)); break;
		case 4: machine().scheduler().timer_set(attotime::from_usec(100), timer_expired_delegate(FUNC(powervr2_device::transfer_punch_through_list_irq), this)); break;
		}
		tafifo_listtype= -1; // no list being received
		listtype_used |= (2+8);
	}
	else if (paratype == 1)
	{ // user tile clip
		#if DEBUG_PVRDLIST
		osd_printf_verbose("Para Type 1 User Tile Clip\n");
		osd_printf_verbose(" (%d , %d)-(%d , %d)\n", tafifo_buff[4], tafifo_buff[5], tafifo_buff[6], tafifo_buff[7]);
		#endif
	}
	else if (paratype == 2)
	{ // object list set
		#if DEBUG_PVRDLIST
		osd_printf_verbose("Para Type 2 Object List Set at %08x\n", tafifo_buff[1]);
		osd_printf_verbose(" (%d , %d)-(%d , %d)\n", tafifo_buff[4], tafifo_buff[5], tafifo_buff[6], tafifo_buff[7]);
		#endif
	}
	else if (paratype == 3)
	{
		#if DEBUG_PVRDLIST
		osd_printf_verbose("Para Type %x Unknown!\n", tafifo_buff[0]);
		#endif
	}
	else
	{ // global parameter or vertex parameter
		#if DEBUG_PVRDLIST
		osd_printf_verbose("Para Type %d", paratype);
		if (paratype == 7)
			osd_printf_verbose(" End of Strip %d", endofstrip);
		if (listtype_used & 3)
			osd_printf_verbose(" List Type %d", listtype);
		osd_printf_verbose("\n");
		#endif

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
				#if DEBUG_PVRDLIST
				osd_printf_verbose(" Texture at %08x format %d\n", (tafifo_buff[3] & 0x1FFFFF) << 3, pixelformat);
				#endif
			}
			if (paratype == 4)
			{ // polygon or mv
				if ((tafifo_listtype == 1) || (tafifo_listtype == 3))
				{
				#if DEBUG_PVRDLIST
					osd_printf_verbose(" Modifier Volume\n");
				#endif
				}
				else
				{
				#if DEBUG_PVRDLIST
					osd_printf_verbose(" Polygon\n");
				#endif
				}
			}
			if (paratype == 5)
			{ // quad
				#if DEBUG_PVRDLIST
				osd_printf_verbose(" Sprite\n");
				#endif
			}
		}

		if (paratype == 7)
		{ // vertex
			if ((tafifo_listtype == 1) || (tafifo_listtype == 3))
			{
				#if DEBUG_PVRDLIST
				osd_printf_verbose(" Vertex modifier volume");
				osd_printf_verbose(" A(%f,%f,%f) B(%f,%f,%f) C(%f,%f,%f)", u2f(tafifo_buff[1]), u2f(tafifo_buff[2]),
					u2f(tafifo_buff[3]), u2f(tafifo_buff[4]), u2f(tafifo_buff[5]), u2f(tafifo_buff[6]), u2f(tafifo_buff[7]),
					u2f(tafifo_buff[8]), u2f(tafifo_buff[9]));
				osd_printf_verbose("\n");
				#endif
			}
			else if (global_paratype == 5)
			{
				#if DEBUG_PVRDLIST
				osd_printf_verbose(" Vertex sprite");
				osd_printf_verbose(" A(%f,%f,%f) B(%f,%f,%f) C(%f,%f,%f) D(%f,%f,)", u2f(tafifo_buff[1]), u2f(tafifo_buff[2]),
					u2f(tafifo_buff[3]), u2f(tafifo_buff[4]), u2f(tafifo_buff[5]), u2f(tafifo_buff[6]), u2f(tafifo_buff[7]),
					u2f(tafifo_buff[8]), u2f(tafifo_buff[9]), u2f(tafifo_buff[10]), u2f(tafifo_buff[11]));
				osd_printf_verbose("\n");
				#endif
				if (texture == 1)
				{
					if (rd->verts_size <= 65530)
					{
						strip *ts;
						vert *tv = &rd->verts[rd->verts_size];
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
						tv[0].u = u2f(tafifo_buff[0xd] & 0xffff0000);
						tv[0].v = u2f(tafifo_buff[0xd] << 16);
						tv[1].u = u2f(tafifo_buff[0xe] & 0xffff0000);
						tv[1].v = u2f(tafifo_buff[0xe] << 16);
						tv[3].u = u2f(tafifo_buff[0xf] & 0xffff0000);
						tv[3].v = u2f(tafifo_buff[0xf] << 16);
						tv[2].u = tv[0].u+tv[3].u-tv[1].u;
						tv[2].v = tv[0].v+tv[3].v-tv[1].v;

						ts = &rd->strips[rd->strips_size++];
						tex_get_info(&ts->ti);
						ts->svert = rd->verts_size;
						ts->evert = rd->verts_size + 3;

						rd->verts_size += 4;
					}
				}
			}
			else if (global_paratype == 4)
			{
				#if DEBUG_PVRDLIST
				osd_printf_verbose(" Vertex polygon");
				osd_printf_verbose(" V(%f,%f,%f) T(%f,%f)", u2f(tafifo_buff[1]), u2f(tafifo_buff[2]), u2f(tafifo_buff[3]), u2f(tafifo_buff[4]), u2f(tafifo_buff[5]));
				osd_printf_verbose("\n");
				#endif
				if (rd->verts_size <= 65530)
				{
					/* add a vertex to our list */
					/* this is used for 3d stuff, ie most of the graphics (see guilty gear, confidential mission, maze of the kings etc.) */
					/* -- this is also wildly inaccurate! */
					vert *tv = &rd->verts[rd->verts_size];

					tv->x=u2f(tafifo_buff[1]);
					tv->y=u2f(tafifo_buff[2]);
					tv->w=u2f(tafifo_buff[3]);
					tv->u=u2f(tafifo_buff[4]);
					tv->v=u2f(tafifo_buff[5]);
					if (texture == 0)
					{
						if(coltype == 0)
							nontextured_pal_int=tafifo_buff[6];
						else if(coltype == 1)
						{
							nontextured_fpal_a=u2f(tafifo_buff[4]);
							nontextured_fpal_r=u2f(tafifo_buff[5]);
							nontextured_fpal_g=u2f(tafifo_buff[6]);
							nontextured_fpal_b=u2f(tafifo_buff[7]);
						}
					}

					if((!rd->strips_size) ||
						rd->strips[rd->strips_size-1].evert != -1)
					{
						strip *ts = &rd->strips[rd->strips_size++];
						tex_get_info(&ts->ti);
						ts->svert = rd->verts_size;
						ts->evert = -1;
					}
					if(endofstrip)
						rd->strips[rd->strips_size-1].evert = rd->verts_size;
					rd->verts_size++;
				}
			}
		}
	}
}

WRITE64_MEMBER( powervr2_device::ta_fifo_poly_w )
{
	if (mem_mask == U64(0xffffffffffffffff))    // 64 bit
	{
		tafifo_buff[tafifo_pos]=(UINT32)data;
		tafifo_buff[tafifo_pos+1]=(UINT32)(data >> 32);
		#if DEBUG_FIFO_POLY
		osd_printf_debug("%s",string_format("ta_fifo_poly_w:  Unmapped write64 %08x = %I64x -> %08x %08x\n", 0x10000000+offset*8, data, tafifo_buff[tafifo_pos], tafifo_buff[tafifo_pos+1]).c_str());
		#endif
		tafifo_pos += 2;
	}
	else
	{
		osd_printf_debug("%s",string_format("ta_fifo_poly_w:  Unmapped write64 %08x = %I64x mask %I64x\n", 0x10000000+offset*8, data, mem_mask).c_str());
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


WRITE8_MEMBER( powervr2_device::ta_fifo_yuv_w )
{
	dc_state *state = machine().driver_data<dc_state>();
	//printf("%08x %08x\n",ta_yuv_index++,ta_yuv_tex_ctrl);

	//popmessage("YUV fifo write %08x %08x",ta_yuv_index,ta_yuv_tex_ctrl);

	yuv_fifo[ta_yuv_index] = data;
	ta_yuv_index++;

	if(ta_yuv_index == 0x180)
	{
		ta_yuv_index = 0;
		for(int y=0;y<16;y++)
		{
			for(int x=0;x<16;x+=2)
			{
				int dst_addr;
				int u,v,y0,y1;

				dst_addr = ta_yuv_tex_base;
				dst_addr+= (ta_yuv_x+x)*2;
				dst_addr+= ((ta_yuv_y+y)*320*2);

				u = yuv_fifo[0x00+(x>>1)+((y>>1)*8)];
				v = yuv_fifo[0x40+(x>>1)+((y>>1)*8)];
				y0 = yuv_fifo[0x80+((x&8) ? 0x40 : 0x00)+((y&8) ? 0x80 : 0x00)+(x&6)+((y&7)*8)];
				y1 = yuv_fifo[0x80+((x&8) ? 0x40 : 0x00)+((y&8) ? 0x80 : 0x00)+(x&6)+((y&7)*8)+1];

				*(UINT8 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + BYTE8_XOR_LE(dst_addr)) = u;
				*(UINT8 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + BYTE8_XOR_LE(dst_addr+1)) = y0;
				*(UINT8 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + BYTE8_XOR_LE(dst_addr+2)) = v;
				*(UINT8 *)((reinterpret_cast<UINT8 *>(dc_texture_ram)) + BYTE8_XOR_LE(dst_addr+3)) = y1;
			}
		}

		ta_yuv_x+=16;
		if(ta_yuv_x == ta_yuv_x_size)
		{
			ta_yuv_x = 0;
			ta_yuv_y+=16;
			if(ta_yuv_y == ta_yuv_y_size)
			{
				ta_yuv_y = 0;
				/* TODO: timing */
				yuv_timer_end->adjust(state->m_maincpu->cycles_to_attotime((ta_yuv_x_size/16)*(ta_yuv_y_size/16)*0x180));
			}
		}
	}
}

// SB_LMMODE0
WRITE64_MEMBER(powervr2_device::ta_texture_directpath0_w )
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
WRITE64_MEMBER(powervr2_device::ta_texture_directpath1_w )
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
UINT32 powervr2_device::dilate0(UINT32 value,int bits) // dilate first "bits" bits in "value"
{
	UINT32 x,m1,m2,m3;
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

UINT32 powervr2_device::dilate1(UINT32 value,int bits) // dilate first "bits" bits in "value"
{
	UINT32 x,m1,m2,m3;
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

void powervr2_device::render_hline(bitmap_rgb32 &bitmap, texinfo *ti, int y, float xl, float xr, float ul, float ur, float vl, float vr, float wl, float wr)
{
	int xxl, xxr;
	float dx, ddx, dudx, dvdx, dwdx;
	UINT32 *tdata;
	float *wbufline;

	// untextured cases aren't handled
//  if (!ti->textured) return;

	if(xr < 0 || xl >= 640)
		return;

	xxl = round(xl);
	xxr = round(xr);

	if(xxl == xxr)
		return;

	dx = xr-xl;
	dudx = (ur-ul)/dx;
	dvdx = (vr-vl)/dx;
	dwdx = (wr-wl)/dx;

	if(xxl < 0)
		xxl = 0;
	if(xxr > 640)
		xxr = 640;

	// Target the pixel center
	ddx = xxl + 0.5f - xl;
	ul += ddx*dudx;
	vl += ddx*dvdx;
	wl += ddx*dwdx;


	tdata = &bitmap.pix32(y, xxl);
	wbufline = &wbuffer[y][xxl];

	while(xxl < xxr) {
		if((wl >= *wbufline)) {
			UINT32 c;
			float u = ul/wl;
			float v = vl/wl;

			/*
			if(ti->flip_u)
			{
			    u = ti->sizex - u;
			}

			if(ti->flip_v)
			{
			    v = ti->sizey - v;
			}*/

			c = (this->*(ti->r))(ti, u, v);

			// debug dip to turn on/off bilinear filtering, it's slooooow
			if (debug_dip_status&0x1)
			{
				if(ti->filter_mode >= TEX_FILTER_BILINEAR)
				{
					UINT32 c1 = (this->*(ti->r))(ti, u+1.0f, v);
					UINT32 c2 = (this->*(ti->r))(ti, u+1.0f, v+1.0f);
					UINT32 c3 = (this->*(ti->r))(ti, u, v+1.0f);
					c = bilinear_filter(c, c1, c2, c3, u, v);
				}
			}

			if(c & 0xff000000) {
				*tdata = ti->blend(c, *tdata);
				*wbufline = wl;
			}
		}
		wbufline++;
		tdata++;

		ul += dudx;
		vl += dvdx;
		wl += dwdx;
		xxl ++;
	}
}

void powervr2_device::render_span(bitmap_rgb32 &bitmap, texinfo *ti,
									float y0, float y1,
									float xl, float xr,
									float ul, float ur,
									float vl, float vr,
									float wl, float wr,
									float dxldy, float dxrdy,
									float duldy, float durdy,
									float dvldy, float dvrdy,
									float dwldy, float dwrdy)
{
	float dy;
	int yy0, yy1;

	if(y1 <= 0)
		return;
	if(y1 > 480)
		y1 = 480;

	if(y0 < 0) {
		xl += -dxldy*y0;
		xr += -dxrdy*y0;
		ul += -duldy*y0;
		ur += -durdy*y0;
		vl += -dvldy*y0;
		vr += -dvrdy*y0;
		wl += -dwldy*y0;
		wr += -dwrdy*y0;
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

	while(yy0 < yy1) {
		render_hline(bitmap, ti, yy0, xl, xr, ul, ur, vl, vr, wl, wr);

		xl += dxldy;
		xr += dxrdy;
		ul += duldy;
		ur += durdy;
		vl += dvldy;
		vr += dvrdy;
		wl += dwldy;
		wr += dwrdy;
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


void powervr2_device::render_tri_sorted(bitmap_rgb32 &bitmap, texinfo *ti, const vert *v0, const vert *v1, const vert *v2)
{
	float dy01, dy02, dy12;

	float dx01dy, dx02dy, dx12dy, du01dy, du02dy, du12dy, dv01dy, dv02dy, dv12dy, dw01dy, dw02dy, dw12dy;

	if(v0->y >= 480 || v2->y < 0)
		return;

	dy01 = v1->y - v0->y;
	dy02 = v2->y - v0->y;
	dy12 = v2->y - v1->y;

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
			render_span(bitmap, ti, v1->y, v2->y, v0->x, v1->x, v0->u, v1->u, v0->v, v1->v, v0->w, v1->w, dx02dy, dx12dy, du02dy, du12dy, dv02dy, dv12dy, dw02dy, dw12dy);
		else
			render_span(bitmap, ti, v1->y, v2->y, v1->x, v0->x, v1->u, v0->u, v1->v, v0->v, v1->w, v0->w, dx12dy, dx02dy, du12dy, du02dy, dv12dy, dv02dy, dw12dy, dw02dy);

	} else if(!dy12) {
		if(v2->x > v1->x)
			render_span(bitmap, ti, v0->y, v1->y, v0->x, v0->x, v0->u, v0->u, v0->v, v0->v, v0->w, v0->w, dx01dy, dx02dy, du01dy, du02dy, dv01dy, dv02dy, dw01dy, dw02dy);
		else
			render_span(bitmap, ti, v0->y, v1->y, v0->x, v0->x, v0->u, v0->u, v0->v, v0->v, v0->w, v0->w, dx02dy, dx01dy, du02dy, du01dy, dv02dy, dv01dy, dw02dy, dw01dy);

	} else {
		if(dx01dy < dx02dy) {
			render_span(bitmap, ti, v0->y, v1->y,
						v0->x, v0->x, v0->u, v0->u, v0->v, v0->v, v0->w, v0->w,
						dx01dy, dx02dy, du01dy, du02dy, dv01dy, dv02dy, dw01dy, dw02dy);
			render_span(bitmap, ti, v1->y, v2->y,
						v1->x, v0->x + dx02dy*dy01, v1->u, v0->u + du02dy*dy01, v1->v, v0->v + dv02dy*dy01, v1->w, v0->w + dw02dy*dy01,
						dx12dy, dx02dy, du12dy, du02dy, dv12dy, dv02dy, dw12dy, dw02dy);
		} else {
			render_span(bitmap, ti, v0->y, v1->y,
						v0->x, v0->x, v0->u, v0->u, v0->v, v0->v, v0->w, v0->w,
						dx02dy, dx01dy, du02dy, du01dy, dv02dy, dv01dy, dw02dy, dw01dy);
			render_span(bitmap, ti, v1->y, v2->y,
						v0->x + dx02dy*dy01, v1->x, v0->u + du02dy*dy01, v1->u, v0->v + dv02dy*dy01, v1->v, v0->w + dw02dy*dy01, v1->w,
						dx02dy, dx12dy, du02dy, du12dy, dv02dy, dv12dy, dw02dy, dw12dy);
		}
	}
}

void powervr2_device::render_tri(bitmap_rgb32 &bitmap, texinfo *ti, const vert *v)
{
	int i0, i1, i2;

	sort_vertices(v, &i0, &i1, &i2);
	render_tri_sorted(bitmap, ti, v+i0, v+i1, v+i2);
}

void powervr2_device::render_to_accumulation_buffer(bitmap_rgb32 &bitmap,const rectangle &cliprect)
{
	dc_state *state = machine().driver_data<dc_state>();
	address_space &space = state->m_maincpu->space(AS_PROGRAM);
#if 0
	int stride;
	UINT16 *bmpaddr16;
	UINT32 k;
#endif


	if (renderselect < 0)
		return;

	//printf("drawtest!\n");

	int rs=renderselect;
	UINT32 c=space.read_dword(0x05000000+((isp_backgnd_t & 0xfffff8)>>1)+(3+3)*4);
	bitmap.fill(c, cliprect);


	int ns=grab[rs].strips_size;
	if(ns)
		memset(wbuffer, 0x00, sizeof(wbuffer));

	for (int cs=0;cs < ns;cs++)
	{
		strip *ts = &grab[rs].strips[cs];
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
				render_tri(bitmap, &ts->ti, grab[rs].verts + i);

		}
	}
	grab[rs].busy=0;
}

// copies the accumulation buffer into the framebuffer, converting to the specified format
// not accurate, ignores field stuff and just uses SOF1 for now
// also ignores scale effects (can scale accumulation buffer to half size with filtering etc.)
// also can specify dither etc.
// basically, just a crude implementation!

/* 0555KRGB = 0 */
void powervr2_device::fb_convert_0555krgb_to_555rgb(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT16 newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000f800) >> 11)) << 5)  |
							((((data & 0x00f80000) >> 19)) << 10);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_0555krgb_to_565rgb(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT16 newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000f800) >> 11)) << 5)  |
							((((data & 0x00f80000) >> 19)) << 11);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_0555krgb_to_888rgb24(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*3);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT32 newdat = (data & 0xf8f8f8);

			space.write_byte(realwriteoffs+xcnt*3+0, newdat >> 0);
			space.write_byte(realwriteoffs+xcnt*3+1, newdat >> 8);
			space.write_byte(realwriteoffs+xcnt*3+2, newdat >> 16);
		}
	}
}

void powervr2_device::fb_convert_0555krgb_to_888rgb32(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*4);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT32 newdat = (data & 0xf8f8f8);

			space.write_dword(realwriteoffs+xcnt*4, newdat);
		}
	}
}

/* 0565RGB = 1 */
void powervr2_device::fb_convert_0565rgb_to_555rgb(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT16 newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000fc00) >> 10)) << 5)  |
							((((data & 0x00f80000) >> 19)) << 10);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_0565rgb_to_565rgb(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT16 newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000fc00) >> 10)) << 5)  |
							((((data & 0x00f80000) >> 19)) << 11);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_0565rgb_to_888rgb24(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*3);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT32 newdat = (data & 0xf8fcf8);

			space.write_byte(realwriteoffs+xcnt*3+0, newdat >> 0);
			space.write_byte(realwriteoffs+xcnt*3+1, newdat >> 8);
			space.write_byte(realwriteoffs+xcnt*3+2, newdat >> 16);
		}
	}
}

void powervr2_device::fb_convert_0565rgb_to_888rgb32(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*4);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT32 newdat = (data & 0xf8fcf8);

			space.write_dword(realwriteoffs+xcnt*4, newdat);
		}
	}
}

/* 1555ARGB = 3 */
void powervr2_device::fb_convert_1555argb_to_555rgb(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT16 newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000f800) >> 11)) << 5)  |
							((((data & 0x00f80000) >> 19)) << 10);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_1555argb_to_565rgb(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT16 newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000f800) >> 11)) << 5)  |
							((((data & 0x00f80000) >> 19)) << 11);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_1555argb_to_888rgb24(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*3);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT32 newdat = (data & 0xf8f8f8);

			space.write_byte(realwriteoffs+xcnt*3+0, newdat >> 0);
			space.write_byte(realwriteoffs+xcnt*3+1, newdat >> 8);
			space.write_byte(realwriteoffs+xcnt*3+2, newdat >> 16);
		}
	}
}

void powervr2_device::fb_convert_1555argb_to_888rgb32(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*4);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT32 newdat = (data & 0xf8f8f8);

			space.write_dword(realwriteoffs+xcnt*4, newdat);
		}
	}
}

/* 888RGB = 4 */
void powervr2_device::fb_convert_888rgb_to_555rgb(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT16 newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000f800) >> 11)) << 5)  |
							((((data & 0x00f80000) >> 16)) << 10);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_888rgb_to_565rgb(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT16 newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000fc00) >> 11)) << 5)  |
							((((data & 0x00f80000) >> 16)) << 11);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_888rgb_to_888rgb24(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*3);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT32 newdat = (data & 0xffffff);

			space.write_byte(realwriteoffs+xcnt*3+0, newdat >> 0);
			space.write_byte(realwriteoffs+xcnt*3+1, newdat >> 8);
			space.write_byte(realwriteoffs+xcnt*3+2, newdat >> 16);
		}
	}
}

void powervr2_device::fb_convert_888rgb_to_888rgb32(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*4);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT32 newdat = (data & 0xffffff);

			space.write_dword(realwriteoffs+xcnt*4, newdat);
		}
	}
}


/* 8888ARGB = 6 */
void powervr2_device::fb_convert_8888argb_to_555rgb(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT16 newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000f800) >> 11)) << 5)  |
							((((data & 0x00f80000) >> 16)) << 10);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_8888argb_to_565rgb(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*2);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT16 newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
							((((data & 0x0000fc00) >> 11)) << 5)  |
							((((data & 0x00f80000) >> 16)) << 11);

			space.write_word(realwriteoffs+xcnt*2, newdat);
		}
	}
}

void powervr2_device::fb_convert_8888argb_to_888rgb24(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*3);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT32 newdat = (data & 0xffffff);

			space.write_byte(realwriteoffs+xcnt*3+0, newdat >> 0);
			space.write_byte(realwriteoffs+xcnt*3+1, newdat >> 8);
			space.write_byte(realwriteoffs+xcnt*3+2, newdat >> 16);
		}
	}
}

void powervr2_device::fb_convert_8888argb_to_888rgb32(address_space &space, int x,int y)
{
	int xcnt,ycnt;
	for (ycnt=0;ycnt<32;ycnt++)
	{
		UINT32 realwriteoffs = 0x05000000 + fb_w_sof1 + (y+ycnt) * (fb_w_linestride<<3) + (x*4);
		UINT32 *src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);

		for (xcnt=0;xcnt<32;xcnt++)
		{
			// data starts in 8888 format, downsample it
			UINT32 data = src[xcnt];
			UINT32 newdat = (data & 0xffffff);

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

	UINT8 packmode = fb_w_ctrl & 0x7;
	UINT8 unpackmode = (fb_r_ctrl & 0x0000000c) >>2;  // aka fb_depth

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
			printf("pvr_accumulationbuffer_to_framebuffer buffer to tile at %d,%d - unsupported pack mode %02x (4444 ARGB)\n",x,y,packmode);
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

		case 0x05:
			printf("pvr_accumulationbuffer_to_framebuffer buffer to tile at %d,%d - unsupported pack mode %02x (0888 KGB 32-bit)\n",x,y,packmode);
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
			printf("pvr_accumulationbuffer_to_framebuffer buffer to tile at %d,%d - unsupported pack mode %02x (Reserved! Don't Use!)\n",x,y,packmode);
			break;
	}


}

void powervr2_device::pvr_drawframebuffer(bitmap_rgb32 &bitmap,const rectangle &cliprect)
{
	int x,y,dy,xi;
	UINT32 addrp;
	UINT32 *fbaddr;
	UINT32 c;
	UINT32 r,g,b;
	UINT8 interlace_on = ((spg_control & 0x10) >> 4);
	INT32 ystart_f1 = (vo_starty & 0x3ff) << interlace_on;
	//INT32 ystart_f2 = (vo_starty >> 16) & 0x3ff;
	INT32 hstart = (vo_startx & 0x3ff);
	int res_x,res_y;
//  rectangle fbclip;

	UINT8 unpackmode = (fb_r_ctrl & 0x0000000c) >>2;  // aka fb_depth
	UINT8 enable = (fb_r_ctrl & 0x00000001);

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
			for (y=0;y <= dy;y++)
			{
				addrp = fb_r_sof1+y*xi*2;
				if(vo_control & 0x100)
				{
					for (x=0;x < xi;x++)
					{
						res_x = x*2+0 + hstart;
						res_y = y + ystart_f1;

						fbaddr=&bitmap.pix32(res_y, res_x);
						c=*((reinterpret_cast<UINT16 *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 1));

						b = (c & 0x001f) << 3;
						g = (c & 0x03e0) >> 2;
						r = (c & 0x7c00) >> 7;

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						res_x = x*2+1 + hstart;

						fbaddr=&bitmap.pix32(res_y, res_x);
						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);
						addrp+=2;
					}
				}
				else
				{
					for (x=0;x < xi;x++)
					{
						res_x = x + hstart;
						res_y = y + ystart_f1;

						fbaddr=&bitmap.pix32(res_y, res_x);
						c=*((reinterpret_cast<UINT16 *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 1));

						b = (c & 0x001f) << 3;
						g = (c & 0x03e0) >> 2;
						r = (c & 0x7c00) >> 7;

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);
						addrp+=2;
					}
				}
			}

			break;
		case 0x01: // 0565 RGB 16-bit
			// should upsample back to 8-bit output using fb_concat
			for (y=0;y <= dy;y++)
			{
				addrp = fb_r_sof1+y*xi*2;
				if(vo_control & 0x100)
				{
					for (x=0;x < xi;x++)
					{
						res_x = x*2+0 + hstart;
						res_y = y + ystart_f1;
						fbaddr=&bitmap.pix32(res_y, res_x);
						c=*((reinterpret_cast<UINT16 *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 1));

						b = (c & 0x001f) << 3;
						g = (c & 0x07e0) >> 3;
						r = (c & 0xf800) >> 8;

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						res_x = x*2+1 + hstart;
						//res_y = y + ystart_f1;
						fbaddr=&bitmap.pix32(res_y, res_x);

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						addrp+=2;
					}
				}
				else
				{
					for (x=0;x < xi;x++)
					{
						res_x = x + hstart;
						res_y = y + ystart_f1;
						fbaddr=&bitmap.pix32(res_y, res_x);
						c=*((reinterpret_cast<UINT16 *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 1));

						b = (c & 0x001f) << 3;
						g = (c & 0x07e0) >> 3;
						r = (c & 0xf800) >> 8;

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);
						addrp+=2;
					}
				}
			}
			break;

		case 0x02: ; // 888 RGB 24-bit - suchie3, Soul Calibur
			for (y=0;y <= dy;y++)
			{
				addrp = fb_r_sof1+y*xi*2;

				if(vo_control & 0x100)
				{
					for (x=0;x < xi;x++)
					{
						res_x = x*2+0 + hstart;
						res_y = y + ystart_f1;

						fbaddr=&bitmap.pix32(res_y, res_x);

						c =*(UINT8 *)((reinterpret_cast<UINT8 *>(dc_framebuffer_ram)) + BYTE8_XOR_LE(addrp));
						b = c;

						c =*(UINT8 *)((reinterpret_cast<UINT8 *>(dc_framebuffer_ram)) + BYTE8_XOR_LE(addrp+1));
						g = c;

						c =*(UINT8 *)((reinterpret_cast<UINT8 *>(dc_framebuffer_ram)) + BYTE8_XOR_LE(addrp+2));
						r = c;

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						res_x = x*2+1 + hstart;

						fbaddr=&bitmap.pix32(res_y, res_x);
						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						addrp+=3;
					}
				}
				else
				{
					for (x=0;x < xi;x++)
					{
						res_x = x + hstart;
						res_y = y + ystart_f1;
						fbaddr=&bitmap.pix32(res_y, res_x);

						c =*(UINT8 *)((reinterpret_cast<UINT8 *>(dc_framebuffer_ram)) + BYTE8_XOR_LE(addrp));
						b = c;

						c =*(UINT8 *)((reinterpret_cast<UINT8 *>(dc_framebuffer_ram)) + BYTE8_XOR_LE(addrp+1));
						g = c;

						c =*(UINT8 *)((reinterpret_cast<UINT8 *>(dc_framebuffer_ram)) + BYTE8_XOR_LE(addrp+2));
						r = c;

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						addrp+=3;
					}
				}
			}
			break;

		case 0x03:        // 0888 ARGB 32-bit
			for (y=0;y <= dy;y++)
			{
				addrp = fb_r_sof1+y*xi*2;

				if(vo_control & 0x100)
				{
					for (x=0;x < xi;x++)
					{
						res_x = x*2+0 + hstart;
						res_y = y + ystart_f1;

						fbaddr=&bitmap.pix32(res_y, res_x);
						c =*((reinterpret_cast<UINT32 *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 2));

						b = (c & 0x0000ff) >> 0;
						g = (c & 0x00ff00) >> 8;
						r = (c & 0xff0000) >> 16;

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						res_x = x*2+1 + hstart;

						fbaddr=&bitmap.pix32(res_y, res_x);
						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						addrp+=4;
					}
				}
				else
				{
					for (x=0;x < xi;x++)
					{
						res_x = x + hstart;
						res_y = y + ystart_f1;
						fbaddr=&bitmap.pix32(res_y, res_x);
						c =*((reinterpret_cast<UINT32 *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 2));

						b = (c & 0x0000ff) >> 0;
						g = (c & 0x00ff00) >> 8;
						r = (c & 0xff0000) >> 16;

						if (cliprect.contains(res_x, res_y))
							*fbaddr = b | (g<<8) | (r<<16);

						addrp+=4;
					}
				}
			}
			break;
	}
}


#if DEBUG_PALRAM
void powervr2_device::debug_paletteram()
{
	UINT64 pal;
	UINT32 r,g,b;
	int i;

	//popmessage("%02x",pal_ram_ctrl);

	for(i=0;i<0x400;i++)
	{
		pal = palette[i];
		switch(pal_ram_ctrl)
		{
			case 0: //argb1555 <- guilty gear uses this mode
			{
				//a = (pal & 0x8000)>>15;
				r = (pal & 0x7c00)>>10;
				g = (pal & 0x03e0)>>5;
				b = (pal & 0x001f)>>0;
				//a = a ? 0xff : 0x00;
				m_palette->set_pen_color(i, pal5bit(r), pal5bit(g), pal5bit(b));
			}
			break;
			case 1: //rgb565
			{
				//a = 0xff;
				r = (pal & 0xf800)>>11;
				g = (pal & 0x07e0)>>5;
				b = (pal & 0x001f)>>0;
				m_palette->set_pen_color(i, pal5bit(r), pal6bit(g), pal5bit(b));
			}
			break;
			case 2: //argb4444
			{
				//a = (pal & 0xf000)>>12;
				r = (pal & 0x0f00)>>8;
				g = (pal & 0x00f0)>>4;
				b = (pal & 0x000f)>>0;
				m_palette->set_pen_color(i, pal4bit(r), pal4bit(g), pal4bit(b));
			}
			break;
			case 3: //argb8888
			{
				//a = (pal & 0xff000000)>>20;
				r = (pal & 0x00ff0000)>>16;
				g = (pal & 0x0000ff00)>>8;
				b = (pal & 0x000000ff)>>0;
				m_palette->set_pen_color(i, r, g, b);
			}
			break;
		}
	}
}
#endif

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

TIMER_CALLBACK_MEMBER(powervr2_device::vbin)
{
	irq_cb(VBL_IN_IRQ);

	//popmessage("VII %d VOI %d VI %d VO %d VS %d",spg_vblank_int & 0x3ff,(spg_vblank_int >> 16) & 0x3ff,spg_vblank & 0x3ff,(spg_vblank >> 16) & 0x3ff,(spg_load >> 16) & 0x3ff);
//  vbin_timer->adjust(m_screen->time_until_pos(spg_vblank_int & 0x3ff));
}

TIMER_CALLBACK_MEMBER(powervr2_device::vbout)
{
	irq_cb(VBL_OUT_IRQ);

//  vbout_timer->adjust(m_screen->time_until_pos((spg_vblank_int >> 16) & 0x3ff));
}

TIMER_CALLBACK_MEMBER(powervr2_device::hbin)
{
	if(spg_hblank_int & 0x1000)
	{
		if(scanline == next_y)
		{
			irq_cb(HBL_IN_IRQ);
			next_y += spg_hblank_int & 0x3ff;
		}
	}
	else if((scanline == (spg_hblank_int & 0x3ff)) || (spg_hblank_int & 0x2000))
	{
		irq_cb(HBL_IN_IRQ);
	}

//  printf("hbin on scanline %d\n",scanline);

	scanline++;

	if(scanline >= ((spg_load >> 16) & 0x3ff))
	{
		scanline = 0;
		next_y = spg_hblank_int & 0x3ff;
	}

	hbin_timer->adjust(m_screen->time_until_pos(scanline, ((spg_hblank_int >> 16) & 0x3ff)-1));
}



TIMER_CALLBACK_MEMBER(powervr2_device::endofrender_video)
{
	printf("VIDEO END %d\n",m_screen->vpos());
//  endofrender_timer_video->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(powervr2_device::endofrender_tsp)
{
	printf("TSP END %d\n",m_screen->vpos());

//  endofrender_timer_tsp->adjust(attotime::never);
//  endofrender_timer_video->adjust(attotime::from_usec(500) );
}

TIMER_CALLBACK_MEMBER(powervr2_device::endofrender_isp)
{
	irq_cb(EOR_ISP_IRQ); // ISP end of render
	irq_cb(EOR_TSP_IRQ); // TSP end of render
	irq_cb(EOR_VIDEO_IRQ); // VIDEO end of render

//  printf("ISP END %d\n",m_screen->vpos());

	endofrender_timer_isp->adjust(attotime::never);
//  endofrender_timer_tsp->adjust(attotime::from_usec(500) );
}

UINT32 powervr2_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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
//  int y,x;

#if DEBUG_PALRAM
	debug_paletteram();
#endif

	// copy our fake framebuffer bitmap (where things have been rendered) to the screen
#if 0
	for (y = visarea->min_y ; y <= visarea->max_y ; y++)
	{
		for (x = visarea->min_x ; x <= visarea->max_x ; x++)
		{
			UINT32* src = &fake_accumulationbuffer_bitmap->pix32(y, x);
			UINT32* dst = &bitmap.pix32(y, x);
			dst[0] = src[0];
		}
	}
#endif

	bitmap.fill(rgb_t(0xff,
							(vo_border_col >> 16) & 0xff,
							(vo_border_col >> 8 ) & 0xff,
							(vo_border_col      ) & 0xff), cliprect); //FIXME: Chroma bit?

	if(!(vo_control & 8))
		pvr_drawframebuffer(bitmap, cliprect);

	// update this here so we only do string lookup once per frame
	debug_dip_status = m_mamedebug->read();

	return 0;
}


/* Naomi 2 attempts (TBD) */

READ32_MEMBER( powervr2_device::pvr2_ta_r )
{
	printf("PVR2 %08x R\n", offset);

	return 0;
}

WRITE32_MEMBER( powervr2_device::pvr2_ta_w )
{
//  int reg;
//  UINT64 shift;
//  UINT32 dat;

//  reg = decode_reg_64(offset, mem_mask, &shift);
//  dat = (UINT32)(data >> shift);

	//printf("PVR2 %08x %08x\n",reg,dat);
}

READ32_MEMBER( powervr2_device::elan_regs_r )
{
	switch(offset)
	{
		case 0x00/4: // ID chip (TODO: BIOS crashes / gives a black screen with this as per now!)
			return 0xe1ad0000;
		case 0x04/4: // REVISION
			return 0x12; //or 0x01?
		case 0x10/4: // SH4 interface control (???)
			/* ---- -x-- enable second PVR */
			/* ---- --x- elan has channel 2 */
			/* ---- ---x broadcast on cs1 (?) */
			return 6;
		case 0x14/4: // SDRAM refresh register
			return 0x2029; //default 0x1429
		case 0x1c/4: // SDRAM CFG
			return 0xa7320961; //default 0xa7320961
		case 0x30/4: // Macro tiler configuration, bit 0 is enable
			return 0;
		case 0x74/4: // IRQ STAT
			return 0;
		case 0x78/4: // IRQ MASK
			return 0;
		default:
			printf("%08x %08x\n",space.device().safe_pc(),offset*4);
			break;
	}

	return 0;
}

WRITE32_MEMBER( powervr2_device::elan_regs_w )
{
	switch(offset)
	{
		default:
			printf("%08x %08x %08x W\n",space.device().safe_pc(),offset*4,data);
			break;
	}
}


WRITE32_MEMBER( powervr2_device::pvrs_ta_w )
{
	//  pvr_ta_w(space,offset,data,mem_mask);
	//  pvr2_ta_w(space,offset,data,mem_mask);
	//printf("PVR2 %08x %08x\n",reg,dat);
}

TIMER_CALLBACK_MEMBER(powervr2_device::pvr_dma_irq)
{
	m_pvr_dma.start = sb_pdst = 0;
	irq_cb(DMA_PVR_IRQ);
}

void powervr2_device::pvr_dma_execute(address_space &space)
{
	dc_state *state = machine().driver_data<dc_state>();
	UINT32 src,dst,size;
	dst = m_pvr_dma.pvr_addr;
	src = m_pvr_dma.sys_addr;
	size = 0;

	/* used so far by usagui and sprtjam*/
	printf("PVR-DMA start\n");
	printf("%08x %08x %08x\n",m_pvr_dma.pvr_addr,m_pvr_dma.sys_addr,m_pvr_dma.size);
	printf("src %s dst %08x\n",m_pvr_dma.dir ? "->" : "<-",m_pvr_dma.sel);

	/* Throw illegal address set */
	#if 0
	if((m_pvr_dma.sys_addr & 0x1c000000) != 0x0c000000)
	{
		/* TODO: timing */
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
		for(;size<m_pvr_dma.size;size+=4)
		{
			space.write_dword(dst,space.read_dword(src));
			src+=4;
			dst+=4;
		}
	}
	else
	{
		for(;size<m_pvr_dma.size;size+=4)
		{
			space.write_dword(src,space.read_dword(dst));
			src+=4;
			dst+=4;
		}
	}
	/* Note: do not update the params, since this DMA type doesn't support it. */
	/* TODO: timing of this */
	machine().scheduler().timer_set(state->m_maincpu->cycles_to_attotime(m_pvr_dma.size/4), timer_expired_delegate(FUNC(powervr2_device::pvr_dma_irq), this));
}

powervr2_device::powervr2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, POWERVR2, "PowerVR 2", tag, owner, clock, "powervr2", __FILE__),
		device_video_interface(mconfig, *this),
		irq_cb(*this),
		m_mamedebug(*this, ":MAMEDEBUG")
{
}

void powervr2_device::device_start()
{
	irq_cb.resolve_safe();

	memset(grab, 0, sizeof(grab));
	pvr_build_parameterconfig();

	computedilated();

//  vbout_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(powervr2_device::vbout),this));
//  vbin_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(powervr2_device::vbin),this));
	hbin_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(powervr2_device::hbin),this));
	yuv_timer_end = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(powervr2_device::yuv_convert_end),this));

	endofrender_timer_isp = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(powervr2_device::endofrender_isp),this));
	endofrender_timer_tsp = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(powervr2_device::endofrender_tsp),this));
	endofrender_timer_video = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(powervr2_device::endofrender_video),this));

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

	tafifo_pos=0;
	tafifo_mask=7;
	tafifo_vertexwords=8;
	tafifo_listtype= -1;
	start_render_received=0;
	renderselect= -1;
	grabsel=0;

//  vbout_timer->adjust(m_screen->time_until_pos((spg_vblank_int >> 16) & 0x3ff));
//  vbin_timer->adjust(m_screen->time_until_pos(spg_vblank_int & 0x3ff));
	hbin_timer->adjust(m_screen->time_until_pos(0, ((spg_hblank_int >> 16) & 0x3ff)-1));

	scanline = 0;
	next_y = 0;

	endofrender_timer_isp->adjust(attotime::never);
	endofrender_timer_tsp->adjust(attotime::never);
	endofrender_timer_video->adjust(attotime::never);
	yuv_timer_end->adjust(attotime::never);

	dc_state *state = machine().driver_data<dc_state>();
	dc_texture_ram = state->dc_texture_ram.target();
	dc_framebuffer_ram = state->dc_framebuffer_ram.target();
}

/* called by TIMER_ADD_PERIODIC, in driver sections (controlled by SPG, that's a PVR sub-device) */
void powervr2_device::pvr_scanline_timer(int vpos)
{
	int vbin_line = spg_vblank_int & 0x3ff;
	int vbout_line = (spg_vblank_int >> 16) & 0x3ff;
	UINT8 interlace_on = ((spg_control & 0x10) >> 4);
	dc_state *state = machine().driver_data<dc_state>();

	vbin_line <<= interlace_on;
	vbout_line <<= interlace_on;

	if(vbin_line-(1+interlace_on) == vpos)
		state->m_maple->maple_hw_trigger();

	if(vbin_line == vpos)
		irq_cb(VBL_IN_IRQ);

	if(vbout_line == vpos)
		irq_cb(VBL_OUT_IRQ);
}
