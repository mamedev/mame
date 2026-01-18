// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_MISC_3DO_AMY_H
#define MAME_MISC_3DO_AMY_H

#pragma once

#include "screen.h"

class amy_device : public device_t
                 , public device_video_interface
                 //, public device_memory_interface
{
public:
	// construction/destruction
	amy_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void pixel_xfer(int x, int y, u16 dot);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
//	virtual space_config_vector memory_space_config() const override;

private:
//	void clut_map(address_map &map) ATTR_COLD;
//	const address_space_config m_space_clut_config;
	bitmap_rgb32 m_bitmap;

};

DECLARE_DEVICE_TYPE(AMY, amy_device)


#endif // MAME_MISC_3DO_AMY_H
