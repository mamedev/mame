// license:BSD-3-Clause
// copyright-holders:O. Galibert

#include "emu.h"
#include "st7626.h"

DEFINE_DEVICE_TYPE(ST7626, st7626_device, "st7626", "ST7626 LCD Controller")

st7626_device::st7626_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ST7626, tag, owner, clock),
	m_vram(*this, "vram", 98*78, ENDIANNESS_LITTLE)
{
}

void st7626_device::device_start()
{
}

void st7626_device::device_reset()
{
}

void st7626_device::map8(address_map &map)
{
	map(0, 0).rw(FUNC(st7626_device::status8_r), FUNC(st7626_device::cmd8_w));
	map(1, 1).rw(FUNC(st7626_device::data8_r), FUNC(st7626_device::data8_w));
}

void st7626_device::data8_w(u8 data)
{
	logerror("data_w %02x %s\n", data, machine().describe_context());
}

u8 st7626_device::data8_r()
{
	logerror("data_r %s\n", machine().describe_context());
	return 0;
}

void st7626_device::cmd8_w(u8 data)
{
	logerror("cmd_w %02x %s\n", data, machine().describe_context());
}

u8 st7626_device::status8_r()
{
	logerror("status_r %s\n", machine().describe_context());
	return 0;
}

u32 st7626_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x0000ff);
	return 0;
}
