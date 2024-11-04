// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Midway Quicksilver II/Graphite skeleton driver

TODO (BIOS):
- Accesses missing keyboard and RTC areas, needs to fix up Super I/O type;
- Currently used SMBus device EEPROM is incompatible with the BIOS used here, it expects a 4 to byte [0x02] (= SDRAM), there's no real code path for currently used DDR SDRAM setting.
Eventually will throw a "SPD device data missing or inconclusive."
- Several hang points before VGA drawing text, pinpoint them all;
- Detects a Pentium II at 700 MHz (?)

TODO:
- Fix HDD BAD_DUMPs ("primary master hard disk fail" in shutms11):
\- hydro -chs 392,255,63
\- offrthnd -chs 784,255,63
  NB: arctthnd just returns a more canonical -chs 16383,16,63,
  the other two hydrthnd dumps -chs 6256,16,63
- In pcipc hydrthnd101b/hydrthnd100d draws some basic debug strings thru
  VGA then throws 6 short beeps "peripheral problem", does extensive checks
  with COM1 $3f8-$3ff, implying that is failing for Diego I/O.
- In pcipc/shutms11 arctthnd keeps repeating arpl calls, is the dump ok?
- In pcipc/shutms11 ultarctc Windows 2000 bootstrap BSoDs with a
INACCESSIBLE_BOOT_DEVICE;
- ultarctcup cdrom has several tools inside for debugging the custom HW;
- It's unclear about how the ultarctcup update really works by reading the
  notes below: it picks up an Arctic Thunder disk and updates it?
  On incompatible HW? Investigate once we get there.

===================================================================================================

Hardware configurations:

Hydro Thunder:   Quicksilver II system and Diego I/O board
Offroad Thunder: Quicksilver II system and Magicbus I/O board
Arctic Thunder:  Graphite system and Substitute I/O board

All of the games communicate with their I/O boards serially.

Quicksilver II hardware:
- Main CPU: Intel Celeron (Pentium II) 333/366MHz
- Motherboard: Intel SE440BX-2 "4S4EB2X0.86A.0017.P10"
https://theretroweb.com/motherboards/s/intel-se440bx-2-seattle-2
- RAM: 64MB PC100-222-620 non-ecc
- Sound: Integrated YMF740G
- Networking: SMC EZ Card 10 / SMC1208T (probably 10ec:8029 1113:1208)
- Video Card: Quantum Obsidian 3DFX Voodoo 2 (CPLD protected)
- Storage: Hard Drive
- OS: TNT Embedded Kernel

Chipsets (440BX AGPset):
- 82443BX Northbridge
- 82371EB PIIX4 PCI-ISA Southbridge

Note: This was once claimed to run on Windows 95 or 98 but has been proven (mostly) false. The TNT Kernel was a "DOS Extender"
that allows core Windows NT functions to work on MS DOS. It's also possible it runs on a custom made OS as both games do not display
anything DOS related.

Graphite hardware:
- Main CPU: Intel Pentium III 733MHz
- Motherboard: BCM GT694VP
- RAM: 128MB PC100/133
- Sound: Integrated AC97 Controller on VT82C686A Southbridge
    -or ES1373/CT5880 Audio Chip
- Networking: SMC EZ Card 10 / SMC1208T (probably 10ec:8029 1113:1208)
- Video Card: 3DFX Voodoo 3
- Storage: Hard Drive (copy protected)
- OS: Windows 2000 Professional

Chipsets (VIA Pro133A):
- VT82C694X Northbridge
- VT82C686A Southbridge

Note: Entirely different motherboard/chipset hardware (most likely needs its own driver). This game's storage device has a
copy protection scheme that "locks" the storage device to the motherboard's serial number. If a drive doesn't match the
motherboard's serial number, the game launcher will give an error.

I/O boards:

MIDWAY GAMES INC
5770-15983-04
DIEGO

|--------------------------------------------------------------|
|       JP13     JP14       JP15        J1         JP2         |
|                                                              |
|                                      JP1         JP5   JP6   |
|                                                  JP4         |
|                                                  JP3         |
|                                                           U1 |
| JP11                              U2   U3                    |
|                                                              |
|     U12                             JP12                     |
|JP9                                                           |
|      U5  U6               U10                             JP8|
|                                  U8             S1           |
|                           Y2                                 |
|      U7        Q2                                   U4       |
|                                                              |
|      JP15                  J2           J3                   |
|      P1        JP10                                 JP7      |
|--------------------------------------------------------------|

Notes:
    J1: Video connector from/to video card
    J2/J3: USB ports (not used)
    JP1: Alternate video connector from/to video card
    JP2: Video output signal to monitor
    JP3: 3 pin jumper: Blue video level, *Open: high, 1-2: low, 2-3: high
    JP4: 3 pin jumper: Green video level, *Open: high, 1-2: low, 2-3: high
    JP5: 3 pin jumper: Red video level, *Open: high, 1-2: low, 2-3: high
    JP6: 3 pin jumper: Video sync polarity, *Open: pos, 1-2: neg, 2-3: pos
    JP7: 13 pin connector for switches and analog controls/potentiometers
    JP8: 14 pin connector for coin, service, volume and test inputs
    JP9: 20 pin ribbon cable connector to wheel driver board
    JP10: 9 pin connector for coin meter
    JP11: 2 pin connector for watchdog reset
    JP12: connector for development use, not used
    JP13-14: connectors, not used
    JP15: Alternate serial port
    JP16: Power connector
    P1: DB9 serial port to computer
    Q2: ULN2064B Darlington Transistor
    S1: Dip Switches (8).
         S1-3: *Off: Game Mode, On: Test Mode
         S1-4: *Off: 25" Cabinet, On: 39" Cabinet
         S1-8: Off: Watchdog Disabled, *On: Watchdog Enabled
    U1: Texas Instruments LS85A Logic Gate
    U2-3: EL244CS Amplifier
    U4: LTC1098 8-bit Serial A/D converter
    U5: PC16550DV UART Interface IC
    U6: Motorola MC74HC273A Octal D Flip-Flop (LS273 based)
    U7: DS14185WM RS-232 Interface IC
    U8: CY7C63513-PVC 8-bit RISC Microcontroller @12MHz (6MHz OSC)
    U10: Atmel 24C01A Serial EEPROM
    U12: MAX707CSA Supervisory Circuit
    Y2: Crystal/XTAL 6.000 MHz


MIDWAY GAMES INC
5770-16226-01
MAGICBUS

|--------------------------------------------------------------------------------------------|
|  J2           JP21      JP22      JP12                        JP9       J1                 |
|                                                    JP24                           JP6      |
|         P1                                                            JP5     JP3 JP2 JP1  |
|       JP23   U19                                                                           |
|                       U22                                                         JP4      |
|JP10                           Q4                                U2        U3        U1     |
|                U17                                                                         |
|     Y1   U18                                                                               |
|                                                              U5                            |
|                 U20                                                                        |
|JP11   U21                                                                                  |
|                                                                                            |
|                                                          U6          S2        S1          |
|                JP20                                                                        |
|                                                                                            |
|JP16      U12                      U16        U15                                           |
|                                                                                            |
|                                                                                      U11   |
|      JP17         JP19           JP18              JP13             JP14           JP15    |
|--------------------------------------------------------------------------------------------|

Notes:
    J1: Video connector from/to video card
    J2: USB port (not used)
    JP1: 3 pin jumper: Blue video level, *Open: high, 1-2: low, 2-3: high
    JP2: 3 pin jumper: Green video level, *Open: high, 1-2: low, 2-3: high
    JP3: 3 pin jumper: Red video level, *Open: high, 1-2: low, 2-3: high
    JP4: 3 pin jumper: Video sync polarity, Open: pos, *1-2: neg, 2-3: pos
    JP5: Alternate video connector from/to video card
    JP6: Video output signal to monitor
    JP9: Power connector
    JP10: 20 pin ribbon cable connector to wheel driver board
    JP11: 20 pin ribbon cable connector, not used
    JP12-14: connectors, not used
    JP15: 14 pin connector for analog controls/potentiometers
    JP16: 15 pin connector for gameplay inputs
    JP17: connector, not used
    JP18: 14 pin connector for coin, service, volume and test inputs
    JP19: connector, not used
    JP20: connector for development use, not used
    JP21: 2 pin connector for watchdog reset
    JP22: 9 pin connector for coin meter
    JP23: Alternate serial port
    JP24: connector, not used
    P1: DB9 serial port to computer
    Q4: ULN2064B Darlington Transistor
    S1: Dip Switches (8)
         S1-7: *Off: UART, On: USB
         S1-8: *Off: Watchdog Enabled, On: Watchdog Disabled
    S2: Dip Switches (8), all set to "off"
    U1: LS85A Logic Gate
    U2-3: EL244CS Amplifier
    U5: MAX707CSA Supervisory Circuit
    U6: Motorola MC74HC273A Octal D Flip-Flop (LS273 based)
    U11: ADC0834 Serial I/O Converter
    U12-15: HC541 Octal Buffer
    U17: Atmel 24C01A Serial EEPROM
    U18: CY7C63513-PVC 8-bit RISC Microcontroller @12MHz (6MHz OSC)
    U19: DS14185WM RS-232 Interface IC
    U20: PC16550DV UART Interface IC
    U21: Oscillator 3.6864 MHz
    U22: HC04 Hex Inverter
    Y1: Crystal/XTAL 6.000 MHz

MIDWAY GAMES INC.
SUBSTITUTE MAGICBUS
5770-16367-02

|--------------------------------------------------------------------------------------------|
|  J2     P1  JP21    JP22       JP12                           JP9       J1                 |
|       JP23                                         JP24                           JP6      |
|        U21         U24          U22                                   JP5     JP3 JP2 JP1  |
|                                                                                            |
|                                                     U23                           JP4      |
|JP10                                                                                        |
|                U11   U7        U8                                                          |
|                                                                                            |
|                                                                                            |
|                                 U6                                                         |
|JP11           Y2    U20                                                                    |
|                                                                                            |
|          U25                                    U4                                         |
|                                                                      S2        S1          |
|JP16      U15   U16   U17    U19   U18    U5     U13    U12    U14                          |
|                                                                                            |
|                                                                                            |
|                                                                                            |
|      JP17         JP19           JP18              JP13             JP14           JP15    |
|--------------------------------------------------------------------------------------------|

Notes:
    J1/JP5: Video connector from/to video card, not used
    J2: USB port, not used
    JP1-4: Video signal jumpers, not used
    JP6: Video output signal to monitor, not used
    JP9: Power connector
    JP10: 20 pin ribbon cable connector to wheel driver board
    JP11: 20 pin ribbon cable connector, not used
    JP12-14: connectors, not used
    JP15: 14 pin connector for analog controls/potentiometers
    JP16: 15 pin connector, attack button
    JP17: connector, not used
    JP18: 14 pin connector for coin, service, volume and test inputs
    JP19: 8 pin connector, start button
    JP20: connector for development use, not used
    JP21: 2 pin connector for watchdog reset
    JP22: 9 pin connector for coin meter
    JP23: Alternate serial port
    JP24: connector, not used
    P1: DB9 serial port to computer
    S1: Dip Switches (8)
    S1: Dip Switches (8)
         S1-7: *Off: UART, On: USB
         S1-8: *Off: Watchdog Enabled, On: Watchdog Disabled
    S2: Dip Switches (8)
    U4-5: MC74HC273A Octal D Flip-Flop (LS273 based)
    U6: 74HC04D Hex inverter
    U7: MAX707CSA Supervisory Circuit
    U8: MAX765CSA Switching Voltage Regulator
    U11: Atmel 24C01A Serial EEPROM
    U12-19: HC541 Octal Buffer
    U20: 87C552 8-bit Microcontroller (Intel MCS-51 based with a 10-bit A/D converter) @ 16MHz
    U21: DS14185WM RS-232 Interface IC
    U22-24: ULN2064B Darlington Transistor
    U25: 74HC367D Hex Buffer/Line Driver
    Y2: Crystal/XTAL 16.000 MHz

**************************************************************************************************/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/pci.h"
#include "machine/pci-ide.h"
#include "machine/pci-smbus.h"
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

namespace {

#define PCI_J4D2_ID "pci:0d.0"
#define PCI_J4D1_ID "pci:0e.0"
#define PCI_J4C1_ID "pci:0f.0"
#define PCI_J4B1_ID "pci:10.0"
// J4E1
#define PCI_AGP_ID "pci:01.0:00.0"

class midway_quicksilver2_state : public driver_device
{
public:
	midway_quicksilver2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_voodoo2(*this, PCI_AGP_ID)
	{
	}

	void midqslvr(machine_config &config);

private:
	required_device<pentium2_device> m_maincpu;
	// optional for debugging ...
	optional_device<voodoo_2_pci_device> m_voodoo2;

	void midqslvr_map(address_map &map) ATTR_COLD;

	static void superio_config(device_t *device);
};

class midway_graphite_state : public driver_device
{
public:
	midway_graphite_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void graphite(machine_config &config);

private:
	required_device<pentium3_device> m_maincpu;

	void graphite_map(address_map &map) ATTR_COLD;
};



void midway_quicksilver2_state::midqslvr_map(address_map &map)
{
	map.unmap_value_high();
}

static void isa_internal_devices(device_slot_interface &device)
{
	device.option_add("fdc37m707", FDC37M707);
}

void midway_quicksilver2_state::superio_config(device_t *device)
{
	fdc37m707_device &fdc = *downcast<fdc37m707_device *>(device);
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

void midway_quicksilver2_state::midqslvr(machine_config &config)
{
	PENTIUM2(config, m_maincpu, 100'000'000); // Celeron, downclocked for debugging
	m_maincpu->set_addrmap(AS_PROGRAM, &midway_quicksilver2_state::midqslvr_map);
	//m_maincpu->set_addrmap(AS_IO, &midway_quicksilver2_state::midqslvr_io);
	m_maincpu->set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
	m_maincpu->smiact().set("pci:00.0", FUNC(i82443bx_host_device::smi_act_w));

	P87C552(config, "iocpu", 16'000'000).set_disable();

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

	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "fdc37m707", true).set_option_machine_config("fdc37m707", superio_config);
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

	// YMF740G goes thru "pci:0c.0"
	// Expansion slots, mapping SVGA for debugging
	// TODO: all untested, check clock
	// TODO: confirm Voodoo going in AGP slot
#if 1
	VOODOO_2_PCI(config, m_voodoo2, 0, m_maincpu, "screen"); // "pci:0d.0" J4D2
	m_voodoo2->set_fbmem(2);
	m_voodoo2->set_tmumem(4, 4);
	m_voodoo2->set_status_cycles(1000);

	// TODO: fix legacy raw setup here
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57);
	screen.set_size(640, 480);
	screen.set_visarea(0, 640 - 1, 0, 480 - 1);
	screen.set_screen_update(PCI_AGP_ID, FUNC(voodoo_2_pci_device::screen_update));
#endif
	// "pci:0d.0" J4D2
	// "pci:0e.0" J4D1
	PCI_SLOT(config, "pci:1", pci_cards, 14, 0, 1, 2, 3, "virge");
}

// Graphite runs on incompatible HW, consider splitting if things starts to get hairy ...

void midway_graphite_state::graphite_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
//  map(0x000a0000, 0x000bffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x20000);
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);
}

void midway_graphite_state::graphite(machine_config &config)
{
	PENTIUM3(config, m_maincpu, 100'000'000); // downclocked for debugging
	m_maincpu->set_addrmap(AS_PROGRAM, &midway_graphite_state::graphite_map);

	PCI_ROOT(config, "pci", 0);
	// ...
}


ROM_START( hydrthnd )
	ROM_REGION32_LE(0x80000, "pci:07.0", 0)
	ROM_LOAD( "lh28f004sct.u8b1", 0x000000, 0x080000, CRC(ab04a343) SHA1(ba77933400fe470f45ab187bc0d315922caadb12) )

	ROM_REGION( 0x2000, "iocpu", 0 )   /* Diego board CY7C63513 MCU code */
	ROM_LOAD( "diego.u8", 0x0000, 0x2000, NO_DUMP ) // 8KB internal EPROM

	DISK_REGION( "pci:07.1:ide1:0:hdd" )
	DISK_IMAGE( "hydro", 0,  BAD_DUMP SHA1(d481d178782943c066b41764628a419cd55f676d) )
ROM_END

ROM_START( hydrthnd101b )
	ROM_REGION32_LE(0x80000, "pci:07.0", 0)
	ROM_LOAD( "lh28f004sct.u8b1", 0x000000, 0x080000, CRC(ab04a343) SHA1(ba77933400fe470f45ab187bc0d315922caadb12) )

	ROM_REGION( 0x2000, "iocpu", 0 )   /* Diego board CY7C63513 MCU code */
	ROM_LOAD( "diego.u8", 0x0000, 0x2000, NO_DUMP ) // 8KB internal EPROM

	DISK_REGION( "pci:07.1:ide1:0:hdd" )
	DISK_IMAGE( "hydro_101b", 0,  SHA1(182a7966c40676031c92dbfbd1b8e594a505a930) )
ROM_END

ROM_START( hydrthnd100d )
	ROM_REGION32_LE(0x80000, "pci:07.0", 0)
	ROM_LOAD( "lh28f004sct.u8b1", 0x000000, 0x080000, CRC(ab04a343) SHA1(ba77933400fe470f45ab187bc0d315922caadb12) )

	ROM_REGION( 0x2000, "iocpu", 0 )   /* Diego board CY7C63513 MCU code */
	ROM_LOAD( "diego.u8", 0x0000, 0x2000, NO_DUMP ) // 8KB internal EPROM

	DISK_REGION( "pci:07.1:ide1:0:hdd" )
	DISK_IMAGE( "hydro_100d", 0,  SHA1(5462f7197b3c510b791093e938a614e706aaed4a) )
ROM_END

ROM_START( offrthnd )
	ROM_REGION32_LE(0x80000, "pci:07.0", 0)
	ROM_LOAD( "lh28f004sct.u8b1", 0x000000, 0x080000, CRC(ab04a343) SHA1(ba77933400fe470f45ab187bc0d315922caadb12) )

	ROM_REGION( 0x2000, "iocpu", 0 )   /* Magicbus board CY7C63513 MCU code */
	ROM_LOAD( "magicbus.u18", 0x0000, 0x2000, NO_DUMP ) // 8KB internal EPROM

	DISK_REGION( "pci:07.1:ide1:0:hdd" )
	DISK_IMAGE( "offrthnd", 0, BAD_DUMP SHA1(d88f1c5b75361a1e310565a8a5a09c674a4a1a22) )
ROM_END

ROM_START( arctthnd )
	ROM_REGION32_LE(0x40000, "bios", ROMREGION_ERASEFF)
	ROM_LOAD( "m29f002bt.u6", 0x000000, 0x040000, CRC(012c9290) SHA1(cdee6f19d5e5ea5bb1dd6a5ec397ac70b3452790) )

	ROM_REGION( 0x2000, "iocpu", 0 )   /* Substitute board 87C552 MCU code */
	ROM_LOAD( "87c552.bin", 0x0000, 0x2000, NO_DUMP ) // 8KB internal EPROM

	DISK_REGION( "pci:07.1:ide1:0:hdd" )
	DISK_IMAGE( "arctthnd", 0,  SHA1(f4373e57c3f453ac09c735b5d8d99ff811416a23) )
ROM_END

ROM_START( ultarctc )
	ROM_REGION32_LE(0x40000, "bios", ROMREGION_ERASEFF)
	ROM_LOAD( "m29f002bt.u6", 0x000000, 0x040000, CRC(012c9290) SHA1(cdee6f19d5e5ea5bb1dd6a5ec397ac70b3452790) )

	ROM_REGION( 0x2000, "iocpu", 0 )   /* Substitute board 87C552 MCU code */
	ROM_LOAD( "87c552.bin", 0x0000, 0x2000, NO_DUMP ) // 8KB internal EPROM

	DISK_REGION( "pci:07.1:ide1:0:hdd" )
	DISK_IMAGE( "uarctict", 0, SHA1(8557a1d7ae8dc41c879350cb1c228f4c27a0dd09) )
ROM_END

/* This is an update CD. This CD along with a dongle was released as a kit to update a normal Arctic Thunder to Ultimate.
Ultimate Arctic Thunder requires a dongle to work. If the dongle isn't detected both during and after installation,
the game will revert back to normal Arctic Thunder. */
ROM_START( ultarctcup )
	ROM_REGION32_LE(0x40000, "bios", ROMREGION_ERASEFF)
	ROM_LOAD( "m29f002bt.u6", 0x000000, 0x040000, CRC(012c9290) SHA1(cdee6f19d5e5ea5bb1dd6a5ec397ac70b3452790) )

	ROM_REGION( 0x2000, "iocpu", 0 )   /* Substitute board 87C552 MCU code */
	ROM_LOAD( "87c552.bin", 0x0000, 0x2000, NO_DUMP ) // 8KB internal EPROM

	DISK_REGION( "pci:07.1:ide1:0:hdd" )
	DISK_IMAGE( "uarctict", 0, SHA1(8557a1d7ae8dc41c879350cb1c228f4c27a0dd09) )

	// TODO: eventually needs mountable option
	DISK_REGION( "cd" )
	DISK_IMAGE_READONLY( "040503_1309", 0, SHA1(453adb81e204b0580ad02c2d98f68525757ec2a1) )
// sourced from these
//    ROM_LOAD( "040503_1309.CUE", 0x0000, 0x000004d, CRC(4a9e2de5) SHA1(04d3d90ad4b235c0ac4606557e16a1410d018fa9) )
//    ROM_LOAD( "040503_1309.BIN", 0x0000, 0x6bd9960, CRC(48a63422) SHA1(9d1cacf07526c5bddf4205c667a9010802f74859) )

ROM_END

} // Anonymous namespace


// there are almost certainly multiple versions of these; updates were offered on floppy disk.  The version numbers for the existing CHDs are unknown.
GAME(1999, hydrthnd,    0,        midqslvr, 0, midway_quicksilver2_state, empty_init, ROT0, "Midway Games", "Hydro Thunder", MACHINE_IS_SKELETON)
GAME(1999, hydrthnd101b,hydrthnd, midqslvr, 0, midway_quicksilver2_state, empty_init, ROT0, "Midway Games", "Hydro Thunder (v1.01b)", MACHINE_IS_SKELETON)
GAME(1999, hydrthnd100d,hydrthnd, midqslvr, 0, midway_quicksilver2_state, empty_init, ROT0, "Midway Games", "Hydro Thunder (v1.00d)", MACHINE_IS_SKELETON)

GAME(2000, offrthnd,    0,        midqslvr, 0, midway_quicksilver2_state, empty_init, ROT0, "Midway Games", "Offroad Thunder", MACHINE_IS_SKELETON)

GAME(2001, arctthnd,    0,        graphite, 0, midway_graphite_state, empty_init, ROT0, "Midway Games", "Arctic Thunder (v1.002)", MACHINE_IS_SKELETON)

GAME(2001, ultarctc,    0,        graphite, 0, midway_graphite_state, empty_init, ROT0, "Midway Games", "Ultimate Arctic Thunder", MACHINE_IS_SKELETON)
GAME(2004, ultarctcup,  ultarctc, graphite, 0, midway_graphite_state, empty_init, ROT0, "Midway Games", "Ultimate Arctic Thunder Update CD ver 1.950 (5/3/04)", MACHINE_IS_SKELETON)
