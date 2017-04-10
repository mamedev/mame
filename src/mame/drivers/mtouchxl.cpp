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

#include "emu.h"
#include "bus/isa/isa_cards.h"
#include "cpu/i386/i386.h"
#include "machine/at.h"
#include "machine/ram.h"
#include "machine/8042kbdc.h"
#include "machine/nvram.h"
#include "machine/ins8250.h"
#include "machine/microtch.h"
#include "machine/atapicdr.h"
#include "speaker.h"

class mt6k_state : public driver_device
{
public:
	mt6k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb(*this, "mb"),
		m_ram(*this, RAM_TAG),
		m_rombank(*this, "rombank")
		{ }
	required_device<cpu_device> m_maincpu;
	required_device<at_mb_device> m_mb;
	required_device<ram_device> m_ram;
	required_memory_bank m_rombank;
	void machine_start() override;
	void machine_reset() override;
	DECLARE_WRITE8_MEMBER(bank_w);
};

WRITE8_MEMBER(mt6k_state::bank_w)
{
	m_rombank->set_entry(data & 0xf);
}

static ADDRESS_MAP_START( at32_map, AS_PROGRAM, 32, mt6k_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAMBANK("bank10")
	AM_RANGE(0x000d0000, 0x000dffff) AM_ROMBANK("rombank")
	AM_RANGE(0x000e0000, 0x000fffff) AM_ROM AM_REGION("bios", 0)
	AM_RANGE(0x00800000, 0x00800bff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( at32_io, AS_IO, 32, mt6k_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8("mb:dma8237_1", am9517a_device, read, write, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8("mb:pic8259_master", pic8259_device, read, write, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8("mb:pit8254", pit8254_device, read, write, 0xffffffff)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE8("mb", at_mb_device, portb_r, portb_w, 0x0000ff00)
	AM_RANGE(0x0060, 0x0067) AM_DEVREADWRITE8("kbdc", kbdc8042_device, data_r, data_w, 0xffffffff)
	AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE8("mb:rtc", mc146818_device, read, write, 0xffffffff)
	AM_RANGE(0x0080, 0x009f) AM_DEVREADWRITE8("mb", at_mb_device, page8_r, page8_w, 0xffffffff)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8("mb:pic8259_slave", pic8259_device, read, write, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_DEVREADWRITE8("mb:dma8237_2", am9517a_device, read, write, 0x00ff00ff)
	AM_RANGE(0x022c, 0x022f) AM_WRITE8(bank_w, 0xff000000)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE8("ns16550", ns16550_device, ins8250_r, ins8250_w, 0xffffffff)
ADDRESS_MAP_END

/**********************************************************
 *
 * Init functions
 *
 *********************************************************/

void mt6k_state::machine_start()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);

	/* MESS managed RAM */
	membank("bank10")->set_base(m_ram->pointer());

	if (m_ram->size() > 0xa0000)
	{
		offs_t ram_limit = 0x100000 + m_ram->size() - 0xa0000;
		space.install_read_bank(0x100000,  ram_limit - 1, "bank1");
		space.install_write_bank(0x100000,  ram_limit - 1, "bank1");
		membank("bank1")->set_base(m_ram->pointer() + 0xa0000);
	}
	m_rombank->configure_entries(0, 16, memregion("ioboard")->base(), 0x10000);
}

void mt6k_state::machine_reset()
{
	m_rombank->set_entry(0);
}

static SLOT_INTERFACE_START(mt6k_ata_devices)
	SLOT_INTERFACE("cdrom", ATAPI_FIXED_CDROM)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT(cdrom)
	MCFG_DEVICE_MODIFY("ide:0")
	MCFG_DEVICE_SLOT_INTERFACE(mt6k_ata_devices, "cdrom", true)
	MCFG_DEVICE_MODIFY("ide:1")
	MCFG_SLOT_DEFAULT_OPTION("")
	MCFG_SLOT_FIXED(true)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( at486, mt6k_state )
	MCFG_CPU_ADD("maincpu", I486, 25000000)
	MCFG_CPU_PROGRAM_MAP(at32_map)
	MCFG_CPU_IO_MAP(at32_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259_master", pic8259_device, inta_cb)

	MCFG_DEVICE_ADD("mb", AT_MB, 0)
	MCFG_NVRAM_ADD_0FILL("nvram")

	// on board devices
	MCFG_ISA16_SLOT_ADD("mb:isabus","board1", pc_isa16_cards, "ide", true)
	MCFG_SLOT_OPTION_MACHINE_CONFIG("ide", cdrom)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa1", pc_isa16_cards, "vga", true)

	MCFG_DEVICE_ADD("ns16550", NS16550, XTAL_1_8432MHz)
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE("microtouch", microtouch_device, rx))
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE("mb:pic8259_master", pic8259_device, ir4_w))
	MCFG_MICROTOUCH_ADD("microtouch", 9600, DEVWRITELINE("ns16550", ins8250_uart_device, rx_w))

	// remove the keyboard controller and use the HLE one which allow keys to be unmapped
	MCFG_DEVICE_REMOVE("mb:keybc");
	MCFG_DEVICE_REMOVE("mb:pc_kbdc");
	MCFG_DEVICE_ADD("kbdc", KBDC8042, 0)
	MCFG_KBDC8042_KEYBOARD_TYPE(KBDC8042_AT386)
	MCFG_KBDC8042_SYSTEM_RESET_CB(INPUTLINE("maincpu", INPUT_LINE_RESET))
	MCFG_KBDC8042_GATE_A20_CB(INPUTLINE("maincpu", INPUT_LINE_A20))
	MCFG_KBDC8042_INPUT_BUFFER_FULL_CB(DEVWRITELINE("mb:pic8259_master", pic8259_device, ir1_w))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("8M")	// Early XL games had 8 MB RAM, later ones require 32MB
	MCFG_RAM_EXTRA_OPTIONS("32M")
MACHINE_CONFIG_END

ROM_START( mtchxl6k )
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD("prom.mb", 0x10000, 0x10000, BAD_DUMP CRC(e44bfd3c) SHA1(c07ec94e11efa30e001f39560010112f73cc0016) ) // isn't the original bios
	
	ROM_REGION(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-04_u12-r00.u12", 0x000000, 0x100000, CRC(2a6fbca4) SHA1(186eb052cb9b77ffe6ee4cb50c1b580532fd8f47) ) 
	
	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r02", 0, SHA1(eaaf26d2b700f16138090de7f372b40b93e8dba9))
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*     YEAR  NAME      PARENT   COMPAT   MACHINE    INPUT       INIT    COMPANY     FULLNAME */
COMP ( 1990, mtchxl6k,      0,    0,       at486,     at_keyboard,    driver_device,      0,      "Merit Industries",  "MegaTouch XL 6000", MACHINE_NOT_WORKING )
