// license:BSD-3-Clause
// copyright-holders: Angelo Salese
#ifndef MAME_BUS_PC98_CBUS_LHA201_H
#define MAME_BUS_PC98_CBUS_LHA201_H

#pragma once

#include "slot.h"
#include "pc9801_55.h"

#include "bus/nscsi/devices.h"
#include "machine/nscsi_bus.h"
#include "machine/wd33c9x.h"

class lha201_device : public pc9801_55_device
{
public:
	lha201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	//lha201_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
//  virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
//  virtual space_config_vector memory_space_config() const override;
	virtual void remap(int space_id, offs_t start, offs_t end) override;

	virtual void internal_map(address_map &map) override ATTR_COLD;
	virtual void io_map(address_map &map) override ATTR_COLD;

private:
	required_ioport m_dsw3;

	u8 m_port34;
	u8 m_port36;
};

DECLARE_DEVICE_TYPE(LHA201, lha201_device)


#endif // MAME_BUS_PC98_CBUS_LHA201_H
