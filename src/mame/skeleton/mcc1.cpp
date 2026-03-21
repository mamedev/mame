// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Preliminary driver for Dynacord MCC-1 MIDI computer.

****************************************************************************/

#include "emu.h"
#include "bus/midi/midi.h"
#include "cpu/m6805/m68705.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/nvram.h"

namespace {

class mcc1_state : public driver_device
{
public:
	mcc1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_inputs(*this, "IN%u", 0U)
		, m_digits(*this, "digit%u", 0U)
		, m_pa_out(0)
		, m_pb_out(0)
		, m_shift_reg{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	{
	}

	void mcc1(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	void pa_w(offs_t offset, u8 data, u8 mem_mask);
	u8 pb_r();
	void pb_w(offs_t offset, u8 data, u8 mem_mask);

	required_ioport_array<4> m_inputs;
	output_finder<10> m_digits;

	u8 m_pa_out;
	u8 m_pb_out;
	u8 m_shift_reg[10];
};

void mcc1_state::machine_start()
{
	m_digits.resolve();

	save_item(NAME(m_pa_out));
	save_item(NAME(m_pb_out));
	save_item(NAME(m_shift_reg));
}

void mcc1_state::pa_w(offs_t offset, u8 data, u8 mem_mask)
{
	data |= ~mem_mask;
	if (BIT(data, 1) && !BIT(m_pa_out, 1))
	{
		for (int i = 0; i < 10; i++)
			m_shift_reg[i] = m_shift_reg[i] >> 1 | (i == 9 ? BIT(m_pa_out, 0) : m_shift_reg[i + 1]) << 7;
	}
	if (BIT(data, 2))
	{
		for (int i = 0; i < 10; i++)
			m_digits[i] = m_shift_reg[i];
	}
	m_pa_out = data;
}

u8 mcc1_state::pb_r()
{
	u8 result = 0xff;
	for (int i = 0; i < 4; i++)
		if (!BIT(m_pb_out, i))
			result &= m_inputs[i]->read();
	return result;
}

void mcc1_state::pb_w(offs_t offset, u8 data, u8 mem_mask)
{
	m_pb_out = (data | ~mem_mask) & 0x0f;
}

static INPUT_PORTS_START(mcc1)
	PORT_START("IN0")
	// TODO: identify these four buttons
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Unknown A")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Unknown B")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Unknown C")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Unknown D")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("1")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("2")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("6")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("7")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("3")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("8")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("9")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("0")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void mcc1_state::mem_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram"); // partially shadowed by internal registers
	map(0x0c00, 0x0c01).rw("acia1", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x0d00, 0x0d01).rw("acia2", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x0e00, 0x0e01).rw("acia3", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x0f00, 0x0f01).rw("acia4", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x1000, 0x1fff).rom().region("firmware", 0x1000);
}

void mcc1_state::mcc1(machine_config &config)
{
	m146805e2_device &maincpu(M146805E2(config, "maincpu", 5'000'000)); // unknown clock
	maincpu.set_addrmap(AS_PROGRAM, &mcc1_state::mem_map);
	maincpu.porta_w().set(FUNC(mcc1_state::pa_w));
	maincpu.portb_r().set(FUNC(mcc1_state::pb_r));
	maincpu.portb_w().set(FUNC(mcc1_state::pb_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1); // HM6116LP-3 + "Lithium Batterie 3V"

	acia6850_device &acia1(ACIA6850(config, "acia1"));
	acia1.irq_handler().set_inputline("maincpu", M6805_IRQ_LINE);
	acia1.txd_handler().set("mdout1", FUNC(midi_port_device::write_txd));
	acia1.write_cts(0);

	acia6850_device &acia2(ACIA6850(config, "acia2"));
	acia2.txd_handler().set("mdout2", FUNC(midi_port_device::write_txd));
	acia2.write_cts(0);

	acia6850_device &acia3(ACIA6850(config, "acia3"));
	acia3.txd_handler().set("mdout3", FUNC(midi_port_device::write_txd));
	acia3.write_cts(0);

	acia6850_device &acia4(ACIA6850(config, "acia4"));
	acia4.txd_handler().set("mdout4", FUNC(midi_port_device::write_txd));
	acia4.write_cts(0);

	clock_device &midiclk(CLOCK(config, "midiclk", 500'000));
	midiclk.signal_handler().set("acia1", FUNC(acia6850_device::write_rxc));
	midiclk.signal_handler().append("acia1", FUNC(acia6850_device::write_txc));
	midiclk.signal_handler().append("acia2", FUNC(acia6850_device::write_txc));
	midiclk.signal_handler().append("acia3", FUNC(acia6850_device::write_txc));
	midiclk.signal_handler().append("acia4", FUNC(acia6850_device::write_txc));

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, nullptr));
	mdin.rxd_handler().set("acia1", FUNC(acia6850_device::write_rxd));
	mdin.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));

	MIDI_PORT(config, "mdout1", midiout_slot, nullptr);
	MIDI_PORT(config, "mdout2", midiout_slot, nullptr);
	MIDI_PORT(config, "mdout3", midiout_slot, nullptr);
	MIDI_PORT(config, "mdout4", midiout_slot, nullptr);
	MIDI_PORT(config, "mdthru", midiout_slot, nullptr);
}

ROM_START(mcc1)
	ROM_REGION(0x2000, "firmware", 0)
	ROM_LOAD("mcc1_v1-13.i103", 0x0000, 0x2000, CRC(4fe38281) SHA1(f06931e257e9e5499f04b5a23027e827d5ebf24e)) // TMS 2764JL-45; first half unused
ROM_END

} // anonymous namespace

SYST(1986, mcc1, 0, 0, mcc1, mcc1, mcc1_state, empty_init, "Dynacord", "MCC-1 MIDI Control Computer", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
