// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Master Compact Mouse

    The New Advanced User Guide (page 371) states the Compact PB assignments are:
      PB4 Joystick RIGHT / Mouse Y-axis
      PB3 Joystick UP    / Mouse X-axis
      PB2 Joystick DOWN  / Mouse right button
      PB1 Joystick LEFT  / Mouse middle button
      PB0 Joystick FIRE  / Mouse left button

**********************************************************************/

#include "emu.h"
#include "mouse.h"


namespace {

class bbcmc_mouse_device : public device_t, public device_bbc_joyport_interface
{
public:
	bbcmc_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBCMC_MOUSE, tag, owner, clock)
		, device_bbc_joyport_interface(mconfig, *this)
		, m_mouse_x(*this, "mouse_x")
		, m_mouse_y(*this, "mouse_y")
		, m_buttons(*this, "buttons")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t pb_r() override
	{
		return (m_buttons->read() & 0x07) | (m_xdir << 3) | (m_ydir << 4) | 0xe0;
	}

	TIMER_CALLBACK_MEMBER(update);

private:
	required_ioport m_mouse_x;
	required_ioport m_mouse_y;
	required_ioport m_buttons;

	// quadrature output
	int m_xdir, m_ydir;

	// internal quadrature state
	int m_x, m_y;
	int m_phase_x, m_phase_y;

	emu_timer *m_mouse_timer;
};


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( mouse )
	PORT_START("mouse_x")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100)

	PORT_START("mouse_y")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100)

	PORT_START("buttons")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Left mouse button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Middle mouse button") PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Right mouse button") PORT_CODE(MOUSECODE_BUTTON2)
INPUT_PORTS_END


ioport_constructor bbcmc_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mouse );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbcmc_mouse_device::device_start()
{
	m_mouse_timer = timer_alloc(FUNC(bbcmc_mouse_device::update), this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbcmc_mouse_device::device_reset()
{
	m_xdir = 0;
	m_ydir = 0;
	m_x = 0;
	m_y = 0;
	m_phase_x = 0;
	m_phase_y = 0;

	m_mouse_timer->adjust(attotime::zero, 0, attotime::from_hz(1400));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

TIMER_CALLBACK_MEMBER(bbcmc_mouse_device::update)
{
	int x = m_mouse_x->read();
	int y = m_mouse_y->read();

	int dx = x - m_x;
	int dy = y - m_y;

	// Process X output
	if (dx)
	{
		// Set the output pins according to the current phase
		switch (m_phase_x)
		{
		case 0:
			m_slot->cb1_w(1); // Set X1 to 1
			break;
		case 1:
			m_xdir = 1;       // Set X2 to 1
			break;
		case 2:
			m_slot->cb1_w(0); // Set X1 to 0
			break;
		case 3:
			m_xdir = 0;       // Set X2 to 0
			break;
		}

		// Change phase
		if (dx > 0)
			m_phase_x++;
		else
			m_phase_x--;

		// Range check the phase
		m_phase_x &= 3;
	}

	// Process Y output
	if (dy)
	{
		// Set the output pins according to the current phase
		switch (m_phase_y)
		{
		case 3:
			m_slot->cb2_w(0); // Set Y1 to 0
			break;
		case 2:
			m_ydir = 0;       // Set Y2 to 0
			break;
		case 1:
			m_slot->cb2_w(1); // Set Y1 to 1
			break;
		case 0:
			m_ydir = 1;       // Set Y2 to 1
			break;
		}

		// Change phase
		if (dy > 0)
			m_phase_y++;
		else
			m_phase_y--;

		// Range check the phase
		m_phase_y &= 3;
	}

	m_x = x;
	m_y = y;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBCMC_MOUSE, device_bbc_joyport_interface, bbcmc_mouse_device, "bbcmc_mouse", "BBC Master Compact Mouse")
