// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

    SiS 630 chipset based PC

    TODO:
    - PCI banking doesn't work as intended
      \- cfr. GUI expansion ROM, host shadow RAM bit 15, misc
    - Verify that PCI listing honors real HW
      \- Currently lists GUI, USB, '900, '7018, ACPI Controller;
    - Identify flash ROM type;
    - Video is sketchy;
      \- Shows SiS AGP header text in less than 1 frame (catchable with debugger only)
	  \- Needs VGA integrated control from host;
    - Floppy drive
      \- LPC accepts a SMC37C673 as default;
    - SMBus isn't extensively tested
      \- POST fails with a CMOS crc error for CPU identifier/speed, may need sensible defaults;
	- Accesses I/O $294 for the ACPI fans in BIOS menu;
    - Either move gamecstl.cpp to here or convert that driver to reuse the base interface
      declared here.
	  \- Currently black screens before booting normal Windows, reading $5004 from the LPC ACPI
	    (flip EAX to non-zero to bypass);
      \- Doesn't accept any PS/2 input, tries to install a "PCI standard CPU Host Bridge" (?),
	     hangs there;

**************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "bus/isa/isa_cards.h"
#include "machine/pci.h"
#include "machine/sis5513_ide.h"
#include "machine/sis630_host.h"
#include "machine/sis630_gui.h"
#include "machine/sis7001_usb.h"
#include "machine/sis7018_audio.h"
#include "machine/sis900_eth.h"
#include "machine/sis950_lpc.h"
//#include "machine/fdc37c93x.h"

class sis630_state : public driver_device
{
public:
	sis630_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ide(*this, "pci:00.1")
	{ }

	void sis630(machine_config &config);

private:

	required_device<pentium3_device> m_maincpu;
	required_device<sis5513_ide_device> m_ide;

//  void main_io(address_map &map);
//  void main_map(address_map &map);
};


static INPUT_PORTS_START(sis630)
INPUT_PORTS_END


void sis630_state::sis630(machine_config &config)
{
	// Slot 1/Socket 370, Coppermine FC-PGA @ 500~850+/100 MHz or Celeron PPGA 300~600+ MHz
	// TODO: lowered rate for debugging aid, needs a slot option anyway
	PENTIUM3(config, m_maincpu, 100'000'000);
//  m_maincpu->set_addrmap(AS_PROGRAM, &sis630_state::main_map);
//  m_maincpu->set_addrmap(AS_IO, &sis630_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("pci:01.0:pic_master", FUNC(pic8259_device::inta_cb));
//  m_maincpu->smiact().set("pci:00.0", FUNC(sis950_lpc_device::smi_act_w));

	PCI_ROOT(config, "pci", 0);
	// up to 512MB, 2 x DIMM sockets
	SIS630_HOST(config, "pci:00.0", 0, "maincpu", "pci:03.0", 256*1024*1024);
	SIS5513_IDE(config, m_ide, 0);
	// TODO: this should be pci:00.0:00.0 but for some reason it won't work with current model
	// (or bus #1 with 00.0)
	// just install it on a different device # for the time being
	SIS630_GUI (config, "pci:03.0", 0);
	m_ide->irq_pri().set("pci:01.0:pic_slave", FUNC(pic8259_device::ir6_w));
		//FUNC(sis950_lpc_device::pc_irq14_w));
	m_ide->irq_sec().set("pci:01.0:pic_slave", FUNC(pic8259_device::ir7_w));
		//FUNC(sis950_lpc_device::pc_mirq0_w));

	SIS950_LPC (config, "pci:01.0", 0, "maincpu");
	LPC_ACPI   (config, "pci:01.0:acpi", 0);
	SMBUS      (config, "pci:01.0:smbus", 0);

	SIS900_ETH(config, "pci:01.1", 0);
	// USB config: 2 on back, 3 on front. Front is fn 2
	SIS7001_USB(config, "pci:01.2", 0, 3);
	SIS7001_USB(config, "pci:01.3", 0, 2);
	SIS7018_AUDIO(config, "pci:01.4", 0);
	// documentation doesn't mention modem part #, derived from Shuttle MS11 MB manual
//  SIS7013_MODEM_AC97(config, "pci:01.6"

	SIS301_VIDEO_BRIDGE(config, "pci:02.0", 0);
	// TODO: 3 expansion PCI slots

	// TODO: 1 parallel + 2 serial ports
	// TODO: 1 game port
	// TODO: move keyboard/mouse PS/2 connectors in here

	// TODO: AMR (Audio/modem riser) + UPT (?), assume EISA complaint, needs specific slot options
	ISA16_SLOT(config, "isa1", 0, "pci:01.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:01.0:isabus", pc_isa16_cards, nullptr, false);
}

ROM_START(shutms11)
	ROM_REGION32_LE(0x40000, "pci:01.0", 0)
	ROM_SYSTEM_BIOS(0, "ms11s11d", "ms11s11d")
	ROMX_LOAD( "ms11s11d.bin",     0x000000, 0x040000, CRC(27077a58) SHA1(32327ebf328cb0c2dec819c3710acc83527803c5), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "ms11s134", "ms11s134")
	ROMX_LOAD( "ms11s134.bin",     0x000000, 0x040000, CRC(d739c4f3) SHA1(2301e57163ac4d9b7eddcabce52fa7d01b22330e), ROM_BIOS(1) )
ROM_END

COMP( 2000, shutms11, 0,      0,      sis630,  sis630, sis630_state, empty_init, "Shuttle", "MS11 PC", MACHINE_IS_SKELETON )
