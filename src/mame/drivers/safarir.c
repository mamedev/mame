// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/****************************************************************************

    Safari Rally hardware

    driver by Zsolt Vasvari

    Games Supported:
        * Safari Rally (Japan Ver.)
          (c)1979 Shin Nihon Kikaku

    Known issues/to-do's:
        * SN76477 discrete sound

    Notes:
        * This hardware is a precursor to Phoenix
        * Export license to Taito in 1980

          ----------------------------------

          CPU board

          76477        18MHz

                        8080

          ----------------------------------

          Video board


           RL07  2114
                 2114
                 2114
                 2114
                 2114           RL01 RL02
                 2114           RL03 RL04
                 2114           RL05 RL06
           RL08  2114

           11MHz

          ----------------------------------

Dumped by Chack'n
27/Jun/2009
modified by Hau
08/09/2009
****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "sound/samples.h"

class safarir_state : public driver_device
{
public:
	safarir_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_ram(*this, "ram"),
		m_bg_scroll(*this, "bg_scroll"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_shared_ptr<UINT8> m_ram;
	required_shared_ptr<UINT8> m_bg_scroll;
	required_device<gfxdecode_device> m_gfxdecode;

	UINT8 *m_ram_1;
	UINT8 *m_ram_2;
	UINT8 m_ram_bank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT8 m_port_last;
	UINT8 m_port_last2;

	DECLARE_WRITE8_MEMBER(ram_w);
	DECLARE_READ8_MEMBER(ram_r);
	DECLARE_WRITE8_MEMBER(ram_bank_w);
	DECLARE_WRITE8_MEMBER(safarir_audio_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start();
	virtual void video_start();
	DECLARE_PALETTE_INIT(safarir);
	UINT32 screen_update_safarir(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*************************************
 *
 *  RAM banking
 *
 *************************************/

WRITE8_MEMBER(safarir_state::ram_w)
{
	if (m_ram_bank)
		m_ram_2[offset] = data;
	else
		m_ram_1[offset] = data;

	((offset & 0x0400) ? m_bg_tilemap : m_fg_tilemap)->mark_tile_dirty(offset & 0x03ff);
}


READ8_MEMBER(safarir_state::ram_r)
{
	return m_ram_bank ? m_ram_2[offset] : m_ram_1[offset];
}


WRITE8_MEMBER(safarir_state::ram_bank_w)
{
	m_ram_bank = data & 0x01;

	machine().tilemap().mark_all_dirty();
}



/*************************************
 *
 *  Video system
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 chars */
	128,    /* 128 characters */
	1,      /* 1 bit per pixel */
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};


static GFXDECODE_START( safarir )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout, 0, 8 )
GFXDECODE_END


PALETTE_INIT_MEMBER(safarir_state, safarir)
{
	int i;

	for (i = 0; i < palette.entries() / 2; i++)
	{
		palette.set_pen_color((i * 2) + 0, rgb_t::black);
		palette.set_pen_color((i * 2) + 1, rgb_t(pal1bit(i >> 2), pal1bit(i >> 1), pal1bit(i >> 0)));
	}
}

TILE_GET_INFO_MEMBER(safarir_state::get_bg_tile_info)
{
	int color;
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT8 code = ram_r(space,tile_index | 0x400);

	if (code & 0x80)
		color = 6;  /* yellow */
	else
	{
		color = ((~tile_index & 0x04) >> 2) | ((tile_index & 0x04) >> 1);

		if (~tile_index & 0x100)
			color |= ((tile_index & 0xc0) == 0x80) ? 1 : 0;
		else
			color |= (tile_index & 0xc0) ? 1 : 0;
	}

	SET_TILE_INFO_MEMBER(0, code & 0x7f, color, 0);
}


TILE_GET_INFO_MEMBER(safarir_state::get_fg_tile_info)
{
	int color, flags;
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT8 code = ram_r(space,tile_index);

	if (code & 0x80)
		color = 7;  /* white */
	else
		color = (~tile_index & 0x04) | ((tile_index >> 1) & 0x03);

	flags = ((tile_index & 0x1f) >= 0x03) ? 0 : TILE_FORCE_LAYER0;

	SET_TILE_INFO_MEMBER(1, code & 0x7f, color, flags);
}


void safarir_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(safarir_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(safarir_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
}


UINT32 safarir_state::screen_update_safarir(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, *m_bg_scroll);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

/*************************************
 *
 *  Audio system
 *
 *************************************/

#define SAMPLE_SOUND1_1     0
#define SAMPLE_SOUND1_2     1
#define SAMPLE_SOUND2       2
#define SAMPLE_SOUND3       3
#define SAMPLE_SOUND4_1     4
#define SAMPLE_SOUND4_2     5
#define SAMPLE_SOUND5_1     6
#define SAMPLE_SOUND5_2     7
#define SAMPLE_SOUND6       8
#define SAMPLE_SOUND7       9
#define SAMPLE_SOUND8       10

#define CHANNEL_SOUND1      0
#define CHANNEL_SOUND2      1
#define CHANNEL_SOUND3      2
#define CHANNEL_SOUND4      3
#define CHANNEL_SOUND5      4
#define CHANNEL_SOUND6      5


WRITE8_MEMBER(safarir_state::safarir_audio_w)
{
	UINT8 rising_bits = data & ~m_port_last;

	if (rising_bits == 0x12) m_samples->start(CHANNEL_SOUND1, SAMPLE_SOUND1_1);
	if (rising_bits == 0x02) m_samples->start(CHANNEL_SOUND1, SAMPLE_SOUND1_2);
	if (rising_bits == 0x95) m_samples->start(CHANNEL_SOUND1, SAMPLE_SOUND6);

	if (rising_bits == 0x04 && (data == 0x15 || data ==0x16)) m_samples->start(CHANNEL_SOUND2, SAMPLE_SOUND2);

	if (data == 0x5f && (rising_bits == 0x49 || rising_bits == 0x5f)) m_samples->start(CHANNEL_SOUND3, SAMPLE_SOUND3, true);
	if (data == 0x00 || rising_bits == 0x01) m_samples->stop(CHANNEL_SOUND3);

	if (data == 0x13)
	{
		if ((rising_bits == 0x13 && m_port_last != 0x04) || (rising_bits == 0x01 && m_port_last == 0x12))
		{
			m_samples->start(CHANNEL_SOUND4, SAMPLE_SOUND7);
		}
		else if (rising_bits == 0x03 && m_port_last2 == 0x15 && !m_samples->playing(CHANNEL_SOUND4))
		{
			m_samples->start(CHANNEL_SOUND4, SAMPLE_SOUND4_1);
		}
	}
	if (data == 0x53 && m_port_last == 0x55) m_samples->start(CHANNEL_SOUND4, SAMPLE_SOUND4_2);

	if (data == 0x1f && rising_bits == 0x1f) m_samples->start(CHANNEL_SOUND5, SAMPLE_SOUND5_1);
	if (data == 0x14 && (rising_bits == 0x14 || rising_bits == 0x04)) m_samples->start(CHANNEL_SOUND5, SAMPLE_SOUND5_2);

	if (data == 0x07 && rising_bits == 0x07 && !m_samples->playing(CHANNEL_SOUND6))
		m_samples->start(CHANNEL_SOUND6, SAMPLE_SOUND8);

	m_port_last2 = m_port_last;
	m_port_last = data;
}


static const char *const safarir_sample_names[] =
{
	"*safarir",
	"sound1-1",
	"sound1-2",
	"sound2",
	"sound3",
	"sound4-1",
	"sound4-2",
	"sound5-1",
	"sound5-2",
	"sound6",
	"sound7",
	"sound8",
	0
};


static MACHINE_CONFIG_FRAGMENT( safarir_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(6)
	MCFG_SAMPLES_NAMES(safarir_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  Machine start
 *
 *************************************/

void safarir_state::machine_start()
{
	m_ram_1 = auto_alloc_array(machine(), UINT8, m_ram.bytes());
	m_ram_2 = auto_alloc_array(machine(), UINT8, m_ram.bytes());
	m_port_last = 0;
	m_port_last2 = 0;

	/* setup for save states */
	save_pointer(NAME(m_ram_1), m_ram.bytes());
	save_pointer(NAME(m_ram_2), m_ram.bytes());
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_port_last));
	save_item(NAME(m_port_last2));
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, safarir_state )
	AM_RANGE(0x0000, 0x17ff) AM_ROM
	AM_RANGE(0x2000, 0x27ff) AM_READWRITE(ram_r, ram_w) AM_SHARE("ram")
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x03ff) AM_READNOP AM_WRITE(ram_bank_w)
	AM_RANGE(0x2c00, 0x2cff) AM_MIRROR(0x03ff) AM_READNOP AM_WRITEONLY AM_SHARE("bg_scroll")
	AM_RANGE(0x3000, 0x30ff) AM_MIRROR(0x03ff) AM_WRITE(safarir_audio_w)    /* goes to SN76477 */
	AM_RANGE(0x3400, 0x3400) AM_MIRROR(0x03ff) AM_WRITENOP  /* cleared at the beginning */
	AM_RANGE(0x3800, 0x38ff) AM_MIRROR(0x03ff) AM_READ_PORT("INPUTS") AM_WRITENOP
	AM_RANGE(0x3c00, 0x3cff) AM_MIRROR(0x03ff) AM_READ_PORT("DSW") AM_WRITENOP
ADDRESS_MAP_END



/*************************************
 *
 *  Port definition
 *
 *************************************/

static INPUT_PORTS_START( safarir )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x04, "Acceleration Rate" )
	PORT_DIPSETTING(    0x00, "Slowest" )
	PORT_DIPSETTING(    0x04, "Slow" )
	PORT_DIPSETTING(    0x08, "Fast" )
	PORT_DIPSETTING(    0x0c, "Fastest" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPSETTING(    0x20, "5000" )
	PORT_DIPSETTING(    0x40, "7000" )
	PORT_DIPSETTING(    0x60, "9000" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( safarir, safarir_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080A, XTAL_18MHz/12)  /* 1.5 MHz ? */
	MCFG_CPU_PROGRAM_MAP(main_map)

	/* video hardware */
	MCFG_PALETTE_ADD("palette", 2*8)
	MCFG_PALETTE_INIT_OWNER(safarir_state, safarir)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", safarir)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 26*8-1)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DRIVER(safarir_state, screen_update_safarir)
	MCFG_SCREEN_PALETTE("palette")

	/* audio hardware */
	MCFG_FRAGMENT_ADD(safarir_audio)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition
 *
 *************************************/

/*
Safari Rally (Japan Ver.)
(c)1979 Shin Nihon Kikaku

----------------------------------------
Top
RLO70002
RLN00001
CPU:M5L8080AP
SND:SN76477N
18.000MHz
----------------------------------------
Bottom
RLO70003
RLN00002
11.000MHz
----------------------------------------
RL-01.9      [cf7703c9]
RL-02.1      [1013ecd3]
RL-03.10     [84545894]
RL-04.2      [5dd12f96]
RL-05.11     [935ed469]
RL-06.3      [24c1cd42]

RL-07.40     [ba525203]
RL-08.43     [d6a50aac]

--- Team Japump!!! ---
*/

ROM_START( safarirj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rl-01.9",  0x0000, 0x0400, CRC(cf7703c9) SHA1(b4182df9332b355edaa518462217a6e31e1c07b2) )
	ROM_LOAD( "rl-02.1",  0x0400, 0x0400, CRC(1013ecd3) SHA1(2fe367db8ca367b36c5378cb7d5ff918db243c78) )
	ROM_LOAD( "rl-03.10", 0x0800, 0x0400, CRC(84545894) SHA1(377494ceeac5ad58b70f77b2b27b609491cb7ffd) )
	ROM_LOAD( "rl-04.2",  0x0c00, 0x0400, CRC(5dd12f96) SHA1(a80ac0705648f0807ea33e444fdbea450bf23f85) )
	ROM_LOAD( "rl-05.11", 0x1000, 0x0400, CRC(935ed469) SHA1(052a59df831dcc2c618e9e5e5fdfa47548550596) )
	ROM_LOAD( "rl-06.3",  0x1400, 0x0400, CRC(24c1cd42) SHA1(fe32ecea77a3777f8137ca248b8f371db37b8b85) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "rl-08.43", 0x0000, 0x0400, CRC(d6a50aac) SHA1(0a0c2cefc556e4e15085318fcac485b82bac2416) )

	ROM_REGION( 0x0400, "gfx2", 0 )
	ROM_LOAD( "rl-07.40", 0x0000, 0x0400, CRC(ba525203) SHA1(1c261cc1259787a7a248766264fefe140226e465) )
ROM_END

ROM_START( safarir ) // Taito PCB, labels are the same as Japan ver.
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rl-01.9",  0x0000, 0x0400, CRC(cf7703c9) SHA1(b4182df9332b355edaa518462217a6e31e1c07b2) )
	ROM_LOAD( "rl-02.1",  0x0400, 0x0400, CRC(1013ecd3) SHA1(2fe367db8ca367b36c5378cb7d5ff918db243c78) )
	ROM_LOAD( "rl-03.10", 0x0800, 0x0400, CRC(84545894) SHA1(377494ceeac5ad58b70f77b2b27b609491cb7ffd) )
	ROM_LOAD( "rl-04.2",  0x0c00, 0x0400, CRC(5dd12f96) SHA1(a80ac0705648f0807ea33e444fdbea450bf23f85) )
	ROM_LOAD( "rl-09.11", 0x1000, 0x0400, CRC(d066b382) SHA1(c82ec668f1ed2246c12a1371ee4a2c070f57a9c2) )
	ROM_LOAD( "rl-06.3",  0x1400, 0x0400, CRC(24c1cd42) SHA1(fe32ecea77a3777f8137ca248b8f371db37b8b85) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "rl-10.43", 0x0000, 0x0400, CRC(c04466c6) SHA1(da76afdfd22a7810de47376a9b23d3d538d77fdc) )

	ROM_REGION( 0x0400, "gfx2", 0 )
	ROM_LOAD( "rl-07.40", 0x0000, 0x0400, CRC(ba525203) SHA1(1c261cc1259787a7a248766264fefe140226e465) )
ROM_END



/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1979, safarir, 0,        safarir, safarir, driver_device, 0, ROT90, "SNK (Taito license)", "Safari Rally (World)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1979, safarirj, safarir, safarir, safarir, driver_device, 0, ROT90, "SNK", "Safari Rally (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
