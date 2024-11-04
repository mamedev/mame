// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
/***************************************************************************

    IBM AT Compatibles

Commodore PC 30-III and PC 40-III
=================================
Links: http://www.richardlagendijk.nl/cip/computer/item/pc30iii/en , ftp://ftp.zimmers.net/pub/cbm-pc/firmware/pc30/
Info: The PC 30-III and PC 40-III share the same mainboard. On a PC 30-III the onboard Paradise VGA is not populated.
Form factor: Desktop PC
CPU: Siemens SAB 80286-12 (PC 30-III), Intel 80286-12 (PC 40-III)
RAM: 1MB on board
Chipset: Faraday FE3020, MOS 5720 1788 41, Faraday FE3000, FE3010B,
Bus: 3x16 bit ISA, 1x8 bit ISA
Video: PC 30-III: ATI EGA Wonder 800+, PC 40-III: Onboard Paradise VGA, 256KB
Mass storage: One HD disk drive standard, second drive optional; PC 30-III: 20MB, PC 40-III: 40MB AT-IDE HD standard, 80MB or 100MB optional
On board: Serial, Parallel, Commodore 1532 Mouse port (MS Bus mouse compatible), Keyboard, Beeper, Floppy (2 devices), AT-IDE (1 device)
Options: 80287

Sanyo MBC-28
============
Links: http://www.cc-computerarchiv.de/CC-Archiv/bc-alt/gb-san/gb-san-12_91.html
Form factor: Desktop
CPU: 80386sx-20
RAM: 1MB - 8MB on board
Mass storage: 1.44MB Floppy disk drive and 80MB IDE hard disk
On board: 2xserial, parallel, bus mouse, keyboard
To-Do: Complains about missing mouse hardware (Bus Mouse), hangs in POST

Siemens PCD-2
=============
Links: http://www.z80.eu/siemenspcd2.html , http://www.z80.eu/downloads/Siemens_PCD-2_SW-Monitor-Buchse-Belegung.pdf , https://www.computerwoche.de/a/at-klon-und-lan-ergaenzen-siemens-palette,1166395
Form factor: low profile desktop
CPU: 80286-12 on a Tandon supplied slot CPU card
RAM: 1MB - 4MB in four SIMM modules
Mass storage: 1.2MB Floppy disk drive and 20MB or 40MB MFM harddisk
Bus: Vertical passive ISA backplane with six slots
On board: 2xserial, parallel, floppy, keyboard, RTC, MFM harddisk controller piggybacked to bus extension on slot CPU
Options: 80287

Compaq Portable II
==================
Links: http://tkc8800.com/post/compaq-portable-ii-restoration , https://www.seasip.info/VintagePC/compaq2.html , https://en.wikipedia.org/wiki/Compaq_Portable_II
Form factor: Luggable
CPU: 80286-8
RAM: 256K or 640K on board, 512kB and 2048kB ISA memory cards and 512kB and 1536kB memory boards that attached to the back of the motherboard, 4.2M max.
Display: Green-screen CRT
Mass storage: one or two 5.25" floppy drives, 10MB or 20MB mfm harddisk connected via an MFM=>IDE bridgeboard
Bus: two 8bit and two 16bit ISA slots
On board: Serial, parallel
Standard cards: Floppy/IDE combo card, special Compaq CGA/MDA hybrid video card
Options: Compaq EGA card (drives internal monitor), 80287, floppy drives (360K, 1.2M, 1.44M)

Compaq Portable III
===================
Links: http://www.old-computers.com/museum/computer.asp?c=1064 , http://www.freakedenough.at/infoseiten/read.php?id=66 , http://www.1000bit.it/ad/bro/compaq/CompaqProtable3.pdf , http://oldcomputers.net/compaqiii.pdf
Info: The later Compaq Portable 386 uses the same case, screen and video adapter; Models: 1 (no), 20 (20MB) and 40 (40MB harddisk)
Form factor: Luggable
CPU: AMD N80L286-12/S 12MHz (could be downclocked to 8MHz)
RAM: 640KB, attitional RAM cards were 512KB or 2MB to give 1.1MB, 1.6MB, 2.1MB, 2.6MB, 4.6MB or 6.6MB of total RAM
Video: AT&T 6300/Olivetti M24 driver compatible "Super CGA" with a 640x400 red/amber Plasma screen
Mass storage: One 1.2MB floppy disk drive, no/20MB/40MB hard disk
On board: Serial, Parallel, RTC, RGBI (external Monitor), keyboard
Options: 80827, Expansion box with 2 ISA slots, 300/1200Baud internal Modem, Compaq EGA Board
To-Do: Emulate Graphics card fully

Ericsson/Nokia Data/ICL WS286
=============================
Links: http://oju.mbnet.fi/retro/EricssonPC_eng.html
Info: WS286 was introduced 1986 as first 8Mhz AT in the world a few weeks ahead competition, aquired by Nokia Data 1988 which in turn was aquired by ICL 1990
Form factor: Desktop PC
CPU: Intel 286, 8MHz
RAM: 640KB
Mass storage: Floppy: 5.25" 1.2Mb, HDD: 40Mb

Nixdorf 8810 M55
================
Links: https://www.computerwoche.de/a/auch-nixdorf-nun-in-der-at-clone-riege,1166613
Info: Rebadged NCR PC-8, an AT-clone in a huge desktop case
Form factor: Desktop PC
CPU: Intel 286; CPU card has a 20Mhz, a 12 MHz and a 14.31818 crystal
RAM: 512K on CPU card, 128K on a piggyback card and a memory expansion board
Bus: Passive backplane, ISA
Video: Paradise EGA on another piggyback board
Mass storage: Floppy: 5.25" 1.2MB, MFM HDD

Lion 3500C/T
==========
Info: BIOS saved according to http://mess.redump.net/dumping/dump_bios_using_debug from a 3560C machine
Form factor: notebook
CPU: Intel 486DX2-66
RAM: 2MB, 4MB, 8MB or 16MB
Chipset: ETEQ ET/486H (ET82C491 & ET82C492), 82C206, 82C712
ROM: 128K Video (E0000-EFFFF) & BIOS ROM  (F0000-FFFFF)
Video: Cirrus Logic GD-6420BF/6430 6342 internal VGA, 640x480 256 color display
Mass storage: Floppy 3.5" 1.44MB, 3.5" HDD, 120MB
Input: Trackball connected as a PS/2 mouse
Options: 100 pin expansion port for 3305 Docking station (2xISA16 slots), external keypad
Ports: External VGA, external keyboard, COM1, external keypad, COM2, LPT1, buzzer
Variants: T denotes an active 8.4" display, C a passive 9.5" color display. 3560T/C (486DX2-66), 3530T/C(486DX2-50), 3500T/C (486DX-33), 3500SXT/SXC(486SX-25)

***************************************************************************/

#include "emu.h"

/* mingw-gcc defines this */
#ifdef i386
#undef i386
#endif /* i386 */

#include "bus/isa/isa_cards.h"
#include "bus/lpci/pci.h"
#include "bus/lpci/vt82c505.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/i386/i386.h"
#include "cpu/i86/i286.h"
#include "machine/at.h"
#include "machine/cs8221.h"
#include "machine/ds128x.h"
#include "machine/idectrl.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/vt82c496.h"
#include "machine/wd7600.h"
#include "emupal.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class at_state : public driver_device
{
public:
	at_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb(*this, "mb"),
		m_ram(*this, RAM_TAG)
	{ }

	void pc30iii(machine_config &config);
	void k286i(machine_config &config);
	void ibm5170(machine_config &config);
	void ct386sx(machine_config &config);
	void xb42639(machine_config &config);
	void at486l(machine_config &config);
	void comportii(machine_config &config);
	void comportiii(machine_config &config);
	void comslt286(machine_config &config);
	void dsys200(machine_config &config);
	void ibm5162(machine_config &config);
	void neat(machine_config &config);
	void at386l(machine_config &config);
	void ibm5170a(machine_config &config);
	void ec1842(machine_config &config);
	void at486(machine_config &config);
	void ficpio2(machine_config &config);
	void at386sx(machine_config &config);
	void pc40iii(machine_config &config);
	void pc45iii(machine_config &config);
	void c286lt(machine_config &config);
	void csl286(machine_config &config);
	void c386sx16(machine_config &config);
	void atturbo(machine_config &config);
	void at386(machine_config &config);
	void ncrpc8(machine_config &config);
	void n8810m15(machine_config &config);
	void n8810m55(machine_config &config);
	void ews286(machine_config &config);
	void olyport40(machine_config &config);
	void micral45(machine_config &config);
	void euroat(machine_config &config);
	void pg750(machine_config &config);

	void init_at();
	void init_atpci();

protected:
	required_device<cpu_device> m_maincpu;
	required_device<at_mb_device> m_mb;
	required_device<ram_device> m_ram;
	uint16_t ps1_unk_r(offs_t offset);
	void ps1_unk_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t ps1_portb_r();

	void init_at_common(int xmsbase);
	uint16_t m_ps1_reg[2];

	static void cfg_single_360K(device_t *device);
	static void cfg_single_1200K(device_t *device);
	static void cfg_single_1440K(device_t *device);
	static void cfg_dual_1440K(device_t *device);
	void at16_io(address_map &map) ATTR_COLD;
	void at16_map(address_map &map) ATTR_COLD;
	void at16l_map(address_map &map) ATTR_COLD;
	void at32_io(address_map &map) ATTR_COLD;
	void at32_map(address_map &map) ATTR_COLD;
	void at32l_map(address_map &map) ATTR_COLD;
	void ficpio_io(address_map &map) ATTR_COLD;
	void ficpio_map(address_map &map) ATTR_COLD;
	void neat_io(address_map &map) ATTR_COLD;
	void ps1_16_io(address_map &map) ATTR_COLD;
};

class at_vrom_fix_state : public at_state
{
public:
	using at_state::at_state;

	void init_megapcpla();

	void ibmps1(machine_config &config);
	void megapcpla(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
};

class megapc_state : public driver_device
{
public:
	megapc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_wd7600(*this, "wd7600"),
		m_isabus(*this, "isabus"),
		m_speaker(*this, "speaker")
	{ }

	void megapcpl(machine_config &config);
	void megapc(machine_config &config);

	void init_megapc();
	void init_megapcpl();

private:
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<wd7600_device> m_wd7600;
	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;

	uint16_t wd7600_ior(offs_t offset);
	void wd7600_iow(offs_t offset, uint16_t data);
	void wd7600_hold(int state);
	void wd7600_tc(offs_t offset, uint8_t data) { m_isabus->eop_w(offset, data); }
	void wd7600_spkr(int state) { m_speaker->level_w(state); }
	void megapc_io(address_map &map) ATTR_COLD;
	void megapc_map(address_map &map) ATTR_COLD;
	void megapcpl_io(address_map &map) ATTR_COLD;
	void megapcpl_map(address_map &map) ATTR_COLD;
};


void at_state::at16_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x09ffff).bankrw("bank10");
	map(0x0e0000, 0x0fffff).rom().region("bios", 0);
	map(0xfe0000, 0xffffff).rom().region("bios", 0);
}

void at_state::at16l_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x09ffff).bankrw("bank10");
	map(0x0e0000, 0x0fffff).rom().region("bios", 0x20000);
	map(0xfe0000, 0xffffff).rom().region("bios", 0x20000);
}

void at_state::at32_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0009ffff).bankrw("bank10");
	map(0x000e0000, 0x000fffff).rom().region("bios", 0);
	map(0x00800000, 0x00800bff).ram().share("nvram");
	map(0xfffe0000, 0xffffffff).rom().region("bios", 0);
}

void at_state::at32l_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0009ffff).bankrw("bank10");
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x20000);
	map(0x00800000, 0x00800bff).ram().share("nvram");
	map(0xfffe0000, 0xffffffff).rom().region("bios", 0x20000);
}

void at_state::ficpio_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0009ffff).bankrw("bank10");
	map(0x00800000, 0x00800bff).ram().share("nvram");
	map(0xfffe0000, 0xffffffff).rom().region("isa", 0x20000);
}

void at_state::at16_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(at_mb_device::map));
}

uint16_t at_state::ps1_unk_r(offs_t offset)
{
	return m_ps1_reg[offset];
}

void at_state::ps1_unk_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if((offset == 0) && (data == 0x60))
		data = 0x68;

	COMBINE_DATA(&m_ps1_reg[offset]);
}

uint8_t at_state::ps1_portb_r()
{
	uint8_t data = m_mb->portb_r();
	/* 0x10 is the dram refresh line bit, 15.085us. */
	data = (data & ~0x10) | ((machine().time().as_ticks(66291) & 1) ? 0x10 : 0);

	return data;
}

void at_state::ps1_16_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(at_mb_device::map));
	map(0x0061, 0x0061).r(FUNC(at_state::ps1_portb_r));
	map(0x0102, 0x0105).rw(FUNC(at_state::ps1_unk_r), FUNC(at_state::ps1_unk_w));
}

void at_state::neat_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(at_mb_device::map));
	map(0x0022, 0x0023).m("cs8221", FUNC(cs8221_device::map));
}

void at_state::at32_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(at_mb_device::map));
}

void at_state::ficpio_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(at_mb_device::map));
	map(0x00a8, 0x00af).rw("chipset", FUNC(vt82c496_device::read), FUNC(vt82c496_device::write));
	map(0x0170, 0x0177).rw("ide2", FUNC(ide_controller_32_device::cs0_r), FUNC(ide_controller_32_device::cs0_w));
	map(0x01f0, 0x01f7).rw("ide", FUNC(ide_controller_32_device::cs0_r), FUNC(ide_controller_32_device::cs0_w));
	map(0x0370, 0x0377).rw("ide2", FUNC(ide_controller_32_device::cs1_r), FUNC(ide_controller_32_device::cs1_w));
	map(0x03f0, 0x03f7).rw("ide", FUNC(ide_controller_32_device::cs1_r), FUNC(ide_controller_32_device::cs1_w));
	map(0x0cf8, 0x0cff).rw("pcibus", FUNC(pci_bus_device::read), FUNC(pci_bus_device::write));
}

void megapc_state::init_megapc()
{
	uint8_t* ROM = memregion("bios")->base();
	ROM[0x19145] = 0x45;  // hack to fix keyboard.  To be removed when the keyboard controller from the MegaPC is dumped
	ROM[0x1fea0] = 0x20;  // to correct checksum
}

void megapc_state::init_megapcpl()
{
	uint8_t* ROM = memregion("bios")->base();
	ROM[0x187b1] = 0x55;  // hack to fix keyboard.  To be removed when the keyboard controller from the MegaPC is dumped
	ROM[0x1fea0] = 0x20;  // to correct checksum
}

void at_vrom_fix_state::init_megapcpla()
{
	uint8_t* ROM = memregion("bios")->base();

	init_at_common(0xa0000);

	ROM[0x33c2a] = 0x45;  // hack to fix keyboard.  To be removed when the keyboard controller from the MegaPC is dumped
	ROM[0x3af37] = 0x45;
	ROM[0x3cf1b] = 0x54;  // this will allow the keyboard to work during the POST memory test
	ROM[0x3fffe] = 0x1c;
	ROM[0x3ffff] = 0x41;  // to correct checksum
}

uint16_t megapc_state::wd7600_ior(offs_t offset)
{
	if (offset < 4)
		return m_isabus->dack_r(offset);
	else
		return m_isabus->dack16_r(offset);
}

void megapc_state::wd7600_iow(offs_t offset, uint16_t data)
{
	if (offset < 4)
		m_isabus->dack_w(offset, data);
	else
		m_isabus->dack16_w(offset, data);
}

void megapc_state::wd7600_hold(int state)
{
	// halt cpu
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	// and acknowledge hold
	m_wd7600->hlda_w(state);
}

void megapc_state::megapc_map(address_map &map)
{
}

void megapc_state::megapcpl_map(address_map &map)
{
}

void megapc_state::megapc_io(address_map &map)
{
	map.unmap_value_high();
}

void megapc_state::megapcpl_io(address_map &map)
{
	map.unmap_value_high();
}

/**********************************************************
 *
 * Init functions
 *
 **********************************************************/

void at_state::init_at_common(int xmsbase)
{
	address_space& space = m_maincpu->space(AS_PROGRAM);

	/* managed RAM */
	membank("bank10")->set_base(m_ram->pointer());

	if (m_ram->size() > xmsbase)
	{
		offs_t ram_limit = 0x100000 + m_ram->size() - xmsbase;
		space.install_ram(0x100000,  ram_limit - 1, m_ram->pointer() + xmsbase);
	}
}

void at_state::init_at()
{
	init_at_common(0xa0000);
}

void at_state::init_atpci()
{
	init_at_common(0x100000);
}

void at_vrom_fix_state::machine_start()
{
	at_state::machine_start();

	address_space& space = m_maincpu->space(AS_PROGRAM);
	space.install_rom(0xc0000, 0xcffff, memregion("bios")->base());
}

void at_state::cfg_single_1200K(device_t *device)
{
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:0")).set_default_option("525hd");
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:1")).set_default_option(nullptr);
}

void at_state::cfg_single_360K(device_t *device)
{
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:0")).set_default_option("525dd");
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:1")).set_default_option(nullptr);
}

void at_state::cfg_single_1440K(device_t *device)
{
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:0")).set_default_option("35hd");
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:1")).set_default_option(nullptr);
}

void at_state::cfg_dual_1440K(device_t *device)
{
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:0")).set_default_option("35hd");
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:1")).set_default_option("35hd");
}

static void pci_devices(device_slot_interface &device)
{
	device.option_add_internal("vt82c505", VT82C505);
}

void at_state::ibm5170(machine_config &config)
{
	/* basic machine hardware */
	i80286_cpu_device &maincpu(I80286(config, m_maincpu, 12_MHz_XTAL / 2));
	maincpu.set_addrmap(AS_PROGRAM, &at_state::at16_map);
	maincpu.set_addrmap(AS_IO, &at_state::at16_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));
	maincpu.shutdown_callback().set("mb", FUNC(at_mb_device::shutdown));

	config.set_maximum_quantum(attotime::from_hz(60));

	AT_MB(config, m_mb).at_softlists(config);
	m_mb->kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	// FIXME: determine ISA bus clock
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "ega", false);
	ISA16_SLOT(config, "isa2", 0, "mb:isabus", pc_isa16_cards, "fdc", false);
	ISA16_SLOT(config, "isa3", 0, "mb:isabus", pc_isa16_cards, "comat", false);
	ISA16_SLOT(config, "isa4", 0, "mb:isabus", pc_isa16_cards, "ide", false);

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_IBM_PC_AT_84));
	pc_kbdc.out_clock_cb().set(m_mb, FUNC(at_mb_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set(m_mb, FUNC(at_mb_device::kbd_data_w));

	/* internal ram */
	RAM(config, m_ram).set_default_size("1664K").set_extra_options("640K,1024K,2M,4M,8M,15M");
}

void at_state::ibm5170a(machine_config &config)
{
	ibm5170(config);
	m_maincpu->set_clock(16_MHz_XTAL / 2);
}

void at_state::ews286(machine_config &config)
{
	ibm5170(config);
	m_maincpu->set_clock(16_MHz_XTAL / 2); // Exact crystal needs to be verified, 8 MHz according to specification

	subdevice<isa16_slot_device>("isa2")->set_option_machine_config("fdc", cfg_single_1200K); // From pictures but also with a 3.5" as second floppy

	SOFTWARE_LIST(config, "ews286_disk_list").set_original("ews286_flop");

	m_ram->set_default_size("640K");
}

void at_state::ec1842(machine_config &config)
{
	ibm5170(config);
	m_maincpu->set_clock(12'000'000);
}

void at_state::ibm5162(machine_config &config)
{
	ibm5170(config);
	m_maincpu->set_clock(6'000'000);
	subdevice<isa16_slot_device>("isa1")->set_default_option("cga");
}

void at_vrom_fix_state::ibmps1(machine_config &config)
{
	ibm5170(config);

	m_maincpu->set_clock(10_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &at_vrom_fix_state::at16l_map);
	m_maincpu->set_addrmap(AS_IO, &at_vrom_fix_state::ps1_16_io);

	subdevice<isa16_slot_device>("isa1")->set_default_option("vga");
	subdevice<isa16_slot_device>("isa1")->set_fixed(true);
	subdevice<pc_kbdc_device>("kbd")->set_default_option(STR_KBD_MICROSOFT_NATURAL);
}

void at_state::atturbo(machine_config &config)
{
	ibm5170(config);
	m_maincpu->set_clock(12'000'000);
	subdevice<isa16_slot_device>("isa1")->set_default_option("svga_et4k");
	subdevice<pc_kbdc_device>("kbd")->set_default_option(STR_KBD_MICROSOFT_NATURAL);
	ISA16_SLOT(config, "isa5", 0, "mb:isabus", pc_isa16_cards, nullptr, false); // FIXME: determine ISA bus clock
}

void at_state::neat(machine_config &config)
{
	atturbo(config);
	m_maincpu->set_addrmap(AS_IO, &at_state::neat_io);

	ds12885_device &rtc(DS12885(config.replace(), "mb:rtc")); // TODO: move this into the cs8221
	rtc.irq().set("mb:pic8259_slave", FUNC(pic8259_device::ir0_w)); // this is in :mb
	rtc.set_century_index(0x32);

	CS8221(config, "cs8221", 0, "maincpu", "mb:isa", "bios");
}

void at_state::xb42639(machine_config &config)
{
	atturbo(config);
	m_maincpu->set_clock(12'500'000);
}

void at_state::k286i(machine_config &config)
{
	ibm5162(config);
	subdevice<pc_kbdc_device>("kbd")->set_default_option(STR_KBD_MICROSOFT_NATURAL);
	ISA16_SLOT(config, "isa5", 0, "mb:isabus", pc_isa16_cards, nullptr, false); // FIXME: determine ISA bus clock
	ISA16_SLOT(config, "isa6", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa7", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa8", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
}

void at_state::at386(machine_config &config)
{
	i386_device &maincpu(I386(config, m_maincpu, 12'000'000));
	maincpu.set_addrmap(AS_PROGRAM, &at_state::at32_map);
	maincpu.set_addrmap(AS_IO, &at_state::at32_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	config.set_maximum_quantum(attotime::from_hz(60));

	AT_MB(config, m_mb).at_softlists(config);
	m_mb->kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// on-board devices
	// FIXME: determine ISA bus clock
	ISA16_SLOT(config, "board1", 0, "mb:isabus", pc_isa16_cards, "fdcsmc", true);
	ISA16_SLOT(config, "board2", 0, "mb:isabus", pc_isa16_cards, "comat", true);
	ISA16_SLOT(config, "board3", 0, "mb:isabus", pc_isa16_cards, "ide", true);
	ISA16_SLOT(config, "board4", 0, "mb:isabus", pc_isa16_cards, "lpt", true);
	// ISA cards
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "svga_et4k", false);
	ISA16_SLOT(config, "isa2", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "mb:isabus", pc_isa16_cards, nullptr, false);

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	pc_kbdc.out_clock_cb().set(m_mb, FUNC(at_mb_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set(m_mb, FUNC(at_mb_device::kbd_data_w));

	/* internal ram */
	RAM(config, m_ram).set_default_size("1664K").set_extra_options("2M,4M,8M,15M,16M,32M,64M");
}

void at_state::at386l(machine_config &config)
{
	at386(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &at_state::at32l_map);
}

void at_state::at486(machine_config &config)
{
	i486_device &maincpu(I486(config, m_maincpu, 25'000'000));
	maincpu.set_addrmap(AS_PROGRAM, &at_state::at32_map);
	maincpu.set_addrmap(AS_IO, &at_state::at32_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	config.set_maximum_quantum(attotime::from_hz(60));

	AT_MB(config, m_mb).at_softlists(config);
	m_mb->kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// on-board devices
	ISA16_SLOT(config, "board1", 0, "mb:isabus", pc_isa16_cards, "fdcsmc", true); // FIXME: deteremine ISA bus clock
	ISA16_SLOT(config, "board2", 0, "mb:isabus", pc_isa16_cards, "comat", true);
	ISA16_SLOT(config, "board3", 0, "mb:isabus", pc_isa16_cards, "ide", true);
	ISA16_SLOT(config, "board4", 0, "mb:isabus", pc_isa16_cards, "lpt", true);
	// ISA cards
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "svga_et4k", false);
	ISA16_SLOT(config, "isa2", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "mb:isabus", pc_isa16_cards, nullptr, false);

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	pc_kbdc.out_clock_cb().set(m_mb, FUNC(at_mb_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set(m_mb, FUNC(at_mb_device::kbd_data_w));

	/* internal ram */
	RAM(config, m_ram).set_default_size("4M").set_extra_options("1M,2M,8M,16M,20M,32M,64M,128M");
}

void at_state::at486l(machine_config &config)
{
	at486(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &at_state::at32l_map);
}

void at_state::at386sx(machine_config &config)
{
	atturbo(config);
	i386sx_device &maincpu(I386SX(config.replace(), m_maincpu, 16'000'000)); /* 386SX */
	maincpu.set_addrmap(AS_PROGRAM, &at_state::at16_map);
	maincpu.set_addrmap(AS_IO, &at_state::at16_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));
}

void at_state::ct386sx(machine_config &config)
{
	at386sx(config);
	m_maincpu->set_addrmap(AS_IO, &at_state::neat_io);
	CS8221(config, "cs8221", 0, "maincpu", "mb:isa", "maincpu");
}

// Commodore PC 30-III
void at_state::pc30iii(machine_config &config)
{
	ibm5170(config);
	m_maincpu->set_clock(6'000'000); // should be 24_MHz_XTAL / 2, but doesn't post with that setting
	subdevice<isa16_slot_device>("isa1")->set_default_option("vga"); // should be ATI EGA Wonder 800+
}

// Commodore PC 40-III
void at_state::pc40iii(machine_config &config)
{
	ibm5170(config);
	m_maincpu->set_clock(6'000'000); // should be 24_MHz_XTAL / 2, but doesn't post with that setting
	subdevice<isa16_slot_device>("isa1")->set_default_option("vga"); // should be onboard Paradise VGA, see ROM declarations
}

void megapc_state::megapc(machine_config &config)
{
	i386sx_device &maincpu(I386SX(config, m_maincpu, 50_MHz_XTAL / 2));
	maincpu.set_addrmap(AS_PROGRAM, &megapc_state::megapc_map);
	maincpu.set_addrmap(AS_IO, &megapc_state::megapc_io);
	maincpu.set_irq_acknowledge_callback("wd7600", FUNC(wd7600_device::intack_cb));

	WD7600(config, m_wd7600, 50_MHz_XTAL / 2);
	m_wd7600->set_cputag(m_maincpu);
	m_wd7600->set_isatag("isa");
	m_wd7600->set_ramtag(m_ram);
	m_wd7600->set_biostag("bios");
	m_wd7600->set_keybctag("keybc");
	m_wd7600->hold_callback().set(FUNC(megapc_state::wd7600_hold));
	m_wd7600->nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_wd7600->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_wd7600->cpureset_callback().set_inputline(m_maincpu, INPUT_LINE_RESET);
	m_wd7600->a20m_callback().set_inputline(m_maincpu, INPUT_LINE_A20);
	// isa dma
	m_wd7600->ior_callback().set(FUNC(megapc_state::wd7600_ior));
	m_wd7600->iow_callback().set(FUNC(megapc_state::wd7600_iow));
	m_wd7600->tc_callback().set(FUNC(megapc_state::wd7600_tc));
	// speaker
	m_wd7600->spkr_callback().set(FUNC(megapc_state::wd7600_spkr));

	// on board devices
	ISA16(config, m_isabus, 0);
	m_isabus->set_memspace(m_maincpu, AS_PROGRAM);
	m_isabus->set_iospace(m_maincpu, AS_IO);
	m_isabus->iochck_callback().set(m_wd7600, FUNC(wd7600_device::iochck_w));
	m_isabus->irq2_callback().set(m_wd7600, FUNC(wd7600_device::irq09_w));
	m_isabus->irq3_callback().set(m_wd7600, FUNC(wd7600_device::irq03_w));
	m_isabus->irq4_callback().set(m_wd7600, FUNC(wd7600_device::irq04_w));
	m_isabus->irq5_callback().set(m_wd7600, FUNC(wd7600_device::irq05_w));
	m_isabus->irq6_callback().set(m_wd7600, FUNC(wd7600_device::irq06_w));
	m_isabus->irq7_callback().set(m_wd7600, FUNC(wd7600_device::irq07_w));
	m_isabus->irq10_callback().set(m_wd7600, FUNC(wd7600_device::irq10_w));
	m_isabus->irq11_callback().set(m_wd7600, FUNC(wd7600_device::irq11_w));
	m_isabus->irq12_callback().set(m_wd7600, FUNC(wd7600_device::irq12_w));
	m_isabus->irq14_callback().set(m_wd7600, FUNC(wd7600_device::irq14_w));
	m_isabus->irq15_callback().set(m_wd7600, FUNC(wd7600_device::irq15_w));
	m_isabus->drq0_callback().set(m_wd7600, FUNC(wd7600_device::dreq0_w));
	m_isabus->drq1_callback().set(m_wd7600, FUNC(wd7600_device::dreq1_w));
	m_isabus->drq2_callback().set(m_wd7600, FUNC(wd7600_device::dreq2_w));
	m_isabus->drq3_callback().set(m_wd7600, FUNC(wd7600_device::dreq3_w));
	m_isabus->drq5_callback().set(m_wd7600, FUNC(wd7600_device::dreq5_w));
	m_isabus->drq6_callback().set(m_wd7600, FUNC(wd7600_device::dreq6_w));
	m_isabus->drq7_callback().set(m_wd7600, FUNC(wd7600_device::dreq7_w));

	ISA16_SLOT(config, "board1", 0, "isabus", pc_isa16_cards, "fdcsmc", true); // FIXME: determine ISA bus clock
	ISA16_SLOT(config, "board2", 0, "isabus", pc_isa16_cards, "comat", true);
	ISA16_SLOT(config, "board3", 0, "isabus", pc_isa16_cards, "ide", true);
	ISA16_SLOT(config, "board4", 0, "isabus", pc_isa16_cards, "lpt", true);
	ISA16_SLOT(config, "board5", 0, "isabus", pc_isa16_cards, "vga", true);
	// ISA cards
	ISA16_SLOT(config, "isa1", 0, "isabus", pc_isa16_cards, nullptr, false);

	at_keyboard_controller_device &keybc(AT_KEYBOARD_CONTROLLER(config, "keybc", 12_MHz_XTAL));
	keybc.hot_res().set("wd7600", FUNC(wd7600_device::kbrst_w));
	keybc.gate_a20().set("wd7600", FUNC(wd7600_device::gatea20_w));
	keybc.kbd_irq().set("wd7600", FUNC(wd7600_device::irq01_w));
	keybc.kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	keybc.kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	pc_kbdc.out_clock_cb().set("keybc", FUNC(at_keyboard_controller_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set("keybc", FUNC(at_keyboard_controller_device::kbd_data_w));

	/* internal ram */
	RAM(config, m_ram).set_default_size("4M").set_extra_options("1M,2M,8M,15M,16M");

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	// video hardware
	PALETTE(config, "palette").set_entries(256); // todo: really needed?

	/* software lists */
	SOFTWARE_LIST(config, "disk_list").set_original("megapc");
}

void megapc_state::megapcpl(machine_config &config)
{
	megapc(config);
	i486_device &maincpu(I486(config.replace(), m_maincpu, 66'000'000 / 2));
	maincpu.set_addrmap(AS_PROGRAM, &megapc_state::megapcpl_map);
	maincpu.set_addrmap(AS_IO, &megapc_state::megapcpl_io);
	maincpu.set_irq_acknowledge_callback("wd7600", FUNC(wd7600_device::intack_cb));
}

void at_vrom_fix_state::megapcpla(machine_config &config)
{
	i486_device &maincpu(I486(config, m_maincpu, 66'000'000 / 2));  // 486SLC
	maincpu.set_addrmap(AS_PROGRAM, &at_vrom_fix_state::at32l_map);
	maincpu.set_addrmap(AS_IO, &at_vrom_fix_state::at32_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	config.set_maximum_quantum(attotime::from_hz(60));

	AT_MB(config, m_mb).at_softlists(config);
	m_mb->kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// on board devices
	ISA16_SLOT(config, "board1", 0, "mb:isabus", pc_isa16_cards, "fdcsmc", true); // FIXME: determine ISA bus clock
	ISA16_SLOT(config, "board2", 0, "mb:isabus", pc_isa16_cards, "comat", true);
	ISA16_SLOT(config, "board3", 0, "mb:isabus", pc_isa16_cards, "ide", true);
	ISA16_SLOT(config, "board4", 0, "mb:isabus", pc_isa16_cards, "lpt", true);
	// ISA cards
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "svga_dm", false);  // closest to the CL-GD5420
	ISA16_SLOT(config, "isa2", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "mb:isabus", pc_isa16_cards, nullptr, false);

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	pc_kbdc.out_clock_cb().set(m_mb, FUNC(at_mb_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set(m_mb, FUNC(at_mb_device::kbd_data_w));

	/* internal ram */
	RAM(config, m_ram).set_default_size("4M").set_extra_options("2M,8M,15M,16M,32M,64M,128M,256M");

	/* software lists */
	SOFTWARE_LIST(config, "disk_list").set_original("megapc");
}

void at_state::ficpio2(machine_config &config)
{
	i486_device &maincpu(I486(config, m_maincpu, 25'000'000));
	maincpu.set_addrmap(AS_PROGRAM, &at_state::ficpio_map);
	maincpu.set_addrmap(AS_IO, &at_state::ficpio_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	config.set_maximum_quantum(attotime::from_hz(60));

	AT_MB(config, m_mb).at_softlists(config);
	m_mb->kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	ds12885_device &rtc(DS12885(config.replace(), "mb:rtc"));
	rtc.irq().set("mb:pic8259_slave", FUNC(pic8259_device::ir0_w)); // this is in :mb
	rtc.set_century_index(0x32);

	RAM(config, m_ram).set_default_size("4M").set_extra_options("1M,2M,8M,16M,32M,64M,128M");

	// on board devices
	ISA16_SLOT(config, "board1", 0, "mb:isabus", pc_isa16_cards, "fdcsmc", true); // FIXME: determine ISA bus clock
	ISA16_SLOT(config, "board2", 0, "mb:isabus", pc_isa16_cards, "comat", true);
	ISA16_SLOT(config, "board3", 0, "mb:isabus", pc_isa16_cards, "lpt", true);

	ide_controller_32_device &ide(IDE_CONTROLLER_32(config, "ide").options(ata_devices, "hdd", nullptr, true));
	ide.irq_handler().set("mb:pic8259_slave", FUNC(pic8259_device::ir6_w));
	ide_controller_32_device &ide2(IDE_CONTROLLER_32(config, "ide2").options(ata_devices, "cdrom", nullptr, true));
	ide2.irq_handler().set("mb:pic8259_slave", FUNC(pic8259_device::ir7_w));

	PCI_BUS(config, "pcibus", 0).set_busnum(0);
	PCI_CONNECTOR(config, "pcibus:0", pci_devices, "vt82c505", true);
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "svga_et4k", false);
	ISA16_SLOT(config, "isa2", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "mb:isabus", pc_isa16_cards, nullptr, false);

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	pc_kbdc.out_clock_cb().set(m_mb, FUNC(at_mb_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set(m_mb, FUNC(at_mb_device::kbd_data_w));

	vt82c496_device &chipset(VT82C496(config, "chipset"));
	chipset.set_cputag(m_maincpu);
	chipset.set_ramtag(m_ram);
	chipset.set_isatag("isa");
}

// Compaq Portable III
void at_state::comportiii(machine_config &config)
{
	/* basic machine hardware */
	i80286_cpu_device &maincpu(I80286(config, m_maincpu, 48_MHz_XTAL / 4 /*12000000*/));
	maincpu.set_addrmap(AS_PROGRAM, &at_state::at16_map);
	maincpu.set_addrmap(AS_IO, &at_state::at16_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));
	maincpu.shutdown_callback().set("mb", FUNC(at_mb_device::shutdown));

	config.set_maximum_quantum(attotime::from_hz(60));

	AT_MB(config, m_mb).at_softlists(config);
	m_mb->kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	// FIXME: determine ISA bus clock
	ISA16_SLOT(config, "board1", 0, "mb:isabus", pc_isa16_cards, "fdc", true).set_option_machine_config("fdc", cfg_single_1200K);
	ISA16_SLOT(config, "board2", 0, "mb:isabus", pc_isa16_cards, "comat", true);
	ISA16_SLOT(config, "board3", 0, "mb:isabus", pc_isa16_cards, "hdc", true);
	ISA16_SLOT(config, "board4", 0, "mb:isabus", pc_isa16_cards, "cga_cportiii", true);
	ISA16_SLOT(config, "isa1",   0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2",   0, "mb:isabus", pc_isa16_cards, nullptr, false);

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_IBM_PC_AT_84));
	pc_kbdc.out_clock_cb().set(m_mb, FUNC(at_mb_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set(m_mb, FUNC(at_mb_device::kbd_data_w));

	/* internal ram */
	RAM(config, m_ram).set_default_size("640K").set_extra_options("1152K,1664K,2176K,2688K,4736K,6784K");
}

void at_state::comportii(machine_config &config)
{
	ibm5170(config);
	m_maincpu->set_clock(48_MHz_XTAL / 6);

	subdevice<isa16_slot_device>("isa2")->set_option_machine_config("fdc", cfg_single_360K);
	subdevice<isa16_slot_device>("isa4")->set_default_option("hdc");
	m_ram->set_default_size("640K").set_extra_options("1152K,1664K,2176K,2688K,4224K");
}

// Nixdorf 8810 M55
void at_state::n8810m15(machine_config &config)
{
	ibm5170(config);
	m_maincpu->set_clock(6'000'000);
	subdevice<isa16_slot_device>("isa1")->set_default_option("cga");
}

// Nixdorf 8810 M55
void at_state::n8810m55(machine_config &config)
{
	ibm5170(config);
	m_maincpu->set_clock(6'000'000);
	subdevice<isa16_slot_device>("isa1")->set_default_option("ega");
}

// AEG Olympia Olyport 40-21
void at_state::olyport40(machine_config &config)
{
	neat(config);
	m_maincpu->set_clock(12'000'000);
	subdevice<isa16_slot_device>("isa1")->set_default_option("cga");
}

// Bull Micral 45
void at_state::micral45(machine_config &config)
{
	atturbo(config);
	m_maincpu->set_clock(12'000'000);
	subdevice<isa16_slot_device>("isa1")->set_default_option("ega");
}

void at_state::euroat(machine_config &config)
{
	ibm5170(config);
	m_maincpu->set_clock(24_MHz_XTAL / 2); // Bus speed can be set up to CPU speed

	subdevice<isa16_slot_device>("isa2")->set_option_machine_config("fdc", cfg_single_1440K); // From pictures but also with a 3.5" as second floppy

	m_ram->set_default_size("640K");
}

// Siemens PG 750
void at_state::pg750(machine_config &config)
{
	i386_device &maincpu(I386(config, m_maincpu, 12'000'000));
	maincpu.set_addrmap(AS_PROGRAM, &at_state::at32_map);
	maincpu.set_addrmap(AS_IO, &at_state::at32_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	config.set_maximum_quantum(attotime::from_hz(60));

	AT_MB(config, m_mb).at_softlists(config);
	m_mb->kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	ds12885_device &rtc(DS12885(config.replace(), "mb:rtc")); // TODO: move this into the cs8221
	rtc.irq().set("mb:pic8259_slave", FUNC(pic8259_device::ir0_w)); // this is in :mb
	rtc.set_century_index(0x32);

	// on-board devices
	ISA16_SLOT(config, "board1", 0, "mb:isabus", pc_isa16_cards, "fdc", true).set_option_machine_config("fdc", cfg_dual_1440K); // FIXME: deteremine ISA bus clock
	ISA16_SLOT(config, "board2", 0, "mb:isabus", pc_isa16_cards, "comat", true);
	ISA16_SLOT(config, "board3", 0, "mb:isabus", pc_isa16_cards, "lpt", true);
	// ISA cards
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "ega", false);
	ISA16_SLOT(config, "isa2", 0, "mb:isabus", pc_isa16_cards, "hdc", false);
	ISA16_SLOT(config, "isa3", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "mb:isabus", pc_isa16_cards, nullptr, false);

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	pc_kbdc.out_clock_cb().set(m_mb, FUNC(at_mb_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set(m_mb, FUNC(at_mb_device::kbd_data_w));

	/* internal ram */
	RAM(config, m_ram).set_default_size("3712K");
}

//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

//**************************************************************************
//  IBM systems
//**************************************************************************
ROM_START( ibm5170 )
	ROM_REGION16_LE(0x20000, "bios", 0) // - IBM 5170, 6 Mhz, one wait state RAM or 8 Mhz, one wait state RAM

	ROM_SYSTEM_BIOS( 0, "rev1", "IBM PC/AT 5170 01/10/84")
	ROMX_LOAD( "6181028.u27", 0x10000, 0x8000, CRC(f6573f2a) SHA1(3e52cfa6a6a62b4e8576f4fe076c858c220e6c1a), ROM_SKIP(1) | ROM_BIOS(0)) /* T 6181028 8506AAA // TMM23256P-5878 // (C)IBM CORP 1981,-1984 */
	ROMX_LOAD( "6181029.u47", 0x10001, 0x8000, CRC(7075fbb2) SHA1(a7b885cfd38710c9bc509da1e3ba9b543a2760be), ROM_SKIP(1) | ROM_BIOS(0)) /* T 6181029 8506AAA // TMM23256P-5879 // (C)IBM CORP 1981,-1984 */

	ROM_SYSTEM_BIOS( 1, "rev2", "IBM PC/AT 5170 06/10/85")  /* Another verification of these crcs would be nice */
	ROMX_LOAD( "6480090.u27", 0x10000, 0x8000, CRC(99703aa9) SHA1(18022e93a0412c8477e58f8c61a87718a0b9ab0e), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "6480091.u47", 0x10001, 0x8000, CRC(013ef44b) SHA1(bfa15d2180a1902cb6d38c6eed3740f5617afd16), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_SYSTEM_BIOS( 2, "landmark", "Landmark/Supersoft diagnostic ROMs") // use Hercules or MDA
	ROMX_LOAD( "5170_even_u27_ 27256.bin", 0x10000, 0x8000, CRC(6790392d) SHA1(c4a5310341f346dd072d096152060ef5e4430a7f), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "5170_odd_u47_ 27256.bin", 0x10001, 0x8000, CRC(4c0f3db4) SHA1(97a0cf589b93551ed1d03bd622cbc8fd5634512f), ROM_SKIP(1) | ROM_BIOS(2))

//  ROM_SYSTEM_BIOS( 3, "atdiag", "IBM PC/AT 5170 w/Super Diagnostics")
//  ROMX_LOAD( "atdiage.bin", 0xf8000, 0x4000, CRC(e8855d0c) SHA1(c9d53e61c08da0a64f43d691bf6cadae5393843a), ROM_SKIP(1) | ROM_BIOS(3))
//  ROMX_LOAD( "atdiago.bin", 0xf8001, 0x4000, CRC(606fa71d) SHA1(165e45bae7ae2da274f1e645c763c5bfcbde027b), ROM_SKIP(1) | ROM_BIOS(3))

	/* Mainboard PALS */
	ROM_REGION( 0x2000, "pals", 0 )
	ROM_LOAD( "1501824_717750.mmipal14l4.u87.jed", 0x0000, 0x02E7, CRC(3c819a27) SHA1(d2f4889e628dbbef50b7f48cb1d1a313232bacc8)) /* MMI 1501824 717750 // (C)1983 IBM(M) */
	ROM_LOAD( "1503135_705075.mmipal14l4.u130.jed", 0x02E7, 0x02E7, CRC(aac77198) SHA1(b318da3a1fbe5402836c1b548e231e0794d0c032)) /* MMI 1503135 705075 // (C) IBM CORP 83 */
	/* P/N 6320947 Serial/Parallel ISA expansion card PAL */
	ROM_LOAD( "1503085.mmipal.u14.jed", 0x1000, 0x0800, NO_DUMP) /* MMI 1503085 8449 // (C) IBM CORP 83 */ /* Not sure of type */

	/* Mainboard PROMS */
	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD( "1501814.82s123an.u115", 0x0000, 0x0020, CRC(849c9217) SHA1(2955ae1705c3b59170f1373f99b3ea5c174c4544)) /* N82S123AN 8713 // SK-D 1501814 */
	ROM_LOAD( "55x8041.82s147an.u72", 0x0020, 0x0200, CRC(f2cc4fe6) SHA1(e285468516bd05083155a8a272583deef655315a)) /* S N82S147AN 8709 // V-C55X8041 */
ROM_END

ROM_START( ibm5170a )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS( 0, "rev3", "IBM PC/AT 5170 11/15/85")
	ROMX_LOAD( "61x9266.u27", 0x10000, 0x8000, CRC(4995be7a) SHA1(8e8e5c863ae3b8c55fd394e345d8cca48b6e575c), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "61x9265.u47", 0x10001, 0x8000, CRC(c32713e4) SHA1(22ed4e2be9f948682891e2fd056a97dbea01203c), ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS( 1, "3270at", "IBM 3270 PC/AT 5281 11/15/85") /* pretty much just a part string and checksum change from the 5170 rev3 */
	ROMX_LOAD( "62x0820.u27", 0x10000, 0x8000, CRC(e9cc3761) SHA1(ff9373c1a1f34a32fb6acdabc189c61b01acf9aa), ROM_SKIP(1) | ROM_BIOS(1)) /* T 62X0820-U27 8714HAK // TMM23256P-6746 // (C)IBM CORP 1981,-1985 */
	ROMX_LOAD( "62x0821.u47", 0x10001, 0x8000, CRC(b5978ccb) SHA1(2a1aeb9ae3cd7e60fc4c383ca026208b82156810), ROM_SKIP(1) | ROM_BIOS(1)) /* T 62X0821-U47 8715HAK // TMM23256P-6747 // (C)IBM CORP 1981,-1985 */

	/* Mainboard PALS */
	ROM_REGION( 0x2000, "pals", 0 )
	ROM_LOAD( "1501824_717750.mmipal14l4.u87.jed", 0x0000, 0x02E7, CRC(3c819a27) SHA1(d2f4889e628dbbef50b7f48cb1d1a313232bacc8)) /* MMI 1501824 717750 // (C)1983 IBM(M) */
	ROM_LOAD( "1503135_705075.mmipal14l4.u130.jed", 0x02E7, 0x02E7, CRC(aac77198) SHA1(b318da3a1fbe5402836c1b548e231e0794d0c032)) /* MMI 1503135 705075 // (C) IBM CORP 83 */    /* P/N 6320947 Serial/Parallel ISA expansion card PAL */
	ROM_LOAD( "1503085.mmipal.u14.jed", 0x1000, 0x0800, NO_DUMP) /* MMI 1503085 8449 // (C) IBM CORP 83 */ /* Not sure of type */

	/* Mainboard PROMS */
	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD( "1501814.82s123an.u115", 0x0000, 0x0020, CRC(849c9217) SHA1(2955ae1705c3b59170f1373f99b3ea5c174c4544)) /* N82S123AN 8713 // SK-D 1501814 */
	ROM_LOAD( "55x8041.82s147an.u72", 0x0020, 0x0200, CRC(f2cc4fe6) SHA1(e285468516bd05083155a8a272583deef655315a)) /* S N82S147AN 8709 // V-C55X8041 */
ROM_END


ROM_START( ibm5162 ) //MB p/n 62x1168 - IBM 5162, 6 Mhz, zero wait state RAM
	ROM_REGION16_LE(0x20000, "bios", 0)

	ROM_LOAD16_BYTE( "78x7460.u34", 0x10000, 0x8000, CRC(1db4bd8f) SHA1(7be669fbb998d8b4626fefa7cd1208d3b2a88c31)) /* 78X7460 U34 // (C) IBM CORP // 1981-1986 */
	ROM_LOAD16_BYTE( "78x7461.u35", 0x10001, 0x8000, CRC(be14b453) SHA1(ec7c10087dbd53f9c6d1174e8f14212e2aec1818)) /* 78X7461 U35 // (C) IBM CORP // 1981-1986 */

	/* Mainboard PALS */
	ROM_REGION( 0x2000, "pals", 0 )
	ROM_LOAD( "59x7599.mmipal20l8.u27.jed", 0x0000, 0x02E7, NO_DUMP) /* MMI PAL20L8ACN5 8631 // N59X7599 IBM (C)85 K3 */
	ROM_LOAD( "1503135.mmipal14l4.u81.jed", 0x02E7, 0x02E7, CRC(aac77198) SHA1(b318da3a1fbe5402836c1b548e231e0794d0c032)) /* MMI 1503135 8625 // (C) IBM CORP 83 */
	/* P/N 6320947 Serial/Parallel ISA expansion card PAL */
	ROM_LOAD( "1503085.mmipal.u14.jed", 0x1000, 0x0800, NO_DUMP) /* MMI 1503085 8449 // (C) IBM CORP 83 */ /* Not sure of type */

	/* Mainboard PROMS */
	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD( "1501814.82s123an.u72", 0x0000, 0x0020, CRC(849c9217) SHA1(2955ae1705c3b59170f1373f99b3ea5c174c4544)) /* N82S123AN 8623 // SK-U 1501814 */
	ROM_LOAD( "59x7594.82s147an.u90", 0x0020, 0x0200, NO_DUMP) /* S N82S147AN 8629 // VCT 59X7594 */
ROM_END


// According to http://nerdlypleasures.blogspot.com/2014/04/the-original-8-bit-ide-interface.html
// the IBM PS/1 Model 2011 use a customised version of the XTA (8-bit IDE) harddisk interface

// https://en.wikipedia.org/wiki/IBM_PS/1
// http://ps-2.kev009.com/pcpartnerinfo/ctstips/937e.htm
// https://ps1stuff.wordpress.com/documentation/ibm-ps1-model-2011/
// https://barotto.github.io/IBMulator/#download

ROM_START( ibm2011 )
	ROM_REGION16_LE( 0x40000, "bios", 0)
	// Spanish version
	ROM_SYSTEM_BIOS( 0, "2011es", "IBM PS/1 2011 ES")
	ROMX_LOAD( "ibm_1057757_24-05-90.bin", 0x00000, 0x20000, CRC(c8f81ea4) SHA1(925ed0e98f9f2997cb86554ef384bcfaf2a4ecbe), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "ibm_1057757_29-15-90.bin", 0x00001, 0x20000, CRC(c2dd6b5c) SHA1(f6b5785002dd628b6b1fb3bb101e076299eba3b6), ROM_SKIP(1) |  ROM_BIOS(0))
	// US version
	ROM_SYSTEM_BIOS( 1, "2011us", "IBM PS/1 2011 US") // constant resets
	ROMX_LOAD( "1057754.bin", 0x00000, 0x20000, CRC(648a6a61) SHA1(6cebaf9f2431e67fea37f34b06916264d6737ab6), ROM_SKIP(1) |  ROM_BIOS(1))
	ROMX_LOAD( "1057756.bin", 0x00001, 0x20000, CRC(862f94ac) SHA1(1eba7fa20301403db7c4f53032267902191ea2c7), ROM_SKIP(1) |  ROM_BIOS(1))
ROM_END

ROM_START( ibm2011rd ) // these international versions were shipped with DOS in a ROM disk and require a different memory map, they don't yet load properly
	ROM_REGION16_LE( 0x80000, "bios", 0)
	// Swedish version
	ROM_SYSTEM_BIOS( 0, "2011se", "IBM PS/1 2011 SE")
	ROMX_LOAD( "ibm2011se_f80000.bin", 0x00000, 0x40000, CRC(1b90693b) SHA1(2cdcfda55fea25a991c1568ff398d97c5e07e96d),  ROM_BIOS(0))
	ROMX_LOAD( "ibm2011se_fc0000.bin", 0x40000, 0x40000, CRC(ef7aa453) SHA1(993dd6e17c6fd5c2ef513d94383f36b1929d1936),  ROM_BIOS(0))
	// Portuguese version
	ROM_SYSTEM_BIOS( 1, "2011pt", "IBM PS/1 2011 PT")
	ROMX_LOAD( "u18_x1_1057451.bin", 0x00000, 0x20000, CRC(0484e15d) SHA1(39fb05843c8371f4b716679e6ce512bcf5a05dac), ROM_SKIP(1) |  ROM_BIOS(1))
	ROMX_LOAD( "u36_x4_1057449.bin", 0x00001, 0x20000, CRC(23d7e4fe) SHA1(9c89efa61fc77485b65fff9133d6a19caca553e9), ROM_SKIP(1) |  ROM_BIOS(1))
	ROMX_LOAD( "u23_x2_1057757.bin", 0x40000, 0x20000, CRC(c8f81ea4) SHA1(925ed0e98f9f2997cb86554ef384bcfaf2a4ecbe), ROM_SKIP(1) |  ROM_BIOS(1))
	ROMX_LOAD( "u28_x3_1057759.bin", 0x40001, 0x20000, CRC(c2dd6b5c) SHA1(f6b5785002dd628b6b1fb3bb101e076299eba3b6), ROM_SKIP(1) |  ROM_BIOS(1))
	// German version
	ROM_SYSTEM_BIOS( 2, "2011de", "IBM PS/1 2011 DE")
	ROMX_LOAD( "x1_1057866_u10.bin", 0x00000, 0x20000, CRC(ef0f0bb4) SHA1(d1e4c081f1a74732eb6e37a3bfb9403819b7d891), ROM_SKIP(1) |  ROM_BIOS(2))
	ROMX_LOAD( "x4_1057864_u36.bin", 0x00001, 0x20000, CRC(16d357ff) SHA1(6521b160bf0dd05b890ad197d9c9359d806da18a), ROM_SKIP(1) |  ROM_BIOS(2))
	ROMX_LOAD( "x2_1057757_u23.bin", 0x40000, 0x20000, CRC(c8f81ea4) SHA1(925ed0e98f9f2997cb86554ef384bcfaf2a4ecbe), ROM_SKIP(1) |  ROM_BIOS(2))
	ROMX_LOAD( "x3_1057759_u28.bin", 0x40001, 0x20000, CRC(c2dd6b5c) SHA1(f6b5785002dd628b6b1fb3bb101e076299eba3b6), ROM_SKIP(1) |  ROM_BIOS(2))
	// Italian version
	ROM_SYSTEM_BIOS( 3, "2011it", "IBM_PS/1 2011 IT")
	ROMX_LOAD( "x1-1057630-u18.bin", 0x00000, 0x20000, CRC(3843830c) SHA1(68b2f443b6ceadbc94a725fe66ad9c9685490dcb), ROM_SKIP(1) |  ROM_BIOS(3))
	ROMX_LOAD( "x4-1057628-u36.bin", 0x00001, 0x20000, CRC(1ddf3afb) SHA1(da55abaf4f775e2e3efdd952beb9f97769e3cac3), ROM_SKIP(1) |  ROM_BIOS(3))
	ROMX_LOAD( "x2_1057757_u23.bin", 0x40000, 0x20000, CRC(c8f81ea4) SHA1(925ed0e98f9f2997cb86554ef384bcfaf2a4ecbe), ROM_SKIP(1) |  ROM_BIOS(3))
	ROMX_LOAD( "x3_1057759_u28.bin", 0x40001, 0x20000, CRC(c2dd6b5c) SHA1(f6b5785002dd628b6b1fb3bb101e076299eba3b6), ROM_SKIP(1) |  ROM_BIOS(3))
	// UK version
	ROM_SYSTEM_BIOS( 4, "2011uk", "IBM_PS/1 2011 UK")
	ROMX_LOAD( "u18_x1.bin", 0x00000, 0x20000, CRC(029c4d8a) SHA1(bf2f56ac2e03098144b3dcc34f7daa09c8e08288), ROM_SKIP(1) |  ROM_BIOS(4))
	ROMX_LOAD( "u36_x4.bin", 0x00001, 0x20000, CRC(bf6c5631) SHA1(68cbff7e229cd77ae8c2e8835dbb9b3047f41e4c), ROM_SKIP(1) |  ROM_BIOS(4))
	ROMX_LOAD( "u23_x2.bin", 0x40000, 0x20000, CRC(c8f81ea4) SHA1(925ed0e98f9f2997cb86554ef384bcfaf2a4ecbe), ROM_SKIP(1) |  ROM_BIOS(4))
	ROMX_LOAD( "u28_x3.bin", 0x40001, 0x20000, CRC(c2dd6b5c) SHA1(f6b5785002dd628b6b1fb3bb101e076299eba3b6), ROM_SKIP(1) |  ROM_BIOS(4))
ROM_END

// From Wikipedia:
// Model     MB FRU    CPU                   ISA Sl.  RAM   VRAM   Hard-Drive         Serial/Modem
// 2121-C42  92F9690   Intel 80386SX @ 16 MHz  0      2 MB  256KB  95F4720  40MB IDE  2400 baud modem
// 2121-B82  92F9690   Intel 80386SX @ 16 MHz  2      2 MB  256KB  92F9943  80MB IDE  2400 baud modem
// 2121-C92            Intel 80386SX @ 16 MHz  0      2 MB  256KB          129MB IDE  2400 baud modem
// 2121-G42            Intel 80386SX @ 20 MHz  0      2 MB  256KB           40MB IDE  2400 baud modem
// 2121-A82            Intel 80386SX @ 20 MHz  2      2 MB  256KB           40MB IDE  2400 baud modem
// 2121-S92            Intel 80386SX @ 20 MHz  0      2 MB  256KB          129MB IDE  2400 baud modem
// 2121-M82            Intel 80386SX @ 20 MHz  2      2 MB  256KB           80MB IDE  2400 baud modem
// 2121-A62                                           2     256KB  56F8863 160MB IDE  2400 baud modem
// 2121-A92                                                 256KB                     serial port
// 2121-A94            Intel 80386SX @ 20 MHz  2    6 MB    256KB          129MB IDE  2400 baud modem

ROM_START( ibm2121 )
	ROM_REGION16_LE( 0x40000, "bios", 0)
	ROM_SYSTEM_BIOS( 0, "2121", "IBM PS/1 2121" )
	ROMX_LOAD( "fc0000.bin", 0x00000, 0x40000, CRC(96bbaf52) SHA1(8737d805444837023a58702279f8fe6e7f08e7ba), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "2121us", "IBM PS/1 2121 US" )
	ROMX_LOAD( "ibm2121us_fc0000.bin", 0x00000, 0x40000, CRC(817aad71) SHA1(43b7b84390fcc081a946cdb4bdce4ba7a4a88074), ROM_BIOS(1))
ROM_END

ROM_START( ibm2121rd ) // international versions shipped with ROM DOS, need a different memory map at least
	ROM_REGION16_LE( 0x80000, "bios", 0)
	ROM_SYSTEM_BIOS( 0, "2121sp", "IBM PS/1 2121 Spanish" )
	ROMX_LOAD( "ibm2121sp_f80000.bin", 0x00000, 0x40000, CRC(90505c4b) SHA1(59becaec25644820a78464d66e472a8a225d94cc), ROM_BIOS(0))
	ROMX_LOAD( "ibm2121sp_fc0000.bin", 0x40000, 0x40000, CRC(f83fac75) SHA1(a42b1b9465983392eaa0159d4bfc30620a7af499), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "2121fr", "IBM PS/1 2121 French" )
	ROMX_LOAD( "ibm2121fr_f80000.bin", 0x00000, 0x40000, CRC(9c6de65d) SHA1(6b219c9480a06bc9218e8212acc7cfd1ceaccd4b), ROM_BIOS(1))
	ROMX_LOAD( "ibm2121fr_fc0000.bin", 0x40000, 0x40000, CRC(f83fac75) SHA1(a42b1b9465983392eaa0159d4bfc30620a7af499), ROM_BIOS(1))
ROM_END

// http://ps-2.kev009.com/pcpartnerinfo/ctstips/937e.htm
ROM_START( ibm2123 )
	ROM_REGION16_LE( 0x20000, "bios", 0)
	ROM_LOAD( "ps1_2123_87f4794_rom.bin", 0x00000, 0x20000, CRC(64f921b8) SHA1(e1856bf3dd3ce21f44078aeca1f58c491b202ad2))
ROM_END

// From Wikipedia:
// 2133 Desktop case. The 3x3 references the available slots and drive bays.
// 2155 Desktop case larger than 2133. The 5x5 references the available slots and drive bays. Including a 5.25" bay.
// 2168 Tower unit. The 6x8 references the available slots and bays. Including 5.25" bays.
// Model     MB FRU      CPU                        RAM   SIMM          Video chip   VRAM   Hard-Drive          Notes
// 2133-711  93F2397  Intel 80386SX @ 25 MHz    2 MB  2×72 Pin FPM                   256KB  59G9567  85MB IDE
// 2133-811          Intel 80386SX @ 25 MHz     4 MB                                                 85MB IDE
// 2133-13   ???      Intel 80386SX @ 25 MHz    2 MB  2x72 Pin FPM                   256KB
// 2133-W13          Intel 80386SX @ 25 MHz     2 MB                                                129MB IDE
// 2133-13T  65G3766  Intel 80486SX @ 25 MHz    4 MB  2×72 Pin FPM                   256KB  93F2329 129MB IDE
// 2133-?43  34G1885  Intel 80486SX @ 20 MHz    4 MB  2×30 Pin FPM                   512KB  93F2329 129MB IDE
// 2133-?50  34G1848  Intel 80486SX @ 25 MHz    4 MB  2×30 Pin FPM                   512KB  93F2329 129MB IDE
// 2133-?53  34G1848  Intel 80486SX @ 25 MHz    4 MB  2×30 Pin FPM                   512KB  93F2329 129MB IDE
// 2133-652          Intel 80486SX @ 33 MHz     4 MB  4×72 Pin FPM  Cirrus CL-GD5424 512KB  84G3927 171MB IDE
// 2133-575          Intel 80486DX @ 33 MHz     4 MB  4×72 Pin FPM                   512KB          170MB IDE
// 2133-594          Intel 80486DX2 @66 MHz     4 MB  4×72 Pin FPM                   512KB          253MB IDE
// 2133-E11          Intel 80386SX @ 25 MHz     2 MB  2×72 Pin FPM  Cirrus CL-GD5424 512 KB          85MB IDE   Canada models, English model
// 2133-F11          Intel 80386SX @ 25 MHz     2 MB  2×72 Pin FPM  Cirrus CL-GD5424 512 KB          85MB IDE   Canada models, French model
// 2133-E43          Intel 80486SX @ 20 MHz     2 MB  8×30 Pin FPM  Tseng ET4000     512KB          129MB IDE   Canada models, English model
// 2133-F43          Intel 80486SX @ 20 MHz     2 MB  8×30 Pin FPM  Tseng ET4000     512KB          129MB IDE   Canada models, French model
// 2133-E53          Intel 80486SX @ 25 MHz     2 MB  8×30 Pin FPM  Tseng ET4000     512KB          129MB IDE   Canada models, English model
// 2133-F53          Intel 80486SX @ 25 MHz     2 MB  8×30 Pin FPM  Tseng ET4000     512KB          129MB IDE   Canada models, French model

ROM_START( ibm2133 )
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	ROM_LOAD( "ps1_2133_52g2974_rom.bin", 0x00000, 0x20000, CRC(89fc7600) SHA1(758e161353f6781c39ac67f1ba293c14038b17dc))
ROM_END


//**************************************************************************
//  Apricot systems
//
// http://bbs.actapricot.org/files/ , http://insight.actapricot.org/insight/products/main.htm
//
//**************************************************************************

// Apricot XEN-S (Venus I Motherboard 286)
// BIOS-String: apricot / Phoenix ROM BIOS PLUS Version 3.10.17 / Apricot XEN-S 25th June 1992
// Gate A20 failure - MAME message: char SEL checker, contact MAMEdev
ROM_START( xb42639 )
	/* actual VGA BIOS not dumped*/
	ROM_REGION16_LE(0x20000, "bios", 0)
	// XEN-S (Venus I Motherboard)
	ROM_LOAD16_BYTE( "3-10-17i.lo", 0x10000, 0x8000, CRC(3786ca1e) SHA1(c682d7c76f234559d03bcf21010c13c4dbeafb69))
	ROM_LOAD16_BYTE( "3-10-17i.hi", 0x10001, 0x8000, CRC(d66710eb) SHA1(e8c1cd5f9ecfbd8825655e416d7ddf2ae362e69b))
ROM_END

// Apricot XEN-S (Venus II Motherboard 286)
// BIOS-String: apricot XEN-S Series Personal Computer / Phoenix ROM BIOS PLUS Version 3.10.04 / XEN-S II BIOS VR 1.2.17 16th October 1990
// Gate A20 failure
ROM_START( xb42639a )
	/* actual VGA BIOS not dumped*/
	ROM_REGION16_LE(0x20000, "bios", 0)
	// XEN-S (Venus II Motherboard)
	ROM_LOAD16_BYTE( "10217.lo", 0x10000, 0x8000, CRC(ea53406f) SHA1(2958dfdbda14de4e6b9d6a8c3781131ab1e32bef))
	ROM_LOAD16_BYTE( "10217.hi", 0x10001, 0x8000, CRC(111725cf) SHA1(f6018a45bda4476d40c5881fb0a506ff75ec1688))
ROM_END

// Apricot XEN-S (Venus I Motherboard 386)
// BIOS-String: apricot / Phoenix ROM BIOS PLUS Version 3.10.17 / Apricot XEN-S 25th June 1992
// MAME message: char SEL checker, contact MAMEdev
ROM_START( xb42664 )
	/* actual VGA BIOS not dumped */
	ROM_REGION32_LE(0x20000, "bios", 0)
	// XEN-S (Venus I Motherboard)
	ROM_LOAD16_BYTE( "3-10-17i.lo", 0x10000, 0x8000, CRC(3786ca1e) SHA1(c682d7c76f234559d03bcf21010c13c4dbeafb69))
	ROM_LOAD16_BYTE( "3-10-17i.hi", 0x10001, 0x8000, CRC(d66710eb) SHA1(e8c1cd5f9ecfbd8825655e416d7ddf2ae362e69b))
ROM_END

// Apricot XEN-S (Venus II Motherboard 386)
// BIOS-String: apricot XEN-S Series Personal Computer / Phoenix ROM BIOS PLUS VERSION 3.10.04 / XEN-S II BIOS VR 1.2.17 16th October 1990
ROM_START( xb42664a )
	/* actual VGA BIOS not dumped*/
	ROM_REGION32_LE(0x20000, "bios", 0)
	// XEN-S (Venus II Motherboard)
	ROM_LOAD16_BYTE( "10217.lo", 0x10000, 0x8000, CRC(ea53406f) SHA1(2958dfdbda14de4e6b9d6a8c3781131ab1e32bef))
	ROM_LOAD16_BYTE( "10217.hi", 0x10001, 0x8000, CRC(111725cf) SHA1(f6018a45bda4476d40c5881fb0a506ff75ec1688))
ROM_END

// Apricot Qi 300 (Rev D,E & F Motherboard) - no display
ROM_START( xb42663 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "qi310223.lo", 0x00000, 0x10000, CRC(53047f49) SHA1(7b38e533f7f27295269549c63e5477d950239167))
	ROM_LOAD16_BYTE( "qi310223.hi", 0x00001, 0x10000, CRC(4852869f) SHA1(98599d4691d40b3fac2936034c70b386ce4caf77))
ROM_END

// Apricot Qi 600 (Neptune Motherboard) - no display, beep code L-1-1-3 (Extended CMOS RAM failure)
ROM_START( qi600 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "qi610223.lo", 0x00000, 0x10000, CRC(563114a9) SHA1(62932b3bf0b5502ff708f604c21773f00afda58e))
	ROM_LOAD16_BYTE( "qi610223.hi", 0x00001, 0x10000, CRC(0ae133f6) SHA1(6039c366f7fe0ebf60b34c1a7d6b2d781b664001))
ROM_END

// Apricot Qi 900 (Scorpion Motherboard)  - no display, beep code L-1-1-3 (Extended CMOS RAM failure)
ROM_START( qi900 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "qi910224.lo", 0x00000, 0x10000, CRC(b012ad3c) SHA1(807e788a6bd03f5e983fe503af3d0b202c754b8a))
	ROM_LOAD16_BYTE( "qi910224.hi", 0x00001, 0x10000, CRC(36e66d56) SHA1(0900c5272ec3ced550f18fb08db59ab7f67a621e))
ROM_END

// Apricot FTs (Scorpion) - no display, beep code L-1-1-3 (Extended CMOS RAM failure)
ROM_START( ftsserv )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "fts10226.lo", 0x00000, 0x10000, CRC(efbd738f) SHA1(d5258760bafdaf1bf13c4a49da76d4b5e7b4ccbd))
	ROM_LOAD16_BYTE( "fts10226.hi", 0x00001, 0x10000, CRC(2460853f) SHA1(a6bba8d2f800140afd129c4d5278f7ae8fe7e63a))
	/* FT Server series Front Panel */
	ROM_REGION(0x10000,"front", 0)
	ROM_LOAD( "fp10009.bin",     0x0000, 0x8000, CRC(8aa7f718) SHA1(9ee6c6a5bb92622ea8d3805196d42ff68887d820))
ROM_END

// Apricot XEN-LS (Venus IV Motherboard) - no display
ROM_START( apxenls3 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "31020.lo", 0x10000, 0x8000, CRC(a19678d2) SHA1(d13c12fa7e94333555eabf58b81bad421e21cd91))
	ROM_LOAD16_BYTE( "31020.hi", 0x10001, 0x8000, CRC(4922e020) SHA1(64e6448323dad2209e004cd93fa181582e768ed5))
ROM_END

// Apricot LANstation (Krypton Motherboard) - no display
ROM_START( aplanst )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "31024", "BIOS 3-10-24")
	ROMX_LOAD( "31024.lo", 0x10000, 0x8000, CRC(e52b59e1) SHA1(cfcaa4d8d658df8df463108ef30695bd4ee7a617), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "31024.hi", 0x10001, 0x8000, CRC(7286aefa) SHA1(dfc0e3f4936780fa62ae9ec392ce17aa65e717cd), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "31025", "BIOS 3-10-25")
	ROMX_LOAD( "31025.lo", 0x10000, 0x8000, CRC(1aec09bc) SHA1(51d56c97c7c1674554aa89b68945329ea967a8bc), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "31025.hi", 0x10001, 0x8000, CRC(0763caa5) SHA1(48510a933dcd6efea3b14d04444f584c3e6fefeb), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "31026", "BIOS 3-10-26i")
	ROMX_LOAD( "31026i.lo", 0x10000, 0x8000, CRC(670b6ab4) SHA1(8d61a0edf187f99b67eb58f5e11276deee801d17), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "31026i.hi", 0x10001, 0x8000, CRC(ef01c54f) SHA1(911f95d65ab96878e5e7ebccfc4b329db47a1351), ROM_SKIP(1) | ROM_BIOS(2))
ROM_END

// Apricot LANstation (Novell Remote Boot) - no display
ROM_START( aplannb )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "lsl31025.lo", 0x00000, 0x10000, CRC(8bb7229b) SHA1(31449d12884ec4e7752e6c1ce7ce9e0d044eadf2))
	ROM_LOAD16_BYTE( "lsh31025.hi", 0x00001, 0x10000, CRC(09e5c1b9) SHA1(d42be83b4181d3733268c29df04a4d2918370f4e))
ROM_END

// Apricot VX FT server - no display
ROM_START( apvxft )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "ft10221.lo", 0x00000, 0x10000, CRC(8f339de0) SHA1(a6542406746eaf1ff7f9e3678c5cbe5522fb314a))
	ROM_LOAD16_BYTE( "ft10221.hi", 0x00001, 0x10000, CRC(3b16bc31) SHA1(0592d1d81e7fd4715b0612083482db122d78c7f2))
ROM_END

// Apricot LS Pro (Caracal Motherboard,Chipset: VLSI VL82C483, ROM: 256KB Flash ROM, PCMCIA Type 2/3 slots)
ROM_START( aplscar )
	ROM_REGION32_LE(0x40000, "bios", 0)
	// 0: MAME exits with "Fatal error: i386: Called modrm_to_EA with modrm value C8!"
	ROM_SYSTEM_BIOS(0, "car306", "Caracal 3.06")
	ROMX_LOAD( "car306.bin",   0x00000, 0x40000, CRC(fc271dea) SHA1(6207cfd312c9957243b8157c90a952404e43b237), ROM_BIOS(0))
	// 1: no display
	ROM_SYSTEM_BIOS(1, "car307", "Caracal 3.07")
	ROMX_LOAD( "car307.bin",   0x00000, 0x40000, CRC(66a01852) SHA1(b0a68c9d67921d27ba483a1c50463406c08d3085), ROM_BIOS(1))
ROM_END

// Apricot XEN PC (A1 Motherboard) - no display
ROM_START( apxena1 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "a1-r26.bin",   0x00000, 0x20000, CRC(d29e983e) SHA1(5977df7f8d7ac2a154aa043bb6f539d96d51fcad))
ROM_END

// Apricot XEN PC (P2 Motherboard, Chipset: M1429G/31, ROM: 128KB Flash ROM, on board: graphics Cirrus Logic GD5434 (via VL))
ROM_START( apxenp2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: ACR97E00-M00-951005-R02-G2 / BIOS V2.0 - Keyboard Interface Error - Pointing DeviceInterface Error
	// after a while the boot continues to the message "Password Violated, System Halted !"
	ROM_SYSTEM_BIOS(0, "p2r02g2", "p2r02g2")
	ROMX_LOAD( "p2r02g2.bin",   0x00000, 0x20000, CRC(311bcc5a) SHA1(be6fa144322077dcf66b065e7f4e61aab8c278b4), ROM_BIOS(0))
	// 1: BIOS-String: ACR97E00-M00-951005-R01-F0 / BIOS V2.0 (error messages as above)
	ROM_SYSTEM_BIOS(1, "lep121s", "SCSI-Enabling ROMs")
	ROMX_LOAD("p2r01f0.bin",   0x00000, 0x20000, CRC(bbc68f2e) SHA1(6954a52a7dda5521794151aff7a04225e9c7df77), ROM_BIOS(1))
ROM_END

// Apricot XEN-i 386 (Leopard Motherboard)
ROM_START( apxeni )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: Phoenix 80386 ROM BIOS PLUS Version 1.10.01 / XEN-i 386 Business Microcomputer / VR 1.2.1 22nd July 1988
	ROM_SYSTEM_BIOS(0, "lep121", "ROM BIOS 1.2.1")
	ROMX_LOAD( "lep121.bin", 0x18000, 0x8000, CRC(948c1927) SHA1(d06bdbd6292db73c815ad1060daf055293dfddf5), ROM_BIOS(0))
	// 1: Phoenix 80386 ROM BIOS PLUS Version 1.10.01 / XEN-i 386 Business Microcomputer / VR 1.2.1 22nd January 1991
	ROM_SYSTEM_BIOS(1, "lep121s", "SCSI-Enabling ROMs")
	ROMX_LOAD( "lep121s.bin", 0x18000, 0x8000, CRC(296118e4) SHA1(d1feaa9704e6ce3bc10c900bdd310d9494b02304), ROM_BIOS(1))
ROM_END

// Apricot LS Pro (Bonsai Motherboard, on board: ethernet (Intel 82596), Chipset: VLSI SCAMP VL82C311 / VL82C333, ROM: 128KB)
ROM_START( aplsbon )
	ROM_REGION32_LE(0x20000 ,"bios", 0)
	// 0: BIOS-String: Phoenix BIOS A486 Version 1.01 / LS Pro BIOS Version 1.06, 4th July 1994 - Pointer device failure
	ROM_SYSTEM_BIOS(0, "bon106", "Bonsai 1-06")
	ROMX_LOAD( "bon106.bin",   0x00000, 0x20000, CRC(98a4eb76) SHA1(e0587afa78aeb9a8803f9b9f9e457e9847b0a2b2), ROM_BIOS(0))
	// 1: flashing screen
	ROM_SYSTEM_BIOS(1, "bon203", "Bonsai 2-03")
	ROMX_LOAD( "bon203.bin",   0x00000, 0x20000, CRC(32a0e125) SHA1(a4fcbd76952599993fa8b76aa36a96386648abb2), ROM_BIOS(1))
	// 2: BIOS-String: Phoenix BIOS A486 Version 1.01 / LS Pro BIOS Version 1.07.03, 2nd February 1995
	ROM_SYSTEM_BIOS(2, "bon10703", "Bonsai 1-07-03")
	ROMX_LOAD( "bon10703.bin",   0x00000, 0x20000, CRC(0275b3c2) SHA1(55ef4cbb7f3166f678aaa478234a42049deaba5f), ROM_BIOS(2))
	// 3: flashing screen
	ROM_SYSTEM_BIOS(3, "bon20402", "Bonsai 2.03")
	ROMX_LOAD( "bon20402.bin",   0x00000, 0x20000, CRC(ac5803fb) SHA1(b8fe92711c6a38a5d9e6497e76a0929c1685c631), ROM_BIOS(3))
ROM_END

// Apricot XEN-LS II (Samurai Motherboard, on board: CD-ROM, graphics, ethernet (Intel 82596), Chipset: VLSI 82C425, VLSI 82C486)
ROM_START( apxlsam ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "sam107", "ROM BIOS Version 1-07")
	ROMX_LOAD( "sam1-07.bin",   0x00000, 0x20000, CRC(65e05a8e) SHA1(c3cd198a129122cb05a28798e54331b06cfdd310), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "sam206", "ROM BIOS Version 2-06")
	ROMX_LOAD( "sam2-06.bin",   0x00000, 0x20000, CRC(9768bb0f) SHA1(8166b77b133072f72f23debf85984eb19578ffc1), ROM_BIOS(1))
ROM_END

// Apricot FTs (Panther Rev F 1.02.26)
ROM_START( aprpand ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "pf10226.std", 0x00000, 0x10000, CRC(7396fb87) SHA1(a109cbad2179eec55f86c0297a59bb015461da21))
	ROM_CONTINUE( 0x00001, 0x10000 )
ROM_END

// Apricot FT//ex 486 (J3 Motherboard, Chipset: Opti 82C696)
ROM_START( aprfte ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "1-2r2-4.486", 0x00000, 0x20000, CRC(bccc236d) SHA1(0765299363e68cf65710a688c360a087856ece8f))
ROM_END


//**************************************************************************
//  Amstrad systems
//**************************************************************************

// Amstrad MegaPC
ROM_START( megapc )
	ROM_REGION(0x40000, "isa", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "41651-bios lo.u18",  0x00000, 0x10000, CRC(1e9bd3b7) SHA1(14fd39ec12df7fae99ccdb0484ee097d93bf8d95))
	ROM_LOAD16_BYTE( "211253-bios hi.u19", 0x00001, 0x10000, CRC(6acb573f) SHA1(376d483db2bd1c775d46424e1176b24779591525))
ROM_END

// Amstrad MegaPC Plus
ROM_START( megapcpl )
	ROM_REGION(0x40000, "isa", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "41652.u18",  0x00000, 0x10000, CRC(6f5b9a1c) SHA1(cae981a35a01234fcec99a96cb38075d7bf23474))
	ROM_LOAD16_BYTE( "486slc.u19", 0x00001, 0x10000, CRC(6fb7e3e9) SHA1(c439cb5a0d83176ceb2a3555e295dc1f84d85103))
ROM_END

// Amstrad MegaPC Plus (Winbond chipset)
ROM_START( megapcpla )
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_LOAD( "megapc_bios.bin",  0x00000, 0x10000, CRC(b84938a2) SHA1(cecab72a96993db4f7c648c229b4211a8c53a380))
	ROM_CONTINUE(0x30000, 0x10000)
ROM_END

// Amstrad PC2386
ROM_START( pc2386 )
	ROM_REGION32_LE( 0x40000, "bios", 0 )
	ROM_LOAD( "c000.bin", 0x00000, 0x4000, CRC(33145bbf) SHA1(c49eaec19f656482e12c8bf282cd4ee5986d227d) )
	ROM_LOAD( "f000.bin", 0x30000, 0x10000, CRC(f54a063c) SHA1(ce70ec493053afab662f51199ef9c9304a209b8e) )
	ROM_FILL(0x3fff1, 1, 0x5b) // f000:e05b is the standard at reset vector jump address
	ROM_FILL(0x3fff2, 1, 0xe0) // why does this rom's point to nowhere sane?
	ROM_FILL(0x3fff3, 1, 0x00) // and why does the rest of the rom look okay?
	ROM_FILL(0x3fff4, 1, 0xf0)

	ROM_REGION( 0x1000, "keyboard", 0 ) // PC2286 / PC2386 102-key keyboard
	ROM_LOAD( "40211.ic801", 0x000, 0x1000, CRC(4440d981) SHA1(a76006a929f26c178e09908c66f28abc92e7744c) )
ROM_END


//**************************************************************************
//  Commodore systems
//**************************************************************************

// Commodore Laptop C286-LT - screen remains blank - CPU: AMD N80C286-12 - Chipset: OAK OTI054 (J9105), OTI055 (FOY 107), OTI053 (J9105), OTI051(J9107)
ROM_START( c286lt )
	ROM_REGION16_LE(0x20000, "bios", 0) // BIOS contains Cirrus Logic VGA firmware, rebadged Sanyo MBC-17NB
	ROM_SYSTEM_BIOS(0, "c286lt13", "C286-LT V1.3")
	ROMX_LOAD( "cbm-c286lt-bios-v1.3-390854-01-1200.bin", 0x00000, 0x20000, CRC(785e87d2) SHA1(e271500169955473d44102a60f051b5f6cfae589), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "c286v17-854", "C286-LT V1.7 390854")
	ROMX_LOAD( "cbm-c286lt-bios-v1.7-390854-04.bin", 0x00000, 0x20000, CRC(2f762ab1) SHA1(d6cb37f0dcb261df86c01d4e1eabe10a52b2070f), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "c286v17-940", "C286-LT V1.7 390940")
	ROMX_LOAD( "cbm-c286lt-bios-v1.7-390940-04.bin", 0x00000, 0x20000, CRC(22d45839) SHA1(bc7159440c52c1f69957da8fdfa76ac0a42ebd16), ROM_BIOS(2))
ROM_END

// Commodore SL 286-16 - this is the wider one, the "slimline model" has only two ISA slots on a riser, an online OTI VGA and a Headland chipset
ROM_START( csl286 ) // continuous short beeps after POST - Chipset is marked "Chips", one IC is P82C212B-12 (16MHz) - system has a WDC VGA card
	ROM_REGION16_LE(0x20000, "bios", 0) // one ISA slot with a riser providing five slots - is or is similar to a DTK PTM 1661c
	ROM_LOAD16_BYTE( "cbm-sl286-16-bios-lo-v1.02-390958-03.bin", 0x10000, 0x8000, CRC(7d0c9472) SHA1(1d614f6835a388f67ece73f40d8a9f65cca3e855))
	ROM_LOAD16_BYTE( "cbm-sl286-16-bios-hi-v1.02-390959-03.bin", 0x10001, 0x8000, CRC(b6d81ddd) SHA1(9478bb846bd1e0dc1904f21d43c6df01ecbc9c83))
ROM_END

// Commodore SL 386SX
ROM_START( c386sx16 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// actual VGA BIOS not dumped - uses a WD Paradise according to http://www.cbmhardware.de/pc/pc.php
	// complains "Time-of-day clock stopped"
	// photos of the system show one ISA16 slot for a riser card, an Acumos AVGA2 chip, a VLSI 82C311 IC, one other VLSI and an Acer chip.
	ROM_SYSTEM_BIOS(0, "c386sxv100", "SL 386SX V1.00") // Commodore 80386SX BIOS Rev. 1.00 - 390914-01/390915-01 - continuous beeps after POST
	ROMX_LOAD( "cbm-sl386sx-bios-lo-v1.0-390914-01.bin", 0x10000, 0x8000, CRC(03e00583) SHA1(8be8478cabd9de3d547a08207ffdcd39bf1bcd94), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "cbm-sl386sx-bios-hi-v1.0-390915-01.bin", 0x10001, 0x8000, CRC(cbe31594) SHA1(d6ace0b5ae4a0f63d047c2918210188f4c77c0c0), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "c386sxv101", "SL 386SX V1.01") // Rev. 1.01 - 390914-02/390915-02 - continuous beeps after POST
	ROMX_LOAD( "cbm-sl386sx-bios-lo-v1.01-390914-02-2700.bin", 0x10000, 0x8000, CRC(711f1523) SHA1(5318127cd42e60dabd221ae8dd16812726a0e889), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "cbm-sl386sx-bios-hi-v1.01-390915-02-3b00.bin", 0x10001, 0x8000, CRC(a1390cbc) SHA1(12aef4b95581e8c4489036c75697f18e9f3727b5), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "c386sxv102", "SL 386SX V1.02") // Rev. 1.02 - 390914-03/390914-03/390915-03
	ROMX_LOAD( "cbm-sl386sx-bios-lo-v1.02-390914-03-0300.bin", 0x10000, 0x8000, CRC(301eb832) SHA1(6c599792b254b6d98dc130040d4f7858fd504f15), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "cbm-sl386sx-bios-hi-v1.02-390915-03-3800.bin", 0x10001, 0x8000, CRC(01815d9d) SHA1(0af291626e71ed65ff6dfee2fe4776a29f2bbb97), ROM_SKIP(1) | ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "c386sxv103", "SL 386SX V1.03") // Commodore 80386SX BIOS Rev. 1.03 -
	// this was replaced with the consistently named ROMs from http://www.zimmers.net/cbmpics/cpcs3.html, the 'hi' ROM looks like a bad dump, with its alternative the POST comes up
	// ROMX_LOAD( "390914-01.u39", 0x10000, 0x8000, CRC(8f849198) SHA1(550b04bac0d0807d6e95ec25391a81272779b41b), ROM_SKIP(1) | ROM_BIOS(3)) /* 390914-01 V1.03 CS-2100 U39 Copyright (C) 1990 CBM */
	// ROMX_LOAD( "390915-01.u38", 0x10001, 0x8000, CRC(ee4bad92) SHA1(6e02ef97a7ce336485814c06a1693bc099ce5cfb), ROM_SKIP(1) | ROM_BIOS(3)) /* 390915-01 V1.03 CS-2100 U38 Copyright (C) 1990 CBM */
	ROMX_LOAD( "cbm-sl386sx-bios-lo-v1.03-390914-03.bin", 0x10000, 0x8000, CRC(8f849198) SHA1(550b04bac0d0807d6e95ec25391a81272779b41b), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "cbm-sl386sx-bios-hi-v1.03-390915-03.bin", 0x10001, 0x8000, CRC(ebdd5097) SHA1(2e4d2375efb9c1ebc0ccf3bb1ff2bb64c449af32), ROM_SKIP(1) | ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "c386sxv104", "SL 386SX V1.04") // Rev. 1.04 - 390914-04/390915-04
	ROMX_LOAD( "cbm-sl386sx-bios-lo-v1.04-390914-04.bin", 0x10000, 0x8000, CRC(377a8e1c) SHA1(9a36f10ad496e44f190937426f3e7de368d6ab7b), ROM_SKIP(1) | ROM_BIOS(4))
	ROMX_LOAD( "cbm-sl386sx-bios-hi-v1.04-390915-04.bin", 0x10001, 0x8000, CRC(4149f5d9) SHA1(9a62b235ac45145ca6720d11b2cbc17b8c25704a), ROM_SKIP(1) | ROM_BIOS(4))
ROM_END

// Commodore 386SX-25 - Form factor: slimline desktop - Chipset: : VLSI 82C311, unreadable, Acer - CPU: i386sx-25, FPU socket provided - On board: Keyboard (DIN), mouse (mini DIN), par,
// 2xser, beeper, Floppy, IDE - VGA on board: acumos AVGA2-340-1 - Mass storage: 3.5" FDD, 80MB IDE HDD - RAM: 8xSIMM30 - OSC: 24.000MHz, 50.000MHz, unreadable - ISA16: 1 (for riser card)
ROM_START( c386sx25 ) // BIOS-String: Award Modular BIOS v4.20 (80386DX) Commodore 386sx-25 BIOS-Version 1.09 391443-09
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "c386sx25_f000.rom", 0x10000, 0x10000, CRC(1322064d) SHA1(6ce16a956b8454d3746ccb923fac45924e4302a4))

	ROM_REGION( 0x8000, "vga", 0)
	ROM_LOAD( "c386sx25_c000.rom", 0x0000, 0x8000, CRC(92212c6b) SHA1(61d222872bf4e21de053705c267103e409adb0ab))
ROM_END

// Commodore Laptop C386SX-LT -  screen remains blank
ROM_START( c386sxlt )
	ROM_REGION16_LE(0x20000, "bios", 0) // BIOS contains Cirrus Logic VGA firmware, rebadged Sanyo MBC-18NB, but different versions exist
	ROM_SYSTEM_BIOS(0, "c386sxlt_b400", "C386SX-LT V1.2 B400")
	ROMX_LOAD( "cbm-386lt-bios-v1.2-390981-03-b400.bin", 0x00000, 0x20000, CRC(b84f6883) SHA1(3f31060726c7c49a891b35ab024524a4239eb4d0), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "c386sxlt_cf00", "C386SX-LT V1.2 CF00")
	ROMX_LOAD( "cbm-386lt-bios-v1.2-390982-03-cf00.bin", 0x00000, 0x20000, CRC(c8cd2641) SHA1(18e55bff494c42389dfb445f2bc11e78db30e5f7), ROM_BIOS(1))
ROM_END

// Commodore DT386
ROM_START( dt386 )
	// BIOS-String: 40-0502-DG1112-00101111-070791-SOLUTION-0 / 386DX-33 BIOS V1.00 #391560-0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "dt386vp10", "DT386 V.pre 1.0")
	ROMX_LOAD( "cbm-dt386dx-33c-bios-hi-vpre1.0-391560-01.bin", 0x10000, 0x10000, CRC(600472f4) SHA1(2513c8bdb24fe27f73c82cbca9e1a983e4a0ba10), ROM_BIOS(0))
	// BIOS-String: 40-0500-DG112-00101111-070791-SOLUTION-0 / 386DX-33 Rev.1E (091592)
	ROM_SYSTEM_BIOS(1, "dt386v1e", "DT386 V.1e")
	ROMX_LOAD( "cbm-dt386dx-33c-bios-hi-v-xxxxxx-xx.bin", 0x10000, 0x10000, CRC(dc1ca1b5) SHA1(7441cb9d5ad5ca6e6425de73295eb74d1281929f), ROM_BIOS(1))
	// BIOS-String: 40-0500-DG1112-00101111-070791-SOLUTION-0 / 386DX-33 Rev.1E (091592) / Commodore BIOS Version 1.0 391560-01
	ROM_SYSTEM_BIOS(2, "dt386v10", "DT386 V.1.0")
	ROMX_LOAD( "cbm-dt386dx-33c-bios-hi-v1.00-391560-01.bin", 0x10000, 0x10000, CRC(da1f7e6d) SHA1(b825fc015233e7eef93a3abbdfc3eeb0da096f50), ROM_BIOS(2))
	// BIOS-String: 40-0501-DG1112-00101111-070791-SOLUTION-0 / Commodore 386DX-33 BIOS Rev. 1.01 391560-02
	ROM_SYSTEM_BIOS(3, "dt386v101", "DT386 V.1.01")
	ROMX_LOAD( "cbm-dt386dx-33c-bios-hi-v1.01-391560-02.bin", 0x10000, 0x10000, CRC(b3157f57) SHA1(a1a96c8d111e3c1da8f655b4b7e1c5be4af140e9), ROM_BIOS(3))
ROM_END

// Commodore DT486 - BIOS contains Paradise VGA ROM - Keyboard error
ROM_START( dt486 ) // BIOS string: 41-0102-001283-00111111-060692-SYM_486-0 - Commodore 486DX-33 BIOS Version 1.01 391521-02
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "dt486", "DT486")
	ROM_LOAD( "cbm-dt486dx-33c-bios-u32--v1.01-391521-02.bin", 0x00000, 0x20000, BAD_DUMP CRC(a3977625) SHA1(83bc563fb41eae3dd5d260f13c6fe8979a77e99c))
ROM_END

// Commodore PC 30-III
ROM_START( pc30iii ) // Chipset: MOS 5720 1788 41, Faraday FE3010B, FE3020, FE3000A, FE3030 - ISA8: 1, ISA16: 3
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "pc30iii_v200", "PC 30-III v2.00")
	ROMX_LOAD( "pc30iii_390339-02_3e58.bin", 0x18000, 0x4000, CRC(f4a5860e) SHA1(b843744fe928bcfd8e037b0208cc85c0746535cf),ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "pc30iii_390340-02_42a8.bin",  0x18001, 0x4000, CRC(934df54a) SHA1(3b1c8916ba2b2517bc9f26dd74254586bcf0e91d),ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "pc30iii_v201", "PC 30-III v2.01")
	ROMX_LOAD( "cbm-pc30c-bios-lo-v2.01-390339-03-35c1.bin", 0x18000, 0x4000, CRC(36307aa9) SHA1(50237ffea703b867de426ab9ebc2af46bac1d0e1),ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "cbm-pc30c-bios-hi-v2.01-390340-03-3f3f.bin",  0x18001, 0x4000, CRC(41bae42d) SHA1(27d6ad9554be86359d44331f25591e3122a31519),ROM_SKIP(1) | ROM_BIOS(1) )
ROM_END

// Commodore PC 40-III
// Chipset: MOS 5720 3888 41, Faraday FE3010B, FE3000A, FE3030, FE3020 - onboard VGA is a Paradise PVGA1A - ISA8: 1, ISA16: 3
ROM_START( pc40iii )
	// VGA BIOS
	// ROM_LOAD( "pc40iii_390337-01_v2.0_f930.bin", 0x00000, 0x4000, CRC(82b210d3) SHA1(1380107deef02455c6ce4d12162fdc32e375cbde))
	// ROM_LOAD( "pc40iii_390338-01_v2.0_b6d0.bin", 0x00001, 0x4000, CRC(526d7424) SHA1(60511ca0e856b7611d556aa82219d646f96c9b94))

	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "pc40iii_v200", "PC 40-III v2.00")
	ROMX_LOAD( "pc40iii_390339-01_v2.0_473a.bin", 0x18000, 0x4000, CRC(2ad2dc0f) SHA1(b41d5988fda8cc23418c3f665d780c617aa3fc2b),ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "pc40iii_390340-01_v2.0_4bc6.bin",  0x18001, 0x4000, CRC(62dc7d93) SHA1(e741528697b1d00450fd18e3db8b925606e0bd22),ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "pc40iii_v201", "PC 40-III v2.03")
	ROMX_LOAD( "cbm-pc40c-bios-lo-v2.03-390339-04-03bc.bin", 0x18000, 0x4000, CRC(e5fd11c6) SHA1(18c21d9a4ae687eef5464b76a0d614b9dfd30ec8),ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "cbm-pc40c-bios-hi-v2.03-390340-04-3344.bin",  0x18001, 0x4000, CRC(63d6f0f7) SHA1(a88dee7694baa71913acbe76cb4e2a4e95979ad9),ROM_SKIP(1) | ROM_BIOS(1) )
ROM_END

// Commodore PC 45-III - this is a PC 40-III with a BIOS update and a bigger, 52MB, harddisk
ROM_START( pc45iii )
	ROM_REGION16_LE(0x20000, "bios", 0) // Commodore 286 BIOS Rev. 2.04 - 390339-05/390340-05
	ROM_LOAD16_BYTE( "cbm-pc45c-bios-lo-v2.04-390339-05.bin", 0x18000, 0x4000, CRC(b87b4cd1) SHA1(a6723d63a255b4010ad32b5dc9797e4724a64c14))
	ROM_LOAD16_BYTE( "cbm-pc45c-bios-hi-v2.04-390340-05.bin", 0x18001, 0x4000, CRC(b6976111) SHA1(e7c92307db3969a6a50ffd8cbc3d2ed16b4df6ad))
ROM_END

// Commodore PC 50-II - a photo of the mainboard shows four ROMs (two each for BIOS and VGA), so the 128K dumps available were probably made from a running system.
ROM_START( pc50ii ) // Chipset: Chips P82C211-12 C(16MHz), P82C212B-12 (16MHz), P82C215-12, P82C206, VLSI 8942VT - ISA8: 1, ISA16: 5
	ROM_REGION16_LE(0x20000, "bios", 0) // keyboard MCU is P8042AH MITAC V2.48 (undumped), onboard video PVGA1A-JK
	// 0: Commodore PC50-II BIOS Rev1.0 - 609200-03
	ROM_SYSTEM_BIOS(0, "pc50iiv100", "PC 50-II V1.00") // complains "Time-of-day clock stopped" and reboots
	ROMX_LOAD( "cbm-pc50b-bios-lo-v1.00-390339-01.bin", 0x10001, 0x8000, CRC(0f0e2fd6) SHA1(61a8043ac919c2a8fe668bf25e5f0b67868d11ae),ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "cbm-pc50b-bios-hi-v1.00-390340-01.bin", 0x10000, 0x8000, CRC(87008421) SHA1(cf41973a7bd439441baec1138dd63044fafe7391),ROM_SKIP(1) | ROM_BIOS(0) )
	// 1: Commodore PC50-II BIOS Rev1.01 - 609200-03
	ROM_SYSTEM_BIOS(1, "pc50iiv101", "PC 50-II V1.01") // same behaviour as above
	ROMX_LOAD( "cbm-pc50b-bios-lo-u31-v1.01-xxxxxx-xx-a800.bin", 0x10001, 0x8000, CRC(bf2c7009) SHA1(6b94df37861b30ef6a39a4ed64d4c9ac1e96043a),ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "cbm-pc50b-bios-hi-u28-v1.01-xxxxxx-xx-cd00.bin",  0x10000, 0x8000, CRC(628fcb2f) SHA1(74241cbcb4e183015d5e7a516d46b08d6f47504a),ROM_SKIP(1) | ROM_BIOS(1) )
	// 2: Commodore PC50-II BIOS Rev1.02 - 609200-03
	ROM_SYSTEM_BIOS(2, "pc50iiv102", "PC 50-II V1.02") // same behaviour as above
	ROMX_LOAD( "cbm-pc50b-bios-lo-u32-v1.02-609200-03o-9e00.bin", 0x10001, 0x8000, CRC(57225c22) SHA1(3b2ded119480ce2dd5bb7c113c5814ce47e17d4c),ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "cbm-pc50b-bios-hi-u27-v1.02-609200-03e-c400.bin", 0x10000, 0x8000, CRC(4ec903af) SHA1(fb70e22c0538d7310c9034626d4d9c0e4f63dfd7),ROM_SKIP(1) | ROM_BIOS(2))

	// VGA BIOS
	// ROM_LOAD( "m_pc50-ii_1bad_pvgadk_odd.bin", 0x00000, 0x8000, CRC(f36eca7e) SHA1(4335fa4a4567cbc010ff2ffeb97a536ed93b0219))
	// ROM_LOAD( "m_pc50-ii_54e3_pvgadk_even.bin", 0x00001, 0x8000, CRC(01f6b964) SHA1(799a84ddde8a7672a6df9439bad6198ec3ff98ec))
ROM_END

// Commodore PC-60-III - complaining "time-of-day-clock stopped" - Phoenix P8242 '87 keyboard BIOS
ROM_START( pc60iii ) // onboard Paradise PVGA1A-JK, 2xRS232, 1xparallel, keyboard
	ROM_REGION32_LE(0x20000, "bios", 0) // Chipset: Chips P82C301C, P82B305, P82A303, P82A304, Chips F82307, 25531/390423-1 COMBO SMC B9016, Commodore CSG, four MOSEL MS82C308-35JC, ISA16: 7, RAM card: 2 (up to 8MB, extended ISA connector)
	// 0: Commodore PC60-III 80386 BIOS Rev. 1.2 - 390473-01/390474-01
	ROM_SYSTEM_BIOS(0, "pc60iiiv12", "PC60-III V1.2")
	ROMX_LOAD( "cbm-pc60c-bios-lo_u73-v1.2-390473-01.bin", 0x00000, 0x10000, CRC(ff2cd8b3) SHA1(62e95f818c5016f4be2741872dc644999dee33ce),ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "cbm-pc60c-bios-hi_u67-v1.2-390474-01.bin", 0x00001, 0x10000, CRC(690fff4b) SHA1(adc262d40da64354c7c76b61f46d2c7ed35e9df9),ROM_SKIP(1) | ROM_BIOS(0) )
	// 1: Commodore PC-60-III 80386/25MHz BIOS Rev. 1.3 390473-02/390474-02
	ROM_SYSTEM_BIOS(1, "pc60iiiv13", "PC60-III V1.3")
	ROMX_LOAD( "cbm-pc60c-bios-lo-v1.30-390473-02.bin", 0x00000, 0x10000, CRC(3edd83e0) SHA1(3ebf393d6c33d9b8600f56c7be9eedb5aefb2645),ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "cbm-pc60c-bios-hi-v1.30-390474-02.bin", 0x00001, 0x10000, CRC(12209ac4) SHA1(76f271944894c77dde735da2b2ba065e81a99564),ROM_SKIP(1) | ROM_BIOS(1) )
	// 2: Commodore PC60-III 80386/25MHz BIOS rev.1.33 390473-04/390474-04
	ROM_SYSTEM_BIOS(2, "pc60iiiv133", "PC60-III V1.33")
	ROMX_LOAD( "cbm-pc60-bios-lo-v1.33-390473-04.bin", 0x00000, 0x10000, CRC(afd0aae0) SHA1(7fa4388c939f30e603f0fc90f9512e500b282432),ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "cbm-pc60-bios-hi-v1.33-390474-04.bin", 0x00001, 0x10000, CRC(7b7958db) SHA1(d542c63ec0d17e1e87403ac01735e75ce58302a9),ROM_SKIP(1) | ROM_BIOS(2) )
	// 3: Commodore PC60-III 80386-25MHz BIOS Rev.1.3.5 - 390473-06/390474-06
	ROM_SYSTEM_BIOS(3, "pc60iiiv135", "PC60-III V1.3.5")
	ROMX_LOAD( "cbm-pc60c-bios-lo-v1.35-390473-06.bin", 0x00000, 0x10000, CRC(6ff4aea9) SHA1(3fcb3a5c275dbfb93c3e55224d731f1b52343d4b),ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "cbm-pc60c-bios-hi-v1.35-390474-06.bin", 0x00001, 0x10000, CRC(5a04e3f0) SHA1(311a3ff3e578ecbce0ecd9f3b006ab772623255a),ROM_SKIP(1) | ROM_BIOS(3) )
	// 4: Commodore 80386 BIOS Rev.1.36 - 390473-07/390474-07
	ROM_SYSTEM_BIOS(4, "c386v136", "Commodore 386 V1.3.6")
	ROMX_LOAD( "cbm-pc60c-bios-lo-v1.36-390473-07-9b0e.bin", 0x00000, 0x10000, CRC(be7504f8) SHA1(a45f7690a41d416bc10ca6f583b8fdd2219a3d8a),ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "cbm-pc60c-bios-hi-v1.36-390474-07-ddf2.bin", 0x00001, 0x10000, CRC(d8e08ffa) SHA1(fb5fb973b01df6e486d76076d3373583758b1d01),ROM_SKIP(1) | ROM_BIOS(4) )
	// 5: Commodore 80386 BIOS Rev.1.36.03 - 390473-07/390474-07
	ROM_SYSTEM_BIOS(5, "c386v13603", "Commodore 386 V1.3.603")
	ROMX_LOAD( "cbm-pc60c-bios-lo-v1.3603-390473-07.bin", 0x00000, 0x10000, CRC(2cda07c7) SHA1(01fd6260192541dd73f88d2cc0f99fe5603efc81),ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "cbm-pc60c-bios-hi-v1.3603-390474-07.bin", 0x00001, 0x10000, CRC(39845b9b) SHA1(9d3cbfde4b2acc1d576aafa80126b75a49d3d8df),ROM_SKIP(1) | ROM_BIOS(5) )
ROM_END

// Commodore PC-70-III - complaining "time-of-day-clock stopped"
ROM_START( pc70iii )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: Commodore 80486 BIOS Rev.1.00 - 390934-01/390935-01
	ROM_SYSTEM_BIOS(0, "pc70v100", "PC70 V1.00")
	ROMX_LOAD("cbm-pc70c_bios-u117-lo-v1.00-390934-01.bin", 0x00000, 0x10000, CRC(3eafd811) SHA1(4deecd5dc429ab09e7c0d308250cb716f8b8e42a), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("cbm-pc70c_bios-u112-hi-v1.00-390935-01.bin", 0x00001, 0x10000, CRC(2d1dfec9) SHA1(d799b3579577108549d9d4138a8a32c35ac3ce1c), ROM_SKIP(1) | ROM_BIOS(0))
	// 1: Commodore PC70-III 80486/25MHz BIOS Rev.1.00.01 - xxxxxx - 00/xxxxxx-00
	ROM_SYSTEM_BIOS(1, "pc70v101", "PC70 V1.00.01")
	ROMX_LOAD("cbm-pc70c-bios-lo-v1.00.01-xxxxxx-00.bin", 0x00000, 0x10000, CRC(6c8bbd31) SHA1(63d1739a58a0d441ebdd543e3994984c433aedb4), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("cbm-pc70c-bios-hi-v1.00.01-xxxxxx-00.bin", 0x00001, 0x10000, CRC(ef279cdd) SHA1(d250368b2f731e842d6f280a6134f1e38846874b), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

// Commodore Tower 386
ROM_START( comt386 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// Phoenix 80386 ROM BIOS PLUS Version 1.10 22 - Twinhead International Corporation
	ROM_LOAD16_BYTE( "cbm-t386-bios-lo-v1.1022c-.bin", 0x10000, 0x8000, CRC(6857777e) SHA1(e80dbffd3523c9a1b027f57138c55768fc8328a6))
	ROM_LOAD16_BYTE( "cbm-t386-bios-hi-v1.1022c-.bin", 0x10001, 0x8000, CRC(6a321a7e) SHA1(c350fb273522f742c6008deda00ed13947a269b7))
ROM_END

// Commodore Tower 486
ROM_START( comt486 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 40-0500-DG1112-00101111-070791-SOLUTION-0 - 4D3FF Rev.D (092892)
	ROM_SYSTEM_BIOS(0, "v0", "Tower 486 V0")
	ROMX_LOAD( "cbm-t486dx-bios-v-xxxxxx-xx.bin", 0x10000, 0x10000, CRC(f51c0ca0) SHA1(2b08a606ae2f37b3e72d687f890d729a58fd3ccd), ROM_BIOS(0))
	// continuous chirps
	ROM_SYSTEM_BIOS(1, "v1", "Tower 486 V1")
	ROMX_LOAD( "cbm-t486dx-66-bios-v1.01-391566-02.bin", 0x10000, 0x10000, CRC(3d740698) SHA1(888f23d85b41c07e15e2811b76194cf478bc80cd), ROM_BIOS(1))
	// BIOS-String: 40-0103-001283-00101111-0606-SYM_486-0 - Commodore 486DX2-66 BIOS Version 1.03 391684-02
	ROM_SYSTEM_BIOS(2, "v2", "Tower 486 V2")
	ROMX_LOAD( "cbm-t486dx-66-bios-v1.03-391684-02.bin", 0x10000, 0x10000, CRC(13e8b04b) SHA1(dc5c84d228f802f7580b3f3b8e70cf8f74de5d79), ROM_BIOS(2))
	// BIOS-String: 40-0103-001283-00101111-060692-SYM_486-0 - Commodore 486DX-50 BIOS Version 1.03 391522-03
	ROM_SYSTEM_BIOS(3, "v3", "Tower 486 V3")
	ROMX_LOAD( "cbm-t486dx-50-bios-v1.03-.bin", 0x10000, 0x10000, CRC(e02bb928) SHA1(6ea121b214403390d382ca4685cfabcbcca1a28b), ROM_BIOS(3))
ROM_END


//**************************************************************************
//  80286 BIOS
//**************************************************************************

ROM_START( at )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: BIOS-String: D286-0011-110387
	ROM_SYSTEM_BIOS(0, "at", "PC 286") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 03-11-1987)*/
	ROMX_LOAD( "at110387.1", 0x10001, 0x8000, CRC(679296a7) SHA1(ae891314cac614dfece686d8e1d74f4763cf40e3),ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "at110387.0", 0x10000, 0x8000, CRC(65ae1f97) SHA1(91a29c7deecf7a9afbba330e64e0eee9aafee4d1),ROM_SKIP(1) | ROM_BIOS(0) )
	// 1: BIOS-String: S286-6181-101590-K0 - additional info from chukaev.ru54.com: UMC chipset
	ROM_SYSTEM_BIOS(1, "ami206", "AMI C 206.1")  /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 15-10-1990)*/
	ROMX_LOAD( "amic206.bin",    0x10000, 0x10000,CRC(25a67c34) SHA1(91e9d8cdc2f1b40a601a23ceaff2189fd1245f3b), ROM_BIOS(1) )
	// 2: (BIOS release date:: 07-07-1991) - Chipset: Headland HT21/E
	ROM_SYSTEM_BIOS(2, "amiht21", "AMI HT 21.1") /* as above */
	ROMX_LOAD( "ht21e.bin",    0x10000, 0x10000, CRC(e80f7fed) SHA1(62d958d98c95e9e4d1b290a6c1054ae98770f276), ROM_BIOS(2) )
	// 3: BIOS-String: D286-1430-040990-K0 - additional info from chukaev.ru54.com: Chipset: TI TACT8230... 1BPB, 2BPB, 3EPB - ISA8: 3, ISA16: 5
	ROM_SYSTEM_BIOS(3, "amip1", "AMI P.1") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "poisk-h.bin",   0x10001, 0x8000, CRC(83fd3f8c) SHA1(ca94850bbd949b97b11710629886b0ee69489a81),ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "poisk-l.bin",   0x10000, 0x8000, CRC(0b2ed291) SHA1(bb51a3f317cf4d429a6cfb44a46ca0ac39d9aaa7),ROM_SKIP(1) | ROM_BIOS(3) )
	// 4: Award 286 Modular BIOS Version 3.11 - WINBOND
	ROM_SYSTEM_BIOS(4, "aw201", "Award 201") /* (BIOS release date:: 21-11-1990) */
	ROMX_LOAD( "83201-5h.bin",  0x10001, 0x8000, CRC(968d1fc0) SHA1(dc4122a6c696f0b43e7894dc1b669346eed755d5),ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "83201-5l.bin",  0x10000, 0x8000, CRC(bf50a89a) SHA1(2349a1db6017a7fb0673e99d3680c8753407be8d),ROM_SKIP(1) | ROM_BIOS(4) )
	// 5: Award 286 Modular BIOS V3.03 NFS 11/10/87" - T.M.C
	ROM_SYSTEM_BIOS(5, "aw303", "Award 303 NFS") /* (BIOS release date:: 15-11-1985) */
	ROMX_LOAD( "aw303-hi.bin",  0x18001, 0x4000, CRC(78f32d7e) SHA1(1c88398fb171b33b7e6191bad63704ae85bfed8b), ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "aw303-lo.bin",  0x18000, 0x4000, CRC(3d2a70c0) SHA1(1329113bec514ed2a6d803067b1132744ef534dd), ROM_SKIP(1) | ROM_BIOS(5) )
	// 6: Award 286 Modular BIOS Version 3.03GS
	ROM_SYSTEM_BIOS(6, "aw303gs", "Award 303GS") /* (BIOS release date:: 15-11-1985) */
	ROMX_LOAD( "aw303gs-hi.bin",  0x18001, 0x4000, CRC(82392e18) SHA1(042453b7b29933a1b72301d21fcf8fa6b293c9c9), ROM_SKIP(1) | ROM_BIOS(6) )
	ROMX_LOAD( "aw303gs-lo.bin",  0x18000, 0x4000, CRC(a4cf8ba1) SHA1(b73e34be3b2754aaed1ac06471f4441fea06c67c), ROM_SKIP(1) | ROM_BIOS(6) )
	// 7: BIOS-String: D286-6069-040990-K0
	ROM_SYSTEM_BIOS(7, "ami_200960", "AMI 200960") /* (BIOS release date:: 09-04-1990 */
	ROMX_LOAD( "ami_286_bios_sn200960_even.bin", 0x10000, 0x8000, CRC(67745815) SHA1(ca6886c7a0716a92a8720fc71ff2d95328c467a5), ROM_SKIP(1) | ROM_BIOS(7) )
	ROMX_LOAD( "ami_286_bios_sn200960_odd.bin", 0x10001, 0x8000, CRC(360a5f73) SHA1(1b1980fd99779d0cdc4764928a641e081b35ee9f), ROM_SKIP(1) | ROM_BIOS(7) )
	// 8: BIOS-String: DVL2-1160-040990-K0
	ROM_SYSTEM_BIOS(8, "dvl2", "DVL2") /* (BIOS release date:: 09-04-1990) */
	ROMX_LOAD( "ami_dvl2-1160-040990-k8_even.bin", 0x10000, 0x8000, CRC(86093016) SHA1(f60b2679c8c23a34bdd64f25d83cb5a5a337bd57), ROM_SKIP(1) | ROM_BIOS(8) )
	ROMX_LOAD( "ami_dvl2-1160-040990-k8_odd.bin", 0x10001, 0x8000, CRC(4e1c944a) SHA1(0763a0a1002baced071fea301f627d2e550878b8), ROM_SKIP(1) | ROM_BIOS(8) )
	// 9: DTK 286 BIOS Ver 3.18 07/01/88
	ROM_SYSTEM_BIOS(9, "dtk318", "DTK v3.18") /* (BIOS release date:: 11-03-1986) */
	ROMX_LOAD( "dtk_286_bios_ver3.18.bin", 0x18000, 0x8000, CRC(b4b8b59a) SHA1(73c12222f5003fdc8bbfee178b20c8dda2fe5cb4), ROM_BIOS(9) )
	// 10: BIOS-String: D286-6061-040990-K0
	ROM_SYSTEM_BIOS(10, "d286-k0", "AMI D286-K0") /* (BIOS release date:: 09-04-1990) */
	ROMX_LOAD( "ami_d286-6061-040990-k0.bin", 0x10000, 0x10000, CRC(1679c1b5) SHA1(9d95da3b40c5f13d096823f383aba099b3a77183), ROM_BIOS(10) )
	// 11: BIOS-String: S286-1169-030389-K0 for ACHIEVE MICROSYSTEMS
	ROM_SYSTEM_BIOS(11, "s286-k0", "Achieve S286-K0") /* (BIOS release date:: 03-03-1989) */
	ROMX_LOAD( "ach_s286-1169-030389-k0_ev.bin", 0x10000, 0x8000, CRC(58f1f29c) SHA1(42f5189d12b75fad5e53ff472b4603c6fcbd46cd), ROM_SKIP(1) | ROM_BIOS(11) )
	ROMX_LOAD( "ach_s286-1169-030389-k0_od.bin", 0x10001, 0x8000, CRC(84bfc180) SHA1(2daa51b09c449712c9a737793b83754951e53a41), ROM_SKIP(1) | ROM_BIOS(11) )
	// 12: Award BIOS Version 3.01B
	ROM_SYSTEM_BIOS(12, "awa301b", "Award BIOS Version 3.01B") /* (BIOS release date:: 01-01-1988) */
	ROMX_LOAD( "aw286lo.rom", 0x18000, 0x4000, CRC(5afbb4a2) SHA1(513fd75d90720820484fdd280e4a6c22a0ef238c), ROM_SKIP(1) | ROM_BIOS(12) )
	ROMX_LOAD( "aw286hi.rom", 0x18001, 0x4000, CRC(b2551251) SHA1(0c8bd12a3d54ae6d2ad0210b9ca4deca94be10ed), ROM_SKIP(1) | ROM_BIOS(12) )
	// 13: no screen display
	ROM_SYSTEM_BIOS(13, "awa286", "awa286") /* (BIOS release date:: 21-11-1990) */
	ROMX_LOAD( "awd286lo.rom", 0x18000, 0x4000, CRC(d1a9c01f) SHA1(9123c6f76d85725036a0f8b9c6480142abea478f), ROM_SKIP(1) | ROM_BIOS(13) )
	ROMX_LOAD( "awd286hi.rom", 0x18001, 0x4000, CRC(b0bde4cc) SHA1(9c3fd2c0f69dde905d4e8f3be421374ef99682df), ROM_SKIP(1) | ROM_BIOS(13) )
	// 14: DTK 286 BIOS Ver. 3.01 07/24/87 - no screen display
	ROM_SYSTEM_BIOS(14, "dtk286", "dtk286") /* (BIOS release date:: 11-03-1986) */
	ROMX_LOAD( "dtk286lo.rom", 0x18000, 0x4000, CRC(dfc70856) SHA1(39158e6ed50236d371277631e77d06f77fb0531e), ROM_SKIP(1) | ROM_BIOS(14) )
	ROMX_LOAD( "dtk286hi.rom", 0x18001, 0x4000, CRC(a98fc743) SHA1(fb9e330148cb5584f61c1febea71c53b6f9d61b7), ROM_SKIP(1) | ROM_BIOS(14) )
	// 15: Phoenix 80286 ROM BIOS Version 3.07 (R04)
	ROM_SYSTEM_BIOS(15, "mitph307", "Mitac Phoenix v3.07") /* (BIOS release date:: 30-07-1987) */
	ROMX_LOAD( "mitac_phoenix_v3.07_even.bin", 0x10000, 0x8000, CRC(1c4becc9) SHA1(bfdea3f2a248312ed8cf4765a1a7dc1a2f7cecd8), ROM_SKIP(1) | ROM_BIOS(15) )
	ROMX_LOAD( "mitac_phoenix_v3.07_odd.bin", 0x10001, 0x8000, CRC(3ee16ed1) SHA1(b77e18e10e9187a01cb55c05b2a6e5311981ab56), ROM_SKIP(1) | ROM_BIOS(15) )
	// 16: BIOS-String: Pyramid Software Development Personal Computer AT BIOS Version 2.14
	ROM_SYSTEM_BIOS(16, "precise", "Precise")  /* (no regular BIOS release date) */
	ROMX_LOAD( "precise 860407_low.bin", 0x10000, 0x8000, CRC(d839c074) SHA1(473ca7b42914ce12f2d6c91afb0b2c2e65194489), ROM_SKIP(1) | ROM_BIOS(16) )
	ROMX_LOAD( "precise 860407_high.bin", 0x10001, 0x8000, CRC(b5e13c54) SHA1(07f5806fb53d0cb7ef7b54312fd6aa163d58b9a5), ROM_SKIP(1) | ROM_BIOS(16) )
	// ROM_LOAD( "precise_860407_keyboard_mcu.bin", 0x0000, 0x800, CRC(d1faad5c) SHA1(cb315a3da632c969012c298bb8e1cf8883b70501))
	// 17:  Access Methods Inc. for Flying Triumph (AMI before they became American Megatrends) - BIOS String: Ref. no. 1406-061296
	// complains about "Channel-2 timer not funcional but boots
	ROM_SYSTEM_BIOS(17, "ami_ft", "AMI Flying Triumph") /* (BIOS release date:: 12-06-1986) */
	ROMX_LOAD( "286_access_methods_rom2_32k.bin", 0x10000, 0x8000, CRC(749c65af) SHA1(7c6e9e217afe020b7b36785549fdbfb89de8f872), ROM_SKIP(1) | ROM_BIOS(17) )
	ROMX_LOAD( "286_access_methods_rom4_32k.bin", 0x10001, 0x8000, CRC(0f15581a) SHA1(2a22635f30388ca371f0f1f31652cfa647bb322d), ROM_SKIP(1) | ROM_BIOS(17) )
	// 18: MS-0010-2 - Phoenix ROM BIOS Version 3.06
	ROM_SYSTEM_BIOS(18, "ms-0010-2", "MS-0010-2") /* (BIOS release date:: 19-01-1987) (ISA8: 3, ISA16: 5) */
	ROMX_LOAD( "286-ms0010-2-lo_32k.bin", 0x10000, 0x8000, CRC(2c381474) SHA1(94b9825d412ea39d67857102a0375852b349fcd6), ROM_SKIP(1) | ROM_BIOS(18) )
	ROMX_LOAD( "286-ms0010-2-hi_32k.bin", 0x10001, 0x8000, CRC(4fdb8c64) SHA1(c2e7f88f0ac97ee5eed0c97864b7f1810e99ea26), ROM_SKIP(1) | ROM_BIOS(18) )
	// 19: M219 V2.1 - chipset: Toshiba CHIP2 TC6154AF
	// BIOS-String: X0-0100-001437-00101111-060692-M219-0
	ROM_SYSTEM_BIOS(19, "m219", "Toshiba M219")
	ROMX_LOAD( "3tcm001.bin", 0x10000, 0x10000, CRC(146a42e9) SHA1(cf511919f271e868e34881912c0a1a859d80f91e), ROM_BIOS(19))
	// ***** Motherboards using the original Chips CS8220 chipset: P82C202, P82C201, P82A203, P82A204, P82A205
	// 20: AL-6410 (found online, no markings on the board itself), Chipset: Chips P82A204, P82A203, P82A205, P82C201, P82C202
	ROM_SYSTEM_BIOS(20, "al6410", "AL-6410") /* (BIOS-String: D286-1103-110387-K0) (BIOS release date:: 03-11-1987) (ISA8: 2, ISA16: 6) */
	ROMX_LOAD( "al-6410_ami_bios_low.bin", 0x10000, 0x8000, CRC(50c4e121) SHA1(5f9c27aabdc6bb810e90bced2053b7c21c4994dd), ROM_SKIP(1) |  ROM_BIOS(20) )
	ROMX_LOAD( "al-6410_ami_bios_high.bin", 0x10001, 0x8000, CRC(a44be083) SHA1(99f73d7ceb315eb3770c94d90228f8859cadc610), ROM_SKIP(1) | ROM_BIOS(20) )
	// 21: AT SYSTEM 6M/8M/10M - Chipset: Chips P82A205; P82C201; P82A203; P82A204 - ISA8:2, ISA16: 6
	ROM_SYSTEM_BIOS(21, "at6m8m10m", "AT SYSTEM 6M/8M/10M") // (BIOS release date:: 04-02-1987) - OSC: 20.000000MHz - MQ-14.3 - 12.000
	ROMX_LOAD( "286-at system 6m8m10m-l_32k.bin", 0x10000, 0x8000, CRC(37e0e1c1) SHA1(f5cd17658554a73bb86c5c8e630dac3e34b38e51), ROM_SKIP(1) | ROM_BIOS(21) )
	ROMX_LOAD( "286-at system 6m8m10m-r_32k.bin", 0x10001, 0x8000, CRC(c672efff) SHA1(7224bb6b4d25ef34bc0aa9d7c450baf9b47fd917), ROM_SKIP(1) | ROM_BIOS(21) )
	// 22: CDTEK - BIOS-String: DSUN-1202-042088-K0 286-BIOS AMI for CDTEK - ISA8:2, ISA16:6 - Chipset ICs plus SN76LS612N, RTC MC146818P
	ROM_SYSTEM_BIOS(22, "cdtekchips", "CDTEK 286") // ISA8:2, ISA16: 6 - OSC: 12.000, 14.31818, 16000.00KHz
	ROMX_LOAD( "286-cdtek2-even_32k.bin", 0x10000, 0x8000, CRC(94867e8d) SHA1(12e61cc8b875b57324c93276c9f6093f2bd0e277), ROM_SKIP(1) | ROM_BIOS(22) )
	ROMX_LOAD( "286-cdtek2-odd_32k.bin", 0x10001, 0x8000, CRC(153ed3bd) SHA1(10b711e0f0d79e0b6d181f24fe66544d2d72a310), ROM_SKIP(1) | ROM_BIOS(22) )
	// 23: This board looks identical to #2 but has different chips fitted: SN76LS612N = Zymos HCT612, Chips P82A204 = TACT80204FN, P82A203 = STK-5134, P82A205 = STK-5135,
	// P82C201 = STK-5132, P82C202 = STK-5133 - BIOS-String: Phoenix 80286 ROM BIOS Version 3.06
	ROM_SYSTEM_BIOS(23, "286tact", "286 TACT") // OSC: 20.0000MHz, 14.31818 - 24.000MHz
	ROMX_LOAD( "286-tact-320548-1_32k.bin", 0x10000, 0x8000, CRC(0b528d19) SHA1(15f5a94d89461655c0f74681bbae5745db009ac2), ROM_SKIP(1) | ROM_BIOS(23) )
	ROMX_LOAD( "286-tact-320548-2_32k.bin", 0x10001, 0x8000, CRC(418aa2d0) SHA1(b6af0b8aa595d8f8de6c0fc851bf1c226dcc7ca7), ROM_SKIP(1) | ROM_BIOS(23) )
	// 24: Tulip 286 CPU card - Chipset: TACT 82204FN, 82202N, Chips P82A205, P82C201, P82A203, SN74LS612N, HD146818P, 2xAM9517A-5PC, 2xP8259A
	// CPU: AND N80L286-10, FPU socket provided - OSC: 16.000 - BIOS-String:
	ROM_SYSTEM_BIOS(24, "286tu", "Tulip 286 CPU card") // no display
	ROMX_LOAD( "tc7be.bin", 0x18000, 0x4000, CRC(260c6994) SHA1(a7e28c2978faaa9c5ccab32932ef1391c1b3d35a), ROM_SKIP(1) | ROM_BIOS(24) )
	ROMX_LOAD( "tc7bo.bin", 0x18001, 0x4000, CRC(c8373edc) SHA1(77ce220914863f482a3a983b43ff8ca8c72b470c), ROM_SKIP(1) | ROM_BIOS(24) )
ROM_END

ROM_START( atturbo )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 20-0001-001223-00101111-050591-KB-8042--0 - additional info from chukaev.ru54.com: Chipset: VLSI VL82C311L-FC4, VL82C113A-FC
	ROM_SYSTEM_BIOS(0, "vl82c", "VL82C311L-FC4")/*(Motherboard Manufacturer: Biostar Microtech Corp.) (BIOS release date: 05-05-1991)*/
	ROMX_LOAD( "2vlm001.bin",     0x10000, 0x10000, CRC(f34d800a) SHA1(638aca592a0e525f957beb525e95ca666a994ee8), ROM_BIOS(0) )
	// 1: same as BIOS '1' in at
	ROM_SYSTEM_BIOS(1, "ami206", "AMI C 206.1") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 15-10-1990)*/
	ROMX_LOAD( "amic206.bin",    0x10000, 0x10000,CRC(25a67c34) SHA1(91e9d8cdc2f1b40a601a23ceaff2189fd1245f3b), ROM_BIOS(1) )
	// 2: same as BIOS '7' in at
	ROM_SYSTEM_BIOS(2, "amiht21", "AMI HT 21.1") /* not a bad dump, sets unknown probably chipset related registers at 0x1e8 before failing post */
	ROMX_LOAD( "ht21e.bin",    0x10000, 0x10000, CRC(e80f7fed) SHA1(62d958d98c95e9e4d1b290a6c1054ae98770f276), ROM_BIOS(2) )
	// 3: same as BIOS '8' in at
	ROM_SYSTEM_BIOS(3, "amip1", "AMI P.1") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "poisk-h.bin",   0x10001, 0x8000, CRC(83fd3f8c) SHA1(ca94850bbd949b97b11710629886b0ee69489a81),ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "poisk-l.bin",   0x10000, 0x8000, CRC(0b2ed291) SHA1(bb51a3f317cf4d429a6cfb44a46ca0ac39d9aaa7),ROM_SKIP(1) | ROM_BIOS(3) )
	// 4: BIOS-String: DG22-1131-040990-K0 / 286-BIOS G2 V1.1 6-28-90 - Headland GC102/GC113-PC/HT101A - CPU/FPU: N80286-12, IIT2C87-10
	ROM_SYSTEM_BIOS(4, "ami1131", "AMI-1131") /*(Motherboard Manufacturer: Elitegroup Computer Co., Ltd.) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "2hlm003h.bin",   0x10001, 0x8000, CRC(2babb42b) SHA1(3da6538f44b434cdec0cbdddd392ccfd34666f06),ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "2hlm003l.bin",   0x10000, 0x8000, CRC(317cbcbf) SHA1(1adad6280d8b07c2921fc5fc13ecaa10e6bfebdc),ROM_SKIP(1) | ROM_BIOS(4) )
	// 5: same as BIOS '0' in at
	ROM_SYSTEM_BIOS(5, "at", "PC 286") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 03-11-1987)*/
	ROMX_LOAD( "at110387.1", 0x10001, 0x8000, CRC(679296a7) SHA1(ae891314cac614dfece686d8e1d74f4763cf40e3),ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "at110387.0", 0x10000, 0x8000, CRC(65ae1f97) SHA1(91a29c7deecf7a9afbba330e64e0eee9aafee4d1),ROM_SKIP(1) | ROM_BIOS(5) )
	// 6
	ROM_SYSTEM_BIOS(6, "bravo", "AST Bravo/286") // fails with keyboard controller test, probably expects specific kbdc rom
	ROMX_LOAD( "107000-704.bin", 0x10000, 0x8000, CRC(94faf87e) SHA1(abaafa6c2ae9b9fba95b244dcbcc1c752ac6c0a0),ROM_SKIP(1) | ROM_BIOS(6) )
	ROMX_LOAD( "107000-705.bin", 0x10001, 0x8000, CRC(e1263c1e) SHA1(b564f1043ef45ecbdf4f06bb500150ad992c2931),ROM_SKIP(1) | ROM_BIOS(6) )
	// ***** Motherboards using the original Chips CS8220 chipset: P82C202, P82C201, P82A203, P82A204, P82A205
	// 7 same as BIOS '20' in at
	ROM_SYSTEM_BIOS(7, "al6410", "AL-6410")
	ROMX_LOAD( "al-6410_ami_bios_low.bin", 0x10000, 0x8000, CRC(50c4e121) SHA1(5f9c27aabdc6bb810e90bced2053b7c21c4994dd), ROM_SKIP(1) |  ROM_BIOS(7) )
	ROMX_LOAD( "al-6410_ami_bios_high.bin", 0x10001, 0x8000, CRC(a44be083) SHA1(99f73d7ceb315eb3770c94d90228f8859cadc610), ROM_SKIP(1) | ROM_BIOS(7) )
	// 8: same as BIOS '21' in at
	ROM_SYSTEM_BIOS(8, "at6m8m10m", "AT SYSTEM 6M/8M/10M")
	ROMX_LOAD( "286-at system 6m8m10m-l_32k.bin", 0x10000, 0x8000, CRC(37e0e1c1) SHA1(f5cd17658554a73bb86c5c8e630dac3e34b38e51), ROM_SKIP(1) | ROM_BIOS(8) )
	ROMX_LOAD( "286-at system 6m8m10m-r_32k.bin", 0x10001, 0x8000, CRC(c672efff) SHA1(7224bb6b4d25ef34bc0aa9d7c450baf9b47fd917), ROM_SKIP(1) | ROM_BIOS(8) )
	// 9: same as BIOS '22' in at
	ROM_SYSTEM_BIOS(9, "cdtekchips", "CDTEK 286")
	ROMX_LOAD( "286-cdtek2-even_32k.bin", 0x10000, 0x8000, CRC(94867e8d) SHA1(12e61cc8b875b57324c93276c9f6093f2bd0e277), ROM_SKIP(1) | ROM_BIOS(9) )
	ROMX_LOAD( "286-cdtek2-odd_32k.bin", 0x10001, 0x8000, CRC(153ed3bd) SHA1(10b711e0f0d79e0b6d181f24fe66544d2d72a310), ROM_SKIP(1) | ROM_BIOS(9) )
	// 10: same as BIOS '23' in at
	ROM_SYSTEM_BIOS(10, "286tact", "286 TACT")
	ROMX_LOAD( "286-tact-320548-1_32k.bin", 0x10000, 0x8000, CRC(0b528d19) SHA1(15f5a94d89461655c0f74681bbae5745db009ac2), ROM_SKIP(1) | ROM_BIOS(10) )
	ROMX_LOAD( "286-tact-320548-2_32k.bin", 0x10001, 0x8000, CRC(418aa2d0) SHA1(b6af0b8aa595d8f8de6c0fc851bf1c226dcc7ca7), ROM_SKIP(1) | ROM_BIOS(10) )
	// 11: BIOS-String: DG22-6080-091991-K0 / 286-BIOS (C) 1989 American Megatrends Inc / (C) 1991 TriGem Corporation
	ROM_SYSTEM_BIOS(11, "tg286m", "TG286M")
	ROMX_LOAD( "ami.bin", 0x10000, 0x10000, CRC(5751542a) SHA1(0fa0e86a0599e8500bb9ca9efa77cfc9c82a9dc0), ROM_BIOS(11))
	// 12: BIOS-String: 286 Modular BIOS Version B3.11.03 - REFRESH TIMING ERROR
	ROM_SYSTEM_BIOS(12, "cl286", "CL286")
	ROMX_LOAD( "award.bin", 0x10000, 0x10000, CRC(839a30b3) SHA1(c40a15c2636cf734e83ddf22213f637766f6456e), ROM_BIOS(12))
	// 13: BIOS-String: 286 Modular BIOS Version 3.03 Copyright Award Software Inc. - DIGICOM
	// ID: Digicom DIGIS 286S Turbo8/10MHz - Chipset: Chips P82C201-10, P82A205, P82A204, P82C202, NS16450N, HCT612, MC146818P, 2xNEC D8237AC-5
	// FPU socket provided - RAM: 1MB (4x9xTMS4256-12) - BIOS: Award 286 BIOS - OSC: 20.000MHz, 16000.00KHz, 1.8432MHz, 14.31818 - ISA8: 3, ISA16: 4
	// Keyboard BIOS: AWARD
	ROM_SYSTEM_BIOS(13, "digis286s", "DIGICOM DIGIS 286S")
	ROMX_LOAD( "80286-286s_turbo_lo.bin", 0x10000, 0x8000, CRC(7ecc1082) SHA1(eb5231e169ab550749c44383da20ab049cdf2a6d), ROM_SKIP(1) | ROM_BIOS(13) )
	ROMX_LOAD( "80286-286s_turbo_hi.bin", 0x10001, 0x8000, CRC(bea8047e) SHA1(17eb6ab8dbc61e372acdda060b84bc4914980322), ROM_SKIP(1) | ROM_BIOS(13) )
ROM_END


//**************************************************************************
//  80286 motherboard
//**************************************************************************

// ID: Peacock computer S-286 Rev A - CPU: 80286, FPU socket provided - Chipset: TI TACT82204FN, STK5134, STK-5136, STK-5132, STK-5133
// RAM: no onboard RAM, ISA8 socket marked "RAM-1" and "RAM-4", matching card "RAM-1A" with 1MB DIP - BIOS: Award 286 BIOS Q2093374
// Keyboard BIOS: Award Q2093198 - OSC: "clock II F22/F28", "clock II F19/F29" - ISA8: 3, ISA16: 4 - DIP8: 01101000
ROM_START( peas286 ) // BIOS-String: 286 Modular BIOS Version - REFRESH TIMING ERROR
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "286-s-286-peacock-lo.bin", 0x10000, 0x08000, CRC(78efd1d5) SHA1(3b6c5c6e77a3f77de1c81c0936d7fb2eab7b2487), ROM_SKIP(1) )
	ROMX_LOAD( "286-s-286-peacock-hi.bin", 0x10001, 0x08000, CRC(5b8ba43a) SHA1(a38ebb1d21f051065ede45643ed394cc5ac1dbf2), ROM_SKIP(1) )
ROM_END

// ID: PC CHIPS M209 - Chipset: UMC UM82C206L, PC 4L50F2052 - BIOS: AMI 286 BIOS 162020 CDTEK - Keyboard BIOS: AMI KEY BOARD BIOS 162020 CDTEK
// CPU:AMD N80L286-16/S - RAM: SIPP30x4, 1MB DIP - BIOS-ID: S286-6181-101590-K0 - ISA8: 1, ISA16: 6 - OSC: 33.333MHz, 14.31818
ROM_START( pccm209 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "pcchips_m209_gq4x.bin", 0x10000, 0x08000, CRC(d1a68208) SHA1(58285d293f723507a5401e55c5e4e5d27681d824))
	ROM_CONTINUE( 0x10001, 0x08000 )
ROM_END

// ID: unknown - ASI 100B0, identified as “HAM 12 TI 286 Motherboard ZERO WAIT” - Chipset: Texas Instruments TACT82301PB, TACT82302PB, TACT82303PB (cf. at386sx)
// CPU: xxx, FPU: IIT 2C87-12 - RAM: 1MB in DIP, 4xSIPP30 - OSC: 24.000MHz, 14.31818 - ISA8: 3, ISA16: 5
ROM_START( asi100b0 ) // BIOS-String: D286-1112-040990-K0
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "asi286_signetics_s-27c256-20fa-l.bin", 0x10000, 0x08000, CRC(54505e72) SHA1(024edd1b435252db38274626c84904422cdb8787), ROM_SKIP(1) )
	ROMX_LOAD( "asi286_signetics_s-27c256-20fa-h.bin", 0x10001, 0x08000, CRC(9aff417a) SHA1(f74da97c797b0856ee6ff634c40eee6403416e4c), ROM_SKIP(1) )
ROM_END

// ID: Wearnes CL286-12/16S (CL286-12S and CL286-16S) - Chipset: Texas Instruments TACT82206FN / Micrel MIC9212CP, MIC9211CP, MIC9215CP, WD16C785-JT
// BIOS/Version: AMI 080190 - Keyboard BIOS: KB-BIOS-VER-F - Rom Type : NMC27C256Q x 2, CL286-12S 080190 EVEN CS8A11, CL286-12S 080190 ODD CSD77D
// CPU: AMD N80L286-12/S, FPU socket provided - RAM: 1MB (8xKM44C256AP-8), 4xSIMM30 - ISA8: 1, ISA18: 5 - On board: Floppy, IDE, par, 2xser
ROM_START( cl28612s ) // dies after initialising the graphics card
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "cl286-12s16s_even.bin", 0x10000, 0x08000, CRC(0e8d1b02) SHA1(43339afd2ce0acc38074359b81629658cc6936f6), ROM_SKIP(1) )
	ROMX_LOAD( "cl286-12s16s_odd.bin", 0x10001, 0x08000, CRC(09e35644) SHA1(cc5ca52cbf0b5fe4c315ce725715f759e6ca4f63), ROM_SKIP(1) )
ROM_END

// TD60C - chipset: CITYGATE D90-272 - BIOS: AMI 286 BIOS, EE265746 - Keyboard-BIOS: JETkey V3.0
// BIOS-String: 30-0101-429999-00101111-050591-D90-0 / TD60C BIOS VERSION 2.42B - ISA16: 6 - CPU: CS80C286, FPU: i287XL
ROM_START( td60c )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "2cgm001.bin", 0x10000, 0x8000, CRC(35e4898b) SHA1(7ef8e097e010ec8dff9e33c4b42a278ff736059c))
	ROM_CONTINUE( 0x10001, 0x8000 )
ROM_END

// BIOS-String: Phoenix 80286 ROM BIOS PLUS Version 3.10.01 - CPU: AMD N80L286-16/S / FPU: socket provided - RAM: 640KB DIP / 4xSIPP30
// Chipset: SUNTAC ST62??? ST62C303-A - BIOS: Phoenix - Keyboard-BIOS: NEC D8041AHC - ISA16: 4 - ISA8: 2 - OSC: [unreadable] - 32.000 MHz
ROM_START( suntac303 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "286-suntac-2055712.bin", 0x10000, 0x8000, CRC(407b89d8) SHA1(d419bdd8bfb6191c68254204efdd756c5131701c))
	ROM_CONTINUE( 0x10001, 0x8000 )
ROM_END

// Olivetti M203 motherboard - complains about "Timer Sync Error"
// on board Paradise PVGA1A VGA chip - Chipset: 2 TACT chips, one VLSI chip - one 16bit ISA "slot" in pin strip form intended for an expansion module
ROM_START( olim203 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "286-olivetti-m203-low.bin", 0x00000, 0x10000, CRC(d683dc20) SHA1(04271529139724d7a091490658b186b59a83676f), ROM_SKIP(1) )
	ROMX_LOAD( "286-olivetti-m203-high.bin", 0x00001, 0x10000, CRC(c7324ecf) SHA1(fa5ee92c21e54ec711d01b211760521a71ef424d), ROM_SKIP(1) )
ROM_END

// Snobol Mini 286 - BIOS-String: DGS2-1402-101090-K0
// Chipset: GST GOLD GS62C101 and GS62C102
ROM_START( snomi286 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "gst_286.bin", 0x10000, 0x8000, CRC(89db769b) SHA1(3996856d637dc379978c0b5eb79362f46b60a80f) )
	ROM_CONTINUE( 0x10001, 0x8000)
ROM_END

// PC-Chips M205 - Chipset: PCChips 4L50F2052 aka PCCHIP1 - ISA8: 3, ISA16: 5
// the 64K ROM has first the 32K even, then the 32K odd part
// BIOS-String: S286-6181-101590-K0
ROM_START( pccm205 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: v2.25, 17 January, 1991 (C) 1990 PC Chips INC.
	ROM_SYSTEM_BIOS(0, "v225", "PC Chips v2.25")
	ROMX_LOAD( "m205.bin", 0x10000, 0x8000, CRC(6f7bc8d6) SHA1(14062505b316e0d4409fb4e502651e09fea0a4c1), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_CONTINUE( 0x10001, 0x8000)
	// 1: v2.23, 29 December, 1990 (C) 1990 EVER-SUCCESS SYSTEMS LTD.
	ROM_SYSTEM_BIOS(1, "v223", "Ever-Success Systems v2.23")
	ROMX_LOAD( "m205_odd.bin", 0x10000, 0x8000, CRC(3a8ed558) SHA1(0648a2f1674bb07a175b983f107bb72c38ac1e61), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_CONTINUE( 0x10001, 0x8000)
ROM_END

// PC-Chips M216 REV 1.2 - Chipset PC CHIPS CHIP 3 - CPU: Harris CS80C286-20, IIT 2C87-10
// BIOS: AMI ; 07/07/91; S/NO. 0245157 - ISA16: 6 - BIOS-String: 30-0000-ZZ1437-00101111-070791-PC CHIPS-8
ROM_START( pccm216 ) // no display
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "2pcm001.bin", 0x10000, 0x10000, CRC(9c7c9f05) SHA1(39cb6645d9aab846f7e64d1d44610ea3cbe52581))
ROM_END

// Unknown 80C286 motherboard (RAM: 4xSIMM30, 1MB DIP, ISA16: 6) - CPU: Harris CS80C286-16, FPU: 80287 - OSC: 32.000MHz, 14.31818
// SARC RC2015; HM6818P; 82C042 or JETkey Keyboard BIOS; 1MB onboard RAM (8x LH64256AD-80)
ROM_START( sarcpc )
	ROM_REGION16_LE(0x20000, "bios", 0) // 27C512
	// BIOS-String: 20-0300-00834-00101111-050591-SARC286 / [80286 Standard System 2V1]
	//ROM_SYSTEM_BIOS(0, "sarcrev12", "SARC Rev. 1.2")
	ROM_LOAD( "sarcrev12.bin", 0x10000, 0x10000, CRC(1c5e3f2d) SHA1(1fcc8b1b9d9383467223dd41e420f9352beca654) )
ROM_END

// Everex EV-1806 (6 16-bit ISA, 1 8-bit ISA) - OSC: 14.31818MHz, 24.000MHz, 30.000MHz - RAM: 4 banks of 9xKM41C256P-8, sockets for 1MBit chips provided
// Everex IC-00121-0 + IC-00122-0; CHIPS P82C206; Intel 8272A(?); 146818A RTC
ROM_START( ev1806 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "everex_ev-1806_rev-f1a-21_even_u62.bin", 0x18000, 0x4000, CRC(7364e49b) SHA1(e8f5f41514005da0e36792e009cf3eae51c19c20), ROM_SKIP(1) )
	ROMX_LOAD( "everex_ev-1806_rev-f1a-21_odd_u61.bin", 0x18001, 0x4000, CRC(05c87bf7) SHA1(8c2243d9ee3d2af1517dc1134a22a7d1ed11262f), ROM_SKIP(1) )
ROM_END


// MAT286 REV.D (5 16-bit ISA, 1 8-bit ISA, RAM: DIP 1MB, 2xSIPP30) - CPU: Siemens SAB 80286-16-N - OSC: 32.000MHz, 14.31818
// Headland Technology HT12P-16/A; HM6818P RTC; JETkey keyboard BIOS; unmarked 40-pin DIP (prob. 80287)
ROM_START( mat286 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// BIOS-String: DH12-1112-061390-K0 - HT-12 286 BIOS - Board is also used in Polish Optimus 286 computer with a special Hercules character ROM (also available)
	// Files separated from single BIOS64 dump (PCB photo shows split ROMs are used)
	ROMX_LOAD( "9221fkf_imp23256_ami-l.bin", 0x10000, 0x08000, CRC(55deb5c2) SHA1(19ce1a7cc985b5895c585e39211475de2e3b0dd1), ROM_SKIP(1) )
	ROMX_LOAD( "9221gjf_imp23256_ami-h.bin", 0x10001, 0x08000, CRC(04a2cec4) SHA1(564d37a8b2c0f4d0e23cd1e280a09d47c9945da8), ROM_SKIP(1) )
ROM_END

ROM_START( ec1842 )
	ROM_REGION16_LE(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "4202004.bin", 0x1c001, 0x2000, CRC(33fb5382) SHA1(35eb62328324d93e7a06f2f9d1ad0002f83fc99b))
	ROM_LOAD16_BYTE( "4202005.bin", 0x1c000, 0x2000, CRC(8e05c119) SHA1(9d81613b4fc305c14ae9fda0b1dd97a290715530))
	ROM_LOAD16_BYTE( "4202006.bin", 0x18001, 0x2000, CRC(6da537ef) SHA1(f79feb433dcf41f5cdef52b845e3550d5f0fb5c0))
	ROM_LOAD16_BYTE( "4202007.bin", 0x18000, 0x2000, CRC(d6ee0e95) SHA1(6fd4c42190e879501198fede70ae43bc420681d0))
	// EGA ROM
	//ROM_LOAD16_BYTE( "4200009.bin", 0xc0000, 0x2000, CRC(9deeb39f) SHA1(255b859d3ea05891aa65a4a742ecaba744dfc923))
	//ROM_LOAD16_BYTE( "4200010.bin", 0xc0001, 0x2000, CRC(f2c38d93) SHA1(dcb3741d06089bf1a80cb766a6b94029ad698d73))
ROM_END

ROM_START( ec1849 )
	ROM_REGION16_LE(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "cpu-card_27c256_015.rom", 0x10000, 0x8000, CRC(68eadf0a) SHA1(903a7f1c3ebc6b27c31b512b2908c483608b5c13))
	ROM_LOAD16_BYTE( "cpu-card_27c256_016.rom", 0x10001, 0x8000, CRC(bc3924d6) SHA1(596be415e6c2bc4ff30a187f146664531565712c))
	//ROM_LOAD16_BYTE( "video-card_573rf6(2764)_040.rom", 0xc0001, 0x2000, CRC(a3ece315) SHA1(e800e11c3b1b6fcaf41bfb7d4058a9d34fdd2b3f))
	//ROM_LOAD16_BYTE( "video-card_573rf6(2764)_041.rom", 0xc0000, 0x2000, CRC(b0a2ba7f) SHA1(c8160e8bc97cd391558f1dddd3fd3ec4a19d030c))
ROM_END

// Morse KP-286
// BIOS-String: DS24-1216-061390-K0
// Chipset: SUNTAC ST62C211 and ST62C203-A
ROM_START ( mkp286 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "286-morse_kp286_lo.bin", 0x10000, 0x8000, CRC(0d35d2c9) SHA1(52b366608ea25a96d8e27c5d77689688fff38609), ROM_SKIP(1) )
	ROMX_LOAD( "286-morse_kp286_hi.bin", 0x10001, 0x8000, CRC(a5f640e0) SHA1(7bbb7fce54079005cb691816d2301a3eda475a82), ROM_SKIP(1) )
ROM_END


// WYSEpc 286 - motherboard: WY-2200-01 - continuous ticks from the speaker
ROM_START( wy220001 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "wyse_tech_rev.a_250325-02_27128-15.bin", 0x18000, 0x4000, CRC(010f1c4d) SHA1(712d6ca4e4bdbc6b105c8691d612407edcfd9cf7), ROM_SKIP(1))
	ROMX_LOAD( "wyse_tech_rev.a_250326-02_27128-15.bin", 0x18001, 0x4000, CRC(37fcd62b) SHA1(ada0e232387c8ba7067168f50f8b7a89eb824c44), ROM_SKIP(1))
ROM_END


// ***** 286 mainboards using the Chips & Technologies CS8221 NEAT chipset: P82C211 + P82C212 + P82C215 + P82C206

ROM_START( neat )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: BIOS-String: ENET-1131-030389-K0
	ROM_SYSTEM_BIOS(0, "neat286", "NEAT 286")
	ROMX_LOAD( "at030389.0", 0x10000, 0x8000, CRC(4c36e61d) SHA1(094e8d5e6819889163cb22a2cf559186de782582),ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "at030389.1", 0x10001, 0x8000, CRC(4e90f294) SHA1(18c21fd8d7e959e2292a9afbbaf78310f9cad12f),ROM_SKIP(1) | ROM_BIOS(0))
	// 1: Phoenix 80286 ROM BIOS PLUS Version 3.10 12 - High Performance 286 ROM BIOS Ver C.12
	ROM_SYSTEM_BIOS(1, "pb800", "Packard Bell PB800")
	ROMX_LOAD( "3.10.12-1.bin", 0x10001, 0x8000, CRC(e6bb54c5) SHA1(fa5a376dd44696c78dcc8994e18938b5e1b3e45a),ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "3.10.12-2.bin", 0x10000, 0x8000, CRC(bde46933) SHA1(c7221192f48d6f2f5b773c3c7d2a52b635cb473e),ROM_SKIP(1) | ROM_BIOS(1))
	// 2: DTK Corp. 286 Computer - DTK 286 Chipset ROM BIOS Version 3.26 - #24062890N - ISA8: 3, ISA16: 5, RAM: 1MB DIP, 4xSIMM30
	ROM_SYSTEM_BIOS(2, "ptm1632c", "UNIT PTM1632C DTK V.3.26")
	ROMX_LOAD( "ptm1632c_l.bin", 0x10000, 0x8000, CRC(df0bc27c) SHA1(f94e2decd13c285c23b6a61c035cab88fa00ba6e), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "ptm1632c_h.bin", 0x10001, 0x8000, CRC(a80136e0) SHA1(5edc2d387efb42cf70361197de808ce1b06d8aec), ROM_SKIP(1) | ROM_BIOS(2))
	// 3: BIOS-String: DTK Corp. 286 COMPUTER - (C) DTK NEAT BIOS Ver 3.25N2 06/06/89 - DTK PTM-1233C - Chipset: P82C211; P82C212B; P82C215 - BIOS: dtk 286E 8864 - IS8: 3 - ISA16: 5
	ROM_SYSTEM_BIOS(3, "ptm1233c", "DTK PTM-1233C")
	ROMX_LOAD( "286-dtk ptm-1233c-low_32k.bin", 0x10000, 0x8000, CRC(8909164c) SHA1(51978929a690746c1956ca6b1f0412777dc5d35b), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "286-dtk ptm-1233c-high_32k.bin", 0x10001, 0x8000, CRC(9105968c) SHA1(737d4df8040655315a648fed8a8d574f39e7dc35), ROM_SKIP(1) | ROM_BIOS(3) )
	// 4: 286-NEAT - BIOS-String: ENET-1131-040990-K0 - NEAT V3.2 6-18-90 - ISA8: 3, ISA16: 5
	ROM_SYSTEM_BIOS(4, "286neat", "286-NEAT")
	ROMX_LOAD( "286-neat_neat012-l-verify.bin", 0x10000, 0x8000, CRC(591d226c) SHA1(7f42797ead8213022192bb2bbbe2de7f6796ac6f), ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "286-neat_neat012-h-verify.bin", 0x10001, 0x8000, CRC(0198e2e4) SHA1(10ced383b6dc00c2e98b7bed0782f59a9c266625), ROM_SKIP(1) | ROM_BIOS(4) )
	// 5: AUVA VIP BAM/16-11 - BIOS-String: Phoenix 80286 ROM BIOS PLUS Version 3.10 20 - ISA8:1, ISA16: 5, Memory Slot: 1
	ROM_SYSTEM_BIOS(5, "bam1611", "VIP BAM/16-11") // OSC: 18.432 - 14.318 - 32.000MHz
	ROMX_LOAD( "286-vip bam-6-11 m215100-lo_32k.bin", 0x10000, 0x8000, CRC(b51b8bc1) SHA1(a7ebbced98aca32a7f0cdf80d1b832dfeb92d5e7), ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "286-vip bam-6-11 m215100-hi_32k.bin", 0x10001, 0x8000, CRC(46ddd5a6) SHA1(fd4267af298c7f70e062a7c4e023caf852bbf082), ROM_SKIP(1) | ROM_BIOS(5) )
	// 6: CP-805 - BIOS-String: ENET-1138-030390-K0
	ROM_SYSTEM_BIOS(6, "cp805", "CP-805")
	ROMX_LOAD( "286-chips ami78384 even.bin", 0x10000, 0x8000, CRC(5280fee0) SHA1(25051ad6bbccddc0738861b614dbafbca5c3bff5), ROM_SKIP(1) | ROM_BIOS(6) )
	ROMX_LOAD( "286-chips ami78384 odd.bin", 0x10001, 0x8000, CRC(24526bf3) SHA1(8f8b46fe2e708fa53d0eeb44a16924cd878bdd33), ROM_SKIP(1) | ROM_BIOS(6) )
	// 7: BIOS-String: ENET-1107-040990-K0
	ROM_SYSTEM_BIOS(7, "ami211", "AMI 21.1") /*(Motherboard Manufacturer: Dataexpert Corp. Motherboard) (Neat 286 BIOS, 82c21x Chipset ) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "ami211.bin",     0x10000, 0x10000,CRC(a0b5d269) SHA1(44db8227d35a09e39b93ed944f85dcddb0dd0d39), ROM_BIOS(7))
	// 8: BIOS-String: ENET-1230-043089-K0
	ROM_SYSTEM_BIOS(8, "amic21", "AMI C 21.1") /* (Motherboard Manufacturer: Unknown.) (Neat 286 BIOS, 82c21x Chipset ) (BIOS release date:: 30-04-1989) */
	ROMX_LOAD( "amic21-2.bin",  0x10001, 0x8000, CRC(8ffe7752) SHA1(68215f07a170ee7bdcb3e52b370d470af1741f7e),ROM_SKIP(1) | ROM_BIOS(8) )
	ROMX_LOAD( "amic21-1.bin",  0x10000, 0x8000, CRC(a76497f6) SHA1(91b47d86967426945b2916cb40e76a8da2d31d54),ROM_SKIP(1) | ROM_BIOS(8) )
	// 9: BIOS-String: - AGC N286 - CPU: AMD N80L286-16/S, FPU socket provided - RAM: 36xTMC64C1024-80N (18pin), 36x16pin sockets provided as an alternative (2xBank 0, 2xBank 1), 4xSIPP30 (2xBank 0/2 and 2xBank 1/3)
	// Chipset: Chips P82C212B-12, P82C206 H, P82C211-12 C, P82C215-12 - OSC: 20.000, 32.000MHz, 24.0000MHz, 32.768KHz, 14.31818 - BIOS: AMI 286 BIOS PLUS CO.NO.1190
	// Keyboard-BIOS: AMI KEYBOARD BIOS PLUS CO.NO. 1190 - ISA8: 2, ISA16: 6
	ROM_SYSTEM_BIOS(9, "n286", "N286")// stops after initialising the graphics card
	ROMX_LOAD( "286-chips-ami1190-even_32k.bin", 0x10000, 0x8000, CRC(db941036) SHA1(994cced82b5fb5f8833c718b4226a7e9620b56df),ROM_SKIP(1) | ROM_BIOS(9) )
	ROMX_LOAD( "286-chips-ami1190-odd_32k.bin", 0x10001, 0x8000, CRC(71cfc2d1) SHA1(8b8cf81161aec3e2c7f653e5d3a6b4e9627663c6),ROM_SKIP(1) | ROM_BIOS(9) )
ROM_END

// Chaintech ELT-286B-160B(E) mainboards - NEAT chipset: Chips P82C206, P82C211C, P82C212B, P82C215
ROM_START( elt286b )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: BIOS/Version: AWARD A2133130/21323132 - BIOS-String: 286 Modular BIOS Version 0N3.03 NFS / ELT
	// Keyboard-BIOS: AWARD A21266586 - OSC: 24.000MHz, 12.000MHz, 20.000MHz, 14(... unreadable) - ISA8: 2, ISA16: 5
	ROM_SYSTEM_BIOS(0, "160b", "ELT-286B-160B")
	ROMX_LOAD( "286-elt-286b-160b_l_32k.bin", 0x10000, 0x8000, CRC(4514a284) SHA1(0f9d4a24bdd0fb6aa15c7c1db860c4e6df632091), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "286-elt-286b-160b_h_32k.bin", 0x10001, 0x8000, CRC(109bbf7c) SHA1(88b6b1c7c08739f8b198f05adbe6edc24be35fd0), ROM_SKIP(1) | ROM_BIOS(0) )
	// 1: BIOS/Version: Phoenix 2061618  BIOS-String: Phoenix 80286 ROM BIOS PLUS Version 3.10 20 / Phoenix C&T 8221 NEAT Dual Mode BIOS / ELT
	// Keyboard-BIOS: Phoenix/Intel i8242 - ISA8: 2, ISA16: 5 - OSC: 32.000MHz, 24.000MHz, 14.31818MHz
	ROM_SYSTEM_BIOS(1, "160eb", "ELT-286B-160BE")
	ROMX_LOAD( "286-2061618 l_32k.bin", 0x10000, 0x8000, CRC(f89aabc4) SHA1(94472edc9692b9da6450fb12994d62230c8cc5c5), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "286-2061618 h_32k.bin", 0x10001, 0x8000, CRC(e23a60bf) SHA1(48af3f123d30cd2fde9e42f2c9a57eec143287b6), ROM_SKIP(1) | ROM_BIOS(1) )
ROM_END

// Advanced Logic Research PWB 7270 REV E - Chipset: Chips P82C212B-12 A, P82C211-12 C, P82C215 A 16 MHz, P62C206 H1, UMC xxx, DP8473V
// CPU: AMD N80L286-16/S, FPU: Intel D80287, Conntector for 386/486 FEATURE - RAM: 1MB (8x514256), 2xSIMM30 - OSC: 32.000MHz, 24.0000MHz, 1.8432MHz, 14.31818 MHz
// BIOS: POWER FLEX+ 3.10.09 ODD  CS0F00, POWER FLEX+ 3.10.09 EVEN CS8A00 - Keyboard BIOS: M5L8042 - On board: IDE, keyboard, 2x25pin (ser/par?) - ISA16: 5
// BIOS-String: Phoenix ROM BIOS PLUS Version 3.10 09 - Advanced Logic Research, Inc. PowerFlex PLUS 286/386sx
// Timer chip counter 2 failed, Keyboard failure
ROM_START( pwb7270e )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "alreven.bin", 0x10000, 0x08000, CRC(b6428bae) SHA1(4e6e6eec67ff62cf8b4a3ce500bd15a54ee3d5fe), ROM_SKIP(1) )
	ROMX_LOAD( "alrodd.bin", 0x10001, 0x08000, CRC(6eedbcf0) SHA1(58173b6f749d40aa294747823d4b442c8938710e), ROM_SKIP(1) )
ROM_END


// ***** 286 motherboards using the Acer (ALi) M1207 chipset

// CMP enterprise CO.LTD. Phoenix 80286 ROM BIOS Version 3.00
// ROM_SYSTEM_BIOS(26, "cmpa286", "CMP A286") /* (Chipset Acer (ALi) M1207-12) (BIOS release date:: 01-09-1986) (ISA8: 2, ISA16: 6) */
ROM_START ( cmpa286 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "286-a286-even_16k.bin", 0x18000, 0x4000, CRC(30809487) SHA1(413de43ca7e1930cdf3c006718d8baf743a9ff1e), ROM_SKIP(1) )
	ROMX_LOAD( "286-a286-odd_16k.bin", 0x18001, 0x4000, CRC(3a11aacf) SHA1(23185531ae10912b974048d3607b563e55d3fa96), ROM_SKIP(1) )
ROM_END

// AUVA VIP-M21502A BAM16-A0 - BIOS-String: DAR2-1105-061390-K0 - 286-BIOS AMI for AUVA 286, 02/08/1991 - ISA8:2, ISA16:5
ROM_START( bam16a0 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "286-vip-m21502a-lo_32k.bin", 0x10000, 0x8000, CRC(413692db) SHA1(54bf664526b137cabf974c1fc659493e76243a88), ROM_SKIP(1) )
	ROMX_LOAD( "286-vip-m21502a-hi_32k.bin", 0x10001, 0x8000, CRC(5db9db04) SHA1(8085384b943454a708be3104b47f6793d0040ab1), ROM_SKIP(1) )
ROM_END


// ***** 286 motherboards using the Chips SCAT 82C235 chipset

// Biostar MB-1212C - ISA8:2, ISA16:5
ROM_START ( mb1212c )
	// 0: BIOS-String: ESC2-1223-083090-K2 - 286 BIOS AMI for MB-1212C version 1.1
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "mb1212_1", "mb1212_1")
	ROMX_LOAD( "biostar mb-1212c.bin", 0x10000, 0x10000, CRC(153a783a) SHA1(acad4a3ffe93d3884dcb743c32d6317a132cda7b), ROM_BIOS(0) )
	// 1: CHIPS SCAT BIOS Version 125D - MB-1212C
	ROM_SYSTEM_BIOS(1, "mb1212_2", "mb1212_2")
	ROMX_LOAD( "mb-1212c.bin", 0x10000, 0x10000, CRC(4675530a) SHA1(c34b1c67ac29695e565363f484e17ab5f8ddaad5), ROM_BIOS(1) )
ROM_END


// ***** 286 motherboards using the Headland G2 chipset

// LM-103S - 1 8-bit ISA, 6 16-bit ISA - RAM: 4xSIPP30, 2 banks DIP (each bank has 4xV53C104AP80 and 2x16pin empty sockets)
// Headland Technology G2 chipset: HT101A + 2x HT102; HM6818P RTC; AMI keyboard BIOS 904189, BIOS AMI 904189
// BIOS-String: D286-1234-121589-K0 - CPU: AMD N80L286-16/S - OSC: 32.000MHz, 14.31818MHz
ROM_START( lm103s )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "ami_lm103-s_lo.bin", 0x10000, 0x8000, CRC(a24be20b) SHA1(ffc5faf6d773154bf2f037556d2e381e81a28a58), ROM_SKIP(1) )
	ROMX_LOAD( "ami_lm103-s_hi.bin", 0x10001, 0x8000, CRC(7b63e60c) SHA1(da78b95b12051b6d4701a412fdc5e7874595c188), ROM_SKIP(1) )
ROM_END

// CDTEK board with Headland G2 chipset - ISA8:1, ISA16:5
ROM_START ( cdtekg2 ) // BIOS-String: D286-1435-040990-K0 - Board is used in Polish California Access 286 with a special Hercules character ROM (also available)
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "286_cdtek headland-l_32k.bin", 0x10000, 0x8000, CRC(341fe2a3) SHA1(f8e10aea477c2b3c92b28a7e0fd0adf8ade22b9e), ROM_SKIP(1) )
	ROMX_LOAD( "286_cdtek headland-h_32k.bin", 0x10001, 0x8000, CRC(bd6fd54f) SHA1(72500ebe4041fbe635562bf55c5d3635257e38f1), ROM_SKIP(1) )
ROM_END

// Octek board with Headland G2 chipset - ISA8:2, ISA16:6
//BIOS-String: 286 Modular BIOS Version 3.03 - O.O.A.L.
ROM_START ( octekg2 ) // BIOS-String: D286-1435-040990-K0
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "286-octek-g2_u44_32k.bin", 0x10000, 0x8000, CRC(05892a08) SHA1(e12795524d87c422b0b5d660b36139592893e9c6), ROM_SKIP(1) )
	ROMX_LOAD( "286-octek-g2_u45_32k.bin", 0x10001, 0x8000, CRC(2f81de14) SHA1(952d9e35a6f8ea74eb8b4bf7ea80d7c358474cb8), ROM_SKIP(1) )
ROM_END

// Octek Fox M 286 Rev 1.1 - Chipset: Headland HT101A/2xHT102 - CPU: AMD N80L286-16/S, FPU socket provided - RAM: 514256x8 (4 empty tag sockets provided), 4xSIMM30
// BIOS: AMI 286 BIOS 212491, Keyboard BIOS: AMI Keyboard BIOS setup -  OSC: 32.000MHz, 14.31818 - ISA8: 2, ISA16: 4 - undumped PAL: PAL 090-103, 089-001, 090-011
// BIOS-String: D286-6069-040990-K0 - BIOS release date:: 09-04-1990
ROM_START( ocfoxm )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "ami_286_bios_sn200960_even.bin", 0x10000, 0x8000, CRC(67745815) SHA1(ca6886c7a0716a92a8720fc71ff2d95328c467a5), ROM_SKIP(1) )
	ROMX_LOAD( "ami_286_bios_sn200960_odd.bin", 0x10001, 0x8000, CRC(360a5f73) SHA1(1b1980fd99779d0cdc4764928a641e081b35ee9f), ROM_SKIP(1) )
ROM_END

ROM_START( headg2 )
	ROM_REGION16_LE( 0x20000, "bios", 0)
	// 0: 286 board with Headland GC101A-PC; GC102-PC chipset and Phoenix BIOS 2493119, ISA8: 2, ISA16: 5
	ROM_SYSTEM_BIOS(0, "head_ph_1", "Headland/Phoenix #1") // Phoenix 80286 ROM BIOS PLUS Version 3.10.21 ((BIOS release date:: 15-01-1988)
	ROMX_LOAD( "286-headland-lo_32k.bin", 0x10000, 0x8000, CRC(21b68bed) SHA1(1e4acda50b12ad463c169ba615805f5dcf257b18), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "286-headland-hi_32k.bin", 0x10001, 0x8000, CRC(04c8ab12) SHA1(b46c14528aca15464e4050b423c2f621a4313a85), ROM_SKIP(1) | ROM_BIOS(0) )
	// 1: BIOS-String: 286 Modular BIOS Version 3.03HL - ISA16: 5
	ROM_SYSTEM_BIOS(1, "head4530", "Headland 4530")
	ROMX_LOAD( "286-headland 4530-high_32k.bin", 0x10001, 0x8000, CRC(f84c0e75) SHA1(42dc068d1cd5105cd576b023e2ccfe0f0646d4e3), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "286-headland 4530-low_32k.bin", 0x10000, 0x8000, CRC(0856dde8) SHA1(cee5d6002c405df984f3c7fa83c4f3e034f1e586), ROM_SKIP(1) | ROM_BIOS(1) )
	// 2: Quadtel Enhanced 286 BIOS Version 3.04.02 - Headland HT101, HT102
	ROM_SYSTEM_BIOS(2, "ami101", "AMI HT 101.1") /* (Quadtel Enhanced 286 BIOS Version 3.04.02) (BIOS release date:: 09/11/1989) */
	ROMX_LOAD( "amiht-h.bin",   0x10001, 0x8000, CRC(8022545f) SHA1(42541d4392ad00b0e064b3a8ccf2786d875c7c19),ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "amiht-l.bin",   0x10000, 0x8000, CRC(285f6b8f) SHA1(2fce4ec53b68c9a7580858e16c926dc907820872),ROM_SKIP(1) | ROM_BIOS(2) )
ROM_END


// ***** 286 motherboards using the Headland HT12/A chipset

// Octek Fox II - Chipset: Headland HT12/A - BIOS String: DH1X-6069-113090-K0 - HT-1X 286 BIOS
ROM_START( o286foxii )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "286-fox2-even_32k.bin", 0x10000, 0x8000, CRC(54dc119f) SHA1(4bc543beef0d2201fa20eac90a0a6ca38ebf0dbf), ROM_SKIP(1))
	ROMX_LOAD( "286-fox2-odd_32k.bin", 0x10001, 0x8000, CRC(e5db7775) SHA1(2bd0572b9f7c76eff51375b551586ca8484e2a74), ROM_SKIP(1))
ROM_END

// BI-025C HT12 286 - Chipset: Headland HT12/A - BIOS-String: DH12-1103-061390-K0 - ISA8: 2, ISA16: 5
ROM_START( bi025c )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "bi-025c-ht12_even.bin", 0x10000, 0x8000, CRC(7ea7e088) SHA1(e245b3ecce39e85cacb17abf60d2cee000d1750d), ROM_SKIP(1))
	ROMX_LOAD( "bi-025c-ht12_odd.bin", 0x10001, 0x8000, CRC(f18b3eef) SHA1(e14d4b3ea0234613e60512cf79e5580c9ce7f3f6), ROM_SKIP(1))
ROM_END

ROM_START( ht12a )
	ROM_REGION16_LE(0x20000, "bios", 0)
	//0: BIOS-String: DH12-1343-061390-K0
	// Original BIOS64 dump split into even and odd bytes (matches another dump of the same BIOS)
	// ROM at U6 has sticker with AMI 253770 label; "BB012" at U8 is probably other half of BIOS, though not clear which half is which
	// Unknown motherboard (similar layout to LM-103S; 4 SIMM, 5 16-bit ISA, 2 8-bit ISA)
	// Headland HT12/A; HM6818A RTC; AMI K053770 keyboard BIOS
	// Jumpers at right edge of board are labeled "KEYLOCK" (J6), "SPEAKER" (J7), "TURBO LED" (J8), "TURBO S.W." (J9), "RESET" (J10)
	// XTALs X3 and X4 in top right corner (behind 80C287) are both unpopulated
	ROM_SYSTEM_BIOS(0, "dh12-k0", "AMI DH12-K0")
	ROMX_LOAD( "286_headland_even.bin", 0x10000, 0x8000, CRC(a2530914) SHA1(1aca289240caa6d4bf811d301c338c157b6902a1), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "286_headland_odd.bin", 0x10001, 0x8000, CRC(b5f69002) SHA1(ee9ceef1fc7a328ee82006cd504e72e16f21b3c8), ROM_SKIP(1) | ROM_BIOS(0) )
	// 1: 286 board with Headland Headland HT12/A chipset, one ROM market IQS, Phoenix BIOS 3479808 - ISA8: 1, ISA16: 6
	ROM_SYSTEM_BIOS(1, "head_ph_2", "Headland/Phoenix #2") // Phoenix BIOS A286 Version 1.01 - BIOS ID JLI01101 - IT9109 - Reference ID 01 - (BIOS release date:: 19-04-1990)
	ROMX_LOAD( "286-headland-iqs-lo_32k.bin", 0x10000, 0x8000, CRC(60424e9d) SHA1(aa813bf48939fe7fcbbfec3133e702bfdff6234e), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "286-headland-iqs-hi_32k.bin", 0x10001, 0x8000, CRC(e56212e0) SHA1(2441845d632d19adc0592e094beb5ec1fbe074f6), ROM_SKIP(1) | ROM_BIOS(1) )
	// BIOS-String: Quadtel HT12 286 BIOS Versio 3.05.03
	// 2: Same board as #1, CPU: AMD N80L286-16S, FPU socket provided, Chipset: Headland HT12 - OSC: 14.31818, 8.000, 32.000MHZ, unpopulated: ASYN BUS CLK
	// RAM: 8xHYB514256B-60 (8x20pin, alternativelx 4x18pin), Parity (2x16pin, 2x18pin) empty, 4xSIMM30 (Bank0 SIMM, Bank1 SIMM) - BIOS: Quadtel BIOS Software 286 253893
	// Keyboard-BIOS: Quadtel BIOS Software KEY 316018 (undumped) - JP7: NO ASYN CLK, JP4: CMOS RAM CLEAR, JP3: COLOR - JP: 80287CLK=ASYN CLK
	ROM_SYSTEM_BIOS(2, "quadtel", "Quadtel")
	ROMX_LOAD( "bios-lo.bin", 0x10000, 0x8000, CRC(433d8044) SHA1(3435d51fad97247b4bcfdb2f3fdb358d99b0e6ea), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "bios-hi.bin", 0x10001, 0x8000, CRC(fe124da4) SHA1(b3e4e598cf1f5cada1b101d0c6434770017de3c6), ROM_SKIP(1) | ROM_BIOS(2) )
	// 3: BIOS-String: DH12-1164-083090-K0 - CPU/FPU: N80L286-16/S, P80C287-10 - ISA8: 2, ISA16: 6
	ROM_SYSTEM_BIOS(3, "head12a01", "Headland HT12/A #1")
	ROMX_LOAD( "2hlm002l.bin", 0x10000, 0x8000, CRC(345b9ea1) SHA1(868cc309e433e0dcc9f3aa147263017b7f822461), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "2hlm002h.bin", 0x10001, 0x8000, CRC(35eed8b8) SHA1(119f2676aef038301c3e0bcdb999da6fd740e6a5), ROM_SKIP(1) | ROM_BIOS(3) )
	// 4: MBL M21 - BIOS-String: DH12-1211-061390-K0 / HT-12 286 BIOS - Chipset: Headland HT12/A
	ROM_SYSTEM_BIOS(4, "ami121", "AMI HT 12.1") /* (BIOS release date:: 13-06-1990) */
	ROMX_LOAD( "ami2od86.bin", 0x10001, 0x8000, CRC(04a2cec4) SHA1(564d37a8b2c0f4d0e23cd1e280a09d47c9945da8),ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "ami2ev86.bin", 0x10000, 0x8000, CRC(55deb5c2) SHA1(19ce1a7cc985b5895c585e39211475de2e3b0dd1),ROM_SKIP(1) | ROM_BIOS(4) )
	// 5: SPEC 286 rev 4a - BIOS-String: DH12-1120-061390-K0
	ROM_SYSTEM_BIOS(5, "ami122", "AMI HT 12.2") /* (BIOS release date:: 13-06-1990) */
	ROMX_LOAD( "ami2ev89.bin", 0x10000, 0x8000, CRC(705d36e0) SHA1(0c9cfb71ced4587f109b9b6dfc2a9c92302fdb99),ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "ami2od89.bin", 0x10001, 0x8000, CRC(7c81bbe8) SHA1(a2c7eca586f6e2e76b9101191e080a1f1cb8b833),ROM_SKIP(1) | ROM_BIOS(5) )
	// 6: BIOS-String: DH12-1112-061390-K0
	ROM_SYSTEM_BIOS(6, "ami123", "AMI HT 12.3") /*(Motherboard Manufacturer: Aquarius Systems USA Inc.) (BIOS release date:: 13-06-1990)*/
	ROMX_LOAD( "ht12h.bin", 0x10001, 0x8000, CRC(db8b471e) SHA1(7b5fa1c131061fa7719247db3e282f6d30226778),ROM_SKIP(1) | ROM_BIOS(6) )
	ROMX_LOAD( "ht12l.bin", 0x10000, 0x8000, CRC(74fd178a) SHA1(97c8283e574abbed962b701f3e8091fb82823b80),ROM_SKIP(1) | ROM_BIOS(6) )
	// 7: ID: H286-C3 158 - Chipset: Headland HT12/A3A0050 - BIOS: AMI 286 BIOS SETUP 649963 - Keyboard BIOS: JET ELECTRONICS CO LTD SN 9 0922133
	// BIOS-String: DH12-1164-083090-K - CPU: AMD N80L286-12/S, FPU socket provided - RAM: SIMM30: 4, 1MB DIP - OSC: 8.000MHZ - ISA8: 1, ISA16: 5
	ROM_SYSTEM_BIOS(7, "h286c3", "H286-C3")
	ROMX_LOAD( "h286-c3-158-hi.bin", 0x10001, 0x8000, CRC(ec0cbbba) SHA1(97d7f32cb9c622cfbd08909ac67d09a8aa734430),ROM_SKIP(1) | ROM_BIOS(7) )
	ROMX_LOAD( "h286-c3-158-lo.bin", 0x10000, 0x8000, CRC(dedcf41a) SHA1(ede2c852b3e947ce4efa54640b2e37db83355a6c),ROM_SKIP(1) | ROM_BIOS(7) )
	// 8: CPU: Harris 286-16, FPU socket provided - Chipset: Headland HT12P-16/A - BIOS: AMIBIOS 03/15/91 - BIOS-String: 20-0000-428022-00101111-031591-HT12-F
	// RAM: 1MB DIP, 4xSIMM30 - ISA8: 1, ISA16: 6 - OSC: 32.000MHz, 14.31818
	ROM_SYSTEM_BIOS(8, "8022", "8022") // no display
	ROMX_LOAD( "20-0000-428022-00101111-031591-ht12 low.bin", 0x10000, 0x8000, CRC(cb74f8e3) SHA1(fc874787f960587ba37442d59af1beebcfd798b9),ROM_SKIP(1) | ROM_BIOS(8) )
	ROMX_LOAD( "h20-0000-428022-00101111-031591-ht12 high.bin", 0x10001, 0x8000, CRC(36179ed9) SHA1(25968319bcd35ff06a0f0edac6ff0246f3f79c25),ROM_SKIP(1) | ROM_BIOS(8) )
ROM_END


// ***** motherboards using the six chip SUNTAC chipset: ST62BC001-B, ST62BC002-B, ST62BC003-B, ST62C006, ST62BC004-B1, ST62BC005-B

// Magitronic B233 (8 ISA slots)
// SUNTAC Chipset, http://toastytech.com/manuals/Magitronic%20B233%20Manual.pdf
// SUNTAC ST62BC002-B, ST62BC005-B, ST62BC003-B, ST62BC001-B, ST62C00B, ST62BC004-B1
ROM_START( magb233 )
	ROM_REGION16_LE(0x20000, "bios", 0)  // BIOS-String: DSUN-1105-043089-K0
	ROMX_LOAD( "magitronic_b233_ami_1986_286_bios_plus_even_sa027343.bin", 0x10000, 0x8000, CRC(d4a18444) SHA1(d95242104fc9b51cf26de72ef5b6c52d99ccce30), ROM_SKIP(1) )
	ROMX_LOAD( "magitronic_b233_ami_1986_286_bios_plus_odd_sa027343.bin", 0x10001, 0x8000, CRC(7ac3db56) SHA1(4340140450c4f8b4f6a19eae50a5dc5449edfdf6), ROM_SKIP(1) )
	// ROM_LOAD("magitronic_b233_ami_1986_keyboard_bios_plus_a025352.bin", 0x0000, 0x0800), CRC(84fd28fd) SHA1(43da0f49e52c921844e60b6f3d22f2a316d865cc) )
ROM_END

// Magitronic B236 (ISA8: 2, ISA16: 6) - 286 Modular BIOS Version 3.03GX
// SUNTAC ST62BC002-B, ST62BC005-B, ST62BC003-B, ST62BC001-B, ST62C00B, ST62BC004-B1
ROM_START( magb236 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "sunt-lo.rom", 0x18000, 0x4000, CRC(d05c0edf) SHA1(bfd9c68cd5dc874b9519056b3a8cc6ea504b0be3), ROM_SKIP(1) )
	ROMX_LOAD( "sunt-hi.rom", 0x18001, 0x4000, CRC(e5dce491) SHA1(282ad2da0ef47147cbc0c68295e3d4249f4147b2), ROM_SKIP(1) )
ROM_END

// AUVA COMPUTER, INC. BAM/12-S2 - VIP - Phoenix 80286 ROM BIOS PLUS Version 3.10 10
// Chipset: SUNTAC ST62BC004-B1, ST62BC001-B, ST62BC002-B, ST62BC003-B, ST62BC005-B - ISA8: 3, ISA16: 5
ROM_START( aubam12s2 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "st-ph-l.rom", 0x00001, 0x10000, CRC(7f60168c) SHA1(a6d8dafa6319753466243dbde9676fa0e402f5fe), ROM_SKIP(1))
	ROMX_LOAD( "st-ph-h.rom", 0x00000, 0x10000, CRC(5b4fd7ee) SHA1(821fe868da5c7ff28f2c7b9bae03d0b8a76af796), ROM_SKIP(1))
ROM_END

// HLB-286 MBA-009 - BIOS: 286 Modular BIOS Version 3.03 HL - HLB-286 System
// SUNTAC ST62BC002-B, ST62BC005-B, ST62BC003-B, ST62BC001-B, ST62C00B, ST62BC004-B1
ROM_START( mba009 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "hlb-286l.bin", 0x18000, 0x4000, CRC(9085b21c) SHA1(4f264612c458ab03f94dbac9852fcf9dea2065cc), ROM_SKIP(1))
	ROMX_LOAD( "hlb-286h.bin", 0x18001, 0x4000, CRC(03cdbee8) SHA1(9ea5f91a76bc8861fdc7e5381e8dc15f8fb428f5), ROM_SKIP(1))
ROM_END

// Everex EV-1815 (C & T/Suntac) - RAM: 4xSIMM30, 512KB or 1MB total (2/4 SIMMs)
ROM_START( ev1815 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: Award 286 Modular BIOS Version 3.03 - GCH
	// additional info from chukaev.ru54.com: SUNTAC ST62BC... 001-B, 002-B, 003-B, 004-B1, 005 - ISA8: 3, ISA16: 5 - CPU/FPU: N80L286-10, 80287
	ROM_SYSTEM_BIOS(0, "ev1815303", "Everex EV-1815 V3.03")  /* (BIOS release date:: 15-11-1985) */
	ROMX_LOAD( "award_v3.03_ev1815_even.bin", 0x18000, 0x4000, CRC(dd64bdd6) SHA1(b3108b692d2aa03701ac894602e9418ae0779702), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "award_v3.03_ev1815_odd.bin", 0x18001, 0x4000, CRC(29f023fb) SHA1(873561bb7087483c0c763ef9cd32c1adf0f7cb5e), ROM_SKIP(1) | ROM_BIOS(0) )
	// 1: Award 286 Modular BIOS Version 3.03GS
	ROM_SYSTEM_BIOS(1, "ev1815303gs", "Everex EV-1815 V3.03GS") /* (BIOS release date:: 15-11-1985) */
	ROMX_LOAD( "award_v3.03gs_ev1815_even.bin", 0x10000, 0x8000, CRC(59489ec2) SHA1(b3c13ba53d4c4ee75a15703236a748121102ce84), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "award_v3.03gs_ev1815_odd.bin", 0x10001, 0x8000, CRC(5bcd9421) SHA1(f32e5a39da593c6982f964fb05b0802d54c3de45), ROM_SKIP(1) | ROM_BIOS(1) )
ROM_END

// Mintek BAY-1000C V1.01 - Chipset: SUNTAC ST62BC002-B, ST62C005-B, ST62BC001-B, ST62BC003-B, ST62BC004-B1, ST62C008
// CPU: AMD N80L286-12/S - RAM: 4xSIMM30, 18x18pin/16pin, 8x20pin, 4x16pin - OSC: 12.000, 14.31818, 25.000MHz - ISA8: 2, ISA16: 6
// BIOS-String:  Phoenix 80286 ROM BIOS PLUS Version 3.10 00 / LYI-CHENG ENTERPRISE CO., LTD.
ROM_START( bay1000c )
	ROM_REGION16_LE( 0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "286-suntac_bay-1000c-ic22_32k.bin", 0x10000, 0x8000, CRC(105d3257) SHA1(cf10d09db57f65fee649adcb39058d9d9aefe9e9))
	ROM_LOAD16_BYTE( "286-suntac_bay-1000c-ic23_32k.bin", 0x10001, 0x8000, CRC(3b997bb1) SHA1(70d3bb9e57624c9f64d70bc5f3c00305a08a8b2e))
ROM_END

ROM_START( suntac6 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// CPU: AMD N80L286-12/S - RAM: 36 sockets 18pin/16pin, fitted: 36x41C256P-8 - BIOS: Award 286 - Keyboard-BIOS: M5L8042-165P
	// ISA8: 2, ISA16: 6 - OSC: 12.000 MHz, 24.000, 14.31818
	// 0: BIOS-String: 286 Modular BIOS Version 3.03 01 Copyright Award Software Inc.
	ROM_SYSTEM_BIOS(0, "terraat", "Terra AT")
	ROMX_LOAD( "286-suntac-award_a2132439lo.bin", 0x18000, 0x4000, CRC(5fa269f8) SHA1(4167017aaeb63f4eedde155ba29f33ae2a94403b), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "286-suntac-award_a2132439hi.bin", 0x18001, 0x4000, CRC(114604c3) SHA1(db957783e6b16f6e1b8a831130d37fe51da84430), ROM_SKIP(1) | ROM_BIOS(0) )
	// 1: BIOS-String: 286 Modular BIOS Version 0N3.03 NFS / ELT - board is identical to #0
	ROM_SYSTEM_BIOS(1, "286suntacelt", "286 SUNTAC ELT")
	ROMX_LOAD( "286-suntac-award_a2184058lo.bin", 0x18000, 0x4000, CRC(dbf48678) SHA1(75cb7971519cf55f9bb024eed70b831af1799506), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "286-suntac-award_a2184058hi.bin", 0x18001, 0x4000, CRC(2abea425) SHA1(1183d1fc665eab11042643c4f2e0eaa0490bb3df), ROM_SKIP(1) | ROM_BIOS(1) )
	// 2: BIOS-String: DSUN-1105-043089-K0 - Keyboard-BIOS: AMI Keyboard BIOS PLUS A086031
	// CPU: Intel N80286-12 - RAM: 36x16pin/16pin, used 36xTC511000AP-80 - ISA8: 2, ISA16: 6 - BIOS: AMI 286 BIOS PLUS SA073155
	ROM_SYSTEM_BIOS(2, "sa073155", "SA073155")
	ROMX_LOAD( "286-suntac_sa073155_ic22.bin", 0x00000, 0x10000, CRC(abf6c367) SHA1(79a07b9b9af2e1963cfcae75fafc3478885237cb), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "286-suntac_sa073155_ic23.bin", 0x00001, 0x10000, CRC(1d286cd9) SHA1(31be40c5008dc67cb24f4418cffaa57682e654c0), ROM_SKIP(1) | ROM_BIOS(2) )
	// 3: BIOS ROMs are marked TCI, Award 286 Modular BIOS Version 3.03HLS
	// complains about "refresh timing error, but works - BIOS release date:: 15-11-1985
	ROM_SYSTEM_BIOS(3, "tci", "TCI")
	ROMX_LOAD( "suntac_80286_lo.bin", 0x18000, 0x4000, CRC(f7bf6c49) SHA1(d8e813c264008f096006f46b90769c0927e44da9), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "suntac_80286_hi.bin", 0x18001, 0x4000, CRC(5f382e78) SHA1(8ba222df9d7028513e37978598d8139906e8834c), ROM_SKIP(1) | ROM_BIOS(3) )
	// 4: BIOS-String: D286-5017-011388
	ROM_SYSTEM_BIOS(4, "st62m02", "ST62M-02-B")
	ROMX_LOAD( "7_st62m02-b_l.bin", 0x10000, 0x8000, CRC(fd24911f) SHA1(71ab1177d0b6b9482353e3b405f4b332cbeecfc3), ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "7_st62m02-b_h.bin", 0x10001, 0x8000, CRC(fca78c7b) SHA1(35e892bf52fb1cbc9bfaed7866c2ef7a31d4b762), ROM_SKIP(1) | ROM_BIOS(4) )
	// 5: BIOS-String: Phoenix ROM BIOS PLUS Version 3.10 00 - LYI-CHENG ENTERPRISE CO., LTD.
	ROM_SYSTEM_BIOS(5, "bay1000", "Bay 1000")
	ROMX_LOAD( "286-suntac-bay-1000c-ic22.bin", 0x18000, 0x4000, CRC(7f3ef79e) SHA1(d4f7086e2902d3b264b1fff76d1bea811aa58fc5), ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "286-suntac-bay-1000c-ic23.bin", 0x18001, 0x4000, CRC(cca972a4) SHA1(22bd712700bfacd34070e704e901623ecc37a390), ROM_SKIP(1) | ROM_BIOS(5) )
	// 6: BIOS-String: Phoenix 80286 ROM BIOS PLUS Version 3.10.22 - Personal computer 286
	ROM_SYSTEM_BIOS(6, "st2806036", "Suntac ST2806036")
	ROMX_LOAD( "286-suntac-2806036-lo.bin", 0x10000, 0x8000, CRC(907dcbf7) SHA1(8782e49926366d7a640c60b875e6c091091a2f54), ROM_SKIP(1) | ROM_BIOS(6) )
	ROMX_LOAD( "286-suntac-2806036-hi.bin", 0x10001, 0x8000, CRC(ba24e88f) SHA1(07983752c8128ae62391737f428d6db42fefdbb8), ROM_SKIP(1) | ROM_BIOS(6) )
	// 7: BIOS-String: 286 Modular BIOS Version 3.03YK2 Copyright Award Software Inc. - YOUTH KEEP ENTERPRISE CO., LTD.
	ROM_SYSTEM_BIOS(7, "youth", "Suntac Youth")
	ROMX_LOAD( "286-suntac-youth-rom1-128.bin", 0x18000, 0x4000, CRC(f232c31b) SHA1(49b9990c951a61bde10478cbb5db4b913baae1e2), ROM_SKIP(1) | ROM_BIOS(7) )
	ROMX_LOAD( "286-suntac-youth-rom3-128.bin", 0x18001, 0x4000, CRC(0735b127) SHA1(0c79cbd7d40b75dcba5fe33bf8e3a96050e12af5), ROM_SKIP(1) | ROM_BIOS(7) )
	// 8: BIOS-String: 286 Modular BIOS Version 3.01, Copyright Award Software Inc. - ECS
	// complains about Refresh Timing Error
	ROM_SYSTEM_BIOS(8, "ecs", "Suntac ECS")
	ROMX_LOAD( "st62_award_3.01_ecs_suntac_rom1.bin", 0x18000, 0x4000, CRC(17296492) SHA1(3a3bf7c20946ef56b767f54c8de45cd46d5c1167), ROM_SKIP(1) | ROM_BIOS(8) )
	ROMX_LOAD( "st62_award_3.01_ecs_suntac_rom3.bin", 0x18001, 0x4000, CRC(5f0aa2d9) SHA1(5ed5897adb4507c399f200dad9337c1c8b246a48), ROM_SKIP(1) | ROM_BIOS(8) )
	// 9: AMI 286 BIOS for MORSE Personal Computer - BIOS-String: DSUN-1216-091589-K0
	ROM_SYSTEM_BIOS(9, "stmorse", "ST-Morse")
	ROMX_LOAD( "st-morse.bio", 0x10000, 0x10000, CRC(7136e89f) SHA1(50d15f96dba855e58bb39c937ad9358fc0084b10), ROM_BIOS(9) )
	// 10:  286-BIOS (C)1987 AMI, for CDTEK - BIOS-String: DSUN-1202-043089-K0
	ROM_SYSTEM_BIOS(10, "sunami", "Suntac AMI")
	ROMX_LOAD( "suntac_ami_286_even_bios.bin", 0x10001, 0x8000, CRC(acdffb05) SHA1(180fd693bf86a6fdecc713d5873f3c0950b56c98), ROM_SKIP(1) | ROM_BIOS(10) )
	ROMX_LOAD( "suntac_ami_286_odd_bios.bin", 0x10000, 0x8000, CRC(9003d5ad) SHA1(6a2de572d11625ecdacc4ad7b5c324b160540541), ROM_SKIP(1) | ROM_BIOS(10) )
	// 11: BIOS-String: Phoenix 286 ROM BIOS PLUS Version 3.10 10 - VIP
	ROM_SYSTEM_BIOS(11, "sunphovip", "Suntac Phoenix VIP")
	ROMX_LOAD( "st-ph-l.rom", 0x10001, 0x8000, CRC(ebb6446f) SHA1(086a8f016c2c0cc56d3bd7ea4e152ae215d4e5ce), ROM_SKIP(1) | ROM_BIOS(11) )
	ROMX_LOAD( "st-ph-h.rom", 0x10000, 0x8000, CRC(1c77bd34) SHA1(0dea2dd8ba69fdfb829d152840817bcbdcc3e394), ROM_SKIP(1) | ROM_BIOS(11) )
	// 12: BIOS-String: 286-BIOS (C) AMI, for SUPERCOM - SSUN-1120-042589-K0
	ROM_SYSTEM_BIOS(12, "supercom", "Suntac Supercom")
	ROMX_LOAD( "supercom_lo.bin", 0x18000, 0x4000, CRC(6c8ce417) SHA1(fc2cdc9d23e9d75bb48d26b102873a9964871f52), ROM_SKIP(1) | ROM_BIOS(12) )
	ROMX_LOAD( "supercom_hi.bin", 0x18001, 0x4000, CRC(6c1b645d) SHA1(0def25267428338804c5858e3f536720a2b7d349), ROM_SKIP(1) | ROM_BIOS(12) )
	// 13:
	ROM_SYSTEM_BIOS(13, "suntacmr", "Suntac MR BIOS")
	ROMX_LOAD( "v000v200-1.bin", 0x10000, 0x8000, CRC(1a34d56e) SHA1(ae950de20641c6394485d891e50136b1dc5261e3), ROM_SKIP(1) | ROM_BIOS(13) )
	ROMX_LOAD( "v000b200-2.bin", 0x10001, 0x8000, CRC(2aeea8bd) SHA1(e6c306cc56dd614d704262a087dcc07b75fd9ac6), ROM_SKIP(1) | ROM_BIOS(13) )
ROM_END

// ***** 286 motherboards using the 5 chip VLSI chipset

ROM_START( vlsi5 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: MG Products (Japanese) - Chipset: VLSI VL82C102A; VLSI VL82C101B; VLSI VL82C104; VLSI VL82C103A; VLSI VL82C100; (VLSI 8908BT; 8906BT; 8852BT; 8907BT; 8906BT)
	// BIOS: AMI 286 BIOS+ - BIOS-String: D286-9987-092588-K0 - ISA8: 2, ISA16: 8
	// (BIOS release date:: 25-09-1988) (ISA8: 3, ISA16: 5)
	ROM_SYSTEM_BIOS(0, "286vlsij", "Japanese 286 VLSI")
	ROMX_LOAD( "286-vlsi_japan-2-even_32k.bin", 0x10000, 0x8000, CRC(e3e64cbc) SHA1(5259e3c8686f2239a5fb0dc38aa80380ef9ec5fa), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "286-vlsi_japan-2-odd_32k.bin", 0x10001, 0x8000, CRC(aa533f39) SHA1(d88c7d4029a283b94b99e2017d29fbf9eb9105b1), ROM_SKIP(1) | ROM_BIOS(0) )
	// 1:  BIOS-String:  D286-1223-121589-K0 - 286-BIOS AMI for MBVLSI-168 - ISA8: 3, ISA16: 5
	ROM_SYSTEM_BIOS(1, "mbvlsi168", "MBVLSI-168")
	ROMX_LOAD( "286-vlsi-002350-041_32k.bin", 0x10000, 0x8000, CRC(0e0e2bc9) SHA1(0af05b15ea8141ece84fb4420e6a21720f01c7a6), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "286-vlsi-002350-042_32k.bin", 0x10001, 0x8000, CRC(5ef7b91d) SHA1(d57c7f4c8d28708f128c5f0b1251d5943c7cdf76), ROM_SKIP(1) | ROM_BIOS(1) )
ROM_END

// Uniron U3911-V3 - Chipset as above - BIOS-String: Phoenix 80286 ROM BIOS PLUS Version 3.10 00 - P/N 891012 - 80286
ROM_START( u3911v3 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "286-uniron-u13_32k.bin", 0x10001, 0x8000, CRC(c1acdf6d) SHA1(cb064dac00620588f66f850fee91ef6b47e57012), ROM_SKIP(1) )
	ROMX_LOAD( "286-uniron-u14_32k.bin", 0x10000, 0x8000, CRC(d2e9c52a) SHA1(ff6726b527b0bebed50c053a698e1b61aada3043), ROM_SKIP(1) )
ROM_END

// Toptek 286 Turbo (board name somewhat uncertain; 5x 8-bit ISA, 3x 16-bit ISA, 2 banks of onboard RAM + 2 banks expansion RAM)
// VLSI VL82C100 + VL82C101B + VL82C102A + VL82C103A + VL82C104; MC146818 or HM6818P RTC; unidentified keyboard controller
ROM_START( toptek286 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// BIOS-String: D286-1295-091589-K0
	// Original BIOS64 dump split into even and odd bytes based on available PCB info
	ROM_LOAD16_BYTE( "toptek_vlsi_even.bin", 0x10000, 0x8000, CRC(f35465e8) SHA1(c85afc2168e355120c63b68d5c11fce7770fe1b7) )
	ROM_LOAD16_BYTE( "toptek_vlsi_odd.bin", 0x10001, 0x8000, CRC(b7272729) SHA1(686c976b9b7989862846a79d00f1f9116f03bc17) )
ROM_END


// ***** 286 motherboards using the 5 chip Winbond chipset W83C201P + W83C202AP + W83C203AP + W83C204P + W83C205AP

// KT216WB5-HI Rev.2 (1 8-bit ISA, 5 16-bit ISA) - CPU: Harris CS80C286-16, FPU: 80287 - OSC: 32.000MHz, 14.31818
// Winbond W83C201P + W83C202AP + W83C203AP + W83C204P + W83C205AP; MC146818AP RTC; JETkey keyboard BIOS - RAM: 1MB DIP, 4xSIPP30
ROM_START( kt216wb5 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// BIOS-String: D286-1149-083090-K0 - WIN 286 BIOS
	ROMX_LOAD( "kt216wb5_even.bin", 0x10000, 0x8000, CRC(6b5509c0) SHA1(73b303b90cc0cd23b7e13362019193c938a2e502), ROM_SKIP(1) )
	ROMX_LOAD( "kt216wb5_odd.bin", 0x10001, 0x8000, CRC(af541ada) SHA1(26d2617dbe8c15f1b0d4782375bcb291a7923703), ROM_SKIP(1) )
ROM_END

// KMA-202F-12R - ISA16:7 - BIOS-String: 286 Modular BIOS Version 3.11
// Winbond W83C201P + W83C202AP + W83C203AP + W83C204P + W83C205AP; DS12887+ RTC; AWARD keyboard BIOS
ROM_START( kma202f )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "286_lo.bin", 0x10000, 0x8000, CRC(0ce69691) SHA1(6904ac54f30f2244058653aaa623804dd02b4332), ROM_SKIP(1) )
	ROMX_LOAD( "286_hi.bin", 0x10001, 0x8000, CRC(1330b6f2) SHA1(691bb4a51ce3d9a026ee33c3fd02fc4e13b4a184), ROM_SKIP(1) )
ROM_END


// ***** unknown chipset

// Octek XT-286 V1.1 - "Keyboard error or locked"
ROM_START( ocxt286 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "ocxt286_1", "Octek XT-286 V1.1")
	ROMX_LOAD( "xt286-328.bin", 0x1e000, 0x2000, CRC(d9caefcc) SHA1(dab3403678feb023362df614596d1306ef7f85db), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "ocxt286_2", "Hedaka HED-919")
	ROMX_LOAD( "hed919-328f.bin", 0x1e000, 0x2000, CRC(15117381) SHA1(d9dfa796edf7e94b9dacf984763ac046cf80f26d), ROM_BIOS(1))
ROM_END

//**************************************************************************
//  80286 Desktop
//**************************************************************************

// SIIG MiniSys 2000 - Motherboard ID: Labway MS101V1.2 - This is a tiny 286 system, the width of a 3.5" disk drive with an external power supply.
// A physical switch allows to change between VGA and composite video output.
// CPU: Intel 286-12 - BIOS: AMIBIOS - BIOS string: DH12-1422-061390-K0 - Chipset: Headland HT12/A, Acer M5105 - RAM: 2xSIMM30
// OSC: 14.31818MHz, 24.000MHz, 24.000, another unreadable - ISA16: 1 (on riser board) - on board: IDE, Floppy, 2xser, par, game, composite, VGA,
// ext. FDD (motherboard and I/O board connected with a riser board - Mass storage: 3.5" FDD, 3.5" HDD (e.g. Seagate ST351A/X in AT mode)
ROM_START( minisys2k )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "minisys2000_even.bin", 0x10001, 0x8000, CRC(a4c3eade) SHA1(ea6d19fa12994882f8a77f67c2c358bba57abe28), ROM_SKIP(1) )
	ROMX_LOAD( "minisys2000_odd.bin", 0x10000, 0x8000, CRC(0e904497) SHA1(a55de8fdaf0442cc7c640dfc88daa37c851fd324), ROM_SKIP(1) )
ROM_END


// ICL DRS M40 (motherboard: ICL M40/M45/915V)
// Chipset: Chips and Technologies NEAT, WD37C65C-PL - CPU: - FPU socket provided - BIOS: Acer (made by Phoenix) - Keyboard BIOS: KBC V4 82HV
// OSC: 10.000000MHz, 16.000MHz, 25.175999MHz, 32.000000MHz, 14.31818 - RAM: DIP 640KB, 4xSIMM30, 2xSIMM/SIPP30 no sockets - VGA on board:  PVGA1A-JK, 256KB RAM
// no regular ISA slots, ISA8: 1, ISA16: 1 on a riser card - On board: IDE, floppy, par, 2xser
ROM_START( icldrsm40 ) // no POST, halts
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "icl_m40_l_bin", 0x10000, 0x8000, CRC(1b493570) SHA1(2a3cee0e7a45f07439c54970513e85c9134fee32), ROM_SKIP(1) )
	ROMX_LOAD( "icl_m40_h.bin", 0x10001, 0x8000, CRC(451421af) SHA1(82d3c94cf04df1b48540fbb3c4d9ad4d6eac8823), ROM_SKIP(1) )

	ROM_REGION( 0x8000, "vga", 0) // WDC WD90C11-LR VGA BIOS
	ROMX_LOAD( "icl_m40_vga_bios_l.bin", 0x0000, 0x4000, CRC(522c5c02) SHA1(37f8299a0dcf6b028e1012313ae787bc389ed1f2), ROM_SKIP(1) )
	ROMX_LOAD( "icl_m40_vga_bios_h.bin", 0x0001, 0x4000, CRC(ed29de22) SHA1(80a508a42dc731fc33584ba2da9e478c401e5d47), ROM_SKIP(1) )
ROM_END

// Twinhead PS-286V Rev 0.1 (used in Twinhead Netstation PC) - Chipset: Twinhead TH4100, TH6260, Zilog Z0765A08PSG FDC, 2x16C450PC,
// BIOS: Phoenix NEAT - BIOS Version: 3.1003D 3462421, Keyboard BIOS: Phoenix - CPU: Intel N80286-12, FPU socket provided
// RAM: 8xiT21014-08, 8xSIMM30 - ISA16: 1 - On board: WD VGA, floppy, ide, 2xser, par - Video: WD90C11-LR, IMSG176J-50Z, Twinhead VGA BIOS V1.20, RAM: 2xIntel T21014-08
// OSC: 14.31818, 24.000MHz - DIP8: on-off-off-off-on-on-on-on
ROM_START( twinnet ) // BIOS-String: Phoenix ROM BIOS PLUS Version 3.10 03 - Twinhead International Corporation
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "ps-286v_lo.bin", 0x10000, 0x8000, CRC(71920f1b) SHA1(f6d76d10b17df7488c5c70a912403dd45f0afbc3), ROM_SKIP(1) )
	ROMX_LOAD( "ps-286v_hi.bin", 0x10001, 0x8000, CRC(d79495e3) SHA1(e5d53ae7059502b2259d575ca8e8fdff7f712389), ROM_SKIP(1) )

	ROM_REGION( 0x8000, "vga", 0) // WDC WD90C11-LR VGA BIOS
	ROM_LOAD( "wdc_vga.bin", 0x0000, 0x8000, CRC(f897048e) SHA1(3baeb553dae4f1c641fb01a16bfe4ae3ca95b13d))
ROM_END

// Zenith Z-248 - Motherboard: 85-3379-01 CPU BOARD 113087 - CPU: AMD N80L286-12/C, FPU socket provided - Chipset: Zymos Poach 1 and 2
// RAM: 6xSIMM30 - OSC: 24.000MHz, 16.000MHz, 14.31818MHz - ISA8: 2, ISA16: 5 - on board diagnostic LEDs: red: CPU D101, ROM D102, RAM D103, INT D104, DSK D105, RDY D106, green: DCOK D107
// Cards in system documented: DTC 5280 CRA MFM HD controller, Graphics card HEATH P/N150-307-3 L1A2334/Chips P82C434A (undumped), Logitech mouse/hand scanner controller, GW302 Parallel Printer Card, Chips P82C605 Dual Serial Printer Card
// "+++ ERROR: Fatal Slushware RAM Error +++" / "--- Fatal Error: Cannot Continue! ---" (cf. olyport40)
ROM_START( zdz248 ) // use CGA
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "zenith_z-248.bin", 0x10000, 0x10000, CRC(e2042dd9) SHA1(bd51cc74b0b7bd42c449bc4b5702274f766e8ea5))
ROM_END

// Samsung Deskmaster 286-12, Microfive motherboard - Chipset: Chips F82C451C, F82C235-12 286 SCAT - CPU: Intel 80286-12, FPU socket provided
// RAM: 6xSIMM40 - BIOS: Phoenix ROM BIOS PLUS 3.10 02M, Chips and Technologies VGA 411 BIOS v211 (one 27C512 EPROM) - Keyboard-BIOS: Intel P8942AN - ISA16: 1
// OSC: 14.31818, 24.000000MHz, 24.000000MHz, 25.175000MHz, 40.000000MHz - on board: FDD, IDE, beeper, keyboard, mouse, ser, par - VGA Video RAM: 8x41C464J-10 (256K)
ROM_START( samdm286 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "samsung-deskmaster-28612-rom.bin", 0x10000, 0x10000, CRC(785d3196) SHA1(214a933d8fa86bfdb633fb5e8595a18a58cdba7d))
ROM_END

// Schneider EuroAT - Uses the same case as the Schneider EuroXT, a compact desktop with room for a single floppy drive and an AT IDE harddisk (Seagate ST-142A, ST-157A)
// Mainboard: Baugr.Nr. 51513 with internal EGA, 52591 EGA components omitted (see: EURO VGA)
// Chipset: 2xHeadland GC102-PC, HT101A/B1A4924, Schneider BIGJIM 30773 (cf. EuroPC 2/EuroXT), WD37C65BJM, Siemens SAB 16C450-N
// EGA chipset (mainboard 51513): G2 GC201-PC, 64K RAM - Main RAM: 1MB
// CPU: Siemens SAB 80286-12, Keyboard-BIOS: Schneider ROM BIOS 1985, 1989 Phoenix
// Connectors: Keyboard, Printer, Serial, Floppy (can use the same external floppy disk drives as the EuroXT), EGA monitor
// OSC: 34.0000, 19.2000MHz, 24.0000, 16.000MHz
// BUS: proprietary connectors, ISA riser (ISA8x1, ISA16x1), BIOS can set CPU and BUS speed up to 12MHz
// Proprietary EURO VGA card: 256KB RAM, ATI 18800-1 chipset
// blank screen, beeps 1-2-4
ROM_START( euroat )
	ROM_REGION16_LE( 0x20000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "v201", "V2.01" ) // also used on Tower AT
	ROMX_LOAD( "euro_at_v201a_l.bin", 0x10000, 0x8000, CRC(0f8a2688) SHA1(95db9010b1c0465f878e5036bcf242ddf0a3be6a), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "euro_at_v201a_h.bin", 0x10001, 0x8000, CRC(75b6771b) SHA1(3aa0921914ea6e24249ce3f995fdcb341124d7e9), ROM_SKIP(1) | ROM_BIOS(0) )
	// EGA ROM dump missing

	ROM_SYSTEM_BIOS( 1, "v203", "V2.03" )
	ROMX_LOAD( "80286at_bioslow_schneider_ag_id50444_v2.03.bin", 0x10000, 0x8000, CRC(de356110) SHA1(6bed59a756afa0b6181187d202b325e35afadd55), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "80286at_bioshigh_schneider_ag_id50445_v2.03.bin", 0x10001, 0x8000, CRC(c4c9c840) SHA1(09deaa659191075b6ccc58403979d61bdf990dcd), ROM_SKIP(1) | ROM_BIOS(1) )

	ROM_REGION( 0x10000, "vga", 0 )
	ROM_LOAD( "euro-vga_52255_bios_v1.02_row.bin", 0x00000, 0x10000, CRC(71d42e58) SHA1(be64990325f52128e102dfc3ed87d2d831183ddc))
ROM_END

// Schneider Tower AT 220 (other designations for the 10 MHz 80826, 512KB RAM series are 201, 202, 240), the last two digits are related to the originally installed
// number of 3.5" 720K floppy drives or the size of the MFM harddisk), Model 260 has a 60MB harddisk and can have a 12.5 MHz CPU (depending on where you look and
// probably what was available in Schneider's part bin), systems with a "mega" in the name have 1MB RAM and 1.44MB floppy drives. All have an EGA graphics card on board
// The case looks like a stack of three thinner slices, and extra modules were available that clamped on: a tape streamer, and a 5.25" 1.2MB or a 360KB drive. They were
// connected to the "External drive" port of the Tower AT, much like with Schneider's other PCs. The mainboard as such is divided between the I/O and video portion that resides on
// the backplane board and the CPU and RAM on the CPU card that also contains the keyboard connector.
// Model 220, Schneider Tower-EGA I/O: Chipset: JIM 50101-1 (cf. EuroPC), WD37C65BJM, Gemini VC-001, VLSI VL16C450-PC, Paradise Systems Inc PPC1 38302C
// 104 pin CPU card connector (ISA without the key), 3xISA16, 1xISA8 - on board: parallel, serial, bus mouse (Atari compatible), EGA, internal floppy (26pin), external floppy (DB25)
// Model 220, Schneider Tower-CPU 286 (Baugr.Nr. 50229 Rev.3B): Dallas DS1287, MBL8042H (Compatibility Software 1986/K Phoenix Technologies Ltd - 805931) - Chipset: 2x G2 GC102, G2 GC101
// OSC: 20.000MHz, 14.318180, beeper, CPU: AMD N80L286-10/S, FPU socket provided - RAM: solder pads for 4xSIMM30, 4x16pin (empty), 4x or 8x51C4256 (512KB or 1MB)
// The Tower AT was available with the Schneider VGA I/O that is described in the tower386sx section. The Tower VGA System 40 and System 70 models had the 12.5MHz CPU card.
// Its BIOS version 2.03 is undumped so far.
// blank screen, beeps 1-2-4
ROM_START( towerat2xx )
	ROM_REGION16_LE( 0x20000, "bios", 0 )
	ROM_SYSTEM_BIOS(0, "v2.02", "V2.02" ) // from a model 220
	ROM_SYSTEM_BIOS(1, "v2.01", "V2.01" )
	ROM_SYSTEM_BIOS(2, "v1.07", "V1.07" ) // seen on a model 240 "mega"
	ROM_SYSTEM_BIOS(3, "v1.06", "V1.06" ) // from a model 220
	ROM_SYSTEM_BIOS(4, "v1.05a", "V1.05a" )
	ROM_SYSTEM_BIOS(5, "v1.01a", "V1.01a" ) // from a model 220

	ROMX_LOAD( "phoenix_860376_schneider_ag_tower_at_bios_0_id.nr.50445_v2.02.bin", 0x10000, 0x8000, CRC(8566b3f2) SHA1(a12b5e9e848de123c62374f78ee1d2b4b53dd468), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "phoenix_860376_schneider_ag_tower_at_bios_1_id.nr.50445_v2.02.bin", 0x10001, 0x8000, CRC(7d8249cf) SHA1(d894332aad4c26798e6b41a5e94c471b0235bd50), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "schneider_tower_at_bios_0_low_v2.01a.bin", 0x10000, 0x8000, CRC(0f8a2688) SHA1(95db9010b1c0465f878e5036bcf242ddf0a3be6a), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "schneider_tower_at_bios_1_high_v2.01a.bin", 0x10001, 0x8000, CRC(75b6771b) SHA1(3aa0921914ea6e24249ce3f995fdcb341124d7e9), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "schneider_tower_at_bios_0_low_v1.07.bin", 0x10000, 0x8000, CRC(70a9421d) SHA1(bf6529f259d5bc7c28df19655c57ecce1c57260f), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "schneider_tower_at_bios_1_high_v1.07.bin", 0x10001, 0x8000, CRC(995a62db) SHA1(42e9a866b5f02509d3094c42842eafed1d577f4e), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "schneider_ag_50444_v1.06.u3", 0x10000, 0x8000, CRC(42891d5a) SHA1(d94292b14f9155b4e05c78960f9722fffca976be), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "schneider_ag_50445_v1.06.u4", 0x10001, 0x8000, CRC(bdced2b9) SHA1(cba58c70420695ec69dbb4817d0c6b14b8bdbadd), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "schneider_tower_at_bios_low_v1.05a.bin", 0x10000, 0x8000, CRC(94ad1628) SHA1(bf7319ed9b37a57e67b0b4bf7845d95d0f593d68), ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "schneider_tower_at_bios_high_v1.05a.bin", 0x10001, 0x8000, CRC(f3d48773) SHA1(9386313b6d05acb30e7ba7e1353c259deaaa77bc), ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "schneider_tower_at_bios_low_v1.01.bin", 0x10000, 0x8000, CRC(a94ca070) SHA1(2acca0601c00e76d510c81dfe92d33397fbeccd1), ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "schneider_tower_at_bios_high_v1.01.bin", 0x10001, 0x8000, CRC(d8f67320) SHA1(3ddf7fdb1370f745c4f1902101605477ee0bb392), ROM_SKIP(1) | ROM_BIOS(5) )

	// todo: find matching EGA ROMs for BIOS V2.01, 1.07 and 1.05
	ROM_REGION( 0x8000, "gfx", 0)
	ROMX_LOAD( "schneider_ag_tower_ega-bios_50477_v1.04.bin", 0x0000, 0x8000, CRC(aabd1017) SHA1(e019c21d6108a0387f7c98e92e4dbc32ab19929f), ROM_BIOS(0) ) // R1.04 matched with system BIOS V2.02
	ROMX_LOAD( "schneider_ag_tower_ega-bios_50477_v1.04.bin", 0x0000, 0x8000, CRC(aabd1017) SHA1(e019c21d6108a0387f7c98e92e4dbc32ab19929f), ROM_BIOS(1) )
	ROMX_LOAD( "schneider_ag_tower_ega_bios_id.nr._50447_r1.02.bin", 0x0000, 0x8000, CRC(1c43aaf6) SHA1(cf98dd8f0d8258761e36e70f086b1234ec703823), ROM_BIOS(2) )
	ROMX_LOAD( "schneider_ag_tower_ega_bios_id.nr._50447_r1.02.bin", 0x0000, 0x8000, CRC(1c43aaf6) SHA1(cf98dd8f0d8258761e36e70f086b1234ec703823), ROM_BIOS(3) ) // R1.02 matched with system BIOS V1.06
	ROMX_LOAD( "schneider_ag_tower_ega_bios_id.nr._50447_r1.02.bin", 0x0000, 0x8000, CRC(1c43aaf6) SHA1(cf98dd8f0d8258761e36e70f086b1234ec703823), ROM_BIOS(4) )
	ROMX_LOAD( "schneider_ega_r1.00.bin", 0x0000, 0x8000, CRC(4e14cb0a) SHA1(6cef69274a52b11201a3477631fa343a7e1a5970), ROM_BIOS(5) ) // R1.00 matched with system BIOS V1.01 */
ROM_END


// Victor V286C - a VGA version exists as well
// CPU: AMD 802L86-10/S  - one ISA16 extended to ISA8: 1, ISA16: 3 on a riser card - Keyboard-BIOS: AT-KB M5L8042
// Chipset: Kyocera AT-S.C.1 VER.A 9771A 89432EAI, Kyocera EAST-2A 9850 8938EAI, MB621103 M AT-1A 8944 Z67, MB622436 MAT-2E 8943 W02
// On board video EGA/CGA/Hercules based on a L1A2919 PARADISE PBI-38306A controller
// RAM: 640KB on board -  OSC: 48.000, 32.0000MHz, 40MHz, 18.63636MHz
// Options: Sockets for a Microsoft 900110003 Bus Mouse controller, FPU 80287 and a DS1287 RTC - 2 DIP switches with 12 positions combined
// Mass storage: 1.2MB floppy, Kyocera KC-30A harddisk on a DTC5187CRH RLL controller
ROM_START( v286c )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "system-bios_kyocera_a2-2_lo.bin", 0x18000, 0x4000, CRC(160c4759) SHA1(937c1bb9483efeba895e038b7132e0e7e5a9aaa4) )
	ROM_LOAD16_BYTE( "system-bios_kyocera_a2-2_hi.bin", 0x18001, 0x4000, CRC(cfe0cbef) SHA1(2610d727d13fa67c7bd9b3545d7846e880c3da37) )

	ROM_REGION(0x4000, "pega", 0)
	ROM_LOAD( "ega-bios_paradise_video-1.bin", 0x0000, 0x4000, CRC(2db77b0b) SHA1(d31ddbbde5be0b0603e9f569c3f924e0afc7c8e4) )
ROM_END

// Atari PC 4, PC4X motherboard - Chipset: NEAT CS8221 (P82C206, P82C211, P82C212, P82C215) - ISA8: 1, ISA16: 4 - RAM: 8XSIPP30
// Paradise Systems PVGA1A-JK, IMSG171P
// on board: external and internal floppy, digital and analog video, ser, par, keyboard, mouse
ROM_START( ataripc4 ) // initializes video, then halts
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "ami_pc4x_1.7_even.bin", 0x10000, 0x8000, CRC(9f142377) SHA1(b3e5c5dfaec133646295d9a16bc1eec54fe2bc35))
	ROM_LOAD16_BYTE( "ami_pc4x_1.7_odd.bin", 0x10001, 0x8000, CRC(ae3d4cb6) SHA1(d7915ef013462aff4f189cda8f6dc0a486777b63))

	ROM_REGION16_LE(0x10000, "pvga", 0)
	ROM_LOAD16_BYTE( "pvga_pc4x_even.bin", 0x00000, 0x8000, CRC(ff222896) SHA1(e22cdcd9c69fc4feef6b8c2903e3506c79ff531b))
	ROM_LOAD16_BYTE( "pvga_pc4x_odd.bin", 0x00001, 0x8000, CRC(8ca04b2f) SHA1(7705d866ecf366bd6ea95071bf5767877461d2d5))
ROM_END

// Atari ABC 286/30, PC4LC motherboard (low cost, compared to PC4) - BIOS-String: DNET-0000-092588-K0- ISA16: 3 - RAM: 4xSIPP30
// NEAT CS8221 (P82C206, P82C211, P82C212, P82C215) - http://www.atari-computermuseum.de/abc286.htm
// on board: internal floppy, digital video (EGA), ser, par, keyboard, mouse
ROM_START( atariabc286 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "ami_pc4x_1.x_even.bin", 0x10000, 0x8000, CRC(930447c7) SHA1(fb7212b413ff8aa056bd23baadc22691ce714f8d))
	ROM_LOAD16_BYTE( "ami_pc4x_1.x_odd.bin", 0x10001, 0x8000, CRC(0891fd25) SHA1(4722b1db1b2c985c67f9a9b807ce68c06a905232))

	// ROM_REGION(0x40000, "ega", 0)
	// ROM_LOAD( "p82c441_ega_bios_v1.0.6.bin", 0x00000, 0x08001, BAD_DUMP CRC(80c11ef2) SHA1(90852d3cbb64504c8d57b469a594c22c247c9a39))
ROM_END

// Profex PC 33 - Chipset: SUNTAC ST62BC004-B1, ST62BC003-B, ST62C008, ST62C005-B, ST62BC001-B, ST72BC002-B
// RAM: 2xSIPP30, 18x18pin/16pin, 8x20pin, 4x16pin - OSC: 12.000, 14.31818, 24.000MHz - ISA8: 2, ISA16: 6
// BIOS: Award A2245250 - Keyboard-BIOS: Award - BIOS-String: 286 Modular BIOS 3.03 Copyright Award Software Inc. / GCH
ROM_START( profpc33 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "st62-pro.bio", 0x10000, 0x10000, CRC(a42f9d0e) SHA1(384f4ddaf92307a5eeb70646a85ad991d904c2d2))
ROM_END

// Epson PC AX / Equity III+ - 102-System Board Error (according to the technical manual this means
// "TlMER SPEED CHECK: An error was detected in timer controller counter 0."
ROM_START( epsax )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "evax.bin", 0x18000, 0x4000, CRC(5f9e5fc9) SHA1(5dbe414b762494fa65df6ac0391b2281f452f3e9))
	ROM_LOAD16_BYTE( "odax.bin", 0x18001, 0x4000, CRC(e9cba352) SHA1(22be1457332a62ae789e779e9666ffd63c91d010))
ROM_END

// Epson PC AX2e
ROM_START( epsax2e ) // see epsax
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "evaxe.bin", 0x10000, 0x8000, CRC(1251f3e9) SHA1(52456943ee4d83c2a3d46e75d292f4fb57a5a2d8))
	ROM_LOAD16_BYTE( "odaxe.bin", 0x10001, 0x8000, CRC(ce08f140) SHA1(1c6e62f2ab45e9574691224dd6a6ab2a823e85cc))
ROM_END

// Bull Micral 45 - Chipset: VLSI 8842AV / R2622 / VC2730-0001, VLSI 8832VB / L81711 / VL16C452-QC, MBL8042N, MC146818AP, ???, 900110003 V1.1 1986 MICROSOFT, FDC9268, NCR 53C80,
// CPU: 286, FPU socket proided, 12MHz/8MHz RAM: 6xSIMM30 (1.152MB - 6MB) - ROM: 64KB, 16KB Video BIOS - On board video: Paradise PEGA2A, 256KB video RAM
// On board: Floppy (2xint, 2xext), SCSI, par, ser, CP8, Microsoft Inport - OSC: 16.257MHz, 48MHz, 14.318180MHz - ISA16: 1, riser card with 1xISA8 and 2xISA16
ROM_START( micral45 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "vu12", "Version U1.2")
	ROMX_LOAD( "bm45_u1.2_p665.bin", 0x10000, 0x8000, CRC(046ab44a) SHA1(06e44b0bd8ae77c12319e11f629338651d53141d), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "bm45_u1.2_p664.bin", 0x10001, 0x8000, CRC(5729c972) SHA1(40b2dbc53829384e54cf953ed8b39e5d424bbff2), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "vu20", "Versio U2.0")
	ROMX_LOAD( "even.fil", 0x10000, 0x8000, CRC(438a7b36) SHA1(b5c9a71cfd7e87cc91453a73f17e93527c5ac7ac),  ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "odd.fil", 0x10001, 0x8000, CRC(9decd446) SHA1(dcbd305f065382f5327296391da388c50bb1b734),  ROM_SKIP(1) | ROM_BIOS(1) )
ROM_END

// Wang PC-250/16
// Phoenix 80286 ROM BIOS PLUS Version 3.10 07 / ROMBIOS Version 03.13.00 (c) Copyright Wang Laboratories, Inc. 1991
ROM_START( wpc250 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "wang_pc_250-16_bios_vers_03.13.00_chip_l47_9514rol_lo.bin", 0x10000, 0x8000, CRC(8f3a3061) SHA1(42b13f662f1f0b00748e21b4aa60cfcbc4b098c0))
	ROM_LOAD16_BYTE( "wang_pc_250-16_bios_vers_03.13.00_chip_l46_9514roh_hi.bin", 0x10001, 0x8000, CRC(3ba0cb84) SHA1(09111f9a6672fad58b11dc1f22240c78521bcc1c))

	// Wang PC-250/16 graphics card using a Chips F82C452
	ROM_REGION(0x8000, "gfx1", 0)
	ROM_LOAD( "wang_3050_bios_rom.bin", 0x0000, 0x8000, CRC(a895ad45) SHA1(afeacfffdf32f225c604d28580327e9ecfa96ea5))
ROM_END

// Philips PCD204 (PCD200 series)
// Chipset: Paradise PVGA1A-JK, Faraday FE3031-JK - BIOS: M1212 U66/U67 R1.00.01 CKS:056B/617D - Keyboard-BIOS: Phoenix 1072217
// BIOS-String: - - On board: Floppy, 1xIDE, VGA, Parallel, Serial - Slot for ISA slot adapter: 1 - HD: Maxtor 7060AT (C/H/S: 1024/7/17)
// OSC: 30.000MHz, 16.000MHz, 1.8432MHz, 28.3220MHz, 25.1750MHz, 10.000, 42.000MHz, 25.000MHz - CPU: Intel 80286-12
ROM_START( pcd204 ) // => emulation runs into hlt
	ROM_REGION16_LE(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "u66_mi212_r1.00.01.bin", 0x00001, 0x10000, CRC(e99f817a) SHA1(5cf8556fa4ef5c314d5450756c042f5e3cde09b4) )
	ROM_LOAD16_BYTE( "u67_mi212_r1.00.01.bin", 0x00000, 0x10000, CRC(d879f99f) SHA1(04c09b46c4a67701257f819d66002b8e93f0a391) )
ROM_END

// Leanord Elan High Tech 286 - Octek VGA-16 using Chips F82C451
// Chipset: Chips, passive backplane and slot CPU
// complains about "0000-55AA - Error Base RAM (64Kb) - Halt*"
ROM_START( elanht286 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "elan_s_ leanord_rom-bios_286_v3.50.bin", 0x10000, 0x10000, CRC(53dc0965) SHA1(13f352ee9eda008d8ddcc7ed06325dd2513ad378) )
ROM_END

// Kaypro 286i
ROM_START( k286i )
	ROM_REGION16_LE(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "81_1598", 0x18000, 0x4000, CRC(e25a1e43) SHA1(d00b976ac94323f3867b1c256e315839c906dd5a) )
	ROM_LOAD16_BYTE( "81_1599", 0x18001, 0x4000, CRC(08e2a17b) SHA1(a86ef116e82eb9240e60b52f76e5e510cdd393fd) )
ROM_END

// Sanyo MBC-28
ROM_START( mbc28 )
	ROM_REGION16_LE(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "mbc-28_sl-dt_ver.1620_low_din_checksum,454f00,27c256-15.bin", 0x10000, 0x8000, CRC(423b4693) SHA1(08e877baa59ebd9a1817dcdd27138c638edcbb84) )
	ROM_LOAD16_BYTE( "mbc-28_sl-dt_ver.1620_high_din_checksum,45ae00,27c256-15.bin", 0x10001, 0x8000, CRC(557b7346) SHA1(c0dca88627f8451211172441fefb4020839fb87f) )
ROM_END

// Siemens PCD-2 - Harddisk: NEC D5126
// CPU card W26361-D458-Z4-06-05, Piggyback MFM controller with WDC WD42C22A-JU PROTO chip W26361-D477-Z2-04-05
// Chips: Intel N82230-2, Intel N82231-2, WDC WD37C65BJM, VLSI 8831AM/X12012/VL16C452-QC
// VGA card: S26361-D463 GS3 using a Video Seven 458-0023
	// ROM_LOAD( "vga_nmc27c256q_435-0029-04_1988_video7_arrow.bin", 0x8000, 0x8000, CRC(0d8d7dff) SHA1(cb5b2ab78d480ec3164d16c9c75f1449fa81a0e7) ) // Video7 VGA card
	// ROM_LOAD( "vga_nmc27c256q_435-0030-04_1988_video7_arrow.bin", 0x8000, 0x4000, CRC(0935c003) SHA1(35ac571818f616b856da8bbf6a7a9172f68b3ab6) )
ROM_START( pcd2 )
	ROM_REGION16_LE(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "bios_tandon_188782-032a_rev_5.21_low.bin", 0x10000, 0x8000, CRC(a8fbffd3) SHA1(8a3ad5bc7f86ff984be10a8b1ae4542be4c80e5f) )
	ROM_LOAD16_BYTE( "bios_tandon_188782-031a_rev_5.21_high.bin", 0x10001, 0x8000, CRC(8d7dfdcc) SHA1(d1d58c0ad7db60399f9a93db48feb10e44ffd624) )

	ROM_REGION( 0x0800, "keyboard", 0 ) // reporting keyboard controller failure
	ROM_LOAD( "kbd_8742_award_upi_1.61_rev_1.01.bin", 0x000, 0x800, CRC(bb8a1979) SHA1(43d35ecf76e5e8d5ddf6c32b0f6f628a7542d6e4) ) // 8742 keyboard controller
ROM_END

// Siemens PCD-2M/-2L - Board : CPUAZ-S26361-D458-V30 + W26361-D458-Z4-09-05 (ISA16 slot CPU) - Chipset : Intel/Zymos N82230-2, Intel/Zymos N82231-2, VLSI VL16C452-QC
// BIOS : Copyright (C) 1985 Tandon Corporation, Al Rights Reserved. BIOS Version 5.2 - Keyboard BIOS: Award: UPI 1.61 REV 1.01 - CPU: Siemens SAB 80286-12-N/S, FPU socket provided
// RAM: 4xSIMM30 - DIP8: EN0, EN1, EN2, 256, 640, 1MB, MAP, COL - OSC: 90.0000, 24.0000, 14.31818, 16.0000 - On board: Floppy, 2xpar, 2xser, ISA16 piggyback connector
ROM_START( pcd2m ) // constant beeps, doesn't work
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "tandon188782-032a_rev_5.23_low.bin", 0x10000, 0x8000, CRC(1c922f7a) SHA1(f405ce0bf29c8e86efda964308e8f58f7ef0e5ca) )
	ROM_LOAD16_BYTE( "tandon_188782-031a_rev_5.23_high.bin", 0x10001, 0x8000, CRC(ee31d405) SHA1(2630d73ddd55e82857c5ff4547d69ad7f5d5d1ca) )
ROM_END

// Compaq SLT/286 - complains about "102 - System board failure" - CPU: Harris CS80C286-12
// Chips: Dallas DS1287, Compaq 109778-001/4758, Bt478KPJ35, S8852C4/DP8473V, Fujitsu 8904 Q16/109445-001, Fujitsu 8850 W00/109444-002, Compaq 19034/8846KK/10452-002
// NS16C450V, Fujitsu 8850 W73/110110-001
ROM_START( comslt286 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "compaq_slt286-bios-revision_j.2-even.bin", 0x10000, 0x8000, CRC(77e894e0) SHA1(e935e62e203ec67eaab198c15a36cc0078fd35b0))
	ROM_LOAD16_BYTE( "compaq_slt286-bios-revision_j.2-odd.bin", 0x10001, 0x8000, CRC(4a0febac) SHA1(7da5ac4bc50f25063a1d1e382b8cff9b297976f8))
ROM_END

// Dell System 200 - complains about "memory overlap at 400000" but seems to work otherwise
ROM_START( dsys200 )
	ROM_REGION16_LE( 0x20000, "bios", 0 )
	ROM_LOAD16_BYTE( "version_3.10_a12_even.bin", 0x10000, 0x8000, CRC(5aa81939) SHA1(d9029d3708c49e72f57ae2a340429c28ec39acab))
	ROM_LOAD16_BYTE( "version_3.10_a12_odd.bin", 0x10001, 0x8000, CRC(942416cb) SHA1(b321704471e159030af82556ff25ac46c27a807e))

	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "20575_b47-00.bin", 0x000, 0x0800, CRC(148187db) SHA1(0d7542dd0b2bc3d6724ae3618a8543cb84a30e92) )
ROM_END

//  NCR, probably PC-8 - should get a "NGA" extended CGA graphics card once it's emulated
ROM_START( ncrpc8 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "pc8main", "NCR PC-8 mainboard") // large full size AT mainboard - Setup Version 2.3
	ROMX_LOAD( "ncr_35117_u127_vers.4-2.bin", 0x10000, 0x8000, CRC(f4338669) SHA1(c1d6e714591c8d7ab966acfdbc3b463e06fbd073), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "ncr_35116_u113_vers.4-2.bin", 0x10001, 0x8000, CRC(b1b6a2e2) SHA1(5b2c0a2be59e064076ed757d84f61bf955ceca08), ROM_SKIP(1) | ROM_BIOS(0))
	// Chips: NCR 006-3500404, NCR 006-3500447D, NCR 006-3500402PT, M5L8042-235P, SN76LS612N
	ROM_SYSTEM_BIOS(1, "pc8card", "NCR PC-8 CPU card") // passive backplane and CPU card - Setup Version 2.1
	ROMX_LOAD( "ncr_u127-30_v.4.bin", 0x10000, 0x8000, CRC(33121525) SHA1(11f8d8af4dad432f558c646d7d0ff23eb642a815), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "ncr_u113-27_v.4.bin", 0x10001, 0x8000, CRC(87424492) SHA1(5b7aba5678fe55c81fee2e07730b8ae03a23160f), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD ("ncr_keyboard_mcu_35091.bin", 0x0000, 0x800, CRC(632556cc) SHA1(b35f30bd0664fc1c2775a594f248d1e30237900a))
ROM_END

// NCR Class 3302 - CPU: AMD N80L286-12/S, FPU socket provided - Chipset: Chips & Technologies NEAT (82C206, 82C211, 82C212, 82C215), VLSI VL16C452-QC, INMOS IMSG176J-50Z
// Motherboard: PN-386XV REV R4.B - RAM: SIMM30x4, On board: 8x4C4256DJ-10, 4x41C256-10 - BIOS: NCR 3.5 - Keyboard BIOS: M5L8042-277P
// OSC: 24.000MHz, 32.000MHz, 36.000MHz, 1.8432MHz, 25.175/28.322, 14.31818, ISA16: 1 on board, used for a riser with 2 slots
ROM_START( ncr3302 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "f000-flex_drive_test.bin", 0x10000, 0x8000, CRC(09c9eb6b) SHA1(5eb00f65659cee018726e7a4122da1c42b2bbef9))
	ROM_LOAD( "f800-setup_ncr3.5-013190.bin", 0x18000, 0x8000, CRC(31e6a1ba) SHA1(2ff7dc233d167775ec3641c7a4b2d891db5f8ba7))

	// on board Paradise VGA PVGA1A-JK
	// DIP switches (x8 near the Paradise PVGA1A-JK) are undocumented. Setting switch 7 to 'open' generates VGA compatible (yet monochrome) signal, closing switch 7
	// causes 'out of range' on a fixed frequency VGA LCD - Graphics RAM: 8xD6164, 8 empty sockets (18 pin) provided
	ROM_REGION(0x8000, "video", 0)
	ROM_LOAD( "c000-wd_1987-1989-740011-003058-019c.bin", 0x0000, 0x8000, CRC(658da782) SHA1(6addcf24795c2e8004c21a8e546b53de41766420))
ROM_END

// Nixdorf 8810 M30
// Chipset: Chips P82C211-12 P82C215, P82C212B-12, Zilog Z0853006VSC, L5A0757/NC-LSI56A-SCC1, Chips P82C604A, P82C206 H1
ROM_START( n8810m30 )
	ROM_REGION16_LE(0x20000, "bios", 0 )
	ROM_LOAD( "at286bios_53889.00.0.17jr.bin", 0x00000, 0x20000, CRC(74870212) SHA1(adb3f379c9aeee6a5beb946d23af6eea706aca1d) )
ROM_END

// Nixdorf 8810 M55 - Paradise PEGA 1A383048 piggybacked onto MFM/Floppy controller card
// Chips: M5L8042-235P, NCR 006-3500402PT, 2xAMD AM9517A-5JC, NCR 006-3500447 D, NCR 006-3500404
ROM_START( n8810m55 )
	ROM_REGION16_LE(0x20000, "bios", 0 )
	ROM_LOAD16_BYTE( "150-3872_u113_27_4.5.1.bin", 0x10001, 0x8000, CRC(35ff4fba) SHA1(557f0f98c27af76f6fa6990592e7150f5fc1fc02))
	ROM_LOAD16_BYTE( "150-3873_u127_30_4.5.1.bin", 0x10000, 0x8000, CRC(5a7e6643) SHA1(f3890919a772eead7232bd227b2c8677377f6e24))
ROM_END

// Olivetti M250E aka DECStation 220 - PVGA1A on board with the ROM included in the main ROM - Board: BA241 - Chips: 16C451, 8742, WD37C65, GA99, GA80
// CPU: 12MHz 80286 - FPU: 80287-2 socket provided - RAM: 1MB, 2MB, 4MB in SIMM modules - ROM: 64KB BIOS 1.07-1.09 - Mass storage: 2x 3.5" FDD HD,
// on board: AT/RLL controller - Shadow memory: E0000-FFFFF - Bus: 3xISA8/18 on riser board - Video: PVGA1A on board, 256KB RAM - Connectors: VGA, ser, par, PS/2 keyboard, PS/2 mouse
ROM_START( m250e )
	ROM_REGION16_LE(0x20000, "bios", 0 )
	ROM_LOAD( "biosglas.bin", 0x10000, 0x10000, CRC(83c33ce4) SHA1(0f132072dca06d8c7bb51b0f0d6304a74406cc8c))
ROM_END

// Olivetti M290 - uses the Olivetti GO481 Paradise PVGA1A-JK VGA card - locks up with "Error 2" and a key symbol
// Chipset: Olivetti GA099-B/28927F74AT, Olivetti GA098-B 28909F74AS, TI TACT82206FN, Olivetti 8920K5
// Floppy/IDE card: WD37C65BJM, NS16C450V
ROM_START( m290 )
	ROM_REGION16_LE( 0x20000, "bios", 0 )
	ROM_LOAD( "m290_pep3_1.25.bin", 0x10000, 0x10000, CRC(cb57d677) SHA1(4bdf5c52567c129b413c866c63b5fb3562fccd23))

	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "m290_csl0_1.10.bin", 0x000, 0x0800, CRC(d767d496) SHA1(84246f7b39e0a005425948931cf93624b831e121) )
ROM_END

// Ericsson WS286
ROM_START( ews286 ) // Computer is brown/yellow-ish with Ericsson logo
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "rys_103_1002_r8a_3c00_ic-pos_71.bin", 0x18000, 0x4000, CRC(af179e56) SHA1(58b1df46d6e68eef472a0529cb9317abaf17880f)) // Last ROM set and has Nokia
	ROM_LOAD16_BYTE( "rys_103_1003_r8a_8600_ic-pos_69.bin", 0x18001, 0x4000, CRC(555502cb) SHA1(1977fe54b69c5e52731bf3eb8bdabe777aac014b)) // copyright patched in both roms
ROM_END

// Nokia Data WS286
//ROM_START(nws286 ) // Computer is grey with Nokia logo.
//  ROM_REGION(0x20000,"bios", 0)
//  ROM_LOAD16_BYTE( "rys_103_1002_r8a_3c00_ic-pos_71.bin", 0x18000, 0x4000, NO_DUMP)
//  ROM_LOAD16_BYTE( "rys_103_1003_r8a_8600_ic-pos_69.bin", 0x18001, 0x4000, NO_DUMP)
//ROM_END


//**************************************************************************
//  80286 Notebook/Laptop/Portable
//**************************************************************************

// CAF Prolite 286/16 - Chipset: CHIPS P82C06, P82C211-12, P82C215-12, P82C212B-17 16MHz, WD47C65BJM, Acer M2201 - on board: beeper, floppy (FDC1.0 undumped)
// CPU: 286, FPU: Socket provided - RAM: 8xSIMM30 - BIOS: CAF Computer Corporation AMI BIOS - Keyboard BIOS: FONTEX TECH.CORP. AMI 286 BIOS PLUS KEYBOARD 07327
// OSC: 32.768KHz, 16MHz, 9.6MHz, 39.000MHz, 18.000000MHz, 16.257MHz, 1.8432MHz, 32.00MHz, 20MHz
// Video on board:  Genoa 7017417 BIOS (undumped), GRAY1.1 (undumped), Genoa GN006001-B, GN006002-B, GN006003-A, 256KB Video RAM - DIP10: off-on-on-off-on-on-on-off-off-off
ROM_START( prolite286 ) // Initialises graphics card, then dies
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "caf_prolite_even.bin", 0x10000, 0x8000, CRC(c3f4b360) SHA1(958fde7fa12425f6ac14fee6ebfd1b1f535c66eb))
	ROM_LOAD16_BYTE( "caf_prolite_odd.bin", 0x10001, 0x8000, CRC(7c2f6f9f) SHA1(6e72f1458308e521e5715cedb83f40ebe0cc4ad7))
ROM_END

// AEG Olympia Olyport 40-21 aka Zenith SuperSport - CPU: AMD N80L286-12/8 - Chipset: Chips P82C2185, P82C211C, P82C206 F-1, P82C212B, P82C604, WD37C65BFM, Hitachi HD6305VOP
// OSC: 22.500, 24.000 - Video: CGA, LCD with 16 grey intensities - Connectors: CRT, Ext. Bus, RS232C, Printer, Ext.FDD - Mass storage: FDD 1.44MB, HD: Conner CP-323 (IDE with detached controller PCB)
ROM_START( olyport40 ) // "+++ ERROR: Fatal Slushware RAM Error +++" / "--- Fatal Error: Cannot Continue! ---" - slushware is a ROM shadowing concept cropping up in Zenith brochures
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "mbevn_v2.7b.bin", 0x10000, 0x8000, CRC(eaf0d73c) SHA1(d642afe8a72b95e5e9d9cdc8cf8db833df54eaf0))
	ROM_LOAD16_BYTE( "mbodd_v2.7b.bin", 0x10001, 0x8000, CRC(a750d652) SHA1(c9a3cca535f6e7c44b83d87efcc289afda71b62f))

	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "nec_d80c42c.bin", 0x000, 0x0800, CRC(49ae4c38) SHA1(d5e6463d1dbcc7d68ef6b9222ff02102918f41b7))
ROM_END

// Compaq Portable II
// Chips: Intel D8742, SN76LS612N, 2x NEC D8237AC-5, 2xIntel P8259A-2, MC146818AP, Intel P8254
// Enhanced Color Graphics board: Chips P82C431, P82C434A
ROM_START( comportii )
	ROM_REGION16_LE(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0,"105620-001", "Ver. D (105620/105622)")
	ROMX_LOAD( "comportii_105622-001.bin", 0x18000, 0x4000, CRC(30804fa4) SHA1(204d16dac4db4df0ba23a336af62da3f66aa914c), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "comportii_105620-001.bin", 0x18001, 0x4000, CRC(45fe43e8) SHA1(f74c2e30f7bd162be4042946ebcefeb236bd2fe7), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1,"106437-001", "Ver. F (106437/106438)")
	ROMX_LOAD( "106438-001.bin", 0x18000, 0x4000, CRC(616361de) SHA1(ce1a6f9be9d374b76a83856f176aaa993d1dd46c), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "106437-001.bin", 0x18001, 0x4000, CRC(b50881ae) SHA1(2a79b39f77b0d3e94e4f765ed6c1961746dad563), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2,"109739-001", "Ver. P.1 (109739/109740)")
	ROMX_LOAD( "109740-001.rom", 0x18000, 0x4000, CRC(0c032f12) SHA1(3ae7833d7f92d6495e2e57caa0260b573187eb72), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "109739-001.rom", 0x18001, 0x4000, CRC(83698b85) SHA1(3d3cff84a747aea3db2612a7ac3ebe9cb4700b33), ROM_SKIP(1) | ROM_BIOS(2) )
ROM_END

// Compaq Portable III
// Chipset: Fujitsu MB672318, MB672316U, 2x Intel P8237A-5, Compaq 8731KX 104111-002, Intel 8272A, 2xAMD P8259A, Graphics: M77H010
// MC146818P, Intel D8742, Fujitsu MB672322
ROM_START( comportiii )
	ROM_REGION16_LE(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "106779-002", "106779-002")
	ROMX_LOAD( "cpiii_87c128_106779-002.bin", 0x18000, 0x4000, CRC(aef8f532) SHA1(b0374d5aa8766f11043cbaee007e6d311f792e44), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "cpiii_87c128_106778-002.bin", 0x18001, 0x4000, CRC(c259f628) SHA1(df0ca8aaead617114fbecb4ececbd1a3bb1d5f30), ROM_SKIP(1) | ROM_BIOS(0) )
	// ROM_LOAD( "cpiii_106436-001.bin", 0x0000, 0x0800, CRC(5acc716b) SHA1(afe166ecf99136d15269e44ebf2d66317945bf9c) ) // keyboard
	ROM_SYSTEM_BIOS(1, "109737-002", "109737-002")
	ROMX_LOAD( "109738-002.bin", 0x10000, 0x8000, CRC(db131b8a) SHA1(6a8517a771272edf16870501fc1ed94c7555ef45), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "109737-002.bin", 0x10001, 0x8000, CRC(8463cc41) SHA1(cb9801591e4a2cd13bbcc40739c9e675ba84c079), ROM_SKIP(1) | ROM_BIOS(1) )
ROM_END

// Nixdorf 8810 M15 Laptop - PC07 - boot from harddisk doesn't work
// Chipset: Faraday FE3020, FE3000A, FE3010EB, FE3030, NEC D65013GF280, Toshiba TC8566AF, MC146818A, NEC D65013GF328, D65013GF371, D65013GF356, NS16C450V, Yamaha V6366C-F, MEI DA7116AFPBW
ROM_START( n8810m15 )
	// ROM_LOAD("charagene_v1.1_daft2c2.bin", 0x00000, 0x4000, CRC(dd324efd) SHA1(67fd91277733596bfad8506dc92d9f776e563dda)) // CGA chargen

	ROM_REGION16_LE(0x20000, "bios", 0 )
	ROM_LOAD16_BYTE( "rbios_even_daft2a3.bin", 0x10000, 0x8000, CRC(790abf68) SHA1(fbdb5e628ee9a605c8c1485a3fbb67736ff03153))
	ROM_LOAD16_BYTE( "rbios_odd_daft2b3.bin", 0x10001, 0x8000, CRC(b09a812a) SHA1(c1b3321715260f9cd8c810325dc10c674ea05174))
ROM_END

// Nixdorf 8810 M16 Laptop - PC17 - CGA version - boot from harddisk doesn't work
// Chipset: Chips P82xxxx, Chips P82C211-12, P82C215, P82C212B-12, MX9007G/MX1 16C4522QC, WD37C65BJM, Yamaha V6366C-F
ROM_START( n8810m16c )
	// ROM_LOAD("201cg rev 1.0.u78", 0x00000, 0x4000, CRC(3e31143b) SHA1(489da357e0ab8a469a3fb81cce160637486c87bc)) // CGA chargen
	ROM_REGION16_LE(0x20000, "bios", 0 )
	ROM_LOAD16_BYTE( "nmc27c256.u35", 0x10000, 0x8000, CRC(51acd116) SHA1(1a0bf24af4eba48d0deb0132a523e131902d2bcd))
	ROM_LOAD16_BYTE( "nmc27c256.u36", 0x10001, 0x8000, CRC(fb47f9da) SHA1(d9bd4aea850a83764454a5c86c8da09f7c640fd6))
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "d8749h.u69", 0x000, 0x0800, CRC(030051da) SHA1(91b60228452cd1d6af99786402bd3b4d3efc2f05) )
ROM_END

// Nixdorf 8810 M16 Laptop - PC17 - VGA version - boot from harddisk doesn't work
// Chipset: MX8945G/MX16C4520C, Chps P82C212B-12, P82C215, P82C206, WD37C65BJM, P82C211-12, Chips F82C455,
ROM_START( n8810m16v )
	// ROM_LOAD("8810m16vga_27c256_221vb_123g1.bin", 0x00000, 0x8000, CRC(3bc80739) SHA1(3d6d7fb01681eccbc0b560818654d5aa1e3c5230)) // C&T VGA BIOS for 82C455
	ROM_REGION16_LE(0x20000, "bios", 0 )
	ROM_LOAD16_BYTE( "8810m16vga_27c256_286bios_a2531511_a.bin", 0x10000, 0x8000, CRC(1de5e49b) SHA1(759878e13801278de96700bbef318a49cca68054))
	ROM_LOAD16_BYTE( "8810m16vga_27c256_286bios_a2531511_b.bin", 0x10001, 0x8000, CRC(a65cf1f8) SHA1(30d46b49e87f272540e24a278848122b3c40bdaf))
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "8810m16vga_8749_201kb_rev3a.bin", 0x000, 0x0800, CRC(030051da) SHA1(91b60228452cd1d6af99786402bd3b4d3efc2f05) )
ROM_END


//**************************************************************************
//  80386 SX and DX BIOS
//**************************************************************************

ROM_START( at386sx )
	ROM_REGION16_LE(0x20000, "bios", 0 )
	// 0: NCR 386 CPU card - Chipset: TACT82301PB, TACT82302PB, TACT82303PB
	ROM_SYSTEM_BIOS( 0, "ncr386sx", "NCR 386sx card" ) // Upgrade card for e.g. NCR PC-8 - Setup Version 2.7.1
	ROMX_LOAD( "ncr_386sx_u12-19_7.3.bin", 0x10001, 0x8000, CRC(9e4c9a2a) SHA1(0a45d9f04f03b7ae39734916af7786bc52e5e917), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "ncr_386sx_u46-17_7.3.bin", 0x10000, 0x8000, CRC(73ad83a2) SHA1(bf6704fb4a0da37251f192cea3af2bc8cc2e9cdb), ROM_SKIP(1) | ROM_BIOS(0))
	// ROM_LOAD( "ncr_386sx_card_150-0004508_u1_v1.1.bin", 0x0000, 0x800, CRC(dd591ac1) SHA1(5bc40ca7340fa57aaf5d707be45a288f14085807))
	// 1: Dell 386SX-33 with 2x 72-pin SIMMs, ISA riser slot -  Chipset: VLSI 82C311, Cirrus Logic GD5420 - BIOS: 27C010 EPROM containing Quadtel VGA BIOS and Phoenix system BIOS 02/09/93
	// BIOS-String: Phoenix 80386 ROM BIOS PLUS Version 1.10 J01 - Copyrights by Phoenix and Dell - Jostens Learning Corporation 386/33SX - CPU: Intel 386sx
	ROM_SYSTEM_BIOS( 1, "dell386sx", "Dell 386sx" ) // emulate integrated VGA card
	ROMX_LOAD( "dell386.bin", 0x00000, 0x20000, CRC(d670f321) SHA1(72ba3a76874e0c76231dc6138eb56a8ca46b4b12), ROM_BIOS(1))
	// 2: A3286/A3886-01 COMP V4.0 - Chipset: Intel S82344A (VLSI), S82343 (VLSI) - BIOS: AMI P9 (386SX) BIOS 910520
	// BIOS-String: - 30-05T1-425004-00101111-050591-ITOPSX-0 / MULTITRONIC PERSONAL COMPUTER - Keyboard-BIOS: AMI P9(386SX) Keyboard BIOS 910520 - OSC: 8.000, 14.318180MHz, (unreadable) - CPU: Intel SMD, unreadable - ISA8: 1, ISA16: 5
	ROM_SYSTEM_BIOS( 2, "a3286a3886", "A3286/A3886-01 COMP V4.0")
	ROMX_LOAD( "386-a3286-a3886-01-even_32k.bin", 0x10000, 0x8000, CRC(56ed3332) SHA1(9d113e57228ee596c0c24eabb193d3670fb9a309), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "386-a3286-a3886-01-odd_32k.bin", 0x10001, 0x8000, CRC(9dbe4874) SHA1(589379055cfedd4268d8b1786491e80527f7fad5), ROM_SKIP(1) | ROM_BIOS(2))
	// 3: CPU/FPU: 386SX/486SLC - Chipset: ALD 93C308-A (93C206 ???)
	// BIOS-String: X0-0100-000000-00101111-060692-386SX-0 / CC-070794-P01
	ROM_SYSTEM_BIOS( 3, "ald93c308", "ALD 93C308" )
	ROMX_LOAD( "3ldm001.bin", 0x10000, 0x10000, CRC(56bab3c7) SHA1(6970bdc7407b4b57c8e1d493f9e3d9ae70671b9c), ROM_BIOS(3))
	// 4: BIOS: Phoenix; 01/15/88 - CPU: 386sx-16 - Chipset: Intel - BIOS-String: Phoenix 80386 ROM BIOS PLUS Version 1.10 02 / 386SX, ADI CORP.
	ROM_SYSTEM_BIOS( 4, "intel", "Intel chipset")
	ROMX_LOAD( "3iip001l.bin", 0x10000, 0x8000, CRC(f7bef447) SHA1(a6d34c3bf0de93c2b71010948c1f16354996b5ab), ROM_SKIP(1) | ROM_BIOS(4))
	ROMX_LOAD( "3iip001h.bin", 0x10001, 0x8000, CRC(f46dc8a2) SHA1(b6566fd761e2e6ec34b61ee3bb043ef62d696b5e), ROM_SKIP(1) | ROM_BIOS(4))
	// 5: BIOS-String: X0-0100-000000-00101111-060692-386sx-0 / Ver. 5.14 - continuous reset
	ROM_SYSTEM_BIOS( 5, "v514", "V. 5.14")
	ROMX_LOAD( "3zzm001.bin", 0x10000, 0x10000, CRC(f465b03d) SHA1(8294825dcaa254c606cee21db7c74f61c1394ade), ROM_BIOS(5))
ROM_END

// NEATsx chipset: Chips 82C811 CPU/Bus controller, 82C812 Page interleave/EMS memory controller, 82C215 Data/Address buffer and 82C206 Integrated Peripheral Controller
ROM_START( ct386sx )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: BIOS-String: ENSX-1131-0040990-K0 / AMI 386SX-BIOS / NEATSX V1.1 05-31-90
	ROM_SYSTEM_BIOS(0, "neatsx", "NEATsx 386sx")
	ROMX_LOAD( "012l-u25.bin", 0x10000, 0x8000, CRC(4ab1862d) SHA1(d4e8d0ff43731270478ca7671a129080ff350a4f),ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "012h-u24.bin", 0x10001, 0x8000, CRC(17472521) SHA1(7588c148fe53d9dc4cb2d0ab6e0fd51a39bb5d1a),ROM_SKIP(1) | ROM_BIOS(0))
	ROM_FILL(0x1e2c9, 1, 0x00) // skip incompatible keyboard controller test
	ROM_FILL(0x1e2cb, 1, 0xbb) // fix checksum
	// 1: VIP-M345000 NPM-16 - BIOS-String: Phoenix 80386 ROM BIOS PLUS Version 1.10 20 / AUVA - Keyboard-BIOS: M5L8042-165P
	// ISA8: 2, ISA16: 5, Memory: 1 - OSC: 32.000MHz, 14.31818
	ROM_SYSTEM_BIOS(1, "m345000", "VIP-M345000 NPM-16")
	ROMX_LOAD( "386-vip-m345000 a1_32k.bin", 0x10001, 0x8000, CRC(8119667f) SHA1(343221a9729f841eb23eafe5505f1216783e5550), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "386-vip-m345000 a2_32k.bin", 0x10000, 0x8000, CRC(ada1a375) SHA1(74128270aa8fed504e8785c5d490b0fa25cc3895), ROM_SKIP(1) | ROM_BIOS(1))
	// 2: DTK Corp. 386sx COMPUTER / DTK 386sx Chipset ROM BIOS Version 4.26 / #96120590N
	ROM_SYSTEM_BIOS(2, "dtk386sx", "DTK 386sx")
	ROMX_LOAD( "3cso001.bin", 0x10000, 0x10000, CRC(8a0e26da) SHA1(94aefc745b51015426a73015ab7892b88e7c8bcf), ROM_BIOS(2))
	// 3: Chipset is labeled SOLUTIONS 88C211, 88C212, 88C215, P82C206
	// BIOS-String: ENSX-1107-040990-K0 - CPU: 386SX-16
	ROM_SYSTEM_BIOS(3, "solutions", "SOLUTIONS NEATsx")
	ROMX_LOAD( "3som001l.bin", 0x10000, 0x8000, CRC(ecec5d42) SHA1(b1aaed408fe9c3b73dff3fa8b19e62600a49cdb2), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "3som001h.bin", 0x10001, 0x8000, CRC(85d64a86) SHA1(528506724668ea3aef6aa0bd8d68cfcaa58bf519), ROM_SKIP(1) | ROM_BIOS(3))
	// 4: Manufacturer/Identifier: ELITEGROUP COMPUTER SYSTEMS, INC. NEATSX REV 1.0. - Chipset: NEATsx: Chips P82C206, Chips P82C215, Chips P82C811, Chips P82C812
	// CPU: Intel NC80386sx-20, FPU socket provided - RAM: DIP 1MB (DIP sockets for up to 4MB), 4xSIPP30 - BIOS EPROMs: 2x S27C256 NEATsx-013 PLUS
	// Keyboard BIOS: 1988 AMI 1131 KEYBOARD BIOS PLUS - OSC: 32.0000MHz, 40.000MHz, 14.31818 - ISA8: 3, ISA16: 5
	ROM_SYSTEM_BIOS(4, "neatsx013", "NEATsx-013 PLUS") // initializes the graphics card, then dies
	ROMX_LOAD( "neatsx-013_l.bin", 0x10000, 0x8000, CRC(7cd4d870) SHA1(c7a5b629dadb43779939043ae4adb5e78c770dc3), ROM_SKIP(1) | ROM_BIOS(4))
	ROMX_LOAD( "neatsx-013_h.bin", 0x10001, 0x8000, CRC(388587d4) SHA1(8ae6f6b14a2f53438b6a02c4f032088edb2df484), ROM_SKIP(1) | ROM_BIOS(4))
ROM_END

ROM_START( at386 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: X0-0100-000000-00101111-060692-386SX-0 / AMIBIOS Ver 5.19a
	ROM_SYSTEM_BIOS(0, "ami386", "AMI 386")
	ROMX_LOAD( "ami386.bin",  0x10000, 0x10000, CRC(3a807d7f) SHA1(8289ba36a3dfc3324333b1a834bc6b0402b546f0), ROM_BIOS(0))
	// 1: Phoenix 80386 ROM BIOS PLUS Verson 1.10 (R22)
	ROM_SYSTEM_BIOS(1, "at386", "unknown 386")  // This dump possibly comes from a MITAC INC 386 board, given that the original driver had it as manufacturer
	ROMX_LOAD( "at386.bin",  0x10000, 0x10000, CRC(3df9732a) SHA1(def71567dee373dc67063f204ef44ffab9453ead), ROM_BIOS(1))
	// 2: BIOS-String: 30-0101-429999-00101111-050591-D90-0 / AMI TD60C BIOS VERSION 2.42B
	ROM_SYSTEM_BIOS(2, "amicg", "AMI CG")
	ROMX_LOAD( "amicg.1",        0x10000, 0x10000,CRC(8408965a) SHA1(9893d3ac851e01b06a68a67d3721df36ca2c96f5), ROM_BIOS(2))
	// 3:
	ROM_SYSTEM_BIOS(3, "msi386", "MSI 386") // MSI 386 mainboard, initializes graphics card, then hangs - Chipset: Chips P82A304, P82A303, P82A302C, 2xP82B305, P82C301C, P82A306A,
	ROMX_LOAD( "ami_386_msi_02297_even.bin", 0x10000, 0x8000, CRC(768590a0) SHA1(90c5203d78591a093fd4f54ceb8d9827f1e64f39), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "ami_386_msi_02297_odd.bin", 0x10001, 0x8000, CRC(7b1360dc) SHA1(552ccda9f90826621e88d9abdc47306b9c2b2b15), ROM_SKIP(1) | ROM_BIOS(3) )
	// 4
	ROM_SYSTEM_BIOS(4, "ami2939", "AMI2939") // no display
	ROMX_LOAD( "ami2939e.rom", 0x10000, 0x8000, CRC(65cbbd32) SHA1(d7d26b496f8e86f01722ad9f171a68f9fcdc477c), ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "ami2939o.rom", 0x10001, 0x8000, CRC(8db6e739) SHA1(cdd47709d6036fad4be40c15bff41752d831d4b8), ROM_SKIP(1) | ROM_BIOS(4) )
	// 5: NCR 386 slot CPU - Upgrade card for e.g. NCR PC-8 - set graphics card to CGA to see a "Timer One Error" message
	ROM_SYSTEM_BIOS(5, "ncr386", "NCR 386 CPU card") // Chipset: SN76LS612PN, 2xAM9517A-5JC, NCR 006-3500402PT M472018 8650A
	ROMX_LOAD( "ncr_386_card_04152_u44_ver5.0.bin", 0x10000, 0x10000, CRC(80e44318) SHA1(54e1d4d646a577c53c65b2292b383ed6d91b65b2), ROM_BIOS(5))
	// ROM_LOAD ("ncr_386_card_keyboard_04181_u27_ver5.6.bin", 0x0000, 0x800, CRC(6c9004e7) SHA1(0fe77f47ff77333d1ff9bfcf8d6d92193ab1f208))
	// 6: BIOS-String: Phoenix 80386 ROM BIOS PLUS Version 1.10 22
	ROM_SYSTEM_BIOS(6, "cbm386", "Commodore 386")
	ROMX_LOAD( "cbm-386-bios-lo-v1.022e-8100.bin", 0x10000, 0x8000, CRC(a054a1b8) SHA1(d952b02cc10534325c1c5aaa8b6dfb77bc20a179), ROM_SKIP(1) | ROM_BIOS(6))
	ROMX_LOAD( "cbm-386-bios-hi-v1.022e-d100.bin", 0x10001, 0x8000, CRC(b9541f3d) SHA1(e37c704521e85b07369d21b0521f4d1871c318dd), ROM_SKIP(1) | ROM_BIOS(6))
	// 7: BIOS-String: X0-0100-000000-00101111-060692-RC2018A-0 / Ver 1.4b / Texas Instruments 486 DLC [S3Q]
	ROM_SYSTEM_BIOS(7, "ti486dlc", "TI 486DLC") // board is equipped with a TI486DLC
	ROMX_LOAD( "ti_486dlc_rev.s3q.bin", 0x10000, 0x10000, CRC(39b150ed) SHA1(5fc96c6232dd3a066349d8e707e938af55893297), ROM_BIOS(7))
	// 8: BIOS-String: 305-3.2 000-00 - Chipset: TACT82206FN; Intel A82385-33 - Keyboard Controller: P/N: 191106-2 C/S E4F4 Rev. 1.4
	// Board with Tandon and Micronics stickers - BIOS: 192475-305A V305 3.2
	// ISA8: 2, ISA16: 5 - OSC: 14.31818 MHz - 66.0000 MHz, CPU: Intel 80386DX-33, FPU: Intel 80387DX-33
	ROM_SYSTEM_BIOS(8, "tanmic385", "Tandon/Micronics with 385")
	ROMX_LOAD( "386-micronics-09-00021-even_32k.bin", 0x10000, 0x8000, CRC(0d4f0093) SHA1(f66364a82c957862a0e54afc3a2f85f911adfd49), ROM_SKIP(1) | ROM_BIOS(8))
	ROMX_LOAD( "386-micronics-09-00021-odd_32k.bin", 0x10001, 0x8000, CRC(54195986) SHA1(f3536340ef1697763e5cd70d0de7bb9b2a4ecde9), ROM_SKIP(1) | ROM_BIOS(8))
	// 9: Biostar MB1325PM - Chipset: Chips P82C206 µIC MI9382 MI9381A
	// BIOS: AMI 386 BIOS - BIOS-String: 30-0101-D61223-00101111-050591-OPBC-F / MB-1325PM. - Keyboard-BIOS: AMI
	// CPU: AMD 386DX/DXL-25 - ISA8: 1, ISA16: 6, ISA8/Memory: 1
	ROM_SYSTEM_BIOS(9, "mb1325pm", "MB1325PM")
	ROMX_LOAD( "386-mb1325pm ok.bin", 0x10000, 0x10000, CRC(768689c1) SHA1(ce46b3baf3cd2586ffaccdded789a54583b73a3b), ROM_BIOS(9))
	// 10: Intel SSBC 386AT - Chipset: Intel P8237A, P8254; P8259A, SN74LS612N - BIOS: 380892-01 Rev. 1.04 U53 L - 380892-02 Rev. 1.04 U52 H
	// Keyboard BIOS: Intel 453775-001 - ISA8: 2, ISA16: 4, Memory expansion: 2 - OSC: 1.8432 MHz - 32.000 - 14.31818
	ROM_SYSTEM_BIOS(10, "ssbc386at", "Intel SSBC 386AT" ) // no display
	ROMX_LOAD( "386-intel-u53-l_32k.bin", 0x10001, 0x8000, CRC(5198a767) SHA1(03dd494e3a218c59c82ebd7b1dd16905bca30773), ROM_SKIP(1) | ROM_BIOS(10))
	ROMX_LOAD( "386-intel-u52-h_32k.bin", 0x10000, 0x8000, CRC(cedbad7a) SHA1(e1365f5a183a342fe58205679a512c4ccd2a705a), ROM_SKIP(1) | ROM_BIOS(10))
	// 11: BEK P405 clone - BIOS-String: 30-0201-428029-00101111-070791-OPWB-0 -  was found as a stray ROM - possibly from a 486 board
	ROM_SYSTEM_BIOS(11, "opwb", "OPWB")
	ROMX_LOAD( "opwb.bin",  0x10000, 0x10000, CRC(e7597fb6) SHA1(2f1eb88138b400cc3ad554d03e532b5d3b0b11ad), ROM_BIOS(11))
	// 12: unknown manufacturer, Logo could be a "J7" -  COPYRIGHT (C) 89 REV.C MADE IN TAIWAN - Chipset: Chips P82C206
	// BIOS: AMI 386 BIOS PLUS 896818 - BIOS-String: DINT-6102-091589-K0 - Keyboard-BIOS: AMI KEYBOARD BIOS PLUS 896819
	// CPU: Intel 386DX-25 FPU: i386DX-33 - OSC: 54.0000MHz, 16.000MHz, 14.31818MHz
	ROM_SYSTEM_BIOS(12, "386atj7", "386AT J7" )
	ROMX_LOAD( "386-big_ami_896818_even.bin", 0x10000, 0x8000, CRC(096e99c4) SHA1(29ff718362af4f5d7c0173f4de84290cec60dded), ROM_SKIP(1) | ROM_BIOS(12))
	ROMX_LOAD( "386-big_ami_896818_odd.bin", 0x10001, 0x8000, CRC(6f92634d) SHA1(e36d401975690043c5d5cb1f781036b319e57f37), ROM_SKIP(1) | ROM_BIOS(12))
	// 13: BIOS-String: DAMI-0000-040990-K0 - silkscreen on board: 17:35-2495-02 - 702430D - H8010-30 - ISA8:2, ISA16: 8 - OSC: 14.31818, 50.000MHz
	// Chipset:Laser 27-2024-00 4L40F1028, Laser 27-2025-00 4L40F1026, 2xKS82C37A, KS92C59A, KS82C54-10P - CPU: Intel 386DX-33, FPU socket provided
	ROM_SYSTEM_BIOS(13, "vt386vt", "VT386VT" )
	ROMX_LOAD( "vt386vt-702430d-rom0_32k.bin", 0x10000, 0x8000, CRC(00013ee6) SHA1(7fed0b176911a94e8127b01bb77445c78f283ff7), ROM_SKIP(1) | ROM_BIOS(13))
	ROMX_LOAD( "vt386vt-702430d-rom1_32k.bin", 0x10001, 0x8000, CRC(c817ec57) SHA1(acdd0e28cb4798059c02e1342da7efe3eaf2c5cb), ROM_SKIP(1) | ROM_BIOS(13))
	// 14: Micronics I-Cache 09-00021-L8949 - Chipset: Chips P82C206 - RAM: M500/385 Memory Board, Cache: 8xCXK5863P-25
	// BIOS-String: Phoenix 80386 ROM BIOS PLUS Version 1.10 10a - Keyboard-BIOS: Intel - CPU/FPU: sockets provided, empty
	// ISA8: 2, ISA16: 4, Memory: 1 - OSC: 66.0000MHz, unreadable
	ROM_SYSTEM_BIOS(14, "l8949", "L8949" )
	ROMX_LOAD( "386-micronics 09-00021-lo_32k.bin", 0x10000, 0x8000, CRC(3a8743e3) SHA1(42262f60cb655ab120d968dbf9eb03387424bf14), ROM_SKIP(1) | ROM_BIOS(14))
	ROMX_LOAD( "386-micronics 09-00021-hi_32k.bin", 0x10001, 0x8000, CRC(c7fce430) SHA1(e0d6e8dbb8b6d68bd92dab63a259d2c9293f5571), ROM_SKIP(1) | ROM_BIOS(14))
	// 15: BIOS-String: 30-0200-ZZ1453-00101111-050591-SCAT3.04-0
	ROM_SYSTEM_BIOS( 15, "kmxc02", "KMX-C-02" )
	ROMX_LOAD( "3ctm005.bin", 0x10000, 0x10000, CRC(cfba6b2a) SHA1(001642016a3c02b031d739bd8b0fcff9470e86d2), ROM_BIOS(15))
	// 16: BIOS: AMI; 11/11/92 - ISA16: 5 - CPU/FPU: Am386DX-40, IIT 3C87-40 - Chipset: FOREX FRX46C521, KS83C206Q
	// BIOS-String: 40-0G00-009999-00101111-111192-4X521-F
	ROM_SYSTEM_BIOS( 16, "frx521", "using the Forex FRX46C521" ) // no display
	ROMX_LOAD( "3fom001.bin", 0x10000, 0x10000, CRC(8fa851c8) SHA1(68ac21357558d98aee4e2ffb903791e4198e0dd0), ROM_BIOS(16))
	// 17: FOREX 386 Super DX System  S3B
	ROM_SYSTEM_BIOS( 17, "frxs3b", "Forex Super DX System S3B") // no display
	ROMX_LOAD( "3fom003.bin", 0x10000, 0x10000, CRC(4e164e0a) SHA1(dc2d08061c443a3e4ced3ab11f1fa094585cbbba), ROM_BIOS(17))
	// 18: BIOS: AMI; 06/06/92 - BIOS-String: 40-0101-001107-00001111-060692-OPWB4SXB-0 / OPTI-495SX (471WB) BIOS VER 1.0
	// cf. driver hot409 - BIOS is capable of detecting 386sx => 486DX2, this particular BIOS was sorted with the 386s on chukaev
	ROM_SYSTEM_BIOS( 18, "495sx", "OPTi 82C495SX")
	ROMX_LOAD( "3opm009.bin", 0x10000, 0x10000, CRC(2abe36eb) SHA1(d113527ebd06f0359f2decd4ac0c6202f982d45e), ROM_BIOS(18))
	// 19: BIOS-String: EEMI-0386-030891-K0 - Chipset: 88C311
	ROM_SYSTEM_BIOS( 19, "eemi", "EEMI")
	ROMX_LOAD( "3zzm002", 0x10000, 0x10000, CRC(c2a7ff22) SHA1(af2e321d3245ad839a41666917bb24cca0f7884d), ROM_BIOS(19))
	// 20: BIOS-String: 30-0300-ZZ1425-00101111-020291-ITOPDX / 23L-1-0000-00-00-0000-00-00-000-K0
	// 000-0-0000-00-00-0000-00-00-00-2
	ROM_SYSTEM_BIOS( 20, "topcat", "TOPCAT")
	ROMX_LOAD( "ami_386_vl82c330_even.bin", 0x10000, 0x8000, CRC(a6f3d881) SHA1(40672d58f79d232dbda9685b9aa20533029fbdfc), ROM_SKIP(1) | ROM_BIOS(20))
	ROM_CONTINUE( 0x10001, 0x8000 )
	// 21: BIOS-String: MR BIOS (tm) V1.35 - RAM Pattern Test Failed at 0F0000H
	ROM_SYSTEM_BIOS( 21, "mrv135", "MR BIOS V1.35")
	ROMX_LOAD( "mrbios_v1.35_opti324_4floppy.bin", 0x10000, 0x10000, CRC(9a21dcd3) SHA1(dcab673fd2df621839671ef8f6a2eff443de39df), ROM_BIOS(21))
	// 22: BIOS-String: SINT-1185-040990-K0 - Chipset: VIA SL9030, SL9010, SL9025, SL9020, SL9020, SL9350, SL9090A
	ROM_SYSTEM_BIOS( 22, "3vim002", "3VIM002")
	ROMX_LOAD( "3vim002l.bin", 0x10000, 0x8000, CRC(368b66df) SHA1(1bef1e8e1818513061f0c7cf3c731da360c8400b), ROM_SKIP(1) | ROM_BIOS(22))
	ROMX_LOAD( "3vim002h.bin", 0x10001, 0x8000, CRC(02dbb9fe) SHA1(cfce750a4a019c71e59011fb7a7d891b40f61c61), ROM_SKIP(1) | ROM_BIOS(22))
	// 23: BIOS-String: 30-0201-ZZ1343-00101111-050591-OPWB-0
	ROM_SYSTEM_BIOS( 23, "zz1343", "zz1343")
	ROMX_LOAD( "ami_zz1343.bin", 0x10000, 0x10000, CRC(f5464c1f) SHA1(cb069c3d1d322aa769d46749716a35259f78264a), ROM_BIOS(23))
	// 24: ID: AT046DX3-B2.1(PQFP) - CPU: AMD Am386DX/DXL-33, FPU socket provided - Keyboard BIOS: JETkey V3 - Chipset: ACC Micro 2168 F9217D103 SC00 333MHz
	// RAM: 8xSIMM30, Cache: 64KB - OSC: 14.31818, 66.660MHz - ISA8: 1, ISA16: 5
	// BIOS-String: 30-004-428002-00101111-070791-ACC2046-0
	ROM_SYSTEM_BIOS( 24, "acc386", "AT046DX3-B2.1")
	ROMX_LOAD( "acc386.bin", 0x10000, 0x10000, CRC(2177c9ac) SHA1(2e320dcd173137b9b0cbf92602e9b77398921aaf), ROM_BIOS(24))
ROM_END


//**************************************************************************
//  80386 SX and DX motherboard
//**************************************************************************

// 486MMBO4088 - Chipset : ETEQ ET486SLC2 A, P82C206 - BIOS : MR 386SX/86SLC BIOS V.1.61
// Keyboard-BIOS: AMI MEGA-KB-F-WP - CPU: TI 486 TX486SLC/E -33MAB / FPU: IIT XC87SLC-33 - RAM: 8xSIMM30, Cache: 3xISSI IS61C256AH-15N, 2xIS61C256A-20N
// OSC: 14.31818MHz, 66.666MHz - ISA16: 7
ROM_START( mmbo4088 ) // screen remains blank
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "eteq-mrb.bin", 0x10000, 0x10000, CRC(10267465) SHA1(9cc1e7e8b0ec6e0798229489dce5b2b0300bfbd8))
ROM_END

// ILON USA, INC. M-396B - Chipset: PC Chip (silk screen sanded off), KS82C6818A - BIOS: AMIBIOS - BIOS-string: 30-0500-ZZ1437-001001111-070791-PC CHIP-F
// Keyboard-BIOS: Regional HT6542 - CPU: i386sx-25, FPU socket provided - RAM: 4xSIMM30, 8x20pin DIP - OSC: 14.31818, 50.000MHz - ISA16: 6
ROM_START( ilm396b )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "m396b.bin", 0x10000, 0x10000, CRC(e46bd610) SHA1(e5899f126ae6478f4db238cd1db835e0d1877893))
ROM_END

// Prolink P386SX-25PW VER:2.00 - Chipset: OPTi 82C281, F82C206 - BIOS/Version: AMI V007B316 - Keyboard-BIOS: AMI KB-BIOS-VER-F - OSC: 14.31818MHz, 50.000000MHz
// CPU: AMD Am386SX-25, FPU: IIT 3C87SX-33 - RAM: 8xSIMM30, Cache: 4x28pin, 2x24pin - ISA16: 8 -
// BIOS-String: 30-0100-008003-00101111-042591-OPSX-0 / OPSX 386SX BIOS / P386SX/25 PW IVN 1.0 1991.7
ROM_START( p386sx25pw )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "p386sx25pw.bin", 0x10000, 0x10000, CRC(9cbebe61) SHA1(ef90c1a9cc7fc3accdac9738aaf519c1b3d8260d))
ROM_END

// ELT-P9 / Most likely ELT-386SX-160D - Chipset: NEC ELT3000A - CPU: Intel 80386sx-16 - RAM: 8xSIPP30, 1MB DIP - BIOS: Phoenix
// Keyboard BIOS: Intel P8242 - DIP6: 000000 - OSC: 14.31818MHz, 32.000MHz  - ISA8: 1, ISA16: 6, ISA8/RAM: 1
ROM_START( eltp9 ) // Phoenix 80386 ROM BIOS PLUS Version 1.10.20
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "386_676849_l.bin", 0x10000, 0x8000, CRC(ab477e5e) SHA1(78d784f9c320fe7206c2601c4dbb47a90e5cbf96), ROM_SKIP(1))
	ROMX_LOAD( "386_676849_h.bin", 0x10001, 0x8000, CRC(02241258) SHA1(2bd80bab573cd88829edfb85522e978e5e477806), ROM_SKIP(1))
ROM_END

// QTC-SXM KT X20T02/HI Rev.3 - Chipset: VLSI / Intel S82343 + S82344A - BIOS Version: QTC-SXN 03.05.07
// Keyboard BIOS: NEC Performance 204 (c) Quadtel 265 - CPU: i386sx-20, FPU: Intel 387sx
// RAM: 4xSIMM30, sockets for 1MB DIP - ISA8: 2, ISA16: 4 - OSC: 15.31818MHz, 16.000MHz, 40.0000
ROM_START( ktx20t02 ) // BIOS-String: Quadtel VL82C286 386SX BIOS Version 3.05.07
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "qtc-sxm-even-u3-05-07.bin", 0x10000, 0x8000, CRC(21f2ca4c) SHA1(7131de700cb95c825e61d611001bab1d3d3bb195), ROM_SKIP(1))
	ROMX_LOAD( "qtc-sxm-odd-u3-05-07.bin", 0x10001, 0x8000, CRC(1543d0f7) SHA1(12281ef81d7aabf291586f96b678074216f0c23a), ROM_SKIP(1))
ROM_END

// PC-CHIPS M317 - Chipset: PC Chip CHIP 5 (14L40F2054) / CHIP 6 (14L50F2053), UMC 82C206F - CPU: 80386, FPU socket privided
// BIOS: AMI 386 BIOS PLUS - Keyboard BIOS: AMI Keyboard BIOS Plus - RAM: 8xSIMM30 - Cache: 64K - solder pads for a memory card connector
// ISA8: 2, ISA16: 6 - OSC: 66.6670MHz, 14.318MHz
ROM_START( pccm317 ) // 40-0300-ZZ1347-00101111-031591-UMCWB-0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "386_pcchips.bin", 0x10000, 0x10000, CRC(8a6046ea) SHA1(1e6c5dacdb5f9a36e558e44466bcd9182d849932))
ROM_END

// ZEOS 386 SX-16 - Chipset: VLSI VL82C201-16QC, 82C202, 82C203, 82C204, 82C205A, 82C100
// DVLX-6099-091589-K0 - AMI 386 BIOS for ZEOS INTERNATIONAL, LTD. / ST. PAUL, MINNESOTA 55112 USA
// (C)1987 American Megatrends Inc.386-BIOS (C) 1989 AMI, for ZEOS INTERNATIONAL - CPU: Intel 386sx-33, FPU sockets provided
// RAM: 36x16/18pin - ISA8: 2, ISA16: 6 - OSC: 64.000000, 14.31818
ROM_START( zeos386sx )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "zeos_386_16 even.bin", 0x10000, 0x8000, CRC(9897f140) SHA1(b01de83d438ceda4a80e92dc1fc7d725323427b6), ROM_SKIP(1))
	ROMX_LOAD( "zeos_386_16 odd.bin", 0x10001, 0x8000, CRC(0f930857) SHA1(c63670eb336f1998532a4f601d1845902be279e7), ROM_SKIP(1))
ROM_END

// Packard Bell PCB-303 Rev.01 - Chipset: ACC Micro 2036 25MHz, UM82C862F - CPU: i386sx-25, FPU socket provided - RAM: 6xSIMM30 - ISA16: 1
// VGA on board: OTI 512KB - on board: Floppy, IDE, ser, par, PS/2 mouse and keyboard - OSC: 50.000MHz x2
ROM_START( pcb303 ) // Phoenix 80386 ROM BIOS PLUS Version 1.10 2715 - 19920411102517 - Error 8602 - Auxiliary Device Failure
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-packard-pcb-303.bin", 0x00000, 0x20000, CRC(bbb18c76) SHA1(94f3785c3b96dfe7ab53a97d097bfb17c57229b7))
ROM_END

// Philips P3239 (aka Headstart / MaxStation / Magnum / Professional 1200, 48CD, 1600, 64CD, P160, SR16CDPhilips 5107-100-36154 (motherboard),
// 5107-200-35452 (CPU card) - Chipset: VLSI VL82C311-FC2, VL82C107-VC, WD37C65CJM
// CPU (on card): Intel 386sx-20, FPU socket provided - BIOS: M27C1001, 48805 P3239 - on board: 2xser, par, VGA, Floppy, IDE
// RAM: 1MB, 1xSIMM72 - VGA on board: CL-GD5325-40QC-A, MUSIC TR9CI710-50DCA, 256KB - OSC: 9.600, 1.8432MHz, 14.31818, 16.000, 40.000 (on CPU card)
// DIP4: 0001
ROM_START( php3239 ) // no display, beep code sounds like morse
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-philips_p3239.bin", 0x00000, 0x20000, CRC(9178e136) SHA1(e3bad39fcf028f459516d4e9a895035891eb801e))
ROM_END

// SOYO 386DX Baby-AT - Chipset: ETEQ ET82C4901 C, ET82C4903 A, Chips P82C206 H1 - CPU: Am386DX/DXL-40, FPU: Cyrix 387DX+
// RAM: 8xSIMM30, Cache: 8x5C6408-12, 1x ATT7C185P-15 - BIOS: AMI 386 BIOS 336468 - BIOS-String: 41-0100-001102-00101111-121291-BENGAUTO-F - REV: C *** 40MHZ ***
// Keyboard BIOS: AMI KB BIOS-VER-F - ISA8: 1, ISA16: 6 - OSC: 8.000MHz, 80.000MHz
ROM_START( sybaby386 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "386dx.bin", 0x10000, 0x10000, CRC(9fcfd21f) SHA1(5e7b42be55a45b0b51d2a7e8bd8d6405dbe69129))
ROM_END

// Diamond Flower International 386SX-16/20CN Rev 1.0 - Chipset : CHIPS P82C206, P82C215A, P82C812,P82C811 - BIOS: AMI
// CPU: Intel 80386SX-20, FPU socket privided - RAM: 8xSIMM30 - ISA8: 2, ISA16: 6
ROM_START( dfi386sx ) // Hangs after initialising the graphics card
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "dfi_386sx_16cn_20cn_27c256_low.bin", 0x10000, 0x8000, CRC(58456dc1) SHA1(51f7052366106fe564aedb99ddf6974f6ad4c5cc), ROM_SKIP(1))
	ROMX_LOAD( "dfi_386sx_16cn_20cn_27c256_high.bin", 0x10001, 0x8000, CRC(1ab36d4b) SHA1(640af1c333255cca85543bc369cb0cede45e1ef6), ROM_SKIP(1))
ROM_END

// 3SIUD-1.1 - CPU: AMD Am386SX/SXL-25 - Chipset: SiS 85C206, UMC (unreadable) - RAM: SIMM30x4, 8x20pin, 4x16pin
// BIOS: AMI 386SX BIOS 70167 - Keyboard-BIOS: NEC KB-BIOS VER7 - ISA8: 1, ISA16: 5 - OSC: 50.000000MHz, 14.31818MHz
// BIOS-String: 30-0200-ZZ1266-00101111-050591-UMC386SX-0 / 3SIUD-1.0
ROM_START( 3siud )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "3siud_bios.bin", 0x10000, 0x10000, CRC(24fa8491) SHA1(635ce3db872e39d84f299356b960b0a16e2cf082))
ROM_END

// AMI 386 BABY SCREAMER - BIOS: AMI MARK V BABY SCREAMER - Chipset: VLSI VL82C331-FC, VL82C332-FC, Megatrends MG-9275, Chips ??? - OSC: 14.31818, 66.666, 24.000MHz
// BIOS-String: 40-0301-000000-00101111-070791-SCREAMER-0 / BIOS RELEASE 42121691 - On board: 2xserial, parallel, floppy, 1xIDE
ROM_START( amibaby )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROMX_LOAD( "ami_mark_v_baby_screamer_even.bin", 0x00000, 0x10000, CRC(50baacb7) SHA1(c9cb6bc3ab23f35050a7f079109005331eb5de2c), ROM_SKIP(1))
	ROMX_LOAD( "ami_mark_v_baby_screamer_odd.bin", 0x00001, 0x10000, CRC(42050eed) SHA1(c5e1ed9717acb2e3adcb388ccecf90a74d495132), ROM_SKIP(1))
ROM_END

// AUVA TAM/25-P2 M31720P - Chipset: µC M19382, M19381A, Chips - CPU: 386DX 25Mhz - BIOS: DA058290 - Keyboard-BIOS: A179859
// BIOS-String: 30-0101-D81105-00101111-050591-OPBC-0 / AUVA 386 TAM/25/P2(P2,A1,A2), 01/10/1992- ISA8: 1, ISA16: 6, ISA16/Memory: 1
ROM_START( tam25p2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "tam25-p2.bin", 0x10000, 0x10000, CRC(0ea69975) SHA1(cb7f071a36653cf4f00a8b158a4900efb8f8b8e8))
ROM_END

// Elitegroup ELT-386SX-160BE - Chips P82C206 - CPU: Intel 386sx-16, FPU: socket provided - BIOS:Phoenix 679006 - Keyboard-BIOS: Intel P8242/Phoenix
// ISA8: 2, ISA16: 5 - OSC: 14.31818MHz, 32.000MHz - BIOS-String: Phoenix 80386 ROM BIOS PLUS Version 1.10 22 / ELT-386SX(P9)
ROM_START( elt386sx )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "elt386l.bin", 0x10000, 0x8000, CRC(95fd5508) SHA1(a66cd78f52f3931c6f8486db0d39f4e55244dcea), ROM_SKIP(1))
	ROMX_LOAD( "elt386h.bin", 0x10001, 0x8000, CRC(90c0597a) SHA1(b67b39662a0bb8c0cde1635d3fd3c1f9fbaad3c0), ROM_SKIP(1))
ROM_END

// TD70N motherboard - Chipset: Citygate D100-011 - ISA16: 6 - Keyboard-BIOS: JETkey V5.0 - CPU/FPU: Am386SX/SXL-33, i387SX
ROM_START( td70n )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: BIOS: AMI, Version 3.10 -  BIOS-String: 20-0100-009999-00101111-060692-CGD90-F / TD70N BIOS VERSION 3.10
	ROM_SYSTEM_BIOS( 0, "td70nv310", "TD70N V3.10" )
	ROMX_LOAD( "3cgm001.bin", 0x10000, 0x8000, CRC(8e58f42c) SHA1(56e2833457424d7176f8360470556629115493df), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_CONTINUE( 0x10001, 0x8000 )
	// 1: BIOS: AMI, Version 3.23T - BIOS-String: 20-0100-009999-00101111-060692-CGD90-F / BIOS VERSION 3.23T
	ROM_SYSTEM_BIOS( 1, "td70nv323", "TD70N V3.23T" )
	ROMX_LOAD( "3cgm002.bin", 0x10000, 0x8000, CRC(bca54fd8) SHA1(35b568c675e58965074162a93cf04918fc8d240f), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_CONTINUE( 0x10001, 0x8000 )
ROM_END

// TD70A and TD70AN motherboards - Chipset: Citygate D110-014, KS83C206Q - ISA8: 1, ISA16: 5 - Keyboard-BIOS: JETkey V5.0 - CPU: Am386SX-40
ROM_START( td70a ) // 8042 GATE-A20 ERROR - SYSTEM HALTED
	ROM_REGION16_LE(0x20000, "bios", 0)
	// BIOS: AMI, Version 2.60 - BIOS-String: 20-0100-009999-00101111-060692-CGD90-F / TD70A BIOS VERSION 2.60
	ROM_SYSTEM_BIOS( 0, "td70a", "TD70A" )
	ROMX_LOAD( "3cgm003.bin", 0x10000, 0x8000, CRC(1a92bf18) SHA1(520cd6923dd7b42544f8874813fbf81841778519), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_CONTINUE( 0x10001, 0x8000 )
	// 1: BIOS-String: 20-0100-009999-00101111-060692-CGD90-F / TD70A BIOS VERSION 2.60G
	ROM_SYSTEM_BIOS( 1, "td70an", "TD70AN")
	ROMX_LOAD( "bios.bin", 0x10000, 0x8000, CRC(0924948b) SHA1(e66b5223a7fb0b3ddb30ad0873ff099abf331262), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_CONTINUE( 0x10001, 0x8000 )
ROM_END

// MORSE KP 386SX V2.21 - Chipset: MORSE 91A300 (sticker), UMC UM82C206L - BIOS: AMI 386SX BIOS (Ver. 2.10) C-1216 - ISA8: 2, ISA16: 6
// BIOS-String: - 30-0200-ZZ1216-00101111-050591-386SX-0 - Keyboard-BIOS: AMI KEYBOARD BIOS PLUS C-1216 - CPU: AM-386SX/SXL-25, FPU: iN80287-12 - OSC: 8.000, 14.31818, 50.000 MHz
ROM_START( mokp386sx )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-morse_kp386sx.bin", 0x10000, 0x10000, CRC(f3a9c69f) SHA1(6e028a11f3770d7cda814dfa698f2ab5d6dba535))
ROM_END

// ID: Morse KP920121523 V2.20 - Chipset: UMC UM82C206L MORSE 91A311 91A310 - CPU: AM386DX/DXL-40, FPU socket provided - RAM: 8xSIMM30, Cache: 32K/64K/128K
// BIOS: AMI 386DX BIOS Ver. 2.11 C-1216 - Keyboard BIOS: KB-BIOS-VER F MEGATRENDS - OSC: 14.31818MHz, 80.000MHz - ISA16: 8, solder pads for a memory extension card
ROM_START( mokp386 ) // BIOS-String: 30-0200-ZZ1216-00101111-050591-KP386DX-0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-morse-kp920121523.bin", 0x10000, 0x10000, CRC(9392e265) SHA1(1df163bef6cf73a1d2a40ac997b96f93d1f2f4d1))
ROM_END

// UNICHIP 386W 367C REV 1.0 - Chipset: UNIchip U4800-VLX/9351EAI/4L04F1914, HMC HM82C206 - CPU: AM386DX-40, FPU socket provided - ISA8: 1, ISA16: 5 - OSC: 14.31818
ROM_START( uni386w )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS: AMI 386C BIOS 2116295 -
	// BIOS-String: 40-0400-001107-00101111-111192-U4800VLX-0 / UNICHIP BIOS VER 2.0A 09/27/1993 - Keyboard-BIOS: AMI 386C BIOS KEYBOARD 2116295 -
	ROM_SYSTEM_BIOS(0, "ver20a", "Ver. 2.0A")
	ROMX_LOAD( "386-2116295.bin", 0x10000, 0x10000, CRC(7922a8f9) SHA1(785008e10edfd393dc39e921a12d1a07a14bac25), ROM_BIOS(0))
	// 1: AMI, 367 UNICHIP 386 BIOS VER 1.0 (1886636) / BIOS-String: 40-0300-001107-00101111-111192-U4800VLX-0
	ROM_SYSTEM_BIOS(1, "ver10", "Ver. 1.0")
	ROMX_LOAD( "3ucm002.bin", 0x10000, 0x10000, CRC(9f2e19da) SHA1(ef64c6ad9d02db849d29e3b998ca42b663656bad), ROM_BIOS(1))
ROM_END

// SCsxAIO - Chipset: Chips 82C236 (SCATsx), Acer M5105 A3E - On board: 2xCOM, Floppy, ISA
// BIOS-String: Peacock 386sx Ver. 2.0 24.03.92 30-0000-D01131-00101111-070797-SCATsx-8 - ISA16: 6
ROM_START( scsxaio )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-peacock_scsxaio.bin", 0x10000, 0x10000, CRC(54c3cacd) SHA1(b3c821b30052d0c771b5004a3746eb2cfd186c79))
ROM_END

// Shuttle 386SX REV 2.0A - Chipset: KU82335 SX042, Intel N82230-2 (Zymos); Intel N82231-2 (Zymos), BIOS: AMI 80386SX BIOS PLUS Ser #039747
// BIOS-String: - DINT-1216-073089-K0 / 386-BIOS AMI for MORSE 386SX Personal Computer
// Keyboard-BIOS: AMI 386 Keyboard BIOS PLUS Ser.# 039747, CPU: unreadable (SMD), FPU: empty socket - OSC: 32.000 MHz, 14.31818 - ISA8: 2, ISA16: 6
ROM_START( sh386sx20 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "386-shuttle386sx-even_32k.bin", 0x10000, 0x8000, CRC(8b0c3d96) SHA1(73b6315928161a013cfe81b226606dfae5a8ef94), ROM_SKIP(1) )
	ROMX_LOAD( "386-shuttle386sx-odd_32k.bin", 0x10001, 0x8000, CRC(9c547735) SHA1(3cef5290324aab9d7523e98bf511eaea351e580d), ROM_SKIP(1) )
ROM_END

// Alaris Cougar - Chipset: OPTi 82C499 - ISA16: 5, ISA16/VL: 2
// BIOS: MR BIOS (r) V1.65 - CPU: 75MHz IBM Blue Lightning
ROM_START( alacou )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "cougrmrb.bin", 0x10000, 0x10000, CRC(c018f1ff) SHA1(92c4689e31b367baf42b12cad8800a851cc3e828))
ROM_END

// Alaris Tornado 2 - CPU: 486 - Chipset: Opti/EFAR/SMC - ISA16: 4, PCI: 3, ISA16/VL: 2 - On board: Floppy, 1xIDE, parallel, 2xserial
ROM_START( alator2 ) // unknown beep code LH-HL
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "tornado2.bin", 0x00000, 0x20000, CRC(2478136d) SHA1(4078960032ca983e183b1c39ae98f7cdc34735d0))
ROM_END

// DTK PEM 2530 - Chipset: VLSI 9032BT/217203/VL82C100-0C
// Board's original ROMs were damaged (Datatech dtk 386 V4.26 A1763), "original" ROMs came from another user, V3.10 ROMs from a different board
// ISA8: 2, ISA16: 5, Memory connector: 1 - OSC: 40.000 MHz - 14.31818 MHz
ROM_START( pem2530 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// Phoenix 80386 ROM BIOS PLUS Version 1.10 01 KENITEC TECHNOLOGIES
	ROM_SYSTEM_BIOS(0, "pem2530ori", "DTK PEM 2530 original")
	ROMX_LOAD( "386-dtk_pem-2530_bios-low.bin", 0x10000, 0x8000, CRC(d9aad218) SHA1(a7feaad2889820852e3543229b0b103288470732), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "386-dtk_pem-2530_bios-high.bin", 0x10001, 0x8000, CRC(550c4d77) SHA1(05aba1a98e738f9b706b5a8f09b5b6c86bd336e2), ROM_SKIP(1) | ROM_BIOS(0))
	// 80386 BIOS Version 3.10 Rev. 2.06 (BIOS not original, works in PEM 2530)
	// additional info from chukaev.ru54.com: CPUBT-S26361-D548 (Siemens?) - Chipset: VL82C320A, VL82C331, VL16C452B - CPU: NG80386SX-20, socket for 80387SX
	ROM_SYSTEM_BIOS(1, "pem2530", "DTK PEM 2530")
	ROMX_LOAD( "386-dtk pem-2530-high_32k.bin", 0x10000, 0x8000, CRC(56a822c0) SHA1(b65797c0f87a0815b393758af9c059e6d7172ae9), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "386-dtk pem-2530-low_32k.bin", 0x10001, 0x8000, CRC(8688d883) SHA1(c3034c8b343786cb89de48fb2f4992160414f89e), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

// SM 386-40F - MR BIOS (r) V1.40 - Ver: V1.40-FORX300
// Chipset: SIS 85C206, FOREX FRX36C200, FOREX FRX36C300
ROM_START( sm38640f )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "forex300.bin", 0x10000, 0x10000, CRC(8d6c20e6) SHA1(cd4944847d112a8d46612e28b97e7366aaee1eea))
ROM_END

// Soyo SY-019H and SY-019I BIOS-String: 30-0200-DH1102-00101111-070791-ETEQ386-0 / REV C3
// Chipset: SIS 85C206, ETEQ ET82C493, ET82C491
ROM_START ( sy019hi )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ami_soyo_sy-19i.bin", 0x10000, 0x10000, CRC(369a040f) SHA1(3dbcbcb8b8a50717cae3b17f44ca1b7c394b75fc))
ROM_END

// PC-Chips M321 - RAM: 8xSIMM30, Cache: 8 sockets, 4 sockets occupied by CY71C199-25PC - CPU: AM386-DX40, FPU: socket provided - OSC: 80.000MHz, 14.31818
// Chipset: PCChips C206/306, CHIP6/4L04F1666, CHIP5/4L04F1282 (rev. 2.3, 2.5 and 2.7 boards) - ISA8: 2, ISA16: 6
ROM_START( pccm321 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// BIOS-String: 30-0201-ZZ1347-00101111-050591-M320-0
	ROM_SYSTEM_BIOS(0, "m321_23", "PCChips M321 Rev.2.3") // also on a rev. 2.5 board with C&T J38600DX-33, ULSI MathCo-DX33
	ROMX_LOAD( "pcchips_m321_rev2.3.bin", 0x10000, 0x10000, CRC(ca0542e4) SHA1(8af9f88e022f8115708178c6c0b313ea0423a2b5), ROM_BIOS(0) )
	// BIOS-String: 30-0100-001437-00101111-060692-PC CHIP-0
	ROM_SYSTEM_BIOS(1, "m321_27_1", "PCChips M321 Rev.2.7 #1")
	ROMX_LOAD( "3pcm002.bin", 0x10000, 0x10000, CRC(0525220a) SHA1(5565daea1db67fb2e6f5e7f5ddf5333569e74ab3), ROM_BIOS(1) )
	// BIOS-String: 30-0100-001437-00101111-060692-PC CHIP-0 - TRANS-AMERITECH ENTERPRISES, Inc.
	ROM_SYSTEM_BIOS(2, "m321_27_2", "PCChips M321 Rev.2.7 #2")
	ROMX_LOAD( "3pcm004.bin", 0x10000, 0x10000, CRC(d7957833) SHA1(b512d9fc404c4282fb964444aa70a9760edad7db), ROM_BIOS(2) )
	// BIOS-String: 30-0100-ZZ1437-00101111-070791-PC CHIP-0
	ROM_SYSTEM_BIOS(3, "m321_27_3", "PCChips M321 Rev.2.7 #3")
	ROMX_LOAD( "pcchips_m321_rev2.7_2.bin", 0x10000, 0x10000, CRC(1c529364) SHA1(a0cb0dc31b34377024efb3124f4167a8e1d748e6), ROM_BIOS(3) )
ROM_END

// Morse M3 V3.00 - Chipset: Morse 4L04F1282 + 4L04F1666 + 92A206S / possibly a rebranded OPTi 82C391 chipset - CPU: Am386DX/DXL-33,
// CPU (80386DX) and FPU socket (W3167) provided - RAM: 8xSIMM30, Cache: 4xMOSEL MS62256-A-25NC, 1xT6C6408-20 - BIOS: AMI 386DX BIOS PLUS Ver.3.00 386DX C-1216
// BIOS-string: 30-0201-ZZ1216-00101111-050591-386DX-0 - Keyboard-BIOS: AMI KEYBOARD BIOS PLUS C-1216 - OSC: 66.667MHz, 14.31818MHz - ISA8: 1, ISA16: 6
ROM_START( mom3v3 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "morse_m3_v3.00.bin", 0x10000, 0x10000, CRC(4748a084) SHA1(169696ea08f25e05928c48cc8328c67c51789f0d))
ROM_END

// PC-Chips M326
// Chipset: SARC RC4018A4/9324 and SARC RC6206A4/9408-AHS or SARC RC4018A4/9324 and RC4919A4-9323 (v5.5 board) or SARC RC2016A4-9320 and RC4019A4-9324 (v5.3)
ROM_START( pccm326  )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: Award Modular BIOS 4.50
	ROM_SYSTEM_BIOS(0, "pccm326", "PCChips M326 V5.2") //  BIOS reports a 66MHz 386DX original board has a TI TX486DLC/E-40PCE and IIT 4C87DLC-40 CPU/FPU combo
	ROMX_LOAD( "m326_v5.2_m601-326.bin", 0x10000, 0x10000, CRC(cca6a443) SHA1(096c8bfa000c682d6c801da27c7fd14243ebb63b), ROM_BIOS(0) )
	// 1: BIOS-String: 40-0100-001437-001001111-080893-4386-0 / Release 10/01/93 - also on an "M601 Rev. 1.3" board with a i486DX-33 (BIOS AMI AB0077440 - Keyboard-BIOS: Regional HT6542)
	ROM_SYSTEM_BIOS(1, "m326r53", "PC-Chips M326 Rev. 5.3")
	ROMX_LOAD( "m326_rev.5.3.bin", 0x10000, 0x10000, CRC(6c156064) SHA1(362ce5a2333641083706a878b807ab87537ca1e6), ROM_BIOS(1) )
	// 2: BIOS: AMI; 08/08/93; Release 10/01/93
	ROM_SYSTEM_BIOS(2, "m326", "M326") // no display
	ROMX_LOAD( "3sam005.bin", 0x10000, 0x10000, CRC(f232cd4b) SHA1(e005aa3a7d160223fb2912cf2cd5cc8af49e79a5), ROM_BIOS(2) )
ROM_END

// Elitegroup UM386 Rev. 1.1 - UMC UM82C482AF, UM82C391A, UM82C206F - OSC: 80.000MHz, 14.31818MHz
// BIOS: AMI-1131 E-91844945 - Keyboard-BIOS: AMI KEYBOARD-BIOS-VER-F / Intel P8942AHP
// RAM: SIMM30x8, Cache: 9xW2464AK-20, 1x ISA8, 7xISA16 - CPU: AM386DX/DXL-40
ROM_START( ecsum386 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// BIOS-String: 30-0500-D01131-00101111-070791-UMCWB-0 / UM386 V1.1 03-06-92
	ROM_LOAD( "ami_um386_rev1.1.bin", 0x10000, 0x10000, CRC(81fe4297) SHA1(efb2ba2be6f08cb487ee1b867a2456ed6b5975ad))
ROM_END


// ***** 386sx motherboards using the ALi 1217 chipset

ROM_START( alim1217 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	//0: BIOS-String: 30-0100-ZZ1453-00101111-070791-ACER1217-0 / CPU: 386SX-40
	ROM_SYSTEM_BIOS( 0, "m121701", "ALi M1217 #1" )
	ROMX_LOAD( "3alm005.bin", 0x10000, 0x10000, CRC(8708727c) SHA1(9be25b1af080aee863441cf0d25d0f984accb086), ROM_BIOS(0))
	// 1: BIOS-String: 30-0100-D01131-00101111-070791-ALI1217-F
	ROM_SYSTEM_BIOS( 1, "m121702", "ALi M1217 #2" )
	ROMX_LOAD( "3alm006.bin", 0x10000, 0x10000, CRC(e448c436) SHA1(dd37127920a945f1273c70c41e79e4fc70a5de01), ROM_BIOS(1))
	// 2: BIOS-String: 30-0501-D81105-00101111-070791-ACER1217-0 - 386SX NPM/33,40-A0(2) 05/12/1993
	ROM_SYSTEM_BIOS( 2, "m919a00", "386SX NPM/33,40-A0" )
	ROMX_LOAD( "m919a00_npm-40.bin", 0x10000, 0x10000, CRC(4f330d82) SHA1(08224c7bcfb2a859b682bf44ac1ac7fd9f2ade78),ROM_BIOS(2))
	// 3: GMB-386SAT - Am386SX-40, IIT 3C87SX-33 - flashing "K/B controller incorrect" - Chipset: ALi M1217-40 - ISA8: 1, ISA16: 5
	// BIOS: AMI 386SX BIOS AA1280569 - BIOS-String: 30-0100-428036-00101111-111192-ALI1217-F - Keyboard-BIOS: JETkey V5
	ROM_SYSTEM_BIOS( 3, "gmb386sat", "GMB-386SAT_V1.0" )
	ROMX_LOAD( "gmb-386sat_v1.0.bin", 0x10000, 0x10000, CRC(59ecc773) SHA1(f2007fce76b3a91f51bfb5f43c1539d5ae06d35f), ROM_BIOS(3))
	// 4: ML-765 V2 - BIOS: AMI AA0508210 - BIOS-String: 30-0103-DJ1113-00101111-070791-ACER1217-0
	ROM_SYSTEM_BIOS( 4, "ml765", "ML-765" )
	ROMX_LOAD( "3alm011.bin", 0x10000, 0x10000, CRC(27a799d4) SHA1(873cf5968923c5a655ff32f3d968b7cddcb08e76), ROM_BIOS(4))
	// 5: BIOS: AA0030659 - BIOS-String: 30-0100-428029-00101111-070791-ACER1217-0
	ROM_SYSTEM_BIOS( 5, "m121703", "ALi M1217 #3" )
	ROMX_LOAD( "3alm012.bin", 0x10000, 0x10000, CRC(5b822a2a) SHA1(e61b27f06cfec54973fbabff277bde617847b1e2), ROM_BIOS(5))
	// 6: 303N1 - Chipset: ALi M1217, M5818 - BIOS: MR BIOS MR001-SX V.1.41 - Keyboard BIOS: JETkey V3.0 - CPU: i386sx, FPU socket provided
	// RAM: 4xSIMM30 - OSC: 80.000MHz - ISA8: 1, ISA16: 5
	ROM_SYSTEM_BIOS( 6, "acer310", "Acer 310" )
	ROMX_LOAD( "3alr001.bin", 0x10000, 0x10000, CRC(b45e5c73) SHA1(81ef79faed3914ccff23b3da5e831d7a99626538), ROM_BIOS(6))
	// 7: 8517 V3.2 - Chipset : ALI M1217-40, M5818 A1 - CPU: AMD Am386SX-40, FPU: ULSI Advanced Math Coprocessor SX/SLC US83S87 - BIOS : MR 386SX BIOS V.1.40 SX M1217
	// BIOS-String: - Keyboard-BIOS: JETkey V3.0 - OSC: 80.000MHz - ISA8: 1, ISA16: 5
	ROM_SYSTEM_BIOS( 7, "8517v32", "8517 V3.2" ) // MR BIOS (r) V1.40
	ROMX_LOAD( "m1217mrb.bin", 0x10000, 0x10000, CRC(8ac66b9d) SHA1(909e5129066d1b0563b03c4834f9894c9291c287), ROM_BIOS(7))
ROM_END

// Computechnik ASC486SLC, 386sx slot CPU - Chipset: ALi M1217-40, PIC P4020 - CPU: TI 486SLC/E -33P AF - RAM: 4xSIMM30
// OSC: 66.000 - BIOS: AMI 386SX BIOS AA1330085 - Keyboard-BIOS: AMI MEGAKEY - On board: 2xser, par, Floppy, IDE
// BIOS-String: 30-0100-001588-00101111-111192-ALI1217-0 / A-03
ROM_START( asc486slc )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD("asc486slc.bin", 0x10000, 0x10000, CRC(aadbd3b6) SHA1(635df5b3cfd4525bc8ad9067703c8860dacd987a))
ROM_END

// Manufacturer/Identifier: ECS 8517 v3.3 - Chipset: ALI-M1217-40, M5818 A1 9347 TS 10 A45296 - CPU: 386sx, FPU socket provided
// RAM: 4xSIMM30 - BIOS Version: AMI 386sx BIOS AA1021639 - Keyboard BIOS: JETkey V5 - BIOS String: 40-0000-zz1218-00101111-070791-ALI1217-0
// Board marking: 8517 V3.3 and 200-03690-341-4 - OSC: 80.000MHz - ISA8: 1, ISA16: 5
ROM_START( ecs8517 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD("8517_v33.bio", 0x10000, 0x10000, CRC(c2b18ace) SHA1(fb10108a8d7a4782442f7a518e0ebab01f2e54bd))
ROM_END


// 386 motherboards using the ALi M1419 chipset

ROM_START( alim1419 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: KMC-A419-8 VER 1.0 - Chipset: M5818 A1, ALi M1421 A1, M1419 A0 - OSC: 14.31818, 80.000MHz
	// BIOS-String: 40-0100-001453-00101111-121291-ALI1419-0 / 486DLC/386DX, ISA8: 1, ISA16: 6
	ROM_SYSTEM_BIOS ( 0, "kmca419", "KMC-A419-8 VER 1.0" )
	ROMX_LOAD( "3alm010.bin", 0x10000, 0x10000, CRC(733d0704) SHA1(b5724f98047e95ea41aaa396a0374357f20cf2de), ROM_BIOS(0))
	// 1: BIOS: Award Modular BIOS v4.50 - BIOS-String: 06/22/93-ACER-M1419-214k6000-00
	ROM_SYSTEM_BIOS( 1, "alim141901", "ALi M1419 #1" )
	ROMX_LOAD( "3alw001.bin", 0x10000, 0x10000, CRC(15bd5c90) SHA1(abe2a8613d2950b27701468144fe9de8063d6e57), ROM_BIOS(1))
	// 2: 386AC P102 - CPU: Am386DX-40, FPU socket provided - Chipset: ALi M1419 A1, M1421 A1 - BIOS: AMI 386DX ISA BIOS AA1226493 -
	// Keyboard-BIOS: JETkey V3 - RAM: 8xSIMM30, Cache: 9x28pin - ISA8: 8, ISA16: 7 - OSC: 14.31818, 80.000MHz
	// BIOS-String: 40-0102-001128-00101111-121291-ALI1419-0
	ROM_SYSTEM_BIOS( 2, "386acp102", "386AC P102")
	ROMX_LOAD( "386ac_p102_ami_aa1226493.bin", 0x10000, 0x10000, CRC(43ba9775) SHA1(9f80ebf1e7ef1d7e5b7c2aad5839b4f982db75d1), ROM_BIOS(2))
	//3: ID: Unknown C3404 Rev:B - Chipset: ALI M1419 A1, M5818 A1, another unreadable (silk screen: M1421) - RAM: 8xSIMM30, Cache: 8x28pin DIP, 64K or 256K
	// CPU: AMD Am386DX-40, solder pads for Intel 80386 socket provided, FPU socket provided - BIOS: AMI 386DX ISA BIOS AA1458354
	// BIOS Version: "386DX Subversion 13.19.02 3/23/1993" - BIOS String: 40-0100-001374-00101111-121291-ALI1419-F - Keyboard BIOS: JETkey V3
	// ISA8: 1, ISA16: 6 - OSC: 14.31818, 80.000 MHz
	ROM_SYSTEM_BIOS( 3, "c3404", "C3404") // displayed BIOS-String in MAME has -0 instead of -F: this denotes the keyboard controller version http://www.idhw.com/textual/guide/inst_mobo_ami.html
	ROMX_LOAD( "386c3404.bio", 0x10000, 0x10000, CRC(8005f1f6) SHA1(1c78a7a3f134fd1299b48cd12b08b013c212fa59), ROM_BIOS(3))
ROM_END


// ***** 386 Motherboards using the Ali M1429 A1 and M1431 A2 chipsets ... they hang before initializing the graphics card

ROM_START( alim1429 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:
	ROM_SYSTEM_BIOS( 0, "386ali", "386 board with Ali chipset" )
	ROMX_LOAD( "386_ali_ami_511767.bin", 0x10000, 0x10000, CRC(3c218db4) SHA1(785ea7c36e8be5e7410524e90170d4985cbc9c24), ROM_BIOS(0))
	// 1: Seritech SER-386AD III (written on the underside of the board) - CPU: AMD Am386DX-40 - ISA16: 5
	// BIOS : AMIBIOS 04/04/1993 Ser.# 579092 - BIOS-String : 40-0212-001133-00101111-040493-ALI1429-F - Keyboard BIOS: Regional HT6542
	ROM_SYSTEM_BIOS( 1, "ser386ad", "SER-386AD III" )
	ROMX_LOAD( "ser386ad3.bin", 0x10000, 0x10000, CRC(d80d6deb) SHA1(9f889f7464255431c13ac91d7df31b325447fef5), ROM_BIOS(1))
	// 2: ISA8: 1, ISA16: 5, BIOS-String: 40-0103-001256-00101111-080893-ALI1429-H, BIOS: AMIBIOS, 08/08/93, 386DX ISA BIOS, AA2722981
	ROM_SYSTEM_BIOS( 2, "revb", "REV:B")
	ROMX_LOAD( "3alm001.bin", 0x10000, 0x10000, CRC(56ea4d9d) SHA1(0633f78a0013a62be974233a3cad6a5d3cbe90d1), ROM_BIOS(2))
	// 3: CPU: 386DX-40 -
	ROM_SYSTEM_BIOS( 3, "alim142901", "ALi M1429 #1" )
	ROMX_LOAD( "3alm007.bin", 0x10000, 0x10000, CRC(b72d754a) SHA1(364a976eac61bc923b76ccddd13f80e0727e5df5), ROM_BIOS(3))
	// 4: REV:8 - CPU: 386DX-40
	ROM_SYSTEM_BIOS( 4, "alim142902", "ALi M1429 #2" )
	ROMX_LOAD( "3alm008.bin", 0x10000, 0x10000, CRC(4cb1052d) SHA1(995a590beb0654c5e784f10019c10bd4b0278d9b), ROM_BIOS(4))
	// 5: ??? REV 2.3, Chipset: Asaki 3A029, 3A031 (= ALi 1429) - BIOS: AMI 386 BIOS S/N: 254468
	// BIOS-String: 40-0215-001926-00101111-040493-ALI1429 - ISA8: 1, ISA16: 5
	ROM_SYSTEM_BIOS( 5, "asaki", "386 with Asaki chipset" )
	ROMX_LOAD( "3asm001.bin", 0x10000, 0x10000, CRC(57c72c4d) SHA1(934223fcd39533bca2e7e57406b1800d9e900ef0), ROM_BIOS(5))
ROM_END

// Daewoo AL486V-D Rev:1.1 - BIOS/Version: AMI v299 08/08/93, BIOS-String: 40-0100-001131-00101111-080893-ALI1429 - Keyboard-BIOS: MEGAKEY
// BIOS: AMI v1.9 299 WinPro-d S/No. E-94237376 - OSC: 14.31818
ROM_START( al486vd ) // this is a 386 class board despite the name
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "al486v-d_v299.bin", 0x10000, 0x10000, CRC(75c75d58) SHA1(50e314cdefe39e8e6f74b9b045a15cc53b3f16ba))
ROM_END


// 386 motherboards using the Chips & Technologies P82C351, P82C355, P82C356 chipset

// ABIT FU340 - 6x 16-bit ISA + 2x 8-bit ISA - RAM: SIMM30x8, Cache: 32/64/128/256KB with TAG (TULARC info)
// BIOS-String: 30-0200-D01247-00101111-050591-PEAKDM_B-0 / FU340 REV-B PAGE MODE BIOS
ROM_START( fu340 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ami_abit_fu340.bin", 0x10000, 0x10000, CRC(9ea90d90) SHA1(091bdae7b1e36ac5168823d80d5907af2a95e583))
ROM_END

// GES 9051N-386C VER -0.01 - CPU/FPU: i386DX-33, i387DX 16-33 - Chipset: Chips F82C351, F82C355, F82C356 - BIOS: AMI 386DX ISA BIOS (AA0365368)
// BIOS-String: 30-1113-002101-00001111-050591-PEAKDM_B-0 / GES 9051N BIOS VERSION 2.0 - ISA8: 3, ISA16: 5
ROM_START( ges9051n )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "3ctm001.bin", 0x10000, 0x10000, CRC(7f03f606) SHA1(d03d5b6541bc7f41d78159f82aa8057229516c37))
ROM_END


// ***** 386sx motherboards using the Chips SCAMPSX chipset

// ANIX CH-386S-16/20/25G P/N:001-0386S-016 VER 1.0 - Chipset: CHIPS F82C836 - BIOS: AMI 386sx BIOS PLUS S/NO. 141334
// BIOS-String: 30-0100-D01425-00101111-050591-SCAMPSX-0 - Keyboard-BIOS: Intel/AMI - CPU: Intel (SMD), label not readable - FPU: socket available - ISA16: 6 - OSC: 14.31818 - 32.000 MHz
ROM_START( anch386s )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-ch-386s.bin", 0x10000, 0x10000, CRC(8902c64b) SHA1(3234bac6240a3a0bd05302c9ca587f5ae083f2f4))
ROM_END

// 80386SX-VH-COM - Chipset: VLSI VL82C311 VL82C113 - BIOS: Award 386SX-BIOS - BIOS-String: 386SX Modular BIOS v3.15 Copyright(c)1984-91 Award Software Inc.
// CPU: i386sx-25, FPU socket provided - RAM: 8xSIMM8 - OSC: 16MHz, 14.31818, 50.000MHz - ISA8: 2, ISA16: 6
ROM_START( 386sxvhcom )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "80386sx-vh-com.bin", 0x10000, 0x10000, CRC(65f5d279) SHA1(81c40ad1f7dde3235a074d97768ed7e09cf05f52))
ROM_END

ROM_START( scamp386sx )
	ROM_REGION16_LE(0x20000,"bios", 0)
	// 0: BIOS-String: 30-0100-D61204-00101111-050591-SCAMPSX-0 / MB-1316/20/25VST
	ROM_SYSTEM_BIOS(0, "mb386sx", "mb386sx-25spb") // VLSI SCAMPSX
	ROMX_LOAD( "386sx_bios_plus.bin", 0x10000, 0x10000, CRC(f71e5a8d) SHA1(e73fda2547d92bf578e93623d5f2349b97e22393), ROM_BIOS(0))
	// 1: BIOS-String: 30-0400-428027-00101111-070791-SCMPSX-0 / VLSI SCAMP 386SX 16/20/25MHz - seen on a PC-Chips/Amtron board
	ROM_SYSTEM_BIOS(1, "scamp01", "VLSI SCAMPSX #1")
	ROMX_LOAD( "ami_386sx_vlsi_scamp_070791.bin", 0x10000, 0x10000, CRC(082d071c) SHA1(69af9a951f138146036b3c9ac3761cc6589b6cf5), ROM_BIOS(1))
	// 2: Dataexpert (Unknown model) - Chipset: VLSI VL82C311-25FC2 (SCAMPSX), HM6818P - BIOS: AMI 05/05/1991 on a 64KB "Fairchild FM27C512"
	// BIOS-String: 30-0100-D41107-00101111-050591-SCAMPSX-0 - SCAMP 386SX WITHOUT COMBO - Keyboard BIOS: 247076 - CPU: AM386SX/SXL-25 - OSC: 50.000MHz, 16.000MHz, 14.31818 - ISA16: 7
	ROM_SYSTEM_BIOS(2, "datax386sx", "Dataexpert 386sx")
	ROMX_LOAD( "bios.bin", 0x10000, 0x10000, CRC(0ba46059) SHA1(b152796e9ace0cd17c413df14d989b9cb23aa529), ROM_BIOS(2))
	// 3: VLSI 311 386SX VER 1.0 - CPU: AM386 SX/SXL-25 - Chipset: VPL201, VPL101 - BIOS: AMI 386sx 409425 - OSC: 50.000 MHz - ISA16: 6
	// BIOS-String: 30-0400-D41107-00101111-070791-SCMPSX-0
	ROM_SYSTEM_BIOS( 3, "vlsi311", "VLSI 311")
	ROMX_LOAD( "vlsi_311_386sx_ver_1.0_bios.bin", 0x10000, 0x10000, CRC(98056235) SHA1(3a3ff07808c4d43e4935c7463741e3ed8e515af9), ROM_BIOS(3))
	// 4: BIOS-String: 30-0100-ZZ1379-00101111-050591-SCAMPSX-0 / SCAMP 386SX - Chipset: (VLSI ???) 82C310, 82C311 - CPU: 386SX-25
	ROM_SYSTEM_BIOS(4, "scamp02", "VLSI SCAMPSX #2")
	ROMX_LOAD( "3vlm002.bin", 0x10000, 0x10000, CRC(4d2b27b3) SHA1(3c67d7bd507ceb4d1762866f69c2cb94cd799a15), ROM_BIOS(4))
ROM_END


// ***** 386 Motherboards using the Chips & Technologies CS8230 chip set: P82C301C, P82C302C, P82A303, P82A304, 2x P82B305, P82A306 A, P82C206

// ECS (Elitegroup) 386A - Chipset: CHIPS P82C301/2/5 + P82A303/4/6 + P82C206 - BIOS/Version: AMI 386-BIOS / 386A V1.1 05-19-90 (2 x 27C256)
// BIOS String: EC&T-1131-040990-K8 - Keyboard BIOS: AMI Keyboard BIOS PLUS - CPU: Intel 80386DX-25, FPU: 80387DX-25 - RAM: 8xSIMM30
// ISA8: 2, ISA8/RAM: 1, ISA16: 5 - OSC: 32.0000MHz, 50.0000MHz, 14.31818
ROM_START( ecs386a ) // hangs after initialising the graphics card
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROMX_LOAD( "386a012l.bin", 0x10000, 0x8000, CRC(8a067777) SHA1(aadc84867155e4e167a0380bd409dba62fd238a1), ROM_SKIP(1))
	ROMX_LOAD( "386a012h.bin", 0x10001, 0x8000, CRC(525fc3bd) SHA1(3ab8cb5989933edc6aa9f99fcab518307b60552b), ROM_SKIP(1))
ROM_END

ROM_START( cs8230 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: EC&T-1332-040990-K0
	ROM_SYSTEM_BIOS(0, "cs823001", "CS8230 #1")
	ROMX_LOAD( "ami_386_cs8230_chipset.bin", 0x10000, 0x10000, CRC(1ee766d0) SHA1(75dba3c9817dfe6caca46f5f4f2f1d76ba88d3c7), ROM_BIOS(0) )
	// 1: BIOS-String: EC&T-1197-022589-K0
	ROM_SYSTEM_BIOS(1, "cs823002", "CS8230 #2")
	ROMX_LOAD( "3ctm004l.bin", 0x10000, 0x8000, CRC(b6efc361) SHA1(88d89bf5e7c57ffe4751e14220ac82a2d0a12994), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "3ctm004h.bin", 0x10001, 0x8000, CRC(f26c2672) SHA1(1d3a2554bbf3dc554970e0d62d9c5fad24977f55), ROM_SKIP(1) | ROM_BIOS(1) )
ROM_END

// ECS-386/32 - OSC: 40.000MHz, 32.000MHz, 14.318MHz - CPU: 386DX-20, FPU socket provided
// 8x SIMM, 5x 16-bit ISA, 2x 8-bit ISA, 1x 32-bit proprietary memory expansion slot
ROM_START( ecs38632 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: EC&T-1131-030389-K0
	ROM_SYSTEM_BIOS( 0, "030389", "030389")
	ROMX_LOAD( "ami_ecs-386_32_lo.bin", 0x10000, 0x8000, CRC(e119d6a4) SHA1(bcc6164173b44832b8ebfa1883e22efc167e2cd4), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "ami_ecs-386_32_hi.bin", 0x10001, 0x8000, CRC(e3072bf8) SHA1(74eec72e190f682cfd5ae5425ebdc854e0ba7bc9), ROM_SKIP(1) | ROM_BIOS(0) )
	// 1: BIOS-String: EC&T-1131-092588-K0
	ROM_SYSTEM_BIOS( 1, "092588", "092588")
	ROMX_LOAD( "ami_1131_bios_l.bin", 0x10000, 0x8000, CRC(145c3905) SHA1(9ee2615982e28082971bae8ef8a1f936313ac8c8), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "ami_1131_bios_h.bin", 0x10001, 0x8000, CRC(73d57778) SHA1(176c90134540d5054c99c077126c7c5a65199175), ROM_SKIP(1) | ROM_BIOS(1) )
ROM_END

// SY-012 16/25 386MB VER: 5.2 - Chipset: Chips P82C301C; P82A306; P82A303; P82C206; P82A304; P82C302; P82B305
// BIOS: AMI 386 BIOS 10084 - BIOS-String: DC&T-1102-082588-K0 - CPU: i386DX-33, ISA8: 2, ISA16: 5, Memory: 1
// OSC: 14.31818 - 20.000 MHz - 50.000 MHz - 32.000 MHz
ROM_START( sy012 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROMX_LOAD( "386-sy-012-l_32k.bin", 0x10000, 0x8000, CRC(6ab197f4) SHA1(7efd9033af3a0b36bc5be64cb28c6218cda4d13c), ROM_SKIP(1) )
	ROMX_LOAD( "386-sy-012-h_32k.bin", 0x10001, 0x8000, CRC(61aedfdb) SHA1(0f492dc8102386a1c475c5637fb7853d81d3efb6), ROM_SKIP(1) )
ROM_END

// Goldstar 611-606A - Chipset: CHIPS P82C206 P82C301 P82A303 P82C302 P82A304 2xP82A305 -
// OSC: 14.318 - 9.6000000 MHz - 40.000000 MHz - 16.000000 MHz
// BIOS: TI CMC3000 - BIOS-String: Phoenix 80386 ROM BIOS PLUS Version 1.10 01 - release 2.7B
ROM_START( gs611606a )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROMX_LOAD( "386-goldstar-e_32k.bin", 0x10000, 0x8000, CRC(3f358257) SHA1(1570f3de1955895c29c1c4240e1cd47aadff1be0), ROM_SKIP(1) )
	ROMX_LOAD( "386-goldstar-o_32k.bin", 0x10001, 0x8000, CRC(c5d75635) SHA1(70ceb4089bfd3af6853c3d6e28dbded0c43f6a40), ROM_SKIP(1) )
ROM_END

// DFI386-20.REV0 - Chipset: Chips 2xP82B305 P82A304, P82C302 P82C301 P82C206, two unreadable - initializes graphics card then hangs
// BIOS: AMI 386 BIOS PLUS Ser.#: 102856 - Keyboard BIOS: AMI 386 BIOS PLUS Ser.#:102856
// CPU: i386DX-20 - ISA8: 1, ISA16: 5, Memory: 1 - Memory card shown in photos
// OSC: OSC1: 14.31818, OSC2: 16.000MHz, OSC3: unreadable, OSC4: 40.000MHz
ROM_START( dfi386 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROMX_LOAD( "386-dfi-386-20_even_32k.bin", 0x10000, 0x8000, CRC(2d1309f8) SHA1(a75816b97d1f763dba39bdccf8e58729a58b0e56), ROM_SKIP(1) )
	ROMX_LOAD( "386-dfi-386-20_odd_32k.bin", 0x10001, 0x8000, CRC(1968fe11) SHA1(b5662daa57751859d2cfa7740f708277cbe35080), ROM_SKIP(1) )
ROM_END


// ***** 386 Motherboards using the Forex FRX36C300 + FRX46C402; SiS 85C206 chipset

// Chipset: FOREX FRX46C402 FRX36C300 SIS 85C206 SiS 85C206 - CPU: Intel 80386DX-16 - ISA16: 7, ISA16/Memory: 1 - OSC: 66.000MHz
ROM_START( frxc402 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS: AMI 386 BIOS PLUS - Ser. 006707 - BIOS-String: 30-0400-ZZ1266-00101111-070791-FORX-0 / FRX/386DX CACHE SYSTEM
	ROM_SYSTEM_BIOS(0, "frx386", "FRX/386")
	ROMX_LOAD( "386-forex.bin", 0x10000, 0x10000, CRC(4a883c14) SHA1(1c2de190ccd152ff894f9fd128e028d4fa63109a), ROM_BIOS(0))
	// 1: Chipset: Forex FRX36C300 + FRX46C402; IMP 82C206 - ISA16: 8, memory extension connector on board but not fitted
	// BIOS: AMI - BIOS-String: - 30-0400-ZZ1139-00101111-070791-FORX-0, FRX/386DX CACHE SYSTEM - Keyboard BIOS: Intel P8942HP with AMI KB-BIOS-VER-F - OSC: 14.31818MHz, 66,667MHz
	ROM_SYSTEM_BIOS(1, "frximp", "Forex 386 with IMP chip")
	ROMX_LOAD( "386-imp82c206p.bin", 0x10000, 0x10000, CRC(6f340961) SHA1(393720e1bfe3d323a34106992a65dd593284bf95), ROM_BIOS(1))
ROM_END

// RAM: 8xSIMM30, Cache: 10 sockets, 5xATT7C199P occupied - ISA8: 2, ISA16: 6 - CPU: AM386DX/DXL-40, FPU: IIT 3C87-25
// OSC: 80.000MHz, 14.318180MHz - Keyboard-BIOS: AMI 386 BIOS ZA902884
ROM_START( smih0107 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// BIOS-String: 30-0400-428005-00101111-070791-FORX-0 / BIOS ID SMIH0107 / IT9112
	ROM_LOAD( "ami_smih0107.bin", 0x10000, 0x10000, CRC(970bb0c0) SHA1(4a958887485f7239d25fa7b0c98569b97ce93800))
ROM_END


// ***** 386 Motherboards using the Forex FRX46C402 + FRX46C411 + SiS 85C206 chipset

// PT-581392 - CPU: AMD 386DX-40 FPU: ULSI Advanced Math Coprocessor DX/DLC 40MHz US83C87
// BIOS : AMI 07/07/1991, on a 27C512 type EPROM (64KB) Ser.# 007139, BIOS-String : 30-0400-ZZ1101-00101111-070791-FORX-0 FRX/386DX CACHE SYSTEM
// Keyboard-BIOS: AMI, Ser.# 007139 - OSC: 14.31818, 80.000MHz - ISA16: 8
ROM_START( pt581392 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "pt-581392 386dx.bin", 0x10000, 0x10000, CRC(389a93de) SHA1(8f1320b1d163167272cfad073f58c355e31fcf6f))
ROM_END

// Micro-Express Inc. Forex 386 Cache - Chipset: Forex FRX46C402, FRX46C411, Morse 92A206S - Keyboard BIOS: Lance LT38C41
// BIOS: EPROM, AMI 386 BIOS, #ZA605315 - CPU: AM386DX-40 - OSC: 66.6670MHz - ISA8: 2, ISA16: 5
// BIOS-String: 30-0701-001585-00101111-121291-microtel-0 / Microtel Computer products present C3HF/09/92
ROM_START( frx386c )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "forex386.bin", 0x10000, 0x10000, CRC(007b5565) SHA1(cf749fe05cacebb2230cd7493523ae55e80eea8b))
ROM_END


// ***** 386sx motherboards using the Headland HT18/C chipset
// moved here from 286, original comment: not a bad dump, sets unknown probably chipset related registers at 0x1e8 before failing post
ROM_START( ht18c )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: (BIOS release date:: 07-07-1991) - Chipset: Headland HT18/C
	ROM_SYSTEM_BIOS(0, "ami181", "AMI HT 18.1")
	ROMX_LOAD( "ht18.bin", 0x10000, 0x10000, CRC(f65a6f9a) SHA1(7dfdf7d243f9f645165dc009c5097dd515f86fbb), ROM_BIOS(0) )
	// 1: CPU: 386SX-25 - BIOS: AMI; 12/12/91
	ROM_SYSTEM_BIOS(1, "ami182", "AMI HT 18.2")
	ROMX_LOAD( "3hlm001.bin", 0x10000, 0x10000, CRC(b1434d6f) SHA1(1863dd60ad2b494141b4b30fe7b02f454bec82a3), ROM_BIOS(1) )
	// 2: CPU: 386SX-25 - BIOS: AMI; 07/07/91
	ROM_SYSTEM_BIOS(2, "ami183", "AMI HT 18.3")
	ROMX_LOAD( "3hlm002.bin", 0x10000, 0x10000, CRC(10a78d11) SHA1(0500d92e2691164bdc5c71b3d6fd0a154f7279d4), ROM_BIOS(2) )
	// 3: CPU: 386SX-25 - BIOS: AMI; 04/30/91
	// BIOS-String: 30-01]1-ZZ1372-00101111-0403091-HT18SX-0
	ROM_SYSTEM_BIOS( 3, "ami184", "AMI HT 18.4") // marked as BAD_DUMP for the "]" in the BIOS string ... and because it actually runs :)
	ROMX_LOAD( "3hlm003.bin", 0x10000, 0x10000, BAD_DUMP CRC(50f7a543) SHA1(8962f7ce2fc5c60059894cae04cf5fccd6cee279), ROM_BIOS(3) )
	// 4: MBA-025 - Chipset: Headland HT18/B, HM6818A - BIOS: AMI 386SX BIOS PLUS T.B 238958 - BIOS-String: 30-0100-009999-00101111-043091-HT18SX-0
	// Keyboard-BIOS: AMI Keyboard BIOS PLUS T.B. 238958 - CPU: AMD AM386 SX/SXL-25 - FPU: empty socket - OSC: 32.000 MHz - 50.000 MHz - 14.31818 - ISA8: 1, ISA16: 6
	ROM_SYSTEM_BIOS( 4, "mba025", "MBA-025" )
	ROMX_LOAD( "386-mba-025-low_32k.bin", 0x10000, 0x8000, CRC(4df55219) SHA1(7dc1adb130ae8c3c88e2c58bde6e3b793fa0c78e), ROM_SKIP(1) | ROM_BIOS(4))
	ROMX_LOAD( "386-mba-025-high_32k.bin", 0x10001, 0x8000, CRC(0406fdc9) SHA1(ee21d584c98b0c11ec2cfb609de83c38b0a893c7), ROM_SKIP(1) | ROM_BIOS(4))
ROM_END


// ***** 386 Motherboards using the Macronix MX83C305(A)(FC), MX83C06(A)(FC) chipset

// CACHING TECH CORPORATION C386MX - Chipset: MX83C306 MX - BIOS: AMI - BIOS-ID: 31-0100-001190-00101111-121291-MXIC
// CPU: 386DX, FPU: Cyrix - RAM: 8xSIMM30 - ISA8: 2, ISA16: 6
ROM_START( ctcc386mx ) // nine short beeps, no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "c386mx.bio", 0x10000, 0x10000, CRC(9b24ce11) SHA1(d027da188cbdfbe34b279bd3bd84eccda75b4a5a))
ROM_END

// TAM/33/40-MA0 (CM318R00,M31-R00) - Chipset: MX83C305, MX83C306 - CPU: AMD Am386DX-40 - ISA16:8
// OSC: 80.000MHz - 14.31818 - BIOS: AMI 386 BIOS PLUS S/N OA2050592 - BIOS-String: 31-0100-001105-00101111-121291-MXIC-0 - 386DX/Cx486DLX TAM/33,30-MA0/MA01, 09/10/1992
ROM_START( tam3340ma0 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "tam_33_40-ma0.bin", 0x10000, 0x10000, CRC(56411a9f) SHA1(a6c80ea531912b758fd5b573d4fa125172cacce7))
ROM_END

// Octek Jaguar V rev.1.4 - Chipset: MX83C: MX83C305FC, MX83C306FC
// CPU: AMD 386DX-40, FPU socket provided - OSC: 80.000MHz, 14.31818
ROM_START( ocjagv )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: MR BIOS (r) V1.40
	ROM_SYSTEM_BIOS(0, "jagvmr14", "Jaguar V MR-BIOS 1.40")
	ROMX_LOAD( "bios.bin", 0x10000, 0x10000, CRC(a552d6ad) SHA1(91bae14c3ec7edbc9ef240fec1be17f3582d7ec2), ROM_BIOS(0))
	//1: AMI BIOS// BIOS: AMI 386DX ISA BIOS AA0797325 - BIOS-String: 31-0100-426069-00101111-121291-MXIC-0 MX-DIR_001
	// Keyboard-BIOS: Intel
	ROM_SYSTEM_BIOS(1, "jagvami", "Jaguar V AMI BIOS")
	ROMX_LOAD( "octek_jaguar_v_ami_bios_isa386dx.bin", 0x10000, 0x10000, CRC(f8d14914) SHA1(14e8ecc4794920dc530fc6bd12ad64494e2544e5), ROM_BIOS(1))
ROM_END

// DTK MBA-032Q TK83305-4N-D-03 - Chipset: MACRONIX MX83C305FC, MX83C306FC, HM6818A - BIOS/Version: Award v4.20, MXIC 80386 CACHE SYSTEM, S/N 0089082653
// BIOS string: 08/07/92-MX83C305/306-113b0000-00 (113B00AC) - Keyboard BIOS: Award, S/N: 008902654 - CPU: i386DX-25, solder pads for 80386, FPU socket provided
// RAM: 8xSIMM39 - OSC: 14.31818M, 80.000MHz - ISA8: 2, ISA16: 6
ROM_START( mba032q )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "mba-032q.bin", 0x10000, 0x10000, CRC(23481187) SHA1(1b6d0f54ce73853fcdd43588196bb6072b39d068))
ROM_END

ROM_START( mx83c305 )
	// 0: AMI BIOS, BIOS-String:  31-0101-009999-00101111-121291-MXIC-0 / 09/02/1992 - Keyboard-BIOS: JETkey V5.0
	// Chipset MX83C05AFC, MX8306AFC - CPU: AMD AM386DX-40, OSC: 14.31818 - ISA8: 1, ISA16: 5
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS( 0, "mxic01", "MXIC #1")
	ROMX_LOAD( "mxic.bin", 0x10000, 0x10000, CRC(81853049) SHA1(d855b8d935417cfcfd6580fe3ed4ea393dd49b35), ROM_BIOS(0))
	// 1: BIOS-String: 30-0200-009999-00101111-111192-MXIC-0 / 12/15/1993
	ROM_SYSTEM_BIOS( 1, "mxic02", "MXIC #2")
	ROMX_LOAD( "3mxm001.bin", 0x10000, 0x10000, CRC(62fcd52b) SHA1(fa34c27be4627c68fe5c828451d86cbfad0ba358), ROM_BIOS(1))
ROM_END


// ***** 386sx motherboards using the Opti F82C206, Opti 82C283 chipset
ROM_START( op82c283 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: Chipset: Opti F82C206, Opti 82C283 - BIOS: ARMAS AMI 386SX BIOS PLUS 9014775 - Keyboard-BIOS: NEC D80C42C
	// BIOS-String: 30-013X-D21185-00001111-031591-OPSX-0 - OSC: 50.000MHz, 14.31818MHz - CPU: AM386 SX/SXL-25 - ISA8: 2, ISA16: 6
	ROM_SYSTEM_BIOS( 0, "armas", "386sx ARMAS" )
	ROMX_LOAD( "386-opti-armas.bin", 0x10000, 0x10000, CRC(d9c696bc) SHA1(467617ab4a211ce460766daa3e5803e190368703), ROM_BIOS(0))
	// 1: 386SX MAIN BOARD REV:A1-1M/N: 3805 - Chipset: OPTi F82C206 / OPTi (unreadable) - BIOS:AMI 386SX BIOS ZZ908380
	// BIOS-String: 30-0100-DG112-00101111-031591-OPSX-0 / A10001B / 128KB RESERVED FOR RAM SHADOW.
	// Keyboard-BIOS: AMI KB-BIOS-VER-F (Intel P8942AHP) - CPU: AM386 SX/SXL-25 - OSC: 14.31818MHz, 50.000MHz - ISA8: 2, ISA16: 6
	ROM_SYSTEM_BIOS( 1, "3805", "386sx 3805" )
	ROMX_LOAD( "386sx-opti-908380.bin", 0x10000, 0x10000, CRC(38502567) SHA1(d65d272aa60642197c9b639a8679f8f41c4a697b), ROM_BIOS(1))
	// 2: CPU: 386SX-20 - BIOS: AMI; 03/15/91 - no display - unknown ASI motherboard
	// BIOS-String: 30-0100-DG1112-00101111-031591-OPSX
	ROM_SYSTEM_BIOS( 2, "c28301", "OPTi 82C283 #1")
	ROMX_LOAD( "3opm010.bin", 0x10000, 0x10000, CRC(7c2acf57) SHA1(d40605621da40204dc6370d2d00b783b3a7f8dce), ROM_BIOS(2))
ROM_END

// Octek Panther II - Chipset: OPTi 82C283, F82C206L/Chips 82C206 - CPU: 386sx - BIOS: AMI 386sx BIOS - Keyboard-BIOS: Intel/AMI
// BIOS-String: 30-0200-420000-00101111-050591-OPSX-0 / OPSX 386SX PAGE INTERLEAVE BIOS - ISA8: 2, ISA16: 4
ROM_START( ocpanii )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "octek_panther_ii_386sx.bin", 0x10000, 0x10000, CRC(866192d5) SHA1(fe6133ee3ba0d71c0d4690a0843ca82106effcf6))
ROM_END

// RYC Alaris LEOPARD LX REV D - Chipset: Opti 82C283 82C206Q - CPU: 486SLC2 (IBM 14 Q) - ISA16: 7
// BIOS: AMI 486SLC ISA BIOS AA0735388 - Keyboard-BIOS: Intel/AMI MEGA-KB-H-WP
// BIOS-String: 30-0100-006328-00101111-060692-OPSXPI-0
ROM_START( alaleolx )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-ryc-leopard-lx.bin", 0x10000, 0x10000, CRC(bbc7bfd2) SHA1(49833b482efb8e361be88f48e252621b147a3b1b))
ROM_END


// ***** 386 motherboards using the OPTi 82C381/382 "HiD/386 AT chipset"

// CPU: 386DX-25 - Chipset: OPTI 82C382 25MHz, 82C381P, 82C206 - BIOS: AMI; 09/15/90
// BIOS-String: EOX3-6069-083090-K0
ROM_START( op82c381 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "3opm004.bin", 0x10000, 0x10000, CRC(933c2c2b) SHA1(191a1a80c128430a0a461ff9202d27969a715d9d))
ROM_END

// Shuttle HOT-304 - Chipset: Opti F82C382, Opti (erased), UMC UM82C206L - OSC: 14.31818MHz, 50.000MHz
// BIOS: AMI, Ser.Nr. 150796 - BIOS-String: 30-0101-DK1343-00001111-050591-OPBC-0 - Keyboard BIOS: AMI Ser.Nr. 209210 - ISA8: 1, ISA16: 6, ISA16/Memory: 1
ROM_START( hot304 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-hot-304.bin", 0x10000, 0x10000, CRC(cd4ad4ec) SHA1(50f1b7a15096fff7442d575a47728ba4709b2f39))
ROM_END

// Arche Technologies Inc. KMA-300G-25 - Chipset: OPTi 82C381, 82C382, UMC206L - CPU: i386-25, FPU: socket provided - RAM: SIMM30x8, Cache: 8x5C6408-35
// ISA8: 1, ISA16: 6, ISA16/Memory: 1 - OSC: 50.000MHz, 14.31818 MHz
// BIOS-String: EOPG-3700-040990-K0
ROM_START( kma300g )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "bios.bin", 0x10000, 0x10000, CRC(3149a4a4) SHA1(6a027ed94568a89a800360119da0c568a2a29e19))
ROM_END


// ***** 386sx motherboards using the OPTi 82C291 chipset

ROM_START( op82c291 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: MR BIOS (r) V1.43
	ROM_SYSTEM_BIOS(0, "mr", "MR")
	ROMX_LOAD( "opti_82c291_mr-bios_v143.bin", 0x10000, 0x10000, CRC(f7989a65) SHA1(cc729b6baac486ac3116f08e78eb58bb39365dd5), ROM_BIOS(0))
	// 1: CPU: 386SX-40 - BIOS: AMI - no display, nine beeps, so probably bad dump
	ROM_SYSTEM_BIOS( 1, "ami", "AMI")
	ROMX_LOAD( "3opm007.bin", 0x10000, 0x10000, CRC(eed82365) SHA1(45f5a608740d161c5a74415ff3f7b573d7e61f58), ROM_BIOS(1))

ROM_END

// DTK Computer PPM-3333P - Chipset: OPTi 82C291 - Opti F82C206 - ISA16: 6 - CPU: AMD Am386SX/SXL-33, FPU: empty socket - OSC: 14.31818 - 66.0000 MHz
ROM_START( ppm3333p )
	ROM_REGION16_LE(0x20000, "bios", 0)
	//0: Award Modular BIOS v4.20 (80386DX) / (119U906X) DTK Computer
	ROM_SYSTEM_BIOS(0, "ppmawa", "PPM-3333P Award")
	ROMX_LOAD( "386sx_opti291-award.bin", 0x10000, 0x10000, CRC(4855b394) SHA1(94dd1a38852eecac538ef4b8bf04bb7c1e4317d2), ROM_BIOS(0))
	//1: BIOS-String: 30-0200-001107-00001111-121291-OPTX 291-0 / OPTI-291WB BIOS VER 1.2
	ROM_SYSTEM_BIOS(1, "ppmami", "PPM-3333P AMI")
	ROMX_LOAD( "386sx_opti291-ami.bin", 0x10000, 0x10000, CRC(35727f8f) SHA1(3fb14cd6ea0d7a2bd545beb1586403cc36a77668), ROM_BIOS(1))
ROM_END


// ***** 386 motherboards using the Opti F82C206, 82C391B, 82C392 chipset

ROM_START( op82c391 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: CPU: 386DX - Chipset: OPTi 82C391 B2, 82C392 B - BIOS: AMI; 07/07/91; AA 0571504
	// BIOS-String: 30-0100-DK1343-00101111-070791-OPWB3/B-0
	ROM_SYSTEM_BIOS(0, "39101", "82C391 #1")
	ROMX_LOAD( "3opm001.bin", 0x10000, 0x10000, CRC(3cb65e60) SHA1(c91deaba1b34008449d6cc2aa94d115c47e0640a), ROM_BIOS(0))
	// 1: BIOS: AMI; 05/05/91; AMI 386C BIOS; #1023992
	ROM_SYSTEM_BIOS(1, "39102", "82C391 #2") // no display, nine beeps
	ROMX_LOAD( "3opm005.bin", 0x10000, 0x10000, CRC(ef3dcdde) SHA1(53a8d0af776362d5b92d1cce92d6ca8dbeb33398), ROM_BIOS(1))
	// 2: BIOS: AMI; 07/07/91 - no display
	ROM_SYSTEM_BIOS(2, "39103", "82C391 #3")
	ROMX_LOAD( "3opm011.bin", 0x10000, 0x10000, CRC(6706c85a) SHA1(70af6de83e59df3d9b74e904fde98d0b9cbdaae9), ROM_BIOS(2))
	// 3: BIOS-String: 30-0100-000000-00101111-070791-OPTi391-0
	ROM_SYSTEM_BIOS(3, "39104", "92C391 #4")
	ROMX_LOAD( "3opm12.bin", 0x10000, 0x10000, CRC(fa9592c5) SHA1(f9042163e7e2762e999687c3ec94d576f5b7c499), ROM_BIOS(3))
	// 4: BIOS-String: MR BIOS (tm) V1.30 - RAM Pattern Test Failed at 0F0000H
	ROM_SYSTEM_BIOS(4, "mrv130", "MR BIOS V1.30")
	ROMX_LOAD( "opti_82c391_386_mr_bios_v130.rom", 0x10000, 0x10000, CRC(458c9405) SHA1(845ea33e4e228aa98d1bd3b0148fa306754e3d78), ROM_BIOS(4))
ROM_END

ROM_START( op386wb )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// OPTi 386WB VER.1.0 - OSC: 66.6670MHz, 50.000MHz, 14,31818 - ISA8: 1, ISA16: 7
	// BIOS: 1006229 - Keyboard-BIOS: Intel P8942AHP
	// 0: BIOS-String: 30-0201-D41107-00101111-050591-OPWB-0
	ROM_SYSTEM_BIOS( 0, "1006229", "1006220")
	ROMX_LOAD( "386-opti-386wb-10.bin", 0x10000, 0x10000, CRC(1a5dd6b2) SHA1(9e6b556bfdf21d6f3cba6a05a3092887a71a24a8), ROM_BIOS(0))
	// 1: BIOS-String: 30-0201-D41107-00101111-050591-OPWB-0
	ROM_SYSTEM_BIOS( 1, "d41107", "D41107")
	ROMX_LOAD( "ami_d41107.bin", 0x10000, 0x10000, CRC(2b6cc50d) SHA1(69f69e0295abf5e331fdf01dc8e6b1c0c6591992), ROM_BIOS(1))

ROM_END

// Shuttle HOT-307H - BIOS-String: 30-0100-DK1343-00101111-070791-OPWB3/B-0 - CPU: 386DX - Chipset: OPTi 82C391 B2, 82C392 B - BIOS: AMI; 07/07/91; AA 0571504
ROM_START( hot307h )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "3opm001.bin", 0x10000, 0x10000, CRC(3cb65e60) SHA1(c91deaba1b34008449d6cc2aa94d115c47e0640a))
ROM_END

// ***** 386 Motherboards using the OPTi495SLC chipset => "qdi" in the 486 BIOS section uses that chipset too

ROM_START( opti495slc )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: Chipset: OPTi 82C495SLC / F82C206, BIOS: AMI 486086 - BIOS-String: 40-040A-001102-00101111-111192-OP495SLC-0
	// Keyboard-BIOS: AMI - CPU: AM386DX-40, FPU socket provided - ISA8: 1, ISA16: 5 - OSC: 14.31818
	ROM_SYSTEM_BIOS(0, "op495slc01", "OP495SLC #1")
	ROMX_LOAD( "op495slc01.bin", 0x10000, 0x10000, CRC(0b25044b) SHA1(1b585f0d73ea963dcfbf421325e7da6dd3dd918f), ROM_BIOS(0))
	// 1: BIOS-String: 40-0200-001107-00101111-111192-OP495SLC-0 - OPTI 495SLC 80386 ONLY - BIOS: AMI 386C BIOS 1605865
	// Keyboard-BIOS: AMI 386C BIOS Keyboard  ISA8: 1, ISA16: 5 - CPU: AMD AM386DX-40 - OSC: 14.3
	ROM_SYSTEM_BIOS(1, "op495slc02", "OP495SLC #2")
	ROMX_LOAD( "op495slc02.bin", 0x10000, 0x10000, CRC(4ff251a2) SHA1(e8655217bd46d50af6b30184bf462376d0e388c6), ROM_BIOS(1))
	// 2: BIOS-String: - Same board exists with an OPTi495XLC chip, possibly from A-Trend
	ROM_SYSTEM_BIOS(2, "op495slc03", "OP495SLC #3") // no display
	ROMX_LOAD( "486dlc-unknown.bin", 0x10000, 0x10000, CRC(2799e876) SHA1(ce7b421ecb27d915585c1a98bebb17cc5c2463e7), ROM_BIOS(2))
	// 3: 364 Ver. 1.1 - Chipset: OPTi 82C495SLC, F82C206Q - CPU: AMD AM386DX-40 - RAM: 8xSIMM30, Cache: 5x14pin
	// ISA8: 1, ISA16: 5 - BIOS: AMI 386C BIOS 1684393 - Keyboard-BIOS: AMI 386C BIOS KEYBOARD 1684393
	// BIOS-String: 40-040A-001107-00101111-111192-OP495SLC-0 / OPTI-495SLC BIOS VER 2.1
	ROM_SYSTEM_BIOS(3, "op495slc04", "OP495SLC #4")
	ROMX_LOAD( "386c_mobo-364-v1.1.bin", 0x10000, 0x10000, CRC(826ec2d1) SHA1(cc13e385c6614eb654ee0669f67df51f1e1fa585), ROM_BIOS(3))
ROM_END


// ***** 386 Motherboards using the OPTi495XLC chipset: OPTi 82C495XLC F82C206, BIOS: AMI 386DX BIOS Ser.#:AA2602776, Keyboard-BIOS: Lance LT38C41 - ISA8: 1, ISA16: 5

ROM_START( opti495xlc )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 40-081L-001343-00101111-080893-OP495XLC-F  / OPTi495XLC For 386
	ROM_SYSTEM_BIOS(0, "optimini", "OPTi Mini 82C495XLC")
	ROMX_LOAD( "386-opti-mini.bio", 0x10000, 0x10000, CRC(04c75e45) SHA1(d5bf92421dda3191c6da12ae2fa31c9ee7a831e1), ROM_BIOS(0) )
	// 1: MR BIOS (r) V1.60
	ROM_SYSTEM_BIOS(1, "mr495xlc", "MR BIOS for OPTi 82C495XLC") // use Hercules
	ROMX_LOAD( "mr-3dx94.rom", 0x10000, 0x10000, CRC(6925759c) SHA1(540177fe2c10e20037893c9763b0bf6e35163c9c), ROM_BIOS(1) )
	// 2: possibly from A-Trend (A1742X REV.C 94V-0), exists with an OPTi495SLC chip, see above section, ISA8: 2, ISA16: 4, ISA16/VL: 2
	// BIOS-String: X0-0804-001117-00101111-080893-OP395XLC-0 / OPTI 495XLC 3/486 BIOS VER 5.02_T 94/07/07
	ROM_SYSTEM_BIOS(2, "op82c495xlc", "82C495XLC") // this one could also be listed as a 486 board as it has solder pads and sockets for CPUs from 386 to true 486s
	ROMX_LOAD( "at080893.bin", 0x10000, 0x10000, CRC(6b49fdaa) SHA1(5b490d1d1216763ef89688c8e383c46470272005), ROM_BIOS(2) )
	// 3: BIOS: AMI; 08/08/93; AA2740000 - hangs
	ROM_SYSTEM_BIOS(3, "mao13", "MAO13 Rev. A")
	ROMX_LOAD( "3opm002.bin", 0x10000, 0x10000, CRC(2d9dcbd1) SHA1(d8b0d1411b09767e10e66b455ebc74295bd1b896), ROM_BIOS(3) )
ROM_END

// Tandy 4000

ROM_START( t4000 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "v010301", "01.03.01")
	ROMX_LOAD( "t4000_even.u32", 0x18000, 0x4000, CRC(f728ae11) SHA1(3bc151b8d704dde7c340a66072c1a418b7eab7db), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "t4000_odd.u25",  0x18001, 0x4000, CRC(6bedcf61) SHA1(df08347f163d6e45bdf9e1b64a811222bffba80a), ROM_SKIP(1) | ROM_BIOS(0))
ROM_END


// 386sx motherboards using the SARC (or CYCLONE) RC2016A5 chipset

// Pine PT-319A rev2.2a - CPU: 386sx - BIOS: AMI; 06/06/92
// BIOS-String: X0-0100-000000-00101111-060692-386SX-0 / Ver 5.20
ROM_START( pt319a )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "3sam001.bin", 0x10000, 0x10000, CRC(cad22030) SHA1(85bb6027579a87bfe7ea0f7df3676fdaa64920ac))
ROM_END


// CX Technology, Inc. Model SXD (4x SIMM30, Cache: 64/128/256KB in 2 banks plus TAG, 4x 16-bit ISA) - Chipset: SARC RC2016A5; HM6818P; CX109; LT38C41 © Lance Corp. (keyboard controller?)
// additional info from chukaev.ru54.com: Chipset: CYCLONE RC2016A5 - ISA16: 6 - ROM: CX109 340C3A62D0A - CPU/FPU: Am386SX/SXL-33, 387
ROM_START( cxsxd )
	ROM_REGION16_LE(0x20000,"bios", 0)
	// BIOS-String: 03/25/93-SARC_RC2016A-219v0000 / CX 386SX System
	ROM_LOAD( "award_cx-sxd_v4.50.srd.bin", 0x10000, 0x10000, CRC(ef1c74d7) SHA1(b40b1cb7143c4e352798bdf3b488d9342a4029a7))
ROM_END

// PC-Chips M396F VER 2.2 - CPU: 386SX, 387SX - ISA16: 6
ROM_START( pccm396f )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: Chipset: PCCHIPS CHIP2 310, HT6542, HM6818A
	// BIOS-String: X0-0100-001437-00101111-060692-M396C-0 - BIOS: AMI 386SX BIOS Ver. 2.10 C-1216
	ROM_SYSTEM_BIOS(0, "chips2", "Chips 2")
	ROMX_LOAD( "3pcm003.bin", 0x10000, 0x10000, CRC(b7fc6737) SHA1(670e38b628cb71dc09742f097349ac48ccf28696), ROM_BIOS(0))
	// 1: Chipset: SARC RC2016A5 - CPU: 386SX-40/486SLC, 387SX - BIOS: AMI; 06/06/92
	// BIOS-String: X0-0100-001437-00101111-060692-M396F-0
	ROM_SYSTEM_BIOS(1, "sarc01", "SARC RC2016A5 #1")
	ROMX_LOAD( "3sam002.bin", 0x10000, 0x10000, CRC(8d5ef8e8) SHA1(5ca2b36d5bee2870f894984909aa2013b5c4d6cf), ROM_BIOS(1))
	// 2: BIOS-String: X0-0100-001437-00101111-060692-M396F-0 - CPU: 386SX-40 (ALI M1386SX A1P)
	ROM_SYSTEM_BIOS(2, "sarc02", "SARC RC2016A5 #2")
	ROMX_LOAD( "3sam003.bin", 0x10000, 0x10000, CRC(95ea08d8) SHA1(812e8488ad63ca24250e245a2f0273f1d1703fc3), ROM_BIOS(2))
ROM_END


// ***** 386 motherboards using the SIS Rabbit : 85C310 / 85C320 / 85C330 / 85C206 chipset

// ASUS ISA-386C - BIOS : AMI 05/05/1991, on a 27C512 type EPROM (64KB)
// BIOS-String : 30-0105-001292-00101111-050591-SISDFC-386 - // ISA8: 2, ISA16:5, ISA16/Memory: 1
ROM_START( isa386c )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "asus_isa-386c_bios.bin", 0x10000, 0x10000, CRC(55e6d1bb) SHA1(e1ac490a30f63b6e4d6d9d0fbaea3d132b8ff053))
ROM_END

// Chaintech 333SC - Chipset: UMC UM82C206L, three smaller SiS chips (unreadable, probably SiS Rabbit)
// CPU/FPU present - BIOS: AMI 386 BIOS - Keyboard-BIOS: AMI
// BIOS-String: ESIS-1128-040990-K0 - ISA8: 2, ISA16: 6 - OSC: 14.31818, 66.000MHz
ROM_START( chn333sc )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "bios.bin", 0x10000, 0x10000, CRC(f8b2b0bc) SHA1(2799cce621b93bf38b04deeb419d25a73f7416f4))
ROM_END

// Octek Jaguar II - Chipset: SiS 85C330, 85C320, 85C206, 85C310 - CPU: i386-33, FPU: i387-33 - RAM: 8xSIMM30, Cache: 8xIS61C64-25N
// BIOS: AMI - Keyboard-BIOS: AMI KB-BIOS-VER-F - ISA8: 2, ISA16: 5, ISA16/Memory: 1 - OSC: 66.000MHz, 14.31818
// BIOS-String: 30-0100-006069-00101111-020291-SISC / 0033P-002
ROM_START( ocjagii )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ami_386_zz457511.bin", 0x10000, 0x10000, CRC(b5bc6a9a) SHA1(b74f592dd3bcd2978ed7d895d483f83413e0f8d5))
ROM_END


ROM_START( sisrabb )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 30-0000-D01128-00101111-070791-SISD-0
	ROM_SYSTEM_BIOS(0, "3sim001", "3sim001")
	ROMX_LOAD( "3sim001.bin", 0x10000, 0x10000, CRC(2982f552) SHA1(f1849c207d8c802faaf8ef628f88b28256e7fb31), ROM_BIOS(0))
	// 1: PLATO TECHNOLOGY CO., LTD. - Chipset: SiS 'rabbit': SIS 85C320, 85C330, 85C310, 85C206 - CPU: Intel 80386DX-33, FPU: i387DX-33
	// RAM: 16xSIMM30, Cache: 8xKM688B65P-25, 2xQS8888-25P - BIOS: AMI 386 BIOS Ser# Z403736 - Keyboard BIOS: AMI - BIOS-String: ESIS-1393-040990-K0 / SIS-B 386 BIOS
	// OSC: 50.000MHz, 66.0000MHz, 14.31818MHz - ISA8: 2, ISA16: 6
	ROM_SYSTEM_BIOS(1, "plato", "Plato")
	ROMX_LOAD( "386-sis-z403736.bin", 0x10000, 0x10000, CRC(8230f4c1) SHA1(952de1e16efe7e3a7514b68ea251b88192de3ac8), ROM_BIOS(1))
ROM_END


// ***** 386 Motherboards  using the Symphony SL82C362 SL82C461 SL82C465 chipset

// FIC 386 SC Rev A2 - Chipset: Symphony SL82C461, SL82C465, SL82C362
ROM_START( 386sc )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS: AMI 386 BIOS Ser.#: ZZ006975, BIOS-String:  30-0200-DF1211-00101111-042591-SYMP-0 / 386DX BIOS for SYMLABS SL82C360 - Keyboard-BIOS: AMI #Z357365
	// CPU: unreadable, FPU: Cyrix 387DX-25 - OSC: 40.000MHz, 14.31818 - ISA8: 1, ISA16: 7
	ROM_SYSTEM_BIOS(0, "386sc1", "386SC #1")
	ROMX_LOAD( "386_sc_symphony.bin", 0x10000, 0x10000, CRC(fabe369c) SHA1(211ff63dd874c273135d1427db3562d752c2bade), ROM_BIOS(0))
	// 1: ID: FIC 386SC REV A2 MBZ86418 - CPU: AMD 386DX/DXL-40, FPU socket provided - RAM: 8xSIMM30, Cache: 512KB
	// BIOS Version: AMI 386DX BIOS - ZZ000294 - BIOS String: 30-0200-ZZ1121-00101111-042591-SYMP-0 - 386DX BIOS FOR SYMLABS SL86C360 - ISA8: 1, ISA16: 7 - OSC: 14.31818, 80.000MHz
	ROM_SYSTEM_BIOS(1, "386sc2", "386SC #2")
	ROMX_LOAD("fic386sc.bio", 0x10000, 0x10000, CRC(6154adb7) SHA1(96c495d9a9975e1af9b42384712e609e3ffcff4e), ROM_BIOS(1))
ROM_END

ROM_START( 386sc2c )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 20-0200-DF1121-00101111-102591-SYM_386B-0 / 386DX/SX (S1A.P)
	ROM_SYSTEM_BIOS(0, "s1a", "S1A.P") // Chipset: SYMPHONY SL82C362, SL82C461, SL82C465
	ROMX_LOAD( "386-sc-2c_ami_za492668.bin", 0x10000, 0x10000, CRC(b408eeb7) SHA1(cf1974492119e1aae623fa366d5760343e827e52), ROM_BIOS(0))
	// 1: BIOS-String: 20-0200-DF1121-00001111-102591-SYM_386B-0 / 386SX/DX (S1B)
	ROM_SYSTEM_BIOS(1, "s1b", "S1B") // also on FIC 386-SC-HG
	ROMX_LOAD( "ami_386_za590821.bin", 0x10000, 0x10000, CRC(51a4c231) SHA1(4ad65408f2a401ff262934f886937a2615c08e21), ROM_BIOS(1))
ROM_END

// FIC 386-SC-HG - Chipset: SYMPHONY SL82C362, SL82C461, SL82C465 - CPU: AMD Am386DX/DXL-40, FPU socket provided - BIOS: AMI 386 BIOS ZA977287
// Keyboard BIOS: Intel/AMI - RAM: 8xSIMM30 - ISA16: 6 - OSC: 80.000MHz
ROM_START( 386schg ) // BIOS-String: 20-0200-DF1121-00101111-102591-SYM_386B-0 / 386DX/SX(S1B)
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-za877287-386-sc-hg.bin", 0x10000, 0x10000, CRC(fcaa06c0) SHA1(5ce6258a26311cec46e51cb16bcb66e9c68d16b2))
ROM_END

// Peacock P386DX-40 REV: 1.0 - Chipset: Symphony SL82C461, SL82C362, SL82C465, DS1287 - CPU: AMD Am386DX-40, FPU: IIT 3C87-40 - RAM: 8xSIMM30, Cache: 4xW4256AK-20, 1xTC5588P-20
// BIOS: AMI 386 BIOS #ZA22147 - OSC: 80.000MHz, 14.31818, 24.000 - on board: IDE, Floppy - ISA16: 6
ROM_START( p386dx40 ) // BIOS-String: 30-0200-DF1121-00101111-070791-SYM_AUTO-0 / Peacock 386 DX Ver. 1.0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "386_peacock-p386dx-40-za224147.bin", 0x10000, 0x10000, CRC(f97562cf) SHA1(354b6ffd345c9f50beca0f836b0d4a92df1e7c48))
ROM_END


// ***** 386 Motherboards using the UM82C491F chipset

ROM_START( um82c491f )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: TAM/33/40-U2 - BIOS: AMI S/NO. OA 242412 - BIOS-String: 40-0102-001105-00101111-040493-UMC491F-0 / TAM/33,40-U2, 08/11/1993
	// ISA8: 1, ISA16: 5 - OSC: 80.000MHz, 14.31818
	ROM_SYSTEM_BIOS(0, "tam3340u2", "TAM/33/40-U2")
	ROMX_LOAD( "tam_umc491f.bin", 0x10000, 0x10000, CRC(718890d5) SHA1(52336cfc7cd0f0f51799c999cefcfed2b2942211), ROM_BIOS(0))
	// 1: Board is only marked "rev.0.3, looks like 386GRN - CPU: AMD AM386DX-40 - OSC: 14.31818 - ISA8: 1, ISA16: 5
	// Chipset: UMC UM82C491F - BIOS-String: 08/30/93-UMC-491-214X2000-OO - BIOS: Award 386 D2026361 - Keyboard BIOS: JETkey V3.0
	// additional info from chukaev.ru54.com: REV:0.4 board has JETkey V5.0 keyboard BIOS, uses same motherboard BIOS
	ROM_SYSTEM_BIOS( 1, "386grn", "386GRN-like board rev.03")
	ROMX_LOAD( "386dx40-27c512.bin", 0x10000, 0x10000, CRC(692a4d52) SHA1(7970a05586eacfe4bfdc575b17bbbfb7ff1c86b0), ROM_BIOS(1))
	// 2: BIOS: AMI; 04/04/93 - CPU: 386DX-40 - BIOS-String: 40-0102-001277-00101111-040493-UMC491F-0
	ROM_SYSTEM_BIOS( 2, "491f01", "UM82C491F #1")
	ROMX_LOAD( "3umm005.bin", 0x10000, 0x10000, CRC(032e78f2) SHA1(5271c4362284ec87840b3fb23542506a72a328c2), ROM_BIOS(2))
	// 3: BIOS-String: 08/30/93-UMC-491-214X2000-OO / CACHE 386/486 SYSTEM BIOS
	ROM_SYSTEM_BIOS( 3, "491f02", "UM82C491F #2")
	ROMX_LOAD( "3umw007.bin", 0x10000, 0x10000, CRC(d82c9bef) SHA1(36e8d1c7629642cbcc337721eef1c73f1f0ed92c), ROM_BIOS(3))
ROM_END


// ***** 386 Motherboards using the UMC UM82C493F/UM82C491F chipset or badge engineered varieties (BIOTEQ)

	// BIOS-String: 40-0100-001494-00101111-080893-UMC491F-0 / 11/26/93 - CPU: TX486DLC/E-40GA, IIT 4C87DLC-40 - ISA8: 1, ISA16: 5 - BIOS: AMI; 208808; 08/08/93
ROM_START( um82c493f )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS( 0, "493f01", "UM82C493F #1" )
	ROMX_LOAD( "3umm007.bin", 0x10000, 0x10000, CRC(8116555a) SHA1(8f056a83de60373ed26026a226eead19868abeca), ROM_BIOS(0))
ROM_END

// 0: 386-4N-D04A
ROM_START( 4nd04a )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// BIOS-String: 40-0102-428003-00101111-080893-UMC491F-0 - UMC 4913 386 IVN 1.0 1993.12.22
	// Chipset: UMC UM82C493F, UM82C491F
	ROM_SYSTEM_BIOS( 0, "ivn10", "386-4N-D04A IVN 1.0" )
	ROMX_LOAD( "386-4n-d04a.bin", 0x10000, 0x10000, CRC(cf386b9c) SHA1(6fd4303e4f0d2ed75d4e7f36dc855037b1779e64), ROM_BIOS(0))
	// 1: 386-4N-D04A PCB V2.0 - BIOS-String: 40-0103-428003-00101111-080893-UMC491F-0 / UMC 4913 386 IVN 1.1 1994.1.31
	ROM_SYSTEM_BIOS( 1, "ivn11", "386-4N-D04A IVN 1.1" )
	ROMX_LOAD( "3umm006.bin", 0x10000, 0x10000, CRC(4056104d) SHA1(5e639e6766dc9a19296358e9a64a76ad57fc733a), ROM_BIOS(1))
	// 2: TK-82C491/493/386-4N-D04 - BIOS-String: (2c4x2u01) U-BOARD / 11/09/93-UMC-491-2C4X2U01-00 - ISA8: 1, ISA16: 5
	ROM_SYSTEM_BIOS( 2, "awa110993", "AWARD 11/09/93") // BIOS: Award; 386 BIOS; A3384454
	ROMX_LOAD( "3umw002.bin", 0x10000, 0x10000, CRC(2c510e81) SHA1(a12c672ec418cc4cd14482901f8ba34c50f319f5), ROM_BIOS(2))
	// 3: TK-82C491/493/386-4N-D04 - BIOS-String: 01/14/94-UMC-491-2C4X2U01-00 / U-BOARD
	ROM_SYSTEM_BIOS( 3, "awa011494", "AWARD 01/14/94")
	ROMX_LOAD( "3umw003.bin", 0x10000, 0x10000, CRC(64067839) SHA1(4ae3462619ef8da67f74d85ee7ab44bdb49a5728), ROM_BIOS(3))
ROM_END

// Biostar MB-1333/40PMB-CH, rev B.3 - Chipset: "Bioteq" [Atmel] AT40391, "Bioteq" G392 [Atmel AT40392], C&T P82C206
// BIOS: AMI 386 BIOS PLUS - Keyboard-BIOS: AMI - CPU: AM386-DX40 - OSC: 14.31818, <unreadable>
ROM_START( mb133340 )
	ROM_REGION32_LE(0x20000, "bios", 0) // the OPWB3 string also exists in the BIOS versions meant for the OPTI 82C391/392 chipsets
	// 0: BIOS-String: 30-0100-D61223-00101111-050591-OPWB3/B-0 / MB-1340PMA-CH, MB-1340PMB-CH, MB-1340PMD-CH, MB-1340PME-CH for B version..
	ROM_SYSTEM_BIOS(0, "opwb3b", "MB-1333/40PMB-CH OPWB3-B")
	ROMX_LOAD( "opwb3b.bin", 0x10000, 0x10000, CRC(c9cf46dd) SHA1(c9e58cb6fed770d92892672d0a910d448c507ac1), ROM_BIOS(0))
	// 1: BIOS-String: 30-0201-D61223-00101111-050591-OPWB-0 / MB-1333PMA-CH, MB-1333PMB-CH, MB-1333PMD-CH, MB-1333PME-CH
	ROM_SYSTEM_BIOS(1, "opwb", "MB-1333/40PMB-CH OPWB")
	ROMX_LOAD( "opwb.bin", 0x10000, 0x10000, CRC(9532c6d1) SHA1(48e889ed61921643147fea95224bcf42bb6e82fa), ROM_BIOS(1))
	// 2: BIOS-String: 40-0100-001223-00101111-040493-UMC491F-0 / MB-1333/40UCG-A, MB-1333/40UCG-B / MB-1433-40UDV-A, MB-1433/50UCV-C, MB6433/50UPC-A for EXT. RTC
	ROM_SYSTEM_BIOS( 2, "m21", "M21" )
	ROMX_LOAD( "3bim001.bin", 0x10000, 0x10000, CRC(9ea0ce67) SHA1(cb55a61cd43705a54e4109d816924c8820f78ae5), ROM_BIOS(2))
ROM_END


// ***** 386 motherboards using the UMC UM82C481AF, UM82C482A/B/F, 82C206F chipset

// QD-U386DX VER 1.0 - CPU/FPU: i386DX-33, IIT 3C87-33 - ISA8:2, ISA16: 5 - BIOS: AMI 386DX ISA BIOS (AA0119183)
// BIOS-String: 30-0200-428003-10101111-070791-UMC480A-F
ROM_START( qdu386dx ) // three short beeps (base 64k RAM failure)
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "3umm001.bin", 0x10000, 0x10000, CRC(5b6a7d0b) SHA1(02696eaaa5dd21fe4b3b39629aa926ae87a9a2bd))
ROM_END

// ASUS ISA-386U30 REV.2.2 - Chipset: UMC UM82C481AF, UM82C482AF, 82C206F - CPU: AM386DX-40 - OSC: 14.31818MHz, 32.000MHz - ISA8: 1, ISA16: 6
// BIOS: AMI 386DX BIOS AA0974582 - BIOS-String: - Keyboard-BIOS: AMI U2518640 MEGA-KB-F-WP
ROM_START( isa386u30 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-isa-386u30.bin", 0x10000, 0x10000, CRC(6d45a044) SHA1(63c06568f9db5ce12dc8dd0fb1ad1009a9fb24f6))
ROM_END

// Chicony CH-486-33C REV 1.0 - Chipset: UMC UM82C481, UM82C482, Chips P82C206 - CPU: Intel Overdrive DX2ODPR66, FPU socket provided
// RAM: 16xSIMM30, Cache: 8xW24256AK-25, P4C188-20PC - BIOS: AMI - Keyboard-BIOS: Intel/AMI
// BIOS-String: 40-0300-ZZ1116-00101111-031591-UMCWB-F - OSC: 33.000MHz - ISA8: 2, ISA16: 5
ROM_START( ch48633c )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ch-486-33c_ami_bios_z944944_am27.bin", 0x10000, 0x10000, CRC(1ca9bf11) SHA1(bb5fb4c7544e2ccb06d423ef4da0729a6bf8b231))
ROM_END

// Elitegroup FX-3000 REV:1.0 - Chipset: UMC UM82C481BF, UM82C482AF, UM82C206F - ISA16: 6
ROM_START( ecsfx3000 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 40-0100-001131-00101111-121291-UMCAUTO-0 / FX3000 V1.3 12-17-92 - Keyboard BIOS: AMI/Intel - BIOS: FX3000-014 - CPU: AM386DX-40
	ROM_SYSTEM_BIOS(0, "v13", "V1.3")
	ROMX_LOAD( "fx-3000-bios.bin", 0x10000, 0x10000, CRC(f93c9563) SHA1(46a71e7fbc9238dd470d6d5ce3bc1e057f3aae24), ROM_BIOS(0))
	// 1: BIOS-String: 30-0500-D01131-00101111-070791-UMCWB-0 / FE386 V1.1 12-03-92 - Keyboard-BIOS: Lance LT38C41 - BIOS: AMI-1131 / S/NO. E-92488183 / FE 386-012 - CPU: Cyrix 486DLC-33GP FPU: Cyrix Cx87DLC-33QP - OSC: 66.667MHz, 14.31818
	ROM_SYSTEM_BIOS(1, "v11", "V1.1")
	ROMX_LOAD( "486-fx3000.bin", 0x10000, 0x10000, CRC(af303f08) SHA1(65dfa2541d2b08746f91012a2ae0121636402aac), ROM_BIOS(1))
ROM_END

ROM_START( um82c481af )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 40-0100-001266-00101111-121291-UMCAUTO-0 - 3DIUD-1.2
	// Chipset: // UMC UM92C206F, UM82C482AF, UM82C481BF - MB manufacturer according to BIOS is Modula Tech Co
	// ISA8: 1, ISA16: 6 - RAM: 8xSIMM30, Cache: 9x28pin, used: 4xIS61C256AH-20N, 1xW2465AK-20, CPU: AMD 386DX-40
	// CPU and FPU sockets provided - BIOS: AMI 386 BIOS, Keyboard-BIOS: AMI
	ROM_SYSTEM_BIOS(0, "3diud", "386 UMC 3DIUD")
	ROMX_LOAD( "386-umc-3flud.bin", 0x10000, 0x10000, CRC(2e795a01) SHA1(02e9e2871c1c1a542f44ab5eef66aee4b04225c1), ROM_BIOS(0))
	// 1: BIOS: Microid Research; 02/26/93 - BIOS-String: MR BIOS (r) V1.44
	ROM_SYSTEM_BIOS(1, "mr1441", "MR BIOS V1.44 #1") // resets continuously
	ROMX_LOAD( "3umr001.bin", 0x10000, 0x10000, CRC(466a115e) SHA1(077d797c653528062f1c87b03c608427c35c5505), ROM_BIOS(1))
	// 2: BIOS : MR 386DX/86DLC BIOS V.1.44 - Keyboard BIOS: JETkey V3.0 - CPU: TI 486DLC-40BGA, FPU: IIT 4C87DLC-40 - RAM: 8xSIMM30, Cache
	// OSC: 14.31818, 80.000MHz - ISA8: 1, ISA16: 6
	ROM_SYSTEM_BIOS(2, "mr1442", "MR BIOS V1.44 #2") // resets continuously
	ROMX_LOAD( "um82481m.bin", 0x10000, 0x10000, CRC(f617e1ce) SHA1(73ee80cb9f50547f26adbe0bfd2435b01728dd09), ROM_BIOS(2))
ROM_END


//**************************************************************************
//  80386 SX and DX Desktop
//**************************************************************************

// Schneider 386SX VGA System 40 (the number indicates the size of the harddisk, there were System 70 as well) - uses the same case as the Schneider Tower AT
// Schneider Tower VGA I/O: Chipset: WD37C65BJM, BIGJIM 50773 1108-0056, two other bigger chips can't be read on the photos
// 104 pin CPU card connector (ISA without the key), 4xISA16, 1xISA8 - on board: IDE, parallel, serial, bus mouse (Atari compatible), VGA, internal floppy (26pin), external floppy (DB25)
// On board graphics: ATI VGA Wonder-16 (256KB), ATI18800-1 1138-0069
// CPU card: CPU: Intel NG680386SX-16 (C-Step), FPU socket provided - Chipset: DDA14-075E, Chips P82C812, P82C811, P82C206, P82C215-12 (16MHz) - RAM: 8xSIMM30
// OSC: 20.000, 14.31818, 24.000000MHz, 32.000000MHz, - keyboard
// beeps 1-2-4
ROM_START( tower386sx )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "v103", "V1.03") // from a 386SX System 70
	ROMX_LOAD("t386s103.bin", 0x10000, 0x10000, CRC(d4e177e6) SHA1(fa11d49d629cdcac4467a9deedd25171ae499346), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v100", "V1.00") // from a 386SX System 40
	ROMX_LOAD("schneider_ag_386sx_bios_1_version_1.00a_id.nr.52504.u16", 0x10000, 0x8000, CRC(2fec2d3a) SHA1(4227da07f6652b89b9d02d7570ad0476672fd80d), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("schneider_ag_386sx_bios_0_version_1.00a_id.nr.52504.u15", 0x10001, 0x8000, CRC(b3331429) SHA1(b214bccfb62add9caea3d734885bc945b868967a), ROM_SKIP(1) |ROM_BIOS(1))

	// models upgraded to 512KB video memory were sold as "CEG" models as the memory upgrade enabled some sort of antialiasing ("continuous edge graphics")
	// in a 256 color mode with a choice from 792.096 colors.
	// according to https://archive.org/stream/byte-magazine-1991-01/1991_01_BYTE_16-01_1990_BYTE_Award_of_Excellence#page/n197/mode/2up this needs an EDSUN D/A chip, it is unknown
	// if it's contained on the platter or on the graphics upgrade piggyback card
	// The 12.5MHz version of the towerat2xx (VGA Tower System 40 or 70) used the same I/O backplane and were also offered with the CEG upgrade.
	ROM_REGION16_LE(0x10000, "vga", 0)
	ROM_LOAD16_BYTE("schneider_ag_vga_bios_low_v1.00_id.nr_51368.u13", 0x0000, 0x8000, CRC(ec4ef170) SHA1(0049ae5eab1a21838e674cf77e88994b954b1da3))
	ROM_LOAD16_BYTE("schneider_ag_vga_bios_high_v1.00_id.nr_51368.u14", 0x0001, 0x8000, CRC(5354962a) SHA1(11a503473e2011f323cc81c0b63d24f231c54c31))
ROM_END

// Atari PC 5 - American Megatrends 386XT Series-4 motherboard - on board EGA
// screen remains blank, 1 beep repeated (DRAM refresh failure)
ROM_START( ataripc5 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD32_BYTE("ami_pc5_1.00_rom0.bin", 0x00000, 0x8000, CRC(496149a6) SHA1(81033b22af830af8306abfde03a194739fe54355))
	ROM_LOAD32_BYTE("ami_pc5_1.00_rom1.bin", 0x00001, 0x8000, CRC(3c82fe66) SHA1(dd6c2c3c3635761b1d928912269b8937cbdc09ae))
	ROM_LOAD32_BYTE("ami_pc5_1.00_rom2.bin", 0x00002, 0x8000, CRC(7dc5b53b) SHA1(33e138baa84a8acc629bde5a6b54e47d0d4508f1))
	ROM_LOAD32_BYTE("ami_pc5_1.00_rom3.bin", 0x00003, 0x8000, CRC(b588b7a8) SHA1(2f2597b14e54d03cf957cce47536266f68d3aa66))
ROM_END

// Datavan Book-Size LAN Station - CPU: Am386SX/SXL-25 - Chipset: Headland HT18/C, RAM: 4xSIMM30
// BIOS: AMI 386SX BIOS PLUS S/NO. 232659 - Keyboard-BIOS: AMI KEYBOARD BIOS PLUS S/NO. 232659
// BIOS-String: 30-0100-ZZ1612-00101111-070791-HT18-F - ISA16: 1, used for LAN board - on board: 2xser, par, VGA
// Video: Realtek RTG3106 VGA, 1024K, MUSIC TR9C1710-66PCA - VGA BIOS: REALTEK / QUADTEL
// Mass storage: NEC FD1138H,  Quantum ProDrive ELS 977/10/17 - unknown DP8390DN based 8bit ISA LAN card
ROM_START( dvbslan ) // 3 short beeps =>  base memory read/write test error in the first 64 KB block of memory
	ROM_REGION16_LE( 0x20000, "bios", 0)
	ROM_LOAD( "ami_bios_386sx_bios_plus.bin", 0x10000, 0x10000, CRC(760afd89) SHA1(452f3e1b8b3b3d00df90c3501c8796de447a2184))

	ROM_REGION( 0x8000, "vga", 0) // REALTEK VGA BIOS
	ROM_LOAD( "realtek_vga.bin", 0x0000, 0x8000, CRC(4e416975) SHA1(826b7ec5494dc83cfcf25922185777bf6db46949))

	ROM_REGION( 0x2000, "lan", 0) // boot ROM from DP8390DN based LAN adapter
	ROM_LOAD( "net_isa_boot.bin", 0x0000, 0x2000, CRC(5bb527fd) SHA1(1eb7c82ef99d64eedffc3ac9c145d33b06cd8cb6))
ROM_END

// Epson PC AX3 - see epsax
ROM_START( epsax3 )
	ROM_REGION16_LE( 0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "evax3.bin", 0x10000, 0x8000, CRC(fec03cdc) SHA1(549027e6b786fa98a14912c1fb6d44c607206253))
	ROM_LOAD16_BYTE( "odax3.bin", 0x10001, 0x8000, CRC(0162d959) SHA1(f17741282e078a66e38d82ca48455b17c54e832f))
ROM_END


//**************************************************************************
//  80386 SX and DX Laptop/Notebook
//**************************************************************************

// Nixdorf N8810/20 - CPU: Intel 80386DX-20 clocked at 20MHz or 8MHz, Motherboard: FT-3 DFUP0279ZAB3
// Chipset: VLSI 9021BT 201141 VL16C452-QC, G2 GC132-PC, SED1345F, Headland HT133/A1A4324, Bt476KP50 128 9020
// RAM: 1MB, upgradeable via memory cards, this example has 4MB ROM: Phoenix keyboard ROM (missing dump)
// Mass storage: Floppy 3.5" 1.44MB, 3.5" HDD 40MB JVC JDE 3848V10-1A, Video: Paradise PVGA1A-JK
// Display: 12" bw double supertwisted LCD VGA, Ports: 2xser, Par, beeper, Bus: ISA16bit: 1, OSC: 28.6838M, 40.000MHz
// Origin: Software dump of the upper 256K
ROM_START( n8810m20 ) // "Keyboard clock line failure", but keyboard works
	ROM_REGION32_LE(0x20000, "bios", 0 )
	ROM_LOAD( "n8810m20_trim.bin", 0x00000, 0x20000, CRC(21b5394d) SHA1(691f50e65398e5c775dedbd9bda71a95ba6a6a29))
	// LCD Parameter Copyright MEI 1989 DAFT3C1 (missing dump), Keyboard-BIOS: Compability Software Phoenix Technologies LTD RBIOS Even 3ND1A4 Odd 3N1B4 (missing dump)
ROM_END

// Toshiba T3200SXC - CPU: 80386sx-20 - Floppy - 120 MB Hard disk - 1 MB RAM on board, 4 MB extra memory already installed. Can be upgraded to a total of 13 MB.
// TFT Display, 640x480, 256 colors - WD 90C21 video chip, 256 KB RAM - ISA8: 1, ISA16: 1
ROM_START( tot3200sxc ) // KBC ERROR
	ROM_REGION16_LE( 0x20000, "bios", 0)
	ROM_LOAD( "012c_t3200sxc_tc57h1024d-85.bin", 0x00000, 0x20000, CRC(5804a3da) SHA1(922dfb35b134a91a4c39e443597dad6798ce69d9))
ROM_END

// Sanyo MBC-18NB notebook - no display
ROM_START( mbc18nb )
	ROM_REGION16_LE( 0x20000, "bios", 0)
	ROM_LOAD( "sanyo_18nb.bin", 0x00000, 0x20000, CRC(64e283cf) SHA1(85ce4074c23b388d66e53cc83a8535bf7a2daf1f))
ROM_END

// Siemens-Nixdorf PCD-3Nsx notebook
// CPU: Intel NG680386SX-16 or -20 - 1MB RAM,upgradeable to 5MB, 40MB harddisk (-16 model)
// 2MB, upgradeable to 8MB, 80MB harddisk (-20 model), 9" mono VGA LCD, 1.44MB floppy drive
// Chipset: Headland HT21/E, , DP8473V, CHIPS F82C601, DS??87, unknown QFP100, ADC0833BCN (on PCU sub), CL-GD610-320C-C+CL-GD620-C
// Microcontrollers: N8042AH (KBC),  N80C51RH  (KBE)
// OSC: 14.318, 32.00024.0090
ROM_START( pcd3nsx )
	ROM_REGION16_LE( 0x20000, "bios", 0 )
	ROM_LOAD( "337-7_2400_3f_10-15-91.bin", 0x00000, 0x20000, CRC(99befce7) SHA1(150cd6a1476ca0ea970a1103b2a2c668c984433a) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD("33b_pcu_rev_3r.bin", 0x0000, 0x1000, CRC(d99308ec) SHA1(26621db2db37ab2bd1da972abb7398f9514329b2) )

	ROM_REGION( 0x800, "kbc", 0 )
	ROM_LOAD("kbc_c3f.bin", 0x000, 0x800, NO_DUMP)

	ROM_REGION( 0x1000, "kbe", 0 )
	ROM_LOAD("kbe_e3d.bin", 0x0000, 0x1000, NO_DUMP)
ROM_END

// Siemens-Nixdorf PCD-3Nsl - CPU: 80386SL@25MHz - 2MB RAM, upgradeable to 8MB - 85MB harddisk - 10" mono LCD VGA
// 1.44MB floppy drive
ROM_START( pcd3nsl )
	ROM_REGION16_LE( 0x20000, "bios", 0 )
	// Phoenix 80386 ROM BIOS PLUS Version 1.10.00 - failure at 100000-10FFFF - Resume memory backup failure
	ROM_SYSTEM_BIOS(0, "05-13-93", "05/13/93")
	ROMX_LOAD( "3n102l30.bin", 0x00000, 0x20000, CRC(02384c19) SHA1(552dc41b40272027e2b031187f8ab1e1513751b9), ROM_BIOS(0) )
	// Phoenix 80386 ROM BIOS PLUS Version 1.10.00 - Memory high address failure at 100000-10FFFF - Resume memory backup failure
	ROM_SYSTEM_BIOS(1, "01-26-94", "01/26/94")
	ROMX_LOAD( "3n120l40.bin", 0x00000, 0x20000, CRC(1336dd75) SHA1(80306d85f417c51a5235ac2f02ceb58bdb51205f), ROM_BIOS(1) )
ROM_END


// Toshiba T2000SX
// 1MB RAM on board, up to 9MB with 2MB, 4MB or 8MB expansion cards - 16 level grayscale VGA 640x480 display, PVGA1F display controller, 256KB VRAM
// Super integration (SI), components: DMAC 82C37Ax2, PIC 82C59Ax2, PIT 82C54, FDC TC8565, SIO TC8570 - 80C42 and 80C50 for keyboard - RTC 146818AF
// 128KB ROM, 32KB Backup RAM - GA-SYS CNT System control gate array - GA-IO CNT I/O gate array
ROM_START( t2000sx )
	ROM_REGION16_LE( 0x20000, "bios", 0 )
	ROM_LOAD( "014d.ic9", 0x00000, 0x20000, CRC(e9010b02) SHA1(75688fc8e222640fa22bcc90343c6966fe0da87f))
ROM_END


// Triumph-Adler Walkstation 386 SX - German version of the Olivetti S20
// VLSI VL82C320 + VL82C331; DP8473V
ROM_START( walk386sx )
	ROM_REGION16_LE( 0x20000, "bios", 0 ) // contains Cirrus Logic VGA BIOS
	ROM_LOAD( "cthj01_1014.bin", 0x00000, 0x20000, CRC(805084b9) SHA1(a92d78050844ccbcce53109c42603639aedd2335) )

	ROM_REGION( 0x2000, "mcu", 0 ) // MC68HC805B6FN
	ROM_LOAD( "cthj02_08_76.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x1000, "cop888", 0 ) // COPCL888-RDT/V
	ROM_LOAD( "s9124ab_c4_e904-34162_00.bin", 0x0000, 0x1000, NO_DUMP )
ROM_END

// Triumph-Adler Walkstation 386DX - German version of the Olivetti D33
// VLSI TOPCAT chipset: VL82C330 + VL82C331 + VL82C332 + VL82C106; Austek A38202C; DP8473V
// Video board: Cirrus Logic CL-GD610 + CL-GD620 + CL-GD63
ROM_START( walk386dx )
	ROM_REGION32_LE( 0x20000, "bios", 0 ) // contains Cirrus Logic VGA BIOS
	ROM_LOAD( "am28f010_ctaa060125rc.bin", 0x00000, 0x20000, CRC(6cc540fe) SHA1(9853793d5433bbc5efc09c7f31c4a8a8f78d4549) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "cthj02_03_76.bin", 0x0000, 0x2000, NO_DUMP )
ROM_END

// Siemens PG 750 - Luggable programmer ("Programmiergerät") for the Siemens S5 automation systems, a 486 EISA version exists as well
// Chipset: CHIPS P82C302, P82C301C, P82C206, four more "CHIPS" ICs
// ISA16: 7
// on board: V24/Mouse, V24/Modem, Printer
ROM_START( pg750 )
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	// 0: Phoenix 80386 ROM BIOS PLUS Version 1.10 14 / SIEMENS PG-750
	// Time-of-day clock stopped
	// EGA/TIGA Graphics System "Highgraph II"
	ROM_DEFAULT_BIOS("v402")
	ROM_SYSTEM_BIOS(0, "v40", "v40")
	ROMX_LOAD( "pg750_4.0_386_l.bin", 0x10000, 0x8000, CRC(1e6dcd40) SHA1(2def9f729f43652a7b8b32a42e4c073f580d39ce), ROM_SKIP(1)|ROM_BIOS(0) )
	ROMX_LOAD( "pg750_4.0_386_h.bin", 0x10001, 0x8000, CRC(389c20dd) SHA1(eb0e86ba88ac9742868689b2aac9911ed7acac74), ROM_SKIP(1)|ROM_BIOS(0) )
	// 1: Phoenix 80386 ROM BIOS PLUS Version 1.10 17 / SIEMENS PG-750/730
	// Time-of-day clock stopped
	ROM_SYSTEM_BIOS(1, "v402", "v402")
	ROMX_LOAD( "pg750_4.02_386_l.bin", 0x10000, 0x8000, CRC(208aac51) SHA1(49d50b7ade8f56bda203375a9b138adf2cb5e500), ROM_SKIP(1)|ROM_BIOS(1) )
	ROMX_LOAD( "pg750_4.02_386_h.bin", 0x10001, 0x8000, CRC(c6e14fb6) SHA1(dad6ab5c5a18341ec0dce53fe712b8367340506b), ROM_SKIP(1)|ROM_BIOS(1) )
ROM_END

//**************************************************************************
//  80486 BIOS
//
//  BIOS files that show "original.tmp" near the beginning are compressed
//  and can be unpacked using, e.g., 7-ZIP.
//**************************************************************************

ROM_START( at486 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 30-0500-ZZ1130-00101111-070791-1219-0 /PAI JUNG ELECTRONIC IND. CO., LTD.
	ROM_SYSTEM_BIOS(0, "at486", "PC/AT 486")
	ROMX_LOAD( "at486.bin",   0x10000, 0x10000, CRC(31214616) SHA1(51b41fa44d92151025fc9ad06e518e906935e689), ROM_BIOS(0))
	// 1: BIOS-String: 40-0100-009999-11101111-070791-UMC480A-0 / United Microelectronics Corporation (UMC) MG-48602
	ROM_SYSTEM_BIOS(1, "mg48602", "UMC MG-48602")
	ROMX_LOAD( "mg48602.bin", 0x10000, 0x10000, CRC(45797823) SHA1(a5fab258aecabde615e1e97af5911d6cf9938c11), ROM_BIOS(1))
	// 2: BIOS-String: 40-0000-001470-00101111-060692-SIS3486-0 / 24X-VS-XX-B
	ROM_SYSTEM_BIOS(2, "ft01232", "Free Tech 01-232")
	ROMX_LOAD( "ft01232.bin", 0x10000, 0x10000, CRC(30efaf92) SHA1(665c8ef05ca052dcc06bb473c9539546bfef1e86), ROM_BIOS(2))

	/* 486 boards from FIC
	naming convention
	xxxxx101 --> for EPROM
	xxxxx701 --> for EEPROM using a Flash Utility v5.02
	xxxxBxxx --> NS 311/312 IO Core Logic
	xxxxCxxx --> NS 332 IO Core Logic
	xxxxGxxx --> Winbond W83787F IO Core Logic
	xxxxJxxx --> Winbond W83877F IO Core Logic
	*/
	// 3: BIOS-String: 06/16/97-VT82C486A-214L2000-00 / Version 3.276GN1
	/* this is the year 2000 beta bios from FIC, supports GIO-VT, GAC-V, GAC-2, VIP-IO, VIO-VP and GVT-2 */
	ROM_SYSTEM_BIOS(3, "ficy2k", "FIC 486 3.276GN1") /* includes CL-GD5429 VGA BIOS 1.00a */
	ROMX_LOAD( "3276gn1.bin",  0x00000, 0x20000, CRC(d4ff0cc4) SHA1(567b6bdbc9bff306c8c955f275e01ae4c45fd5f2), ROM_BIOS(3))
	// 4: BIOS-String: 04/29/94-VT82C486A-214L2000-00 / Award Modular BIOS v4.50
	ROM_SYSTEM_BIOS(4, "ficgac2", "FIC 486-GAC-2") /* includes CL-GD542X VGA BIOS 1.50 */
	ROMX_LOAD( "att409be.bin", 0x00000, 0x20000, CRC(c58e017b) SHA1(14c19e720ce62eb2afe28a70f4e4ebafab0f9e77), ROM_BIOS(4))
	// 5: BIOS-String: 04/08/96-VT82C486A-214L2000-00 / Version 3.27GN1
	ROM_SYSTEM_BIOS(5, "ficgacv", "FIC 486-GAC-V 3.27GN1") /* includes CL-GD542X VGA BIOS 1.41 */
	ROMX_LOAD( "327gn1.awd",   0x00000, 0x20000, CRC(017614d4) SHA1(2228c28f21a7e78033d24319449297936465b164), ROM_BIOS(5))
	// 6: BIOS-String: 05/06/94-VT82C486A-214L2000-00 / Version 3.15GN - ISA16:4, ISA/VL: 2
	ROM_SYSTEM_BIOS(6, "ficgiovp", "FIC 486-GIO-VP 3.15GN") // Chipset: VIP VT82C486A, Promise PDC20230C, one further VIA, one other unreadable
	ROMX_LOAD( "giovp315.rom", 0x10000, 0x10000, CRC(e102c3f5) SHA1(f15a7e9311cc17afe86da0b369607768b030ddec), ROM_BIOS(6))
	// 7: BIOS-String: 11/20/94-VT82C486A-214L2000-00 / Version 3.06G (11/25/94) - OSC: 24.0L3P - ISA16:3, ISA/VL: 2
	ROM_SYSTEM_BIOS(7, "ficgiovt", "FIC 486-GIO-VT 3.06G") // 1994-11-20 - Chipset: Winbond W83757AF, W83758P, VIA VT82C486A, VT8255N, VT82C482
	ROMX_LOAD( "306gcd00.awd", 0x10000, 0x10000, CRC(75f3ded4) SHA1(999d4b58204e0b0f33262d0613c855b528bf9597), ROM_BIOS(7))
	// 8: BIOS-String: 11/02/94-VT82C486A-214L2000-00 Version 3.07G - ISA8: 1, ISA16: 4, ISA/VL: 2
	ROM_SYSTEM_BIOS(8, "ficgvt2", "FIC 486-GVT-2 3.07G") // Chipset: VIA VT82C486A, VT82C482, VIA VT8255N
	ROMX_LOAD( "3073.bin",     0x10000, 0x10000, CRC(a6723863) SHA1(ee93a2f1ec84a3d67e267d0a490029f9165f1533), ROM_BIOS(8))
	// 9: BIOS-String: 06/27/95-VT82C505-2A4L4000-00 / Version 5.15S - Chipset: S3 Trio64, FDC 37665GT, VT82C496G, VT82C406MV
	ROM_SYSTEM_BIOS(9, "ficgpak2", "FIC 486-PAK-2 5.15S") /* includes Phoenix S3 TRIO64 Enhanced VGA BIOS 1.4-01 */
	ROMX_LOAD( "515sbd8a.awd", 0x00000, 0x20000, CRC(778247e1) SHA1(07d8f0f2464abf507be1e8dfa06cd88737782411), ROM_BIOS(9))
	// 10: BIOS-String: 04/01/96-VT496G-2A4L6F0IC-00 0000C-00 - runs into Award BootBlock BIOS - Chipset: VIA VT82C505, VT82C416, VT82C496G, Winbond W83787F
	ROM_SYSTEM_BIOS(10, "ficpio3g7", "FIC 486-PIO-3 1.15G705") // pnp  - ISA16: 4, PCI: 3
	ROMX_LOAD( "115g705.awd",  0x00000, 0x20000, CRC(ddb1544a) SHA1(d165c9ecdc9397789abddfe0fef69fdf954fa41b), ROM_BIOS(10))
	// 11: BIOS-String: 04/01/96-VT496G-2A4L6F0IC-00 0000C-00 - runs into Award BootBlock BIOS
	ROM_SYSTEM_BIOS(11, "ficpio3g1", "FIC 486-PIO-3 1.15G105") /* non-pnp */
	ROMX_LOAD( "115g105.awd",  0x00000, 0x20000, CRC(b327eb83) SHA1(9e1ff53e07ca035d8d43951bac345fec7131678d), ROM_BIOS(11))
	// 12: BIOS-String: 11/27/96-VT496G-2A4L6F0IC-00 0000C-00 - runs into Award BootBlock BIOS
	ROM_SYSTEM_BIOS(12, "ficpos", "FIC 486-POS")
	ROMX_LOAD( "116di6b7.bin", 0x00000, 0x20000, CRC(d1d84616) SHA1(2f2b27ce100cf784260d8e155b48db8cfbc63285), ROM_BIOS(12))
	// 13: BIOS-String: 06/27/95-VT82C505-2A4L4000-00 / Version 5.15 / Chipset: VIA VT82C496G PC/AT
	ROM_SYSTEM_BIOS(13, "ficpvt", "FIC 486-PVT 5.15") // ISA16: 6, ISA/VL: 2
	ROMX_LOAD( "5150eef3.awd", 0x00000, 0x20000, CRC(eb35785d) SHA1(1e601bc8da73f22f11effe9cdf5a84d52576142b), ROM_BIOS(13))
	// 14: BIOS-String: 10/05/95-VT82C505-2A4L4000-00 / Version 5.162W2(PCTIO)
	ROM_SYSTEM_BIOS(14, "ficpvtio", "FIC 486-PVT-IO 5.162W2")  // Chipset: VT82C406MV, VT82C496G, W83777/83787F, W83758P
	ROMX_LOAD( "5162cf37.awd", 0x00000, 0x20000, CRC(378d813d) SHA1(aa674eff5b972b31924941534c3c988f6f78dc93), ROM_BIOS(14))
	// 15: BIOS-String: 40-00AG-001247-00101111-060692-SIS3486-0 / AV4 ISA/VL-BUS SYSTEM BIOS / Chipset: SIS 85C460ATQ
	ROM_SYSTEM_BIOS(15, "ava4529j", "AVA4529J") // this is a board with two VLB slots
	ROMX_LOAD("amibios_486dx_isa_bios_aa4025963.bin", 0x10000, 0x10000, CRC(65558d9e) SHA1(2e2840665d069112a2c7169afec687ad03449295), ROM_BIOS(15))
	// 16: BIOS-String: 40-0200-001291-00101111-111192-OPT495SX-0 / 34C-OP-WBp-25/33/40/50-D5-ZZ
	// Chipset: OPTi 82C495SX - CPU: 486DX - BIOS: AMI 486DX ISA BIOS AA7524842 - ISA8: 1, ISA16: 4, ISA16/VL: 2
	ROM_SYSTEM_BIOS(16, "pat48pv", "PAT-48PV")
	ROMX_LOAD("pat48pv.bin", 0x10000, 0x10000, CRC(69e457c4) SHA1(7015b2bccb10ce6e1ad6e992eac785f9d59a7a24), ROM_BIOS(16))
	// 17: Morse P1 V3.10 - CPU: 486DX - ISA8: 2, ISA16: 6 - Chipset: Morse 91A401A- Award Modular BIOS v4.20 / V3.00 - KEYBOARD ERROR OR NO KEYBOARD PRESENT
	ROM_SYSTEM_BIOS(17, "p1", "P1")
	ROMX_LOAD("morse_p1.bin", 0x10000, 0x10000, CRC(23d99406) SHA1(b58bbf1f66af7ed56b5233cbe2eb5ab623cf9420), ROM_BIOS(17))
	// 18: Chipset: SiS 85C206 CONTAQ 82C592 82C591 - CPU/FPU: 486, socket provided - OSC: 33.333MHz, 14.31818 - BIOS: AMI 486DX ISA BIOS AA0083611 (28pin)
	// BIOS-String: 40-0700-D01508-00101111-070791-CTQ 486-0 - ISA8: 2, ISA16: 6
	ROM_SYSTEM_BIOS(18, "82c591", "82C591")
	ROMX_LOAD("486-contaq.bin", 0x10000, 0x10000, CRC(e5d2cf16) SHA1(1357a964ef78eaad6894dcc9dce62be50cdf6df5), ROM_BIOS(18))
	// 19: Chipset: PCCHIPS CHIP 16 (9430-AS), CHIP 18 (9432-AS) - CPU: i486DX2-66 - BIOS: AWARD (28pin) - ISA16: 4, ISA16/VL: 3 - OSC: 14.31818MHz
	// BIOS-String: 07/13/94--2C4X6H01-00 / Release 07/15/94'
	ROM_SYSTEM_BIOS(19, "chips", "Chips")
	ROMX_LOAD("486-pcchips.bin", 0x10000, 0x10000, CRC(4e49eca1) SHA1(2343ca9f4760037eb2ef6e7b011b9690e542d6ea), ROM_BIOS(19))
	// 20: CAM/33(50)-P8 M458(A)P80 - Chipset: Opti 82C495SX, F82C206Q 82C392SX - CPU: 486DX-33 (solder pads for 486sx and 486DX) - OSC: 14.318MHz, 33.000MHz
	// Keyboard-BIOS: AMI Keyboard BIOS PLUS A317473 - BIOS: AMI 486 BIOS PLUS 214097 (28pin) - RAM: SIMM30x8 - Cache: 1xIS61C256A, 8xUM61256BK-25 - ISA8: 1, ISA16: 6
	// BIOS-String: X0-0101-001105-00101111-060692-495SX_A-0 / 486DX/SX CAM/33,50-P8, CPM/25,33-P8, 12/14/1992
	ROM_SYSTEM_BIOS(20, "cam33", "CAM/33")
	ROMX_LOAD("486-cam.bin", 0x10000, 0x10000, CRC(d36a13ea) SHA1(14db51dbcf8decf1cb333c57a36971ef578c89b4), ROM_BIOS(20))
	// 21: 486-PIO3 1.1 - Chipset: Winbond W83787F, VIA VT82C505, VT82C416, VT82C496G - ISA16: 4, PCI:3 - BIOS: AWARD F 4825803 1.14G705 (32pin) - CPU: Socket 3
	// RAM: 2xSIMM72, Cache: 9 sockets marked SRAM 128Kx8 (2 banks +1) - On board: 2xIDE, Floppy, par, 2xser
	// BIOS-String: 02/01/96-VT496G-2A4L6F0IC-00 0000C-00 . runs into BootBlock BIOS
	ROM_SYSTEM_BIOS(21, "pio3", "486-PIO-3")
	ROMX_LOAD("486-pio3.bin", 0x00000, 0x20000, CRC(1edb5600) SHA1(36887cd08881dfa063b37c7c11a6b65c443bd741), ROM_BIOS(21))
	// 22: 486 G486IP IMS - Chipset: IMS 8848 IMS 8849 - CPU: i486DX2-66 - BIOS: AMI 486DX ISA BIOS AB5870352 - Keyboard-BIOS: MEGAKEY (AMI/Intel) - ISA8: 1, ISA16: 4, PCI: 3
	// RAM: SIMM30: 4, SIMM72: 2, Cache: 10 sockets (UM61256AK-15) - BIOS-String: 41-0000-ZZ1124-00101111-060692-IMS8849-0 / PCI BIOS, Dated JUN-16-94 / FOR G486IP
	ROM_SYSTEM_BIOS(22, "g486ip", "G486IP")
	ROMX_LOAD("g486ip_ims.bin", 0x00000, 0x20000, CRC(4431794a) SHA1(f70e8c326455229c3bb7f305c2f51c4ac11979ed), ROM_BIOS(22))
	// 23: EFA 486 UPIO
	// BIOS-String: 11/09/95-UMC-881/886A-2A4X5E39C-00 00 / N486U-PIO/A, Rev 1.03 ROM - NOT FOR SALE - boots into BootBlock BIOS
	ROM_SYSTEM_BIOS(23, "486upio", "486 UPIO")
	ROMX_LOAD("upio_103.bin", 0x00000, 0x20000, CRC(4e9139cd) SHA1(f2b00356957c712ca652c3751b31161b3110ec69), ROM_BIOS(23))
	// 24: Acer 486 Version 2.2 - Chipset: ALi M1429, M1431 - CPU: 486 - RAM: SIMM30x8
	// screen remains blank
	ROM_SYSTEM_BIOS(24, "acer48622", "Acer 486 V2.2.")
	ROMX_LOAD("4alm002.bin", 0x10000, 0x10000, CRC(88291af2) SHA1(7ff912e9f0550631377d1a4c3aa266a081e7dce9), ROM_BIOS(24))
	// 25: ACR6BE00-M00-940720-R01-E0 / BIOS V2.0 - Keyboard Interface Error, Pointing DeviceInterface Error
	ROM_SYSTEM_BIOS(25, "4alo001", "4ALO001")
	ROMX_LOAD("4alo001.bin", 0x00000, 0x20000, CRC(4afb9c50) SHA1(5e56682ba1e04bd0b074de3b2a93fb5322325d01), ROM_BIOS(25))
	// 26: dies after initialising the graphics card
	ROM_SYSTEM_BIOS(26, "4alp001", "4ALP001")
	ROMX_LOAD("4alp001.bin", 0x10000, 0x10000, CRC(9b4a2881) SHA1(f324bb0304164e9ede1dd2eebb085a76aae398be), ROM_BIOS(26))
	// 27: BIOS-String: 30-0500-ZZ1130-00101111-070791-1219-0
	ROM_SYSTEM_BIOS(27, "zz1130", "ZZ1130")
	ROMX_LOAD("4zzw001.bin", 0x10000, 0x10000, CRC(dc21c952) SHA1(affdc4efbca4dad561e4f0141463844ec84ae519), ROM_BIOS(27))
	// 28: screen remains blank
	ROM_SYSTEM_BIOS(28, "optimus", "Optimus")
	ROMX_LOAD("mb_bios_ami_930808.bin", 0x10000, 0x10000, CRC(89151d5b) SHA1(92a93cae054525adfdc6277a1236e699ea9fbc32), ROM_BIOS(28))
	// 29: 40-0100-DG1112-00101111-070791-UMC480A / Rev. 251191 UMC-486A - ASI board
	// The BIOS comes from http://www.elhvb.com/supportbios.info/Archives/BIOS/0-A/ASI/UMC-486A/index.html and contains 28 extra plain textattr
	// bytes at the end of the file. These have been lopped off, but the emulated machine complains about a ROM error, thus marked BAD_DUMP
	ROM_SYSTEM_BIOS(29, "umc486a", "UMC-486A")
	ROMX_LOAD("umc481icorr.ami", 0x10000, 0x10000, BAD_DUMP CRC(d27b2fd4) SHA1(e639dbc7d65b29ffca26701af766fa75bfe33787), ROM_BIOS(29))
	// 30: 40-0201-D41107-00101111-031591-OPBC-0
	ROM_SYSTEM_BIOS(30, "a9c11f1f", "a9c11f1f")
	ROMX_LOAD("ami_486_zz686886.bin", 0x10000, 0x10000, CRC(a9c11f1f) SHA1(2a27ecae9547ddd3d230c30a94deb83a4d6b4436), ROM_BIOS(30))
	//31: Award Modular BIOS v4.20 / Version 1.09K
	ROM_SYSTEM_BIOS(31, "109k", "1.09K")
	ROMX_LOAD("award_486dx_0097042.bin", 0x10000, 0x10000, CRC(b620534b) SHA1(d3777a82cb35639d386a1840dd5cf52527ec6f8b), ROM_BIOS(31))
	// 32: UNIC2 94V-0 9326 - OPTI chipset (3 chips)
	// BIOS-String: X0-0100-001378-00101111-060692-495SX_A
	ROM_SYSTEM_BIOS(32, "unic2", "UNIC 2 94V-0")
	ROMX_LOAD("amibios-486dx-1992-aa8707058-m27c512.bin", 0x10000, 0x10000, CRC(a2b3e326) SHA1(b29c5668fb3337893ef3a96f053f90b929bac0d6), ROM_BIOS(32))
	// 33: BIOS-String: 40-0100-001276-00101111-060692-495_X86-0
	// OPTi 82C495 SX, F82C206 - CPU: two solder pads and two CPU sockets - RAM: 8xSIMM30, Cach: 10x28pin DIP - ISA8: 2, ISA16: 3, ISA16/VLB: 3
	ROM_SYSTEM_BIOS(33, "495sx1", "495sx-1")
	ROMX_LOAD("495sx-1.bin", 0x10000, 0x10000, CRC(318a99a3) SHA1(e57380ed2a802cd1648a32317313ade5221f1213), ROM_BIOS(33))
	// 34: Chipset: marked QDI - CPU: Socket 3, 25MHz-50MHz - RAM: 4xSIMM30, 2xSIMM72, Cache: 8x32pin DIP (4x28pin DIP filled), 1x32pin DIP (filled with 28pin DIP)
	// BIOS: AMI 486DX ISA BIOS AC2849676 - OSC: 18.31818 - ISA8: 1, ISA16: 3, ISA16/VL: 3
	ROM_SYSTEM_BIOS(34, "qdi486", "QDI486") // screen remains blank
	ROMX_LOAD("qdi486.bin", 0x10000, 0x10000, CRC(f44a5a45) SHA1(57cfd7c6524eba21395642bd57b726b45eef4b6a), ROM_BIOS(34))
ROM_END


//**************************************************************************
//  80486 motherboard
//************************************************************************//

// Addtech Research 4GLX3 Green-B 4GPV3.1 aka VisionEX 4GPV3 - Chipset: Contaq 82C596A - BIOS Version: AMI 01/10/94 486 ISA BIOS AA 6729627
// Keyboard BIOS: AMI - CPU: Socket 3, solder pads for 80486DX/SX provided - RAM: 8xSIMM30, Cache: 9x28pin DIP - ISA16: 8, ISA16/VLB: 2
ROM_START( ar4glx3 ) // BIOS-String: 40-0101-006666-00101111-011094-CTQ-596A / KIM Computer by CTL Corporation / Model : [4GLX3] --- Made in U.S.A.
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "4glx3_bios.bin", 0x10000, 0x10000, CRC(3367e0b2) SHA1(8dbe58ed783c56ca2cb61ded6f603314739dcfb8))
ROM_END

// TMC PAT48AV 1.4 - ALi M1429 A1, M1431 A2 - BIOS: AMI 486DX ISA BIOS AB4179743 - Keyboard BIOS: AMI-KB-H-WP - CPU: 486DX socket
// RAM: 8xSIMM30 - Cache: 8xIS61C256AH-20N - OSC: 14.31818 - ISA16: 4, ISA16/VLB: 3
ROM_START( tmpat48av ) // screen remains blank - BIOS-String: 40-0106-001291-00101111-080893-ALI1429 / 486DX-AC-WBu-25/33/40/50-L6-ZZ
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "pat48av_bios_27c512.bin", 0x10000, 0x10000, CRC(a054b7b3) SHA1(edc2554a73aba94d586f8b49a5c5bbbe2890331c))
ROM_END

// BIOSTAR - MB-1433/50 AEA-P - V:1 - Chipset: VLSI 82C3480 & 83C3490 (marked BIOTEQ) - BIOS/Version: AMIBIOS 12/12/91 - Keyboard BIOS: MEGA-KB-F-WP
// CPU: It's ST 486Dx2-80 - RAM: 8xSIMM30, Cache: 8xW24M257AK-15, 1xW2465AK-15 - ISA8: 1, ISA16: 6 - OSC: 66.000MHz, 14.318
ROM_START( mb1433aeap ) // 40-0100-001223-00101111-121291-B82C3480-0 / M1433/50AEA-P
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "mb-1433-aea-p.bin", 0x10000, 0x10000, CRC(daac20f7) SHA1(b6889a4896f83b306e69efa87cae8d03147b6dbd))
ROM_END

// FIC ELI6-II (from Unisys ELI 46665 Desktop) - Chipset: VIA 82C486A, VT82C487, SMC FDC37C665QF P - BIOS label: ELI6-U555
// BIOS Version: Award U573 3/14/95 - CPU: Intel Overdrive DX40DPR100 in Socket 3 - RAM: 4xSIMM72, Cache: 9x256K-15 - ISA16:6 - On board: ISA, Floppy
ROM_START( ficeli6ii ) // BIOS String: 03/14/95-VT82C486A-214L2000-00 / Award Modular BIOS v4.50 /3.20G /F5 1.00 / UNISYS ELI6-II Version U573.BIN (03/14/95)
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "fic_eli6ii.bio", 0x10000, 0x10000, CRC(b61cc026) SHA1(59eae42bf1dea01ba04fecd9bb367e47d4a256d4))
ROM_END

// Mitac PWA-IH4077D - Chipset: EFAR EC802GL, EC100G, UMC UM82C863F, UM82C865F - CPU: Socket 3 - RAM: 4xSIMM72, Cache: 9xIS61C256AH-20N
// BIOS: known: R2.03/CKS: 2400H, ATT4077D BIOS R1.01.00/CKS: 3000H - Keyboard BIOS: Award KB-200 or VIA VT82C42N (on ATT4077D)
// On board: 2xser, par, Floppy, IDE - ISA16: 4, ISA17/VL: 2
ROM_START( pwaih4077d ) // BIOS-String: 04/02/98-EFAR-EC802G-B-2C403D31-00 / (IH4077D R2.08G)
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ih4077d.208g", 0x10000, 0x10000, CRC(0f834ba2) SHA1(7e08e454dfa3cf5079845fe61b9ae74b1dcc7981))
ROM_END

// ASUS ISA-486SIO rev. 1.2 - Chipset : SiS 85C460 ATQ, Winbond W85C16B - BIOS : AMI 486DX/ISA BIOS AA2310181 - Keyboard BIOS: AMI
// CPU: Intel 80486SX-25 - RAM: 8xSIMM30, Cache: 9x28pin DIP (used: 4xM5M5276P-25, 1xUM6164BK-20) - ISA8: 1, ISA16: 6 - OSC: 25.000MHz, 14.31818
ROM_START( a486sio )// BIOS-String : 40-0104-001292-00101111-050591-I486SI-F
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "isa-486sio.bin", 0x10000, 0x10000, CRC(f339f8ff) SHA1(d53f0ff30cc7f0c70ffeeda33d16dddbeedd6098))
ROM_END

// Micronics JX30GP - Motherboard P/N: 09-00189-10 REV B1 - Chipset: MIC 471, MIC491, PC87312VF (Super I/O), KS82C6818A -
// CPU: Socket 3, solder pads for 80486QFP - RAM: 4xSIMM72, Cache: 6xUM61256FK-15, 1xW24257AK-15 - DIP4: 0000 - OSC: 14.318
// ISA16: 5, ISA16/VLB: 2 - on board: Floppy, ISA, PS/2 keyboard and mouse
ROM_START( mijx30gp ) // BIOS: Phoenix, 80486 ROM BIOS PLUS Version 0.10 GJX30G-04P, Gateway 2000 Local Bus BIOS - shows Error 8602 - Auxiliary Device Failure
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "micronics_gjx30g-04p_09-24-93.bin", 0x10000, 0x10000, CRC(66477a66) SHA1(549eecf707bbb43bcdc89715b36cc23e3cb1a074))
	ROM_IGNORE(0x10000) // the second half of the 128K ROM seems to contain BIOS source code
ROM_END

// AMI 80486 EISA Enterprise-II - Chipset: AMI/Intel 82357 - RAM: 8xSIMM30 (32MB max), 128K cache memory - CPU: i486DX 25/33, Weitek WTL4167 FPU socket provided
// on board: Floppy, PS/2 mouse, memory card socket, i/o expansion module slot - EISA: 8
// BIOS-String: 41-0001-004616-00111111-070791-AMI-EP-0 - EISA CMOS INOPERATIONAL / Software Port NMI INOPERATIONAL / Fail-safe Timer NMI INOPERATIONAL / Keyboard error
ROM_START( amient2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "16092391.cs1", 0x00000, 0x20000, CRC(b0ae5c8d) SHA1(0d6cda74433a5d70e8fc2bec6a77ed97a09a984e))
ROM_END

// AMI Enterprise-III EISA Local Bus Series 68 - CPU: i486SX/DX/DX2 25/33/50/66MHz - RAM: 16xSIMM32 (up to 256MB RAM!!!), Cache 256K
// EISA: 6, EISA/VL: 2 - BIOS: Flash - repeated short single beeps (DRAM refresh error)
// BIOS-String: 41-0103-004668-00111111-111192-AMIS68
ROM_START( amient3 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "s68p.rom", 0x00000, 0x20000, CRC(7342b9ef) SHA1(da6b2312ccc175473443b1d562d5e4f3952cac5a))
ROM_END

// AMI ENTERPRISE-IV EISA VLB - CPU: 486SX/486SX/486DX/DX2/Pentium overdrive 25/33/50/66 MHz - RAM: 4xSIMM72 (256MB), Cache: 256KB
// EISA: 5, EISA/VL: 2 - on board: floppy, IDE, par, 2xser - screen remains blank
// BIOS-String: 41-0103-004687-00111111-111192-AMIS87
ROM_START( amient4 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "s87p.rom", 0x00000, 0x20000, CRC(ff092e7f) SHA1(4999278dca001de74dff518f1f1c9ea8212d7ed4))
ROM_END

// AMI Super Voyager PCI-II - Chipset: AMI, SMC - CPU: 486DX/DX2/DX4 - RAM: 4xSIMM72 (64MB), Cache: 5x32pin, 4x28pin+TAG (127/256K)
// BIOS: Flash - Keyboard-BIOS: AMIKEY - ISA16: 4, PCI: 3 - on board: Floppy, 2xIDE, 2xer, par - screen remains blank
ROM_START( amisvpci2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "s724p.rom", 0x00000, 0x20000, CRC(fa9ea9b3) SHA1(b0f2206a7f0d6a00e094f7d151c16022a5292858))
ROM_END

// AMI Super Voyager VLB - screen remains blank
// BIOS-String: 40-0103-004669-00111111-111192-AMIS69B2
ROM_START( amisvvlb )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "s69b2p.rom", 0x00000, 0x20000, CRC(3a13944c) SHA1(9d7733ee50023edd5b0bf4b098c9c6c35a4dc2b0))
ROM_END

// AMI Super Voyager VLB-II - AMI Super Voyager VLB-II - CPU: 486/Overdrive - Chipset: AMI, SMC FDC37C665GT
// RAM: 4xSIMM72, Cache: 8x28pin + TAG - ISA16: 5, ISA16/VL: 2 - BIOS: 28pin in a 32pin socket - on board: Floppy, IDE
// BIOS-String: 40-0100-004682-00111111-111192-AMIS82E
ROM_START( amisvvlb2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "s82p.rom", 0x00000, 0x20000, CRC(2e2d33ae) SHA1(349be4b29bc8a4447ab4c73fe4c695276565489f))
ROM_END

// AMI Super Voyager VLB-III - Chipset: FDC37C665GT, AMI - CPU: 486SX, 487DX, 486DX/DX2/DX4, Overdrive
// RAM: 4xSIMM72 - on board: mouse, par, 2xser, Floppy, IDE - ISA16: 5, ISA16/VL: 2 - screen remains blank, repeated single beeps
ROM_START( amisvvlb3 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "s707p.rom", 0x00000, 0x20000, CRC(5c0e55c0) SHA1(0c5232a21180d4541202ac6dd34677339b7cbecc))
ROM_END

// Edom 486VL3H MV020 - CPU: 486 - Chipset: HiNT CS8005, HMC HM82C206AQ - RAM: 8xSIMM30
// Cache: IS61C256AH-20Z (8), AE88128AK-15 - Keyboard-BIOS: JETkey V5.0 - ISA8: 2, ISA16: 3, ISA16/VLB: 3
// BIOS-String: 09/05/94-HINT-8005-214D1W00-00 / HiNT CS8005 FOR 486SX/DX/DX2 VL-BUS [MV020]
ROM_START( ed486vl3h )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "4hiw001.bin", 0x10000, 0x10000, CRC(760842fd) SHA1(2fe156f092c84cf385079da1209a8f1e06005f5e))
ROM_END

// MSI MS-4134 - Chipset: ALI M1429, M1431, M1435 - CPU: Socket 3, RAM: 4xSIMM30, 2xSIMM72, Cache: 8x32pin + TAG, used: 9xW2464AK-20
// ISA16: 3, ISA16/VL: 2, PCI: 3 - BIOS: AMI
// BIOS-String:
ROM_START( ms4134 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:  BIOS-String: 40-0105-421169-00101111-080893-1429GPCI-0 / AL57 111594
	ROM_SYSTEM_BIOS(0, "al57", "AL57")
	ROMX_LOAD( "al57.rom", 0x00000, 0x20000, CRC(6b6c2a11) SHA1(1e40ef8a7a7b3be057ba6a121abfd0d983d5d5c9), ROM_BIOS(0))
	// 1: BIOS-String: 40-0105-421169-00101111-0890893-1429GPCI-0 / AL51 5/23/1994
	ROM_SYSTEM_BIOS(1, "al51", "AL51")
	ROMX_LOAD( "4alm001.bin", 0x00000, 0x20000, CRC(0ea9f232) SHA1(5af1a0cf047b68a7070b8c45081a80e817aade84), ROM_BIOS(1))
ROM_END

// Abit 486 EISA-AE4 - Chipset: SiS 85C406, 85C411, three other SiS chips unreadable - CPU: 486, FPU socket provided - RAM: 8xSIMM30, Cache: 8x28pin
// ISA16: 2, EISA: 6 - BIOS: Award EISA486/CU - Keyboard-BIOS: NEC KB-BIOS VER:400 JU-JET 1989 -  OSC: 50.000MHz, 14.31818MHz
// Award Modular BIOS v4.20 / AE4 EISA SYSTEM BIOS
ROM_START( abae4 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ae4.bin", 0x10000, 0x10000, CRC(d9cbc3c6) SHA1(eeeaef7fd188598d477897f0248c99940cd1a5d7))
ROM_END

// FIC 486-KVD - Chipset: VIA VT82C485 - CPU: 486, solder pad for 486sx present - RAM: 8xSIMM30, Cache: 4xIS61C256A-20N+1xCY7C185-20PC, 4 empty sockets (28pin)
// ISA16: 5, ISA16/VL: 2 - BIOS: AMI 486DX ISA BIOS AA7211137 - Keyboard-BIOS: MEGA-KB-H-WP

ROM_START( fic486kvd )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: X0-0100-001121-00101111-021993-VIA-0
	ROM_SYSTEM_BIOS(0, "021993", "021993")
	ROMX_LOAD( "486kvd_aa72111137.bin", 0x10000, 0x10000, CRC(a1f1810f) SHA1(405afbf1635c6b41343aabfeeb3cf4cdc947a5ba), ROM_BIOS(0))
	// 1: BIOS-String: X0-0100-001121-00101111-021993-VIA-0
	ROM_SYSTEM_BIOS(1, "060692", "060692")
	ROMX_LOAD( "486-aa9615811.bin", 0x10000, 0x10000, CRC(b6b1a8e4) SHA1(be537fc27f6dedbd7fd935a7900ec075d2183837), ROM_BIOS(1))
ROM_END

// Eagle EAGLEN486 GC10A - Chipset: NEC ADC006, LGS Prime 3B 9543 - CPU: Socket 3 - RAM: 2xSIMM72, Cache: fake (not connected, marked write back)
// On board: IDE, Floppy, 2xser, par - ISA16: 4, PCI: 2 - BIOS: 32pin (sst29ee010), only the first half is occupied - // BIOS-String: Phoenix NuBIOS Version 4.04
ROM_START( gc10a )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "nec_sst29ee010_orig.bin", 0x10000, 0x10000, CRC(7b1feabb) SHA1(468734b766b9c438b2659fddf2cabcfde5a574a2))
	ROM_IGNORE(0x10000)
ROM_END

// Arstoria AS496 - Chipset: SiS 85C495, 95C497, Winbond - CPU: Socket 3 - RAM: SIMM72x4, Cache: 4+1 - BIOS: 32pin  Keyboard-BIOS: BESTKEY - ISA16: 4, PCI: 3
// BIOS-String: 09/12/96-SiS-496-497/A/B-2A4IBR2CC-00 / ARSTORIA AS496 V2 09/12/96 - boots to Boot Block BIOS
ROM_START( as496 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "as496.bin", 0x00000, 0x20000, CRC(745f8cc8) SHA1(46b9be25a7027a879482a412c9fe5687bbb28f08))
ROM_END

// Peacock PCK 486 DX DOC 50-60064-00 - Chipset: Symphony SL82C465 SL82C461 SL82C362 Chips F82C721 - CPU: i486DX-33, FPU socket privoded
// BIOS: AMI 486DX ISA BIOS AA3364567 - Keyboard-BIOS: AMI/Intel P8942AHP - On board: 2xser, Floppy, IDE, par - OSC: 33.000MHz
// BIOS-String: 40-0100-806294-00101111-060692-SYMP-0 / Peacock Computer 486 BIOS Rev. 2.0 / 30.11.92 - ISA16: 6
ROM_START( pck486dx )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "pck486dx.bin", 0x10000, 0x10000, CRC(d0edeba8) SHA1(b5b9492f32e35764c802be2b05a387a9b3aa7989))
ROM_END

// FIC 486-GIO-VT2 - Chipset: Winbond W83758P, Winbond W83757AF, VIA VT82C482, VT82C486A, VT82C461 - ISA8: 1, ISA16: 3, ISA/VL: 2
// On board: Game, 2xIDE, 2xser, par, Floppy
ROM_START( ficgiovt2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 07/06/VT82C486A-214L2000-00 / Version  3.26G
	ROM_SYSTEM_BIOS(0, "ficgiovt2_326", "FIC 486-GIO-VT2 3.26G")
	ROMX_LOAD( "326g1c00.awd", 0x10000, 0x10000, CRC(2e729ab5) SHA1(b713f97fa0e0b62856dab917f417f5b21020b354), ROM_BIOS(0))
	// 1: BIOS-String: 06/19/95-VT82C486A-214L2000-00 / Version VBS1.08H 486-GIO-VT2
	ROM_SYSTEM_BIOS(1, "vt2vbs108","VBS1.08H 486-GVT-2")
	ROMX_LOAD( "award_486_gio_vt2.bin", 0x10000, 0x10000, CRC(58d7c7f9) SHA1(097f15ec2bd672cb3f1763298ca802c7ff26021f), ROM_BIOS(1)) // Vobis version, Highscreen boot logo
	// 2: BIOS-String: 07/17/97-VT82C486A-214L2000-00 / Version 3.276
	ROM_SYSTEM_BIOS(2, "ficgiovt2_3276", "FIC 486-GIO-VT2 3.276")
	ROMX_LOAD( "32760000.bin", 0x10000, 0x10000, CRC(ad179128) SHA1(595f67ba4a1c8eb5e118d75bf657fff3803dcf4f), ROM_BIOS(2))
	// 3: BIOS-String: 08/30/94-VT82C486A-214L2000-00 / Version VBS1.04 486-GIO-VT2 - Keyboard-BIOS: VT82C42N
	ROM_SYSTEM_BIOS(3, "vt2vbs104","VBS1.04 486-GVT-2")
	ROMX_LOAD( "486-gio-vt2.bin", 0x10000, 0x10000, CRC(7282133d) SHA1(c78606027eca509cd6d439e4689b8d50753ee80c), ROM_BIOS(3)) // Vobis version, Highscreen boot logo
ROM_END

// FIC 486-GVT - VIA VT82C486, VIA VT82C482 - AMIBIOS 08/08/93 - CPU: P24T, solder pads for 486 provided - RAM: SIMM30: 4, SIMM72: 2, Cache: 9x28pin DIP
// ISA16: 4, ISA16/VLB: 2
ROM_START( fic486gvt ) // BIOS-String: X0-0100-001121-00101111-112593-VT486N8-0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486gvt.bin", 0x10000, 0x10000, CRC(4c5b4bde) SHA1(04711725fe89d9c793a369d82d411a5495ae3aea))
ROM_END

// Octek Hawk REV 1.1 - BIOS: AMI AA1481746 486DX ISA BIOS 28pin - Keyboard-BIOS: Intel/AMI - Chipset: OPTi F82C206L, 82C496 - OSC: 66.667MHz, 14.31818MHz
// BIOS-String: 40-0100-000000-00101111-121291-OPTIDXBB-0 / HAWK -011 - CPU: Intel Overdrive DX2ODPR66 - ISA16: 7
ROM_START( ochawk )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "hawk.bio", 0x10000, 0x10000, CRC(365b925d) SHA1(3a1776c80540b6878ff79857c2d4e19320a2792a))
ROM_END

// Abit AB-PW4 - Chipset: Winbond W83C491F, W83C492F (SL82C491 Symphony Wagner) - BIOS/Version: Award D2144079 - CPU: i486sx-25 - ISA8: 1, ISA16: 3, ISA16/VL: 3
ROM_START( abpw4 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 03/21/95-Winbond-83C491-2C4J6A11-46 / Award v4.50G / GREEN CACHE 486 VESA SYSTEM BIOS
	ROM_SYSTEM_BIOS(0, "2c4j6a11", "2C4J6A11-46")
	ROMX_LOAD( "award_486_bios_d2144079_c1984-1995.bin",0x10000, 0x10000, CRC(c69184da) SHA1(e8a799f9a3eebfd09c1d19a909574fca17fce7a0), ROM_BIOS(0))
	// 1: BIOS-String: 09/12/95-Winbond-83C491-2C4J6A12-2E
	ROM_SYSTEM_BIOS(1, "2c4j6a12", "2C4J6A12-2E")
	ROMX_LOAD( "pw4_2e.bin", 0x10000, 0x10000, CRC(c4aeac4d) SHA1(e58f2e2d5c337f447808535629686dde54c09fab), ROM_BIOS(1))
ROM_END

// Vintage Sprite SM 486-50USC - Chipset: UM82C491F - BIOS: EPROM/MR-BIOS 1.50 - Keyboard-BIOS: JETkey V3.0
// CPU: Intel 486DX2-66 - OSC: 33.333000MHz, 14.31818MHz - ISA16: 5, ISA16/VL: 2
ROM_START( sm48650usc ) // constant reset
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "mrbios_1150usc_um82c491f.bin", 0x10000, 0x10000, CRC(b6ef1220) SHA1(94511df49713ec30467c8d9b18eb04e83fa7a809))
ROM_END

// Octek Hippo VL+ - CPU: 486 - BIOS: EPROM/MR - Keyboard-BIOS: MR/Amikey - Chipset: DCA/Octek (label stickers) - ISA16: 3, ISA16/VL: 3
// MR BIOS (r) V1.52 / 486SLC CPU 28MHz
ROM_START( ochipvlp )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:  // reset loop
	ROM_SYSTEM_BIOS( 0, "v152", "V1.52")
	ROMX_LOAD( "vlmr152.rom", 0x10000, 0x10000, CRC(b4febf98) SHA1(a28ffa20fe772eac5fd149821d5637af63965371), ROM_BIOS(0))
	// 1: MR BIOS (r) V3.21 2GB support
	ROM_SYSTEM_BIOS( 1, "v321", "V3.21 with 2GB support")
	ROMX_LOAD( "v053b407.rom", 0x10000, 0x10000, CRC(415d92b1) SHA1(e5a9f2a677002368d20f1281e2ac3469b19079f9), ROM_BIOS(1))
ROM_END

// Octek Hippo COM - Chipset: UMC UM82C865F, UM82C863F, UM82C491F - CPU: 486sx - BIOS: EPROM/AMI 486DX ISA BIOS - Keyboard-BIOS: MEGATRENDS MEGA-KB-H-WP / Intel
// BIOS-String: 40-0102-428003-00101111-080893-UMC491F-0 / U491/3 GREEN 486 MAIN BOARD INV1.1 94.2.21 - ISA16: 4 - On board: 1xIDE, Floppy, Game, 2xserial, 1xparallel
ROM_START( ochipcom )
	ROM_REGION32_LE( 0x20000, "bios", 0)
	ROM_LOAD( "hippo_com_bios.bin", 0x10000, 0x10000, CRC(d35f65a1) SHA1(885f55f87d2070c6a846768e5cf76499dad8d15c))
ROM_END

// J-Bond A433C-C/A450C-C RAM: 8xSIMM30, Cache: 8xCY7199-25PC/2xCY7C166-20PC - 2 8-bit ISA, 6 16-bit ISA)
// Chipset: ETEQ ET82C491 + ET82C493; CHIPS P82C206; AMI KB-BIOS-VER-F P8042AHP - OSC: 33.000MHz - CPU: i486DX-33
// BIOS: AMI 486 BIOS ZZ566787 - BIOS-String: 40-0200-001353-0010111-070791-ETEQ4/1C-0 / ETEQ 486 Mar. 05, 1992
ROM_START( a433cc )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ami_j-bond_a433c-c.bin", 0x10000, 0x10000, CRC(66031e98) SHA1(d2d1a26837d3ca943a6ef09ec3e6fbfaaa62cc46))
ROM_END

// ASUS PVI-486AP4 (Socket 3, 4xSIMM72, Cache: 128/256/512KB, 4 PCI, 4 ISA, 1 VLB)
// Intel Aries PCIset S82425EX + S82426EX; DS12887 RTC; VIA VT82C42N - BIOS: 32pin
ROM_START( a486ap4 )
	ROM_REGION32_LE(0x20000, "bios", 0)
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

// ASUS PCI/I-486SP3G V3.02 (Socket 3, RAM: 4xSIMM72, Cache: 128/256/512K, 1 IDE, 1 SCSI, 3 PCI, 4 ISA) - BIOS: 32pin
// Intel Saturn II chipset: 82424ZX CDC + 82423TX DPU + 82378ZB SIO; NCR 53C820; National PC87332; DS12887 RTC; VIA VT82C42N
ROM_START( a486sp3g )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 10/21/94-SATURN-II-P/I-SP3G-00 / #401A0-302
	ROM_SYSTEM_BIOS(0, "v302", "ASUS PCI/I-486SP3G V3.02")
	ROMX_LOAD( "awsg0302.bin", 0x00000, 0x20000, CRC(21e918a0) SHA1(c7f937e3e90a43d7c7f867e686625b28a9c2484c), ROM_BIOS(0))
	// 1: BIOS-String: 08/15/95-SATURN-II-P/I-SP3G-00 / #401A0-304
	ROM_SYSTEM_BIOS(1, "v304", "ASUS PCI/I-486SP3G V3.04")
	ROMX_LOAD( "awsg0304.bin", 0x00000, 0x20000, CRC(f4d830d2) SHA1(086ccd14c7b0c521be1958d58b3539c4bfe4721f), ROM_BIOS(1))
	// 2: BIOS-String: 04/21/99-SATURN-II-P/I-SP3G-00 / #401A0-0306-1
	ROM_SYSTEM_BIOS(2, "v306", "ASUS PCI/I-486SP3G V3.06")
	ROMX_LOAD( "0306.001.bin", 0x00000, 0x20000, CRC(278e1025) SHA1(75835e59cf28bb6b9258f676766633cbffa56848), ROM_BIOS(2))
ROM_END

// ASUS VL/EISA-486SV1 (8 EISA, 1 VLB) -
ROM_START( a486sv1 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// BIOS-String: 05/20/94-SIS-486/EISA-E-486SV1-00 / #401A0-0112
	//ROM_SYSTEM_BIOS(0, "v112", "Award BIOS V1.12")
	ROM_LOAD( "e4sv0112.awd", 0x10000, 0x10000, CRC(d1d42fc9) SHA1(61549bf597517bb3c33e724e32b3cca981e65000))
ROM_END

// FIC 486-VIP-IO (3 ISA, 4 PCI)
// VIA GMC chipset: VT82C505 + VT82C486A + VT82C482 + VT82C483 + VT83C461 IDE; DS12885Q RTC; National PC87332VLJ-S I/O
ROM_START( ficvipio )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 12/07/94-VT82C505-2A4L4000-00 / Version 4.26GN2(ES2) (12/07/94)
	ROM_SYSTEM_BIOS(0, "426gn2", "FIC 486-VIP-IO 4.26GN2")
	ROMX_LOAD( "426gn2.awd",   0x00000, 0x20000, CRC(5f472aa9) SHA1(9160abefae32b450e973651c052657b4becc72ba), ROM_BIOS(0))
	// 1: BIOS-String: 02/08/96-VT82C505-2A4L4000-00 / Version 4.27GN2A (02/14/96)
	ROM_SYSTEM_BIOS(1, "427gn2a", "FIC 486-VIP-IO 4.27GN2A")
	ROMX_LOAD( "427gn2a.awd",  0x00000, 0x20000, CRC(035ad56d) SHA1(0086db3eff711fc710b30e7f422fc5b4ab8d47aa), ROM_BIOS(1))
	// 2: BIOS-String: 01/18/95-VT82C505-2A4L4000-00 / Version 4.26GN2A (01/18/95)
	ROM_SYSTEM_BIOS(2, "426gn2a", "FIC 486-VIP-IO 4.26GN2A")
	ROMX_LOAD( "486-vip-io.bin", 0x00000, 0x20000, CRC(907ed412) SHA1(5d2c584a230826935f56151a7c74419baf54796b), ROM_BIOS(2))
ROM_END

// Shuttle HOT-409 (6 16-bit ISA incl. 2 VLB, 2 8-bit ISA, 8 SIMM30, Cache: 64/128/256K+Tag in 2 banks)
// OPTi 82C495SX + 82C392SX + F82C206; MEGA-KB-1-WP
ROM_START( hot409 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 40-0200-001343-00101111-111192-OPT495SX-0 / Version 2.0
	ROM_SYSTEM_BIOS(0, "hot409v20", "Shuttle HOT-409 V2.0")
	ROMX_LOAD( "ami1992.bin", 0x10000, 0x10000, CRC(a19c3fd4) SHA1(404822c98344061b60883533395a89fe4902c177), ROM_BIOS(0))
	// 1: BIOS-String: 40-0204-001343-00101111-080893-OPT495SX-0 / OPTi495SX Version 3.0
	ROM_SYSTEM_BIOS(1, "hot409lba", "Shuttle HOT-409 V3.0 with LBA")
	ROMX_LOAD( "409lba.rom", 0x10000, 0x10000, CRC(78c5e47e) SHA1(7f14a88a5548fc67dd00e73fd09745e899b93a89), ROM_BIOS(1))
	// 2: BIOS-String: 40-0200-001343-00101111-111192-OPT495SX-0 / VERSION 1.1
	ROM_SYSTEM_BIOS(2, "hot409v11", "Shuttle HOT-409 V1.1")
	ROMX_LOAD( "amibios_hot409.bin", 0x10000, 0x10000, CRC(17729ee5) SHA1(ea3f5befe16ede7e9f4be3b367624745a6935ece), ROM_BIOS(2))
ROM_END

// Siemens-Nixdorf 486 mainboards and BIOS versions
// The same mainboards were used in various case versions to get the different model lines, so an identification by the mainboard number (Dxxx) is safest
ROM_START( pcd4x )
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	// D756, was used in PCD-4Lsx, contains Cirrus Logic VGA ROM
	ROM_SYSTEM_BIOS(0, "d756v320r316", "D756 BIOS V3.20 R3.16")
	ROMX_LOAD( "fts_biosupdated756noflashbiosepromv320_320316_149.bin", 0x00000, 0x20000, CRC(2ab60725) SHA1(333b64424c08ecbbaf47110c99ad0335da211489), ROM_BIOS(0) )
	// D674, was used in PCD-4M, PCD-4Msx, PCD-4RSXA/4RA - this is a CPU card that is plugged into an ISA backplane; OSC: 14.31818
	// Chipset: LSI HT342-B-07, LSI HT321-D, Intel B6842-V31 AWARD(c)SNI UPI V4.3, IS9412BL PC87311AVF US4823312 - CPU: 486sx soldered onto the mainboard, but a socket for a 486DX is present - RAM: SIMM30x8
	// the CPU card can accept 16 bit "piggyback" modules, e.g. an ET4000 graphics card or a MFM harddisk controller to save ISA slots
	// on board: IDE, Floppy, beeper, keyboard connector, parallel, 2xserial, RTC DS12887, connectors for NMI and keylock, 4 DIP switches labelled DX2, Upgrade, 25, color
	// jumper X7: skip on/off, jumper x6: drv l/h
	ROM_SYSTEM_BIOS(1, "d674v320r316", "D674 BIOS V3.20 R3.16")
	ROMX_LOAD( "fts_biosupdated674noflashbiosepromv320_320316_144.bin", 0x00000, 0x20000, CRC(1293d27c) SHA1(22f36c4a5a0912011ed54ff917244f412208ffc0), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "d674v320r304", "D674 BIOS V3.20 R3.04")
	ROMX_LOAD( "d674_27c1024_3.20.bin", 0x00000, 0x20000, CRC(dfdad89e) SHA1(6cb78d8b5c8822dc84970ba912bc66a5e7cd2fb4), ROM_BIOS(2) )
	// D802, was used in PCD-4HVL
	ROM_SYSTEM_BIOS(3, "d802v320r316", "D802 BIOS V3.20 R3.34.802")
	ROMX_LOAD( "fts_biosupdated802noflashbiosepromv320_320334_152.bin", 0x00000, 0x20000, CRC(fb1cd3d2) SHA1(98043c6f0299e1c56e5f266ea5f117ae456447ff), ROM_BIOS(3) )
	// D620
	ROM_SYSTEM_BIOS(4, "d620", "D620")
	ROMX_LOAD( "w26361-d620-z4-01-5_award_v3.10_r2.02.bin", 0x00000, 0x20000, CRC(2708cc2a) SHA1(a399c938ffeba4cb28a22e54235f3f9c5e2892f6), ROM_BIOS(4) )
ROM_END


// ***** 486 motherboards using the ALi M1487 M1489 chipset

// Abit AB-PB4 REV.:1.2 - Chipset: ALi M1487 M1489, Winbond W83787F, W83768F - On board: Floppy, 2xser, 2xIDE, par
// ISA16: 3, PCI: 3, PISA: 1 - OSC: 14.3F5P - CPU: Socket 3 - BIOS: Award D2317569, 32pin
ROM_START( abpb4 ) // both BIOS versions end up in the Boot Block BIOS
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 10/30/95-ALI-1487/89-2A4KDA12C-5E / GREEN 486 PCI SYSTEM BIOS
	ROM_SYSTEM_BIOS( 0, "pb4", "PB4")
	ROMX_LOAD( "486-ab-pb4.bin", 0x00000, 0x20000, CRC(90884abc) SHA1(1ee11b026cb783b28cc4728ab896dbeac14eb954), ROM_BIOS(0))
	// 1: BIOS-String: 07/03/96-ALI-1487/89-2A4KDA1BC-F2 / GREEN PCI/ISA SYSTEM ROM
	ROM_SYSTEM_BIOS( 1, "pb4pf2", "PB4P-F2")
	ROMX_LOAD( "pb4p_f2.bin", 0x00000, 0x20000, CRC(9ab8d277) SHA1(10e424f5dd5c98877a5a7c9ae6205b2c442ac0e0), ROM_BIOS(1))
ROM_END

// EFA 486 APIO all BIOS versions boot into BootBlock BIOS
ROM_START( 486apio )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:
	ROM_SYSTEM_BIOS(0, "a2", "A2")
	ROMX_LOAD( "apioa2.bin", 0x00000, 0x20000, CRC(0cf343be) SHA1(3a5757c802a30fb0d8d4fd623bee02af3b91fdd7), ROM_BIOS(0))
	// 1: 07/23/96-ALI-1487/89-2A4KDE3HC-00 / N486APIO Ver 2.00 SMC665GT
	ROM_SYSTEM_BIOS(1, "20sm", "2.0SM")
	ROMX_LOAD( "apio2smc.bin", 0x00000, 0x20000, CRC(1ced0692) SHA1(8afca17f0d793a3266b04ce8d70a359a29de3af7), ROM_BIOS(1))
	// 2: BIOS-String: 03/05/96-ALI-1487/89-2A4KDE3JC-00 / N486APIO, Rev 2.1
	ROM_SYSTEM_BIOS(2, "ag2", "AG2")
	ROMX_LOAD( "1019ag2.bin", 0x00000, 0x20000, CRC(4066124e) SHA1(7adbf528d8132122da4f950ee78931abd5d949e4), ROM_BIOS(2))
ROM_END

// MSI MS-4145 - Chipset: ALi M1487, M1489, W83787F, W83758F - CPU: Socket 3 - RAM: 3xSIMM72 - Cache: 8x28/32pin + TAG - ISA16: 4, PCI: 3
// BIOS: 32pin - on board: 2xIDE, Floppy, par, 2ser
ROM_START( ms4145 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:
	ROM_SYSTEM_BIOS(0, "ag56", "AG56")
	ROMX_LOAD( "ag56.rom", 0x00000, 0x20000, CRC(217d7258) SHA1(bd0d484607fbcf54821822e20e4bf5fcaf456591), ROM_BIOS(0))
	// 1:
	ROM_SYSTEM_BIOS(1, "ag56p", "AG56P")
	ROMX_LOAD( "ag56p.rom", 0x00000, 0x20000, CRC(f2737425) SHA1(e6058a98a7ca4c03d1c1b7d30602fe97d88bc04a), ROM_BIOS(1))
	// 2:
	ROM_SYSTEM_BIOS(2, "ag56s", "AG56S")
	ROMX_LOAD( "ag56s.rom", 0x00000, 0x20000, CRC(c015ed49) SHA1(4c447bab1cba9d38b99c2e36e0824809e876931e), ROM_BIOS(2))
ROM_END

// TMC Research Corporation PCI48AF - Chipset: ALi M1487, M1489, TMSIA 9347W/TC4069UBP, FDC37C665GT - CPU: Socket 3 - RAM: SIMM72x4, Cache: 4x32pin + TAG
// ISA16: 4, PCI: 4 - On board: 2xISA, Floppy, 2xpar, 2xser - Keyboard-BIOS: AMIKEY-2
ROM_START( pci48af )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:
	ROM_SYSTEM_BIOS(0, "af2", "AF2")
	ROMX_LOAD( "pci48af2.rom", 0x00000, 0x20000, CRC(556113cc) SHA1(cbbbcaa300253e766bce5292ffdfedf72c76e287), ROM_BIOS(0))
	// 1: 03/25/96-ALI-1487/89-2A4KDM29C-00 / 486DX-AC-WBc-25/33/40/50/66/80/100/120/133-A2-ZG - boots to BootBlock BIOS
	ROM_SYSTEM_BIOS(1, "afa", "AFA")
	ROMX_LOAD( "pci48afa.bin", 0x00000, 0x20000, CRC(9127efb5) SHA1(cf77fcca00b6e48067caefa518bedb287f945147), ROM_BIOS(1))
	// 2:
	ROM_SYSTEM_BIOS(2, "f2a", "F2A")
	ROMX_LOAD( "48af-f2a.rom", 0x00000, 0x20000, CRC(1e0d8216) SHA1(c6e4342dea2b7feac6e239dc99aee65508f9c297), ROM_BIOS(2))
ROM_END

ROM_START( alim1489 ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: // V1.2A (with fake cache SRAM) - Chipset: ALi M1489, M1487, UM8663AF, UM8667 - BIOS: 10/10/94 AMI AD0153466 (32pin) - ISA16: 4, PCI: 3
	// On board: 2xser, Game, par, Floppy, 2xIDE - OSC: 14.31818
	ROM_SYSTEM_BIOS(0, "ali148901", "ALi M1489 #1")
	ROMX_LOAD( "ali.bin", 0x00000, 0x20000, CRC(d894223b) SHA1(088a94d2425f0abc85fafa922a5c6792da608d28), ROM_BIOS(0))
	// 1: Chipset: ALi M1489 A1, M1487 B1, GoldStar Prime 3b - CPU: Am486DX4-100 - RAM: 2xSIMM72, Cache: 9x32pin, used: 5xCY7C199-15PC
	// BIOS: AMI 32pin - ISA16: 4, PCI: 3 - On board: 2xIDE, Floppy, 2xser, par
	ROM_SYSTEM_BIOS(1, "ali148902", "ALi M1489 #2")
	ROMX_LOAD( "ali_pci486v1-hj3.bin", 0x00000, 0x20000, CRC(fca45439) SHA1(a4bad38301c9e7f780a95a07b7062f0a277a7a10), ROM_BIOS(1))
ROM_END


// ***** 486 motherboards using the CONTAQ 82C596 chipset

// MSI MS-4125 - Chipset: CONTAQ 82C596 SiS 85C206 - ISA8: 1, ISA16: 3, ISA16/VL: 2 - BIOS: AMI 486DX ISA BIOS AA65441044 (28pin) - Keyboard-BIOS: AMI/Intel P8942AHP
// BIOS-String: 40-0104-001169-00101111-111192-CTQ596-0 / AC5E 052193
ROM_START( ms4125 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD("ms4125.bin", 0x10000, 0x10000, CRC(0e56b292) SHA1(9db26e8167b477c550d756d1ca2363283ebff3ed))
ROM_END

// Diamond Flower, Inc. (DFI) 486-CCV Rev B - Chipset: CONTAQ 82C596, KS83C206EQ - BIOS: 11/11/92 AMI AB8644083 (28pin) - Keyboard-BIOS: AMIKEY-2
// BIOS-String: 40-0100-ZZ1211-00101111-111192-CONTAQ/5-0 - OSC: 14.31818MHz - ISA8: 2, ISA16: 4, ISA16/VL: 2
ROM_START( 486ccv )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "contaq.bin", 0x10000, 0x10000, CRC(2ac46033) SHA1(a121c22ded4932e3ba8d65c2b097b898f02147c7))
ROM_END


// ***** 486 motherboards using the OPTi495SLC chipset

// QDI PX486P3 - Chipset: OPTi 82C495SLC, F82C206 - CPU: 486 - BIOS: 11/11/92 AMI (28pin)
// Keyboard-BIOS: AMIKEY - ISA8: 1, ISA16: 3, ISA16/VL: 3 (one marked MASTER/SLAVE, two marked SLAVE)
ROM_START( px486p3 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 40-0402-428003-00101111-111192-OP495SLC-0 / PX486DX33/50P3 IVN 2.0 19/11/1993
	ROM_SYSTEM_BIOS(0, "ivn20", "IVN 2.0")
	ROM_LOAD( "px486p3.bin", 0x10000, 0x10000, CRC(4d717aad) SHA1(2d84cf197845d58781f77e4d539ca994fd8733c8))
	// 1: BIOS-String: 40-0401-428003-00101111-111192-OP495SLC-0 / PX486DX33/50P3 IVN 1.0 25/06/1993
	ROM_SYSTEM_BIOS(1, "ivn10", "IVN 1.0")
	ROMX_LOAD( "qdi_px486.u23", 0x10000, 0x10000, CRC(c80ecfb6) SHA1(34cc9ef68ff719cd0771297bf184efa83a805f3e), ROM_BIOS(1))
ROM_END

// NAT48PV-1.0 VL - Chipset:82C495SLC, Chips F82C206J - RAM: 8xSIMM30, Cache: 9xIS61C256A - OSC: 14.31818 - BIOS: AMI 486DX ISA BIOS (28pin) AA5312581
// Keyboard-BIOS: MEGA-KB-F-WP P8042AHP - ISA8: 2, ISA16: 3, ISA16/VL: 2
// 40-040A-001291-00101111-111192-OP495SLC-0 / 486DX-OP-WBq-25/33/50-K2-ZZ
ROM_START( nat48pv )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "amibios_aa5312581.bin", 0x10000, 0x10000, CRC(8a788c79) SHA1(050972b7a369a463d6654ec52c0804002e9bcb37))
ROM_END


// ***** 486 motherboards using the OPTi OPTi 82C392, 82C493, 82C206 chipset

// Auva-Cam-33-P2 = See-Thru Sto486Wb - CPU: 486 - ISA8: 1, ISA16: 7 - Chipset: OPTi 82C392, 82C493, 82C206
// MR BIOS (tm) V1.30
ROM_START( sto486wb )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "opti_82c493_486_mr_bios_v130.rom", 0x10000, 0x10000, CRC(350d5495) SHA1(4f771ef5fe627e0556fb28f8972e545a0823a74d))
ROM_END

// Silicon Valley Computer, Inc. 486WB6A3.B1 - Chipset: OPTi 82C493/392, F82C206 - BIOS: AMI 486 BIOS ZZ342708 - Keyboard BIOS:AMI KB-BIOS-VER-F
// CPU: Intel 80486DX-33, secondary socket - RAM: 8xSIMM30, Cache: 9xMosel MS6264A-20NC - OSC: 33.333MHz, 14.31818 - ISA8: 1, ISA16: 6, ISA16/RAM extension: 1
ROM_START( 486wb6a3 ) // BIOS String: 40-0101-ZZ9999-00101111-060691-OPWBSX-F
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486_ami.bin", 0x10000, 0x10000, CRC(1f5e9263) SHA1(534a6ace19ba6185614e04e3bd2d0aabe1193e2c))
ROM_END

ROM_START( op82c392 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: 486-A4865-A4866-XX V2 1 COMP - CPU: 486DX-33 - Chipset: Opti 82C392, 82C493, Opti F82C206 - BIOS: 486DX AMI (28pin) - Keyboard-BIOS: AMI
	// BIOS-String: - ECB: 1, ISA8: 2, ISA16: 5 - OSC: 14.318, 66.000000MHz - RAM: 8xSIMM30, Cache: 16 sockets +1 provided
	ROM_SYSTEM_BIOS(0, "a4865", "A4865")
	ROMX_LOAD( "a4865-a4866.bin", 0x10000, 0x10000, CRC(9c726164) SHA1(b6ad8565a489b9d5991eea37905be2e6fc59fa48), ROM_BIOS(0))
	// 1: Chipset: OPTi 82C392, 82C493, UMC UM82C206L - CPU: i486DX-33, FPU socket provided - OSC: 34.000MHz, 14.31818 - Keyboard-BIOS: AMI/Intel P8942AHP
	// BIOS: AMI 486 BIOS Z600436 - BIOS-String: 40-0131-425004-01001111-070791-OPWB493-0 / ABC COMPUTER CO., LTD. - 40-0101-DK1343-00101111-00101111-060691-OPWBSX-0 - ISA8: 2, ISA16: 6
	ROM_SYSTEM_BIOS( 1, "82c493", "82C493")
	ROMX_LOAD( "486-920087335.bin", 0x10000, 0x10000, CRC(38571ffe) SHA1(aa6048213139c88901aca9cd38251a3937b6e52d), ROM_BIOS(1))
	// 2: Chipset: OPTi 82C392, 82C493, Chips 206 - CPU: two sockets provided - RAM: 8xSIPP30, on board RAM/Cache? photo too blurry - Keyboard BIOS: Intel/AMI
	// OSC: 66.666MHz, 14.31818 - ISA8: 1 (solder pads for memory slot provided), ISA16: 7 - BIOS-String: 40-0101-009999-00101111-060691-OPWBSX-0 / OPTi-WB GW486SX/DX BIOS, July 3, 1992
	ROM_SYSTEM_BIOS( 2, "060691", "06/06/91")
	ROMX_LOAD( "gw486sxdx.bin", 0x10000, 0x10000, CRC(99ecd9ce) SHA1(616bc1192c0ffb9d90f8aa32d93a8badc45f9d56), ROM_BIOS(2))
ROM_END


// motherboards using the OPTi 82C802A, 82C602A chipset

// Edom MV035F - Chipset: OPTi 82C802A, 82C602A - CPU: TI 486DX2-80 - RAM: 4xSIMM30, 3xSIMM72, Cache: 4x32pin, 4x28pin, TAG
// BIOS: 28pin - Keyboard-BIOS: VIA - ISA16: 5, ISA16/VL: 3
// From the source: "The seller claimed that it POSTed, but all the BIOS options were grayed out. I pulled and dumped the BIOS
// chip, ran it through MODBIN and found out the OEM of the board simply set all BIOS setup options to 'SHOW-ONLY'. I set all
// except PCI CONFIGURATION to 'Enabled' (since this board doesn't have any PCI slots), blanked the EPROM and burned the modified
// BIOS to it, and now I can set all CMOS setup options normally."
// BIOS-String reflects the edit: 11/17/95-OPTi-802G-2C4UKW01-00 / Computer Spirit v1.0 (Hack 1.0 by Eep386
ROM_START( edmv035f )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "mf035fed.bin", 0x10000, 0x10000, BAD_DUMP CRC(5c1ce352) SHA1(c7944b06e4a3473bb720caa5043f4b55bccf3835))
ROM_END

// Octek Hippo DCA2 - Chipset: OPTi 802G - BIOS: 28pin - CPU: Socket 3 - ISA8: 2, ISA16: 3, ISA16/VL: 3 - RAM: 4xSIMM72, Octek claimed, Cache would be taken out of main RAM
ROM_START( ochipdca2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 10/27/94-OPTI-802G-2C4UKO01-00 / (2C4UKO01) EVALUATION ROM - NOT FOR SALE
	ROM_SYSTEM_BIOS(0, "hv2433", "AWARD HV2433")
	ROMX_LOAD( "hv2433.awa", 0x10000, 0x10000, CRC(d6179601) SHA1(8a9c7ec959f6626268e0e242760439272fc9e28c), ROM_BIOS(0))
	// 1: beep code
	ROM_SYSTEM_BIOS(1, "h2433", "AMI H2433")
	ROMX_LOAD( "h2433.ami", 0x10000, 0x10000, CRC(a646a191) SHA1(086ae94554e3c2b292f2e32b5cb080c15dfa3e0b), ROM_BIOS(1))
	// 2: beep code L-H-H-L
	ROM_SYSTEM_BIOS(2, "mr321", "MR-BIOS 3.21") // supports AMD X5-133
	ROMX_LOAD( "095061.bin", 0x10000, 0x10000, CRC(0a58cab2) SHA1(e64d6ca0bad6eeed492260853d7d60cd2a60a222), ROM_BIOS(2))
	// 3: beep code L-H-H-L
	ROM_SYSTEM_BIOS(3, "mr31", "MR-BIOS 3.1")
	ROMX_LOAD( "dca2mr31.rom", 0x10000, 0x10000, CRC(43b7415f) SHA1(45df892d146b8e2594274773c93d1623207b40fc), ROM_BIOS(3))
ROM_END


// Motherboards using the Opti 82C895 82C602A chipset

// ExpertChip EXP4044 - CPU: Socket3 - Chipset: OPTi 82C895, 82C602 - RAM: 4xSIMM30, 2xSIMM72, Cache: 4x28pin, 4x32pin + TAG
// Keyboard-BIOS: MEGAKEY or Winbond W83C42 - ISA16: 3, ISA16/VL: 3
// BIOS-String: 06/23/94-OPTI-802G-2C4UKD01-00 / EXP4044 GREEN BIOS Ver 1.1
ROM_START( exp4044 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD("4ecw001.bin", 0x10000, 0x10000, CRC(cf186fa4) SHA1(d65cc2f2c6feaa1a537319aaef86df12b44afdec))
ROM_END

// Jetway J-403TG
ROM_START( jwj403tg )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: JETWAY J-403TG (VER.D2E) Chipset: OPTi 82C895, 85C602 - CPU: Socket 3 - Keyboard BIOS: AMIKEY-2 -
	// RAM: 4xSIMM32, 3xSIMM72, Cache: 8xW24257AK-15, 1xW24129AK - ISA16: 5, ISA16/VLB: 3 (2xMASTER, 1xSLAVE)
	ROM_SYSTEM_BIOS(0, "verd2e", "VER.D2E") // BIOS-String: 40-0101-428060-00101111-111192-UMC491C-0
	ROMX_LOAD( "ver_d2e.bin", 0x10000, 0x10000, CRC(5d3b86bb) SHA1(d7c3cfbb5b858efacc7cee872a4ef5c9666f9d06), ROM_BIOS(0))
	// 1: JETWAY J-403TG GREEN VLB ver 2.0 - Chipset: OPTi 82C895, second IC with Energy Star logo - BIOS: Award 486DX J156079 - Keyboard BIOS: VIA VT82C42N
	// CPU: Socket 3 - RAM: 4xSIMM30, 3xSIMM72, Cache: 8xW24257AK-15 - ISA8: 2, ISA16: 3, ISA16/VLB: 3
	ROM_SYSTEM_BIOS(1, "greenver20", "GREEN VLB ver 2.0") // BIOS-String: 07/21/94-OPTI-802G-2C4UKJ11-00 / V1.A
	ROMX_LOAD( "403tg.bin", 0x10000, 0x10000, CRC(41ebe3e8) SHA1(567d61c5912cfb5fbfc9a1b674e7edad09a2165c), ROM_BIOS(1))
	// 2: JETWAY J-403TG VLB Rev D - Chipset: OPTi 82C895, 82C602 - BIOS: AMI 486DX ISA BIOS AB5257763 - Keyboard BIOS: JETkey V5.0
	// CPU: 80486DX - RAM: 4xSIMM30, 3xSIMM72, Cache: 8x32pin DIP (used: 8xH61256-20) - ISA8: 2, ISA16: 3, ISA16/VLB: 3
	ROM_SYSTEM_BIOS(2, "revd", "Rev D") // BIOS-String: 40-P301-001276-00101110-121593-OPTi895-H - blank screen
	ROMX_LOAD( "b2790126", 0x10000, 0x10000, CRC(b2790126) SHA1(dad277c91dac9daffcd1e3f3e9a1a1e59c92e72e), ROM_BIOS(2))
	// 3: MR BIOS for the 82C895 chipset - MR BIOS (r) V2.02
	ROM_SYSTEM_BIOS(3, "82c895", "82C895")
	ROMX_LOAD("opt895mr.mr", 0x10000, 0x10000, CRC(516cb091) SHA1(4c5b51cd05974001da4b764b4b14987657770a45), ROM_BIOS(3))
ROM_END

// QDI V4P895P3/SMT V5.0 - Chipset: Opti 82C895 82C602A - CPU: Am486DX2-66 - ISA8: 1, ISA16: 3, ISA16/VL: 3
// RAM: 4xSIMM30, 2xSIMM72, Cache: 8xUM61256FK-15 - BIOS: AMI 486DX ISA BIOS Ac0928698 (28pin in a 32pin socket) - Keyboard-BIOS: AMIKEY-2
ROM_START( v4p895p3 ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-v4p895p3-smt.bin", 0x10000, 0x10000, CRC(683f8470) SHA1(eca1c21a8f8c57389d9fdf1cd76d2dec0928524a))
ROM_END

// Shuttle HOT-419 - Chipset: OPTi 92C895A, 82C602A - CPU: Socket 3 - RAM: 4xSIMM72, Cache: 8+1 UM61256K-15 - ISA8: 2, ISA16:3, ISA16/VL: 3
// BIOS: AMI AB0585433 (28pin) - Keyboard-BIOS: AMIKEY-2
ROM_START( hot419 ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:
	ROM_SYSTEM_BIOS(0, "072594", "07/25/94")
	ROMX_LOAD( "hot419_original_bios.bin", 0x10000, 0x10000, CRC(ff882008) SHA1(1a98d61fd49a2a07ff4f12ccba55cba11e4fde23), ROM_BIOS(0))
	// 1:
	ROM_SYSTEM_BIOS(1, "419aip06", "419AIP06")
	ROMX_LOAD( "419aip.rom", 0x10000, 0x10000, CRC(389ca65d) SHA1(457491c60aa45499e2cd8dad9db3bf3312977a4f), ROM_BIOS(1))
ROM_END

// TMC PAT48PG4-V1.20 - BIOS Version: Award 10/13/95 - Chipset: OPTi 82C895+82C602 - EPROM Label: 486 AWARD SOFTWARE 1984-1995 T1103040
// Keyboard BIOS: AMIKEY-2 - CPU: socket for 80486 - RAM: 4xSIMM30, 2xSIMM72, Cache: 9xISSI IS61M256-15N - ISA16: 4, ISA16, VLB: 3
ROM_START( tmpat48pg4 ) // BIOS-String: 10/13/95-OPTI-802G-2C4UKM21-00.00-00 / 486DX-OP-WOe-25/33/40/50/66/80/100/120-C7-ZG
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "award.bin", 0x10000, 0x10000, CRC(c217214b) SHA1(583820b2fe96ca4bfacf9267800afe7cc76e5ffa))
ROM_END


// Motherboards using the SiS 85C461 chipset

// Abit AB-AV4 (aka VL-BUS 486) - Chipset: SiS 85C461, HM5818A - BIOS Version: AMI 11/11/92 486DX ISA BIOS AA7247480
// Keyboard BIOS: AMI-KB-H-WP - CPU: socket for 80486PGA, solder pads for QFP486sx - RAM: 8xSIMM30, Cache: 7xW24257AK-20, 1xEm81256B-20P, 1x71256S20TP
// ISA16: 5, ISA16/VLB: 3 - OSC: 14.31818
ROM_START( abav4 ) // BIOS String: 40-01BB-001247-00101111-111192-SIS461 / CACHE 486 SYSTEM BIOS
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "amibios.bin", 0x10000, 0x10000, CRC(7bf8142e) SHA1(17e09eebabc0d2c393d27db38b571af3c0ccbb41))
ROM_END

// ASUS ISA-486SV2 - Chipset: SiS 85C461 - BIOS: AMI 486DX ISA BIOS AA7892378 28pin - Keyboard-BIOS: Intel/AMI
// BIOS-String: 40-110A-001292-00101111-111192-I486SI-0 - ISA16: 5, ISA16/VL: 2 - CPU: 486DX in a blue socket (overdrive ready)
ROM_START( a486sv2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-asus isa-486sv2.bin", 0x10000, 0x10000, CRC(de925130) SHA1(2e3db7a1d4645082290d6303a16446af2959f34a))
ROM_END

// GENOA TurboExpress 486 VL ASY 01-00302 - Chipset: SiS 85C407 85C461 - CPU: Socket3 - OSC: 14.31818MHz - ISA16: 4, ISA16/VL: 3 - BIOS: AMI 486DX ISA BIOS AB0562153 (28pin)
// BIOS-String: 40-0100-006156-00101111-080893-SIS461-0 / GENOA TurboExpress 486VL - 3 (Ver. C) - Keyboard-BIOS: AMIKEY
ROM_START( gete486vl )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-genoa_vlb.bin", 0x10000, 0x10000, CRC(9be0f329) SHA1(3b1adedd6aad40c623757e4976e0dcadb253f255))
ROM_END

// Lucky Star UCM-486V30 (aka SIS486 3-VLBUS) - BIOS/Version: AMI 01/14/1993 SUPPORT VESA, 486DX ISA BIOS AA8580239 - Keyboard BIOS: AMI MEGA-KB-H-WP
// Chipset: SIS 85C461, HM6818A - CPU: P24T socket - OSC: 14.31818 - RAM: 8xSIMM30, Cache: 9x28pin DIP
ROM_START( lsucm486v30 ) // BIOS string: 40-0100-001256-00101111-111192-SIS3486-0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ucm-486v30.bin", 0x10000, 0x10000, CRC(90ec73a2) SHA1(2fc17f7eed09c2f7d0139670677cc84bbc2964de))
ROM_END

// SOYO 486 VESA 025D2 - Chipset: SiS 85C461, 85C407 - BIOS: AMI - Keyboard BIOS: AMI (c)1988 - CPU: i486sx-33, full 486 socket provided
// RAM: 8xSIMM30, Cache: 8x28pin - ISA8: 1, ISA16: 4, ISA16/VL: 2 - BIOS-String: 40-0100-001102-00101111-080893-SIS461 / REV MG
ROM_START( so025d2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "soyo-486-vesa.bin", 0x10000, 0x10000, CRC(da798d7b) SHA1(15a8c1e244ee29ef5c61e05659f8ec7f8eaa8ab7))
ROM_END

// ***** 486 motherboards using the SiS BTQ 85C401/85C402 + 85C206 chipset

// ABIT AB-AX4 - Chipset: SIS BTQ 85C401, 85C402, 85C206 - BIOS: AMIBIOS 06/06/92 - Keyboard BIOS: AMI - CPU: socket for 80486
// RAM: 8xSIMM30, Cache: 8xEtronTech Em51256A-20P, 1X AS7C256-20PC - ISA8: 1, ISA16: 6 - OSC: 33.333MHz, 14.31818
ROM_START( abax4 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: X0-01AA-001247-00101111-060692-SISAUTO-0 / AX4 ISA SYSTEM BIOS
	ROM_SYSTEM_BIOS(0, "060692", "ABIT AB-AX4 06/06/92")
	ROMX_LOAD( "486dx_dump.bin", 0x10000, 0x10000, CRC(c7af4380) SHA1(59507a0f7d929ac19c5b56334f54643127c0d2be), ROM_BIOS(0))
	// 1: BIOS-String: 30-0200-D01247-00101111-070791-SIS486 / AT486DX 33MHz BIOS
	ROM_SYSTEM_BIOS(1, "070791", "ABIT AB-AX4 07/07/91")
	ROMX_LOAD( "486-aa1177369.bin", 0x10000, 0x10000, CRC(530535de) SHA1(7f6e627a77ebcaec97f08e6c797d31e9321e26fc), ROM_BIOS(1))
ROM_END

// ASUS ISA-486 - Rev. 1.4 - Chipset: SiS BTQ 85C401/85C402 + 85C206 - BIOS/Version: AMI 486DX ISA BIOS 05/05/91 AA1258865 - CPU: Intel 80486DX-33, FPU socket for 4167 provided
// RAM: 8xSIMM30, Cache: 8x28pin DIP (4x71256 fitted) - OSC: 14.31818, 33.000MHz - DIP6: 111000 - ISA8/RAM extension: 1, ISA16: 7 -
ROM_START( a486isa ) // BIOS String: 40-0102-001292-00101111-050591-SIS-486-0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "isa-486.bin", 0x10000, 0x10000, CRC(57375912) SHA1(8035b5d1cfe824a20a94571a57b86fdb4018f073))
ROM_END

// Mitac MBA-029 - Chipset: SIS BTQ 85C401, 85C402, 85C206 - BIOS: AMI - CPU: 486 socket - OSC: 14.34818, xxxxx (unreadable)
// RAM: 4xSIMM30, Cache: 8x28pin - ISA 8: 1 (not soldered in), ISA16: 6
ROM_START( mba029 ) // BIOS-String: 30-0200-ZZ1594-00101111-070791-SISAUTO-0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "mba-029_bios_27c512.bin", 0x10000, 0x10000, CRC(8d660697) SHA1(6b2be9ec9a2d12c9348c26ac25514af406fa752e))
ROM_END

// ***** 486 motherboards using the SiS 85C496/85C497 chipset
// cfr. pc/pcipc_sis.cpp


// ***** 486 motherboards using the SiS 85C471 + 85C407 chipset

// 486IG-B-2-1 - CPU: 486 - RAM: 8xSIMM30, Cache: 4x32pin, 4x28pin, TAG - Chipset: SIS85C471/407 - BIOS: AMI WIN BIOS 07/25/94
// Keyboard-BIOS: AMIKEY - BIOS-String: 40-0002-428020-00101111-080893-SIS471B-F - ISA8: 1, ISA16: 3, ISA16/VL: 3
ROM_START( 486igb21 ) // display remains blank
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486igb21.amw", 0x10000, 0x10000, CRC(b62cc7af) SHA1(048f80ad995a1516f97bb7544d3fb608a93893b1))
ROM_END

// Abit AH4/AH4T/AN4R2 ( the T model has a voltage regulator for DX4 CPUs) - CPU: Socket 3 - Chipset: SIS 85C471 / SIS 85C407
// RAM: 4xSIMM72, Cache: 9x28pin (32pin sockets except TAG) - BIOS: AMI - Keyboard-BIOS: AMIKEY - OSC: 14.31818 - ISA8: 1, ISA16: 4, ISA16/VL: 3
// BIOS-String: 08/30/95-SIS-85C471-2C4I9A12-02
ROM_START( abah4 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ah4t_an4r2_02.bin", 0x10000, 0x10000, CRC(b45dc3b7) SHA1(94206ac9ed50fc37d954cc3cd1fb062fd75ea984))
ROM_END

// Aopen VI15G - Chipset: SiS 85C471, 85C407 - CPU: Socket 3 - RAM: 4xSocket72, Cache: 4x32pin, 4x28pin, TAG - BIOS: 28pin
// Keyboard-BIOS: AMIKEY-2 - ISA16: 4, ISA16/VL: 3
ROM_START( aovi15g )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:
	ROM_SYSTEM_BIOS(0, "072594", "07/25/94")
	ROMX_LOAD( "amiwinch.bin", 0x10000, 0x10000, CRC(9ee72bef) SHA1(25f08714a384d68777d1570cf28a4db8b390257e), ROM_BIOS(0))
	// 1:
	ROM_SYSTEM_BIOS(1, "r23", "R23")
	ROMX_LOAD( "vi15gr23.rom", 0x10000, 0x10000, CRC(b704f571) SHA1(485795b8756069723dac865919dc3915a3162c12), ROM_BIOS(1))
ROM_END

// ASUS VL/I-486SV2G (GX4) (4xSIMM72, Cache: 128/256/512/1024KB, 7 ISA, 2 VLB)
// SiS 85C471 + 85C407; AMIKEY-2
ROM_START( a486sv2g )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 11/17/94-SIS-85C471-I486SV2G-00 / #401A0-0304
	ROM_SYSTEM_BIOS(0, "v304", "ASUS VL/I-486SV2G (GX4) V3.04")
	ROMX_LOAD( "sv2g0304.bin", 0x10000, 0x10000, CRC(cceabe6f) SHA1(45d0e25603045255d1ccaf5cbddd1a9146f61529), ROM_BIOS(0))
	// 1: BIOS-String: 01/11/95-SIS-85C471-I486SV2G-00 / #401A0-0305-1
	ROM_SYSTEM_BIOS(1, "v305", "ASUS VL/I-486SV2G (GX4) V3.05")
	ROMX_LOAD( "0305.001", 0x10000, 0x10000, CRC(9f2f9b75) SHA1(789807d82e39d69f948f7897f99b2fe362330dd1), ROM_BIOS(1))
	// 2: BIOS-String: 03/28/95-SIS-85C471-I486SV2G-00 / #401A0-0306 - complains about BIOS ROM checksum error
	ROM_SYSTEM_BIOS(2, "v306", "ASUS VL/I-486SV2G (GX4) V3.06")
	ROMX_LOAD( "asus_0306.bio", 0x10000, 0x10000, BAD_DUMP CRC(c87b7b55) SHA1(651938bcfdf6813a1e66c0e1b4812efe91740c91), ROM_BIOS(2))
	// 3: BIOS-String: 08/22/95-SIS-85C471-I486SV2G-00 / #401A0-0401
	ROM_SYSTEM_BIOS(3, "v401", "ASUS VL/I-486SV2G (GX4) V4.01")
	ROMX_LOAD( "sv2g0401.bin", 0x10000, 0x10000, CRC(f544f65a) SHA1(9a5e39cfbd545a0026f959b42dbc742246205b3c), ROM_BIOS(3))
	// 4: BIOS-String: 11/03/95-SIS-85C471-I486SV2G-00 / #401A0-0402-1
	ROM_SYSTEM_BIOS(4, "v402", "ASUS VL/I-486SV2G (GX4) V4.02")
	ROMX_LOAD( "sv2g0402.bin", 0x10000, 0x10000, CRC(db8fe666) SHA1(e499da86261bc6b312a6bc3d94b9465e17c5a449), ROM_BIOS(4))
	// 5: BIOS-String: 11/19/97-SIS-85C471-I486SV2GC-00 / #401A0-0402-1
	ROM_SYSTEM_BIOS(5, "v402b", "ASUS VL/I-486SV2G (GX4) V4.02 beta")
	ROMX_LOAD( "0402.001.bin", 0x10000, 0x10000, CRC(4705a480) SHA1(334c3d57cb6cb157798cd189207288c731a4dd7b), ROM_BIOS(5))
ROM_END

// Chaintech 486SLE M106 4SLE-Z1 - Chipset: SiS 85C407 85C471 - CPU: i486DX2-66 - BIOS: Award v4.50G - Keyboard-BIOS: Lance LT48C41
// BIOS-String: 11/09/94-SIS-85C471E-2C4I9C31-00 / 11/24/94 - ISA8: 1, ISA16: 3, ISA16/VL: 3 - OSC: 14.31818
ROM_START( ch4slez1 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-chaintech_486_sle.bin", 0x10000, 0x10000, CRC(8292bdb7) SHA1(461d582ea9fee4113d3a8ac050f76c7057ead7c7))
ROM_END

// Gemlight GMB-486SG rev 2.2 - Chipset: SiS 85C471 85C407 - BIOS/Version: Award - Keyboard BIOS: JETkey V5.0G
// CPU: 80486DX2-66 - RAM: 4xSIMM72, Cache: 5x 28pin DIP (TI256 SA 20TP fittet), 4x32pin DIP (W24257AK-15 fitted) - ISA8: 1, ISA16: 2, ISA16/VLB: 3
ROM_START( gmb486sg ) // BIOS-String: 01/10/95-SIS-85C471B/E/G-2C4I9G30-00
	ROM_REGION32_LE(0x20000, "bios", 0) // screen remains blank
	ROM_LOAD( "gmb486sg.bin", 0x10000, 0x10000, CRC(1f199b35) SHA1(0c4b19762426a30f7121c5c17f1b25a54a5df1f0))
ROM_END

// Gigabyte GA-486VF REV.6 - Chipset: SiS 85C407 85C471 - CPU: Cyrix Cx486 DX 40 - BIOS: Award L4162439, 28pin - Keyboard-BIOS: Lance LT38C41
// BIOS-String: 04/27/94-SIS-85C471-2C4I8G01-00 - ISA8: 1, ISA16: 3, ISA16/VL: 3 - OSC: 14.318MHz
ROM_START( ga486vf )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ga-486svf.bin", 0x10000, 0x10000, CRC(e9fb3153) SHA1(b8e307658f95c3e910728ac9316ad83e7afdb551))
ROM_END

// Gigabyte GA-486VS - CPU: 486 - Chipset: SiS 85C471, 85C407 - Keyboard-BIOS: Lance LT38C41 - ISA16: 3, ISA16/VL: 3
// BIOS-String: 11/21/94-SIS-85C471B/E/G/2C4I9G01-00 / Nov 21, 1994 Rev.A
ROM_START( ga486vs )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "vs1121.rom", 0x10000, 0x10000, CRC(0afadecf) SHA1(66c0655b5c4905438603097998a98407bfa376e6))
ROM_END

// MSI MS-4132 G VER:1 - Chipset: SiS 85C471, 85C407, BIOS: AMI 486DX ISA BIOS 08/08/93 AB4827039 - CPU: SOCKET 3, solder pads for 8486
// RAM: 4xSIMM30, 2xSIMM72, Cache: 5xW24257AK-15, 4xIS61C256AH-15N - ISA16: 4, ISA16/VLB: 3 (2 master, 1 slave) - OSC: 14.31818MHz
ROM_START( ms4132 ) // BIOS String: 40-0100-001169-00101111-080893-SIS471B / A75A 033194
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ms4132g.bin", 0x10000, 0x10000, CRC(23385e9d) SHA1(3637febf6e037aec9328d99877550ee9dee4c78c))
ROM_END

// MSI MS:4138 VER:1.3 - Chipset: SiS 85C471, 85C407 - CPU: Socket 3 - BIOS: EPROM/AMI 486DX ISA BIOS AC0250679
// Keyboard-BIOS: Winbond W83C42 - BIOS-String: - ISA16: 4, ISA16/VL: 3
ROM_START( ms4138 )
	ROM_REGION32_LE( 0x20000, "bios", 0)
	// 0: no display
	ROM_SYSTEM_BIOS( 0, "a75n", "A75N")
	ROMX_LOAD( "a75n.rom", 0x10000, 0x10000, CRC(f9b2130c) SHA1(7798b68275e547e858ba162abc5cf94dd6a85f4c), ROM_BIOS(0))
	// 1: no display
	ROM_SYSTEM_BIOS( 1, "msi4138", "MSI MS-4138")
	ROMX_LOAD( "ms-4138.bin", 0x10000, 0x10000, CRC(5461c523) SHA1(adb9fe0afa860897d575403a810ff44c85b9f93c), ROM_BIOS(1))
	// 2: BIOS-String: 08/14/95-SIS-85C471B/E/G-2C3I9W40-00 / W753BETA 26JAN96
	ROM_SYSTEM_BIOS( 2, "w753beta", "W753BETA")
	ROMX_LOAD( "w753beta.bin", 0x10000, 0x10000, CRC(4aeeba0b) SHA1(9d088c940599110ce3acea84bb881a61d42b6dcf), ROM_BIOS(2))
ROM_END

// DTK PKM-0038S E-2A  aka Gemlight GMB-486SG - Chipset: SIS 85C471, 85C407 - BIOS/Version: 01/10/95 Award (DTK PKM0038S.P02.03.02), 28pin - Keyboard-BIOS: JETkey V5.0
// CPU: Socket 3 - ISA8: 1, ISA16: 3, ISA16/VL: 3 - OSC: 14.318
ROM_START( pkm0038s )
	ROM_REGION32_LE( 0x20000, "bios", 0)
	// 0: BIOS-String: 01/10/95-SIS-85C471B/E/G-2C4I9G30-00 / (2C4I9G30) DTKPKM0038S.P2.03.02
	ROM_SYSTEM_BIOS(0, "p20302", "P2.03.02")
	ROMX_LOAD( "pkm0038s.bin", 0x10000, 0x10000, BAD_DUMP CRC(f6e7dd88) SHA1(5a2986ff0e6352ade8d5b0abaa86e436dddcf226), ROM_BIOS(0)) // BIOS ROM checksum error
	// 1: BIOS-String: 07/11/94-SIS-85C471E-2C4I9G30-00 / (2C4I9G30) DTKPKM0038S.P02.02.0
	ROM_SYSTEM_BIOS(1, "p2020", "P2.02.0")
	ROMX_LOAD( "4siw005.bin", 0x10000, 0x10000, CRC(0b8794d3) SHA1(153b38f6815a8a3d6723eb17df7ffc37054b3194), ROM_BIOS(1))
ROM_END

// EFA 4DMS HL3G-L4-VI - Chipset: SIS-85C471B/E/G / SIS471E/G - CPU: Socket 3 - RAM: 2xSIMM72, Cache: 8x32pin+TAG - ISA16: 5, ISA16/VL: 3
// BIOS: Award - BIOS-String: 12/07/95-SIS-85C471B/E/G-2C4I9E30-00 / 4DMS-HL3GC Award BIOS REV 1.05 01/12/'96
ROM_START( 4dmshl3g )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "3g105.bin", 0x00000, 0x20000, CRC(60e4841f) SHA1(60ad11e4e4a60eef858d837470a9014706e7576a))
ROM_END

// Soyo 025K2 - Chipset: SiS 85C471, 85C407 - BIOS: Award BIOS ISA 486 S/N 240383 - Rev G2 - 09/26/94 - Keyboard BIOS: JETkey V5.0
// CPU: Socket 3 - RAM: 4xSIMM30, 2xSIMM72, Cache: 8xtm11256-20, 1xAster AE88128AK-15 - ISA16: 4, ISA16/VLB: 3 (one marked MASTER)
ROM_START( so025k2 ) // BIOS-String: 09/26/94-SIS-85C471B/E/G-2C4I9S23-00 / REV .G2.
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "sy025k2.bin", 0x10000, 0x10000, CRC(a75fdf9a) SHA1(ce7a595ec3bb33acac76a72f704a58e08d54847a))
ROM_END

// Zida 4DVS - Chipset:  SiS 85C471, 85C407 - CPU: Socket 3, RAM: 4xSIMM72, Cache: 4x28pin, 4x32pin, TAG
// ISA8: 1, ISA16: 3, ISA16/VL: 3 - Keyboard-BIOS: AMIKEY-2
// BIOS-String:
ROM_START( zi4dvs )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: 02/09/95-SIS-85C471B/E/G-2C4I9000-00 / 12/10/95 SIS 85C471G
	ROM_SYSTEM_BIOS(0, "4dvs20", "4DVS20")
	ROMX_LOAD( "4dvs20.awa", 0x10000, 0x10000, CRC(831d33cb) SHA1(e5c3f01a9c93a7cf9dbcdc750e87952a5b6a5cf4), ROM_BIOS(0))
	// 1: blank screen
	ROM_SYSTEM_BIOS(1, "072594", "AMI 07/25/94")
	ROMX_LOAD( "4dvs-471.amw", 0x10000, 0x10000, CRC(da749314) SHA1(686321ffa59cd2259f4fe65a28b86c88cf739393), ROM_BIOS(1))
	// 2: blank screen
	ROM_SYSTEM_BIOS(2, "121593", "AMI 12/15/93")
	ROMX_LOAD( "486-ab8068594.bin", 0x10000, 0x10000, CRC(92eee700) SHA1(bd34360cf9a9849e0805cdb575cd7a0e007dd2f5), ROM_BIOS(2))
ROM_END

ROM_START( sis85c471 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: Chipset: SiS 85C407, another chip with the Energy Star/Green PC label (85C441) - CPU: 486 - BIOS: Award BIOS ISA 486 036875 - Keyboard-BIOS: Lance LT38C41
	// BIOS-String: 04/28/94-SIS-85C471-2C4I8S21-00 / REV. B - ISA16: 4, ISA16/VL: 3 (2 master)
	ROM_SYSTEM_BIOS(0, "revb", "REV. B")
	ROMX_LOAD("486-sis_green.bin", 0x10000, 0x10000, CRC(9d3b5022) SHA1(f11b27bb24deb2466226486cf8ba66ddbeff87d6), ROM_BIOS(0))
	// 1: Chipset: SiS 85C407 85C471 - CPU: Cyrix Cx486DX2-66 - BIOS: Award E0042537 - Keyboard-BIOS: Lance LT38C41 - ISA8: 1, ISA16: 3, ISA16/VL: 3
	// BIOS-String: 02/07/94-SIS-85C371-2C4I8C30-00 / 02/17/94
	ROM_SYSTEM_BIOS(1, "486sl", "486SL")
	ROMX_LOAD("486-sis_486sl.bin", 0x10000, 0x10000, CRC(7261263e) SHA1(d5c4ee484941bbb8ca756c5f6e53382748bbcfd6), ROM_BIOS(1))
	// 2: REV:1.2 - Chipset: SiS 85C471 P85C407 - CPU: Socket 3 - BIOS: AMI 486DX ISA BIOS AC03601316 (28pin) - Keyboard-BIOS: JETkey V5.0G - RAM: SIMM72x4, Cache: 8 sockets+1
	// ISA8: 1, ISA16: 4, ISA16/VL: 3
	ROM_SYSTEM_BIOS(2, "rev12", "REV.1.2") // no display
	ROMX_LOAD("486-sis_ac0360136.bin", 0x10000, 0x10000, CRC(940e3643) SHA1(f931f5c2b39ebb6c509033984ab050ffa1ff4334), ROM_BIOS(2))
ROM_END


// ***** 486 motherboards using the UMC UM8498F, UM8496F chipset

// Aquarius MD-4DUVC VER:2.1 / Aquarius MD-4DUV VER:2.1
ROM_START( md4duvc ) // "Memory test fail"
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: CPU: Socket 3/Overdrive - Chipset: UMC UM8496F, UM8498F - RAM: 2xSIMM72, 4xSIMM30, Cache: 9xW2457AX-15, sockets are 32pin
	// ISA16: 4, ISA16/VL: 3 - BIOS: 28/32pin
	ROM_SYSTEM_BIOS(0, "md4duvc", "MD-4DUVC") // BIOS-String: 01/12/94--2C4X6H01-00 / Release 01/04/95
	ROMX_LOAD( "md-4duvc.dmg", 0x10000, 0x10000, CRC(40d208bb) SHA1(c879599d2635c093fce420d1e7081631d27c621a), ROM_BIOS(0))
	// 1: Chipset: UMC UM8498F, UM8496F - BIOS/Version: 486SX/DX Award, R3.1,  I194220, 1994-1995 - CPU: Socket 3
	// RAM: 4xSIMM72, Cache: 9xW24257AK-15 - ISA16: 4, ISA16/VLB: 3
	ROM_SYSTEM_BIOS(1, "md4duv", "MD-4DUV") // BIOS-String: 11/1794-UMC-498GP-2C4X6A31-00 / MB-4DUV/UVC VER 3.1
	ROMX_LOAD( "atrom.bin", 0x10000, 0x10000, CRC(ecb764f5) SHA1(f34a7671e9efc6a6cd7ff1516c0c8ecbbfcd55e0), ROM_BIOS(1))
ROM_END

// BIOSTAR MB-1433UIV - Chipset: BIOTEQ 83C3498, 83C3496 - CPU: Socket 3 - RAM: 3xSIMM30, 4xSIMM72, Cache: 4x32pin, 4x28pin + TAG
// ISA16: 4, ISA16/VL: 3 - BIOS: 28pin
ROM_START( mb1433uiv )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:
	ROM_SYSTEM_BIOS(0, "um8496fw", "UM8496FW")
	ROMX_LOAD( "um8496fw.ami", 0x10000, 0x10000, CRC(4a6dcc36) SHA1(f159f67eb662272244cd1781814ebcb5204a2625), ROM_BIOS(0))
	// 1:
	ROM_SYSTEM_BIOS(1, "w4", "W4")
	ROMX_LOAD( "um498-w4.amw", 0x10000, 0x10000, CRC(12fe6697) SHA1(2506dea874728916dc37f7dad8e8caf214a28525), ROM_BIOS(1))
ROM_END

// PC-Chips M912 - Chipset: UM8498F, UM8496F - CPU: 486 - BIOS: AMI - ISA16: 4, ISA16/VL: 3
ROM_START( pccm912 ) // no display
	ROM_REGION32_LE( 0x20000, "bios", 0)
	// 0:
	ROM_SYSTEM_BIOS( 0, "072594", "07/25/94")
	ROMX_LOAD( "m912.bin", 0x10000, 0x10000, CRC(7784aaf5) SHA1(f54935c5da12160251104d0273040fea22ccbc70), ROM_BIOS(0))
	// 1:
	ROM_SYSTEM_BIOS( 1, "120295", "12/02/95")
	ROMX_LOAD( "m912_12-02-1995x.bin", 0x10000, 0x10000, CRC(28a4a140) SHA1(a58989ab5ad5d040ad4f25888c5b7d77f31a4d82), ROM_BIOS(1))
ROM_END

// Pine Technology PT-430 - Chipset: UMC UM8498F UM8496F - BIOS: AMI 486DX ISA BIOS AB8906726 28pin - Keyboard-BIOS: silkscreen 8742, but socket empty
// BIOS-String: - ISA8: 1, ISA16: 3, ISA16/VL: 3 - OSC: 14.31818MHz
ROM_START( pt430 ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "pt430.bin", 0x10000, 0x10000, CRC(d455c949) SHA1(c57c82ed015528f3d223f59c94ed6b8a9c323c39))
ROM_END

// PowerTech MB457 aka Pine PT-2068.1 - Chipset: UMC UM8498F & UMC UM8496F - CPU: Intel 80486DX4-100, solder pads for UMC U5 Green CPU
// RAM: 4xSIMM30, 1xSIMM72 - ISA8: 1, ISA16: 4 - BIOS/Version: Award 486DX J314549
ROM_START( ptmb457 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 10/19/94-UMC-498GP-2C4X6S21-00 / REV A
	ROM_SYSTEM_BIOS(0, "101994", "10/19/94")
	ROMX_LOAD( "umc-mb457-1.bin", 0x10000, 0x10000, CRC(71f66fec) SHA1(c798d68eb851fc93cc8a3dd67009b47388488e51), ROM_BIOS(0))
	// 1: BIOS-String: 02/07/95-UMC-498GP-2C4X6000-00 / PT-2068.1
	ROM_SYSTEM_BIOS( 1, "020795", "02/07/95")
	ROMX_LOAD( "umc-mb457-2.bin", 0x10000, 0x10000, CRC(2654aefd) SHA1(b888f4f891108a5ef268688840ff20be3a8e6aa5), ROM_BIOS(1))
ROM_END

// Soyo 025R2 - Chipset: UM8498F, UM8486F - CPU: Socket 3 - RAM: 4xSIMM30, 2xSIMM72, Cache: 4x28pin, 4x32pin, TAG, used: 4xUM61512AK-15, AE88128AK-15
// BIOS: 28pin - ISA16: 4, ISA16/VL: 3
// BIOS-String: 08/28/UMC-498GP-2C4X6S21-00 / REV B2
ROM_START( so025r2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD("25r2-b2.bin", 0x10000, 0x10000, CRC(3b73360c) SHA1(eaaf47236154a9cc81ffda4c11086960aed0dadf))
ROM_END

// ID: ADI F4DXL-UC4 - Chipset: UM8498F, one unreadable - BIOS: AMI 486DX BIOS AB345213 - CPU: Socket 3 - RAM: 4xSIMM30, 2xSIMM72
// Cache: 4x32pin DIP, 5x28pin DIP (used: 9xCL63C256N-20) - OSC: FM14318 - ISA16: 4, ISA16/VLB: 3
ROM_START( f4dxluc4 ) // BIOS-String: 40-0E0-008060-00101111-12159-UM498-0 / ADI/UC4/V 1.0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486_ab345213.bin", 0x10000, 0x10000, CRC(e5b85a92) SHA1(aade1fb1463b07a616c2594293bf0215c9652511))
ROM_END

ROM_START( um8498f ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_DEFAULT_BIOS("498gp")
	// 0: BIOS-String: 40-P101-001437-00101111-072594-GREEN-H - CPU: Socket 3 - RAM: 4xSIMM30, 2xSIMM72, Cache: 9xUM61256AK-15
	// BIOS: AMI AB9300757
	ROM_SYSTEM_BIOS(0, "v14", "V1.4") // no display
	ROMX_LOAD( "4umm001.bin", 0x10000, 0x10000, CRC(a5b768b4) SHA1(904ce2814d6542b65acec0c84532946172f2296d), ROM_BIOS(0))
	// 1:
	ROM_SYSTEM_BIOS(1, "um849801", "UM8498 #1") // no display
	ROMX_LOAD( "um8498.ami", 0x10000, 0x10000, CRC(51f71bc7) SHA1(0986d60081d2c578a66789c0c53fe1d5919c553f),ROM_BIOS(1))
	// 2: Chipset: UM8498F + UM8496F - BIOS label: Award BIOS ISA 486 485427 - BIOS version: Award Modular BIOS v4.50G - CPU: UMC U55X 486-33F, solder pads for 80486socket
	// RAM: 4xSIMM30, 2xSIMM72 - ISA8: 1, ISA16: 5
	// BIOS-String: 12/08/94-UMC-498GP-2C4X6S21-00 / REV A1
	ROM_SYSTEM_BIOS(2, "498gp", "498GP")
	ROMX_LOAD( "award_bios_isa_486.bin", 0x10000, 0x10000, CRC(ce3ccaa4) SHA1(3fdc9282d9934e18e45b46b50644022fc0387f33), ROM_BIOS(2))
ROM_END


// ***** 486 motherboards using the UM82C482A UM82C481A chipset

// Elitegroup UM486/UM486sx Rev.1.4. - Chipset: UMC UM82C482A UM82C481A UM82C206F - ISA8: 2, ISA16: 6 - CPU: i486DX-50, FPU socket provided
// RAM: 8xSIMM30 in 2 banks, Cache: 8xW24256AK-25+4xCY7C164-20PC - OSC: 33.000MHz, 14.31818MHz - BIOS: AMI 486 BIOS ZZ364969 - Keyboard-BIOS: AMI KB-BIOS-VER-F
// BIOS-String: 40-0500-D01131-00101111-070791-UMCWB-0 / UM486/486SX V1.3 09-24-91
ROM_START( um486 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-um486-um486sx.bin", 0x10000, 0x10000, CRC(0f20d193) SHA1(e9c7365b0343a815e5abc9726d128353becc18d3))
ROM_END

// Elitegroup UM486V-AIO - Chipset: UMC UM82C482AF, UM82C481BF, UM82C863F, UM82C865F, UM82C206F - ISA16: 4, ISA16/VL: 2
// BIOS: AMI - CPU: 486 - On board: Floppy, 1xIDE, parallel, 2x serial
// BIOS-String: 40-0100-001131-00101111-111192-UMC480-0 / UM100 V2.1 04-26-93
ROM_START( um486v )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "um486v.bin", 0x10000, 0x10000, CRC(eb52d3fd) SHA1(84f63904cfceca9171b5c469545068e19ae280a8))
ROM_END


// ***** 486 motherboards using the UM8886BF, UM8881F chipset

// A-Trend ATC-1415 - Chipset: UMC UM8881F/UM8886BF, OEC12C885, VT8235N - ISA8: 1, ISA16: 3, PCI: 3
// CPU: Socket 3 - BIOS: Award C 0247007 UMC8881/6 BIS VER : 3.20 95-12-08 / M271001 (128k)
// BIOS-String: 11/24/95-UMC-881/886A-2A4X5A2HC-00
ROM_START( atc1415 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "4umw002.bin", 0x00000, 0x20000, CRC(eacaa3de) SHA1(88c828def72aeae0798ba5a4a5c31cd465545c0b))
ROM_END

// Biostar MB8433UUD-A (4 SIMM, 2 IDE, 3 PCI, 4 ISA)
// UMC UM8881F, UM8886BF, UM8663AF; DS12887 RTC
ROM_START( mb8433uud ) // BootBlock BIOS
	ROM_REGION32_LE(0x20000, "bios", 0) // Intel Flash P28F010
	// 0: BIOS-String: 05/20/96-UMC-881E/886B-2A4X5B08C-00 / UUD960520S EVALUATION ROM - NOT FOR SALE
	ROM_SYSTEM_BIOS(0, "520s", "520S")
	ROMX_LOAD( "uud0520s.bin", 0x00000, 0x20000, CRC(0e347559) SHA1(060d3040b103dee051c5c2cfe8c53382acdfedad), ROM_BIOS(0))
	// 1: BIOS-String: 05/20/96-UMC-881E/886B-2A4X5B08C-00 / BIOSTAR MB-8433UUD v2014
	ROM_SYSTEM_BIOS(1, "2014", "2014")
	ROMX_LOAD( "uud2014.bin", 0x00000, 0x20000, CRC(315f7519) SHA1(e0174e4982d1861c64d871a7806b793a914f2366), ROM_BIOS(1))
	// 2: BIOS-String: 12/05/95-UMC-881E/886B-2A4X5B08C-00 / UUD951222S
	ROM_SYSTEM_BIOS(2, "8881d", "8881D")
	ROMX_LOAD( "um8881_d.awa", 0x00000, 0x20000, CRC(a47327b9) SHA1(6b57155cf34e2c597b7621e49ae8354c05cebfde), ROM_BIOS(2))
	// 3: BIOS-String: 11/07/95-UMC-881E/886B-2A4X5B08C-0 / UUD951108A
	ROM_SYSTEM_BIOS(3, "08a", "08A")
	ROMX_LOAD( "8433uud.awd", 0x00000, 0x20000, CRC(17ca5c2a) SHA1(7cda78f87f35fe8574a1f2d70d548cc2bc2af05b), ROM_BIOS(3))
ROM_END

// Gigabyte GA-486AM/S - CPU: Socket 3 - Chipset: UMC UM8886AF, UM8881F, UM88663AF, UM8667 - RAM: 4xSIMM72, Cache: 9x32pin, occupied: 4xUM6152AK-15, 1xUM61M256K-15
// BIOS: 32pin - OSC: 14.31818 - On board: 2xIDE, Floppy, par, 2xser - ISA16: 4, PCI: 3
ROM_START( ga486am ) // BootBlock
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "4am1107.bin", 0x00000, 0x20000, CRC(e836798d) SHA1(afd72654ed210bb19e37f9df96b9ecb75c84712a))
ROM_END

// PC-Chips M915i - CPU: 486 - Chipset: UM8881F, UM8886F - ISA16: 2, ISA16/VL: 2, PCI: 4 - On board: 2xIDE
ROM_START( pccm915i )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: no display
	ROM_SYSTEM_BIOS(0, "9151108s", "9151108S")
	ROMX_LOAD( "9151108s.rom", 0x00000, 0x20000, CRC(cba5525c) SHA1(9bdb586090f613a7172f6b46ed78e36331bf2135), ROM_BIOS(0))
	// 1: Amptron DX9200 aka PcChips M915i with Fake cache - Chipset: UMC UM8881F + UM8886BF, SECKS82C6818A - EPROM: AMD AM27C010
	// BIOS label: AMI 486DX ISA BIOS 1993 AC9051796 - BIOS Version: AMI 10/10/1995: 11-08-1995 - on board: 2xIDE
	// CPU: Socket 3 - RAM: 4xSIMM72, Cache: 9xblack blocks of plastic - OSC: 14.31818 - ISA16: 2, ISA16/VLB: 2, PCI: 4
	ROM_SYSTEM_BIOS(1, "dx9200", "Amptron DX9200") // BIOS String: 41-p400-001437-00101111-101094-486AVIP-H
	ROMX_LOAD( "m915_fake_cache.bin", 0x00000, 0x20000, CRC(82a4e810) SHA1(b20a6e128d6298adf8487d190dd182a751dfccf9), ROM_BIOS(1))
ROM_END

// PC-Chips M919 - this motherboard showcased the issues that gave PC-Chips its bad name, it was available with fake cache, a proprietary cache socket or with fully operational cache
// Chipset: UMC UM8881F/9714-EYS and UM8886BF/9652-FXS (V3.4B/F), UMC UM8886BF/9618-FXS and UM8881F/9622-EYS (Rev. 1.5)
// http://th2chips.freeservers.com/m919/ this mentions that the BIOS requires a flashable chip
ROM_START( pccm919 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "m919v1", "PC-Chips M919 v1")
	ROMX_LOAD( "9190914s.rom", 0x00000, 0x20000, CRC(bb18ff2d) SHA1(530d13df21f2d483ec0dddda44fb4fe7e29ec040), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "m919v2", "PC-Chips M919 v2")
	ROMX_LOAD( "9191016s.rom", 0x00000, 0x20000, CRC(2a2125a6) SHA1(753061ae6f80c0ca42d1af91aada657910feae18), ROM_BIOS(1))
ROM_END

// Shuttle HOT-433 - Chipset: UM8886BF, UM8881F, UM8669F, ??667
// CPU: Cyrix 5x86-120GP - ISA16: 4, PCI: 4 - On board: PS2-Mouse, 2xser, Floppy, par, 2xIDE
// Versions 1-3 can use Flash or EPROM, Version 4 only EPROM
ROM_START( hot433 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 07/25/95-UMC-881/886A-2A4X5H21-00 / (433WIE10) UMC880A For486PCI Green_PC
	ROM_SYSTEM_BIOS(0, "wie10", "WIE10")
	ROMX_LOAD( "433wie10.bin", 0x00000, 0x20000, CRC(90604ef4) SHA1(61e160678d48cb5752c84090ca990e09382ae01d), ROM_BIOS(0))
	// 1: BIOS: 10/10/94 AMI (or 02/02/95 depending on where you look), 486PCI/ISA 057890 in a Winbond W29EE011-15 - no display
	ROM_SYSTEM_BIOS(1, "v401", "V4.0 #1")
	ROMX_LOAD( "hot433.bin", 0x00000, 0x20000, CRC(1c279c6f) SHA1(4a0e99fafc5719959fb5800a61629c3f36778240), ROM_BIOS(1))
	// 2: Original AMI BIOS for rev 1-3 w/mouse support
	ROM_SYSTEM_BIOS(2, "aip16", "AIP16")
	ROMX_LOAD( "433aip16.rom", 0x00000, 0x20000, CRC(a9503fc6) SHA1(0ebd936f5478477e37528e6e487c567b064248f7), ROM_BIOS(2))
	// 3: AMI BIOS for the EPROM Programmer, not flashable
	ROM_SYSTEM_BIOS(3, "aue2a", "AUE2A")
	ROMX_LOAD( "433aue2a.rom", 0x00000, 0x20000, CRC(35f5633f) SHA1(01148eba919985165ab9cd12b5e6f509d6d1385f), ROM_BIOS(3))
	// 4: AMI BIOS for the EPROM Programmer, not flashable
	ROM_SYSTEM_BIOS(4, "aue33", "AUE33")
	ROMX_LOAD( "433aue33.rom", 0x00000, 0x20000, CRC(803c4b1e) SHA1(483c799c08eed0d446384d67e9d23341499806b1), ROM_BIOS(4))
	// 5: AMI BIOS for rev 1-3.  Some reports say for rev4
	ROM_SYSTEM_BIOS(5, "aus2a", "AUS2A")
	ROMX_LOAD( "433aus2a.rom", 0x00000, 0x20000, CRC(766d1f3f) SHA1(1e59140bc91ab98fcadcf7bb77e222932696419f), ROM_BIOS(5))
	// 6: Latest AMI BIOS for rev 1-3
	ROM_SYSTEM_BIOS(6, "aus2c", "AUS2C")
	ROMX_LOAD( "433aus2c.rom", 0x00000, 0x20000, CRC(bdc65766) SHA1(e87cc4aed14ae7fcdf6423063b0ababe65b41044), ROM_BIOS(6))
	// 7: AMI BIOS for rev 1-3 w/mouse support
	ROM_SYSTEM_BIOS(7, "aus26", "AUS26")
	ROMX_LOAD( "433aus36.rom", 0x00000, 0x20000, CRC(8f864716) SHA1(0bf4b8114cbb406646d89eed7833556611e1fbe6), ROM_BIOS(7))
	// 8: Latest AMI BIOS for rev4 of the Shuttle HOT-433 motherboard.
	ROM_SYSTEM_BIOS(8, "aus33", "AUS33")
	ROMX_LOAD( "433aus33.rom", 0x00000, 0x20000, CRC(278c9cc2) SHA1(ecd348106d5118eb1e1a8c6bd25c1a4bf322f3e6), ROM_BIOS(8))
	// 9: boots to BootBlock BIOS
	ROM_SYSTEM_BIOS(9, "2a4x5h21", "2A4X5H21")
	ROMX_LOAD( "2a4x5h21.bin", 0x00000, 0x20000, CRC(27c47b90) SHA1(09d17bc5edcd02a0ff4a3a7e9f1072202880251a), ROM_BIOS(9))
	// 10: no display
	ROM_SYSTEM_BIOS(10, "shuttle", "Shuttle HOT-433")
	ROMX_LOAD("bios.bin", 0x00000, 0x20000, CRC(a5ffafac) SHA1(0ec71a82c80a96a5724ba375934c709db468dabc), ROM_BIOS(10))
ROM_END

// PROTECH PM486PU-S7 - Chipset: UMC 881/886A (UM8881F/UM8886AF), SMC FDC, Winbond
// BIOS Chip: TI/TMS 27C010A (128K) - CPU: i486DX-33 - On board: 2xIDE, FDC, 2xser, par - RAM: 2xSIMM72, Cache: 4xGLT721208-15 +1 TAG - ISA16: 4, PCI: 3
// BIOS-String: 10/11/95-UMC-881/886A-2A4X5P62-00 / (PM486PU-S7) 486 WITH I/O PCI LOCAL BUS SYSTEM ...
ROM_START( pm486pu )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "pm486pu-s7-2a4x5p62-00.bin", 0x00000, 0x20000, CRC(143bdc07) SHA1(e2cf2ac61fd3e4797e5d737dfec4a2b214469190))
ROM_END

// Pine PT-432b - Chipset: UMC UM8886BF, UM8881F, UM8663F, UM8287, UM8667 - CPU: Am486DX4-100 - RAM: 4xSIMM72, Cache: 8+1 sockets
// ISA16: 4, PCI: 3 - BIOS: AMI 486PCI ISA BIOS AA0841149 (32pin) - On board: 2xser, par, 2xIDE, Floppy
ROM_START( pt432b ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "sr_m401-a.bin", 0x00000, 0x20000, CRC(ff8cd351) SHA1(a9c6a54f38b1b548fba4d7d42643f117441b09a6))
ROM_END

// TD-4IP-UMC-AIO - CPU: Socket 3 - Chipset: UMC UM8881F, UM8886BF, UM8663BF, UM8667, bq3285P - RAM: 2xSIMM72, Cache: W24257AK-15, W24512AK-20 (4)
// BIOS: Award 020175447 / 29EE010 (128k) - BIOS-String: 12/19/95-UMC-881E/886B-2A4X5CF9C-00 - ISA16: 4, PCI: 3
ROM_START( td4ipaio )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "4ummw001.bin", 0x00000, 0x20000, CRC(4e716d77) SHA1(1b7af88c4da5ca388acbc0cb66bd26a0ae8f4931))
ROM_END

ROM_START( um8886 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: no display - UMC PCI 95C-0123 - Chipset: UMC UM8886AF, UM8881F, 4xUM8002, UM8663AF, UM8667 - CPU: Socket 3 - On board: 2xser, par, Floppy, 2xIDE - 4xISA16, 4xPCI
	// BIOS: AMI 486 PCI ISA in M27C1001 EPROM
	ROM_SYSTEM_BIOS( 0, "pci95c", "PCI 95C-0123")
	ROMX_LOAD( "486-umc_pci_95c-0123.bin", 0x00000, 0x20000, CRC(9db58de4) SHA1(5441f3181fb26911d796c4bf019136aa8e4c060b), ROM_BIOS(0))
	// 1: no display - V1.1A - Chipset: UMC UM8886AF UM8881F, UM8667, UM8663AF - CPU: i486DX2-66 - On board: 2xser, 2xIDE, Floppy, par - BIOS: AMI 486DX ISA BIOS AC6288199 - ISA16: 4, PCI: 3
	ROM_SYSTEM_BIOS( 1, "pcimini", "PCI mini")
	ROMX_LOAD( "486-umc-pci mini.bin", 0x00000, 0x20000, CRC(4ee12b46) SHA1(9397f67b21f11cfda57abd5ab28f93055909ee97), ROM_BIOS(1))
ROM_END

// Elitegroup UM8810 PAIO - Chipset: UMC UM8881F, UM8886F, UM82C865FSMC FDC37C665GT, PCI0640B - CPU: Socket 3 - RAM: 4xSIMM72, Cache: 32pin sockets, 9xUM61256FK-15 occupied
// BIOS: 32pin - Keyboard-BIOS: AMIKEY-2 - ISA16: 4, PCI: 3 - On board: 2xIDE, Floppy, par, 2xser
ROM_START( um8810paio )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 09/27/94-UMC-881/886-2A4X5E11-00 / UM8810P-AIO V1.0 09-27-94'
	ROM_SYSTEM_BIOS(0, "awd", "AWD")
	ROMX_LOAD( "awd_09-27-94.bin", 0x00000, 0x20000, CRC(25c671ba) SHA1(6b7641fb0f619e233d956e7cdb4e246f3c525673), ROM_BIOS(0))
	// 1: BootBlock BIOS
	ROM_SYSTEM_BIOS(1, "aw123", "AW_1-23")
	ROMX_LOAD( "aw_1-23.bin", 0x00000, 0x20000, CRC(420c3a43) SHA1(e0744b0bf171d279650d1f3a731e231562fe60ae), ROM_BIOS(1))
	// 2: blank screen, possibly for update only
	ROM_SYSTEM_BIOS(2, "v21", "V21")
	ROMX_LOAD( "881_v21_01-27-95.rom", 0x00000, 0x20000, CRC(2443759b) SHA1(02a9e51b51c2d2d21d5ae821b6d8aa97f62c3327), ROM_BIOS(2))
	// 3: blank screen
	ROM_SYSTEM_BIOS(3, "21e", "21E")
	ROMX_LOAD( "881avio_01-08-96.21e", 0x00000, 0x20000, CRC(446cd0c3) SHA1(32a61c0578326d17840db37855644543b6406535), ROM_BIOS(3))
	// 4: blank screen
	ROM_SYSTEM_BIOS(4, "31e", "31E")
	ROMX_LOAD( "881avio_01-08-96.31e", 0x00000, 0x20000, CRC(b5ec7dca) SHA1(390f79f50de986c3874464202251398baa87f4b5), ROM_BIOS(4))
	// 5: blank screen
	ROM_SYSTEM_BIOS(5, "32h", "32H")
	ROMX_LOAD( "8810aio_06-27-96.32h", 0x00000, 0x20000, CRC(b7e9d590) SHA1(bfdfa64bc87f2eb0d362e87755dce70584675427), ROM_BIOS(5))
	// 6: blank screen, possibly for update only
	ROM_SYSTEM_BIOS(6, "32j", "32J")
	ROMX_LOAD( "8810aio_12-11-96.32j", 0x00000, 0x20000, CRC(c6baec8b) SHA1(79bccaefb75e2df071d6edd191eea74aa82dc620), ROM_BIOS(6))
	// 7: blank screen
	ROM_SYSTEM_BIOS(7, "32f", "32F")
	ROMX_LOAD( "881032f.rom", 0x00000, 0x20000, CRC(5c337ae6) SHA1(29bc48a3d5165afa79612baf6deb9c04c7e55b32), ROM_BIOS(7))
	// 8: blank screen
	ROM_SYSTEM_BIOS(8, "v145", "V145")
	ROMX_LOAD( "8810aio_v145_09-21-94.bin", 0x00000, 0x20000, CRC(a253c017) SHA1(8273defe95a13ea0a260d4a410d601f82a947ad9), ROM_BIOS(8))
	// 9: 06/23/95-UMC-881/886-2A4X5E11-00 / Version VBS1.10H UM8810 ECS (Highscreen boot logo)
	ROM_SYSTEM_BIOS(9, "v110h", "V1.10H")
	ROMX_LOAD( "vbs1.10h.bin", 0x00000, 0x20000, CRC(1bf29727) SHA1(77ccd34110ec0387cdcfa260332b403d0c197d17), ROM_BIOS(9))
ROM_END


// ***** 486 motherboards using the UMC UM82C491F UM82C493F or clones (BIOTEQ) chipset

// Chicony CH-491E Rev. 1.4 - Chipset: UMC UM82C491F UM82C493F - BIOS: 04/04/93 AMI AB1987679 28pin - Keyboard-BIOS: AMIKEY
// BIOS-String: 40-0102-001116-00101111-040493-UMC491F-0 / UMC 491 for 80486 AUTO - ISA16: 4, ISA16/VL: 3
ROM_START( ch491e )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ch491e.bin", 0x10000, 0x10000, CRC(2d24ff24) SHA1(72f35c19e907c6d0a03a49bd362c4f57cc89da1c))
ROM_END

// Aquarius System (ASI) MB-4D33/50NR VER:01 - Chipset: UMC UM82C491F UM82C493F - CPU: AM486DX2-66 - BIOS: Award 1060176, 28pin - Keyboard-BIOS: JETkey V5.0
// BIOS-String: 03/09/94-UMC-491-2C4X2A30-00 / MB-4D33/50NR-02 - ISA8: 1, ISA16: 3, ISA16/VL: 3
ROM_START( mb4d33 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-mb-4d33.bin", 0x10000, 0x10000, CRC(f1299131) SHA1(d8e2749e180135e23483e36a0a05479e64f23d8c))
ROM_END

	// BIOS-String: 40-0100-001281-00101111-080893-UMC491F-0 / HL3SC/SM AMI BIOS VER 4.9 01/09/'95
ROM_START( 4dmuhl3s )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "3s-49-1.rom", 0x10000, 0x10000, CRC(78a8ea39) SHA1(35e2d0103da28b93a8c1addb1f083bd6e4239d2a))
ROM_END

// Elitegroup ECS UC4915 A AIO - Chipset: UMC UM82C491F UM82C493F UM82C865F SMC FDC37C662QF P,  PROCHIP PR 4030 - CPU: Socket 3
// BIOS: AMI 486DX ISA BIOS AB2683223 28pin in 32pin socket - Keyboard-BIOS: Intel/AMI MEGA-KB-H-WP
// BIOS-String: 40-0401-001131-00101111-040493-UMC491C-0 / VOBIS UC4915-A V1.1 11-05-93' - ISA16: 4, ISA16/VL: 2 - OSC: 14.31818 - On board: IDE, Floppy, 2xser, par, Game
ROM_START( ec4915aio )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-ecs-uc4915-a-aio.bin", 0x10000, 0x10000, CRC(5b3429a3) SHA1(a1b3ddb6a0939d20ae66e034914ea94648ca7149))
ROM_END

//  Elitegroup UC4913 REV:1.1 (Peacock sticker)- Chipset: UMC UM 82491F 82C493F - CPU: 486 - BIOS: AMI 486DX ISA BIOS AA8960448 (28pin) - Keyboard-BIOS: AMI/Intel
// OSC: 14.31818 - RAM: SIMM30x8, Cache: 9 sockets, 4 (UM61256CK-20)+1 (MS6264A-20NC) filled - ISA8: 2, ISA16: 3, ISA16/VL: 3
// BIOS-String: 40-0401-001131-00101111-040493-UMC491C-0 / Peacock AG UC4913 BIOS Ver. 1.0 01.09.93
ROM_START( ec4913 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-peacock-uc4913-aa8960338.bin", 0x10000, 0x10000, CRC(58e6753c) SHA1(077c11532572ca0399f76a7ba2d31b8c1ca75d48))
ROM_END

// Biostar MB-1433UCV - Chipset: BIOTEQ 82C3491, 82C3493 (check mb133340 for a 386 motherboard using the same chipset)
// CPU: 486DX2-66 - RAM: 8xSIMM30, Cache: 8+1x28pin(AS57C256-20PC) - ISA8: 1, ISA16: 3, ISA16+VL: 3 - BIOS: AMI AB0975913 - Keyboard-BIOS: JETkey V5.0 - RTC: TH6887A 9410
ROM_START( mb1433ucv )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 40-0100-001223-00101111-040493-UMC491F-0 / MB-1333/40UCG-A, MB-1333/40UCQ-B / MB-1433-40UDV-A, MB-1433/50UCV-C, MB-6433/50UPC-A for EXT. RTC
	ROM_SYSTEM_BIOS(0, "ucvd", "UCV-D")
	ROMX_LOAD( "biostar_bios_mb-1433-50ucv-d_pcb_ver_2.bin", 0x10000, 0x10000, CRC(e5ff2d76) SHA1(d2abe00eb2051ec7cb9423cdb8b52e91f7e2d416), ROM_BIOS(0))
	// 1: BIOS-String: 40-0100-001223-00101111-04093-UMC491f-0 / UCV-G
	ROM_SYSTEM_BIOS(1, "ucvg", "UCV-G")
	ROMX_LOAD( "bioteq.rom", 0x10000, 0x10000, CRC(93321e89) SHA1(450e35787607a4b6aecd3159d6c0599a03cd42b1), ROM_BIOS(1))
ROM_END

// Mitac PWA-IH4077C - Chipset: UMC UM82C491F + UM82c493F - CPU: Socket 3, solder pads for PQFP CPU - RAM: 4xSIMM30, Cache: 9xUM61256AK-15 - OSC: 14.31818
ROM_START( pwaih4077c )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS String: 04/13/94-UMC-491-2C4X2m31-00 - UMC491F 80486 BIOS...1.01.00 (3371-3372) - BIOS label: IH4077CN BIOS R1.01.00
	// BIOS Version: Award 4.50G 04/13/94 - Keyboard BIOS: VIA VT82C42N
	ROM_SYSTEM_BIOS(0, "012694", "01/26/94")
	ROMX_LOAD( "ih4077c-1.00.00.bin", 0x10000, 0x10000, CRC(e75cca73) SHA1(4f27d16f4f8fce9d3410821ec62780f7df669776), ROM_BIOS(0))
	// 1: BIOS-String: 04/13/94-UMC-491-2C4X2m31-00 - UMC491F 80486 BIOS...1.01.00 (3371-3372)
	ROM_SYSTEM_BIOS(1, "041394", "04/13/94")
	ROMX_LOAD( "4077c101.bio", 0x10000, 0x10000, CRC(b6a27c48) SHA1(18ee3b2fc4897cbaafc0e0298938ba58a3a7f84c), ROM_BIOS(1))
ROM_END


// ***** motherboards using the Unichip U4800 chipset

// Gemlight GMB-486UNP v2.1 - Chipset: Unichip U4800-VLX, SIS85C206 - CPU: solder pads for 486FQFP, i486DX2-66 - RAM: 8xSIMM30, Cache: 8xUM61256AK-15
// BIOS: AMI 486DX ISA BIOS - ISA16: 4, ISA16/VLB: 3
ROM_START( gmb486unp ) // BIOS String: 40-0405-428036-00101111-080893-U4800-VLX-H / screen remains blank
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "gmb-486unp.bin", 0x10000, 0x10000, CRC(17d770c7) SHA1(8655610ceaf7bd9c17d7c0a550805ae55f128660))
ROM_END

// UNICHIP 486 WB 4407 REV 1.0 - Chipset: KS83C206Q UNICHIP U4800-VLX - BIOS: AMI 486 ISA BIOS AA6562949, 28pin - Keyboard-BIOS: AMI 2050778
// BIOS-String: 40-0200-001107-0010111-111192-U4800VLX-0 / 4407 UNICHIP BIOS VER 1.0 - OSC: 14.31818 - ISA16: 4, ISA16/VL: 3
ROM_START( uniwb4407 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "unichip_486_wb_4407.bin", 0x10000, 0x10000, CRC(91237686) SHA1(7db14451cc3e00a2273a453152a817bccbdfb10e))
ROM_END

ROM_START( uni4800 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// BIOS-String: 40-05TB-006257-00101111-060692-UNI4800-0
	ROM_LOAD( "uni4800.ami", 0x10000, 0x10000, CRC(555c4c34) SHA1(0a907d7a0ec16e9369d77a1ac1afb0e69e3d6c1a))
ROM_END


// ***** 486 motherboards using the VIA VT82C495 VT82C481 chipset

// FIC 4386-VC-V - CPU: 486 - Chipset: VIA VT82C495 VT82C481 - ISA8: 2, ISA16: 3, ISA16/VL: 2 - OSC: 33.333MHz - BIOS: AMI 486DX ISA BIOS AA6387315 (28pin) -
// BIOS-String: X0-0100-001121-00101111-021993-VIA-0 / Version 1.02 - Keyboard-BIOS: Lance LT38C41
ROM_START( fic4386vcv )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-4386-vc.bin", 0x10000, 0x10000, CRC(659210c2) SHA1(a730a547f3af215459632160fa670fde7e9c4f9a))
ROM_END

// HIGHSCREEN 486 Universal Board C82C33-A VIA4386-VIO - Chipset: VIA VT82C495 VT82C481, Winbond WB3757F - CPU: AM486DX2-66
// BIOS: Award F0599630 - Keyboard BIOS: AMI 1131 KEYBOARD BIOS PLUS - BIOS-String: Award Modular BIOS v4.20 / Version 1.143K
// On board: IDE, Floppy, 2xser, par, game - OSC: 32.0000MHz - ISA16: 6
ROM_START( via4386vio ) // probably a FIC board - KEYBOARD ERROR OR NO KEYBOARD PRESENT
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-highscreen.bin", 0x10000, 0x10000, CRC(059b6e51) SHA1(f8ede823e41cfa6f72bd9717ec75419079f9c40b))
ROM_END

// FIC 4386-VC-HD - Chipset: VIA VT82C481, VT82C495 - this board can take either 386 or 486 CPUs
// Keyboard-BIOS: Lance LT38C41 - CPU: AMD AMD386DX-40, FPU: IIT 3C87-40 - ISA16: 6
ROM_START( fic4386vchd )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS: AMI; Version 1.04; 06/06/92 - BIOS-String: X0-0100-001121-00101111-021993-VIA-0 / Version 1.04
	ROM_SYSTEM_BIOS(0, "ami104", "AMI V1.04")
	ROMX_LOAD( "3vim001.bin", 0x10000, 0x10000, CRC(668d8cab) SHA1(409b81e33ca07b0a9724dbb6ca395a3a0887aa02), ROM_BIOS(0))
	// 1: BIOS: Award F0111730 v1.15K 03/12/93-VENUS-VIA - BIOS-String: Award Modular BIOS v4.20 / Version 1.15K
	ROM_SYSTEM_BIOS(1, "awav115k", "Award V1.15k") // KEYBOARD ERROR OR NO KEYBOARD PRESENT
	ROMX_LOAD( "4386-vc-hd v1.15k.bin", 0x10000, 0x10000, CRC(acc5db45) SHA1(cb93322735e96614d3c54fbfcd4291ff1b3ca57c), ROM_BIOS(1))
	// 2: AWARD v4.20 F0166061 (28pin) - Keyboard-BIOS: Lance LT38C41 - CPU: 486 - BIOS-String
	ROM_SYSTEM_BIOS(2, "awav110k", "Award V1.10K") // KEYBOARD ERROR OR NO KEYBOARD PRESENT
	ROMX_LOAD("486-4386-vc-hd.bin", 0x10000, 0x10000, CRC(a32d30fc) SHA1(815a63e624b3145d9955aa3ce8c4c1e34fb438bb), ROM_BIOS(2))
	// 3: ID: Peacock 4386-VCHD - Chipset: VIA VT82C481, VT82C495 - CPU: AMD Am386DX-40, socket for 486 provided
	// RAM: 8xSIMM30, Cache: 10x28pin sockets - BIOS: Award 386DX F0121091 PEA 2_0 - ISA16: 6 - OSC: 80.0000MHz
	ROM_SYSTEM_BIOS(3, "pea20", "Pea 2_0") // BIOS-String: Award Modular BIOS V4.20 / 4386 BIOS Ver. 2.0 01.04.93 - KEYBOARD ERROR OR NO KEYBOARD PRESENT
	ROMX_LOAD("386-peacock-4386-vchd.bin", 0x10000, 0x10000, CRC(1cd08629) SHA1(9a2b359ade2e93ab1d164e3e4f2cb9e8604cd43d), ROM_BIOS(3))
ROM_END

// FIC 486-VC-HD - BIOS Version: AMI 05/05/1991 - Chipset: VIA VT82C495 VT82C481, DS1287/1187 - EPROM Label: 486EB
// Keyboard BIOS: Lance LT38C41 - CPU: Intel 80486DX-33, solder pads for 80486 - RAM: 8xSIMM30, Cache: 10x28pin DIP (4xKM68257BP-25, 1xMCM6206CP fitted)
// OSC: 33.333MHz - ISA16: 6
ROM_START( fic486vchd ) // BIOS ID String: 40-04C1-ZZ1124-00101111-050591-ET/486H-0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-vc-hd_doc11670.bin", 0x10000, 0x10000, CRC(607ebe18) SHA1(870080bb49bad42fb4433f9208c17ad1c7ee437d))
ROM_END


// ***** 486 motherboards using the VIA VT82C505 + VT82C496G + VT82C406MV chipset

// FIC 486-PIO-2 (4 ISA, 4 PCI)
// VIA VT82C505 (ISA/VL to PCI bridge) + VT82C496G (system chipset) + VT82C406MV (keyboard controller, RTC, CMOS), NS311/312 or NS332 I/O
ROM_START( ficpio2 )
	ROM_REGION32_LE(0x40000, "isa", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("ficpio2c1")
	// 0
	ROM_SYSTEM_BIOS(0, "ficpio2c7", "FIC 486-PIO-2 1.15C701") /* pnp, i/o core: NS 332, doesn't boot, requires cache emulation? */
	ROMX_LOAD( "115c701.awd",  0x020000, 0x20000, CRC(b0dd7975) SHA1(bfde13b0fbd141bc945d37d92faca9f4f59b716d), ROM_BIOS(0))
	// 1
	ROM_SYSTEM_BIOS(1, "ficpio2b7", "FIC 486-PIO-2 1.15B701") /* pnp, i/o core: NS 311/312, doesn't boot, requires cache emulation? */
	ROMX_LOAD( "115b701.awd",  0x020000, 0x20000, CRC(ac24abad) SHA1(01174d84ed32fb1d95cd632d09f773acb8666c83), ROM_BIOS(1))
	// 2: BIOS-String: 04/18/96-VT496G-2A4LF0IC-00 / Version 1.15C101
	ROM_SYSTEM_BIOS(2, "ficpio2c1", "FIC 486-PIO-2 1.15C101") /* non-pnp, i/o core: NS 332, working  */
	ROMX_LOAD( "115c101.awd",  0x020000, 0x20000, CRC(5fadde88) SHA1(eff79692c1ecf34b6ea3f02409d14ce1f5c51bf9), ROM_BIOS(2))
	// 3: BIOS-String: 04/18/96-VT496G-2A4LF0IC-00 / Version 1.15B101
	ROM_SYSTEM_BIOS(3, "ficpio2b1", "FIC 486-PIO-2 1.15B101") /* non-pnp, i/o core: NS 311/312, working  */
	ROMX_LOAD( "115b101.awd",  0x020000, 0x20000, CRC(ff69617d) SHA1(ecbfc7315dcf6bd3e5b59e3ae9258759f64fe7a0), ROM_BIOS(3))
	// 4: no display - CPU: Socket3 - On board: 2xser, par, 2xIDE, Floppy, par - BIOS: Award F4215801, 32pin - ISA16: 4, PCI: 4
	ROM_SYSTEM_BIOS(4, "ficpio2", "FIC 486-PIO-2 DOC 14580")
	ROMX_LOAD( "486-pio2.bin", 0x20000, 0x20000, CRC(4609945d) SHA1(7ad446bc3b27f3f636fb5884e58b055681f081eb), ROM_BIOS(4))
ROM_END

// FIC 486-VIP-IO2 (3 ISA, 4 PCI)
// VIA VT82C505 + VT82C496G + VT82C406MV
// Boot block - BIOS-String: 05/28/96-VT496G-2A4L6F0IC-00 / 1.164G701
ROM_START( ficvipio2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "1164g701.awd", 0x00000, 0x20000, CRC(7b762683) SHA1(84debce7239c8b1978246688ae538f7c4f519d13))
ROM_END


//**************************************************************************
//  80486 Desktop
//**************************************************************************

// NEC APC IV aka - available in three different packages: Wide desktop: Year: 1986 / Chipset: CHIPS P82C201, P82A204 and P82A205
// Portable: white/blue LCD
ROM_START( necapciv )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: Phoenix 80286 ROM BIOS Version 1.57 / APC IV / NEC Corporation
	// on board: 2xser, par, Floppy - see https://stason.org/TULARC/pc/motherboards/N/NEC-TECHNOLOGIES-INC-286-APC-IV-SERIES-G9YAN.html for settings
	ROM_SYSTEM_BIOS(0, "wide157", "Wide desktop V1.57")
	ROMX_LOAD( "yan7m06.bin", 0x10000, 0x8000, CRC(21deafcb) SHA1(477fb36d64a9a60f6dc572fef1095391f6da73b3), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "yan7k06.bin", 0x10001, 0x8000, CRC(97563bae) SHA1(81ea93e1cd55e284609fdff6574aa49b06cd8a7f), ROM_SKIP(1) | ROM_BIOS(0))
	// 1: Phoenix 80286 ROM BIOS Version 3.07 03 / NEC Corporation
	// Narrow desktop (APC IV Powermate 1): Available with EGA (LIAI852/EVC215-001 chip) or VGA and 8Mhz/640k version and a 10Mhz/1MB version. [Ctrl]+[Alt]+[-]
	// is the speed switch on the 10MHz version.
	// on board: ser, par, Floppy - see http://www.uncreativelabs.de/th99/m/M-O/31487.htm for settings
	ROM_SYSTEM_BIOS(1, "narrow", "Narrow desktop V3.07 03")
	ROMX_LOAD( "bbx10j02.bin", 0x10000, 0x8000, CRC(5bb8c773) SHA1(21df040a92b2ee17e83955776af4ab14350d5ffd), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "bbx10g02.bin", 0x10001, 0x8000, CRC(050159ef) SHA1(b1e627f5d5ef749c51597b7be75f56bb8ff4d8af), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

// NCR Class 3433 - CPU: 486SX/DX 25/33, coprocessor socket provided - Chipset: NCR WPD CLEMSON 006-2001325 CQO1842 9209N
// LSI LOGIC L1A5840 006-2000654B NAR 9212Delta WG35494 GERMANY, NCR 006-2001895 WPD FALCON E CQO 2291 9218N,
// WD58C65JM, VLSI 9210AV 211411 VGA8203C4570 NCR PB 006-2001329, Dallas DS1387
// OSC: 16.000000MHz, 1.8432MHz,  65.00000MHz, 25.175/26.322, 36.000000MHz, 50.000000MHz, 14.31818MHz, 66.66600MHz
// on board VGA: NCR 77C22 VGA-2 609-3400639 CQO1570 9210R, 8xMCM514245AJ70
// Slots: 4xMCA, 3x32-bit external memory card, 1xSCSI host adapter - ports: PS/2 mouse, keyboard, 2xserial, parallel, floppy
ROM_START( ncr3433 )
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	// 0: NCR ROM BIOS Version 82.01.00 (3433, 3434, 3432, 3437)
	ROM_SYSTEM_BIOS(0, "82.01.00", "82.01.00")
	ROMX_LOAD( "rbios206.bin", 0x00000, 0x20000, CRC(06e77547) SHA1(41c517b284ed6335024c2b8398504cf3eb8a09e1), ROM_BIOS(0) )
	// 1: NCR ROM BIOS Version 83.01.00 (3433, 3434, 3432, 3437)
	ROM_SYSTEM_BIOS(1, "83.01.00", "83.01.00")
	ROMX_LOAD( "rbios1.bin", 0x00000, 0x20000, CRC(cf793ce9) SHA1(69d531d14a86bdada8940ba2466515ebcd870d0d), ROM_BIOS(1) )

	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "i8742_150-0008390_vers_3.1.bin", 0x000, 0x800, CRC(1bf17f29) SHA1(031ea1bb40756e3e5a1f01b01a53cc3a7c074338) )
ROM_END

// Amstrad PC9486 - Board Type - UX486VIO-A Rev. 1.0 - Chipset: UMC - UM82C481BF/UM82C482AF/UM82C206F - BIOS/Version: AMI 486DX ISA BIOS 1993 - AA9222968
// BIOS String: 40-0100-001131-00111111-111192-UMC480-0 / Amstrad PC9486 - CPU: 80486sx-25 in Socket 3, solder pads for 486 CPU - RAM: 4xSIMM30, 1xSIMM72, Cache: 64K/128K256K
// on board VGA: Cirrus Logic CL-GD5426-80QC-A, 2xKM416C256AJ-7 - Jumpers: Parallel out/bidirectional, VGA enable/disable, SIMM type, PQFP or socket,
// 20/25/33/40/50 MHz, Parity enable/disable, -  CPU type - on board: VGA, Floppy, IDE - slots: 1, for riser card
ROM_START( pc9486 )
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	ROM_LOAD ( "9486_saverom.bin", 0x00000, 0x20000, CRC(cbc35a4e) SHA1(dfa614c8255a1407c9850fa4ff99a6b2a52e1a4f) )
ROM_END

//**************************************************************************
//  80486 Laptop/Notebook
//**************************************************************************

// Siemens-Nixdorf PCD-4NL 486 subnotebook
// PhoenixBIOS(TM) A486 Version 1.03
// complains about "Pointer device failure" and "Memory failure at 00100000, read AA55 expecting 002C
ROM_START( pcd4nl )
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	ROM_LOAD( "pcd4nl.bin", 0x00000, 0x20000, CRC(8adb4900) SHA1(a01c665fed769ff815bc2e5ae30901f7e12d721b) )
ROM_END

// Siemens-Nixdorf PCD-4ND 486 notebook - display remains blank
// Graphics chip: WDC WD90C24A-ZZ on VESA Local Bus, 4MB on mainboard, 4MB/8MB/16MB on CF card like RAM modules
// CPU: Intel 486 SX, 486DX2, 486DX4-75 or -100,  128KB Flash-Eprom for system and video BIOS, ESS688 soundchip
ROM_START( pcd4nd )
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	ROM_SYSTEM_BIOS(0, "pcd4ndno1", "pcd4ndno1")
	ROMX_LOAD( "bf3m51.bin", 0x00000, 0x20000, CRC(6a2f90dd) SHA1(75704a83976e4bb02a028e761d01bd053cc0d4e7), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "pcd4ndno2", "pcd4ndno2")
	ROMX_LOAD( "bf3q42.bin", 0x00000, 0x20000, CRC(fa81cf6e) SHA1(91313a6856ca22f40710a6c9c8a65f8e340784ab), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "pcd4ndno3", "pcd4ndno3")
	ROMX_LOAD( "pcd-4nd_flash_28010.bin", 0x00000, 0x20000, CRC(53c0beea) SHA1(bfa17947529c51a8c9315884e156c01ddd23c0d8), ROM_BIOS(2) )
ROM_END

// LION 3500C notebook
ROM_START( lion3500 )
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	ROM_LOAD( "lion3500.bin", 0x00000, 0x20000, CRC(fc409ac3) SHA1(9a7aa08b981dedffff31fda5d3496469ae2ec3a5) )
ROM_END

// Highscreen 486-25 aka Midwest Micro Elite TS34T-25 notebook
// integrated trackball - CPU: i486sx-25 - Chipset: Chips F82C721, Intel ?80C51SLBG, MCCS1468188F, AvaSem AV9129-08CW28, ACC Micro 2046, LT1137CS
// Video: CL-GD6410-320C-A - HD: Maxtor 2585AT
ROM_START( ts34t25 )// blank display
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	ROM_LOAD( "p101a002_sys_bios_62fc.u24", 0x00000, 0x20000, CRC(2ce568bc) SHA1(84dc595abf0bf1948a6479afdea4a169f40e3b1b))

	ROM_REGION( 0x8000, "pmu", 0 ) // rom contains "PMU" string
	ROM_LOAD( "s34t0003_51slbios_019f.u31", 0x00000, 0x8000, CRC(40467716) SHA1(f976f2ce13eb22e0ed164d31d6382eda489545c1))
ROM_END

// Highscreen Colani Blue Note
// Chipset: Cirrus Logic CL-GD6235-65QC-A, CL-PD6720-QC-A ETEQ ET6000 A, Appian ADI2, SMC FDC37C651 QF P, DIA UA0300-QA, Chips F82C206J
// Video: Dual Scan Color LCD (monochrome available) - RAM: 4MB on board, RAM expansion board - on board: trackball, ser, par, PS2, VGA
// mass storage: Floppy, IDE - expansion: 2xPCMCIA, expansion bus
ROM_START( bluenote )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "7500d_rev26_121593.bin", 0x00000, 0x20000, CRC(d564f855) SHA1(181e4097c3b4ca2e8e79f1732d4aef9edd5b4586))
ROM_END

// Siemens PG-750 486 EISA
// blank screen, beeps
ROM_START( pg750eisa )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "pg_pg750_486_eisa.bin", 0x10000, 0x10000, CRC(2e6149a9) SHA1(9fcf29a6169efa1359c7c3eff09326dd3e4001dc))
ROM_END

} // anonymous namespace


/***************************************************************************
  Game driver(s)
***************************************************************************/

//    YEAR  NAME       PARENT   COMPAT   MACHINE    INPUT  CLASS         INIT            COMPANY        FULLNAME                FLAGS
COMP( 198?, asi100b0,  ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "<unknown>",    "ASI 100B0, identified as HAM 12 TI 286 Motherboard ZERO WAIT", MACHINE_NOT_WORKING )
COMP( 1987, at,        ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "<generic>",   "PC/AT (6 MHz, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1987, ataripc4,  ibm5170, 0,       neat,      0,     at_state,     init_at,        "Atari", "PC4", MACHINE_NOT_WORKING )
COMP( 1989, atariabc286,ibm5170,0,       neat,      0,     at_state,     init_at,        "Atari", "ABC-286/30", MACHINE_NOT_WORKING )
COMP( 1987, atturbo,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "<generic>",   "PC/AT Turbo (12 MHz, MF2 Keyboard)" , MACHINE_NOT_WORKING )
COMP( 198?, aubam12s2, ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "AUVA COMPUTER, INC.", "BAM/12-S2", MACHINE_NOT_WORKING )
COMP( 198?, bam16a0,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "AUVA", "VIP-M21502A BAM16-A0", MACHINE_NOT_WORKING )
COMP( 198?, bay1000c,  ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Mintek", "BAY-1000C V1.01", MACHINE_NOT_WORKING )
COMP( 199?, bi025c,    ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "<unknown>",   "BI-025C HT-12 286 (HT12/A chipset)", MACHINE_NOT_WORKING )
COMP( 1990, c286lt,    ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Commodore Business Machines",  "Laptop C286LT", MACHINE_NOT_WORKING )
COMP( 199?, cdtekg2,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "CDTEK", "286 mainboard with Headland G2 chipset", MACHINE_NOT_WORKING )
COMP( 1990, cl28612s,  ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Wearnes", "CL286-12/16S (CL286-12S and CL286-16S)", MACHINE_NOT_WORKING )
COMP( 198?, cmpa286,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "CMP enterprise CO.LTD.", "286 motherboard", MACHINE_NOT_WORKING )
COMP( 1987, comportii ,ibm5170, 0,       comportii, 0,     at_state,     init_at,        "Compaq",      "Portable II", MACHINE_NOT_WORKING )
COMP( 1987, comportiii,ibm5170, 0,       comportiii,0,     at_state,     init_at,        "Compaq",      "Portable III", MACHINE_NOT_WORKING )
COMP( 1988, comslt286, ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Compaq",      "SLT/286", MACHINE_NOT_WORKING )
COMP( 199?, csl286,    ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Commodore Business Machines",  "SL 286-16", MACHINE_NOT_WORKING )
COMP( 1988, dsys200,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Dell Computer Corporation",    "System 200", MACHINE_NOT_WORKING )
COMP( 1989, ec1842,    ibm5150, 0,       ec1842,    0,     at_state,     init_at,        "<unknown>",   "EC-1842", MACHINE_NOT_WORKING )
COMP( 1993, ec1849,    ibm5170, 0,       ec1842,    0,     at_state,     init_at,        "<unknown>",   "EC-1849", MACHINE_NOT_WORKING )
COMP( 198?, elanht286, ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "Leanord SA", "Elan High Tech 286", MACHINE_NOT_WORKING )
COMP( 198?, elt286b,   ibm5170, 0,       neat,      0,     at_state,     init_at,        "Chaintech", "ELT-286B-160B(E)", MACHINE_NOT_WORKING )
COMP( 198?, epsax,     ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "Epson",       "PC AX", MACHINE_NOT_WORKING )
COMP( 198?, epsax2e,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Epson",       "PC AX2e", MACHINE_NOT_WORKING )
COMP( 1989, euroat,    ibm5170, 0,       euroat,    0,     at_state,     init_at,        "Schneider Rundfunkwerke AG", "Euro AT", MACHINE_NOT_WORKING )
COMP( 198?, ev1806,    ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "Everex Systems", "EV-1806", MACHINE_NOT_WORKING ) // continuous beeps (RAM not detected?)
COMP( 198?, ev1815,    ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "Everex Systems", "EV-1815", MACHINE_NOT_WORKING ) // continuous beeps (RAM not detected?)
COMP( 1986, ews286,    ibm5170, 0,       ews286,    0,     at_state,     init_at,        "Ericsson",    "Ericsson WS286", MACHINE_NOT_WORKING )
COMP( 199?, headg2,    ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "<unknown>", "286 motherboards with Headland G2 chipset", MACHINE_NOT_WORKING )
COMP( 19??, ht12a,     ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "<unknown>", "unknown 286 AT clones (HT12/A chipset)", MACHINE_NOT_WORKING )
COMP( 1985, ibm5162,   ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "International Business Machines",  "PC/XT-286 5162", MACHINE_NOT_WORKING )
COMP( 1984, ibm5170,   0,       ibm5150, ibm5170,   0,     at_state,     init_at,        "International Business Machines",  "PC/AT 5170", MACHINE_NOT_WORKING )
COMP( 1985, ibm5170a,  ibm5170, 0,       ibm5170a,  0,     at_state,     init_at,        "International Business Machines",  "PC/AT 5170 8MHz", MACHINE_NOT_WORKING )
COMP( 1989, ibm2011,   ibm5170, 0,       ibmps1,    0,     at_vrom_fix_state, init_at,   "International Business Machines",  "PS/1 2011", MACHINE_NOT_WORKING )
COMP( 1989, ibm2011rd, ibm5170, 0,       ibmps1,    0,     at_vrom_fix_state, init_at,   "International Business Machines",  "PS/1 2011 (international models with ROM DOS)", MACHINE_NOT_WORKING )
COMP( 198?, icldrsm40, ibm5170, 0,       neat,      0,     at_state,     init_at,        "ICL", "DRS M40", MACHINE_NOT_WORKING )
COMP( 1985, k286i,     ibm5170, 0,       k286i,     0,     at_state,     init_at,        "Kaypro",      "286i", MACHINE_NOT_WORKING )
COMP( 199?, kma202f,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "<unknown>",   "KMA-202F-12R (Winbond chipset)", MACHINE_NOT_WORKING )
COMP( 19??, kt216wb5,  ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "KT Technology", "KT216WB5-HI Rev.2", MACHINE_NOT_WORKING )
COMP( 198?, lm103s,    ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "<unknown>",   "LM-103S", MACHINE_NOT_WORKING )
COMP( 198?, m250e,     ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Olivetti",    "M250E", MACHINE_NOT_WORKING )
COMP( 1987, m290,      ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Olivetti",    "M290", MACHINE_NOT_WORKING )
COMP( 198?, magb233,   ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "Magitronic Technology", "Magitronic B233", MACHINE_NOT_WORKING )
COMP( 198?, magb236,   ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "Magitronic Technology", "Magitronic B236", MACHINE_NOT_WORKING )
COMP( 19??, mat286,    ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "<unknown>",   "MAT286 Rev.D", MACHINE_NOT_WORKING )
COMP( 199?, mb1212c,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Biostar",     "MB-1212C", MACHINE_NOT_WORKING )
COMP( 199?, mba009,    ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "<unknown>", "HLB-286 MBA-009", MACHINE_NOT_WORKING )
COMP( 199?, micral45,  ibm5170, 0,       micral45,  0,     at_state,     init_at,        "Bull", "Micral 45", MACHINE_NOT_WORKING )
COMP( 199?, minisys2k, ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "SIIG", "MiniSys 2000", MACHINE_NOT_WORKING )
COMP( 198?, mkp286,    ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Morse", "KP-286", MACHINE_NOT_WORKING )
COMP( 1987, n8810m15,  ibm5170, 0,       n8810m15,  0,     at_state,     init_at,        "Nixdorf Computer AG", "8810 M15", MACHINE_NOT_WORKING )
COMP( 1990, n8810m16c, ibm5170, 0,       n8810m15,  0,     at_state,     init_at,        "Nixdorf Computer AG", "8810 M16 CGA version", MACHINE_NOT_WORKING )
COMP( 1990, n8810m30,  ibm5170, 0,       neat,      0,     at_state,     init_at,        "Nixdorf Computer AG", "8810 M30", MACHINE_NOT_WORKING )
COMP( 1986, n8810m55,  ibm5170, 0,       n8810m55,  0,     at_state,     init_at,        "Nixdorf Computer AG", "8810 M55", MACHINE_NOT_WORKING )
COMP( 1990, n8810m16v, ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Nixdorf Computer AG", "8810 M16 VGA version", MACHINE_NOT_WORKING )
COMP( 199?, ncr3302,   ibm5170, 0,       neat,      0,     at_state,     init_at,        "NCR", "Class 3302 Model 0110", MACHINE_NOT_WORKING )
COMP( 1986, ncrpc8,    ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "NCR",         "PC-8", MACHINE_NOT_WORKING )
COMP( 1989, neat,      ibm5170, 0,       neat,      0,     at_state,     init_at,        "<generic>",   "NEAT (12 MHz, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1986, necapciv,  ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "NEC", "APC IV", MACHINE_NOT_WORKING )
//COMP( 1988, nws286,    ibm5170,  0,      ews286,    0,     at_state,     at,        "Nokia Data",  "Nokia Data WS286", MACHINE_NOT_WORKING )
COMP( 198?, o286foxii, ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Octek",       "Fox II", MACHINE_NOT_WORKING )
COMP( 1990, ocfoxm,    ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Octek", "Fox M 286", MACHINE_NOT_WORKING )
COMP( 199?, octekg2,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Octek", "286 motherboard with Headland G2 chipset", MACHINE_NOT_WORKING )
COMP( 198?, ocxt286,   ibm5170, 0,       ibm5170,   0,     at_state,     init_at,        "Octek", "XT-286 motherboard", MACHINE_NOT_WORKING )
COMP( 199?, olim203,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Olivetti", "M203 motherboard", MACHINE_NOT_WORKING )
COMP( 1988, pc30iii,   ibm5170, 0,       pc30iii,   0,     at_state,     init_at,        "Commodore Business Machines",  "PC 30-III", MACHINE_NOT_WORKING )
COMP( 1988, pc40iii,   ibm5170, 0,       pc40iii,   0,     at_state,     init_at,        "Commodore Business Machines",  "PC 40-III", MACHINE_NOT_WORKING )
COMP( 198?, pc45iii,   ibm5170, 0,       pc40iii,   0,     at_state,     init_at,        "Commodore Business Machines",  "PC 45-III", MACHINE_NOT_WORKING )
COMP( 198?, pccm205,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "PC-Chips", "M205", MACHINE_NOT_WORKING )
COMP( 198?, pccm209,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "PC-Chips", "M209", MACHINE_NOT_WORKING )
COMP( 198?, pccm216,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "PC-Chips", "M216", MACHINE_NOT_WORKING )
COMP( 1986, pcd2,      ibm5170, 0,       ibm5170,   0,     at_state,     init_at,        "Siemens",     "PCD-2", MACHINE_NOT_WORKING )
COMP( 1991, pcd204,    ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Philips",     "PCD204 (PCD200 series)", MACHINE_NOT_WORKING )
COMP( 198?, pcd2m,     ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Siemens",     "PCD-2M", MACHINE_NOT_WORKING )
COMP( 198?, peas286,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Peacock Computer", "S-286 Rev A", MACHINE_NOT_WORKING )
COMP( 1990, profpc33,  ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Profex", "PC 33", MACHINE_NOT_WORKING )
COMP( 198?, prolite286,ibm5170, 0,       neat,      0,     at_state,     init_at,        "CAF", "Prolite 286/16", MACHINE_NOT_WORKING )
COMP( 198?, pwb7270e,  ibm5170, 0,       neat,      0,     at_state,     init_at,        "Advanced Logic Research", "PWB 7270 REV E", MACHINE_NOT_WORKING )
COMP( 199?, samdm286,  ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Samsung", "Deskmaster 286-12", MACHINE_NOT_WORKING )
COMP( 199?, sarcpc,    ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "<unknown>",   "80286 Standard System (SARC RC2015 chipset)", MACHINE_NOT_WORKING )
COMP( 198?, snomi286,  ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Snobol", "Mini 286", MACHINE_NOT_WORKING )
COMP( 198?, suntac303, ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "<unknown>", "286 motherboards with Suntac ST62C303-A chipset", MACHINE_NOT_WORKING )
COMP( 199?, suntac6,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "<unknown>", "286 motherboards with 6-chip SUNTAC chipset", MACHINE_NOT_WORKING )
COMP( 198?, td60c,     ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "<unknown>",   "TD60C", MACHINE_NOT_WORKING )
COMP( 19??, toptek286, ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "Toptek Micro Computer", "286 Turbo", MACHINE_NOT_WORKING )
COMP( 198?, towerat2xx,ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Schneider Rundfunkwerke AG", "Tower AT 201, 202, 220, 240 and 260 (286,EGA)", MACHINE_NOT_WORKING )
COMP( 198?, olyport40, ibm5170, 0,       olyport40, 0,     at_state,     init_at,        "AEG Olympia", "Olyport 40-21", MACHINE_NOT_WORKING )
COMP( 199?, twinnet,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Twinhead", "Netstation PC", MACHINE_NOT_WORKING )
COMP( 198?, u3911v3,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Uniron", "U3911-V3", MACHINE_NOT_WORKING )
COMP( 198?, v286c,     ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Victor", "V286C", MACHINE_NOT_WORKING )
COMP( 198?, vlsi5,     ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "<unknown>", "286 motherboards with 5-chip VLSI chipset", MACHINE_NOT_WORKING )
COMP( 1981, wpc250,    ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Wang Laboratories, Inc.", "PC-250/16", MACHINE_NOT_WORKING )
COMP( 198?, wy220001,  ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "Wyse", "WYSEpc 286", MACHINE_NOT_WORKING )
COMP( 1989, xb42639,   ibm5170, 0,       xb42639,   0,     at_state,     init_at,        "Apricot",     "Apricot XEN-S (Venus I Motherboard 286)" , MACHINE_NOT_WORKING )
COMP( 1990, xb42639a,  ibm5170, 0,       xb42639,   0,     at_state,     init_at,        "Apricot",     "Apricot XEN-S (Venus II Motherboard 286)" , MACHINE_NOT_WORKING )
COMP( 198?, zdz248,    ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Zenith Data Systems", "Z-248", MACHINE_NOT_WORKING )
COMP( 199?, 386sxvhcom,ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "<unknown>",   "80386SX-VH-COM", MACHINE_NOT_WORKING )
COMP( 199?, 3siud,     ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "<unknown>",   "3SIUD-1.1", MACHINE_NOT_WORKING )
COMP( 199?, alaleolx,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Alaris RYC", "LEOPARD LX", MACHINE_NOT_WORKING )
COMP( 199?, alim1217,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "<unknown>",   "386sx motherboards using the ALi M1217 chipset", MACHINE_NOT_WORKING )
COMP( 199?, anch386s,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "ANIX",        "CH-386S-16/20/25G", MACHINE_NOT_WORKING )
COMP( 199?, asc486slc, ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Computechnik", "ASC486SLC", MACHINE_NOT_WORKING )
COMP( 1988, at386sx,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "<generic>",   "PC/AT 386SX (16 MHz, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1990, c386sx16,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Commodore Business Machines", "386SX-16", MACHINE_NOT_WORKING )
COMP( 1990, c386sx25,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Commodore Business Machines", "386SX-25", MACHINE_NOT_WORKING )
COMP( 1991, c386sxlt,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Commodore Business Machines",  "Laptop C386SX-LT", MACHINE_NOT_WORKING )
COMP( 1988, ct386sx,   ibm5170, 0,       ct386sx,   0,     at_state,     init_at,        "<generic>",   "NEAT 386SX (16 MHz, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1993, cxsxd,     ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "CX Technology", "CX SXD", MACHINE_NOT_WORKING )
COMP( 199?, dfi386sx,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Diamond Flower International", "386SX-16/20CN Rev 1.0", MACHINE_NOT_WORKING )
COMP( 199?, dvbslan,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Datavan", "Book-Size LAN station", MACHINE_NOT_WORKING )
COMP( 199?, ecs8517,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Elitegroup", "ECS 8517 v3.3", MACHINE_NOT_WORKING )
COMP( 199?, elt386sx,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Elitegroup",  "ELT-386SX-160BE", MACHINE_NOT_WORKING )
COMP( 198?, eltp9,     ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Chaintech", "ELT-P9 / most likely ELT-386SX-160D", MACHINE_NOT_WORKING )
COMP( 198?, epsax3,    ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Epson",       "PC AX3", MACHINE_NOT_WORKING )
COMP( 19??, ht18c,     ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "<unknown>",   "unknown 386sx AT clones (HT18/C chipset)", MACHINE_NOT_WORKING )
COMP( 199?, ibm2121,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "International Business Machines",  "PS/1 2121", MACHINE_NOT_WORKING )
COMP( 199?, ibm2121rd, ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "International Business Machines",  "PS/1 2121 (international models with ROM DOS)", MACHINE_NOT_WORKING )
COMP( 199?, ibm2123,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "International Business Machines",  "PS/1 2123", MACHINE_NOT_WORKING )
COMP( 199?, ilm396b,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "ILON USA, INC.", "M-396B", MACHINE_NOT_WORKING )
COMP( 198?, ktx20t02,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Quadtel",     "QTC-SXM KT X20T02/HI Rev.3", MACHINE_NOT_WORKING )
COMP( 199?, mbc18nb,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Sanyo",       "MBC-18NB", MACHINE_NOT_WORKING )
COMP( 1992, mbc28,     ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Sanyo",       "MBC-28", MACHINE_NOT_WORKING ) // Complains about missing mouse hardware
COMP( 1993, megapc,    0,       0,       megapc,    0,     megapc_state, init_megapc,    "Amstrad plc", "MegaPC", MACHINE_NOT_WORKING )
COMP( 199?, mmbo4088,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "<unknown>",   "486MMBO4088 (TI TX486SLC/E)", MACHINE_NOT_WORKING )
COMP( 199?, mokp386sx, ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Morse",       "KP 386SX V2.21", MACHINE_NOT_WORKING )
COMP( 199?, ocpanii,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Octek",       "Panther II", MACHINE_NOT_WORKING )
COMP( 199?, op82c283,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "<unknown>",   "386sx motherboards using the OPTi 82C283 chipset", MACHINE_NOT_WORKING )
COMP( 199?, op82c291,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "<unknown>",   "386sx motherboards using the OPTi 82C291 chipset", MACHINE_NOT_WORKING )
COMP( 199?, p386sx25pw,ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Prolink",  "P386SX-25PW VER:2.00", MACHINE_NOT_WORKING )
COMP( 198?, pc50ii,    ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Commodore Business Machines",  "PC 50-II", MACHINE_NOT_WORKING )
COMP( 198?, pcb303,    ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Packard Bell", "PCB-303 Rev.01", MACHINE_NOT_WORKING )
COMP( 199?, pccm396f,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "PC-Chips",    "M396F", MACHINE_NOT_WORKING )
COMP( 199?, pcd3nsl,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Siemens-Nixdorf", "PCD-3Nsl Notebook Computer", MACHINE_NOT_WORKING )
COMP( 199?, pcd3nsx,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Siemens-Nixdorf", "PCD-3Nsx Notebook Computer", MACHINE_NOT_WORKING )
COMP( 199?, php3239,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Philips",     "P3239", MACHINE_NOT_WORKING )
COMP( 199?, ppm3333p,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "DTK Computer", "PPM-3333P", MACHINE_NOT_WORKING )
COMP( 199?, pt319a,    ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Pine",        "PT-319A", MACHINE_NOT_WORKING )
COMP( 199?, scamp386sx,ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "<unknown>",   "386sx motherboards using the SCAMPSX chipset", MACHINE_NOT_WORKING )
COMP( 199?, scsxaio,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Peacock",     "386sx Ver. 2.0 motherboard SCsxAIO", MACHINE_NOT_WORKING )
COMP( 199?, sh386sx20, ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Shuttle", "386SX REV 2.0A", MACHINE_NOT_WORKING )
COMP( 1991, t2000sx,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Toshiba",     "T2000SX", MACHINE_NOT_WORKING )
COMP( 199?, td70a,     ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "<unknown>",   "TD70A and TD70AN", MACHINE_NOT_WORKING )
COMP( 199?, td70n,     ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "<unknown>",   "TD70N", MACHINE_NOT_WORKING )
COMP( 199?, tot3200sxc,ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Toshiba",     "T3200SXC", MACHINE_NOT_WORKING )
COMP( 198?, tower386sx,ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Schneider Rundfunkwerke AG", "386SX System 40 (VGA)", MACHINE_NOT_WORKING )
COMP( 1992, walk386sx, ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Triumph-Adler", "Walkstation 386 SX", MACHINE_NOT_WORKING ) // screen remains blank
COMP( 198?, zeos386sx, ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "ZEOS", "386 SX-16", MACHINE_NOT_WORKING )
COMP( 199?, 386sc,     ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "386 SC Rev A2", MACHINE_NOT_WORKING )
COMP( 199?, 386sc2c,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "386 motherboard using the Symphony chipset", MACHINE_NOT_WORKING )
COMP( 199?, 386schg,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "386-SC-HG", MACHINE_NOT_WORKING )
COMP( 199?, 4nd04a,    ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "386-4N-D04A (UMC chipset)", MACHINE_NOT_WORKING )
COMP( 199?, al486vd,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Daewoo",      "AL486V-D Rev:1.1", MACHINE_NOT_WORKING )
COMP( 199?, alacou,    ibm5170, 0,       at386,     0,     at_state,     init_at,        "Alaris",      "Cougar", MACHINE_NOT_WORKING )
COMP( 199?, alim1419,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "386 motherboards using the ALi M1419 chipset", MACHINE_NOT_WORKING )
COMP( 199?, alim1429,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "386 motherboards using the ALi M1429 A1 and M1431 A2 chipset", MACHINE_NOT_WORKING )
COMP( 199?, amibaby,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "AMI",         "Mark V Baby Screamer", MACHINE_NOT_WORKING )
COMP( 1990, aplanst,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Apricot",     "Apricot LANstation (Krypton Motherboard)", MACHINE_NOT_WORKING )
COMP( 1990, aplannb,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Apricot",     "Apricot LANstation (Novell Remote Boot)", MACHINE_NOT_WORKING )
COMP( 1992, aplscar,   ibm5170, 0,       at486l,    0,     at_state,     init_at,        "Apricot",     "Apricot LS Pro (Caracal Motherboard)", MACHINE_NOT_WORKING )
COMP( 1987, apxeni,    ibm5170, 0,       at386,     0,     at_state,     init_at,        "Apricot",     "Apricot XEN-i 386 (Leopard Motherboard)" , MACHINE_NOT_WORKING )
COMP( 1988, at386,     ibm5170, 0,       at386,     0,     at_state,     init_at,        "<generic>",   "PC/AT 386 (12 MHz, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1988, ataripc5,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "Atari", "PC5", MACHINE_NOT_WORKING )
COMP( 199?, chn333sc,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "Chaintech",   "333SC", MACHINE_NOT_WORKING )
COMP( 199?, comt386,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Commodore Business Machines",  "Tower 386", MACHINE_NOT_WORKING )
COMP( 198?, cs8230,    ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "386 motherboards using the CS8230 chipset", MACHINE_NOT_WORKING )
COMP( 199?, ctcc386mx, ibm5170, 0,       at386,     0,     at_state,     init_at,        "Caching Tech Corporation", "C386MX", MACHINE_NOT_WORKING )
COMP( 198?, dfi386,    ibm5170, 0,       at386,     0,     at_state,     init_at,        "DFI", "386-20.REV0", MACHINE_NOT_WORKING )
COMP( 199?, dt386,     ibm5170, 0,       at386,     0,     at_state,     init_at,        "Commodore Business Machines", "DT386", MACHINE_NOT_WORKING )
COMP( 1988, ecs38632,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "Elitegroup Computer Systems",      "ECS-386/32", MACHINE_NOT_WORKING )
COMP( 1988, ecs386a,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Elitegroup Computer Systems",      "ECS-386A", MACHINE_NOT_WORKING )
COMP( 199?, ecsfx3000, ibm5170, 0,       at386,     0,     at_state,     init_at,        "Elitegroup Computer Systems", "FX-3000 REV1.0", MACHINE_NOT_WORKING )
COMP( 1992, ecsum386,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "Elitegroup Computer Systems",      "UM386 (Rev 1.1)", MACHINE_NOT_WORKING )
COMP( 199?, fic4386vchd,ibm5170,0,       at486,     0,     at_state,     init_at,        "First International Computer", "4386-VC-HD", MACHINE_NOT_WORKING )
COMP( 199?, frx386c,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Micro-Express Inc.", "Forex 386 Cache", MACHINE_NOT_WORKING )
COMP( 199?, frxc402,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>", "386 motherboards with a FOREX FRX46C402/FRX36C300/SIS85C206 chipset", MACHINE_NOT_WORKING )
COMP( 1991, fu340,     ibm5170, 0,       at386,     0,     at_state,     init_at,        "Abit",        "FU340", MACHINE_NOT_WORKING )
COMP( 199?, ges9051n,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "GES 9051N-386C VER -0.01", MACHINE_NOT_WORKING )
COMP( 198?, gs611606a, ibm5170, 0,       at386,     0,     at_state,     init_at,        "Goldstar",    "GOLDSTAR P/N 611-606A Rev 1.0A", MACHINE_NOT_WORKING )
COMP( 198?, hot304,    ibm5170, 0,       at386,     0,     at_state,     init_at,        "Shuttle Computer International", "HOT-304", MACHINE_NOT_WORKING )
COMP( 198?, hot307h,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Shuttle Computer International", "HOT-307H", MACHINE_NOT_WORKING )
COMP( 199?, isa386u30, ibm5170, 0,       at386,     0,     at_state,     init_at,        "Asus",        "ISA-386U30 REV.2.2", MACHINE_NOT_WORKING )
COMP( 1989, isa386c,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Asus",        "ISA-386C", MACHINE_NOT_WORKING )
COMP( 199?, kma300g,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Arche Technologies Inc.", "KMA-300G-25", MACHINE_NOT_WORKING )
COMP( 199?, mb133340,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "Biostar",     "MB-1340UCQ-B", MACHINE_NOT_WORKING )
COMP( 199?, mba032q,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "DTK", "MBA-032Q TK83305-4N-D-03", MACHINE_NOT_WORKING )
COMP( 199?, megapcpl,  megapc,  0,       megapcpl,  0,     megapc_state, init_megapcpl,  "Amstrad plc", "MegaPC Plus", MACHINE_NOT_WORKING )
COMP( 199?, megapcpla, megapc,  0,       megapcpla, 0,     at_vrom_fix_state, init_megapcpla, "Amstrad plc", "MegaPC Plus (WINBUS chipset)", MACHINE_NOT_WORKING )
COMP( 199?, mokp386,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Morse", "KP920121523 V2.20", MACHINE_NOT_WORKING )
COMP( 199?, mom3v3,    ibm5170, 0,       at386,     0,     at_state,     init_at,        "Morse", "M3 V3.0", MACHINE_NOT_WORKING )
COMP( 199?, mx83c305,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>", "386 motherboards using the MX83C305(A)(FC)/MX83C05(A)(FC) chipset", MACHINE_NOT_WORKING )
COMP( 1989, n8810m20,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "Nixdorf", "8810/20", MACHINE_NOT_WORKING )
COMP( 1992, ocjagii,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Octek",       "Jaguar II", MACHINE_NOT_WORKING )
COMP( 1992, ocjagv,    ibm5170, 0,       at386,     0,     at_state,     init_at,        "Octek",       "Jaguar V v1.4", MACHINE_NOT_WORKING )
COMP( 199?, op82c381,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "386 motherboards using the OPTi 82C381 chipset", MACHINE_NOT_WORKING )
COMP( 199?, op82c391,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "386 motherboards using the OPTi 82C391 chipset", MACHINE_NOT_WORKING )
COMP( 199?, opti495slc,ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>", "386 motherboards using a OPTi 82C495SLC chipset", MACHINE_NOT_WORKING )
COMP( 199?, opti495xlc,ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>", "386 motherboards using a OPTi 82C495XLC chipset", MACHINE_NOT_WORKING )
COMP( 1987, t4000,     ibm5170, 0,       at386,     0,     at_state,     init_at,        "Tandy Radio Shack", "Tandy 4000", MACHINE_NOT_WORKING )
COMP( 199?, op386wb,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "OPTi", "OPTi 386WB VER.1.0", MACHINE_NOT_WORKING )
COMP( 199?, p386dx40,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "Peacock", "P386DX-40", MACHINE_NOT_WORKING )
COMP( 1989, pc2386,    ibm5170, 0,       at386l,    0,     at_state,     init_at,        "Amstrad plc", "PC2386", MACHINE_NOT_WORKING )
COMP( 198?, pc60iii,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Commodore Business Machines",  "PC 60-III", MACHINE_NOT_WORKING )
COMP( 199?, pccm317,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "PC-Chips", "M317", MACHINE_NOT_WORKING )
COMP( 199?, pccm321,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "PC-Chips", "M321", MACHINE_NOT_WORKING )
COMP( 199?, pccm326,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "PC-Chips", "M326", MACHINE_NOT_WORKING )
COMP( 198?, pem2530,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "DTK", "PEM 2539", MACHINE_NOT_WORKING )
COMP( 199?, pg750,     ibm5170, 0,       pg750,     0,     at_state,     init_at,        "Siemens", "PG 750", MACHINE_NOT_WORKING )
COMP( 199?, pt581392,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "386 motherboard using the Forex FRX46C402 + FRX46C411 + SiS 85C206 chipset", MACHINE_NOT_WORKING )
COMP( 199?, qdu386dx,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>", "QD-U386DX VER 1.0", MACHINE_NOT_WORKING )
COMP( 1988, qi600,     ibm5170, 0,       at386,     0,     at_state,     init_at,        "Apricot",     "Apricot Qi 600 (Neptune Motherboard)", MACHINE_NOT_WORKING )
COMP( 199?, sisrabb,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "386 motherboards using the SiS Rabbit chipset", MACHINE_NOT_WORKING )
COMP( 199?, sm38640f,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "SM 386-40F", MACHINE_NOT_WORKING )
COMP( 19??, smih0107,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "Forex Computer Company", "unknown 386 AT clone with Forex chipset", MACHINE_NOT_WORKING )
COMP( 199?, sy012,     ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "SY-012 16/25 386MB VER: 5.2", MACHINE_NOT_WORKING )
COMP( 199?, sy019hi,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Soyo", "SY-019H and SY-019I", MACHINE_NOT_WORKING )
COMP( 199?, sybaby386, ibm5170, 0,       at386,     0,     at_state,     init_at,        "Soyo", "Baby AT 386", MACHINE_NOT_WORKING )
COMP( 199?, tam25p2,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "AUVA", "TAM/25-P2 M31720P", MACHINE_NOT_WORKING )
COMP( 199?, tam3340ma0,ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "TAM/33/40-MA0", MACHINE_NOT_WORKING )
COMP( 199?, um82c481af,ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "386 motherboards using the UMC UM82C481AF chipset", MACHINE_NOT_WORKING )
COMP( 199?, um82c491f, ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "386 motherboard using the UMC UM82C491F chipset", MACHINE_NOT_WORKING )
COMP( 199?, um82c493f, ibm5170, 0,       at386,     0,     at_state,     init_at,        "<unknown>",   "386 motherboards using the UMC UM82C491F + UM82C493F chipset or BIOTEQ equivalents", MACHINE_NOT_WORKING )
COMP( 199?, uni386w,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "UNICHIP", "386W 367C REV 1.0", MACHINE_NOT_WORKING )
COMP( 1992, walk386dx, ibm5170, 0,       at386,     0,     at_state,     init_at,        "Triumph-Adler", "Walkstation 386DX", MACHINE_NOT_WORKING ) // screen remains blank
COMP( 1988, xb42663,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Apricot",     "Apricot Qi 300 (Rev D,E & F Motherboard)", MACHINE_NOT_WORKING )
COMP( 1989, xb42664,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Apricot",     "Apricot XEN-S (Venus I Motherboard 386)" , MACHINE_NOT_WORKING )
COMP( 1990, xb42664a,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "Apricot",     "Apricot XEN-S (Venus II Motherboard 386)" , MACHINE_NOT_WORKING )
COMP( 199?, 486apio,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "EFA",   "486 APIO", MACHINE_NOT_WORKING )
COMP( 199?, 486ccv,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Diamond Flower, Inc. (DFI)", "486-CCV", MACHINE_NOT_WORKING )
COMP( 199?, 486igb21,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "<unknown>", "486IG-B-2-1", MACHINE_NOT_WORKING )
COMP( 199?, 486wb6a3,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Silicon Valley Computer, Inc.", "486WB6A3.B1", MACHINE_NOT_WORKING )
COMP( 199?, 4dmshl3g,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "EFA",   "4DMS HL3G-L4-VI", MACHINE_NOT_WORKING )
COMP( 199?, 4dmuhl3s,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "EFA",   "4DMU HL3S", MACHINE_NOT_WORKING )
COMP( 1992, a433cc,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "J-Bond",      "A433C-C/A450C-C", MACHINE_NOT_WORKING )
COMP( 1994, a486ap4,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Asus",        "PVI-486AP4", MACHINE_NOT_WORKING )
COMP( 199?, a486isa,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Asus",        "ISA-486", MACHINE_NOT_WORKING )
COMP( 199?, a486sio,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Asus",        "ISA-486SIO rev. 1.2", MACHINE_NOT_WORKING )
COMP( 1994, a486sp3g,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Asus",        "PCI/I-486SP3G", MACHINE_NOT_WORKING )
COMP( 199?, a486sv2,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Asus",        "ISA-486SV2", MACHINE_NOT_WORKING )
COMP( 1994, a486sv2g,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Asus",        "VL/I-486SV2G", MACHINE_NOT_WORKING )
COMP( 1994, a486sv1,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Asus",        "VL/EISA-486SV1", MACHINE_NOT_WORKING )
COMP( 199?, abae4,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "Abit", "486 EISA-AE4", MACHINE_NOT_WORKING )
COMP( 199?, abah4,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "Abit", "AB-AH4", MACHINE_NOT_WORKING )
COMP( 199?, abav4,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "Abit", "AB-AV4", MACHINE_NOT_WORKING )
COMP( 199?, abax4,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "Abit", "AB-AX4", MACHINE_NOT_WORKING )
COMP( 199?, abpb4,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "Abit", "AB-PB4", MACHINE_NOT_WORKING )
COMP( 199?, abpw4,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "Abit", "AB-PW4", MACHINE_NOT_WORKING )
COMP( 199?, alator2,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Alaris",      "Tornado 2", MACHINE_NOT_WORKING )
COMP( 199?, alim1489,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "<unknown>", "486 motherboards using the ALi 1487/1489 chipset", MACHINE_NOT_WORKING )
COMP( 199?, amient2,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "AMI",         "EISA Enterprise-II", MACHINE_NOT_WORKING )
COMP( 199?, amient3,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "AMI",         "EISA Enterprise-III", MACHINE_NOT_WORKING )
COMP( 199?, amient4,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "AMI",         "EISA Enterprise-IV", MACHINE_NOT_WORKING )
COMP( 199?, amisvpci2, ibm5170, 0,       at486,     0,     at_state,     init_at,        "AMI",         "Super Voyager PCI-II", MACHINE_NOT_WORKING )
COMP( 199?, amisvvlb,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "AMI",         "Super Voyager VLB", MACHINE_NOT_WORKING )
COMP( 199?, amisvvlb2, ibm5170, 0,       at486,     0,     at_state,     init_at,        "AMI",         "Super Voyager VLB-II", MACHINE_NOT_WORKING )
COMP( 199?, amisvvlb3, ibm5170, 0,       at486,     0,     at_state,     init_at,        "AMI",         "Super Voyager VLB-III", MACHINE_NOT_WORKING )
COMP( 199?, aovi15g,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Aopen", "VI15G", MACHINE_NOT_WORKING )
COMP( 1992, aplsbon,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot LS Pro (Bonsai Motherboard)", MACHINE_NOT_WORKING )
COMP( 1991, aprfte,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot FT//ex 486 (J3 Motherboard)", MACHINE_NOT_WORKING )
COMP( 1992, aprpand,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot FTs (Panther Rev F 1.02.26)", MACHINE_NOT_WORKING )
COMP( 1989, apvxft,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot VX FT server", MACHINE_NOT_WORKING )
COMP( 1993, apxena1,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot XEN PC (A1 Motherboard)", MACHINE_NOT_WORKING )
COMP( 1991, apxenls3,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot XEN-LS (Venus IV Motherboard)", MACHINE_NOT_WORKING )
COMP( 1993, apxenp2,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot XEN PC (P2 Motherboard)", MACHINE_NOT_WORKING )
COMP( 1993, apxlsam,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot XEN-LS II (Samurai Motherboard)", MACHINE_NOT_WORKING )
COMP( 199?, ar4glx3,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Addtech Research", "4GLX3 Green-B 4GPV3.1", MACHINE_NOT_WORKING )
COMP( 199?, as496,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "Arstoria",    "AS496", MACHINE_NOT_WORKING )
COMP( 1990, at486,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "<generic>",   "PC/AT 486 (25 MHz, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 199?, atc1415,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "A-Trend", "ATC-1415", MACHINE_NOT_WORKING )
COMP( 199?, bluenote,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Highscreen",  "Colani Blue Note", MACHINE_NOT_WORKING )
COMP( 199?, ch48633c,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Chicony",   "CH-486-33C", MACHINE_NOT_WORKING )
COMP( 199?, ch491e,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Chicony",   "CH-491E", MACHINE_NOT_WORKING )
COMP( 199?, ch4slez1,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Chaintech", "486SLE M106 4SLE-Z1", MACHINE_NOT_WORKING )
COMP( 199?, comt486,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Commodore Business Machines",  "Tower 486", MACHINE_NOT_WORKING )
COMP( 199?, dt486,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "Commodore Business Machines", "DT486", MACHINE_NOT_WORKING )
COMP( 199?, ec4913,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Elitegroup", "UC4913 REV:1.1", MACHINE_NOT_WORKING )
COMP( 199?, ec4915aio, ibm5170, 0,       at486,     0,     at_state,     init_at,        "Elitegroup", "UC4915 A AIO", MACHINE_NOT_WORKING )
COMP( 199?, ed486vl3h, ibm5170, 0,       at486,     0,     at_state,     init_at,        "Edom", "486VL3H", MACHINE_NOT_WORKING )
COMP( 199?, edmv035f,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Edom", "MV035F", MACHINE_NOT_WORKING )
COMP( 199?, exp4044,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "ExpertChip", "EXP4044", MACHINE_NOT_WORKING )
COMP( 199?, f4dxluc4,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "ADI", "F4DXL-UC4", MACHINE_NOT_WORKING )
COMP( 199?, fic486gvt, ibm5170, 0,       at486,     0,     at_state,     init_at,        "First International Computer", "486-GVT", MACHINE_NOT_WORKING )
COMP( 199?, fic4386vcv,ibm5170, 0,       at486,     0,     at_state,     init_at,        "First International Computer", "4386-VC-V", MACHINE_NOT_WORKING )
COMP( 199?, fic486kvd, ibm5170, 0,       at486,     0,     at_state,     init_at,        "First International Computer", "486 KVD", MACHINE_NOT_WORKING )
COMP( 199?, fic486vchd,ibm5170, 0,       at486,     0,     at_state,     init_at,        "First International Computer", "486-VC-HD", MACHINE_NOT_WORKING )
COMP( 199?, ficeli6ii, ibm5170, 0,       at486,     0,     at_state,     init_at,        "First International Computer", "FIC ELI6-II", MACHINE_NOT_WORKING )
COMP( 1994, ficgiovt2, ibm5170, 0,       at486,     0,     at_state,     init_at,        "First International Computer", "486-GIO-VT2", MACHINE_NOT_WORKING )
COMP( 1995, ficpio2,   ibm5170, 0,       ficpio2,   0,     at_state,     init_atpci,     "First International Computer", "486-PIO-2", MACHINE_NOT_WORKING )
COMP( 1994, ficvipio,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "First International Computer", "486-VIP-IO", MACHINE_NOT_WORKING )
COMP( 199?, ficvipio2, ibm5170, 0,       at486,     0,     at_state,     init_at,        "First International Computer", "486-VIP-IO2", MACHINE_NOT_WORKING )
COMP( 1991, ftsserv,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot FTs (Scorpion)", MACHINE_NOT_WORKING )
COMP( 199?, ga486am,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Gigabyte",    "GA-486AM/S", MACHINE_NOT_WORKING )
COMP( 199?, ga486vf,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Gigabyte",    "GA-486VF", MACHINE_NOT_WORKING )
COMP( 199?, ga486vs,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Gigabyte",    "GA-486VS", MACHINE_NOT_WORKING )
COMP( 199?, gc10a,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "Eagle", "EAGLEN486 GC10A", MACHINE_NOT_WORKING )
COMP( 199?, gete486vl, ibm5170, 0,       at486,     0,     at_state,     init_at,        "GENOA",       "TurboExpress 486 VL", MACHINE_NOT_WORKING )
COMP( 199?, gmb486sg,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Gemlight", "GMB-486SG v2.2", MACHINE_NOT_WORKING )
COMP( 199?, gmb486unp, ibm5170, 0,       at486,     0,     at_state,     init_at,        "Gemlight", "GMB-486UNP v2.1", MACHINE_NOT_WORKING )
COMP( 199?, hot409,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Shuttle Computer International", "HOT-409", MACHINE_NOT_WORKING )
COMP( 199?, hot419,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Shuttle Computer International", "HOT-419", MACHINE_NOT_WORKING )
COMP( 199?, hot433,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Shuttle Computer International", "HOT-433", MACHINE_NOT_WORKING )
COMP( 199?, ibm2133,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "International Business Machines",  "PS/1 2133", MACHINE_NOT_WORKING )
COMP( 199?, jwj403tg,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Jetway", "J-403TG", MACHINE_NOT_WORKING )
COMP( 1993, lion3500,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Lion",        "3500", MACHINE_NOT_WORKING )
COMP( 199?, lsucm486v30,ibm5170,0,       at486,     0,     at_state,     init_at,        "Lucky Star", "UCM-486V30", MACHINE_NOT_WORKING )
COMP( 199?, mb1433aeap,ibm5170, 0,       at486,     0,     at_state,     init_at,        "Biostar",     "MB-1433/50 AEA-P - V:1", MACHINE_NOT_WORKING )
COMP( 199?, mb1433ucv, ibm5170, 0,       at486,     0,     at_state,     init_at,        "Biostar",     "MB-1433UCV", MACHINE_NOT_WORKING )
COMP( 199?, mb1433uiv, ibm5170, 0,       at486,     0,     at_state,     init_at,        "Biostar",     "MB-1433UIV", MACHINE_NOT_WORKING )
COMP( 199?, mb4d33,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Aquarius System (ASI)", "MB-4D33/50NR", MACHINE_NOT_WORKING )
COMP( 199?, mb8433uud, ibm5170, 0,       at486,     0,     at_state,     init_at,        "Biostar",     "MB8433-UUD-A", MACHINE_NOT_WORKING ) // boots to Award BootBlock BIOS
COMP( 199?, mba029,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Mitac", "MBA-029", MACHINE_NOT_WORKING )
COMP( 199?, md4duvc,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Aquarius System (ASI)", "MD-4DUV VER:2.1", MACHINE_NOT_WORKING )
COMP( 199?, mijx30gp,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Micronics",   "JX30GP, Motherboard P/N: 09-00189-10 REV B1", MACHINE_NOT_WORKING )
COMP( 199?, ms4125,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "MSI",         "MS-4125", MACHINE_NOT_WORKING )
COMP( 199?, ms4132,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "MSI",         "MS-4132 G VER:1", MACHINE_NOT_WORKING )
COMP( 199?, ms4134,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "MSI",         "MS-4134", MACHINE_NOT_WORKING )
COMP( 199?, ms4138,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "MSI",         "MS-4138", MACHINE_NOT_WORKING )
COMP( 199?, ms4145,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "MSI",         "MS-4145", MACHINE_NOT_WORKING )
COMP( 199?, nat48pv,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "<unknown>", "NAT48PV-1.00 VL", MACHINE_NOT_WORKING )
COMP( 199?, ncr3433,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "NCR", "Class 3433", MACHINE_NOT_WORKING )
COMP( 199?, ochawk,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Octek",       "Hawk", MACHINE_NOT_WORKING )
COMP( 199?, ochipcom,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Octek",       "Hippo COM", MACHINE_NOT_WORKING )
COMP( 1994, ochipdca2, ibm5170, 0,       at486,     0,     at_state,     init_at,        "Octek",       "Hippo DCA2", MACHINE_NOT_WORKING )
COMP( 199?, ochipvlp,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Octek",       "Hippo VL+", MACHINE_NOT_WORKING )
COMP( 199?, op82c392,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "<unknown>", "486 motherboards using the OPTi OPTi 82C392, 82C493 chipset", MACHINE_NOT_WORKING )
COMP( 199?, pc70iii,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Commodore Business Machines",  "PC 70-III", MACHINE_NOT_WORKING )
COMP( 199?, pc9486,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Amstrad",     "PC9486", MACHINE_NOT_WORKING )
COMP( 199?, pccm912,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "PC-Chips", "M912", MACHINE_NOT_WORKING )
COMP( 199?, pccm915i,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "PC-Chips", "M915i", MACHINE_NOT_WORKING )
COMP( 199?, pccm919,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "PC-Chips", "M919", MACHINE_NOT_WORKING )
COMP( 1993, pcd4nd,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Siemens-Nixdorf", "PCD-4ND", MACHINE_NOT_WORKING )
COMP( 1995, pcd4nl,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Siemens-Nixdorf", "PCD-4NL", MACHINE_NOT_WORKING )
COMP( 199?, pcd4x,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "Siemens-Nixdorf", "PCD-4H, PCD-4M", MACHINE_NOT_WORKING )
COMP( 199?, pci48af,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "TMC Research Corporation", "PCI48AF", MACHINE_NOT_WORKING )
COMP( 199?, pck486dx,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Peacock",  "PCK 486 DX", MACHINE_NOT_WORKING )
COMP( 199?, pg750eisa, ibm5170, 0,       at486,     0,     at_state,     init_at,        "Siemens", "PG-750 486 EISA", MACHINE_NOT_WORKING )
COMP( 199?, pkm0038s,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "DTK", "PKM-0038S aka Gemlight GMB-486SG", MACHINE_NOT_WORKING )
COMP( 199?, pm486pu,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "PROTECH",  "PM486PU-S7", MACHINE_NOT_WORKING )
COMP( 199?, pt430,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "Pine Technology", "PT-430", MACHINE_NOT_WORKING )
COMP( 199?, pt432b,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Pine Technology", "PT-432b aka SR-M401-A", MACHINE_NOT_WORKING )
COMP( 199?, ptmb457,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "PowerTech", "MB457", MACHINE_NOT_WORKING )
COMP( 199?, pwaih4077c,ibm5170, 0,       at486,     0,     at_state,     init_at,        "Mitac", "PWA-IH4077C", MACHINE_NOT_WORKING )
COMP( 199?, pwaih4077d,ibm5170, 0,       at486,     0,     at_state,     init_at,        "Mitac", "PWA-IH4077D", MACHINE_NOT_WORKING )
COMP( 199?, px486p3,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "QDI", "PX486P3", MACHINE_NOT_WORKING )
COMP( 1990, qi900,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot Qi 900 (Scorpion Motherboard)", MACHINE_NOT_WORKING )
COMP( 199?, sis85c471, ibm5170, 0,       at486,     0,     at_state,     init_at,        "<unknown>", "486 motherboards using the SiS 85C471/85C407 chipset", MACHINE_NOT_WORKING )
COMP( 199?, sm48650usc,ibm5170, 0,       at486,     0,     at_state,     init_at,        "Vintage Sprite", "SM 486-50USC", MACHINE_NOT_WORKING )
COMP( 199?, so025d2,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "SOYO", "025D2", MACHINE_NOT_WORKING )
COMP( 199?, so025k2,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "SOYO", "025K2", MACHINE_NOT_WORKING )
COMP( 199?, so025r2,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "SOYO", "025R2", MACHINE_NOT_WORKING )
COMP( 199?, sto486wb,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "See-Thru", "Sto486Wb aka AUVA Cam-33-P2", MACHINE_NOT_WORKING )
COMP( 199?, td4ipaio,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "<unknown>", "TD-4IP-UMC-AIO", MACHINE_NOT_WORKING )
COMP( 199?, tmpat48pg4,ibm5170, 0,       at486,     0,     at_state,     init_at,        "TMC", "PAT48PG4", MACHINE_NOT_WORKING )
COMP( 199?, tmpat48av, ibm5170, 0,       at486,     0,     at_state,     init_at,        "TMC", "PAT48AV", MACHINE_NOT_WORKING )
COMP( 199?, ts34t25,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Highscreen",  "486-25", MACHINE_NOT_WORKING )
COMP( 199?, um486,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "Elitegroup", "UM486/UM486sx", MACHINE_NOT_WORKING )
COMP( 199?, um486v,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Elitegroup", "UM486V-AIO", MACHINE_NOT_WORKING )
COMP( 199?, um8810paio,ibm5170, 0,       at486,     0,     at_state,     init_at,        "Elitegroup", "UM8810 PAIO", MACHINE_NOT_WORKING )
COMP( 199?, um8886,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "<unknown>", "486 motherboards using the UMC UM8886/UM8881 chipset", MACHINE_NOT_WORKING )
COMP( 199?, um8498f,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "<unknown>", "486 motherboards using the UMC UM8498F, UM8496F chipset", MACHINE_NOT_WORKING )
COMP( 199?, uni4800,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "<unknown>", "486 motherboards using the UNI4800 chipset", MACHINE_NOT_WORKING )
COMP( 199?, uniwb4407, ibm5170, 0,       at486,     0,     at_state,     init_at,        "UNICHIP", "486 WB 4407 REV 1.0", MACHINE_NOT_WORKING )
COMP( 199?, v4p895p3,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "QDI", "V4P895P3/SMT V5.0", MACHINE_NOT_WORKING )
COMP( 199?, via4386vio,ibm5170, 0,       at486,     0,     at_state,     init_at,        "<unknown>", "Via 4386 VIO / Highscreen universal board", MACHINE_NOT_WORKING )
COMP( 199?, zi4dvs,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "ZIDA", "4DVS", MACHINE_NOT_WORKING )

