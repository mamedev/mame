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

MACHINE_CONFIG_START(spectrum_melodik_device::device_add_mconfig)
	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("ay8912", AY8912, XTAL(3'579'545) / 2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* passthru */
	MCFG_SPECTRUM_PASSTHRU_EXPANSION_SLOT_ADD()
MACHINE_CONFIG_END

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
	address_space& spaceio = machine().device("maincpu")->memory().space(AS_IO);
	m_slot = dynamic_cast<spectrum_expansion_slot_device *>(owner());

	spaceio.install_write_handler(0x8000, 0x8000, 0, 0x3ffd, 0, write8_delegate(FUNC(ay8910_device::address_w), (ay8910_device*)m_psg));
	spaceio.install_readwrite_handler(0xc000, 0xc000, 0, 0x3ffd, 0, read8_delegate(FUNC(ay8910_device::data_r), (ay8910_device*)m_psg), write8_delegate(FUNC(ay8910_device::data_w), (ay8910_device*)m_psg));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_melodik_device::device_reset()
{
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
