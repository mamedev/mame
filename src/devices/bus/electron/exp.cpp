// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

        Electron Expansion Port emulation

**********************************************************************/

#include "exp.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ELECTRON_EXPANSION_SLOT = &device_creator<electron_expansion_slot_device>;


//**************************************************************************
//  DEVICE ELECTRON_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_electron_expansion_interface - constructor
//-------------------------------------------------

device_electron_expansion_interface::device_electron_expansion_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<electron_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_electron_expansion_interface - destructor
//-------------------------------------------------

device_electron_expansion_interface::~device_electron_expansion_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_expansion_slot_device - constructor
//-------------------------------------------------

electron_expansion_slot_device::electron_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ELECTRON_EXPANSION_SLOT, "Acorn Electron Expansion port", tag, owner, clock, "electron_expansion_slot", __FILE__),
		device_slot_interface(mconfig, *this),
	m_io(nullptr),
	m_card(nullptr),
	m_irq_handler(*this),
	m_nmi_handler(*this),
	m_reset_handler(*this)
{
}


//-------------------------------------------------
//  expansion_slot_device - destructor
//-------------------------------------------------

electron_expansion_slot_device::~electron_expansion_slot_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_expansion_slot_device::device_start()
{
	// resolve callbacks
	m_irq_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
	m_reset_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void electron_expansion_slot_device::device_reset()
{
	if (get_card_device())
	{
		get_card_device()->reset();
	}
}

//-------------------------------------------------
//  set_io_space - set address space we are attached to
//-------------------------------------------------

void electron_expansion_slot_device::set_io_space(address_space *io)
{
	m_io = io;
}


//-------------------------------------------------
//  SLOT_INTERFACE( electron_expansion_devices )
//-------------------------------------------------


// slot devices
//#include "plus1.h"
//#include "plus3.h"
//#include "aplus3.h"
//#include "aplus5.h"
//#include "slogger.h"
//#include "fbjoy.h"
#include "m2105.h"


SLOT_INTERFACE_START( electron_expansion_devices )
	//SLOT_INTERFACE("plus1", ELECTRON_PLUS1)
	//SLOT_INTERFACE("plus3", ELECTRON_PLUS3)
	//SLOT_INTERFACE("aplus3", ELECTRON_APLUS3)
	//SLOT_INTERFACE("aplus5", ELECTRON_APLUS5)
	//SLOT_INTERFACE("slogger", ELECTRON_SLOGGER)
	//SLOT_INTERFACE("fbjoy", ELECTRON_FBJOY)
	SLOT_INTERFACE("m2105", ELECTRON_M2105)
SLOT_INTERFACE_END
