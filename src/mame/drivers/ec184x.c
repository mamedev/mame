// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    drivers/ec184x.c

    Driver file for EC-184x series

    TODO (ec1840)
    - memory bank size is smaller (128K)

    TODO (ec1841)
    - hard disk is connected but requires changes to isa_hdc.c

***************************************************************************/


#include "emu.h"

#include "includes/genpc.h"

#include "bus/isa/xsu_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "cpu/i86/i86.h"
#include "machine/ram.h"

#define VERBOSE_DBG 1       /* general debug messages */

#define DBG_LOG(N,M,A) \
	do { \
	if(VERBOSE_DBG>=N) \
		{ \
			if( M ) \
				logerror("%11.6f at %s: %-24s",machine().time().as_double(),machine().describe_context(),(char*)M ); \
			logerror A; \
		} \
	} while (0)



class ec184x_state : public driver_device
{
public:
	ec184x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	required_device<cpu_device> m_maincpu;

	DECLARE_MACHINE_RESET(ec184x);
	DECLARE_DRIVER_INIT(ec184x);

	struct {
		UINT8 enable[4];
		int boards;
		int board_size;
	} m_memory;

	DECLARE_READ8_MEMBER(memboard_r);
	DECLARE_WRITE8_MEMBER(memboard_w);
};

/*
 * EC-1841 memory controller.  The machine can hold four memory boards;
 * each board has a control register, its address is set by a DIP switch
 * on the board itself.
 *
 * Only one board should be enabled for read, and one for write.
 * Normally, this is the same board.
 *
 * Each board is divided into 4 banks, internally numbererd 0..3.
 * POST tests each board on startup, and an error (indicated by
 * I/O CH CK bus signal) causes it to disable failing bank(s) by writing
 * 'reconfiguration code' (inverted number of failing memory bank) to
 * the register.

 * bit 1-0  'reconfiguration code'
 * bit 2    enable read access
 * bit 3    enable write access
 */

READ8_MEMBER(ec184x_state::memboard_r)
{
	UINT8 data;

	data = offset % 4;
	if (data > m_memory.boards)
		data = 0xff;
	else
		data = m_memory.enable[data];
	DBG_LOG(1,"ec1841_memboard",("R (%d of %d) == %02X\n", offset, m_memory.boards, data ));

	return data;
}

WRITE8_MEMBER(ec184x_state::memboard_w)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	ram_device *m_ram = machine().device<ram_device>(RAM_TAG);
	UINT8 current;

	current = m_memory.enable[offset];

	DBG_LOG(1,"ec1841_memboard",("W (%d of %d) <- %02X (%02X)\n", offset, m_memory.boards, data, current));

	if (offset > m_memory.boards) {
		return;
	}

	if (BIT(current, 2) && !BIT(data, 2)) {
		// disable read access
		program.unmap_read(0, m_memory.board_size-1);
		DBG_LOG(1,"ec1841_memboard_w",("unmap_read(%d)\n", offset));
	}

	if (BIT(current, 3) && !BIT(data, 3)) {
		// disable write access
		program.unmap_write(0, 0x7ffff);
		DBG_LOG(1,"ec1841_memboard_w",("unmap_write(%d)\n", offset));
	}

	if (!BIT(current, 2) && BIT(data, 2)) {
		for(int i=0; i<4; i++)
			m_memory.enable[i] &= 0xfb;
		// enable read access
		membank("bank10")->set_base(m_ram->pointer() + offset*0x80000);
		program.install_read_bank(0, m_memory.board_size-1, "bank10");
		DBG_LOG(1,"ec1841_memboard_w",("map_read(%d)\n", offset));
	}

	if (!BIT(current, 3) && BIT(data, 3)) {
		for(int i=0; i<4; i++)
			m_memory.enable[i] &= 0xf7;
		// enable write access
		membank("bank20")->set_base(m_ram->pointer() + offset*0x80000);
		program.install_write_bank(0, m_memory.board_size-1, "bank20");
		DBG_LOG(1,"ec1841_memboard_w",("map_write(%d)\n", offset));
	}

	m_memory.enable[offset] = data;
}

DRIVER_INIT_MEMBER( ec184x_state, ec184x )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	ram_device *m_ram = machine().device<ram_device>(RAM_TAG);

	m_memory.board_size = 512 * 1024; // XXX
	m_memory.boards = m_ram->size()/m_memory.board_size - 1;
	if (m_memory.boards > 3)
		m_memory.boards = 3;

	program.install_read_bank(0, m_memory.board_size-1, "bank10");
	program.install_write_bank(0, m_memory.board_size-1, "bank20");
	membank( "bank10" )->set_base( m_ram->pointer() );
	membank( "bank20" )->set_base( m_ram->pointer() );
}

MACHINE_RESET_MEMBER( ec184x_state, ec184x )
{
	memset(m_memory.enable, 0, sizeof(m_memory.enable));
	// mark 1st board enabled
	m_memory.enable[0] = 0xc;
}


static ADDRESS_MAP_START( ec1840_map, AS_PROGRAM, 8, ec184x_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x7ffff) AM_RAM
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xdc000, 0xdffff) AM_RAM
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ec1841_map, AS_PROGRAM, 16, ec184x_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x7ffff) AM_RAM
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ec1847_map, AS_PROGRAM, 8, ec184x_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xdc000, 0xdffff) AM_RAM
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ec1840_io, AS_IO, 8, ec184x_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static ADDRESS_MAP_START( ec1841_io, AS_IO, 16, ec184x_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x02b0, 0x02b3) AM_READWRITE8(memboard_r, memboard_w, 0xffff);
ADDRESS_MAP_END

static ADDRESS_MAP_START( ec1847_io, AS_IO, 8, ec184x_state )
	ADDRESS_MAP_UNMAP_HIGH
//  AM_RANGE(0x0210, 0x021f) AM_RAM // internal (non-standard?) bus extender
ADDRESS_MAP_END


static INPUT_PORTS_START( ec1841 )
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( ec1840 )
	DEVICE_INPUT_DEFAULTS("DSW0", 0x31, 0x21)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START( ec1841 )
	DEVICE_INPUT_DEFAULTS("DSW0", 0x31, 0x21)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START( ec1847 )
	DEVICE_INPUT_DEFAULTS("DSW0", 0x31, 0x31)
DEVICE_INPUT_DEFAULTS_END

// XXX verify everything
static MACHINE_CONFIG_START( ec1840, ec184x_state )
	MCFG_CPU_ADD("maincpu", I8088, 4096000)
	MCFG_CPU_PROGRAM_MAP(ec1840_map)
	MCFG_CPU_IO_MAP(ec1840_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_IBM5150_MOTHERBOARD_ADD("mb","maincpu")
	MCFG_DEVICE_INPUT_DEFAULTS(ec1840)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", ec184x_isa8_cards, "ec1840.0002", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", ec184x_isa8_cards, "ec1841.0003", false)   // actually ec1840.0003 -- w/o mouse port
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", ec184x_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", ec184x_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa5", ec184x_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa6", ec184x_isa8_cards, NULL, false)

	MCFG_SOFTWARE_LIST_ADD("flop_list","ec1841")

	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_EC_1841)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ec1841, ec184x_state )
	MCFG_CPU_ADD("maincpu", I8086, 4096000)
	MCFG_CPU_PROGRAM_MAP(ec1841_map)
	MCFG_CPU_IO_MAP(ec1841_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_MACHINE_RESET_OVERRIDE(ec184x_state, ec184x)

	MCFG_EC1841_MOTHERBOARD_ADD("mb", "maincpu")
	MCFG_DEVICE_INPUT_DEFAULTS(ec1841)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", ec184x_isa8_cards, "ec1841.0002", false)   // cga
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", ec184x_isa8_cards, "ec1841.0003", false)   // fdc + mouse port
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", ec184x_isa8_cards, "hdc", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", ec184x_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa5", ec184x_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa6", ec184x_isa8_cards, NULL, false)

	MCFG_SOFTWARE_LIST_ADD("flop_list","ec1841")

	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_EC_1841)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")
	MCFG_RAM_EXTRA_OPTIONS("1024K,1576K,2048K") // 640K variant not emulated
MACHINE_CONFIG_END

// XXX verify everything
static MACHINE_CONFIG_START( ec1847, ec184x_state )
	MCFG_CPU_ADD("maincpu", I8088, 4772720)
	MCFG_CPU_PROGRAM_MAP(ec1847_map)
	MCFG_CPU_IO_MAP(ec1847_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb","maincpu")
	MCFG_DEVICE_INPUT_DEFAULTS(ec1847)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "hercules", false)  // cga, ega and vga(?) are options too
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "fdc_xt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, NULL, false)    // native variant (wd1010 + z80) not emulated
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, NULL, false)    // native serial (2x8251) not emulated
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa5", pc_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa6", pc_isa8_cards, NULL, false)

	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END

ROM_START( ec1840 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_DEFAULT_BIOS("v4")
	// supports MDA only
	ROM_SYSTEM_BIOS(0, "v1", "EC-1840.01")
	ROMX_LOAD( "000-01.bin", 0xfe000, 0x0800, CRC(c3ab1fad) SHA1(8168bdee30698f4f9aa7bbb6dfabe62dd723cec5), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "001-01.bin", 0xff000, 0x0800, CRC(601d1155) SHA1(9684d33b92743749704587a48e679ef7a3b20f9c), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "002-01.bin", 0xfe001, 0x0800, CRC(ce4dddb7) SHA1(f9b1da60c848e68ff1c154d695a36a0833de4804), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "003-01.bin", 0xff001, 0x0800, CRC(14b40431) SHA1(ce7fffa41897405ee64fd4e86015e774f8bd108a), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_SYSTEM_BIOS(1, "v4", "EC-1840.04")
	ROMX_LOAD( "000-04-971b.bin", 0xfe000, 0x0800, CRC(06aeaee8) SHA1(9f954e4c48156d573a8e0109e7ca652be9e6036a), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "001-04-92b7.bin", 0xff000, 0x0800, CRC(3fae650a) SHA1(c98b777fdeceadd72d6eb9465b3501b9ead55a08), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "002-04-9e17.bin", 0xfe001, 0x0800, CRC(d59712df) SHA1(02ea1b3ae9662f5c64c58920a32ca9db0f6fbd12), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "003-04-3ccb.bin", 0xff001, 0x0800, CRC(7fc362c7) SHA1(538e13639ad2b4c30bd72582e323181e63513306), ROM_SKIP(1) | ROM_BIOS(2))

	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
ROM_END

ROM_START( ec1841 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_DEFAULT_BIOS("v2")
	ROM_SYSTEM_BIOS(0, "v1", "EC-1841.01")
	ROMX_LOAD( "012-01-3107.bin", 0xfc000, 0x0800, CRC(77957396) SHA1(785f1dceb6e2b4618f5c5f0af15eb74a8c951448), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "013-01-203f.bin", 0xfc001, 0x0800, CRC(768bd3d5) SHA1(2e948f2ad262de306d889b7964c3f1aad45ff5bc), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "014-01-fa40.bin", 0xfd000, 0x0800, CRC(47722b58) SHA1(a6339ee8af516f834826b7828a5cf79cb650480c), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "015-01-bf1d.bin", 0xfd001, 0x0800, CRC(b585b5ea) SHA1(d0ebed586eb13031477c2e071c50416682f80489), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "016-01-65f7.bin", 0xfe000, 0x0800, CRC(28a07db4) SHA1(17fbcd60dacd1d3f8d8355db429f97e4d1d1ac88), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "017-01-5be1.bin", 0xfe001, 0x0800, CRC(928bda26) SHA1(ee889184067e2680b29a8ef1c3a76cf5afd4c78d), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "018-01-7090.bin", 0xff000, 0x0800, CRC(75ca7d7e) SHA1(6356426820c5326a7893a437d54b02f250ef8609), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "019-01-0492.bin", 0xff001, 0x0800, CRC(8a9d593e) SHA1(f3936d2cb4e6d130dd732973f126c3aa20612463), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_SYSTEM_BIOS(1, "v2", "EC-1841.02")
	ROMX_LOAD( "012-02-37f6.bin", 0xfc000, 0x0800, CRC(8f5c6a20) SHA1(874b62f9cee8d3b974f33732f94eff10fc002c44), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "013-02-2552.bin", 0xfc001, 0x0800, CRC(e3c10128) SHA1(d6ed743ebe9c130925c9f17aad1a45db9194c967), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "014-02-0fbe.bin", 0xfd000, 0x0800, CRC(f8517e5e) SHA1(8034cd6ff5778365dc9daa494524f1753a74f1ed), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "015-02-d736.bin", 0xfd001, 0x0800, CRC(8538c52a) SHA1(ee981ce90870b6546a18f2a2e64d71b0038ce0dd), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "016-02-5b2c.bin", 0xfe000, 0x0800, CRC(3d1d1e67) SHA1(c527e29796537787c0f6c329f3c203f6131ca77f), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "017-02-4b9d.bin", 0xfe001, 0x0800, CRC(1b985264) SHA1(5ddcb9c13564be208c5068c105444a87159c67ee), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "018-02-7090.bin", 0xff000, 0x0800, CRC(75ca7d7e) SHA1(6356426820c5326a7893a437d54b02f250ef8609), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "019-02-0493.bin", 0xff001, 0x0800, CRC(61aae23d) SHA1(7b3aa24a63ee31b194297eb1e61c3827edfcb95a), ROM_SKIP(1) | ROM_BIOS(2))

	ROM_SYSTEM_BIOS(2, "v3", "EC-1841.03")
	ROMX_LOAD( "012-03-37e7.bin", 0xfc000, 0x0800, CRC(49992bd5) SHA1(119121e1b4af1c44b9b8c2edabe7dc1d3019c4a6), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "013-03-2554.bin", 0xfc001, 0x0800, CRC(834bd7d7) SHA1(e37514fc4cb8a5cbe68e7564e0e07d5116c4021a), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "014-03-0fbe.bin", 0xfd000, 0x0800, CRC(f8517e5e) SHA1(8034cd6ff5778365dc9daa494524f1753a74f1ed), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "015-03-d736.bin", 0xfd001, 0x0800, CRC(8538c52a) SHA1(ee981ce90870b6546a18f2a2e64d71b0038ce0dd), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "016-03-5b2c.bin", 0xfe000, 0x0800, CRC(3d1d1e67) SHA1(c527e29796537787c0f6c329f3c203f6131ca77f), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "017-03-4b9d.bin", 0xfe001, 0x0800, CRC(1b985264) SHA1(5ddcb9c13564be208c5068c105444a87159c67ee), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "018-03-7090.bin", 0xff000, 0x0800, CRC(75ca7d7e) SHA1(6356426820c5326a7893a437d54b02f250ef8609), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "019-03-0493.bin", 0xff001, 0x0800, CRC(61aae23d) SHA1(7b3aa24a63ee31b194297eb1e61c3827edfcb95a), ROM_SKIP(1) | ROM_BIOS(3))
ROM_END

ROM_START( ec1845 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROMX_LOAD( "184500.bin", 0xfc000, 0x0800, CRC(7c472ef7) SHA1(3af53f27b49bbc731bf51f9300fbada23a1bfcfc), ROM_SKIP(1))
	ROMX_LOAD( "184501.bin", 0xfc001, 0x0800, CRC(db240dc6) SHA1(d7bb022213d09bbf2a8107fe4f1cd27b23939e18), ROM_SKIP(1))
	ROMX_LOAD( "184502.bin", 0xfd000, 0x0800, CRC(149e7e29) SHA1(7f2a297588fef1bc750c57e6ae0d5acf3d27c486), ROM_SKIP(1))
	ROMX_LOAD( "184503.bin", 0xfd001, 0x0800, CRC(e28cbd74) SHA1(cf1fba4e67c8e1dd8cdda547118e84b704029b03), ROM_SKIP(1))
	ROMX_LOAD( "184504.bin", 0xfe000, 0x0800, CRC(55fa7a1d) SHA1(58f7abab08b9d2f0a1c1636e11bb72af2694c95f), ROM_SKIP(1))
	ROMX_LOAD( "184505.bin", 0xfe001, 0x0800, CRC(c807e3f5) SHA1(08117e449f0d04f96041cff8d34893f500f3760d), ROM_SKIP(1))
	ROMX_LOAD( "184506.bin", 0xff000, 0x0800, CRC(24f5c27c) SHA1(7822dd7f715ef00ccf6d8408be8bbfe01c2eba20), ROM_SKIP(1))
	ROMX_LOAD( "184507.bin", 0xff001, 0x0800, CRC(75122203) SHA1(7b0fbdf1315230633e39574ac7360163bc7361e1), ROM_SKIP(1))
ROM_END

ROM_START( ec1847 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "vXXX", "EC-1847.0x")
	ROMX_LOAD( "308_d47_2764.bin", 0xc8000, 0x2000, CRC(f06924f2) SHA1(83a5dedf1c06f875c598f087bbc087524bc9bfa3), ROM_BIOS(1))
	ROMX_LOAD( "188m_d47_2764.bin", 0xf4000, 0x2000, CRC(bc8742c7) SHA1(3af09d14e891e976b7a9a2a6e1af63f0eabe5426), ROM_BIOS(1))
	ROMX_LOAD( "188m_d48_2764.bin", 0xfe000, 0x2000, CRC(7d290e95) SHA1(e73e6c8e19477fce5de3f95b89693dc6ad6781ab), ROM_BIOS(1))

	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
	ROM_LOAD( "317_d28_2732.bin", 0x00000, 0x1000, CRC(8939599b) SHA1(53d02460cf93596882a96758ef4bac5fa1ce55b2)) // monochrome font
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*     YEAR     ROM NAME    PARENT      COMPAT  MACHINE     INPUT                       INIT        COMPANY     FULLNAME */
COMP ( 1987,    ec1840,     ibm5150,    0,      ec1840,     ec1841,   ec184x_state,     ec184x,     "<unknown>",  "EC-1840", MACHINE_NOT_WORKING)
COMP ( 1987,    ec1841,     ibm5150,    0,      ec1841,     ec1841,   ec184x_state,     ec184x,     "<unknown>",  "EC-1841", 0)
COMP ( 1989,    ec1845,     ibm5150,    0,      ec1841,     ec1841,   ec184x_state,     ec184x,     "<unknown>",  "EC-1845", MACHINE_NOT_WORKING)
COMP ( 1990,    ec1847,     ibm5150,    0,      ec1847,     ec1841,   driver_device,    0,          "<unknown>",  "EC-1847", MACHINE_NOT_WORKING)
