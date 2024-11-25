// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Korg DW-8000 and EX-8000 synthesizers.

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6801.h"
//#include "bus/midi/midi.h"
#include "machine/nvram.h"


namespace {

class korgdw8k_state : public driver_device
{
public:
	korgdw8k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rombank(*this, "rombank")
	{
	}

	void dw8000(machine_config &config);
	void dw8000ex(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void dac_w(u8 data);
	void vcfa_w(u8 data);
	void ddl_w(u8 data);
	void led_data_w(u8 data);
	void led_addr_w(u8 data);
	void bank_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void expanded_mem_map(address_map &map) ATTR_COLD;

	required_device<hd6303x_cpu_device> m_maincpu;
	optional_memory_bank m_rombank;
};

void korgdw8k_state::machine_start()
{
	if (m_rombank.found())
		m_rombank->configure_entries(0, 2, memregion("program")->base(), 0x4000);
}

void korgdw8k_state::machine_reset()
{
	if (m_rombank.found())
		bank_w(0);
}

void korgdw8k_state::dac_w(u8 data)
{
}

void korgdw8k_state::vcfa_w(u8 data)
{
}

void korgdw8k_state::ddl_w(u8 data)
{
}

void korgdw8k_state::led_data_w(u8 data)
{
}

void korgdw8k_state::led_addr_w(u8 data)
{
}

void korgdw8k_state::bank_w(u8 data)
{
	//m_rombank->set_entry(BIT(data, 0));
	m_rombank->set_entry(1);
}


void korgdw8k_state::mem_map(address_map &map)
{
	map(0x3000, 0x37ff).ram().share("nvram");
	map(0x3800, 0x3fff).ram();
	map(0x4040, 0x4040).w(FUNC(korgdw8k_state::dac_w));
	map(0x4060, 0x4060).w(FUNC(korgdw8k_state::vcfa_w));
	map(0x4080, 0x4080).w(FUNC(korgdw8k_state::ddl_w));
	map(0x40a0, 0x40a0).w(FUNC(korgdw8k_state::led_data_w));
	map(0x40c0, 0x40c0).w(FUNC(korgdw8k_state::led_addr_w));
	map(0xc000, 0xffff).rom().region("program", 0);
}

void korgdw8k_state::expanded_mem_map(address_map &map)
{
	mem_map(map);
	map(0x8000, 0xbfff).ram();
	map(0xc000, 0xffff).bankr("rombank").w(FUNC(korgdw8k_state::bank_w));
}


static INPUT_PORTS_START(dw8000)
INPUT_PORTS_END

void korgdw8k_state::dw8000(machine_config &config)
{
	HD6303X(config, m_maincpu, 8_MHz_XTAL); // HD63B03XP
	m_maincpu->set_addrmap(AS_PROGRAM, &korgdw8k_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // HM6116LP-4 + battery
}

void korgdw8k_state::dw8000ex(machine_config &config)
{
	dw8000(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &korgdw8k_state::expanded_mem_map);
}

ROM_START(dw8000)
	ROM_REGION(0x4000, "program", 0)
	ROM_SYSTEM_BIOS(0, "v13", "Version 13")
	ROMX_LOAD("dw8000-v0713.ic38", 0x0000, 0x4000, CRC(d8d89329) SHA1(a0e3402528036a2ba5767e707b8d2e4e365dbd4a), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v12", "Version 12")
	ROMX_LOAD("dw-8000-v12.ic38",  0x0000, 0x4000, CRC(f21b3848) SHA1(2e91791e71205049446e75ec53267d54150e1ac7), ROM_BIOS(1))

	ROM_REGION(0x20000, "waveform", 0) // Standard waves
	ROM_LOAD("hn613256p-t70_32004086.ic46", 0x00000, 0x08000, CRC(f24d5e8b) SHA1(bf8eafffd55e70d6a515abbc311667d3bc12157b))
	ROM_LOAD("hn613256p-t71_32004087.ic45", 0x08000, 0x08000, CRC(a8c5a80c) SHA1(c2c4fb748f1fbdea3abe20072105eedebc321ece))
	ROM_LOAD("hn613256p-cb4_32004088.ic44", 0x10000, 0x08000, CRC(2610d6f6) SHA1(6894b8368f64f62bb541b3f88e58d88f7138b3a5))
	ROM_LOAD("hn613256p-cb5_32004089.ic43", 0x18000, 0x08000, CRC(0958145d) SHA1(fd7fd60bde8739a937bd0af11caac76e566b4a00))
ROM_END

ROM_START(dw8000ex)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("korg-dw8000-ex.bin", 0x0000, 0x8000, CRC(beb31c83) SHA1(e9879ccb6c3417c099e0b673852cad6c79026c73))

	ROM_REGION(0x20000, "waveform", 0) // Standard waves
	ROM_LOAD("hn613256p-t70_32004086.ic46", 0x00000, 0x08000, CRC(f24d5e8b) SHA1(bf8eafffd55e70d6a515abbc311667d3bc12157b))
	ROM_LOAD("hn613256p-t71_32004087.ic45", 0x08000, 0x08000, CRC(a8c5a80c) SHA1(c2c4fb748f1fbdea3abe20072105eedebc321ece))
	ROM_LOAD("hn613256p-cb4_32004088.ic44", 0x10000, 0x08000, CRC(2610d6f6) SHA1(6894b8368f64f62bb541b3f88e58d88f7138b3a5))
	ROM_LOAD("hn613256p-cb5_32004089.ic43", 0x18000, 0x08000, CRC(0958145d) SHA1(fd7fd60bde8739a937bd0af11caac76e566b4a00))

	ROM_REGION(0x20000, "exp", 0) // Expansion waves ("Version E") (TBD: do these actually require a different expansion?)
	ROM_LOAD("exp-1.bin", 0x00000, 0x08000, CRC(14a5d504) SHA1(76ba6a715f1126d4cce59f172943c9f710ce981a))
	ROM_LOAD("exp-3.bin", 0x08000, 0x08000, CRC(1d2fd4a0) SHA1(41864bf1180cc226d149bcfd02496882affdad19))
	ROM_LOAD("exp-2.bin", 0x10000, 0x08000, CRC(e4a445ca) SHA1(53513fbf29523567d87ed119ecf1b116da904abc))
	ROM_LOAD("exp-4.bin", 0x18000, 0x08000, CRC(bfd0701f) SHA1(07d3b903df3b980b4d0df0d64315083abdd86b41))
ROM_END

ROM_START(ex8000)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("ex8000_firmware.ic38", 0x0000, 0x4000, CRC(a6d0fcdc) SHA1(a5eed30248c8d51b8d2e00e4b23fdcdf2a2f94af))

	ROM_REGION(0x20000, "waveform", 0)
	ROM_LOAD("hn613256p-t70_32004086.ic46", 0x00000, 0x08000, CRC(f24d5e8b) SHA1(bf8eafffd55e70d6a515abbc311667d3bc12157b))
	ROM_LOAD("hn613256p-t71_32004087.ic45", 0x08000, 0x08000, CRC(a8c5a80c) SHA1(c2c4fb748f1fbdea3abe20072105eedebc321ece))
	ROM_LOAD("hn613256p-cb4_32004088.ic44", 0x10000, 0x08000, CRC(2610d6f6) SHA1(6894b8368f64f62bb541b3f88e58d88f7138b3a5))
	ROM_LOAD("hn613256p-cb5_32004089.ic43", 0x18000, 0x08000, CRC(0958145d) SHA1(fd7fd60bde8739a937bd0af11caac76e566b4a00))
ROM_END

} // anonymous namespace


SYST(1985, dw8000,   0,      0, dw8000,   dw8000, korgdw8k_state, empty_init, "Korg",               "DW-8000 Programmable Digital Waveform Synthesizer", MACHINE_IS_SKELETON)
SYST(1985, dw8000ex, dw8000, 0, dw8000ex, dw8000, korgdw8k_state, empty_init, "Korg / Musitronics", "DW-8000-EX Programmable Digital Waveform Synthesizer", MACHINE_IS_SKELETON)
SYST(1985, ex8000,   dw8000, 0, dw8000,   dw8000, korgdw8k_state, empty_init, "Korg",               "EX-8000 Programmable Polyphonic Synthe Module", MACHINE_IS_SKELETON)
