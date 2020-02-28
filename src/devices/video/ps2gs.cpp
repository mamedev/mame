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

#include "emu.h"
#include "ps2gs.h"

DEFINE_DEVICE_TYPE(SONYPS2_GS, ps2_gs_device, "ps2gs", "Playstation 2 GS")

/*static*/ size_t const ps2_gs_device::FORMAT_PIXEL_WIDTHS[] = {
	32, 24, 16, 0,  0,  0,  0,  0,
	0,  0,  16, 0,  0,  0,  0,  0,
	0,  0,  0,  8,  4,  0,  0,  0,
	0,  0,  0,  8,  0,  0,  0,  0,
	0,  0,  0,  0,  4,  0,  0,  0,
	0,  0,  0,  0,  4,  0,  0,  0,
	32, 24, 16, 0,  0,  0,  0,  0,
	0,  0,  16, 0,  0,  0,  0,  0
};

/*static*/ char const *const ps2_gs_device::FORMAT_NAMES[] = {
	"PSMCT32",  "PSMCT24",  "PSMCT16",  "Unknown",  "Unknown",  "Unknown",  "Unknown",  "Unknown",
	"Unknown",  "Unknown",  "PCMCT16S", "Unknown",  "Unknown",  "Unknown",  "Unknown",  "Unknown",
	"Unknown",  "Unknown",  "Unknown",  "PSMT8",    "PSMT4",    "Unknown",  "Unknown",  "Unknown",
	"Unknown",  "Unknown",  "Unknown",  "PSMT8H",   "Unknown",  "Unknown",  "Unknown",  "Unknown",
	"Unknown",  "Unknown",  "Unknown",  "Unknown",  "PSMT4HL",  "Unknown",  "Unknown",  "Unknown",
	"Unknown",  "Unknown",  "Unknown",  "Unknown",  "PSMT4HH",  "Unknown",  "Unknown",  "Unknown",
	"PSMZ32",   "PSMZ24",   "PSMZ16",   "Unknown",  "Unknown",  "Unknown",  "Unknown",  "Unknown",
	"Unknown",  "Unknown",  "PSMZ16S",  "Unknown",  "Unknown",  "Unknown",  "Unknown",  "Unknown"
};

/*static*/ uint32_t const ps2_gs_device::KICK_COUNTS[] = {
	1, 2, 2, 3, 3, 3, 2, 1
};

ps2_gs_device::ps2_gs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYPS2_GS, tag, owner, clock)
	, m_intc(*this, finder_base::DUMMY_TAG)
	, m_vu1(*this, finder_base::DUMMY_TAG)
	, m_gif(*this, "gif")
{
}

ps2_gs_device::~ps2_gs_device()
{
}

void ps2_gs_device::device_add_mconfig(machine_config &config)
{
	SONYPS2_GIF(config, m_gif, clock(), DEVICE_SELF, m_vu1);
}

void ps2_gs_device::device_start()
{
	save_item(NAME(m_bitbltbuf));
	save_item(NAME(m_src_buf_base));
	save_item(NAME(m_src_buf_width));
	save_item(NAME(m_src_buf_fmt));
	save_item(NAME(m_dst_buf_base));
	save_item(NAME(m_dst_buf_width));
	save_item(NAME(m_dst_buf_fmt));

	save_item(NAME(m_trx_pos));
	save_item(NAME(m_src_ul_x));
	save_item(NAME(m_src_ul_y));
	save_item(NAME(m_dst_ul_x));
	save_item(NAME(m_dst_ul_y));
	save_item(NAME(m_copy_dir));

	save_item(NAME(m_trx_reg));
	save_item(NAME(m_trx_width));
	save_item(NAME(m_trx_height));

	save_item(NAME(m_trx_dir));

	save_item(NAME(m_base_regs));

	save_item(NAME(m_pmode));
	save_item(NAME(m_read_circuit_enable));
	save_item(NAME(m_use_fixed_alpha));
	save_item(NAME(m_alpha_out_select));
	save_item(NAME(m_blend_to_background));
	save_item(NAME(m_fixed_alpha));

	save_item(NAME(m_smode2));
	save_item(NAME(m_interlace));
	save_item(NAME(m_frame_interlace));
	save_item(NAME(m_dpms_mode));

	save_item(NAME(m_dispfb));
	save_item(NAME(m_dispfb_base));
	save_item(NAME(m_dispfb_width));
	save_item(NAME(m_dispfb_format));
	save_item(NAME(m_dispfb_x));
	save_item(NAME(m_dispfb_y));

	save_item(NAME(m_display));
	save_item(NAME(m_display_xpos));
	save_item(NAME(m_display_ypos));
	save_item(NAME(m_magh));
	save_item(NAME(m_magv));
	save_item(NAME(m_display_width));
	save_item(NAME(m_display_height));

	save_item(NAME(m_bgcolor));
	save_item(NAME(m_bg_r));
	save_item(NAME(m_bg_g));
	save_item(NAME(m_bg_b));

	save_item(NAME(m_csr));
	save_item(NAME(m_imr));
	save_item(NAME(m_busdir));
	save_item(NAME(m_sig_label_id));

	save_item(NAME(m_vertex_count));
	save_item(NAME(m_kick_count));

	save_item(NAME(m_context[0].m_xyoffset));
	save_item(NAME(m_context[0].m_offset_x));
	save_item(NAME(m_context[0].m_offset_y));

	save_item(NAME(m_context[0].m_scissor));
	save_item(NAME(m_context[0].m_scissor_x0));
	save_item(NAME(m_context[0].m_scissor_x1));
	save_item(NAME(m_context[0].m_scissor_y0));
	save_item(NAME(m_context[0].m_scissor_y1));

	save_item(NAME(m_context[0].m_test));
	save_item(NAME(m_context[0].m_alpha_test));
	save_item(NAME(m_context[0].m_alpha_func));
	save_item(NAME(m_context[0].m_alpha_ref));
	save_item(NAME(m_context[0].m_alpha_fail));
	save_item(NAME(m_context[0].m_dstalpha_test));
	save_item(NAME(m_context[0].m_dstalpha_pass1));
	save_item(NAME(m_context[0].m_depth_test));
	save_item(NAME(m_context[0].m_depth_func));

	save_item(NAME(m_context[0].m_frame));
	save_item(NAME(m_context[0].m_fb_base));
	save_item(NAME(m_context[0].m_fb_format));
	save_item(NAME(m_context[0].m_fb_width));
	save_item(NAME(m_context[0].m_fb_mask));

	save_item(NAME(m_context[0].m_zbuf));
	save_item(NAME(m_context[0].m_z_base));
	save_item(NAME(m_context[0].m_z_format));
	save_item(NAME(m_context[0].m_z_mask));

	save_item(NAME(m_context[1].m_xyoffset));
	save_item(NAME(m_context[1].m_offset_x));
	save_item(NAME(m_context[1].m_offset_y));

	save_item(NAME(m_context[1].m_scissor));
	save_item(NAME(m_context[1].m_scissor_x0));
	save_item(NAME(m_context[1].m_scissor_x1));
	save_item(NAME(m_context[1].m_scissor_y0));
	save_item(NAME(m_context[1].m_scissor_y1));

	save_item(NAME(m_context[1].m_test));
	save_item(NAME(m_context[1].m_alpha_test));
	save_item(NAME(m_context[1].m_alpha_func));
	save_item(NAME(m_context[1].m_alpha_ref));
	save_item(NAME(m_context[1].m_alpha_fail));
	save_item(NAME(m_context[1].m_dstalpha_test));
	save_item(NAME(m_context[1].m_dstalpha_pass1));
	save_item(NAME(m_context[1].m_depth_test));
	save_item(NAME(m_context[1].m_depth_func));

	save_item(NAME(m_context[1].m_frame));
	save_item(NAME(m_context[1].m_fb_base));
	save_item(NAME(m_context[1].m_fb_format));
	save_item(NAME(m_context[1].m_fb_width));
	save_item(NAME(m_context[1].m_fb_mask));

	save_item(NAME(m_context[1].m_zbuf));
	save_item(NAME(m_context[1].m_z_base));
	save_item(NAME(m_context[1].m_z_format));
	save_item(NAME(m_context[1].m_z_mask));

	m_ram = std::make_unique<uint32_t[]>(0x400000/4);
	m_vertices = std::make_unique<vertex_t[]>(0x10000); // Arbitrary count
}

void ps2_gs_device::device_reset()
{
	m_bitbltbuf = 0;
	m_src_buf_base = 0;
	m_src_buf_width = 0;
	m_src_buf_fmt = PSMCT32;
	m_dst_buf_base = 0;
	m_dst_buf_width = 0;
	m_dst_buf_fmt = PSMCT32;

	m_trx_pos = 0;
	m_src_ul_x = 0;
	m_src_ul_y = 0;
	m_dst_ul_x = 0;
	m_dst_ul_y = 0;
	m_copy_dir = 0;

	m_trx_reg = 0;
	m_trx_width = 0;
	m_trx_height = 0;

	m_trx_dir = 0;

	memset(m_base_regs, 0, sizeof(uint64_t) * 15);

	m_pmode = 0;
	memset(m_read_circuit_enable, 0, 2);
	m_use_fixed_alpha = 0;
	m_alpha_out_select = 0;
	m_blend_to_background = 0;
	m_fixed_alpha = 0;

	m_smode2 = 0;
	m_interlace = 0;
	m_frame_interlace = 0;
	m_dpms_mode = 0;

	memset(m_dispfb, 0, sizeof(uint64_t) * 2);
	memset(m_dispfb_base, 0, sizeof(uint32_t) * 2);
	memset(m_dispfb_width, 0, sizeof(uint32_t) * 2);
	memset(m_dispfb_format, 0, sizeof(uint8_t) * 2);
	memset(m_dispfb_x, 0, sizeof(uint32_t) * 2);
	memset(m_dispfb_y, 0, sizeof(uint32_t) * 2);

	memset(m_display, 0, sizeof(uint64_t) * 2);
	memset(m_display_xpos, 0, sizeof(uint32_t) * 2);
	memset(m_display_ypos, 0, sizeof(uint32_t) * 2);
	memset(m_magh, 0, sizeof(uint8_t) * 2);
	memset(m_magv, 0, sizeof(uint8_t) * 2);
	memset(m_display_width, 0, sizeof(uint32_t) * 2);
	memset(m_display_height, 0, sizeof(uint32_t) * 2);

	m_bgcolor = 0;
	m_bg_r = 0;
	m_bg_g = 0;
	m_bg_b = 0;

	m_csr = 0;
	m_imr = 0;
	m_busdir = 0;
	m_sig_label_id = 0;

	memset(m_context, 0, sizeof(context_t) * 2);
	m_vertex_count = 0;
	m_kick_count = 0;
}

READ64_MEMBER(ps2_gs_device::priv_regs0_r)
{
	uint64_t ret = m_base_regs[offset >> 1];
	switch (offset)
	{
		case 0x00:
			ret = m_pmode;
			logerror("%s: regs0_r: PMODE (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret);
			break;

		case 0x04:
			ret = m_smode2;
			logerror("%s: regs0_r: SMODE2 (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret);
			break;

		case 0x0e:
		case 0x12:
			ret = m_dispfb[(offset - 0x0e) / 4];
			logerror("%s: regs0_r: DISPFB2 (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret);
			break;

		case 0x10:
		case 0x14:
			ret = m_display[(offset - 0x10) / 4];
			logerror("%s: regs0_r: DISPLAY2 (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret);
			break;

		case 0x1c:
			ret = m_bgcolor;
			logerror("%s: regs0_r: BGCOLOR (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret);
			break;

		case 0x02: logerror("%s: regs0_r: SMODE1 (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret); break;
		case 0x06: logerror("%s: regs0_r: SRFSH (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret); break;
		case 0x08: logerror("%s: regs0_r: SYNCH1 (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret); break;
		case 0x0a: logerror("%s: regs0_r: SYNCH2 (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret); break;
		case 0x0c: logerror("%s: regs0_r: SYNCV (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret); break;
		case 0x16: logerror("%s: regs0_r: EXTBUF (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret); break;
		case 0x18: logerror("%s: regs0_r: EXTDATA (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret); break;
		case 0x1a: logerror("%s: regs0_r: EXTWRITE (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret); break;
		default:   logerror("%s: regs0_r: Unknown (%08x)\n", machine().describe_context(), 0x12000000 + (offset << 3)); break;
	}
	return ret;
}

WRITE64_MEMBER(ps2_gs_device::priv_regs0_w)
{
	switch (offset)
	{
		case 0x00: // PMODE
			m_pmode = data;
			m_read_circuit_enable[0] = BIT(data, 0);
			m_read_circuit_enable[1] = BIT(data, 1);
			m_use_fixed_alpha = BIT(data, 5);
			m_alpha_out_select = BIT(data, 6);
			m_blend_to_background = BIT(data, 7);
			m_fixed_alpha = (data >> 8) & 0xff;
			logerror("%s: regs0_w: PMODE = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data);
			break;

		case 0x04: // SMODE2
			m_smode2 = data;
			m_interlace = BIT(data, 0);
			m_frame_interlace = BIT(data, 1);
			m_dpms_mode = (data >> 2) & 3;
			logerror("%s: regs0_w: SMODE2 = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data);
			break;

		case 0x0e: // DISPFB1
		case 0x12: // DISPFB2
		{
			const uint8_t index = (offset - 0x0e) / 4;
			m_dispfb[index] = data;
			m_dispfb_base[index] = (data & 0x1ff) << 11;
			m_dispfb_width[index] = (data & 0x7e00) >> 3;
			m_dispfb_format[index] = (data >> 15) & 0x1f;
			m_dispfb_x[index] = (data >> 32) & 0x7ff;
			m_dispfb_y[index] = (data >> 42) & 0x7ff;
			logerror("%s: regs0_w: DISPFB%d = %08x%08x\n", machine().describe_context(), index + 1, (uint32_t)(data >> 32), (uint32_t)data);
			break;
		}

		case 0x10: // DISPLAY1
		case 0x14: // DISPLAY2
		{
			const uint8_t index = (offset - 0x10) / 4;
			m_display[index] = data;
			m_display_xpos[index] = data & 0xfff;
			m_display_ypos[index] = (data >> 12) & 0x7ff;
			m_magh[index] = (data >> 23) & 0xf;
			m_magv[index] = (data >> 27) & 3;
			m_display_width[index] = (data >> 32) & 0xfff;
			m_display_height[index] = (data >> 44) & 0x7ff;
			logerror("%s: regs0_w: DISPLAY%d = %08x%08x\n", machine().describe_context(), index + 1, (uint32_t)(data >> 32), (uint32_t)data);
			break;
		}

		case 0x1c: // BGCOLOR
			m_bgcolor = data;
			m_bg_r = data & 0xff;
			m_bg_g = (data >> 8) & 0xff;
			m_bg_b = (data >> 16) & 0xff;
			logerror("%s: regs0_w: BGCOLOR = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data);
			break;

		case 0x02: logerror("%s: regs0_w: SMODE1 = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data); break;
		case 0x06: logerror("%s: regs0_w: SRFSH = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data); break;
		case 0x08: logerror("%s: regs0_w: SYNCH1 = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data); break;
		case 0x0a: logerror("%s: regs0_w: SYNCH2 = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data); break;
		case 0x0c: logerror("%s: regs0_w: SYNCV = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data); break;
		case 0x16: logerror("%s: regs0_w: EXTBUF = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data); break;
		case 0x18: logerror("%s: regs0_w: EXTDATA = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data); break;
		case 0x1a: logerror("%s: regs0_w: EXTWRITE = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data); break;
		default:   logerror("%s: regs0_w: Unknown %08x = %08x%08x\n", machine().describe_context(), 0x12000000 + (offset << 3), (uint32_t)(data >> 32), (uint32_t)data); break;
	}
	m_base_regs[offset >> 1] = data;
}

READ64_MEMBER(ps2_gs_device::priv_regs1_r)
{
	uint64_t ret = 0;
	switch (offset)
	{
		case 0x00:
			ret = m_csr | (CSR_REV | CSR_ID | CSR_FIFO_EMPTY);
			logerror("%s: regs1_r: CSR (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret);
			break;
		case 0x02:
			ret = m_imr;
			logerror("%s: regs1_r: IMR (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret);
			break;
		case 0x08:
			ret = m_busdir;
			logerror("%s: regs1_r: BUSDIR (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret);
			break;
		case 0x10:
			ret = m_sig_label_id;
			logerror("%s: regs1_r: SIGLBLID (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret);
			break;
		default:
			logerror("%s: regs1_r: Unknown (%08x)\n", machine().describe_context(), 0x12000000 + (offset << 3));
			break;
	}
	return ret;
}

WRITE64_MEMBER(ps2_gs_device::priv_regs1_w)
{
	switch (offset)
	{
		case 0x00:
			logerror("%s: regs1_w: CSR = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data);
			m_csr = data &~ (CSR_RESET | CSR_SIGNAL | CSR_HSINT | CSR_VSINT | CSR_EDWINT | CSR_FLUSH);
			//m_csr |= (CSR_SIGNAL | CSR_HSINT | CSR_VSINT | CSR_EDWINT | CSR_FLUSH);
			break;
		case 0x02:
			logerror("%s: regs1_w: IMR = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data);
			m_imr = data;
			break;
		case 0x08:
			logerror("%s: regs1_w: BUSDIR = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data);
			m_busdir = data;
			break;
		case 0x10:
			logerror("%s: regs1_w: SIGLBLID = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data);
			m_sig_label_id = data;
			break;
		default:
			logerror("%s: regs1_w: Unknown %08x = %08x%08x\n", machine().describe_context(), 0x12000000 + (offset << 3), (uint32_t)(data >> 32), (uint32_t)data);
			break;
	}
}

void ps2_gs_device::write_packed(const uint8_t reg, const uint64_t hi, const uint64_t lo)
{
	switch (reg)
	{
		case 0x0e:
			regs_w(machine().dummy_space(), (uint32_t)hi, lo, ~0ULL);
			break;
		default:
			logerror("%s: write_packed: Unknown register %02x = %08x%08x%08x%08x\n", machine().describe_context(), reg, (uint32_t)(hi >> 32), (uint32_t)hi, (uint32_t)(lo >> 32), (uint32_t)lo);
			break;
	}
}

WRITE64_MEMBER(ps2_gs_device::regs_w)
{
	static char const *const dir_strs[4] = {
		"Host->Local", "Local->Host", "Local->Local", "None"
	};
	static char const *const prim_strs[8] = {
		"Point", "Line", "Line Strip", "Tri", "Tri Strip", "Tri Fan", "Sprite", "Invalid"
	};

	switch (offset)
	{
		case 0x00: // PRIM
			m_prim = data;
			m_prim_type = data & 7;
			m_gouraud_enable = BIT(data, 3);
			m_texture_enable = BIT(data, 4);
			m_fog_enable = BIT(data, 5);
			m_blend_enable = BIT(data, 6);
			m_aa_enable = BIT(data, 7);
			m_no_perspective = BIT(data, 8);
			m_curr_context = (data >> 9) & 1;
			m_fix_fragments = BIT(data, 10);
			m_kick_count = KICK_COUNTS[m_prim_type];
			logerror("%s: regs_w: PRIM = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data);
			logerror("%s          TYPE=%s GOUR=%d TEX=%d FOG=%d BLEND=%d\n", machine().describe_context(), prim_strs[m_prim_type], BIT(data, 3), BIT(data, 4), BIT(data, 5), BIT(data, 6));
			logerror("%s          AA=%d NOPERSP=%d CONTEXT=%d FIXFRAG=%d\n", machine().describe_context(), BIT(data, 7), BIT(data, 8), BIT(data, 9), BIT(data, 10));
			break;

		case 0x01: // RGBAQ
		{
			m_rgbaq = data;
			m_vc_r = data & 0xff;
			m_vc_g = (data >> 8) & 0xff;
			m_vc_b = (data >> 16) & 0xff;
			m_vc_a = (data >> 24) & 0xff;
			uint32_t q = (uint32_t)(data >> 32);
			m_q = *reinterpret_cast<float*>(&q);
			logerror("%s: regs_w: RGBAQ = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data);
			logerror("%s          R=%02x G=%02x B=%02x A=%02x Q=%f\n", machine().describe_context(), m_vc_r, m_vc_g, m_vc_b, m_vc_a, m_q);
			break;
		}

		case 0x05: // XYZ2
		{
			uint16_t x = (uint16_t)data;
			uint16_t y = (uint16_t)(data >> 16);
			uint32_t z = (uint32_t)(data >> 32);

			m_vertices[m_vertex_count] = {
				x, y, z,
				m_vc_r, m_vc_g, m_vc_b, m_vc_a, m_q,
				0, 0, 0 // TODO: U/S, V/T, fog
			};

			m_vertex_count++;

			logerror("%s: regs_w: XYZ2 = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data);
			logerror("%s:         X=%f Y=%f Z=%08x\n", machine().describe_context(), x / 16.0f, y / 16.0f, z);
			if (m_vertex_count >= m_kick_count)
			{
				logerror("%s:         Should begin primitive drawing...\n", machine().describe_context());
				switch (m_prim_type & 7)
				{
					case PRIM_TYPE_LINE_STRIP:
						m_vertices[0] = m_vertices[m_vertex_count - 1];
						m_vertex_count = 1;
						m_kick_count = 1;
						break;
					case PRIM_TYPE_TRI_STRIP:
					case PRIM_TYPE_TRI_FAN:
						m_vertices[0] = m_vertices[m_vertex_count - 2];
						m_vertices[1] = m_vertices[m_vertex_count - 1];
						m_vertex_count = 2;
						m_kick_count = 1;
						break;
					case PRIM_TYPE_POINT:
					case PRIM_TYPE_LINE:
					case PRIM_TYPE_TRI:
					case PRIM_TYPE_SPRITE:
					case PRIM_TYPE_INVALID:
					default:
						m_vertex_count = 0;
						m_kick_count = 0;
						break;
				}
			}
			break;
		}

		case 0x18: // XYOFFSET1
		case 0x19: // XYOFFSET2
		{
			const uint8_t index = offset - 0x18;
			m_context[index].m_xyoffset = data;
			m_context[index].m_offset_x = data & 0xffff;
			m_context[index].m_offset_y = (data >> 32) & 0xffff;
			logerror("%s: regs_w: XYFOFFSET%d = %08x%08x\n", machine().describe_context(), index + 1, (uint32_t)(data >> 32), (uint32_t)data);
			logerror("%s          X=%f Y=%f\n", machine().describe_context(), m_context[index].m_offset_x / 16.0f, m_context[index].m_offset_y / 16.0f);
			break;
		}

		case 0x1a: // PRMODECONT
			m_prmodecont = data;
			m_use_prim_for_attrs = BIT(data, 0);
			logerror("%s: regs_w: PRMODECONT = %08x%08x, %s\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data, m_use_prim_for_attrs ? "Use PRIM" : "Use PRMODE");
			break;

		case 0x40: // SCISSOR1
		case 0x41: // SCISSOR2
		{
			const uint8_t index = offset - 0x40;
			m_context[index].m_scissor = data;
			m_context[index].m_scissor_x0 = data & 0x7ff;
			m_context[index].m_scissor_x1 = (data >> 16) & 0x7ff;
			m_context[index].m_scissor_y0 = (data >> 32) & 0x7ff;
			m_context[index].m_scissor_y1 = (data >> 48) & 0x7ff;
			logerror("%s: regs_w: SCISSOR%d = %08x%08x\n", machine().describe_context(), index + 1, (uint32_t)(data >> 32), (uint32_t)data);
			logerror("%s          X0=d Y0=%d X1=%d Y1=%d\n", machine().describe_context(), m_context[index].m_scissor_x0, m_context[index].m_scissor_y0, m_context[index].m_scissor_x1, m_context[index].m_scissor_y1);
			break;
		}

		case 0x45: // DTHE
			m_dthe = data;
			m_dither = BIT(data, 0);
			logerror("%s: regs_w: DTHE = %08x%08x, %s\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data, m_clamp_color ? "Dither" : "No Dither");
			break;

		case 0x46: // COLCLAMP
			m_colclamp = data;
			m_clamp_color = BIT(data, 0);
			logerror("%s: regs_w: COLCLAMP = %08x%08x, %s\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data, m_clamp_color ? "Clamp Color" : "Wrap Color");
			break;

		case 0x47: // TEST1
		case 0x48: // TEST2
		{
			const uint8_t index = offset - 0x47;
			m_context[index].m_test = data;
			m_context[index].m_alpha_test = BIT(data, 0);
			m_context[index].m_alpha_func = (data >> 1) & 7;
			m_context[index].m_alpha_ref = (data >> 4) & 0xff;
			m_context[index].m_alpha_fail = (data >> 12) & 3;
			m_context[index].m_dstalpha_test = BIT(data, 14);
			m_context[index].m_dstalpha_pass1 = BIT(data, 15);
			m_context[index].m_depth_test = BIT(data, 16);
			m_context[index].m_depth_func = (data >> 17) & 3;
			logerror("%s: regs_w: SCISSOR%d = %08x%08x\n", machine().describe_context(), index + 1, (uint32_t)(data >> 32), (uint32_t)data);
			logerror("%s          X0=d Y0=%d X1=%d Y1=%d\n", machine().describe_context(), m_context[index].m_scissor_x0, m_context[index].m_scissor_y0, m_context[index].m_scissor_x1, m_context[index].m_scissor_y1);
			break;
		}

		case 0x4c: // FRAME1
		case 0x4d: // FRAME2
		{
			const uint8_t index = offset - 0x4c;
			m_context[index].m_frame = data;
			m_context[index].m_fb_base = (data & 0x1ff) << 11;
			m_context[index].m_fb_width = (data >> 10) & 0xfc0;
			m_context[index].m_fb_format = (data >> 24) & 0x3f;
			m_context[index].m_fb_mask = (uint32_t)(data >> 32);
			logerror("%s: regs_w: FRAME%d = %08x%08x\n", machine().describe_context(), index + 1, (uint32_t)(data >> 32), (uint32_t)data);
			logerror("%s          BASE=%08x WIDTH=%d FORMAT=%d MASK=%08x\n", machine().describe_context(), m_context[index].m_fb_base, m_context[index].m_fb_width, m_context[index].m_fb_format, m_context[index].m_fb_mask);
			break;
		}

		case 0x4e: // ZBUF1
		case 0x4f: // ZBUF2
		{
			const uint8_t index = offset - 0x4e;
			m_context[index].m_zbuf = data;
			m_context[index].m_z_base = (data & 0x1ff) << 11;
			m_context[index].m_z_format = (data >> 24) & 0xf;
			m_context[index].m_z_mask = BIT(data, 32);
			logerror("%s: regs_w: ZBUF%d = %08x%08x\n", machine().describe_context(), index + 1, (uint32_t)(data >> 32), (uint32_t)data);
			logerror("%s          BASE=%08x FORMAT=%d MASK=%d\n", machine().describe_context(), m_context[index].m_z_base, m_context[index].m_z_format, BIT(data, 32));
			break;
		}
		case 0x50: // BITBLTBUF
			m_bitbltbuf = data;
			m_src_buf_base  = ((uint32_t)m_bitbltbuf & 0x7fff) << 6;
			m_src_buf_width = ((uint32_t)(m_bitbltbuf >> 16) & 0x3f) << 6;
			m_src_buf_fmt   = (uint8_t)((m_bitbltbuf >> 24) & 0x3f);
			m_dst_buf_base  = ((uint32_t)(m_bitbltbuf >> 32) & 0x7fff) << 6;
			m_dst_buf_width = ((uint32_t)(m_bitbltbuf >> 48) & 0x3f) << 6;
			m_dst_buf_fmt   = (uint8_t)((m_bitbltbuf >> 56) & 0x3f);
			logerror("%s: regs_w: BITBLTBUF = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data);
			logerror("%s:         SRCBASE=%08x SRCWIDTH=%d SRCFMT=%s\n", machine().describe_context(), m_src_buf_base, m_src_buf_width, FORMAT_NAMES[m_src_buf_fmt]);
			logerror("%s:         DSTBASE=%08x DSTWIDTH=%d DSTFMT=%s\n", machine().describe_context(), m_dst_buf_base, m_dst_buf_width, FORMAT_NAMES[m_dst_buf_fmt]);
			break;
		case 0x51: // TRXPOS
			m_trx_pos = data;
			m_src_ul_x = (uint32_t)m_trx_pos & 0x7ff;
			m_src_ul_y = (uint32_t)(m_trx_pos >> 16) & 0x7ff;
			m_dst_ul_x = (uint32_t)(m_trx_pos >> 32) & 0x7ff;
			m_dst_ul_y = (uint32_t)(m_trx_pos >> 48) & 0x7ff;
			m_copy_dir = (uint8_t)(m_trx_pos >> 59) & 3;
			logerror("%s: regs_w: TRXPOS = %08x%08x, SRCUL=%d,%d  DSTUL=%d,%d, DIR=%d\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data, m_src_ul_x, m_src_ul_y, m_dst_ul_x, m_dst_ul_y, m_copy_dir);
			break;
		case 0x52: // TRXREG
			m_trx_reg = data;
			m_trx_width = (uint32_t)m_trx_reg & 0xfff;
			m_trx_height = (uint32_t)(m_trx_reg >> 32) & 0xfff;
			logerror("%s: regs_w: TRXREG = %08x%08x, DIMS=%dx%d\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data, m_trx_width, m_trx_height);
			break;
		case 0x53: // TRXDIR
			m_trx_dir = data & 3;
			logerror("%s: regs_w: TRXDIR = %08x%08x, %s\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data, dir_strs[m_trx_dir]);
			break;
		case 0x54: // HWREG
			logerror("%s: regs_w: HWREG = %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data);
			copy_dword_from_host(data);
			break;
		default:
			logerror("%s: regs_w: Unknown register %02x = %08x%08x\n", machine().describe_context(), offset, (uint32_t)(data >> 32), (uint32_t)data);
			break;
	}
}

void ps2_gs_device::copy_dword_from_host(uint64_t data)
{
	if (m_src_buf_fmt == PSMCT24 || m_src_buf_fmt == PSMZ24)
	{
		// Special-case for unevenly-sized data.
		// TODO: What do we do with leftover data, if any, when an IMAGE transfer finishes?
		logerror("%s: Unsupported src format: PSMCT24/PSMZ24\n", machine().describe_context());
		return;
	}
	const size_t bpp = FORMAT_PIXEL_WIDTHS[m_src_buf_fmt];
	offs_t dst_offset = (m_dst_buf_base + m_dst_ul_y * m_dst_buf_width + m_dst_ul_x) >> 2;
	for (size_t shift = 0, index = 0; shift < 64; shift += bpp, index++)
	{
		uint64_t src = (data >> shift) & ((1ULL << bpp) - 1ULL);
		switch (m_dst_buf_fmt)
		{
			case PSMCT32:
			case PSMZ32:
			{
				m_ram[dst_offset + index] = (uint32_t)src;
				break;
			}
			case PSMCT24:
			case PSMZ24:
				logerror("%s: Unsupported dst format: PSMCT24/PSMZ24\n", machine().describe_context());
				break;
			case PSMCT16:
			case PSMCT16S:
			case PSMZ16:
			case PSMZ16S:
			{
				const offs_t dst_shift = ((index & 1) << 4);
				m_ram[dst_offset + (index >> 1)] &= ~(0x0000ffff << dst_shift);
				m_ram[dst_offset + (index >> 1)] |= (uint32_t)src << dst_shift;
				break;
			}
			case PSMT8:
			{
				const offs_t dst_shift = ((index & 3) << 3);
				m_ram[dst_offset + (index >> 2)] &= ~(0x000000ff << dst_shift);
				m_ram[dst_offset + (index >> 2)] |= (uint32_t)src << dst_shift;
				break;
			}
			case PSMT8H:
			{
				m_ram[dst_offset + index] &= 0x00ffffff;
				m_ram[dst_offset + index] |= (uint32_t)src << 24;
				break;
			}
			case PSMT4:
			{
				const offs_t dst_shift = ((index & 7) << 2);
				m_ram[dst_offset + (index >> 3)] &= ~(0x0000000f << dst_shift);
				m_ram[dst_offset + (index >> 3)] |= (uint32_t)src << dst_shift;
				break;
			}
			case PSMT4HL:
				m_ram[dst_offset + index] &= 0xf0ffffff;
				m_ram[dst_offset + index] |= (uint32_t)src << 24;
				break;
			case PSMT4HH:
				m_ram[dst_offset + index] &= 0x0fffffff;
				m_ram[dst_offset + index] |= (uint32_t)src << 28;
				break;
			default:
				logerror("%s: copy_dword_from_host: Unknown format %02x\n", machine().describe_context(), m_dst_buf_fmt);
				return;
		}
	}
}

void ps2_gs_device::vblank_start()
{
}

void ps2_gs_device::vblank_end()
{
	m_curr_field ^= 1;
	if (m_curr_field)
		m_csr &= ~CSR_FIELD_ODD;
	else
		m_csr |= CSR_FIELD_ODD;
}

ps2_gif_device* ps2_gs_device::interface()
{
	return m_gif.target();
}

READ32_MEMBER(ps2_gs_device::gif_r)
{
	return m_gif->read(space, offset, mem_mask);
}

WRITE32_MEMBER(ps2_gs_device::gif_w)
{
	m_gif->write(space, offset, data, mem_mask);
}
