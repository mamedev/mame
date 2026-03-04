// license:BSD-3-Clause
// copyright-holders:
/*
Games Family P4-4P (Pentium 4 - 4 Players)

PC with Chinese Windows 2000 Pro (build 2195: "Service Pack 4Free") and several emulators, including:
 -MAME v0.78 (Dec 30 2003)
 -MAME v0.96u3 (May 25 2005)
 -MAME v0.106 (May 16 2006)
 -Nebula 2.1.5
 -FB Alpha v0.2.94.98
 -ZiNc 1.1
 -A copy of "KOF2002_WinKawaks1.45" in roms folder (leftover?)
(programs may change on other revisions)

SiS651/SiS962 based chipset plus an additional PCB for JAMMA, inputs and basic config (and protection) with:
 Atmel AT89C2051 (near a 4 dipswitches bank and a 6.000 MHz xtal)
 Microchip CF745 (near another 4 dipswitches bank and a 4.000 MHz xtal)
 2 x Microchip PIC12F508
 Altera Max EPM7128SQC100-10 CPLD

HDD has traces of following drivers:
- C-Media 3D Audio
- SiS651 VGA and AGP drivers
- Realtek/Avance Logic ALC201A(101) AC'97 (SiS OEM?)
- a PPJoyBus.sys and PPortJoy.sys
- a C:\VGATVWIN folder (may need typical SiS TV Out)
- C:\mamep\babystar.exe loads an English version frontend, C:\mamew\babystar.exe loads a Chinese one;
- C:\emu\game2\play1.exe is a stock MAME 0.106. Notice that has -ddraw and -direct3d enabled by
  default (which you have to disable if fiddling in Safe Mode)

TODO:
- Upgrade '630 to '651 variants;
- Hangs at ISA $c1 the first time around, soft reset makes it to go further;
- Hangs on CPU check;
- On shutms11 tries to install Ethernet and SiS7018 drivers, shows "System Loading" for a split
  second then proceeds in pinging the USB port (on Safe Mode too, dongle?);
- MAME runs at 800x600@37fps, fix it in pc_vga_sis;
- "Type OK" screen cannot be surpassed, bug in keyboard emulation or just like above?

*/

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
#include "machine/sis950_acpi.h"
#include "machine/sis950_lpc.h"
#include "machine/sis950_smbus.h"

namespace {

class gfamily_state : public driver_device
{
public:
	gfamily_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void gfamily(machine_config &config);

private:

	required_device<pentium4_device> m_maincpu;

//  void main_io(address_map &map) ATTR_COLD;
//  void main_map(address_map &map) ATTR_COLD;
	static void ite_superio_config(device_t *device);
};


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

void gfamily_state::ite_superio_config(device_t *device)
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


void gfamily_state::gfamily(machine_config &config)
{
	// Socket 478
	// Actually an Intel Celeron SL6SC 1.7GHz (with the config found with the default BIOS)
	PENTIUM4(config, m_maincpu, 100'000'000); //1'700'000'000);
	m_maincpu->set_irq_acknowledge_callback("pci:01.0:pic_master", FUNC(pic8259_device::inta_cb));
	m_maincpu->smiact().set("pci:00.0", FUNC(sis630_host_device::smi_act_w));

	// TODO: everything below needs upgrading to SiS651
	// TODO: unknown flash ROM types
	// Needs a $80000 sized ROM
	AMD_29F400T(config, "flash");

	PCI_ROOT(config, "pci", 0);
	// up to 512MB, 2 x DIMM sockets
	SIS630_HOST(config, "pci:00.0", 0, "maincpu", 256*1024*1024);
	sis5513_ide_device &ide(SIS5513_IDE(config, "pci:00.1", 0, "maincpu"));
	ide.irq_pri().set("pci:01.0", FUNC(sis950_lpc_device::pc_iirqa_w));
	ide.irq_sec().set("pci:01.0", FUNC(sis950_lpc_device::pc_iirqb_w));

	sis950_lpc_device &lpc(SIS950_LPC(config, "pci:01.0", XTAL(33'000'000), "maincpu", "flash"));
	lpc.fast_reset_cb().set([this] (int state) {
		if (state)
			machine().schedule_soft_reset();
	});
	sis950_acpi_device &acpi(SIS950_ACPI(config, "pci:01.0:acpi", 0));
	acpi.smi().set_inputline("maincpu", INPUT_LINE_SMI);
	SIS950_SMBUS(config, "pci:01.0:smbus", 0);

	SIS900_ETH(config, "pci:01.1", 0);
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

	// TODO: PCI/AGP slots

	// TODO: looks different
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
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( gmfamily )

	/* Different PC motherboards with different configurations.
	   By now, we're throwing all known BIOSes here. */
	ROM_REGION32_LE(0x80000, "flash", ROMREGION_ERASEFF)

	/* CPU: Intel Celeron 1.7GHz / 128kb / 400MHz SL6SC
	   RAM: 256MB-DDR400
	   PCB: Realtec ALC655 audio, Realtec RTL8201BL Ethernet (25.000 MHz xtal), Winbond W83194BG-648 (14.31818 MHz xtal) */
	ROM_SYSTEM_BIOS(0, "ay36_8897", "04/082006-SiS-651-6A6IXRMAC-00")
	ROMX_LOAD("686_amibios_ay36_8897.bin", 0x00000, 0x80000, CRC(e04c5750) SHA1(240ca6b270bdebf129e4ce43e79275aa067b6ada), ROM_BIOS(0))

	/* CPU: Pentium 4 2.40GHz/512/533 SL6DV
	   RAM: 512MB
	   PCB: ECS (Elitegroup) 651-M v2.4. Seems like a later low-cost version of the consumer v2.0 motherboard. */
	ROM_SYSTEM_BIOS(1, "sy_sis651", "05/13/2005-SiS-651-6A6IXE19C-00")
	ROMX_LOAD("award_i6a6ixe19.bin",       0x40000, 0x40000, CRC(95fa392c) SHA1(40f557339649c47e6c3d941670604e0559edf8db), ROM_BIOS(1)) // Winbond W49F002UP12N

	// PICs and MCUs from the I/O board, all of them protected
	ROM_REGION(0x10000, "unsorted", 0)
	ROM_LOAD("at89c2051.u3", 0x0000, 0x4000, NO_DUMP) // 2 Kbytes internal ROM
	ROM_LOAD("cf745.u2",     0x4000, 0x2000, NO_DUMP) // 1 Kbytes internal ROM
	ROM_LOAD("pic12f508.u0", 0x6000, 0x2000, NO_DUMP) // 1 Kbytes internal ROM
	ROM_LOAD("pic12f508.u6", 0x8000, 0x2000, NO_DUMP) // 1 Kbytes internal ROM

	DISK_REGION( "pci:00.1:ide1:0:hdd" ) // From a Norton Ghost recovery image
	DISK_IMAGE( "gamesfamily_1.1", 0, SHA1(0410c24cea2a5dc816f4972df65cb1cb0bf1d730) )
ROM_END

} // Anonymous namespace


GAME( 2007?, gmfamily, 0, gfamily, 0, gfamily_state, empty_init, ROT0, "bootleg", "Games Family", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION ) // C:\Windows\system32\drivers folder has a 2007-11-11 timestamp
