// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    SGI "Newport" graphics board used in the Indy and some Indigo2s

    Newport is modular, consisting of the following custom chips:
    - REX3: Raster Engine, which is basically a blitter which can also draw antialiased lines.
            REX also acts as the interface to the rest of the system - all the other chips on
        a Newport board are accessed through it.
    - RB2: Frame buffer input controller
    - RO1: Frame buffer output controller
    - XMAP9: Final display generator
    - CMAP: Palette mapper
    - VC2: Video timing controller / CRTC

    Taken from the Linux Newport driver, slave addresses for Newport devices are:
            VC2         0
            Both CMAPs  1
            CMAP 0      2
            CMAP 1      3
            Both XMAPs  4
            XMAP 0      5
            XMAP 1      6
            RAMDAC      7
            VIDEO (CC1) 8
            VIDEO (AB1) 9
*/

#include "emu.h"
#include "video/newport.h"

#define LOG_UNKNOWN     (1 << 0)
#define LOG_VC2         (1 << 1)
#define LOG_CMAP0       (1 << 2)
#define LOG_CMAP1       (1 << 3)
#define LOG_XMAP0       (1 << 4)
#define LOG_XMAP1       (1 << 5)
#define LOG_REX3        (1 << 6)
#define LOG_COMMANDS    (1 << 7)
#define LOG_REJECTS     (1 << 8)
#define LOG_ALL         (LOG_UNKNOWN | LOG_VC2 | LOG_CMAP0 | LOG_CMAP1 | LOG_XMAP0 | LOG_XMAP1 | LOG_REX3)

#define VERBOSE 		(0)//(LOG_UNKNOWN | LOG_REX3 | LOG_COMMANDS | LOG_REJECTS)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NEWPORT_VIDEO, newport_video_device, "newport_video", "SGI Newport graphics board")


newport_video_device::newport_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NEWPORT_VIDEO, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_hpc3(*this, finder_base::DUMMY_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void newport_video_device::device_start()
{
	m_rgbci = make_unique_clear<uint8_t[]>((1280+64) * (1024+64));
	m_olay = make_unique_clear<uint8_t[]>((1280+64) * (1024+64));
	m_pup = make_unique_clear<uint8_t[]>((1280+64) * (1024+64));
	m_cid = make_unique_clear<uint8_t[]>((1280+64) * (1024+64));

	save_pointer(NAME(m_rgbci), (1280+64) * (1024+64));
	save_pointer(NAME(m_olay), (1280+64) * (1024+64));
	save_pointer(NAME(m_pup), (1280+64) * (1024+64));
	save_pointer(NAME(m_cid), (1280+64) * (1024+64));
	save_item(NAME(m_vc2.m_vid_entry));
	save_item(NAME(m_vc2.m_cursor_entry));
	save_item(NAME(m_vc2.m_cursor_x));
	save_item(NAME(m_vc2.m_cursor_y));
	save_item(NAME(m_vc2.m_cur_cursor_x));
	save_item(NAME(m_vc2.m_did_entry));
	save_item(NAME(m_vc2.m_scanline_len));
	save_item(NAME(m_vc2.m_ram_addr));
	save_item(NAME(m_vc2.m_vt_frame_ptr));
	save_item(NAME(m_vc2.m_vt_line_ptr));
	save_item(NAME(m_vc2.m_vt_line_run));
	save_item(NAME(m_vc2.m_vt_line_count));
	save_item(NAME(m_vc2.m_cursor_table_ptr));
	save_item(NAME(m_vc2.m_work_cursor_y));
	save_item(NAME(m_vc2.m_did_frame_ptr));
	save_item(NAME(m_vc2.m_did_line_ptr));
	save_item(NAME(m_vc2.m_display_ctrl));
	save_item(NAME(m_vc2.m_config));
	save_item(NAME(m_vc2.m_ram));
	save_item(NAME(m_vc2.m_reg_idx));
	save_item(NAME(m_vc2.m_reg_data));

	save_item(NAME(m_xmap0.m_config));
	save_item(NAME(m_xmap0.m_revision));
	save_item(NAME(m_xmap0.m_entries));
	save_item(NAME(m_xmap0.m_cursor_cmap));
	save_item(NAME(m_xmap0.m_popup_cmap));
	save_item(NAME(m_xmap0.m_mode_table_idx));
	save_item(NAME(m_xmap0.m_mode_table));
	save_item(NAME(m_xmap1.m_config));
	save_item(NAME(m_xmap1.m_revision));
	save_item(NAME(m_xmap1.m_entries));
	save_item(NAME(m_xmap1.m_cursor_cmap));
	save_item(NAME(m_xmap1.m_popup_cmap));
	save_item(NAME(m_xmap1.m_mode_table_idx));
	save_item(NAME(m_xmap1.m_mode_table));

	save_item(NAME(m_rex3.m_draw_mode0));
	save_item(NAME(m_rex3.m_draw_mode1));
	save_item(NAME(m_rex3.m_write_width));
	save_item(NAME(m_rex3.m_ls_mode));
	save_item(NAME(m_rex3.m_ls_pattern));
	save_item(NAME(m_rex3.m_ls_pattern_saved));
	save_item(NAME(m_rex3.m_z_pattern));
	save_item(NAME(m_rex3.m_color_back));
	save_item(NAME(m_rex3.m_color_vram));
	save_item(NAME(m_rex3.m_alpha_ref));
	save_item(NAME(m_rex3.m_smask_x));
	save_item(NAME(m_rex3.m_smask_y));
	save_item(NAME(m_rex3.m_setup));
	save_item(NAME(m_rex3.m_step_z));
	save_item(NAME(m_rex3.m_x_start));
	save_item(NAME(m_rex3.m_y_start));
	save_item(NAME(m_rex3.m_x_end));
	save_item(NAME(m_rex3.m_y_end));

	save_item(NAME(m_rex3.m_x_save));
	save_item(NAME(m_rex3.m_xy_move));
	save_item(NAME(m_rex3.m_x_move));
	save_item(NAME(m_rex3.m_y_move));
	save_item(NAME(m_rex3.m_bres_d));
	save_item(NAME(m_rex3.m_bres_s1));
	save_item(NAME(m_rex3.m_bres_octant_inc1));
	save_item(NAME(m_rex3.m_bres_round_inc2));
	save_item(NAME(m_rex3.m_bres_e1));
	save_item(NAME(m_rex3.m_bres_s2));
	save_item(NAME(m_rex3.m_a_weight0));
	save_item(NAME(m_rex3.m_a_weight1));
	save_item(NAME(m_rex3.m_x_start_f));
	save_item(NAME(m_rex3.m_y_start_f));
	save_item(NAME(m_rex3.m_x_end_f));
	save_item(NAME(m_rex3.m_y_end_f));
	save_item(NAME(m_rex3.m_x_start_i));
	save_item(NAME(m_rex3.m_y_start_i));
	save_item(NAME(m_rex3.m_x_end_i));
	save_item(NAME(m_rex3.m_y_end_i));
	save_item(NAME(m_rex3.m_xy_start_i));
	save_item(NAME(m_rex3.m_xy_end_i));
	save_item(NAME(m_rex3.m_x_start_end_i));
	save_item(NAME(m_rex3.m_color_red));
	save_item(NAME(m_rex3.m_color_alpha));
	save_item(NAME(m_rex3.m_color_green));
	save_item(NAME(m_rex3.m_color_blue));
	save_item(NAME(m_rex3.m_slope_red));
	save_item(NAME(m_rex3.m_slope_alpha));
	save_item(NAME(m_rex3.m_slope_green));
	save_item(NAME(m_rex3.m_slope_blue));
	save_item(NAME(m_rex3.m_write_mask));
	save_item(NAME(m_rex3.m_color_i));
	save_item(NAME(m_rex3.m_zero_overflow));
	save_item(NAME(m_rex3.m_host_dataport));
	save_item(NAME(m_rex3.m_dcb_mode));
	save_item(NAME(m_rex3.m_dcb_reg_select));
	save_item(NAME(m_rex3.m_dcb_slave_select));
	save_item(NAME(m_rex3.m_dcb_data_msw));
	save_item(NAME(m_rex3.m_dcb_data_lsw));
	save_item(NAME(m_rex3.m_top_scanline));
	save_item(NAME(m_rex3.m_xy_window));
	save_item(NAME(m_rex3.m_x_window));
	save_item(NAME(m_rex3.m_y_window));
	save_item(NAME(m_rex3.m_clip_mode));
	save_item(NAME(m_rex3.m_config));
	save_item(NAME(m_rex3.m_status));
	save_item(NAME(m_rex3.m_xfer_width));
	save_item(NAME(m_rex3.m_read_active));

	save_item(NAME(m_cmap0.m_palette_idx));
	save_item(NAME(m_cmap0.m_palette));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void newport_video_device::device_reset()
{
	memset(&m_vc2, 0, sizeof(vc2_t));
	memset(&m_xmap0, 0, sizeof(xmap_t));
	memset(&m_xmap1, 0, sizeof(xmap_t));
	memset(&m_rex3, 0, sizeof(rex3_t));
	memset(&m_cmap0, 0, sizeof(cmap_t));

	m_rex3.m_draw_mode0 = 0x00000000;
	m_rex3.m_draw_mode1 = 0x3002f001;
	m_rex3.m_dcb_mode = 0x00000780;

	m_xmap0.m_entries = 0x2;
	m_xmap1.m_entries = 0x2;

#if ENABLE_NEWVIEW_LOG
	m_newview_log = nullptr;
#endif
}

#if ENABLE_NEWVIEW_LOG
void newport_video_device::start_logging()
{
	uint16_t log_index = 0xffff;
	char log_name_buf[128];
	FILE *log_test = nullptr;
	do
	{
		log_index++;
		snprintf(log_name_buf, 128, "newview%04d.log", log_index);
		log_test = fopen(log_name_buf, "rb");
	} while(log_test != nullptr);

	m_newview_log = fopen(log_name_buf, "wb");

	popmessage("Recording Newport to %s", log_name_buf);

	fwrite(&m_vc2, sizeof(vc2_t), 1, m_newview_log);
	fwrite(&m_xmap0, sizeof(xmap_t), 1, m_newview_log);
	fwrite(&m_xmap1, sizeof(xmap_t), 1, m_newview_log);
	fwrite(&m_rex3, sizeof(rex3_t), 1, m_newview_log);
	fwrite(&m_cmap0, sizeof(cmap_t), 1, m_newview_log);
	fwrite(&m_rgbci[0], 1, (1280+64)*(1024+64), m_newview_log);
	fwrite(&m_olay[0], 1, (1280+64)*(1024+64), m_newview_log);
	fwrite(&m_pup[0], 1, (1280+64)*(1024+64), m_newview_log);
	fwrite(&m_cid[0], 1, (1280+64)*(1024+64), m_newview_log);
}

void newport_video_device::stop_logging()
{
	popmessage("Newport recording stopped.");
	fclose(m_newview_log);
	m_newview_log = nullptr;
}
#endif

uint8_t newport_video_device::get_cursor_pixel(int x, int y)
{
	if (x < 0 || y < 0)
		return 0;

	bool monochrome_cursor = BIT(m_vc2.m_display_ctrl, DCR_CURSOR_SIZE_BIT) == DCR_CURSOR_SIZE_64;

	int size = monochrome_cursor ? 64 : 32;
	if (x >= size || y >= size)
		return 0;

	const int shift = 15 - (x % 16);

	if (monochrome_cursor)
	{
		const int address = y * 4 + (x / 16);
		const uint16_t word = m_vc2.m_ram[m_vc2.m_cursor_entry + address];
		return BIT(word, shift);
	}
	else
	{
		const int address = y * 2 + (x / 16);
		const uint16_t word0 = m_vc2.m_ram[m_vc2.m_cursor_entry + address];
		const uint16_t word1 = m_vc2.m_ram[m_vc2.m_cursor_entry + address + 64];
		return BIT(word0, shift) | (BIT(word1, shift) << 1);
	}
}

uint32_t newport_video_device::screen_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bool enable_cursor = BIT(m_vc2.m_display_ctrl, DCR_CURSOR_FUNC_ENABLE_BIT) != 0
					  && BIT(m_vc2.m_display_ctrl, DCR_CURSOR_ENABLE_BIT) != 0
					  && BIT(m_vc2.m_display_ctrl, DCR_CURSOR_MODE_BIT) == DCR_CURSOR_MODE_GLYPH;

	const uint16_t cursor_msb = (uint16_t)m_xmap0.m_cursor_cmap << 5;
	const uint16_t popup_msb = (uint16_t)m_xmap0.m_popup_cmap << 5;

	/* loop over rows and copy to the destination */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint8_t *src_ci = &m_rgbci[1344 * y];
		uint8_t *src_pup = &m_pup[1344 * y];
		uint32_t *dest = &bitmap.pix32(y, cliprect.min_x);

		m_vc2.m_did_frame_ptr = m_vc2.m_did_entry + (uint16_t)y;
		m_vc2.m_did_line_ptr = m_vc2.m_ram[m_vc2.m_did_frame_ptr];

		// Fetch the initial DID entry
		uint16_t curr_did_entry = m_vc2.m_ram[m_vc2.m_did_line_ptr];
		uint32_t table_entry = m_xmap0.m_mode_table[curr_did_entry & 0x1f];
		uint16_t ci_msb = 0;
		switch ((table_entry >> 8) & 3)
		{
			case 0: ci_msb = (m_xmap0.m_mode_table[curr_did_entry & 0x1f] & 0xf8) << 5; break;
			case 1: ci_msb = 0x1d00; break;
			case 2: ci_msb = 0x1e00; break;
			case 3: ci_msb = 0x1f00; break;
		}

		// Prepare for the next DID entry
		m_vc2.m_did_line_ptr++;
		curr_did_entry = m_vc2.m_ram[m_vc2.m_did_line_ptr];

		/* loop over columns */
		for (int x = cliprect.min_x; x < cliprect.max_x; x++)
		{
			if ((uint16_t)x == (curr_did_entry >> 5))
			{
				table_entry = m_xmap0.m_mode_table[curr_did_entry & 0x1f];
				switch ((table_entry >> 8) & 3)
				{
					case 0: ci_msb = (m_xmap0.m_mode_table[curr_did_entry & 0x1f] & 0xf8) << 5; break;
					case 1: ci_msb = 0x1d00; break;
					case 2: ci_msb = 0x1e00; break;
					case 3: ci_msb = 0x1f00; break;
				}
				m_vc2.m_did_line_ptr++;
				curr_did_entry = m_vc2.m_ram[m_vc2.m_did_line_ptr];
			}
			uint8_t cursor_pixel = 0;
			if (x >= (m_vc2.m_cursor_x - 31) && x <= m_vc2.m_cursor_x && y >= (m_vc2.m_cursor_y - 31) && y <= m_vc2.m_cursor_y && enable_cursor)
			{
				cursor_pixel = get_cursor_pixel(x - ((int)m_vc2.m_cursor_x - 31), y - ((int)m_vc2.m_cursor_y - 31));
			}

			if (cursor_pixel)
			{
				*dest++ = m_cmap0.m_palette[cursor_msb | cursor_pixel];
			}
			else if (*src_pup)
			{
				const uint8_t src = (*src_pup >> 2) & 3;
				*dest++ = m_cmap0.m_palette[popup_msb | src];
			}
			else
			{
				switch ((table_entry >> 8) & 3)
				{
					case 0:
						switch ((table_entry >> 10) & 3)
						{
							case 0: // 4bpp
							{
								const uint8_t shift = BIT(table_entry, 0) ? 4 : 0;
								const uint8_t pix_in = *src_ci;
								*dest++ = m_cmap0.m_palette[ci_msb | ((pix_in >> shift) & 0x0f)];
								break;
							}
							case 1: // 8bpp
								*dest++ = m_cmap0.m_palette[ci_msb | *src_ci];
								break;
							case 2: // 12bpp (not yet supported)
							case 3: // 24bpp (not yet supported)
								break;
						}
						break;
					case 1:
					case 2:
					case 3:
					{
						const uint8_t pix_in = *src_ci;
						const uint8_t r = (0x92 * BIT(pix_in, 2)) | (0x49 * BIT(pix_in, 1)) | (0x24 * BIT(pix_in, 0));
						const uint8_t g = (0x92 * BIT(pix_in, 5)) | (0x49 * BIT(pix_in, 4)) | (0x24 * BIT(pix_in, 3));
						const uint8_t b = (0xaa * BIT(pix_in, 7)) | (0x55 * BIT(pix_in, 6));
						*dest++ = (r << 16) | (g << 8) | b;
						break;
					}
				}
			}

			src_ci++;
			src_pup++;
		}
	}

#if ENABLE_NEWVIEW_LOG
	if (machine().input().code_pressed_once(KEYCODE_TILDE))
	{
		if (m_newview_log == nullptr)
			start_logging();
		else
			stop_logging();
	}
#endif

	return 0;
}


void newport_video_device::cmap0_write(uint32_t data)
{
	switch (m_rex3.m_dcb_reg_select)
	{
	case 0x00:
		LOGMASKED(LOG_CMAP0, "CMAP0 Palette Index Write: %04x\n", data & 0xffff);
		m_cmap0.m_palette_idx = (uint16_t)data;
		break;
	case 0x02:
		m_cmap0.m_palette[m_cmap0.m_palette_idx] = data >> 8;
		if (m_cmap0.m_palette_idx < 0x2000)
			set_pen_color(m_cmap0.m_palette_idx, rgb_t((uint8_t)(data >> 24), (uint8_t)(data >> 16), (uint8_t)(data >> 8)));
		LOGMASKED(LOG_CMAP0, "CMAP0 Palette Entry %04x Write: %08x\n", m_cmap0.m_palette_idx, data >> 8);
		break;
	default:
		LOGMASKED(LOG_CMAP0 | LOG_UNKNOWN, "Unknown CMAP0 Register %d Write: %08x\n", m_rex3.m_dcb_reg_select, data);
		break;
	}
}

uint32_t newport_video_device::cmap0_read()
{
	switch (m_rex3.m_dcb_reg_select)
	{
	case 0x04:
		LOGMASKED(LOG_CMAP0, "CMAP0 Status Read: %08x\n", 0x8);
		return 0x8;
	case 0x06: /* Revision */
		LOGMASKED(LOG_CMAP0, "CMAP0 Revision Read: CMAP Rev 1, Board Rev 2, 8bpp (000000a1)\n");
		return 0xa1;
	default:
		LOGMASKED(LOG_CMAP0 | LOG_UNKNOWN, "Unknown CMAP0 Register %d Read\n", m_rex3.m_dcb_reg_select);
		return 0;
	}
}

uint32_t newport_video_device::cmap1_read()
{
	switch (m_rex3.m_dcb_reg_select)
	{
	case 0x04:
		LOGMASKED(LOG_CMAP1, "CMAP1 Status Read: %08x\n", 0x8);
		return 0x8;
	case 0x06: /* Revision */
		LOGMASKED(LOG_CMAP1, "CMAP1 Revision Read: CMAP Rev 1, Board Rev 2, 8bpp (000000a1)\n");
		return 0xa1;
	default:
		LOGMASKED(LOG_CMAP1 | LOG_UNKNOWN, "Unknown CMAP0 Register %d Read\n", m_rex3.m_dcb_reg_select);
		return 0;
	}
}

uint32_t newport_video_device::xmap0_read()
{
	switch (m_rex3.m_dcb_reg_select)
	{
	case 0:
		LOGMASKED(LOG_XMAP0, "XMAP0 Config Read: %08x\n", m_xmap0.m_config);
		return m_xmap0.m_config;
	case 1:
		LOGMASKED(LOG_XMAP0, "XMAP0 Revision Read: %08x\n", 0);
		return 1;
	case 2:
		LOGMASKED(LOG_XMAP0, "XMAP0 FIFO Availability Read: %08x\n", 0x2);
		return 0x2;
	case 3:
		LOGMASKED(LOG_XMAP0, "XMAP0 Cursor CMAP MSB Read: %08x\n", m_xmap0.m_cursor_cmap);
		return m_xmap0.m_cursor_cmap;
	case 4:
		LOGMASKED(LOG_XMAP0, "XMAP0 Pop Up CMAP MSB Read: %08x\n", m_xmap0.m_popup_cmap);
		return m_xmap0.m_popup_cmap;
	case 5:
	{
		uint8_t mode_idx = (m_xmap0.m_mode_table_idx & 0x7c) >> 2;
		switch (m_xmap0.m_mode_table_idx & 3)
		{
		case 0:
		{
			uint8_t ret = (uint8_t)(m_xmap0.m_mode_table[mode_idx] >> 16);
			LOGMASKED(LOG_XMAP0, "XMAP0 Mode Register Read: %02x (Byte 0): %08x\n", mode_idx, ret);
			return ret;
		}
		case 1:
		{
			uint8_t ret = (uint8_t)(m_xmap0.m_mode_table[mode_idx] >> 8);
			LOGMASKED(LOG_XMAP0, "XMAP0 Mode Register Read: %02x (Byte 1): %08x\n", mode_idx, ret);
			return ret;
		}
		case 2:
		{
			uint8_t ret = (uint8_t)m_xmap0.m_mode_table[mode_idx];
			LOGMASKED(LOG_XMAP0, "XMAP0 Mode Register Read: %02x (Byte 2): %08x\n", mode_idx, ret);
			return ret;
		}
		}
		break;
	}
	case 6:
		LOGMASKED(LOG_XMAP0, "XMAP0 Unused Read: %08x\n", 0);
		return 0;
	case 7:
		LOGMASKED(LOG_XMAP0, "XMAP0 Mode Table Address Read: %08x\n", m_xmap0.m_mode_table_idx);
		return m_xmap0.m_mode_table_idx;
	default:
		LOGMASKED(LOG_XMAP0 | LOG_UNKNOWN, "XMAP0 Unknown DCB Register Select Value: %02x, returning 0\n", m_rex3.m_dcb_reg_select);
		return 0;
	}
	return 0;
}

void newport_video_device::xmap0_write(uint32_t data)
{
	switch (m_rex3.m_dcb_reg_select)
	{
	case 0:
		LOGMASKED(LOG_XMAP0, "XMAP0 Config Write: %02x\n", (uint8_t)data);
		m_xmap0.m_config = (uint8_t)data;
		break;
	case 1:
		LOGMASKED(LOG_XMAP0, "XMAP0 Revision Write (Ignored): %02x\n", (uint8_t)data);
		break;
	case 2:
		LOGMASKED(LOG_XMAP0, "XMAP0 FIFO Availability Write (Ignored): %02x\n", (uint8_t)data);
		break;
	case 3:
		LOGMASKED(LOG_XMAP0, "XMAP0 Cursor CMAP MSB Write: %02x\n", (uint8_t)data);
		m_xmap0.m_cursor_cmap = (uint8_t)data;
		break;
	case 4:
		LOGMASKED(LOG_XMAP0, "XMAP0 Pop Up CMAP MSB Write: %02x\n", (uint8_t)data);
		m_xmap0.m_popup_cmap = (uint8_t)data;
		break;
	case 5:
		LOGMASKED(LOG_XMAP0, "XMAP0 Mode Register Write: %02x = %06x\n", data >> 24, data & 0xffffff);
		m_xmap0.m_mode_table[data >> 24] = data & 0xffffff;
		break;
	case 6:
		LOGMASKED(LOG_XMAP0, "XMAP0 Unused Write (Ignored): %08x\n", data);
		break;
	case 7:
		LOGMASKED(LOG_XMAP0, "XMAP0 Mode Table Address Write: %02x\n", (uint8_t)data);
		m_xmap0.m_mode_table_idx = (uint8_t)data;
		break;
	default:
		LOGMASKED(LOG_XMAP0 | LOG_UNKNOWN, "XMAP0 Unknown DCB Register Select Value: %02x = %08x\n", m_rex3.m_dcb_reg_select, data);
		break;
	}
}

uint32_t newport_video_device::xmap1_read()
{
	switch (m_rex3.m_dcb_reg_select)
	{
	case 0:
		LOGMASKED(LOG_XMAP1, "XMAP1 Config Read: %08x\n", m_xmap1.m_config);
		return m_xmap1.m_config;
	case 1:
		LOGMASKED(LOG_XMAP1, "XMAP1 Revision Read: %08x\n", 0);
		return 1;
	case 2:
		LOGMASKED(LOG_XMAP1, "XMAP1 FIFO Availability Read: %08x\n", 0x02);
		return 0x2;
	case 3:
		LOGMASKED(LOG_XMAP1, "XMAP1 Cursor CMAP MSB Read: %08x\n", m_xmap1.m_cursor_cmap);
		return m_xmap1.m_cursor_cmap;
	case 4:
		LOGMASKED(LOG_XMAP1, "XMAP1 Pop Up CMAP MSB Read: %08x\n", m_xmap1.m_popup_cmap);
		return m_xmap1.m_popup_cmap;
	case 5:
	{
		const uint8_t mode_idx = (m_xmap1.m_mode_table_idx & 0x7c) >> 2;
		switch (m_xmap1.m_mode_table_idx & 3)
		{
		case 0:
		{
			uint8_t ret = (uint8_t)(m_xmap1.m_mode_table[mode_idx] >> 16);
			LOGMASKED(LOG_XMAP1, "XMAP1 Mode Register Read: %02x (Byte 0): %08x\n", mode_idx, ret);
			return ret;
		}
		case 1:
		{
			uint8_t ret = (uint8_t)(m_xmap1.m_mode_table[mode_idx] >> 8);
			LOGMASKED(LOG_XMAP1, "XMAP1 Mode Register Read: %02x (Byte 1): %08x\n", mode_idx, ret);
			return ret;
		}
		case 2:
		{
			uint8_t ret = (uint8_t)m_xmap1.m_mode_table[mode_idx];
			LOGMASKED(LOG_XMAP1, "XMAP1 Mode Register Read: %02x (Byte 2): %08x\n", mode_idx, ret);
			return ret;
		}
		}
		break;
	}
	case 6:
		LOGMASKED(LOG_XMAP1, "XMAP1 Unused Read: %08x\n", 0);
		return 0;
	case 7:
		LOGMASKED(LOG_XMAP1, "XMAP1 Mode Table Address Read: %08x\n", m_xmap0.m_mode_table_idx);
		return m_xmap1.m_mode_table_idx;
	default:
		LOGMASKED(LOG_XMAP1 | LOG_UNKNOWN, "XMAP1 Unknown DCB Register Select Value: %02x, returning 0\n", m_rex3.m_dcb_reg_select);
		return 0;
	}
	return 0;
}

void newport_video_device::xmap1_write(uint32_t data)
{
	switch (m_rex3.m_dcb_reg_select)
	{
	case 0:
		LOGMASKED(LOG_XMAP1, "XMAP1 Config Write: %02x\n", (uint8_t)data);
		m_xmap1.m_config = (uint8_t)data;
		break;
	case 1:
		LOGMASKED(LOG_XMAP1, "XMAP1 Revision Write (Ignored): %02x\n", (uint8_t)data);
		break;
	case 2:
		LOGMASKED(LOG_XMAP1, "XMAP1 FIFO Availability Write (Ignored): %02x\n", (uint8_t)data);
		break;
	case 3:
		LOGMASKED(LOG_XMAP1, "XMAP1 Cursor CMAP MSB Write: %02x\n", (uint8_t)data);
		m_xmap1.m_cursor_cmap = (uint8_t)data;
		break;
	case 4:
		LOGMASKED(LOG_XMAP1, "XMAP1 Pop Up CMAP MSB Write: %02x\n", (uint8_t)data);
		m_xmap1.m_popup_cmap = (uint8_t)data;
		break;
	case 5:
		LOGMASKED(LOG_XMAP1, "XMAP1 Mode Register Write: %02x = %06x\n", data >> 24, data & 0xffffff);
		m_xmap1.m_mode_table[data >> 24] = data & 0xffffff;
		break;
	case 6:
		LOGMASKED(LOG_XMAP1, "XMAP1 Unused Write (Ignored): %08x\n", data);
		break;
	case 7:
		LOGMASKED(LOG_XMAP1, "XMAP1 Mode Table Address Write: %02x\n", (uint8_t)data);
		m_xmap1.m_mode_table_idx = (uint8_t)data;
		break;
	default:
		LOGMASKED(LOG_XMAP0 | LOG_UNKNOWN, "XMAP0 Unknown DCB Register Select Value: %02x = %08x\n", m_rex3.m_dcb_reg_select, data);
		break;
	}
}

uint32_t newport_video_device::vc2_read()
{
	switch (m_rex3.m_dcb_reg_select)
	{
	case 0x01: /* Register Read */
		switch (m_vc2.m_reg_idx)
		{
			case 0x00:
				LOGMASKED(LOG_VC2, "VC2 Register Read: Video Entry Pointer, %08x\n", m_vc2.m_vid_entry);
				return m_vc2.m_vid_entry;
			case 0x01:
				LOGMASKED(LOG_VC2, "VC2 Register Read: Cursor Entry Pointer, %08x\n", m_vc2.m_cursor_entry);
				return m_vc2.m_cursor_entry;
			case 0x02:
				LOGMASKED(LOG_VC2, "VC2 Register Read: Cursor X, %08x\n", m_vc2.m_cursor_x);
				return m_vc2.m_cursor_x;
			case 0x03:
				LOGMASKED(LOG_VC2, "VC2 Register Read: Cursor Y, %08x\n", m_vc2.m_cursor_y);
				return m_vc2.m_cursor_y;
			case 0x04:
				LOGMASKED(LOG_VC2, "VC2 Register Read: Current Cursor X, %08x\n", m_vc2.m_cur_cursor_x);
				return m_vc2.m_cur_cursor_x;
			case 0x05:
				LOGMASKED(LOG_VC2, "VC2 Register Read: DID Entry, %08x\n", m_vc2.m_did_entry);
				return m_vc2.m_did_entry;
			case 0x06:
				LOGMASKED(LOG_VC2, "VC2 Register Read: Scanline Length, %08x\n", m_vc2.m_scanline_len);
				return m_vc2.m_scanline_len;
			case 0x07:
				LOGMASKED(LOG_VC2, "VC2 Register Read: RAM Address, %08x\n", m_vc2.m_ram_addr);
				return m_vc2.m_ram_addr;
			case 0x08:
				LOGMASKED(LOG_VC2, "VC2 Register Read: VT Frame Pointer, %08x\n", m_vc2.m_vt_frame_ptr);
				return m_vc2.m_vt_frame_ptr;
			case 0x09:
				LOGMASKED(LOG_VC2, "VC2 Register Read: VT Line Sequence Pointer, %08x\n", m_vc2.m_vt_line_ptr);
				return m_vc2.m_vt_line_ptr;
			case 0x0a:
				LOGMASKED(LOG_VC2, "VC2 Register Read: VT Lines in Run, %08x\n", m_vc2.m_vt_line_run);
				return m_vc2.m_vt_line_run;
			case 0x0b:
				LOGMASKED(LOG_VC2, "VC2 Register Read: Vertical Line Count, %08x\n", m_vc2.m_vt_line_count);
				return m_vc2.m_vt_line_count;
			case 0x0c:
				LOGMASKED(LOG_VC2, "VC2 Register Read: Cursor Table Pointer, %08x\n", m_vc2.m_cursor_table_ptr);
				return m_vc2.m_cursor_table_ptr;
			case 0x0d:
				LOGMASKED(LOG_VC2, "VC2 Register Read: Working Cursor Y, %08x\n", m_vc2.m_work_cursor_y);
				return m_vc2.m_work_cursor_y;
			case 0x0e:
				LOGMASKED(LOG_VC2, "VC2 Register Read: DID Frame Pointer, %08x\n", m_vc2.m_did_frame_ptr);
				return m_vc2.m_did_frame_ptr;
			case 0x0f:
				LOGMASKED(LOG_VC2, "VC2 Register Read: DID Line Pointer, %08x\n", m_vc2.m_did_line_ptr);
				return m_vc2.m_did_line_ptr;
			case 0x10:
				LOGMASKED(LOG_VC2, "VC2 Register Read: Display Control, %08x\n", m_vc2.m_display_ctrl);
				return m_vc2.m_display_ctrl;
			case 0x1f:
				LOGMASKED(LOG_VC2, "VC2 Register Read: Configuration, %08x\n", m_vc2.m_config);
				return m_vc2.m_config;
			default:
				return 0;
		}
		break;
	case 0x03: /* RAM Read */
	{
		LOGMASKED(LOG_VC2, "VC2 RAM Read: %04x = %08x\n", m_vc2.m_ram_addr, m_vc2.m_ram[m_vc2.m_ram_addr]);
		uint16_t ret = m_vc2.m_ram[m_vc2.m_ram_addr];
		m_vc2.m_ram_addr++;
		if (m_vc2.m_ram_addr == 0x8000)
		{
			m_vc2.m_ram_addr = 0x0000;
		}
		return ret;
	}
	default:
		LOGMASKED(LOG_VC2 | LOG_UNKNOWN, "Unknown VC2 Register Read: %02x\n", m_rex3.m_dcb_reg_select);
		return 0;
	}
}

void newport_video_device::vc2_write(uint32_t data)
{
	switch (m_rex3.m_xfer_width)
	{
	case 0x01: /* Register Select */
		switch (m_rex3.m_dcb_reg_select)
		{
		case 0x00:
			m_vc2.m_reg_idx = (uint8_t)data;
			LOGMASKED(LOG_VC2, "VC2 Register Select: %02x\n", m_vc2.m_reg_idx);
			break;
		default:
			LOGMASKED(LOG_VC2 | LOG_UNKNOWN, "Unknown VC2 Register Select: DCB Register %02x, data = %08x\n", m_rex3.m_dcb_reg_select, data);
			break;
		}
		break;
	case 0x02: /* RAM Write */
		switch (m_rex3.m_dcb_reg_select)
		{
		case 0x03:
			LOGMASKED(LOG_VC2, "VC2 RAM Write: %04x = %08x\n", m_vc2.m_ram_addr, (uint16_t)data);
			m_vc2.m_ram[m_vc2.m_ram_addr] = (uint16_t)data;
			m_vc2.m_ram_addr++;
			if (m_vc2.m_ram_addr >= 0x8000)
			{
				m_vc2.m_ram_addr = 0x0000;
			}
			break;
		default:
			LOGMASKED(LOG_VC2 | LOG_UNKNOWN, "Unknown Word Write: DCB Register %02x, data = %08x\n", m_rex3.m_dcb_reg_select, data);
			break;
		}
		break;
	case 0x03: /* Register Write */
		switch (m_rex3.m_dcb_reg_select)
		{
		case 0x00:
			LOGMASKED(LOG_VC2, "VC2 Register Setup:\n");
			m_vc2.m_reg_idx = data >> 24;
			m_vc2.m_reg_data = (uint16_t)(data >> 8);
			switch (m_vc2.m_reg_idx)
			{
				case 0x00:
					m_vc2.m_vid_entry = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: Video Entry Pointer, %04x\n", m_vc2.m_vid_entry);
					break;
				case 0x01:
					m_vc2.m_cursor_entry = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: Cursor Entry Pointer, %04x\n", m_vc2.m_cursor_entry);
					break;
				case 0x02:
					m_vc2.m_cursor_x = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: Cursor X, %04x\n", m_vc2.m_cursor_x);
					break;
				case 0x03:
					m_vc2.m_cursor_y = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: Cursor Y, %04x\n", m_vc2.m_cursor_y);
					m_vc2.m_cur_cursor_x = m_vc2.m_cursor_x;
					break;
				case 0x04:
					m_vc2.m_cur_cursor_x = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: Current Cursor X, %04x\n", m_vc2.m_cur_cursor_x);
					break;
				case 0x05:
					m_vc2.m_did_entry = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: DID Entry Pointer, %04x\n", m_vc2.m_did_entry);
					break;
				case 0x06:
					m_vc2.m_scanline_len = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: Scanline Length, %04x\n", m_vc2.m_scanline_len);
					break;
				case 0x07:
					m_vc2.m_ram_addr = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: RAM Address, %04x\n", m_vc2.m_ram_addr);
					break;
				case 0x08:
					m_vc2.m_vt_frame_ptr = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: VT Frame Table Ptr, %04x\n", m_vc2.m_vt_frame_ptr);
					break;
				case 0x09:
					m_vc2.m_vt_line_ptr = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: VT Line Sequence Pointer, %04x\n", m_vc2.m_vt_line_ptr);
					break;
				case 0x0a:
					m_vc2.m_vt_line_run = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: VT Lines in Run, %04x\n", m_vc2.m_vt_line_run);
					break;
				case 0x0b:
					m_vc2.m_vt_line_count = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: Vertical Line Count, %04x\n", m_vc2.m_vt_line_count);
					break;
				case 0x0c:
					m_vc2.m_cursor_table_ptr = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: Cursor Table Pointer, %04x\n", m_vc2.m_cursor_table_ptr);
					break;
				case 0x0d:
					m_vc2.m_work_cursor_y = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: Working Cursor Y, %04x\n", m_vc2.m_work_cursor_y);
					break;
				case 0x0e:
					m_vc2.m_did_frame_ptr = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: DID Frame Table Pointer, %04x\n", m_vc2.m_did_frame_ptr);
					break;
				case 0x0f:
					m_vc2.m_did_line_ptr = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: DID Line Table Pointer, %04x\n", m_vc2.m_did_line_ptr);
					break;
				case 0x10:
					m_vc2.m_display_ctrl = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: Display Control, %04x\n", m_vc2.m_display_ctrl);
					break;
				case 0x1f:
					m_vc2.m_config = m_vc2.m_reg_data;
					LOGMASKED(LOG_VC2, "VC2 Register Write: Configuration, %04x\n", m_vc2.m_config);
					break;
				default:
					LOGMASKED(LOG_VC2 | LOG_UNKNOWN, "VC2 Register Write: Unknown VC2 Register: %02x = %04x\n", m_vc2.m_reg_idx, m_vc2.m_reg_data);
					break;
			}
			break;
		default:
			LOGMASKED(LOG_VC2 | LOG_UNKNOWN, "Unknown VC2 Register Write: %02x = %08x\n", m_rex3.m_dcb_reg_select, data);
			break;
		}
		break;
	default:
		LOGMASKED(LOG_VC2 | LOG_UNKNOWN, "Unknown VC2 Transfer Width: Width %02x, DCB Register %02x, Value %08x\n", m_rex3.m_xfer_width, m_rex3.m_dcb_reg_select, data);
		break;
	}
}

WRITE_LINE_MEMBER(newport_video_device::vblank_w)
{
	if (state)
	{
		if (BIT(m_vc2.m_display_ctrl, 0))
		{
			m_rex3.m_status |= 0x20;
			m_hpc3->raise_local_irq(0, ioc2_device::INT3_LOCAL0_GRAPHICS);
		}
	}
}

READ64_MEMBER(newport_video_device::rex3_r)
{
	uint64_t ret = 0;
	switch (offset & ~(0x800/8))
	{
	case 0x0000/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Draw Mode 1 Read: %08x\n", m_rex3.m_draw_mode1);
			ret |= (uint64_t)m_rex3.m_draw_mode1 << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Draw Mode 0 Read: %08x\n", m_rex3.m_draw_mode0);
			ret |= m_rex3.m_draw_mode0;
		}
		break;
	case 0x0008/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Line Stipple Mode Read: %08x\n", m_rex3.m_ls_mode);
			ret |= (uint64_t)m_rex3.m_ls_mode << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Line Stipple Pattern Read: %08x\n", m_rex3.m_ls_pattern);
			ret |= m_rex3.m_ls_pattern;
		}
		break;
	case 0x0010/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Line Stipple Pattern (Save) Read: %08x\n", m_rex3.m_ls_pattern_saved);
			ret |= (uint64_t)m_rex3.m_ls_pattern_saved << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Pattern Register Read: %08x\n", m_rex3.m_z_pattern);
			ret |= m_rex3.m_z_pattern;
		}
		break;
	case 0x0018/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Opaque Pattern / Blendfunc Dest Color Read: %08x\n", m_rex3.m_color_back);
			ret |= (uint64_t)m_rex3.m_color_back << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 VRAM Fastclear Color Read: %08x\n", m_rex3.m_color_vram);
			ret |= m_rex3.m_color_vram;
		}
		break;
	case 0x0020/8:
		LOGMASKED(LOG_REX3, "REX3 AFUNCTION Reference Alpha Read: %08x\n", m_rex3.m_alpha_ref);
		ret |= (uint64_t)m_rex3.m_alpha_ref << 32;
		break;
	case 0x0028/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 0 X Min/Max Read: %08x\n", m_rex3.m_smask_x[0]);
			ret |= (uint64_t)m_rex3.m_smask_x[0] << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 0 Y Min/Max Read: %08x\n", m_rex3.m_smask_y[0]);
			ret |= m_rex3.m_smask_y[0];
		}
		break;
	case 0x0030/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Line/Span Setup Read: %08x\n", m_rex3.m_setup);
			ret |= (uint64_t)m_rex3.m_setup << 32;
		}
		break;
	case 0x0100/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "%s: REX3 X Start Read: %08x\n", machine().describe_context(), m_rex3.m_x_start);
			ret |= (uint64_t)m_rex3.m_x_start << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 YStart Read: %08x\n", m_rex3.m_y_start);
			ret |= m_rex3.m_y_start;
		}
		break;
	case 0x0108/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 XEnd Read: %08x\n", m_rex3.m_x_end);
			ret |= (uint64_t)m_rex3.m_x_end << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 YEnd Read: %08x\n", m_rex3.m_y_end);
			ret |= m_rex3.m_y_end;
		}
		break;
	case 0x0110/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 XSave Read: %08x\n", m_rex3.m_x_save);
			ret |= (uint64_t)(uint16_t)m_rex3.m_x_save << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 XYMove Read: %08x\n", m_rex3.m_xy_move);
			ret |= m_rex3.m_xy_move;
		}
		break;
	case 0x0118/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Bresenham D Read: %08x\n", m_rex3.m_bres_d);
			ret |= (uint64_t)m_rex3.m_bres_d << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Bresenham S1 Read: %08x\n", m_rex3.m_bres_s1);
			ret |= m_rex3.m_bres_s1;
		}
		break;
	case 0x0120/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Bresenham Octant & Incr1 Read: %08x\n", m_rex3.m_bres_octant_inc1);
			ret |= (uint64_t)m_rex3.m_bres_octant_inc1 << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Bresenham Octant Rounding Mode & Incr2 Read: %08x\n", m_rex3.m_bres_round_inc2);
			ret |= m_rex3.m_bres_round_inc2;
		}
		break;
	case 0x0128/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Bresenham E1 Read: %08x\n", m_rex3.m_bres_e1);
			ret |= (uint64_t)m_rex3.m_bres_e1 << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Bresenham S2 Read: %08x\n", m_rex3.m_bres_s2);
			ret |= m_rex3.m_bres_s2;
		}
		break;
	case 0x0130/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 AA Line Weight Table 1/2 Read: %08x\n", m_rex3.m_a_weight0);
			ret |= (uint64_t)m_rex3.m_a_weight0 << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 AA Line Weight Table 2/2 Read: %08x\n", m_rex3.m_a_weight1);
			ret |= m_rex3.m_a_weight1;
		}
		break;
	case 0x0138/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 GL XStart Read: %08x\n", m_rex3.m_x_start_f);
			ret |= (uint64_t)m_rex3.m_x_start_f << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 GL YStart Read: %08x\n", m_rex3.m_y_start_f);
			ret |= m_rex3.m_y_start_f;
		}
		break;
	case 0x0140/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 GL XEnd Read: %08x\n", m_rex3.m_x_end_f);
			ret |= (uint64_t)m_rex3.m_x_end_f << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 GL YEnd Read: %08x\n", m_rex3.m_y_end_f);
			ret |= m_rex3.m_y_end_f;
		}
		break;
	case 0x0148/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 XStart (integer) Read: %08x\n", m_rex3.m_x_start_i);
			ret |= (uint64_t)m_rex3.m_x_start_i << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 GL XEnd (copy) Read: %08x\n", m_rex3.m_x_end_f);
			ret |= m_rex3.m_x_end_f;
		}
		break;
	case 0x0150/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 XYStart (integer) Read: %08x\n", m_rex3.m_xy_start_i);
			ret |= (uint64_t)m_rex3.m_xy_start_i << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 XYEnd (integer) Read: %08x\n", m_rex3.m_xy_end_i);
			ret |= m_rex3.m_xy_end_i;
		}
		break;
	case 0x0158/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 XStartEnd (integer) Read: %08x\n", m_rex3.m_x_start_end_i);
			ret |= (uint64_t)m_rex3.m_x_start_end_i << 32;
		}
		break;
	case 0x0200/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Red/CI Full State Read: %08x\n", m_rex3.m_color_red);
			ret |= (uint64_t)m_rex3.m_color_red << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Alpha Full State Read: %08x\n", m_rex3.m_color_alpha);
			ret |= m_rex3.m_color_alpha;
		}
		break;
	case 0x0208/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Green Full State Read: %08x\n", m_rex3.m_color_green);
			ret |= (uint64_t)m_rex3.m_color_green << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Blue Full State Read: %08x\n", m_rex3.m_color_blue);
			ret |= m_rex3.m_color_blue;
		}
		break;
	case 0x0210/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Red/CI Slope Read: %08x\n", m_rex3.m_slope_red);
			ret |= (uint64_t)m_rex3.m_slope_red << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Alpha Slope Read: %08x\n", m_rex3.m_slope_alpha);
			ret |= m_rex3.m_slope_alpha;
		}
		break;
	case 0x0218/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Green Slope Read: %08x\n", m_rex3.m_slope_green);
			ret |= (uint64_t)m_rex3.m_slope_green << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Blue Slope Read: %08x\n", m_rex3.m_slope_blue);
			ret |= m_rex3.m_slope_blue;
		}
		break;
	case 0x0220/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Write Mask Read: %08x\n", m_rex3.m_write_mask);
			ret |= (uint64_t)m_rex3.m_write_mask << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Packed Color Fractions Read: %08x\n", m_rex3.m_color_i);
			ret |= m_rex3.m_color_i;
		}
		break;
	case 0x0228/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Color Index Zeros Overflow Read: %08x\n", m_rex3.m_zero_overflow);
			ret |= (uint64_t)m_rex3.m_zero_overflow << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Red/CI Slope (copy) Read: %08x\n", m_rex3.m_slope_red);
			ret |= m_rex3.m_slope_red;
		}
		break;
	case 0x0230/8:
		if (m_rex3.m_read_active)
			m_rex3.m_host_dataport = do_pixel_word_read();
		LOGMASKED(LOG_REX3, "%s: REX3 Host Data Port Read: %08x%08x\n", machine().describe_context(), (uint32_t)(m_rex3.m_host_dataport >> 32),
			(uint32_t)m_rex3.m_host_dataport);
		ret = m_rex3.m_host_dataport;
		break;
	case 0x0238/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Display Control Bus Mode Read: %08x\n", m_rex3.m_dcb_mode);
			ret |= (uint64_t)m_rex3.m_dcb_mode << 32;
		}
		break;
	case 0x0240/8:
		if (ACCESSING_BITS_32_63)
		{
			switch (m_rex3.m_dcb_slave_select)
			{
			case 0x00:
				ret |= (uint64_t)vc2_read() << 32;
				break;
			case 0x02:
				ret |= (uint64_t)cmap0_read() << 32;
				break;
			case 0x03:
				ret |= (uint64_t)cmap1_read() << 32;
				break;
			case 0x05:
				ret |= (uint64_t)xmap0_read() << 32;
				break;
			case 0x06:
				ret |= (uint64_t)xmap1_read() << 32;
				break;
			default:
				LOGMASKED(LOG_REX3, "REX3 Display Control Bus Data MSW Read: %08x\n", m_rex3.m_dcb_data_msw);
				ret |= (uint64_t)m_rex3.m_dcb_data_msw << 32;
				break;
			}
			if (BIT(m_rex3.m_dcb_mode, 3))
			{
				m_rex3.m_dcb_reg_select++;
				m_rex3.m_dcb_reg_select &= 7;
			}
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Display Control Bus Data LSW Read: %08x\n", m_rex3.m_dcb_data_lsw);
			ret |= m_rex3.m_dcb_data_lsw;
		}
		break;
	case 0x1300/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 1 X Min/Max Read: %08x\n", m_rex3.m_smask_x[1]);
			ret |= (uint64_t)m_rex3.m_smask_x[1] << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 1 Y Min/Max Read: %08x\n", m_rex3.m_smask_y[1]);
			ret |= m_rex3.m_smask_y[1];
		}
		break;
	case 0x1308/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 2 X Min/Max Read: %08x\n", m_rex3.m_smask_x[2]);
			ret |= (uint64_t)m_rex3.m_smask_x[2] << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 2 Y Min/Max Read: %08x\n", m_rex3.m_smask_y[2]);
			ret |= m_rex3.m_smask_y[2];
		}
		break;
	case 0x1310/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 3 X Min/Max Read: %08x\n", m_rex3.m_smask_x[3]);
			ret |= (uint64_t)m_rex3.m_smask_x[3] << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 3 Y Min/Max Read: %08x\n", m_rex3.m_smask_y[3]);
			ret |= m_rex3.m_smask_y[3];
		}
		break;
	case 0x1318/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 4 X Min/Max Read: %08x\n", m_rex3.m_smask_x[4]);
			ret |= (uint64_t)m_rex3.m_smask_x[4] << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 4 Y Min/Max Read: %08x\n", m_rex3.m_smask_y[4]);
			ret |= m_rex3.m_smask_y[4];
		}
		break;
	case 0x1320/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Top of Screen Scanline Read: %08x\n", m_rex3.m_top_scanline);
			ret |= (uint64_t)m_rex3.m_top_scanline << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 XY Window Read: %08x\n", m_rex3.m_xy_window);
			ret |= m_rex3.m_xy_window;
		}
		break;
	case 0x1328/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Clipping Mode Read: %08x\n", m_rex3.m_clip_mode);
			ret |= (uint64_t)m_rex3.m_clip_mode << 32;
		}
		break;
	case 0x1330/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Config Read: %08x\n", m_rex3.m_config);
			ret |= (uint64_t)m_rex3.m_config << 32;
		}
		break;
	case 0x1338/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Status Read: %08x\n", m_rex3.m_status);
			uint32_t old_status = m_rex3.m_status;
			m_rex3.m_status = 0;
			m_hpc3->lower_local_irq(0, ioc2_device::INT3_LOCAL0_GRAPHICS);
			ret |= (uint64_t)(old_status | 3) << 32;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 User Status Read: %08x\n", m_rex3.m_status);
			ret |= m_rex3.m_status;
		}
		break;
	default:
		LOGMASKED(LOG_REX3 | LOG_UNKNOWN, "Unknown REX3 Read: %08x (%08x%08x)\n", 0x1f0f0000 + (offset << 2), (uint32_t)(mem_mask >> 32), (uint32_t)mem_mask);
		return 0;
	}
	return ret;
}

void newport_video_device::write_pixel(uint8_t color, bool shade)
{
	if (shade)
		write_pixel(m_rex3.m_x_start_i, m_rex3.m_y_start_i, (uint8_t)(m_rex3.m_color_red >> 11));
	else
		write_pixel(m_rex3.m_x_start_i, m_rex3.m_y_start_i, color);
}

bool newport_video_device::pixel_clip_pass(int16_t x, int16_t y)
{
	bool mask0_pass = true;
	if (BIT(m_rex3.m_clip_mode, 0))
	{
		const int16_t min_x = (int16_t)(m_rex3.m_smask_x[0] >> 16);
		const int16_t min_y = (int16_t)(m_rex3.m_smask_y[0] >> 16);
		const int16_t max_x = (int16_t)m_rex3.m_smask_x[0];
		const int16_t max_y = (int16_t)m_rex3.m_smask_y[0];

		if (x < min_x)
			mask0_pass = false;
		else if (y < min_y)
			mask0_pass = false;
		else if (x > max_x)
			mask0_pass = false;
		else if (y > max_y)
			mask0_pass = false;
	}

	if (!mask0_pass)
	{
		LOGMASKED(LOG_REJECTS, "Rejecting pixel at %d,%d due to Mask 0 clipping (%d,%d - %d,%d)\n", x, y,
			(int16_t)(m_rex3.m_smask_x[0] >> 16), (int16_t)(m_rex3.m_smask_y[0] >> 16), (int16_t)m_rex3.m_smask_x[0], (int16_t)m_rex3.m_smask_y[0]);
		return false;
	}

	x += m_rex3.m_x_window;
	y += m_rex3.m_y_window;
	x -= 0x1000;
	y -= 0x1000;

	if (m_rex3.m_clip_mode & 0x1e)
	{
		uint8_t bit = 1;
		for (; bit < 5; bit++)
		{
			if (!BIT(m_rex3.m_clip_mode, bit))
				continue;

			int16_t min_x = (int16_t)(m_rex3.m_smask_x[bit] >> 16) - 0x1000;
			int16_t min_y = (int16_t)(m_rex3.m_smask_y[bit] >> 16) - 0x1000;
			int16_t max_x = (int16_t)m_rex3.m_smask_x[bit] - 0x1000;
			int16_t max_y = (int16_t)m_rex3.m_smask_y[bit] - 0x1000;

			if (x < min_x)
			{
				LOGMASKED(LOG_REJECTS, "Skipping Mask %d because %04x,%04x is outside %04x,%04x to %04x,%04x (MinX)\n", bit, x, y, min_x, min_y, max_x, max_y);
				continue;
			}
			if (x > max_x)
			{
				LOGMASKED(LOG_REJECTS, "Skipping Mask %d because %04x,%04x is outside %04x,%04x to %04x,%04x (MaxX)\n", bit, x, y, min_x, min_y, max_x, max_y);
				continue;
			}
			if (y < min_y)
			{
				LOGMASKED(LOG_REJECTS, "Skipping Mask %d because %04x,%04x is outside %04x,%04x to %04x,%04x (MinY)\n", bit, x, y, min_x, min_y, max_x, max_y);
				continue;
			}
			if (y > max_y)
			{
				LOGMASKED(LOG_REJECTS, "Skipping Mask %d because %04x,%04x is outside %04x,%04x to %04x,%04x (MaxY)\n", bit, x, y, min_x, min_y, max_x, max_y);
				continue;
			}
			break;
		}
		if (bit == 5)
		{
			LOGMASKED(LOG_REJECTS, "Rejecting pixel at %d,%d due to Mask 1-4 clipping\n", x, y);
			return false;
		}
	}

	if (x < 0 || y < 0 || x >= (1280+64) || y >= 1024)
	{
		LOGMASKED(LOG_REJECTS, "Rejecting pixel at %d,%d due to VRAM clipping\n", x, y);
		return false;
	}

	return true;
}

void newport_video_device::store_pixel(uint8_t *dest_buf, uint8_t src)
{
	if (BIT(m_rex3.m_draw_mode1, 5))
		src <<= 4;

	const uint8_t dst = *dest_buf;
	*dest_buf &= ~m_rex3.m_write_mask;

	switch ((m_rex3.m_draw_mode1 >> 28) & 15)
	{
		case 0:                                                         break;
		case 1:     *dest_buf |= (src & dst) & m_rex3.m_write_mask;     break;
		case 2:     *dest_buf |= (src & ~dst) & m_rex3.m_write_mask;    break;
		case 3:     *dest_buf |= (src) & m_rex3.m_write_mask;           break;
		case 4:     *dest_buf |= (~src & dst) & m_rex3.m_write_mask;    break;
		case 5:     *dest_buf |= (dst) & m_rex3.m_write_mask;           break;
		case 6:     *dest_buf |= (src ^ dst) & m_rex3.m_write_mask;     break;
		case 7:     *dest_buf |= (src | dst) & m_rex3.m_write_mask;     break;
		case 8:     *dest_buf |= ~(src | dst) & m_rex3.m_write_mask;    break;
		case 9:     *dest_buf |= ~(src ^ dst) & m_rex3.m_write_mask;    break;
		case 10:    *dest_buf |= ~(dst) & m_rex3.m_write_mask;          break;
		case 11:    *dest_buf |= (src | ~dst) & m_rex3.m_write_mask;    break;
		case 12:    *dest_buf |= ~(src) & m_rex3.m_write_mask;          break;
		case 13:    *dest_buf |= (~src | dst) & m_rex3.m_write_mask;    break;
		case 14:    *dest_buf |= ~(src & dst) & m_rex3.m_write_mask;    break;
		case 15:    *dest_buf |= 0xff & m_rex3.m_write_mask;            break;
	}
}

void newport_video_device::write_pixel(int16_t x, int16_t y, uint8_t color)
{
	if (!pixel_clip_pass(x, y))
		return;

	x += m_rex3.m_x_window;
	y += m_rex3.m_y_window;
	x -= 0x1000;
	y -= 0x1000;

	switch (m_rex3.m_draw_mode1 & 7)
	{
		case 1: // RGB/CI planes
			store_pixel(&m_rgbci[y * (1280 + 64) + x], color);
			break;
		case 2: // RGBA planes
			// Not yet handled
			break;
		case 4: // Overlay planes
			store_pixel(&m_olay[y * (1280 + 64) + x], color);
			break;
		case 5: // Popup planes
			store_pixel(&m_pup[y * (1280 + 64) + x], (color << 2) | (color << 6));
			break;
		case 6: // CID planes
			store_pixel(&m_cid[y * (1280 + 64) + x], color);
			break;
	}
}

void newport_video_device::do_v_iline(uint8_t color, bool skip_last, bool shade)
{
	int16_t x1 = m_rex3.m_x_start_i;
	int16_t y1 = m_rex3.m_y_start_i;
	int16_t y2 = m_rex3.m_y_end_i;

	int16_t incy = (y2 < y1) ? -1 : 1;

	do
	{
		if (shade)
			write_pixel(x1, y1, (uint8_t)(m_rex3.m_color_red >> 11));
		else
			write_pixel(x1, y1, color);

		y1 += incy;

		if (shade)
			m_rex3.m_color_red += (m_rex3.m_slope_red << 8) >> 8;
	} while (y1 != y2);

	if (!skip_last)
	{
		if (shade)
			write_pixel(x1, y1, (uint8_t)(m_rex3.m_color_red >> 11));
		else
			write_pixel(x1, y1, color);
	}

	write_x_start(x1 << 11);
	write_y_start(y1 << 11);
}

void newport_video_device::do_h_iline(uint8_t color, bool skip_last, bool shade)
{
	int16_t x1 = m_rex3.m_x_start_i;
	int16_t y1 = m_rex3.m_y_start_i;
	int16_t x2 = m_rex3.m_x_end_i;

	int16_t incx = (x2 < x1) ? -1 : 1;

	do
	{
		if (shade)
			write_pixel(x1, y1, (uint8_t)(m_rex3.m_color_red >> 11));
		else
			write_pixel(x1, y1, color);

		x1 += incx;

		if (shade)
			m_rex3.m_color_red += (m_rex3.m_slope_red << 8) >> 8;
	} while (x1 != x2);

	if (!skip_last) {
		if (shade)
			write_pixel(x1, y1, (uint8_t)(m_rex3.m_color_red >> 11));
		else
			write_pixel(x1, y1, color);
	}

	write_x_start(x1 << 11);
	write_y_start(y1 << 11);
}

void newport_video_device::do_iline(uint8_t color, bool skip_last, bool shade)
{
	int16_t x1 = m_rex3.m_x_start_i;
	int16_t y1 = m_rex3.m_y_start_i;
	int16_t x2 = m_rex3.m_x_end_i;
	int16_t y2 = m_rex3.m_y_end_i;

	unsigned char c1 = 0;
	int16_t incy = 1;

	int16_t dx = abs(x2 - x1);
	int16_t dy = abs(y2 - y1);

	if (dy > dx)
	{
		int16_t t = y2;
		y2 = x2;
		x2 = t;

		t = y1;
		y1 = x1;
		x1 = t;

		t = dx;
		dx = dy;
		dy = t;

		c1 = 1;
	}

	if (x1 > x2)
	{
		int16_t t = y2;
		y2 = y1;
		y1 = t;

		t = x1;
		x1 = x2;
		x2 = t;
	}

	int horiz = dy << 1;
	int diago = (dy - dx) << 1;
	int e = (dy << 1) - dx;

	if (y1 <= y2)
		incy = 1;
	else
		incy = -1;

	if (c1)
	{
		do
		{
			if (shade)
				write_pixel(y1, x1, (uint8_t)(m_rex3.m_color_red >> 11));
			else
				write_pixel(y1, x1, color);

			if (e > 0)
			{
				y1 += incy;
				e = e + diago;
			}
			else
			{
				e = e + horiz;
			}

			x1++;
			if (shade)
				m_rex3.m_color_red += (m_rex3.m_slope_red << 8) >> 8;
		} while (x1 != x2);

		if (!skip_last)
		{
			if (shade)
				write_pixel(y1, x1, (uint8_t)(m_rex3.m_color_red >> 11));
			else
				write_pixel(y1, x1, color);
		}
	}
	else
	{
		do
		{
			if (shade)
				write_pixel(x1, y1, (uint8_t)(m_rex3.m_color_red >> 11));
			else
				write_pixel(x1, y1, color);

			if (e > 0)
			{
				y1 += incy;
				e = e + diago;
			}
			else
			{
				e = e + horiz;
			}

			x1++;
			if (shade)
				m_rex3.m_color_red += (m_rex3.m_slope_red << 8) >> 8;
		} while (x1 != x2);

		if (!skip_last)
		{
			if (shade)
				write_pixel(x1, y1, (uint8_t)(m_rex3.m_color_red >> 11));
			else
				write_pixel(x1, y1, color);
		}
	}

	write_x_start(x1 << 11);
	write_y_start(y1 << 11);
}

uint8_t newport_video_device::do_pixel_read()
{
	if (m_rex3.m_xy_start_i == m_rex3.m_xy_end_i)
		m_rex3.m_read_active = false;
	LOGMASKED(LOG_COMMANDS, "Reading from %04x, %04x\n", m_rex3.m_x_start_i, m_rex3.m_y_start_i);
	m_rex3.m_bres_octant_inc1 = 0;
	const uint8_t ret = m_rgbci[m_rex3.m_y_start_i * (1280 + 64) + m_rex3.m_x_start_i];
	m_rex3.m_x_start_i++;
	if (m_rex3.m_x_start_i > m_rex3.m_x_end_i)
	{
		m_rex3.m_y_start_i++;
		m_rex3.m_x_start_i = m_rex3.m_x_save;
	}

	write_x_start(m_rex3.m_x_start_i << 11);
	write_y_start(m_rex3.m_y_start_i << 11);
	return ret;
}

uint64_t newport_video_device::do_pixel_word_read()
{
	const uint16_t x_start = (uint16_t)(m_rex3.m_xy_start_i >> 16);
	const uint16_t x_end = (uint16_t)(m_rex3.m_xy_end_i >> 16);
	const bool doubleword = BIT(m_rex3.m_draw_mode1, 10);
	const uint16_t max_width = doubleword ? 8 : 4;
	uint16_t width = (x_end - x_start) + 1;
	if (width > max_width)
		width = max_width;

	uint64_t ret = 0;
	uint64_t shift = 56;
	for (uint16_t i = 0; i < width; i++)
	{
		ret |= (uint64_t)do_pixel_read() << shift;
		shift -= 8;
	}
	return ret;
}

void newport_video_device::do_rex3_command()
{
	static const char* const s_opcode_str[4] = { "Noop", "Read", "Draw", "Scr2Scr" };
	static const char* const s_adrmode_str[8] = {
		"Span", "Block", "IntLine", "FracLine", "AALine", "Unk5", "Unk6", "Unk7"
	};

	const uint32_t mode0 = m_rex3.m_draw_mode0;
	const uint32_t mode1 = m_rex3.m_draw_mode1;

	int16_t start_x = m_rex3.m_x_start_i;
	int16_t start_y = m_rex3.m_y_start_i;
	int16_t end_x = m_rex3.m_x_end_i;
	int16_t end_y = m_rex3.m_y_end_i;
	int16_t dx = start_x > end_x ? -1 : 1;
	int16_t dy = start_y > end_y ? -1 : 1;

	LOGMASKED(LOG_COMMANDS, "REX3 Command: %08x|%08x - %s %s\n", mode0, mode1, s_opcode_str[mode0 & 3], s_adrmode_str[(mode0 >> 2) & 7]);

	switch (mode0)
	{
	case 0x00000000: // NoOp
		break;
	case 0x00000006: // Block, Draw
	{
		LOGMASKED(LOG_COMMANDS, "%04x, %04x = %02x\n", start_x, start_y, m_rex3.m_color_i & 0xff);
		m_rex3.m_bres_octant_inc1 = 0;
		write_pixel(m_rex3.m_color_i & 0xff, false);
		start_x++;
		if (start_x > end_x)
		{
			start_y++;
			start_x = m_rex3.m_x_save;
		}

		write_x_start(start_x << 11);
		write_y_start(start_y << 11);
		break;
	}
	case 0x00000046: // ColorHost, Block, Draw
	{
		m_rex3.m_bres_octant_inc1 = 0;
		if (BIT(mode1, 7)) // Packed
		{
			const bool doubleword = BIT(mode1, 10);
			uint16_t remaining_length = (end_x - start_x) + 1;
			uint16_t length = doubleword ? 8 : 4;
			if (remaining_length < length)
				length = remaining_length;
			LOGMASKED(LOG_COMMANDS, "%04x, %04x = %08x%08x\n", start_x, start_y, (uint32_t)(m_rex3.m_host_dataport >> 32), (uint32_t)m_rex3.m_host_dataport);
			uint64_t shift = 56;
			for (uint16_t i = 0; i < length; i++)
			{
				write_pixel(start_x, start_y, (uint8_t)(m_rex3.m_host_dataport >> shift));
				start_x++;
				shift -= 8;
			}
		}
		else
		{
			LOGMASKED(LOG_COMMANDS, "%04x, %04x = %02x\n", start_x, start_y, (uint8_t)(m_rex3.m_host_dataport >> 56));
			write_pixel(start_x, start_y, m_rex3.m_host_dataport >> 56);
			start_x++;
		}
		if (start_x > end_x)
		{
			start_y++;
			start_x = m_rex3.m_x_save;
		}
		write_x_start(start_x << 11);
		write_y_start(start_y << 11);
		break;
	}
	case 0x00000045: // ColorHost, Block, Read
	case 0x00000065: // ColorHost, Block, Read
	{
		m_rex3.m_read_active = true;
		break;
	}
	case 0x00000022: // DoSetup, Span, Draw
	case 0x00000102: // StopOnX, Span, Draw
	case 0x00000122: // StopOnX, DoSetup, Span, Draw
	case 0x00022102: // LSOpaque, EnLSPattern, StopOnX, Span, Draw
	case 0x00080122: // LROnly, StopOnX, DoSetup, Span, Draw
	case 0x000c0122: // LROnly, Shade, StopOnX, DoSetup, Span, Draw
	case 0x000c9102: // LROnly, Shade, Length32, EnZPattern, StopOnX, Span, Draw
	{
		if (BIT(mode0, 19) && dx < 0) // LROnly
			break;

		if (!BIT(mode0, 8))
			end_x = start_x;

		end_x += dx;
		end_y += dy;

		bool shade = BIT(mode0, 18);

		uint32_t color = m_rex3.m_color_i & 0xff;
		LOGMASKED(LOG_COMMANDS, "%04x, %04x to %04x, %04x = %08x\n", start_x, start_y, end_x, end_y, color);
		for (; start_x != end_x; start_x += dx)
		{
			if (shade)
			{
				write_pixel(start_x, start_y, (uint8_t)(m_rex3.m_color_red >> 11));
				m_rex3.m_color_red += (m_rex3.m_slope_red << 8) >> 8;
			}
			else
			{
				write_pixel(start_x, start_y, color);
			}
		}
		start_y++;

		write_x_start(start_x << 11);
		write_y_start(start_y << 11);
		break;
	}
	case 0x00000326: // StopOnX, StopOnY, DoSetup, Block, Draw
	{
		end_x += dx;
		end_y += dy;

		uint32_t color = m_rex3.m_color_i & 0xff;
		if (BIT(mode1, 17))
		{
			color = m_rex3.m_color_vram & 0xff;
		}
		LOGMASKED(LOG_COMMANDS, "%04x, %04x to %04x, %04x = %08x, %04x\n", start_x, start_y, end_x, end_y, m_cmap0.m_palette[color], m_rex3.m_x_save);
		for (; start_y != end_y; start_y += dy)
		{
			for (; start_x != end_x; start_x += dx)
			{
				write_pixel(start_x, start_y, color);
			}
			start_x = m_rex3.m_x_save;
		}

		write_x_start(start_x << 11);
		write_y_start(start_y << 11);
		break;
	}
	case 0x00000327: // StopOnX, StopOnY, DoSetup, Block, Scr2Scr
	{
		end_x += dx;
		end_y += dy;
		LOGMASKED(LOG_COMMANDS, "%04x, %04x - %04x, %04x to %04x, %04x\n", start_x, start_y, end_x, end_y, start_x + m_rex3.m_x_move, start_y + m_rex3.m_y_move);
		for (; start_y != end_y; start_y += dy)
		{
			for (; start_x != end_x; start_x += dx)
			{
				write_pixel(start_x + m_rex3.m_x_move, start_y + m_rex3.m_y_move, m_rgbci[(start_y + m_rex3.m_y_window - 0x1000) * (1280 + 64) + (start_x + m_rex3.m_x_window - 0x1000)]);
			}
			start_x = m_rex3.m_x_save;
		}
		write_x_start(start_x << 11);
		write_y_start(start_y << 11);
		break;
	}
	case 0x0000000a: // I_Line, Draw
	case 0x0000032a: // StopOnX, StopOnY, DoSetup, I_Line, Draw
	case 0x00000b2a: // SkipLast, StopOnX, StopOnY, DoSetup, I_Line, Draw
	case 0x0000232e: // EnLSPattern, StopOnX, StopOnY, DoSetup, F_Line, Draw
	case 0x0004232e: // Shade, EnLSPattern, StopOnX, StopOnY, DoSetup, F_Line, Draw
	case 0x00442332: // EndFilter, Shade, EnLSPattern, StopOnX, StopOnY, DoSetp, A_Line, Draw
	{
		const bool skip_last = BIT(mode0, 11);
		const bool shade = BIT(mode0, 18);

		LOGMASKED(LOG_COMMANDS, "%cLine: %04x, %04x to %04x, %04x = %08x\n", (mode0 & 0x1c) == 3 ? 'F' : 'I',
			start_x, start_y, end_x, end_y, m_cmap0.m_palette[m_rex3.m_color_i]);
		if (start_x == end_x && start_y == end_y)
		{
			write_pixel(m_rex3.m_color_i, shade);
		}
		else if (start_x == end_x)
		{
			do_v_iline(m_rex3.m_color_i, skip_last, shade);
		}
		else if (start_y == end_y)
		{
			do_h_iline(m_rex3.m_color_i, skip_last, shade);
		}
		else
		{
			do_iline(m_rex3.m_color_i, skip_last, shade);
		}
		break;
	}
	case 0x00001106: // EnZPattern, StopOnX, Block, Draw
	case 0x00002106: // EnLSPattern, StopOnX, Block, Draw
	case 0x00009106: // Length32, EnZPattern, StopOnX, Block, Draw
	case 0x00022106: // LSOpaque, EnLSPattern, StopOnX, Block, Draw
	case 0x00019106: // ZPOpaque, EnLSPattern, StopOnX, Block, Draw
	{
		const bool opaque = (mode0 == 0x00019106) || (mode0 == 0x00022106);
		const uint32_t pattern = BIT(mode0, 12) ? m_rex3.m_z_pattern : m_rex3.m_ls_pattern;
		const uint8_t foreground = m_rex3.m_color_i & 0xff;
		const uint8_t background = m_rex3.m_color_back & 0xff;
		LOGMASKED(LOG_COMMANDS, "%08x at %04x, %04x color %08x\n", pattern, start_x, start_y, foreground);
		end_x += dx;
		int16_t end = end_x;
		if (BIT(mode0, 15))
		{
			if ((end_x - start_x) >= 32)
			{
				end = start_x + 31;
			}
		}
		for (; start_x != end; start_x += dx)
		{
			if (pattern & (1 << (31 - (start_x - m_rex3.m_x_start_i))))
			{
				write_pixel(start_x, start_y, foreground);
			}
			else if (opaque)
			{
				write_pixel(start_x, start_y, background);
			}
		}
		if (BIT(m_rex3.m_bres_octant_inc1, 24))
			start_y--;
		else
			start_y++;

		start_x = m_rex3.m_x_save;
		write_x_start(start_x << 11);
		write_y_start(start_y << 11);
		break;
	}
	default:
		LOGMASKED(LOG_COMMANDS | LOG_UNKNOWN, "Draw command %08x not recognized\n", m_rex3.m_draw_mode0);
		break;
	}
}

void newport_video_device::write_x_start(int32_t val)
{
	m_rex3.m_x_start = val & 0x07ffff80;
	m_rex3.m_x_start_i = (int16_t)(val >> 11);
	m_rex3.m_x_start_f = (uint32_t)val & 0x007fff80;
	m_rex3.m_xy_start_i = (m_rex3.m_xy_start_i & 0x0000ffff) | (m_rex3.m_x_start_i << 16);
}

void newport_video_device::write_y_start(int32_t val)
{
	m_rex3.m_y_start = val & 0x07ffff80;
	m_rex3.m_y_start_i = (int16_t)(val >> 11);
	m_rex3.m_y_start_f = (uint32_t)val & 0x007fff80;
	m_rex3.m_xy_start_i = (m_rex3.m_xy_start_i & 0xffff0000) | (uint16_t)m_rex3.m_y_start_i;
}

void newport_video_device::write_x_end(int32_t val)
{
	m_rex3.m_x_end = val & 0x07ffff80;
	m_rex3.m_x_end_i = (int16_t)(val >> 11);
	m_rex3.m_x_end_f = (uint32_t)val & 0x007fff80;
	m_rex3.m_xy_end_i = (m_rex3.m_xy_end_i & 0x0000ffff) | (m_rex3.m_x_end_i << 16);
}

void newport_video_device::write_y_end(int32_t val)
{
	m_rex3.m_y_end = val & 0x07ffff80;
	m_rex3.m_y_end_i = (int16_t)(val >> 11);
	m_rex3.m_y_end_f = (uint32_t)val & 0x007fff80;
	m_rex3.m_xy_end_i = (m_rex3.m_xy_end_i & 0xffff0000) | (uint16_t)m_rex3.m_y_end_i;
}

WRITE64_MEMBER(newport_video_device::rex3_w)
{
#if ENABLE_NEWVIEW_LOG
	if (m_newview_log != nullptr)
	{
		uint32_t offset_lo = (uint32_t)offset;
		uint32_t data_hi = (uint32_t)(data >> 32);
		uint32_t data_lo = (uint32_t)data;
		uint32_t mem_mask_hi = (uint32_t)(mem_mask >> 32);
		uint32_t mem_mask_lo = (uint32_t)mem_mask;

		fwrite(&offset_lo, sizeof(uint32_t), 1, m_newview_log);
		fwrite(&data_hi, sizeof(uint32_t), 1, m_newview_log);
		fwrite(&data_lo, sizeof(uint32_t), 1, m_newview_log);
		fwrite(&mem_mask_hi, sizeof(uint32_t), 1, m_newview_log);
		fwrite(&mem_mask_lo, sizeof(uint32_t), 1, m_newview_log);
	}
#endif

	switch (offset & ~(0x800/8))
	{
	case 0x0000/8:
		if (ACCESSING_BITS_32_63)
		{
			const uint32_t data32 = (uint32_t)(data >> 32);
			LOGMASKED(LOG_REX3, "REX3 Draw Mode 1 Write: %08x\n", data32);
			switch (data32 & 7)
			{
			case 0x00:
				LOGMASKED(LOG_REX3, "    Planes Enabled:     None\n");
				break;
			case 0x01:
				LOGMASKED(LOG_REX3, "    Planes Enabled:     R/W RGB/CI\n");
				break;
			case 0x02:
				LOGMASKED(LOG_REX3, "    Planes Enabled:     R/W RGBA\n");
				break;
			case 0x04:
				LOGMASKED(LOG_REX3, "    Planes Enabled:     R/W OLAY\n");
				break;
			case 0x05:
				LOGMASKED(LOG_REX3, "    Planes Enabled:     R/W PUP\n");
				break;
			case 0x06:
				LOGMASKED(LOG_REX3, "    Planes Enabled:     R/W CID\n");
				break;
			default:
				LOGMASKED(LOG_REX3 | LOG_UNKNOWN, "    Unknown Plane Enable Value\n");
				break;
			}
			switch ((data32 & 0x00000018) >> 3)
			{
			case 0x00:
				LOGMASKED(LOG_REX3, "    Plane Draw Depth:    4 bits\n");
				m_rex3.m_write_width = 0x0000000f;
				break;
			case 0x01:
				LOGMASKED(LOG_REX3, "    Plane Draw Depth:    8 bits\n");
				m_rex3.m_write_width = 0x000000ff;
				break;
			case 0x02:
				LOGMASKED(LOG_REX3, "    Plane Draw Depth:   12 bits\n");
				m_rex3.m_write_width = 0x000000ff; // TODO: 24-bit
				break;
			case 0x03:
				LOGMASKED(LOG_REX3, "    Plane Draw Depth:   32 bits\n");
				m_rex3.m_write_width = 0x000000ff; // TODO: 24-bit
				break;
			}
			LOGMASKED(LOG_REX3, "    DBuf Source Buffer: %d\n", BIT(data32, 5));
			LOGMASKED(LOG_REX3, "    GL Y Coordinates:   %d\n", BIT(data32, 6));
			LOGMASKED(LOG_REX3, "    Enable Pxl Packing: %d\n", BIT(data32, 7));
			switch ((data32 & 0x00000300) >> 8)
			{
			case 0x00:
				LOGMASKED(LOG_REX3, "    HOSTRW Depth:        4 bits\n");
				break;
			case 0x01:
				LOGMASKED(LOG_REX3, "    HOSTRW Depth:        8 bits\n");
				break;
			case 0x02:
				LOGMASKED(LOG_REX3, "    HOSTRW Depth:       12 bits\n");
				break;
			case 0x03:
				LOGMASKED(LOG_REX3, "    HOSTRW Depth:       32 bits\n");
				break;
			}
			LOGMASKED(LOG_REX3, "    DWord Transfers:    %d\n", BIT(data32, 10));
			LOGMASKED(LOG_REX3, "    Swap Endianness:    %d\n", BIT(data32, 11));
			LOGMASKED(LOG_REX3, "    Compare Src > Dest: %d\n", BIT(data32, 12));
			LOGMASKED(LOG_REX3, "    Compare Src = Dest: %d\n", BIT(data32, 13));
			LOGMASKED(LOG_REX3, "    Compare Src < Dest: %d\n", BIT(data32, 14));
			LOGMASKED(LOG_REX3, "    RGB Mode Select:    %d\n", BIT(data32, 15));
			LOGMASKED(LOG_REX3, "    Enable Dithering:   %d\n", BIT(data32, 16));
			LOGMASKED(LOG_REX3, "    Enable Fast Clear:  %d\n", BIT(data32, 17));
			LOGMASKED(LOG_REX3, "    Enable Blending:    %d\n", BIT(data32, 18));
			switch ((data32 & 0x00380000) >> 19)
			{
			case 0x00:
				LOGMASKED(LOG_REX3, "    Src Blend Factor:   0\n");
				break;
			case 0x01:
				LOGMASKED(LOG_REX3, "    Src Blend Factor:   1\n");
				break;
			case 0x02:
				LOGMASKED(LOG_REX3, "    Src Blend Factor:   Normalized Dest (or COLORBACK)\n");
				break;
			case 0x03:
				LOGMASKED(LOG_REX3, "    Src Blend Factor:   1 - Normalized Dest (or COLORBACK)\n");
				break;
			case 0x04:
				LOGMASKED(LOG_REX3, "    Src Blend Factor:   Normalized Src\n");
				break;
			case 0x05:
				LOGMASKED(LOG_REX3, "    Src Blend Factor:   1 - Normalized Src\n");
				break;
			default:
				LOGMASKED(LOG_REX3 | LOG_UNKNOWN, "    Unknown Src Blend Factor: %02x\n", (data32 & 0x00380000) >> 19);
				break;
			}
			switch ((data32 & 0x01c00000) >> 22)
			{
			case 0x00:
				LOGMASKED(LOG_REX3, "    Dest Blend Factor:  0\n");
				break;
			case 0x01:
				LOGMASKED(LOG_REX3, "    Dest Blend Factor:  1\n");
				break;
			case 0x02:
				LOGMASKED(LOG_REX3, "    Dest Blend Factor:  Normalized Dest (or COLORBACK)\n");
				break;
			case 0x03:
				LOGMASKED(LOG_REX3, "    Dest Blend Factor:  1 - Normalized Dest (or COLORBACK)\n");
				break;
			case 0x04:
				LOGMASKED(LOG_REX3, "    Dest Blend Factor:  Normalized Src\n");
				break;
			case 0x05:
				LOGMASKED(LOG_REX3, "    Dest Blend Factor:  1 - Normalized Src\n");
				break;
			default:
				LOGMASKED(LOG_REX3 | LOG_UNKNOWN, "    Unknown Src Blend Factor: %02x\n", (data32 & 0x00380000) >> 19);
				break;
			}
			LOGMASKED(LOG_REX3, "  COLORBACK Dest Blend: %d\n", BIT(data32, 25));
			LOGMASKED(LOG_REX3, "   Enable Pxl Prefetch: %d\n", BIT(data32, 26));
			LOGMASKED(LOG_REX3, "    SFACTOR Src Alpha:  %d\n", BIT(data32, 27));
			switch ((data32 & 0xf0000000) >> 28)
			{
			case 0x00:
				LOGMASKED(LOG_REX3, "    Logical Op. Type:   0\n");
				break;
			case 0x01:
				LOGMASKED(LOG_REX3, "    Logical Op. Type:   Src & Dst\n");
				break;
			case 0x02:
				LOGMASKED(LOG_REX3, "    Logical Op. Type:   Src & ~Dst\n");
				break;
			case 0x03:
				LOGMASKED(LOG_REX3, "    Logical Op. Type:   Src\n");
				break;
			case 0x04:
				LOGMASKED(LOG_REX3, "    Logical Op. Type:   ~Src & Dst\n");
				break;
			case 0x05:
				LOGMASKED(LOG_REX3, "    Logical Op. Type:   Dst\n");
				break;
			case 0x06:
				LOGMASKED(LOG_REX3, "    Logical Op. Type:   Src ^ Dst\n");
				break;
			case 0x07:
				LOGMASKED(LOG_REX3, "    Logical Op. Type:   Src | Dst\n");
				break;
			case 0x08:
				LOGMASKED(LOG_REX3, "    Logical Op. Type:   ~(Src | Dst)\n");
				break;
			case 0x09:
				LOGMASKED(LOG_REX3, "    Logical Op. Type:   ~(Src ^ Dst)\n");
				break;
			case 0x0a:
				LOGMASKED(LOG_REX3, "    Logical Op. Type:   ~Dst\n");
				break;
			case 0x0b:
				LOGMASKED(LOG_REX3, "    Logical Op. Type:   Src | ~Dst\n");
				break;
			case 0x0c:
				LOGMASKED(LOG_REX3, "    Logical Op. Type:   ~Src\n");
				break;
			case 0x0d:
				LOGMASKED(LOG_REX3, "    Logical Op. Type:   ~Src | Dst\n");
				break;
			case 0x0e:
				LOGMASKED(LOG_REX3, "    Logical Op. Type:   ~(Src & Dst)\n");
				break;
			case 0x0f:
				LOGMASKED(LOG_REX3, "    Logical Op. Type:   1\n");
				break;
			}
			m_rex3.m_draw_mode1 = data32;
		}
		if (ACCESSING_BITS_0_31)
		{
			const uint32_t data32 = (uint32_t)data;
			LOGMASKED(LOG_REX3, "REX3 Draw Mode 0 Write: %08x\n", data32);
			switch (data32 & 3)
			{
			case 0x00:
				LOGMASKED(LOG_REX3, "    Primitive Function: No Op\n");
				break;
			case 0x01:
				LOGMASKED(LOG_REX3, "    Primitive Function: Read From FB\n");
				break;
			case 0x02:
				LOGMASKED(LOG_REX3, "    Primitive Function: Draw To FB\n");
				break;
			case 0x03:
				LOGMASKED(LOG_REX3, "    Primitive Function: Copy FB To FB\n");
				break;
			}
			switch ((data32 & 0x0000001c) >> 2)
			{
			case 0x00:
				LOGMASKED(LOG_REX3, "    Addressing Mode: Span/Point\n");
				break;
			case 0x01:
				LOGMASKED(LOG_REX3, "    Addressing Mode: Block\n");
				break;
			case 0x02:
				LOGMASKED(LOG_REX3, "    Addressing Mode: Bresenham Line, Integer Endpoints\n");
				break;
			case 0x03:
				LOGMASKED(LOG_REX3, "    Addressing Mode: Bresenham Line, Fractional Endpoints\n");
				break;
			case 0x04:
				LOGMASKED(LOG_REX3, "    Addressing Mode: AA Bresenham Line\n");
				break;
			default:
				LOGMASKED(LOG_REX3 | LOG_UNKNOWN, "    Unknown Addressing Mode: %02x\n", (data32 & 0x0000001c) >> 2);
				break;
			}
			LOGMASKED(LOG_REX3, "    Iterator Setup:     %d\n", BIT(data32, 5));
			LOGMASKED(LOG_REX3, "    RGB/CI Draw Source: %d\n", BIT(data32, 6));
			LOGMASKED(LOG_REX3, "     Alpha Draw Source: %d\n", BIT(data32, 7));
			LOGMASKED(LOG_REX3, "    Stop On X:          %d\n", BIT(data32, 8));
			LOGMASKED(LOG_REX3, "    Stop On Y:          %d\n", BIT(data32, 9));
			LOGMASKED(LOG_REX3, "    Skip Start Point:   %d\n", BIT(data32, 10));
			LOGMASKED(LOG_REX3, "    Skip End Point:     %d\n", BIT(data32, 11));
			LOGMASKED(LOG_REX3, "    Enable Patterning:  %d\n", BIT(data32, 12));
			LOGMASKED(LOG_REX3, "    Enable Stippling:   %d\n", BIT(data32, 13));
			LOGMASKED(LOG_REX3, "    Stipple Advance:    %d\n", BIT(data32, 14));
			LOGMASKED(LOG_REX3, "    Limit Draw To 32px: %d\n", BIT(data32, 15));
			LOGMASKED(LOG_REX3, "     Z Opaque Stipple   %d\n", BIT(data32, 16));
			LOGMASKED(LOG_REX3, "    LS Opaque Stipple:  %d\n", BIT(data32, 17));
			LOGMASKED(LOG_REX3, "    Enable Lin. Shade:  %d\n", BIT(data32, 18));
			LOGMASKED(LOG_REX3, "    Left-Right Only:    %d\n", BIT(data32, 19));
			LOGMASKED(LOG_REX3, "    Offset by XYMove:   %d\n", BIT(data32, 20));
			LOGMASKED(LOG_REX3, "    Enable CI Clamping: %d\n", BIT(data32, 21));
			LOGMASKED(LOG_REX3, "    Enable End Filter:  %d\n", BIT(data32, 22));
			LOGMASKED(LOG_REX3, "    Enable Y+2 Stride:  %d\n", BIT(data32, 23));
			m_rex3.m_draw_mode0 = data32;
		}
		break;
	case 0x0008/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Line Stipple Mode Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_ls_mode = (uint32_t)(data >> 32) & 0xfffffff;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Line Stipple Pattern Write: %08x\n", (uint32_t)data);
			m_rex3.m_ls_pattern = (uint32_t)data;
		}
		break;
	case 0x0010/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Line Stipple Pattern (Save) Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_ls_pattern_saved = (uint32_t)(data >> 32);
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Pattern Register Write: %08x\n", (uint32_t)data);
			m_rex3.m_z_pattern = (uint32_t)data;
		}
		break;
	case 0x0018/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Opaque Pattern / Blendfunc Dest Color Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_color_back = (uint32_t)(data >> 32);
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 VRAM Fastclear Color Write: %08x\n", (uint32_t)data);
			m_rex3.m_color_vram = (uint32_t)data;
		}
		break;
	case 0x0020/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 AFUNCTION Reference Alpha Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_alpha_ref = (uint8_t)(data >> 32);
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Stall GFIFO Write: %08x\n", data);
			break;
		}
		break;
	case 0x0028/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 0 X Min/Max Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_smask_x[0] = (uint32_t)(data >> 32);
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 0 Y Min/Max Write: %08x\n", (uint32_t)data);
			m_rex3.m_smask_y[0] = (uint32_t)data;
		}
		break;
	case 0x0030/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Line/Span Setup Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_setup = (uint32_t)(data >> 32);
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 ZPattern Enable Write\n");
			m_rex3.m_step_z = 1;
		}
		break;
	case 0x0038/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Update LSPATTERN/LSRCOUNT\n");
			m_rex3.m_ls_pattern = m_rex3.m_ls_pattern_saved;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Update LSPATSAVE/LSRCNTSAVE\n");
			m_rex3.m_ls_pattern_saved = m_rex3.m_ls_pattern;
		}
		break;
	case 0x0100/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 XStart Write: %08x\n", (uint32_t)(data >> 32));
			write_x_start((int32_t)(data >> 32));
			m_rex3.m_x_save = m_rex3.m_x_start_i;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 YStart Write: %08x\n", (uint32_t)data);
			write_y_start((int32_t)data);
		}
		break;
	case 0x0108/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 XEnd Write: %08x\n", (uint32_t)(data >> 32));
			write_x_end((int32_t)(data >> 32));
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 YEnd Write: %08x\n", (uint32_t)data);
			write_y_end((int32_t)data);
		}
		break;
	case 0x0110/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 XSave Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_x_save = (int16_t)(data >> 32);
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 XYMove Write: %08x\n", (uint32_t)data);
			m_rex3.m_xy_move = (uint32_t)data;
			m_rex3.m_x_move = (int16_t)(m_rex3.m_xy_move >> 16);
			m_rex3.m_y_move = (int16_t)m_rex3.m_xy_move;
		}
		break;
	case 0x0118/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Bresenham D Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_bres_d = (uint32_t)(data >> 32) & 0x7ffffff;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Bresenham S1 Write: %08x\n", (uint32_t)data);
			m_rex3.m_bres_s1 = (uint32_t)data & 0x1ffff;
		}
		break;
	case 0x0120/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Bresenham Octant & Incr1 Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_bres_octant_inc1 = (uint32_t)(data >> 32) & 0x70fffff;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Bresenham Octant Rounding Mode & Incr2 Write: %08x\n", (uint32_t)data);
			m_rex3.m_bres_round_inc2 = data & 0xff1fffff;
		}
		break;
	case 0x0128/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Bresenham E1 Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_bres_e1 = (uint16_t)(data >> 32);
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Bresenham S2 Write: %08x\n", (uint32_t)data);
			m_rex3.m_bres_s2 = (uint32_t)data & 0x3ffffff;
		}
		break;
	case 0x0130/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 AA Line Weight Table 1/2 Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_a_weight0 = (uint32_t)(data >> 32);
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 AA Line Weight Table 2/2 Write: %08x\n", (uint32_t)data);
			m_rex3.m_a_weight1 = (uint32_t)data;
		}
		break;
	case 0x0138/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 GL XStart Write: %08x\n", (uint32_t)(data >> 32));
			write_x_start((int32_t)(data >> 32) & 0x007fff80);
			m_rex3.m_x_save = m_rex3.m_x_start_i;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 GL YStart Write: %08x\n", (uint32_t)data);
			write_y_start((int32_t)data & 0x007fff80);
		}
		break;
	case 0x0140/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 GL XEnd Write: %08x\n", (uint32_t)(data >> 32));
			write_x_end((int32_t)(data >> 32) & 0x007fff80);
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 GL YEnd Write: %08x\n", (uint32_t)data);
			write_y_end((int32_t)data & 0x007fff80);
		}
		break;
	case 0x0148/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 XStart (integer) Write: %08x\n", (uint32_t)(data >> 32));
			write_x_start(((int32_t)(int16_t)(data >> 32)) << 11);
			m_rex3.m_x_save = m_rex3.m_x_start_i;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 GL XEnd (copy) Write: %08x\n", (uint32_t)data);
			write_x_end((int32_t)data & 0x007fff80);
		}
		break;
	case 0x0150/8:
		if (ACCESSING_BITS_32_63)
		{
			const uint32_t data32 = (uint32_t)(data >> 32);
			LOGMASKED(LOG_REX3, "REX3 XYStart (integer) Write: %08x\n", data32);
			write_x_start(((int32_t)(int16_t)(data >> 48)) << 11);
			write_y_start(((int32_t)(int16_t)(data >> 32)) << 11);
			m_rex3.m_x_save = m_rex3.m_x_start_i;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 XYEnd (integer) Write: %08x\n", (uint32_t)data);
			write_x_end(((int32_t)(int16_t)(data >> 16)) << 11);
			write_y_end(((int32_t)(int16_t)(data >>  0)) << 11);
		}
		break;
	case 0x0158/8:
		if (ACCESSING_BITS_32_63)
		{
			const uint32_t data32 = (uint32_t)(data >> 32);
			LOGMASKED(LOG_REX3, "REX3 XStartEnd (integer) Write: %08x\n", data32);
			write_x_start(((int32_t)(int16_t)(data >> 48)) << 11);
			write_x_end(((int32_t)(int16_t)(data >> 32)) << 11);
			m_rex3.m_x_save = m_rex3.m_x_start_i;
		}
		break;
	case 0x0200/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Red/CI Full State Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_color_red = (int32_t)((data >> 32) & 0xffffff);
			m_rex3.m_color_i = (uint32_t)(m_rex3.m_color_red >> 11) & 0x000000ff;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Alpha Full State Write: %08x\n", (uint32_t)data);
			m_rex3.m_color_alpha = (int32_t)(data & 0xfffff);
		}
		break;
	case 0x0208/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Green Full State Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_color_green = (int32_t)((data >> 32) & 0xfffff);
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Blue Full State Write: %08x\n", (uint32_t)data);
			m_rex3.m_color_blue = (int32_t)(data & 0xfffff);
		}
		break;
	case 0x0210/8:
	{
		if (ACCESSING_BITS_32_63)
		{
			uint32_t data32 = (uint32_t)(data >> 32);
			LOGMASKED(LOG_REX3, "REX3 Red/CI Slope Write: %08x\n", data32);
			data32 &= 0x807fffff;
			int32_t temp = 0;
			if (BIT(data32, 31))
			{
				temp  = 0x00800000 - (data32 & 0x7fffff);
				temp |= 0x00800000;
			}
			else
			{
				temp = data32 & 0x7fffff;
			}
			m_rex3.m_slope_red = temp;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Alpha Slope Write: %08x\n", (uint32_t)data);
			data &= 0x8007ffff;
			int32_t temp = 0;
			if (BIT(data, 31))
			{
				temp  = 0x00080000 - (data & 0x7ffff);
				temp |= 0x00080000;
			}
			else
			{
				temp = data & 0x7ffff;
			}
			m_rex3.m_slope_alpha = temp;
		}
		break;
	}
	case 0x0218/8:
	{
		if (ACCESSING_BITS_32_63)
		{
			uint32_t data32 = (uint32_t)(data >> 32);
			LOGMASKED(LOG_REX3, "REX3 Green Slope Write: %08x\n", data32);
			data32 &= 0x8007ffff;
			int32_t temp = 0;
			if (BIT(data32, 31))
			{
				temp  = 0x00080000 - (data32 & 0x7ffff);
				temp |= 0x00080000;
			}
			else
			{
				temp = data32 & 0x7ffff;
			}
			m_rex3.m_slope_green = temp;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Blue Slope Write: %08x\n", (uint32_t)data);
			data &= 0x8007ffff;
			int32_t temp = 0;
			if (BIT(data, 31))
			{
				temp  = 0x00080000 - (data & 0x7ffff);
				temp |= 0x00080000;
			}
			else
			{
				temp = data & 0x7ffff;
			}
			m_rex3.m_slope_blue = temp;
		}
		break;
	}
	case 0x0220/8:
		if (ACCESSING_BITS_32_63)
		{
			const uint32_t data32 = (uint32_t)(data >> 32);
			LOGMASKED(LOG_REX3, "REX3 Write Mask Write: %08x\n", data32);
			m_rex3.m_write_mask = data32 & 0xffffff;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Packed Color Fractions Write: %08x\n", (uint32_t)data);
			m_rex3.m_color_i = (uint32_t)data;
			m_rex3.m_color_red = m_rex3.m_color_i << 11;
		}
		break;
	case 0x0228/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Color Index Zeros Overflow Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_zero_overflow = (uint32_t)(data >> 32);
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Red/CI Slope (copy) Write: %08x\n", (uint32_t)data);
			m_rex3.m_slope_red = (uint32_t)data;
		}
		break;
	case 0x0230/8:
		LOGMASKED(LOG_REX3, "REX3 Host Data Port Write: %08x%08x & %08x%08x\n", (uint32_t)(data >> 32), (uint32_t)data, (uint64_t)(mem_mask >> 32), (uint32_t)mem_mask);
		COMBINE_DATA(&m_rex3.m_host_dataport);
		break;
	case 0x0238/8:
		if (ACCESSING_BITS_32_63)
		{
			data >>= 32;
			LOGMASKED(LOG_REX3, "REX3 Display Control Bus Mode Write: %08x\n", (uint32_t)data);
			switch (data & 3)
			{
			case 0x00:
				LOGMASKED(LOG_REX3, "    Transfer Width:     4 bytes\n");
				m_rex3.m_xfer_width = 4;
				break;
			case 0x01:
				LOGMASKED(LOG_REX3, "    Transfer Width:     1 bytes\n");
				m_rex3.m_xfer_width = 1;
				break;
			case 0x02:
				LOGMASKED(LOG_REX3, "    Transfer Width:     2 bytes\n");
				m_rex3.m_xfer_width = 2;
				break;
			case 0x03:
				LOGMASKED(LOG_REX3, "    Transfer Width:     3 bytes\n");
				m_rex3.m_xfer_width = 3;
				break;
			}
			LOGMASKED(LOG_REX3, "   Enable Data Packing: %d\n", BIT(data, 2));
			LOGMASKED(LOG_REX3, " Enable Auto-Increment: %d\n", BIT(data, 3));
			LOGMASKED(LOG_REX3, "    DCB Reg Select Adr: %d\n", (data & 0x00000070 ) >> 4);
			LOGMASKED(LOG_REX3, "     DCB Slave Address: %d\n", (data & 0x00000780 ) >> 7);
			LOGMASKED(LOG_REX3, "    Use Sync XFer ACK:  %d\n", (data & 0x00000800 ) >> 11);
			LOGMASKED(LOG_REX3, "    Use Async XFer ACK: %d\n", (data & 0x00001000 ) >> 12);
			LOGMASKED(LOG_REX3, "   GIO CLK Cycle Width: %d\n", (data & 0x0003e000 ) >> 13);
			LOGMASKED(LOG_REX3, "    GIO CLK Cycle Hold: %d\n", (data & 0x007c0000 ) >> 18);
			LOGMASKED(LOG_REX3, "   GIO CLK Cycle Setup: %d\n", (data & 0x0f800000 ) >> 23);
			LOGMASKED(LOG_REX3, "    Swap Byte Ordering: %d\n", (data & 0x10000000 ) >> 28);
			m_rex3.m_dcb_reg_select = (data & 0x00000070) >> 4;
			m_rex3.m_dcb_slave_select = (data & 0x00000780) >> 7;
			m_rex3.m_dcb_mode = data & 0x1fffffff;
		}
		break;
	case 0x0240/8:
		if (ACCESSING_BITS_32_63)
		{
			const uint32_t data32 = (uint32_t)(data >> 32);
			m_rex3.m_dcb_data_msw = data32;
			switch (m_rex3.m_dcb_slave_select)
			{
			case 0x00:
				vc2_write(data32);
				break;
			case 0x01:
				cmap0_write(data32);
				break;
			case 0x04:
				xmap0_write(data32);
				xmap1_write(data32);
				break;
			case 0x05:
				xmap0_write(data32);
				break;
			case 0x06:
				xmap1_write(data32);
				break;
			default:
				LOGMASKED(LOG_REX3 | LOG_UNKNOWN, "REX3 Display Control Bus Data MSW Write: %08x\n", data32);
				break;
			}
			if (BIT(m_rex3.m_dcb_mode, 3))
			{
				m_rex3.m_dcb_reg_select++;
				m_rex3.m_dcb_reg_select &= 7;
			}
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Display Control Bus Data LSW Write: %08x\n", (uint32_t)data);
			m_rex3.m_dcb_data_lsw = (uint32_t)data;
		}
		break;
	case 0x1300/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 1 X Min/Max Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_smask_x[1] = (uint32_t)(data >> 32);
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 1 Y Min/Max Write: %08x\n", (uint32_t)data);
			m_rex3.m_smask_y[1] = (uint32_t)data;
		}
		break;
	case 0x1308/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 2 X Min/Max Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_smask_x[2] = (uint32_t)(data >> 32);
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 2 Y Min/Max Write: %08x\n", (uint32_t)data);
			m_rex3.m_smask_y[2] = (uint32_t)data;
		}
		break;
	case 0x1310/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 3 X Min/Max Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_smask_x[3] = (uint32_t)(data >> 32);
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 3 Y Min/Max Write: %08x\n", (uint32_t)data);
			m_rex3.m_smask_y[3] = (uint32_t)data;
		}
		break;
	case 0x1318/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 4 X Min/Max Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_smask_x[4] = (uint32_t)(data >> 32);
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Screenmask 4 Y Min/Max Write: %08x\n", (uint32_t)data);
			m_rex3.m_smask_y[4] = (uint32_t)data;
		}
		break;
	case 0x1320/8:
		if (ACCESSING_BITS_32_63)
		{
			const uint32_t data32 = (uint32_t)(data >> 32);
			LOGMASKED(LOG_REX3, "REX3 Top of Screen Scanline Write: %08x\n", data32);
			m_rex3.m_top_scanline = data32 & 0x3ff;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 XY Window Write: %08x\n", (uint32_t)data);
			m_rex3.m_xy_window = (uint32_t)data;
			m_rex3.m_x_window = (int16_t)(m_rex3.m_xy_window >> 16);
			m_rex3.m_y_window = (int16_t)m_rex3.m_xy_window;
		}
		break;
	case 0x1328/8:
		if (ACCESSING_BITS_32_63)
		{
			const uint32_t data32 = (uint32_t)(data >> 32);
			LOGMASKED(LOG_REX3, "REX3 Clipping Mode Write: %08x\n", data32);
			m_rex3.m_clip_mode = data32 & 0x1fff;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "Request GFIFO Stall\n");
		}
		break;
	case 0x1330/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Config Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_config = (data >> 32) & 0x1fffff;
		}
		break;
	case 0x1340/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "Reset DCB Bus and Flush BFIFO\n");
		}
		break;
	default:
		LOGMASKED(LOG_REX3 | LOG_UNKNOWN, "Unknown REX3 Write: %08x (%08x): %08x\n", 0xbf0f0000 + (offset << 2), mem_mask, data);
		break;
	}

	if (offset & 0x00000100)
	{
		do_rex3_command();
	}
}
