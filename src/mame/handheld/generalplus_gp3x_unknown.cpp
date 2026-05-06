// license:BSD-3-Clause
// copyright-holders:David Haywood

// these are similar to the devices in generalplus_gpl162xx_lcdtype.cpp
// including the other HD 360 Degrees Rocker Palm Eyecare Console
// which has an identical case
// 
// they've been rebuilt for a different (unknown) tech level
// (but still unSP2.0 based, even if they have GP3x in the header)
// register use (in the tiny bit of code) looks similar to generalplus_gpce4.cpp
// so could be related to that, with most of the system code in internal ROM
//
// they probably have undumped internal ROM
//
// they show 2 different checksums (one for each external ROM) in the hidden test mode

#include "emu.h"

#include "cpu/unsp/unsp.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class generalplus_gp3x_state : public driver_device
{
public:
	generalplus_gp3x_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
	{ }

	void generalplus_gp3x(machine_config &config) ATTR_COLD;

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<unsp_12_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	void map(address_map &map) ATTR_COLD;
};

uint32_t generalplus_gp3x_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( generalplus_gp3x )
INPUT_PORTS_END

void generalplus_gp3x_state::map(address_map &map)
{
}

void generalplus_gp3x_state::machine_start()
{
}

void generalplus_gp3x_state::machine_reset()
{
}

void generalplus_gp3x_state::generalplus_gp3x(machine_config &config)
{
	UNSP_20(config, m_maincpu, 96000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &generalplus_gp3x_state::map);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(10));
	m_screen->set_size(320, 240);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(generalplus_gp3x_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x10000);
}

ROM_START( gp3x788 )
	ROM_REGION16_BE( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x18000, NO_DUMP ) // uncertain size

	ROM_REGION16_BE( 0x100000, "spi1", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "w25q81dv.u3", 0x000000, 0x100000, CRC(d7801e04) SHA1(4d3e9ffb9e5eb2d790267ec7c57aec975f35ee88) )

	ROM_REGION16_BE( 0x400000, "spi2", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "p25d32sh.u2", 0x000000, 0x400000, CRC(a90f18f1) SHA1(48fc2809750529b54db0e4033530c4eafb5033d4) )
ROM_END

ROM_START( 100in1rg )
	ROM_REGION16_BE( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x18000, NO_DUMP ) // uncertain size

	ROM_REGION16_BE( 0x100000, "spi1", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "by25q80awt.bin", 0x000000, 0x100000, CRC(18db16f2) SHA1(d87ad433b4c5552ae6f2e8dd9c7a8dc8b2027a23) )

	ROM_REGION16_BE( 0x400000, "spi2", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "p25d32.bin", 0x000000, 0x400000, CRC(25e81e82) SHA1(137e29b004bada3b53d1355929cbfc5491399976) )
ROM_END




} // anonymous namespace

CONS( 202?, gp3x788,      0,       0,      generalplus_gp3x,   generalplus_gp3x, generalplus_gp3x_state, empty_init, "Prince Electronics", "GM-1417 - HD 360 Degrees Rocker Palm Eyecare Console - 788 in 1", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 202?, 100in1rg,     0,       0,      generalplus_gp3x,   generalplus_gp3x, generalplus_gp3x_state, empty_init, "<unknown>", "100-in-1 Retro Gaming Console (SY-909)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
