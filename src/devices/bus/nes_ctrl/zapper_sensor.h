// license:BSD-3-Clause
// copyright-holders:kmg
/**********************************************************************

    HLE of the common photodiode reading routine used in the NES,
    Vs. System, and Playchoice-10 light guns, and R.O.B.

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_ZAPPER_SENSOR
#define MAME_BUS_NES_CTRL_ZAPPER_SENSOR

#pragma once

#include "screen.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_zapper_sensor_device

class nes_zapper_sensor_device : public device_t
{
public:
	// construction/destruction
	nes_zapper_sensor_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }

	bool detect_light(int x, int y);

protected:
	// device-level overrides
	virtual void device_start() override { }

private:
	// radius of circle picked up by the gun's photodiode
	static constexpr int radius = 5;
	// brightness threshold
	static constexpr int bright = 0xc0;
	// # of CRT scanlines that sustain brightness
	static constexpr int sustain = 22;

	required_device<screen_device> m_screen;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_ZAPPER_SENSOR, nes_zapper_sensor_device)

#endif // MAME_BUS_NES_CTRL_ZAPPER_SENSOR
