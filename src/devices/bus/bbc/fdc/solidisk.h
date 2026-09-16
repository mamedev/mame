// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Solidisk 8271/1770 FDC

**********************************************************************/

#ifndef MAME_BUS_BBC_FDC_SOLIDISK_H
#define MAME_BUS_BBC_FDC_SOLIDISK_H

#pragma once

#include "fdc.h"
#include "machine/i8271.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_stlfdc_device :
	public device_t,
	public device_bbc_fdc_interface
{
public:
	static void floppy_formats(format_registration &fr);
	DECLARE_INPUT_CHANGED_MEMBER(fdc_changed);

protected:
	// construction/destruction
	bbc_stlfdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void motor_w(int state);
	void side_w(int state);

	required_device<wd1770_device> m_wd1770;
	optional_device<i8271_device> m_i8271;
	required_device_array<floppy_connector, 2> m_floppy;
	optional_ioport m_dfdc;
};


class bbc_stl1770_1_device : public bbc_stlfdc_device
{
public:
	bbc_stl1770_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	bbc_stl1770_1_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config& config) override;
	virtual const tiny_rom_entry* device_rom_region() const override;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
};


class bbc_stl1770_2_device : public bbc_stlfdc_device
{
public:
	bbc_stl1770_2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	bbc_stl1770_2_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config& config) override;
	virtual const tiny_rom_entry* device_rom_region() const override;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
};


class bbc_stldfdc_1_device : public bbc_stl1770_1_device
{
public:
	// construction/destruction
	bbc_stldfdc_1_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config& config) override;
	virtual const tiny_rom_entry* device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_STL1770_1, bbc_stl1770_1_device)
DECLARE_DEVICE_TYPE(BBC_STL1770_2, bbc_stl1770_2_device)
DECLARE_DEVICE_TYPE(BBC_STLDFDC_1, bbc_stldfdc_1_device)


#endif /* MAME_BUS_BBC_FDC_SOLIDISK_H */
