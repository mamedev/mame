// license:BSD-3-Clause
// copyright-holders:Carl
#include "analogue.h"

const device_type PSX_ANALOG_JOYSTICK = &device_creator<psx_analog_joystick_device>;
const device_type PSX_DUALSHOCK = &device_creator<psx_dualshock_device>;

psx_analog_controller_device::psx_analog_controller_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_psx_controller_interface(mconfig, *this),
		m_pad0(*this, "PSXPAD0"),
		m_pad1(*this, "PSXPAD1"),
		m_rstickx(*this, "PSXRSTICKX"),
		m_rsticky(*this, "PSXRSTICKY"),
		m_lstickx(*this, "PSXLSTICKX"),
		m_lsticky(*this, "PSXLSTICKY")
{
}

psx_dualshock_device::psx_dualshock_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		psx_analog_controller_device(mconfig, PSX_DUALSHOCK, "Playstation Dualshock Pad", tag, owner, clock, "psx_dualshock_pad", __FILE__)
{
	m_type = DUALSHOCK;
}

psx_analog_joystick_device::psx_analog_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		psx_analog_controller_device(mconfig, PSX_ANALOG_JOYSTICK, "Playstation Analog Joystick", tag, owner, clock, "psx_analog_joystick", __FILE__)
{
	m_type = JOYSTICK;
}

void psx_analog_controller_device::device_reset()
{
	m_confmode = false;
	m_analogmode = false;
	m_analoglock = false;

	m_cmd = 0;
}

UINT8 psx_analog_controller_device::pad_data(int count, bool analog)
{
	UINT8 data = 0;
	switch(count)
	{
		case 2:
			data = m_pad0->read();
			if(!analog || (m_type == JOYSTICK))
				data |= 6; // l3/r3
			break;
		case 3:
			data = m_pad1->read();
			break;
		case 4:
			data = m_rstickx->read();
			break;
		case 5:
			data = m_rsticky->read();
			break;
		case 6:
			data = m_lstickx->read();
			break;
		case 7:
			data = m_lsticky->read();
			break;
	}
	return data;
}

bool psx_analog_controller_device::get_pad(int count, UINT8 *odata, UINT8 idata)
{
	if(m_confmode)
	{
		switch(count)
		{
			case 0:
				m_temp = 0;
				*odata = 0xf3;
				break;
			case 1:
				m_cmd = idata;
				if((m_cmd & 0xf0) != 0x40)
					return false;
				*odata = 0x5a;
				break;
			default:
				switch(m_cmd)
				{
					default: // 40,41,48,49,4a,4b,4f -- all unknown
						*odata = 0x00;
						break;
					case CONFIG_MODE: // 43
						if(count == 3)
							m_temp = idata;
						/* no break */
					case QUERY_PAD_STATE: // 42
						*odata = pad_data(count, true);
						break;
					case 0x44: // set mode and lock ?
						switch(count)
						{
							case 3:
								m_analogmode = idata ? true : false; // only 0x01 ?
								break;
							case 4:
								m_analoglock = idata ? true : false; // only 0x03 ?
								break;
						}
						*odata = 0x00;
						break;
					case 0x45: // get mode ?
					{
						const UINT8 val[] = { 1, 2, 0, 2, 1, 0 };
						if(count == 4)
							*odata = m_analogmode;
						else
							*odata = val[count-2];
						break;
					}
					case 0x46: // query act (vibrate) ?
					{
						const UINT8 val[2][6] = {{ 0, 0, 1, 2, 0, 10 },
												{ 0, 0, 1, 1, 1, 14 }};
						*odata = val[m_temp][count-2];
						if(count == 3)
							m_temp = idata ? 1 : 0;
						break;
					}
					case 0x47: // query comb (combination?) ?
					{
						const UINT8 val[] = { 0, 0, 2, 0, 1, 0 };
						*odata = val[count-2];
						break;
					}
					case 0x4c: // query mode ?
						switch(count)
						{
							case 3:
								m_temp = idata;
								/* no break */
							default:
								*odata = 0x00;
								break;
							case 5:
								*odata = m_analogmode ? 0x07 : 0x04; // ?
								break;
						}
						break;
					case 0x4d: // set act (vibrate) ?
						*odata = 0xff;
						break;
				}
				break;
			case 8:
				if(m_cmd == CONFIG_MODE)
					m_confmode = m_temp;
				return false;
		}
	}
	else if(m_analogmode)
	{
		switch(count)
		{
			case 0:
				if(m_type == JOYSTICK)
					*odata = 0x53;
				else
					*odata = 0x73;
				break;
			case 1:
				m_cmd = idata;
				if((m_cmd & 0xfe) != QUERY_PAD_STATE)
					return false;
				*odata = 0x5a;
				break;
			case 3:
				if(m_cmd == CONFIG_MODE)
					m_temp = idata;
				/* no break */
			default:
				*odata = pad_data(count, true);
				break;
			case 8:
				if(m_cmd == CONFIG_MODE)
					m_confmode = m_temp;
				return false;
		}
	}
	else
	{
		switch(count)
		{
			case 0:
				*odata = 0x41;
				break;
			case 1:
				m_cmd = idata;
				if((m_cmd & 0xfe) != QUERY_PAD_STATE)
					return false;
				*odata = 0x5a;
				break;
			case 3:
				if(m_cmd == CONFIG_MODE)
					m_temp = idata;
				/* no break */
			default:
				*odata = pad_data(count, false);
				break;
			case 4:
				if(m_cmd == CONFIG_MODE)
					m_confmode = m_temp;
				return false;
		}
	}
	return true;
}

static INPUT_PORTS_START( psx_analog_controller )
	PORT_START("PSXPAD0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("R3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("L3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SELECT )

	PORT_START("PSXPAD1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Square")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Cross")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Circle")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Triangle")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("R1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("L1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("R2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("L2")

	PORT_START("PSXRSTICKX")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_NAME("Right Analog X") PORT_SENSITIVITY(100)

	PORT_START("PSXRSTICKY")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_NAME("Right Analog Y") PORT_SENSITIVITY(100)

	PORT_START("PSXLSTICKX")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Z ) PORT_NAME("Left Analog X") PORT_SENSITIVITY(100)

	PORT_START("PSXLSTICKY")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_NAME("Left Analog Y") PORT_SENSITIVITY(100)

	PORT_START("PSXMISC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("Analog") PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, psx_analog_controller_device, change_mode, 0)
INPUT_PORTS_END

ioport_constructor psx_analog_controller_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(psx_analog_controller);
}

INPUT_CHANGED_MEMBER(psx_analog_controller_device::change_mode)
{
	if(!m_analoglock)
		m_analogmode = newval;
}
