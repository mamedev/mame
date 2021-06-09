// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo.h

    3dfx Voodoo Graphics SST-1/2 emulator.

***************************************************************************/

#ifndef MAME_VIDEO_VOODOO_H
#define MAME_VIDEO_VOODOO_H

#pragma once

#include "video/poly.h"
#include "video/rgbutil.h"
#include "screen.h"


/*
cleanup to do:
 voodoo_regs class
 rasterizer template
*/


/*************************************
 *
 *  Misc. constants
 *
 *************************************/

/* enumeration describing reasons we might be stalled */
enum
{
	NOT_STALLED = 0,
	STALLED_UNTIL_FIFO_LWM,
	STALLED_UNTIL_FIFO_EMPTY
};



// Use old table lookup versus straight double divide
#define USE_FAST_RECIP  0

/* maximum number of TMUs */
#define MAX_TMU                 2

/* accumulate operations less than this number of clocks */
#define ACCUMULATE_THRESHOLD    0

/* number of clocks to set up a triangle (just a guess) */
#define TRIANGLE_SETUP_CLOCKS   100

/* maximum number of rasterizers */
#define MAX_RASTERIZERS         1024

/* size of the rasterizer hash table */
#define RASTER_HASH_SIZE        97

/* flags for LFB writes */
#define LFB_RGB_PRESENT         1
#define LFB_ALPHA_PRESENT       2
#define LFB_DEPTH_PRESENT       4
#define LFB_DEPTH_PRESENT_MSW   8

/* flags for the register access array */
#define REGISTER_READ           0x01        /* reads are allowed */
#define REGISTER_WRITE          0x02        /* writes are allowed */
#define REGISTER_PIPELINED      0x04        /* writes are pipelined */
#define REGISTER_FIFO           0x08        /* writes go to FIFO */
#define REGISTER_WRITETHRU      0x10        /* writes are valid even for CMDFIFO */

/* shorter combinations to make the table smaller */
#define REG_R                   (REGISTER_READ)
#define REG_W                   (REGISTER_WRITE)
#define REG_WT                  (REGISTER_WRITE | REGISTER_WRITETHRU)
#define REG_RW                  (REGISTER_READ | REGISTER_WRITE)
#define REG_RWT                 (REGISTER_READ | REGISTER_WRITE | REGISTER_WRITETHRU)
#define REG_RP                  (REGISTER_READ | REGISTER_PIPELINED)
#define REG_WP                  (REGISTER_WRITE | REGISTER_PIPELINED)
#define REG_RWP                 (REGISTER_READ | REGISTER_WRITE | REGISTER_PIPELINED)
#define REG_RWPT                (REGISTER_READ | REGISTER_WRITE | REGISTER_PIPELINED | REGISTER_WRITETHRU)
#define REG_RF                  (REGISTER_READ | REGISTER_FIFO)
#define REG_WF                  (REGISTER_WRITE | REGISTER_FIFO)
#define REG_RWF                 (REGISTER_READ | REGISTER_WRITE | REGISTER_FIFO)
#define REG_RPF                 (REGISTER_READ | REGISTER_PIPELINED | REGISTER_FIFO)
#define REG_WPF                 (REGISTER_WRITE | REGISTER_PIPELINED | REGISTER_FIFO)
#define REG_RWPF                (REGISTER_READ | REGISTER_WRITE | REGISTER_PIPELINED | REGISTER_FIFO)

/* lookup bits is the log2 of the size of the reciprocal/log table */
#define RECIPLOG_LOOKUP_BITS    9

/* input precision is how many fraction bits the input value has; this is a 64-bit number */
#define RECIPLOG_INPUT_PREC     32

/* lookup precision is how many fraction bits each table entry contains */
#define RECIPLOG_LOOKUP_PREC    22

/* output precision is how many fraction bits the result should have */
#define RECIP_OUTPUT_PREC       15
#define LOG_OUTPUT_PREC         8



/*************************************
 *
 *  Register constants
 *
 *************************************/

struct rgba
{
#ifdef LSB_FIRST
	u8               b, g, r, a;
#else
	u8               a, r, g, b;
#endif
};

union voodoo_reg
{
	s32               i;
	u32              u;
	float               f;
	rgba                rgb;
};

namespace voodoo
{

class fbz_colorpath
{
public:
	static constexpr u32 DECODE_LIVE = 0xfffffffe;

	constexpr fbz_colorpath(u32 value) :
		m_value(value) { }

	constexpr fbz_colorpath(u32 normalized, u32 live) :
		m_value((normalized == DECODE_LIVE) ? live : normalized) { }

	constexpr u32 cc_rgbselect() const            { return BIT(m_value, 0, 2); }
	constexpr u32 cc_aselect() const              { return BIT(m_value, 2, 2); }
	constexpr u32 cc_localselect() const          { return BIT(m_value, 4, 1); }
	constexpr u32 cca_localselect() const         { return BIT(m_value, 5, 2); }
	constexpr u32 cc_localselect_override() const { return BIT(m_value, 7, 1); }
	constexpr u32 cc_zero_other() const           { return BIT(m_value, 8, 1); }
	constexpr u32 cc_sub_clocal() const           { return BIT(m_value, 9, 1); }
	constexpr u32 cc_mselect() const              { return BIT(m_value, 10, 3); }
	constexpr u32 cc_reverse_blend() const        { return BIT(m_value, 13, 1); }
	constexpr u32 cc_add_aclocal() const          { return BIT(m_value, 14, 2); }
	constexpr u32 cc_invert_output() const        { return BIT(m_value, 16, 1); }
	constexpr u32 cca_zero_other() const          { return BIT(m_value, 17, 1); }
	constexpr u32 cca_sub_clocal() const          { return BIT(m_value, 18, 1); }
	constexpr u32 cca_mselect() const             { return BIT(m_value, 19, 3); }
	constexpr u32 cca_reverse_blend() const       { return BIT(m_value, 22, 1); }
	constexpr u32 cca_add_aclocal() const         { return BIT(m_value, 23, 2); }
	constexpr u32 cca_invert_output() const       { return BIT(m_value, 25, 1); }
	constexpr u32 cca_subpixel_adjust() const     { return BIT(m_value, 26, 1); }
	constexpr u32 texture_enable() const          { return BIT(m_value, 27, 1); }
	constexpr u32 rgbzw_clamp() const             { return BIT(m_value, 28, 1); }
	constexpr u32 anti_alias() const              { return BIT(m_value, 29, 1); }

	constexpr u32 normalize()
	{
		// ignore the subpixel adjust and texture enable flags
		return m_value & ~((1 << 26) | (1 << 27));
	}

private:
	u32 m_value;
};

class fbz_mode
{
public:
	static constexpr u32 DECODE_LIVE = 0xfffffffe;

	constexpr fbz_mode(u32 value) :
		m_value(value) { }

	constexpr fbz_mode(u32 normalized, u32 live) :
		m_value((normalized == DECODE_LIVE) ? live : normalized) { }

	constexpr u32 enable_clipping() const       { return BIT(m_value, 0, 1); }
	constexpr u32 enable_chromakey() const      { return BIT(m_value, 1, 1); }
	constexpr u32 enable_stipple() const        { return BIT(m_value, 2, 1); }
	constexpr u32 wbuffer_select() const        { return BIT(m_value, 3, 1); }
	constexpr u32 enable_depthbuf() const       { return BIT(m_value, 4, 1); }
	constexpr u32 depth_function() const        { return BIT(m_value, 5, 3); }
	constexpr u32 enable_dithering() const      { return BIT(m_value, 8, 1); }
	constexpr u32 rgb_buffer_mask() const       { return BIT(m_value, 9, 1); }
	constexpr u32 aux_buffer_mask() const       { return BIT(m_value, 10, 1); }
	constexpr u32 dither_type() const           { return BIT(m_value, 11, 1); }
	constexpr u32 stipple_pattern() const       { return BIT(m_value, 12, 1); }
	constexpr u32 enable_alpha_mask() const     { return BIT(m_value, 13, 1); }
	constexpr u32 draw_buffer() const           { return BIT(m_value, 14, 2); }
	constexpr u32 enable_depth_bias() const     { return BIT(m_value, 16, 1); }
	constexpr u32 y_origin() const              { return BIT(m_value, 17, 1); }
	constexpr u32 enable_alpha_planes() const   { return BIT(m_value, 18, 1); }
	constexpr u32 alpha_dither_subtract() const { return BIT(m_value, 19, 1); }
	constexpr u32 depth_source_compare() const  { return BIT(m_value, 20, 1); }
	constexpr u32 depth_float_select() const    { return BIT(m_value, 21, 1); }	// voodoo 2 only

	constexpr u32 normalize()
	{
		// ignore the draw buffer
		return m_value & ~(3 << 14);
	}

private:
	u32 m_value;
};

class alpha_mode
{
public:
	static constexpr u32 DECODE_LIVE = 0xfffffffe;

	constexpr alpha_mode(u32 value) :
		m_value(value) { }

	constexpr alpha_mode(u32 normalized, u32 live) :
		m_value((normalized == DECODE_LIVE) ? live : ((normalized & 0x00ffffff) | (live & 0xff000000))) { }

	constexpr u32 alphatest() const     { return BIT(m_value, 0, 1); }
	constexpr u32 alphafunction() const { return BIT(m_value, 1, 3); }
	constexpr u32 alphablend() const    { return BIT(m_value, 4, 1); }
	constexpr u32 antialias() const     { return BIT(m_value, 5, 1); }
	constexpr u32 srcrgbblend() const   { return BIT(m_value, 8, 4); }
	constexpr u32 dstrgbblend() const   { return BIT(m_value, 12, 4); }
	constexpr u32 srcalphablend() const { return BIT(m_value, 16, 4); }
	constexpr u32 dstalphablend() const { return BIT(m_value, 20, 4); }
	constexpr u32 alpharef() const      { return BIT(m_value, 24, 8); }

	constexpr u32 normalize()
	{
		// always ignore alpha ref value
		u32 result = m_value & ~(0xff << 24);

		// if not doing alpha testing, ignore the alpha function
		if (!alphatest())
			result &= ~(7 << 1);

		// if not doing alpha blending, ignore the source and dest blending factors
		if (!alphablend())
			result &= ~((15 << 8) | (15 << 12) | (15 << 16) | (15 << 20));

		return result;
	}

private:
	u32 m_value;
};

class fog_mode
{
public:
	static constexpr u32 DECODE_LIVE = 0xfffffffe;

	constexpr fog_mode(u32 value) :
		m_value(value) { }

	constexpr fog_mode(u32 normalized, u32 live) :
		m_value((normalized == DECODE_LIVE) ? live : normalized) { }

	constexpr u32 enable_fog() const   { return BIT(m_value, 0, 1); }
	constexpr u32 fog_add() const      { return BIT(m_value, 1, 1); }
	constexpr u32 fog_mult() const     { return BIT(m_value, 2, 1); }
	constexpr u32 fog_zalpha() const   { return BIT(m_value, 3, 2); }
	constexpr u32 fog_constant() const { return BIT(m_value, 5, 1); }
	constexpr u32 fog_dither() const   { return BIT(m_value, 6, 1); }	// voodoo 2 only
	constexpr u32 fog_zones() const    { return BIT(m_value, 7, 1); }	// voodoo 2 only

	constexpr u32 normalize()
	{
		// only care about the rest if fog is enabled
		return enable_fog() ? m_value : 0;
	}

private:
	u32 m_value;
};

class texture_mode
{
public:
	static constexpr u32 DECODE_LIVE = 0xfffffffe;

	constexpr texture_mode(u32 value) :
		m_value(value) { }

	constexpr texture_mode(u32 normalized, u32 live) :
		m_value((normalized == DECODE_LIVE) ? live : normalized) { }

	constexpr u32 enable_perspective() const   { return BIT(m_value, 0, 1); }
	constexpr u32 minification_filter() const  { return BIT(m_value, 1, 1); }
	constexpr u32 magnification_filter() const { return BIT(m_value, 2, 1); }
	constexpr u32 clamp_neg_w() const          { return BIT(m_value, 3, 1); }
	constexpr u32 enable_lod_dither() const    { return BIT(m_value, 4, 1); }
	constexpr u32 ncc_table_select() const     { return BIT(m_value, 5, 1); }
	constexpr u32 clamp_s() const              { return BIT(m_value, 6, 1); }
	constexpr u32 clamp_t() const              { return BIT(m_value, 7, 1); }
	constexpr u32 format() const               { return BIT(m_value, 8, 4); }
	constexpr u32 tc_zero_other() const        { return BIT(m_value, 12, 1); }
	constexpr u32 tc_sub_clocal() const        { return BIT(m_value, 13, 1); }
	constexpr u32 tc_mselect() const           { return BIT(m_value, 14, 3); }
	constexpr u32 tc_reverse_blend() const     { return BIT(m_value, 17, 1); }
	constexpr u32 tc_add_aclocal() const       { return BIT(m_value, 18, 2); }
	constexpr u32 tc_invert_output() const     { return BIT(m_value, 20, 1); }
	constexpr u32 tca_zero_other() const       { return BIT(m_value, 21, 1); }
	constexpr u32 tca_sub_clocal() const       { return BIT(m_value, 22, 1); }
	constexpr u32 tca_mselect() const          { return BIT(m_value, 23, 3); }
	constexpr u32 tca_reverse_blend() const    { return BIT(m_value, 26, 1); }
	constexpr u32 tca_add_aclocal() const      { return BIT(m_value, 27, 2); }
	constexpr u32 tca_invert_output() const    { return BIT(m_value, 29, 1); }
	constexpr u32 trilinear() const            { return BIT(m_value, 30, 1); }
	constexpr u32 seq_8_downld() const         { return BIT(m_value, 31, 1); }

	constexpr u32 normalize()
	{
		// ignore the NCC table and seq_8_downld flags
		u32 result = m_value & ~((1 << 5) | (1 << 31));

		// classify texture formats into 3 format categories
		if (format() < 8)
			result = (result & ~(0xf << 8)) | (0 << 8);
		else if (format() >= 10 && format() <= 12)
			result = (result & ~(0xf << 8)) | (10 << 8);
		else
			result = (result & ~(0xf << 8)) | (8 << 8);

		return result;
	}

private:
	u32 m_value;
};

}

class voodoo_regs
{
public:
	voodoo_regs();

	voodoo_reg &operator[](int index) { return m_regs[index]; }

private:
	voodoo_reg m_regs[0x400];
};

/* Codes to the right:
    R = readable
    W = writeable
    P = pipelined
    F = goes to FIFO
*/

/* 0x000 */
#define vdstatus        (0x000/4)   /* R  P  */
#define intrCtrl        (0x004/4)   /* RW P   -- Voodoo2/Banshee only */
#define vertexAx        (0x008/4)   /*  W PF */
#define vertexAy        (0x00c/4)   /*  W PF */
#define vertexBx        (0x010/4)   /*  W PF */
#define vertexBy        (0x014/4)   /*  W PF */
#define vertexCx        (0x018/4)   /*  W PF */
#define vertexCy        (0x01c/4)   /*  W PF */
#define startR          (0x020/4)   /*  W PF */
#define startG          (0x024/4)   /*  W PF */
#define startB          (0x028/4)   /*  W PF */
#define startZ          (0x02c/4)   /*  W PF */
#define startA          (0x030/4)   /*  W PF */
#define startS          (0x034/4)   /*  W PF */
#define startT          (0x038/4)   /*  W PF */
#define startW          (0x03c/4)   /*  W PF */

/* 0x040 */
#define dRdX            (0x040/4)   /*  W PF */
#define dGdX            (0x044/4)   /*  W PF */
#define dBdX            (0x048/4)   /*  W PF */
#define dZdX            (0x04c/4)   /*  W PF */
#define dAdX            (0x050/4)   /*  W PF */
#define dSdX            (0x054/4)   /*  W PF */
#define dTdX            (0x058/4)   /*  W PF */
#define dWdX            (0x05c/4)   /*  W PF */
#define dRdY            (0x060/4)   /*  W PF */
#define dGdY            (0x064/4)   /*  W PF */
#define dBdY            (0x068/4)   /*  W PF */
#define dZdY            (0x06c/4)   /*  W PF */
#define dAdY            (0x070/4)   /*  W PF */
#define dSdY            (0x074/4)   /*  W PF */
#define dTdY            (0x078/4)   /*  W PF */
#define dWdY            (0x07c/4)   /*  W PF */

/* 0x080 */
#define triangleCMD     (0x080/4)   /*  W PF */
#define fvertexAx       (0x088/4)   /*  W PF */
#define fvertexAy       (0x08c/4)   /*  W PF */
#define fvertexBx       (0x090/4)   /*  W PF */
#define fvertexBy       (0x094/4)   /*  W PF */
#define fvertexCx       (0x098/4)   /*  W PF */
#define fvertexCy       (0x09c/4)   /*  W PF */
#define fstartR         (0x0a0/4)   /*  W PF */
#define fstartG         (0x0a4/4)   /*  W PF */
#define fstartB         (0x0a8/4)   /*  W PF */
#define fstartZ         (0x0ac/4)   /*  W PF */
#define fstartA         (0x0b0/4)   /*  W PF */
#define fstartS         (0x0b4/4)   /*  W PF */
#define fstartT         (0x0b8/4)   /*  W PF */
#define fstartW         (0x0bc/4)   /*  W PF */

/* 0x0c0 */
#define fdRdX           (0x0c0/4)   /*  W PF */
#define fdGdX           (0x0c4/4)   /*  W PF */
#define fdBdX           (0x0c8/4)   /*  W PF */
#define fdZdX           (0x0cc/4)   /*  W PF */
#define fdAdX           (0x0d0/4)   /*  W PF */
#define fdSdX           (0x0d4/4)   /*  W PF */
#define fdTdX           (0x0d8/4)   /*  W PF */
#define fdWdX           (0x0dc/4)   /*  W PF */
#define fdRdY           (0x0e0/4)   /*  W PF */
#define fdGdY           (0x0e4/4)   /*  W PF */
#define fdBdY           (0x0e8/4)   /*  W PF */
#define fdZdY           (0x0ec/4)   /*  W PF */
#define fdAdY           (0x0f0/4)   /*  W PF */
#define fdSdY           (0x0f4/4)   /*  W PF */
#define fdTdY           (0x0f8/4)   /*  W PF */
#define fdWdY           (0x0fc/4)   /*  W PF */

/* 0x100 */
#define ftriangleCMD    (0x100/4)   /*  W PF */
#define fbzColorPath    (0x104/4)   /* RW PF */
#define fogMode         (0x108/4)   /* RW PF */
#define alphaMode       (0x10c/4)   /* RW PF */
#define fbzMode         (0x110/4)   /* RW  F */
#define lfbMode         (0x114/4)   /* RW  F */
#define clipLeftRight   (0x118/4)   /* RW  F */
#define clipLowYHighY   (0x11c/4)   /* RW  F */
#define nopCMD          (0x120/4)   /*  W  F */
#define fastfillCMD     (0x124/4)   /*  W  F */
#define swapbufferCMD   (0x128/4)   /*  W  F */
#define fogColor        (0x12c/4)   /*  W  F */
#define zaColor         (0x130/4)   /*  W  F */
#define chromaKey       (0x134/4)   /*  W  F */
#define chromaRange     (0x138/4)   /*  W  F  -- Voodoo2/Banshee only */
#define userIntrCMD     (0x13c/4)   /*  W  F  -- Voodoo2/Banshee only */

/* 0x140 */
#define stipple         (0x140/4)   /* RW  F */
#define color0          (0x144/4)   /* RW  F */
#define color1          (0x148/4)   /* RW  F */
#define fbiPixelsIn     (0x14c/4)   /* R     */
#define fbiChromaFail   (0x150/4)   /* R     */
#define fbiZfuncFail    (0x154/4)   /* R     */
#define fbiAfuncFail    (0x158/4)   /* R     */
#define fbiPixelsOut    (0x15c/4)   /* R     */
#define fogTable        (0x160/4)   /*  W  F */

/* 0x1c0 */
#define cmdFifoBaseAddr (0x1e0/4)   /* RW     -- Voodoo2 only */
#define cmdFifoBump     (0x1e4/4)   /* RW     -- Voodoo2 only */
#define cmdFifoRdPtr    (0x1e8/4)   /* RW     -- Voodoo2 only */
#define cmdFifoAMin     (0x1ec/4)   /* RW     -- Voodoo2 only */
#define colBufferAddr   (0x1ec/4)   /* RW     -- Banshee only */
#define cmdFifoAMax     (0x1f0/4)   /* RW     -- Voodoo2 only */
#define colBufferStride (0x1f0/4)   /* RW     -- Banshee only */
#define cmdFifoDepth    (0x1f4/4)   /* RW     -- Voodoo2 only */
#define auxBufferAddr   (0x1f4/4)   /* RW     -- Banshee only */
#define cmdFifoHoles    (0x1f8/4)   /* RW     -- Voodoo2 only */
#define auxBufferStride (0x1f8/4)   /* RW     -- Banshee only */

/* 0x200 */
#define fbiInit4        (0x200/4)   /* RW     -- Voodoo/Voodoo2 only */
#define clipLeftRight1  (0x200/4)   /* RW     -- Banshee only */
#define vRetrace        (0x204/4)   /* R      -- Voodoo/Voodoo2 only */
#define clipTopBottom1  (0x204/4)   /* RW     -- Banshee only */
#define backPorch       (0x208/4)   /* RW     -- Voodoo/Voodoo2 only */
#define videoDimensions (0x20c/4)   /* RW     -- Voodoo/Voodoo2 only */
#define fbiInit0        (0x210/4)   /* RW     -- Voodoo/Voodoo2 only */
#define fbiInit1        (0x214/4)   /* RW     -- Voodoo/Voodoo2 only */
#define fbiInit2        (0x218/4)   /* RW     -- Voodoo/Voodoo2 only */
#define fbiInit3        (0x21c/4)   /* RW     -- Voodoo/Voodoo2 only */
#define hSync           (0x220/4)   /*  W     -- Voodoo/Voodoo2 only */
#define vSync           (0x224/4)   /*  W     -- Voodoo/Voodoo2 only */
#define clutData        (0x228/4)   /*  W  F  -- Voodoo/Voodoo2 only */
#define dacData         (0x22c/4)   /*  W     -- Voodoo/Voodoo2 only */
#define maxRgbDelta     (0x230/4)   /*  W     -- Voodoo/Voodoo2 only */
#define hBorder         (0x234/4)   /*  W     -- Voodoo2 only */
#define vBorder         (0x238/4)   /*  W     -- Voodoo2 only */
#define borderColor     (0x23c/4)   /*  W     -- Voodoo2 only */

/* 0x240 */
#define hvRetrace       (0x240/4)   /* R      -- Voodoo2 only */
#define fbiInit5        (0x244/4)   /* RW     -- Voodoo2 only */
#define fbiInit6        (0x248/4)   /* RW     -- Voodoo2 only */
#define fbiInit7        (0x24c/4)   /* RW     -- Voodoo2 only */
#define swapPending     (0x24c/4)   /*  W     -- Banshee only */
#define leftOverlayBuf  (0x250/4)   /*  W     -- Banshee only */
#define rightOverlayBuf (0x254/4)   /*  W     -- Banshee only */
#define fbiSwapHistory  (0x258/4)   /* R      -- Voodoo2/Banshee only */
#define fbiTrianglesOut (0x25c/4)   /* R      -- Voodoo2/Banshee only */
#define sSetupMode      (0x260/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sVx             (0x264/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sVy             (0x268/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sARGB           (0x26c/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sRed            (0x270/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sGreen          (0x274/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sBlue           (0x278/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sAlpha          (0x27c/4)   /*  W PF  -- Voodoo2/Banshee only */

/* 0x280 */
#define sVz             (0x280/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sWb             (0x284/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sWtmu0          (0x288/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sS_W0           (0x28c/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sT_W0           (0x290/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sWtmu1          (0x294/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sS_Wtmu1        (0x298/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sT_Wtmu1        (0x29c/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sDrawTriCMD     (0x2a0/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sBeginTriCMD    (0x2a4/4)   /*  W PF  -- Voodoo2/Banshee only */

/* 0x2c0 */
#define bltSrcBaseAddr  (0x2c0/4)   /* RW PF  -- Voodoo2 only */
#define bltDstBaseAddr  (0x2c4/4)   /* RW PF  -- Voodoo2 only */
#define bltXYStrides    (0x2c8/4)   /* RW PF  -- Voodoo2 only */
#define bltSrcChromaRange (0x2cc/4) /* RW PF  -- Voodoo2 only */
#define bltDstChromaRange (0x2d0/4) /* RW PF  -- Voodoo2 only */
#define bltClipX        (0x2d4/4)   /* RW PF  -- Voodoo2 only */
#define bltClipY        (0x2d8/4)   /* RW PF  -- Voodoo2 only */
#define bltSrcXY        (0x2e0/4)   /* RW PF  -- Voodoo2 only */
#define bltDstXY        (0x2e4/4)   /* RW PF  -- Voodoo2 only */
#define bltSize         (0x2e8/4)   /* RW PF  -- Voodoo2 only */
#define bltRop          (0x2ec/4)   /* RW PF  -- Voodoo2 only */
#define bltColor        (0x2f0/4)   /* RW PF  -- Voodoo2 only */
#define bltCommand      (0x2f8/4)   /* RW PF  -- Voodoo2 only */
#define bltData         (0x2fc/4)   /*  W PF  -- Voodoo2 only */

/* 0x300 */
#define textureMode     (0x300/4)   /*  W PF */
#define tLOD            (0x304/4)   /*  W PF */
#define tDetail         (0x308/4)   /*  W PF */
#define texBaseAddr     (0x30c/4)   /*  W PF */
#define texBaseAddr_1   (0x310/4)   /*  W PF */
#define texBaseAddr_2   (0x314/4)   /*  W PF */
#define texBaseAddr_3_8 (0x318/4)   /*  W PF */
#define trexInit0       (0x31c/4)   /*  W  F  -- Voodoo/Voodoo2 only */
#define trexInit1       (0x320/4)   /*  W  F */
#define nccTable        (0x324/4)   /*  W  F */



// 2D registers
#define banshee2D_clip0Min          (0x008/4)
#define banshee2D_clip0Max          (0x00c/4)
#define banshee2D_dstBaseAddr       (0x010/4)
#define banshee2D_dstFormat         (0x014/4)
#define banshee2D_srcColorkeyMin    (0x018/4)
#define banshee2D_srcColorkeyMax    (0x01c/4)
#define banshee2D_dstColorkeyMin    (0x020/4)
#define banshee2D_dstColorkeyMax    (0x024/4)
#define banshee2D_bresError0        (0x028/4)
#define banshee2D_bresError1        (0x02c/4)
#define banshee2D_rop               (0x030/4)
#define banshee2D_srcBaseAddr       (0x034/4)
#define banshee2D_commandExtra      (0x038/4)
#define banshee2D_lineStipple       (0x03c/4)
#define banshee2D_lineStyle         (0x040/4)
#define banshee2D_pattern0Alias     (0x044/4)
#define banshee2D_pattern1Alias     (0x048/4)
#define banshee2D_clip1Min          (0x04c/4)
#define banshee2D_clip1Max          (0x050/4)
#define banshee2D_srcFormat         (0x054/4)
#define banshee2D_srcSize           (0x058/4)
#define banshee2D_srcXY             (0x05c/4)
#define banshee2D_colorBack         (0x060/4)
#define banshee2D_colorFore         (0x064/4)
#define banshee2D_dstSize           (0x068/4)
#define banshee2D_dstXY             (0x06c/4)
#define banshee2D_command           (0x070/4)


/*************************************
 *
 *  Voodoo Banshee I/O space registers
 *
 *************************************/

/* 0x000 */
#define io_status                       (0x000/4)   /*  */
#define io_pciInit0                     (0x004/4)   /*  */
#define io_sipMonitor                   (0x008/4)   /*  */
#define io_lfbMemoryConfig              (0x00c/4)   /*  */
#define io_miscInit0                    (0x010/4)   /*  */
#define io_miscInit1                    (0x014/4)   /*  */
#define io_dramInit0                    (0x018/4)   /*  */
#define io_dramInit1                    (0x01c/4)   /*  */
#define io_agpInit                      (0x020/4)   /*  */
#define io_tmuGbeInit                   (0x024/4)   /*  */
#define io_vgaInit0                     (0x028/4)   /*  */
#define io_vgaInit1                     (0x02c/4)   /*  */
#define io_dramCommand                  (0x030/4)   /*  */
#define io_dramData                     (0x034/4)   /*  */

/* 0x040 */
#define io_pllCtrl0                     (0x040/4)   /*  */
#define io_pllCtrl1                     (0x044/4)   /*  */
#define io_pllCtrl2                     (0x048/4)   /*  */
#define io_dacMode                      (0x04c/4)   /*  */
#define io_dacAddr                      (0x050/4)   /*  */
#define io_dacData                      (0x054/4)   /*  */
#define io_rgbMaxDelta                  (0x058/4)   /*  */
#define io_vidProcCfg                   (0x05c/4)   /*  */
#define io_hwCurPatAddr                 (0x060/4)   /*  */
#define io_hwCurLoc                     (0x064/4)   /*  */
#define io_hwCurC0                      (0x068/4)   /*  */
#define io_hwCurC1                      (0x06c/4)   /*  */
#define io_vidInFormat                  (0x070/4)   /*  */
#define io_vidInStatus                  (0x074/4)   /*  */
#define io_vidSerialParallelPort        (0x078/4)   /*  */
#define io_vidInXDecimDeltas            (0x07c/4)   /*  */

/* 0x080 */
#define io_vidInDecimInitErrs           (0x080/4)   /*  */
#define io_vidInYDecimDeltas            (0x084/4)   /*  */
#define io_vidPixelBufThold             (0x088/4)   /*  */
#define io_vidChromaMin                 (0x08c/4)   /*  */
#define io_vidChromaMax                 (0x090/4)   /*  */
#define io_vidCurrentLine               (0x094/4)   /*  */
#define io_vidScreenSize                (0x098/4)   /*  */
#define io_vidOverlayStartCoords        (0x09c/4)   /*  */
#define io_vidOverlayEndScreenCoord     (0x0a0/4)   /*  */
#define io_vidOverlayDudx               (0x0a4/4)   /*  */
#define io_vidOverlayDudxOffsetSrcWidth (0x0a8/4)   /*  */
#define io_vidOverlayDvdy               (0x0ac/4)   /*  */
#define io_vgab0                        (0x0b0/4)   /*  */
#define io_vgab4                        (0x0b4/4)   /*  */
#define io_vgab8                        (0x0b8/4)   /*  */
#define io_vgabc                        (0x0bc/4)   /*  */

/* 0x0c0 */
#define io_vgac0                        (0x0c0/4)   /*  */
#define io_vgac4                        (0x0c4/4)   /*  */
#define io_vgac8                        (0x0c8/4)   /*  */
#define io_vgacc                        (0x0cc/4)   /*  */
#define io_vgad0                        (0x0d0/4)   /*  */
#define io_vgad4                        (0x0d4/4)   /*  */
#define io_vgad8                        (0x0d8/4)   /*  */
#define io_vgadc                        (0x0dc/4)   /*  */
#define io_vidOverlayDvdyOffset         (0x0e0/4)   /*  */
#define io_vidDesktopStartAddr          (0x0e4/4)   /*  */
#define io_vidDesktopOverlayStride      (0x0e8/4)   /*  */
#define io_vidInAddr0                   (0x0ec/4)   /*  */
#define io_vidInAddr1                   (0x0f0/4)   /*  */
#define io_vidInAddr2                   (0x0f4/4)   /*  */
#define io_vidInStride                  (0x0f8/4)   /*  */
#define io_vidCurrOverlayStartAddr      (0x0fc/4)   /*  */



/*************************************
 *
 *  Voodoo Banshee AGP space registers
 *
 *************************************/

/* 0x000 */
#define agpReqSize              (0x000/4)   /*  */
#define agpHostAddressLow       (0x004/4)   /*  */
#define agpHostAddressHigh      (0x008/4)   /*  */
#define agpGraphicsAddress      (0x00c/4)   /*  */
#define agpGraphicsStride       (0x010/4)   /*  */
#define agpMoveCMD              (0x014/4)   /*  */
#define cmdBaseAddr0            (0x020/4)   /*  */
#define cmdBaseSize0            (0x024/4)   /*  */
#define cmdBump0                (0x028/4)   /*  */
#define cmdRdPtrL0              (0x02c/4)   /*  */
#define cmdRdPtrH0              (0x030/4)   /*  */
#define cmdAMin0                (0x034/4)   /*  */
#define cmdAMax0                (0x03c/4)   /*  */

/* 0x040 */
#define cmdFifoDepth0           (0x044/4)   /*  */
#define cmdHoleCnt0             (0x048/4)   /*  */
#define cmdBaseAddr1            (0x050/4)   /*  */
#define cmdBaseSize1            (0x054/4)   /*  */
#define cmdBump1                (0x058/4)   /*  */
#define cmdRdPtrL1              (0x05c/4)   /*  */
#define cmdRdPtrH1              (0x060/4)   /*  */
#define cmdAMin1                (0x064/4)   /*  */
#define cmdAMax1                (0x06c/4)   /*  */
#define cmdFifoDepth1           (0x074/4)   /*  */
#define cmdHoleCnt1             (0x078/4)   /*  */

/* 0x080 */
#define cmdFifoThresh           (0x080/4)   /*  */
#define cmdHoleInt              (0x084/4)   /*  */

/* 0x100 */
#define yuvBaseAddress          (0x100/4)   /*  */
#define yuvStride               (0x104/4)   /*  */
#define crc1                    (0x120/4)   /*  */
#define crc2                    (0x130/4)   /*  */



/*************************************
 *
 *  Dithering tables
 *
 *************************************/

static const u8 dither_matrix_4x4[16] =
{
	 0,  8,  2, 10,
	12,  4, 14,  6,
	 3, 11,  1,  9,
	15,  7, 13,  5
};

//static const u8 dither_matrix_2x2[16] =
//{
//      2, 10,  2, 10,
//  14,  6, 14,  6,
//      2, 10,  2, 10,
//  14,  6, 14,  6
//};
// Using this matrix allows iteagle video memory tests to pass
static const u8 dither_matrix_2x2[16] =
{
	8, 10, 8, 10,
	11, 9, 11, 9,
	8, 10, 8, 10,
	11, 9, 11, 9
};

// Dither 4x4 subtraction matrix used in alpha blending
static const u8 dither_subtract_4x4[16] =
{
	(15 - 0) >> 1,  (15 - 8) >> 1,  (15 - 2) >> 1, (15 - 10) >> 1,
	(15 - 12) >> 1,  (15 - 4) >> 1, (15 - 14) >> 1,  (15 - 6) >> 1,
	(15 - 3) >> 1, (15 - 11) >> 1,  (15 - 1) >> 1,  (15 - 9) >> 1,
	(15 - 15) >> 1,  (15 - 7) >> 1, (15 - 13) >> 1,  (15 - 5) >> 1
};

// Dither 2x2 subtraction matrix used in alpha blending
static const u8 dither_subtract_2x2[16] =
{
	(15 - 8) >> 1, (15 - 10) >> 1, (15 - 8) >> 1, (15 - 10) >> 1,
	(15 - 11) >> 1, (15 - 9) >> 1, (15 - 11) >> 1, (15 - 9) >> 1,
	(15 - 8) >> 1, (15 - 10) >> 1, (15 - 8) >> 1, (15 - 10) >> 1,
	(15 - 11) >> 1, (15 - 9) >> 1, (15 - 11) >> 1, (15 - 9) >> 1
};

/*************************************
 *
 *  Macros for extracting pixels
 *
 *************************************/

#define EXTRACT_565_TO_888(val, a, b, c)                    \
	(a) = (((val) >> 8) & 0xf8) | (((val) >> 13) & 0x07);   \
	(b) = (((val) >> 3) & 0xfc) | (((val) >> 9) & 0x03);    \
	(c) = (((val) << 3) & 0xf8) | (((val) >> 2) & 0x07);
#define EXTRACT_x555_TO_888(val, a, b, c)                   \
	(a) = (((val) >> 7) & 0xf8) | (((val) >> 12) & 0x07);   \
	(b) = (((val) >> 2) & 0xf8) | (((val) >> 7) & 0x07);    \
	(c) = (((val) << 3) & 0xf8) | (((val) >> 2) & 0x07);
#define EXTRACT_555x_TO_888(val, a, b, c)                   \
	(a) = (((val) >> 8) & 0xf8) | (((val) >> 13) & 0x07);   \
	(b) = (((val) >> 3) & 0xf8) | (((val) >> 8) & 0x07);    \
	(c) = (((val) << 2) & 0xf8) | (((val) >> 3) & 0x07);
#define EXTRACT_1555_TO_8888(val, a, b, c, d)               \
	(a) = ((s16)(val) >> 15) & 0xff;                      \
	EXTRACT_x555_TO_888(val, b, c, d)
#define EXTRACT_5551_TO_8888(val, a, b, c, d)               \
	EXTRACT_555x_TO_888(val, a, b, c)                       \
	(d) = ((val) & 0x0001) ? 0xff : 0x00;
#define EXTRACT_x888_TO_888(val, a, b, c)                   \
	(a) = ((val) >> 16) & 0xff;                             \
	(b) = ((val) >> 8) & 0xff;                              \
	(c) = ((val) >> 0) & 0xff;
#define EXTRACT_888x_TO_888(val, a, b, c)                   \
	(a) = ((val) >> 24) & 0xff;                             \
	(b) = ((val) >> 16) & 0xff;                             \
	(c) = ((val) >> 8) & 0xff;
#define EXTRACT_8888_TO_8888(val, a, b, c, d)               \
	(a) = ((val) >> 24) & 0xff;                             \
	(b) = ((val) >> 16) & 0xff;                             \
	(c) = ((val) >> 8) & 0xff;                              \
	(d) = ((val) >> 0) & 0xff;
#define EXTRACT_4444_TO_8888(val, a, b, c, d)               \
	(a) = (((val) >> 8) & 0xf0) | (((val) >> 12) & 0x0f);   \
	(b) = (((val) >> 4) & 0xf0) | (((val) >> 8) & 0x0f);    \
	(c) = (((val) >> 0) & 0xf0) | (((val) >> 4) & 0x0f);    \
	(d) = (((val) << 4) & 0xf0) | (((val) >> 0) & 0x0f);
#define EXTRACT_332_TO_888(val, a, b, c)                    \
	(a) = (((val) >> 0) & 0xe0) | (((val) >> 3) & 0x1c) | (((val) >> 6) & 0x03); \
	(b) = (((val) << 3) & 0xe0) | (((val) >> 0) & 0x1c) | (((val) >> 3) & 0x03); \
	(c) = (((val) << 6) & 0xc0) | (((val) << 4) & 0x30) | (((val) << 2) & 0x0c) | (((val) << 0) & 0x03);


/*************************************
 *
 *  Misc. macros
 *
 *************************************/

/* macro for clamping a value between minimum and maximum values */
#define CLAMP(val,min,max)      do { if ((val) < (min)) { (val) = (min); } else if ((val) > (max)) { (val) = (max); } } while (0)

/* macro to compute the base 2 log for LOD calculations */
#define LOGB2(x)                (log((double)(x)) / log(2.0))



/*************************************
 *
 *  Macros for extracting bitfields
 *
 *************************************/

#define INITEN_ENABLE_HW_INIT(val)          (((val) >> 0) & 1)
#define INITEN_ENABLE_PCI_FIFO(val)         (((val) >> 1) & 1)
#define INITEN_REMAP_INIT_TO_DAC(val)       (((val) >> 2) & 1)
#define INITEN_ENABLE_SNOOP0(val)           (((val) >> 4) & 1)
#define INITEN_SNOOP0_MEMORY_MATCH(val)     (((val) >> 5) & 1)
#define INITEN_SNOOP0_READWRITE_MATCH(val)  (((val) >> 6) & 1)
#define INITEN_ENABLE_SNOOP1(val)           (((val) >> 7) & 1)
#define INITEN_SNOOP1_MEMORY_MATCH(val)     (((val) >> 8) & 1)
#define INITEN_SNOOP1_READWRITE_MATCH(val)  (((val) >> 9) & 1)
#define INITEN_SLI_BUS_OWNER(val)           (((val) >> 10) & 1)
#define INITEN_SLI_ODD_EVEN(val)            (((val) >> 11) & 1)
#define INITEN_SECONDARY_REV_ID(val)        (((val) >> 12) & 0xf)   /* voodoo 2 only */
#define INITEN_MFCTR_FAB_ID(val)            (((val) >> 16) & 0xf)   /* voodoo 2 only */
#define INITEN_ENABLE_PCI_INTERRUPT(val)    (((val) >> 20) & 1)     /* voodoo 2 only */
#define INITEN_PCI_INTERRUPT_TIMEOUT(val)   (((val) >> 21) & 1)     /* voodoo 2 only */
#define INITEN_ENABLE_NAND_TREE_TEST(val)   (((val) >> 22) & 1)     /* voodoo 2 only */
#define INITEN_ENABLE_SLI_ADDRESS_SNOOP(val) (((val) >> 23) & 1)    /* voodoo 2 only */
#define INITEN_SLI_SNOOP_ADDRESS(val)       (((val) >> 24) & 0xff)  /* voodoo 2 only */

#define LFBMODE_WRITE_FORMAT(val)           (((val) >> 0) & 0xf)
#define LFBMODE_WRITE_BUFFER_SELECT(val)    (((val) >> 4) & 3)
#define LFBMODE_READ_BUFFER_SELECT(val)     (((val) >> 6) & 3)
#define LFBMODE_ENABLE_PIXEL_PIPELINE(val)  (((val) >> 8) & 1)
#define LFBMODE_RGBA_LANES(val)             (((val) >> 9) & 3)
#define LFBMODE_WORD_SWAP_WRITES(val)       (((val) >> 11) & 1)
#define LFBMODE_BYTE_SWIZZLE_WRITES(val)    (((val) >> 12) & 1)
#define LFBMODE_Y_ORIGIN(val)               (((val) >> 13) & 1)
#define LFBMODE_WRITE_W_SELECT(val)         (((val) >> 14) & 1)
#define LFBMODE_WORD_SWAP_READS(val)        (((val) >> 15) & 1)
#define LFBMODE_BYTE_SWIZZLE_READS(val)     (((val) >> 16) & 1)

#define CHROMARANGE_BLUE_EXCLUSIVE(val)     (((val) >> 24) & 1)
#define CHROMARANGE_GREEN_EXCLUSIVE(val)    (((val) >> 25) & 1)
#define CHROMARANGE_RED_EXCLUSIVE(val)      (((val) >> 26) & 1)
#define CHROMARANGE_UNION_MODE(val)         (((val) >> 27) & 1)
#define CHROMARANGE_ENABLE(val)             (((val) >> 28) & 1)

#define FBIINIT0_VGA_PASSTHRU(val)          (((val) >> 0) & 1)
#define FBIINIT0_GRAPHICS_RESET(val)        (((val) >> 1) & 1)
#define FBIINIT0_FIFO_RESET(val)            (((val) >> 2) & 1)
#define FBIINIT0_SWIZZLE_REG_WRITES(val)    (((val) >> 3) & 1)
#define FBIINIT0_STALL_PCIE_FOR_HWM(val)    (((val) >> 4) & 1)
#define FBIINIT0_PCI_FIFO_LWM(val)          (((val) >> 6) & 0x1f)
#define FBIINIT0_LFB_TO_MEMORY_FIFO(val)    (((val) >> 11) & 1)
#define FBIINIT0_TEXMEM_TO_MEMORY_FIFO(val) (((val) >> 12) & 1)
#define FBIINIT0_ENABLE_MEMORY_FIFO(val)    (((val) >> 13) & 1)
#define FBIINIT0_MEMORY_FIFO_HWM(val)       (((val) >> 14) & 0x7ff)
#define FBIINIT0_MEMORY_FIFO_BURST(val)     (((val) >> 25) & 0x3f)

#define FBIINIT1_PCI_DEV_FUNCTION(val)      (((val) >> 0) & 1)
#define FBIINIT1_PCI_WRITE_WAIT_STATES(val) (((val) >> 1) & 1)
#define FBIINIT1_MULTI_SST1(val)            (((val) >> 2) & 1)      /* not on voodoo 2 */
#define FBIINIT1_ENABLE_LFB(val)            (((val) >> 3) & 1)
#define FBIINIT1_X_VIDEO_TILES(val)         (((val) >> 4) & 0xf)
#define FBIINIT1_VIDEO_TIMING_RESET(val)    (((val) >> 8) & 1)
#define FBIINIT1_SOFTWARE_OVERRIDE(val)     (((val) >> 9) & 1)
#define FBIINIT1_SOFTWARE_HSYNC(val)        (((val) >> 10) & 1)
#define FBIINIT1_SOFTWARE_VSYNC(val)        (((val) >> 11) & 1)
#define FBIINIT1_SOFTWARE_BLANK(val)        (((val) >> 12) & 1)
#define FBIINIT1_DRIVE_VIDEO_TIMING(val)    (((val) >> 13) & 1)
#define FBIINIT1_DRIVE_VIDEO_BLANK(val)     (((val) >> 14) & 1)
#define FBIINIT1_DRIVE_VIDEO_SYNC(val)      (((val) >> 15) & 1)
#define FBIINIT1_DRIVE_VIDEO_DCLK(val)      (((val) >> 16) & 1)
#define FBIINIT1_VIDEO_TIMING_VCLK(val)     (((val) >> 17) & 1)
#define FBIINIT1_VIDEO_CLK_2X_DELAY(val)    (((val) >> 18) & 3)
#define FBIINIT1_VIDEO_TIMING_SOURCE(val)   (((val) >> 20) & 3)
#define FBIINIT1_ENABLE_24BPP_OUTPUT(val)   (((val) >> 22) & 1)
#define FBIINIT1_ENABLE_SLI(val)            (((val) >> 23) & 1)
#define FBIINIT1_X_VIDEO_TILES_BIT5(val)    (((val) >> 24) & 1)     /* voodoo 2 only */
#define FBIINIT1_ENABLE_EDGE_FILTER(val)    (((val) >> 25) & 1)
#define FBIINIT1_INVERT_VID_CLK_2X(val)     (((val) >> 26) & 1)
#define FBIINIT1_VID_CLK_2X_SEL_DELAY(val)  (((val) >> 27) & 3)
#define FBIINIT1_VID_CLK_DELAY(val)         (((val) >> 29) & 3)
#define FBIINIT1_DISABLE_FAST_READAHEAD(val) (((val) >> 31) & 1)

#define FBIINIT2_DISABLE_DITHER_SUB(val)    (((val) >> 0) & 1)
#define FBIINIT2_DRAM_BANKING(val)          (((val) >> 1) & 1)
#define FBIINIT2_ENABLE_TRIPLE_BUF(val)     (((val) >> 4) & 1)
#define FBIINIT2_ENABLE_FAST_RAS_READ(val)  (((val) >> 5) & 1)
#define FBIINIT2_ENABLE_GEN_DRAM_OE(val)    (((val) >> 6) & 1)
#define FBIINIT2_ENABLE_FAST_READWRITE(val) (((val) >> 7) & 1)
#define FBIINIT2_ENABLE_PASSTHRU_DITHER(val) (((val) >> 8) & 1)
#define FBIINIT2_SWAP_BUFFER_ALGORITHM(val) (((val) >> 9) & 3)
#define FBIINIT2_VIDEO_BUFFER_OFFSET(val)   (((val) >> 11) & 0x1ff)
#define FBIINIT2_ENABLE_DRAM_BANKING(val)   (((val) >> 20) & 1)
#define FBIINIT2_ENABLE_DRAM_READ_FIFO(val) (((val) >> 21) & 1)
#define FBIINIT2_ENABLE_DRAM_REFRESH(val)   (((val) >> 22) & 1)
#define FBIINIT2_REFRESH_LOAD_VALUE(val)    (((val) >> 23) & 0x1ff)

#define FBIINIT3_TRI_REGISTER_REMAP(val)    (((val) >> 0) & 1)
#define FBIINIT3_VIDEO_FIFO_THRESH(val)     (((val) >> 1) & 0x1f)
#define FBIINIT3_DISABLE_TMUS(val)          (((val) >> 6) & 1)
#define FBIINIT3_FBI_MEMORY_TYPE(val)       (((val) >> 8) & 7)
#define FBIINIT3_VGA_PASS_RESET_VAL(val)    (((val) >> 11) & 1)
#define FBIINIT3_HARDCODE_PCI_BASE(val)     (((val) >> 12) & 1)
#define FBIINIT3_FBI2TREX_DELAY(val)        (((val) >> 13) & 0xf)
#define FBIINIT3_TREX2FBI_DELAY(val)        (((val) >> 17) & 0x1f)
#define FBIINIT3_YORIGIN_SUBTRACT(val)      (((val) >> 22) & 0x3ff)

#define FBIINIT4_PCI_READ_WAITS(val)        (((val) >> 0) & 1)
#define FBIINIT4_ENABLE_LFB_READAHEAD(val)  (((val) >> 1) & 1)
#define FBIINIT4_MEMORY_FIFO_LWM(val)       (((val) >> 2) & 0x3f)
#define FBIINIT4_MEMORY_FIFO_START_ROW(val) (((val) >> 8) & 0x3ff)
#define FBIINIT4_MEMORY_FIFO_STOP_ROW(val)  (((val) >> 18) & 0x3ff)
#define FBIINIT4_VIDEO_CLOCKING_DELAY(val)  (((val) >> 29) & 7)     /* voodoo 2 only */

#define FBIINIT5_DISABLE_PCI_STOP(val)      (((val) >> 0) & 1)      /* voodoo 2 only */
#define FBIINIT5_PCI_SLAVE_SPEED(val)       (((val) >> 1) & 1)      /* voodoo 2 only */
#define FBIINIT5_DAC_DATA_OUTPUT_WIDTH(val) (((val) >> 2) & 1)      /* voodoo 2 only */
#define FBIINIT5_DAC_DATA_17_OUTPUT(val)    (((val) >> 3) & 1)      /* voodoo 2 only */
#define FBIINIT5_DAC_DATA_18_OUTPUT(val)    (((val) >> 4) & 1)      /* voodoo 2 only */
#define FBIINIT5_GENERIC_STRAPPING(val)     (((val) >> 5) & 0xf)    /* voodoo 2 only */
#define FBIINIT5_BUFFER_ALLOCATION(val)     (((val) >> 9) & 3)      /* voodoo 2 only */
#define FBIINIT5_DRIVE_VID_CLK_SLAVE(val)   (((val) >> 11) & 1)     /* voodoo 2 only */
#define FBIINIT5_DRIVE_DAC_DATA_16(val)     (((val) >> 12) & 1)     /* voodoo 2 only */
#define FBIINIT5_VCLK_INPUT_SELECT(val)     (((val) >> 13) & 1)     /* voodoo 2 only */
#define FBIINIT5_MULTI_CVG_DETECT(val)      (((val) >> 14) & 1)     /* voodoo 2 only */
#define FBIINIT5_SYNC_RETRACE_READS(val)    (((val) >> 15) & 1)     /* voodoo 2 only */
#define FBIINIT5_ENABLE_RHBORDER_COLOR(val) (((val) >> 16) & 1)     /* voodoo 2 only */
#define FBIINIT5_ENABLE_LHBORDER_COLOR(val) (((val) >> 17) & 1)     /* voodoo 2 only */
#define FBIINIT5_ENABLE_BVBORDER_COLOR(val) (((val) >> 18) & 1)     /* voodoo 2 only */
#define FBIINIT5_ENABLE_TVBORDER_COLOR(val) (((val) >> 19) & 1)     /* voodoo 2 only */
#define FBIINIT5_DOUBLE_HORIZ(val)          (((val) >> 20) & 1)     /* voodoo 2 only */
#define FBIINIT5_DOUBLE_VERT(val)           (((val) >> 21) & 1)     /* voodoo 2 only */
#define FBIINIT5_ENABLE_16BIT_GAMMA(val)    (((val) >> 22) & 1)     /* voodoo 2 only */
#define FBIINIT5_INVERT_DAC_HSYNC(val)      (((val) >> 23) & 1)     /* voodoo 2 only */
#define FBIINIT5_INVERT_DAC_VSYNC(val)      (((val) >> 24) & 1)     /* voodoo 2 only */
#define FBIINIT5_ENABLE_24BIT_DACDATA(val)  (((val) >> 25) & 1)     /* voodoo 2 only */
#define FBIINIT5_ENABLE_INTERLACING(val)    (((val) >> 26) & 1)     /* voodoo 2 only */
#define FBIINIT5_DAC_DATA_18_CONTROL(val)   (((val) >> 27) & 1)     /* voodoo 2 only */
#define FBIINIT5_RASTERIZER_UNIT_MODE(val)  (((val) >> 30) & 3)     /* voodoo 2 only */

#define FBIINIT6_WINDOW_ACTIVE_COUNTER(val) (((val) >> 0) & 7)      /* voodoo 2 only */
#define FBIINIT6_WINDOW_DRAG_COUNTER(val)   (((val) >> 3) & 0x1f)   /* voodoo 2 only */
#define FBIINIT6_SLI_SYNC_MASTER(val)       (((val) >> 8) & 1)      /* voodoo 2 only */
#define FBIINIT6_DAC_DATA_22_OUTPUT(val)    (((val) >> 9) & 3)      /* voodoo 2 only */
#define FBIINIT6_DAC_DATA_23_OUTPUT(val)    (((val) >> 11) & 3)     /* voodoo 2 only */
#define FBIINIT6_SLI_SYNCIN_OUTPUT(val)     (((val) >> 13) & 3)     /* voodoo 2 only */
#define FBIINIT6_SLI_SYNCOUT_OUTPUT(val)    (((val) >> 15) & 3)     /* voodoo 2 only */
#define FBIINIT6_DAC_RD_OUTPUT(val)         (((val) >> 17) & 3)     /* voodoo 2 only */
#define FBIINIT6_DAC_WR_OUTPUT(val)         (((val) >> 19) & 3)     /* voodoo 2 only */
#define FBIINIT6_PCI_FIFO_LWM_RDY(val)      (((val) >> 21) & 0x7f)  /* voodoo 2 only */
#define FBIINIT6_VGA_PASS_N_OUTPUT(val)     (((val) >> 28) & 3)     /* voodoo 2 only */
#define FBIINIT6_X_VIDEO_TILES_BIT0(val)    (((val) >> 30) & 1)     /* voodoo 2 only */

#define FBIINIT7_GENERIC_STRAPPING(val)     (((val) >> 0) & 0xff)   /* voodoo 2 only */
#define FBIINIT7_CMDFIFO_ENABLE(val)        (((val) >> 8) & 1)      /* voodoo 2 only */
#define FBIINIT7_CMDFIFO_MEMORY_STORE(val)  (((val) >> 9) & 1)      /* voodoo 2 only */
#define FBIINIT7_DISABLE_CMDFIFO_HOLES(val) (((val) >> 10) & 1)     /* voodoo 2 only */
#define FBIINIT7_CMDFIFO_READ_THRESH(val)   (((val) >> 11) & 0x1f)  /* voodoo 2 only */
#define FBIINIT7_SYNC_CMDFIFO_WRITES(val)   (((val) >> 16) & 1)     /* voodoo 2 only */
#define FBIINIT7_SYNC_CMDFIFO_READS(val)    (((val) >> 17) & 1)     /* voodoo 2 only */
#define FBIINIT7_RESET_PCI_PACKER(val)      (((val) >> 18) & 1)     /* voodoo 2 only */
#define FBIINIT7_ENABLE_CHROMA_STUFF(val)   (((val) >> 19) & 1)     /* voodoo 2 only */
#define FBIINIT7_CMDFIFO_PCI_TIMEOUT(val)   (((val) >> 20) & 0x7f)  /* voodoo 2 only */
#define FBIINIT7_ENABLE_TEXTURE_BURST(val)  (((val) >> 27) & 1)     /* voodoo 2 only */

#define TEXLOD_LODMIN(val)                  (((val) >> 0) & 0x3f)
#define TEXLOD_LODMAX(val)                  (((val) >> 6) & 0x3f)
#define TEXLOD_LODBIAS(val)                 (((val) >> 12) & 0x3f)
#define TEXLOD_LOD_ODD(val)                 (((val) >> 18) & 1)
#define TEXLOD_LOD_TSPLIT(val)              (((val) >> 19) & 1)
#define TEXLOD_LOD_S_IS_WIDER(val)          (((val) >> 20) & 1)
#define TEXLOD_LOD_ASPECT(val)              (((val) >> 21) & 3)
#define TEXLOD_LOD_ZEROFRAC(val)            (((val) >> 23) & 1)
#define TEXLOD_TMULTIBASEADDR(val)          (((val) >> 24) & 1)
#define TEXLOD_TDATA_SWIZZLE(val)           (((val) >> 25) & 1)
#define TEXLOD_TDATA_SWAP(val)              (((val) >> 26) & 1)
#define TEXLOD_TDIRECT_WRITE(val)           (((val) >> 27) & 1)     /* Voodoo 2 only */

#define TEXDETAIL_DETAIL_MAX(val)           (((val) >> 0) & 0xff)
#define TEXDETAIL_DETAIL_BIAS(val)          (((val) >> 8) & 0x3f)
#define TEXDETAIL_DETAIL_SCALE(val)         (((val) >> 14) & 7)
#define TEXDETAIL_RGB_MIN_FILTER(val)       (((val) >> 17) & 1)     /* Voodoo 2 only */
#define TEXDETAIL_RGB_MAG_FILTER(val)       (((val) >> 18) & 1)     /* Voodoo 2 only */
#define TEXDETAIL_ALPHA_MIN_FILTER(val)     (((val) >> 19) & 1)     /* Voodoo 2 only */
#define TEXDETAIL_ALPHA_MAG_FILTER(val)     (((val) >> 20) & 1)     /* Voodoo 2 only */
#define TEXDETAIL_SEPARATE_RGBA_FILTER(val) (((val) >> 21) & 1)     /* Voodoo 2 only */

#define TREXINIT_SEND_TMU_CONFIG(val)       (((val) >> 18) & 1)









/***************************************************************************
    CONSTANTS
***************************************************************************/
/* enumeration specifying which model of Voodoo we are emulating */
enum
{
	TYPE_VOODOO_1,
	TYPE_VOODOO_2,
	TYPE_VOODOO_BANSHEE,
	TYPE_VOODOO_3
};

#define STD_VOODOO_1_CLOCK          50000000
#define STD_VOODOO_2_CLOCK          90000000
#define STD_VOODOO_BANSHEE_CLOCK    90000000
#define STD_VOODOO_3_CLOCK          132000000


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- device interface ----- */

class voodoo_device : public device_t
{
public:
	~voodoo_device();

	void set_fbmem(int value) { m_fbmem = value; }
	void set_tmumem(int value1, int value2) { m_tmumem0 = value1; m_tmumem1 = value2; }
	template <typename T> void set_screen_tag(T &&tag) { m_screen_finder.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_cpu_tag(T &&tag) { m_cpu_finder.set_tag(std::forward<T>(tag)); }
	auto vblank_callback() { return m_vblank.bind(); }
	auto stall_callback() { return m_stall.bind(); }
	auto pciint_callback() { return m_pciint.bind(); }

	void set_screen(screen_device &screen) { assert(!m_screen); m_screen = &screen; }
	void set_cpu(cpu_device &cpu) { assert(!m_cpu); m_cpu = &cpu; }

	u32 voodoo_r(offs_t offset);
	void voodoo_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	TIMER_CALLBACK_MEMBER( vblank_off_callback );
	TIMER_CALLBACK_MEMBER( stall_cpu_callback );
	TIMER_CALLBACK_MEMBER( vblank_callback );

	void voodoo_postload();

	int voodoo_update(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	int voodoo_get_type();
	int voodoo_is_stalled();
	void voodoo_set_init_enable(u32 newval);

protected:
	voodoo_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 vdt);

	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

	struct tmu_shared_state;

	struct voodoo_stats
	{
		voodoo_stats()
		{
			std::fill(std::begin(texture_mode), std::end(texture_mode), 0);
			buffer[0] = 0;
		}

		u8             lastkey = 0;            // last key state
		u8             display = 0;            // display stats?
		s32             swaps = 0;              // total swaps
		s32             stalls = 0;             // total stalls
		s32             total_triangles = 0;    // total triangles
		s32             total_pixels_in = 0;    // total pixels in
		s32             total_pixels_out = 0;   // total pixels out
		s32             total_chroma_fail = 0;  // total chroma fail
		s32             total_zfunc_fail = 0;   // total z func fail
		s32             total_afunc_fail = 0;   // total a func fail
		s32             total_clipped = 0;      // total clipped
		s32             total_stippled = 0;     // total stippled
		s32             lfb_writes = 0;         // LFB writes
		s32             lfb_reads = 0;          // LFB reads
		s32             reg_writes = 0;         // register writes
		s32             reg_reads = 0;          // register reads
		s32             tex_writes = 0;         // texture writes
		s32             texture_mode[16];       // 16 different texture modes
		u8             render_override = 0;    // render override
		char                buffer[1024];           // string
	};


	/* note that this structure is an even 64 bytes long */
	struct thread_stats_block
	{
		s32             pixels_in = 0;          // pixels in statistic
		s32             pixels_out = 0;         // pixels out statistic
		s32             chroma_fail = 0;        // chroma test fail statistic
		s32             zfunc_fail = 0;         // z function test fail statistic
		s32             afunc_fail = 0;         // alpha function test fail statistic
		s32             clip_fail = 0;          // clipping fail statistic
		s32             stipple_count = 0;      // stipple statistic
		s32             filler[64/4 - 7];       // pad this structure to 64 bytes
	};


	struct fifo_state
	{
		void reset() { in = out = 0; }
		void add(u32 data);
		u32 remove();
		u32 peek() { return base[out]; }
		bool empty() const { return in == out; }
		bool full() const { return ((in + 1) == out) || ((in == (size - 1)) && (out == 0)); }
		s32 items() const;
		s32 space() const { return size - 1 - items(); }

		u32 *          base = nullptr;         // base of the FIFO
		s32             size = 0;               // size of the FIFO
		s32             in = 0;                 // input pointer
		s32             out = 0;                // output pointer
	};


	struct cmdfifo_info
	{
		u8             enable = 0;             // enabled?
		u8             count_holes = 0;        // count holes?
		u32            base = 0;               // base address in framebuffer RAM
		u32            end = 0;                // end address in framebuffer RAM
		u32            rdptr = 0;              // current read pointer
		u32            amin = 0;               // minimum address
		u32            amax = 0;               // maximum address
		u32            depth = 0;              // current depth
		u32            holes = 0;              // number of holes
	};


	struct pci_state
	{
		fifo_state          fifo;                   // PCI FIFO
		u32            init_enable = 0;        // initEnable value
		u8             stall_state = 0;        // state of the system if we're stalled
		u8             op_pending = 0;         // true if an operation is pending
		attotime            op_end_time = attotime::zero; // time when the pending operation ends
		emu_timer *         continue_timer = nullptr; // timer to use to continue processing
		u32            fifo_mem[64*2];         // memory backing the PCI FIFO
	};


	struct tmu_state
	{
		class stw_t;
		void recompute_texture_params();
		void init(u8 vdt, tmu_shared_state &share, voodoo_reg *r, void *memory, int tmem);
		s32 prepare();
		static s32 new_log2(double &value, const int &offset);
		rgbaint_t genTexture(s32 x, const u8 *dither4, voodoo::texture_mode const TEXMODE, rgb_t *LOOKUP, s32 LODBASE, const stw_t &iterstw, s32 &lod);
		rgbaint_t combineTexture(voodoo::texture_mode const TEXMODE, const rgbaint_t& c_local, const rgbaint_t& c_other, s32 lod);

		struct ncc_table
		{
			void write(offs_t regnum, u32 data);
			void update();

			u8             dirty = 0;              // is the texel lookup dirty?
			voodoo_reg *        m_reg = nullptr;          // pointer to our registers
			s32             ir[4], ig[4], ib[4];    // I values for R,G,B
			s32             qr[4], qg[4], qb[4];    // Q values for R,G,B
			s32             y[16];                  // Y values
			rgb_t *             palette = nullptr;      // pointer to associated RGB palette
			rgb_t *             palettea = nullptr;     // pointer to associated ARGB palette
			rgb_t               texel[256];             // texel lookup
		};

		u8 *           ram = nullptr;          // pointer to our RAM
		u32            mask = 0;               // mask to apply to pointers
		voodoo_reg *        m_reg = nullptr;          // pointer to our register base
		u32            regdirty = 0;           // true if the LOD/mode/base registers have changed

		u32            texaddr_mask = 0;       // mask for texture address
		u8             texaddr_shift = 0;      // shift for texture address

		s64             starts = 0, startt = 0; // starting S,T (14.18)
		s64             startw = 0;             // starting W (2.30)
		s64             dsdx = 0, dtdx = 0;     // delta S,T per X
		s64             dwdx = 0;               // delta W per X
		s64             dsdy = 0, dtdy = 0;     // delta S,T per Y
		s64             dwdy = 0;               // delta W per Y

		s32             lodmin = 0, lodmax = 0; // min, max LOD values
		s32             lodbias = 0;            // LOD bias
		u32            lodmask = 0;            // mask of available LODs
		u32            lodoffset[9];           // offset of texture base for each LOD
		s32             detailmax = 0;          // detail clamp
		s32             detailbias = 0;         // detail bias
		u8             detailscale = 0;        // detail scale

		u32            wmask = 0;              // mask for the current texture width
		u32            hmask = 0;              // mask for the current texture height

		u32            bilinear_mask = 0;      // mask for bilinear resolution (0xf0 for V1, 0xff for V2)

		ncc_table           ncc[2];                 // two NCC tables

		rgb_t *             lookup = nullptr;       // currently selected lookup
		rgb_t *             texel[16];              // texel lookups for each format

		rgb_t               palette[256];           // palette lookup table
		rgb_t               palettea[256];          // palette+alpha lookup table
	};


	struct tmu_shared_state
	{
		void init();

		rgb_t               rgb332[256];            // RGB 3-3-2 lookup table
		rgb_t               alpha8[256];            // alpha 8-bit lookup table
		rgb_t               int8[256];              // intensity 8-bit lookup table
		rgb_t               ai44[256];              // alpha, intensity 4-4 lookup table

		rgb_t*              rgb565;                 // RGB 5-6-5 lookup table
		rgb_t               argb1555[65536];        // ARGB 1-5-5-5 lookup table
		rgb_t               argb4444[65536];        // ARGB 4-4-4-4 lookup table
	};


	struct fbi_state
	{
		struct setup_vertex
		{
			float               x, y;                   // X, Y coordinates
			float               z, wb;                  // Z and broadcast W values
			float               r, g, b, a;             // A, R, G, B values
			float               s0, t0, w0;             // W, S, T for TMU 0
			float               s1, t1, w1;             // W, S, T for TMU 1
		};

		u8 *           ram = nullptr;          // pointer to frame buffer RAM
		u32            mask = 0;               // mask to apply to pointers
		u32            rgboffs[3] = { 0, 0, 0 }; // word offset to 3 RGB buffers
		u32            auxoffs = 0;            // word offset to 1 aux buffer

		u8             frontbuf = 0;           // front buffer index
		u8             backbuf = 0;            // back buffer index
		u8             swaps_pending = 0;      // number of pending swaps
		u8             video_changed = 0;      // did the frontbuffer video change?

		u32            yorigin = 0;            // Y origin subtract value
		u32            lfb_base = 0;           // base of LFB in memory
		u8             lfb_stride = 0;         // stride of LFB accesses in bits

		u32            width = 0;              // width of current frame buffer
		u32            height = 0;             // height of current frame buffer
		u32            xoffs = 0;              // horizontal offset (back porch)
		u32            yoffs = 0;              // vertical offset (back porch)
		u32            vsyncstart = 0;         // vertical sync start scanline
		u32            vsyncstop = 0;          // vertical sync stop
		u32            rowpixels = 0;          // pixels per row
		u32            tile_width = 0;         // width of video tiles
		u32            tile_height = 0;        // height of video tiles
		u32            x_tiles = 0;            // number of tiles in the X direction

		emu_timer *         vsync_stop_timer = nullptr; // VBLANK End timer
		emu_timer *         vsync_start_timer = nullptr; // VBLANK timer
		u8             vblank = 0;             // VBLANK state
		u8             vblank_count = 0;       // number of VBLANKs since last swap
		u8             vblank_swap_pending = 0;// a swap is pending, waiting for a vblank
		u8             vblank_swap = 0;        // swap when we hit this count
		u8             vblank_dont_swap = 0;   // don't actually swap when we hit this point

		/* triangle setup info */
		u8             cheating_allowed = 0;   // allow cheating?
		s32             sign;                   // triangle sign
		s16             ax, ay;                 // vertex A x,y (12.4)
		s16             bx, by;                 // vertex B x,y (12.4)
		s16             cx, cy;                 // vertex C x,y (12.4)
		s32             startr, startg, startb, starta; // starting R,G,B,A (12.12)
		s32             startz;                 // starting Z (20.12)
		s64             startw;                 // starting W (16.32)
		s32             drdx, dgdx, dbdx, dadx; // delta R,G,B,A per X
		s32             dzdx;                   // delta Z per X
		s64             dwdx;                   // delta W per X
		s32             drdy, dgdy, dbdy, dady; // delta R,G,B,A per Y
		s32             dzdy;                   // delta Z per Y
		s64             dwdy;                   // delta W per Y

		thread_stats_block  lfb_stats;              // LFB-access statistics

		u8             sverts = 0;             // number of vertices ready */
		setup_vertex        svert[3];               // 3 setup vertices */

		fifo_state          fifo;                   // framebuffer memory fifo */
		cmdfifo_info        cmdfifo[2];             // command FIFOs */

		u8             fogblend[64];           // 64-entry fog table */
		u8             fogdelta[64];           // 64-entry fog table */
		u8             fogdelta_mask;          // mask for for delta (0xff for V1, 0xfc for V2) */

		rgb_t               pen[65536];             // mapping from pixels to pens */
		rgb_t               clut[512];              // clut gamma data */
		u8             clut_dirty = 1;         // do we need to recompute? */
		rgb_t               rgb565[65536];          // RGB 5-6-5 lookup table */
	};


	struct dac_state
	{
		void data_w(u8 regum, u8 data);
		void data_r(u8 regnum);

		u8             m_reg[8];                 // 8 registers
		u8             read_result;            // pending read result
	};


	struct raster_info;
	struct poly_extra_data
	{
		voodoo_device * device;
		raster_info *       info;                   // pointer to rasterizer information
		u16 *destbase;

		s16             ax, ay;                 // vertex A x,y (12.4)
		s32             startr, startg, startb, starta; // starting R,G,B,A (12.12)
		s32             startz;                 // starting Z (20.12)
		s64             startw;                 // starting W (16.32)
		s32             drdx, dgdx, dbdx, dadx; // delta R,G,B,A per X
		s32             dzdx;                   // delta Z per X
		s64             dwdx;                   // delta W per X
		s32             drdy, dgdy, dbdy, dady; // delta R,G,B,A per Y
		s32             dzdy;                   // delta Z per Y
		s64             dwdy;                   // delta W per Y

		s64             starts0, startt0;       // starting S,T (14.18)
		s64             startw0;                // starting W (2.30)
		s64             ds0dx, dt0dx;           // delta S,T per X
		s64             dw0dx;                  // delta W per X
		s64             ds0dy, dt0dy;           // delta S,T per Y
		s64             dw0dy;                  // delta W per Y
		s32             lodbase0;               // used during rasterization

		s64             starts1, startt1;       // starting S,T (14.18)
		s64             startw1;                // starting W (2.30)
		s64             ds1dx, dt1dx;           // delta S,T per X
		s64             dw1dx;                  // delta W per X
		s64             ds1dy, dt1dy;           // delta S,T per Y
		s64             dw1dy;                  // delta W per Y
		s32             lodbase1;               // used during rasterization

		u16            dither[16];             // dither matrix, for fastfill
	};


	class voodoo_renderer : public poly_manager<float, poly_extra_data, 1, 10000>
	{
	public:
		voodoo_renderer(running_machine &machine) :
			poly_manager(machine) { }

		using mfp = void (voodoo_device::*)(s32, const extent_t &, const poly_extra_data &, int);
	};


	struct static_raster_info
	{
		constexpr u32 compute_hash() const;

		voodoo_renderer::mfp callback_mfp;
		u32            eff_color_path;         // effective fbzColorPath value
		u32            eff_alpha_mode;         // effective alphaMode value
		u32            eff_fog_mode;           // effective fogMode value
		u32            eff_fbz_mode;           // effective fbzMode value
		u32            eff_tex_mode_0;         // effective textureMode value for TMU #0
		u32            eff_tex_mode_1;         // effective textureMode value for TMU #1
	};


	struct raster_info
	{
		raster_info *       next = nullptr;         // pointer to next entry with the same hash
		voodoo_renderer::render_delegate callback; // callback pointer
		u8             display;                // display index
		u32            hits;                   // how many hits (pixels) we've used this for
		u32            polys;                  // how many polys we've used this for
		u32            hash = 0U;
		static_raster_info const *static_info;
	};


	struct banshee_info
	{
		u32            io[0x40];               // I/O registers
		u32            agp[0x80];              // AGP registers
		u8             vga[0x20];              // VGA registers
		u8             crtc[0x27];             // VGA CRTC registers
		u8             seq[0x05];              // VGA sequencer registers
		u8             gc[0x05];               // VGA graphics controller registers
		u8             att[0x15];              // VGA attribute registers
		u8             attff;                  // VGA attribute flip-flop

		u32            blt_regs[0x20];         // 2D Blitter registers
		u32            blt_dst_base = 0;
		u32            blt_dst_x = 0;
		u32            blt_dst_y = 0;
		u32            blt_dst_width = 0;
		u32            blt_dst_height = 0;
		u32            blt_dst_stride = 0;
		u32            blt_dst_bpp = 0;
		u32            blt_cmd = 0;
		u32            blt_src_base = 0;
		u32            blt_src_x = 0;
		u32            blt_src_y = 0;
		u32            blt_src_width = 0;
		u32            blt_src_height = 0;
		u32            blt_src_stride = 0;
		u32            blt_src_bpp = 0;
	};


	static static_raster_info predef_raster_table[];
	static static_raster_info generic_raster_table[3];

	// not all of these need to be static, review.

	void check_stalled_cpu(attotime current_time);
	void flush_fifos(attotime current_time);
	void init_fbi(fbi_state *f, void *memory, int fbmem);
	s32 register_w(offs_t offset, u32 data);
	s32 swapbuffer(u32 data);
	s32 lfb_w(offs_t offset, u32 data, u32 mem_mask);
	u32 lfb_r(offs_t offset, bool lfb_3d);
	s32 texture_w(offs_t offset, u32 data);
	s32 lfb_direct_w(offs_t offset, u32 data, u32 mem_mask);
	s32 banshee_2d_w(offs_t offset, u32 data);
	void stall_cpu(int state, attotime current_time);
	void soft_reset();
	void recompute_video_memory();
	void adjust_vblank_timer();
	s32 fastfill();
	s32 triangle();
	s32 begin_triangle();
	s32 draw_triangle();
	s32 setup_and_draw_triangle();
	s32 triangle_create_work_item(u16 *drawbuf, int texcount);
	raster_info *add_rasterizer(static_raster_info const &cinfo, bool is_generic);
	raster_info *find_rasterizer(int texcount);
	void dump_rasterizer_stats();

	void accumulate_statistics(const thread_stats_block &block);
	void update_statistics(bool accumulate);
	void reset_counters();

	u32 register_r(offs_t offset);

	void swap_buffers();
	int cmdfifo_compute_expected_depth(cmdfifo_info &f);
	u32 cmdfifo_execute(cmdfifo_info *f);
	s32 cmdfifo_execute_if_ready(cmdfifo_info &f);
	void cmdfifo_w(cmdfifo_info *f, offs_t offset, u32 data);

	void init_save_state();

	void raster_fastfill(s32 scanline, const voodoo_renderer::extent_t &extent, const poly_extra_data &extradata, int threadid);
	void raster_generic_0tmu(s32 scanline, const voodoo_renderer::extent_t &extent, const poly_extra_data &extradata, int threadid);
	void raster_generic_1tmu(s32 scanline, const voodoo_renderer::extent_t &extent, const poly_extra_data &extradata, int threadid);
	void raster_generic_2tmu(s32 scanline, const voodoo_renderer::extent_t &extent, const poly_extra_data &extradata, int threadid);

#define RASTERIZER_HEADER(name) \
	void raster_##name(s32 y, const voodoo_renderer::extent_t &extent, const poly_extra_data &extradata, int threadid);
#define RASTERIZER_ENTRY(fbzcp, alpha, fog, fbz, tex0, tex1) \
	RASTERIZER_HEADER(fbzcp##_##alpha##_##fog##_##fbz##_##tex0##_##tex1)
#include "voodoo_rast.ipp"

#undef RASTERIZER_ENTRY

	bool chroma_key_test(thread_stats_block &stats, voodoo::fbz_mode const fbzmode, rgbaint_t rgaIntColor);
	bool alpha_mask_test(thread_stats_block &stats, voodoo::fbz_mode const fbzmode, u8 alpha);
	bool alpha_test(thread_stats_block &stats, voodoo::alpha_mode const alphamode, u8 alpha);
	bool depth_test(u16 zaColorReg, thread_stats_block &stats, s32 destDepth, voodoo::fbz_mode const fbzmode, s32 biasdepth);
	bool combine_color(rgbaint_t &color, thread_stats_block &threadstats, voodoo::fbz_colorpath const fbzcp, voodoo::fbz_mode const fbzmode, rgbaint_t texel, s32 iterz, s64 iterw);
	void apply_fogging(rgbaint_t &color, voodoo::fbz_mode const fbzmode, voodoo::fog_mode const fogmode, voodoo::fbz_colorpath const fbzcp, s32 x, u8 const *dither4, s32 wfloat, s32 iterz, s64 iterw, const rgbaint_t &iterargb);

	void banshee_blit_2d(u32 data);

// FIXME: this stuff should not be public
public:
	voodoo_reg          m_reg[0x400];             // raw registers
	const u8       m_type;                // type of system
	u8             m_alt_regmap;             // enable alternate register map?
	u8             m_chipmask;               // mask for which chips are available
	u8             m_index;                  // index of board


	screen_device *     m_screen;               // the screen we are acting on
	cpu_device *        m_cpu;                  // the CPU we interact with
	u32            m_freq;                   // operating frequency
	attoseconds_t       m_attoseconds_per_cycle;  // attoseconds per cycle
	int                 m_trigger;                // trigger used for stalling

	const u8 *     m_regaccess;              // register access array
	const char *const * m_regnames;               // register names array

	std::unique_ptr<voodoo_renderer> m_poly;              // polygon manager
	std::unique_ptr<thread_stats_block[]> m_thread_stats; // per-thread statistics

	voodoo_stats        m_stats;                  // internal statistics

	offs_t              m_last_status_pc;         // PC of last status description (for logging)
	u32            m_last_status_value;      // value of last status read (for logging)

	int                 m_next_rasterizer;        // next rasterizer index
	raster_info         m_rasterizer[MAX_RASTERIZERS]; // array of rasterizers
	raster_info *       m_raster_hash[RASTER_HASH_SIZE]; // hash table of rasterizers
	raster_info *       m_generic_rasterizer[3];

	bool                m_send_config;
	u32            m_tmu_config;

	u8             m_fbmem;
	u8             m_tmumem0;
	u8             m_tmumem1;
	devcb_write_line    m_vblank;
	devcb_write_line    m_stall;
	// This is for internally generated PCI interrupts in Voodoo3
	devcb_write_line    m_pciint;

	pci_state           m_pci;                    // PCI state
	dac_state           m_dac;                    // DAC state
	fbi_state           m_fbi;                    // FBI states
	tmu_state           m_tmu[MAX_TMU];           // TMU states
	tmu_shared_state    m_tmushare;               // TMU shared state
	banshee_info        m_banshee;                // Banshee state

	optional_device<screen_device> m_screen_finder; // the screen we are acting on
	optional_device<cpu_device> m_cpu_finder;   // the CPU we interact with

	std::unique_ptr<u8[]> m_fbmem_alloc;
	std::unique_ptr<u8[]> m_tmumem_alloc[2];
};

class voodoo_1_device : public voodoo_device
{
public:
	voodoo_1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class voodoo_2_device : public voodoo_device
{
public:
	voodoo_2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class voodoo_banshee_device : public voodoo_device
{
public:
	voodoo_banshee_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u32 banshee_r(offs_t offset, u32 mem_mask = ~0);
	void banshee_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 banshee_fb_r(offs_t offset);
	void banshee_fb_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 banshee_io_r(offs_t offset, u32 mem_mask = ~0);
	void banshee_io_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 banshee_rom_r(offs_t offset);
	u8 banshee_vga_r(offs_t offset);
	void banshee_vga_w(offs_t offset, u8 data);

protected:
	voodoo_banshee_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 vdt);

	// device-level overrides
	u32 banshee_agp_r(offs_t offset);
	void banshee_agp_w(offs_t offset, u32 data, u32 mem_mask = ~0);
};


class voodoo_3_device : public voodoo_banshee_device
{
public:
	voodoo_3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(VOODOO_1,       voodoo_1_device)
DECLARE_DEVICE_TYPE(VOODOO_2,       voodoo_2_device)
DECLARE_DEVICE_TYPE(VOODOO_BANSHEE, voodoo_banshee_device)
DECLARE_DEVICE_TYPE(VOODOO_3,       voodoo_3_device)

// use SSE on 64-bit implementations, where it can be assumed
#if 1 && ((!defined(MAME_DEBUG) || defined(__OPTIMIZE__)) && (defined(__SSE2__) || defined(_MSC_VER)) && defined(PTR64))
#include <emmintrin.h>
#ifdef __SSE4_1__
#include <smmintrin.h>
#endif
class voodoo_device::tmu_state::stw_t
{
public:
	stw_t() { }
	stw_t(const stw_t& other) = default;
	stw_t &operator=(const stw_t& other) = default;

	void set(s64 s, s64 t, s64 w) { m_st = _mm_set_pd(s << 8, t << 8); m_w = _mm_set1_pd(w); }
	int is_w_neg() const { return _mm_comilt_sd(m_w, _mm_set1_pd(0.0)); }
	void get_st_shiftr(s32 &s, s32 &t, const s32 &shift) const
	{
		s64 tmpS = _mm_cvtsd_si64(_mm_shuffle_pd(m_st, _mm_setzero_pd(), 1));
		s = tmpS >> shift;
		s64 tmpT = _mm_cvtsd_si64(m_st);
		t = tmpT >> shift;
	}
	void add(const stw_t& other)
	{
		m_st = _mm_add_pd(m_st, other.m_st);
		m_w = _mm_add_pd(m_w, other.m_w);
	}
	void calc_stow(s32 &sow, s32 &tow, s32 &oowlog) const
	{
		__m128d tmp = _mm_div_pd(m_st, m_w);
		// Allow for 8 bits of decimal in integer
		//tmp = _mm_mul_pd(tmp, _mm_set1_pd(256.0));
		__m128i tmp2 = _mm_cvttpd_epi32(tmp);
#ifdef __SSE4_1__
		sow = _mm_extract_epi32(tmp2, 1);
		tow = _mm_extract_epi32(tmp2, 0);
#else
		sow = _mm_cvtsi128_si32(_mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 0, 1)));
		tow = _mm_cvtsi128_si32(tmp2);
#endif
		double dW = _mm_cvtsd_f64(m_w);
		oowlog = -new_log2(dW, 0);
	}
private:
	__m128d m_st;
	__m128d m_w;
};
#else
class voodoo_device::tmu_state::stw_t
{
public:
	stw_t() {}
	stw_t(const stw_t& other) = default;
	stw_t &operator=(const stw_t& other) = default;

	void set(s64 s, s64 t, s64 w) { m_s = s; m_t = t; m_w = w; }
	int is_w_neg() const { return (m_w < 0) ? 1 : 0; }
	void get_st_shiftr(s32 &s, s32 &t, const s32 &shift) const
	{
		s = m_s >> shift;
		t = m_t >> shift;
	}
	inline void add(const stw_t& other)
	{
		m_s += other.m_s;
		m_t += other.m_t;
		m_w += other.m_w;
	}
	// Computes s/w and t/w and returns log2 of 1/w
	// s, t and c are 16.32 values.  The results are 24.8.
	inline void calc_stow(s32 &sow, s32 &tow, s32 &oowlog) const
	{
		double recip = double(1ULL << (47 - 39)) / m_w;
		double resAD = m_s * recip;
		double resBD = m_t * recip;
		oowlog = new_log2(recip, 56);
		sow = resAD;
		tow = resBD;
	}
private:
	s64 m_s, m_t, m_w;
};
#endif

#endif // MAME_VIDEO_VOODOO_H
