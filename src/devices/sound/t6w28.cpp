// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

  t6w28.c (based on sn74696.c)

  The t6w28 sound core is used in the SNK NeoGeo Pocket. It is a stereo
  sound chip based on 2 partial sn76489a cores.

  The block diagram for this chip is as follows:

Offset 0:
        Tone 0          /---------->   Att0  ---\
                        |                       |
        Tone 1          |  /------->   Att1  ---+
                        |  |                    |    Right
        Tone 2          |  |  /---->   Att2  ---+-------->
         |              |  |  |                 |
        Noise   -----+------------->   Att3  ---/
                     |  |  |  |
                     |  |  |  |
 Offset 1:           |  |  |  |
        Tone 0  --------+---------->   Att0  ---\
                     |     |  |                 |
        Tone 1  -----------+------->   Att1  ---+
                     |        |                 |     Left
        Tone 2  --------------+---->   Att2  ---+-------->
                     |                          |
        Noise        \------------->   Att3  ---/


***************************************************************************/

#include "emu.h"
#include "t6w28.h"


#define MAX_OUTPUT 0x7fff

#define STEP 0x10000

void t6w28_device::write(offs_t offset, uint8_t data)
{
	int n, r, c;


	/* update the output buffer before changing the registers */
	m_channel->update();

	offset &= 1;

	if (data & 0x80)
	{
		r = (data & 0x70) >> 4;
		m_last_register[offset] = r;
		m_register[offset * 8 + r] = (m_register[offset * 8 + r] & 0x3f0) | (data & 0x0f);
	}
	else
	{
		r = m_last_register[offset];
	}
	c = r/2;
	switch (r)
	{
		case 0: /* tone 0 : frequency */
		case 2: /* tone 1 : frequency */
		case 4: /* tone 2 : frequency */
			if ((data & 0x80) == 0) m_register[offset * 8 + r] = (m_register[offset * 8 + r] & 0x0f) | ((data & 0x3f) << 4);
			m_period[offset * 4 + c] = STEP * m_register[offset * 8 + r];
			if (m_period[offset * 4 + c] == 0) m_period[offset * 4 + c] = STEP;
			if (r == 4)
			{
				/* update noise shift frequency */
				if ((m_register[offset * 8 + 6] & 0x03) == 0x03)
					m_period[offset * 4 + 3] = 2 * m_period[offset * 4 + 2];
			}
			break;
		case 1: /* tone 0 : volume */
		case 3: /* tone 1 : volume */
		case 5: /* tone 2 : volume */
		case 7: /* noise  : volume */
			m_volume[offset * 4 + c] = m_vol_table[data & 0x0f];
			if ((data & 0x80) == 0) m_register[offset * 8 + r] = (m_register[offset * 8 + r] & 0x3f0) | (data & 0x0f);
			break;
		case 6: /* noise  : frequency, mode */
			{
					if ((data & 0x80) == 0) m_register[offset * 8 + r] = (m_register[offset * 8 + r] & 0x3f0) | (data & 0x0f);
				n = m_register[offset * 8 + 6];
				m_noise_mode[offset] = (n & 4) ? 1 : 0;
				/* N/512,N/1024,N/2048,Tone #3 output */
				m_period[offset * 4 + 3] = ((n&3) == 3) ? 2 * m_period[offset * 4 + 2] : (STEP << (5+(n&3)));
					/* Reset noise shifter */
				m_rng[offset] = m_feedback_mask; /* this is correct according to the smspower document */
				//m_rng = 0xF35; /* this is not, but sounds better in do run run */
				m_output[offset * 4 + 3] = m_rng[offset] & 1;
			}
			break;
	}
}



//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void t6w28_device::sound_stream_update(sound_stream &stream)
{
	/* If the volume is 0, increase the counter */
	for (int i = 0;i < 8;i++)
	{
		if (m_volume[i] == 0)
		{
			/* note that I do count += samples, NOT count = samples + 1. You might think */
			/* it's the same since the volume is 0, but doing the latter could cause */
			/* interferencies when the program is rapidly modulating the volume. */
			if (m_count[i] <= stream.samples()*STEP) m_count[i] += stream.samples()*STEP;
		}
	}

	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		int vol[8];
		unsigned int out0, out1;
		int left;


		/* vol[] keeps track of how long each square wave stays */
		/* in the 1 position during the sample period. */
		vol[0] = vol[1] = vol[2] = vol[3] = vol[4] = vol[5] = vol[6] = vol[7] = 0;

		for (int i = 2;i < 3;i++)
		{
			if (m_output[i]) vol[i] += m_count[i];
			m_count[i] -= STEP;
			/* m_period[i] is the half period of the square wave. Here, in each */
			/* loop I add m_period[i] twice, so that at the end of the loop the */
			/* square wave is in the same status (0 or 1) it was at the start. */
			/* vol[i] is also incremented by m_period[i], since the wave has been 1 */
			/* exactly half of the time, regardless of the initial position. */
			/* If we exit the loop in the middle, m_output[i] has to be inverted */
			/* and vol[i] incremented only if the exit status of the square */
			/* wave is 1. */
			while (m_count[i] <= 0)
			{
				m_count[i] += m_period[i];
				if (m_count[i] > 0)
				{
					m_output[i] ^= 1;
					if (m_output[i]) vol[i] += m_period[i];
					break;
				}
				m_count[i] += m_period[i];
				vol[i] += m_period[i];
			}
			if (m_output[i]) vol[i] -= m_count[i];
		}

		for (int i = 4;i < 7;i++)
		{
			if (m_output[i]) vol[i] += m_count[i];
			m_count[i] -= STEP;
			/* m_period[i] is the half period of the square wave. Here, in each */
			/* loop I add m_period[i] twice, so that at the end of the loop the */
			/* square wave is in the same status (0 or 1) it was at the start. */
			/* vol[i] is also incremented by m_period[i], since the wave has been 1 */
			/* exactly half of the time, regardless of the initial position. */
			/* If we exit the loop in the middle, m_output[i] has to be inverted */
			/* and vol[i] incremented only if the exit status of the square */
			/* wave is 1. */
			while (m_count[i] <= 0)
			{
				m_count[i] += m_period[i];
				if (m_count[i] > 0)
				{
					m_output[i] ^= 1;
					if (m_output[i]) vol[i] += m_period[i];
					break;
				}
				m_count[i] += m_period[i];
				vol[i] += m_period[i];
			}
			if (m_output[i]) vol[i] -= m_count[i];
		}

		left = STEP;
		do
		{
			int nextevent;


			if (m_count[3] < left) nextevent = m_count[3];
			else nextevent = left;

			if (m_output[3]) vol[3] += m_count[3];
			m_count[3] -= nextevent;
			if (m_count[3] <= 0)
			{
				if (m_noise_mode[0] == 1) /* White Noise Mode */
				{
					if (((m_rng[0] & m_whitenoise_taps) != m_whitenoise_taps) && ((m_rng[0] & m_whitenoise_taps) != 0)) /* crappy xor! */
					{
						m_rng[0] >>= 1;
						m_rng[0] |= m_feedback_mask;
					}
					else
					{
						m_rng[0] >>= 1;
					}
					m_output[3] = m_whitenoise_invert ? !(m_rng[0] & 1) : m_rng[0] & 1;
				}
				else /* Periodic noise mode */
				{
					if (m_rng[0] & 1)
					{
						m_rng[0] >>= 1;
						m_rng[0] |= m_feedback_mask;
					}
					else
					{
						m_rng[0] >>= 1;
					}
					m_output[3] = m_rng[0] & 1;
				}
				m_count[3] += m_period[3];
				if (m_output[3]) vol[3] += m_period[3];
			}
			if (m_output[3]) vol[3] -= m_count[3];

			left -= nextevent;
		} while (left > 0);

		if (m_enabled)
		{
			out0 = vol[4] * m_volume[4] + vol[5] * m_volume[5] +
					vol[6] * m_volume[6] + vol[3] * m_volume[7];

			out1 = vol[4] * m_volume[0] + vol[5] * m_volume[1] +
					vol[6] * m_volume[2] + vol[3] * m_volume[3];
		}
		else
		{
			out0 = 0;
			out1 = 0;
		}

		if (out0 > MAX_OUTPUT * STEP) out0 = MAX_OUTPUT * STEP;
		if (out1 > MAX_OUTPUT * STEP) out1 = MAX_OUTPUT * STEP;

		stream.put_int(0, sampindex, out0 / STEP, 32768);
		stream.put_int(1, sampindex, out1 / STEP, 32768);
	}
}



void t6w28_device::set_gain(int gain)
{
	int i;
	double out;

	gain &= 0xff;

	/* increase max output basing on gain (0.2 dB per step) */
	out = MAX_OUTPUT / 3;
	while (gain-- > 0)
		out *= 1.023292992; /* = (10 ^ (0.2/20)) */

	/* build volume table (2dB per step) */
	for (i = 0;i < 15;i++)
	{
		/* limit volume to avoid clipping */
		if (out > MAX_OUTPUT / 3) m_vol_table[i] = MAX_OUTPUT / 3;
		else m_vol_table[i] = out;

		out /= 1.258925412; /* = 10 ^ (2/20) = 2dB */
	}
	m_vol_table[15] = 0;
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void t6w28_device::device_start()
{
	int i;

	m_sample_rate = clock() / 16;
	m_channel = stream_alloc(0, 2, m_sample_rate);

	for (i = 0;i < 8;i++) m_volume[i] = 0;

	m_last_register[0] = 0;
	m_last_register[1] = 0;
	for (i = 0;i < 8;i+=2)
	{
		m_register[i] = 0;
		m_register[i + 1] = 0x0f;   /* volume = 0 */
	}

	for (i = 0;i < 8;i++)
	{
		m_output[i] = 0;
		m_period[i] = m_count[i] = STEP;
	}

	/* Default is SN76489 non-A */
	m_feedback_mask = 0x4000;     /* mask for feedback */
	m_whitenoise_taps = 0x03;   /* mask for white noise taps */
	m_whitenoise_invert = 1; /* white noise invert flag */

	m_rng[0] = m_feedback_mask;
	m_rng[1] = m_feedback_mask;
	m_output[3] = m_rng[0] & 1;

	set_gain(0);

	/* values from sn76489a */
	m_feedback_mask = 0x8000;
	m_whitenoise_taps = 0x06;
	m_whitenoise_invert = false;

	save_item(NAME(m_register));
	save_item(NAME(m_last_register));
	save_item(NAME(m_volume));
	save_item(NAME(m_rng));
	save_item(NAME(m_noise_mode));
	save_item(NAME(m_period));
	save_item(NAME(m_count));
	save_item(NAME(m_output));
	save_item(NAME(m_enabled));
}


void t6w28_device::set_enable(bool enable)
{
	m_enabled = enable;
}

DEFINE_DEVICE_TYPE(T6W28, t6w28_device, "t6w28", "T6W28")

t6w28_device::t6w28_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, T6W28, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_channel(nullptr)
{
}
