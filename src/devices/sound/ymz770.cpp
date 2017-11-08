// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, MetalliC
/***************************************************************************

    ymz770.c

    Emulation by R. Belmont and MetalliC
    AMM decode by Olivier Galibert

-----
TODO:
- A lot of unimplemented features, even simple ones like panning,
  these should be added once we find out any software that uses it.
- Sequencer is very preliminary
- verify if pan 100% correct
- What does channel ATBL mean?
- Is YMZ774(and other variants) the same family as this chip?
  What are the differences?

***************************************************************************/

#include "emu.h"
#include "ymz770.h"
#include "mpeg_audio.h"

// device type definition
DEFINE_DEVICE_TYPE(YMZ770, ymz770_device, "ymz770", "Yamaha YMZ770C-F AMMS-A")
DEFINE_DEVICE_TYPE(YMZ774, ymz774_device, "ymz774", "Yamaha YMZ774-S AMMS2")

//-------------------------------------------------
//  ymz770_device - constructor
//-------------------------------------------------

ymz770_device::ymz770_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ymz770_device(mconfig, YMZ770, tag, owner, clock, 16000)
{
}

ymz770_device::ymz770_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t sclock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_sclock(sclock)
	, m_cur_reg(0)
	, m_mute(0)
	, m_doen(0)
	, m_vlma(0)
	, m_bsl(0)
	, m_cpl(0)
	, m_rom(*this, DEVICE_SELF)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ymz770_device::device_start()
{
	// create the stream
	m_stream = machine().sound().stream_alloc(*this, 0, 2, m_sclock);

	for (auto & elem : m_channels)
	{
		elem.is_playing = false;
		elem.decoder = new mpeg_audio(&m_rom[0], mpeg_audio::AMM, false, 0);
	}
	for (auto & elem : m_sequences)
		elem.is_seq_playing = false;

	// register for save states
	save_item(NAME(m_cur_reg));
	save_item(NAME(m_mute));
	save_item(NAME(m_doen));
	save_item(NAME(m_vlma));
	save_item(NAME(m_bsl));
	save_item(NAME(m_cpl));

	for (int ch = 0; ch < 16; ch++)	// TODO array size
	{
		save_item(NAME(m_channels[ch].phrase), ch);
		save_item(NAME(m_channels[ch].pan), ch);
		save_item(NAME(m_channels[ch].pan_delay), ch);
		save_item(NAME(m_channels[ch].volume), ch);
		save_item(NAME(m_channels[ch].volume_delay), ch);
		save_item(NAME(m_channels[ch].volume2), ch);
		save_item(NAME(m_channels[ch].loop), ch);
		save_item(NAME(m_channels[ch].is_playing), ch);
		save_item(NAME(m_channels[ch].last_block), ch);
		save_item(NAME(m_channels[ch].output_remaining), ch);
		save_item(NAME(m_channels[ch].output_ptr), ch);
		save_item(NAME(m_channels[ch].atbl), ch);
		save_item(NAME(m_channels[ch].pptr), ch);
		save_item(NAME(m_channels[ch].output_data), ch);
	}
	for (int ch = 0; ch < 8; ch++)
	{
		save_item(NAME(m_sequences[ch].sequence), ch);
		save_item(NAME(m_sequences[ch].seqcontrol), ch);
		save_item(NAME(m_sequences[ch].seqdelay), ch);
		save_item(NAME(m_sequences[ch].is_seq_playing), ch);
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ymz770_device::device_reset()
{
	for (auto & elem : m_channels)
	{
		elem.phrase = 0;
		elem.pan = 64;
		elem.pan_delay = 0;
		elem.volume = 0;
		elem.volume_delay = 0;
		elem.volume2 = 0;
		elem.loop = 0;
		elem.is_playing = false;
		elem.output_remaining = 0;
		elem.decoder->clear();
	}
	for (auto & elem : m_sequences)
	{
		elem.sequence = 0;
		elem.seqcontrol = 0;
		elem.seqdelay = 0;
		elem.is_seq_playing = false;
	}
}


//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void ymz770_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *outL, *outR;

	outL = outputs[0];
	outR = outputs[1];

	for (int i = 0; i < samples; i++)
	{
		sequencer();

		// process channels
		int32_t mixl = 0;
		int32_t mixr = 0;

		for (auto & elem : m_channels)
		{
			if (elem.output_remaining > 0)
			{
				// force finish current block
				int32_t smpl = elem.output_data[elem.output_ptr++] * elem.volume;	// volume is linear, 0 - 128 (100%)
				mixr += (smpl * elem.pan) >> 14;	// pan seems linear, 0 - 128, where 0 = 100% left, 128 = 100% right, 64 = 50% left 50% right
				mixl += (smpl * (128 - elem.pan)) >> 14;
				elem.output_remaining--;

				if (elem.output_remaining == 0 && !elem.is_playing)
					elem.decoder->clear();
			}

			else if (elem.is_playing)
			{
retry:
				if (elem.last_block)
				{
					if (elem.loop)
					{
						if (elem.loop != 255)
							--elem.loop;
						// loop sample
						int phrase = elem.phrase;
						elem.atbl = m_rom[(4*phrase)+0] >> 4 & 7;
						elem.pptr = 8 * get_phrase_offs(phrase);
					}
					else
					{
						elem.is_playing = false;
						elem.output_remaining = 0;
						elem.decoder->clear();
					}
				}

				if (elem.is_playing)
				{
					// next block
					int sample_rate, channel_count;
					if (!elem.decoder->decode_buffer(elem.pptr, m_rom.bytes()*8, elem.output_data, elem.output_remaining, sample_rate, channel_count) || elem.output_remaining == 0)
					{
						elem.is_playing = !elem.last_block; // detect infinite retry loop
						elem.last_block = true;
						elem.output_remaining = 0;
						goto retry;
					}

					elem.last_block = elem.output_remaining < 1152;
					elem.output_remaining--;
					elem.output_ptr = 1;

					int32_t smpl = elem.output_data[0] * elem.volume;
					mixr += (smpl * elem.pan) >> 14;
					mixl += (smpl * (128 - elem.pan)) >> 14;
				}
			}
		}

		mixr *= m_vlma;	// main volume is linear, 0 - 255, where 128 = 100%
		mixl *= m_vlma;
		mixr >>= 7 - m_bsl;
		mixl >>= 7 - m_bsl;
		// Clip limiter: 0 - off, 1 - 6.02 dB (100%), 2 - 4.86 dB (87.5%), 3 - 3.52 dB (75%). values taken from YMZ773 docs, might be incorrect for YMZ770.
		constexpr int32_t ClipMax3 = 32768 * 75 / 100;
		constexpr int32_t ClipMax2 = 32768 * 875 / 1000;
		switch (m_cpl)
		{
		case 3:
			mixl = (mixl > ClipMax3) ? ClipMax3 : (mixl < -ClipMax3) ? -ClipMax3 : mixl;
			mixr = (mixr > ClipMax3) ? ClipMax3 : (mixr < -ClipMax3) ? -ClipMax3 : mixr;
			break;
		case 2:
			mixl = (mixl > ClipMax2) ? ClipMax2 : (mixl < -ClipMax2) ? -ClipMax2 : mixl;
			mixr = (mixr > ClipMax2) ? ClipMax2 : (mixr < -ClipMax2) ? -ClipMax2 : mixr;
			break;
		case 1:
			mixl = (mixl > 32767) ? 32767 : (mixl < -32768) ? -32768 : mixl;
			mixr = (mixr > 32767) ? 32767 : (mixr < -32768) ? -32768 : mixr;
			break;
		}
		if (m_mute)
			mixr = mixl = 0;
		outL[i] = mixl;
		outR[i] = mixr;
	}
}

void ymz770_device::sequencer()
{
	for (auto & elem : m_sequences)
	{
		if (elem.is_seq_playing)
		{
			if (elem.seqdelay > 0)
			{
				elem.seqdelay--;
			}
			else
			{
				int reg = *elem.seqdata++;
				uint8_t data = *elem.seqdata++;
				switch (reg)
				{
				case 0x0f:
					if (elem.seqcontrol & 1)
					{
						// loop sequence
						uint8_t sqn = elem.sequence;
						uint32_t pptr = get_seq_offs(sqn);
						elem.seqdata = &m_rom[pptr];
					}
					else
					{
						elem.is_seq_playing = false;
					}
					break;
				case 0x0e:
					elem.seqdelay = 32 - 1;
					break;
				default:
					internal_reg_write(reg, data);
					break;
				}
			}
		}
	}
}

//-------------------------------------------------
//  write - write to the chip's registers
//-------------------------------------------------

WRITE8_MEMBER( ymz770_device::write )
{
	if (offset & 1)
	{
		m_stream->update();
		internal_reg_write(m_cur_reg, data);
	}
	else
	{
		m_cur_reg = data;
	}
}


void ymz770_device::internal_reg_write(uint8_t reg, uint8_t data)
{
	// global registers
	if (reg < 0x40)
	{
		switch (reg)
		{
			case 0x00:
				m_mute = data & 1;
				m_doen = data >> 1 & 1;
				break;

			case 0x01:
				m_vlma = data;
				break;

			case 0x02:
				m_bsl = data & 7;
				m_cpl = data >> 4 & 7;
				break;

			// unused
			default:
				break;
		}
	}

	// playback registers
	else if (reg < 0x60)
	{
		int ch = reg >> 2 & 0x07;

		switch (reg & 0x03)
		{
			case 0:
				m_channels[ch].phrase = data;
				break;

			case 1:
				m_channels[ch].volume = data;
				break;

			case 2:
				m_channels[ch].pan = data << 3;
				break;

			case 3:
				if (data & 6)
				{
					uint8_t phrase = m_channels[ch].phrase;
					m_channels[ch].atbl = m_rom[(4*phrase)+0] >> 4 & 7;
					m_channels[ch].pptr = 8 * get_phrase_offs(phrase);
					m_channels[ch].last_block = false;

					m_channels[ch].is_playing = true;
				}
				else
				{
					m_channels[ch].is_playing = false;
				}

				m_channels[ch].loop = (data & 1) ? 255 : 0;
				break;
		}
	}

	// sequencer registers
	else
	{
		int ch = reg >> 4 & 0x07;

		switch (reg & 0x0f)
		{
			case 0:
				m_sequences[ch].sequence = data;
				break;
			case 1:
				if (data & 6)
				{
					uint8_t sqn = m_sequences[ch].sequence;
					uint32_t pptr = get_seq_offs(sqn);
					m_sequences[ch].seqdata = &m_rom[pptr];
					m_sequences[ch].seqdelay = 0;
					m_sequences[ch].is_seq_playing = true;
				}
				else
				{
					m_sequences[ch].is_seq_playing = false;
				}

				m_sequences[ch].seqcontrol = data;
				break;

			default:
				break;
		}
	}
}

//-------------------------------------------------
//  ymz774_device
//-------------------------------------------------

ymz774_device::ymz774_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ymz770_device(mconfig, YMZ774, tag, owner, clock, 44100)
{
}

READ8_MEMBER(ymz774_device::read)
{
	if (offset & 1)
	{
		if (m_cur_reg == 0xe3 || m_cur_reg == 0xe4)
		{
			m_stream->update();
			uint8_t res = 0;
			auto bank = (m_cur_reg == 0xe3) ? 8 : 0;
			for (auto i = 0; i < 8; i++)
				if (m_channels[i + bank].is_playing)
					res |= 1 << i;
			return res;
		}
	}
	logerror("unimplemented read %02X\n", m_cur_reg);
	return 0;
}

void ymz774_device::internal_reg_write(uint8_t reg, uint8_t data)
{
	// playback registers
	if (reg < 0x10)  // phrase num H and L
	{
		int ch = ((reg >> 1) & 7) + m_bank * 8;
		if (reg & 1)
			m_channels[ch].phrase = (m_channels[ch].phrase & 0xff00) | data;
		else
			m_channels[ch].phrase = (m_channels[ch].phrase & 0x00ff) | ((data & 7) << 8);
	}
	else if (reg < 0x60)
	{
		int ch = (reg & 7) + m_bank * 8;
		switch (reg & 0xf8)
		{
		case 0x10: // Volume 1
			m_channels[ch].volume = data;
			break;
		case 0x18: // Volume 1 delayed transition
			if (data) logerror("unimplemented write %02X %02X\n", reg, data);
			m_channels[ch].volume_delay = data;
			break;
		case 0x20: // Volume 2
			m_channels[ch].volume2 = data;
			break;
		case 0x28: // Pan L/R
			m_channels[ch].pan = data;
			break;
		case 0x30: // Pan L/R delayed transition
			if (data) logerror("unimplemented write %02X %02X\n", reg, data);
			m_channels[ch].pan_delay = data;
			break;
		case 0x48: // Loop
			m_channels[ch].loop = data;
			break;
		case 0x50: // Start / Stop
			if (data)
			{
				int phrase = m_channels[ch].phrase;
				m_channels[ch].atbl = m_rom[(4 * phrase) + 0] >> 4 & 7;
				m_channels[ch].pptr = 8 * get_phrase_offs(phrase);
				m_channels[ch].last_block = false;

				m_channels[ch].is_playing = true;
			}
			else
			{
				m_channels[ch].is_playing = false;
			}
			break;
		case 0x58: // Pause / Resume
			if (data) logerror("pause/resume unimplemented %02X %02X\n", reg, data);
			break;
		}
	}
	else if (reg < 0xd0)
	{
		if (m_bank == 0) // Sequencer, is it used in PGM2 ?
		{
			if (data) logerror("sequencer unimplemented %02X %02X\n", reg, data);
		}
		// else bank1 - Equalizer control
	}
	// global registers
	else
	{
		switch (reg) {
		case 0xd0:
			m_vlma = data;
			break;
		case 0xd2:
			m_cpl = data;
			break;
		case 0xf0:
			m_bank = data & 1;
			if (data > 1) logerror("Set bank %02X!\n", data);
			break;
		}
	}
}
