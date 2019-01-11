// license:BSD-3-Clause
// copyright-holders:Sven Schnelle

#include "emu.h"
#include "topcat.h"

// define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(TOPCAT, topcat_device, "topcat", "HP Topcat ASIC")

#define VRAM_SIZE   (0x100000)

topcat_device::topcat_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
{
}

topcat_device::topcat_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: topcat_device(mconfig, TOPCAT, tag, owner, clock)
{
}

void topcat_device::device_start()
{
	m_vram.resize(VRAM_SIZE);

	m_cursor_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(topcat_device::cursor_callback),this));
	m_cursor_timer->adjust(attotime::from_hz(3));
}

void topcat_device::device_reset()
{
	memset(&m_vram[0], 0, VRAM_SIZE);

	m_palette[1] = rgb_t(255, 255, 255);
	m_palette[0] = rgb_t(0, 0, 0);
	m_pixel_replacement_rule = TOPCAT_REPLACE_RULE_SRC;
}

READ16_MEMBER(topcat_device::vram_r)
{
	return m_vram[offset];
}

WRITE16_MEMBER(topcat_device::vram_w)
{
	COMBINE_DATA(&m_vram[offset]);
}

TIMER_CALLBACK_MEMBER(topcat_device::cursor_callback)
{
	m_cursor_timer->adjust(attotime::from_hz(5));
	m_cursor_state ^= true;

	if (m_cursor_ctrl & 0x02) {
		for(int i = 0; i < m_cursor_width; i++) {
			m_vram[(m_cursor_y_pos * 512) + (m_cursor_x_pos + i)/2] = m_cursor_state ? 0xffff : 0;
			m_vram[((m_cursor_y_pos-1) * 512) + (m_cursor_x_pos + i)/2] = m_cursor_state ? 0xffff : 0;
			m_vram[((m_cursor_y_pos-2) * 512) + (m_cursor_x_pos + i)/2] = m_cursor_state ? 0xffff : 0;
		}
	}
}

void topcat_device::update_cursor(int x, int y, uint8_t ctrl, uint8_t width)
{
	for(int i = 0; i < m_cursor_width; i++) {
		m_vram[(m_cursor_y_pos * 512) + (m_cursor_x_pos + i)/2] = 0;
		m_vram[((m_cursor_y_pos-1) * 512) + (m_cursor_x_pos + i)/2] = 0;
		m_vram[((m_cursor_y_pos-2) * 512) + (m_cursor_x_pos + i)/2] = 0;
	}
	m_cursor_x_pos = x;
	m_cursor_y_pos = y;
	m_cursor_ctrl = ctrl;
	m_cursor_width = width;
}

void topcat_device::execute_rule(uint16_t src, replacement_rule_t rule, uint16_t *dst)
{
	switch(rule & 0x0f) {
	case TOPCAT_REPLACE_RULE_CLEAR:
		*dst = 0;
		break;
	case TOPCAT_REPLACE_RULE_SRC_AND_DST:
		*dst &= src;
		break;
	case TOPCAT_REPLACE_RULE_SRC_AND_NOT_DST:
		*dst = ~(*dst) & src;
		break;
	case TOPCAT_REPLACE_RULE_SRC:
		*dst = src;
		break;
	case TOPCAT_REPLACE_RULE_NOT_SRC_AND_DST:
		*dst &= ~src;
		break;
	case TOPCAT_REPLACE_RULE_NOP:
		break;
	case TOPCAT_REPLACE_RULE_SRC_XOR_DST:
		*dst ^= src;
		break;
	case TOPCAT_REPLACE_RULE_SRC_OR_DST:
		*dst |= src;
		break;
	case TOPCAT_REPLACE_RULE_NOT_SRC_AND_NOT_DST:
		*dst = ~(*dst) & ~src;
		break;
	case TOPCAT_REPLACE_RULE_NOT_SRC_XOR_DST:
		*dst ^= ~src;
		break;
	case TOPCAT_REPLACE_RULE_NOT_DST:
		*dst ^= 0xffff;
		break;
	case TOPCAT_REPLACE_RULE_SRC_OR_NOT_DST:
		*dst = src | ~(*dst);
		break;
	case TOPCAT_REPLACE_RULE_NOT_SRC:
		*dst = ~src;
		break;
	case TOPCAT_REPLACE_RULE_NOT_SRC_OR_DST:
		*dst = ~src | *dst;
		break;
	case TOPCAT_REPLACE_RULE_NOT_SRC_OR_NOT_DST:
		*dst = ~src | ~(*dst);
		break;
	case TOPCAT_REPLACE_RULE_SET:
		*dst = 0xffff;
		break;

	}
}

void topcat_device::window_move(void)
{
	for(int line = 0; line < m_block_mover_pixel_height; line++) {
		for(int column = 0; column < m_block_mover_pixel_width; column++) {
			uint16_t sdata = m_vram[((m_source_y_pixel + line) * 1024 + (m_source_x_pixel + column))/2];
			uint16_t *ddata = &m_vram[((m_dst_y_pixel + line) * 1024 + (m_dst_x_pixel + column))/2];
			execute_rule(sdata, (replacement_rule_t)((m_move_replacement_rule >> 4) & 0x0f), ddata);
			execute_rule(sdata, (replacement_rule_t)(m_move_replacement_rule & 0x0f), ddata);
		}
	}
}

READ16_MEMBER(topcat_device::ctrl_r)
{

	uint16_t ret = 0xffff;

	switch(offset) {
	case TOPCAT_REG_VBLANK:
		ret = m_vblank;
		break;
	case TOPCAT_REG_WMOVE_ACTIVE:
		ret = m_wmove_active;
		break;
	case TOPCAT_REG_VERT_RETRACE_INTRQ:
		ret = m_vert_retrace_intrq;
		break;
	case TOPCAT_REG_WMOVE_INTRQ:
		ret = m_wmove_intrq;
		break;
	case TOPCAT_REG_DISPLAY_PLANE_ENABLE:
		ret = m_display_enable_planes;
		break;
	case TOPCAT_REG_DISPLAY_WRITE_ENABLE_PLANE:
		ret = m_write_enable_plane;
		break;
	case TOPCAT_REG_DISPLAY_READ_ENABLE_PLANE:
		ret = m_read_enable_plane;
		break;
	case TOPCAT_REG_FB_WRITE_ENABLE:
		ret = m_fb_write_enable;
		break;
	case TOPCAT_REG_START_WMOVE:
		ret = 0;
		break;
	case TOPCAT_REG_ENABLE_BLINK_PLANES:
		ret = m_enable_blink_planes;
		break;
	case TOPCAT_REG_ENABLE_ALT_FRAME:
		ret = m_enable_alt_frame;
		break;
	case TOPCAT_REG_CURSOR_CNTL:
		ret = m_cursor_ctrl;
		break;
	case TOPCAT_REG_PIXEL_REPLACE_RULE:
		ret = m_pixel_replacement_rule;
		break;
	case TOPCAT_REG_MOVE_REPLACE_RULE:
		ret = m_move_replacement_rule;
		break;
	case TOPCAT_REG_SOURCE_X_PIXEL:
		ret = m_source_x_pixel;
		break;
	case TOPCAT_REG_SOURCE_Y_PIXEL:
		ret = m_source_y_pixel;
		break;
	case TOPCAT_REG_DST_X_PIXEL:
		ret = m_dst_x_pixel;
		break;
	case TOPCAT_REG_DST_Y_PIXEL:
		ret = m_dst_y_pixel;
		break;
	case TOPCAT_REG_BLOCK_MOVER_PIXEL_WIDTH:
		ret = m_block_mover_pixel_width;
		break;
	case TOPCAT_REG_BLOCK_MOVER_PIXEL_HEIGHT:
		ret = m_block_mover_pixel_height;
		break;
	default:
		logerror("unknown register read %02x\n", offset);
		return space.unmap();
	}
	return ret;
}

WRITE16_MEMBER(topcat_device::ctrl_w)
{
	if (mem_mask == 0xff00)
		data >>= 8;

	if (mem_mask == 0x00ff) {
		logerror("%s: write ignored\n", __FUNCTION__);
		return;
	}

	switch(offset) {
	case TOPCAT_REG_VBLANK:
		m_vblank = data & 0xff;
		break;
	case TOPCAT_REG_WMOVE_ACTIVE:
		break;
	case TOPCAT_REG_VERT_RETRACE_INTRQ:
		m_vert_retrace_intrq = data;
		break;
	case TOPCAT_REG_WMOVE_INTRQ:
		m_wmove_intrq = data;
		break;
	case TOPCAT_REG_DISPLAY_PLANE_ENABLE:
		m_display_enable_planes = data;
		break;
	case TOPCAT_REG_DISPLAY_WRITE_ENABLE_PLANE:
		m_write_enable_plane = data;
		break;
	case TOPCAT_REG_DISPLAY_READ_ENABLE_PLANE:
		m_read_enable_plane = data;
		break;
	case TOPCAT_REG_FB_WRITE_ENABLE:
		m_fb_write_enable = data;
		break;
	case TOPCAT_REG_START_WMOVE:
		window_move();
		break;
	case TOPCAT_REG_ENABLE_BLINK_PLANES:
		logerror("ENABLE_BLINK_PLANES: %04x\n", data);
		m_enable_blink_planes = data;
		break;
	case TOPCAT_REG_ENABLE_ALT_FRAME:
		logerror("ENABLE_ALT_PLANE: %04x\n", data);
		m_enable_alt_frame = data;
		break;
	case TOPCAT_REG_PIXEL_REPLACE_RULE:
		logerror("PIXEL RR: data %04X mask %04X\n", data, mem_mask);
		m_pixel_replacement_rule = data;
		break;
	case TOPCAT_REG_MOVE_REPLACE_RULE:
		logerror("MOVE RR: data %04X mask %04X\n", data, mem_mask);
		m_move_replacement_rule = data;
		break;
	case TOPCAT_REG_SOURCE_X_PIXEL:
		m_source_x_pixel = data;
		break;
	case TOPCAT_REG_SOURCE_Y_PIXEL:
		m_source_y_pixel = data;
		break;
	case TOPCAT_REG_DST_X_PIXEL:
		m_dst_x_pixel = data;
		break;
	case TOPCAT_REG_DST_Y_PIXEL:
		m_dst_y_pixel = data;
		break;
	case TOPCAT_REG_BLOCK_MOVER_PIXEL_WIDTH:
		m_block_mover_pixel_width = data;
		break;
	case TOPCAT_REG_BLOCK_MOVER_PIXEL_HEIGHT:
		m_block_mover_pixel_height = data;
		break;
	case TOPCAT_REG_CURSOR_CNTL:
		update_cursor(m_cursor_x_pos, m_cursor_y_pos, data, m_cursor_width);
		break;
	case TOPCAT_REG_CURSOR_X_POS:
		update_cursor(data, m_cursor_y_pos, m_cursor_ctrl, m_cursor_width);
		break;
	case TOPCAT_REG_CURSOR_Y_POS:
		update_cursor(m_cursor_x_pos, data, m_cursor_ctrl, m_cursor_width);
		break;
	case TOPCAT_REG_CURSOR_WIDTH:
		update_cursor(m_cursor_x_pos, m_cursor_y_pos, m_cursor_ctrl, data);
		break;
	default:
		logerror("unknown register: %02X = %04x\n", offset, data, mem_mask);
		break;
	}
}

uint32_t topcat_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t *scanline;
	int x, y;
	uint32_t pixels;

	for (y = 0; y < 768; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 1024/2; x++)
		{
			pixels = m_vram[(y * 512) + x];

			*scanline++ = m_palette[(pixels>>8) & 1];
			*scanline++ = m_palette[(pixels & 1)];
		}
	}
	return 0;
}
