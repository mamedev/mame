// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * sound_module.h
 *
 */
#ifndef MAME_OSD_SOUND_SOUND_MODULE_H
#define MAME_OSD_SOUND_SOUND_MODULE_H

#pragma once

#include <cstdint>

//============================================================
//  CONSTANTS
//============================================================

#define OSD_SOUND_PROVIDER   "sound"

class sound_module
{
public:
	sound_module() : m_sample_rate(0), m_audio_latency(1) { }

	virtual ~sound_module() = default;

	virtual void update_audio_stream(bool is_throttled, const int16_t *buffer, int samples_this_frame) = 0;
	virtual void set_mastervolume(int attenuation) = 0;

	int sample_rate() const { return m_sample_rate; }

	// FIXME: these should be set by the implementation on initialisation, not by the OSD object
	int m_sample_rate;
	int m_audio_latency;
};

#endif // MAME_OSD_SOUND_SOUND_MODULE_H
