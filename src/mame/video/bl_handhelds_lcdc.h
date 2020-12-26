// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_BL_HANDHELDS_LCDC_H
#define MAME_MACHINE_BL_HANDHELDS_LCDC_H

#pragma once

DECLARE_DEVICE_TYPE(BL_HANDHELDS_LCDC, bl_handhelds_lcdc_device)

class bl_handhelds_lcdc_device : public device_t
{
public:
	// construction/destruction
	bl_handhelds_lcdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void lcdc_command_w(u8 data);
	u8 lcdc_data_r();
	void lcdc_data_w(u8 data);

	u32 render_to_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	u8 m_displaybuffer[256 * 256 * 2];
	u16 m_posx, m_posy;
	u16 m_posminx, m_posmaxx;
	u16 m_posminy, m_posmaxy;
	u8 m_command;
	u8 m_commandstep;

};

#endif // MAME_MACHINE_BL_HANDHELDS_LCDC_H
