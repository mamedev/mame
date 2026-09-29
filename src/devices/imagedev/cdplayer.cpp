// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    cdplayer.cpp

    Audio CD player transport

    Plays Red Book audio from a CD image in real time and hands every
    44.1 kHz stereo sample to the owner, for hardware that takes a
    CD player's line output as an input signal.

*********************************************************************/

#include "emu.h"
#include "cdplayer.h"

#include "multibyte.h"

#include <algorithm>
#include <cmath>
#include <vector>


DEFINE_DEVICE_TYPE(CD_PLAYER, cd_player_device, "cd_player", "Audio CD Player")

ALLOW_SAVE_TYPE(cd_player_device::transport);


cd_player_device::cd_player_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	cdrom_image_device(mconfig, CD_PLAYER, tag, owner, clock),
	m_sample_cb(*this),
	m_sample_timer(nullptr),
	m_state(transport::STOPPED),
	m_track(0),
	m_lba(0),
	m_sample(0),
	m_cached_lba(-1)
{
}

void cd_player_device::device_resolve_objects()
{
	cdrom_image_device::device_resolve_objects();

	m_sample_cb.resolve();
}

void cd_player_device::device_start()
{
	cdrom_image_device::device_start();

	std::fill(std::begin(m_sector), std::end(m_sector), 0);

	m_sample_timer = timer_alloc(FUNC(cd_player_device::sample_tick), this);

	save_item(NAME(m_state));
	save_item(NAME(m_track));
	save_item(NAME(m_lba));
	save_item(NAME(m_sample));
}

void cd_player_device::device_post_load()
{
	cdrom_image_device::device_post_load();

	m_cached_lba = -1;
}

std::pair<std::error_condition, std::string> cd_player_device::call_load()
{
	auto result = cdrom_image_device::call_load();

	set_state(transport::STOPPED);
	seek_track(0);

	return result;
}

void cd_player_device::call_unload()
{
	set_state(transport::STOPPED);
	seek_track(0);

	cdrom_image_device::call_unload();
}

u32 cd_player_device::track_start(int track) const
{
	return m_cdrom_handle ? m_cdrom_handle->get_track_start(track) : 0;
}

u32 cd_player_device::disc_end() const
{
	return m_cdrom_handle ? m_cdrom_handle->get_track_start(0xaa) : 0;
}

u32 cd_player_device::track_elapsed_frames() const
{
	return m_lba - track_start(m_track);
}

u32 cd_player_device::track_length_frames() const
{
	if (!m_cdrom_handle)
		return 0;

	return ((m_track + 1 < track_count()) ? track_start(m_track + 1) : disc_end()) - track_start(m_track);
}

void cd_player_device::seek_track(int track)
{
	m_track = track;
	m_lba = track_start(track);
	m_sample = 0;
}

void cd_player_device::set_state(transport state)
{
	if (state == transport::PLAYING && !m_cdrom_handle)
		state = transport::STOPPED;

	if (state == m_state)
		return;

	m_state = state;

	if (m_state == transport::PLAYING)
	{
		attotime const period = attotime::from_hz(SAMPLE_RATE);
		m_sample_timer->adjust(period, 0, period);
	}
	else
	{
		m_sample_timer->adjust(attotime::never);

		if (!m_sample_cb.isnull())
			m_sample_cb(0, 0);
	}
}

void cd_player_device::play()
{
	set_state(transport::PLAYING);
}

void cd_player_device::pause()
{
	if (m_state != transport::STOPPED)
		set_state(transport::PAUSED);
}

void cd_player_device::stop()
{
	set_state(transport::STOPPED);
	seek_track(m_track);
}

void cd_player_device::previous_track()
{
	if (!m_cdrom_handle)
		return;

	if (m_state != transport::STOPPED && track_elapsed_frames() >= 2 * FRAMES_PER_SECOND)
		seek_track(m_track);
	else
		seek_track(std::max(m_track - 1, 0));
}

void cd_player_device::next_track()
{
	if (!m_cdrom_handle)
		return;

	if (m_track + 1 < track_count())
		seek_track(m_track + 1);
}

void cd_player_device::select_track(int track)
{
	if (m_cdrom_handle && track_count() > 0)
		seek_track(std::clamp(track, 1, track_count()) - 1);
}

TIMER_CALLBACK_MEMBER(cd_player_device::sample_tick)
{
	if (m_lba >= disc_end())
	{
		stop();
		seek_track(0);
		return;
	}

	if (m_cached_lba != s32(m_lba))
	{
		if (m_cdrom_handle->get_track_type(m_track) != cdrom_file::CD_TRACK_AUDIO || !read_data(m_lba, m_sector, cdrom_file::CD_TRACK_AUDIO))
			std::fill(std::begin(m_sector), std::end(m_sector), 0);

		m_cached_lba = m_lba;
	}

	// Red Book samples are stored big-endian
	u8 const *const data = &m_sector[m_sample * 4];
	s16 const left = get_s16be(&data[0]);
	s16 const right = get_s16be(&data[2]);

	if (++m_sample == cdrom_file::MAX_SECTOR_DATA / 4)
	{
		m_sample = 0;
		m_lba++;

		if (m_track + 1 < track_count() && m_lba >= track_start(m_track + 1))
			m_track++;
	}

	if (!m_sample_cb.isnull())
		m_sample_cb(left, right);
}

namespace {

constexpr int SINC_PHASES = 512;

// Hann-windowed sinc, TAPS samples wide, sampled at 1/SINC_PHASES sample steps
float const *sinc_table()
{
	static std::vector<float> const table = [] ()
	{
		int const half = cd_audio_comparator::TAPS / 2;
		std::vector<float> result(cd_audio_comparator::TAPS * SINC_PHASES + 1);
		for (int i = 0; i < int(result.size()); i++)
		{
			double const d = double(i) / SINC_PHASES - half;
			double const sinc = (d == 0.0) ? 1.0 : std::sin(M_PI * d) / (M_PI * d);
			result[i] = float(sinc * (0.5 + 0.5 * std::cos(M_PI * d / half)));
		}
		return result;
	}();

	return table.data();
}

} // anonymous namespace

cd_audio_comparator::cd_audio_comparator(s16 hysteresis) :
	m_hysteresis(hysteresis),
	m_state(true)
{
	std::fill(std::begin(m_history), std::end(m_history), 0);
}

bool cd_audio_comparator::update(s16 sample, double &position)
{
	std::copy(std::begin(m_history) + 1, std::end(m_history), std::begin(m_history));
	m_history[TAPS - 1] = sample;

	s16 const last = m_history[TAPS / 2 - 1];
	s16 const current = m_history[TAPS / 2];

	if (m_state ? (current >= -m_hysteresis) : (current <= m_hysteresis))
		return false;

	m_state = !m_state;
	position = 0.0;

	if ((last < 0) != (current < 0))
	{
		float const *const table = sinc_table();
		auto const level = [this, table] (int phase)
		{
			float result = 0.0f;
			for (int i = 0; i < TAPS; i++)
				result += m_history[i] * table[(TAPS - 1 - i) * SINC_PHASES + phase];
			return result;
		};

		int lo = 0, hi = SINC_PHASES;
		bool const lo_negative = last < 0;
		while (hi - lo > 1)
		{
			int const mid = (lo + hi) / 2;
			if ((level(mid) < 0.0f) == lo_negative)
				lo = mid;
			else
				hi = mid;
		}

		position = double(lo + hi) / (2 * SINC_PHASES);
	}

	return true;
}

void cd_audio_comparator::register_save(device_t &device, int index)
{
	device.save_item(NAME(m_state), index);
	device.save_item(NAME(m_history), index);
}
