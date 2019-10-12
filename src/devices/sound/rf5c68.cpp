// license:BSD-3-Clause
// copyright-holders:Olivier Galibert,Aaron Giles
/*********************************************************/
/*    ricoh RF5C68(or clone) PCM controller              */
/*                                                       */
/*    TODO : RF5C164 (Sega CD/Mega CD)                   */
/*           has difference?                             */
/*********************************************************/

#include "emu.h"
#include "rf5c68.h"


// device type definition
DEFINE_DEVICE_TYPE(RF5C68, rf5c68_device, "rf5c68", "Ricoh RF5C68")
DEFINE_DEVICE_TYPE(RF5C164, rf5c164_device, "rf5c164", "Ricoh RF5C164")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  rf5c68_device - constructor
//-------------------------------------------------

rf5c68_device::rf5c68_device(const machine_config & mconfig, device_type type, const char * tag, device_t * owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 16) // 15 bit Address + 2 Memory select outputs(total 64KB), PSRAM/SRAM/ROM
	, m_stream(nullptr)
	, m_cbank(0)
	, m_wbank(0)
	, m_enable(0)
{
}

rf5c68_device::rf5c68_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rf5c68_device(mconfig, RF5C68, tag, owner, clock)
{
}


//-------------------------------------------------
//  rf5c68_device - constructor
//-------------------------------------------------

rf5c164_device::rf5c164_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rf5c68_device(mconfig, RF5C164, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rf5c68_device::device_start()
{
	m_data = &space(0);
	// Find our direct access
	m_cache = space().cache<0, 0, ENDIANNESS_LITTLE>();
	m_sample_end_cb.bind_relative_to(*owner());

	/* allocate the stream */
	m_stream = stream_alloc(0, 2, clock() / 384);

	for (int ch = 0; ch < NUM_CHANNELS; ch++)
	{
		save_item(NAME(m_chan[ch].enable), ch);
		save_item(NAME(m_chan[ch].env), ch);
		save_item(NAME(m_chan[ch].pan), ch);
		save_item(NAME(m_chan[ch].start), ch);
		save_item(NAME(m_chan[ch].addr), ch);
		save_item(NAME(m_chan[ch].step), ch);
		save_item(NAME(m_chan[ch].loopst), ch);
	}
	save_item(NAME(m_cbank));
	save_item(NAME(m_wbank));
	save_item(NAME(m_enable));
}

//-------------------------------------------------
//  device_clock_changed
//-------------------------------------------------

void rf5c68_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / 384);
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector rf5c68_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_data_config) };
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void rf5c68_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *left = outputs[0];
	stream_sample_t *right = outputs[1];

	/* start with clean buffers */
	memset(left, 0, samples * sizeof(*left));
	memset(right, 0, samples * sizeof(*right));

	/* bail if not enabled */
	if (!m_enable)
		return;

	/* loop over channels */
	for (pcm_channel &chan : m_chan)
	{
		/* if this channel is active, accumulate samples */
		if (chan.enable)
		{
			int lv = (chan.pan & 0x0f) * chan.env;
			int rv = ((chan.pan >> 4) & 0x0f) * chan.env;

			/* loop over the sample buffer */
			for (int j = 0; j < samples; j++)
			{
				int sample;

				/* trigger sample callback */
				if(!m_sample_end_cb.isnull())
				{
					if(((chan.addr >> 11) & 0xfff) == 0xfff)
						m_sample_end_cb((chan.addr >> 11)/0x2000);
				}

				/* fetch the sample and handle looping */
				sample = m_cache->read_byte((chan.addr >> 11) & 0xffff);
				if (sample == 0xff)
				{
					chan.addr = chan.loopst << 11;
					sample = m_cache->read_byte((chan.addr >> 11) & 0xffff);

					/* if we loop to a loop point, we're effectively dead */
					if (sample == 0xff)
						break;
				}
				chan.addr += chan.step;

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
	for (int j = 0; j < samples; j++)
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

u8 rf5c68_device::rf5c68_r(offs_t offset)
{
	uint8_t shift;

	m_stream->update();
	shift = (offset & 1) ? 11 + 8 : 11;

//  printf("%08x\n",(m_chan[(offset & 0x0e) >> 1].addr));

	return (m_chan[(offset & 0x0e) >> 1].addr) >> (shift);
}

void rf5c68_device::rf5c68_w(offs_t offset, u8 data)
{
	pcm_channel &chan = m_chan[m_cbank];
	int i;

	/* force the stream to update first */
	m_stream->update();

	/* switch off the address */
	switch (offset)
	{
		case 0x00:  /* envelope */
			chan.env = data;
			break;

		case 0x01:  /* pan */
			chan.pan = data;
			break;

		case 0x02:  /* FDL */
			chan.step = (chan.step & 0xff00) | (data & 0x00ff);
			break;

		case 0x03:  /* FDH */
			chan.step = (chan.step & 0x00ff) | ((data << 8) & 0xff00);
			break;

		case 0x04:  /* LSL */
			chan.loopst = (chan.loopst & 0xff00) | (data & 0x00ff);
			break;

		case 0x05:  /* LSH */
			chan.loopst = (chan.loopst & 0x00ff) | ((data << 8) & 0xff00);
			break;

		case 0x06:  /* ST */
			chan.start = data;
			if (!chan.enable)
				chan.addr = chan.start << (8 + 11);
			break;

		case 0x07:  /* control reg */
			m_enable = (data >> 7) & 1;
			if (data & 0x40)
				m_cbank = data & 7;
			else
				m_wbank = (data & 0xf) << 12;
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

u8 rf5c68_device::rf5c68_mem_r(offs_t offset)
{
	return m_cache->read_byte(m_wbank | offset);
}


//-------------------------------------------------
//    RF5C68 write memory
//-------------------------------------------------

void rf5c68_device::rf5c68_mem_w(offs_t offset, u8 data)
{
	m_data->write_byte(m_wbank | offset, data);
}
