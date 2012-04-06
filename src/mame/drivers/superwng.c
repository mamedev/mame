/****************************************************************************************
Super Wing - (c) 1985 Wing (UPL?)

driver by Tomasz Slanina

Hardware a bit (interrupts, sound) similar to mouser

TODO:

 - unused rom 6.8s (located on the pcb near the gfx rom 7.8p, but contains
   data (similar to the one in roms 4.5p and 5.5r).
   There are two possibilities: its bad dump of gfx rom (two extra bit layers
   of current gfx) or it's banked at 0x4000 - 0x7fff area.
  - missing color prom dump (missing on pcb)
  - some unknown DSW and inputs
  - unknown writes
  - tile and sprite flipping (both h and v controlled by the same bit?)
  - sprite coords and tile num (is the >>2 correct ?) (name entry, flipped screen)
  - measure clocks

  To enter initials - use tilt. It's probably separate button (?), not a tilt sensor.

*****************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

#define MASTER_CLOCK XTAL_18_432MHz

class superwng_state : public driver_device
{
public:
	superwng_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *    m_videoram_bg;
	UINT8 *    m_colorram_bg;
	UINT8 *    m_videoram_fg;
	UINT8 *    m_colorram_fg;

	int			m_tile_bank;

	UINT8      m_sound_byte;
	UINT8      m_nmi_enable;

	device_t *	m_maincpu;
	device_t *	m_audiocpu;

	tilemap_t *	m_bg_tilemap;
	tilemap_t *	m_fg_tilemap;

	DECLARE_WRITE8_MEMBER(superwng_nmi_enable_w);
	DECLARE_WRITE8_MEMBER(superwng_sound_interrupt_w);
	DECLARE_WRITE8_MEMBER(superwng_sound_nmi_clear_w);
	DECLARE_WRITE8_MEMBER(superwng_bg_vram_w);
	DECLARE_WRITE8_MEMBER(superwng_bg_cram_w);
	DECLARE_WRITE8_MEMBER(superwng_fg_vram_w);
	DECLARE_WRITE8_MEMBER(superwng_fg_cram_w);
	DECLARE_WRITE8_MEMBER(superwng_tilebank_w);
	DECLARE_WRITE8_MEMBER(superwng_flip_screen_w);
	DECLARE_WRITE8_MEMBER(superwng_cointcnt1_w);
	DECLARE_WRITE8_MEMBER(superwng_cointcnt2_w);
	DECLARE_WRITE8_MEMBER(superwng_flip_screen_x_w);
	DECLARE_WRITE8_MEMBER(superwng_flip_screen_y_w);
};

static TILE_GET_INFO( get_bg_tile_info )
{
	superwng_state *state = machine.driver_data<superwng_state>();
	int code = state->m_videoram_bg[tile_index];
	int attr = state->m_colorram_bg[tile_index];

	code= (code&0x7f) | ((attr&0x40)<<1) | ((code&0x80)<<1);
	code|=state->m_tile_bank?0x200:0;

	int flipx=(attr&0x80) ? TILE_FLIPX : 0;
	int flipy=(attr&0x80) ? TILE_FLIPY : 0;

	SET_TILE_INFO(	0, code, attr & 0xf, flipx|flipy);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	superwng_state *state = machine.driver_data<superwng_state>();
	int code = state->m_videoram_fg[tile_index];
	int attr = state->m_colorram_fg[tile_index];

	code= (code&0x7f) | ((attr&0x40)<<1) | ((code&0x80)<<1);

	code|=state->m_tile_bank?0x200:0;

	int flipx=(attr&0x80) ? TILE_FLIPX : 0;
	int flipy=(attr&0x80) ? TILE_FLIPY : 0;

	SET_TILE_INFO( 0, code, attr & 0xf, flipx|flipy);
}

WRITE8_MEMBER(superwng_state::superwng_flip_screen_x_w)
{
	flip_screen_x_set(machine(), ~data & 1);
}

WRITE8_MEMBER(superwng_state::superwng_flip_screen_y_w)
{
	flip_screen_y_set(machine(), ~data & 1);
}

static VIDEO_START( superwng )
{
	superwng_state *state = machine.driver_data<superwng_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_bg_tilemap->set_scrollx(0, 64);
}

static SCREEN_UPDATE_IND16( superwng )
{
	superwng_state *state = screen.machine().driver_data<superwng_state>();
	int flip=flip_screen_get(screen.machine());

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	rectangle tmp=cliprect;

	if(flip)
	{
		tmp.min_x+=32;
		state->m_fg_tilemap->draw(bitmap, tmp, 0, 0);
	}
	else
	{
		tmp.max_x-=32;
		state->m_fg_tilemap->draw(bitmap, tmp, 0, 0);
	}

	{
		//sprites

		for(int i=0x3e; i>=0; i-=2)
		{
			int code=(state->m_videoram_bg[i]>>2)+0x40;
			int sx=256-state->m_videoram_bg[i+1]-8;
			int sy = state->m_colorram_bg[i]+8;
			int attr = state->m_colorram_bg[i+1];

			if (flip)
			{
				sy-=8;
				sx-=8;
			}

			if(state->m_videoram_bg[i+1] | state->m_colorram_bg[i])
			{

				drawgfx_transpen(bitmap, cliprect,screen.machine().gfx[1],
								code,
								attr,
								flip, flip,
								sx, sy, 0);
			}
		}
	}

	return 0;
}


static const UINT8 superwng_colors[]= /* temporary */
{
	0x00, 0xc4, 0xff, 0x87, 0x00, 0xb0, 0xff, 0x2f, 0x00, 0x07, 0xff, 0xe0, 0x00, 0x86, 0xff, 0xc6,
	0x00, 0x07, 0x3f, 0xff, 0x00, 0xb0, 0x38, 0x27, 0x00, 0x20, 0xff, 0x27, 0x00, 0xa4, 0xff, 0x87,
	0x00, 0x58, 0xa8, 0x27, 0x00, 0x38, 0x3f, 0x27, 0x00, 0x80, 0xe4, 0x38, 0x00, 0x87, 0xff, 0x07,
	0x00, 0xc0, 0x07, 0x3f, 0x00, 0x1f, 0x3f, 0xff, 0x00, 0x86, 0x05, 0xff, 0x00, 0xc0, 0xe8, 0xff
};

PALETTE_INIT( superwng )
{
	int i;
	const UINT8 * ptr=superwng_colors;

	for (i = 0; i < machine.total_colors(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		bit0 = BIT(*ptr, 0);
		bit1 = BIT(*ptr, 1);
		bit2 = BIT(*ptr, 2);
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(*ptr, 3);
		bit1 = BIT(*ptr, 4);
		bit2 = BIT(*ptr, 5);
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(*ptr, 6);
		bit1 = BIT(*ptr, 7);
		b = 0x4f * bit0 + 0xa8 * bit1;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		++ptr;
	}
}

WRITE8_MEMBER(superwng_state::superwng_nmi_enable_w)
{
	m_nmi_enable = data;
}

static INTERRUPT_GEN( superwng_nmi_interrupt )
{
	superwng_state *state = device->machine().driver_data<superwng_state>();

	if (BIT(state->m_nmi_enable, 0))
		nmi_line_pulse(device);
}

WRITE8_MEMBER(superwng_state::superwng_sound_interrupt_w)
{
	m_sound_byte = data;
	device_set_input_line(m_audiocpu, 0, ASSERT_LINE);
}

static READ8_DEVICE_HANDLER( superwng_sound_byte_r )
{
	superwng_state *state = device->machine().driver_data<superwng_state>();
	device_set_input_line(state->m_audiocpu, 0, CLEAR_LINE);
	return state->m_sound_byte;
}

WRITE8_MEMBER(superwng_state::superwng_sound_nmi_clear_w)
{
	device_set_input_line(m_audiocpu, INPUT_LINE_NMI, CLEAR_LINE);
}

static INTERRUPT_GEN( superwng_sound_nmi_assert )
{
	superwng_state *state = device->machine().driver_data<superwng_state>();
	if (BIT(state->m_nmi_enable, 0))
		device_set_input_line(device, INPUT_LINE_NMI, ASSERT_LINE);
}

WRITE8_MEMBER(superwng_state::superwng_bg_vram_w)
{
	m_videoram_bg[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(superwng_state::superwng_bg_cram_w)
{
	m_colorram_bg[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(superwng_state::superwng_fg_vram_w)
{
	m_videoram_fg[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(superwng_state::superwng_fg_cram_w)
{
	m_colorram_fg[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(superwng_state::superwng_tilebank_w)
{
	m_tile_bank = data;
	m_bg_tilemap->mark_all_dirty();
	m_fg_tilemap->mark_all_dirty();
}

WRITE8_MEMBER(superwng_state::superwng_flip_screen_w)
{
	flip_screen_set(machine(), ~data & 0x01);
	m_bg_tilemap->mark_all_dirty();
	m_fg_tilemap->mark_all_dirty();
}

WRITE8_MEMBER(superwng_state::superwng_cointcnt1_w)
{
	coin_counter_w(machine(), 0, data);
}

WRITE8_MEMBER(superwng_state::superwng_cointcnt2_w)
{
	coin_counter_w(machine(), 1, data);
}

static ADDRESS_MAP_START( superwng_map, AS_PROGRAM, 8, superwng_state )
	AM_RANGE(0x0000, 0x6fff) AM_ROM
	AM_RANGE(0x7000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0x83ff) AM_RAM_WRITE(superwng_bg_vram_w) AM_BASE(m_videoram_bg)
	AM_RANGE(0x8400, 0x87ff) AM_RAM_WRITE(superwng_fg_vram_w) AM_BASE(m_videoram_fg)
	AM_RANGE(0x8800, 0x8bff) AM_RAM_WRITE(superwng_bg_cram_w) AM_BASE(m_colorram_bg)
	AM_RANGE(0x8c00, 0x8fff) AM_RAM_WRITE(superwng_fg_cram_w) AM_BASE(m_colorram_fg)
	AM_RANGE(0x9800, 0x99ff) AM_RAM  //collision map
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("P1")
	AM_RANGE(0xa000, 0xa000) AM_WRITENOP //unknown
	AM_RANGE(0xa080, 0xa080) AM_READ_PORT("P2")
	AM_RANGE(0xa100, 0xa100) AM_READ_PORT("DSW1")
	AM_RANGE(0xa100, 0xa100) AM_WRITE(superwng_sound_interrupt_w)
	AM_RANGE(0xa180, 0xa180) AM_READ_PORT("DSW2")
	AM_RANGE(0xa180, 0xa180) AM_WRITENOP //watchdog ? int ack ?
	AM_RANGE(0xa181, 0xa181) AM_WRITE(superwng_nmi_enable_w)
	AM_RANGE(0xa182, 0xa182) AM_WRITE(superwng_tilebank_w)
	AM_RANGE(0xa183, 0xa183) AM_WRITE(superwng_flip_screen_w)
	AM_RANGE(0xa184, 0xa184) AM_WRITE(superwng_cointcnt1_w)
	AM_RANGE(0xa185, 0xa185) AM_WRITENOP //unknown , always(?) 0
	AM_RANGE(0xa186, 0xa186) AM_WRITE(superwng_cointcnt2_w)
	AM_RANGE(0xa187, 0xa187) AM_WRITENOP //unknown , always(?) 0
ADDRESS_MAP_END

static ADDRESS_MAP_START( superwng_sound_map, AS_PROGRAM, 8, superwng_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x3000, 0x3000) AM_WRITE(superwng_sound_nmi_clear_w)
	AM_RANGE(0x4000, 0x4000) AM_DEVREADWRITE_LEGACY("ay1", ay8910_r, ay8910_data_w)
	AM_RANGE(0x5000, 0x5000) AM_DEVWRITE_LEGACY("ay1", ay8910_address_w)
	AM_RANGE(0x6000, 0x6000) AM_DEVREADWRITE_LEGACY("ay2", ay8910_r, ay8910_data_w)
	AM_RANGE(0x7000, 0x7000) AM_DEVWRITE_LEGACY("ay2", ay8910_address_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( superwng )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Launch Ball" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Right Flipper" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Left Flipper" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Launch Ball" ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Right Flipper" ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Left Flipper" ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(	0x00,  DEF_STR( Off ) )
	PORT_DIPSETTING(	0x01,  DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(	0x00,  DEF_STR( Off ) )
	PORT_DIPSETTING(	0x02,  DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(	0x00,  DEF_STR( Off ) )
	PORT_DIPSETTING(	0x04,  DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(	0x00,  DEF_STR( Off ) )
	PORT_DIPSETTING(	0x10,  DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(	0x00,  DEF_STR( Off ) )
	PORT_DIPSETTING(	0x20,  DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(	0x00,  DEF_STR( Off ) )
	PORT_DIPSETTING(	0x40,  DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x00, "3" )
	PORT_DIPSETTING(	0x80, "5" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x00,	DEF_STR( Unknown ) ) //causes writes to 0xa000 every frame
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(	0x00,  DEF_STR( Off ) )
	PORT_DIPSETTING(	0x40,  DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(	0x00,  DEF_STR( Off ) )
	PORT_DIPSETTING(	0x80,  DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0,4 },
	{8*8+0,8*8+1,8*8+2,8*8+3,0,1,2,3},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7},
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0,4 },
	{8*8+0,8*8+1,8*8+2,8*8+3,0,1,2,3,

	16*8+8*8+0,16*8+8*8+1,16*8+8*8+2,16*8+8*8+3,16*8+0,16*8+1,16*8+2,16*8+3},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7,
	16*8*2+8*0, 16*8*2+8*1, 16*8*2+8*2, 16*8*2+8*3, 16*8*2+8*4, 16*8*2+8*5, 16*8*2+8*6, 16*8*2+8*7,

	},
	16*8*4
};

static GFXDECODE_START( superwng )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,       0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout,     0, 16 )
GFXDECODE_END

static MACHINE_START( superwng )
{
	superwng_state *state = machine.driver_data<superwng_state>();

	state->m_maincpu = machine.device("maincpu");
	state->m_audiocpu = machine.device("audiocpu");

	state->save_item(NAME(state->m_sound_byte));
	state->save_item(NAME(state->m_nmi_enable));
}

static MACHINE_RESET( superwng )
{
	superwng_state *state = machine.driver_data<superwng_state>();

	state->m_sound_byte = 0;
	state->m_nmi_enable = 0;
}

static const ay8910_interface ay8910_config_1 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(superwng_sound_byte_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static const ay8910_interface ay8910_config_2 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_CONFIG_START( superwng, superwng_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(superwng_map)
	MCFG_CPU_VBLANK_INT("screen", superwng_nmi_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(superwng_sound_map)
	MCFG_CPU_PERIODIC_INT(superwng_sound_nmi_assert, 4*60)

	MCFG_MACHINE_START(superwng)
	MCFG_MACHINE_RESET(superwng)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MCFG_GFXDECODE(superwng)

	MCFG_PALETTE_LENGTH(0x40)
	MCFG_PALETTE_INIT(superwng)
	MCFG_VIDEO_START( superwng )
	MCFG_SCREEN_UPDATE_STATIC(superwng)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, MASTER_CLOCK/12)
	MCFG_SOUND_CONFIG(ay8910_config_1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("ay2", AY8910, MASTER_CLOCK/12)
	MCFG_SOUND_CONFIG(ay8910_config_2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

ROM_START( superwng )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "2.5l",         0x0000, 0x2000, CRC(8d102f8d) SHA1(ff6d994273a2e493a68637822cd0b1a2f69fd054) )
	ROM_LOAD( "3.5m",         0x2000, 0x2000, CRC(3b08bd19) SHA1(2020e2835df86a6a279bbf9d013a489f0e32a4bd) )
	ROM_LOAD( "4.5p",         0x4000, 0x2000, CRC(6a49746d) SHA1(f5cd5eb77f60972a3897243f9ee3d61aac0878fc) )
	ROM_LOAD( "5.5r",         0x6000, 0x2000, CRC(ebd23487) SHA1(16e8faf989aa80dbf9934450ec4ba642a6f88c63) )

	ROM_LOAD( "6.8s",        0x10000, 0x4000, CRC(774433e0) SHA1(82b10d797581c14914bcce320f2aa5d3fb1fba33) ) /* unknown */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1.1a",         0x0000, 0x2000, CRC(a70aa39e) SHA1(b03de65d7bd020eb77495997128dce5ccbdbefac) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "7.8p",        0x0000, 0x4000, CRC(b472603c) SHA1(96f477a47a5be3db1292fea4f5c91ab155013f74) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "bprom.bin", 0x0000, 0x0040, NO_DUMP)
ROM_END


GAME( 1985, superwng,   0,      superwng, superwng, 0, ROT90, "Wing", "Super Wing", GAME_WRONG_COLORS )
