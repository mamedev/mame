// license:BSD-3-Clause
// copyright-holders:Carl,Miodrag Milanovic
#ifndef MAME_BUS_ISA_COM_H
#define MAME_BUS_ISA_COM_H

#pragma once

#include "isa.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_com_device

class isa8_com_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_com_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void pc_com_interrupt_1(int state) { m_isa->irq4_w(state); }
	void pc_com_interrupt_2(int state) { m_isa->irq3_w(state); }

protected:
	isa8_com_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_COM, isa8_com_device)

// ======================> isa8_com_at_device

class isa8_com_at_device :
		public isa8_com_device
{
public:
	// construction/destruction
	isa8_com_at_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_COM_AT, isa8_com_at_device)

#endif  // MAME_BUS_ISA_COM_H
