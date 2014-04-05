/**********************************************************************

    Sega Master System "Gender Adapter" emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "gender.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SMS_GENDER_ADAPTER = &device_creator<sms_gender_adapter_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sms_gender_adapter_device - constructor
//-------------------------------------------------

sms_gender_adapter_device::sms_gender_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SMS_GENDER_ADAPTER, "Gender Adapter", tag, owner, clock, "sms_gender_adapter", __FILE__),
	device_sms_expansion_slot_interface(mconfig, *this),
	m_subslot(*this, "subslot")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_gender_adapter_device::device_start()
{
}


//-------------------------------------------------
// read
//-------------------------------------------------

READ8_MEMBER(sms_gender_adapter_device::read)
{
	return m_subslot->read_cart(space, offset);
}

READ8_MEMBER(sms_gender_adapter_device::read_ram)
{
	return m_subslot->read_ram(space, offset);
}

int sms_gender_adapter_device::get_lphaser_xoffs()
{
	if (m_subslot->m_cart)
		return m_subslot->m_cart->get_lphaser_xoffs();
	else
		return 0;
}


//-------------------------------------------------
// write
//-------------------------------------------------

WRITE8_MEMBER(sms_gender_adapter_device::write_mapper)
{
	m_subslot->write_mapper(space, offset, data);
}

WRITE8_MEMBER(sms_gender_adapter_device::write)
{
	m_subslot->write_cart(space, offset, data);
}

WRITE8_MEMBER(sms_gender_adapter_device::write_ram)
{
	m_subslot->write_ram(space, offset, data);
}

static SLOT_INTERFACE_START(sms_cart)
	SLOT_INTERFACE_INTERNAL("rom",  SEGA8_ROM_STD)
	SLOT_INTERFACE_INTERNAL("codemasters",  SEGA8_ROM_CODEMASTERS)
	SLOT_INTERFACE_INTERNAL("4pak",  SEGA8_ROM_4PAK)
	SLOT_INTERFACE_INTERNAL("zemina",  SEGA8_ROM_ZEMINA)
	SLOT_INTERFACE_INTERNAL("nemesis",  SEGA8_ROM_NEMESIS)
	SLOT_INTERFACE_INTERNAL("janggun",  SEGA8_ROM_JANGGUN)
	SLOT_INTERFACE_INTERNAL("korean",  SEGA8_ROM_KOREAN)
	SLOT_INTERFACE_INTERNAL("korean_nb",  SEGA8_ROM_KOREAN_NB)
SLOT_INTERFACE_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( genderadp_slot )
	MCFG_SMS_CARTRIDGE_ADD("subslot", sms_cart, NULL)
MACHINE_CONFIG_END


machine_config_constructor sms_gender_adapter_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( genderadp_slot );
}
