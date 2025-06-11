// license:BSD-3-Clause
// copyright-holders:R. Belmont, Patrick Mackinlay

#ifndef MAME_VIDEO_ARIEL_H
#define MAME_VIDEO_ARIEL_H

#pragma once

class ariel_device
	: public device_t
	, public device_palette_interface
{
public:
	ariel_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	u8 address_r();
	void address_w(u8 data);
	u8 palette_r();
	void palette_w(u8 data);
	u8 control_r();
	void control_w(u8 data);
	u8 key_color_r();
	void key_color_w(u8 data);

protected:
	// device_palette_interface implementation
	virtual void device_start() override ATTR_COLD;

	// device_palette_interface implementation
	virtual u32 palette_entries() const noexcept override { return 256; }

private:
	u8 m_address;
	u8 m_address_rgb;
	u8 m_control;
	u8 m_key_color;

	std::unique_ptr<std::array<u8, 3>[]> m_color_ram;
};

DECLARE_DEVICE_TYPE(ARIEL, ariel_device)

#endif // MAME_VIDEO_ARIEL_H
