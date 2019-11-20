// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_VT1682_TIMER_H
#define MAME_MACHINE_VT1682_TIMER_H

#pragma once

DECLARE_DEVICE_TYPE(VT_VT1682_TIMER, vrt_vt1682_timer_device)

class vrt_vt1682_timer_device : public device_t
{
public:
	// construction/destruction
	vrt_vt1682_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// so that we can filter logging, sound timer gets used hundreds of times a frame, so logging it is unwise
	void set_sound_timer() { m_is_sound_timer = true; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	bool m_is_sound_timer;

};

#endif // MAME_MACHINE_VT1682_TIMER_H
