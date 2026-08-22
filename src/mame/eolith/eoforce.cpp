// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Eolith EO Force HW

x86 side
- Slot-1 CPU
- VIA VT82C694T (Apollo Pro 133T northbridge)
- VIA VT82C686B (southbridge)
- ESS Maestro-3 ES1980S + ES1921S AC'97 2.1 codec
- RTL8139C, near a 25 MHz clock, paired with a YCL 20PMT04B Single 10Base-T/100Base-TX
- 1 PCI slot + 1 ISA slot, both empty and presumably unused by the game
- an audio rig connecting to the JAMMA board
- PC133U-333-542 128MB RAM
- NVIDIA NV11 [GeForce2 MX/MX 400] on board under an heatsink

ARM side, connects with a ... SATA connector? (verify)
- a Samsung ARM (which type?)
- 40 MHz Sunny SCO-063 xtal
- a RST1 push button
- Philips TDA1519C Stereo Power Amplifier
- VR2 knob (volume)
- a two bit SW1 dip bank, 8 bit SW2 bank
- 3 buttons on front panel, TEST, SERV and CLR
- JAMMA connector

BIOS is a custom PhoenixBIOS 4.0 Release 6.0, with EO Force customized splash screen.
There's no way in setup menu to disable quiet mode, at best you can enable Phoenix FirstWare icons
(which is a BDR Backup and Disaster Recovery tool).

Game runs on a Korean Windows 98SE, with an ACCESSHW ABI, a Vault folder containing the game data +
an exe for deploying from CD-Rom and a DivX codec installation.

TODO:
- Upgrade chipset, add remaining on-board peripherals
- hangs while checking I/O port $8100 bp e0295,1,{eax^=1;g} bp e02b6,1,{eax^=1;g}
- hangs with a jp $-2 (coming from SMI?) bp f49c6,1,{eip+=2;g}
- CPU gets identified with a :) (just like sega/lindberg.cpp)
- "Resource Conflict - Allocation error static node # 0E"
- Throws a "WARNING: this HDD cannot be read" and tight loops there (PC=7114c) if *any* HDD present
- with shutms11 does stuff in safe mode only, throws a laconic "Burning launch error" if EWCS.EXE
  is executed. TNGP.LOG is populated with just a TNGP.DLL loading attempt.

**************************************************************************************************/

#include "emu.h"

#include "bus/isa/isa_cards.h"
#include "bus/pci/pci_slot.h"
//#include "bus/rs232/hlemouse.h"
//#include "bus/rs232/null_modem.h"
//#include "bus/rs232/rs232.h"
//#include "bus/rs232/sun_kbd.h"
//#include "bus/rs232/terminal.h"
#include "cpu/i386/i386.h"
//#include "machine/fdc37c665ir.h"
#include "machine/pci.h"
#include "machine/vt82c586b_acpi.h"
#include "machine/vt82c586b_ide.h"
#include "machine/vt82c586b_isa.h"
#include "machine/vt82c586b_usb.h"
#include "machine/vt82c598mvp.h"

namespace {

class eoforce_state : public driver_device
{
public:
	eoforce_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void burnstrk(machine_config &config);

private:
	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

//	static void superio_config(device_t *device);

	required_device<pentium2_device> m_maincpu;
};

void eoforce_state::main_map(address_map &map)
{
	map.unmap_value_high();
}

void eoforce_state::main_io(address_map &map)
{
	map.unmap_value_high();
}

void eoforce_state::burnstrk(machine_config &config)
{
	// Slot 1
	// TODO: unverified clock, likely higher
	PENTIUM2(config, m_maincpu, 66'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &eoforce_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &eoforce_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("pci:07.0:pic0", FUNC(pic8259_device::inta_cb));
	m_maincpu->smiact().set("pci:00.0", FUNC(vt82c691_host_device::smi_act_w));

	// TODO: config space not known
	PCI_ROOT(config, "pci");
	// TODO: VT82C694T
	VT82C691_HOST(config, "pci:00.0", 0, "maincpu", 128*1024*1024);
	VT82C691_BRIDGE(config, "pci:01.0", 0 );

	// TODO: VT82C686B
	vt82c596b_isa_device &isa(VT82C596B_ISA(config, "pci:07.0", XTAL(33'000'000), m_maincpu));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.a20m().set_inputline("maincpu", INPUT_LINE_A20);
	isa.cpureset().set_inputline("maincpu", INPUT_LINE_RESET);
	isa.pcirst().set([this] (int state) {
		if (state)
			machine().schedule_soft_reset();
	});
//  isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);

	vt82c586b_ide_device &ide(VT82C586B_IDE(config, "pci:07.1", 0, m_maincpu));
	// TODO: use ad-hoc remapping from ISA
	ide.irq_pri().set("pci:07.0", FUNC(vt82c596b_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(vt82c596b_isa_device::pc_irq15_w));

	VT82C586B_USB (config, "pci:07.2", 0);

	vt82c596b_acpi_device &acpi_pci(VT82C596B_ACPI(config, "pci:07.3", 0));
	acpi_pci.sci_pin_cb().set("pci:07.0", FUNC(vt82c596b_isa_device::acpi_pin_config_w));
	acpi_pipc_device &acpi_dev(ACPI_PIPC(config, "pci:07.3:acpi"));
	acpi_dev.smi().set_inputline("maincpu", INPUT_LINE_SMI);
	acpi_dev.sci().set("pci:07.0", FUNC(vt82c596b_isa_device::pc_acpi_w));
	SMBUS_PIPC(config, "pci:07.3:smbus");

	// TODO: GeForce 2
	PCI_SLOT(config, "pci:01.0:0", agp_cards, 0, 0, 1, 2, 3, "geforce256_ddr");

	PCI_SLOT(config, "pci:1", pci_cards, 8,  0, 1, 2, 3, nullptr);

	// FIXME: determine ISA bus clock
	// TODO: no Super I/O found?
	//ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "superio", true).set_option_machine_config("superio", superio_config);
	ISA16_SLOT(config, "isa1",   0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

//	rs232_port_device &serport0(RS232_PORT(config, "serport0", isa_com, "logitech_mouse"));
//	serport0.rxd_handler().set("board4:superio", FUNC(fdc37c665ir_device::rxd1_w));
//	serport0.dcd_handler().set("board4:superio", FUNC(fdc37c665ir_device::ndcd1_w));
//	serport0.dsr_handler().set("board4:superio", FUNC(fdc37c665ir_device::ndsr1_w));
//	serport0.ri_handler().set("board4:superio", FUNC(fdc37c665ir_device::nri1_w));
//	serport0.cts_handler().set("board4:superio", FUNC(fdc37c665ir_device::ncts1_w));
//
//	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
//	serport1.rxd_handler().set("board4:superio", FUNC(fdc37c665ir_device::rxd2_w));
//	serport1.dcd_handler().set("board4:superio", FUNC(fdc37c665ir_device::ndcd2_w));
//	serport1.dsr_handler().set("board4:superio", FUNC(fdc37c665ir_device::ndsr2_w));
//	serport1.ri_handler().set("board4:superio", FUNC(fdc37c665ir_device::nri2_w));
//	serport1.cts_handler().set("board4:superio", FUNC(fdc37c665ir_device::ncts2_w));
}

ROM_START( burnstrk )
	ROM_REGION32_LE(0x80000, "pci:07.0", 0)
	ROM_LOAD( "sst39sf040.bin", 0x00000, 0x80000, CRC(dc04109c) SHA1(6f2ec771ee3ad2117a645e66280df7eb6ec844c7))

	// Samsung SV2011H/DOM
	// LBA 39,179,952 20GB
	// chdman mangles this as usual with HDDs this big. Allegedly should be -chs 39680,16,63
	// but dump is actually 18 GB so tentatively used -chs 36627,16,63.
	// There's a checksum.txt in the Vault folder, all game subfolders are missing ...
	DISK_REGION( "pci:07.1:ide1:0:hdd" )
	DISK_IMAGE("burning_striker", 0, BAD_DUMP SHA1(3bd84bcd4dfeb480915cd7de969d53a38e5e9e45))
ROM_END

} // Anonymous namespace


GAME(2003, burnstrk, 0, burnstrk, 0, eoforce_state, empty_init, ROT0, "Eolith", "Burning Striker", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING)
