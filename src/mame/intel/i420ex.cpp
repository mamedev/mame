// license:BSD-3-Clause
// copyright-holders: Angelo Salese
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
- Remaining bits in chipset (SMI, pin mapper & pirqrc);
- Keyboard fails working in places (same as ls5amvp3, forgets to read at $60 for irq ack);
- CMOS memory sets System Time with illegal hour
  (workaround: fix it manually then "Exit Saving Changes");
- AZ08 / AZ07 BIOSes: goes "Auto Configuration Error" with PCI bus, propagate that if cards are
  hooked up;
- isa1:sb16_lle: DMA crashes with high DMA (unsupported?)
- serport0:logitech_mouse: fails freedos13 init on every odd boot;

**************************************************************************************************/

#include "emu.h"

#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pci/clgd543x_alpine.h"
#include "bus/pci/pci_slot.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "cpu/i386/i386.h"
#include "machine/at_keybc.h"
#include "machine/ds128x.h"
#include "machine/i82091aa.h"
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

	void entrada(machine_config &config) ATTR_COLD;
	void a486ap4(machine_config &config) ATTR_COLD;

protected:
	void i420ex(machine_config &config) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ds12885_device> m_rtc;
	required_device<speaker_sound_device> m_speaker;

private:
	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	static void intel_superio_config(device_t *device);
};


void i420ex_state::main_map(address_map &map)
{
	map.unmap_value_high();
}

void i420ex_state::main_io(address_map &map)
{
	map.unmap_value_high();
}

static void pc_isa_onboard(device_slot_interface &device)
{
	device.option_add_internal("superio", I82091AA);
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

void i420ex_state::intel_superio_config(device_t *device)
{
	i82091aa_device &aip = *downcast<i82091aa_device *>(device);

	aip.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
	aip.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
	aip.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
	aip.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
	aip.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
	aip.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));
}

void i420ex_state::i420ex(machine_config &config)
{
	// Socket 3 / PGA237
	// FSB 25/33 MHz
	I486DX4(config, m_maincpu, 33'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &i420ex_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &i420ex_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("ib:intc1", FUNC(pic8259_device::inta_cb));
//  m_maincpu->smiact().set("pci:05.0", FUNC(i82425ex_psc_device::smi_act_w));

	i82426ex_ib_device &ib(I82426EX_IB(config, "ib", XTAL(14'318'181), "maincpu", "keybc"));
	ib.intr().set_inputline("maincpu", INPUT_LINE_IRQ0);
	ib.spkr().set([this] (int state) { m_speaker->level_w(state); });
	ib.rtcale().set([this](u8 data) { m_rtc->address_w(data); });
	ib.rtccs_read().set([this]() { return m_rtc->data_r(); });
	ib.rtccs_write().set([this](u8 data) { m_rtc->data_w(data); });
	ib.cpurst().set([this] (int state) {
		if (state)
			machine().schedule_soft_reset();
	});

	// TODO: config space not known
	// 05.0 is clearly host: it's what the BIOS addresses at startup
	PCI_ROOT(config, "pci", 0);
	i82425ex_psc_device &psc(I82425EX_PSC(config, "pci:05.0", 0, "maincpu", "ib", 64*1024*1024));
	psc.ide1_irq_w().set("ib:intc2", FUNC(pic8259_device::ir6_w));
	psc.ide2_irq_w().set("ib:intc2", FUNC(pic8259_device::ir7_w));

	// TODO: really DS12887
	DS12885(config, m_rtc);
	m_rtc->irq().set("ib:intc2", FUNC(pic8259_device::ir0_w));
	m_rtc->set_century_index(0x32);

	ps2_keyboard_controller_device &keybc(PS2_KEYBOARD_CONTROLLER(config, "keybc", XTAL(12'000'000)));
	// TODO: use its own BIOS
	// Detection fails with Compaq
//  keybc.set_default_bios_tag("compaq");
	keybc.hot_res().set_inputline("maincpu", INPUT_LINE_RESET);
	keybc.gate_a20().set_inputline("maincpu", INPUT_LINE_A20);
	keybc.kbd_irq().set("ib:intc1", FUNC(pic8259_device::ir1_w));
	keybc.kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	keybc.kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));
	keybc.aux_irq().set("ib:intc2", FUNC(pic8259_device::ir4_w));
	keybc.aux_clk().set("aux", FUNC(pc_kbdc_device::clock_write_from_mb));
	keybc.aux_data().set("aux", FUNC(pc_kbdc_device::data_write_from_mb));

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, "ms_naturl"));
	pc_kbdc.out_clock_cb().set(keybc, FUNC(at_kbc_device_base::kbd_clk_w));
	pc_kbdc.out_data_cb().set(keybc, FUNC(at_kbc_device_base::kbd_data_w));

	pc_kbdc_device &aux_kbdc(PC_KBDC(config, "aux", ps2_mice, nullptr));
	aux_kbdc.out_clock_cb().set(keybc, FUNC(at_kbc_device_base::kbd_clk_w));
	aux_kbdc.out_data_cb().set(keybc, FUNC(at_kbc_device_base::kbd_data_w));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
}

void i420ex_state::entrada(machine_config &config)
{
	i420ex(config);

	GD5434_PCI(config, "pci:06.0", 0);

	// TODO: on proprietary PISA riser
	PCI_SLOT(config, "pci:1", pci_cards, 7,  0, 1, 2, 3, nullptr);
	PCI_SLOT(config, "pci:2", pci_cards, 8,  1, 2, 3, 0, nullptr);

	ISA16_SLOT(config, "board1", 0, "ib:isabus", pc_isa_onboard, "superio", true).set_option_machine_config("superio", intel_superio_config);
	ISA16_SLOT(config, "isa1",   0, "ib:isabus", pc_isa16_cards, nullptr, false);

	rs232_port_device& serport0(RS232_PORT(config, "serport0", isa_com, "logitech_mouse"));
	serport0.rxd_handler().set("board1:superio", FUNC(i82091aa_device::rxd1_w));
	serport0.dcd_handler().set("board1:superio", FUNC(i82091aa_device::ndcd1_w));
	serport0.dsr_handler().set("board1:superio", FUNC(i82091aa_device::ndsr1_w));
	serport0.ri_handler().set("board1:superio", FUNC(i82091aa_device::nri1_w));
	serport0.cts_handler().set("board1:superio", FUNC(i82091aa_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("board1:superio", FUNC(i82091aa_device::rxd2_w));
	serport1.dcd_handler().set("board1:superio", FUNC(i82091aa_device::ndcd2_w));
	serport1.dsr_handler().set("board1:superio", FUNC(i82091aa_device::ndsr2_w));
	serport1.ri_handler().set("board1:superio", FUNC(i82091aa_device::nri2_w));
	serport1.cts_handler().set("board1:superio", FUNC(i82091aa_device::ncts2_w));
}

void i420ex_state::a486ap4(machine_config &config)
{
	i420ex(config);

	PCI_SLOT(config, "pci:1", pci_cards, 7,  0, 1, 2, 3, nullptr);
	PCI_SLOT(config, "pci:2", pci_cards, 8,  1, 2, 3, 0, nullptr);
	PCI_SLOT(config, "pci:3", pci_cards, 9,  2, 3, 0, 1, nullptr);
	PCI_SLOT(config, "pci:4", pci_cards, 10, 3, 0, 1, 2, nullptr);

	ISA16_SLOT(config, "isa1",   0, "ib:isabus", pc_isa16_cards, "fdc_smc", false);
	ISA16_SLOT(config, "isa2",   0, "ib:isabus", pc_isa16_cards, "comat", false);
	ISA16_SLOT(config, "isa3",   0, "ib:isabus", pc_isa16_cards, "lpt", false);
	ISA16_SLOT(config, "isa4",   0, "ib:isabus", pc_isa16_cards, nullptr, false);
	// TODO: VLB really
	ISA16_SLOT(config, "isa5",   0, "ib:isabus", pc_isa16_cards, "svga_et4k", false);
}


// dump comes from flash updater, each has $80 bytes of header at top (ignoreable) then the actual dump
// .bi1 is eventually copied by BIOS at ISA state $ed (compressed x86 code?)
ROM_START( entrada )
	ROM_REGION32_LE( 0x20000, "pci:05.0", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "az08", "1.00.08.AZ0")
	ROMX_LOAD( "1008az0_.bi1", 0x00000, 0x00080, CRC(c8d22934) SHA1(bd8e9387130d493246dac4bb89bd9aa502721033), ROM_BIOS(0) )
	ROM_CONTINUE(              0x00000, 0x0c000 )
	ROMX_LOAD( "1008az0_.bio", 0x10000, 0x00080, CRC(23fdaa9c) SHA1(0d2c9eeb52d300b5e39cb47d0b498c902b896ed7), ROM_BIOS(0) )
	ROM_CONTINUE(              0x10000, 0x10000 )
	ROM_SYSTEM_BIOS(1, "az07", "1.00.07.AZ0")
	ROMX_LOAD( "1007az0_.bi1", 0x00000, 0x00080, CRC(d692eac6) SHA1(3d65ac556a3b86461abcc29579d038f2b1e864e0), ROM_BIOS(1) )
	ROM_CONTINUE(              0x00000, 0x0c000 )
	ROMX_LOAD( "1007az0_.bio", 0x10000, 0x00080, CRC(41ff4579) SHA1(f6c40d8fa6ddf57a3efe9c77a75fa4076bf0e0e8), ROM_BIOS(1) )
	ROM_CONTINUE(              0x10000, 0x10000 )
	ROM_SYSTEM_BIOS(2, "az05", "1.00.05.AZ0")
	ROMX_LOAD( "1005az0_.bi1", 0x00000, 0x00080, CRC(91287697) SHA1(682712ff12a8ddb1172b879cfdfe79ec466430ca), ROM_BIOS(2) )
	ROM_CONTINUE(              0x00000, 0x0c000 )
	ROMX_LOAD( "1005az0_.bio", 0x10000, 0x00080, CRC(3212d818) SHA1(7b2aacf76263f4aa2e14d20a44c10cf5dcb198cd), ROM_BIOS(2) )
	ROM_CONTINUE(              0x10000, 0x10000 )
	ROM_SYSTEM_BIOS(3, "az03", "1.00.02.AZ0")
	ROMX_LOAD( "03_az0.bi1",   0x00000, 0x00080, CRC(3976fb38) SHA1(17159457ff8b7fa7ab0a97ccf71e5e5c8df0fe3e), ROM_BIOS(3) )
	ROM_CONTINUE(              0x00000, 0x0c000 )
	ROMX_LOAD( "03_az0.bio",   0x10000, 0x00080, CRC(e0d3ac40) SHA1(a315be210a6233bf5cd863fb4a6313cdcea26442), ROM_BIOS(3) )
	ROM_CONTINUE(              0x10000, 0x10000 )
	// AZ02 known to exist
ROM_END

// ASUS PVI-486AP4 (Socket 3, 4xSIMM72, Cache: 128/256/512KB, 4 PCI, 4 ISA, 1 VLB)
// Intel Aries PCIset S82425EX + S82426EX; DS12887 RTC; VIA VT82C42N - BIOS: 32pin
ROM_START( a486ap4 )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	// 0: BIOS-String: 07/20/94-ARIES-P/I-AP4G-00 / #401A0-0104
	ROM_SYSTEM_BIOS(0, "486ap4v104", "ASUS PVI-486AP4 V1.04")
	ROMX_LOAD( "awai0104.bin", 0x00000, 0x20000, CRC(52ea7123) SHA1(3d242ea6d1bcdddd41e32e40708133c72f2bd060), ROM_BIOS(0))
	// 1: BIOS-String: 10/21/94-ARIES-P/I-AP4G-00 / #401A0-0203
	ROM_SYSTEM_BIOS(1, "486ap4v203", "ASUS PVI-486AP4 V2.03")
	ROMX_LOAD( "awai0203.bin", 0x00000, 0x20000, CRC(68d3a3f4) SHA1(6eee0c9aed2ede028eb170f8dd7921563293b99f), ROM_BIOS(1))
	// 2: BIOS-String: 11/08/94-ARIES-P/I-AP4G-00 / #401A0-0204
	ROM_SYSTEM_BIOS(2, "486ap4v204", "ASUS PVI-486AP4 V2.04")
	ROMX_LOAD( "awai0204.bin", 0x00000, 0x20000, CRC(b62b35bb) SHA1(b6fa3d7b1c88da37ce74aca329a31d2587652d97), ROM_BIOS(2))
	// 3: BIOS-String: 11/25/97/ARIES-P/I-AP4G-00 / #401A0-0205-2
	ROM_SYSTEM_BIOS(3, "486ap4v205-2", "ASUS PVI-486AP4 V2.05-2")
	ROMX_LOAD( "0205.002", 0x00000, 0x20000, CRC(632e8ee6) SHA1(3cf57b2654b0365e41ef5f5c82f68eeadf0e7a21), ROM_BIOS(3))
ROM_END


} // anonymous namespace


COMP( 1994, entrada, 0, 0,      entrada, 0, i420ex_state, empty_init, "Intel", "Classic/PCI LP \"Entrada\" (Intel I420EX Aries chipset)", MACHINE_NOT_WORKING )
COMP( 1994, a486ap4, 0, 0,      a486ap4, 0, i420ex_state, empty_init, "Asus",  "PVI-486AP4 (Intel I420EX Aries chipset)", MACHINE_NOT_WORKING )

