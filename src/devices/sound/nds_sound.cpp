// license:BSD-3-Clause
// copyright-holders:cam900
/***************************************************************************

    Nintendo DS Sound Hardware Emulation

	Preliminary emulation core by cam900

    Nintendo DS Sound has 16 sound channel and 2 sound capture channels.
    basically each souhd channel has ADPCM/8 or 16 bit linear PCM
    playback, with PSG (channel 8-13) or Noise (channel 14-15).

    TODO:
    - needs to further verifications from real hardware

    Tech info: http://problemkaputt.de/gbatek.htm

***************************************************************************/

#include "emu.h"
#include "nds_sound.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// ADPCM tables
static const s8 adpcm_index_table[8] =
{
	-1, -1, -1, -1, 2, 4, 6, 8
};

static const u16 adpcm_diff_table[89] =
{
	0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x0010,
	0x0011, 0x0013, 0x0015, 0x0017, 0x0019, 0x001C, 0x001F, 0x0022, 0x0025,
	0x0029, 0x002D, 0x0032, 0x0037, 0x003C, 0x0042, 0x0049, 0x0050, 0x0058,
	0x0061, 0x006B, 0x0076, 0x0082, 0x008F, 0x009D, 0x00AD, 0x00BE, 0x00D1,
	0x00E6, 0x00FD, 0x0117, 0x0133, 0x0151, 0x0173, 0x0198, 0x01C1, 0x01EE,
	0x0220, 0x0256, 0x0292, 0x02D4, 0x031C, 0x036C, 0x03C3, 0x0424, 0x048E,
	0x0502, 0x0583, 0x0610, 0x06AB, 0x0756, 0x0812, 0x08E0, 0x09C3, 0x0ABD,
	0x0BD0, 0x0CFF, 0x0E4C, 0x0FBA, 0x114C, 0x1307, 0x14EE, 0x1706, 0x1954,
	0x1BDC, 0x1EA5, 0x21B6, 0x2515, 0x28CA, 0x2CDF, 0x315B, 0x364B, 0x3BB9,
	0x41B2, 0x4844, 0x4F7E, 0x5771, 0x602F, 0x69CE, 0x7462, 0x7FFF
};

// actually 0x04000400-0x0400051f in ARM7
void nds_sound_device::amap(address_map &map)
{
	map(0x000, 0x003).r(FUNC(nds_sound_device::channel_control_r)).select(0x0f0);
	map(0x000, 0x00f).w(FUNC(nds_sound_device::channel_w));
	map(0x100, 0x103).rw(FUNC(nds_sound_device::control_r), FUNC(nds_sound_device::control_w));
	map(0x104, 0x107).rw(FUNC(nds_sound_device::bias_r), FUNC(nds_sound_device::bias_w));
	map(0x108, 0x109).rw(FUNC(nds_sound_device::capture_control_r), FUNC(nds_sound_device::capture_control_w));
	map(0x110, 0x113).r(FUNC(nds_sound_device::capture_addr_r)).select(0x008);
	map(0x110, 0x11f).w(FUNC(nds_sound_device::capture_addrlen_w));
}

// device type definition
DEFINE_DEVICE_TYPE(NDS_SOUND, nds_sound_device, "nds_sound", "Nintendo DS Sound Hardware")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nds_sound_device - constructor
//-------------------------------------------------

nds_sound_device::nds_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NDS_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_data_config("data", ENDIANNESS_LITTLE, 32, 27) // 32 bit bus, TODO: correct?
	, m_stream(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nds_sound_device::device_start()
{
	space(0).cache(m_cache);
	space(0).specific(m_space);

	// stream system initialize
	m_stream = stream_alloc(0, 2, clock() / 2 / CLOCK_DIVIDE); // ~32.768khz, TODO: correct sample rate?

	// setup configurations
	for (auto & elem : m_channel)
		elem.host = this;
	for (auto & elem : m_capture)
	{
		elem.host = this;
		for (int f = 0; f < 8; f++)
			elem.fifo[f].d = 0;
	}

	for (int i = 8; i <= 13; i++)
		m_channel[i].psg = true;
	for (int i = 14; i <= 15; i++)
		m_channel[i].noise = true;

	m_capture[0].input = &m_channel[0];
	m_capture[0].output = &m_channel[1];
	m_capture[1].input = &m_channel[2];
	m_capture[1].output = &m_channel[3];

	save_item(STRUCT_MEMBER(m_channel, control));
	save_item(STRUCT_MEMBER(m_channel, sourceaddr));
	save_item(STRUCT_MEMBER(m_channel, freq));
	save_item(STRUCT_MEMBER(m_channel, loopstart));
	save_item(STRUCT_MEMBER(m_channel, length));

	save_item(STRUCT_MEMBER(m_channel, playing));
	save_item(STRUCT_MEMBER(m_channel, adpcm_out));
	save_item(STRUCT_MEMBER(m_channel, adpcm_index));
	save_item(STRUCT_MEMBER(m_channel, prev_adpcm_out));
	save_item(STRUCT_MEMBER(m_channel, prev_adpcm_index));
	save_item(STRUCT_MEMBER(m_channel, cur_addr));
	save_item(STRUCT_MEMBER(m_channel, cur_state));
	save_item(STRUCT_MEMBER(m_channel, cur_bitaddr));
	save_item(STRUCT_MEMBER(m_channel, delay));
	save_item(STRUCT_MEMBER(m_channel, sample));
	save_item(STRUCT_MEMBER(m_channel, lfsr));
	save_item(STRUCT_MEMBER(m_channel, lfsr_out));
	save_item(STRUCT_MEMBER(m_channel, counter));
	save_item(STRUCT_MEMBER(m_channel, output));
	save_item(STRUCT_MEMBER(m_channel, loutput));
	save_item(STRUCT_MEMBER(m_channel, routput));

	save_item(STRUCT_MEMBER(m_capture, control));
	save_item(STRUCT_MEMBER(m_capture, dstaddr));
	save_item(STRUCT_MEMBER(m_capture, length));

	save_item(STRUCT_MEMBER(m_capture, counter));
	save_item(STRUCT_MEMBER(m_capture, cur_addr));
	save_item(STRUCT_MEMBER(m_capture, cur_waddr));
	save_item(STRUCT_MEMBER(m_capture, cur_bitaddr));
	save_item(STRUCT_MEMBER(m_capture, enable));

	for (int c = 0; c < 2; c++)
	{
		for (int f = 0; f < 8; f++)
			save_item(NAME(m_capture[c].fifo[f].d), (f << 1) | c);
	}
	save_item(STRUCT_MEMBER(m_capture, fifo_head));
	save_item(STRUCT_MEMBER(m_capture, fifo_tail));
	save_item(STRUCT_MEMBER(m_capture, fifo_empty));
	save_item(STRUCT_MEMBER(m_capture, fifo_full));

	save_item(NAME(m_control));
	save_item(NAME(m_bias));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nds_sound_device::device_reset()
{
	// TODO: registers are resetted?
}


//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void nds_sound_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / 2 / CLOCK_DIVIDE);
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector nds_sound_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_data_config) };
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void nds_sound_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	outputs[0].fill(0);
	outputs[1].fill(0);
	for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
	{
		if (enable())
		{
			// mix outputs
			int lmix = 0, rmix = 0;
			for (int i = 0; i < 16; i++)
			{
				channel_t *channel = &m_channel[i];
				channel->update();
				// bypass mixer
				if (((i == 1) && (!mix_ch1())) || ((i == 3) && (!mix_ch3())))
					continue;

				lmix += channel->loutput;
				rmix += channel->routput;
			}

			// send mixer output to capture
			m_capture[0].update(lmix);
			m_capture[1].update(rmix);

			// select left/right output source
			switch (lout_from())
			{
			case 0: // left mixer
				break;
			case 1: // channel 1
				lmix = m_channel[1].loutput;
				break;
			case 2: // channel 3
				lmix = m_channel[3].loutput;
				break;
			case 3: // channel 1 + 3
				lmix = m_channel[1].loutput + m_channel[3].loutput;
				break;
			}

			switch (rout_from())
			{
			case 0: // right mixer
				break;
			case 1: // channel 1
				rmix = m_channel[1].routput;
				break;
			case 2: // channel 3
				rmix = m_channel[3].routput;
				break;
			case 3: // channel 1 + 3
				rmix = m_channel[1].routput + m_channel[3].routput;
				break;
			}

			// adjust master volume
			lmix = (lmix * mvol()) >> 13;
			rmix = (rmix * mvol()) >> 13;

			// add bias and clip output
			lmix = std::max<int>(0, std::min<int>(0x3ff, (lmix + (m_bias & 0x3ff))));
			rmix = std::max<int>(0, std::min<int>(0x3ff, (rmix + (m_bias & 0x3ff))));

			// set output
			outputs[0].put_int_clamp(sampindex, lmix, 0x3ff);
			outputs[1].put_int_clamp(sampindex, rmix, 0x3ff);
		}
	}
}


//-------------------------------------------------
//  channel_control_r - read sound channel control
//  register
//-------------------------------------------------

u32 nds_sound_device::channel_control_r(offs_t offset)
{
	return m_channel[(offset >> 2) & 0xf].control;
}


//-------------------------------------------------
//  channel_w - write sound channel register
//-------------------------------------------------

void nds_sound_device::channel_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_channel[(offset >> 2) & 0xf].write(offset & 3, data, mem_mask);
}


//-------------------------------------------------
//  channel_t::write - write sound channel register
//-------------------------------------------------

void nds_sound_device::channel_t::write(offs_t offset, u32 data, u32 mem_mask)
{
	const u32 old = control;
	switch (offset & 3)
	{
	case 0: // Control/Status
		COMBINE_DATA(&control);
		if (BIT(old ^ control, 31))
			keyon();
		else
			keyoff();

		// reset hold flag
		if (!playing && !hold())
		{
			sample = lfsr_out = 0;
			output = loutput = routput = 0;
		}
		break;
	case 1: // Source address
		mem_mask &= 0x7ffffff;
		COMBINE_DATA(&sourceaddr);
		break;
	case 2: // Frequency, Loopstart
		if (ACCESSING_BITS_0_15)
			freq = (freq & BIT(~mem_mask, 0, 16)) | (BIT(data & mem_mask, 0, 16));
		if (ACCESSING_BITS_16_31)
			loopstart = (loopstart & BIT(~mem_mask, 16, 16)) | (BIT(data & mem_mask, 16, 16));
		break;
	case 3: // Length
		mem_mask &= 0x3fffff;
		COMBINE_DATA(&length);
		break;
	}
}


//-------------------------------------------------
//  channel_t::keyon - execute sound channel keyon
//-------------------------------------------------

void nds_sound_device::channel_t::keyon()
{
	if (!playing)
	{
		playing = true;
		delay = format() == 2 ? 11 : 3; // 3 (11 for ADPCM) delay for playing sample
		cur_bitaddr = cur_addr = 0;
		cur_state = format() == 2 ? STATE_ADPCM_LOAD : (loopstart == 0 ? STATE_POST_LOOP : STATE_PRE_LOOP);
		counter = 0x10000;
		sample = 0;
		lfsr_out = 0x7fff;
		lfsr = 0x7fff;
	}
}


//-------------------------------------------------
//  channel_t::keyoff - execute sound channel keyoff
//-------------------------------------------------

void nds_sound_device::channel_t::keyoff()
{
	if (playing)
	{
		if (busy())
			control &= ~(1 << 31);
		if (!hold())
		{
			sample = lfsr_out = 0;
			output = loutput = routput = 0;
		}

		playing = false;
	}
}


//-------------------------------------------------
//  channel_t::fetch - fetch sound sample
//-------------------------------------------------

void nds_sound_device::channel_t::fetch()
{
	if (playing)
	{
		// fetch samples
		switch (format())
		{
		case 0: // PCM8
			sample = s16(host->m_cache.read_byte(addr()) << 8);
			break;
		case 1: // PCM16
			sample = host->m_cache.read_word(addr());
			break;
		case 2: // ADPCM
			sample = cur_state == STATE_ADPCM_LOAD ? 0 : adpcm_out;
			break;
		case 3: // PSG or Noise
			sample = 0;
			if (psg) // psg
				sample = duty() == 7 ? -0x8000 : cur_bitaddr < (7 - duty()) ? -0x8000 : 0x7fff;
			else if (noise) // noise
				sample = lfsr_out;
			break;
		}
	}

	// apply delay
	if (format() != 3 && delay > 0)
		sample = 0;
}


//-------------------------------------------------
//  channel_t::advance - advance sound channel
//-------------------------------------------------

void nds_sound_device::channel_t::advance()
{
	if (playing)
	{
		// advance bit address
		switch (format())
		{
		case 0: // PCM8
			cur_bitaddr += 8;
			break;
		case 1: // PCM16
			cur_bitaddr += 16;
			break;
		case 2: // ADPCM
			if (cur_state == STATE_ADPCM_LOAD) // load ADPCM data
			{
				if (cur_bitaddr == 0)
					prev_adpcm_out = adpcm_out = host->m_cache.read_word(addr());
				if (cur_bitaddr == 16)
					prev_adpcm_index = adpcm_index = std::max<int>(0, std::min<int>(88, host->m_cache.read_byte(addr()) & 0x7f));
			}
			else // decode ADPCM
			{
				u8 input = BIT(host->m_cache.read_byte(addr()), cur_bitaddr & 4, 4);
				int diff = ((BIT(input, 0, 3) * 2 + 1) * adpcm_diff_table[adpcm_index] / 8);
				if (BIT(input, 3)) diff = -diff;
				adpcm_out = std::max<int>(-0x8000, std::min<int>(0x7fff, adpcm_out + diff));
				adpcm_index = std::max<int>(0, std::min<int>(88, adpcm_index + adpcm_index_table[BIT(input, 0, 3)]));
			}
			cur_bitaddr += 4;
			break;
		case 3: // PSG or Noise
			if (psg) // psg
				cur_bitaddr = (cur_bitaddr + 1) & 7;
			else if (noise) // noise
			{
				if (BIT(lfsr, 1))
				{
					lfsr = (lfsr >> 1) ^ 0x6000;
					lfsr_out = -0x8000;
				}
				else
				{
					lfsr >>= 1;
					lfsr_out = 0x7fff;
				}
			}
			break;
		}

		// address update
		if (format() != 3)
		{
			// adjust delay
			delay--;

			// update address, loop
			while (cur_bitaddr >= 32)
			{
				// already loaded?
				if (format() == 2 && cur_state == STATE_ADPCM_LOAD)
				{
					cur_state = loopstart == 0 ? STATE_POST_LOOP : STATE_PRE_LOOP;
				}
				cur_addr++;
				if (cur_state == STATE_PRE_LOOP && cur_addr >= loopstart)
				{
					cur_state = STATE_POST_LOOP;
					cur_addr = 0;
					if (format() == 2)
					{
						prev_adpcm_out = adpcm_out;
						prev_adpcm_index = adpcm_index;
					}
				}
				else if (cur_state == STATE_POST_LOOP && cur_addr >= length)
				{
					switch (repeat())
					{
					case 0: // manual; not correct?
					case 2: // one-shot
					case 3: // prohibited
						keyoff();
						break;
					case 1: // loop infinitely
						if (format() == 2)
						{
							if (loopstart == 0) // reload ADPCM
							{
								cur_state = STATE_ADPCM_LOAD;
							}
							else // restore
							{
								adpcm_out = prev_adpcm_out;
								adpcm_index = prev_adpcm_index;
							}
						}
						cur_addr = 0;
						break;
					}
				}
				cur_bitaddr -= 32;
			}
		}
	}
}


//-------------------------------------------------
//  channel_t::update - update sound channel
//-------------------------------------------------

void nds_sound_device::channel_t::update()
{
	if (playing)
	{
		// get output
		fetch();
		counter -= CLOCK_DIVIDE;
		while (counter <= freq)
		{
			// advance
			advance();
			counter += 0x10000 - freq;
		}
		output = (sample * volume()) >> (7 + voldiv());
		loutput = (sample * lvol()) >> 7;
		routput = (sample * rvol()) >> 7;
	}
}


//-------------------------------------------------
//  control_r - read global control register
//-------------------------------------------------

u32 nds_sound_device::control_r()
{
	return m_control;
}


//-------------------------------------------------
//  bias_r - read global output bias
//-------------------------------------------------

u32 nds_sound_device::bias_r()
{
	return m_bias;
}


//-------------------------------------------------
//  control_w - write global control register
//-------------------------------------------------

void nds_sound_device::control_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_control);
}


//-------------------------------------------------
//  bias_w - write global output bias
//-------------------------------------------------

void nds_sound_device::bias_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_bias);
}


//-------------------------------------------------
//  capture_control_r - read sound capture control
//  register
//-------------------------------------------------

u8 nds_sound_device::capture_control_r(offs_t offset)
{
	offset &= 1;
	return m_capture[offset].control;
}


//-------------------------------------------------
//  capture_addr_r - read sound capture destination
//  address register
//-------------------------------------------------

u32 nds_sound_device::capture_addr_r(offs_t offset)
{
	return m_capture[(offset >> 1) & 1].dstaddr;
}


//-------------------------------------------------
//  capture_control_w - write sound capture control
//  register
//-------------------------------------------------

void nds_sound_device::capture_control_w(offs_t offset, u8 data)
{
	offset &= 1;
	const u8 old = m_capture[offset].control;
	m_capture[offset].control = data;
	if (BIT(old ^ m_capture[offset].control, 7))
	{
		if (m_capture[offset].busy())
			m_capture[offset].capture_on();
		else if (!m_capture[offset].busy())
			m_capture[offset].capture_off();
	}
}


//-------------------------------------------------
//  capture_addrlen_w - write sound capture
//  register
//-------------------------------------------------

void nds_sound_device::capture_addrlen_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_capture[(offset >> 1) & 1].addrlen_w(offset & 1, data, mem_mask);
}


//-------------------------------------------------
//  capture_t::addrlen_w - write sound capture
//  register
//-------------------------------------------------

void nds_sound_device::capture_t::addrlen_w(offs_t offset, u32 data, u32 mem_mask)
{
	switch (offset & 1)
	{
	case 0: // Destination Address
		mem_mask &= 0x7ffffff;
		COMBINE_DATA(&dstaddr);
		break;
	case 1: // Buffer Length
		mem_mask &= 0xffff;
		COMBINE_DATA(&length);
		break;
	}
}


//-------------------------------------------------
//  capture_t::update - update sound capture
//-------------------------------------------------

void nds_sound_device::capture_t::update(int mix)
{
	if (enable)
	{
		int inval = 0;
		// get inputs
		// TODO: hardware bugs aren't emulated, add mode behavior not verified
		if (addmode())
			inval = get_source() ? input->output + output->output : mix;
		else
			inval = get_source() ? input->output : mix;

		// clip output
		inval = std::max<int>(-0x8000, std::min<int>(0x7fff, inval));

		// update counter
		counter -= CLOCK_DIVIDE;
		while (counter <= output->freq)
		{
			// write to memory; TODO: verify write behavior
			if (format()) // 8 bit output
			{
				switch (cur_bitaddr)
				{
				case 0:
					fifo[(fifo_head + (cur_bitaddr >> 5)) & 7].sb.l = (inval >> 8) & 0xff;
					break;
				case 8:
					fifo[(fifo_head + (cur_bitaddr >> 5)) & 7].sb.h = (inval >> 8) & 0xff;
					break;
				case 16:
					fifo[(fifo_head + (cur_bitaddr >> 5)) & 7].sb.h2 = (inval >> 8) & 0xff;
					break;
				case 24:
					fifo[(fifo_head + (cur_bitaddr >> 5)) & 7].sb.h3 = (inval >> 8) & 0xff;
					break;
				default:
					break;
				}
				cur_bitaddr += 8;
			}
			else
			{
				switch (cur_bitaddr)
				{
				case 0:
					fifo[(fifo_head + (cur_bitaddr >> 4)) & 7].sw.l = inval & 0xffff;
					break;
				case 16:
					fifo[(fifo_head + (cur_bitaddr >> 4)) & 7].sw.h = inval & 0xffff;
					break;
				default:
					break;
				}
				cur_bitaddr += 16;
			}

			// update address
			while (cur_bitaddr >= 32)
			{
				// clear FIFO empty flag
				fifo_empty = false;

				// advance FIFO head position
				fifo_head = (fifo_head + 1) & 7;
				if ((fifo_head & fifo_mask()) == (fifo_tail & fifo_mask()))
					fifo_full = true;

				// update loop
				if (++cur_addr >= length)
				{
					if (repeat())
						cur_addr = 0;
					else
						capture_off();
				}

				if (fifo_full)
				{
					// execute FIFO
					fifo_write();

					// check repeat
					if (cur_waddr >= length && repeat())
						cur_waddr = 0;
				}

				cur_bitaddr -= 32;
			}
			counter += 0x10000 - output->freq;
		}
	}
}


//-------------------------------------------------
//  capture_t::fifo_write - write FIFO data to
//  memory, return value is empty flag
//-------------------------------------------------

bool nds_sound_device::capture_t::fifo_write()
{
	if (fifo_empty)
		return true;

	// clear FIFO full flag
	fifo_full = false;

	// write FIFO data to memory
	host->m_space.write_dword(waddr(), fifo[fifo_tail].d);
	cur_waddr++;

	// advance FIFO tail position
	fifo_tail = (fifo_tail + 1) & 7;
	if ((fifo_head & fifo_mask()) == (fifo_tail & fifo_mask()))
		fifo_empty = true;

	return fifo_empty;
}


//-------------------------------------------------
//  capture_t::capture_on - turn on sound capture
//-------------------------------------------------

void nds_sound_device::capture_t::capture_on()
{
	if (!enable)
	{
		enable = true;

		// reset address
		cur_bitaddr = 0;
		cur_addr = cur_waddr = 0;
		counter = 0x10000;

		// reset FIFO
		fifo_head = fifo_tail = 0;
		fifo_empty = true;
		fifo_full = false;
	}
}


//-------------------------------------------------
//  capture_t::capture_off - turn off sound capture
//-------------------------------------------------

void nds_sound_device::capture_t::capture_off()
{
	if (enable)
	{
		// flush FIFO
		while (cur_waddr < length)
		{
			if (fifo_write())
				break;
		}

		enable = false;
		if (busy())
			control &= ~(1 << 7);
	}
}
