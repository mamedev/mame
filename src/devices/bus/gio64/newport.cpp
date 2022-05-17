// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

    newport.cpp

    SGI "Newport" graphics board emulation

    Newport is modular, consisting of the following custom chips:
    - REX3: Raster Engine, essentially a blitter which can also draw
            antialiased lines. REX also acts as the interface to the
            rest of the system - all the other chips on a Newport
            board are accessed through it.
    - RB2: Frame buffer input controller
    - RO1: Frame buffer output controller
    - XMAP9: Final display generator
    - CMAP: Palette mapper
    - VC2: Video timing controller / CRTC

    Newport-derived graphics boards were shipped for the Indy
    workstation and some Indigo 2 workstations.

    Known Issues:
    - Antialiased line drawing currently uses the fractional line
      drawing routine instead.
    - Timing-wise, all draw commands complete instantly and all
      FIFOs are always empty. Thus the performance of an emulated
      Newport board is likely to be better than real life in scenes
      that would otherwise be drawing-bound.
    - The REX3 emulation currently handles a bunch of functionality
      that should be the responsibility of RO1 and RB2. However,
      emulating it properly would bring down the emulation speed
      for not much practical benefit.

*********************************************************************/

#include "emu.h"
#include "newport.h"

#define LOG_UNKNOWN     (1 << 0)
#define LOG_VC2         (1 << 1)
#define LOG_CMAP        (1 << 2)
#define LOG_XMAP        (1 << 4)
#define LOG_REX3        (1 << 6)
#define LOG_RAMDAC      (1 << 7)
#define LOG_COMMANDS    (1 << 8)
#define LOG_REJECTS     (1 << 9)
#define LOG_ALL         (LOG_UNKNOWN | LOG_VC2 | LOG_CMAP | LOG_XMAP | LOG_REX3 | LOG_RAMDAC | LOG_COMMANDS | LOG_REJECTS)

#define VERBOSE         (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XMAP9,      xmap9_device,      "xmap9",      "SGI XMAP9")
DEFINE_DEVICE_TYPE(CMAP,       cmap_device,       "cmap",       "SGI CMAP")
DEFINE_DEVICE_TYPE(VC2,        vc2_device,        "vc2",        "SGI VC2")
DEFINE_DEVICE_TYPE(RB2,        rb2_device,        "rb2",        "SGI RB2")
DEFINE_DEVICE_TYPE(GIO64_XL8,  gio64_xl8_device,  "gio64_xl8",  "SGI 8-bit XL board")
DEFINE_DEVICE_TYPE(GIO64_XL24, gio64_xl24_device, "gio64_xl24", "SGI 24-bit XL board")

/*************************************
 *
 *  XMAP9 Device
 *
 *************************************/

xmap9_device::xmap9_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XMAP9, tag, owner, clock)
{
}

void xmap9_device::device_start()
{
	save_item(NAME(m_config));
	save_item(NAME(m_revision));
	save_item(NAME(m_fifo_available));
	save_item(NAME(m_cursor_cmap));
	save_item(NAME(m_popup_cmap));
	save_item(NAME(m_mode_table_idx));
	save_item(NAME(m_mode_table));
}

void xmap9_device::device_reset()
{
	m_config = 0;
	m_fifo_available = 0x2;
	m_cursor_cmap = 0;
	m_popup_cmap = 0;
	m_mode_table_idx = 0;
	std::fill(std::begin(m_mode_table), std::end(m_mode_table), 0);
}

void xmap9_device::serialize(FILE *file)
{
	fwrite(&m_config, sizeof(uint32_t), 1, file);
	fwrite(&m_revision, sizeof(uint32_t), 1, file);
	fwrite(&m_fifo_available, sizeof(uint32_t), 1, file);
	fwrite(&m_cursor_cmap, sizeof(uint32_t), 1, file);
	fwrite(&m_popup_cmap, sizeof(uint32_t), 1, file);
	fwrite(&m_mode_table_idx, sizeof(uint32_t), 1, file);
	fwrite(m_mode_table, sizeof(uint32_t), 0x20, file);
}

void xmap9_device::deserialize(FILE *file)
{
	fread(&m_config, sizeof(uint32_t), 1, file);
	fread(&m_revision, sizeof(uint32_t), 1, file);
	fread(&m_fifo_available, sizeof(uint32_t), 1, file);
	fread(&m_cursor_cmap, sizeof(uint32_t), 1, file);
	fread(&m_popup_cmap, sizeof(uint32_t), 1, file);
	fread(&m_mode_table_idx, sizeof(uint32_t), 1, file);
	fread(&m_mode_table, sizeof(uint32_t), 0x20, file);
}

uint32_t xmap9_device::read(uint32_t offset)
{
	switch (offset)
	{
	case 0:
		LOGMASKED(LOG_XMAP, "Config Read: %08x\n", m_config);
		return m_config;
	case 1:
		LOGMASKED(LOG_XMAP, "Revision Read: %08x\n", m_revision);
		return m_revision;
	case 2:
		LOGMASKED(LOG_XMAP, "FIFO Availability Read: %08x\n", m_fifo_available);
		return m_fifo_available;
	case 3:
		LOGMASKED(LOG_XMAP, "Cursor CMAP MSB Read: %08x\n", m_cursor_cmap);
		return m_cursor_cmap;
	case 4:
		LOGMASKED(LOG_XMAP, "Pop Up CMAP MSB Read: %08x\n", m_popup_cmap);
		return m_popup_cmap;
	case 5:
	{
		const uint8_t mode_idx = (m_mode_table_idx & 0x7c) >> 2;
		switch (m_mode_table_idx & 3)
		{
		case 0:
		{
			const uint8_t ret = (uint8_t)(m_mode_table[mode_idx] >> 16);
			LOGMASKED(LOG_XMAP, "Mode Register Read: %02x (Byte 0): %08x\n", mode_idx, ret);
			return ret;
		}
		case 1:
		{
			const uint8_t ret = (uint8_t)(m_mode_table[mode_idx] >> 8);
			LOGMASKED(LOG_XMAP, "Mode Register Read: %02x (Byte 1): %08x\n", mode_idx, ret);
			return ret;
		}
		case 2:
		{
			const uint8_t ret = (uint8_t)m_mode_table[mode_idx];
			LOGMASKED(LOG_XMAP, "Mode Register Read: %02x (Byte 2): %08x\n", mode_idx, ret);
			return ret;
		}
		}
		break;
	}
	case 6:
		LOGMASKED(LOG_XMAP, "Unused Read: %08x\n", 0);
		return 0;
	case 7:
		LOGMASKED(LOG_XMAP, "Mode Table Address Read: %08x\n", m_mode_table_idx);
		return m_mode_table_idx;
	default:
		LOGMASKED(LOG_XMAP | LOG_UNKNOWN, "Unknown DCB Register Select Value: %02x, returning 0\n", offset);
		return 0;
	}
	return 0;
}

void xmap9_device::write(uint32_t offset, uint32_t data)
{
	switch (offset)
	{
	case 0:
		LOGMASKED(LOG_XMAP, "Config Write: %02x\n", (uint8_t)data);
		m_config = (uint8_t)data;
		break;
	case 1:
		LOGMASKED(LOG_XMAP, "Revision Write (Ignored): %02x\n", (uint8_t)data);
		break;
	case 2:
		LOGMASKED(LOG_XMAP, "FIFO Availability Write (Ignored): %02x\n", (uint8_t)data);
		break;
	case 3:
		LOGMASKED(LOG_XMAP, "Cursor CMAP MSB Write: %02x\n", (uint8_t)data);
		m_cursor_cmap = (uint8_t)data;
		break;
	case 4:
		LOGMASKED(LOG_XMAP, "Pop Up CMAP MSB Write: %02x\n", (uint8_t)data);
		m_popup_cmap = (uint8_t)data;
		break;
	case 5:
		LOGMASKED(LOG_XMAP, "Mode Register Write: %02x = %06x\n", data >> 24, data & 0xffffff);
		m_mode_table[data >> 24] = data & 0xffffff;
		break;
	case 6:
		LOGMASKED(LOG_XMAP, "Unused Write (Ignored): %08x\n", data);
		break;
	case 7:
		LOGMASKED(LOG_XMAP, "Mode Table Address Write: %02x\n", (uint8_t)data);
		m_mode_table_idx = (uint8_t)data;
		break;
	default:
		LOGMASKED(LOG_XMAP | LOG_UNKNOWN, "Unknown DCB Register Select Value: %02x = %08x\n", offset, data);
		break;
	}
}


/*************************************
 *
 *  CMAP Device
 *
 *************************************/

cmap_device::cmap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CMAP, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
{
}

void cmap_device::device_start()
{
	save_item(NAME(m_status));
	save_item(NAME(m_revision));
	save_item(NAME(m_palette_idx));
	save_item(NAME(m_palette));
}

void cmap_device::device_reset()
{
	m_status = 8;
	m_palette_idx = 0;
	std::fill(std::begin(m_palette), std::end(m_palette), 0);
}

void cmap_device::serialize(FILE *file)
{
	fwrite(&m_status, sizeof(uint32_t), 1, file);
	fwrite(&m_revision, sizeof(uint32_t), 1, file);
	fwrite(&m_palette_idx, sizeof(uint16_t), 1, file);
	fwrite(m_palette, sizeof(uint32_t), 0x10000, file);
}

void cmap_device::deserialize(FILE *file)
{
	fread(&m_status, sizeof(uint32_t), 1, file);
	fread(&m_revision, sizeof(uint32_t), 1, file);
	fread(&m_palette_idx, sizeof(uint16_t), 1, file);
	fread(m_palette, sizeof(uint32_t), 0x10000, file);
}

void cmap_device::write(uint32_t offset, uint32_t data)
{
	switch (offset)
	{
	case 0x00:
		LOGMASKED(LOG_CMAP, "Palette Index Write: %04x\n", data & 0xffff);
		m_palette_idx = (uint16_t)data;
		break;
	case 0x02:
		m_palette[m_palette_idx] = data >> 8;
		if (m_palette_idx < 0x2000)
			set_pen_color(m_palette_idx, rgb_t((uint8_t)(data >> 24), (uint8_t)(data >> 16), (uint8_t)(data >> 8)));
		LOGMASKED(LOG_CMAP, "Palette Entry %04x Write: %08x\n", m_palette_idx, data >> 8);
		break;
	default:
		LOGMASKED(LOG_CMAP | LOG_UNKNOWN, "Unknown Register %d Write: %08x\n", offset, data);
		break;
	}
}

uint32_t cmap_device::read(uint32_t offset)
{
	switch (offset)
	{
	case 0x04:
		LOGMASKED(LOG_CMAP, "Status Read: %08x\n", m_status);
		return m_status;
	case 0x06:
		LOGMASKED(LOG_CMAP, "Revision Read: %08x\n", m_revision);
		return m_revision;
	default:
		LOGMASKED(LOG_CMAP | LOG_UNKNOWN, "Unknown Register %d Read\n", offset);
		return 0;
	}
}


/*************************************
 *
 *  VC2 Device
 *
 *************************************/

/*static*/ const size_t vc2_device::RAM_SIZE = 0x8000;

vc2_device::vc2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VC2, tag, owner, clock)
	, m_vert_int(*this)
	, m_screen_timing_changed(*this)
{
}

void vc2_device::device_start()
{
	m_vert_int.resolve_safe();
	m_screen_timing_changed.resolve_safe();

	m_ram = std::make_unique<uint16_t[]>(RAM_SIZE);
	m_vt_table = make_unique_clear<uint32_t[]>(2048 * 2048);

	save_item(NAME(m_vid_entry));
	save_item(NAME(m_cursor_entry));
	save_item(NAME(m_cursor_x));
	save_item(NAME(m_cursor_y));
	save_item(NAME(m_cur_cursor_x));
	save_item(NAME(m_did_entry));
	save_item(NAME(m_scanline_len));
	save_item(NAME(m_ram_addr));
	save_item(NAME(m_vt_frame_ptr));
	save_item(NAME(m_vt_line_ptr));
	save_item(NAME(m_vt_line_run));
	save_item(NAME(m_vt_line_count));
	save_item(NAME(m_cursor_table_ptr));
	save_item(NAME(m_work_cursor_y));
	save_item(NAME(m_did_frame_ptr));
	save_item(NAME(m_did_line_ptr));
	save_item(NAME(m_display_ctrl));
	save_item(NAME(m_config));
	save_item(NAME(m_reg_idx));
	save_item(NAME(m_reg_data));
	save_pointer(NAME(&m_ram[0]), RAM_SIZE);

	save_pointer(NAME(&m_vt_table[0]), 2048 * 2048);

	save_item(NAME(m_readout_x0));
	save_item(NAME(m_readout_y0));
	save_item(NAME(m_readout_x1));
	save_item(NAME(m_readout_y1));

	save_item(NAME(m_enable_cursor));
}

void vc2_device::device_reset()
{
	m_vid_entry = 0;
	m_cursor_entry = 0;
	m_cursor_x = 0;
	m_cursor_y = 0;
	m_cur_cursor_x = 0;
	m_did_entry = 0;
	m_scanline_len = 0;
	m_ram_addr = 0;
	m_vt_frame_ptr = 0;
	m_vt_line_ptr = 0;
	m_vt_line_run = 0;
	m_vt_line_count = 0;
	m_cursor_table_ptr = 0;
	m_work_cursor_y = 0;
	m_did_frame_ptr = 0;
	m_did_line_ptr = 0;
	m_display_ctrl = 0;
	m_config = 0;
	m_reg_idx = 0;
	m_reg_data = 0;
	memset(&m_ram[0], 0, sizeof(uint16_t) * RAM_SIZE);

	memset(&m_vt_table[0], 0, sizeof(uint32_t) * 2048 * 2048);

	m_readout_x0 = 0;
	m_readout_y0 = 0;
	m_readout_x1 = 0;
	m_readout_y1 = 0;

	m_enable_cursor = false;
}

void vc2_device::serialize(FILE *file)
{
	fwrite(&m_vid_entry, sizeof(uint16_t), 1, file);
	fwrite(&m_cursor_entry, sizeof(uint16_t), 1, file);
	fwrite(&m_cursor_x, sizeof(uint16_t), 1, file);
	fwrite(&m_cursor_y, sizeof(uint16_t), 1, file);
	fwrite(&m_cur_cursor_x, sizeof(uint16_t), 1, file);
	fwrite(&m_did_entry, sizeof(uint16_t), 1, file);
	fwrite(&m_scanline_len, sizeof(uint16_t), 1, file);
	fwrite(&m_ram_addr, sizeof(uint16_t), 1, file);
	fwrite(&m_vt_frame_ptr, sizeof(uint16_t), 1, file);
	fwrite(&m_vt_line_ptr, sizeof(uint16_t), 1, file);
	fwrite(&m_vt_line_run, sizeof(uint16_t), 1, file);
	fwrite(&m_vt_line_count, sizeof(uint16_t), 1, file);
	fwrite(&m_cursor_table_ptr, sizeof(uint16_t), 1, file);
	fwrite(&m_work_cursor_y, sizeof(uint16_t), 1, file);
	fwrite(&m_did_frame_ptr, sizeof(uint16_t), 1, file);
	fwrite(&m_did_line_ptr, sizeof(uint16_t), 1, file);
	fwrite(&m_display_ctrl, sizeof(uint16_t), 1, file);
	fwrite(&m_config, sizeof(uint16_t), 1, file);
	fwrite(&m_reg_idx, sizeof(uint8_t), 1, file);
	fwrite(&m_ram[0], sizeof(uint16_t), RAM_SIZE, file);
	fwrite(&m_vt_table[0], sizeof(uint32_t), 2048 * 2048, file);
	fwrite(&m_readout_x0, sizeof(int), 1, file);
	fwrite(&m_readout_y0, sizeof(int), 1, file);
	fwrite(&m_readout_x1, sizeof(int), 1, file);
	fwrite(&m_readout_y1, sizeof(int), 1, file);
	fwrite(&m_enable_cursor, sizeof(bool), 1, file);
}

void vc2_device::deserialize(FILE *file)
{
	fread(&m_vid_entry, sizeof(uint16_t), 1, file);
	fread(&m_cursor_entry, sizeof(uint16_t), 1, file);
	fread(&m_cursor_x, sizeof(uint16_t), 1, file);
	fread(&m_cursor_y, sizeof(uint16_t), 1, file);
	fread(&m_cur_cursor_x, sizeof(uint16_t), 1, file);
	fread(&m_did_entry, sizeof(uint16_t), 1, file);
	fread(&m_scanline_len, sizeof(uint16_t), 1, file);
	fread(&m_ram_addr, sizeof(uint16_t), 1, file);
	fread(&m_vt_frame_ptr, sizeof(uint16_t), 1, file);
	fread(&m_vt_line_ptr, sizeof(uint16_t), 1, file);
	fread(&m_vt_line_run, sizeof(uint16_t), 1, file);
	fread(&m_vt_line_count, sizeof(uint16_t), 1, file);
	fread(&m_cursor_table_ptr, sizeof(uint16_t), 1, file);
	fread(&m_work_cursor_y, sizeof(uint16_t), 1, file);
	fread(&m_did_frame_ptr, sizeof(uint16_t), 1, file);
	fread(&m_did_line_ptr, sizeof(uint16_t), 1, file);
	fread(&m_display_ctrl, sizeof(uint16_t), 1, file);
	fread(&m_config, sizeof(uint16_t), 1, file);
	fread(&m_reg_idx, sizeof(uint8_t), 1, file);
	fread(&m_ram[0], sizeof(uint16_t), RAM_SIZE, file);
	fread(&m_vt_table[0], sizeof(uint32_t), 2048 * 2048, file);
	fread(&m_readout_x0, sizeof(int), 1, file);
	fread(&m_readout_y0, sizeof(int), 1, file);
	fread(&m_readout_x1, sizeof(int), 1, file);
	fread(&m_readout_y1, sizeof(int), 1, file);
	fread(&m_enable_cursor, sizeof(bool), 1, file);
}

void vc2_device::write(uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	if (mem_mask == 0x000000ff)
	{
		// Register Select
		if (offset == 0)
		{
			m_reg_idx = (uint8_t)data;
			LOGMASKED(LOG_VC2, "Register Select: %02x\n", m_reg_idx);
		}
		else
		{
			LOGMASKED(LOG_VC2 | LOG_UNKNOWN, "Unknown Register Select: Register %02x, data = %08x\n", offset, data);
		}
	}
	else if (mem_mask == 0x0000ffff)
	{
		// RAM Word
		if (offset == 3)
		{
			LOGMASKED(LOG_VC2, "RAM Write: %04x = %08x\n", m_ram_addr, (uint16_t)data);
			m_ram[m_ram_addr] = (uint16_t)data;
			m_ram_addr++;
			if (m_ram_addr >= RAM_SIZE)
			{
				m_ram_addr = 0;
			}
		}
		else
		{
			LOGMASKED(LOG_VC2 | LOG_UNKNOWN, "Unknown Word Write: Register %02x, data = %08x\n", offset, data);
		}
	}
	else if (mem_mask == 0xffffffff)
	{
		// Register Write
		if (offset == 0)
		{
			LOGMASKED(LOG_VC2, "Register Setup:\n");
			m_reg_idx = data >> 24;
			m_reg_data = (uint16_t)(data >> 8);
			switch (m_reg_idx)
			{
				case 0x00:
					m_vid_entry = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: Video Entry Pointer, %04x\n", m_vid_entry);
					update_screen_size();
					break;
				case 0x01:
					m_cursor_entry = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: Cursor Entry Pointer, %04x\n", m_cursor_entry);
					break;
				case 0x02:
					m_cursor_x = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: Cursor X, %04x\n", m_cursor_x);
					break;
				case 0x03:
					m_cursor_y = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: Cursor Y, %04x\n", m_cursor_y);
					m_cur_cursor_x = m_cursor_x;
					break;
				case 0x04:
					m_cur_cursor_x = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: Current Cursor X, %04x\n", m_cur_cursor_x);
					break;
				case 0x05:
					m_did_entry = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: DID Entry Pointer, %04x\n", m_did_entry);
					break;
				case 0x06:
					m_scanline_len = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: Scanline Length, %04x\n", m_scanline_len);
					break;
				case 0x07:
					m_ram_addr = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: RAM Address, %04x\n", m_ram_addr);
					break;
				case 0x08:
					m_vt_frame_ptr = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: VT Frame Table Ptr, %04x\n", m_vt_frame_ptr);
					break;
				case 0x09:
					m_vt_line_ptr = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: VT Line Sequence Pointer, %04x\n", m_vt_line_ptr);
					break;
				case 0x0a:
					m_vt_line_run = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: VT Lines in Run, %04x\n", m_vt_line_run);
					break;
				case 0x0b:
					m_vt_line_count = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: Vertical Line Count, %04x\n", m_vt_line_count);
					break;
				case 0x0c:
					m_cursor_table_ptr = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: Cursor Table Pointer, %04x\n", m_cursor_table_ptr);
					break;
				case 0x0d:
					m_work_cursor_y = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: Working Cursor Y, %04x\n", m_work_cursor_y);
					break;
				case 0x0e:
					m_did_frame_ptr = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: DID Frame Table Pointer, %04x\n", m_did_frame_ptr);
					break;
				case 0x0f:
					m_did_line_ptr = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: DID Line Table Pointer, %04x\n", m_did_line_ptr);
					break;
				case 0x10:
					m_display_ctrl = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: Display Control, %04x\n", m_display_ctrl);
					m_enable_cursor = BIT(m_display_ctrl, CURSOR_FUNC_ENABLE_BIT) != 0
								   && BIT(m_display_ctrl, CURSOR_ENABLE_BIT) != 0
								   && BIT(m_display_ctrl, CURSOR_MODE_BIT) == CURSOR_MODE_GLYPH;
					break;
				case 0x1f:
					m_config = m_reg_data;
					LOGMASKED(LOG_VC2, "Register Write: Configuration, %04x\n", m_config);
					break;
				default:
					LOGMASKED(LOG_VC2 | LOG_UNKNOWN, "Register Write: Unknown Register: %02x = %04x\n", m_reg_idx, m_reg_data);
					break;
			}
		}
		else
		{
			LOGMASKED(LOG_VC2 | LOG_UNKNOWN, "Unknown Register Write: %02x = %08x\n", offset, data);
		}
	}
	else
	{
		LOGMASKED(LOG_VC2 | LOG_UNKNOWN, "Unknown Memory Mask: %08x, Offset %02x, Value %08x\n", mem_mask, offset, data);
	}
}

uint32_t vc2_device::read(uint32_t offset)
{
	switch (offset)
	{
	case 0x01: /* Register Read */
		switch (m_reg_idx)
		{
			case 0x00:
				LOGMASKED(LOG_VC2, "Register Read: Video Entry Pointer, %08x\n", m_vid_entry);
				return m_vid_entry;
			case 0x01:
				LOGMASKED(LOG_VC2, "Register Read: Cursor Entry Pointer, %08x\n", m_cursor_entry);
				return m_cursor_entry;
			case 0x02:
				LOGMASKED(LOG_VC2, "Register Read: Cursor X, %08x\n", m_cursor_x);
				return m_cursor_x;
			case 0x03:
				LOGMASKED(LOG_VC2, "Register Read: Cursor Y, %08x\n", m_cursor_y);
				return m_cursor_y;
			case 0x04:
				LOGMASKED(LOG_VC2, "Register Read: Current Cursor X, %08x\n", m_cur_cursor_x);
				return m_cur_cursor_x;
			case 0x05:
				LOGMASKED(LOG_VC2, "Register Read: DID Entry, %08x\n", m_did_entry);
				return m_did_entry;
			case 0x06:
				LOGMASKED(LOG_VC2, "Register Read: Scanline Length, %08x\n", m_scanline_len);
				return m_scanline_len;
			case 0x07:
				LOGMASKED(LOG_VC2, "Register Read: RAM Address, %08x\n", m_ram_addr);
				return m_ram_addr;
			case 0x08:
				LOGMASKED(LOG_VC2, "Register Read: VT Frame Pointer, %08x\n", m_vt_frame_ptr);
				return m_vt_frame_ptr;
			case 0x09:
				LOGMASKED(LOG_VC2, "Register Read: VT Line Sequence Pointer, %08x\n", m_vt_line_ptr);
				return m_vt_line_ptr;
			case 0x0a:
				LOGMASKED(LOG_VC2, "Register Read: VT Lines in Run, %08x\n", m_vt_line_run);
				return m_vt_line_run;
			case 0x0b:
				LOGMASKED(LOG_VC2, "Register Read: Vertical Line Count, %08x\n", m_vt_line_count);
				return m_vt_line_count;
			case 0x0c:
				LOGMASKED(LOG_VC2, "Register Read: Cursor Table Pointer, %08x\n", m_cursor_table_ptr);
				return m_cursor_table_ptr;
			case 0x0d:
				LOGMASKED(LOG_VC2, "Register Read: Working Cursor Y, %08x\n", m_work_cursor_y);
				return m_work_cursor_y;
			case 0x0e:
				LOGMASKED(LOG_VC2, "Register Read: DID Frame Pointer, %08x\n", m_did_frame_ptr);
				return m_did_frame_ptr;
			case 0x0f:
				LOGMASKED(LOG_VC2, "Register Read: DID Line Pointer, %08x\n", m_did_line_ptr);
				return m_did_line_ptr;
			case 0x10:
				LOGMASKED(LOG_VC2, "Register Read: Display Control, %08x\n", m_display_ctrl);
				return m_display_ctrl;
			case 0x1f:
				LOGMASKED(LOG_VC2, "Register Read: Configuration, %08x\n", m_config);
				return m_config;
			default:
				return 0;
		}
		break;
	case 0x03: /* RAM Read */
	{
		LOGMASKED(LOG_VC2, "RAM Read: %04x = %08x\n", m_ram_addr, m_ram[m_ram_addr]);
		uint16_t ret = m_ram[m_ram_addr];
		m_ram_addr++;
		if (m_ram_addr >= RAM_SIZE)
		{
			m_ram_addr = 0;
		}
		return ret;
	}
	default:
		LOGMASKED(LOG_VC2 | LOG_UNKNOWN, "Unknown Register Read: %02x\n", offset);
		return 0;
	}
}

uint16_t vc2_device::begin_did_line(int y)
{
	m_did_frame_ptr = m_did_entry + (uint16_t)y;
	m_did_line_ptr = m_ram[m_did_frame_ptr];
	return m_ram[m_did_line_ptr];
}

uint16_t vc2_device::next_did_line_entry()
{
	m_did_line_ptr++;
	return m_ram[m_did_line_ptr];
}

void vc2_device::decode_vt_line(uint32_t line, uint32_t line_seq_ptr)
{
	bool eol_sa = false;
	bool eol_sb = false;
	uint32_t vt_entry = 0;
	uint32_t i = 0;
	do
	{
		vt_entry &= ~(0x7f << 14);
		vt_entry |= (m_ram[line_seq_ptr] & 0x7f) << 14;
		eol_sa = BIT(m_ram[line_seq_ptr], 15);
		if (!BIT(m_ram[line_seq_ptr], 7))
		{
			eol_sb = BIT(m_ram[line_seq_ptr + 1], 15);
			vt_entry &= (0x7f << 14);
			vt_entry |= (m_ram[line_seq_ptr + 1] & 0x7f00) >> 1;
			vt_entry |= m_ram[line_seq_ptr + 1] & 0x7f;
			line_seq_ptr += 2;
		}
		else
		{
			eol_sb = true;
			line_seq_ptr++;
		}
		for (uint32_t j = 0; j < ((m_ram[line_seq_ptr] & 0x7f00) >> 7); i++, j++)
		{
			const uint8_t sa = ((vt_entry >> 14) & 0x7f) << 1;
			const uint8_t sb = ((vt_entry >> 7) & 0x7f) << 1;
			const uint8_t sc = ((vt_entry >> 0) & 0x7f) << 1;
			m_vt_table[line * 2048 + i] = 0xff000000 | (sa << 16) | (sb << 8) | sc;
		}
	} while (!(eol_sa && eol_sb));
}

void vc2_device::decode_vt_table()
{
	memset(&m_vt_table[0], 0, sizeof(uint32_t) * 2048 * 2048);
	uint32_t curr_vt_entry = m_vid_entry;
	uint32_t line_counter = 0;
	uint32_t line_seq_len = 0;
	do
	{
		const uint32_t line_seq_ptr = m_ram[curr_vt_entry];
		line_seq_len = m_ram[curr_vt_entry + 1];
		if (line_seq_len)
		{
			for (uint32_t i = 0; i < line_seq_len; i++, line_counter++)
			{
				decode_vt_line(line_counter, line_seq_ptr);
			}
		}
		curr_vt_entry += 2;
	} while (line_seq_len != 0);
}

void vc2_device::update_screen_size()
{
	decode_vt_table();

	bool x_started = false;
	bool y_started = false;
	bool x_done = false;
	bool y_done = false;
	bool done = false;
	m_readout_x0 = 0;
	m_readout_y0 = 0;
	m_readout_x1 = 0;
	m_readout_y1 = 0;

	for (int y = 0; y < 2048 && !done; y++)
	{
		uint32_t *src = &m_vt_table[y * 2048];
		for (int x = 0; x < 2048 && !done; x++)
		{
			if (BIT(*src, 7))
			{
				if (!x_started)
				{
					x_started = true;
					m_readout_x0 = x;
				}
				if (!y_started)
				{
					y_started = true;
					m_readout_y0 = y;
				}
			}
			else
			{
				if (x_started && !x_done)
				{
					m_readout_x1 = x;
					x_done = true;
				}
				if (y_started && !y_done && x == m_readout_x0)
				{
					m_readout_y1 = y;
					y_done = true;
				}
			}
			done = x_done && y_done;
			src++;
		}
	}

	m_screen_timing_changed(1);
}

bool vc2_device::is_cursor_active(int x, int y)
{
	return x >= (m_cursor_x - 31) && x <= m_cursor_x && y >= (m_cursor_y - 31) && y <= m_cursor_y && m_enable_cursor;
}

uint8_t vc2_device::get_cursor_pixel(int x, int y)
{
	if (!is_cursor_active(x, y))
		return 0;

	x -= (int)m_cursor_x - 31;
	y -= (int)m_cursor_y - 31;

	if (x < 0 || y < 0)
		return 0;

	bool monochrome_cursor = BIT(m_display_ctrl, CURSOR_SIZE_BIT) == CURSOR_SIZE_64;

	int size = monochrome_cursor ? 64 : 32;
	if (x >= size || y >= size)
		return 0;

	const int shift = 15 - (x % 16);

	if (monochrome_cursor)
	{
		const int address = y * 4 + (x / 16);
		const uint16_t word = m_ram[m_cursor_entry + address];
		return BIT(word, shift);
	}
	else
	{
		const int address = y * 2 + (x / 16);
		const uint16_t word0 = m_ram[m_cursor_entry + address];
		const uint16_t word1 = m_ram[m_cursor_entry + address + 64];
		return BIT(word0, shift) | (BIT(word1, shift) << 1);
	}
}

WRITE_LINE_MEMBER(vc2_device::vblank_w)
{
	if (state)
	{
		if (BIT(m_display_ctrl, 0))
		{
			m_vert_int(1);
		}
	}
}


/*************************************
 *
 *  RB2 Device
 *
 *************************************/

/*static*/ const size_t rb2_device::BUFFER_SIZE = (1280 + 64) * (1024 + 64);

rb2_device::rb2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, RB2, tag, owner, clock)
{
}

void rb2_device::device_start()
{
	m_rgbci = std::make_unique<uint32_t[]>(BUFFER_SIZE);
	m_cidaux = std::make_unique<uint32_t[]>(BUFFER_SIZE);

	save_item(NAME(m_global_mask));
	save_item(NAME(m_write_mask));
	save_item(NAME(m_blend));
	save_item(NAME(m_fast_clear));
	save_item(NAME(m_rgbmode));
	save_item(NAME(m_dblsrc));
	save_item(NAME(m_plane_enable));
	save_item(NAME(m_draw_depth));
	save_item(NAME(m_draw_bpp));
	save_item(NAME(m_logicop));
	save_item(NAME(m_src_shift));
	save_item(NAME(m_dst_shift));

	save_pointer(NAME(&m_rgbci[0]), BUFFER_SIZE);
	save_pointer(NAME(&m_cidaux[0]), BUFFER_SIZE);
}

void rb2_device::device_reset()
{
	m_write_mask = 0;
	m_blend = false;
	m_fast_clear = false;
	m_rgbmode = false;
	m_dblsrc = false;
	m_plane_enable = 0;
	m_draw_depth = 0;
	m_draw_bpp = 8;
	m_logicop = 0;
	m_src_shift = 0;
	m_dst_shift = 0;

	m_dest_buf = &m_rgbci[0];
	m_buf_ptr = &m_rgbci[0];
}

void rb2_device::serialize(FILE *file)
{
	fwrite(&m_global_mask, sizeof(uint32_t), 1, file);
	fwrite(&m_write_mask, sizeof(uint32_t), 1, file);
	fwrite(&m_blend, sizeof(bool), 1, file);
	fwrite(&m_fast_clear, sizeof(bool), 1, file);
	fwrite(&m_rgbmode, sizeof(bool), 1, file);
	fwrite(&m_dblsrc, sizeof(bool), 1, file);
	fwrite(&m_plane_enable, sizeof(uint8_t), 1, file);
	fwrite(&m_draw_depth, sizeof(uint8_t), 1, file);
	fwrite(&m_logicop, sizeof(uint8_t), 1, file);
	fwrite(&m_src_shift, sizeof(uint8_t), 1, file);
	fwrite(&m_dst_shift, sizeof(uint8_t), 1, file);
}

void rb2_device::deserialize(FILE *file)
{
	fread(&m_global_mask, sizeof(uint32_t), 1, file);
	fread(&m_write_mask, sizeof(uint32_t), 1, file);
	fread(&m_blend, sizeof(bool), 1, file);
	fread(&m_fast_clear, sizeof(bool), 1, file);
	fread(&m_rgbmode, sizeof(bool), 1, file);
	fread(&m_dblsrc, sizeof(bool), 1, file);
	fread(&m_plane_enable, sizeof(uint8_t), 1, file);
	fread(&m_draw_depth, sizeof(uint8_t), 1, file);
	fread(&m_logicop, sizeof(uint8_t), 1, file);
	fread(&m_src_shift, sizeof(uint8_t), 1, file);
	fread(&m_dst_shift, sizeof(uint8_t), 1, file);
}

uint32_t rb2_device::expand_to_all_lanes(uint32_t src)
{
	switch (m_draw_depth) {
	case 0:
		src |= src << 4;
		src |= src << 8;
		src |= src << 16;
		break;
	case 1:
		src |= src << 8;
		src |= src << 16;
		break;
	case 2:
		src |= src << 12;
		break;
	case 3:
		break;
	}
	switch (m_plane_enable)
	{
	case 1: // RGB/CI
	case 2: // RGBA
	case 6: // CID
		return src;
	case 4: // OLAY
		return src << 8;
	case 5: // PUP
		return src << 2;
	}
	return src;
}

void rb2_device::set_write_mask(uint32_t data)
{
	m_write_mask = data;
}

void rb2_device::set_flags(uint16_t data)
{
	m_blend = BIT(data, 12);
	m_fast_clear = BIT(data, 11);
	m_rgbmode = BIT(data, 10);
	m_dblsrc = BIT(data, 9);
	m_plane_enable = (data & 0x1c0) >> 6;
	m_draw_depth = (data & 0x30) >> 4;
	m_logicop = data & 0xf;

	static const uint32_t s_draw_bpp[4] = { 4, 8, 12, 24 };
	static const uint32_t s_store_shift[8][4][2] = {
		{   { 0,  0 }, // None, 4bpp, Buffer 0/1
			{ 0,  0 }, // None, 8bpp, Buffer 0/1
			{ 0,  0 }, // None, 12bpp, Buffer 0/1
			{ 0,  0 }, // None, 24bpp, Buffer 0/1 (not valid)
		},
		{   { 0,  0 }, // RGB/CI, 4bpp, Buffer 0/1
			{ 0,  8 }, // RGB/CI, 8bpp, Buffer 0/1
			{ 0, 12 }, // RGB/CI, 12bpp, Buffer 0/1
			{ 0,  0 }, // RGB/CI, 24bpp, Buffer 0/1 (not valid)
		},
		{   { 0,  0 }, // RGBA, 4bpp, Buffer 0/1
			{ 0,  8 }, // RGBA, 8bpp, Buffer 0/1
			{ 0, 12 }, // RGBA, 12bpp, Buffer 0/1
			{ 0,  0 }, // RGBA, 24bpp, Buffer 0/1 (not valid)
		},
		{   { 0,  0 }, // Invalid, 4bpp, Buffer 0/1
			{ 0,  0 }, // Invalid, 8bpp, Buffer 0/1
			{ 0,  0 }, // Invalid, 12bpp, Buffer 0/1
			{ 0,  0 }, // Invalid, 24bpp, Buffer 0/1 (not valid)
		},
		{   { 8, 16 }, // Overlay, 4bpp, Buffer 0/1
			{ 8, 16 }, // Overlay, 8bpp, Buffer 0/1
			{ 8, 16 }, // Overlay, 12bpp, Buffer 0/1
			{ 8, 16 }, // Overlay, 24bpp, Buffer 0/1 (not valid)
		},
		{   { 2,  6 }, // Popup, 4bpp, Buffer 0/1
			{ 2,  6 }, // Popup, 8bpp, Buffer 0/1
			{ 2,  6 }, // Popup, 12bpp, Buffer 0/1
			{ 2,  6 }, // Popup, 24bpp, Buffer 0/1 (not valid)
		},
		{   { 0,  4 }, // CID, 4bpp, Buffer 0/1
			{ 0,  4 }, // CID, 8bpp, Buffer 0/1
			{ 0,  4 }, // CID, 12bpp, Buffer 0/1
			{ 0,  4 }, // CID, 24bpp, Buffer 0/1 (not valid)
		},
		{   { 0,  0 }, // Invalid, 4bpp, Buffer 0/1
			{ 0,  0 }, // Invalid, 8bpp, Buffer 0/1
			{ 0,  0 }, // Invalid, 12bpp, Buffer 0/1
			{ 0,  0 }, // Invalid, 24bpp, Buffer 0/1 (not valid)
		},
	};

	m_src_shift = s_store_shift[m_plane_enable][m_draw_depth][m_dblsrc];
	m_draw_bpp = s_draw_bpp[m_draw_depth];

	switch (m_plane_enable)
	{
		case 1: // RGB/CI planes
		case 2: // RGBA planes
			m_dest_buf = &m_rgbci[0];
			break;
		case 4: // Overlay planes
		case 5: // Popup planes
		case 6: // CID planes
			m_dest_buf = &m_cidaux[0];
			break;
	}
}

void rb2_device::set_address(uint32_t address)
{
	m_buf_ptr = &m_dest_buf[address];
}

uint32_t rb2_device::read_pixel()
{
	return *m_buf_ptr >> m_src_shift;
}

void rb2_device::write_pixel(uint32_t data)
{
	data = expand_to_all_lanes(data);
	if (m_blend || m_fast_clear)
		store_pixel(data);
	else
		logic_pixel(data);
}

void rb2_device::logic_pixel(uint32_t src)
{
	const uint32_t dst = read_pixel();

	switch (m_logicop)
	{
		case 0:     store_pixel(0x000000);        break;
		case 1:     store_pixel(src & dst);       break;
		case 2:     store_pixel(src & ~dst);      break;
		case 3:     store_pixel(src);             break;
		case 4:     store_pixel(~src & dst);      break;
		case 5:     store_pixel(dst);             break;
		case 6:     store_pixel(src ^ dst);       break;
		case 7:     store_pixel(src | dst);       break;
		case 8:     store_pixel(~(src | dst));    break;
		case 9:     store_pixel(~(src ^ dst));    break;
		case 10:    store_pixel(~dst);            break;
		case 11:    store_pixel(src | ~dst);      break;
		case 12:    store_pixel(~src);            break;
		case 13:    store_pixel(~src | dst);      break;
		case 14:    store_pixel(~(src & dst));    break;
		case 15:    store_pixel(0xffffff);        break;
	}
}

void rb2_device::store_pixel(uint32_t value)
{
	const uint32_t write_mask = m_write_mask & m_global_mask;
	*m_buf_ptr &= ~write_mask;
	*m_buf_ptr |= value & write_mask;
}


/*************************************
 *
 *  Newport Device
 *
 *************************************/

/*static*/ const uint32_t newport_base_device::s_host_shifts[4] = { 8, 8, 16, 32 };

newport_base_device::newport_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_gio64_card_interface(mconfig, *this)
	, m_screen(*this, "screen")
	, m_xmap(*this, "xmap%u", 0U)
	, m_cmap(*this, "cmap%u", 0U)
	, m_vc2(*this, "vc2")
	, m_rb2(*this, "rb2")
	, m_write_mask_w(*this)
	, m_draw_flags_w(*this)
	, m_set_address(*this)
	, m_write_pixel(*this)
	, m_read_pixel(*this)
{
}

gio64_xl8_device::gio64_xl8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: newport_base_device(mconfig, GIO64_XL8, tag, owner, clock)
{
}

gio64_xl24_device::gio64_xl24_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: newport_base_device(mconfig, GIO64_XL24, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void newport_base_device::device_start()
{
	m_dcb_timeout_timer = timer_alloc(FUNC(newport_base_device::dcb_timeout_tick), this);

	save_item(NAME(m_rex3.m_draw_mode0));
	save_item(NAME(m_rex3.m_color_host));
	save_item(NAME(m_rex3.m_draw_mode1));
	save_item(NAME(m_rex3.m_plane_enable));
	save_item(NAME(m_rex3.m_plane_depth));
	save_item(NAME(m_rex3.m_rwpacked));
	save_item(NAME(m_rex3.m_hostdepth));
	save_item(NAME(m_rex3.m_rwdouble));
	save_item(NAME(m_rex3.m_sfactor));
	save_item(NAME(m_rex3.m_dfactor));
	save_item(NAME(m_rex3.m_store_shift));
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
	save_item(NAME(m_rex3.m_x_start_frac));
	save_item(NAME(m_rex3.m_y_start_frac));
	save_item(NAME(m_rex3.m_x_end_frac));
	save_item(NAME(m_rex3.m_y_end_frac));

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
	save_item(NAME(m_rex3.m_curr_color_red));
	save_item(NAME(m_rex3.m_curr_color_alpha));
	save_item(NAME(m_rex3.m_curr_color_green));
	save_item(NAME(m_rex3.m_curr_color_blue));
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
	save_item(NAME(m_rex3.m_dcb_mask));

	save_item(NAME(m_ramdac_lut_r));
	save_item(NAME(m_ramdac_lut_g));
	save_item(NAME(m_ramdac_lut_b));
	save_item(NAME(m_ramdac_lut_index));

	m_write_mask_w.resolve_safe();
	m_draw_flags_w.resolve_safe();
	m_set_address.resolve_safe();
	m_write_pixel.resolve_safe();
	m_read_pixel.resolve_safe(0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void newport_base_device::device_reset()
{
	memset(&m_rex3, 0, sizeof(rex3_t));
	memset(m_ramdac_lut_r, 0, sizeof(uint32_t) * 256);
	memset(m_ramdac_lut_g, 0, sizeof(uint32_t) * 256);
	memset(m_ramdac_lut_b, 0, sizeof(uint32_t) * 256);

	m_rex3.m_draw_mode0 = 0x00000000;
	m_rex3.m_draw_mode1 = 0x3002f001;
	m_rex3.m_dcb_mode = 0x00000780;
	m_rex3.m_x_window = 0x1000;
	m_rex3.m_y_window = 0x1000;

#if ENABLE_NEWVIEW_LOG
	m_newview_log = nullptr;
#endif
}

void newport_base_device::mem_map(address_map &map)
{
	map(0x000f0000, 0x000f1fff).rw(FUNC(newport_base_device::rex3_r), FUNC(newport_base_device::rex3_w));
}

#if ENABLE_NEWVIEW_LOG
void newport_base_device::start_logging()
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

	m_vc2->serialize(m_newview_log);
	printf("vc2: %08x\n", (uint32_t)ftell(m_newview_log));
	m_xmap[0]->serialize(m_newview_log);
	printf("xmap0: %08x\n", (uint32_t)ftell(m_newview_log));
	m_xmap[1]->serialize(m_newview_log);
	printf("xmap1: %08x\n", (uint32_t)ftell(m_newview_log));
	fwrite(&m_rex3, sizeof(rex3_t), 1, m_newview_log);
	printf("rex3: %08x\n", (uint32_t)ftell(m_newview_log));
	m_cmap[0]->serialize(m_newview_log);
	printf("cmap0: %08x\n", (uint32_t)ftell(m_newview_log));
	m_cmap[1]->serialize(m_newview_log);
	printf("cmap1: %08x\n", (uint32_t)ftell(m_newview_log));
	m_rb2->serialize(m_newview_log);
	printf("rb2: %08x\n", (uint32_t)ftell(m_newview_log));
	fwrite(m_rb2->rgbci(0), sizeof(uint32_t), (1280+64)*(1024+64), m_newview_log);
	printf("rgbci: %08x\n", (uint32_t)ftell(m_newview_log));
	fwrite(m_rb2->cidaux(0), sizeof(uint32_t), (1280+64)*(1024+64), m_newview_log);
	printf("cidaux: %08x\n", (uint32_t)ftell(m_newview_log));
}

void newport_base_device::stop_logging()
{
	popmessage("Newport recording stopped.");
	fclose(m_newview_log);
	m_newview_log = nullptr;
}
#endif

TIMER_CALLBACK_MEMBER(newport_base_device::dcb_timeout_tick)
{
	m_rex3.m_status &= ~STATUS_BACKBUSY;
}

uint32_t newport_base_device::screen_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const uint16_t cursor_msb = (uint16_t)m_xmap[0]->cursor_cmap() << 5;
	const uint16_t popup_msb = (uint16_t)m_xmap[0]->popup_cmap() << 5;
	const uint32_t *palette = m_cmap[0]->palette_base();

	const int y_start = m_vc2->readout_y0();
	const int y_end = m_vc2->readout_y1();

	/* loop over rows and copy to the destination */
	for (int y = cliprect.min_y, sy = y_start; y <= cliprect.max_y && sy < y_end; y++, sy++)
	{
		uint32_t *dest = &bitmap.pix(y, cliprect.min_x);
		const uint32_t *src_rgbci = m_rb2->rgbci(y);
		const uint32_t *src_cidaux = m_rb2->cidaux(y);

		// Fetch the initial DID entry
		uint16_t curr_did_entry = m_vc2->begin_did_line(y);
		uint32_t table_entry = m_xmap[0]->mode_entry(curr_did_entry & 0x1f);
		uint8_t pix_mode = (table_entry >> 8) & 3;
		uint8_t pix_size = (table_entry >> 10) & 3;
		uint8_t aux_pix_mode = (table_entry >> 16) & 7;
		uint16_t aux_msb = (table_entry >> 11) & 0x1f00;
		uint16_t ci_msb = 0;
		switch ((table_entry >> 8) & 3)
		{
			case 0: ci_msb = (table_entry & 0xf8) << 5; break;
			case 1: ci_msb = 0x1d00; break;
			case 2: ci_msb = 0x1e00; break;
			case 3: ci_msb = 0x1f00; break;
		}

		// Prepare for the next DID entry
		curr_did_entry = m_vc2->next_did_line_entry();

		// loop over columns
		for (int x = cliprect.min_x; x < cliprect.max_x; x++)
		{
			if ((uint16_t)x == (curr_did_entry >> 5))
			{
				table_entry = m_xmap[0]->mode_entry(curr_did_entry & 0x1f);
				pix_mode = (table_entry >> 8) & 3;
				pix_size = (table_entry >> 10) & 3;
				aux_pix_mode = (table_entry >> 16) & 7;
				aux_msb = (table_entry >> 11) & 0x1f00;
				switch ((table_entry >> 8) & 3)
				{
					case 0: ci_msb = (table_entry & 0xf8) << 5; break;
					case 1: ci_msb = 0x1d00; break;
					case 2: ci_msb = 0x1e00; break;
					case 3: ci_msb = 0x1f00; break;
				}
				curr_did_entry = m_vc2->next_did_line_entry();
			}
			uint8_t cursor_pixel = m_vc2->get_cursor_pixel(x, y);

			bool pixel_replaced = false;
			if (cursor_pixel)
			{
				*dest++ = palette[cursor_msb | cursor_pixel];
				pixel_replaced = true;
			}
			else if (*src_cidaux & 0xcc)
			{
				const uint32_t src = (*src_cidaux >> 2) & 3;
				*dest++ = palette[popup_msb | src];
				pixel_replaced = true;
			}
			else if (aux_pix_mode != 0)
			{
				if (m_xmap[0]->is_8bpp())
				{
					switch (aux_pix_mode)
					{
						case 1: // 2-Bit Underlay
							*dest++ = palette[aux_msb | ((*src_cidaux >> 8) & 3)];
							pixel_replaced = true;
							continue;
						case 2: // 2-Bit Overlay
						{
							const uint32_t pix_in = (*src_cidaux >> 8) & 3;
							if (pix_in)
							{
								*dest++ = palette[aux_msb | pix_in];
								pixel_replaced = true;
							}
							break;
						}
						case 6: // 1-Bit Overlay
						{
							const uint32_t shift = BIT(table_entry, 1) ? 9 : 8;
							const uint32_t pix_in = (*src_cidaux >> shift) & 1;
							if (pix_in)
							{
								*dest++ = palette[aux_msb | pix_in];
								pixel_replaced = true;
							}
							break;
						}
						case 7: // 1-Bit Overlay, 1-Bit Underlay
						{
							const uint32_t pix_in = (*src_cidaux >> 8) & 1;
							if (pix_in)
								*dest++ = palette[aux_msb | pix_in];
							else
								*dest++ = palette[aux_msb | ((*src_cidaux >> 9) & 1)];
							pixel_replaced = true;
							break;
						}
						default:
							break;
					}
				}
				else
				{
					switch (aux_pix_mode)
					{
						case 1: // 8-Bit Underlay
							*dest++ = palette[aux_msb | ((*src_cidaux >> 8) & 0xff)];
							pixel_replaced = true;
							break;
						case 2: // 8-Bit Overlay
						{
							const uint32_t pix_in = ((*src_cidaux >> 8) & 0xf) | ((*src_cidaux >> 16) & 0xf0);
							if (pix_in)
							{
								*dest++ = palette[aux_msb | pix_in];
								pixel_replaced = true;
							}
							break;
						}
						case 6: // 4-Bit Overlay
						{
							const uint32_t shift = BIT(table_entry, 1) ? 12 : 8;
							const uint32_t pix_in = (*src_cidaux >> shift) & 0xf;
							if (pix_in)
							{
								*dest++ = palette[aux_msb | pix_in];
								pixel_replaced = true;
							}
							break;
						}
						case 7: // 4-Bit Overlay, 4-Bit Underlay
						{
							const uint32_t pix_in = (*src_cidaux >> 8) & 0xf;
							if (pix_in)
								*dest++ = palette[aux_msb | pix_in];
							else
								*dest++ = palette[aux_msb | ((*src_cidaux >> 12) & 0xf)];
							pixel_replaced = true;
							break;
						}
						default:
							break;
					}
				}
			}

			if (!pixel_replaced)
			{
				switch (pix_mode)
				{
					case 0: // CI
						switch (pix_size)
						{
							case 0: // 4bpp
							{
								const uint8_t shift = BIT(table_entry, 0) ? 4 : 0;
								const uint32_t pix_in = *src_rgbci;
								*dest++ = palette[ci_msb | ((pix_in >> shift) & 0x0f)];
								break;
							}
							case 1: // 8bpp
								*dest++ = palette[ci_msb | (uint8_t)*src_rgbci];
								break;
							case 2: // 12bpp
							{
								const uint8_t shift = BIT(table_entry, 0) ? 12 : 0;
								const uint32_t pix_in = (*src_rgbci >> shift) & 0x00000fff;
								*dest++ = palette[(ci_msb & 0x1000) | pix_in];
								break;
							}
							case 3: // 24bpp (not yet supported)
								dest++;
								break;
						}
						break;
					case 1: // RGB Map0
						switch (pix_size)
						{
							case 0: // 4bpp
							{
								const uint8_t shift = BIT(table_entry, 0) ? 4 : 0;
								const uint8_t pix_in = (uint8_t)(*src_rgbci >> shift);
								*dest++ = convert_4bpp_bgr_to_24bpp_rgb(pix_in);
								break;
							}
							case 1: // 8bpp
							{
								const uint8_t shift = BIT(table_entry, 0) ? 8 : 0;
								const uint8_t pix_in = (uint8_t)(*src_rgbci >> shift);
								*dest++ = convert_8bpp_bgr_to_24bpp_rgb(pix_in);
								break;
							}
							case 2: // 12bpp
							{
								const uint8_t shift = BIT(table_entry, 0) ? 12 : 0;
								const uint16_t pix_in = (uint16_t)(*src_rgbci >> shift);
								*dest++ = convert_12bpp_bgr_to_24bpp_rgb(pix_in);
								break;
							}
							case 3: // 24bpp
							{
								const uint32_t pix_in = (uint32_t)*src_rgbci;
								const uint8_t r = (uint8_t)(pix_in >> 0);
								const uint8_t g = (uint8_t)(pix_in >> 8);
								const uint8_t b = (uint8_t)(pix_in >> 16);
								*dest++ = (r << 16) | (g << 8) | b;
								break;
							}
						}
						break;
					case 2: // RGB Map1 (not yet supported)
						*dest++ = 0xff00ff;
						break;
					case 3: // RGB Map2 (not yet supported)
						*dest++ = 0x00ff00;
						break;
				}
			}

			ramdac_remap(dest - 1);

			src_rgbci++;
			src_cidaux++;
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

#if ENABLE_NEWVIEW_LOG
	if (m_newview_log != nullptr)
	{
		uint32_t offset_lo = 0x80000000;
		uint32_t data_hi = 0;
		uint32_t data_lo = 0;
		uint32_t mem_mask_hi = 0;
		uint32_t mem_mask_lo = 0;

		fwrite(&offset_lo, sizeof(uint32_t), 1, m_newview_log);
		fwrite(&data_hi, sizeof(uint32_t), 1, m_newview_log);
		fwrite(&data_lo, sizeof(uint32_t), 1, m_newview_log);
		fwrite(&mem_mask_hi, sizeof(uint32_t), 1, m_newview_log);
		fwrite(&mem_mask_lo, sizeof(uint32_t), 1, m_newview_log);
	}
#endif

	return 0;
}

void newport_base_device::ramdac_remap(uint32_t *dest)
{
	uint32_t out = 0xff000000;
	out |= m_ramdac_lut_r[(uint8_t)(*dest >> 16)];
	out |= m_ramdac_lut_g[(uint8_t)(*dest >>  8)];
	out |= m_ramdac_lut_b[(uint8_t)(*dest >>  0)];
	*dest = out;
}

uint32_t newport_base_device::convert_4bpp_bgr_to_8bpp(uint8_t pix_in)
{
	const uint8_t r = 0xff * BIT(pix_in, 0);
	const uint8_t g = (0xaa * BIT(pix_in, 2)) | (0x55 * BIT(pix_in, 1));
	const uint8_t b = 0xff * BIT(pix_in, 3);
	return (b & 0xc0) | ((g & 0xe0) >> 2) | ((r & 0xe0) >> 5);
}

uint32_t newport_base_device::convert_4bpp_bgr_to_12bpp(uint8_t pix_in)
{
	const uint32_t r = 0xff * BIT(pix_in, 0);
	const uint32_t g = (0xaa * BIT(pix_in, 2)) | (0x55 * BIT(pix_in, 1));
	const uint32_t b = 0xff * BIT(pix_in, 3);
	return ((b & 0xf0) << 4) | (g & 0xf0) | ((r & 0xf0) >> 4);
}

uint32_t newport_base_device::convert_4bpp_bgr_to_24bpp(uint8_t pix_in)
{
	const uint8_t r = 0xff * BIT(pix_in, 0);
	const uint8_t g = (0xaa * BIT(pix_in, 2)) | (0x55 * BIT(pix_in, 1));
	const uint8_t b = 0xff * BIT(pix_in, 3);
	return (b << 16) | (g << 8) | r;
}

uint32_t newport_base_device::convert_8bpp_bgr_to_4bpp(uint8_t pix_in)
{
	const uint8_t r = (0x92 * BIT(pix_in, 2)) | (0x49 * BIT(pix_in, 1)) | (0x24 * BIT(pix_in, 0));
	const uint8_t g = (0x92 * BIT(pix_in, 5)) | (0x49 * BIT(pix_in, 4)) | (0x24 * BIT(pix_in, 3));
	const uint8_t b = (0xaa * BIT(pix_in, 7)) | (0x55 * BIT(pix_in, 6));
	return (BIT(b, 7) << 3) | ((g & 0xc0) >> 5) | BIT(r, 7);
}

uint32_t newport_base_device::convert_8bpp_bgr_to_12bpp(uint8_t pix_in)
{
	const uint8_t r = (0x92 * BIT(pix_in, 2)) | (0x49 * BIT(pix_in, 1)) | (0x24 * BIT(pix_in, 0));
	const uint8_t g = (0x92 * BIT(pix_in, 5)) | (0x49 * BIT(pix_in, 4)) | (0x24 * BIT(pix_in, 3));
	const uint8_t b = (0xaa * BIT(pix_in, 7)) | (0x55 * BIT(pix_in, 6));
	return ((b & 0xf0) << 4) | (g & 0xf0) | ((r & 0xf0) >> 4);
}

uint32_t newport_base_device::convert_8bpp_bgr_to_24bpp(uint8_t pix_in)
{
	const uint8_t r = (0x92 * BIT(pix_in, 2)) | (0x49 * BIT(pix_in, 1)) | (0x24 * BIT(pix_in, 0));
	const uint8_t g = (0x92 * BIT(pix_in, 5)) | (0x49 * BIT(pix_in, 4)) | (0x24 * BIT(pix_in, 3));
	const uint8_t b = (0xaa * BIT(pix_in, 7)) | (0x55 * BIT(pix_in, 6));
	return (b << 16) | (g << 8) | r;
}

uint32_t newport_base_device::convert_12bpp_bgr_to_4bpp(uint16_t pix_in)
{
	const uint8_t r = 0x11 * ((pix_in >> 0) & 0xf);
	const uint8_t g = 0x11 * ((pix_in >> 4) & 0xf);
	const uint8_t b = 0x11 * ((pix_in >> 8) & 0xf);
	return (BIT(b, 7) << 3) | ((g & 0xc0) >> 5) | BIT(r, 7);
}

uint32_t newport_base_device::convert_12bpp_bgr_to_8bpp(uint16_t pix_in)
{
	const uint8_t r = 0x11 * ((pix_in >> 0) & 0xf);
	const uint8_t g = 0x11 * ((pix_in >> 4) & 0xf);
	const uint8_t b = 0x11 * ((pix_in >> 8) & 0xf);
	return (b & 0xc0) | ((g & 0xe0) >> 2) | ((r & 0xe0) >> 5);
}

uint32_t newport_base_device::convert_12bpp_bgr_to_24bpp(uint16_t pix_in)
{
	const uint8_t r = 0x11 * ((pix_in >> 0) & 0xf);
	const uint8_t g = 0x11 * ((pix_in >> 4) & 0xf);
	const uint8_t b = 0x11 * ((pix_in >> 8) & 0xf);
	return (b << 16) | (g << 8) | r;
}

uint32_t newport_base_device::convert_24bpp_bgr_to_4bpp(uint32_t pix_in)
{
	const uint8_t r = (uint8_t)(pix_in >> 0);
	const uint8_t g = (uint8_t)(pix_in >> 8);
	const uint8_t b = (uint8_t)(pix_in >> 16);
	return (BIT(b, 7) << 3) | ((g & 0xc0) >> 5) | BIT(r, 7);
}

uint32_t newport_base_device::convert_24bpp_bgr_to_8bpp(uint32_t pix_in)
{
	const uint8_t r = (uint8_t)(pix_in >> 0);
	const uint8_t g = (uint8_t)(pix_in >> 8);
	const uint8_t b = (uint8_t)(pix_in >> 16);
	return (b & 0xc0) | ((g & 0xe0) >> 2) | ((r & 0xe0) >> 5);
}

uint32_t newport_base_device::convert_24bpp_bgr_to_12bpp(uint32_t pix_in)
{
	const uint8_t r = (uint8_t)(pix_in >> 0);
	const uint8_t g = (uint8_t)(pix_in >> 8);
	const uint8_t b = (uint8_t)(pix_in >> 16);
	return ((b & 0xf0) << 4) | (g & 0xf0) | ((r & 0xf0) >> 4);
}

uint32_t newport_base_device::convert_4bpp_bgr_to_24bpp_rgb(uint8_t pix_in)
{
	const uint8_t r = 0xff * BIT(pix_in, 0);
	const uint8_t g = (0xaa * BIT(pix_in, 2)) | (0x55 * BIT(pix_in, 1));
	const uint8_t b = 0xff * BIT(pix_in, 3);
	return (r << 16) | (g << 8) | b;
}

uint32_t newport_base_device::convert_8bpp_bgr_to_24bpp_rgb(uint8_t pix_in)
{
	const uint8_t r = (0x92 * BIT(pix_in, 2)) | (0x49 * BIT(pix_in, 1)) | (0x24 * BIT(pix_in, 0));
	const uint8_t g = (0x92 * BIT(pix_in, 5)) | (0x49 * BIT(pix_in, 4)) | (0x24 * BIT(pix_in, 3));
	const uint8_t b = (0xaa * BIT(pix_in, 7)) | (0x55 * BIT(pix_in, 6));
	return (r << 16) | (g << 8) | b;
}

uint32_t newport_base_device::convert_12bpp_bgr_to_24bpp_rgb(uint16_t pix_in)
{
	const uint8_t r = 0x11 * ((pix_in >> 0) & 0xf);
	const uint8_t g = 0x11 * ((pix_in >> 4) & 0xf);
	const uint8_t b = 0x11 * ((pix_in >> 8) & 0xf);
	return (r << 16) | (g << 8) | b;
}

void newport_base_device::ramdac_write(uint32_t data)
{
	switch (m_rex3.m_dcb_reg_select)
	{
	case 0:
		LOGMASKED(LOG_RAMDAC, "RAMDAC LUT index write: %08x\n", data);
		m_ramdac_lut_index = (uint8_t)data;
		break;
	case 1:
		m_ramdac_lut_r[m_ramdac_lut_index] = (uint8_t)(data >>  8) << 16;
		m_ramdac_lut_g[m_ramdac_lut_index] = (uint8_t)(data >> 16) << 8;
		m_ramdac_lut_b[m_ramdac_lut_index] = (uint8_t)(data >> 24) << 0;
		m_ramdac_lut_index++;
		LOGMASKED(LOG_RAMDAC, "RAMDAC LUT entry write: %08x\n", data);
		break;
	default:
		LOGMASKED(LOG_RAMDAC | LOG_UNKNOWN, "Unknown RAMDAC register %02x write: %08x\n", m_rex3.m_dcb_reg_select, data);
		break;
	}
}

WRITE_LINE_MEMBER(newport_base_device::vrint_w)
{
	if (state)
	{
		m_rex3.m_status |= STATUS_VRINT;
		m_gio64->interrupt<2>(ASSERT_LINE);
	}
}

// TOOD: Figure out a better way of doing this
WRITE_LINE_MEMBER(newport_base_device::update_screen_size)
{
	const int x_start = m_vc2->readout_x0();
	const int y_start = m_vc2->readout_y0();
	const int x_end = m_vc2->readout_x1();
	const int y_end = m_vc2->readout_y1();

	m_screen->set_size((uint16_t)(x_end - x_start), (uint16_t)(y_end - y_start));
	m_screen->set_visarea_full();
}

uint64_t newport_base_device::rex3_r(offs_t offset, uint64_t mem_mask)
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
			case DCB_ADDR_VC2:
				ret |= (uint64_t)m_vc2->read(m_rex3.m_dcb_reg_select) << 32;
				break;
			case DCB_ADDR_CMAP0:
				ret |= (uint64_t)m_cmap[0]->read(m_rex3.m_dcb_reg_select) << 32;
				break;
			case DCB_ADDR_CMAP1:
				ret |= (uint64_t)m_cmap[1]->read(m_rex3.m_dcb_reg_select) << 32;
				break;
			case DCB_ADDR_XMAP0:
				ret |= (uint64_t)m_xmap[0]->read(m_rex3.m_dcb_reg_select) << 32;
				break;
			case DCB_ADDR_XMAP1:
				ret |= (uint64_t)m_xmap[1]->read(m_rex3.m_dcb_reg_select) << 32;
				break;
			case DCB_ADDR_RAMDAC:
				LOGMASKED(LOG_REX3, "REX3 Display Control Bus Data MSW Read from RAMDAC (not yet implemented)\n");
				break;
			case DCB_ADDR_CC1:
				LOGMASKED(LOG_REX3, "REX3 Display Control Bus Data MSW Read from CC1 (not yet implemented)\n");
				break;
			case DCB_ADDR_AB1:
				LOGMASKED(LOG_REX3, "REX3 Display Control Bus Data MSW Read from AB1 (not yet implemented)\n");
				break;
			case DCB_ADDR_PCD:
				LOGMASKED(LOG_REX3, "REX3 Display Control Bus Data MSW Read from PCD (not yet implemented)\n");
				// Presenter not connected; simulate a bus timeout
				m_rex3.m_status |= STATUS_BACKBUSY;
				m_dcb_timeout_timer->adjust(attotime::from_msec(1));
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
			m_rex3.m_status &= ~STATUS_VRINT;
			m_gio64->interrupt<2>(CLEAR_LINE);
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

	if (offset & 0x00000100)
	{
		do_rex3_command();
	}

#if ENABLE_NEWVIEW_LOG
	if (m_newview_log != nullptr)
	{
		uint32_t offset_lo = (uint32_t)offset | 0x40000000;
		uint32_t data_hi = (uint32_t)(ret >> 32);
		uint32_t data_lo = (uint32_t)ret;
		uint32_t mem_mask_hi = (uint32_t)(mem_mask >> 32);
		uint32_t mem_mask_lo = (uint32_t)mem_mask;

		fwrite(&offset_lo, sizeof(uint32_t), 1, m_newview_log);
		fwrite(&data_hi, sizeof(uint32_t), 1, m_newview_log);
		fwrite(&data_lo, sizeof(uint32_t), 1, m_newview_log);
		fwrite(&mem_mask_hi, sizeof(uint32_t), 1, m_newview_log);
		fwrite(&mem_mask_lo, sizeof(uint32_t), 1, m_newview_log);
	}
#endif

	return ret;
}

uint32_t newport_base_device::get_host_color()
{
	static const uint32_t s_color_masks[4] = { 0xf, 0xff, 0xfff, 0xffffffff };
	uint32_t color = (uint32_t)(m_rex3.m_host_dataport >> m_rex3.m_host_shift) & s_color_masks[m_rex3.m_hostdepth];
	if (m_rex3.m_rwpacked)
	{
		if ((m_rex3.m_rwdouble && m_rex3.m_host_shift > 0) || m_rex3.m_host_shift > 32)
			m_rex3.m_host_shift -= s_host_shifts[m_rex3.m_hostdepth];
		else
			m_rex3.m_host_shift = 64 - s_host_shifts[m_rex3.m_hostdepth];
	}
	uint8_t convert_index = (m_rex3.m_hostdepth << 2) | m_rex3.m_plane_depth;
	switch (convert_index & 15)
	{
	default:
		// No conversion needed
		break;
	case 1:     // 4bpp -> 8bpp
		color = convert_4bpp_bgr_to_8bpp((uint8_t)color);
		break;
	case 2:     // 4bpp -> 12bpp
		color = convert_4bpp_bgr_to_12bpp((uint8_t)color);
		break;
	case 3:     // 4bpp -> 24bpp
		color = convert_4bpp_bgr_to_24bpp((uint8_t)color);
		break;
	case 4:     // 8bpp -> 4bpp
		color = convert_8bpp_bgr_to_4bpp((uint8_t)color);
		break;
	case 6:     // 8bpp -> 12bpp
		color = convert_8bpp_bgr_to_12bpp((uint8_t)color);
		break;
	case 7:     // 8bpp -> 24bpp
		color = convert_8bpp_bgr_to_24bpp((uint8_t)color);
		break;
	case 8:     // 12bpp -> 4bpp
		color = convert_12bpp_bgr_to_4bpp((uint16_t)color);
		break;
	case 9:     // 12bpp -> 8bpp
		color = convert_12bpp_bgr_to_8bpp((uint16_t)color);
		break;
	case 11:    // 12bpp -> 24bpp
		color = convert_12bpp_bgr_to_24bpp((uint16_t)color);
		break;
	case 12:    // 32bpp -> 4bpp
		color = convert_24bpp_bgr_to_4bpp(color);
		break;
	case 13:    // 32bpp -> 8bpp
		color = convert_24bpp_bgr_to_8bpp(color);
		break;
	case 14:    // 32bpp -> 12bpp
		color = convert_24bpp_bgr_to_12bpp(color);
		break;
	}
	if (BIT(m_rex3.m_draw_mode1, 11))
		color = swapendian_int32(color);
	return color;
}

void newport_base_device::output_pixel(uint32_t color)
{
	const bool shade = BIT(m_rex3.m_draw_mode0, 18);
	const bool rgbmode = BIT(m_rex3.m_draw_mode1, 15);
	if (m_rex3.m_color_host)
		output_pixel(m_rex3.m_x_start_i, m_rex3.m_y_start_i, get_host_color());
	else if (shade || rgbmode)
		output_pixel(m_rex3.m_x_start_i, m_rex3.m_y_start_i, get_rgb_color(m_rex3.m_x_start_i, m_rex3.m_y_start_i));
	else
		output_pixel(m_rex3.m_x_start_i, m_rex3.m_y_start_i, color);
}

bool newport_base_device::pixel_clip_pass(int16_t x, int16_t y)
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

void newport_base_device::output_pixel(int16_t x, int16_t y, uint32_t color)
{
	if (!pixel_clip_pass(x, y))
	{
		return;
	}

	x += m_rex3.m_x_window;
	y += m_rex3.m_y_window;
	x -= 0x1000;
	y -= 0x1000;

	const uint32_t address = (uint32_t)(y * (1280 + 64) + x);
	m_set_address(address);
	if (BIT(m_rex3.m_draw_mode1, 18))
		blend_pixel(color);
	else
		m_write_pixel(color);
}

void newport_base_device::blend_pixel(uint32_t src)
{
	const uint32_t dst = m_read_pixel();

	float sa = ((src >> 24) & 0xff) / 255.0f;
	float sb = 0.0f;
	float sg = 0.0f;
	float sr = 0.0f;

	float db = 0.0f;
	float dg = 0.0f;
	float dr = 0.0f;

	float sbb = 0.0f;
	float sgb = 0.0f;
	float srb = 0.0f;

	float dbb = 0.0f;
	float dgb = 0.0f;
	float drb = 0.0f;

	switch (m_rex3.m_plane_depth)
	{
	case 0: // 4bpp (not supported)
		break;
	case 1: // 8bpp
		sb = (0xaa * BIT(src, 7)) | (0x55 * BIT(src, 6));
		sg = (0x92 * BIT(src, 5)) | (0x49 * BIT(src, 4)) | (0x24 * BIT(src, 3));
		sr = (0x92 * BIT(src, 2)) | (0x49 * BIT(src, 1)) | (0x24 * BIT(src, 0));

		if (m_rex3.m_blend_back)
		{
			db = (uint8_t)(m_rex3.m_color_back >> 16) / 255.0f;
			dg = (uint8_t)(m_rex3.m_color_back >>  8) / 255.0f;
			dr = (uint8_t)(m_rex3.m_color_back >>  0) / 255.0f;
		}
		else
		{
			const uint32_t dstc = dst & 0xfff;
			db = (((dstc >> 8) & 15) * 0x11) / 255.0f;
			dg = (((dstc >> 4) & 15) * 0x11) / 255.0f;
			dr = (((dstc >> 0) & 15) * 0x11) / 255.0f;
		}
		break;
	case 2: // 12bpp
		sb = (((src >> 8) & 15) * 0x11) / 255.0f;
		sg = (((src >> 4) & 15) * 0x11) / 255.0f;
		sr = (((src >> 0) & 15) * 0x11) / 255.0f;

		if (m_rex3.m_blend_back)
		{
			db = (uint8_t)(m_rex3.m_color_back >> 16) / 255.0f;
			dg = (uint8_t)(m_rex3.m_color_back >>  8) / 255.0f;
			dr = (uint8_t)(m_rex3.m_color_back >>  0) / 255.0f;
		}
		else
		{
			const uint32_t dstc = dst & 0xfff;
			db = (((dstc >> 8) & 15) * 0x11) / 255.0f;
			dg = (((dstc >> 4) & 15) * 0x11) / 255.0f;
			dr = (((dstc >> 0) & 15) * 0x11) / 255.0f;
		}
		break;
	case 3: // 24bpp
		sa = (uint8_t)(src >> 24) / 255.0f;
		sb = (uint8_t)(src >> 16) / 255.0f;
		sg = (uint8_t)(src >>  8) / 255.0f;
		sr = (uint8_t)(src >>  0) / 255.0f;

		if (m_rex3.m_blend_back)
		{
			db = (uint8_t)(m_rex3.m_color_back >> 16) / 255.0f;
			dg = (uint8_t)(m_rex3.m_color_back >>  8) / 255.0f;
			dr = (uint8_t)(m_rex3.m_color_back >>  0) / 255.0f;
		}
		else
		{
			const uint32_t dstc = dst;
			db = (uint8_t)(dstc >> 16) / 255.0f;
			dg = (uint8_t)(dstc >>  8) / 255.0f;
			dr = (uint8_t)(dstc >>  0) / 255.0f;
		}
		break;
	}

	switch (m_rex3.m_sfactor)
	{
	case 0: // 0
	default:
		break;
	case 1: // 1
		sbb = sb;
		sgb = sg;
		srb = sr;
		break;
	case 2: // dstc
		sbb = sb * db;
		sgb = sg * dg;
		srb = sr * dr;
		break;
	case 3: // 1 - dstc
		sbb = sb * (1.0f - db);
		sgb = sg * (1.0f - dg);
		srb = sr * (1.0f - dr);
		break;
	case 4: // srca
		if (BIT(m_rex3.m_draw_mode1, 27))
		{
			sbb = sb * sa;
			sgb = sg * sa;
			srb = sr * sa;
		}
		else
		{
			sbb = sb;
			sgb = sg;
			srb = sr;
		}
		break;
	case 5: // 1 - srca
		if (BIT(m_rex3.m_draw_mode1, 27))
		{
			sbb = sb * (1.0f - sa);
			sgb = sg * (1.0f - sa);
			srb = sr * (1.0f - sa);
		}
		break;
	}

	switch (m_rex3.m_dfactor)
	{
	case 0: // 0
	default:
		break;
	case 1: // 1
		dbb = db;
		dgb = dg;
		drb = dr;
		break;
	case 2: // srcc
		dbb = db * sb;
		dgb = dg * sg;
		drb = dr * sr;
		break;
	case 3: // 1 - srcc
		dbb = db * (1.0f - sb);
		dgb = dg * (1.0f - sg);
		drb = dr * (1.0f - sr);
		break;
	case 4: // srca
		dbb = db * sa;
		dgb = dg * sa;
		drb = dr * sa;
		break;
	case 5: // 1 - srca
		dbb = db * (1.0f - sa);
		dgb = dg * (1.0f - sa);
		drb = dr * (1.0f - sa);
		break;
	}

	const float b_blend = sbb + dbb;
	const float g_blend = sgb + dgb;
	const float r_blend = srb + drb;

	const uint8_t b_blend_i = b_blend > 1.0f ? 255 : (b_blend < 0.0f ? 0 : (uint8_t)(b_blend * 255.0f));
	const uint8_t g_blend_i = g_blend > 1.0f ? 255 : (g_blend < 0.0f ? 0 : (uint8_t)(g_blend * 255.0f));
	const uint8_t r_blend_i = r_blend > 1.0f ? 255 : (r_blend < 0.0f ? 0 : (uint8_t)(r_blend * 255.0f));

	switch (m_rex3.m_plane_depth)
	{
	case 0: // 4bpp (not supported)
	case 1: // 8bpp (not supported)
		break;
	case 2: // 12bpp
		m_write_pixel(((b_blend_i & 0xf0) << 4) | (g_blend_i & 0xf0) | ((r_blend_i & 0xf0) >> 4));
		break;
	case 3: // 24bpp
		m_write_pixel((b_blend_i << 16) | (g_blend_i << 8) | r_blend_i);
		break;
	}
}

uint32_t newport_base_device::get_rgb_color(int16_t x, int16_t y)
{
	static const uint8_t s_bayer[4][4] = { { 0, 12, 3, 15 },{ 8, 4, 11, 7 },{ 2, 14, 1, 13 },{ 10, 6, 9, 5 } };

	uint32_t red = ((m_rex3.m_curr_color_red >> 11) & 0x1ff);
	uint32_t green = ((m_rex3.m_curr_color_green >> 11) & 0x1ff);
	uint32_t blue = ((m_rex3.m_curr_color_blue >> 11) & 0x1ff);
	uint32_t alpha = ((m_rex3.m_curr_color_alpha >> 11) & 0x1ff);

	if (red >= 0x180 || BIT(m_rex3.m_curr_color_red, 31))
	{
		red = 0;
	}
	else if (red > 0xff)
	{
		red = 0xff;
	}

	if (green >= 0x180 || BIT(m_rex3.m_curr_color_green, 31))
	{
		green = 0;
	}
	else if (green > 0xff)
	{
		green = 0xff;
	}

	if (blue >= 0x180 || BIT(m_rex3.m_curr_color_blue, 31))
	{
		blue = 0;
	}
	else if (blue > 0xff)
	{
		blue = 0xff;
	}

	if (alpha >= 0x180 || BIT(m_rex3.m_curr_color_alpha, 31))
	{
		alpha = 0;
	}
	else if (alpha > 0xff)
	{
		alpha = 0xff;
	}
	alpha <<= 24;

	if (!BIT(m_rex3.m_draw_mode1, 15)) // RGB
	{
		return get_default_color(m_rex3.m_curr_color_red);
	}

	if (BIT(m_rex3.m_draw_mode1, 16)) // Dithering
	{
		switch (m_rex3.m_plane_depth)
		{
		case 0: // 4bpp
		{
			const uint8_t sr = (red >> 3) - (red >> 4);
			const uint8_t sg = (green >> 2) - (green >> 4);
			const uint8_t sb = (blue >> 3) - (blue >> 4);

			uint8_t dr = BIT(sr, 4);
			uint8_t dg = (sg >> 4) & 3;
			uint8_t db = BIT(sb, 4);

			if ((sr & 0xf) > s_bayer[x & 3][y & 3]) dr++;
			if ((sg & 0xf) > s_bayer[x & 3][y & 3]) dg++;
			if ((sb & 0xf) > s_bayer[x & 3][y & 3]) db++;

			if (dr > 1) dr = 1;
			if (dg > 3) dg = 3;
			if (db > 1) db = 1;

			uint32_t color = (db << 3) | (dg << 1) | dr;
			return alpha | (color << 4) | color;
		}

		case 1: // 8bpp
		{
			const uint8_t sr = (red >> 1) - (red >> 4);
			const uint8_t sg = (green >> 1) - (green >> 4);
			const uint8_t sb = (blue >> 2) - (blue >> 4);

			uint8_t dr = (sr >> 4) & 7;
			uint8_t dg = (sg >> 4) & 7;
			uint8_t db = (sb >> 4) & 3;

			if ((sr & 0xf) > s_bayer[x & 3][y & 3]) dr++;
			if ((sg & 0xf) > s_bayer[x & 3][y & 3]) dg++;
			if ((sb & 0xf) > s_bayer[x & 3][y & 3]) db++;

			if (dr > 7) dr = 7;
			if (dg > 7) dg = 7;
			if (db > 3) db = 3;

			return alpha | (db << 6) | (dg << 3) | dr;
		}

		case 2: // 12bpp
		{
			const uint32_t sr = red - (red >> 4);
			const uint32_t sg = green - (green >> 4);
			const uint32_t sb = blue - (blue >> 4);

			uint32_t dr = (sr >> 4) & 15;
			uint32_t dg = (sg >> 4) & 15;
			uint32_t db = (sb >> 4) & 15;

			if ((sr & 0xf) > s_bayer[x & 3][y & 3]) dr++;
			if ((sg & 0xf) > s_bayer[x & 3][y & 3]) dg++;
			if ((sb & 0xf) > s_bayer[x & 3][y & 3]) db++;

			if (dr > 15) dr = 15;
			if (dg > 15) dg = 15;
			if (db > 15) db = 15;

			uint32_t color = (db << 8) | (dg << 4) | dr;
			return alpha | (color << 12) | color;
		}

		case 3: // 24bpp
			return alpha | (blue << 16) | (green << 8) | red;

		default:
			return 0;
		}
	}
	else
	{
		switch (m_rex3.m_plane_depth)
		{
		case 0: // 4bpp
			return alpha | (BIT(blue, 7) << 3) | ((green & 0xc0) >> 5) | BIT(red, 7);
		case 1: // 8bpp
			return alpha | (blue & 0xc0) | ((green & 0xe0) >> 2) | ((red & 0xe0) >> 5);
		case 2: // 12bpp
			return alpha | ((blue & 0xf0) << 4) | (green & 0xf0) | ((red & 0xf0) >> 4);
		case 3: // 24bpp
			return alpha | (blue << 16) | (green << 8) | red;
		default:
			return 0;
		}
	}
}

uint8_t newport_base_device::get_octant(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t dx, int32_t dy)
{
	if (x1 < x2)
	{
		if (y2 < y1)
			return (dx > dy) ? 0 : 1;
		else
			return (dx > dy) ? 7 : 6;
	}
	else
	{
		if (y2 < y1)
			return (dx > dy) ? 3 : 2;
		else
			return (dx > dy) ? 4 : 5;
	}
}

void newport_base_device::do_fline(uint32_t color)
{
	const int32_t x1 = (int32_t)((m_rex3.m_x_start >> 7) << 12) >> 12;
	const int32_t y1 = (int32_t)((m_rex3.m_y_start >> 7) << 12) >> 12;
	const int32_t x2 = (int32_t)((m_rex3.m_x_end >> 7) << 12) >> 12;
	const int32_t y2 = (int32_t)((m_rex3.m_y_end >> 7) << 12) >> 12;

	const int32_t x10 = x1 & ~0xf;
	const int32_t y10 = y1 & ~0xf;
	const int32_t x20 = x2 & ~0xf;
	const int32_t y20 = y2 & ~0xf;

	const bool iterate_one = (m_rex3.m_draw_mode0 & 0x300) == 0;
	const bool skip_first = BIT(m_rex3.m_draw_mode0, 10);
	const bool skip_last = BIT(m_rex3.m_draw_mode0, 11);
	const bool shade = BIT(m_rex3.m_draw_mode0, 18);
	const bool rgbmode = BIT(m_rex3.m_draw_mode1, 15);

	int32_t x = x10;
	int32_t y = y10;

	int16_t x1_fract = m_rex3.m_x_start_frac;
	int16_t y1_fract = m_rex3.m_y_start_frac;

	int32_t dx = abs(x1 - x2);
	int32_t dy = abs(y1 - y2);

	const int32_t dx_i = abs(x10 - x20) - 1;
	const int32_t dy_i = abs(y10 - y20) - 1;

	static const bresenham_octant_info_t s_bresenham_infos[8] =
	{
		{  1,  1,  0,  1,  0 },
		{  0,  1,  1,  1,  1 },
		{  0, -1,  1,  1,  1 },
		{ -1, -1,  0,  1,  0 },
		{ -1, -1,  0, -1,  0 },
		{  0, -1, -1, -1,  1 },
		{  0,  1, -1, -1,  1 },
		{  1,  1,  0, -1,  0 }
	};

	const uint8_t octant = get_octant(x1, y1, x2, y2, dx, dy);
	const int32_t incrx1 = s_bresenham_infos[octant].incrx1;
	const int32_t incrx2 = s_bresenham_infos[octant].incrx2;
	const int32_t incry1 = s_bresenham_infos[octant].incry1;
	const int32_t incry2 = s_bresenham_infos[octant].incry2;
	int32_t loop = s_bresenham_infos[octant].loop ? dy_i : dx_i;
	if (BIT(m_rex3.m_draw_mode0, 15) && loop > 32)
		loop = 32;
	const int32_t x_major = 1 - s_bresenham_infos[octant].loop;

	const int32_t incr1 = s_bresenham_infos[octant].loop ? (2 * dx) : (2 * dy);
	const int32_t incr2 = s_bresenham_infos[octant].loop ? (2 * (dx - dy)) : (2 * (dy - dx));
	int32_t d = s_bresenham_infos[octant].loop ? (3 * dx - 2 * dy) : (3 * dy - 2 * dx);

	switch (octant)
	{
	case 0:
		// Nothing special needed
		break;
	case 1:
	{
		const int16_t temp_fract = x1_fract;
		x1_fract = y1_fract;
		y1_fract = temp_fract;
		const int32_t temp_d = dx;
		dx = dy;
		dy = temp_d;
		break;
	}
	case 2:
	{
		const int16_t temp_fract = 0x10 - x1_fract;
		x1_fract = y1_fract;
		y1_fract = temp_fract;
		const int32_t temp_d = dx;
		dx = dy;
		dy = temp_d;
		break;
	}
	case 3:
		x1_fract = 0x10 - x1_fract;
		break;
	case 4:
		x1_fract = 0x10 - x1_fract;
		y1_fract = 0x10 - y1_fract;
		break;
	case 5:
	{
		const int16_t temp_fract = 0x10 - x1_fract;
		x1_fract = 0x10 - y1_fract;
		y1_fract = temp_fract;
		const int32_t temp_d = dx;
		dx = dy;
		dy = temp_d;
		break;
	}
	case 6:
	{
		const int16_t temp_fract = 0x10 - y1_fract;
		y1_fract = x1_fract;
		x1_fract = temp_fract;
		const int32_t temp_d = dx;
		dx = dy;
		dy = temp_d;
		break;
	}
	case 7:
		y1_fract = 0x10 - y1_fract;
		break;
	}

	d += 2 * (((dx * y1_fract) >> 4) - ((dy * x1_fract) >> 4)); // Adjust d due to fractional endpoints
	const int32_t E = d - 2 * dx; // Variable used for adjusting the start point up one pixel
	if (E > 0)
	{
		d = E;
		x += incrx2 * (1 - x_major);
		y += incry2 * x_major;
	}

	if (!skip_first || iterate_one)
	{
		const int16_t x16 = (int16_t)(x >> 4);
		const int16_t y16 = (int16_t)(y >> 4);
		if (shade || rgbmode)
			output_pixel(x16, y16, get_rgb_color(x16, y16));
		else
			output_pixel(x16, y16, color);

		if (shade)
			iterate_shade();

		if (d < 0)
		{
			x += incrx1;
			y -= incry1;
			d += incr1;
		}
		else
		{
			x += incrx2;
			y -= incry2;
			d += incr2;
		}

		if (iterate_one)
		{
			write_x_start(x << 7);
			write_y_start(y << 7);
			return;
		}
	}

	for (int32_t i = 1; i < loop; i++)
	{
		const int16_t x16 = (int16_t)(x >> 4);
		const int16_t y16 = (int16_t)(y >> 4);
		if (shade || rgbmode)
			output_pixel(x16, y16, get_rgb_color(x16, y16));
		else
			output_pixel(x16, y16, color);

		if (shade)
			iterate_shade();

		if (d < 0)
		{
			x += incrx1;
			y -= incry1;
			d += incr1;
		}
		else
		{
			x += incrx2;
			y -= incry2;
			d += incr2;
		}
	}

	if (!skip_last)
	{
		const int16_t x16 = (int16_t)(x2 >> 4);
		const int16_t y16 = (int16_t)(y2 >> 4);
		if (shade || rgbmode)
			output_pixel(x16, y16, get_rgb_color(x16, y16));
		else
			output_pixel(x16, y16, color);

		if (shade)
			iterate_shade();

		write_x_start(x2);
		write_y_start(y2);
	}
	else
	{
		write_x_start(x << 7);
		write_y_start(y << 7);
	}
}

void newport_base_device::do_iline(uint32_t color)
{
	int32_t x1 = m_rex3.m_x_start_i;
	int32_t y1 = m_rex3.m_y_start_i;
	int32_t x2 = m_rex3.m_x_end_i;
	int32_t y2 = m_rex3.m_y_end_i;

	const bool iterate_one = (m_rex3.m_draw_mode0 & 0x300) == 0;
	const bool skip_first = BIT(m_rex3.m_draw_mode0, 10);
	const bool skip_last = BIT(m_rex3.m_draw_mode0, 11);
	const bool shade = BIT(m_rex3.m_draw_mode0, 18);
	const bool rgbmode = BIT(m_rex3.m_draw_mode1, 15);

	int32_t x = x1;
	int32_t y = y1;
	int32_t dx = abs(x2 - x1);
	int32_t dy = abs(y2 - y1);

	static const bresenham_octant_info_t s_bresenham_infos[8] =
	{
		{  1,  1,  0,  1, 0 },
		{  0,  1,  1,  1, 1 },
		{  0, -1,  1,  1, 1 },
		{ -1, -1,  0,  1, 0 },
		{ -1, -1,  0, -1, 0 },
		{  0, -1, -1, -1, 1 },
		{  0,  1, -1, -1, 1 },
		{  1,  1,  0, -1, 0 }
	};

	const uint8_t octant = get_octant(x1, y1, x2, y2, dx, dy);
	const int32_t incrx1 = s_bresenham_infos[octant].incrx1;
	const int32_t incrx2 = s_bresenham_infos[octant].incrx2;
	const int32_t incry1 = s_bresenham_infos[octant].incry1;
	const int32_t incry2 = s_bresenham_infos[octant].incry2;
	int32_t loop = s_bresenham_infos[octant].loop ? dy : dx;
	if (BIT(m_rex3.m_draw_mode0, 15) && loop > 32)
		loop = 32;

	const int32_t incr1 = 2 * (s_bresenham_infos[octant].loop ? dx : dy);
	const int32_t incr2 = 2 * (s_bresenham_infos[octant].loop ? (dx - dy) : (dy - dx));
	int32_t d = incr1 - (s_bresenham_infos[octant].loop ? dy : dx);

	if (!skip_first || iterate_one)
	{
		const int16_t x16 = (int16_t)x;
		const int16_t y16 = (int16_t)y;

		if (shade || rgbmode)
			output_pixel(x16, y16, get_rgb_color(x16, y16));
		else
			output_pixel(x16, y16, color);

		if (shade)
			iterate_shade();

		if (d < 0)
		{
			x += incrx1;
			y -= incry1;
			d += incr1;
		}
		else
		{
			x += incrx2;
			y -= incry2;
			d += incr2;
		}

		if (iterate_one)
		{
			write_x_start(x << 11);
			write_y_start(y << 11);
			return;
		}
	}

	for (int32_t i = 1; i < loop; i++)
	{
		const int16_t x16 = (int16_t)x;
		const int16_t y16 = (int16_t)y;

		if (shade || rgbmode)
			output_pixel(x16, y16, get_rgb_color(x16, y16));
		else
			output_pixel(x16, y16, color);

		if (shade)
			iterate_shade();

		if (d < 0)
		{
			x += incrx1;
			y -= incry1;
			d += incr1;
		}
		else
		{
			x += incrx2;
			y -= incry2;
			d += incr2;
		}
	}

	if (!skip_last)
	{
		const int16_t x16 = (int16_t)x2;
		const int16_t y16 = (int16_t)y2;

		if (shade || rgbmode)
			output_pixel(x16, y16, get_rgb_color(x16, y16));
		else
			output_pixel(x16, y16, color);

		if (shade)
			iterate_shade();

		write_x_start(x2 << 11);
		write_y_start(y2 << 11);
	}
	else
	{
		write_x_start(x << 11);
		write_y_start(y << 11);
	}
}

uint32_t newport_base_device::do_pixel_read()
{
	m_rex3.m_bres_octant_inc1 = 0;
	const int16_t src_x = m_rex3.m_x_start_i + m_rex3.m_x_window - 0x1000;
	const int16_t src_y = m_rex3.m_y_start_i + m_rex3.m_y_window - 0x1000;
	uint32_t ret = 0;
	m_set_address(src_y * (1280 + 64) + src_x);
	ret = m_read_pixel();
	if (m_rex3.m_plane_enable == 1 || m_rex3.m_plane_enable == 2) // RGB/CI or RGBA planes
	{
		uint8_t convert_index = (m_rex3.m_plane_depth << 2) | m_rex3.m_hostdepth;
		switch (convert_index & 15)
		{
		default:
			// No conversion needed
			break;
		case 1:     // 4bpp -> 8bpp
			ret = convert_4bpp_bgr_to_8bpp((uint8_t)ret);
			break;
		case 2:     // 4bpp -> 12bpp
			ret = convert_4bpp_bgr_to_12bpp((uint8_t)ret);
			break;
		case 3:     // 4bpp -> 24bpp
			ret = convert_4bpp_bgr_to_24bpp((uint8_t)ret);
			break;
		case 4:     // 8bpp -> 4bpp
			ret = convert_8bpp_bgr_to_4bpp((uint8_t)ret);
			break;
		case 6:     // 8bpp -> 12bpp
			ret = convert_8bpp_bgr_to_12bpp((uint8_t)ret);
			break;
		case 7:     // 8bpp -> 24bpp
			ret = convert_8bpp_bgr_to_24bpp((uint8_t)ret);
			break;
		case 8:     // 12bpp -> 4bpp
			ret = convert_12bpp_bgr_to_4bpp((uint16_t)ret);
			break;
		case 9:     // 12bpp -> 8bpp
			ret = convert_12bpp_bgr_to_8bpp((uint16_t)ret);
			break;
		case 11:    // 12bpp -> 24bpp
			ret = convert_12bpp_bgr_to_24bpp((uint16_t)ret);
			break;
		case 12:    // 32bpp -> 4bpp
			ret = convert_24bpp_bgr_to_4bpp(ret);
			break;
		case 13:    // 32bpp -> 8bpp
			ret = convert_24bpp_bgr_to_8bpp(ret);
			break;
		case 14:    // 32bpp -> 12bpp
			ret = convert_24bpp_bgr_to_12bpp(ret);
			break;
		}
	}
	LOGMASKED(LOG_COMMANDS, "Read %08x from %04x, %04x\n", ret, src_x, src_y);
	m_rex3.m_x_start_i++;
	int16_t dy = (m_rex3.m_y_end_i < m_rex3.m_y_start_i) ? -1 : 1;
	if (m_rex3.m_x_start_i > m_rex3.m_x_end_i)
	{
		m_rex3.m_y_start_i += dy;
		m_rex3.m_x_start_i = m_rex3.m_x_save;
	}

	write_x_start(m_rex3.m_x_start_i << 11);
	write_y_start(m_rex3.m_y_start_i << 11);
	return ret;
}

uint64_t newport_base_device::do_pixel_word_read()
{
	const uint16_t x_start = (uint16_t)(m_rex3.m_xy_start_i >> 16);
	const uint16_t x_end = (uint16_t)(m_rex3.m_xy_end_i >> 16);
	const bool doubleword = BIT(m_rex3.m_draw_mode1, 10);

	uint16_t width = (x_end - x_start) + 1;
	uint64_t ret = 0;
	uint64_t shift = 0;
	switch ((m_rex3.m_draw_mode1 >> 8) & 3)
	{
		case 0: // 4bpp
		{
			const uint16_t max_width = doubleword ? 16 : 8;
			if (width > max_width)
				width = max_width;

			shift = 60;
			for (uint16_t i = 0; i < width; i++)
			{
				ret |= (uint64_t)(do_pixel_read() & 0x0000000f) << shift;
				shift -= 4;
			}
			break;
		}

		case 1: // 8bpp
		{
			const uint16_t max_width = doubleword ? 8 : 4;
			if (width > max_width)
				width = max_width;

			shift = 56;
			for (uint16_t i = 0; i < width; i++)
			{
				ret |= (uint64_t)(do_pixel_read() & 0x000000ff) << shift;
				shift -= 8;
			}
			break;
		}

		case 2: // 12bpp
		{
			const uint16_t max_width = doubleword ? 4 : 2;
			if (width > max_width)
				width = max_width;

			shift = 48;
			for (uint16_t i = 0; i < width; i++)
			{
				ret |= (uint64_t)(do_pixel_read() & 0x00000fff) << shift;
				shift -= 16;
			}
			break;
		}

		case 3: // 32bpp
		{
			const uint16_t max_width = doubleword ? 2 : 1;
			if (width > max_width)
				width = max_width;

			shift = 32;
			for (uint16_t i = 0; i < width; i++)
			{
				ret |= (uint64_t)do_pixel_read() << shift;
				shift -= 32;
			}
			break;
		}
	}
	return ret;
}

void newport_base_device::iterate_shade()
{
	if (m_rex3.m_slope_red & 0x7fffff)
		m_rex3.m_curr_color_red += (m_rex3.m_slope_red << 8) >> 8;
	if (m_rex3.m_slope_green & 0x7ffff)
		m_rex3.m_curr_color_green += (m_rex3.m_slope_green << 12) >> 12;
	if (m_rex3.m_slope_blue & 0x7ffff)
		m_rex3.m_curr_color_blue += (m_rex3.m_slope_blue << 12) >> 12;
	if (m_rex3.m_slope_alpha & 0x7ffff)
		m_rex3.m_curr_color_alpha += (m_rex3.m_slope_alpha << 12) >> 12;

	if (BIT(m_rex3.m_draw_mode0, 21)) // CIClamp
	{
		if (BIT(m_rex3.m_draw_mode1, 15)) // RGBMode
		{
			const uint32_t val_red = ((m_rex3.m_curr_color_red >> 11) & 0x1ff);
			const uint32_t val_grn = ((m_rex3.m_curr_color_green >> 11) & 0x1ff);
			const uint32_t val_blu = ((m_rex3.m_curr_color_blue >> 11) & 0x1ff);
			const uint32_t val_alpha = ((m_rex3.m_curr_color_alpha >> 11) & 0x1ff);

			if (val_red >= 0x180 || BIT(m_rex3.m_curr_color_red, 31))
				m_rex3.m_curr_color_red = 0;
			else if (val_red > 0xff)
				m_rex3.m_curr_color_red = 0x7ffff;

			if (val_grn >= 0x180 || BIT(m_rex3.m_curr_color_green, 31))
				m_rex3.m_curr_color_green = 0;
			else if (val_grn > 0xff)
				m_rex3.m_curr_color_green = 0x7ffff;

			if (val_blu >= 0x180 || BIT(m_rex3.m_curr_color_blue, 31))
				m_rex3.m_curr_color_blue = 0;
			else if (val_blu > 0xff)
				m_rex3.m_curr_color_blue = 0x7ffff;

			if (val_alpha >= 0x180 || BIT(m_rex3.m_curr_color_alpha, 31))
				m_rex3.m_curr_color_alpha = 0;
			else if (val_alpha > 0xff)
				m_rex3.m_curr_color_alpha = 0x7ffff;
		}
		else
		{
			switch ((m_rex3.m_draw_mode1 >> 3) & 3)
			{
			case 0: // 4bpp
				if (BIT(m_rex3.m_color_red, 15))
					m_rex3.m_color_red = 0x00007fff;
				break;
			case 1: // 8bpp
				if (BIT(m_rex3.m_color_red, 19))
					m_rex3.m_color_red = 0x0007ffff;
				break;
			case 2: // 12bpp
				if (BIT(m_rex3.m_color_red, 21))
					m_rex3.m_color_red = 0x001fffff;
				break;
			case 3: // 24bpp
					// No clamping on CI
				break;
			}
		}
	}
}

uint32_t newport_base_device::get_default_color(uint32_t src)
{
	uint32_t color = BIT(m_rex3.m_draw_mode1, 17) ? m_rex3.m_color_vram : src;
	switch (m_rex3.m_plane_depth)
	{
		case 0: // 4bpp
			color &= 0xf;
			color |= color << 4;
			color |= color << 8;
			color |= color << 16;
			break;
		case 1: // 8bpp
			color &= 0xff;
			color |= color << 8;
			color |= color << 16;
			break;
		case 2: // 12bpp
			if (BIT(m_rex3.m_draw_mode1, 15))
				color = ((m_rex3.m_color_vram & 0xf00000) >> 12) | ((m_rex3.m_color_vram & 0xf000) >> 8) | ((m_rex3.m_color_vram & 0xf0) >> 4);
			else
				color &= 0x00000fff;
			color |= color << 12;
			break;
		case 3: // 24bpp
			color = m_rex3.m_color_vram & 0xffffff;
			break;
	}

	return color;
}

void newport_base_device::do_rex3_command()
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

	const uint8_t opcode = mode0 & 3;
	const uint8_t adrmode = (mode0 >> 2) & 7;

	switch (opcode)
	{
		case 0: // NoOp
			break;
		case 1: // Read
			m_rex3.m_host_dataport = do_pixel_word_read();
			break;
		case 2: // Draw
			switch (adrmode)
			{
				case 0: // Span
				{
					end_x += dx;

					int16_t prim_end_x = end_x;
					bool stop_on_x = BIT(mode0, 8);

					if (BIT(mode0, 15) && abs(end_x - start_x) > 32)
						prim_end_x = start_x + 32 * dx;

					if (m_rex3.m_color_host && m_rex3.m_rwpacked)
					{
						stop_on_x = true;
						static const int16_t s_max_host_lengths[2][4] = { { 4, 4, 2, 1 }, { 8, 8, 4, 2 } };
						const int16_t max_length = s_max_host_lengths[m_rex3.m_rwdouble ? 1 : 0][m_rex3.m_hostdepth];
						int16_t length = abs(prim_end_x - start_x);
						if (length > max_length)
							prim_end_x = start_x + dx * max_length;
					}

					const bool shade = BIT(mode0, 18);
					const bool rgbmode = BIT(mode1, 15);
					const bool opaque = BIT(mode0, 16) || BIT(mode0, 17);
					const bool fastclear = BIT(mode1, 17);
					const uint32_t pattern = BIT(mode0, 12) ? m_rex3.m_z_pattern : (BIT(mode0, 13) ? m_rex3.m_ls_pattern : 0xffffffff);

					LOGMASKED(LOG_COMMANDS, "%04x, %04x to %04x, %04x = %08x\n", start_x, start_y, end_x, end_y, pattern);

					uint32_t color = get_default_color(m_rex3.m_color_i);
					const bool lr_abort = BIT(mode0, 19) && dx < 0;

					uint32_t bit = 31;
					do
					{
						if (lr_abort)
							break;

						if (shade)
							iterate_shade();

						if (BIT(pattern, bit))
						{
							if (m_rex3.m_color_host)
								output_pixel(start_x, start_y, get_host_color());
							else if ((shade || rgbmode) && !fastclear)
								output_pixel(start_x, start_y, get_rgb_color(start_x, start_y));
							else
								output_pixel(start_x, start_y, color);
						}
						else if (opaque)
						{
							output_pixel(start_x, start_y, m_rex3.m_color_back);
						}

						bit = (bit - 1) & 0x1f;
						start_x += dx;
					} while (start_x != prim_end_x && start_x != end_x && stop_on_x);

					if ((dx > 0 && start_x >= end_x) || (dx < 0 && start_x <= end_x) || lr_abort)
					{
						m_rex3.m_curr_color_red = m_rex3.m_color_red;
						m_rex3.m_curr_color_alpha = m_rex3.m_color_alpha;
						m_rex3.m_curr_color_green = m_rex3.m_color_green;
						m_rex3.m_curr_color_blue = m_rex3.m_color_blue;
					}

					write_x_start(start_x << 11);
					break;
				}

				case 1: // Block
				{
					end_x += dx;
					end_y += dy;

					int16_t prim_end_x = end_x;
					bool stop_on_x = BIT(mode0, 8);
					const bool stop_on_y = BIT(mode0, 9);

					if (BIT(mode0, 15) && (end_x - start_x) >= 32)
						prim_end_x = start_x + 32 * dx;

					if (m_rex3.m_color_host && m_rex3.m_rwpacked)
					{
						stop_on_x = true;
						static const int16_t s_max_host_lengths[2][4] = { { 4, 4, 2, 1 }, { 8, 8, 4, 2 } };
						const int16_t max_length = s_max_host_lengths[m_rex3.m_rwdouble ? 1 : 0][m_rex3.m_hostdepth];
						int16_t length = abs(prim_end_x - start_x);
						if (length > max_length)
							prim_end_x = start_x + dx * max_length;
					}

					const bool shade = BIT(mode0, 18);
					const bool rgbmode = BIT(mode1, 15);
					const bool opaque = BIT(mode0, 16) || BIT(mode0, 17);
					const bool fastclear = BIT(mode1, 17);
					const uint32_t pattern = BIT(mode0, 12) ? m_rex3.m_z_pattern : (BIT(mode0, 13) ? m_rex3.m_ls_pattern : 0xffffffff);

					uint32_t color = get_default_color(m_rex3.m_color_i);
					const bool lr_abort = BIT(mode0, 19) && dx < 0;

					do
					{
						uint32_t bit = 31;
						do
						{
							if (lr_abort)
								break;

							if (shade)
								iterate_shade();

							if (BIT(pattern, bit))
							{
								if (m_rex3.m_color_host)
									output_pixel(start_x, start_y, get_host_color());
								else if ((shade || rgbmode) && !fastclear)
									output_pixel(start_x, start_y, get_rgb_color(start_x, start_y));
								else
									output_pixel(start_x, start_y, color);
							}
							else if (opaque)
							{
								output_pixel(start_x, start_y, m_rex3.m_color_back);
							}

							bit = (bit - 1) & 0x1f;
							start_x += dx;
						} while (start_x != prim_end_x && start_x != end_x && stop_on_x);

						if ((dx > 0 && start_x >= end_x) || (dx < 0 && start_x <= end_x) || lr_abort)
						{
							m_rex3.m_curr_color_red = m_rex3.m_color_red;
							m_rex3.m_curr_color_alpha = m_rex3.m_color_alpha;
							m_rex3.m_curr_color_green = m_rex3.m_color_green;
							m_rex3.m_curr_color_blue = m_rex3.m_color_blue;
							start_x = m_rex3.m_x_save;
							start_y += dy;
						}
					} while (start_y != end_y && stop_on_y);

					write_x_start(start_x << 11);
					write_y_start(start_y << 11);
					break;
				}

				case 2: // I_Line
					do_iline(get_default_color(m_rex3.m_color_i));
					break;

				case 3: // F_Line
					do_fline(get_default_color(m_rex3.m_color_i));
					break;

				case 4: // A_Line
					do_iline(get_default_color(m_rex3.m_color_i)); // FIXME
					break;

				default: // Invalid
					break;
			}
			break;
		case 3: // Scr2Scr
			if (adrmode < 2)
			{
				const bool stop_on_x = BIT(mode0, 8);
				const bool stop_on_y = BIT(mode0, 9);

				end_x += dx;
				end_y += dy;

				LOGMASKED(LOG_COMMANDS, "%04x, %04x - %04x, %04x to %04x, %04x\n", start_x, start_y, end_x, end_y, start_x + m_rex3.m_x_move, start_y + m_rex3.m_y_move);
				do
				{
					do
					{
						m_set_address((start_y + m_rex3.m_y_window - 0x1000) * (1280 + 64) + (start_x + m_rex3.m_x_window - 0x1000));
						uint32_t src = m_read_pixel();

						output_pixel(start_x + m_rex3.m_x_move, start_y + m_rex3.m_y_move, src);

						start_x += dx;
					} while (start_x != end_x && stop_on_x);

					if (start_x == end_x)
					{
						start_x = m_rex3.m_x_save;
						start_y += dy;
					}
				} while (start_y != end_y && stop_on_y);

				write_x_start(start_x << 11);
				write_y_start(start_y << 11);
			}
			break;
	}
}

void newport_base_device::write_x_start(int32_t val)
{
	m_rex3.m_x_start = val & 0x07ffff80;
	m_rex3.m_x_start_frac = (val >> 7) & 0xf;
	m_rex3.m_x_start_i = (int16_t)(val >> 11);
	m_rex3.m_x_start_f = (uint32_t)val & 0x007fff80;
	m_rex3.m_xy_start_i = (m_rex3.m_xy_start_i & 0x0000ffff) | (m_rex3.m_x_start_i << 16);
}

void newport_base_device::write_y_start(int32_t val)
{
	m_rex3.m_y_start = val & 0x07ffff80;
	m_rex3.m_y_start_frac = (val >> 7) & 0xf;
	m_rex3.m_y_start_i = (int16_t)(val >> 11);
	m_rex3.m_y_start_f = (uint32_t)val & 0x007fff80;
	m_rex3.m_xy_start_i = (m_rex3.m_xy_start_i & 0xffff0000) | (uint16_t)m_rex3.m_y_start_i;
}

void newport_base_device::write_x_end(int32_t val)
{
	m_rex3.m_x_end = val & 0x07ffff80;
	m_rex3.m_x_end_frac = (val >> 7) & 0xf;
	m_rex3.m_x_end_i = (int16_t)(val >> 11);
	m_rex3.m_x_end_f = (uint32_t)val & 0x007fff80;
	m_rex3.m_xy_end_i = (m_rex3.m_xy_end_i & 0x0000ffff) | (m_rex3.m_x_end_i << 16);
}

void newport_base_device::write_y_end(int32_t val)
{
	m_rex3.m_y_end = val & 0x07ffff80;
	m_rex3.m_y_end_frac = (val >> 7) & 0xf;
	m_rex3.m_y_end_i = (int16_t)(val >> 11);
	m_rex3.m_y_end_f = (uint32_t)val & 0x007fff80;
	m_rex3.m_xy_end_i = (m_rex3.m_xy_end_i & 0xffff0000) | (uint16_t)m_rex3.m_y_end_i;
}

void newport_base_device::rex3_w(offs_t offset, uint64_t data, uint64_t mem_mask)
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
			switch ((data32 >> 3) & 3)
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
			switch ((data32 >> 19) & 7)
			{
			case 0x00:
				LOGMASKED(LOG_REX3, "    Src Blend Factor:   0\n");
				break;
			case 0x01:
				LOGMASKED(LOG_REX3, "    Src Blend Factor:   1\n");
				break;
			case 0x02:
				LOGMASKED(LOG_REX3, "    Src Blend Factor:   Normalized Dest Color (or COLORBACK)\n");
				break;
			case 0x03:
				LOGMASKED(LOG_REX3, "    Src Blend Factor:   1 - Normalized Dest Color (or COLORBACK)\n");
				break;
			case 0x04:
				LOGMASKED(LOG_REX3, "    Src Blend Factor:   Normalized Src Alpha\n");
				break;
			case 0x05:
				LOGMASKED(LOG_REX3, "    Src Blend Factor:   1 - Normalized Src Alpha\n");
				break;
			default:
				LOGMASKED(LOG_REX3 | LOG_UNKNOWN, "    Unknown Src Blend Factor: %02x\n", (data32 >> 19) & 7);
				break;
			}
			switch ((data32 >> 22) & 7)
			{
			case 0x00:
				LOGMASKED(LOG_REX3, "    Dest Blend Factor:  0\n");
				break;
			case 0x01:
				LOGMASKED(LOG_REX3, "    Dest Blend Factor:  1\n");
				break;
			case 0x02:
				LOGMASKED(LOG_REX3, "    Dest Blend Factor:  Normalized Src Color\n");
				break;
			case 0x03:
				LOGMASKED(LOG_REX3, "    Dest Blend Factor:  1 - Normalized Src Color\n");
				break;
			case 0x04:
				LOGMASKED(LOG_REX3, "    Dest Blend Factor:  Normalized Src Alpha\n");
				break;
			case 0x05:
				LOGMASKED(LOG_REX3, "    Dest Blend Factor:  1 - Normalized Src Alpha\n");
				break;
			default:
				LOGMASKED(LOG_REX3 | LOG_UNKNOWN, "    Unknown Src Blend Factor: %02x\n", (data32 & 0x00380000) >> 19);
				break;
			}
			LOGMASKED(LOG_REX3, "  COLORBACK Dest Blend: %d\n", BIT(data32, 25));
			LOGMASKED(LOG_REX3, "   Enable Pxl Prefetch: %d\n", BIT(data32, 26));
			LOGMASKED(LOG_REX3, "    SFACTOR Src Alpha:  %d\n", BIT(data32, 27));
			switch ((data32 >> 28) & 15)
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

			m_rex3.m_plane_enable = m_rex3.m_draw_mode1 & 7;
			m_rex3.m_plane_depth = (m_rex3.m_draw_mode1 >> 3) & 3;
			m_rex3.m_rwpacked = BIT(m_rex3.m_draw_mode1, 7);
			m_rex3.m_hostdepth = (m_rex3.m_draw_mode1 >> 8) & 3;
			m_rex3.m_rwdouble = BIT(m_rex3.m_draw_mode1, 10);
			m_rex3.m_sfactor = (m_rex3.m_draw_mode1 >> 19) & 7;
			m_rex3.m_dfactor = (m_rex3.m_draw_mode1 >> 22) & 7;
			m_rex3.m_blend_back = BIT(m_rex3.m_draw_mode1, 25);

			m_rex3.m_host_shift = 64 - s_host_shifts[m_rex3.m_hostdepth];

			uint16_t draw_flags = BIT(data32, 18) << 12;
			draw_flags |= BIT(data32, 17) << 11;
			draw_flags |= BIT(data32, 15) << 10;
			draw_flags |= BIT(data32,  5) << 9;
			draw_flags |= (data32 & 7) << 6;
			draw_flags |= (data32 & 0x18) << 1;
			draw_flags |= (data32 >> 28) & 0xf;
			m_draw_flags_w(draw_flags);
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

			m_rex3.m_color_host = BIT(m_rex3.m_draw_mode0, 6);
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
			m_rex3.m_curr_color_red = m_rex3.m_color_red;
			if (!BIT(m_rex3.m_draw_mode1, 15))
			{
				switch (m_rex3.m_plane_depth)
				{
				case 0: // 4bpp
					m_rex3.m_color_i = (uint32_t)((data >> 43) & 0xf);
					break;
				case 1: // 8bpp
					m_rex3.m_color_i = (uint32_t)((data >> 43) & 0xff);
					break;
				case 2: // 12bpp
					m_rex3.m_color_i = (uint32_t)((data >> 41) & 0xfff);
					break;
				case 3: // 32bpp
					// Invalid for CI mode
					break;
				}
			}
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Alpha Full State Write: %08x\n", (uint32_t)data);
			m_rex3.m_color_alpha = (int32_t)(data & 0xfffff);
			m_rex3.m_curr_color_alpha = m_rex3.m_color_alpha;
		}
		break;
	case 0x0208/8:
		if (ACCESSING_BITS_32_63)
		{
			LOGMASKED(LOG_REX3, "REX3 Green Full State Write: %08x\n", (uint32_t)(data >> 32));
			m_rex3.m_color_green = (int32_t)((data >> 32) & 0xfffff);
			m_rex3.m_curr_color_green = m_rex3.m_color_green;
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Blue Full State Write: %08x\n", (uint32_t)data);
			m_rex3.m_color_blue = (int32_t)(data & 0xfffff);
			m_rex3.m_curr_color_blue = m_rex3.m_color_blue;
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
			m_write_mask_w(m_rex3.m_write_mask);
		}
		if (ACCESSING_BITS_0_31)
		{
			LOGMASKED(LOG_REX3, "REX3 Packed Color Write: %08x\n", (uint32_t)data);
			m_rex3.m_color_i = (uint32_t)data;
			if (BIT(m_rex3.m_draw_mode1, 15))
			{
				m_rex3.m_color_red = (data & 0xff) << 11;
				m_rex3.m_color_green = (data & 0xff00) << 3;
				m_rex3.m_color_blue = (data & 0xff0000) >> 5;

				m_rex3.m_curr_color_red = m_rex3.m_color_red;
				m_rex3.m_curr_color_green = m_rex3.m_color_green;
				m_rex3.m_curr_color_blue = m_rex3.m_color_blue;
			}
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
		m_rex3.m_host_shift = 64 - s_host_shifts[m_rex3.m_hostdepth];
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
				m_rex3.m_dcb_mask = 0xffffffff;
				break;
			case 0x01:
				LOGMASKED(LOG_REX3, "    Transfer Width:     1 bytes\n");
				m_rex3.m_dcb_mask = 0x000000ff;
				break;
			case 0x02:
				LOGMASKED(LOG_REX3, "    Transfer Width:     2 bytes\n");
				m_rex3.m_dcb_mask = 0x0000ffff;
				break;
			case 0x03:
				LOGMASKED(LOG_REX3, "    Transfer Width:     3 bytes\n");
				m_rex3.m_dcb_mask = 0xffffffff;
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
			case DCB_ADDR_VC2:
				m_vc2->write(m_rex3.m_dcb_reg_select, data32, m_rex3.m_dcb_mask);
				break;
			case DCB_ADDR_CMAP01:
				m_cmap[0]->write(m_rex3.m_dcb_reg_select, data32);
				m_cmap[1]->write(m_rex3.m_dcb_reg_select, data32);
				break;
			case DCB_ADDR_CMAP0:
				m_cmap[0]->write(m_rex3.m_dcb_reg_select, data32);
				break;
			case DCB_ADDR_CMAP1:
				m_cmap[1]->write(m_rex3.m_dcb_reg_select, data32);
				break;
			case DCB_ADDR_XMAP01:
				m_xmap[0]->write(m_rex3.m_dcb_reg_select, data32);
				m_xmap[1]->write(m_rex3.m_dcb_reg_select, data32);
				break;
			case DCB_ADDR_XMAP0:
				m_xmap[0]->write(m_rex3.m_dcb_reg_select, data32);
				break;
			case DCB_ADDR_XMAP1:
				m_xmap[1]->write(m_rex3.m_dcb_reg_select, data32);
				break;
			case DCB_ADDR_RAMDAC:
				ramdac_write(data32);
				break;
			case DCB_ADDR_CC1:
				LOGMASKED(LOG_REX3, "REX3 Display Control Bus Data MSW Write to CC1 (not yet implemented): %08x\n", data32);
				break;
			case DCB_ADDR_AB1:
				LOGMASKED(LOG_REX3, "REX3 Display Control Bus Data MSW Write to AB1 (not yet implemented): %08x\n", data32);
				break;
			case DCB_ADDR_PCD:
				LOGMASKED(LOG_REX3, "REX3 Display Control Bus Data MSW Write to PCD (not yet implemented): %08x\n", data32);
				// Presenter not connected; simulate a bus timeout
				m_rex3.m_status |= STATUS_BACKBUSY;
				m_dcb_timeout_timer->adjust(attotime::from_msec(1));
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

void newport_base_device::device_add_mconfig(machine_config &config)
{
	VC2(config, m_vc2, 0);
	m_vc2->vert_int().set(FUNC(newport_base_device::vrint_w));
	m_vc2->screen_timing_changed().set(FUNC(newport_base_device::update_screen_size));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	//m_screen->set_size(2048, 2048);
	//m_screen->set_visarea(0, 2047, 0, 2047);
	m_screen->set_size(1280+64, 1024+64);
	m_screen->set_visarea(0, 1279, 0, 1023);
	m_screen->set_screen_update(FUNC(newport_base_device::screen_update));
	m_screen->screen_vblank().set(m_vc2, FUNC(vc2_device::vblank_w));
}

void newport_base_device::device_add_mconfig(machine_config &config, uint32_t xmap_revision, uint32_t cmap_revision, uint32_t global_mask)
{
	newport_base_device::device_add_mconfig(config);

	XMAP9(config, m_xmap[0], 0, xmap_revision);
	XMAP9(config, m_xmap[1], 0, xmap_revision);
	CMAP(config, m_cmap[0], 0, cmap_revision);
	CMAP(config, m_cmap[1], 0, cmap_revision);
	RB2(config, m_rb2, 0, global_mask);
	write_mask().set(m_rb2, FUNC(rb2_device::set_write_mask));
	draw_flags().set(m_rb2, FUNC(rb2_device::set_flags));
	pixel_address().set(m_rb2, FUNC(rb2_device::set_address));
	pixel_write().set(m_rb2, FUNC(rb2_device::write_pixel));
	pixel_read().set(m_rb2, FUNC(rb2_device::read_pixel));
}

void gio64_xl8_device::device_add_mconfig(machine_config &config)
{
	newport_base_device::device_add_mconfig(config, 1, 0xa1, 0x000000ff);
}

void gio64_xl24_device::device_add_mconfig(machine_config &config)
{
	newport_base_device::device_add_mconfig(config, 3, 0x02, 0x00ffffff);
}
