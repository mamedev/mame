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
	s32 i;
	u32 u;
	float f;
	rgba rgb;
};

namespace voodoo
{

// ======================> fbz_colorpath

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


// ======================> fbz_mode

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


// ======================> alpha_mode

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


// ======================> fog_mode

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


// ======================> texture_mode

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


// ======================> texture_lod

class texture_lod
{
public:
	constexpr texture_lod(u32 value) :
		m_value(value) { }

	constexpr u32 lod_min() const        { return BIT(m_value, 0, 6); }
	constexpr u32 lod_max() const        { return BIT(m_value, 6, 6); }
	constexpr u32 lod_bias() const       { return BIT(m_value, 12, 6); }
	constexpr u32 lod_odd() const        { return BIT(m_value, 18, 1); }
	constexpr u32 lod_tsplit() const     { return BIT(m_value, 19, 1); }
	constexpr u32 lod_s_is_wider() const { return BIT(m_value, 20, 1); }
	constexpr u32 lod_aspect() const     { return BIT(m_value, 21, 2); }
	constexpr u32 lod_zerofrac() const   { return BIT(m_value, 23, 1); }
	constexpr u32 tmultibaseaddr() const { return BIT(m_value, 24, 1); }
	constexpr u32 tdata_swizzle() const  { return BIT(m_value, 25, 1); }
	constexpr u32 tdata_swap() const     { return BIT(m_value, 26, 1); }
	constexpr u32 tdirect_write() const  { return BIT(m_value, 27, 1); }  // Voodoo 2 only
	constexpr u32 magic() const          { return BIT(m_value, 28, 4); }

private:
	u32 m_value;
};


// ======================> texture_detail

class texture_detail
{
public:
	constexpr texture_detail(u32 value) :
		m_value(value) { }

	constexpr u32 detail_max() const           { return BIT(m_value, 0, 8); }
	constexpr u32 detail_bias() const          { return BIT(m_value, 8, 6); }
	constexpr u32 detail_scale() const         { return BIT(m_value, 14, 3); }
	constexpr u32 rgb_min_filter() const       { return BIT(m_value, 17, 1); }
	constexpr u32 rgb_mag_filter() const       { return BIT(m_value, 18, 1); }
	constexpr u32 alpha_min_filter() const     { return BIT(m_value, 19, 1); }
	constexpr u32 alpha_mag_filter() const     { return BIT(m_value, 20, 1); }
	constexpr u32 separate_rgba_filter() const { return BIT(m_value, 21, 1); }

private:
	u32 m_value;
};


// ======================> init_en

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


// ======================> lfb_mode

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


// ======================> chroma_range

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


// ======================> voodoo_regs

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
static constexpr u32 vdstatus =        0x000/4;   // R  P
static constexpr u32 intrCtrl =        0x004/4;   // RW P   -- Voodoo2/Banshee only
static constexpr u32 vertexAx =        0x008/4;   //  W PF
static constexpr u32 vertexAy =        0x00c/4;   //  W PF
static constexpr u32 vertexBx =        0x010/4;   //  W PF
static constexpr u32 vertexBy =        0x014/4;   //  W PF
static constexpr u32 vertexCx =        0x018/4;   //  W PF
static constexpr u32 vertexCy =        0x01c/4;   //  W PF
static constexpr u32 startR =          0x020/4;   //  W PF
static constexpr u32 startG =          0x024/4;   //  W PF
static constexpr u32 startB =          0x028/4;   //  W PF
static constexpr u32 startZ =          0x02c/4;   //  W PF
static constexpr u32 startA =          0x030/4;   //  W PF
static constexpr u32 startS =          0x034/4;   //  W PF
static constexpr u32 startT =          0x038/4;   //  W PF
static constexpr u32 startW =          0x03c/4;   //  W PF

/* 0x040 */
static constexpr u32 dRdX =            0x040/4;   //  W PF
static constexpr u32 dGdX =            0x044/4;   //  W PF
static constexpr u32 dBdX =            0x048/4;   //  W PF
static constexpr u32 dZdX =            0x04c/4;   //  W PF
static constexpr u32 dAdX =            0x050/4;   //  W PF
static constexpr u32 dSdX =            0x054/4;   //  W PF
static constexpr u32 dTdX =            0x058/4;   //  W PF
static constexpr u32 dWdX =            0x05c/4;   //  W PF
static constexpr u32 dRdY =            0x060/4;   //  W PF
static constexpr u32 dGdY =            0x064/4;   //  W PF
static constexpr u32 dBdY =            0x068/4;   //  W PF
static constexpr u32 dZdY =            0x06c/4;   //  W PF
static constexpr u32 dAdY =            0x070/4;   //  W PF
static constexpr u32 dSdY =            0x074/4;   //  W PF
static constexpr u32 dTdY =            0x078/4;   //  W PF
static constexpr u32 dWdY =            0x07c/4;   //  W PF

/* 0x080 */
static constexpr u32 triangleCMD =     0x080/4;   //  W PF
static constexpr u32 fvertexAx =       0x088/4;   //  W PF
static constexpr u32 fvertexAy =       0x08c/4;   //  W PF
static constexpr u32 fvertexBx =       0x090/4;   //  W PF
static constexpr u32 fvertexBy =       0x094/4;   //  W PF
static constexpr u32 fvertexCx =       0x098/4;   //  W PF
static constexpr u32 fvertexCy =       0x09c/4;   //  W PF
static constexpr u32 fstartR =         0x0a0/4;   //  W PF
static constexpr u32 fstartG =         0x0a4/4;   //  W PF
static constexpr u32 fstartB =         0x0a8/4;   //  W PF
static constexpr u32 fstartZ =         0x0ac/4;   //  W PF
static constexpr u32 fstartA =         0x0b0/4;   //  W PF
static constexpr u32 fstartS =         0x0b4/4;   //  W PF
static constexpr u32 fstartT =         0x0b8/4;   //  W PF
static constexpr u32 fstartW =         0x0bc/4;   //  W PF

/* 0x0c0 */
static constexpr u32 fdRdX =           0x0c0/4;   //  W PF
static constexpr u32 fdGdX =           0x0c4/4;   //  W PF
static constexpr u32 fdBdX =           0x0c8/4;   //  W PF
static constexpr u32 fdZdX =           0x0cc/4;   //  W PF
static constexpr u32 fdAdX =           0x0d0/4;   //  W PF
static constexpr u32 fdSdX =           0x0d4/4;   //  W PF
static constexpr u32 fdTdX =           0x0d8/4;   //  W PF
static constexpr u32 fdWdX =           0x0dc/4;   //  W PF
static constexpr u32 fdRdY =           0x0e0/4;   //  W PF
static constexpr u32 fdGdY =           0x0e4/4;   //  W PF
static constexpr u32 fdBdY =           0x0e8/4;   //  W PF
static constexpr u32 fdZdY =           0x0ec/4;   //  W PF
static constexpr u32 fdAdY =           0x0f0/4;   //  W PF
static constexpr u32 fdSdY =           0x0f4/4;   //  W PF
static constexpr u32 fdTdY =           0x0f8/4;   //  W PF
static constexpr u32 fdWdY =           0x0fc/4;   //  W PF

/* 0x100 */
static constexpr u32 ftriangleCMD =    0x100/4;   //  W PF
static constexpr u32 fbzColorPath =    0x104/4;   // RW PF
static constexpr u32 fogMode =         0x108/4;   // RW PF
static constexpr u32 alphaMode =       0x10c/4;   // RW PF
static constexpr u32 fbzMode =         0x110/4;   // RW  F
static constexpr u32 lfbMode =         0x114/4;   // RW  F
static constexpr u32 clipLeftRight =   0x118/4;   // RW  F
static constexpr u32 clipLowYHighY =   0x11c/4;   // RW  F
static constexpr u32 nopCMD =          0x120/4;   //  W  F
static constexpr u32 fastfillCMD =     0x124/4;   //  W  F
static constexpr u32 swapbufferCMD =   0x128/4;   //  W  F
static constexpr u32 fogColor =        0x12c/4;   //  W  F
static constexpr u32 zaColor =         0x130/4;   //  W  F
static constexpr u32 chromaKey =       0x134/4;   //  W  F
static constexpr u32 chromaRange =     0x138/4;   //  W  F  -- Voodoo2/Banshee only
static constexpr u32 userIntrCMD =     0x13c/4;   //  W  F  -- Voodoo2/Banshee only

/* 0x140 */
static constexpr u32 stipple =         0x140/4;   // RW  F
static constexpr u32 color0 =          0x144/4;   // RW  F
static constexpr u32 color1 =          0x148/4;   // RW  F
static constexpr u32 fbiPixelsIn =     0x14c/4;   // R
static constexpr u32 fbiChromaFail =   0x150/4;   // R
static constexpr u32 fbiZfuncFail =    0x154/4;   // R
static constexpr u32 fbiAfuncFail =    0x158/4;   // R
static constexpr u32 fbiPixelsOut =    0x15c/4;   // R
static constexpr u32 fogTable =        0x160/4;   //  W  F

/* 0x1c0 */
static constexpr u32 cmdFifoBaseAddr = 0x1e0/4;   // RW     -- Voodoo2 only
static constexpr u32 cmdFifoBump =     0x1e4/4;   // RW     -- Voodoo2 only
static constexpr u32 cmdFifoRdPtr =    0x1e8/4;   // RW     -- Voodoo2 only
static constexpr u32 cmdFifoAMin =     0x1ec/4;   // RW     -- Voodoo2 only
static constexpr u32 colBufferAddr =   0x1ec/4;   // RW     -- Banshee only
static constexpr u32 cmdFifoAMax =     0x1f0/4;   // RW     -- Voodoo2 only
static constexpr u32 colBufferStride = 0x1f0/4;   // RW     -- Banshee only
static constexpr u32 cmdFifoDepth =    0x1f4/4;   // RW     -- Voodoo2 only
static constexpr u32 auxBufferAddr =   0x1f4/4;   // RW     -- Banshee only
static constexpr u32 cmdFifoHoles =    0x1f8/4;   // RW     -- Voodoo2 only
static constexpr u32 auxBufferStride = 0x1f8/4;   // RW     -- Banshee only

/* 0x200 */
static constexpr u32 fbiInit4 =        0x200/4;   // RW     -- Voodoo/Voodoo2 only
static constexpr u32 clipLeftRight1 =  0x200/4;   // RW     -- Banshee only
static constexpr u32 vRetrace =        0x204/4;   // R      -- Voodoo/Voodoo2 only
static constexpr u32 clipTopBottom1 =  0x204/4;   // RW     -- Banshee only
static constexpr u32 backPorch =       0x208/4;   // RW     -- Voodoo/Voodoo2 only
static constexpr u32 videoDimensions = 0x20c/4;   // RW     -- Voodoo/Voodoo2 only
static constexpr u32 fbiInit0 =        0x210/4;   // RW     -- Voodoo/Voodoo2 only
static constexpr u32 fbiInit1 =        0x214/4;   // RW     -- Voodoo/Voodoo2 only
static constexpr u32 fbiInit2 =        0x218/4;   // RW     -- Voodoo/Voodoo2 only
static constexpr u32 fbiInit3 =        0x21c/4;   // RW     -- Voodoo/Voodoo2 only
static constexpr u32 hSync =           0x220/4;   //  W     -- Voodoo/Voodoo2 only
static constexpr u32 vSync =           0x224/4;   //  W     -- Voodoo/Voodoo2 only
static constexpr u32 clutData =        0x228/4;   //  W  F  -- Voodoo/Voodoo2 only
static constexpr u32 dacData =         0x22c/4;   //  W     -- Voodoo/Voodoo2 only
static constexpr u32 maxRgbDelta =     0x230/4;   //  W     -- Voodoo/Voodoo2 only
static constexpr u32 hBorder =         0x234/4;   //  W     -- Voodoo2 only
static constexpr u32 vBorder =         0x238/4;   //  W     -- Voodoo2 only
static constexpr u32 borderColor =     0x23c/4;   //  W     -- Voodoo2 only

/* 0x240 */
static constexpr u32 hvRetrace =       0x240/4;   // R      -- Voodoo2 only
static constexpr u32 fbiInit5 =        0x244/4;   // RW     -- Voodoo2 only
static constexpr u32 fbiInit6 =        0x248/4;   // RW     -- Voodoo2 only
static constexpr u32 fbiInit7 =        0x24c/4;   // RW     -- Voodoo2 only
static constexpr u32 swapPending =     0x24c/4;   //  W     -- Banshee only
static constexpr u32 leftOverlayBuf =  0x250/4;   //  W     -- Banshee only
static constexpr u32 rightOverlayBuf = 0x254/4;   //  W     -- Banshee only
static constexpr u32 fbiSwapHistory =  0x258/4;   // R      -- Voodoo2/Banshee only
static constexpr u32 fbiTrianglesOut = 0x25c/4;   // R      -- Voodoo2/Banshee only
static constexpr u32 sSetupMode =      0x260/4;   //  W PF  -- Voodoo2/Banshee only
static constexpr u32 sVx =             0x264/4;   //  W PF  -- Voodoo2/Banshee only
static constexpr u32 sVy =             0x268/4;   //  W PF  -- Voodoo2/Banshee only
static constexpr u32 sARGB =           0x26c/4;   //  W PF  -- Voodoo2/Banshee only
static constexpr u32 sRed =            0x270/4;   //  W PF  -- Voodoo2/Banshee only
static constexpr u32 sGreen =          0x274/4;   //  W PF  -- Voodoo2/Banshee only
static constexpr u32 sBlue =           0x278/4;   //  W PF  -- Voodoo2/Banshee only
static constexpr u32 sAlpha =          0x27c/4;   //  W PF  -- Voodoo2/Banshee only

/* 0x280 */
static constexpr u32 sVz =             0x280/4;   //  W PF  -- Voodoo2/Banshee only
static constexpr u32 sWb =             0x284/4;   //  W PF  -- Voodoo2/Banshee only
static constexpr u32 sWtmu0 =          0x288/4;   //  W PF  -- Voodoo2/Banshee only
static constexpr u32 sS_W0 =           0x28c/4;   //  W PF  -- Voodoo2/Banshee only
static constexpr u32 sT_W0 =           0x290/4;   //  W PF  -- Voodoo2/Banshee only
static constexpr u32 sWtmu1 =          0x294/4;   //  W PF  -- Voodoo2/Banshee only
static constexpr u32 sS_Wtmu1 =        0x298/4;   //  W PF  -- Voodoo2/Banshee only
static constexpr u32 sT_Wtmu1 =        0x29c/4;   //  W PF  -- Voodoo2/Banshee only
static constexpr u32 sDrawTriCMD =     0x2a0/4;   //  W PF  -- Voodoo2/Banshee only
static constexpr u32 sBeginTriCMD =    0x2a4/4;   //  W PF  -- Voodoo2/Banshee only

/* 0x2c0 */
static constexpr u32 bltSrcBaseAddr =  0x2c0/4;   // RW PF  -- Voodoo2 only
static constexpr u32 bltDstBaseAddr =  0x2c4/4;   // RW PF  -- Voodoo2 only
static constexpr u32 bltXYStrides =    0x2c8/4;   // RW PF  -- Voodoo2 only
static constexpr u32 bltSrcChromaRange = 0x2cc/4; // RW PF  -- Voodoo2 only
static constexpr u32 bltDstChromaRange = 0x2d0/4; // RW PF  -- Voodoo2 only
static constexpr u32 bltClipX =        0x2d4/4;   // RW PF  -- Voodoo2 only
static constexpr u32 bltClipY =        0x2d8/4;   // RW PF  -- Voodoo2 only
static constexpr u32 bltSrcXY =        0x2e0/4;   // RW PF  -- Voodoo2 only
static constexpr u32 bltDstXY =        0x2e4/4;   // RW PF  -- Voodoo2 only
static constexpr u32 bltSize =         0x2e8/4;   // RW PF  -- Voodoo2 only
static constexpr u32 bltRop =          0x2ec/4;   // RW PF  -- Voodoo2 only
static constexpr u32 bltColor =        0x2f0/4;   // RW PF  -- Voodoo2 only
static constexpr u32 bltCommand =      0x2f8/4;   // RW PF  -- Voodoo2 only
static constexpr u32 bltData =         0x2fc/4;   //  W PF  -- Voodoo2 only

/* 0x300 */
static constexpr u32 textureMode =     0x300/4;   //  W PF
static constexpr u32 tLOD =            0x304/4;   //  W PF
static constexpr u32 tDetail =         0x308/4;   //  W PF
static constexpr u32 texBaseAddr =     0x30c/4;   //  W PF
static constexpr u32 texBaseAddr_1 =   0x310/4;   //  W PF
static constexpr u32 texBaseAddr_2 =   0x314/4;   //  W PF
static constexpr u32 texBaseAddr_3_8 = 0x318/4;   //  W PF
static constexpr u32 trexInit0 =       0x31c/4;   //  W  F  -- Voodoo/Voodoo2 only
static constexpr u32 trexInit1 =       0x320/4;   //  W  F
static constexpr u32 nccTable =        0x324/4;   //  W  F



// 2D registers
static constexpr u32 banshee2D_clip0Min =       0x008/4;
static constexpr u32 banshee2D_clip0Max =       0x00c/4;
static constexpr u32 banshee2D_dstBaseAddr =    0x010/4;
static constexpr u32 banshee2D_dstFormat =      0x014/4;
static constexpr u32 banshee2D_srcColorkeyMin = 0x018/4;
static constexpr u32 banshee2D_srcColorkeyMax = 0x01c/4;
static constexpr u32 banshee2D_dstColorkeyMin = 0x020/4;
static constexpr u32 banshee2D_dstColorkeyMax = 0x024/4;
static constexpr u32 banshee2D_bresError0 =     0x028/4;
static constexpr u32 banshee2D_bresError1 =     0x02c/4;
static constexpr u32 banshee2D_rop =            0x030/4;
static constexpr u32 banshee2D_srcBaseAddr =    0x034/4;
static constexpr u32 banshee2D_commandExtra =   0x038/4;
static constexpr u32 banshee2D_lineStipple =    0x03c/4;
static constexpr u32 banshee2D_lineStyle =      0x040/4;
static constexpr u32 banshee2D_pattern0Alias =  0x044/4;
static constexpr u32 banshee2D_pattern1Alias =  0x048/4;
static constexpr u32 banshee2D_clip1Min =       0x04c/4;
static constexpr u32 banshee2D_clip1Max =       0x050/4;
static constexpr u32 banshee2D_srcFormat =      0x054/4;
static constexpr u32 banshee2D_srcSize =        0x058/4;
static constexpr u32 banshee2D_srcXY =          0x05c/4;
static constexpr u32 banshee2D_colorBack =      0x060/4;
static constexpr u32 banshee2D_colorFore =      0x064/4;
static constexpr u32 banshee2D_dstSize =        0x068/4;
static constexpr u32 banshee2D_dstXY =          0x06c/4;
static constexpr u32 banshee2D_command =        0x070/4;


/*************************************
 *
 *  Voodoo Banshee I/O space registers
 *
 *************************************/

/* 0x000 */
static constexpr u32 io_status =                       0x000/4;
static constexpr u32 io_pciInit0 =                     0x004/4;
static constexpr u32 io_sipMonitor =                   0x008/4;
static constexpr u32 io_lfbMemoryConfig =              0x00c/4;
static constexpr u32 io_miscInit0 =                    0x010/4;
static constexpr u32 io_miscInit1 =                    0x014/4;
static constexpr u32 io_dramInit0 =                    0x018/4;
static constexpr u32 io_dramInit1 =                    0x01c/4;
static constexpr u32 io_agpInit =                      0x020/4;
static constexpr u32 io_tmuGbeInit =                   0x024/4;
static constexpr u32 io_vgaInit0 =                     0x028/4;
static constexpr u32 io_vgaInit1 =                     0x02c/4;
static constexpr u32 io_dramCommand =                  0x030/4;
static constexpr u32 io_dramData =                     0x034/4;

/* 0x040 */
static constexpr u32 io_pllCtrl0 =                     0x040/4;
static constexpr u32 io_pllCtrl1 =                     0x044/4;
static constexpr u32 io_pllCtrl2 =                     0x048/4;
static constexpr u32 io_dacMode =                      0x04c/4;
static constexpr u32 io_dacAddr =                      0x050/4;
static constexpr u32 io_dacData =                      0x054/4;
static constexpr u32 io_rgbMaxDelta =                  0x058/4;
static constexpr u32 io_vidProcCfg =                   0x05c/4;
static constexpr u32 io_hwCurPatAddr =                 0x060/4;
static constexpr u32 io_hwCurLoc =                     0x064/4;
static constexpr u32 io_hwCurC0 =                      0x068/4;
static constexpr u32 io_hwCurC1 =                      0x06c/4;
static constexpr u32 io_vidInFormat =                  0x070/4;
static constexpr u32 io_vidInStatus =                  0x074/4;
static constexpr u32 io_vidSerialParallelPort =        0x078/4;
static constexpr u32 io_vidInXDecimDeltas =            0x07c/4;

/* 0x080 */
static constexpr u32 io_vidInDecimInitErrs =           0x080/4;
static constexpr u32 io_vidInYDecimDeltas =            0x084/4;
static constexpr u32 io_vidPixelBufThold =             0x088/4;
static constexpr u32 io_vidChromaMin =                 0x08c/4;
static constexpr u32 io_vidChromaMax =                 0x090/4;
static constexpr u32 io_vidCurrentLine =               0x094/4;
static constexpr u32 io_vidScreenSize =                0x098/4;
static constexpr u32 io_vidOverlayStartCoords =        0x09c/4;
static constexpr u32 io_vidOverlayEndScreenCoord =     0x0a0/4;
static constexpr u32 io_vidOverlayDudx =               0x0a4/4;
static constexpr u32 io_vidOverlayDudxOffsetSrcWidth = 0x0a8/4;
static constexpr u32 io_vidOverlayDvdy =               0x0ac/4;
static constexpr u32 io_vgab0 =                        0x0b0/4;
static constexpr u32 io_vgab4 =                        0x0b4/4;
static constexpr u32 io_vgab8 =                        0x0b8/4;
static constexpr u32 io_vgabc =                        0x0bc/4;

/* 0x0c0 */
static constexpr u32 io_vgac0 =                        0x0c0/4;
static constexpr u32 io_vgac4 =                        0x0c4/4;
static constexpr u32 io_vgac8 =                        0x0c8/4;
static constexpr u32 io_vgacc =                        0x0cc/4;
static constexpr u32 io_vgad0 =                        0x0d0/4;
static constexpr u32 io_vgad4 =                        0x0d4/4;
static constexpr u32 io_vgad8 =                        0x0d8/4;
static constexpr u32 io_vgadc =                        0x0dc/4;
static constexpr u32 io_vidOverlayDvdyOffset =         0x0e0/4;
static constexpr u32 io_vidDesktopStartAddr =          0x0e4/4;
static constexpr u32 io_vidDesktopOverlayStride =      0x0e8/4;
static constexpr u32 io_vidInAddr0 =                   0x0ec/4;
static constexpr u32 io_vidInAddr1 =                   0x0f0/4;
static constexpr u32 io_vidInAddr2 =                   0x0f4/4;
static constexpr u32 io_vidInStride =                  0x0f8/4;
static constexpr u32 io_vidCurrOverlayStartAddr =      0x0fc/4;



/*************************************
 *
 *  Voodoo Banshee AGP space registers
 *
 *************************************/

/* 0x000 */
static constexpr u32 agpReqSize =              0x000/4;
static constexpr u32 agpHostAddressLow =       0x004/4;
static constexpr u32 agpHostAddressHigh =      0x008/4;
static constexpr u32 agpGraphicsAddress =      0x00c/4;
static constexpr u32 agpGraphicsStride =       0x010/4;
static constexpr u32 agpMoveCMD =              0x014/4;
static constexpr u32 cmdBaseAddr0 =            0x020/4;
static constexpr u32 cmdBaseSize0 =            0x024/4;
static constexpr u32 cmdBump0 =                0x028/4;
static constexpr u32 cmdRdPtrL0 =              0x02c/4;
static constexpr u32 cmdRdPtrH0 =              0x030/4;
static constexpr u32 cmdAMin0 =                0x034/4;
static constexpr u32 cmdAMax0 =                0x03c/4;

/* 0x040 */
static constexpr u32 cmdFifoDepth0 =           0x044/4;
static constexpr u32 cmdHoleCnt0 =             0x048/4;
static constexpr u32 cmdBaseAddr1 =            0x050/4;
static constexpr u32 cmdBaseSize1 =            0x054/4;
static constexpr u32 cmdBump1 =                0x058/4;
static constexpr u32 cmdRdPtrL1 =              0x05c/4;
static constexpr u32 cmdRdPtrH1 =              0x060/4;
static constexpr u32 cmdAMin1 =                0x064/4;
static constexpr u32 cmdAMax1 =                0x06c/4;
static constexpr u32 cmdFifoDepth1 =           0x074/4;
static constexpr u32 cmdHoleCnt1 =             0x078/4;

/* 0x080 */
static constexpr u32 cmdFifoThresh =           0x080/4;
static constexpr u32 cmdHoleInt =              0x084/4;

/* 0x100 */
static constexpr u32 yuvBaseAddress =          0x100/4;
static constexpr u32 yuvStride =               0x104/4;
static constexpr u32 crc1 =                    0x120/4;
static constexpr u32 crc2 =                    0x130/4;



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

#define TREXINIT_SEND_TMU_CONFIG(val)       (((val) >> 18) & 1)


}

#endif // MAME_VIDEO_VOODOO_REGS_H
