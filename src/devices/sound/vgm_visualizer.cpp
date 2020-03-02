// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    vgm_visualizer.cpp

    Virtual VGM visualizer device.

    Provides a waterfall view, spectrograph view, and VU view.

***************************************************************************/

#include "emu.h"
#include "sound/vgm_visualizer.h"
#include "fft.h"

#include <cmath>


static float lerp(float a, float b, float f)
{
	return (b - a) * f + a;
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(VGMVIZ, vgmviz_device, "vgmviz", "VGM Visualizer")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vgmviz_device - constructor
//-------------------------------------------------

vgmviz_device::vgmviz_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VGMVIZ, tag, owner, clock)
	, device_mixer_interface(mconfig, *this, 2)
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
	WDL_fft_init();
	fill_window();
}


//-------------------------------------------------
//  fill_window - fill in the windowing data
//-------------------------------------------------

void vgmviz_device::fill_window()
{
	double window_pos_delta = (3.14159265358979 * 2) / FFT_LENGTH;
	double power = 0;
	for (int i = 0; i < (FFT_LENGTH / 2) + 1; i++)
	{
		double window_pos = i * window_pos_delta;
		m_window[i] = 0.53836 - cos(window_pos) * 0.46164;
		power += m_window[i];
	}
	power = 0.5 / (power * 2.0 - m_window[FFT_LENGTH / 2]);
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
	double *audio_l = m_audio_buf[buf_index][0];
	double *audio_r = m_audio_buf[buf_index][1];
	double *buf_l = m_fft_buf[0];
	double *buf_r = m_fft_buf[1];
	double *window = m_window;
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
			cmpl->re = sqrt(cmpl->re * cmpl->re + cmpl->im * cmpl->im);
		}
	}
}


//-------------------------------------------------
//  apply_waterfall - calculate the waterfall-view
//  data
//-------------------------------------------------

void vgmviz_device::apply_waterfall()
{
	int total_bars = FFT_LENGTH / 2;
	int bar_step = total_bars / 256;
	WDL_FFT_COMPLEX* bins[2] = { (WDL_FFT_COMPLEX*)m_fft_buf[0], (WDL_FFT_COMPLEX*)m_fft_buf[1] };
	int bar_index = 0;
	for (int bar = 0; bar < 256; bar++, bar_index += bar_step)
	{
		if (bar_index < 2)
		{
			continue;
		}
		double val = 0.0;
		for (int i = 0; i < bar_step; i++)
		{
			int permuted = WDL_fft_permute(FFT_LENGTH / 2, (bar * bar_step) + i);
			val = std::max<WDL_FFT_REAL>(bins[0][permuted].re + bins[1][permuted].re, val);
		}
		int level = (int)(log(val * 32768.0) * 31.0);
		m_waterfall_buf[m_waterfall_length % (FFT_LENGTH / 2 + 16)][255 - bar] = (level < 0) ? 0 : (level > 255 ? 255 : level);
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
		m_curr_levels[0] = 0.0;
		m_curr_levels[1] = 0.0;
		m_curr_peaks[0] = 0.0;
		m_curr_peaks[1] = 0.0;
		return;
	}

	m_curr_levels[0] = 0.0;
	m_curr_levels[1] = 0.0;

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
		memset(m_audio_buf[i][0], 0, sizeof(double) * FFT_LENGTH);
		memset(m_audio_buf[i][1], 0, sizeof(double) * FFT_LENGTH);
		m_audio_count[i] = 0;
	}
	memset(m_fft_buf[0], 0, sizeof(double) * FFT_LENGTH);
	memset(m_fft_buf[1], 0, sizeof(double) * FFT_LENGTH);
	m_current_rate = 0;
	m_audio_fill_index = 0;
	m_audio_frames_available = 0;
	memset(m_curr_levels, 0, sizeof(double) * 2);
	memset(m_curr_peaks, 0, sizeof(double) * 2);

	m_waterfall_length = 0;
	for (int i = 0; i < 1024; i++)
	{
		memset(m_waterfall_buf[i], 0, sizeof(int) * 256);
	}
}


//-------------------------------------------------
//  sound_stream_update - update the outgoing
//  audio stream and process as necessary
//-------------------------------------------------

void vgmviz_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// clear output buffers
	for (int output = 0; output < m_outputs; output++)
		std::fill_n(outputs[output], samples, 0);

	m_current_rate = stream.sample_rate();

	// loop over samples
	const u8 *outmap = &m_outputmap[0];

	// for each input, add it to the appropriate output
	for (int pos = 0; pos < samples; pos++)
	{
		for (int inp = 0; inp < m_auto_allocated_inputs; inp++)
		{
			outputs[outmap[inp]][pos] += inputs[inp][pos];
		}

		for (int i = 0; i < m_outputs; i++)
		{
			m_audio_buf[m_audio_fill_index][i][m_audio_count[m_audio_fill_index]] = (outputs[i][pos] + 32768.0) / 65336.0;
		}

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
	m_screen->set_size(FFT_LENGTH / 2 + 16, 768);
	m_screen->set_visarea(0, FFT_LENGTH / 2 + 15, 0, 767);
	m_screen->set_screen_update(FUNC(vgmviz_device::screen_update));

	PALETTE(config, m_palette, FUNC(vgmviz_device::init_palette), 512 + FFT_LENGTH / 2 + 1);
}


//-------------------------------------------------
//  screen_update - update vu meters
//-------------------------------------------------

uint32_t vgmviz_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	find_levels();

	const pen_t *pal = m_palette->pens();
	int chan_x = 0;
	const int black_idx = (512 + FFT_LENGTH / 2);
	for (int chan = 0; chan < 2; chan++)
	{
		int level = (int)(m_curr_levels[chan] * 255.0);
		int peak = (int)(m_curr_peaks[chan] * 255.0);
		for (int y = 0; y < 512; y++)
		{
			int bar_y = 255 - (y >> 1);
			for (int x = 0; x < 7; x++)
			{
				uint32_t *line = &bitmap.pix32(y + 256);
				bool lit = bar_y <= level || bar_y == peak;
				line[chan_x + x] = pal[lit ? bar_y : black_idx];
			}
		}
		chan_x += 8;
		m_curr_peaks[chan] *= 0.99;
	}

	int total_bars = FFT_LENGTH / 2;
	WDL_FFT_COMPLEX *bins[2] = { (WDL_FFT_COMPLEX *)m_fft_buf[0], (WDL_FFT_COMPLEX *)m_fft_buf[1] };
	for (int bar = 0; bar < total_bars; bar++)
	{
		if (bar < 2)
		{
			continue;
		}
		int permuted = WDL_fft_permute(FFT_LENGTH/2, bar);
		double val = (bins[0][permuted].re + bins[1][permuted].re) * 0.5;
		int level = (int)(log(val * 32768.0) * 63.0);
		for (int y = 0; y < 512; y++)
		{
			int bar_y = 511 - y;
			uint32_t *line = &bitmap.pix32(y + 256);
			bool lit = bar_y <= level;
			line[bar + 16] = pal[lit ? (256 + bar) : black_idx];
		}
	}

	const int width = FFT_LENGTH / 2 + 16;
	for (int y = 0; y < 256; y++)
	{
		uint32_t* line = &bitmap.pix32(y);
		for (int x = 0; x < width; x++)
		{
			if (m_waterfall_length < width)
			{
				const int sample = m_waterfall_buf[x][y];
				*line++ = pal[256 + FFT_LENGTH / 2 + sample];
			}
			else
			{
				const int sample = m_waterfall_buf[((m_waterfall_length - width) + x) % width][y];
				*line++ = pal[256 + FFT_LENGTH / 2 + sample];
			}
		}
	}
	return 0;
}
