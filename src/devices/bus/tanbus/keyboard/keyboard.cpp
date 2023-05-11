// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Microtan Keyboard Interface

**********************************************************************/

#include "emu.h"
#include "keyboard.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MICROTAN_KBD_SLOT, microtan_kbd_slot_device, "microtan_kbd_slot", "Microtan Keyboard Interface")



//**************************************************************************
//  DEVICE MICROTAN_KBD PORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_microtan_kbd_interface - constructor
//-------------------------------------------------

device_microtan_kbd_interface::device_microtan_kbd_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "microtankbd")
{
	m_slot = dynamic_cast<microtan_kbd_slot_device *>(device.owner());
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  microtan_kbd_slot_device - constructor
//-------------------------------------------------

microtan_kbd_slot_device::microtan_kbd_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MICROTAN_KBD_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_microtan_kbd_interface>(mconfig, *this)
	, m_kbd(nullptr)
	, m_strobe_handler(*this)
	, m_reset_handler(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void microtan_kbd_slot_device::device_start()
{
	m_kbd = get_card_device();

	// resolve callbacks
	m_strobe_handler.resolve_safe();
	m_reset_handler.resolve_safe();
}


//-------------------------------------------------
//  kbd_in
//-------------------------------------------------

u8 microtan_kbd_slot_device::read()
{
	if (m_kbd)
		return m_kbd->read() & 0x7f;
	else
		return 0x7f;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void microtan_kbd_slot_device::write(uint8_t data)
{
	if (m_kbd)
		m_kbd->write(data);
}

//-------------------------------------------------
//  SLOT_INTERFACE( microtan_kbd_devices )
//-------------------------------------------------


// slot devices
#include "mt006.h"
#include "mt009.h"
//#include "mt0020.h"
#include "spinveti.h"


void microtan_kbd_devices(device_slot_interface &device)
{
	device.option_add("mt006", MICROTAN_KBD_MT006);
	device.option_add("mt009", MICROTAN_KBD_MT009);
	//device.option_add("mt0020", MICROTAN_KBD_MT0020);
	device.option_add("spinveti", MICROTAN_KBD_SPINVETI);
}
