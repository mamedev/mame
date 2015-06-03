// license:BSD-3-Clause
// copyright-holders:Hau, Chack'n
/***************************************************************************


Monkey Magic
(c)1979 Nintendo


--- Team Japump!!! ---
Dumped by Chack'n
Written by Hau
01/Jun/2015
***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "sound/samples.h"

#define USE_SAMPLES     (1)


class mnkymgic_state : public driver_device
{
public:
	mnkymgic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
#if (USE_SAMPLES)
		m_samples(*this, "samples")
#endif
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_videoram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
#if (USE_SAMPLES)
	required_device<samples_device> m_samples;
#endif

	/* misc */
	tilemap_t *m_bg_tilemap;
	UINT8 m_obj_ball_x;
	UINT8 m_obj_ball_y;
	UINT8 m_color_map;
	UINT8 m_sound_state;

	DECLARE_WRITE8_MEMBER(mnkymgic_videoram_w);
	DECLARE_WRITE8_MEMBER(mnkymgic_obj_ball_x_w);
	DECLARE_WRITE8_MEMBER(mnkymgic_obj_ball_y_w);
	DECLARE_WRITE8_MEMBER(mnkymgic_colormap_w);
	DECLARE_WRITE8_MEMBER(mnkymgic_audio_w);
	virtual void machine_start();
	virtual void machine_reset();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	DECLARE_PALETTE_INIT(mnkymgic);
    void draw_ball(bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_mnkymgic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*************************************
 *
 *  Video system
 *
 *************************************/

PALETTE_INIT_MEMBER(mnkymgic_state, mnkymgic)
{
	offs_t i;

	for (i = 0; i < palette.entries() / 2; i++)
	{
		palette.set_pen_color((i * 2) + 0, rgb_t::black);
		palette.set_pen_color((i * 2) + 1, rgb_t(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2)));
	}
}


TILE_GET_INFO_MEMBER(mnkymgic_state::get_bg_tile_info)
{
	const UINT8 *prom = memregion("proms")->base();

	int code = m_videoram[tile_index];
	int color = (prom[code + m_color_map] ^ 0xff) & 0x07;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}


void mnkymgic_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mnkymgic_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 12, 32, 16);
}

WRITE8_MEMBER(mnkymgic_state::mnkymgic_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(mnkymgic_state::mnkymgic_obj_ball_x_w)
{
	m_obj_ball_x = data;
}

WRITE8_MEMBER(mnkymgic_state::mnkymgic_obj_ball_y_w)
{
	m_obj_ball_y = data;
}

WRITE8_MEMBER(mnkymgic_state::mnkymgic_colormap_w)
{
	m_color_map = (data & 0x08) << 4;
}


void mnkymgic_state::draw_ball(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	offs_t offs_x, offs_y;

	if (m_obj_ball_x > 0x02 && m_obj_ball_x < 0xff && m_obj_ball_y > 0x02 && m_obj_ball_y < 0xff)
	{
		int ball_y = (((m_obj_ball_y & 0xf0) >> 4) * 12) + (m_obj_ball_y & 0x0f);

		for (offs_y = 0; offs_y < 4; offs_y++)
		{
			for (offs_x = 0; offs_x < 4; offs_x++)
			{
				bitmap.pix16(ball_y-offs_y, m_obj_ball_x-offs_x) = m_palette->pen(0x0f);;
			}
		}
	}
}


UINT32 mnkymgic_state::screen_update_mnkymgic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_ball(bitmap, cliprect);

	return 0;
}


/*************************************
 *
 *  Audio system
 *
 *************************************/

#define SAMPLE_SOUND1		0
#define SAMPLE_SOUND2_1		1
#define SAMPLE_SOUND2_2		2
#define SAMPLE_SOUND3		3
#define SAMPLE_SOUND4		4
#define SAMPLE_SOUND5		5
#define SAMPLE_SOUND6_1		6
#define SAMPLE_SOUND6_2		7

#define CHANNEL_SOUND1		0


#if (USE_SAMPLES)
WRITE8_MEMBER(mnkymgic_state::mnkymgic_audio_w)
{
	if (data != m_sound_state)
	{
		if (~data & 0x80)
		{
			switch (m_sound_state)
			{
				case 0xff:
					m_samples->start(CHANNEL_SOUND1, SAMPLE_SOUND4);
					break;

				case 0xfe:
					m_samples->start(CHANNEL_SOUND1, SAMPLE_SOUND3);
					break;

				case 0xfd:
					m_samples->start(CHANNEL_SOUND1, SAMPLE_SOUND5);
					break;

				case 0xfc:
					m_samples->start(CHANNEL_SOUND1, SAMPLE_SOUND2_1);
					break;

				case 0xfb:
					m_samples->start(CHANNEL_SOUND1, SAMPLE_SOUND2_2);
					break;

				case 0xfa:
					m_samples->start(CHANNEL_SOUND1, SAMPLE_SOUND6_1);
					break;

				case 0xf9:
					m_samples->start(CHANNEL_SOUND1, SAMPLE_SOUND6_2);
					break;

				case 0xf8:
					m_samples->start(CHANNEL_SOUND1, SAMPLE_SOUND1);
					break;

				default:
					break;
			}
		}
	}

	m_sound_state = data;
}


static const char *const mnkymgic_sample_names[] =
{
	"*mnkymgic",
	"1",
	"2",
	"2-2",
	"3",
	"4",
	"5",
	"6",
	"6-2",
	0
};

#endif


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, mnkymgic_state )
	AM_RANGE(0x0000, 0x13ff) AM_ROM
	AM_RANGE(0x2000, 0x21ff) AM_RAM
	AM_RANGE(0x3000, 0x31ff) AM_RAM_WRITE(mnkymgic_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x8002, 0x8002) AM_WRITE(mnkymgic_obj_ball_x_w)
	AM_RANGE(0x8003, 0x8003) AM_WRITE(mnkymgic_obj_ball_y_w)
	AM_RANGE(0x8004, 0x8004) AM_READ_PORT("VBLANK")
ADDRESS_MAP_END


/*************************************
 *
 *  Port handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_io_map, AS_IO, 8, mnkymgic_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x80) AM_WRITE(mnkymgic_colormap_w)
	AM_RANGE(0x81, 0x81) AM_WRITE(mnkymgic_audio_w)
	AM_RANGE(0x85, 0x85) AM_READ_PORT("PADDLE")
	AM_RANGE(0x86, 0x86) AM_READ_PORT("INPUTS")
	AM_RANGE(0x87, 0x87) AM_READ_PORT("DSW")
ADDRESS_MAP_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( mnkymgic )
	PORT_START("VBLANK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PADDLE")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_CENTERDELTA(0)
INPUT_PORTS_END


/*************************************
 *
 *  Graphics Layouts
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,12,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 3, 2, 1, 0, 7, 6, 5, 4 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16
};

static GFXDECODE_START( mnkymgic )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout, 0, 8 )
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void mnkymgic_state::machine_start()
{
	/* Set up save state */
	save_item(NAME(m_obj_ball_x));
	save_item(NAME(m_obj_ball_y));
	save_item(NAME(m_color_map));
	save_item(NAME(m_sound_state));
}

void mnkymgic_state::machine_reset()
{
	m_obj_ball_x = 0;
	m_obj_ball_y = 0;
	m_sound_state = 0x80;
}

static MACHINE_CONFIG_START( mnkymgic, mnkymgic_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8085A, 6144000/2)		/* 3.072MHz ? */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_io_map)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mnkymgic)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 0, 191)
	MCFG_SCREEN_UPDATE_DRIVER(mnkymgic_state, screen_update_mnkymgic)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(mnkymgic_state, mnkymgic)

#if (USE_SAMPLES)
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_NAMES(mnkymgic_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
#endif
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( mnkymgic )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1a.bin",  0x0000, 0x0400, CRC(ec772e2e) SHA1(7efc1bbb24b2ed73c518aea1c4ef4b9a93034e31) )
	ROM_LOAD( "2a.bin",  0x0400, 0x0400, CRC(e5d482ca) SHA1(208b808e9208bb6f5f5f89ffbeb5a885be33733a) )
	ROM_LOAD( "3a.bin",  0x0800, 0x0400, CRC(e8d38deb) SHA1(d7384234fb47e4b1d0421f58571fa748662b05f5) )
	ROM_LOAD( "4a.bin",  0x0c00, 0x0400, CRC(3048bd6c) SHA1(740051589f6ba44b2ee68edf76a3177bb973d78e) )
	ROM_LOAD( "5a.bin",  0x1000, 0x0400, CRC(2cab8f04) SHA1(203a3c005f18f968cd14c972bbb9fd7e0fc3b670) )

	ROM_REGION( 0x600, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "6h.bin",  0x0000, 0x0200, CRC(b6321b6f) SHA1(06611f7419d2982e006a3e81b79677e59e194f38) )		/* tilemap */
	ROM_LOAD( "7h.bin",  0x0200, 0x0200, CRC(9ec0e82c) SHA1(29983f690a1b6134bb1983921f42c14898788095) )
	ROM_LOAD( "6j.bin",  0x0400, 0x0200, CRC(7ce83302) SHA1(1870610ff07ab11622e183e04e3fce29328ff291) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "7j.bin",  0x0000, 0x0200, CRC(b7eb8e1c) SHA1(b65a8efb88668dcf1c1d00e31a9b15a67c2972c8) )		/* tile palettes selector */
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME(1979, mnkymgic, 0, mnkymgic, mnkymgic, driver_device, 0, ROT270, "Nintendo", "Monkey Magic (Japan)", GAME_SUPPORTS_SAVE | GAME_IMPERFECT_SOUND )
