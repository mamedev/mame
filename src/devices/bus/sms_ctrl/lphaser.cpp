// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System "Light Phaser" (light gun) emulation


Release data from the Sega Retro project:

  Year: 1986    Country/region: US    Model code: 3050
  Year: 1987    Country/region: EU    Model code: ?
  Year: 1989    Country/region: BR    Model code: 010470
  Year: 198?    Country/region: KR    Model code: ?

Notes:

  The Light Phaser gun doesn't work with the Sega Mark III.

**********************************************************************/

#include "emu.h"
#include "lphaser.h"

#include "screen.h"

//#define VERBOSE 1
#include "logmacro.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sms_light_phaser_device : public device_t, public device_sms_control_interface
{
public:
	// construction/destruction
	sms_light_phaser_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(position_changed) { sensor_check(0); }

	// device_sms_control_interface implementation
	virtual uint8_t in_r() override { return m_trigger->read(); }

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_ioport m_trigger;
	required_ioport m_x_axis;
	required_ioport m_y_axis;

	emu_timer *m_update_timer;

	int m_sensor_last_state;

	TIMER_CALLBACK_MEMBER(sensor_check);
	int bright_aim_area(int lgun_x, int lgun_y );
	uint16_t screen_hpos_nonscaled(int scaled_hpos);
	uint16_t screen_vpos_nonscaled(int scaled_vpos);
};


INPUT_PORTS_START(sms_light_phaser)
	PORT_START("LPHASER_T")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // TL (trigger)
	PORT_BIT( 0x2f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LPHASER_X")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(15) PORT_CHANGED_MEMBER(DEVICE_SELF, sms_light_phaser_device, position_changed, 0)

	PORT_START("LPHASER_Y")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(15) PORT_CHANGED_MEMBER(DEVICE_SELF, sms_light_phaser_device, position_changed, 0)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor sms_light_phaser_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sms_light_phaser);
}


//-------------------------------------------------
//  sms_light_phaser_device - constructor
//-------------------------------------------------

sms_light_phaser_device::sms_light_phaser_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMS_LIGHT_PHASER, tag, owner, clock),
	device_sms_control_interface(mconfig, *this),
	m_trigger(*this, "LPHASER_T"),
	m_x_axis(*this, "LPHASER_X"),
	m_y_axis(*this, "LPHASER_Y"),
	m_update_timer(nullptr),
	m_sensor_last_state(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_light_phaser_device::device_start()
{
	m_update_timer = timer_alloc(FUNC(sms_light_phaser_device::sensor_check), this);

	m_sensor_last_state = 1;

	save_item(NAME(m_sensor_last_state));
}


void sms_light_phaser_device::device_reset()
{
	// TODO: sensor check now?  The device itself doesn't respond to system reset
}


/*
    Light Phaser (light gun) emulation notes:
    - The sensor is activated based on the brightness of individual pixels being
      drawn by the beam, in circular area where the gun is aiming.
    - Currently, brightness is calculated based only on single pixels.
    - In general, after the trigger is pressed, games draw the next frame using
      a bright pattern, to make sure the sensor will be activated. If the emulation
      skips that frame, the sensor may stay deactivated. Setting frameskip to 0
      is recommended to avoid problems.
    - When the sensor switches from on (0) to off (1), a value is latched for the
      HCount register.
    - When the sensor switches from off to on, a flag is set. The emulation uses
      this flag to signal that the TH line is activated during status reads of the
      input port. After reading, the flag is cleared, or it is cleared when the
      Pause status is read at the end of a frame. This is necessary because the
      "Color & Switch Test" ROM only reads the TH state after VINT occurs.
    - The gun test in "Color & Switch Test" is an example that requires checks
      of the sensor status independent of other events, like trigger press or TH bit
      reads. Another example is the title screen of "Hang-On & Safari Hunt", where
      the game only reads the HCount register in a loop, expecting a latch by the gun.
    - The whole procedure is managed by a periodic timer callback, which is scheduled
      to run in intervals when the beam is within the circular area.
*/
int sms_light_phaser_device::bright_aim_area(int lgun_x, int lgun_y)
{

	constexpr int LGUN_RADIUS = 6;
	constexpr int LGUN_X_INTERVAL = 4;

	// brightness of the lightgray color in the frame drawn by Light Phaser games
	constexpr uint8_t SENSOR_MIN_BRIGHTNESS = 0x7f;

	screen_device &scr(*screen());
	const int r_x_r = LGUN_RADIUS * LGUN_RADIUS;
	const rectangle &visarea = scr.visible_area();
	rectangle aim_area;
	int beam_x = scr.hpos();
	int beam_y = scr.vpos();
	const int beam_x_orig = beam_x;
	const int beam_y_orig = beam_y;
	int result = 1;
	bool new_check_point = false;

	aim_area.min_y = std::max(lgun_y - LGUN_RADIUS, visarea.min_y);
	aim_area.max_y = std::min(lgun_y + LGUN_RADIUS, visarea.max_y);

	while (!new_check_point)
	{
		// If beam's y doesn't point to a line where the aim area is, change it to the first line where the beam enters that area.
		if (beam_y < aim_area.min_y || beam_y > aim_area.max_y)
		{
			beam_y = aim_area.min_y;
		}
		int dy = abs(beam_y - lgun_y);

		// Calculate distance in x of the radius, relative to beam's y distance.  First try some shortcuts.
		double dx_radius = 0;
		if (dy == 0)
		{
			dx_radius = LGUN_RADIUS;
		}
		else if (dy != LGUN_RADIUS)
		{
			// step 1: r^2 = dx^2 + dy^2
			// step 2: dx^2 = r^2 - dy^2
			// step 3: dx = sqrt(r^2 - dy^2)
			dx_radius = ceil(sqrt(double(r_x_r - (dy * dy))));
		}

		aim_area.min_x = std::max(int32_t(lgun_x - dx_radius), visarea.min_x);
		aim_area.max_x = std::min(int32_t(lgun_x + dx_radius), visarea.max_x);

		while (!new_check_point)
		{
			// If beam's x has passed the aim area, change it to the next line and go back to recheck y/x coordinates.
			if (beam_x > aim_area.max_x)
			{
				beam_x = visarea.min_x;
				beam_y++;
				break;
			}

			// If beam's x isn't in the aim area, change it to the next point where the beam enters that area.
			if (beam_x < aim_area.min_x)
			{
				beam_x = aim_area.min_x;
			}

			if (beam_x_orig != beam_x || beam_y_orig != beam_y)
			{
				// adopt the new coordinates to adjust the timer
				new_check_point = true;
				break;
			}

			if (m_sensor_last_state == 0)
			{
				// sensor is already on - keep sensor on until out of the aim area
				result = 0;
			}
			else
			{
				scr.update_now();
				const rgb_t color = scr.pixel(beam_x, beam_y);

				// reference: http://www.w3.org/TR/AERT#color-contrast
				const uint8_t brightness = (color.r() * 0.299) + (color.g() * 0.587) + (color.b() * 0.114);
				LOG("color brightness: %2X for x %d y %d\n", brightness, beam_x, beam_y);

				result = (brightness >= SENSOR_MIN_BRIGHTNESS) ? 0 : 1;
			}

			if (result == 0) // sensor on
			{
				// Set next check for when sensor will be off
				beam_x = aim_area.max_x + 1;

				// adopt the new coordinates to adjust the timer
				new_check_point = true;
			}
			else
			{
				// Next check will happen after the minimum interval
				beam_x += LGUN_X_INTERVAL;
			}
		}
	}

	m_update_timer->adjust(scr.time_until_pos(beam_y, beam_x));
	return result;
}


uint16_t sms_light_phaser_device::screen_hpos_nonscaled(int scaled_hpos)
{
	const rectangle &visarea = screen()->visible_area();
	int offset_x = (scaled_hpos * (visarea.max_x - visarea.min_x)) / 255;
	return visarea.min_x + offset_x;
}


uint16_t sms_light_phaser_device::screen_vpos_nonscaled(int scaled_vpos)
{
	const rectangle &visarea = screen()->visible_area();
	int offset_y = (scaled_vpos * (visarea.max_y - visarea.min_y)) / 255;
	return visarea.min_y + offset_y;
}


TIMER_CALLBACK_MEMBER(sms_light_phaser_device::sensor_check)
{
	if (screen())
	{
		const int x = screen_hpos_nonscaled(m_x_axis->read());
		const int y = screen_vpos_nonscaled(m_y_axis->read());

		int sensor_new_state = bright_aim_area(x, y);
		if (sensor_new_state != m_sensor_last_state)
		{
			th_w(sensor_new_state);
			m_sensor_last_state = sensor_new_state;
		}
	}
}

} // anonymous namespace



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(SMS_LIGHT_PHASER, device_sms_control_interface, sms_light_phaser_device, "sms_light_phaser", "Sega Master System Light Phaser")
