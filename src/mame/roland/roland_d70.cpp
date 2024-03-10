// license:BSD-3-Clause
// copyright-holders:giulioz, ValleyBell
/****************************************************************************

    Driver for Roland D-70 synthesizer.
    Derived by the CM32P driver by ValleyBell

****************************************************************************/

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/mcs96/i8x9x.h"
#include "cpu/mcs96/i8xc196.h"
#include "emu.h"
#include "emupal.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "screen.h"
#include "softlist_dev.h"
#include "sound/rolandpcm.h"
#include "speaker.h"
#include "video/t6963c.h"
#include <queue>

namespace {

// unscramble address: ROM dump offset -> proper (descrambled) offset
#define UNSCRAMBLE_ADDR_INT(_offset) \
	bitswap<19>(_offset, 18, 17, 15, 14, 16, 12, 11, 7, 9, 13, 10, 8, 3, 2, 1, 6, 4, 5, 0)
// scramble address: proper offset -> ROM dump offset
#define SCRAMBLE_ADDR_INT(_offset) \
	bitswap<19>(_offset, 18, 17, 14, 16, 15, 9, 13, 12, 8, 10, 7, 11, 3, 1, 2, 6, 5, 4, 0)

#define UNSCRAMBLE_DATA(_data) bitswap<8>(_data, 1, 2, 7, 3, 5, 0, 4, 6)

// Bitmasks for the display board interface via PORT1
static constexpr uint8_t CONT_MASK = 0b10000000;
static constexpr uint8_t RW_MASK = 0b01000000;
static constexpr uint8_t AD_MASK = 0b00100000;
static constexpr uint8_t SCK_MASK = 0b00010000;
static constexpr uint8_t SI_MASK = 0b00001000;
static constexpr uint8_t SO_MASK = 0b00000100;
static constexpr uint8_t ACK_MASK = 0b00000010;
static constexpr uint8_t ENCO_MASK = 0b00000001;

static INPUT_PORTS_START(d70)
  PORT_START("KEY0")
  PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Performance") PORT_CODE(KEYCODE_K)
  PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Patch") PORT_CODE(KEYCODE_L)
  PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Tone") PORT_CODE(KEYCODE_COLON)
  PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A/B") PORT_CODE(KEYCODE_Q)
  PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Int/Card") PORT_CODE(KEYCODE_W)
  PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Command") PORT_CODE(KEYCODE_E)
  PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Write") PORT_CODE(KEYCODE_R)
  PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Enter") PORT_CODE(KEYCODE_T)

  PORT_START("KEY1")
  PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bank 1")
  PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bank 2")
  PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bank 3")
  PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bank 4")
  PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bank 5")
  PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bank 6")
  PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bank 7")
  PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bank 8")

  PORT_START("KEY2")
  PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Number 1") PORT_CODE(KEYCODE_1)
  PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Number 2") PORT_CODE(KEYCODE_2)
  PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Number 3") PORT_CODE(KEYCODE_3)
  PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Number 4") PORT_CODE(KEYCODE_4)
  PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Number 5") PORT_CODE(KEYCODE_5)
  PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Number 6") PORT_CODE(KEYCODE_6)
  PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Number 7") PORT_CODE(KEYCODE_7)
  PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Number 8") PORT_CODE(KEYCODE_8)

  PORT_START("KEY3")
  PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Inc/Ins") PORT_CODE(KEYCODE_H)
  PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dec/Del") PORT_CODE(KEYCODE_J)
  PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("S") PORT_CODE(KEYCODE_DOWN)
  PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A") PORT_CODE(KEYCODE_LEFT)
  PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D") PORT_CODE(KEYCODE_RIGHT)
  PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("W") PORT_CODE(KEYCODE_UP)
  PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Midi Out") PORT_CODE(KEYCODE_N)
  PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Tone Display") PORT_CODE(KEYCODE_B)

  PORT_START("KEY4")
  PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Exit") PORT_CODE(KEYCODE_SLASH)
  PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F5") PORT_CODE(KEYCODE_G)
  PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F4") PORT_CODE(KEYCODE_F)
  PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F3") PORT_CODE(KEYCODE_D)
  PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F2") PORT_CODE(KEYCODE_S)
  PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F1") PORT_CODE(KEYCODE_A)
  PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("User") PORT_CODE(KEYCODE_STOP)
  PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part") PORT_CODE(KEYCODE_COMMA)

  PORT_START("KEY5")
  PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Edit") PORT_CODE(KEYCODE_X)
  PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Portamento")
  PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Resonance")
  PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Pan")
  PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Tuning") PORT_CODE(KEYCODE_V)
  PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Attack")
  PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Release")
  PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Pcm Card") PORT_CODE(KEYCODE_C)

  PORT_START("KEY6")
  PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Play") PORT_CODE(KEYCODE_Z)
  PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Solo")
  PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Cutoff")
  PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Level")
  PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Upper 4")
  PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Upper 3")
  PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Lower 2")
  PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Lower 1")

  PORT_START("KEY7")
  PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
  PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
  PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
  PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
  PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNUSED)
  PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNUSED)
  PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNUSED)
  PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Effect/Ctrl") PORT_CODE(KEYCODE_M)
INPUT_PORTS_END

class roland_d70_state : public driver_device {
public:
  roland_d70_state(const machine_config &mconfig, device_type type, const char *tag)
	  : driver_device(mconfig, type, tag), m_cpu(*this, "maincpu"),
		m_pcm(*this, "pcm"), m_lcd(*this, "lcd"), m_midi_timer(*this, "midi_timer"),
		m_ram(*this, "ram"), m_dsp_ram(*this, "dsp_ram"), m_keys(*this, "KEY%u", 0),
		sw_scan_index(0), sw_scan_bank(-1), sw_scan_state(0xFF), sw_scan_mode(0),
		sw_scan_current_out(0xFF), m_midi_rx(0), m_midi_pos(0) {}

	void d70(machine_config &config);
	void init_d70();

protected:
	virtual void machine_start() override;

private:
	required_device<i8x9x_device> m_cpu;
	required_device<mb87419_mb87420_device> m_pcm;
	required_device<t6963c_device> m_lcd;
	required_device<timer_device> m_midi_timer;
	required_device<ram_device> m_ram;
	required_device<ram_device> m_dsp_ram;
	required_ioport_array<8> m_keys;

	void lcd_map(address_map &map);
	void lcd_palette(palette_device &palette) const;

	void bank_w(uint8_t data);
	u8 ksga_io_r(offs_t offset);
	void ksga_io_w(offs_t offset, u8 data);
	u16 port0_r();
	u8 port1_r();
	void port1_w(u8 data);
	u8 pcmrom_r(offs_t offset);
	u8 dsp_io_r(offs_t offset);
	void dsp_io_w(offs_t offset, u8 data);
	u8 tvf_io_r(offs_t offset);
	void tvf_io_w(offs_t offset, u8 data);
	u8 snd_io_r(offs_t offset);
	void snd_io_w(offs_t offset, u8 data);

	u8 ach0_r();
	u8 ach1_r();
	u8 ach2_r();
	u8 ach3_r();
	u8 ach4_r();

	TIMER_DEVICE_CALLBACK_MEMBER(midi_timer_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(samples_timer_cb);

	void midi_in_w(int state);

	void d70_map(address_map &map);

	void descramble_rom_internal(u8 *dst, const u8 *src);
	void descramble_rom_external(u8 *dst, const u8 *src);

	u8 sound_io_buffer[0x100];
	u8 dsp_io_buffer[0x80];
	int sw_scan_index;
	int sw_scan_bank;
	u8 sw_scan_state;
	u8 sw_scan_mode;
	u8 sw_scan_current_out;
	u8 m_midi_rx;
	int m_midi_pos;
	std::queue<u8> midi_queue;
};

void roland_d70_state::machine_start() {
	membank("bank")->configure_entries(0, 8, memregion("maincpu")->base(), 0x4000);
	membank("bank")->configure_entries(0x20, 2, m_ram->pointer(), 0x4000);
	membank("bank")->configure_entries(0x30, 2, m_ram->pointer(), 0x4000);
	membank("fixed")->set_base(m_ram->pointer());
}

void roland_d70_state::bank_w(uint8_t data) {
	membank("bank")->set_entry(data);
}

u8 roland_d70_state::ksga_io_r(offs_t offset) {
	return 0;
}

void roland_d70_state::ksga_io_w(offs_t offset, u8 data) {
}

void roland_d70_state::lcd_map(address_map &map) {
	map(0x0000, 0x7fff).ram();
}

void roland_d70_state::midi_in_w(int state) {
	if (m_midi_pos == 0 || m_midi_pos == 9) {
		m_midi_pos += 1;
	} else if (m_midi_pos == 10) {
		midi_queue.push(m_midi_rx);
		// logerror("midi enqueued %x\n", m_midi_rx);
		m_midi_rx = 0;
		m_midi_pos = 0;
	} else {
		m_midi_rx |= state << (m_midi_pos - 1);
		m_midi_pos += 1;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(roland_d70_state::midi_timer_cb) {
	// CPU doesn't have a proper serial interface so we are forced
	// to simulate it this way for now
	if (midi_queue.empty())
		return;

	u8 midi = midi_queue.front();
	if (midi == 0x90)
		midi = 0x9a;
	if (midi == 0x80)
		midi = 0x8a;
	midi_queue.pop();
	logerror("midi_in %02x\n", midi);
	m_cpu->serial_w(midi);
}

u16 roland_d70_state::port0_r() {
	return 0x00;
}

u8 roland_d70_state::roland_d70_state::port1_r() {
	sw_scan_current_out = m_ram->read(0x0F);
	// logerror("p1r %02x\n", sw_scan_current_out);
	return sw_scan_current_out;
}

void roland_d70_state::roland_d70_state::port1_w(u8 data) {
	// logerror("p1w %02x\n", data);
	// sw_scan_current_out = data & ~(SO_MASK);

	if (data & RW_MASK) {
	if ((data & CONT_MASK) && !(sw_scan_state & CONT_MASK)) {
		sw_scan_index = 0;
		sw_scan_bank += 1;
	}

	// Clock cycle
	if ((data & SCK_MASK) && !(sw_scan_state & SCK_MASK)) {
		if ((data & AD_MASK)) {
		if (sw_scan_mode == 0) {
			sw_scan_current_out &= ~(SO_MASK);
		} else if (sw_scan_mode == 1) {
			u8 buttonState = sw_scan_bank != -1 ? (m_keys[sw_scan_bank]->read() >> sw_scan_index) & 0x1 : 1;

			if (buttonState) {
			sw_scan_current_out |= SO_MASK;
			} else {
			sw_scan_current_out &= ~(SO_MASK);
			}
			sw_scan_index += 1;
		}
		} else {
		sw_scan_mode += 1;
		}
	}
	} else {
	sw_scan_mode = 0;
	sw_scan_index = 0;
	sw_scan_bank = -1;
	}

	sw_scan_state = data;

	m_ram->write(0x0F, sw_scan_current_out);
}

u8 roland_d70_state::pcmrom_r(offs_t offset) {
	const u8 *pcm_rom = memregion("pcm")->base();
	// logerror("pcm read %x %x\n", offset, pcm_rom[offset]);
	return pcm_rom[offset];
}

u8 roland_d70_state::dsp_io_r(offs_t offset) {
	return dsp_io_buffer[offset];
}

void roland_d70_state::dsp_io_w(offs_t offset, u8 data) {
	dsp_io_buffer[offset] = data;
	// do read/write to some external memory, makes the RCC-CPU check pass.
	// (routine at 0x4679)
	switch (offset) {
		case 0x04:
		// write to partials?? (written in loop at 0x4375)
		break;

		case 0x06: {
		u8 *ram = m_dsp_ram->pointer();
		offs_t ofs = data;
		ram[0x000 | ofs] = dsp_io_buffer[0x00] & 0x03;
		ram[0x100 | ofs] = dsp_io_buffer[0x01];
		ram[0x200 | ofs] = dsp_io_buffer[0x02];
		} break;

		case 0x0A: {
		const u8 *ram = m_dsp_ram->pointer();
		offs_t ofs = data;
		dsp_io_buffer[0x00] = ram[0x000 | ofs];
		dsp_io_buffer[0x01] = ram[0x100 | ofs];
		dsp_io_buffer[0x02] = ram[0x200 | ofs];
		} break;
	}
}

u8 roland_d70_state::tvf_io_r(offs_t offset) {
	logerror("tvf read %x\n", offset);
	return 0;
}

void roland_d70_state::tvf_io_w(offs_t offset, u8 data) {
	logerror("twf write $x= %x\n", offset, data);
}

u8 roland_d70_state::snd_io_r(offs_t offset) {
	// logerror("lp read %x\n", offset);
	// lots of offset modification magic to achieve the following:
	//  - offsets 00..1F are "sound chip read"
	//  - offsets 20..3F are a readback of what was written to registers 00..1F
	//  - This behaviour is reversed for offset 01/21, which is used for reading
	//  the PCM sample tables.
	// All this is just for making debugging easier, as it allows one to check the
	// register state using the Memory Viewer.
	if (offset == 0x01 || offset == 0x21)
	offset ^= 0x20; // remove when PCM data readback via sound chip is confirmed
					// to work
	if (offset < 0x20)
	return m_pcm->read(offset);
	if (offset < 0x40)
	offset -= 0x20;

	if (offset == 0x01) {
		// code for reading from the PCM sample table is at 0xB027
		// The code at 0xB0AC writes to 1411/1F (??), then 1403/02 (bank), then
		// 1409/08/0B/0A (address). It waits a few cycles and at 0xB0F7 it reads the
		// resulting data from 1401.
		offs_t bank = sound_io_buffer[0x03];
		offs_t addr = (sound_io_buffer[0x09] << 0) | (sound_io_buffer[0x0A] << 8) |
						(sound_io_buffer[0x0B] << 16);
		addr = ((addr >> 6) + 2) & 0x3FFFF;
		addr |= (bank << 16);
		// write actual ROM address to 1440..1443 for debugging
		sound_io_buffer[0x43] = (addr >> 0) & 0xFF;
		sound_io_buffer[0x42] = (addr >> 8) & 0xFF;
		sound_io_buffer[0x41] = (addr >> 16) & 0xFF;
		sound_io_buffer[0x40] = (addr >> 24) & 0xFF;
		return pcmrom_r(addr);
	}
	return sound_io_buffer[offset];
}

void roland_d70_state::snd_io_w(offs_t offset, u8 data) {
	// register map
	// ------------
	// Note: 16-bit words are Little Endian, the firmware writes the odd byte is
	// first
	//  00/01 - ??
	//  02/03 - ROM bank (only bits 11-13 are used, bit 11 = PCM card, bits 12-13
	//  select between IC18/19/20) 04/05 - frequency (2.14 fixed point, 0x4000 =
	//  32000 Hz) 06/07 - volume 08/09 - sample start address, fraction (2.14
	//  fixed point, i.e. 1 byte = 0x4000) 0A/0B - sample start address (high
	//  word, i.e. address bits 2..17) 0C/0D - sample end address (high word)
	//  0E/0F - sample loop address (high word)
	//  11/13/15/17 - voice enable mask (11 = least significant 8 bits, 17 = most
	//  significant 8 bits) 1A - ?? 1F - voice select
	if (offset < 0x20) {
		m_pcm->write(offset, data);
	}
	sound_io_buffer[offset] = data;
}

u8 roland_d70_state::ach0_r() { return 128; }
u8 roland_d70_state::ach1_r() { return 128; }
u8 roland_d70_state::ach2_r() { return 128; }
u8 roland_d70_state::ach3_r() { return 128; }
u8 roland_d70_state::ach4_r() { return 128; }

TIMER_DEVICE_CALLBACK_MEMBER(roland_d70_state::samples_timer_cb) {
}

void roland_d70_state::lcd_palette(palette_device &palette) const {
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(69, 62, 66));
}

void roland_d70_state::d70_map(address_map &map) {
	map(0x0100, 0x0100).w(FUNC(roland_d70_state::bank_w));
	map(0x0400, 0x07ff).rw(FUNC(roland_d70_state::ksga_io_r), FUNC(roland_d70_state::ksga_io_w));
	map(0x0800, 0x0802).rw(m_lcd, FUNC(t6963c_device::read), FUNC(t6963c_device::write)).umask16(0x00FF);
	map(0x0900, 0x09ff).rw(FUNC(roland_d70_state::snd_io_r), FUNC(roland_d70_state::snd_io_w));
	map(0x0a00, 0x0aff).rw(FUNC(roland_d70_state::dsp_io_r), FUNC(roland_d70_state::dsp_io_w));
	map(0x0c00, 0x0cff).rw(FUNC(roland_d70_state::tvf_io_r), FUNC(roland_d70_state::tvf_io_w));
	map(0x1000, 0x7FFF).rom().region("maincpu", 0x1000);
	map(0x8000, 0xbfff).bankrw("bank");
	map(0xc000, 0xffff).bankrw("fixed");
}

void roland_d70_state::d70(machine_config &config) {
	i8x9x_device &maincpu(N8097BH(config, m_cpu, 12_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &roland_d70_state::d70_map);
	maincpu.serial_tx_cb().set("mdout", FUNC(midi_port_device::write_txd));
	maincpu.in_p0_cb().set(FUNC(roland_d70_state::port0_r));
	maincpu.in_p1_cb().set(FUNC(roland_d70_state::port1_r));
	maincpu.out_p1_cb().set(FUNC(roland_d70_state::port1_w));
	maincpu.ach0_cb().set(FUNC(roland_d70_state::ach0_r));
	maincpu.ach1_cb().set(FUNC(roland_d70_state::ach1_r));
	maincpu.ach2_cb().set(FUNC(roland_d70_state::ach2_r));
	maincpu.ach3_cb().set(FUNC(roland_d70_state::ach3_r));
	maincpu.ach4_cb().set(FUNC(roland_d70_state::ach4_r));

	RAM(config, m_ram).set_default_size("128K");

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MB87419_MB87420(config, m_pcm, 32.768_MHz_XTAL);
	m_pcm->int_callback().set_inputline(m_cpu, i8x9x_device::EXTINT_LINE);
	m_pcm->add_route(0, "lspeaker", 1.0);
	m_pcm->add_route(1, "rspeaker", 1.0);

	RAM(config, m_dsp_ram).set_default_size("8K");

	T6963C(config, m_lcd, 0);
	m_lcd->set_addrmap(0, &roland_d70_state::lcd_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_size(240, 64);
	screen.set_visarea_full();
	screen.set_screen_update("lcd", FUNC(t6963c_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(roland_d70_state::lcd_palette), 2);

	TIMER(config, m_midi_timer).configure_periodic(FUNC(roland_d70_state::midi_timer_cb), attotime::from_hz(1250));

	TIMER(config, "samples_timer").configure_periodic(FUNC(roland_d70_state::samples_timer_cb), attotime::from_hz(32000 * 2));

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(FUNC(roland_d70_state::midi_in_w));
	mdin.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
	MIDI_PORT(config, "mdthru", midiout_slot, "midiout");
}

void roland_d70_state::init_d70() {
	// Roland did a fair amount of scrambling on the address and data lines.
	// Only the first 0x80 bytes of the ROMs are readable text in a raw dump.
	// The U-110 actually checks some of these header bytes, but it uses
	// post-scrambling variants of offsets/values.
	u8 *src = static_cast<u8 *>(memregion("pcmorg")->base());
	u8 *dst = static_cast<u8 *>(memregion("pcm")->base());
	// descramble internal ROMs
	descramble_rom_internal(&dst[0x000000], &src[0x000000]);
	descramble_rom_internal(&dst[0x100000], &src[0x100000]);
	descramble_rom_internal(&dst[0x200000], &src[0x200000]);
	descramble_rom_internal(&dst[0x300000], &src[0x300000]);
	descramble_rom_internal(&dst[0x400000], &src[0x400000]);
	descramble_rom_internal(&dst[0x500000], &src[0x500000]);
}

void roland_d70_state::descramble_rom_internal(u8 *dst, const u8 *src) {
	for (offs_t srcpos = 0x00; srcpos < 0x80000; srcpos++) {
		const offs_t dstpos = UNSCRAMBLE_ADDR_INT(srcpos);
		dst[dstpos] = UNSCRAMBLE_DATA(src[srcpos]);
	}
}

ROM_START(d70)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v203", "Version 2.03")
	ROMX_LOAD("roland_d70_v110_combined.bin", 0x00000, 0x20000, CRC(52deab1e) SHA1(87d7196888edec65c9feddd16d4c715f6992abc7), ROM_BIOS(0))

	ROM_REGION(0x600000, "pcmorg", 0) // ROMs before descrambling
	ROM_LOAD("roland_d70_waverom-a.bin", 0x000000, 0x80000, CRC(8e53b2a3) SHA1(4872530870d5079776e80e477febe425dc0ec1df))
	ROM_LOAD("roland_d70_waverom-b.bin", 0x100000, 0x80000, CRC(c8220761) SHA1(49e55fa672020f95fd9c858ceaae94d6db93df7d))
	ROM_LOAD("roland_d70_waverom-c.bin", 0x200000, 0x80000, CRC(733c4054) SHA1(9b6b59ab74e5bf838702abb087c408aaa85b7b1f))
	ROM_LOAD("roland_d70_waverom-d.bin", 0x300000, 0x80000, CRC(b6c662d2) SHA1(3fcbcfd0d8d0fa419c710304c12482e2f79a907f))
	ROM_LOAD("roland_d70_waverom-e.bin", 0x400000, 0x80000, CRC(d46cc7a4) SHA1(d378ac89a5963e37f7c157b3c8e71892c334fd7b))
	ROM_LOAD("roland_d70_waverom-f.bin", 0x500000, 0x80000, CRC(d4b01f5e) SHA1(acd867d68e49e5f59f1006ed14a7ca197b6dc4af))
	ROM_REGION(0x600000, "pcm", ROMREGION_ERASEFF) // ROMs after descrambling

	ROM_REGION(0x400, "lcd:cgrom", 0)
	ROM_LOAD("t6963c_0101.bin", 0x000, 0x400, CRC(547d118b) SHA1(0dd3e3acd3d47e6ece644c98c390fc86587373e9))
	// This t6963c_0101 internal CG ROM is similar to lm24014w_0101.bin which may be
	// used as a replacement
ROM_END

} // anonymous namespace

SYST(1991, d70, 0, 0, d70, d70, roland_d70_state, init_d70, "Roland", "D-70 Super LA Synthetizer", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
