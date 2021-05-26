// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Yamaha PSR-60/PSR-70 PortaSound keyboards
    Preliminary driver by R. Belmont, major thanks to reverse-engineering work by JKN0

    Documentation: https://github.com/JKN0/PSR70-reverse
    More documentation: https://retroandreverse.blogspot.com/2021/01/reversing-psr-70-hardware.html
                        https://retroandreverse.blogspot.com/2021/01/reversing-psr-70-firmware.html
                        https://retroandreverse.blogspot.com/2021/01/digging-into-ymopq.html

    Service manual: https://elektrotanya.com/yamaha_psr-70_sm.pdf/download.html

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
#include "sound/ymopq.h"
#include "bus/midi/midi.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "psr60.lh"

class psr60_state : public driver_device
{
	static constexpr int DRVIF_MAX_TARGETS = 23;

public:
	psr60_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ym3533(*this, "ym3533"),
		m_ppi(*this, "ppi"),
		m_acia(*this, "acia"),
		m_rom2bank(*this, "rom2bank"),
		m_keyboard(*this, "P1_%u", 0),
		m_drvif(*this, "DRVIF_%u", 0),
		m_drvif_out(*this, "DRVIF_%u_DP%u", 0U, 1U)
	{ }

	void psr60(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<z80_device> m_maincpu;
	required_device<ym3533_device> m_ym3533;
	required_device<i8255_device> m_ppi;
	required_device<acia6850_device> m_acia;
	required_memory_bank m_rom2bank;
	required_ioport_array<10> m_keyboard;
	required_ioport_array<DRVIF_MAX_TARGETS> m_drvif;
	output_finder<DRVIF_MAX_TARGETS,4> m_drvif_out;

	void psr60_map(address_map &map);
	void psr60_io_map(address_map &map);

	u8 ppi_pa_r();
	void ppi_pb_w(u8 data);
	void ppi_pc_w(u8 data);
	void recalc_irqs();

	int m_acia_irq, m_ym_irq, m_drvif_irq;
	u16 m_keyboard_select;
	u8 m_drvif_data[2];
	u8 m_drvif_select;

	WRITE_LINE_MEMBER(write_acia_clock) { m_acia->write_txc(state); m_acia->write_rxc(state); }
	WRITE_LINE_MEMBER(acia_irq_w) { m_acia_irq = state; recalc_irqs(); }

	WRITE_LINE_MEMBER(ym_irq_w) { m_ym_irq = state; recalc_irqs(); }

	u8 drvif_r(offs_t offset);
	void drvif_w(offs_t offset, u8 data);

public:
	INPUT_CHANGED_MEMBER(drvif_changed);
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
	// 30-40: IG14330 "DRVIF" (front panel LED driver/multiplexer?)
	// 80-FF: YM2154 "RYP4" (drum sample playback)
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
	else
		printf("DRVIF: %02X = %02X\n", offset, data);
}

INPUT_CHANGED_MEMBER(psr60_state::drvif_changed)
{
	m_drvif_irq = 1;
	m_drvif_data[0] = param;
	m_drvif_data[1] = m_drvif[param]->read();
	recalc_irqs();
}

void psr60_state::recalc_irqs()
{
	int irq_state = m_acia_irq | m_ym_irq | m_drvif_irq; // (|| RYP4 interrupts eventually)
	m_maincpu->set_input_line(0, irq_state);
}

void psr60_state::machine_start()
{
	m_drvif_out.resolve();
	m_rom2bank->configure_entries(0, 2, memregion("rom2")->base(), 0x4000);
	m_rom2bank->set_entry(0);
	m_acia_irq = CLEAR_LINE;
}

void psr60_state::machine_reset()
{
}

#define DRVIF_PORT(num, sw1, sw2, sw3, sw4) \
	PORT_START("DRVIF_" #num) \
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(sw1) PORT_CHANGED_MEMBER(DEVICE_SELF, psr60_state, drvif_changed, num) \
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(sw2) PORT_CHANGED_MEMBER(DEVICE_SELF, psr60_state, drvif_changed, num) \
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(sw3) PORT_CHANGED_MEMBER(DEVICE_SELF, psr60_state, drvif_changed, num) \
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(sw4) PORT_CHANGED_MEMBER(DEVICE_SELF, psr60_state, drvif_changed, num)

static INPUT_PORTS_START(psr60)
	PORT_START("P1_9")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT)    PORT_NAME("Octave 0 C")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)         PORT_NAME("Octave 0 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)         PORT_NAME("Octave 0 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)         PORT_NAME("Octave 0 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)         PORT_NAME("Octave 0 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)         PORT_NAME("Octave 0 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)         PORT_NAME("Octave 0 F#")

	PORT_START("P1_8")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)         PORT_NAME("Octave 0 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)         PORT_NAME("Octave 0 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)         PORT_NAME("Octave 0 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)         PORT_NAME("Octave 0 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)         PORT_NAME("Octave 0 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)         PORT_NAME("Octave 1 C")

	PORT_START("P1_7")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_NAME("Octave 1 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_NAME("Octave 1 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_NAME("Octave 1 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_NAME("Octave 1 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_NAME("Octave 1 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)         PORT_NAME("Octave 1 F#")

	PORT_START("P1_6")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_NAME("Octave 1 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_NAME("Octave 1 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_NAME("Octave 1 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_NAME("Octave 1 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_NAME("Octave 1 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_NAME("Octave 2 C")

	PORT_START("P1_5")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_NAME("Octave 2 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_NAME("Octave 2 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_NAME("Octave 2 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_NAME("Octave 2 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_NAME("Octave 2 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)         PORT_NAME("Octave 2 F#")

	PORT_START("P1_4")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_NAME("Octave 2 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_NAME("Octave 2 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_NAME("Octave 2 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_NAME("Octave 2 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_NAME("Octave 2 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_NAME("Octave 3 C")

	PORT_START("P1_3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_NAME("Octave 3 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_NAME("Octave 3 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_NAME("Octave 3 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_NAME("Octave 3 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_NAME("Octave 3 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)         PORT_NAME("Octave 3 F#")

	PORT_START("P1_2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_NAME("Octave 3 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_NAME("Octave 3 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_NAME("Octave 3 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_NAME("Octave 3 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_NAME("Octave 3 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_NAME("Octave 4 C")

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
INPUT_PORTS_END

static INPUT_PORTS_START(psr70)
	PORT_START("P1_9")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT)    PORT_NAME("Octave 0 C")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)         PORT_NAME("Octave 0 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)         PORT_NAME("Octave 0 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)         PORT_NAME("Octave 0 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)         PORT_NAME("Octave 0 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)         PORT_NAME("Octave 0 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)         PORT_NAME("Octave 0 F#")

	PORT_START("P1_8")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)         PORT_NAME("Octave 0 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)         PORT_NAME("Octave 0 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)         PORT_NAME("Octave 0 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)         PORT_NAME("Octave 0 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)         PORT_NAME("Octave 0 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)         PORT_NAME("Octave 1 C")

	PORT_START("P1_7")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_NAME("Octave 1 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_NAME("Octave 1 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_NAME("Octave 1 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_NAME("Octave 1 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_NAME("Octave 1 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)         PORT_NAME("Octave 1 F#")

	PORT_START("P1_6")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_NAME("Octave 1 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_NAME("Octave 1 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_NAME("Octave 1 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_NAME("Octave 1 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_NAME("Octave 1 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_NAME("Octave 2 C")

	PORT_START("P1_5")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_NAME("Octave 2 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_NAME("Octave 2 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_NAME("Octave 2 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_NAME("Octave 2 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_NAME("Octave 2 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)         PORT_NAME("Octave 2 F#")

	PORT_START("P1_4")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_NAME("Octave 2 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_NAME("Octave 2 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_NAME("Octave 2 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_NAME("Octave 2 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_NAME("Octave 2 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_NAME("Octave 3 C")

	PORT_START("P1_3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_NAME("Octave 3 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_NAME("Octave 3 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_NAME("Octave 3 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_NAME("Octave 3 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_NAME("Octave 3 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)         PORT_NAME("Octave 3 F#")

	PORT_START("P1_2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_NAME("Octave 3 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_NAME("Octave 3 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_NAME("Octave 3 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_NAME("Octave 3 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_NAME("Octave 3 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_NAME("Octave 4 C")

	PORT_START("P1_1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_NAME("Octave 4 C#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_NAME("Octave 4 D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_NAME("Octave 4 D#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_NAME("Octave 4 E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_NAME("Octave 4 F")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)         PORT_NAME("Octave 4 F#")

	PORT_START("P1_0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // cassette input
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_NAME("Octave 4 G")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_NAME("Octave 4 G#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_NAME("Octave 4 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_NAME("Octave 4 A#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_NAME("Octave 4 B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_NAME("Octave 5 C")

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
INPUT_PORTS_END

void psr60_state::psr60(machine_config &config)
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

	config.set_default_layout(layout_psr60);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YM3533(config, m_ym3533, 3.579545_MHz_XTAL);
	m_ym3533->irq_handler().set(FUNC(psr60_state::ym_irq_w));
	m_ym3533->add_route(0, "lspeaker", 1.0);
	m_ym3533->add_route(1, "rspeaker", 1.0);
}

ROM_START( psr60 )
	ROM_REGION(0x8000, "rom1", 0)
	ROM_LOAD("yamaha_psr60_pgm_ic109.bin", 0x000000, 0x008000, CRC(95604916) SHA1(811fb88fc968c58234600eb9fbf1f64e411754cd))

	ROM_REGION(0x8000, "rom2", 0)
	ROM_LOAD("yamaha_psr60_pgm_ic110.bin", 0x000000, 0x008000, CRC(39db8c74) SHA1(7750104d1e5df3357aa553ac58768dbc34051cd8))
ROM_END

ROM_START(psr70)
	ROM_REGION(0x8000, "rom1", 0)
	ROM_LOAD("psr70-rom1.bin", 0x000000, 0x008000, CRC(bf134412) SHA1(318f33f8ef5e2d865e8ae657993763c9e032af8e))

	ROM_REGION(0x8000, "rom2", 0)
	ROM_LOAD("yamaha_psr60_pgm_ic110.bin", 0x000000, 0x008000, CRC(39db8c74) SHA1(7750104d1e5df3357aa553ac58768dbc34051cd8))
ROM_END

CONS(1985, psr60, 0,     0, psr60, psr60, psr60_state, empty_init, "Yamaha", "PSR-60 PortaSound", MACHINE_NOT_WORKING)
CONS(1985, psr70, psr60, 0, psr60, psr70, psr60_state, empty_init, "Yamaha", "PSR-70 PortaSound", MACHINE_NOT_WORKING)
