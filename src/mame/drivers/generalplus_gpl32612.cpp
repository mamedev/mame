// license:BSD-3-Clause
// copyright-holders:David Haywood


/*****************************************************************************

    unlike earlier SunPlus / GeneralPlus based SoCs this one is
    ARM based

	NAND types
	
	MX30LF1G08AA
	ID = C2F1
	Capacity = (2048+64) x 64 x 512

	Star Wars Blaster - MX30LF1G08AA 
	TMNT Hero Portal  - MX30LF1G08AA
	DC Hero Portal    - MX30LF1G08AA 

*****************************************************************************/


#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"

#include "screen.h"
#include "speaker.h"

class generalplus_gpl32612_game_state : public driver_device
{
public:
	generalplus_gpl32612_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void gpl32612(machine_config &config);

	void nand_init840();

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;

	required_device<screen_device> m_screen;

	uint32_t screen_update_gpl32612(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void nand_init(int blocksize, int blocksize_stripped);

	std::vector<uint8_t> m_strippedrom;
};

uint32_t generalplus_gpl32612_game_state::screen_update_gpl32612(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void generalplus_gpl32612_game_state::machine_start()
{
}

void generalplus_gpl32612_game_state::machine_reset()
{
}

static INPUT_PORTS_START( gpl32612 )
INPUT_PORTS_END


void generalplus_gpl32612_game_state::gpl32612(machine_config &config)
{
	ARM9(config, m_maincpu, 240000000); // unknown core / frequency, but ARM based

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(generalplus_gpl32612_game_state::screen_update_gpl32612));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

// NAND dumps, so there will be a bootloader / boot strap at least

ROM_START( jak_swbstrik )
	ROM_REGION( 0x8400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "starwarsblaster.bin", 0x000000, 0x8400000, CRC(02c3c4d6) SHA1(a6ae05a7d7b2015023113f6baad25458f3c01102) )
ROM_END

ROM_START( jak_tmnthp )
	ROM_REGION( 0x8400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "tmntheroportal.bin", 0x000000, 0x8400000, CRC(75ec7127) SHA1(cd05f55a1f5a7fd3d1b0658ad6805b8777857a7e) )
ROM_END

ROM_START( jak_dchp )
	ROM_REGION( 0x8400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "dcheroportal_mx30lf1g08aa_c2f1.bin", 0x000000, 0x8400000, CRC(576a3005) SHA1(6cd9edc4def707aede3f82a21c87269d2a6bc870) )
ROM_END

void generalplus_gpl32612_game_state::nand_init(int blocksize, int blocksize_stripped)
{
	uint8_t* rom = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();

	int numblocks = size / blocksize;

	m_strippedrom.resize(numblocks * blocksize_stripped);

	for (int i = 0; i < numblocks; i++)
	{
		const int base = i * blocksize;
		const int basestripped = i * blocksize_stripped;

		for (int j = 0; j < blocksize_stripped; j++)
		{
			m_strippedrom[basestripped + j] = rom[base + j];
		}
	}

	// debug to allow for easy use of unidasm.exe
	if (0)
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"stripped_%s", machine().system().name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(&m_strippedrom[0], blocksize_stripped * numblocks, 1, fp);
			fclose(fp);
		}
	}
}

void generalplus_gpl32612_game_state::nand_init840()
{
	nand_init(0x840, 0x800);
}


//    year, name,         parent,  compat, machine,      input,        class,              init,       company,  fullname,                             flags
CONS( 200?, jak_swbstrik,    0,       0,      gpl32612, gpl32612, generalplus_gpl32612_game_state, nand_init840, "JAKKS Pacific Inc", "Star Wars Blaster Strike", MACHINE_IS_SKELETON )
CONS( 200?, jak_tmnthp,      0,       0,      gpl32612, gpl32612, generalplus_gpl32612_game_state, nand_init840, "JAKKS Pacific Inc", "Teenage Mutant Ninja Turtles Hero Portal", MACHINE_IS_SKELETON )
// Hero Portal Dreamworks Dragons
// Hero Portal Power Rangers
CONS( 200?, jak_dchp,        0,       0,      gpl32612, gpl32612, generalplus_gpl32612_game_state, nand_init840, "JAKKS Pacific Inc", "DC Super Heroes The Watchtower Hero Portal", MACHINE_IS_SKELETON )
