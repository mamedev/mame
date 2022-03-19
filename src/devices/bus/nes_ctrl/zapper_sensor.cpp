// license:BSD-3-Clause
// copyright-holders:kmg
/**********************************************************************

    HLE of the common photodiode reading routine used in the NES,
    Vs. System, and PlayChoice-10 light guns, and R.O.B.

    These all use a Sharp IR3T07, though the NES zapper is (always?)
    seen with the IR3T07A revision.

**********************************************************************/

#include "emu.h"
#include "zapper_sensor.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_ZAPPER_SENSOR, nes_zapper_sensor_device, "nes_zapper_sensor", "Nintendo Zapper Lightgun Photodiode")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  constructor
//-------------------------------------------------

nes_zapper_sensor_device::nes_zapper_sensor_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NES_ZAPPER_SENSOR, tag, owner, clock)
	, m_screen(*this, finder_base::DUMMY_TAG)
{
}


//-------------------------------------------------
//  detect_light
//-------------------------------------------------

bool nes_zapper_sensor_device::detect_light(int x, int y)
{
	int vpos = m_screen->vpos();
	int hpos = m_screen->hpos();

	// update the screen if necessary
	if (!m_screen->vblank())
		if (vpos > y - radius || (vpos == y - radius && hpos >= x - radius))
			m_screen->update_now();

	int sum = 0;
	int scanned = 0;

	// sum brightness of pixels nearby the gun position
	for (int i = x - radius; i <= x + radius; i++)
		for (int j = y - radius; j <= y + radius; j++)
			// look at pixels within circular sensor
			if ((x - i) * (x - i) + (y - j) * (y - j) <= radius * radius)
			{
				rgb_t pix = m_screen->pixel(i, j);

				// only detect light if gun position is near, and behind, where the PPU is drawing on the CRT, from NesDev wiki:
				// "Zap Ruder test ROM show that the photodiode stays on for about 26 scanlines with pure white, 24 scanlines with light gray, or 19 lines with dark gray."
				if (j <= vpos && j > vpos - sustain && (j != vpos || i <= hpos))
					sum += pix.r() + pix.g() + pix.b();
				scanned++;
			}

	// light detected if average brightness is above threshold
	return sum >= bright * scanned;
}
