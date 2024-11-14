// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    NEC PC-9801-55/-55U/-55L

***************************************************************************/

#ifndef MAME_BUS_CBUS_PC9801_55_H
#define MAME_BUS_CBUS_PC9801_55_H

#pragma once

#include "bus/cbus/pc9801_cbus.h"
#include "bus/nscsi/devices.h"
#include "machine/nscsi_bus.h"
#include "machine/wd33c9x.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc9801_118_device

class pc9801_55_device : public device_t
{
public:
	// construction/destruction
	//pc9801_55_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	pc9801_55_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

	void scsi_irq_w(int state);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	// optional information overrides
//  virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<pc9801_slot_device> m_bus;
	required_device<nscsi_bus_device> m_scsi_bus;
	required_device<wd33c9x_base_device> m_wdc;

	u8 comms_r(offs_t offset);
	void comms_w(offs_t offset, u8 data);
};

class pc9801_55u_device : public pc9801_55_device
{
public:
	pc9801_55u_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

};

class pc9801_55l_device : public pc9801_55_device
{
public:
	pc9801_55l_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

};

// device type definition
//DECLARE_DEVICE_TYPE(PC9801_55, pc9801_55_device)
DECLARE_DEVICE_TYPE(PC9801_55U, pc9801_55u_device)
DECLARE_DEVICE_TYPE(PC9801_55L, pc9801_55l_device)

#endif // MAME_BUS_CBUS_PC9801_55_H
