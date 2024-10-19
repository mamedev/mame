// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
#include "emu.h"
#include "hlemouse.h"

//#define VERBOSE 1
#include "logmacro.h"

/***************************************************************************
    DEVICE TYPE GLOBALS
***************************************************************************/

DEFINE_DEVICE_TYPE(HP_46060B_MOUSE, bus::hp_hil::hle_hp_46060b_device, "hp_46060b", "HP 46060B Mouse")

namespace bus::hp_hil {

namespace {

/***************************************************************************
    INPUT PORT DEFINITIONS
***************************************************************************/

INPUT_PORTS_START( hle_hp_46060b_device )
	PORT_START("mouse_buttons")
	PORT_BIT(hle_hp_46060b_device::MOUSE_LBUTTON, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Left Button") PORT_CODE(MOUSECODE_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hle_hp_46060b_device::mouse_button), 0)
	PORT_BIT(hle_hp_46060b_device::MOUSE_MBUTTON, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Mouse Middle Button") PORT_CODE(MOUSECODE_BUTTON3) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hle_hp_46060b_device::mouse_button), 0)
	PORT_BIT(hle_hp_46060b_device::MOUSE_RBUTTON, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Mouse Right Button") PORT_CODE(MOUSECODE_BUTTON2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hle_hp_46060b_device::mouse_button), 0)

	PORT_START("mouse_x")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hle_hp_46060b_device::mouse_x), 0)
	PORT_START("mouse_y")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hle_hp_46060b_device::mouse_y), 0)
INPUT_PORTS_END
} // anonymous namespace

hle_hp_46060b_device::hle_hp_46060b_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: hle_device_base(mconfig, HP_46060B_MOUSE, tag, owner, clock),
	mouse_x_delta{0},
	mouse_y_delta{0},
	mouse_buttons{0}
{ }

void hle_hp_46060b_device::device_reset()
{
	m_fifo.clear();

	mouse_x_delta = mouse_y_delta = 0;
}

int hle_hp_46060b_device::hil_poll()
{
	int frames = 0;
	uint16_t reports = 0;

	if (mouse_x_delta || mouse_y_delta)
		reports |= 2;

	if (!m_fifo.empty())
		reports |= 0x40;

	if (!reports)
		return 0;

	m_hp_hil_mlc->hil_write(m_device_id16 | reports);  // report X & Y data
	frames++;
	if (mouse_x_delta || mouse_y_delta) {
		m_hp_hil_mlc->hil_write(m_device_id16 | (uint8_t)mouse_x_delta);
		m_hp_hil_mlc->hil_write(m_device_id16 | (uint8_t)mouse_y_delta);
		mouse_x_delta = 0;
		mouse_y_delta = 0;
		frames+=2;
	}

	while (!m_fifo.empty()) {
		m_hp_hil_mlc->hil_write(m_device_id16 | m_fifo.dequeue());
		frames++;
	}

	return frames;
}


void hle_hp_46060b_device::hil_idd()
{
	m_hp_hil_mlc->hil_write(m_device_id16 | 0x68);
	m_hp_hil_mlc->hil_write(m_device_id16 | 0x12); // report X & Y Axes
	m_hp_hil_mlc->hil_write(m_device_id16 | 0xc2); // resolution Low
	m_hp_hil_mlc->hil_write(m_device_id16 | 0x1e); // resolution High
	return;
}

ioport_constructor hle_hp_46060b_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hle_hp_46060b_device);
}

INPUT_CHANGED_MEMBER(hle_hp_46060b_device::mouse_y)
{
	int8_t delta = newval - oldval;
	mouse_y_delta += -delta;
}

INPUT_CHANGED_MEMBER(hle_hp_46060b_device::mouse_x)
{
	int8_t delta = newval - oldval;
	mouse_x_delta += delta;
}

INPUT_CHANGED_MEMBER(hle_hp_46060b_device::mouse_button)
{
	const ioport_value data = field.port().read();
	uint32_t _data = data & MOUSE_BUTTONS;

	switch(_data ^ mouse_buttons) {
		case MOUSE_LBUTTON:
			if (!m_fifo.full()) {
				m_fifo.enqueue(_data ? 0x80 : 0x81);
			}
			break;
		case MOUSE_RBUTTON:
			if (!m_fifo.full()) {
				m_fifo.enqueue(_data ? 0x82 : 0x83);
			}
			break;
	}
	mouse_buttons = data;
}

} // namespace bus::hp_hil
