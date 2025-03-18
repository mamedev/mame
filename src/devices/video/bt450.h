// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_VIDEO_BT450_H
#define MAME_VIDEO_BT450_H

#pragma once

class bt450_device
	: public device_t
	, public device_palette_interface
{
public:
	bt450_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	u8 address_r();
	void address_w(u8 data);
	u8 palette_r(address_space &space);
	void palette_w(u8 data);

protected:
	// device_palette_interface implementation
	virtual void device_start() override ATTR_COLD;

	// device_palette_interface implementation
	virtual u32 palette_entries() const noexcept override { return 16 + 3; }

private:
	u8 m_address;
	u8 m_address_rgb;

	std::unique_ptr<std::array<u8, 3>[]> m_color_ram;
};

DECLARE_DEVICE_TYPE(BT450, bt450_device)

#endif // MAME_VIDEO_BT450_H
