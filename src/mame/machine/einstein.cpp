// license:GPL-2.0+
// copyright-holders:Kevin Thacker, Dirk Best, Phill Harvey-Smith
/***************************************************************************

    Tatung Einstein


***************************************************************************/

#include "emu.h"
#include "includes/einstein.h"

/****************************************************************
    EINSTEIN NON-Z80 DEVICES DAISY CHAIN SUPPORT
****************************************************************/


const device_type EINSTEIN_KEYBOARD_DAISY = &device_creator<einstein_keyboard_daisy_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z80ctc_device - constructor
//-------------------------------------------------

einstein_keyboard_daisy_device::einstein_keyboard_daisy_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, EINSTEIN_KEYBOARD_DAISY, "Einstein keyboard daisy chain", tag, owner, clock, "einstein_keyboard", __FILE__),
		device_z80daisy_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void einstein_keyboard_daisy_device::device_start()
{
}

//**************************************************************************
//  DAISY CHAIN INTERFACE
//**************************************************************************

//-------------------------------------------------
//  z80daisy_irq_state - return the overall IRQ
//  state for this device
//-------------------------------------------------

int einstein_keyboard_daisy_device::z80daisy_irq_state()
{
	einstein_state *einstein = device().machine().driver_data<einstein_state>();

	if (einstein->m_interrupt & einstein->m_interrupt_mask & EINSTEIN_KEY_INT)
		return Z80_DAISY_INT;

	return 0;
}


//-------------------------------------------------
//  z80daisy_irq_ack - acknowledge an IRQ and
//  return the appropriate vector
//-------------------------------------------------

int einstein_keyboard_daisy_device::z80daisy_irq_ack()
{
	return 0xf7;
}

//-------------------------------------------------
//  z80daisy_irq_reti - clear the interrupt
//  pending state to allow other interrupts through
//-------------------------------------------------

void einstein_keyboard_daisy_device::z80daisy_irq_reti()
{
}

const device_type EINSTEIN_ADC_DAISY = &device_creator<einstein_adc_daisy_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z80ctc_device - constructor
//-------------------------------------------------

einstein_adc_daisy_device::einstein_adc_daisy_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, EINSTEIN_ADC_DAISY, "Einstein ADC daisy chain", tag, owner, clock, "einstein_adc_daisy", __FILE__),
		device_z80daisy_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void einstein_adc_daisy_device::device_start()
{
}

//**************************************************************************
//  DAISY CHAIN INTERFACE
//**************************************************************************

//-------------------------------------------------
//  z80daisy_irq_state - return the overall IRQ
//  state for this device
//-------------------------------------------------

int einstein_adc_daisy_device::z80daisy_irq_state()
{
	einstein_state *einstein = device().machine().driver_data<einstein_state>();

	if (einstein->m_interrupt & einstein->m_interrupt_mask & EINSTEIN_ADC_INT)
		return Z80_DAISY_INT;

	return 0;
}


//-------------------------------------------------
//  z80daisy_irq_ack - acknowledge an IRQ and
//  return the appropriate vector
//-------------------------------------------------

int einstein_adc_daisy_device::z80daisy_irq_ack()
{
	return 0xfb;
}

//-------------------------------------------------
//  z80daisy_irq_reti - clear the interrupt
//  pending state to allow other interrupts through
//-------------------------------------------------

void einstein_adc_daisy_device::z80daisy_irq_reti()
{
}


const device_type EINSTEIN_FIRE_DAISY = &device_creator<einstein_fire_daisy_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z80ctc_device - constructor
//-------------------------------------------------

einstein_fire_daisy_device::einstein_fire_daisy_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, EINSTEIN_FIRE_DAISY, "Einstein fire button daisy chain", tag, owner, clock, "einstein_fire_daisy", __FILE__),
		device_z80daisy_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void einstein_fire_daisy_device::device_start()
{
}

//**************************************************************************
//  DAISY CHAIN INTERFACE
//**************************************************************************

//-------------------------------------------------
//  z80daisy_irq_state - return the overall IRQ
//  state for this device
//-------------------------------------------------

int einstein_fire_daisy_device::z80daisy_irq_state()
{
	einstein_state *einstein = device().machine().driver_data<einstein_state>();

	if (einstein->m_interrupt & einstein->m_interrupt_mask & EINSTEIN_FIRE_INT)
		return Z80_DAISY_INT;

	return 0;
}


//-------------------------------------------------
//  z80daisy_irq_ack - acknowledge an IRQ and
//  return the appropriate vector
//-------------------------------------------------

int einstein_fire_daisy_device::z80daisy_irq_ack()
{
	return 0xfd;
}

//-------------------------------------------------
//  z80daisy_irq_reti - clear the interrupt
//  pending state to allow other interrupts through
//-------------------------------------------------

void einstein_fire_daisy_device::z80daisy_irq_reti()
{
}
