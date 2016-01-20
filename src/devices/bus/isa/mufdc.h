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

#pragma once

#ifndef __ISA_MUFDC_H__
#define __ISA_MUFDC_H__

#include "emu.h"
#include "isa.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mufdc_device

class mufdc_device : public device_t,
						public device_isa8_card_interface
{
public:
	// construction/destruction
	mufdc_device(const machine_config &mconfig, device_type type, std::string tag, device_t *owner, UINT32 clock, std::string name, std::string shortname);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	DECLARE_READ8_MEMBER( fdc_input_r );
	DECLARE_WRITE_LINE_MEMBER( fdc_irq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_isa8_card_interface
	virtual UINT8 dack_r(int line) override;
	virtual void dack_w(int line, UINT8 data) override;
	virtual void eop_w(int state) override;

private:
	required_device<pc_fdc_interface> m_fdc;
	required_ioport m_config;
};

class fdc344_device : public mufdc_device
{
public:
	// construction/destruction
	fdc344_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual const rom_entry *device_rom_region() const override;

protected:
	virtual void device_config_complete() override { m_shortname = "fdc344"; }
};

class fdcmag_device : public mufdc_device
{
public:
	// construction/destruction
	fdcmag_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual const rom_entry *device_rom_region() const override;

protected:
	virtual void device_config_complete() override { m_shortname = "fdcmag"; }
};

// device type definition
extern const device_type ISA8_FDC344;
extern const device_type ISA8_FDCMAG;

#endif
