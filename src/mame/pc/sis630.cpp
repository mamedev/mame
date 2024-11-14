// license: BSD-3-Clause
// copyright-holders: Angelo Salese, R. Belmont
// thanks-to: Diego Nappino
/**************************************************************************************************

SiS 630 chipset based PC

TODO (main):
- Finalize Super I/O handling of ITE 8705F (I/O $2e / $2f)
\- fan monitor, cfr. I/O $294 reads in shutms11 BIOS fan tests;
\- FDC doesn't work, moans on first boot;
- '900 Ethernet PXE (missing ROM dump);
- USB controllers (OpenHCI compliant);
- ACPI is not fully lpc-acpi compliant;
- EISA slots;
- SMBus;
- PS/2 mouse is unstable, worked around by disabling and using a serial mouse instead.

TODO (usability, to be moved in a SW list):
- windows xp sp3: tests HW then does an ACPI devtrap write ($48), will eventually BSoD with
  ACPI STOP #a5 error with param $11
\- To bypass hold F7 while the "to install SCSI drivers [...] press F6" appears.
   And by F7 I really mean it :shrug:

- windows xp sp3: BSoD during install with a STOP #0a IRQL_NOT_LESS_OR_EQUAL;

- windows neptune: BSoD during ethernet check (after time clock setup)
  with a STOP #a0 INTERNAL_POWER_ERROR with param1 0x5 ("reserved"!?)

- gamecstl Kontron BIOS:
\- hangs at PC=0xf3cf2, again wanting a SMI# from devtrap_en_w;
\- No PS/2 inputs;

- gamecstl dump (tested from shutms11, also see notes below):
\- Currently black screens before booting normal Windows, reading $5004 from the LPC ACPI
   (flip EAX to non-zero to bypass).
   NB: it also writes to $5048 once (devtrap_en_w), which should generate a SMI# event;
\- Doesn't accept any PS/2 input, tries to install a "PCI standard CPU Host Bridge" (?),
   hangs there;
\- GUI is never recognized no matter what, punts with DirectX not installed;

- xubuntu 6.10: throws several SCSIDEV unhandled $46 & $51 commands
  (get configuration/read disc information),
  eventually punts to prompt with a "can't access tty: job control turned off" (on live CD) or
  hangs at "Configuring network interfaces" (on actual install);

- xubuntu 10.10: stalls after '900 ethernet check;

- Haiku 0.1: hangs throwing an "unhandled READ TOC format 2",
  serial COM1 prints a "vm_mark_page_range_inuse: page 0x9f in non-free state 7!"

- Red Hat 6.2: Triple Faults on x87 exception check
\- prints the type of mounted floating point exception if bypassed.

Notes on possible shutms11 BIOS bugs:
- BeOS 5 hardwires serial mouse checks at $2e8, ignoring BIOS PnP.
- FreeDOS 1.3 ctmouse often fails serial detection (will work if you load it at prompt)
- BIOS in general often throws serial conflicts when changing port setups,
  even on perfectly valid combinations (i.e. COM1 assigned while COM2 is *disabled*)
- PCI device listing often forgets to list print ACPI irq.

===================================================================================================

    Cristaltec "Game Cristal" (MAME bootleg)

    Skeleton driver by R. Belmont, based on taitowlf.cpp by Ville Linde

    Notes:
    - Specs: P3-866, SiS 630 motherboard + integrated graphics card,
      SiS 7018 sound, DirectX 8.1.
    - Image is just a more or less stock Windows 98SE with a customized shell
      "Ialoader.exe" (sic) to boot the frontend, located in C:\WINDOWS
      It will eventually hang after throwing a "DirectX missing" error.
    - In order to bypass the shell launcher, you should:
      1. edit C:\WINDOWS\system.ini and change shell property to explorer.exe
      2. remove the autoexec.bat contents, it will otherwise copy a bunch of .ini
         files from C:\dat to C:\WINDOWS, and replacing the system.ini shell launcher.
      Alternatively you can also execute "open.exe" from MS-DOS, that removes above customization
      too.
    - (gamecstl) Device Manager installed devices:
      - two Samsung SyncMaster 900SL monitors ('630 + '301?);
      - SiS 630 display adapter;
      - Samsung CD-ROM SC-152L;
      - SiS 5513 Dual PCI IDE Controller;
      - COM1, COM2, LPT1 enabled;
      - a QDI USBDisk USB Mass Storage SCSI driver;
      - a SiS 7001 PCI to USB Open Host Controller + USB root hub x 2;
    - C:\drvs contains a collection of drivers, mostly the ones described above.
    - C:\WINDOWS\temp has a couple footprints:
      1. has a Portuguese version of the Intel Processor Identification Utility,
         most likely used for binding the emulator to the CPU serial via CPUID;
      2. has u3spwd.exe (USB Flash Disk), likely used to copy the necessary files for
         making the frontend to work;
    - Input is via a custom COM1 port JAMMA adaptor.
    - The custom emulator is a heavily modified version of MAME32. If you extract the
      disk image, it's in C:\GH4\GH4.EXE. It's UPX compressed, so unpack it before doing
      any forensics. The emulator does run on Windows as new as XP Pro SP2 but you can't
      control it due to the lack of the custom input.
    - C:\GH4\mvs contains movie clips of the emulated games.
      These are MS-CRAM encoded, 288x208 at 20 fps, stereo MS ADPCM with 11025 Hz sample rate,
      36 seconds length.
      Mentioning this because SiS 630 has several HW registers dedicated to video playback,
      which will be most likely used once we get there.
    - C:\GH4\rdir contains filled NVRAM directory of the supported games.
      These are probably copied from a factory default (like skipping NVRAM errors in spang & mk),
      needs to be extensively checked if they can be flushed and given a working state with no
      arbitrary user data (i.e. the mk games sports about 3 hours of playtime each)

    Updates 27/11/2007 (Diego Nappino):
    The COM1 port is opened at 19200 bps, No parity, 8 bit data, 1 stop bit.
    The protocol is based on a 6 bytes frame with a leading byte valued 0x05 and a trailing one at 0x02
    The four middle bytes are used, in negative logic (0xFF = No button pressed), to implement the inputs.
    Each bit meaning as follows:

             Byte 1         Byte 2          Byte 3        Byte 4
    Bit 0    P1-Credit      P1-Button C     P2-Left        UNUSED
    Bit 1    P1-Start       P1-Button D     P2-Right       UNUSED
    Bit 2    P1-Down        P1-Button E     P2-Button A    SERVICE
    Bit 3    P1-Up          TEST            P2-Button B    UNUSED
    Bit 4    P1-Left        P2-Credit       P2-Button C    UNUSED
    Bit 5    P1-Right       P2-Start        P2-Button D    UNUSED
    Bit 6    P1-Button A    P2-Down         P2-Button E    UNUSED
    Bit 7    P1-Button B    P2-Up           VIDEO-MODE     UNUSED

    The JAMMA adaptor sends a byte frame each time an input changes.
    So for example, if the P1-Button A and P1-Button B are both pressed, it will send:

    0x05 0xFC 0xFF 0xFF 0xFF 0x02

    And when the buttons are both released

    0x05 0xFF 0xFF 0xFF 0xFF 0x02

    CPUID info:

    Original set:
    CPUID Level:       EAX:           EBX:           ECX:           EDX:
    00000000       00000003       756E6547       6C65746E       49656E69
    00000001       0000068A       00000002       00000000       0387F9FF
    00000002       03020101       00000000       00000000       0C040882
    00000003       00000000       00000000       CA976D2E       000082F6
    80000000       00000000       00000000       CA976D2E       000082F6
    C0000000       00000000       00000000       CA976D2E       000082F6

    Version 2:
    CPUID Level:       EAX:           EBX:           ECX:           EDX:
    00000000       00000003       756E6547       6C65746E       49656E69
    00000001       0000068A       00000002       00000000       0387F9FF
    00000002       03020101       00000000       00000000       0C040882
    00000003       00000000       00000000       B8BA1941       00038881
    80000000       00000000       00000000       B8BA1941       00038881
    C0000000       00000000       00000000       B8BA1941       00038881

**************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "machine/intelfsh.h"
#include "machine/it8705f.h"
#include "machine/pci.h"
#include "machine/sis5513_ide.h"
#include "machine/sis630_host.h"
#include "machine/sis630_gui.h"
#include "machine/sis7001_usb.h"
#include "machine/sis7018_audio.h"
#include "machine/sis900_eth.h"
#include "machine/sis950_lpc.h"
#include "machine/sis950_smbus.h"


namespace {

class sis630_state : public driver_device
{
public:
	sis630_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ide_00_1(*this, "pci:00.1")
		, m_lpc_01_0(*this, "pci:01.0")
	{ }

	void sis630(machine_config &config);

	void asuspolo(machine_config &config);
	void asuscusc(machine_config &config);
	void gamecstl(machine_config &config);
	void zidav630e(machine_config &config);

private:

	required_device<pentium3_device> m_maincpu;
	required_device<sis5513_ide_device> m_ide_00_1;
	required_device<sis950_lpc_device> m_lpc_01_0;

//  void main_io(address_map &map) ATTR_COLD;
//  void main_map(address_map &map) ATTR_COLD;
	static void ite_superio_config(device_t *device);
};


static INPUT_PORTS_START(sis630)
INPUT_PORTS_END

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
	device.option_add("it8705f", IT8705F);
}

void sis630_state::ite_superio_config(device_t *device)
{
	it8705f_device &fdc = *downcast<it8705f_device *>(device);
//  fdc.set_sysopt_pin(1);
	fdc.irq1().set(":pci:01.0", FUNC(sis950_lpc_device::pc_irq1_w));
	fdc.irq8().set(":pci:01.0", FUNC(sis950_lpc_device::pc_irq8n_w));
	fdc.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
	fdc.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
	fdc.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
	fdc.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
	fdc.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
	fdc.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));
}

void sis630_state::sis630(machine_config &config)
{
	// Slot 1/Socket 370, Coppermine FC-PGA @ 500~850+/100 MHz or Celeron PPGA 300~600+ MHz
	// TODO: lowered rate for debugging aid, needs a slot option anyway
	PENTIUM3(config, m_maincpu, 100'000'000);
//  m_maincpu->set_addrmap(AS_PROGRAM, &sis630_state::main_map);
//  m_maincpu->set_addrmap(AS_IO, &sis630_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("pci:01.0:pic_master", FUNC(pic8259_device::inta_cb));
//  m_maincpu->smiact().set("pci:00.0", FUNC(sis950_lpc_device::smi_act_w));

	// TODO: unknown flash ROM types
	// Needs a $80000 sized ROM
	AMD_29F400T(config, "flash");

	PCI_ROOT(config, "pci", 0);
	// up to 512MB, 2 x DIMM sockets
	SIS630_HOST(config, "pci:00.0", 0, "maincpu", 256*1024*1024);
	SIS5513_IDE(config, m_ide_00_1, 0, "maincpu");
	// TODO: both on same line as default, should also trigger towards LPC
	m_ide_00_1->irq_pri().set("pci:01.0:pic_slave", FUNC(pic8259_device::ir6_w));
		//FUNC(sis950_lpc_device::pc_irq14_w));
	m_ide_00_1->irq_sec().set("pci:01.0:pic_slave", FUNC(pic8259_device::ir7_w));
		//FUNC(sis950_lpc_device::pc_mirq0_w));

	SIS950_LPC(config, m_lpc_01_0, XTAL(33'000'000), "maincpu", "flash");
	m_lpc_01_0->fast_reset_cb().set([this] (int state) {
		if (state)
			machine().schedule_soft_reset();
	});
	LPC_ACPI(config, "pci:01.0:acpi", 0);
	SIS950_SMBUS(config, "pci:01.0:smbus", 0);

	SIS900_ETH(config, "pci:01.1", 0);
	// USB config: 2 on back, 3 on front. Front is fn 2
	SIS7001_USB(config, "pci:01.2", 0, 3);
	SIS7001_USB(config, "pci:01.3", 0, 2);
	SIS7018_AUDIO(config, "pci:01.4", 0);
	// documentation doesn't mention modem part #, derived from Shuttle MS11 MB manual
//  SIS7013_MODEM_AC97(config, "pci:01.6"

	// "Virtual PCI-to-PCI Bridge"
	SIS630_BRIDGE(config, "pci:02.0", 0, "pci:02.0:00.0");
	// GUI must go under the virtual bridge
	// This will be correctly identified as bus #1-dev #0-func #0 by the Award BIOS
	SIS630_GUI(config, "pci:02.0:00.0", 0);

	// optional stuff (according to Kontron 786LCD manual)
//  "pci:08.0" SCSI controller (vendor=1000 NCR / LSI Logic / Symbios Logic device=0012 53C895A)
//  "pci:09.0" IEEE1394 controller (vendor=1033 NEC device=00ce uPD72872 / μPD72872)

	// TODO: 3 expansion PCI slots (PC104+)
	// "pci:09.x" to "pci:12.x"?
	// (PIC-MG)
	// "pci:20.x" to "pci:17.x"?

	// TODO: 1 parallel + 2 serial ports
	// TODO: 1 game port ('7018?)

	// TODO: move in MB implementations
	// (some unsupported variants uses W83697HF, namely Gigabyte GA-6SMZ7)
	ISA16_SLOT(config, "superio", 0, "pci:01.0:isabus", isa_internal_devices, "it8705f", true).set_option_machine_config("it8705f", ite_superio_config);

	rs232_port_device& serport0(RS232_PORT(config, "serport0", isa_com, "microsoft_mouse"));
	serport0.rxd_handler().set("superio:it8705f", FUNC(it8705f_device::rxd1_w));
	serport0.dcd_handler().set("superio:it8705f", FUNC(it8705f_device::ndcd1_w));
	serport0.dsr_handler().set("superio:it8705f", FUNC(it8705f_device::ndsr1_w));
	serport0.ri_handler().set("superio:it8705f", FUNC(it8705f_device::nri1_w));
	serport0.cts_handler().set("superio:it8705f", FUNC(it8705f_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("superio:it8705f", FUNC(it8705f_device::rxd2_w));
	serport1.dcd_handler().set("superio:it8705f", FUNC(it8705f_device::ndcd2_w));
	serport1.dsr_handler().set("superio:it8705f", FUNC(it8705f_device::ndsr2_w));
	serport1.ri_handler().set("superio:it8705f", FUNC(it8705f_device::nri2_w));
	serport1.cts_handler().set("superio:it8705f", FUNC(it8705f_device::ncts2_w));

	// TODO: AMR (Audio/modem riser) + UPT (Panel Link-TV out), assume [E]ISA compliant, needs specific slot options
//  ISA16_SLOT(config, "isa1", 0, "pci:01.0:isabus", pc_isa16_cards, nullptr, false);
//  ISA16_SLOT(config, "isa2", 0, "pci:01.0:isabus", pc_isa16_cards, nullptr, false);
}

void sis630_state::asuspolo(machine_config &config)
{
	sis630_state::sis630(config);

	// one expansion PCI only
	// SiS950 rebadged as ITE chip (unreadable on available photo)
}

void sis630_state::asuscusc(machine_config &config)
{
	sis630_state::sis630(config);

	// 2x expansion PCIs
	// "ASUS Mozart"
}

void sis630_state::zidav630e(machine_config &config)
{
	sis630_state::sis630(config);

	// SiS630E
	// 3x expansion PCIs
	// ITE 8705F (Super I/O)
	// Winbond chip nearby with unreadable marking

	// Max allowed CPU 333 MHz (according to AIDA16)
}

// Kontron 786LCD/3.5 based
void sis630_state::gamecstl(machine_config &config)
{
	sis630_state::sis630(config);
	// TODO: Actually Celeron, as also stated by the BIOS
	PENTIUM3(config.replace(), m_maincpu, 100'000'000);

	// tries to install '900 on Windows boot, which implies it doesn't have it
	// (leave it on for now since it has specific option in Setup BIOS)
	//config.device_remove("pci:01.1");

	// TODO: mapped RAM config
	// TODO: add custom inputs
	// TODO: eventually remove PS/2 connector defaults
//  subdevice<pc_kbdc_device>("pci:01.0:ps2_con")->set_default_option(nullptr);
//  subdevice<pc_kbdc_device>("pci:01.0:aux_con")->set_default_option(nullptr);
}

ROM_START(shutms11)
	ROM_REGION32_LE(0x80000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "ms11s11d", "ms11s11d")
	ROMX_LOAD( "ms11s11d.bin",     0x040000, 0x040000, CRC(27077a58) SHA1(32327ebf328cb0c2dec819c3710acc83527803c5), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "ms11s134", "ms11s134")
	ROMX_LOAD( "ms11s134.bin",     0x040000, 0x040000, CRC(d739c4f3) SHA1(2301e57163ac4d9b7eddcabce52fa7d01b22330e), ROM_BIOS(1) )
ROM_END

ROM_START(asuspolo)
	ROM_REGION32_LE(0x80000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "polotv", "Polo-TV 1009")
	ROMX_LOAD( "potv1009.awd",     0x040000, 0x040000, CRC(981e1c75) SHA1(0e1cd42ad62fca63e4919c708348ce18947faaa4), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "polo",   "Polo 1011.001 (beta)")
	ROMX_LOAD( "1011efx.001",      0x040000, 0x040000, CRC(00d73848) SHA1(b2b4ed8e9ec10b853dfdabe1af580b01983864fc), ROM_BIOS(1) )
ROM_END

ROM_START(asuscusc)
	ROM_REGION32_LE(0x80000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "cusc",        "Cusc 1009")
	ROMX_LOAD( "cusc1009.awd",     0x040000, 0x040000, CRC(f7d8cab9) SHA1(47e7728d487a8105de1bc0eeb58a603e334304c0), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "cusc_beta",   "Cusc 1011.001 (beta)")
	ROMX_LOAD( "cusc1011.001",     0x040000, 0x040000, CRC(c2935b70) SHA1(8dedfc7423ebbee5dbe3af3ad92cd0f9866ca876), ROM_BIOS(1) )
ROM_END

ROM_START(zidav630e)
	ROM_REGION32_LE(0x80000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "award_v108",  "Award BIOS v1.08")
	ROMX_LOAD( "v630108e.bin",     0x040000, 0x040000, CRC(25c91274) SHA1(95ff37ad0cfb39bb4ceff2db1cd47f13849ea53a), ROM_BIOS(0) )
	// Corrupt file
//  ROM_SYSTEM_BIOS(1, "award_v104",  "Award BIOS v1.04")
//  ROMX_LOAD( "V630e104.bin",     0x040000, 0x040000, CRC(?) SHA1(?), ROM_BIOS(1) )
ROM_END

/*
 * Arcade based GameCristal
 */

ROM_START(gamecstl)
	ROM_REGION32_LE(0x80000, "flash", ROMREGION_ERASEFF )
	// from gamecstl HDD dump, under "C:\drvs\bios\bios1_9"
	ROM_LOAD( "prod19.rom",     0x040000, 0x040000, BAD_DUMP CRC(9262306c) SHA1(5cd805622ecb4d326591b5f2cf918fe5cb1bce8e) )
	ROM_CONTINUE(               0x000000, 0x040000 )

	DISK_REGION( "pci:00.1:ide1:0:hdd" )
	DISK_IMAGE( "gamecstl", 0, SHA1(b431af3c42c48ba07972d77a3d24e60ee1e4359e) )
ROM_END

ROM_START(gamecst2)
	ROM_REGION32_LE(0x80000, "pci:01.0:flash", ROMREGION_ERASEFF )
	ROM_LOAD( "prod19.rom",     0x040000, 0x040000, BAD_DUMP CRC(9262306c) SHA1(5cd805622ecb4d326591b5f2cf918fe5cb1bce8e) )
	ROM_CONTINUE(               0x000000, 0x040000 )

	DISK_REGION( "pci:00.1:ide1:0:hdd" )
	DISK_IMAGE( "gamecst2", 0, SHA1(14e1b311cb474801c7bdda3164a0c220fb102159) )
ROM_END

} // anonymous namespace


COMP( 2000, shutms11,  0,      0,      sis630,   sis630, sis630_state, empty_init, "Shuttle", "MS11 PC (SiS630 chipset)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
COMP( 2001, asuspolo,  0,      0,      asuspolo, sis630, sis630_state, empty_init, "Asus", "Polo \"Genie\" (SiS630 chipset)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // hangs at CMOS check first time, corrupts flash ROM on successive boots
COMP( 2001, asuscusc,  0,      0,      asuscusc, sis630, sis630_state, empty_init, "Asus", "Terminator P-3 \"Cusc\" (SiS630 chipset)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // fails CMOS test, does crc with I/O accesses at $c00
COMP( 2001, zidav630e, 0,      0,      zidav630e,sis630, sis630_state, empty_init, "Zida", "V630E Baby AT (SiS630 chipset)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // Flash ROM corrupts often, otherwise same-y as shutms11


// Arcade based games
GAME( 2002, gamecstl,  0,        gamecstl, sis630, sis630_state, empty_init, ROT0, "Cristaltec", "GameCristal",                 MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2002, gamecst2,  gamecstl, gamecstl, sis630, sis630_state, empty_init, ROT0, "Cristaltec", "GameCristal (version 2.613)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
