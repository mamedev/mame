// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2diskii.h

    Apple II Disk II Controller Card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2DISKII_H
#define MAME_BUS_A2BUS_A2DISKII_H

#pragma once

#include "a2bus.h"
#include "machine/applefdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_floppy_device:
	public device_t,
	public device_a2bus_card_interface
{
protected:
	// construction/destruction
	a2bus_floppy_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;

	required_device<applefdc_base_device> m_fdc;

private:
	uint8_t *m_rom;
};

class a2bus_diskii_device: public a2bus_floppy_device
{
public:
	a2bus_diskii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class a2bus_iwmflop_device: public a2bus_floppy_device
{
public:
	a2bus_iwmflop_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};

class a2bus_agat7flop_device: public a2bus_floppy_device
{
public:
	a2bus_agat7flop_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override;
};

class a2bus_agat9flop_device: public a2bus_floppy_device
{
public:
	a2bus_agat9flop_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_DISKII,    a2bus_diskii_device)
DECLARE_DEVICE_TYPE(A2BUS_IWM_FDC,   a2bus_iwmflop_device)
DECLARE_DEVICE_TYPE(A2BUS_AGAT7_FDC, a2bus_agat7flop_device)
DECLARE_DEVICE_TYPE(A2BUS_AGAT9_FDC, a2bus_agat9flop_device)

#endif  // MAME_BUS_A2BUS_A2DISKII_H
