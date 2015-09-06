// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
/***************************************************************************

    drivers/pc.c

Driver file for IBM PC, IBM PC XT, and related machines.

    PC-XT memory map

    00000-9FFFF   RAM
    A0000-AFFFF   NOP       or videoram EGA/VGA
    B0000-B7FFF   videoram  MDA, page #0
    B8000-BFFFF   videoram  CGA and/or MDA page #1, T1T mapped RAM
    C0000-C7FFF   NOP       or ROM EGA/VGA
    C8000-C9FFF   ROM       XT HDC #1
    CA000-CBFFF   ROM       XT HDC #2
    D0000-EFFFF   NOP       or 'adapter RAM'
    F0000-FDFFF   NOP       or ROM Basic + other Extensions
    FE000-FFFFF   ROM

IBM5550
=======
Information can be found at http://homepage3.nifty.com/ibm5550/index-e.html
It's an heavily modified IBM PC-XT machine, with a completely different
video HW too.

***************************************************************************/

#include "emu.h"

#include "cpu/i86/i86.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "includes/genpc.h"

class pc_state : public driver_device
{
public:
	pc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;

	DECLARE_READ8_MEMBER(unk_r);

	DECLARE_DRIVER_INIT(bondwell);

	DECLARE_INPUT_CHANGED_MEMBER(pc_turbo_callback);

	double m_turbo_off_speed;
};

static ADDRESS_MAP_START( pc8_map, AS_PROGRAM, 8, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( zenith_map, AS_PROGRAM, 8, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xf7fff) AM_RAM
	AM_RANGE(0xf8000, 0xfffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( pc16_map, AS_PROGRAM, 16, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ibm5550_map, AS_PROGRAM, 16, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xeffff) AM_RAM
	AM_RANGE(0xfc000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(pc8_io, AS_IO, 8, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static ADDRESS_MAP_START(pc16_io, AS_IO, 16, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0070, 0x007f) AM_RAM // needed for Poisk-2
ADDRESS_MAP_END

READ8_MEMBER(pc_state::unk_r)
{
	return 0;
}

static ADDRESS_MAP_START(ibm5550_io, AS_IO, 16, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00a0, 0x00a1) AM_READ8(unk_r, 0x00ff )
ADDRESS_MAP_END

INPUT_CHANGED_MEMBER(pc_state::pc_turbo_callback)
{
	m_maincpu->set_clock_scale((newval & 2) ? 1 : m_turbo_off_speed);
}

DRIVER_INIT_MEMBER(pc_state,bondwell)
{
	m_turbo_off_speed = 4.77/12;
}

static INPUT_PORTS_START( pccga )
	PORT_START("DSW1") /* IN2 */
	PORT_DIPNAME( 0x80, 0x80, "COM1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "COM2: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "COM3: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "COM4: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "LPT1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "LPT2: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "LPT3: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x00, "Game port enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )

	PORT_START("DSW2") /* IN3 */
	PORT_DIPNAME( 0x08, 0x08, "HDC1 (C800:0 port 320-323)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "HDC2 (CA00:0 port 324-327)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_BIT( 0x02, 0x02,   IPT_UNUSED ) /* no turbo switch */
	PORT_BIT( 0x01, 0x01,   IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( bondwell )
	PORT_INCLUDE(pccga)

	PORT_MODIFY("DSW2") /* IN3 */
	PORT_DIPNAME( 0x02, 0x02, "Turbo Switch" ) PORT_CHANGED_MEMBER(DEVICE_SELF, pc_state, pc_turbo_callback, 0)
	PORT_DIPSETTING(    0x00, "Off (4.77 MHz)" )
	PORT_DIPSETTING(    0x02, "On (12 MHz)" )
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( pccga )
	DEVICE_INPUT_DEFAULTS("DSW0", 0x30, 0x20)
DEVICE_INPUT_DEFAULTS_END

#define MCFG_CPU_PC(mem, port, type, clock) \
	MCFG_CPU_ADD("maincpu", type, clock)                \
	MCFG_CPU_PROGRAM_MAP(mem##_map) \
	MCFG_CPU_IO_MAP(port##_io) \
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

static MACHINE_CONFIG_START( pccga, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(pc8, pc8, I8088, 4772720)   /* 4,77 MHz */

	MCFG_IBM5160_MOTHERBOARD_ADD("mb", "maincpu")
	MCFG_DEVICE_INPUT_DEFAULTS(pccga)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "cga", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "fdc_xt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "lpt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "com", false)

	/* keyboard */
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_IBM_PC_XT_83)
	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","ibm5150")
MACHINE_CONFIG_END

static DEVICE_INPUT_DEFAULTS_START( iskr3104 )
	DEVICE_INPUT_DEFAULTS("DSW0", 0x30, 0x00)
DEVICE_INPUT_DEFAULTS_END

static MACHINE_CONFIG_START( iskr3104, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(pc16, pc16, I8086, 4772720)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb", "maincpu")
	MCFG_DEVICE_INPUT_DEFAULTS(iskr3104)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "ega", false)
	MCFG_SLOT_OPTION_DEFAULT_BIOS("ega", "iskr3104")

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "fdc_xt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "lpt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "com", false)

	/* keyboard */
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_IBM_PC_XT_83)
	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( poisk2, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(pc16, pc16, I8086, 4772720)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb", "maincpu")
	MCFG_DEVICE_INPUT_DEFAULTS(pccga)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "cga_poisk2", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "fdc_xt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "lpt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "com", false)

	/* keyboard */
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_IBM_PC_XT_83)
	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( zenith, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(zenith, pc8, I8088, XTAL_14_31818MHz/3) /* 4,77 MHz */

	MCFG_IBM5150_MOTHERBOARD_ADD("mb", "maincpu")
	MCFG_DEVICE_INPUT_DEFAULTS(pccga)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "cga", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "fdc_xt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "lpt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "com", false)

	/* keyboard */
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_IBM_PC_XT_83)
	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","ibm5150")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ibm5550, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(ibm5550, ibm5550, I8086, 8000000)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb", "maincpu")
	MCFG_DEVICE_INPUT_DEFAULTS(pccga)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "cga", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "fdc_xt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "lpt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "com", false)

	/* keyboard */
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_IBM_PC_XT_83)
	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(bondwell, pccga)
	MCFG_DEVICE_REMOVE("maincpu")
	MCFG_CPU_PC(pc8, pc8, I8088, 4772720) // turbo?
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(mk88, poisk2)
	MCFG_DEVICE_MODIFY("isa1")
	MCFG_SLOT_DEFAULT_OPTION("cga_ec1841")
MACHINE_CONFIG_END

ROM_START( bw230 )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD("bondwell.bin", 0xfe000, 0x2000, CRC(d435a405) SHA1(a57c705d1144c7b61940b6f5c05d785c272fc9bb))
ROM_END

ROM_START( zdsupers )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "v31d", "v3.1d" )
	ROMX_LOAD( "z184m v3.1d.10d", 0xf8000, 0x8000, CRC(44012c3b) SHA1(f2f28979798874386ca8ba3dd3ead24ae7c2aeb4), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v29e", "v2.9e" )
	ROMX_LOAD( "z184m v2.9e.10d", 0xf8000, 0x8000, CRC(de2f200b) SHA1(ad5ce601669a82351e412fc6c1c70c47779a1e55), ROM_BIOS(2))
ROM_END


ROM_START( dgone )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "dgone.bin",  0xf8000, 0x08000, CRC(2c38c86e) SHA1(c0f85a000d1d13cd354965689e925d677822549e))
ROM_END

ROM_START( ssam88s )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "samsung_samtron_88s_vers_2.0a.bin",  0xf8000, 0x08000, CRC(d1252a91) SHA1(469d15b6ecd7b70234975dc12c6bda4212a66652))
ROM_END

ROM_START( mk88 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_DEFAULT_BIOS("v392")
	ROM_SYSTEM_BIOS(0, "v290", "v2.90")
	ROMX_LOAD( "mk88m.bin", 0xfc000, 0x2000, CRC(09c9da3b) SHA1(d1e7ad23b5f5b3576ad128c1198294129754f39f), ROM_BIOS(1))
	ROMX_LOAD( "mk88b.bin", 0xfe000, 0x2000, CRC(8a922476) SHA1(c19c3644ab92fd12e13f32b410cd26e3c844a03b), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v391", "v3.91")
	ROMX_LOAD( "mkm.bin", 0xfc000, 0x2000, CRC(65f979e8) SHA1(13e85be9bc8ceb5ab9e559e7d0089e26fbbb84fc), ROM_BIOS(2))
	ROMX_LOAD( "mkb.bin", 0xfe000, 0x2000, CRC(830a0447) SHA1(11bc200fdbcfbbe335f4c282020750c0b5ca4167), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v392", "v3.92")
	ROMX_LOAD( "m88.bin", 0xfc000, 0x2000, CRC(fe1b4e36) SHA1(fcb420af0ff09a7d43fcb9b7d0b0233a2071c159), ROM_BIOS(3))
	ROMX_LOAD( "b88.bin", 0xfe000, 0x2000, CRC(58a418df) SHA1(216398d4e4302ee7efcc2c8f9ff9d8a1161229ea), ROM_BIOS(3))
ROM_END

ROM_START( iskr3104 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROMX_LOAD( "198.bin", 0xfc000, 0x2000, CRC(bcfd8e41) SHA1(e21ddf78839aa51fa5feb23f511ff5e2da31b433),ROM_SKIP(1))
	ROMX_LOAD( "199.bin", 0xfc001, 0x2000, CRC(2da5fe79) SHA1(14d5dccc141a0b3367f7f8a7188306fdf03c2b6c),ROM_SKIP(1))
	// EGA card from Iskra-3104
	//ROMX_LOAD( "143-03.bin", 0xc0001, 0x2000, CRC(d0706345) SHA1(e04bb40d944426a4ae2e3a614d3f4953d7132ede),ROM_SKIP(1))
	//ROMX_LOAD( "143-02.bin", 0xc0000, 0x2000, CRC(c8c18ebb) SHA1(fd6dac76d43ab8b582e70f1d5cc931d679036fb9),ROM_SKIP(1))
ROM_END

ROM_START( poisk2 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v20", "v2.0")
	ROMX_LOAD( "b_p2_20h.rf4", 0xfc001, 0x2000, CRC(d53189b7) SHA1(ace40f1a40642b51fe5d2874acef81e48768b23b), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "b_p2_20l.rf4", 0xfc000, 0x2000, CRC(2d61fcc9) SHA1(11873c8741ba37d6c2fe1f482296aece514b7618), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v21", "v2.1")
	ROMX_LOAD( "b_p2_21h.rf4", 0xfc001, 0x2000, CRC(22197297) SHA1(506c7e63027f734d62ef537f484024548546011f), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "b_p2_21l.rf4", 0xfc000, 0x2000, CRC(0eb2ea7f) SHA1(67bb5fec53ebfa2a5cad2a3d3d595678d6023024), ROM_SKIP(1) | ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v24", "v2.4")
	ROMX_LOAD( "b_p2_24h.rf4", 0xfc001, 0x2000, CRC(ea842c9e) SHA1(dcdbf27374149dae0ef76d410cc6c615d9b99372), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "b_p2_24l.rf4", 0xfc000, 0x2000, CRC(02f21250) SHA1(f0b133fb4470bddf2f7bf59688cf68198ed8ce55), ROM_SKIP(1) | ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "v21d", "v2.1d")
	ROMX_LOAD( "opp2_1h.rf4", 0xfc001, 0x2000, CRC(b7cd7f4f) SHA1(ac473822fb44d7b898d628732cf0a27fcb4d26d6), ROM_SKIP(1) | ROM_BIOS(4))
	ROMX_LOAD( "opp2_1l.rf4", 0xfc000, 0x2000, CRC(1971dca3) SHA1(ecd61cc7952af834d8abc11db372c3e70775489d), ROM_SKIP(1) | ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "v22d", "v2.2d")
	ROMX_LOAD( "opp2_2h.rf4", 0xfc001, 0x2000, CRC(b9e3a5cc) SHA1(0a28afbff612471ee81d69a98789e75253c57a30), ROM_SKIP(1) | ROM_BIOS(5))
	ROMX_LOAD( "opp2_2l.rf4", 0xfc000, 0x2000, CRC(6877aad6) SHA1(1d0031d044beb4f9f321e3c8fdedf57467958900), ROM_SKIP(1) | ROM_BIOS(5))
	ROM_SYSTEM_BIOS(5, "v23d", "v2.3d")
	ROMX_LOAD( "opp2_3h.rf4", 0xfc001, 0x2000, CRC(ac7d4f06) SHA1(858d6e084a38814280b3e29fb54971f4f532e484), ROM_SKIP(1) | ROM_BIOS(6))
	ROMX_LOAD( "opp2_3l.rf4", 0xfc000, 0x2000, CRC(3c877ea1) SHA1(0753168659653538311c0ad1df851cbbdba426f4), ROM_SKIP(1) | ROM_BIOS(6))
ROM_END

ROM_START( mc1702 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_LOAD16_BYTE( "2764_2_(573rf4).rom", 0xfc000,  0x2000, CRC(34a0c8fb) SHA1(88dc247f2e417c2848a2fd3e9b52258ad22a2c07))
	ROM_LOAD16_BYTE( "2764_3_(573rf4).rom", 0xfc001, 0x2000, CRC(68ab212b) SHA1(f3313f77392877d28ce290ffa3432f0a32fc4619))
	ROM_LOAD( "ba1m_(573rf5).rom", 0x0000, 0x0800, CRC(08d938e8) SHA1(957b6c691dbef75c1c735e8e4e81669d056971e4))
ROM_END

ROM_START( ibm5550 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_LOAD( "ipl5550.rom", 0xfc000, 0x4000, CRC(40cf34c9) SHA1(d41f77fdfa787b0e97ed311e1c084b8699a5b197))
ROM_END

ROM_START( pc7000 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROMX_LOAD( "mitsubishi-m5l27128k-1.bin", 0xf8000, 0x4000, CRC(9683957f) SHA1(4569eab6d88eb1bba0d553d1358e593c326978aa), ROM_SKIP(1))
	ROMX_LOAD( "mitsubishi-m5l27128k-2.bin", 0xf8001, 0x4000, CRC(99b229a4) SHA1(5800c8bafed26873d8cfcc79a05f93a780a31c91), ROM_SKIP(1))
ROM_END

ROM_START( olivm15 )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "oliv_m15.bin",0xfc000, 0x04000, CRC(bf2ef795) SHA1(02d497131f5ca2c78f2accd38ab0eab6813e3ebf))
ROM_END

ROM_START( sx16 )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "tmm27128ad.bin",0xfc000, 0x4000, CRC(f8543362) SHA1(fef625e260ca89ba02174584bdc12db609f0780e))
ROM_END

ROM_START( compc1 )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("380270-01.bin", 0xfc000, 0x4000, BAD_DUMP CRC(75135d37) SHA1(177283642240fee191ba2d87e1d0c2a377c78ccb))
	ROM_REGION(0x8000, "gfx1", 0)
	ROM_LOAD("pc1_char.bin", 0x0000, 0x8000, CRC(4773a945) SHA1(bcc38abecc75d3f641d42987cb0d2ed71d71bc4c))
ROM_END

// Note: Commodore PC20-III, PC10-III and COLT share the same BIOS
ROM_START( pc10iii )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_DEFAULT_BIOS("v441")
	ROM_SYSTEM_BIOS(0, "v435", "v4.35")
	ROMX_LOAD("318085-01.u201", 0xf8000, 0x8000, NO_DUMP, ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v436", "v4.36")
	ROMX_LOAD("318085-02.u201", 0xf8000, 0x8000, NO_DUMP, ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v436c", "v4.36c")
	ROMX_LOAD("318085-04.u201", 0xf8000, 0x8000, NO_DUMP, ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "v438", "v4.38")
	ROMX_LOAD("318085-05.u201", 0xf8000, 0x8000, CRC(ae9e6a31) SHA1(853ee251cf230818c407a8d13ef060a21c90a8c1), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "v439", "v4.39")
	ROMX_LOAD("318085-06.u201", 0xf8000, 0x8000, NO_DUMP, ROM_BIOS(5))
	ROM_SYSTEM_BIOS(5, "v440", "v4.40")
	ROMX_LOAD("318085-07.u201", 0xf8000, 0x8000, NO_DUMP, ROM_BIOS(6))
	ROM_SYSTEM_BIOS(6, "v441", "v4.41")
	ROMX_LOAD("318085-08.u201", 0xf8000, 0x8000, CRC(7e228dc8) SHA1(958dfdd637bd31c01b949fac729d6973a7e630bc), ROM_BIOS(7))
	ROM_REGION(0x8000, "gfx1", 0)
	ROM_LOAD("318086-02.u607", 0x0000, 0x8000, CRC(b406651c) SHA1(856f58353391a74a06ebb8ec9f8333d7d69e5fd6))
	//ROM_LOAD("5788005.u33", 0x00000, 0x2000, BAD_DUMP CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* temp so you can read the text */
ROM_END

ROM_START( mbc16 )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "mbc16.bin", 0xfc000, 0x4000, CRC(f3e0934a) SHA1(e4b91c3d395be0414e20f23ad4919b8ac52639b2))
	ROM_REGION(0x2000,"gfx1", 0)
	//ATI Graphics Solution SR (graphics card, need to make it ISA card)
	ROM_LOAD( "atigssr.bin", 0x0000, 0x2000, CRC(aca81498) SHA1(0d84c89487ee7a6ac4c9e73fdb30c5fd8aa595f8))
ROM_END

ROM_START( ataripc3 )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "c101701-004 308.u61",0xf8000, 0x8000, CRC(929a2443) SHA1(8e98f3c9180c55b1f5521727779c016083d27960))

	ROM_REGION(0x8000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, BAD_DUMP CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) // not the real character ROM

	ROM_REGION(0x8000,"plds", 0)
	ROM_LOAD( "c101681 6ffb.u60",0x000, 0x100, NO_DUMP ) // PAL20L10NC
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT      MACHINE     INPUT       INIT        COMPANY            FULLNAME */
COMP( 1984, dgone,      ibm5150,    0,          pccga,      pccga, driver_device,      0,      "Data General", "Data General/One" , MACHINE_NOT_WORKING)/* CGA, 2x 3.5" disk drives */
COMP( 1985, bw230,      ibm5150,    0,          bondwell,   bondwell, pc_state,   bondwell,   "Bondwell Holding", "BW230 (PRO28 Series)", 0 )
COMP( 1984, compc1,     ibm5150,    0,          pccga,      pccga, driver_device,      0,      "Commodore Business Machines", "Commodore PC-1" , MACHINE_NOT_WORKING)
COMP( 1987, pc10iii,    ibm5150,    0,          pccga,      pccga, driver_device,      0,      "Commodore Business Machines", "Commodore PC-10 III" , MACHINE_NOT_WORKING)

COMP( 1992, iskr3104,   ibm5150,    0,          iskr3104,   pccga, driver_device,      0,      "Schetmash", "Iskra 3104", MACHINE_NOT_WORKING)
COMP( 1989, mk88,       ibm5150,    0,          mk88,       pccga, driver_device,      0,      "<unknown>", "MK-88", MACHINE_NOT_WORKING)
COMP( 1991, poisk2,     ibm5150,    0,          poisk2,     pccga, driver_device,      0,      "<unknown>", "Poisk-2", MACHINE_NOT_WORKING)
COMP( 1990, mc1702,     ibm5150,    0,          pccga,      pccga, driver_device,      0,      "<unknown>", "Elektronika MC-1702", MACHINE_NOT_WORKING)

COMP( 1987, zdsupers,   ibm5150,    0,          zenith,     pccga, driver_device,      0,      "Zenith Data Systems", "SuperSport", 0)

COMP( 198?, olivm15,    ibm5150,    0,          pccga,      pccga, driver_device,      0,      "Olivetti", "M15", MACHINE_NOT_WORKING | MACHINE_NO_SOUND) // is this a pc clone or not?

COMP( 1983, ibm5550,    ibm5150,    0,          ibm5550,    pccga, driver_device,      0,      "International Business Machines", "IBM 5550", MACHINE_NOT_WORKING)

COMP( 1985, pc7000,     ibm5150,    0,          pccga,      pccga, driver_device,      0,      "Sharp", "PC-7000", MACHINE_NOT_WORKING)

COMP( 1988, sx16,       ibm5150,    0,          pccga,      pccga, driver_device,      0,      "Sanyo", "SX-16", MACHINE_NOT_WORKING)
COMP( 198?, mbc16,      ibm5150,    0,          pccga,      pccga, driver_device,      0,      "Sanyo", "MBC-16" , MACHINE_NOT_WORKING)

COMP( 198?, ataripc3,   ibm5150,    0,          pccga,      pccga, driver_device,      0,      "Atari", "PC-3" , MACHINE_NOT_WORKING)
COMP( 1989, ssam88s,    ibm5150,    0,          pccga,      pccga, driver_device,      0,      "Samsung", "Samtron 88S" , MACHINE_NOT_WORKING)
