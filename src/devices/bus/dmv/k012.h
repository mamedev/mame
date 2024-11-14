// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
// thanks-to:rfka01
#ifndef MAME_BUS_DMV_K012_H
#define MAME_BUS_DMV_K012_H

#pragma once

#include "dmvbus.h"
#include "machine/wd1000.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dmv_k012_device

class dmv_k012_device :
		public device_t,
		public device_dmvslot_interface
{
public:
	// construction/destruction
	dmv_k012_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	dmv_k012_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// dmvcart_interface overrides
	virtual void io_read(int ifsel, offs_t offset, uint8_t &data) override;
	virtual void io_write(int ifsel, offs_t offset, uint8_t data) override;

	void hdc_intrq_w(int state);

private:
	required_device<wd1000_device> m_hdc;
	required_ioport m_jumpers;
};


// ======================> dmv_c3282_device

class dmv_c3282_device :
		public dmv_k012_device
{
public:
	// construction/destruction
	dmv_c3282_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(DMV_K012, dmv_k012_device)
DECLARE_DEVICE_TYPE(DMV_C3282, dmv_c3282_device)

#endif  // MAME_BUS_DMV_K012_H
