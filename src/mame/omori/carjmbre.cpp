// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    Car Jamboree
    Omori Electric CAD (OEC) 1983

    TODO:
    - colors are probably wrong
    - sprite priorities? (eg. player car jumping on the ramp, 1 part disappears)
    - first 2 letters on titlescreen look misaligned with the tilemap
    - The spriteram holds 2 sprite lists (00-7f and 80-ff), they are identical.
      Is it an unused feature? Or a RAM access speed workaround?

----------------------------------------------------------------------------

    PCB sketch:

    c14                c.d19
    c13                c.d18           c10
    c12                                c9
    c11         2125   2125
                2125   2125
                2125   2125  2114 2114
                2125   2125  2114 2114
                2125   2125            c8
                2125   2125            c7
                                       c6
                                       c5
                                       c4
                                       c3
    5101                               c2
    5101                               c1
                                       6116
    18.432MHz
              6116
    Z80A      c15
                                       Z80A
           8910         SW
           8910

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "video/resnet.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class carjmbre_state : public driver_device
{
public:
	carjmbre_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	bool m_nmi_enabled = false;
	uint8_t m_bgcolor = 0;
	tilemap_t *m_tilemap = nullptr;

	void bgcolor_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void nmi_enable_w(uint8_t data);
	void flipscreen_w(uint8_t data);
	INTERRUPT_GEN_MEMBER(vblank_nmi);

	void carjmbre_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(get_tile_info);

	void carjmbre(machine_config &config);
	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
};

void carjmbre_state::machine_start()
{
	// zerofill
	m_nmi_enabled = false;
	m_bgcolor = 0;

	// register for savestates
	save_item(NAME(m_nmi_enabled));
	save_item(NAME(m_bgcolor));
}


/***************************************************************************

  Video

***************************************************************************/

// palette info from Popper schematics (OEC 1983, very similar video hw)
static const res_net_decode_info carjmbre_decode_info =
{
	1,      // there may be two proms needed to construct color
	0, 63,  // start/end
	//  R,   G,   B
	{   0,   0,   0 },     // offsets
	{   0,   3,   6 },     // shifts
	{0x07,0x07,0x03 }      // masks
};

static const res_net_info carjmbre_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT,
	{
		{ RES_NET_AMP_NONE, 0, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_NONE, 0, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_NONE, 0, 0, 2, {  470, 220,   0 } }
	}
};

void carjmbre_state::carjmbre_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	std::vector<rgb_t> rgb;

	compute_res_net_all(rgb, color_prom, carjmbre_decode_info, carjmbre_net_info);
	palette.set_pen_colors(0, rgb);
	palette.palette()->normalize_range(0, 63);
}

void carjmbre_state::bgcolor_w(uint8_t data)
{
	// guessed, seems to match with flyer
	m_bgcolor = ~data & 0x3f;
}


// tilemap

void carjmbre_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset & 0x3ff);
}

TILE_GET_INFO_MEMBER(carjmbre_state::get_tile_info)
{
	int attr = m_videoram[tile_index | 0x400];
	int code = (m_videoram[tile_index] & 0xff) | (attr << 1 & 0x100);
	tileinfo.set(0, code, attr & 0xf, 0);
}

void carjmbre_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(carjmbre_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap->set_transparent_pen(0);
}


uint32_t carjmbre_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_bgcolor, cliprect);
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


// sprites

void carjmbre_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0x80 - 4; offs >= 0; offs -= 4)
	{
		int sy = m_spriteram[offs];
		int code = m_spriteram[offs + 1];
		int color = m_spriteram[offs + 2] & 0xf;
		int flipx = m_spriteram[offs + 2] >> 6 & 1;
		int flipy = m_spriteram[offs + 2] >> 7 & 1;
		int sx = m_spriteram[offs + 3];

		// align to tilemap
		if (flip_screen())
		{
			sy += 1;
			sx = 233 - sx;
			flipx = !flipx;
			flipy = !flipy;
		}
		else
		{
			sy = 241 - sy;
			sx -= 7;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx, sy, 0);
	}
}



/***************************************************************************

  I/O, Memory Maps

***************************************************************************/

// maincpu side

INTERRUPT_GEN_MEMBER(carjmbre_state::vblank_nmi)
{
	if (m_nmi_enabled)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void carjmbre_state::nmi_enable_w(uint8_t data)
{
	// d0: enable/clear vblank nmi
	m_nmi_enabled = bool(data & 1);
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void carjmbre_state::flipscreen_w(uint8_t data)
{
	// d0: flip screen (cocktail mode)
	flip_screen_set(data & 1);
}

void carjmbre_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8803, 0x8803).w(FUNC(carjmbre_state::nmi_enable_w));
	map(0x8805, 0x8805).w(FUNC(carjmbre_state::bgcolor_w));
	map(0x8807, 0x8807).w(FUNC(carjmbre_state::flipscreen_w));
	map(0x8000, 0x87ff).ram(); // 6116
	map(0x9000, 0x97ff).ram().w(FUNC(carjmbre_state::videoram_w)).share("videoram"); // 2114*4
	map(0x9800, 0x98ff).ram().share("spriteram"); // 5101*2
	map(0xa000, 0xa000).portr("IN1");
	map(0xa800, 0xa800).portr("IN2");
	map(0xb800, 0xb800).portr("DSW").w("soundlatch", FUNC(generic_latch_8_device::write));
}


// audiocpu side

void carjmbre_state::sound_map(address_map &map)
{
	map(0x0000, 0x0fff).mirror(0x1000).rom();
	map(0x2000, 0x27ff).ram(); // 6116
}

void carjmbre_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x20, 0x21).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x22, 0x22).nopw(); // bdir/bc2/bc1 1/0/1 inactive write
	map(0x24, 0x24).r("ay1", FUNC(ay8910_device::data_r));
	map(0x30, 0x31).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x32, 0x32).nopw(); // bdir/bc2/bc1 1/0/1 inactive write
	map(0x34, 0x34).r("ay2", FUNC(ay8910_device::data_r));
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( carjmbre )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW:4,5")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x18, "Free")
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x00, "10k then every 100k" )
	PORT_DIPSETTING(    0x20, "20k then every 100k" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

static const gfx_layout carjmbre_spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	2,
	{ RGN_FRAC(2,4), RGN_FRAC(0,4) },
	{ STEP8(0,1), STEP8(256*16*8,1) },
	{ STEP16(0,8) },
	16*8
};

static GFXDECODE_START( gfx_carjmbre )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x2_planar, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, carjmbre_spritelayout, 0, 16 )
GFXDECODE_END


void carjmbre_state::carjmbre(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(18'432'000)/6);
	m_maincpu->set_addrmap(AS_PROGRAM, &carjmbre_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(carjmbre_state::vblank_nmi));

	Z80(config, m_audiocpu, XTAL(18'432'000)/6/2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &carjmbre_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &carjmbre_state::sound_io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(carjmbre_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_carjmbre);
	PALETTE(config, m_palette, FUNC(carjmbre_state::carjmbre_palette), 64);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	AY8910(config, "ay1", XTAL(18'432'000)/6/2).add_route(ALL_OUTPUTS, "mono", 0.25);

	AY8910(config, "ay2", XTAL(18'432'000)/6/2).add_route(ALL_OUTPUTS, "mono", 0.25);
}



/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( carjmbre )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c1",    0x0000, 0x1000, CRC(62b21739) SHA1(710e5c52f27603aa8f864f6f28d7272f21271d60) )
	ROM_LOAD( "c2",    0x1000, 0x1000, CRC(9ab1a0fa) SHA1(519cf67b98e62b2b42232788ba01ab6637880afc) )
	ROM_LOAD( "c3",    0x2000, 0x1000, CRC(bb29e100) SHA1(93e3cfcf7f8b0b36327f402d9a64c04c3b2c7549) )
	ROM_LOAD( "c4",    0x3000, 0x1000, CRC(c63d8f97) SHA1(9f08fd1cd24a1fb4011864c06580985e009d9af4) )
	ROM_LOAD( "c5",    0x4000, 0x1000, CRC(4d593942) SHA1(30cc649a4be3d7f3705f55d8d0dadb0b63d59ec9) )
	ROM_LOAD( "c6",    0x5000, 0x1000, CRC(fb576963) SHA1(5bf5c54a7c12aa55272629c12b414bf49cda0f1f) )
	ROM_LOAD( "c7",    0x6000, 0x1000, CRC(2b8c4511) SHA1(428a48d6b14455d66720a115bc5f35293dc50de7) )
	ROM_LOAD( "c8",    0x7000, 0x1000, CRC(51cc22a7) SHA1(f614368bfee04f084c70bf145801ac46e5631acb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c15",   0x0000, 0x1000, CRC(7d7779d1) SHA1(f8f5246be4cc9632076d3330fc3d3343b911dfee) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "c10",   0x0000, 0x1000, CRC(75ddbe56) SHA1(5e1363967a822265618793ccb74bf3ef5e0e00b5) )
	ROM_LOAD( "c9",    0x1000, 0x1000, CRC(2accb821) SHA1(ce2804536fc1abd3377dc864c8c9976ca28c1b6e) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "c11",   0x0000, 0x1000, CRC(d90cd126) SHA1(7ee110cf19b45ee654016ba0ce92f3db6ea2ed92) )
	ROM_LOAD( "c12",   0x1000, 0x1000, CRC(b3bb39d7) SHA1(89c901be6fae2356ce4d2653e94bf28d6bcf41fe) )
	ROM_LOAD( "c13",   0x2000, 0x1000, CRC(3004010b) SHA1(00d5d2185014159112eb90d8ed50092a3b4ab664) )
	ROM_LOAD( "c14",   0x3000, 0x1000, CRC(fb5f0d31) SHA1(7a27af91efc836bb48c6ed3b283b7c5f7b31c4b5) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "c.d19", 0x0000, 0x0020, CRC(220bceeb) SHA1(46b9f867d014596e2aa7503f104dc721965f0ed5) )
	ROM_LOAD( "c.d18", 0x0020, 0x0020, CRC(7b9ed1b0) SHA1(ec5e1f56e5a2fc726083866c08ac0e1de0ed6ace) )
ROM_END

} // anonymous namespace


GAME( 1983, carjmbre, 0, carjmbre, carjmbre, carjmbre_state, empty_init, ROT90, "Omori Electric Co., Ltd.", "Car Jamboree", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_GRAPHICS )
