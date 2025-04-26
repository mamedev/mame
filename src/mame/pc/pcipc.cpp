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

#include "bus/isa/isa_cards.h"
#include "bus/pci/pci_slot.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "cpu/i386/i386.h"
#include "machine/fdc37c93x.h"
#include "machine/i82371eb_acpi.h"
#include "machine/i82371eb_ide.h"
#include "machine/i82371eb_isa.h"
#include "machine/i82371eb_usb.h"
#include "machine/i82371sb.h"
#include "machine/i82439hx.h"
#include "machine/i82439tx.h"
#include "machine/i82443bx_host.h"
#include "machine/pci.h"
#include "machine/pci-ide.h"
#include "machine/w83977tf.h"

#include "softlist_dev.h"

// enable ISA verbose messaging at I/O $80
// NOTE: xubuntu 6.10 will ping the port a lot once it gets to GNOME.
#define VERBOSE_ISA_DEBUG 0

namespace {

class pcipc_state : public driver_device
{
public:
	struct boot_state_info {
		uint8_t val;
		const char *const message;
	};

	static const boot_state_info boot_state_infos_phoenix[];
	static const boot_state_info boot_state_infos_phoenix_ver40_rev6[];
	static const boot_state_info boot_state_infos_award[];

	void pcipc(machine_config &config);
	void pcipcs7(machine_config &config);
	void pcipctx(machine_config &config);
	void pcinv3(machine_config &config);
	void pciagp(machine_config &config);

	pcipc_state(const machine_config &mconfig, device_type type, const char *tag);

protected:
	void x86_softlists(machine_config &config);

private:
	void pcipc_map(address_map &map) ATTR_COLD;
	void pcipc_map_io(address_map &map) ATTR_COLD;
	[[maybe_unused]] void boot_state_phoenix_w(uint8_t data);
	void boot_state_phoenix_ver40_rev6_w(uint8_t data);
	void boot_state_award_w(uint8_t data);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	static void smc_superio_config(device_t *device);
	static void winbond_superio_config(device_t *device);
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

const pcipc_state::boot_state_info pcipc_state::boot_state_infos_phoenix[] = {
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
	{ 0x69, "Initialize SMI handler."},
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

const pcipc_state::boot_state_info pcipc_state::boot_state_infos_phoenix_ver40_rev6[] = {
	// Code, "(Beeps) POST Routine Description"
	{ 0x02, "Verify Real Mode." },
	{ 0x03, "Disable Non-Maskable Interrupt (NMI)." },
	{ 0x04, "Get CPU type." },
	{ 0x06, "Initialize system hardware." },
	{ 0x07, "Disable shadow and execute code from the ROM." },
	{ 0x08, "Initialize chipset with initial POST values." },
	{ 0x09, "Set IN POST flag." },
	{ 0x0A, "Initialize CPU registers." },
	{ 0x0B, "Enable CPU cache." },
	{ 0x0C, "Initialize caches to initial POST values." },
	{ 0x0E, "Initialize I/O component." },
	{ 0x0F, "Initialize the local bus IDE." },
	{ 0x10, "Initialize Power Management." },
	{ 0x11, "Load alternate registers with initial POST values." },
	{ 0x12, "Restore CPU control word during warm boot." },
	{ 0x13, "Initialize PCI Bus Mastering devices." },
	{ 0x14, "Initialize keyboard controller." },
	{ 0x16, "(1-2-2-3) BIOS ROM checksum." },
	{ 0x17, "Initialize cache before memory Auto size." },
	{ 0x18, "8254 timer initialization." },
	{ 0x1A, "8237 DMA controller initialization." },
	{ 0x1C, "Reset Programmable Interrupt Controller." },
	{ 0x20, "(1-3-1-1) Test DRAM refresh." },
	{ 0x22, "(1-3-1-3) Test 8742 Keyboard Controller." },
	{ 0x24, "Set ES segment register to 4 GB." },
	{ 0x28, "Auto size DRAM." },
	{ 0x29, "Initialize POST Memory Manager." },
	{ 0x2A, "Clear 512 kB base RAM." },
	{ 0x2C, "(1-3-4-1) RAM failure on address line xxxx*." },
	{ 0x2E, "(1-3-4-3) RAM failure on data bits xxxx* of low byte of memory bus." },
	{ 0x2F, "Enable cache before system BIOS shadow." },
	{ 0x32, "Test CPU bus-clock frequency." },
	{ 0x33, "Initialize Phoenix Dispatch Manager." },
	{ 0x36, "Warm start shut down." },
	{ 0x38, "Shadow system BIOS ROM." },
	{ 0x3A, "Auto size cache." },
	{ 0x3C, "Advanced configuration of chipset registers." },
	{ 0x3D, "Load alternate registers with CMOS values." },
	{ 0x41, "Initialize extended memory for RomPilot." },
	{ 0x42, "Initialize interrupt vectors." },
	{ 0x45, "POST device initialization." },
	{ 0x46, "(2-1-2-3) Check ROM copyright notice." },
	{ 0x47, "Initialize I20 support." },
	{ 0x48, "Check video configuration against CMOS." },
	{ 0x49, "Initialize PCI bus and devices." },
	{ 0x4A, "Initialize all video adapters in system." },
	{ 0x4B, "QuietBoot start (optional)." },
	{ 0x4C, "Shadow video BIOS ROM." },
	{ 0x4E, "Display BIOS copyright notice." },
	{ 0x4F, "Initialize MultiBoot." },
	{ 0x50, "Display CPU type and speed." },
	{ 0x51, "Initialize EISA board." },
	{ 0x52, "Test keyboard." },
	{ 0x54, "Set key click if enabled." },
	{ 0x55, "Enable USB devices." },
	{ 0x58, "(2-2-3-1) Test for unexpected interrupts." },
	{ 0x59, "Initialize POST display service." },
	{ 0x5A, "Display prompt 'Press F2 to enter SETUP'." },
	{ 0x5B, "Disable CPU cache." },
	{ 0x5C, "Test RAM between 512 and 640 kB." },
	{ 0x60, "Test extended memory." },
	{ 0x62, "Test extended memory address lines." },
	{ 0x64, "Jump to UserPatch1." },
	{ 0x66, "Configure advanced cache registers." },
	{ 0x67, "Initialize Multi Processor APIC." },
	{ 0x68, "Enable external and CPU caches." },
	{ 0x69, "Setup System Management Mode (SMM) area." },
	{ 0x6A, "Display external L2 cache size." },
	{ 0x6B, "Load custom defaults (optional)." },
	{ 0x6C, "Display shadow-area message." },
	{ 0x6E, "Display possible high address for UMB recovery." },
	{ 0x70, "Display error messages." },
	{ 0x72, "Check for configuration errors." },
	{ 0x76, "Check for keyboard errors." },
	{ 0x7C, "Set up hardware interrupt vectors." },
	{ 0x7D, "Initialize Intelligent System Monitoring." },
	{ 0x7E, "Initialize coprocessor if present." },
	{ 0x80, "Disable onboard Super I/O ports and IRQs." },
	{ 0x81, "Late POST device initialization." },
	{ 0x82, "Detect and install external RS232 ports." },
	{ 0x83, "Configure non-MCD IDE controllers." },
	{ 0x84, "Detect and install external parallel ports." },
	{ 0x85, "Initialize PC-compatible PnP ISA devices." },
	{ 0x86, "Re-initialize onboard I/O ports." },
	{ 0x87, "Configure Motherboard Configurable Devices (optional)." },
	{ 0x88, "Initialize BIOS Data Area." },
	{ 0x89, "Enable Non-Maskable Interrupts (NMIs)." },
	{ 0x8A, "Initialize Extended BIOS Data Area." },
	{ 0x8B, "Test and initialize PS/2 mouse." },
	{ 0x8C, "Initialize floppy controller." },
	{ 0x8F, "Determine number of ATA drives (optional)." },
	{ 0x90, "Initialize hard-disk controllers." },
	{ 0x91, "Initialize local-bus hard-disk controllers." },
	{ 0x92, "Jump to UserPatch2." },
	{ 0x93, "Build MPTABLE for multi-processor boards." },
	{ 0x95, "Install CD ROM for boot." },
	{ 0x96, "Clear huge ES segment register." },
	{ 0x97, "Fix up Multi Processor table." },
	{ 0x98, "(1-2) Search for option ROMs. One long, two short beeps on checksum failure." },
	{ 0x99, "Check for SMART Drive (optional)." },
	{ 0x9A, "Shadow option ROMs." },
	{ 0x9C, "Set up Power Management." },
	{ 0x9D, "Initialize security engine (optional)." },
	{ 0x9E, "Enable hardware interrupts." },
	{ 0x9F, "Determine number of ATA and SCSI drives." },
	{ 0xA0, "Set time of day." },
	{ 0xA2, "Check key lock." },
	{ 0xA4, "Initialize typematic rate." },
	{ 0xA8, "Erase F2 prompt." },
	{ 0xAA, "Scan for F2 key stroke." },
	{ 0xAC, "Enter SETUP." },
	{ 0xAE, "Clear Boot flag." },
	{ 0xB0, "Check for errors." },
	{ 0xB1, "Inform RomPilot about the end of POST." },
	{ 0xB2, "POST done - prepare to boot operating system." },
	{ 0xB4, "1 One short beep before boot." },
	{ 0xB5, "Terminate QuietBoot (optional)." },
	{ 0xB6, "Check password (optional)." },
	{ 0xB7, "Initialize ACPI BIOS." },
	{ 0xB9, "Prepare Boot." },
	{ 0xBA, "Initialize SMBIOS." },
	{ 0xBB, "Initialize PnP Option ROMs." },
	{ 0xBC, "Clear parity checkers." },
	{ 0xBD, "Display MultiBoot menu." },
	{ 0xBE, "Clear screen (optional)." },
	{ 0xBF, "Check virus and backup reminders." },
	{ 0xC0, "Try to boot with INT 19." },
	{ 0xC1, "Initialize POST Error Manager (PEM)." },
	{ 0xC2, "Initialize error logging." },
	{ 0xC3, "Initialize error display function." },
	{ 0xC4, "Initialize system error handler." },
	{ 0xC5, "PnPnd dual CMOS (optional)." },
	{ 0xC6, "Initialize note dock (optional)." },
	{ 0xC7, "Initialize note dock late." },
	{ 0xC8, "Force check (optional)." },
	{ 0xC9, "Extended checksum (optional)." },
	{ 0xCA, "Redirect Int 15h to enable remote keyboard." },
	{ 0xCB, "Redirect Int 13h to Memory Technologies Devices such as ROM, RAM, PCMCIA, and serial disk." },
	{ 0xCC, "Redirect Int 10h to enable remote serial video." },
	{ 0xCD, "Re-map I/O and memory for PCMCIA." },
	{ 0xCE, "Initialize digitizer and display message." },
	{ 0xD2, "Unknown interrupt." },
	// The following are for boot block in Flash ROM
	{ 0xE0, "Initialize the chipset." },
	{ 0xE1, "Initialize the bridge." },
	{ 0xE2, "Initialize the CPU." },
	{ 0xE3, "Initialize system timer." },
	{ 0xE4, "Initialize system I/O." },
	{ 0xE5, "Check force recovery boot." },
	{ 0xE6, "Checksum BIOS ROM." },
	{ 0xE7, "Go to BIOS." },
	{ 0xE8, "Set Huge Segment." },
	{ 0xE9, "Initialize Multi Processor." },
	{ 0xEA, "Initialize OEM special code." },
	{ 0xEB, "Initialize PIC and DMA." },
	{ 0xEC, "Initialize Memory type." },
	{ 0xED, "Initialize Memory size." },
	{ 0xEE, "Shadow Boot Block." },
	{ 0xEF, "System memory test." },
	{ 0xF0, "Initialize interrupt vectors." },
	{ 0xF1, "Initialize Run Time Clock." },
	{ 0xF2, "Initialize video." },
	{ 0xF3, "Initialize System Management Manager." },
	{ 0xF4, "Output one beep." },
	{ 0xF5, "Clear Huge Segment." },
	{ 0xF6, "Boot to Mini DOS." },
	{ 0xF7, "Boot to Full DOS." },
	{ 0, nullptr }
};

const pcipc_state::boot_state_info pcipc_state::boot_state_infos_award[] = {
	{0x01, "Processor test; Processor status verification" },
	{0x02, "Processor test 2; Read/Write and verify all CPU registers" },
	{0x03, "Initialize chips; Disable NMI, PIE, AIE, UEI, SQWV.  Disable video, parity checking, DMA.  Reset math coprocessor.  Clear all page registers and CMOS shutdown.  Initialize DMA controller 0 and 1.  Initialize interrupt controllers 0 and 1." },
	{0x04, "Test memory refresh toggle" },
	{0x05, "Blank video, initialize keyboard; Keyboard controller initialization" },
	{0x07, "Test CMOS interface and battery" },
	{0x08, "Set up low memory; Early chipset initialization, memory presence test, OEM chipset routines, clear low 64K memory, test first 64K memory" },
	{0x09, "Early cache initialization; Cyrix CPU specific, CPU and cache initialization" },
	{0x0A, "Set up interrupt vector table; Initialize first 120 interrupt vectors" },
	{0x0B, "Test CMOS RAM checksum" },
	{0x0C, "Initialize keyboard; Detect the type of keyboard controller" },
	{0x0D, "Initialize video interface; Detect CPU clock, read CMOS location 14h to find the type of video in use, detect and initialize video adapter" },
	{0x0E, "Test video memory; Write sign-on message to screen, setup shadow RAM" },
	{0x0F, "Test DMA controller 0; BIOS checksum test, keyboard detect and initialization" },
	{0x10, "Test DMA controller 1" },
	{0x11, "Test DMA page registers" },
	//{0x12-13, "Reserved" },
	{0x14, "Test timer counter 2" },
	{0x15, "Test 8259-1 mask bits" },
	{0x16, "Test 8259-2 mask bits" },
	{0x17, "Test stuck 8259 interrupt bits; Test stuck key" },
	{0x18, "Test 8259 interrupt functionality" },
	{0x19, "Test stuck NMI bits (parity I/O check)" },
	{0x1A, "Benchmark; Display CPU clock" },
	//{0x1B-1E, "Reserved" },
	{0x1F, "Set EISA mode; If the EISA memory checksum is good then EISA is initialized.  If it's not good then ISA tests and clear EISA mode flag" },
	{0x20, "Enable slot 0; System board" },
	//{0x21-2F, "Enable slots 1-15" },
	{0x30, "Size base and extended memory; Size the base memory from 256K to 640K and the extended memory above 1MB" },
	{0x31, "Test base and extended memory; Test the base memory from 256K to 640K and the extended memory above 1MB using various bit patterns" },
	{0x32, "Test EISA extended memory" },
	//{0x33-3B, "Reserved" },
	{0x3C, "Setup enabled" },
	{0x3D, "Initialize and install mouse if present" },
	{0x3E, "Setup cache controller" },
	{0x40, "Display virus protect disable or enable" },
	{0x41, "Initialize floppy" },
	{0x42, "Initialize hard drive" },
	{0x43, "Detect & Init. serial & parallel ports" },
	{0x44, "Reserved" },
	{0x45, "Detect and Init. math coprocessor" },
	{0x46, "Reserved" },
	{0x47, "Reserved" },
	//{0x48-4D, "Reserved" },
	{0x4E, "Mfg. POST loop, or display messages" },
	{0x4F, "Security password" },
	{0x50, "Write CMOS; Write CMOS back to RAM and clear screen" },
	{0x51, "Pre-boot enable; Enable parity checking, enable NMI, enable cache before boot" },
	{0x52, "Initialize option ROM's; Initialize and ROM's present at locations C800h to EFFFFh" },
	{0x53, "Initialize time value" },
	{0x60, "Setup virus protect" },
	{0x61, "Set boot speed" },
	{0x62, "Setup numlock" },
	{0x63, "Boot attempt" },
	{0xB0, "Spurious" },
	{0xB1, "Unclaimed NMI" },
	{0xBE, "Chipset default initialization; Program chipset registers and power-on BIOS defaults." },
	{0xBF, "Chipset initialization; Reserved" },
	{0xC0, "Turn off chipset cache" },
	{0xC1, "Memory presence test; OEM specific, test the size of on-board memory" },
	{0xC5, "Early shadow; OEM specific, early shadow enable for fast boot" },
	{0xC6, "Cache presence test; External cache-size detection test" },
	//{0xE1-EF, "Setup pages" },
	{0xFF, "Boot loader" },
	{ 0, nullptr }
};

void pcipc_state::boot_state_phoenix_w(uint8_t data)
{
#if VERBOSE_ISA_DEBUG
	const char *desc = "";
	for(int i=0; boot_state_infos_phoenix[i].message; i++)
		if(boot_state_infos_phoenix[i].val == data) {
			desc = boot_state_infos_phoenix[i].message;
			break;
		}
	logerror("Boot state %02x - %s\n", data, desc);
#endif
}

void pcipc_state::boot_state_phoenix_ver40_rev6_w(uint8_t data)
{
#if VERBOSE_ISA_DEBUG
	const char *desc = "";
	for(int i=0; boot_state_infos_phoenix_ver40_rev6[i].message; i++)
		if(boot_state_infos_phoenix_ver40_rev6[i].val == data) {
			desc = boot_state_infos_phoenix_ver40_rev6[i].message;
			break;
		}
	logerror("Boot state %02x - %s\n", data, desc);
#endif
}


void pcipc_state::boot_state_award_w(uint8_t data)
{
#if VERBOSE_ISA_DEBUG
	const char *desc = "";
	for(int i=0; boot_state_infos_award[i].message; i++)
		if(boot_state_infos_award[i].val == data) {
			desc = boot_state_infos_award[i].message;
			break;
		}
	logerror("Boot state %02x - %s\n", data, desc);
#endif
}

static void isa_internal_devices(device_slot_interface &device)
{
	device.option_add("fdc37c93x", FDC37C93X);
	device.option_add("w83977tf", W83977TF);
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

void pcipc_state::smc_superio_config(device_t *device)
{
	fdc37c93x_device &fdc = *downcast<fdc37c93x_device *>(device);
	fdc.set_sysopt_pin(1);
	fdc.gp20_reset().set_inputline(":maincpu", INPUT_LINE_RESET);
	fdc.gp25_gatea20().set_inputline(":maincpu", INPUT_LINE_A20);
	fdc.irq1().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq1_w));
	fdc.irq8().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq8n_w));
	fdc.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
	fdc.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
	fdc.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
	fdc.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
	fdc.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
	fdc.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));
}

void pcipc_state::winbond_superio_config(device_t *device)
{
	w83977tf_device &fdc = *downcast<w83977tf_device *>(device);
//  fdc.set_sysopt_pin(1);
	fdc.gp20_reset().set_inputline(":maincpu", INPUT_LINE_RESET);
	fdc.gp25_gatea20().set_inputline(":maincpu", INPUT_LINE_A20);
	fdc.irq1().set(":pci:07.0", FUNC(i82371eb_isa_device::pc_irq1_w));
	fdc.irq8().set(":pci:07.0", FUNC(i82371eb_isa_device::pc_irq8n_w));
//  fdc.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
//  fdc.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
//  fdc.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
//  fdc.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
//  fdc.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
//  fdc.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));
}


void pcipc_state::pcipc_map(address_map &map)
{
	map.unmap_value_high();
}

void pcipc_state::pcipc_map_io(address_map &map)
{
	map.unmap_value_high();
}

void pcipc_state::x86_softlists(machine_config &config)
{
	/* software lists */
	SOFTWARE_LIST(config, "pc_disk_list").set_original("ibm5150");
	SOFTWARE_LIST(config, "at_disk_list").set_original("ibm5170");
	SOFTWARE_LIST(config, "at_cdrom_list").set_original("ibm5170_cdrom");
	SOFTWARE_LIST(config, "at_hdd_list").set_original("ibm5170_hdd");
	SOFTWARE_LIST(config, "midi_disk_list").set_compatible("midi_flop");
	SOFTWARE_LIST(config, "photocd_list").set_compatible("photo_cd");
}

void pcipc_state::pcipc(machine_config &config)
{
	pentium_device &maincpu(PENTIUM(config, "maincpu", 90000000));
	maincpu.set_addrmap(AS_PROGRAM, &pcipc_state::pcipc_map);
	maincpu.set_addrmap(AS_IO, &pcipc_state::pcipc_map_io);
	maincpu.set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
	maincpu.smiact().set("pci:00.0", FUNC(i82439hx_host_device::smi_act_w));

	PCI_ROOT(config, "pci", 0);
	I82439HX(config, "pci:00.0", 0, "maincpu", 256*1024*1024);

	i82371sb_isa_device &isa(I82371SB_ISA(config, "pci:07.0", 0, "maincpu"));
	isa.boot_state_hook().set(FUNC(pcipc_state::boot_state_phoenix_ver40_rev6_w));
	isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);

	i82371sb_ide_device &ide(I82371SB_IDE(config, "pci:07.1", 0, "maincpu"));
	ide.irq_pri().set("pci:07.0", FUNC(i82371sb_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(i82371sb_isa_device::pc_mirq0_w));

	PCI_SLOT(config, "pci:1", pci_cards, 15, 0, 1, 2, 3, nullptr);
	PCI_SLOT(config, "pci:2", pci_cards, 16, 1, 2, 3, 0, nullptr);
	PCI_SLOT(config, "pci:3", pci_cards, 17, 2, 3, 0, 1, nullptr);
	PCI_SLOT(config, "pci:4", pci_cards, 18, 3, 0, 1, 2, "virge");

	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "fdc37c93x", true).set_option_machine_config("fdc37c93x", smc_superio_config);
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

	rs232_port_device &serport0(RS232_PORT(config, "serport0", isa_com, nullptr));
	serport0.rxd_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::rxd1_w));
	serport0.dcd_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ndcd1_w));
	serport0.dsr_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ndsr1_w));
	serport0.ri_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::nri1_w));
	serport0.cts_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::rxd2_w));
	serport1.dcd_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ndcd2_w));
	serport1.dsr_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ndsr2_w));
	serport1.ri_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::nri2_w));
	serport1.cts_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ncts2_w));

	//  SW1000XG(config, "pci:11.0");

	x86_softlists(config);
}

void pcipc_state::pcipcs7(machine_config &config)
{
	pcipc_state::pcipc(config);
	pentium_mmx_device &maincpu(PENTIUM_MMX(config.replace(), "maincpu", 266'000'000)); // socket 7 CPU
	maincpu.set_addrmap(AS_PROGRAM, &pcipc_state::pcipc_map);
	maincpu.set_addrmap(AS_IO, &pcipc_state::pcipc_map_io);
	maincpu.set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
	maincpu.smiact().set("pci:00.0", FUNC(i82439hx_host_device::smi_act_w));
}

void pcipc_state::pcipctx(machine_config &config)
{
	pentium_device &maincpu(PENTIUM(config, "maincpu", 60000000));
	maincpu.set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));

	PCI_ROOT(config, "pci", 0);
	I82439TX(config, "pci:00.0", 0, "maincpu", 256*1024*1024);

	i82371sb_isa_device &isa(I82371SB_ISA(config, "pci:07.0", 0, "maincpu"));
	isa.boot_state_hook().set(FUNC(pcipc_state::boot_state_award_w));
//  IDE_PCI(config, "pci:07.1", 0, 0x80867010, 0x03, 0x00000000);

	PCI_SLOT(config, "pci:1", pci_cards, 15, 0, 1, 2, 3, nullptr);
	PCI_SLOT(config, "pci:2", pci_cards, 16, 1, 2, 3, 0, nullptr);
	PCI_SLOT(config, "pci:3", pci_cards, 17, 2, 3, 0, 1, nullptr);
	PCI_SLOT(config, "pci:4", pci_cards, 18, 3, 0, 1, 2, "mga2064w");

	x86_softlists(config);
}

void pcipc_state::pciagp(machine_config &config)
{
	// TODO: starts at 233'000'000, consider adding FSB & AGP clocks here
	pentium2_device &maincpu(PENTIUM2(config, "maincpu", 90'000'000));
	maincpu.set_addrmap(AS_PROGRAM, &pcipc_state::pcipc_map);
	maincpu.set_addrmap(AS_IO, &pcipc_state::pcipc_map_io);
	maincpu.set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
	maincpu.smiact().set("pci:00.0", FUNC(i82443bx_host_device::smi_act_w));

	PCI_ROOT(config, "pci", 0);
	I82443BX_HOST(config, "pci:00.0", 0, "maincpu", 128*1024*1024);
	I82443BX_BRIDGE(config, "pci:01.0", 0 );

	i82371eb_isa_device &isa(I82371EB_ISA(config, "pci:07.0", 0, "maincpu"));
	isa.boot_state_hook().set(FUNC(pcipc_state::boot_state_award_w));
	isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);

	i82371eb_ide_device &ide(I82371EB_IDE(config, "pci:07.1", 0, "maincpu"));
	ide.irq_pri().set("pci:07.0", FUNC(i82371eb_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(i82371eb_isa_device::pc_mirq0_w));

	I82371EB_USB (config, "pci:07.2", 0);
	I82371EB_ACPI(config, "pci:07.3", 0);
	LPC_ACPI     (config, "pci:07.3:acpi", 0);
	SMBUS        (config, "pci:07.3:smbus", 0);

	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "w83977tf", true).set_option_machine_config("w83977tf", winbond_superio_config);
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

#if 0
	rs232_port_device &serport0(RS232_PORT(config, "serport0", isa_com, nullptr));
	serport0.rxd_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::rxd1_w));
	serport0.dcd_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ndcd1_w));
	serport0.dsr_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ndsr1_w));
	serport0.ri_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::nri1_w));
	serport0.cts_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::rxd2_w));
	serport1.dcd_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ndcd2_w));
	serport1.dsr_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ndsr2_w));
	serport1.ri_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::nri2_w));
	serport1.cts_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ncts2_w));
#endif

	// FIXME: int mapping is unchecked for all slots
	PCI_SLOT(config, "pci:01.0:1", agp_cards, 1, 0, 1, 2, 3, "riva128");

	PCI_SLOT(config, "pci:1", pci_cards, 15, 1, 2, 3, 0, nullptr);
	PCI_SLOT(config, "pci:2", pci_cards, 16, 1, 2, 3, 0, nullptr);
	PCI_SLOT(config, "pci:3", pci_cards, 17, 2, 3, 0, 1, nullptr);
	PCI_SLOT(config, "pci:4", pci_cards, 18, 3, 0, 1, 2, nullptr);

	x86_softlists(config);
}

ROM_START(pcipc)
	ROM_REGION32_LE(0x40000, "pci:07.0", 0) /* PC bios */
	ROM_SYSTEM_BIOS(0, "m55ns04", "m55ns04") // Micronics M55HI-Plus with no sound
	ROMX_LOAD("m55-04ns.rom", 0x20000, 0x20000, CRC(0116b2b0) SHA1(19b0203decfd4396695334517488d488aec3ccde), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "m55s04", "m55s04") // with sound
	ROMX_LOAD("m55-04s.rom", 0x20000, 0x20000, CRC(34a7422e) SHA1(68753fe373c97844beff83ea75c634c77cfedb8f), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "crisis", "Version 07/01/98, for flash recovery")
	ROMX_LOAD("crisis.rom", 0x00000, 0x40000, CRC(38a1458a) SHA1(8881ac336392cca79a772b4168f63efc31f953dd), ROM_BIOS(2) )
	// FIXME: this is incompatible, it's a Gigabyte GA-586HX with W83877F Super I/O
	ROM_SYSTEM_BIOS(3, "5hx29", "5hx29")
	ROMX_LOAD("5hx29.bin",   0x20000, 0x20000, CRC(07719a55) SHA1(b63993fd5186cdb4f28c117428a507cd069e1f68), ROM_BIOS(3) )
//  ROM_REGION(0x8000,"ibm_vga", 0)
//  ROM_LOAD("ibm-vga.bin", 0x00000, 0x8000, BAD_DUMP CRC(74e3fadb) SHA1(dce6491424f1726203776dfae9a967a98a4ba7b5) )
ROM_END

ROM_START(pcipctx)
	ROM_REGION32_LE(0x40000, "pci:07.0", 0) /* PC bios */
	ROM_SYSTEM_BIOS(0, "ga586t2", "Gigabyte GA-586T2") // ITE 8679 I/O
	ROMX_LOAD("gb_ga586t2.bin",  0x20000, 0x20000, CRC(3a50a6e1) SHA1(dea859b4f1492d0d08aacd260ed1e83e00ebac08), ROM_BIOS(0))

	ROM_REGION(0x8000,"ibm_vga", 0)
	ROM_LOAD("ibm-vga.bin", 0x00000, 0x8000, BAD_DUMP CRC(74e3fadb) SHA1(dce6491424f1726203776dfae9a967a98a4ba7b5) )
ROM_END

ROM_START(pciagp)
	ROM_REGION32_LE(0x40000, "pci:07.0", 0) /* PC bios */
	// a.k.a. the BIOS present in savquest.cpp
	ROM_SYSTEM_BIOS(0, "dfi_p2xbl", "Octek Rhino BX-ATX")
	ROMX_LOAD( "p2xbl_award_451pg.bin", 0x00000, 0x040000, CRC(37d0030e) SHA1(c6773d0e02325116f95c497b9953f59a9ac81317), ROM_BIOS(0) )
ROM_END

#define rom_pcipcs7    rom_pcipc

static INPUT_PORTS_START(pcipc)
INPUT_PORTS_END

} // anonymous namespace


COMP(1998, pcipc,    0,     0, pcipc,   pcipc, pcipc_state, empty_init, "Hack Inc.", "Sandbox PCI PC (430HX)", MACHINE_NO_SOUND )
COMP(1998, pcipcs7,  pcipc, 0, pcipcs7, pcipc, pcipc_state, empty_init, "Hack Inc.", "Sandbox PCI PC (430HX, Socket 7 CPU)", MACHINE_NO_SOUND ) // alternative of above, for running already installed OSes at their nominal speed + fiddling with MMX
COMP(1998, pcipctx,  0,     0, pcipctx, pcipc, pcipc_state, empty_init, "Hack Inc.", "Sandbox PCI PC (430TX)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // unemulated super I/O
COMP(1999, pciagp,   0,     0, pciagp,  pcipc, pcipc_state, empty_init, "Hack Inc.", "Sandbox PCI/AGP PC (440BX)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // errors out with ISA state $05 (keyboard, blame 8042kbdc.cpp) bp e140c,1,{eax&=~1;g}) does stuff if bypassed but eventually PnP breaks OS booting
