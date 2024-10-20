// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/******************************************************************************

    Odyssey.
    Silicon Gaming.
    PC based hardware.

    Preliminary driver by Roberto Fresca.

    TODO:
    - bp f7ae6,1,{eip+=0xf;g} fails ISA state $0f;
    - Hangs after playing with SCSI regs, needs dump to proceed;
    - Accesses S3 video in New MMIO mode, core fumbles on video mode setup
      (prepares linear for 32bpp, core sets SVGA 8bpp instead)
    - Hangs after playing with LPT1 & IDE checks;

*******************************************************************************

  Hardware Notes
  --------------

  System Hardware:

  The slot machine system is based on an Intel Pentium motherboard.
  Could be either a Thor or Tucson motherboard with a Pentium micro
  processor running at 133 MHz or higher, with the following bus
  interfaces:

  - PCI bus for connecting to the video controller, peripheral memory
    board, and SCSI disk controller.

  - ISA bus for connecting to the GPIO system (through parallel port),
    ethernet board, on-board sound chip.

  The Thor motherboard is based on Intel's Triton I chipset, which includes
  the 82437FX/82438FX PCI bridge chips, the PIIX ISA bridge chip, the 87306
  chip that provides the serial ports, timers and interrupts, and the IEEE
  1284 parallel port interface to the GPIO system.

  The Tucson motherboard is based on Intel's Triton-II chipset, which includes
  the 82439HX PCI bridge chip, the PIIX3 ISA bridge chip, and the 87306B super
  I/O chip that provides the serial ports, timers and interrupts, and the IEEE
  1284 parallel port interface to the GPIO system.

  The original manufacturer's BIOS is removed from the motherboard. The system
  uses the Silicon Gaming BIOS on the Peripheral Memory Board instead.

  The motherboard has 4x 16MB SIMMs, getting an amount of 64MB of RAM.


  Peripheral Memory Board:

  The Peripheral Memory Board stores the boot code, motherboard basic I/O
  system (BIOS), operating system (OS), drivers, authentication software,
  system configuration, statistics, and game state information. Data on
  the Peripheral Memory Board remains after the system is powered off,
  using the following memory modules:

  - ROM
  - NVRAM
  - EEPROM

  A GPIO box is connected to the motherboard through parallel interface.
  Could be either GPIO I or GPIO II.


  The display monitor is a 26" Philips CRT with a 16:9 aspect ratio,
  mounted in portrait mode onto the monitor bezel. The electronic
  chassis is manufactured by Neotec.


******************************************************************************/

#include "emu.h"

#include "cpu/i386/i386.h"
#include "machine/pci.h"
#include "machine/pci-ide.h"
#include "machine/i82371sb.h"
#include "machine/i82439hx.h"
#include "bus/isa/isa_cards.h"
#include "bus/pci/vision.h"
//#include "bus/rs232/hlemouse.h"
//#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
//#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "machine/pc87306.h"


namespace {

class odyssey_state : public driver_device
{
public:
	odyssey_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void odyssey(machine_config &config);

private:
	required_device<pentium_device> m_maincpu;

	void odyssey_map(address_map &map) ATTR_COLD;
	void odyssey_io(address_map &map) ATTR_COLD;

	static void national_superio_config(device_t *device);
};



void odyssey_state::odyssey_map(address_map &map)
{
	map.unmap_value_high();
}

void odyssey_state::odyssey_io(address_map &map)
{
	map.unmap_value_high();
}

static INPUT_PORTS_START( odyssey )
INPUT_PORTS_END

static void isa_internal_devices(device_slot_interface &device)
{
	device.option_add("pc87306", PC87306);
}

void odyssey_state::national_superio_config(device_t *device)
{
	pc87306_device &fdc = *downcast<pc87306_device *>(device);
	//fdc.set_sysopt_pin(1);
	fdc.gp20_reset().set_inputline(":maincpu", INPUT_LINE_RESET);
	fdc.gp25_gatea20().set_inputline(":maincpu", INPUT_LINE_A20);
	fdc.irq1().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq1_w));
	fdc.irq8().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq8n_w));
	fdc.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
	fdc.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
	fdc.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
	fdc.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
	fdc.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
	fdc.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));
}

static void isa_com(device_slot_interface &device)
{
//  device.option_add("microsoft_mouse", MSFT_HLE_SERIAL_MOUSE);
//  device.option_add("logitech_mouse", LOGITECH_HLE_SERIAL_MOUSE);
//  device.option_add("wheel_mouse", WHEEL_HLE_SERIAL_MOUSE);
//  device.option_add("msystems_mouse", MSYSTEMS_HLE_SERIAL_MOUSE);
//  device.option_add("rotatable_mouse", ROTATABLE_HLE_SERIAL_MOUSE);
	device.option_add("terminal", SERIAL_TERMINAL);
//  device.option_add("null_modem", NULL_MODEM);
//  device.option_add("sun_kbd", SUN_KBD_ADAPTOR);
}

// This emulates a Tucson / Triton-II chipset, earlier i430fx TBD
// PCI config space is trusted by Intel TC430HX "Technical Product Specification" page 39
void odyssey_state::odyssey(machine_config &config)
{
	// a Celeron at 1.70 GHz on the MB I checked. <- doesn't match being a Triton/Triton-II ... -AS
	// It also fails refresh check if it's barely above 66 MHz (ISA state $08)
	PENTIUM(config, m_maincpu, 66'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &odyssey_state::odyssey_map);
	m_maincpu->set_addrmap(AS_IO, &odyssey_state::odyssey_io);
	m_maincpu->set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
	m_maincpu->smiact().set("pci:00.0", FUNC(i82439hx_host_device::smi_act_w));

	PCI_ROOT(config, "pci", 0);
	I82439HX(config, "pci:00.0", 0, m_maincpu, 64*1024*1024);

	// TODO: 82371FB
	// accesses both mbirq regs, regular PIIX rather than PIIX3?
	i82371sb_isa_device &isa(I82371SB_ISA(config, "pci:07.0", 0, "maincpu"));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);

	i82371sb_ide_device &ide(I82371SB_IDE(config, "pci:07.1", 0, "maincpu"));
	ide.irq_pri().set("pci:07.0", FUNC(i82371sb_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(i82371sb_isa_device::pc_mirq0_w));

	// TODO: 82371FB USB at 07.2

	// On-board S3 Vision 968
	VISION968_PCI(config, "pci:08.0", 0);

	// pci:0d.0 (J4E1) PCI expansion slot 1
	//PCI_SLOT(config, "pci:1", pci_cards, 13, 0, 1, 2, 3, nullptr);

	// pci:0e.0 (J4D2) PCI expansion slot 2
	//PCI_SLOT(config, "pci:2", pci_cards, 14, 0, 1, 2, 3, nullptr);

	// pci:0f.0 (J4D1) PCI expansion slot 3
	//PCI_SLOT(config, "pci:3", pci_cards, 15, 0, 1, 2, 3, nullptr);

	// pci:10.0 (J4C1) PCI expansion slot 4
	PCI_SLOT(config, "pci:4", pci_cards, 16, 0, 1, 2, 3, "ncr53c825");

	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "pc87306", true).set_option_machine_config("pc87306", national_superio_config);
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

	rs232_port_device& serport0(RS232_PORT(config, "serport0", isa_com, nullptr));
	serport0.rxd_handler().set("board4:pc87306", FUNC(pc87306_device::rxd1_w));
	serport0.dcd_handler().set("board4:pc87306", FUNC(pc87306_device::ndcd1_w));
	serport0.dsr_handler().set("board4:pc87306", FUNC(pc87306_device::ndsr1_w));
	serport0.ri_handler().set("board4:pc87306", FUNC(pc87306_device::nri1_w));
	serport0.cts_handler().set("board4:pc87306", FUNC(pc87306_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("board4:pc87306", FUNC(pc87306_device::rxd2_w));
	serport1.dcd_handler().set("board4:pc87306", FUNC(pc87306_device::ndcd2_w));
	serport1.dsr_handler().set("board4:pc87306", FUNC(pc87306_device::ndsr2_w));
	serport1.ri_handler().set("board4:pc87306", FUNC(pc87306_device::nri2_w));
	serport1.cts_handler().set("board4:pc87306", FUNC(pc87306_device::ncts2_w));
}


/**************************************
*              ROM Load               *
**************************************/

/*
#define ODYSSEY_BIOS \
    ROM_REGION( 0x80000, "maincpu", 0 ) \
    ROM_SYSTEM_BIOS( 0, "bios0", "SGI BIOS 76" ) \
    ROM_LOAD_BIOS( 0,  "sgi_bios_76.bin", 0x000000, 0x80000, CRC(00592222) SHA1(29281d25aaf2051e0794dece8be146bb63d5c488) ) \
    ROM_SYSTEM_BIOS( 1, "bios1", "SGI BIOS 65" ) \
    ROM_LOAD_BIOS( 1,  "sgi_bios_65.bin", 0x000000, 0x80000, CRC(00592222) SHA1(29281d25aaf2051e0794dece8be146bb63d5c488) ) \
    ROM_SYSTEM_BIOS( 2, "bios2", "SGI BIOS 55" ) \
    ROM_LOAD_BIOS( 2,  "sgi_bios_55.bin", 0x000000, 0x80000, CRC(00592222) SHA1(29281d25aaf2051e0794dece8be146bb63d5c488) ) \
    ROM_SYSTEM_BIOS( 3, "bios3", "SGI BIOS 46" ) \
    ROM_LOAD_BIOS( 3,  "sgi_bios_46.bin", 0x000000, 0x80000, CRC(00592222) SHA1(29281d25aaf2051e0794dece8be146bb63d5c488) ) \
    ROM_SYSTEM_BIOS( 4, "bios4", "SGI BIOS 31" ) \
    ROM_LOAD_BIOS( 4,  "sgi_bios_31.bin", 0x000000, 0x80000, CRC(00592222) SHA1(29281d25aaf2051e0794dece8be146bb63d5c488) ) \
    ROM_SYSTEM_BIOS( 5, "bios5", "SGI BIOS 00" ) \
    ROM_LOAD_BIOS( 5,  "sgi_bios_00.bin", 0x000000, 0x80000, CRC(00592222) SHA1(29281d25aaf2051e0794dece8be146bb63d5c488) )
*/

ROM_START( odyssey )

//  ODYSSEY_BIOS

	ROM_REGION32_LE( 0x100000, "pmb", ROMREGION_ERASE00 )   // Peripheral Memory Board (II) ROMS
	ROM_LOAD( "sgi_u13_165_0017_0_rev_a_l97_1352.bin", 0x00000, 0x80000, CRC(31ca868c) SHA1(d1db4ef12add336e25374fcf5d3238b8fbca05dd) )  // U13 - 165-0017 BIOS (27C040/27C4001 EPROM)
	// boot image? Contains valid x86 code at $f50, login info to a network at $000
	ROM_LOAD( "sgi_u5_165_0030_0_at28c010.bin",        0x80000, 0x20000, CRC(75a80169) SHA1(a8ece0f82a49f721fb178dbe25fc859bd65ce44f) )  // U5 - 165-0030 CONFIG (Atmel 28C010-12PC EEPROM)

	ROM_REGION( 0x380000, "other", 0 )  // remaining BIOS
	// doesn't seem to have valid x86 boot vectors, may be reused later.
	ROM_LOAD( "sgi_bios_76.bin", 0x000000, 0x80000, CRC(00592222) SHA1(29281d25aaf2051e0794dece8be146bb63d5c488) )
	ROM_LOAD( "sgi_bios_65.bin", 0x080000, 0x80000, CRC(af970c2a) SHA1(0fb49bca34dbd0725b5abb9c876bb849be31b3ed) )
	ROM_LOAD( "sgi_bios_62.bin", 0x100000, 0x80000, CRC(e76b0ec1) SHA1(e537a8592759e34c6c039f2d4cb77c9b58459841) )
	ROM_LOAD( "sgi_bios_55.bin", 0x180000, 0x80000, CRC(0138ef08) SHA1(fad1c0edf37042fffcb5a4006fd69ac59b55ab33) )
	ROM_LOAD( "sgi_bios_46.bin", 0x200000, 0x80000, CRC(37090b87) SHA1(431c0a1954d5bf7fd4fa6f2b983010fbf3c8ce13) )
	ROM_LOAD( "sgi_bios_31.bin", 0x280000, 0x80000, CRC(0954278b) SHA1(dc04a0604159ddd3d24bdd292b2947cc443054f8) )
	ROM_LOAD( "sgi_bios_00.bin", 0x300000, 0x80000, CRC(41480fb5) SHA1(073596d3ba40ae67e3be3f410d7b29c77988df47) )

	ROM_REGION32_LE( 0x100000, "pci:07.0", 0 )
	ROM_COPY( "other", 0x00000, 0x00000, 0x80000 )
	ROM_COPY( "pmb",   0x00000, 0x80000, 0x80000 )

	ROM_REGION( 0x10000, "vbios", 0 )   // video card BIOS
	ROM_LOAD( "videobios", 0x000000, 0x00d000, NO_DUMP )

	DISK_REGION( "scsi_hdd_image" ) // SCSI HDD
	DISK_IMAGE( "odyssey", 0, NO_DUMP )

ROM_END

} // anonymous namespace


/**************************************
*           Game Driver(s)            *
**************************************/

/*    YEAR  NAME      PARENT  MACHINE  INPUT    STATE          INIT        ROT      COMPANY           FULLNAME    FLAGS  */
GAME( 1998, odyssey,  0,      odyssey, odyssey, odyssey_state, empty_init, ROT270, "Silicon Gaming", "Odyssey",   MACHINE_IS_SKELETON )
