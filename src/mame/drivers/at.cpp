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
	void atvga(machine_config &config);
	void at386(machine_config &config);
	void m290(machine_config &config);
	void ncrpc8(machine_config &config);
	void n8810m15(machine_config &config);
	void n8810m55(machine_config &config);
	void ews286(machine_config &config);

	void init_at();
	void init_atpci();

protected:
	required_device<cpu_device> m_maincpu;
	required_device<at_mb_device> m_mb;
	required_device<ram_device> m_ram;
	DECLARE_READ16_MEMBER(ps1_unk_r);
	DECLARE_WRITE16_MEMBER(ps1_unk_w);
	DECLARE_READ8_MEMBER(ps1_portb_r);

	void init_at_common(int xmsbase);
	uint16_t m_ps1_reg[2];

	static void cfg_single_360K(device_t *device);
	static void cfg_single_1200K(device_t *device);
	void at16_io(address_map &map);
	void at16_map(address_map &map);
	void at16l_map(address_map &map);
	void at32_io(address_map &map);
	void at32_map(address_map &map);
	void at32l_map(address_map &map);
	void ficpio_io(address_map &map);
	void ficpio_map(address_map &map);
	void neat_io(address_map &map);
	void ps1_16_io(address_map &map);
};

class at_vrom_fix_state : public at_state
{
public:
	using at_state::at_state;

	void init_megapcpla();

	void ibmps1(machine_config &config);
	void megapcpla(machine_config &config);

protected:
	virtual void machine_start() override;
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

	DECLARE_READ16_MEMBER( wd7600_ior );
	DECLARE_WRITE16_MEMBER( wd7600_iow );
	DECLARE_WRITE_LINE_MEMBER( wd7600_hold );
	DECLARE_WRITE8_MEMBER( wd7600_tc ) { m_isabus->eop_w(offset, data); }
	DECLARE_WRITE_LINE_MEMBER( wd7600_spkr ) { m_speaker->level_w(state); }
	void megapc_io(address_map &map);
	void megapc_map(address_map &map);
	void megapcpl_io(address_map &map);
	void megapcpl_map(address_map &map);
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

READ16_MEMBER( at_state::ps1_unk_r )
{
	return m_ps1_reg[offset];
}

WRITE16_MEMBER( at_state::ps1_unk_w )
{
	if((offset == 0) && (data == 0x60))
		data = 0x68;

	COMBINE_DATA(&m_ps1_reg[offset]);
}

READ8_MEMBER( at_state::ps1_portb_r )
{
	uint8_t data = m_mb->portb_r(space, offset);
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

READ16_MEMBER( megapc_state::wd7600_ior )
{
	if (offset < 4)
		return m_isabus->dack_r(offset);
	else
		return m_isabus->dack16_r(offset);
}

WRITE16_MEMBER( megapc_state::wd7600_iow )
{
	if (offset < 4)
		m_isabus->dack_w(offset, data);
	else
		m_isabus->dack16_w(offset, data);
}

WRITE_LINE_MEMBER( megapc_state::wd7600_hold )
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

	/* MESS managed RAM */
	membank("bank10")->set_base(m_ram->pointer());

	if (m_ram->size() > xmsbase)
	{
		offs_t ram_limit = 0x100000 + m_ram->size() - xmsbase;
		space.install_read_bank(0x100000,  ram_limit - 1, "bank1");
		space.install_write_bank(0x100000,  ram_limit - 1, "bank1");
		membank("bank1")->set_base(m_ram->pointer() + xmsbase);
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
	space.install_read_bank(0xc0000, 0xcffff, "vrom_bank");
	membank("vrom_bank")->set_base(machine().root_device().memregion("bios")->base());
}

void at_state::cfg_single_1200K(device_t *device)
{
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:0")).set_default_option("525hd");
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:1")).set_default_option("");
}

void at_state::cfg_single_360K(device_t *device)
{
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:0")).set_default_option("525dd");
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:1")).set_default_option("");
}

static void pci_devices(device_slot_interface &device)
{
	device.option_add_internal("vt82c505", VT82C505);
}

void at_state::ibm5170(machine_config &config)
{
	/* basic machine hardware */
	i80286_cpu_device &maincpu(I80286(config, m_maincpu, 12_MHz_XTAL / 2 /*6000000*/));
	maincpu.set_addrmap(AS_PROGRAM, &at_state::at16_map);
	maincpu.set_addrmap(AS_IO, &at_state::at16_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));
	maincpu.shutdown_callback().set("mb", FUNC(at_mb_device::shutdown));

	AT_MB(config, m_mb, 0);
	config.m_minimum_quantum = attotime::from_hz(60);

	m_mb->at_softlists(config);

	// FIXME: determine ISA bus clock
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "ega", false);
	ISA16_SLOT(config, "isa2", 0, "mb:isabus", pc_isa16_cards, "fdc", false);
	ISA16_SLOT(config, "isa3", 0, "mb:isabus", pc_isa16_cards, "comat", false);
	ISA16_SLOT(config, "isa4", 0, "mb:isabus", pc_isa16_cards, "ide", false);
	PC_KBDC_SLOT(config, "kbd", pc_at_keyboards, STR_KBD_IBM_PC_AT_84).set_pc_kbdc_slot(subdevice("mb:pc_kbdc"));

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
	m_maincpu->set_clock(12000000);
}

void at_state::ibm5162(machine_config &config)
{
	ibm5170(config);
	m_maincpu->set_clock(6000000);
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
	subdevice<pc_kbdc_slot_device>("kbd")->set_default_option(STR_KBD_MICROSOFT_NATURAL);
}

void at_state::atvga(machine_config &config)
{
	ibm5170(config);
	m_maincpu->set_clock(12000000);
	subdevice<isa16_slot_device>("isa1")->set_default_option("svga_et4k");
	subdevice<pc_kbdc_slot_device>("kbd")->set_default_option(STR_KBD_MICROSOFT_NATURAL);
	ISA16_SLOT(config, "isa5", 0, "mb:isabus", pc_isa16_cards, nullptr, false); // FIXME: determine ISA bus clock
}

void at_state::neat(machine_config &config)
{
	atvga(config);
	m_maincpu->set_addrmap(AS_IO, &at_state::neat_io);

	ds12885_device &rtc(DS12885(config.replace(), "mb:rtc")); // TODO: move this into the cs8221
	rtc.irq().set("mb:pic8259_slave", FUNC(pic8259_device::ir0_w)); // this is in :mb
	rtc.set_century_index(0x32);

	CS8221(config, "cs8221", 0, "maincpu", "mb:isa", "bios");
}

void at_state::xb42639(machine_config &config)
{
	atvga(config);
	m_maincpu->set_clock(12500000);
}

void at_state::k286i(machine_config &config)
{
	ibm5162(config);
	subdevice<pc_kbdc_slot_device>("kbd")->set_default_option(STR_KBD_MICROSOFT_NATURAL);
	ISA16_SLOT(config, "isa5", 0, "mb:isabus", pc_isa16_cards, nullptr, false); // FIXME: determine ISA bus clock
	ISA16_SLOT(config, "isa6", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa7", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa8", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
}

void at_state::at386(machine_config &config)
{
	i386_device &maincpu(I386(config, m_maincpu, 12000000));
	maincpu.set_addrmap(AS_PROGRAM, &at_state::at32_map);
	maincpu.set_addrmap(AS_IO, &at_state::at32_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	AT_MB(config, m_mb, 0).at_softlists(config);

	config.m_minimum_quantum = attotime::from_hz(60);

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
	PC_KBDC_SLOT(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL).set_pc_kbdc_slot(subdevice("mb:pc_kbdc"));

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
	at386(config);
	i486_device &maincpu(I486(config.replace(), m_maincpu, 25000000));
	maincpu.set_addrmap(AS_PROGRAM, &at_state::at32_map);
	maincpu.set_addrmap(AS_IO, &at_state::at32_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));
}

void at_state::at486l(machine_config &config)
{
	at486(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &at_state::at32l_map);
}

void at_state::at386sx(machine_config &config)
{
	atvga(config);
	i386sx_device &maincpu(I386SX(config.replace(), m_maincpu, 16000000)); /* 386SX */
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
	m_maincpu->set_clock(6000000); // should be 24_MHz_XTAL / 2, but doesn't post with that setting
	subdevice<isa16_slot_device>("isa1")->set_default_option("vga"); // should be ATI EGA Wonder 800+
}

// Commodore PC 40-III
void at_state::pc40iii(machine_config &config)
{
	ibm5170(config);
	m_maincpu->set_clock(6000000); // should be 24_MHz_XTAL / 2, but doesn't post with that setting
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
	keybc.kbd_clk().set("pc_kbdc", FUNC(pc_kbdc_device::clock_write_from_mb));
	keybc.kbd_data().set("pc_kbdc", FUNC(pc_kbdc_device::data_write_from_mb));

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "pc_kbdc", 0));
	pc_kbdc.out_clock_cb().set("keybc", FUNC(at_keyboard_controller_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set("keybc", FUNC(at_keyboard_controller_device::kbd_data_w));
	PC_KBDC_SLOT(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL).set_pc_kbdc_slot(subdevice("pc_kbdc"));

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
	i486_device &maincpu(I486(config.replace(), m_maincpu, 66000000 / 2));
	maincpu.set_addrmap(AS_PROGRAM, &megapc_state::megapcpl_map);
	maincpu.set_addrmap(AS_IO, &megapc_state::megapcpl_io);
	maincpu.set_irq_acknowledge_callback("wd7600", FUNC(wd7600_device::intack_cb));
}

void at_vrom_fix_state::megapcpla(machine_config &config)
{
	i486_device &maincpu(I486(config, m_maincpu, 66000000 / 2));  // 486SLC
	maincpu.set_addrmap(AS_PROGRAM, &at_vrom_fix_state::at32l_map);
	maincpu.set_addrmap(AS_IO, &at_vrom_fix_state::at32_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	AT_MB(config, m_mb, 0).at_softlists(config);

	config.m_minimum_quantum = attotime::from_hz(60);

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
	PC_KBDC_SLOT(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL).set_pc_kbdc_slot(subdevice("mb:pc_kbdc"));

	/* internal ram */
	RAM(config, m_ram).set_default_size("4M").set_extra_options("2M,8M,15M,16M,32M,64M,128M,256M");

	/* software lists */
	SOFTWARE_LIST(config, "disk_list").set_original("megapc");
}

void at_state::ficpio2(machine_config &config)
{
	i486_device &maincpu(I486(config, m_maincpu, 25000000));
	maincpu.set_addrmap(AS_PROGRAM, &at_state::ficpio_map);
	maincpu.set_addrmap(AS_IO, &at_state::ficpio_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	AT_MB(config, m_mb, 0).at_softlists(config);
	config.m_minimum_quantum = attotime::from_hz(60);

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
	PC_KBDC_SLOT(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL).set_pc_kbdc_slot(subdevice("mb:pc_kbdc"));

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

	AT_MB(config, m_mb, 0).at_softlists(config);
	config.m_minimum_quantum = attotime::from_hz(60);

	// FIXME: determine ISA bus clock
	ISA16_SLOT(config, "board1", 0, "mb:isabus", pc_isa16_cards, "fdc", true).set_option_machine_config("fdc", cfg_single_1200K);
	ISA16_SLOT(config, "board2", 0, "mb:isabus", pc_isa16_cards, "comat", true);
	ISA16_SLOT(config, "board3", 0, "mb:isabus", pc_isa16_cards, "hdc", true);
	ISA16_SLOT(config, "board4", 0, "mb:isabus", pc_isa16_cards, "cga_cportiii", true);
	ISA16_SLOT(config, "isa1",   0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2",   0, "mb:isabus", pc_isa16_cards, nullptr, false);

	PC_KBDC_SLOT(config, "kbd", pc_at_keyboards, STR_KBD_IBM_PC_AT_84).set_pc_kbdc_slot(subdevice("mb:pc_kbdc"));

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
	m_maincpu->set_clock(6000000);
	subdevice<isa16_slot_device>("isa1")->set_default_option("cga");
}

// Nixdorf 8810 M55
void at_state::n8810m55(machine_config &config)
{
	ibm5170(config);
	m_maincpu->set_clock(6000000);
	subdevice<isa16_slot_device>("isa1")->set_default_option("ega");
}

//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

//**************************************************************************
//  IBM systems
//**************************************************************************
ROM_START( ibm5170 )
	ROM_REGION(0x20000,"bios", 0)

	ROM_SYSTEM_BIOS( 0, "rev1", "IBM PC/AT 5170 01/10/84")
	ROMX_LOAD( "6181028.u27", 0x10000, 0x8000, CRC(f6573f2a) SHA1(3e52cfa6a6a62b4e8576f4fe076c858c220e6c1a), ROM_SKIP(1) | ROM_BIOS(0)) /* T 6181028 8506AAA // TMM23256P-5878 // (C)IBM CORP 1981,-1984 */
	ROMX_LOAD( "6181029.u47", 0x10001, 0x8000, CRC(7075fbb2) SHA1(a7b885cfd38710c9bc509da1e3ba9b543a2760be), ROM_SKIP(1) | ROM_BIOS(0)) /* T 6181029 8506AAA // TMM23256P-5879 // (C)IBM CORP 1981,-1984 */

	ROM_SYSTEM_BIOS( 1, "rev2", "IBM PC/AT 5170 06/10/85")  /* Another verifaction of these crcs would be nice */
	ROMX_LOAD( "6480090.u27", 0x10000, 0x8000, CRC(99703aa9) SHA1(18022e93a0412c8477e58f8c61a87718a0b9ab0e), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "6480091.u47", 0x10001, 0x8000, CRC(013ef44b) SHA1(bfa15d2180a1902cb6d38c6eed3740f5617afd16), ROM_SKIP(1) | ROM_BIOS(1))

//  ROM_SYSTEM_BIOS( 2, "atdiag", "IBM PC/AT 5170 w/Super Diagnostics")
//  ROMX_LOAD( "atdiage.bin", 0xf8000, 0x4000, CRC(e8855d0c) SHA1(c9d53e61c08da0a64f43d691bf6cadae5393843a), ROM_SKIP(1) | ROM_BIOS(2))
//  ROMX_LOAD( "atdiago.bin", 0xf8001, 0x4000, CRC(606fa71d) SHA1(165e45bae7ae2da274f1e645c763c5bfcbde027b), ROM_SKIP(1) | ROM_BIOS(2))

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
	//ROM_LOAD16_BYTE( "video-card_573rf6( 2764)_040.rom", 0xc0001, 0x2000, CRC(a3ece315) SHA1(e800e11c3b1b6fcaf41bfb7d4058a9d34fdd2b3f))
	//ROM_LOAD16_BYTE( "video-card_573rf6( 2764)_041.rom", 0xc0000, 0x2000, CRC(b0a2ba7f) SHA1(c8160e8bc97cd391558f1dddd3fd3ec4a19d030c))
ROM_END

ROM_START( ibm5170a )
	ROM_REGION(0x20000,"bios", 0)
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


ROM_START( ibm5162 ) //MB p/n 62x1168
	ROM_REGION16_LE(0x20000,"bios", 0)

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

ROM_START( ibmps1es )
	ROM_REGION(0x40000, "bios", 0)
	ROM_LOAD16_BYTE( "ibm_1057757_24-05-90.bin", 0x00000, 0x20000, CRC(c8f81ea4) SHA1(925ed0e98f9f2997cb86554ef384bcfaf2a4ecbe))
	ROM_LOAD16_BYTE( "ibm_1057757_29-15-90.bin", 0x00001, 0x20000, CRC(c2dd6b5c) SHA1(f6b5785002dd628b6b1fb3bb101e076299eba3b6))
ROM_END


//**************************************************************************
//  Apricot systems
//**************************************************************************

// Apricot XEN-S (Venus I Motherboard 286)
ROM_START( xb42639 )
	/* actual VGA BIOS not dumped*/
	ROM_REGION(0x20000, "bios", 0)
	// XEN-S (Venus I Motherboard)
	ROM_LOAD16_BYTE( "3-10-17i.lo", 0x10000, 0x8000, CRC(3786ca1e) SHA1(c682d7c76f234559d03bcf21010c13c4dbeafb69))
	ROM_LOAD16_BYTE( "3-10-17i.hi", 0x10001, 0x8000, CRC(d66710eb) SHA1(e8c1cd5f9ecfbd8825655e416d7ddf2ae362e69b))
ROM_END

// Apricot XEN-S (Venus II Motherboard 286)
ROM_START( xb42639a )
	/* actual VGA BIOS not dumped*/
	ROM_REGION(0x20000, "bios", 0)
	// XEN-S (Venus II Motherboard)
	ROM_LOAD16_BYTE( "10217.lo", 0x10000, 0x8000, CRC(ea53406f) SHA1(2958dfdbda14de4e6b9d6a8c3781131ab1e32bef))
	ROM_LOAD16_BYTE( "10217.hi", 0x10001, 0x8000, CRC(111725cf) SHA1(f6018a45bda4476d40c5881fb0a506ff75ec1688))
ROM_END

// Apricot XEN-S (Venus I Motherboard 386)
ROM_START( xb42664 )
	/* actual VGA BIOS not dumped */
	ROM_REGION(0x20000, "bios", 0)
	// XEN-S (Venus I Motherboard)
	ROM_LOAD16_BYTE( "3-10-17i.lo", 0x10000, 0x8000, CRC(3786ca1e) SHA1(c682d7c76f234559d03bcf21010c13c4dbeafb69))
	ROM_LOAD16_BYTE( "3-10-17i.hi", 0x10001, 0x8000, CRC(d66710eb) SHA1(e8c1cd5f9ecfbd8825655e416d7ddf2ae362e69b))
ROM_END

// Apricot XEN-S (Venus II Motherboard 386)
ROM_START( xb42664a )
	/* actual VGA BIOS not dumped*/
	ROM_REGION(0x20000, "bios", 0)
	// XEN-S (Venus II Motherboard)
	ROM_LOAD16_BYTE( "10217.lo", 0x10000, 0x8000, CRC(ea53406f) SHA1(2958dfdbda14de4e6b9d6a8c3781131ab1e32bef))
	ROM_LOAD16_BYTE( "10217.hi", 0x10001, 0x8000, CRC(111725cf) SHA1(f6018a45bda4476d40c5881fb0a506ff75ec1688))
ROM_END

// Apricot Qi 300 (Rev D,E & F Motherboard)
ROM_START( xb42663 )
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "qi310223.lo", 0x00000, 0x10000, CRC(53047f49) SHA1(7b38e533f7f27295269549c63e5477d950239167))
	ROM_LOAD16_BYTE( "qi310223.hi", 0x00001, 0x10000, CRC(4852869f) SHA1(98599d4691d40b3fac2936034c70b386ce4caf77))
ROM_END

// Apricot Qi 600 (Neptune Motherboard)
ROM_START( qi600 )
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "qi610223.lo", 0x00000, 0x10000, CRC(563114a9) SHA1(62932b3bf0b5502ff708f604c21773f00afda58e))
	ROM_LOAD16_BYTE( "qi610223.hi", 0x00001, 0x10000, CRC(0ae133f6) SHA1(6039c366f7fe0ebf60b34c1a7d6b2d781b664001))
ROM_END

// Apricot Qi 900 (Scorpion Motherboard)
ROM_START( qi900 )
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "qi910224.lo", 0x00000, 0x10000, CRC(b012ad3c) SHA1(807e788a6bd03f5e983fe503af3d0b202c754b8a))
	ROM_LOAD16_BYTE( "qi910224.hi", 0x00001, 0x10000, CRC(36e66d56) SHA1(0900c5272ec3ced550f18fb08db59ab7f67a621e))
ROM_END

// Apricot FTs (Scorpion)
ROM_START( ftsserv )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "fts10226.lo", 0x00000, 0x10000, CRC(efbd738f) SHA1(d5258760bafdaf1bf13c4a49da76d4b5e7b4ccbd))
	ROM_LOAD16_BYTE( "fts10226.hi", 0x00001, 0x10000, CRC(2460853f) SHA1(a6bba8d2f800140afd129c4d5278f7ae8fe7e63a))
	/* FT Server series Front Panel */
	ROM_REGION(0x10000,"front", 0)
	ROM_LOAD( "fp10009.bin",     0x0000, 0x8000, CRC(8aa7f718) SHA1(9ee6c6a5bb92622ea8d3805196d42ff68887d820))
ROM_END

// Apricot XEN-LS (Venus IV Motherboard)
ROM_START( apxenls3 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "31020.lo", 0x10000, 0x8000, CRC(a19678d2) SHA1(d13c12fa7e94333555eabf58b81bad421e21cd91))
	ROM_LOAD16_BYTE( "31020.hi", 0x10001, 0x8000, CRC(4922e020) SHA1(64e6448323dad2209e004cd93fa181582e768ed5))
ROM_END

// Apricot LANstation (Krypton Motherboard)
ROM_START( aplanst )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "31024", "Bios 3-10-24")
	ROMX_LOAD( "31024.lo", 0x10000, 0x8000, CRC(e52b59e1) SHA1(cfcaa4d8d658df8df463108ef30695bd4ee7a617), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "31024.hi", 0x10001, 0x8000, CRC(7286aefa) SHA1(dfc0e3f4936780fa62ae9ec392ce17aa65e717cd), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "31025", "Bios 3-10-25")
	ROMX_LOAD( "31025.lo", 0x10000, 0x8000, CRC(1aec09bc) SHA1(51d56c97c7c1674554aa89b68945329ea967a8bc), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "31025.hi", 0x10001, 0x8000, CRC(0763caa5) SHA1(48510a933dcd6efea3b14d04444f584c3e6fefeb), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "31026", "Bios 3-10-26i")
	ROMX_LOAD( "31026i.lo", 0x10000, 0x8000, CRC(670b6ab4) SHA1(8d61a0edf187f99b67eb58f5e11276deee801d17), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "31026i.hi", 0x10001, 0x8000, CRC(ef01c54f) SHA1(911f95d65ab96878e5e7ebccfc4b329db47a1351), ROM_SKIP(1) | ROM_BIOS(2))
ROM_END

// Apricot LANstation (Novell Remote Boot)
ROM_START( aplannb )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "lsl31025.lo", 0x00000, 0x10000, CRC(8bb7229b) SHA1(31449d12884ec4e7752e6c1ce7ce9e0d044eadf2))
	ROM_LOAD16_BYTE( "lsh31025.hi", 0x00001, 0x10000, CRC(09e5c1b9) SHA1(d42be83b4181d3733268c29df04a4d2918370f4e))
ROM_END

// Apricot VX FT server
ROM_START( apvxft )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "ft10221.lo", 0x00000, 0x10000, CRC(8f339de0) SHA1(a6542406746eaf1ff7f9e3678c5cbe5522fb314a))
	ROM_LOAD16_BYTE( "ft10221.hi", 0x00001, 0x10000, CRC(3b16bc31) SHA1(0592d1d81e7fd4715b0612083482db122d78c7f2))
ROM_END

// Apricot LS Pro (Caracal Motherboard)
ROM_START( aplscar )
	ROM_REGION(0x40000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "car306", "Caracal 3.06")
	ROMX_LOAD( "car306.bin",   0x00000, 0x40000, CRC(fc271dea) SHA1(6207cfd312c9957243b8157c90a952404e43b237), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "car307", "Caracal 3.07")
	ROMX_LOAD( "car307.bin",   0x00000, 0x40000, CRC(66a01852) SHA1(b0a68c9d67921d27ba483a1c50463406c08d3085), ROM_BIOS(1))
ROM_END

// Apricot XEN PC (A1 Motherboard)
ROM_START( apxena1 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD( "a1-r26.bin",   0x00000, 0x20000, CRC(d29e983e) SHA1(5977df7f8d7ac2a154aa043bb6f539d96d51fcad))
ROM_END

// Apricot XEN PC (P2 Motherboard)
ROM_START( apxenp2 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "p2r02g2", "p2r02g2")
	ROMX_LOAD( "p2r02g2.bin",   0x00000, 0x20000, CRC(311bcc5a) SHA1(be6fa144322077dcf66b065e7f4e61aab8c278b4), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "lep121s", "SCSI-Enabling ROMs")
	ROMX_LOAD("p2r01f0.bin",   0x00000, 0x20000, CRC(bbc68f2e) SHA1(6954a52a7dda5521794151aff7a04225e9c7df77), ROM_BIOS(1))
ROM_END

// Apricot XEN-i 386 (Leopard Motherboard)
ROM_START( apxeni )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "lep121", "Rom Bios 1.2.1")
	ROMX_LOAD( "lep121.bin", 0x18000, 0x8000, CRC(948c1927) SHA1(d06bdbd6292db73c815ad1060daf055293dfddf5), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "lep121s", "SCSI-Enabling ROMs")
	ROMX_LOAD( "lep121s.bin", 0x18000, 0x8000, CRC(296118e4) SHA1(d1feaa9704e6ce3bc10c900bdd310d9494b02304), ROM_BIOS(1))
ROM_END

// Apricot LS Pro (Bonsai Motherboard)
ROM_START( aplsbon )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "bon106", "Boinsai 1-06")
	ROMX_LOAD( "bon106.bin",   0x00000, 0x20000, CRC(98a4eb76) SHA1(e0587afa78aeb9a8803f9b9f9e457e9847b0a2b2), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "bon203", "Boinsai 2-03")
	ROMX_LOAD( "bon203.bin",   0x00000, 0x20000, CRC(32a0e125) SHA1(a4fcbd76952599993fa8b76aa36a96386648abb2), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "bon10703", "Boinsai 1-07-03")
	ROMX_LOAD( "bon10703.bin",   0x00000, 0x20000, CRC(0275b3c2) SHA1(55ef4cbb7f3166f678aaa478234a42049deaba5f), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "bon20402", "Boinsai 2.03")
	ROMX_LOAD( "bon20402.bin",   0x00000, 0x20000, CRC(ac5803fb) SHA1(b8fe92711c6a38a5d9e6497e76a0929c1685c631), ROM_BIOS(3))
ROM_END

// Apricot XEN-LS II (Samurai Motherboard)
ROM_START( apxlsam )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "sam107", "ROM BIOS Version 1-07")
	ROMX_LOAD( "sam1-07.bin",   0x00000, 0x20000, CRC(65e05a8e) SHA1(c3cd198a129122cb05a28798e54331b06cfdd310), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "sam206", "ROM BIOS Version 2-06")
	ROMX_LOAD( "sam2-06.bin",   0x00000, 0x20000, CRC(9768bb0f) SHA1(8166b77b133072f72f23debf85984eb19578ffc1), ROM_BIOS(1))
ROM_END

// Apricot FTs (Panther Rev F 1.02.26)
ROM_START( aprpand )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "pf10226.std", 0x00000, 0x10000, CRC(7396fb87) SHA1(a109cbad2179eec55f86c0297a59bb015461da21))
	ROM_CONTINUE( 0x00001, 0x10000 )
ROM_END

// Apricot FT//ex 486 (J3 Motherboard)
ROM_START( aprfte )
	ROM_REGION(0x20000,"bios", 0)
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
	ROM_REGION(0x40000, "bios", 0)
	ROM_LOAD( "megapc_bios.bin",  0x00000, 0x10000, CRC(b84938a2) SHA1(cecab72a96993db4f7c648c229b4211a8c53a380))
	ROM_CONTINUE(0x30000, 0x10000)
ROM_END

// Amstrad PC2386
ROM_START( pc2386 )
	ROM_REGION( 0x40000, "bios", 0 )
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

// Commodore Laptop C286-LT - screen remains blank
ROM_START( c286lt )
	ROM_REGION(0x20000, "bios", 0) // BIOS contains Cirrus Logic VGA firmware, rebadged Sanyo MBC-17NB
	ROM_SYSTEM_BIOS(0, "c286lt13", "C286-LT V1.3")
	ROMX_LOAD( "cbm-c286lt-bios-v1.3-390854-01-1200.bin", 0x00000, 0x20000, CRC(785e87d2) SHA1(e271500169955473d44102a60f051b5f6cfae589), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "c286v17-854", "C286-LT V1.7 390854")
	ROMX_LOAD( "cbm-c286lt-bios-v1.7-390854-04.bin", 0x00000, 0x20000, CRC(2f762ab1) SHA1(d6cb37f0dcb261df86c01d4e1eabe10a52b2070f), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "c286v17-940", "C286-LT V1.7 390940")
	ROMX_LOAD( "cbm-c286lt-bios-v1.7-390940-04.bin", 0x00000, 0x20000, CRC(22d45839) SHA1(bc7159440c52c1f69957da8fdfa76ac0a42ebd16), ROM_BIOS(2))
ROM_END

// Commodore SL 286-16
ROM_START( csl286 ) // continuous short beeps after POST
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "cbm-sl286-16-bios-lo-v1.02-390958-03.bin", 0x10000, 0x8000, CRC(7d0c9472) SHA1(1d614f6835a388f67ece73f40d8a9f65cca3e855))
	ROM_LOAD16_BYTE( "cbm-sl286-16-bios-hi-v1.02-390959-03.bin", 0x10001, 0x8000, CRC(b6d81ddd) SHA1(9478bb846bd1e0dc1904f21d43c6df01ecbc9c83))
ROM_END

// Commodore SL 386SX
ROM_START( c386sx16 )
	ROM_REGION(0x20000, "bios", 0)
	// actual VGA BIOS not dumped - uses a WD Paradise according to http://www.cbmhardware.de/pc/pc.php
	// complains "Time-of-day clock stopped"
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

// Commodore Laptop C386SX-LT -  screen remains blank
ROM_START( c386sxlt )
	ROM_REGION(0x20000, "bios", 0) // BIOS contains Cirrus Logic VGA firmware, rebadged Sanyo MBC-18NB, but different versions exist
	ROM_SYSTEM_BIOS(0, "c386sxlt_b400", "C386SX-LT V1.2 B400")
	ROMX_LOAD( "cbm-386lt-bios-v1.2-390981-03-b400.bin", 0x00000, 0x20000, CRC(b84f6883) SHA1(3f31060726c7c49a891b35ab024524a4239eb4d0), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "c386sxlt_cf00", "C386SX-LT V1.2 CF00")
	ROMX_LOAD( "cbm-386lt-bios-v1.2-390982-03-cf00.bin", 0x00000, 0x20000, CRC(c8cd2641) SHA1(18e55bff494c42389dfb445f2bc11e78db30e5f7), ROM_BIOS(1))
ROM_END

// Commodore DT386
ROM_START( dt386 )
	// BIOS-String: 40-0502-DG1112-00101111-070791-SOLUTION-0 / 386DX-33 BIOS V1.00 #391560-0
	ROM_REGION(0x20000, "bios", 0)
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

// Commodore DT486 - BIOS contains VGA ROM - Keyboard error
ROM_START( dt486 ) // BIOS string: 41-0102-001283-00111111-060692-SYM_486-0 - Commodore 486DX-33 BIOS Version 1.01 391521-02
	ROM_REGION(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "dt486", "DT486")
	ROM_LOAD( "cbm-dt486dx-33c-bios-u32--v1.01-391521-02.bin", 0x00000, 0x20000, BAD_DUMP CRC(a3977625) SHA1(83bc563fb41eae3dd5d260f13c6fe8979a77e99c))
ROM_END

// Commodore PC 30-III
ROM_START( pc30iii )
	ROM_REGION(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "pc30iii_v200", "PC 30-III v2.00")
	ROMX_LOAD( "pc30iii_390339-02_3e58.bin", 0x18000, 0x4000, CRC(f4a5860e) SHA1(b843744fe928bcfd8e037b0208cc85c0746535cf),ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "pc30iii_390340-02_42a8.bin",  0x18001, 0x4000, CRC(934df54a) SHA1(3b1c8916ba2b2517bc9f26dd74254586bcf0e91d),ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "pc30iii_v201", "PC 30-III v2.01")
	ROMX_LOAD( "cbm-pc30c-bios-lo-v2.01-390339-03-35c1.bin", 0x18000, 0x4000, CRC(36307aa9) SHA1(50237ffea703b867de426ab9ebc2af46bac1d0e1),ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "cbm-pc30c-bios-hi-v2.01-390340-03-3f3f.bin",  0x18001, 0x4000, CRC(41bae42d) SHA1(27d6ad9554be86359d44331f25591e3122a31519),ROM_SKIP(1) | ROM_BIOS(1) )
ROM_END

// Commodore PC 40-III
ROM_START( pc40iii )
	// VGA BIOS
	// ROM_LOAD( "pc40iii_390337-01_v2.0_f930.bin", 0x00000, 0x4000, CRC(82b210d3) SHA1(1380107deef02455c6ce4d12162fdc32e375cbde))
	// ROM_LOAD( "pc40iii_390338-01_v2.0_b6d0.bin", 0x00001, 0x4000, CRC(526d7424) SHA1(60511ca0e856b7611d556aa82219d646f96c9b94))

	ROM_REGION(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "pc40iii_v200", "PC 40-III v2.00")
	ROMX_LOAD( "pc40iii_390339-01_v2.0_473a.bin", 0x18000, 0x4000, CRC(2ad2dc0f) SHA1(b41d5988fda8cc23418c3f665d780c617aa3fc2b),ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "pc40iii_390340-01_v2.0_4bc6.bin",  0x18001, 0x4000, CRC(62dc7d93) SHA1(e741528697b1d00450fd18e3db8b925606e0bd22),ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "pc40iii_v201", "PC 40-III v2.03")
	ROMX_LOAD( "cbm-pc40c-bios-lo-v2.03-390339-04-03bc.bin", 0x18000, 0x4000, CRC(e5fd11c6) SHA1(18c21d9a4ae687eef5464b76a0d614b9dfd30ec8),ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "cbm-pc40c-bios-hi-v2.03-390340-04-3344.bin",  0x18001, 0x4000, CRC(63d6f0f7) SHA1(a88dee7694baa71913acbe76cb4e2a4e95979ad9),ROM_SKIP(1) | ROM_BIOS(1) )
ROM_END

// Commodore PC 45-III - this is a PC 40-III with a BIOS update and a bigger, 52MB, harddisk
ROM_START( pc45iii )
	ROM_REGION(0x20000, "bios", 0) // Commodore 286 BIOS Rev. 2.04 - 390339-05/390340-05
	ROM_LOAD16_BYTE( "cbm-pc45c-bios-lo-v2.04-390339-05.bin", 0x18000, 0x4000, CRC(b87b4cd1) SHA1(a6723d63a255b4010ad32b5dc9797e4724a64c14))
	ROM_LOAD16_BYTE( "cbm-pc45c-bios-hi-v2.04-390340-05.bin", 0x18001, 0x4000, CRC(b6976111) SHA1(e7c92307db3969a6a50ffd8cbc3d2ed16b4df6ad))
ROM_END

// Commodore PC 50-II - a photo of the mainboard shows four ROMs (two each for BIOS and VGA), so the 128K dumps available were probably made from a running system.
ROM_START( pc50ii )
	ROM_REGION(0x20000, "bios", 0)
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

// Commodore PC-60-III - complaining "time-of-day-clock stopped"
ROM_START( pc60iii )
	ROM_REGION(0x20000, "bios", 0)
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
	ROM_REGION(0x20000, "bios", 0)
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
	ROM_REGION(0x20000, "bios", 0)
	// Phoenix 80386 ROM BIOS PLUS Version 1.10 22 - Twinhead International Corporation
	ROM_LOAD16_BYTE( "cbm-t386-bios-lo-v1.1022c-.bin", 0x10000, 0x8000, CRC(6857777e) SHA1(e80dbffd3523c9a1b027f57138c55768fc8328a6))
	ROM_LOAD16_BYTE( "cbm-t386-bios-hi-v1.1022c-.bin", 0x10001, 0x8000, CRC(6a321a7e) SHA1(c350fb273522f742c6008deda00ed13947a269b7))
ROM_END

// Commodore Tower 486
ROM_START( comt486 )
	ROM_REGION(0x20000, "bios", 0)
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
	ROM_REGION(0x20000,"bios", 0)
	// 0: BIOS-String: ENET-1107-040990-K0
	ROM_SYSTEM_BIOS(0, "ami211", "AMI 21.1") /*(Motherboard Manufacturer: Dataexpert Corp. Motherboard) (Neat 286 Bios, 82c21x Chipset ) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "ami211.bin",     0x10000, 0x10000,CRC(a0b5d269) SHA1(44db8227d35a09e39b93ed944f85dcddb0dd0d39), ROM_BIOS(0))
	// 1: BIOS-String: D286-0011-110387
	ROM_SYSTEM_BIOS(1, "at", "PC 286") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 03-11-1987)*/
	ROMX_LOAD( "at110387.1", 0x10001, 0x8000, CRC(679296a7) SHA1(ae891314cac614dfece686d8e1d74f4763cf40e3),ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "at110387.0", 0x10000, 0x8000, CRC(65ae1f97) SHA1(91a29c7deecf7a9afbba330e64e0eee9aafee4d1),ROM_SKIP(1) | ROM_BIOS(1) )
	// 2: BIOS-String: S286-6181-101590-K0
	ROM_SYSTEM_BIOS(2, "ami206", "AMI C 206.1")  /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 15-10-1990)*/
	ROMX_LOAD( "amic206.bin",    0x10000, 0x10000,CRC(25a67c34) SHA1(91e9d8cdc2f1b40a601a23ceaff2189fd1245f3b), ROM_BIOS(2) )
	//3: BIOS-String: ENET-1230-043089-K0
	ROM_SYSTEM_BIOS(3, "amic21", "AMI C 21.1")
	ROMX_LOAD( "amic21-2.bin",  0x10001, 0x8000, CRC(8ffe7752) SHA1(68215f07a170ee7bdcb3e52b370d470af1741f7e),ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "amic21-1.bin",  0x10000, 0x8000, CRC(a76497f6) SHA1(91b47d86967426945b2916cb40e76a8da2d31d54),ROM_SKIP(1) | ROM_BIOS(3) )
	// 4: Quadtel Enhanced 286 BIOS Version 3.04.02
	ROM_SYSTEM_BIOS(4, "ami101", "AMI HT 101.1") /* Quadtel Enhanced 286 Bios Version 3.04.02 */
	ROMX_LOAD( "amiht-h.bin",   0x10001, 0x8000, CRC(8022545f) SHA1(42541d4392ad00b0e064b3a8ccf2786d875c7c19),ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "amiht-l.bin",   0x10000, 0x8000, CRC(285f6b8f) SHA1(2fce4ec53b68c9a7580858e16c926dc907820872),ROM_SKIP(1) | ROM_BIOS(4) )
	// 5: BIOS-String: DH12-1211-061390-K0
	ROM_SYSTEM_BIOS(5, "ami121", "AMI HT 12.1")
	ROMX_LOAD( "ami2od86.bin",  0x10001, 0x8000, CRC(04a2cec4) SHA1(564d37a8b2c0f4d0e23cd1e280a09d47c9945da8),ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "ami2ev86.bin",  0x10000, 0x8000, CRC(55deb5c2) SHA1(19ce1a7cc985b5895c585e39211475de2e3b0dd1),ROM_SKIP(1) | ROM_BIOS(5) )
	// 6: BIOS-String: DH12-1120-061390-K0
	ROM_SYSTEM_BIOS(6, "ami122", "AMI HT 12.2")
	ROMX_LOAD( "ami2od89.bin",  0x10001, 0x8000, CRC(7c81bbe8) SHA1(a2c7eca586f6e2e76b9101191e080a1f1cb8b833),ROM_SKIP(1) | ROM_BIOS(6) )
	ROMX_LOAD( "ami2ev89.bin",  0x10000, 0x8000, CRC(705d36e0) SHA1(0c9cfb71ced4587f109b9b6dfc2a9c92302fdb99),ROM_SKIP(1) | ROM_BIOS(6) )
	// 7: BIOS-String: DH12-1112-061390-K0
	ROM_SYSTEM_BIOS(7, "ami123", "AMI HT 12.3") /*(Motherboard Manufacturer: Aquarius Systems USA Inc.) (BIOS release date:: 13-06-1990)*/
	ROMX_LOAD( "ht12h.bin",     0x10001, 0x8000, CRC(db8b471e) SHA1(7b5fa1c131061fa7719247db3e282f6d30226778),ROM_SKIP(1) | ROM_BIOS(7) )
	ROMX_LOAD( "ht12l.bin",     0x10000, 0x8000, CRC(74fd178a) SHA1(97c8283e574abbed962b701f3e8091fb82823b80),ROM_SKIP(1) | ROM_BIOS(7) )
	// 8
	ROM_SYSTEM_BIOS(8, "ami181", "AMI HT 18.1") /* not a bad dump, sets unknown probably chipset related registers at 0x1e8 before failing post */
	ROMX_LOAD( "ht18.bin",     0x10000, 0x10000, CRC(f65a6f9a) SHA1(7dfdf7d243f9f645165dc009c5097dd515f86fbb), ROM_BIOS(8) )
	// 9
	ROM_SYSTEM_BIOS(9, "amiht21", "AMI HT 21.1") /* as above */
	ROMX_LOAD( "ht21e.bin",    0x10000, 0x10000, CRC(e80f7fed) SHA1(62d958d98c95e9e4d1b290a6c1054ae98770f276), ROM_BIOS(9) )
	// 10: BIOS-String: D286-1430-040990-K0
	ROM_SYSTEM_BIOS(10, "amip1", "AMI P.1") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "poisk-h.bin",   0x10001, 0x8000, CRC(83fd3f8c) SHA1(ca94850bbd949b97b11710629886b0ee69489a81),ROM_SKIP(1) | ROM_BIOS(10) )
	ROMX_LOAD( "poisk-l.bin",   0x10000, 0x8000, CRC(0b2ed291) SHA1(bb51a3f317cf4d429a6cfb44a46ca0ac39d9aaa7),ROM_SKIP(1) | ROM_BIOS(10) )
	// 11: Award 286 Modular BIOS Version 3.11 - WINBOND
	ROM_SYSTEM_BIOS(11, "aw201", "Award 201")
	ROMX_LOAD( "83201-5h.bin",  0x10001, 0x8000, CRC(968d1fc0) SHA1(dc4122a6c696f0b43e7894dc1b669346eed755d5),ROM_SKIP(1) | ROM_BIOS(11) )
	ROMX_LOAD( "83201-5l.bin",  0x10000, 0x8000, CRC(bf50a89a) SHA1(2349a1db6017a7fb0673e99d3680c8753407be8d),ROM_SKIP(1) | ROM_BIOS(11) )
	// 12: Award 286 Modular BIOS V3.03 NFS 11/10/87" - T.M.C
	ROM_SYSTEM_BIOS(12, "aw303", "Award 303 NFS")
	ROMX_LOAD( "aw303-hi.bin",  0x18001, 0x4000, CRC(78f32d7e) SHA1(1c88398fb171b33b7e6191bad63704ae85bfed8b), ROM_SKIP(1) | ROM_BIOS(12) )
	ROMX_LOAD( "aw303-lo.bin",  0x18000, 0x4000, CRC(3d2a70c0) SHA1(1329113bec514ed2a6d803067b1132744ef534dd), ROM_SKIP(1) | ROM_BIOS(12) )
	// 13: Award 286 Modular BIOS Version 3.03GS
	ROM_SYSTEM_BIOS(13, "aw303gs", "Award 303GS")
	ROMX_LOAD( "aw303gs-hi.bin",  0x18001, 0x4000, CRC(82392e18) SHA1(042453b7b29933a1b72301d21fcf8fa6b293c9c9), ROM_SKIP(1) | ROM_BIOS(13) )
	ROMX_LOAD( "aw303gs-lo.bin",  0x18000, 0x4000, CRC(a4cf8ba1) SHA1(b73e34be3b2754aaed1ac06471f4441fea06c67c), ROM_SKIP(1) | ROM_BIOS(13) )
	// 14: BIOS-String: D286-6069-040990-K0
	ROM_SYSTEM_BIOS(14, "ami_200960", "AMI 200960")
	ROMX_LOAD( "ami_286_bios_sn200960_even.bin", 0x10000, 0x8000, CRC(67745815) SHA1(ca6886c7a0716a92a8720fc71ff2d95328c467a5), ROM_SKIP(1) | ROM_BIOS(14) )
	ROMX_LOAD( "ami_286_bios_sn200960_odd.bin", 0x10001, 0x8000, CRC(360a5f73) SHA1(1b1980fd99779d0cdc4764928a641e081b35ee9f), ROM_SKIP(1) | ROM_BIOS(14) )
	// 15: Award 286 Modular BIOS Version 3.03 - GCH
	ROM_SYSTEM_BIOS(15, "ev1815303", "Everex EV-1815 V3.03")
	ROMX_LOAD( "award_v3.03_ev1815_even.bin", 0x18000, 0x4000, CRC(dd64bdd6) SHA1(b3108b692d2aa03701ac894602e9418ae0779702), ROM_SKIP(1) | ROM_BIOS(15) )
	ROMX_LOAD( "award_v3.03_ev1815_odd.bin", 0x18001, 0x4000, CRC(29f023fb) SHA1(873561bb7087483c0c763ef9cd32c1adf0f7cb5e), ROM_SKIP(1) | ROM_BIOS(15) )
	// 16: Award 286 Modular BIOS Version 3.03GS
	ROM_SYSTEM_BIOS(16, "ev1815303gs", "Everex EV-1815 V3.03GS")
	ROMX_LOAD( "award_v3.03gs_ev1815_even.bin", 0x10000, 0x8000, CRC(59489ec2) SHA1(b3c13ba53d4c4ee75a15703236a748121102ce84), ROM_SKIP(1) | ROM_BIOS(16) )
	ROMX_LOAD( "award_v3.03gs_ev1815_odd.bin", 0x10001, 0x8000, CRC(5bcd9421) SHA1(f32e5a39da593c6982f964fb05b0802d54c3de45), ROM_SKIP(1) | ROM_BIOS(16) )
	// 17: BIOS-String: DVL2-1160-040990-K0
	ROM_SYSTEM_BIOS(17, "dvl2", "DVL2")
	ROMX_LOAD( "ami_dvl2-1160-040990-k8_even.bin", 0x10000, 0x8000, CRC(86093016) SHA1(f60b2679c8c23a34bdd64f25d83cb5a5a337bd57), ROM_SKIP(1) | ROM_BIOS(17) )
	ROMX_LOAD( "ami_dvl2-1160-040990-k8_odd.bin", 0x10001, 0x8000, CRC(4e1c944a) SHA1(0763a0a1002baced071fea301f627d2e550878b8), ROM_SKIP(1) | ROM_BIOS(17) )
	// 18: DTK 286 BIOS Ver 3.18 07/01/88
	ROM_SYSTEM_BIOS(18, "dtk318", "DTK v3.18")
	ROMX_LOAD( "dtk_286_bios_ver3.18.bin", 0x18000, 0x8000, CRC(b4b8b59a) SHA1(73c12222f5003fdc8bbfee178b20c8dda2fe5cb4), ROM_BIOS(18) )
	// 19: BIOS-String: D286-6061-040990-K0
	ROM_SYSTEM_BIOS(19, "d286-k0", "AMI D286-K0")
	ROMX_LOAD( "ami_d286-6061-040990-k0.bin", 0x10000, 0x10000, CRC(1679c1b5) SHA1(9d95da3b40c5f13d096823f383aba099b3a77183), ROM_BIOS(19) )
	// 20: BIOS-String: S286-1169-030389-K0 for ACHIEVE MICROSYSTEMS
	ROM_SYSTEM_BIOS(20, "s286-k0", "Achieve S286-K0")
	ROMX_LOAD( "ach_s286-1169-030389-k0_ev.bin", 0x10000, 0x8000, CRC(58f1f29c) SHA1(42f5189d12b75fad5e53ff472b4603c6fcbd46cd), ROM_SKIP(1) | ROM_BIOS(20) )
	ROMX_LOAD( "ach_s286-1169-030389-k0_od.bin", 0x10001, 0x8000, CRC(84bfc180) SHA1(2daa51b09c449712c9a737793b83754951e53a41), ROM_SKIP(1) | ROM_BIOS(20) )
	// 21: Award BIOS Version 3.01B
	ROM_SYSTEM_BIOS(21, "awa301b", "Award BIOS Version 3.01B")
	ROMX_LOAD( "aw286lo.rom", 0x18000, 0x4000, CRC(5afbb4a2) SHA1(513fd75d90720820484fdd280e4a6c22a0ef238c), ROM_SKIP(1) | ROM_BIOS(21) )
	ROMX_LOAD( "aw286hi.rom", 0x18001, 0x4000, CRC(b2551251) SHA1(0c8bd12a3d54ae6d2ad0210b9ca4deca94be10ed), ROM_SKIP(1) | ROM_BIOS(21) )
	// 22
	ROM_SYSTEM_BIOS(22, "awa286", "awa286") // no screen display
	ROMX_LOAD( "awd286lo.rom", 0x18000, 0x4000, CRC(d1a9c01f) SHA1(9123c6f76d85725036a0f8b9c6480142abea478f), ROM_SKIP(1) | ROM_BIOS(22) )
	ROMX_LOAD( "awd286hi.rom", 0x18001, 0x4000, CRC(b0bde4cc) SHA1(9c3fd2c0f69dde905d4e8f3be421374ef99682df), ROM_SKIP(1) | ROM_BIOS(22) )
	// 23: DTK 286 BIOS Ver. 3.01 07/24/87
	ROM_SYSTEM_BIOS(23, "dtk286", "dtk286") // no screen display
	ROMX_LOAD( "dtk286lo.rom", 0x18000, 0x4000, CRC(dfc70856) SHA1(39158e6ed50236d371277631e77d06f77fb0531e), ROM_SKIP(1) | ROM_BIOS(23) )
	ROMX_LOAD( "dtk286hi.rom", 0x18001, 0x4000, CRC(a98fc743) SHA1(fb9e330148cb5584f61c1febea71c53b6f9d61b7), ROM_SKIP(1) | ROM_BIOS(23) )
	// 24: Phoenix 80286 ROM BIOS Version 3.07 (R04)
	ROM_SYSTEM_BIOS(24, "mitph307", "Mitac Phoenix v3.07")
	ROMX_LOAD( "mitac_phoenix_v3.07_even.bin", 0x10000, 0x8000, CRC(1c4becc9) SHA1(bfdea3f2a248312ed8cf4765a1a7dc1a2f7cecd8), ROM_SKIP(1) | ROM_BIOS(24) )
	ROMX_LOAD( "mitac_phoenix_v3.07_odd.bin", 0x10001, 0x8000, CRC(3ee16ed1) SHA1(b77e18e10e9187a01cb55c05b2a6e5311981ab56), ROM_SKIP(1) | ROM_BIOS(24) )
	// 25: BIOS-String: Pyramid Software Development Personal Computer AT Bios Version 2.14
	ROM_SYSTEM_BIOS(25, "precise", "Precise")
	ROMX_LOAD( "precise 860407_low.bin", 0x10000, 0x8000, CRC(d839c074) SHA1(473ca7b42914ce12f2d6c91afb0b2c2e65194489), ROM_SKIP(1) | ROM_BIOS(25) )
	ROMX_LOAD( "precise 860407_high.bin", 0x10001, 0x8000, CRC(b5e13c54) SHA1(07f5806fb53d0cb7ef7b54312fd6aa163d58b9a5), ROM_SKIP(1) | ROM_BIOS(25) )
	// ROM_LOAD( "precise_860407_keyboard_mcu.bin", 0x0000, 0x800, CRC(d1faad5c) SHA1(cb315a3da632c969012c298bb8e1cf8883b70501))
	// Access Methods Inc. for Flying Triumph (AMI before they became American Megatrends) - BIOS String: Ref. no. 1406-061296
	// complains about "Channel-2 timer not funcional but boots
	ROM_SYSTEM_BIOS(26, "ami_ft", "AMI Flying Triumph")
	ROMX_LOAD( "286_access_methods_rom2_32k.bin", 0x10000, 0x8000, CRC(749c65af) SHA1(7c6e9e217afe020b7b36785549fdbfb89de8f872), ROM_SKIP(1) | ROM_BIOS(26) )
	ROMX_LOAD( "286_access_methods_rom4_32k.bin", 0x10001, 0x8000, CRC(0f15581a) SHA1(2a22635f30388ca371f0f1f31652cfa647bb322d), ROM_SKIP(1) | ROM_BIOS(26) )
	// BIOS ROMs are marked TCI, Award 286 Modular BIOS Version 3.03HLS
	// complains about "refresh timing error, but works
	ROM_SYSTEM_BIOS(27, "tci_suntac", "tci_suntac")
	ROMX_LOAD( "suntac_80286_lo.bin", 0x18000, 0x4000, CRC(f7bf6c49) SHA1(d8e813c264008f096006f46b90769c0927e44da9), ROM_SKIP(1) | ROM_BIOS(27) )
	ROMX_LOAD( "suntac_80286_hi.bin", 0x18001, 0x4000, CRC(5f382e78) SHA1(8ba222df9d7028513e37978598d8139906e8834c), ROM_SKIP(1) | ROM_BIOS(27) )
ROM_END

ROM_START( atvga )
	ROM_REGION(0x20000,"bios", 0)
	// 0: BIOS-String: 20-0001-001223-00101111-050591-KB-8042--0
	ROM_SYSTEM_BIOS(0, "vl82c", "VL82C311L-FC4")/*(Motherboard Manufacturer: Biostar Microtech Corp.) (BIOS release date: 05-05-1991)*/
	ROMX_LOAD( "2vlm001.bin",     0x10000, 0x10000, CRC(f34d800a) SHA1(638aca592a0e525f957beb525e95ca666a994ee8), ROM_BIOS(0) )
	// 1: same as BIOS '0' in at
	ROM_SYSTEM_BIOS(1, "ami211", "AMI 21.1") /*(Motherboard Manufacturer: Dataexpert Corp. Motherboard) (Neat 286 Bios, 82c21x Chipset ) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "ami211.bin",     0x10000, 0x10000,CRC(a0b5d269) SHA1(44db8227d35a09e39b93ed944f85dcddb0dd0d39), ROM_BIOS(1))
	// 2: same as BIOS '2' in at
	ROM_SYSTEM_BIOS(2, "ami206", "AMI C 206.1") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 15-10-1990)*/
	ROMX_LOAD( "amic206.bin",    0x10000, 0x10000,CRC(25a67c34) SHA1(91e9d8cdc2f1b40a601a23ceaff2189fd1245f3b), ROM_BIOS(2) )
	// 3: same as BIOS '3' in at
	ROM_SYSTEM_BIOS(3, "amic21", "AMI C 21.1")
	ROMX_LOAD( "amic21-2.bin",  0x10001, 0x8000, CRC(8ffe7752) SHA1(68215f07a170ee7bdcb3e52b370d470af1741f7e),ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "amic21-1.bin",  0x10000, 0x8000, CRC(a76497f6) SHA1(91b47d86967426945b2916cb40e76a8da2d31d54),ROM_SKIP(1) | ROM_BIOS(3) )
	// 4: same as BIOS '4' in at
	ROM_SYSTEM_BIOS(4, "ami101", "AMI HT 101.1") /* Quadtel Enhanced 286 Bios Version 3.04.02 */
	ROMX_LOAD( "amiht-h.bin",   0x10001, 0x8000, CRC(8022545f) SHA1(42541d4392ad00b0e064b3a8ccf2786d875c7c19),ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "amiht-l.bin",   0x10000, 0x8000, CRC(285f6b8f) SHA1(2fce4ec53b68c9a7580858e16c926dc907820872),ROM_SKIP(1) | ROM_BIOS(4) )
	// 5: same as BIOS '5' in at
	ROM_SYSTEM_BIOS(5, "ami121", "AMI HT 12.1")
	ROMX_LOAD( "ami2od86.bin",  0x10001, 0x8000, CRC(04a2cec4) SHA1(564d37a8b2c0f4d0e23cd1e280a09d47c9945da8),ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "ami2ev86.bin",  0x10000, 0x8000, CRC(55deb5c2) SHA1(19ce1a7cc985b5895c585e39211475de2e3b0dd1),ROM_SKIP(1) | ROM_BIOS(5) )
	// 6: same as BIOS '6' in at
	ROM_SYSTEM_BIOS(6, "ami122", "AMI HT 12.2")
	ROMX_LOAD( "ami2od89.bin",  0x10001, 0x8000, CRC(7c81bbe8) SHA1(a2c7eca586f6e2e76b9101191e080a1f1cb8b833),ROM_SKIP(1) | ROM_BIOS(6) )
	ROMX_LOAD( "ami2ev89.bin",  0x10000, 0x8000, CRC(705d36e0) SHA1(0c9cfb71ced4587f109b9b6dfc2a9c92302fdb99),ROM_SKIP(1) | ROM_BIOS(6) )
	// 7: same as BIOS '7' in at
	ROM_SYSTEM_BIOS(7, "ami123", "AMI HT 12.3") /*(Motherboard Manufacturer: Aquarius Systems USA Inc.) (BIOS release date:: 13-06-1990)*/
	ROMX_LOAD( "ht12h.bin",     0x10001, 0x8000, CRC(db8b471e) SHA1(7b5fa1c131061fa7719247db3e282f6d30226778),ROM_SKIP(1) | ROM_BIOS(7) )
	ROMX_LOAD( "ht12l.bin",     0x10000, 0x8000, CRC(74fd178a) SHA1(97c8283e574abbed962b701f3e8091fb82823b80),ROM_SKIP(1) | ROM_BIOS(7) )
	// 8: same as BIOS '8' in at
	ROM_SYSTEM_BIOS(8, "ami181", "AMI HT 18.1") /* not a bad dump, sets unknown probably chipset related registers at 0x1e8 before failing post */
	ROMX_LOAD( "ht18.bin",     0x10000, 0x10000, CRC(f65a6f9a) SHA1(7dfdf7d243f9f645165dc009c5097dd515f86fbb), ROM_BIOS(8) )
	// 9: same as BIOS '9' in at
	ROM_SYSTEM_BIOS(9, "amiht21", "AMI HT 21.1") /* as above */
	ROMX_LOAD( "ht21e.bin",    0x10000, 0x10000, CRC(e80f7fed) SHA1(62d958d98c95e9e4d1b290a6c1054ae98770f276), ROM_BIOS(9) )
	// 10: same as BIOS '10' in at
	ROM_SYSTEM_BIOS(10, "amip1", "AMI P.1") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "poisk-h.bin",   0x10001, 0x8000, CRC(83fd3f8c) SHA1(ca94850bbd949b97b11710629886b0ee69489a81),ROM_SKIP(1) | ROM_BIOS(10) )
	ROMX_LOAD( "poisk-l.bin",   0x10000, 0x8000, CRC(0b2ed291) SHA1(bb51a3f317cf4d429a6cfb44a46ca0ac39d9aaa7),ROM_SKIP(1) | ROM_BIOS(10) )
	// 11: BIOS-String: DG22-1131-040990-K11 / 286-BIOS G2 V1.1 6-28-90
	ROM_SYSTEM_BIOS(11, "ami1131", "AMI-1131") /*(Motherboard Manufacturer: Elitegroup Computer Co., Ltd.) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "2hlm003h.bin",   0x10001, 0x8000, CRC(2babb42b) SHA1(3da6538f44b434cdec0cbdddd392ccfd34666f06),ROM_SKIP(1) | ROM_BIOS(11) )
	ROMX_LOAD( "2hlm003l.bin",   0x10000, 0x8000, CRC(317cbcbf) SHA1(1adad6280d8b07c2921fc5fc13ecaa10e6bfebdc),ROM_SKIP(1) | ROM_BIOS(11) )
	// 12: same as BIOS '1' in at
	ROM_SYSTEM_BIOS(12, "at", "PC 286") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 03-11-1987)*/
	ROMX_LOAD( "at110387.1", 0x10001, 0x8000, CRC(679296a7) SHA1(ae891314cac614dfece686d8e1d74f4763cf40e3),ROM_SKIP(1) | ROM_BIOS(12) )
	ROMX_LOAD( "at110387.0", 0x10000, 0x8000, CRC(65ae1f97) SHA1(91a29c7deecf7a9afbba330e64e0eee9aafee4d1),ROM_SKIP(1) | ROM_BIOS(12) )
	// 13
	ROM_SYSTEM_BIOS(13, "bravo", "AST Bravo/286") // fails with keyboard controller test, probably expects specific kbdc rom
	ROMX_LOAD( "107000-704.bin", 0x10000, 0x8000, CRC(94faf87e) SHA1(abaafa6c2ae9b9fba95b244dcbcc1c752ac6c0a0),ROM_SKIP(1) | ROM_BIOS(13) )
	ROMX_LOAD( "107000-705.bin", 0x10001, 0x8000, CRC(e1263c1e) SHA1(b564f1043ef45ecbdf4f06bb500150ad992c2931),ROM_SKIP(1) | ROM_BIOS(13) )
ROM_END

// Chips & Technologies CS8221 NEAT chipset: P82C211 + P82C212 + P82C215 + P82C206
ROM_START( neat )
	ROM_REGION(0x20000,"bios", 0)
	// 0: BIOS-String: ENET-1131-030389-K0
	ROM_SYSTEM_BIOS(0, "neat286", "NEAT 286")
	ROMX_LOAD( "at030389.0", 0x10000, 0x8000, CRC(4c36e61d) SHA1(094e8d5e6819889163cb22a2cf559186de782582),ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "at030389.1", 0x10001, 0x8000, CRC(4e90f294) SHA1(18c21fd8d7e959e2292a9afbbaf78310f9cad12f),ROM_SKIP(1) | ROM_BIOS(0))
	// 1: Phoenix 80286 ROM BIOS PLUS Version 3.10 12 - High Performance 286 ROM BIOS Ver C.12
	ROM_SYSTEM_BIOS(1, "pb800", "Packard Bell PB800")
	ROMX_LOAD( "3.10.12-1.bin", 0x10001, 0x8000, CRC(e6bb54c5) SHA1(fa5a376dd44696c78dcc8994e18938b5e1b3e45a),ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "3.10.12-2.bin", 0x10000, 0x8000, CRC(bde46933) SHA1(c7221192f48d6f2f5b773c3c7d2a52b635cb473e),ROM_SKIP(1) | ROM_BIOS(1))
	// 2: DTK Corp. 286 Computer - DTK 286 Chipset ROM BIOS Version 3.26 - #24062890N
	ROM_SYSTEM_BIOS(2, "ptm1632c", "UNIT PTM1632C DTK V.3.26")
	ROMX_LOAD( "ptm1632c_l.bin", 0x10000, 0x8000, CRC(df0bc27c) SHA1(f94e2decd13c285c23b6a61c035cab88fa00ba6e), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "ptm1632c_h.bin", 0x10001, 0x8000, CRC(a80136e0) SHA1(5edc2d387efb42cf70361197de808ce1b06d8aec), ROM_SKIP(1) | ROM_BIOS(2))
	// 3: BIOS-String: DTK Corp. 286 COMPUTER - (C) DTK NEAT BIOS Ver 3.25N2 06/06/89 - DTK PTM-1233C - Chipset: P82C211; P82C212B; P82C215 - BIOS: dtk 286E 8864 - IS8: 3 - ISA16: 5
	ROM_SYSTEM_BIOS(3, "ptm1233c", "DTK PTM-1233C")
	ROMX_LOAD( "286-dtk ptm-1233c-low_32k.bin", 0x10000, 0x8000, CRC(8909164c) SHA1(51978929a690746c1956ca6b1f0412777dc5d35b), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "286-dtk ptm-1233c-high_32k.bin", 0x10001, 0x8000, CRC(9105968c) SHA1(737d4df8040655315a648fed8a8d574f39e7dc35), ROM_SKIP(1) | ROM_BIOS(3) )
ROM_END


//**************************************************************************
//  80286 motherboard
//**************************************************************************

// Octek Fox II - Chipset: Headland HT12/A - BIOS String:
ROM_START( o286foxii)
	ROM_REGION(0x20000, "bios", 0)
	ROMX_LOAD( "286-fox2-even_32k.bin", 0x10000, 0x8000, CRC(54dc119f) SHA1(4bc543beef0d2201fa20eac90a0a6ca38ebf0dbf), ROM_SKIP(1))
	ROMX_LOAD( "286-fox2-odd_32k.bin", 0x10001, 0x8000, CRC(e5db7775) SHA1(2bd0572b9f7c76eff51375b551586ca8484e2a74), ROM_SKIP(1))
ROM_END

// Unknown 80C286 motherboard (4 SIMM, 6 16-bit ISA)
// SARC RC2015; HM6818P; 82C042 or JETkey Keyboard BIOS; 1MB onboard RAM (8x LH64256AD-80)
ROM_START( sarcpc )
	ROM_REGION(0x20000,"bios", 0) // 27C512
	// BIOS-String: 20-0300-00834-00101111-050591-SARC286 / [80286 Standard System 2V1]
	//ROM_SYSTEM_BIOS(0, "sarcrev12", "SARC Rev. 1.2")
	ROM_LOAD( "sarcrev12.bin", 0x10000, 0x10000, CRC(1c5e3f2d) SHA1(1fcc8b1b9d9383467223dd41e420f9352beca654) )
ROM_END

// Toptek 286 Turbo (board name somewhat uncertain; 5x 8-bit ISA, 3x 16-bit ISA, 2 banks of onboard RAM + 2 banks expansion RAM)
// VLSI VL82C100 + VL82C101B + VL82C102A + VL82C103A + VL82C104; MC146818 or HM6818P RTC; unidentified keyboard controller
ROM_START( toptek286 )
	ROM_REGION(0x20000, "bios", 0)
	// BIOS-String: D286-1295-091589-K0
	// Original BIOS64 dump split into even and odd bytes based on available PCB info
	ROM_LOAD16_BYTE( "toptek_vlsi_even.bin", 0x10000, 0x8000, CRC(f35465e8) SHA1(c85afc2168e355120c63b68d5c11fce7770fe1b7) )
	ROM_LOAD16_BYTE( "toptek_vlsi_odd.bin", 0x10001, 0x8000, CRC(b7272729) SHA1(686c976b9b7989862846a79d00f1f9116f03bc17) )
ROM_END

// Everex EV-1806 (6 16-bit ISA, 1 8-bit ISA)
// Everex IC-00121-0 + IC-00122-0; CHIPS P82C206; Intel 8272A(?); 146818A RTC
ROM_START( ev1806 )
	ROM_REGION(0x20000, "bios", 0)
	ROMX_LOAD( "everex_ev-1806_rev-f1a-21_even_u62.bin", 0x18000, 0x4000, CRC(7364e49b) SHA1(e8f5f41514005da0e36792e009cf3eae51c19c20), ROM_SKIP(1) )
	ROMX_LOAD( "everex_ev-1806_rev-f1a-21_odd_u61.bin", 0x18001, 0x4000, CRC(05c87bf7) SHA1(8c2243d9ee3d2af1517dc1134a22a7d1ed11262f), ROM_SKIP(1) )
ROM_END

// Unknown motherboard (similar layout to LM-103S; 4 SIMM, 5 16-bit ISA, 2 8-bit ISA)
// Headland HT12/A; HM6818A RTC; AMI K053770 keyboard BIOS
// Jumpers at right edge of board are labeled "KEYLOCK" (J6), "SPEAKER" (J7), "TURBO LED" (J8), "TURBO S.W." (J9), "RESET" (J10)
// XTALs X3 and X4 in top right corner (behind 80C287) are both unpopulated
ROM_START( ht286 )
	ROM_REGION(0x20000, "bios", 0)
	//ROM_SYSTEM_BIOS(0, "dh12-k0", "AMI DH12-K0")
	// BIOS-String: DH12-1343-061390-K0
	// Original BIOS64 dump split into even and odd bytes (matches another dump of the same BIOS)
	// ROM at U6 has sticker with AMI 253770 label; "BB012" at U8 is probably other half of BIOS, though not clear which half is which
	ROM_LOAD16_BYTE( "286_headland_even.bin", 0x10000, 0x8000, CRC(a2530914) SHA1(1aca289240caa6d4bf811d301c338c157b6902a1) )
	ROM_LOAD16_BYTE( "286_headland_odd.bin", 0x10001, 0x8000, CRC(b5f69002) SHA1(ee9ceef1fc7a328ee82006cd504e72e16f21b3c8) )
ROM_END

// KT216WB5-HI Rev.2 (1 8-bit ISA, 5 16-bit ISA)
// Winbond W83C201P + W83C202AP + W83C203AP + W83C204P + W83C205AP; MC146818AP RTC; JETkey keyboard BIOS
ROM_START( kt216wb5 )
	ROM_REGION(0x20000, "bios", 0)
	// BIOS-String: D286-1149-083090-K0 - WIN 286 BIOS
	ROMX_LOAD( "kt216wb5_even.bin", 0x10000, 0x8000, CRC(6b5509c0) SHA1(73b303b90cc0cd23b7e13362019193c938a2e502), ROM_SKIP(1) )
	ROMX_LOAD( "kt216wb5_odd.bin", 0x10001, 0x8000, CRC(af541ada) SHA1(26d2617dbe8c15f1b0d4782375bcb291a7923703), ROM_SKIP(1) )
ROM_END

// LM-103S (1 8-bit ISA, 6 16-bit ISA, 4 memory slots)
// Headland Technology G2 chipset: HT101A + 2x HT102; HM6818P RTC; AMI keyboard BIOS
ROM_START( lm103s )
	ROM_REGION(0x20000, "bios", 0)
	// BIOS-String: D286-1234-121589-K0
	ROMX_LOAD( "ami_lm103-s_lo.bin", 0x10000, 0x8000, CRC(a24be20b) SHA1(ffc5faf6d773154bf2f037556d2e381e81a28a58), ROM_SKIP(1) )
	ROMX_LOAD( "ami_lm103-s_hi.bin", 0x10001, 0x8000, CRC(7b63e60c) SHA1(da78b95b12051b6d4701a412fdc5e7874595c188), ROM_SKIP(1) )
ROM_END

// Magictronic B233 (8 ISA slots)
// SUNTAC Chipset, http://toastytech.com/manuals/Magitronic%20B233%20Manual.pdf
ROM_START( magb233 )
	ROM_REGION(0x20000, "bios", 0)
	// BIOS-String: DSUN-1105-043089-K0
	ROMX_LOAD( "magitronic_b233_ami_1986_286_bios_plus_even_sa027343.bin", 0x10000, 0x8000, CRC(d4a18444) SHA1(d95242104fc9b51cf26de72ef5b6c52d99ccce30), ROM_SKIP(1) )
	ROMX_LOAD( "magitronic_b233_ami_1986_286_bios_plus_odd_sa027343.bin", 0x10001, 0x8000, CRC(7ac3db56) SHA1(4340140450c4f8b4f6a19eae50a5dc5449edfdf6), ROM_SKIP(1) )
	// ROM_LOAD("magitronic_b233_ami_1986_keyboard_bios_plus_a025352.bin", 0x0000, 0x1000), CRC(84fd28fd) SHA1(43da0f49e52c921844e60b6f3d22f2a316d865cc) )
ROM_END

// MAT286 REV.D (5 16-bit ISA, 1 8-bit ISA, 2 memory slots)
// Headland Technology HT12P-16/A; HM6818P RTC; JETkey keyboard BIOS; unmarked 40-pin DIP
ROM_START( mat286 )
	ROM_REGION(0x20000, "bios", 0)
	// BIOS-String: DH12-1112-061390-K0 - HT-12 286 BIOS
	// Files separated from single BIOS64 dump (PCB photo shows split ROMs are used)
	ROMX_LOAD( "9221fkf_imp23256_ami-l.bin", 0x10000, 0x08000, CRC(55deb5c2) SHA1(19ce1a7cc985b5895c585e39211475de2e3b0dd1), ROM_SKIP(1) )
	ROMX_LOAD( "9221gjf_imp23256_ami-h.bin", 0x10001, 0x08000, CRC(04a2cec4) SHA1(564d37a8b2c0f4d0e23cd1e280a09d47c9945da8), ROM_SKIP(1) )
ROM_END


//**************************************************************************
//  80286 Desktop
//**************************************************************************

// Kaypro 286i
ROM_START( k286i )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "81_1598", 0x18000, 0x4000, CRC(e25a1e43) SHA1(d00b976ac94323f3867b1c256e315839c906dd5a) )
	ROM_LOAD16_BYTE( "81_1599", 0x18001, 0x4000, CRC(08e2a17b) SHA1(a86ef116e82eb9240e60b52f76e5e510cdd393fd) )
ROM_END

// Sanyo MBC-28
ROM_START( mbc28 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "mbc-28_sl-dt_ver.1620_low_din_checksum,454f00,27c256-15.bin", 0x10000, 0x8000, CRC(423b4693) SHA1(08e877baa59ebd9a1817dcdd27138c638edcbb84) )
	ROM_LOAD16_BYTE( "mbc-28_sl-dt_ver.1620_high_din_checksum,45ae00,27c256-15.bin", 0x10001, 0x8000, CRC(557b7346) SHA1(c0dca88627f8451211172441fefb4020839fb87f) )
ROM_END

// Siemens PCD-2
	// ROM_LOAD( "vga_nmc27c256q_435-0029-04_1988_video7_arrow.bin", 0x8000, 0x0800, CRC(0d8d7dff) SHA(cb5b2ab78d480ec3164d16c9c75f1449fa81a0e7) ) // Video7 VGA card
	// ROM_LOAD( "vga_nmc27c256q_435-0030-04_1988_video7_arrow.bin", 0x8000, 0x0800, CRC(0935c003) SHA(35ac571818f616b856da8bbf6a7a9172f68b3ab6) )
ROM_START( pcd2 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "bios_tandon_188782-032a_rev_5.21_low.bin", 0x10000, 0x8000, CRC(a8fbffd3) SHA1(8a3ad5bc7f86ff984be10a8b1ae4542be4c80e5f) )
	ROM_LOAD16_BYTE( "bios_tandon_188782-031a_rev_5.21_high.bin", 0x10001, 0x8000, CRC(8d7dfdcc) SHA1(d1d58c0ad7db60399f9a93db48feb10e44ffd624) )

	ROM_REGION( 0x0800, "keyboard", 0 ) // reporting keyboard controller failure
	ROM_LOAD( "kbd_8742_award_upi_1.61_rev_1.01.bin", 0x000, 0x800, CRC(bb8a1979) SHA1(43d35ecf76e5e8d5ddf6c32b0f6f628a7542d6e4) ) // 8742 keyboard controller
ROM_END

// Compaq SLT/286 - complains about "102 - System board failure"
ROM_START( comslt286 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "compaq_slt286-bios-revision_j.2-even.bin", 0x10000, 0x8000, CRC(77e894e0) SHA1(e935e62e203ec67eaab198c15a36cc0078fd35b0))
	ROM_LOAD16_BYTE( "compaq_slt286-bios-revision_j.2-odd.bin", 0x10001, 0x8000, CRC(4a0febac) SHA1(7da5ac4bc50f25063a1d1e382b8cff9b297976f8))
ROM_END

// Dell System 200 - complains about "memory overlap at 400000" but seems to work otherwise
ROM_START( dsys200 )
	ROM_REGION( 0x20000, "bios", 0 )
	ROM_LOAD16_BYTE( "version_3.10_a12_even.bin", 0x10000, 0x8000, CRC(5aa81939) SHA1(d9029d3708c49e72f57ae2a340429c28ec39acab))
	ROM_LOAD16_BYTE( "version_3.10_a12_odd.bin", 0x10001, 0x8000, CRC(942416cb) SHA1(b321704471e159030af82556ff25ac46c27a807e))

	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "20575_b47-00.bin", 0x000, 0x0800, CRC(148187db) SHA1(0d7542dd0b2bc3d6724ae3618a8543cb84a30e92) )
ROM_END

//  NCR, probably PC-8 - should get a "NGA" extended CGA graphics card once it's emulated
ROM_START( ncrpc8 )
	ROM_REGION(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "pc8main", "NCR PC-8 mainboard") // large full size AT mainboard - Setup Version 2.3
	ROMX_LOAD( "ncr_35117_u127_vers.4-2.bin", 0x10000, 0x8000, CRC(f4338669) SHA1(c1d6e714591c8d7ab966acfdbc3b463e06fbd073), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "ncr_35116_u113_vers.4-2.bin", 0x10001, 0x8000, CRC(b1b6a2e2) SHA1(5b2c0a2be59e064076ed757d84f61bf955ceca08), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "pc8card", "NCR PC-8 CPU card") // passive backplane and CPU card - Setup Version 2.1
	ROMX_LOAD( "ncr_u127-30_v.4.bin", 0x10000, 0x8000, CRC(33121525) SHA1(11f8d8af4dad432f558c646d7d0ff23eb642a815), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "ncr_u113-27_v.4.bin", 0x10001, 0x8000, CRC(87424492) SHA1(5b7aba5678fe55c81fee2e07730b8ae03a23160f), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD ("ncr_keyboard_mcu_35091.bin", 0x0000, 0x800, CRC(632556cc) SHA1(b35f30bd0664fc1c2775a594f248d1e30237900a))
ROM_END

// Nixdorf 8810 M30
ROM_START( n8810m30 )
	ROM_REGION(0x20000, "bios", 0 )
	ROM_LOAD( "at286bios_53889.00.0.17jr.bin", 0x00000, 0x20000, CRC(74870212) SHA1(adb3f379c9aeee6a5beb946d23af6eea706aca1d) )
ROM_END

// Nixdorf 8810 M55
ROM_START( n8810m55 )
	ROM_REGION(0x20000, "bios", 0 )
	ROM_LOAD16_BYTE( "150-3872_u113_27_4.5.1.bin", 0x10001, 0x8000, CRC(35ff4fba) SHA1(557f0f98c27af76f6fa6990592e7150f5fc1fc02))
	ROM_LOAD16_BYTE( "150-3873_u127_30_4.5.1.bin", 0x10000, 0x8000, CRC(5a7e6643) SHA1(f3890919a772eead7232bd227b2c8677377f6e24))
ROM_END

// Olivetti M290 - has an Olivetti branded Paradise VGA - locks up with "Error 2" and a key symbol
ROM_START( m290 )
	ROM_REGION( 0x20000, "bios", 0 )
	ROM_LOAD( "m290_pep3_1.25.bin", 0x10000, 0x10000, CRC(cb57d677) SHA1(4bdf5c52567c129b413c866c63b5fb3562fccd23))

	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "m290_csl0_1.10.bin", 0x000, 0x0800, CRC(d767d496) SHA1(84246f7b39e0a005425948931cf93624b831e121) )
ROM_END

// Ericsson WS286
ROM_START( ews286 ) // Computer is brown/yellow-ish with Ericsson logo
	ROM_REGION(0x20000, "bios", 0)
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

// Compaq Portable II
ROM_START( comportii )
	ROM_REGION(0x20000,"bios", 0)
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
ROM_START( comportiii )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "106779-002", "106779-002")
	ROMX_LOAD( "cpiii_87c128_106779-002.bin", 0x18000, 0x4000, CRC(aef8f532) SHA1(b0374d5aa8766f11043cbaee007e6d311f792e44), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "cpiii_87c128_106778-002.bin", 0x18001, 0x4000, CRC(c259f628) SHA1(df0ca8aaead617114fbecb4ececbd1a3bb1d5f30), ROM_SKIP(1) | ROM_BIOS(0) )
	// ROM_LOAD( "cpiii_106436-001.bin", 0x0000, 0x1000, CRC(5acc716b) SHA(afe166ecf99136d15269e44ebf2d66317945bf9c) ) // keyboard
	ROM_SYSTEM_BIOS(1, "109737-002", "109737-002")
	ROMX_LOAD( "109738-002.bin", 0x10000, 0x8000, CRC(db131b8a) SHA1(6a8517a771272edf16870501fc1ed94c7555ef45), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "109737-002.bin", 0x10001, 0x8000, CRC(8463cc41) SHA1(cb9801591e4a2cd13bbcc40739c9e675ba84c079), ROM_SKIP(1) | ROM_BIOS(1) )
ROM_END

// Nixdorf 8810 M15 Laptop - PC07 - boot from harddisk doesn't work
ROM_START( n8810m15 )
	// ROM_LOAD("charagene_v1.1_daft2c2.bin", 0x00000, 0x4000, CRC(dd324efd) SHA1(67fd91277733596bfad8506dc92d9f776e563dda)) // CGA chargen

	ROM_REGION(0x20000, "bios", 0 )
	ROM_LOAD16_BYTE( "rbios_even_daft2a3.bin", 0x10000, 0x8000, CRC(790abf68) SHA1(fbdb5e628ee9a605c8c1485a3fbb67736ff03153))
	ROM_LOAD16_BYTE( "rbios_odd_daft2b3.bin", 0x10001, 0x8000, CRC(b09a812a) SHA1(c1b3321715260f9cd8c810325dc10c674ea05174))
ROM_END

// Nixdorf 8810 M16 Laptop - PC17 - CGA version - boot from harddisk doesn't work
ROM_START( n8810m16c )
	// ROM_LOAD("201cg rev 1.0.u78", 0x00000, 0x4000, CRC(3e31143b) SHA1(489da357e0ab8a469a3fb81cce160637486c87bc)) // CGA chargen
	ROM_REGION(0x20000, "bios", 0 )
	ROM_LOAD16_BYTE( "nmc27c256.u35", 0x10000, 0x8000, CRC(51acd116) SHA1(1a0bf24af4eba48d0deb0132a523e131902d2bcd))
	ROM_LOAD16_BYTE( "nmc27c256.u36", 0x10001, 0x8000, CRC(fb47f9da) SHA1(d9bd4aea850a83764454a5c86c8da09f7c640fd6))
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "d8749h.u69", 0x000, 0x0800, CRC(030051da) SHA1(91b60228452cd1d6af99786402bd3b4d3efc2f05) )
ROM_END

// Nixdorf 8810 M16 Laptop - PC17 - VGA version - boot from harddisk doesn't work
ROM_START( n8810m16v )
	// ROM_LOAD("8810m16vga_27c256_221vb_123g1.bin", 0x00000, 0x4000, CRC(3bc80739) SHA1(3d6d7fb01681eccbc0b560818654d5aa1e3c5230)) // C&T VGA BIOS for 82C455
	ROM_REGION(0x20000, "bios", 0 )
	ROM_LOAD16_BYTE( "8810m16vga_27c256_286bios_a2531511_a.bin", 0x10000, 0x8000, CRC(1de5e49b) SHA1(759878e13801278de96700bbef318a49cca68054))
	ROM_LOAD16_BYTE( "8810m16vga_27c256_286bios_a2531511_b.bin", 0x10001, 0x8000, CRC(a65cf1f8) SHA1(30d46b49e87f272540e24a278848122b3c40bdaf))
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "8810m16vga_8749_201kb_rev3a.bin", 0x000, 0x0800, CRC(030051da) SHA1(91b60228452cd1d6af99786402bd3b4d3efc2f05) )
ROM_END


//**************************************************************************
//  80386 SX and DX BIOS
//**************************************************************************

ROM_START( at386sx )
	ROM_REGION(0x20000,"bios", 0)
	// 0: BIOS-String: 30-0100-D61204-00101111-050591-SCAMPSX-0 / MB-1316/20/25VST
	ROM_SYSTEM_BIOS(0, "mb386sx", "mb386sx-25spb") // VLSI SCAMPSX
	ROMX_LOAD( "386sx_bios_plus.bin", 0x10000, 0x10000, CRC(f71e5a8d) SHA1(e73fda2547d92bf578e93623d5f2349b97e22393), ROM_BIOS(0))
	// 1: BIOS-String: 30-0400-428027-00101111-070791-SCMPSX-0 / VLSI SCAMP 386SX 16/20/25MHz
	ROM_SYSTEM_BIOS(1, "scamp", "VLSI SCAMPSX")
	ROMX_LOAD( "ami_386sx_vlsi_scamp_070791.bin", 0x10000, 0x10000, CRC(082d071c) SHA1(69af9a951f138146036b3c9ac3761cc6589b6cf5), ROM_BIOS(1))
	// 2: NCR 386 CPU card
	ROM_SYSTEM_BIOS(2, "ncr386sx", "NCR 386sx card") // Upgrade card for e.g. NCR PC-8 - Setup Version 2.7.1
	ROMX_LOAD( "ncr_386sx_u12-19_7.3.bin", 0x10001, 0x8000, CRC(9e4c9a2a) SHA1(0a45d9f04f03b7ae39734916af7786bc52e5e917), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "ncr_386sx_u46-17_7.3.bin", 0x10000, 0x8000, CRC(73ad83a2) SHA1(bf6704fb4a0da37251f192cea3af2bc8cc2e9cdb), ROM_SKIP(1) | ROM_BIOS(2))
	// ROM_LOAD( "ncr_386sx_card_150-0004508_u1_v1.1.bin", 0x0000, 0x800, CRC(dd591ac1) SHA1(5bc40ca7340fa57aaf5d707be45a288f14085807))
	// 3: BIOS-String: 30-0501-D81105-00101111-070791-ACER1217-0 - 386SX NPM/33,40-A0(2) 05/12/1993
	ROM_SYSTEM_BIOS(3, "m919a00", "PC-Chips M919")
	ROMX_LOAD( "m919a00_npm-40.bin", 0x10000, 0x10000, CRC(4f330d82) SHA1(08224c7bcfb2a859b682bf44ac1ac7fd9f2ade78),ROM_BIOS(3))
ROM_END

ROM_START( ct386sx )
	ROM_REGION(0x20000,"bios", 0)
	// 0: BIOS-String: ENSX-1131-0040990-K0
	ROM_SYSTEM_BIOS(0, "neatsx", "NEATsx 386sx")
	ROMX_LOAD( "012l-u25.bin", 0x10000, 0x8000, CRC(4ab1862d) SHA1(d4e8d0ff43731270478ca7671a129080ff350a4f),ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "012h-u24.bin", 0x10001, 0x8000, CRC(17472521) SHA1(7588c148fe53d9dc4cb2d0ab6e0fd51a39bb5d1a),ROM_SKIP(1) | ROM_BIOS(0))
	ROM_FILL(0x1e2c9, 1, 0x00) // skip incompatible keyboard controller test
	ROM_FILL(0x1e2cb, 1, 0xbb) // fix checksum
ROM_END

ROM_START( at386 )
	ROM_REGION(0x20000,"bios", 0)
	// 0: BIOS-String: X0-0100-000000-00101111-060692-386SX-0 / AMIBIOS Ver 5.19a
	ROM_SYSTEM_BIOS(0, "ami386", "AMI 386")
	ROMX_LOAD( "ami386.bin",  0x10000, 0x10000, CRC(3a807d7f) SHA1(8289ba36a3dfc3324333b1a834bc6b0402b546f0), ROM_BIOS(0))
	// 1: Phoenix 80386 ROM BIOS PLUS Verson 1.10 (R22)
	ROM_SYSTEM_BIOS(1, "at386", "unknown 386")  // This dump possibly comes from a MITAC INC 386 board, given that the original driver had it as manufacturer
	ROMX_LOAD( "at386.bin",  0x10000, 0x10000, CRC(3df9732a) SHA1(def71567dee373dc67063f204ef44ffab9453ead), ROM_BIOS(1))
	// 2: BIOS-String: 30-0101-429999-00101111-050591-D90-0 / AMI TD60C BIOS VERSION 2.42B
	ROM_SYSTEM_BIOS(2, "amicg", "AMI CG")
	ROMX_LOAD( "amicg.1",        0x10000, 0x10000,CRC(8408965a) SHA1(9893d3ac851e01b06a68a67d3721df36ca2c96f5), ROM_BIOS(2))
	// 3: Award Modular BIOS 4.50
	ROM_SYSTEM_BIOS(3, "pccm326", "PCChips M326 V5.2") //  BIOS reports a 66MHz 386DX original board has a TI TX486DLC/E-40PCE and IIT 4C87DLC-40 CPU/FPU combo
	ROMX_LOAD( "m326_v5.2_m601-326.bin", 0x10000, 0x10000, CRC(cca6a443) SHA1(096c8bfa000c682d6c801da27c7fd14243ebb63b), ROM_BIOS(3) )
	// 4: BIOS-String: 40-0100-001437-001001111-080893-4386-0 / Release 10/01/93.
	ROM_SYSTEM_BIOS(4, "m326r53", "PC-Chips M326 Rev. 5.3")
	ROMX_LOAD( "m326_rev.5.3.bin", 0x10000, 0x10000, CRC(6c156064) SHA1(362ce5a2333641083706a878b807ab87537ca1e6), ROM_BIOS(4) )
	// 5:
	ROM_SYSTEM_BIOS(5, "msi386", "MSI 386") // MSI 386 mainboard, initializes graphics card, then hangs
	ROMX_LOAD( "ami_386_msi_02297_even.bin", 0x10000, 0x8000, CRC(768590a0) SHA1(90c5203d78591a093fd4f54ceb8d9827f1e64f39), ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "ami_386_msi_02297_odd.bin", 0x10001, 0x8000, CRC(7b1360dc) SHA1(552ccda9f90826621e88d9abdc47306b9c2b2b15), ROM_SKIP(1) | ROM_BIOS(5) )
	// 6: BIOS-String: 20-0200-DF1121-00101111-102591-SYM_386B-0 / 386DX/SX (S1A.P)
	ROM_SYSTEM_BIOS(6, "386sc2c", "386-SC-2C")
	ROMX_LOAD("386-sc-2c_ami_za492668.bin", 0x10000, 0x10000, CRC(b408eeb7) SHA1(cf1974492119e1aae623fa366d5760343e827e52), ROM_BIOS(6) )
	// 7: BIOS-String: EC&T-1332-040990-K0
	ROM_SYSTEM_BIOS(7, "cs8230", "CS8230")
	ROMX_LOAD( "ami_386_cs8230_chipset.bin", 0x10000, 0x10000, CRC(1ee766d0) SHA1(75dba3c9817dfe6caca46f5f4f2f1d76ba88d3c7), ROM_BIOS(7) )
	// 8
	ROM_SYSTEM_BIOS(8, "ami2939", "AMI2939") // no display
	ROMX_LOAD( "ami2939e.rom", 0x10000, 0x8000, CRC(65cbbd32) SHA1(d7d26b496f8e86f01722ad9f171a68f9fcdc477c), ROM_SKIP(1) | ROM_BIOS(8) )
	ROMX_LOAD( "ami2939o.rom", 0x10001, 0x8000, CRC(8db6e739) SHA1(cdd47709d6036fad4be40c15bff41752d831d4b8), ROM_SKIP(1) | ROM_BIOS(8) )
	// 9: BIOS-String: 30-0201-ZZ1347-00101111-050591-M320-0
	ROM_SYSTEM_BIOS(9, "m321_23", "PCChips M321 Rev.2.3")
	ROMX_LOAD( "pcchips_m321_rev2.3.bin", 0x10000, 0x10000, CRC(ca0542e4) SHA1(8af9f88e022f8115708178c6c0b313ea0423a2b5), ROM_BIOS(9) )
	// 10: BIOS-String: 30-0200-DH1102-00101111-070791-ETEQ386-0 / REV C3
	ROM_SYSTEM_BIOS(10, "sy19i", "Soyo SY-19I")
	ROMX_LOAD( "ami_soyo_sy-19i.bin", 0x10000, 0x10000, CRC(369a040f) SHA1(3dbcbcb8b8a50717cae3b17f44ca1b7c394b75fc), ROM_BIOS(10) )
	// 11: NCR 386 slot CPU - Upgrade card for e.g. NCR PC-8 - set graphics card to CGA to see a "Timer One Error" message
	ROM_SYSTEM_BIOS(11, "ncr386", "NCR 386 CPU card")
	ROMX_LOAD( "ncr_386_card_04152_u44_ver5.0.bin", 0x10000, 0x10000, CRC(80e44318) SHA1(54e1d4d646a577c53c65b2292b383ed6d91b65b2), ROM_BIOS(11))
	// ROM_LOAD ("ncr_386_card_keyboard_04181_u27_ver5.6.bin", 0x0000, 0x800, CRC(6c9004e7) SHA1(0fe77f47ff77333d1ff9bfcf8d6d92193ab1f208))
	// 12: BIOS-String: Phoenix 80386 ROM BIOS PLUS Version 1.10 22
	ROM_SYSTEM_BIOS(12, "cbm386", "Commodore 386")
	ROMX_LOAD( "cbm-386-bios-lo-v1.022e-8100.bin", 0x10000, 0x8000, CRC(a054a1b8) SHA1(d952b02cc10534325c1c5aaa8b6dfb77bc20a179), ROM_SKIP(1) | ROM_BIOS(12))
	ROMX_LOAD( "cbm-386-bios-hi-v1.022e-d100.bin", 0x10001, 0x8000, CRC(b9541f3d) SHA1(e37c704521e85b07369d21b0521f4d1871c318dd), ROM_SKIP(1) | ROM_BIOS(12))
	// 13: flashing "K/B controller incorrect"
	ROM_SYSTEM_BIOS(13, "gmb386sat", "GMB-386SAT_V1.0")
	ROMX_LOAD( "gmb-386sat_v1.0.bin", 0x10000, 0x10000, CRC(59ecc773) SHA1(f2007fce76b3a91f51bfb5f43c1539d5ae06d35f), ROM_BIOS(13))
	// 14: BIOS-String: X0-0100-000000-00101111-060692-RC2018A-0 / Ver 1.4b / Texas Instruments 486 DLC [S3Q]
	ROM_SYSTEM_BIOS(14, "ti486dlc", "TI 486DLC") // board is equipped with a TI486DLC
	ROMX_LOAD( "ti_486dlc_rev.s3q.bin", 0x10000, 0x10000, CRC(39b150ed) SHA1(5fc96c6232dd3a066349d8e707e938af55893297), ROM_BIOS(14))
	// 15: BIOS-String: 40-0400-001107-00101111-111192-U4800VLX-0 / UNICHIP BIOS VER 2.0A 09/27/1993
	ROM_SYSTEM_BIOS(15, "386bw367", "386BW367 Rev. 1.0")
	ROMX_LOAD( "unichip_386bw_367_rev_1.0.bin", 0x10000, 0x10000, CRC(7922a8f9) SHA1(785008e10edfd393dc39e921a12d1a07a14bac25), ROM_BIOS(15))
ROM_END


//**************************************************************************
//  80386 SX and DX motherboard
//**************************************************************************

// CX Technology, Inc. Model SXD (4x SIMM, 4x 16-bit ISA)
// SARC RC2016A5; HM6818P; CX109; LT38C41 © Lance Corp. (keyboard controller?)
ROM_START( cxsxd )
	ROM_REGION(0x20000,"bios", 0)
	// BIOS-String: 03/25/93-SARC_RC2016A-219v0000 / CX 386SX System
	ROM_LOAD( "award_cx-sxd_v4.50.srd.bin", 0x10000, 0x10000, CRC(ef1c74d7) SHA1(b40b1cb7143c4e352798bdf3b488d9342a4029a7))
ROM_END

// Chips & Technologies CS8230 chip set: P82C301C, P82C302C, P82A303, P82A304, 2x P82B305, P82A306 A, P82C206
// 8x SIMM, 5x 16-bit ISA, 2x 8-bit ISA, 1x 32-bit proprietary memory expansion slot
ROM_START( ecs38632 )
	ROM_REGION(0x20000, "bios", 0)
	// BIOS-String: EC&T-1131-030389-K0
	ROMX_LOAD( "ami_ecs-386_32_lo.bin", 0x10000, 0x8000, CRC(e119d6a4) SHA1(bcc6164173b44832b8ebfa1883e22efc167e2cd4), ROM_SKIP(1))
	ROMX_LOAD( "ami_ecs-386_32_hi.bin", 0x10001, 0x8000, CRC(e3072bf8) SHA1(74eec72e190f682cfd5ae5425ebdc854e0ba7bc9), ROM_SKIP(1))
ROM_END

// UMC UM82C482AF, UM82C391A, UM82C206F
// 8x SIMM, 8x ISA
ROM_START( ecsum386 )
	ROM_REGION(0x20000, "bios", 0)
	// BIOS-String: 30-0500-D01131-00101111-070791-UMCWB-0 / UM386 V1.1 03-06-92
	ROM_LOAD( "ami_um386_rev1.1.bin", 0x10000, 0x10000,CRC(81fe4297) SHA1(efb2ba2be6f08cb487ee1b867a2456ed6b5975ad))
ROM_END

// Chips & Technologies P82C351, P82C355, P82C356
// 6x 16-bit ISA + 2x 8-bit ISA
ROM_START( fu340 )
	ROM_REGION(0x20000, "bios", 0)
	// BIOS-String: 30-0200-D01247-00101111-050591-PEAKDM_B-0 / FU340 REV-B PAGE MODE BIOS
	ROM_LOAD( "ami_abit_fu340.bin", 0x10000, 0x10000,CRC(9ea90d90) SHA1(091bdae7b1e36ac5168823d80d5907af2a95e583))
ROM_END

// Forex FRX36C300 + FRX46C402; SiS 85C206
// 8x SIMM, 8x ISA
ROM_START( smih0107 )
	ROM_REGION(0x20000, "bios", 0)
	// BIOS-String: 30-0400-428005-00101111-070791-FORX-0 / BIOS ID SMIH0107 / IT9112
	ROM_LOAD( "ami_smih0107.bin", 0x10000, 0x10000, CRC(970bb0c0) SHA1(4a958887485f7239d25fa7b0c98569b97ce93800))
ROM_END


//**************************************************************************
//  80386 SX and DX Laptop/Notebook
//**************************************************************************

// Siemens-Nixdorf PCD-3Nsx notebook
ROM_START( pcd3nsx )
	ROM_REGION( 0x20000, "bios", 0 )
	// Phoenix 80386 ROM BIOS PLUS Version 1.10.00 - failure at 100000-10FFFF - Resume memory backup failure
	ROM_SYSTEM_BIOS(0, "pcd3nsxno1", "pcd3nsxno1")
	ROMX_LOAD( "3n102l30.bin", 0x00000, 0x20000, CRC(02384c19) SHA1(552dc41b40272027e2b031187f8ab1e1513751b9), ROM_BIOS(0) )
	// Phoenix 80386 ROM BIOS PLUS Version 1.10.00 - Memory high address failure at 100000-10FFFF - Resume memory backup failure
	ROM_SYSTEM_BIOS(1, "pcd3nsxno2", "pcd3nsxno2")
	ROMX_LOAD( "3n120l40.bin", 0x00000, 0x20000, CRC(1336dd75) SHA1(80306d85f417c51a5235ac2f02ceb58bdb51205f), ROM_BIOS(1) )
ROM_END

// Toshiba T2000SX
ROM_START( t2000sx )
	ROM_REGION( 0x20000, "bios", 0 )
	ROM_LOAD( "014d.ic9", 0x00000, 0x20000, CRC(e9010b02) SHA1(75688fc8e222640fa22bcc90343c6966fe0da87f))
ROM_END

// Triumph-Adler Walkstation 386 SX - German version of the Olivetti S20
// VLSI VL82C320 + VL82C331; DP8473V
ROM_START( walk386sx )
	ROM_REGION( 0x20000, "bios", 0 ) // contains Cirrus Logic VGA BIOS
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
	ROM_REGION( 0x20000, "bios", 0 ) // contains Cirrus Logic VGA BIOS
	ROM_LOAD( "am28f010_ctaa060125rc.bin", 0x00000, 0x20000, CRC(6cc540fe) SHA1(9853793d5433bbc5efc09c7f31c4a8a8f78d4549) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "cthj02_03_76.bin", 0x0000, 0x2000, NO_DUMP )
ROM_END


//**************************************************************************
//  80486 BIOS
//**************************************************************************

ROM_START( at486 )
	ROM_REGION(0x20000, "bios", 0)
	// 0: BIOS-String: 30-0500-ZZ1130-00101111-070791-1219-0
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
	// 6: BIOS-String: 05/06/94-VT82C486A-214L2000-00 / Version 3.15GN
	ROM_SYSTEM_BIOS(6, "ficgiovp", "FIC 486-GIO-VP 3.15GN")
	ROMX_LOAD( "giovp315.rom", 0x10000, 0x10000, CRC(e102c3f5) SHA1(f15a7e9311cc17afe86da0b369607768b030ddec), ROM_BIOS(6))
	// 7: BIOS-String: 11/20/94-VT82C486A-214L2000-00 / Version 3.06G (11/25/94)
	ROM_SYSTEM_BIOS(7, "ficgiovt", "FIC 486-GIO-VT 3.06G") /* 1994-11-20 */
	ROMX_LOAD( "306gcd00.awd", 0x10000, 0x10000, CRC(75f3ded4) SHA1(999d4b58204e0b0f33262d0613c855b528bf9597), ROM_BIOS(7))
	// 8: BIOS-String: 07/06/VT82C486A-214L2000-00 / Version  3.26G
	ROM_SYSTEM_BIOS(8, "ficgiovt2_326", "FIC 486-GIO-VT2 3.26G")
	ROMX_LOAD( "326g1c00.awd", 0x10000, 0x10000, CRC(2e729ab5) SHA1(b713f97fa0e0b62856dab917f417f5b21020b354), ROM_BIOS(8))
	// 9: BIOS-String: 06/19/95-VT82C486A-214L2000-00 / Version VBS1.08H 486-GIO-VT2
	ROM_SYSTEM_BIOS(9, "486_gio_vt2","VBS1.08H 486-GVT-2")
	ROMX_LOAD( "award_486_gio_vt2.bin", 0x10000, 0x10000, CRC(58d7c7f9) SHA1(097f15ec2bd672cb3f1763298ca802c7ff26021f), ROM_BIOS(9)) // Vobis version, Highscreen boot logo
	// 10: BIOS-String: 07/17/97-VT82C486A-214L2000-00 / Version 3.276
	ROM_SYSTEM_BIOS(10, "ficgiovt2_3276", "FIC 486-GIO-VT2 3.276")
	ROMX_LOAD( "32760000.bin", 0x10000, 0x10000, CRC(ad179128) SHA1(595f67ba4a1c8eb5e118d75bf657fff3803dcf4f), ROM_BIOS(10))
	// 11: BIOS-String: 11/02/94-VT82C486A-214L2000-00 Version 3.07G
	ROM_SYSTEM_BIOS(11, "ficgvt2", "FIC 486-GVT-2 3.07G")
	ROMX_LOAD( "3073.bin",     0x10000, 0x10000, CRC(a6723863) SHA1(ee93a2f1ec84a3d67e267d0a490029f9165f1533), ROM_BIOS(11))
	// 12: BIOS-String: 06/27/95-VT82C505-2A4L4000-00 / Version 5.15S
	ROM_SYSTEM_BIOS(12, "ficgpak2", "FIC 486-PAK-2 5.15S") /* includes Phoenix S3 TRIO64 Enhanced VGA BIOS 1.4-01 */
	ROMX_LOAD( "515sbd8a.awd", 0x00000, 0x20000, CRC(778247e1) SHA1(07d8f0f2464abf507be1e8dfa06cd88737782411), ROM_BIOS(12))
	// 13
	ROM_SYSTEM_BIOS(13, "ficpio3g7", "FIC 486-PIO-3 1.15G705") /* pnp */
	ROMX_LOAD( "115g705.awd",  0x00000, 0x20000, CRC(ddb1544a) SHA1(d165c9ecdc9397789abddfe0fef69fdf954fa41b), ROM_BIOS(13))
	// 14
	ROM_SYSTEM_BIOS(14, "ficpio3g1", "FIC 486-PIO-3 1.15G105") /* non-pnp */
	ROMX_LOAD( "115g105.awd",  0x00000, 0x20000, CRC(b327eb83) SHA1(9e1ff53e07ca035d8d43951bac345fec7131678d), ROM_BIOS(14))
	// 15
	ROM_SYSTEM_BIOS(15, "ficpos", "FIC 486-POS")
	ROMX_LOAD( "116di6b7.bin", 0x00000, 0x20000, CRC(d1d84616) SHA1(2f2b27ce100cf784260d8e155b48db8cfbc63285), ROM_BIOS(15))
	// 16: BIOS-String: 06/27/95-VT82C505-2A4L4000-00 / Version 5.15
	ROM_SYSTEM_BIOS(16, "ficpvt", "FIC 486-PVT 5.15")
	ROMX_LOAD( "5150eef3.awd", 0x00000, 0x20000, CRC(eb35785d) SHA1(1e601bc8da73f22f11effe9cdf5a84d52576142b), ROM_BIOS(16))
	// 17: BIOS-String: 10/05/95-VT82C505-2A4L4000-00 / Version 5.162W2(PCTIO)
	ROM_SYSTEM_BIOS(17, "ficpvtio", "FIC 486-PVT-IO 5.162W2")
	ROMX_LOAD( "5162cf37.awd", 0x00000, 0x20000, CRC(378d813d) SHA1(aa674eff5b972b31924941534c3c988f6f78dc93), ROM_BIOS(17))
	// 18: BIOS-String: 40-0401-428003-00101111-111192-OP495SLC-0 / PX486DX33/50P3 IVN 1.0 25/06/1993
	ROM_SYSTEM_BIOS(18, "qdi", "QDI PX486DX33/50P3")
	ROMX_LOAD( "qdi_px486.u23", 0x10000, 0x10000, CRC(c80ecfb6) SHA1(34cc9ef68ff719cd0771297bf184efa83a805f3e), ROM_BIOS(18))
	// 19: BIOS-String: 40-00AG-001247-00101111-060692-SIS3486-0 / AV4 ISA/VL-BUS SYSTEM BIOS
	ROM_SYSTEM_BIOS(19, "ava4529j", "AVA4529J") // this is a board with two VLB slots
	ROMX_LOAD("amibios_486dx_isa_bios_aa4025963.bin", 0x10000, 0x10000, CRC(65558d9e) SHA1(2e2840665d069112a2c7169afec687ad03449295), ROM_BIOS(19))
ROM_END


//**************************************************************************
//  80486 motherboard
//**************************************************************************

// J-Bond A433C-C/A450C-C (8 SIMM, 2 8-bit ISA, 6 16-bit ISA)
// ETEQ ET82C491 + ET82C493; CHIPS P82C206; AMI KB-BIOS-VER-F P8042AHP
ROM_START( a433cc )
	ROM_REGION(0x20000, "bios", 0)
	// BIOS-String: 40-0200-001353-0010111-070791-ETEQ4/1C-0 / ETEQ 486 Mar. 05, 1992
	ROM_LOAD( "ami_j-bond_a433c-c.bin", 0x10000, 0x10000, CRC(66031e98) SHA1(d2d1a26837d3ca943a6ef09ec3e6fbfaaa62cc46))
ROM_END

// ASUS PVI-486AP4 (Socket 3, 4 SIMM, 4 PCI, 4 ISA, 1 VLB)
// Intel Aries PCIset S82425EX + S82426EX; DS12887 RTC; VIA VT82C42N
ROM_START( a486ap4 )
	ROM_REGION(0x20000, "bios", 0)
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

// ASUS PVI-486SP3 (Socket 3, 2 SIMM, 2 IDE, 3 PCI, 4 ISA, 1 VLB)
// SiS 85C496 + 85C497; UMC UM8669F; AMIKEY-2
ROM_START( a486sp3 )
	ROM_REGION(0x20000, "bios", 0) // Winbond W29EE011-15
	// 0: BIOS-String: 07/22/94-SATURN-P/I-4SP3-00 / #401A0-0207
	ROM_SYSTEM_BIOS(0, "v207", "ASUS PVI-486SP3 V2.07")
	ROMX_LOAD( "awsi0207.bin", 0x00000, 0x20000, CRC(0cb862aa) SHA1(7ffead05c1df47ec36afba395191145279c5e789), ROM_BIOS(0))
	// 1: BIOS-String: 07/22/94-SATURN-P/I-4SP3-00 / #401A0-0207
	ROM_SYSTEM_BIOS(1, "v2737", "ASUS PVI-486SP3 V2.07 #2")
	ROMX_LOAD( "awsi2737.bin", 0x00000, 0x20000, CRC(8cd9a89c) SHA1(6c68c23cc5e8ae66261e9fe931f2ce07efe767b6), ROM_BIOS(1))
ROM_END

// ASUS PCI/I-486SP3G V3.02 (Socket 3, 4 SIMM, 1 IDE, 1 SCSI, 3 PCI, 4 ISA)
// Intel Saturn II chipset: 82424ZX CDC + 82423TX DPU + 82378ZB SIO; NCR 53C820; National PC87332; DS12887 RTC; VIA VT82C42N
ROM_START( a486sp3g )
	ROM_REGION(0x20000, "bios", 0)
	// 0: BIOS-String: 10/21/94-SATURN-II-P/I-SP3G-00 / #401A0-302
	ROM_SYSTEM_BIOS(0, "v302", "ASUS PCI/I-486SP3G V3.02")
	ROMX_LOAD( "awsg0302.bin", 0x00000, 0x20000, CRC(21e918a0) SHA1(c7f937e3e90a43d7c7f867e686625b28a9c2484c), ROM_BIOS(0))
	// 1: BIOS-String: 08/15/95-SATURN-II-P/I-SP3G-00 / #401A0-304
	ROM_SYSTEM_BIOS(1, "v304", "ASUS PCI/I-486SP3G V3.04")
	ROMX_LOAD( "awsg0304.bin", 0x00000, 0x20000, CRC(f4d830d2) SHA1(086ccd14c7b0c521be1958d58b3539c4bfe4721f), ROM_BIOS(1))
ROM_END

// ASUS PCI/I-A486S (4 SIMM, 1 EISA)
// SiS chipset; SMC I/O; AMIKEY-2
ROM_START( aa486s )
	ROM_REGION(0x20000, "bios", 0)
	// 0: BIOS-String: 05/22/95/SiS-496-497B-PCI-A486-0-00 / #401A0-0203
	ROM_SYSTEM_BIOS(0, "v203", "ASUS PCI/I-A486S V2.03")
	ROMX_LOAD( "si4a0203.awd", 0x00000, 0x20000, CRC(95fcb7c6) SHA1(c19164d67af18c774e6eb06bd1570d95a24b2856), ROM_BIOS(0))
	// 1 boots into "boot block" rescue BIOS
	ROM_SYSTEM_BIOS(1, "v304", "ASUS PCI/I-A486S V3.04")
	ROMX_LOAD( "si4a0304.awd", 0x00000, 0x20000, CRC(a00ad907) SHA1(598d97ea29f930a9359429dc540d27bfdd0fcd20), ROM_BIOS(1))
ROM_END

// ASUS VL/EISA-486SV1 (8 EISA, 1 VLB)
ROM_START( a486sv1 )
	ROM_REGION(0x20000, "bios", 0)
	// BIOS-String: 05/20/94-SIS-486/EISA-E-486SV1-00 / #401A0-0112
	//ROM_SYSTEM_BIOS(0, "v112", "Award BIOS V1.12")
	ROM_LOAD( "e4sv0112.awd", 0x10000, 0x10000, CRC(d1d42fc9) SHA1(61549bf597517bb3c33e724e32b3cca981e65000))
ROM_END

// ASUS VL/I-486SV2G (GX4) (4 SIMM, 7 ISA, 2 VLB)
// SiS 85C471 + 85C407; AMIKEY-2
ROM_START( a486sv2g )
	ROM_REGION(0x20000, "bios", 0)
	// 0: BIOS-String: 11/17/94-SIS-85C471-I486SV2G-00 / #401A0-0304
	ROM_SYSTEM_BIOS(0, "v304", "ASUS VL/I-486SV2G (GX4) V3.04")
	ROMX_LOAD( "sv2g0304.bin", 0x10000, 0x10000, CRC(cceabe6f) SHA1(45d0e25603045255d1ccaf5cbddd1a9146f61529), ROM_BIOS(0))
	// 1: BIOS-String: 01/11/95-SIS-85C471-I486SV2G-00 / #401A0-0305-1
	ROM_SYSTEM_BIOS(1, "v305", "ASUS VL/I-486SV2G (GX4) V3.05")
	ROMX_LOAD( "0305.001", 0x10000, 0x10000, CRC(9f2f9b75) SHA1(789807d82e39d69f948f7897f99b2fe362330dd1), ROM_BIOS(1))
	// 2: BIOS-String: 08/22/95-SIS-85C471-I486SV2G-00 / #401A0-0401
	ROM_SYSTEM_BIOS(2, "v401", "ASUS VL/I-486SV2G (GX4) V4.01")
	ROMX_LOAD( "sv2g0401.bin", 0x10000, 0x10000, CRC(f544f65a) SHA1(9a5e39cfbd545a0026f959b42dbc742246205b3c), ROM_BIOS(2))
	// 3: BIOS-String: 11/03/95-SIS-85C471-I486SV2G-00 / #401A0-0402-1
	ROM_SYSTEM_BIOS(3, "v402", "ASUS VL/I-486SV2G (GX4) V4.02")
	ROMX_LOAD( "sv2g0402.bin", 0x10000, 0x10000, CRC(db8fe666) SHA1(e499da86261bc6b312a6bc3d94b9465e17c5a449), ROM_BIOS(3))
ROM_END

// Biostar MB8433UUD-A (4 SIMM, 2 IDE, 3 PCI, 4 ISA)
// UMC UM8881F, UM8886BF, UM8663AF; DS12887 RTC
ROM_START( mb8433uud )
	ROM_REGION(0x20000, "bios", 0) // Intel Flash P28F010
	ROM_LOAD( "uud0520s.bin", 0x00000, 0x20000, CRC(0e347559) SHA1(060d3040b103dee051c5c2cfe8c53382acdfedad))
ROM_END

// FIC 486-PIO-2 (4 ISA, 4 PCI)
// VIA VT82C505 (ISA/VL to PCI bridge) + VT82C496G (system chipset) + VT82C406MV (keyboard controller, RTC, CMOS), NS311/312 or NS332 I/O
ROM_START( ficpio2 )
	ROM_REGION(0x40000, "isa", 0)
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
ROM_END

// FIC 486-VIP-IO (3 ISA, 4 PCI)
// VIA GMC chipset: VT82C505 + VT82C486A + VT82C482 + VT82C483 + VT83C461 IDE; DS12885Q RTC; National PC87332VLJ-S I/O
ROM_START( ficvipio )
	ROM_REGION(0x20000, "bios", 0)
	// 0: BIOS-String: 12/07/94-VT82C505-2A4L4000-00 / Version 4.26GN2(ES2) (12/07/94)
	ROM_SYSTEM_BIOS(0, "426gn2", "FIC 486-VIP-IO 4.26GN2")
	ROMX_LOAD( "426gn2.awd",   0x00000, 0x20000, CRC(5f472aa9) SHA1(9160abefae32b450e973651c052657b4becc72ba), ROM_BIOS(0))
	// 1: BIOS-String: 02/08/96-VT82C505-2A4L4000-00 / Version 4.27GN2A (02/14/96)
	ROM_SYSTEM_BIOS(1, "427gn2a", "FIC 486-VIP-IO 4.27GN2A")
	ROMX_LOAD( "427gn2a.awd",  0x00000, 0x20000, CRC(035ad56d) SHA1(0086db3eff711fc710b30e7f422fc5b4ab8d47aa), ROM_BIOS(1))
ROM_END

// FIC 486-VIP-IO2 (3 ISA, 4 PCI)
// VIA VT82C505 + VT82C496G + VT82C406MV
ROM_START( ficvipio2 )
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD( "1164g701.awd", 0x00000, 0x20000, CRC(7b762683) SHA1(84debce7239c8b1978246688ae538f7c4f519d13))
ROM_END

// Shuttle HOT-409 (6 16-bit ISA incl. 2 VLB, 2 8-bit ISA, 8 SIMM)
// OPTi 82C495SX + 82C392SX + F82C206; MEGA-KB-1-WP
ROM_START( hot409 )
	ROM_REGION(0x20000, "bios", 0)
	// 0: BIOS-String: 40-0200-001343-00101111-111192-OPT495SX-0 / Version 2.0
	ROM_SYSTEM_BIOS(0, "hot409", "Shuttle HOT-409")
	ROMX_LOAD( "ami1992.bin", 0x10000, 0x10000, CRC(a19c3fd4) SHA1(404822c98344061b60883533395a89fe4902c177), ROM_BIOS(0))
	// 1: BIOS-String: 40-0204-001343-00101111-080893-OPT495SX-0 / OPTi495SX Version 3.0
	ROM_SYSTEM_BIOS(1, "hot409lba", "Shuttle HOT-409 with LBA")
	ROMX_LOAD( "409lba.rom", 0x10000, 0x10000, CRC(78c5e47e) SHA1(7f14a88a5548fc67dd00e73fd09745e899b93a89), ROM_BIOS(1))
ROM_END

// Siemens-Nixdorf 486 mainboards and BIOS versions
// The same mainboards were used in various case versions to get the different model lines, so an identification by the mainboard number (Dxxx) is safest
ROM_START( pcd4x )
	ROM_REGION( 0x20000, "bios", 0 )
	// D756, was used in PCD-4Lsx, contains Cirrus Logic VGA ROM
	ROM_SYSTEM_BIOS(0, "d756v320r316", "D756 BIOS V3.20 R3.16")
	ROMX_LOAD( "fts_biosupdated756noflashbiosepromv320_320316_149.bin", 0x00000, 0x20000, CRC(2ab60725) SHA1(333b64424c08ecbbaf47110c99ad0335da211489), ROM_BIOS(0) )
	// D674, was used in PCD-4M, PCD-4Msx, PCD-4RSXA/4RA
	ROM_SYSTEM_BIOS(1, "d674v320r316", "D674 BIOS V3.20 R3.16")
	ROMX_LOAD( "fts_biosupdated674noflashbiosepromv320_320316_144.bin", 0x00000, 0x20000, CRC(1293d27c) SHA1(22f36c4a5a0912011ed54ff917244f412208ffc0), ROM_BIOS(1) )
	// D802, was used in PCD-4HVL
	ROM_SYSTEM_BIOS(2, "d802v320r316", "D802 BIOS V3.20 R3.34.802")
	// PCD-4NL, contains C&T VGA BIOS
	ROMX_LOAD( "fts_biosupdated802noflashbiosepromv320_320334_152.bin", 0x00000, 0x20000, CRC(fb1cd3d2) SHA1(98043c6f0299e1c56e5f266ea5f117ae456447ff), ROM_BIOS(2) )
ROM_END


//**************************************************************************
//  80486 Laptop/Notebook
//**************************************************************************

// Siemens-Nixdorf PCD-4NL 486 subnotebook
// PhoenixBIOS(TM) A486 Version 1.03
// complains about "Pointer device failure" and "Memory failure at 00100000, read AA55 expecting 002C
ROM_START( pcd4nl )
	ROM_REGION( 0x20000, "bios", 0 )
	ROM_LOAD( "pcd4nl.bin", 0x00000, 0x20000, CRC(8adb4900) SHA1(a01c665fed769ff815bc2e5ae30901f7e12d721b) )
ROM_END

// Siemens-Nixdorf PCD-4ND 486 notebook - display remains blank
ROM_START( pcd4nd )
	ROM_REGION( 0x20000, "bios", 0 )
	ROM_SYSTEM_BIOS(0, "pcd4ndno1", "pcd4ndno1")
	ROMX_LOAD( "bf3m51.bin", 0x00000, 0x20000, CRC(6a2f90dd) SHA1(75704a83976e4bb02a028e761d01bd053cc0d4e7), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "pcd4ndno2", "pcd4ndno2")
	ROMX_LOAD( "bf3q42.bin", 0x00000, 0x20000, CRC(fa81cf6e) SHA1(91313a6856ca22f40710a6c9c8a65f8e340784ab), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "pcd4ndno3", "pcd4ndno3")
	ROMX_LOAD( "pcd-4nd_flash_28010.bin", 0x00000, 0x20000, CRC(53c0beea) SHA1(bfa17947529c51a8c9315884e156c01ddd23c0d8), ROM_BIOS(2) )
ROM_END

// LION 3500C notebook
ROM_START( lion3500 )
	ROM_REGION( 0x20000, "bios", 0 )
	ROM_LOAD( "lion3500.bin", 0x00000, 0x20000, CRC(fc409ac3) SHA1(9a7aa08b981dedffff31fda5d3496469ae2ec3a5) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME       PARENT   COMPAT   MACHINE    INPUT  CLASS         INIT            COMPANY        FULLNAME                FLAGS
COMP( 1984, ibm5170,   0,       ibm5150, ibm5170,   0,     at_state,     init_at,        "International Business Machines",  "PC/AT 5170", MACHINE_NOT_WORKING )
COMP( 1985, ibm5170a,  ibm5170, 0,       ibm5170a,  0,     at_state,     init_at,        "International Business Machines",  "PC/AT 5170 8MHz", MACHINE_NOT_WORKING )
COMP( 1985, ibm5162,   ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "International Business Machines",  "PC/XT-286 5162", MACHINE_NOT_WORKING )
COMP( 1989, ibmps1es,  ibm5170, 0,       ibmps1,    0,     at_vrom_fix_state, init_at,   "International Business Machines",  "PS/1 (Spanish)", MACHINE_NOT_WORKING )
COMP( 1987, at,        ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "<generic>",   "PC/AT (CGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1987, atvga,     ibm5170, 0,       atvga,     0,     at_state,     init_at,        "<generic>",   "PC/AT (VGA, MF2 Keyboard)" , MACHINE_NOT_WORKING )
COMP( 1988, at386,     ibm5170, 0,       at386,     0,     at_state,     init_at,        "<generic>",   "PC/AT 386 (VGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1988, ecs38632,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "Elitegroup Computer Systems",      "ECS-386/32", MACHINE_NOT_WORKING )
COMP( 1992, ecsum386,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "Elitegroup Computer Systems",      "UM386 (Rev 1.1)", MACHINE_NOT_WORKING )
COMP( 1991, fu340,     ibm5170, 0,       at386,     0,     at_state,     init_at,        "Abit",        "FU340", MACHINE_NOT_WORKING )
COMP( 19??, smih0107,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "Forex Computer Company", "unknown 386 AT clone with Forex chipset", MACHINE_NOT_WORKING )
COMP( 1988, ct386sx,   ibm5170, 0,       ct386sx,   0,     at_state,     init_at,        "<generic>",   "NEAT 386SX (VGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1988, at386sx,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "<generic>",   "PC/AT 386SX (VGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1993, cxsxd,     ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "CX Technology", "CX SXD", MACHINE_NOT_WORKING )
COMP( 1990, at486,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "<generic>",   "PC/AT 486 (VGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1989, neat,      ibm5170, 0,       neat,      0,     at_state,     init_at,        "<generic>",   "NEAT (VGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1989, ec1842,    ibm5150, 0,       ec1842,    0,     at_state,     init_at,        "<unknown>",   "EC-1842", MACHINE_NOT_WORKING )
COMP( 1993, ec1849,    ibm5170, 0,       ec1842,    0,     at_state,     init_at,        "<unknown>",   "EC-1849", MACHINE_NOT_WORKING )
COMP( 1993, megapc,    0,       0,       megapc,    0,     megapc_state, init_megapc,    "Amstrad plc", "MegaPC", MACHINE_NOT_WORKING )
COMP( 199?, megapcpl,  megapc,  0,       megapcpl,  0,     megapc_state, init_megapcpl,  "Amstrad plc", "MegaPC Plus", MACHINE_NOT_WORKING )
COMP( 199?, megapcpla, megapc,  0,       megapcpla, 0,     at_vrom_fix_state, init_megapcpla, "Amstrad plc", "MegaPC Plus (WINBUS chipset)", MACHINE_NOT_WORKING )
COMP( 1989, pc2386,    ibm5170, 0,       at386l,    0,     at_state,     init_at,        "Amstrad plc", "PC2386", MACHINE_NOT_WORKING )
COMP( 1991, aprfte,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot FT//ex 486 (J3 Motherboard)", MACHINE_NOT_WORKING )
COMP( 1991, ftsserv,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot FTs (Scorpion)", MACHINE_NOT_WORKING )
COMP( 1992, aprpand,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot FTs (Panther Rev F 1.02.26)", MACHINE_NOT_WORKING )
COMP( 1990, aplanst,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Apricot",     "Apricot LANstation (Krypton Motherboard)", MACHINE_NOT_WORKING )
COMP( 1990, aplannb,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Apricot",     "Apricot LANstation (Novell Remote Boot)", MACHINE_NOT_WORKING )
COMP( 1992, aplscar,   ibm5170, 0,       at486l,    0,     at_state,     init_at,        "Apricot",     "Apricot LS Pro (Caracal Motherboard)", MACHINE_NOT_WORKING )
COMP( 1992, aplsbon,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot LS Pro (Bonsai Motherboard)", MACHINE_NOT_WORKING )
COMP( 1988, xb42663,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Apricot",     "Apricot Qi 300 (Rev D,E & F Motherboard)", MACHINE_NOT_WORKING )
COMP( 1988, qi600,     ibm5170, 0,       at386,     0,     at_state,     init_at,        "Apricot",     "Apricot Qi 600 (Neptune Motherboard)", MACHINE_NOT_WORKING )
COMP( 1990, qi900,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot Qi 900 (Scorpion Motherboard)", MACHINE_NOT_WORKING )
COMP( 1989, apvxft,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot VX FT server", MACHINE_NOT_WORKING )
COMP( 1991, apxenls3,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot XEN-LS (Venus IV Motherboard)", MACHINE_NOT_WORKING )
COMP( 1993, apxlsam,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot XEN-LS II (Samurai Motherboard)", MACHINE_NOT_WORKING )
COMP( 1987, apxeni,    ibm5170, 0,       at386,     0,     at_state,     init_at,        "Apricot",     "Apricot XEN-i 386 (Leopard Motherboard)" , MACHINE_NOT_WORKING )
COMP( 1989, xb42639,   ibm5170, 0,       xb42639,   0,     at_state,     init_at,        "Apricot",     "Apricot XEN-S (Venus I Motherboard 286)" , MACHINE_NOT_WORKING )
COMP( 1990, xb42639a,  ibm5170, 0,       xb42639,   0,     at_state,     init_at,        "Apricot",     "Apricot XEN-S (Venus II Motherboard 286)" , MACHINE_NOT_WORKING )
COMP( 1989, xb42664,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Apricot",     "Apricot XEN-S (Venus I Motherboard 386)" , MACHINE_NOT_WORKING )
COMP( 1990, xb42664a,  ibm5170, 0,       at386,     0,     at_state,     init_at,        "Apricot",     "Apricot XEN-S (Venus II Motherboard 386)" , MACHINE_NOT_WORKING )
COMP( 1993, apxena1,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot XEN PC (A1 Motherboard)", MACHINE_NOT_WORKING )
COMP( 1993, apxenp2,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Apricot",     "Apricot XEN PC (P2 Motherboard)", MACHINE_NOT_WORKING )
COMP( 1992, a433cc,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "J-Bond",      "A433C-C/A450C-C", MACHINE_NOT_WORKING )
COMP( 1994, a486ap4,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Asus",        "PVI-486AP4", MACHINE_NOT_WORKING )
COMP( 1994, a486sp3,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Asus",        "PVI-486SP3", MACHINE_NOT_WORKING )
COMP( 1994, a486sp3g,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Asus",        "PCI/I-486SP3G", MACHINE_NOT_WORKING )
COMP( 1995, aa486s,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Asus",        "PCI/I-A486S", MACHINE_NOT_WORKING )
COMP( 1994, a486sv1,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Asus",        "VL/EISA-486SV1", MACHINE_NOT_WORKING )
COMP( 1994, a486sv2g,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Asus",        "VL/I-486SV2G", MACHINE_NOT_WORKING )
COMP( 199?, mb8433uud, ibm5170, 0,       at486,     0,     at_state,     init_at,        "Biostar",     "MB8433-UUD-A", MACHINE_NOT_WORKING ) // lands in Award BootBlock BIOS
COMP( 1990, c386sx16,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Commodore Business Machines", "386SX-16", MACHINE_NOT_WORKING )
COMP( 199?, dt386,     ibm5170, 0,       at386,     0,     at_state,     init_at,        "Commodore Business Machines", "DT386", MACHINE_NOT_WORKING )
COMP( 199?, dt486,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "Commodore Business Machines", "DT486", MACHINE_NOT_WORKING )
COMP( 1988, pc30iii,   ibm5170, 0,       pc30iii,   0,     at_state,     init_at,        "Commodore Business Machines",  "PC 30-III", MACHINE_NOT_WORKING )
COMP( 1988, pc40iii,   ibm5170, 0,       pc40iii,   0,     at_state,     init_at,        "Commodore Business Machines",  "PC 40-III", MACHINE_NOT_WORKING )
COMP( 198?, pc45iii,   ibm5170, 0,       pc40iii,   0,     at_state,     init_at,        "Commodore Business Machines",  "PC 45-III", MACHINE_NOT_WORKING )
COMP( 198?, pc50ii,    ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Commodore Business Machines",  "PC 50-II", MACHINE_NOT_WORKING )
COMP( 198?, pc60iii,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Commodore Business Machines",  "PC 60-III", MACHINE_NOT_WORKING )
COMP( 199?, pc70iii,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Commodore Business Machines",  "PC 70-III", MACHINE_NOT_WORKING )
COMP( 1990, c286lt,    ibm5170, 0,       atvga,     0,     at_state,     init_at,        "Commodore Business Machines",  "Laptop C286LT", MACHINE_NOT_WORKING )
COMP( 1991, c386sxlt,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Commodore Business Machines",  "Laptop C386SX-LT", MACHINE_NOT_WORKING )
COMP( 199?, csl286,    ibm5170, 0,       atvga,     0,     at_state,     init_at,        "Commodore Business Machines",  "SL 286-16", MACHINE_NOT_WORKING )
COMP( 199?, comt386,   ibm5170, 0,       at386,     0,     at_state,     init_at,        "Commodore Business Machines",  "Tower 386", MACHINE_NOT_WORKING )
COMP( 199?, comt486,   ibm5170, 0,       at486,     0,     at_state,     init_at,        "Commodore Business Machines",  "Tower 486", MACHINE_NOT_WORKING )
COMP( 1988, dsys200,   ibm5170, 0,       atvga,     0,     at_state,     init_at,        "Dell Computer Corporation",    "System 200", MACHINE_NOT_WORKING )
COMP( 1995, ficpio2,   ibm5170, 0,       ficpio2,   0,     at_state,     init_atpci,     "First International Computer", "486-PIO-2", MACHINE_NOT_WORKING )
COMP( 1994, ficvipio,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "First International Computer", "486-VIP-IO", MACHINE_NOT_WORKING )
COMP( 199?, ficvipio2, ibm5170, 0,       at486,     0,     at_state,     init_at,        "First International Computer", "486-VIP-IO2", MACHINE_NOT_WORKING )
COMP( 199?, hot409,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Shuttle Computer International", "HOT-409", MACHINE_NOT_WORKING )
COMP( 1985, k286i,     ibm5170, 0,       k286i,     0,     at_state,     init_at,        "Kaypro",      "286i", MACHINE_NOT_WORKING )
COMP( 1991, t2000sx,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Toshiba",     "T2000SX", MACHINE_NOT_WORKING )
COMP( 1992, mbc28,     ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Sanyo",       "MBC-28", MACHINE_NOT_WORKING ) // Complains about missing mouse hardware
COMP( 199?, sarcpc,    ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "<unknown>",   "80286 Standard System (SARC RC2015 chipset)", MACHINE_NOT_WORKING )
COMP( 1986, pcd2,      ibm5170, 0,       ibm5170,   0,     at_state,     init_at,        "Siemens",     "PCD-2", MACHINE_NOT_WORKING )
COMP( 199?, pcd3nsx,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Siemens-Nixdorf", "PCD-3Nsx", MACHINE_NOT_WORKING )
COMP( 199?, pcd4x,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "Siemens-Nixdorf", "PCD-4H, PCD-4M", MACHINE_NOT_WORKING )
COMP( 1995, pcd4nl,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Siemens-Nixdorf", "PCD-4NL", MACHINE_NOT_WORKING )
COMP( 1993, pcd4nd,    ibm5170, 0,       at486,     0,     at_state,     init_at,        "Siemens-Nixdorf", "PCD-4ND", MACHINE_NOT_WORKING )
COMP( 19??, toptek286, ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "Toptek Micro Computer", "286 Turbo", MACHINE_NOT_WORKING )
COMP( 1992, walk386sx, ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Triumph-Adler", "Walkstation 386 SX", MACHINE_NOT_WORKING ) // screen remains blank
COMP( 1992, walk386dx, ibm5170, 0,       at386,     0,     at_state,     init_at,        "Triumph-Adler", "Walkstation 386DX", MACHINE_NOT_WORKING ) // screen remains blank
COMP( 1987, comportii ,ibm5170, 0,       comportii, 0,     at_state,     init_at,        "Compaq",      "Portable II", MACHINE_NOT_WORKING )
COMP( 1987, comportiii,ibm5170, 0,       comportiii,0,     at_state,     init_at,        "Compaq",      "Portable III", MACHINE_NOT_WORKING )
COMP( 1988, comslt286, ibm5170, 0,       atvga,     0,     at_state,     init_at,        "Compaq",      "SLT/286", MACHINE_NOT_WORKING )
COMP( 1986, ews286,    ibm5170, 0,       ews286,    0,     at_state,     init_at,        "Ericsson",    "Ericsson WS286", MACHINE_NOT_WORKING )
COMP( 198?, ev1806,    ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "Everex Systems", "EV-1806", MACHINE_NOT_WORKING ) // continuous beeps (RAM not detected?)
COMP( 19??, ht286,     ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "unknown",     "unknown 286 AT clone (HT12/A chipset)", MACHINE_NOT_WORKING )
COMP( 19??, kt216wb5,  ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "KT Technology", "KT216WB5-HI Rev.2", MACHINE_NOT_WORKING )
COMP( 198?, lm103s,    ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "unknown",     "LM-103S", MACHINE_NOT_WORKING )
COMP( 198?, magb233,   ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "Magitronic Technology", "Magitronic B233", MACHINE_NOT_WORKING )
COMP( 19??, mat286,    ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "unknown",     "MAT286 Rev.D", MACHINE_NOT_WORKING )
COMP( 1986, ncrpc8,    ibm5170, 0,       atvga,     0,     at_state,     init_at,        "NCR",         "PC-8", MACHINE_NOT_WORKING )
COMP( 1987, n8810m15,  ibm5170, 0,       n8810m15,  0,     at_state,     init_at,        "Nixdorf Computer AG", "8810 M15", MACHINE_NOT_WORKING )
COMP( 1990, n8810m16c, ibm5170, 0,       n8810m15,  0,     at_state,     init_at,        "Nixdorf Computer AG", "8810 M16 CGA version", MACHINE_NOT_WORKING )
COMP( 1990, n8810m16v, ibm5170, 0,       atvga,     0,     at_state,     init_at,        "Nixdorf Computer AG", "8810 M16 VGA version", MACHINE_NOT_WORKING )
COMP( 1990, n8810m30,  ibm5170, 0,       neat,      0,     at_state,     init_at,        "Nixdorf Computer AG", "8810 M30", MACHINE_NOT_WORKING )
COMP( 1986, n8810m55,  ibm5170, 0,       n8810m55,  0,     at_state,     init_at,        "Nixdorf Computer AG", "8810 M55", MACHINE_NOT_WORKING )
COMP( 1987, m290,      ibm5170, 0,       atvga,     0,     at_state,     init_at,        "Olivetti",    "M290", MACHINE_NOT_WORKING )
COMP( 1993, lion3500,  ibm5170, 0,       at486,     0,     at_state,     init_at,        "Lion",        "3500", MACHINE_NOT_WORKING )
COMP( 198?, o286foxii, ibm5170, 0,       atvga,     0,     at_state,     init_at,        "Octek",       "Fox II", MACHINE_NOT_WORKING )
//COMP( 1988, nws286,    ibm5170,  0,      ews286,    0,     at_state,     at,        "Nokia Data",  "Nokia Data WS286", MACHINE_NOT_WORKING )
