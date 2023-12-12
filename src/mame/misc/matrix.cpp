// license:BSD-3-Clause
// copyright-holders:
/*
    'Matrix' slot machine or poker game (the bezel has poker cards) by unidentified manufacturer
    Game title is taken from ROM labels and cabinet. Might be incomplete.

    Hardware consists of:

    Motherboard (GXM-530D):
    Cyrix MediaGX GXm-266GP 2.9V
    Cyrix GXm Cx5530 with GCT bios
    128MB RAM
    SMC FDC37C931
    5-dip bank

    Daughter card (FLASH ROM SSD 374-525-627-33-78J54):
    Lattice ispLSI 1032E 70LJ D980B06
    Unpopulated spaces marked for: DS5002FP, PIC16C54, 93C56 EEPROM, a couple more unreadable
    8-dip bank
    6 ROMs
    1 RAM
*/

#include "emu.h"

#include "bus/isa/isa_cards.h"
#include "cpu/i386/i386.h"
#include "machine/mediagx_cs5530_bridge.h"
#include "machine/mediagx_host.h"
#include "machine/pci.h"

#include "screen.h"


namespace {

class matrix_state : public driver_device
{
public:
	matrix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void matrix(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void main_map(address_map &map);
};


void matrix_state::main_map(address_map &map)
{
}

static INPUT_PORTS_START( matrix )
INPUT_PORTS_END


void matrix_state::matrix(machine_config &config)
{
	// basic machine hardware
	MEDIAGX(config, m_maincpu, 233'000'000); // Cyrix MediaGX GXm-266GP
	m_maincpu->set_addrmap(AS_PROGRAM, &matrix_state::main_map);

	PCI_ROOT(config, "pci", 0);
	MEDIAGX_HOST(config, "pci:00.0", 0, "maincpu", 128*1024*1024);

	// TODO: unconfirmed PCI space
	mediagx_cs5530_bridge_device &isa(MEDIAGX_CS5530_BRIDGE(config, "pci:07.0", 0, "maincpu"));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	//isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);

	// TODO: unknown number of ISA slots
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
}


ROM_START( matrix )
	ROM_REGION32_LE(0x40000, "pci:07.0", 0)
	ROM_LOAD("d586_bios.bin", 0x00000, 0x40000, CRC(39fc093a) SHA1(3376bac4f0d6e729d5939e3078ecdf700464cba3) )

	ROM_REGION(0x300000, "unsorted", 0) // encrypted?
	ROM_LOAD( "matrix_031203u5.bin",  0x000000, 0x080000, CRC(95aa8fb7) SHA1(8cbfa783a887779350609d6f3ea1e88187bd21a4) )
	ROM_LOAD( "matrix_031203u6.bin",  0x080000, 0x080000, CRC(38822bc6) SHA1(b57bd9fa44cab9fa4cef8873454c8be0dc7ab781) )
	ROM_LOAD( "matrix_031203u7.bin",  0x100000, 0x080000, CRC(74d31f1a) SHA1(bf6eae262cab6d24276f43370f3b9e4f687b9a52) )
	ROM_LOAD( "matrix_031203u18.bin", 0x180000, 0x080000, CRC(7b20c6cb) SHA1(51d9a442c510a60f85d9ad7b56cfe67c60f4ab1b) )
	ROM_LOAD( "matrix_031203u19.bin", 0x200000, 0x080000, CRC(c612c80c) SHA1(ef7586369fd1f9c6b8f3e78806c3be16b5aa1a3d) )
	ROM_LOAD( "matrix_031203u20.bin", 0x280000, 0x080000, CRC(f87ac4ae) SHA1(ef9b730a1113d36ef6a041fe36d77edfa255ad98) )
ROM_END

} // anonymous namespace


GAME( 200?, matrix, 0, matrix, matrix, matrix_state, empty_init, ROT0, "<unknown>", "Matrix", MACHINE_IS_SKELETON )
