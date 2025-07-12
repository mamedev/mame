// license:BSD-3-Clause
// copyright-holders:Derrick Renaud, Couriersud
#ifndef MAME_SOUND_FLT_RC_H
#define MAME_SOUND_FLT_RC_H

#pragma once

#include "machine/rescap.h"

/*
 * FLT_RC_LOWPASS_3R:
 *
 * signal >--R1--+--R2--+
 *               |      |
 *               C      R3---> amp
 *               |      |
 *              GND    GND
 *
 * Set C=0 to disable filter
 *
 * FLT_RC_LOWPASS:
 *
 * signal >--R1--+----> amp
 *               |
 *               C
 *               |
 *              GND
 *
 * Set C=0 to disable filter
 *
 * FLT_RC_HIGHPASS:
 *
 * signal >--C---+----> amp
 *               |
 *               R1
 *               |
 *              GND
 *
 * Set C = 0 to disable filter
 *
 * FLT_RC_AC:
 *
 * Same as FLT_RC_HIGHPASS, but with a standard cutoff frequency of ~16Hz
 * (10KOhm R, 1uF C)
 * This filter may be setup just with
 *
 * FILTER_RC(config, "tag", 0).set_ac();
 *
 * Default behaviour:
 *
 * Without set_ac(), a disabled FLT_RC_LOWPASS_3R is created
 *
 */

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> filter_rc_device

class filter_rc_device : public device_t, public device_sound_interface
{
public:
	enum
	{
		LOWPASS_3R   = 0,
		LOWPASS      = 2,
		HIGHPASS     = 3,
		AC           = 4
	};

	filter_rc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration
	filter_rc_device &set_rc(int type, double R1, double R2, double R3, double C)
	{
		m_type = type;
		m_R1 = R1;
		m_R2 = R2;
		m_R3 = R3;
		m_C = C;
		return *this;
	}

	filter_rc_device &set_lowpass(double R, double C)
	{
		m_type = LOWPASS;
		m_R1 = R;
		m_R2 = 0;
		m_R3 = 0;
		m_C = C;
		return *this;
	}

	filter_rc_device &filter_rc_set_RC(int type, double R1, double R2, double R3, double C)
	{
		m_stream->update();
		set_rc(type, R1, R2, R3, C);
		m_last_sample_rate = 0;
		return *this;
	}

	filter_rc_device &set_ac()
	{
		return set_rc(filter_rc_device::AC, RES_K(10), 0, 0, CAP_U(1));
	}

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	void recalc();

private:
	sound_stream*  m_stream;
	sound_stream::sample_t m_k;
	sound_stream::sample_t m_memory;
	int            m_type;
	int            m_last_sample_rate;
	double         m_R1;
	double         m_R2;
	double         m_R3;
	double         m_C;
};

DECLARE_DEVICE_TYPE(FILTER_RC, filter_rc_device)

#endif // MAME_SOUND_FLT_RC_H
