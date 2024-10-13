// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

/*
** TODO:
** - Not all keypad keys hooked up yet
*/

#include "emu.h"
#include "msx.h"
#include "msx_keyboard.h"
#include "bus/msx/slot/bruc100.h"
#include "bus/msx/slot/ram.h"
#include "bus/msx/slot/ram_mm.h"

#include "msx_nocode.lh"

using namespace msx_keyboard;


namespace {

class bruc100_state : public msx_state
{
public:
	bruc100_state(const machine_config &mconfig, device_type type, const char *tag);

	void bruc100(machine_config &config);
	void bruc100a(machine_config &config);

private:
	required_device<msx_slot_bruc100_device> m_bruc100_firm;

	void io_map(address_map &map) ATTR_COLD;
	void port90_w(u8 data);
};


bruc100_state::bruc100_state(const machine_config &mconfig, device_type type, const char *tag)
	: msx_state(mconfig, type, tag, 10.738635_MHz_XTAL, 3)
	, m_bruc100_firm(*this, "firm")
{
}

void bruc100_state::io_map(address_map &map)
{
	msx1_io_map(map);
	map(0x90, 0x90).w(FUNC(bruc100_state::port90_w));
}

void bruc100_state::port90_w(u8 data)
{
	m_bruc100_firm->select_bank(BIT(data, 7));
	m_cent_ctrl_out->write(data);
}

/* MSX - Frael Bruc 100-1 */

ROM_START(bruc100)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("v1_1_mcl", 0x0000, 0x8000, CRC(c7bc4298) SHA1(3abca440cba16ac5e162b602557d30169f77adab))
	ROM_LOAD("f_v1_0", 0x8000, 0x2000, CRC(707a62b6) SHA1(e4ffe02abbda17986cb161c332e9e54d24fd053c))
	ROM_RELOAD(0xa000, 0x2000)
ROM_END

void bruc100_state::bruc100(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// Non-standard cassette port
	// 0 Cartridge slots
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_BRUC100, "firm", 0, 0, 2, "mainrom");
	// Expansion slot in slot 1
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 4).set_total_size(0x10000);   // 64KB RAM

	msx1(VDP_TMS9129, SND_AY8910, config, layout_msx_nocode);
	m_maincpu->set_addrmap(AS_IO, &bruc100_state::io_map);
}

/* MSX - Frael Bruc 100-2 */

ROM_START(bruc100a)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("bruc100-2bios.rom", 0x0000, 0x8000, CRC(24464a7b) SHA1(88611b54cdbb79aa5380570f3dfef8b3a1cc2057))
	// v1.3
	ROM_SYSTEM_BIOS(0, "v13", "v1.3 firmware")
	ROMX_LOAD("bruc100_2_firmware1.3.rom", 0x8000, 0x8000, CRC(286fd001) SHA1(85ab6946950d4e329d5703b5defcce46cd96a50e), ROM_BIOS(0))
	// v1.2
	ROM_SYSTEM_BIOS(1, "v12", "v1.2 firmware")
	ROMX_LOAD("c_v1_2.u8", 0x8000, 0x8000, CRC(de12f81d) SHA1(f92d3aaa314808b7a9a14c40871d24f0d558ea00), ROM_BIOS(1))
	// v1.1
	ROM_SYSTEM_BIOS(2, "v11", "v1.1 firmware")
	// Firmware 1.1 - 8kb bootlogo rom (mirrored to 2000-3fff)
	ROMX_LOAD("bruc100_2_firmware1.2.rom", 0x8000, 0x2000, CRC(54d60863) SHA1(b4c9a06054cda5fd31311a79cc06e6f018cf828f), ROM_BIOS(2))
	ROMX_LOAD("bruc100_2_firmware1.2.rom", 0xa000, 0x2000, CRC(54d60863) SHA1(b4c9a06054cda5fd31311a79cc06e6f018cf828f), ROM_BIOS(2))
ROM_END

void bruc100_state::bruc100a(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// Non-standard cassette port
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_BRUC100, "firm", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);   // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	// Expansion slot in slot 3

	msx1(VDP_TMS9129, SND_AY8910, config, layout_msx_nocode);
	m_maincpu->set_addrmap(AS_IO, &bruc100_state::io_map);
}

} // anonymous namespace

COMP(1987, bruc100,    0,        0,     bruc100,    bruc100,  bruc100_state, empty_init, "Frael", "Bruc 100-1 (MSX1, Italy)", 0)
COMP(1988, bruc100a,   bruc100,  0,     bruc100a,   bruc100,  bruc100_state, empty_init, "Frael", "Bruc 100-2 (MSX1, Italy)", 0)
