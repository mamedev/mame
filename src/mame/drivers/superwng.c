// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/****************************************************************************************

Super Wing - (c) 1985 Wing (UPL?)

driver by Tomasz Slanina

probably a sequel to flipjack
Hardware a bit (interrupts, sound) similar to mouser as well

TODO:
- unused rom 6.8s (located on the pcb near the gfx rom 7.8p, but contains
  data (similar to the one in roms 4.5p and 5.5r)

  The game currently crashes after the bonus round rather than moving on to
  the next level, it writes 01 to 0xa187 which is probably ROM bank, however
  banking the ROM in there results in the game crashing anyway, and looking
  at the data I wonder if it is corrupt, there are odd patterns in FF fill
  areas.

  (to access the bonus round take out the targets on the middle-left then hit
   the ball into one of the portals at the top left)


- dump color prom
- some unknown DSW and inputs
- hopper
- unknown writes
- measure clocks


*****************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

#define MASTER_CLOCK XTAL_18_432MHz

class superwng_state : public driver_device
{
public:
	superwng_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_videoram_bg(*this, "videorabg"),
		m_videoram_fg(*this, "videorafg"),
		m_colorram_bg(*this, "colorrabg"),
		m_colorram_fg(*this, "colorrafg"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_shared_ptr<UINT8> m_videoram_bg;
	required_shared_ptr<UINT8> m_videoram_fg;
	required_shared_ptr<UINT8> m_colorram_bg;
	required_shared_ptr<UINT8> m_colorram_fg;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	UINT8 m_tile_bank;
	UINT8 m_sound_byte;
	UINT8 m_nmi_enable;

	tilemap_t * m_bg_tilemap;
	tilemap_t * m_fg_tilemap;

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
	DECLARE_WRITE8_MEMBER(superwng_hopper_w);
	DECLARE_READ8_MEMBER(superwng_sound_byte_r);
	DECLARE_WRITE8_MEMBER(superwng_unk_a187_w);
	DECLARE_WRITE8_MEMBER(superwng_unk_a185_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(superwng);
	UINT32 screen_update_superwng(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(superwng_nmi_interrupt);
	INTERRUPT_GEN_MEMBER(superwng_sound_nmi_assert);
};

WRITE8_MEMBER(superwng_state::superwng_unk_a187_w)
{
	membank("bank1")->set_entry(data&1);
}

WRITE8_MEMBER(superwng_state::superwng_unk_a185_w)
{
//  printf("superwng_unk_a185_w %02x\n", data);
}

TILE_GET_INFO_MEMBER(superwng_state::get_bg_tile_info)
{
	int code = m_videoram_bg[tile_index];
	int attr = m_colorram_bg[tile_index];

	code= (code&0x7f) | ((attr&0x40)<<1) | ((code&0x80)<<1);
	code|=m_tile_bank?0x200:0;

	int flipx=(attr&0x80) ? TILE_FLIPX : 0;
	int flipy=(attr&0x80) ? TILE_FLIPY : 0;

	SET_TILE_INFO_MEMBER(0, code, attr & 0xf, flipx|flipy);
}

TILE_GET_INFO_MEMBER(superwng_state::get_fg_tile_info)
{
	int code = m_videoram_fg[tile_index];
	int attr = m_colorram_fg[tile_index];

	code= (code&0x7f) | ((attr&0x40)<<1) | ((code&0x80)<<1);

	code|=m_tile_bank?0x200:0;

	int flipx=(attr&0x80) ? TILE_FLIPX : 0;
	int flipy=(attr&0x80) ? TILE_FLIPY : 0;

	SET_TILE_INFO_MEMBER(0, code, attr & 0xf, flipx|flipy);
}

void superwng_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(superwng_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(superwng_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_scrollx(0, 64);
}

UINT32 superwng_state::screen_update_superwng(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	rectangle tmp = cliprect;

	if (flip_screen())
	{
		tmp.min_x += 32;
		m_fg_tilemap->draw(screen, bitmap, tmp, 0, 0);
	}
	else
	{
		tmp.max_x -= 32;
		m_fg_tilemap->draw(screen, bitmap, tmp, 0, 0);
	}

	//sprites
	for (int i = 0x3e; i >= 0; i -= 2)
	{
		/*      76543210
		video0: xxxxxx    code
		              x   /flip
		               x  enable?
		video1: xxxxxxxx  x
		color0: xxxxxxxx  y
		color1: xxx       unused?
		           x      ?
		            xxxx  color
		*/
		if (~m_videoram_bg[i] & 1)
			continue;

		int code = (m_videoram_bg[i] >> 2) | 0x40;
		int flip = ~m_videoram_bg[i] >> 1 & 1;
		int sx = 240 - m_videoram_bg[i + 1];
		int sy = m_colorram_bg[i];
		int color = m_colorram_bg[i + 1] & 0xf;

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
						code,
						color,
						flip, flip,
						sx, sy, 0);
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

PALETTE_INIT_MEMBER(superwng_state, superwng)
{
	int i;
	const UINT8 * ptr=superwng_colors;

	for (i = 0; i < palette.entries(); i++)
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

		palette.set_pen_color(i,rgb_t(r,g,b));
		++ptr;
	}
}

WRITE8_MEMBER(superwng_state::superwng_nmi_enable_w)
{
	m_nmi_enable = data;
}

INTERRUPT_GEN_MEMBER(superwng_state::superwng_nmi_interrupt)
{
	if (BIT(m_nmi_enable, 0))
		nmi_line_pulse(device);
}

WRITE8_MEMBER(superwng_state::superwng_sound_interrupt_w)
{
	m_sound_byte = data;
	m_audiocpu->set_input_line(0, ASSERT_LINE);
}

READ8_MEMBER(superwng_state::superwng_sound_byte_r)
{
	m_audiocpu->set_input_line(0, CLEAR_LINE);
	return m_sound_byte;
}

WRITE8_MEMBER(superwng_state::superwng_sound_nmi_clear_w)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

INTERRUPT_GEN_MEMBER(superwng_state::superwng_sound_nmi_assert)
{
	if (BIT(m_nmi_enable, 0))
		device.execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
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
	flip_screen_set(~data & 0x01);
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

WRITE8_MEMBER(superwng_state::superwng_hopper_w)
{
}

static ADDRESS_MAP_START( superwng_map, AS_PROGRAM, 8, superwng_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x6fff) AM_ROMBANK("bank1")
	AM_RANGE(0x7000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0x83ff) AM_RAM_WRITE(superwng_bg_vram_w) AM_SHARE("videorabg")
	AM_RANGE(0x8400, 0x87ff) AM_RAM_WRITE(superwng_fg_vram_w) AM_SHARE("videorafg")
	AM_RANGE(0x8800, 0x8bff) AM_RAM_WRITE(superwng_bg_cram_w) AM_SHARE("colorrabg")
	AM_RANGE(0x8c00, 0x8fff) AM_RAM_WRITE(superwng_fg_cram_w) AM_SHARE("colorrafg")
	AM_RANGE(0x9800, 0x99ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("P1")
	AM_RANGE(0xa000, 0xa000) AM_WRITE(superwng_hopper_w)
	AM_RANGE(0xa080, 0xa080) AM_READ_PORT("P2")
	AM_RANGE(0xa100, 0xa100) AM_READ_PORT("DSW1")
	AM_RANGE(0xa100, 0xa100) AM_WRITE(superwng_sound_interrupt_w)
	AM_RANGE(0xa180, 0xa180) AM_READ_PORT("DSW2")
	AM_RANGE(0xa180, 0xa180) AM_WRITENOP // watchdog? int ack?
	AM_RANGE(0xa181, 0xa181) AM_WRITE(superwng_nmi_enable_w)
	AM_RANGE(0xa182, 0xa182) AM_WRITE(superwng_tilebank_w)
	AM_RANGE(0xa183, 0xa183) AM_WRITE(superwng_flip_screen_w)
	AM_RANGE(0xa184, 0xa184) AM_WRITE(superwng_cointcnt1_w)
	AM_RANGE(0xa185, 0xa185) AM_WRITE(superwng_unk_a185_w)  // unknown, always(?) 0
	AM_RANGE(0xa186, 0xa186) AM_WRITE(superwng_cointcnt2_w)
	AM_RANGE(0xa187, 0xa187) AM_WRITE(superwng_unk_a187_w) // unknown, always(?) 0
ADDRESS_MAP_END

static ADDRESS_MAP_START( superwng_sound_map, AS_PROGRAM, 8, superwng_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x3000, 0x3000) AM_WRITE(superwng_sound_nmi_clear_w)
	AM_RANGE(0x4000, 0x4000) AM_DEVREADWRITE("ay1", ay8910_device, data_r, data_w)
	AM_RANGE(0x5000, 0x5000) AM_DEVWRITE("ay1", ay8910_device, address_w)
	AM_RANGE(0x6000, 0x6000) AM_DEVREADWRITE("ay2", ay8910_device, data_r, data_w)
	AM_RANGE(0x7000, 0x7000) AM_DEVWRITE("ay2", ay8910_device, address_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( superwng )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Shoot")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Right Flipper")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Left Flipper")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL PORT_NAME("P2 Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_NAME("P2 Shoot")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Right Flipper")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Left Flipper")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Tilt")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x00, "5" )

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
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown )) // hopper related, writes 0 to 0xa000 every frame if it is set
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown )) // hopper related, if 0x20 is set, and this is set, it will lock up with HOPPER EMPTY
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
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

void superwng_state::machine_start()
{
	save_item(NAME(m_tile_bank));
	save_item(NAME(m_sound_byte));
	save_item(NAME(m_nmi_enable));
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base()+0x4000, 0x4000);
}

void superwng_state::machine_reset()
{
	m_sound_byte = 0;
	m_nmi_enable = 0;
}

static MACHINE_CONFIG_START( superwng, superwng_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(superwng_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", superwng_state,  superwng_nmi_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(superwng_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(superwng_state, superwng_sound_nmi_assert,  4*60)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(superwng_state, screen_update_superwng)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", superwng)

	MCFG_PALETTE_ADD("palette", 0x40)
	MCFG_PALETTE_INIT_OWNER(superwng_state, superwng)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, MASTER_CLOCK/12)
	MCFG_AY8910_PORT_A_READ_CB(READ8(superwng_state, superwng_sound_byte_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("ay2", AY8910, MASTER_CLOCK/12)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

ROM_START( superwng )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "2.5l",         0x0000, 0x2000, CRC(8d102f8d) SHA1(ff6d994273a2e493a68637822cd0b1a2f69fd054) )
	ROM_LOAD( "3.5m",         0x2000, 0x2000, CRC(3b08bd19) SHA1(2020e2835df86a6a279bbf9d013a489f0e32a4bd) )
	ROM_LOAD( "4.5p",         0x4000, 0x2000, CRC(6a49746d) SHA1(f5cd5eb77f60972a3897243f9ee3d61aac0878fc) )
	ROM_LOAD( "5.5r",         0x6000, 0x2000, CRC(ebd23487) SHA1(16e8faf989aa80dbf9934450ec4ba642a6f88c63) )
	ROM_LOAD( "6.8s",         0x8000, 0x4000, BAD_DUMP CRC(774433e0) SHA1(82b10d797581c14914bcce320f2aa5d3fb1fba33) ) // banked but probably bad, bits at 0xxx39 offsets appear to be missing / corrupt.

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1.1a",         0x0000, 0x2000, CRC(a70aa39e) SHA1(b03de65d7bd020eb77495997128dce5ccbdbefac) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "7.8p",        0x0000, 0x4000, CRC(b472603c) SHA1(96f477a47a5be3db1292fea4f5c91ab155013f74) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "bprom.bin", 0x0000, 0x0040, NO_DUMP)
ROM_END


GAME( 1985, superwng,   0,      superwng, superwng, driver_device, 0, ROT90, "Wing", "Super Wing", MACHINE_NOT_WORKING | MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE ) // crashes after bonus stage, see notes, bad rom?
