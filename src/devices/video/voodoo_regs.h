// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo_regs.h

    3dfx Voodoo Graphics SST-1/2 emulator.

***************************************************************************/

#ifndef MAME_VIDEO_VOODOO_REGS_H
#define MAME_VIDEO_VOODOO_REGS_H

#pragma once


namespace voodoo
{

//**************************************************************************
//  SPECIFIC REGISTER TYPES
//**************************************************************************

// ======================> reg_color

class reg_color
{
public:
	constexpr reg_color(u32 value) :
		m_value(value) { }

	constexpr u32 blue() const            { return BIT(m_value, 0, 8); }
	constexpr u32 green() const           { return BIT(m_value, 8, 8); }
	constexpr u32 red() const             { return BIT(m_value, 16, 8); }
	constexpr u32 alpha() const           { return BIT(m_value, 24, 8); }
	constexpr rgb_t argb() const          { return m_value; }

private:
	u32 m_value;
};


// ======================> reg_clip_minmax

class reg_clip_minmax
{
public:
	constexpr reg_clip_minmax(u32 value) :
		m_value(value) { }

	constexpr u32 min() const { return BIT(m_value, 16, 10); }
	constexpr u32 max() const { return BIT(m_value, 0, 10); }

private:
	u32 m_value;
};


// ======================> reg_fbz_colorpath

class reg_fbz_colorpath
{
public:
	static constexpr u32 DECODE_LIVE = 0xfffffffe;

	constexpr reg_fbz_colorpath(u32 value) :
		m_value(value) { }

	constexpr reg_fbz_colorpath(u32 normalized, reg_fbz_colorpath const live) :
		m_value((normalized == DECODE_LIVE) ? live.m_value : normalized) { }

	constexpr u32 raw() const                     { return m_value; }
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
	constexpr u32 antialias() const               { return BIT(m_value, 29, 1); }   // not implemented

	constexpr u32 normalize()
	{
		// ignore the subpixel adjust and texture enable flags
		return m_value & ~((1 << 26) | (1 << 27));
	}

private:
	u32 m_value;
};


// ======================> reg_fbz_mode

class reg_fbz_mode
{
public:
	static constexpr u32 DECODE_LIVE = 0xfffffffe;

	constexpr reg_fbz_mode(u32 value) :
		m_value(value) { }

	constexpr reg_fbz_mode(u32 normalized, reg_fbz_mode const live) :
		m_value((normalized == DECODE_LIVE) ? live.m_value : normalized) { }

	constexpr u32 raw() const                   { return m_value; }
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
	constexpr u32 depth_float_select() const    { return BIT(m_value, 21, 1); } // voodoo 2 only

	constexpr u32 normalize()
	{
		// ignore the draw buffer
		return m_value & ~(3 << 14);
	}

private:
	u32 m_value;
};


// ======================> reg_alpha_mode

class reg_alpha_mode
{
public:
	static constexpr u32 DECODE_LIVE = 0xfffffffe;

	constexpr reg_alpha_mode(u32 value) :
		m_value(value) { }

	constexpr reg_alpha_mode(u32 normalized, reg_alpha_mode const live) :
		m_value((normalized == DECODE_LIVE) ? live.m_value : normalized) { }

	constexpr u32 raw() const           { return m_value; }
	constexpr u32 alphatest() const     { return BIT(m_value, 0, 1); }
	constexpr u32 alphafunction() const { return BIT(m_value, 1, 3); }
	constexpr u32 alphablend() const    { return BIT(m_value, 4, 1); }
	constexpr u32 antialias() const     { return BIT(m_value, 5, 1); }  // not implemented
	constexpr u32 srcrgbblend() const   { return BIT(m_value, 8, 4); }
	constexpr u32 dstrgbblend() const   { return BIT(m_value, 12, 4); }
	constexpr u32 srcalphablend() const { return BIT(m_value, 16, 4); }
	constexpr u32 dstalphablend() const { return BIT(m_value, 20, 4); }
	constexpr u32 alpharef() const      { return BIT(m_value, 24, 8); }

	constexpr u32 normalize()
	{
		// always ignore the alpha ref; it is stashed in the poly data
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


// ======================> reg_fog_mode

class reg_fog_mode
{
public:
	static constexpr u32 DECODE_LIVE = 0xfffffffe;

	constexpr reg_fog_mode(u32 value) :
		m_value(value) { }

	constexpr reg_fog_mode(u32 normalized, reg_fog_mode const live) :
		m_value((normalized == DECODE_LIVE) ? live.m_value : normalized) { }

	constexpr u32 raw() const          { return m_value; }
	constexpr u32 enable_fog() const   { return BIT(m_value, 0, 1); }
	constexpr u32 fog_add() const      { return BIT(m_value, 1, 1); }
	constexpr u32 fog_mult() const     { return BIT(m_value, 2, 1); }
	constexpr u32 fog_zalpha() const   { return BIT(m_value, 3, 2); }
	constexpr u32 fog_constant() const { return BIT(m_value, 5, 1); }
	constexpr u32 fog_dither() const   { return BIT(m_value, 6, 1); }   // voodoo 2 only
	constexpr u32 fog_zones() const    { return BIT(m_value, 7, 1); }   // voodoo 2 only

	constexpr u32 normalize()
	{
		// only care about the rest if fog is enabled
		return enable_fog() ? m_value : 0;
	}

private:
	u32 m_value;
};


// ======================> reg_texture_mode

class reg_texture_mode
{
public:
	static constexpr u32 NONE = 0xffffffff;
	static constexpr u32 DECODE_LIVE = 0xfffffffe;

	// these bits are set for special cases in the rasterizer
	// (normally they are masked out)
	static constexpr u32 TMU_CONFIG_MASK = 0x80000000;

	// mask of equation bits
	static constexpr u32 EQUATION_MASK = 0x3ffff000;

	constexpr reg_texture_mode(u32 value) :
		m_value(value) { }

	constexpr reg_texture_mode(u32 normalized, reg_texture_mode const live) :
		m_value((normalized == DECODE_LIVE) ? live.m_value : normalized) { }

	constexpr bool is_none() const { return (m_value == NONE); }
	constexpr bool is_live() const { return (m_value == DECODE_LIVE); }

	constexpr u32 raw() const                  { return m_value; }
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
	constexpr u32 trilinear() const            { return BIT(m_value, 30, 1); }  // not implemented
	constexpr u32 seq_8_downld() const         { return BIT(m_value, 31, 1); }  // repurposed as send_config

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


// ======================> reg_texture_lod

class reg_texture_lod
{
public:
	static constexpr u32 DECODE_LIVE = 0xfffffffe;

	constexpr reg_texture_lod(u32 value) :
		m_value(value) { }

	constexpr reg_texture_lod(u32 normalized, reg_texture_lod const live) :
		m_value((normalized == DECODE_LIVE) ? live.m_value : normalized) { }

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
	constexpr u32 tmirrors() const       { return BIT(m_value, 28, 1); }  // Voodoo Banshee+ only
	constexpr u32 tmirrort() const       { return BIT(m_value, 29, 1); }  // Voodoo Banshee+ only

	constexpr u32 normalize()
	{
		// always normalize to 0 for now
		return 0;
	}

private:
	u32 m_value;
};


// ======================> reg_texture_detail

class reg_texture_detail
{
public:
	constexpr reg_texture_detail(u32 value) :
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


// ======================> reg_lfb_mode

class reg_lfb_mode
{
public:
	constexpr reg_lfb_mode(u32 value) :
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


// ======================> reg_chroma_range

class reg_chroma_range
{
public:
	constexpr reg_chroma_range(u32 value) :
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


// ======================> reg_setup_mode

class reg_setup_mode
{
public:
	constexpr reg_setup_mode(u32 value) :
		m_value(value) { }

	constexpr u32 setup_rgb() const                    { return BIT(m_value, 0, 1); }
	constexpr u32 setup_alpha() const                  { return BIT(m_value, 1, 1); }
	constexpr u32 setup_z() const                      { return BIT(m_value, 2, 1); }
	constexpr u32 setup_wb() const                     { return BIT(m_value, 3, 1); }
	constexpr u32 setup_w0() const                     { return BIT(m_value, 4, 1); }
	constexpr u32 setup_st0() const                    { return BIT(m_value, 5, 1); }
	constexpr u32 setup_w1() const                     { return BIT(m_value, 6, 1); }
	constexpr u32 setup_st1() const                    { return BIT(m_value, 7, 1); }
	constexpr u32 fan_mode() const                     { return BIT(m_value, 16, 1); }
	constexpr u32 enable_culling() const               { return BIT(m_value, 17, 1); }
	constexpr u32 culling_sign() const                 { return BIT(m_value, 18, 1); }
	constexpr u32 disable_ping_pong_correction() const { return BIT(m_value, 19, 1); }

private:
	u32 m_value;
};


// ======================> reg_init_en

class reg_init_en
{
public:
	constexpr reg_init_en(u32 value) :
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
	constexpr u32 secondary_rev_id() const         { return BIT(m_value, 12, 4); }  // voodoo 2 only
	constexpr u32 mfctr_fab_id() const             { return BIT(m_value, 16, 4); }  // voodoo 2 only
	constexpr u32 enable_pci_interrupt() const     { return BIT(m_value, 20, 1); }  // voodoo 2 only
	constexpr u32 pci_interrupt_timeout() const    { return BIT(m_value, 21, 1); }  // voodoo 2 only
	constexpr u32 enable_nand_tree_test() const    { return BIT(m_value, 22, 1); }  // voodoo 2 only
	constexpr u32 enable_sli_address_snoop() const { return BIT(m_value, 23, 1); }  // voodoo 2 only
	constexpr u32 sli_snoop_address() const        { return BIT(m_value, 24, 8); }  // voodoo 2 only

private:
	u32 m_value;
};


// ======================> reg_fbi_init0

class reg_fbi_init0
{
public:
	constexpr reg_fbi_init0(u32 value) :
		m_value(value) { }

	constexpr u32 vga_passthru() const          { return BIT(m_value, 0, 1); }
	constexpr u32 graphics_reset() const        { return BIT(m_value, 1, 1); }
	constexpr u32 fifo_reset() const            { return BIT(m_value, 2, 1); }
	constexpr u32 swizzle_reg_writes() const    { return BIT(m_value, 3, 1); }
	constexpr u32 stall_pcie_for_hwm() const    { return BIT(m_value, 4, 1); }
	constexpr u32 pci_fifo_lwm() const          { return BIT(m_value, 6, 5); }
	constexpr u32 lfb_to_memory_fifo() const    { return BIT(m_value, 11, 1); }
	constexpr u32 texmem_to_memory_fifo() const { return BIT(m_value, 12, 1); }
	constexpr u32 enable_memory_fifo() const    { return BIT(m_value, 13, 1); }
	constexpr u32 memory_fifo_hwm() const       { return BIT(m_value, 14, 11); }
	constexpr u32 memory_fifo_burst() const     { return BIT(m_value, 25, 6); }

private:
	u32 m_value;
};


// ======================> reg_fbi_init1

class reg_fbi_init1
{
public:
	constexpr reg_fbi_init1(u32 value) :
		m_value(value) { }

	constexpr u32 pci_dev_function() const       { return BIT(m_value, 0, 1); }
	constexpr u32 pci_write_wait_states() const  { return BIT(m_value, 1, 1); }
	constexpr u32 multi_sst1() const             { return BIT(m_value, 2, 1); } // not on voodoo 2
	constexpr u32 enable_lfb() const             { return BIT(m_value, 3, 1); }
	constexpr u32 x_video_tiles() const          { return BIT(m_value, 4, 4); }
	constexpr u32 video_timing_reset() const     { return BIT(m_value, 8, 1); }
	constexpr u32 software_override() const      { return BIT(m_value, 9, 1); }
	constexpr u32 software_hsync() const         { return BIT(m_value, 10, 1); }
	constexpr u32 software_vsync() const         { return BIT(m_value, 11, 1); }
	constexpr u32 software_blank() const         { return BIT(m_value, 12, 1); }
	constexpr u32 drive_video_timing() const     { return BIT(m_value, 13, 1); }
	constexpr u32 drive_video_blank() const      { return BIT(m_value, 14, 1); }
	constexpr u32 drive_video_sync() const       { return BIT(m_value, 15, 1); }
	constexpr u32 drive_video_dclk() const       { return BIT(m_value, 16, 1); }
	constexpr u32 video_timing_vclk() const      { return BIT(m_value, 17, 1); }
	constexpr u32 video_clk_2x_delay() const     { return BIT(m_value, 18, 2); }
	constexpr u32 video_timing_source() const    { return BIT(m_value, 20, 2); }
	constexpr u32 enable_24bpp_output() const    { return BIT(m_value, 22, 1); }
	constexpr u32 enable_sli() const             { return BIT(m_value, 23, 1); }
	constexpr u32 x_video_tiles_bit5() const     { return BIT(m_value, 24, 1); }    // voodoo 2 only
	constexpr u32 enable_edge_filter() const     { return BIT(m_value, 25, 1); }
	constexpr u32 invert_vid_clk_2x() const      { return BIT(m_value, 26, 1); }
	constexpr u32 vid_clk_2x_sel_delay() const   { return BIT(m_value, 27, 2); }
	constexpr u32 vid_clk_delay() const          { return BIT(m_value, 29, 2); }
	constexpr u32 disable_fast_readahead() const { return BIT(m_value, 31, 1); }

private:
	u32 m_value;
};


// ======================> reg_fbi_init2

class reg_fbi_init2
{
public:
	constexpr reg_fbi_init2(u32 value) :
		m_value(value) { }

	constexpr u32 disable_dither_sub() const     { return BIT(m_value, 0, 1); }
	constexpr u32 dram_banking() const           { return BIT(m_value, 1, 1); }
	constexpr u32 enable_triple_buf() const      { return BIT(m_value, 4, 1); }
	constexpr u32 enable_fast_ras_read() const   { return BIT(m_value, 5, 1); }
	constexpr u32 enable_get_dram_oe() const     { return BIT(m_value, 6, 1); }
	constexpr u32 enable_fast_readwrite() const  { return BIT(m_value, 7, 1); }
	constexpr u32 enable_passthru_dither() const { return BIT(m_value, 8, 1); }
	constexpr u32 swap_buffer_algorithm() const  { return BIT(m_value, 9, 2); }
	constexpr u32 video_buffer_offset() const    { return BIT(m_value, 11, 9); }
	constexpr u32 enable_dram_banking() const    { return BIT(m_value, 20, 1); }
	constexpr u32 enable_dram_read_fifo() const  { return BIT(m_value, 21, 1); }
	constexpr u32 enable_dram_refresh() const    { return BIT(m_value, 22, 1); }
	constexpr u32 refresh_load_value() const     { return BIT(m_value, 23, 9); }

private:
	u32 m_value;
};


// ======================> reg_fbi_init3

class reg_fbi_init3
{
public:
	constexpr reg_fbi_init3(u32 value) :
		m_value(value) { }

	constexpr u32 tri_register_remap() const { return BIT(m_value, 0, 1); }
	constexpr u32 video_fifo_thresh() const  { return BIT(m_value, 1, 5); }
	constexpr u32 disable_tmus() const       { return BIT(m_value, 6, 1); }
	constexpr u32 fbi_memory_type() const    { return BIT(m_value, 8, 3); }
	constexpr u32 vga_pass_reset_val() const { return BIT(m_value, 11, 1); }
	constexpr u32 hardcode_pci_base() const  { return BIT(m_value, 12, 1); }
	constexpr u32 fbi2trex_delay() const     { return BIT(m_value, 13, 4); }
	constexpr u32 trex2fbi_delay() const     { return BIT(m_value, 17, 5); }
	constexpr u32 yorigin_subtract() const   { return BIT(m_value, 22, 10); }

private:
	u32 m_value;
};


// ======================> reg_fbi_init4

class reg_fbi_init4
{
public:
	constexpr reg_fbi_init4(u32 value) :
		m_value(value) { }

	constexpr u32 pci_read_waits() const        { return BIT(m_value, 0, 1); }
	constexpr u32 enable_lfb_readahead() const  { return BIT(m_value, 1, 1); }
	constexpr u32 memory_fifo_lwm() const       { return BIT(m_value, 2, 6); }
	constexpr u32 memory_fifo_start_row() const { return BIT(m_value, 8, 10); }
	constexpr u32 memory_fifo_stop_row() const  { return BIT(m_value, 18, 10); }
	constexpr u32 video_clocking_delay() const  { return BIT(m_value, 29, 7); } // voodoo 2 only

private:
	u32 m_value;
};


// ======================> reg_fbi_init5

class reg_fbi_init5
{
public:
	constexpr reg_fbi_init5(u32 value) :
		m_value(value) { }

	constexpr u32 disable_pci_stop() const      { return BIT(m_value, 0, 1); }  // voodoo 2 only
	constexpr u32 pci_slave_speed() const       { return BIT(m_value, 1, 1); }  // voodoo 2 only
	constexpr u32 dac_data_output_width() const { return BIT(m_value, 2, 1); }  // voodoo 2 only
	constexpr u32 dac_data_17_output() const    { return BIT(m_value, 3, 1); }  // voodoo 2 only
	constexpr u32 dac_data_18_output() const    { return BIT(m_value, 4, 1); }  // voodoo 2 only
	constexpr u32 generic_strapping() const     { return BIT(m_value, 5, 4); }  // voodoo 2 only
	constexpr u32 buffer_allocation() const     { return BIT(m_value, 9, 2); }  // voodoo 2 only
	constexpr u32 drive_vid_clk_slave() const   { return BIT(m_value, 11, 1); } // voodoo 2 only
	constexpr u32 drive_dac_data_16() const     { return BIT(m_value, 12, 1); } // voodoo 2 only
	constexpr u32 vclk_input_select() const     { return BIT(m_value, 13, 1); } // voodoo 2 only
	constexpr u32 multi_cvg_detect() const      { return BIT(m_value, 14, 1); } // voodoo 2 only
	constexpr u32 sync_retrace_reads() const    { return BIT(m_value, 15, 1); } // voodoo 2 only
	constexpr u32 enable_rhborder_color() const { return BIT(m_value, 16, 1); } // voodoo 2 only
	constexpr u32 enable_lhborder_color() const { return BIT(m_value, 17, 1); } // voodoo 2 only
	constexpr u32 enable_bvborder_color() const { return BIT(m_value, 18, 1); } // voodoo 2 only
	constexpr u32 enable_tvborder_color() const { return BIT(m_value, 19, 1); } // voodoo 2 only
	constexpr u32 double_horiz() const          { return BIT(m_value, 20, 1); } // voodoo 2 only
	constexpr u32 double_vert() const           { return BIT(m_value, 21, 1); } // voodoo 2 only
	constexpr u32 enable_16bit_gamma() const    { return BIT(m_value, 22, 1); } // voodoo 2 only
	constexpr u32 invert_dac_hsync() const      { return BIT(m_value, 23, 1); } // voodoo 2 only
	constexpr u32 invert_dac_vsync() const      { return BIT(m_value, 24, 1); } // voodoo 2 only
	constexpr u32 enable_24bit_dacdata() const  { return BIT(m_value, 25, 1); } // voodoo 2 only
	constexpr u32 enable_interlacing() const    { return BIT(m_value, 26, 1); } // voodoo 2 only
	constexpr u32 dac_data_18_control() const   { return BIT(m_value, 27, 1); } // voodoo 2 only
	constexpr u32 rasterizer_unit_mode() const  { return BIT(m_value, 30, 2); } // voodoo 2 only

private:
	u32 m_value;
};


// ======================> reg_fbi_init6

class reg_fbi_init6
{
public:
	constexpr reg_fbi_init6(u32 value) :
		m_value(value) { }

	constexpr u32 window_active_counter() const { return BIT(m_value, 0, 3); }  // voodoo 2 only
	constexpr u32 window_drag_counter() const   { return BIT(m_value, 3, 5); }  // voodoo 2 only
	constexpr u32 sli_sync_master() const       { return BIT(m_value, 8, 1); }  // voodoo 2 only
	constexpr u32 dac_data_22_output() const    { return BIT(m_value, 9, 2); }  // voodoo 2 only
	constexpr u32 dac_data_23_output() const    { return BIT(m_value, 11, 2); } // voodoo 2 only
	constexpr u32 sli_syncin_output() const     { return BIT(m_value, 13, 2); } // voodoo 2 only
	constexpr u32 sli_syncout_output() const    { return BIT(m_value, 15, 2); } // voodoo 2 only
	constexpr u32 dac_rd_output() const         { return BIT(m_value, 17, 2); } // voodoo 2 only
	constexpr u32 dac_wr_output() const         { return BIT(m_value, 19, 2); } // voodoo 2 only
	constexpr u32 pci_fifo_lwm_rdy() const      { return BIT(m_value, 21, 7); } // voodoo 2 only
	constexpr u32 vga_pass_n_output() const     { return BIT(m_value, 28, 2); } // voodoo 2 only
	constexpr u32 x_video_tiles_bit0() const    { return BIT(m_value, 30, 1); } // voodoo 2 only

private:
	u32 m_value;
};


// ======================> reg_fbi_init7

class reg_fbi_init7
{
public:
	constexpr reg_fbi_init7(u32 value) :
		m_value(value) { }

	constexpr u32 generic_strapping() const     { return BIT(m_value, 0, 8); }  // voodoo 2 only
	constexpr u32 cmdfifo_enable() const        { return BIT(m_value, 8, 1); }  // voodoo 2 only
	constexpr u32 cmdfifo_memory_store() const  { return BIT(m_value, 9, 1); }  // voodoo 2 only
	constexpr u32 disable_cmdfifo_holes() const { return BIT(m_value, 10, 1); } // voodoo 2 only
	constexpr u32 cmdfifo_read_thresh() const   { return BIT(m_value, 11, 5); } // voodoo 2 only
	constexpr u32 sync_cmdfifo_writes() const   { return BIT(m_value, 16, 1); } // voodoo 2 only
	constexpr u32 sync_cmdfifo_reads() const    { return BIT(m_value, 17, 1); } // voodoo 2 only
	constexpr u32 reset_pci_packer() const      { return BIT(m_value, 18, 1); } // voodoo 2 only
	constexpr u32 enable_chroma_stuff() const   { return BIT(m_value, 19, 1); } // voodoo 2 only
	constexpr u32 cmdfifo_pci_timeout() const   { return BIT(m_value, 20, 7); } // voodoo 2 only
	constexpr u32 enable_texture_burst() const  { return BIT(m_value, 27, 1); } // voodoo 2 only

private:
	u32 m_value;
};


// ======================> reg_intr_ctrl

class reg_intr_ctrl
{
public:
	static constexpr u32 HSYNC_RISING_GENERATED = 1 << 6;
	static constexpr u32 HSYNC_FALLING_GENERATED = 1 << 7;
	static constexpr u32 VSYNC_RISING_GENERATED = 1 << 8;
	static constexpr u32 VSYNC_FALLING_GENERATED = 1 << 9;
	static constexpr u32 PCI_FIFO_FULL_GENERATED = 1 << 10;
	static constexpr u32 USER_INTERRUPT_GENERATED = 1 << 11;
	static constexpr u32 USER_INTERRUPT_TAG_MASK = 0xff << 12;
	static constexpr u32 EXTERNAL_PIN_ACTIVE = 1 << 31;

	constexpr reg_intr_ctrl(u32 value) :
		m_value(value) { }

	constexpr u32 hsync_rising_enable() const        { return BIT(m_value, 0, 1); }
	constexpr u32 hsync_falling_enable() const       { return BIT(m_value, 1, 1); }
	constexpr u32 vsync_rising_enable() const        { return BIT(m_value, 2, 1); }
	constexpr u32 vsync_falling_enable() const       { return BIT(m_value, 3, 1); }
	constexpr u32 pci_fifo_full_enable() const       { return BIT(m_value, 4, 1); }
	constexpr u32 user_interrupt_enable() const      { return BIT(m_value, 5, 1); }
	constexpr u32 hsync_rising_generated() const     { return BIT(m_value, 6, 1); }
	constexpr u32 hsync_falling_generated() const    { return BIT(m_value, 7, 1); }
	constexpr u32 vsync_rising_generated() const     { return BIT(m_value, 8, 1); }
	constexpr u32 vsync_falling_generated() const    { return BIT(m_value, 9, 1); }
	constexpr u32 pci_fifo_full_generated() const    { return BIT(m_value, 10, 1); }
	constexpr u32 user_interrupt_generated() const   { return BIT(m_value, 11, 1); }
	constexpr u32 user_interrupt_command_tag() const { return BIT(m_value, 12, 8); }
	constexpr u32 external_pin_active() const        { return BIT(m_value, 31, 1); }

private:
	u32 m_value;
};


// ======================> reg_hsync

template<bool Rev1>
class reg_hsync
{
public:
	constexpr reg_hsync(u32 value) :
		m_value(value) { }

	constexpr u32 raw() const       { return m_value; }
	constexpr u32 hsync_on() const  { return BIT(m_value, 0, Rev1 ? 8 : 9); }
	constexpr u32 hsync_off() const { return BIT(m_value, 16, Rev1 ? 10 : 11); }

private:
	u32 m_value;
};


// ======================> reg_vsync

template<bool Rev1>
class reg_vsync
{
public:
	constexpr reg_vsync(u32 value) :
		m_value(value) { }

	constexpr u32 raw() const       { return m_value; }
	constexpr u32 vsync_on() const  { return BIT(m_value, 0, Rev1 ? 12 : 13); }
	constexpr u32 vsync_off() const { return BIT(m_value, 16, Rev1 ? 12 : 13); }

private:
	u32 m_value;
};


// ======================> reg_video_dimensions

template<bool Rev1>
class reg_video_dimensions
{
public:
	constexpr reg_video_dimensions(u32 value) :
		m_value(value) { }

	constexpr u32 raw() const     { return m_value; }
	constexpr u32 xwidth() const  { return BIT(m_value, 0, Rev1 ? 10 : 11); }
	constexpr u32 yheight() const { return BIT(m_value, 16, Rev1 ? 10 : 11); }

private:
	u32 m_value;
};


// ======================> reg_back_porch

template<bool Rev1>
class reg_back_porch
{
public:
	constexpr reg_back_porch(u32 value) :
		m_value(value) { }

	constexpr u32 raw() const        { return m_value; }
	constexpr u32 horizontal() const { return BIT(m_value, 0, Rev1 ? 8 : 9); }
	constexpr u32 vertical() const   { return BIT(m_value, 16, Rev1 ? 8 : 9); }

private:
	u32 m_value;
};



//**************************************************************************
//  CORE VOODOO REGISTERS
//**************************************************************************

// ======================> voodoo_regs

class voodoo_regs
{
public:
	static constexpr s32 s24(s32 value) { return s32(value << 8) >> 8; }

	// 0x000
	static constexpr u32 reg_vdstatus =        0x000/4;   // R  P
	static constexpr u32 reg_intrCtrl =        0x004/4;   // RW P   -- Voodoo2/Banshee only
	static constexpr u32 reg_vertexAx =        0x008/4;   //  W PF
	static constexpr u32 reg_vertexAy =        0x00c/4;   //  W PF
	static constexpr u32 reg_vertexBx =        0x010/4;   //  W PF
	static constexpr u32 reg_vertexBy =        0x014/4;   //  W PF
	static constexpr u32 reg_vertexCx =        0x018/4;   //  W PF
	static constexpr u32 reg_vertexCy =        0x01c/4;   //  W PF
	static constexpr u32 reg_startR =          0x020/4;   //  W PF
	static constexpr u32 reg_startG =          0x024/4;   //  W PF
	static constexpr u32 reg_startB =          0x028/4;   //  W PF
	static constexpr u32 reg_startZ =          0x02c/4;   //  W PF
	static constexpr u32 reg_startA =          0x030/4;   //  W PF
	static constexpr u32 reg_startS =          0x034/4;   //  W PF
	static constexpr u32 reg_startT =          0x038/4;   //  W PF
	static constexpr u32 reg_startW =          0x03c/4;   //  W PF

	// 0x040
	static constexpr u32 reg_dRdX =            0x040/4;   //  W PF
	static constexpr u32 reg_dGdX =            0x044/4;   //  W PF
	static constexpr u32 reg_dBdX =            0x048/4;   //  W PF
	static constexpr u32 reg_dZdX =            0x04c/4;   //  W PF
	static constexpr u32 reg_dAdX =            0x050/4;   //  W PF
	static constexpr u32 reg_dSdX =            0x054/4;   //  W PF
	static constexpr u32 reg_dTdX =            0x058/4;   //  W PF
	static constexpr u32 reg_dWdX =            0x05c/4;   //  W PF
	static constexpr u32 reg_dRdY =            0x060/4;   //  W PF
	static constexpr u32 reg_dGdY =            0x064/4;   //  W PF
	static constexpr u32 reg_dBdY =            0x068/4;   //  W PF
	static constexpr u32 reg_dZdY =            0x06c/4;   //  W PF
	static constexpr u32 reg_dAdY =            0x070/4;   //  W PF
	static constexpr u32 reg_dSdY =            0x074/4;   //  W PF
	static constexpr u32 reg_dTdY =            0x078/4;   //  W PF
	static constexpr u32 reg_dWdY =            0x07c/4;   //  W PF

	// 0x080
	static constexpr u32 reg_triangleCMD =     0x080/4;   //  W PF
	static constexpr u32 reg_fvertexAx =       0x088/4;   //  W PF
	static constexpr u32 reg_fvertexAy =       0x08c/4;   //  W PF
	static constexpr u32 reg_fvertexBx =       0x090/4;   //  W PF
	static constexpr u32 reg_fvertexBy =       0x094/4;   //  W PF
	static constexpr u32 reg_fvertexCx =       0x098/4;   //  W PF
	static constexpr u32 reg_fvertexCy =       0x09c/4;   //  W PF
	static constexpr u32 reg_fstartR =         0x0a0/4;   //  W PF
	static constexpr u32 reg_fstartG =         0x0a4/4;   //  W PF
	static constexpr u32 reg_fstartB =         0x0a8/4;   //  W PF
	static constexpr u32 reg_fstartZ =         0x0ac/4;   //  W PF
	static constexpr u32 reg_fstartA =         0x0b0/4;   //  W PF
	static constexpr u32 reg_fstartS =         0x0b4/4;   //  W PF
	static constexpr u32 reg_fstartT =         0x0b8/4;   //  W PF
	static constexpr u32 reg_fstartW =         0x0bc/4;   //  W PF

	// 0x0c0
	static constexpr u32 reg_fdRdX =           0x0c0/4;   //  W PF
	static constexpr u32 reg_fdGdX =           0x0c4/4;   //  W PF
	static constexpr u32 reg_fdBdX =           0x0c8/4;   //  W PF
	static constexpr u32 reg_fdZdX =           0x0cc/4;   //  W PF
	static constexpr u32 reg_fdAdX =           0x0d0/4;   //  W PF
	static constexpr u32 reg_fdSdX =           0x0d4/4;   //  W PF
	static constexpr u32 reg_fdTdX =           0x0d8/4;   //  W PF
	static constexpr u32 reg_fdWdX =           0x0dc/4;   //  W PF
	static constexpr u32 reg_fdRdY =           0x0e0/4;   //  W PF
	static constexpr u32 reg_fdGdY =           0x0e4/4;   //  W PF
	static constexpr u32 reg_fdBdY =           0x0e8/4;   //  W PF
	static constexpr u32 reg_fdZdY =           0x0ec/4;   //  W PF
	static constexpr u32 reg_fdAdY =           0x0f0/4;   //  W PF
	static constexpr u32 reg_fdSdY =           0x0f4/4;   //  W PF
	static constexpr u32 reg_fdTdY =           0x0f8/4;   //  W PF
	static constexpr u32 reg_fdWdY =           0x0fc/4;   //  W PF

	// 0x100
	static constexpr u32 reg_ftriangleCMD =    0x100/4;   //  W PF
	static constexpr u32 reg_fbzColorPath =    0x104/4;   // RW PF
	static constexpr u32 reg_fogMode =         0x108/4;   // RW PF
	static constexpr u32 reg_alphaMode =       0x10c/4;   // RW PF
	static constexpr u32 reg_fbzMode =         0x110/4;   // RW  F
	static constexpr u32 reg_lfbMode =         0x114/4;   // RW  F
	static constexpr u32 reg_clipLeftRight =   0x118/4;   // RW  F
	static constexpr u32 reg_clipLowYHighY =   0x11c/4;   // RW  F
	static constexpr u32 reg_nopCMD =          0x120/4;   //  W  F
	static constexpr u32 reg_fastfillCMD =     0x124/4;   //  W  F
	static constexpr u32 reg_swapbufferCMD =   0x128/4;   //  W  F
	static constexpr u32 reg_fogColor =        0x12c/4;   //  W  F
	static constexpr u32 reg_zaColor =         0x130/4;   //  W  F
	static constexpr u32 reg_chromaKey =       0x134/4;   //  W  F
	static constexpr u32 reg_chromaRange =     0x138/4;   //  W  F  -- Voodoo2/Banshee only
	static constexpr u32 reg_userIntrCMD =     0x13c/4;   //  W  F  -- Voodoo2/Banshee only

	// 0x140
	static constexpr u32 reg_stipple =         0x140/4;   // RW  F
	static constexpr u32 reg_color0 =          0x144/4;   // RW  F
	static constexpr u32 reg_color1 =          0x148/4;   // RW  F
	static constexpr u32 reg_fbiPixelsIn =     0x14c/4;   // R
	static constexpr u32 reg_fbiChromaFail =   0x150/4;   // R
	static constexpr u32 reg_fbiZfuncFail =    0x154/4;   // R
	static constexpr u32 reg_fbiAfuncFail =    0x158/4;   // R
	static constexpr u32 reg_fbiPixelsOut =    0x15c/4;   // R
	static constexpr u32 reg_fogTable =        0x160/4;   //  W  F

	// 0x1c0
	static constexpr u32 reg_cmdFifoBaseAddr = 0x1e0/4;   // RW     -- Voodoo2 only
	static constexpr u32 reg_cmdFifoBump =     0x1e4/4;   // RW     -- Voodoo2 only
	static constexpr u32 reg_cmdFifoRdPtr =    0x1e8/4;   // RW     -- Voodoo2 only
	static constexpr u32 reg_cmdFifoAMin =     0x1ec/4;   // RW     -- Voodoo2 only
	static constexpr u32 reg_colBufferAddr =   0x1ec/4;   // RW     -- Banshee only
	static constexpr u32 reg_cmdFifoAMax =     0x1f0/4;   // RW     -- Voodoo2 only
	static constexpr u32 reg_colBufferStride = 0x1f0/4;   // RW     -- Banshee only
	static constexpr u32 reg_cmdFifoDepth =    0x1f4/4;   // RW     -- Voodoo2 only
	static constexpr u32 reg_auxBufferAddr =   0x1f4/4;   // RW     -- Banshee only
	static constexpr u32 reg_cmdFifoHoles =    0x1f8/4;   // RW     -- Voodoo2 only
	static constexpr u32 reg_auxBufferStride = 0x1f8/4;   // RW     -- Banshee only

	// 0x200
	static constexpr u32 reg_fbiInit4 =        0x200/4;   // RW     -- Voodoo/Voodoo2 only
	static constexpr u32 reg_clipLeftRight1 =  0x200/4;   // RW     -- Banshee only
	static constexpr u32 reg_vRetrace =        0x204/4;   // R      -- Voodoo/Voodoo2 only
	static constexpr u32 reg_clipTopBottom1 =  0x204/4;   // RW     -- Banshee only
	static constexpr u32 reg_backPorch =       0x208/4;   // RW     -- Voodoo/Voodoo2 only
	static constexpr u32 reg_videoDimensions = 0x20c/4;   // RW     -- Voodoo/Voodoo2 only
	static constexpr u32 reg_fbiInit0 =        0x210/4;   // RW     -- Voodoo/Voodoo2 only
	static constexpr u32 reg_fbiInit1 =        0x214/4;   // RW     -- Voodoo/Voodoo2 only
	static constexpr u32 reg_fbiInit2 =        0x218/4;   // RW     -- Voodoo/Voodoo2 only
	static constexpr u32 reg_fbiInit3 =        0x21c/4;   // RW     -- Voodoo/Voodoo2 only
	static constexpr u32 reg_hSync =           0x220/4;   //  W     -- Voodoo/Voodoo2 only
	static constexpr u32 reg_vSync =           0x224/4;   //  W     -- Voodoo/Voodoo2 only
	static constexpr u32 reg_clutData =        0x228/4;   //  W  F  -- Voodoo/Voodoo2 only
	static constexpr u32 reg_dacData =         0x22c/4;   //  W     -- Voodoo/Voodoo2 only
	static constexpr u32 reg_maxRgbDelta =     0x230/4;   //  W     -- Voodoo/Voodoo2 only
	static constexpr u32 reg_hBorder =         0x234/4;   //  W     -- Voodoo2 only
	static constexpr u32 reg_vBorder =         0x238/4;   //  W     -- Voodoo2 only
	static constexpr u32 reg_borderColor =     0x23c/4;   //  W     -- Voodoo2 only

	// 0x240
	static constexpr u32 reg_hvRetrace =       0x240/4;   // R      -- Voodoo2 only
	static constexpr u32 reg_fbiInit5 =        0x244/4;   // RW     -- Voodoo2 only
	static constexpr u32 reg_fbiInit6 =        0x248/4;   // RW     -- Voodoo2 only
	static constexpr u32 reg_fbiInit7 =        0x24c/4;   // RW     -- Voodoo2 only
	static constexpr u32 reg_swapPending =     0x24c/4;   //  W     -- Banshee only
	static constexpr u32 reg_leftOverlayBuf =  0x250/4;   //  W     -- Banshee only
	static constexpr u32 reg_rightOverlayBuf = 0x254/4;   //  W     -- Banshee only
	static constexpr u32 reg_fbiSwapHistory =  0x258/4;   // R      -- Voodoo2/Banshee only
	static constexpr u32 reg_fbiTrianglesOut = 0x25c/4;   // R      -- Voodoo2/Banshee only
	static constexpr u32 reg_sSetupMode =      0x260/4;   //  W PF  -- Voodoo2/Banshee only
	static constexpr u32 reg_sVx =             0x264/4;   //  W PF  -- Voodoo2/Banshee only
	static constexpr u32 reg_sVy =             0x268/4;   //  W PF  -- Voodoo2/Banshee only
	static constexpr u32 reg_sARGB =           0x26c/4;   //  W PF  -- Voodoo2/Banshee only
	static constexpr u32 reg_sRed =            0x270/4;   //  W PF  -- Voodoo2/Banshee only
	static constexpr u32 reg_sGreen =          0x274/4;   //  W PF  -- Voodoo2/Banshee only
	static constexpr u32 reg_sBlue =           0x278/4;   //  W PF  -- Voodoo2/Banshee only
	static constexpr u32 reg_sAlpha =          0x27c/4;   //  W PF  -- Voodoo2/Banshee only

	// 0x280
	static constexpr u32 reg_sVz =             0x280/4;   //  W PF  -- Voodoo2/Banshee only
	static constexpr u32 reg_sWb =             0x284/4;   //  W PF  -- Voodoo2/Banshee only
	static constexpr u32 reg_sWtmu0 =          0x288/4;   //  W PF  -- Voodoo2/Banshee only
	static constexpr u32 reg_sS_W0 =           0x28c/4;   //  W PF  -- Voodoo2/Banshee only
	static constexpr u32 reg_sT_W0 =           0x290/4;   //  W PF  -- Voodoo2/Banshee only
	static constexpr u32 reg_sWtmu1 =          0x294/4;   //  W PF  -- Voodoo2/Banshee only
	static constexpr u32 reg_sS_Wtmu1 =        0x298/4;   //  W PF  -- Voodoo2/Banshee only
	static constexpr u32 reg_sT_Wtmu1 =        0x29c/4;   //  W PF  -- Voodoo2/Banshee only
	static constexpr u32 reg_sDrawTriCMD =     0x2a0/4;   //  W PF  -- Voodoo2/Banshee only
	static constexpr u32 reg_sBeginTriCMD =    0x2a4/4;   //  W PF  -- Voodoo2/Banshee only

	// 0x2c0
	static constexpr u32 reg_bltSrcBaseAddr =  0x2c0/4;   // RW PF  -- Voodoo2 only
	static constexpr u32 reg_bltDstBaseAddr =  0x2c4/4;   // RW PF  -- Voodoo2 only
	static constexpr u32 reg_bltXYStrides =    0x2c8/4;   // RW PF  -- Voodoo2 only
	static constexpr u32 reg_bltSrcChromaRange = 0x2cc/4; // RW PF  -- Voodoo2 only
	static constexpr u32 reg_bltDstChromaRange = 0x2d0/4; // RW PF  -- Voodoo2 only
	static constexpr u32 reg_bltClipX =        0x2d4/4;   // RW PF  -- Voodoo2 only
	static constexpr u32 reg_bltClipY =        0x2d8/4;   // RW PF  -- Voodoo2 only
	static constexpr u32 reg_bltSrcXY =        0x2e0/4;   // RW PF  -- Voodoo2 only
	static constexpr u32 reg_bltDstXY =        0x2e4/4;   // RW PF  -- Voodoo2 only
	static constexpr u32 reg_bltSize =         0x2e8/4;   // RW PF  -- Voodoo2 only
	static constexpr u32 reg_bltRop =          0x2ec/4;   // RW PF  -- Voodoo2 only
	static constexpr u32 reg_bltColor =        0x2f0/4;   // RW PF  -- Voodoo2 only
	static constexpr u32 reg_bltCommand =      0x2f8/4;   // RW PF  -- Voodoo2 only
	static constexpr u32 reg_bltData =         0x2fc/4;   //  W PF  -- Voodoo2 only

	// 0x300
	static constexpr u32 reg_textureMode =     0x300/4;   //  W PF
	static constexpr u32 reg_tLOD =            0x304/4;   //  W PF
	static constexpr u32 reg_tDetail =         0x308/4;   //  W PF
	static constexpr u32 reg_texBaseAddr =     0x30c/4;   //  W PF
	static constexpr u32 reg_texBaseAddr_1 =   0x310/4;   //  W PF
	static constexpr u32 reg_texBaseAddr_2 =   0x314/4;   //  W PF
	static constexpr u32 reg_texBaseAddr_3_8 = 0x318/4;   //  W PF
	static constexpr u32 reg_trexInit0 =       0x31c/4;   //  W  F  -- Voodoo/Voodoo2 only
	static constexpr u32 reg_trexInit1 =       0x320/4;   //  W  F
	static constexpr u32 reg_nccTable =        0x324/4;   //  W  F

	// constructor
	voodoo_regs() :
		m_starts(0), m_startt(0),
		m_startw(0),
		m_dsdx(0), m_dtdx(0),
		m_dwdx(0),
		m_dsdy(0), m_dtdy(0),
		m_dwdy(0)
	{
		for (int index = 0; index < std::size(m_regs); index++)
			m_regs[index].u = 0;
	}

	// state saving
	void register_save(save_proxy &save);

	// register aliasing
	static u32 alias(u32 regnum) { return (regnum < 0x40) ? s_alias_map[regnum] : regnum; }

	// simple readers
	u32 read(u32 index) const { return m_regs[index].u; }
	float read_float(u32 index) const { return m_regs[index].f; }
	s32 ax() const { return s16(m_regs[reg_vertexAx].u); }
	s32 ay() const { return s16(m_regs[reg_vertexAy].u); }
	s32 bx() const { return s16(m_regs[reg_vertexBx].u); }
	s32 by() const { return s16(m_regs[reg_vertexBy].u); }
	s32 cx() const { return s16(m_regs[reg_vertexCx].u); }
	s32 cy() const { return s16(m_regs[reg_vertexCy].u); }
	s32 start_r() const { return s24(m_regs[reg_startR].u); }
	s32 start_g() const { return s24(m_regs[reg_startG].u); }
	s32 start_b() const { return s24(m_regs[reg_startB].u); }
	s32 start_a() const { return s24(m_regs[reg_startA].u); }
	s32 start_z() const { return m_regs[reg_startZ].u; }
	s64 start_s() const { return m_starts; }
	s64 start_t() const { return m_startt; }
	s64 start_w() const { return m_startw; }
	s32 dr_dx() const { return s24(m_regs[reg_dRdX].u); }
	s32 dg_dx() const { return s24(m_regs[reg_dGdX].u); }
	s32 db_dx() const { return s24(m_regs[reg_dBdX].u); }
	s32 da_dx() const { return s24(m_regs[reg_dAdX].u); }
	s32 dz_dx() const { return m_regs[reg_dZdX].u; }
	s64 ds_dx() const { return m_dsdx; }
	s64 dt_dx() const { return m_dtdx; }
	s64 dw_dx() const { return m_dwdx; }
	s32 dr_dy() const { return s24(m_regs[reg_dRdY].u); }
	s32 dg_dy() const { return s24(m_regs[reg_dGdY].u); }
	s32 db_dy() const { return s24(m_regs[reg_dBdY].u); }
	s32 da_dy() const { return s24(m_regs[reg_dAdY].u); }
	s32 dz_dy() const { return m_regs[reg_dZdY].u; }
	s64 ds_dy() const { return m_dsdy; }
	s64 dt_dy() const { return m_dtdy; }
	s64 dw_dy() const { return m_dwdy; }
	reg_fbz_colorpath fbz_colorpath() const { return reg_fbz_colorpath(m_regs[reg_fbzColorPath].u); }
	reg_fog_mode fog_mode() const { return reg_fog_mode(m_regs[reg_fogMode].u); }
	reg_alpha_mode alpha_mode() const { return reg_alpha_mode(m_regs[reg_alphaMode].u); }
	reg_fbz_mode fbz_mode() const { return reg_fbz_mode(m_regs[reg_fbzMode].u); }
	reg_lfb_mode lfb_mode() const { return reg_lfb_mode(m_regs[reg_lfbMode].u); }
	reg_color fog_color() const { return m_regs[reg_fogColor].u; }
	u32 za_color() const { return m_regs[reg_zaColor].u; }
	reg_color chroma_key() const { return reg_color(m_regs[reg_chromaKey].u); }
	reg_color color0() const { return reg_color(m_regs[reg_color0].u); }
	reg_color color1() const { return reg_color(m_regs[reg_color1].u); }
	reg_chroma_range chroma_range() const { return m_regs[reg_chromaRange].u; }
	u32 stipple() const { return m_regs[reg_stipple].u; }
	reg_fbi_init0 fbi_init0() const { return reg_fbi_init0(m_regs[reg_fbiInit0].u); }
	reg_fbi_init1 fbi_init1() const { return reg_fbi_init1(m_regs[reg_fbiInit1].u); }
	reg_fbi_init2 fbi_init2() const { return reg_fbi_init2(m_regs[reg_fbiInit2].u); }
	reg_fbi_init3 fbi_init3() const { return reg_fbi_init3(m_regs[reg_fbiInit3].u); }
	reg_fbi_init4 fbi_init4() const { return reg_fbi_init4(m_regs[reg_fbiInit4].u); }
	reg_texture_mode texture_mode() const { return reg_texture_mode(m_regs[reg_textureMode].u); }
	reg_texture_lod texture_lod() const { return reg_texture_lod(m_regs[reg_tLOD].u); }
	reg_texture_detail texture_detail() const { return reg_texture_detail(m_regs[reg_tDetail].u); }
	u32 texture_baseaddr() const { return m_regs[reg_texBaseAddr].u; }
	u32 texture_baseaddr_1() const { return m_regs[reg_texBaseAddr_1].u; }
	u32 texture_baseaddr_2() const { return m_regs[reg_texBaseAddr_2].u; }
	u32 texture_baseaddr_3_8() const { return m_regs[reg_texBaseAddr_3_8].u; }
	reg_clip_minmax clip_left_right() const { return reg_clip_minmax(m_regs[reg_clipLeftRight].u); }
	reg_clip_minmax clip_lowy_highy() const { return reg_clip_minmax(m_regs[reg_clipLowYHighY].u); }
	u32 swap_history() const { return m_regs[reg_fbiSwapHistory].u; }
	template<bool Rev1> reg_hsync<Rev1> hsync() const { return reg_hsync<Rev1>(m_regs[reg_hSync].u); }
	template<bool Rev1> reg_vsync<Rev1> vsync() const { return reg_vsync<Rev1>(m_regs[reg_vSync].u); }
	template<bool Rev1> reg_video_dimensions<Rev1> video_dimensions() const { return reg_video_dimensions<Rev1>(m_regs[reg_videoDimensions].u); }
	template<bool Rev1> reg_back_porch<Rev1> back_porch() const { return reg_back_porch<Rev1>(m_regs[reg_backPorch].u); }

	// voodoo-2-specific reads
	reg_intr_ctrl intr_ctrl() const { return reg_intr_ctrl(m_regs[reg_intrCtrl].u); }
	reg_fbi_init5 fbi_init5() const { return reg_fbi_init5(m_regs[reg_fbiInit5].u); }
	reg_fbi_init6 fbi_init6() const { return reg_fbi_init6(m_regs[reg_fbiInit6].u); }
	reg_fbi_init7 fbi_init7() const { return reg_fbi_init7(m_regs[reg_fbiInit7].u); }
	reg_setup_mode setup_mode() const { return reg_setup_mode(m_regs[reg_sSetupMode].u); }

	// special case for the TMU configuration register, which is otherwise undocumented
	u32 trexinit_send_tmu_config() const { return (m_regs[reg_trexInit1].u >> 18) & 1; }

	// easier clip accessors
	s32 clip_left() const { return clip_left_right().min(); }
	s32 clip_right() const { return clip_left_right().max(); }
	s32 clip_top() const { return clip_lowy_highy().min(); }
	s32 clip_bottom() const { return clip_lowy_highy().max(); }

	// write access
	void write(u32 index, u32 data) { m_regs[index].u = data; }
	void add(u32 index, u32 data) { m_regs[index].u += data; }
	void clear_set(u32 index, u32 clear, u32 set) { m_regs[index].u &= ~clear; m_regs[index].u |= set; }
	void write_float(u32 index, float data) { m_regs[index].f = data; }
	void write_start_s(s64 data) { m_starts = data; }
	void write_start_t(s64 data) { m_startt = data; }
	void write_start_w(s64 data) { m_startw = data; }
	void write_ds_dx(s64 data) { m_dsdx = data; }
	void write_dt_dx(s64 data) { m_dtdx = data; }
	void write_dw_dx(s64 data) { m_dwdx = data; }
	void write_ds_dy(s64 data) { m_dsdy = data; }
	void write_dt_dy(s64 data) { m_dtdy = data; }
	void write_dw_dy(s64 data) { m_dwdy = data; }

	// special swap history helper
	void update_swap_history(u32 count) { m_regs[reg_fbiSwapHistory].u = (m_regs[reg_fbiSwapHistory].u << 4) | count; }

	// access to a chunk of registers
	u32 *subset(u32 index) { return reinterpret_cast<u32 *>(&m_regs[index]); }

private:
	union register_data
	{
		u32 u;
		float f;
	};
	register_data m_regs[0x100];
	s64 m_starts, m_startt;         // starting S,T (14.18)
	s64 m_startw;                   // starting W (2.30) -> 16.48
	s64 m_dsdx, m_dtdx;             // delta S,T per X
	s64 m_dwdx;                     // delta W per X
	s64 m_dsdy, m_dtdy;             // delta S,T per Y
	s64 m_dwdy;                     // delta W per Y
	static u8 const s_alias_map[0x40];
};



//**************************************************************************
//  BANSHEE-SPECIFIC REGISTERS
//**************************************************************************

// ======================> banshee_2d_regs

class banshee_2d_regs
{
public:
	static constexpr u32 clip0Min =       0x008/4;
	static constexpr u32 clip0Max =       0x00c/4;
	static constexpr u32 dstBaseAddr =    0x010/4;
	static constexpr u32 dstFormat =      0x014/4;
	static constexpr u32 srcColorkeyMin = 0x018/4;
	static constexpr u32 srcColorkeyMax = 0x01c/4;
	static constexpr u32 dstColorkeyMin = 0x020/4;
	static constexpr u32 dstColorkeyMax = 0x024/4;
	static constexpr u32 bresError0 =     0x028/4;
	static constexpr u32 bresError1 =     0x02c/4;
	static constexpr u32 rop =            0x030/4;
	static constexpr u32 srcBaseAddr =    0x034/4;
	static constexpr u32 commandExtra =   0x038/4;
	static constexpr u32 lineStipple =    0x03c/4;
	static constexpr u32 lineStyle =      0x040/4;
	static constexpr u32 pattern0Alias =  0x044/4;
	static constexpr u32 pattern1Alias =  0x048/4;
	static constexpr u32 clip1Min =       0x04c/4;
	static constexpr u32 clip1Max =       0x050/4;
	static constexpr u32 srcFormat =      0x054/4;
	static constexpr u32 srcSize =        0x058/4;
	static constexpr u32 srcXY =          0x05c/4;
	static constexpr u32 colorBack =      0x060/4;
	static constexpr u32 colorFore =      0x064/4;
	static constexpr u32 dstSize =        0x068/4;
	static constexpr u32 dstXY =          0x06c/4;
	static constexpr u32 command =        0x070/4;

	// constructor
	banshee_2d_regs() { reset(); }

	// state saving
	void register_save(save_proxy &save);

	// reset
	void reset() { std::fill_n(&m_regs[0], std::size(m_regs), 0); }

	// getters
	char const *name(u32 index) const
	{
		if (index < 0x20)
			return s_names[index & 0x7f];
		if (index < 0x40)
			return "launch";
		return "pattern";
	}

	// simple readers
	u32 read(u32 index) const { return m_regs[index & 0x7f]; }

	// write access
	u32 write(u32 index, u32 data, u32 mem_mask = 0xffffffff) { return COMBINE_DATA(&m_regs[index & 0x7f]); }

	// special swap history helper
private:
	u32 m_regs[0x80];
	static char const * const s_names[0x20];
};


// ======================> banshee_io_regs

class banshee_io_regs
{
public:
	static constexpr u32 status =                       0x000/4;
	static constexpr u32 pciInit0 =                     0x004/4;
	static constexpr u32 sipMonitor =                   0x008/4;
	static constexpr u32 lfbMemoryConfig =              0x00c/4;
	static constexpr u32 miscInit0 =                    0x010/4;
	static constexpr u32 miscInit1 =                    0x014/4;
	static constexpr u32 dramInit0 =                    0x018/4;
	static constexpr u32 dramInit1 =                    0x01c/4;
	static constexpr u32 agpInit =                      0x020/4;
	static constexpr u32 tmuGbeInit =                   0x024/4;
	static constexpr u32 vgaInit0 =                     0x028/4;
	static constexpr u32 vgaInit1 =                     0x02c/4;
	static constexpr u32 dramCommand =                  0x030/4;
	static constexpr u32 dramData =                     0x034/4;
	static constexpr u32 pllCtrl0 =                     0x040/4;
	static constexpr u32 pllCtrl1 =                     0x044/4;
	static constexpr u32 pllCtrl2 =                     0x048/4;
	static constexpr u32 dacMode =                      0x04c/4;
	static constexpr u32 dacAddr =                      0x050/4;
	static constexpr u32 dacData =                      0x054/4;
	static constexpr u32 rgbMaxDelta =                  0x058/4;
	static constexpr u32 vidProcCfg =                   0x05c/4;
	static constexpr u32 hwCurPatAddr =                 0x060/4;
	static constexpr u32 hwCurLoc =                     0x064/4;
	static constexpr u32 hwCurC0 =                      0x068/4;
	static constexpr u32 hwCurC1 =                      0x06c/4;
	static constexpr u32 vidInFormat =                  0x070/4;
	static constexpr u32 vidInStatus =                  0x074/4;
	static constexpr u32 vidSerialParallelPort =        0x078/4;
	static constexpr u32 vidInXDecimDeltas =            0x07c/4;
	static constexpr u32 vidInDecimInitErrs =           0x080/4;
	static constexpr u32 vidInYDecimDeltas =            0x084/4;
	static constexpr u32 vidPixelBufThold =             0x088/4;
	static constexpr u32 vidChromaMin =                 0x08c/4;
	static constexpr u32 vidChromaMax =                 0x090/4;
	static constexpr u32 vidCurrentLine =               0x094/4;
	static constexpr u32 vidScreenSize =                0x098/4;
	static constexpr u32 vidOverlayStartCoords =        0x09c/4;
	static constexpr u32 vidOverlayEndScreenCoord =     0x0a0/4;
	static constexpr u32 vidOverlayDudx =               0x0a4/4;
	static constexpr u32 vidOverlayDudxOffsetSrcWidth = 0x0a8/4;
	static constexpr u32 vidOverlayDvdy =               0x0ac/4;
	static constexpr u32 vgab0 =                        0x0b0/4;
	static constexpr u32 vgab4 =                        0x0b4/4;
	static constexpr u32 vgab8 =                        0x0b8/4;
	static constexpr u32 vgabc =                        0x0bc/4;
	static constexpr u32 vgac0 =                        0x0c0/4;
	static constexpr u32 vgac4 =                        0x0c4/4;
	static constexpr u32 vgac8 =                        0x0c8/4;
	static constexpr u32 vgacc =                        0x0cc/4;
	static constexpr u32 vgad0 =                        0x0d0/4;
	static constexpr u32 vgad4 =                        0x0d4/4;
	static constexpr u32 vgad8 =                        0x0d8/4;
	static constexpr u32 vgadc =                        0x0dc/4;
	static constexpr u32 vidOverlayDvdyOffset =         0x0e0/4;
	static constexpr u32 vidDesktopStartAddr =          0x0e4/4;
	static constexpr u32 vidDesktopOverlayStride =      0x0e8/4;
	static constexpr u32 vidInAddr0 =                   0x0ec/4;
	static constexpr u32 vidInAddr1 =                   0x0f0/4;
	static constexpr u32 vidInAddr2 =                   0x0f4/4;
	static constexpr u32 vidInStride =                  0x0f8/4;
	static constexpr u32 vidCurrOverlayStartAddr =      0x0fc/4;

	// constructor
	banshee_io_regs() { reset(); }

	// state saving
	void register_save(save_proxy &save);

	// reset
	void reset() { std::fill_n(&m_regs[0], std::size(m_regs), 0); }

	// getters
	char const *name(u32 index) const { return s_names[index & 0x3f]; }

	// simple readers
	u32 read(u32 index) const { return m_regs[index & 0x3f]; }

	// write access
	u32 write(u32 index, u32 data, u32 mem_mask = 0xffffffff) { return COMBINE_DATA(&m_regs[index & 0x3f]); }

	// special swap history helper
private:
	u32 m_regs[0x40];
	static char const * const s_names[0x40];
};


// ======================> banshee_cmd_agp_regs

class banshee_cmd_agp_regs
{
public:
	static constexpr u32 agpReqSize =         0x000/4;
	static constexpr u32 agpHostAddressLow =  0x004/4;
	static constexpr u32 agpHostAddressHigh = 0x008/4;
	static constexpr u32 agpGraphicsAddress = 0x00c/4;
	static constexpr u32 agpGraphicsStride =  0x010/4;
	static constexpr u32 agpMoveCMD =         0x014/4;
	static constexpr u32 cmdBaseAddr0 =       0x020/4;
	static constexpr u32 cmdBaseSize0 =       0x024/4;
	static constexpr u32 cmdBump0 =           0x028/4;
	static constexpr u32 cmdRdPtrL0 =         0x02c/4;
	static constexpr u32 cmdRdPtrH0 =         0x030/4;
	static constexpr u32 cmdAMin0 =           0x034/4;
	static constexpr u32 cmdAMax0 =           0x03c/4;
	static constexpr u32 cmdFifoDepth0 =      0x044/4;
	static constexpr u32 cmdHoleCnt0 =        0x048/4;
	static constexpr u32 cmdBaseAddr1 =       0x050/4;
	static constexpr u32 cmdBaseSize1 =       0x054/4;
	static constexpr u32 cmdBump1 =           0x058/4;
	static constexpr u32 cmdRdPtrL1 =         0x05c/4;
	static constexpr u32 cmdRdPtrH1 =         0x060/4;
	static constexpr u32 cmdAMin1 =           0x064/4;
	static constexpr u32 cmdAMax1 =           0x06c/4;
	static constexpr u32 cmdFifoDepth1 =      0x074/4;
	static constexpr u32 cmdHoleCnt1 =        0x078/4;
	static constexpr u32 cmdFifoThresh =      0x080/4;
	static constexpr u32 cmdHoleInt =         0x084/4;
	static constexpr u32 yuvBaseAddress =     0x100/4;
	static constexpr u32 yuvStride =          0x104/4;
	static constexpr u32 crc1 =               0x120/4;
	static constexpr u32 crc2 =               0x130/4;

	// constructor
	banshee_cmd_agp_regs() { reset(); }

	// state saving
	void register_save(save_proxy &save);

	// reset
	void reset() { std::fill_n(&m_regs[0], std::size(m_regs), 0); }

	// getters
	char const *name(u32 index) const { return s_names[index & 0x7f]; }

	// simple readers
	u32 read(u32 index) const { return m_regs[index & 0x7f]; }

	// write access
	u32 write(u32 index, u32 data, u32 mem_mask = 0xffffffff) { return COMBINE_DATA(&m_regs[index & 0x7f]); }

private:
	u32 m_regs[0x80];
	static char const * const s_names[0x80];
};


// ======================> banshee_cmd_agp_regs

class banshee_vga_regs
{
public:
	static constexpr u32 attributeData =       0x3c0 & 0x1f;
	static constexpr u32 attributeIndex =      0x3c1 & 0x1f;
	static constexpr u32 inputStatus0 =        0x3c2 & 0x1f;
	static constexpr u32 miscOutputW =         0x3c2 & 0x1f;
	static constexpr u32 motherboardEnable =   0x3c3 & 0x1f;
	static constexpr u32 sequencerIndex =      0x3c4 & 0x1f;
	static constexpr u32 sequencerData =       0x3c5 & 0x1f;
	static constexpr u32 pixelMask =           0x3c6 & 0x1f;
	static constexpr u32 readIndex =           0x3c7 & 0x1f;
	static constexpr u32 readStatus =          0x3c7 & 0x1f;
	static constexpr u32 writeIndex =          0x3c8 & 0x1f;
	static constexpr u32 data =                0x3c9 & 0x1f;
	static constexpr u32 featureControlR =     0x3ca & 0x1f;
	static constexpr u32 miscOutputR =         0x3cc & 0x1f;
	static constexpr u32 gfxControllerIndex =  0x3ce & 0x1f;
	static constexpr u32 gfxControllerData =   0x3cf & 0x1f;
	static constexpr u32 crtcIndex =           0x3d4 & 0x1f;
	static constexpr u32 crtcData =            0x3d5 & 0x1f;
	static constexpr u32 inputStatus1 =        0x3da & 0x1f;
	static constexpr u32 featureControlW =     0x3da & 0x1f;

	// constructor
	banshee_vga_regs() { reset(); }

	// state saving
	void register_save(save_proxy &save);

	// reset
	void reset()
	{
		std::fill_n(&m_regs[0], std::size(m_regs), 0);
		std::fill_n(&m_crtc[0], std::size(m_crtc), 0);
		std::fill_n(&m_seq[0], std::size(m_seq), 0);
		std::fill_n(&m_gc[0], std::size(m_gc), 0);
		std::fill_n(&m_attr[0], std::size(m_attr), 0);
		m_attr_flip_flop = 0;
	}

	// simple readers
	u8 read(u32 index) const { return m_regs[index & 0x1f]; }
	u8 read_crtc(u32 index) const { return (index < std::size(m_crtc)) ? m_crtc[index] : 0xff; }
	u8 read_seq(u32 index) const { return (index < std::size(m_seq)) ? m_seq[index] : 0xff; }
	u8 read_gc(u32 index) const { return (index < std::size(m_gc)) ? m_gc[index] : 0xff; }
	u8 read_attr(u32 index) const { return (index < std::size(m_attr)) ? m_attr[index] : 0xff; }

	// write access
	void write(u32 index, u8 data) { m_regs[index & 0x1f] = data; }
	void write_crtc(u32 index, u8 data) { if (index < std::size(m_crtc)) m_crtc[index] = data; }
	void write_seq(u32 index, u8 data) { if (index < std::size(m_seq)) m_seq[index] = data; }
	void write_gc(u32 index, u8 data) { if (index < std::size(m_gc)) m_gc[index] = data; }
	void write_attr(u32 index, u8 data) { if (index < std::size(m_attr)) m_attr[index] = data; }

	// basic accessors
	u8 attribute_index() const { return m_regs[attributeIndex & 0x1f]; }
	u8 sequencer_index() const { return m_regs[sequencerIndex & 0x1f]; }
	u8 gfx_controller_index() const { return m_regs[gfxControllerIndex & 0x1f]; }
	u8 crtc_index() const { return m_regs[crtcIndex & 0x1f]; }

	// flip flop
	void clear_flip_flop() { m_attr_flip_flop = 0; }
	u8 toggle_flip_flop() { u8 old = m_attr_flip_flop; m_attr_flip_flop ^= 1; return old; }

private:
	u8 m_regs[0x20];        // core VGA registers
	u8 m_crtc[0x27];        // CRTC registers
	u8 m_seq[0x05];         // sequencer registers
	u8 m_gc[0x05];          // graphics controller registers
	u8 m_attr[0x15];        // attribute registers
	u8 m_attr_flip_flop;    // attribute flip-flop
};

}

#endif // MAME_VIDEO_VOODOO_REGS_H
