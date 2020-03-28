// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    vgm_visualizer.h

    Virtual VGM visualizer device.

    Provides a waterfall view, spectrograph view, and VU view.

***************************************************************************/

#ifndef MAME_SOUND_VGMVIZ_H
#define MAME_SOUND_VGMVIZ_H

#pragma once

#include "screen.h"
#include "emupal.h"

#include <vector>

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(VGMVIZ, vgmviz_device)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vgmviz_device

class vgmviz_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	vgmviz_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: vgmviz_device(mconfig, tag, owner, 0)
	{
	}
	vgmviz_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	virtual ~vgmviz_device();

	void cycle_spectrogram();

protected:
	enum spec_mode : int
	{
		SPEC_RAW,
		SPEC_BAR4,
		SPEC_BAR8,
		SPEC_BAR16,

		SPEC_COUNT
	};

	static constexpr int FFT_LENGTH = 512;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_sound_interface-level overrides
	void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	void init_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void fill_window();
	void apply_window(uint32_t buf_index);
	void apply_fft();
	void apply_waterfall();
	void find_levels();

	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	float m_audio_buf[2][2][FFT_LENGTH];
	float m_fft_buf[2][FFT_LENGTH];
	int m_current_rate;
	int m_audio_fill_index;
	int m_audio_frames_available;
	int m_audio_count[2];
	bool m_audio_available;

	int m_waterfall_length;
	int m_waterfall_buf[1024][256];
	float m_curr_levels[2];
	float m_curr_peaks[2];
	float m_window[FFT_LENGTH];
	spec_mode m_spec_mode;
};

#endif // MAME_SOUND_VGMVIZ_H
