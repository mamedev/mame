// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    drivers/ec184x.c

    Driver file for EC-184x series

    To do:
    - verify ec1840 clocks etc.
    - did 640KB memory board exist or it's 512+128 boards?

***************************************************************************/


#include "emu.h"
#include "machine/genpc.h"

#include "bus/isa/xsu_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/i86/i86.h"
#include "machine/ram.h"

#include "softlist_dev.h"


#define LOG_KEYBOARD  (1U << 1)
#define LOG_DEBUG     (1U << 2)

//#define VERBOSE (LOG_DEBUG)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGKBD(...) LOGMASKED(LOG_KEYBOARD, __VA_ARGS__)
#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


namespace {

static constexpr int EC1841_MEMBOARD_SIZE = 512 * 1024;


class ec184x_state : public driver_device
{
public:
	ec184x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
	{ }

	void ec1840(machine_config &config);
	void ec1841(machine_config &config);

	void init_ec1840();
	void init_ec1841();

private:
	DECLARE_MACHINE_RESET(ec1841);
	uint8_t memboard_r(offs_t offset);
	void memboard_w(offs_t offset, uint8_t data);

	void ec1840_io(address_map &map) ATTR_COLD;
	void ec1840_map(address_map &map) ATTR_COLD;
	void ec1841_io(address_map &map) ATTR_COLD;
	void ec1841_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;

	struct
	{
		uint8_t enable[4]{};
		int boards = 0;
	} m_memory;
};

/*
 * EC-1841 memory controller.  The machine can hold four memory boards;
 * each board has a control register, its address is set by a DIP switch
 * on the board itself.
 *
 * Only one board should be enabled for read, and one for write.
 * Normally, this is the same board.
 *
 * Each board is divided into 4 banks, internally numbered 0..3.
 * POST tests each board on startup, and an error (indicated by
 * I/O CH CK bus signal) causes it to disable failing bank(s) by writing
 * 'reconfiguration code' (inverted number of failing memory bank) to
 * the register.

 * bit 1-0  'reconfiguration code'
 * bit 2    enable read access
 * bit 3    enable write access
 */

uint8_t ec184x_state::memboard_r(offs_t offset)
{
	uint8_t data;

	data = offset % 4;
	if (data >= m_memory.boards)
		data = 0xff;
	else
		data = m_memory.enable[data];
	LOG("ec1841_memboard R (%d of %d) == %02X\n", offset + 1, m_memory.boards, data);

	return data;
}

void ec184x_state::memboard_w(offs_t offset, uint8_t data)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	uint8_t current = m_memory.enable[offset];

	LOG("ec1841_memboard W (%d of %d) <- %02X (%02X)\n", offset + 1, m_memory.boards, data, current);

	if (offset >= m_memory.boards)
	{
		return;
	}

	if (BIT(current, 2) && !BIT(data, 2))
	{
		// disable read access
		program.unmap_read(0, EC1841_MEMBOARD_SIZE - 1);
		LOG("ec1841_memboard_w unmap_read(%d)\n", offset);
	}

	if (BIT(current, 3) && !BIT(data, 3))
	{
		// disable write access
		program.unmap_write(0, EC1841_MEMBOARD_SIZE - 1);
		LOG("ec1841_memboard_w unmap_write(%d)\n", offset);
	}

	if (!BIT(current, 2) && BIT(data, 2))
	{
		for (int i = 0; i < 4; i++)
			m_memory.enable[i] &= 0xfb;
		// enable read access
		program.install_rom(0, EC1841_MEMBOARD_SIZE - 1, m_ram->pointer() + offset * EC1841_MEMBOARD_SIZE);
		LOG("ec1841_memboard_w map_read(%d)\n", offset);
	}

	if (!BIT(current, 3) && BIT(data, 3))
	{
		for (int i = 0; i < 4; i++)
			m_memory.enable[i] &= 0xf7;
		// enable write access
		program.install_writeonly(0, EC1841_MEMBOARD_SIZE - 1, m_ram->pointer() + offset * EC1841_MEMBOARD_SIZE);
		LOG("ec1841_memboard_w map_write(%d)\n", offset);
	}

	m_memory.enable[offset] = data;
}

void ec184x_state::init_ec1840()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	program.install_ram(0, m_ram->size() - 1, m_ram->pointer());
}

void ec184x_state::init_ec1841()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	m_memory.boards = m_ram->size() / EC1841_MEMBOARD_SIZE;
	if (m_memory.boards > 4) m_memory.boards = 4;

	program.install_ram(0, EC1841_MEMBOARD_SIZE - 1, m_ram->pointer());

	// 640K configuration is special -- 512K board mapped at 0 + 128K board mapped at 512K
	if (m_ram->size() == 640 * 1024)
	{
		program.install_ram(EC1841_MEMBOARD_SIZE, m_ram->size() - 1, m_ram->pointer() + EC1841_MEMBOARD_SIZE);
	}
}

MACHINE_RESET_MEMBER(ec184x_state, ec1841)
{
	memset(m_memory.enable, 0, sizeof(m_memory.enable));
	// mark 1st board enabled
	m_memory.enable[0] = 0xc;
}


void ec184x_state::ec1840_map(address_map &map)
{
	map.unmap_value_high();
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void ec184x_state::ec1841_map(address_map &map)
{
	map.unmap_value_high();
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void ec184x_state::ec1840_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m("mb", FUNC(ec1840_mb_device::map));
}

void ec184x_state::ec1841_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m("mb", FUNC(ec1841_mb_device::map));
	map(0x02b0, 0x02b3).rw(FUNC(ec184x_state::memboard_r), FUNC(ec184x_state::memboard_w));
}


void ec184x_state::ec1840(machine_config &config)
{
	I8086(config, m_maincpu, 4096000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ec184x_state::ec1840_map);
	m_maincpu->set_addrmap(AS_IO, &ec184x_state::ec1840_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259", FUNC(pic8259_device::inta_cb));

	ec1840_mb_device &mb(EC1840_MOTHERBOARD(config, "mb"));
	mb.set_cputag(m_maincpu);
	mb.int_callback().set_inputline(m_maincpu, 0);
	mb.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	mb.kbdclk_callback().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	mb.kbddata_callback().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	// FIXME: determine ISA bus clock
	// 7-slot backplane, at least two slots are always taken by CPU and memory cards
	ISA8_SLOT(config, "isa1", 0, "mb:isa", ec184x_isa8_cards, "ec1840.0002", false);
	ISA8_SLOT(config, "isa2", 0, "mb:isa", ec184x_isa8_cards, "ec1840.0003", false);
	ISA8_SLOT(config, "isa3", 0, "mb:isa", ec184x_isa8_cards, "ec1840.0004", false);
	ISA8_SLOT(config, "isa4", 0, "mb:isa", ec184x_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa5", 0, "mb:isa", ec184x_isa8_cards, nullptr, false);

	SOFTWARE_LIST(config, "flop_list").set_original("ec1841");

	pc_kbdc_device &kbd(PC_KBDC(config, "kbd", pc_xt_keyboards, STR_KBD_EC_1841));
	kbd.out_clock_cb().set("mb", FUNC(ec1840_mb_device::keyboard_clock_w));
	kbd.out_data_cb().set("mb", FUNC(ec1840_mb_device::keyboard_data_w));

	RAM(config, m_ram).set_default_size("640K").set_extra_options("128K,256K,384K,512K");
}

void ec184x_state::ec1841(machine_config &config)
{
	I8086(config, m_maincpu, XTAL(12'288'000) / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &ec184x_state::ec1841_map);
	m_maincpu->set_addrmap(AS_IO, &ec184x_state::ec1841_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259", FUNC(pic8259_device::inta_cb));

	MCFG_MACHINE_RESET_OVERRIDE(ec184x_state, ec1841)

	ec1841_mb_device &mb(EC1841_MOTHERBOARD(config, "mb"));
	mb.set_cputag(m_maincpu);
	mb.int_callback().set_inputline(m_maincpu, 0);
	mb.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	mb.kbdclk_callback().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	mb.kbddata_callback().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	// FIXME: determine ISA bus clock
	ISA8_SLOT(config, "isa1", 0, "mb:isa", ec184x_isa8_cards, "ec1841.0002", false); // cga
	ISA8_SLOT(config, "isa2", 0, "mb:isa", ec184x_isa8_cards, "ec1841.0003", false); // fdc (IRQ6) + mouse port (IRQ2..5)
	ISA8_SLOT(config, "isa3", 0, "mb:isa", ec184x_isa8_cards, "ec1840.0004", false); // lpt (IRQ7||5) [+ serial (IRQx)]
	ISA8_SLOT(config, "isa4", 0, "mb:isa", ec184x_isa8_cards, "hdc", false);
	ISA8_SLOT(config, "isa5", 0, "mb:isa", ec184x_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa6", 0, "mb:isa", ec184x_isa8_cards, nullptr, false);

	SOFTWARE_LIST(config, "flop_list").set_original("ec1841");

	pc_kbdc_device &kbd(PC_KBDC(config, "kbd", pc_xt_keyboards, STR_KBD_EC_1841));
	kbd.out_clock_cb().set("mb", FUNC(ec1841_mb_device::keyboard_clock_w));
	kbd.out_data_cb().set("mb", FUNC(ec1841_mb_device::keyboard_data_w));

	RAM(config, m_ram).set_default_size("640K").set_extra_options("512K,1024K,1576K,2048K");
}

ROM_START( ec1840 )
	ROM_REGION16_LE(0x10000,"bios", 0)
	ROM_DEFAULT_BIOS("v4")
	// supports MDA only
	ROM_SYSTEM_BIOS(0, "v1", "EC-1840.01")
	ROMX_LOAD("000-01.bin", 0xe000, 0x0800, CRC(c3ab1fad) SHA1(8168bdee30698f4f9aa7bbb6dfabe62dd723cec5), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("001-01.bin", 0xf000, 0x0800, CRC(601d1155) SHA1(9684d33b92743749704587a48e679ef7a3b20f9c), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("002-01.bin", 0xe001, 0x0800, CRC(ce4dddb7) SHA1(f9b1da60c848e68ff1c154d695a36a0833de4804), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("003-01.bin", 0xf001, 0x0800, CRC(14b40431) SHA1(ce7fffa41897405ee64fd4e86015e774f8bd108a), ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "v4", "EC-1840.04")
	ROMX_LOAD("000-04-971b.bin", 0xe000, 0x0800, CRC(06aeaee8) SHA1(9f954e4c48156d573a8e0109e7ca652be9e6036a), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("001-04-92b7.bin", 0xf000, 0x0800, CRC(3fae650a) SHA1(c98b777fdeceadd72d6eb9465b3501b9ead55a08), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("002-04-9e17.bin", 0xe001, 0x0800, CRC(d59712df) SHA1(02ea1b3ae9662f5c64c58920a32ca9db0f6fbd12), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("003-04-3ccb.bin", 0xf001, 0x0800, CRC(7fc362c7) SHA1(538e13639ad2b4c30bd72582e323181e63513306), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
ROM_END

ROM_START( ec1841 )
	ROM_REGION16_LE(0x10000,"bios", 0)
	ROM_DEFAULT_BIOS("v2")
	ROM_SYSTEM_BIOS(0, "v1", "EC-1841.01")
	ROMX_LOAD("012-01-3107.bin", 0xc000, 0x0800, CRC(77957396) SHA1(785f1dceb6e2b4618f5c5f0af15eb74a8c951448), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("013-01-203f.bin", 0xc001, 0x0800, CRC(768bd3d5) SHA1(2e948f2ad262de306d889b7964c3f1aad45ff5bc), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("014-01-fa40.bin", 0xd000, 0x0800, CRC(47722b58) SHA1(a6339ee8af516f834826b7828a5cf79cb650480c), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("015-01-bf1d.bin", 0xd001, 0x0800, CRC(b585b5ea) SHA1(d0ebed586eb13031477c2e071c50416682f80489), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("016-01-65f7.bin", 0xe000, 0x0800, CRC(28a07db4) SHA1(17fbcd60dacd1d3f8d8355db429f97e4d1d1ac88), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("017-01-5be1.bin", 0xe001, 0x0800, CRC(928bda26) SHA1(ee889184067e2680b29a8ef1c3a76cf5afd4c78d), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("018-01-7090.bin", 0xf000, 0x0800, CRC(75ca7d7e) SHA1(6356426820c5326a7893a437d54b02f250ef8609), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("019-01-0492.bin", 0xf001, 0x0800, CRC(8a9d593e) SHA1(f3936d2cb4e6d130dd732973f126c3aa20612463), ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "v2", "EC-1841.02")
	ROMX_LOAD("012-02-37f6.bin", 0xc000, 0x0800, CRC(8f5c6a20) SHA1(874b62f9cee8d3b974f33732f94eff10fc002c44), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("013-02-2552.bin", 0xc001, 0x0800, CRC(e3c10128) SHA1(d6ed743ebe9c130925c9f17aad1a45db9194c967), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("014-02-0fbe.bin", 0xd000, 0x0800, CRC(f8517e5e) SHA1(8034cd6ff5778365dc9daa494524f1753a74f1ed), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("015-02-d736.bin", 0xd001, 0x0800, CRC(8538c52a) SHA1(ee981ce90870b6546a18f2a2e64d71b0038ce0dd), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("016-02-5b2c.bin", 0xe000, 0x0800, CRC(3d1d1e67) SHA1(c527e29796537787c0f6c329f3c203f6131ca77f), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("017-02-4b9d.bin", 0xe001, 0x0800, CRC(1b985264) SHA1(5ddcb9c13564be208c5068c105444a87159c67ee), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("018-02-7090.bin", 0xf000, 0x0800, CRC(75ca7d7e) SHA1(6356426820c5326a7893a437d54b02f250ef8609), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("019-02-0493.bin", 0xf001, 0x0800, CRC(61aae23d) SHA1(7b3aa24a63ee31b194297eb1e61c3827edfcb95a), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "v3", "EC-1841.03")
	ROMX_LOAD("012-03-37e7.bin", 0xc000, 0x0800, CRC(49992bd5) SHA1(119121e1b4af1c44b9b8c2edabe7dc1d3019c4a6), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("013-03-2554.bin", 0xc001, 0x0800, CRC(834bd7d7) SHA1(e37514fc4cb8a5cbe68e7564e0e07d5116c4021a), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("014-03-0fbe.bin", 0xd000, 0x0800, CRC(f8517e5e) SHA1(8034cd6ff5778365dc9daa494524f1753a74f1ed), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("015-03-d736.bin", 0xd001, 0x0800, CRC(8538c52a) SHA1(ee981ce90870b6546a18f2a2e64d71b0038ce0dd), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("016-03-5b2c.bin", 0xe000, 0x0800, CRC(3d1d1e67) SHA1(c527e29796537787c0f6c329f3c203f6131ca77f), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("017-03-4b9d.bin", 0xe001, 0x0800, CRC(1b985264) SHA1(5ddcb9c13564be208c5068c105444a87159c67ee), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("018-03-7090.bin", 0xf000, 0x0800, CRC(75ca7d7e) SHA1(6356426820c5326a7893a437d54b02f250ef8609), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("019-03-0493.bin", 0xf001, 0x0800, CRC(61aae23d) SHA1(7b3aa24a63ee31b194297eb1e61c3827edfcb95a), ROM_SKIP(1) | ROM_BIOS(2))
ROM_END

ROM_START( ec1845 )
	ROM_REGION16_LE(0x10000,"bios", 0)
	ROMX_LOAD("184500.bin", 0xc000, 0x0800, CRC(7c472ef7) SHA1(3af53f27b49bbc731bf51f9300fbada23a1bfcfc), ROM_SKIP(1))
	ROMX_LOAD("184501.bin", 0xc001, 0x0800, CRC(db240dc6) SHA1(d7bb022213d09bbf2a8107fe4f1cd27b23939e18), ROM_SKIP(1))
	ROMX_LOAD("184502.bin", 0xd000, 0x0800, CRC(149e7e29) SHA1(7f2a297588fef1bc750c57e6ae0d5acf3d27c486), ROM_SKIP(1))
	ROMX_LOAD("184503.bin", 0xd001, 0x0800, CRC(e28cbd74) SHA1(cf1fba4e67c8e1dd8cdda547118e84b704029b03), ROM_SKIP(1))
	ROMX_LOAD("184504.bin", 0xe000, 0x0800, CRC(55fa7a1d) SHA1(58f7abab08b9d2f0a1c1636e11bb72af2694c95f), ROM_SKIP(1))
	ROMX_LOAD("184505.bin", 0xe001, 0x0800, CRC(c807e3f5) SHA1(08117e449f0d04f96041cff8d34893f500f3760d), ROM_SKIP(1))
	ROMX_LOAD("184506.bin", 0xf000, 0x0800, CRC(24f5c27c) SHA1(7822dd7f715ef00ccf6d8408be8bbfe01c2eba20), ROM_SKIP(1))
	ROMX_LOAD("184507.bin", 0xf001, 0x0800, CRC(75122203) SHA1(7b0fbdf1315230633e39574ac7360163bc7361e1), ROM_SKIP(1))
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT   COMPAT  MACHINE  INPUT  STATE         INIT         COMPANY      FULLNAME   FLAGS
COMP( 1986, ec1840, ibm5150, 0,      ec1840,  0,     ec184x_state, init_ec1840, "<unknown>", "EC-1840", 0 )
COMP( 1987, ec1841, ibm5150, 0,      ec1841,  0,     ec184x_state, init_ec1841, "<unknown>", "EC-1841", 0 )
COMP( 1989, ec1845, ibm5150, 0,      ec1841,  0,     ec184x_state, init_ec1841, "<unknown>", "EC-1845", MACHINE_NOT_WORKING )
