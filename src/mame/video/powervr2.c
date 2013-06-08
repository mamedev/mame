/*
    dc.c - Dreamcast video emulation

*/

#include "emu.h"
#include "powervr2.h"
#include "includes/dc.h"
#include "cpu/sh4/sh4.h"
#include "render.h"
#include "rendutil.h"
#include "video/rgbutil.h"

const device_type POWERVR2 = &device_creator<powervr2_device>;

const int powervr2_device::pvr_parconfseq[] = {1,2,3,2,3,4,5,6,5,6,7,8,9,10,11,12,13,14,13,14,15,16,17,16,17,0,0,0,0,0,18,19,20,19,20,21,22,23,22,23};
const int powervr2_device::pvr_wordsvertex[24]  = {8,8,8,8,8,16,16,8,8,8, 8, 8,8,8,8,8,16,16, 8,16,16,8,16,16};
const int powervr2_device::pvr_wordspolygon[24] = {8,8,8,8,8, 8, 8,8,8,8,16,16,8,8,8,8, 8, 8,16,16,16,8, 8, 8};

#define DEBUG_FIFO_POLY (0)
#define DEBUG_PVRTA (0)
#define DEBUG_PVRTA_REGS (0)
#define DEBUG_PVRDLIST  (0)
#define DEBUG_PALRAM (1)
#define DEBUG_PVRCTRL   (0)

/* PVR TA macro defines */
/*
SPG_HBLANK_INT
---- --xx xxxx xxxx ---- ---- ---- ---- hblank_in_interrupt
---- ---- ---- ---- --xx ---- ---- ---- hblank_int_mode
---- ---- ---- ---- ---- --xx xxxx xxxx line_comp_val
*/
#define spg_hblank_in_irq   ((pvrta_regs[SPG_HBLANK_INT] & 0x03ff0000) >> 16)
#define spg_hblank_in_irq_new   ((pvrta_regs[SPG_HBLANK_INT] & 0x03ff0000) >> 16)
#define spg_hblank_int_mode ((pvrta_regs[SPG_HBLANK_INT] & 0x00003000) >> 12)
#define spg_line_comp_val   ((pvrta_regs[SPG_HBLANK_INT] & 0x000003ff) >> 0)

/*
SPG_VBLANK_INT
---- --xx xxxx xxxx ---- ---- ---- ---- vblank_out_interrupt_line_number
---- ---- ---- ---- ---- --xx xxxx xxxx vblank_in_interrupt_line_number
*/
#define spg_vblank_out_irq_line_num ((pvrta_regs[SPG_VBLANK_INT] & 0x03ff0000) >> 16)
#define spg_vblank_in_irq_line_num  ((pvrta_regs[SPG_VBLANK_INT] & 0x000003ff) >> 0)
#define spg_vblank_out_irq_line_num_new ((pvrta_regs[SPG_VBLANK_INT] & 0x03ff0000) >> 16)
#define spg_vblank_in_irq_line_num_new  ((pvrta_regs[SPG_VBLANK_INT] & 0x000003ff) >> 0)


/*
VO_BORDER_COL
---- ---x ---- ---- ---- ---- ---- ---- Chroma ;suchie3 sets 0xff there, maybe it's 8 bits too?
---- ---- xxxx xxxx ---- ---- ---- ---- Red
---- ---- ---- ---- xxxx xxxx ---- ---- Green
---- ---- ---- ---- ---- ---- xxxx xxxx Blue
*/
#define vo_border_K ((pvrta_regs[VO_BORDER_COL] & 0x01000000) >> 24)
#define vo_border_R ((pvrta_regs[VO_BORDER_COL] & 0x00ff0000) >> 16)
#define vo_border_G ((pvrta_regs[VO_BORDER_COL] & 0x0000ff00) >> 8)
#define vo_border_B ((pvrta_regs[VO_BORDER_COL] & 0x000000ff) >> 0)

/*
SPG_HBLANK
---- ---- --xx xxxx xxxx ---- ---- ---- hbend
---- ---- ---- ---- ---- --xx xxxx xxxx hbstart
*/
#define spg_hbend    ((pvrta_regs[SPG_HBLANK] & 0x03ff0000) >> 16)
#define spg_hbstart  ((pvrta_regs[SPG_HBLANK] & 0x000003ff) >> 0)


/*
SPG_LOAD
---- ---- --xx xxxx xxxx ---- ---- ---- vcount
---- ---- ---- ---- ---- --xx xxxx xxxx hcount
*/
#define spg_vcount   ((pvrta_regs[SPG_LOAD] & 0x03ff0000) >> 16)
#define spg_hcount   ((pvrta_regs[SPG_LOAD] & 0x000003ff) >> 0)

/*
SPG_VBLANK
---- ---- --xx xxxx xxxx ---- ---- ---- vbend
---- ---- ---- ---- ---- --xx xxxx xxxx vbstart
*/
#define spg_vbend    ((pvrta_regs[SPG_VBLANK] & 0x03ff0000) >> 16)
#define spg_vbstart  ((pvrta_regs[SPG_VBLANK] & 0x000003ff) >> 0)


/*
VO_CONTROL
---- ---- --xx xxxx ---- ---- ---- ---- pclk_delay
---- ---- ---- ---- ---- ---x ---- ---- pixel_double ;used in test mode
---- ---- ---- ---- ---- ---- xxxx ---- field_mode
---- ---- ---- ---- ---- ---- ---- x--- blank_video
---- ---- ---- ---- ---- ---- ---- -x-- blank_pol
---- ---- ---- ---- ---- ---- ---- --x- vsync_pol
---- ---- ---- ---- ---- ---- ---- ---x hsync_pol
*/
#define spg_pclk_delay   ((pvrta_regs[VO_CONTROL] & 0x003f0000) >> 16)
#define spg_pixel_double ((pvrta_regs[VO_CONTROL] & 0x00000100) >> 8)
#define spg_field_mode   ((pvrta_regs[VO_CONTROL] & 0x000000f0) >> 4)
#define spg_blank_video  ((pvrta_regs[VO_CONTROL] & 0x00000008) >> 3)
#define spg_blank_pol    ((pvrta_regs[VO_CONTROL] & 0x00000004) >> 2)
#define spg_vsync_pol    ((pvrta_regs[VO_CONTROL] & 0x00000002) >> 1)
#define spg_hsync_pol    ((pvrta_regs[VO_CONTROL] & 0x00000001) >> 0)

/*
VO_STARTX
---- ---- ---- ---- ---- ---x xxxx xxxx horzontal start position
*/
#define vo_horz_start_pos ((pvrta_regs[VO_STARTX] & 0x000003ff) >> 0)

/*
VO_STARTY
---- ---x xxxx xxxx ---- ---- ---- ---- vertical start position on field 2
---- ---- ---- ---- ---- ---x xxxx xxxx vertical start position on field 1
*/

#define vo_vert_start_pos_f2 ((pvrta_regs[VO_STARTY] & 0x03ff0000) >> 16)
#define vo_vert_start_pos_f1 ((pvrta_regs[VO_STARTY] & 0x000003ff) >> 0)

/*
SPG_STATUS
---- ---- ---- ---- --x- ---- ---- ---- vsync
---- ---- ---- ---- ---x ---- ---- ---- hsync
---- ---- ---- ---- ---- x--- ---- ---- blank
---- ---- ---- ---- ---- -x-- ---- ---- field number
---- ---- ---- ---- ---- --xx xxxx xxxx state->scanline
*/


// Perform a standard bilinear filter across four pixels
inline INT32 powervr2_device::clamp(INT32 in, INT32 min, INT32 max)
{
	if(in < min) return min;
	if(in > max) return max;
	return in;
}

inline UINT32 powervr2_device::bilinear_filter(UINT32 c0, UINT32 c1, UINT32 c2, UINT32 c3, float u, float v)
{
	UINT32 ui = (u * 256.0);
	UINT32 vi = (v * 256.0);
	return rgba_bilinear_filter(c0, c1, c3, c2, ui, vi);
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
		(((c1 & 0x0000ff00)*(c2 & 0x0000ff00) & 0x00ff0000) >> 8);
	c1 >>= 16;
	c2 >>= 16;
	cr |=
		(((c1 & 0x000000ff)*(c2 & 0x000000ff) & 0x0000ff00) << 8)  |
		(((c1 & 0x0000ff00)*(c2 & 0x0000ff00) & 0x00ff0000) << 8);
	return cr;
}

// Per-component multiply with 1-color value
inline UINT32 powervr2_device::blic(UINT32 c1, UINT32 c2)
{
	UINT32 cr =
		(((c1 & 0x000000ff)*(0x00100-(c2 & 0x000000ff)) & 0x0000ff00) >> 8)  |
		(((c1 & 0x0000ff00)*(0x10000-(c2 & 0x0000ff00)) & 0x00ff0000) >> 8);
	c1 >>= 16;
	c2 >>= 16;
	cr |=
		(((c1 & 0x000000ff)*(0x00100-(c2 & 0x000000ff)) & 0x0000ff00) << 8)  |
		(((c1 & 0x0000ff00)*(0x10000-(c2 & 0x0000ff00)) & 0x00ff0000) << 8);
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

UINT32 powervr2_device::tex_r_p4_1555_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int off = dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int addrp = t->address + (off >> 1);
	int c = ((reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] >> ((off & 1) << 2)) & 0xf;
	return cv_1555(pvrta_regs[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p4_1555_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] & 0xf;
	return cv_1555(pvrta_regs[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p4_565_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int off = dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int addrp = t->address + (off >> 1);
	int c = ((reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] >> ((off & 1) << 2)) & 0xf;
	return cv_565(pvrta_regs[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p4_565_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] & 0xf;
	return cv_565(pvrta_regs[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p4_4444_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int off = dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int addrp = t->address + (off >> 1);
	int c = ((reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] >> ((off & 1) << 2)) & 0xf;
	return cv_4444(pvrta_regs[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p4_4444_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] & 0xf;
	return cv_4444(pvrta_regs[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p4_8888_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int off = dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int addrp = t->address + (off >> 1);
	int c = ((reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] >> ((off & 1) << 2)) & 0xf;
	return pvrta_regs[t->palbase + c];
}

UINT32 powervr2_device::tex_r_p4_8888_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)] & 0xf;
	return pvrta_regs[t->palbase + c];
}

UINT32 powervr2_device::tex_r_p8_1555_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int addrp = t->address + dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_1555(pvrta_regs[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p8_1555_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_1555(pvrta_regs[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p8_565_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int addrp = t->address + dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_565(pvrta_regs[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p8_565_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_565(pvrta_regs[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p8_4444_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int addrp = t->address + dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_4444(pvrta_regs[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p8_4444_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return cv_4444(pvrta_regs[t->palbase + c]);
}

UINT32 powervr2_device::tex_r_p8_8888_tw(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int addrp = t->address + dilated1[t->cd][xt] + dilated0[t->cd][yt];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return pvrta_regs[t->palbase + c];
}

UINT32 powervr2_device::tex_r_p8_8888_vq(texinfo *t, float x, float y)
{
	int xt = ((int)x) & (t->sizex-1);
	int yt = ((int)y) & (t->sizey-1);
	int idx = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(t->address + dilated1[t->cd][xt >> 1] + dilated0[t->cd][yt >> 1])];
	int addrp = t->vqbase + 8*idx + dilated1[t->cd][xt & 1] + dilated0[t->cd][yt & 3];
	int c = (reinterpret_cast<UINT8 *>(dc_texture_ram))[BYTE_XOR_LE(addrp)];
	return pvrta_regs[t->palbase + c];
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
	if (!t->textured) return;

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
	t->stride = (t->mode & 1) && strideselect ? (pvrta_regs[TEXT_CONTROL] & 0x1f) << 5 : t->sizex;

	t->blend_mode  = blend_mode;
	t->filter_mode = filtermode;
	t->flip_u      = (flipuv >> 1) & 1;
	t->flip_v      = flipuv & 1;

	t->r = &powervr2_device::tex_r_default;
	t->cd = dilatechose[t->sizes];
	t->palbase = 0;
	t->vqbase = t->address;
	t->blend = use_alpha ? blend_functions[t->blend_mode] : bl10;

	//  fprintf(stderr, "tex %d %d %d %d\n", t->pf, t->mode, pvrta_regs[PAL_RAM_CTRL], t->mipmapped);

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
		case 0:  /*t->r = &powervr2_device::tex_r_yuv_tw*/; miptype = -1; break;
		case 1:  t->r = &powervr2_device::tex_r_yuv_n; miptype = -1; break;
		default: /*t->r = &powervr2_device::tex_r_yuv_vq*/; miptype = -1; break;
		}
		break;

	case 4: // bumpmap
		break;

	case 5: // 4bpp palette
		t->palbase = 0x400 | ((t->palette & 0x3f) << 4);
		switch(t->mode) {
		case 0: case 1:
			miptype = 0;

			switch(pvrta_regs[PAL_RAM_CTRL]) {
			case 0: t->r = &powervr2_device::tex_r_p4_1555_tw; break;
			case 1: t->r = &powervr2_device::tex_r_p4_565_tw;  break;
			case 2: t->r = &powervr2_device::tex_r_p4_4444_tw; break;
			case 3: t->r = &powervr2_device::tex_r_p4_8888_tw; break;
			}
			break;
		case 2: case 3:
			miptype = 3; // ?
			switch(pvrta_regs[PAL_RAM_CTRL]) {
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
		t->palbase = 0x400 | ((t->palette & 0x30) << 4);
		switch(t->mode) {
		case 0: case 1:
			miptype = 1;

			switch(pvrta_regs[PAL_RAM_CTRL]) {
			case 0: t->r = &powervr2_device::tex_r_p8_1555_tw; break;
			case 1: t->r = &powervr2_device::tex_r_p8_565_tw; break;
			case 2: t->r = &powervr2_device::tex_r_p8_4444_tw; break;
			case 3: t->r = &powervr2_device::tex_r_p8_8888_tw; break;
			}
			break;
		case 2: case 3:
			miptype = 3; // ?
			switch(pvrta_regs[PAL_RAM_CTRL]) {
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
		static const int mipmap_vq_offset[8] =  { 0x00006, 0x00016, 0x00056, 0x00156, 0x00556, 0x01556, 0x05556, 0x15556 }; // vq textures

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

// register decode helper
static inline int decode_reg_64(UINT32 offset, UINT64 mem_mask, UINT64 *shift)
{
	int reg = offset * 2;

	*shift = 0;

	// non 32-bit accesses have not yet been seen here, we need to know when they are
	if ((mem_mask != U64(0xffffffff00000000)) && (mem_mask != U64(0x00000000ffffffff)))
	{
		/*assume to return the lower 32-bits ONLY*/
		return reg & 0xffffffff;
	}

	if (mem_mask == U64(0xffffffff00000000))
	{
		reg++;
		*shift = 32;
	}

	return reg;
}

READ64_MEMBER( powervr2_device::pvr_ta_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg_64(offset, mem_mask, &shift);

	switch (reg)
	{
	case SPG_STATUS:
		{
			UINT8 fieldnum,vsync,hsync,blank;

			fieldnum = (space.machine().primary_screen->frame_number() & 1) ? 1 : 0;

			vsync = space.machine().primary_screen->vblank() ? 1 : 0;
			if(spg_vsync_pol) { vsync^=1; }

			hsync = space.machine().primary_screen->hblank() ? 1 : 0;
			if(spg_hsync_pol) { hsync^=1; }

			/* FIXME: following is just a wild guess */
			blank = (space.machine().primary_screen->vblank() | space.machine().primary_screen->hblank()) ? 0 : 1;
			if(spg_blank_pol) { blank^=1; }

			pvrta_regs[reg] = (vsync << 13) | (hsync << 12) | (blank << 11) | (fieldnum << 10) | (space.machine().primary_screen->vpos() & 0x3ff);
			break;
		}
	case SPG_TRIGGER_POS:
		printf("Warning: read at h/v counter ext latches\n");
		break;
	case TA_LIST_INIT:
		return 0; //bit 31 always return 0, a probable left-over in Crazy Taxi reads this and discards the read (?)
	}

	#if DEBUG_PVRTA_REGS
	if (reg != 0x43)
		mame_printf_verbose("PVRTA: [%08x] read %x @ %x (reg %x), mask %" I64FMT "x (PC=%x)\n", 0x5f8000+reg*4, pvrta_regs[reg], offset, reg, mem_mask, space.device().safe_pc());
	#endif
	return (UINT64)pvrta_regs[reg] << shift;
}

WRITE64_MEMBER( powervr2_device::pvr_ta_w )
{
	int reg;
	UINT64 shift;
	UINT32 dat;
	UINT32 sizera,offsetra;
	int a;
	int sanitycount;

	reg = decode_reg_64(offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);
	//old = pvrta_regs[reg];

	// Dreamcast BIOS attempts to set PVRID to zero and then dies
	// if it succeeds.  Don't allow.
	if ((reg != PVRID) && (reg != REVISION))
	{
		pvrta_regs[reg] = dat; // 5f8000+reg*4=dat
	}

	switch (reg)
	{
	case SOFTRESET:
		if (dat & 1)
		{
			#if DEBUG_PVRTA
			mame_printf_verbose("pvr_ta_w:  TA soft reset\n");
			#endif
			listtype_used=0;
		}
		if (dat & 2)
		{
			#if DEBUG_PVRTA
			mame_printf_verbose("pvr_ta_w:  Core Pipeline soft reset\n");
			#endif
			if (start_render_received == 1)
			{
				for (a=0;a < NUM_BUFFERS;a++)
					if (grab[a].busy == 1)
						grab[a].busy = 0;
				start_render_received = 0;
			}
		}
		if (dat & 4)
		{
			#if DEBUG_PVRTA
			mame_printf_verbose("pvr_ta_w:  sdram I/F soft reset\n");
			#endif
		}
		break;
	case STARTRENDER:
		g_profiler.start(PROFILER_USER1);
		#if DEBUG_PVRTA
		mame_printf_verbose("Start Render Received:\n");
		mame_printf_verbose("  Region Array at %08x\n",pvrta_regs[REGION_BASE]);
		mame_printf_verbose("  ISP/TSP Parameters at %08x\n",pvrta_regs[PARAM_BASE]);

		#endif
		// select buffer to draw using PARAM_BASE
		for (a=0;a < NUM_BUFFERS;a++)
		{
			if ((grab[a].ispbase == pvrta_regs[PARAM_BASE]) && (grab[a].valid == 1) && (grab[a].busy == 0))
			{
				grab[a].busy = 1;
				renderselect = a;
				start_render_received=1;


				grab[a].fbwsof1=pvrta_regs[FB_W_SOF1];
				grab[a].fbwsof2=pvrta_regs[FB_W_SOF2];

				rectangle clip(0, 1023, 0, 1023);

				// we've got a request to draw, so, draw to the accumulation buffer!
				// this should really be done for each tile!
				render_to_accumulation_buffer(*fake_accumulationbuffer_bitmap,clip);

				endofrender_timer_isp->adjust(attotime::from_usec(4000) ); // hack, make sure render takes some amount of time

				/* copy the tiles to the framebuffer (really the rendering should be in this loop too) */
				if (pvrta_regs[FPU_PARAM_CFG] & 0x200000)
					sizera=6;
				else
					sizera=5;
				offsetra=pvrta_regs[REGION_BASE];

				//printf("base is %08x\n", offsetra);

				// sanity
				sanitycount = 0;
				for (;;)
				{
					UINT32 st[6];

					st[0]=space.read_dword((0x05000000+offsetra));
					st[1]=space.read_dword((0x05000004+offsetra)); // Opaque List Pointer
					st[2]=space.read_dword((0x05000008+offsetra)); // Opaque Modifier Volume List Pointer
					st[3]=space.read_dword((0x0500000c+offsetra)); // Translucent List Pointer
					st[4]=space.read_dword((0x05000010+offsetra)); // Translucent Modifier Volume List Pointer

					if (sizera == 6)
					{
						st[5] = space.read_dword((0x05000014+offsetra)); // Punch Through List Pointer
						offsetra+=0x18;
					}
					else
					{
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

					// prevent infinite loop if asked to process invalid data
					if(sanitycount>2000)
						break;
				}




				break;
			}
		}
		if (a != NUM_BUFFERS)
			break;
		assert_always(0, "TA grabber error A!\n");
		break;
	case TA_LIST_INIT:
		if(dat & 0x80000000)
		{
			tafifo_pos=0;
			tafifo_mask=7;
			tafifo_vertexwords=8;
			tafifo_listtype= -1;
	#if DEBUG_PVRTA
			mame_printf_verbose("TA_OL_BASE       %08x TA_OL_LIMIT  %08x\n", pvrta_regs[TA_OL_BASE], pvrta_regs[TA_OL_LIMIT]);
			mame_printf_verbose("TA_ISP_BASE      %08x TA_ISP_LIMIT %08x\n", pvrta_regs[TA_ISP_BASE], pvrta_regs[TA_ISP_LIMIT]);
			mame_printf_verbose("TA_ALLOC_CTRL    %08x\n", pvrta_regs[TA_ALLOC_CTRL]);
			mame_printf_verbose("TA_NEXT_OPB_INIT %08x\n", pvrta_regs[TA_NEXT_OPB_INIT]);
	#endif
			pvrta_regs[TA_NEXT_OPB] = pvrta_regs[TA_NEXT_OPB_INIT];
			pvrta_regs[TA_ITP_CURRENT] = pvrta_regs[TA_ISP_BASE];
			alloc_ctrl_OPB_Mode = pvrta_regs[TA_ALLOC_CTRL] & 0x100000; // 0 up 1 down
			alloc_ctrl_PT_OPB = (4 << ((pvrta_regs[TA_ALLOC_CTRL] >> 16) & 3)) & 0x38; // number of 32 bit words (0,8,16,32)
			alloc_ctrl_TM_OPB = (4 << ((pvrta_regs[TA_ALLOC_CTRL] >> 12) & 3)) & 0x38;
			alloc_ctrl_T_OPB = (4 << ((pvrta_regs[TA_ALLOC_CTRL] >> 8) & 3)) & 0x38;
			alloc_ctrl_OM_OPB = (4 << ((pvrta_regs[TA_ALLOC_CTRL] >> 4) & 3)) & 0x38;
			alloc_ctrl_O_OPB = (4 << ((pvrta_regs[TA_ALLOC_CTRL] >> 0) & 3)) & 0x38;
			listtype_used |= (1+4);
			// use TA_ISP_BASE and select buffer for grab data
			grabsel = -1;
			// try to find already used buffer but not busy
			for (a=0;a < NUM_BUFFERS;a++)
			{
				if ((grab[a].ispbase == pvrta_regs[TA_ISP_BASE]) && (grab[a].busy == 0) && (grab[a].valid == 1))
				{
					grabsel=a;
					break;
				}
			}
			// try a buffer not used yet
			if (grabsel < 0)
			{
				for (a=0;a < NUM_BUFFERS;a++)
				{
					if (grab[a].valid == 0)
					{
						grabsel=a;
						break;
					}
				}
			}
			// find a non busy buffer starting from the last one used
			if (grabsel < 0)
			{
				for (a=0;a < 3;a++)
				{
					if (grab[(grabsellast+1+a) & 3].busy == 0)
					{
						grabsel=a;
						break;
					}
				}
			}
			if (grabsel < 0)
				assert_always(0, "TA grabber error B!\n");
			grabsellast=grabsel;
			grab[grabsel].ispbase=pvrta_regs[TA_ISP_BASE];
			grab[grabsel].busy=0;
			grab[grabsel].valid=1;
			grab[grabsel].verts_size=0;
			grab[grabsel].strips_size=0;

			g_profiler.stop();
		}
		break;
//#define TA_YUV_TEX_BASE       ((0x005f8148-0x005f8000)/4)
	case TA_YUV_TEX_BASE:
		printf("TA_YUV_TEX_BASE initialized to %08x\n", dat);

		// hack, this interrupt is generated after transfering a set amount of data
		//state->state->dc_sysctrl_regs[SB_ISTNRM] |= IST_EOXFER_YUV;
		//state->state->dc_update_interrupt_status();

		break;
	case TA_YUV_TEX_CTRL:
		printf("TA_YUV_TEX_CTRL initialized to %08x\n", dat);
		break;

	case SPG_VBLANK_INT:
		/* clear pending irqs and modify them with the updated ones */
		vbin_timer->adjust(attotime::never);
		vbout_timer->adjust(attotime::never);

		vbin_timer->adjust(space.machine().primary_screen->time_until_pos(spg_vblank_in_irq_line_num));
		vbout_timer->adjust(space.machine().primary_screen->time_until_pos(spg_vblank_out_irq_line_num));
		break;
	/* TODO: timer adjust for SPG_HBLANK_INT too */
	case TA_LIST_CONT:
	#if DEBUG_PVRTA
		mame_printf_verbose("List continuation processing\n");
	#endif
		if(dat & 0x80000000)
		{
			tafifo_listtype= -1; // no list being received
			listtype_used |= (1+4);
		}
		break;
	case SPG_VBLANK:
	case SPG_HBLANK:
	case SPG_LOAD:
	case VO_STARTX:
	case VO_STARTY:
		{
			rectangle visarea = space.machine().primary_screen->visible_area();
			/* FIXME: right visible area calculations aren't known yet*/
			visarea.min_x = 0;
			visarea.max_x = ((spg_hbstart - spg_hbend - vo_horz_start_pos) <= 0x180 ? 320 : 640) - 1;
			visarea.min_y = 0;
			visarea.max_y = ((spg_vbstart - spg_vbend - vo_vert_start_pos_f1) <= 0x100 ? 240 : 480) - 1;


			space.machine().primary_screen->configure(spg_hbstart, spg_vbstart, visarea, space.machine().primary_screen->frame_period().attoseconds );
		}
		break;
	}

	#if DEBUG_PVRTA_REGS
	if ((reg != 0x14) && (reg != 0x15))
		mame_printf_verbose("PVRTA: [%08x=%x] write %" I64FMT "x to %x (reg %x %x), mask %" I64FMT "x\n", 0x5f8000+reg*4, dat, data>>shift, offset, reg, (reg*4)+0x8000, mem_mask);
	#endif
}

TIMER_CALLBACK_MEMBER(powervr2_device::transfer_opaque_list_irq)
{
	dc_state *state = machine().driver_data<dc_state>();
	state->dc_sysctrl_regs[SB_ISTNRM] |= IST_EOXFER_OPLST;
	state->dc_update_interrupt_status();
}

TIMER_CALLBACK_MEMBER(powervr2_device::transfer_opaque_modifier_volume_list_irq)
{
	dc_state *state = machine().driver_data<dc_state>();
	state->dc_sysctrl_regs[SB_ISTNRM] |= IST_EOXFER_OPMV;
	state->dc_update_interrupt_status();
}

TIMER_CALLBACK_MEMBER(powervr2_device::transfer_translucent_list_irq)
{
	dc_state *state = machine().driver_data<dc_state>();
	state->dc_sysctrl_regs[SB_ISTNRM] |= IST_EOXFER_TRLST;
	state->dc_update_interrupt_status();
}

TIMER_CALLBACK_MEMBER(powervr2_device::transfer_translucent_modifier_volume_list_irq)
{
	dc_state *state = machine().driver_data<dc_state>();
	state->dc_sysctrl_regs[SB_ISTNRM] |= IST_EOXFER_TRMV;
	state->dc_update_interrupt_status();
}

TIMER_CALLBACK_MEMBER(powervr2_device::transfer_punch_through_list_irq)
{
	dc_state *state = machine().driver_data<dc_state>();
	state->dc_sysctrl_regs[SB_ISTNRM] |= (1 << 21);
	state->dc_update_interrupt_status();
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
		mame_printf_verbose("Para Type 0 End of List\n");
		#endif
		/* Process transfer FIFO done irqs here */
		/* FIXME: timing of these */
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
		mame_printf_verbose("Para Type 1 User Tile Clip\n");
		mame_printf_verbose(" (%d , %d)-(%d , %d)\n", tafifo_buff[4], tafifo_buff[5], tafifo_buff[6], tafifo_buff[7]);
		#endif
	}
	else if (paratype == 2)
	{ // object list set
		#if DEBUG_PVRDLIST
		mame_printf_verbose("Para Type 2 Object List Set at %08x\n", tafifo_buff[1]);
		mame_printf_verbose(" (%d , %d)-(%d , %d)\n", tafifo_buff[4], tafifo_buff[5], tafifo_buff[6], tafifo_buff[7]);
		#endif
	}
	else if (paratype == 3)
	{
		#if DEBUG_PVRDLIST
		mame_printf_verbose("Para Type %x Unknown!\n", tafifo_buff[0]);
		#endif
	}
	else
	{ // global parameter or vertex parameter
		#if DEBUG_PVRDLIST
		mame_printf_verbose("Para Type %d", paratype);
		if (paratype == 7)
			mame_printf_verbose(" End of Strip %d", endofstrip);
		if (listtype_used & 3)
			mame_printf_verbose(" List Type %d", listtype);
		mame_printf_verbose("\n");
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
				mame_printf_verbose(" Texture at %08x format %d\n", (tafifo_buff[3] & 0x1FFFFF) << 3, pixelformat);
				#endif
			}
			if (paratype == 4)
			{ // polygon or mv
				if ((tafifo_listtype == 1) || (tafifo_listtype == 3))
				{
				#if DEBUG_PVRDLIST
					mame_printf_verbose(" Modifier Volume\n");
				#endif
				}
				else
				{
				#if DEBUG_PVRDLIST
					mame_printf_verbose(" Polygon\n");
				#endif
				}
			}
			if (paratype == 5)
			{ // quad
				#if DEBUG_PVRDLIST
				mame_printf_verbose(" Sprite\n");
				#endif
			}
		}

		if (paratype == 7)
		{ // vertex
			if ((tafifo_listtype == 1) || (tafifo_listtype == 3))
			{
				#if DEBUG_PVRDLIST
				mame_printf_verbose(" Vertex modifier volume");
				mame_printf_verbose(" A(%f,%f,%f) B(%f,%f,%f) C(%f,%f,%f)", u2f(tafifo_buff[1]), u2f(tafifo_buff[2]),
					u2f(tafifo_buff[3]), u2f(tafifo_buff[4]), u2f(tafifo_buff[5]), u2f(tafifo_buff[6]), u2f(tafifo_buff[7]),
					u2f(tafifo_buff[8]), u2f(tafifo_buff[9]));
				mame_printf_verbose("\n");
				#endif
			}
			else if (global_paratype == 5)
			{
				#if DEBUG_PVRDLIST
				mame_printf_verbose(" Vertex sprite");
				mame_printf_verbose(" A(%f,%f,%f) B(%f,%f,%f) C(%f,%f,%f) D(%f,%f,)", u2f(tafifo_buff[1]), u2f(tafifo_buff[2]),
					u2f(tafifo_buff[3]), u2f(tafifo_buff[4]), u2f(tafifo_buff[5]), u2f(tafifo_buff[6]), u2f(tafifo_buff[7]),
					u2f(tafifo_buff[8]), u2f(tafifo_buff[9]), u2f(tafifo_buff[10]), u2f(tafifo_buff[11]));
				mame_printf_verbose("\n");
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
				mame_printf_verbose(" Vertex polygon");
				mame_printf_verbose(" V(%f,%f,%f) T(%f,%f)", u2f(tafifo_buff[1]), u2f(tafifo_buff[2]), u2f(tafifo_buff[3]), u2f(tafifo_buff[4]), u2f(tafifo_buff[5]));
				mame_printf_verbose("\n");
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
		mame_printf_debug("ta_fifo_poly_w:  Unmapped write64 %08x = %" I64FMT "x -> %08x %08x\n", 0x10000000+offset*8, data, tafifo_buff[tafifo_pos], tafifo_buff[tafifo_pos+1]);
		#endif
		tafifo_pos += 2;
	}
	else
	{
		fatalerror("ta_fifo_poly_w:  Only 64 bit writes supported!\n");
	}

	tafifo_pos &= tafifo_mask;

	// if the command is complete, process it
	if (tafifo_pos == 0)
		process_ta_fifo();

}

WRITE64_MEMBER( powervr2_device::ta_fifo_yuv_w )
{
	//dc_state *state = space.machine().driver_data<dc_state>();

//  int reg;
//  UINT64 shift;
//  UINT32 dat;

//  reg = decode_reg_64(offset, mem_mask, &shift);
//  dat = (UINT32)(data >> shift);

//  printf("YUV FIFO: [%08x=%x] write %" I64FMT "x to %x, mask %" I64FMT "x %08x\n", 0x10800000+reg*4, dat, data, offset, mem_mask,test);
}

// SB_LMMODE0
WRITE64_MEMBER(powervr2_device::ta_texture_directpath0_w )
{
	int mode = pvrctrl_regs[SB_LMMODE0]&1;
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
	int mode = pvrctrl_regs[SB_LMMODE1]&1;
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
	if (!ti->textured) return;

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
	ddx = xxl + 0.5 - xl;
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
					UINT32 c1 = (this->*(ti->r))(ti, u+1.0, v);
					UINT32 c2 = (this->*(ti->r))(ti, u+1.0, v+1.0);
					UINT32 c3 = (this->*(ti->r))(ti, u, v+1.0);
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

	dy = yy0+0.5-y0;

	if(0)
		fprintf(stderr, "%f %f %f %f -> %f %f | %f %f -> %f %f\n",
				y0,
				dy, dxldy, dxrdy, dy*dxldy, dy*dxrdy,
				xl, xr, xl + dy*dxldy, xr + dy*dxrdy);
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
	int cs,rs,ns;
	UINT32 c;
#if 0
	int stride;
	UINT16 *bmpaddr16;
	UINT32 k;
#endif


	if (renderselect < 0)
		return;

	//printf("drawtest!\n");

	rs=renderselect;
	c=pvrta_regs[ISP_BACKGND_T];
	c=space.read_dword(0x05000000+((c&0xfffff8)>>1)+(3+3)*4);
	bitmap.fill(c, cliprect);


	ns=grab[rs].strips_size;
	if(ns)
		memset(wbuffer, 0x00, sizeof(wbuffer));

	for (cs=0;cs < ns;cs++)
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

	UINT32 wc = pvrta_regs[FB_W_CTRL];
	UINT32 stride = pvrta_regs[FB_W_LINESTRIDE];
	UINT32 writeoffs = pvrta_regs[FB_W_SOF1];

	UINT32* src;


	UINT8 packmode = wc & 0x7;

	switch (packmode)
	{
		// used by ringout
		case 0x00: //0555 KRGB
		{
			int xcnt,ycnt;
			for (ycnt=0;ycnt<32;ycnt++)
			{
				UINT32 realwriteoffs = 0x05000000 + writeoffs + (y+ycnt) * (stride<<3) + (x*2);
				src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);


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
		break;

		// used by cleoftp
		case 0x01: //565 RGB 16 bit
		{
			int xcnt,ycnt;
			for (ycnt=0;ycnt<32;ycnt++)
			{
				UINT32 realwriteoffs = 0x05000000 + writeoffs + (y+ycnt) * (stride<<3) + (x*2);
				src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);


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
		break;

		case 0x02:
			printf("pvr_accumulationbuffer_to_framebuffer buffer to tile at %d,%d - unsupported pack mode %02x (4444 ARGB)\n",x,y,packmode);
			break;

		case 0x03: // 1555 ARGB 16 bit
		{
			int xcnt,ycnt;
			for (ycnt=0;ycnt<32;ycnt++)
			{
				UINT32 realwriteoffs = 0x05000000 + writeoffs + (y+ycnt) * (stride<<3) + (x*2);
				src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);


				for (xcnt=0;xcnt<32;xcnt++)
				{
					// data starts in 8888 format, downsample it
					UINT32 data = src[xcnt];
					UINT16 newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
									((((data & 0x0000f800) >> 11)) << 5)  |
									((((data & 0x00f80000) >> 19)) << 10);
					// alpha?

					space.write_word(realwriteoffs+xcnt*2, newdat);
				}
			}
		}
		break;

		// used by Suchie3
		case 0x04: // 888 RGB 24-bit (HACK! should not downconvert and pvr_drawframebuffer should change accordingly)
		{
			int xcnt,ycnt;
			for (ycnt=0;ycnt<32;ycnt++)
			{
				UINT32 realwriteoffs = 0x05000000 + writeoffs + (y+ycnt) * (stride<<3) + (x*2);
				src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);


				for (xcnt=0;xcnt<32;xcnt++)
				{
					// data is 8888 format
					UINT32 data = src[xcnt];
					UINT16 newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
									((((data & 0x0000fc00) >> 10)) << 5)  |
									((((data & 0x00f80000) >> 19)) << 11);

					space.write_word(realwriteoffs+xcnt*2, newdat);
				}
			}
		}
		break;

		case 0x05:
			printf("pvr_accumulationbuffer_to_framebuffer buffer to tile at %d,%d - unsupported pack mode %02x (0888 KGB 32-bit)\n",x,y,packmode);
			break;

		case 0x06: // 8888 ARGB 32 bit (HACK! should not downconvert and pvr_drawframebuffer should change accordingly)
		{
			int xcnt,ycnt;
			for (ycnt=0;ycnt<32;ycnt++)
			{
				UINT32 realwriteoffs = 0x05000000 + writeoffs + (y+ycnt) * (stride<<3) + (x*2);
				src = &fake_accumulationbuffer_bitmap->pix32(y+ycnt, x);


				for (xcnt=0;xcnt<32;xcnt++)
				{
					// data is 8888 format
					UINT32 data = src[xcnt];
					UINT16 newdat = ((((data & 0x000000f8) >> 3)) << 0)   |
									((((data & 0x0000fc00) >> 10)) << 5)  |
									((((data & 0x00f80000) >> 19)) << 11);

					space.write_word(realwriteoffs+xcnt*2, newdat);
				}
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

	UINT32 wc = pvrta_regs[FB_R_CTRL];
	UINT8 unpackmode = (wc & 0x0000000c) >>2;  // aka fb_depth
	UINT8 enable = (wc & 0x00000001);

	// ??
	if (!enable) return;

	// only for rgb565 framebuffer
	xi=((pvrta_regs[FB_R_SIZE] & 0x3ff)+1) << 1;
	dy=((pvrta_regs[FB_R_SIZE] >> 10) & 0x3ff)+1;

	dy++;
	dy*=2; // probably depends on interlace mode, fields etc...

	switch (unpackmode)
	{
		case 0x00: // 0555 RGB 16-bit, Cleo Fortune Plus
			// should upsample back to 8-bit output using fb_concat
			for (y=0;y <= dy;y++)
			{
				addrp=pvrta_regs[FB_R_SOF1]+y*xi*2;
				if(spg_pixel_double)
				{
					for (x=0;x < xi;x++)
					{
						fbaddr=&bitmap.pix32(y, x*2+0);
						c=*((reinterpret_cast<UINT16 *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 1));

						b = (c & 0x001f) << 3;
						g = (c & 0x03e0) >> 2;
						r = (c & 0x7c00) >> 7;

						if (y<=cliprect.max_y)
							*fbaddr = b | (g<<8) | (r<<16);

						fbaddr=&bitmap.pix32(y, x*2+1);
						if (y<=cliprect.max_y)
							*fbaddr = b | (g<<8) | (r<<16);
						addrp+=2;
					}
				}
				else
				{
					for (x=0;x < xi;x++)
					{
						fbaddr=&bitmap.pix32(y, x);
						c=*((reinterpret_cast<UINT16 *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 1));

						b = (c & 0x001f) << 3;
						g = (c & 0x03e0) >> 2;
						r = (c & 0x7c00) >> 7;

						if (y<=cliprect.max_y)
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
				addrp=pvrta_regs[FB_R_SOF1]+y*xi*2;
				if(spg_pixel_double)
				{
					for (x=0;x < xi;x++)
					{
						fbaddr=&bitmap.pix32(y, x*2+0);
						c=*((reinterpret_cast<UINT16 *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 1));

						b = (c & 0x001f) << 3;
						g = (c & 0x07e0) >> 3;
						r = (c & 0xf800) >> 8;

						if (y<=cliprect.max_y)
							*fbaddr = b | (g<<8) | (r<<16);

						fbaddr=&bitmap.pix32(y, x*2+1);

						if (y<=cliprect.max_y)
							*fbaddr = b | (g<<8) | (r<<16);

						addrp+=2;
					}
				}
				else
				{
					for (x=0;x < xi;x++)
					{
						fbaddr=&bitmap.pix32(y, x);
						c=*((reinterpret_cast<UINT16 *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 1));

						b = (c & 0x001f) << 3;
						g = (c & 0x07e0) >> 3;
						r = (c & 0xf800) >> 8;

						if (y<=cliprect.max_y)
							*fbaddr = b | (g<<8) | (r<<16);
						addrp+=2;
					}
				}
			}
			break;

		case 0x02: ; // 888 RGB 24-bit - suchie3 - HACKED, see pvr_accumulationbuffer_to_framebuffer!
			for (y=0;y <= dy;y++)
			{
				addrp=pvrta_regs[FB_R_SOF1]+y*xi*2;
				if(spg_pixel_double)
				{
					for (x=0;x < xi;x++)
					{
						fbaddr=&bitmap.pix32(y, x*2+0);
						c=*((reinterpret_cast<UINT16 *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 1));

						b = (c & 0x001f) << 3;
						g = (c & 0x07e0) >> 3;
						r = (c & 0xf800) >> 8;

						if (y<=cliprect.max_y)
							*fbaddr = b | (g<<8) | (r<<16);

						fbaddr=&bitmap.pix32(y, x*2+1);

						if (y<=cliprect.max_y)
							*fbaddr = b | (g<<8) | (r<<16);
						addrp+=2;
					}
				}
				else
				{
					for (x=0;x < xi;x++)
					{
						fbaddr=&bitmap.pix32(y, x);
						c=*((reinterpret_cast<UINT16 *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 1));

						b = (c & 0x001f) << 3;
						g = (c & 0x07e0) >> 3;
						r = (c & 0xf800) >> 8;

						if (y<=cliprect.max_y)
							*fbaddr = b | (g<<8) | (r<<16);
						addrp+=2;
					}
				}
			}
			break;

		case 0x03:        // 0888 ARGB 32-bit - HACKED, see pvr_accumulationbuffer_to_framebuffer!
			for (y=0;y <= dy;y++)
			{
				addrp=pvrta_regs[FB_R_SOF1]+y*xi*2;
				if(spg_pixel_double)
				{
					for (x=0;x < xi;x++)
					{
						fbaddr=&bitmap.pix32(y, x*2+0);
						c=*((reinterpret_cast<UINT16 *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 1));

						b = (c & 0x001f) << 3;
						g = (c & 0x07e0) >> 3;
						r = (c & 0xf800) >> 8;

						if (y<=cliprect.max_y)
							*fbaddr = b | (g<<8) | (r<<16);

						fbaddr=&bitmap.pix32(y, x*2+1);
						if (y<=cliprect.max_y)
							*fbaddr = b | (g<<8) | (r<<16);

						addrp+=2;
					}
				}
				else
				{
					for (x=0;x < xi;x++)
					{
						fbaddr=&bitmap.pix32(y, x);
						c=*((reinterpret_cast<UINT16 *>(dc_framebuffer_ram)) + (WORD2_XOR_LE(addrp) >> 1));

						b = (c & 0x001f) << 3;
						g = (c & 0x07e0) >> 3;
						r = (c & 0xf800) >> 8;

						if (y<=cliprect.max_y)
							*fbaddr = b | (g<<8) | (r<<16);
						addrp+=2;
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

	//popmessage("%02x",pvrta_regs[PAL_RAM_CTRL]);

	for(i=0;i<0x400;i++)
	{
		pal = pvrta_regs[((0x005F9000-0x005F8000)/4)+i];
		switch(pvrta_regs[PAL_RAM_CTRL])
		{
			case 0: //argb1555 <- guilty gear uses this mode
			{
				//a = (pal & 0x8000)>>15;
				r = (pal & 0x7c00)>>10;
				g = (pal & 0x03e0)>>5;
				b = (pal & 0x001f)>>0;
				//a = a ? 0xff : 0x00;
				palette_set_color_rgb(machine(), i, pal5bit(r), pal5bit(g), pal5bit(b));
			}
			break;
			case 1: //rgb565
			{
				//a = 0xff;
				r = (pal & 0xf800)>>11;
				g = (pal & 0x07e0)>>5;
				b = (pal & 0x001f)>>0;
				palette_set_color_rgb(machine(), i, pal5bit(r), pal6bit(g), pal5bit(b));
			}
			break;
			case 2: //argb4444
			{
				//a = (pal & 0xf000)>>12;
				r = (pal & 0x0f00)>>8;
				g = (pal & 0x00f0)>>4;
				b = (pal & 0x000f)>>0;
				palette_set_color_rgb(machine(), i, pal4bit(r), pal4bit(g), pal4bit(b));
			}
			break;
			case 3: //argb8888
			{
				//a = (pal & 0xff000000)>>20;
				r = (pal & 0x00ff0000)>>16;
				g = (pal & 0x0000ff00)>>8;
				b = (pal & 0x000000ff)>>0;
				palette_set_color_rgb(machine(), i, r, g, b);
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
	dc_state *state = machine().driver_data<dc_state>();
	state->dc_sysctrl_regs[SB_ISTNRM] |= IST_VBL_IN; // V Blank-in interrupt
	state->dc_update_interrupt_status();

	vbin_timer->adjust(machine().primary_screen->time_until_pos(spg_vblank_in_irq_line_num));
}

TIMER_CALLBACK_MEMBER(powervr2_device::vbout)
{
	dc_state *state = machine().driver_data<dc_state>();
	state->dc_sysctrl_regs[SB_ISTNRM] |= IST_VBL_OUT; // V Blank-out interrupt
	state->dc_update_interrupt_status();

	vbout_timer->adjust(machine().primary_screen->time_until_pos(spg_vblank_out_irq_line_num));
}

TIMER_CALLBACK_MEMBER(powervr2_device::hbin)
{
	if(spg_hblank_int_mode & 1)
	{
		if(scanline == next_y)
		{
			dc_state *state = machine().driver_data<dc_state>();
			state->dc_sysctrl_regs[SB_ISTNRM] |= IST_HBL_IN; // H Blank-in interrupt
			state->dc_update_interrupt_status();
			next_y+=spg_line_comp_val;
		}
	}
	else if((scanline == spg_line_comp_val) || (spg_hblank_int_mode & 2))
	{
		dc_state *state = machine().driver_data<dc_state>();
		state->dc_sysctrl_regs[SB_ISTNRM] |= IST_HBL_IN; // H Blank-in interrupt
		state->dc_update_interrupt_status();
	}

//  printf("hbin on scanline %d\n",scanline);

	scanline++;

	if(scanline >= spg_vblank_in_irq_line_num)
	{
		scanline = 0;
		next_y = spg_line_comp_val;
	}

	hbin_timer->adjust(machine().primary_screen->time_until_pos(scanline, spg_hblank_in_irq-1));
}



TIMER_CALLBACK_MEMBER(powervr2_device::endofrender_video)
{
	dc_state *state = machine().driver_data<dc_state>();
	state->dc_sysctrl_regs[SB_ISTNRM] |= IST_EOR_VIDEO;// VIDEO end of render
	state->dc_update_interrupt_status();
	endofrender_timer_video->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(powervr2_device::endofrender_tsp)
{
	dc_state *state = machine().driver_data<dc_state>();
	state->dc_sysctrl_regs[SB_ISTNRM] |= IST_EOR_TSP;  // TSP end of render
	state->dc_update_interrupt_status();

	endofrender_timer_tsp->adjust(attotime::never);
	endofrender_timer_video->adjust(attotime::from_usec(500) );
}

TIMER_CALLBACK_MEMBER(powervr2_device::endofrender_isp)
{
	dc_state *state = machine().driver_data<dc_state>();
	state->dc_sysctrl_regs[SB_ISTNRM] |= IST_EOR_ISP;  // ISP end of render
	state->dc_update_interrupt_status();

	endofrender_timer_isp->adjust(attotime::never);
	endofrender_timer_tsp->adjust(attotime::from_usec(500) );
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

	bitmap.fill(MAKE_ARGB(0xff,vo_border_R,vo_border_G,vo_border_B), cliprect); //FIXME: Chroma bit?

	if(!spg_blank_video)
		pvr_drawframebuffer(bitmap, cliprect);

	// update this here so we only do string lookup once per frame
	debug_dip_status = ioport(":MAMEDEBUG")->read();

	return 0;
}


/* Naomi 2 attempts (TBD) */

READ64_MEMBER( powervr2_device::pvr2_ta_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg_64(offset, mem_mask, &shift);

	switch (reg)
	{
	}

	printf("PVR2 %08x R\n",reg);

	return 0;
}

WRITE64_MEMBER( powervr2_device::pvr2_ta_w )
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


WRITE64_MEMBER( powervr2_device::pvrs_ta_w )
{
	pvr_ta_w(space,offset,data,mem_mask);
	pvr2_ta_w(space,offset,data,mem_mask);
	//printf("PVR2 %08x %08x\n",reg,dat);
}

TIMER_CALLBACK_MEMBER(powervr2_device::pvr_dma_irq)
{
	dc_state *state = machine().driver_data<dc_state>();
	m_pvr_dma.start = pvrctrl_regs[SB_PDST] = 0;
	state->dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_PVR;
	state->dc_update_interrupt_status();
}

READ64_MEMBER(powervr2_device::pvr_ctrl_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg_64(offset, mem_mask, &shift);

	#if DEBUG_PVRCTRL
	mame_printf_verbose("PVRCTRL: [%08x] read %x @ %x (reg %x), mask %" I64FMT "x (PC=%x)\n", 0x5f7c00+reg*4, pvrctrl_regs[reg], offset, reg, mem_mask, space.device().safe_pc());
	#endif

	return (UINT64)pvrctrl_regs[reg] << shift;
}

WRITE64_MEMBER(powervr2_device::pvr_ctrl_w )
{
	int reg;
	UINT64 shift;
	UINT32 dat;
	UINT8 old;

	reg = decode_reg_64(offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);

	switch (reg)
	{
		case SB_PDSTAP: m_pvr_dma.pvr_addr = dat; break;
		case SB_PDSTAR: m_pvr_dma.sys_addr = dat; break;
		case SB_PDLEN: m_pvr_dma.size = dat; break;
		case SB_PDDIR: m_pvr_dma.dir = dat & 1; break;
		case SB_PDTSEL:
			m_pvr_dma.sel = dat & 1;
			//if(m_pvr_dma.sel & 1)
			//  printf("Warning: Unsupported irq mode trigger PVR-DMA\n");
			break;
		case SB_PDEN: m_pvr_dma.flag = dat & 1; break;
		case SB_PDST:
			old = m_pvr_dma.start & 1;
			m_pvr_dma.start = dat & 1;

			if(((old & 1) == 0) && m_pvr_dma.flag && m_pvr_dma.start && ((m_pvr_dma.sel & 1) == 0)) // 0 -> 1
				pvr_dma_execute(space);
			break;
	}

	#if DEBUG_PVRCTRL
	mame_printf_verbose("PVRCTRL: [%08x=%x] write %" I64FMT "x to %x (reg %x), mask %" I64FMT "x\n", 0x5f7c00+reg*4, dat, data>>shift, offset, reg, mem_mask);
	#endif

//  pvrctrl_regs[reg] |= dat;
	pvrctrl_regs[reg] = dat;
}

void powervr2_device::pvr_dma_execute(address_space &space)
{
	UINT32 src,dst,size;
	dst = m_pvr_dma.pvr_addr;
	src = m_pvr_dma.sys_addr;
	size = 0;

	/* used so far by usagui and sprtjam*/
	//printf("PVR-DMA start\n");
	//printf("%08x %08x %08x\n",m_pvr_dma.pvr_addr,m_pvr_dma.sys_addr,m_pvr_dma.size);
	//printf("src %s dst %08x\n",m_pvr_dma.dir ? "->" : "<-",m_pvr_dma.sel);

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
	machine().scheduler().timer_set(attotime::from_usec(250), timer_expired_delegate(FUNC(powervr2_device::pvr_dma_irq), this));
}

powervr2_device::powervr2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, POWERVR2, "PowerVR 2", tag, owner, clock)
{
}

void powervr2_device::device_start()
{
	memset(pvrctrl_regs, 0, sizeof(pvrctrl_regs));
	memset(pvrta_regs, 0, sizeof(pvrta_regs));
	memset(grab, 0, sizeof(grab));
	pvr_build_parameterconfig();

	computedilated();

	vbout_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(powervr2_device::vbout),this));
	vbin_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(powervr2_device::vbin),this));
	hbin_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(powervr2_device::hbin),this));

	endofrender_timer_isp = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(powervr2_device::endofrender_isp),this));
	endofrender_timer_tsp = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(powervr2_device::endofrender_tsp),this));
	endofrender_timer_video = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(powervr2_device::endofrender_video),this));

	fake_accumulationbuffer_bitmap = auto_bitmap_rgb32_alloc(machine(),1024,1024);

	save_item(NAME(m_pvr_dma.pvr_addr));
	save_item(NAME(m_pvr_dma.sys_addr));
	save_item(NAME(m_pvr_dma.size));
	save_item(NAME(m_pvr_dma.sel));
	save_item(NAME(m_pvr_dma.dir));
	save_item(NAME(m_pvr_dma.flag));
	save_item(NAME(m_pvr_dma.start));
	save_pointer(NAME(pvrta_regs),0x2000/4);
	save_pointer(NAME(pvrctrl_regs),0x100/4);
	save_item(NAME(debug_dip_status));
	save_pointer(NAME(tafifo_buff),32);
	save_item(NAME(scanline));
	save_item(NAME(next_y));
}

void powervr2_device::device_reset()
{
	pvrta_regs[VO_CONTROL]=     0x00000108;
	pvrta_regs[SOFTRESET]=      0x00000007;
	pvrta_regs[VO_STARTX]=      0x0000009d;
	pvrta_regs[VO_STARTY]=      0x00150015;
	pvrta_regs[SPG_HBLANK]=     0x007e0345;
	pvrta_regs[SPG_LOAD]=       0x01060359;
	pvrta_regs[SPG_VBLANK]=     0x01500104;
	pvrta_regs[SPG_HBLANK_INT]= 0x031d0000;
	pvrta_regs[SPG_VBLANK_INT]= 0x01500104;

	// if the next 2 registers do not have the correct values, the naomi bios will hang
	pvrta_regs[PVRID]=0x17fd11db;
	pvrta_regs[REVISION]=0x11;

	tafifo_pos=0;
	tafifo_mask=7;
	tafifo_vertexwords=8;
	tafifo_listtype= -1;
	start_render_received=0;
	renderselect= -1;
	grabsel=0;

	vbout_timer->adjust(machine().primary_screen->time_until_pos(spg_vblank_out_irq_line_num_new));
	vbin_timer->adjust(machine().primary_screen->time_until_pos(spg_vblank_in_irq_line_num_new));
	hbin_timer->adjust(machine().primary_screen->time_until_pos(0, spg_hblank_in_irq_new-1));

	scanline = 0;
	next_y = 0;

	endofrender_timer_isp->adjust(attotime::never);
	endofrender_timer_tsp->adjust(attotime::never);
	endofrender_timer_video->adjust(attotime::never);

	dc_state *state = machine().driver_data<dc_state>();
	dc_texture_ram = state->dc_texture_ram.target();
	dc_framebuffer_ram = state->dc_framebuffer_ram.target();
}
