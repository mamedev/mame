// license:BSD-3-Clause
// copyright-holders:O. Galibert

#ifndef MAME_VIDEO_ST7626_H
#define MAME_VIDEO_ST7626_H

#pragma once

class st7626_device : public device_t
{
public:
	st7626_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void map8(address_map &map) ATTR_COLD;

	void data8_w(u8 data);
	u8 data8_r();
	void cmd8_w(u8 data);
	u8 status8_r();

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

private:
	memory_share_creator<u16> m_vram;
};

DECLARE_DEVICE_TYPE(ST7626, st7626_device)

#endif // MAME_VIDEO_ST7626_H
