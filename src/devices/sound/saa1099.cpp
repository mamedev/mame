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

    Unspecified bits should be written as zero.

***************************************************************************/

#include "emu.h"
#include "saa1099.h"

static constexpr int clock_divider = 256;

static constexpr int LEFT   = 0x00;
static constexpr int RIGHT  = 0x01;

static constexpr u16 amplitude_lookup[16] = {
		0*32767/16,  1*32767/16,  2*32767/16,   3*32767/16,
		4*32767/16,  5*32767/16,  6*32767/16,   7*32767/16,
		8*32767/16,  9*32767/16, 10*32767/16, 11*32767/16,
	12*32767/16, 13*32767/16, 14*32767/16, 15*32767/16
};

static constexpr u8 envelope[8][64] = {
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
DEFINE_DEVICE_TYPE(SAA1099, saa1099_device, "saa1099", "Philips SAA1099")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  saa1099_device - constructor
//-------------------------------------------------

saa1099_device::saa1099_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SAA1099, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_noise_params{ 0, 0 }
	, m_env_enable{ false, false }
	, m_env_reverse_right{ false, false }
	, m_env_mode{ 0, 0 }
	, m_env_bits{ false, false }
	, m_env_clock{ false, false }
	, m_env_step{ 0, 0 }
	, m_all_ch_enable(false)
	, m_sync_state(false)
	, m_selected_reg(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saa1099_device::device_start()
{
	/* for each chip allocate one stream */
	m_stream = stream_alloc(0, 2, clock()/clock_divider);

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

	save_item(STRUCT_MEMBER(m_channels, frequency));
	save_item(STRUCT_MEMBER(m_channels, freq_enable));
	save_item(STRUCT_MEMBER(m_channels, noise_enable));
	save_item(STRUCT_MEMBER(m_channels, octave));
	save_item(STRUCT_MEMBER(m_channels, amplitude));
	save_item(STRUCT_MEMBER(m_channels, envelope));
	save_item(STRUCT_MEMBER(m_channels, counter));
	save_item(STRUCT_MEMBER(m_channels, level));

	save_item(STRUCT_MEMBER(m_noise, counter));
	save_item(STRUCT_MEMBER(m_noise, freq));
	save_item(STRUCT_MEMBER(m_noise, level));
}


//-------------------------------------------------
//  device_clock_changed
//-------------------------------------------------

void saa1099_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock()/clock_divider);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void saa1099_device::sound_stream_update(sound_stream &stream)
{
	int j, ch;
	/* if the channels are disabled we're done */
	if (!m_all_ch_enable)
		return;

	for (ch = 0; ch < 2; ch++)
	{
		switch (m_noise_params[ch])
		{
		case 0:
		case 1:
		case 2: m_noise[ch].freq = 256 << m_noise_params[ch]; break;
		case 3: m_noise[ch].freq = m_channels[ch * 3].freq(); break; // todo: this case will be clock()/[ch*3's octave divisor, 0 is = 256*2, higher numbers are higher] * 2 if the tone generator phase reset bit (0x1c bit 1) is set.
		}
	}

	/* fill all data needed */
	for( j = 0; j < stream.samples(); j++ )
	{
		int output_l = 0, output_r = 0;

		/* for each channel */
		for (ch = 0; ch < 6; ch++)
		{
			/* check the actual position in the square wave */
			while (m_channels[ch].counter <= 0)
			{
				/* calculate new frequency now after the half wave is updated */
				m_channels[ch].counter += m_channels[ch].freq();
				m_channels[ch].level ^= 1;

				/* eventually clock the envelope counters */
				if (ch == 1 && m_env_clock[0] == 0)
					envelope_w(0);
				if (ch == 4 && m_env_clock[1] == 0)
					envelope_w(1);
			}
			m_channels[ch].counter -= clock_divider;

			u8 level = 0; // output level (0...2, 0 = off, 1 = 100%, 2 = 50%)
			// if the noise is enabled
			u8 noise_out = m_noise[ch/3].level & 1; // noise output (noise 0: chan 0-2, noise 1: chan 3-5)
			u8 tone_out = m_channels[ch].level & 1; // tone output
			if (m_channels[ch].noise_enable)
			{
				// if both noise and square wave are enabled, output is tone output
				if (m_channels[ch].freq_enable)
				{
					// half amplitude if the noise level is high
					if (noise_out)
						level = tone_out << 1;
					else
						level = tone_out;
				}
				else
				{
					// output is noise output
					level = noise_out;
				}
			}
			// if the square wave is enabled
			else if (m_channels[ch].freq_enable)
			{
				// output is tone output
				level = tone_out;
			}
			// if the output level is high
			if (level > 0)
			{
				output_l += m_channels[ch].amplitude[ LEFT] * m_channels[ch].envelope[ LEFT] / 16 / level;
				output_r += m_channels[ch].amplitude[RIGHT] * m_channels[ch].envelope[RIGHT] / 16 / level;
			}
		}

		for (ch = 0; ch < 2; ch++)
		{
			/* update the state of the noise generator
			 * polynomial is x^18 + x^11 + x (i.e. 0x20400) and is a plain XOR, initial state is probably all 1s
			 * see http://www.vogons.org/viewtopic.php?f=9&t=51695 */
			while (m_noise[ch].counter <= 0)
			{
				m_noise[ch].counter += m_noise[ch].freq; // clock / ((511 - frequency) * 2^(8 - octave)) or clock / 2^(8 + noise period)
				if( ((m_noise[ch].level & 0x20000) == 0) != ((m_noise[ch].level & 0x0400) == 0) )
					m_noise[ch].level = (m_noise[ch].level << 1) | 1;
				else
					m_noise[ch].level <<= 1;
			}
			m_noise[ch].counter -= clock_divider;
		}
		/* write sound data to the buffer */
		stream.put_int(LEFT, j, output_l, 32768 * 6);
		stream.put_int(RIGHT, j, output_r, 32768 * 6);
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
		if (m_env_reverse_right[ch])
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


void saa1099_device::control_w(u8 data)
{
	if ((data & 0xff) > 0x1c)
	{
		/* Error! */
		logerror("%s: Unknown register selected\n", machine().describe_context());
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


void saa1099_device::data_w(u8 data)
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
		for (ch = 0; ch < 6; ch++)
			m_channels[ch].freq_enable = BIT(data, ch);
		break;
	/* channel i noise enable */
	case 0x15:
		for (ch = 0; ch < 6; ch++)
			m_channels[ch].noise_enable = BIT(data, ch);
		break;
	/* noise generators parameters */
	case 0x16:
		m_noise_params[0] = data & 0x03;
		m_noise_params[1] = (data >> 4) & 0x03;
		break;
	/* envelope generators parameters */
	case 0x18:  case 0x19:
		ch = reg - 0x18;
		m_env_reverse_right[ch] = BIT(data, 0);
		m_env_mode[ch] = (data >> 1) & 0x07;
		m_env_bits[ch] = BIT(data, 4);
		m_env_clock[ch] = BIT(data, 5);
		m_env_enable[ch] = BIT(data, 7);
		/* reset the envelope */
		m_env_step[ch] = 0;
		break;
	/* channels enable & reset generators */
	case 0x1c:
		m_all_ch_enable = BIT(data, 0);
		m_sync_state = BIT(data, 1);
		if (BIT(data, 1))
		{
			/* Synch & Reset generators */
			logerror("%s: -reg 0x1c- Chip reset\n", machine().describe_context());
			for (int i = 0; i < 6; i++)
			{
				m_channels[i].level = 0;
				m_channels[i].counter = m_channels[i].freq();
			}
		}
		break;
	default:    /* Error! */
		if (data != 0)
			logerror("%s: Unknown operation (reg:%02x, data:%02x)\n", machine().describe_context(), reg, data);
	}
}

void saa1099_device::write(offs_t offset, u8 data)
{
	if (offset & 1)
		control_w(data);
	else
		data_w(data);
}
