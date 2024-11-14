// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Yamaha PSR-16 & related keyboards.

    These are based on the YM3420AD or YM3420BF "CPU and FM Tone Generator",
    a 65C02-based SoC which includes 256 bytes of internal RAM, 16 KB of
    internal ROM (enabled only on PSR-36?), a MIDI UART and a DAC. It is
    paired with the YM3419AD (XE193A00) or YM3419BF (XF026A00) RYP7 rhythm
    generator, which has its own external ROM.

    pss-680/780 are same machine, 780 is just different main rom and cosmetic changes.
    Mainboard: XE421 TCMK-19EHB
    IC1  YM3420B       QFP100   OPU
    IC2  YM3419B       QFP64    Drum "rompler"
    IC3  XE405B0-070   DIP32    ROM (rompler)
    IC4  XG503B0-132   DIP32    ROM (main program)
    IC6  TC5565APL-12  DIP28    8KB SRAM
    IC7  XE415AO       QFP100   Custom ?

*******************************************************************************/

#include "emu.h"
//#include "bus/midi/midi.h"
#include "cpu/m6502/m65c02.h"

namespace {

class yamaha_psr16_state : public driver_device
{
public:
	yamaha_psr16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rombank(*this, "rombank")
	{
	}

	void psr16(machine_config &config);
	void psr36(machine_config &config);
	void pss680(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u8 busy_r();
	void bank_w(u8 data);

	void common_map(address_map &map) ATTR_COLD;
	void psr16_map(address_map &map) ATTR_COLD;
	void psr36_map(address_map &map) ATTR_COLD;
	void pss680_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	optional_memory_bank m_rombank;
};

void yamaha_psr16_state::machine_start()
{
	if (m_rombank.found())
	{
		m_rombank->configure_entries(0, 0x10, memregion("program")->base(), 0x4000);
		m_rombank->set_entry(0);
	}
}


u8 yamaha_psr16_state::busy_r()
{
	// Code waits for bit 7 of $0200 to go low before writing to $02XX
	return 0;
}

void yamaha_psr16_state::bank_w(u8 data)
{
	m_rombank->set_entry(data & 0x0f);
}

void yamaha_psr16_state::common_map(address_map &map)
{
	map(0x0000, 0x00ff).mirror(0x100).ram();
	map(0x0200, 0x0200).r(FUNC(yamaha_psr16_state::busy_r));
	map(0x0310, 0x0312).nopw();
	map(0x0318, 0x0318).nopr();
	map(0x0319, 0x031a).nopw();
	map(0x031b, 0x031b).nopr();
	map(0x0327, 0x0327).nopw();
}

void yamaha_psr16_state::psr16_map(address_map &map)
{
	common_map(map);
	map(0xc000, 0xffff).rom().region("program", 0);
}

void yamaha_psr16_state::psr36_map(address_map &map)
{
	common_map(map);
	map(0x2000, 0x27ff).ram();
	map(0x4000, 0x7fff).rom().region("program", 0x4000);
	map(0x8000, 0xbfff).rom().region("program", 0);
	map(0xc000, 0xffff).rom().region("maincpu", 0);
}

void yamaha_psr16_state::pss680_map(address_map &map)
{
	common_map(map);
	map(0x1800, 0x1800).w(FUNC(yamaha_psr16_state::bank_w));
	map(0x1801, 0x1802).nopw();
	map(0x1810, 0x1811).nopr();
	map(0x2000, 0x3fff).ram();
	map(0x4000, 0x7fff).bankr("rombank");
	map(0x8000, 0xffff).rom().region("program", 0);
}



static INPUT_PORTS_START(psr16)
INPUT_PORTS_END

void yamaha_psr16_state::psr16(machine_config &config)
{
	M65C02(config, m_maincpu, 5.5_MHz_XTAL / 4); // XTAL value from PSS-480; internal divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &yamaha_psr16_state::psr16_map);
}

void yamaha_psr16_state::psr36(machine_config &config)
{
	psr16(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &yamaha_psr16_state::psr36_map);
}

void yamaha_psr16_state::pss680(machine_config &config)
{
	psr16(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &yamaha_psr16_state::pss680_map);
}

ROM_START(psr16)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("yamaha_xe988a0_7247-h001.bin", 0x0000, 0x4000, CRC(20c5c0b2) SHA1(d7a066b680b7d4cfded62d4d50808c21784774d0)) // 28-pin mask ROM
ROM_END

ROM_START(psr36)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("ym3420bf_internal.bin", 0x0000, 0x4000, NO_DUMP)
	ROM_FILL(0x3ffa, 1, 0x07)
	ROM_FILL(0x3ffb, 1, 0x40)
	ROM_FILL(0x3ffc, 1, 0x00)
	ROM_FILL(0x3ffd, 1, 0x40)
	ROM_FILL(0x3ffe, 1, 0x04)
	ROM_FILL(0x3fff, 1, 0x40)

	ROM_REGION(0x20000, "program", 0)
	ROM_LOAD("yamaha_xe137c0_7247-h005.bin", 0x00000, 0x20000, CRC(b9566cd5) SHA1(e35979299de2442d62e039d45b54bf77e9571f49)) // 28-pin mask ROM (second 32K is copy of first 32K; last half is all zero fill)

	ROM_REGION(0x20000, "waves", 0)
	ROM_LOAD("yamaha_xe137b0_4881-7554.bin", 0x00000, 0x20000, CRC(6fe1382e) SHA1(82320d0a6c14825d790c0d67263786f767b7c318)) // 28-pin mask ROM (drum samples)
ROM_END

ROM_START(pss480)
	ROM_REGION(0x40000, "program", 0)
	ROM_LOAD("xe418c0-077.ic4", 0x00000, 0x40000, CRC(1e9a1933) SHA1(c0015cbb34f50ac5687db9def679117e4b2bad32)) // uPD23C2001C (DIP32 5V 2Mbit)

	ROM_REGION(0x20000, "waves", 0)
	ROM_LOAD("xe402a0-611.ic3", 0x00000, 0x20000, CRC(e0f6f612) SHA1(1307a69233da31d19750c33dfb46a216c0be9c08)) // uPD23C1000C (DIP28 5V 1Mbit) (location has holes for a 32-pin ROM)
ROM_END

ROM_START(pss680)
	ROM_REGION(0x40000, "program", 0)
	ROM_LOAD("xe416d0-093.ic4", 0x00000, 0x40000, CRC(150d1392) SHA1(d578324cfae73cd5f6c628eb3044be07684bc5d0)) // 32-pin mask ROM

	ROM_REGION(0x40000, "waves", 0)
	ROM_LOAD("xe405b0-070.ic3", 0x00000, 0x40000, CRC(53336c52) SHA1(6bcad44fc93cfa5cd603cf24adfd736a911d3509)) // 32-pin mask ROM
ROM_END

ROM_START(pss780)
	ROM_REGION(0x40000, "program", 0)
	ROM_LOAD("xg503b0-132.ic4", 0x00000, 0x40000, CRC(0c055206) SHA1(33cc3f4ab27cf6e5068627625f6ab236209ad776)) // 32-pin mask ROM

	ROM_REGION(0x40000, "waves", 0)
	ROM_LOAD("xe405b0-070.ic3", 0x00000, 0x40000, CRC(53336c52) SHA1(6bcad44fc93cfa5cd603cf24adfd736a911d3509)) // 32-pin mask ROM
ROM_END

} // anonymous namespace

SYST(1988, psr16,  0,      0, psr16,  psr16, yamaha_psr16_state, empty_init, "Yamaha", "PSR-16",  MACHINE_IS_SKELETON)
SYST(1988, psr36,  0,      0, psr36,  psr16, yamaha_psr16_state, empty_init, "Yamaha", "PSR-36",  MACHINE_IS_SKELETON)
SYST(1988, pss480, 0,      0, pss680, psr16, yamaha_psr16_state, empty_init, "Yamaha", "PSS-480 / PSS-580", MACHINE_IS_SKELETON)
SYST(1988, pss680, 0,      0, pss680, psr16, yamaha_psr16_state, empty_init, "Yamaha", "PSS-680", MACHINE_IS_SKELETON)
SYST(1989, pss780, pss680, 0, pss680, psr16, yamaha_psr16_state, empty_init, "Yamaha", "PSS-780", MACHINE_IS_SKELETON)
