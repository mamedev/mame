// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

"Pull The Trigger"
developed by the Korean company "Digital Silkroad Co. Ltd." and distributed by Covielsa.

TODO:
- Needs bump to SiS651;
- At startup it needs a missing soft reset trigger;
- It then draw a basic Phoenix BIOS but afterwards it accesses a missing BAR I/O
  which craps out the flash memory somehow;
- In shutms11 HDD image just resets during loading;

===================================================================================================

Standard PC with:
-Elitegroup L4S5MG/651+ V5.0 motherboard.
-512MB RAM, DDR PC2100 (one SIMM).
-Wibu Systems WibuKey USB security dongle.
-GeForce 4 MX440 8X 64MB DDR TV-OUT video card.
-Exact CPU model and speed unknown.
-Samsung SP0411N/OMD 40GB HDD.

And an external PCB for inputs (guns, etc.) with:
-ATF1502AS
-AT89C51
-AT89C2051
-11.0592 MHz xtal near the AT89C2051
-11.0592 MHz xtal near the AT89C51
-4.9152 MHz xtal near the ATF1502AS.

Also, the machine allows players to keep their scores on military dogtag like cards with a SEEPROM,
named "DSR-PT1 MEMORY" (they were probably for sale).

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

class pulltrig_state : public driver_device
{
public:
	pulltrig_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ide_00_1(*this, "pci:00.1")
		, m_lpc_01_0(*this, "pci:01.0")
	{ }


	void pulltrig(machine_config &config);

private:
	required_device<pentium4_device> m_maincpu;
	required_device<sis5513_ide_device> m_ide_00_1;
	required_device<sis950_lpc_device> m_lpc_01_0;

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

void pulltrig_state::ite_superio_config(device_t *device)
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



void pulltrig_state::pulltrig(machine_config &config)
{
	PENTIUM4(config, m_maincpu, 100'000'000); // Exact CPU and frequency unknown
	m_maincpu->set_irq_acknowledge_callback("pci:01.0:pic_master", FUNC(pic8259_device::inta_cb));
//  m_maincpu->smiact().set("pci:00.0", FUNC(sis950_lpc_device::smi_act_w));

	// TODO: everything below needs upgrading to SiS651
	// TODO: unknown flash ROM type (wrong one)
	// Needs a $40000 sized ROM, not $80000
	AMD_29F400T(config, "flash");

	PCI_ROOT(config, "pci", 0);
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

	// TODO: 3 PCI slots, 1 AGP, whatever is CNR slot

	// confirmed IT8705F
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


ROM_START(pulltrig)
	ROM_REGION32_LE(0x80000, "flash", ROMREGION_ERASE00)
	ROM_LOAD("51402.bin",    0x00000, 0x40000, CRC(33bed7c0) SHA1(169374b6dac5bbba335e113a97ac34dc830c2599))
	ROM_COPY("flash",        0x00000, 0x40000, 0x40000 )

	ROM_REGION(0x50000, "io", 0)
	ROM_LOAD("at89c51.bin",  0x00000, 0x01000, NO_DUMP) // AT89C51, protected
	ROM_LOAD("at89c2051.u3", 0x10000, 0x04000, NO_DUMP) // 2 Kbytes internal ROM

	DISK_REGION("pci:00.1:ide1:0:hdd")
	DISK_IMAGE("pullthetrigger", 0, SHA1(438d57499e435e9e4e4a5980e9b1ca0df4f4ccca))
ROM_END

} // anonymous namespace


GAME(2003, pulltrig, 0, pulltrig, 0, pulltrig_state, empty_init, ROT0, "Digital Silkroad", "Pull The Trigger", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING)
