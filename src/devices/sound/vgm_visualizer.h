// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    vgm_visualizer.h

    Virtual VGM visualizer device.

    Provides a waterfall view, spectrograph view, and VU view.

***************************************************************************/

#ifndef MAME_SOUND_VGM_VISUALIZER_H
#define MAME_SOUND_VGM_VISUALIZER_H

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

class vgmviz_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	vgmviz_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: vgmviz_device(mconfig, tag, owner, 0)
	{
	}
	vgmviz_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	virtual ~vgmviz_device();

	void cycle_viz_mode();
	void toggle_normalize();

protected:
	enum spec_viz_mode : int
	{
		SPEC_VIZ_BAR,
		SPEC_VIZ_PILLAR,
		SPEC_VIZ_TOP
	};

	enum viz_mode : int
	{
		VIZ_WAVEFORM,
		VIZ_WATERFALL,

		VIZ_SPEC_START,
		VIZ_RAW_SPEC = VIZ_SPEC_START,
		VIZ_TOP_SPEC,
		VIZ_BAR_SPEC4,
		VIZ_PILLAR_SPEC4,
		VIZ_TOP_SPEC4,
		VIZ_BAR_SPEC8,
		VIZ_PILLAR_SPEC8,
		VIZ_TOP_SPEC8,
		VIZ_BAR_SPEC16,
		VIZ_PILLAR_SPEC16,
		VIZ_TOP_SPEC16,
		VIZ_SPEC_END = VIZ_TOP_SPEC16,

		VIZ_COUNT
	};

	static constexpr int FFT_LENGTH = 512;
	static constexpr int BUF_LENGTH = FFT_LENGTH;
	static constexpr int SCREEN_WIDTH = FFT_LENGTH / 2;
	static constexpr int SCREEN_HEIGHT = 768;
	static constexpr size_t NORMALIZE_BUF_SIZE = 131072;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface-level overrides
	void sound_stream_update(sound_stream &stream) override;

	void update_waveform();
	void update_fft();

	void init_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_waveform(bitmap_rgb32 &bitmap);
	void draw_waterfall(bitmap_rgb32 &bitmap);
	template <int BarSize, int SpecMode> void draw_spectrogram(bitmap_rgb32 &bitmap);

	void fill_window();
	void apply_window(uint32_t buf_index);
	void apply_fft();
	void apply_waterfall();
	void find_levels();

	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	uint32_t m_history_length;
	float m_audio_buf[2][2][FFT_LENGTH];
	float m_fft_buf[2][FFT_LENGTH];
	int m_current_rate;
	int m_audio_fill_index;
	int m_audio_frames_available;
	int m_audio_count[2];
	bool m_audio_available;

	int m_waterfall_length;
	int m_waterfall_buf[SCREEN_WIDTH * 2][FFT_LENGTH / 2];

	float m_curr_levels[2];
	float m_curr_peaks[2];

	float m_window[FFT_LENGTH];

	bitmap_rgb32 m_bitmap;
	bool m_clear_pending;

	viz_mode m_viz_mode;

	static const bool NEEDS_FFT[VIZ_COUNT];
};

#endif // MAME_SOUND_VGM_VISUALIZER_H
