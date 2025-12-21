// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2scsi.h

    Implementation of the Apple II SCSI Card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2SCSI_H
#define MAME_BUS_A2BUS_A2SCSI_H

#pragma once

#include "a2bus.h"
#include "machine/ncr5380.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_scsi_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_scsi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void drq_w(int state);

protected:
	a2bus_scsi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, uint8_t data) override;

	required_device<ncr5380_device> m_ncr5380;
	required_device<nscsi_bus_device> m_scsibus;
	required_region_ptr<u8> m_rom;

private:
	uint8_t m_ram[8192];  // 8 banks of 1024 bytes
	int m_rambank, m_rombank;
	uint8_t m_drq;
	uint8_t m_bank;
	bool m_816block;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_SCSI, a2bus_scsi_device)

#endif // MAME_BUS_A2BUS_A2SCSI_H
