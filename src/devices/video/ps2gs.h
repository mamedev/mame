// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony Playstation 2 Graphics Synthesizer device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_VIDEO_PS2GS_H
#define MAME_VIDEO_PS2GS_H

#pragma once

class ps2_gs_device;

#include "ps2gif.h"
#include "machine/ps2intc.h"
#include "cpu/mips/ps2vu.h"

class ps2_gs_device : public device_t
{
public:
	template <typename T, typename U>

	ps2_gs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&intc_tag, U &&vu1_tag)
		: ps2_gs_device(mconfig, tag, owner, clock)
	{
		m_intc.set_tag(std::forward<T>(intc_tag));
		m_vu1.set_tag(std::forward<U>(vu1_tag));
	}

	ps2_gs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~ps2_gs_device() override;

	uint64_t priv_regs0_r(offs_t offset);
	void priv_regs0_w(offs_t offset, uint64_t data);
	uint64_t priv_regs1_r(offs_t offset);
	void priv_regs1_w(offs_t offset, uint64_t data);

	void regs_w(offs_t offset, uint64_t data);

	uint32_t gif_r(offs_t offset);
	void gif_w(offs_t offset, uint32_t data);

	ps2_gif_device* interface();

	void reg_write(const uint8_t reg, const uint64_t value);
	void write_packed(const uint8_t reg, const uint64_t hi, const uint64_t lo);

	void vblank_start();
	void vblank_end();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void copy_dword_from_host(uint64_t data);

	enum : uint64_t
	{
		HOST_TO_LOCAL,
		LOCAL_TO_HOST,
		LOCAL_TO_LOCAL,
		NO_TRANSFER
	};

	enum : uint8_t
	{
		UL_TO_BR,
		BL_TO_UR,
		UR_TO_BL,
		BR_TO_UL
	};

	enum : uint8_t
	{
		PSMCT32     = 0x00,
		PSMCT24     = 0x01,
		PSMCT16     = 0x02,
		PSMCT16S    = 0x0a,
		PSMT8       = 0x13,
		PSMT4       = 0x14,
		PSMT8H      = 0x1b,
		PSMT4HL     = 0x24,
		PSMT4HH     = 0x2c,
		PSMZ32      = 0x30,
		PSMZ24      = 0x31,
		PSMZ16      = 0x32,
		PSMZ16S     = 0x3a
	};

	enum : uint8_t
	{
		ALPHA_FUNC_NEVER,
		ALPHA_FUNC_ALWAYS,
		ALPHA_FUNC_LESS,
		ALPHA_FUNC_LEQUAL,
		ALPHA_FUNC_EQUAL,
		ALPHA_FUNC_GEQUAL,
		ALPHA_FUNC_GREATER,
		ALPHA_FUNC_NOTEQUAL
	};

	enum : uint8_t
	{
		ALPHA_FAIL_KEEP,
		ALPHA_FAIL_UPDATE_FB,
		ALPHA_FAIL_UPDATE_ZB,
		ALPHA_FAIL_UPDATE_RGB
	};

	enum : uint8_t
	{
		DEPTH_FUNC_NEVER,
		DEPTH_FUNC_ALWAYS,
		DEPTH_FUNC_GEQUAL,
		DEPTH_FUNC_GREATER
	};

	enum : uint8_t
	{
		PRIM_TYPE_POINT,
		PRIM_TYPE_LINE,
		PRIM_TYPE_LINE_STRIP,
		PRIM_TYPE_TRI,
		PRIM_TYPE_TRI_STRIP,
		PRIM_TYPE_TRI_FAN,
		PRIM_TYPE_SPRITE,
		PRIM_TYPE_INVALID
	};

	enum : uint32_t
	{
		CSR_SIGNAL     = 0x00000001,
		CSR_FINISH     = 0x00000002,
		CSR_HSINT      = 0x00000004,
		CSR_VSINT      = 0x00000008,
		CSR_EDWINT     = 0x00000010,
		CSR_FLUSH      = 0x00000100,
		CSR_RESET      = 0x00000200,
		CSR_NFIELD     = 0x00001000,
		CSR_FIELD_ODD  = 0x00002000,
		CSR_FIFO_MASK  = 0x0000c000,
		CSR_FIFO_MID   = 0x00000000,
		CSR_FIFO_EMPTY = 0x00004000,
		CSR_FIFO_HI    = 0x00008000,
		CSR_FIFO_INV   = 0x0000c000,
		CSR_REV        = 0x001b0000,
		CSR_ID         = 0x55000000
	};

	struct vertex_t
	{
		uint16_t m_x;
		uint16_t m_y;
		uint32_t m_z;

		uint8_t m_r;
		uint8_t m_g;
		uint8_t m_b;
		uint8_t m_a;
		float m_q;

		uint32_t m_us;
		uint32_t m_vt;

		uint8_t m_fog;
	};

	required_device<ps2_intc_device> m_intc;
	required_device<sonyvu1_device> m_vu1;
	required_device<ps2_gif_device> m_gif;

	std::unique_ptr<uint32_t[]> m_ram;
	std::unique_ptr<vertex_t[]> m_vertices;

	struct context_t
	{
		uint64_t m_xyoffset; // 0x18, 0x19
		uint32_t m_offset_x;
		uint32_t m_offset_y;

		uint64_t m_scissor; // 0x40, 0x41
		uint16_t m_scissor_x0;
		uint16_t m_scissor_x1;
		uint16_t m_scissor_y0;
		uint16_t m_scissor_y1;

		uint64_t m_test; // 0x47, 0x48
		bool m_alpha_test;
		uint8_t m_alpha_func;
		uint8_t m_alpha_ref;
		uint8_t m_alpha_fail;
		bool m_dstalpha_test;
		bool m_dstalpha_pass1;
		bool m_depth_test;
		uint8_t m_depth_func;

		uint64_t m_frame; // 0x4c, 0x4d
		uint32_t m_fb_base;
		uint32_t m_fb_width;
		uint8_t m_fb_format;
		uint32_t m_fb_mask;

		uint64_t m_zbuf; // 0x4e, 0x4f
		uint32_t m_z_base;
		uint8_t m_z_format;
		bool m_z_mask;
	};

	context_t m_context[2];

	uint64_t m_prim; // 0x00
	uint8_t m_prim_type;
	bool m_gouraud_enable;
	bool m_texture_enable;
	bool m_fog_enable;
	bool m_blend_enable;
	bool m_aa_enable;
	bool m_no_perspective;
	uint8_t m_curr_context;
	bool m_fix_fragments;

	uint64_t m_rgbaq; // 0x01
	uint8_t m_vc_r;
	uint8_t m_vc_g;
	uint8_t m_vc_b;
	uint8_t m_vc_a;
	float m_q;

	uint64_t m_prmodecont; // 0x1a
	bool m_use_prim_for_attrs;

	uint64_t m_dthe; // 0x45
	bool m_dither;

	uint64_t m_colclamp; // 0x46
	bool m_clamp_color;

	uint64_t m_bitbltbuf; // 0x50
	uint32_t m_src_buf_base;
	uint32_t m_src_buf_width;
	uint8_t m_src_buf_fmt;
	uint32_t m_dst_buf_base;
	uint32_t m_dst_buf_width;
	uint8_t m_dst_buf_fmt;

	uint64_t m_trx_pos; // 0x51
	uint32_t m_src_ul_x;
	uint32_t m_src_ul_y;
	uint32_t m_dst_ul_x;
	uint32_t m_dst_ul_y;
	uint8_t  m_copy_dir;

	uint64_t m_trx_reg; // 0x52
	uint32_t m_trx_width;
	uint32_t m_trx_height;

	uint64_t m_trx_dir; // 0x53

	// Privileged regs
	uint64_t m_base_regs[15];

	uint64_t m_pmode; // Privileged 0x00
	bool m_read_circuit_enable[2];
	bool m_use_fixed_alpha;
	uint8_t m_alpha_out_select;
	bool m_blend_to_background;
	uint8_t m_fixed_alpha;

	uint64_t m_smode2; // Privileged 0x02
	bool m_interlace;
	bool m_frame_interlace;
	uint8_t m_dpms_mode;

	uint64_t m_dispfb[2]; // Privileged 0x07, 0x09
	uint32_t m_dispfb_base[2];
	uint32_t m_dispfb_width[2];
	uint8_t m_dispfb_format[2];
	uint32_t m_dispfb_x[2];
	uint32_t m_dispfb_y[2];

	uint64_t m_display[2]; // Privileged 0x08, 0x0a
	uint32_t m_display_xpos[2];
	uint32_t m_display_ypos[2];
	uint8_t m_magh[2];
	uint8_t m_magv[2];
	uint32_t m_display_width[2];
	uint32_t m_display_height[2];

	uint64_t m_bgcolor; // Privileged 0x0e
	uint8_t m_bg_r;
	uint8_t m_bg_g;
	uint8_t m_bg_b;

	uint64_t m_csr;
	uint64_t m_imr;
	uint64_t m_busdir;
	uint64_t m_sig_label_id;

	uint32_t m_vertex_count;
	uint32_t m_kick_count;

	uint8_t m_curr_field;

	static size_t const FORMAT_PIXEL_WIDTHS[0x40];
	static char const *const FORMAT_NAMES[0x40];
	static uint32_t const KICK_COUNTS[8];
};

DECLARE_DEVICE_TYPE(SONYPS2_GS, ps2_gs_device)

#endif // MAME_VIDEO_PS2GS_H
