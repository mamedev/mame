// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2hsscsi.h

    Implementation of the Apple II High Speed SCSI Card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2HSSCSI_H
#define MAME_BUS_A2BUS_A2HSSCSI_H

#pragma once

#include "a2bus.h"
#include "machine/ncr5380n.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_hsscsi_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_hsscsi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER( drq_w );

protected:
	a2bus_hsscsi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, uint8_t data) override;

	required_device<ncr5380n_device> m_ncr5380;
	required_device<nscsi_bus_device> m_scsibus;

private:
	uint8_t *m_rom;
	uint8_t m_ram[8192];  // 8 banks of 1024 bytes
	int m_rambank, m_rombank;
	uint8_t m_drq;
	uint8_t m_bank;
	bool m_816block;
	uint8_t m_c0ne, m_c0nf;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_HSSCSI, a2bus_hsscsi_device)

#endif // MAME_BUS_A2BUS_A2HSSCSI_H
