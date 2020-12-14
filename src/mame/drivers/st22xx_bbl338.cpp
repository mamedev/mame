// license:BSD-3-Clause
// copyright-holders:David Haywood, AJR

#include "emu.h"
#include "cpu/m6502/st2205u.h"
#include "screen.h"

class st22xx_bbl338_state : public driver_device
{
public:
	st22xx_bbl338_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{
	}
	
	void st22xx_bbl338(machine_config &config);

private:
	void st22xx_bbl338_map(address_map &map);

	required_device<st2xxx_device> m_maincpu;
	required_device<screen_device> m_screen;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

void st22xx_bbl338_state::st22xx_bbl338_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START(st22xx_bbl338)
INPUT_PORTS_END

u32 st22xx_bbl338_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void st22xx_bbl338_state::st22xx_bbl338(machine_config &config)
{
	ST2302U(config, m_maincpu, 12000000); // likely higher clock
	m_maincpu->set_addrmap(AS_DATA, &st22xx_bbl338_state::st22xx_bbl338_map);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(160, 128); // incorrect
	m_screen->set_visarea(0, 160 - 1, 0, 128 - 1);
	m_screen->set_screen_update(FUNC(st22xx_bbl338_state::screen_update));
}

ROM_START( bbl338 )
	// is internal ROM used? the code in the external ROM contains a bank for 4000-6fff including vectors at least.
	// it sets stack to 14f, but quickly jumps to 150 in RAM where there is no code? was something meant to have copied
	// code there earlier?

	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "en29lv160ab.u1", 0x000000, 0x200000, CRC(2c73e16c) SHA1(e2c69b3534e32ef384c0c2f5618118a419326e3a) )
ROM_END


ROM_START( dphh8213 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "mx29lv160cb.u1", 0x000000, 0x200000, CRC(c8e7e355) SHA1(726f28c2c9ab012a6842f9f30a0a71538741ba14) )
ROM_END




// this is uses a higher resolution display than the common units, but not as high as the SunPlus based ones
COMP( 201?, bbl338, 0,      0,      st22xx_bbl338, st22xx_bbl338, st22xx_bbl338_state, empty_init, "BaoBaoLong", "Portable Game Player BBL-338 (BaoBaoLong, 48-in-1)", MACHINE_IS_SKELETON )

// Chinese menus only, low resolution
COMP( 201?, dphh8213, 0,      0,      st22xx_bbl338, st22xx_bbl338, st22xx_bbl338_state, empty_init, "<unknown>", "Digital Pocket Hand Held System 20-in-1 - Model 8213 (China)", MACHINE_IS_SKELETON )
