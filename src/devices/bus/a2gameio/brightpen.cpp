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
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

	// device_a2gameio_interface implementation
	virtual int sw0_r() override;

private:
	// input ports
	required_ioport m_brightpen_point;
	required_ioport m_brightpen_x;
	required_ioport m_brightpen_y;

	// radius of circle picked up by the bright pen
	static constexpr int x_radius = 6;
	static constexpr int y_radius = 3;
	// brightness threshold
	static constexpr int bright = 0x50;
	// # of CRT scanlines that sustain brightness
	static constexpr int sustain = 22;
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

ioport_constructor apple2_brightpen_device::device_input_ports() const {
	return INPUT_PORTS_NAME(apple2_brightpen);
};

void apple2_brightpen_device::device_start()
{
}

// light detection logic based on zapper_sensor.cpp nes_zapper_sensor_device::detect_light

int apple2_brightpen_device::sw0_r() {
        int x = m_brightpen_x->read();
        int y = m_brightpen_y->read();
	int vpos = m_screen->vpos();
	int hpos = m_screen->hpos();

	// update the screen if necessary
	if (!machine().side_effects_disabled()) {
		if (!m_screen->vblank()) {
			if (vpos > y - y_radius || (vpos == y - y_radius && hpos >= x - x_radius)) {
				m_screen->update_now();
			}
		}
	}

	int sum = 0;
	int scanned = 0;

	// sum brightness of pixels nearby the gun position
	for (int i = x - x_radius; i <= x + x_radius; i++) {
		for (int j = y - y_radius; j <= y + y_radius; j++) {
			// look at pixels within circular sensor
			if ((x - i) * (x - i) + (y - j) * (y - j) <= x_radius * y_radius) {
				rgb_t pix = m_screen->pixel(i, j);

				// only detect light if gun position is near, and behind,
				// where the PPU is drawing on the CRT,
				if (j <= vpos && j > vpos - sustain && (j != vpos || i <= hpos))
					sum += pix.r() + pix.g() + pix.b();
				scanned++;
			}
		}
	}

	// light detected if average brightness is above threshold
	return (sum >= bright * scanned) && BIT(m_brightpen_point->read(), 0);
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(APPLE2_BRIGHTPEN, device_a2gameio_interface, apple2_brightpen_device, "a2brightpen", "Softape Bright Pen")
