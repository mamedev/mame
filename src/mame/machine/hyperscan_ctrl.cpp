// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*****************************************************************************

        Hyperscan controller

        The controller contains a SPC11122A MCU and a Winbond W55AD808 ADC.

        Pinouts:

            3   4
           2  I  5
            1 I 6

        1: Brown  : GND
        2: Yellow : DATA
        3: Grey   : CLK
        4: Green  : ENABLE
        5: Blue   : RESET
        6: Red    : VCC 3.3v

*****************************************************************************/

#include "emu.h"
#include "hyperscan_ctrl.h"


DEFINE_DEVICE_TYPE(HYPERSCAN_CTRL, hyperscan_ctrl_device, "hyperscan_ctrl", "Hyperscan controller HLE")


hyperscan_ctrl_device::hyperscan_ctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HYPERSCAN_CTRL, tag, owner, clock)
	, m_inputs(*this, "IN.%u", 0U)
{
}

static INPUT_PORTS_START( hyperscan_ctrl )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Blue")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START)   PORT_NAME("Start")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SELECT)  PORT_NAME("Select")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Left Shoulder")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Right Shoulder")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Left Trigger")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("Right Trigger")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.1")
	PORT_BIT(0x1f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Yellow")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Red")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Green")

	PORT_START("IN.2")
	PORT_BIT(0xff, 0x7f, IPT_AD_STICK_Y) PORT_NAME("Analog Y") PORT_REVERSE PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_CENTERDELTA(50)

	PORT_START("IN.3")
	PORT_BIT(0xff, 0x7f, IPT_AD_STICK_X) PORT_NAME("Analog X") PORT_REVERSE PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_CENTERDELTA(50)
INPUT_PORTS_END


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hyperscan_ctrl_device::device_start()
{
	save_item(NAME(m_input_data));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hyperscan_ctrl_device::device_reset()
{
	m_input_data[0] = m_input_data[1] = m_input_data[2] = m_input_data[3] = 0;
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor hyperscan_ctrl_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( hyperscan_ctrl );
}


uint16_t hyperscan_ctrl_device::read(offs_t offset)
{
	int port = (offset >> 4) & 0x0f;
	int addr = offset & 0x0f;

	if (addr < 4)
	{
		m_input_data[addr] = m_inputs[addr]->read();
		return m_input_data[addr];
	}
	else
	{
		// this is used to validate the inputs
		return ((m_input_data[0] + m_input_data[1] + m_input_data[2] + m_input_data[3]) << 2) | port;
	}
}
