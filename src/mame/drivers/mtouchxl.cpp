// license:BSD-3-Clause
// copyright-holders:R. Belmont, Wilbert Pol, Miodrag Milanovic
/***************************************************************************

  mtouchxl.cpp: Merit Industries MegaTouch XL
  
  Hardware includes a base 486? PC with VGA and a customized ISA I/O
  card.  The I/O card includes audio and an option ROM which patches int 19h
  (POST Completed) to instead jump back to the option ROM which loads
  "ROM-DOS", installs drivers for the Microtouch screen, and then boots 
  from the CD-ROM drive.
  
  Currently: ROM-DOS loads and installs virtual drive C:, but things go wrong
  in a hurry after that :)
  
***************************************************************************/

/* mingw-gcc defines this */
#ifdef i386
#undef i386
#endif /* i386 */

#include "emu.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "cpu/i386/i386.h"
#include "cpu/i86/i286.h"
#include "machine/at.h"
#include "machine/cs8221.h"
#include "machine/ds128x.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/vt82c496.h"
#include "machine/wd7600.h"
#include "speaker.h"

class mt6k_state : public driver_device
{
public:
	mt6k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb(*this, "mb"),
		m_ram(*this, RAM_TAG)
		{ }
	required_device<cpu_device> m_maincpu;
	required_device<at_mb_device> m_mb;
	required_device<ram_device> m_ram;
	DECLARE_DRIVER_INIT(at);

	void init_at_common(int xmsbase);
};

class megapc_state : public driver_device
{
public:
	megapc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_wd7600(*this, "wd7600"),
		m_isabus(*this, "isabus"),
		m_speaker(*this, "speaker")
		{ }

public:
	required_device<cpu_device> m_maincpu;
	required_device<wd7600_device> m_wd7600;
	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;
};

static ADDRESS_MAP_START( at32_map, AS_PROGRAM, 32, mt6k_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAMBANK("bank10")
	AM_RANGE(0x000d0000, 0x000dffff) AM_ROM AM_REGION("ioboard", 0)
	AM_RANGE(0x000e0000, 0x000fffff) AM_ROM AM_REGION("bios", 0)
	AM_RANGE(0x00800000, 0x00800bff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( at32_io, AS_IO, 32, mt6k_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x00ff) AM_DEVICE16("mb", at_mb_device, map, 0xffffffff)
ADDRESS_MAP_END

/**********************************************************
 *
 * Init functions
 *
 **********************************************************/

void mt6k_state::init_at_common(int xmsbase)
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

DRIVER_INIT_MEMBER(mt6k_state,at)
{
	init_at_common(0xa0000);
}

static MACHINE_CONFIG_START( at486, mt6k_state )
	MCFG_CPU_REPLACE("maincpu", I486, 25000000)
	MCFG_CPU_PROGRAM_MAP(at32_map)
	MCFG_CPU_IO_MAP(at32_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259_master", pic8259_device, inta_cb)

	MCFG_DEVICE_ADD("mb", AT_MB, 0)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))
	MCFG_FRAGMENT_ADD(at_softlists)
	MCFG_NVRAM_ADD_0FILL("nvram")

	// on board devices
	MCFG_ISA16_SLOT_ADD("mb:isabus","board1", pc_isa16_cards, "fdcsmc", true)
	MCFG_ISA16_SLOT_ADD("mb:isabus","board2", pc_isa16_cards, "comat", true)
	MCFG_ISA16_SLOT_ADD("mb:isabus","board3", pc_isa16_cards, "ide", true)
	MCFG_ISA16_SLOT_ADD("mb:isabus","board4", pc_isa16_cards, "lpt", true)
	// ISA cards
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa1", pc_isa16_cards, "vga", false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa2", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa3", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa4", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa5", pc_isa16_cards, nullptr, false)
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("8M")	// Early XL games had 8 MB RAM, later ones require 32MB
	MCFG_RAM_EXTRA_OPTIONS("32M")
MACHINE_CONFIG_END

ROM_START( mtchxl6k )
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD("at486.bin",   0x10000, 0x10000, CRC(31214616) SHA1(51b41fa44d92151025fc9ad06e518e906935e689))
	
	ROM_REGION(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-04_u12-r00.u12", 0x000000, 0x100000, CRC(2a6fbca4) SHA1(186eb052cb9b77ffe6ee4cb50c1b580532fd8f47) ) 
	
	DISK_REGION("board3:ide:ide:0:cdrom:image")
	DISK_IMAGE_READONLY("r02", 0, SHA1(eaaf26d2b700f16138090de7f372b40b93e8dba9))
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*     YEAR  NAME      PARENT   COMPAT   MACHINE    INPUT       INIT    COMPANY     FULLNAME */
COMP ( 1990, mtchxl6k,      0,    0,       at486,     0,    mt6k_state,      at,      "Merit Industries",  "MegaTouch XL 6000", MACHINE_NOT_WORKING )
