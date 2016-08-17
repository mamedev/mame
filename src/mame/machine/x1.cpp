// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Barry Rodewald

#include "includes/x1.h"


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

const device_type X1_KEYBOARD = &device_creator<x1_keyboard_device>;

//-------------------------------------------------
//  z80ctc_device - constructor
//-------------------------------------------------

x1_keyboard_device::x1_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, X1_KEYBOARD, "X1 Keyboard", tag, owner, clock, "x1_keyboard", __FILE__),
		device_z80daisy_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void x1_keyboard_device::device_start()
{
}

//**************************************************************************
//  DAISY CHAIN INTERFACE
//**************************************************************************

//-------------------------------------------------
//  z80daisy_irq_state - return the overall IRQ
//  state for this device
//-------------------------------------------------

int x1_keyboard_device::z80daisy_irq_state()
{
	x1_state *state = machine().driver_data<x1_state>();
	if(state->m_key_irq_flag != 0)
		return Z80_DAISY_INT;
	return 0;
}


//-------------------------------------------------
//  z80daisy_irq_ack - acknowledge an IRQ and
//  return the appropriate vector
//-------------------------------------------------

int x1_keyboard_device::z80daisy_irq_ack()
{
	x1_state *state = machine().driver_data<x1_state>();
	state->m_key_irq_flag = 0;
	state->m_maincpu->set_input_line(INPUT_LINE_IRQ0,CLEAR_LINE);
	return state->m_key_irq_vector;
}

//-------------------------------------------------
//  z80daisy_irq_reti - clear the interrupt
//  pending state to allow other interrupts through
//-------------------------------------------------

void x1_keyboard_device::z80daisy_irq_reti()
{
}
