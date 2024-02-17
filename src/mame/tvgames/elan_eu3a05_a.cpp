// license:BSD-3-Clause
// copyright-holders:David Haywood

// Format not understood, it is not OKI ADPCM or IMA ADPCM, maybe something more basic?

#include "emu.h"
#include "elan_eu3a05_a.h"

DEFINE_DEVICE_TYPE(ELAN_EU3A05_SOUND, elan_eu3a05_sound_device, "elan_eu3a05sound", "Elan EU3A05 / EU3A14 Sound")

#define LOG_AUDIO       (1U << 1)

#define LOG_ALL         (LOG_AUDIO)

#define VERBOSE         (0)
#include "logmacro.h"


elan_eu3a05_sound_device::elan_eu3a05_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ELAN_EU3A05_SOUND, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	device_memory_interface(mconfig, *this),
	m_space_config("regs", ENDIANNESS_NATIVE, 8, 6, 0, address_map_constructor(FUNC(elan_eu3a05_sound_device::map), this)),
	m_stream(nullptr),
	m_space_read_cb(*this, 0),
	m_sound_end_cb(*this)
{
}

device_memory_interface::space_config_vector elan_eu3a05_sound_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void elan_eu3a05_sound_device::map(address_map &map)
{
	map(0x00, 0x3f).rw(FUNC(elan_eu3a05_sound_device::read_unmapped), FUNC(elan_eu3a05_sound_device::write_unmapped));

	map(0x00, 0x11).rw(FUNC(elan_eu3a05_sound_device::elan_eu3a05_sound_addr_r), FUNC(elan_eu3a05_sound_device::elan_eu3a05_sound_addr_w)); // 6 * 24-bit (3 byte) channel addresses
	map(0x12, 0x23).rw(FUNC(elan_eu3a05_sound_device::elan_eu3a05_sound_size_r), FUNC(elan_eu3a05_sound_device::elan_eu3a05_sound_size_w)); // 6 * 24-bit (3 byte) channel lengths
	map(0x24, 0x24).rw(FUNC(elan_eu3a05_sound_device::reg50a4_r), FUNC(elan_eu3a05_sound_device::reg50a4_w)); // unknown TODO
	map(0x25, 0x25).rw(FUNC(elan_eu3a05_sound_device::elan_eu3a05_sound_trigger_r), FUNC(elan_eu3a05_sound_device::elan_eu3a05_sound_trigger_w));
	map(0x26, 0x27).rw(FUNC(elan_eu3a05_sound_device::elan_eu3a05_sound_volume_r), FUNC(elan_eu3a05_sound_device::elan_eu3a05_sound_volume_w)); // 0x26 = volume channels 0,1,2,3  0x27 = volume channels 5,6  (lunar rescue sets 0x03 0x00 and just uses a single channel)
	map(0x28, 0x28).r(FUNC(elan_eu3a05_sound_device::elan_eu3a05_50a8_r)); // stopped status? (read only?)
	map(0x29, 0x29).rw(FUNC(elan_eu3a05_sound_device::reg50a9_r), FUNC(elan_eu3a05_sound_device::reg50a9_w)); // IRQ mask?

	// no other reads/writes seen?
}



void elan_eu3a05_sound_device::device_start()
{
	m_stream = stream_alloc(0, 1, 8000);

	save_item(NAME(m_sound_byte_address));
	save_item(NAME(m_sound_byte_len));
	save_item(NAME(m_sound_current_nib_pos));
	save_item(NAME(m_isstopped));
	save_item(NAME(m_sound_trigger));
	save_item(NAME(m_sound_unk));
	save_item(NAME(m_volumes));
	save_item(NAME(m_50a4));
	save_item(NAME(m_50a9));
}

void elan_eu3a05_sound_device::device_reset()
{
	for (int i = 0; i < 6; i++)
	{
		m_sound_byte_address[i] = 0;
		m_sound_byte_len[i] = 0;
		m_sound_current_nib_pos[i] = 0;
		m_adpcm[i].reset();
	}

	m_isstopped = 0x3f;

	m_sound_trigger = 0x00;
	m_sound_unk = 0x00;

	m_volumes[0] = 0xff;
	m_volumes[1] = 0x0f;

	m_50a4 = 0x00;
	m_50a9 = 0x00;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void elan_eu3a05_sound_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	// reset the output stream
	outputs[0].fill(0);

	int volume = m_volumes[0] | (m_volumes[1] << 8);

	int outpos = 0;
	// loop while we still have samples to generate
	int samples = outputs[0].samples();
	while (samples-- != 0)
	{
		int total = 0;
		for (int channel = 0; channel < 6; channel++)
		{
			if (!((m_isstopped >> channel) & 1))
			{
				//LOGMASKED( LOG_AUDIO, "m_isstopped %02x channel %d is active %08x %06x\n", m_isstopped, channel, m_sound_byte_address[channel], m_sound_current_nib_pos[channel]);

				int readoffset = m_sound_byte_address[channel] + (m_sound_current_nib_pos[channel] / 2);

				int nibble = m_space_read_cb(readoffset);

				nibble = nibble >> ((m_sound_current_nib_pos[channel] & 1) ? 0 : 4);
				uint16_t decoded = (uint16_t)m_adpcm[channel].clock(nibble & 0xf);
				decoded = (decoded * ((volume >> (channel * 2)) & 3)) / 3;
				decoded <<= 4;

				total += (int32_t)(int16_t)decoded - 0x8000;

				m_sound_current_nib_pos[channel]++;

				if (m_sound_current_nib_pos[channel] >= m_sound_byte_len[channel] * 2)
				{
					m_sound_current_nib_pos[channel] = 0;
					m_adpcm[channel].reset();
					m_isstopped |= (1 << channel);

					// maybe, seems to match the system IRQ mask when the sound interrupts are enabled?
					if (m_50a9 & (1 << channel))
						m_sound_end_cb[channel](1); // generate interrupt based on which channel just stopped?
				}
			}
			else
			{
				//LOGMASKED( LOG_AUDIO, "m_isstopped %02x channel %d is NOT active %08x %06x\n", m_isstopped, channel, m_sound_byte_address[channel], m_sound_current_nib_pos[channel]);
			}
		}
		outputs[0].put_int(outpos, total, 32768 * 6);
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

void elan_eu3a05_sound_device::elan_eu3a05_sound_addr_w(offs_t offset, uint8_t data)
{
	m_stream->update();
	handle_sound_addr_w(offset / 3, offset % 3, data);
}

uint8_t elan_eu3a05_sound_device::elan_eu3a05_sound_addr_r(offs_t offset)
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

void elan_eu3a05_sound_device::elan_eu3a05_sound_size_w(offs_t offset, uint8_t data)
{
	m_stream->update();
	handle_sound_size_w(offset / 3, offset % 3, data);
}

uint8_t elan_eu3a05_sound_device::elan_eu3a05_sound_size_r(offs_t offset)
{
	m_stream->update();
	return handle_sound_size_r(offset / 3, offset % 3);
}

uint8_t elan_eu3a05_sound_device::elan_eu3a05_sound_trigger_r()
{
	m_stream->update();

	LOGMASKED( LOG_AUDIO, "%s: sound read from trigger?\n", machine().describe_context());
	return m_sound_trigger;
}


void elan_eu3a05_sound_device::elan_eu3a05_sound_trigger_w(uint8_t data)
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
uint8_t elan_eu3a05_sound_device::elan_eu3a05_sound_unk_r()
{
	LOGMASKED( LOG_AUDIO, "%s: elan_eu3a05_sound_unk_r\n", machine().describe_context());
	// don't think this reads back what was written probably a status of something instead?
	return 0x00; //m_sound_unk;
}

void elan_eu3a05_sound_device::elan_eu3a05_sound_unk_w(uint8_t data)
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
		if (m_sound_byte_len[which])
		{
			m_sound_current_nib_pos[which] = 0;
			m_isstopped &= ~(1 << which);
			m_adpcm[which].reset();
		}
	}
}


uint8_t elan_eu3a05_sound_device::elan_eu3a05_50a8_r()
{
	m_stream->update();

	LOGMASKED( LOG_AUDIO, "%s: elan_eu3a05_50a8_r\n", machine().describe_context());
	// batvgc checks bit 0x80
	return m_isstopped | 0xc0;
}

uint8_t elan_eu3a05_sound_device::elan_eu3a05_sound_volume_r(offs_t offset)
{
	LOGMASKED( LOG_AUDIO, "%s: sound_volume_r (offset %d, data %02x)\n", machine().describe_context(), offset, m_volumes[offset]);
	return m_volumes[offset];
}

void elan_eu3a05_sound_device::elan_eu3a05_sound_volume_w(offs_t offset, uint8_t data)
{
	m_stream->update();

	LOGMASKED( LOG_AUDIO, "%s: sound_volume_w (offset %d, data %02x)\n", machine().describe_context(), offset, data);
	m_volumes[offset] = data;
}

uint8_t elan_eu3a05_sound_device::read_unmapped(offs_t offset)
{
	LOGMASKED( LOG_AUDIO, "%s: elan_eu3a05_sound_device::read_unmapped (offset %02x)\n", machine().describe_context(), offset);
	return 0x00;
}

void elan_eu3a05_sound_device::write_unmapped(offs_t offset, uint8_t data)
{
	LOGMASKED( LOG_AUDIO, "%s: elan_eu3a05_sound_device::write_unmapped (offset %02x) (data %02x)\n", machine().describe_context(), offset, data);
}

uint8_t elan_eu3a05_sound_device::reg50a4_r()
{
	LOGMASKED( LOG_AUDIO, "%s: reg50a4_r (unknown) (data %02x)\n", machine().describe_context(), m_50a4);
	return m_50a4;
}

void elan_eu3a05_sound_device::reg50a4_w(uint8_t data)
{
	LOGMASKED( LOG_AUDIO, "%s: reg50a4_w (unknown) (data %02x)\n", machine().describe_context(), data);
	m_50a4 = data;
}

uint8_t elan_eu3a05_sound_device::reg50a9_r()
{
	LOGMASKED( LOG_AUDIO, "%s: reg50a9_r (IRQ mask?) (data %02x)\n", machine().describe_context(), m_50a9);
	return m_50a9;
}

void elan_eu3a05_sound_device::reg50a9_w(uint8_t data)
{
	LOGMASKED( LOG_AUDIO, "%s: reg50a4_w (IRQ mask?) (data %02x)\n", machine().describe_context(), data);
	m_50a9 = data;
}

