// license:BSD-3-Clause
// copyright-holders:David Haywood
/* MPU4 'Plasma' addition */

/* the Plasma was an oversized DMD, but was rarely used, Big Chief might be the only game with it, at least it's the only dump we have? */
// http://www.youtube.com/watch?v=PAs8p48u0Jc
// code currently uses a 'screen' but it has been since decided this is not how to handle such display types in MAME.

#include "emu.h"
#include "mpu4.h"

#include "cpu/m68000/m68000.h"
#include "machine/z80scc.h"
#include "emupal.h"
#include "screen.h"

namespace {

class mpu4plasma_state : public mpu4_state
{
public:
	mpu4plasma_state(const machine_config &mconfig, device_type type, const char *tag)
		: mpu4_state(mconfig, type, tag)
		, m_plasmaram(*this, "plasmaram")
		, m_palette(*this, "palette")
	{
	}

	void mpu4plasma_f(machine_config &config);

	template<typename... T>
	auto mpu4plasma(T... traits)
	{
		return trait_wrapper(this, &mpu4plasma_state::mpu4plasma_f, traits...);
	}

private:
	required_shared_ptr<uint16_t> m_plasmaram;
	required_device<palette_device> m_palette;

	uint32_t screen_update_mpu4plasma(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mpu4plasma_map(address_map &map) ATTR_COLD;
};

#include "mpu4plasma.lh"

void mpu4plasma_state::mpu4plasma_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();

	// why does it test this much ram, just sloppy code expecting mirroring?
	map(0x400000, 0x4fffff).ram().share("plasmaram");
	// comms?
	map(0xffff00, 0xffff07).rw("scc", FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask16(0x00ff);
}

uint32_t mpu4plasma_state::screen_update_mpu4plasma(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// don't know if this really gets drawn straight from ram..
	int const base = 0x1600 / 2;

	uint16_t const *const rambase = m_plasmaram;

	int i = 0;

	for (int y=0; y<40; y++)
	{
		uint16_t *const dst_bitmap = &bitmap.pix(y);

		for (int x=0; x<128/16; x++)
		{
			uint16_t const pix = rambase[base+i];
			for (int p=0; p<16; p++)
				dst_bitmap[x*16 + p] = ((pix << p) & 0x8000) ? 1 : 0;

			i++;
		}
	}

	return 0;
}


void mpu4plasma_state::mpu4plasma_f(machine_config &config)
{
	mod2_f(config);
	m68000_device &plasmacpu(M68000(config, "plasmacpu", 10000000));
	plasmacpu.set_addrmap(AS_PROGRAM, &mpu4plasma_state::mpu4plasma_map);

	SCC8530N(config, "scc", 4915200).out_int_callback().set_inputline("plasmacpu", 4);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0, 128-1, 0*8, 40-1);
	screen.set_screen_update(FUNC(mpu4plasma_state::screen_update_mpu4plasma));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(0x200);

	MPU4_CHARACTERISER_PAL(config, m_characteriser, 0);
	m_characteriser->set_cpu_tag("maincpu");
	m_characteriser->set_allow_6809_cheat(true);
	m_characteriser->set_lamp_table(nullptr);
}

// plasma v0.1
#define M4BIGCHF_PLASMA \
	ROM_REGION( 0x40000, "plasmacpu", 0 ) \
	ROM_LOAD16_BYTE( "b6cpl.p0", 0x00000, 0x020000, CRC(7fbb2efb) SHA1(c21136bf10407f1685f3933d426ef53925aca8d8) ) \
	ROM_LOAD16_BYTE( "b6cpl.p1", 0x00001, 0x020000, CRC(a9f67f3e) SHA1(1309cc2dc8565ee79ac8cdc754187c8db6ddb3ea) )
ROM_START( m4bigchf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b6cs.p1", 0x0000, 0x010000, CRC(4f45086b) SHA1(e1d639b068951df8f25b9c77d4fb86336ad19933) )
	M4BIGCHF_PLASMA
ROM_END

ROM_START( m4bigchfa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bchf20-6", 0x0000, 0x010000, CRC(7940eb01) SHA1(b23537e91842a0d9b25b9c76b245d2be3d9af57f) )
	M4BIGCHF_PLASMA
ROM_END

ROM_START( m4bigchfb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b6cc.p1", 0x0000, 0x010000, CRC(8d3916b4) SHA1(1818137da9d53000053a8023c4994c6539459df0) )
	M4BIGCHF_PLASMA
ROM_END

ROM_START( m4bigchfc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "big chief 6.bin", 0x0000, 0x010000, CRC(edee08b7) SHA1(8de6160a4a4e5cd57f64c49d913f763aa87dc69a) )
	M4BIGCHF_PLASMA
ROM_END


// plasma v0.2
#define M4CLICK_PLASMA \
	ROM_REGION( 0x40000, "plasmacpu", 0 ) \
	ROM_LOAD16_BYTE( "clcpl.p0", 0x00000, 0x010000, CRC(b774c1f1) SHA1(2708a1e4c539d72d9ada8c37d3372b64f6edc4a0) ) \
	ROM_LOAD16_BYTE( "clcpl.p1", 0x00001, 0x010000, CRC(51cf5f53) SHA1(f7b14acb0fd831aa19fdaa1bb36272ac9910f0b3) )
ROM_START( m4click )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clcs.p1", 0x0000, 0x010000, CRC(99dcea3e) SHA1(2ec9842f7d920d449cfeac43f7fe79f8c62ecec9) )
	M4CLICK_PLASMA

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "clcr.p1", 0x0000, 0x010000, CRC(b0dd4b66) SHA1(fbbd2e5e6a9c498225b219da4f11dd2d3a6c3545) )
	ROM_LOAD( "cc8ad.p1", 0x0000, 0x010000, CRC(80c64bbb) SHA1(ff004106b8be204fe38af19ec751926b1f7ad8df) )
	ROM_LOAD( "cc8b.p1", 0x0000, 0x010000, CRC(ebc8e052) SHA1(5ac2df221a2d8d374511e0973253509bed4414d2) )
	ROM_LOAD( "cc8bcd.p1", 0x0000, 0x010000, CRC(8e4950bf) SHA1(7165acd61263c7a1cada3379811d5c4e109a1a3e) )
	ROM_LOAD( "cc8bd.p1", 0x0000, 0x010000, CRC(122b6277) SHA1(686053964aa5bd1d3323e0b3a0adf1212ac5a992) )
	ROM_LOAD( "cc8c.p1", 0x0000, 0x010000, CRC(bf70180f) SHA1(473f0bc56a3a1e940f448ce758021a51fcb29344) )
	ROM_LOAD( "cc8d.p1", 0x0000, 0x010000, CRC(80c64bbb) SHA1(ff004106b8be204fe38af19ec751926b1f7ad8df) )
	ROM_LOAD( "cc8dy.p1", 0x0000, 0x010000, CRC(bd3e009c) SHA1(fb07860d7e6bca5b5b12c12aa8624ebfaf223877) )
	ROM_LOAD( "cc8s.p1", 0x0000, 0x010000, CRC(2c04f991) SHA1(c54ddd571dd9484c23c9f36a87b52fa357f4fdf2) )
	ROM_LOAD( "cc8y.p1", 0x0000, 0x010000, CRC(5352b635) SHA1(76d789dd1a912fbe35b6410cee60178854be3d3a) )
	ROM_LOAD( "cl3ad.p1", 0x0000, 0x010000, CRC(086ed5d9) SHA1(bbf8acf7660c365fecdf51943625ef63a4990b67) )
	ROM_LOAD( "cl3b.p1", 0x0000, 0x010000, CRC(1e90adfd) SHA1(a0c9dc92f6d851a99f011f032f16255bc9c7216d) )
	ROM_LOAD( "cl3bd.p1", 0x0000, 0x010000, CRC(5592b909) SHA1(3d36b5ae986306f2aea37f176b71f4b915f17620) )
	ROM_LOAD( "cl3d.p1", 0x0000, 0x010000, CRC(086ed5d9) SHA1(bbf8acf7660c365fecdf51943625ef63a4990b67) )
	ROM_LOAD( "cl3dy.p1", 0x0000, 0x010000, CRC(96c463a2) SHA1(ee97ee3db452f1e568eb92dc6627b61348e15b4b) )
	ROM_LOAD( "cl3r.p1", 0x0000, 0x010000, CRC(5cb8d2f8) SHA1(16b2643248c424bbdea984bf83f67c4300ccd85a) )
	ROM_LOAD( "cl3xrd.p1", 0x0000, 0x010000, CRC(a1b6317a) SHA1(9b8ed6596b70a4197c759a1b1a0aad4bb2a4d5d7) )
	ROM_LOAD( "cl3y.p1", 0x0000, 0x010000, CRC(3a325d8c) SHA1(9a46f59463601206509d0e394d59fbab736f7850) )
	ROM_LOAD( "clcb.p1", 0x0000, 0x010000, CRC(64333462) SHA1(b9bcd0ecb6eac828b268b59ac174acacaa74e363) )
	ROM_LOAD( "clcdg.p1", 0x0000, 0x010000, CRC(76633c65) SHA1(65a27e5e4bf2ebb09284072f611c4176ce8d0157) )
	ROM_LOAD( "clcdy.p1", 0x0000, 0x010000, CRC(8c4752ff) SHA1(fb6942722242daf180abee7a16d7359b51ee09ca) )
	ROM_LOAD( "cl3s.p1", 0x0000, 0x010000, CRC(d300d6e4) SHA1(c256d550250e270ed913b362c61921210598eb0e) )
ROM_END

#define M4APACH_PLASMA \
	ROM_REGION( 0x40000, "plasmacpu", 0 ) \
	ROM_LOAD16_BYTE( "a6ppl.p0", 0x00000, 0x020000, CRC(350da2df) SHA1(a390e0c7e1e624c17f0e254e0b99ef9dbf56269d) ) \
	ROM_LOAD16_BYTE( "a6ppl.p1", 0x00001, 0x020000, CRC(63038aba) SHA1(8ec4e02109e872460a9598e469b59919cc5450dd) )

ROM_START( m4apach )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a6ps.p1", 0x0000, 0x010000, CRC(41e375c7) SHA1(93556a90227cde6814123c8a7f29f734884e182c) )
	M4APACH_PLASMA

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "a6pcx.p1", 0x0000, 0x010000, CRC(517d8c9a) SHA1(487cecfb10b24eff1582ca6bc97a2dc004e65b0f) ) // was in adders & ladders set
ROM_END


#define M4ELITE_PLASMA \
	ROM_REGION( 0x40000, "plasmacpu", 0 ) \
	ROM_LOAD16_BYTE( "plasma.p0", 0x00000, 0x010000, NO_DUMP ) \
	ROM_LOAD16_BYTE( "plasma.p1", 0x00001, 0x010000, NO_DUMP )

ROM_START( m4elite )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e6ls.p1", 0x0000, 0x010000, CRC(ef4c0d3a) SHA1(1d9433689c457f19d31bd68df4728a87120e474a) )
	M4ELITE_PLASMA
ROM_END

} // anonymous namespace

using namespace mpu4_traits;

#define GAME_FLAGS (MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)

// 00 50 40 14 4c 80 34 44 5c 9c 9c 9c dc 9c dc 94 2c cc ec a8 ec a0 30 40 54  (m435)
GAMEL(199?, m4bigchf,  0,        mpu4plasma(R4, RT1), mpu4, mpu4plasma_state, init_m4, ROT0, "Barcrest","Big Chief (Barcrest) (MPU4 w/ Plasma DMD) (set 1)",            GAME_FLAGS|MACHINE_NO_SOUND,layout_mpu4plasma )
GAMEL(199?, m4bigchfa, m4bigchf, mpu4plasma(R4, RT1), mpu4, mpu4plasma_state, init_m4, ROT0, "Barcrest","Big Chief (Barcrest) (MPU4 w/ Plasma DMD) (set 2)",            GAME_FLAGS|MACHINE_NO_SOUND,layout_mpu4plasma )
GAMEL(199?, m4bigchfb, m4bigchf, mpu4plasma(R4, RT1), mpu4, mpu4plasma_state, init_m4, ROT0, "Barcrest","Big Chief (Barcrest) (MPU4 w/ Plasma DMD) (set 3)",            GAME_FLAGS|MACHINE_NO_SOUND,layout_mpu4plasma )
GAMEL(199?, m4bigchfc, m4bigchf, mpu4plasma(R4, RT1), mpu4, mpu4plasma_state, init_m4, ROT0, "Barcrest","Big Chief (Barcrest) (MPU4 w/ Plasma DMD) (set 4)",            GAME_FLAGS|MACHINE_NO_SOUND,layout_mpu4plasma )

// 00 44 44 c4 70 04 c4 50 a4 d4 30 a0 54 20 64 40 64 e4 f4 14 80
GAMEL(199?, m4click,   0,        mpu4plasma(R4, RT1), mpu4, mpu4plasma_state, init_m4, ROT0, "Barcrest","Clickity Click (Barcrest) (MPU4 w/ Plasma DMD)",               GAME_FLAGS|MACHINE_NO_SOUND,layout_mpu4plasma )

// not confirmed to be plasma, is this an alt version of big chief? maybe it uses the same plasma roms?
// 00 24 24 2c 70 20 0c 60 3c 5c 5c 5c 7c 4c 68 40 34 38 7c 54  (m4duty sequence)
GAMEL(199?, m4apach,   0,        mpu4plasma(R4, RT1), mpu4, mpu4plasma_state, init_m4, ROT0, "Barcrest","Apache (Barcrest) (MPU4 w/ Plasma DMD?)",                      GAME_FLAGS|MACHINE_NO_SOUND,layout_mpu4plasma )
// not confirmed to be plasma, but acts like it
// 00 30 20 14 2c a0 54 24 3c 9c 9c 9c bc 94 6c 80 58 bc bc 98 9c 9c  (m441)
GAMEL(199?, m4elite,   0,        mpu4plasma(R4, RT1), mpu4, mpu4plasma_state, init_m4, ROT0, "Barcrest","Elite (Barcrest) (MPU4 w/ Plasma DMD?)",                       GAME_FLAGS|MACHINE_NO_SOUND,layout_mpu4plasma )
