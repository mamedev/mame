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

#include "microtouchlayout.h"

#include "bus/ata/atapicdr.h"
#include "bus/ata/hdd.h"
#include "bus/isa/isa_cards.h"
#include "cpu/i386/i386.h"
#include "machine/8042kbdc.h"
#include "machine/at.h"
#include "machine/bankdev.h"
#include "machine/ds1205.h"
#include "machine/ds128x.h"
#include "machine/ins8250.h"
#include "machine/intelfsh.h"
#include "machine/microtch.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#ifdef REAL_PCI_CHIPSET
#include "machine/sis85c496.h"
#endif
#include "sound/ad1848.h"

#include "speaker.h"


namespace {

class mtxl_state : public driver_device
{
public:
	mtxl_state(const machine_config &mconfig, device_type type, const char *tag) :
			driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
#ifndef REAL_PCI_CHIPSET
			m_mb(*this, "mb"),
#endif
			m_ram(*this, RAM_TAG),
			m_iocard(*this, "dbank"),
			m_multikey(*this, "multikey")
		{ }

	void at486(machine_config &config);
	void at486hd(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
#ifndef REAL_PCI_CHIPSET
	required_device<at_mb_device> m_mb;
#endif
	required_device<ram_device> m_ram;
	required_device<address_map_bank_device> m_iocard;
	optional_device<ds1205_device> m_multikey;
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	uint8_t coin_r();
	void bank_w(uint8_t data);
	uint8_t key_r();
	void key_w(uint8_t data);
	static void cdrom(device_t *device);
	static void hdd(device_t *device);
	void at32_io(address_map &map) ATTR_COLD;
	void at32_map(address_map &map) ATTR_COLD;
	void dbank_map(address_map &map) ATTR_COLD;
};

void mtxl_state::bank_w(uint8_t data)
{
	m_iocard->set_bank(data & 0x1f);
}

uint8_t mtxl_state::key_r()
{
	return m_multikey->read_dq() ? 0xff : 0xdf;
}

uint8_t mtxl_state::coin_r()
{
	return ioport("Coin")->read();
}

void mtxl_state::key_w(uint8_t data)
{
	m_multikey->write_rst((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
	m_multikey->write_clk((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);
	m_multikey->write_dq((data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
}

void mtxl_state::at32_map(address_map &map)
{
	map.unmap_value_high();
#ifndef REAL_PCI_CHIPSET
	map(0x00000000, 0x0009ffff).bankrw("bank10");
	map(0x000c8000, 0x000cffff).ram().share("nvram");
	map(0x000d0000, 0x000dffff).m(m_iocard, FUNC(address_map_bank_device::amap32));
	map(0x000e0000, 0x000fffff).rom().region("bios", 0);
	map(0xfffe0000, 0xffffffff).rom().region("bios", 0);
#endif
}

void mtxl_state::at32_io(address_map &map)
{
	map.unmap_value_high();
#ifndef REAL_PCI_CHIPSET
	map(0x0000, 0x001f).rw("mb:dma8237_1", FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x003f).rw("mb:pic8259_master", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x005f).rw("mb:pit8254", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0060, 0x0067).rw("kbdc", FUNC(kbdc8042_device::data_r), FUNC(kbdc8042_device::data_w));
	map(0x0061, 0x0061).rw("mb", FUNC(at_mb_device::portb_r), FUNC(at_mb_device::portb_w));
	map(0x0070, 0x007f).w("mb:rtc", FUNC(mc146818_device::address_w)).umask32(0x00ff00ff);
	map(0x0070, 0x007f).rw("mb:rtc", FUNC(mc146818_device::data_r), FUNC(mc146818_device::data_w)).umask32(0xff00ff00);
	map(0x0080, 0x009f).rw("mb", FUNC(at_mb_device::page8_r), FUNC(at_mb_device::page8_w));
	map(0x00a0, 0x00bf).rw("mb:pic8259_slave", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x00c0, 0x00df).rw("mb:dma8237_2", FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask32(0x00ff00ff);
	map(0x0224, 0x0227).rw("cs4231", FUNC(ad1848_device::read), FUNC(ad1848_device::write));
#endif
	map(0x0228, 0x022b).portr("Unknown");
	map(0x022f, 0x022f).w(FUNC(mtxl_state::bank_w));
	map(0x022d, 0x022d).rw(FUNC(mtxl_state::key_r), FUNC(mtxl_state::key_w));
	map(0x022c, 0x022c).r(FUNC(mtxl_state::coin_r));
#ifndef REAL_PCI_CHIPSET
	map(0x03f8, 0x03ff).rw("ns16550", FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
#endif
}

void mtxl_state::dbank_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("ioboard", 0);
	map(0x100000, 0x17ffff).rw("flash", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
}

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

	/* managed RAM */
	membank("bank10")->set_base(m_ram->pointer());

	if (m_ram->size() > 0xa0000)
	{
		offs_t ram_limit = 0x100000 + m_ram->size() - 0xa0000;
		space.install_ram(0x100000,  ram_limit - 1, m_ram->pointer() + 0xa0000);
	}
#endif
}

void mtxl_state::machine_reset()
{
	m_iocard->set_bank(0);
}

#ifndef REAL_PCI_CHIPSET
static void mt6k_ata_devices(device_slot_interface &device)
{
	device.option_add("cdrom", ATAPI_FIXED_CDROM);
	device.option_add("hdd", IDE_HARDDISK);
}

void mtxl_state::cdrom(device_t *device)
{
	auto ide0 = dynamic_cast<device_slot_interface *>(device->subdevice("ide:0"));
	ide0->option_reset();
	mt6k_ata_devices(*ide0);
	ide0->set_default_option("cdrom");
	ide0->set_fixed(true);

	auto ide1 = dynamic_cast<device_slot_interface *>(device->subdevice("ide:1"));
	ide1->set_default_option("hdd");
	ide1->set_fixed(true);
}

void mtxl_state::hdd(device_t *device)
{
	auto ide0 = dynamic_cast<device_slot_interface *>(device->subdevice("ide:0"));
	ide0->option_reset();
	mt6k_ata_devices(*ide0);
	ide0->set_default_option("hdd");
	ide0->set_fixed(true);

	auto ide1 = dynamic_cast<device_slot_interface *>(device->subdevice("ide:1"));
	ide1->set_default_option("cdrom");
	ide1->set_fixed(true);
}
#endif

void mtxl_state::at486(machine_config &config)
{
	I486DX4(config, m_maincpu, 33000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mtxl_state::at32_map);
	m_maincpu->set_addrmap(AS_IO, &mtxl_state::at32_io);
#ifndef REAL_PCI_CHIPSET
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	AT_MB(config, "mb");
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// on board devices
	ISA16_SLOT(config, "board1", 0, "mb:isabus", pc_isa16_cards, "ide", true).set_option_machine_config("ide", cdrom); // FIXME: determine ISA bus clock
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "svga_dm", true); // original is a gd-5440

	ns16550_device &uart(NS16550(config, "ns16550", XTAL(1'843'200)));
	uart.out_tx_callback().set("microtouch", FUNC(microtouch_device::rx));
	uart.out_int_callback().set("mb:pic8259_master", FUNC(pic8259_device::ir4_w));

	MICROTOUCH(config, "microtouch", 9600).stx().set(uart, FUNC(ins8250_uart_device::rx_w));

	ad1848_device &cs4231(AD1848(config, "cs4231", 0));
	cs4231.irq().set("mb:pic8259_master", FUNC(pic8259_device::ir5_w));
	cs4231.drq().set("mb:dma8237_1", FUNC(am9517a_device::dreq1_w));

	subdevice<am9517a_device>("mb:dma8237_1")->out_iow_callback<1>().set("cs4231", FUNC(ad1848_device::dack_w));

	// remove the keyboard controller and use the HLE one which allow keys to be unmapped
	config.device_remove("mb:keybc");
	kbdc8042_device &kbdc(KBDC8042(config, "kbdc"));
	kbdc.set_keyboard_type(kbdc8042_device::KBDC8042_STANDARD);
	kbdc.system_reset_callback().set_inputline(m_maincpu, INPUT_LINE_RESET);
	kbdc.gate_a20_callback().set_inputline(m_maincpu, INPUT_LINE_A20);
	kbdc.input_buffer_full_callback().set("mb:pic8259_master", FUNC(pic8259_device::ir1_w));

	ds12885_device &rtc(DS12885(config.replace(), "mb:rtc"));
	rtc.irq().set("mb:pic8259_slave", FUNC(pic8259_device::ir0_w));
	rtc.set_century_index(0x32);
#endif
	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("32M"); // Early XL games had 8 MB RAM, 6000 and later require 32MB

	/* bankdev for dxxxx */
	ADDRESS_MAP_BANK(config, "dbank").set_map(&mtxl_state::dbank_map).set_options(ENDIANNESS_LITTLE, 32, 32, 0x10000);

	/* Flash ROM */
	AMD_29F040(config, "flash");

	/* Security key */
	DS1205(config, "multikey");

#ifdef REAL_PCI_CHIPSET
	/* PCI root */
	PCI_ROOT(config, ":pci");
	// FIXME: This MCFG fragment does not compile. -R
	//MCFG_SIS85C496_ADD(":pci:05.0", ":maincpu", 32*1024*1024)
#endif

	config.set_default_layout(layout_microtouch);
}

void mtxl_state::at486hd(machine_config &config)
{
	I486DX4(config, m_maincpu, 33000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mtxl_state::at32_map);
	m_maincpu->set_addrmap(AS_IO, &mtxl_state::at32_io);
#ifndef REAL_PCI_CHIPSET
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	AT_MB(config, "mb");
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// on board devices
	ISA16_SLOT(config, "board1", 0, "mb:isabus", pc_isa16_cards, "ide", true).set_option_machine_config("ide", hdd); // FIXME: determine ISA bus clock
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "svga_dm", true); // original is a gd-5440

	ns16550_device &uart(NS16550(config, "ns16550", XTAL(1'843'200)));
	uart.out_tx_callback().set("microtouch", FUNC(microtouch_device::rx));
	uart.out_int_callback().set("mb:pic8259_master", FUNC(pic8259_device::ir4_w));

	MICROTOUCH(config, "microtouch", 9600).stx().set(uart, FUNC(ins8250_uart_device::rx_w));

	ad1848_device &cs4231(AD1848(config, "cs4231", 0));
	cs4231.irq().set("mb:pic8259_master", FUNC(pic8259_device::ir5_w));
	cs4231.drq().set("mb:dma8237_1", FUNC(am9517a_device::dreq1_w));

	subdevice<am9517a_device>("mb:dma8237_1")->out_iow_callback<1>().set("cs4231", FUNC(ad1848_device::dack_w));

	// remove the keyboard controller and use the HLE one which allow keys to be unmapped
	config.device_remove("mb:keybc");
	kbdc8042_device &kbdc(KBDC8042(config, "kbdc"));
	kbdc.set_keyboard_type(kbdc8042_device::KBDC8042_STANDARD);
	kbdc.system_reset_callback().set_inputline(m_maincpu, INPUT_LINE_RESET);
	kbdc.gate_a20_callback().set_inputline(m_maincpu, INPUT_LINE_A20);
	kbdc.input_buffer_full_callback().set("mb:pic8259_master", FUNC(pic8259_device::ir1_w));

	ds12885_device &rtc(DS12885(config.replace(), "mb:rtc"));
	rtc.irq().set("mb:pic8259_slave", FUNC(pic8259_device::ir0_w));
	rtc.set_century_index(0x32);
#endif
	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("32M"); // Early XL games had 8 MB RAM, 6000 and later require 32MB

	/* bankdev for dxxxx */
	ADDRESS_MAP_BANK(config, "dbank").set_map(&mtxl_state::dbank_map).set_options(ENDIANNESS_LITTLE, 32, 32, 0x10000);

	/* Flash ROM */
	AMD_29F040(config, "flash");

	/* Security key */
	DS1205(config, "multikey");

#ifdef REAL_PCI_CHIPSET
	/* PCI root */
	PCI_ROOT(config, ":pci");
	// FIXME: This MCFG fragment does not compile. -R
	//MCFG_SIS85C496_ADD(":pci:05.0", ":maincpu", 32*1024*1024)
#endif

	config.set_default_layout(layout_microtouch);
}

#ifdef REAL_PCI_CHIPSET
#define MOTHERBOARD_ROMS \
	ROM_REGION32_LE(0x20000, ":pci:05.0", 0) \
	ROM_LOAD( "094572516 bios - 486.bin", 0x000000, 0x020000, CRC(1c0b3ba0) SHA1(ff86dd6e476405e716ac7a4de4a216d2d2b49f15))
#else
#define MOTHERBOARD_ROMS \
	ROM_REGION32_LE(0x20000, "bios", 0) \
	ROM_LOAD("prom.mb", 0x10000, 0x10000, BAD_DUMP CRC(e44bfd3c) SHA1(c07ec94e11efa30e001f39560010112f73cc0016) ) \
	ROM_REGION(0x80, "mb:rtc", 0) \
	ROM_LOAD("mb_rtc", 0, 0x80, BAD_DUMP CRC(ac77f726) SHA1(e6ae2010d8cebb82d0414c70e41ae9dbcbc460e4))
#endif

ROM_START( mtouchxl )
	MOTHERBOARD_ROMS

	ROM_REGION32_LE(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-03_u12-r3", 0x000000, 0x100000, CRC(5a14b68a) SHA1(351a3ae14c335ac0b52e6f4976f9819c11a668f9) )

	ROM_REGION(192, "multikey", ROMREGION_ERASE00)
	ROM_LOAD( "multikey",     0x000000, 0x0000c0, BAD_DUMP CRC(2bdaf557) SHA1(be7f5cab5b6565f7bf8066282cfe3b42c7d7b7fd) )

	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r1", 0, SHA1(874545bfc48eacba4c4887d1c45a40ebc7da456a))
ROM_END

ROM_START( mtchxl5k )
	MOTHERBOARD_ROMS

	ROM_REGION32_LE(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-03_u12-r3", 0x000000, 0x100000, CRC(5a14b68a) SHA1(351a3ae14c335ac0b52e6f4976f9819c11a668f9) )

	ROM_REGION(192, "multikey", ROMREGION_ERASE00)
	ROM_LOAD( "multikey",     0x000000, 0x0000c0, BAD_DUMP CRC(32cd3bab) SHA1(b31f05c3819c74a29a46bbcf4de3722bae874df2) )

	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r5i", 0, SHA1(e776a842b557f402e179862397b2ded5cf926702))
ROM_END

ROM_START( mtchxl5ko )
	MOTHERBOARD_ROMS

	ROM_REGION32_LE(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-03_u12-r3", 0x000000, 0x100000, CRC(5a14b68a) SHA1(351a3ae14c335ac0b52e6f4976f9819c11a668f9) )

	ROM_REGION(192, "multikey", ROMREGION_ERASE00)
	ROM_LOAD( "multikey",     0x000000, 0x0000c0, BAD_DUMP CRC(32cd3bab) SHA1(b31f05c3819c74a29a46bbcf4de3722bae874df2) )

	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r5b", 0, SHA1(37c2562053f0f4ed18c72a8ea04be371a6ac8413))
ROM_END

ROM_START( mtchxl5ko2 )
	MOTHERBOARD_ROMS

	ROM_REGION32_LE(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-03_u12-r3", 0x000000, 0x100000, CRC(5a14b68a) SHA1(351a3ae14c335ac0b52e6f4976f9819c11a668f9) )

	ROM_REGION(192, "multikey", ROMREGION_ERASE00)
	ROM_LOAD( "multikey",     0x000000, 0x0000c0, BAD_DUMP CRC(32cd3bab) SHA1(b31f05c3819c74a29a46bbcf4de3722bae874df2) )

	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r5e", 0, SHA1(a07dc6da346bee999f822a3517ea1d65a68dd4a2))
ROM_END

ROM_START( mtchxl6k )
	MOTHERBOARD_ROMS

	ROM_REGION32_LE(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-04_u12-r00.u12", 0x000000, 0x100000, CRC(2a6fbca4) SHA1(186eb052cb9b77ffe6ee4cb50c1b580532fd8f47) )

	ROM_REGION(192, "multikey", 0)
	ROM_LOAD( "multikey", 0, 192, BAD_DUMP CRC(d54ed86c) SHA1(83557dc604b2c7e8ab0787a3c3d73e1fb2556515) ) // hand made

	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r07", 0, SHA1(95599e181d9249db09464420522180d753857f3b))
ROM_END

ROM_START( mtchxl6ko4 )
	MOTHERBOARD_ROMS

	ROM_REGION32_LE(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-04_u12-r00.u12", 0x000000, 0x100000, CRC(2a6fbca4) SHA1(186eb052cb9b77ffe6ee4cb50c1b580532fd8f47) )

	ROM_REGION(192, "multikey", 0)
	ROM_LOAD( "multikey", 0, 192, BAD_DUMP CRC(d54ed86c) SHA1(83557dc604b2c7e8ab0787a3c3d73e1fb2556515) ) // hand made

	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r04", 0, SHA1(c4a40bb84de4a54fd4ee6f5d2179a1cb9fac2b09))
ROM_END

ROM_START( mtchxl6ko )
	MOTHERBOARD_ROMS

	ROM_REGION32_LE(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-04_u12-r00.u12", 0x000000, 0x100000, CRC(2a6fbca4) SHA1(186eb052cb9b77ffe6ee4cb50c1b580532fd8f47) )

	ROM_REGION(192, "multikey", 0)
	ROM_LOAD( "multikey", 0, 192, BAD_DUMP CRC(d54ed86c) SHA1(83557dc604b2c7e8ab0787a3c3d73e1fb2556515) ) // hand made

	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r02", 0, SHA1(eaaf26d2b700f16138090de7f372b40b93e8dba9))
ROM_END

ROM_START( mtchxlgld )
	MOTHERBOARD_ROMS

	ROM_REGION32_LE(0x100000, "ioboard", 0)
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

	ROM_REGION32_LE(0x100000, "ioboard", 0)
	ROM_LOAD( "sa3014-04_u12-r00.u12", 0x000000, 0x100000, CRC(2a6fbca4) SHA1(186eb052cb9b77ffe6ee4cb50c1b580532fd8f47) )

	ROM_REGION(0x8000, "nvram", 0)
	ROM_LOAD( "u12-nvram-ds1235", 0x000000, 0x008000, CRC(b3b5379d) SHA1(91b3d8b7eb2df127ba35700317aa1aac14e49bb9) )

	ROM_REGION(192, "multikey", ROMREGION_ERASE00)
	ROM_LOAD( "multikey",     0x000000, 0x0000c0, BAD_DUMP CRC(b7c85d00) SHA1(c91dcafd8138d504acdc6ce9621f6cc3119cdb67) )

	DISK_REGION("board1:ide:ide:0:cdrom")
	DISK_IMAGE_READONLY("r00", 0, SHA1(635e267f1abea060ce813eb7e78b88d57ea3f951))
ROM_END

ROM_START( mtchxlti )
	MOTHERBOARD_ROMS

	ROM_REGION32_LE(0x100000, "ioboard", ROMREGION_ERASE00)

	ROM_REGION(0x8000, "nvram", ROMREGION_ERASE00)

	ROM_REGION(192, "multikey", ROMREGION_ERASE00)

	DISK_REGION("board1:ide:ide:0:hdd")
	DISK_IMAGE_READONLY("r00", 0, SHA1(8e9a2f9e670f02139cee11b7e8f758639d8b2838))
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME        PARENT     COMPAT  MACHINE  INPUT     CLASS       INIT        COMPANY             FULLNAME */
// Any indicates this is from a CD-R at a trade show that was claimed to be a prototype, but R1 is several versions in?
COMP( 1997, mtouchxl,   0,         0,      at486,   mtouchxl, mtxl_state, empty_init, "Merit Industries", "MegaTouch XL (Version R1, prototype?)", 0 )
COMP( 1998, mtchxl5k,   0,         0,      at486,   mtouchxl, mtxl_state, empty_init, "Merit Industries", "MegaTouch XL Super 5000 (Version R5I)", MACHINE_NOT_WORKING )
COMP( 1998, mtchxl5ko,  mtchxl5k,  0,      at486,   mtouchxl, mtxl_state, empty_init, "Merit Industries", "MegaTouch XL Super 5000 (Version R5B)", MACHINE_NOT_WORKING )
COMP( 1998, mtchxl5ko2, mtchxl5k,  0,      at486,   mtouchxl, mtxl_state, empty_init, "Merit Industries", "MegaTouch XL Super 5000 (Version R5E)", MACHINE_NOT_WORKING )
COMP( 1999, mtchxl6k,   0,         0,      at486,   mtouchxl, mtxl_state, empty_init, "Merit Industries", "MegaTouch XL 6000 (Version r07)",       0 )
COMP( 1999, mtchxl6ko4, mtchxl6k,  0,      at486,   mtouchxl, mtxl_state, empty_init, "Merit Industries", "MegaTouch XL 6000 (Version r04)",       0 )
COMP( 1999, mtchxl6ko,  mtchxl6k,  0,      at486,   mtouchxl, mtxl_state, empty_init, "Merit Industries", "MegaTouch XL 6000 (Version r02)",       0 )
COMP( 2000, mtchxlgld,  0,         0,      at486,   mtouchxl, mtxl_state, empty_init, "Merit Industries", "MegaTouch XL Gold (Version r01)",       MACHINE_NOT_WORKING )
COMP( 2000, mtchxlgldo, mtchxlgld, 0,      at486,   mtouchxl, mtxl_state, empty_init, "Merit Industries", "MegaTouch XL Gold (Version r00)",       MACHINE_NOT_WORKING )
// this is a cracked operator bootleg, but the original files exist on the disk and could be replaced to create an imperfect non-cracked dump
COMP( 2002, mtchxlti,   0,         0,      at486hd, mtouchxl, mtxl_state, empty_init, "bootleg", "MegaTouch XL Titanium (Version r0?, cracked)",   MACHINE_NOT_WORKING )
