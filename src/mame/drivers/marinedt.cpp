// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

	Marine Date (c) 1981 Taito

	similar to crbaloon.cpp
	
	TODO:
	- sprites
	- sound
	- inputs
	- colors
	- collision detection
	- Merge devices from crbaloon driver.

============================================================================

Marine Date
Taito 1981
PCB Layout
----------
Top board
MGO70001
MGN00001
 |---------------------------------------------|
 | VOL   VR1  VR2  VR3  VR4  VR5  VR6  VR7     |
 |  LM3900 LM3900 LM3900 LM3900 LM3900 LM3900 |-|
 |MB3712                                      |P|
 |   4006  LM3900 LM3900 LM3900               | |
 |2  4030                                     |-|
 |2                                            |
 |W                                  DSW(8)    |
 |A                                           |-|
 |Y   HD14584     NE555       MG17   DSW(8)   |Q|
 |                                            | |
 |    HD14584                                 |-|
 |          HD14584                            |
 |---------------------------------------------|
Notes: (PCB contains lots of resistors/caps/transistors etc)
      MG17    - 82S123 bipolar PROM (no location on PCB)
      MB3712  - Hitachi MB3712 Audio Power Amplifier
      LM3900  - Texas Instruments LM3900 Quad Operational Amplifier
      HD14584 - Hitachi HD14584 Hex schmitt Trigger
      NE555   - NE555 Timer
      4006    - RCA CD4006 18-Stage Static Register
      4030    - RCA CD4030 Quad Exclusive-Or Gate
      VR*     - Volume pots for each sound
      VOL     - Master Volume pot
Middle board
MGO70002
MGN00002
 |---------------------------------------------|
 |                                    MG15.1A  |
|-|                                   MG14.2A |-|
|S|                                           |Q|
| |                                           | |
|-|               MG16.4E                     |-|
 |                                             |
 |                                             |
|-|    MG13.6H              MG12.6C           |-|
|R|                                           |P|
| |                                   PC3259  | |
|-|                                   PC3259  |-|
 |                                             |
 |---------------------------------------------|
Notes:
      MG12/13    - Hitachi HN462532 4kx8 EPROM
      MG14/15/16 - 82S123 bipolar PROM
      PC3259     - PC3259 8025 H08 unknown DIP24 IC. Package design indicates it was manufactured by Fujitsu
Lower board
AA017779
sticker: MGN00003
sticker: CLN00002
 |---------------------------------------------|
 | 9.987MHz               2114                 |
|-|                       2114                 |
|R|             MG07.10D       2114            |
| |             MG06.9D        2114            |
|-|                            2114           1|
 |              MG05.7D                       8|Edge
 |              MG04.6D                       W|Connector 'T'
|-|             MG03.5D                       A|
|S|             MG02.4D                       Y|
| |             MG01.3D  MG09.4F               |
|-|                      MG10.3F               |
 |              Z80      MG11.1F               |
 |---------------------------------------------|
Notes:
      Z80  - Clock 2.49675MHz [9.987/4]
      2114 - 1kx4 SRAM
      All EPROMs are 2716
      Wire jumpers for ROM configuration - J1 open
                                           J2 1-2, 3-9, 4-8, 5-7
                                           J4 1-2, 4-5, 7-8, 10-11
Top and Middle PCBs are plugged in with the solder-sides together.
Lower PCB is plugged in with components facing up.
	
***************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "screen.h"
#include "speaker.h"

#define MAIN_CLOCK XTAL_9_987MHz

class marinedt_state : public driver_device
{
public:
	marinedt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_vram(*this, "vram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_in_track(*this, {"P1_TRACKX", "P2_TRACKX", "P1_TRACKY", "P2_TRACKY"})
	{
	}

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(marinedt);
	DECLARE_READ8_MEMBER(trackball_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_WRITE8_MEMBER(obj_0_w);
	DECLARE_WRITE8_MEMBER(obj_1_w);
	DECLARE_WRITE8_MEMBER(layer_enable_w);
	DECLARE_WRITE8_MEMBER(output_w);
	TILE_GET_INFO_MEMBER(get_tile_info); 

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_vram;
	required_device<gfxdecode_device> m_gfxdecode; 
	required_ioport_array<4> m_in_track;

	tilemap_t *m_tilemap;
	std::unique_ptr<bitmap_ind16> m_seabitmap;
	struct
	{
		uint8_t offs;
		uint8_t x;
		uint8_t y;
		bitmap_ind16 bitmap;
	}m_obj[2];
	uint8_t m_layer_en;
	uint8_t m_in_select;
	bool m_screen_flip;
	
	void init_seabitmap();
	void obj_reg_w(uint8_t which,uint8_t reg, uint8_t data);
};

TILE_GET_INFO_MEMBER(marinedt_state::get_tile_info)
{
	int code = m_vram[tile_index];

	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}

// initialize sea bitmap gradient
void marinedt_state::init_seabitmap()
{
	const rectangle clip(32, 256, 32, 256);
	m_seabitmap = std::make_unique<bitmap_ind16>(512, 512);

	m_seabitmap->fill(64, clip);	

	for (int y = clip.min_y; y <= clip.max_y; y++)
	{
		for (int x = clip.min_x; x <= clip.max_x; x++)
		{
			// TODO: exact formula (related to total h size?)
			uint8_t blue_pen = 0x48 + ((x-32) / 8);
			// clamp
			if(blue_pen > 0x5f)
				blue_pen = 0x5f;
			
			m_seabitmap->pix16(y, x) = blue_pen;
		}
	}
}

void marinedt_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(marinedt_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	
	m_tilemap->set_transparent_pen(0);
	
	init_seabitmap();
	
//	m_obj[0].bitmap = std::make_unique<bitmap_ind16>(512, 512);
//	m_obj[1].bitmap = std::make_unique<bitmap_ind16>(512, 512);

//	m_screen->register_screen_bitmap(m_seabitmap);
	m_screen->register_screen_bitmap(m_obj[0].bitmap);
	m_screen->register_screen_bitmap(m_obj[1].bitmap);
	
	save_item(NAME(m_obj[0].x));
	save_item(NAME(m_obj[0].y));
	save_item(NAME(m_obj[0].offs));
	save_item(NAME(m_obj[1].x));
	save_item(NAME(m_obj[1].y));
	save_item(NAME(m_obj[1].offs));
	save_item(NAME(m_obj[0].bitmap));
	save_item(NAME(m_obj[1].bitmap));
	save_item(NAME(m_layer_en));
}

uint32_t marinedt_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	if(m_layer_en & 8)
		copybitmap(bitmap, *m_seabitmap, m_screen_flip == false, m_screen_flip == false, m_screen_flip ? 0 : -256, m_screen_flip ? 0 : -224, cliprect);
	else
		bitmap.fill(0,cliprect);

	if(m_layer_en & 2)
		copybitmap_trans(bitmap, m_obj[1].bitmap, 0, 0, 0, 0, cliprect, 0); 
	if(m_layer_en & 1)
		copybitmap_trans(bitmap, m_obj[0].bitmap, 0, 0, 0, 0, cliprect, 0); 

	m_tilemap->draw(screen, bitmap, cliprect, 0, 0); 
	return 0;
}

WRITE8_MEMBER(marinedt_state::vram_w)
{
	m_vram[offset] = data;
	m_tilemap->mark_tile_dirty(offset); 
}

inline void marinedt_state::obj_reg_w(uint8_t which, uint8_t reg,uint8_t data)
{
	rectangle visarea = m_screen->visible_area();
	//const uint8_t base_pen;// = which == 0 ? 0x30 : 0x20;
	gfx_element *gfx = m_gfxdecode->gfx(which+1);
	
	switch(reg)
	{
		case 0: m_obj[which].offs = data; break;
		case 1: m_obj[which].x = data; break;
		case 2: m_obj[which].y = data; break;
	}
	
	const uint8_t tilenum = ((m_obj[which].offs & 4) << 1) | ( (m_obj[which].offs & 0x38) >> 3);
	const uint8_t color = (m_obj[which].offs & 3);
	const bool fx = m_screen_flip;
	const bool fy = BIT(m_obj[which].offs,7);
	
	//base_pen = (which == 0 ? 0x30 : 0x20) + color*4;
	m_obj[which].bitmap.fill(0,visarea);
	// redraw sprite in framebuffer using above
	// bitmap,cliprect,tilenum,color,flipx,flipy,xpos,ypos,transpen
	gfx->transpen(m_obj[which].bitmap,visarea,tilenum,color,fx,!fy,m_obj[which].x,m_obj[which].y,0);
}

WRITE8_MEMBER(marinedt_state::obj_0_w) { obj_reg_w(0,offset,data); }
WRITE8_MEMBER(marinedt_state::obj_1_w) { obj_reg_w(1,offset,data); }

READ8_MEMBER(marinedt_state::trackball_r)
{
	return (m_in_track[m_in_select & 3])->read();
}

WRITE8_MEMBER(marinedt_state::layer_enable_w) 
{
	/*
		---- x--- sea layer draw enable (disabled in test mode)
		---- --x- obj 2 draw enable
		---- ---x obj 1 draw enable
	*/
	m_layer_en = data;
}

WRITE8_MEMBER(marinedt_state::output_w)
{
	/*
		---- x--- trackball input select (x/y)
		---- -x-- trackball player select
		---- --x- flipscreen
		---- ---x global coin lockout (disabled in service mode)
	*/
	
	m_in_select = (data & 0xc) >> 2;
	m_screen_flip = BIT(data,1);
	flip_screen_set(!m_screen_flip);
	machine().bookkeeping().coin_lockout_global_w(!(data & 1));
}

static ADDRESS_MAP_START( marinedt_map, AS_PROGRAM, 8, marinedt_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff) /* A15 is not decoded */
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("ipl",0)
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0x0400) AM_RAM	
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0x0400) AM_RAM_WRITE(vram_w) AM_SHARE("vram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( marinedt_io, AS_IO, 8, marinedt_state )
	ADDRESS_MAP_GLOBAL_MASK(0x0f)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSW1")
	AM_RANGE(0x01, 0x01) AM_READ(trackball_r)
	AM_RANGE(0x02, 0x04) AM_WRITE(obj_0_w)
	AM_RANGE(0x03, 0x03) AM_READ_PORT("SYSTEM") 
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW2")
	AM_RANGE(0x08, 0x0b) AM_WRITE(obj_1_w)
	AM_RANGE(0x0d, 0x0d) AM_WRITE(layer_enable_w)
	AM_RANGE(0x0e, 0x0e) AM_WRITENOP // watchdog
	AM_RANGE(0x0f, 0x0f) AM_WRITE(output_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( marinedt )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Ink Button")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Ink Button") PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )
	
	// TODO: diplocations needs to be verified
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:4,3,2,1")
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:8,7,6,5")
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )
	
	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DSWB" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
 	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_HIGH, "SWB:3")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x10, "Number of Coin Chutes") PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPNAME( 0x20, 0x00, "Year Display" ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:8,7")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0xc0, "6" )

	PORT_START("P1_TRACKX")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("P1_TRACKY")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("P2_TRACKX")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL

	PORT_START("P2_TRACKY")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_COCKTAIL
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(7,-1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout objlayout =
{
	32,32,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(32*8*7,1), STEP4(32*8*6,1), STEP4(32*8*5,1), STEP4(32*8*4,1), STEP4(32*8*3,1), STEP4(32*8*2,1), STEP4(32*8*1,1), STEP4(32*8*0,1) },
	{ STEP16(0,8), STEP16(16*8,8) },
	32*32*2
};

static GFXDECODE_START( marinedt )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, objlayout,     48, 4 )
	GFXDECODE_ENTRY( "gfx3", 0, objlayout,     32, 4 )
GFXDECODE_END


void marinedt_state::machine_start()
{
}

void marinedt_state::machine_reset()
{
	m_layer_en = 0;
}


PALETTE_INIT_MEMBER(marinedt_state, marinedt)
{
	const uint8_t *color_prom = memregion("proms")->base();
	int i;
	int bit0, bit1, bit2;
	int r, g, b;
	
	for (i = 0; i < 64; i++)
	{
		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = (0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = (0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		// TODO: blue arrangement wrong according to screenshot, gameplay border should be red instead of blue
		b = (0x47 * bit0 + 0x97 * bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}

	
	for (i = 0; i < 32; i++)
	{
		b = color_prom[i+0x60];
		palette.set_pen_color(64+31-i, rgb_t(0, 0, b));
	}
}

static MACHINE_CONFIG_START( marinedt )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,MAIN_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(marinedt_map)
	MCFG_CPU_IO_MAP(marinedt_io)
 	MCFG_CPU_VBLANK_INT_DRIVER("screen", marinedt_state,  irq0_line_hold)
	
	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(marinedt_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS(MAIN_CLOCK/2, 328, 0, 256, 263, 32, 256) // template to get ~60 fps
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", marinedt)

	MCFG_PALETTE_ADD("palette", 64+32)
	MCFG_PALETTE_INIT_OWNER(marinedt_state, marinedt)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Machine driver(s)

***************************************************************************/

ROM_START( marinedt )
	ROM_REGION( 0x4000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "mg01.3d",     0x0000, 0x0800, CRC(ad09f04d) SHA1(932fc973b4a2fbbebd7e6437ed30c8444e3d4afb))
	ROM_LOAD( "mg02.4d",     0x0800, 0x0800, CRC(555a2b0f) SHA1(143a8953ce5070c31dc4c1f623833b2a5a2cf657))
	ROM_LOAD( "mg03.5d",     0x1000, 0x0800, CRC(2abc79b3) SHA1(1afb331a2c0e320b6d026bc5cb47a53ac3356c2a))
	ROM_LOAD( "mg04.6d",     0x1800, 0x0800, CRC(be928364) SHA1(8d9ae71e2751c009187e41d84fbad9519ab551e1) )
	ROM_LOAD( "mg05.7d",     0x2000, 0x0800, CRC(44cd114a) SHA1(833165c5c00c6e505acf29fef4a3ae3f9647b443) )
	ROM_LOAD( "mg06.9d",     0x2800, 0x0800, CRC(a7e2c69b) SHA1(614fc479d13c1726382fe7b4b0379c1dd4915af0) )
	ROM_LOAD( "mg07.10d",    0x3000, 0x0800, CRC(b85d1f9a) SHA1(4fd3e76b1816912df84477dba4655d395f5e7072) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "mg11.1f",     0x0000, 0x0800, CRC(50d66dd7) SHA1(858d1d2a75e091b0e382d964c5e4ddcd8e6f07dd))
	ROM_LOAD( "mg10.3f",     0x0800, 0x0800, CRC(b41251e3) SHA1(e125a971b401c78efeb4b03d0fab43e392d3fc14) )
	ROM_LOAD( "mg09.4f",     0x1000, 0x0800, CRC(f4c349ca) SHA1(077f65eeac616a778d6c42bb95677fa2892ab697) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "mg12.6c",     0x0000, 0x1000, CRC(7c6486d5) SHA1(a7f17a803937937f05fc90621883a0fd44b297a0) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "mg13.6h",     0x0000, 0x1000, CRC(17817044) SHA1(8c9b96620e3c414952e6d85c6e81b0df85c88e7a) )

	ROM_REGION( 0x0080, "proms", ROMREGION_INVERT )
	ROM_LOAD( "mg14.2a",  0x0000, 0x0020, CRC(f75f4e3a) SHA1(36e665987f475c57435fa8c224a2a3ce0c5e672b) ) // tilemap colors
	ROM_LOAD( "mg15.1a",  0x0020, 0x0020, CRC(cd3ab489) SHA1(a77478fb94d0cf8f4317f89cc9579def7c294b4f) ) // sprite colors
	ROM_LOAD( "mg16.4e",  0x0040, 0x0020, CRC(92c868bc) SHA1(483ae6f47845ddacb701528e82bd388d7d66a0fb) ) // (related to sprites)
	ROM_LOAD( "mg17.bpr", 0x0060, 0x0020, CRC(13261a02) SHA1(050edd18e4f79d19d5206f55f329340432fd4099) ) // sea bitmap colors
ROM_END

GAME( 1981, marinedt,  0,   marinedt,  marinedt, marinedt_state,  0,       ROT90, "Taito",      "Marine Date", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_COLORS | MACHINE_NO_SOUND )
