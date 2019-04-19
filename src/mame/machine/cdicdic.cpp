// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    CD-i Mono-I CDIC MCU simulation
    -------------------

    written by Ryan Holtz


*******************************************************************************

STATUS:

- Just enough for the Mono-I CD-i board to work somewhat properly.

TODO:

- Decapping and proper emulation.

*******************************************************************************/

#include "emu.h"
#include "machine/cdicdic.h"

#include "cdrom.h"
#include "romload.h"
#include "sound/cdda.h"

#define LOG_DECODES		(1 << 0)
#define LOG_SAMPLES		(1 << 1)
#define LOG_COMMANDS	(1 << 2)
#define LOG_SECTORS		(1 << 3)
#define LOG_IRQS		(1 << 4)
#define LOG_READS		(1 << 5)
#define LOG_WRITES		(1 << 6)
#define LOG_UNKNOWNS	(1 << 7)

#define VERBOSE			(0)
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(CDI_CDIC, cdicdic_device, "cdicdic", "CD-i CDIC")


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const int32_t cdicdic_device::s_cdic_adpcm_filter_coef[5][2] =
{
	{ 0,0 },
	{ 60,0 },
	{ 115,-52 },
	{ 98,-55 },
	{ 122,-60 },
};

//**************************************************************************
//  INLINES
//**************************************************************************

int cdicdic_device::is_valid_sample_buf(uint16_t addr) const
{
	const uint8_t *cdram8 = ((uint8_t*)m_ram.get()) + addr + 8;
	if (cdram8[2] != 0xff)
	{
		return 1;
	}
	return 0;
}

double cdicdic_device::sample_buf_freq(uint16_t addr) const
{
	const uint8_t *cdram8 = ((uint8_t*)m_ram.get()) + addr + 8;
	switch (cdram8[2] & 0x3f)
	{
		case 0:
		case 1:
		case 16:
		case 17:
			return clock2() / 512.0f;

		case 4:
		case 5:
			return clock2() / 1024.0f;

		default:
			return clock2() / 1024.0f;
	}
}

int cdicdic_device::sample_buf_size(uint16_t addr) const
{
	const uint8_t *cdram8 = ((uint8_t*)m_ram.get()) + addr + 8;
	switch (cdram8[2] & 0x3f)
	{
		case 0:
		case 4:
			return 4;

		case 1:
		case 5:
		case 16:
			return 2;

		case 17:
			return 1;

		default:
			return 2;
	}
}

static inline int16_t clamp(int16_t in)
{
	return in;
}

//**************************************************************************
//  MEMBER FUNCTIONS
//**************************************************************************

uint32_t cdicdic_device::increment_cdda_frame_bcd(uint32_t bcd)
{
	uint8_t nybbles[6] =
	{
		static_cast<uint8_t>(bcd & 0x0000000f),
		static_cast<uint8_t>((bcd & 0x000000f0) >> 4),
		static_cast<uint8_t>((bcd & 0x00000f00) >> 8),
		static_cast<uint8_t>((bcd & 0x0000f000) >> 12),
		static_cast<uint8_t>((bcd & 0x000f0000) >> 16),
		static_cast<uint8_t>((bcd & 0x00f00000) >> 20)
	};
	nybbles[0]++;
	if (nybbles[0] == 5 && nybbles[1] == 7)
	{
		nybbles[0] = 0;
		nybbles[1] = 0;
		nybbles[2]++;
	}
	else if (nybbles[0] == 10)
	{
		nybbles[1]++;
	}
	if (nybbles[2] == 10)
	{
		nybbles[3]++;
		nybbles[2] = 0;
	}
	if (nybbles[3] == 6)
	{
		nybbles[4]++;
		nybbles[3] = 0;
	}
	if (nybbles[4] == 10)
	{
		nybbles[5]++;
		nybbles[4] = 0;
	}
	return (nybbles[5] << 20) | (nybbles[4] << 16) | (nybbles[3] << 12) | (nybbles[2] << 8) | (nybbles[1] << 4) | nybbles[0];
}

uint32_t cdicdic_device::increment_cdda_sector_bcd(uint32_t bcd)
{
	uint8_t nybbles[6] =
	{
		static_cast<uint8_t>(bcd & 0x0000000f),
		static_cast<uint8_t>((bcd & 0x000000f0) >> 4),
		static_cast<uint8_t>((bcd & 0x00000f00) >> 8),
		static_cast<uint8_t>((bcd & 0x0000f000) >> 12),
		static_cast<uint8_t>((bcd & 0x000f0000) >> 16),
		static_cast<uint8_t>((bcd & 0x00f00000) >> 20)
	};
	nybbles[2]++;
	if (nybbles[2] == 10)
	{
		nybbles[3]++;
		nybbles[2] = 0;
	}
	if (nybbles[3] == 6)
	{
		nybbles[4]++;
		nybbles[3] = 0;
	}
	if (nybbles[4] == 10)
	{
		nybbles[5]++;
		nybbles[4] = 0;
	}
	return (nybbles[5] << 20) | (nybbles[4] << 16) | (nybbles[3] << 12) | (nybbles[2] << 8) | (nybbles[1] << 4) | nybbles[0];
}

void cdicdic_device::decode_xa_mono(int32_t *cdic_xa_last, const uint8_t *xa, int16_t *dp)
{
	int32_t l0 = cdic_xa_last[0];
	int32_t l1 = cdic_xa_last[1];

	for (int32_t b = 0; b < 18; b++)
	{
		for (int32_t s = 0; s < 4; s++)
		{
			uint8_t flags = xa[(4 + (s << 1)) ^ 1];
			uint8_t shift = flags & 0xf;
			uint8_t filter = flags >> 4;
			int32_t f0 = s_cdic_adpcm_filter_coef[filter][0];
			int32_t f1 = s_cdic_adpcm_filter_coef[filter][1];

			for (int32_t i = 0; i < 28; i++)
			{
				int16_t d = (xa[(16 + (i << 2) + s) ^ 1] & 0xf) << 12;
				d = clamp((d >> shift) + (((l0 * f0) + (l1 * f1) + 32) >> 6));
				*dp = d;
				dp++;
				l1 = l0;
				l0 = d;
			}

			flags = xa[(5 + (s << 1)) ^ 1];
			shift = flags & 0xf;
			filter = flags >> 4;
			f0 = s_cdic_adpcm_filter_coef[filter][0];
			f1 = s_cdic_adpcm_filter_coef[filter][1];

			for (int32_t i = 0; i < 28; i++)
			{
				int16_t d = (xa[(16 + (i << 2) + s) ^ 1] >> 4) << 12;
				d = clamp((d >> shift) + (((l0 * f0) + (l1 * f1) + 32) >> 6));
				*dp = d;
				dp++;
				l1 = l0;
				l0 = d;
			}
		}

		xa += 128;
	}

	cdic_xa_last[0] = l0;
	cdic_xa_last[1] = l1;
}

void cdicdic_device::decode_xa_mono8(int *cdic_xa_last, const unsigned char *xa, signed short *dp)
{
	int32_t l0 = cdic_xa_last[0];
	int32_t l1 = cdic_xa_last[1];

	for (int32_t b = 0; b < 18; b++)
	{
		for (int32_t s = 0; s < 4; s++)
		{
			uint8_t flags = xa[(4 + s) ^ 1];
			uint8_t shift = flags & 0xf;
			uint8_t filter = flags >> 4;
			int32_t f0 = s_cdic_adpcm_filter_coef[filter][0];
			int32_t f1 = s_cdic_adpcm_filter_coef[filter][1];

			for (int32_t i = 0; i < 28; i++)
			{
				int16_t d = (xa[(16 + (i << 2) + s) ^ 1] << 8);
				d = clamp((d >> shift) + (((l0 * f0) + (l1 * f1) + 32) >> 6));
				*dp = d;
				dp++;
				l1 = l0;
				l0 = d;
			}
		}

		xa += 128;
	}

	cdic_xa_last[0] = l0;
	cdic_xa_last[1] = l1;
}

void cdicdic_device::decode_xa_stereo(int32_t *cdic_xa_last, const uint8_t *xa, int16_t *dp)
{
	int32_t l0 = cdic_xa_last[0];
	int32_t l1 = cdic_xa_last[1];
	int32_t l2 = cdic_xa_last[2];
	int32_t l3 = cdic_xa_last[3];

	for (int32_t b = 0; b < 18; b++)
	{
		for (int32_t s = 0; s < 4; s++)
		{
			uint8_t flags0 = xa[(4 + (s << 1)) ^ 1];
			uint8_t shift0 = flags0 & 0xf;
			uint8_t filter0 = flags0 >> 4;
			uint8_t flags1 = xa[(5 + (s << 1)) ^ 1];
			uint8_t shift1 = flags1 & 0xf;
			uint8_t filter1 = flags1 >> 4;

			int32_t f0 = s_cdic_adpcm_filter_coef[filter0][0];
			int32_t f1 = s_cdic_adpcm_filter_coef[filter0][1];
			int32_t f2 = s_cdic_adpcm_filter_coef[filter1][0];
			int32_t f3 = s_cdic_adpcm_filter_coef[filter1][1];

			for (int32_t i = 0; i < 28; i++)
			{
				int16_t d = xa[(16 + (i << 2) + s) ^ 1];
				int16_t d0 = (d & 0xf) << 12;
				int16_t d1 = (d >> 4) << 12;
				d0 = clamp((d0 >> shift0) + (((l0 * f0) + (l1 * f1) + 32) >> 6));
				*dp = d0;
				dp++;
				l1 = l0;
				l0 = d0;

				d1 = clamp((d1 >> shift1) + (((l2 * f2) + (l3 * f3) + 32) >> 6));
				*dp = d1;
				dp++;
				l3 = l2;
				l2 = d1;
			}
		}

		xa += 128;
	}

	cdic_xa_last[0] = l0;
	cdic_xa_last[1] = l1;
	cdic_xa_last[2] = l2;
	cdic_xa_last[3] = l3;
}

void cdicdic_device::decode_xa_stereo8(int32_t *cdic_xa_last, const uint8_t *xa, int16_t *dp)
{
	int32_t l0 = cdic_xa_last[0];
	int32_t l1 = cdic_xa_last[1];
	int32_t l2 = cdic_xa_last[2];
	int32_t l3 = cdic_xa_last[3];

	for (int32_t b = 0; b < 18; b++)
	{
		for (int32_t s = 0; s < 4; s += 2)
		{
			uint8_t flags0 = xa[(4 + s) ^ 1];
			uint8_t shift0 = flags0 & 0xf;
			uint8_t filter0 = flags0 >> 4;
			uint8_t flags1 = xa[(5 + s) ^ 1];
			uint8_t shift1 = flags1 & 0xf;
			uint8_t filter1 = flags1 >> 4;
			int32_t f0 = s_cdic_adpcm_filter_coef[filter0][0];
			int32_t f1 = s_cdic_adpcm_filter_coef[filter0][1];
			int32_t f2 = s_cdic_adpcm_filter_coef[filter1][0];
			int32_t f3 = s_cdic_adpcm_filter_coef[filter1][1];

			for (int32_t i = 0; i < 28; i++)
			{
				int16_t d0 = (xa[(16 + (i << 2) + s + 0) ^ 1] << 8);
				int16_t d1 = (xa[(16 + (i << 2) + s + 1) ^ 1] << 8);

				d0 = clamp((d0 >> shift0) + (((l0 * f0) + (l1 * f1) + 32) >> 6));
				*dp = d0;
				dp++;
				l1 = l0;
				l0 = d0;

				d1 = clamp((d1 >> shift1) + (((l2 * f2) + (l3 * f3) + 32) >> 6));
				*dp = d1;
				dp++;
				l3 = l2;
				l2 = d1;
			}
		}

		xa += 128;
	}

	cdic_xa_last[0] = l0;
	cdic_xa_last[1] = l1;
	cdic_xa_last[2] = l2;
	cdic_xa_last[3] = l3;
}

void cdicdic_device::decode_audio_sector(const uint8_t *xa, int32_t triggered)
{
	// Get XA format from sector header
	const uint8_t *hdr = xa + 4;
	int32_t channels;
	int32_t bits = 4;
	int16_t samples[18*28*16+16];

	if (hdr[2] == 0xff && triggered == 1)
	{
		return;
	}

	LOGMASKED(LOG_DECODES, "decode_audio_sector: got header type %02x\n", hdr[2]);

	switch (hdr[2] & 0x3f)   // ignore emphasis and reserved bits
	{
		case 0:
			channels = 1;
			m_audio_sample_freq = clock2() / 512.0f; // / 1024.0f;
			bits = 4;
			m_audio_sample_size = 4;
			break;

		case 1:
			channels = 2;
			m_audio_sample_freq = clock2() / 512.0f;
			bits = 4;
			m_audio_sample_size = 2;
			break;

		case 4:
			channels = 1;
			m_audio_sample_freq = clock2() / 1024.0f;   ///2.0f;
			bits = 4;
			m_audio_sample_size = 4;
			break;

		case 5:
			channels = 2;
			m_audio_sample_freq = clock2() / 1024.0f;   //37800.0f/2.0f;
			bits = 4;
			m_audio_sample_size = 2;
			break;

		case 16:
			channels = 1;
			m_audio_sample_freq = clock2() / 512.0f;
			bits = 8;
			m_audio_sample_size = 2;
			break;

		case 17:
			channels = 2;
			m_audio_sample_freq = clock2() / 512.0f;
			bits = 8;
			m_audio_sample_size = 1;
			break;

		default:
			fatalerror("play_xa: unhandled xa mode %08x\n",hdr[2]);
	}

	m_dmadac[0]->set_frequency(m_audio_sample_freq);
	m_dmadac[0]->enable(1);

	m_dmadac[1]->set_frequency(m_audio_sample_freq);
	m_dmadac[1]->enable(1);

	switch (channels)
	{
		case 1:
			switch (bits)
			{
				case 4:
					decode_xa_mono(m_xa_last, hdr + 4, samples);
					for (int32_t index = 18*28*8 - 1; index >= 0; index--)
					{
						samples[index*2 + 1] = samples[index];
						samples[index*2 + 0] = samples[index];
					}
					samples[18*28*16 + 0] = samples[18*28*16 + 2] = samples[18*28*16 + 4] = samples[18*28*16 + 6] = samples[18*28*16 + 8] = samples[18*28*16 + 10] = samples[18*28*16 + 12] = samples[18*28*16 + 14] = samples[18*28*16 - 2];
					samples[18*28*16 + 1] = samples[18*28*16 + 3] = samples[18*28*16 + 5] = samples[18*28*16 + 7] = samples[18*28*16 + 9] = samples[18*28*16 + 11] = samples[18*28*16 + 13] = samples[18*28*16 + 15] = samples[18*28*16 - 1];
					break;
				case 8:
					decode_xa_mono8(m_xa_last, hdr + 4, samples);
					for (int32_t index = 18*28*8 - 1; index >= 0; index--)
					{
						samples[index*2 + 1] = samples[index];
						samples[index*2 + 0] = samples[index];
					}
					samples[18*28*8 + 0] = samples[18*28*8 + 2] = samples[18*28*8 + 4] = samples[18*28*8 + 6] = samples[18*28*8 + 8] = samples[18*28*8 + 10] = samples[18*28*8 + 12] = samples[18*28*8 + 14] = samples[18*28*8 - 2];
					samples[18*28*8 + 1] = samples[18*28*8 + 3] = samples[18*28*8 + 5] = samples[18*28*8 + 7] = samples[18*28*8 + 9] = samples[18*28*8 + 11] = samples[18*28*8 + 13] = samples[18*28*8 + 15] = samples[18*28*8 - 1];
					break;
			}
			break;
		case 2:
			switch (bits)
			{
				case 4:
					decode_xa_stereo(m_xa_last, hdr + 4, samples);
					samples[18*28*8 + 0] = samples[18*28*8 + 2] = samples[18*28*8 + 4] = samples[18*28*8 + 6] = samples[18*28*8 + 8] = samples[18*28*8 + 10] = samples[18*28*8 + 12] = samples[18*28*8 + 14] = samples[18*28*8 - 2];
					samples[18*28*8 + 1] = samples[18*28*8 + 3] = samples[18*28*8 + 5] = samples[18*28*8 + 7] = samples[18*28*8 + 9] = samples[18*28*8 + 11] = samples[18*28*8 + 13] = samples[18*28*8 + 15] = samples[18*28*8 - 1];
					//fwrite(samples, 1, 18*28*4*m_audio_sample_size, temp_adpcm);
					break;
				case 8:
					decode_xa_stereo8(m_xa_last, hdr + 4, samples);
					samples[18*28*4 + 0] = samples[18*28*4 + 2] = samples[18*28*4 + 4] = samples[18*28*4 + 6] = samples[18*28*4 + 8] = samples[18*28*4 + 10] = samples[18*28*4 + 12] = samples[18*28*4 + 14] = samples[18*28*4 - 2];
					samples[18*28*4 + 1] = samples[18*28*4 + 3] = samples[18*28*4 + 5] = samples[18*28*4 + 7] = samples[18*28*4 + 9] = samples[18*28*4 + 11] = samples[18*28*4 + 13] = samples[18*28*4 + 15] = samples[18*28*4 - 1];
					break;
			}
			break;
	}

	m_dmadac[0]->flush();
	m_dmadac[1]->flush();

	m_dmadac[0]->transfer(0, 1, 2, 18*28*2*m_audio_sample_size, samples);
	m_dmadac[1]->transfer(1, 1, 2, 18*28*2*m_audio_sample_size, samples);
}

// After an appropriate delay for decoding to take place...
TIMER_CALLBACK_MEMBER( cdicdic_device::audio_sample_trigger )
{
	sample_trigger();
}

void cdicdic_device::sample_trigger()
{
	if (m_decode_addr == 0xffff)
	{
		LOGMASKED(LOG_SAMPLES, "Decode stop requested, stopping playback\n");
		m_audio_sample_timer->adjust(attotime::never);
		return;
	}

	if (!m_decode_delay)
	{
		// Indicate that data has been decoded
		LOGMASKED(LOG_SAMPLES, "Flagging that audio data has been decoded\n");
		m_audio_buffer |= 0x8000;

		// Set the CDIC interrupt line
		LOGMASKED(LOG_SAMPLES, "Setting CDIC interrupt line for soundmap decode\n");
		m_intreq_callback(ASSERT_LINE);
	}
	else
	{
		m_decode_delay = 0;
	}

	if (is_valid_sample_buf(m_decode_addr & 0x3ffe))
	{
		LOGMASKED(LOG_SAMPLES, "Hit audio_sample_trigger, with m_decode_addr == %04x, calling decode_audio_sector\n", m_decode_addr);

		// Decode the data at Z+4, the same offset as a normal CD sector.
		decode_audio_sector(((uint8_t*)m_ram.get()) + (m_decode_addr & 0x3ffe) + 4, 1);

		// Swap buffer positions to indicate our new buffer position at the next read
		m_decode_addr ^= 0x1a00;

		LOGMASKED(LOG_SAMPLES, "Updated m_decode_addr, new value is %04x\n", m_decode_addr);

		//// Delay for Frequency * (18*28*2*size in bytes) before requesting more data
		LOGMASKED(LOG_SAMPLES, "Data is valid, setting up a new callback\n");
		m_decode_period = attotime::from_hz(sample_buf_freq(m_decode_addr & 0x3ffe)) * (18*28*2*sample_buf_size(m_decode_addr & 0x3ffe));
		m_audio_sample_timer->adjust(m_decode_period);
		//dmadac_enable(&dmadac[0], 2, 0);
	}
	else
	{
		// Swap buffer positions to indicate our new buffer position at the next read
		m_decode_addr ^= 0x1a00;

		LOGMASKED(LOG_SAMPLES, "Data is not valid, indicating to shut down on the next audio sample\n");
		m_decode_addr = 0xffff;
		m_audio_sample_timer->adjust(m_decode_period);
	}
}

TIMER_CALLBACK_MEMBER( cdicdic_device::trigger_readback_int )
{
	process_delayed_command();
}

void cdicdic_device::process_delayed_command()
{
	switch (m_command)
	{
		case 0x23: // Reset Mode 1
		case 0x24: // Reset Mode 2
		case 0x29: // Read Mode 1
		case 0x2a: // Read Mode 2
		{
			static const char* const s_cmds[8] =
			{
				"Reset Mode 1",
				"Reset Mode 2", 0, 0, 0, 0,
				"Read Mode 1",
				"Read Mode 2"
			};
			LOGMASKED(LOG_COMMANDS, "Command: %s\n", s_cmds[m_command - 0x23]);
			uint8_t buffer[2560] = { 0 };
			uint32_t msf = m_time >> 8;
			uint8_t nybbles[6] =
			{
				static_cast<uint8_t>(msf & 0x0000000f),
				static_cast<uint8_t>((msf & 0x000000f0) >> 4),
				static_cast<uint8_t>((msf & 0x00000f00) >> 8),
				static_cast<uint8_t>((msf & 0x0000f000) >> 12),
				static_cast<uint8_t>((msf & 0x000f0000) >> 16),
				static_cast<uint8_t>((msf & 0x00f00000) >> 20)
			};
			if (msf & 0x000080)
			{
				msf &= 0xffff00;
				nybbles[0] = 0;
				nybbles[1] = 0;
			}
			if (nybbles[2] >= 2)
			{
				nybbles[2] -= 2;
			}
			else
			{
				nybbles[2] = 8 + nybbles[2];
				if (nybbles[3] > 0)
				{
					nybbles[3]--;
				}
				else
				{
					nybbles[3] = 5;
					if (nybbles[4] > 0)
					{
						nybbles[4]--;
					}
					else
					{
						nybbles[4] = 9;
						nybbles[5]--;
					}
				}
			}
			uint32_t lba = nybbles[0] + nybbles[1]*10 + ((nybbles[2] + nybbles[3]*10)*75) + ((nybbles[4] + nybbles[5]*10)*75*60);

			LOGMASKED(LOG_COMMANDS, "Reading Mode %d sector from MSF location %06x\n", m_command - 0x28, m_time | 2);

			cdrom_read_data(m_cd, lba, buffer, CD_TRACK_RAW_DONTCARE);

			m_time += 0x100;
			if ((m_time & 0x00000f00) == 0x00000a00)
			{
				m_time &= 0xfffff0ff;
				m_time += 0x00001000;
			}
			if ((m_time & 0x0000ff00) == 0x00007500)
			{
				m_time &= 0xffff00ff;
				m_time += 0x00010000;
				if ((m_time & 0x000f0000) == 0x000a0000)
				{
					m_time &= 0xfff0ffff;
					m_time += 0x00100000;
				}
			}
			if ((m_time & 0x00ff0000) == 0x00600000)
			{
				m_time &= 0xff00ffff;
				m_time += 0x01000000;
				if ((m_time & 0x0f000000) == 0x0a000000)
				{
					m_time &= 0xf0ffffff;
					m_time += 0x10000000;
				}
			}

			m_data_buffer &= ~0x0004;
			m_data_buffer ^= 0x0001;

			if ((buffer[CDIC_SECTOR_FILE2] << 8) == m_file)
			{
				if (((buffer[CDIC_SECTOR_SUBMODE2] & (CDIC_SUBMODE_FORM | CDIC_SUBMODE_DATA | CDIC_SUBMODE_AUDIO | CDIC_SUBMODE_VIDEO)) == (CDIC_SUBMODE_FORM | CDIC_SUBMODE_AUDIO)) &&
					(m_channel & m_audio_channel & (1 << buffer[CDIC_SECTOR_CHAN2])))
				{
					LOGMASKED(LOG_SECTORS, "Audio sector\n");

					m_x_buffer |= 0x8000;
					//m_data_buffer |= 0x4000;
					m_data_buffer |= 0x0004;

					for (int index = 6; index < 2352/2; index++)
					{
						m_ram[(m_data_buffer & 5) * (0xa00/2) + (index - 6)] = (buffer[index*2] << 8) | buffer[index*2 + 1];
					}

					decode_audio_sector(((uint8_t*)m_ram.get()) + ((m_data_buffer & 5) * 0xa00 + 4), 0);

					LOGMASKED(LOG_IRQS, "Setting CDIC interrupt line for audio sector\n");
					m_intreq_callback(ASSERT_LINE);
				}
				else if ((buffer[CDIC_SECTOR_SUBMODE2] & (CDIC_SUBMODE_DATA | CDIC_SUBMODE_AUDIO | CDIC_SUBMODE_VIDEO)) == 0x00)
				{
					m_x_buffer |= 0x8000;
					//m_data_buffer |= 0x4000;

					for (int index = 6; index < 2352/2; index++)
					{
						m_ram[(m_data_buffer & 5) * (0xa00/2) + (index - 6)] = (buffer[index*2] << 8) | buffer[index*2 + 1];
					}

					if ((buffer[CDIC_SECTOR_SUBMODE2] & CDIC_SUBMODE_TRIG) == CDIC_SUBMODE_TRIG ||
						(buffer[CDIC_SECTOR_SUBMODE2] & CDIC_SUBMODE_EOR) == CDIC_SUBMODE_EOR ||
						(buffer[CDIC_SECTOR_SUBMODE2] & CDIC_SUBMODE_EOF) == CDIC_SUBMODE_EOF)
					{
						LOGMASKED(LOG_IRQS, "Setting CDIC interrupt line for message sector\n");
						m_intreq_callback(ASSERT_LINE);
					}
					else
					{
						LOGMASKED(LOG_SECTORS, "Message sector, ignored\n");
					}
				}
				else
				{
					m_x_buffer |= 0x8000;
					//m_data_buffer |= 0x4000;

					for (int index = 6; index < 2352/2; index++)
					{
						m_ram[(m_data_buffer & 5) * (0xa00/2) + (index - 6)] = (buffer[index*2] << 8) | buffer[index*2 + 1];
					}

					LOGMASKED(LOG_IRQS, "Setting CDIC interrupt line for data sector\n");
					m_intreq_callback(ASSERT_LINE);
				}

				if ((buffer[CDIC_SECTOR_SUBMODE2] & CDIC_SUBMODE_EOF) == 0 && m_command != 0x23)
				{
					m_interrupt_timer->adjust(attotime::from_hz(75)); // 75Hz = 1x CD-ROM speed
				}
				else if (m_command == 0x23) // Mode 1 Reset
				{
					m_interrupt_timer->adjust(attotime::never);
				}
			}
			break;
		}

		case 0x2e: // Abort
			LOGMASKED(LOG_COMMANDS, "Command: Abort\n");
			m_interrupt_timer->adjust(attotime::never);
			//m_data_buffer &= ~4;
			break;

		case 0x28: // Play CDDA audio
		{
			LOGMASKED(LOG_COMMANDS, "Command: Play CDDA Audio\n");
			uint8_t buffer[2560] = { 0 };
			uint32_t msf = (m_time & 0xffff7f00) >> 8;
			uint32_t next_msf = increment_cdda_frame_bcd((m_time & 0xffff7f00) >> 8);
			uint32_t rounded_next_msf = increment_cdda_sector_bcd((m_time & 0xffff0000) >> 8);
			uint8_t nybbles[6] =
			{
				static_cast<uint8_t>(msf & 0x0000000f),
				static_cast<uint8_t>((msf & 0x000000f0) >> 4),
				static_cast<uint8_t>((msf & 0x00000f00) >> 8),
				static_cast<uint8_t>((msf & 0x0000f000) >> 12),
				static_cast<uint8_t>((msf & 0x000f0000) >> 16),
				static_cast<uint8_t>((msf & 0x00f00000) >> 20)
			};

			uint32_t lba = nybbles[0] + nybbles[1]*10 + ((nybbles[2] + nybbles[3]*10)*75) + ((nybbles[4] + nybbles[5]*10)*75*60);

			if (!cdrom_read_data(m_cd, lba, buffer, CD_TRACK_RAW_DONTCARE))
			{
				osd_printf_verbose("Unable to read CD-ROM data.\n");
			}

			if (!(msf & 0x0000ff))
			{
				LOGMASKED(LOG_COMMANDS, "Playing CDDA sector from MSF location %06x\n", m_time | 2);
				m_cdda->start_audio(lba, rounded_next_msf);
			}

			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x924/2] = 0x0001;                      //  CTRL
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x926/2] = 0x0001;                      //  TRACK
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x928/2] = 0x0000;                      //  INDEX
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x92a/2] = (m_time >> 24) & 0x000000ff; //  MIN
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x92c/2] = (m_time >> 16) & 0x000000ff; //  SEC
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x92e/2] = (m_time >>  8) & 0x0000007f; //  FRAC
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x930/2] = 0x0000;                      //  ZERO
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x932/2] = (m_time >> 24) & 0x000000ff; //  AMIN
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x934/2] = (m_time >> 16) & 0x000000ff; //  ASEC
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x936/2] = (m_time >>  8) & 0x0000007f; //  AFRAC
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x938/2] = 0x0000;                      //  CRC1
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x93a/2] = 0x0000;                      //  CRC2

			m_time = next_msf << 8;

			// the following line BREAKS 'The Apprentice', hangs when you attempt to start the game
			//m_interrupt_timer->adjust(attotime::from_hz(75));

			m_x_buffer |= 0x8000;
			//m_data_buffer |= 0x4000;

			for (int index = 6; index < 2352/2; index++)
			{
				m_ram[(m_data_buffer & 5) * (0xa00/2) + (index - 6)] = (buffer[index*2] << 8) | buffer[index*2 + 1];
			}

			LOGMASKED(LOG_IRQS, "Setting CDIC interrupt line for CDDA sector\n");
			m_intreq_callback(ASSERT_LINE);
			break;
		}
		case 0x2c: // Seek
		{
			LOGMASKED(LOG_COMMANDS, "Command: Seek\n");
			uint8_t buffer[2560] = { 0 };
			uint32_t msf = (m_time & 0xffff7f00) >> 8;
			uint32_t next_msf = increment_cdda_frame_bcd((m_time & 0xffff7f00) >> 8);
			uint8_t nybbles[6] =
			{
				static_cast<uint8_t>(msf & 0x0000000f),
				static_cast<uint8_t>((msf & 0x000000f0) >> 4),
				static_cast<uint8_t>((msf & 0x00000f00) >> 8),
				static_cast<uint8_t>((msf & 0x0000f000) >> 12),
				static_cast<uint8_t>((msf & 0x000f0000) >> 16),
				static_cast<uint8_t>((msf & 0x00f00000) >> 20)
			};
			uint32_t lba = nybbles[0] + nybbles[1]*10 + ((nybbles[2] + nybbles[3]*10)*75) + ((nybbles[4] + nybbles[5]*10)*75*60);

			m_interrupt_timer->adjust(attotime::from_hz(75));

			cdrom_read_data(m_cd, lba, buffer, CD_TRACK_RAW_DONTCARE);

			m_data_buffer ^= 0x0001;
			m_x_buffer |= 0x8000;
			m_data_buffer |= 0x4000;

			for (int index = 6; index < 2352/2; index++)
			{
				m_ram[(m_data_buffer & 5) * (0xa00/2) + (index - 6)] = (buffer[index*2] << 8) | buffer[index*2 + 1];
			}

			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x924/2] = 0x0041;                      //  CTRL
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x926/2] = 0x0001;                      //  TRACK
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x928/2] = 0x0000;                      //  INDEX
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x92a/2] = (m_time >> 24) & 0x000000ff; //  MIN
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x92c/2] = (m_time >> 16) & 0x000000ff; //  SEC
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x92e/2] = (m_time >>  8) & 0x0000007f; //  FRAC
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x930/2] = 0x0000;                      //  ZERO
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x932/2] = (m_time >> 24) & 0x000000ff; //  AMIN
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x934/2] = (m_time >> 16) & 0x000000ff; //  ASEC
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x936/2] = (m_time >>  8) & 0x0000007f; //  AFRAC
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x938/2] = 0x0000;                      //  CRC1
			m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x93a/2] = 0x0000;                      //  CRC2

			m_time = next_msf << 8;

			LOGMASKED(LOG_IRQS, "Setting CDIC interrupt line for Seek sector\n");
			m_intreq_callback(ASSERT_LINE);
			break;
		}
	}
}

READ16_MEMBER( cdicdic_device::regs_r )
{
	uint32_t addr = offset + 0x3c00/2;

	switch (addr)
	{
		case 0x3c00/2: // Command register
			LOGMASKED(LOG_READS, "cdic_r: Command Register = %04x & %04x\n", m_command, mem_mask);
			return m_command;

		case 0x3c02/2: // Time register (MSW)
			LOGMASKED(LOG_READS, "cdic_r: Time Register (MSW) = %04x & %04x\n", m_time >> 16, mem_mask);
			return m_time >> 16;

		case 0x3c04/2: // Time register (LSW)
			LOGMASKED(LOG_READS, "cdic_r: Time Register (LSW) = %04x & %04x\n", (uint16_t)(m_time & 0x0000ffff), mem_mask);
			return m_time & 0x0000ffff;

		case 0x3c06/2: // File register
			LOGMASKED(LOG_READS, "cdic_r: File Register = %04x & %04x\n", m_file, mem_mask);
			return m_file;

		case 0x3c08/2: // Channel register (MSW)
			LOGMASKED(LOG_READS, "cdic_r: Channel Register (MSW) = %04x & %04x\n", m_channel >> 16, mem_mask);
			return m_channel >> 16;

		case 0x3c0a/2: // Channel register (LSW)
			LOGMASKED(LOG_READS, "cdic_r: Channel Register (LSW) = %04x & %04x\n", m_channel & 0x0000ffff, mem_mask);
			return m_channel & 0x0000ffff;

		case 0x3c0c/2: // Audio Channel register
			LOGMASKED(LOG_READS, "cdic_r: Audio Channel Register = %04x & %04x\n", m_audio_channel, mem_mask);
			return m_audio_channel;

		case 0x3ff4/2: // ABUF
		{
			uint16_t temp = m_audio_buffer;
			LOGMASKED(LOG_READS, "cdic_r: Audio Buffer Register = %04x & %04x\n", temp, mem_mask);
			m_audio_buffer &= 0x7fff;
			if (!((m_audio_buffer | m_x_buffer) & 0x8000))
			{
				m_intreq_callback(CLEAR_LINE);
				LOGMASKED(LOG_IRQS, "Clearing CDIC interrupt line\n");
			}
			return temp;
		}

		case 0x3ff6/2: // XBUF
		{
			uint16_t temp = m_x_buffer;
			LOGMASKED(LOG_READS, "cdic_r: X-Buffer Register = %04x & %04x\n", temp, mem_mask);
			m_x_buffer &= 0x7fff;
			if (!((m_audio_buffer | m_x_buffer) & 0x8000))
			{
				m_intreq_callback(CLEAR_LINE);
				LOGMASKED(LOG_IRQS, "Clearing CDIC interrupt line\n");
			}
			return temp;
		}

		case 0x3ffa/2: // AUDCTL
		{
			LOGMASKED(LOG_READS, "cdic_r: Z-Buffer Register = %04x & %04x\n", m_z_buffer, mem_mask);
			if (m_audio_sample_timer->remaining().is_never())
			{
				m_z_buffer ^= 0x0001;
			}
			return m_z_buffer;
		}

		case 0x3ffe/2:
		{
			LOGMASKED(LOG_READS, "cdic_r: Data buffer Register = %04x & %04x\n", m_data_buffer, mem_mask);
			return m_data_buffer;
		}
		default:
			LOGMASKED(LOG_READS | LOG_UNKNOWNS, "cdic_r: Unknown address: %04x & %04x\n", addr*2, mem_mask);
			return 0;
	}
}

WRITE16_MEMBER( cdicdic_device::regs_w )
{
	uint32_t addr = offset + 0x3c00/2;

	switch (addr)
	{
		case 0x3c00/2: // Command register
			LOGMASKED(LOG_WRITES, "cdic_w: Command Register = %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_command);
			break;

		case 0x3c02/2: // Time register (MSW)
			m_time &= ~(mem_mask << 16);
			m_time |= (data & mem_mask) << 16;
			LOGMASKED(LOG_WRITES, "cdic_w: Time Register (MSW) = %04x & %04x\n", data, mem_mask);
			break;

		case 0x3c04/2: // Time register (LSW)
			m_time &= ~mem_mask;
			m_time |= data & mem_mask;
			LOGMASKED(LOG_WRITES, "cdic_w: Time Register (LSW) = %04x & %04x\n", data, mem_mask);
			break;

		case 0x3c06/2: // File register
			LOGMASKED(LOG_WRITES, "cdic_w: File Register = %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_file);
			break;

		case 0x3c08/2: // Channel register (MSW)
			m_channel &= ~(mem_mask << 16);
			m_channel |= (data & mem_mask) << 16;
			LOGMASKED(LOG_WRITES, "cdic_w: Channel Register (MSW) = %04x & %04x\n", data, mem_mask);
			break;

		case 0x3c0a/2: // Channel register (LSW)
			m_channel &= ~mem_mask;
			m_channel |= data & mem_mask;
			LOGMASKED(LOG_WRITES, "cdic_w: Channel Register (LSW) = %04x & %04x\n", data, mem_mask);
			break;

		case 0x3c0c/2: // Audio Channel register
			LOGMASKED(LOG_WRITES, "cdic_w: Audio Channel Register = %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_audio_channel);
			break;

		case 0x3ff4/2:
			LOGMASKED(LOG_WRITES, "cdic_w: Audio Buffer Register = %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_audio_buffer);
			break;

		case 0x3ff6/2:
			LOGMASKED(LOG_WRITES, "cdic_w: X Buffer Register = %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_x_buffer);
			break;

		case 0x3ff8/2:
		{
			uint32_t start = m_scc->dma().channel[0].memory_address_counter;
			uint32_t count = m_scc->dma().channel[0].transfer_counter;
			uint32_t device_index = (data & 0x3fff) >> 1;
			LOGMASKED(LOG_WRITES, "cdic_w: DMA Control Register = %04x & %04x\n", data, mem_mask);
			LOGMASKED(LOG_WRITES, "Memory address counter: %08x\n", m_scc->dma().channel[0].memory_address_counter);
			LOGMASKED(LOG_WRITES, "Doing copy, transferring %04x bytes\n", count * 2 );
			for (uint32_t index = start / 2; index < (start / 2 + count); index++)
			{
				if (m_scc->dma().channel[0].operation_control & OCR_D)
				{
					m_memory_space->write_word(index * 2, m_ram[device_index++]);
				}
				else
				{
					m_ram[device_index++] = m_memory_space->read_word(index * 2);
				}
			}
			m_scc->dma().channel[0].memory_address_counter += m_scc->dma().channel[0].transfer_counter * 2;
			break;
		}

		case 0x3ffa/2:
		{
			LOGMASKED(LOG_WRITES, "cdic_w: Z-Buffer Register = %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_z_buffer);
			if (m_z_buffer & 0x2000)
			{
				if (m_audio_sample_timer->remaining().is_never())
				{
					m_decode_addr = m_z_buffer & 0x3a00;
					m_decode_delay = 1;
					m_audio_sample_timer->adjust(attotime::from_hz(75));
				}
			}
			else
			{
				m_decode_addr = 0xffff;
				m_audio_sample_timer->adjust(attotime::never);
			}
			break;
		}

		case 0x3ffc/2:
			LOGMASKED(LOG_WRITES, "cdic_w: Interrupt Vector Register = %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_interrupt_vector);
			break;

		case 0x3ffe/2:
		{
			LOGMASKED(LOG_WRITES, "cdic_w: Data Buffer Register = %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_data_buffer);
			if (m_data_buffer & 0x8000)
			{
				switch (m_command)
				{
					//case 0x24: // Reset Mode 2
					case 0x2e: // Abort
					{
						m_interrupt_timer->adjust(attotime::never);
						m_dmadac[0]->enable(0);
						m_dmadac[1]->enable(0);
						//m_data_buffer &= 0xbfff;
						break;
					}
					case 0x2b: // Stop CDDA
						m_cdda->stop_audio();
						m_interrupt_timer->adjust(attotime::never);
						break;
					case 0x23: // Reset Mode 1
					case 0x29: // Read Mode 1
					case 0x2a: // Read Mode 2
					case 0x28: // Play CDDA
					case 0x2c: // Seek
					{
						attotime period = m_interrupt_timer->remaining();
						if (!period.is_never())
						{
							m_interrupt_timer->adjust(period);
						}
						else
						{
							if (m_command != 0x23 && m_command != 0x24)
							{
								m_interrupt_timer->adjust(attotime::from_hz(75));
							}
						}
						break;
					}
					default:
						LOGMASKED(LOG_COMMANDS, "Unknown CDIC command: %02x\n", m_command );
						break;
				}
			}
			m_data_buffer &= 0x7fff;
			break;
		}
		default:
			LOGMASKED(LOG_WRITES | LOG_UNKNOWNS, "cdic_w: Unknown address: %04x = %04x & %04x\n", addr*2, data, mem_mask);
			break;
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdicdic_device - constructor
//-------------------------------------------------

cdicdic_device::cdicdic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CDI_CDIC, tag, owner, clock)
	, m_intreq_callback(*this)
	, m_memory_space(*this, ":maincpu", AS_PROGRAM)
	, m_dmadac(*this, ":dac%u", 1U)
	, m_scc(*this, ":maincpu")
	, m_cdda(*this, ":cdda")
	, m_cdrom_dev(*this, ":cdrom")
	, m_clock2(clock)
{
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void cdicdic_device::device_resolve_objects()
{
	m_intreq_callback.resolve_safe();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdicdic_device::device_start()
{
	save_item(NAME(m_command));
	save_item(NAME(m_time));
	save_item(NAME(m_file));
	save_item(NAME(m_channel));
	save_item(NAME(m_audio_channel));
	save_item(NAME(m_audio_buffer));
	save_item(NAME(m_x_buffer));
	save_item(NAME(m_dma_control));
	save_item(NAME(m_z_buffer));
	save_item(NAME(m_interrupt_vector));
	save_item(NAME(m_data_buffer));

	save_item(NAME(m_audio_sample_freq));
	save_item(NAME(m_audio_sample_size));

	m_interrupt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cdicdic_device::trigger_readback_int), this));
	m_interrupt_timer->adjust(attotime::never);

	m_audio_sample_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cdicdic_device::audio_sample_trigger), this));
	m_audio_sample_timer->adjust(attotime::never);

	m_ram = std::make_unique<uint16_t[]>(0x3c00/2);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cdicdic_device::device_reset()
{
	m_command = 0;
	m_time = 0;
	m_file = 0;
	m_channel = 0xffffffff;
	m_audio_channel = 0xffff;
	m_audio_buffer = 0;
	m_x_buffer = 0;
	m_dma_control = 0;
	m_z_buffer = 0;
	m_interrupt_vector = 0x0f;
	m_data_buffer = 0;

	m_audio_sample_freq = 0;
	m_audio_sample_size = 0;

	m_decode_addr = 0;
	m_decode_delay = 0;

	if (m_cdrom_dev)
	{
		// MESS case (has CDROM device)
		m_cd = m_cdrom_dev->get_cdrom_file();
		m_cdda->set_cdrom(m_cd);
	}
	else
	{
		// MAME case
		m_cd = cdrom_open(machine().rom_load().get_disk_handle(":cdrom"));
		m_cdda->set_cdrom(m_cd);
	}

	m_intreq_callback(CLEAR_LINE);
}

WRITE16_MEMBER( cdicdic_device::ram_w )
{
	COMBINE_DATA(&m_ram[offset]);
}

READ16_MEMBER( cdicdic_device::ram_r )
{
	return m_ram[offset];
}

uint8_t cdicdic_device::intack_r()
{
	return m_interrupt_vector & 0xff;
}
