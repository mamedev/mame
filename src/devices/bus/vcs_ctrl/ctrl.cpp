// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System controller port emulation

**********************************************************************/

#include "emu.h"
#include "ctrl.h"


namespace {

// radius (in screen pixels) of the circle sampled around the aim point - a
// real photodiode isn't focused down to a single pixel
constexpr int SENSOR_RADIUS = 5;

// brightness threshold
constexpr int SENSOR_BRIGHTNESS_THRESHOLD = 0xc0;

// number of CRT scanlines the phosphor stays lit after the beam passes
constexpr int SENSOR_SUSTAIN_LINES = 22;

} // anonymous namespace



//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

DEFINE_DEVICE_TYPE(VCS_CONTROL_PORT, vcs_control_port_device, "vcs_control_port", "Atari VCS controller port")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vcs_control_port_interface - constructor
//-------------------------------------------------

device_vcs_control_port_interface::device_vcs_control_port_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "vcsctrl")
{
	m_port = dynamic_cast<vcs_control_port_device *>(device.owner());
}


//-------------------------------------------------
//  has_lightpen_timing -
//-------------------------------------------------

bool device_vcs_control_port_interface::has_lightpen_timing() const
{
	return !m_port->m_lightpen_time_cb.isnull() || m_port->m_screen.found();
}


//-------------------------------------------------
//  time_until_lightpen_pos -
//-------------------------------------------------

attotime device_vcs_control_port_interface::time_until_lightpen_pos(int x255, int y255) const
{
	if (!m_port->m_lightpen_time_cb.isnull())
	{
		// a driver-supplied, chip-native raster timing is available and preferred:
		// some video chip cores (e.g. mos6566_device) don't keep their internal
		// raster counters phase-locked to the generic screen_device's hpos()/vpos(),
		// so estimating from the screen below can be wildly wrong for those
		return m_port->m_lightpen_time_cb(x255, y255);
	}

	const rectangle &visarea = m_port->m_screen->visible_area();

	int const x = visarea.left() + x255 * visarea.width() / 256;
	int const y = visarea.top() + y255 * visarea.height() / 256;

	return m_port->m_screen->time_until_pos(y, x);
}


//-------------------------------------------------
//  light_detected -
//-------------------------------------------------

bool device_vcs_control_port_interface::light_detected(int x255, int y255) const
{
	if (!m_port->m_screen.found())
		return false;

	screen_device &screen = *m_port->m_screen;
	const rectangle &visarea = screen.visible_area();
	int const x = visarea.left() + x255 * visarea.width() / 256;
	int const y = visarea.top() + y255 * visarea.height() / 256;

	int const vpos = screen.vpos();
	int const hpos = screen.hpos();

	// update the screen if necessary
	if (!screen.vblank())
		if (vpos > y + SENSOR_RADIUS || (vpos == y + SENSOR_RADIUS && hpos >= x - SENSOR_RADIUS))
			screen.update_now();

	int sum = 0;
	int scanned = 0;

	// sum brightness of pixels nearby the aim position
	for (int i = x - SENSOR_RADIUS; i <= x + SENSOR_RADIUS; i++)
		for (int j = y - SENSOR_RADIUS; j <= y + SENSOR_RADIUS; j++)
			// look at pixels within circular sensor
			if ((x - i) * (x - i) + (y - j) * (y - j) <= SENSOR_RADIUS * SENSOR_RADIUS)
			{
				rgb_t pix = screen.pixel(i, j);

				// only detect light if the aim position is near, and behind, where the beam is drawing on the CRT
				if (j <= vpos && j > vpos - SENSOR_SUSTAIN_LINES && (j != vpos || i <= hpos))
					sum += pix.r() + pix.g() + pix.b();
				scanned++;
			}

	// light detected if average brightness is above threshold
	return scanned != 0 && sum >= SENSOR_BRIGHTNESS_THRESHOLD * scanned;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vcs_control_port_device - constructor
//-------------------------------------------------

vcs_control_port_device::vcs_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VCS_CONTROL_PORT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_screen(*this, finder_base::DUMMY_TAG),
	m_lightpen_time_cb(*this),
	m_device(nullptr),
	m_write_trigger(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vcs_control_port_device::device_start()
{
	m_lightpen_time_cb.resolve();

	m_device = dynamic_cast<device_vcs_control_port_interface *>(get_card_device());
}


//-------------------------------------------------
//  SLOT_INTERFACE( vcs_control_port_devices )
//-------------------------------------------------

#include "c1350.h"
#include "cx85.h"
#include "joybooster.h"
#include "joystick.h"
#include "keypad.h"
#include "lightgun.h"
#include "lightpen.h"
#include "paddles.h"
#include "trakball.h"
#include "wheel.h"

void vcs_control_port_devices(device_slot_interface &device)
{
	device.option_add("joy", VCS_JOYSTICK);
	device.option_add("pad", VCS_PADDLES);
	device.option_add("lpu", VCS_LIGHTPEN_UP);
	device.option_add("lpl", VCS_LIGHTPEN_LEFT);
	device.option_add("lpr", VCS_LIGHTPEN_RIGHT);
	device.option_add("magnum", MAGNUM_LIGHT_PHASER);
	device.option_add("stacklr", STACK_LIGHT_RIFLE);
	device.option_add("gunstick", GUN_STICK);
	device.option_add("inkwell184c", INKWELL_184C);
	device.option_add("joybstr", VCS_JOYSTICK_BOOSTER);
	device.option_add("wheel", VCS_WHEEL);
	device.option_add("keypad", VCS_KEYPAD);
	device.option_add("cx85", ATARI_CX85);
	device.option_add("trakball", ATARI_TRAKBALL);
	device.option_add("c1350", C1350);
	device.option_add("c1351", C1351);
}

void a800_control_port_devices(device_slot_interface &device)
{
	vcs_control_port_devices(device);
	device.set_option_machine_config("pad", &vcs_paddles_device::reverse_players);
}
