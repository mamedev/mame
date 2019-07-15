// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

System Board Y2

'YATA-2 ASIC : 32-bit RISC processor @ 266 MHz'

This SoC looks suspiciously like the ones used for some Dreamcast derivatives, suggesting that this too
could be a DC / Naomi based platform, but with added encryption etc.

The System Board Y2 was released by SI Electronics, LTD. in 2009, The hardware was developed after Kaga
Electronics had acquired SI Electronics from Sega Sammy in 2008.  SI Electronics was also responsible
for the Atomiswave manufacturing, again suggesting this could be DC based.

The rest of the specs are quite close to DC / Naomi too.

--

ROMs are contained on a small sub-board

*/

#include "emu.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class system_board_y2_state : public driver_device
{
public:
	system_board_y2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void system_board_y2(machine_config &config);

private:
	virtual void video_start() override;
	uint32_t screen_update_system_board_y2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

void system_board_y2_state::video_start()
{
}

uint32_t system_board_y2_state::screen_update_system_board_y2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( system_board_y2 )
INPUT_PORTS_END


void system_board_y2_state::system_board_y2(machine_config &config)
{
	/*
	SH4LE(config, m_maincpu, 266666666);
	m_maincpu->set_md(0, 1);
	m_maincpu->set_md(1, 0);
	m_maincpu->set_md(2, 1);
	m_maincpu->set_md(3, 0);
	m_maincpu->set_md(4, 0);
	m_maincpu->set_md(5, 1);
	m_maincpu->set_md(6, 0);
	m_maincpu->set_md(7, 1);
	m_maincpu->set_md(8, 0);
	m_maincpu->set_sh4_clock(CPU_CLOCK);
	*/

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(system_board_y2_state::screen_update_system_board_y2));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x1000);

	SPEAKER(config, "mono").front_center();
}

ROM_START( kof2002um ) // The King of Fighters 复仇之路/Fùchóu zhī lù/Road to Revenge
	ROM_REGION( 0x8000000, "boot", 0 ) // sound program only? or boot too?
	ROM_LOAD( "s29gl01gp11fcr2.u103", 0x0000000, 0x8000000, CRC(722cbad1) SHA1(0292be12255ee4bd586166a3f5cd108c5453295b) )

	ROM_REGION( 0x42000000, "nand_u101", 0 ) // presumably accessed like a filesystem (encrypted)
	ROM_LOAD( "nand08gw3b2cn6.u101", 0x00000000, 0x42000000, CRC(ddeebb49) SHA1(6907205a0e0b69e2b37528f71647c70b4dd9e0f2) )
	ROM_REGION( 0x42000000, "nand_u102", 0 )
	ROM_LOAD( "nand08gw3b2cn6.u102", 0x00000000, 0x42000000, CRC(ac2dc586) SHA1(5168b4c0c6343b6c040a206da04fa7cdbc3b35b9) )
ROM_END

ROM_START( kof2002umj )
	ROM_REGION( 0x8000000, "boot", 0 ) // sound program only? or boot too?
	ROM_LOAD( "s29gl01gp11fcr2.u103", 0x0000000, 0x8000000, CRC(916c9d68) SHA1(65c09f75b6a71b0d79a827c6829d1c05d8699a32) )

	ROM_REGION( 0x42000000, "nand_u101", 0 ) // presumably accessed like a filesystem (encrypted)
	ROM_LOAD( "nand08gw3b2cn6.u101", 0x00000000, 0x42000000, CRC(46e03f1d) SHA1(62d8eeb7513e851bf11a26a84b5d310270f3fcf6) )
	ROM_REGION( 0x42000000, "nand_u102", 0 )
	ROM_LOAD( "nand08gw3b2cn6.u102", 0x00000000, 0x42000000, CRC(db931dca) SHA1(1b1fc88732944e9ede09e584c7b07e28a59df3e2) )
ROM_END


/* The title screen shows "The King of Fighters - Road to Revenge" (Chinese / English) while the speech on the title screen announcer says "The King of Fighters 2002 Unlimited Match"
   There is a PS2 version with the Unlimited Match title screen, but unless it's used for a different region the arcade doesn't show that title, only announces it. */
GAME( 2009, kof2002um,  0,         system_board_y2, system_board_y2,  system_board_y2_state, empty_init, ROT0, "SNK Playmore / New Channel", "The King of Fighters - Fuchou Zhi Lu/Road to Revenge / The King of Fighters 2002 Unlimited Match (China)", MACHINE_IS_SKELETON ) // also Export?
GAME( 2009, kof2002umj, kof2002um, system_board_y2, system_board_y2,  system_board_y2_state, empty_init, ROT0, "SNK Playmore",               "The King of Fighters 2002 Unlimited Match (Japan)",                                                        MACHINE_IS_SKELETON )
