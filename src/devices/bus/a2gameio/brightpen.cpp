// license:BSD-3-Clause
// copyright-holders:kmg
/*********************************************************************

    Apple II Softape Bright Pen interface for the Apple ][/][+

*********************************************************************/

#include "emu.h"
#include "bus/a2gameio/brightpen.h"
#include "screen.h"


namespace {

#define BRIGHTPEN_POINT "BRIGHTPEN_POINT"
#define BRIGHTPEN_X     "BRIGHTPEN_X"
#define BRIGHTPEN_Y     "BRIGHTPEN_Y"

// ======================> apple2_brightpen_device

class apple2_brightpen_device : public device_t, public device_a2gameio_interface
{
public:
	// construction/destruction
	apple2_brightpen_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, APPLE2_BRIGHTPEN, tag, owner, clock)
		, device_a2gameio_interface(mconfig, *this)
		, m_brightpen_point(*this, BRIGHTPEN_POINT)
		, m_brightpen_x(*this, BRIGHTPEN_X)
		, m_brightpen_y(*this, BRIGHTPEN_Y)
	{
	}

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_a2gameio_interface implementation
	virtual int sw0_r() override;

private:
	// input ports
	required_ioport m_brightpen_point;
	required_ioport m_brightpen_x;
	required_ioport m_brightpen_y;

	// radius of circle picked up by the bright pen
	static constexpr int PEN_X_RADIUS = 2;
	static constexpr int PEN_Y_RADIUS = 1;
	// brightness threshold
	static constexpr int BRIGHTNESS_THRESHOLD = 0x20;
	// # of CRT scanlines that sustain brightness
	static constexpr int SUSTAIN = 22;
};

//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START( apple2_brightpen )
	PORT_START(BRIGHTPEN_X)
	PORT_BIT( 0x3ff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15) PORT_MINMAX(0, 559) PORT_NAME("Bright Pen X")
	PORT_START(BRIGHTPEN_Y)
	PORT_BIT( 0x3ff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15) PORT_MINMAX(0, 191) PORT_NAME("Bright Pen Y")
	PORT_START(BRIGHTPEN_POINT)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Bright Pen pointed at screen")
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor apple2_brightpen_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(apple2_brightpen);
}

void apple2_brightpen_device::device_start()
{
}

// light detection logic based on zapper_sensor.cpp nes_zapper_sensor_device::detect_light

int apple2_brightpen_device::sw0_r()
{
	if (!BIT(m_brightpen_point->read(), 0)) return 0;

	int pen_x_pos = m_brightpen_x->read();
	int pen_y_pos = m_brightpen_y->read();
	int beam_vpos = m_screen->vpos();
	int beam_hpos = m_screen->hpos();

	// update the screen if the beam position is within the radius of the pen
	if (!machine().side_effects_disabled())
	{
		if (!m_screen->vblank())
		{
			if (beam_vpos > pen_y_pos - PEN_Y_RADIUS || (beam_vpos == pen_y_pos - PEN_Y_RADIUS && beam_hpos >= pen_x_pos - PEN_X_RADIUS))
			{
				m_screen->update_now();
			}
		}
	}

	int brightness_sum = 0;
	int pixels_scanned = 0;

	// sum brightness of pixels nearby the pen position
	for (int i = pen_x_pos - PEN_X_RADIUS; i <= pen_x_pos + PEN_X_RADIUS; i++)
	{
		for (int j = pen_y_pos - PEN_Y_RADIUS; j <= pen_y_pos + PEN_Y_RADIUS; j++)
		{
			// look at pixels within circular sensor
			if ((pen_x_pos - i) * (pen_x_pos - i) + (pen_y_pos - j) * (pen_y_pos - j) <= PEN_X_RADIUS * PEN_Y_RADIUS)
			{
				rgb_t pix = m_screen->pixel(i, j);

				// only detect light if pen position is near, and behind,
				// where the video generator is drawing on the CRT,
				if (j <= beam_vpos && j > beam_vpos - SUSTAIN && (j != beam_vpos || i <= beam_hpos))
				{
					brightness_sum += pix.r() + pix.g() + pix.b();
				}
				pixels_scanned++;
			}
		}
	}

	// light detected if average brightness is above threshold
	return brightness_sum >= BRIGHTNESS_THRESHOLD * pixels_scanned;
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(APPLE2_BRIGHTPEN, device_a2gameio_interface, apple2_brightpen_device, "a2brightpen", "Softape Bright Pen")
