// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    This is an enhanced 'Mega Drive / Genesis on a Chip' combined with a SunPlus SPG2xx system for the 'Bonus Games' menu

    It can take MegaDrive cartridges

    The MD side of things doesn't work as it needs enhanced chipset emulation?

    TODO:
    hook up the SunPlus side again (see spg2xx_zone.cpp for hookup)

*/

#include "emu.h"
#include "megadriv.h"

namespace {

class megadriv_sunplus_state : public md_ctrl_state
{
public:
	megadriv_sunplus_state(const machine_config &mconfig, device_type type, const char *tag) :
		// Mega Drive part
		md_ctrl_state(mconfig, type, tag),
		m_md_is_running(true),
		m_bank(0),
		m_rom(*this, "maincpu")
	{ }

	// Mega Drive part
	uint16_t read(offs_t offset);
	void megadriv_sunplus_pal(machine_config &config);

	void init_reactmd();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	bool m_md_is_running;

	// Mega Drive part
	int m_bank;
	required_region_ptr<uint16_t> m_rom;

	uint32_t screen_update_hybrid(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank_hybrid(int state);

	void megadriv_sunplus_map(address_map &map) ATTR_COLD;
};


// todo, use actual MD map, easier once maps are part of base class.
void megadriv_sunplus_state::megadriv_sunplus_map(address_map &map)
{
	megadriv_68k_base_map(map);

	map(0x000000, 0x3fffff).r(FUNC(megadriv_sunplus_state::read)); /* Cartridge Program Rom */
}

uint16_t megadriv_sunplus_state::read(offs_t offset)
{
	return m_rom[(((m_bank * 0x10000) + (offset << 1)) & (0x400000 - 1))/2];
}


// controller is wired directly into unit, no controller slots
static INPUT_PORTS_START( megadriv_sunplus )
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD1")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	//PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PAD2")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	//PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void megadriv_sunplus_state::machine_start()
{
	logerror("megadriv_sunplus_state::machine_start\n");
	md_ctrl_state::machine_start();

	m_vdp->stop_timers();
	save_item(NAME(m_bank));
}

void megadriv_sunplus_state::machine_reset()
{
	logerror("megadriv_sunplus_state::machine_reset\n");
	md_ctrl_state::machine_reset();
}

uint32_t megadriv_sunplus_state::screen_update_hybrid(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_md_is_running)
	{
		/* Copies a bitmap */
		return screen_update_megadriv(screen, bitmap, cliprect);
	}

	return 0;
}

void megadriv_sunplus_state::screen_vblank_hybrid(int state)
{
	if (m_md_is_running)
	{
		/* Used to Sync the timing */
		md_ctrl_state::screen_vblank_megadriv(state);
	}
}


void megadriv_sunplus_state::megadriv_sunplus_pal(machine_config &config)
{
	md_pal(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &megadriv_sunplus_state::megadriv_sunplus_map);

	m_screen->set_screen_update(FUNC(megadriv_sunplus_state::screen_update_hybrid));
	m_screen->screen_vblank().set(FUNC(megadriv_sunplus_state::screen_vblank_hybrid));

	ctrl1_3button(config);
	ctrl2_3button(config);
}


void megadriv_sunplus_state::init_reactmd()
{
	uint16_t *ROM = (uint16_t*)memregion("sunplus")->base();
	int size = memregion("sunplus")->bytes();

	for (int i = 0; i < size/2; i++)
	{
		ROM[i] = bitswap<16>(ROM[i], 15, 13, 14, 12,  7,  6,  5,  4,
									 11, 10, 9,  8,   3,  1,  2,  0);

		ROM[i] = ROM[i] ^ 0xa5a5;
	}

	init_megadrie();
}


ROM_START( reactmd )
	ROM_REGION( 0x2000000, "maincpu", ROMREGION_ERASE00 ) // this contains the MD games and main boot menu
	ROM_LOAD16_WORD_SWAP( "reactormd.bin", 0x0000, 0x2000000, CRC(fe9664a4) SHA1(d475b524f576c9d1d90aed20c7467cc652396baf) )

	ROM_REGION( 0x4000000, "sunplus", ROMREGION_ERASE00 ) // this contains the SunPlus games
	ROM_LOAD16_WORD_SWAP( "reactor_md_sunplus-full.bin", 0x0000, 0x4000000, CRC(843aa58c) SHA1(07cdc6d4aa0057939c145ece01a9aca73c7f1f2b) )
	ROM_IGNORE(0x4000000) // the 2nd half of the ROM can't be accessed by the PCB (address line tied low) (contains garbage? data)
ROM_END

} // anonymous namespace


// Two systems in one unit - Genesis on a Chip and SunPlus, only the SunPlus part is currently emulated.  Genesis on a chip is a very poor implementation with many issues on real hardware.
// This should actually boot to a menu on the MD side, with the SunPlus only being enabled if selected from that menu.  MD side menu runs in some enhanced / custom MD mode though.
// Badminton hangs, as it does in the 49-in-1 above
CONS( 2009, reactmd,  0, 0, megadriv_sunplus_pal, megadriv_sunplus, megadriv_sunplus_state, init_reactmd, "AtGames / Sega / Waixing", "Reactor MD (PAL)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
