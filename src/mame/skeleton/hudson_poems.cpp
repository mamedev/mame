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
		m_gfxdecode(*this, "gfxdecode"),
		m_mainram(*this, "mainram"),
		m_palram(*this, "palram")
	{ }

	void hudson_poems(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void draw_tile(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t tile, int xx, int yy);
	void draw_tile8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t tile, int xx, int yy);

	TIMER_DEVICE_CALLBACK_MEMBER(screen_scanline);

	void mem_map(address_map &map);

	uint32_t poems_rand_r();
	//uint32_t poems_8000038_r();
	//uint32_t poems_8020020_r();
	//uint32_t poems_800aa04_r();
	
	void unk_vregs_w(offs_t offset, u32 data, u32 mem_mask);
	void unktable_w(offs_t offset, u32 data, u32 mem_mask);
	void unktable_reset_w(offs_t offset, u32 data, u32 mem_mask);
	void set_palette_val(int entry);
	void palette_w(offs_t offset, u32 data, u32 mem_mask);

	uint32_t m_unkvregs[0x80/4];
	uint16_t m_unktable[256];
	uint16_t m_unktableoffset;

	required_device<xtensa_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<u32> m_mainram;
	required_shared_ptr<u32> m_palram;
};

void hudson_poems_state::machine_start()
{
	for (int i = 0;i < 0x80/4; i++)
		m_unkvregs[i] = 0x00;

	save_item(NAME(m_unkvregs));
	
}

void hudson_poems_state::machine_reset()
{
	m_maincpu->set_pc(0x00000040);
	m_unktableoffset = 0;
}

static INPUT_PORTS_START( hudson_poems )
INPUT_PORTS_END


void hudson_poems_state::draw_tile(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t tile, int xx, int yy)
{
	int flipx = tile & 0x800;
	int flipy = tile & 0x400;
	tile &= 0x3ff;

	int realoffset = tile * 8;
	int count = 0;
	for (int y = 0; y < 8; y++)
	{
		int realy = y;
		if (flipy)
			realy = 7 - y;

		u16* const dstptr_bitmap = &bitmap.pix(realy + yy);

		uint32_t pix = m_mainram[(0x9c00 / 4) + count + realoffset];
		int pos = xx;

		if (flipx)
		{
			for (int i = 28; i >= 0; i -= 4)
			{
				if (pos < cliprect.max_x) dstptr_bitmap[pos] = (pix >> i) & 0xf;
				pos++;
			}
			count++;
		}
		else
		{
			for (int i = 0; i < 32; i += 4)
			{
				if (pos < cliprect.max_x) dstptr_bitmap[pos] = (pix >> i) & 0xf;
				pos++;
			}
			count++;
		}
	}
}

void hudson_poems_state::draw_tile8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t tile, int xx, int yy)
{
	int offset = tile * 16;
	int count = 0;
	for (int y = 0; y < 8; y++)
	{
		u16* const dstptr_bitmap = &bitmap.pix(y + yy);

		uint64_t pix = ((uint64_t)(m_mainram[(0x9c00 / 4) + count + 1 + offset]) << 32) | (uint64_t)(m_mainram[(0x9c00 / 4) + count + offset]);
		int pos = xx;

		for (int i = 0; i < 64; i += 8)
		{
			if (pos < cliprect.max_x) dstptr_bitmap[pos] = (pix >> i) & 0xff;
			pos++;
		}

		count += 2;
	}
}

uint32_t hudson_poems_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
#if 0
	// just to show that some stuff gets uploaded to mainram if you let execution continue, looks like tilemap or tile data?
	int count = 0;
	for (int y=0;y<224/8;y++)
	{
		for (int x=0;x<256/8;x++)
		{

			count++;
			draw_tile(screen, bitmap, cliprect, count * 8, x * 8, y * 8);
		}
	}
#endif
	int count = 0;
	int width, base, bpp;

//  width = 512; base = (0xa800/4); bpp = 4; // konami logo
//	width = 128; base = (0x9418/4); bpp = 8; // poems logo
	width = 512; base = (0xc600/4); bpp = 4; // bemani logo
//	width = 512; base = (0xd400/4); bpp = 4; // warning screen

	for (int y=0;y<224/8;y++)
	{
		for (int x=0;x<width/8/2;x++)
		{
			//uint32_t tiles = m_mainram[(0xa800/4)+count];
			uint32_t tiles = m_mainram[base+count];

			if (bpp == 4)
			{
				draw_tile(screen, bitmap, cliprect, (tiles&0xffff), (x * 16), y * 8);
				draw_tile(screen, bitmap, cliprect, ((tiles>>16)&0xffff), (x * 16)+8, y * 8);
			}
			else if (bpp == 8)
			{
				draw_tile8(screen, bitmap, cliprect, (tiles&0xffff), (x * 16), y * 8);
				draw_tile8(screen, bitmap, cliprect, ((tiles>>16)&0xffff), (x * 16)+8, y * 8);
			}

			count++;

		}
	}
	return 0;
}



uint32_t hudson_poems_state::poems_rand_r()
{
	return (machine().rand() & 0x1) ? machine().rand() : 0;
}

/*
uint32_t hudson_poems_state::poems_8020020_r()
{
	return 0xffffffff;
}

uint32_t hudson_poems_state::poems_8000038_r()
{
	return (machine().rand() & 1) ? 0x00000000 : 0xffffffff; 
}

uint32_t hudson_poems_state::poems_800aa04_r()
{
	return (machine().rand() & 1) ? 0x00000000 : 0xffffffff; 
}
*/
void hudson_poems_state::unk_vregs_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_unkvregs[offset]);
}

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



void hudson_poems_state::mem_map(address_map &map)
{
	map(0x00000000, 0x007fffff).mirror(0x20000000).rom().region("maincpu", 0);

	map(0x04000000, 0x04003fff).ram();

	map(0x04002040, 0x04002043).r(FUNC(hudson_poems_state::poems_rand_r));

	map(0x08000000, 0x0800007f).w(FUNC(hudson_poems_state::unk_vregs_w));

	map(0x08000038, 0x0800003b).r(FUNC(hudson_poems_state::poems_rand_r));
	map(0x08000800, 0x08000bff).ram().w(FUNC(hudson_poems_state::palette_w)).share("palram");

	map(0x08008200, 0x08008203).r(FUNC(hudson_poems_state::poems_rand_r));
	map(0x08002000, 0x08008fff).ram();
	map(0x08009000, 0x08009fff).ram();

	map(0x0800aa04, 0x0800aa07).r(FUNC(hudson_poems_state::poems_rand_r));

	// this does't actually appear to be palette
	map(0x0800b000, 0x0800b003).w(FUNC(hudson_poems_state::unktable_w)); // writes a table of increasing 16-bit values here
	map(0x0800b004, 0x0800b007).w(FUNC(hudson_poems_state::unktable_reset_w));

	map(0x0800c040, 0x0800c05f).ram();

//	map(0x08020008, 0x0802000b).r(FUNC(hudson_poems_state::poems_rand_r));
//	map(0x08020010, 0x08020013).r(FUNC(hudson_poems_state::poems_rand_r));
//	map(0x08020014, 0x08020017).r(FUNC(hudson_poems_state::poems_rand_r));
//	map(0x08020018, 0x0802001b).r(FUNC(hudson_poems_state::poems_rand_r));
	map(0x08020020, 0x08020023).r(FUNC(hudson_poems_state::poems_rand_r));

	map(0x2c000000, 0x2c7fffff).ram().share("mainram");
}


static GFXDECODE_START( gfx_poems )
	GFXDECODE_ENTRY( "maincpu", 0, gfx_8x8x4_packed_lsb, 0, 16 )
	GFXDECODE_ENTRY( "maincpu", 0, gfx_8x8x8_raw, 0, 1 )
GFXDECODE_END

TIMER_DEVICE_CALLBACK_MEMBER(hudson_poems_state::screen_scanline)
{
	int scanline = param;

	if (scanline == 200)
	{
		m_maincpu->irq_request_hack(0x10);
	}

	if (scanline == 100) 
	{
		m_maincpu->irq_request_hack(0x2);
	}

//	if ((scanline %= 80) == 48) 
	{
//		m_maincpu->irq_request_hack(0x4);

		// this needs to change in RAM, presumably from an interrupt, but no idea how to get there
		m_maincpu->space(AS_PROGRAM).write_dword(0x2c01d92c, 0x01);
	}

	if (scanline == 150)
	{
//		m_maincpu->irq_off_hack();
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
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x100);

	SPEAKER(config, "speaker").front_center();

}

ROM_START( marimba )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD( "marimbatengoku.u2", 0x000000, 0x800000, CRC(b2ac0c5b) SHA1(48f3cdf399b032d86234125eeac3fb1cdc73538a) ) // glob with TSOP pads

	ROM_REGION( 0x400, "nv", 0 )
	ROM_LOAD( "at24c08a.u4", 0x000000, 0x400, CRC(e128a679) SHA1(73fb551d87ed911bd469899343fd36d9d579af39) )
ROM_END

} // anonymous namespace


CONS( 2005, marimba,      0,       0,      hudson_poems, hudson_poems, hudson_poems_state, empty_init, "Konami", "Marimba Tengoku (Japan)", MACHINE_IS_SKELETON )
