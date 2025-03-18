// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

The lexitvsprt contains "W55V9x" strings, which appear to identify the tech used here as the following
Nuvoton product
http://e-tech.com.hk/pd_nuvo_at_game.html


---

Track & Field Challenge TV Game
https://www.youtube.com/watch?v=wjn1lLylqog

Uses epoxy blobs for CPU etc.
These have been identified as Winbond 2005 BA5962 (large glob) + Winbond 200506 BA5934 (smaller glob)
seems to be G65816 derived with custom vectors?

PCB               Game
TV0001 R1.1       My First DDR
TV0002 R1.0       Track & Field

DDR & TF PCBs look identical, all the parts are in the same place, the traces are the same, and the silkscreened part # for resistors and caps are the same.

Some of m_unkregs must retain value (or return certain things) or RAM containing vectors gets blanked and game crashes.
The G65816 code on these is VERY ugly and difficult to follow, many redundant statements, excessive mode switching, accessing things via pointers to pointers etc.

One of the vectors points to 0x6000, there is nothing mapped there, could it be a small internal ROM (sound related?) or some debug trap for development?


---

4 Player System notes:

Mountain Bike Rally uses scrolling / split (helps confirm the same row skip logic seen in other games when using split)
Turn and Whack (cards) game runs far too quickly (might show us where timer config is)
The Power Game game also appears to run far too quickly
Territory Pursuit uses y-flipped sprites

The code to play/request a sample from ROM is at 0D:FB7B
It is called after pushing 3 words onto the stack containing the sample address

eg.
05:85B6: pea $0000
05:85B9: pea $0005
05:85BC: pea $f8bc
05:85BF: jsl $0dfb7b    --- play sample

Samples appear to be terminated with 0x8000

The game also has some terrible PSG-like music, is this coming from a secondary MCU as there is an additional glob on
each of the units using the tech, and the audio quality varies significantly.

*/

#include "emu.h"

#include "cpu/g65816/g65816.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "machine/timer.h"


namespace {

class trkfldch_state : public driver_device
{
public:
	trkfldch_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_mainram(*this, "mainram"),
		m_spriteram(*this, "spriteram"),
		m_palram(*this, "palram"),
		m_palette(*this, "palette"),
		m_in(*this, "IN%u", 0U)
	{ }

	void trkfldch(machine_config &config);
	void vectors_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	virtual uint8_t unkregs_r(offs_t offset);
	virtual void unkregs_w(offs_t offset, uint8_t data);

private:

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_palram;
	required_device<palette_device> m_palette;
	required_ioport_array<4> m_in;

	void draw_sprites(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int pri);
	void render_text_tile_layer(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, uint16_t base);
	void render_tile_layer(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int which);
	uint32_t screen_update_trkfldch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void trkfldch_map(address_map &map) ATTR_COLD;

	uint8_t read_vector(offs_t offset);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	uint8_t m_which_vector = 0;

	uint8_t tilemap_scroll_window_r(int which, uint8_t reg, uint16_t real_base);

	uint8_t tmap0_scroll_window_r(offs_t offset);
	uint8_t tmap1_scroll_window_r(offs_t offset);

	void tilemap_scroll_window_w(int which, uint8_t reg, uint8_t data, uint16_t real_base);

	void tmap0_scroll_window_w(offs_t offset, uint8_t data);
	void tmap1_scroll_window_w(offs_t offset, uint8_t data);

	uint8_t m_dmaregs[0xe]{};

	uint8_t dmaregs_r(offs_t offset);
	void dmaregs_w(offs_t offset, uint8_t data);

	uint8_t m_modebank[0xb]{};

	uint8_t modebankregs_r(offs_t offset);
	void modebankregs_w(offs_t offset, uint8_t data);

	uint8_t m_tilemapbase[0x3]{};

	uint8_t tilemapbase_r(offs_t offset);
	void tilemapbase_w(offs_t offset, uint8_t data);

	uint8_t m_sysregs[0x10]{};

	uint8_t sysregs_r(offs_t offset);
	void sysregs_w(offs_t offset, uint8_t data);

	uint8_t m_unkregs[0x90]{};

	uint8_t m_tmapscroll_window[2][0x12]{};

	uint8_t m_unkdata[0x100000]{};
	int m_unkdata_addr = 0;

};

class trkfldch_lexi_state : public trkfldch_state
{
public:
	trkfldch_lexi_state(const machine_config &mconfig, device_type type, const char *tag) :
		trkfldch_state(mconfig, type, tag),
		m_extra(*this, "EXTRA")
	{ }


protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t unkregs_r(offs_t offset) override;
	void unkregs_w(offs_t offset, uint8_t data) override;

private:
	required_ioport m_extra;
	uint8_t m_input_bit;
	uint8_t m_iopos;
};



void trkfldch_state::video_start()
{
}

void trkfldch_state::render_tile_layer(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int which)
{
//  tilemap 0 = trkfld events, my1stddr background

//  tilemap 1=  my1stddr HUD layer


// first group of tile scroll registers starting at 7820

//  none of these written by hammer throw in trkfld or ddr
//  uint8_t unk20 = m_tmapscroll_window[0][0x00]; // 0f - 100 meters  00 - javelin, long jump, triple  00 after events, 1e high jump  -- hurdle holes -- ddr    (assume default of 00?)
//  uint8_t unk21 = m_tmapscroll_window[0][0x01]; // 1f - 100 meters  00 - javelin, long jump, triple  00 after events, 00 high jump  00 hurdle holes -- ddr    (assume default of 00?)
//  uint8_t unk22 = m_tmapscroll_window[0][0x02]; // 1f - 100 meters  1e - javelin, long jump, triple  1e after events, 1e high jump  1e hurdle holes -- ddr    (assume default of 1e?)
//  uint8_t unk23 = m_tmapscroll_window[0][0x03]; // -- - 100 meters, 1e - javelin, long jump, triple  -- after events, -- high jump  -- hurdle holes -- ddr    (assume default of 1e?)
//  uint8_t unk24 = m_tmapscroll_window[0][0x04]; //                                                   00 after events                00 hurdle holes -- ddr    (assume default of 00?)
//  uint8_t unk25 = m_tmapscroll_window[0][0x05]; //                                                   28 after events                14 hurdle holes -- ddr    (assume default of 28?)

//  uint16_t xscroll = (m_tmapscroll_window[0][0x06] << 0) | (m_tmapscroll_window[0][0x07] << 8); // trkfld tilemap 0, race top            //   why do the split screen races use a different set of scroll registers? maybe global (applies to both layers?) as only one is enabled at this point.
//  uint16_t xscroll = (m_tmapscroll_window[0][0x08] << 0) | (m_tmapscroll_window[0][0x09] << 8); // trkfld tilemap 0, race bot (window?)
//  uint16_t xscroll = (m_tmapscroll_window[0][0x0a] << 0) | (m_tmapscroll_window[0][0x0b] << 8); // trkfld tilemap 0?, javelin
//  0c,0d never written
//  0e,0f never written
//  uint16_t yscroll = (m_tmapscroll_window[0][0x10] << 0) | (m_tmapscroll_window[0][0x11] << 8); // trkfld tilemap 0, javelin, holes, hammer throw

// second group of tile scroll registers starting at 7832

//  32 never written                                                                                                                                        (assume default of 00? to fit 20-25 pattern?)
//  uint8_t windowtop    = m_tmapscroll_window[1][0x01]; // 0x00 - on trkfld hurdle the holes (also resets to 0x00 &  after event - possible default values)             (assume default of 00?)
//  uint8_t windowbottom = m_tmapscroll_window[1][0x02]; // 0x1e   ^                                          0x1e                                                       (assume default of 1e?)
//  uint8_t unk35        = m_tmapscroll_window[1][0x03];  set to 1e (30) (30*8=240) by trkfld high jump otherwise uninitialized always - something to do with bottom     (assume default of 1e?)
//  for left / right on tilemap 1
//  uint8_t windowleft   = m_tmapscroll_window[1][0x04];  //0x29  when unused on my1stddr, 0x25 when used   14 when used on hurdle holes, resets to 00 after event       (assume default of 00?)
//  uint8_t windowright  = m_tmapscroll_window[1][0x05];  //0x29                           0x29             28 when used on hurdle holes, resets to 28 after event       (assume default of 28?)

//  06,076never written
//  uint16_t xscroll = (m_tmapscroll_window[1][0x08] << 0) | (m_tmapscroll_window[1][0x09] << 8); // trkfld tilemap 1?, javelin
//  0a,0b never written
//  0c,0d never written
//  0e,0f never written
//  uint16_t yscroll = (m_tmapscroll_window[1][0x10] << 0) | (m_tmapscroll_window[1][0x11] << 8); // my1stddr tilemap 1 scroller, trkfld tilemap 1 holes (both window)

//  printf("window %02x %02x  %02x %02x  %02x %02x\n", unk20, unk21, unk22, unk23, unk24, unk25);
//  printf("xscroll %04x\n", xscroll);
//  printf("yscroll %04x\n", yscroll);
//  printf("window left/right %02x %02x\n", windowleft, windowright);
//  printf("window top/bottom %02x %02x\n", windowtop, windowbottom);


	int base, gfxbase, gfxregion;
	int tilexsize = 8;

	if (which == 0)
	{
		base = (m_tilemapbase[0x00] << 8);
		gfxbase = (m_modebank[0x01] * 0x2000);
	}
	else //if (which == 1)
	{
		base = (m_tilemapbase[0x01] << 8);
		gfxbase = (m_modebank[0x03] * 0x2000);
	}

	if (m_modebank[0x00] & 1) // seems like it might be a global control for bpp?
	{
		gfxbase -= 0x200;
		gfxregion = 0;
	}
	else
	{
		gfxbase -= 0x2ac;
		gfxregion = 2;
	}



	for (int y = 0; y < 30; y++)
	{
		for (int x = 0; x < 41; x++)
		{
			// fppt tttt   tttt tttt

			rectangle clip;
			clip.set(x*8, (x*8)+7, y*8, (y*8)+7);

			clip &= cliprect;


			address_space &mem = m_maincpu->space(AS_PROGRAM);

			int tile_address = (y * 41) + x;

			uint8_t byte = mem.read_byte(base+((tile_address * 2)));
			uint8_t attr = mem.read_byte(base+((tile_address * 2)+1));

			int tile = (attr << 8) | byte;

			int flipx = (tile & 0x8000) >> 15;
			int pal =   (tile & 0x6000) >> 13;

			tile &= 0x1fff;

			tile += gfxbase;

			gfx_element* gfx = m_gfxdecode->gfx(gfxregion);

			gfx->transpen(bitmap, clip,tile, pal, flipx, 0, x*tilexsize, y*8, 0);

		}
	}
}

void trkfldch_state::render_text_tile_layer(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, uint16_t base)
{
	// this isn't correct, it doesn't seem like a 'real' tilemap, maybe some kind of sprite / tile hybrid, or something with end of line markers?
	// it is needed for the 'good' 'perfect' 'miss' text on DDR ingame
	if (0)
	{
		// guess, but it fits with where the other tilegfxbase registers are, and is only written on my1stddr when this layer is enabled
		int tilegfxbase = (m_modebank[0x07] * 0x80) - 0x100;

		int offs = 0;
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 32; x++)
			{
				address_space& mem = m_maincpu->space(AS_PROGRAM);

				uint8_t byte = mem.read_byte(base + offs);
				offs++;

				int tile = tilegfxbase | byte;

				gfx_element* gfx = m_gfxdecode->gfx(4);

				gfx->transpen(bitmap, cliprect, tile, 0, 0, 0, x * 8, y * 16, 0);

			}
		}
	}
}



// regs                        11 13 15 17
//
// DDR Title = 5000            03 03 05 00
// DDR Ingame Girl 1 = 7000    04 05 06 80
// DDR Music Select  = 9000    05 03 04 00
// trkfldch logos    = b000    06 07 06 00
// trkfldch ingame uses different bpp so different addressing here too?


void trkfldch_state::draw_sprites(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int pri)
{
	for (int i = 0x500 - 5; i >= 0; i -= 5)
	{
		int priority = (m_spriteram[i + 4] & 0x20)>>5;

		// list is NOT drawn in order, but instead z sorted, see shadows in trkfldch
		if (priority != pri)
			continue;

		// logerror("entry %02x %02x %02x %02x %02x\n", m_spriteram[i + 0], m_spriteram[i + 1], m_spriteram[i + 2], m_spriteram[i + 3], m_spriteram[i + 4]);
		int tilegfxbase = (m_modebank[0x05] * 0x800);

		/* m_spriteram[i + 0]  --pp tt-y
		   m_spriteram[i + 1]  yyyy yyyy
		   m_spriteram[i + 2]  tttt tttt
		   m_spriteram[i + 3]  xxxx xxxx
		   m_spriteram[i + 4]  --zfF-tdx

		   p = palette bits
		   t = tile bits
		   y = y pos bits
		   x = x pos bits
		   z = priority
		   fF = x/y flip
		   d = pixel double
		*/

		int y = m_spriteram[i + 1];
		int x = m_spriteram[i + 3];
		int tile = m_spriteram[i + 2];

		int doublesize = m_spriteram[i + 4] & 0x02;

		int tilehigh = m_spriteram[i + 4] & 0x04;
		int tilehigh2 = m_spriteram[i + 0] & 0x04;
		int tilehigh3 = m_spriteram[i + 0] & 0x08;

		int pal = 0;

		int flipx = m_spriteram[i + 4] & 0x10;
		int flipy = m_spriteram[i + 4] & 0x08;

		if (tilehigh)
			tile += 0x100;

		if (tilehigh2)
			tile += 0x200;

		if (tilehigh3)
			tile += 0x400;

		int xhigh = m_spriteram[i + 4] & 0x01;
		int yhigh = m_spriteram[i + 0] & 0x01; // or enable bit?

		x = x | (xhigh << 8);
		y = y | (yhigh << 8);

		y -= 0x100;
		y -= 16;
		x -= 16;

		gfx_element* gfx;

		if (m_modebank[0x00] & 1) // seems like it might be a global control for bpp?
		{
			gfx = m_gfxdecode->gfx(1);
			tilegfxbase -= 0x80;
		}
		else
		{
			pal = (m_spriteram[i + 0] & 0x30)>>4;
			gfx = m_gfxdecode->gfx(3);
			tilegfxbase -= 0x40;
			tilegfxbase -= 0x6b;

		}

		gfx->zoom_transpen(bitmap, cliprect, tile + tilegfxbase, pal, flipx, flipy, x, y, doublesize ? 0x20000 : 0x10000, doublesize ? 0x20000 : 0x10000, 0);
	}
}

uint32_t trkfldch_state::screen_update_trkfldch(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	bitmap.fill(0, cliprect);

	// 3 lots of 0x100 values, but maybe not rgb, seems to be yuv (borrowed from tetrisp2.cpp)
	for (int i = 0; i < 256; i++)
	{
		uint8_t u =  m_palram[0x000 + i] & 0xff;
		uint8_t y1 = m_palram[0x100 + i] & 0xff;
		uint8_t v =  m_palram[0x200 + i] & 0xff;
		double bf = y1+1.772*(u - 128);
		double gf = y1-0.334*(u - 128) - 0.714 * (v - 128);
		double rf = y1+1.772*(v - 128);
		// clamp to 0-255 range
		rf = std::min(rf,255.0);
		rf = std::max(rf,0.0);
		gf = std::min(gf,255.0);
		gf = std::max(gf,0.0);
		bf = std::min(bf,255.0);
		bf = std::max(bf,0.0);

		uint8_t r = (uint8_t)rf;
		uint8_t g = (uint8_t)gf;
		uint8_t b = (uint8_t)bf;

		m_palette->set_pen_color(i, r, g, b);
	}

	if (1) // one of the m_modebank[0x00] bits almost certainly would enable / disable this
	{
		render_tile_layer(screen, bitmap, cliprect, 0);
	}

	draw_sprites(screen, bitmap, cliprect, 0);

	if (m_modebank[0x00] & 0x10) // definitely looks like layer enable
	{
		render_tile_layer(screen, bitmap, cliprect, 1);
	}

	draw_sprites(screen, bitmap, cliprect, 1);

	if (m_modebank[0x00] & 0x20) // this layer is buggy and not currently drawn
	{
		int base = (m_tilemapbase[0x02] << 8);
		render_text_tile_layer(screen, bitmap, cliprect, base);
	}

	return 0;
}

uint8_t trkfldch_state::tilemap_scroll_window_r(int which, uint8_t reg, uint16_t real_base)
{
	uint8_t ret = m_tmapscroll_window[which][reg];
	logerror("%s: tilemap_scroll_window_r (tilemap %d) reg: %02x (returning %02x) (real address %04x)\n", machine().describe_context(), which, reg, ret, real_base + reg);
	return ret;
}

uint8_t trkfldch_state::tmap0_scroll_window_r(offs_t offset)
{
	uint8_t ret = tilemap_scroll_window_r(0, offset, 0x7820);
	return ret;
}

uint8_t trkfldch_state::tmap1_scroll_window_r(offs_t offset)
{
	uint8_t ret = tilemap_scroll_window_r(1, offset, 0x7832);
	return ret;
}

void trkfldch_state::tilemap_scroll_window_w(int which, uint8_t reg, uint8_t data, uint16_t real_base)
{
	m_tmapscroll_window[which][reg] = data;

	switch (reg)
	{
	case 0x00:
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (window top reg0?) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;

	case 0x01:
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (window top reg1?) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;

	case 0x02:
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (window bottom reg0?) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;

	case 0x03:
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (window bottom reg1?) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;

	case 0x04:
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (window left reg) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;

	case 0x05:
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (window right reg) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;

	case 0x06:
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (non-window X scroll low) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;

	case 0x07:
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (non-window X scroll high) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;

	case 0x08:
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (window X scroll low) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;

	case 0x09:
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (window X scroll high) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;

	case 0x0a:
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (another X scroll low) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;

	case 0x0b:
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (another X scroll high) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;

	case 0x0c:
		// unused, probably unknown Y scroll low
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (unknown) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;

	case 0x0d:
		// unused, probably unknown Y scroll low
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (unknown) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;

	case 0x0e:
		// unused, probably unknown Y scroll low
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (unknown) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;

	case 0x0f:
		// unused, probably unknown Y scroll low
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (unknown) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;

	case 0x10:
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (window Y scroll low) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;

	case 0x11:
		logerror("%s: tilemap_scroll_window_w (tilemap %d) reg: %02x (window Y scroll high) (data %02x) (real address %04x)\n", machine().describe_context(), which, reg, data, real_base + reg);
		break;
	}
}

void trkfldch_state::tmap0_scroll_window_w(offs_t offset, uint8_t data)
{
	tilemap_scroll_window_w(0, offset, data, 0x7820);
}

void trkfldch_state::tmap1_scroll_window_w(offs_t offset, uint8_t data)
{
	tilemap_scroll_window_w(1, offset, data, 0x7832);
}




uint8_t trkfldch_state::dmaregs_r(offs_t offset)
{
	uint8_t ret = m_dmaregs[offset];

	switch (offset)
	{
	case 0x05: // abl4play polls this expecting it to be 0 to continue (probably becomes after DMA is complete, or can show the status in realtime?)
		ret = 0x00;
		break;

	case 0x06: // abl4play polls this expecting it to be 0 to continue (probably becomes after DMA is complete, or can show the status in realtime?)
		ret = 0x00;
		break;
	}

	logerror("%s: dmaregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
	return ret;
}

void trkfldch_state::dmaregs_w(offs_t offset, uint8_t data)
{
	m_dmaregs[offset] = data;

	switch (offset)
	{

	case 0x00: // sprite list location (dma source?)
		logerror("%s: dmaregs_w %04x %02x (dma source low )\n", machine().describe_context(), offset, data);
		break;

	case 0x01: // sprite list location (dma source?)
		logerror("%s: dmaregs_w %04x %02x (dma source med )\n", machine().describe_context(), offset, data);
		break;

	case 0x02:
		logerror("%s: dmaregs_w %04x %02x (dma source high)\n", machine().describe_context(), offset, data);
		break;

	case 0x03:
		logerror("%s: dmaregs_w %04x %02x (dma dest low )\n", machine().describe_context(), offset, data);
		break;

	case 0x04:
		logerror("%s: dmaregs_w %04x %02x (dma dest high)\n", machine().describe_context(), offset, data);
		break;

	case 0x05:
		logerror("%s: dmaregs_w %04x %02x (dma length low ) (and trigger)\n", machine().describe_context(), offset, data);
		{
			address_space& mem = m_maincpu->space(AS_PROGRAM);
			uint16_t dmalength = (m_dmaregs[0x06] << 8) | m_dmaregs[0x05];
			uint32_t dmasource = (m_dmaregs[0x02] << 16) | (m_dmaregs[0x01] << 8) | m_dmaregs[0x00];
			uint16_t dmadest = (m_dmaregs[0x04] << 8) | m_dmaregs[0x03];

			//if (dmadest != 0x6800)
			logerror("%s: performing dma src: %06x dst %04x len %04x and extra params %02x %02x %02x %02x %02x %02x\n", machine().describe_context(), dmasource, dmadest, dmalength, m_dmaregs[0x07], m_dmaregs[0x08], m_dmaregs[0x09], m_dmaregs[0x0b], m_dmaregs[0x0c], m_dmaregs[0x0d]);

			int writeoffset = 0;
			int writedo = m_dmaregs[0x0d];

			int readoffset = 0;
			int readdo = m_dmaregs[0x09];

			for (uint32_t j = 0; j < dmalength; j++)
			{
				uint8_t byte = mem.read_byte(dmasource + readoffset);
				readdo--;
				if (readdo < 0)
				{
					readdo = m_dmaregs[0x09];
					readoffset += m_dmaregs[0x07] | (m_dmaregs[0x08] << 8);
				}
				else
				{
					readoffset++;
				}

				mem.write_byte(dmadest + writeoffset, byte);
				writedo--;
				if (writedo < 0)
				{
					writedo = m_dmaregs[0x0d];
					writeoffset += m_dmaregs[0xb] | (m_dmaregs[0x0c] << 8);
				}
				else
				{
					writeoffset++;
				}

			}
		}
		break;

	case 0x06:
		logerror("%s: dmaregs_w %04x %02x (dma length high)\n", machine().describe_context(), offset, data);
		break;

	case 0x07: // after a long time
		logerror("%s: dmaregs_w %04x %02x (dma source read skip size)\n", machine().describe_context(), offset, data);
		break;

	case 0x08: // rarely (my1stddr)
		logerror("%s: dmaregs_w %04x %02x (dma source unknown)\n", machine().describe_context(), offset, data);
		break;

	case 0x09: // after a long time
		logerror("%s: dmaregs_w %04x %02x (dma source read group size)\n", machine().describe_context(), offset, data);
		break;

	case 0x0a: // unused?
		logerror("%s: dmaregs_w %04x %02x (dma unused)\n", machine().describe_context(), offset, data);
		break;

	case 0x0b: // after a long time
		logerror("%s: dmaregs_w %04x %02x (dma dest write skip size)\n", machine().describe_context(), offset, data);
		break;

	case 0x0c: // rarely (my1stddr)
		logerror("%s: dmaregs_w %04x %02x (dma dest unknown)\n", machine().describe_context(), offset, data);
		break;

	case 0x0d: // after a long time
		logerror("%s: dmaregs_w %04x %02x (dma dest write group size)\n", machine().describe_context(), offset, data);
		break;

	}
}


uint8_t trkfldch_state::modebankregs_r(offs_t offset)
{
	uint8_t ret = m_modebank[offset];
	logerror("%s: modebankregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
	return ret;
}

void trkfldch_state::modebankregs_w(offs_t offset, uint8_t data)
{
	m_modebank[offset] = data;

	switch (offset)
	{
	case 0x00: // gfxmode select (4bpp / 8bpp) and layer enables
		logerror("%s: unkregs_w (enable, bpp select) %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x01: // tilegfxbank 1
		logerror("%s: unkregs_w %04x %02x (tilegfxbank 1)\n", machine().describe_context(), offset, data);
		break;

	case 0x02: // 00 - startup (probably more tilegfxbank 1 bits)
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x03: // tilegfxbank 2
		logerror("%s: unkregs_w %04x %02x (tilegfxbank 2)\n", machine().describe_context(), offset, data);
		break;

	case 0x04: // 00 - startup (probably more tilegfxbank 2 bits)
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x05: // spritegfxbank
		logerror("%s: unkregs_w  %04x %02x (spritegfxbank)\n", machine().describe_context(), offset, data);
		break;

	case 0x06: // 00 (probably more spritegfxbank bits)
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x07: // gfxbank for weird layer
		logerror("%s: unkregs_w %04x %02x (weird gfx bank)\n", machine().describe_context(), offset, data);
		break;

	case 0x08: // more gfxbank for weird layer?
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x09:  // unknowns? another unknown layer?
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x0a:  // unknowns? another unknown layer?
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;
	}
}


uint8_t trkfldch_state::tilemapbase_r(offs_t offset)
{
	uint8_t ret = m_tilemapbase[offset];
	logerror("%s: tilemapbase_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
	return ret;
}

void trkfldch_state::tilemapbase_w(offs_t offset, uint8_t data)
{
	m_tilemapbase[offset] = data;
	logerror("%s: tilemapbase_w %04x %02x (tilebase %d)\n", machine().describe_context(), offset, data, offset);
}


void trkfldch_state::trkfldch_map(address_map &map)
{
	map(0x000000, 0x003fff).ram().share("mainram");

	map(0x006800, 0x006cff).ram().share("spriteram");

	map(0x007000, 0x0072ff).ram().share("palram");

	// 7800 - 78xx look like registers?

	map(0x007800, 0x00780f).rw(FUNC(trkfldch_state::sysregs_r), FUNC(trkfldch_state::sysregs_w));


	map(0x007810, 0x00781a).rw(FUNC(trkfldch_state::modebankregs_r), FUNC(trkfldch_state::modebankregs_w));

	map(0x007820, 0x007831).rw(FUNC(trkfldch_state::tmap0_scroll_window_r), FUNC(trkfldch_state::tmap0_scroll_window_w));
	map(0x007832, 0x007843).rw(FUNC(trkfldch_state::tmap1_scroll_window_r), FUNC(trkfldch_state::tmap1_scroll_window_w));

	map(0x007854, 0x007856).rw(FUNC(trkfldch_state::tilemapbase_r), FUNC(trkfldch_state::tilemapbase_w));

	map(0x007860, 0x00786d).rw(FUNC(trkfldch_state::dmaregs_r), FUNC(trkfldch_state::dmaregs_w));

	map(0x007870, 0x0078ff).rw(FUNC(trkfldch_state::unkregs_r), FUNC(trkfldch_state::unkregs_w));

	map(0x008000, 0xffffff).rom().region("maincpu", 0x000000); // good for code mapped at 008000 and 050000 at least
}

void trkfldch_state::vectors_map(address_map &map)
{
	map(0x00, 0x1f).r(FUNC(trkfldch_state::read_vector));
}

uint8_t trkfldch_state::read_vector(offs_t offset)
{
	uint8_t *rom = memregion("maincpu")->base();

	/* what appears to be a table of vectors appears at the START of ROM, maybe this gets copied to RAM, maybe used directly?
	00 : (invalid)
	02 : (invalid)
	04 : 0xA2C6  (dummy)
	06 : 0xA334  (real function - vbl?)
	08 : 0xA300  (dummy)
	0a : 0xA2E0  (dummy)
	0c : 0xA2B9  (dummy)
	0e : 0xA2ED  (dummy)
	10 : 0xA2D3  (dummy)
	12 : 0xA327  (dummy)
	14 : 0xA30D  (real function) (dummy in trkfld?) (controls arrow speed in ddr?)
	16 : 0x6000  (points at ram? or some internal ROM? we have nothing mapped here, not cleared as RAM either)
	18 : 0xA31A  (dummy) (not dummy in trkfld? - timer interrupt?)
	1a : 0xA2AC  (dummy)
	1c : 0xA341  (boot vector)
	1e : (invalid)
	*/

	logerror("reading vector offset %02x\n", offset);

	if (offset == 0x0b)
	{   // NMI
		return rom[m_which_vector+1];
	}
	else if (offset == 0x0a)
	{   // NMI
		return rom[m_which_vector];
	}

	// boot vector
	return rom[offset];
}


TIMER_DEVICE_CALLBACK_MEMBER(trkfldch_state::scanline)
{
	int scanline = param;

	if (scanline == 200)
	{
		m_which_vector = 0x06;
		m_maincpu->set_input_line(G65816_LINE_NMI, ASSERT_LINE);
	}
	else if (scanline == 201)
	{
		m_which_vector = 0x06;
		m_maincpu->set_input_line(G65816_LINE_NMI, CLEAR_LINE);
	}

	if (scanline == 20)
	{
		m_which_vector = 0x14;
		m_maincpu->set_input_line(G65816_LINE_NMI, ASSERT_LINE);
	}
	else if (scanline == 21)
	{
		m_which_vector = 0x14;
		m_maincpu->set_input_line(G65816_LINE_NMI, CLEAR_LINE);
	}

	// this is clearly a timer interrupt, trkfldch needs it to count
	if ((scanline >= 80) && (scanline < 120))
	{
		if ((scanline & 1) == 0)
		{
			m_which_vector = 0x18;
			m_maincpu->set_input_line(G65816_LINE_NMI, ASSERT_LINE);
		}
		else
		{
			m_which_vector = 0x18;
			m_maincpu->set_input_line(G65816_LINE_NMI, CLEAR_LINE);
		}
	}
}

static INPUT_PORTS_START( trkfldch )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_START("IN3")
INPUT_PORTS_END

static INPUT_PORTS_START( konsb )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_START("IN3")
INPUT_PORTS_END

static INPUT_PORTS_START( my1stddr )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("O") // selects / forward
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY // directions correct based on 'letters' minigame
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("X") // goes back
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_START("IN3")
INPUT_PORTS_END

static INPUT_PORTS_START( abl4play )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Select")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( shtscore )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Select")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Kick")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( lexi )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRA")
	PORT_BIT( 0x00008, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x00020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x00080, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x00200, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x00800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x08000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x20000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
INPUT_PORTS_END

static const gfx_layout tiles8x8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 48,49, 32,33, 16,17, 0, 1 },
	{ 8,10,12, 14, 0,2,4,6  },
	{ STEP8(0,64) },
	512,
};

static const gfx_layout tiles8x16x8_layout =
{
	8,16,
	RGN_FRAC(1,1),
	8,
	{ 48,49, 32,33, 16,17, 0, 1 },
	{ 8,10,12, 14, 0,2,4,6  },
	{ STEP16(0,64) },
	1024,
};


static const gfx_layout tiles16x16x8_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 96, 97, 64, 65, 32, 33, 0, 1  },
	{ 8,10,12,14, 0,2,4,6, 24,26,28,30, 16,18,20,22 },
	{ STEP16(0,128) },
	128*16,
};


// TODO: if we're going to use gfxdecode then allocate this manually with the correct number of tiles
//  might be we have to use manual drawing tho, the base offset is already strange
static const gfx_layout tiles8x8x6_layout =
{
	8,8,
	0x5500*4,
	6,
	{ 32, 33, 16, 17, 0, 1  },
	{ 8,10,12,14, 0,2,4,6 },
	{ STEP8(0,48) },
	48*8,
};


static const gfx_layout tiles16x16x6_layout =
{
	16,16,
	0x5500,
	6,
	{ 64, 65, 32, 33, 0, 1  },
	{ 8,10,12,14, 0,2,4,6, 24,26,28,30, 16,18,20,22 },
	{ STEP16(0,96) },
	96*16,
};


static GFXDECODE_START( gfx_trkfldch )
	GFXDECODE_ENTRY( "maincpu", 0, tiles8x8x8_layout, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0, tiles16x16x8_layout, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0x40, tiles8x8x6_layout, 0, 4 )
	GFXDECODE_ENTRY( "maincpu", 0x40, tiles16x16x6_layout, 0, 4 )
	GFXDECODE_ENTRY( "maincpu", 0, tiles8x16x8_layout, 0, 1 )
GFXDECODE_END

/*

7800 / 7801 seem to be IRQ related

7800 : 0001 - ? (there is no irq 0x00)
       0002 - ? (there is no irq 0x02)
       0004 used in irq 0x04
       0008 used in irq 0x06
       0010 used in irq 0x08
       0020 used in irq 0x0a
       0x40 used in irq 0x0c
       0x80 used in irq 0x0e (and by code accessing other ports in the main execution?!)

7801 : 0001 used in irq 0x10
     : 0002 used in irq 0x12
     : 0004 used in irq 0x14
     : 0008 - ? (there is no irq 0x016, it points to unknown area? and we have no code touching this bit)
     : 0010 used in irq 0x18
     : 0020 used in irq 0x1a and 0x06?! (used with OR instead of EOR in 0x06, force IRQ?)
     : 0x40 - ? (there is no irq 0x1c - it's the boot vector)
     : 0x80 - ? (there is no irq 0x1e)

*/

uint8_t trkfldch_state::sysregs_r(offs_t offset)
{
	uint8_t ret = m_sysregs[offset];

	switch (offset)
	{
	case 0x00: // IRQ status?, see above
		ret = machine().rand();
		logerror("%s: sysregs_r (IRQ state?) %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x01: // IRQ status?, see above
		ret = machine().rand();
		logerror("%s: sysregs_r (IRQ state?) %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x02: // code to write 0x7f here during startup, also code to read port, mask with 0x7f and rewrite (clear top bit, probably gets set by hw?)
		logerror("%s: sysregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x03: // there is code to read, set bit 0x04 and write back out as well as code to clear to 0x00 (no other uses?)
		logerror("%s: sysregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;


	case 0x04: // bit 0x80 is checked and looped on after writes to 78b6 (the 'large amount of data upload' port)
		ret = 0x80;
		logerror("%s: sysregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x05: // bit 0x20 is checked and looped on right after DMA trigger (assuming DMA trigger is correct)
		ret = 0x20;
		logerror("%s: sysregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x06: // no real reads, only side-effect of reading 7805
		logerror("%s: sysregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;
	}

	return ret;
}


void trkfldch_state::sysregs_w(offs_t offset, uint8_t data)
{
	m_sysregs[offset] = data;

	switch (offset)
	{
	case 0x00: // IRQ ack/force?, see above
		logerror("%s: sysregs_w (IRQ ack/force?) %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x01: // IRQ maybe status, see above
		logerror("%s: sysregs_w (IRQ ack/force?) %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x02: // startup
		logerror("%s: sysregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x03: // startup
		logerror("%s: sysregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x04: // startup
		logerror("%s: sysregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x05: // startup
		logerror("%s: sysregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	default:
		logerror("%s: sysregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	}
}



uint8_t trkfldch_state::unkregs_r(offs_t offset)
{
	uint8_t ret = m_unkregs[offset];

	switch (offset)
	{

	case 0x00: // read in irq (inputs?)
		ret = m_in[0]->read();
		logerror("%s: unkregs_r %04x (returning %02x) (Player 1 inputs)\n", machine().describe_context(), offset, ret);
		break;

	case 0x01:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	// 0x02

	case 0x03:
		ret = m_in[1]->read();
		logerror("%s: unkregs_r %04x (returning %02x) (Player 2 inputs)\n", machine().describe_context(), offset, ret);
		break;

	case 0x04:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x05:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x06:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x07:
		ret = m_in[2]->read();
		logerror("%s: unkregs_r %04x (returning %02x) (Player 3 inputs)\n", machine().describe_context(), offset, ret);
		break;

	// 0x08
	// 0x09
	// 0x0a

	case 0x0b:
		ret = m_in[3]->read();
		logerror("%s: unkregs_r %04x (returning %02x) (Player 4 inputs)\n", machine().describe_context(), offset, ret);
		break;


	case 0x0f:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x10: // only read as a side-effect of reading 0x7f?
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x46: // 0x70
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x47:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	default:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;
	}
	return ret;
}



void trkfldch_state::unkregs_w(offs_t offset, uint8_t data)
{
	m_unkregs[offset] = data;

	switch (offset)
	{
	// 7x = I/O area?

	case 0x01: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x02: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x03: // some kind of serial device?
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x04: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x05: // some kind of serial device? (used with 73?)
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x06: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x07: // every second or so
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x08: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x09: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x0a: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x0f: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;


	case 0x11: // startup (my1stddr)
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x12: // startup (my1stddr)
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x13:
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x14:
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;


	case 0x45: // (real address 78b5) startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x46: // significant data transfer shortly after boot, seems to clock writes with 0073 writing  d0 / c0? (then writes 2 bytes here)
			   // is this sending song patterns to another CPU?
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		m_unkdata[m_unkdata_addr] = data;

		m_unkdata_addr++;
		m_unkdata_addr &= 0xfffff;
		break;

	case 0x5a: // (real address 78ca)  startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	default:
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;
	}

}


void trkfldch_lexi_state::machine_start()
{
	trkfldch_state::machine_start();
	save_item(NAME(m_input_bit));
	save_item(NAME(m_iopos));
}

void trkfldch_lexi_state::machine_reset()
{
	trkfldch_state::machine_reset();
	m_input_bit = 0;
	m_iopos = 0;
}

uint8_t trkfldch_lexi_state::unkregs_r(offs_t offset)
{
	switch (offset)
	{

	case 0x00:
	{
		uint8_t ret = m_input_bit;
		logerror("%s: unkregs_r %04x (returning %02x) (Player 1 inputs)\n", machine().describe_context(), offset, ret);
		return ret;
	}

	default:
		return trkfldch_state::unkregs_r(offset);
	}
}


void trkfldch_lexi_state::unkregs_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x03:
		logerror("%s: unkregs_w %04x %02x (io strobe?)\n", machine().describe_context(), offset, data);
		if (data == 0x03)
		{
			m_iopos = 0;
		}
		else
		{
			m_iopos++;
		}

		m_input_bit = (m_extra->read() >> m_iopos) & 1;
		break;

	default:
		trkfldch_state::unkregs_w(offset, data);
		break;
	}
}

void trkfldch_state::machine_start()
{
	save_item(NAME(m_unkdata_addr));
	save_item(NAME(m_unkdata));

	for (int i = 0; i < 256; i++)
	{
		m_palette->set_pen_color(i, machine().rand(), machine().rand(), machine().rand());
	}

}

void trkfldch_state::machine_reset()
{
	m_which_vector = 0x06;

	for (int i = 0; i < 0x90; i++)
		m_unkregs[i] = 0x90;

	for (int i = 0; i < 0x100000; i++)
		m_unkdata[i] = 0;

	for (int j = 0; j < 2; j++)
		for (int i = 0; i < 0x12; i++)
			m_tmapscroll_window[j][i] = 0x00;

	// these don't always get initialized by the code, these appear to be reasonable default values based on what they get reset to by the games after first use
	for (int j = 0; j < 2; j++)
	{
		m_tmapscroll_window[j][0x00] = 0x00;
		m_tmapscroll_window[j][0x01] = 0x00;
		m_tmapscroll_window[j][0x02] = 0x1e;
		m_tmapscroll_window[j][0x03] = 0x1e;
		m_tmapscroll_window[j][0x04] = 0x00;
		m_tmapscroll_window[j][0x05] = 0x28;
	}

	for (int i = 0; i < 0xe; i++)
		m_dmaregs[i] = 0x00;

	for (int i = 0; i < 0xb; i++)
		m_modebank[i] = 0x00;

	for (int i = 0; i < 0x10; i++)
		m_sysregs[i] = 0x00;

	m_tilemapbase[0x00] = 0x00;
	m_tilemapbase[0x01] = 0x00;
	m_tilemapbase[0x02] = 0x00;


	m_unkdata_addr = 0;

	// the game code doesn't set the DMA step / skip params to default values until after it's used them with other values, so assume they reset to these
	m_dmaregs[0x07] = 0x01;
	m_dmaregs[0x08] = 0x00;
	m_dmaregs[0x09] = 0x00;

	m_dmaregs[0x0b] = 0x01;
	m_dmaregs[0x0c] = 0x00;
	m_dmaregs[0x0d] = 0x00;
}

void trkfldch_state::trkfldch(machine_config &config)
{
	/* basic machine hardware */
	G65816(config, m_maincpu, 20000000);
	//m_maincpu->set_addrmap(AS_DATA, &trkfldch_state::mem_map);
	m_maincpu->set_addrmap(AS_PROGRAM, &trkfldch_state::trkfldch_map);
	m_maincpu->set_addrmap(g65816_device::AS_VECTORS, &trkfldch_state::vectors_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(trkfldch_state::scanline), "screen", 0, 1);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(320, 240);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(trkfldch_state::screen_update_trkfldch));
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_trkfldch);

	PALETTE(config, m_palette, palette_device::BLACK, 256);
}

ROM_START( trkfldch )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "trackandfield.bin", 0x000000, 0x400000,  CRC(f4f1959d) SHA1(344dbfe8df1897adf77da6e5ca0435c4d47d6842) )
ROM_END

ROM_START( my1stddr )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "myfirstddr.bin", 0x000000, 0x400000, CRC(2ef57bfc) SHA1(9feea5adb9de8fe17e915f3a037e8ddd70e58ae7) )
ROM_END

ROM_START( abl4play )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "abl4play.bin", 0x000000, 0x800000, CRC(5d57fb70) SHA1(34cdf80dc8cb08e5cd98c724268e4c5f483780d7) )
ROM_END

ROM_START( shtscore )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "shootnscore.bin", 0x000000, 0x400000, CRC(37aa16bd) SHA1(609d0191301480c51ec1188c67101a4e88a5170f) )
ROM_END

ROM_START( lexitvsprt )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "29l3211.u2a", 0x000000, 0x400000, CRC(65e5223c) SHA1(13eae6e34100fb9761335e87a3cf728bb31e860f) )
ROM_END

ROM_START( senspid )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD( "spidermanmat.bin", 0x00000, 0x400000, CRC(11f5181c) SHA1(7f0d5d34f924b6a7182102451a2ac1cc41e575b0) )
ROM_END


/*
Included with Orange model

    しあわせのおうじ (イギリス民話)
        Shiawase no Ouji (English minwa)
    ほしのぎんか (グリム童話)
        Hoshi no Ginka (Grimm douwa)
    おやゆびひめ (アンデルセン)
        Oyayubi-hime (Anderson)
    こびととくつや (グリム童話)
        Kobi to Toku Tsuya (Grimm douwa)
    みっつのねがい (日本昔話)
        Mittsu no Negai (Nippon mukashi banashi)
    うらしまたろう (日本昔話)
        Urashima Tarou (Nippon mukashi banashi)
    アフロンとがまじいの ちきゅうにやさしく (オリジナル)
        Afuron to Gamajii no Chikyuu ni Yasashiku (Original)
    かさこじぞう (日本昔話)
        Kasakojizo (Nippon mukashi banashi)
    きんのがちょう (グリム童話)
        Kin no Gachou (Grimm douwa)
    あそんでまなぼう (オリジナル)
        Asonde Manabou (Original)
*/
ROM_START( teleshi )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "s29gl128n90tfir.bin", 0x000000, 0x1000000, CRC(8c032142) SHA1(7dff151ea1abd5911e753f0708b0b4f66599791f) )
ROM_END

/*
Included with Purple model

    ももたろう (日本昔話)
        Momotarou (Nippon mukashi banashi)
    あかずきん (グリム童話)
        Aka Zukin (Grimm douwa)
    きたかぜのくれたテーブルかけ (ノルウェー民話)
        Kita Kaze no Kureta Table Kake (Norway minwa)
    いっすんぼうし (日本昔話)
        Issun-boushi (Nippon mukashi banashi)
    おおきなかぶ (ロシア民話)
        Ookina Kabu (Russia minwa)
    ありときりぎりす (イソップ童話)
        Ari to Kirigirisu (Aesop douwa)
    マッチうりのしょうじょ (アンデルセン)
        Match Uri no Shoujo (Anderson)
    かさこじぞう (日本昔話)
        Kasa Jizou (Nippon mukashi banashi)
    きんのがちょう (グリム童話)
        Kin no Gachou (Grimm douwa)
    あそんでまなぼう2 (オリジナル)
        Asonde Manabou 2 (Original)
*/
ROM_START( teleship )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "s29gl128n90tfir2.bin", 0x000000, 0x1000000, CRC(28c12a48) SHA1(68557849e2b6f3669de76540de841ca99119ec58) )
ROM_END

/*
The following were also available (via the Download service?)

世界のお話 (ナレーション入り) [Sekai no Ohanashi (Narration-iri)]

    ブレーメンのおんがくたい (グリム童話)
        Bremen no Ongakutai (Grimm douwa)
    みにくいあひるのこ (アンデルセン)
        Minikui Ahiru no Ko (Andersen)
    てぶくろ (ウクライナ民話)
        Tebukuro (Ukraine minwa)
    はちかつぎひめ (日本昔話)
        Hachi Katsugi-hime (Nippon mukashi banashi)

遊んで学ぼう・お歌で遊ぼう [Asonde Manabu - Outa de Asobou]

    あそんでまなぼう (オリジナル)
        Asonde Manabou (Original)
    あそんでまなぼう2 (オリジナル)
        Asonde Manabou 2 (Original)
    おうたであそぼう3 (オリジナル)
        Asonde Manabou 3 (Original)
    おうたであそぼう4 (オリジナル)
        Asonde Manabou 4 (Original)
*/


} // anonymous namespace


CONS( 2007, trkfldch,  0,          0,  trkfldch, trkfldch,trkfldch_state,      empty_init,    "Konami",                                     "Track & Field Challenge", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
CONS( 2006, my1stddr,  0,          0,  trkfldch, my1stddr,trkfldch_state,      empty_init,    "Konami",                                     "My First Dance Dance Revolution (US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Japan version has different songs
CONS( 200?, abl4play,  0,          0,  trkfldch, abl4play,trkfldch_state,      empty_init,    "Advance Bright Ltd",                         "4 Player System - 10 in 1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
CONS( 200?, shtscore,  0,          0,  trkfldch, shtscore,trkfldch_state,      empty_init,    "Halsall / time4toys.com / Electronic Games", "Shoot n' Score", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
CONS( 200?, lexitvsprt,0,          0,  trkfldch, lexi,    trkfldch_lexi_state, empty_init,    "Lexibook",                                   "TV Sports Plug & Play 5-in-1 (JG7000)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
// don't have a picture of the box, title screen doesn't give a more complete title, I/O seems closer lexitvsprt
CONS( 2007, senspid,   0,          0,  trkfldch, trkfldch,trkfldch_state,      empty_init,    "Senario",                                    "The Amazing Spider-Man (Senario, floor mat)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )


// additional online content could be downloaded onto these if they were connected to a PC via USB
CONS( 2008, teleshi,   0,          0,  trkfldch, konsb,   trkfldch_state,      empty_init,    "Konami",                                     "Teleshibai (Japan)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // テレしばい - this one is orange
CONS( 2008, teleship,  0,          0,  trkfldch, konsb,   trkfldch_state,      empty_init,    "Konami",                                     "Teleshibai - Purple Version (Japan)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // テレしばい (パープルバージョン) - this has Purple Version as part of the name  on the box
