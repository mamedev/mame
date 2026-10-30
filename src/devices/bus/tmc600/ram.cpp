// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Telercas TMCE-200/TMCE-225 CMOS RAM card emulation

**********************************************************************/

#include "emu.h"
#include "ram.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(TMCE200, tmce200_device, "tmce200", "Telercas TMCE-200 8K RAM")
DEFINE_DEVICE_TYPE(TMCE225, tmce225_device, "tmce225", "Telercas TMCE-225 16K RAM")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tmce200_device - constructor
//-------------------------------------------------

tmce200_device::tmce200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	tmce200_device(mconfig, TMCE200, tag, owner, clock, 0x2000)
{
}

tmce200_device::tmce200_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, offs_t size) :
	device_t(mconfig, type, tag, owner, clock),
	device_tmc600_eurobus_card_interface(mconfig, *this),
	m_ram(*this, "ram", size, ENDIANNESS_LITTLE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tmce200_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tmce200_device::device_reset()
{
	m_slot->memspace()->install_ram(0x8000, 0x8000 + m_ram.bytes() - 1, m_ram);
}



//-------------------------------------------------
//  tmce225_device - constructor
//-------------------------------------------------

tmce225_device::tmce225_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	tmce200_device(mconfig, TMCE225, tag, owner, clock, 0x4000)
{
}
