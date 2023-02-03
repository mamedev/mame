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
	virtual ~sound_module() = default;

	virtual void update_audio_stream(bool is_throttled, const int16_t *buffer, int samples_this_frame) = 0;
	virtual void set_mastervolume(int attenuation) = 0;
};

#endif // MAME_OSD_SOUND_SOUND_MODULE_H
