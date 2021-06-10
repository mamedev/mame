// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo_regs.h

    3dfx Voodoo Graphics SST-1/2 emulator.

***************************************************************************/

#ifndef MAME_VIDEO_VOODOO_REGS_H
#define MAME_VIDEO_VOODOO_REGS_H

#pragma once



//**************************************************************************
//  CONSTANTS
//**************************************************************************

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

class dither_helper;

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

class init_en
{
public:
	constexpr init_en(u32 value) :
		m_value(value) { }

	constexpr u32 enable_hw_init() const           { return BIT(m_value, 0, 1); }
	constexpr u32 enable_pci_fifo() const          { return BIT(m_value, 1, 1); }
	constexpr u32 remap_init_to_dac() const        { return BIT(m_value, 2, 1); }
	constexpr u32 enable_snoop0() const            { return BIT(m_value, 4, 1); }
	constexpr u32 snoop0_memory_match() const      { return BIT(m_value, 5, 1); }
	constexpr u32 snoop0_readwrite_match() const   { return BIT(m_value, 6, 1); }
	constexpr u32 enable_snoop1() const            { return BIT(m_value, 7, 1); }
	constexpr u32 snoop1_memory_match() const      { return BIT(m_value, 8, 1); }
	constexpr u32 snoop1_readwrite_match() const   { return BIT(m_value, 9, 1); }
	constexpr u32 sli_bus_owner() const            { return BIT(m_value, 10, 1); }
	constexpr u32 sli_odd_even() const             { return BIT(m_value, 11, 1); }
	constexpr u32 secondary_rev_id() const         { return BIT(m_value, 12, 4); }	// voodoo 2 only
	constexpr u32 mfctr_fab_id() const             { return BIT(m_value, 16, 4); }	// voodoo 2 only
	constexpr u32 enable_pci_interrupt() const     { return BIT(m_value, 20, 1); }	// voodoo 2 only
	constexpr u32 pci_interrupt_timeout() const    { return BIT(m_value, 21, 1); }	// voodoo 2 only
	constexpr u32 enable_nand_tree_test() const    { return BIT(m_value, 22, 1); }	// voodoo 2 only
	constexpr u32 enable_sli_address_snoop() const { return BIT(m_value, 23, 1); }	// voodoo 2 only
	constexpr u32 sli_snoop_address() const        { return BIT(m_value, 24, 8); }	// voodoo 2 only

private:
	u32 m_value;
};

class lfb_mode
{
public:
	constexpr lfb_mode(u32 value) :
		m_value(value) { }

	constexpr u32 write_format() const          { return BIT(m_value, 0, 4); }
	constexpr u32 write_buffer_select() const   { return BIT(m_value, 4, 2); }
	constexpr u32 read_buffer_select() const    { return BIT(m_value, 6, 2); }
	constexpr u32 enable_pixel_pipeline() const { return BIT(m_value, 8, 1); }
	constexpr u32 rgba_lanes() const            { return BIT(m_value, 9, 2); }
	constexpr u32 word_swap_writes() const      { return BIT(m_value, 11, 1); }
	constexpr u32 byte_swizzle_writes() const   { return BIT(m_value, 12, 1); }
	constexpr u32 y_origin() const              { return BIT(m_value, 13, 1); }
	constexpr u32 write_w_select() const        { return BIT(m_value, 14, 1); }
	constexpr u32 word_swap_reads() const       { return BIT(m_value, 15, 1); }
	constexpr u32 byte_swizzle_reads() const    { return BIT(m_value, 16, 1); }

private:
	u32 m_value;
};

class chroma_range
{
public:
	constexpr chroma_range(u32 value) :
		m_value(value) { }

	constexpr u32 blue() const            { return BIT(m_value, 0, 8); }
	constexpr u32 green() const           { return BIT(m_value, 8, 8); }
	constexpr u32 red() const             { return BIT(m_value, 16, 8); }
	constexpr u32 blue_exclusive() const  { return BIT(m_value, 24, 1); }
	constexpr u32 green_exclusive() const { return BIT(m_value, 25, 1); }
	constexpr u32 red_exclusive() const   { return BIT(m_value, 26, 1); }
	constexpr u32 union_mode() const      { return BIT(m_value, 27, 1); }
	constexpr u32 enable() const          { return BIT(m_value, 28, 1); }

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
 *  Macros for extracting bitfields
 *
 *************************************/

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


#endif // MAME_VIDEO_VOODOO_REGS_H
