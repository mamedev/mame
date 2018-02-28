// license:BSD-3-Clause
// copyright-holders: Carl, R. Belmont, Wilbert Pol, Miodrag Milanovic
/***************************************************************************

  mtouchxl.cpp: Merit Industries MegaTouch XL

  Hardware includes a base 486 PC with VGA and a customized ISA I/O
  card.  The I/O card includes audio and an option ROM which patches int 19h
  (POST Completed) to instead jump back to the option ROM which loads
  "ROM-DOS", installs drivers for the Microtouch screen, and then boots
  from the CD-ROM drive.

  Audio is a CS4231 combination CODEC/Mixer also found in Gravis Ultraound MAX
  and some SPARCstations.

  Some boards use the DS1205 chip for security, others use the DS1991 iButton

  Megatouch XL (Software) (* indicated verified dumps of CD + Boot ROM,
                           - means we have it working but would like a redump)
  Megatouch XL (1997) (CD versions: R0, R0A, R0B, R0C, R0D, *R1, R2, R3, R3A, R3B, R3C)
  Megatouch XL 5000 (1998) (CD versions: R5A, *R5B, R5D, *R5E, R5G, R5H, *R5I)
  Megatouch XL 6000 (1999) (CD versions: *R02, *R04, R05, *R07)
  Megatouch XL Gold (2000) (CD versions: *R00, *R01.  HDD versions: R01)
  Megatouch XL Platinum / Double Platinum (2001)
  Megatouch XL Titanium / Titanium 2 (2002)

***************************************************************************/

// use under construction modern PCI SiS 85c496/497 chipset
//#define REAL_PCI_CHIPSET

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
#include "machine/bankdev.h"
#include "machine/intelfsh.h"
#include "machine/ds128x.h"
#include "machine/ds1205.h"
#ifdef REAL_PCI_CHIPSET
#include "machine/sis85c496.h"
#endif
#include "sound/ad1848.h"
#include "speaker.h"

class mtxl_state : public driver_device
{
public:
	mtxl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
#ifndef REAL_PCI_CHIPSET
		m_mb(*this, "mb"),
#endif
		m_ram(*this, RAM_TAG),
		m_iocard(*this, "dbank"),
		m_multikey(*this, "multikey")
		{ }
	required_device<cpu_device> m_maincpu;
#ifndef REAL_PCI_CHIPSET
	required_device<at_mb_device> m_mb;
#endif
	required_device<ram_device> m_ram;
	required_device<address_map_bank_device> m_iocard;
	optional_device<ds1205_device> m_multikey;
	void machine_start() override;
	void machine_reset() override;
	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_READ8_MEMBER(key_r);
	DECLARE_WRITE8_MEMBER(key_w);
	DECLARE_READ8_MEMBER(coin_r);
	void at486(machine_config &config);
	static void cdrom(device_t *device);
	void at32_io(address_map &map);
	void at32_map(address_map &map);
	void dbank_map(address_map &map);
};

WRITE8_MEMBER(mtxl_state::bank_w)
{
	m_iocard->set_bank(data & 0x1f);
}

READ8_MEMBER(mtxl_state::key_r)
{
	return m_multikey->read_dq() ? 0xff : 0xdf;
}

READ8_MEMBER(mtxl_state::coin_r)
{
	return ioport("Coin")->read();
}

WRITE8_MEMBER(mtxl_state::key_w)
{
	m_multikey->write_rst((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
	m_multikey->write_clk((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);
	m_multikey->write_dq((data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
}

ADDRESS_MAP_START(mtxl_state::at32_map)
	ADDRESS_MAP_UNMAP_HIGH
	#ifndef REAL_PCI_CHIPSET
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAMBANK("bank10")
	AM_RANGE(0x000c8000, 0x000cffff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x000d0000, 0x000dffff) AM_DEVICE("dbank", address_map_bank_device, amap32)
	AM_RANGE(0x000e0000, 0x000fffff) AM_ROM AM_REGION("bios", 0)
	AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)
	#endif
ADDRESS_MAP_END

ADDRESS_MAP_START(mtxl_state::at32_io)
	ADDRESS_MAP_UNMAP_HIGH
	#ifndef REAL_PCI_CHIPSET
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8("mb:dma8237_1", am9517a_device, read, write, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8("mb:pic8259_master", pic8259_device, read, write, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8("mb:pit8254", pit8254_device, read, write, 0xffffffff)
	AM_RANGE(0x0060, 0x0067) AM_DEVREADWRITE8("kbdc", kbdc8042_device, data_r, data_w, 0xffffffff)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE8("mb", at_mb_device, portb_r, portb_w, 0x0000ff00)
	AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE8("mb:rtc", mc146818_device, read, write, 0xffffffff)
	AM_RANGE(0x0080, 0x009f) AM_DEVREADWRITE8("mb", at_mb_device, page8_r, page8_w, 0xffffffff)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8("mb:pic8259_slave", pic8259_device, read, write, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_DEVREADWRITE8("mb:dma8237_2", am9517a_device, read, write, 0x00ff00ff)
	AM_RANGE(0x0224, 0x0227) AM_DEVREADWRITE8("cs4231", ad1848_device, read, write, 0xffffffff)
	#endif
	AM_RANGE(0x0228, 0x022b) AM_READ_PORT("Unknown")
	AM_RANGE(0x022c, 0x022f) AM_WRITE8(bank_w, 0xff000000)
	AM_RANGE(0x022c, 0x022f) AM_READWRITE8(key_r, key_w, 0x0000ff00)
	AM_RANGE(0x022c, 0x022f) AM_READ8(coin_r, 0x000000ff)
	#ifndef REAL_PCI_CHIPSET
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE8("ns16550", ns16550_device, ins8250_r, ins8250_w, 0xffffffff)
	#endif
ADDRESS_MAP_END

ADDRESS_MAP_START(mtxl_state::dbank_map)
	AM_RANGE(0x000000, 0x0fffff) AM_ROM AM_REGION("ioboard", 0)
	AM_RANGE(0x100000, 0x17ffff) AM_DEVREADWRITE8("flash", intelfsh8_device, read, write, 0xffffffff)
ADDRESS_MAP_END

static INPUT_PORTS_START(mtouchxl)
	PORT_START("Coin")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_SERVICE1) PORT_NAME("Setup")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_SERVICE2) PORT_NAME("Calibrate")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN2)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN3)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN4)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_START("Unknown")
	PORT_BIT(0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END

/**********************************************************
 *
 * Init functions
 *
 *********************************************************/

void mtxl_state::machine_start()
{
#ifndef REAL_PCI_CHIPSET
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
#endif
}

void mtxl_state::machine_reset()
{
	m_iocard->set_bank(0);
}

#ifndef REAL_PCI_CHIPSET
static SLOT_INTERFACE_START(mt6k_ata_devices)
	SLOT_INTERFACE("cdrom", ATAPI_FIXED_CDROM)
SLOT_INTERFACE_END

void mtxl_state::cdrom(device_t *device)
{
	auto ide0 = device->subdevice("ide:0");
	device_slot_interface::static_option_reset(*ide0);
	SLOT_INTERFACE_NAME(mt6k_ata_devices)(ide0);
	device_slot_interface::static_set_default_option(*ide0, "cdrom");
	device_slot_interface::static_set_fixed(*ide0, true);

	auto ide1 = device->subdevice("ide:1");
	device_slot_interface::static_set_default_option(*ide1, "");
	device_slot_interface::static_set_fixed(*ide1, true);
MACHINE_CONFIG_END
#endif

MACHINE_CONFIG_START(mtxl_state::at486)
	MCFG_CPU_ADD("maincpu", I486DX4, 33000000)
	MCFG_CPU_PROGRAM_MAP(at32_map)
	MCFG_CPU_IO_MAP(at32_io)
#ifndef REAL_PCI_CHIPSET
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259_master", pic8259_device, inta_cb)

	MCFG_DEVICE_ADD("mb", AT_MB, 0)
	MCFG_NVRAM_ADD_0FILL("nvram")

	// on board devices
	MCFG_ISA16_SLOT_ADD("mb:isabus","board1", pc_isa16_cards, "ide", true)
	MCFG_SLOT_OPTION_MACHINE_CONFIG("ide", cdrom)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa1", pc_isa16_cards, "svga_dm", true) // original is a gd-5440

	MCFG_DEVICE_ADD("ns16550", NS16550, XTAL(1'843'200))
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE("microtouch", microtouch_device, rx))
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE("mb:pic8259_master", pic8259_device, ir4_w))
	MCFG_MICROTOUCH_ADD("microtouch", 9600, DEVWRITELINE("ns16550", ins8250_uart_device, rx_w))

	MCFG_SOUND_ADD("cs4231", AD1848, 0)
	MCFG_AD1848_IRQ_CALLBACK(DEVWRITELINE("mb:pic8259_master", pic8259_device, ir5_w))
	MCFG_AD1848_DRQ_CALLBACK(DEVWRITELINE("mb:dma8237_1", am9517a_device, dreq1_w))

	MCFG_DEVICE_MODIFY("mb:dma8237_1")
	MCFG_I8237_OUT_IOW_1_CB(DEVWRITE8("^cs4231", ad1848_device, dack_w))

	// remove the keyboard controller and use the HLE one which allow keys to be unmapped
	MCFG_DEVICE_REMOVE("mb:keybc");
	MCFG_DEVICE_REMOVE("mb:pc_kbdc");
	MCFG_DEVICE_ADD("kbdc", KBDC8042, 0)
	MCFG_KBDC8042_KEYBOARD_TYPE(KBDC8042_AT386)
	MCFG_KBDC8042_SYSTEM_RESET_CB(INPUTLINE("maincpu", INPUT_LINE_RESET))
	MCFG_KBDC8042_GATE_A20_CB(INPUTLINE("maincpu", INPUT_LINE_A20))
	MCFG_KBDC8042_INPUT_BUFFER_FULL_CB(DEVWRITELINE("mb:pic8259_master", pic8259_device, ir1_w))
	MCFG_DEVICE_REMOVE("mb:rtc")
	MCFG_DS12885_ADD("mb:rtc")
	MCFG_MC146818_IRQ_HANDLER(DEVWRITELINE("pic8259_slave", pic8259_device, ir0_w))
	MCFG_MC146818_CENTURY_INDEX(0x32)
#endif
	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32M")    // Early XL games had 8 MB RAM, 6000 and later require 32MB

	/* bankdev for dxxxx */
	MCFG_DEVICE_ADD("dbank", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(dbank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATA_WIDTH(32)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x10000)

	/* Flash ROM */
	MCFG_AMD_29F040_ADD("flash")

	/* Security key */
	MCFG_DS1205_ADD("multikey")

#ifdef REAL_PCI_CHIPSET
	/* PCI root */
	MCFG_PCI_ROOT_ADD( ":pci")
	MCFG_SIS85C496_ADD(":pci:05.0", ":maincpu", 32*1024*1024)
#endif
MACHINE_CONFIG_END

#ifdef REAL_PCI_CHIPSET
#define MOTHERBOARD_ROMS \
	ROM_REGION(0x20000, ":pci:05.0", 0) \
	ROM_LOAD( "094572516 bios - 486.bin", 0x000000, 0x020000, CRC(1c0b3ba0) SHA1(ff86dd6e476405e716ac7a4de4a216d2d2b49f15))
#else
#define MOTHERBOARD_ROMS \
	ROM_REGION(0x20000, "bios", 0) \
	ROM_LOAD("prom.mb", 0x10000, 0x10000, BAD_DUMP CRC(e44bfd3c) SHA1(c07ec94e11efa30e001f39560010112f73cc0016) ) \
	ROM_REGION(0x80, "mb:rtc", 0) \
	ROM_LOAD("mb_rtc", 0, 0x80, BAD_DUMP CRC(b724e5d3) SHA1(45a19ec4201d2933d033689b7a01a0260962fb0b))
#endif

ROM_START( mtouchxl )
	MOTHERBOARD_ROMS

	ROM_REGION(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-03_u12-r3", 0x000000, 0x100000, CRC(5a14b68a) SHA1(351a3ae14c335ac0b52e6f4976f9819c11a668f9) )

	ROM_REGION(192, "multikey", ROMREGION_ERASE00)
	ROM_LOAD( "multikey",     0x000000, 0x0000c0, BAD_DUMP CRC(2bdaf557) SHA1(be7f5cab5b6565f7bf8066282cfe3b42c7d7b7fd) )

	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r1", 0, SHA1(874545bfc48eacba4c4887d1c45a40ebc7da456a))
ROM_END

ROM_START( mtchxl5k )
	MOTHERBOARD_ROMS

	ROM_REGION(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-03_u12-r3", 0x000000, 0x100000, CRC(5a14b68a) SHA1(351a3ae14c335ac0b52e6f4976f9819c11a668f9) )

	ROM_REGION(192, "multikey", ROMREGION_ERASE00)
	ROM_LOAD( "multikey",     0x000000, 0x0000c0, BAD_DUMP CRC(32cd3bab) SHA1(b31f05c3819c74a29a46bbcf4de3722bae874df2) )

	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r5i", 0, SHA1(e776a842b557f402e179862397b2ded5cf926702))
ROM_END

ROM_START( mtchxl5ko )
	MOTHERBOARD_ROMS

	ROM_REGION(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-03_u12-r3", 0x000000, 0x100000, CRC(5a14b68a) SHA1(351a3ae14c335ac0b52e6f4976f9819c11a668f9) )

	ROM_REGION(192, "multikey", ROMREGION_ERASE00)
	ROM_LOAD( "multikey",     0x000000, 0x0000c0, BAD_DUMP CRC(32cd3bab) SHA1(b31f05c3819c74a29a46bbcf4de3722bae874df2) )

	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r5b", 0, SHA1(37c2562053f0f4ed18c72a8ea04be371a6ac8413))
ROM_END

ROM_START( mtchxl5ko2 )
	MOTHERBOARD_ROMS

	ROM_REGION(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-03_u12-r3", 0x000000, 0x100000, CRC(5a14b68a) SHA1(351a3ae14c335ac0b52e6f4976f9819c11a668f9) )

	ROM_REGION(192, "multikey", ROMREGION_ERASE00)
	ROM_LOAD( "multikey",     0x000000, 0x0000c0, BAD_DUMP CRC(32cd3bab) SHA1(b31f05c3819c74a29a46bbcf4de3722bae874df2) )

	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r5e", 0, SHA1(a07dc6da346bee999f822a3517ea1d65a68dd4a2))
ROM_END

ROM_START( mtchxl6k )
	MOTHERBOARD_ROMS

	ROM_REGION(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-04_u12-r00.u12", 0x000000, 0x100000, CRC(2a6fbca4) SHA1(186eb052cb9b77ffe6ee4cb50c1b580532fd8f47) )

	ROM_REGION(192, "multikey", 0)
	ROM_LOAD( "multikey", 0, 192, BAD_DUMP CRC(d54ed86c) SHA1(83557dc604b2c7e8ab0787a3c3d73e1fb2556515) ) // hand made

	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r07", 0, SHA1(95599e181d9249db09464420522180d753857f3b))
ROM_END

ROM_START( mtchxl6ko4 )
	MOTHERBOARD_ROMS

	ROM_REGION(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-04_u12-r00.u12", 0x000000, 0x100000, CRC(2a6fbca4) SHA1(186eb052cb9b77ffe6ee4cb50c1b580532fd8f47) )

	ROM_REGION(192, "multikey", 0)
	ROM_LOAD( "multikey", 0, 192, BAD_DUMP CRC(d54ed86c) SHA1(83557dc604b2c7e8ab0787a3c3d73e1fb2556515) ) // hand made

	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r04", 0, SHA1(c4a40bb84de4a54fd4ee6f5d2179a1cb9fac2b09))
ROM_END

ROM_START( mtchxl6ko )
	MOTHERBOARD_ROMS

	ROM_REGION(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-04_u12-r00.u12", 0x000000, 0x100000, CRC(2a6fbca4) SHA1(186eb052cb9b77ffe6ee4cb50c1b580532fd8f47) )

	ROM_REGION(192, "multikey", 0)
	ROM_LOAD( "multikey", 0, 192, BAD_DUMP CRC(d54ed86c) SHA1(83557dc604b2c7e8ab0787a3c3d73e1fb2556515) ) // hand made

	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r02", 0, SHA1(eaaf26d2b700f16138090de7f372b40b93e8dba9))
ROM_END

ROM_START( mtchxlgld )
	MOTHERBOARD_ROMS

	ROM_REGION(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-04_u12-r00.u12", 0x000000, 0x100000, CRC(2a6fbca4) SHA1(186eb052cb9b77ffe6ee4cb50c1b580532fd8f47) )

	ROM_REGION(0x8000, "nvram", 0)
	ROM_LOAD( "u12-nvram-ds1235", 0x000000, 0x008000, CRC(b3b5379d) SHA1(91b3d8b7eb2df127ba35700317aa1aac14e49bb9) )

	ROM_REGION(192, "multikey", ROMREGION_ERASE00)
	ROM_LOAD( "multikey",     0x000000, 0x0000c0, BAD_DUMP CRC(b7c85d00) SHA1(c91dcafd8138d504acdc6ce9621f6cc3119cdb67) )

	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r01", 0, SHA1(9946bb14d3f77eadbbc606ca9c79f233e402189b))
ROM_END

ROM_START( mtchxlgldo )
	MOTHERBOARD_ROMS

	ROM_REGION(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-04_u12-r00.u12", 0x000000, 0x100000, CRC(2a6fbca4) SHA1(186eb052cb9b77ffe6ee4cb50c1b580532fd8f47) )

	ROM_REGION(0x8000, "nvram", 0)
	ROM_LOAD( "u12-nvram-ds1235", 0x000000, 0x008000, CRC(b3b5379d) SHA1(91b3d8b7eb2df127ba35700317aa1aac14e49bb9) )

	ROM_REGION(192, "multikey", ROMREGION_ERASE00)
	ROM_LOAD( "multikey",     0x000000, 0x0000c0, BAD_DUMP CRC(b7c85d00) SHA1(c91dcafd8138d504acdc6ce9621f6cc3119cdb67) )

	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r00", 0, SHA1(635e267f1abea060ce813eb7e78b88d57ea3f951))
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*     YEAR  NAME      PARENT   COMPAT   MACHINE      INPUT           DEVICE              INIT    COMPANY              FULLNAME */
// Any indicates this is from a CD-R at a trade show that was claimed to be a prototype, but R1 is several versions in?
COMP ( 1997, mtouchxl,   0,         0,      at486,   mtouchxl,     mtxl_state,  0,    "Merit Industries",  "MegaTouch XL (Version R1, prototype?)", 0 )
COMP ( 1998, mtchxl5k,   0,         0,      at486,   mtouchxl,     mtxl_state,  0,    "Merit Industries",  "MegaTouch XL Super 5000 (Version R5I)", MACHINE_NOT_WORKING )
COMP ( 1998, mtchxl5ko,  mtchxl5k,  0,      at486,   mtouchxl,     mtxl_state,  0,    "Merit Industries",  "MegaTouch XL Super 5000 (Version R5B)", MACHINE_NOT_WORKING )
COMP ( 1998, mtchxl5ko2, mtchxl5k,  0,      at486,   mtouchxl,     mtxl_state,  0,    "Merit Industries",  "MegaTouch XL Super 5000 (Version R5E)", MACHINE_NOT_WORKING )
COMP ( 1999, mtchxl6k,   0,         0,      at486,   mtouchxl,     mtxl_state,  0,    "Merit Industries",  "MegaTouch XL 6000 (Version r07)",       0 )
COMP ( 1999, mtchxl6ko4, mtchxl6k,  0,      at486,   mtouchxl,     mtxl_state,  0,    "Merit Industries",  "MegaTouch XL 6000 (Version r04)",       0 )
COMP ( 1999, mtchxl6ko,  mtchxl6k,  0,      at486,   mtouchxl,     mtxl_state,  0,    "Merit Industries",  "MegaTouch XL 6000 (Version r02)",       0 )
COMP ( 2000, mtchxlgld,  0,         0,      at486,   mtouchxl,     mtxl_state,  0,    "Merit Industries",  "MegaTouch XL Gold (Version r01)",       MACHINE_NOT_WORKING )
COMP ( 2000, mtchxlgldo, mtchxlgld, 0,      at486,   mtouchxl,     mtxl_state,  0,    "Merit Industries",  "MegaTouch XL Gold (Version r00)",       MACHINE_NOT_WORKING )
