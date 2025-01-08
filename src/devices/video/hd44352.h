// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Hitachi HD44352 LCD controller

***************************************************************************/

#ifndef MAME_VIDEO_HD44352_H
#define MAME_VIDEO_HD44352_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> hd44352_device

class hd44352_device : public device_t
{
public:
	// construction/destruction
	hd44352_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto on_cb() { return m_on_cb.bind(); }

	// device interface
	uint8_t data_read();
	void data_write(uint8_t data);
	void control_write(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_validity_check(validity_checker &valid) const override;

	TIMER_CALLBACK_MEMBER(on_tick);

private:
	uint8_t compute_newval(uint8_t type, uint8_t oldval, uint8_t newval);
	uint8_t get_char(uint16_t pos);

	emu_timer *m_on_timer;

	uint8_t m_video_ram[2][0x180];
	uint8_t m_control_lines;
	uint8_t m_data_bus;
	uint8_t m_par[3];
	uint8_t m_state;
	uint16_t m_bank;
	uint16_t m_offset;
	uint8_t m_char_width;
	uint8_t m_lcd_on;
	uint8_t m_scroll;
	uint32_t m_contrast;

	uint8_t m_custom_char[4][8];      // 4 chars * 8 bytes
	uint8_t m_byte_count;
	uint8_t m_cursor_status;
	uint8_t m_cursor[8];
	uint8_t m_cursor_x;
	uint8_t m_cursor_y;
	uint8_t m_cursor_lcd;

	devcb_write_line    m_on_cb;        // ON line callback
	required_region_ptr<uint8_t> m_char_rom;
};

// device type definition
DECLARE_DEVICE_TYPE(HD44352, hd44352_device)

#endif // MAME_VIDEO_HD44352_H
