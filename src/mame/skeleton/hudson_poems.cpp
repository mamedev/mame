// license:BSD-3-Clause
// copyright-holders:David Haywood

/* Based on the "POEMS ES1 Flash ROM Writer Version 0.24 (C)2003-2004 HUDSON SOFT" string
   found in ROM this is assumed to be 'POEMS' hardware.

   https://game.watch.impress.co.jp/docs/20041209/toy166.htm
   https://forum.beyond3d.com/threads/hudson-softs-32-bit-cpu-poems-for-new-system.14358/
   https://web.archive.org/web/20021207035427/http://www.tensilica.com/html/pr_2002_10_15.html

   The above links mention Konami using this hardware for a PLAY-POEMS plug and play sports
   device, so it is likely Marimba Tengoku uses the same.

   this likely means this is using an Xtensa instruction set (needs verifiecation)

   https://0x04.net/~mwk/doc/xtensa.pdf
*/

#include "emu.h"

#include "screen.h"
#include "speaker.h"


namespace {

class hudsom_poems : public driver_device
{
public:
	hudsom_poems(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
	{ }

	void hudson_poems(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

void hudsom_poems::machine_start()
{
}

void hudsom_poems::machine_reset()
{
}

static INPUT_PORTS_START( hudson_poems )
INPUT_PORTS_END


uint32_t hudsom_poems::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void hudsom_poems::hudson_poems(machine_config &config)
{
	// 27Mhz XTAL
	
	// Xtensa based CPU?

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(320, 240); // resolution not confirmed
	screen.set_visarea(0, 320-1, 0, 240-1);
	screen.set_screen_update(FUNC(hudsom_poems::screen_update));

	SPEAKER(config, "speaker").front_center();
}

ROM_START( marimba )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD( "marimbatengoku.u2", 0x000000, 0x800000, CRC(b2ac0c5b) SHA1(48f3cdf399b032d86234125eeac3fb1cdc73538a) ) // glob with TSOP pads

	ROM_REGION( 0x400, "nv", 0 )
	ROM_LOAD( "at24c08a.u4", 0x000000, 0x400, CRC(e128a679) SHA1(73fb551d87ed911bd469899343fd36d9d579af39) )
ROM_END

} // anonymous namespace


CONS( 2005, marimba,      0,       0,      hudson_poems, hudson_poems, hudsom_poems, empty_init, "Konami", "Marimba Tengoku (Japan)", MACHINE_IS_SKELETON )
