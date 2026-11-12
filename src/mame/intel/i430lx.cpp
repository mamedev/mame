// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Intel 430LX PCIset "Mercury"
Intel 430NX PCIset "Neptune"
Intel 430NX PCIset "Neptune MP"

Gigabyte GA-586IP config:
- Intel 82434NX (PCMC) as northbridge
- Intel 82433NX (LBX) local bus accelerator
- Intel 82378ZB (SIO) as southbridge
- Texas Instruments BENCHMARQ bq3287AMT RTC (ds12885 clone? Wants at least 128 bytes of SRAM)
- No super I/O for keyboard/RTC, those are southbridge responsibility under X-Bus
- NCR53C810 SCSI controller (concealed under a Price Tag style on Retro Web picture)

Regular SIO is reused by earlier I420ZX "Saturn II" chipset and in BeBox
"Mercury" 82433LX/82434LX are earlier revisions of the northbridge
"Neptune MP" variants uses an 82379AB (SIO.A), which maps an extra APIC

TODO:
- Remaining X-Bus peripherals for SIO (FDC, COM x2, LPT);
- Implement remaining features in north/southbridges (SMI and PIRQ not enabled by these BIOSes,
  needs to be tested in a Windows install);
- ga586ip: loads a NCRPCI-3.06.00 BIOS module if it finds a 53C810 SCSI card, hangs there just
  like ncr53c825;

**************************************************************************************************/

#include "emu.h"

#include "bus/ata/ataintf.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pci/et4kw32p.h"
#include "bus/pci/pci_slot.h"
//#include "bus/rs232/hlemouse.h"
//#include "bus/rs232/null_modem.h"
//#include "bus/rs232/rs232.h"
//#include "bus/rs232/sun_kbd.h"
//#include "bus/rs232/terminal.h"
#include "cpu/i386/i386.h"
#include "machine/at_keybc.h"
#include "machine/ds128x.h"
#include "machine/idectrl.h"
#include "machine/intelfsh.h"
#include "machine/i82378zb_sio.h"
#include "machine/i82434lx_pcmc.h"
#include "machine/pci.h"
#include "sound/spkrdev.h"

#include "softlist_dev.h"

namespace {

class i430lx_state : public driver_device
{
public:
	i430lx_state(const machine_config &mconfig, device_type type, const char *tag)
		: i430lx_state(mconfig, type, tag, "pci:02.0:xbus_flash", "pci:02.0:xbus_keybc")
	{ }

	void i430nx(machine_config &config) ATTR_COLD;
	void sy029c2(machine_config &config) ATTR_COLD;

protected:
	i430lx_state(const machine_config &mconfig, device_type type, const char *tag, const char *flash_tag, const char *keybc_tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rtc(*this, "rtc")
		, m_flash(*this, flash_tag)
		, m_keybc(*this, keybc_tag)
		, m_at_con(*this, "at_con")
//      , m_ide(*this, "pci:02.0:xbus_ide%u", 0U)
	{ }

//  void i430lx(machine_config &config) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ds12885_device> m_rtc;
	required_device<intelfsh8_device> m_flash;
	required_device<at_keyboard_controller_device> m_keybc;
	required_device<pc_kbdc_device> m_at_con;
	//required_device_array<ide_controller_32_device, 2> m_ide;

	void x86_softlists(machine_config &config);
	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};

class pcd5h_state : public i430lx_state
{
public:
	pcd5h_state(const machine_config &mconfig, device_type type, const char *tag)
		: i430lx_state(mconfig, type, tag, "pci:01.0:xbus_flash", "pci:01.0:xbus_keybc")
	{ }

	void pcd5h(machine_config &config) ATTR_COLD;
};


void i430lx_state::main_map(address_map &map)
{
	map.unmap_value_high();
}

void i430lx_state::main_io(address_map &map)
{
	map.unmap_value_high();
}

void i430lx_state::x86_softlists(machine_config &config)
{
	SOFTWARE_LIST(config, "pc_disk_list").set_original("ibm5150");
	SOFTWARE_LIST(config, "at_disk_list").set_original("ibm5170");
	SOFTWARE_LIST(config, "at_cdrom_list").set_original("ibm5170_cdrom");
	SOFTWARE_LIST(config, "win_cdrom_list").set_original("generic_cdrom").set_filter("ibmpc");
	SOFTWARE_LIST(config, "at_hdd_list").set_original("ibm5170_hdd");
	SOFTWARE_LIST(config, "midi_disk_list").set_compatible("midi_flop");
	SOFTWARE_LIST(config, "photocd_list").set_compatible("photo_cd");
}


void i430lx_state::i430nx(machine_config &config)
{
	// Socket 5 / PGA320
	// FSB 60/66 MHz
	PENTIUM(config, m_maincpu, 66'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &i430lx_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &i430lx_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("pci:02.0:pic0", FUNC(pic8259_device::inta_cb));
//  m_maincpu->smiact().set("pci:00.0", FUNC(i82434nx_pcmc_device::smi_act_w));

	// Texas Instruments BENCHMARQ bq3287AMT RTC
	DS12885(config, m_rtc, XTAL(32'768));
	m_rtc->set_binary(true);
	// TODO: alarm irq
//  m_rtc->irq().set ...

	// config space not verified, but should be good given how BIOSes accesses at $cxxx
	PCI_ROOT(config, "pci");
	// max RAM 512MB
	I82434NX_PCMC(config, "pci:00.0", "maincpu", 64*1024*1024);

	i82378zb_sio_device &isa(I82378ZB_SIO(config, "pci:02.0", 0, "maincpu"));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.a20m().set_inputline("maincpu", INPUT_LINE_A20);
	isa.cpureset().set_inputline("maincpu", INPUT_LINE_RESET);
	isa.rtcale().set([this](u8 data) { m_rtc->address_w(data); });
	isa.rtccs_read().set([this]() { return m_rtc->data_r(); });
	isa.rtccs_write().set([this](u8 data) { m_rtc->data_w(data); });

	// X-Bus devices
	// TODO: unknown flash type, just to get a 0x20000 sized one
	AMD_29F010(config, m_flash);

	AT_KEYBOARD_CONTROLLER(config, m_keybc, XTAL(12'000'000));
	m_keybc->hot_res().set("pci:02.0", FUNC(i82378zb_sio_device::cpu_reset_w));
	m_keybc->gate_a20().set("pci:02.0", FUNC(i82378zb_sio_device::cpu_a20_w));
	m_keybc->kbd_irq().set("pci:02.0", FUNC(i82378zb_sio_device::pc_irq1_w));
	m_keybc->kbd_clk().set(m_at_con, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_keybc->kbd_data().set(m_at_con, FUNC(pc_kbdc_device::data_write_from_mb));

	PC_KBDC(config, m_at_con, pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL);
	m_at_con->out_clock_cb().set(m_keybc, FUNC(at_keyboard_controller_device::kbd_clk_w));
	m_at_con->out_data_cb().set(m_keybc, FUNC(at_keyboard_controller_device::kbd_data_w));

//  IDE_CONTROLLER_32(config, m_ide[0]).options(ata_devices, "hdd", nullptr, false);
//  m_ide[0]->irq_handler().set("pci:02.0", FUNC(i82378zb_sio_device::pc_irq14_w));
//
//  IDE_CONTROLLER_32(config, m_ide[1]).options(ata_devices, "cdrom", nullptr, false);
//  m_ide[1]->irq_handler().set("pci:02.0", FUNC(i82378zb_sio_device::pc_irq15_w));

	// 1x AT keyboard
	// 4x ISA slots
	ISA16_SLOT(config, "isa1", 0, "pci:02.0:isabus", pc_isa16_cards, "fdc_smc", false);
	ISA16_SLOT(config, "isa2", 0, "pci:02.0:isabus", pc_isa16_cards, "ide", false);
	ISA16_SLOT(config, "isa3", 0, "pci:02.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "pci:02.0:isabus", pc_isa16_cards, nullptr, false);

	// 4x PCI slots
	PCI_SLOT(config, "pci:1", pci_cards, 7, 0, 1, 2, 3, "gd5446");
	PCI_SLOT(config, "pci:2", pci_cards, 6, 1, 2, 3, 0, nullptr);
	PCI_SLOT(config, "pci:3", pci_cards, 5, 2, 3, 0, 1, nullptr);
	PCI_SLOT(config, "pci:4", pci_cards, 4, 3, 0, 1, 2, nullptr);

	x86_softlists(config);
}

// very similar to ga586ip, with unknown RTC type and extra ISA slot
void i430lx_state::sy029c2(machine_config &config)
{
	i430lx_state::i430nx(config);
	ISA16_SLOT(config, "isa5", 0, "pci:02.0:isabus", pc_isa16_cards, nullptr, false);
}

// Siemens Nixdorf PCD-5H, System Board name is D842
void pcd5h_state::pcd5h(machine_config &config)
{
	PENTIUM(config, m_maincpu, 75'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &pcd5h_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &pcd5h_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("pci:01.0:pic0", FUNC(pic8259_device::inta_cb));
//  m_maincpu->smiact().set("pci:00.0", FUNC(i82434nx_pcmc_device::smi_act_w));

	DS12885(config, m_rtc, XTAL(32'768));
	m_rtc->set_binary(true);

	PCI_ROOT(config, "pci");
	I82434NX_PCMC(config, "pci:00.0", "maincpu", 64*1024*1024);

	i82378zb_sio_device &isa(I82378ZB_SIO(config, "pci:01.0", 0, "maincpu"));
	//isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.a20m().set_inputline("maincpu", INPUT_LINE_A20);
	isa.cpureset().set_inputline("maincpu", INPUT_LINE_RESET);
	isa.rtcale().set([this](u8 data) { m_rtc->address_w(data); });
	isa.rtccs_read().set([this]() { return m_rtc->data_r(); });
	isa.rtccs_write().set([this](u8 data) { m_rtc->data_w(data); });

	// X-Bus devices
	// TODO: unknown flash type, just to get a 0x20000 sized one
	AMD_29F010(config, m_flash);

	AT_KEYBOARD_CONTROLLER(config, m_keybc, XTAL(12'000'000));
	m_keybc->hot_res().set("pci:01.0", FUNC(i82378zb_sio_device::cpu_reset_w));
	m_keybc->gate_a20().set("pci:01.0", FUNC(i82378zb_sio_device::cpu_a20_w));
	m_keybc->kbd_irq().set("pci:01.0", FUNC(i82378zb_sio_device::pc_irq1_w));
	m_keybc->kbd_clk().set(m_at_con, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_keybc->kbd_data().set(m_at_con, FUNC(pc_kbdc_device::data_write_from_mb));

	PC_KBDC(config, m_at_con, pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL);
	m_at_con->out_clock_cb().set(m_keybc, FUNC(at_keyboard_controller_device::kbd_clk_w));
	m_at_con->out_data_cb().set(m_keybc, FUNC(at_keyboard_controller_device::kbd_data_w));

//  IDE_CONTROLLER_32(config, m_ide[0]).options(ata_devices, "hdd", nullptr, false);
//  m_ide[0]->irq_handler().set("pci:01.0", FUNC(i82378zb_sio_device::pc_irq14_w));
//
//  IDE_CONTROLLER_32(config, m_ide[1]).options(ata_devices, "cdrom", nullptr, false);
//  m_ide[1]->irq_handler().set("pci:01.0", FUNC(i82378zb_sio_device::pc_irq15_w));

	// 4x ISA slots and 1x PCI slot on the bus extension card
	ISA16_SLOT(config, "isa1", 0, "pci:01.0:isabus", pc_isa16_cards, "fdc_smc", false);
	ISA16_SLOT(config, "isa2", 0, "pci:01.0:isabus", pc_isa16_cards, "ide", false);
	ISA16_SLOT(config, "isa3", 0, "pci:01.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "pci:01.0:isabus", pc_isa16_cards, nullptr, false);

	PCI_SLOT(config, "pci:1", pci_cards, 12, 0, 2, 1, 3, nullptr);

	ET4KW32P_PCI(config, "pci:03.0");

	x86_softlists(config);
}

ROM_START( ga586ip )
	ROM_REGION32_LE(0x20000, "pci:02.0:xbus_flash", 0)
	// 05/06/96-NEPTUNE-2A59AG01-00
	ROM_SYSTEM_BIOS(0, "v20",  "GA-586IP V2.0 (4.51G)")
	ROMX_LOAD( "ip.20",  0x00000, 0x20000, CRC(77963e13) SHA1(af091749ac0e14b69dbfe5b2f4cb040ed06e56d9), ROM_BIOS(0))
	// 10/18/94-NEPTUNE-2A59AG01-00
	ROM_SYSTEM_BIOS(1, "v19l", "GA-586IP V1.9L (4.50)")
	ROMX_LOAD( "ip.19l", 0x00000, 0x20000, CRC(5263ab01) SHA1(832fd6af13e05691b4e6e5505d382d161acb7fc1), ROM_BIOS(1))
	// 10/18/94-NEPTUNE-2A59AG01-00
	ROM_SYSTEM_BIOS(2, "v18",  "GA-586IP V1.8 (4.50)")
	ROMX_LOAD( "ip.18",  0x00000, 0x20000, CRC(45c4c162) SHA1(8b3183fa8c2be91e3889eea8612e6e7ec6dac97e), ROM_BIOS(2))
ROM_END

ROM_START( pcd5h )
	ROM_REGION32_LE(0x20000, "pci:01.0:xbus_flash", 0)
	// Siemens Nixdorf System Board D842
	// PhoenixBIOS 4.04 core, unpacks its own video BIOS into C000 shadow RAM
	ROM_SYSTEM_BIOS(0, "v107", "PhoenixBIOS 4.04 Rev. 1.07.842")
	ROMX_LOAD( "d842_107.bin", 0x00000, 0x20000, CRC(c812e98f) SHA1(b9252d6c01fa13841410a412f5ce69147baee3ff), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v106", "PhoenixBIOS 4.04 Rev. 1.06.842")
	ROMX_LOAD( "d842_106.bin", 0x00000, 0x20000, CRC(8b8eb093) SHA1(315e302b28588924e9fd5594f61c119bb6b7470a), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v105", "PhoenixBIOS 4.04 Rev. 1.05.842")
	ROMX_LOAD( "d842_105.bin", 0x00000, 0x20000, CRC(ce92d70c) SHA1(2eddc391c7575f8f21f5c5e78aa45f6c4152a347), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v103", "PhoenixBIOS 4.04 Rev. 1.03.842") // Not from TheRetroWeb
	ROMX_LOAD( "d842_103.bin", 0x00000, 0x20000, CRC(520f9b00) SHA1(f1dab8ae6f635a6941d0091ff44a811fe35a89a9), ROM_BIOS(3))
	// PhoenixBIOS Pentium 1.03 core, Tseng video BIOS in the first 32KB
	// TODO: This is currently not mapping correctly
	ROM_SYSTEM_BIOS(4, "v110p", "PhoenixBIOS Pentium 1.03 Rev. 1.10.842")
	ROMX_LOAD( "d842_110p.bin", 0x00000, 0x20000, CRC(7b7a6504) SHA1(5ad01bf7e23ed0553136e885e72d8c9f207567b3), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v109p", "PhoenixBIOS Pentium 1.03 Rev. 1.09.842")
	ROMX_LOAD( "d842_109p.bin", 0x00000, 0x20000, CRC(0bf194f7) SHA1(0ca0590cf1c02bde20b87558680957563e375616), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "v103p", "PhoenixBIOS Pentium 1.03 Rev. 1.03.842")
	ROMX_LOAD( "d842_103p.bin", 0x00000, 0x20000, CRC(d8657b04) SHA1(8f3a188ad083cb3b591a31df8e7e05b639eb5f10), ROM_BIOS(6))
ROM_END

ROM_START( sy029c2 )
	ROM_REGION32_LE(0x20000, "pci:02.0:xbus_flash", 0)
	// 08/12/94-NEPTUNE-2A59AS21-00
	ROM_SYSTEM_BIOS(0, "a1",  "SY-029C2 rev A.1 (4.50G)")
	ROMX_LOAD( "p54c.bin",  0x00000, 0x20000, CRC(86a39522) SHA1(d97088bdda4e832b1f5830306cf4dcf1c60180d3), ROM_BIOS(0))
ROM_END


} // anonymous namespace

// LX chipset
// ...

// NX chipset
COMP( 1994, ga586ip, 0, 0,      i430nx,  0, i430lx_state, empty_init, "Gigabyte",        "GA-586IP (Intel I430NX Neptune chipset)", MACHINE_NOT_WORKING )
COMP( 1995, pcd5h,   0, 0,      pcd5h,   0, pcd5h_state,  empty_init, "Siemens Nixdorf", "PCD-5H (Intel I430NX Neptune chipset)",   MACHINE_NOT_WORKING )
COMP( 1994, sy029c2, 0, 0,      sy029c2, 0, i430lx_state, empty_init, "Soyo",            "SY-029C2 (Intel I430NX Neptune chipset)", MACHINE_NOT_WORKING )
