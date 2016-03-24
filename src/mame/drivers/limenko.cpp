// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Tomasz Slanina
/*

  Limenko Power System 2

  driver by Pierpaolo Prazzoli and Tomasz Slanina

  Power System 2 General specs:
  - Cartridge Based System
  - Hyperstone E1-32XN CPU
  - QDSP QS1000 Sound Hardware

  Games Supported:
  - Dynamite Bomber (Korea) (Rev 1.5)
  - Legend of Heroes
  - Super Bubble 2003 (2 sets)

  Known Games Not Dumped:
  - Happy Hunter (shooting themed prize game)

  To Do:
  - Legend of Heroes link up, 2 cabinets can be linked for a 4 player game

*/

#include "emu.h"
#include "cpu/e132xs/e132xs.h"
#include "machine/eepromser.h"
#include "sound/qs1000.h"
#include "sound/okim6295.h"


class limenko_state : public driver_device
{
public:
	limenko_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_qs1000(*this, "qs1000"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_mainram(*this, "mainram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_md_videoram(*this, "md_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_videoreg(*this, "videoreg") { }

	required_device<cpu_device> m_maincpu;
	optional_device<okim6295_device> m_oki;
	optional_device<qs1000_device> m_qs1000;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT32> m_mainram;
	required_shared_ptr<UINT32> m_fg_videoram;
	required_shared_ptr<UINT32> m_md_videoram;
	required_shared_ptr<UINT32> m_bg_videoram;
	required_shared_ptr<UINT32> m_spriteram;
	required_shared_ptr<UINT32> m_spriteram2;
	required_shared_ptr<UINT32> m_videoreg;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_md_tilemap;
	tilemap_t *m_fg_tilemap;

	int m_spriteram_bit;
	bitmap_ind16 m_sprites_bitmap;
	bitmap_ind8 m_sprites_bitmap_pri;
	int m_prev_sprites_count;
	UINT8 m_spotty_sound_cmd;

	DECLARE_WRITE32_MEMBER(limenko_coincounter_w);
	DECLARE_WRITE32_MEMBER(bg_videoram_w);
	DECLARE_WRITE32_MEMBER(md_videoram_w);
	DECLARE_WRITE32_MEMBER(fg_videoram_w);
	DECLARE_WRITE32_MEMBER(spotty_soundlatch_w);
	DECLARE_WRITE32_MEMBER(limenko_soundlatch_w);
	DECLARE_WRITE32_MEMBER(spriteram_buffer_w);
	DECLARE_WRITE8_MEMBER(spotty_sound_cmd_w);
	DECLARE_READ8_MEMBER(spotty_sound_cmd_r);
	DECLARE_READ8_MEMBER(spotty_sound_r);
	DECLARE_READ32_MEMBER(dynabomb_speedup_r);
	DECLARE_READ32_MEMBER(legendoh_speedup_r);
	DECLARE_READ32_MEMBER(sb2003_speedup_r);
	DECLARE_READ32_MEMBER(spotty_speedup_r);
	DECLARE_READ8_MEMBER(qs1000_p1_r);
	DECLARE_WRITE8_MEMBER(qs1000_p1_w);
	DECLARE_WRITE8_MEMBER(qs1000_p2_w);
	DECLARE_WRITE8_MEMBER(qs1000_p3_w);

	DECLARE_CUSTOM_INPUT_MEMBER(spriteram_bit_r);

	DECLARE_DRIVER_INIT(common);
	DECLARE_DRIVER_INIT(sb2003);
	DECLARE_DRIVER_INIT(dynabomb);
	DECLARE_DRIVER_INIT(legendoh);
	DECLARE_DRIVER_INIT(spotty);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_md_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	virtual void video_start() override;
	UINT32 screen_update_limenko(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_single_sprite(bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,int priority);
	void draw_sprites(UINT32 *sprites, const rectangle &cliprect, int count);
	void copy_sprites(bitmap_ind16 &bitmap, bitmap_ind16 &sprites_bitmap, bitmap_ind8 &priority_bitmap, const rectangle &cliprect);
};

/*****************************************************************************************************
  MISC FUNCTIONS
*****************************************************************************************************/

WRITE32_MEMBER(limenko_state::limenko_coincounter_w)
{
	machine().bookkeeping().coin_counter_w(0,data & 0x10000);
}



WRITE32_MEMBER(limenko_state::bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE32_MEMBER(limenko_state::md_videoram_w)
{
	COMBINE_DATA(&m_md_videoram[offset]);
	m_md_tilemap->mark_tile_dirty(offset);
}

WRITE32_MEMBER(limenko_state::fg_videoram_w)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

CUSTOM_INPUT_MEMBER(limenko_state::spriteram_bit_r)
{
	return m_spriteram_bit;
}

WRITE32_MEMBER(limenko_state::spriteram_buffer_w)
{
	rectangle clip(0, 383, 0, 239);

	m_sprites_bitmap_pri.fill(0, clip);
	m_sprites_bitmap.fill(0, clip);

	// toggle spriterams location in the memory map
	m_spriteram_bit ^= 1;

	if(m_spriteram_bit)
	{
		// draw the sprites to the frame buffer
		draw_sprites(m_spriteram2,clip,m_prev_sprites_count);
	}
	else
	{
		// draw the sprites to the frame buffer
		draw_sprites(m_spriteram,clip,m_prev_sprites_count);
	}

	// buffer the next number of sprites to draw
	m_prev_sprites_count = (m_videoreg[0] & 0x1ff0000) >> 16;
}

/*****************************************************************************************************
 SOUND FUNCTIONS
 *****************************************************************************************************/

WRITE32_MEMBER(limenko_state::limenko_soundlatch_w)
{
	soundlatch_byte_w(space, 0, data >> 16);
	m_qs1000->set_irq(ASSERT_LINE);

	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}

WRITE32_MEMBER(limenko_state::spotty_soundlatch_w)
{
	soundlatch_byte_w(space, 0, data >> 16);
}

READ8_MEMBER(limenko_state::qs1000_p1_r)
{
	return soundlatch_byte_r(space, 0);
}

WRITE8_MEMBER(limenko_state::qs1000_p1_w)
{
}

WRITE8_MEMBER(limenko_state::qs1000_p2_w)
{
	// Unknown. Often written with 0
}

WRITE8_MEMBER(limenko_state::qs1000_p3_w)
{
	// .... .xxx - Data ROM bank (64kB)
	// ...x .... - ?
	// ..x. .... - /IRQ clear

	membank("qs1000:bank")->set_entry(data & 0x07);

	if (!BIT(data, 5))
		m_qs1000->set_irq(CLEAR_LINE);
}

/*****************************************************************************************************
  MEMORY MAPS
*****************************************************************************************************/

static ADDRESS_MAP_START( limenko_map, AS_PROGRAM, 32, limenko_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x40000000, 0x403fffff) AM_ROM AM_REGION("user2",0)
	AM_RANGE(0x80000000, 0x80007fff) AM_RAM_WRITE(fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0x80008000, 0x8000ffff) AM_RAM_WRITE(md_videoram_w) AM_SHARE("md_videoram")
	AM_RANGE(0x80010000, 0x80017fff) AM_RAM_WRITE(bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x80018000, 0x80018fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x80019000, 0x80019fff) AM_RAM AM_SHARE("spriteram2")
	AM_RANGE(0x8001c000, 0x8001dfff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x8001e000, 0x8001ebff) AM_RAM // ? not used
	AM_RANGE(0x8001ffec, 0x8001ffff) AM_RAM AM_SHARE("videoreg")
	AM_RANGE(0x8003e000, 0x8003e003) AM_WRITE(spriteram_buffer_w)
	AM_RANGE(0xffe00000, 0xffffffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( limenko_io_map, AS_IO, 32, limenko_state )
	AM_RANGE(0x0000, 0x0003) AM_READ_PORT("IN0")
	AM_RANGE(0x0800, 0x0803) AM_READ_PORT("IN1")
	AM_RANGE(0x1000, 0x1003) AM_READ_PORT("IN2")
	AM_RANGE(0x4000, 0x4003) AM_WRITE(limenko_coincounter_w)
	AM_RANGE(0x4800, 0x4803) AM_WRITE_PORT("EEPROMOUT")
	AM_RANGE(0x5000, 0x5003) AM_WRITE(limenko_soundlatch_w)
ADDRESS_MAP_END


/* Spotty memory map */

static ADDRESS_MAP_START( spotty_map, AS_PROGRAM, 32, limenko_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x40002000, 0x400024d3) AM_RAM //?
	AM_RANGE(0x80000000, 0x80007fff) AM_RAM_WRITE(fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0x80008000, 0x8000ffff) AM_RAM_WRITE(md_videoram_w) AM_SHARE("md_videoram")
	AM_RANGE(0x80010000, 0x80017fff) AM_RAM_WRITE(bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x80018000, 0x80018fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x80019000, 0x80019fff) AM_RAM AM_SHARE("spriteram2")
	AM_RANGE(0x8001c000, 0x8001dfff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x8001e000, 0x8001ebff) AM_RAM // ? not used
	AM_RANGE(0x8001ffec, 0x8001ffff) AM_RAM AM_SHARE("videoreg")
	AM_RANGE(0x8003e000, 0x8003e003) AM_WRITE(spriteram_buffer_w)
	AM_RANGE(0xfff00000, 0xffffffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( spotty_io_map, AS_IO, 32, limenko_state )
	AM_RANGE(0x0000, 0x0003) AM_READ_PORT("IN0")
	AM_RANGE(0x0800, 0x0803) AM_READ_PORT("IN1")
	AM_RANGE(0x0800, 0x0803) AM_WRITENOP // hopper related
	AM_RANGE(0x1000, 0x1003) AM_READ_PORT("IN2")
	AM_RANGE(0x4800, 0x4803) AM_WRITE_PORT("EEPROMOUT")
	AM_RANGE(0x5000, 0x5003) AM_WRITE(spotty_soundlatch_w)
ADDRESS_MAP_END

WRITE8_MEMBER(limenko_state::spotty_sound_cmd_w)
{
	m_spotty_sound_cmd = data;
}

READ8_MEMBER(limenko_state::spotty_sound_cmd_r)
{
	return 0; //??? some status bit? if set it executes a jump in the code
}

READ8_MEMBER(limenko_state::spotty_sound_r)
{
	// check m_spotty_sound_cmd bits...

	if(m_spotty_sound_cmd == 0xf7)
		return soundlatch_byte_r(space,0);
	else
		return m_oki->read(space,0);
}

static ADDRESS_MAP_START( spotty_sound_io_map, AS_IO, 8, limenko_state )
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_READ(spotty_sound_r) AM_DEVWRITE("oki", okim6295_device, write) //? sound latch and ?
	AM_RANGE(MCS51_PORT_P3, MCS51_PORT_P3) AM_READWRITE(spotty_sound_cmd_r, spotty_sound_cmd_w) //not sure about anything...
ADDRESS_MAP_END

/*****************************************************************************************************
  VIDEO HARDWARE EMULATION
*****************************************************************************************************/

TILE_GET_INFO_MEMBER(limenko_state::get_bg_tile_info)
{
	int tile  = m_bg_videoram[tile_index] & 0x7ffff;
	int color = (m_bg_videoram[tile_index]>>28) & 0xf;
	SET_TILE_INFO_MEMBER(0,tile,color,0);
}

TILE_GET_INFO_MEMBER(limenko_state::get_md_tile_info)
{
	int tile  = m_md_videoram[tile_index] & 0x7ffff;
	int color = (m_md_videoram[tile_index]>>28) & 0xf;
	SET_TILE_INFO_MEMBER(0,tile,color,0);
}

TILE_GET_INFO_MEMBER(limenko_state::get_fg_tile_info)
{
	int tile  = m_fg_videoram[tile_index] & 0x7ffff;
	int color = (m_fg_videoram[tile_index]>>28) & 0xf;
	SET_TILE_INFO_MEMBER(0,tile,color,0);
}

void limenko_state::draw_single_sprite(bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		int priority)
{
	int pal_base = gfx->colorbase() + gfx->granularity() * (color % gfx->colors());
	const UINT8 *source_base = gfx->get_data(code % gfx->elements());

	int sprite_screen_height = ((1<<16)*gfx->height()+0x8000)>>16;
	int sprite_screen_width = ((1<<16)*gfx->width()+0x8000)>>16;

	if (sprite_screen_width && sprite_screen_height)
	{
		/* compute sprite increment per screen pixel */
		int dx = (gfx->width()<<16)/sprite_screen_width;
		int dy = (gfx->height()<<16)/sprite_screen_height;

		int ex = sx+sprite_screen_width;
		int ey = sy+sprite_screen_height;

		int x_index_base;
		int y_index;

		if( flipx )
		{
			x_index_base = (sprite_screen_width-1)*dx;
			dx = -dx;
		}
		else
		{
			x_index_base = 0;
		}

		if( flipy )
		{
			y_index = (sprite_screen_height-1)*dy;
			dy = -dy;
		}
		else
		{
			y_index = 0;
		}

		if( sx < clip.min_x)
		{ /* clip left */
			int pixels = clip.min_x-sx;
			sx += pixels;
			x_index_base += pixels*dx;
		}
		if( sy < clip.min_y )
		{ /* clip top */
			int pixels = clip.min_y-sy;
			sy += pixels;
			y_index += pixels*dy;
		}
		/* NS 980211 - fixed incorrect clipping */
		if( ex > clip.max_x+1 )
		{ /* clip right */
			int pixels = ex-clip.max_x-1;
			ex -= pixels;
		}
		if( ey > clip.max_y+1 )
		{ /* clip bottom */
			int pixels = ey-clip.max_y-1;
			ey -= pixels;
		}

		if( ex>sx )
		{ /* skip if inner loop doesn't draw anything */
			int y;

			for( y=sy; y<ey; y++ )
			{
				const UINT8 *source = source_base + (y_index>>16) * gfx->rowbytes();
				UINT16 *dest = &dest_bmp.pix16(y);
				UINT8 *pri = &m_sprites_bitmap_pri.pix8(y);

				int x, x_index = x_index_base;
				for( x=sx; x<ex; x++ )
				{
					int c = source[x_index>>16];
					if( c != 0 )
					{
						if (pri[x]<priority)
						{
							dest[x] = pal_base+c;
							pri[x] = priority;
						}

					}
					x_index += dx;
				}

				y_index += dy;
			}
		}
	}
}

// sprites aren't tile based (except for 8x8 ones)
void limenko_state::draw_sprites(UINT32 *sprites, const rectangle &cliprect, int count)
{
	int i;

	UINT8 *base_gfx = memregion("gfx1")->base();
	UINT8 *gfx_max  = base_gfx + memregion("gfx1")->bytes();

	UINT8 *gfxdata;

	for(i = 0; i <= count*2; i += 2)
	{
		int x, width, flipx, y, height, flipy, code, color, pri;

		if(~sprites[i] & 0x80000000) continue;

		x = ((sprites[i] & 0x1ff0000) >> 16);
		width = (((sprites[i] & 0xe000000) >> 25) + 1) * 8;
		flipx = sprites[i] & 0x10000000;
		y = sprites[i] & 0x1ff;
		height = (((sprites[i] & 0xe00) >> 9) + 1) * 8;
		flipy = sprites[i] & 0x1000;
		code = sprites[i + 1] & 0x7ffff;
		color = (sprites[i + 1] & 0xf0000000) >> 28;

		if(sprites[i + 1] & 0x04000000)
		{
			// below fg
			pri = 1;
		}
		else
		{
			// above everything
			pri = 2;
		}

		gfxdata = base_gfx + 64 * code;

		/* Bounds checking */
		if ( (gfxdata + width * height - 1) >= gfx_max )
			continue;

		/* prepare GfxElement on the fly */
		gfx_element gfx(m_palette, gfxdata, width, height, width, m_palette->entries(), 0, 256);

		draw_single_sprite(m_sprites_bitmap,cliprect,&gfx,0,color,flipx,flipy,x,y,pri);

		// wrap around x
		draw_single_sprite(m_sprites_bitmap,cliprect,&gfx,0,color,flipx,flipy,x-512,y,pri);

		// wrap around y
		draw_single_sprite(m_sprites_bitmap,cliprect,&gfx,0,color,flipx,flipy,x,y-512,pri);

		// wrap around x and y
		draw_single_sprite(m_sprites_bitmap,cliprect,&gfx,0,color,flipx,flipy,x-512,y-512,pri);
	}
}

void limenko_state::copy_sprites(bitmap_ind16 &bitmap, bitmap_ind16 &sprites_bitmap, bitmap_ind8 &priority_bitmap, const rectangle &cliprect)
{
	int y;
	for( y=cliprect.min_y; y<=cliprect.max_y; y++ )
	{
		UINT16 *source = &sprites_bitmap.pix16(y);
		UINT16 *dest = &bitmap.pix16(y);
		UINT8 *dest_pri = &priority_bitmap.pix8(y);
		UINT8 *source_pri = &m_sprites_bitmap_pri.pix8(y);

		int x;
		for( x=cliprect.min_x; x<=cliprect.max_x; x++ )
		{
			if( source[x]!= 0 )
			{
				if(dest_pri[x] < source_pri[x])
					dest[x] = source[x];
			}
		}
	}
}

void limenko_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(limenko_state::get_bg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,128,64);
	m_md_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(limenko_state::get_md_tile_info),this),TILEMAP_SCAN_ROWS,8,8,128,64);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(limenko_state::get_fg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,128,64);

	m_md_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

	m_sprites_bitmap.allocate(384,240);
	m_sprites_bitmap_pri.allocate(384,240);

	save_item(NAME(m_spriteram_bit));
	save_item(NAME(m_prev_sprites_count));
}

UINT32 limenko_state::screen_update_limenko(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// m_videoreg[4] ???? It always has this value: 0xffeffff8 (2 signed bytes? values: -17 and -8 ?)

	screen.priority().fill(0, cliprect);

	m_bg_tilemap->enable(m_videoreg[0] & 4);
	m_md_tilemap->enable(m_videoreg[0] & 2);
	m_fg_tilemap->enable(m_videoreg[0] & 1);

	m_bg_tilemap->set_scrolly(0, m_videoreg[3] & 0xffff);
	m_md_tilemap->set_scrolly(0, m_videoreg[2] & 0xffff);
	m_fg_tilemap->set_scrolly(0, m_videoreg[1] & 0xffff);

	m_bg_tilemap->set_scrollx(0, (m_videoreg[3] & 0xffff0000) >> 16);
	m_md_tilemap->set_scrollx(0, (m_videoreg[2] & 0xffff0000) >> 16);
	m_fg_tilemap->set_scrollx(0, (m_videoreg[1] & 0xffff0000) >> 16);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_md_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,1);

	if(m_videoreg[0] & 8)
		copy_sprites(bitmap, m_sprites_bitmap, screen.priority(), cliprect);

	return 0;
}

/*****************************************************************************************************
  INPUT PORTS
*****************************************************************************************************/

static INPUT_PORTS_START( legendoh )
	PORT_START("IN0")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(3)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(3)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(3)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(4)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(4)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(4)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00200000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00400000, IP_ACTIVE_HIGH, IPT_SPECIAL ) //security bit
	PORT_BIT( 0x00800000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_DIPNAME( 0x20000000, 0x00000000, "Sound Enable" )
	PORT_DIPSETTING(          0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x80000000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, limenko_state,spriteram_bit_r, NULL) //changes spriteram location
	PORT_BIT( 0x4000ffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x00010000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0x00020000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x00040000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
//  PORT_BIT( 0x00080000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // 0x80000 -> video disabled?
INPUT_PORTS_END

static INPUT_PORTS_START( sb2003 )
	PORT_START("IN0")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0xff00ffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0xff00ffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x00200000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_SPECIAL ) //security bit
	PORT_BIT( 0x00800000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_DIPNAME( 0x20000000, 0x00000000, "Sound Enable" )
	PORT_DIPSETTING(          0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x80000000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, limenko_state,spriteram_bit_r, NULL) //changes spriteram location
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_SERVICE1 ) // checked in dynabomb I/O test, but doesn't work in game
	PORT_BIT( 0x5f00ffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x00010000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0x00020000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x00040000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
//  PORT_BIT( 0x00080000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // 0x80000 -> video disabled?
INPUT_PORTS_END

static INPUT_PORTS_START( spotty )
	PORT_START("IN0")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_NAME("Hold 1")
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_NAME("Hold 2")
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_NAME("Hold 3")
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Hold 4")
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet")
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Stop")
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Change")
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00ffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Prize Hopper 1")
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Prize Hopper 2")
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Prize Hopper 3")
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00ffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00080000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, limenko_state,spriteram_bit_r, NULL) //changes spriteram location
	PORT_SERVICE_NO_TOGGLE( 0x00200000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_SPECIAL ) //security bit
	PORT_BIT( 0x00800000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_DIPNAME( 0x20000000, 0x20000000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x20000000, DEF_STR( On ) )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x5f10ffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x00010000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0x00020000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x00040000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
//  PORT_BIT( 0x00080000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // 0x80000 -> video disabled?
INPUT_PORTS_END

/*****************************************************************************************************
  GRAPHICS DECODES
*****************************************************************************************************/


static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0,8,16,24,32,40,48,56 },
	{ 64*0,64*1,64*2,64*3,64*4,64*5,64*6,64*7 },
	64*8,
};

static GFXDECODE_START( limenko )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 0, 16 ) /* tiles */
GFXDECODE_END


/*****************************************************************************************************
  MACHINE DRIVERS
*****************************************************************************************************/


static MACHINE_CONFIG_START( limenko, limenko_state )
	MCFG_CPU_ADD("maincpu", E132XN, 20000000*4) /* 4x internal multiplier */
	MCFG_CPU_PROGRAM_MAP(limenko_map)
	MCFG_CPU_IO_MAP(limenko_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", limenko_state,  irq0_line_hold)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(384, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 383, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(limenko_state, screen_update_limenko)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", limenko)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("qs1000", QS1000, XTAL_24MHz)
	MCFG_QS1000_EXTERNAL_ROM(true)
	MCFG_QS1000_IN_P1_CB(READ8(limenko_state, qs1000_p1_r))
	MCFG_QS1000_OUT_P1_CB(WRITE8(limenko_state, qs1000_p1_w))
	MCFG_QS1000_OUT_P2_CB(WRITE8(limenko_state, qs1000_p2_w))
	MCFG_QS1000_OUT_P3_CB(WRITE8(limenko_state, qs1000_p3_w))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( spotty, limenko_state )
	MCFG_CPU_ADD("maincpu", GMS30C2232, 20000000)   /* 20 MHz, no internal multiplier */
	MCFG_CPU_PROGRAM_MAP(spotty_map)
	MCFG_CPU_IO_MAP(spotty_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", limenko_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", AT89C4051, 4000000)    /* 4 MHz */
	MCFG_CPU_IO_MAP(spotty_sound_io_map)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(384, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 383, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(limenko_state, screen_update_limenko)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", limenko)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 4000000 / 4 , OKIM6295_PIN7_HIGH) //?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/*****************************************************************************************************
  ROM LOADING
*****************************************************************************************************/

/*

Dynamite Bomber
Limenko

The main board is identical to the one used on the other Limenko games.
The ROM board is slightly different (much simpler)

REV : LMSYS_B
SEL : B1-06-00
|-----------------------------------------------------------|
|        U4+                 U20(DIP40)&                    |
||-|                                           U19+&     |-||
|| |                                                     | ||
|| |                                                     | ||
|| |     U3+                     U6+                     | ||
|| |                                                     | ||
|| |                                         U18(DIP32)  | ||
|| |                             U5+                     | ||
|| |     U2+                                             | ||
|| |                                         U17(DIP32)  | ||
|| |                                                     | ||
|| |                                                     | ||
||-|     U1+                                 U16(DIP32)  |-||
|                                                           |
|-----------------------------------------------------------|
Notes:
       + - These ROMs surface mounted, type MX29F1610 16MBit SOP44
       & - These locations not populated

*/

ROM_START( dynabomb )
	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD16_WORD_SWAP( "rom.u6", 0x000000, 0x200000, CRC(457e015d) SHA1(3afb56cdf903c9084c1f283dc50ec504ce3e199f) )

	ROM_REGION32_BE( 0x400000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "rom.u5", 0x000000, 0x200000, CRC(7e837adf) SHA1(8613fa187b8d4574b3935aa439aec2515033d64c) )

	ROM_REGION( 0x80000, "qs1000:cpu", 0 ) /* QS1000 CPU */
	ROM_LOAD( "rom.u16", 0x00000, 0x20000, CRC(f66d7e4d) SHA1(44f1851405ba525f1ed53521f4de12545ea9c46a) )
	ROM_FILL(            0x20000, 0x60000, 0xff)

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "rom.u1", 0x000000, 0x200000, CRC(bf33eff6) SHA1(089b6d88d6d744bcfa036c6869f0444d6ceb26c9) )
	ROM_LOAD32_BYTE( "rom.u2", 0x000001, 0x200000, CRC(790bbcd5) SHA1(fc52c15fffc77dc3b3bc89a9606223c4fbaa578c) )
	ROM_LOAD32_BYTE( "rom.u3", 0x000002, 0x200000, CRC(ec094b12) SHA1(13c105df066ff308cc7e1842907644790946e5b5) )
	ROM_LOAD32_BYTE( "rom.u4", 0x000003, 0x200000, CRC(88b24e3c) SHA1(5f267f08144b413b55ef5e15c52e9cda096b80e7) )

	ROM_REGION( 0x1000000, "qs1000", 0 ) /* QDSP wavetable ROMs */
	ROM_LOAD( "rom.u18",  0x000000, 0x080000, CRC(50d76732) SHA1(6179c7365b62df620a10a1253d524807408821de) )
	ROM_LOAD( "rom.u17",  0x080000, 0x080000, CRC(20f2417c) SHA1(1bdc0b03215f5002eed4c25d670bbb5411189907) )
	ROM_LOAD( "qs1003.u4",0x200000, 0x200000, CRC(19e4b469) SHA1(9460e5b6a0fbf3fdd6a9fa0dcbf5062a2e07fe02) )
	// U19 empty
	// U20 empty
ROM_END

ROM_START( sb2003 ) /* No specific Country/Region */
	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD16_WORD_SWAP( "sb2003_05.u6", 0x00000000, 0x200000, CRC(8aec4554) SHA1(57a12b142eb7bf08dd1e78d3c79222001bbaa636) )

	ROM_REGION32_BE( 0x400000, "user2", ROMREGION_ERASEFF )
	// u5 empty

	ROM_REGION( 0x80000, "qs1000:cpu", 0 ) /* QS1000 CPU */
	ROM_LOAD( "07.u16", 0x00000, 0x20000, CRC(78acc607) SHA1(30a1aed40d45233dce88c6114989c71aa0f99ff7) )
	ROM_FILL(           0x20000, 0x60000, 0xff)

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "01.u1", 0x000000, 0x200000, CRC(d2c7091a) SHA1(deff050eb0aee89f60d5ad13053e4f1bd4d35961) )
	ROM_LOAD32_BYTE( "02.u2", 0x000001, 0x200000, CRC(a0734195) SHA1(8947f351434e2f750c4bdf936238815baaeb8402) )
	ROM_LOAD32_BYTE( "03.u3", 0x000002, 0x200000, CRC(0f020280) SHA1(2c10baec8dbb201ee5e1c4c9d6b962e2ed02df7d) )
	ROM_LOAD32_BYTE( "04.u4", 0x000003, 0x200000, CRC(fc2222b9) SHA1(c7ee8cffbbee1673a9f107f3f163d029c3900230) )

	ROM_REGION( 0x1000000, "qs1000", 0 ) /* QDSP wavetable ROMs */
	ROM_LOAD( "06.u18",    0x000000, 0x200000, CRC(b6ad0d32) SHA1(33e73963ea25e131801dc11f25be6ab18bef03ed) )
	ROM_LOAD( "qs1003.u4", 0x200000, 0x200000, CRC(19e4b469) SHA1(9460e5b6a0fbf3fdd6a9fa0dcbf5062a2e07fe02) )
	// U17 empty
	// U19 empty
	// U20 (S-ROM) empty
ROM_END

ROM_START( sb2003a ) /* Asia Region */
	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD16_WORD_SWAP( "sb2003a_05.u6", 0x000000, 0x200000, CRC(265e45a7) SHA1(b9c8b63aa89c08f3d9d404621e301b122f85389a) )

	ROM_REGION32_BE( 0x400000, "user2", ROMREGION_ERASEFF )
	// u5 empty

	ROM_REGION( 0x80000, "qs1000:cpu", 0 ) /* QS1000 CPU */
	ROM_LOAD( "07.u16", 0x00000, 0x20000, CRC(78acc607) SHA1(30a1aed40d45233dce88c6114989c71aa0f99ff7) )
	ROM_FILL(           0x20000, 0x60000, 0xff)

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "01.u1", 0x000000, 0x200000, CRC(d2c7091a) SHA1(deff050eb0aee89f60d5ad13053e4f1bd4d35961) )
	ROM_LOAD32_BYTE( "02.u2", 0x000001, 0x200000, CRC(a0734195) SHA1(8947f351434e2f750c4bdf936238815baaeb8402) )
	ROM_LOAD32_BYTE( "03.u3", 0x000002, 0x200000, CRC(0f020280) SHA1(2c10baec8dbb201ee5e1c4c9d6b962e2ed02df7d) )
	ROM_LOAD32_BYTE( "04.u4", 0x000003, 0x200000, CRC(fc2222b9) SHA1(c7ee8cffbbee1673a9f107f3f163d029c3900230) )

	ROM_REGION( 0x1000000, "qs1000", 0 ) /* QDSP wavetable ROM */
	ROM_LOAD( "06.u18",   0x000000, 0x200000, CRC(b6ad0d32) SHA1(33e73963ea25e131801dc11f25be6ab18bef03ed) )
	ROM_LOAD( "qs1003.u4",0x200000, 0x200000, CRC(19e4b469) SHA1(9460e5b6a0fbf3fdd6a9fa0dcbf5062a2e07fe02) )
	// U17 empty
	// U19 empty
	// U20 (S-ROM) empty
ROM_END

/*

Legend Of Heroes
Limenko, 2000

This game runs on a cartridge-based system with Hyperstone E1-32XN CPU and
QDSP QS1000 sound hardware.

PCB Layout
----------

LIMENKO MAIN BOARD SYSTEM
MODEL : LMSYS
REV : LM-003B
SEL : B3-06-00
|-----------------------------------------------------------|
|                                                           |
||-|                 IS61C256    |--------| IS41C16256   |-||
|| |                             |SYS     |              | ||
|| |           |------|          |L2D_HYP |              | ||
|| | QS1003    |QS1000|          |VER1.0  |              | ||
|| |           |      |24MHz     |--------| IC41C16256   | ||
|| |           |------|                                  | ||
|| |                             32MHz     20MHz         | ||
|| |                                                     | ||
|| |              PAL                                    | ||
|| |DA1311                       |--------| IS41C16105   | ||
|| |               IS61C6416     |E1-32XN |              | ||
||-|                             |        |              |-||
|  TL084                         |        |                 |
|                                |--------| IC41C16105      |
|                                                           |
|  TL082         93C46                               PWR_LED|
|                                                    RUN_LED|
|VOL                                                        |
| KIA6280                                           RESET_SW|
|                                                    TEST_SW|
|                                                           |
|---|          JAMMA            |------|    22-WAY      |---|
    |---------------------------|      |----------------|


ROM Board
---------

REV : LMSYS_D
SEL : D2-09-00
|-----------------------------------------------------------|
|   +&*SYS_ROM7              SOU_ROM2      SOU_PRG          |
||-|+&*SYS_ROM8                                          |-||
|| |  +SYS_ROM6              SOU_ROM1                    | ||
|| |  +SYS_ROM5                           +CG_ROM10      | ||
|| |  &SYS_ROM1              CG_ROM12    +*CG_ROM11      | ||
|| |                                                     | ||
|| |                                      +CG_ROM20      | ||
|| |  &SYS_ROM2              CG_ROM22    +*CG_ROM21      | ||
|| |                                                     | ||
|| |                                      +CG_ROM30      | ||
|| |  &SYS_ROM3              CG_ROM32    +*CG_ROM31      | ||
|| |                                                     | ||
||-|                                      +CG_ROM40      |-||
|      SYS_ROM4              CG_ROM42    +*CG_ROM41         |
|-----------------------------------------------------------|
Notes:
      * - These ROMs located on the other side of the PCB
      + - These ROMs surface mounted, type MX29F1610 16MBit SOP44
      & - These locations not populated

Link up 2 cabinets, up to 4 players can play at a time as a team

*/

ROM_START( legendoh )
	ROM_REGION32_BE( 0x200000, "user1", ROMREGION_ERASEFF ) /* Hyperstone CPU Code */
	/* sys_rom1 empty */
	/* sys_rom2 empty */
	/* sys_rom3 empty */
	ROM_LOAD16_WORD_SWAP( "01.sys_rom4", 0x180000, 0x80000, CRC(49b4a91f) SHA1(21619e8cd0b2fba8c2e08158497575a1760f52c5) )

	ROM_REGION32_BE( 0x400000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "sys_rom6", 0x000000, 0x200000, CRC(5c13d467) SHA1(ed07b7e1b22293e256787ab079d00c2fb070bf4f) )
	ROM_LOAD16_WORD_SWAP( "sys_rom5", 0x200000, 0x200000, CRC(19dc8d23) SHA1(433687c6aa24b9456436eecb1dcb57814af3009d) )
	/* sys_rom8 empty */
	/* sys_rom7 empty */

	ROM_REGION( 0x1200000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "cg_rom10",     0x0000000, 0x200000, CRC(93a48489) SHA1(a14157d31b4e9c8eb7ebe1b2f1b707ec8c8561a0) )
	ROM_LOAD32_BYTE( "cg_rom20",     0x0000001, 0x200000, CRC(1a6c0258) SHA1(ac7c3b8c2fdfb542103032144a30293d44759fd1) )
	ROM_LOAD32_BYTE( "cg_rom30",     0x0000002, 0x200000, CRC(a0559ef4) SHA1(6622f7107b374c9da816b9814fe93347e7422190) )
	ROM_LOAD32_BYTE( "cg_rom40",     0x0000003, 0x200000, CRC(a607b2b5) SHA1(9a6b867d6a777cbc910b98d505367819e0c20077) )
	ROM_LOAD32_BYTE( "cg_rom11",     0x0800000, 0x200000, CRC(a9fd5a50) SHA1(d15fc4d1697c1505aa98979af09bcfbbc2521145) )
	ROM_LOAD32_BYTE( "cg_rom21",     0x0800001, 0x200000, CRC(b05cdeb2) SHA1(43115146496ee3a820278ffc0b5f0325d6af6335) )
	ROM_LOAD32_BYTE( "cg_rom31",     0x0800002, 0x200000, CRC(a9a0d386) SHA1(501af14ea1af70be4862172701af4850750d3f36) )
	ROM_LOAD32_BYTE( "cg_rom41",     0x0800003, 0x200000, CRC(1c014f45) SHA1(a76246e90b41cc892575f3a3dc26d8d674e3fc3a) )
	ROM_LOAD32_BYTE( "02.cg_rom12",  0x1000000, 0x080000, CRC(8b2e8cbc) SHA1(6ed6db843e27d715e473752dd3853a28bb81a368) )
	ROM_LOAD32_BYTE( "03.cg_rom22",  0x1000001, 0x080000, CRC(a35960c8) SHA1(86914701930512cae81d1ad892d482264f80f695) )
	ROM_LOAD32_BYTE( "04.cg_rom32",  0x1000002, 0x080000, CRC(3f486cab) SHA1(6507d4bb9b4aa7d43f1026e932c82629d4fa44dd) )
	ROM_LOAD32_BYTE( "05.cg_rom42",  0x1000003, 0x080000, CRC(5d807bec) SHA1(c72c77ed0478f705018519cf68a54d22524d05fd) )

	ROM_REGION( 0x80000, "qs1000:cpu", 0 ) /* QS1000 CPU */
	ROM_LOAD( "sou_prg.06", 0x000000, 0x80000, CRC(bfafe7aa) SHA1(3e65869fe0970bafb59a0225642834042fdedfa6) )

	ROM_REGION( 0x1000000, "qs1000", 0 ) /* QDSP wavetable ROMs */
	ROM_LOAD( "sou_rom.07", 0x000000, 0x080000, CRC(4c6eb6d2) SHA1(58bced7bd944e03b0e3dfe1107c01819a33b2b31) )
	ROM_LOAD( "sou_rom.08", 0x080000, 0x080000, CRC(42c32dd5) SHA1(4702771288ba40119de63feb67eed85667235d81) )
	ROM_LOAD( "qs1003.u4",  0x200000, 0x200000, CRC(19e4b469) SHA1(9460e5b6a0fbf3fdd6a9fa0dcbf5062a2e07fe02) )
ROM_END

/*

Spotty

+---------------------------------+
|               GMS30C2232  16256 |
|                           16256 |
|J        M6295 SOU_ROM1 20MHz    |
|A             AT89C4051          |
|M       GAL      4MHz  SYS_ROM1* |
|M 93C46                SYS_ROM2  |
|A        16256x3       CG_ROM1   |
|          L2DHYP       CG_ROM2*  |
| SW1 SW2         32MHz CG_ROM3   |
+---------------------------------+

Hyundia GMS30C2232 (Hyperstone core)
Atmel AT89C4051 (8051 MCU with internal code)
SYS L2D HYP Ver 1.0 ASIC Express
EEPROM 93C46
SW1 = Test
SW2 = Reset
* Unpopulated

*/

ROM_START( spotty )
	ROM_REGION32_BE( 0x100000, "user1", ROMREGION_ERASEFF ) /* Hyperstone CPU Code */
	/* sys_rom1 empty */
	ROM_LOAD16_WORD_SWAP( "sys_rom2",     0x080000, 0x80000, CRC(6ded8d9b) SHA1(547c532f4014d818c4412244b60dbc439496de20) )

	ROM_REGION( 0x01000, "audiocpu", 0 )
	ROM_LOAD( "at89c4051.mcu", 0x000000, 0x01000, CRC(82ceab26) SHA1(9bbc454bdcbc70dc01f10a13c9fc01c884918fe8) )

	/* Expand the gfx roms here */
	ROM_REGION( 0x200000, "gfx1", ROMREGION_ERASE00 )

	ROM_REGION( 0x200000, "user2", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "gc_rom1",      0x000000, 0x80000, CRC(ea03f9c5) SHA1(5038c03c519c774da253f9ae4fa205e7eeaa2780) )
	ROM_LOAD32_BYTE( "gc_rom3",      0x000001, 0x80000, CRC(0ddac0b9) SHA1(f4ac8e6dd7f1cbdeb97139008982e6c17a3d18b9) )
	/* gc_rom2 empty */

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sou_rom1",     0x000000, 0x40000, CRC(5791195b) SHA1(de0df8f89f395cbf3508b01aeea05675e110ad04) )
ROM_END



READ32_MEMBER(limenko_state::dynabomb_speedup_r)
{
	if(m_maincpu->pc() == 0xc25b8)
	{
		m_maincpu->eat_cycles(50);
	}

	return m_mainram[0xe2784/4];
}

READ32_MEMBER(limenko_state::legendoh_speedup_r)
{
	if(m_maincpu->pc() == 0x23e32)
	{
		m_maincpu->eat_cycles(50);
	}

	return m_mainram[0x32ab0/4];
}

READ32_MEMBER(limenko_state::sb2003_speedup_r)
{
	if(m_maincpu->pc() == 0x26da4)
	{
		m_maincpu->eat_cycles(50);
	}

	return m_mainram[0x135800/4];
}

READ32_MEMBER(limenko_state::spotty_speedup_r)
{
	if(m_maincpu->pc() == 0x8560)
	{
		m_maincpu->eat_cycles(50);
	}

	return m_mainram[0x6626c/4];
}

DRIVER_INIT_MEMBER(limenko_state,common)
{
	// Set up the QS1000 program ROM banking, taking care not to overlap the internal RAM
	machine().device("qs1000:cpu")->memory().space(AS_IO).install_read_bank(0x0100, 0xffff, "bank");
	membank("qs1000:bank")->configure_entries(0, 8, memregion("qs1000:cpu")->base()+0x100, 0x10000);

	m_spriteram_bit = 1;
}

DRIVER_INIT_MEMBER(limenko_state,dynabomb)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xe2784, 0xe2787, read32_delegate(FUNC(limenko_state::dynabomb_speedup_r), this));

	DRIVER_INIT_CALL(common);
}

DRIVER_INIT_MEMBER(limenko_state,legendoh)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x32ab0, 0x32ab3, read32_delegate(FUNC(limenko_state::legendoh_speedup_r), this));

	DRIVER_INIT_CALL(common);
}

DRIVER_INIT_MEMBER(limenko_state,sb2003)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x135800, 0x135803, read32_delegate(FUNC(limenko_state::sb2003_speedup_r), this));

	DRIVER_INIT_CALL(common);
}


DRIVER_INIT_MEMBER(limenko_state,spotty)
{
	UINT8 *dst    = memregion("gfx1")->base();
	UINT8 *src    = memregion("user2")->base();
	int x;

	/* expand 4bpp roms to 8bpp space */
	for (x=0; x<0x200000;x+=4)
	{
		dst[x+1] = (src[x+0]&0xf0) >> 4;
		dst[x+0] = (src[x+0]&0x0f) >> 0;
		dst[x+3] = (src[x+1]&0xf0) >> 4;
		dst[x+2] = (src[x+1]&0x0f) >> 0;
	}

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x6626c, 0x6626f, read32_delegate(FUNC(limenko_state::spotty_speedup_r), this));

	m_spriteram_bit = 1;

	save_item(NAME(m_spotty_sound_cmd));
}

GAME( 2000, dynabomb, 0,      limenko, sb2003, limenko_state,   dynabomb, ROT0, "Limenko", "Dynamite Bomber (Korea, Rev 1.5)",   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2000, legendoh, 0,      limenko, legendoh, limenko_state, legendoh, ROT0, "Limenko", "Legend of Heroes",                   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2003, sb2003,   0,      limenko, sb2003, limenko_state,   sb2003,   ROT0, "Limenko", "Super Bubble 2003 (World, Ver 1.0)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2003, sb2003a,  sb2003, limenko, sb2003, limenko_state,   sb2003,   ROT0, "Limenko", "Super Bubble 2003 (Asia, Ver 1.0)",  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

// this game only uses the same graphics chip used in Limenko's system
GAME( 2001, spotty,   0,      spotty,  spotty, limenko_state,   spotty,   ROT0, "Prince Co.", "Spotty (Ver. 2.0.2)",             MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
