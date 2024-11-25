// license:BSD-3-Clause
// copyright-holders:David Haywood

/* Based on the "POEMS ES1 Flash ROM Writer Version 0.24 (C)2003-2004 HUDSON SOFT" string
   found in ROM this is assumed to be 'POEMS' hardware.

   https://game.watch.impress.co.jp/docs/20041209/toy166.htm
   https://forum.beyond3d.com/threads/hudson-softs-32-bit-cpu-poems-for-new-system.14358/
   https://web.archive.org/web/20021207035427/http://www.tensilica.com/html/pr_2002_10_15.html

   The above links mention Konami using this hardware for a PLAY-POEMS plug and play sports
   devices, and indicate it is based around the Xtensa instruction set, which has been confirmed
   for the single dumped device.

   https://0x04.net/~mwk/doc/xtensa.pdf

   Known PLAY-POEMS devices (all from Konami)

   2004/11/11   熱血パワプロチャンプ                                  (Baseball game)
   2004/11/11   爽快ゴルフチャンプ                                       (Golf game)
   2004/12/09   絶体絶命でんぢゃらすじーさん ミニゲームで対決じゃっ!     (Mini-Game Collection)
   2005/09/15   マリンバ天国                                          (Marimba Tengoku)
   2005/11/17   絶体絶命でんぢゃらすじーさん パーティーじゃっ!全員集合!!  (Mini-Game Collection)
   2005/11/24   ぐ〜チョコランタン スプーだいすき!プレイマット                (Kid's Floor Mat)

   -------------

   Marimba Tengoku has a secret test menu, see notes near input port definition.
   The ROM check code for that menu is at 0x661010 in ROM and does a full 32-bit word sum
   of the ROM, comparing it against the value stored in the last 4 bytes (it passes)

 */

#include "emu.h"

#include "cpu/xtensa/xtensa.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define LOG_SPRITEBASE       (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

namespace {

class hudson_poems_state : public driver_device
{
public:
	hudson_poems_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_mainram(*this, "mainram"),
		m_palram(*this, "palram")
	{ }

	void hudson_poems(machine_config &config);

	void init_marimba();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void draw_tile(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 tile, int xx, int yy, int gfxbase, int extrapal, bool use_flips);
	void draw_tile8(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 tile, int xx, int yy, int gfxbase, int extrapal, bool use_flips);

	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int which, int priority);

	u32 poems_random_r();
	void unk_trigger_w(offs_t offset, u32 data, u32 mem_mask);

	TIMER_DEVICE_CALLBACK_MEMBER(screen_scanline);

	template<int Which> void tilemap_map(address_map &map, u32 base) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u32 poems_8020010_r();
	u32 poems_count_r();
	u32 poems_unk_r();
	u32 poems_8000038_r(offs_t offset, u32 mem_mask);
	u32 poems_8000200_r(offs_t offset, u32 mem_mask);
	u32 unk_aa04_r(offs_t offset, u32 mem_mask);
	void unk_aa00_w(offs_t offset, u32 data, u32 mem_mask);

	u32 fade_r(offs_t offset, u32 mem_mask);
	void fade_w(offs_t offset, u32 data, u32 mem_mask);
	void unktable_w(offs_t offset, u32 data, u32 mem_mask);
	void unktable_reset_w(offs_t offset, u32 data, u32 mem_mask);
	void set_palette_val(int entry);
	void palette_w(offs_t offset, u32 data, u32 mem_mask);
	void mainram_w(offs_t offset, u32 data, u32 mem_mask);

	void spritegfx_base_w(offs_t offset, u32 data, u32 mem_mask);
	void spritelist_base_w(offs_t offset, u32 data, u32 mem_mask);

	void dma_trigger_w(offs_t offset, u32 data, u32 mem_mask);
	void dma_fill_w(offs_t offset, u32 data, u32 mem_mask);
	void dma_source_w(offs_t offset, u32 data, u32 mem_mask);
	void dma_dest_w(offs_t offset, u32 data, u32 mem_mask);
	void dma_length_w(offs_t offset, u32 data, u32 mem_mask);
	void dma_mode_w(offs_t offset, u32 data, u32 mem_mask);

	template<int Layer> void tilemap_base_w(offs_t offset, u32 data, u32 mem_mask);
	template<int Layer> void tilemap_unk_w(offs_t offset, u32 data, u32 mem_mask);
	template<int Layer> void tilemap_cfg_w(offs_t offset, u32 data, u32 mem_mask);
	template<int Layer> void tilemap_scr_w(offs_t offset, u32 data, u32 mem_mask);
	template<int Layer> void tilemap_high_w(offs_t offset, u32 data, u32 mem_mask);

	template<int Layer> u32 tilemap_base_r(offs_t offset, u32 mem_mask);
	template<int Layer> u32 tilemap_high_r(offs_t offset, u32 mem_mask);
	template<int Layer> u32 tilemap_unk_r(offs_t offset, u32 mem_mask);
	template<int Layer> u32 tilemap_cfg_r(offs_t offset, u32 mem_mask);
	template<int Layer> u32 tilemap_scr_r(offs_t offset, u32 mem_mask);

	u16 m_unktable[256];
	u16 m_unktableoffset;

	u32 m_spritegfxbase[4];
	u32 m_spritelistbase;
	u32 m_tilemapbase[4];
	u32 m_tilemapunk[4];
	u32 m_tilemapcfg[4];
	u32 m_tilemapscr[4];
	u32 m_tilemaphigh[4];

	u32 m_fade;

	u32 m_dmamode;
	u32 m_dmasource;
	u32 m_dmadest;
	u32 m_dmalength;
	u32 m_dmafill;

	s32 m_hackcounter;

	required_device<xtensa_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<u32> m_mainram;
	required_shared_ptr<u32> m_palram;
};

void hudson_poems_state::machine_start()
{
	save_item(NAME(m_unktable));
	save_item(NAME(m_unktableoffset));

	save_item(NAME(m_spritegfxbase));
	save_item(NAME(m_spritelistbase));

	save_item(NAME(m_tilemapbase));
	save_item(NAME(m_tilemapunk));
	save_item(NAME(m_tilemapcfg));
	save_item(NAME(m_tilemapscr));
	save_item(NAME(m_tilemaphigh));

	save_item(NAME(m_hackcounter));

	save_item(NAME(m_fade));

	save_item(NAME(m_dmamode));
	save_item(NAME(m_dmasource));
	save_item(NAME(m_dmadest));
	save_item(NAME(m_dmalength));
	save_item(NAME(m_dmafill));
}



void hudson_poems_state::machine_reset()
{
	m_unktableoffset = 0;
	m_hackcounter = 0;

	for (int i = 0; i < 4; i++)
	{
		m_spritegfxbase[i] = m_tilemapbase[i] = m_tilemaphigh[i] = m_tilemapunk[i] = m_tilemapcfg[i] = m_tilemapscr[i] = 0;
	}

	m_fade = 0;

	m_dmamode = 0;
	m_dmasource = 0;
	m_dmadest = 0;
	m_dmalength = 0;
	m_dmafill = 0;
}

/*
You can unlock the test menu on boot by inputing a 3 step sequence while the Konami logo is being shown:
Step 1 checks if you input white or yellow 2 times
Step 2 checks if you input red or blue 2 times
Step 3 checks if you input white or green 3 times

These are not grouped inputs, so for step 1 you must input white x2 or yellow x2 instead of white x1 and yellow x1 for it to count.

*/

static INPUT_PORTS_START( hudson_poems )
	PORT_START( "IN1" )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("White")
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Yellow (Select Up)")
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Red (Ok)")
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Blue (Select Down)")
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("Green")
INPUT_PORTS_END

static INPUT_PORTS_START( poemgolf )
	PORT_START( "IN1" )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON1 ) // O
INPUT_PORTS_END

static INPUT_PORTS_START( poemzet )
	PORT_START( "IN1" )
INPUT_PORTS_END

static INPUT_PORTS_START( poemspoo )
	PORT_START( "IN1" )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void hudson_poems_state::draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int spritebase = (m_spritelistbase & 0x0003ffff) / 4;
	gfx_element *gfx = m_gfxdecode->gfx(2);

	for (int i = 0; i < 64; i++)
	{
		const u16 spriteword0 = m_mainram[(spritebase + i * 2) + 0] & 0xffff;
		const u16 spriteword1 = m_mainram[(spritebase + i * 2) + 0] >> 16;
		const u16 spriteword2 = m_mainram[(spritebase + i * 2) + 1] & 0xffff;
		const u16 spriteword3 = m_mainram[(spritebase + i * 2) + 1] >> 16;

		const int x = (spriteword3 & 0x03ff);
		const int y = (spriteword2 & 0x03ff);
		int tilenum = spriteword1 & 0x07ff;
		const int pal = (spriteword2 & 0x7c00)>>10;

		// is it selecting from multiple tile pages (which can have different bases?) (probably from a register somewhere)
		const int tilebase = (m_spritegfxbase[(spriteword0 & 0x0300)>>8] & 0x0003ffff) / 32; // m_spritegfxbase contains a full memory address pointer to RAM

		tilenum += tilebase;

		/* based on code analysis of function at 006707A4
		word0 ( 0abb bbBB ---- -dFf ) OR ( 1abb bbcc dddd dddd ) a = ?  b = alpha blend? (toggles between all bits on and off for battery logo) BB = sprite base  F = flipX f = flipY
		word1 ( wwhh -ttt tttt tttt ) - other bits are used, but pulled from ram? t = tile number?  ww = width hh = height
		word2 ( 0Ppp ppyy yyyy yyyy ) P = palette bank p = palette y = ypos
		word3 ( aabb bbxx xxxx xxxx ) x = xpos
		*/

		const int alpha = (spriteword0 & 0x3c00)>>10;
		const int flipx = spriteword0 & 2;
		const int flipy = spriteword0 & 1;
		int height = (spriteword1 & 0x3000)>>12;
		height = 1 << height;
		int width = (spriteword1 & 0xc000)>>14;
		width = 1 << width;

		if (alpha == 0x00)
			continue;


		if (spriteword0 & 0x8000)
		{
			// unsure for now
		}
		else
		{
			const int basetilenum = tilenum;
			for (int xx = 0; xx < width; xx++)
			{
				int xpos, ypos;

				if (!flipx)
					xpos = x + xx * 8;
				else
					xpos = x + (((width-1) * 8) - xx * 8);

				for (int yy = 0; yy < height; yy++)
				{
					if (!flipy)
						ypos = y + yy * 8;
					else
						ypos = y + (((height-1) * 8) - yy * 8);

					if (alpha == 0xf)
						gfx->transpen(bitmap, cliprect, basetilenum + xx + (yy * 0x20), pal, flipx, flipy, xpos, ypos, 0);
					else
						gfx->alpha(bitmap, cliprect, basetilenum + xx + (yy * 0x20), pal, flipx, flipy, xpos, ypos, 0, 0xff-(alpha<<4));

				}
			}
		}
	}

	/*
	popmessage("%08x %08x\n%08x %08x\n%08x %08x\n%08x %08x\n", m_mainram[spritebase + 0], m_mainram[spritebase + 1],
	                                                           m_mainram[spritebase + 2], m_mainram[spritebase + 3],
	                                                           m_mainram[spritebase + 4], m_mainram[spritebase + 5],
	                                                           m_mainram[spritebase + 6], m_mainram[spritebase + 7]);
	*/
}

void hudson_poems_state::draw_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int which, int priority)
{
	int width, base, bpp, gfxbase, extrapal;

	bool use_flips;

	// guess, could be more/less bits, or somewhere else entirely, works for the 2 scenes that need it
	const int thispriority = (m_tilemapcfg[which] & 0x01f00000) >> 20;

	if (thispriority != priority)
		return;

	// seems to get set for disabled layers
	if (m_tilemapcfg[which] & 0x02000000)
		return;

	extrapal = 0;

	if (m_tilemapunk[which] == 2)
		width = 512;
	else
		width = 256;

	if (m_tilemapcfg[which] & 0x20000000)
		bpp = 8;
	else
		bpp = 4;

	// this might not be the correct enable flag
	// marimba needs 2 of the tile bits to act as flip, while poemspoo, poembase, poemgolf use them as tile number bits
	if (m_tilemapcfg[which] & 0x10000000)
		use_flips = false;
	else
		use_flips = true;


	if (m_tilemapcfg[which] & 0x00080000)
		extrapal = 1;
	else
		extrapal = 0;

	// contains a full 32-bit address
	base = (m_tilemapbase[which] & 0x0003ffff) / 4;


	// contains a full 32-bit address.  for this to work in the test mode the interrupts must be disabled as soon as test mode is entered, otherwise the pointer
	// gets overwritten with an incorrect one.  Test mode does not require interrupts to function, although the bit we use to disable them is likely incorrect.
	// this does NOT really seem to be tied to tilemap number, probably from a config reg
	// this reg?
	const int whichbase = (m_tilemapcfg[which] & 0x00000060) >> 5;
	gfxbase = (m_spritegfxbase[whichbase] & 0x0003ffff);

	int yscroll = (m_tilemapscr[which] >> 16) & 0x7ff;
	if (yscroll & 0x400)
		yscroll -= 0x800;

	int xscroll = (m_tilemapscr[which] >> 0) & 0x7ff;
	if (xscroll & 0x400)
		xscroll -= 0x800;

	int tilemap_drawheight = ((m_tilemaphigh[which] >> 16) & 0xff);
	int tilemap_drawwidth = ((m_tilemaphigh[which] >> 0) & 0x1ff);

	// 0 might be maximum, poemzet2
	if (tilemap_drawheight == 0)
		tilemap_drawheight = 240;

	// 0 might be maximum, poemzet
	if (tilemap_drawwidth == 0)
		tilemap_drawwidth = 320;

	// clamp to size of tilemap (test mode has 256 wide tilemap in RAM, but sets full 320 width?)
	if (tilemap_drawwidth > width)
		tilemap_drawwidth = width;

	// note m_tilemaphigh seems to be in pixels, we currently treat it as tiles
	// these could actually be 'end pos' regs, with unknown start regs currently always set to 0
	for (int y = 0; y < tilemap_drawheight / 8; y++)
	{
		const int ypos = (y * 8 + yscroll) & 0x1ff;

		for (int x = 0; x < tilemap_drawwidth / 8 / 2; x++)
		{
			u32 tiles = m_mainram[base + (y * width / 8 / 2) + x];
			const int xpos = (x * 16 + xscroll) & 0x1ff;

			if (bpp == 4)
			{
				draw_tile(screen, bitmap, cliprect, (tiles & 0xffff), xpos, ypos, gfxbase, extrapal, use_flips);
				draw_tile(screen, bitmap, cliprect, ((tiles >> 16) & 0xffff), xpos + 8, ypos, gfxbase, extrapal, use_flips);
			}
			else if (bpp == 8)
			{
				draw_tile8(screen, bitmap, cliprect, (tiles & 0xffff), xpos, ypos, gfxbase, extrapal, use_flips);
				draw_tile8(screen, bitmap, cliprect, ((tiles >> 16) & 0xffff), xpos + 8, ypos, gfxbase, extrapal, use_flips);
			}
		}
	}
}



void hudson_poems_state::spritegfx_base_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_spritegfxbase[offset]);
	LOGMASKED(LOG_SPRITEBASE, "%s: spritegfx_base_w %d %08x\n", machine().describe_context(), offset, data);
}

void hudson_poems_state::spritelist_base_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_spritelistbase);
}

template<int Layer> void hudson_poems_state::tilemap_base_w(offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_tilemapbase[Layer]); }
template<int Layer> void hudson_poems_state::tilemap_cfg_w(offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_tilemapcfg[Layer]); }
template<int Layer> void hudson_poems_state::tilemap_unk_w(offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_tilemapunk[Layer]); }
template<int Layer> void hudson_poems_state::tilemap_high_w(offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_tilemaphigh[Layer]); }
template<int Layer> void hudson_poems_state::tilemap_scr_w(offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_tilemapscr[Layer]); }

template<int Layer> u32 hudson_poems_state::tilemap_base_r(offs_t offset, u32 mem_mask) { return m_tilemapbase[Layer]; }
template<int Layer> u32 hudson_poems_state::tilemap_high_r(offs_t offset, u32 mem_mask) { return m_tilemaphigh[Layer]; }
template<int Layer> u32 hudson_poems_state::tilemap_unk_r(offs_t offset, u32 mem_mask) { return m_tilemapunk[Layer]; }
template<int Layer> u32 hudson_poems_state::tilemap_cfg_r(offs_t offset, u32 mem_mask) { return m_tilemapcfg[Layer]; }
template<int Layer> u32 hudson_poems_state::tilemap_scr_r(offs_t offset, u32 mem_mask) { return m_tilemapscr[Layer]; }


void hudson_poems_state::draw_tile(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 tile, int xx, int yy, int gfxbase, int extrapal, bool use_flips)
{
	gfx_element *gfx = m_gfxdecode->gfx(2);
	int flipx = 0;
	int flipy = 0;
	int pal = (tile & 0xf000)>>12;

	if (use_flips)
	{
		flipx = tile & 0x0800;
		flipy = tile & 0x0400;
		tile &= 0x3ff;
	}
	else
	{
		tile &= 0xfff;
	}

	pal += extrapal * 0x10;
	tile += gfxbase / 32;
	gfx->transpen(bitmap,cliprect,tile,pal,flipx,flipy,xx,yy, 0);
}

void hudson_poems_state::draw_tile8(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 tile, int xx, int yy, int gfxbase, int extrapal, bool use_flips)
{
	gfx_element *gfx = m_gfxdecode->gfx(3);
	int flipx = 0;
	int flipy = 0;
	int pal = 0;//(tile & 0xf000)>>8;

	if (use_flips)
	{
		flipx = tile & 0x0800;
		flipy = tile & 0x0400;
		tile &= 0x3ff;
	}
	else
	{
		tile &= 0xfff;
	}

	pal += extrapal;
	tile += gfxbase / 64;
	gfx->transpen(bitmap,cliprect,tile,pal,flipx,flipy,xx,yy, 0);
}

u32 hudson_poems_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const paldata = m_palette->pens();
	bitmap.fill(paldata[0x100], cliprect); // poemzet 'play poems' logo (maybe lowest enabled tilemap is just opaque instead?)

	for (int pri = 0x1f; pri >= 0; pri--)
	{
		draw_tilemap(screen, bitmap, cliprect, 0, pri);
		draw_tilemap(screen, bitmap, cliprect, 1, pri); // title screen background
		draw_tilemap(screen, bitmap, cliprect, 2, pri); // seems to be status bar in the game demo, but suggests that tilegfx base isn't tied to tilmap number
		draw_tilemap(screen, bitmap, cliprect, 3, pri); // background for the character in game demo etc.
	}

	draw_sprites(screen, bitmap, cliprect);

	return 0;
}

u32 hudson_poems_state::poems_unk_r()
{
	return 0x00000000;
}

u32 hudson_poems_state::poems_random_r()
{
	return machine().rand();
}


u32 hudson_poems_state::poems_count_r()
{
	return ((m_hackcounter++) & 0x3) ? 0xffffffff : 0;
}

u32 hudson_poems_state::poems_8020010_r()
{
	return 0x4;
}

u32 hudson_poems_state::unk_aa04_r(offs_t offset, u32 mem_mask)
{
	return ((m_hackcounter++) & 0x3) ? 0xffffffff : 0;
}

void hudson_poems_state::unk_aa00_w(offs_t offset, u32 data, u32 mem_mask)
{
	logerror("%s: unk_aa00_w %08x %08x\n", machine().describe_context(), data, mem_mask);
}

u32 hudson_poems_state::poems_8000038_r(offs_t offset, u32 mem_mask)
{
	if (!machine().side_effects_disabled())
		if (m_maincpu->pc() != 0x2c000b5a)
			logerror("%s: poems_8000038_r %08x\n", machine().describe_context(), mem_mask);

	if (machine().rand() & 1)
		return 0xffffffff;
	else
		return 0x00000000;

	/*
	if (m_mainram[0x1baf8/4] == 0x00000000)
	    return 0xffffffff;
	else
	    return 0x00000000;
	*/
}

u32 hudson_poems_state::poems_8000200_r(offs_t offset, u32 mem_mask)
{
	if (!machine().side_effects_disabled())
		logerror("%s: poems_8000200_r %08x\n", machine().describe_context(), mem_mask);

	// in some places it loops on bit 0 being clear, in other places it seems to simply be used like an ack
	return 0x00000001;
}

void hudson_poems_state::set_palette_val(int entry)
{
	const u16 datax = m_palram[entry];
	const int b = ((datax) & 0x001f) >> 0;
	const int g = ((datax) & 0x03e0) >> 5;
	const int r = ((datax) & 0x7c00) >> 10;
	m_palette->set_pen_color(entry, pal5bit(r), pal5bit(g), pal5bit(b));
}

void hudson_poems_state::palette_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_palram[offset]);
	set_palette_val(offset);
}

u32 hudson_poems_state::fade_r(offs_t offset, u32 mem_mask)
{
	return m_fade;
}

void hudson_poems_state::fade_w(offs_t offset, u32 data, u32 mem_mask)
{
	logerror("%s: fade_w %08x %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_fade);

	u8 val = 0x1f - ((data >> 16) & 0x1f);

	const double intensity = (double)(val & 0x1f) / (double)0x1f;
	for (int i = 0; i < 0x200; i++)
		m_palette->set_pen_contrast(i, intensity);

}

void hudson_poems_state::unktable_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (mem_mask & 0x0000ffff)
	{
		if (m_unktableoffset < 256)
		{
			m_unktable[m_unktableoffset] = data & 0x0000ffff;
			m_unktableoffset++;
		}
	}

	if (mem_mask & 0xffff0000)
	{
		if (m_unktableoffset < 256)
		{
			m_unktable[m_unktableoffset] = (data & 0xffff0000)>>16;
			m_unktableoffset++;
		}
	}
}

void hudson_poems_state::unktable_reset_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_unktableoffset = 0;
}

void hudson_poems_state::unk_trigger_w(offs_t offset, u32 data, u32 mem_mask)
{
	logerror("%s: unk_trigger_w %08x %08x\n", machine().describe_context(), data, mem_mask);
	m_maincpu->set_input_line(0x4, ASSERT_LINE);
}


void hudson_poems_state::mainram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_mainram[offset]);

	m_gfxdecode->gfx(2)->mark_dirty(offset / 8);
	m_gfxdecode->gfx(3)->mark_dirty(offset / 16);
}

void hudson_poems_state::dma_trigger_w(offs_t offset, u32 data, u32 mem_mask)
{
	// poembase writes 00030101 here when it hasn't set src / 'mode' regs, maybe for clear operations
	//          writes 00030001 when those things are valid
	address_space &cpuspace = m_maincpu->space(AS_PROGRAM);

	logerror("%s: dma_trigger_w %08x %08x (with source %08x dest %08x length %08x mode %08x)\n", machine().describe_context(), data, mem_mask, m_dmasource, m_dmadest, m_dmalength, m_dmamode);

	if ((data == 0x00030001) && (m_dmamode == 0x00003210))
	{
		int length = m_dmalength;
		offs_t source = m_dmasource;
		offs_t dest = m_dmadest;

		while (length >= 0)
		{
			u32 dmadat = cpuspace.read_dword(source);
			cpuspace.write_dword(dest, dmadat);

			source += 4;
			dest += 4;
			length--;
		}
	}
	else if (data == 0x00030101) // mode is not used here, but fill is
	{
		int length = m_dmalength;
		offs_t dest = m_dmadest;

		while (length >= 0)
		{
			cpuspace.write_dword(dest, m_dmafill);

			dest += 4;
			length--;
		}
	}
	else
	{
		logerror("unsure how to handle this DMA, ignoring\n");
	}
}

void hudson_poems_state::dma_source_w(offs_t offset, u32 data, u32 mem_mask)
{
	logerror("%s: dma_source_w %08x %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_dmasource);
}

void hudson_poems_state::dma_dest_w(offs_t offset, u32 data, u32 mem_mask)
{
	logerror("%s: dma_dest_w %08x %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_dmadest);
}

void hudson_poems_state::dma_length_w(offs_t offset, u32 data, u32 mem_mask)
{
	logerror("%s: dma_length_w %08x %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_dmalength);
}

void hudson_poems_state::dma_mode_w(offs_t offset, u32 data, u32 mem_mask)
{
	logerror("%s: dma_mode_w %08x %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_dmamode);
}

void hudson_poems_state::dma_fill_w(offs_t offset, u32 data, u32 mem_mask)
{
	logerror("%s: dma_fill_w %08x %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_dmafill);
}

template <int Which>
void hudson_poems_state::tilemap_map(address_map &map, u32 base)
{
	map(base+0x00, base+0x03).rw(FUNC(hudson_poems_state::tilemap_cfg_r<Which>), FUNC(hudson_poems_state::tilemap_cfg_w<Which>));
	map(base+0x04, base+0x07).rw(FUNC(hudson_poems_state::tilemap_base_r<Which>), FUNC(hudson_poems_state::tilemap_base_w<Which>));
	map(base+0x08, base+0x0b).rw(FUNC(hudson_poems_state::tilemap_unk_r<Which>), FUNC(hudson_poems_state::tilemap_unk_w<Which>)); // usually 1 or 2 (tilemap width in ram?)
	map(base+0x0c, base+0x0f).ram(); // usually 0 or 1
	map(base+0x10, base+0x13).ram(); // not used?
	map(base+0x14, base+0x17).ram(); // not used?
	map(base+0x18, base+0x1b).rw(FUNC(hudson_poems_state::tilemap_high_r<Which>), FUNC(hudson_poems_state::tilemap_high_w<Which>)); // top word is display height, bottom word seems to be display width
	map(base+0x1c, base+0x1f).rw(FUNC(hudson_poems_state::tilemap_scr_r<Which>), FUNC(hudson_poems_state::tilemap_scr_w<Which>)); // top word is y scroll, bottom word is x scroll
	map(base+0x20, base+0x23).ram(); // used, always 00001000 (maybe zoom?)
	map(base+0x24, base+0x27).ram(); // not used?
	map(base+0x28, base+0x2b).ram(); // not used?
	map(base+0x2c, base+0x2f).ram(); // used, always 00001000 (maybe zoom?)
	map(base+0x30, base+0x33).ram(); // not used?
	map(base+0x34, base+0x37).ram(); // not used?
	map(base+0x38, base+0x3b).ram(); // not used?
	map(base+0x3c, base+0x3f).ram(); // not used?
}

void hudson_poems_state::mem_map(address_map &map)
{
	map(0x00000000, 0x007fffff).mirror(0x20000000).rom().region("maincpu", 0);

	/////////////////// unknown
	map(0x04000040, 0x04000043).r(FUNC(hudson_poems_state::poems_random_r)).nopw();
	map(0x04000140, 0x04000143).r(FUNC(hudson_poems_state::poems_random_r)).nopw();
	map(0x04000240, 0x04000243).r(FUNC(hudson_poems_state::poems_random_r)).nopw();
	map(0x04000340, 0x04000343).r(FUNC(hudson_poems_state::poems_random_r)).nopw();

	//map(0x04000324, 0x04000327).nopw(); // uploads a table here from ROM after uploading fixed values from ROM to some other addresses
	map(0x0400033c, 0x0400033f).w(FUNC(hudson_poems_state::unk_trigger_w)); // maybe DMA?
	map(0x04001000, 0x04001003).r(FUNC(hudson_poems_state::poems_random_r)).nopw();


	map(0x0400100c, 0x0400100f).nopr(); // read in various places at end of calls, every frame, but result seems to go unused?
	map(0x04002040, 0x04002043).portr("IN1");

	/////////////////// palette / regs?

	map(0x08000038, 0x0800003b).r(FUNC(hudson_poems_state::poems_8000038_r));
	//map(0x0800003c, 0x0800003f).nopw(); // used close to the fade write code, writes FFFFFFFF

	//map(0x08000048, 0x0800004b).nopw(); // ^^
	//map(0x0800004c, 0x0800004f).nopw(); // ^^
	//map(0x08000050, 0x08000053).nopw(); // ^^ 16-bit write, sometimes writes 00000101 & 0000FFFF
	//map(0x08000054, 0x08000057).nopw(); // ^^ writes 15555555 while fading
	map(0x0800005c, 0x0800005f).rw(FUNC(hudson_poems_state::fade_r), FUNC(hudson_poems_state::fade_w));

	// are these runtime registers, or DMA sources?
	map(0x08000070, 0x0800007f).w(FUNC(hudson_poems_state::spritegfx_base_w)); // ^^ sometimes writes 2C009C00 (one of the tile data bases)

	// registers 0x080 - 0x0bf are tilemap 0
	tilemap_map<0>(map, 0x08000080);
	// registers 0x0c0 - 0x0ff are tilemap 1
	tilemap_map<1>(map, 0x080000c0);
	// registers 0x100 - 0x13f are tilemap 2
	tilemap_map<2>(map, 0x08000100);
	// registers 0x140 - 0x17f are tilemap 3
	tilemap_map<3>(map, 0x08000140);

	// registers at 0x180 are sprites

	//map(0x08000180, 0x08000183).nopw(); // gets set to 0000007F on startup (list length?)
	map(0x08000184, 0x08000187).w(FUNC(hudson_poems_state::spritelist_base_w)); // gets set to 2C009400 on startup

	map(0x08000800, 0x08000fff).ram().w(FUNC(hudson_poems_state::palette_w)).share("palram");

	/////////////////// sound regs??

	map(0x08008000, 0x08008003).nopw();

	map(0x08008200, 0x08008203).r(FUNC(hudson_poems_state::poems_8000200_r));
	map(0x08008204, 0x08008207).nopr().nopw(); // read only seems to be used to ack after write, value doesn't matter, similar code to 200 above

	map(0x08008400, 0x08008403).nopw();

	map(0x08008a00, 0x08008a7f).ram(); // filled with 32 copies of '0x0001'
	map(0x08008c00, 0x08008c7f).ram(); // filled with 32 copies of '0x0003'
	map(0x08009000, 0x080093ff).ram(); // filled with 32 copies of '0x0200, 0x0004, 0x0000, 0x0100, 0x0c02, 0x0000, 0x0000, 0x0000'
	map(0x08009400, 0x080097ff).ram(); // filled with 32 copies of '0x0000, 0x3fff, 0x0000, 0x0000, 0x3fff, 0x0000, 0x0000, 0x0000'
	map(0x08009800, 0x08009bff).ram(); // filled with 32 copies of '0x0080, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000'

	map(0x0800a000, 0x0800a003).nopw();
	map(0x0800a004, 0x0800a007).nopw();

	map(0x0800aa00, 0x0800aa03).w(FUNC(hudson_poems_state::unk_aa00_w)); // writes 2c020000, which is the top of RAM? (or middle?)
	map(0x0800aa04, 0x0800aa07).r(FUNC(hudson_poems_state::unk_aa04_r));

	map(0x0800b000, 0x0800b003).w(FUNC(hudson_poems_state::unktable_w)); // writes a table of increasing 16-bit values here
	map(0x0800b004, 0x0800b007).w(FUNC(hudson_poems_state::unktable_reset_w));

	map(0x0800c040, 0x0800c05f).ram();

	///////////////////
	map(0x0801c000, 0x0801c003).w(FUNC(hudson_poems_state::dma_trigger_w));
	map(0x0801c004, 0x0801c007).w(FUNC(hudson_poems_state::dma_source_w));
	map(0x0801c008, 0x0801c00b).w(FUNC(hudson_poems_state::dma_length_w));
	map(0x0801c018, 0x0801c01b).w(FUNC(hudson_poems_state::dma_dest_w));
	map(0x0801c024, 0x0801c027).w(FUNC(hudson_poems_state::dma_fill_w));
	map(0x0801c028, 0x0801c02b).w(FUNC(hudson_poems_state::dma_mode_w));

	map(0x08020000, 0x08020003).nopr().nopw();
	map(0x08020004, 0x08020007).nopr().nopw();
	map(0x08020008, 0x0802000b).nopr();
	map(0x08020010, 0x08020013).r(FUNC(hudson_poems_state::poems_8020010_r));
	map(0x08020014, 0x08020017).nopw(); // writes 0x10
	map(0x08020018, 0x0802001b).r(FUNC(hudson_poems_state::poems_unk_r)).nopw(); // writes 0x10
	map(0x08020020, 0x08020023).r(FUNC(hudson_poems_state::poems_count_r));

	///////////////////

	map(0x2c000000, 0x2c03ffff).ram().w(FUNC(hudson_poems_state::mainram_w)).share("mainram");
}

static GFXDECODE_START( gfx_poems )
	GFXDECODE_ENTRY( "maincpu", 0, gfx_8x8x4_packed_lsb, 0, 32 )
	GFXDECODE_ENTRY( "maincpu", 0, gfx_8x8x8_raw, 0, 2 )

	GFXDECODE_RAM( "mainram", 0, gfx_8x8x4_packed_lsb, 0, 32 )
	GFXDECODE_RAM( "mainram", 0, gfx_8x8x8_raw, 0, 2 )
GFXDECODE_END

TIMER_DEVICE_CALLBACK_MEMBER(hudson_poems_state::screen_scanline)
{
	const int scanline = param;

	if (scanline == 100)
	{
		if (m_tilemapunk[0] != 1) // wrong, this is probably just tilemap size, with 1 being 256 and 2 being 512, but prevents unwanted IRQs in Test Mode
			m_maincpu->set_input_line(0x10, ASSERT_LINE);
	}

	if (scanline == 150)
	{
		m_maincpu->set_input_line(0x04, CLEAR_LINE);
	}
}

void hudson_poems_state::hudson_poems(machine_config &config)
{
	// 27Mhz XTAL, Xtensa based CPU
	XTENSA(config, m_maincpu, 27_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &hudson_poems_state::mem_map);
	// the vectors can be configured when the CPU is manufactured, so treat as config options
	m_maincpu->set_startupvector(0x00000040);
	m_maincpu->set_irq_vector(4, 0x2c0001b4); // 0x10
	m_maincpu->set_irq_vector(1, 0x2c000194); // 0x02
	m_maincpu->set_irq_vector(2, 0x2c000194); // 0x04
	// 0x2c0001b4 these are also valid vectors
	// 0x2c0001c4
	// 0x2c0001d4
	// 0x2c000000 is a register window exception (4)
	// 0x2c000040 is a register window exception (4)
	// 0x2c000080 is a register window exception (8)
	// 0x2c0000c0 is a register window exception (8)
	// 0x2c000100 is a register window exception (12)
	// 0x2c000140 is a register window exception (12)

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(320, 240);
	screen.set_visarea(0, 320-1, 0, 240-1);
	screen.set_screen_update(FUNC(hudson_poems_state::screen_update));

	TIMER(config, "scantimer").configure_scanline(FUNC(hudson_poems_state::screen_scanline), "screen", 0, 1);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_poems);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x200);

	SPEAKER(config, "speaker").front_center();
}

void hudson_poems_state::init_marimba()
{
}

ROM_START( marimba )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD( "marimbatengoku.u2", 0x000000, 0x800000, CRC(b2ac0c5b) SHA1(48f3cdf399b032d86234125eeac3fb1cdc73538a) ) // glob with TSOP pads

	ROM_REGION( 0x400, "nv", 0 )
	ROM_LOAD( "at24c08a.u4", 0x000000, 0x400, CRC(e128a679) SHA1(73fb551d87ed911bd469899343fd36d9d579af39) )
ROM_END

ROM_START( poemgolf )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "poems_golf.u2", 0x000000, 0x400000, CRC(9684a232) SHA1(8a7b97e274dfdddfb6af4df13d714947ef01f29e) ) // glob with TSOP pads

	ROM_REGION( 0x400, "nv", 0 )
	ROM_LOAD( "at24c08a.u3", 0x000000, 0x400, CRC(55856855) SHA1(27b9b42eeea8dd06be43886e40bb2396efc88a67) )
ROM_END

ROM_START( poembase )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "poems_baseball.u2", 0x000000, 0x400000, CRC(105e6c37) SHA1(2a4fe66c08a57bde25047479cbd6ed9ca7c78fa2) ) // glob with TSOP pads

	ROM_REGION( 0x400, "nv", 0 )
	ROM_LOAD( "at24c08a.u3", 0x000000, 0x400, CRC(b83afff4) SHA1(5501e490177b69d61099cc8f1439b91320572584) )
ROM_END

ROM_START( poemzet )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "poems_denjaras.u2", 0x000000, 0x400000, CRC(278d74c2) SHA1(1bd02cce890778409151b20a048892ba3692fd9b) ) // glob with TSOP pads

	ROM_REGION( 0x400, "nv", 0 )
	ROM_LOAD( "at24c08a.u3", 0x000000, 0x400, CRC(594c7c3d) SHA1(b1ddf1d30267f10dac00064b60d8a6b594ae6fc1) )
ROM_END

ROM_START( poemzet2 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "poems_oldman.u2", 0x000000, 0x400000, CRC(d721c4a9) SHA1(59d88c7f515d10d98e00ac076ebd7ce30cb0bb27) ) // glob with TSOP pads

	ROM_REGION( 0x400, "nv", 0 )
	ROM_LOAD( "at24c08a.u3", 0x000000, 0x400, CRC(5f3d5352) SHA1(94386f5703751e66d6a62e23c344ab7711aad769) )
ROM_END

ROM_START(poemspoo)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "poems_spoo.u2", 0x000000, 0x400000, CRC(1ebc473a) SHA1(6ebc33f274f71183bcd4a93d06e565eff71e2f47) ) // glob with TSOP pads

	// seeprom position not populated
ROM_END


} // anonymous namespace


CONS( 2005, marimba,      0,       0,      hudson_poems, hudson_poems, hudson_poems_state, init_marimba, "Konami", "Marimba Tengoku (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND )

// waits for 2c008f00 to become 0 (missing irq?) happens before it gets to the DMA transfers
CONS( 2004, poemgolf,     0,       0,      hudson_poems, poemgolf,     hudson_poems_state, init_marimba, "Konami", "Soukai Golf Champ (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND )
// waits for 2c005d00 to become 0 (missing irq?) happens before it gets to the DMA transfers
CONS( 2004, poembase,     0,       0,      hudson_poems, poemgolf,     hudson_poems_state, init_marimba, "Konami", "Nekketsu Pawapuro Champ (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND )

CONS( 2004, poemzet,      0,       0,      hudson_poems, poemzet,      hudson_poems_state, init_marimba, "Konami", "Zettai Zetsumei Dangerous Jiisan - Mini Game de Taiketsu ja!", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND )

CONS( 2005, poemzet2,     0,       0,      hudson_poems, poemzet,      hudson_poems_state, init_marimba, "Konami", "Zettai Zetsumei Dangerous Jiisan Party ja! Zen-in Shuugou!!", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND )

CONS( 2005, poemspoo,     0,       0,      hudson_poems, poemspoo,     hudson_poems_state, init_marimba, "Konami", "Goo Choco Lantan Spoo Daisuki! Playmat", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND)
