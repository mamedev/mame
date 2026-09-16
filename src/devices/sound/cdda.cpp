// license:BSD-3-Clause
// copyright-holders:Aaron Giles,smf
/*
    CD-DA "Red Book" audio sound hardware handler
    Relies on the actual CD logic and reading in cdrom.c.
*/

#include "emu.h"
#include "cdda.h"

static constexpr int MAX_SECTORS = 4;
static constexpr int MAX_SCAN_SECTORS = 2;

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void cdda_device::sound_stream_update(sound_stream &stream)
{
	get_audio_data(stream);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdda_device::device_start()
{
	/* allocate an audio cache */
	m_audio_cache = std::make_unique<uint8_t[]>(cdrom_file::MAX_SECTOR_DATA * MAX_SECTORS );

	m_stream = stream_alloc(0, 2, clock());

	m_audio_playing = 0;
	m_audio_pause = 0;
	m_audio_ended_normally = 0;
	m_audio_lba = 0;
	m_audio_length = 0;
	m_audio_samples = 0;
	m_audio_bptr = 0;
	m_sequence_counter = 0;
	m_audio_scan = 0;
	m_audio_scan_direction = 0;

	save_item(NAME(m_audio_playing));
	save_item(NAME(m_audio_pause));
	save_item(NAME(m_audio_ended_normally));
	save_item(NAME(m_audio_lba));
	save_item(NAME(m_audio_length));
	save_pointer(NAME(m_audio_cache), cdrom_file::MAX_SECTOR_DATA * MAX_SECTORS);
	save_item(NAME(m_audio_samples));
	save_item(NAME(m_audio_bptr));
	save_item(NAME(m_sequence_counter));
	save_item(NAME(m_audio_scan));
	save_item(NAME(m_audio_scan_direction));
}


/*-------------------------------------------------
    start_audio - begin playback of a Red
    Book audio track
-------------------------------------------------*/

void cdda_device::start_audio(uint32_t startlba, uint32_t numblocks)
{
	m_stream->update();
	m_audio_playing = true;
	m_audio_pause = false;
	m_audio_ended_normally = false;
	m_audio_lba = startlba;
	m_audio_length = numblocks;
	m_audio_samples = 0;
}


/*-------------------------------------------------
    stop_audio - stop playback of a Red Book
    audio track
-------------------------------------------------*/

void cdda_device::stop_audio()
{
	m_stream->update();
	m_audio_playing = false;
	m_audio_ended_normally = true;
}


/*-------------------------------------------------
    pause_audio - pause/unpause playback of
    a Red Book audio track
-------------------------------------------------*/

void cdda_device::pause_audio(int pause)
{
	m_stream->update();
	m_audio_pause = pause;
}

/*-------------------------------------------------
    scan_forward - begins an audible fast scan
    in the forward direction
-------------------------------------------------*/
void cdda_device::scan_forward()
{
	m_audio_scan = true;
	m_audio_scan_direction = true;
}

/*-------------------------------------------------
    scan_reverse - begins an audible fast scan
    in the backwards direction
-------------------------------------------------*/
void cdda_device::scan_reverse()
{
	m_audio_scan = true;
	m_audio_scan_direction = false;
}

/*-------------------------------------------------
    cancel_scan - stops scan mode and resumes
    normal operation
-------------------------------------------------*/
void cdda_device::cancel_scan()
{
	m_audio_scan = false;
}

/*-------------------------------------------------
    set_audio_lba - sets the current audio LBA
    play position without changing any modes.
-------------------------------------------------*/

void cdda_device::set_audio_lba(uint32_t lba)
{
	m_stream->update();
	m_audio_lba = lba;
}

/*-------------------------------------------------
    get_audio_lba - returns the current LBA
    (physical sector) during Red Book playback
-------------------------------------------------*/

uint32_t cdda_device::get_audio_lba()
{
	m_stream->update();
	return m_audio_lba - ((m_audio_samples + (cdrom_file::MAX_SECTOR_DATA / 4) - 1) / (cdrom_file::MAX_SECTOR_DATA / 4));
}

/*-------------------------------------------------
    set_audio_length - sets the current audio LBA
    play position without changing any modes.
-------------------------------------------------*/

void cdda_device::set_audio_length(uint32_t sectors)
{
	m_audio_length = sectors;
}

/*-------------------------------------------------
    audio_active - returns Red Book audio
    playback status
-------------------------------------------------*/

int cdda_device::audio_active()
{
	m_stream->update();
	return m_audio_playing;
}


/*-------------------------------------------------
    audio_paused - returns if Red Book
    playback is paused
-------------------------------------------------*/

int cdda_device::audio_paused()
{
	return m_audio_pause;
}


/*-------------------------------------------------
    audio_ended - returns if a Red Book
    track reached it's natural end
-------------------------------------------------*/

int cdda_device::audio_ended()
{
	m_stream->update();
	return m_audio_ended_normally;
}


/*-------------------------------------------------
    get_audio_data - reads Red Book data off
    the disc if playback is in progress and
    converts it to 2 16-bit 44.1 kHz streams
-------------------------------------------------*/

void cdda_device::get_audio_data(sound_stream &stream)
{
	int16_t *audio_cache = (int16_t *) m_audio_cache.get();

	for (int sampindex = 0; sampindex < stream.samples(); )
	{
		/* if no file, audio not playing, audio paused, or out of disc data,
		   just zero fill */
		if (m_disc->sequence_counter() != m_sequence_counter || !m_disc->exists() || !m_audio_playing || m_audio_pause || (!m_audio_length && !m_audio_samples))
		{
			if( m_audio_playing && !m_audio_pause && !m_audio_length )
			{
				m_audio_playing = false;
				m_audio_ended_normally = true;
				m_audio_end_cb(ASSERT_LINE);
			}

			m_sequence_counter = m_disc->sequence_counter();
			m_audio_data[0] = m_audio_data[1] = 0;
			return;
		}

		int samples = stream.samples() - sampindex;
		if (samples > m_audio_samples)
		{
			samples = m_audio_samples;
		}

		for (int i = 0; i < samples; i++)
		{
			/* CD-DA data on the disc is big-endian */
			m_audio_data[0] = s16(big_endianize_int16( audio_cache[ m_audio_bptr ] ));
			stream.put_int(0, sampindex + i, m_audio_data[0], 32768);
			m_audio_bptr++;
			m_audio_data[1] = s16(big_endianize_int16( audio_cache[ m_audio_bptr ] ));
			stream.put_int(1, sampindex + i, m_audio_data[1], 32768);
			m_audio_bptr++;
		}

		sampindex += samples;
		m_audio_samples -= samples;

		if (m_audio_samples == 0)
		{
			int sectors = m_audio_length;
			if (sectors > MAX_SECTORS)
			{
				sectors = MAX_SECTORS;
			}

			for (int i = 0; i < sectors; i++)
			{
				const auto adr_control = m_disc->get_adr_control(m_disc->get_track(m_audio_lba));

				// Don't attempt to play data tracks
				if (BIT(adr_control, 2))
				{
					std::fill_n(&m_audio_cache[cdrom_file::MAX_SECTOR_DATA * i], cdrom_file::MAX_SECTOR_DATA, 0);
				}
				else
				{
					m_disc->read_data(m_audio_lba, &m_audio_cache[cdrom_file::MAX_SECTOR_DATA * i], cdrom_file::CD_TRACK_AUDIO);
				}

				if (!m_audio_scan)
				{
					m_audio_lba++;
				}
				else
				{
					if (m_audio_scan_direction)
					{
						m_audio_lba += 2;
					}
					else
					{
						m_audio_lba -= 2;
					}
				}
			}

			m_audio_samples = (cdrom_file::MAX_SECTOR_DATA*sectors)/4;
			m_audio_length -= sectors;

			/* reset feedout ptr */
			m_audio_bptr = 0;
		}
	}
}

/*-------------------------------------------------
    get_channel_sample - reads currently decoded
    data sample on the stream.
    Used by PC Engine CD class family for volume
    metering on audio CD player.
-------------------------------------------------*/

int16_t cdda_device::get_channel_sample(int channel)
{
	m_stream->update();
	return m_audio_data[channel];
}

DEFINE_DEVICE_TYPE(CDDA, cdda_device, "cdda", "CD/DA")

cdda_device::cdda_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CDDA, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_disc(*this, finder_base::DUMMY_TAG)
	, m_stream(nullptr)
	, m_audio_end_cb(*this)
{
}
