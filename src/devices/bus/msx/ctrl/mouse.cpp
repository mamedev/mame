// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    MSX Mouse emulation

**********************************************************************/

#include "emu.h"
#include "mouse.h"


namespace {

INPUT_PORTS_START(msx_mouse)
	PORT_START("BUTTONS")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2)
	PORT_BIT(0xcf, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("MOUSE_X")
	PORT_BIT(0xffff, 0, IPT_MOUSE_X) PORT_SENSITIVITY(50)

	PORT_START("MOUSE_Y")
	PORT_BIT(0xffff, 0, IPT_MOUSE_Y) PORT_SENSITIVITY(50)
INPUT_PORTS_END


class msx_mouse_device : public device_t, public device_msx_general_purpose_port_interface
{
public:
	msx_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read() override;
	virtual void pin_8_w(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(msx_mouse); }

private:
	required_ioport m_buttons;
	required_ioport m_port_mouse_x;
	required_ioport m_port_mouse_y;
	u16 m_data;
	u8 m_stat;
	u8 m_old_pin8;
	s16 m_mouse_x;
	s16 m_mouse_y;
	attotime m_last_pin8_change;
	attotime m_timeout;
};

msx_mouse_device::msx_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_MOUSE, tag, owner, clock)
	, device_msx_general_purpose_port_interface(mconfig, *this)
	, m_buttons(*this, "BUTTONS")
	, m_port_mouse_x(*this, "MOUSE_X")
	, m_port_mouse_y(*this, "MOUSE_Y")
{
}

void msx_mouse_device::device_start()
{
	save_item(NAME(m_data));
	save_item(NAME(m_stat));
	save_item(NAME(m_old_pin8));
	save_item(NAME(m_last_pin8_change));
	save_item(NAME(m_mouse_x));
	save_item(NAME(m_mouse_y));
	m_timeout = attotime::from_msec(3);
}

void msx_mouse_device::device_reset()
{
	m_data = 0;
	m_stat = 3;
	m_old_pin8 = 0;
	m_last_pin8_change = attotime::zero;
	m_mouse_x = 0;
	m_mouse_y = 0;
}

u8 msx_mouse_device::read()
{
	return (m_buttons->read() & 0xf0) | ((m_data >> (4 * (3 - m_stat))) & 0x0f);
}

void msx_mouse_device::pin_8_w(int state)
{
	if (m_old_pin8 != state)
	{
		attotime now = machine().scheduler().time();
		if (now - m_last_pin8_change > m_timeout)
		{
			// force restart
			m_stat = 3;
		}
		m_last_pin8_change = now;
		m_stat = (m_stat + 1) & 0x03;

		if (m_stat == 0)
		{
			s16 mouse_x = m_port_mouse_x->read();
			s16 mouse_y = m_port_mouse_y->read();

			m_data = (u8(m_mouse_x - mouse_x) << 8) | u8(m_mouse_y - mouse_y);
			m_mouse_x = mouse_x;
			m_mouse_y = mouse_y;
		}
	}
	m_old_pin8 = state;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MSX_MOUSE, device_msx_general_purpose_port_interface, msx_mouse_device, "msx_mouse", "MSX Mouse")
