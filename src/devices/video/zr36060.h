// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_VIDEO_ZR36060_H
#define MAME_VIDEO_ZR36060_H

#pragma once

class zr36060_device : public device_t, public device_memory_interface
{
public:
	zr36060_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;
private:
	void regs_map(address_map &map);

	u16 m_address;
	address_space_config m_space_config;
};

DECLARE_DEVICE_TYPE(ZR36060, zr36060_device)

#endif // MAME_VIDEO_ZR36060_H
