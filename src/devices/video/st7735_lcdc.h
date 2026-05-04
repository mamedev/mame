// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_VIDEO_ST7735_LCDC_H
#define MAME_VIDEO_ST7735_LCDC_H

#pragma once

DECLARE_DEVICE_TYPE(ST7735, st7735_lcdc_device)

class st7735_lcdc_device : public device_t
{
public:
	// construction/destruction
	st7735_lcdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void lcdc_command_w(u8 data);
	u8 lcdc_data_r();
	void lcdc_data_w(u8 data);

	u32 render_to_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 m_displaybuffer[256 * 256 * 2];
	u16 m_posx, m_posy;
	u16 m_posminx, m_posmaxx;
	u16 m_posminy, m_posmaxy;
	u8 m_command;
	u8 m_commandstep;
	u8 m_displayon;
	u8 m_sleep;
};

#endif // MAME_VIDEO_ST7735_LCDC_H
