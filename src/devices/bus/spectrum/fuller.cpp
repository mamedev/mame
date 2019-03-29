// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Fuller Box Games Unit

**********************************************************************/

#include "emu.h"
#include "fuller.h"
#include "speaker.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_FULLER, spectrum_fuller_device, "spectrum_fuller", "Fuller Box")


//-------------------------------------------------
//  INPUT_PORTS( fuller )
//-------------------------------------------------

static INPUT_PORTS_START( fuller )
	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_fuller_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( fuller );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_fuller_device::device_add_mconfig(machine_config &config)
{
	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8912(config, m_psg, 3.579545_MHz_XTAL / 2); // unverified clock
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
//  spectrum_fuller_device - constructor
//-------------------------------------------------

spectrum_fuller_device::spectrum_fuller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_FULLER, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_exp(*this, "exp")
	, m_psg(*this, "ay8912")
	, m_joy(*this, "JOY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_fuller_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ_LINE_MEMBER(spectrum_fuller_device::romcs)
{
	return m_exp->romcs();
}

void spectrum_fuller_device::opcode_fetch(offs_t offset)
{
	m_exp->opcode_fetch(offset);
}

uint8_t spectrum_fuller_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}

void spectrum_fuller_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_exp->romcs())
		m_exp->mreq_w(offset, data);
}

uint8_t spectrum_fuller_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	switch (offset & 0xff)
	{
	case 0x5f:
		data &= m_psg->data_r();
		break;
	case 0x7f:
		data &= m_joy->read() | (0xff ^ 0x8f);
		break;
	}
	return data;
}

void spectrum_fuller_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xff)
	{
	case 0x3f:
		m_psg->address_w(data);
		break;
	case 0x5f:
		m_psg->data_w(data);
		break;
	}
	m_exp->iorq_w(offset, data);
}
