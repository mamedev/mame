// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*

 Quake Arcade Tournament

 Only the HDD is dumped.  The HDD is stickered 'Release Beta 2'

 We've also seen CDs of this for sale, so maybe there should be a CD too, for the music?

TODO:
- In pcipc throws "E0409 -- Security key not found." in glquake.exe when it starts running.

===================================================================================================
 -- set info

Quake Arcade Tournament by Lazer-Tron

PC running Windows 95 with a Dongle on the parallel port

Created .chd with version 0.125

It found the following disk paramaters...

Input offset    511
Cylinders   263
Heads       255
Sectors     63
Byte/Sector 512
Sectors/Hunk    8
Logical size    2,163,248,864

The "backup" directory on hard disk was created by the dumper.

 -- Hardware info found on the following web pages:
http://web.archive.org/web/20070810060806/http://www.wave-report.com/archives/1998/98170702.htm
http://www.thedodgegarage.com/3dfx/q3d_quicksilver.htm
http://quakearcadetournament.blogspot.com/
http://web.archive.org/web/20001001045148/http://www.quantum3d.com:80/press%20releases/4-20-98.html
https://www.quaddicted.com/webarchive/www.quaddicted.com/quake-nostalgia/quake-arcade-tournament-edition/

Quantum3D Heavy Metal HM233G (part of Quantum3D's Quicksilver family)
- NLX form factor system that is based on the Intel 440LX chipset
- Intel NX440LX motherboard
    - Intel 82440LX AGPset (82443LX Northbridge / 82371AB PIIX4 PCI-ISA Southbridge)
    - SMC FDC37C677 I/O
    - Yamaha OPL3-SA3 (YMF715) Audio codec (16-bit per sample 3D audio)
    - Intel Pro 10/100 PCI Ethernet NIC
    - (Optional) Cirrus CL-GD5465 AGP Graphics Controller
    - Intel/Phoenix BIOS
- Intel Pentium II 233 233MHz CPU processor with 512KB of L2 cache
- (1) 32MB PC66 66 MHz SDRAM 168-pin DIMM
- Microsoft Windows 95 OSR2.5
- shock-mounted 3.1GB Ultra DMA-33 EIDE hard drive
- 12-24x CD-ROM drive
- 1.44 MB floppy drive
- Quantum3D Obsidian2 90-4440 AGP AGPTV Voodoo2-based realtime 3D graphics accelerator
      (a professional version of the Quantum3D Obsidian2 S-12 AGPTV)
- Companion PCI 2D/VGA: Quantum3D Ventana MGV-PCI (Alliance Semiconductor ProMotion aT25)
      or Quantum3D Ventana "MGV Rush" (custom Quantum3D Ventana 50 Voodoo Rush with 3D-disabled
      2D-only BIOS and no TV-out, only using the Alliance Semiconductor ProMotion aT25)
- Quantum3D GCI-2 (Game Control Interface II) I/O board - designed to interface coin-op and
      industrial input/output control devices to a PC. Fits in either PCI or ISA bus slot for
      mechanical attachment only. Communications between the GCI and the PC are via a standard
      RS-232 serial interface, using a 14-byte packet protocol. Power is provided by a 4-pin
      Molex style disk driver power connector.

Note: Quantum3D Quicksilver QS233G configuration seem very similar to the HM233G, with the only
    exceptions being that the Quantum3D Obsidian2 90-4440 is replaced with the earlier Quantum3D
    Obsidian 100SB-4440V Voodoo Graphics realtime 3D graphics accelerator with 2D Alliance
    Semiconductor ProMotion aT25 MGV 2000 daughter card, and the Quantum3D GCI-2 might be replaced
    with an earlier Quantum3D GCI.

Dongle: Rainbow Technologies parallel-port security dongle (at least 1024 bytes)

HDD image contains remnants of an Actua Soccer Arcade installation.

*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"
#include "machine/pci-ide.h"
#include "machine/i82443bx_host.h"
#include "machine/i82371eb_isa.h"
#include "machine/i82371eb_ide.h"
#include "machine/i82371eb_acpi.h"
#include "machine/i82371eb_usb.h"
#include "machine/w83977tf.h"
#include "bus/isa/isa_cards.h"
//#include "bus/rs232/hlemouse.h"
//#include "bus/rs232/null_modem.h"
//#include "bus/rs232/rs232.h"
//#include "bus/rs232/sun_kbd.h"
//#include "bus/rs232/terminal.h"
#include "video/voodoo_pci.h"

// TODO: change me up to agp_slot
#define PCI_AGP_ID "pci:01.0:00.0"

namespace {

class quakeat_state : public driver_device
{
public:
	quakeat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_voodoo(*this, PCI_AGP_ID)
	{ }

	void ga6la7(machine_config &config);
	void quake(machine_config &config);

private:
	required_device<pentium2_device> m_maincpu;
	required_device<voodoo_banshee_pci_device> m_voodoo;

	void ga6la7_map(address_map &map) ATTR_COLD;
	void ga6la7_io(address_map &map) ATTR_COLD;
	void quake_map(address_map &map) ATTR_COLD;

	static void winbond_superio_config(device_t *device);
};

void quakeat_state::ga6la7_map(address_map &map)
{
	map.unmap_value_high();
}

void quakeat_state::ga6la7_io(address_map &map)
{
	map.unmap_value_high();
}


static INPUT_PORTS_START( quake )
INPUT_PORTS_END

static void isa_internal_devices(device_slot_interface &device)
{
	device.option_add("w83977tf", W83977TF);
}

void quakeat_state::winbond_superio_config(device_t *device)
{
	// TODO: Winbond w83977ef
	w83977tf_device &fdc = *downcast<w83977tf_device *>(device);
//  fdc.set_sysopt_pin(1);
	fdc.gp20_reset().set_inputline(":maincpu", INPUT_LINE_RESET);
	fdc.gp25_gatea20().set_inputline(":maincpu", INPUT_LINE_A20);
	fdc.irq1().set(":pci:07.0", FUNC(i82371eb_isa_device::pc_irq1_w));
	fdc.irq8().set(":pci:07.0", FUNC(i82371eb_isa_device::pc_irq8n_w));
//  fdc.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
//  fdc.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
//  fdc.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
//  fdc.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
//  fdc.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
//  fdc.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));
}

void quakeat_state::ga6la7(machine_config &config)
{
	// TODO: Socket 370 Celeron with 366-566 MHz
	PENTIUM2(config, m_maincpu, 90'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &quakeat_state::ga6la7_map);
	m_maincpu->set_addrmap(AS_IO, &quakeat_state::ga6la7_io);
	m_maincpu->set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
	m_maincpu->smiact().set("pci:00.0", FUNC(i82443bx_host_device::smi_act_w));

	PCI_ROOT(config, "pci", 0);
	// 16MB - 384MB of supported EDO RAM
	I82443LX_HOST(config, "pci:00.0", 0, "maincpu", 256*1024*1024);
	I82443LX_BRIDGE(config, "pci:01.0", 0 ); //"pci:01.0:00.0");
	//I82443LX_AGP   (config, "pci:01.0:00.0");

	i82371eb_isa_device &isa(I82371EB_ISA(config, "pci:07.0", 0, "maincpu"));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);

	i82371eb_ide_device &ide(I82371EB_IDE(config, "pci:07.1", 0, "maincpu"));
	ide.irq_pri().set("pci:07.0", FUNC(i82371eb_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(i82371eb_isa_device::pc_mirq0_w));

	I82371EB_USB (config, "pci:07.2", 0);
	I82371EB_ACPI(config, "pci:07.3", 0);
	LPC_ACPI     (config, "pci:07.3:acpi", 0);
	SMBUS        (config, "pci:07.3:smbus", 0);

	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "w83977tf", true).set_option_machine_config("w83977tf", winbond_superio_config);
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

	VOODOO_BANSHEE_X86_PCI(config, m_voodoo, 0, m_maincpu, "screen"); // "pci:0d.0" J4D2
	m_voodoo->set_fbmem(8);
	m_voodoo->set_status_cycles(1000);
//  subdevice<generic_voodoo_device>(PCI_AGP_ID":voodoo")->vblank_callback().set("pci:07.0", FUNC(i82371eb_isa_device::pc_irq5_w));

	// TODO: fix legacy raw setup here
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57);
	screen.set_size(640, 480);
	screen.set_visarea(0, 640 - 1, 0, 480 - 1);
	screen.set_screen_update(PCI_AGP_ID, FUNC(voodoo_banshee_pci_device::screen_update));
}

void quakeat_state::quake(machine_config &config)
{
	ga6la7(config);
	// ...
}

ROM_START( ga6la7 )
	ROM_REGION32_LE(0x40000, "pci:07.0", 0)
	ROM_LOAD("6la7a.14", 0x00000, 0x40000, CRC(4fb23f37) SHA1(b8c6a647c6f3e5000b8d4d99c42eb1dc66be0cd8) )
ROM_END

ROM_START(quake)
	// 4N4XL0X0.86A.0011.P05
	ROM_REGION32_LE(0x20000, "pci:07.0", 0)  /* motherboard bios */
	// TODO: compressed
//  ROM_LOAD("p05-0011.bio", 0x000000, 0x10000, NO_DUMP )
//  ROM_CONTINUE( 0x1ffff-0xa0, 0xa0 )
	ROM_LOAD("quakearcadetournament.pcbios", 0x000000, 0x20000, NO_DUMP )

	// Hitachi DK237A-21 A/A0A0, IDE/ATA 2.5" 2.1GB 4000 RPM
	// WS03131880
	DISK_REGION( "disks" )
	// wrong chs 263,255,63
//  DISK_IMAGE( "quakeat", 0, BAD_DUMP SHA1(c44695b9d521273c9d3c0e18c88f0dca0185bd7b) )
	// regenerated from above, with -chs 4200,16,63 as per reported HDD label
	DISK_IMAGE( "quakeat", 0, BAD_DUMP SHA1(9a422ad342aeddd447514d0287efde49e3de5fa8) )
ROM_END

} // anonymous namespace


COMP( 1999, ga6la7,  0,  0, ga6la7, 0, quakeat_state, empty_init, "Gigabyte", "GA-6LA7", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // errors out with ISA state 0x05 (keyboard), then wants flash ROM i/f to work properly

GAME( 1998, quake,  0,      quake,  quake, quakeat_state, empty_init, ROT0, "Lazer-Tron / iD Software", "Quake Arcade Tournament (Release Beta 2)", MACHINE_IS_SKELETON )
// Actua Soccer Arcade
