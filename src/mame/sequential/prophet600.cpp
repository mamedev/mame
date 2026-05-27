// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    prophet600.cpp - Sequential Circuits Prophet-600, designed by Dave Smith

    This was the first commercial synthesizer with built-in MIDI.

    Skeleton driver by R. Belmont

    Hardware:
        CPU: Z80 CPU + 8253 PIT + 6850 UART

    Memory map:
    0x0000-0x1fff: ROM
    0x2000-0x27ff: RAM 1
    0x3000-0x37ff: RAM 2
    0x4000-0x4001: DAC for CV/gate drive
    0x6000-0x6001: 6850 writes
    0xe000-0xe001: 6850 reads

    I/O map:
    00-07:  8253 PIT
    08: set row to scan for keyboard/panel input and LED / lamp output
    09: read: analog comparitor, some value vs. the DAC.  write: output for LEDs/lamps
        comparitor bits:
        bit 1: FFFStatus
        bit 2: output of 8253 channel 2
        bit 3: ADCCompare: returns sign of if current value is > or < the DAC value

    0a: read: keyboard/panel input scan bits, write: select a pot on the panel for the comparitor
    0b: write: update all gate signals
    0c: ???
    0d: write: select which CV signal will be updated with the current DAC value
    0e: write: mask, masks
        bit 0: FFFP -FF P
        bit 1: mask gate from 8253
        bit 3: FFFD FF D
        bit 4: FFFCL -FF CL


    Info:
    - Prophet 600 Technical Manual

    - https://github.com/gligli/p600fw - GPLv3 licensed complete replacement firmware
    for the Prophet 600, but written in C and runs on an AVR that replaces the original
    Z80.

***************************************************************************/

#include "emu.h"

#include "bus/midi/midi.h"
#include "cpu/z80/z80.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/pit8253.h"
#include "video/pwm.h"

#include "prophet600.lh"


namespace {

enum
{
	CV_CV_Osc1A = 0, CV_Osc2A, CV_Osc3A, CV_Osc4A, CV_Osc5A, CV_Osc6A,
	CV_Osc1B, CV_Osc2B, CV_Osc3B, CV_Osc4B, CV_Osc5B, CV_Osc6B,
	CV_Flt1, CV_Flt2, CV_Flt3, CV_Flt4, CV_Flt5, CV_Flt6,
	CV_Amp1, CV_Amp2, CV_Amp3, CV_Amp4, CV_Amp5, CV_Amp6,
	CV_PModOscB, CV_VolA, CV_VolB, CV_MasterVol, CV_APW, CV_ExtFilter, CV_Resonance, CV_BPW,

	CV_MAX
};

class prophet600_state : public driver_device
{
public:
	prophet600_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_acia(*this, "uart"),
		m_display(*this, "display")
	{ }

	void prophet600(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void pit_ch0_tick_w(int state);
	void pit_ch2_tick_w(int state);
	void acia_irq_w(int state);
	void acia_clock_w(int state);

	void dac_w(offs_t offset, uint8_t data);
	void scanrow_w(uint8_t data);
	void led_w(uint8_t data);
	uint8_t scan_r();
	void potmux_w(uint8_t data);
	uint8_t comparitor_r();
	void mask_w(uint8_t data);
	void cv_w(uint8_t data);
	void gate_w(uint8_t data);

	void cpu_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<acia6850_device> m_acia;
	required_device<pwm_display_device> m_display;

	uint16_t m_dac = 0;
	uint8_t m_scanrow = 0;
	uint8_t m_comparitor = 0;

	bool m_nmi_gate = false;

	// gates
	bool m_ASaw = false;
	bool m_ATri = false;
	bool m_Sync = false;
	bool m_BSaw = false;
	bool m_BTri = false;
	bool m_PModFA = false;
	bool m_PModFil = false;

	// control voltages
	uint16_t m_CVs[CV_MAX]{};
};

void prophet600_state::pit_ch0_tick_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state);
}

void prophet600_state::pit_ch2_tick_w(int state)
{
	m_comparitor &= ~0x04;
	m_comparitor |= (state == ASSERT_LINE) ? 0x04 : 0x00;
}

void prophet600_state::acia_irq_w(int state)
{
	if (!m_nmi_gate)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, state);
	}
}

void prophet600_state::acia_clock_w(int state)
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}

void prophet600_state::dac_w(offs_t offset, uint8_t data)
{
	if (offset)
	{
		m_dac &= 0xff;
		m_dac |= (data<<8);
		return;
	}

	m_dac &= 0xff00;
	m_dac |= data;
}

void prophet600_state::scanrow_w(uint8_t data)
{
	// selects LEDs and keyboard keys to check
	m_scanrow = data;

	// scanrow 0x10 = LEDs, 0x20 = 7-segment #1, 0x40 = 7-segment #2
	m_display->write_my(data >> 4 & 7);
}

void prophet600_state::led_w(uint8_t data)
{
	// LEDs in row 0x10 are Seq1=0, Seq2=1, ArpUD=2, ArpAssign=3, Preset=4, Record=5, ToTape=6, FromTape=7
	m_display->write_mx(data);
}

uint8_t prophet600_state::scan_r()
{
	return 0;
}

void prophet600_state::mask_w(uint8_t data)
{
	m_nmi_gate = (data & 0x02) ? true : false;
	if (m_nmi_gate) // gate is set, comparitor line is pulled up to Vcc
	{
		m_comparitor |= 0x04;
	}
	else
	{
		m_comparitor &= ~0x04;
	}

	//printf("8253 gate = %x\n", data & 0x02);
}

void prophet600_state::cv_w(uint8_t data)
{
	int cvnum = data & 7;

	if (!(data & 0x08)) m_CVs[cvnum] = m_dac;
	if (!(data & 0x10)) m_CVs[cvnum+8] = m_dac;
	if (!(data & 0x20)) m_CVs[cvnum+16] = m_dac;
	if (!(data & 0x40)) m_CVs[cvnum+24] = m_dac;
}

void prophet600_state::gate_w(uint8_t data)
{
	m_ASaw = (data & 0x01);
	m_ATri = (data & 0x02);
	m_Sync = (data & 0x04);
	m_BSaw = (data & 0x08);
	m_BTri = (data & 0x10);
	m_PModFA = (data & 0x20);
	m_PModFil = (data & 0x40);
}

/* Pots: Mixer=0,Cutoff=1,Resonance=2,FilEnvAmt=3,FilRel=4,FilSus=5,
    FilDec=6,FilAtt=7,AmpRel=8,AmpSus=9,AmpDec=10,AmpAtt=11,
    Glide=12,BPW=13,MVol=14,MTune=15,PitchWheel=16,ModWheel=22,
    Speed,APW,PModFilEnv,LFOFreq,PModOscB,LFOAmt,FreqB,FreqA,FreqBFine
*/
void prophet600_state::potmux_w(uint8_t data)
{
}

uint8_t prophet600_state::comparitor_r()
{
	//m_comparitor ^= 0x04;
	return m_comparitor;
}

void prophet600_state::cpu_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0x2000, 0x27ff).ram();
	map(0x3000, 0x37ff).ram();
	map(0x4000, 0x4001).w(FUNC(prophet600_state::dac_w));
	map(0x6000, 0x6001).w(m_acia, FUNC(acia6850_device::write));
	map(0xe000, 0xe001).r(m_acia, FUNC(acia6850_device::read));
}

void prophet600_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x07).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x08, 0x08).w(FUNC(prophet600_state::scanrow_w));
	map(0x09, 0x09).rw(FUNC(prophet600_state::comparitor_r), FUNC(prophet600_state::led_w));
	map(0x0a, 0x0a).rw(FUNC(prophet600_state::scan_r), FUNC(prophet600_state::potmux_w));
	map(0x0b, 0x0b).w(FUNC(prophet600_state::gate_w));
	map(0x0d, 0x0d).w(FUNC(prophet600_state::cv_w));
	map(0x0e, 0x0e).w(FUNC(prophet600_state::mask_w));
}

void prophet600_state::machine_start()
{
	// TODO: savestate
}

// master crystal is 8 MHz, all clocks derived from there
void prophet600_state::prophet600(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(8'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &prophet600_state::cpu_map);
	m_maincpu->set_addrmap(AS_IO, &prophet600_state::io_map);

	pit8253_device &pit8253(PIT8253(config, "pit", XTAL(8'000'000)/4));
	pit8253.set_clk<0>(XTAL(8'000'000)/4);
	pit8253.set_clk<1>(XTAL(8'000'000)/4);
	pit8253.set_clk<2>(XTAL(8'000'000)/4);
	pit8253.out_handler<0>().set(FUNC(prophet600_state::pit_ch0_tick_w));
	pit8253.out_handler<2>().set(FUNC(prophet600_state::pit_ch2_tick_w));

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_acia->irq_handler().set(FUNC(prophet600_state::acia_irq_w));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	clock_device &acia_clock(CLOCK(config, "acia_clock", XTAL(8'000'000)/16)); // 500kHz = 16 times the MIDI rate
	acia_clock.signal_handler().set(FUNC(prophet600_state::acia_clock_w));

	PWM_DISPLAY(config, m_display).set_size(3, 8);
	m_display->set_segmask(0x6, 0xff);

	config.set_default_layout(layout_prophet600);
}

static INPUT_PORTS_START( prophet600 )
INPUT_PORTS_END

ROM_START( prpht600 )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "six-0-8.10", 0x0000, 0x2000, CRC(78e3f048) SHA1(61548b6de3d9b5c0ae76f8e751ece0b57de17118) )
ROM_END

ROM_START( prpht600a )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "1982_sci_six-0-2-1.10", 0x0000, 0x2000, CRC(ea89c882) SHA1(a0a21b365ac69c10005daa0802d07618a8b6bd19) )
ROM_END

} // anonymous namespace


CONS( 1983, prpht600,  0,        0, prophet600, prophet600, prophet600_state, empty_init, "Sequential Circuits", "Prophet-600 (set 1)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
CONS( 1983, prpht600a, prpht600, 0, prophet600, prophet600, prophet600_state, empty_init, "Sequential Circuits", "Prophet-600 (set 2)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
