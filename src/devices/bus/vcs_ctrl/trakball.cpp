// license:BSD-3-Clause
/**********************************************************************

    Atari CX22/CX80 Trak-Ball

Note: this module only works in trackball mode and not in joystick emulation mode

Reference: Atari, CX22 Trakball Field Service Manual, Rev. 01 (FD100660), November 1983

**********************************************************************/

#include "emu.h"
#include "trakball.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define TRAKBALL_BUTTON_TAG "trackball_buttons"
#define TRAKBALL_XAXIS_TAG  "trackball_x"
#define TRAKBALL_YAXIS_TAG  "trackball_y"

#define TRAKBALL_POS_UNINIT 0xffffffff /* default out-of-range position */

//**************************************************************************
//  DEVICE TYPE DEFINITION
//**************************************************************************

DEFINE_DEVICE_TYPE(ATARI_TRAKBALL, atari_trakball_device, "atari_trakball", "Atari CX22/CX80 Trak-Ball")


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START(atari_trakball)
	PORT_START(TRAKBALL_BUTTON_TAG) /* Trak-ball - button */
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_WRITE_LINE_MEMBER(atari_trakball_device, trigger_w)
	PORT_BIT( 0xd0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START(TRAKBALL_XAXIS_TAG) /* Trak-ball - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(80) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, atari_trakball_device, trakball_moved, 0)

	PORT_START(TRAKBALL_YAXIS_TAG) /* Trak-ball - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(80) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, atari_trakball_device, trakball_moved, 1)
INPUT_PORTS_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  atari_trakball_device - constructor
//-------------------------------------------------

atari_trakball_device::atari_trakball_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ATARI_TRAKBALL, tag, owner, clock)
	, device_vcs_control_port_interface(mconfig, *this)
	, m_trakballb(*this, TRAKBALL_BUTTON_TAG)
	, m_trakballxy(*this, { TRAKBALL_XAXIS_TAG, TRAKBALL_YAXIS_TAG })
	, m_last_pos{ TRAKBALL_POS_UNINIT, TRAKBALL_POS_UNINIT }
	, m_last_direction{ 0, 0 }
{
}

//-------------------------------------------------
//  device_input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor atari_trakball_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(atari_trakball);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atari_trakball_device::device_start()
{
}

#define QUADRATURE_ANGLE_RESOLUTION 0x02

//-----------------------------------------------------------------
//  trakbal_pos_and_dir_upd - update tracked position and direction
//-----------------------------------------------------------------

void atari_trakball_device::trakball_pos_and_dir_upd(int axis)
{
	int diff_pos = 0;
	int cur_pos = 0;

	cur_pos = m_trakballxy[axis]->read();
	if (m_last_pos[axis] == TRAKBALL_POS_UNINIT) {
		if (!machine().side_effects_disabled()) {
			m_last_pos[axis] = cur_pos;
		}
	}
	diff_pos = cur_pos - m_last_pos[axis];
	// wrap-around the position
	if (diff_pos > 0x7f) {
		diff_pos -= 0x100;
	} else if (diff_pos < -0x80) {
		diff_pos += 0x100;
	}
	if (!machine().side_effects_disabled()) {
		m_last_pos[axis] = cur_pos;
		if (diff_pos) {
			m_last_direction[axis] = diff_pos > 0;
		}
	}
}

//---------------------------------------------------------
//  trakball_moved - called when moved outside of polling
//---------------------------------------------------------

INPUT_CHANGED_MEMBER( atari_trakball_device::trakball_moved )
{
	const int axis(param);

	trakball_pos_and_dir_upd(axis);
}

//-------------------------------------------------
//  vcs_joy_r - read digital inputs
//-------------------------------------------------

u8 atari_trakball_device::vcs_joy_r()
{
	u8 vcs_joy_return = 0;

	for (int axis = 0; axis < 2; axis++) {
		trakball_pos_and_dir_upd(axis);
	}

	vcs_joy_return =
		m_trakballb->read() |
		(m_last_direction[0] ? 0x01 : 0x00) |
		((m_last_pos[0] & QUADRATURE_ANGLE_RESOLUTION) ? 0x02 : 0x00) |
		(m_last_direction[1] ? 0x04 : 0x00) |
		((m_last_pos[1] & QUADRATURE_ANGLE_RESOLUTION) ? 0x08 : 0x00);

	return vcs_joy_return;
}
