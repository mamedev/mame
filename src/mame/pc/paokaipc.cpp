// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Go Go Strike (c) 2007? Paokai

Some kind of x86 pc-like hardware, exact CPU type etc. unknown hardware is by Paokai,
motherboard has logos, large chip with logo too, http://www.paokai.com.tw/

Update:
- BIOS has a FIC PT-2200 reference, using that as base;
- CF image loads a SiS55X driver, for SiS315 AGP x4 + SiS7019 sound card?
- eGalax Inc. USB Touchscreen, with following possible device IDs:
Device = 0x0123, 0x0001, insmod /driver/tkusb.o  vidlist="0123 3823 3823 0EEF 0EEF" pidlist="0001 0001 0002 0001 0002";  /driver/tpaneld &
Device = 0x3823, 0x0001, insmod /driver/tkusb.o  vidlist="0123 3823 3823 0EEF 0EEF" pidlist="0001 0001 0002 0001 0002";  /driver/tpaneld &
Device = 0x3823, 0x0002, insmod /driver/tkusb.o  vidlist="0123 3823 3823 0EEF 0EEF" pidlist="0001 0001 0002 0001 0002";  /driver/tpaneld &
Device = 0x0EEF, 0x0001, insmod /driver/tkusb.o  vidlist="0123 3823 3823 0EEF 0EEF" pidlist="0001 0001 0002 0001 0002";  /driver/tpaneld &
Device = 0x0EEF, 0x0002, insmod /driver/tkusb.o  vidlist="0123 3823 3823 0EEF 0EEF" pidlist="0001 0001 0002 0001 0002";  /driver/tpaneld &
- An unspecified "w040101" driver;

CF card has a Linux partition, partially bootable with shutms11 driver.
- starts with a "LILO boot", loads a proprietary driver TWDrv.o (3M Touch Systems TouchWare)
  then eventually crashes not finding sound modules;
- Shows being a "gcc 3.2.2 (Red Hat Linux 3.2.2-5)" distro.
  Notice that latter seems mislabeled, and RedHat is actually version 9
  http://rpm.pbone.net/info_idpl_19558085_distro_redhat9_com_gcc-3.2.2-5.i386.rpm.html

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
#include "machine/8042kbdc.h"
#include "machine/ds128x.h"
#include "machine/i82371sb.h"
#include "machine/i82439hx.h"
#include "machine/pci.h"
#include "machine/pc97338.h"

/*
 * VT82C416
 */
// TODO: move to common file, improve (clearly uses ports $8xx)
// Handle RTC, PS/2 keyboard and PnP
// normally paired with VT82C570M "Apollo Master"

class vt82c416_device : public device_t, public device_isa16_card_interface
{
public:
	// construction/destruction
	vt82c416_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<ds12885ext_device> m_rtc;
	required_device<kbdc8042_device> m_kbdc;

	void remap(int space_id, offs_t start, offs_t end) override;

	void device_map(address_map &map) ATTR_COLD;
	void pnp_map(address_map &map) ATTR_COLD;
};

DEFINE_DEVICE_TYPE(VT82C416, vt82c416_device, "vt82c416", "VT82C416 X-Bus Controller")

vt82c416_device::vt82c416_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VT82C416, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_rtc(*this, "rtc")
	, m_kbdc(*this, "kbdc")
{
}

void vt82c416_device::device_add_mconfig(machine_config &config)
{
	DS12885EXT(config, m_rtc, 32.768_kHz_XTAL);
	//m_rtc->irq().set(m_pic8259_2, FUNC(pic8259_device::ir0_w));
	m_rtc->set_century_index(0x32);

	// TODO: clearly has issues, particularly with A20
	// Assume it uses a VT82C42
	KBDC8042(config, m_kbdc, 0);
	m_kbdc->set_keyboard_type(kbdc8042_device::KBDC8042_STANDARD);
	m_kbdc->system_reset_callback().set_inputline(":maincpu", INPUT_LINE_RESET);
	m_kbdc->gate_a20_callback().set_inputline(":maincpu", INPUT_LINE_A20);
	m_kbdc->input_buffer_full_callback().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq1_w));
	m_kbdc->set_keyboard_tag("at_keyboard");

	at_keyboard_device &at_keyb(AT_KEYB(config, "at_keyboard", pc_keyboard_device::KEYBOARD_TYPE::AT, 1));
	at_keyb.keypress().set(m_kbdc, FUNC(kbdc8042_device::keyboard_w));
#
}


void vt82c416_device::device_start()
{
	set_isa_device();
}

void vt82c416_device::device_reset()
{

}

void vt82c416_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_isa->install_device(0x060, 0x07f, *this, &vt82c416_device::device_map);
		m_isa->install_device(0x800, 0x8ff, *this, &vt82c416_device::pnp_map);
	}
}

void vt82c416_device::device_map(address_map &map)
{
	map(0x00, 0x0f).rw(m_kbdc, FUNC(kbdc8042_device::data_r), FUNC(kbdc8042_device::data_w));
	map(0x10, 0x17).w(m_rtc, FUNC(ds12885ext_device::address_w)).umask32(0x00ff00ff);
	map(0x10, 0x17).rw(m_rtc, FUNC(ds12885ext_device::data_r), FUNC(ds12885ext_device::data_w)).umask32(0xff00ff00);
}

void vt82c416_device::pnp_map(address_map &map)
{
	// checks for bit 3, unhappy if high
	map(0x80, 0x80).lr8(NAME([] () {return 0; }));
	map(0x88, 0x88).lr8(NAME([] () {return 0; }));
}

namespace {

class paokaipc_state : public driver_device
{
public:
	paokaipc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void paokaipc(machine_config &config);

private:
	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	static void superio_config(device_t *device);
};

void paokaipc_state::main_map(address_map &map)
{
	map.unmap_value_high();
}

void paokaipc_state::main_io(address_map &map)
{
	map.unmap_value_high();
}

static void isa_internal_devices(device_slot_interface &device)
{
	device.option_add("superio", PC97338);
	device.option_add("xbus", VT82C416);
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

void paokaipc_state::superio_config(device_t *device)
{
	// TODO: National PC87336
	pc97338_device &fdc = *downcast<pc97338_device *>(device);
//  fdc.set_sysopt_pin(1);
//  fdc.gp20_reset().set_inputline(":maincpu", INPUT_LINE_RESET);
//  fdc.gp25_gatea20().set(":pci:07.0", FUNC(i82371sb_isa_device::a20gate_w));
	fdc.irq1().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq1_w));
	fdc.irq8().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq8n_w));
	fdc.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
	fdc.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
	fdc.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
	fdc.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
	fdc.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
	fdc.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));
}

void paokaipc_state::paokaipc(machine_config &config)
{
	// Socket 7 / PGA321
	pentium_device &maincpu(PENTIUM(config, "maincpu", 90000000));
	maincpu.set_addrmap(AS_PROGRAM, &paokaipc_state::main_map);
	maincpu.set_addrmap(AS_IO, &paokaipc_state::main_io);
	maincpu.set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
	maincpu.smiact().set("pci:00.0", FUNC(i82439hx_host_device::smi_act_w));

	PCI_ROOT(config, "pci", 0);
	I82439HX(config, "pci:00.0", 0, "maincpu", 256*1024*1024);

	i82371sb_isa_device &isa(I82371SB_ISA(config, "pci:07.0", 0, "maincpu"));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);

	i82371sb_ide_device &ide(I82371SB_IDE(config, "pci:07.1", 0, "maincpu"));
	ide.subdevice<bus_master_ide_controller_device>("ide1")->slot(0).set_default_option("cf");
	ide.irq_pri().set("pci:07.0", FUNC(i82371sb_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(i82371sb_isa_device::pc_mirq0_w));

	PCI_SLOT(config, "pci:1", pci_cards, 15, 0, 1, 2, 3, nullptr);
	PCI_SLOT(config, "pci:2", pci_cards, 16, 1, 2, 3, 0, nullptr);
	PCI_SLOT(config, "pci:3", pci_cards, 17, 2, 3, 0, 1, nullptr);
	PCI_SLOT(config, "pci:4", pci_cards, 18, 3, 0, 1, 2, "virge");

	ISA16_SLOT(config, "board3", 0, "pci:07.0:isabus", isa_internal_devices, "xbus", true);
	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "superio", true).set_option_machine_config("superio", superio_config);
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

	rs232_port_device &serport0(RS232_PORT(config, "serport0", isa_com, nullptr));
	serport0.rxd_handler().set("board4:superio", FUNC(pc97338_device::rxd1_w));
	serport0.dcd_handler().set("board4:superio", FUNC(pc97338_device::ndcd1_w));
	serport0.dsr_handler().set("board4:superio", FUNC(pc97338_device::ndsr1_w));
	serport0.ri_handler().set("board4:superio", FUNC(pc97338_device::nri1_w));
	serport0.cts_handler().set("board4:superio", FUNC(pc97338_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("board4:superio", FUNC(pc97338_device::rxd2_w));
	serport1.dcd_handler().set("board4:superio", FUNC(pc97338_device::ndcd2_w));
	serport1.dsr_handler().set("board4:superio", FUNC(pc97338_device::ndsr2_w));
	serport1.ri_handler().set("board4:superio", FUNC(pc97338_device::nri2_w));
	serport1.cts_handler().set("board4:superio", FUNC(pc97338_device::ncts2_w));
}

ROM_START( gogostrk )
	ROM_REGION32_LE( 0x40000, "pci:07.0", 0 )
	ROM_LOAD( "39sf020a.rom1", 0x000000, 0x040000, CRC(236d4d95) SHA1(50579acddc93c05d5f8e17ad3669a29d2dc49965) )

	DISK_REGION( "pci:07.1:ide1:0:cf" )    // 128 MB CF Card
	DISK_IMAGE( "ggs-5-2-07", 0,SHA1(f214fd39ec8ac02f008823f4b179ea6c6835e1b8) )
ROM_END

} // anonymous namespace

GAME( 2007?, gogostrk, 0, paokaipc, 0, paokaipc_state, empty_init, ROT0, "American Alpha / Paokai", "Go Go Strike", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // motherboard is dated 2006, if the CF card string is a date it's 2007
