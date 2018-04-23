// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Generic Z80 daisy chain device

***************************************************************************/

#include "emu.h"
#include "z80daisy_generic.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(Z80DAISY_GENERIC, z80daisy_generic_device, "z80daisy_generic", "Generic Z80 daisy chain device")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z80daisy_generic_device - constructor
//-------------------------------------------------

z80daisy_generic_device::z80daisy_generic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, Z80DAISY_GENERIC, tag, owner, clock),
	device_z80daisy_interface(mconfig, *this),
	m_int_handler(*this),
	m_int(0), m_mask(0), m_vector(0xff)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z80daisy_generic_device::device_start()
{
	// resolve callbacks
	m_int_handler.resolve_safe();

	// register for save states
	save_item(NAME(m_int));
	save_item(NAME(m_mask));
	save_item(NAME(m_vector));
}

//-------------------------------------------------
//  z80daisy_irq_state - return the overall IRQ
//  state for this device
//-------------------------------------------------

int z80daisy_generic_device::z80daisy_irq_state()
{
	if (m_int & ~m_mask)
		return Z80_DAISY_INT;

	return 0;
}

//-------------------------------------------------
//  z80daisy_irq_ack - acknowledge an IRQ and
//  return the appropriate vector
//-------------------------------------------------

int z80daisy_generic_device::z80daisy_irq_ack()
{
	return m_vector;
}

//-------------------------------------------------
//  z80daisy_irq_reti - clear the interrupt
//  pending state to allow other interrupts through
//-------------------------------------------------

void z80daisy_generic_device::z80daisy_irq_reti()
{
}

//-------------------------------------------------
//  update_interrupt() - check interrupt status
//-------------------------------------------------

void z80daisy_generic_device::update_interrupt()
{
	m_int_handler(m_int & ~m_mask ? ASSERT_LINE : CLEAR_LINE);
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

WRITE_LINE_MEMBER( z80daisy_generic_device::int_w )
{
	m_int = state;
	update_interrupt();
}

WRITE_LINE_MEMBER( z80daisy_generic_device::mask_w )
{
	m_mask = state;
	update_interrupt();
}
