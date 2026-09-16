// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    Multi Unique FDC

    8-bit floppy controller, supports 4 drives with 360k, 720k,
    1.2MB or 1.44MB. It was sold under a few different names:

    - Ably-Tech FDC-344
    - Magitronic Multi Floppy Controller Card
    - Micro-Q (same as FDC-344?)
    - Modular Circuit Technology MCT-FDC-HD4 (not dumped)

***************************************************************************/

#ifndef MAME_BUS_ISA_MUFDC_H
#define MAME_BUS_ISA_MUFDC_H

#pragma once

#include "isa.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mufdc_device

class mufdc_device : public device_t, public device_isa8_card_interface
{
protected:
	// construction/destruction
	mufdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_isa8_card_interface
	virtual uint8_t dack_r(int line) override;
	virtual void dack_w(int line, uint8_t data) override;
	virtual void dack_line_w(int line, int state) override;
	virtual void eop_w(int state) override;

private:
	static void floppy_formats(format_registration &fr);

	uint8_t fdc_input_r();
	void fdc_irq_w(int state);
	void fdc_drq_w(int state);

	required_device<mcs3201_device> m_fdc;
	required_ioport m_config;
};

class fdc344_device : public mufdc_device
{
public:
	// construction/destruction
	fdc344_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class fdcmag_device : public mufdc_device
{
public:
	// construction/destruction
	fdcmag_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA8_FDC344, fdc344_device)
DECLARE_DEVICE_TYPE(ISA8_FDCMAG, fdcmag_device)

#endif // MAME_BUS_ISA_MUFDC_H
