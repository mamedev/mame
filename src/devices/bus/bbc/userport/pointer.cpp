// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    AMX Mouse

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/AMX_Mouse.html

    Marconi RB2 Tracker Ball

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Marconi_MarcusRB2.html
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_Trackerball.html

    The Tracker Ball outputs are fed directly into the user port with the port defined as
    an input. The CB1 and CB2 lines are used in conjunction with PB3 and PB4 for
    sensing pulses and determining the direction of rotation of the ball.

    The connections are:
      CB1 X1
      CB2 Y2
      PB0 Left switch button
      PB1 Middle switch button
      PB2 Right switch button
      PB3 X2
      PB4 Y1

    Quadrature implementation derived from SmallyMouse2,
    see https://github.com/simoninns/SmallyMouse2

**********************************************************************/

#include "emu.h"
#include "pointer.h"


namespace {

class bbc_pointer_device : public device_t, public device_bbc_userport_interface
{
protected:
	bbc_pointer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_bbc_userport_interface(mconfig, *this)
		, m_pointer_x(*this, "POINTER_X")
		, m_pointer_y(*this, "POINTER_Y")
		, m_buttons(*this, "BUTTONS")
	{
	}

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update);

	required_ioport m_pointer_x;
	required_ioport m_pointer_y;
	required_ioport m_buttons;

	// quadrature output
	int m_xdir, m_ydir;

	// internal quadrature state
	int m_x, m_y;
	int m_phase_x, m_phase_y;

	emu_timer *m_pointer_timer;
};


// ======================> bbc_amxmouse_device

class bbc_amxmouse_device : public bbc_pointer_device
{
public:
	bbc_amxmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_pointer_device(mconfig, BBC_AMXMOUSE, tag, owner, clock)
	{
	}

protected:
	uint8_t pb_r() override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> bbc_m512mouse_device

class bbc_m512mouse_device : public bbc_pointer_device
{
public:
	bbc_m512mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_pointer_device(mconfig, BBC_M512MOUSE, tag, owner, clock)
	{
	}

protected:
	uint8_t pb_r() override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> bbc_tracker_device

class bbc_tracker_device : public bbc_pointer_device
{
public:
	bbc_tracker_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_pointer_device(mconfig, BBC_TRACKER, tag, owner, clock)
	{
	}

protected:
	uint8_t pb_r() override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( amxmouse )
	PORT_START("POINTER_X")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100)

	PORT_START("POINTER_Y")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100)

	PORT_START("BUTTONS")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Mouse Left Button (Execute)") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Mouse Middle Button (Move)") PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Mouse Right Button (Cancel)") PORT_CODE(MOUSECODE_BUTTON2)
INPUT_PORTS_END


static INPUT_PORTS_START( m512mouse )
	PORT_START("POINTER_X")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100)

	PORT_START("POINTER_Y")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100)

	PORT_START("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Mouse Left Button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Mouse Right Button") PORT_CODE(MOUSECODE_BUTTON2)
INPUT_PORTS_END


static INPUT_PORTS_START( tracker )
	PORT_START("POINTER_X")
	PORT_BIT(0xff, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(100)

	PORT_START("POINTER_Y")
	PORT_BIT(0xff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(100)

	PORT_START("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Left Button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Middle Button") PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Right Button") PORT_CODE(MOUSECODE_BUTTON2)
INPUT_PORTS_END


ioport_constructor bbc_amxmouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( amxmouse );
}

ioport_constructor bbc_m512mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( m512mouse );
}

ioport_constructor bbc_tracker_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( tracker );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_pointer_device::device_start()
{
	m_pointer_timer = timer_alloc(FUNC(bbc_pointer_device::update), this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_pointer_device::device_reset()
{
	m_xdir = 0;
	m_ydir = 0;
	m_x = 0;
	m_y = 0;
	m_phase_x = 0;
	m_phase_y = 0;

	m_pointer_timer->adjust(attotime::zero, 0, attotime::from_hz(1400));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

TIMER_CALLBACK_MEMBER(bbc_pointer_device::update)
{
	int x = m_pointer_x->read();
	int y = m_pointer_y->read();

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

uint8_t bbc_amxmouse_device::pb_r()
{
	return (m_buttons->read() & 0xe0) | (m_xdir << 0) | (m_ydir << 2) | 0x1a;
}

uint8_t bbc_m512mouse_device::pb_r()
{
	return (m_buttons->read() & 0x07) | (m_xdir << 3) | (m_ydir << 4) | 0xe0;
}

uint8_t bbc_tracker_device::pb_r()
{
	return (m_buttons->read() & 0x07) | (m_xdir << 3) | (m_ydir << 4) | 0xe0;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_AMXMOUSE,  device_bbc_userport_interface, bbc_amxmouse_device,  "bbc_amxmouse",  "AMX Mouse (BBC Micro)")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_M512MOUSE, device_bbc_userport_interface, bbc_m512mouse_device, "bbc_m512mouse", "Acorn Master 512 Mouse")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_TRACKER,   device_bbc_userport_interface, bbc_tracker_device,   "bbc_tracker",   "Marconi RB2 Tracker Ball")
