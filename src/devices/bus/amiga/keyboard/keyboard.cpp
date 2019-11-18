// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Amiga Keyboard Interface

***************************************************************************/

#include "emu.h"
#include "keyboard.h"

#include "a1200.h"
#include "a2000.h"
#include "mitsumi.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_KEYBOARD_INTERFACE, amiga_keyboard_bus_device, "amiga_kbd", "Amiga Keyboard Interface")

template class device_finder<device_amiga_keyboard_interface, false>;
template class device_finder<device_amiga_keyboard_interface, true>;


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  amiga_keyboard_bus_device - constructor
//-------------------------------------------------

amiga_keyboard_bus_device::amiga_keyboard_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_KEYBOARD_INTERFACE, tag, owner, clock),
	device_single_card_slot_interface<device_amiga_keyboard_interface>(mconfig, *this),
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
	m_kbd = get_card_device();

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
	device_interface(device, "amigakbd")
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
	device.option_add("a1000_us", A1000_KBD_US);
	device.option_add("a1000_de", A1000_KBD_DE);
	device.option_add("a1000_fr", A1000_KBD_FR);
	device.option_add("a1000_it", A1000_KBD_IT);
	device.option_add("a1000_se", A1000_KBD_SE);
	device.option_add("a1000_dk", A1000_KBD_DK);
	device.option_add("a1000_gb", A1000_KBD_GB);
	device.option_add("a1200_us", A1200_KBD); // FIXME: sort this out properly when we get mask ROM dumps
	device.option_add("a2000_g80_us", A2000_KBD_G80_US);
	device.option_add("a2000_g80_de", A2000_KBD_G80_DE);
	device.option_add("a2000_g80_se", A2000_KBD_G80_SE);
	device.option_add("a2000_g80_dk", A2000_KBD_G80_DK);
	device.option_add("a2000_g80_gb", A2000_KBD_G80_GB);
	device.option_add("a2000_us", A2000_KBD_US);
	device.option_add("a2000_de", A2000_KBD_DE);
	device.option_add("a2000_fr", A2000_KBD_FR);
	device.option_add("a2000_it", A2000_KBD_IT);
	device.option_add("a2000_se", A2000_KBD_SE);
	device.option_add("a2000_es", A2000_KBD_ES);
	device.option_add("a2000_dk", A2000_KBD_DK);
	device.option_add("a2000_ch", A2000_KBD_CH);
	device.option_add("a2000_no", A2000_KBD_NO);
	device.option_add("a2000_gb", A2000_KBD_GB);
}

void a500_keyboard_devices(device_slot_interface &device)
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
}

void a600_keyboard_devices(device_slot_interface &device)
{
	device.option_add("a600_us", A600_KBD_US);
	device.option_add("a600_de", A600_KBD_DE);
	device.option_add("a600_fr", A600_KBD_FR);
	device.option_add("a600_it", A600_KBD_IT);
	device.option_add("a600_se", A600_KBD_SE);
	device.option_add("a600_es", A600_KBD_ES);
	device.option_add("a600_dk", A600_KBD_DK);
	device.option_add("a600_ch", A600_KBD_CH);
	device.option_add("a600_no", A600_KBD_NO);
	device.option_add("a600_gb", A600_KBD_GB);
}
