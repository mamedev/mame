// license:BSD-3-Clause
// copyright-holders: Uki

/*****************************************************************************

Dr. Micro (c) 1983 Sanritsu

        driver by Uki

Quite similar to Appoooh

*****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/msm5205.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class drmicro_state : public driver_device
{
public:
	drmicro_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram%u", 0U),
		m_adpcm_rom(*this, "adpcm"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_msm(*this, "msm")
	{ }

	void drmicro(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_region_ptr<uint8_t> m_adpcm_rom;

	// video-related
	tilemap_t *m_bg[2]{};
	uint8_t m_flipscreen = 0;

	// misc
	uint8_t m_nmi_enable = 0;
	uint16_t m_pcm_adr = 0;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<msm5205_device> m_msm;

	void nmi_enable_w(uint8_t data);
	void pcm_set_w(uint8_t data);
	template <uint8_t Which> void videoram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	template <uint8_t Which> void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	void pcm_w(int state);
	void prg_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


/****************************************************************************/

template <uint8_t Which>
void drmicro_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[Which][offset] = data;

	m_bg[Which]->mark_tile_dirty((offset & 0x3ff));
}


/****************************************************************************/

template <uint8_t Which>
TILE_GET_INFO_MEMBER(drmicro_state::get_bg_tile_info)
{
	int code = m_videoram[Which][tile_index];
	int col = m_videoram[Which][tile_index + 0x400];

	code += (col & 0xc0) << 2;
	int const flags = ((col & 0x20) ? TILEMAP_FLIPY : 0) | ((col & 0x10) ? TILEMAP_FLIPX : 0);
	col &= 0x0f;

	tileinfo.set(Which, code, col, flags);
}

/****************************************************************************/

void drmicro_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x20;

	for (int i = 0; i < 0x200; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}

void drmicro_state::video_start()
{
	m_bg[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(drmicro_state::get_bg_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(drmicro_state::get_bg_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg[1]->set_transparent_pen(0);
}

template <uint8_t Which>
void drmicro_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0x00; offs < 0x20; offs += 4)
	{
		int x = m_videoram[Which][offs + 3];
		int y = m_videoram[Which][offs + 0];
		int const attr = m_videoram[Which][offs + 2];
		int chr = m_videoram[Which][offs + 1];

		int const fx = (chr & 0x01) ^ m_flipscreen;
		int const fy = ((chr & 0x02) >> 1) ^ m_flipscreen;

		chr = (chr >> 2) | (attr & 0xc0);

		int const col = (attr & 0x0f) + 0x00;

		if (!m_flipscreen)
			y = (240 - y) & 0xff;
		else
			x = (240 - x) & 0xff;

		m_gfxdecode->gfx(2 + Which)->transpen(bitmap, cliprect,
				chr,
				col,
				fx, fy,
				x, y, 0);

		if (x > 240)
		{
			m_gfxdecode->gfx(2 + Which)->transpen(bitmap, cliprect,
					chr,
					col,
					fx, fy,
					x - 256, y, 0);
		}
	}
}

uint32_t drmicro_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_bg[1]->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites<0>(bitmap, cliprect);
	draw_sprites<1>(bitmap, cliprect);

	return 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

INTERRUPT_GEN_MEMBER(drmicro_state::interrupt)
{
	if (m_nmi_enable)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void drmicro_state::nmi_enable_w(uint8_t data)
{
	m_nmi_enable = data & 1;
	m_flipscreen = (data & 2) ? 1 : 0;
	flip_screen_set(data & 2);

	// bit2, 3 unknown
}


void drmicro_state::pcm_w(int state)
{
	int data = m_adpcm_rom[m_pcm_adr / 2];

	if (data != 0x70) // ??
	{
		if (~m_pcm_adr & 1)
			data >>= 4;

		m_msm->data_w(data & 0x0f);
		m_msm->reset_w(0);

		m_pcm_adr = (m_pcm_adr + 1) & 0x7fff;
	}
	else
		m_msm->reset_w(1);
}

void drmicro_state::pcm_set_w(uint8_t data)
{
	m_pcm_adr = ((data & 0x3f) << 9);
	pcm_w(1);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void drmicro_state::prg_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe7ff).ram().w(FUNC(drmicro_state::videoram_w<1>)).share(m_videoram[1]);
	map(0xe800, 0xefff).ram().w(FUNC(drmicro_state::videoram_w<0>)).share(m_videoram[0]);
	map(0xf000, 0xffff).ram();
}

void drmicro_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("P1").w("sn1", FUNC(sn76496_device::write));
	map(0x01, 0x01).portr("P2").w("sn2", FUNC(sn76496_device::write));
	map(0x02, 0x02).w("sn3", FUNC(sn76496_device::write));
	map(0x03, 0x03).portr("DSW1").w(FUNC(drmicro_state::pcm_set_w));
	map(0x04, 0x04).portr("DSW2").w(FUNC(drmicro_state::nmi_enable_w));
	map(0x05, 0x05).noprw(); // unused? / watchdog?
}

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( drmicro )
	PORT_START("P1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_START("P2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x00, "30000 100000" )
	PORT_DIPSETTING(    0x08, "50000 150000" )
	PORT_DIPSETTING(    0x10, "70000 200000" )
	PORT_DIPSETTING(    0x18, "100000 300000" )
	PORT_SERVICE_DIPLOC(  0x20, IP_ACTIVE_HIGH, "SW1:!6" )  // Service Mode shows as "X"
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:!1,!2,!3")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_HIGH, "SW2:!4" ) // Service Mode shows as "X"
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_HIGH, "SW2:!5" ) // Service Mode shows as "X"
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_HIGH, "SW2:!6" ) // Service Mode shows as "X"
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_HIGH, "SW2:!7" ) // Service Mode shows as "X"
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_HIGH, "SW2:!8" ) // Service Mode shows as "X"
INPUT_PORTS_END

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout spritelayout4 =
{
	16,16,
	0x100,
	2,
	{0,0x2000*8},
	{STEP8(7,-1),STEP8(71,-1)},
	{STEP8(0,8),STEP8(128,8)},
	8*8*4
};

static const gfx_layout spritelayout8 =
{
	16,16,
	0x100,
	3,
	{0x2000*16,0x2000*8,0},
	{STEP8(7,-1),STEP8(71,-1)},
	{STEP8(0,8),STEP8(128,8)},
	8*8*4
};

static const gfx_layout charlayout4 =
{
	8,8,
	0x400,
	2,
	{0,0x2000*8},
	{STEP8(7,-1)},
	{STEP8(0,8)},
	8*8*1
};

static const gfx_layout charlayout8 =
{
	8,8,
	0x400,
	3,
	{0x2000*16,0x2000*8,0},
	{STEP8(7,-1)},
	{STEP8(0,8)},
	8*8*1
};

static GFXDECODE_START( gfx_drmicro )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout4,     0, 64 ) // tiles
	GFXDECODE_ENTRY( "gfx2", 0x0000, charlayout8,   256, 32 ) // tiles
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout4,   0, 64 ) // sprites
	GFXDECODE_ENTRY( "gfx2", 0x0000, spritelayout8, 256, 32 ) // sprites
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void drmicro_state::machine_start()
{
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_pcm_adr));
	save_item(NAME(m_flipscreen));
}

void drmicro_state::machine_reset()
{
	m_nmi_enable = 0;
	m_pcm_adr = 0;
	m_flipscreen = 0;
}


void drmicro_state::drmicro(machine_config &config)
{
	static constexpr XTAL MCLK = 18.432_MHz_XTAL;

	// basic machine hardware
	Z80(config, m_maincpu, MCLK / 6); // 3.072MHz?
	m_maincpu->set_addrmap(AS_PROGRAM, &drmicro_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &drmicro_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(drmicro_state::interrupt));

	config.set_maximum_quantum(attotime::from_hz(60));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(drmicro_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_drmicro);
	PALETTE(config, m_palette, FUNC(drmicro_state::palette), 512, 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SN76496(config, "sn1", MCLK / 4).add_route(ALL_OUTPUTS, "mono", 0.50);
	SN76496(config, "sn2", MCLK / 4).add_route(ALL_OUTPUTS, "mono", 0.50);
	SN76496(config, "sn3", MCLK / 4).add_route(ALL_OUTPUTS, "mono", 0.50);

	MSM5205(config, m_msm, 384_kHz_XTAL);
	m_msm->vck_legacy_callback().set(FUNC(drmicro_state::pcm_w));   // IRQ handler
	m_msm->set_prescaler_selector(msm5205_device::S64_4B);  // 6 KHz
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.75);
}

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( drmicro )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dm-00.13b", 0x0000,  0x2000, CRC(270f2145) SHA1(1557428387e2c0f711c676a13a763c8d48aa497b) )
	ROM_LOAD( "dm-01.14b", 0x2000,  0x2000, CRC(bba30c80) SHA1(a084429fad58fa6348936084652235d5f55e3b89) )
	ROM_LOAD( "dm-02.15b", 0x4000,  0x2000, CRC(d9e4ca6b) SHA1(9fb6d1d6b45628891deae389cf1d142332b110ba) )
	ROM_LOAD( "dm-03.13d", 0x6000,  0x2000, CRC(b7bcb45b) SHA1(61035afc642bac2e1c56c36c188bed4e1949523f) )
	ROM_LOAD( "dm-04.14d", 0x8000,  0x2000, CRC(071db054) SHA1(75929b7692bebf2246fa84581b6d1eedb02c9aba) )
	ROM_LOAD( "dm-05.15d", 0xa000,  0x2000, CRC(f41b8d8a) SHA1(802830f3f0362ec3df257f31dc22390e8ae4207c) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "dm-23.5l",  0x0000,  0x2000, CRC(279a76b8) SHA1(635650621bdce5873bb5faf64f8352149314e784) )
	ROM_LOAD( "dm-24.5n",  0x2000,  0x2000, CRC(ee8ed1ec) SHA1(7afc05c73186af9fe3d3f3ce13412c8ee560b146) )

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "dm-20.4a",  0x0000,  0x2000, CRC(6f5dbf22) SHA1(41ef084336e2ebb1016b28505dcb43483e37a0de) )
	ROM_LOAD( "dm-21.4c",  0x2000,  0x2000, CRC(8b17ff47) SHA1(5bcc14489ea1d4f1fe8e51c24a72a8e787ab8159) )
	ROM_LOAD( "dm-22.4d",  0x4000,  0x2000, CRC(84daf771) SHA1(d187debcca59ceab6cd696be246370120ee575c6) )

	ROM_REGION( 0x04000, "adpcm", 0 )
	ROM_LOAD( "dm-40.12m",  0x0000,  0x2000, CRC(3d080af9) SHA1(f9527fae69fe3ca0762024ac4a44b1f02fbee66a) )
	ROM_LOAD( "dm-41.13m",  0x2000,  0x2000, CRC(ddd7bda2) SHA1(bbe9276cb47fa3e82081d592522640e04b4a9223) )

	ROM_REGION( 0x00220, "proms", 0 )
	ROM_LOAD( "dm-62.9h", 0x0000,  0x0020, CRC(e3e36eaf) SHA1(5954400190e587a20cad60f5829f4bddc85ea526) )
	ROM_LOAD( "dm-61.4m", 0x0020,  0x0100, CRC(0dd8e365) SHA1(cbd43a2d4af053860932af32ca5e13bef728e38a) )
	ROM_LOAD( "dm-60.6e", 0x0120,  0x0100, CRC(540a3953) SHA1(bc65388a1019dadf8c71705e234763f5c735e282) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1983, drmicro, 0, drmicro, drmicro, drmicro_state, empty_init, ROT270, "Sanritsu", "Dr. Micro", MACHINE_SUPPORTS_SAVE )
