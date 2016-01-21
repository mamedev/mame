// license:BSD-3-Clause
// copyright-holders:Olivier Galibert,Aaron Giles
/*********************************************************/
/*    ricoh RF5C68(or clone) PCM controller              */
/*********************************************************/

#include "emu.h"
#include "rf5c68.h"


// device type definition
const device_type RF5C68 = &device_creator<rf5c68_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  rf5c68_device - constructor
//-------------------------------------------------

rf5c68_device::rf5c68_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, RF5C68, "RF5C68", tag, owner, clock, "rf5c68", __FILE__),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_cbank(0),
		m_wbank(0),
		m_enable(0)
{
	memset(m_data, 0, sizeof(UINT8)*0x10000);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rf5c68_device::device_start()
{
	m_sample_end_cb.bind_relative_to(*owner());

	/* allocate memory for the chip */
	memset(m_data, 0xff, sizeof(m_data));

	/* allocate the stream */
	m_stream = stream_alloc(0, 2, clock() / 384);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void rf5c68_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *left = outputs[0];
	stream_sample_t *right = outputs[1];
	int i, j;

	/* start with clean buffers */
	memset(left, 0, samples * sizeof(*left));
	memset(right, 0, samples * sizeof(*right));

	/* bail if not enabled */
	if (!m_enable)
		return;

	/* loop over channels */
	for (i = 0; i < RF5C68_NUM_CHANNELS; i++)
	{
		rf5c68_pcm_channel *chan = &m_chan[i];

		/* if this channel is active, accumulate samples */
		if (chan->enable)
		{
			int lv = (chan->pan & 0x0f) * chan->env;
			int rv = ((chan->pan >> 4) & 0x0f) * chan->env;

			/* loop over the sample buffer */
			for (j = 0; j < samples; j++)
			{
				int sample;

				/* trigger sample callback */
				if(!m_sample_end_cb.isnull())
				{
					if(((chan->addr >> 11) & 0xfff) == 0xfff)
						m_sample_end_cb((chan->addr >> 11)/0x2000);
				}

				/* fetch the sample and handle looping */
				sample = m_data[(chan->addr >> 11) & 0xffff];
				if (sample == 0xff)
				{
					chan->addr = chan->loopst << 11;
					sample = m_data[(chan->addr >> 11) & 0xffff];

					/* if we loop to a loop point, we're effectively dead */
					if (sample == 0xff)
						break;
				}
				chan->addr += chan->step;

				/* add to the buffer */
				if (sample & 0x80)
				{
					sample &= 0x7f;
					left[j] += (sample * lv) >> 5;
					right[j] += (sample * rv) >> 5;
				}
				else
				{
					left[j] -= (sample * lv) >> 5;
					right[j] -= (sample * rv) >> 5;
				}
			}
		}
	}

	/* now clamp and shift the result (output is only 10 bits) */
	for (j = 0; j < samples; j++)
	{
		stream_sample_t temp;

		temp = left[j];
		if (temp > 32767) temp = 32767;
		else if (temp < -32768) temp = -32768;
		left[j] = temp & ~0x3f;

		temp = right[j];
		if (temp > 32767) temp = 32767;
		else if (temp < -32768) temp = -32768;
		right[j] = temp & ~0x3f;
	}
}


//-------------------------------------------------
//    RF5C68 write register
//-------------------------------------------------

READ8_MEMBER( rf5c68_device::rf5c68_r )
{
	UINT8 shift;

	m_stream->update();
	shift = (offset & 1) ? 11 + 8 : 11;

//  printf("%08x\n",(m_chan[(offset & 0x0e) >> 1].addr));

	return (m_chan[(offset & 0x0e) >> 1].addr) >> (shift);
}

WRITE8_MEMBER( rf5c68_device::rf5c68_w )
{
	rf5c68_pcm_channel *chan = &m_chan[m_cbank];
	int i;

	/* force the stream to update first */
	m_stream->update();

	/* switch off the address */
	switch (offset)
	{
		case 0x00:  /* envelope */
			chan->env = data;
			break;

		case 0x01:  /* pan */
			chan->pan = data;
			break;

		case 0x02:  /* FDL */
			chan->step = (chan->step & 0xff00) | (data & 0x00ff);
			break;

		case 0x03:  /* FDH */
			chan->step = (chan->step & 0x00ff) | ((data << 8) & 0xff00);
			break;

		case 0x04:  /* LSL */
			chan->loopst = (chan->loopst & 0xff00) | (data & 0x00ff);
			break;

		case 0x05:  /* LSH */
			chan->loopst = (chan->loopst & 0x00ff) | ((data << 8) & 0xff00);
			break;

		case 0x06:  /* ST */
			chan->start = data;
			if (!chan->enable)
				chan->addr = chan->start << (8 + 11);
			break;

		case 0x07:  /* control reg */
			m_enable = (data >> 7) & 1;
			if (data & 0x40)
				m_cbank = data & 7;
			else
				m_wbank = data & 15;
			break;

		case 0x08:  /* channel on/off reg */
			for (i = 0; i < 8; i++)
			{
				m_chan[i].enable = (~data >> i) & 1;
				if (!m_chan[i].enable)
					m_chan[i].addr = m_chan[i].start << (8 + 11);
			}
			break;
	}
}


//-------------------------------------------------
//    RF5C68 read memory
//-------------------------------------------------

READ8_MEMBER( rf5c68_device::rf5c68_mem_r )
{
	return m_data[m_wbank * 0x1000 + offset];
}


//-------------------------------------------------
//    RF5C68 write memory
//-------------------------------------------------

WRITE8_MEMBER( rf5c68_device::rf5c68_mem_w )
{
	m_data[m_wbank * 0x1000 + offset] = data;
}
