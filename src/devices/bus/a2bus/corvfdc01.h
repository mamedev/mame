// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    corvfdc01.h

    Implemention of the Corvus Systems CORVUS01 floppy controller

*********************************************************************/

#ifndef MAME_BUS_A2BUS_CORVFDC01_H
#define MAME_BUS_A2BUS_CORVFDC01_H

#pragma once

#include "a2bus.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_corvfdc01_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_corvfdc01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_corvfdc01_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;

	required_device<fd1793_device> m_wdfdc;
	required_device<floppy_connector> m_con1;
	required_device<floppy_connector> m_con2;
	required_device<floppy_connector> m_con3;
	required_device<floppy_connector> m_con4;

private:
	void intrq_w(int state);
	void drq_w(int state);

	uint8_t *m_rom;
	uint8_t m_fdc_local_status, m_fdc_local_command;
	floppy_image_device *m_curfloppy;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_CORVFDC01, a2bus_corvfdc01_device)

#endif // MAME_BUS_A2BUS_CORVFDC01_H
