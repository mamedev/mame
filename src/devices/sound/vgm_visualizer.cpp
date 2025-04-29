// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    vgm_visualizer.cpp

    Virtual VGM visualizer device.

    Provides a waterfall view, spectrograph view, and VU view.

***************************************************************************/

#include "emu.h"
#include "vgm_visualizer.h"

#include "wdlfft/fft.h"

#include <cmath>

constexpr int vgmviz_device::SCREEN_HEIGHT;

constexpr float lerp(float a, float b, float f)
{
	return (b - a) * f + a;
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(VGMVIZ, vgmviz_device, "vgmviz", "VGM Visualizer")



/*static*/ const bool vgmviz_device::NEEDS_FFT[VIZ_COUNT] =
{
	false,  // VIZ_WAVEFORM
	true,   // VIZ_WATERFALL
	true,   // VIZ_RAWSPEC
	true,   // VIZ_BARSPEC4
	true,   // VIZ_BARSPEC8
	true    // VIZ_BARSPEC16
};

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vgmviz_device - constructor
//-------------------------------------------------

vgmviz_device::vgmviz_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VGMVIZ, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_screen(*this, "screen")
	, m_palette(*this, "palette")
{
}


//-------------------------------------------------
//  ~vgmviz_device - destructor
//-------------------------------------------------

vgmviz_device::~vgmviz_device()
{
}


//-------------------------------------------------
//  device_start - handle device startup
//-------------------------------------------------

void vgmviz_device::device_start()
{
	stream_alloc(2, 2, machine().sample_rate());
	WDL_fft_init();
	fill_window();
	m_bitmap.resize(SCREEN_WIDTH, SCREEN_HEIGHT);
}


//-------------------------------------------------
//  fill_window - fill in the windowing data
//-------------------------------------------------

void vgmviz_device::fill_window()
{
	float window_pos_delta = (M_PI * 2) / FFT_LENGTH;
	float power = 0;
	for (int i = 0; i < (FFT_LENGTH / 2) + 1; i++)
	{
		float window_pos = i * window_pos_delta;
		m_window[i] = 0.53836f - cosf(window_pos) * 0.46164f;
		power += m_window[i];
	}
	power = 0.5f / (power * 2.0f - m_window[FFT_LENGTH / 2]);
	for (int i = 0; i < (FFT_LENGTH / 2) + 1; i++)
	{
		m_window[i] *= power;
	}
}


//-------------------------------------------------
//  fill_window - apply windowing data to the
//  mixed signal
//-------------------------------------------------

void vgmviz_device::apply_window(uint32_t buf_index)
{
	float *audio_l = m_audio_buf[buf_index][0];
	float *audio_r = m_audio_buf[buf_index][1];
	float *buf_l = m_fft_buf[0];
	float *buf_r = m_fft_buf[1];
	float *window = m_window;
	for (int i = 0; i < (FFT_LENGTH / 2) + 1; i++)
	{
		*buf_l++ = *audio_l++ * *window;
		*buf_r++ = *audio_r++ * *window;
		window++;
	}
	for (int i = 0; i < (FFT_LENGTH / 2) - 1; i++)
	{
		window--;
		*buf_l++ = *audio_l++ * *window;
		*buf_r++ = *audio_r++ * *window;
	}
}


//-------------------------------------------------
//  apply_fft - run the FFT on the windowed data
//-------------------------------------------------

void vgmviz_device::apply_fft()
{
	WDL_real_fft((WDL_FFT_REAL*)m_fft_buf[0], FFT_LENGTH, 0);
	WDL_real_fft((WDL_FFT_REAL*)m_fft_buf[1], FFT_LENGTH, 0);

	for (int i = 1; i < FFT_LENGTH/2; i++)
	{
		for (int chan = 0; chan < 2; chan++)
		{
			WDL_FFT_COMPLEX* cmpl = (WDL_FFT_COMPLEX*)m_fft_buf[chan] + i;
			cmpl->re = sqrtf(cmpl->re * cmpl->re + cmpl->im * cmpl->im);
		}
	}
}


//-------------------------------------------------
//  apply_waterfall - calculate the waterfall-view
//  data
//-------------------------------------------------

void vgmviz_device::apply_waterfall()
{
	const int total_bars = FFT_LENGTH / 2;
	WDL_FFT_COMPLEX* bins[2] = { (WDL_FFT_COMPLEX*)m_fft_buf[0], (WDL_FFT_COMPLEX*)m_fft_buf[1] };
	for (int bar = 0; bar < std::min<int>(total_bars, SCREEN_HEIGHT); bar++)
	{
		if (bar < 2)
		{
			continue;
		}
		int permuted = WDL_fft_permute(FFT_LENGTH / 2, bar);
		float val = std::max<float>(bins[0][permuted].re, bins[1][permuted].re);
		int level = int(log10f(val * 32768.0f) * 95.0f);
		m_waterfall_buf[m_waterfall_length % SCREEN_WIDTH][total_bars - bar] = (level < 0) ? 0 : (level > 255 ? 255 : level);
	}
	m_waterfall_length++;
}


//-------------------------------------------------
//  find_levels - find average and peak levels
//-------------------------------------------------

void vgmviz_device::find_levels()
{
	if (m_audio_frames_available < 2 || m_current_rate == 0)
	{
		m_curr_levels[0] = 0.0f;
		m_curr_levels[1] = 0.0f;
		m_curr_peaks[0] = 0.0f;
		m_curr_peaks[1] = 0.0f;
		return;
	}

	m_curr_levels[0] = 0.0f;
	m_curr_levels[1] = 0.0f;

	int read_index = m_audio_fill_index;
	const int samples_needed = m_current_rate / 60;
	int samples_remaining = samples_needed;
	int samples_found = 0;
	do
	{
		for (int i = std::min<int>(FFT_LENGTH - 1, m_audio_count[read_index]); i >= 0 && samples_remaining > 0; i--, samples_remaining--)
		{
			for (int chan = 0; chan < 2; chan++)
			{
				if (m_audio_buf[read_index][chan][i] > m_curr_levels[chan])
				{
					m_curr_levels[chan] += m_audio_buf[read_index][chan][i];
				}
			}
			samples_found++;
			samples_remaining--;
		}
		read_index = 1 - m_audio_fill_index;
	} while (samples_remaining > 0 && read_index != m_audio_fill_index);

	if (samples_found > 0)
	{
		for (int chan = 0; chan < 2; chan++)
		{
			if (m_curr_levels[chan] > m_curr_peaks[chan])
			{
				m_curr_peaks[chan] = m_curr_levels[chan];
			}
		}
	}
}


//-------------------------------------------------
//  device_reset - handle device reset
//-------------------------------------------------

void vgmviz_device::device_reset()
{
	for (int i = 0; i < 2; i++)
	{
		memset(m_audio_buf[i][0], 0, sizeof(float) * FFT_LENGTH);
		memset(m_audio_buf[i][1], 0, sizeof(float) * FFT_LENGTH);
		m_audio_count[i] = 0;
	}
	memset(m_fft_buf[0], 0, sizeof(float) * FFT_LENGTH);
	memset(m_fft_buf[1], 0, sizeof(float) * FFT_LENGTH);
	m_current_rate = 0;
	m_audio_fill_index = 0;
	m_audio_frames_available = 0;
	memset(m_curr_levels, 0, sizeof(float) * 2);
	memset(m_curr_peaks, 0, sizeof(float) * 2);

	m_waterfall_length = 0;
	for (int i = 0; i < SCREEN_WIDTH; i++)
	{
		memset(m_waterfall_buf[i], 0, sizeof(int) * 256);
	}

	m_viz_mode = VIZ_WAVEFORM;
	m_clear_pending = true;
	m_bitmap.fill(0);

	m_history_length = 0;
}


//-------------------------------------------------
//  cycle_spectrogram - cycle the visualization
//  mode among the valid modes.
//-------------------------------------------------

void vgmviz_device::cycle_viz_mode()
{
	m_viz_mode = (viz_mode)((int)m_viz_mode + 1);
	if (m_viz_mode == VIZ_COUNT)
	{
		m_viz_mode = VIZ_WAVEFORM;
	}
	m_bitmap.fill(0);
	m_clear_pending = true;
}


//-------------------------------------------------
//  sound_stream_update - update the outgoing
//  audio stream and process as necessary
//-------------------------------------------------

void vgmviz_device::sound_stream_update(sound_stream &stream)
{
	// Passthrough the audio
	stream.copy(0, 0);
	stream.copy(1, 1);

	// now consume the inputs
	for (int pos = 0; pos < stream.samples(); pos++)
	{
		for (int i = 0; i < stream.output_count(); i++)
		{
			// Original code took 16-bit sample / 65536.0 instead of 32768.0, so multiply by 0.5 here but is it necessary?
			const float sample = stream.get(i, pos) * 0.5f;
			m_audio_buf[m_audio_fill_index][i][m_audio_count[m_audio_fill_index]] = sample + 0.5f;
		}

		switch (m_viz_mode)
		{
		default:
			update_waveform();
			break;
		case VIZ_WATERFALL:
		case VIZ_RAW_SPEC:
		case VIZ_BAR_SPEC4:
		case VIZ_BAR_SPEC8:
		case VIZ_BAR_SPEC16:
		case VIZ_PILLAR_SPEC4:
		case VIZ_PILLAR_SPEC8:
		case VIZ_PILLAR_SPEC16:
		case VIZ_TOP_SPEC:
		case VIZ_TOP_SPEC4:
		case VIZ_TOP_SPEC8:
		case VIZ_TOP_SPEC16:
			update_fft();
			break;
		}
	}
}


//-------------------------------------------------
//  update_waveform - perform a wave-style update
//-------------------------------------------------

void vgmviz_device::update_waveform()
{
	m_history_length++;
	m_audio_count[m_audio_fill_index]++;
	if (m_audio_count[m_audio_fill_index] >= FFT_LENGTH)
	{
		m_audio_fill_index = 1 - m_audio_fill_index;
		if (m_audio_frames_available < 2)
		{
			m_audio_frames_available++;
		}
		m_audio_count[m_audio_fill_index] = 0;
	}
}

//-------------------------------------------------
//  update_fft - keep the FFT up-to-date
//-------------------------------------------------

void vgmviz_device::update_fft()
{
	m_audio_count[m_audio_fill_index]++;
	if (m_audio_count[m_audio_fill_index] >= FFT_LENGTH)
	{
		apply_window(m_audio_fill_index);
		apply_fft();
		apply_waterfall();

		m_audio_fill_index = 1 - m_audio_fill_index;
		if (m_audio_frames_available < 2)
		{
			m_audio_frames_available++;
		}
		m_audio_count[m_audio_fill_index] = 0;
	}
}

//-------------------------------------------------
//  init_palette - initialize the palette
//-------------------------------------------------

void vgmviz_device::init_palette(palette_device &palette) const
{
	for (int i = 0; i < 256; i++)
	{
		float percent = (float)i / 255.0f;
		if (percent < 0.75f)
		{
			float r = lerp(0.0f, 1.0f, percent / 0.75f);
			float g = 1.0f;
			float b = 0.0f;
			palette.set_pen_color(i, rgb_t((uint8_t)(r * 255), (uint8_t)(g * 255), (uint8_t)(b * 255)));
		}
		else
		{
			float r = lerp(1.0f, 1.0f, (percent - 0.75f) / 0.25f);
			float g = lerp(1.0f, 0.0f, (percent - 0.75f) / 0.25f);
			float b = 0.0f;
			palette.set_pen_color(i, rgb_t((uint8_t)(r * 255), (uint8_t)(g * 255), (uint8_t)(b * 255)));
		}
	}

	for (int i = 0; i < FFT_LENGTH / 2; i++)
	{
		double h = ((double)i / (FFT_LENGTH / 2)) * 360.0;
		double s = 1.0;
		double v = 1.0;

		double c = s * v;
		double x = c * (1 - fabs(fmod(h / 60.0, 2.0) - 1.0));
		double m = v - c;
		double rs = 0.0;
		double gs = 0.0;
		double bs = 0.0;

		if (h >= 0.0 && h < 60.0)
		{
			rs = c;
			gs = x;
			bs = 0.0;
		}
		else if (h >= 60.0 && h < 120.0)
		{
			rs = x;
			gs = c;
			bs = 0.0;
		}
		else if (h >= 120.0 && h < 180.0)
		{
			rs = 0.0;
			gs = c;
			bs = x;
		}
		else if (h >= 180.0 && h < 240.0)
		{
			rs = 0.0;
			gs = x;
			bs = c;
		}
		else if (h >= 240.0 && h < 300.0)
		{
			rs = x;
			gs = 0.0;
			bs = c;
		}
		else if (h < 360.0)
		{
			rs = c;
			gs = 0.0;
			bs = x;
		}

		palette.set_pen_color(i + 256, rgb_t((uint8_t)((rs + m) * 255), (uint8_t)((gs + m) * 255), (uint8_t)((bs + m) * 255)));
	}

	for (int y = 0; y < 256; y++)
	{
		float percent = (float)y / 255.0f;
		if (percent < 0.75f)
		{
			float r = 0.0f;
			float g = 0.0f;
			float b = lerp(0.0f, 1.0f, percent / 0.5f);
			palette.set_pen_color(y + 256 + FFT_LENGTH / 2, rgb_t((uint8_t)(r * 255), (uint8_t)(g * 255), (uint8_t)(b * 255)));
		}
		else
		{
			float r = lerp(0.0f, 1.0f, (percent - 0.5f) / 0.5f);
			float g = lerp(0.0f, 1.0f, (percent - 0.5f) / 0.5f);
			float b = 1.0f;
			palette.set_pen_color(y + 256 + FFT_LENGTH / 2, rgb_t((uint8_t)(r * 255), (uint8_t)(g * 255), (uint8_t)(b * 255)));
		}
	}

	palette.set_pen_color(512 + FFT_LENGTH / 2, rgb_t(0, 0, 0));
}


//-------------------------------------------------
//  device_add_mconfig - handle device setup
//-------------------------------------------------

void vgmviz_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(SCREEN_WIDTH, SCREEN_HEIGHT);
	m_screen->set_visarea(0, SCREEN_WIDTH-1, 0, SCREEN_HEIGHT-1);
	m_screen->set_screen_update(FUNC(vgmviz_device::screen_update));

	PALETTE(config, m_palette, FUNC(vgmviz_device::init_palette), 512 + FFT_LENGTH / 2 + 1);
}


//-------------------------------------------------
//  screen_update - update vu meters
//-------------------------------------------------

template void vgmviz_device::draw_spectrogram<1, vgmviz_device::SPEC_VIZ_BAR>(bitmap_rgb32 &bitmap);
template void vgmviz_device::draw_spectrogram<4, vgmviz_device::SPEC_VIZ_BAR>(bitmap_rgb32 &bitmap);
template void vgmviz_device::draw_spectrogram<8, vgmviz_device::SPEC_VIZ_BAR>(bitmap_rgb32 &bitmap);
template void vgmviz_device::draw_spectrogram<16, vgmviz_device::SPEC_VIZ_BAR>(bitmap_rgb32 &bitmap);
template void vgmviz_device::draw_spectrogram<4, vgmviz_device::SPEC_VIZ_TOP>(bitmap_rgb32 &bitmap);
template void vgmviz_device::draw_spectrogram<8, vgmviz_device::SPEC_VIZ_TOP>(bitmap_rgb32 &bitmap);
template void vgmviz_device::draw_spectrogram<16, vgmviz_device::SPEC_VIZ_TOP>(bitmap_rgb32 &bitmap);
template void vgmviz_device::draw_spectrogram<3, vgmviz_device::SPEC_VIZ_PILLAR>(bitmap_rgb32 &bitmap);
template void vgmviz_device::draw_spectrogram<6, vgmviz_device::SPEC_VIZ_PILLAR>(bitmap_rgb32 &bitmap);
template void vgmviz_device::draw_spectrogram<12, vgmviz_device::SPEC_VIZ_PILLAR>(bitmap_rgb32 &bitmap);

uint32_t vgmviz_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	switch (m_viz_mode)
	{
	default:
		bitmap.fill(0, cliprect);
		draw_waveform(bitmap);
		break;
	case VIZ_WATERFALL:
		bitmap.fill(0, cliprect);
		draw_waterfall(bitmap);
		break;
	case VIZ_RAW_SPEC:
		draw_spectrogram<1, SPEC_VIZ_BAR>(bitmap);
		break;
	case VIZ_TOP_SPEC:
		draw_spectrogram<1, SPEC_VIZ_TOP>(bitmap);
		break;
	case VIZ_BAR_SPEC4:
		draw_spectrogram<4, SPEC_VIZ_BAR>(bitmap);
		break;
	case VIZ_PILLAR_SPEC4:
		draw_spectrogram<3, SPEC_VIZ_PILLAR>(bitmap);
		break;
	case VIZ_TOP_SPEC4:
		draw_spectrogram<4, SPEC_VIZ_TOP>(bitmap);
		break;
	case VIZ_BAR_SPEC8:
		draw_spectrogram<8, SPEC_VIZ_BAR>(bitmap);
		break;
	case VIZ_PILLAR_SPEC8:
		draw_spectrogram<6, SPEC_VIZ_PILLAR>(bitmap);
		break;
	case VIZ_TOP_SPEC8:
		draw_spectrogram<8, SPEC_VIZ_TOP>(bitmap);
		break;
	case VIZ_BAR_SPEC16:
		draw_spectrogram<16, SPEC_VIZ_BAR>(bitmap);
		break;
	case VIZ_PILLAR_SPEC16:
		draw_spectrogram<12, SPEC_VIZ_PILLAR>(bitmap);
		break;
	case VIZ_TOP_SPEC16:
		draw_spectrogram<16, SPEC_VIZ_TOP>(bitmap);
		break;
	}
	return 0;
}

template <int BarSize, int SpecMode> void vgmviz_device::draw_spectrogram(bitmap_rgb32 &bitmap)
{
	const int black_index = 512 + FFT_LENGTH / 2;

	if (m_clear_pending || SpecMode == SPEC_VIZ_BAR)
	{
		bitmap.fill(0);
		m_clear_pending = false;
	}

	pen_t const *const pal = m_palette->pens();

	int width = SCREEN_WIDTH;
	switch (SpecMode)
	{
	case SPEC_VIZ_PILLAR:
		for (int y = 2; y < SCREEN_HEIGHT; y++)
		{
			uint32_t const *const src = &m_bitmap.pix(y);
			uint32_t *const dst = &bitmap.pix(y - 2);
			for (int x = SCREEN_WIDTH - 1; x >= 1; x--)
			{
				dst[x] = src[x - 1];
			}
		}
		width = int(SCREEN_WIDTH * 0.75f);
		break;
	case SPEC_VIZ_TOP:
		for (int y = 1; y < SCREEN_HEIGHT; y++)
		{
			std::copy_n(&m_bitmap.pix(y), SCREEN_WIDTH, &bitmap.pix(y - 1));
		}
		break;
	default:
		break;
	}

	const int total_bars = FFT_LENGTH / 2;
	WDL_FFT_COMPLEX *bins[2] = { (WDL_FFT_COMPLEX *)m_fft_buf[0], (WDL_FFT_COMPLEX *)m_fft_buf[1] };

	for (int bar = 2; bar < total_bars && bar < width; bar += BarSize)
	{
		float max_val = 0.0f;
		for (int sub_bar = 0; sub_bar < BarSize && (bar + sub_bar) < total_bars; sub_bar++)
		{
			int permuted = WDL_fft_permute(FFT_LENGTH/2, bar + sub_bar);
			const float max_channel = std::max<float>(bins[0][permuted].re, bins[1][permuted].re);
			max_val = std::max<float>(max_channel, max_val);
		}

		float raw_level = logf(max_val * 32768.0f);

		int level = 0;
		int pal_index = 0;
		switch (SpecMode)
		{
		case SPEC_VIZ_BAR:
			level = int(raw_level * 95.0f);
			if (level <= 767)
			{
				pal_index = 255 - level / 3;
			}
			for (int y = 0; y < SCREEN_HEIGHT; y++)
			{
				const int bar_y = SCREEN_HEIGHT - y;
				uint32_t *const line = &bitmap.pix(y);
				for (int x = 0; x < BarSize; x++)
				{
					line[(bar - 2) + x] = bar_y <= level ? pal[256 + pal_index] : pal[black_index];
				}
			}
			break;
		case SPEC_VIZ_PILLAR:
			level = int(raw_level * 59.0f);
			if (level < 383)
			{
				pal_index = 255 - (int)(level * 0.75f);
			}

			for (int y = 0; y < SCREEN_HEIGHT; y++)
			{
				const int bar_y = SCREEN_HEIGHT - y;
				uint32_t *const line = &bitmap.pix(y);
				line[0] = 0;
				if (bar_y > level)
				{
					continue;
				}
				const uint32_t entry = pal[256 + pal_index];
				for (int x = (bar_y < level) ? 0 : 1; x < (BarSize - 1); x++)
				{
					if (bar_y < (level - 1))
					{
						const uint8_t r = uint8_t(((entry >> 16) & 0xff) * 0.75f);
						const uint8_t g = uint8_t(((entry >>  8) & 0xff) * 0.75f);
						const uint8_t b = uint8_t(((entry >>  0) & 0xff) * 0.75f);
						line[(bar - 2) + x] = 0xff000000 | (r << 16) | (g << 8) | b;
					}
					else
					{
						line[(bar - 2) + x] = pal[256 + pal_index];
					}
				}
				const uint8_t r = uint8_t(((entry >> 16) & 0xff) * 0.5f);
				const uint8_t g = uint8_t(((entry >>  8) & 0xff) * 0.5f);
				const uint8_t b = uint8_t(((entry >>  0) & 0xff) * 0.5f);
				line[(bar - 2) + (BarSize - 1)] = 0xff000000 | (r << 16) | (g << 8) | b;
				if (bar_y < level)
				{
					line[(bar - 2) + (BarSize - 2)] = 0xff000000 | (r << 16) | (g << 8) | b;
				}
			}
			for (int x = 0; x < SCREEN_WIDTH; x++)
			{
				bitmap.pix(SCREEN_HEIGHT - 1, x) = 0;
				bitmap.pix(SCREEN_HEIGHT - 2, x) = 0;
			}
			for (int y = 0; y < SCREEN_HEIGHT; y++)
				std::copy_n(&bitmap.pix(y), SCREEN_WIDTH, &m_bitmap.pix(y));
			break;
		case SPEC_VIZ_TOP:
			level = int(raw_level * 63.0f);
			if (level < 255)
			{
				pal_index = 255 - level;
			}

			for (int x = 0; x < BarSize; x++)
			{
				bitmap.pix(SCREEN_HEIGHT - 1, (bar - 2) + x) = level > 0 ? pal[256 + pal_index] : pal[black_index];
			}

			for (int y = 0; y < SCREEN_HEIGHT; y++)
				std::copy_n(&bitmap.pix(y), SCREEN_WIDTH, &m_bitmap.pix(y));
			break;
		}
	}
}

void vgmviz_device::draw_waterfall(bitmap_rgb32 &bitmap)
{
	const pen_t *pal = m_palette->pens();
	float tex_height = ((float)FFT_LENGTH / 2) - 1.0f;
	for (int y = 0; y < SCREEN_HEIGHT; y++)
	{
		const float v0 = (float)y / SCREEN_HEIGHT;
		const float v1 = (float)(y + 1) / SCREEN_HEIGHT;
		const float v0h = v0 * tex_height;
		const float v1h = v1 * tex_height;
		const int v0_index = (int)v0h;
		const int v1_index = (int)v1h;
		const float interp = v0h - (float)v0_index;
		uint32_t* line = &bitmap.pix(y);
		for (int x = 0; x < SCREEN_WIDTH; x++)
		{
			if (m_waterfall_length < SCREEN_WIDTH)
			{
				const float s0 = m_waterfall_buf[x][v0_index];
				const float s1 = m_waterfall_buf[x][v1_index];
				const int sample = (int)std::round(lerp(s0, s1, interp));
				*line++ = pal[256 + FFT_LENGTH / 2 + sample];
			}
			else
			{
				const int x_index = ((m_waterfall_length - SCREEN_WIDTH) + x) % SCREEN_WIDTH;
				const float s0 = m_waterfall_buf[x_index][v0_index];
				const float s1 = m_waterfall_buf[x_index][v1_index];
				const int sample = (int)std::round(lerp(s0, s1, interp));
				*line++ = pal[256 + FFT_LENGTH / 2 + sample];
			}
		}
	}
}

void vgmviz_device::draw_waveform(bitmap_rgb32 &bitmap)
{
	static const uint32_t MED_GRAY = 0xff7f7f7f;
	static const uint32_t WHITE = 0xffffffff;
	static const uint32_t LEFT_COLOR = 0xffbf0000;
	static const uint32_t RIGHT_COLOR = 0xff00bf00;
	static const int CHANNEL_HEIGHT = (SCREEN_HEIGHT / 2) - 1;
	static const int CHANNEL_CENTER = CHANNEL_HEIGHT / 2;

	if (m_audio_frames_available == 0)
		return;

	for (int x = 0; x < SCREEN_WIDTH; x++)
	{
		bitmap.pix(CHANNEL_CENTER, x) = MED_GRAY;
		bitmap.pix(CHANNEL_HEIGHT + 1 + CHANNEL_CENTER, x) = MED_GRAY;

		const float raw_l = m_audio_buf[1 - m_audio_fill_index][0][((int)m_history_length + 1 + x) % FFT_LENGTH];
		const int sample_l = (int)((raw_l - 0.5f) * (CHANNEL_HEIGHT - 1));
		const int dy_l = (sample_l == 0) ? 0 : ((sample_l < 0) ? -1 : 1);
		const int endy_l = CHANNEL_CENTER;
		int y = endy_l - sample_l;
		do
		{
			bitmap.pix(y, x) = LEFT_COLOR;
			y += dy_l;
		} while(y != endy_l);

		const float raw_r = m_audio_buf[1 - m_audio_fill_index][1][((int)m_history_length + 1 + x) % FFT_LENGTH];
		const int sample_r = (int)((raw_r - 0.5f) * (CHANNEL_HEIGHT - 1));
		const int dy_r = (sample_r == 0) ? 0 : ((sample_r < 0) ? -1 : 1);
		const int endy_r = CHANNEL_HEIGHT + 1 + CHANNEL_CENTER;
		y = endy_r - sample_r;
		do
		{
			bitmap.pix(y, x) = RIGHT_COLOR;
			y += dy_r;
		} while(y != endy_r);

		bitmap.pix(CHANNEL_HEIGHT, x) = WHITE;
		bitmap.pix(CHANNEL_HEIGHT + 1, x) = WHITE;
	}
}
