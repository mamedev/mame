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

READ16_MEMBER(megadriv_radica_state::read)
{
	return m_rom[(((m_bank * 0x10000) + (offset << 1)) & (0x400000 - 1))/2];
}

READ16_MEMBER(megadriv_radica_state::read_a13)
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


MACHINE_START_MEMBER(megadriv_radica_state, megadriv_radica_6button)
{
	MACHINE_START_CALL_MEMBER(megadriv);
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

MACHINE_START_MEMBER(megadriv_radica_state, megadriv_radica_3button)
{
	MACHINE_START_CALL_MEMBER(megadriv);
	m_vdp->stop_timers();
	save_item(NAME(m_bank));
}

MACHINE_RESET_MEMBER(megadriv_radica_state, megadriv_radica)
{
	m_bank = 0;
	MACHINE_RESET_CALL_MEMBER(megadriv);
	m_maincpu->reset();
}

void megadriv_radica_state::megadriv_radica_3button_ntsc(machine_config &config)
{
	md_ntsc(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &megadriv_radica_state::megadriv_radica_map);
	MCFG_MACHINE_START_OVERRIDE(megadriv_radica_state, megadriv_radica_3button)
	MCFG_MACHINE_RESET_OVERRIDE(megadriv_radica_state, megadriv_radica)
}

void megadriv_radica_state::megadriv_radica_3button_pal(machine_config &config)
{
	md_pal(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &megadriv_radica_state::megadriv_radica_map);
	MCFG_MACHINE_START_OVERRIDE(megadriv_radica_state, megadriv_radica_3button)
	MCFG_MACHINE_RESET_OVERRIDE(megadriv_radica_state, megadriv_radica)
}

void megadriv_radica_state::megadriv_radica_6button_pal(machine_config &config)
{
	md_pal(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &megadriv_radica_state::megadriv_radica_map);
	MCFG_MACHINE_START_OVERRIDE(megadriv_radica_state, megadriv_radica_6button)
	MCFG_MACHINE_RESET_OVERRIDE(megadriv_radica_state, megadriv_radica)
}




ROM_START( rad_sf )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radicasf.bin", 0x000000, 0x400000,  CRC(868afb44) SHA1(f4339e36272c18b1d49aa4095127ed18e0961df6) )
ROM_END

ROM_START( rad_gen1 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radicav1.bin", 0x000000, 0x400000,  CRC(3b4c8438) SHA1(5ed9c053f9ebc8d4bf571d57e562cf347585d158) )
ROM_END

ROM_START( rad_ssoc )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "sensiblesoccer.bin", 0x000000, 0x400000,  CRC(b8745ab3) SHA1(0ab3f26e5ffd288e5a3a5db676951b9095299eb0) ) // should be byteswapped?
ROM_END

void megadriv_radica_state::init_megadriv_radica_6button_pal()
{
	init_megadrie();
	// 6 button game, so overwrite 3 button io handlers
	m_megadrive_io_read_data_port_ptr = read8_delegate(FUNC(md_base_state::megadrive_io_read_data_port_6button),this);
	m_megadrive_io_write_data_port_ptr = write16_delegate(FUNC(md_base_state::megadrive_io_write_data_port_6button),this);
}

// NTSC releases
CONS( 2004, rad_gen1, 0, 0, megadriv_radica_3button_ntsc, megadriv_radica_3button_1player, megadriv_radica_state, init_megadriv,                    "Radica / Sega",                     "Genesis Collection Volume 1 (Radica, Arcade Legends) (USA)", 0)

// PAL releases
CONS( 2004, rad_sf,   0, 0, megadriv_radica_6button_pal,  megadriv_radica_6button,         megadriv_radica_state, init_megadriv_radica_6button_pal, "Radica / Capcom / Sega",            "Street Fighter II: Special Champion Edition [Ghouls'n Ghosts] (Radica, Arcade Legends) (Europe)", 0) // SF2 game is region locked,  US version ROM is definitely different
CONS( 2004, rad_ssoc, 0, 0, megadriv_radica_3button_pal,  megadriv_radica_3button,         megadriv_radica_state, init_megadrie,                    "Radica / Sensible Software / Sega", "Sensible Soccer plus [Cannon Fodder, Mega lo Mania] (Radica, Arcade Legends) (Europe)", 0)  // still branded as Arcade Legends even if none of these were ever arcade games

