// license:BSD-3-Clause
// copyright-holders:David Haywood

/* 
   HSV / HSL palette decode for Radica TV games (and XaviX TV games)

   this is based on emupal.h, style etc. (including use of statics) copied where possible.  If considered generic enough could probably be merged?
   note colours don't appear to be 100% accurate right now, might not be a linear mapping, might need +/- a fixed value fur hue?
*/

#include "emu.h"
#include "rad_hsvpal.h"

DEFINE_DEVICE_TYPE(RADICA_HSVPAL, radica_hsvpal_device, "radica_hsvpal", "Radica HSV Palette")

radica_hsvpal_device::radica_hsvpal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: palette_device(mconfig, tag, owner, clock)
{
}

double radica_hsvpal_device::hue2rgb(double p, double q, double t)
{
	if (t < 0) t += 1;
	if (t > 1) t -= 1;
	if (t < 1 / 6.0f) return p + (q - p) * 6 * t;
	if (t < 1 / 2.0f) return q;
	if (t < 2 / 3.0f) return p + (q - p) * (2 / 3.0f - t) * 6;
	return p;
}
