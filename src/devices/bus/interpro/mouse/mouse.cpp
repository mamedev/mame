// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * A high-level emulation implementation of the Intergraph InterPro mouse.
 *
 * Little information is available on this hardware. The mouse has 3 buttons,
 * all of which are commonly used by the system software, but earlier InterPro
 * systems appear to have supported many more buttons; it's not known if those
 * earlier versions use the same port or protocol. The mouse has a male D-sub
 * 9-pin connector, using an unknown, probably TTL level signalling protocol.
 *
 * It's probable that this mouse can be connected to the mouse port on InterPro
 * system main boards as well as the mouse port(s) on EDGE graphics boards.
 */

#include "emu.h"
#include "mouse.h"

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(INTERPRO_MOUSE_PORT, interpro_mouse_port_device, "interpro_mouse_port", "InterPro Mouse Port")
DEFINE_DEVICE_TYPE(INTERPRO_MOUSE, interpro_mouse_device, "interpro_mouse", "InterPro Mouse")

static INPUT_PORTS_START(interpro_mouse)
	PORT_START("mouse_buttons")
	PORT_BIT(interpro_mouse_device::MOUSE_LBUTTON, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Left Button") PORT_CODE(MOUSECODE_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, interpro_mouse_device, mouse_button, nullptr)
	PORT_BIT(interpro_mouse_device::MOUSE_MBUTTON, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Mouse Middle Button") PORT_CODE(MOUSECODE_BUTTON3) PORT_CHANGED_MEMBER(DEVICE_SELF, interpro_mouse_device, mouse_button, nullptr)
	PORT_BIT(interpro_mouse_device::MOUSE_RBUTTON, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Mouse Right Button") PORT_CODE(MOUSECODE_BUTTON2) PORT_CHANGED_MEMBER(DEVICE_SELF, interpro_mouse_device, mouse_button, nullptr)

	PORT_START("mouse_x")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, interpro_mouse_device, mouse_x, nullptr)
	PORT_START("mouse_y")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, interpro_mouse_device, mouse_y, nullptr)
INPUT_PORTS_END

interpro_mouse_port_device::interpro_mouse_port_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INTERPRO_MOUSE_PORT, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_state_func(*this)
	, m_device(nullptr)
{
}

void interpro_mouse_port_device::device_config_complete()
{
	m_device = dynamic_cast<device_interpro_mouse_port_interface *>(get_card_device());
}

void interpro_mouse_port_device::device_validity_check(validity_checker &valid) const
{
	device_t *const card(get_card_device());

	if (card && !dynamic_cast<device_interpro_mouse_port_interface *>(card))
		osd_printf_error("Device %s (%s) does not implement device_interpro_mouse_port_interface\n", card->tag(), card->name());
}

void interpro_mouse_port_device::device_start()
{
	m_state_func.resolve_safe();
}

device_interpro_mouse_port_interface::device_interpro_mouse_port_interface(machine_config const &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
	, m_port(dynamic_cast<interpro_mouse_port_device *>(device.owner()))
{
}

interpro_mouse_device::interpro_mouse_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, INTERPRO_MOUSE, tag, owner, clock)
	, device_interpro_mouse_port_interface(mconfig, *this)
{
}

void interpro_mouse_device::device_start()
{
}

void interpro_mouse_device::device_reset()
{
}

ioport_constructor interpro_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(interpro_mouse);
}

void interpro_mouse_devices(device_slot_interface &device)
{
	device.option_add("interpro_mouse", INTERPRO_MOUSE);
}

INPUT_CHANGED_MEMBER(interpro_mouse_device::mouse_button)
{
	const ioport_value data = field.port().read();

	LOG("mouse_button 0x%08x\n", data);

	state_w(machine().dummy_space(), 0, data & MOUSE_BUTTONS, MOUSE_BUTTONS);
}

INPUT_CHANGED_MEMBER(interpro_mouse_device::mouse_x)
{
	// compute x delta
	int delta = newval - oldval;
	if (delta > 0x80)
		delta -= 0x100;
	else if (delta < -0x80)
		delta += 0x100;

	LOG("mouse_x delta %d\n", delta);

	state_w(machine().dummy_space(), 0, (delta << 8) & MOUSE_XPOS, MOUSE_XPOS);
}

INPUT_CHANGED_MEMBER(interpro_mouse_device::mouse_y)
{
	// compute y delta
	int delta = newval - oldval;
	if (delta > 0x80)
		delta -= 0x100;
	else if (delta < -0x80)
		delta += 0x100;

	LOG("mouse_y delta %d\n", delta);

	state_w(machine().dummy_space(), 0, (delta << 0) & MOUSE_YPOS, MOUSE_YPOS);
}
