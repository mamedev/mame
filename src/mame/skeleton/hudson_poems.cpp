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

   Marimba Tengoku has a secret test menue (unsure how to access it)
   the ROM check code for that menu is at 0x661010 in ROM and does a full 32-bit word sum
   of the ROM, comparing it against the value stored in the last 4 bytes (it passes)

 */

#include "emu.h"

#include "cpu/xtensa/xtensa.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


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
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void draw_tile(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t tile, int xx, int yy, int gfxbase, int extrapal);
	void draw_tile8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t tile, int xx, int yy, int gfxbase, int extrapal);

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint32_t poems_random_r();
	void unk_trigger_w(offs_t offset, u32 data, u32 mem_mask);

	TIMER_DEVICE_CALLBACK_MEMBER(screen_scanline);

	void mem_map(address_map &map);

	uint32_t poems_8020010_r();
	uint32_t poems_count_r();
	uint32_t poems_unk_r();
	uint32_t poems_8000038_r(offs_t offset, u32 mem_mask);
	uint32_t poems_8000200_r(offs_t offset, u32 mem_mask);
	uint32_t unk_aa04_r(offs_t offset, u32 mem_mask);
	void unk_aa00_w(offs_t offset, u32 data, u32 mem_mask);

	void fade_w(offs_t offset, u32 data, u32 mem_mask);
	void unktable_w(offs_t offset, u32 data, u32 mem_mask);
	void unktable_reset_w(offs_t offset, u32 data, u32 mem_mask);
	void set_palette_val(int entry);
	void palette_w(offs_t offset, u32 data, u32 mem_mask);
	void mainram_w(offs_t offset, u32 data, u32 mem_mask);

	void spritegfx_base_w(offs_t offset, u32 data, u32 mem_mask);
	void spritelist_base_w(offs_t offset, u32 data, u32 mem_mask);
	void tilemap_base_w(offs_t offset, u32 data, u32 mem_mask);
	void tilemap_unk_w(offs_t offset, u32 data, u32 mem_mask);
	void tilemap_cfg_w(offs_t offset, u32 data, u32 mem_mask);
	void tilemap_scr_w(offs_t offset, u32 data, u32 mem_mask);


	uint16_t m_unktable[256];
	uint16_t m_unktableoffset;

	uint32_t m_spritegfxbase[4];
	uint32_t m_spritelistbase;
	uint32_t m_tilemapbase;
	uint32_t m_tilemapunk;
	uint32_t m_tilemapscr;

	int m_hackcounter;

	required_device<xtensa_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<u32> m_mainram;
	required_shared_ptr<u32> m_palram;
};

void hudson_poems_state::machine_start()
{
	save_item(NAME(m_spritegfxbase));

}

void hudson_poems_state::machine_reset()
{
	m_maincpu->set_pc(0x00000040);
	m_unktableoffset = 0;
	m_hackcounter = 0;
	m_tilemapscr = 0;
}

/*
You can unlock the test menu on boot by inputing a 3 step sequence while the Konami logo is being shown:
Step 1 checks if you input white or yellow 2 times
Step 2 checks if you input red or blue 2 times
Step 3 checks if you input white or green 3 times

These are not grouped inputs, so for step 1 you must input white x2 or yellow x2 instead of white x1 and yellow x1 for it to count.
Not tested on real hardware yet but you can see the test menu render in memory at 2c002a96.

*/

static INPUT_PORTS_START( hudson_poems )
	PORT_START( "IN1" )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("White")
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Yellow (Select Up)")
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Red (Ok)")
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Blue (Select Down)")
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("Green")
INPUT_PORTS_END

void hudson_poems_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int spritebase = (m_spritelistbase & 0x0003ffff) / 4;
	gfx_element *gfx = m_gfxdecode->gfx(2);

	for (int i = 0; i < 64; i++)
	{
		int tilebase;
		uint16_t spriteword0 = m_mainram[(spritebase + i * 2) + 0] & 0xffff;
		uint16_t spriteword1 = m_mainram[(spritebase + i * 2) + 0] >> 16;
		uint16_t spriteword2 = m_mainram[(spritebase + i * 2) + 1] & 0xffff;
		uint16_t spriteword3 = m_mainram[(spritebase + i * 2) + 1] >> 16;

		int x = (spriteword3 & 0x03ff);
		int y = (spriteword2 & 0x03ff);
		int tilenum = spriteword1 & 0x03ff;
		int pal = (spriteword2 & 0x7c00)>>10;

		// is it selecting from multiple tile pages (which can have different bases?) (probably from a register somewhere)
		tilebase = (m_spritegfxbase[(spriteword0 & 0x0300)>>8] & 0x0003ffff) / 32; // m_spritegfxbase contains a full memory address pointer to RAM
	
		tilenum += tilebase;

		/* based on code analysis of function at 006707A4
		word0 ( 0abb bbBB ---- -dFf ) OR ( 1abb bbcc dddd dddd ) BB = sprite base  F = flipX f = flipY
		word1 ( wwhh ---t tttt tttt ) - other bits are used, but pulled from ram? t = tile number?  ww = width hh = height
		word2 ( 0Ppp ppyy yyyy yyyy ) P = palette bank p = palette y = ypos
		word3 ( aabb bbxx xxxx xxxx ) x = xpos
		*/

		int flipx = spriteword0 & 2;
		int flipy = spriteword0 & 1;
		int height = (spriteword1 & 0x3000)>>12;
		height = 1 << height;
		int width = (spriteword1 & 0xc000)>>14;
		width = 1 << width;

		if (spriteword0 & 0x8000)
		{

		}
		else
		{
			int basetilenum = tilenum;
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

					gfx->transpen(bitmap, cliprect, basetilenum + xx + (yy * 0x20), pal, flipx, flipy, xpos, ypos, 0);
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

void hudson_poems_state::spritegfx_base_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_spritegfxbase[offset]);
	logerror("%s: spritegfx_base_w %d %08x\n", machine().describe_context(), offset, data);
}

void hudson_poems_state::tilemap_base_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_tilemapbase);
}

void hudson_poems_state::tilemap_cfg_w(offs_t offset, u32 data, u32 mem_mask)
{
	logerror("tilemap_cfg_w %08x\n", data);
}

void hudson_poems_state::tilemap_unk_w(offs_t offset, u32 data, u32 mem_mask)
{
	//logerror("m_tilemapunk %08x\n", data);
	COMBINE_DATA(&m_tilemapunk);
}

void hudson_poems_state::tilemap_scr_w(offs_t offset, u32 data, u32 mem_mask)
{
	logerror("tilemap_scroll_w %08x\n", data);
	COMBINE_DATA(&m_tilemapscr);
}

void hudson_poems_state::spritelist_base_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_spritelistbase);
}

void hudson_poems_state::draw_tile(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t tile, int xx, int yy, int gfxbase, int extrapal)
{
	gfx_element *gfx = m_gfxdecode->gfx(2);
	int flipx = tile & 0x0800;
	int flipy = tile & 0x0400;
	int pal = (tile & 0xf000)>>12;
	pal += extrapal * 0x10;
	tile &= 0x3ff;
	tile += gfxbase / 32;
	gfx->opaque(bitmap,cliprect,tile,pal,flipx,flipy,xx,yy);
}

void hudson_poems_state::draw_tile8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t tile, int xx, int yy, int gfxbase, int extrapal)
{
	gfx_element *gfx = m_gfxdecode->gfx(3);
	int flipx = tile & 0x0800;
	int flipy = tile & 0x0400;
	int pal = 0;//(tile & 0xf000)>>8;
	pal += extrapal;
	tile &= 0x3ff;
	tile += gfxbase / 64;
	gfx->opaque(bitmap,cliprect,tile,pal,flipx,flipy,xx,yy);
}

uint32_t hudson_poems_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	int width, base, bpp, gfxbase, extrapal;

	bool attempt_draw = true;

	int hack_select = m_mainram[0x9000 / 4];

	switch (hack_select)
	{
	case 0x09:
	{
		if ((m_mainram[0x1a00 / 4]) == 0xffffffff) // gross hack, this is the tile data!
		{
			width = 256; bpp = 4; extrapal = 0; break;// test mode
		}
		else
		{
			width = 512;  bpp = 4; extrapal = 0; break;// konami logo
		}
	}
	case 0x0d: width = 512; bpp = 8; extrapal = 0; break;// poems logo
	case 0x10: width = 512; bpp = 4; extrapal = 0; break;// bemani logo
	case 0x14: width = 512; bpp = 4; extrapal = 0; break;// warning screen
	case 0x38: width = 512; bpp = 4; extrapal = 0; break;// title 1 logo (shouldn't be this tall?, contains the top half of below)
	//case 0x38: width = 512; base = (0x14800/4); bpp = 4; gfxbase = 0x9800+0x7800; extrapal = 1; break;// title 1 background
	//case 0x38: width = 512; base = (0x15800/4); bpp = 4; gfxbase = 0x9800+0x7800; extrapal = 1; break;// title 1 background (same as above, but set to use palette 1)
	//case 0x38: width = 512; base = (0x16800/4); bpp = 4; gfxbase = 0x9800+base_hack; break;// title 1 logo (shouldn't be this tall?, bottom half of title screen, but not the button/text)
	case 0x44: width = 512; bpp = 4; extrapal = 0; break;// game demo
	default: attempt_draw = false; break;
	}

	// contains a full 32-bit address
	base = (m_tilemapbase & 0x0003ffff) / 4;

	// contains a full 32-bit address.  for this to work in the test mode the interrupts must be disabled as soon as test mode is entered, otherwise the pointer
	// gets overwritten with an incorrect one.  Test mode does not require interrupts to function, although the bit we use to disable them is likely incorrect.
	gfxbase = (m_spritegfxbase[0] & 0x0003ffff);

	if (!attempt_draw)
	{
		// just to show that some stuff gets uploaded to mainram if you let execution continue, looks like tilemap or tile data?
		int count = 0;
		for (int y=0;y<224/8;y++)
		{
			for (int x=0;x<256/8;x++)
			{

				count++;
				draw_tile(screen, bitmap, cliprect, count * 8, x * 8, y * 8, 0x9c00, 0);
			}
		}
	}
	else
	{
		int count = 0;
		for (int y = 0; y < 224 / 8; y++)
		{
			int yscroll = (m_tilemapscr >> 16) & 0x7ff;
			if (yscroll & 0x400)
				yscroll -= 0x800;

			int ypos = (y * 8 + yscroll) & 0x1ff;

			for (int x = 0; x < width / 8 / 2; x++)
			{
				uint32_t tiles = m_mainram[base + count];

				if (bpp == 4)
				{
					draw_tile(screen, bitmap, cliprect, (tiles & 0xffff), (x * 16), ypos, gfxbase, extrapal);
					draw_tile(screen, bitmap, cliprect, ((tiles >> 16) & 0xffff), (x * 16) + 8, ypos, gfxbase, extrapal);
				}
				else if (bpp == 8)
				{
					draw_tile8(screen, bitmap, cliprect, (tiles & 0xffff), (x * 16), ypos, gfxbase, extrapal);
					draw_tile8(screen, bitmap, cliprect, ((tiles >> 16) & 0xffff), (x * 16) + 8, ypos, gfxbase, extrapal);
				}

				count++;

			}
		}
	}

	draw_sprites(screen, bitmap, cliprect);

	return 0;
}

uint32_t hudson_poems_state::poems_unk_r()
{
	return 0x00000000;
}

uint32_t hudson_poems_state::poems_random_r()
{
	return machine().rand();
}


uint32_t hudson_poems_state::poems_count_r()
{
	return ((m_hackcounter++) & 0x3) ? 0xffffffff : 0;
}

uint32_t hudson_poems_state::poems_8020010_r()
{
	return 0x4;
}

uint32_t hudson_poems_state::unk_aa04_r(offs_t offset, u32 mem_mask)
{
	//logerror("%s: unk_aa04_r %08x\n", machine().describe_context(), mem_mask);
	return ((m_hackcounter++) & 0x3) ? 0xffffffff : 0;
}

void hudson_poems_state::unk_aa00_w(offs_t offset, u32 data, u32 mem_mask)
{
	logerror("%s: unk_aa00_w %08x %08x\n", machine().describe_context(), data, mem_mask);
}

uint32_t hudson_poems_state::poems_8000038_r(offs_t offset, u32 mem_mask)
{
	if (!machine().side_effects_disabled())
		if (m_maincpu->pc() != 0x2c000b5a)
			logerror("%s: poems_8000038_r %08x\n", machine().describe_context(), mem_mask);

	if (m_mainram[0x1baf8/4] == 0x00000000)
		return 0xffffffff;
	else
		return 0x00000000;
}

uint32_t hudson_poems_state::poems_8000200_r(offs_t offset, u32 mem_mask)
{
	if (!machine().side_effects_disabled())
		logerror("%s: poems_8000200_r %08x\n", machine().describe_context(), mem_mask);

	// in some places it loops on bit 0 being clear, in other places it seems to simply be used like an ack
	return 0x00000001;
}

/*
uint32_t hudson_poems_state::poems_8020020_r()
{
    return 0xffffffff;
}

*/

void hudson_poems_state::set_palette_val(int entry)
{
	// not actually palette, but just to visualize it for now
	uint16_t datax = m_palram[entry];
	int b = ((datax) & 0x001f) >> 0;
	int g = ((datax) & 0x03e0) >> 5;
	int r = ((datax) & 0x7c00) >> 10;
	m_palette->set_pen_color(entry, pal5bit(r), pal5bit(g), pal5bit(b));
}

void hudson_poems_state::palette_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_palram[offset]);
	set_palette_val(offset);
}

void hudson_poems_state::fade_w(offs_t offset, u32 data, u32 mem_mask)
{
	logerror("%s: fade_w %08x %08x\n", machine().describe_context(), data, mem_mask);
}

void hudson_poems_state::unktable_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (mem_mask & 0x0000ffff)
	{
		if (m_unktableoffset < 256)
		{
			m_unktable[m_unktableoffset] = data & 0x0000ffff;
			set_palette_val(m_unktableoffset);
			m_unktableoffset++;
		}
	}

	if (mem_mask & 0xffff0000)
	{
		if (m_unktableoffset < 256)
		{
			m_unktable[m_unktableoffset] = (data & 0xffff0000)>>16;
			set_palette_val(m_unktableoffset);
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
	//  this needs to change in RAM, presumably from an interrupt, but no idea how to get there
//  m_maincpu->space(AS_PROGRAM).write_byte(0x2c01d92c, 0x01);
	m_maincpu->irq_request_hack(0x4);
}


void hudson_poems_state::mainram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_mainram[offset]);

	m_gfxdecode->gfx(2)->mark_dirty(offset / 8);
	m_gfxdecode->gfx(3)->mark_dirty(offset / 16);
	m_gfxdecode->gfx(4)->mark_dirty(offset / 4);
	m_gfxdecode->gfx(5)->mark_dirty(offset / 2);
}



void hudson_poems_state::mem_map(address_map &map)
{
	map(0x00000000, 0x007fffff).mirror(0x20000000).rom().region("maincpu", 0);

	/////////////////// unknown
	map(0x04000040, 0x04000043).r(FUNC(hudson_poems_state::poems_random_r));
	map(0x04000140, 0x04000143).r(FUNC(hudson_poems_state::poems_random_r));
	map(0x04000240, 0x04000243).r(FUNC(hudson_poems_state::poems_random_r));
	map(0x04000340, 0x04000343).r(FUNC(hudson_poems_state::poems_random_r));

	//map(0x04000324, 0x04000327).nopw(); // uploads a table here from ROM after uploading fixed values from ROM to some other addresses
	map(0x0400033c, 0x0400033f).w(FUNC(hudson_poems_state::unk_trigger_w)); // maybe DMA?
	map(0x04001000, 0x04001003).r(FUNC(hudson_poems_state::poems_random_r));


	map(0x0400100c, 0x0400100f).nopr(); // read in various places at end of calls, every frame, but result seems to go unused?
	map(0x04002040, 0x04002043).portr("IN1");

	/////////////////// palette / regs?

	map(0x08000038, 0x0800003b).r(FUNC(hudson_poems_state::poems_8000038_r));
	//map(0x0800003c, 0x0800003f).nopw(); // used close to the fade write code, writes FFFFFFFF

	//map(0x08000048, 0x0800004b).nopw(); // ^^
	//map(0x0800004c, 0x0800004f).nopw(); // ^^
	//map(0x08000050, 0x08000053).nopw(); // ^^ 16-bit write, sometimes writes 00000101 & 0000FFFF
	//map(0x08000054, 0x08000057).nopw(); // ^^ writes 15555555 while fading
	map(0x0800005c, 0x0800005f).w(FUNC(hudson_poems_state::fade_w));

	// are these runtime registers, or DMA sources?
	map(0x08000070, 0x0800007f).w(FUNC(hudson_poems_state::spritegfx_base_w)); // ^^ sometimes writes 2C009C00 (one of the tile data bases)

	map(0x08000080, 0x08000083).w(FUNC(hudson_poems_state::tilemap_cfg_w)); // also a similar time to palette uploads
	map(0x08000084, 0x08000087).w(FUNC(hudson_poems_state::tilemap_base_w)); 
	map(0x08000088, 0x0800008b).w(FUNC(hudson_poems_state::tilemap_unk_w));
	//map(0x0800008c, 0x0800008f).nopw(); 
	//map(0x08000090, 0x08000093).nopw();
	//map(0x08000094, 0x08000097).nopw();
	//map(0x08000098, 0x0800009b).nopw();
	map(0x0800009c, 0x0800009f).w(FUNC(hudson_poems_state::tilemap_scr_w));
	//map(0x080000a0, 0x080000a3).nopw();
	//map(0x080000a4, 0x080000a7).nopw();
	//map(0x080000a8, 0x080000ab).nopw();
	//map(0x080000ac, 0x080000af).nopw();

	//map(0x08000180, 0x08000183).nopw(); // gets set to 0000007F on startup
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

const gfx_layout gfx_8x8x2 =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0,1 },
	{ STEP8(0,2) },
	{ STEP8(0,16) },
	8*16
};

static GFXDECODE_START( gfx_poems )
	GFXDECODE_ENTRY( "maincpu", 0, gfx_8x8x4_packed_lsb, 0, 16 )
	GFXDECODE_ENTRY( "maincpu", 0, gfx_8x8x8_raw, 0, 1 )

	GFXDECODE_RAM( "mainram", 0, gfx_8x8x4_packed_lsb, 0, 32 )
	GFXDECODE_RAM( "mainram", 0, gfx_8x8x8_raw, 0, 2 )
	GFXDECODE_RAM( "mainram", 0, gfx_8x8x2, 0, 64 )
	GFXDECODE_RAM( "mainram", 0, gfx_8x8x1, 0, 128 )
GFXDECODE_END

TIMER_DEVICE_CALLBACK_MEMBER(hudson_poems_state::screen_scanline)
{
	int scanline = param;

	if (scanline == 100)
	{
		if (m_tilemapunk != 1) // unlikely, this is probably just tilemap size, with 1 being 256 and 2 being 512
			m_maincpu->irq_request_hack(0x10);
	}

	if (scanline == 200)
	{
		//m_maincpu->irq_request_hack(0x2);
	}

	if (scanline == 150)
	{
		m_maincpu->irq_off_hack();
	}
}

void hudson_poems_state::hudson_poems(machine_config &config)
{
	// 27Mhz XTAL, Xtensa based CPU
	XTENSA(config, m_maincpu, 27_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &hudson_poems_state::mem_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(320, 240); // resolution not confirmed
	screen.set_visarea(0, 320-1, 0, 240-1);
	screen.set_screen_update(FUNC(hudson_poems_state::screen_update));
	screen.set_palette(m_palette);

	TIMER(config, "scantimer").configure_scanline(FUNC(hudson_poems_state::screen_scanline), "screen", 0, 1);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_poems);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x400);

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

} // anonymous namespace


CONS( 2005, marimba,      0,       0,      hudson_poems, hudson_poems, hudson_poems_state, init_marimba, "Konami", "Marimba Tengoku (Japan)", MACHINE_IS_SKELETON )
