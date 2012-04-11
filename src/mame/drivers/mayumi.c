/*****************************************************************************

    Kikiippatsu Mayumi-chan (c) 1988 Victory L.L.C.

    Driver by Uki

*****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "machine/nvram.h"

#define MCLK 10000000


class mayumi_state : public driver_device
{
public:
	mayumi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
//  UINT8 *    m_nvram;       // this currently uses generic nvram handlers

	/* video-related */
	tilemap_t *m_tilemap;

	/* misc */
	int m_int_enable;
	int m_input_sel;
	DECLARE_WRITE8_MEMBER(mayumi_videoram_w);
	DECLARE_WRITE8_MEMBER(bank_sel_w);
	DECLARE_WRITE8_MEMBER(input_sel_w);
	DECLARE_READ8_MEMBER(key_matrix_r);
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

static TILE_GET_INFO( get_tile_info )
{
	mayumi_state *state = machine.driver_data<mayumi_state>();
	int code = state->m_videoram.target()[tile_index] + (state->m_videoram.target()[tile_index + 0x800] & 0x1f) * 0x100;
	int col = (state->m_videoram.target()[tile_index + 0x1000] >> 3) & 0x1f;

	SET_TILE_INFO(0, code, col, 0);
}

static VIDEO_START( mayumi )
{
	mayumi_state *state = machine.driver_data<mayumi_state>();
	state->m_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
}

WRITE8_MEMBER(mayumi_state::mayumi_videoram_w)
{
	m_videoram.target()[offset] = data;
	m_tilemap->mark_tile_dirty(offset & 0x7ff);
}

static SCREEN_UPDATE_IND16( mayumi )
{
	mayumi_state *state = screen.machine().driver_data<mayumi_state>();
	state->m_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

/*************************************
 *
 *  Interrupt generator
 *
 *************************************/

static INTERRUPT_GEN( mayumi_interrupt )
{
	mayumi_state *state = device->machine().driver_data<mayumi_state>();

	if (state->m_int_enable)
		 device_set_input_line(device, 0, HOLD_LINE);
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(mayumi_state::bank_sel_w)
{
	int bank = BIT(data, 7) | (BIT(data, 6) << 1);

	memory_set_bank(machine(), "bank1", bank);

	m_int_enable = data & 1;

	flip_screen_set(data & 2);
}

WRITE8_MEMBER(mayumi_state::input_sel_w)
{
	m_input_sel = data;
}

READ8_MEMBER(mayumi_state::key_matrix_r)
{
	int p, i, ret;
	static const char *const keynames[2][5] =
			{
				{ "KEY5", "KEY6", "KEY7", "KEY8", "KEY9" },
				{ "KEY0", "KEY1", "KEY2", "KEY3", "KEY4" }
			};

	ret = 0xff;

	p = ~m_input_sel & 0x1f;

	for (i = 0; i < 5; i++)
	{
		if (BIT(p, i))
			ret &= input_port_read(machine(), keynames[offset][i]);
	}

	return ret;
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( mayumi_map, AS_PROGRAM, 8, mayumi_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xe000, 0xf7ff) AM_RAM_WRITE(mayumi_videoram_w) AM_SHARE("videoram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( mayumi_io_map, AS_IO, 8, mayumi_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x30, 0x30) AM_READ_PORT("IN0") AM_WRITE(bank_sel_w)
	AM_RANGE(0xc0, 0xc0) AM_WRITE(input_sel_w)
	AM_RANGE(0xc1, 0xc2) AM_READ(key_matrix_r)	// 0xc0-c3 8255ppi
	AM_RANGE(0xc3, 0xc3) AM_WRITENOP		// 0xc0-c3 8255ppi
	AM_RANGE(0xd0, 0xd1) AM_DEVREADWRITE_LEGACY("ymsnd", ym2203_r, ym2203_w)
ADDRESS_MAP_END

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mayumi )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 1-2" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 1-3" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x04, "Service Mode (DSW 1-6)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 1-7" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, "Unknown 1-8" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-1" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-2" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-3" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-5" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-6" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 2-7" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-8" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("KEY0")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY7")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY8")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY9")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW , IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE( 0x10, IP_ACTIVE_HIGH )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) // analyzer
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE4 ) // memory reset
INPUT_PORTS_END

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	8192,   /* 8192 characters */
	3,      /* 3 bits per pixel */
	{0x20000*8,0x10000*8,0},
	{STEP8(0,1)},
	{STEP8(0,8)},
	8*8
};

static GFXDECODE_START( mayumi )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout, 0, 32 )
GFXDECODE_END

/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

static const ym2203_interface ym2203_config =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_INPUT_PORT("DSW1"),
		DEVCB_INPUT_PORT("DSW2"),
		DEVCB_NULL,
		DEVCB_NULL
	},
	NULL
};

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( mayumi )
{
	mayumi_state *state = machine.driver_data<mayumi_state>();
	UINT8 *ROM = machine.region("maincpu")->base();

	memory_configure_bank(machine, "bank1", 0, 4, &ROM[0x10000], 0x4000);
	memory_set_bank(machine, "bank1", 0);

	state->save_item(NAME(state->m_int_enable));
	state->save_item(NAME(state->m_input_sel));
}

static MACHINE_RESET( mayumi )
{
	mayumi_state *state = machine.driver_data<mayumi_state>();

	state->m_int_enable = 0;
	state->m_input_sel = 0;
}

static MACHINE_CONFIG_START( mayumi, mayumi_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MCLK/2) /* 5.000 MHz ? */
	MCFG_CPU_PROGRAM_MAP(mayumi_map)
	MCFG_CPU_IO_MAP(mayumi_io_map)
	MCFG_CPU_VBLANK_INT("screen", mayumi_interrupt)

	MCFG_MACHINE_START( mayumi )
	MCFG_MACHINE_RESET( mayumi )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(2*8, 62*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_STATIC(mayumi)

	MCFG_GFXDECODE(mayumi)
	MCFG_PALETTE_LENGTH(256)

	MCFG_PALETTE_INIT(RRRR_GGGG_BBBB)
	MCFG_VIDEO_START(mayumi)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, MCLK/4)
	MCFG_SOUND_CONFIG(ym2203_config)
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.40)

	MCFG_NVRAM_ADD_0FILL("nvram")

MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( mayumi )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* CPU */
	ROM_LOAD( "my00.bin",  0x00000, 0x08000, CRC(33189e37) SHA1(cbf75f56360ef7da5b7b1207b58cd0d72bcaf207) )
	ROM_LOAD( "my01.bin",  0x10000, 0x10000, CRC(5280fb39) SHA1(cee7653f4353031701ec1608881b37073b178d9f) ) // Banked
	ROM_COPY( "maincpu", 0x10000, 0x08000, 0x4000 )

	ROM_REGION( 0x30000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "my10.bin", 0x00000, 0x10000, CRC(3b4f4f97) SHA1(50bda1484e965f15630bd2e05861d74ddeb0d88e) )
	ROM_LOAD( "my20.bin", 0x10000, 0x10000, CRC(18544029) SHA1(74bd8bb422db33bd7af08afbf9b801bd31a3f199) )
	ROM_LOAD( "my30.bin", 0x20000, 0x10000, CRC(7f22d53f) SHA1(f8e5874ba0fa003ba0d6a504b2169acdf1491484) )

	ROM_REGION( 0x0300, "proms", 0 ) /* color PROMs */
	ROM_LOAD( "my-9m.bin", 0x0000,  0x0100, CRC(b18fd669) SHA1(e2b1477c1bc49994b0b652d63a2205363aab9a74) ) // R
	ROM_LOAD( "my-9l.bin", 0x0100,  0x0100, CRC(f3fef561) SHA1(247f579fe91ad7e516c93a873b2ecca780bf6da0) ) // G
	ROM_LOAD( "my-9k.bin", 0x0200,  0x0100, CRC(3e7a8012) SHA1(24129586a1c39f68dad274b5afbdd6c027ab0901) ) // B
ROM_END

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1988, mayumi, 0, mayumi, mayumi, 0, ROT0, "Sanritsu / Victory L.L.C.",  "Kikiippatsu Mayumi-chan (Japan)", GAME_SUPPORTS_SAVE )
