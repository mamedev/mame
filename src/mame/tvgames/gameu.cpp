// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "cpu/unsp/unsp.h"

#include "screen.h"
#include "speaker.h"


namespace {

class gameu_handheld_state : public driver_device
{
public:
	gameu_handheld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void gameu_handheld(machine_config &config);

	void init_gameu();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<unsp_20_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void map(address_map &map);
};

uint32_t gameu_handheld_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void gameu_handheld_state::machine_start()
{

}

void gameu_handheld_state::machine_reset()
{
}

static INPUT_PORTS_START( gameu_handheld )
INPUT_PORTS_END


void gameu_handheld_state::map(address_map &map)
{
	map(0x000000, 0x00ffff).rom().region("maincpu", 0x00000);
}


void gameu_handheld_state::gameu_handheld(machine_config &config)
{
	UNSP_20(config, m_maincpu, 96000000); // unknown type of SunPlus
	m_maincpu->set_addrmap(AS_PROGRAM, &gameu_handheld_state::map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262); // unknown resolution
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(gameu_handheld_state::screen_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

void gameu_handheld_state::init_gameu()
{
	uint16_t *ROM = (uint16_t*)memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();

	for (int i = 0; i < size/2; i++)
	{
		ROM[i] = ROM[i] ^ 0x3b90;

		ROM[i] = bitswap<16>(ROM[i], 8, 7, 13,  15,  4,  5,  12,  10,
									 3,  1,  11,  9,  6,  14,  0, 2);

		ROM[i] = ((ROM[i] & 0xff00) >> 8) | ((ROM[i] & 0x00ff) << 8);
	}

	#if 0
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"decrypted_%s", machine().system().name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(ROM, size, 1, fp);
			fclose(fp);
		}
	}
	#endif
}

ROM_START( gameu50 )
	ROM_REGION( 0x2000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "gameu.bin", 0x000000, 0x2000000, CRC(13c42bce) SHA1(f769ceabb8ab4e60c0d663dffd5cca91c6aec206) )
ROM_END

} // anonymous namespace

// unit looks a bit like a knock-off Wii-U tablet, but much smaller
CONS( 201?, gameu50,       0,              0,      gameu_handheld, gameu_handheld, gameu_handheld_state, init_gameu, "YSN", "Play Portable Color GameU+ (50-in-1) (Japan)", MACHINE_IS_SKELETON )
