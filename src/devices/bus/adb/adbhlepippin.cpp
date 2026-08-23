// license:BSD-3-Clause
// copyright-holders:R. Belmont

// HLE for Pippin "AppleJack" controller
// Protocol reference: https://github.com/lampmerchant/tashnotes/blob/main/macintosh/adb/protocols/atmark_pippin_controller.md

#include "emu.h"
#include "adbhlepippin.h"

#include "adbhle.h"

namespace
{

static constexpr u16 BUTTON_LEFT_SHOULDER  = 0x0001;
static constexpr u16 BUTTON_RIGHT_SHOULDER = 0x0002;
static constexpr u16 BUTTON_GREEN          = 0x0004;
static constexpr u16 BUTTON_RED            = 0x0008;
static constexpr u16 BUTTON_DPAD_DOWN      = 0x0010;
static constexpr u16 BUTTON_DPAD_RIGHT     = 0x0020;
static constexpr u16 BUTTON_DPAD_LEFT      = 0x0040;
static constexpr u16 BUTTON_DPAD_UP        = 0x0080;
static constexpr u16 BUTTON_YELLOW         = 0x0100;
static constexpr u16 BUTTON_BLUE           = 0x0200;
static constexpr u16 BUTTON_DIAMOND        = 0x0400;
static constexpr u16 BUTTON_CIRCLE         = 0x0800;
static constexpr u16 BUTTON_SQUARE         = 0x1000;

class adb_hle_pippin_controller_device : public adb_hle_device
{
public:
	adb_hle_pippin_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual unsigned adb_talk(u8 reg, std::span<u8> data) override;
	virtual void adb_talk_complete(u8 reg, bool success) override;
	virtual void adb_poll() override;
	virtual void adb_bus_reset() override;
	virtual bool adb_set_handler(u8 handler) override;

private:
	void reset_controller();
	bool input_changed() const;
	static u8 released(u16 buttons, u16 mask);

	required_ioport m_buttons;
	required_ioport m_trackball_x;
	required_ioport m_trackball_y;

	u16 m_last_buttons;
	u8 m_last_x;
	u8 m_last_y;
	u16 m_pending_buttons;
	u8 m_pending_x;
	u8 m_pending_y;
	bool m_report_pending;
	bool m_force_report;
};

static INPUT_PORTS_START(adb_hle_pippin_controller)
	// Button mappings attempt to match PlayStation, but diamond/circle/square
	// are kind of a challenge in that respect (they're on the *front* of the controller,
	// opposite the shoulder buttons, which nobody else ever did to my knowledge)
	PORT_START("BUTTONS")
	PORT_BIT(BUTTON_LEFT_SHOULDER, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Left Shoulder")
	PORT_BIT(BUTTON_RIGHT_SHOULDER, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Right Shoulder")
	PORT_BIT(BUTTON_GREEN, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Green / 1")
	PORT_BIT(BUTTON_RED, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Red / 2")
	PORT_BIT(BUTTON_DPAD_DOWN, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_NAME("Directional Pad Down") PORT_8WAY
	PORT_BIT(BUTTON_DPAD_RIGHT, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_NAME("Directional Pad Right") PORT_8WAY
	PORT_BIT(BUTTON_DPAD_LEFT, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_NAME("Directional Pad Left") PORT_8WAY
	PORT_BIT(BUTTON_DPAD_UP, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_NAME("Directional Pad Up") PORT_8WAY
	PORT_BIT(BUTTON_YELLOW, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Yellow / 4")
	PORT_BIT(BUTTON_BLUE, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Blue / 3")
	PORT_BIT(BUTTON_DIAMOND, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("Diamond")
	PORT_BIT(BUTTON_CIRCLE, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Circle")
	PORT_BIT(BUTTON_SQUARE, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_NAME("Square")
	PORT_BIT(0xe000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("TRACKBALLX")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0)

	PORT_START("TRACKBALLY")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0)
INPUT_PORTS_END

adb_hle_pippin_controller_device::adb_hle_pippin_controller_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		u32 clock) :
	adb_hle_device(mconfig, ADB_HLE_PIPPIN_CONTROLLER, tag, owner, clock, 3, 1),
	m_buttons(*this, "BUTTONS"),
	m_trackball_x(*this, "TRACKBALLX"),
	m_trackball_y(*this, "TRACKBALLY"),
	m_last_buttons(0),
	m_last_x(0),
	m_last_y(0),
	m_pending_buttons(0),
	m_pending_x(0),
	m_pending_y(0),
	m_report_pending(false),
	m_force_report(false)
{
}

ioport_constructor adb_hle_pippin_controller_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(adb_hle_pippin_controller);
}

void adb_hle_pippin_controller_device::device_start()
{
	adb_hle_device::device_start();

	save_item(NAME(m_last_buttons));
	save_item(NAME(m_last_x));
	save_item(NAME(m_last_y));
	save_item(NAME(m_pending_buttons));
	save_item(NAME(m_pending_x));
	save_item(NAME(m_pending_y));
	save_item(NAME(m_report_pending));
	save_item(NAME(m_force_report));
}

void adb_hle_pippin_controller_device::device_reset()
{
	adb_hle_device::device_reset();
	reset_controller();
}

unsigned adb_hle_pippin_controller_device::adb_talk(u8 reg, std::span<u8> data)
{
	if ((reg != 0) || (!m_force_report && !input_changed()))
	{
		return 0;
	}

	u16 const buttons = m_buttons->read() & 0x1fff;
	u8 const x = m_trackball_x->read();
	u8 const y = m_trackball_y->read();
	int const delta_x = std::clamp(int(s8(x - m_last_x)), -64, 63);
	int const delta_y = std::clamp(int(s8(y - m_last_y)), -64, 63);

	data[0] = (released(buttons, BUTTON_LEFT_SHOULDER) << 7) | (u8(delta_y) & 0x7f);
	data[1] = (released(buttons, BUTTON_RIGHT_SHOULDER) << 7) | (u8(delta_x) & 0x7f);
	if (adb_handler() == 0x46)
	{
		data[2] =
			(released(buttons, BUTTON_GREEN) << 7) |
			(released(buttons, BUTTON_RED) << 6) |
			(released(buttons, BUTTON_DPAD_DOWN) << 5) |
			(released(buttons, BUTTON_DPAD_RIGHT) << 4) |
			(released(buttons, BUTTON_DPAD_LEFT) << 3) |
			(released(buttons, BUTTON_DPAD_UP) << 2) |
			(released(buttons, BUTTON_YELLOW) << 1) |
			released(buttons, BUTTON_BLUE);
		data[3] =
			0xf8 |
			(released(buttons, BUTTON_DIAMOND) << 2) |
			(released(buttons, BUTTON_CIRCLE) << 1) |
			released(buttons, BUTTON_SQUARE);
	}

	m_pending_buttons = buttons;
	m_pending_x = m_last_x + delta_x;
	m_pending_y = m_last_y + delta_y;
	m_report_pending = true;
	return (adb_handler() == 0x46) ? 4 : 2;
}

void adb_hle_pippin_controller_device::adb_talk_complete(u8 reg, bool success)
{
	if ((reg == 0) && m_report_pending && success)
	{
		m_last_buttons = m_pending_buttons;
		m_last_x = m_pending_x;
		m_last_y = m_pending_y;
		m_force_report = false;
	}
	m_report_pending = false;
	adb_poll();
}

void adb_hle_pippin_controller_device::adb_poll()
{
	set_service_request(m_force_report || input_changed());
}

void adb_hle_pippin_controller_device::adb_bus_reset()
{
	reset_controller();
}

bool adb_hle_pippin_controller_device::adb_set_handler(u8 handler)
{
	if ((handler != 0x01) && (handler != 0x46))
	{
		return false;
	}

	if (handler != adb_handler())
	{
		m_force_report = true;
		set_service_request(true);
	}
	return true;
}

void adb_hle_pippin_controller_device::reset_controller()
{
	m_last_buttons = m_buttons->read() & 0x1fff;
	m_last_x = m_trackball_x->read();
	m_last_y = m_trackball_y->read();
	m_pending_buttons = m_last_buttons;
	m_pending_x = m_last_x;
	m_pending_y = m_last_y;
	m_report_pending = false;
	m_force_report = false;
	set_service_request(false);
}

bool adb_hle_pippin_controller_device::input_changed() const
{
	return
		((m_buttons->read() & 0x1fff) != m_last_buttons) ||
		(m_trackball_x->read() != m_last_x) ||
		(m_trackball_y->read() != m_last_y);
}

u8 adb_hle_pippin_controller_device::released(u16 buttons, u16 mask)
{
	return (buttons & mask) ? 0 : 1;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(
		ADB_HLE_PIPPIN_CONTROLLER,
		adb_slot_card_interface,
		adb_hle_pippin_controller_device,
		"adbhlepippin",
		"ADB HLE Bandai Pippin Controller")
