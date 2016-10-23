// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

/*
  Sandbox experiment on a new-pci pc

  Virtual hardware:
  - A pentium as main CPU
  - A Micronics M55HI-Plus PCI/ISA motherboard without integrated sound
  -> intel 430hx, aka 82439hx northbridge (pci, no agp)
  -> intel piix3, aka 82371sb southbridge (pci-isa bridge, ide, ioapic, timer, irq, dma, usb)
  -> smsc fdc37c93x superio (isa-connected, keyboard, rtc, fdc, rs232, ide)
  - A Matrox Millennium PCI video card

  We'll see about sound, networking, etc later

*/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"
#include "machine/pci-ide.h"
#include "machine/i82439hx.h"
#include "machine/i82371sb.h"
//#include "machine/fdc37c93x.h"
#include "video/mga2064w.h"

class pcipc_state : public driver_device
{
public:
	struct boot_state_info {
		uint8_t val;
		const char *const message;
	};

	static const boot_state_info boot_state_infos[];

	void boot_state_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	pcipc_state(const machine_config &mconfig, device_type type, const char *tag);

	virtual void machine_start() override;
	virtual void machine_reset() override;


};

pcipc_state::pcipc_state(const machine_config &mconfig, device_type type, const char *tag) : driver_device(mconfig, type, tag)
{
}

void pcipc_state::machine_start()
{
}

void pcipc_state::machine_reset()
{
}

const pcipc_state::boot_state_info pcipc_state::boot_state_infos[] = {
	{ 0x02, "Verify Real Mode." },
	{ 0x04, "Get CPU type." },
	{ 0x06, "Initialize system hardware." },
	{ 0x08, "Initialize chipset registers with initial POST values." },
	{ 0x09, "Get in POST Reg." },
	{ 0x0A, "Initialize CPU registers." },
	{ 0x0C, "Initialize cache initial POST values." },
	{ 0x0E, "Initialize I/O." },
	{ 0x0F, "Initialize the localbus IDE." },
	{ 0x10, "Initialize Power Management." },
	{ 0x11, "Load alternate registers with initial POST values." },
	{ 0x12, "Jump to UserPatch0." },
	{ 0x14, "Initialize keyboard controller." },
	{ 0x16, "BIOS ROM checksum." },
	{ 0x18, "8254 timer initialization." },
	{ 0x1A, "8237 DMA controller initialization." },
	{ 0x1C, "Reset Programmable Interrupt Controller." },
	{ 0x20, "Test DRAM refresh." },
	{ 0x22, "Test 8742 Keyboard Controller." },
	{ 0x24, "Set ES segment register to 4 GB." },
	{ 0x28, "Autosize DRAM." },
	{ 0x2A, "Clear 512K base RAM." },
	{ 0x2C, "Test 512K base address lines." },
	{ 0x2E, "Test 512K base memory." },
	{ 0x32, "Test CPU bus-clock frequency." },
	{ 0x34, "Test CMOS RAM." },
	{ 0x35, "Initialize alternate chipset registers." },
	{ 0x37, "Reinitialize the chipset (MB only)." },
	{ 0x38, "Shadow system BIOS ROM." },
	{ 0x39, "Reinitialize the cache (MB only)." },
	{ 0x3A, "Autosize cache." },
	{ 0x3C, "Configure advanced chipset registers." },
	{ 0x3D, "Load alternate registers with CMOS values." },
	{ 0x40, "Set initial CPU speed." },
	{ 0x42, "Initialize interrupt vectors." },
	{ 0x44, "Initialize BIOS interrupts." },
	{ 0x46, "Check ROM copyright notice." },
	{ 0x47, "Initialize manager for PCI Option ROMs." },
	{ 0x48, "Check video configuration against CMOS." },
	{ 0x49, "Initialize PCI bus and devices." },
	{ 0x4A, "Initialize all video adapters in system." },
	{ 0x4C, "Shadow video BIOS ROM." },
	{ 0x4E, "Display copyright notice." },
	{ 0x50, "Display CPU type and speed." },
	{ 0x51, "Initialize EISA board." },
	{ 0x52, "Test keyboard." },
	{ 0x54, "Set key click if enabled." },
	{ 0x56, "Enable keyboard." },
	{ 0x58, "Test for unexpected interrupts." },
	{ 0x5A, "Display prompt \"Press F2 to enter SETUP\"." },
	{ 0x5C, "Test RAM between 512 and 640k." },
	{ 0x60, "Test extended memory." },
	{ 0x62, "Test extended memory address lines." },
	{ 0x64, "Jump to UserPatch1." },
	{ 0x66, "Configure advanced cache registers." },
	{ 0x68, "Enable external and CPU caches." },
	{ 0x6A, "Display external cache size." },
	{ 0x6C, "Display shadow message." },
	{ 0x6E, "Display non-disposable segments." },
	{ 0x70, "Display error messages." },
	{ 0x72, "Check for configuration errors." },
	{ 0x74, "Test real-time clock." },
	{ 0x76, "Check for keyboard errors." },
	{ 0x7C, "Set up hardware interrupt vectors." },
	{ 0x7E, "Test coprocessor if present." },
	{ 0x80, "Disable onboard I/O ports." },
	{ 0x82, "Detect and install external RS232 ports." },
	{ 0x84, "Detect and install external parallel ports." },
	{ 0x86, "Re-initialize on-board I/O ports." },
	{ 0x88, "Initialize BIOSData Area." },
	{ 0x8A, "Initialize Extended BIOS Data Area." },
	{ 0x8C, "Initialize floppy controller." },
	{ 0x90, "Initialize hard-disk controller." },
	{ 0x91, "Initialize localbus hard-disk controller." },
	{ 0x92, "Jump to UserPatch2." },
	{ 0x93, "Build MPTABLE for multi-processor boards." },
	{ 0x94, "Disable A20 address line." },
	{ 0x96, "Clear huge ES segment register." },
	{ 0x98, "Search for option ROMs." },
	{ 0x9A, "Shadow option ROMs." },
	{ 0x9C, "Set up Power Management." },
	{ 0x9E, "Enable hardware interrupts." },
	{ 0xA0, "Set time of day." },
	{ 0xA2, "Check key lock." },
	{ 0xA4, "Initialize typematic rate." },
	{ 0xA8, "Erase F2 prompt." },
	{ 0xAA, "Scan for F2 keystroke." },
	{ 0xAC, "Enter SETUP." },
	{ 0xAE, "Clear in-POST flag." },
	{ 0xB0, "Check for errors." },
	{ 0xB2, "POST done - prepare to boot operating system." },
	{ 0xB4, "One beep." },
	{ 0xB6, "Check password (optional)." },
	{ 0xB8, "Clear global descriptor table." },
	{ 0xBC, "Clear parity checkers." },
	{ 0xBE, "Clear screen (optional)." },
	{ 0xBF, "Check virus and backup reminders." },
	{ 0xC0, "Try to boot with INT 19." },
	{ 0xD0, "Interrupt handler error." },
	{ 0xD2, "Unknown interrupt error." },
	{ 0xD4, "Pending Interrupt." },
	{ 0xD6, "Initialize option ROM error." },
	{ 0xD8, "Shutdown error." },
	{ 0xDA, "Extended Block Move." },
	{ 0xDC, "Shutdown 10 error." },
	{ 0xE2, "Initialize the chipset." },
	{ 0xE3, "Initialize refresh counter." },
	{ 0xE4, "Check for Forced Flash." },
	{ 0xE5, "Check HW status of ROM." },
	{ 0xE6, "BIOS ROM is OK." },
	{ 0xE7, "Do a complete RAM test." },
	{ 0xE8, "Do OEM initialization." },
	{ 0xE9, "Initialize interrupt controller." },
	{ 0xEA, "Read in the bootstrap code." },
	{ 0xEB, "Initialize all vectors." },
	{ 0xEC, "Boot the Flash program." },
	{ 0xED, "Initialize the boot device." },
	{ 0xEE, "Boot code was read OK." },
	{ 0, nullptr }
};

void pcipc_state::boot_state_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	const char *desc = "";
	for(int i=0; boot_state_infos[i].message; i++)
		if(boot_state_infos[i].val == data) {
			desc = boot_state_infos[i].message;
			break;
		}
	logerror("Boot state %02x - %s\n", data, desc);

}

static MACHINE_CONFIG_START(pcipc, pcipc_state)
	MCFG_CPU_ADD("maincpu", PENTIUM, 66000000)

	MCFG_PCI_ROOT_ADD(    ":pci")
	MCFG_I82439HX_ADD(    ":pci:00.0", ":maincpu", 256*1024*1024)
	MCFG_I82371SB_ISA_ADD(":pci:07.0")
	MCFG_I82371SB_BOOT_STATE_HOOK(DEVWRITE8(":", pcipc_state, boot_state_w))
//  MCFG_IDE_PCI_ADD(     ":pci:07.1", 0x80867010, 0x03, 0x00000000)
	MCFG_MGA2064W_ADD(    ":pci:12.0")
MACHINE_CONFIG_END

ROM_START(pcipc)
	ROM_REGION32_LE(0x40000, ":pci:07.0", 0) /* PC bios */
	ROM_SYSTEM_BIOS(0, "n7ns04", "Version 21/01/98, without integrated sound") \
	ROMX_LOAD("m7ns04.rom", 0x00000, 0x40000, CRC(9c1f656b) SHA1(f4a0a522d8c47b6ddb6c01fe9a34ddf5b1977f8d), ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS(1, "n7s04", "Version 21/01/98, with integrated sound") \
	ROMX_LOAD("m7s04.rom",  0x00000, 0x40000, CRC(3689f5a9) SHA1(8daacdb0dc6783d2161680564ffe83ac2515f7ef), ROM_BIOS(2) ) \
	ROM_SYSTEM_BIOS(2, "crisis", "Version 07/01/98, for flash recovery") \
	ROMX_LOAD("crisis.rom", 0x00000, 0x40000, CRC(38a1458a) SHA1(8881ac336392cca79a772b4168f63efc31f953dd), ROM_BIOS(3) ) \
ROM_END

static INPUT_PORTS_START(pcipc)
INPUT_PORTS_END

COMP(1998, pcipc, 0, 0, pcipc, pcipc, driver_device, 0, "Hack Inc.", "Sandbox PCI PC", MACHINE_NO_SOUND)
