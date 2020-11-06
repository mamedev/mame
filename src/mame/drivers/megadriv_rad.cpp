// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    Radica 'Mega Drive' and 'Genesis' clones
    these were mini battery operated "TV Game" consoles with wired in controller and no cartslot
    fully licensed by Sega

    reproduction 'System on a Chip' hardware, not perfect, flaws will need emulating eventually.

    not dumped

    Genesis Volume 2
    Genesis SF2 / GnG (PAL one is locked to PAL)

    Outrun 2019 (probably identical ROM to MD version, just custom controller)

    more?

*/

#include "emu.h"
#include "includes/megadriv.h"
#include "includes/megadriv_rad.h"

// todo, use actual MD map, easier once maps are part of base class.
void megadriv_radica_state::megadriv_radica_map(address_map &map)
{
	map(0x000000, 0x3fffff).r(FUNC(megadriv_radica_state::read)); /* Cartridge Program Rom */
	map(0xa00000, 0xa01fff).rw(FUNC(megadriv_radica_state::megadriv_68k_read_z80_ram), FUNC(megadriv_radica_state::megadriv_68k_write_z80_ram));
	map(0xa02000, 0xa03fff).w(FUNC(megadriv_radica_state::megadriv_68k_write_z80_ram));
	map(0xa04000, 0xa04003).rw(FUNC(megadriv_radica_state::megadriv_68k_YM2612_read), FUNC(megadriv_radica_state::megadriv_68k_YM2612_write));
	map(0xa06000, 0xa06001).w(FUNC(megadriv_radica_state::megadriv_68k_z80_bank_write));
	map(0xa10000, 0xa1001f).rw(FUNC(megadriv_radica_state::megadriv_68k_io_read), FUNC(megadriv_radica_state::megadriv_68k_io_write));
	map(0xa11100, 0xa11101).rw(FUNC(megadriv_radica_state::megadriv_68k_check_z80_bus), FUNC(megadriv_radica_state::megadriv_68k_req_z80_bus));
	map(0xa11200, 0xa11201).w(FUNC(megadriv_radica_state::megadriv_68k_req_z80_reset));
	map(0xa13000, 0xa130ff).r(FUNC(megadriv_radica_state::read_a13));
	map(0xc00000, 0xc0001f).rw(m_vdp, FUNC(sega315_5313_device::vdp_r), FUNC(sega315_5313_device::vdp_w));
	map(0xe00000, 0xe0ffff).ram().mirror(0x1f0000).share("megadrive_ram");
}

uint16_t megadriv_radica_state::read(offs_t offset)
{
	return m_rom[(((m_bank * 0x10000) + (offset << 1)) & (0x400000 - 1))/2];
}

uint16_t megadriv_radica_state::read_a13(offs_t offset)
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
static INPUT_PORTS_START( megadriv_radica_3button )
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD1")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("PAD2")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END

// the 6-in-1 and Sonic Gold units really only have a single wired controller, and no way to connect a 2nd one, despite having some 2 player games!
static INPUT_PORTS_START( megadriv_radica_3button_1player )
	PORT_INCLUDE( megadriv_radica_3button )

	PORT_MODIFY("PAD2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( megadriv_radica_6button )
	PORT_INCLUDE( megadriv_radica_3button )

	PORT_START("EXTRA1")    /* Extra buttons for Joypad 1 (6 button + start + mode) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRA2")    /* Extra buttons for Joypad 2 (6 button + start + mode) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0")
	PORT_START("UNK")
INPUT_PORTS_END

static INPUT_PORTS_START( megadriv_msi_6button )
	PORT_INCLUDE( megadriv_radica_3button )

	PORT_START("EXTRA1")    /* Extra buttons for Joypad 1 (6 button + start + mode) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRA2") // no 2nd pad
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PAD2") // no 2nd pad
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0")
	PORT_START("UNK")
INPUT_PORTS_END

void megadriv_radica_6button_state::machine_start()
{
	md_base_state::machine_start();
	m_vdp->stop_timers();

	m_io_pad_6b[0] = ioport("EXTRA1");
	m_io_pad_6b[1] = ioport("EXTRA2");
	m_io_pad_6b[2] = ioport("IN0");
	m_io_pad_6b[3] = ioport("UNK");

	// setup timers for 6 button pads
	for (int i = 0; i < 3; i++)
		m_io_timeout[i] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(md_base_state::io_timeout_timer_callback),this), (void*)(uintptr_t)i);

	save_item(NAME(m_bank));
}

void megadriv_radica_3button_state::machine_start()
{
	md_base_state::machine_start();
	m_vdp->stop_timers();
	save_item(NAME(m_bank));
}

void megadriv_radica_3button_state::machine_reset()
{
	m_bank = 0;
	md_base_state::machine_reset();
}

void megadriv_radica_3button_state::megadriv_radica_3button_ntsc(machine_config &config)
{
	md_ntsc(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &megadriv_radica_state::megadriv_radica_map);
}

void megadriv_radica_3button_state::megadriv_radica_3button_pal(machine_config &config)
{
	md_pal(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &megadriv_radica_state::megadriv_radica_map);
}

void megadriv_radica_6button_state::megadriv_radica_6button_pal(machine_config &config)
{
	md_pal(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &megadriv_radica_state::megadriv_radica_map);
}

void megadriv_radica_6button_state::megadriv_radica_6button_ntsc(machine_config &config)
{
	md_ntsc(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &megadriv_radica_state::megadriv_radica_map);
}


ROM_START( rad_sf2 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_megadrive_streetfighter2_usa.bin", 0x000000, 0x400000, CRC(a4426df8) SHA1(091f2a95ebd091141de5bcb83562c6087708cb32) )
ROM_END

ROM_START( rad_sf2p )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_megadrive_streetfighter2_uk.bin", 0x000000, 0x400000,  CRC(868afb44) SHA1(f4339e36272c18b1d49aa4095127ed18e0961df6) )
ROM_END

ROM_START( rad_gen1 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_megadrive_vol1_blue_usa.bin", 0x000000, 0x400000,  CRC(3b4c8438) SHA1(5ed9c053f9ebc8d4bf571d57e562cf347585d158) )
ROM_END

ROM_START( rad_md1 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_megadrive_vol1_blue_europe.bin", 0x000000, 0x400000, CRC(85867db1) SHA1(ddc596e2e68dc872bc0679a2de7a295b4c6d6b8e) )
ROM_END

ROM_START( rad_gen2 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_genesis_vol2_red_usa.bin", 0x000000, 0x400000, CRC(7c1a0f0e) SHA1(a6441f75a4cd48f1563aeafdfbdde00202d4067c) )
ROM_END

ROM_START( rad_md2 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_megadrive_vol2_red_uk.bin", 0x000000, 0x400000, CRC(b68fd025) SHA1(b8f9c505653d6dd2b62840f078f828360faf8abc) )
ROM_END

ROM_START( rad_ssoc )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "radica_sensiblesoccer_uk.bin", 0x000000, 0x400000,  CRC(b8745ab3) SHA1(0ab3f26e5ffd288e5a3a5db676951b9095299eb0) ) // should be byteswapped?
ROM_END

ROM_START( rad_sonic )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_supersonicgold_usa.bin", 0x000000, 0x400000, CRC(853c9140) SHA1(cf70a9cdd3be4d8d1b6195698db3a941f4908791) )
ROM_END

ROM_START( rad_sonicp )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_supersonicgold_uk.bin", 0x000000, 0x400000, CRC(ed774018) SHA1(cc2f7183e128c947463e3a43a0184b835ea16db8) )
ROM_END

// once byteswapped this matches "outrun 2019 (usa) (beta).bin  megadriv:outr2019up Out Run 2019 (USA, Prototype)"
// this was dumped from a PAL/UK unit, so maybe that 'beta' is really an alt Euro release, or was simply dumped from one of these Radica units and mislabeled?
ROM_START( rad_orun )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "outrun.bin", 0x000000, 0x100000, CRC(4fd6d653) SHA1(57f0e4550ff883e4bb7857caef2c893c21f80b42) )
ROM_END

ROM_START( msi_sf2 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	// The first part of the ROM seems to be a boot ROM for the enhanced MD clone menus, even if it does nothing here
	// and is probably leftover from one of the multigame systems, hacked to only launch one game. We should emulate it...
	// .. but the game ROM starts at 0xc8000 so we can cheat for now
	ROM_LOAD16_WORD_SWAP( "29lv320.bin", 0x000000, 0xc8000, CRC(465b12f0) SHA1(7a058f6feb4f08f56ae0f7369c2ca9a9fe2ed40e) )
	ROM_CONTINUE(0x00000,0x338000) 
ROM_END


void megadriv_radica_6button_state::init_megadriv_radica_6button_pal()
{
	init_megadrie();
	// 6 button game, so overwrite 3 button io handlers
	m_megadrive_io_read_data_port_ptr = read8sm_delegate(*this, FUNC(md_base_state::megadrive_io_read_data_port_6button));
	m_megadrive_io_write_data_port_ptr = write16sm_delegate(*this, FUNC(md_base_state::megadrive_io_write_data_port_6button));
}

void megadriv_radica_6button_state::init_megadriv_radica_6button_ntsc()
{
	init_megadriv();
	// 6 button game, so overwrite 3 button io handlers
	m_megadrive_io_read_data_port_ptr = read8sm_delegate(*this, FUNC(md_base_state::megadrive_io_read_data_port_6button));
	m_megadrive_io_write_data_port_ptr = write16sm_delegate(*this, FUNC(md_base_state::megadrive_io_write_data_port_6button));
}

// US versions show 'Genesis' on the menu,    show a www.radicagames.com splash screen, and use NTSC versions of the ROMs, sometimes region locked
// EU versions show 'Mega Drive' on the menu, show a www.radicagames.com splash screen, and use PAL versions of the ROMs, sometimes region locked
// UK versions show "Mega Drive' on the menu, show a www.radicauk.com splash screen,    and use PAL versions of the ROMs, sometimes region locked


CONS( 2004, rad_gen1,  0,        0, megadriv_radica_3button_ntsc, megadriv_radica_3button_1player, megadriv_radica_3button_state, init_megadriv,                    "Radica / Sega",                     "Genesis Collection Volume 1 (Radica, Arcade Legends) (USA)", 0)
CONS( 2004, rad_md1,   rad_gen1, 0, megadriv_radica_3button_pal,  megadriv_radica_3button_1player, megadriv_radica_3button_state, init_megadrie,                    "Radica / Sega",                     "Mega Drive Collection Volume 1 (Radica, Arcade Legends) (Europe)", 0)
// A UK version exists, showing the Radica UK boot screen

CONS( 2004, rad_gen2,  0,        0, megadriv_radica_3button_ntsc, megadriv_radica_3button_1player, megadriv_radica_3button_state, init_megadriv,                    "Radica / Sega",                     "Genesis Collection Volume 2 (Radica, Arcade Legends) (USA)", 0)
CONS( 2004, rad_md2,   rad_gen2, 0, megadriv_radica_3button_pal,  megadriv_radica_3button_1player, megadriv_radica_3button_state, init_megadrie,                    "Radica / Sega",                     "Mega Drive Collection Volume 2 (Radica, Arcade Legends) (UK)", 0)
// is there a Europe version with Radica Games boot screen and Mega Drive text?

// box calls this Volume 3
CONS( 2004, rad_sonic, 0,        0, megadriv_radica_3button_ntsc, megadriv_radica_3button_1player, megadriv_radica_3button_state, init_megadriv,                    "Radica / Sega",                     "Super Sonic Gold (Radica Plug & Play) (USA)", 0)
CONS( 2004, rad_sonicp,rad_sonic,0, megadriv_radica_3button_pal,  megadriv_radica_3button_1player, megadriv_radica_3button_state, init_megadrie,                    "Radica / Sega",                     "Super Sonic Gold (Radica Plug & Play) (UK)", 0)
// is there a Europe version with Radica Games boot screen and Mega Drive text?

CONS( 2004, rad_sf2,   0,        0, megadriv_radica_6button_ntsc, megadriv_radica_6button,         megadriv_radica_6button_state, init_megadriv_radica_6button_ntsc,"Radica / Capcom / Sega",            "Street Fighter II: Special Champion Edition [Ghouls'n Ghosts] (Radica, Arcade Legends) (USA)", 0)
CONS( 2004, rad_sf2p,  rad_sf2,  0, megadriv_radica_6button_pal,  megadriv_radica_6button,         megadriv_radica_6button_state, init_megadriv_radica_6button_pal, "Radica / Capcom / Sega",            "Street Fighter II: Special Champion Edition [Ghouls'n Ghosts] (Radica, Arcade Legends) (UK)", 0)
// is there a Europe version with Radica Games boot screen and Mega Drive text?

// still branded as Arcade Legends even if none of these were ever arcade games, European exclusive
CONS( 2004, rad_ssoc,  0,        0, megadriv_radica_3button_pal,  megadriv_radica_3button,         megadriv_radica_3button_state, init_megadrie,                    "Radica / Sensible Software / Sega", "Sensible Soccer plus [Cannon Fodder, Mega lo Mania] (Radica, Arcade Legends) (UK)", 0)
// is there a Europe version with Radica Games boot screen and Mega Drive text?

// not region locked, no Radica logos, uncertain if other regions would differ
CONS( 2004, rad_orun,  0,        0, megadriv_radica_3button_pal,  megadriv_radica_3button_1player, megadriv_radica_3button_state, init_megadrie,                    "Radica / Sega",                     "Out Run 2019 (Radica Plug & Play, UK)", 0)

// From a European unit but NTSC? - code is hacked from original USA Genesis game with region check still intact? (does the clone hardware always identify as such? or does the bypassed boot code skip the check?)
// TODO: move out of here eventually once the enhanced MD part is emulated rather than bypassed (it's probably the same as the 145-in-1 multigame unit, but modified to only include this single game)
CONS( 2018, msi_sf2,   0,        0, megadriv_radica_6button_ntsc, megadriv_msi_6button,         megadriv_radica_6button_state, init_megadriv_radica_6button_ntsc,    "MSI / Capcom / Sega",            "Street Fighter II: Special Champion Edition (MSI Plug & Play) (Europe)", 0)
