// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
#ifndef MAME_VIDEO_TOPCAT_H
#define MAME_VIDEO_TOPCAT_H

#pragma once

class topcat_device : public device_t
{
public:
	topcat_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	bool has_changed() { bool ret = m_changed; m_changed = false; return ret; }
	void set_fb_width(int _pixels) { m_fb_width = _pixels; }
	void set_fb_height(int _pixels) { m_fb_height = _pixels; }
	void set_planemask(int _mask) { m_plane_mask = _mask; }
	void get_cursor_pos(int &startx, int &starty, int &endx, int &endy);

	uint16_t vram_r(offs_t offset, uint16_t mem_mask = ~0);
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ctrl_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	void ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void vblank_w(int state);
	void topcat_mem(address_map &map) ATTR_COLD;

	bool plane_enabled();

	auto irq_out_cb() { return m_int_write_func.bind(); }
protected:
	topcat_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(cursor_callback);

private:

	typedef enum {
		TOPCAT_REPLACE_RULE_CLEAR, /* 0 */
		TOPCAT_REPLACE_RULE_SRC_AND_DST,
		TOPCAT_REPLACE_RULE_SRC_AND_NOT_DST,
		TOPCAT_REPLACE_RULE_SRC,
		TOPCAT_REPLACE_RULE_NOT_SRC_AND_DST,
		TOPCAT_REPLACE_RULE_NOP,
		TOPCAT_REPLACE_RULE_SRC_XOR_DST,
		TOPCAT_REPLACE_RULE_SRC_OR_DST,
		TOPCAT_REPLACE_RULE_NOT_SRC_AND_NOT_DST,
		TOPCAT_REPLACE_RULE_NOT_SRC_XOR_DST,
		TOPCAT_REPLACE_RULE_NOT_DST,
		TOPCAT_REPLACE_RULE_SRC_OR_NOT_DST,
		TOPCAT_REPLACE_RULE_NOT_SRC,
		TOPCAT_REPLACE_RULE_NOT_SRC_OR_DST,
		TOPCAT_REPLACE_RULE_NOT_SRC_OR_NOT_DST,
		TOPCAT_REPLACE_RULE_SET,
	} replacement_rule_t;

	enum topcat_reg {
		TOPCAT_REG_VBLANK=0x20,
		TOPCAT_REG_WMOVE_ACTIVE=0x22,
		TOPCAT_REG_VERT_RETRACE_INTRQ=0x24,
		TOPCAT_REG_WMOVE_INTRQ=0x26,
		TOPCAT_REG_DISPLAY_PLANE_ENABLE=0x40,
		TOPCAT_REG_WRITE_ENABLE_PLANE=0x44,
		TOPCAT_REG_READ_ENABLE_PLANE=0x46,
		TOPCAT_REG_FB_WRITE_ENABLE=0x48,
		TOPCAT_REG_WMOVE_IE=0x4a,
		TOPCAT_REG_VBLANK_IE=0x4c,
		TOPCAT_REG_START_WMOVE=0x4e,
		TOPCAT_REG_ENABLE_BLINK_PLANES=0x50,
		TOPCAT_REG_ENABLE_ALT_FRAME=0x54,
		TOPCAT_REG_CURSOR_PLANE_ENABLE=0x56,
		TOPCAT_REG_PIXEL_REPLACE_RULE=0x75,
		TOPCAT_REG_MOVE_REPLACE_RULE=0x77,
		TOPCAT_REG_SOURCE_X_PIXEL=0x79,
		TOPCAT_REG_SOURCE_Y_PIXEL=0x7b,
		TOPCAT_REG_DST_X_PIXEL=0x7d,
		TOPCAT_REG_DST_Y_PIXEL=0x7f,
		TOPCAT_REG_BLOCK_MOVER_PIXEL_WIDTH=0x81,
		TOPCAT_REG_BLOCK_MOVER_PIXEL_HEIGHT=0x83,
		TOPCAT_REG_CURSOR_X_POS=0x85,
		TOPCAT_REG_CURSOR_Y_POS=0x87,
		TOPCAT_REG_CURSOR_WIDTH=0x89,
	};

	void window_move();

	void execute_rule(bool src, replacement_rule_t rule, bool &dst);

	void update_cursor(int x, int y, uint16_t ctrl, uint8_t width);

	void modify_vram(int x, int y, bool state) {
		if (state)
			m_vram[y * m_fb_width + x] |= m_plane_mask;
		else
			m_vram[y * m_fb_width + x] &= ~m_plane_mask;
	}

	void modify_vram_offset(int offset, bool state) {
		if (state)
			m_vram[offset] |= m_plane_mask;
		else
			m_vram[offset] &= ~m_plane_mask;
	}

	bool get_vram_pixel(int x, int y) const {
		return m_vram[y * m_fb_width + x] & m_plane_mask;
	}

	void update_int();

	devcb_write_line m_int_write_func;

	uint16_t m_vblank = 0;
	uint8_t m_wmove_active = 0;
	uint16_t m_vert_retrace_intrq = 0;
	uint16_t m_wmove_intrq = 0;
	uint16_t m_display_enable_planes = 0;
	uint16_t m_fb_write_enable = 0;
	uint16_t m_enable_blink_planes = 0;
	uint16_t m_enable_alt_frame = 0;
	uint16_t m_cursor_plane_enable = 0;
	uint16_t m_move_replacement_rule = 0;
	uint16_t m_pixel_replacement_rule = 0;
	uint16_t m_source_x_pixel = 0;
	uint16_t m_source_y_pixel = 0;
	uint16_t m_dst_x_pixel = 0;
	uint16_t m_dst_y_pixel = 0;
	uint16_t m_block_mover_pixel_width = 0;
	uint16_t m_block_mover_pixel_height = 0;
	uint16_t m_unknown_reg4a = 0;
	uint16_t m_unknown_reg4c = 0;
	emu_timer *m_cursor_timer = nullptr;
	bool m_cursor_state = false;
	uint16_t m_cursor_x_pos = 0;
	uint16_t m_cursor_y_pos = 0;
	uint16_t m_cursor_width = 0;

	int m_fb_width = 0;
	int m_fb_height = 0;
	uint8_t m_plane_mask = 0;

	bool m_read_enable = false;
	bool m_write_enable = false;
	bool m_fb_enable = false;
	bool m_changed = false;

	required_shared_ptr<uint8_t> m_vram;
};

DECLARE_DEVICE_TYPE(TOPCAT, topcat_device)
#endif // MAME_VIDEO_TOPCAT_H
