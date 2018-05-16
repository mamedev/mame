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
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_Z80DAISY_GENERIC_ADD(_tag, _vector) \
	MCFG_DEVICE_ADD(_tag, Z80DAISY_GENERIC, 0) \
	downcast<z80daisy_generic_device &>(*device).set_vector(_vector); \

#define MCFG_Z80DAISY_GENERIC_INT_CB(_devcb) \
	devcb = &downcast<z80daisy_generic_device &>(*device).set_int_handler(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class z80daisy_generic_device : public device_t, public device_z80daisy_interface
{
public:
	// construction/destruction
	z80daisy_generic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	template <class Object> devcb_base &set_int_handler(Object &&cb) { return m_int_handler.set_callback(std::forward<Object>(cb)); }

	// configuration
	void set_vector(uint8_t vector) { m_vector = vector; }

	DECLARE_WRITE_LINE_MEMBER(int_w);
	DECLARE_WRITE_LINE_MEMBER(mask_w);

protected:
	// device-level overrides
	virtual void device_start() override;

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
