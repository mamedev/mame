// license:BSD-3-Clause
/*********************************************************************

    a2wicotrackball.cpp

    Implemention of the Wico Trackball

    Wico Trackball Interface PCB
    Wico 1983

    This is a trackball interface for the Apple II

    For API information, see:
      Track Balls, Bill Morgan, Apple Assembly Line, Vol. 3, Iss. 9, June 1983

*********************************************************************/

#include "emu.h"
#include "a2wico_trackball.h"

namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_wicotrackball_device:
		public device_t,
		public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_wicotrackball_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	a2bus_wicotrackball_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

private:
	uint8_t read_position(int axis);

	required_ioport m_wicotrackballb;
	required_ioport_array<2> m_wicotrackballxy;

	bool m_speed[2];
	uint8_t m_buttons;
	bool m_wraparound;
	uint8_t m_axis[2];
	uint32_t m_last_pos[2];
};

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define WICOTRACKBALL_BUTTONS_TAG   "a2wicotrackball_buttons"
#define WICOTRACKBALL_XAXIS_TAG     "a2wicotrackball_x"
#define WICOTRACKBALL_YAXIS_TAG     "a2wicotrackball_y"

#define WICOTRACKBALL_POS_UNINIT    0xffffffff /* default out-of-range position */

static INPUT_PORTS_START( wicotrackball )
	PORT_START(WICOTRACKBALL_BUTTONS_TAG) /* Trackball - buttons */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Trackball Button 1") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Trackball Button 2") PORT_CODE(MOUSECODE_BUTTON2)

	PORT_START(WICOTRACKBALL_XAXIS_TAG) /* Trackball - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(40) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START(WICOTRACKBALL_YAXIS_TAG) /* Trackball - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(40) PORT_KEYDELTA(0) PORT_PLAYER(1)
INPUT_PORTS_END


/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    input_ports - device-specific input ports
-------------------------------------------------*/

ioport_constructor a2bus_wicotrackball_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(wicotrackball);
}

/***************************************************************************
    LIVE DEVICE
***************************************************************************/

a2bus_wicotrackball_device::a2bus_wicotrackball_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_wicotrackballb(*this, WICOTRACKBALL_BUTTONS_TAG), m_wicotrackballxy(*this, { WICOTRACKBALL_XAXIS_TAG, WICOTRACKBALL_YAXIS_TAG }),
	m_speed{ false, false }, m_buttons{ 0 }, m_wraparound{false}, m_axis{ 0, 0 },
	m_last_pos{ WICOTRACKBALL_POS_UNINIT, WICOTRACKBALL_POS_UNINIT }
{
}

a2bus_wicotrackball_device::a2bus_wicotrackball_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_wicotrackball_device(mconfig, A2BUS_WICOTRACKBALL, tag, owner, clock)
{
}

/*-------------------------------------------------
    device_start - device-specific startup
-------------------------------------------------*/

void a2bus_wicotrackball_device::device_start()
{
	// register save state variables
	save_item(NAME(m_speed));
	save_item(NAME(m_wraparound));
	save_item(NAME(m_buttons));
	save_item(NAME(m_axis));
	save_item(NAME(m_last_pos));
}

void a2bus_wicotrackball_device::device_reset()
{
	m_speed[0] = m_speed[1] = false;
	m_wraparound = false;
	m_buttons = 0;
	m_axis[0] = m_axis[1] = 0;
}

uint8_t a2bus_wicotrackball_device::read_position(int axis)
{
	int const speed_scale = 1 << (m_speed[0] + m_speed[1] * 2);
	int const cur_pos = m_wicotrackballxy[axis]->read();

	uint8_t result = 0;
	if (m_last_pos[axis] != WICOTRACKBALL_POS_UNINIT) {
		int diff_pos = cur_pos - m_last_pos[axis];

		// wrap-around the positoin
		if (diff_pos > 0x7f) {
			diff_pos -= 0x100;
		} else if (diff_pos < -0x80) {
			diff_pos += 0x100;
		}

		int const updated_axis = int(unsigned(m_axis[axis])) + diff_pos / speed_scale;;
		if (m_wraparound) {
			result = unsigned(updated_axis);
		} else {
			result = unsigned(std::clamp<int>(updated_axis, 0, 0xff));
		}
		if (!machine().side_effects_disabled()) {
			m_axis[axis] = result;
		}
	}
	if (!machine().side_effects_disabled()) {
		m_last_pos[axis] = cur_pos;
	}

	return result;
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_wicotrackball_device::read_c0nx(uint8_t offset)
{
	uint8_t data = 0;

	switch (offset) {
		case 0x0: /* read X-position */
			data = read_position(0);
			break;
		case 0x1: /* read Y-position */
			data = read_position(1);
			break;
		case 0x2: /* set Bounded/Wraparound Soft Switch to Bounded */
			if (!machine().side_effects_disabled()) {
				m_wraparound = false;
			}
			break;
		case 0x3: /* set Bounded/Wraparound Soft Switch to Wraparound */
			if (!machine().side_effects_disabled()) {
				m_wraparound = true;
			}
			break;
		case 0x6: /* read buttons */
			data = m_buttons = m_wicotrackballb->read();
			break;
	}
	return data;
}

/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_wicotrackball_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset) {
		case 0x0: /* set X-position */
			m_last_pos[0] = m_wicotrackballxy[0]->read();
			m_axis[0] = data;
			break;
		case 0x1: /* set Y-position */
			m_last_pos[1] = m_wicotrackballxy[1]->read();
			m_axis[1] = data;
			break;
		case 0x2: /* set Bounded/Wraparound Soft Switch to Bounded */
			m_wraparound = false;
			break;
		case 0x3: /* set Bounded/Wraparound Soft Switch to Wraparound */
			m_wraparound = true;
			break;
		case 0x4: /* set Speed 1/2 Soft Switch to Speed 1 */
			m_speed[0] = false;
			break;
		case 0x5: /* set Speed 1/2 Soft Switch to Speed 2 */
			m_speed[0] = true;
			break;
		case 0x6: /* set Speed 3/4 Soft Switch to Speed 3 */
			m_speed[1] = false;
			break;
		case 0x7: /* set Speed 3/4 Soft Switch to Speed 4 */
			m_speed[1] = true;
			break;
	}
}

} // anonymous namespace


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_WICOTRACKBALL, device_a2bus_card_interface, a2bus_wicotrackball_device, "a2wicotrackball", "Apple II Wico Trackball Card")
