// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_MISC_3DO_AMY_H
#define MAME_MISC_3DO_AMY_H

#pragma once

#include "screen.h"

class amy_device : public device_t
                 , public device_video_interface
{
public:
	amy_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void pixel_xfer(int x, int y, u16 dot);
	void blank_line(int y);
	void dac_enable(bool enabled);
	void clut_write(u32 data);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	bitmap_rgb32 m_bitmap;

	bool m_is_dac_enabled;
	struct {
		u8 r, g, b;
	} m_custom_clut[32];
};

DECLARE_DEVICE_TYPE(AMY, amy_device)


#endif // MAME_MISC_3DO_AMY_H
