// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "3do_amy.h"

// a.k.a. Brooktree Bt9103
DEFINE_DEVICE_TYPE(AMY, amy_device, "amy", "3DO DA9103KPJ-XN \"Amy\" Digital Color Encoder")

amy_device::amy_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, AMY, tag, owner, clock)
	, device_video_interface(mconfig, *this)
//	, device_memory_interface(mconfig, *this)
{
}

void amy_device::device_start()
{
	screen().register_screen_bitmap(m_bitmap);
}

void amy_device::device_reset()
{
}

void amy_device::pixel_xfer(int x, int y, u16 dot)
{
	// ...
}

u32 amy_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copyrozbitmap(bitmap, cliprect, m_bitmap, 0, 0, 0x4000, 0, 0, 0x10000, false);
	return 0;
}

