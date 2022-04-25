// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Samsung Semiconductor KS0164 Wavetable Synthesizer

#include "emu.h"
#include "ks0164.h"


DEFINE_DEVICE_TYPE(KS0164, ks0164_device, "ks0164", "Samsung KS0164 Wavetable Synthesizer")

ks0164_device::ks0164_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KS0164, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  device_memory_interface(mconfig, *this),
	  device_serial_interface(mconfig, *this),
	  m_midi_tx(*this),
	  m_mem_region(*this, DEVICE_SELF),
	  m_cpu(*this, "cpu"),
	  m_mem_config("mem", ENDIANNESS_BIG, 16, 23)
{
}

device_memory_interface::space_config_vector ks0164_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_mem_config)
	};
}

void ks0164_device::device_add_mconfig(machine_config &config)
{
	KS0164CPU(config, m_cpu, DERIVED_CLOCK(1, 6));
	m_cpu->set_addrmap(AS_PROGRAM, &ks0164_device::cpu_map);
}

void ks0164_device::device_start()
{
	if(!has_configured_map(0) && m_mem_region) {
		u32 size = m_mem_region->bytes();
		u32 rend = size-1;

		// Round up to the nearest power-of-two-minus-one
		u32 rmask = rend;
		rmask |= rmask >> 1;
		rmask |= rmask >> 2;
		rmask |= rmask >> 4;
		rmask |= rmask >> 8;
		rmask |= rmask >> 16;
		// Mirror over the high bits.  rmask is a
		// power-of-two-minus-one, so the xor works
		space().install_rom(0, rend, ((1 << 23) - 1) ^ rmask, m_mem_region->base());
	}

	m_stream = stream_alloc(0, 2, clock()/3/2/2/32);
	space().cache(m_mem_cache);
	m_timer = timer_alloc(0);

	m_midi_tx.resolve_safe();

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(clock(), 542);

	save_item(NAME(m_bank1_base));
	save_item(NAME(m_bank1_select));
	save_item(NAME(m_bank2_base));
	save_item(NAME(m_bank2_select));
	save_item(NAME(m_sregs));
	save_item(NAME(m_mpu_in));
	save_item(NAME(m_mpu_out));
	save_item(NAME(m_mpu_status));
	save_item(NAME(m_unk60));
	save_item(NAME(m_voice_select));
	save_item(NAME(m_irqen_76));
	save_item(NAME(m_irqen_77));
	save_item(NAME(m_timer_interrupt));
}

void ks0164_device::device_reset()
{
	m_bank1_select = 0;
	m_bank1_base = 0;
	m_bank2_select = 0;
	m_bank2_base = 0;
	memset(m_sregs, 0, sizeof(m_sregs));
	m_unk60 = 0;
	m_voice_select = 0;
	m_irqen_76 = 0;
	m_irqen_77 = 0;
	m_timer_interrupt = false;

	m_mpu_in = 0x00;
	m_mpu_out = 0x00;
	m_mpu_status = 0x00;
	m_midi_in = 0x00;
	m_midi_in_active = false;

	m_timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));
}

void ks0164_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	m_timer_interrupt = true;
	if(m_irqen_76 & 0x40)
		m_cpu->set_input_line(14, ASSERT_LINE);
}

void ks0164_device::tra_complete()
{
	logerror("transmit done\n");
}

void ks0164_device::rcv_complete()
{
	receive_register_extract();
	m_midi_in = get_received_char();
	m_midi_in_active = true;
	m_cpu->set_input_line(6, ASSERT_LINE);

	logerror("recieved %02x\n", m_midi_in);
}

void ks0164_device::tra_callback()
{
	m_midi_tx(transmit_register_get_data_bit());
}

u8 ks0164_device::midi_r()
{
	m_midi_in_active = false;
	m_cpu->set_input_line(6, CLEAR_LINE);
	return m_midi_in;
}

void ks0164_device::midi_w(u8 data)
{
	logerror("want to transmit %02x\n", data);
}

u8 ks0164_device::midi_status_r()
{
	// transmit done/tx empty on bit 1
	return m_midi_in_active ? 1 : 0;
}

void ks0164_device::midi_status_w(u8 data)
{
	logerror("midi status_w %02x\n", data);
}


void ks0164_device::mpuin_set(bool control, u8 data)
{
	//  logerror("mpu push %s %02x\n", control ? "ctrl" : "data", data);
	m_mpu_in = data;
	if(control)
		m_mpu_status |= MPUS_RX_CTRL;
	else
		m_mpu_status &= ~MPUS_RX_CTRL;
	m_mpu_status |= MPUS_RX_FULL;

	if(m_mpu_status & MPUS_RX_INT)
		m_cpu->set_input_line(11, ASSERT_LINE);
}

void ks0164_device::mpu401_data_w(u8 data)
{
	mpuin_set(false, data);
}

void ks0164_device::mpu401_ctrl_w(u8 data)
{
	mpuin_set(true, data);
}

u8 ks0164_device::mpu401_data_r()
{
	//  logerror("mpu pop %02x\n", m_mpu_out);
	return m_mpu_out;
}

u8 ks0164_device::mpu401_status_r()
{
	u8 res = 0x3f;
	if(!(m_mpu_status & MPUS_TX_FULL))
		res |= 0x80;
	if(m_mpu_status & MPUS_RX_FULL)
		res |= 0x40;

	return res;
}

u8 ks0164_device::mpu401_istatus_r()
{
	//  logerror("mpu istatus read %02x (%04x)\n", m_mpu_status, m_cpu->pc());
	return m_mpu_status;
}

void ks0164_device::mpu401_istatus_w(u8 data)
{
	m_mpu_status = (m_mpu_status & ~(MPUS_RX_INT|MPUS_TX_INT)) | (data & (MPUS_RX_INT|MPUS_TX_INT));
	m_cpu->set_input_line(11, (m_mpu_status & (MPUS_RX_INT|MPUS_RX_FULL)) == (MPUS_RX_INT|MPUS_RX_FULL) ? ASSERT_LINE : CLEAR_LINE);
	//  logerror("mpu status write %02x (%04x)\n", m_mpu_status, m_cpu->pc());
}

u8 ks0164_device::mpu401_r()
{
	m_mpu_status &= ~MPUS_RX_FULL;
	m_cpu->set_input_line(11, CLEAR_LINE);
	//  logerror("mpu_r %02x (%04x)\n", m_mpu_in, m_cpu->pc());
	return m_mpu_in;
}

void ks0164_device::mpu401_w(u8 data)
{
	m_mpu_out = data;
	m_mpu_status |= MPUS_TX_FULL;
	//  logerror("mpu_w %02x (%04x)\n", m_mpu_out, m_cpu->pc());
}

u16 ks0164_device::vec_r(offs_t offset, u16 mem_mask)
{
	return m_mem_cache.read_word(offset << 1, mem_mask);
}

u16 ks0164_device::rom_r(offs_t offset, u16 mem_mask)
{
	return m_mem_cache.read_word((offset << 1) + 0x80, mem_mask);
}

u16 ks0164_device::bank1_r(offs_t offset, u16 mem_mask)
{
	return m_mem_cache.read_word(((offset << 1) & 0x3fff) | m_bank1_base, mem_mask);
}

void ks0164_device::bank1_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_mem_cache.write_word(((offset << 1) & 0x3fff) | m_bank1_base, data, mem_mask);
}

u16 ks0164_device::bank2_r(offs_t offset, u16 mem_mask)
{
	return m_mem_cache.read_word(((offset << 1) & 0x3fff) | m_bank2_base, mem_mask);
}

void ks0164_device::bank2_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_mem_cache.write_word(((offset << 1) & 0x3fff) | m_bank2_base, data, mem_mask);
}

u16 ks0164_device::bank1_select_r()
{
	return m_bank1_select;
}

void ks0164_device::bank1_select_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bank1_select);
	m_bank1_base = m_bank1_select << 14;
}

u16 ks0164_device::bank2_select_r()
{
	return m_bank2_select;
}

void ks0164_device::bank2_select_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bank2_select);
	m_bank2_base = m_bank2_select << 14;
}

u16 ks0164_device::voice_r(offs_t offset)
{
	m_stream->update();
	logerror("voice read %02x.%02x -> %04x (%04x)\n", m_voice_select & 0x1f, offset, m_sregs[m_voice_select & 0x1f][offset], m_cpu->pc());
	return m_sregs[m_voice_select & 0x1f][offset];
}

void ks0164_device::voice_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_stream->update();
	u16 old = m_sregs[m_voice_select & 0x1f][offset];
	COMBINE_DATA(&m_sregs[m_voice_select & 0x1f][offset]);
	if(0)
		if(m_cpu->pc() < 0x5f94 || m_cpu->pc() > 0x5fc0)
			logerror("voice %02x.%02x = %04x @ %04x (%04x)\n", m_voice_select & 0x1f, offset, m_sregs[m_voice_select & 0x1f][offset], mem_mask, m_cpu->pc());
	if(offset == 0 && (data & 1) && !(old & 1))
		logerror("keyon %02x mode=%04x (%s %c %c %c) cur=%02x%04x.%04x loop=%02x%04x.%04x end=%02x%04x.%04x pitch=%02x.%03x 10=%02x/%02x:%02x/%02x 14=%03x/%03x:%03x/%03x 18=%04x/%04x c=%04x   %04x %04x %04x %04x %04x  %04x %04x %04x %04x %04x\n",
				 m_voice_select,

				 m_sregs[m_voice_select & 0x1f][0x00],

				 m_sregs[m_voice_select & 0x1f][0x00] & 0x8000 ? " 8" : "16",
				 m_sregs[m_voice_select & 0x1f][0x00] & 0x0400 ? 'c' : 'l',
				 m_sregs[m_voice_select & 0x1f][0x00] & 0x0008 ? '3' : '-',
				 m_sregs[m_voice_select & 0x1f][0x00] & 0x0004 ? '2' : '-',

				 m_sregs[m_voice_select & 0x1f][0x01], // cur
				 m_sregs[m_voice_select & 0x1f][0x02],
				 m_sregs[m_voice_select & 0x1f][0x03],

				 m_sregs[m_voice_select & 0x1f][0x09], // loop
				 m_sregs[m_voice_select & 0x1f][0x0a],
				 m_sregs[m_voice_select & 0x1f][0x0b],

				 m_sregs[m_voice_select & 0x1f][0x0d], // end
				 m_sregs[m_voice_select & 0x1f][0x0e],
				 m_sregs[m_voice_select & 0x1f][0x0f],

				 m_sregs[m_voice_select & 0x1f][0x08] & 0x1f, // pitch
				 m_sregs[m_voice_select & 0x1f][0x08] >> 5,

				 m_sregs[m_voice_select & 0x1f][0x10] >> 9,
				 m_sregs[m_voice_select & 0x1f][0x12] >> 9,
				 m_sregs[m_voice_select & 0x1f][0x11] >> 9,
				 m_sregs[m_voice_select & 0x1f][0x13] >> 9,

				 m_sregs[m_voice_select & 0x1f][0x14] >> 5,
				 m_sregs[m_voice_select & 0x1f][0x16] >> 5,
				 m_sregs[m_voice_select & 0x1f][0x15] >> 5,
				 m_sregs[m_voice_select & 0x1f][0x17] >> 5,

				 m_sregs[m_voice_select & 0x1f][0x18],
				 m_sregs[m_voice_select & 0x1f][0x1c],

				 m_sregs[m_voice_select & 0x1f][0x0c],

				 m_sregs[m_voice_select & 0x1f][0x04],
				 m_sregs[m_voice_select & 0x1f][0x05],
				 m_sregs[m_voice_select & 0x1f][0x06],
				 m_sregs[m_voice_select & 0x1f][0x07],
				 m_sregs[m_voice_select & 0x1f][0x19],
				 m_sregs[m_voice_select & 0x1f][0x1a],
				 m_sregs[m_voice_select & 0x1f][0x1b],
				 m_sregs[m_voice_select & 0x1f][0x1d],
				 m_sregs[m_voice_select & 0x1f][0x1e],
				 m_sregs[m_voice_select & 0x1f][0x1f]);
}

u8 ks0164_device::irqen_76_r()
{
	return m_irqen_76;
}

// alternates 1e/5e
void ks0164_device::irqen_76_w(u8 data)
{
	m_irqen_76 = data;
	if(m_irqen_76 & 0x40)
		m_cpu->set_input_line(14, m_timer_interrupt ? ASSERT_LINE : CLEAR_LINE);

	else {
		m_timer_interrupt = false;
		m_cpu->set_input_line(14, CLEAR_LINE);
	}

	//  logerror("irqen_76 = %02x (%04x)\n", m_irqen_76, m_cpu->pc());
}

u8 ks0164_device::irqen_77_r()
{
	return m_irqen_77;
}

void ks0164_device::irqen_77_w(u8 data)
{
	m_irqen_77 = data;
	logerror("irqen_77 = %02x (%04x)\n", m_irqen_77, m_cpu->pc());
}

u8 ks0164_device::unk60_r()
{
	return m_unk60;
}

void ks0164_device::unk60_w(u8 data)
{
	m_unk60 = data;
	logerror("unk60 = %02x (%04x)\n", m_unk60, m_cpu->pc());
}

u8 ks0164_device::voice_select_r()
{
	return m_voice_select;
}

void ks0164_device::voice_select_w(u8 data)
{
	m_voice_select = data;
	logerror("voice_select = %02x (%04x)\n", m_voice_select, m_cpu->pc());
}

void ks0164_device::cpu_map(address_map &map)
{
	map(0x0000, 0x001f).r(FUNC(ks0164_device::vec_r));

	map(0x0020, 0x005f).rw(FUNC(ks0164_device::voice_r), FUNC(ks0164_device::voice_w));
	map(0x0060, 0x0060).rw(FUNC(ks0164_device::unk60_r), FUNC(ks0164_device::unk60_w));
	map(0x0061, 0x0061).rw(FUNC(ks0164_device::voice_select_r), FUNC(ks0164_device::voice_select_w));
	map(0x0062, 0x0063).rw(FUNC(ks0164_device::bank1_select_r), FUNC(ks0164_device::bank1_select_w));
	map(0x0064, 0x0065).rw(FUNC(ks0164_device::bank2_select_r), FUNC(ks0164_device::bank2_select_w));

	map(0x0068, 0x0068).rw(FUNC(ks0164_device::mpu401_r), FUNC(ks0164_device::mpu401_w));
	map(0x0069, 0x0069).rw(FUNC(ks0164_device::mpu401_istatus_r), FUNC(ks0164_device::mpu401_istatus_w));
	map(0x006c, 0x006c).rw(FUNC(ks0164_device::midi_r), FUNC(ks0164_device::midi_w));
	map(0x006d, 0x006d).rw(FUNC(ks0164_device::midi_status_r), FUNC(ks0164_device::midi_status_w));

	map(0x0076, 0x0076).rw(FUNC(ks0164_device::irqen_76_r), FUNC(ks0164_device::irqen_76_w));
	map(0x0077, 0x0077).rw(FUNC(ks0164_device::irqen_77_r), FUNC(ks0164_device::irqen_77_w));

	map(0x0080, 0x3fff).r(FUNC(ks0164_device::rom_r));
	map(0x4000, 0x7fff).rw(FUNC(ks0164_device::bank1_r), FUNC(ks0164_device::bank1_w));
	map(0x8000, 0xbfff).rw(FUNC(ks0164_device::bank2_r), FUNC(ks0164_device::bank2_w));
	map(0xe000, 0xffff).ram();
}

u16 ks0164_device::uncomp_8_16(u8 value)
{
	int xp = value >> 5;
	s16 o = (0x10 | (value & 0xf)) << 10;
	o = o >> xp;
	if(value & 0x10)
		o = -o;
	return o;
}

void ks0164_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	for(int sample = 0; sample != outputs[0].samples(); sample++) {
		s32 suml = 0, sumr = 0;
		for(int voice = 0; voice < 0x20; voice++) {
			u16 *regs = m_sregs[voice];
			if(regs[0] & 0x0001) {

				u64 current = (u64(regs[1]) << 32) | (u64(regs[2]) << 16) | regs[3];

				if(current & 0xc000000000)
					continue;

				u32 adr = current >> 16;
				s16 samp0, samp1;
				switch(regs[0] & 0x8400) {
				case 0x0000: // 16 bits linear
					samp0 = m_mem_cache.read_word(2*adr);
					samp1 = m_mem_cache.read_word(2*adr+2);
					break;

				case 0x8400: // 8 bits compressed
					samp0 = uncomp_8_16(m_mem_cache.read_byte(adr));
					samp1 = uncomp_8_16(m_mem_cache.read_byte(adr+1));
					break;

				default:
					logerror("Sample mode %04x\n", regs[0] & 0x8400);
					samp0 = samp1 = 0;
					break;
				}

				s16 samp = samp0 + (((samp1 - samp0) * (current & 0xffff)) >> 16);
				u32 step = 0x10000 | (regs[8] & ~0x1f);
				u32 shift = regs[8] & 0x1f;
				if(shift > 0x10)
					step >>= 0x20 - shift;
				else if(shift)
					step <<= shift;
				current += step;
				u64 end = (u64(regs[0xd]) << 32) | (u64(regs[0xe]) << 16) | regs[0xf];
				if(current >= end) {
					// Is there a loop enabled flag?
					u64 loop = (u64(regs[9]) << 32) | (regs[0xa] << 16) | regs[0xb];
					current = current - end + loop;
				}
				regs[1] = current >> 32;
				regs[2] = current >> 16;
				regs[3] = current;

				suml += samp;
				sumr += samp;
			}
		}
		outputs[0].put_int(sample, suml, 32768 * 32);
		outputs[1].put_int(sample, sumr, 32768 * 32);
	}
}
