// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Kung-Fu Master / Spartan X (Tecfri bootleg)
single PCB with 2x Z80
similar looking to the '1942p' and 'spyhuntpr' PCBs


P2 inputs don't work in 'cocktail' mode (maybe it's just unsupported on this PCB?)

DIPS etc. are near the 2nd CPU, should it be reading them?

visible area is 16 lines less than the original, otherwise you get bad sprites
but I think this is probably correct.

some sprites are a bit glitchy when entering playfield (see title screen)
probably an original bug?

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/iremipt.h"
#include "sound/ay8910.h"

class spartanxtec_state : public driver_device
{
public:
	spartanxtec_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_m62_tileram(*this, "m62_tileram"),
		m_spriteram(*this, "spriteram"),
		m_scroll_lo(*this, "scroll_lo"),
		m_scroll_hi(*this, "scroll_hi"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	required_shared_ptr<UINT8> m_m62_tileram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_scroll_lo;
	required_shared_ptr<UINT8> m_scroll_hi;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_spartanxtec(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(spartanxtec);

	tilemap_t*             m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(kungfum_tileram_w);
	TILE_GET_INFO_MEMBER(get_kungfum_bg_tile_info);
	DECLARE_WRITE8_MEMBER(spartanxtec_soundlatch_w);
	DECLARE_WRITE8_MEMBER(a801_w);
	DECLARE_WRITE8_MEMBER(sound_irq_ack);
	DECLARE_WRITE8_MEMBER(irq_ack);

	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;


};




WRITE8_MEMBER(spartanxtec_state::kungfum_tileram_w)
{
	m_m62_tileram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}


TILE_GET_INFO_MEMBER(spartanxtec_state::get_kungfum_bg_tile_info)
{
	int code;
	int color;
	int flags;
	code = m_m62_tileram[tile_index];
	color = m_m62_tileram[tile_index + 0x800];
	flags = 0;
	if ((color & 0x20))
	{
		flags |= TILE_FLIPX;
	}
	SET_TILE_INFO_MEMBER(0, code | ((color & 0xc0)<< 2), color & 0x1f, flags);

	/* is the following right? */
	if ((tile_index / 64) < 6 || ((color & 0x1f) >> 1) > 0x0c)
		tileinfo.category = 1;
	else
		tileinfo.category = 0;
}

void spartanxtec_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(spartanxtec_state::get_kungfum_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap->set_scroll_rows(32);
}


void spartanxtec_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(1);

	for (int i = 0; i < 0x400; i += 4)
	{
		int x = m_spriteram[i+2]+128;
		int y = (225-m_spriteram[i+1])&0xff; // 224 or 225? 224 sometimes gives an ugly line on player death sprite, 225 also aligns better with original
		int code = m_spriteram[i+0];
		int attr = m_spriteram[i+3];
		code |= (attr & 0xc0) << 2;

		int colour = attr & 0x1f;
		int flipx = attr & 0x20;
		int flipy = 0;

		gfx->transpen(bitmap,cliprect,code,colour,flipx,flipy,x,y,7);

	}


}


UINT32 spartanxtec_state::screen_update_spartanxtec(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// there are 4 sets of scroll registers
	// how to split them isn't clear, 4 groups of 8 rows would be logical
	// but only 6 rows should use the first scroll register and the
	// remaining 2 rows scroll values from there can't wrap onto the bottom
	// as that doesn't work. (breaks bottom 2 lines of playfield scroll)
	// HOWEVER sprites are also broken in that area, so I think this bootleg
	// probably just displays less lines.


	for (int i = 0; i < 32; i++)
	{
		int scrollval;

		scrollval = m_scroll_lo[i/8] | (m_scroll_hi[i/8] << 8);

		m_bg_tilemap->set_scrollx((i-2)&0xff, scrollval+28-128);
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1, 0);

	return 0;
}




WRITE8_MEMBER(spartanxtec_state::spartanxtec_soundlatch_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER(spartanxtec_state::a801_w)
{
	if (data != 0xf0) printf("a801_w %02x\n", data);
}

WRITE8_MEMBER(spartanxtec_state::irq_ack)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}


static ADDRESS_MAP_START( spartanxtec_map, AS_PROGRAM, 8, spartanxtec_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc400, 0xc7ff) AM_RAM AM_SHARE("spriteram")

	AM_RANGE(0x8000, 0x8000) AM_WRITE(spartanxtec_soundlatch_w)

	AM_RANGE(0x8100, 0x8100) AM_READ_PORT("DSW1")
	AM_RANGE(0x8101, 0x8101) AM_READ_PORT("DSW2")
	AM_RANGE(0x8102, 0x8102) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x8103, 0x8103) AM_READ_PORT("P1")

	AM_RANGE(0x8200, 0x8200) AM_WRITE(irq_ack)

	AM_RANGE(0xA801, 0xA801) AM_WRITE(a801_w)

	AM_RANGE(0xa900, 0xa903) AM_RAM AM_SHARE("scroll_lo")
	AM_RANGE(0xa980, 0xa983) AM_RAM AM_SHARE("scroll_hi")

	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(kungfum_tileram_w) AM_SHARE("m62_tileram")

	AM_RANGE(0xe000, 0xefff) AM_RAM

ADDRESS_MAP_END



WRITE8_MEMBER(spartanxtec_state::sound_irq_ack)
{
	m_audiocpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}


static ADDRESS_MAP_START( spartanxtec_sound_map, AS_PROGRAM, 8, spartanxtec_state )

	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM

	AM_RANGE(0xc000, 0xc000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( spartanxtec_sound_io, AS_IO, 8, spartanxtec_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x0000, 0x0000) AM_WRITE( sound_irq_ack )

	AM_RANGE(0x0012, 0x0013) AM_DEVWRITE("ay3", ay8910_device, address_data_w)
	AM_RANGE(0x0012, 0x0012) AM_DEVREAD("ay3", ay8910_device, data_r)

	AM_RANGE(0x0014, 0x0015) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x0014, 0x0014) AM_DEVREAD("ay1", ay8910_device, data_r)

	AM_RANGE(0x0018, 0x0019) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x0018, 0x0018) AM_DEVREAD("ay2", ay8910_device, data_r)
ADDRESS_MAP_END




static INPUT_PORTS_START( spartanxtec )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x02, "Energy Loss" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	/* Manual says that only coin mode 1 is available and SW2:3 should be always OFF */
	/* However, coin mode 2 works perfectly. */
	IREM_Z80_COINAGE_TYPE_3_LOC(SW1)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1") // I don't think this is implemented in the bootleg, it just shifts the sprites a bit
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, "Cocktail (invalid?)" )
	PORT_DIPNAME( 0x04, 0x04, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	/* In slowmo mode, press 2 to slow game speed */
	PORT_DIPNAME( 0x08, 0x08, "Slow Motion Mode (Cheat)" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* In freeze mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x10, 0x10, "Freeze (Cheat)" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* In level selection mode, press 1 to select and 2 to restart */
	PORT_DIPNAME( 0x20, 0x20, "Level Selection Mode (Cheat)" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(19)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0,1,2,3,4,5,6,7, 128, 129, 130, 131, 132, 133, 134, 135 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8  },
	32*8
};


static GFXDECODE_START( news )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0x100, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles16x16_layout, 0, 32 )
GFXDECODE_END



void spartanxtec_state::machine_start()
{
}

void spartanxtec_state::machine_reset()
{
}

PALETTE_INIT_MEMBER(spartanxtec_state, spartanxtec)
{
	// todo, proper weights for this bootleg PCB
	const UINT8 *color_prom = memregion("cprom")->base();
	for (int i = 0; i < 0x200; i++)
	{
		int r, g, b;

		b = (color_prom[i+0x000]&0x0f)<<4;
		g = (color_prom[i+0x200]&0x0f)<<4;
		r = (color_prom[i+0x400]&0x0f)<<4;

		palette.set_pen_color(i, rgb_t(r,g,b));
	}
}



static MACHINE_CONFIG_START( spartanxtec, spartanxtec_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,4000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(spartanxtec_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", spartanxtec_state,  irq0_line_assert)

	MCFG_CPU_ADD("audiocpu", Z80,4000000)
	MCFG_CPU_PROGRAM_MAP(spartanxtec_sound_map)
	MCFG_CPU_IO_MAP(spartanxtec_sound_io)
	MCFG_CPU_PERIODIC_INT_DRIVER(spartanxtec_state, irq0_line_assert, 1000) // controls speed of music
//  MCFG_CPU_VBLANK_INT_DRIVER("screen", spartanxtec_state,  irq0_line_hold)

	/* video hardware */
	// todo, proper screen timings for this bootleg PCB - as visible area is less it's probably ~60hz, not 55
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1790))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA((64*8-256)/2, 64*8-(64*8-256)/2-1, 0*8, 32*8-1-16)
	MCFG_SCREEN_UPDATE_DRIVER(spartanxtec_state, screen_update_spartanxtec)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_INIT_OWNER(spartanxtec_state,spartanxtec)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", news)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 1000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("ay2", AY8910, 1000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("ay3", AY8910, 1000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

MACHINE_CONFIG_END



ROM_START( spartanxtec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x04000, CRC(d5d6cddf) SHA1(baaec83be455bf2267d51ea2a2c1fcda22f27bd5) )
	ROM_LOAD( "2.bin", 0x04000, 0x04000, CRC(2803bb72) SHA1(d0f93c61f3f08fb866e2a4617a7824e72f61c97f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "3.bin", 0x00000, 0x01000, CRC(9a18af94) SHA1(1644295aa0c837dced5934360e41d77e0a93ccd1) )

	ROM_REGION( 0x6000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "5.bin", 0x00000, 0x0800, CRC(8a3d2978) SHA1(e50ba8d63e894c6a555d92c3144682be68f111b0))
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x0800, 0x0800)
	ROM_CONTINUE(0x1800, 0x0800)
	ROM_LOAD( "6.bin", 0x02000, 0x0800, CRC(b1570b6b) SHA1(380a692309690e6ff6b57fda657192fff95167e0) )
	ROM_CONTINUE(0x3000, 0x0800)
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x3800, 0x0800)
	ROM_LOAD( "4.bin", 0x04000, 0x0800, CRC(b55672ef) SHA1(7bd556a76e130be1262aa7db09df84c6463ce9ef) )
	ROM_CONTINUE(0x5000, 0x0800)
	ROM_CONTINUE(0x4800, 0x0800)
	ROM_CONTINUE(0x5800, 0x0800)

	ROM_REGION( 0x18000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "7.bin", 0x00000, 0x08000, CRC(aa897e30) SHA1(90b3b316800be106d3baa6783ca894703f369d4e) )
	ROM_LOAD( "8.bin", 0x08000, 0x08000, CRC(98a1803b) SHA1(3edfc45c289f850b07a0231ce0b792cbec6fb245) )
	ROM_LOAD( "9.bin", 0x10000, 0x08000, CRC(e3bf0d73) SHA1(4562422c07399e240081792b96b9018d1e7dd97b) )

	ROM_REGION( 0x600, "cprom", 0 )
	// first half of all of these is empty
	ROM_LOAD( "4_MCM7643_82s137.BIN", 0x0000, 0x0200, CRC(548a0ab1) SHA1(e414b61feba73bcc1a53e17c848aceea3b8100e7) ) ROM_CONTINUE(0x0000,0x0200)
	ROM_LOAD( "5_MCM7643_82s137.BIN", 0x0200, 0x0200, CRC(a678480e) SHA1(515fa2b09c666a46dc145313eda3c465afff4451) ) ROM_CONTINUE(0x0200,0x0200)
	ROM_LOAD( "6_MCM7643_82s137.BIN", 0x0400, 0x0200, CRC(5a707f85) SHA1(35932daf453787780550464b78465581e1ef35e1) ) ROM_CONTINUE(0x0400,0x0200)

	ROM_REGION( 0x18000, "timing", 0 ) // i think
	ROM_LOAD( "7_82s147.BIN", 0x0000, 0x0200, CRC(54a9e294) SHA1(d44d21ab8141bdfe697fd303cdc1b5c4177909bc) )

	ROM_REGION( 0x18000, "unkprom", 0 ) // just linear increasing value
	ROM_LOAD( "1_tbp24s10_82s129.BIN", 0x0000, 0x0100, CRC(b6135ee0) SHA1(248a978987cff86c2bbad10ef332f63a6abd5bee) )
	ROM_LOAD( "2_tbp24s10_82s129.BIN", 0x0000, 0x0100, CRC(b6135ee0) SHA1(248a978987cff86c2bbad10ef332f63a6abd5bee) )
ROM_END



GAME( 1987, spartanxtec,  kungfum,    spartanxtec, spartanxtec, driver_device,  0, ROT0, "bootleg (Tecfri)", "Spartan X (Tecfri hardware bootleg)", 0 )
