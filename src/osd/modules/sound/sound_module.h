// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * sound_module.h
 *
 */

#ifndef SOUND_MODULE_H_
#define SOUND_MODULE_H_

#include "osdepend.h"
#include "modules/osdmodule.h"

//============================================================
//  CONSTANTS
//============================================================

#define OSD_SOUND_PROVIDER   "sound"

class sound_module
{
public:
	sound_module() : m_sample_rate(0), m_audio_latency(1) { }

	virtual ~sound_module() { }

	virtual void update_audio_stream(bool is_throttled, const int16_t *buffer, int samples_this_frame) = 0;
	virtual void set_mastervolume(int attenuation) = 0;
	
  // todo: should really be pure virtual and implemented for all platforms
  virtual void capture_audio_stream(bool is_throttled, const int16_t *buffer, int samples_this_frame)
  {
    if (buffer)
      memset((void*)buffer, 0, samples_this_frame * sizeof(int16_t));
  }
	
	int sample_rate() const { return m_sample_rate; }

	int m_sample_rate;
	int m_audio_latency;
};

#endif /* FONT_MODULE_H_ */
