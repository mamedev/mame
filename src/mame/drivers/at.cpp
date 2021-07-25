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

***************************************************************************/

#include "emu.h"

/* mingw-gcc defines this */
#ifdef i386
#undef i386
#endif /* i386 */

#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/i386/i386.h"
#include "cpu/i86/i286.h"
#include "machine/at.h"
#include "machine/cs8221.h"
#include "machine/ds128x.h"
#include "machine/nvram.h"
#include "machine/ram.h"
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
	void comportii(machine_config &config);
	void comportiii(machine_config &config);
	void comslt286(machine_config &config);
	void dsys200(machine_config &config);
	void ibm5162(machine_config &config);
	void neat(machine_config &config);
	void ibm5170a(machine_config &config);
	void ec1842(machine_config &config);
	void at386sx(machine_config &config);
	void pc40iii(machine_config &config);
	void pc45iii(machine_config &config);
	void c286lt(machine_config &config);
	void csl286(machine_config &config);
	void c386sx16(machine_config &config);
	void atturbo(machine_config &config);
	void m290(machine_config &config);
	void ncrpc8(machine_config &config);
	void n8810m15(machine_config &config);
	void n8810m55(machine_config &config);
	void ews286(machine_config &config);
	void olyport40(machine_config &config);
	void micral45(machine_config &config);
	void euroat(machine_config &config);

	void init_at();

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
	void at16_io(address_map &map);
	void at16_map(address_map &map);
	void at16l_map(address_map &map);
	void neat_io(address_map &map);
	void ps1_16_io(address_map &map);
};

class at_vrom_fix_state : public at_state
{
public:
	using at_state::at_state;

	void ibmps1(machine_config &config);

protected:
	virtual void machine_start() override;
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
		space.install_ram(0x100000,  ram_limit - 1, m_ram->pointer() + xmsbase);
	}
}

void at_state::init_at()
{
	init_at_common(0xa0000);
}

void at_vrom_fix_state::machine_start()
{
	at_state::machine_start();

	address_space& space = m_maincpu->space(AS_PROGRAM);
	space.install_rom(0xc0000, 0xcffff, machine().root_device().memregion("bios")->base());
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

// Schneider EuroAT
void at_state::euroat(machine_config &config)
{
	ibm5170(config);
	m_maincpu->set_clock(24_MHz_XTAL / 2); // Bus speed can be set up to CPU speed

	subdevice<isa16_slot_device>("isa2")->set_option_machine_config("fdc", cfg_single_1440K); // From pictures but also with a 3.5" as second floppy

	m_ram->set_default_size("640K");
}


/***************************************************************************
  ROM DEFINITIONS
***************************************************************************/

/***************************************************************************
  IBM systems
***************************************************************************/

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


/***************************************************************************
  Apricot systems

  http://bbs.actapricot.org/files/ , http://insight.actapricot.org/insight/products/main.htm

***************************************************************************/

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


/***************************************************************************
  Commodore systems
***************************************************************************/

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
	// 16: BIOS-String: Pyramid Software Development Personal Computer AT Bios Version 2.14
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
	ROM_LOAD16_BYTE( "m205.bin", 0x10000, 0x8000, CRC(6f7bc8d6) SHA1(14062505b316e0d4409fb4e502651e09fea0a4c1) )
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
	ROM_SYSTEM_BIOS(7, "ami211", "AMI 21.1") /*(Motherboard Manufacturer: Dataexpert Corp. Motherboard) (Neat 286 Bios, 82c21x Chipset ) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "ami211.bin",     0x10000, 0x10000,CRC(a0b5d269) SHA1(44db8227d35a09e39b93ed944f85dcddb0dd0d39), ROM_BIOS(7))
	// 8: BIOS-String: ENET-1230-043089-K0
	ROM_SYSTEM_BIOS(8, "amic21", "AMI C 21.1") /* (Motherboard Manufacturer: Unknown.) (Neat 286 Bios, 82c21x Chipset ) (BIOS release date:: 30-04-1989) */
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
	ROM_SYSTEM_BIOS(2, "ami101", "AMI HT 101.1") /* (Quadtel Enhanced 286 Bios Version 3.04.02) (BIOS release date:: 09/11/1989) */
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


//**************************************************************************
//  80286 Desktop
//**************************************************************************

// SIIG MiniSys 2000 - Motherboard ID: Labway MS101V1.2 - This is a tiny 286 system, the width of a 3.5" disk drive with an external power supply.
// A physical switch allows to change between VGA and composite video output.
// CPU: Intel 286-12 - BIOS: AMIBIOS - Bios string: DH12-1422-061390-K0 - Chipset: Headland HT12/A, Acer M5105 - RAM: 2xSIMM30
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

// Olivetti M290 - has an Olivetti branded Paradise PVGA1A-JK VGA card - locks up with "Error 2" and a key symbol
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


/***************************************************************************
  80386 SX BIOS
***************************************************************************/

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


/**************************************************************************
  80386 SX motherboard
**************************************************************************/

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


/***** 386sx motherboards using the Chips SCAMPSX chipset *****/

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


/***** 386sx motherboards using the ALi 1217 chipset *****/

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


/***** 386sx motherboards using the Headland HT18/C chipset *****/

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


/***** 386sx motherboards using the Opti F82C206, Opti 82C283 chipset *****/

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

/***** 386sx motherboards using the OPTi 82C291 chipset *****/

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


/***** 386sx motherboards using the SARC (or CYCLONE) RC2016A5 chipset *****/

// Pine PT-319A rev2.2a - CPU: 386sx - BIOS: AMI; 06/06/92
// BIOS-String: X0-0100-000000-00101111-060692-386SX-0 / Ver 5.20
ROM_START( pt319a )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "3sam001.bin", 0x10000, 0x10000, CRC(cad22030) SHA1(85bb6027579a87bfe7ea0f7df3676fdaa64920ac))
ROM_END


// CX Technology, Inc. Model SXD (4x SIMM30, Cache: 64/128/256KB in 2 banks plus TAG, 4x 16-bit ISA) - Chipset: SARC RC2016A5; HM6818P; CX109; LT38C41 © Lance Corp. (keyboard controller?)
// additional info from chukaev.ru54.com: Chipset: CYCLONE RC2016A5 - ISA16: 6 - ROM: CX109 340C3A62D0A - CPU/FPU: Am386SX/SXL-33, 387
ROM_START( cxsxd )  // BIOS-String: 03/25/93-SARC_RC2016A-219v0000 / CX 386SX System
	ROM_REGION16_LE(0x20000,"bios", 0)
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


/**************************************************************************
  80386 SX Desktop
**************************************************************************/

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


/**************************************************************************
  80386 SX Laptop/Notebook
**************************************************************************/

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


/***************************************************************************
  Game driver(s)
***************************************************************************/

//    YEAR  NAME       PARENT   COMPAT   MACHINE    INPUT  CLASS         INIT            COMPANY        FULLNAME                FLAGS
COMP( 198?, asi100b0,  ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "<unknown",    "ASI 100B0, identified as HAM 12 TI 286 Motherboard ZERO WAIT", MACHINE_NOT_WORKING )
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
COMP( 19??, ht12a,     ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "unknown",     "unknown 286 AT clones (HT12/A chipset)", MACHINE_NOT_WORKING )
COMP( 1985, ibm5162,   ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "International Business Machines",  "PC/XT-286 5162", MACHINE_NOT_WORKING )
COMP( 1984, ibm5170,   0,       ibm5150, ibm5170,   0,     at_state,     init_at,        "International Business Machines",  "PC/AT 5170", MACHINE_NOT_WORKING )
COMP( 1985, ibm5170a,  ibm5170, 0,       ibm5170a,  0,     at_state,     init_at,        "International Business Machines",  "PC/AT 5170 8MHz", MACHINE_NOT_WORKING )
COMP( 1989, ibm2011,   ibm5170, 0,       ibmps1,    0,     at_vrom_fix_state, init_at,   "International Business Machines",  "PS/1 2011", MACHINE_NOT_WORKING )
COMP( 1989, ibm2011rd, ibm5170, 0,       ibmps1,    0,     at_vrom_fix_state, init_at,   "International Business Machines",  "PS/1 2011 (international models with ROM DOS)", MACHINE_NOT_WORKING )
COMP( 198?, icldrsm40, ibm5170, 0,       neat,      0,     at_state,     init_at,        "ICL", "DRS M40", MACHINE_NOT_WORKING )
COMP( 1985, k286i,     ibm5170, 0,       k286i,     0,     at_state,     init_at,        "Kaypro",      "286i", MACHINE_NOT_WORKING )
COMP( 199?, kma202f,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "<unknown>",   "KMA-202F-12R (Winbond chipset)", MACHINE_NOT_WORKING )
COMP( 19??, kt216wb5,  ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "KT Technology", "KT216WB5-HI Rev.2", MACHINE_NOT_WORKING )
COMP( 198?, lm103s,    ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "unknown",     "LM-103S", MACHINE_NOT_WORKING )
COMP( 1987, m290,      ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Olivetti",    "M290", MACHINE_NOT_WORKING )
COMP( 198?, magb233,   ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "Magitronic Technology", "Magitronic B233", MACHINE_NOT_WORKING )
COMP( 198?, magb236,   ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "Magitronic Technology", "Magitronic B236", MACHINE_NOT_WORKING )
COMP( 19??, mat286,    ibm5170, 0,       ibm5162,   0,     at_state,     init_at,        "unknown",     "MAT286 Rev.D", MACHINE_NOT_WORKING )
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
COMP( 198?, peas286,   ibm5170, 0,       atturbo,   0,     at_state,     init_at,        "Peacock computer", "S-286 Rev A", MACHINE_NOT_WORKING )
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
COMP( 198?, eltp9,     ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Chaintech", "ELT-P9 / Most likely ELT-386SX-160D", MACHINE_NOT_WORKING )
COMP( 198?, epsax3,    ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Epson",       "PC AX3", MACHINE_NOT_WORKING )
COMP( 19??, ht18c,     ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "<unknown>",   "unknown 386sx AT clones (HT18/C chipset)", MACHINE_NOT_WORKING )
COMP( 199?, ibm2121,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "International Business Machines",  "PS/1 2121", MACHINE_NOT_WORKING )
COMP( 199?, ibm2121rd, ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "International Business Machines",  "PS/1 2121 (international models with ROM DOS)", MACHINE_NOT_WORKING )
COMP( 199?, ibm2123,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "International Business Machines",  "PS/1 2123", MACHINE_NOT_WORKING )
COMP( 199?, ilm396b,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "ILON USA, INC.", "M-396B", MACHINE_NOT_WORKING )
COMP( 198?, ktx20t02,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Quadtel",     "QTC-SXM KT X20T02/HI Rev.3", MACHINE_NOT_WORKING )
COMP( 199?, mbc18nb,   ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Sanyo",       "MBC-18NB", MACHINE_NOT_WORKING )
COMP( 1992, mbc28,     ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "Sanyo",       "MBC-28", MACHINE_NOT_WORKING ) // Complains about missing mouse hardware
COMP( 199?, mmbo4088,  ibm5170, 0,       at386sx,   0,     at_state,     init_at,        "<unknown>",   "486MMBO4088 (TI TX486SLC/E", MACHINE_NOT_WORKING )
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
