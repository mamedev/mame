// license:BSD-3-Clause
// copyright-holders:R. Belmont

// High-level Apple Desktop Bus mouse

#include "emu.h"
#include "adbhlemouse.h"

#include "adbhle.h"

namespace
{

class adb_hle_mouse_device : public adb_hle_device
{
public:
	adb_hle_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual unsigned adb_talk(u8 reg, std::span<u8> data) override;
	virtual void adb_talk_complete(u8 reg, bool success) override;
	virtual void adb_poll() override;
	virtual void adb_bus_reset() override;

private:
	void reset_mouse();

	required_ioport m_buttons;
	required_ioport m_mouse_x;
	required_ioport m_mouse_y;

	u8 m_last_buttons;
	u8 m_last_x;
	u8 m_last_y;
	u8 m_pending_buttons;
	u8 m_pending_x;
	u8 m_pending_y;
	bool m_report_pending;
};

static INPUT_PORTS_START(adb_hle_mouse)
	PORT_START("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Button 1") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Mouse Button 2") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("MOUSEX")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("MOUSEY")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)
INPUT_PORTS_END

adb_hle_mouse_device::adb_hle_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	adb_hle_device(mconfig, ADB_HLE_MOUSE, tag, owner, clock, 3, 1),
	m_buttons(*this, "BUTTONS"),
	m_mouse_x(*this, "MOUSEX"),
	m_mouse_y(*this, "MOUSEY"),
	m_last_buttons(0),
	m_last_x(0),
	m_last_y(0),
	m_pending_buttons(0),
	m_pending_x(0),
	m_pending_y(0),
	m_report_pending(false)
{
}

ioport_constructor adb_hle_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(adb_hle_mouse);
}

void adb_hle_mouse_device::device_start()
{
	adb_hle_device::device_start();

	save_item(NAME(m_last_buttons));
	save_item(NAME(m_last_x));
	save_item(NAME(m_last_y));
	save_item(NAME(m_pending_buttons));
	save_item(NAME(m_pending_x));
	save_item(NAME(m_pending_y));
	save_item(NAME(m_report_pending));
}

void adb_hle_mouse_device::device_reset()
{
	adb_hle_device::device_reset();
	reset_mouse();
}

unsigned adb_hle_mouse_device::adb_talk(u8 reg, std::span<u8> data)
{
	if (reg != 0)
	{
		return 0;
	}

	u8 const buttons = m_buttons->read() & 0x03;
	u8 const x = m_mouse_x->read();
	u8 const y = m_mouse_y->read();
	if ((buttons == m_last_buttons) && (x == m_last_x) && (y == m_last_y))
	{
		return 0;
	}

	s8 const delta_x = s8(x - m_last_x);
	s8 const delta_y = s8(y - m_last_y);
	data[0] = (BIT(~buttons, 0) << 7) | (u8(delta_y) & 0x7f);
	data[1] = (BIT(~buttons, 1) << 7) | (u8(delta_x) & 0x7f);

	m_pending_buttons = buttons;
	m_pending_x = x;
	m_pending_y = y;
	m_report_pending = true;
	return 2;
}

void adb_hle_mouse_device::adb_talk_complete(u8 reg, bool success)
{
	if ((reg == 0) && m_report_pending && success)
	{
		m_last_buttons = m_pending_buttons;
		m_last_x = m_pending_x;
		m_last_y = m_pending_y;
	}
	m_report_pending = false;
	adb_poll();
}

void adb_hle_mouse_device::adb_poll()
{
	u8 const buttons = m_buttons->read() & 0x03;
	u8 const x = m_mouse_x->read();
	u8 const y = m_mouse_y->read();
	set_service_request((buttons != m_last_buttons) || (x != m_last_x) || (y != m_last_y));
}

void adb_hle_mouse_device::adb_bus_reset()
{
	reset_mouse();
}

void adb_hle_mouse_device::reset_mouse()
{
	m_last_buttons = m_buttons->read() & 0x03;
	m_last_x = m_mouse_x->read();
	m_last_y = m_mouse_y->read();
	m_pending_buttons = m_last_buttons;
	m_pending_x = m_last_x;
	m_pending_y = m_last_y;
	m_report_pending = false;
	set_service_request(false);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(ADB_HLE_MOUSE, adb_slot_card_interface, adb_hle_mouse_device, "adbhlemouse", "ADB HLE Mouse")
