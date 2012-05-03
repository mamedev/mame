/*

Chance 32 - Poker Game by PAL Company

1x HD46505SP / HD6845SP
1x Z84C0008PEC

XTAL: 12.000 Mhz


 todo, fix tilemaps, colours, inputs, hook up oki, add CRTC device

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "video/mc6845.h"



class chance32_state : public driver_device
{
public:
	chance32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fgram(*this, "fgram"),
		m_bgram(*this, "bgram")

	{ }

	DECLARE_WRITE8_MEMBER(chance32_fgram_w)
	{
		m_fgram[offset] = data;
		m_fg_tilemap->mark_tile_dirty(offset / 2);
	}

	DECLARE_WRITE8_MEMBER(chance32_bgram_w)
	{
		m_bgram[offset] = data;
		m_bg_tilemap->mark_tile_dirty(offset / 2);
	}

	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_READ8_MEMBER(mux_r);

	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;

	required_shared_ptr<UINT8> m_fgram;
	required_shared_ptr<UINT8> m_bgram;

	UINT8 mux_data;
};

static TILE_GET_INFO( get_fg_tile_info )
{
	chance32_state *state = machine.driver_data<chance32_state>();
	int code = (state->m_fgram[tile_index * 2 + 1] << 8) | state->m_fgram[tile_index * 2];
	int flip = (~code >> 12)&1;
	SET_TILE_INFO(
			1,
			code & 0x0fff,
			code >> 13,
			TILE_FLIPYX(flip<<1)|flip);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	chance32_state *state = machine.driver_data<chance32_state>();
	int code = (state->m_bgram[tile_index * 2 +1] << 8) | state->m_bgram[tile_index * 2];
	int flip = (~code >> 12)&1;
	SET_TILE_INFO(
			0,
			code & 0x0fff,
			code >> 13,
			TILE_FLIPYX(flip<<1|flip));
}


VIDEO_START( chance32 )
{
	chance32_state *state = machine.driver_data<chance32_state>();

	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 16, 8, 35, 29);
	state->m_fg_tilemap->set_transparent_pen(0);

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 8, 35, 29);

	state->m_fg_tilemap->set_flip(TILE_FLIPX|TILE_FLIPY);
	state->m_bg_tilemap->set_flip(TILE_FLIPX|TILE_FLIPY);
}

SCREEN_UPDATE_IND16( chance32 )
{
	chance32_state *state = screen.machine().driver_data<chance32_state>();

	/* TODO: wtf? */
	state->m_bg_tilemap->set_scrollx(0, 352);
	state->m_bg_tilemap->set_scrolly(0, 160);
	state->m_fg_tilemap->set_scrollx(0, 352);
	state->m_fg_tilemap->set_scrolly(0, 160);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);

	return 0;
}


static ADDRESS_MAP_START( chance32_map, AS_PROGRAM, 8, chance32_state )
	AM_RANGE(0x0000, 0xcfff) AM_ROM
	AM_RANGE(0xd800, 0xdfff) AM_RAM

	AM_RANGE(0xe000, 0xefff) AM_RAM_WRITE(paletteram_xGGGGGRRRRRBBBBB_byte_le_w) AM_SHARE("paletteram")

	AM_RANGE(0xf000, 0xf7ff) AM_RAM_WRITE(chance32_fgram_w) AM_SHARE("fgram")
	AM_RANGE(0xf800, 0xffff) AM_RAM_WRITE(chance32_bgram_w) AM_SHARE("bgram")
ADDRESS_MAP_END

WRITE8_MEMBER(chance32_state::mux_w)
{
	mux_data = data;
}

READ8_MEMBER(chance32_state::mux_r)
{
	UINT8 res,i;
	const char *const muxnames[4] = { "IN0", "IN1", "IN2", "IN3" };
	res = 0;

	for(i=0;i<4;i++)
	{
		if(mux_data & 1 << i)
			res |= ioport(muxnames[i])->read();
	}

	return res;
}

static ADDRESS_MAP_START( chance32_portmap, AS_IO, 8, chance32_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	//AM_RANGE(0x10, 0x10) AM_WRITENOP // ?
	AM_RANGE(0x13, 0x13) AM_WRITE(mux_w)
	AM_RANGE(0x20, 0x20) AM_READ_PORT("DSW0")
	AM_RANGE(0x21, 0x21) AM_READ_PORT("DSW1")
	AM_RANGE(0x22, 0x22) AM_READ_PORT("DSW2")
	AM_RANGE(0x23, 0x23) AM_READ_PORT("DSW3")
	AM_RANGE(0x24, 0x24) AM_READ_PORT("DSW4")
	AM_RANGE(0x25, 0x25) AM_READ(mux_r)
//	AM_RANGE(0x26, 0x26) AM_READ_PORT("UNK") // vblank?
	AM_RANGE(0x30, 0x30) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x31, 0x31) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0x50, 0x50) AM_DEVREADWRITE("oki", okim6295_device, read, write)
//	AM_RANGE(0x60, 0x60) AM_WRITENOP // lamps

ADDRESS_MAP_END



static INPUT_PORTS_START( chance32 )
	PORT_START("UNK")

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "DSW0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DSW1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DSW2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "DSW3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, "DSW4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x00, "0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x00, "1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, "2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x00, "3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	16,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3,4,5,6,7 },
	{ 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8, 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	{ 7*128, 6*128, 5*128, 4*128, 3*128, 2*128, 1*128, 0*128 },
	128*8
};

static GFXDECODE_START( chance32 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 8 )
GFXDECODE_END



static MACHINE_START( chance32 )
{

}

static MACHINE_RESET( chance32 )
{

}

static const mc6845_interface mc6845_intf =
{
	"screen",	/* screen we are acting on */
	16,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};


static MACHINE_CONFIG_START( chance32, chance32_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,12000000/2)
	MCFG_CPU_PROGRAM_MAP(chance32_map)
	MCFG_CPU_IO_MAP(chance32_portmap)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_MACHINE_START(chance32)
	MCFG_MACHINE_RESET(chance32)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*16, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 35*16-1, 0, 29*8-1)
	MCFG_SCREEN_UPDATE_STATIC(chance32)

	MCFG_MC6845_ADD("crtc", H46505, 12000000/14, mc6845_intf)	/* hand tuned to get ~60 fps */

	MCFG_GFXDECODE(chance32)
	MCFG_PALETTE_LENGTH(0x800)

	MCFG_VIDEO_START(chance32)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 12000000/12, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



ROM_START( chance32 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.u52", 0x00000, 0x10000, CRC(331048b2) SHA1(deb4da570b3efe6e15deefb6351f925b642b4614)  )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD16_BYTE( "4.u64", 0x00001, 0x20000, CRC(da80d9bd) SHA1(3b5235ab59fd55f0ec5584b3cf1aa5c8f36c76f6) )
	ROM_LOAD16_BYTE( "5.u65", 0x00000, 0x20000, CRC(7528773b) SHA1(95c8e55cdec2c5c1dcdcc5a7edc6e590e3829f92) )
	ROM_LOAD16_BYTE( "6.u66", 0x40001, 0x20000, CRC(cee2ffb0) SHA1(527c2072d39484317b0320afd975df1bbe244a01) )
	ROM_LOAD16_BYTE( "7.u67", 0x40000, 0x20000, CRC(42dc4b69) SHA1(44c8f902db4c7ac235d5ea15d1b509f98663690a) )

	ROM_REGION( 0x40000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD16_BYTE( "1.u71", 0x00001, 0x20000, CRC(f8e85873) SHA1(6ad24f7fcbc62a03180e168d70239df1ce662f0d) )
	ROM_LOAD16_BYTE( "2.u72", 0x00000, 0x20000, CRC(860b534d) SHA1(44649ea93acdf173356bfcd7e81916253b52c378) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "8.u21", 0x00000, 0x40000, CRC(161b35dd) SHA1(d20a75a4c4ed9cd9cfc12faee921122274840f06) )

	ROM_REGION( 0x40000, "gals", 0 ) // no idea if these are any good
	ROM_LOAD( "gal20v8a.u53.jed.bin", 0x0000, 0x157, CRC(9f680800) SHA1(2fa41ead85136e851d465432a7b9d3ec848c7a22) )
	ROM_LOAD( "gal20v8a.u54.jed.bin", 0x0000, 0x157, CRC(9f680800) SHA1(2fa41ead85136e851d465432a7b9d3ec848c7a22) )
	ROM_LOAD( "gal20v8a.u55.jed.bin", 0x0000, 0x157, CRC(9f680800) SHA1(2fa41ead85136e851d465432a7b9d3ec848c7a22) )
	ROM_LOAD( "gal20v8a.u56.jed.bin", 0x0000, 0x157, CRC(6bab01ad) SHA1(c69e4be41a989a52788af8062f48bbe26bc3dab8) )
	ROM_LOAD( "gal20v8a.u57.jed.bin", 0x0000, 0x157, CRC(787c4159) SHA1(f4a869b317c6be1024f1ca21bcc4af478c8227c8) )
	ROM_LOAD( "gal20v8a.u58.jed.bin", 0x0000, 0x157, CRC(7b16053b) SHA1(cdb289d4f27c7a1a918393943bb8db9712e2f52e) )

	ROM_LOAD( "gal16v8a.u47.jed.bin", 0x0000, 0x117, CRC(a733f0de) SHA1(6eec26043cedb3cae4efe93faa84a07327be468b) )
	ROM_LOAD( "gal16v8a.u48.jed.bin", 0x0000, 0x117, CRC(5f1360ef) SHA1(56e4ee0dbae5602d810b2f7c744a71eb1a1e08a8) )

	ROM_LOAD( "gal16v8a.u32.jed.bin", 0x0000, 0x117, CRC(c0784cd1) SHA1(0ae2ce482d379e29c2a9f130fc0d9ed928faef98) )

	ROM_LOAD( "gal16v8a.u24.jed.bin", 0x0000, 0x117, CRC(a733f0de) SHA1(6eec26043cedb3cae4efe93faa84a07327be468b) )
ROM_END





GAME( 19??, chance32,  0,    chance32, chance32,  0, ROT0, "Pal Company", "Chance Thirty Two", GAME_NOT_WORKING )
