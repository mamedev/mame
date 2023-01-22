// license:BSD-3-Clause
// copyright-holders:Guru
/**************************************************************************************************

X Tom 3D

TODO:
- clears a work RAM snippet then jumps to that snippet ... it doesn't do
  that if you soft reset emulation.
- understand how to load game ROMs. u3 contains a FAT12 file system for DOS/Windows
partition.

This game runs on PC-based hardware.
Major components are....

MAIN BOARD
----------
    CPU: Intel Celeron (socket 370) 333MHz
Chipset: Intel AGPset FW82443ZX, PCIset FW82371EB
    RAM: Samsung KMM366S823CTS 8M x 64-bit SDRAM DIMM
  Video: 3DFX 500-0013-04 PCB-mounted BGA
         EliteMT M32L1632512A video RAM (x4)
         14.31818MHz XTAL
   BIOS: Atmel 29C010 flash ROM
  Other: Holtek HT6542B i8042-based keyboard controller
         3V coin battery

SOUND BOARD
-----------
A40MX04 QFP84 CPLD
Yamaha YMZ280B + YAC516
16MHz XTAL
PIC12C508 (secured, not read)
Atmel 93C46 EEPROM
LM358 OP AMP (x3)

ROM BOARD
---------
MX29F1610MC 16M FlashROM (x7)

**************************************************************************************************/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"
#include "machine/pci-ide.h"
#include "machine/i82443bx_host.h"
#include "machine/i82371eb_isa.h"
#include "machine/i82371eb_ide.h"
#include "machine/i82371eb_acpi.h"
#include "machine/i82371eb_usb.h"
#include "video/virge_pci.h"
#include "bus/isa/isa_cards.h"
//#include "bus/rs232/hlemouse.h"
//#include "bus/rs232/null_modem.h"
//#include "bus/rs232/rs232.h"
//#include "bus/rs232/sun_kbd.h"
//#include "bus/rs232/terminal.h"
#include "machine/fdc37c93x.h"
#include "video/voodoo_pci.h"


namespace {

#define PCI_J4D2_ID "pci:0d.0"

class xtom3d_state : public driver_device
{
public:
	xtom3d_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_voodoo(*this, PCI_J4D2_ID)
	{
	}

	void xtom3d(machine_config &config);

private:
	required_device<pentium2_device> m_maincpu;
	// TODO: optional for debugging
	optional_device<voodoo_banshee_pci_device> m_voodoo;

	void xtom3d_map(address_map &map);

	static void superio_config(device_t *device);
};

void xtom3d_state::xtom3d_map(address_map &map)
{
	map.unmap_value_high();
}

static void isa_internal_devices(device_slot_interface &device)
{
	device.option_add("fdc37c93x", FDC37C93X);
}

void xtom3d_state::superio_config(device_t *device)
{
	// TODO: check super I/O type
	fdc37c93x_device &fdc = *downcast<fdc37c93x_device *>(device);
	fdc.set_sysopt_pin(0);
	fdc.gp20_reset().set_inputline(":maincpu", INPUT_LINE_RESET);
	fdc.gp25_gatea20().set_inputline(":maincpu", INPUT_LINE_A20);
	fdc.irq1().set(":pci:07.0", FUNC(i82371eb_isa_device::pc_irq1_w));
	fdc.irq8().set(":pci:07.0", FUNC(i82371eb_isa_device::pc_irq8n_w));
#if 0
	fdc.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
	fdc.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
	fdc.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
	fdc.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
	fdc.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
	fdc.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));
#endif
}

// TODO: unverified PCI config space
void xtom3d_state::xtom3d(machine_config &config)
{
	PENTIUM2(config, m_maincpu, 450'000'000 / 16); // actually Pentium II 450
	m_maincpu->set_addrmap(AS_PROGRAM, &xtom3d_state::xtom3d_map);
	//m_maincpu->set_addrmap(AS_IO, &xtom3d_state::xtom3d_io);
	m_maincpu->set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
	m_maincpu->smiact().set("pci:00.0", FUNC(i82443bx_host_device::smi_act_w));

	PCI_ROOT(config, "pci", 0);
	I82443BX_HOST(config, "pci:00.0", 0, "maincpu", 32*1024*1024);
	I82443BX_BRIDGE(config, "pci:01.0", 0 ); //"pci:01.0:00.0");
	//I82443BX_AGP   (config, "pci:01.0:00.0");

	i82371eb_isa_device &isa(I82371EB_ISA(config, "pci:07.0", 0));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);

	i82371eb_ide_device &ide(I82371EB_IDE(config, "pci:07.1", 0));
	ide.irq_pri().set("pci:07.0", FUNC(i82371eb_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(i82371eb_isa_device::pc_mirq0_w));

	I82371EB_USB (config, "pci:07.2", 0);
	I82371EB_ACPI(config, "pci:07.3", 0);
	LPC_ACPI     (config, "pci:07.3:acpi", 0);

	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "fdc37c93x", true).set_option_machine_config("fdc37c93x", superio_config);
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

	// YMF740G goes thru "pci:0c.0"
	// Expansion slots, mapping SVGA for debugging
	// TODO: all untested, check clock
	// TODO: confirm Voodoo going in J4D2
	#if 0
	VOODOO_BANSHEE_PCI(config, m_voodoo, 0, m_maincpu, "screen"); // "pci:0d.0" J4D2
	m_voodoo->set_fbmem(2);
	m_voodoo->set_tmumem(4, 4);
	m_voodoo->set_status_cycles(1000);

	// TODO: fix legacy raw setup here
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57);
	screen.set_size(640, 480);
	screen.set_visarea(0, 640 - 1, 0, 480 - 1);
	screen.set_screen_update(PCI_J4D2_ID, FUNC(voodoo_2_pci_device::screen_update));
	#endif
	// "pci:0d.0" J4D2
	// "pci:0e.0" J4D1
	VIRGE_PCI(config, "pci:0e.0", 0); // J4C1
}


ROM_START( xtom3d )
	ROM_REGION32_LE(0x20000, "pci:07.0", 0)
	ROM_LOAD( "bios.u22", 0x000000, 0x020000, CRC(f7c58044) SHA1(fd967d009e0d3c8ed9dd7be852946f2b9dee7671) )

	ROM_REGION(0xe00000, "user2", 0)
	ROM_LOAD( "u3",  0x000000, 0x200000, CRC(f332e030) SHA1(f04fc7fc97e6ada8122ea7d111455043d7cc42df) )
	ROM_LOAD( "u4",  0x200000, 0x200000, CRC(ac40ea0b) SHA1(6fcb86f493885d62d20df6bddaa1a1b19d478c65) )
	ROM_LOAD( "u5",  0x400000, 0x200000, CRC(0fb98a20) SHA1(d21f33b0ca65dc6f90a411a9682f960e9c60244c) )
	ROM_LOAD( "u6",  0x600000, 0x200000, CRC(5c092c58) SHA1(d347e1ed957cc989dc71f4f347af926589ae926d) )
	ROM_LOAD( "u7",  0x800000, 0x200000, CRC(833c179c) SHA1(586555f5a4066a762fc05a43ef01be9fa202bb7f) )
	ROM_LOAD( "u19", 0xa00000, 0x200000, CRC(a1ae73d0) SHA1(232c73bfee426b5f651a015c505c26b8ed7176b7) )
	ROM_LOAD( "u20", 0xc00000, 0x200000, CRC(452131d9) SHA1(f62a0f1a7da9025ac1f7d5de4df90166871ac1e5) )
ROM_END

} // Anonymous namespace


GAME(1999, xtom3d, 0, xtom3d, 0, xtom3d_state, empty_init, ROT0, "Jamie System Development", "X Tom 3D", MACHINE_IS_SKELETON)
