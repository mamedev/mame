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

	m_eq_port = 0x00;
	m_eq_address = true;
	m_voice = 0x00;
	m_keyon = 0;
	m_keyoff = 0;
}

void swp20_device::map(address_map &map)
{
	map(0x00, 0x3f).rw(FUNC(swp20_device::snd_r), FUNC(swp20_device::snd_w));

	map(0x01, 0x01).w(FUNC(swp20_device::voice_w));

	map(0x10, 0x10).rw(FUNC(swp20_device::pitch_r<1>), FUNC(swp20_device::pitch_w<1>));
	map(0x11, 0x11).rw(FUNC(swp20_device::pitch_r<0>), FUNC(swp20_device::pitch_w<0>));

	map(0x14, 0x14).rw(FUNC(swp20_device::pan_l_r), FUNC(swp20_device::pan_l_w));
	map(0x15, 0x15).rw(FUNC(swp20_device::pan_r_r), FUNC(swp20_device::pan_r_w));

	map(0x26, 0x26).rw(FUNC(swp20_device::sample_start_r<2>), FUNC(swp20_device::sample_start_w<2>));
	map(0x27, 0x27).rw(FUNC(swp20_device::sample_start_r<1>), FUNC(swp20_device::sample_start_w<1>));
	map(0x28, 0x28).rw(FUNC(swp20_device::sample_start_r<0>), FUNC(swp20_device::sample_start_w<0>));
	map(0x29, 0x29).rw(FUNC(swp20_device::sample_format_r), FUNC(swp20_device::sample_format_w));
	map(0x2a, 0x2a).rw(FUNC(swp20_device::sample_end_r<1>), FUNC(swp20_device::sample_end_w<1>));
	map(0x2b, 0x2b).rw(FUNC(swp20_device::sample_end_r<0>), FUNC(swp20_device::sample_end_w<0>));
	map(0x2c, 0x2c).rw(FUNC(swp20_device::sample_address_r<3>), FUNC(swp20_device::sample_address_w<3>));
	map(0x2d, 0x2d).rw(FUNC(swp20_device::sample_address_r<2>), FUNC(swp20_device::sample_address_w<2>));
	map(0x2e, 0x2e).rw(FUNC(swp20_device::sample_address_r<1>), FUNC(swp20_device::sample_address_w<1>));
	map(0x2f, 0x2f).rw(FUNC(swp20_device::sample_address_r<0>), FUNC(swp20_device::sample_address_w<0>));

	map(0x37, 0x37).w(FUNC(swp20_device::waverom_access_w));
	map(0x3a, 0x3a).r(FUNC(swp20_device::waverom_val_r<1>));
	map(0x3b, 0x3b).r(FUNC(swp20_device::waverom_val_r<0>));

	map(0x3c, 0x3c).w(FUNC(swp20_device::eq_w));
}

void swp20_device::voice_w(u8 data)
{
	// Code uses 20-3f for voices on the second swp, it looks like
	// just leaking internal information and the top bits are not
	// significant

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

void swp20_device::pan_l_w(u8 data)
{
	m_stream->update();
	m_pan_l[m_voice] = data;
}

u8 swp20_device::pan_l_r()
{
	return m_pan_l[m_voice];
}

void swp20_device::pan_r_w(u8 data)
{
	m_stream->update();
	m_pan_r[m_voice] = data;
}

u8 swp20_device::pan_r_r()
{
	return m_pan_r[m_voice];
}

template<int sel> void swp20_device::pitch_w(u8 data)
{
	m_stream->update();
	m_pitch[m_voice] = (m_pitch[m_voice] & ~(0xff << (8*sel))) | (data << (8*sel));
}

template<int sel> u8 swp20_device::pitch_r()
{
	return m_pitch[m_voice] >> (8*sel);
}
template<int sel> void swp20_device::sample_start_w(u8 data)
{
	m_stream->update();

	m_sample_start[m_voice] = (m_sample_start[m_voice] & ~(0xff << (8*sel))) | (data << (8*sel));
	//  if(!sel)
	//      logerror("sample_start[%02x] = %04x\n", m_voice, m_sample_start[m_voice]);
}

template<int sel> u8 swp20_device::sample_start_r()
{
	return m_sample_start[m_voice] >> (8*sel);
}

template<int sel> void swp20_device::sample_end_w(u8 data)
{
	m_stream->update();

	m_sample_end[m_voice] = (m_sample_end[m_voice] & ~(0xff << (8*sel))) | (data << (8*sel));
	//  if(!sel)
	//      logerror("sample_end[%02x] = %04x\n", m_voice, m_sample_end[m_voice]);
}

template<int sel> u8 swp20_device::sample_end_r()
{
	return m_sample_end[m_voice] >> (8*sel);
}

void swp20_device::sample_format_w(u8 data)
{
	m_stream->update();

	m_sample_format[m_voice] = data;
}

u8 swp20_device::sample_format_r()
{
	return m_sample_format[m_voice];
}

template<int sel> void swp20_device::sample_address_w(u8 data)
{
	m_stream->update();

	m_sample_address[m_voice] = (m_sample_address[m_voice] & ~(0xff << (8*sel))) | (data << (8*sel));
	if(!sel)
		logerror("sample_address[%02x] = %04x\n", m_voice, m_sample_address[m_voice]);
}

template<int sel> u8 swp20_device::sample_address_r()
{
	return m_sample_address[m_voice] >> (8*sel);
}

void swp20_device::eq_w(u8 data)
{
	if(m_eq_address)
		m_eq_port = data;
	else {
		if(0)
			logerror("eq %02x = %02x\n", m_eq_port, data);
	}

	m_eq_address = !m_eq_address;
}

u8 swp20_device::snd_r(offs_t offset)
{
	//  logerror("r %02x %s\n", offset, machine().describe_context());
	return 0;
}

static u8 rr[0x20*0x40];

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

	default: {
		if(data != rr[m_voice * 0x40 + offset]) {
			rr[m_voice * 0x40 + offset] = data;
			if(offset != 0x34)
				logerror("w %02x.%02x, %02x %s\n", m_voice, offset, data, machine().describe_context());
		}
	}
	}
}

void swp20_device::sound_stream_update(sound_stream &stream)
{
}

