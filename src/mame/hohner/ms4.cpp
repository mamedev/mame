// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"
#include "cpu/mcs51/i80c52.h"
#include "bus/midi/midi.h"
#include "sound/sam8905.h"
#include "speaker.h"

#define LOG_SAM    (1U << 1)
//#define LOG_SAM    0
#define VERBOSE (LOG_SAM)
#include "logmacro.h"

namespace {

class ms4_state : public driver_device
{
public:
	ms4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sam(*this, "sam")
		, m_midi_out(*this, "mdout")
	{ }

	void ms4(machine_config &config);

private:
	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

    void port2_w(u8 data);
	u8 port3_r();
	void port3_w(u8 data);

	u8 sam_r(offs_t offset);
	void sam_w(offs_t offset, u8 data);

	u16 sam_waveform_r(offs_t offset);

	void midi_rxd_w(int state) { m_midi_rxd = state; }

	virtual void machine_start() override ATTR_COLD;

	required_device<i80c32_device> m_maincpu;
	required_device<sam8905_device> m_sam;
	required_device<midi_port_device> m_midi_out;

	u8 m_port3 = 0xff;
	u8 m_midi_rxd = 1;
};

void ms4_state::program_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("program", 0);
}

void ms4_state::data_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	// SAM8905 decodes only A0-A2, chip select on A15
	map(0x8000, 0xffff).rw(FUNC(ms4_state::sam_r), FUNC(ms4_state::sam_w));
}

void ms4_state::port2_w(u8 data)
{
    // if (m_port2 != data)
    // {
    //     LOGMASKED(LOG_PORT, "P2 write: 0x%02X\n", data);
    //     // Log P2 changes specifically since it's used for external memory address high byte
    //     LOGMASKED(LOG_SAM, "P2 = 0x%02X (ext addr A8-A15)\n", data);
    // }
    // m_port2 = data;
}

u8 ms4_state::port3_r()
{
	// P3.0 = RXD - MIDI input
	u8 data = (m_port3 & 0xfe) | (m_midi_rxd & 0x01);
	return data;
}

void ms4_state::port3_w(u8 data)
{
	// P3.1 = TXD output to MIDI
	if ((m_port3 ^ data) & 0x02)
		m_midi_out->write_txd(BIT(data, 1));

	m_port3 = data;
}

u8 ms4_state::sam_r(offs_t offset)
{
	u8 reg = offset & 0x07;
    if (offset > 0x07) {
	LOGMASKED(LOG_SAM, "INVALID SAM read [%d] (addr=%04X) PC=%04X\n",
		reg, 0x8000 + offset, m_maincpu->pc());
	}
	u8 data = m_sam->read(reg);
	LOGMASKED(LOG_SAM, "SAM read [%d] = 0x%02X (addr=%04X) PC=%04X\n",
		reg, data, 0x8000 + offset, m_maincpu->pc());
	return data;
}

void ms4_state::sam_w(offs_t offset, u8 data)
{
	u8 reg = offset & 0x07;
    if (offset > 0x07) {
		LOGMASKED(LOG_SAM, "INVALID SAM write [%d] = 0x%02X (addr=%04X) PC=%04X\n",
		reg, data, 0x8000 + offset, m_maincpu->pc());
	}
	m_sam->write(reg, data);
	LOGMASKED(LOG_SAM, "SAM write [%d] = 0x%02X (addr=%04X) PC=%04X\n",
		reg, data, 0x8000 + offset, m_maincpu->pc());
}

u16 ms4_state::sam_waveform_r(offs_t offset)
{
	// No waveform ROM connected yet - return silence
	return 0;
}

void ms4_state::machine_start()
{
	save_item(NAME(m_port3));
	save_item(NAME(m_midi_rxd));
}

static INPUT_PORTS_START(ms4)
INPUT_PORTS_END

void ms4_state::ms4(machine_config &config)
{
	I80C32(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ms4_state::program_map);
	m_maincpu->set_addrmap(AS_DATA, &ms4_state::data_map);
    m_maincpu->port_out_cb<2>().set(FUNC(ms4_state::port2_w)); // keyfox2
	m_maincpu->port_in_cb<3>().set(FUNC(ms4_state::port3_r));
	m_maincpu->port_out_cb<3>().set(FUNC(ms4_state::port3_w));

	// MIDI
	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(FUNC(ms4_state::midi_rxd_w));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	// Sound - single SAM8905
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SAM8905(config, m_sam, 45'158'400, 1024);
	m_sam->waveform_read_callback().set(FUNC(ms4_state::sam_waveform_r));
	m_sam->add_route(0, "lspeaker", 1.0);
	m_sam->add_route(1, "rspeaker", 1.0);
}

ROM_START(ms4)
	ROM_REGION(0x10000, "program", 0)
//	ROM_LOAD("ms4_05_r1_0.bin", 0x0000, 0x10000, CRC(b02cd104) SHA1(1c10d26481c20d0de2df9cf3e2b5112cd40fe3ba)) // SOLO SOUNDS CH1-4
//	ROM_LOAD("ms4_06_r1_1.bin", 0x0000, 0x10000, CRC(b02cd104) SHA1(1c10d26481c20d0de2df9cf3e2b5112cd40fe3ba)) // DRUMS CH16 + ACCOMP CH5-8

//	ROM_LOAD("kf31_ic7_630792l_v1_1.bin", 0x0000, 0x10000, CRC(b02cd104) SHA1(1c10d26481c20d0de2df9cf3e2b5112cd40fe3ba))
//	ROM_LOAD("kf31_ic7_630792l_v1_0_uart.bin", 0x0000, 0x10000, CRC(b02cd104) SHA1(1c10d26481c20d0de2df9cf3e2b5112cd40fe3ba))

//	ROM_LOAD("xe9l_v141_uart.bin", 0x0000, 0x10000, CRC(b02cd104) SHA1(1c10d26481c20d0de2df9cf3e2b5112cd40fe3ba))
	ROM_LOAD("xe9r_v141_uart.bin", 0x0000, 0x10000, CRC(b02cd104) SHA1(1c10d26481c20d0de2df9cf3e2b5112cd40fe3ba))

ROM_END

} // anonymous namespace

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY   FULLNAME      FLAGS
SYST( 1992, ms4,  0,      0,      ms4,     ms4,   ms4_state, empty_init, "Solton", "Solton MS4", MACHINE_IMPERFECT_SOUND )
