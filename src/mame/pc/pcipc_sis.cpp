// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*
 * Sandbox for SiS based x86 PCs, targeting the new PCI model
 *
 * Notes:
 * - sis85c471 doesn't belong here, it
 *
 * TODO:
 * - Identify motherboard name(s)
 *
 */

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"
#include "machine/sis85c496.h"

class sis496_state : public driver_device
{
public:
	sis496_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void sis496(machine_config &config);

private:
	required_device<i486dx4_device> m_maincpu;

	void main_io(address_map &map);
	void main_map(address_map &map);
};

void sis496_state::main_map(address_map &map)
{
	map.unmap_value_high();
}

void sis496_state::main_io(address_map &map)
{
	map.unmap_value_high();
}

void sis496_state::sis496(machine_config &config)
{
	// Basic machine hardware
	I486DX4(config, m_maincpu, 75000000); // I486DX4, 75 or 100 Mhz
	m_maincpu->set_addrmap(AS_PROGRAM, &sis496_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &sis496_state::main_io);
//  m_maincpu->set_irq_acknowledge_callback("pci:01.0:pic_master", FUNC(pic8259_device::inta_cb));

	PCI_ROOT(config, "pci", 0);
	SIS85C496_HOST(config, "pci:00.0", 0, "maincpu", 32*1024*1024);

	// TODO: handled by sis85c497
	// isa16_device &isa(ISA16(config, "isa", 0));
}

ROM_START( sis85c496 )
	ROM_REGION32_LE(0x20000, "pci:00.0", 0)
	// Chipset: SiS 85C496/85C497 - CPU: Socket 3 - RAM: 2xSIMM72, Cache - Keyboard-BIOS: JETkey V5.0
	// ISA16: 3, PCI: 3 - BIOS: SST29EE010 (128k) AMI 486DX ISA BIOS AA2558003 - screen remains blank
	ROM_LOAD( "4sim002.bin", 0x00000, 0x20000, CRC(ea898f85) SHA1(7236cd2fc985985f21979e4808cb708be8d0445f))
ROM_END

COMP( 199?, sis85c496, 0, 0,       sis496,     0,     sis496_state,     empty_init,        "<unknown>", "486 motherboards using the SiS 85C496/85C497 chipset", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

