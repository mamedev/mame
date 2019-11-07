// license:BSD-3-Clause
// copyright-holders:David Haywood

// Format not understood, it is not OKI ADPCM or IMA ADPCM, maybe something more basic?

#include "emu.h"
#include "elan_eu3a05.h"

DEFINE_DEVICE_TYPE(ELAN_EU3A05_SOUND, elan_eu3a05_sound_device, "elan_eu3a05sound", "Elan EU3A05 / EU3A14 Sound")

#define LOG_AUDIO       (1U << 0)

#define LOG_ALL         (LOG_AUDIO)

#define VERBOSE             (0)
#include "logmacro.h"


elan_eu3a05_sound_device::elan_eu3a05_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ELAN_EU3A05_SOUND, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_stream(nullptr),
	m_space_read_cb(*this),
	m_sound_end_cb{ { *this }, { *this }, { *this }, { *this }, { *this }, { *this } }
{
}

void elan_eu3a05_sound_device::device_start()
{
	m_space_read_cb.resolve_safe(0);
	m_stream = stream_alloc(0, 1, 8000);

	for (devcb_write_line &cb : m_sound_end_cb)
		cb.resolve_safe();

	save_item(NAME(m_sound_byte_address));
	save_item(NAME(m_sound_byte_len));
	save_item(NAME(m_sound_current_nib_pos));
	save_item(NAME(m_isstopped));
	save_item(NAME(m_sound_trigger));
	save_item(NAME(m_sound_unk));
	save_item(NAME(m_volumes));
	save_item(NAME(m_5024));
	save_item(NAME(m_50a9));
}

void elan_eu3a05_sound_device::device_reset()
{
	for (int i = 0; i < 6; i++)
	{
		m_sound_byte_address[i] = 0;
		m_sound_byte_len[i] = 0;
		m_sound_current_nib_pos[i] = 0;
	}

	m_isstopped = 0x3f;

	m_sound_trigger = 0x00;
	m_sound_unk = 0x00;

	m_volumes[0] = 0xff;
	m_volumes[1] = 0x0f;

	m_5024 = 0x00;
	m_50a9 = 0x00;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void elan_eu3a05_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// reset the output stream
	memset(outputs[0], 0, samples * sizeof(*outputs[0]));

	int outpos = 0;
	// loop while we still have samples to generate
	while (samples-- != 0)
	{
		for (int channel = 0; channel < 6; channel++)
		{
			if (!((m_isstopped >> channel) & 1))
			{
				//LOGMASKED( LOG_AUDIO, "m_isstopped %02x channel %d is active %08x %06x\n", m_isstopped, channel, m_sound_byte_address[channel], m_sound_current_nib_pos[channel]);

				int readoffset = m_sound_byte_address[channel] + (m_sound_current_nib_pos[channel] / 2);

				int nibble = m_space_read_cb(readoffset);

				nibble = nibble >> ((m_sound_current_nib_pos[channel] & 1) ? 0 : 4);
				nibble &= 0x0f;

				// it's actually some form of ADPCM? but apparently NOT the OKI ADPCM
				if (nibble & 0x08)
					nibble -= 0x10;

				outputs[0][outpos] += nibble * 0x100;

				m_sound_current_nib_pos[channel]++;

				if (m_sound_current_nib_pos[channel] >= m_sound_byte_len[channel] * 2)
				{
					m_sound_current_nib_pos[channel] = 0;
					m_isstopped |= (1 << channel);

					m_sound_end_cb[channel](1); // generate interrupt based on which channel just stopped?
				}
			}
			else
			{
				//LOGMASKED( LOG_AUDIO, "m_isstopped %02x channel %d is NOT active %08x %06x\n", m_isstopped, channel, m_sound_byte_address[channel], m_sound_current_nib_pos[channel]);
			}
		}
		outpos++;
	}
}




void elan_eu3a05_sound_device::handle_sound_addr_w(int which, int offset, uint8_t data)
{
	switch (offset)
	{
	case 0x00:
		m_sound_byte_address[which] = (m_sound_byte_address[which] & 0xffff00) | (data<<0);
		LOGMASKED( LOG_AUDIO, "%s: sound_0 (%d) write lo address %02x (real address is now %08x)\n", machine().describe_context(), which, data, m_sound_byte_address[which]);
		break;

	case 0x01:
		m_sound_byte_address[which] = (m_sound_byte_address[which] & 0xff00ff) | (data<<8);
		LOGMASKED( LOG_AUDIO, "%s: sound_0 (%d) write md address %02x (real address is now %08x)\n", machine().describe_context(), which, data, m_sound_byte_address[which]);
		break;

	case 0x02:
		m_sound_byte_address[which] = (m_sound_byte_address[which] & 0x00ffff) | (data<<16);
		LOGMASKED( LOG_AUDIO, "%s: sound_0 (%d) write hi address %02x (real address is now %08x)\n", machine().describe_context(), which, data, m_sound_byte_address[which]);
		break;
	}
}

uint8_t elan_eu3a05_sound_device::handle_sound_addr_r(int which, int offset)
{
	switch (offset)
	{
	case 0x00:
		LOGMASKED( LOG_AUDIO, "%s: sound_0 (%d) read lo address\n", machine().describe_context(), which);
		return (m_sound_byte_address[which]>>0) & 0xff;

	case 0x01:
		LOGMASKED( LOG_AUDIO, "%s: sound_0 (%d) read mid address\n", machine().describe_context(), which);
		return (m_sound_byte_address[which]>>8) & 0xff;

	case 0x02:
		LOGMASKED( LOG_AUDIO, "%s: sound_0 (%d) read hi address\n", machine().describe_context(), which);
		return (m_sound_byte_address[which]>>16) & 0xff;
	}

	return 0x00;
}

WRITE8_MEMBER(elan_eu3a05_sound_device::elan_eu3a05_sound_addr_w)
{
	m_stream->update();
	handle_sound_addr_w(offset / 3, offset % 3, data);
}

READ8_MEMBER(elan_eu3a05_sound_device::elan_eu3a05_sound_addr_r)
{
	m_stream->update();
	return handle_sound_addr_r(offset / 3, offset % 3);
}

void elan_eu3a05_sound_device::handle_sound_size_w(int which, int offset, uint8_t data)
{
	switch (offset)
	{
	case 0x00:
		m_sound_byte_len[which] = (m_sound_byte_len[which] & 0xffff00) | (data<<0);
		LOGMASKED( LOG_AUDIO, "%s: sound_1 (%d) write lo size %02x (real size is now %08x)\n", machine().describe_context(), which, data, m_sound_byte_len[which]);
		break;

	case 0x01:
		m_sound_byte_len[which] = (m_sound_byte_len[which] & 0xff00ff) | (data<<8);
		LOGMASKED( LOG_AUDIO, "%s: sound_1 (%d) write md size %02x (real size is now %08x)\n", machine().describe_context(), which, data, m_sound_byte_len[which]);
		break;

	case 0x02:
		m_sound_byte_len[which] = (m_sound_byte_len[which] & 0x00ffff) | (data<<16);
		LOGMASKED( LOG_AUDIO, "%s: sound_1 (%d) write hi size %02x (real size is now %08x)\n", machine().describe_context(), which, data, m_sound_byte_len[which]);
		break;
	}
}

uint8_t elan_eu3a05_sound_device::handle_sound_size_r(int which, int offset)
{
	switch (offset)
	{
	case 0x00:
		LOGMASKED( LOG_AUDIO, "%s: sound_1 (%d) read lo size\n", machine().describe_context(), which);
		return (m_sound_byte_len[which]>>0) & 0xff;

	case 0x01:
		LOGMASKED( LOG_AUDIO, "%s: sound_1 (%d) read mid size\n", machine().describe_context(), which);
		return (m_sound_byte_len[which]>>8) & 0xff;

	case 0x02:
		LOGMASKED( LOG_AUDIO, "%s: sound_1 (%d) read hi size\n", machine().describe_context(), which);
		return (m_sound_byte_len[which]>>16) & 0xff;
	}

	return 0x00;
}

WRITE8_MEMBER(elan_eu3a05_sound_device::elan_eu3a05_sound_size_w)
{
	m_stream->update();
	handle_sound_size_w(offset / 3, offset % 3, data);
}

READ8_MEMBER(elan_eu3a05_sound_device::elan_eu3a05_sound_size_r)
{
	m_stream->update();
	return handle_sound_size_r(offset / 3, offset % 3);
}

READ8_MEMBER(elan_eu3a05_sound_device::elan_eu3a05_sound_trigger_r)
{
	m_stream->update();

	LOGMASKED( LOG_AUDIO, "%s: sound read from trigger?\n", machine().describe_context());
	return m_sound_trigger;
}


WRITE8_MEMBER(elan_eu3a05_sound_device::elan_eu3a05_sound_trigger_w)
{
	m_stream->update();

	LOGMASKED( LOG_AUDIO, "%s: sound write to trigger? %02x\n", machine().describe_context(), data);
	m_sound_trigger = data;

	for (int i = 0; i < 6; i++)
	{
		int bit = (data >> i) & 1;

		if (bit)
			handle_sound_trigger(i);
	}

	if (data & 0xc0)
		LOGMASKED( LOG_AUDIO, "  UNEXPECTED BITS SET");
}

/* this is read/written with the same individual bits for each channel as the trigger
   maybe related to interrupts? */
READ8_MEMBER(elan_eu3a05_sound_device::elan_eu3a05_sound_unk_r)
{
	LOGMASKED( LOG_AUDIO, "%s: elan_eu3a05_sound_unk_r\n", machine().describe_context());
	// don't think this reads back what was written probably a status of something instead?
	return 0x00; //m_sound_unk;
}

WRITE8_MEMBER(elan_eu3a05_sound_device::elan_eu3a05_sound_unk_w)
{
	LOGMASKED( LOG_AUDIO, "%s: elan_eu3a05_sound_unk_w %02x\n", machine().describe_context(), data);

	for (int i = 0; i < 6; i++)
	{
		int bit = (data >> i) & 1;

		if (bit)
			LOGMASKED( LOG_AUDIO, "(unknown operation on channel %d)\n", i);
	}

	m_sound_unk = data;

	if (data & 0xc0)
		LOGMASKED( LOG_AUDIO, "  UNEXPECTED BITS SET");
}

void elan_eu3a05_sound_device::handle_sound_trigger(int which)
{
	LOGMASKED( LOG_AUDIO, "Triggering operation on channel (%d) with params %08x %08x\n", which, m_sound_byte_address[which], m_sound_byte_len[which]);

	if (m_isstopped & (1 << which)) // golden tee will repeatedly try to start the music on the title screen (although could depend on a status read first?)
	{
		m_sound_current_nib_pos[which] = 0;
		m_isstopped &= ~(1 << which);
	}
}


READ8_MEMBER(elan_eu3a05_sound_device::elan_eu3a05_50a8_r)
{
	m_stream->update();

	LOGMASKED( LOG_AUDIO, "%s: elan_eu3a05_50a8_r\n", machine().describe_context());
	return m_isstopped;
}

READ8_MEMBER(elan_eu3a05_sound_device::elan_eu3a05_sound_volume_r)
{
	return m_volumes[offset];
}

WRITE8_MEMBER(elan_eu3a05_sound_device::elan_eu3a05_sound_volume_w)
{
	m_volumes[offset] = data;
}

WRITE8_MEMBER(elan_eu3a05_sound_device::write)
{
	switch (offset)
	{
	case 0x00: case 0x01: case 0x02: // channel 0 address
	case 0x03: case 0x04: case 0x05: // channel 1 address
	case 0x06: case 0x07: case 0x08: // channel 2 address
	case 0x09: case 0x0a: case 0x0b: // channel 3 address
	case 0x0c: case 0x0d: case 0x0e: // channel 4 address
	case 0x0f: case 0x10: case 0x11: // channel 5 address
		elan_eu3a05_sound_addr_w(space, offset, data);
		break;

	case 0x12: case 0x13: case 0x14: // channel 0 length
	case 0x15: case 0x16: case 0x17: // channel 1 length
	case 0x18: case 0x19: case 0x1a: // channel 2 length
	case 0x1b: case 0x1c: case 0x1d: // channel 3 length
	case 0x1e: case 0x1f: case 0x20: // channel 4 length
	case 0x21: case 0x22: case 0x23: // channel 5 length
		elan_eu3a05_sound_size_w(space, offset - 0x12, data);
		break;

	case 0x24: // unk
		m_5024 = data;
		break;

	case 0x25: // trigger
		elan_eu3a05_sound_trigger_w(space, offset - 0x25, data);
		break;

	case 0x26: // volume channels 0,1,2,3 ? (lunar rescue sets 0x03 here and 0x00 below and just uses a single channel)
	case 0x27: // volume channels 5,6 ?
		elan_eu3a05_sound_volume_w(space, offset - 0x26, data);
		break;

	case 0x28: // stopped status?
		LOGMASKED( LOG_AUDIO, "%s: write to stop state register? %02x\n", machine().describe_context(), data);
		break;

	case 0x29: // interrupt enable? or looping?
		m_50a9 = data;
		break;
	}
}

READ8_MEMBER(elan_eu3a05_sound_device::read)
{
	uint8_t ret = 0x00;

	switch (offset)
	{
	case 0x00: case 0x01: case 0x02: // channel 0 address
	case 0x03: case 0x04: case 0x05: // channel 1 address
	case 0x06: case 0x07: case 0x08: // channel 2 address
	case 0x09: case 0x0a: case 0x0b: // channel 3 address
	case 0x0c: case 0x0d: case 0x0e: // channel 4 address
	case 0x0f: case 0x10: case 0x11: // channel 5 address
		ret = elan_eu3a05_sound_addr_r(space, offset);
		break;

	case 0x12: case 0x13: case 0x14: // channel 0 length
	case 0x15: case 0x16: case 0x17: // channel 1 length
	case 0x18: case 0x19: case 0x1a: // channel 2 length
	case 0x1b: case 0x1c: case 0x1d: // channel 3 length
	case 0x1e: case 0x1f: case 0x20: // channel 4 length
	case 0x21: case 0x22: case 0x23: // channel 5 length
		ret = elan_eu3a05_sound_size_r(space, offset - 0x12);
		break;

	case 0x24: // unk
		ret = m_5024;
		break;

	case 0x25: // trigger
		ret = elan_eu3a05_sound_trigger_r(space, offset - 0x25);
		break;

	case 0x26: // volume channels 0,1,2,3 ?
	case 0x27: // volume channels 5,6 ?
		ret = elan_eu3a05_sound_volume_r(space, offset - 0x26);
		break;

	case 0x28: // stopped status?
		ret = elan_eu3a05_50a8_r(space, offset - 0x28);
		break;

	case 0x29: // interrupt enable? or looping?
		ret = m_50a9;
		break;
	}

	return ret;
}