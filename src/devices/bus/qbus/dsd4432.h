// copyright-holders:AJR
/***************************************************************************

    Data Systems Design A4432 Floppy Disk Interface

***************************************************************************/

#ifndef MAME_BUS_QBUS_DSD4432_H
#define MAME_BUS_QBUS_DSD4432_H

#pragma once

#include "qbus.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dsd4432_device

class dsd4432_device : public device_t, public device_qbus_card_interface
{
public:
	// device type constructor
	dsd4432_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	required_region_ptr<u16> m_bootstrap;
	required_ioport m_bootaddr;
	required_ioport m_iv;
	required_ioport m_mode;
};

// device type declaration
DECLARE_DEVICE_TYPE(DSD4432, dsd4432_device)

#endif // MAME_BUS_QBUS_DSD4432_H
