// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic, Carl

#include "emu.h"
#include "cpu/i386/i386.h"
#include "bus/lpci/pci.h"
#include "bus/lpci/i82371ab.h"
#include "bus/lpci/i82371sb.h"
#include "bus/lpci/i82439tx.h"
#include "machine/at.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/keyboards.h"

class at586_state : public driver_device
{
public:
	at586_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }
	required_device<cpu_device> m_maincpu;

};

static MACHINE_CONFIG_FRAGMENT( tx_config )
	MCFG_I82439TX_CPU( "maincpu" )
	MCFG_I82439TX_REGION( "isa" )
MACHINE_CONFIG_END

static SLOT_INTERFACE_START( pci_devices )
	SLOT_INTERFACE_INTERNAL("i82439tx", I82439TX)
	SLOT_INTERFACE_INTERNAL("i82371ab", I82371AB)
	SLOT_INTERFACE_INTERNAL("i82371sb", I82371SB)
SLOT_INTERFACE_END

static ADDRESS_MAP_START( at586_map, AS_PROGRAM, 32, at586_state )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAMBANK("bank10")
	AM_RANGE(0x000a0000, 0x000bffff) AM_NOP
	AM_RANGE(0x00800000, 0x00800bff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("isa", 0x20000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( at586_io, AS_IO, 32, at586_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_device, read, write)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( at586, at586_state )
	MCFG_CPU_ADD("maincpu", PENTIUM, 60000000)
	MCFG_CPU_PROGRAM_MAP(at586_map)
	MCFG_CPU_IO_MAP(at586_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pcibus:1:i82371ab:pic8259_master", pic8259_device, inta_cb)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("1M,2M,8M,16M,32M,64M,128M,256M")

	MCFG_PCI_BUS_ADD("pcibus", 0)
	MCFG_PCI_BUS_DEVICE("pcibus:0", pci_devices, "i82439tx", true)
	MCFG_SLOT_OPTION_MACHINE_CONFIG("i82439tx", tx_config)

	MCFG_PCI_BUS_DEVICE("pcibus:1", pci_devices, "i82371ab", true)

	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371ab:isabus","isa1", pc_isa16_cards, "svga_et4k", false)
	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371ab:isabus","isa2", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371ab:isabus","isa3", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371ab:isabus","isa4", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371ab:isabus","isa5", pc_isa16_cards, nullptr, false)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL)

	MCFG_FRAGMENT_ADD( at_softlists )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( at586x3, at586_state )
	MCFG_CPU_ADD("maincpu", PENTIUM, 60000000)
	MCFG_CPU_PROGRAM_MAP(at586_map)
	MCFG_CPU_IO_MAP(at586_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pcibus:1:i82371sb:pic8259_master", pic8259_device, inta_cb)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("1M,2M,8M,16M,32M,64M,128M,256M")

	MCFG_PCI_BUS_ADD("pcibus", 0)
	MCFG_PCI_BUS_DEVICE("pcibus:0", pci_devices, "i82439tx", true)
	MCFG_SLOT_OPTION_MACHINE_CONFIG("i82439tx", tx_config)

	MCFG_PCI_BUS_DEVICE("pcibus:1", pci_devices, "i82371sb", true)

	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371sb:isabus","isa1", pc_isa16_cards, "svga_et4k", false)
	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371sb:isabus","isa2", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371sb:isabus","isa3", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371sb:isabus","isa4", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371sb:isabus","isa5", pc_isa16_cards, nullptr, false)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL)

	MCFG_FRAGMENT_ADD( at_softlists )
MACHINE_CONFIG_END

ROM_START( at586 )
	ROM_REGION32_LE(0x40000, "isa", 0)
	ROM_SYSTEM_BIOS(0, "sptx", "SP-586TX")
	ROMX_LOAD("sp586tx.bin",   0x20000, 0x20000, CRC(1003d72c) SHA1(ec9224ff9b0fdfd6e462cb7bbf419875414739d6), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "unisys", "Unisys 586") // probably bad dump due to need of hack in i82439tx to work
	ROMX_LOAD("at586.bin",     0x20000, 0x20000, CRC(717037f5) SHA1(1d49d1b7a4a40d07d1a897b7f8c827754d76f824), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(2, "ga586t2", "Gigabyte GA-586T2") // ITE 8679 I/O
	ROMX_LOAD("gb_ga586t2.bin",  0x20000, 0x20000, CRC(3a50a6e1) SHA1(dea859b4f1492d0d08aacd260ed1e83e00ebac08), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "5tx52", "Acorp 5TX52") // W83877TF I/O
	ROMX_LOAD("acorp_5tx52.bin", 0x20000, 0x20000, CRC(04d69419) SHA1(983377674fef05e710c8665c14cc348c99166fb6), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "txp4", "ASUS TXP4") // W83977TF-A I/O
	ROMX_LOAD("asus_txp4.bin",   0x20000, 0x20000, CRC(a1321bb1) SHA1(92e5f14d8505119f85b148a63510617ac12bcdf3), ROM_BIOS(5))
ROM_END

ROM_START( at586x3 )
	ROM_REGION32_LE(0x40000, "isa", 0)
	ROM_LOAD("5hx29.bin",   0x20000, 0x20000, CRC(07719a55) SHA1(b63993fd5186cdb4f28c117428a507cd069e1f68))
ROM_END

/* FIC VT-503 (Intel TX chipset, ITE 8679 Super I/O) */
ROM_START( ficvt503 )
	ROM_REGION32_LE(0x40000, "isa", 0)
	ROM_SYSTEM_BIOS(0, "109gi13", "1.09GI13") /* 1997-10-02 */
	ROMX_LOAD("109gi13.bin", 0x20000, 0x20000, CRC(0c32af48) SHA1(2cce40a98598f1ed1f398975f7a90c8be4200667), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "109gi14", "1.09GI14") /* 1997-11-07 */
	ROMX_LOAD("109gi14.awd", 0x20000, 0x20000, CRC(588c5cc8) SHA1(710e5405850fd975b362a422bfe9bc6d6c9a36cd), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "109gi15", "1.09GI15") /* 1997-11-07 */
	ROMX_LOAD("109gi15.awd", 0x20000, 0x20000, CRC(649a3481) SHA1(e681c6ab55a67cec5978dfffa75fcddc2aa0de4d), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "109gi16", "1.09GI16") /* 2000-03-23 */
	ROMX_LOAD("109gi16.bin", 0x20000, 0x20000, CRC(a928f271) SHA1(127a83a60752cc33b3ca49774488e511ec7bac55), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "115gk140", "1.15GK140") /* 1999-03-03 */
	ROMX_LOAD("115gk140.awd", 0x20000, 0x20000, CRC(65e88956) SHA1(f94bb0732e00b5b0f18f4e349db24a289f8379c5), ROM_BIOS(5))
ROM_END

COMP ( 1990, at586,    ibm5170, 0,       at586,     0, driver_device,   0,  "<generic>",  "PC/AT 586 (PIIX4)", MACHINE_NOT_WORKING )
COMP ( 1990, at586x3,  ibm5170, 0,       at586x3,   0, driver_device,   0,  "<generic>",  "PC/AT 586 (PIIX3)", MACHINE_NOT_WORKING )
COMP ( 1997, ficvt503, ibm5170, 0,       at586,     0, driver_device,   0,  "FIC", "VT-503",                   MACHINE_NOT_WORKING )
