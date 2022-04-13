// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore PET/VIC-20/C64/Plus-4 Datassette Port emulation

**********************************************************************/

#include "emu.h"
#include "cass.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(PET_DATASSETTE_PORT, pet_datassette_port_device, "pet_datassette_port", "Commodore Datasette Port")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_pet_datassette_port_interface - constructor
//-------------------------------------------------

device_pet_datassette_port_interface::device_pet_datassette_port_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "petcass")
{
	m_slot = dynamic_cast<pet_datassette_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_pet_datassette_port_interface - destructor
//-------------------------------------------------

device_pet_datassette_port_interface::~device_pet_datassette_port_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pet_datassette_port_device - constructor
//-------------------------------------------------

pet_datassette_port_device::pet_datassette_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PET_DATASSETTE_PORT, tag, owner, clock),
	device_single_card_slot_interface<device_pet_datassette_port_interface>(mconfig, *this),
	m_read_handler(*this), m_cart(nullptr)
{
}


//-------------------------------------------------
//  pet_datassette_port_device - destructor
//-------------------------------------------------

pet_datassette_port_device::~pet_datassette_port_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pet_datassette_port_device::device_start()
{
	m_cart = get_card_device();

	// resolve callbacks
	m_read_handler.resolve_safe();
}


READ_LINE_MEMBER( pet_datassette_port_device::read ) { int state = 1; if (m_cart != nullptr) state = m_cart->datassette_read(); return state; }
WRITE_LINE_MEMBER( pet_datassette_port_device::write ) { if (m_cart != nullptr) m_cart->datassette_write(state); }
READ_LINE_MEMBER( pet_datassette_port_device::sense_r ) { int state = 1; if (m_cart != nullptr) state = m_cart->datassette_sense(); return state; }
WRITE_LINE_MEMBER( pet_datassette_port_device::motor_w ) { if (m_cart != nullptr) m_cart->datassette_motor(state); }

WRITE_LINE_MEMBER( pet_datassette_port_device::read_w ) { m_read_handler(state); }


// slot devices
#include "c2n.h"
#include "diag264_lb_tape.h"

//-------------------------------------------------
//  SLOT_INTERFACE( cbm_datassette_devices )
//-------------------------------------------------

void cbm_datassette_devices(device_slot_interface &device)
{
	device.option_add("c2n", C2N);
	device.option_add("c1530", C1530);
}
