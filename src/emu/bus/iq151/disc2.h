// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __IQ151_DISC2_H__
#define __IQ151_DISC2_H__

#include "emu.h"
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
	iq151_disc2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry *device_rom_region() const;

	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// iq151cart_interface overrides
	virtual void read(offs_t offset, UINT8 &data);
	virtual void io_read(offs_t offset, UINT8 &data);
	virtual void io_write(offs_t offset, UINT8 data);

private:

	required_device<upd765a_device> m_fdc;
	UINT8 *     m_rom;
	bool        m_rom_enabled;
};


// device type definition
extern const device_type IQ151_DISC2;

#endif  /* __IQ151_DISC2_H__ */
