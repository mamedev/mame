// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Countertop Champ II? (c) 1993 U.S. Games

Bartop style multigame/quiz/fortune teller
Dump contains a MS-DOS 3.3 ROM disk, ebay auction shows a Pine Technology PT-319A
(SARC RC2016A5 chipset) + 2 other populated ISA16 cards out of 6.

TODO:
- Does extensive checks to COM1, towards what it claims to be `mtv1` (MicroTouch?)
- ROM disk banking incomplete, meaning of $ca000 writes unknown;

**************************************************************************************************/

#include "emu.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/i386/i386.h"
#include "machine/at.h"
#include "machine/bankdev.h"
#include "machine/ds128x.h"
#include "machine/nvram.h"
#include "machine/ram.h"


namespace {

class champ2_state : public driver_device
{
public:
	champ2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mb(*this, "mb")
		, m_ram(*this, "ram")
		, m_bank(*this, "bank%u", 0U)
	{ }

	void champ2(machine_config &config);
	void init_at();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void init_at_common(int xmsbase);

private:
	required_device<i386_device> m_maincpu;
	required_device<at_mb_device> m_mb;
	required_device<ram_device> m_ram;
	required_device_array<address_map_bank_device, 2> m_bank;
	void bank_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


void champ2_state::main_map(address_map &map)
{
	map(0x000000, 0x09ffff).bankrw("bank10");
	// Selectable thru jumpers (0xc800, 0xd000, 0xd800, 0xe000)
	map(0x0c8000, 0x0c9fff).m(m_bank[0], FUNC(address_map_bank_device::amap8));
	map(0x0ca000, 0x0cbfff).m(m_bank[1], FUNC(address_map_bank_device::amap8));
	// writes to $+2000 then $+0000, same value (0x03) at POST,
	// then writes 0 to $+2000 and N to $+0000
	map(0x0c8000, 0x0c8000).lw8(
		NAME([this] (offs_t offset, u8 data) {
			logerror("$c8000 bank %02x\n", data);
			m_bank[1]->set_bank(data & 0x7f);
		})
	);
	map(0x0ca000, 0x0ca000).lw8(
		NAME([this] (offs_t offset, u8 data) {
			logerror("$ca000 bank %02x\n", data);
			//m_bank[0]->set_bank(data & 0x7f);
		})
	);
	map(0x0e0000, 0x0fffff).rom().region("bios", 0);
	map(0xfe0000, 0xffffff).rom().region("bios", 0);
}

void champ2_state::main_io(address_map &map)
{
	map.global_mask(0x3ff);
	map(0x0000, 0x00ff).m(m_mb, FUNC(at_mb_device::map));
	map(0x0200, 0x0201).portr("GAME");
}


void champ2_state::bank_map(address_map &map)
{
	map(0x00000, 0xfffff).rom().region("game_prg", 0);
}

static INPUT_PORTS_START( champ2 )
	PORT_START("GAME")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, "GAME" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	// punts with "terminated - illegal" if any of these two are low
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END

void champ2_state::machine_start()
{
}

void champ2_state::machine_reset()
{
	m_bank[0]->set_bank(0);
	m_bank[1]->set_bank(0);
}


void champ2_state::champ2(machine_config &config)
{
	I386SX(config, m_maincpu, 16'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &champ2_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &champ2_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));
//  m_maincpu->shutdown_callback().set("mb", FUNC(at_mb_device::shutdown));

	AT_MB(config, m_mb).at_softlists(config);
	m_mb->kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	pc_kbdc.out_clock_cb().set(m_mb, FUNC(at_mb_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set(m_mb, FUNC(at_mb_device::kbd_data_w));

	// FIXME: determine ISA bus clock
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "vga", false);
	ISA16_SLOT(config, "isa2", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa6", 0, "mb:isabus", pc_isa16_cards, nullptr, false);

	RAM(config, m_ram).set_default_size("15M").set_extra_options("640K,1024K,1664K,2M,4M,8M,15M");

	for (auto bank : m_bank)
		ADDRESS_MAP_BANK(config, bank).set_map(&champ2_state::bank_map).set_options(ENDIANNESS_LITTLE, 8, 20, 0x2000);
}


ROM_START( champ2 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// borrowed from pc/at.cpp pt319a
	ROM_LOAD( "3sam001.bin", 0x10000, 0x10000, BAD_DUMP CRC(cad22030) SHA1(85bb6027579a87bfe7ea0f7df3676fdaa64920ac))

	ROM_REGION( 0x100000, "game_prg", 0 )
	ROM_LOAD( "champ2.u2", 0x00000, 0x80000, CRC(058bd1a4) SHA1(e4c0db329cda0cdcab7c7b4d130f1c38fa32385f) )
	ROM_LOAD( "champ2.u3", 0x80000, 0x80000, CRC(3bb1951f) SHA1(8057327285f57787cc7da678427767ee7f979a64) )

	ROM_REGION( 0x1caf, "pal", 0 )
	ROM_LOAD( "champ2.u7",  0x00000, 0x1caf, BAD_DUMP CRC(bd70e89b) SHA1(3567d22057e366a439ffce2dd35180f5df80d47c) )
ROM_END

void champ2_state::init_at_common(int xmsbase)
{
	address_space& space = m_maincpu->space(AS_PROGRAM);

	/* managed RAM */
	membank("bank10")->set_base(m_ram->pointer());

	if (m_ram->size() > xmsbase)
	{
		offs_t ram_limit = 0x100000 + m_ram->size() - xmsbase;
		space.install_ram(0x100000,  ram_limit - 1, m_ram->pointer() + xmsbase);
	}
}

void champ2_state::init_at()
{
	init_at_common(0xa0000);
}

} // anonymous namespace


//GAME( 1993, champ,  0, champ2,  champ2, champ2_state, empty_init, ROT0,  "U.S. Games", "Countertop Champion", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
// Title assumed by "Champ II" ROM labels
GAME( 1994, champ2,  0, champ2,  champ2, champ2_state, init_at, ROT0,  "U.S. Games", "Countertop Champion 2 (ver 2.11)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
