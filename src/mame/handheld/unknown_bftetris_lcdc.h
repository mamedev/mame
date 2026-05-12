// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_HANDHELD_UNKNOWN_BFTETRIS_LCDC_H
#define MAME_HANDHELD_UNKNOWN_BFTETRIS_LCDC_H

#pragma once

DECLARE_DEVICE_TYPE(UNKNOWN_BFTETRIS_LCDC, bftetris_lcdc_device)

class bftetris_lcdc_device : public device_t
{
public:
	// construction/destruction
	bftetris_lcdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void lcdc_command_w(u16 data);
	u16 lcdc_data_r();
	void lcdc_data_w(u16 data);

	u32 render_to_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u16 m_displaybuffer[512 * 256];
	u16 m_posx, m_posy;
	u16 m_posminx, m_posmaxx;
	u16 m_posminy, m_posmaxy;
	u8 m_command;
	u8 m_commandstep;
	u8 m_displayon;
	u8 m_sleep;
};

#endif // MAME_HANDHELD_UNKNOWN_BFTETRIS_LCDC_H
