// license:BSD-3-Clause
// copyright-holders:Hiromitsu Shioya, Olivier Galibert
/*********************************************************/
/*    SEGA 16ch 8bit PCM                                 */
/*********************************************************/

#include "emu.h"
#include "segapcm.h"

#include <algorithm>

// device type definition
DEFINE_DEVICE_TYPE(SEGAPCM_DISCRETE, segapcm_discrete_device, "segapcm_discrete", "Sega PCM (Discrete logic)")
DEFINE_DEVICE_TYPE(SEGA_315_5218,    sega_315_5218_device,    "sega_315_5218",    "Sega 315-5218 PCM")

// reg      function
// ------------------------------------------------
// 0x00     ?
// 0x01     ?
// 0x02     volume left
// 0x03     volume right
// 0x04     loop address (08-15)
// 0x05     loop address (16-23)
// 0x06     end address
// 0x07     address delta
// 0x80     ?
// 0x81     ?
// 0x82     ?
// 0x83     ?
// 0x84     current address (08-15), 00-07 is internal?
// 0x85     current address (16-23)
// 0x86     bit 0: channel disable?
//          bit 1: loop disable
//          bit 7: unknown (discrete logic)
//          other bits: bank (315-5218)
// 0x87     ?

// only lower half of register set is valid
void segapcm_discrete_device::map(address_map &map)
{
	map(0x00, 0xff).ram().share(m_ram); // for debug, or scratchpad RAM?
	map(0x42, 0x42).select(0x38).rw(FUNC(segapcm_discrete_device::voice_lvol_r), FUNC(segapcm_discrete_device::voice_lvol_w));
	map(0x43, 0x43).select(0x38).rw(FUNC(segapcm_discrete_device::voice_rvol_r), FUNC(segapcm_discrete_device::voice_rvol_w));
	map(0x44, 0x45).select(0x38).rw(FUNC(segapcm_discrete_device::voice_loop_r), FUNC(segapcm_discrete_device::voice_loop_w));
	map(0x46, 0x46).select(0x38).rw(FUNC(segapcm_discrete_device::voice_end_r), FUNC(segapcm_discrete_device::voice_end_w));
	map(0x47, 0x47).select(0x38).rw(FUNC(segapcm_discrete_device::voice_freq_r), FUNC(segapcm_discrete_device::voice_freq_w));
	map(0xc4, 0xc5).select(0x38).rw(FUNC(segapcm_discrete_device::voice_addr_r), FUNC(segapcm_discrete_device::voice_addr_w));
	map(0xc6, 0xc6).select(0x38).rw(FUNC(segapcm_discrete_device::voice_ctrl_r), FUNC(segapcm_discrete_device::voice_ctrl_w));
}

void sega_315_5218_device::map(address_map &map)
{
	map(0x00, 0xff).ram().share(m_ram); // for debug, or scratchpad RAM?
	map(0x02, 0x02).select(0x78).rw(FUNC(sega_315_5218_device::voice_lvol_r), FUNC(sega_315_5218_device::voice_lvol_w));
	map(0x03, 0x03).select(0x78).rw(FUNC(sega_315_5218_device::voice_rvol_r), FUNC(sega_315_5218_device::voice_rvol_w));
	map(0x04, 0x05).select(0x78).rw(FUNC(sega_315_5218_device::voice_loop_r), FUNC(sega_315_5218_device::voice_loop_w));
	map(0x06, 0x06).select(0x78).rw(FUNC(sega_315_5218_device::voice_end_r), FUNC(sega_315_5218_device::voice_end_w));
	map(0x07, 0x07).select(0x78).rw(FUNC(sega_315_5218_device::voice_freq_r), FUNC(sega_315_5218_device::voice_freq_w));
	map(0x84, 0x85).select(0x78).rw(FUNC(sega_315_5218_device::voice_addr_r), FUNC(sega_315_5218_device::voice_addr_w));
	map(0x86, 0x86).select(0x78).rw(FUNC(sega_315_5218_device::voice_ctrl_r), FUNC(sega_315_5218_device::voice_ctrl_w));
}

//-------------------------------------------------
//  segapcm_base_device - constructor
//-------------------------------------------------

segapcm_base_device::segapcm_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface<21>(mconfig, *this)
	, m_ram(*this, "ram")
	, m_stream(nullptr)
{
}

//-------------------------------------------------
//  segapcm_device - constructor
//-------------------------------------------------

template <unsigned MaxVoices>
segapcm_device<MaxVoices>::segapcm_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: segapcm_base_device(mconfig, type, tag, owner, clock)
{
}

//-------------------------------------------------
//  segapcm_discrete_device - constructor
//-------------------------------------------------

segapcm_discrete_device::segapcm_discrete_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: segapcm_device<8>(mconfig, SEGAPCM_DISCRETE, tag, owner, clock)
{
}

//-------------------------------------------------
//  sega_315_5218_device - constructor
//-------------------------------------------------

sega_315_5218_device::sega_315_5218_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: segapcm_device<16>(mconfig, SEGA_315_5218, tag, owner, clock)
	, m_bankshift(12)
	, m_bankmask(0x70)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void segapcm_base_device::device_start()
{
	std::fill_n(&m_ram[0], m_ram.length(), 0xff);
}

template <unsigned MaxVoices>
void segapcm_device<MaxVoices>::device_start()
{
	segapcm_base_device::device_start();

	m_stream = stream_alloc(0, 2, clock() / CLOCK_DIVIDER);

	for (auto &elem : m_voice)
		elem.host = this;

	save_item(STRUCT_MEMBER(m_voice, addr));
	save_item(STRUCT_MEMBER(m_voice, loop));
	save_item(STRUCT_MEMBER(m_voice, end));
	save_item(STRUCT_MEMBER(m_voice, freq));
	save_item(STRUCT_MEMBER(m_voice, lvol));
	save_item(STRUCT_MEMBER(m_voice, rvol));
	save_item(STRUCT_MEMBER(m_voice, ctrl));
	save_item(STRUCT_MEMBER(m_voice, lout));
	save_item(STRUCT_MEMBER(m_voice, rout));
}


//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

template <unsigned MaxVoices>
void segapcm_device<MaxVoices>::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / CLOCK_DIVIDER);
}


//-------------------------------------------------
//  rom_bank_pre_change - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void segapcm_base_device::rom_bank_pre_change()
{
	m_stream->update();
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

template <unsigned MaxVoices>
void segapcm_device<MaxVoices>::sound_stream_update(sound_stream &stream)
{
	/* loop over streams */
	for (int i = 0; i < stream.samples(); i++)
	{
		int32_t lout = 0, rout = 0;
		for (auto &elem : m_voice)
		{
			elem.tick();
			lout += elem.lout;
			rout += elem.rout;
		}
		stream.put_int(0, i, lout, 32768);
		stream.put_int(1, i, rout, 32768);
	}
}

void segapcm_base_device::voice_t::tick()
{
	// only process active channels
	lout = rout = 0;
	if (BIT(~ctrl, 0))
	{
		// handle looping if we've hit the end
		if ((addr >> 16) == end)
		{
			if (BIT(ctrl, 1))
			{
				ctrl |= 1;
				return;
			}
			else
				addr = (uint32_t(loop) << 8);
		}
		// fetch the sample
		const uint32_t bank = host->get_bank(ctrl);
		const int8_t v = host->read_byte(bank + (addr >> 8)) - 0x80;
		// apply panning and advance
		lout = v * (lvol & 0x7f);
		rout = v * (rvol & 0x7f);
		addr = (addr + freq) & 0xffffff;
	}
	else
		addr &= 0xffff00;
}

// read/write handlers
template <unsigned MaxVoices>
uint8_t segapcm_device<MaxVoices>::voice_addr_r(offs_t offset)
{
	m_stream->update();
	voice_t &voice = m_voice[offset >> 3];
	const uint8_t shift = 8 + ((offset & 1) << 3);
	return (voice.addr >> shift) & 0xff;
}

template <unsigned MaxVoices>
uint8_t segapcm_device<MaxVoices>::voice_loop_r(offs_t offset)
{
	m_stream->update();
	voice_t &voice = m_voice[offset >> 3];
	const uint8_t shift = (offset & 1) << 3;
	return (voice.loop >> shift) & 0xff;
}

template <unsigned MaxVoices>
uint8_t segapcm_device<MaxVoices>::voice_end_r(offs_t offset)
{
	m_stream->update();
	voice_t &voice = m_voice[offset >> 3];
	return voice.end;
}

template <unsigned MaxVoices>
uint8_t segapcm_device<MaxVoices>::voice_freq_r(offs_t offset)
{
	m_stream->update();
	voice_t &voice = m_voice[offset >> 3];
	return voice.freq;
}

template <unsigned MaxVoices>
uint8_t segapcm_device<MaxVoices>::voice_lvol_r(offs_t offset)
{
	m_stream->update();
	voice_t &voice = m_voice[offset >> 3];
	return voice.lvol;
}

template <unsigned MaxVoices>
uint8_t segapcm_device<MaxVoices>::voice_rvol_r(offs_t offset)
{
	m_stream->update();
	voice_t &voice = m_voice[offset >> 3];
	return voice.rvol;
}

template <unsigned MaxVoices>
uint8_t segapcm_device<MaxVoices>::voice_ctrl_r(offs_t offset)
{
	m_stream->update();
	voice_t &voice = m_voice[offset >> 3];
	return voice.ctrl;
}

template <unsigned MaxVoices>
void segapcm_device<MaxVoices>::voice_addr_w(offs_t offset, uint8_t data)
{
	m_stream->update();
	voice_t &voice = m_voice[offset >> 3];
	const uint8_t shift = 8 + ((offset & 1) << 3);
	voice.addr = (voice.addr & ~(0xff << shift)) | (uint32_t(data) << shift);
}

template <unsigned MaxVoices>
void segapcm_device<MaxVoices>::voice_loop_w(offs_t offset, uint8_t data)
{
	m_stream->update();
	voice_t &voice = m_voice[offset >> 3];
	const uint8_t shift = (offset & 1) << 3;
	voice.loop = (voice.loop & ~(0xff << shift)) | (uint32_t(data) << shift);
}

template <unsigned MaxVoices>
void segapcm_device<MaxVoices>::voice_end_w(offs_t offset, uint8_t data)
{
	m_stream->update();
	voice_t &voice = m_voice[offset >> 3];
	voice.end = data;
}

template <unsigned MaxVoices>
void segapcm_device<MaxVoices>::voice_freq_w(offs_t offset, uint8_t data)
{
	m_stream->update();
	voice_t &voice = m_voice[offset >> 3];
	voice.freq = data;
}

template <unsigned MaxVoices>
void segapcm_device<MaxVoices>::voice_lvol_w(offs_t offset, uint8_t data)
{
	m_stream->update();
	voice_t &voice = m_voice[offset >> 3];
	voice.lvol = data;
}

template <unsigned MaxVoices>
void segapcm_device<MaxVoices>::voice_rvol_w(offs_t offset, uint8_t data)
{
	m_stream->update();
	voice_t &voice = m_voice[offset >> 3];
	voice.rvol = data;
}

template <unsigned MaxVoices>
void segapcm_device<MaxVoices>::voice_ctrl_w(offs_t offset, uint8_t data)
{
	m_stream->update();
	voice_t &voice = m_voice[offset >> 3];
	voice.ctrl = data;
}

// template class instantiation
template class segapcm_device<8>;
template class segapcm_device<16>;
