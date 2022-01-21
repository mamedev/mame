// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/***************************************************************************

Saitek OSA Module: Kasparov Sparc (1993)

The chess engine is by the Spracklen's. Their last, and also their strongest.

Hardware notes:
- Fujitsu MB86930 SPARClite @ 20MHz
- 256KB ROM (4*AMD AM27C512)
- 1MB DRAM (8*NEC 424256-60), expandable to 4MB

The module doesn't have its own LCD screen. It has a grill+fan underneath
at the front part, and a heatsink on the CPU.

TODO:
- skeleton device, missing SPARClite emulation, maybe only needs the MMU?

***************************************************************************/

#include "emu.h"
#include "sparc.h"


DEFINE_DEVICE_TYPE(OSA_SPARC, saitekosa_sparc_device, "osa_sparc", "Sparc")


//-------------------------------------------------
//  initialization
//-------------------------------------------------

saitekosa_sparc_device::saitekosa_sparc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, OSA_SPARC, tag, owner, clock),
	device_saitekosa_expansion_interface(mconfig, *this),
	m_maincpu(*this, "maincpu")
{ }

void saitekosa_sparc_device::device_start()
{
}

void saitekosa_sparc_device::device_reset()
{
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

#define ROM_LOAD32_BYTE_BIOS(bios, name, offset, length, hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_SKIP(3) | ROM_BIOS(bios))

ROM_START( sparc )
	ROM_REGION32_BE(0x40000, "maincpu", 0)

	ROM_DEFAULT_BIOS("v518")

	ROM_SYSTEM_BIOS(0, "v512a", "Sparc (rev. 512A)")
	ROM_LOAD32_BYTE_BIOS(0, "sm16b_512.u5",  0x000000, 0x10000, CRC(96bca59d) SHA1(2c7e693d0cdf69b6e566c6dd03bd24d39e32aa82) )
	ROM_LOAD32_BYTE_BIOS(0, "sm16b_512.u4",  0x000001, 0x10000, CRC(15dd621d) SHA1(e8f7404e84fe027b086fcb918fbcaf2ce4203567) )
	ROM_LOAD32_BYTE_BIOS(0, "sm16b_512.u3",  0x000002, 0x10000, CRC(3201c6e4) SHA1(9a209219a0ab4b4f874381a16773bf33f8f7ba25) )
	ROM_LOAD32_BYTE_BIOS(0, "sm16b_512a.u2", 0x000003, 0x10000, CRC(56dedec7) SHA1(4f9d37e0ca639f892a574aa10a3fb42bba9b82c6) )

	ROM_SYSTEM_BIOS(1, "v518", "Sparc (rev. 518)")
	ROM_LOAD32_BYTE_BIOS(1, "sm16b_518.u5", 0x000000, 0x10000, CRC(cee97a8f) SHA1(541d9d3ee469bacb5a4c537f443836a684e5d168) )
	ROM_LOAD32_BYTE_BIOS(1, "sm16b_518.u4", 0x000001, 0x10000, CRC(7ac37299) SHA1(36f95520fbc9dfed2078d848bf6c3cc7c5823895) )
	ROM_LOAD32_BYTE_BIOS(1, "sm16b_518.u3", 0x000002, 0x10000, CRC(29426d9e) SHA1(ad14f6f2f3e6460d834382faeab78dca9ceb2a63) )
	ROM_LOAD32_BYTE_BIOS(1, "sm16b_518.u2", 0x000003, 0x10000, CRC(b2274cf5) SHA1(d04f41fa1f61439a4655003398f3ae27c2d06f82) )

	ROM_REGION(0x1000, "pals", 0)
	ROM_LOAD("palce16v8h.u23.jed", 0x0000, 0x0c25, CRC(de79fabc) SHA1(27e01ec405e261109dbe10c254b7127eda0f1886) )
	ROM_LOAD("palce16v8h.u32.jed", 0x0000, 0x0c25, CRC(422b66c8) SHA1(44b3394e0586c126ee95129c65e6692ffc01fa8e) )
ROM_END

const tiny_rom_entry *saitekosa_sparc_device::device_rom_region() const
{
	return ROM_NAME(sparc);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void saitekosa_sparc_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
#if 0
	SPARCV8(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(0, &saitekosa_sparc_device::main_map);
	m_maincpu->set_mmu(nullptr);
#endif
}


//-------------------------------------------------
//  internal i/o
//-------------------------------------------------

void saitekosa_sparc_device::main_map(address_map &map)
{
}


//-------------------------------------------------
//  host i/o
//-------------------------------------------------

u8 saitekosa_sparc_device::data_r()
{
	return 0xff;
}
