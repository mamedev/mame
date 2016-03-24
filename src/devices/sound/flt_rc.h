// license:BSD-3-Clause
// copyright-holders:Derrick Renaud, Couriersud
#pragma once

#ifndef __FLT_RC_H__
#define __FLT_RC_H__

#include "machine/rescap.h"

/*
 * FLT_RC_LOWPASS:
 *
 * signal >--R1--+--R2--+
 *               |      |
 *               C      R3---> amp
 *               |      |
 *              GND    GND
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
 * Same as FLT_RC_HIGHPASS, but with standard frequency of 16 HZ
 * This filter may be setup just with
 *
 * MCFG_FILTER_RC_ADD("tag", 0)
 * MCFG_FILTER_RC_AC(&flt_rc_ac_default)
 *
 * Default behaviour:
 *
 * Without MCFG_SOUND_CONFIG, a disabled FLT_RC_LOWPASS is created
 *
 */

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_FILTER_RC_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, FILTER_RC, _clock)

#define MCFG_FILTER_RC_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, FILTER_RC, _clock)

#define MCFG_FILTER_RC_AC() \
	filter_rc_device::static_set_rc(*device, FLT_RC_AC, 10000, 0, 0, CAP_U(1));


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define FLT_RC_LOWPASS      0
#define FLT_RC_HIGHPASS     1
#define FLT_RC_AC           2

// ======================> filter_rc_device

class filter_rc_device : public device_t,
							public device_sound_interface
{
public:
	filter_rc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~filter_rc_device() { }

	// static configuration
	static void static_set_rc(device_t &device, int type, double R1, double R2, double R3, double C);

	void filter_rc_set_RC(int type, double R1, double R2, double R3, double C);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	void recalc();

private:
	sound_stream*  m_stream;
	int            m_k;
	int            m_memory;
	int            m_type;
	double         m_R1;
	double         m_R2;
	double         m_R3;
	double         m_C;
};

extern const device_type FILTER_RC;

#endif /* __FLT_RC_H__ */
