// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC98_CBUS_SOUND_H
#define MAME_BUS_PC98_CBUS_SOUND_H

#pragma once

#include "pc9801_86.h"
#include "pc9801_118.h"
#include "slot.h"

#include "machine/eepromser.h"

class sound_pc9821ce_device : public pc9801_86_device
							, public device_memory_interface
{
public:
	// construction/destruction
	sound_pc9821ce_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

private:
	address_space_config m_space_io_config;
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	void pnp_map(address_map &map);
	void pnp_io_map(address_map &map);

	u16 config_r(offs_t offset);
	void config_w(offs_t offset, u16 data, u16 mem_mask);

	u8 m_index;
};

class sound_pc9821cx3_device : public pc9801_118_device
						     , public device_memory_interface
{
public:
	// construction/destruction
	sound_pc9821cx3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

private:
	address_space_config m_space_io_config;

	void pnp_io_map(address_map &map);

	u8 control_r(offs_t offset);
	void control_w(offs_t offset, u8 data);

	u8 m_index;
};


DECLARE_DEVICE_TYPE(SOUND_PC9821CE,  sound_pc9821ce_device)
DECLARE_DEVICE_TYPE(SOUND_PC9821CX3, sound_pc9821cx3_device)


#endif // MAME_BUS_PC98_CBUS_SOUND_H
