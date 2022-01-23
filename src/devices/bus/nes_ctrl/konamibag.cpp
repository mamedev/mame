// license:BSD-3-Clause
// copyright-holders:kmg, Fabio Priuli
/**********************************************************************

    Nintendo Family Computer - Konami Exciting Boxing Air Bag

**********************************************************************/

#include "emu.h"
#include "konamibag.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_KONAMIBAG, nes_konamibag_device, "nes_konamibag", "Konami Exciting Boxing Air Bag")


static INPUT_PORTS_START( nes_konamibag )
	PORT_START("BAG.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Left Hook")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // opponent dodges right, unclear how this is triggered
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) // opponent dodges left, unclear how this is triggered
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Right Hook")

	PORT_START("BAG.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Left Jab")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Body")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Right Jab")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Straight")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_konamibag_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_konamibag );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_konamibag_device - constructor
//-------------------------------------------------

nes_konamibag_device::nes_konamibag_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NES_KONAMIBAG, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_sensor(*this, "BAG.%u", 0)
	, m_cur_sensor(0)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_konamibag_device::device_start()
{
	save_item(NAME(m_cur_sensor));
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u8 nes_konamibag_device::read_exp(offs_t offset)
{
	return (offset == 1) ? m_sensor[m_cur_sensor]->read() << 1 : 0;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_konamibag_device::write(u8 data)
{
	m_cur_sensor = BIT(data, 1);
}
