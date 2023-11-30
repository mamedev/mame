// license:BSD-3-Clause
// copyright-holders:David Haywood

/* Based on the "POEMS ES1 Flash ROM Writer Version 0.24 (C)2003-2004 HUDSON SOFT" string
   found in ROM this is assumed to be 'POEMS' hardware.

   https://game.watch.impress.co.jp/docs/20041209/toy166.htm
   https://forum.beyond3d.com/threads/hudson-softs-32-bit-cpu-poems-for-new-system.14358/
   https://web.archive.org/web/20021207035427/http://www.tensilica.com/html/pr_2002_10_15.html

   The above links mention Konami using this hardware for a PLAY-POEMS plug and play sports
   devices, and indicate it is based around the Xtensa instruction set, which has been confirmed
   for the single dumped device.

   https://0x04.net/~mwk/doc/xtensa.pdf

   Known PLAY-POEMS devices (all from Konami)

   2004/11/11   熱血パワプロチャンプ                                  (Baseball game)
   2004/11/11   爽快ゴルフチャンプ                                       (Golf game)
   2004/12/09   絶体絶命でんぢゃらすじーさん ミニゲームで対決じゃっ!     (Mini-Game Collection)
   2005/09/15   マリンバ天国                                          (Marimba Tengoku)
   2005/11/17   絶体絶命でんぢゃらすじーさん パーティーじゃっ!全員集合!!  (Mini-Game Collection)
   2005/11/24   ぐ〜チョコランタン スプーだいすき!プレイマット                (Kid's Floor Mat)

 */

#include "emu.h"

#include "cpu/xtensa/xtensa.h"

#include "screen.h"
#include "speaker.h"


namespace {

class hudson_poems_state : public driver_device
{
public:
	hudson_poems_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void hudson_poems(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};

void hudson_poems_state::machine_start()
{
}

void hudson_poems_state::machine_reset()
{
	m_maincpu->set_pc(0x20010058);
}

static INPUT_PORTS_START( hudson_poems )
INPUT_PORTS_END


uint32_t hudson_poems_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void hudson_poems_state::mem_map(address_map &map)
{
	map(0x20000000, 0x207fffff).rom().region("maincpu", 0);
	map(0x2c000000, 0x2c7fffff).ram();
}

void hudson_poems_state::hudson_poems(machine_config &config)
{
	// 27Mhz XTAL

	// Xtensa based CPU?
	XTENSA(config, m_maincpu, 27_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &hudson_poems_state::mem_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(320, 240); // resolution not confirmed
	screen.set_visarea(0, 320-1, 0, 240-1);
	screen.set_screen_update(FUNC(hudson_poems_state::screen_update));

	SPEAKER(config, "speaker").front_center();
}

ROM_START( marimba )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD( "marimbatengoku.u2", 0x000000, 0x800000, CRC(b2ac0c5b) SHA1(48f3cdf399b032d86234125eeac3fb1cdc73538a) ) // glob with TSOP pads

	ROM_REGION( 0x400, "nv", 0 )
	ROM_LOAD( "at24c08a.u4", 0x000000, 0x400, CRC(e128a679) SHA1(73fb551d87ed911bd469899343fd36d9d579af39) )
ROM_END

} // anonymous namespace


CONS( 2005, marimba,      0,       0,      hudson_poems, hudson_poems, hudson_poems_state, empty_init, "Konami", "Marimba Tengoku (Japan)", MACHINE_IS_SKELETON )
