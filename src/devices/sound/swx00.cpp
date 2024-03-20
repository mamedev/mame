// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWX00, rompler/dsp/cpu combo, audio support

#include "emu.h"
#include "swx00.h"


DEFINE_DEVICE_TYPE(SWX00_SOUND, swx00_sound_device, "swx00_sound", "Yamaha SWX00 (sound subsystem)")

swx00_sound_device::swx00_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SWX00_SOUND, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  device_rom_interface(mconfig, *this)
{
}

void swx00_sound_device::device_add_mconfig(machine_config &config)
{
}

void swx00_sound_device::device_start()
{
	save_item(NAME(m_sample_start));
	save_item(NAME(m_sample_end));
	save_item(NAME(m_sample_address));

	save_item(NAME(m_glo_pan));
	save_item(NAME(m_rev_dry));
	save_item(NAME(m_cho_var));

	save_item(NAME(m_attack));
	save_item(NAME(m_release));

	save_item(NAME(m_keyon));
	save_item(NAME(m_state_sel));

	save_item(NAME(m_dsp_offsets));
	save_item(NAME(m_dsp_values));

	save_item(NAME(m_rom_address));
	save_item(NAME(m_rom_read_status));
}

void swx00_sound_device::device_reset()
{
	std::fill(m_sample_start.begin(), m_sample_start.end(), 0);
	std::fill(m_sample_end.begin(), m_sample_end.end(), 0);
	std::fill(m_sample_address.begin(), m_sample_address.end(), 0);

	std::fill(m_glo_pan.begin(), m_glo_pan.end(), 0);
	std::fill(m_rev_dry.begin(), m_rev_dry.end(), 0);
	std::fill(m_cho_var.begin(), m_cho_var.end(), 0);

	std::fill(m_attack.begin(), m_attack.end(), 0);
	std::fill(m_release.begin(), m_release.end(), 0);

	std::fill(m_dsp_offsets.begin(), m_dsp_offsets.end(), 0);
	std::fill(m_dsp_values.begin(), m_dsp_values.end(), 0);

	m_keyon = 0;
	m_state_sel = 0;
	m_rom_address = 0;
	m_rom_read_status = 0;
}

void swx00_sound_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
}

void swx00_sound_device::map(address_map &map)
{
	map(0x000, 0x7ff).rw(FUNC(swx00_sound_device::snd_r), FUNC(swx00_sound_device::snd_w));

	map(0x000, 0x000).rw(FUNC(swx00_sound_device::state_r), FUNC(swx00_sound_device::state_sel_w));

	map(0x008, 0x00b).w(FUNC(swx00_sound_device::keyon_w));
	map(0x00c, 0x00c).w(FUNC(swx00_sound_device::keyon_commit_w));

	map(0x110, 0x110).w(FUNC(swx00_sound_device::dsp_offh_w));
	map(0x120, 0x120).w(FUNC(swx00_sound_device::dsp_valh_w));

	rchan(map, 0x05).rw(FUNC(swx00_sound_device::sample_start_r), FUNC(swx00_sound_device::sample_start_w));

	map(0x180, 0x1ff).w(FUNC(swx00_sound_device::dsp_offl_w));
	map(0x200, 0x31f).w(FUNC(swx00_sound_device::dsp_vall_w)); // 06-0b
	
	rchan(map, 0x13).rw(FUNC(swx00_sound_device::glo_pan_r), FUNC(swx00_sound_device::glo_pan_w));
	rchan(map, 0x14).rw(FUNC(swx00_sound_device::attack_r), FUNC(swx00_sound_device::attack_w));
	rchan(map, 0x15).rw(FUNC(swx00_sound_device::release_r), FUNC(swx00_sound_device::release_w));
	rchan(map, 0x16).rw(FUNC(swx00_sound_device::rev_dry_r), FUNC(swx00_sound_device::rev_dry_w));
	rchan(map, 0x17).rw(FUNC(swx00_sound_device::cho_var_r), FUNC(swx00_sound_device::cho_var_w));
	rchan(map, 0x18).rw(FUNC(swx00_sound_device::sample_address_h_r), FUNC(swx00_sound_device::sample_address_h_w));
	rchan(map, 0x19).rw(FUNC(swx00_sound_device::sample_address_l_r), FUNC(swx00_sound_device::sample_address_l_w));
	rchan(map, 0x1a).rw(FUNC(swx00_sound_device::sample_pitch_r), FUNC(swx00_sound_device::sample_pitch_w));
	rchan(map, 0x1b).rw(FUNC(swx00_sound_device::sample_end_r), FUNC(swx00_sound_device::sample_end_w));

	map(0x808, 0x809).w(FUNC(swx00_sound_device::rom_read_adrh_w));
	map(0x80a, 0x80b).w(FUNC(swx00_sound_device::rom_read_adrl_w));
	map(0x80c, 0x80d).r(FUNC(swx00_sound_device::rom_read_status_r));
	map(0x80e, 0x80f).rw(FUNC(swx00_sound_device::rom_read_r), FUNC(swx00_sound_device::rom_read_w));
}

u16 swx00_sound_device::snd_r(offs_t offset)
{
	u32 chan = offset & 0x1f;
	u32 slot = offset >> 5;
	logerror("snd_r %03x %02x.%02x\n", offset*2, chan, slot);
	return 0;
}

void swx00_sound_device::snd_w(offs_t offset, u16 data, u16 mem_mask)
{
	u32 chan = offset & 0x1f;
	u32 slot = offset >> 5;
	if(slot == 0x10 || slot == 0x11)
		return;
	logerror("snd_w %03x %02x.%02x %04x @ %04x\n", offset*2, chan, slot, data, mem_mask);
}

void swx00_sound_device::rom_read_adrh_w(offs_t, u16 data, u16 mem_mask)
{
	m_rom_address = (m_rom_address & ~(mem_mask << 16)) | ((data & mem_mask) << 16);	
}

void swx00_sound_device::rom_read_adrl_w(offs_t, u16 data, u16 mem_mask)
{
	m_rom_address = (m_rom_address & ~mem_mask) | (data & mem_mask);
	m_rom_read_status = 1;
}

u16 swx00_sound_device::rom_read_status_r()
{
	return m_rom_read_status;
}

u16 swx00_sound_device::rom_read_r()
{
	m_rom_read_status = 0;
	return read_word(m_rom_address);
}

void swx00_sound_device::rom_read_w(u16)
{
	m_rom_read_status = 0;
}

u16 swx00_sound_device::sample_start_r(offs_t chan)
{
	return m_sample_start[chan];
}

void swx00_sound_device::sample_start_w(offs_t chan, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_sample_start[chan]);
	if(ACCESSING_BITS_0_7)
		logerror("sample_start[%02x] = %04x\n", chan, m_sample_start[chan]);
}

u16 swx00_sound_device::sample_end_r(offs_t chan)
{
	return m_sample_end[chan];
}

void swx00_sound_device::sample_end_w(offs_t chan, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_sample_end[chan]);
	if(ACCESSING_BITS_0_7)
		logerror("sample_end[%02x] = %04x\n", chan, m_sample_end[chan]);
}

u16 swx00_sound_device::sample_pitch_r(offs_t chan)
{
	return m_sample_pitch[chan];
}

void swx00_sound_device::sample_pitch_w(offs_t chan, u16 data, u16 mem_mask)
{
	u16 old = m_sample_pitch[chan];
	COMBINE_DATA(&m_sample_pitch[chan]);
	if(ACCESSING_BITS_0_7 && m_sample_pitch[chan] != old)
		logerror("sample_pitch[%02x] = %04x\n", chan, m_sample_pitch[chan]);
}

u16 swx00_sound_device::sample_address_h_r(offs_t chan)
{
	return m_sample_address[chan] >> 16;
}

u16 swx00_sound_device::sample_address_l_r(offs_t chan)
{
	return m_sample_address[chan];
}

void swx00_sound_device::sample_address_h_w(offs_t chan, u16 data, u16 mem_mask)
{
	m_sample_address[chan] = (m_sample_address[chan] & ~(mem_mask << 16)) | ((data & mem_mask) << 16);
}

void swx00_sound_device::sample_address_l_w(offs_t chan, u16 data, u16 mem_mask)
{
	m_sample_address[chan] = (m_sample_address[chan] & ~mem_mask) | (data & mem_mask);
	if(ACCESSING_BITS_0_7)
		logerror("sample_address[%02x] = %08x\n", chan, m_sample_address[chan]);
}

u16 swx00_sound_device::glo_pan_r(offs_t chan)
{
	return m_glo_pan[chan];
}

void swx00_sound_device::glo_pan_w(offs_t chan, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_glo_pan[chan]);
	logerror("%02x: glo=%02x panl=%x panr=%x\n", chan, m_glo_pan[chan] >> 8, (m_glo_pan[chan] >> 4) & 0xf, m_glo_pan[chan] & 0xf);
}

u16 swx00_sound_device::rev_dry_r(offs_t chan)
{
	return m_rev_dry[chan];
}

void swx00_sound_device::rev_dry_w(offs_t chan, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_rev_dry[chan]);
	logerror("%02x: rev=%02x dry=%02x\n", chan, m_rev_dry[chan] >> 8, m_rev_dry[chan] & 0xff);
}

u16 swx00_sound_device::cho_var_r(offs_t chan)
{
	return m_cho_var[chan];
}

void swx00_sound_device::cho_var_w(offs_t chan, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_cho_var[chan]);
	logerror("%02x: cho=%02x var=%02x\n", chan, m_cho_var[chan] >> 8, m_cho_var[chan] & 0xff);
}

void swx00_sound_device::keyon_w(offs_t offset, u8 data)
{
	u32 shift = 24 - 8*offset;
	m_keyon = (m_keyon & ~(0xff << shift)) | (data << shift);
}

void swx00_sound_device::keyon_commit_w(u8)
{
	logerror("keyon commit %08x\n", m_keyon);
	m_keyon = 0;
}

u16 swx00_sound_device::attack_r(offs_t chan)
{
	return m_attack[chan];
}

void swx00_sound_device::attack_w(offs_t chan, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_attack[chan]);
	if(ACCESSING_BITS_0_7)
		logerror("%02x: attack %02x.%02x\n", chan, m_attack[chan] >> 8, m_attack[chan] & 0xff);
}

u16 swx00_sound_device::release_r(offs_t chan)
{
	return m_release[chan];
}

void swx00_sound_device::release_w(offs_t chan, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_release[chan]);
	if(ACCESSING_BITS_0_7)
		logerror("%02x: release %02x.%02x\n", chan, m_release[chan] >> 8, m_release[chan] & 0xff);
}

void swx00_sound_device::state_sel_w(u8 data)
{
	m_state_sel = data;
}

u8 swx00_sound_device::state_r()
{
	return 0;
}

void swx00_sound_device::dsp_valh_w(u8 data)
{
	m_dsp_valh = data;
}

void swx00_sound_device::dsp_vall_w(offs_t reg, u8 data)
{
	m_dsp_values[reg] = ((m_dsp_valh << 8) | data) & 0x3ff;
	//	logerror("dsp value[%03x] = %03x\n", reg, m_dsp_values[reg]);
}

void swx00_sound_device::dsp_offh_w(u8 data)
{
	m_dsp_offh = data;
}

void swx00_sound_device::dsp_offl_w(offs_t reg, u8 data)
{
	if(reg & 1) {
		m_dsp_offsets[reg >> 1] = (m_dsp_offh << 16) | (m_dsp_offsets[reg >> 1] & 0xff00) | data;
		//		logerror("dsp offset[%02x] = %06x\n", reg >> 1, m_dsp_offsets[reg >> 1]);
	} else
		m_dsp_offsets[reg >> 1] = (m_dsp_offh << 16) | (data << 8) | (m_dsp_offsets[reg >> 1] & 0xff);
}
