// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*
 * Sandbox for VIA based PCs
 *
 * Using Lucky Star 5AMVP3 given we already have a mostly compatible Super I/O (ITE 8661F)
 * Eventually needs to be backported to misc/startouch.cpp
 *
 */

#include "emu.h"

#include "bus/isa/isa_cards.h"
#include "bus/pci/pci_slot.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "cpu/i386/i386.h"
#include "machine/it8671f.h"
#include "machine/pci.h"
#include "machine/vt82c586b_isa.h"
#include "machine/vt82c598mvp.h"
//#include "machine/w83787f.h"
//#include "video/voodoo_pci.h"

#include "softlist.h"
#include "softlist_dev.h"

namespace {

class mvp3_state : public driver_device
{
public:
	mvp3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void mvp3(machine_config &config);

protected:
	// TODO: this binding should be more basic
	required_device<pentium_mmx_device> m_maincpu;

private:
	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	static void ite_superio_config(device_t *device);
};

void mvp3_state::main_map(address_map &map)
{
	map.unmap_value_high();
}

void mvp3_state::main_io(address_map &map)
{
	map.unmap_value_high();
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

static void isa_internal_devices(device_slot_interface &device)
{
	// TODO: downgrade to ITE IT8661F
	device.option_add("it8671f", IT8671F);
}

void mvp3_state::ite_superio_config(device_t *device)
{
	it8671f_device &ite = *downcast<it8671f_device *>(device);
	ite.krst_gpio2().set_inputline(":maincpu", INPUT_LINE_RESET);
	ite.ga20_gpio6().set_inputline(":maincpu", INPUT_LINE_A20);
//	ite.irq1().set(":pci:07.0", FUNC(i82371eb_isa_device::pc_irq1_w));
//	ite.irq8().set(":pci:07.0", FUNC(i82371eb_isa_device::pc_irq8n_w));
	ite.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
	ite.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
	ite.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
	ite.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
	ite.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
	ite.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));
}

void mvp3_state::mvp3(machine_config &config)
{
	// Socket 7 / PGA321
	PENTIUM_MMX(config, m_maincpu, 66'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mvp3_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &mvp3_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("pci:07.0:pic0", FUNC(pic8259_device::inta_cb));

	// TODO: config space not known
	PCI_ROOT(config, "pci", 0);
	// Max 768 MB
	VT82C598MVP_HOST(config, "pci:00.0", 0, "maincpu", 256*1024*1024);

	vt82c586b_isa_device &isa(VT82C586B_ISA(config, "pci:07.0", 0, m_maincpu));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
//	isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);

	// TODO: IDE, USB, ACPI

	// TODO: 1 AGP slot, 4 PCI slots

	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "it8671f", true).set_option_machine_config("it8671f", ite_superio_config);
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

	rs232_port_device &serport0(RS232_PORT(config, "serport0", isa_com, "logitech_mouse"));
	serport0.rxd_handler().set("board4:it8671f", FUNC(it8671f_device::rxd1_w));
	serport0.dcd_handler().set("board4:it8671f", FUNC(it8671f_device::ndcd1_w));
	serport0.dsr_handler().set("board4:it8671f", FUNC(it8671f_device::ndsr1_w));
	serport0.ri_handler().set("board4:it8671f", FUNC(it8671f_device::nri1_w));
	serport0.cts_handler().set("board4:it8671f", FUNC(it8671f_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("board4:it8671f", FUNC(it8671f_device::rxd2_w));
	serport1.dcd_handler().set("board4:it8671f", FUNC(it8671f_device::ndcd2_w));
	serport1.dsr_handler().set("board4:it8671f", FUNC(it8671f_device::ndsr2_w));
	serport1.ri_handler().set("board4:it8671f", FUNC(it8671f_device::nri2_w));
	serport1.cts_handler().set("board4:it8671f", FUNC(it8671f_device::ncts2_w));

	SOFTWARE_LIST(config, "pc_disk_list").set_original("ibm5150");
	SOFTWARE_LIST(config, "at_disk_list").set_original("ibm5170");
	SOFTWARE_LIST(config, "at_cdrom_list").set_original("ibm5170_cdrom");
	SOFTWARE_LIST(config, "win_cdrom_list").set_original("generic_cdrom").set_filter("ibmpc");
	SOFTWARE_LIST(config, "at_hdd_list").set_original("ibm5170_hdd");
	SOFTWARE_LIST(config, "midi_disk_list").set_compatible("midi_flop");
	SOFTWARE_LIST(config, "photocd_list").set_compatible("photo_cd");
}

ROM_START( ls5amvp3 )
	ROM_REGION32_LE(0x20000, "pci:07.0", 0)
	ROM_SYSTEM_BIOS(0, "lec90", "LEC9-00")
	ROMX_LOAD( "lec9-0.bin", 0x00000, 0x20000, CRC(7e3a18e5) SHA1(e178eb1ff2a1257f7daf9a32ba4c05739a445568), ROM_BIOS(0))
ROM_END

} // anonymous namespace

COMP(1998, ls5amvp3,    0,     0, mvp3,   0, mvp3_state, empty_init, "Lucky Star", "5AMVP3 (VIA MVP3 chipset)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

