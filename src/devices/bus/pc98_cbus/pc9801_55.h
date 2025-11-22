// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    NEC PC-9801-55/-55U/-55L

***************************************************************************/

#ifndef MAME_BUS_PC98_CBUS_PC9801_55_H
#define MAME_BUS_PC98_CBUS_PC9801_55_H

#pragma once

#include "slot.h"
#include "bus/nscsi/devices.h"
#include "machine/nscsi_bus.h"
#include "machine/wd33c9x.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc9801_118_device

class pc9801_55_device : public device_t
					   , public device_memory_interface
					   , public device_pc98_cbus_slot_interface
{
public:
	// construction/destruction
	//pc9801_55_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	pc9801_55_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::DISK | feature::PROTECTION; }

	void scsi_irq_w(int state);
	void scsi_drq_w(int state);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

//  virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;
	virtual void remap(int space_id, offs_t start, offs_t end) override;
	virtual u8 dack_r(int line) override;
	virtual void dack_w(int line, u8 data) override;

private:
	required_device<nscsi_bus_device> m_scsi_bus;
	required_device<wd33c9x_base_device> m_wdc;
	address_space_config m_space_io_config;
	required_memory_region m_bios;
	required_ioport m_dsw1;
	required_ioport m_dsw2;

	void io_map(address_map &map) ATTR_COLD;
	void internal_map(address_map &map) ATTR_COLD;
	void increment_addr();

	u8 m_ar;
	u8 m_port30;
	u8 m_pkg_id;
	u8 m_rom_bank;
	bool m_dma_enable;
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

#endif // MAME_BUS_PC98_CBUS_PC9801_55_H
