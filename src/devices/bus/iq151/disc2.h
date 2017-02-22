// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __IQ151_DISC2_H__
#define __IQ151_DISC2_H__

#include "iq151.h"
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

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// iq151cart_interface overrides
	virtual void read(offs_t offset, uint8_t &data) override;
	virtual void io_read(offs_t offset, uint8_t &data) override;
	virtual void io_write(offs_t offset, uint8_t data) override;

private:

	required_device<upd765a_device> m_fdc;
	uint8_t *     m_rom;
	bool        m_rom_enabled;
};


// device type definition
extern const device_type IQ151_DISC2;

#endif  /* __IQ151_DISC2_H__ */
