// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_HANDHELD_CLASS200_LCDC_H
#define MAME_HANDHELD_CLASS200_LCDC_H

#pragma once

#include <memory>


class class200_lcdc_device : public device_t
{
public:
	// construction/destruction
	class200_lcdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void lcdc_command_w(u8 data);
	void lcdc_data_w(u8 data);
	u8 lcdc_data_r();

	u32 render_to_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void class200_lcdc_advance();

	std::unique_ptr<u16[]> m_gram;
	u16 m_lcd_regs[256];
	u16 m_lcd_command_word;
	u16 m_lcd_data_word;
	u8 m_lcd_command_phase;
	u8 m_lcd_data_phase;
	u8 m_lcd_read_phase;
	u8 m_lcd_reg;
	u32 m_lcd_gram_writes;
};

DECLARE_DEVICE_TYPE(CLASS200_LCDC, class200_lcdc_device)

#endif // MAME_HANDHELD_CLASS200_LCDC_H
