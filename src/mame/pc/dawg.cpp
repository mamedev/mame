// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Dawg Gone Fun

AMD Geode AGX0533EEXF080
AMD Geode CS5535
SMSC SI010N268-NE (Microchip LPC super I/O)
86S5IDl0-1MDG DDR1 PC2100 RAM bank
TI DP83816 MacPhyter-II (PCI Ethernet)
Serial port with "Windows CE Core 5.0" sticker

TODO:
- Fails for CPUID off the bat;
- Upgrade southbridge to CS5535;

**************************************************************************************************/

#include "emu.h"

#include "bus/isa/isa_cards.h"
#include "cpu/i386/i386.h"
#include "machine/8042kbdc.h"
#include "machine/mc146818.h"
#include "machine/mediagx_cs5530_bridge.h"
#include "machine/mediagx_cs5530_ide.h"
#include "machine/mediagx_cs5530_video.h"
#include "machine/mediagx_host.h"
#include "machine/pci.h"
#include "machine/zfmicro_usb.h"

namespace {

class dawg_state : public driver_device
{
public:
	dawg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rtc(*this, "rtc")
		, m_kbdc(*this, "kbdc")
	{ }

	void dawg(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<ds1287_device> m_rtc;
	required_device<kbdc8042_device> m_kbdc;
};



static INPUT_PORTS_START( dawg )
INPUT_PORTS_END


void dawg_state::dawg(machine_config &config)
{
	MEDIAGX(config, m_maincpu, 233'000'000); // Cyrix MediaGX GXm-266GP
	m_maincpu->set_irq_acknowledge_callback("pci:12.0:pic8259_master", FUNC(pic8259_device::inta_cb));

	// TODO: from FDC37C93x super I/O?
	// NOTE: both aren't initialized at $3f0 - $370 but accessed anyway, wtf
	DS1287(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->set_binary(true);
	m_rtc->set_epoch(1980);
	m_rtc->irq().set("pci:12.0", FUNC(mediagx_cs5530_bridge_device::pc_irq8n_w));

	KBDC8042(config, m_kbdc, 0);
	// TODO: PS/2 mouse
	m_kbdc->set_keyboard_type(kbdc8042_device::KBDC8042_STANDARD);
	m_kbdc->system_reset_callback().set_inputline(":maincpu", INPUT_LINE_RESET);
	m_kbdc->gate_a20_callback().set_inputline(":maincpu", INPUT_LINE_A20);
	m_kbdc->input_buffer_full_callback().set(":pci:12.0", FUNC(mediagx_cs5530_bridge_device::pc_irq1_w));
	m_kbdc->set_keyboard_tag("at_keyboard");

	at_keyboard_device &at_keyb(AT_KEYB(config, "at_keyboard", pc_keyboard_device::KEYBOARD_TYPE::AT, 1));
	at_keyb.keypress().set(m_kbdc, FUNC(kbdc8042_device::keyboard_w));

	PCI_ROOT(config, "pci", 0);
	MEDIAGX_HOST(config, "pci:00.0", 0, "maincpu", 128*1024*1024);
	// TODO: no clue about the ID used for this, definitely tested
	// Tries to initialize MediaGX F4 -> ISA -> PCI
	// May actually be a ZFMicro PCI Bridge (0x10780400)?
	PCI_BRIDGE(config, "pci:01.0", 0, 0x10780000, 0);

	// "pci:12.0" or "pci:10.0" depending on pin H26 (readable in bridge thru PCI index $44)
	mediagx_cs5530_bridge_device &isa(MEDIAGX_CS5530_BRIDGE(config, "pci:12.0", 0, "maincpu", "pci:12.2"));
	isa.set_kbdc_tag("kbdc");
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	//isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);
	isa.rtcale().set([this](u8 data) { m_rtc->address_w(data); });
	isa.rtccs_read().set([this]() { return m_rtc->data_r(); });
	isa.rtccs_write().set([this](u8 data) { m_rtc->data_w(data); });

	// "pci:12.1" SMI & ACPI

	mediagx_cs5530_ide_device &ide(MEDIAGX_CS5530_IDE(config, "pci:12.2", 0, "maincpu"));
	ide.subdevice<bus_master_ide_controller_device>("ide1")->slot(0).set_default_option("cf");
	ide.irq_pri().set("pci:12.0", FUNC(mediagx_cs5530_bridge_device::pc_irq14_w));
	ide.irq_sec().set("pci:12.0", FUNC(mediagx_cs5530_bridge_device::pc_irq15_w));

	// "pci:12.3" XpressAUDIO
	MEDIAGX_CS5530_VIDEO(config, "pci:12.4", 0);

	ZFMICRO_USB(config, "pci:13.0", 0);

	// 2 PCI slots, 2 ISA slots
	ISA16_SLOT(config, "isa1", 0, "pci:12.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:12.0:isabus", pc_isa16_cards, nullptr, false);
}


ROM_START( dawg )
	ROM_REGION32_LE(0x80000, "pci:12.0", 0)
	ROM_LOAD("dawg.56", 0x00000, 0x80000, CRC(7ad3578f) SHA1(fba268f530765959fee5ad681adb8a7b70e573db) )

	DISK_REGION( "pci:12.2:ide1:0:cf" )
	// baddump: contains a "System Volume Information" folder
	DISK_IMAGE( "dawg", 0, BAD_DUMP SHA1(0ada23b43c9fb0d2d8393edfb569a7c5c431775f))
ROM_END




} // anonymous namespace

// year derived from dump contents
GAME( 2009, dawg, 0, dawg, dawg, dawg_state, empty_init, ROT0, "Pace-O-Matic", "Dawg Gone Fun", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
