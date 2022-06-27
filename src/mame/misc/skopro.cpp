// license:BSD-3-Clause
// copyright-holders:

/*
    Skonec SkoPro v1.0 PC hardware

    Intel Pentium E2160 1M Cache, 1.80 GHz, 800 MHz FSB
    Biostar MG31-M7 TE motherboard
    1GB RAM
    nVidia GeForce 8400GS
    Integrated sound?
    Hitachi Deskstar HDS721680PLA320 80 GB
    EGIS2JVS V1.2 card
    Windows XP embedded

    The system was meant to be modular, but only 2 titles are said to have been actually released.

    Announced titles:
    Douga De Puzzle - Idol Paradise
    Dragon Dance
    Exception
    Otenami Haiken Ritaanzu!
    Shangai
*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "screen.h"

class skopro_state : public driver_device
{
public:
	skopro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void skopro(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void skopro_map(address_map &map);
};

void skopro_state::video_start()
{
}

uint32_t skopro_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void skopro_state::skopro_map(address_map &map)
{
}

static INPUT_PORTS_START( skopro )
INPUT_PORTS_END


void skopro_state::machine_start()
{
}

void skopro_state::machine_reset()
{
}

void skopro_state::skopro(machine_config &config)
{
	/* basic machine hardware */
	PENTIUM4(config, m_maincpu, 100000000); // actually a Pentium E2160 at 1.80 GHz
	m_maincpu->set_addrmap(AS_PROGRAM, &skopro_state::skopro_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(skopro_state::screen_update));
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( drgdance)
	ROM_REGION(0x20000, "bios", 0) \
	ROM_LOAD("mbbios", 0x10000, 0x10000, NO_DUMP )


	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "dragon_dance", 0, SHA1(73868dd9354d936100ba56f460e872087ede012c) )
ROM_END


GAME( 2008, drgdance,  0,   skopro, skopro, skopro_state, empty_init, ROT0, "Success", "Dragon Dance (V1.02J)",  MACHINE_IS_SKELETON )
