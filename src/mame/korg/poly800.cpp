// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Korg Poly-800 synthesizer.

****************************************************************************/

#include "emu.h"
#include "bus/midi/midi.h"
#include "cpu/i8085/i8085.h"
#include "machine/6850acia.h"
#include "machine/i8155.h"
#include "machine/nvram.h"
//#include "sound/msm5232.h"


namespace {

class poly800_state : public driver_device
{
public:
	poly800_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void poly800(machine_config &config);
	void poly800mdk(machine_config &config);

protected:
	void common_map(address_map &map) ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
	void mdk_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

protected:
	required_device<i8085a_cpu_device> m_maincpu;
};

class poly800ii_state : public poly800_state
{
public:
	poly800ii_state(const machine_config &mconfig, device_type type, const char *tag)
		: poly800_state(mconfig, type, tag)
	{
	}

	void poly800ii(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;
};

void poly800_state::common_map(address_map &map)
{
	map(0x4000, 0x40ff).mirror(0x1f00).rw("pio", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0xe000, 0xe001).mirror(0x1f7e).w("acia", FUNC(acia6850_device::write));
	map(0xe080, 0xe081).mirror(0x1f7e).r("acia", FUNC(acia6850_device::read));
}

void poly800_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("klm596", 0);
	map(0x2000, 0x27ff).mirror(0x1800).ram().share("nvram");
	common_map(map);
}

void poly800_state::mdk_map(address_map &map)
{
	map(0x0000, 0x2fff).rom().region("klm596", 0);
	map(0x3000, 0x37ff).mirror(0x0800).ram().share("nvram");
	common_map(map);
}

void poly800ii_state::mem_map(address_map &map)
{
	map(0x0000, 0x27ff).rom().region("klm779", 0);
	map(0x2800, 0x37ff).ram().share("nvram");
	common_map(map);
}

void poly800_state::io_map(address_map &map)
{
	map(0x40, 0x47).mirror(0x18).rw("pio", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
}

static INPUT_PORTS_START(poly800)
INPUT_PORTS_END

void poly800_state::poly800(machine_config &config)
{
	I8085A(config, m_maincpu, 5_MHz_XTAL); // MSM80C85ARS
	m_maincpu->set_addrmap(AS_PROGRAM, &poly800_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &poly800_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // HM6116LP-4 + battery

	i8155_device &pio(I8155(config, "pio", 5_MHz_XTAL / 2)); // MSM81C55RS
	pio.out_to_callback().set("acia", FUNC(acia6850_device::write_rxc));
	pio.out_to_callback().append("acia", FUNC(acia6850_device::write_txc));

	acia6850_device &acia(ACIA6850(config, "acia")); // HD63B50P
	acia.txd_handler().set("midi_out", FUNC(midi_port_device::write_txd));
	acia.irq_handler().set_inputline(m_maincpu, I8085_RST65_LINE);

	//MSM5232(config, "msm"); // MSM-5232RS

	MIDI_PORT(config, "midi_in", midiin_slot, "midiin").rxd_handler().set("acia", FUNC(acia6850_device::write_rxd));
	MIDI_PORT(config, "midi_out", midiout_slot, "midiout");
}

void poly800_state::poly800mdk(machine_config &config)
{
	poly800(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &poly800_state::mdk_map);
}

void poly800ii_state::poly800ii(machine_config &config)
{
	poly800(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &poly800ii_state::mem_map);

	// NVRAM is uPD4464C-15 + battery with A12 tied to GND
}

ROM_START(poly800)
	ROM_REGION(0x2002, "klm596", 0)
	ROM_SYSTEM_BIOS(0, "v36", "Version 36")
	ROM_SYSTEM_BIOS(1, "v30", "Version 30")
	ROMX_LOAD("830236.ic22", 0x0000, 0x2000, CRC(16cd416d) SHA1(a1f6f5be4f70b47ea4d1dccd723179c9991f4a92), ROM_BIOS(0))
	ROMX_LOAD("830230.ic22", 0x0000, 0x2002, CRC(1ba81f55) SHA1(94a0d45680d107ab4d1b542e8378a599ded36f06), ROM_BIOS(1))
	// Where did the extra 2 bytes at the end come from?
ROM_END

ROM_START(poly800mdk)
	ROM_REGION(0x4000, "klm596", 0)
	ROM_LOAD("mdk.ic22", 0x0000, 0x4000, CRC(a25ab16a) SHA1(a9294bf90a8fa81dd825bcbf9fcd3b5014045936)) // 27C128
ROM_END

ROM_START(poly800ii)
	ROM_REGION(0x4000, "klm779", 0)
	ROM_LOAD("851005.ic24", 0x0000, 0x4000, CRC(09ae4fc5) SHA1(1e8a418919ef61334fb55aefa8aaebcaefc28ebd)) // 27C128 (last 3/8ths not addressable)
ROM_END

} // anonymous namespace


SYST(1984, poly800,    0, 0, poly800,    poly800, poly800_state,   empty_init, "Korg", "Poly-800 Programmable Polyphonic Synthesizer", MACHINE_IS_SKELETON)
SYST(1984, poly800mdk, 0, 0, poly800mdk, poly800, poly800_state,   empty_init, "Korg", "Poly-800 Programmable Polyphonic Synthesizer (MIDI Dump Kit)", MACHINE_IS_SKELETON)
SYST(1986, poly800ii,  0, 0, poly800ii,  poly800, poly800ii_state, empty_init, "Korg", "Poly-800II Programmable Polyphonic Synthesizer", MACHINE_IS_SKELETON)
