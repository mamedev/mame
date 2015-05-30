// license:BSD-3-Clause
// copyright-holders:smf

#include "dsjoy.h"

const device_type DEMPA_SHINBUNSHA_JOYSTICK = &device_creator<dempa_shinbunsha_joystick_device>;

dempa_shinbunsha_joystick_device::dempa_shinbunsha_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, DEMPA_SHINBUNSHA_JOYSTICK, "Dempa Shinbunsha Joystick", tag, owner, clock, "dempa_shinbunsha_joystick", __FILE__),
	device_centronics_peripheral_interface( mconfig, *this ),
	m_lptjoy(*this, "lptjoy"),
	m_data(0xff),
	m_perror(1)
{
}

void dempa_shinbunsha_joystick_device::device_start()
{
	save_item(NAME(m_data));
	save_item(NAME(m_perror));
}

void dempa_shinbunsha_joystick_device::update_perror()
{
	int perror = (~m_data & ~m_lptjoy->read() & 0x3f) == 0;

	if (m_perror != perror)
	{
		m_perror = perror;

		output_perror(perror);
	}
}

static INPUT_PORTS_START( dempa_shinbunsha_joystick )
	PORT_START("lptjoy")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_NAME("LPT Joystick Right") PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_NAME("LPT Joystick Left") PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_NAME("LPT Joystick Up") PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_NAME("LPT Joystick Down") PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("LPT Joystick Button 2") PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("LPT Joystick Button 1") PORT_PLAYER(1)
INPUT_PORTS_END

ioport_constructor dempa_shinbunsha_joystick_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( dempa_shinbunsha_joystick );
}
