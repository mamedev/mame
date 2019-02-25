// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Tyson Smith
/*
        Silicon Graphics LG1 "Light" graphics board used as
        entry level graphics in the Indigo and IRIS Crimson.
*/

#include "emu.h"
#include "video/light.h"
#include "screen.h"

#define LOG_REX (1 << 0)
#define LOG_ALL (LOG_REX)

#define VERBOSE (0)
#include "logmacro.h"

enum
{
	REX15_PAGE0_SET             = 0x00000000,
	REX15_PAGE0_GO              = 0x00000800,
	REX15_PAGE1_SET             = 0x00004790,
	REX15_PAGE1_GO              = 0x00004f90,

	REX15_P0REG_COMMAND         = 0x00000000,
	REX15_P0REG_XSTARTI         = 0x0000000c,
	REX15_P0REG_YSTARTI         = 0x0000001c,
	REX15_P0REG_XYMOVE          = 0x00000034,
	REX15_P0REG_COLORREDI       = 0x00000038,
	REX15_P0REG_COLORGREENI     = 0x00000040,
	REX15_P0REG_COLORBLUEI      = 0x00000048,
	REX15_P0REG_COLORBACK       = 0x0000005c,
	REX15_P0REG_ZPATTERN        = 0x00000060,
	REX15_P0REG_XENDI           = 0x00000084,
	REX15_P0REG_YENDI           = 0x00000088,

	REX15_P1REG_WCLOCKREV       = 0x00000054,
	REX15_P1REG_CFGDATA         = 0x00000058,
	REX15_P1REG_CFGSEL          = 0x0000005c,
	REX15_P1REG_VC1_ADDRDATA    = 0x00000060,
	REX15_P1REG_CFGMODE         = 0x00000068,
	REX15_P1REG_XYOFFSET        = 0x0000006c,

	REX15_OP_NOP                = 0x00000000,
	REX15_OP_DRAW               = 0x00000001,

	REX15_OP_FLAG_BLOCK         = 0x00000008,
	REX15_OP_FLAG_LENGTH32      = 0x00000010,
	REX15_OP_FLAG_QUADMODE      = 0x00000020,
	REX15_OP_FLAG_XYCONTINUE    = 0x00000080,
	REX15_OP_FLAG_STOPONX       = 0x00000100,
	REX15_OP_FLAG_STOPONY       = 0x00000200,
	REX15_OP_FLAG_ENZPATTERN    = 0x00000400,
	REX15_OP_FLAG_LOGICSRC      = 0x00080000,
	REX15_OP_FLAG_ZOPAQUE       = 0x00800000,
	REX15_OP_FLAG_ZCONTINUE     = 0x01000000,

	REX15_WRITE_ADDR            = 0x00,
	REX15_PALETTE_RAM           = 0x01,
	REX15_PIXEL_READ_MASK       = 0x02,
	REX15_CONTROL               = 0x06
};

DEFINE_DEVICE_TYPE(LIGHT_VIDEO, light_video_device, "light_video", "SGI Light graphics board")


light_video_device::light_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, LIGHT_VIDEO, tag, owner, clock)
	, m_palette(*this, "palette")
{
}


//-------------------------------------------------
//  device_add_mconfig - device-specific machine configuration
//-------------------------------------------------

void light_video_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(x_res, y_res);
	screen.set_visarea(0, x_res-1, 0, y_res-1);
	screen.set_screen_update(FUNC(light_video_device::screen_update));

	PALETTE(config, m_palette).set_entries(256);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void light_video_device::device_start()
{
	m_framebuffer = std::make_unique<uint8_t[]>(x_res*y_res);

	save_item(NAME(m_lg1.m_config_sel));
	save_item(NAME(m_lg1.m_write_addr));
	save_item(NAME(m_lg1.m_control));
	save_item(NAME(m_lg1.m_command));
	save_item(NAME(m_lg1.m_x_start_i));
	save_item(NAME(m_lg1.m_y_start_i));
	save_item(NAME(m_lg1.m_xy_move));
	save_item(NAME(m_lg1.m_color_red_i));
	save_item(NAME(m_lg1.m_color_green_i));
	save_item(NAME(m_lg1.m_color_blue_i));
	save_item(NAME(m_lg1.m_color_back));
	save_item(NAME(m_lg1.m_z_pattern));
	save_item(NAME(m_lg1.m_x_end_i));
	save_item(NAME(m_lg1.m_y_end_i));
	save_item(NAME(m_lg1.m_x_curr_i));
	save_item(NAME(m_lg1.m_y_curr_i));
	save_item(NAME(m_lg1.m_palette_idx));
	save_item(NAME(m_lg1.m_palette_channel));
	save_item(NAME(m_lg1.m_palette_entry));
	save_item(NAME(m_lg1.m_pix_read_mask));

	save_pointer(NAME(&m_framebuffer[0]), x_res*y_res);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void light_video_device::device_reset()
{
	memset(&m_lg1, 0, sizeof(lg1_t));
	memset(&m_framebuffer[0], 0, x_res*y_res);
}

uint32_t light_video_device::screen_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const rgb_t *pens = m_palette->palette()->entry_list_raw();
	const uint8_t *src = &m_framebuffer[0];
	for (uint32_t y = 0; y < y_res; y++) {
		uint32_t *dst = &bitmap.pix32(y);
		for (uint32_t x = 0; x < x_res; x++) {
			*dst++ = pens[*src++];
		}
	}
	return 0;
}

READ32_MEMBER(light_video_device::entry_r)
{
	uint32_t ret = 0;
	switch (offset)
	{
	case REX15_PAGE0_GO/4:
		LOGMASKED(LOG_REX, "%s: LG1 Read: Status(?) (Go) %08x = %08x & %08x\n", machine().describe_context(), 0x1f3f0000 + offset*4, ret, mem_mask);
		do_rex_command();
		break;
	case 0x0014/4:
		ret = 0x033c0000;
		LOGMASKED(LOG_REX, "%s: LG1 Read: Presence Detect(?) %08x = %08x & %08x\n", machine().describe_context(), 0x1f3f0000 + offset*4, ret, mem_mask);
		break;
	default:
		LOGMASKED(LOG_REX, "%s: Unknown LG1 Read: %08x = %08x & %08x\n", machine().describe_context(), 0x1f3f0000 + offset*4, ret, mem_mask);
		break;
	}
	return ret;
}

WRITE32_MEMBER(light_video_device::entry_w)
{
	bool go = (offset >= REX15_PAGE1_GO/4) || (offset >= REX15_PAGE0_GO/4 && offset < REX15_PAGE1_SET/4);

	switch (offset)
	{
	case (REX15_PAGE0_SET+REX15_P0REG_COMMAND)/4:
	case (REX15_PAGE0_GO+REX15_P0REG_COMMAND)/4:
		m_lg1.m_command = data;
		LOGMASKED(LOG_REX, "%s: LG1 REX1.5 Command Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
		if (go)
			do_rex_command();
		break;
	case (REX15_PAGE0_SET+REX15_P0REG_XSTARTI)/4:
	case (REX15_PAGE0_GO+REX15_P0REG_XSTARTI)/4:
		m_lg1.m_x_start_i = data;
		m_lg1.m_x_curr_i = m_lg1.m_x_start_i;
		LOGMASKED(LOG_REX, "%s: LG1 REX1.5 XStartI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
		break;
	case (REX15_PAGE0_SET+REX15_P0REG_YSTARTI)/4:
	case (REX15_PAGE0_GO+REX15_P0REG_YSTARTI)/4:
		m_lg1.m_y_start_i = data;
		m_lg1.m_y_curr_i = m_lg1.m_y_start_i;
		LOGMASKED(LOG_REX, "%s: LG1 REX1.5 YStartI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
		break;
	case (REX15_PAGE0_SET+REX15_P0REG_XYMOVE)/4:
	case (REX15_PAGE0_GO+REX15_P0REG_XYMOVE)/4:
		m_lg1.m_xy_move = data;
		LOGMASKED(LOG_REX, "%s: LG1 REX1.5 XYMove Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
		if (go)
			do_rex_command();
		break;
	case (REX15_PAGE0_SET+REX15_P0REG_COLORREDI)/4:
	case (REX15_PAGE0_GO+REX15_P0REG_COLORREDI)/4:
		m_lg1.m_color_red_i = data;
		LOGMASKED(LOG_REX, "%s: LG1 REX1.5 ColorRedI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
		break;
	case (REX15_PAGE0_SET+REX15_P0REG_COLORGREENI)/4:
	case (REX15_PAGE0_GO+REX15_P0REG_COLORGREENI)/4:
		m_lg1.m_color_green_i = data;
		LOGMASKED(LOG_REX, "%s: LG1 REX1.5 ColorGreenI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
		break;
	case (REX15_PAGE0_SET+REX15_P0REG_COLORBLUEI)/4:
	case (REX15_PAGE0_GO+REX15_P0REG_COLORBLUEI)/4:
		m_lg1.m_color_blue_i = data;
		LOGMASKED(LOG_REX, "%s: LG1 REX1.5 ColorBlueI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
		break;
	case (REX15_PAGE0_SET+REX15_P0REG_COLORBACK)/4:
	case (REX15_PAGE0_GO+REX15_P0REG_COLORBACK)/4:
		m_lg1.m_color_back = data;
		LOGMASKED(LOG_REX, "%s: LG1 REX1.5 ColorBlueI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
		break;
	case (REX15_PAGE0_SET+REX15_P0REG_ZPATTERN)/4:
	case (REX15_PAGE0_GO+REX15_P0REG_ZPATTERN)/4:
		m_lg1.m_z_pattern = data;
		LOGMASKED(LOG_REX, "%s: LG1 REX1.5 ZPattern Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
		if (go)
			do_rex_command();
		break;
	case (REX15_PAGE0_SET+REX15_P0REG_XENDI)/4:
	case (REX15_PAGE0_GO+REX15_P0REG_XENDI)/4:
		m_lg1.m_x_end_i = data;
		LOGMASKED(LOG_REX, "%s: LG1 REX1.5 XEndI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
		break;
	case (REX15_PAGE0_SET+REX15_P0REG_YENDI)/4:
	case (REX15_PAGE0_GO+REX15_P0REG_YENDI)/4:
		m_lg1.m_y_end_i = data;
		LOGMASKED(LOG_REX, "%s: LG1 REX1.5 YEndI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
		break;

	case (REX15_PAGE1_SET+REX15_P1REG_CFGSEL)/4:
	case (REX15_PAGE1_GO+REX15_P1REG_CFGSEL)/4:
		m_lg1.m_config_sel = data;
		switch (data)
		{
		case REX15_WRITE_ADDR:
			LOGMASKED(LOG_REX, "%s: LG1 ConfigSel Write = %08x (Write Addr)\n", machine().describe_context(), data);
			break;
		case REX15_PALETTE_RAM:
			LOGMASKED(LOG_REX, "%s: LG1 ConfigSel Write = %08x (Palette RAM)\n", machine().describe_context(), data);
			break;
		case REX15_CONTROL:
			LOGMASKED(LOG_REX, "%s: LG1 ConfigSel Write = %08x (Control)\n", machine().describe_context(), data);
			break;
		case REX15_PIXEL_READ_MASK:
			LOGMASKED(LOG_REX, "%s: LG1 ConfigSel Write = %08x (Pixel Read Mask)\n", machine().describe_context(), data);
			break;
		default:
			LOGMASKED(LOG_REX, "%s: LG1 ConfigSel Write = %08x (Unknown)\n", machine().describe_context(), data);
			break;
		}
		break;
	case (REX15_PAGE1_SET+REX15_P1REG_CFGDATA)/4:
	case (REX15_PAGE1_GO+REX15_P1REG_CFGDATA)/4:
		if (go) // Ignore 'Go' writes for now, unsure what they do
			break;
		switch (m_lg1.m_config_sel)
		{
		case REX15_WRITE_ADDR:
			LOGMASKED(LOG_REX, "%s: LG1 ConfigSel Write: Setting WriteAddr to %08x\n", machine().describe_context(), data);
			m_lg1.m_write_addr = data;
			if (m_lg1.m_control == 0xf)
			{
				LOGMASKED(LOG_REX, "%s: LG1 about to set palette entry %02x\n", machine().describe_context(), (uint8_t)data);
				m_lg1.m_palette_idx = (uint8_t)data;
				m_lg1.m_palette_channel = 0;
			}
			break;
		case REX15_PALETTE_RAM:
			LOGMASKED(LOG_REX, "%s: LG1 ConfigSel Write: Setting Palette RAM %02x entry %d to %02x\n", machine().describe_context(),
				m_lg1.m_palette_idx, m_lg1.m_palette_channel, (uint8_t)data);
			m_lg1.m_palette_entry[m_lg1.m_palette_channel++] = (uint8_t)data;
			if (m_lg1.m_palette_channel == 3)
			{
				m_palette->set_pen_color(m_lg1.m_palette_idx, m_lg1.m_palette_entry[0], m_lg1.m_palette_entry[1], m_lg1.m_palette_entry[2]);
			}
			break;
		case REX15_CONTROL:
			LOGMASKED(LOG_REX, "%s: LG1 ConfigSel Write: Setting Control to %08x\n", machine().describe_context(), data);
			m_lg1.m_control = data;
			break;
		case REX15_PIXEL_READ_MASK:
			LOGMASKED(LOG_REX, "%s: LG1 ConfigSel Write = %08x (Pixel Read Mask, entry %02x)\n", machine().describe_context(), data, m_lg1.m_palette_idx);
			m_lg1.m_pix_read_mask[m_lg1.m_palette_idx] = (uint8_t)data;
			break;
		default:
			LOGMASKED(LOG_REX, "%s: LG1 Unknown ConfigData Write = %08x\n", machine().describe_context(), data);
			break;
		}
		break;
	default:
		LOGMASKED(LOG_REX, "%s: Unknown LG1 Write: %08x = %08x & %08x\n", machine().describe_context(), 0x1f3f0000 + offset*4, data, mem_mask);
		break;
	}
}

void light_video_device::do_rex_command()
{
	if (m_lg1.m_command == 0)
	{
		return;
	}
	if (m_lg1.m_command == 0x30080329)
	{
		bool xycontinue = (m_lg1.m_command & REX15_OP_FLAG_XYCONTINUE);
		bool copy = (m_lg1.m_command & REX15_OP_FLAG_LOGICSRC);
		const uint32_t start_x = xycontinue ? m_lg1.m_x_curr_i : m_lg1.m_x_start_i;
		const uint32_t start_y = xycontinue ? m_lg1.m_y_curr_i : m_lg1.m_y_start_i;
		const uint32_t end_x = m_lg1.m_x_end_i;
		const uint32_t end_y = m_lg1.m_y_end_i;
		const uint32_t src_start_x = start_x + (m_lg1.m_xy_move >> 16);
		const uint32_t src_start_y = start_y + (uint16_t)m_lg1.m_xy_move;;

		LOGMASKED(LOG_REX, "LG1: Command %08x: Block copy from %d,%d-%d,%d inclusive.\n", m_lg1.m_command, start_x, start_y, end_x, end_y);
		if (copy)
		{
			for (uint32_t y = start_y, src_y = src_start_y; y <= end_y; y++, src_y++)
				for (uint32_t x = start_x, src_x = src_start_x; x <= end_x; x++, src_x++)
					m_framebuffer[y*x_res + x] = m_framebuffer[src_y*x_res + src_x];
		}
		else
		{
			for (uint32_t y = start_y; y <= end_y; y++)
				for (uint32_t x = start_x; x <= end_x; x++)
					m_framebuffer[y*x_res + x] = m_lg1.m_color_red_i;
		}
	}
	else if (m_lg1.m_command == 0x30000329)
	{
		bool xycontinue = (m_lg1.m_command & REX15_OP_FLAG_XYCONTINUE);
		uint32_t start_x = xycontinue ? m_lg1.m_x_curr_i : m_lg1.m_x_start_i;
		uint32_t start_y = xycontinue ? m_lg1.m_y_curr_i : m_lg1.m_y_start_i;
		uint32_t end_x = m_lg1.m_x_end_i;
		uint32_t end_y = m_lg1.m_y_end_i;

		LOGMASKED(LOG_REX, "LG1: Command %08x: Block draw from %d,%d-%d,%d inclusive.\n", m_lg1.m_command, start_x, start_y, end_x, end_y);
		for (uint32_t y = start_y; y <= end_y; y++)
		{
			for (uint32_t x = start_x; x <= end_x; x++)
			{
				m_framebuffer[y*x_res + x] = m_lg1.m_color_red_i;
			}
		}
	}
	else if (m_lg1.m_command == 0x300005a1 ||
		m_lg1.m_command == 0x300005a9 ||
		m_lg1.m_command == 0x300005b9)
	{
		bool xycontinue = (m_lg1.m_command & REX15_OP_FLAG_XYCONTINUE);
		uint32_t start_x = xycontinue ? m_lg1.m_x_curr_i : m_lg1.m_x_start_i;
		uint32_t start_y = xycontinue ? m_lg1.m_y_curr_i : m_lg1.m_y_start_i;
		uint32_t end_x = m_lg1.m_x_end_i;
		LOGMASKED(LOG_REX, "LG1: Command %08x: Pattern draw from %d-%d at %d\n", m_lg1.m_command, start_x, end_x, start_y);
		for (uint32_t x = start_x; x <= end_x && x < (start_x + 32); x++)
		{
			if (BIT(m_lg1.m_z_pattern, 31 - (x - start_x)))
			{
				m_framebuffer[start_y*x_res + x] = m_lg1.m_color_red_i;
			}
			m_lg1.m_x_curr_i++;
		}

		if (m_lg1.m_command & REX15_OP_FLAG_BLOCK)
		{
			if (m_lg1.m_x_curr_i > m_lg1.m_x_end_i)
			{
				m_lg1.m_y_curr_i--;
				m_lg1.m_x_curr_i = m_lg1.m_x_start_i;
			}
		}
	}
	else
	{
		LOGMASKED(LOG_REX, "%s: Unknown LG1 command: %08x\n", machine().describe_context(), m_lg1.m_command);
	}
}
