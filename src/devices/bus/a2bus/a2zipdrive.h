// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2zipdrive.h

    ZIP Technologies ZipDrive IDE card
    Parsons Engineering Focus Drive IDE card

    See important NOTE at the top of a2zipdrive.cpp!

*********************************************************************/

#ifndef MAME_BUS_A2BUS_ZIPDRIVE_H
#define MAME_BUS_A2BUS_ZIPDRIVE_H

#pragma once

#include "a2bus.h"
#include "bus/ata/ataintf.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_zipdrivebase_device:
	public device_t,
	public device_a2bus_card_interface
{
protected:
	// construction/destruction
	a2bus_zipdrivebase_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual uint8_t read_c800(uint16_t offset) override;

	required_device<ata_interface_device> m_ata;

	uint8_t *m_rom;
	uint16_t m_lastdata;
};

class a2bus_zipdrive_device : public a2bus_zipdrivebase_device
{
public:
	a2bus_zipdrive_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class a2bus_focusdrive_device : public a2bus_zipdrivebase_device
{
public:
	a2bus_focusdrive_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_ZIPDRIVE, a2bus_zipdrive_device)
DECLARE_DEVICE_TYPE(A2BUS_FOCUSDRIVE, a2bus_focusdrive_device)

#endif // MAME_BUS_A2BUS_ZIPDRIVE_H
