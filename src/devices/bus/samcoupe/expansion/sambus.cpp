// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    SAMBUS 4-slot Expansion Interface for SAM Coupe

***************************************************************************/

#include "emu.h"
#include "sambus.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SAM_SAMBUS, sam_sambus_device, "sambus", "SAMBUS 4-slot Expansion Interface")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sam_sambus_device::device_add_mconfig(machine_config &config)
{
	MSM6242(config, m_rtc, 32.768_kHz_XTAL);

	SAMCOUPE_EXPANSION(config, m_exp[0], samcoupe_expansion_modules);
	SAMCOUPE_EXPANSION(config, m_exp[1], samcoupe_expansion_modules);
	SAMCOUPE_EXPANSION(config, m_exp[2], samcoupe_expansion_modules);
	SAMCOUPE_EXPANSION(config, m_exp[3], samcoupe_expansion_modules);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sam_sambus_device - constructor
//-------------------------------------------------

sam_sambus_device::sam_sambus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SAM_SAMBUS, tag, owner, clock),
	device_samcoupe_expansion_interface(mconfig, *this),
	m_rtc(*this, "rtc"),
	m_exp(*this, "%u", 0U),
	m_print(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sam_sambus_device::device_start()
{
	// register for savestates
	save_item(NAME(m_print));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void sam_sambus_device::xmem_w(int state)
{
	for (int i = 0; i < 4; i++)
		m_exp[i]->xmem_w(state);
}

void sam_sambus_device::print_w(int state)
{
	for (int i = 0; i < 4; i++)
		m_exp[i]->print_w(state);

	m_print = state;
}

uint8_t sam_sambus_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	for (int i = 0; i < 4; i++)
		data &= m_exp[i]->mreq_r(offset);

	return data;
}

void sam_sambus_device::mreq_w(offs_t offset, uint8_t data)
{
	for (int i = 0; i < 4; i++)
		m_exp[i]->mreq_w(offset, data);
}

uint8_t sam_sambus_device::iorq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_print && (offset & 0x07) == 0x07)
		data &= m_rtc->read(offset >> 12);

	for (int i = 0; i < 4; i++)
		data &= m_exp[i]->iorq_r(offset);

	return data;
}

void sam_sambus_device::iorq_w(offs_t offset, uint8_t data)
{
	if (m_print && (offset & 0x07) == 0x07)
		m_rtc->write(offset >> 12, data);

	for (int i = 0; i < 4; i++)
		m_exp[i]->iorq_w(offset, data);
}
