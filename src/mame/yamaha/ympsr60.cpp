// license:BSD-3-Clause
// copyright-holders:R. Belmont,Aaron Giles
/*
    Yamaha PSR-60/PSR-70 PortaSound keyboards
    Preliminary driver by R. Belmont, major thanks to reverse-engineering work by JKN0

    Note: PSR-50 is likely the same hardware.

    Special thanks to

    Documentation: https://github.com/JKN0/PSR70-reverse
    More documentation: https://retroandreverse.blogspot.com/2021/01/reversing-psr-70-hardware.html
                        https://retroandreverse.blogspot.com/2021/01/reversing-psr-70-firmware.html
                        https://retroandreverse.blogspot.com/2021/01/digging-into-ymopq.html

    Service manuals: https://elektrotanya.com/yamaha_psr-60.pdf/download.html
                     https://elektrotanya.com/yamaha_psr-70_sm.pdf/download.html

    Thanks to Lord Nightmare for help with the BBD filter chain

    CPU: Z80 @ 6 MHz
    Sound: YM3533 "OPQ" FM @ 3.58 MHz + YM2154 "RYP" sample playback chip for drums
    Panel and keyboard I/O: 82C55A PPI and Yamaha IG14330 "DRVIF"
    MIDI I/O: HD6350 ACIA, baud rate clock is 500 kHz

    Z80 IRQ is a wire-OR of IRQs from the OPQ, RYP4, ACIA, and DRVIF

    PPI ports:
    PA0-PA5: keyboard matrix readback (PA5-PA0 for most matrix selects, where PA5 is the lowest key and PA0 is the highest)
    PA6: single-use matrix readback for C1 (when PC1 is active)
    PA7: cassette input
    Keyboard matrix select writes:
        PC1: C1 - F#1
        PC0: G1 - C2
        PB7: C#2 - F#2
        PB6: G2 - C3
        PB5: C#3 - F#3
        PB4: G3 - C4
        PB3: C#4 - F#4
        PB2: G4 - C5
        PB1: C#5 - F#5
        PB0: G5 - C6
    PC3: unused
    PC4: ROM2 bank (goes to A14)
    PC5: cassette output
    PC6: MUTE
    PC7: to "RS2" signal, IC (reset, "initial clear" in Yamaha-speak) pin on T9500
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "sound/bbd.h"
#include "sound/flt_biquad.h"
#include "sound/flt_rc.h"
#include "sound/mixer.h"
#include "sound/ym2154.h"
#include "sound/ymopq.h"
#include "bus/midi/midi.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "psr60.lh"
#include "psr70.lh"

namespace {

class psr60_state : public driver_device
{
	static constexpr int DRVIF_MAX_TARGETS = 23;
	static constexpr int RYP4_MAX_TARGETS = 10;

public:
	psr60_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ym3533(*this, "ym3533"),
		m_ppi(*this, "ppi"),
		m_acia(*this, "acia"),
		m_lmixer(*this, "lmixer"),
		m_rmixer(*this, "rmixer"),
		m_bbd_mixer(*this, "bbdmixer"),
		m_bbd(*this, "bbd"),
		m_ryp4(*this, "ryp4"),
		m_ic206b(*this, "ic206b"),
		m_ic206a(*this, "ic206a"),
		m_ic205(*this, "ic205"),
		m_postbbd_rc(*this, "postbbd_rc"),
		m_ic204b(*this, "ic204b"),
		m_first_rc(*this, "first_rc"),
		m_ic204a(*this, "ic204a"),
		m_rom2bank(*this, "rom2bank"),
		m_keyboard(*this, "P1_%u", 0),
		m_drvif(*this, "DRVIF_%u", 0),
		m_drvif_out(*this, "DRVIF_%u_DP%u", 0U, 1U),
		m_ryp4_in(*this, "RYP4_%u", 1U),
		m_ryp4_out(*this, "RYP4_%u", 1U),
		m_mastervol(*this, "MASTERVOL")
	{ }

	void psr_common(machine_config &config);
	void psr60(machine_config &config);
	void psr70(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<ym3533_device> m_ym3533;
	required_device<i8255_device> m_ppi;
	required_device<acia6850_device> m_acia;
	required_device<mixer_device> m_lmixer;
	required_device<mixer_device> m_rmixer;
	required_device<mixer_device> m_bbd_mixer;
	required_device<mn3204p_device> m_bbd;
	required_device<ym2154_device> m_ryp4;
	required_device<filter_biquad_device> m_ic206b;
	required_device<filter_biquad_device> m_ic206a;
	required_device<filter_biquad_device> m_ic205;
	required_device<filter_rc_device> m_postbbd_rc;
	required_device<filter_biquad_device> m_ic204b;
	required_device<filter_rc_device> m_first_rc;
	required_device<filter_biquad_device> m_ic204a;
	required_memory_bank m_rom2bank;
	required_ioport_array<10> m_keyboard;
	required_ioport_array<DRVIF_MAX_TARGETS> m_drvif;
	output_finder<DRVIF_MAX_TARGETS, 4> m_drvif_out;
	required_ioport_array<RYP4_MAX_TARGETS> m_ryp4_in;
	output_finder<RYP4_MAX_TARGETS> m_ryp4_out;
	required_ioport m_mastervol;

	void psr60_map(address_map &map) ATTR_COLD;
	void psr60_io_map(address_map &map) ATTR_COLD;

	u8 ppi_pa_r();
	void ppi_pb_w(u8 data);
	void ppi_pc_w(u8 data);
	void recalc_irqs();

	TIMER_CALLBACK_MEMBER(bbd_tick);
	void bbd_setup_next_tick();

	emu_timer *m_bbd_timer;

	int m_acia_irq, m_ym_irq, m_drvif_irq, m_ym2154_irq;
	u16 m_keyboard_select;
	u8 m_bbd_config;
	u8 m_drvif_data[2];
	u8 m_drvif_select;
	u8 m_sustain_fuzz;

	void write_acia_clock(int state) { m_acia->write_txc(state); m_acia->write_rxc(state); }
	void acia_irq_w(int state) { m_acia_irq = state; recalc_irqs(); }
	void ym_irq_w(int state) { m_ym_irq = state; recalc_irqs(); }
	void ryp4_irq_w(int state) { m_ym2154_irq = state; recalc_irqs(); }

	u8 ryp4_an_r(offs_t offset);
	void ryp4_out_w(u8 data);

	u8 drvif_r(offs_t offset);
	void drvif_w(offs_t offset, u8 data);

public:
	INPUT_CHANGED_MEMBER(drvif_changed);
	INPUT_CHANGED_MEMBER(mastervol_changed);

	// optional sustain pedal input; if this doesn't change, sustain will not work
	// if no pedal present, it seems sustain should still work, so toggle the value
	// here a bit to make the keyboard notice
	ioport_value sustain_fuzz() { return (m_sustain_fuzz = !m_sustain_fuzz) ? 8 : 12; }
};

void psr60_state::psr60_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("rom1", 0);
	map(0x8000, 0xbfff).bankr("rom2bank");
	map(0xc000, 0xc0ff).rw(m_ym3533, FUNC(ym3533_device::read), FUNC(ym3533_device::write));
	map(0xe000, 0xffff).ram();  // work RAM
}

void psr60_state::psr60_io_map(address_map &map)
{
	map.global_mask(0xff);  // top 8 bits of the address are ignored by this hardware for I/O access
	map(0x10, 0x11).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x20, 0x23).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x30, 0x3f).rw(FUNC(psr60_state::drvif_r), FUNC(psr60_state::drvif_w));
	map(0x80, 0xff).rw(m_ryp4, FUNC(ym2154_device::read), FUNC(ym2154_device::write));
}

u8 psr60_state::ppi_pa_r()
{
	u8 result = 0;
	for (int bit = 0; bit < 10; bit++)
		if (BIT(m_keyboard_select, bit))
			result |= m_keyboard[bit]->read();
	return result;
}

void psr60_state::ppi_pb_w(u8 data)
{
	m_keyboard_select = (m_keyboard_select & ~0xff) | data;
}

void psr60_state::ppi_pc_w(u8 data)
{
	m_rom2bank->set_entry(BIT(data, 4));
	m_keyboard_select = (m_keyboard_select & ~0x300) | ((data & 3) << 8);
}

u8 psr60_state::ryp4_an_r(offs_t offset)
{
	return (offset < RYP4_MAX_TARGETS) ? (m_ryp4_in[offset]->read() * 255 / 100) : 0;
}

void psr60_state::ryp4_out_w(u8 data)
{
	m_bbd_config = data;

	// bit 0 (CT0) enables/disables the effect
	m_ic206a->set_output_gain(0, BIT(data, 0) ? 1.0 : 0.0);

	// bits 1 + 2 go to the 'T' and 'C' pins and control the frequency
	// modulation, which we simulate in a periodic timer
}

TIMER_CALLBACK_MEMBER(psr60_state::bbd_tick)
{
	m_bbd->tick();
	bbd_setup_next_tick();
}

void psr60_state::bbd_setup_next_tick()
{
	attotime curtime = machine().time();

	// only two states have been observed to be measured: CT1=1/CT2=0 and CT1=0/CT2=1
	double bbd_freq;
	if (BIT(m_bbd_config, 1) && !BIT(m_bbd_config, 2))
	{
		// Stereo symphonic off: min freq 35 kHz, max freq 107 kHz, varies at 0,3 Hz
		curtime.m_seconds %= 3;
		double pos = curtime.as_double() / 3;
		pos = (pos < 0.5) ? (2 * pos) : 2 * (1.0 - pos);
		bbd_freq = 35000 + (107000 - 35000) * pos;
	}
	else
	{
		// Stereo symphonic on: min freq 48 kHz, max freq 61 kHz, varies at 6 Hz
		curtime.m_seconds = 0;
		double pos = curtime.as_double() * 6;
		pos -= floor(pos);
		pos = (pos < 0.5) ? (2 * pos) : 2 * (1.0 - pos);
		bbd_freq = 48000 + (61000 - 48000) * pos;
	}

	// BBD driver provides two out-of-phase clocks to basically run the BBD at 2x
	m_bbd_timer->adjust(attotime::from_ticks(1, bbd_freq * 2));
}

//
// DRVIF: driver interface chip
//
// This chip manages a number of external drivers. Each driver has 4 switch
// inputs and 4 LED outputs. Interrupts are triggered when changes are detected,
// and the index of the changed port is provided.
//
// In reality, this is probably a microcontroller.
//
u8 psr60_state::drvif_r(offs_t offset)
{
	if (offset == 0)
		return (m_drvif_irq << 7) | 0x40;
	else if (offset <= 2)
	{
		m_drvif_irq = 0;
		recalc_irqs();
		return m_drvif_data[offset - 1];
	}
	else
		return 0;
}
void psr60_state::drvif_w(offs_t offset, u8 data)
{
	if (offset == 1)
		m_drvif_select = data & 0x1f;
	else if (offset == 2 && m_drvif_select < DRVIF_MAX_TARGETS)
	{
		for (int bit = 0; bit < 4; bit++)
			m_drvif_out[m_drvif_select][bit] = BIT(data, 3 - bit);
	}
//  else
//      printf("DRVIF: %02X = %02X\n", offset, data);
}

INPUT_CHANGED_MEMBER(psr60_state::drvif_changed)
{
	m_drvif_irq = 1;
	m_drvif_data[0] = param;
	m_drvif_data[1] = m_drvif[param]->read();
	recalc_irqs();
}

INPUT_CHANGED_MEMBER(psr60_state::mastervol_changed)
{
	float mastervol = m_mastervol->read() / 50.0;
	m_lmixer->set_output_gain(0, mastervol);
	m_rmixer->set_output_gain(0, mastervol);
}

void psr60_state::recalc_irqs()
{
	int const irq_state = m_acia_irq | m_ym_irq | m_drvif_irq | m_ym2154_irq;
	m_maincpu->set_input_line(0, irq_state);
}

void psr60_state::machine_start()
{
	m_bbd_timer = timer_alloc(FUNC(psr60_state::bbd_tick), this);

	m_drvif_out.resolve();
	m_rom2bank->configure_entries(0, 2, memregion("rom2")->base(), 0x4000);
	m_rom2bank->set_entry(0);
	m_acia_irq = 0;
	m_ym_irq = 0;
	m_drvif_irq = 0;
	m_ym2154_irq = 0;
}

void psr60_state::machine_reset()
{
	bbd_setup_next_tick();
}

#define DRVIF_PORT(num, sw1, sw2, sw3, sw4) \
	PORT_START("DRVIF_" #num) \
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(sw1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr60_state::drvif_changed), num) \
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(sw2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr60_state::drvif_changed), num) \
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(sw3) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr60_state::drvif_changed), num) \
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(sw4) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr60_state::drvif_changed), num)

#define RYP4_PORT(num, defval, name) \
	PORT_START("RYP4_" #num) \
	PORT_ADJUSTER(defval, name)

static INPUT_PORTS_START(psr60)
	PORT_START("P1_9")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 C")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 F#")

	PORT_START("P1_8")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 C")

	PORT_START("P1_7")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 F#")

	PORT_START("P1_6")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Octave 2 C")

	PORT_START("P1_5")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_NAME("Octave 2 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_NAME("Octave 2 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_NAME("Octave 2 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_NAME("Octave 2 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_NAME("Octave 2 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_NAME("Octave 2 F#")

	PORT_START("P1_4")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_NAME("Octave 2 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_NAME("Octave 2 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_NAME("Octave 2 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_NAME("Octave 2 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_NAME("Octave 2 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Octave 3 C")

	PORT_START("P1_3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("Octave 3 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_NAME("Octave 3 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("Octave 3 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_NAME("Octave 3 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_NAME("Octave 3 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("Octave 3 F#")

	PORT_START("P1_2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_NAME("Octave 3 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("Octave 3 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_NAME("Octave 3 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_NAME("Octave 3 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_NAME("Octave 3 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_NAME("Octave 4 C")

	PORT_START("P1_1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1_0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )

	DRVIF_PORT( 0, "Pause", "Unused1.2", "Unused1.3", "Unused1.4")
	DRVIF_PORT( 1, "Pitch Up", "Pitch Down", "Transposer Up", "Transposer Down")
	DRVIF_PORT( 2, "Memory", "Fingered", "Single Finger", "Off")
	DRVIF_PORT( 3, "Fill In 3", "Fill In 2", "Fill In 1", "Keyboard Percussion")
	DRVIF_PORT( 4, "Orchestra On", "Variation", "Hand Clap 2", "Hand Clap 1")
	DRVIF_PORT( 5, "Pops", "Disco", "Reggae", "Big Band")
	DRVIF_PORT( 6, "March/Polka", "Samba", "Salsa", "Rock'N'Roll")
	DRVIF_PORT( 7, "Intro/Ending", "Start", "Synchro", "Stop")
	DRVIF_PORT( 8, "MIDI Mode", "Unused9.2", "Unused9.3", "Unused9.4")
	DRVIF_PORT( 9, "Brass 1", "Strings", "Pipe Organ", "Jazz Organ")
	DRVIF_PORT(10, "Calliope", "Clarinet", "Brass & Chimes", "Brass 2")
	DRVIF_PORT(11, "Unused12.1", "Unused12.2", "Unused12.3", "Unused12.4")
	DRVIF_PORT(12, "Unused13.1", "Unused13.2", "Unused13.3", "Unused13.4")
	DRVIF_PORT(13, "Solo On", "To Lower", "Trio", "Duet")
	DRVIF_PORT(14, "Sustain", "Stereo Symphonic", "Sustain 2", "Sustain 1")
	DRVIF_PORT(15, "Trumpet", "Violin", "Piccolo", "Jazz Flute")
	DRVIF_PORT(16, "Oboe", "Saxophone", "Horn", "Trombone")
	DRVIF_PORT(17, "Pop Synth", "Percussion 2", "Percussion 1", "Electric Guitar")
	DRVIF_PORT(18, "Bass", "SlapSynth", "Funk Synth/Blues Synth", "Programmer Off")
	DRVIF_PORT(19, "Save", "Record Solo", "Record Orchestra", "Record Chord/Bass")
	DRVIF_PORT(20, "Load", "Play Back Solo", "Play Back Orchestra", "Play Back Chord/Bass")
	DRVIF_PORT(21, "Unused22.1", "Unused22.2", "Unused22.3", "Unused22.4")
	DRVIF_PORT(22, "Unused23.1", "Unused23.2", "Unused23.3", "Unused23.4")

	RYP4_PORT( 1, 75, "Solo Volume")
	RYP4_PORT( 2, 75, "Orchestra Volume")
	RYP4_PORT( 3, 75, "Rhythm Volume")
	RYP4_PORT( 4, 50, "Rhythm Tempo")
	RYP4_PORT( 5, 75, "Chord Volume")
	RYP4_PORT( 6, 75, "Bass Volume")
	RYP4_PORT( 7,  0, "Sustain") PORT_CUSTOM_MEMBER(FUNC(psr60_state::sustain_fuzz))
	RYP4_PORT( 8,  0, "Unused8")
	RYP4_PORT( 9,  0, "Unused9")
	RYP4_PORT(10,  0, "Unused10")

	PORT_START("MASTERVOL")
	PORT_ADJUSTER(50, "PSR Master Volume") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr60_state::mastervol_changed), 0)
INPUT_PORTS_END

static INPUT_PORTS_START(psr70)
	PORT_START("P1_9")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 C")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 F#")

	PORT_START("P1_8")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 0 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 C")

	PORT_START("P1_7")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 F#")

	PORT_START("P1_6")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 1 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Octave 2 C")

	PORT_START("P1_5")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_NAME("Octave 2 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_NAME("Octave 2 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_NAME("Octave 2 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_NAME("Octave 2 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_NAME("Octave 2 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_NAME("Octave 2 F#")

	PORT_START("P1_4")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_NAME("Octave 2 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_NAME("Octave 2 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_NAME("Octave 2 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_NAME("Octave 2 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_NAME("Octave 2 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Octave 3 C")

	PORT_START("P1_3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("Octave 3 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_NAME("Octave 3 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("Octave 3 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_NAME("Octave 3 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_NAME("Octave 3 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("Octave 3 F#")

	PORT_START("P1_2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_NAME("Octave 3 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("Octave 3 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_NAME("Octave 3 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_NAME("Octave 3 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_NAME("Octave 3 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_NAME("Octave 4 C")

	PORT_START("P1_1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 4 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 4 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 4 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 4 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 4 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 4 F#")

	PORT_START("P1_0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 4 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 4 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 4 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 4 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 4 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Octave 5 C")

	DRVIF_PORT( 0, "Unused1.1", "Unused1.2", "Unused1.3", "Split")
	DRVIF_PORT( 1, "Pitch Up", "Pitch Down", "Transposer Up", "Transposer Down")
	DRVIF_PORT( 2, "Memory", "Fingered", "Single Finger", "Off")
	DRVIF_PORT( 3, "Fill In 3", "Fill In 2", "Fill In 1", "Keyboard Percussion")
	DRVIF_PORT( 4, "Pops", "Disco", "Reggae", "Big Band")
	DRVIF_PORT( 5, "March/Polka", "Samba", "Salsa", "Rock'N'Roll")
	DRVIF_PORT( 6, "Intro/Ending", "Start", "Synchro", "Stop")
	DRVIF_PORT( 7, "MIDI Mode", "Unused9.2", "Unused9.3", "Custom Clear")
	DRVIF_PORT( 8, "Orchestra On", "Variation", "Hand Clap 2", "Hand Clap 1")
	DRVIF_PORT( 9, "Pause", "Custom 3", "Custom 2", "Custom 1")
	DRVIF_PORT(10, "Program", "Rhythm", "Bass", "Chord")
	DRVIF_PORT(11, "Solo On", "To Lower", "Trio", "Duet")
	DRVIF_PORT(12, "Brass 1", "Strings", "Pipe Organ", "Jazz Organ")
	DRVIF_PORT(13, "Calliope", "Clarinet", "Brass & Chimes", "Brass 2")
	DRVIF_PORT(14, "Unused15.1", "Unused15.2", "Unused15.3", "Unused15.4")
	DRVIF_PORT(15, "Unused16.1", "Unused16.2", "Unused16.3", "Unused16.4")
	DRVIF_PORT(16, "Solo Sustain", "Stereo Symphonic", "Sustain 2", "Sustain 1")
	DRVIF_PORT(17, "Trumpet", "Violin", "Piccolo", "Jazz Flute")
	DRVIF_PORT(18, "Oboe", "Saxophone", "Horn", "Trombone")
	DRVIF_PORT(19, "Pop Synth", "Percussion 2", "Percussion 1", "Programmer Off")
	DRVIF_PORT(20, "Registration Memory", "Program 3", "Program 2", "Program 1")
	DRVIF_PORT(21, "Unused22.1", "Record Solo", "Record Orchestra", "Record Chord/Bass")
	DRVIF_PORT(22, "Unused23.1", "Play Back Solo", "Play Back Orchestra", "Play Back Chord/Bass")

	RYP4_PORT( 1, 75, "Solo Volume")
	RYP4_PORT( 2, 75, "Orchestra Volume")
	RYP4_PORT( 3, 75, "Rhythm Volume")
	RYP4_PORT( 4, 50, "Rhythm Tempo")
	RYP4_PORT( 5, 75, "Chord Volume")
	RYP4_PORT( 6, 75, "Bass Volume")
	RYP4_PORT( 7,  0, "Sustain") PORT_CUSTOM_MEMBER(FUNC(psr60_state::sustain_fuzz))
	RYP4_PORT( 8,  0, "Unused8")
	RYP4_PORT( 9,  0, "Unused9")
	RYP4_PORT(10,  0, "Unused10")

	PORT_START("MASTERVOL")
	PORT_ADJUSTER(50, "PSR Master Volume") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr60_state::mastervol_changed), 0)
INPUT_PORTS_END

void psr60_state::psr_common(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &psr60_state::psr60_map);
	m_maincpu->set_addrmap(AS_IO, &psr60_state::psr60_io_map);

	I8255A(config, m_ppi, 6_MHz_XTAL);
	m_ppi->in_pa_callback().set(FUNC(psr60_state::ppi_pa_r));
	m_ppi->out_pb_callback().set(FUNC(psr60_state::ppi_pb_w));
	m_ppi->out_pc_callback().set(FUNC(psr60_state::ppi_pc_w));

	ACIA6850(config, m_acia, 500_kHz_XTAL); // actually an HD6350, differences unknown (if any)
	m_acia->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_acia->irq_handler().set(FUNC(psr60_state::acia_irq_w));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	clock_device &acia_clock(CLOCK(config, "acia_clock", 500_kHz_XTAL));    // 31250 * 16 = 500,000
	acia_clock.signal_handler().set(FUNC(psr60_state::write_acia_clock));

	SPEAKER(config, "speaker", 2).front();

	MIXER(config, m_lmixer);
	m_lmixer->add_route(0, "speaker", 1.0, 0);

	MIXER(config, m_rmixer);
	m_rmixer->add_route(0, "speaker", 1.0, 1);

	// begin BBD filter chain....
	// thanks to Lord Nightmare for figuring this out
	FILTER_BIQUAD(config, m_ic206b);
	m_ic206b->opamp_mfb_lowpass_setup(RES_K(100), 0, RES_K(100), 0, CAP_P(100));
	m_ic206b->add_route(0, m_lmixer, 0.11);
	m_ic206b->add_route(0, m_rmixer, 0.11);

	// not accurate, not correctly modeling the 22k resistor in series with the cap
	FILTER_BIQUAD(config, m_ic206a);
	m_ic206a->opamp_mfb_lowpass_setup(RES_K(33), 0, RES_K(22), 0, CAP_P(0.0039));
	m_ic206a->add_route(0, m_ic206b, 1.0);

	FILTER_BIQUAD(config, m_ic205);
	m_ic205->opamp_sk_lowpass_setup(RES_K(22), RES_K(22), RES_M(999.9), RES_R(0.001), CAP_U(0.0068), CAP_P(82));
	m_ic205->add_route(0, m_ic206a, 1.0);

	FILTER_RC(config, m_postbbd_rc);
	m_postbbd_rc->set_rc(filter_rc_device::LOWPASS, RES_K(22), 0, 0, CAP_U(0.0015));
	m_postbbd_rc->add_route(0, m_ic205, 1.0);

	MIXER(config, m_bbd_mixer);
	m_bbd_mixer->add_route(0, m_postbbd_rc, 1.0);

	MN3204P(config, m_bbd);
	m_bbd->add_route(0, m_bbd_mixer, 0.5);
	m_bbd->add_route(1, m_bbd_mixer, 0.5);

	FILTER_BIQUAD(config, m_ic204b);
	m_ic204b->opamp_sk_lowpass_setup(RES_K(22), RES_K(22), RES_M(999.9), RES_R(0.001), CAP_U(0.0068), CAP_P(82));
	m_ic204b->add_route(0, m_bbd, 1.0);

	FILTER_RC(config, m_first_rc);
	m_first_rc->set_rc(filter_rc_device::LOWPASS, RES_R(22), 0, 0, CAP_U(0.0015));
	m_first_rc->add_route(0, m_ic204b, 1.0);

	// not accurate, not correctly modeling the 22k resistor bypassing the 22k resistor in series
	// with the cap, and just assuming two 22k resistors in parallel
	FILTER_BIQUAD(config, m_ic204a);
	m_ic204a->opamp_mfb_lowpass_setup(RES_K(11), 0, RES_K(47), 0, CAP_P(150));
	m_ic204a->add_route(0, m_first_rc, 1.0);
	// end BBD filter chain....

	YM3533(config, m_ym3533, 3.579545_MHz_XTAL);
	m_ym3533->irq_handler().set(FUNC(psr60_state::ym_irq_w));
	m_ym3533->add_route(0, m_lmixer, 0.16);     // channel 1 = ORC
	m_ym3533->add_route(0, m_rmixer, 0.16);
	m_ym3533->add_route(1, m_lmixer, 0.22);     // channel 2 = SABC
	m_ym3533->add_route(1, m_rmixer, 0.22);
	m_ym3533->add_route(1, m_ic204a, 1.0);      // routed to BBD via filters

	YM2154(config, m_ryp4, 2.25_MHz_XTAL);
	m_ryp4->irq_handler().set(FUNC(psr60_state::ryp4_irq_w));
	m_ryp4->io_read_handler().set(FUNC(psr60_state::ryp4_an_r));
	m_ryp4->io_write_handler().set(FUNC(psr60_state::ryp4_out_w));
	m_ryp4->add_route(0, m_lmixer, 0.50);
	m_ryp4->add_route(1, m_rmixer, 0.50);
}

void psr60_state::psr60(machine_config &config)
{
	psr_common(config);
	config.set_default_layout(layout_psr60);
}

void psr60_state::psr70(machine_config &config)
{
	psr_common(config);
	config.set_default_layout(layout_psr70);
}

ROM_START( psr60 )
	ROM_REGION(0x8000, "rom1", 0)
	ROM_LOAD("yamaha_psr60_pgm_ic109.bin", 0x000000, 0x008000, CRC(95604916) SHA1(811fb88fc968c58234600eb9fbf1f64e411754cd))

	ROM_REGION(0x8000, "rom2", 0)
	ROM_LOAD("yamaha_psr60_pgm_ic110.bin", 0x000000, 0x008000, CRC(39db8c74) SHA1(7750104d1e5df3357aa553ac58768dbc34051cd8))

	ROM_REGION(0x8000, "ryp4:group0", 0)
	ROM_LOAD("ym21908.bin", 0x0000, 0x8000, CRC(5d0e1d9f) SHA1(dec735ed3df6dc0d81d532186b1073d4ea6290e2))

	ROM_REGION(0x8000, "ryp4:group1", 0)
	ROM_LOAD("ym21909.bin", 0x0000, 0x8000, CRC(a555b42a) SHA1(f96cdea88ffc0af4cf6009529398a0181a91a70c))
ROM_END

ROM_START(psr70)
	ROM_REGION(0x8000, "rom1", 0)
	ROM_LOAD("psr70-rom1.bin", 0x000000, 0x008000, CRC(bf134412) SHA1(318f33f8ef5e2d865e8ae657993763c9e032af8e))

	ROM_REGION(0x8000, "rom2", 0)
	ROM_LOAD("yamaha_psr60_pgm_ic110.bin", 0x000000, 0x008000, CRC(39db8c74) SHA1(7750104d1e5df3357aa553ac58768dbc34051cd8))

	ROM_REGION(0x8000, "ryp4:group0", 0)
	ROM_LOAD("ym21908.bin", 0x0000, 0x8000, CRC(5d0e1d9f) SHA1(dec735ed3df6dc0d81d532186b1073d4ea6290e2))

	ROM_REGION(0x8000, "ryp4:group1", 0)
	ROM_LOAD("ym21909.bin", 0x0000, 0x8000, CRC(a555b42a) SHA1(f96cdea88ffc0af4cf6009529398a0181a91a70c))
ROM_END

} // namespace

CONS(1985, psr60, 0,     0, psr60, psr60, psr60_state, empty_init, "Yamaha", "PSR-60 PortaSound", MACHINE_IMPERFECT_SOUND)
CONS(1985, psr70, psr60, 0, psr70, psr70, psr60_state, empty_init, "Yamaha", "PSR-70 PortaSound", MACHINE_IMPERFECT_SOUND)
