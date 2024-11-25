// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/****************************************************************************

    Gotya / The Hand driver by Zsolt Vasvari


TODO: Emulated sound

Epson 7910 for the main melody
Hitachi HD38880BP and HD38882PA06 for voice

I think HD38880 is a CPU/MCU, because the game just sends it a sound command (0-0x1a)

couriersud:
   The chips above are speech synthesis chips. HD38880 is the main chip
   whereas HD38882 is an eprom interface. PARCOR based.
   http://www.freepatentsonline.com/4435832.html
   Datasheet lists no parcor coefficients

*****************************************************************************

 About GotYa (from the board owner)

 I believe it is a prototype for several reasons.
 There were quite a few jumpers on the board, hand written labels with
 the dates on them. I also have the manual, the game name is clearly Got-Ya
 and is a Game-A-Tron game.  The game itself had a few flyers from GAT inside
 so I have a hard time believing it was a bootleg.

----

 so despite the fact that 'gotya' might look like it's a bootleg of thehand,
 it's more likely just a prototype / alternate version, it's hard to tell

----

According to Andrew Welburn:

'The Hand' is the original game, GAT licensed it for US manufacture.
It wasn't a runaway seller, they didn't make many, but they had to
change certain aspect of the game to 'localise' it and re-badge it as
their own. There are at least 3-4 scans of manuals online, if it was
proto, a full manual would be rare, and then having several independent
people find and scan the manual would be super rare.

The hand labelling of ROMs is entirely normal for small-run games. The
wire patching is on the underside and is minor tracking changes, common
on small-runs of PCBs.

The games themselves show the most obvious changes. The classic layout
at the top of the screen for 'The hand' with the 'hi-score' is replaced
with 'Got-Ya' in text, a fairly minor hack. The Copyright symbol is
still there in Got-Ya but with the company name scrubbed out.

All in all, Got-Ya should NOT be marked as a prototype in MAME, it's a
US territory license hack of another game 'The Hand'. Nothing about it
says prototype, and the original base game is 'The Hand'.

****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/watchdog.h"
#include "sound/samples.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"
#include "video/resnet.h"


namespace {

class gotya_state : public driver_device
{
public:
	gotya_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_scroll(*this, "scroll"),
		m_videoram(*this, "videoram%u", 1U),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_samples(*this, "samples")
	{ }

	void gotya(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<samples_device> m_samples;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_scroll_bit_8 = 0;

	// sound-related
	uint8_t m_theme_playing = 0;

	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void video_control_w(uint8_t data);
	void soundlatch_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_rows);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_status_row(bitmap_ind16 &bitmap, const rectangle &cliprect, int sx, int col);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_status(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

void gotya_state::palette(palette_device &palette) const
{
	uint8_t const *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 0, 0,
			3, &resistances_rg[0], gweights, 0, 0,
			2, &resistances_b[0],  bweights, 0, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		// red component
		int bit0 = BIT(color_prom[i], 0);
		int bit1 = BIT(color_prom[i], 1);
		int bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 32;

	for (int i = 0; i < 0x40; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x07;
		palette.set_pen_indirect(i, ctabentry);
	}
}

void gotya_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[0][offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void gotya_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void gotya_state::video_control_w(uint8_t data)
{
	/* bit 0 - scroll bit 8
	   bit 1 - flip screen
	   bit 2 - sound disable ??? */

	m_scroll_bit_8 = data & 0x01;

	if (flip_screen() != (data & 0x02))
	{
		flip_screen_set(data & 0x02);
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(gotya_state::get_bg_tile_info)
{
	int code = m_videoram[0][tile_index];
	int color = m_colorram[tile_index] & 0x0f;

	tileinfo.set(0, code, color, 0);
}

TILEMAP_MAPPER_MEMBER(gotya_state::tilemap_scan_rows)
{
	// logical (col,row) -> memory offset
	row = 31 - row;
	col = 63 - col;
	return ((row) * (num_cols >> 1)) + (col & 31) + ((col >> 5) * 0x400);
}

void gotya_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gotya_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(gotya_state::tilemap_scan_rows)), 8, 8, 64, 32);
}

void gotya_state::draw_status_row(bitmap_ind16 &bitmap, const rectangle &cliprect, int sx, int col)
{
	if (flip_screen())
		sx = 35 - sx;

	for (int row = 29; row >= 0; row--)
	{
		int sy;

		if (flip_screen())
			sy = row;
		else
			sy = 31 - row;

		m_gfxdecode->gfx(0)->opaque(bitmap, cliprect,
				m_videoram[1][row * 32 + col],
				m_videoram[1][row * 32 + col + 0x10] & 0x0f,
				flip_screen_x(), flip_screen_y(),
				8 * sx, 8 * sy);
	}
}

void gotya_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 2; offs < 0x0e; offs += 2)
	{
		int code = m_spriteram[offs + 0x01] >> 2;
		int color = m_spriteram[offs + 0x11] & 0x0f;
		int sx = 256 - m_spriteram[offs + 0x10] + (m_spriteram[offs + 0x01] & 0x01) * 256;
		int sy = m_spriteram[offs + 0x00];

		if (flip_screen())
			sy = 240 - sy;

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code, color,
				flip_screen_x(), flip_screen_y(),
				sx, sy, 0);
	}
}

void gotya_state::draw_status(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_status_row(bitmap, cliprect, 0,  1);
	draw_status_row(bitmap, cliprect, 1,  0);
	draw_status_row(bitmap, cliprect, 2,  2);  // these two are blank, but I dont' know if the data comes
	draw_status_row(bitmap, cliprect, 33, 13); // from RAM or 'hardcoded' into the hardware. Likely the latter
	draw_status_row(bitmap, cliprect, 35, 14);
	draw_status_row(bitmap, cliprect, 34, 15);
}

uint32_t gotya_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, -(*m_scroll + (m_scroll_bit_8 * 256)) - 2 * 8);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	draw_status(bitmap, cliprect);
	return 0;
}


static const char *const sample_names[] =
{                                               // Address triggered at
	"*thehand",
	"01",   // game start tune              // 075f
	"02",   // coin in                      // 0074
	"03",   // eat dot                      // 0e45
	"05",   // eat dollar sign              // 0e45

	"06",   // door open                    // 19e1
	"07",   // door close                   // 1965

	"08",   // theme song                   // 0821
	//"09"                                  // 1569

	// one of these two is played after eating the last dot
	"0a",   // piccolo                      // 17af
	"0b",   // tune                         // 17af

	//"0f"                                  // 08ee
	"10",   // 'We're even. Bye Bye!'       // 162a
	"11",   // 'You got me!'                // 1657
	"12",   // 'You have lost out'          // 085e

	"13",   // 'Rock'                       // 14de
	"14",   // 'Scissors'                   // 14f3
	"15",   // 'Paper'                      // 1508

	// one of these is played when going by the girl between levels
	"16",   // 'Very good!'                 // 194a
	"17",   // 'Wonderful!'                 // 194a
	"18",   // 'Come on!'                   // 194a
	"19",   // 'I love you!'                // 194a
	"1a",   // 'See you again!'             // 194a
	nullptr
};

struct gotya_sample
{
	int8_t sound_command;
	uint8_t channel;
	uint8_t looping;
};

// all the speech can go to one channel?

static const struct gotya_sample gotya_samples[] =
{
	{ 0x01, 0, 0 },
	{ 0x02, 1, 0 },
	{ 0x03, 2, 0 },
	{ 0x05, 2, 0 },
	{ 0x06, 3, 0 },
	{ 0x07, 3, 0 },
	{ 0x08, 0, 1 },
	{ 0x0a, 0, 0 },
	{ 0x0b, 0, 0 },

	{ 0x10, 3, 0 },
	{ 0x11, 3, 0 },
	{ 0x12, 0, 0 }, // this should stop the main tune
	{ 0x13, 3, 0 },
	{ 0x14, 3, 0 },
	{ 0x15, 3, 0 },
	{ 0x16, 3, 0 },
	{ 0x17, 3, 0 },
	{ 0x18, 3, 0 },
	{ 0x19, 3, 0 },
	{ 0x1a, 3, 0 },
	{   -1, 0, 0 }  // end of array
};

void gotya_state::soundlatch_w(uint8_t data)
{
	if (data == 0)
	{
		m_samples->stop(0);
		m_theme_playing = 0;
		return;
	}

	// search for sample to play
	for (int sample_number = 0; gotya_samples[sample_number].sound_command != -1; sample_number++)
	{
		if (gotya_samples[sample_number].sound_command == data)
		{
			if (gotya_samples[sample_number].looping && m_theme_playing)
			{
				// don't restart main theme
				return;
			}

			m_samples->start(gotya_samples[sample_number].channel, sample_number, gotya_samples[sample_number].looping);

			if (gotya_samples[sample_number].channel == 0)
			{
				m_theme_playing = gotya_samples[sample_number].looping;
			}
			return;
		}
	}
}


void gotya_state::prg_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x5000, 0x5fff).ram();
	map(0x6000, 0x6000).portr("P1");
	map(0x6001, 0x6001).portr("P2");
	map(0x6002, 0x6002).portr("DSW");
	map(0x6004, 0x6004).w(FUNC(gotya_state::video_control_w));
	map(0x6005, 0x6005).w(FUNC(gotya_state::soundlatch_w));
	map(0x6006, 0x6006).writeonly().share(m_scroll);
	map(0x6007, 0x6007).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xc000, 0xc7ff).ram().w(FUNC(gotya_state::videoram_w)).share(m_videoram[0]);
	map(0xc800, 0xcfff).ram().w(FUNC(gotya_state::colorram_w)).share(m_colorram);
	map(0xd000, 0xd3df).ram().share(m_videoram[1]);
	map(0xd3e0, 0xd3ff).ram().share(m_spriteram);
}


static INPUT_PORTS_START( gotya )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Paper") PORT_PLAYER(1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Scissors") PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Rock") PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_DIPNAME( 0x10, 0x10, "Sound Test" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Paper") PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Scissors") PORT_PLAYER(2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Rock") PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "15000" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Game Type" )         /* Manual Says:  Before main switch on: Test Pattern */
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )   /*                After main switch on: Endless game */
	PORT_DIPSETTING(    0x00, "Endless" )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	256,    // 256 characters
	2,      // 2 bits per pixel
	{ 0, 4 },   // the bitplanes are packed in one byte
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	16*8    // every char takes 16 consecutive bytes
};

static const gfx_layout spritelayout =
{
	16,16,  // 16*16 characters
	64,     // 64 characters
	2,      // 2 bits per pixel
	{ 0, 4 },   // the bitplanes are packed in one byte
	{ 0, 1, 2, 3, 24*8+0, 24*8+1, 24*8+2, 24*8+3,
		16*8+0, 16*8+1, 16*8+2, 16*8+3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 39*8, 38*8, 37*8, 36*8, 35*8, 34*8, 33*8, 32*8,
		7*8,  6*8,  5*8,  4*8,  3*8,  2*8,  1*8,  0*8 },
	64*8    // every char takes 64 consecutive bytes
};

static GFXDECODE_START( gfx_gotya )
	GFXDECODE_ENTRY( "chars",   0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 0, 16 )
GFXDECODE_END


void gotya_state::machine_start()
{
	save_item(NAME(m_scroll_bit_8));
	save_item(NAME(m_theme_playing));
}

void gotya_state::machine_reset()
{
	m_scroll_bit_8 = 0;
	m_theme_playing = 0;
}

void gotya_state::gotya(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 18432000 / 6); // 3.072 MHz ???
	m_maincpu->set_addrmap(AS_PROGRAM, &gotya_state::prg_map);
	m_maincpu->set_vblank_int("screen", FUNC(gotya_state::irq0_line_hold));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(36*8, 32*8);
	screen.set_visarea(0, 36*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(gotya_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gotya);
	PALETTE(config, m_palette, FUNC(gotya_state::palette), 16*4, 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SAMPLES(config, m_samples);
	m_samples->set_channels(4);
	m_samples->set_samples_names(sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 1.0);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( thehand )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hand6.bin",  0x0000, 0x1000, CRC(a33b806c) SHA1(1e552af5362e7b003f55e78bb59589e1db55557c) )
	ROM_LOAD( "hand5.bin",  0x1000, 0x1000, CRC(89bcde82) SHA1(d074bb6a1975160eb533d5fd9289170a68209046) )
	ROM_LOAD( "hand4.bin",  0x2000, 0x1000, CRC(c6844a83) SHA1(84e220dce3f5ddee9dd0377f3bebdd4027fc9108) )
	ROM_LOAD( "gb-03.bin",  0x3000, 0x1000, CRC(f34d90ab) SHA1(bec5f6a34a273f308083a280f2b425d9c273c69b) )

	ROM_REGION( 0x1000,  "chars", 0 )
	ROM_LOAD( "hand12.bin", 0x0000, 0x1000, CRC(95773b46) SHA1(db8d7ace4eafd4c72edfeff6003ca6e96e0239b5) )

	ROM_REGION( 0x1000,  "sprites", 0 )
	ROM_LOAD( "gb-11.bin",  0x0000, 0x1000, CRC(5d5eca1b) SHA1(d7c6b5f4d398d5e33cc411ed593d6f53a9979493) )

	ROM_REGION( 0x0120,  "proms", 0 )
	ROM_LOAD( "prom.1a",    0x0000, 0x0020, CRC(4864a5a0) SHA1(5b49f60b085fa026d4e8d4a5ad28ee7037a8ff9c) )    // color PROM
	ROM_LOAD( "prom.4c",    0x0020, 0x0100, CRC(4745b5f6) SHA1(02a7f759e9bc8089cbd9213a71bbe671f9641638) )    // lookup table

	ROM_REGION( 0x1000,  "user1", 0 )       // no idea what these are
	ROM_LOAD( "hand1.bin",  0x0000, 0x0800, CRC(ccc537e0) SHA1(471fd49225aa14b91d085178e1b58b6c4ae76481) )
	ROM_LOAD( "gb-02.bin",  0x0800, 0x0800, CRC(65a7e284) SHA1(91e9c34dcf20608863ad5475dc0c4309971c8eee) )

	ROM_REGION( 0x8000,  "user2", 0 )       // HD38880 code/samples?
	ROM_LOAD( "gb-10.bin",  0x4000, 0x1000, CRC(8101915f) SHA1(c4d21b1938ea7e0d47c48e74037f005280ac101b) )
	ROM_LOAD( "gb-09.bin",  0x5000, 0x1000, CRC(619bba76) SHA1(2a2deffe6f058fc840329fbfffbc0c70a0147c14) )
	ROM_LOAD( "gb-08.bin",  0x6000, 0x1000, CRC(82f59528) SHA1(6bfa2329eb291040bfc229c56420865253b0132a) )
	ROM_LOAD( "hand7.bin",  0x7000, 0x1000, CRC(fbf1c5de) SHA1(dd3181a8da1972e3c997678bb868256a10f33d04) )
ROM_END

ROM_START( gotya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gb-06.bin",  0x0000, 0x1000, CRC(7793985a) SHA1(23aa8bd161e700bea59b92075423cdf55e9a26c3) )
	ROM_LOAD( "gb-05.bin",  0x1000, 0x1000, CRC(683d188b) SHA1(5341c62f5cf384c73be0d7a0a230bb8cebfbe709) )
	ROM_LOAD( "gb-04.bin",  0x2000, 0x1000, CRC(15b72f09) SHA1(bd941722ed1310d5c8ca8a44899368cba3815f3b) )
	ROM_LOAD( "gb-03.bin",  0x3000, 0x1000, CRC(f34d90ab) SHA1(bec5f6a34a273f308083a280f2b425d9c273c69b) )    // this is the only ROM that passes the ROM test

	ROM_REGION( 0x1000,  "chars", 0 )
	ROM_LOAD( "gb-12.bin",  0x0000, 0x1000, CRC(4993d735) SHA1(9e47876238a8af3659721191a5f75c33507ed1a5) )

	ROM_REGION( 0x1000,  "sprites", 0 )
	ROM_LOAD( "gb-11.bin",  0x0000, 0x1000, CRC(5d5eca1b) SHA1(d7c6b5f4d398d5e33cc411ed593d6f53a9979493) )

	ROM_REGION( 0x0120,  "proms", 0 )
	ROM_LOAD( "prom.1a",    0x0000, 0x0020, CRC(4864a5a0) SHA1(5b49f60b085fa026d4e8d4a5ad28ee7037a8ff9c) )    // color PROM
	ROM_LOAD( "prom.4c",    0x0020, 0x0100, CRC(4745b5f6) SHA1(02a7f759e9bc8089cbd9213a71bbe671f9641638) )    // lookup table

	ROM_REGION( 0x1000,  "user1", 0 )       // no idea what these are
	ROM_LOAD( "gb-01.bin",  0x0000, 0x0800, CRC(c31dba64) SHA1(15ae54b7d475ca3f0a3acc45cd8da2916c5fdef2) )
	ROM_LOAD( "gb-02.bin",  0x0800, 0x0800, CRC(65a7e284) SHA1(91e9c34dcf20608863ad5475dc0c4309971c8eee) )

	ROM_REGION( 0x8000,  "user2", 0 )       // HD38880 code/samples?
	ROM_LOAD( "gb-10.bin",  0x4000, 0x1000, CRC(8101915f) SHA1(c4d21b1938ea7e0d47c48e74037f005280ac101b) )
	ROM_LOAD( "gb-09.bin",  0x5000, 0x1000, CRC(619bba76) SHA1(2a2deffe6f058fc840329fbfffbc0c70a0147c14) )
	ROM_LOAD( "gb-08.bin",  0x6000, 0x1000, CRC(82f59528) SHA1(6bfa2329eb291040bfc229c56420865253b0132a) )
	ROM_LOAD( "gb-07.bin",  0x7000, 0x1000, CRC(92a9f8bf) SHA1(9231cd86f24f1e6a585c3a919add50c1f8e42a4c) )
ROM_END

} // anonymous namespace


GAME( 1981, thehand, 0,       gotya, gotya, gotya_state, empty_init, ROT270, "T.I.C.", "The Hand", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, gotya,   thehand, gotya, gotya, gotya_state, empty_init, ROT270, "T.I.C. (Game-A-Tron license)", "Got-Ya (12/24/1981)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
