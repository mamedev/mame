// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP20, rompler

#include "emu.h"
#include "swp20.h"

DEFINE_DEVICE_TYPE(SWP20, swp20_device, "swp20", "Yamaha SWP20 sound chip")

swp20_device::swp20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SWP20, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  device_rom_interface(mconfig, *this)
{
}

void swp20_device::device_start()
{
	m_stream = stream_alloc(0, 2, 44100);

	save_item(NAME(m_sample_address));
}

void swp20_device::device_reset()
{
	std::fill(m_sample_address.begin(), m_sample_address.end(), 0);

	m_waverom_access = 0;
	m_waverom_val = 0;

	m_p3c_port = 0x00;
	m_p3c_address = true;
	m_voice = 0x00;
	m_keyon = 0;
	m_keyoff = 0;
}

void swp20_device::map(address_map &map)
{
	map(0x00, 0x3f).rw(FUNC(swp20_device::snd_r), FUNC(swp20_device::snd_w));

	map(0x01, 0x01).w(FUNC(swp20_device::voice_w));

	map(0x2d, 0x2d).rw(FUNC(swp20_device::sample_address_r<2>), FUNC(swp20_device::sample_address_w<2>));
	map(0x2e, 0x2e).rw(FUNC(swp20_device::sample_address_r<1>), FUNC(swp20_device::sample_address_w<1>));
	map(0x2f, 0x2f).rw(FUNC(swp20_device::sample_address_r<0>), FUNC(swp20_device::sample_address_w<0>));

	map(0x37, 0x37).w(FUNC(swp20_device::waverom_access_w));
	map(0x3a, 0x3a).r(FUNC(swp20_device::waverom_val_r<1>));
	map(0x3b, 0x3b).r(FUNC(swp20_device::waverom_val_r<0>));

	map(0x3c, 0x3c).w(FUNC(swp20_device::p3c_w));
}

// init mu80:
//  48394: <- 47aea
//   write 04.7f 00.14 01.90 to +3c
//   write 01.90 80-ff, 473ea++
//   write 01.94 80-ff, 4746a++
//   write 01.98 80-ff, 474ea++
//   write 01.9c 80-ff, 4756a++

//   write 01.a0
//   write 40-5f.data
// etc

void swp20_device::voice_w(u8 data)
{
	m_voice = data & 0x1f;
}

void swp20_device::waverom_access_w(u8 data)
{
	m_waverom_access = data;
}

template<int sel> u8 swp20_device::waverom_val_r()
{
	return read_word(m_sample_address[0x1f]*2) >> (8*sel);
}

template<int sel> void swp20_device::sample_address_w(offs_t offset, u8 data)
{
	m_stream->update();

	m_sample_address[m_voice] = (m_sample_address[m_voice] & ~(0xff << (8*sel))) | (data << (8*sel));
	if(!sel)
		logerror("sample_address[%02x] = %04x\n", m_voice, m_sample_address[m_voice]);
}

template<int sel> u8 swp20_device::sample_address_r(offs_t offset)
{
	return m_sample_address[m_voice] >> (8*sel);
}

void swp20_device::p3c_w(u8 data)
{
	if(m_p3c_address)
		m_p3c_port = data;
	else
		logerror("p3c %02x = %02x\n", m_p3c_port, data);

	m_p3c_address = !m_p3c_address;
}

u8 swp20_device::snd_r(offs_t offset)
{
	logerror("r %02x %s\n", offset, machine().describe_context());
	return 0;
}

void swp20_device::snd_w(offs_t offset, u8 data)
{
	// Registers 0-f are global, 10-3f per-voice
	switch(offset) {
	case 0x04: case 0x05: case 0x06: case 0x07: {
		int off = 8*(offset & 3);
		u32 mask = 0xff << off;
		m_keyon = (m_keyon & ~mask) | (data << off);
		logerror("keyon %08x\n", m_keyon);
		break;
	}
	case 0x08: case 0x09: case 0x0a: case 0x0b: {
		int off = 8*(offset & 3);
		u32 mask = 0xff << off;
		m_keyoff = (m_keyoff & ~mask) | (data << off);
		logerror("keyoff %08x\n", m_keyoff);
		break;
	}

	default:
		logerror("w %02x.%02x, %02x %s\n", m_voice, offset, data, machine().describe_context());
	}
}

void swp20_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	outputs[0].fill(0);
}

