// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    Radica 'Mega Drive' and 'Genesis' clones
    these were mini battery operated "TV Game" consoles with wired in controller and no cartslot
    fully licensed by Sega

    reproduction 'System on a Chip' hardware, not perfect, flaws will need emulating eventually.

*/

#include "emu.h"
#include "megadriv.h"


namespace {

class megadriv_radica_state_base : public md_ctrl_state
{
public:
	megadriv_radica_state_base(const machine_config &mconfig, device_type type, const char *tag) :
		md_ctrl_state(mconfig, type, tag),
		m_bank(0),
		m_romsize(0x400000),
		m_rom(*this, "maincpu")
	{ }

protected:
	virtual uint16_t read(offs_t offset);
	uint16_t read_a13(offs_t offset);

	virtual void megadriv_radica_map(address_map &map) ATTR_COLD;

	void radica_base_map(address_map &map) ATTR_COLD;

	int m_bank;
	int m_romsize;

	required_region_ptr<uint16_t> m_rom;
};



class megadriv_radica_state : public megadriv_radica_state_base
{
public:
	megadriv_radica_state(const machine_config& mconfig, device_type type, const char* tag) :
		megadriv_radica_state_base(mconfig, type, tag)
	{ }

	void megadriv_radica_3button_ntsc(machine_config &config);
	void megadriv_radica_3button_pal(machine_config &config);

	void megadriv_radica_6button_ntsc(machine_config &config);
	void megadriv_radica_6button_pal(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
};


void megadriv_radica_state_base::radica_base_map(address_map &map)
{
	megadriv_68k_base_map(map);

	map(0x000000, 0x3fffff).r(FUNC(megadriv_radica_state_base::read)); // Cartridge Program ROM
}

void megadriv_radica_state_base::megadriv_radica_map(address_map &map)
{
	radica_base_map(map);

	map(0xa13000, 0xa130ff).r(FUNC(megadriv_radica_state_base::read_a13));
}

uint16_t megadriv_radica_state_base::read(offs_t offset)
{
	return m_rom[(((m_bank * 0x10000) + (offset << 1)) & (m_romsize - 1))/2];
}

uint16_t megadriv_radica_state_base::read_a13(offs_t offset)
{
	if (offset < 0x80)
		m_bank = offset & 0x3f;

	// low bit gets set when selecting cannon fodder or mega lo mania in the rad_ssoc set, pointing to the wrong area, but rad_gen1 needs it for the menu
	// as they're standalones it could just be different logic
	if (m_bank != 0x3f)
		m_bank &= 0x3e;

	return 0;
}

// controller is wired directly into unit, no controller slots
static INPUT_PORTS_START( radica_3button )
	PORT_INCLUDE( md_common )

	// TODO: how do the MENU buttons on the two controllers work?
INPUT_PORTS_END

// the 6-in-1 and Sonic Gold units really only have a single wired controller, and no way to connect a 2nd one, despite having some 2 player games!
static INPUT_PORTS_START( radica_3button_1player )
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD2")
	PORT_BIT( 0x0fff, IP_ACTIVE_LOW, IPT_UNUSED )

	// TODO: how does the MENU button on the controller work?
INPUT_PORTS_END

static INPUT_PORTS_START( radica_6button )
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD1") // Extra buttons for Joypad 1 (6 button + start + mode)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("%p Z")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("%p Y")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("%p X")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SELECT )  PORT_PLAYER(1) PORT_NAME("%p Mode")

	PORT_MODIFY("PAD2") // Extra buttons for Joypad 2 (6 button + start + mode)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("%p Z")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("%p Y")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("%p X")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SELECT )  PORT_PLAYER(2) PORT_NAME("%p Mode")
INPUT_PORTS_END


void megadriv_radica_state::machine_start()
{
	megadriv_radica_state_base::machine_start();

	m_vdp->stop_timers();

	save_item(NAME(m_bank));
}

void megadriv_radica_state::machine_reset()
{
	m_bank = 0;
	megadriv_radica_state_base::machine_reset();
}

void megadriv_radica_state::megadriv_radica_3button_ntsc(machine_config &config)
{
	md_ntsc(config);

	ctrl1_3button(config);
	ctrl2_3button(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &megadriv_radica_state::megadriv_radica_map);
}

void megadriv_radica_state::megadriv_radica_3button_pal(machine_config &config)
{
	md_pal(config);

	ctrl1_3button(config);
	ctrl2_3button(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &megadriv_radica_state::megadriv_radica_map);
}

void megadriv_radica_state::megadriv_radica_6button_pal(machine_config &config)
{
	megadriv_radica_3button_pal(config);

	ctrl1_6button(config);
	ctrl2_6button(config);
}

void megadriv_radica_state::megadriv_radica_6button_ntsc(machine_config &config)
{
	megadriv_radica_3button_ntsc(config);

	ctrl1_6button(config);
	ctrl2_6button(config);
}

ROM_START( rad_sf2 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_megadrive_streetfighter2_usa.bin", 0x000000, 0x400000, CRC(a4426df8) SHA1(091f2a95ebd091141de5bcb83562c6087708cb32) )
ROM_END

ROM_START( rad_sf2uk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_megadrive_streetfighter2_uk.bin", 0x000000, 0x400000,  CRC(868afb44) SHA1(f4339e36272c18b1d49aa4095127ed18e0961df6) )
ROM_END

ROM_START( mdtvp3j )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "playtv_vol3.bin", 0x000000, 0x400000,  CRC(d2daf376) SHA1(147b88d7aff834146c649077b43312c71b973298) )
ROM_END

ROM_START( rad_gen1 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_megadrive_vol1_blue_usa.bin", 0x000000, 0x400000,  CRC(3b4c8438) SHA1(5ed9c053f9ebc8d4bf571d57e562cf347585d158) )
ROM_END

ROM_START( rad_md1 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_megadrive_vol1_blue_europe.bin", 0x000000, 0x400000, CRC(85867db1) SHA1(ddc596e2e68dc872bc0679a2de7a295b4c6d6b8e) )
ROM_END

ROM_START( rad_md1uk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radicauk.u2", 0x000000, 0x400000, CRC(03a6734b) SHA1(255048d46b593bc975b3a6c44e8b8e35917511c7) )
ROM_END

ROM_START( mdtvp1j )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "l08y6_i_32m.u2", 0x000000, 0x400000, CRC(740a8859) SHA1(cf1212ef28e75e2cea752cf10a06ea715a30ae07) ) // 04-07-23 date sticker (23 July 2004)
ROM_END

ROM_START( rad_gen2 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_genesis_vol2_red_usa.bin", 0x000000, 0x400000, CRC(7c1a0f0e) SHA1(a6441f75a4cd48f1563aeafdfbdde00202d4067c) )
ROM_END

ROM_START( rad_md2uk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_megadrive_vol2_red_uk.bin", 0x000000, 0x400000, CRC(b68fd025) SHA1(b8f9c505653d6dd2b62840f078f828360faf8abc) )
ROM_END

ROM_START( mdtvp2j )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "playtv_vol2.bin", 0x000000, 0x400000, CRC(4d887d12) SHA1(b7f70abd12c3a3c68d1ad127a1475b704e898f51) )
ROM_END

ROM_START( rad_ssoc )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "radica_sensiblesoccer_uk.bin", 0x000000, 0x400000,  CRC(b8745ab3) SHA1(0ab3f26e5ffd288e5a3a5db676951b9095299eb0) ) // should be byteswapped?
ROM_END

ROM_START( rad_sonic )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_supersonicgold_usa.bin", 0x000000, 0x400000, CRC(853c9140) SHA1(cf70a9cdd3be4d8d1b6195698db3a941f4908791) )
ROM_END

ROM_START( rad_sonicuk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_supersonicgold_uk.bin", 0x000000, 0x400000, CRC(ed774018) SHA1(cc2f7183e128c947463e3a43a0184b835ea16db8) )
ROM_END

// once byteswapped this matches "outrun 2019 (usa) (beta).bin  megadriv:outr2019up Out Run 2019 (USA, Prototype)"
// this was dumped from a PAL/UK unit, so maybe that 'beta' is really an alt Euro release, or was simply dumped from one of these Radica units and mislabeled?
ROM_START( rad_orun )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "outrun.bin", 0x000000, 0x100000, CRC(4fd6d653) SHA1(57f0e4550ff883e4bb7857caef2c893c21f80b42) )
ROM_END

ROM_START( rad_mncr )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	// radica_menacer_byteswapped.bin = mpr-15075-f.u1        megadriv:menacer  Menacer 6-Game Cartridge (Europe, USA)
	ROM_LOAD16_WORD_SWAP( "radica_menacer.bin", 0x000000, 0x100000, CRC(5f9ef4a4) SHA1(f28350e7325cb7469d760d97ee452a9d846eb3d4) )
ROM_END

ROM_START( banmrid )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "b75909a.u10", 0x000000, 0x400000, CRC(b439e06e) SHA1(3a87fc16186b7042dd92c7cf68c4284cd86f9175) )
ROM_END


} // anonymous namespace


// US versions show 'Genesis' on the menu,    show a www.radicagames.com splash screen, and use NTSC versions of the ROMs, sometimes region locked
// EU versions show 'Mega Drive' on the menu, show a www.radicagames.com splash screen, and use PAL versions of the ROMs, sometimes region locked
// UK versions show "Mega Drive' on the menu, show a www.radicauk.com splash screen,    and use PAL versions of the ROMs, sometimes region locked

CONS( 2004, rad_gen1,  0,        0, megadriv_radica_3button_ntsc, radica_3button_1player, megadriv_radica_state, init_megadriv, "Radica / Sega",                     "Genesis Collection Volume 1 (Radica, Arcade Legends) (USA)", 0)
CONS( 2004, rad_md1,   rad_gen1, 0, megadriv_radica_3button_pal,  radica_3button_1player, megadriv_radica_state, init_megadrie, "Radica / Sega",                     "Mega Drive Collection Volume 1 (Radica, Arcade Legends) (Europe)", 0)
CONS( 2004, rad_md1uk, rad_gen1, 0, megadriv_radica_3button_pal,  radica_3button_1player, megadriv_radica_state, init_megadrie, "Radica / Sega",                     "Mega Drive Collection Volume 1 (Radica, Arcade Legends) (UK)", 0)
CONS( 2004, mdtvp1j,   rad_gen1, 0, megadriv_radica_3button_ntsc, radica_3button_1player, megadriv_radica_state, init_megadriv, "Sega Toys",                         "Mega Drive Play TV 1 (Japan)", 0) // expects US region despite being a Japanese unit (Bean Machine is region locked)

CONS( 2004, rad_gen2,  0,        0, megadriv_radica_3button_ntsc, radica_3button_1player, megadriv_radica_state, init_megadriv, "Radica / Sega",                     "Genesis Collection Volume 2 (Radica, Arcade Legends) (USA)", 0)
CONS( 2004, rad_md2uk, rad_gen2, 0, megadriv_radica_3button_pal,  radica_3button_1player, megadriv_radica_state, init_megadrie, "Radica / Sega",                     "Mega Drive Collection Volume 2 (Radica, Arcade Legends) (UK)", 0)
// is there a Europe version with Radica Games boot screen and Mega Drive text?
CONS( 2004, mdtvp2j,   rad_gen2, 0, megadriv_radica_3button_ntsc, radica_3button_1player, megadriv_radica_state, init_megadriv, "Sega Toys",                         "Mega Drive Play TV 2 (Japan)", 0)

// box calls this Volume 3
CONS( 2004, rad_sonic,  0,        0, megadriv_radica_3button_ntsc, radica_3button_1player, megadriv_radica_state, init_megadriv, "Radica / Sega",                     "Super Sonic Gold (Radica Plug & Play) (USA)", 0)
CONS( 2004, rad_sonicuk,rad_sonic,0, megadriv_radica_3button_pal,  radica_3button_1player, megadriv_radica_state, init_megadrie, "Radica / Sega",                     "Super Sonic Gold (Radica Plug & Play) (UK)", 0)
// is there a Europe version with Radica Games boot screen and Mega Drive text?

CONS( 2004, rad_sf2,   0,        0, megadriv_radica_6button_ntsc, radica_6button,         megadriv_radica_state, init_megadriv, "Radica / Capcom / Sega",            "Street Fighter II: Special Champion Edition [Ghouls'n Ghosts] (Radica, Arcade Legends) (USA)", 0)
CONS( 2004, rad_sf2uk, rad_sf2,  0, megadriv_radica_6button_pal,  radica_6button,         megadriv_radica_state, init_megadrie, "Radica / Capcom / Sega",            "Street Fighter II: Special Champion Edition [Ghouls'n Ghosts] (Radica, Arcade Legends) (UK)", 0)
// is there a Europe version with Radica Games boot screen and Mega Drive text?
CONS( 2004, mdtvp3j,   rad_sf2,  0, megadriv_radica_6button_ntsc, radica_6button,         megadriv_radica_state, init_megadriv, "Sega Toys",                         "Mega Drive Play TV 3 (Japan)", 0) // This one does contain the Japanese ROM for SF2 (but the World release of GnG) so SF2 runs in Japanese, but GnG runs in English

// still branded as Arcade Legends even if none of these were ever arcade games, European exclusive
CONS( 2004, rad_ssoc,  0,        0, megadriv_radica_3button_pal,  radica_3button,         megadriv_radica_state, init_megadrie, "Radica / Sensible Software / Sega", "Sensible Soccer plus [Cannon Fodder, Mega lo Mania] (Radica, Arcade Legends) (UK)", 0)
// is there a Europe version with Radica Games boot screen and Mega Drive text?

// not region locked, no Radica logos, uncertain if other regions would differ
CONS( 2004, rad_orun,  0,        0, megadriv_radica_3button_pal,  radica_3button_1player, megadriv_radica_state, init_megadrie, "Radica / Sega",                     "Out Run 2019 (Radica Plug & Play, UK)", 0)

// this has been verified as identical to the 6-in-1 cartridge that came with the Menacer gun for the MD
CONS( 2004, rad_mncr,  0,        0, megadriv_radica_3button_ntsc, radica_3button_1player, megadriv_radica_state, init_megadriv, "Radica / Sega",                     "Menacer (Radica Plug & Play)", MACHINE_NOT_WORKING )

// 仮面ライダー龍騎 サバイバルファイト
CONS( 2002, banmrid,    0,        0, megadriv_radica_3button_ntsc, radica_3button_1player, megadriv_radica_state, init_megadriv, "Bandai",                     "Kamen Rider Ryuki: Survival Fight (Japan)", MACHINE_NOT_WORKING )
