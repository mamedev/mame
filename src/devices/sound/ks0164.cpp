// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Samsung Semiconductor KS0164 Wavetable Synthesizer

#include "emu.h"
#include "ks0164.h"

#define LOG_KEYON  (1U << 1)
#define LOG_SERIAL (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_KEYON | LOG_SERIAL)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(KS0164, ks0164_device, "ks0164", "Samsung KS0164 Wavetable Synthesizer")

// Picked up from vrender0
const u16 ks0164_device::sample_dec[0x100]=
{
	0x8000, 0x8400, 0x8800, 0x8c00, 0x9000, 0x9400, 0x9800, 0x9c00,
	0xa000, 0xa400, 0xa800, 0xac00, 0xb000, 0xb400, 0xb800, 0xbc00,
	0x4000, 0x4400, 0x4800, 0x4c00, 0x5000, 0x5400, 0x5800, 0x5c00,
	0x6000, 0x6400, 0x6800, 0x6c00, 0x7000, 0x7400, 0x7800, 0x7c00,
	0xc000, 0xc200, 0xc400, 0xc600, 0xc800, 0xca00, 0xcc00, 0xce00,
	0xd000, 0xd200, 0xd400, 0xd600, 0xd800, 0xda00, 0xdc00, 0xde00,
	0x2000, 0x2200, 0x2400, 0x2600, 0x2800, 0x2a00, 0x2c00, 0x2e00,
	0x3000, 0x3200, 0x3400, 0x3600, 0x3800, 0x3a00, 0x3c00, 0x3e00,
	0xe000, 0xe100, 0xe200, 0xe300, 0xe400, 0xe500, 0xe600, 0xe700,
	0xe800, 0xe900, 0xea00, 0xeb00, 0xec00, 0xed00, 0xee00, 0xef00,
	0x1000, 0x1100, 0x1200, 0x1300, 0x1400, 0x1500, 0x1600, 0x1700,
	0x1800, 0x1900, 0x1a00, 0x1b00, 0x1c00, 0x1d00, 0x1e00, 0x1f00,
	0xf000, 0xf080, 0xf100, 0xf180, 0xf200, 0xf280, 0xf300, 0xf380,
	0xf400, 0xf480, 0xf500, 0xf580, 0xf600, 0xf680, 0xf700, 0xf780,
	0x0800, 0x0880, 0x0900, 0x0980, 0x0a00, 0x0a80, 0x0b00, 0x0b80,
	0x0c00, 0x0c80, 0x0d00, 0x0d80, 0x0e00, 0x0e80, 0x0f00, 0x0f80,
	0xf800, 0xf840, 0xf880, 0xf8c0, 0xf900, 0xf940, 0xf980, 0xf9c0,
	0xfa00, 0xfa40, 0xfa80, 0xfac0, 0xfb00, 0xfb40, 0xfb80, 0xfbc0,
	0x0400, 0x0440, 0x0480, 0x04c0, 0x0500, 0x0540, 0x0580, 0x05c0,
	0x0600, 0x0640, 0x0680, 0x06c0, 0x0700, 0x0740, 0x0780, 0x07c0,
	0xfc00, 0xfc20, 0xfc40, 0xfc60, 0xfc80, 0xfca0, 0xfcc0, 0xfce0,
	0xfd00, 0xfd20, 0xfd40, 0xfd60, 0xfd80, 0xfda0, 0xfdc0, 0xfde0,
	0x0200, 0x0220, 0x0240, 0x0260, 0x0280, 0x02a0, 0x02c0, 0x02e0,
	0x0300, 0x0320, 0x0340, 0x0360, 0x0380, 0x03a0, 0x03c0, 0x03e0,
	0xfe00, 0xfe10, 0xfe20, 0xfe30, 0xfe40, 0xfe50, 0xfe60, 0xfe70,
	0xfe80, 0xfe90, 0xfea0, 0xfeb0, 0xfec0, 0xfed0, 0xfee0, 0xfef0,
	0x0100, 0x0110, 0x0120, 0x0130, 0x0140, 0x0150, 0x0160, 0x0170,
	0x0180, 0x0190, 0x01a0, 0x01b0, 0x01c0, 0x01d0, 0x01e0, 0x01f0,
	0x0000, 0x0008, 0x0010, 0x0018, 0x0020, 0x0028, 0x0030, 0x0038,
	0x0040, 0x0048, 0x0050, 0x0058, 0x0060, 0x0068, 0x0070, 0x0078,
	0xff80, 0xff88, 0xff90, 0xff98, 0xffa0, 0xffa8, 0xffb0, 0xffb8,
	0xffc0, 0xffc8, 0xffd0, 0xffd8, 0xffe0, 0xffe8, 0xfff0, 0xfff8,
};

ks0164_device::ks0164_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KS0164, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  device_memory_interface(mconfig, *this),
	  device_serial_interface(mconfig, *this),
	  m_midi_tx(*this),
	  m_mem_region(*this, DEVICE_SELF),
	  m_cpu(*this, "cpu"),
	  m_mem_config("mem", ENDIANNESS_BIG, 16, 23),
	  m_notif_rom_space()
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

	m_notif_rom_space = space().add_change_notifier([this] (read_or_write mode) {
		// HACK: If something external changes the ROM space after initial load then reset the CPU because the program code also changed (used by BMkey ROM PCBs)
		for(int voice = 0; voice < 0x20; voice++) {
			// Disable all voices
			m_sregs[voice][0] &= ~1;
		}

		m_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	});

	m_stream = stream_alloc(0, 2, clock()/3/2/2/32);
	space().cache(m_mem_cache);
	m_timer = timer_alloc(FUNC(ks0164_device::irq_timer_tick), this);

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

TIMER_CALLBACK_MEMBER(ks0164_device::irq_timer_tick)
{
	m_timer_interrupt = true;
	if(m_irqen_76 & 0x40)
		m_cpu->set_input_line(14, ASSERT_LINE);
}

void ks0164_device::tra_complete()
{
	LOGMASKED(LOG_SERIAL, "transmit done\n");
}

void ks0164_device::rcv_complete()
{
	receive_register_extract();
	m_midi_in = get_received_char();
	m_midi_in_active = true;
	m_cpu->set_input_line(6, ASSERT_LINE);

	LOGMASKED(LOG_SERIAL, "recieved %02x\n", m_midi_in);
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
	LOG("want to transmit %02x\n", data);
}

u8 ks0164_device::midi_status_r()
{
	// transmit done/tx empty on bit 1
	return m_midi_in_active ? 1 : 0;
}

void ks0164_device::midi_status_w(u8 data)
{
	LOG("midi status_w %02x\n", data);
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
	if (!machine().side_effects_disabled())
	{
		m_mpu_status &= ~MPUS_TX_FULL;
		//  logerror("mpu pop %02x\n", m_mpu_out);
	}
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
	if (!machine().side_effects_disabled())
	{
		m_mpu_status &= ~MPUS_RX_FULL;
		m_cpu->set_input_line(11, CLEAR_LINE);
		//  logerror("mpu_r %02x (%04x)\n", m_mpu_in, m_cpu->pc());
	}
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
	// logerror("voice read %02x.%02x -> %04x (%04x)\n", m_voice_select & 0x1f, offset, m_sregs[m_voice_select & 0x1f][offset], m_cpu->pc());
	return m_sregs[m_voice_select & 0x1f][offset];
}

void ks0164_device::voice_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_stream->update();
	u16 old = m_sregs[m_voice_select & 0x1f][offset];
	COMBINE_DATA(&m_sregs[m_voice_select & 0x1f][offset]);
	if(0 && m_sregs[m_voice_select & 0x1f][offset] != old && offset == 0)
		LOGMASKED(LOG_KEYON, "voice %02x.%02x = %04x @ %04x (%04x)\n", m_voice_select & 0x1f, offset, m_sregs[m_voice_select & 0x1f][offset], mem_mask, m_cpu->pc());
	if(offset == 0 && (data & 1) && !(old & 1))
		LOGMASKED(LOG_KEYON, "keyon %02x mode=%04x (%s %c %c %c %c) cur=%02x%04x.%04x loop=%02x%04x.%04x end=%02x%04x.%04x pitch=%x.%03x 10=%02x/%02x:%02x/%02x 14=%03x/%03x:%03x/%03x 18=%04x/%04x c=%04x   %04x %04x %04x %04x %04x  %04x %04x %04x %04x %04x\n",
				 m_voice_select,

				 m_sregs[m_voice_select & 0x1f][0x00],

				 m_sregs[m_voice_select & 0x1f][0x00] & 0x8000 ? " 8" : "16", // 8-bit/16-bit samples
				 m_sregs[m_voice_select & 0x1f][0x00] & 0x0400 ? 'c' : 'l', // compressed/linear samples
				 m_sregs[m_voice_select & 0x1f][0x00] & 0x0010 ? '4' : '-',
				 m_sregs[m_voice_select & 0x1f][0x00] & 0x0008 ? 'l' : '-', // loop
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

				 m_sregs[m_voice_select & 0x1f][0x08] & 0xf, // pitch
				 m_sregs[m_voice_select & 0x1f][0x08] >> 4,

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
	LOG("irqen_77 = %02x (%04x)\n", m_irqen_77, m_cpu->pc());
}

u8 ks0164_device::unk60_r()
{
	return m_unk60;
}

void ks0164_device::unk60_w(u8 data)
{
	m_unk60 = data;
	LOG("unk60 = %02x (%04x)\n", m_unk60, m_cpu->pc());
}

u8 ks0164_device::voice_select_r()
{
	return m_voice_select;
}

void ks0164_device::voice_select_w(u8 data)
{
	m_voice_select = data;
	// logerror("voice_select = %02x (%04x)\n", m_voice_select, m_cpu->pc());
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

void ks0164_device::sound_stream_update(sound_stream &stream)
{
	for(int sample = 0; sample != stream.samples(); sample++) {
		s32 suml = 0, sumr = 0;
		for(int voice = 0; voice < 0x20; voice++) {
			u16 *regs = m_sregs[voice];
			if(regs[0] & 0x0001) {
				u64 current = (u64(regs[1]) << 32) | (u64(regs[2]) << 16) | regs[3];
				u32 adr = current >> 16;
				s16 samp0, samp1;
				switch(regs[0] & 0x8400) {
				case 0x0000: // 16 bits linear
					samp0 = m_mem_cache.read_word(2*adr);
					samp1 = m_mem_cache.read_word(2*adr+2);
					break;

				case 0x8400: // 8 bits compressed
					samp0 = sample_dec[m_mem_cache.read_byte(adr)];
					samp1 = sample_dec[m_mem_cache.read_byte(adr+1)];
					break;

				default:
					LOG("Sample mode %04x\n", regs[0] & 0x8400);
					samp0 = samp1 = 0;
					break;
				}

				s16 samp = samp0 + (((samp1 - samp0) * (current & 0xffff)) >> 16);
				u32 step = 0x10000 | (regs[8] & ~0xf);
				u32 shift = regs[8] & 0xf;
				if(shift >= 0x8)
					step >>= 0x10 - shift;
				else if(shift)
					step <<= shift;
				current += step;
				u64 end = (u64(regs[0xd]) << 32) | (u64(regs[0xe]) << 16) | regs[0xf];
				if(current >= end) {
					if (regs[0] & 8) {
						u64 loop = (u64(regs[9]) << 32) | (u64(regs[0xa]) << 16) | regs[0xb];
						while(current >= end)
							current = current - end + loop;
					} else {
						regs[0] = ~1;
						regs[0xc] = 0;
						regs[0x10] = regs[0x12] = regs[0x14] = regs[0x16] = 0;
					}
				}
				regs[1] = current >> 32;
				regs[2] = current >> 16;
				regs[3] = current;

				suml += (s64(samp) * regs[0x12] * regs[0x16]) >> 32;
				sumr += (s64(samp) * regs[0x10] * regs[0x14]) >> 32;

				if(regs[0xc]) {
					regs[0x10] += regs[0x11];
					regs[0x12] += regs[0x13];
					regs[0x14] += regs[0x15];
					regs[0x16] += regs[0x17];
					regs[0xc] --;
				}
			}
		}
		stream.put_int(0, sample, suml, 32768 * 32);
		stream.put_int(1, sample, sumr, 32768 * 32);
	}
}
