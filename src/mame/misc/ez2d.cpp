// license:BSD-3-Clause
// copyright-holders:
/*

TODO:
- Jumps to PC=0xfb000 after the first 2 PCI dword configs, which points to empty 0xff opcodes.
  $3a000 contains an "= Award Decompression BIOS =" header.
  Original ASUS CUBX BIOSes actually have valid opcodes in that range, the dump should be bad.
- ASUS CUBX fails reading RTC, needs virtualizing thru ISA.

- In pcipc ez2d2m HDD boots to a Korean Windows 98SE, it will error out with a
  "Error 7 : Set up the system correctly." after driver installation. Follows dump heuristics:
  \- Disables Windows splash screen, instead it prints "Amuseworld System IV" (edited from autoexec.bat)
  \- Contain a driver for a 3Com 3C90x EtherLink
  \- "C:\Hardware Tools" contains diagnostic DOS .exe files testing ISA range I/Os;
  \- "C:\ez2dancer" is the bulk of game, mostly with *.abm compressed files;
  \- "C:\Drivers" and "C:\Install" contains various BIOSes and install programs,
        is former for earlier HW?
    \-  "C:\Install\Bios1006" contains a "1006cu.awd" file
    \- There are VIA and ALi AGP references (interchangeable?)
    \- "C:\Drivers\BIOSji438" , "fic_437" & "fic_438" all contains FIC PA-2013 BIOSes (original HW?)
    \- "C:\Drivers\Basic" contains Microsoft Tweak UI install files, this looks used by the changes in
       "This PC" icon on bottom right of desktop and for HW cursor when system is in SVGA mode to a dot.
    \- "C:\Drivers\BIOS991026" contains a Riva TNT2 dump (W2137.rom)
    \- "C:\Drivers\monitor" contains a video/monitor test;
  \- Windows reboot and shutdown screens are customized so that former displays the game title, latter
    just goes black screen.

Thanks to Guru for hardware infos and pics for Ez2dancer 2nd Move.
Later games in the series might run on newer, beefier hardware.

ASUS CUBX-103 motherboard
Intel Celeron 533 MHz CPU
128 MB PC100 DIMM RAM
Fujitsu MPD3043AT HD (4.3GB) - The ez2d2m dump in this driver comes from a different, bigger hd
Leadtek Winfast 3D S325 32MB Video Card (might not be the original one)
Sound Blaster Live CT4830 Sound Card
EZ2D-IOCARD-c
Hardlock E-Y-E security dongle

Other games thought to run on this or derived hardware:
Ez2Dancer series:
* Ez2Dancer 1st Move (2000)
* Ez2Dancer UK Move(2002)
* Ez2Dancer UK Move Special Edition (2003)
* Ez2Dancer SuperChina (2004)

Ez2DJ series:
* Ez2Dj The 1st Tracks (1999)
* Ez2DJ The 1st Tracks Special Edition (1999)
* Ez2DJ 2nd TraX: It Rules Once Again (2000)
* Ez2DJ 3rd TraX: Absolute Pitch (2001)
* Ez2DJ 4th TraX: Over Mind (2002)
* Ez2DJ Mini (2003)
* Ez2DJ Dual Platinum (2003)
* Ez2DJ Platinum: Limited Edition (2003)
* Ez2DJ Single (2004)
* Ez2DJ 6th TraX: Self Evolution (2004)
* Ez2DJ 7th TraX: Resistance (2007)
* Ez2DJ 7th TraX Class R Codename: Violet (2009)
* Ez2DJ 7th TraX: Bonus Edition (2011)
* Ez2DJ Azure Expression (2012)
* Ez2DJ Azure Expression: Integral Composition (2012)
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
#include "bus/pci/rivatnt.h"
//#include "bus/rs232/hlemouse.h"
//#include "bus/rs232/null_modem.h"
//#include "bus/rs232/rs232.h"
//#include "bus/rs232/sun_kbd.h"
//#include "bus/rs232/terminal.h"
#include "machine/w83977tf.h"


namespace {

class ez2d_state : public driver_device
{
public:
	ez2d_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void cubx(machine_config &config);
	void ez2d(machine_config &config);

private:
	required_device<pentium2_device> m_maincpu;

	void ez2d_map(address_map &map) ATTR_COLD;
	void ez2d_io(address_map &map) ATTR_COLD;

	static void winbond_superio_config(device_t *device);
};


void ez2d_state::ez2d_map(address_map &map)
{
	map.unmap_value_high();
}

void ez2d_state::ez2d_io(address_map &map)
{
	map.unmap_value_high();
}

static INPUT_PORTS_START( ez2d )
INPUT_PORTS_END

static void isa_internal_devices(device_slot_interface &device)
{
	device.option_add("w83977tf", W83977TF);
}

void ez2d_state::winbond_superio_config(device_t *device)
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

void ez2d_state::cubx(machine_config &config)
{
	// actually a Celeron at 533 MHz
	PENTIUM2(config, m_maincpu, 90'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ez2d_state::ez2d_map);
	m_maincpu->set_addrmap(AS_IO, &ez2d_state::ez2d_io);
	m_maincpu->set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
	m_maincpu->smiact().set("pci:00.0", FUNC(i82443bx_host_device::smi_act_w));

	PCI_ROOT(config, "pci", 0);
	I82443BX_HOST(config, "pci:00.0", 0, "maincpu", 128*1024*1024);
	I82443BX_BRIDGE(config, "pci:01.0", 0 ); //"pci:01.0:00.0");
	//I82443BX_AGP   (config, "pci:01.0:00.0");

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

#if 0
	rs232_port_device& serport0(RS232_PORT(config, "serport0", isa_com, nullptr)); // "microsoft_mouse"));
	serport0.rxd_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::rxd1_w));
	serport0.dcd_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ndcd1_w));
	serport0.dsr_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ndsr1_w));
	serport0.ri_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::nri1_w));
	serport0.cts_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::rxd2_w));
	serport1.dcd_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ndcd2_w));
	serport1.dsr_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ndsr2_w));
	serport1.ri_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::nri2_w));
	serport1.cts_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ncts2_w));
#endif

	PCI_SLOT(config, "pci:01.0:1", agp_cards, 0, 0, 1, 2, 3, "rivatnt").set_fixed(true);
}

void ez2d_state::ez2d(machine_config &config)
{
	ez2d_state::cubx(config);

	PCI_SLOT(config.replace(), "pci:01.0:1", agp_cards, 0, 0, 1, 2, 3, "rivatnt2_m64").set_fixed(true);
	// TODO: Sound Blaster Live CT4830
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( asuscubx )
	ROM_REGION32_LE(0x40000, "pci:07.0", 0)
	ROM_LOAD("cubx1007.awd", 0x00000, 0x40000, CRC(42a35507) SHA1(4e428e8419e533424d9564b290e2d7f4931744ff) )
ROM_END

ROM_START( ez2d2m )
	ROM_REGION32_LE(0x40000, "pci:07.0", 0)
	ROM_SYSTEM_BIOS( 0, "1006cu", "OEM" )
	// From HDD "C:\Install\Bios1006"
	ROMX_LOAD("1006cu.awd", 0x00000, 0x40000, CRC(086c320a) SHA1(4b4c07e594602c467e678187f80e3a5c1445bd30), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "award", "Award (unknown rev)" )
	ROMX_LOAD("ez2dancer2ndmove_motherboard_v29c51002t_award_bios", 0x00000, 0x40000, BAD_DUMP CRC(02a5e84b) SHA1(94b341d268ce9d42597c68bc98c3b8b62e137205), ROM_BIOS(1) ) // 29f020

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "ez2d2m", 0, SHA1(431f0bef3b81f83dad3818bca8994faa8ce9d5b7) )
ROM_END

} // anonymous namespace


COMP( 2000, asuscubx, 0,   0, cubx, 0, ez2d_state, empty_init, "ASUS",        "CUBX", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

GAME( 2001, ez2d2m, 0, ez2d, ez2d, ez2d_state, empty_init, ROT0,   "Amuse World", "Ez2dancer 2nd Move",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
