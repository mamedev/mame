// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Amstrad PC1512 mouse port emulation

**********************************************************************/

#include "emu.h"
#include "bus/pc1512/mouse.h"



//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

DEFINE_DEVICE_TYPE(PC1512_MOUSE_PORT, pc1512_mouse_port_device, "pc1512_mouse_port", "Amstrad PC1512 mouse port")
DEFINE_DEVICE_TYPE(PC1512_MOUSE,      pc1512_mouse_device,      "pc1512_mouse",      "Amstrad PC1512 mouse")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_pc1512_mouse_port_interface - constructor
//-------------------------------------------------

device_pc1512_mouse_port_interface::device_pc1512_mouse_port_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "pc1512mouse")
{
	m_port = dynamic_cast<pc1512_mouse_port_device *>(device.owner());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pc1512_mouse_port_device - constructor
//-------------------------------------------------

pc1512_mouse_port_device::pc1512_mouse_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PC1512_MOUSE_PORT, tag, owner, clock),
	device_single_card_slot_interface<device_pc1512_mouse_port_interface>(mconfig, *this),
	m_write_x(*this),
	m_write_y(*this),
	m_write_m1(*this),
	m_write_m2(*this),
	m_device(nullptr)
{
}

pc1512_mouse_device::pc1512_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PC1512_MOUSE, tag, owner, clock),
	device_pc1512_mouse_port_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc1512_mouse_port_device::device_start()
{
	m_device = get_card_device();

	// resolve callbacks
	m_write_x.resolve_safe();
	m_write_y.resolve_safe();
	m_write_m1.resolve_safe();
	m_write_m2.resolve_safe();
}

void pc1512_mouse_device::device_start()
{
}


//-------------------------------------------------
//  INPUT_PORTS( mouse )
//-------------------------------------------------

static INPUT_PORTS_START( mouse )
	PORT_START("MOUSEB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Left Mouse Button") PORT_CODE(MOUSECODE_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, pc1512_mouse_device, mouse_button_1_changed, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Right Mouse Button") PORT_CODE(MOUSECODE_BUTTON2) PORT_CHANGED_MEMBER(DEVICE_SELF, pc1512_mouse_device, mouse_button_2_changed, 0)

	PORT_START("MOUSEX")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, pc1512_mouse_device, mouse_x_changed, 0)

	PORT_START("MOUSEY")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, pc1512_mouse_device, mouse_y_changed, 0)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor pc1512_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mouse );
}


//-------------------------------------------------
//  SLOT_INTERFACE( pc1512_mouse_port_devices )
//-------------------------------------------------

void pc1512_mouse_port_devices(device_slot_interface &device)
{
	device.option_add("mouse", PC1512_MOUSE);
}
