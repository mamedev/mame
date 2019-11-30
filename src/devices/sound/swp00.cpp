// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP00, rompler/dsp combo

#include "emu.h"
#include "swp00.h"

DEFINE_DEVICE_TYPE(SWP00, swp00_device, "swp00", "Yamaha SWP00 (TC170C120SF / XQ036A00) sound chip")

swp00_device::swp00_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SWP00, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  device_rom_interface(mconfig, *this, 25+2, ENDIANNESS_LITTLE, 32)
{
}

void swp00_device::device_add_mconfig(machine_config &config)
{
}

void swp00_device::device_start()
{
	m_stream = stream_alloc(0, 2, 44100);
}

void swp00_device::device_reset()
{
}

void swp00_device::rom_bank_updated()
{
	m_stream->update();
}

void swp00_device::map(address_map &map)
{
	map(0x0000, 0x7ff).rw(FUNC(swp00_device::snd_r), FUNC(swp00_device::snd_w));
}


static u8 rr[0x20*0x20];

u8 swp00_device::snd_r(offs_t offset)
{
	if(0) {
		int chan = (offset >> 6) & 0x3f;
		int slot = offset & 0x3f;
		std::string preg = "-";
		if(slot >= 0x21 && slot <= 0x2b && (slot & 1))
			preg = util::string_format("fp%03x", (slot-0x21)/2 + 6*chan);
		else if(slot == 0x30 || slot == 0x31)
			preg = util::string_format("dt%02x", (slot-0x30) + 2*chan);
		else if(slot == 0x0e || slot == 0x0f)
			preg = util::string_format("ct%02x", (slot-0x0e) + 2*chan);
		else
			preg = util::string_format("%02x.%02x", chan, slot);
		logerror("snd_r [%04x %04x] %-5s, %04x\n", offset, offset*2, preg, rr[offset]);
	}
	if(0 && offset == 0x080f)
		machine().debug_break();
	if(offset == 0x080f)
		return 0;
	return rr[offset];
}

void swp00_device::snd_w(offs_t offset, u8 data)
{
	if(rr[offset] == data)
		return;

	rr[offset] = data;

	int chan = (offset >> 6) & 0x1f;
	int slot = offset & 0x3f;

	std::string preg = "-";
#if 0
	if(slot >= 0x21 && slot <= 0x2b && (slot & 1))
		preg = util::string_format("fp%03x", (slot-0x21)/2 + 6*chan);
	else if(slot == 0x0e || slot == 0x0f)
		preg = util::string_format("sy%02x", (slot-0x0e) + 2*chan);
	else if(slot == 0x30 || slot == 0x31)
		preg = util::string_format("dt%02x", (slot-0x30) + 2*chan);
	else if(slot == 0x38)
		preg = util::string_format("vl%02x", chan);
	else if(slot == 0x3e || slot == 0x3f)
		preg = util::string_format("lf%02x", (slot-0x3e) + 2*chan);
	else
#endif
		preg = util::string_format("%02x.%02x", chan, slot);

	logerror("snd_w [%04x %04x] %-5s, %02x\n", offset, offset*2, preg, data);
}



// Synthesis

void swp00_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
}
