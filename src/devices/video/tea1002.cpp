// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    TEA1002

    PAL colour encoder and video summer

***************************************************************************/

#include "emu.h"
#include "tea1002.h"


namespace {

//**************************************************************************
//  CONSTANTS
//**************************************************************************

const int s_tint = -6; // what is this based on?

const float s_luminance[] =
{
	 0, 22.5, 44, 66.5,  8.5, 31, 52.5, 100, // INV = 0
	75, 52.5, 31,  8.5, 66.5, 44, 22.5, 0    // INV = 1
};

const int s_phase[] =
{
	0, 103, 241, 167, 347,  61, 283, 0, // INV = 0
	0, 283,  61, 347, 167, 241, 103, 0  // INV = 1
};

const int s_amplitude[] =
{
	0, 48, 44, 33, 33, 44, 48, 0, // INV = 0
	0, 24, 22, 17, 17, 22, 24, 0  // INV = 1
};

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TEA1002, tea1002_device, "tea1002", "Mullard TEA1002 PAL colour encoder")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tea1002_device - constructor
//-------------------------------------------------

tea1002_device::tea1002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TEA1002, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tea1002_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

// this could be done in device_start() and cached, but it's only
// accessed once at PALETTE_INIT anyway
rgb_t tea1002_device::color(int index)
{
	// calculate yuv
	double y = s_luminance[index] / 100;
	double u = cos((s_phase[index] + s_tint) * M_PI / 180) * s_amplitude[index] / 100;
	double v = sin((s_phase[index] + s_tint) * M_PI / 180) * s_amplitude[index] / 100;

	// and convert to rgb
	double r = y + v * 1.14;
	double g = y - u * 0.395 - v * 0.581;
	double b = y + u * 2.032;

	return rgb_t(rgb_t::clamp(r * 255), rgb_t::clamp(g * 255), rgb_t::clamp(b * 255));
}
