// license:BSD-3-Clause
// copyright-holders:David Haywood

// Karaoke system for TV (uses standard Karaoke CDs)
// also has a basic display on the unit

// main(?) PCB has a SUNPLUS SPHE8104GW 'Automotive CD Audio Processor' (includes 32-bit CPU)
// sub(?) PCB has a chip with main details erased

#include "emu.h"

#include "screen.h"

namespace {

class eks515_state : public driver_device
{
public:
	eks515_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_screen(*this, "screen")
	{ }

	void eks515(machine_config &config) ATTR_COLD;

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<screen_device> m_screen;

	void prog_map(address_map &map) ATTR_COLD;
};

void eks515_state::machine_start()
{
}

void eks515_state::machine_reset()
{
}

static INPUT_PORTS_START( eks515 )
INPUT_PORTS_END

uint32_t eks515_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void eks515_state::eks515(machine_config &config)
{
	// unknown CPU types

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	m_screen->set_screen_update(FUNC(eks515_state::screen_update));
}

ROM_START( eks515 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "en25f80.u2", 0x000000, 0x100000, CRC(5e7495f3) SHA1(9ffa06bc5508ab2d91f0fb833102c41d1b1ecdc5) )

	ROM_REGION( 0x100000, "subcpu", ROMREGION_ERASEFF )
	ROM_LOAD( "en25t80.u7", 0x000000, 0x100000, CRC(2a37d746) SHA1(c5f4e0d800e03b359e255060bcedbd615262d0b6) )
ROM_END

} // anonymous namespace

// name and product code taken from sticker on back of unit
CONS( 201?, eks515, 0, 0, eks515, eks515, eks515_state, empty_init, "Easy Karaoke", "Karaoke Screen Party (EKS-515)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
