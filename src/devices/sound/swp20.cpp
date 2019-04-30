// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP20, rompler

#include "emu.h"
#include "swp20.h"

DEFINE_DEVICE_TYPE(SWP20, swp20_device, "swp20", "Yamaha SWP20 sound chip")

swp20_device::swp20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SWP20, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  device_rom_interface(mconfig, *this, 23+2, ENDIANNESS_LITTLE, 16)
{
}

void swp20_device::device_start()
{
}

void swp20_device::device_reset()
{
	m_p3c_port = 0x00;
	m_p3c_address = true;
}

void swp20_device::rom_bank_updated()
{
}

void swp20_device::map(address_map &map)
{
	map(0x00, 0x3f).rw(FUNC(swp20_device::snd_r), FUNC(swp20_device::snd_w));

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
	logerror("w %02x, %02x %s\n", offset, data, machine().describe_context());
}

void swp20_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
}

