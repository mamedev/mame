// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Elka Synthex synthesizer.

****************************************************************************/

#include "emu.h"
#include "bus/midi/midi.h"
#include "cpu/m6502/m6502.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"


namespace {

class synthex_state : public driver_device
{
public:
	synthex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_midiacia(*this, "midiacia")
		, m_dac_data_latched(0)
		, m_dac_input_select(0)
		, m_sh_latch(0)
	{
	}

	void synthex(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u8 ram2_r(offs_t offset);
	void ram2_w(offs_t offset, u8 data);
	u8 ram3_r(offs_t offset);
	void ram3_w(offs_t offset, u8 data);
	void c1_w(u8 data);
	void c2_w(u8 data);
	void sh_w(u8 data);
	u8 kom_r();

	u8 cs1_komp_r();
	u8 cs2_komp_r();
	u8 cs3_komp_r();
	u8 cs4_komp_r();

	u8 seq_exts3_r();
	u8 seq_extkyb_r(offs_t offset);

	virtual void driver_start() override;

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<acia6850_device> m_midiacia;

	std::unique_ptr<u8[]> m_ram2_ptr;
	std::unique_ptr<u8[]> m_ram3_ptr;

	u16 m_dac_data_latched;
	u8 m_dac_input_select;
	u8 m_sh_latch;
};

void synthex_state::machine_start()
{
	m_ram2_ptr = make_unique_clear<u8[]>(0x400);
	m_ram3_ptr = make_unique_clear<u8[]>(0x400);
	subdevice<nvram_device>("ram2")->set_base(&m_ram2_ptr[0], 0x400);
	subdevice<nvram_device>("ram3")->set_base(&m_ram3_ptr[0], 0x400);

	m_midiacia->write_dcd(0);
	m_midiacia->write_cts(0);

	save_pointer(NAME(m_ram2_ptr), 0x400);
	save_pointer(NAME(m_ram3_ptr), 0x400);
	save_item(NAME(m_dac_data_latched));
	save_item(NAME(m_dac_input_select));
	save_item(NAME(m_sh_latch));
}

u8 synthex_state::ram2_r(offs_t offset)
{
	return bitswap<8>(m_ram2_ptr[bitswap<10>(offset, 6, 1, 3, 7, 2, 4, 5, 9, 8, 0)], 5, 6, 4, 3, 7, 2, 1, 0);
}

void synthex_state::ram2_w(offs_t offset, u8 data)
{
	m_ram2_ptr[bitswap<10>(offset, 6, 1, 3, 7, 2, 4, 5, 9, 8, 0)] = bitswap<8>(data, 3, 6, 7, 5, 4, 2, 1, 0);
}

u8 synthex_state::ram3_r(offs_t offset)
{
	return bitswap<8>(m_ram3_ptr[bitswap<10>(offset, 6, 1, 3, 7, 2, 4, 5, 9, 8, 0)], 5, 6, 4, 3, 7, 2, 1, 0);
}

void synthex_state::ram3_w(offs_t offset, u8 data)
{
	m_ram3_ptr[bitswap<10>(offset, 6, 1, 3, 7, 2, 4, 5, 9, 8, 0)] = bitswap<8>(data, 3, 6, 7, 5, 4, 2, 1, 0);
}

void synthex_state::c1_w(u8 data)
{
	// Schematic incorrectly crosses D4 and D5 between LS174 and DAC
	m_dac_data_latched = (m_dac_data_latched & 0x300) | (data & 0x3f) << 2;
}

void synthex_state::c2_w(u8 data)
{
	m_dac_data_latched = u16(data & 0xc0) << 2 | (m_dac_data_latched & 0x0fc);
	m_dac_input_select = data & 0x07;
}

void synthex_state::sh_w(u8 data)
{
	m_sh_latch = (data & 0xfc) >> 2;
}

u8 synthex_state::kom_r()
{
	return 0xff;
}

u8 synthex_state::cs1_komp_r()
{
	return 0xff;
}

u8 synthex_state::cs2_komp_r()
{
	return 0xff;
}

u8 synthex_state::cs3_komp_r()
{
	return 0xff;
}

u8 synthex_state::cs4_komp_r()
{
	return 0xff;
}

u8 synthex_state::seq_exts3_r()
{
	// N.B. bit 5 = IRQ status
	return 0xff;
}

u8 synthex_state::seq_extkyb_r(offs_t offset)
{
	return 0xff;
}

void synthex_state::mem_map(address_map &map)
{
	// The logical schematic contains some errors.
	// On the LS42 at 3C (which decodes A14 and A15), pins 5 and 9 are both labeled "XF2", which 2E uses to decode the ROM area.
	// On the LS138 at 2F (which decodes A10–A13), pin 5 is labeled "F1", the 0x1700–0x177f decode produced on pin 8 of 4F.
	// The wiring diagram shows that pin 5 of the former IC is in fact connected to pin 5 of the latter.
	map(0x0000, 0x03ff).ram();
	map(0x0400, 0x07ff).rw(FUNC(synthex_state::ram2_r), FUNC(synthex_state::ram2_w));
	map(0x0800, 0x0bff).rw(FUNC(synthex_state::ram3_r), FUNC(synthex_state::ram3_w));
	map(0x0e00, 0x0e01).mirror(0x1e0).w(m_midiacia, FUNC(acia6850_device::write));
	map(0x0e02, 0x0e03).mirror(0x1e0).r(m_midiacia, FUNC(acia6850_device::read));
	map(0x0e04, 0x0e04).mirror(0x1e3).r(FUNC(synthex_state::seq_exts3_r));
	map(0x0e18, 0x0e1f).mirror(0x1e0).r(FUNC(synthex_state::seq_extkyb_r));
	map(0x1260, 0x1260).mirror(0x199).r(FUNC(synthex_state::cs1_komp_r));
	map(0x1262, 0x1262).mirror(0x199).r(FUNC(synthex_state::cs2_komp_r));
	map(0x1264, 0x1264).mirror(0x199).r(FUNC(synthex_state::cs3_komp_r));
	map(0x1266, 0x1266).mirror(0x199).r(FUNC(synthex_state::cs4_komp_r));
	map(0x1460, 0x1460).mirror(0x1f).w(FUNC(synthex_state::c2_w));
	map(0x14e0, 0x14e0).mirror(0x1f).w(FUNC(synthex_state::c1_w));
	map(0x1660, 0x1660).mirror(0x1f).w(FUNC(synthex_state::sh_w));
	map(0x17e0, 0x17e0).mirror(0x1f).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x1ae0, 0x1ae0).mirror(0x1f).r(FUNC(synthex_state::kom_r));
	// Four 4K ROM areas are decoded (in descending order), but the PCB has no space for a ROM4
	map(0xd000, 0xffff).rom().region("program", 0);
}

static INPUT_PORTS_START(synthex)
INPUT_PORTS_END

void synthex_state::synthex(machine_config &config)
{
	M6502(config, m_maincpu, 4_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &synthex_state::mem_map);

	WATCHDOG_TIMER(config, "watchdog"); // timeout based on RC delays

	NVRAM(config, "ram2", nvram_device::DEFAULT_ALL_0); // 2x 5114 + battery
	NVRAM(config, "ram3", nvram_device::DEFAULT_ALL_0); // 2x 5114 + battery

	//AD7520(config, "maindac");

	ACIA6850(config, m_midiacia);
	m_midiacia->txd_handler().set("midiout", FUNC(midi_port_device::write_txd));
	m_midiacia->irq_handler().set_inputline(m_maincpu, m6502_device::NMI_LINE);

	clock_device &midiclock(CLOCK(config, "midiclock", 4_MHz_XTAL / 8));
	midiclock.signal_handler().set(m_midiacia, FUNC(acia6850_device::write_txc));
	midiclock.signal_handler().append(m_midiacia, FUNC(acia6850_device::write_rxc));

	MIDI_PORT(config, "midiin", midiin_slot, nullptr).rxd_handler().set(m_midiacia, FUNC(acia6850_device::write_rxd));
	MIDI_PORT(config, "midiout", midiout_slot, nullptr);
}

void synthex_state::driver_start()
{
	u8 *program = memregion("program")->base();
	std::vector<u8> romdata(&program[0x0000], &program[0x3000]);
	for (offs_t offset = 0x0000; offset < 0x3000; offset++)
	{
		u16 addr_swapped = bitswap<14>(offset, 13, 12, 11, 7, 3, 1, 0, 2, 4, 5, 6, 8, 9, 10);
		u8 data_swapped = bitswap<8>(romdata[addr_swapped], 5, 1, 4, 3, 2, 0, 6, 7);
		program[offset] = data_swapped;
	}
}

ROM_START(synthex)
	ROM_REGION(0x3000, "program", ROMREGION_ERASEFF)
	ROM_LOAD("sx_fm3.1d", 0x2000, 0x1000, CRC(589e5274) SHA1(3aa0d42367449a2eefcdf50a374780c1eb3674ee))
	ROM_LOAD("sx_em3.1f", 0x1000, 0x1000, CRC(f5eb1df5) SHA1(eedcc709a61c4ba7fbea4805e0f5b86828d81825))
	ROM_LOAD("sx_t41.1h", 0x0000, 0x0800, CRC(520459b9) SHA1(2c2bd1399e99bdc7f81de4e642de19b19547a6f6))
	// ROM4 decode is not used
ROM_END

} // anonymous namespace


SYST(1981, synthex, 0, 0, synthex, synthex, synthex_state, empty_init, "Elka", "Synthex", MACHINE_IS_SKELETON)
