// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
/***************************************************************************

    drivers/genpc.c

    Driver file for generic PC machines

***************************************************************************/


#include "emu.h"

#include "includes/genpc.h"

#include "cpu/nec/nec.h"
#include "cpu/i86/i86.h"
#include "machine/pc_lpt.h"
#include "bus/pc_kbd/keyboards.h"

class genpc_state : public driver_device
{
public:
	genpc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START( pc8_map, AS_PROGRAM, 8, genpc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc16_map, AS_PROGRAM, 16, genpc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(pc8_io, AS_IO, 8, genpc_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static ADDRESS_MAP_START(pc16_io, AS_IO, 16, genpc_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END


static INPUT_PORTS_START( pcgen )
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START(cga)
	DEVICE_INPUT_DEFAULTS("DSW0",0x30, 0x20)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(vga)
	DEVICE_INPUT_DEFAULTS("DSW0",0x30, 0x00)
DEVICE_INPUT_DEFAULTS_END


static MACHINE_CONFIG_START( pcmda, genpc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V20, 4772720)
	MCFG_CPU_PROGRAM_MAP(pc8_map)
	MCFG_CPU_IO_MAP(pc8_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb","maincpu")

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "mda", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "com", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "fdc_xt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "hdc", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa5", pc_isa8_cards, "adlib", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa6", pc_isa8_cards, NULL, false)

	/* keyboard */
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","ibm5150")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( pcherc, genpc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V20, 4772720)
	MCFG_CPU_PROGRAM_MAP(pc8_map)
	MCFG_CPU_IO_MAP(pc8_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb","maincpu")

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "hercules", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "com", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "fdc_xt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "hdc", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa5", pc_isa8_cards, "adlib", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa6", pc_isa8_cards, NULL, false)

	/* keyboard */
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","ibm5150")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( pccga, genpc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",  I8086, 4772720)
	MCFG_CPU_PROGRAM_MAP(pc16_map)
	MCFG_CPU_IO_MAP(pc16_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb","maincpu")
	MCFG_DEVICE_INPUT_DEFAULTS(cga)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "cga", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "com", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "fdc_xt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "hdc", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa5", pc_isa8_cards, "sblaster1_0", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa6", pc_isa8_cards, NULL, false)

	/* keyboard */
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","ibm5150")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( pcega, genpc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",  I8086, 4772720)
	MCFG_CPU_PROGRAM_MAP(pc16_map)
	MCFG_CPU_IO_MAP(pc16_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb","maincpu")
	MCFG_DEVICE_INPUT_DEFAULTS(vga)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "com", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "fdc_xt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "hdc", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "sblaster1_0", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa5", pc_isa8_cards, "ega", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa6", pc_isa8_cards, NULL, false)

	/* keyboard */
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","ibm5150")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( xtvga, genpc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",  I8086, 4772720)
	MCFG_CPU_PROGRAM_MAP(pc16_map)
	MCFG_CPU_IO_MAP(pc16_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb","maincpu")
	MCFG_DEVICE_INPUT_DEFAULTS(vga)

	MCFG_ISA8_SLOT_ADD("mb:isa","isa1", pc_isa8_cards, "com", false)
	MCFG_ISA8_SLOT_ADD("mb:isa","isa2", pc_isa8_cards, "fdc_xt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa","isa3", pc_isa8_cards, "hdc", false)
	MCFG_ISA8_SLOT_ADD("mb:isa","isa4", pc_isa8_cards, "sblaster1_0", false)
	MCFG_ISA8_SLOT_ADD("mb:isa","isa5", pc_isa8_cards, "svga_et4k", false)
	MCFG_ISA8_SLOT_ADD("mb:isa","isa6", pc_isa8_cards, NULL, false)

	/* keyboard */
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","ibm5150")
MACHINE_CONFIG_END


ROM_START( pcmda )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "anon", "XT Anonymous Generic Turbo BIOS")
	ROMX_LOAD("pcxt.rom",    0xfe000, 0x02000, CRC(031aafad) SHA1(a641b505bbac97b8775f91fe9b83d9afdf4d038f),ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "anon2007", "XT Anonymous Generic Turbo BIOS 2007")
	ROMX_LOAD( "pcxt2007.bin", 0xfe000, 0x2000, CRC(1d7bd86c) SHA1(33a500f599b4dad2fe6d7a5c3e89b13bd5dd2987),ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "ami", "XT AMI")
	ROMX_LOAD( "ami.bin", 0xfe000, 0x2000, CRC(b381eb22) SHA1(9735193de119270c946a17ed58c3ab9554e0852e),ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "award", "XT Award 2.05")
	ROMX_LOAD( "award2.05.bin", 0xfe000, 0x2000, CRC(5b3953e5) SHA1(4a36171aa8d993008187f39f732b9296401b7b6c),ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "dtk", "XT DTK Erso bios 2.42")
	ROMX_LOAD( "dtk2.42.bin", 0xfe000, 0x2000, CRC(3f2d2a76) SHA1(02fa057f2c22ab199a8d9795ab1ae570f2b13a36),ROM_BIOS(5))
	ROM_SYSTEM_BIOS(5, "peter", "XT Peter Kohlman 3.75") // V20 Rom only
	ROMX_LOAD( "peterv203.75.bin", 0xfe000, 0x2000, CRC(b053a6a4) SHA1(f53218ad3d725f12d9149b22d8afcf6a8869a3bd),ROM_BIOS(6))
	ROM_SYSTEM_BIOS(6, "pho227", "XT Phoenix Bios 2.27")
	ROMX_LOAD( "phoenix2.27.bin", 0xfe000, 0x2000, CRC(168ffef0) SHA1(69465db2f9246a614044d1f433d374506a13a07f),ROM_BIOS(7))
	ROM_SYSTEM_BIOS(7, "pho251", "XT Phoenix Bios 2.51")
	ROMX_LOAD( "phoenix2.51.bin", 0xfe000, 0x2000, CRC(9b7e9c40) SHA1(c948a8d3d715e469105c6e2acd8b46ec274b25a8),ROM_BIOS(8))
	ROM_SYSTEM_BIOS(8, "turbo", "XT Turbo Bios 3.10")
	ROMX_LOAD( "turbo3.10.bin", 0xfe000, 0x2000, CRC(8aaca1e3) SHA1(9c03da16713e08c0112a04c8bdfa394e7341c1fc),ROM_BIOS(9))
ROM_END

#define rom_pcherc    rom_pcmda

#define rom_pcega    rom_pcmda

ROM_START( pc )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "anon", "XT Anonymous Generic Turbo BIOS")
	ROMX_LOAD("pcxt.rom",    0xfe000, 0x02000, CRC(031aafad) SHA1(a641b505bbac97b8775f91fe9b83d9afdf4d038f),ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "anon2007", "XT Anonymous Generic Turbo BIOS 2007")
	ROMX_LOAD( "pcxt2007.bin", 0xfe000, 0x2000, CRC(1d7bd86c) SHA1(33a500f599b4dad2fe6d7a5c3e89b13bd5dd2987),ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "ami", "XT AMI")
	ROMX_LOAD( "ami.bin", 0xfe000, 0x2000, CRC(b381eb22) SHA1(9735193de119270c946a17ed58c3ab9554e0852e),ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "award", "XT Award 2.05")
	ROMX_LOAD( "award2.05.bin", 0xfe000, 0x2000, CRC(5b3953e5) SHA1(4a36171aa8d993008187f39f732b9296401b7b6c),ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "dtk", "XT DTK Erso bios 2.42")
	ROMX_LOAD( "dtk2.42.bin", 0xfe000, 0x2000, CRC(3f2d2a76) SHA1(02fa057f2c22ab199a8d9795ab1ae570f2b13a36),ROM_BIOS(5))
	ROM_SYSTEM_BIOS(5, "peter", "XT Peter Kohlman 3.75")
	ROMX_LOAD( "peterv203.75.bin", 0xfe000, 0x2000, CRC(b053a6a4) SHA1(f53218ad3d725f12d9149b22d8afcf6a8869a3bd),ROM_BIOS(6))
	ROM_SYSTEM_BIOS(6, "pho227", "XT Phoenix Bios 2.27")
	ROMX_LOAD( "phoenix2.27.bin", 0xfe000, 0x2000, CRC(168ffef0) SHA1(69465db2f9246a614044d1f433d374506a13a07f),ROM_BIOS(7))
	ROM_SYSTEM_BIOS(7, "pho251", "XT Phoenix Bios 2.51")
	ROMX_LOAD( "phoenix2.51.bin", 0xfe000, 0x2000, CRC(9b7e9c40) SHA1(c948a8d3d715e469105c6e2acd8b46ec274b25a8),ROM_BIOS(8))
	ROM_SYSTEM_BIOS(8, "turbo", "XT Turbo Bios 3.10")
	ROMX_LOAD( "turbo3.10.bin", 0xfe000, 0x2000, CRC(8aaca1e3) SHA1(9c03da16713e08c0112a04c8bdfa394e7341c1fc),ROM_BIOS(9))
	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

#define rom_xtvga    rom_pcmda

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*     YEAR     NAME        PARENT      COMPAT  MACHINE     INPUT       INIT        COMPANY     FULLNAME */
COMP(  1987,    pc,         ibm5150,    0,      pccga,      pcgen, driver_device,       0,          "<generic>",  "PC (CGA)" , 0)
COMP(  1987,    pcega,      ibm5150,    0,      pcega,      pcgen, driver_device,       0,          "<generic>",  "PC (EGA)" , 0)
COMP ( 1987,    pcmda,      ibm5150,    0,      pcmda,      pcgen, driver_device,       0,          "<generic>",  "PC (MDA)" , 0)
COMP ( 1987,    pcherc,     ibm5150,    0,      pcherc,     pcgen, driver_device,      0,       "<generic>",  "PC (Hercules)" , 0)
COMP ( 1987,    xtvga,      ibm5150,    0,      xtvga,      pcgen, driver_device,       0,          "<generic>",  "PC (VGA)" , MACHINE_NOT_WORKING)
