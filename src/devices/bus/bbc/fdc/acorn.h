// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn 8271 and 1770 FDC

**********************************************************************/


#ifndef MAME_BUS_BBC_FDC_ACORN_H
#define MAME_BUS_BBC_FDC_ACORN_H

#pragma once

#include "fdc.h"
#include "imagedev/floppy.h"
#include "machine/i8271.h"
#include "machine/wd_fdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_acorn8271_device :
	public device_t,
	public device_bbc_fdc_interface
{
public:
	// construction/destruction
	bbc_acorn8271_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void floppy_formats(format_registration &fr);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

private:
	void motor_w(int state);
	void side_w(int state);

	required_device<i8271_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
};

class bbc_acorn1770_device :
	public device_t,
	public device_bbc_fdc_interface
{
public:
	// construction/destruction
	bbc_acorn1770_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

private:
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);

	required_device<wd1770_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

	int m_fdc_ie;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_ACORN8271, bbc_acorn8271_device)
DECLARE_DEVICE_TYPE(BBC_ACORN1770, bbc_acorn1770_device)


#endif // MAME_BUS_BBC_FDC_ACORN_H
