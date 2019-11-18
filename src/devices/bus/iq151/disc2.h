// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#ifndef MAME_BUS_IQ151_DISC2_H
#define MAME_BUS_IQ151_DISC2_H

#pragma once

#include "iq151.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> iq151_disc2_device

class iq151_disc2_device :
		public device_t,
		public device_iq151cart_interface
{
public:
	// construction/destruction
	iq151_disc2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// iq151cart_interface overrides
	virtual void read(offs_t offset, uint8_t &data) override;
	virtual void io_read(offs_t offset, uint8_t &data) override;
	virtual void io_write(offs_t offset, uint8_t data) override;

private:
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	required_device<upd765a_device> m_fdc;
	uint8_t *     m_rom;
	bool        m_rom_enabled;
};


// device type definition
DECLARE_DEVICE_TYPE(IQ151_DISC2, iq151_disc2_device)

#endif // MAME_BUS_IQ151_DISC2_H
