// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

"EuroByte Electronics & Multimedia IASA" PC-based touch games, sold in Spain by Sleic / Petaco.

Windows 3.1 based

TODO:
- Needs Med3931 sound card and Super I/O to continue;
- Trio64 coprocessor part needs improving (draws with half width);
- Hangs checking touch screen interface in COM2;
- Underlying .ini files will load a Greek driver keyboard, this may or may not become a problem;

-----+----------------------------+------+----------------------------------
Dump | Game                       | Year | Notes
-----+----------------------------+------+----------------------------------
NO   | Superchip                  | 1999 |
NO   | Star Touch                 | 2000 |
YES  | Star Touch / EuroPlay 2001 | 2001 | Original Game: http://www.eurobyte.com.gr/gb_europlay.htm

Hardware overview:
MB Soyo SY-5EH5 or similar (e.g. Biostar M5ATD)
16384 KB RAM
Intel Pentium MMX 233 MHz or compatible (e.g. Cyrix M II-300GP 66MHz Bus 3.5x 2.9V)

- Soyo SY-5EH5 uses a VIA Apollo MVP3 chipset with VT82C598 + VT82C586B
- Biostar M5ATD uses an ALi ALADDiN IV+ M1531B + M1543

MicroTouch ISA
ExpertColor Med3931 ISA sound card or other 82C931-based similar card (e.g. BTC 1817DS OPTi ISA)
PCI VGA ExpertColor M50-02 (S3, Trio64V2/DX 86C775, 512KB RAM)
Parallel port dongle HASP4
Creative Video Blaster camera (parallel port)
HDD Samsung SV0322A or other IDE HDD with similar capacity (e.g. Seagate ST32122A).

**************************************************************************************************/

#include "emu.h"

#include "bus/isa/isa_cards.h"
#include "bus/pci/pci_slot.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "cpu/i386/i386.h"
#include "machine/fdc37c665ir.h"
#include "machine/pci.h"
#include "machine/vt82c586b_acpi.h"
#include "machine/vt82c586b_ide.h"
#include "machine/vt82c586b_isa.h"
#include "machine/vt82c586b_usb.h"
#include "machine/vt82c598mvp.h"

namespace {

class startouch_state : public driver_device
{
public:
	startouch_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void europl01(machine_config &config);

private:
	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	static void superio_config(device_t *device);

	required_device<pentium_mmx_device> m_maincpu;
};

void startouch_state::main_map(address_map &map)
{
	map.unmap_value_high();
}

void startouch_state::main_io(address_map &map)
{
	map.unmap_value_high();
}

static void isa_com(device_slot_interface &device)
{
	device.option_add("microsoft_mouse", MSFT_HLE_SERIAL_MOUSE);
	device.option_add("logitech_mouse", LOGITECH_HLE_SERIAL_MOUSE);
	device.option_add("wheel_mouse", WHEEL_HLE_SERIAL_MOUSE);
	device.option_add("msystems_mouse", MSYSTEMS_HLE_SERIAL_MOUSE);
	device.option_add("rotatable_mouse", ROTATABLE_HLE_SERIAL_MOUSE);
	device.option_add("terminal", SERIAL_TERMINAL);
	device.option_add("null_modem", NULL_MODEM);
	device.option_add("sun_kbd", SUN_KBD_ADAPTOR);
}

static void isa_internal_devices(device_slot_interface &device)
{
	device.option_add_internal("superio", FDC37C665IR);
}

void startouch_state::superio_config(device_t *device)
{
	// TODO: upgrade to FDC37C669
	fdc37c665ir_device &fdc = *downcast<fdc37c665ir_device *>(device);
	fdc.fintr().set(":pci:07.0", FUNC(vt82c586b_isa_device::pc_irq6_w));
	fdc.pintr1().set(":pci:07.0", FUNC(vt82c586b_isa_device::pc_irq7_w));
	fdc.irq3().set(":pci:07.0", FUNC(vt82c586b_isa_device::pc_irq3_w));
	fdc.irq4().set(":pci:07.0", FUNC(vt82c586b_isa_device::pc_irq4_w));
	fdc.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
	fdc.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
	fdc.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
	fdc.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
	fdc.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
	fdc.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));
}

void startouch_state::europl01(machine_config &config)
{
	// Super Socket 7 / PGA321
	// TODO: 233 MHz
	PENTIUM_MMX(config, m_maincpu, 66'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &startouch_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &startouch_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("pci:07.0:pic0", FUNC(pic8259_device::inta_cb));

	// TODO: config space not known
	PCI_ROOT(config, "pci", 0);
	// Max 768 MB
	VT82C598MVP_HOST(config, "pci:00.0", 0, "maincpu", 256*1024*1024);
	VT82C598MVP_BRIDGE(config, "pci:01.0", 0 );

	vt82c586b_isa_device &isa(VT82C586B_ISA(config, "pci:07.0", XTAL(33'000'000), m_maincpu));
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
	ide.irq_pri().set("pci:07.0", FUNC(vt82c586b_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(vt82c586b_isa_device::pc_irq15_w));

	VT82C586B_USB (config, "pci:07.2", 0);
	VT82C586B_ACPI(config, "pci:07.3", 0);
	ACPI_PIPC     (config, "pci:07.3:acpi");

	PCI_SLOT(config, "pci:01.0:1", agp_cards, 1, 0, 1, 2, 3, nullptr);

	PCI_SLOT(config, "pci:1", pci_cards, 13, 0, 1, 2, 3, nullptr);
	PCI_SLOT(config, "pci:2", pci_cards, 14, 1, 2, 3, 0, nullptr);
	PCI_SLOT(config, "pci:3", pci_cards, 15, 2, 3, 0, 1, nullptr);
	PCI_SLOT(config, "pci:4", pci_cards, 16, 3, 0, 1, 2, "trio64dx");

	// TODO: add Med3931
	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "superio", true).set_option_machine_config("superio", superio_config);
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

	rs232_port_device &serport0(RS232_PORT(config, "serport0", isa_com, "logitech_mouse"));
	serport0.rxd_handler().set("board4:superio", FUNC(fdc37c665ir_device::rxd1_w));
	serport0.dcd_handler().set("board4:superio", FUNC(fdc37c665ir_device::ndcd1_w));
	serport0.dsr_handler().set("board4:superio", FUNC(fdc37c665ir_device::ndsr1_w));
	serport0.ri_handler().set("board4:superio", FUNC(fdc37c665ir_device::nri1_w));
	serport0.cts_handler().set("board4:superio", FUNC(fdc37c665ir_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("board4:superio", FUNC(fdc37c665ir_device::rxd2_w));
	serport1.dcd_handler().set("board4:superio", FUNC(fdc37c665ir_device::ndcd2_w));
	serport1.dsr_handler().set("board4:superio", FUNC(fdc37c665ir_device::ndsr2_w));
	serport1.ri_handler().set("board4:superio", FUNC(fdc37c665ir_device::nri2_w));
	serport1.cts_handler().set("board4:superio", FUNC(fdc37c665ir_device::ncts2_w));
}

ROM_START(europl01)
	// Sleic used different motherboards for this machine. By now, we're adding all the known BIOSes here
	// TODO: needs separating, incompatible chipsets
	ROM_REGION32_LE(0x20000, "pci:07.0", 0)
	ROM_SYSTEM_BIOS(0, "soyo_5ehm13_1aa1", "Soyo 5EHM (Award BIOS 5EH V1.3-1AA1)")                                                                 // Soyo 5EHM V1.3
	ROMX_LOAD("award_1998_pci-pnp_586_223123413.bin", 0x00000, 0x20000, CRC(d30fe6c2) SHA1(022cf24d982b82e4c13ebbe974adae3a1638d1cd), ROM_BIOS(0)) //   39SF010
	ROM_SYSTEM_BIOS(1, "soyo_5ehm12_1ca2", "Soyo 5EHM (Award BIOS 5EH V1.2-1CA2)")                                                                 // Soyo 5EHM V1.2 (1MB cache)
	ROMX_LOAD("award_1998_pci-pnp_586_222951562.u2",  0x00000, 0x20000, CRC(5bb1bcbc) SHA1(6e2a7b5b3fc892ed20d0b12a1a533231c8953177), ROM_BIOS(1)) //   39SF010
	ROM_SYSTEM_BIOS(2, "bst_m5atd1228b_1", "Biostar M5ATD (Award BIOS ATD1228B, set 1)")                                                           // Biostar M5ATD V1.2 (ALi M5819P + ALi M1543 B1 + ALi M1531 B1 + UMC UM61L6464F-6)
	ROMX_LOAD("award_1998_pci_pnp_586_149197780.u11", 0x00000, 0x20000, CRC(1ec5749b) SHA1(3dd1dac852b00c8108aaf9c89f47ae1922d645f0), ROM_BIOS(2)) //   W29C011
	ROM_SYSTEM_BIOS(3, "bst_m5atd1228b_2", "Biostar M5ATD (Award BIOS ATD1228B, set 2)")                                                           // Biostar M5ATD
	ROMX_LOAD("award_1998_pci-pnp_586_149278871.bin", 0x00000, 0x20000, CRC(3c6aea4d) SHA1(9e56b0f27c204a0eaaf1174070fc95faacc84b0b), ROM_BIOS(3)) //   W29C011

	ROM_REGION(0x20000, "hd_firmware", 0) // Samsung SV0322A
	ROM_LOAD("jk200-35.bin", 0x00000, 0x20000, CRC(601fa709) SHA1(13ded4826a64209faac8bc81708172b81195ab96))

	ROM_REGION(0x66da4, "dongle", 0)
	ROM_LOAD("9b91f19d.hsp", 0x00000, 0x66da4, CRC(0cf78908) SHA1(c596f415accd6b91be85ea8c1b89ea380d0dc6c8))

	DISK_REGION( "pci:07.1:ide1:0:hdd" ) // Sleic-Petaco Star Touch 2001. Version 2.0. Date 06/April/2001
	DISK_IMAGE("sleic-petaco_startouch_2001_v2.0", 0, SHA1(3164a5786d6b9bb0dd9910b4d27a77a6b746dedf)) // Labeled "Star Touch 2001" but when running, game title is EuroPlay 2001
ROM_END

} // Anonymous namespace


GAME(2001, europl01, 0, europl01, 0, startouch_state, empty_init, ROT0, "Sleic / Petaco", "EuroPlay 2001", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
