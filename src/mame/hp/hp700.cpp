// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for HP-700 series terminals.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"


namespace {

class hp700_state : public driver_device
{
public:
	hp700_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_duart(*this, "duart")
	{ }

	void hp700_92(machine_config &config);
	void hp700_70(machine_config &config);

private:
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void hp700_70_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<scn2681_device> m_duart;
};

void hp700_state::mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom().region("maincpu", 0);
	map(0x20000, 0x2ffff).ram();
	map(0x30000, 0x31fff).ram().share("nvram");
	map(0x34000, 0x34000).nopr();
	map(0x38000, 0x38fff).lrw8(
							   NAME([this] (offs_t offset) {
								   return m_duart->read(offset >> 8);
							   }),
							   NAME([this] (offs_t offset, u8 data) {
								   m_duart->write(offset >> 8, data);
							   }));
	map(0xffff0, 0xfffff).rom().region("maincpu", 0x1fff0);
}

void hp700_state::hp700_70_map(address_map &map)
{
	map(0x00000, 0x3ffff).rom().region("maincpu", 0);
	map(0x60000, 0x6ffff).ram();
	map(0x70000, 0x71fff).ram().share("nvram");
	map(0x780f0, 0x780ff).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0xffff0, 0xfffff).rom().region("maincpu", 0x1fff0);
}

void hp700_state::io_map(address_map &map)
{
	map(0x00f2, 0x00f2).nopw();
}

static INPUT_PORTS_START(hp700_92)
INPUT_PORTS_END

void hp700_state::hp700_92(machine_config &config)
{
	V20(config, m_maincpu, 29.4912_MHz_XTAL / 3); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &hp700_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &hp700_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCN2681(config, m_duart, 29.4912_MHz_XTAL / 8); // divider not verified
	m_duart->irq_cb().set_inputline(m_maincpu, INPUT_LINE_NMI);
}

void hp700_state::hp700_70(machine_config &config)
{
	V20(config, m_maincpu, 39.3216_MHz_XTAL / 4); // divider not verified (XTAL value not readable, assumed to be same as HP 700/60)
	m_maincpu->set_addrmap(AS_PROGRAM, &hp700_state::hp700_70_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCN2681(config, m_duart, 39.3216_MHz_XTAL / 8); // divider not verified; device type probably wrong (48-pin DIP labeled 1LVS-0001)
	m_duart->irq_cb().set_inputline(m_maincpu, INPUT_LINE_NMI);
}


/**************************************************************************************************************

Hewlett-Packard HP-700/92.
Chips: TC5564APL-15, proprietary square chip, D70108C-10 (V20), SCN2681, Beeper
Crystals: 29.4912

***************************************************************************************************************/

ROM_START(hp700_92)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("5181-8672.u803", 0x00000, 0x20000, CRC(21440d2f) SHA1(69a3de064ae2b18adc46c2fdd0bf69620375efe7))
ROM_END

/**************************************************************************************************************

HP 700/70 Terminal with AlphaWindows capability

Board D-3416 C1093-60001
-------------
2 EPROMs
EPROM Type: M27C1001 (128KB - 1 MBit)
-------------

-------------
1LV5-0001
93491
SINGAPORE
-------------

-------------
NEC
9401Y5 V20
D70108C-10(C)'84 NEC
-------------

-------------
HP ASIC 1MH1-0202
94281 Singapore
HJP02
-------------

***************************************************************************************************************/

ROM_START(hp700_70)
	ROM_REGION(0x40000, "maincpu", 0) // "© HP 1994 REV: 3440"
	ROM_LOAD("c1093-80008.u802", 0x00000, 0x20000, CRC(25c527a6) SHA1(97e82774d25eab6fd4cc6ff7a5a473341281abb1)) // "CKSM 96A5"
	ROM_LOAD("c1093-80009.u817", 0x20000, 0x20000, CRC(369e6855) SHA1(938ac9cd120d0aa7c76011d1a5e91244a142b397)) // "CKSM 7B6B"
ROM_END

} // anonymous namespace


COMP(1987, hp700_92, 0, 0, hp700_92, hp700_92, hp700_state, empty_init, "Hewlett-Packard", "HP 700/92 Display Terminal", MACHINE_IS_SKELETON)
COMP(1994, hp700_70, 0, 0, hp700_70, hp700_92, hp700_state, empty_init, "Hewlett-Packard", "HP 700/70 Windowing Terminal", MACHINE_IS_SKELETON)
