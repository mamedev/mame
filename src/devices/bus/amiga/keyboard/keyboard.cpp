// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Amiga Keyboard Interface

***************************************************************************/

#include "emu.h"
#include "keyboard.h"
#include "a500.h"
#include "a1200.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_KEYBOARD_INTERFACE, amiga_keyboard_bus_device, "amiga_kbd", "Amiga Keyboard Interface")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  amiga_keyboard_bus_device - constructor
//-------------------------------------------------

amiga_keyboard_bus_device::amiga_keyboard_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_KEYBOARD_INTERFACE, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_kbd(nullptr),
	m_kclk_handler(*this), m_kdat_handler(*this), m_krst_handler(*this)
{
}

//-------------------------------------------------
//  amiga_keyboard_bus_device - destructor
//-------------------------------------------------

amiga_keyboard_bus_device::~amiga_keyboard_bus_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void amiga_keyboard_bus_device::device_start()
{
	// get connected keyboard
	m_kbd = dynamic_cast<device_amiga_keyboard_interface *>(get_card_device());

	// resolve callbacks
	m_kclk_handler.resolve_safe();
	m_kdat_handler.resolve_safe();
	m_krst_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void amiga_keyboard_bus_device::device_reset()
{
}

//-------------------------------------------------
//  host to module interface
//-------------------------------------------------

WRITE_LINE_MEMBER( amiga_keyboard_bus_device::kdat_in_w )
{
	if (m_kbd)
		m_kbd->kdat_w(state);
}


//**************************************************************************
//  KEYBOARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_amiga_keyboard_interface - constructor
//-------------------------------------------------

device_amiga_keyboard_interface::device_amiga_keyboard_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_host = dynamic_cast<amiga_keyboard_bus_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_amiga_keyboard_interface - destructor
//-------------------------------------------------

device_amiga_keyboard_interface::~device_amiga_keyboard_interface()
{
}


//**************************************************************************
//  SLOT INTERFACE
//**************************************************************************

void amiga_keyboard_devices(device_slot_interface &device)
{
	device.option_add("a500_us", A500_KBD_US);
	device.option_add("a500_de", A500_KBD_DE);
	device.option_add("a500_fr", A500_KBD_FR);
	device.option_add("a500_it", A500_KBD_IT);
	device.option_add("a500_se", A500_KBD_SE);
	device.option_add("a500_es", A500_KBD_ES);
	device.option_add("a500_dk", A500_KBD_DK);
	device.option_add("a500_ch", A500_KBD_CH);
	device.option_add("a500_no", A500_KBD_NO);
	device.option_add("a500_gb", A500_KBD_GB);
	device.option_add("a1200_us", A1200_KBD);
}
