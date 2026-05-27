// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Barry Rodewald
/****************************************************
 *
 * Sharp X1 Keyboard device
 *
 * TODO:
 * - de-stateize X1 public variables
 *
 ***************************************************/

#include "emu.h"
#include "x1.h"


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(X1_KEYBOARD, x1_keyboard_device, "x1_keyboard", "Sharp X1 Keyboard")

//-------------------------------------------------
//  z80ctc_device - constructor
//-------------------------------------------------

x1_keyboard_device::x1_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, X1_KEYBOARD, tag, owner, clock)
	, device_z80daisy_interface(mconfig, *this)
	, m_flag_cb(*this, 0)
	, m_ack_cb(*this, 0x00)
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
	if(m_flag_cb() != 0)
		return Z80_DAISY_INT;
	return 0;
}


//-------------------------------------------------
//  z80daisy_irq_ack - acknowledge an IRQ and
//  return the appropriate vector
//-------------------------------------------------

int x1_keyboard_device::z80daisy_irq_ack()
{
	return m_ack_cb();
}

//-------------------------------------------------
//  z80daisy_irq_reti - clear the interrupt
//  pending state to allow other interrupts through
//-------------------------------------------------

void x1_keyboard_device::z80daisy_irq_reti()
{
}

uint8_t x1_state::key_irq_ack_r()
{
	if(!machine().side_effects_disabled())
	{
		m_key_irq_flag = 0;
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	}
	return m_key_irq_vector;
}
