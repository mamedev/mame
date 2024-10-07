// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*****************************************************************************************************************

    Marine Date (c) 1981 Taito

    driver by Angelo Salese,
    original "wiped off due of not anymore licenseable" driver by insideoutboy.

    TODO:
    - discrete sound
    - imperfect colors: unused bit 2 of color prom, guessworked sea gradient, mg16 entirely unused.
      also unused colors 0x10-0x1f (might be a flashing bank)
    - collision detection isn't perfect, sometimes octopus gets stuck and dies even if moves are still available.
      HW collision detection isn't perfect even from the reference, presumably needs a trojan run on the real HW.
    - ROM writes (irq mask?)
    - Merge devices with crbaloon/bking/grchamp drivers (PC3259).
    - Currently defaults to cocktail instead of upright. When upright chosen, screen is upside down. (MT 07311)

*****************************************************************************************************************

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

*****************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

#define MAIN_CLOCK XTAL(9'987'000)

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

	void marinedt(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;

private:
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void marinedt_palette(palette_device &palette) const;
	uint8_t trackball_r();
	uint8_t pc3259_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	void obj_0_w(offs_t offset, uint8_t data);
	void obj_1_w(offs_t offset, uint8_t data);
	void bgm_w(uint8_t data);
	void sfx_w(uint8_t data);
	void layer_enable_w(uint8_t data);
	void output_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);

	void marinedt_io(address_map &map) ATTR_COLD;
	void marinedt_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_vram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_ioport_array<4> m_in_track;

	tilemap_t *m_tilemap = nullptr;
	std::unique_ptr<bitmap_ind16> m_seabitmap[2];
	struct
	{
		uint8_t offs = 0;
		uint8_t x = 0;
		uint8_t y = 0;
		bitmap_ind16 bitmap;
	}m_obj[2];

	uint8_t m_layer_en = 0;
	uint8_t m_in_select = 0;
	bool m_screen_flip = false;
	uint8_t m_sea_bank = 0;

	void init_seabitmap();
	void obj_reg_w(uint8_t which,uint8_t reg, uint8_t data);
	uint32_t obj_to_obj_collision();
	uint32_t obj_to_layer_collision();
};

TILE_GET_INFO_MEMBER(marinedt_state::get_tile_info)
{
	int code = m_vram[tile_index];

	tileinfo.set(0, code, 0, 0);
}

// initialize sea bitmap gradient
void marinedt_state::init_seabitmap()
{
	const rectangle clip(32, 256, 32, 256);
	m_seabitmap[0] = std::make_unique<bitmap_ind16>(512, 512);
	m_seabitmap[1] = std::make_unique<bitmap_ind16>(512, 512);

	m_seabitmap[0]->fill(64, clip);
	m_seabitmap[1]->fill(64+32, clip);

	for (int y = clip.min_y; y <= clip.max_y; y++)
	{
		for (int x = clip.min_x; x <= clip.max_x; x++)
		{
			// TODO: exact formula (related to total h size?)
			uint8_t blue_pen = 0x48 + ((x-32) / 8);
			// clamp
			if(blue_pen > 0x5f)
				blue_pen = 0x5f;

			m_seabitmap[0]->pix(y, x) = blue_pen;
			m_seabitmap[1]->pix(y, x) = blue_pen+0x20;
		}
	}
}

void marinedt_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(marinedt_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_tilemap->set_transparent_pen(0);

	init_seabitmap();

//  m_obj[0].bitmap = std::make_unique<bitmap_ind16>(512, 512);
//  m_obj[1].bitmap = std::make_unique<bitmap_ind16>(512, 512);

//  m_screen->register_screen_bitmap(m_seabitmap);
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
	save_item(NAME(m_sea_bank));
}

uint32_t marinedt_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	if(m_layer_en & 8)
		copybitmap(bitmap, *m_seabitmap[m_sea_bank], m_screen_flip == false, m_screen_flip == false, m_screen_flip ? 0 : -256, m_screen_flip ? 0 : -224, cliprect);
	else
		bitmap.fill(0,cliprect);

	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	if(m_layer_en & 2)
		copybitmap_trans(bitmap, m_obj[1].bitmap, 0, 0, 0, 0, cliprect, 0);
	if(m_layer_en & 1)
		copybitmap_trans(bitmap, m_obj[0].bitmap, 0, 0, 0, 0, cliprect, 0);

	return 0;
}

void marinedt_state::vram_w(offs_t offset, uint8_t data)
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
		// TODO: are offsets good?
		case 1: m_obj[which].x = (data + 4) & 0xff; break;
		case 2: m_obj[which].y = (data + 1) & 0xff; break;
	}

	const uint8_t tilenum = ((m_obj[which].offs & 4) << 1) | ( (m_obj[which].offs & 0x38) >> 3);
	const uint8_t color = (m_obj[which].offs & 3);
	const bool fx = BIT(m_obj[which].offs,6);
	const bool fy = BIT(m_obj[which].offs,7);

	//base_pen = (which == 0 ? 0x30 : 0x20) + color*4;
	m_obj[which].bitmap.fill(0,visarea);
	// redraw sprite in framebuffer using above
	// bitmap,cliprect,tilenum,color,flipx,flipy,xpos,ypos,transpen
	gfx->transpen(m_obj[which].bitmap,visarea,tilenum,color,fx,!fy,m_obj[which].x,m_obj[which].y,0);
}

void marinedt_state::obj_0_w(offs_t offset, uint8_t data) { obj_reg_w(0,offset,data); }
void marinedt_state::obj_1_w(offs_t offset, uint8_t data) { obj_reg_w(1,offset,data); }

uint8_t marinedt_state::trackball_r()
{
	return (m_in_track[m_in_select & 3])->read();
}

// discrete sound
void marinedt_state::bgm_w(uint8_t data)
{
	// ...
}

void marinedt_state::sfx_w(uint8_t data)
{
	/*
	 x--- ---- unknown, probably ties to PC3259 pin 16 like crbaloon
	 --x- ---- jet sound SFX
	 ---x ---- foam SFX
	 ---- x--- ink SFX
	 ---- -x-- collision SFX
	 ---- --x- dots hit SFX
	 ---- ---x irq mask in crbaloon, doesn't seem to apply here?
	 */
//  if(data & 0x7e)
//      popmessage("%02x",data);
}

void marinedt_state::layer_enable_w(uint8_t data)
{
	/*
	    ---x ---- enabled when shark appears (enables red gradient on sea bitmap apparently)
	    ---- x--- sea layer draw enable (disabled in test mode)
	    ---- --x- obj 2 draw enable
	    ---- ---x obj 1 draw enable
	*/
	m_layer_en = data & 0xf;
	m_sea_bank = (data & 0x10) >> 4;
}

void marinedt_state::output_w(uint8_t data)
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

// collision detection
// we return a value in the form of y<<5|x in case collision occurred
inline uint32_t marinedt_state::obj_to_obj_collision()
{
	// bail out if any obj is disabled
	if((m_layer_en & 3) != 3)
		return 0;

	for(int y=0;y<32;y++)
	{
		for(int x=0;x<32;x++)
		{
			int resx,resy;

			resx = m_obj[0].x + x;
			resy = m_obj[0].y + y;

			if((m_obj[0].bitmap.pix(resy,resx) & 3) == 0)
				continue;

			// return value is never read most likely
			if(m_obj[1].bitmap.pix(resy,resx) != 0)
				return ((resy / 8) * 32) | (((resx / 8) - 1) & 0x1f);
		}
	}

	return 0;
}

inline uint32_t marinedt_state::obj_to_layer_collision()
{
	// bail out if obj target is disabled
	if((m_layer_en & 1) == 0)
		return 0;

	for(int y=0;y<32;y++)
	{
		for(int x=0;x<32;x++)
		{
			uint16_t resx,resy;

			resx = m_obj[0].x + x;
			resy = m_obj[0].y + y;

			if((m_obj[0].bitmap.pix(resy,resx) & 3) == 0)
				continue;

			if(!m_screen_flip)
				resy -= 32;

			// TODO: non screen flip path doesn't work properly
			if(m_tilemap->pixmap().pix(resy,resx) != 0)
			{
				if(m_screen_flip)
					return ((resy / 8) * 32) | (((resx / 8) - 1) & 0x1f);
				else
					return (((resy / 8) * 32) | (((resx / 8) - 1) & 0x1f)) ^ 0x3ff;
			}
		}
	}

	return 0;
}

uint8_t marinedt_state::pc3259_r(offs_t offset)
{
	uint32_t rest,reso;
	uint8_t reg = offset >> 2;
	uint8_t xt,xo;
	rest = obj_to_layer_collision();
	reso = obj_to_obj_collision();

	switch(reg)
	{
		case 0:
			xt = rest & 0xf;
			xo = reso & 0xf;
			return xt|(xo<<4);
		case 1:
			xt = (rest & 0xf0) >> 4;
			xo = (reso & 0xf0) >> 4;
			return xt|(xo<<4);
		case 2:
			xt = (rest & 0x300) >> 8;
			xo = (reso & 0x300) >> 8;
			return xt|(xo<<4);
		case 3:
		{
			uint8_t res = 0;
			res |= ((reso != 0)<<7);
			res |= ((rest != 0)<<3);
			return res;
		}
	}


	return 0;
}

void marinedt_state::marinedt_map(address_map &map)
{
	map.global_mask(0x7fff); /* A15 is not decoded */
	map(0x0000, 0x3fff).rom().region("ipl", 0);
	map(0x4000, 0x43ff).mirror(0x0400).ram();
	map(0x4800, 0x4bff).mirror(0x0400).ram().w(FUNC(marinedt_state::vram_w)).share("vram");
}

void marinedt_state::marinedt_io(address_map &map)
{
	map.global_mask(0x0f);
	map(0x00, 0x00).portr("DSW1");
	map(0x01, 0x01).r(FUNC(marinedt_state::trackball_r));
	map(0x02, 0x02).select(0xc).r(FUNC(marinedt_state::pc3259_r));
	map(0x02, 0x04).w(FUNC(marinedt_state::obj_0_w));
	map(0x03, 0x03).portr("SYSTEM");
	map(0x04, 0x04).portr("DSW2");
	map(0x05, 0x05).w(FUNC(marinedt_state::bgm_w));
	map(0x06, 0x06).w(FUNC(marinedt_state::sfx_w));
	map(0x08, 0x0b).w(FUNC(marinedt_state::obj_1_w));
	map(0x0d, 0x0d).w(FUNC(marinedt_state::layer_enable_w));
	map(0x0e, 0x0e).nopw(); // watchdog
	map(0x0f, 0x0f).w(FUNC(marinedt_state::output_w));
}

static INPUT_PORTS_START( marinedt )
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
	PORT_DIPNAME( 0x02, 0x00, "Disable sprite-tile collision (Cheat)" ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_HIGH, "SWB:3")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SWB:4")
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

static GFXDECODE_START( gfx_marinedt )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 4 )
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


void marinedt_state::marinedt_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 64; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		//bit2 = BIT(color_prom[i], 2);
		int const r = (0x55 * bit0) + (0xaa * bit1);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		int const g = (0x55 * bit0) + (0xaa * bit1);

		// blue component
		bit0 = BIT(color_prom[i], 5);
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int b = (0x55 * bit0) + (0xaa * bit1);
		// matches yellow haired siren
		if (bit2 == 0)
			b /= 2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}

	for (int i = 0; i < 32; i++)
	{
		int const b = color_prom[i + 0x60];
		palette.set_pen_color(64 + 31 - i, rgb_t(0, 0, b));
		palette.set_pen_color(64 + 63 - i, rgb_t(0xff, 0, b));
	}
}

void marinedt_state::marinedt(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MAIN_CLOCK/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &marinedt_state::marinedt_map);
	m_maincpu->set_addrmap(AS_IO, &marinedt_state::marinedt_io);
	m_maincpu->set_vblank_int("screen", FUNC(marinedt_state::irq0_line_hold));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(marinedt_state::screen_update));
	m_screen->set_raw(MAIN_CLOCK/2, 328, 0, 256, 263, 32, 256); // template to get ~60 fps
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_marinedt);

	PALETTE(config, "palette", FUNC(marinedt_state::marinedt_palette), 64 + 64);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	//AY8910(config, "aysnd", MAIN_CLOCK/4).add_route(ALL_OUTPUTS, "mono", 0.30);
}


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

} // anonymous namespace


GAME( 1981, marinedt, 0, marinedt, marinedt, marinedt_state, empty_init, ROT270, "Taito", "Marine Date", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_COLORS | MACHINE_NO_SOUND )
