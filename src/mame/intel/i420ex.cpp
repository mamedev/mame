// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Intel 420EX PCIset EX "Aries" x86 based HW

- northbridge 82425EX "PSC"
- southbridge 82426EX "IB"
- Intel UPI-C42 keyboard MCU
- Dallas DS12887
- Super I/O 82091AA "AIP" (same as BeBox)
- Onboard Cirrus Logic CL-GD5434
- max RAM 64MB;
- Proprietary PISA riser slot for 1x ISA + 2 PCI slots;

TODO:
- skeleton;

**************************************************************************************************/

#include "emu.h"

#include "bus/isa/isa_cards.h"
#include "bus/pci/pci_slot.h"
//#include "bus/rs232/hlemouse.h"
//#include "bus/rs232/null_modem.h"
//#include "bus/rs232/rs232.h"
//#include "bus/rs232/sun_kbd.h"
//#include "bus/rs232/terminal.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"

#include "softlist.h"
#include "softlist_dev.h"

namespace {

class i420ex_state : public driver_device
{
public:
	i420ex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void i420ex(machine_config &config) ATTR_COLD;

protected:
	required_device<cpu_device> m_maincpu;

private:
	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


void i420ex_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000'0000, 0x0009'ffff).ram();
	map(0x000f'0000, 0x000f'ffff).rom().region("bios", 0);
	map(0xffff'0000, 0xffff'ffff).rom().region("bios", 0);
}

void i420ex_state::main_io(address_map &map)
{
	map.unmap_value_high();
}

void i420ex_state::i420ex(machine_config &config)
{
	// Socket 3 / PGA237
	// FSB 25/33 MHz
	I486DX4(config, m_maincpu, 33'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &i420ex_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &i420ex_state::main_io);
//	m_maincpu->set_irq_acknowledge_callback("pci:07.0:pic0", FUNC(pic8259_device::inta_cb));
//	m_maincpu->smiact().set("pci:00.0", FUNC(vt82c598mvp_host_device::smi_act_w));

	// TODO: config space not known
	PCI_ROOT(config, "pci", 0);

	// TODO: on proprietary riser
//	PCI_SLOT(config, "pci:1", pci_cards, 8,  0, 1, 2, 3, nullptr);
//	PCI_SLOT(config, "pci:2", pci_cards, 9,  1, 2, 3, 0, nullptr);

//	ISA16_SLOT(config, "board1", 0, "pci:07.0:isabus", isa_internal_devices, "i82091aa", true).set_option_machine_config("i82091aa", intel_superio_config);
//	ISA16_SLOT(config, "board2", 0, "pci:07.0:isabus", isa_internal_devices, "rtc", true).set_option_machine_config("rtc", rtc_config);
//	ISA16_SLOT(config, "board3", 0, "pci:07.0:isabus", isa_internal_devices, "kbd", true).set_option_machine_config("kbd", kbd_config);
	// most likely PCI version really (gd5434 can be ISA16, VL-Bus or PCI)
//	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "vga", true);
//	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
}


// dump comes from flash updater, each has $80 bytes of header at top (ignoreable) then the actual dump
ROM_START( entrada )
	ROM_REGION32_LE( 0x10000, "bios", ROMREGION_ERASEFF )
	// AZ3 version
	ROM_LOAD( "03_az0.bio", 0x00000, 0x00080, CRC(e0d3ac40) SHA1(a315be210a6233bf5cd863fb4a6313cdcea26442) )
	ROM_CONTINUE(           0x00000, 0x10000 )

	// unknown, maybe a bank for the onboard UPI-C42 keyboard MCU?
	// doesn't decode as x86 code
	ROM_REGION( 0x10000, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "03_az0.bi1", 0x00000, 0x00080, CRC(3976fb38) SHA1(17159457ff8b7fa7ab0a97ccf71e5e5c8df0fe3e) )
	ROM_CONTINUE(           0x00000, 0x0c000 )
ROM_END

} // anonymous namespace


COMP( 1994, entrada, 0, 0,      i420ex, 0, i420ex_state, empty_init, "Intel", "Classic/PCI LP \"Entrada\" (Intel I420EX Aries chipset)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

