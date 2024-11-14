// license:BSD-3-Clause
// copyright-holders:Grull Osgo, Angelo Salese
/**************************************************************************************************

Game Magic (c) 1997 Bally Gaming Co.

Preliminary driver by Grull Osgo

TODO:
- gammagic: pings ESS Solo Adlib ports, prints "Voodoo two 50hz enabled" (dual screen?),
  "ignoring write to swapbufferCMD when CMDFIFO is enabled", hangs there;

- 99bottles: "not High Sierra or ISO9660", likely bad (disc-at-once with one track?)

- Missing 68k dump portion.
  Very unlikely it transfers code from serial, and CD-ROM dump doesn't have any clear file that
  would indicate a code transfer or an handshake between main and sub CPUs;

===================================================================================================

Game Magic

Is a Multigame machine build on a Bally's V8000 platform.

This is the first PC based gaming machine developed by Bally Gaming.

V8000 platform includes:

1 Motherboard MICRONICS M55Hi-Plus PCI/ISA, Chipset INTEL i430HX (TRITON II), 64 MB Ram (4 SIMM M x 16 MB SIMM)
On board Sound Blaster Vibra 16C chipset.
    [has reference to an ESS Solo-1/Maestro driver -AS]
1 TOSHIBA CD-ROM or DVD-ROM Drive w/Bootable CD-ROM with Game.
1 OAK SVGA PCI Video Board.
1 Voodoo Graphics PCI Video Board, connected to the monitor.
    [Voodoo 1 or 2 according to strings in dump -AS]
1 21" SVGA Color Monitor, 16x9 Aspect, Vertical mount, with touchscreen.
    [running at 50Hz with option for 60Hz declared in config file -AS]
1 Bally's IO-Board, Based on 68000 procesor as interface to all gaming devices
(Buttons, Lamps, Switches, Coin acceptor, Bill Validator, Hopper, Touchscreen, etc...)

PC and IO-Board communicates via RS-232 Serial Port.

Additional CD-ROM games: "99 Bottles of Beer"

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
#include "cpu/m68000/m68020.h"
#include "machine/fdc37c93x.h"
#include "machine/i82371sb.h"
#include "machine/i82439hx.h"
#include "machine/pci.h"
#include "video/voodoo_pci.h"

namespace {

class gammagic_state : public driver_device
{
public:
	gammagic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void gammagic(machine_config &config);

private:
	void gammagic_io(address_map &map) ATTR_COLD;
	void gammagic_map(address_map &map) ATTR_COLD;

	static void smc_superio_config(device_t *device);

	void v8000_map(address_map &map) ATTR_COLD;
};

void gammagic_state::gammagic_map(address_map &map)
{
	map.unmap_value_high();
}

void gammagic_state::gammagic_io(address_map &map)
{
	map.unmap_value_high();
}

void gammagic_state::v8000_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x13ffff).ram();
	map(0x1ff800, 0x1fffff).ram();
	//map(0x500010, 0x50001f).rw("asc", FUNC(i82510_device::read), FUNC(i82510_device::write)).umask32(0xff00ff00);
}

static INPUT_PORTS_START( gammagic )
INPUT_PORTS_END

static void isa_internal_devices(device_slot_interface &device)
{
	device.option_add("fdc37c93x", FDC37C93X);
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

void gammagic_state::smc_superio_config(device_t *device)
{
	fdc37c93x_device &fdc = *downcast<fdc37c93x_device *>(device);
	fdc.set_sysopt_pin(1);
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

void gammagic_state::gammagic(machine_config &config)
{
	pentium_device &maincpu(PENTIUM(config, "maincpu", 133000000));
	maincpu.set_addrmap(AS_PROGRAM, &gammagic_state::gammagic_map);
	maincpu.set_addrmap(AS_IO, &gammagic_state::gammagic_io);
	maincpu.set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
	maincpu.smiact().set("pci:00.0", FUNC(i82439hx_host_device::smi_act_w));

	PCI_ROOT(config, "pci", 0);
	// TODO: confirm size
	I82439HX(config, "pci:00.0", 0, "maincpu", 256*1024*1024);

	i82371sb_isa_device &isa(I82371SB_ISA(config, "pci:07.0", 0, "maincpu"));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);

	i82371sb_ide_device &ide(I82371SB_IDE(config, "pci:07.1", 0, "maincpu"));
	ide.irq_pri().set("pci:07.0", FUNC(i82371sb_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(i82371sb_isa_device::pc_mirq0_w));
	ide.subdevice<bus_master_ide_controller_device>("ide1")->slot(0).set_default_option("xm3301");
//  ide.subdevice<bus_master_ide_controller_device>("ide1")->slot(0).set_option_machine_config("xm3301", cdrom_config);
	ide.subdevice<bus_master_ide_controller_device>("ide2")->slot(0).set_default_option(nullptr);

	PCI_SLOT(config, "pci:1", pci_cards, 15, 0, 1, 2, 3, nullptr);
	PCI_SLOT(config, "pci:2", pci_cards, 16, 1, 2, 3, 0, "ess_solo1");
//  PCI_SLOT(config, "pci:3", pci_cards, 17, 2, 3, 0, 1, "voodoo");
	PCI_SLOT(config, "pci:4", pci_cards, 18, 3, 0, 1, 2, "oti64111");

	// FIXME: this should obviously map to above instead of direct PCI mount ...
	voodoo_2_pci_device &voodoo(VOODOO_2_PCI(config, "pci:11.0", 0, "maincpu", "voodoo_screen"));
	voodoo.set_fbmem(2);
	voodoo.set_tmumem(4, 4);
	voodoo.set_status_cycles(1000);

	// FIXME: ... and run in VGA passthru mode not define its own screen canvas
	screen_device &screen(SCREEN(config, "voodoo_screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57);
	screen.set_size(800, 262);
	screen.set_visarea(0, 512 - 1, 0, 240 - 1);
	screen.set_screen_update("pci:11.0", FUNC(voodoo_2_pci_device::screen_update));

	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "fdc37c93x", true).set_option_machine_config("fdc37c93x", smc_superio_config);
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

	rs232_port_device &serport0(RS232_PORT(config, "serport0", isa_com, nullptr));
	serport0.rxd_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::rxd1_w));
	serport0.dcd_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ndcd1_w));
	serport0.dsr_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ndsr1_w));
	serport0.ri_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::nri1_w));
	serport0.cts_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::rxd2_w));
	serport1.dcd_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ndcd2_w));
	serport1.dsr_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ndsr2_w));
	serport1.ri_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::nri2_w));
	serport1.cts_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ncts2_w));

	// TODO: unknown clock/type, at least '020 core, needs to be moved as a RS232 option
	cpu_device &subcpu(M68EC020(config, "v8000", 12_MHz_XTAL));
	subcpu.set_addrmap(AS_PROGRAM, &gammagic_state::v8000_map);
	// TODO: setchip mentions a 82510 as DUART & an ACRTC!?
}


ROM_START( gammagic )
	ROM_REGION32_LE(0x40000, "pci:07.0", 0)
	ROM_LOAD("m7s04.rom",   0, 0x40000, CRC(3689f5a9) SHA1(8daacdb0dc6783d2161680564ffe83ac2515f7ef))

	ROM_REGION(0x80000, "v8000", 0)
	ROM_LOAD16_BYTE("m0.bin", 0x00001, 0x40000, CRC(805ea951) SHA1(81974f1a0b4dfbfc743e97485dc113c270bd21a7))
	ROM_LOAD16_BYTE("m1.bin", 0x00000, 0x40000, CRC(a8ca8d37) SHA1(a4b50726dbe164d1cc4043f447a846448160f351))
	ROM_LOAD16_BYTE("m0-v4.bin", 0x00001, 0x40000, CRC(805ea951) SHA1(81974f1a0b4dfbfc743e97485dc113c270bd21a7))
	ROM_LOAD16_BYTE("m1-v4.bin", 0x00000, 0x40000, CRC(a8ca8d37) SHA1(a4b50726dbe164d1cc4043f447a846448160f351))

	ROM_REGION(0x10000, "setchip", 0)
	ROM_LOAD("clear_mem.bin", 0, 0x10000, CRC(7832028a) SHA1(a3d9f599ed7501fa7b2088c22d9666404b0e585e))

	DISK_REGION( "pci:07.1:ide1:0:xm3301" )
	DISK_IMAGE_READONLY( "gammagic", 0, SHA1(947650b13f87eea6608a32a1bae7dca19d911f15) )
ROM_END

ROM_START( 99bottles )
	ROM_REGION32_LE(0x40000, "pci:07.0", 0)
	ROM_LOAD("m7s04.rom",   0, 0x40000, CRC(3689f5a9) SHA1(8daacdb0dc6783d2161680564ffe83ac2515f7ef))

	// TODO: move to OTI card
	//ROM_LOAD("otivga_tx2953526.rom", 0x0000, 0x8000, CRC(916491af) SHA1(d64e3a43a035d70ace7a2d0603fc078f22d237e1))

	// assume same as gammagic for now
	ROM_REGION(0x80000, "v8000", 0)
	ROM_LOAD16_BYTE("m0.bin", 0x00001, 0x40000, CRC(805ea951) SHA1(81974f1a0b4dfbfc743e97485dc113c270bd21a7))
	ROM_LOAD16_BYTE("m1.bin", 0x00000, 0x40000, CRC(a8ca8d37) SHA1(a4b50726dbe164d1cc4043f447a846448160f351))
	ROM_LOAD16_BYTE("m0-v4.bin", 0x00001, 0x40000, CRC(805ea951) SHA1(81974f1a0b4dfbfc743e97485dc113c270bd21a7))
	ROM_LOAD16_BYTE("m1-v4.bin", 0x00000, 0x40000, CRC(a8ca8d37) SHA1(a4b50726dbe164d1cc4043f447a846448160f351))

	ROM_REGION(0x10000, "setchip", 0)
	ROM_LOAD("clear_mem.bin", 0, 0x10000, CRC(7832028a) SHA1(a3d9f599ed7501fa7b2088c22d9666404b0e585e))

	DISK_REGION( "pci:07.1:ide1:0:xm3301" )
	DISK_IMAGE_READONLY( "99bottles", 0, BAD_DUMP SHA1(0b874178c8dd3cfc451deb53dc7936dc4ad5a04f))
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/


GAME( 1999, gammagic,  0,        gammagic, gammagic, gammagic_state, empty_init, ROT0, "Bally Gaming Co.", "Game Magic",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, 99bottles, gammagic, gammagic, gammagic, gammagic_state, empty_init, ROT0, "Bally Gaming Co.", "99 Bottles of Beer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
