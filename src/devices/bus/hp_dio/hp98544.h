// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_BUS_HPDIO_98544_H
#define MAME_BUS_HPDIO_98544_H

#pragma once

#include "hp_dio.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dio16_98544_device

class dio16_98544_device :
		public device_t,
		public device_dio16_card_interface
{
public:
	// construction/destruction
	dio16_98544_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER(vram_r);
	DECLARE_WRITE16_MEMBER(vram_w);
	DECLARE_READ16_MEMBER(rom_r);
	DECLARE_WRITE16_MEMBER(rom_w);
	DECLARE_READ16_MEMBER(ctrl_r);
	DECLARE_WRITE16_MEMBER(ctrl_w);

	TIMER_CALLBACK_MEMBER(cursor_callback);

 protected:
	dio16_98544_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

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

private:
	enum topcat_reg {
		TOPCAT_REG_VBLANK=0x20,
		TOPCAT_REG_WMOVE_ACTIVE=0x22,
		TOPCAT_REG_VERT_RETRACE_INTRQ=0x24,
		TOPCAT_REG_WMOVE_INTRQ=0x26,
		TOPCAT_REG_DISPLAY_PLANE_ENABLE=0x40,
		TOPCAT_REG_DISPLAY_WRITE_ENABLE_PLANE=0x44,
		TOPCAT_REG_DISPLAY_READ_ENABLE_PLANE=0x46,
		TOPCAT_REG_FB_WRITE_ENABLE=0x48,
		TOPCAT_REG_START_WMOVE=0x4e,
		TOPCAT_REG_ENABLE_BLINK_PLANES=0x50,
		TOPCAT_REG_ENABLE_ALT_FRAME=0x54,
		TOPCAT_REG_CURSOR_CNTL=0x56,
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

	void window_move(void);
	void execute_rule(uint16_t src, replacement_rule_t rule, uint16_t *dst);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void update_cursor(int x, int y, uint8_t ctrl, uint8_t width);
	std::vector<uint16_t> m_vram;
	uint32_t m_palette[2];
	uint8_t *m_rom;

	uint8_t m_vblank;
	uint8_t m_wmove_active;
	uint8_t m_vert_retrace_intrq;
	uint8_t m_wmove_intrq;
	uint8_t m_display_enable_planes;
	uint8_t m_write_enable_plane;
	uint8_t m_read_enable_plane;
	uint8_t m_fb_write_enable;
	uint8_t m_enable_blink_planes;
	uint8_t m_enable_alt_frame;
	uint8_t m_cursor_ctrl;
	uint8_t m_move_replacement_rule;
	uint8_t m_pixel_replacement_rule;
	uint16_t m_source_x_pixel;
	uint16_t m_source_y_pixel;
	uint16_t m_dst_x_pixel;
	uint16_t m_dst_y_pixel;
	uint16_t m_block_mover_pixel_width;
	uint16_t m_block_mover_pixel_height;

	emu_timer *m_cursor_timer;
	bool m_cursor_state;
	uint16_t m_cursor_x_pos;
	uint16_t m_cursor_y_pos;
	uint16_t m_cursor_width;
};

// device type definition
DECLARE_DEVICE_TYPE(HPDIO_98544, dio16_98544_device)

#endif // MAME_BUS_HPDIO_98544_H
