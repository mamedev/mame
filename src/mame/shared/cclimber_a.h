// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, hap
/***************************************************************************

    Crazy Climber sound hardware

***************************************************************************/

#ifndef MAME_SHARED_CCLIMBER_A_H
#define MAME_SHARED_CCLIMBER_A_H

#pragma once

#include "sound/dac.h"
#include "sound/flt_vol.h"

DECLARE_DEVICE_TYPE(CCLIMBER_AUDIO, cclimber_audio_device)

// ======================> cclimber_audio_device

class cclimber_audio_device : public device_t
{
public:
	// construction/destruction
	cclimber_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto &set_sample_clockdiv(u8 div) { m_sample_clockdiv = div; return *this; } // determines base sound pitch (default 2)

	void sample_trigger(int state);
	void sample_trigger_w(u8 data);
	void sample_rate_w(u8 data);
	void sample_volume_w(u8 data);

protected:
	// device level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override { sample_volume_w(0); }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<dac_4bit_r2r_device> m_dac;
	required_device<filter_volume_device> m_volume;
	required_region_ptr<u8> m_rom;

	void start_address_w(u8 data) { m_start_address = data; }
	void loop_address_w(u8 data) { m_loop_address = data; }

	u8 m_sample_clockdiv;

	emu_timer *m_sample_timer;
	TIMER_CALLBACK_MEMBER(sample_tick);

	u16 m_address;
	u8 m_start_address;
	u8 m_loop_address;
	u8 m_sample_rate;
	int m_sample_trigger;
};


#endif // MAME_SHARED_CCLIMBER_A_H
