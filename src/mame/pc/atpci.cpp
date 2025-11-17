// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic, Carl

#include "emu.h"
#include "cpu/i386/i386.h"
#include "bus/isa/isa_cards.h"
#include "bus/lpci/pci.h"
#include "bus/lpci/i82371ab.h"
#include "bus/lpci/i82371sb.h"
#include "bus/lpci/i82439tx.h"
#include "machine/at.h"
#include "machine/fdc37c93x.h"
#include "machine/pckeybrd.h"


namespace {

class at586_state : public driver_device
{
public:
	at586_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void at586x3(machine_config &config);
	void at586(machine_config &config);
	void at586m55(machine_config &config);

protected:
	void at_softlists(machine_config &config);

	void boot_state_w(uint8_t data);

	void tx_config(device_t *device);
	void sb_config(device_t *device);
	void superio_config(device_t *device);

	void at586_io(address_map &map) ATTR_COLD;
	void at586_map(address_map &map) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
};

void at586_state::boot_state_w(uint8_t data)
{
	logerror("Boot state %02x\n", data);
	printf("[%02X]",data);
}

void at586_state::tx_config(device_t *device)
{
	downcast<i82439tx_device *>(device)->set_cpu(m_maincpu);
	downcast<i82439tx_device *>(device)->set_region("isa");
}

void at586_state::sb_config(device_t *device)
{
	i82371sb_device &sb = *downcast<i82371sb_device *>(device);
	sb.set_cpu(m_maincpu);
	sb.boot_state_hook().set(":", FUNC(at586_state::boot_state_w));
	sb.smi().set_inputline(m_maincpu, INPUT_LINE_SMI);
}


void at586_state::superio_config(device_t *device)
{
	fdc37c93x_device &fdc = *downcast<fdc37c93x_device *>(device);
	fdc.set_sysopt_pin(1);
	fdc.gp20_reset().set_inputline(m_maincpu, INPUT_LINE_RESET);
	fdc.gp25_gatea20().set_inputline(m_maincpu, INPUT_LINE_A20);
	fdc.irq1().set(":pcibus:7:i82371sb:pic8259_master", FUNC(pic8259_device::ir1_w));
}


static void isa_internal_devices(device_slot_interface &device)
{
	device.option_add("fdc37c93x", FDC37C93X);
}

static void pci_devices(device_slot_interface &device)
{
	device.option_add_internal("i82439tx", I82439TX_LEGACY);
	device.option_add_internal("i82371ab", I82371AB);
	device.option_add_internal("i82371sb", I82371SB);
}

void at586_state::at586_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).bankrw("bank10");
	map(0x000a0000, 0x000bffff).noprw();
	map(0x00800000, 0x00800bff).ram().share("nvram");
	map(0xfffe0000, 0xffffffff).rom().region("isa", 0x20000);
}

void at586_state::at586_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0cf8, 0x0cff).rw("pcibus", FUNC(pci_bus_device::read), FUNC(pci_bus_device::write));
}

void at586_state::at_softlists(machine_config &config)
{
	/* software lists */
	SOFTWARE_LIST(config, "pc_disk_list").set_original("ibm5150");
	SOFTWARE_LIST(config, "at_disk_list").set_original("ibm5170");
	SOFTWARE_LIST(config, "at_cdrom_list").set_original("ibm5170_cdrom");
	SOFTWARE_LIST(config, "at_hdd_list").set_original("ibm5170_hdd");
	SOFTWARE_LIST(config, "midi_disk_list").set_compatible("midi_flop");
	SOFTWARE_LIST(config, "photocd_list").set_compatible("photo_cd");
}

void at586_state::at586(machine_config &config)
{
	pentium_device &maincpu(PENTIUM(config, m_maincpu, 60000000));
	maincpu.set_addrmap(AS_PROGRAM, &at586_state::at586_map);
	maincpu.set_addrmap(AS_IO, &at586_state::at586_io);
	maincpu.set_irq_acknowledge_callback("pcibus:1:i82371ab:pic8259_master", FUNC(pic8259_device::inta_cb));

	RAM(config, RAM_TAG).set_default_size("4M").set_extra_options("1M,2M,8M,16M,32M,64M,128M,256M");

	PCI_BUS(config, "pcibus", 0).set_busnum(0);
	PCI_CONNECTOR(config, "pcibus:0", pci_devices, "i82439tx", true).set_option_machine_config("i82439tx", [this](device_t *device) { tx_config(device); });
	PCI_CONNECTOR(config, "pcibus:1", pci_devices, "i82371ab", true);

	// FIXME: determine ISA bus clock
	ISA16_SLOT(config, "isa1", 0, "pcibus:1:i82371ab:isabus", pc_isa16_cards, "svga_et4k", false);
	ISA16_SLOT(config, "isa2", 0, "pcibus:1:i82371ab:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pcibus:1:i82371ab:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "pcibus:1:i82371ab:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "pcibus:1:i82371ab:isabus", pc_isa16_cards, nullptr, false);

	at_softlists(config);
}

void at586_state::at586x3(machine_config &config)
{
	pentium_device &maincpu(PENTIUM(config, m_maincpu, 60000000));
	maincpu.set_addrmap(AS_PROGRAM, &at586_state::at586_map);
	maincpu.set_addrmap(AS_IO, &at586_state::at586_io);
	maincpu.smiact().set("pcibus:0:i82439tx", FUNC(i82439tx_device::smi_act_w));
	maincpu.set_irq_acknowledge_callback("pcibus:1:i82371sb:pic8259_master", FUNC(pic8259_device::inta_cb));

	RAM(config, RAM_TAG).set_default_size("4M").set_extra_options("1M,2M,8M,16M,32M,64M,128M,256M");

	PCI_BUS(config, "pcibus", 0).set_busnum(0);
	PCI_CONNECTOR(config, "pcibus:0", pci_devices, "i82439tx", true).set_option_machine_config("i82439tx", [this](device_t *device) { tx_config(device); });
	PCI_CONNECTOR(config, "pcibus:1", pci_devices, "i82371sb", true).set_option_machine_config("i82371sb", [this](device_t *device) { sb_config(device); });

	// FIXME: determine ISA bus clock
	ISA16_SLOT(config, "isa1", 0, "pcibus:1:i82371sb:isabus", pc_isa16_cards, "svga_et4k", false);
	ISA16_SLOT(config, "isa2", 0, "pcibus:1:i82371sb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pcibus:1:i82371sb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "pcibus:1:i82371sb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "pcibus:1:i82371sb:isabus", pc_isa16_cards, nullptr, false);

	at_softlists(config);
}

static INPUT_PORTS_START(at586m55)
INPUT_PORTS_END

void at586_state::at586m55(machine_config &config)
{
	pentium_device &pentium(PENTIUM(config, m_maincpu, 60000000));
	pentium.set_addrmap(AS_PROGRAM, &at586_state::at586_map);
	pentium.set_addrmap(AS_IO, &at586_state::at586_io);
	pentium.smiact().set("pcibus:0:i82439tx", FUNC(i82439tx_device::smi_act_w));
	pentium.set_irq_acknowledge_callback("pcibus:7:i82371sb:pic8259_master", FUNC(pic8259_device::inta_cb));

	RAM(config, RAM_TAG).set_default_size("4M").set_extra_options("1M,2M,8M,16M,32M,64M,128M,256M");

	PCI_BUS(config, "pcibus", 0).set_busnum(0);
	PCI_CONNECTOR(config, "pcibus:0", pci_devices, "i82439tx", true).set_option_machine_config("i82439tx", [this](device_t *device) { tx_config(device); });
	PCI_CONNECTOR(config, "pcibus:7", pci_devices, "i82371sb", true).set_option_machine_config("i82371sb", [this](device_t *device) { sb_config(device); });

	ISA16_SLOT(config, "board4", 0, "pcibus:7:i82371sb:isabus",  isa_internal_devices, "fdc37c93x", true).set_option_machine_config("fdc37c93x", [this](device_t *device) { superio_config(device); });
	ISA16_SLOT(config, "isa1", 0, "pcibus:7:i82371sb:isabus",  pc_isa16_cards, "svga_et4k", false);
	ISA16_SLOT(config, "isa2", 0, "pcibus:7:i82371sb:isabus",  pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pcibus:7:i82371sb:isabus",  pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "pcibus:7:i82371sb:isabus",  pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "pcibus:7:i82371sb:isabus",  pc_isa16_cards, nullptr, false);

	at_softlists(config);
}

ROM_START( at586 )
	ROM_REGION32_LE(0x40000, "isa", 0)
	ROM_SYSTEM_BIOS(0, "sptx", "SP-586TX")
	ROMX_LOAD("sp586tx.bin",   0x20000, 0x20000, CRC(1003d72c) SHA1(ec9224ff9b0fdfd6e462cb7bbf419875414739d6), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "unisys", "Unisys 586") // probably bad dump due to need of hack in i82439tx to work
	ROMX_LOAD("at586.bin",     0x20000, 0x20000, CRC(717037f5) SHA1(1d49d1b7a4a40d07d1a897b7f8c827754d76f824), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "ga586t2", "Gigabyte GA-586T2") // ITE 8679 I/O
	ROMX_LOAD("gb_ga586t2.bin",  0x20000, 0x20000, CRC(3a50a6e1) SHA1(dea859b4f1492d0d08aacd260ed1e83e00ebac08), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "5tx52", "Acorp 5TX52") // W83877TF I/O
	ROMX_LOAD("acorp_5tx52.bin", 0x20000, 0x20000, CRC(04d69419) SHA1(983377674fef05e710c8665c14cc348c99166fb6), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "txp4", "ASUS TXP4") // W83977TF-A I/O
	ROMX_LOAD("asus_txp4.bin",   0x20000, 0x20000, CRC(a1321bb1) SHA1(92e5f14d8505119f85b148a63510617ac12bcdf3), ROM_BIOS(4))
ROM_END

// Elitegroup SI5PI AIO Rev. 1.1 - Chipset: SiS 85C501, 85C502, 85C502, FDC37C666GT, UM82C865F, PCI0640B - CPU: Socket 4 - RAM: 4xSIMM72, Cache: 18x32pin
// BIOS: Award 32pin - Keyboard-BIOS: AMIKEY-2 - ISA16: 4, PCI: 4 - On board: 2xISA, 2xser, Floppy, par
ROM_START( ecssi5pi )
	ROM_REGION32_LE(0x40000, "isa", 0)
	// BIOS-String: 11/17/94-SiS-501-503-2A5IAE11-00 / SI5PI AIO 11/18/94
	ROM_SYSTEM_BIOS(0, "111894", "11/18/94")
	ROMX_LOAD("si5piaio11_isa_pci_bios_013908435.bin", 0x20000, 0x20000, CRC(c39aa47c) SHA1(6b31bdc5441c5f7a29d0ceb8989ccfed92c49900), ROM_BIOS(0))
	// BIOS-String: 12/02/94-SiS-501-503-2A5IAE11-00 / SI5PI AIO 12/02/94
	ROM_SYSTEM_BIOS(1, "120294", "12/02/94")
	ROMX_LOAD("si5piaio.bin", 0x20000, 0x20000, CRC(9dfb8510) SHA1(b86cc1930dc78db3c4c9d1ed13ec60b2333db7d1), ROM_BIOS(1))
ROM_END


ROM_START(at586x3)
	ROM_REGION32_LE(0x40000, "isa", 0)
	ROM_SYSTEM_BIOS(0, "5hx29", "5HX29")
	ROMX_LOAD("5hx29.bin", 0x20000, 0x20000, CRC(07719a55) SHA1(b63993fd5186cdb4f28c117428a507cd069e1f68), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "n7ns04", "Version 21/01/98, without integrated sound") // Micronics M7S-HI with SMSC FDC37C93X I/O
	ROMX_LOAD("m7ns04.rom", 0x00000, 0x40000, CRC(9c1f656b) SHA1(f4a0a522d8c47b6ddb6c01fe9a34ddf5b1977f8d), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "n7s04", "Version 21/01/98, with integrated sound")
	ROMX_LOAD("m7s04.rom", 0x00000, 0x40000, CRC(3689f5a9) SHA1(8daacdb0dc6783d2161680564ffe83ac2515f7ef), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "m7shi03", "m7shi03") // Micronics M7S-Hi
	ROMX_LOAD("m7shi03.snd", 0x00000, 0x40000, CRC(3a35a939) SHA1(74af69eb5ca546b0960540e7c3ea62a532157f2a), ROM_BIOS(3))
ROM_END

/* FIC VT-503 (Intel TX chipset, ITE 8679 Super I/O) */
ROM_START( ficvt503 )
	ROM_REGION32_LE(0x40000, "isa", 0)
	ROM_SYSTEM_BIOS(0, "109gi13", "1.09GI13") /* 1997-10-02 */
	ROMX_LOAD("109gi13.bin", 0x20000, 0x20000, CRC(0c32af48) SHA1(2cce40a98598f1ed1f398975f7a90c8be4200667), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "109gi14", "1.09GI14") /* 1997-11-07 */
	ROMX_LOAD("109gi14.awd", 0x20000, 0x20000, CRC(588c5cc8) SHA1(710e5405850fd975b362a422bfe9bc6d6c9a36cd), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "109gi15", "1.09GI15") /* 1997-11-07 */
	ROMX_LOAD("109gi15.awd", 0x20000, 0x20000, CRC(649a3481) SHA1(e681c6ab55a67cec5978dfffa75fcddc2aa0de4d), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "109gi16", "1.09GI16") /* 2000-03-23 */
	ROMX_LOAD("109gi16.bin", 0x20000, 0x20000, CRC(a928f271) SHA1(127a83a60752cc33b3ca49774488e511ec7bac55), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "115gk140", "1.15GK140") /* 1999-03-03 */
	ROMX_LOAD("115gk140.awd", 0x20000, 0x20000, CRC(65e88956) SHA1(f94bb0732e00b5b0f18f4e349db24a289f8379c5), ROM_BIOS(4))
ROM_END

/* Micronics M55Hi-Plus (Intel 430HX chipset, SMSC FDC37C93X Super I/O) */
ROM_START(m55hipl)
	ROM_REGION32_LE(0x40000, "isa", 0)
	ROM_SYSTEM_BIOS(0, "m55ns04", "m55ns04") // Micronics M55HI-Plus with no sound
	ROMX_LOAD("m55-04ns.rom", 0x20000, 0x20000, CRC(0116b2b0) SHA1(19b0203decfd4396695334517488d488aec3ccde), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "m55s04", "m55s04") // with sound
	ROMX_LOAD("m55-04s.rom", 0x20000, 0x20000, CRC(34a7422e) SHA1(68753fe373c97844beff83ea75c634c77cfedb8f), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "m55ns03", "m55ns03") // Micronics M55HI-Plus with no sound
	ROMX_LOAD("m55ns03.rom", 0x20000, 0x20000, CRC(6a3deb49) SHA1(78bfc20e0f8699f4d153d241a757153afcde3efb), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "m55hi03", "m55hi03") // with sound
	ROMX_LOAD("m55hi03.rom", 0x20000, 0x20000, CRC(bd476200) SHA1(7633ba27819ad45c6253abb728b1ef0c49229743), ROM_BIOS(3))
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT   COMPAT   MACHINE   INPUT     CLASS        INIT        COMPANY      FULLNAME             FLAGS
COMP( 1990, at586,    ibm5170, 0,       at586,    0,        at586_state, empty_init, "<generic>", "PC/AT 586 (PIIX4)", MACHINE_NOT_WORKING )
COMP( 1990, at586x3,  ibm5170, 0,       at586x3,  0,        at586_state, empty_init, "<generic>", "PC/AT 586 (PIIX3)", MACHINE_NOT_WORKING )
COMP( 1997, ficvt503, ibm5170, 0,       at586,    0,        at586_state, empty_init, "FIC",       "VT-503",            MACHINE_NOT_WORKING )
COMP( 1990, m55hipl,  ibm5170, 0,       at586m55, at586m55, at586_state, empty_init, "Micronics", "M55Hi-Plus",        MACHINE_NOT_WORKING )
COMP( 199?, ecssi5pi, ibm5170, 0,       at586,    0,        at586_state, empty_init, "Elitegroup","SI5PI AIO",         MACHINE_NOT_WORKING )
