// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

    Pro Mahjong Kiwame © Athena Co., Ltd. 1994

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/tmp68301.h"
#include "machine/nvram.h"
#include "sound/x1_010.h"
#include "video/x1_001.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class kiwame_state : public driver_device
{
public:
	kiwame_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_spritegen(*this, "spritegen"),
		m_key(*this, "KEY%u", 0U),
		m_kiwame_row_select(0)
	{ }

	void kiwame(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void row_select_w(u16 data);
	u16 input_r(offs_t offset);
	void kiwame_vblank(int state);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void kiwame_map(address_map &map) ATTR_COLD;

	required_device<tmp68301_device> m_maincpu;
	required_device<x1_001_device> m_spritegen;
	required_ioport_array<10> m_key;

	u16 m_kiwame_row_select;
};

void kiwame_state::machine_start()
{
	save_item(NAME(m_kiwame_row_select));
}

void kiwame_state::row_select_w(u16 data)
{
	m_kiwame_row_select = data & 0x001f;
}

u16 kiwame_state::input_r(offs_t offset)
{
	const int row_select = m_kiwame_row_select;

	for (int i = 0; i < 5; i++)
		if (BIT(row_select, i))
			return m_key[i + (offset == 0 ? 0 : 5)]->read();

	return 0xffff;
}

u32 kiwame_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x1f0, cliprect);

	m_spritegen->draw_sprites(screen, bitmap,cliprect,0x1000);
	return 0;
}

void kiwame_state::kiwame_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x20ffff).ram().share("nvram");
	map(0x800000, 0x803fff).rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0x900000, 0x900000).w(m_spritegen, FUNC(x1_001_device::spritebgflag_w8));
	map(0xa00000, 0xa005ff).rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xa00600, 0xa00607).rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xb00000, 0xb003ff).ram().share("palette").w("palette", FUNC(palette_device::write16));  // Palette
	map(0xc00000, 0xc03fff).rw("x1snd", FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0xd00000, 0xd00003).r(FUNC(kiwame_state::input_r));                 // mahjong panel
	map(0xd00004, 0xd00005).portr("COINS");
	map(0xd00008, 0xd00009).nopr();
	map(0xe00000, 0xe00001).portr("DSW1");                                  // DSW
	map(0xe00002, 0xe00003).portr("DSW2");                                  // DSW
}


static INPUT_PORTS_START( kiwame )
	PORT_START("COINS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(5)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0002, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x001c, DEF_STR( None ) )
	PORT_DIPSETTING(      0x0018, "Prelim  1" )
	PORT_DIPSETTING(      0x0014, "Prelim  2" )
	PORT_DIPSETTING(      0x0010, "Final   1" )
	PORT_DIPSETTING(      0x000c, "Final   2" )
	PORT_DIPSETTING(      0x0008, "Final   3" )
	PORT_DIPSETTING(      0x0004, "Qrt Final" )
	PORT_DIPSETTING(      0x0000, "SemiFinal" )
	PORT_DIPNAME( 0x00e0, 0x00e0, "Points Gap" ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(      0x00e0, DEF_STR( None ) )
	PORT_DIPSETTING(      0x00c0, "+6000" )
	PORT_DIPSETTING(      0x00a0, "+4000" )
	PORT_DIPSETTING(      0x0080, "+2000" )
	PORT_DIPSETTING(      0x0060, "-2000" )
	PORT_DIPSETTING(      0x0040, "-4000" )
	PORT_DIPSETTING(      0x0020, "-6000" )
	PORT_DIPSETTING(      0x0000, "-8000" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Player's TSUMO" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, "Manual" )
	PORT_DIPSETTING(      0x0000, "Auto"   )

/*
        row 0   1   2   3   4
bit 0       a   b   c   d   lc
    1       e   f   g   h
    2       i   j   k   l
    3       m   n   ch  po  ff
    4       ka  re  ro
    5       st  bt
*/

	PORT_START("KEY0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY5")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY6")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY7")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY8")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY9")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/* The sprite bitplanes are separated (but there are 2 per rom) */
static const gfx_layout layout_sprites =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2) + 8, RGN_FRAC(1,2) + 0, 8, 0 },
	{ STEP8(0,1), STEP8(8*2*8,1) },
	{ STEP8(0,8*2), STEP8(8*2*8*2,8*2) },
	16*16*2
};

static GFXDECODE_START( gfx_sprites )
	GFXDECODE_ENTRY( "gfx1", 0, layout_sprites, 0, 32 )
GFXDECODE_END


void kiwame_state::kiwame(machine_config &config)
{
	// TODO: determine actual clocks for CPU and video/sound ICs (only XTAL available is 20 MHz)

	/* basic machine hardware */
	TMP68301(config, m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &kiwame_state::kiwame_map);
	m_maincpu->parallel_w_cb().set(FUNC(kiwame_state::row_select_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 2x LH52B256D-70LL + battery (possibly only lower bytes battery-backed)

	X1_001(config, m_spritegen, 16000000, "palette", gfx_sprites);
	// position kludges
	m_spritegen->set_fg_xoffsets(-16, 0); // correct (test grid)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 56*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(kiwame_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, 0);
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 512);    // sprites only

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	x1_010_device &x1snd(X1_010(config, "x1snd", 16000000));
	x1snd.add_route(0, "speaker", 1.0, 0);
	x1snd.add_route(1, "speaker", 1.0, 1);
}


/***************************************************************************

                            Pro Mahjong Kiwame

PCB  : P0-101-1 (the board is made by Allumer/Seta)
CPU  : TMP68301AF-16 (68000 core)
Sound: X1-010
OSC  : 20.0000MHz

ROMs:
fp001001.bin - Main program (27c2001, even)
fp001002.bin - Main program (27c2001, odd)
fp001003.bin - Graphics (23c4000)
fp001005.bin - Samples (27c4000, high)
fp001006.bin - Samples (27c4000, low)

Chips:  X1-001A
        X1-002A
        X1-004
        X1-006
        X1-007
        X1-010

- To initialize high scores, power-on holding start button in service mode.
- The PCB has an unpopulated MAX232 at U158 and 5-pin header at CN1, which
  suggest that linked play was contemplated.

***************************************************************************/

ROM_START( kiwame )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "fp001001.bin", 0x000000, 0x040000, CRC(31b17e39) SHA1(4f001bf365d6c259ac8a13894e207a44c15e1d8b) )
	ROM_LOAD16_BYTE( "fp001002.bin", 0x000001, 0x040000, CRC(5a6e2efb) SHA1(a3b2ecfb5b91c6013370b359f89db0da8f120ad9) )

	ROM_REGION( 0x080000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD( "fp001003.bin", 0x000000, 0x080000, CRC(0f904421) SHA1(de5810746cfab1a4a7d1b055b1a97bc7fbc173dd) )

	ROM_REGION( 0x100000, "x1snd", 0 )  /* Samples */
	ROM_LOAD( "fp001006.bin", 0x000000, 0x080000, CRC(96cf395d) SHA1(877b291598e3a42e5003b2f50a16d162348ce72d) )
	ROM_LOAD( "fp001005.bin", 0x080000, 0x080000, CRC(65b5fe9a) SHA1(35605be00c7c455551d18386fcb5ad013aa2907e) )

	// default NVRAM, avoids "BACKUP RAM ERROR" at boot (useful for inp record/playback)
	ROM_REGION( 0x10000, "nvram", 0 )
	ROM_LOAD( "nvram.bin", 0, 0x10000, CRC(1f719400) SHA1(c63bbe5d3a0a917f74c1bd5e57cd44389e4e645c) )
ROM_END

} // anonymous namespace

GAME( 1994, kiwame,    0,        kiwame,    kiwame,    kiwame_state,   empty_init,     ROT0,   "Athena",                    "Pro Mahjong Kiwame", MACHINE_SUPPORTS_SAVE )
