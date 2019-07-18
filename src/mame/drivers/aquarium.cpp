// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Aquarium (c)1996 Excellent Systems */

/*

AQUARIUM
EXCELLENT SYSTEMS
ES-9206
+------------------------------------------------+
|                  6116      23C16000   23C16000*|
|         YM2151   6116      23C16000*  23C16000*|
|                    +-------+ +-------+         |
|     1.056MHz M6295 |ES-9303| |ES 9207|         |
|YM3012              |       | |       | AS7C256 |
|                    +-------+ +-------+ AS7C256 |
|J                                       AS7C256 |
|A                    4         AS7C256  AS7C256 |
|M                Z80B   32MHz                   |
|M                5      68000P-16  14.318MHz    |
|A                    PAL   AS7C256      AS7C256 |
|                        +-------+       AS7C256 |
|         PAL            |ES-9208|  PB1       3  |
|     1   PAL  6         |       |   8   27C4096*|
|                        +-------+               |
|SW4*SW3* SW2 SW1    AS7C256       2        7    |
+------------------------------------------------+

   CPU: TMP68HC000P-16
 Sound: Z0840006PSC Z80B
        OKI M6295, YM2151 & YM3012 DAC
   OSC: 32MHz, 14.31818MHz & 1056kHz resonator
Custom: EXCELLENT SYSTEM ES-9208 347101 (QFP160)
        EXCELLENT SYSTEM LTD. ES 9207 9343 T (QFP208)
        ES-9303 EXCELLENT 9338 C001 (QFP120)
 Other: PB1 - Push button reset

* Denotes unpopulated components


Notes:
- A bug in the program code causes the OKI to be reset on the very
  first coin inserted.

// Sound banking + video references
// https://www.youtube.com/watch?v=nyAQPrkt_a4
// https://www.youtube.com/watch?v=0gn2Kj2M46Q


*/


#include "emu.h"
#include "includes/aquarium.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/ym2151.h"
#include "speaker.h"


void aquarium_state::watchdog_w(u8 data)
{
	m_watchdog->write_line_ck(BIT(data, 7));
	// bits 0 & 1 also used
}

void aquarium_state::z80_bank_w(u8 data)
{
	// uses bits ---x --xx
	data = bitswap<8>(data, 7, 6, 5, 2, 3, 1, 4, 0);

	//printf("aquarium bank %04x %04x\n", data, mem_mask);
	// aquarium bank 0003 00ff - correct (title)   011
	// aquarium bank 0006 00ff - correct (select)  110
	// aquarium bank 0005 00ff - level 1 (correct)
	// (all music seems correct w/regards the reference video)

	m_audiobank->set_entry(data & 0x7);
}

u8 aquarium_state::snd_bitswap(u8 scrambled_data)
{
	return bitswap<8>(scrambled_data, 0, 1, 2, 3, 4, 5, 6, 7);
}

u8 aquarium_state::oki_r()
{
	return snd_bitswap(m_oki->read());
}

void aquarium_state::oki_w(u8 data)
{
	logerror("%s:Writing %04x to the OKI M6295\n", machine().describe_context(), snd_bitswap(data));
	m_oki->write(snd_bitswap(data));
}


void aquarium_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0xc00000, 0xc00fff).ram().w(FUNC(aquarium_state::mid_videoram_w)).share("mid_videoram");
	map(0xc01000, 0xc01fff).ram().w(FUNC(aquarium_state::bak_videoram_w)).share("bak_videoram");
	map(0xc02000, 0xc03fff).ram().w(FUNC(aquarium_state::txt_videoram_w)).share("txt_videoram");
	map(0xc80000, 0xc81fff).rw(m_sprgen, FUNC(excellent_spr_device::read), FUNC(excellent_spr_device::write)).umask16(0x00ff);
	map(0xd00000, 0xd00fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xd80014, 0xd8001f).writeonly().share("scroll");
	map(0xd80068, 0xd80069).nopw();        /* probably not used */
	map(0xd80080, 0xd80081).portr("DSW");
	map(0xd80082, 0xd80083).nopr(); /* stored but not read back ? check code at 0x01f440 */
	map(0xd80084, 0xd80085).portr("INPUTS");
	map(0xd80086, 0xd80087).portr("SYSTEM");
	map(0xd80088, 0xd80088).w(FUNC(aquarium_state::watchdog_w));
	map(0xd8008b, 0xd8008b).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xff0000, 0xffffff).ram();
}

void aquarium_state::snd_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr("bank1");
}

void aquarium_state::snd_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x02, 0x02).rw(FUNC(aquarium_state::oki_r), FUNC(aquarium_state::oki_w));
	map(0x04, 0x04).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x06, 0x06).w(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_w)); // only written with 0 for some reason
	map(0x08, 0x08).w(FUNC(aquarium_state::z80_bank_w));
}

static INPUT_PORTS_START( aquarium )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, "Winning Rounds (Player VS CPU)" )    PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x000c, "1/1" )
	PORT_DIPSETTING(      0x0008, "2/3" )
	PORT_DIPSETTING(      0x0004, "3/5" )
//  PORT_DIPSETTING(      0x0000, "1/1" )                   /* Not used or listed in manual */
	PORT_DIPNAME( 0x0030, 0x0030, "Winning Rounds (Player VS Player)" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0030, "1/1" )
	PORT_DIPSETTING(      0x0020, "2/3" )
	PORT_DIPSETTING(      0x0010, "3/5" )
//  PORT_DIPSETTING(      0x0000, "1/1" )                   /* Not used or listed in manual */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7" )            /* Listed in the manual as always OFF */
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )            /* Listed in the manual as always OFF */
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW2:4" )            /* Listed in the manual as always OFF */
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )            /* Listed in the manual as always OFF */
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )            /* Listed in the manual as always OFF */

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* used in testmode, but not in game? */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* used in testmode, but not in game? */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundlatch", generic_latch_8_device, pending_r)
INPUT_PORTS_END

static const gfx_layout layout_5bpp_hi =
{
	16, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0, 1) },
	{ STEP16(0, 16) },
	16*16
};

static GFXDECODE_START( gfx_aquarium )
	GFXDECODE_ENTRY( "txt",    0, gfx_8x8x4_packed_msb,   0x200, 16 )
	GFXDECODE_ENTRY( "mid",    0, gfx_16x16x4_packed_msb, 0x400, 32 ) // low 4bpp of 5bpp data
	GFXDECODE_ENTRY( "bak",    0, gfx_16x16x4_packed_msb, 0x400, 32 ) // low 4bpp of 5bpp data
	GFXDECODE_ENTRY( "bak_hi", 0, layout_5bpp_hi,         0x400, 32 ) // hi 1bpp of 5bpp data
	GFXDECODE_ENTRY( "mid_hi", 0, layout_5bpp_hi,         0x400, 32 ) // hi 1bpp of 5bpp data
GFXDECODE_END

void aquarium_state::aquarium(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/2); // clock not verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &aquarium_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(aquarium_state::irq1_line_hold));

	Z80(config, m_audiocpu, XTAL(32'000'000)/6); // clock not verified on pcb
	m_audiocpu->set_addrmap(AS_PROGRAM, &aquarium_state::snd_map);
	m_audiocpu->set_addrmap(AS_IO, &aquarium_state::snd_portmap);

	// Is this the actual IC type? Some other Excellent games from this period use a MAX693.
	MB3773(config, m_watchdog, 0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 64*8);
	m_screen->set_visarea(2*8, 42*8-1, 2*8, 34*8-1);
	m_screen->set_screen_update(FUNC(aquarium_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_aquarium);
	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 0x1000/2);

	EXCELLENT_SPRITE(config, m_sprgen, 0);
	m_sprgen->set_palette(m_palette);
	m_sprgen->set_color_base(0x300);
	m_sprgen->set_colpri_callback(FUNC(aquarium_state::aquarium_colpri_cb), this);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	m_soundlatch->set_separate_acknowledge(true);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(14'318'181)/4)); // clock not verified on pcb
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.45);
	ymsnd.add_route(1, "rspeaker", 0.45);

	OKIM6295(config, m_oki, XTAL(1'056'000), okim6295_device::PIN7_HIGH); // pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "lspeaker", 0.47);
	m_oki->add_route(ALL_OUTPUTS, "rspeaker", 0.47);
}

ROM_START( aquarium )
	ROM_REGION( 0x080000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "aquar3.13h",  0x000000, 0x080000, CRC(f197991e) SHA1(0a217d735e2643605dbfd6ee20f98f46b37d4838) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 (sound) code */
	ROM_LOAD( "excellent_5.10c",  0x000000, 0x40000, CRC(fa555be1) SHA1(07236f2b2ba67e92984b9ddf4a8154221d535245) )

	ROM_REGION( 0x080000, "mid", 0 ) /* BG Tiles */
	ROM_LOAD16_WORD_SWAP( "excellent_1.15b", 0x000000, 0x080000, CRC(575df6ac) SHA1(071394273e512666fe124facdd8591a767ad0819) ) // 4bpp
	/* data is expanded here from mid_hi */
	ROM_REGION( 0x020000, "mid_hi", 0 ) /* BG Tiles */
	ROM_LOAD( "excellent_6.15d", 0x000000, 0x020000, CRC(9065b146) SHA1(befc218bbcd63453ea7eb8f976796d36f2b2d552) ) // 1bpp

	ROM_REGION( 0x080000, "bak", 0 ) /* BG Tiles */
	ROM_LOAD16_WORD_SWAP( "excellent_8.14g", 0x000000, 0x080000, CRC(915520c4) SHA1(308207cb20f1ed6df365710c808644a6e4f07614) ) // 4bpp
	/* data is expanded here from bak_hi */
	ROM_REGION( 0x020000, "bak_hi", 0 ) /* BG Tiles */
	ROM_LOAD( "excellent_7.17g", 0x000000, 0x020000, CRC(b96b2b82) SHA1(2b719d0c185d1eca4cd9ea66bed7842b74062288) ) // 1bpp

	ROM_REGION( 0x060000, "txt", 0 ) /* FG Tiles */
	ROM_LOAD16_WORD_SWAP( "excellent_2.17e", 0x000000, 0x020000, CRC(aa071b05) SHA1(517415bfd8e4dd51c6eb03a25c706f8613d34a09) )

	ROM_REGION( 0x200000, "spritegen", 0 ) /* Sprites? */
	ROM_LOAD16_WORD_SWAP( "d23c8000.1f",   0x000000, 0x0100000, CRC(14758b3c) SHA1(b372ccb42acb55a3dd15352a9d4ed576878a6731) ) // PCB denotes 23C16000 but a 23C8000 MASK is used

	ROM_REGION( 0x100000, "oki", 0 ) /* Samples */
	ROM_LOAD( "excellent_4.7d",  0x000000, 0x80000, CRC(9a4af531) SHA1(bb201b7a6c9fd5924a0d79090257efffd8d4aba1) )
ROM_END

ROM_START( aquariumj )
	ROM_REGION( 0x080000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "excellent_3.13h",  0x000000, 0x080000, CRC(344509a1) SHA1(9deb610732dee5066b3225cd7b1929b767579235) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 (sound) code */
	ROM_LOAD( "excellent_5.10c",  0x000000, 0x40000, CRC(fa555be1) SHA1(07236f2b2ba67e92984b9ddf4a8154221d535245) )

	ROM_REGION( 0x080000, "mid", 0 ) /* BG Tiles */
	ROM_LOAD16_WORD_SWAP( "excellent_1.15b", 0x000000, 0x080000, CRC(575df6ac) SHA1(071394273e512666fe124facdd8591a767ad0819) ) // 4bpp
	/* data is expanded here from mid_hi */
	ROM_REGION( 0x020000, "mid_hi", 0 ) /* BG Tiles */
	ROM_LOAD( "excellent_6.15d", 0x000000, 0x020000, CRC(9065b146) SHA1(befc218bbcd63453ea7eb8f976796d36f2b2d552) ) // 1bpp

	ROM_REGION( 0x080000, "bak", 0 ) /* BG Tiles */
	ROM_LOAD16_WORD_SWAP( "excellent_8.14g", 0x000000, 0x080000, CRC(915520c4) SHA1(308207cb20f1ed6df365710c808644a6e4f07614) ) // 4bpp
	/* data is expanded here from bak_hi */
	ROM_REGION( 0x020000, "bak_hi", 0 ) /* BG Tiles */
	ROM_LOAD( "excellent_7.17g", 0x000000, 0x020000, CRC(b96b2b82) SHA1(2b719d0c185d1eca4cd9ea66bed7842b74062288) ) // 1bpp

	ROM_REGION( 0x060000, "txt", 0 ) /* FG Tiles */
	ROM_LOAD16_WORD_SWAP( "excellent_2.17e", 0x000000, 0x020000, CRC(aa071b05) SHA1(517415bfd8e4dd51c6eb03a25c706f8613d34a09) )

	ROM_REGION( 0x200000, "spritegen", 0 ) /* Sprites? */
	ROM_LOAD16_WORD_SWAP( "d23c8000.1f",   0x000000, 0x0100000, CRC(14758b3c) SHA1(b372ccb42acb55a3dd15352a9d4ed576878a6731) ) // PCB denotes 23C16000 but a 23C8000 MASK is used

	ROM_REGION( 0x100000, "oki", 0 ) /* Samples */
	ROM_LOAD( "excellent_4.7d",  0x000000, 0x80000, CRC(9a4af531) SHA1(bb201b7a6c9fd5924a0d79090257efffd8d4aba1) )
ROM_END

void aquarium_state::expand_gfx(int low, int hi)
{
	/* The BG tiles are 5bpp, this rearranges the data from
	   the roms containing the 1bpp data so we can decode it
	   correctly */
	gfx_element *gfx_l = m_gfxdecode->gfx(low);
	gfx_element *gfx_h = m_gfxdecode->gfx(hi);

	// allocate memory for the assembled data
	u8 *srcdata = auto_alloc_array(machine(), u8, gfx_l->elements() * gfx_l->width() * gfx_l->height());

	// loop over elements
	u8 *dest = srcdata;
	for (int c = 0; c < gfx_l->elements(); c++)
	{
		const u16 *c0base = gfx_l->get_data(c);
		const u16 *c1base = gfx_h->get_data(c);

		// loop over height
		for (int y = 0; y < gfx_l->height(); y++)
		{
			const u16 *c0 = c0base;
			const u16 *c1 = c1base;

			for (int x = 0; x < gfx_l->width(); x++)
			{
				u8 hi_data = *c1++;
				*dest++ = (*c0++ & 0xf) | ((hi_data << 4) & 0x10);
			}
			c0base += gfx_l->rowbytes();
			c1base += gfx_h->rowbytes();
		}
	}

	gfx_l->set_raw_layout(srcdata, gfx_l->width(), gfx_l->height(), gfx_l->elements(), 8 * gfx_l->width(), 8 * gfx_l->width() * gfx_l->height());
	gfx_l->set_granularity(32);
	m_gfxdecode->set_gfx(hi, nullptr);
}

void aquarium_state::init_aquarium()
{
	expand_gfx(1, 4);
	expand_gfx(2, 3);

	u8 *Z80 = memregion("audiocpu")->base();

	/* configure and set up the sound bank */
	m_audiobank->configure_entries(0, 0x8, &Z80[0x00000], 0x8000);
	m_audiobank->set_entry(0x00);
}

GAME( 1996, aquarium,  0,        aquarium, aquarium, aquarium_state, init_aquarium, ROT0, "Excellent System", "Aquarium (US)",    MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1996, aquariumj, aquarium, aquarium, aquarium, aquarium_state, init_aquarium, ROT0, "Excellent System", "Aquarium (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
