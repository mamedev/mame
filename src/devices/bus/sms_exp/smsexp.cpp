// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System expansion slot emulation

**********************************************************************/

#include "emu.h"
#include "smsexp.h"

// slot devices
#include "gender.h"




//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SMS_EXPANSION_SLOT, sms_expansion_slot_device, "sms_expansion_slot", "Sega SMS expansion slot")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_sms_expansion_slot_interface - constructor
//-------------------------------------------------

device_sms_expansion_slot_interface::device_sms_expansion_slot_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
}


//-------------------------------------------------
//  ~device_sms_expansion_slot_interface - destructor
//-------------------------------------------------

device_sms_expansion_slot_interface::~device_sms_expansion_slot_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sms_expansion_slot_device - constructor
//-------------------------------------------------

sms_expansion_slot_device::sms_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMS_EXPANSION_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_device(nullptr)
{
}


//-------------------------------------------------
//  sms_expansion_slot_device - destructor
//-------------------------------------------------

sms_expansion_slot_device::~sms_expansion_slot_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_expansion_slot_device::device_start()
{
	m_device = dynamic_cast<device_sms_expansion_slot_interface *>(get_card_device());
}


//-------------------------------------------------
//  SLOT_INTERFACE( sms_expansion_devices )
//-------------------------------------------------

void sms_expansion_devices(device_slot_interface &device)
{
	device.option_add("genderadp", SMS_GENDER_ADAPTER);
}
