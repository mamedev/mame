// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam floppy disk controller emulation

**********************************************************************/

#ifndef MAME_BUS_ADAMNET_FDC_H
#define MAME_BUS_ADAMNET_FDC_H

#pragma once

#include "adamnet.h"
#include "cpu/m6800/m6801.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adam_fdc_device

class adam_fdc_device :  public device_t,
							public device_adamnet_card_interface
{
public:
	// construction/destruction
	adam_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	adam_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_adamnet_card_interface overrides
	virtual void adamnet_reset_w(int state) override;

	required_device<m6801_cpu_device> m_maincpu;
	required_device<wd2793_device> m_fdc;
	required_device<floppy_connector> m_connector;
	floppy_image_device *m_floppy;
	required_shared_ptr<uint8_t> m_ram;
	required_ioport m_sw3;

	static void floppy_formats(format_registration &fr);

	uint8_t read_data_r(offs_t offset);
	uint8_t write_data_r(offs_t offset);
	virtual uint8_t p1_r();
	virtual void p1_w(uint8_t data);
	uint8_t p2_r();
	void p2_w(uint8_t data);

private:
	void adam_fdc_mem(address_map &map) ATTR_COLD;
};


// ======================> adam_fdc_320kb_device

class adam_fdc_320kb_device : public adam_fdc_device
{
public:
	// construction/destruction
	adam_fdc_320kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	adam_fdc_320kb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void p1_w(uint8_t data) override;
};


// ======================> adam_fdc_a720dipi_device

class adam_fdc_a720dipi_device : public adam_fdc_320kb_device
{
public:
	// construction/destruction
	adam_fdc_a720dipi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t p1_r() override;
};


// ======================> adam_fdc_fp720at_device

class adam_fdc_fp720at_device : public adam_fdc_320kb_device
{
public:
	// construction/destruction
	adam_fdc_fp720at_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// ======================> adam_fdc_mihddd_device

class adam_fdc_mihddd_device : public adam_fdc_320kb_device
{
public:
	// construction/destruction
	adam_fdc_mihddd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void p1_w(uint8_t data) override;
};


// device type declarations
DECLARE_DEVICE_TYPE(ADAM_FDC, adam_fdc_device)
DECLARE_DEVICE_TYPE(ADAM_FDC_320KB, adam_fdc_320kb_device)
DECLARE_DEVICE_TYPE(ADAM_FDC_A720DIPI, adam_fdc_a720dipi_device)
DECLARE_DEVICE_TYPE(ADAM_FDC_FP720AT, adam_fdc_fp720at_device)
DECLARE_DEVICE_TYPE(ADAM_FDC_MIHDDD, adam_fdc_mihddd_device)


#endif // MAME_BUS_ADAMNET_FDC_H
