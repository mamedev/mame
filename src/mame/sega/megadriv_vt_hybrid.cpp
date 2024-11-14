// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    This is an enhanced 'Mega Drive / Genesis on a Chip' combined with a VT02/VT03 system for the 'Bonus Games' menu

    The menu for the MD side of things doesn't work as it needs enhanced chipset emulation?
    at the moment it just boots the game in the lowest ROM bank (Flicky)

    TODO:
    hook up the VT side again

*/

#include "emu.h"
#include "megadriv.h"

namespace {

class megadriv_vt0203_state : public md_ctrl_state
{
public:
	megadriv_vt0203_state(const machine_config &mconfig, device_type type, const char *tag) :
		// Mega Drive part
		md_ctrl_state(mconfig, type, tag),
		m_md_is_running(true),
		m_bank(0),
		m_rom(*this, "maincpu")
	{ }

	// Mega Drive part
	uint16_t read(offs_t offset);
	void megadriv_vt0203_pal(machine_config &config);

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

	void megadriv_vt0203_map(address_map &map) ATTR_COLD;
};


// todo, use actual MD map, easier once maps are part of base class.
void megadriv_vt0203_state::megadriv_vt0203_map(address_map &map)
{
	megadriv_68k_base_map(map);

	map(0x000000, 0x3fffff).r(FUNC(megadriv_vt0203_state::read)); /* Cartridge Program Rom */
}

uint16_t megadriv_vt0203_state::read(offs_t offset)
{
	return m_rom[(((m_bank * 0x10000) + (offset << 1)) & (0x400000 - 1))/2];
}


// controller is wired directly into unit, no controller slots
static INPUT_PORTS_START( megadriv_vt0203 )
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD1")
#if 0
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
#else
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
#endif

	PORT_MODIFY("PAD2")
#if 0
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
##else
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
#endif
INPUT_PORTS_END


void megadriv_vt0203_state::machine_start()
{
	logerror("megadriv_vt0203_state::machine_start\n");
	md_ctrl_state::machine_start();

	m_vdp->stop_timers();
	save_item(NAME(m_bank));
}

void megadriv_vt0203_state::machine_reset()
{
	logerror("megadriv_vt0203_state::machine_reset\n");
	md_ctrl_state::machine_reset();
}

uint32_t megadriv_vt0203_state::screen_update_hybrid(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_md_is_running)
	{
		/* Copies a bitmap */
		return screen_update_megadriv(screen, bitmap, cliprect);
	}

	return 0;
}

void megadriv_vt0203_state::screen_vblank_hybrid(int state)
{
	if (m_md_is_running)
	{
		/* Used to Sync the timing */
		md_ctrl_state::screen_vblank_megadriv(state);
	}
}


void megadriv_vt0203_state::megadriv_vt0203_pal(machine_config &config)
{
	md_pal(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &megadriv_vt0203_state::megadriv_vt0203_map);

	m_screen->set_screen_update(FUNC(megadriv_vt0203_state::screen_update_hybrid));
	m_screen->screen_vblank().set(FUNC(megadriv_vt0203_state::screen_vblank_hybrid));

	ctrl1_3button(config);
	ctrl2_3button(config);

	// TODO: add the VT part, this might require refactoring of the VT stuff as the SoC currently contains the screen
	//       but instead we'll need to use a shared screen that is reconfigured depending on which part is enabled
}

} // anonymous namespace


ROM_START( sarc110 )
	ROM_REGION( 0x1000000, "maincpu", 0 ) // Mega Drive part
	ROM_LOAD16_WORD_SWAP( "superarcade.bin", 0x000000, 0x1000000, CRC(be732867) SHA1(3857b2fbddd6a548c81caf64122e47a0df079be5) )

	ROM_REGION( 0x400000, "mainrom", 0 ) // VT02/03 part
	ROM_LOAD( "ic1.prg", 0x00000, 0x400000, CRC(de76f71f) SHA1(ff6b37a76c6463af7ae901918fc008b4a2863951) )
ROM_END

ROM_START( sarc110a )
	ROM_REGION( 0x1000000, "maincpu", 0 ) // Mega Drive part
	ROM_LOAD16_WORD_SWAP( "superarcade.bin", 0x000000, 0x1000000, CRC(be732867) SHA1(3857b2fbddd6a548c81caf64122e47a0df079be5) )

	ROM_REGION( 0x400000, "mainrom", 0 ) // VT02/03 part
	ROM_LOAD( "ic1_ver2.prg", 0x00000, 0x400000, CRC(b97a0dc7) SHA1(bace32d73184df914113de5336e29a7a6f4c03fa) )
ROM_END


CONS( 200?, sarc110, 0,       0, megadriv_vt0203_pal,  megadriv_vt0203, megadriv_vt0203_state, init_megadrie, "<unknown>",                     "Super Arcade 101-in-1 (set 1)", MACHINE_NOT_WORKING)
CONS( 200?, sarc110a,sarc110, 0, megadriv_vt0203_pal,  megadriv_vt0203, megadriv_vt0203_state, init_megadrie, "<unknown>",                     "Super Arcade 101-in-1 (set 2)", MACHINE_NOT_WORKING)
