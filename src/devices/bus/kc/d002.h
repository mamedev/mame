// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#ifndef MAME_BUS_KC_D002_H
#define MAME_BUS_KC_D002_H

#pragma once

#include "kc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> kc_d002_device

class kc_d002_device :
		public device_t,
		public device_kcexp_interface
{
public:
	// construction/destruction
	kc_d002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// kcexp_interface overrides
	virtual void read(offs_t offset, uint8_t &data) override;
	virtual void write(offs_t offset, uint8_t data) override;
	virtual void io_read(offs_t offset, uint8_t &data) override;
	virtual void io_write(offs_t offset, uint8_t data) override;
	virtual void mei_w(int state) override;

private:
	// interface callbacks
	void out_irq_w(int state);
	void out_nmi_w(int state);
	void out_halt_w(int state);

	kcexp_slot_device *m_slot;

	// internal state
	required_device_array<kcexp_slot_device, 5> m_expansions;
};


// device type definition
DECLARE_DEVICE_TYPE(KC_D002, kc_d002_device)

#endif // MAME_BUS_KC_D002_H
