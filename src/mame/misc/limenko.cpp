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
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/qs1000.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class limenko_state : public driver_device
{
public:
	limenko_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_qs1000(*this, "qs1000")
		, m_oki(*this, "oki")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_mainram(*this, "mainram")
		, m_fg_videoram(*this, "fg_videoram")
		, m_md_videoram(*this, "md_videoram")
		, m_bg_videoram(*this, "bg_videoram")
		, m_spriteram(*this, "spriteram")
		, m_videoreg(*this, "videoreg")
		, m_gfx_region(*this, "gfx")
		, m_qs1000_bank(*this, "qs1000_bank")
	{
	}

	void limenko(machine_config &config);
	void spotty(machine_config &config);

	void init_common();
	void init_sb2003();
	void init_dynabomb();
	void init_legendoh();
	void init_spotty();

	int spriteram_bit_r();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	optional_device<qs1000_device> m_qs1000;
	optional_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<u32> m_mainram;
	required_shared_ptr<u32> m_fg_videoram;
	required_shared_ptr<u32> m_md_videoram;
	required_shared_ptr<u32> m_bg_videoram;
	required_shared_ptr<u32> m_spriteram;
	required_shared_ptr<u32> m_videoreg;
	required_region_ptr<u8> m_gfx_region;

	memory_bank_creator m_qs1000_bank;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_md_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	int m_spriteram_bit = 0;
	bitmap_ind16 m_sprites_bitmap;
	bitmap_ind8 m_sprites_bitmap_pri;
	int m_prev_sprites_count = 0;

	void coincounter_w(u32 data);
	void bg_videoram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void md_videoram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void fg_videoram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void spriteram_buffer_w(u32 data);

	u32 dynabomb_speedup_r();
	u32 legendoh_speedup_r();
	u32 sb2003_speedup_r();
	u32 spotty_speedup_r();

	void qs1000_p1_w(u8 data);
	void qs1000_p2_w(u8 data);
	void qs1000_p3_w(u8 data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_md_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_single_sprite(bitmap_ind16 &dest_bmp,const rectangle &clip,u8 width,u8 height,u32 code,u32 color,bool flipx,bool flipy,int offsx,int offsy,
							u8 transparent_color, u8 priority);
	void draw_sprites();
	void copy_sprites(bitmap_ind16 &bitmap, bitmap_ind16 &sprites_bitmap, bitmap_ind8 &priority_bitmap, const rectangle &cliprect);

	void limenko_io_map(address_map &map) ATTR_COLD;
	void limenko_map(address_map &map) ATTR_COLD;
	void spotty_io_map(address_map &map) ATTR_COLD;
	void spotty_map(address_map &map) ATTR_COLD;

	// spotty audiocpu
	uint8_t audiocpu_p1_r();
	void audiocpu_p1_w(uint8_t data);
	uint8_t audiocpu_p3_r();
	void audiocpu_p3_w(uint8_t data);

	uint8_t m_audiocpu_p1 = 0;
	uint8_t m_audiocpu_p3 = 0;
};

/*****************************************************************************************************
  MISC FUNCTIONS
*****************************************************************************************************/

void limenko_state::coincounter_w(u32 data)
{
	machine().bookkeeping().coin_counter_w(0,data & 0x10000);
}


void limenko_state::bg_videoram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

void limenko_state::md_videoram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_md_videoram[offset]);
	m_md_tilemap->mark_tile_dirty(offset);
}

void limenko_state::fg_videoram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

int limenko_state::spriteram_bit_r()
{
	return m_spriteram_bit;
}

void limenko_state::spriteram_buffer_w(u32 data)
{
	// toggle spriterams location in the memory map
	m_spriteram_bit ^= 1;

	// draw the sprites to the frame buffer
	draw_sprites();

	// buffer the next number of sprites to draw
	m_prev_sprites_count = (m_videoreg[0] & 0x1ff0000) >> 16;
}

/*****************************************************************************************************
 SOUND FUNCTIONS
 *****************************************************************************************************/

void limenko_state::qs1000_p1_w(u8 data)
{
}

void limenko_state::qs1000_p2_w(u8 data)
{
	// Unknown. Often written with 0
}

void limenko_state::qs1000_p3_w(u8 data)
{
	// .... .xxx - Data ROM bank (64kB)
	// ...x .... - ?
	// ..x. .... - /IRQ clear

	m_qs1000_bank->set_entry(data & 0x07);

	if (!BIT(data, 5))
		m_soundlatch->acknowledge_w();
}

// spotty audio

uint8_t limenko_state::audiocpu_p1_r()
{
	return m_audiocpu_p1;
}

void limenko_state::audiocpu_p1_w(uint8_t data)
{
	m_audiocpu_p1 = data;
}

// 7-------  msm select
// -654----  unused
// ----3---  read latch maincpu
// -----2--  latch maincpu new data available
// ------1-  write msm
// -------0  read msm

uint8_t limenko_state::audiocpu_p3_r()
{
	return (m_soundlatch->pending_r() ? 0 : 1) << 2;
}

void limenko_state::audiocpu_p3_w(uint8_t data)
{
	if (BIT(m_audiocpu_p3, 0) == 1 && BIT(data, 0) == 0 && BIT(data, 7) == 0)
		m_audiocpu_p1 = m_oki->read();

	if (BIT(m_audiocpu_p3, 1) == 1 && BIT(data, 1) == 0 && BIT(data, 7) == 0)
		m_oki->write(m_audiocpu_p1);

	if (BIT(m_audiocpu_p3, 3) == 1 && BIT(data, 3) == 0)
		m_audiocpu_p1 = m_soundlatch->read();

	m_audiocpu_p3 = data;
}

/*****************************************************************************************************
  MEMORY MAPS
*****************************************************************************************************/

void limenko_state::limenko_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram().share("mainram");
	map(0x40000000, 0x403fffff).rom().region("maindata", 0);
	map(0x80000000, 0x80007fff).ram().w(FUNC(limenko_state::fg_videoram_w)).share("fg_videoram");
	map(0x80008000, 0x8000ffff).ram().w(FUNC(limenko_state::md_videoram_w)).share("md_videoram");
	map(0x80010000, 0x80017fff).ram().w(FUNC(limenko_state::bg_videoram_w)).share("bg_videoram");
	map(0x80018000, 0x80019fff).ram().share("spriteram");
	map(0x8001c000, 0x8001dfff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x8001e000, 0x8001ebff).ram(); // ? not used
	map(0x8001ffec, 0x8001ffff).ram().share("videoreg");
	map(0x8003e000, 0x8003e003).w(FUNC(limenko_state::spriteram_buffer_w));
	map(0xffe00000, 0xffffffff).rom().region("maincpu", 0);
}

void limenko_state::limenko_io_map(address_map &map)
{
	map(0x0000, 0x0003).portr("IN0");
	map(0x0800, 0x0803).portr("IN1");
	map(0x1000, 0x1003).portr("IN2");
	map(0x4000, 0x4003).w(FUNC(limenko_state::coincounter_w));
	map(0x4800, 0x4803).portw("EEPROMOUT");
	map(0x5000, 0x5003).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask32(0x00ff0000).cswidth(32);
}


/* Spotty memory map */

void limenko_state::spotty_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram().share("mainram");
	map(0x40002000, 0x400024d3).ram(); //?
	map(0x80000000, 0x80007fff).ram().w(FUNC(limenko_state::fg_videoram_w)).share("fg_videoram");
	map(0x80008000, 0x8000ffff).ram().w(FUNC(limenko_state::md_videoram_w)).share("md_videoram");
	map(0x80010000, 0x80017fff).ram().w(FUNC(limenko_state::bg_videoram_w)).share("bg_videoram");
	map(0x80018000, 0x80019fff).ram().share("spriteram");
	map(0x8001c000, 0x8001dfff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x8001e000, 0x8001ebff).ram(); // ? not used
	map(0x8001ffec, 0x8001ffff).ram().share("videoreg");
	map(0x8003e000, 0x8003e003).w(FUNC(limenko_state::spriteram_buffer_w));
	map(0xfff00000, 0xffffffff).rom().region("maincpu", 0);
}

void limenko_state::spotty_io_map(address_map &map)
{
	map(0x0000, 0x0003).portr("IN0");
	map(0x0800, 0x0803).portr("IN1");
	map(0x0800, 0x0803).nopw(); // hopper related
	map(0x1000, 0x1003).portr("IN2");
	map(0x4800, 0x4803).portw("EEPROMOUT");
	map(0x5000, 0x5003).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask32(0x00ff0000).cswidth(32);
}


/*****************************************************************************************************
  VIDEO HARDWARE EMULATION
*****************************************************************************************************/

TILE_GET_INFO_MEMBER(limenko_state::get_bg_tile_info)
{
	const u32 tile  = m_bg_videoram[tile_index] & 0x7ffff;
	const u32 color = (m_bg_videoram[tile_index]>>28) & 0xf;
	tileinfo.set(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(limenko_state::get_md_tile_info)
{
	const u32 tile  = m_md_videoram[tile_index] & 0x7ffff;
	const u32 color = (m_md_videoram[tile_index]>>28) & 0xf;
	tileinfo.set(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(limenko_state::get_fg_tile_info)
{
	const u32 tile  = m_fg_videoram[tile_index] & 0x7ffff;
	const u32 color = (m_fg_videoram[tile_index]>>28) & 0xf;
	tileinfo.set(0, tile, color, 0);
}

void limenko_state::draw_single_sprite(bitmap_ind16 &dest_bmp,const rectangle &clip,u8 width, u8 height,
							u32 code,u32 color,bool flipx,bool flipy,int offsx,int offsy,
							u8 transparent_color, u8 priority)
{
	/* Start drawing */
	const u16 pal = color << 8;
	const u8 *source_base = &m_gfx_region[code];

	int xinc = flipx ? -1 : 1;
	int yinc = flipy ? -1 : 1;

	int x_index_base = flipx ? width - 1 : 0;
	int y_index = flipy ? height - 1 : 0;

	// start coordinates
	int sx = offsx;
	int sy = offsy;

	// end coordinates
	int ex = sx + width;
	int ey = sy + height;

	if (sx < clip.min_x)
	{ // clip left
		int pixels = clip.min_x - sx;
		sx += pixels;
		x_index_base += xinc * pixels;
	}
	if (sy < clip.min_y)
	{ // clip top
		int pixels = clip.min_y - sy;
		sy += pixels;
		y_index += yinc * pixels;
	}
	// NS 980211 - fixed incorrect clipping
	if (ex > clip.max_x + 1)
	{ // clip right
		ex = clip.max_x + 1;
	}
	if (ey > clip.max_y + 1)
	{ // clip bottom
		ey = clip.max_y + 1;
	}

	if (ex > sx)
	{ // skip if inner loop doesn't draw anything
		for (int y = sy; y < ey; y++)
		{
			u8 const *const source = source_base + y_index * width;
			u16 *const dest = &dest_bmp.pix(y);
			u8 *const pri = &m_sprites_bitmap_pri.pix(y);
			int x_index = x_index_base;
			for (int x = sx; x < ex; x++)
			{
				const u8 c = source[x_index];
				if (c != transparent_color)
				{
						if (pri[x] < priority)
						{
							dest[x] = pal + c;
							pri[x] = priority;
						}
				}
				x_index += xinc;
			}
			y_index += yinc;
		}
	}
}

// sprites aren't tile based (except for 8x8 ones)
void limenko_state::draw_sprites()
{
	const rectangle cliprect(0,511,0,511);
	m_sprites_bitmap_pri.fill(0, cliprect);
	m_sprites_bitmap.fill(0, cliprect);

	u32 *sprites = m_spriteram + (0x200 * 2 * m_spriteram_bit);

	for (int i = 0; i <= m_prev_sprites_count * 2; i += 2)
	{
		if (~sprites[i] & 0x80000000) continue;

		const int x      =  ((sprites[i + 0] & 0x01ff0000) >> 16);
		const int width  = (((sprites[i + 0] & 0x0e000000) >> 25) + 1) * 8;
		const bool flipx =    sprites[i + 0] & 0x10000000;
		const int y      =    sprites[i + 0] & 0x000001ff;
		const int height = (((sprites[i + 0] & 0x00000e00) >> 9) + 1) * 8;
		const bool flipy =    sprites[i + 0] & 0x00001000;
		const u32 code   =   (sprites[i + 1] & 0x0007ffff) << 6;
		const u32 color  =   (sprites[i + 1] & 0xf0000000) >> 28;

		int pri = 0;
		if (sprites[i + 1] & 0x04000000)
		{
			// below fg
			pri = 1;
		}
		else
		{
			// above everything
			pri = 2;
		}

		/* Bounds checking */
		if ((code + (width * height)) > m_gfx_region.length())
			continue;

		draw_single_sprite(m_sprites_bitmap,cliprect,width,height,code,color,flipx,flipy,x,y,0,pri);

		// wrap around x
		draw_single_sprite(m_sprites_bitmap,cliprect,width,height,code,color,flipx,flipy,x-512,y,0,pri);

		// wrap around y
		draw_single_sprite(m_sprites_bitmap,cliprect,width,height,code,color,flipx,flipy,x,y-512,0,pri);

		// wrap around x and y
		draw_single_sprite(m_sprites_bitmap,cliprect,width,height,code,color,flipx,flipy,x-512,y-512,0,pri);
	}
}

void limenko_state::copy_sprites(bitmap_ind16 &bitmap, bitmap_ind16 &sprites_bitmap, bitmap_ind8 &priority_bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u16 const *const source = &sprites_bitmap.pix(y);
		u16 *const dest = &bitmap.pix(y);
		u8 const *const dest_pri = &priority_bitmap.pix(y);
		u8 const *const source_pri = &m_sprites_bitmap_pri.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			if (source[x] != 0)
			{
				if (dest_pri[x] < source_pri[x])
					dest[x] = source[x];
			}
		}
	}
}

void limenko_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(limenko_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8,8,128,64);
	m_md_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(limenko_state::get_md_tile_info)), TILEMAP_SCAN_ROWS, 8,8,128,64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(limenko_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8,8,128,64);

	m_md_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

	m_sprites_bitmap.allocate(512,512);
	m_sprites_bitmap_pri.allocate(512,512);

	save_item(NAME(m_spriteram_bit));
	save_item(NAME(m_prev_sprites_count));
}

u32 limenko_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

	if (m_videoreg[0] & 8)
		copy_sprites(bitmap, m_sprites_bitmap, screen.priority(), cliprect);

	return 0;
}

/*****************************************************************************************************
  INPUT PORTS
*****************************************************************************************************/

static INPUT_PORTS_START(legendoh)
	PORT_START("IN0")
	PORT_BIT(0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(1)
	PORT_BIT(0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(1)
	PORT_BIT(0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(1)
	PORT_BIT(0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(1)
	PORT_BIT(0x00800000, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(1)
	PORT_BIT(0x01000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(3)
	PORT_BIT(0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(3)
	PORT_BIT(0x04000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(3)
	PORT_BIT(0x08000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(3)
	PORT_BIT(0x10000000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(3)
	PORT_BIT(0x20000000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(3)
	PORT_BIT(0x40000000, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(3)
	PORT_BIT(0x80000000, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(3)
	PORT_BIT(0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(2)
	PORT_BIT(0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(2)
	PORT_BIT(0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2)
	PORT_BIT(0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(2)
	PORT_BIT(0x00800000, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(2)
	PORT_BIT(0x01000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(4)
	PORT_BIT(0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(4)
	PORT_BIT(0x04000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(4)
	PORT_BIT(0x08000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(4)
	PORT_BIT(0x10000000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(4)
	PORT_BIT(0x20000000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(4)
	PORT_BIT(0x40000000, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(4)
	PORT_BIT(0x80000000, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(4)
	PORT_BIT(0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN2")
	PORT_BIT(0x00010000, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x00020000, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT(0x00040000, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x00080000, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x00100000, IP_ACTIVE_LOW, IPT_SERVICE1)
	PORT_SERVICE_NO_TOGGLE(0x00200000, IP_ACTIVE_LOW)
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_CUSTOM) //security bit
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT(0x01000000, IP_ACTIVE_LOW, IPT_START3)
	PORT_BIT(0x02000000, IP_ACTIVE_LOW, IPT_START4)
	PORT_BIT(0x04000000, IP_ACTIVE_LOW, IPT_COIN3)
	PORT_BIT(0x08000000, IP_ACTIVE_LOW, IPT_COIN4)
	PORT_BIT(0x10000000, IP_ACTIVE_LOW, IPT_SERVICE2)
	PORT_DIPNAME(0x20000000, 0x00000000, "Sound Enable")
	PORT_DIPSETTING(         0x20000000, DEF_STR(Off))
	PORT_DIPSETTING(         0x00000000, DEF_STR(On))
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(limenko_state, spriteram_bit_r) //changes spriteram location
	PORT_BIT(0x4000ffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("EEPROMOUT")
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
//  PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x80000 -> video disabled?
INPUT_PORTS_END

static INPUT_PORTS_START(sb2003)
	PORT_START("IN0")
	PORT_BIT(0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(1)
	PORT_BIT(0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(1)
	PORT_BIT(0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(1)
	PORT_BIT(0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(0x00100000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x00200000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(1)
	PORT_BIT(0x00800000, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(1)
	PORT_BIT(0xff00ffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(2)
	PORT_BIT(0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(2)
	PORT_BIT(0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2)
	PORT_BIT(0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x00100000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x00200000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(2)
	PORT_BIT(0x00800000, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(2)
	PORT_BIT(0xff00ffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN2")
	PORT_BIT(0x00010000, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x00020000, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT(0x00040000, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x00080000, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_SERVICE_NO_TOGGLE(0x00200000, IP_ACTIVE_LOW)
	PORT_BIT(0x00400000, IP_ACTIVE_LOW, IPT_CUSTOM) //security bit
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_DIPNAME(0x20000000, 0x00000000, "Sound Enable")
	PORT_DIPSETTING(         0x20000000, DEF_STR(Off))
	PORT_DIPSETTING(         0x00000000, DEF_STR(On))
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(limenko_state, spriteram_bit_r) //changes spriteram location
	PORT_BIT(0x00100000, IP_ACTIVE_LOW, IPT_SERVICE1) // checked in dynabomb I/O test, but doesn't work in game
	PORT_BIT(0x5f00ffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("EEPROMOUT")
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
//  PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x80000 -> video disabled?
INPUT_PORTS_END

static INPUT_PORTS_START(spotty)
	PORT_START("IN0")
	PORT_BIT(0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_NAME("Hold 1")
	PORT_BIT(0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_NAME("Hold 2")
	PORT_BIT(0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_NAME("Hold 3")
	PORT_BIT(0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_NAME("Hold 4")
	PORT_BIT(0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Bet")
	PORT_BIT(0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Stop")
	PORT_BIT(0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Change")
	PORT_BIT(0x00800000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0xff00ffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x00010000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00020000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00040000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00080000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00100000, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("Prize Hopper 1")
	PORT_BIT(0x00200000, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("Prize Hopper 2")
	PORT_BIT(0x00400000, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("Prize Hopper 3")
	PORT_BIT(0x00800000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0xff00ffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN2")
	PORT_BIT(0x00010000, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x00020000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00040000, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(limenko_state, spriteram_bit_r) //changes spriteram location
	PORT_SERVICE_NO_TOGGLE(0x00200000, IP_ACTIVE_LOW)
	PORT_BIT(0x00400000, IP_ACTIVE_LOW, IPT_CUSTOM) //security bit
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_DIPNAME(0x20000000, 0x20000000, DEF_STR(Demo_Sounds))
	PORT_DIPSETTING(         0x00000000, DEF_STR(Off))
	PORT_DIPSETTING(         0x20000000, DEF_STR(On))
	PORT_BIT(0x80000000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x5f10ffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("EEPROMOUT")
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
//  PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x80000 -> video disabled?
INPUT_PORTS_END

/*****************************************************************************************************
  GRAPHICS DECODES
*****************************************************************************************************/


static GFXDECODE_START(gfx_limenko)
	GFXDECODE_ENTRY("gfx", 0, gfx_8x8x8_raw, 0, 16) /* tiles */
GFXDECODE_END


/*****************************************************************************************************
  MACHINE DRIVERS
*****************************************************************************************************/

void limenko_state::limenko(machine_config &config)
{
	E132XN(config, m_maincpu, 20000000*4); /* 4x internal multiplier */
	m_maincpu->set_addrmap(AS_PROGRAM, &limenko_state::limenko_map);
	m_maincpu->set_addrmap(AS_IO, &limenko_state::limenko_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(limenko_state::irq0_line_hold));

	EEPROM_93C46_16BIT(config, "eeprom");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(384, 240);
	screen.set_visarea(0, 383, 0, 239);
	screen.set_screen_update(FUNC(limenko_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_limenko);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x1000);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set(m_qs1000, FUNC(qs1000_device::set_irq));
	m_soundlatch->set_separate_acknowledge(true);

	QS1000(config, m_qs1000, XTAL(24'000'000));
	m_qs1000->set_external_rom(true);
	m_qs1000->p1_in().set("soundlatch", FUNC(generic_latch_8_device::read));
	m_qs1000->p1_out().set(FUNC(limenko_state::qs1000_p1_w));
	m_qs1000->p2_out().set(FUNC(limenko_state::qs1000_p2_w));
	m_qs1000->p3_out().set(FUNC(limenko_state::qs1000_p3_w));
	m_qs1000->add_route(0, "lspeaker", 1.0);
	m_qs1000->add_route(1, "rspeaker", 1.0);
}

void limenko_state::spotty(machine_config &config)
{
	GMS30C2232(config, m_maincpu, 20000000);   /* 20 MHz, no internal multiplier */
	m_maincpu->set_addrmap(AS_PROGRAM, &limenko_state::spotty_map);
	m_maincpu->set_addrmap(AS_IO, &limenko_state::spotty_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(limenko_state::irq0_line_hold));

	at89c4051_device &audiocpu(AT89C4051(config, "audiocpu", 4000000));
	audiocpu.port_in_cb<1>().set(FUNC(limenko_state::audiocpu_p1_r));
	audiocpu.port_out_cb<1>().set(FUNC(limenko_state::audiocpu_p1_w));
	audiocpu.port_in_cb<3>().set(FUNC(limenko_state::audiocpu_p3_r));
	audiocpu.port_out_cb<3>().set(FUNC(limenko_state::audiocpu_p3_w));

	EEPROM_93C46_16BIT(config, "eeprom");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(384, 240);
	screen.set_visarea(0, 383, 0, 239);
	screen.set_screen_update(FUNC(limenko_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_limenko);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x1000);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	OKIM6295(config, m_oki, 4000000 / 4 , okim6295_device::PIN7_HIGH); //?
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}


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

ROM_START(dynabomb)
	ROM_REGION32_BE(0x200000, "maincpu", 0) /* Hyperstone CPU Code */
	ROM_LOAD16_WORD_SWAP("rom.u6", 0x000000, 0x200000, CRC(457e015d) SHA1(3afb56cdf903c9084c1f283dc50ec504ce3e199f))

	ROM_REGION32_BE(0x400000, "maindata", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("rom.u5", 0x000000, 0x200000, CRC(7e837adf) SHA1(8613fa187b8d4574b3935aa439aec2515033d64c))

	ROM_REGION(0x80000, "qs1000:cpu", 0) /* QS1000 CPU */
	ROM_LOAD("rom.u16", 0x00000, 0x20000, CRC(f66d7e4d) SHA1(44f1851405ba525f1ed53521f4de12545ea9c46a))
	ROM_FILL(           0x20000, 0x60000, 0xff)

	ROM_REGION(0x800000, "gfx", 0)
	ROM_LOAD32_BYTE("rom.u1", 0x000000, 0x200000, CRC(bf33eff6) SHA1(089b6d88d6d744bcfa036c6869f0444d6ceb26c9))
	ROM_LOAD32_BYTE("rom.u2", 0x000001, 0x200000, CRC(790bbcd5) SHA1(fc52c15fffc77dc3b3bc89a9606223c4fbaa578c))
	ROM_LOAD32_BYTE("rom.u3", 0x000002, 0x200000, CRC(ec094b12) SHA1(13c105df066ff308cc7e1842907644790946e5b5))
	ROM_LOAD32_BYTE("rom.u4", 0x000003, 0x200000, CRC(88b24e3c) SHA1(5f267f08144b413b55ef5e15c52e9cda096b80e7))

	ROM_REGION(0x1000000, "qs1000", 0) /* QDSP wavetable ROMs */
	ROM_LOAD("rom.u18",  0x000000, 0x080000, CRC(50d76732) SHA1(6179c7365b62df620a10a1253d524807408821de))
	ROM_LOAD("rom.u17",  0x080000, 0x080000, CRC(20f2417c) SHA1(1bdc0b03215f5002eed4c25d670bbb5411189907))
	ROM_LOAD("qs1003.u4",0x200000, 0x200000, CRC(19e4b469) SHA1(9460e5b6a0fbf3fdd6a9fa0dcbf5062a2e07fe02))
	// U19 empty
	// U20 empty
ROM_END

ROM_START(sb2003) /* No specific Country/Region */
	ROM_REGION32_BE(0x200000, "maincpu", 0) /* Hyperstone CPU Code */
	ROM_LOAD16_WORD_SWAP("sb2003_05.u6", 0x00000000, 0x200000, CRC(8aec4554) SHA1(57a12b142eb7bf08dd1e78d3c79222001bbaa636))

	ROM_REGION32_BE(0x400000, "maindata", ROMREGION_ERASEFF)
	// u5 empty

	ROM_REGION(0x80000, "qs1000:cpu", 0) /* QS1000 CPU */
	ROM_LOAD("07.u16", 0x00000, 0x20000, CRC(78acc607) SHA1(30a1aed40d45233dce88c6114989c71aa0f99ff7))
	ROM_FILL(          0x20000, 0x60000, 0xff)

	ROM_REGION(0x800000, "gfx", 0)
	ROM_LOAD32_BYTE("01.u1", 0x000000, 0x200000, CRC(d2c7091a) SHA1(deff050eb0aee89f60d5ad13053e4f1bd4d35961))
	ROM_LOAD32_BYTE("02.u2", 0x000001, 0x200000, CRC(a0734195) SHA1(8947f351434e2f750c4bdf936238815baaeb8402))
	ROM_LOAD32_BYTE("03.u3", 0x000002, 0x200000, CRC(0f020280) SHA1(2c10baec8dbb201ee5e1c4c9d6b962e2ed02df7d))
	ROM_LOAD32_BYTE("04.u4", 0x000003, 0x200000, CRC(fc2222b9) SHA1(c7ee8cffbbee1673a9f107f3f163d029c3900230))

	ROM_REGION(0x1000000, "qs1000", 0) /* QDSP wavetable ROMs */
	ROM_LOAD("06.u18",    0x000000, 0x200000, CRC(b6ad0d32) SHA1(33e73963ea25e131801dc11f25be6ab18bef03ed))
	ROM_LOAD("qs1003.u4", 0x200000, 0x200000, CRC(19e4b469) SHA1(9460e5b6a0fbf3fdd6a9fa0dcbf5062a2e07fe02))
	// U17 empty
	// U19 empty
	// U20 (S-ROM) empty
ROM_END

ROM_START(sb2003a) /* Asia Region */
	ROM_REGION32_BE(0x200000, "maincpu", 0) /* Hyperstone CPU Code */
	ROM_LOAD16_WORD_SWAP("sb2003a_05.u6", 0x000000, 0x200000, CRC(265e45a7) SHA1(b9c8b63aa89c08f3d9d404621e301b122f85389a))

	ROM_REGION32_BE(0x400000, "maindata", ROMREGION_ERASEFF)
	// u5 empty

	ROM_REGION(0x80000, "qs1000:cpu", 0) /* QS1000 CPU */
	ROM_LOAD("07.u16", 0x00000, 0x20000, CRC(78acc607) SHA1(30a1aed40d45233dce88c6114989c71aa0f99ff7))
	ROM_FILL(          0x20000, 0x60000, 0xff)

	ROM_REGION(0x800000, "gfx", 0)
	ROM_LOAD32_BYTE("01.u1", 0x000000, 0x200000, CRC(d2c7091a) SHA1(deff050eb0aee89f60d5ad13053e4f1bd4d35961))
	ROM_LOAD32_BYTE("02.u2", 0x000001, 0x200000, CRC(a0734195) SHA1(8947f351434e2f750c4bdf936238815baaeb8402))
	ROM_LOAD32_BYTE("03.u3", 0x000002, 0x200000, CRC(0f020280) SHA1(2c10baec8dbb201ee5e1c4c9d6b962e2ed02df7d))
	ROM_LOAD32_BYTE("04.u4", 0x000003, 0x200000, CRC(fc2222b9) SHA1(c7ee8cffbbee1673a9f107f3f163d029c3900230))

	ROM_REGION(0x1000000, "qs1000", 0) /* QDSP wavetable ROM */
	ROM_LOAD("06.u18",   0x000000, 0x200000, CRC(b6ad0d32) SHA1(33e73963ea25e131801dc11f25be6ab18bef03ed))
	ROM_LOAD("qs1003.u4",0x200000, 0x200000, CRC(19e4b469) SHA1(9460e5b6a0fbf3fdd6a9fa0dcbf5062a2e07fe02))
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

ROM_START(legendoh)
	ROM_REGION32_BE(0x200000, "maincpu", ROMREGION_ERASEFF) /* Hyperstone CPU Code */
	/* sys_rom1 empty */
	/* sys_rom2 empty */
	/* sys_rom3 empty */
	ROM_LOAD16_WORD_SWAP("01.sys_rom4", 0x180000, 0x80000, CRC(49b4a91f) SHA1(21619e8cd0b2fba8c2e08158497575a1760f52c5))

	ROM_REGION32_BE(0x400000, "maindata", 0)
	ROM_LOAD16_WORD_SWAP("sys_rom6", 0x000000, 0x200000, CRC(5c13d467) SHA1(ed07b7e1b22293e256787ab079d00c2fb070bf4f))
	ROM_LOAD16_WORD_SWAP("sys_rom5", 0x200000, 0x200000, CRC(19dc8d23) SHA1(433687c6aa24b9456436eecb1dcb57814af3009d))
	/* sys_rom8 empty */
	/* sys_rom7 empty */

	ROM_REGION(0x1200000, "gfx", 0)
	ROM_LOAD32_BYTE("cg_rom10",     0x0000000, 0x200000, CRC(93a48489) SHA1(a14157d31b4e9c8eb7ebe1b2f1b707ec8c8561a0))
	ROM_LOAD32_BYTE("cg_rom20",     0x0000001, 0x200000, CRC(1a6c0258) SHA1(ac7c3b8c2fdfb542103032144a30293d44759fd1))
	ROM_LOAD32_BYTE("cg_rom30",     0x0000002, 0x200000, CRC(a0559ef4) SHA1(6622f7107b374c9da816b9814fe93347e7422190))
	ROM_LOAD32_BYTE("cg_rom40",     0x0000003, 0x200000, CRC(a607b2b5) SHA1(9a6b867d6a777cbc910b98d505367819e0c20077))
	ROM_LOAD32_BYTE("cg_rom11",     0x0800000, 0x200000, CRC(a9fd5a50) SHA1(d15fc4d1697c1505aa98979af09bcfbbc2521145))
	ROM_LOAD32_BYTE("cg_rom21",     0x0800001, 0x200000, CRC(b05cdeb2) SHA1(43115146496ee3a820278ffc0b5f0325d6af6335))
	ROM_LOAD32_BYTE("cg_rom31",     0x0800002, 0x200000, CRC(a9a0d386) SHA1(501af14ea1af70be4862172701af4850750d3f36))
	ROM_LOAD32_BYTE("cg_rom41",     0x0800003, 0x200000, CRC(1c014f45) SHA1(a76246e90b41cc892575f3a3dc26d8d674e3fc3a))
	ROM_LOAD32_BYTE("02.cg_rom12",  0x1000000, 0x080000, CRC(8b2e8cbc) SHA1(6ed6db843e27d715e473752dd3853a28bb81a368))
	ROM_LOAD32_BYTE("03.cg_rom22",  0x1000001, 0x080000, CRC(a35960c8) SHA1(86914701930512cae81d1ad892d482264f80f695))
	ROM_LOAD32_BYTE("04.cg_rom32",  0x1000002, 0x080000, CRC(3f486cab) SHA1(6507d4bb9b4aa7d43f1026e932c82629d4fa44dd))
	ROM_LOAD32_BYTE("05.cg_rom42",  0x1000003, 0x080000, CRC(5d807bec) SHA1(c72c77ed0478f705018519cf68a54d22524d05fd))

	ROM_REGION(0x80000, "qs1000:cpu", 0) /* QS1000 CPU */
	ROM_LOAD("sou_prg.06", 0x000000, 0x80000, CRC(bfafe7aa) SHA1(3e65869fe0970bafb59a0225642834042fdedfa6))

	ROM_REGION(0x1000000, "qs1000", 0) /* QDSP wavetable ROMs */
	ROM_LOAD("sou_rom.07", 0x000000, 0x080000, CRC(4c6eb6d2) SHA1(58bced7bd944e03b0e3dfe1107c01819a33b2b31))
	ROM_LOAD("sou_rom.08", 0x080000, 0x080000, CRC(42c32dd5) SHA1(4702771288ba40119de63feb67eed85667235d81))
	ROM_LOAD("qs1003.u4",  0x200000, 0x200000, CRC(19e4b469) SHA1(9460e5b6a0fbf3fdd6a9fa0dcbf5062a2e07fe02))
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

ROM_START(spotty)
	ROM_REGION32_BE(0x100000, "maincpu", ROMREGION_ERASEFF) /* Hyperstone CPU Code */
	/* sys_rom1 empty */
	ROM_LOAD16_WORD_SWAP("sys_rom2",     0x080000, 0x80000, CRC(6ded8d9b) SHA1(547c532f4014d818c4412244b60dbc439496de20))

	ROM_REGION(0x01000, "audiocpu", 0)
	ROM_LOAD("at89c4051.mcu", 0x000000, 0x01000, CRC(82ceab26) SHA1(9bbc454bdcbc70dc01f10a13c9fc01c884918fe8))

	/* Expand the gfx roms here */
	ROM_REGION(0x200000, "gfx", ROMREGION_ERASE00)

	ROM_REGION(0x200000, "maindata", ROMREGION_ERASE00)
	ROM_LOAD32_BYTE("gc_rom1",      0x000000, 0x80000, CRC(ea03f9c5) SHA1(5038c03c519c774da253f9ae4fa205e7eeaa2780))
	ROM_LOAD32_BYTE("gc_rom3",      0x000001, 0x80000, CRC(0ddac0b9) SHA1(f4ac8e6dd7f1cbdeb97139008982e6c17a3d18b9))
	/* gc_rom2 empty */

	ROM_REGION(0x40000, "oki", 0)
	ROM_LOAD("sou_rom1",     0x000000, 0x40000, CRC(5791195b) SHA1(de0df8f89f395cbf3508b01aeea05675e110ad04))
ROM_END



u32 limenko_state::dynabomb_speedup_r()
{
	if (m_maincpu->pc() == 0xc25b8)
	{
		m_maincpu->eat_cycles(50);
	}

	return m_mainram[0xe2784/4];
}

u32 limenko_state::legendoh_speedup_r()
{
	if (m_maincpu->pc() == 0x23e32)
	{
		m_maincpu->eat_cycles(50);
	}

	return m_mainram[0x32ab0/4];
}

u32 limenko_state::sb2003_speedup_r()
{
	if (m_maincpu->pc() == 0x26da4)
	{
		m_maincpu->eat_cycles(50);
	}

	return m_mainram[0x135800/4];
}

u32 limenko_state::spotty_speedup_r()
{
	if (m_maincpu->pc() == 0x8560)
	{
		m_maincpu->eat_cycles(50);
	}

	return m_mainram[0x6626c/4];
}

void limenko_state::init_common()
{
	// Set up the QS1000 program ROM banking, taking care not to overlap the internal RAM
	m_qs1000->cpu().space(AS_IO).install_read_bank(0x0100, 0xffff, m_qs1000_bank);
	m_qs1000_bank->configure_entries(0, 8, memregion("qs1000:cpu")->base()+0x100, 0x10000);

	m_spriteram_bit = 1;
}

void limenko_state::init_dynabomb()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xe2784, 0xe2787, read32smo_delegate(*this, FUNC(limenko_state::dynabomb_speedup_r)));

	init_common();
}

void limenko_state::init_legendoh()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x32ab0, 0x32ab3, read32smo_delegate(*this, FUNC(limenko_state::legendoh_speedup_r)));

	init_common();
}

void limenko_state::init_sb2003()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x135800, 0x135803, read32smo_delegate(*this, FUNC(limenko_state::sb2003_speedup_r)));

	init_common();
}


void limenko_state::init_spotty()
{
	u8 *dst    = memregion("gfx")->base();
	u8 *src    = memregion("maindata")->base();

	/* expand 4bpp roms to 8bpp space */
	for (int x = 0; x < 0x200000; x += 4)
	{
		dst[x+1] = (src[x+0]&0xf0) >> 4;
		dst[x+0] = (src[x+0]&0x0f) >> 0;
		dst[x+3] = (src[x+1]&0xf0) >> 4;
		dst[x+2] = (src[x+1]&0x0f) >> 0;
	}

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x6626c, 0x6626f, read32smo_delegate(*this, FUNC(limenko_state::spotty_speedup_r)));

	m_spriteram_bit = 1;

	save_item(NAME(m_audiocpu_p1));
	save_item(NAME(m_audiocpu_p3));
}

} // anonymous namespace


GAME(2000, dynabomb, 0,      limenko, sb2003,   limenko_state, init_dynabomb, ROT0, "Limenko",    "Dynamite Bomber (Korea, Rev 1.5)",   MACHINE_SUPPORTS_SAVE)
GAME(2000, legendoh, 0,      limenko, legendoh, limenko_state, init_legendoh, ROT0, "Limenko",    "Legend of Heroes",                   MACHINE_SUPPORTS_SAVE)
GAME(2003, sb2003,   0,      limenko, sb2003,   limenko_state, init_sb2003,   ROT0, "Limenko",    "Super Bubble 2003 (World, Ver 1.0)", MACHINE_SUPPORTS_SAVE)
GAME(2003, sb2003a,  sb2003, limenko, sb2003,   limenko_state, init_sb2003,   ROT0, "Limenko",    "Super Bubble 2003 (Asia, Ver 1.0)",  MACHINE_SUPPORTS_SAVE)

// this game only uses the same graphics chip used in Limenko's system
GAME(2001, spotty,   0,      spotty,  spotty,   limenko_state, init_spotty,   ROT0, "Prince Co.", "Spotty (Ver. 2.0.2)",                MACHINE_SUPPORTS_SAVE)
