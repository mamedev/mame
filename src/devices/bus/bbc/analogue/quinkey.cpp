// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Microwriter Quinkey

    The Quinkey is provided with an interface that connects to the BBC Micro
    analogue port, which provides sockets for connecting upto four Quinkey
    keyboards.

**********************************************************************/

#include "emu.h"
#include "quinkey.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_QUINKEY_INTF, bbc_quinkey_intf_device, "bbc_quinkey_intf", "Microwriter Quinkey Interface")
DEFINE_DEVICE_TYPE(BBC_QUINKEY_SLOT, bbc_quinkey_slot_device, "bbc_quinkey_slot", "Microwriter Quinkey slot")
DEFINE_DEVICE_TYPE(BBC_QUINKEY,      bbc_quinkey_device,      "bbc_quinkey",      "Microwriter Quinkey")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_quinkey_intf_device::device_add_mconfig(machine_config &config)
{
	BBC_QUINKEY_SLOT(config, "1", bbc_quinkey_devices, "quinkey");
	BBC_QUINKEY_SLOT(config, "2", bbc_quinkey_devices, nullptr);
	BBC_QUINKEY_SLOT(config, "3", bbc_quinkey_devices, nullptr);
	BBC_QUINKEY_SLOT(config, "4", bbc_quinkey_devices, nullptr);
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( quinkey )
	PORT_START("KEYS")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Command")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Thumb")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Index Finger")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Middle Finger")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Ring Finger")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Little Finger")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor bbc_quinkey_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(quinkey);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_quinkey_intf_device - constructor
//-------------------------------------------------

bbc_quinkey_intf_device::bbc_quinkey_intf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_QUINKEY_INTF, tag, owner, clock)
	, device_bbc_analogue_interface(mconfig, *this)
	, m_quinkey(*this, "%u", 1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_quinkey_intf_device::device_start()
{
}


uint16_t bbc_quinkey_intf_device::ch_r(offs_t channel)
{
	return m_quinkey[channel]->read() << 8;
}


//-------------------------------------------------
//  device_bbc_quinkey_interface - constructor
//-------------------------------------------------

device_bbc_quinkey_interface::device_bbc_quinkey_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "bbcquinkey")
	, m_slot(dynamic_cast<bbc_quinkey_slot_device *>(device.owner()))
{
}


//-------------------------------------------------
//  bbc_quinkey_slot_device - constructor
//-------------------------------------------------

bbc_quinkey_slot_device::bbc_quinkey_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_QUINKEY_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_bbc_quinkey_interface>(mconfig, *this)
	, m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_quinkey_slot_device::device_start()
{
	m_card = get_card_device();
}


uint8_t bbc_quinkey_slot_device::read()
{
	if (m_card)
		return m_card->read();
	else
		return 0x00;
}


//-------------------------------------------------
//  bbc_quinkey_device - constructor
//-------------------------------------------------

bbc_quinkey_device::bbc_quinkey_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_QUINKEY, tag, owner, clock)
	, device_bbc_quinkey_interface(mconfig, *this)
	, m_keys(*this, "KEYS")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_quinkey_device::device_start()
{
}


uint8_t bbc_quinkey_device::read()
{
	return m_keys->read();
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_quinkey_devices )
//-------------------------------------------------

void bbc_quinkey_devices(device_slot_interface &device)
{
	device.option_add("quinkey", BBC_QUINKEY);
}
