// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Didaktik Melodik

**********************************************************************/

#include "emu.h"
#include "melodik.h"
#include "speaker.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_MELODIK, spectrum_melodik_device, "spectrum_melodik", "Didaktik Melodik")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_melodik_device::device_add_mconfig(machine_config &config)
{
	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8912(config, m_psg, 3.579545_MHz_XTAL / 2);
	m_psg->add_route(ALL_OUTPUTS, "mono", 0.25);

	/* passthru */
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_melodik_device - constructor
//-------------------------------------------------

spectrum_melodik_device::spectrum_melodik_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_MELODIK, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_exp(*this, "exp")
	, m_psg(*this, "ay8912")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_melodik_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ_LINE_MEMBER(spectrum_melodik_device::romcs)
{
	return m_exp->romcs();
}

void spectrum_melodik_device::pre_opcode_fetch(offs_t offset)
{
	m_exp->pre_opcode_fetch(offset);
}

uint8_t spectrum_melodik_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}

void spectrum_melodik_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_exp->romcs())
		m_exp->mreq_w(offset, data);
}

uint8_t spectrum_melodik_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	switch (offset & 0xc002)
	{
	case 0xc000:
		data &= m_psg->data_r();
		break;
	}
	return data;
}

void spectrum_melodik_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xc002)
	{
	case 0x8000:
		m_psg->address_w(data);
		break;
	case 0xc000:
		m_psg->data_w(data);
		break;
	}
	m_exp->iorq_w(offset, data);
}
