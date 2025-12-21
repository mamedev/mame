// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_FUNTECH_UMC6650_H
#define MAME_FUNTECH_UMC6650_H

#pragma once

class umc6650_device : public device_t, public device_memory_interface
{
public:
	umc6650_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

private:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	required_memory_region m_romkey;
	address_space_config m_space_io_config;

	void internal_map(address_map &map) ATTR_COLD;
	address_space *m_space_io;
	u8 m_address;
};

DECLARE_DEVICE_TYPE(UMC6650, umc6650_device)

#endif // MAME_FUNTECH_UMC6650_H
