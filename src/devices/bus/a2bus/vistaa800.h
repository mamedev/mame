// license:BSD-3-Clause
// copyright-holders:R. Justice
/*********************************************************************

    vistaa800.h

    Vista A800 8" disk Controller Card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_VISTAA800_H
#define MAME_BUS_A2BUS_VISTAA800_H

#pragma once

#include "a2bus.h"

#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


class a2bus_vistaa800_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_vistaa800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_vistaa800_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual uint8_t read_c800(uint16_t offset) override;

	required_device<fd1797_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_region_ptr<u8> m_rom;

private:
	static void floppy_formats(format_registration &fr);

	void fdc_intrq_w(uint8_t state);
	void fdc_drq_w(uint8_t state);
	void fdc_sso_w(uint8_t state);
	
	uint16_t m_dmaaddr;
	bool m_dmaenable_read;
	bool m_dmaenable_write;
	uint8_t m_density;
	uint8_t m_side;
	uint8_t m_sso;

};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_VISTAA800, a2bus_vistaa800_device)

#endif  // MAME_BUS_A2BUS_VISTAA800_H
