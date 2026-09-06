// license:BSD-3-Clause
// copyright-holders:grubbyplaya
/***************************************************************************

        Toshiba T6B79 LCD controller

***************************************************************************/

#ifndef MAME_VIDEO_T6B79_H
#define MAME_VIDEO_T6B79_H

#pragma once

class t6b79_device : public device_t
{
public:
	t6b79_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void control_write(uint8_t data);
	uint8_t control_read();
	void data_write(uint8_t data);
	uint8_t data_read();
	void stb_write(uint8_t data);
	uint8_t stb_read();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_lcd_ram[64 * 48];

	uint8_t m_busy_flag;
	uint8_t m_display_on;
	uint8_t m_contrast;
	uint8_t m_xpos;
	uint8_t m_ypos;
	uint8_t m_zpos;
	int8_t  m_direction;
	uint8_t m_active_counter;
	uint8_t m_word_len;
	uint8_t m_opa1;
	uint8_t m_opa2;
	uint8_t m_output_reg;
	uint8_t m_stb;

	uint8_t m_height;
	uint8_t m_width;

	void advance_y();
};

DECLARE_DEVICE_TYPE(T6B79, t6b79_device)

#endif // MAME_VIDEO_T6B79_H