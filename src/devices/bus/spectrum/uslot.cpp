// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Currah MicroSlot

**********************************************************************/

#include "emu.h"
#include "uslot.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_USLOT, spectrum_uslot_device, "spectrum_uslot", "Spectrum Currah \xC2\xB5Slot")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_uslot_device::device_add_mconfig(machine_config &config)
{
	/* passthru */
	SPECTRUM_EXPANSION_SLOT(config, m_exp1, spectrum_expansion_devices, nullptr);
	m_exp1->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp1->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));

	SPECTRUM_EXPANSION_SLOT(config, m_exp2, spectrum_expansion_devices, nullptr);
	m_exp2->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp2->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_uslot_device - constructor
//-------------------------------------------------

spectrum_uslot_device::spectrum_uslot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_USLOT, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_exp1(*this, "exp1")
	, m_exp2(*this, "exp2")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_uslot_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_uslot_device::device_reset()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

bool spectrum_uslot_device::romcs()
{
	return m_exp1->romcs() || m_exp2->romcs();
}

void spectrum_uslot_device::pre_opcode_fetch(offs_t offset)
{
	m_exp1->pre_opcode_fetch(offset);
	m_exp2->pre_opcode_fetch(offset);
}

void spectrum_uslot_device::post_opcode_fetch(offs_t offset)
{
	m_exp1->post_opcode_fetch(offset);
	m_exp2->post_opcode_fetch(offset);
}

void spectrum_uslot_device::pre_data_fetch(offs_t offset)
{
	m_exp1->pre_data_fetch(offset);
	m_exp2->pre_data_fetch(offset);
}

void spectrum_uslot_device::post_data_fetch(offs_t offset)
{
	m_exp1->post_data_fetch(offset);
	m_exp2->post_data_fetch(offset);
}

uint8_t spectrum_uslot_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_exp1->romcs())
		data &= m_exp1->mreq_r(offset);

	if (m_exp2->romcs())
		data &= m_exp2->mreq_r(offset);

	return data;
}

void spectrum_uslot_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_exp1->romcs())
		m_exp1->mreq_w(offset, data);

	if (m_exp2->romcs())
		m_exp2->mreq_w(offset, data);
}

uint8_t spectrum_uslot_device::iorq_r(offs_t offset)
{
	return m_exp1->iorq_r(offset) & m_exp2->iorq_r(offset);
}

void spectrum_uslot_device::iorq_w(offs_t offset, uint8_t data)
{
	m_exp1->iorq_w(offset, data);
	m_exp2->iorq_w(offset, data);
}
