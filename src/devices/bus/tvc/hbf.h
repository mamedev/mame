// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#ifndef MAME_BUS_TVC_HBF_H
#define MAME_BUS_TVC_HBF_H

#pragma once

#include "tvc.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "formats/tvc_dsk.h"


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
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// tvcexp_interface overrides
	virtual uint8_t id_r() override { return 0x02; } // ID_A to GND, ID_B to VCC
	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;
	virtual DECLARE_READ8_MEMBER(io_read) override;
	virtual DECLARE_WRITE8_MEMBER(io_write) override;

private:
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	// internal state
	required_device<fd1793_device>   m_fdc;

	uint8_t *     m_rom;
	uint8_t *     m_ram;
	uint8_t       m_rom_bank;     // A12 and A13
};


// device type definition
DECLARE_DEVICE_TYPE(TVC_HBF, tvc_hbf_device)

#endif // MAME_BUS_TVC_HBF_H
