// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
/***************************************************************************

    IBM AT Compatibles

Commodore PC 30-III and PC 40-III
=================================
Links: http://www.richardlagendijk.nl/cip/computer/item/pc30iii/en , ftp://ftp.zimmers.net/pub/cbm-pc/firmware/pc30/
Info: The PC 30-III and PC 40-III share the same mainboard. On a PC 30-III the onboard Paradise VGA is not populated.
Form factor: Desktop PC
CPU: Intel 80286-12
RAM: 1MB on board
Chipset: Faraday
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
	void ibm5162(machine_config &config);
	void neat(machine_config &config);
	void at386l(machine_config &config);
	void ibm5170a(machine_config &config);
	void ec1842(machine_config &config);
	void at486(machine_config &config);
	void ficpio2(machine_config &config);
	void at386sx(machine_config &config);
	void pc40iii(machine_config &config);
	void atvga(machine_config &config);
	void at386(machine_config &config);
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
	RAM(config, m_ram).set_default_size("1664K").set_extra_options("2M,4M,8M,15M");
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
	m_isabus->set_cputag("maincpu");
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

//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************
ROM_START( ibm5170 )
	ROM_REGION(0x20000,"bios", 0)

	ROM_SYSTEM_BIOS( 0, "rev1", "IBM PC/AT 5170 01/10/84")
	ROMX_LOAD("6181028.u27", 0x10000, 0x8000, CRC(f6573f2a) SHA1(3e52cfa6a6a62b4e8576f4fe076c858c220e6c1a), ROM_SKIP(1) | ROM_BIOS(0)) /* T 6181028 8506AAA // TMM23256P-5878 // (C)IBM CORP 1981,-1984 */
	ROMX_LOAD("6181029.u47", 0x10001, 0x8000, CRC(7075fbb2) SHA1(a7b885cfd38710c9bc509da1e3ba9b543a2760be), ROM_SKIP(1) | ROM_BIOS(0)) /* T 6181029 8506AAA // TMM23256P-5879 // (C)IBM CORP 1981,-1984 */

	ROM_SYSTEM_BIOS( 1, "rev2", "IBM PC/AT 5170 06/10/85")  /* Another verifaction of these crcs would be nice */
	ROMX_LOAD("6480090.u27", 0x10000, 0x8000, CRC(99703aa9) SHA1(18022e93a0412c8477e58f8c61a87718a0b9ab0e), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("6480091.u47", 0x10001, 0x8000, CRC(013ef44b) SHA1(bfa15d2180a1902cb6d38c6eed3740f5617afd16), ROM_SKIP(1) | ROM_BIOS(1))

//  ROM_SYSTEM_BIOS( 2, "atdiag", "IBM PC/AT 5170 w/Super Diagnostics")
//  ROMX_LOAD("atdiage.bin", 0xf8000, 0x4000, CRC(e8855d0c) SHA1(c9d53e61c08da0a64f43d691bf6cadae5393843a), ROM_SKIP(1) | ROM_BIOS(2))
//  ROMX_LOAD("atdiago.bin", 0xf8001, 0x4000, CRC(606fa71d) SHA1(165e45bae7ae2da274f1e645c763c5bfcbde027b), ROM_SKIP(1) | ROM_BIOS(2))

	/* Mainboard PALS */
	ROM_REGION( 0x2000, "pals", 0 )
	ROM_LOAD("1501824_717750.mmipal14l4.u87.jed", 0x0000, 0x02E7, CRC(3c819a27) SHA1(d2f4889e628dbbef50b7f48cb1d1a313232bacc8)) /* MMI 1501824 717750 // (C)1983 IBM(M) */
	ROM_LOAD("1503135_705075.mmipal14l4.u130.jed", 0x02E7, 0x02E7, CRC(aac77198) SHA1(b318da3a1fbe5402836c1b548e231e0794d0c032)) /* MMI 1503135 705075 // (C) IBM CORP 83 */
	/* P/N 6320947 Serial/Parallel ISA expansion card PAL */
	ROM_LOAD("1503085.mmipal.u14.jed", 0x1000, 0x0800, NO_DUMP) /* MMI 1503085 8449 // (C) IBM CORP 83 */ /* Not sure of type */

	/* Mainboard PROMS */
	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD("1501814.82s123an.u115", 0x0000, 0x0020, CRC(849c9217) SHA1(2955ae1705c3b59170f1373f99b3ea5c174c4544)) /* N82S123AN 8713 // SK-D 1501814 */
	ROM_LOAD("55x8041.82s147an.u72", 0x0020, 0x0200, CRC(f2cc4fe6) SHA1(e285468516bd05083155a8a272583deef655315a)) /* S N82S147AN 8709 // V-C55X8041 */
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
	ROMX_LOAD("61x9266.u27", 0x10000, 0x8000, CRC(4995be7a) SHA1(8e8e5c863ae3b8c55fd394e345d8cca48b6e575c), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("61x9265.u47", 0x10001, 0x8000, CRC(c32713e4) SHA1(22ed4e2be9f948682891e2fd056a97dbea01203c), ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS( 1, "3270at", "IBM 3270 PC/AT 5281 11/15/85") /* pretty much just a part string and checksum change from the 5170 rev3 */
	ROMX_LOAD("62x0820.u27", 0x10000, 0x8000, CRC(e9cc3761) SHA1(ff9373c1a1f34a32fb6acdabc189c61b01acf9aa), ROM_SKIP(1) | ROM_BIOS(1)) /* T 62X0820-U27 8714HAK // TMM23256P-6746 // (C)IBM CORP 1981,-1985 */
	ROMX_LOAD("62x0821.u47", 0x10001, 0x8000, CRC(b5978ccb) SHA1(2a1aeb9ae3cd7e60fc4c383ca026208b82156810), ROM_SKIP(1) | ROM_BIOS(1)) /* T 62X0821-U47 8715HAK // TMM23256P-6747 // (C)IBM CORP 1981,-1985 */

	/* Mainboard PALS */
	ROM_REGION( 0x2000, "pals", 0 )
	ROM_LOAD("1501824_717750.mmipal14l4.u87.jed", 0x0000, 0x02E7, CRC(3c819a27) SHA1(d2f4889e628dbbef50b7f48cb1d1a313232bacc8)) /* MMI 1501824 717750 // (C)1983 IBM(M) */
	ROM_LOAD("1503135_705075.mmipal14l4.u130.jed", 0x02E7, 0x02E7, CRC(aac77198) SHA1(b318da3a1fbe5402836c1b548e231e0794d0c032)) /* MMI 1503135 705075 // (C) IBM CORP 83 */    /* P/N 6320947 Serial/Parallel ISA expansion card PAL */
	ROM_LOAD("1503085.mmipal.u14.jed", 0x1000, 0x0800, NO_DUMP) /* MMI 1503085 8449 // (C) IBM CORP 83 */ /* Not sure of type */

	/* Mainboard PROMS */
	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD("1501814.82s123an.u115", 0x0000, 0x0020, CRC(849c9217) SHA1(2955ae1705c3b59170f1373f99b3ea5c174c4544)) /* N82S123AN 8713 // SK-D 1501814 */
	ROM_LOAD("55x8041.82s147an.u72", 0x0020, 0x0200, CRC(f2cc4fe6) SHA1(e285468516bd05083155a8a272583deef655315a)) /* S N82S147AN 8709 // V-C55X8041 */
ROM_END


ROM_START( ibm5162 ) //MB p/n 62x1168
	ROM_REGION16_LE(0x20000,"bios", 0)

	ROM_LOAD16_BYTE("78x7460.u34", 0x10000, 0x8000, CRC(1db4bd8f) SHA1(7be669fbb998d8b4626fefa7cd1208d3b2a88c31)) /* 78X7460 U34 // (C) IBM CORP // 1981-1986 */
	ROM_LOAD16_BYTE("78x7461.u35", 0x10001, 0x8000, CRC(be14b453) SHA1(ec7c10087dbd53f9c6d1174e8f14212e2aec1818)) /* 78X7461 U35 // (C) IBM CORP // 1981-1986 */

	/* Mainboard PALS */
	ROM_REGION( 0x2000, "pals", 0 )
	ROM_LOAD("59x7599.mmipal20l8.u27.jed", 0x0000, 0x02E7, NO_DUMP) /* MMI PAL20L8ACN5 8631 // N59X7599 IBM (C)85 K3 */
	ROM_LOAD("1503135.mmipal14l4.u81.jed", 0x02E7, 0x02E7, CRC(aac77198) SHA1(b318da3a1fbe5402836c1b548e231e0794d0c032)) /* MMI 1503135 8625 // (C) IBM CORP 83 */
	/* P/N 6320947 Serial/Parallel ISA expansion card PAL */
	ROM_LOAD("1503085.mmipal.u14.jed", 0x1000, 0x0800, NO_DUMP) /* MMI 1503085 8449 // (C) IBM CORP 83 */ /* Not sure of type */

	/* Mainboard PROMS */
	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD("1501814.82s123an.u72", 0x0000, 0x0020, CRC(849c9217) SHA1(2955ae1705c3b59170f1373f99b3ea5c174c4544)) /* N82S123AN 8623 // SK-U 1501814 */
	ROM_LOAD("59x7594.82s147an.u90", 0x0020, 0x0200, NO_DUMP) /* S N82S147AN 8629 // VCT 59X7594 */
ROM_END

ROM_START( ibmps1es )
	ROM_REGION(0x40000, "bios", 0)
	ROM_LOAD16_BYTE( "ibm_1057757_24-05-90.bin", 0x00000, 0x20000, CRC(c8f81ea4) SHA1(925ed0e98f9f2997cb86554ef384bcfaf2a4ecbe))
	ROM_LOAD16_BYTE( "ibm_1057757_29-15-90.bin", 0x00001, 0x20000, CRC(c2dd6b5c) SHA1(f6b5785002dd628b6b1fb3bb101e076299eba3b6))
ROM_END

ROM_START( at )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "ami211", "AMI 21.1") /*(Motherboard Manufacturer: Dataexpert Corp. Motherboard) (Neat 286 Bios, 82c21x Chipset ) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "ami211.bin",     0x10000, 0x10000,CRC(a0b5d269) SHA1(44db8227d35a09e39b93ed944f85dcddb0dd0d39), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "at", "PC 286") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 03-11-1987)*/
	ROMX_LOAD("at110387.1", 0x10001, 0x8000, CRC(679296a7) SHA1(ae891314cac614dfece686d8e1d74f4763cf40e3),ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD("at110387.0", 0x10000, 0x8000, CRC(65ae1f97) SHA1(91a29c7deecf7a9afbba330e64e0eee9aafee4d1),ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "ami206", "AMI C 206.1")  /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 15-10-1990)*/
	ROMX_LOAD( "amic206.bin",    0x10000, 0x10000,CRC(25a67c34) SHA1(91e9d8cdc2f1b40a601a23ceaff2189fd1245f3b), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(3, "amic21", "AMI C 21.1")
	ROMX_LOAD( "amic21-2.bin",  0x10001, 0x8000, CRC(8ffe7752) SHA1(68215f07a170ee7bdcb3e52b370d470af1741f7e),ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "amic21-1.bin",  0x10000, 0x8000, CRC(a76497f6) SHA1(91b47d86967426945b2916cb40e76a8da2d31d54),ROM_SKIP(1) | ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(4, "ami101", "AMI HT 101.1") /* Quadtel Enhanced 286 Bios Version 3.04.02 */
	ROMX_LOAD( "amiht-h.bin",   0x10001, 0x8000, CRC(8022545f) SHA1(42541d4392ad00b0e064b3a8ccf2786d875c7c19),ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "amiht-l.bin",   0x10000, 0x8000, CRC(285f6b8f) SHA1(2fce4ec53b68c9a7580858e16c926dc907820872),ROM_SKIP(1) | ROM_BIOS(4) )
	ROM_SYSTEM_BIOS(5, "ami121", "AMI HT 12.1")
	ROMX_LOAD( "ami2od86.bin",  0x10001, 0x8000, CRC(04a2cec4) SHA1(564d37a8b2c0f4d0e23cd1e280a09d47c9945da8),ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "ami2ev86.bin",  0x10000, 0x8000, CRC(55deb5c2) SHA1(19ce1a7cc985b5895c585e39211475de2e3b0dd1),ROM_SKIP(1) | ROM_BIOS(5) )
	ROM_SYSTEM_BIOS(6, "ami122", "AMI HT 12.2")
	ROMX_LOAD( "ami2od89.bin",  0x10001, 0x8000, CRC(7c81bbe8) SHA1(a2c7eca586f6e2e76b9101191e080a1f1cb8b833),ROM_SKIP(1) | ROM_BIOS(6) )
	ROMX_LOAD( "ami2ev89.bin",  0x10000, 0x8000, CRC(705d36e0) SHA1(0c9cfb71ced4587f109b9b6dfc2a9c92302fdb99),ROM_SKIP(1) | ROM_BIOS(6) )
	ROM_SYSTEM_BIOS(7, "ami123", "AMI HT 12.3") /*(Motherboard Manufacturer: Aquarius Systems USA Inc.) (BIOS release date:: 13-06-1990)*/
	ROMX_LOAD( "ht12h.bin",     0x10001, 0x8000, CRC(db8b471e) SHA1(7b5fa1c131061fa7719247db3e282f6d30226778),ROM_SKIP(1) | ROM_BIOS(7) )
	ROMX_LOAD( "ht12l.bin",     0x10000, 0x8000, CRC(74fd178a) SHA1(97c8283e574abbed962b701f3e8091fb82823b80),ROM_SKIP(1) | ROM_BIOS(7) )
	ROM_SYSTEM_BIOS(8, "ami181", "AMI HT 18.1") /* not a bad dump, sets unknown probably chipset related registers at 0x1e8 before failing post */
	ROMX_LOAD( "ht18.bin",     0x10000, 0x10000, CRC(f65a6f9a) SHA1(7dfdf7d243f9f645165dc009c5097dd515f86fbb), ROM_BIOS(8) )
	ROM_SYSTEM_BIOS(9, "amiht21", "AMI HT 21.1") /* as above */
	ROMX_LOAD( "ht21e.bin",    0x10000, 0x10000, CRC(e80f7fed) SHA1(62d958d98c95e9e4d1b290a6c1054ae98770f276), ROM_BIOS(9) )
	ROM_SYSTEM_BIOS(10, "amip1", "AMI P.1") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "poisk-h.bin",   0x10001, 0x8000, CRC(83fd3f8c) SHA1(ca94850bbd949b97b11710629886b0ee69489a81),ROM_SKIP(1) | ROM_BIOS(10) )
	ROMX_LOAD( "poisk-l.bin",   0x10000, 0x8000, CRC(0b2ed291) SHA1(bb51a3f317cf4d429a6cfb44a46ca0ac39d9aaa7),ROM_SKIP(1) | ROM_BIOS(10) )
	ROM_SYSTEM_BIOS(11, "aw201", "Award 201")
	ROMX_LOAD( "83201-5h.bin",  0x10001, 0x8000, CRC(968d1fc0) SHA1(dc4122a6c696f0b43e7894dc1b669346eed755d5),ROM_SKIP(1) | ROM_BIOS(11) )
	ROMX_LOAD( "83201-5l.bin",  0x10000, 0x8000, CRC(bf50a89a) SHA1(2349a1db6017a7fb0673e99d3680c8753407be8d),ROM_SKIP(1) | ROM_BIOS(11) )
	ROM_SYSTEM_BIOS(12, "aw303", "Award 303 NFS")
	ROMX_LOAD( "aw303-hi.bin",  0x18001, 0x4000, CRC(78f32d7e) SHA1(1c88398fb171b33b7e6191bad63704ae85bfed8b), ROM_SKIP(1) | ROM_BIOS(12) )
	ROMX_LOAD( "aw303-lo.bin",  0x18000, 0x4000, CRC(3d2a70c0) SHA1(1329113bec514ed2a6d803067b1132744ef534dd), ROM_SKIP(1) | ROM_BIOS(12) )
	ROM_SYSTEM_BIOS(13, "aw303gs", "Award 303GS")
	ROMX_LOAD( "aw303gs-hi.bin",  0x18001, 0x4000, CRC(82392e18) SHA1(042453b7b29933a1b72301d21fcf8fa6b293c9c9), ROM_SKIP(1) | ROM_BIOS(13) )
	ROMX_LOAD( "aw303gs-lo.bin",  0x18000, 0x4000, CRC(a4cf8ba1) SHA1(b73e34be3b2754aaed1ac06471f4441fea06c67c), ROM_SKIP(1) | ROM_BIOS(13) )
	ROM_SYSTEM_BIOS(14, "ami_200960", "AMI 200960")
	ROMX_LOAD( "ami_286_bios_sn200960_even.bin", 0x10000, 0x8000, CRC(67745815) SHA1(ca6886c7a0716a92a8720fc71ff2d95328c467a5), ROM_SKIP(1) | ROM_BIOS(14) )
	ROMX_LOAD( "ami_286_bios_sn200960_odd.bin", 0x10001, 0x8000, CRC(360a5f73) SHA1(1b1980fd99779d0cdc4764928a641e081b35ee9f), ROM_SKIP(1) | ROM_BIOS(14) )
	ROM_SYSTEM_BIOS(15, "magitronic_b233", "Magitronic B233") // SUNTAC Chipset, http://toastytech.com/manuals/Magitronic%20B233%20Manual.pdf
	ROMX_LOAD( "magitronic_b233_ami_1986_286_bios_plus_even_sa027343.bin", 0x10000, 0x8000, CRC(d4a18444) SHA1(d95242104fc9b51cf26de72ef5b6c52d99ccce30), ROM_SKIP(1) | ROM_BIOS(15) )
	ROMX_LOAD( "magitronic_b233_ami_1986_286_bios_plus_odd_sa027343.bin", 0x10001, 0x8000, CRC(7ac3db56) SHA1(4340140450c4f8b4f6a19eae50a5dc5449edfdf6), ROM_SKIP(1) | ROM_BIOS(15) )
	// ROM_LOAD("magitronic_b233_ami_1986_keyboard_bios_plus_a025352.bin", 0x0000, 0x1000), CRC(84fd28fd) SHA1(43da0f49e52c921844e60b6f3d22f2a316d865cc) )
ROM_END


ROM_START( atvga )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "vl82c", "VL82C311L-FC4")/*(Motherboard Manufacturer: Biostar Microtech Corp.) (BIOS release date: 05-05-1991)*/
	ROMX_LOAD( "2vlm001.bin",     0x10000, 0x10000, CRC(f34d800a) SHA1(638aca592a0e525f957beb525e95ca666a994ee8), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "ami211", "AMI 21.1") /*(Motherboard Manufacturer: Dataexpert Corp. Motherboard) (Neat 286 Bios, 82c21x Chipset ) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "ami211.bin",     0x10000, 0x10000,CRC(a0b5d269) SHA1(44db8227d35a09e39b93ed944f85dcddb0dd0d39), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "ami206", "AMI C 206.1") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 15-10-1990)*/
	ROMX_LOAD( "amic206.bin",    0x10000, 0x10000,CRC(25a67c34) SHA1(91e9d8cdc2f1b40a601a23ceaff2189fd1245f3b), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(3, "amic21", "AMI C 21.1")
	ROMX_LOAD( "amic21-2.bin",  0x10001, 0x8000, CRC(8ffe7752) SHA1(68215f07a170ee7bdcb3e52b370d470af1741f7e),ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "amic21-1.bin",  0x10000, 0x8000, CRC(a76497f6) SHA1(91b47d86967426945b2916cb40e76a8da2d31d54),ROM_SKIP(1) | ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(4, "ami101", "AMI HT 101.1") /* Quadtel Enhanced 286 Bios Version 3.04.02 */
	ROMX_LOAD( "amiht-h.bin",   0x10001, 0x8000, CRC(8022545f) SHA1(42541d4392ad00b0e064b3a8ccf2786d875c7c19),ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "amiht-l.bin",   0x10000, 0x8000, CRC(285f6b8f) SHA1(2fce4ec53b68c9a7580858e16c926dc907820872),ROM_SKIP(1) | ROM_BIOS(4) )
	ROM_SYSTEM_BIOS(5, "ami121", "AMI HT 12.1")
	ROMX_LOAD( "ami2od86.bin",  0x10001, 0x8000, CRC(04a2cec4) SHA1(564d37a8b2c0f4d0e23cd1e280a09d47c9945da8),ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "ami2ev86.bin",  0x10000, 0x8000, CRC(55deb5c2) SHA1(19ce1a7cc985b5895c585e39211475de2e3b0dd1),ROM_SKIP(1) | ROM_BIOS(5) )
	ROM_SYSTEM_BIOS(6, "ami122", "AMI HT 12.2")
	ROMX_LOAD( "ami2od89.bin",  0x10001, 0x8000, CRC(7c81bbe8) SHA1(a2c7eca586f6e2e76b9101191e080a1f1cb8b833),ROM_SKIP(1) | ROM_BIOS(6) )
	ROMX_LOAD( "ami2ev89.bin",  0x10000, 0x8000, CRC(705d36e0) SHA1(0c9cfb71ced4587f109b9b6dfc2a9c92302fdb99),ROM_SKIP(1) | ROM_BIOS(6) )
	ROM_SYSTEM_BIOS(7, "ami123", "AMI HT 12.3") /*(Motherboard Manufacturer: Aquarius Systems USA Inc.) (BIOS release date:: 13-06-1990)*/
	ROMX_LOAD( "ht12h.bin",     0x10001, 0x8000, CRC(db8b471e) SHA1(7b5fa1c131061fa7719247db3e282f6d30226778),ROM_SKIP(1) | ROM_BIOS(7) )
	ROMX_LOAD( "ht12l.bin",     0x10000, 0x8000, CRC(74fd178a) SHA1(97c8283e574abbed962b701f3e8091fb82823b80),ROM_SKIP(1) | ROM_BIOS(7) )
	ROM_SYSTEM_BIOS(8, "ami181", "AMI HT 18.1") /* not a bad dump, sets unknown probably chipset related registers at 0x1e8 before failing post */
	ROMX_LOAD( "ht18.bin",     0x10000, 0x10000, CRC(f65a6f9a) SHA1(7dfdf7d243f9f645165dc009c5097dd515f86fbb), ROM_BIOS(8) )
	ROM_SYSTEM_BIOS(9, "amiht21", "AMI HT 21.1") /* as above */
	ROMX_LOAD( "ht21e.bin",    0x10000, 0x10000, CRC(e80f7fed) SHA1(62d958d98c95e9e4d1b290a6c1054ae98770f276), ROM_BIOS(9) )
	ROM_SYSTEM_BIOS(10, "amip1", "AMI P.1") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "poisk-h.bin",   0x10001, 0x8000, CRC(83fd3f8c) SHA1(ca94850bbd949b97b11710629886b0ee69489a81),ROM_SKIP(1) | ROM_BIOS(10) )
	ROMX_LOAD( "poisk-l.bin",   0x10000, 0x8000, CRC(0b2ed291) SHA1(bb51a3f317cf4d429a6cfb44a46ca0ac39d9aaa7),ROM_SKIP(1) | ROM_BIOS(10) )
	ROM_SYSTEM_BIOS(11, "ami1131", "AMI-1131") /*(Motherboard Manufacturer: Elitegroup Computer Co., Ltd.) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "2hlm003h.bin",   0x10001, 0x8000, CRC(2babb42b) SHA1(3da6538f44b434cdec0cbdddd392ccfd34666f06),ROM_SKIP(1) | ROM_BIOS(11) )
	ROMX_LOAD( "2hlm003l.bin",   0x10000, 0x8000, CRC(317cbcbf) SHA1(1adad6280d8b07c2921fc5fc13ecaa10e6bfebdc),ROM_SKIP(1) | ROM_BIOS(11) )
	ROM_SYSTEM_BIOS(12, "at", "PC 286") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 03-11-1987)*/
	ROMX_LOAD("at110387.1", 0x10001, 0x8000, CRC(679296a7) SHA1(ae891314cac614dfece686d8e1d74f4763cf40e3),ROM_SKIP(1) | ROM_BIOS(12) )
	ROMX_LOAD("at110387.0", 0x10000, 0x8000, CRC(65ae1f97) SHA1(91a29c7deecf7a9afbba330e64e0eee9aafee4d1),ROM_SKIP(1) | ROM_BIOS(12) )
	ROM_SYSTEM_BIOS(13, "bravo", "AST Bravo/286") // fails with keyboard controller test, probably expects specific kbdc rom
	ROMX_LOAD("107000-704.bin", 0x10000, 0x8000, CRC(94faf87e) SHA1(abaafa6c2ae9b9fba95b244dcbcc1c752ac6c0a0),ROM_SKIP(1) | ROM_BIOS(13) )
	ROMX_LOAD("107000-705.bin", 0x10001, 0x8000, CRC(e1263c1e) SHA1(b564f1043ef45ecbdf4f06bb500150ad992c2931),ROM_SKIP(1) | ROM_BIOS(13) )
ROM_END

ROM_START( xb42639 )
	/* actual VGA BIOS not dumped*/
	ROM_REGION(0x20000, "bios", 0)
	// XEN-S (Venus I Motherboard)
	ROM_LOAD16_BYTE("3-10-17i.lo", 0x10000, 0x8000, CRC(3786ca1e) SHA1(c682d7c76f234559d03bcf21010c13c4dbeafb69))
	ROM_LOAD16_BYTE("3-10-17i.hi", 0x10001, 0x8000, CRC(d66710eb) SHA1(e8c1cd5f9ecfbd8825655e416d7ddf2ae362e69b))
ROM_END

ROM_START( xb42639a )
	/* actual VGA BIOS not dumped*/
	ROM_REGION(0x20000, "bios", 0)
	// XEN-S (Venus II Motherboard)
	ROM_LOAD16_BYTE("10217.lo", 0x10000, 0x8000, CRC(ea53406f) SHA1(2958dfdbda14de4e6b9d6a8c3781131ab1e32bef))
	ROM_LOAD16_BYTE("10217.hi", 0x10001, 0x8000, CRC(111725cf) SHA1(f6018a45bda4476d40c5881fb0a506ff75ec1688))
ROM_END

ROM_START( xb42664 )
	/* actual VGA BIOS not dumped */
	ROM_REGION(0x20000, "bios", 0)
	// XEN-S (Venus I Motherboard)
	ROM_LOAD16_BYTE("3-10-17i.lo", 0x10000, 0x8000, CRC(3786ca1e) SHA1(c682d7c76f234559d03bcf21010c13c4dbeafb69))
	ROM_LOAD16_BYTE("3-10-17i.hi", 0x10001, 0x8000, CRC(d66710eb) SHA1(e8c1cd5f9ecfbd8825655e416d7ddf2ae362e69b))
ROM_END

ROM_START( xb42664a )
	/* actual VGA BIOS not dumped*/
	ROM_REGION(0x20000, "bios", 0)
	// XEN-S (Venus II Motherboard)
	ROM_LOAD16_BYTE("10217.lo", 0x10000, 0x8000, CRC(ea53406f) SHA1(2958dfdbda14de4e6b9d6a8c3781131ab1e32bef))
	ROM_LOAD16_BYTE("10217.hi", 0x10001, 0x8000, CRC(111725cf) SHA1(f6018a45bda4476d40c5881fb0a506ff75ec1688))
ROM_END

ROM_START( neat )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "neat286", "NEAT 286")
	ROMX_LOAD("at030389.0", 0x10000, 0x8000, CRC(4c36e61d) SHA1(094e8d5e6819889163cb22a2cf559186de782582),ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("at030389.1", 0x10001, 0x8000, CRC(4e90f294) SHA1(18c21fd8d7e959e2292a9afbbaf78310f9cad12f),ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "pb800", "Packard Bell PB800")
	ROMX_LOAD("3.10.12-1.bin", 0x10001, 0x8000, CRC(e6bb54c5) SHA1(fa5a376dd44696c78dcc8994e18938b5e1b3e45a),ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("3.10.12-2.bin", 0x10000, 0x8000, CRC(bde46933) SHA1(c7221192f48d6f2f5b773c3c7d2a52b635cb473e),ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

ROM_START( at386sx )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "mb386sx", "mb386sx-25spb") // VLSI SCAMPSX
	ROMX_LOAD("386sx_bios_plus.bin", 0x10000, 0x10000, CRC(f71e5a8d) SHA1(e73fda2547d92bf578e93623d5f2349b97e22393), ROM_BIOS(0))
ROM_END

ROM_START( ct386sx )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "neatsx", "NEATsx 386sx")
	ROMX_LOAD("012l-u25.bin", 0x10000, 0x8000, CRC(4ab1862d) SHA1(d4e8d0ff43731270478ca7671a129080ff350a4f),ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("012h-u24.bin", 0x10001, 0x8000, CRC(17472521) SHA1(7588c148fe53d9dc4cb2d0ab6e0fd51a39bb5d1a),ROM_SKIP(1) | ROM_BIOS(0))
	ROM_FILL(0x1e2c9, 1, 0x00) // skip incompatible keyboard controller test
	ROM_FILL(0x1e2cb, 1, 0xbb) // fix checksum
ROM_END

ROM_START( at386 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "ami386", "AMI 386")
	ROMX_LOAD("ami386.bin",  0x10000, 0x10000, CRC(3a807d7f) SHA1(8289ba36a3dfc3324333b1a834bc6b0402b546f0), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "at386", "unknown 386")  // This dump possibly comes from a MITAC INC 386 board, given that the original driver had it as manufacturer
	ROMX_LOAD("at386.bin",  0x10000, 0x10000, CRC(3df9732a) SHA1(def71567dee373dc67063f204ef44ffab9453ead), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "amicg", "AMI CG")
	ROMX_LOAD("amicg.1",        0x10000, 0x10000,CRC(8408965a) SHA1(9893d3ac851e01b06a68a67d3721df36ca2c96f5), ROM_BIOS(2))
ROM_END


ROM_START( at486 )
	ROM_REGION(0x20000, "bios", 0)

	ROM_SYSTEM_BIOS(0, "at486", "PC/AT 486")
	ROMX_LOAD("at486.bin",   0x10000, 0x10000, CRC(31214616) SHA1(51b41fa44d92151025fc9ad06e518e906935e689), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "mg48602", "UMC MG-48602")
	ROMX_LOAD("mg48602.bin", 0x10000, 0x10000, CRC(45797823) SHA1(a5fab258aecabde615e1e97af5911d6cf9938c11), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "ft01232", "Free Tech 01-232")
	ROMX_LOAD("ft01232.bin", 0x10000, 0x10000, CRC(30efaf92) SHA1(665c8ef05ca052dcc06bb473c9539546bfef1e86), ROM_BIOS(2))

	/* 486 boards from FIC

	naming convention
	xxxxx101 --> for EPROM
	xxxxx701 --> for EEPROM using a Flash Utility v5.02
	xxxxBxxx --> NS 311/312 IO Core Logic
	xxxxCxxx --> NS 332 IO Core Logic
	xxxxGxxx --> Winbond W83787F IO Core Logic
	xxxxJxxx --> Winbond W83877F IO Core Logic

	*/

	/* this is the year 2000 beta bios from FIC, supports GIO-VT, GAC-V, GAC-2, VIP-IO, VIO-VP and GVT-2 */
	ROM_SYSTEM_BIOS(3, "ficy2k", "FIC 486 3.276GN1") /* 1997-06-16, includes CL-GD5429 VGA BIOS 1.00a */
	ROMX_LOAD("3276gn1.bin",  0x00000, 0x20000, CRC(d4ff0cc4) SHA1(567b6bdbc9bff306c8c955f275e01ae4c45fd5f2), ROM_BIOS(3))

	ROM_SYSTEM_BIOS(4, "ficgac2", "FIC 486-GAC-2") /* 1994-04-29, includes CL-GD542X VGA BIOS 1.50 */
	ROMX_LOAD("att409be.bin", 0x00000, 0x20000, CRC(c58e017b) SHA1(14c19e720ce62eb2afe28a70f4e4ebafab0f9e77), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "ficgacv", "FIC 486-GAC-V 3.27GN1") /* 1996-04-08, includes CL-GD542X VGA BIOS 1.41 */
	ROMX_LOAD("327gn1.awd",   0x00000, 0x20000, CRC(017614d4) SHA1(2228c28f21a7e78033d24319449297936465b164), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "ficgiovp", "FIC 486-GIO-VP 3.15GN") /* 1994-05-06 */
	ROMX_LOAD("giovp315.rom", 0x10000, 0x10000, CRC(e102c3f5) SHA1(f15a7e9311cc17afe86da0b369607768b030ddec), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(7, "ficgiovt", "FIC 486-GIO-VT 3.06G") /* 1994-11-20 */
	ROMX_LOAD("306gcd00.awd", 0x10000, 0x10000, CRC(75f3ded4) SHA1(999d4b58204e0b0f33262d0613c855b528bf9597), ROM_BIOS(7))

	ROM_SYSTEM_BIOS(8, "ficgiovt2_326", "FIC 486-GIO-VT2 3.26G")  /* 1994-07-06 */
	ROMX_LOAD("326g1c00.awd", 0x10000, 0x10000, CRC(2e729ab5) SHA1(b713f97fa0e0b62856dab917f417f5b21020b354), ROM_BIOS(8))
	ROM_SYSTEM_BIOS(9, "486_gio_vt2","VBS1.08H 486-GVT-2")        /* 1995-06-19 */
	ROMX_LOAD("award_486_gio_vt2.bin", 0x10000, 0x10000, CRC(58d7c7f9) SHA1(097f15ec2bd672cb3f1763298ca802c7ff26021f), ROM_BIOS(9)) // Vobis version, Highscreen boot logo
	ROM_SYSTEM_BIOS(10, "ficgiovt2_3276", "FIC 486-GIO-VT2 3.276") /* 1997-07-17 */
	ROMX_LOAD("32760000.bin", 0x10000, 0x10000, CRC(ad179128) SHA1(595f67ba4a1c8eb5e118d75bf657fff3803dcf4f), ROM_BIOS(10))

	ROM_SYSTEM_BIOS(11, "ficgvt2", "FIC 486-GVT-2 3.07G") /* 1994-11-02 */
	ROMX_LOAD("3073.bin",     0x10000, 0x10000, CRC(a6723863) SHA1(ee93a2f1ec84a3d67e267d0a490029f9165f1533), ROM_BIOS(11))
	ROM_SYSTEM_BIOS(12, "ficgpak2", "FIC 486-PAK-2 5.15S") /* 1995-06-27, includes Phoenix S3 TRIO64 Enhanced VGA BIOS 1.4-01 */
	ROMX_LOAD("515sbd8a.awd", 0x00000, 0x20000, CRC(778247e1) SHA1(07d8f0f2464abf507be1e8dfa06cd88737782411), ROM_BIOS(12))

	ROM_SYSTEM_BIOS(13, "ficpio3g7", "FIC 486-PIO-3 1.15G705") /* pnp */
	ROMX_LOAD("115g705.awd",  0x00000, 0x20000, CRC(ddb1544a) SHA1(d165c9ecdc9397789abddfe0fef69fdf954fa41b), ROM_BIOS(13))
	ROM_SYSTEM_BIOS(14, "ficpio3g1", "FIC 486-PIO-3 1.15G105") /* non-pnp */
	ROMX_LOAD("115g105.awd",  0x00000, 0x20000, CRC(b327eb83) SHA1(9e1ff53e07ca035d8d43951bac345fec7131678d), ROM_BIOS(14))

	ROM_SYSTEM_BIOS(15, "ficpos", "FIC 486-POS")
	ROMX_LOAD("116di6b7.bin", 0x00000, 0x20000, CRC(d1d84616) SHA1(2f2b27ce100cf784260d8e155b48db8cfbc63285), ROM_BIOS(15))
	ROM_SYSTEM_BIOS(16, "ficpvt", "FIC 486-PVT 5.15")          /* 1995-06-27 */
	ROMX_LOAD("5150eef3.awd", 0x00000, 0x20000, CRC(eb35785d) SHA1(1e601bc8da73f22f11effe9cdf5a84d52576142b), ROM_BIOS(16))
	ROM_SYSTEM_BIOS(17, "ficpvtio", "FIC 486-PVT-IO 5.162W2")  /* 1995-10-05 */
	ROMX_LOAD("5162cf37.awd", 0x00000, 0x20000, CRC(378d813d) SHA1(aa674eff5b972b31924941534c3c988f6f78dc93), ROM_BIOS(17))
	ROM_SYSTEM_BIOS(18, "ficvipio426", "FIC 486-VIP-IO 4.26GN2") /* 1994-12-07 */
	ROMX_LOAD("426gn2.awd",   0x00000, 0x20000, CRC(5f472aa9) SHA1(9160abefae32b450e973651c052657b4becc72ba), ROM_BIOS(18))
	ROM_SYSTEM_BIOS(19, "ficvipio427", "FIC 486-VIP-IO 4.27GN2A") /* 1996-02-14 */
	ROMX_LOAD("427gn2a.awd",  0x00000, 0x20000, CRC(035ad56d) SHA1(0086db3eff711fc710b30e7f422fc5b4ab8d47aa), ROM_BIOS(19))
	ROM_SYSTEM_BIOS(20, "ficvipio2", "FIC 486-VIP-IO2")
	ROMX_LOAD("1164g701.awd", 0x00000, 0x20000, CRC(7b762683) SHA1(84debce7239c8b1978246688ae538f7c4f519d13), ROM_BIOS(20))

	ROM_SYSTEM_BIOS(21, "qdi", "QDI PX486DX33/50P3")
	ROMX_LOAD("qdi_px486.u23", 0x10000, 0x10000, CRC(c80ecfb6) SHA1(34cc9ef68ff719cd0771297bf184efa83a805f3e), ROM_BIOS(21))
ROM_END


// FIC 486-PIO-2 (4 ISA, 4 PCI)
// VIA VT82C505 (ISA/VL to PCI bridge) + VT82C496G (system chipset) + VT82C406MV (keyboard controller, RTC, CMOS), NS311/312 or NS332 I/O
ROM_START( ficpio2 )
	ROM_REGION(0x40000, "isa", 0)
	ROM_SYSTEM_BIOS(0, "ficpio2c7", "FIC 486-PIO-2 1.15C701") /* pnp, i/o core: NS 332, doesn't boot, requires cache emulation? */
	ROMX_LOAD("115c701.awd",  0x020000, 0x20000, CRC(b0dd7975) SHA1(bfde13b0fbd141bc945d37d92faca9f4f59b716d), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "ficpio2b7", "FIC 486-PIO-2 1.15B701") /* pnp, i/o core: NS 311/312, doesn't boot, requires cache emulation? */
	ROMX_LOAD("115b701.awd",  0x020000, 0x20000, CRC(ac24abad) SHA1(01174d84ed32fb1d95cd632d09f773acb8666c83), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "ficpio2c1", "FIC 486-PIO-2 1.15C101") /* non-pnp, i/o core: NS 332, working  */
	ROMX_LOAD("115c101.awd",  0x020000, 0x20000, CRC(5fadde88) SHA1(eff79692c1ecf34b6ea3f02409d14ce1f5c51bf9), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "ficpio2b1", "FIC 486-PIO-2 1.15B101") /* non-pnp, i/o core: NS 311/312, working  */
	ROMX_LOAD("115b101.awd",  0x020000, 0x20000, CRC(ff69617d) SHA1(ecbfc7315dcf6bd3e5b59e3ae9258759f64fe7a0), ROM_BIOS(3))
ROM_END

ROM_START( c386sx16 )
	ROM_REGION(0x20000,"bios", 0)
	/* actual VGA BIOS not dumped - uses a WD Paradise according to http://www.cbmhardware.de/pc/pc.php */

	/* Commodore 80386SX BIOS Rev. 1.03 */
	/* Copyright (C) 1985-1990 Commodore Electronics Ltd. */
	/* Copyright (C) 1985-1990 Phoenix Technologies Ltd. */
	ROM_LOAD16_BYTE( "390914-01.u39", 0x10000, 0x8000, CRC(8f849198) SHA1(550b04bac0d0807d6e95ec25391a81272779b41b)) /* 390914-01 V1.03 CS-2100 U39 Copyright (C) 1990 CBM */
	ROM_LOAD16_BYTE( "390915-01.u38", 0x10001, 0x8000, CRC(ee4bad92) SHA1(6e02ef97a7ce336485814c06a1693bc099ce5cfb)) /* 390915-01 V1.03 CS-2100 U38 Copyright (C) 1990 CBM */
ROM_END

// Commodore PC 30-III
ROM_START( pc30iii )
	ROM_REGION(0x20000,"bios", 0)
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

	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "pc40iii_v200", "PC 40-III v2.00")
	ROMX_LOAD( "pc40iii_390339-01_v2.0_473a.bin", 0x18000, 0x4000, CRC(2ad2dc0f) SHA1(b41d5988fda8cc23418c3f665d780c617aa3fc2b),ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "pc40iii_390340-01_v2.0_4bc6.bin",  0x18001, 0x4000, CRC(62dc7d93) SHA1(e741528697b1d00450fd18e3db8b925606e0bd22),ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "pc40iii_v201", "PC 40-III v2.03")
	ROMX_LOAD( "cbm-pc40c-bios-lo-v2.03-390339-04-03bc.bin", 0x18000, 0x4000, CRC(e5fd11c6) SHA1(18c21d9a4ae687eef5464b76a0d614b9dfd30ec8),ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "cbm-pc40c-bios-hi-v2.03-390340-04-3344.bin",  0x18001, 0x4000, CRC(63d6f0f7) SHA1(a88dee7694baa71913acbe76cb4e2a4e95979ad9),ROM_SKIP(1) | ROM_BIOS(1) )
ROM_END

ROM_START( xb42663 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "qi310223.lo", 0x00000, 0x10000, CRC(53047f49) SHA1(7b38e533f7f27295269549c63e5477d950239167))
	ROM_LOAD16_BYTE( "qi310223.hi", 0x00001, 0x10000, CRC(4852869f) SHA1(98599d4691d40b3fac2936034c70b386ce4caf77))
ROM_END

ROM_START( qi600 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "qi610223.lo", 0x00000, 0x10000, CRC(563114a9) SHA1(62932b3bf0b5502ff708f604c21773f00afda58e))
	ROM_LOAD16_BYTE( "qi610223.hi", 0x00001, 0x10000, CRC(0ae133f6) SHA1(6039c366f7fe0ebf60b34c1a7d6b2d781b664001))
ROM_END

ROM_START( qi900 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "qi910224.lo", 0x00000, 0x10000, CRC(b012ad3c) SHA1(807e788a6bd03f5e983fe503af3d0b202c754b8a))
	ROM_LOAD16_BYTE( "qi910224.hi", 0x00001, 0x10000, CRC(36e66d56) SHA1(0900c5272ec3ced550f18fb08db59ab7f67a621e))
ROM_END

ROM_START( ftsserv )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "fts10226.lo", 0x00000, 0x10000, CRC(efbd738f) SHA1(d5258760bafdaf1bf13c4a49da76d4b5e7b4ccbd))
	ROM_LOAD16_BYTE( "fts10226.hi", 0x00001, 0x10000, CRC(2460853f) SHA1(a6bba8d2f800140afd129c4d5278f7ae8fe7e63a))
	/* FT Server series Front Panel */
	ROM_REGION(0x10000,"front", 0)
	ROM_LOAD( "fp10009.bin",     0x0000, 0x8000, CRC(8aa7f718) SHA1(9ee6c6a5bb92622ea8d3805196d42ff68887d820))
ROM_END

ROM_START( apxenls3 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "31020.lo", 0x10000, 0x8000, CRC(a19678d2) SHA1(d13c12fa7e94333555eabf58b81bad421e21cd91))
	ROM_LOAD16_BYTE( "31020.hi", 0x10001, 0x8000, CRC(4922e020) SHA1(64e6448323dad2209e004cd93fa181582e768ed5))
ROM_END

ROM_START( aplanst )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "31024", "Bios 3-10-24")
	ROMX_LOAD("31024.lo", 0x10000, 0x8000, CRC(e52b59e1) SHA1(cfcaa4d8d658df8df463108ef30695bd4ee7a617), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("31024.hi", 0x10001, 0x8000, CRC(7286aefa) SHA1(dfc0e3f4936780fa62ae9ec392ce17aa65e717cd), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "31025", "Bios 3-10-25")
	ROMX_LOAD("31025.lo", 0x10000, 0x8000, CRC(1aec09bc) SHA1(51d56c97c7c1674554aa89b68945329ea967a8bc), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("31025.hi", 0x10001, 0x8000, CRC(0763caa5) SHA1(48510a933dcd6efea3b14d04444f584c3e6fefeb), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "31026", "Bios 3-10-26i")
	ROMX_LOAD("31026i.lo", 0x10000, 0x8000, CRC(670b6ab4) SHA1(8d61a0edf187f99b67eb58f5e11276deee801d17), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("31026i.hi", 0x10001, 0x8000, CRC(ef01c54f) SHA1(911f95d65ab96878e5e7ebccfc4b329db47a1351), ROM_SKIP(1) | ROM_BIOS(2))
ROM_END

ROM_START( aplannb )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "lsl31025.lo", 0x00000, 0x10000, CRC(8bb7229b) SHA1(31449d12884ec4e7752e6c1ce7ce9e0d044eadf2))
	ROM_LOAD16_BYTE( "lsh31025.hi", 0x00001, 0x10000, CRC(09e5c1b9) SHA1(d42be83b4181d3733268c29df04a4d2918370f4e))
ROM_END

ROM_START( apvxft )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "ft10221.lo", 0x00000, 0x10000, CRC(8f339de0) SHA1(a6542406746eaf1ff7f9e3678c5cbe5522fb314a))
	ROM_LOAD16_BYTE( "ft10221.hi", 0x00001, 0x10000, CRC(3b16bc31) SHA1(0592d1d81e7fd4715b0612083482db122d78c7f2))
ROM_END

ROM_START( aplscar )
	ROM_REGION(0x40000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "car306", "Caracal 3.06")
	ROMX_LOAD("car306.bin",   0x00000, 0x40000, CRC(fc271dea) SHA1(6207cfd312c9957243b8157c90a952404e43b237), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "car307", "Caracal 3.07")
	ROMX_LOAD("car307.bin",   0x00000, 0x40000, CRC(66a01852) SHA1(b0a68c9d67921d27ba483a1c50463406c08d3085), ROM_BIOS(1))
ROM_END

ROM_START( apxena1 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD("a1-r26.bin",   0x00000, 0x20000, CRC(d29e983e) SHA1(5977df7f8d7ac2a154aa043bb6f539d96d51fcad))
ROM_END

ROM_START( apxenp2 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "p2r02g2", "p2r02g2")
	ROMX_LOAD("p2r02g2.bin",   0x00000, 0x20000, CRC(311bcc5a) SHA1(be6fa144322077dcf66b065e7f4e61aab8c278b4), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "lep121s", "SCSI-Enabling ROMs")
	ROMX_LOAD("p2r01f0.bin",   0x00000, 0x20000, CRC(bbc68f2e) SHA1(6954a52a7dda5521794151aff7a04225e9c7df77), ROM_BIOS(1))
ROM_END

ROM_START( apxeni )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "lep121", "Rom Bios 1.2.1")
	ROMX_LOAD( "lep121.bin", 0x18000, 0x8000, CRC(948c1927) SHA1(d06bdbd6292db73c815ad1060daf055293dfddf5), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "lep121s", "SCSI-Enabling ROMs")
	ROMX_LOAD( "lep121s.bin", 0x18000, 0x8000, CRC(296118e4) SHA1(d1feaa9704e6ce3bc10c900bdd310d9494b02304), ROM_BIOS(1))
ROM_END

ROM_START( aplsbon )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "bon106", "Boinsai 1-06")
	ROMX_LOAD("bon106.bin",   0x00000, 0x20000, CRC(98a4eb76) SHA1(e0587afa78aeb9a8803f9b9f9e457e9847b0a2b2), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "bon203", "Boinsai 2-03")
	ROMX_LOAD("bon203.bin",   0x00000, 0x20000, CRC(32a0e125) SHA1(a4fcbd76952599993fa8b76aa36a96386648abb2), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "bon10703", "Boinsai 1-07-03")
	ROMX_LOAD("bon10703.bin",   0x00000, 0x20000, CRC(0275b3c2) SHA1(55ef4cbb7f3166f678aaa478234a42049deaba5f), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "bon20402", "Boinsai 2.03")
	ROMX_LOAD("bon20402.bin",   0x00000, 0x20000, CRC(ac5803fb) SHA1(b8fe92711c6a38a5d9e6497e76a0929c1685c631), ROM_BIOS(3))
ROM_END

ROM_START( apxlsam )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "sam107", "ROM BIOS Version 1-07")
	ROMX_LOAD("sam1-07.bin",   0x00000, 0x20000, CRC(65e05a8e) SHA1(c3cd198a129122cb05a28798e54331b06cfdd310), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "sam206", "ROM BIOS Version 2-06")
	ROMX_LOAD("sam2-06.bin",   0x00000, 0x20000, CRC(9768bb0f) SHA1(8166b77b133072f72f23debf85984eb19578ffc1), ROM_BIOS(1))
ROM_END

ROM_START( aprpand )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "pf10226.std", 0x00000, 0x10000, CRC(7396fb87) SHA1(a109cbad2179eec55f86c0297a59bb015461da21))
	ROM_CONTINUE( 0x00001, 0x10000 )
ROM_END

ROM_START( aprfte )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD( "1-2r2-4.486", 0x00000, 0x20000, CRC(bccc236d) SHA1(0765299363e68cf65710a688c360a087856ece8f))
ROM_END

ROM_START( megapc )
	ROM_REGION(0x40000, "isa", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "41651-bios lo.u18",  0x00000, 0x10000, CRC(1e9bd3b7) SHA1(14fd39ec12df7fae99ccdb0484ee097d93bf8d95))
	ROM_LOAD16_BYTE( "211253-bios hi.u19", 0x00001, 0x10000, CRC(6acb573f) SHA1(376d483db2bd1c775d46424e1176b24779591525))
ROM_END

ROM_START( megapcpl )
	ROM_REGION(0x40000, "isa", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "41652.u18",  0x00000, 0x10000, CRC(6f5b9a1c) SHA1(cae981a35a01234fcec99a96cb38075d7bf23474))
	ROM_LOAD16_BYTE( "486slc.u19", 0x00001, 0x10000, CRC(6fb7e3e9) SHA1(c439cb5a0d83176ceb2a3555e295dc1f84d85103))
ROM_END

ROM_START( megapcpla )
	ROM_REGION(0x40000, "bios", 0)
	ROM_LOAD( "megapc_bios.bin",  0x00000, 0x10000, CRC(b84938a2) SHA1(cecab72a96993db4f7c648c229b4211a8c53a380))
	ROM_CONTINUE(0x30000, 0x10000)
ROM_END

ROM_START( t2000sx )
	ROM_REGION( 0x20000, "bios", 0 )
	ROM_LOAD( "014d.ic9", 0x00000, 0x20000, CRC(e9010b02) SHA1(75688fc8e222640fa22bcc90343c6966fe0da87f))
ROM_END

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

// Kaypro 286i
ROM_START( k286i )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "81_1598", 0x18000, 0x4000, CRC(e25a1e43) SHA1(d00b976ac94323f3867b1c256e315839c906dd5a) )
	ROM_LOAD16_BYTE( "81_1599", 0x18001, 0x4000, CRC(08e2a17b) SHA1(a86ef116e82eb9240e60b52f76e5e510cdd393fd) )
ROM_END

// Sanyo MBC-28
ROM_START( mbc28 ) // Complains about missing mouse hardware
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "mbc-28_sl-dt_ver.1620_low_din_checksum,454f00,27c256-15.bin", 0x10000, 0x8000, CRC(423b4693) SHA1(08e877baa59ebd9a1817dcdd27138c638edcbb84) )
	ROM_LOAD16_BYTE( "mbc-28_sl-dt_ver.1620_high_din_checksum,45ae00,27c256-15.bin", 0x10001, 0x8000, CRC(557b7346) SHA1(c0dca88627f8451211172441fefb4020839fb87f) )
ROM_END

// Siemens PCD-2
ROM_START( pcd2 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "bios_tandon_188782-032a_rev_5.21_low.bin", 0x10000, 0x8000, CRC(a8fbffd3) SHA1(8a3ad5bc7f86ff984be10a8b1ae4542be4c80e5f) )
	ROM_LOAD16_BYTE( "bios_tandon_188782-031a_rev_5.21_high.bin", 0x10001, 0x8000, CRC(8d7dfdcc) SHA1(d1d58c0ad7db60399f9a93db48feb10e44ffd624) )
	// ROM_LOAD( "kbd_8742_award_upi_1.61_rev_1.01.bin", 0x0000, 0x0800, CRC(bb8a1979) SHA(43d35ecf76e5e8d5ddf6c32b0f6f628a7542d6e4) ) // 8742 keyboard controller
	// ROM_LOAD( "vga_nmc27c256q_435-0029-04_1988_video7_arrow.bin", 0x8000, 0x0800, CRC(0d8d7dff) SHA(cb5b2ab78d480ec3164d16c9c75f1449fa81a0e7) ) // Video7 VGA card
	// ROM_LOAD( "vga_nmc27c256q_435-0030-04_1988_video7_arrow.bin", 0x8000, 0x0800, CRC(0935c003) SHA(35ac571818f616b856da8bbf6a7a9172f68b3ab6) )
ROM_END

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

// Ericsson WS286
ROM_START(ews286 ) // Computer is brown/yellow-ish with Ericsson logo
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "rys_103_1002_r8a_3c00_ic-pos_71.bin", 0x18000, 0x4000, CRC(af179e56) SHA1(58b1df46d6e68eef472a0529cb9317abaf17880f)) // Last ROM set and has Nokia
	ROM_LOAD16_BYTE( "rys_103_1003_r8a_8600_ic-pos_69.bin", 0x18001, 0x4000, CRC(555502cb) SHA1(1977fe54b69c5e52731bf3eb8bdabe777aac014b)) // copyright patched in both roms
ROM_END

// Nokia Data WS286
//ROM_START(nws286 ) // Computer is grey with Nokia logo.
//  ROM_REGION(0x20000,"bios", 0)
//  ROM_LOAD16_BYTE( "rys_103_1002_r8a_3c00_ic-pos_71.bin", 0x18000, 0x4000, NO_DUMP)
//  ROM_LOAD16_BYTE( "rys_103_1003_r8a_8600_ic-pos_69.bin", 0x18001, 0x4000, NO_DUMP)
//ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME       PARENT   COMPAT   MACHINE    INPUT  CLASS         INIT            COMPANY        FULLNAME                FLAGS
COMP( 1984, ibm5170,   0,       ibm5150, ibm5170,   0,     at_state,     init_at,        "International Business Machines",  "IBM PC/AT 5170", MACHINE_NOT_WORKING )
COMP( 1985, ibm5170a,  ibm5170, 0,       ibm5170a,  0,     at_state,     init_at,        "International Business Machines",  "IBM PC/AT 5170 8MHz", MACHINE_NOT_WORKING )
COMP( 1985, ibm5162,   ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "International Business Machines",  "IBM PC/XT-286 5162", MACHINE_NOT_WORKING )
COMP( 1989, ibmps1es,  ibm5170, 0,       ibmps1,    0,     at_vrom_fix_state, init_at,   "International Business Machines",  "IBM PS/1 (Spanish)", MACHINE_NOT_WORKING )
COMP( 1987, at,        ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "<generic>",   "PC/AT (CGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1987, atvga,     ibm5170, 0,       atvga,     0,     at_state,     init_at,        "<generic>",   "PC/AT (VGA, MF2 Keyboard)" , MACHINE_NOT_WORKING )
COMP( 1988, at386,     ibm5170, 0,       at386,     0,     at_state,     init_at,        "<generic>",   "PC/AT 386 (VGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1988, ct386sx,   ibm5170, 0,       ct386sx,   0,     at_state,     init_at,        "<generic>",   "NEAT 386SX (VGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1988, at386sx,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "<generic>",   "PC/AT 386SX (VGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1990, at486,     ibm5170, 0,       at486,     0,     at_state,     init_at,        "<generic>",   "PC/AT 486 (VGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1989, neat,      ibm5170, 0,       neat,      0,     at_state,     init_at,        "<generic>",   "NEAT (VGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1989, ec1842,    ibm5150, 0,       ec1842,    0,     at_state,     init_at,        "<unknown>",   "EC-1842", MACHINE_NOT_WORKING )
COMP( 1993, ec1849,    ibm5170, 0,       ec1842,    0,     at_state,     init_at,        "<unknown>",   "EC-1849", MACHINE_NOT_WORKING )
COMP( 1993, megapc,    0,       0,       megapc,    0,     megapc_state, init_megapc,    "Amstrad plc", "MegaPC", MACHINE_NOT_WORKING )
COMP( 199?, megapcpl,  megapc,  0,       megapcpl,  0,     megapc_state, init_megapcpl,  "Amstrad plc", "MegaPC Plus", MACHINE_NOT_WORKING )
COMP( 199?, megapcpla, megapc,  0,       megapcpla, 0,     at_vrom_fix_state, init_megapcpla, "Amstrad plc", "MegaPC Plus (WINBUS chipset)", MACHINE_NOT_WORKING )
COMP( 1989, pc2386,    ibm5170, 0,       at386l,    0,     at_state,     init_at,        "Amstrad plc", "Amstrad PC2386", MACHINE_NOT_WORKING )
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
COMP( 1990, c386sx16,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Commodore Business Machines", "Commodore 386SX-16", MACHINE_NOT_WORKING )
COMP( 1988, pc30iii,   ibm5170, 0,       pc30iii,   0,     at_state,     init_at,        "Commodore Business Machines",  "PC 30-III", MACHINE_NOT_WORKING )
COMP( 1988, pc40iii,   ibm5170, 0,       pc40iii,   0,     at_state,     init_at,        "Commodore Business Machines",  "PC 40-III", MACHINE_NOT_WORKING )
COMP( 1995, ficpio2,   ibm5170, 0,       ficpio2,   0,     at_state,     init_atpci,     "FIC", "486-PIO-2", MACHINE_NOT_WORKING )
COMP( 1985, k286i,     ibm5170, 0,       k286i,     0,     at_state,     init_at,        "Kaypro",      "286i", MACHINE_NOT_WORKING )
COMP( 1991, t2000sx,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Toshiba",     "T2000SX", MACHINE_NOT_WORKING )
COMP( 199?, mbc28,     ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Sanyo",       "MBC-28", MACHINE_NOT_WORKING )
COMP( 1986, pcd2,      ibm5170, 0,       ibm5170,   0,     at_state,     init_at,        "Siemens",     "PCD-2", MACHINE_NOT_WORKING )
COMP( 1987, comportii ,ibm5170, 0,       comportii, 0,     at_state,     init_at,        "Compaq",      "Portable II", MACHINE_NOT_WORKING )
COMP( 1987, comportiii,ibm5170, 0,       comportiii,0,     at_state,     init_at,        "Compaq",      "Portable III", MACHINE_NOT_WORKING )
COMP( 1986, ews286,    ibm5170, 0,       ews286,    0,     at_state,     init_at,        "Ericsson",    "Ericsson WS286", MACHINE_NOT_WORKING )
//COMP( 1988, nws286,    ibm5170,  0,      ews286,    0,     at_state,     at,        "Nokia Data",  "Nokia Data WS286", MACHINE_NOT_WORKING )
