// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWX00, rompler/dsp/cpu combo, audio support

#include "emu.h"
#include "swx00.h"


DEFINE_DEVICE_TYPE(SWX00_SOUND, swx00_sound_device, "swx00_sound", "Yamaha SWX00 (sound subsystem)")

// Some tables we need.  Lifted from the swp00, probably incorrect.

const std::array<s32, 0x80> swx00_sound_device::attack_linear_step = {
	0x00027, 0x0002b, 0x0002f, 0x00033, 0x00037, 0x0003d, 0x00042, 0x00048,
	0x0004d, 0x00056, 0x0005e, 0x00066, 0x0006f, 0x0007a, 0x00085, 0x00090,
	0x0009b, 0x000ac, 0x000bd, 0x000cc, 0x000de, 0x000f4, 0x00109, 0x00120,
	0x00135, 0x00158, 0x00179, 0x00199, 0x001bc, 0x001e7, 0x00214, 0x00240,
	0x0026b, 0x002af, 0x002f2, 0x00332, 0x00377, 0x003d0, 0x0042c, 0x00480,
	0x004dc, 0x0055e, 0x005e9, 0x0066e, 0x006f4, 0x007a4, 0x00857, 0x0090b,
	0x009c3, 0x00acb, 0x00bd6, 0x00ce6, 0x00e00, 0x00f5e, 0x010d2, 0x01234,
	0x0139e, 0x015d0, 0x017f3, 0x01a20, 0x01c4a, 0x01f52, 0x02232, 0x0250f,
	0x027ff, 0x02c72, 0x03109, 0x0338b, 0x039c4, 0x04038, 0x04648, 0x04c84,
	0x05262, 0x05c1c, 0x065af, 0x06f5c, 0x07895, 0x0866f, 0x09470, 0x0a19e,
	0x0ae4c, 0x0c566, 0x0db8d, 0x0f00f, 0x10625, 0x12937, 0x14954, 0x16c17,
	0x1886e, 0x1c71c, 0x20000, 0x239e1, 0x2647c, 0x2aaab, 0x2ecfc, 0x3241f,
	0x35e51, 0x3a83b, 0x40000, 0x4325c, 0x47dc1, 0x4c8f9, 0x50505, 0x55555,
	0x58160, 0x5d174, 0x60606, 0x62b2e, 0x67b24, 0x6a63c, 0x6d3a0, 0x6eb3e,
	0x71c72, 0x73616, 0x75075, 0x76b98, 0x78788, 0x78788, 0x7a44c, 0x7a44c,
	0x7a44c, 0x7a44c, 0x7a44c, 0x7a44c, 0x7a44c, 0x7a44c, 0x7a44c, 0x7a44c,
};

const std::array<s32, 0x20> swx00_sound_device::decay_linear_step = {
	0x15083, 0x17ad2, 0x1a41a, 0x1cbe7, 0x1f16d, 0x22ef1, 0x26a44, 0x2a1e4,
	0x2da35, 0x34034, 0x3a197, 0x40000, 0x45b82, 0x4b809, 0x51833, 0x57262,
	0x5d9f7, 0x6483f, 0x6b15c, 0x71c72, 0x77976, 0x7d119, 0x83127, 0x88889,
	0x8d3dd, 0x939a8, 0x991f2, 0x9d89e, 0xa0a0a, 0xa57eb, 0xa72f0, 0xac769,
};

const std::array<s32, 16> swx00_sound_device::panmap = {
	0x000, 0x040, 0x080, 0x0c0,
	0x100, 0x140, 0x180, 0x1c0,
	0x200, 0x240, 0x280, 0x2c0,
	0x300, 0x340, 0x380, 0xfff
};

const std::array<u8, 4> swx00_sound_device::dpcm_offset = { 7, 6, 4, 0 };

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
	m_stream = stream_alloc(0, 2, 44100);

	save_item(NAME(m_sample_start));
	save_item(NAME(m_sample_end));
	save_item(NAME(m_sample_address));

	save_item(NAME(m_glo_pan));
	save_item(NAME(m_rev_dry));
	save_item(NAME(m_cho_var));

	save_item(NAME(m_attack));
	save_item(NAME(m_decay));

	save_item(NAME(m_keyon));
	save_item(NAME(m_state_sel));

	save_item(NAME(m_dsp_offsets));
	save_item(NAME(m_dsp_values));

	save_item(NAME(m_rom_address));
	save_item(NAME(m_rom_read_status));

	save_item(NAME(m_sample_pos));
	save_item(NAME(m_envelope_level));

	save_item(NAME(m_glo_level_cur));
	save_item(NAME(m_pan_l));
	save_item(NAME(m_pan_r));

	save_item(NAME(m_active));
	save_item(NAME(m_decay_on));
	save_item(NAME(m_decay_done));

	save_item(NAME(m_dpcm_current));
	save_item(NAME(m_dpcm_next));
	save_item(NAME(m_dpcm_address));
	save_item(NAME(m_dpcm_sum));


	for(int i=0; i != 128; i++) {
		u32 v = 0;
		switch(i >> 3) {
		default:  v = ((i & 7) + 8) << (1 + (i >> 3)); break;
		case 0xb: v = ((i & 7) + 4) << 13; break;
		case 0xc: v = ((i & 6) + 6) << 14; break;
		case 0xd: v = ((i & 4) + 7) << 15; break;
		case 0xe: v = 15 << 15; break;
		case 0xf: v = 31 << 15; break;
		}
		m_global_step[i] = v;
	}

	// Delta-packed samples decompression.

	for(int i=0; i<128; i++) {
		s16 base = ((i & 0x1f) << (3+(i >> 5))) + (((1 << (i >> 5))-1) << 8);
		m_dpcm[i | 0x80] = - base;
		m_dpcm[i]        = + base;
	}
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
	std::fill(m_decay.begin(), m_decay.end(), 0);

	std::fill(m_dsp_offsets.begin(), m_dsp_offsets.end(), 0);
	std::fill(m_dsp_values.begin(), m_dsp_values.end(), 0);

	std::fill(m_sample_pos.begin(), m_sample_pos.end(), 0);
	std::fill(m_envelope_level.begin(), m_envelope_level.end(), 0);

	std::fill(m_glo_level_cur.begin(), m_glo_level_cur.end(), 0);
	std::fill(m_pan_l.begin(), m_pan_l.end(), 0);
	std::fill(m_pan_r.begin(), m_pan_r.end(), 0);

	std::fill(m_active.begin(), m_active.end(), false);
	std::fill(m_decay_on.begin(), m_decay_on.end(), false);
	std::fill(m_decay_done.begin(), m_decay_done.end(), false);

	std::fill(m_dpcm_current.begin(), m_dpcm_current.end(), false);
	std::fill(m_dpcm_next.begin(), m_dpcm_next.end(), false);
	std::fill(m_dpcm_address.begin(), m_dpcm_address.end(), false);
	std::fill(m_dpcm_sum.begin(), m_dpcm_sum.end(), 0);

	m_keyon = 0;
	m_state_sel = 0;
	m_rom_address = 0;
	m_rom_read_status = 0;
}

void swx00_sound_device::map(address_map &map)
{
	map(0x000, 0x7ff).rw(FUNC(swx00_sound_device::snd_r), FUNC(swx00_sound_device::snd_w));

	map(0x001, 0x001).rw(FUNC(swx00_sound_device::state_r), FUNC(swx00_sound_device::state_sel_w));

	map(0x008, 0x00b).w(FUNC(swx00_sound_device::keyon_w));
	map(0x00c, 0x00c).w(FUNC(swx00_sound_device::keyon_commit_w));

	map(0x110, 0x110).w(FUNC(swx00_sound_device::dsp_offh_w));
	map(0x120, 0x120).w(FUNC(swx00_sound_device::dsp_valh_w));

	rchan(map, 0x05).rw(FUNC(swx00_sound_device::sample_start_r), FUNC(swx00_sound_device::sample_start_w));

	map(0x180, 0x1ff).w(FUNC(swx00_sound_device::dsp_offl_w));
	map(0x200, 0x31f).w(FUNC(swx00_sound_device::dsp_vall_w)); // 06-0b

	rchan(map, 0x13).rw(FUNC(swx00_sound_device::glo_pan_r), FUNC(swx00_sound_device::glo_pan_w));
	rchan(map, 0x14).rw(FUNC(swx00_sound_device::attack_r), FUNC(swx00_sound_device::attack_w));
	rchan(map, 0x15).rw(FUNC(swx00_sound_device::decay_r), FUNC(swx00_sound_device::decay_w));
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
	m_stream->update();
	for(int chan = 0; chan != 32; chan++)
		if(BIT(m_keyon, chan)) {
			logerror("keyon %02x\n", chan);
			m_sample_pos[chan] = -m_sample_start[chan] << 15;

			m_sample_pos[chan] = 0;

			m_active[chan] = true;
			m_decay_on[chan] = false;
			m_decay_done[chan] = false;

			m_dpcm_current[chan] = 0;
			m_dpcm_next[chan] = 0;
			m_dpcm_address[chan] = ((m_sample_address[chan] & 0xffffff) << 1) - m_sample_start[chan];
			m_dpcm_sum[chan] = 0;

			m_glo_level_cur[chan] = (m_glo_pan[chan] >> 4) & 0xff0;
			m_pan_l[chan] = panmap[(m_glo_pan[chan] >> 4) & 15];
			m_pan_r[chan] = panmap[m_glo_pan[chan] & 15];

			if(m_decay[chan] & 0x8000) {
				m_envelope_level[chan] = 0;
				m_decay_on[chan] = true;
			} else if((m_attack[chan] & 0x8000) || (m_attack[chan] & 0xff))
				m_envelope_level[chan] = (m_attack[chan] & 0xff) << 20;
			else
				m_envelope_level[chan] = 0x8000000;
		}

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

u16 swx00_sound_device::decay_r(offs_t chan)
{
	return m_decay[chan];
}

void swx00_sound_device::decay_w(offs_t chan, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_decay[chan]);
	if(ACCESSING_BITS_0_7)
		logerror("%02x: decay %02x.%02x\n", chan, m_decay[chan] >> 8, m_decay[chan] & 0xff);
}

void swx00_sound_device::state_sel_w(u8 data)
{
	m_state_sel = data;
}

u8 swx00_sound_device::state_r()
{
	int chan = m_state_sel & 0x1f;
	switch(m_state_sel & 0xe0) {
	case 0x40: { // Envelope state
		if(!m_active[chan])
			return 0xff;

		u8 vol;
		if(m_decay_on[chan] || (m_attack[chan] & 0x7f) || (m_attack[chan] & 0x8000))
			vol = m_envelope_level[chan] >> 22;
		else
			vol = 0;

		if(m_decay_done[chan])
			vol |= 0x40;
		if(m_decay_on[chan])
			vol |= 0x80;

		return vol;
	}

	default:
		if(0)
			logerror("state_r %02x\n", m_state_sel);
		break;
	}
	return 0;
}

void swx00_sound_device::dsp_valh_w(u8 data)
{
	m_dsp_valh = data;
}

void swx00_sound_device::dsp_vall_w(offs_t reg, u8 data)
{
	m_dsp_values[reg] = ((m_dsp_valh << 8) | data) & 0x3ff;
	//  logerror("dsp value[%03x] = %03x\n", reg, m_dsp_values[reg]);
}

void swx00_sound_device::dsp_offh_w(u8 data)
{
	m_dsp_offh = data;
}

void swx00_sound_device::dsp_offl_w(offs_t reg, u8 data)
{
	if(reg & 1) {
		m_dsp_offsets[reg >> 1] = (m_dsp_offh << 16) | (m_dsp_offsets[reg >> 1] & 0xff00) | data;
		//      logerror("dsp offset[%02x] = %06x\n", reg >> 1, m_dsp_offsets[reg >> 1]);
	} else
		m_dsp_offsets[reg >> 1] = (m_dsp_offh << 16) | (data << 8) | (m_dsp_offsets[reg >> 1] & 0xff);
}

bool swx00_sound_device::istep(s32 &value, s32 limit, s32 step)
{
	//  fprintf(stderr, "istep(%x, %x, %x)\n", value, limit, step);
	if(value < limit) {
		value += step;
		if(value >= limit) {
			value = limit;
			return true;
		}
		return false;
	}

	if(value > limit) {
		value -= step;
		if(value <= limit) {
			value = limit;
			return true;
		}
		return false;
	}

	return true;
}

s32 swx00_sound_device::fpadd(s32 value, s32 step)
{
	s32 e = value >> 24;
	s32 m = value & 0xffffff;

	m += step << e;
	if(m & 0xfe000000)
		return 0xfffffff;

	while(m & 0x01000000) {
		m <<= 1;
		e ++;
	}
	if(e >= 16)
		return 0xfffffff;
	return (e << 24) | (m & 0xffffff);
}

s32 swx00_sound_device::fpsub(s32 value, s32 step)
{
	s32 e = value >> 24;
	s32 m = (value & 0xffffff) | 0xfe000000;
	m = e < 0xc ? m - (step << e) : (m >> (e - 0xb)) - (step << 0xb);
	if(m >= 0)
		return 0;
	if(e >= 0xc)
		e = 0xb;
	while(m < 0xfe000000) {
		if(!e)
			return 0;
		e --;
		m >>= 1;
	}
	while(e != 0xf && (m >= 0xff000000)) {
		e ++;
		m <<= 1;
	}

	return (e << 24) | (m & 0xffffff);
}

bool swx00_sound_device::fpstep(s32 &value, s32 limit, s32 step)
{
	// value, limit and step are 4.24 but step has its exponent and
	// top four bits zero

	if(value == limit)
		return true;
	if(value < limit) {
		value = fpadd(value, step);
		if(value >= limit) {
			value = limit;
			return true;
		}
		return false;
	}

	value = fpsub(value, step);
	if(value <= limit) {
		value = limit;
		return true;
	}
	return false;
}

// sample is signed 24.8
s32 swx00_sound_device::fpapply(s32 value, s32 sample)
{
	if(value >= 0x10000000)
		return 0;
	return (s64(sample) - ((s64(sample) * ((value >> 9) & 0x7fff)) >> 16)) >> (value >> 24);
}

void swx00_sound_device::sound_stream_update(sound_stream &stream)
{
	for(int i=0; i != stream.samples(); i++) {
		s32 dry_l = 0, dry_r = 0;
		s32 rev   = 0;
		s32 cho_l = 0, cho_r = 0;
		s32 var_l = 0, var_r = 0;

		for(int chan = 0; chan != 32; chan++) {
			if(!m_active[chan])
				continue;

			s16 val0, val1;
			u32 base_address = m_sample_address[chan] & 0xffffff;
			s32 spos = m_sample_pos[chan] >> 15;
			switch(m_sample_address[chan] >> 30) {
			case 0: { // 16-bits linear
				offs_t adr = base_address + spos;
				val0 = read_word(adr);
				val1 = read_word(adr+1);
				break;
			}

			case 1: { // 12-bits linear
				offs_t adr = base_address + (spos >> 2)*3;
				switch(spos & 3) {
				case 0: { // Cabc ..AB .... ....
					u16 w0 = read_word(adr);
					u16 w1 = read_word(adr+1);
					val0 = (w0 & 0x0fff) << 4;
					val1 = ((w0 & 0xf000) >> 8) | ((w1 & 0x00ff) << 8);
					break;
				}
				case 1: { // c... BCab ...A ....
					u16 w0 = read_word(adr);
					u16 w1 = read_word(adr+1);
					u16 w2 = read_word(adr+2);
					val0 = ((w0 & 0xf000) >> 8) | ((w1 & 0x00ff) << 8);
					val1 = ((w1 & 0xff00) >> 4) | ((w2 & 0x000f) << 12);
					break;
				}
				case 2: { // .... bc.. ABCa ....
					u16 w1 = read_word(adr+1);
					u16 w2 = read_word(adr+2);
					val0 = ((w1 & 0xff00) >> 4) | ((w2 & 0x000f) << 12);
					val1 = w2 & 0xfff0;
					break;
				}
				case 3: { // .... .... abc. .ABC
					u16 w2 = read_word(adr+2);
					u16 w3 = read_word(adr+3);
					val0 = w2 & 0xfff0;
					val1 = (w3 & 0x0fff) << 4;
					break;
				}
				}
				break;
			}

			case 2: { // 8-bits linear
				offs_t adr = base_address + (spos >> 1);
				if(spos & 1) {
					u16 w0 = read_word(adr);
					u16 w1 = read_word(adr+1);
					val0 = w0 & 0xff00;
					val1 = w1 << 8;
				} else {
					u16 w0 = read_word(adr);
					val0 = w0 << 8;
					val1 = w0 & 0xff00;
				}
				break;
			}

			case 3: { // 8-bits delta-pcm
				u8 offset = dpcm_offset[(m_sample_address[chan] >> 24) & 3];
				u8 scale = (m_sample_address[chan] >> 26) & 7;
				u32 target_address = (base_address << 1) + spos + 1;
				while(m_dpcm_address[chan] <= target_address) {
					m_dpcm_current[chan] = m_dpcm_next[chan];
					u16 idx = read_word(m_dpcm_address[chan] >> 1);
					if(m_dpcm_address[chan] & 1)
						idx &= 0xff;
					else
						idx >>= 8;
					m_dpcm_sum[chan] += m_dpcm[idx] - offset;
					s32 sample = (m_dpcm_sum[chan] << scale) >> 3;
					m_dpcm_address[chan] ++;
					if(sample < -0x8000)
						sample = -0x8000;
					else if(sample > 0x7fff)
						sample = 0x7fff;
					m_dpcm_next[chan] = sample;
				}
				val0 = m_dpcm_current[chan];
				val1 = m_dpcm_next[chan];
				break;
			}
			}

			s32 mul = m_sample_pos[chan] & 0x7fff;
			s32 sample = (val1 * mul + val0 * (0x8000 - mul)) >> 7;

			s32 envelope_level;
			if(m_decay_on[chan] || (m_attack[chan] & 0xff) || (m_attack[chan] & 0x8000))
				envelope_level = m_envelope_level[chan];
			else
				envelope_level = 0;

			s32 tremolo_level = 0;

			dry_l += fpapply(envelope_level + (m_glo_level_cur[chan] << 16) + tremolo_level + ((m_rev_dry[chan] << 20) & 0xff00000) + (m_pan_l[chan] << 16), sample);
			dry_r += fpapply(envelope_level + (m_glo_level_cur[chan] << 16) + tremolo_level + ((m_rev_dry[chan] << 20) & 0xff00000) + (m_pan_r[chan] << 16), sample);
			rev   += fpapply(envelope_level + (m_glo_level_cur[chan] << 16) + tremolo_level + ((m_rev_dry[chan] << 12) & 0xff00000),                         sample);
			cho_l += fpapply(envelope_level + (m_glo_level_cur[chan] << 16) + tremolo_level + ((m_cho_var[chan] << 12) & 0xff00000) + (m_pan_l[chan] << 16), sample);
			cho_r += fpapply(envelope_level + (m_glo_level_cur[chan] << 16) + tremolo_level + ((m_cho_var[chan] << 12) & 0xff00000) + (m_pan_r[chan] << 16), sample);
			var_l += fpapply(envelope_level + (m_glo_level_cur[chan] << 16) + tremolo_level + ((m_cho_var[chan] << 20) & 0xff00000) + (m_pan_l[chan] << 16), sample);
			var_r += fpapply(envelope_level + (m_glo_level_cur[chan] << 16) + tremolo_level + ((m_cho_var[chan] << 20) & 0xff00000) + (m_pan_r[chan] << 16), sample);

			u32 sample_increment = ((m_sample_pitch[chan] & 0xfff) << (8 + (s16(m_sample_pitch[chan]) >> 12))) >> 4;
			m_sample_pos[chan] += sample_increment;
			if((m_sample_pos[chan] >> 15) >= m_sample_end[chan]) {
				if(!m_sample_end[chan])
					m_active[chan] = false;
				else {
					s32 prev = m_sample_pos[chan];
					do
						m_sample_pos[chan] -= m_sample_end[chan] << 15;
					while((m_sample_pos[chan] >> 15) >= m_sample_end[chan]);
					m_dpcm_address[chan] += (m_sample_pos[chan] >> 15) - (prev >> 15);
					m_dpcm_sum[chan] = 0;
				}
			}

			istep(m_glo_level_cur[chan], (m_glo_pan[chan] >> 4) & 0xff0, 1);
			istep(m_pan_l[chan], panmap[(m_glo_pan[chan] >> 4) & 15], 1);
			istep(m_pan_r[chan], panmap[m_glo_pan[chan] & 15], 1);

			if(m_decay_on[chan]) {
				if((m_decay[chan] & 0x6000) == 0x6000)
					m_decay_done[chan] = fpstep(m_envelope_level[chan], (m_decay[chan] & 0xff) << 20, decay_linear_step[(m_decay[chan] >> 8) & 0x1f]);
				else
					m_decay_done[chan] = istep(m_envelope_level[chan], (m_decay[chan] & 0xff) << 20, m_global_step[m_decay[chan] >> 8] << 1);
				if(m_envelope_level[chan] & 0x8000000)
					m_active[chan] = false;

			} else if(m_attack[chan] & 0x8000)
				m_decay_on[chan] = fpstep(m_envelope_level[chan], 0, attack_linear_step[(m_attack[chan] >> 8) & 0x7f]);
			else
				m_decay_on[chan] = istep(m_envelope_level[chan], 0, m_global_step[m_attack[chan] >> 8] << 1);
		}

		dry_l >>= 8;
		dry_r >>= 8;
		rev   >>= 8;
		cho_l >>= 8;
		cho_r >>= 8;
		var_l >>= 8;
		var_r >>= 8;

		(void)rev;
		(void)cho_l;
		(void)cho_r;
		(void)var_l;
		(void)var_r;

		stream.put_int(0, i, dry_l, 32768);
		stream.put_int(1, i, dry_r, 32768);
	}
}
