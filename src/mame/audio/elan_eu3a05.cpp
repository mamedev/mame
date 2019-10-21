// license:BSD-3-Clause
// copyright-holders:David Haywood

// Format not understood, it is not OKI ADPCM or IMA ADPCM, maybe something more basic?

#include "emu.h"
#include "elan_eu3a05.h"

DEFINE_DEVICE_TYPE(RADICA6502_SOUND, radica6502_sound_device, "radica6502sound", "Elan EU3A05 / EU3A14 Sound")

#define LOG_AUDIO       (1U << 0)

#define LOG_ALL         (LOG_AUDIO)

#define VERBOSE             (0)
#include "logmacro.h"


radica6502_sound_device::radica6502_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, RADICA6502_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_space_read_cb(*this)
{
}

void radica6502_sound_device::device_start()
{
	m_space_read_cb.resolve_safe(0);
	m_stream = stream_alloc(0, 1, 8000);
}

void radica6502_sound_device::device_reset()
{
	for (int i = 0; i < 6; i++)
	{
		m_sound_byte_address[i] = 0;
		m_sound_byte_len[i] = 0;
		m_sound_current_nib_pos[i] = 0;
	}

	m_isstopped = 0x3f;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void radica6502_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
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

					// maybe should generate an interrupt with vector
					// ffb8, ffbc, ffc0, ffc4, ffc8, or ffcc depending on which channel finished??
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


void radica6502_sound_device::handle_sound_addr_w(int which, int offset, uint8_t data)
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

uint8_t radica6502_sound_device::handle_sound_addr_r(int which, int offset)
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

WRITE8_MEMBER(radica6502_sound_device::radicasi_sound_addr_w)
{
	m_stream->update();
	handle_sound_addr_w(offset / 3, offset % 3, data);
}

READ8_MEMBER(radica6502_sound_device::radicasi_sound_addr_r)
{
	m_stream->update();
	return handle_sound_addr_r(offset / 3, offset % 3);
}

void radica6502_sound_device::handle_sound_size_w(int which, int offset, uint8_t data)
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

uint8_t radica6502_sound_device::handle_sound_size_r(int which, int offset)
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

WRITE8_MEMBER(radica6502_sound_device::radicasi_sound_size_w)
{
	m_stream->update();
	handle_sound_size_w(offset / 3, offset % 3, data);
}

READ8_MEMBER(radica6502_sound_device::radicasi_sound_size_r)
{
	m_stream->update();
	return handle_sound_size_r(offset / 3, offset % 3);
}

READ8_MEMBER(radica6502_sound_device::radicasi_sound_trigger_r)
{
	m_stream->update();

	LOGMASKED( LOG_AUDIO, "%s: sound read from trigger?\n", machine().describe_context());
	return m_sound_trigger;
}


WRITE8_MEMBER(radica6502_sound_device::radicasi_sound_trigger_w)
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
READ8_MEMBER(radica6502_sound_device::radicasi_sound_unk_r)
{
	LOGMASKED( LOG_AUDIO, "%s: radicasi_sound_unk_r\n", machine().describe_context());
	// don't think this reads back what was written probably a status of something instead?
	return 0x00; //m_sound_unk;
}

WRITE8_MEMBER(radica6502_sound_device::radicasi_sound_unk_w)
{
	LOGMASKED( LOG_AUDIO, "%s: radicasi_sound_unk_w %02x\n", machine().describe_context(), data);

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

void radica6502_sound_device::handle_sound_trigger(int which)
{
	LOGMASKED( LOG_AUDIO, "Triggering operation on channel (%d) with params %08x %08x\n", which, m_sound_byte_address[which], m_sound_byte_len[which]);

	m_sound_current_nib_pos[which] = 0;
	m_isstopped &= ~(1 << which);
}


READ8_MEMBER(radica6502_sound_device::radicasi_50a8_r)
{
	m_stream->update();

	LOGMASKED( LOG_AUDIO, "%s: radicasi_50a8_r\n", machine().describe_context());
	return m_isstopped;
}
