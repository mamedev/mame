// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#ifndef MAME_BUS_TVC_HBF_H
#define MAME_BUS_TVC_HBF_H

#pragma once

#include "tvc.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tvc_hbf_device

class tvc_hbf_device :
		public device_t,
		public device_tvcexp_interface
{
public:
	// construction/destruction
	tvc_hbf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// tvcexp_interface overrides
	virtual uint8_t id_r() override { return 0x02; } // ID_A to GND, ID_B to VCC
	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
	virtual uint8_t io_read(offs_t offset) override;
	virtual void io_write(offs_t offset, uint8_t data) override;

private:
	static void floppy_formats(format_registration &fr);

	// internal state
	required_device<fd1793_device>   m_fdc;

	uint8_t *     m_rom;
	uint8_t *     m_ram;
	uint8_t       m_rom_bank;     // A12 and A13
};


// device type definition
DECLARE_DEVICE_TYPE(TVC_HBF, tvc_hbf_device)

#endif // MAME_BUS_TVC_HBF_H
