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


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_melodik_device::device_reset()
{
	m_exp->set_io_space(&io_space());

	io_space().install_write_handler(0x8000, 0x8000, 0, 0x3ffd, 0, write8smo_delegate(FUNC(ay8910_device::address_w), m_psg.target()));
	io_space().install_readwrite_handler(0xc000, 0xc000, 0, 0x3ffd, 0, read8smo_delegate(FUNC(ay8910_device::data_r), m_psg.target()), write8smo_delegate(FUNC(ay8910_device::data_w), m_psg.target()));
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ_LINE_MEMBER(spectrum_melodik_device::romcs)
{
	return m_exp->romcs();
}

READ8_MEMBER(spectrum_melodik_device::mreq_r)
{
	return m_exp->mreq_r(space, offset);
}

WRITE8_MEMBER(spectrum_melodik_device::mreq_w)
{
	if (m_exp->romcs())
		m_exp->mreq_w(space, offset, data);
}

READ8_MEMBER(spectrum_melodik_device::port_fe_r)
{
	uint8_t data = 0xff;

	if (m_exp->romcs())
		data &= m_exp->port_fe_r(space, offset);

	return data;
}
