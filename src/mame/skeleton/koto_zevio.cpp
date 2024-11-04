// license:BSD-3-Clause
// copyright-holders:David Haywood

// The Zevio SoC was developed by Koto Laboratory, the same company behind the Wonderswan

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "screen.h"
#include "speaker.h"


namespace {

class zevio_state : public driver_device
{
public:
	zevio_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void zevio(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint32_t z900b0014_r();
	uint32_t zb8000024_r();

	void arm_map(address_map &map) ATTR_COLD;
};

uint32_t zevio_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void zevio_state::machine_start()
{
}

void zevio_state::machine_reset()
{
}

uint32_t zevio_state::z900b0014_r()
{
	return machine().rand();
}

uint32_t zevio_state::zb8000024_r()
{
	return machine().rand();
}

static INPUT_PORTS_START( zevio )
INPUT_PORTS_END

void zevio_state::arm_map(address_map &map)
{
	map(0x00000000, 0x007fffff).rom().region("maincpu", 0);

	map(0x10000000, 0x10ffffff).ram();

	map(0x900a0f04, 0x900a0f07).nopw();
	map(0x900b0014, 0x900b0017).r(FUNC(zevio_state::z900b0014_r));

	map(0xb8000024, 0xb8000027).r(FUNC(zevio_state::zb8000024_r));

	map(0xb8000800, 0xb8000fff).ram();
}


void zevio_state::zevio(machine_config &config)
{
	ARM9(config, m_maincpu, 72000000); // unknown ARM core, unknown frequency
	m_maincpu->set_addrmap(AS_PROGRAM, &zevio_state::arm_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(zevio_state::screen_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}


// ドラゴンボールＺ スカウターバトル体感かめはめ波 おらとおめえとスカウター
ROM_START( dbzscout )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mr27t6402l.ic6", 0x000000, 0x800000, CRC(9cb896d6) SHA1(4185ee4593c2ef3b637f6004d1f80dadd4530902) )
ROM_END

ROM_START( dbzonep )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mr27t6402l.u1", 0x000000, 0x800000, CRC(57f7c319) SHA1(65118a9c61defc75cefe5e45062c0a4788e2a26c) )
	// original dump had the first 0x100 bytes repeated again at the end, why?

	ROM_REGION( 0x800, "eeprom", ROMREGION_ERASEFF )
	ROM_LOAD( "s24cs16a.u6", 0x000000, 0x800, CRC(a1724ea8) SHA1(93a6f73e30f47b6a0c83f62dfd9d8236473518a8) )
ROM_END

} // anonymous namespace

CONS( 2007, dbzscout,     0,              0,      zevio, zevio, zevio_state, empty_init, "Bandai / Koto", "Dragon Ball Z: Scouter Battle Taikan Kamehameha: Ora to Omee to Scouter (Japan)", MACHINE_IS_SKELETON )
CONS( 2008, dbzonep,      0,              0,      zevio, zevio, zevio_state, empty_init, "Bandai / Koto", "Dragon Ball Z x One Piece: Battle Taikan Gum-Gum no Kamehameha: Omee no Koe de Ora o Yobu (Japan)", MACHINE_IS_SKELETON )
