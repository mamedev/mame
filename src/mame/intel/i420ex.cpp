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
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pci/pci_slot.h"
//#include "bus/rs232/hlemouse.h"
//#include "bus/rs232/null_modem.h"
//#include "bus/rs232/rs232.h"
//#include "bus/rs232/sun_kbd.h"
//#include "bus/rs232/terminal.h"
#include "cpu/i386/i386.h"
#include "machine/at_keybc.h"
#include "machine/ds128x.h"
#include "machine/i82425ex_psc.h"
#include "machine/i82426ex_ib.h"
#include "machine/pci.h"
#include "sound/spkrdev.h"

#include "softlist.h"
#include "softlist_dev.h"
#include "speaker.h"

namespace {

class i420ex_state : public driver_device
{
public:
	i420ex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rtc(*this, "rtc")
		, m_speaker(*this, "speaker")
	{ }

	void i420ex(machine_config &config) ATTR_COLD;

protected:
	required_device<cpu_device> m_maincpu;
	required_device<ds12885_device> m_rtc;
	required_device<speaker_sound_device> m_speaker;

private:
	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


void i420ex_state::main_map(address_map &map)
{
	map.unmap_value_high();
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
	m_maincpu->set_irq_acknowledge_callback("ib:intc1", FUNC(pic8259_device::inta_cb));
//	m_maincpu->smiact().set("pci:05.0", FUNC(i82425ex_psc_device::smi_act_w));

	i82426ex_ib_device &ib(I82426EX_IB(config, "ib", XTAL(14'318'181), "keybc"));
	ib.intr().set_inputline("maincpu", INPUT_LINE_IRQ0);
	ib.spkr().set([this] (int state) { m_speaker->level_w(state); });
	ib.rtcale().set([this](u8 data) { m_rtc->address_w(data); });
	ib.rtccs_read().set([this]() { return m_rtc->data_r(); });
	ib.rtccs_write().set([this](u8 data) { m_rtc->data_w(data); });

	// TODO: config space not known
	// 05.0 is clearly host: it's what the BIOS addresses at startup
	PCI_ROOT(config, "pci", 0);
	I82425EX_PSC(config, "pci:05.0", 0, "maincpu", "ib", 64*1024*1024);

	// TODO: on proprietary riser
//	PCI_SLOT(config, "pci:1", pci_cards, 2,  0, 1, 2, 3, nullptr);
//	PCI_SLOT(config, "pci:2", pci_cards, 3,  1, 2, 3, 0, nullptr);

//	ISA16_SLOT(config, "board1", 0, "pci:07.0:isabus", isa_internal_devices, "i82091aa", true).set_option_machine_config("i82091aa", intel_superio_config);
//	ISA16_SLOT(config, "board2", 0, "pci:07.0:isabus", isa_internal_devices, "rtc", true).set_option_machine_config("rtc", rtc_config);
//	ISA16_SLOT(config, "board3", 0, "pci:07.0:isabus", isa_internal_devices, "kbd", true).set_option_machine_config("kbd", kbd_config);
	// most likely PCI version really (gd5434 can be ISA16, VL-Bus or PCI)
//	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "vga", true);
//	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

	// TODO: really DS12887
	DS12885(config, m_rtc);
	m_rtc->irq().set("ib:intc2", FUNC(pic8259_device::ir0_w));
	m_rtc->set_century_index(0x32);

	ps2_keyboard_controller_device &keybc(PS2_KEYBOARD_CONTROLLER(config, "keybc", XTAL(12'000'000)));
	// TODO: use its own BIOS
	//keybc.set_default_bios_tag("compaq");
//	keybc.hot_res().set(m_chipset, FUNC(vl82c420_device::kbrst_w));
//	keybc.gate_a20().set(m_chipset, FUNC(vl82c420_device::gatea20_w));
//	keybc.kbd_irq().set(m_chipset, FUNC(vl82c420_device::irq01_w));
	keybc.kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	keybc.kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));
//	keybc.aux_irq().set(m_chipset, FUNC(vl82c420_device::irq04_w));
	keybc.aux_clk().set("aux", FUNC(pc_kbdc_device::clock_write_from_mb));
	keybc.aux_data().set("aux", FUNC(pc_kbdc_device::data_write_from_mb));

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, "pcat"));
	pc_kbdc.out_clock_cb().set(keybc, FUNC(at_kbc_device_base::kbd_clk_w));
	pc_kbdc.out_data_cb().set(keybc, FUNC(at_kbc_device_base::kbd_data_w));

	pc_kbdc_device &aux_kbdc(PC_KBDC(config, "aux", ps2_mice, nullptr));
	aux_kbdc.out_clock_cb().set(keybc, FUNC(at_kbc_device_base::kbd_clk_w));
	aux_kbdc.out_data_cb().set(keybc, FUNC(at_kbc_device_base::kbd_data_w));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
}


// dump comes from flash updater, each has $80 bytes of header at top (ignoreable) then the actual dump
ROM_START( entrada )
	ROM_REGION32_LE( 0x20000, "pci:05.0", ROMREGION_ERASEFF )
	// AZ3 version
	ROM_LOAD( "03_az0.bio", 0x10000, 0x00080, CRC(e0d3ac40) SHA1(a315be210a6233bf5cd863fb4a6313cdcea26442) )
	ROM_CONTINUE(           0x10000, 0x10000 )

	// unknown, maybe a bank for the onboard UPI-C42 keyboard MCU?
	// doesn't decode as x86 code
	ROM_REGION( 0x10000, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "03_az0.bi1", 0x00000, 0x00080, CRC(3976fb38) SHA1(17159457ff8b7fa7ab0a97ccf71e5e5c8df0fe3e) )
	ROM_CONTINUE(           0x00000, 0x0c000 )
ROM_END

} // anonymous namespace


COMP( 1994, entrada, 0, 0,      i420ex, 0, i420ex_state, empty_init, "Intel", "Classic/PCI LP \"Entrada\" (Intel I420EX Aries chipset)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

