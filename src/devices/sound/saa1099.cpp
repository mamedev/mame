// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, Manuel Abadia
/***************************************************************************

    Philips SAA1099 Sound driver

    By Juergen Buchmueller and Manuel Abadia

    SAA1099 register layout:
    ========================

    offs | 7654 3210 | description
    -----+-----------+---------------------------
    0x00 | ---- xxxx | Amplitude channel 0 (left)
    0x00 | xxxx ---- | Amplitude channel 0 (right)
    0x01 | ---- xxxx | Amplitude channel 1 (left)
    0x01 | xxxx ---- | Amplitude channel 1 (right)
    0x02 | ---- xxxx | Amplitude channel 2 (left)
    0x02 | xxxx ---- | Amplitude channel 2 (right)
    0x03 | ---- xxxx | Amplitude channel 3 (left)
    0x03 | xxxx ---- | Amplitude channel 3 (right)
    0x04 | ---- xxxx | Amplitude channel 4 (left)
    0x04 | xxxx ---- | Amplitude channel 4 (right)
    0x05 | ---- xxxx | Amplitude channel 5 (left)
    0x05 | xxxx ---- | Amplitude channel 5 (right)
         |           |
    0x08 | xxxx xxxx | Frequency channel 0
    0x09 | xxxx xxxx | Frequency channel 1
    0x0a | xxxx xxxx | Frequency channel 2
    0x0b | xxxx xxxx | Frequency channel 3
    0x0c | xxxx xxxx | Frequency channel 4
    0x0d | xxxx xxxx | Frequency channel 5
         |           |
    0x10 | ---- -xxx | Channel 0 octave select
    0x10 | -xxx ---- | Channel 1 octave select
    0x11 | ---- -xxx | Channel 2 octave select
    0x11 | -xxx ---- | Channel 3 octave select
    0x12 | ---- -xxx | Channel 4 octave select
    0x12 | -xxx ---- | Channel 5 octave select
         |           |
    0x14 | ---- ---x | Channel 0 frequency enable (0 = off, 1 = on)
    0x14 | ---- --x- | Channel 1 frequency enable (0 = off, 1 = on)
    0x14 | ---- -x-- | Channel 2 frequency enable (0 = off, 1 = on)
    0x14 | ---- x--- | Channel 3 frequency enable (0 = off, 1 = on)
    0x14 | ---x ---- | Channel 4 frequency enable (0 = off, 1 = on)
    0x14 | --x- ---- | Channel 5 frequency enable (0 = off, 1 = on)
         |           |
    0x15 | ---- ---x | Channel 0 noise enable (0 = off, 1 = on)
    0x15 | ---- --x- | Channel 1 noise enable (0 = off, 1 = on)
    0x15 | ---- -x-- | Channel 2 noise enable (0 = off, 1 = on)
    0x15 | ---- x--- | Channel 3 noise enable (0 = off, 1 = on)
    0x15 | ---x ---- | Channel 4 noise enable (0 = off, 1 = on)
    0x15 | --x- ---- | Channel 5 noise enable (0 = off, 1 = on)
         |           |
    0x16 | ---- --xx | Noise generator parameters 0
    0x16 | --xx ---- | Noise generator parameters 1
         |           |
    0x18 | --xx xxxx | Envelope generator 0 parameters
    0x18 | x--- ---- | Envelope generator 0 control enable (0 = off, 1 = on)
    0x19 | --xx xxxx | Envelope generator 1 parameters
    0x19 | x--- ---- | Envelope generator 1 control enable (0 = off, 1 = on)
         |           |
    0x1c | ---- ---x | All channels enable (0 = off, 1 = on)
    0x1c | ---- --x- | Synch & Reset generators

    Version History:
    ================
    ??-??-200? - First version of the driver submitted for MESS (GPL/MESS license)
    ??-??-200? - Submitted to DOSBOX for Creative Music System/Game Blaster emulation
    ??-??-201? - Driver relicensed to BSD 3 Clause (GPL+ compatible)
    06-27-2015 - Applied clock divisor fix from DOSBOX SVN, http://www.vogons.org/viewtopic.php?p=344227#p344227

***************************************************************************/

#include "emu.h"
#include "saa1099.h"

#define LEFT    0x00
#define RIGHT   0x01

static const int amplitude_lookup[16] = {
		0*32767/16,  1*32767/16,  2*32767/16,   3*32767/16,
		4*32767/16,  5*32767/16,  6*32767/16,   7*32767/16,
		8*32767/16,  9*32767/16, 10*32767/16, 11*32767/16,
	12*32767/16, 13*32767/16, 14*32767/16, 15*32767/16
};

static const UINT8 envelope[8][64] = {
	/* zero amplitude */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* maximum amplitude */
	{15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
		15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
		15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
		15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15, },
	/* single decay */
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* repetitive decay */
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
		15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
		15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
		15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 },
	/* single triangular */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
		15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* repetitive triangular */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
		15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
		15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 },
	/* single attack */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* repetitive attack */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15 }
};


// device type definition
const device_type SAA1099 = &device_creator<saa1099_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  saa1099_device - constructor
//-------------------------------------------------

saa1099_device::saa1099_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SAA1099, "SAA1099", tag, owner, clock, "saa1099", __FILE__),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_all_ch_enable(0),
		m_sync_state(0),
		m_selected_reg(0),
		m_sample_rate(0.0)
{
	memset(m_noise_params, 0, sizeof(int)*2);
	memset(m_env_enable, 0, sizeof(int)*2);
	memset(m_env_reverse_right, 0, sizeof(int)*2);
	memset(m_env_mode, 0, sizeof(int)*2);
	memset(m_env_bits, 0, sizeof(int)*2);
	memset(m_env_clock, 0, sizeof(int)*2);
	memset(m_env_step, 0, sizeof(int)*2);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saa1099_device::device_start()
{
	/* copy global parameters */
	m_master_clock = clock();
	m_sample_rate = clock() / 256;

	/* for each chip allocate one stream */
	m_stream = stream_alloc(0, 2, m_sample_rate);

	save_item(NAME(m_noise_params));
	save_item(NAME(m_env_enable));
	save_item(NAME(m_env_reverse_right));
	save_item(NAME(m_env_mode));
	save_item(NAME(m_env_bits));
	save_item(NAME(m_env_clock));
	save_item(NAME(m_env_step));
	save_item(NAME(m_all_ch_enable));
	save_item(NAME(m_sync_state));
	save_item(NAME(m_selected_reg));

	for (int i = 0; i < 6; i++)
	{
		save_item(NAME(m_channels[i].frequency), i);
		save_item(NAME(m_channels[i].freq_enable), i);
		save_item(NAME(m_channels[i].noise_enable), i);
		save_item(NAME(m_channels[i].octave), i);
		save_item(NAME(m_channels[i].amplitude), i);
		save_item(NAME(m_channels[i].envelope), i);
		save_item(NAME(m_channels[i].counter), i);
		save_item(NAME(m_channels[i].freq), i);
		save_item(NAME(m_channels[i].level), i);
	}

	for (int i = 0; i < 2; i++)
	{
		save_item(NAME(m_noise[i].counter), i);
		save_item(NAME(m_noise[i].freq), i);
		save_item(NAME(m_noise[i].level), i);
	}
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void saa1099_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int j, ch;
	/* if the channels are disabled we're done */
	if (!m_all_ch_enable)
	{
		/* init output data */
		memset(outputs[LEFT],0,samples*sizeof(*outputs[LEFT]));
		memset(outputs[RIGHT],0,samples*sizeof(*outputs[RIGHT]));
		return;
	}

	for (ch = 0; ch < 2; ch++)
	{
		switch (m_noise_params[ch])
		{
		case 0: m_noise[ch].freq = m_master_clock/256.0 * 2; break;
		case 1: m_noise[ch].freq = m_master_clock/512.0 * 2; break;
		case 2: m_noise[ch].freq = m_master_clock/1024.0 * 2; break;
		case 3: m_noise[ch].freq = m_channels[ch * 3].freq;   break;
		}
	}

	/* fill all data needed */
	for( j = 0; j < samples; j++ )
	{
		int output_l = 0, output_r = 0;

		/* for each channel */
		for (ch = 0; ch < 6; ch++)
		{
			if (m_channels[ch].freq == 0.0)
				m_channels[ch].freq = (double)((2 * m_master_clock / 512) << m_channels[ch].octave) /
					(511.0 - (double)m_channels[ch].frequency);

			/* check the actual position in the square wave */
			m_channels[ch].counter -= m_channels[ch].freq;
			while (m_channels[ch].counter < 0)
			{
				/* calculate new frequency now after the half wave is updated */
				m_channels[ch].freq = (double)((2 * m_master_clock / 512) << m_channels[ch].octave) /
					(511.0 - (double)m_channels[ch].frequency);

				m_channels[ch].counter += m_sample_rate;
				m_channels[ch].level ^= 1;

				/* eventually clock the envelope counters */
				if (ch == 1 && m_env_clock[0] == 0)
					envelope_w(0);
				if (ch == 4 && m_env_clock[1] == 0)
					envelope_w(1);
			}

			// if the noise is enabled
			if (m_channels[ch].noise_enable)
			{
				// if the noise level is high (noise 0: chan 0-2, noise 1: chan 3-5)
				if (m_noise[ch/3].level & 1)
				{
					// subtract to avoid overflows, also use only half amplitude
					output_l -= m_channels[ch].amplitude[ LEFT] * m_channels[ch].envelope[ LEFT] / 16 / 2;
					output_r -= m_channels[ch].amplitude[RIGHT] * m_channels[ch].envelope[RIGHT] / 16 / 2;
				}
			}
			// if the square wave is enabled
			if (m_channels[ch].freq_enable)
			{
				// if the channel level is high
				if (m_channels[ch].level & 1)
				{
					output_l += m_channels[ch].amplitude[ LEFT] * m_channels[ch].envelope[ LEFT] / 16;
					output_r += m_channels[ch].amplitude[RIGHT] * m_channels[ch].envelope[RIGHT] / 16;
				}
			}
		}

		for (ch = 0; ch < 2; ch++)
		{
			/* check the actual position in noise generator */
			m_noise[ch].counter -= m_noise[ch].freq;
			while (m_noise[ch].counter < 0)
			{
				m_noise[ch].counter += m_sample_rate;
				if( ((m_noise[ch].level & 0x4000) == 0) == ((m_noise[ch].level & 0x0040) == 0) )
					m_noise[ch].level = (m_noise[ch].level << 1) | 1;
				else
					m_noise[ch].level <<= 1;
			}
		}
		/* write sound data to the buffer */
		outputs[LEFT][j] = output_l / 6;
		outputs[RIGHT][j] = output_r / 6;
	}
}


void saa1099_device::envelope_w(int ch)
{
	if (m_env_enable[ch])
	{
		int step, mode, mask;
		mode = m_env_mode[ch];
		/* step from 0..63 and then loop in steps 32..63 */
		step = m_env_step[ch] =
			((m_env_step[ch] + 1) & 0x3f) | (m_env_step[ch] & 0x20);

		mask = 15;
		if (m_env_bits[ch])
			mask &= ~1;     /* 3 bit resolution, mask LSB */

		m_channels[ch*3+0].envelope[ LEFT] =
		m_channels[ch*3+1].envelope[ LEFT] =
		m_channels[ch*3+2].envelope[ LEFT] = envelope[mode][step] & mask;
		if (m_env_reverse_right[ch] & 0x01)
		{
			m_channels[ch*3+0].envelope[RIGHT] =
			m_channels[ch*3+1].envelope[RIGHT] =
			m_channels[ch*3+2].envelope[RIGHT] = (15 - envelope[mode][step]) & mask;
		}
		else
		{
			m_channels[ch*3+0].envelope[RIGHT] =
			m_channels[ch*3+1].envelope[RIGHT] =
			m_channels[ch*3+2].envelope[RIGHT] = envelope[mode][step] & mask;
		}
	}
	else
	{
		/* envelope mode off, set all envelope factors to 16 */
		m_channels[ch*3+0].envelope[ LEFT] =
		m_channels[ch*3+1].envelope[ LEFT] =
		m_channels[ch*3+2].envelope[ LEFT] =
		m_channels[ch*3+0].envelope[RIGHT] =
		m_channels[ch*3+1].envelope[RIGHT] =
		m_channels[ch*3+2].envelope[RIGHT] = 16;
	}
}


WRITE8_MEMBER( saa1099_device::control_w )
{
	if ((data & 0xff) > 0x1c)
	{
		/* Error! */
		logerror("%s: (SAA1099 '%s') Unknown register selected\n", machine().describe_context(), tag());
	}

	m_selected_reg = data & 0x1f;
	if (m_selected_reg == 0x18 || m_selected_reg == 0x19)
	{
		/* clock the envelope channels */
		if (m_env_clock[0])
			envelope_w(0);
		if (m_env_clock[1])
			envelope_w(1);
	}
}


WRITE8_MEMBER( saa1099_device::data_w )
{
	int reg = m_selected_reg;
	int ch;

	/* first update the stream to this point in time */
	m_stream->update();

	switch (reg)
	{
	/* channel i amplitude */
	case 0x00:  case 0x01:  case 0x02:  case 0x03:  case 0x04:  case 0x05:
		ch = reg & 7;
		m_channels[ch].amplitude[LEFT] = amplitude_lookup[data & 0x0f];
		m_channels[ch].amplitude[RIGHT] = amplitude_lookup[(data >> 4) & 0x0f];
		break;
	/* channel i frequency */
	case 0x08:  case 0x09:  case 0x0a:  case 0x0b:  case 0x0c:  case 0x0d:
		ch = reg & 7;
		m_channels[ch].frequency = data & 0xff;
		break;
	/* channel i octave */
	case 0x10:  case 0x11:  case 0x12:
		ch = (reg - 0x10) << 1;
		m_channels[ch + 0].octave = data & 0x07;
		m_channels[ch + 1].octave = (data >> 4) & 0x07;
		break;
	/* channel i frequency enable */
	case 0x14:
		m_channels[0].freq_enable = data & 0x01;
		m_channels[1].freq_enable = data & 0x02;
		m_channels[2].freq_enable = data & 0x04;
		m_channels[3].freq_enable = data & 0x08;
		m_channels[4].freq_enable = data & 0x10;
		m_channels[5].freq_enable = data & 0x20;
		break;
	/* channel i noise enable */
	case 0x15:
		m_channels[0].noise_enable = data & 0x01;
		m_channels[1].noise_enable = data & 0x02;
		m_channels[2].noise_enable = data & 0x04;
		m_channels[3].noise_enable = data & 0x08;
		m_channels[4].noise_enable = data & 0x10;
		m_channels[5].noise_enable = data & 0x20;
		break;
	/* noise generators parameters */
	case 0x16:
		m_noise_params[0] = data & 0x03;
		m_noise_params[1] = (data >> 4) & 0x03;
		break;
	/* envelope generators parameters */
	case 0x18:  case 0x19:
		ch = reg - 0x18;
		m_env_reverse_right[ch] = data & 0x01;
		m_env_mode[ch] = (data >> 1) & 0x07;
		m_env_bits[ch] = data & 0x10;
		m_env_clock[ch] = data & 0x20;
		m_env_enable[ch] = data & 0x80;
		/* reset the envelope */
		m_env_step[ch] = 0;
		break;
	/* channels enable & reset generators */
	case 0x1c:
		m_all_ch_enable = data & 0x01;
		m_sync_state = data & 0x02;
		if (data & 0x02)
		{
			int i;

			/* Synch & Reset generators */
			logerror("%s: (SAA1099 '%s') -reg 0x1c- Chip reset\n", machine().describe_context(), tag());
			for (i = 0; i < 6; i++)
			{
				m_channels[i].level = 0;
				m_channels[i].counter = 0.0;
			}
		}
		break;
	default:    /* Error! */
		logerror("%s: (SAA1099 '%s') Unknown operation (reg:%02x, data:%02x)\n", machine().describe_context(), tag(), reg, data);
	}
}
