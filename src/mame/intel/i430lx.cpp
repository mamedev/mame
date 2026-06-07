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
- Texas Instruments BENCHMARQ bq3287AMT RTC (ds1287 clone?)
- No super I/O for keyboard/RTC, those are southbridge responsibility under X-Bus

Regular SIO is reused by earlier I420ZX "Saturn II" chipset and in BeBox
"Mercury" 82433LX/82434LX are earlier revisions of the northbridge
"Neptune MP" variants uses an 82379AB (SIO.A), which maps an extra APIC

TODO:
- X-Bus peripherals for SIO (FDC, IDE, COM x2, LPT, port $92);
- Monkey write config_maps for both bridges;
- Understand why a PCI VGA card doesn't draw (BIOS shenanigans?);
- RTC doesn't save, likely wrong type;

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
#include "machine/mc146818.h"
#include "machine/i82378zb_sio.h"
#include "machine/i82434lx_pcmc.h"
#include "machine/pci.h"
#include "sound/spkrdev.h"

#include "softlist.h"
#include "softlist_dev.h"

namespace {

class i430lx_state : public driver_device
{
public:
	i430lx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rtc(*this, "rtc")
//		, m_speaker(*this, "speaker")
	{ }

	void i430nx(machine_config &config) ATTR_COLD;

protected:
//	void i430lx(machine_config &config) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ds1287_device> m_rtc;
//	required_device<speaker_sound_device> m_speaker;

private:
	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


void i430lx_state::main_map(address_map &map)
{
	map.unmap_value_high();
}

void i430lx_state::main_io(address_map &map)
{
	map.unmap_value_high();
}

void i430lx_state::i430nx(machine_config &config)
{
	// Socket 5 / PGA320
	// FSB 60/66 MHz
	PENTIUM(config, m_maincpu, 66'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &i430lx_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &i430lx_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("pci:07.0:pic0", FUNC(pic8259_device::inta_cb));
//  m_maincpu->smiact().set("pci:00.0", FUNC(i82434nx_pcmc_device::smi_act_w));

	// Texas Instruments BENCHMARQ bq3287AMT RTC
	DS1287(config, m_rtc, XTAL(32'768));
	m_rtc->set_binary(true);
	// TODO: alarm irq
//	m_rtc->irq().set ...

	// TODO: config space not known
	PCI_ROOT(config, "pci");
	// max RAM 512MB
	I82434NX_PCMC(config, "pci:00.0", "maincpu", 64*1024*1024);

	i82378zb_sio_device &isa(I82378ZB_SIO(config, "pci:07.0", 0, "maincpu"));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.a20m().set_inputline("maincpu", INPUT_LINE_A20);
	isa.cpureset().set_inputline("maincpu", INPUT_LINE_RESET);
	isa.rtcale().set([this](u8 data) { m_rtc->address_w(data); });
	isa.rtccs_read().set([this]() { return m_rtc->data_r(); });
	isa.rtccs_write().set([this](u8 data) { m_rtc->data_w(data); });

	// 1x AT keyboard
	// 4x ISA slots
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	// TODO: temporary mapping, want a PCI card here (which somehow doesn't work)
	ISA16_SLOT(config, "isa4", 0, "pci:07.0:isabus", pc_isa16_cards, "svga_et4k", false);

	// 4x PCI slots
	PCI_SLOT(config, "pci:1", pci_cards, 8,  0, 1, 2, 3, nullptr);
	PCI_SLOT(config, "pci:2", pci_cards, 9,  1, 2, 3, 0, nullptr);
	PCI_SLOT(config, "pci:3", pci_cards, 10, 2, 3, 0, 1, nullptr);
	PCI_SLOT(config, "pci:4", pci_cards, 11, 3, 0, 1, 2, nullptr);
}

ROM_START( ga586ip )
	ROM_REGION32_LE(0x20000, "pci:07.0", 0)
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

} // anonymous namespace

// LX chipset
// ...

// NX chipset
COMP( 1994, ga586ip, 0, 0,      i430nx, 0, i430lx_state, empty_init, "Gigabyte",  "GA-586IP (Intel I430NX Neptune chipset)", MACHINE_NOT_WORKING )
