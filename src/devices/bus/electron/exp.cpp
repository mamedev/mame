// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

        Electron Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ELECTRON_EXPANSION_SLOT = device_creator<electron_expansion_slot_device>;


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

electron_expansion_slot_device::electron_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, ELECTRON_EXPANSION_SLOT, "Acorn Electron Expansion port", tag, owner, clock, "electron_expansion_slot", __FILE__),
		device_slot_interface(mconfig, *this),
	m_card(nullptr),
	m_irq_handler(*this),
	m_nmi_handler(*this)
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
	m_card = dynamic_cast<device_electron_expansion_interface *>(get_card_device());

	// resolve callbacks
	m_irq_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
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
//  SLOT_INTERFACE( electron_expansion_devices )
//-------------------------------------------------


// slot devices
#include "fbjoy.h"
//#include "fbprint.h"
//#include "jafamode7.h"
//#include "plus1.h"
#include "plus3.h"
#include "pwrjoy.h"
//#include "rombox.h"
//#include "romboxplus.h"
#include "m2105.h"
//#include "voxbox.h"


SLOT_INTERFACE_START( electron_expansion_devices )
	//SLOT_INTERFACE("ap1", ELECTRON_AP1)
	SLOT_INTERFACE("fbjoy", ELECTRON_FBJOY)
	//SLOT_INTERFACE("fbprint", ELECTRON_FBPRINT)
	//SLOT_INTERFACE("jafamode7", ELECTRON_JAFAMODE7)
	//SLOT_INTERFACE("plus1", ELECTRON_PLUS1)
	SLOT_INTERFACE("plus3", ELECTRON_PLUS3)
	SLOT_INTERFACE("pwrjoy", ELECTRON_PWRJOY)
	//SLOT_INTERFACE("rombox", ELECTRON_ROMBOX)
	//SLOT_INTERFACE("romboxplus", ELECTRON_ROMBOXPLUS)
	SLOT_INTERFACE("m2105", ELECTRON_M2105)
	//SLOT_INTERFACE("voxbox", ELECTRON_VOXBOX)
SLOT_INTERFACE_END
