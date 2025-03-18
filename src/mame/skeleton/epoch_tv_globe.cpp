// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

main SoC is marked

PONTO
PONTO-1
QHLL3.03
TAIWAN 0915

main PCB is marked

LCT REV1.9
20-K5410100G2
Tiger-Main
KTG-KZ003-08

is this related to koto_zevio.cpp, Koto Laboratory is credited for the hardware
development of both, and both are 3D capable Plug and Play SoCs

*******************************************************************************/

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "screen.h"
#include "speaker.h"


namespace {

class epoch_tv_globe_state : public driver_device
{
public:
	epoch_tv_globe_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void epoch_tv_globe(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void arm_map(address_map &map);
};

uint32_t epoch_tv_globe_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void epoch_tv_globe_state::machine_start()
{

}

void epoch_tv_globe_state::machine_reset()
{
}

static INPUT_PORTS_START( epoch_tv_globe )
INPUT_PORTS_END

void epoch_tv_globe_state::arm_map(address_map &map)
{
	map(0x00000000, 0x007fffff).rom().region("maincpu", 0);
}


void epoch_tv_globe_state::epoch_tv_globe(machine_config &config)
{
	ARM9(config, m_maincpu, 24000000 * 4); // unknown ARM core, unknown frequency (24Mhz XTAL)
	m_maincpu->set_addrmap(AS_PROGRAM, &epoch_tv_globe_state::arm_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(epoch_tv_globe_state::screen_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

ROM_START( eptvglob )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "a25l010.ic4", 0x000000, 0x20000, CRC(2e28c7a6) SHA1(ea26f8ccb9882e21f3d415af59fc04bdde36db6a) )

	// was also an ATMLH904 device at IC7, but it was empty

	ROM_REGION( 0x8400000, "nand", ROMREGION_ERASEFF )
	ROM_LOAD( "k9f1g08u0b.ic8", 0x000000, 0x8400000, CRC(f1880c56) SHA1(c50f01f799b3296cda56d05a02a59aa78e0c8422) )
ROM_END

ROM_START( digixar )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mr27t640.ic6", 0x000000, 0x800000, CRC(f593ac1b) SHA1(58cafab21d690de23b4781800c272bebf6b2b46f) )
ROM_END



} // anonymous namespace

CONS( 201?, eptvglob,       0,              0,      epoch_tv_globe, epoch_tv_globe, epoch_tv_globe_state, empty_init, "Epoch", "TV Globe (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 201?, digixar,        0,              0,      epoch_tv_globe, epoch_tv_globe, epoch_tv_globe_state, empty_init, "Bandai / Koto", "Digimon X Arena (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
