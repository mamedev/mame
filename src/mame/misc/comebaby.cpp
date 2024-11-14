// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Angelo Salese
/* Come On Baby
  (c) 2000 ExPotato Co. Ltd (Excellent Potato)

TODO:
- Throws "Primary master hard disk fail" in shutms11. Disk has a non canonical -chs of 524,255,63.
  winimage will throw plenty of errors on manual file extraction (related to Korean paths?).
- Currently needs enabled_logical true in super I/O handling (needs the real
  ITE Giga I/O to fix);
- In this driver with a manually rebuilt image will loop during the fake "now loading" screen
  (customized Win 98 splash screen);
- In pcipc with a manually rebuilt image will throw an exception in "Internat" module once it loads
  Windows 98 (and installs the diff drivers).

===================================================================================================

  There also appears to be a sequel which may be running on the same hardware, but which does not seem to have been released.
  Come On Baby - Ballympic Heroes!  (c) 2001

  Other games in this series include:
  Come On Baby 2 (c) 2002
  Come On Baby Jr (c) 2003 (which seems to be otherwise identical to Come On Baby but in a smaller cabinet)
  Come On Baby 2 Jr (c) 2003 (which seems to be otherwise identical to Come On Baby 2 but in a smaller cabinet)
  These may or may not be on identical hardware.

  This is a Korean PC based board running Windows.  The game runs fully from
  the hard disk making these things rather fragile and prone to damage.

  PC Motherboard: PC Partner BXAS1-928
  BIOS String: 07/03/2000-440BX-ITE8671-2A69KV3IC-00
  aka. BIOS-I-2M
  Award BIOS
  B1120IAG.BIN  For Super IO = ITE 8671
        Clock Gen IC : Winbond-39A
        (Nov/2000 built)

  CPU - Slot 1 Celeron 633MHz (66x9.5)
  Memory - 65536k SDRAM PC100

  Brief motherboard overview (from PC Partner BXAS1-928 motherboard manual)
  --

  HARDWARE CONFIGURATION
  This motherboard is based on Intel 82440BX chipset. The chipset is a
  highly integrated solution for a cost-effective and compact motherboard.
  The motherboard supports standard DIMM module, PC-100 DIMM module or
  Registered DIMM Module.

  Features on-board include....
  super-I/O, Ultra DMA33 (Ultra DMA66 optional for VIA chipset), PCI bus master IDE, AGP Ver 1.0, PCI Ver 2.1 compliance,
  USB, VRM 8.4 compliance, ECC, ATX specification 2.01 compliance, hardware monitoring (optional), On-board PCI Sound
  Sub-system(optional, not populated). Supports PC-100 DIMM Module.

  Key Features:

  Processor
  - Full support for the Intel Pentium II & Intel Pentium III, Intel Celeron and Deschutes processors using Slot 1 connector.
  - Slot 1 connector for Intel Pentium II & Intel Pentium III microprocessors.
  - Supports 66MHz and 100MHz bus speed including all Pentium II & Pentium III processors and future processor.

  CPU Speed Setting
  - Jumper setting or no jumper is needed to set for various speed of CPU (Factory optional).

  VRM (Voltage Regulator Modules) on Board
  - Flexible motherboard design with on-board VRM 8.4, easy to upgrade with
  Future Intel Pentium II & Pentium III processors.

  Cache
  - Pentium II & Pentium III Processor built-in L2 cache.

  System Memory
  - Up to 384MB (SDRAM) or 768MB (Registered SDRAM) - for 440BX.
  - A total of three 168-pin DIMM sockets for 440BX.
  - Supports SDRAM (only for 66MHz bus speed).
  - Supports PC-100.

  Memory Organization
  - Supports single-density DIMMs of 1MB, 2MB, 4MB, 8MB and 16MB depth (x64 or 72).
  - Supports double-density DIMMs of 2MB, 4MB, 8MB, 16MB and 32MB depth
  (x64 or 72).
  - Supports error checking correction (ECC) using parity DRAM modules (for 440BX).
  - Banks of different DRAM types depths can be mixed.

  Expansion Slots
  - Supports SB-LINK Port for Creative Sound PCI Card.
  - 1 AGP slot (ver. 1.0, 1x/2x mode supported).
  - 5 PCI bus master slots (rev. 2.1 compliant, with 1 PCI slot sharing with 1 ISA slot)
  - 2 ISA slots (1 ISA slot sharing with 1 PCI slot).

  On-Board I/O
  - Two PCI fast IDE ports supporting up to 4 ATA2, Ultra DMA33 IDE HDDs, Ultra DMA66 (optional only for VIA Chipset)
  IDE HDDs, CD-Roms, ZIP devices and LS-120 drives as boot drive.
  - Supports bus master IDE, PIO mode 4 (up to 16M bytes/sec), Ultra DMA33 (up
  to 33M bytes/sec) transfer.
  - One ECP/EPP parallel port .
  - Two 16550-compatible UART serial ports.
  - One floppy port supporting two FDDs of 360KB, 720KB, 1.2MB, 1.44MB
  or 2.88MB formated capacity.
  - Two USB ports.
  - PS/2 keyboard port.
  - PS/2 mouse port.
  - Infrared (IrDA) support (via a header).
  - One Line / Speaker out, one Mic in, one Line in and MIDI / Gameport

  System BIOS
  - 2MB flash BIOS supporting PnP, APM, ATAPI, ACPI and DMI;
  - Jumper selection for 5V or 12V flash memory voltage.
  - Auto detects and supports LBA hard disks with formatted capacities over
  8.4GB.
  - Easily upgradable by end-user.

  Plug-and-Play
  - Supports Plug-and-Play Specification 1.1.
  - Plug-and-play for DOS, Windows 3.X, Windows 95 as well as Windows 98.
  - Fully steerable PCI interrupts.

  Power Management
  - Supports SMM, APM and ACPI.
  - Break switch for instant suspend/resume on system operation.
  - Energy star "Green PC" compliant .
  - Supports WAKE-ON-LAN (WOL).
  - Supports Wake on Ring for External Modem.
  - Supports ATX specification 2.01.

  Creative PCI Sound (optional, not populated)
  - Full DOS game support (DDMA, PC/PCI, CLS).
  - PCI 2.1 Bus Master, hardware sound acceleration.
  - Direct sound and sound Blaster Compatible.
  - Full Duplex, 3D Enhancement, Software wavetable.
  - PNP and APM 1.2 support.
  - Win95/98, NT drivers ready.

  Keyboard Password Turn ON
  - Special feature for system security.

  System monitoring (optional)
  - Hardware monitoring circuitry is supported, provides voltages, temperature, fan speeds etc. monitoring.

  --

  The donor PC looks like a standard Korean Windows 98 setup.
  The only exceptions we see are that there's a game logo.sys/logo.bmp in the
  root directory to hide the Windows 98 startup screen, and a shortcut to
  the game in the startup programs.
  Also of interest, Windows 98 was installed from a setup folder on the HD.
  To me this hints that there may have been some expectation of the disk
  being plugged into random hardware.

  The game is pretty much a standard PC game running on a Windows 98 PC.
  It uses DirectSound and the Microsoft MCI interfaces and 3dfx Glide for video.
  The PC that the game was dumped from has Sound Blaster and Ensoniq drivers,
  but it works fine with some other sound configurations.
  The sound chip on the motherboard is not populated. There is a cheap Korean
  sound card "CS-6500P Made In Korea OJU CTN CO LTD." plugged into one of the
  slots containing a CRYSTAL CS4281-CM chip.
  The donor PC has a "3dfxvoodoo3" driver installation directory, but it works
  fine with a Voodoo4 4500.

  The game itself has some protection, it expects a file C:\\WINDOWS\win386p.swp of 84 bytes
  to have the hard disk volume serial number in ascii number (not hex) format at offset 4.

  The game appears to use parallel port (0x378,0x379) for I/O.
  The direct port access means it won't run on XP.
  For the controls, it writes a device select to 0x378, and reads the device value from 0x379.
  There is some other output, maybe lights?
   --------------------------------------------
  |SELECT|RETURN                               |
  |--------------------------------------------|
  | 0x8  | self test/protection, return 5      |
  | 0x0  | P1 4 way joystick                   |
  |      | ----x--- right                      |
  |      | -----x-- left                       |
  |      | ------x- down                       |
  |      | -------x up                         |
  | 0x1  | P1 buttons                          |
  |      | -----x-- C+D (is also start button) |
  |      | ------x- B                          |
  |      | -------x A                          |
  | 0x2  | P2 joystick (as P1)                 |
  | 0x3  | P2 buttons  (as P1)                 |
  | 0x4  | Coin/Service                        |
  |      | -----x-- Coin                       |
  |      | ------x- Coin                       |
  |      | -------x Test                       |
   --------------------------------------------

  Easy enough to fix a broken game if you have the controls to plug into it.
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
#include "bus/isa/isa_cards.h"
#include "bus/pci/virge_pci.h"
//#include "bus/rs232/hlemouse.h"
//#include "bus/rs232/null_modem.h"
//#include "bus/rs232/rs232.h"
//#include "bus/rs232/sun_kbd.h"
//#include "bus/rs232/terminal.h"
#include "machine/fdc37c93x.h"
#include "video/voodoo_pci.h"

#define ENABLE_VOODOO 0

namespace {

#define PCI_AGP_ID "pci:01.0:00.0"

class comebaby_state : public driver_device
{
public:
	comebaby_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_voodoo3(*this, PCI_AGP_ID)
	{ }

	void comebaby(machine_config &config);

private:
	required_device<pentium2_device> m_maincpu;
	// optional for making the compile switch to work
	optional_device<voodoo_3_pci_device> m_voodoo3;

	void comebaby_map(address_map &map) ATTR_COLD;

	static void superio_config(device_t *device);
};

void comebaby_state::comebaby_map(address_map &map)
{
	map.unmap_value_high();
}

static INPUT_PORTS_START( comebaby )
INPUT_PORTS_END

static void isa_internal_devices(device_slot_interface &device)
{
	device.option_add("fdc37c93x", FDC37C93X);
}

void comebaby_state::superio_config(device_t *device)
{
	// TODO: wrong super I/O type
	// It's an ITE 8671 "Giga I/O", unemulated
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
void comebaby_state::comebaby(machine_config &config)
{
	PENTIUM2(config, m_maincpu, 66'666'666); // Actually a Celeron (66'666'666 * 19) / 2
	m_maincpu->set_addrmap(AS_PROGRAM, &comebaby_state::comebaby_map);
	//m_maincpu->set_addrmap(AS_IO, &comebaby_state::comebaby_io);
	m_maincpu->set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
	m_maincpu->smiact().set("pci:00.0", FUNC(i82443bx_host_device::smi_act_w));

	PCI_ROOT(config, "pci", 0);
	I82443BX_HOST(config, "pci:00.0", 0, "maincpu", 64*1024*1024);
	I82443BX_BRIDGE(config, "pci:01.0", 0 ); //"pci:01.0:00.0");
	//I82443BX_AGP   (config, "pci:01.0:00.0");

	i82371eb_isa_device &isa(I82371EB_ISA(config, "pci:07.0", 0, m_maincpu));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);

	i82371eb_ide_device &ide(I82371EB_IDE(config, "pci:07.1", 0, m_maincpu));
	ide.irq_pri().set("pci:07.0", FUNC(i82371eb_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(i82371eb_isa_device::pc_mirq0_w));

	I82371EB_USB (config, "pci:07.2", 0);
	I82371EB_ACPI(config, "pci:07.3", 0);
	LPC_ACPI     (config, "pci:07.3:acpi", 0);
	SMBUS        (config, "pci:07.3:smbus", 0);

	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "fdc37c93x", true).set_option_machine_config("fdc37c93x", superio_config);
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

	// YMF740G goes thru "pci:0c.0"
	// Expansion slots, mapping SVGA for debugging
	// TODO: all untested, check clock
#if ENABLE_VOODOO
	VOODOO_3_PCI(config, m_voodoo3, 0, m_maincpu, "screen"); // "pci:0d.0" J4D2
	m_voodoo3->set_fbmem(16);
	m_voodoo3->set_status_cycles(1000);

	// TODO: fix legacy raw setup here
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57);
	screen.set_size(640, 480);
	screen.set_visarea(0, 640 - 1, 0, 480 - 1);
	screen.set_screen_update(PCI_AGP_ID, FUNC(voodoo_3_pci_device::screen_update));
#else
	// "pci:0d.0" J4D2
	// "pci:0e.0" J4D1
	PCI_SLOT(config, "pci:1", pci_cards, 14, 0, 1, 2, 3, "virge").set_fixed(true);
#endif

}


ROM_START(comebaby)
	ROM_REGION32_LE(0x40000, "pci:07.0", 0)  /* motherboard bios */
	ROM_LOAD("b1120iag.bin", 0x000000, 0x40000, CRC(9b6f95f1) SHA1(65d6a2fea9911593f093b2e2a43d1534b54d60b3) )

	DISK_REGION( "pci:07.1:ide1:0:hdd" )
	DISK_IMAGE( "comebaby", 0, BAD_DUMP SHA1(ea57919319c0b6a1d4abd7822cff028855bf082f) )
ROM_END

} // anonymous namespace


GAME( 2000, comebaby, 0, comebaby, comebaby, comebaby_state, empty_init, ROT0, "ExPotato", "Come On Baby", MACHINE_IS_SKELETON )
