// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Cumana QFS 8877A FDC

**********************************************************************/

#ifndef MAME_BUS_BBC_FDC_CUMANA_H
#define MAME_BUS_BBC_FDC_CUMANA_H

#pragma once

#include "fdc.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_cumanafdc_device :
	public device_t,
	public device_bbc_fdc_interface
{
public:
	static void floppy_formats(format_registration &fr);

	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);
	void motor_w(int state);

protected:
	// construction/destruction
	bbc_cumanafdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

	required_device<mb8877_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

	bool m_invert;

private:
	int m_drive_control;
	int m_fdc_ie;
};

class bbc_cumana1_device : public bbc_cumanafdc_device
{
public:
	bbc_cumana1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class bbc_cumana2_device : public bbc_cumanafdc_device
{
public:
	bbc_cumana2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_CUMANA1, bbc_cumana1_device)
DECLARE_DEVICE_TYPE(BBC_CUMANA2, bbc_cumana2_device)


#endif // MAME_BUS_BBC_FDC_CUMANA_H
