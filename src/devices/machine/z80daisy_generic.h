// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Generic Z80 daisy chain device

***************************************************************************/

#ifndef MAME_MACHINE_Z80DAISY_GENERIC_H
#define MAME_MACHINE_Z80DAISY_GENERIC_H

#pragma once

#include "z80daisy.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class z80daisy_generic_device : public device_t, public device_z80daisy_interface
{
public:
	// construction/destruction
	z80daisy_generic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto int_handler() { return m_int_handler.bind(); }

	// configuration
	void set_vector(uint8_t vector) { m_vector = vector; }

	void int_w(int state);
	void mask_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

private:
	void update_interrupt();

	devcb_write_line m_int_handler;

	int m_int;
	int m_mask;
	int m_vector;
};

// device type definition
DECLARE_DEVICE_TYPE(Z80DAISY_GENERIC, z80daisy_generic_device)

#endif // MAME_MACHINE_Z80DAISY_GENERIC_H
