// license:BSD-3-Clause
// copyright-holders:David Haywood,Paul Priest
/*** DRIVER INFO **************************************************************

Macross Plus                        (c)1996 Banpresto
Quiz Bishoujo Senshi Sailor Moon    (c)1997 Banpresto

Driver by David Haywood

TODO:
 - what is the 'BIOS' ROM for? it appears to be data tables and is very different between games but we don't map it anywhere
 - convert tilemaps to devices?
 - quizmoon title gfx/sound in attract mode is incorrect? reference: https://youtu.be/925rRv2JG50

 68020 interrupts
 lev 1 : 0x64 : 0000 084c - unknown..
 lev 2 : 0x68 : 0000 0882 - unknown..
 lev 3 : 0x6c : 0000 08b0 - vblank?
 lev 4 : 0x70 : 001f 002a - x
 lev 5 : 0x74 : 001f 002a - x
 lev 6 : 0x78 : 001f 002a - x
 lev 7 : 0x7c : 001f 002a - x


**** README INFO **************************************************************

--- ROMSET: macrossp ---

Macross Plus
Banpresto Co., Ltd., 1996

PCB: BP964
CPU: MC68EC020FG25
SND: TMP68HC000N-16, OTTOR2 ES5506000102
OSC: 50.000MHz, 32.000MHz, 27.000MHz
RAM: TC55257CFL-85 x 8 (28 PIN TSOP), MCM62068AEJ25 x 12 (28 PIN SSOP), TC55328AJ-15 x 6 (28 PIN SSOP)
GFX: IKM-AA004   (208 PIN PQFP)
     IKM-AA0062  (208 PIN PQFP)
     IKM-AA005   x 2 (208 PIN PQFP)
DIPS: 2x 8-Position


DIPSW-1
------------------------------------------------------------------
    DipSwitch Title   | Function | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
------------------------------------------------------------------
                      | 1cn/1pl  |off|off|off|off|               |*
                      | 1cn/2pl  |on |off|off|off|               |
                      | 1cn/3pl  |off|on |off|off|               |
                      | 1cn/4pl  |on |on |off|off|               |
                      | 1cn/5pl  |off|off|on |off|               |
                      | 1cn/6pl  |on |off|on |off|               |
     Coin Chute A     | 1cn/7pl  |off|on |on |off|               |
                      | 2cn/1pl  |on |on |on |off|               |
                      | 2cn/3pl  |off|off|off|on |               |
                      | 2cn/5pl  |on |off|off|on |               |
                      | 3cn/1pl  |off|on |off|on |               |
                      | 3cn/2pl  |on |on |off|on |               |
                      | 3cn/4pl  |off|off|on |on |               |
                      | 4cn/1pl  |on |off|on |on |               |
                      | 4cn/3pl  |off|on |on |on |               |
                      |  Free    |on |on |on |on |               |
------------------------------------------------------------------
                      | 1cn/1pl  |               |off|off|off|off|*
                      | 1cn/2pl  |               |on |off|off|off|
                      | 1cn/3pl  |               |off|on |off|off|
                      | 1cn/4pl  |               |on |on |off|off|
                      | 1cn/5pl  |               |off|off|on |off|
                      | 1cn/6pl  |               |on |off|on |off|
     Coin Chute B     | 1cn/7pl  |               |off|on |on |off|
                      | 2cn/1pl  |               |on |on |on |off|
                      | 2cn/3pl  |               |off|off|off|on |
                      | 2cn/5pl  |               |on |off|off|on |
                      | 3cn/1pl  |               |off|on |off|on |
                      | 3cn/2pl  |               |on |on |off|on |
                      | 3cn/4pl  |               |off|off|on |on |
                      | 4cn/1pl  |               |on |off|on |on |
                      | 4cn/3pl  |               |off|on |on |on |
                      | 5cn/3pl  |               |on |on |on |on |
------------------------------------------------------------------


DIPSW-2
------------------------------------------------------------------
    DipSwitch Title   | Function | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
------------------------------------------------------------------
                      |  Normal  |off|off|                       |*
      Difficulty      |   Easy   |on |off|                       |
        Level         |   Hard   |off|on |                       |
                      |  V.Hard  |on |on |                       |
------------------------------------------------------------------
                      |     3    |       |off|off|               |*
    Players Count     |     4    |       |on |off|               |
                      |     5    |       |off|on |               |
                      |     2    |       |on |on |               |
------------------------------------------------------------------
      Demo Sound      |   Play   |               |off|           |*
                      |   Mute   |               |on |           |
------------------------------------------------------------------
      Screen Flip     |    Off   |                   |off|       |*
                      |    On    |                   |on |       |
------------------------------------------------------------------
Title Screen Language | Japanese |                       |off|   |*#
                      | English  |                       |on |   |
------------------------------------------------------------------
   Test / Game Mode   |   Game   |                           |off|*
                      |   Test   |                           |on |
------------------------------------------------------------------

* Denotes Factory Defaults
# English title page doesn't display the Japanese translation of the word "Macross"


ROMs:  (Filename = ROM label, extension also on ROM label)

TOP ROM PCB
-----------
BP964A-C.U1 \
BP964A-C.U2  |
BP964A-C.U3  |
BP964A-C.U4  > 27C040
BP964A.U19   |
BP964A.U20   |
BP964A.U21  /

BP964A.U9   \
BP964A.U10   |
BP964A.U11   |
BP964A.U12   |
BP964A.U13   |
BP964A.U14   > 32M (44 pin SOP - surface mounted)
BP964A.U15   |
BP964A.U16   |
BP964A.U17   |
BP964A.U18   |
BP964A.U24  /

ROMs:  (Filename = ROM Label)

MOTHERBOARD PCB
---------------
BP964A.U49  27C010

--- ROMSET: quizmoon ---

Quiz Bishoujo Senshi Sailor Moon - Chiryoku Tairyoku Toki no Un -
(c)1997 Banpresto / Gazelle
BP965A

-----------
Motherboard
-----------
CPU  : MC68EC020FG25
Sound: TMP68HC000N-16, ENSONIQ OTTO R2 (ES5506)
OSC  : 50.000MHz (X1), 32.000MHz (X2), 27.000MHz (X3)

ROMs:
u49.bin - (ST 27c1001)

GALs (16V8B, not dumped):
u009.bin
u200.bin

Custom chips:
IKM-AA004 1633JF8433 JAPAN 9523YAA (U62, 208pin QFP)
IKM-AA005 1670F1541 JAPAN 9525EAI (U47&48, 208pin QFP)
IKM-AA006 1633JF8432 JAPAN 9525YAA (U31, 208pin QFP)

--------------
Mask ROM board
--------------
u5.bin - Main programs (TI 27c040)
u6.bin |
u7.bin |
u8.bin |
u1.bin | (ST 27c1001)
u2.bin |
u3.bin |
u4.bin /

u09.bin - Graphics (uPD23C32000GX)
u10.bin |
u11.bin |
u12.bin |
u13.bin |
u15.bin |
u17.bin / (uPD23C16000GX)

u20.bin - Sound programs (ST 27c1001)
u21.bin /

u24.bin - Samples (uPD23C32000GX)
u25.bin |
u26.bin |
u27.bin /


----------
PCB Layout
----------

MOTHERBOARD
|---------------------------------------------------------------|
|TA8205AH  GAL16V8B(3)    |----------------|BP964A_U49          |
|  VOL                    |----------------||-------|  TC55328  |
| TL074    ES5506                           |AA005  |  TC55328  |
|  D6376                                    |       |  TC55328  |
|               TC55257    |-------------|  |       |  TC55328  |
|               TC55257    |   68000     |  |-------|           |
|     GAL16V8B(2)          |             |                      |
|     50MHz     TC55257    |-------------|  |-------|           |
|               TC55257                     |AA005  |       |-| |
|                                           |       |       | | |
|               TC55257                     |       |       | | |
|      |-------|TC55257                     |-------|       | | |
|      |68EC020|                                            | | |
|      |       | GAL16V8B(1)                |-------|       | | |
|      |-------|                            |AA0062 |       | | |
|          MCM6206  MCM6206                 |       |       | | |
|          MCM6206  MCM6206                 |       |       | | |
|                                           |-------|       |-| |
|          MCM6206  MCM6206                                     |
|                                           |-------|  TC55328  |
|          MCM6206  MCM6206                 |AA004  |  TC55328  |
|          MCM6206  MCM6206                 |       |           |
|          MCM6206  MCM6206                 |       |   32MHz   |
|DSW1(8)                  |----------------||-------|   27MHz   |
|DSW2(8)                  |----------------| TC55257  TC55257   |
|---------------------------------------------------------------|
Notes:
      68020 clock 25.000MHz [50/2] (QFP100)
      68000 clock 16.000MHz [32/2] (SDIP64)
      ES5506 clock 16.000MHz [32/2] (PLCC68)
      VSync 60Hz
      HSync 15.83kHz
      TC55257     - Toshiba TC55257CFL-85 32k x8 SRAM (SOJ28)
      TC55328     - Toshiba TC55328AJ-15 32k x8 SRAM (SOP28)
      MCM6206     - Motorola MCM6206BAEJ25 32k x8 SRAM (SOJ28)
      GAL16V8B(1) - stamped 'U008' (DIP20)
      GAL16V8B(2) - stamped 'U200' (DIP20)
      GAL16V8B(3) - stamped 'U009' (DIP20)
      D6376       - NEC uPD6376 Audio 2-Channel 16-Bit D/A Converter (SOIC16)
      Custom ICs  -
                   IKM-AA004 1633JF8433 (QFP208)
                   IKM-AA0062 1633CF8461 (QFP208)
                   IKM-AA005 1670F1541 (x2, QFP208)

ROM Board
---------

MASK ROM BOARD
|----------------------------------------|
|  |----------------|                    |
|  |----------------|                    |
|                              U19       |
| *U27    *U26                           |
|                                        |
|                                        |
| *U25     U24        *U23    *U22       |
|                                        |
|                                    |-| |
|  U18     U17         U21     U20   | | |
|                                    | | |
|                                    | | |
|                                    | | |
|  U16     U15         U14     U13   | | |
|                                    | | |
|                                    | | |
|  U9      U10         U11     U12   | | |
|                                    |-| |
|                                        |
| *U5     *U6         *U7     *U8        |
|                                        |
|  U1      U2          U3      U4        |
|                                        |
|  |----------------|                    |
|  |----------------|                    |
|----------------------------------------|
Notes:
      * - These locations not populated
      ROMs at locations U1, U2, U3, U4, U19, U20 & U21 are 27C040 EPROMs
      All other ROMs are surface mounted 32MBit SOP44 MaskROMs

******************************************************************************/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68020.h"
#include "machine/gen_latch.h"
#include "sound/es5506.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include <algorithm>

namespace {

//#define DEBUG_KEYS 1


class macrossp_state : public driver_device
{
public:
	macrossp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_scr_videoram(*this, "scr%u_videoram", 0U),
		m_scr_linezoom(*this, "scr%u_linezoom", 0U),
		m_scr_videoregs(*this, "scr%u_videoregs", 0U),

		m_text_videoram(*this, "text_videoram"),
		m_text_linezoom(*this, "text_linezoom"),
		m_text_videoregs(*this, "text_videoregs"),

		m_mainram(*this, "mainram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{
	}

	void quizmoon(machine_config &config);
	void macrossp(machine_config &config);

	void init_quizmoon();
	void init_macrossp();

private:
	// memory pointers
	required_shared_ptr<uint32_t> m_spriteram;
	required_shared_ptr_array<uint32_t, 3> m_scr_videoram;
	required_shared_ptr_array<uint32_t, 3> m_scr_linezoom;
	required_shared_ptr_array<uint32_t, 3> m_scr_videoregs;
	required_shared_ptr<uint32_t> m_text_videoram;
	required_shared_ptr<uint32_t> m_text_linezoom;
	required_shared_ptr<uint32_t> m_text_videoregs;
	required_shared_ptr<uint32_t> m_mainram;
	std::unique_ptr<uint32_t []> m_spriteram_old;
	std::unique_ptr<uint32_t []> m_spriteram_old2;

	// video-related
	tilemap_t  *m_scr_tilemap[3]{};
	tilemap_t  *m_text_tilemap = nullptr;

	// misc
	bool    m_sndpending = false;
	uint8_t m_snd_toggle = 0;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_16_device> m_soundlatch;

	uint32_t soundstatus_r();
	void soundcmd_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t soundcmd_r();
	void palette_fade_w(uint16_t data);
	void macrossp_speedup_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	[[maybe_unused]] void quizmoon_speedup_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template <unsigned Which> void scr_videoram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void text_videoram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template <unsigned Which> TILE_GET_INFO_MEMBER(get_scr_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, int line, int pri);
	void irqhandler(int state);

	void es5506_bank1_map(address_map &map) ATTR_COLD;
	void es5506_bank3_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/*
Sprite list is drawn backwards, and priorities with backgrounds are not transitive

=== Vid Registers ===
[0] - tiles
0x000003ff - global scrollx
0x00000c00 - color mode
0x0000c000 - priority
0x03ff0000 - global scrolly
0x90000000 - enable? Always 0x9

[1] - ???
0xffff0000 - another scrolly register, mainly used when zooming. unused by emulation
0x0000ffff - another scrollx register, mainly used when zooming. unused by emulation

[2] - zoom params
0xf0000000 - zoom enable (== 0xe, not == 0x2). Presumably one bit for x and y enable
0x01ff0000 - incy (0x40 is 1:1, incx is in lineram. might be more bits)

Interesting test cases (macrossp, quizmoon doesn't use tilemap zoom):
1) Title screen logo zoom
2) Second level, as zoom into end of canyon
3) Second level, as doors open to revels tracks/blue background for boss
4) Boss should go under bridge on level 4 when he first appears

*/

/*** SCR A-C LAYER ***/

template <unsigned Which>
void macrossp_state::scr_videoram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_scr_videoram[Which][offset]);

	m_scr_tilemap[Which]->mark_tile_dirty(offset);
}

template <unsigned Which>
TILE_GET_INFO_MEMBER(macrossp_state::get_scr_tile_info)
{
	uint32_t const attr = m_scr_videoram[Which][tile_index];
	uint32_t const tileno = attr & 0x0000ffff;

	uint32_t color;
	switch (m_scr_videoregs[Which][0] & 0x00000c00)
	{
		case 0x00000800:
			color = (attr & 0x000e0000) >> 15;
			break;

		case 0x00000400:
			color = (attr & 0x003e0000) >> 17;
			break;

		default:
			color = machine().rand() & 7;
			break;
	}

	tileinfo.set(1 + Which, tileno, color, TILE_FLIPYX((attr & 0xc0000000) >> 30));
}

/*** TEXT LAYER ***/

void macrossp_state::text_videoram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_text_videoram[offset]);

	m_text_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(macrossp_state::get_text_tile_info)
{
	uint32_t const tileno = m_text_videoram[tile_index] & 0x0000ffff;
	uint32_t const colour = (m_text_videoram[tile_index] & 0x00fe0000) >> 17;

	tileinfo.set(4, tileno, colour, 0);
}



/*** VIDEO START / UPDATE ***/

void macrossp_state::video_start()
{
	uint32_t const sprram_size = m_spriteram.length();
	m_spriteram_old = make_unique_clear<uint32_t []>(sprram_size);
	m_spriteram_old2 = make_unique_clear<uint32_t []>(sprram_size);

	m_text_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(macrossp_state::get_text_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_scr_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(macrossp_state::get_scr_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_scr_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(macrossp_state::get_scr_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_scr_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(macrossp_state::get_scr_tile_info<2>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);

	m_text_tilemap->set_transparent_pen(0);
	m_scr_tilemap[0]->set_transparent_pen(0);
	m_scr_tilemap[1]->set_transparent_pen(0);
	m_scr_tilemap[2]->set_transparent_pen(0);

	m_gfxdecode->gfx(0)->set_granularity(64);
	m_gfxdecode->gfx(1)->set_granularity(64);
	m_gfxdecode->gfx(2)->set_granularity(64);
	m_gfxdecode->gfx(3)->set_granularity(64);

	save_pointer(NAME(m_spriteram_old), sprram_size);
	save_pointer(NAME(m_spriteram_old2), sprram_size);
}



void macrossp_state::draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	//  uint32_t *source = m_spriteram;
	uint32_t const *source = (m_spriteram_old2.get() + m_spriteram.bytes() / 4) - 3; // buffers by two frames
	uint32_t const *finish = m_spriteram_old2.get();

	// reverse order
	while (source >= finish)
	{
		/*

		 --hh hhyy yyyy yyyy   CCww wwxx xxxx xxxx

		 ---- --zz zzzz zzzz   ---- --ZZ ZZZZ ZZZZ

		 fFa- pp-- cccc c---   tttt tttt tttt tttt

		*/


		int const wide = (source[0] & 0x00003c00) >> 10;
		int const high = (source[0] & 0x3c000000) >> 26;

		int xpos = (source[0] & 0x000003ff) >> 0;
		int ypos = (source[0] & 0x03ff0000) >> 16;

		int const xzoom = (source[1] & 0x000003ff) >> 0; // 0x100 is zoom factor of 1.0
		int const yzoom = (source[1] & 0x03ff0000) >> 16;

		int col;
		int const tileno = (source[2] & 0x0000ffff) >> 0;

		bool const flipx = BIT(source[2], 30);
		bool const flipy = BIT(source[2], 31);

		int const alpha = BIT(source[2], 29) ? 0x80 : 0xff; // alpha blending enable?

		int loopno = 0;

		int const pri = (source[2] & 0x0c000000) >> 26;
		uint32_t primask = 0;
		if (pri <= 0) primask |= GFX_PMASK_1;
		if (pri <= 1) primask |= GFX_PMASK_2;
		if (pri <= 2) primask |= GFX_PMASK_4;
		if (pri <= 3) primask |= GFX_PMASK_8;

		switch (source[0] & 0x0000c000)
		{
			case 0x00008000:
				col = (source[2] & 0x00380000) >> 17;
				break;

			case 0x00004000:
				col = (source[2] & 0x00f80000) >> 19;
				break;

			default:
				col = machine().rand();
				break;
		}

		if (xpos > 0x1ff) xpos -= 0x400;
		if (ypos > 0x1ff) ypos -= 0x400;

		// loop params
		int ymin = 0;
		int ymax = high + 1;
		int yinc = 1;
		int yoffst = 0;
		if (flipy)
		{
			yoffst = (high * yzoom * 16);
			ymin = high;
			ymax = -1;
			yinc = -1;
		}

		int xmin = 0;
		int xmax = wide + 1;
		int xinc = 1;
		int xoffst = 0;
		if (flipx)
		{
			xoffst = (wide * xzoom * 16);
			xmin = wide;
			xmax = -1;
			xinc = -1;
		}

		int yoffset = yoffst;
		for (int ycnt = ymin; ycnt != ymax; ycnt += yinc)
		{
			int xoffset = xoffst;
			for (int xcnt = xmin; xcnt != xmax; xcnt += xinc)
			{
				int fudged_xzoom = xzoom << 8;
				int fudged_yzoom = yzoom << 8;

				// cover seams as don't know exactly how many pixels on target will cover, and can't specify fractional offsets to start
				if (xzoom < 0x100) fudged_xzoom += 0x600;
				if (yzoom < 0x100) fudged_yzoom += 0x600;

				gfx->prio_zoom_alpha(bitmap, cliprect,
					tileno + loopno, col,
					flipx, flipy,
					xpos + (xoffset >> 8), ypos + (yoffset >> 8),
					fudged_xzoom, fudged_yzoom,
					screen.priority(), primask, 0, alpha);

				xoffset += ((xzoom * 16) * xinc);
				loopno++;
			}
			yoffset += ((yzoom * 16) * yinc);
		}
		source -= 3;
	}
}


void macrossp_state::draw_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, int line, int pri)
{
	tilemap_t *tm = m_scr_tilemap[layer];
	uint32_t const *const vr = m_scr_videoregs[layer];
	uint32_t const *const lr = m_scr_linezoom[layer];

	if ((vr[2] & 0xf0000000) == 0xe0000000) // zoom enable (guess, surely wrong)
	{
		int incx;

		int startx     = ((vr[0] & 0x000003ff) << 16);
		int starty     = ((vr[0] & 0x03ff0000) >> 0);
		int const incy = (vr[2] & 0x01ff0000) >> 6;

		if (line & 1)
			incx = (lr[line / 2] & 0x0000ffff) >> 0;
		else
			incx = (lr[line / 2] & 0xffff0000) >> 16;

		incx <<= 10;

		// scroll register contain position relative to the center of the screen, so adjust
		startx -= (368 / 2) * (incx - 0x10000);
		starty -= (240 / 2) * (incy - 0x10000);

// previous logic, which gives mostly comparable results, vr[1] is now unused
//      int startx = (vr[1] & 0x0000ffff) << 16;
//      int starty = (vr[1] & 0xffff0000) >> 0;
//      startx -= (368 / 2) * incx;
//      starty -= (240 / 2) * incy;

		tm->draw_roz(screen, bitmap, cliprect,
				startx,starty,incx,0,0,incy,
				1,  // wraparound
				0, 1 << pri);
	}
	else
	{
		tm->set_scrollx(0, ((vr[0] & 0x000003ff) >>  0));
		tm->set_scrolly(0, ((vr[0] & 0x03ff0000) >> 16));
		tm->draw(screen, bitmap, cliprect, 0, 1<<pri);
	}
}

uint32_t macrossp_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int layerpri[3];
	int sprites = true;
	int backgrounds = true;

	const rectangle &visarea = screen.visible_area();
	rectangle clip = visarea;

	// 0 <= layerpri <= 2
	layerpri[0] = (m_scr_videoregs[0][0] & 0x0000c000) >> 14;
	layerpri[1] = (m_scr_videoregs[1][0] & 0x0000c000) >> 14;
	layerpri[2] = (m_scr_videoregs[2][0] & 0x0000c000) >> 14;

	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->black_pen(), cliprect);

#ifdef DEBUG_KEYS
	const input_code lay_keys[8] = {KEYCODE_Q, KEYCODE_W, KEYCODE_E, KEYCODE_R, KEYCODE_T};
	bool lay_debug = false;
	for (int pri = 0; pri <= 4; pri++)
	{
		if (machine().input().code_pressed(lay_keys[pri]))
			lay_debug = true;
	}
	if (machine().input().code_pressed(KEYCODE_G))
		sprites = false;
	if (machine().input().code_pressed(KEYCODE_H))
		backgrounds = false;
#endif

	for (int pri = 0; pri <= 3; pri++)
	{
#ifdef DEBUG_KEYS
		if (lay_debug && !machine().input().code_pressed(lay_keys[pri]))
			continue;
#endif

		if (!backgrounds)
			continue;

		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			clip.min_y = clip.max_y = y;

			// quizmoon map requires that layer 2 be drawn over layer 3 when same pri
			for (int layer = 2; layer >= 0; layer--)
			{
				if (layerpri[layer] == pri)
					draw_layer(screen, bitmap, clip, layer, y, pri);
			}
		}

	}

#ifdef DEBUG_KEYS
	if (!lay_debug && !machine().input().code_pressed(lay_keys[4]))
#endif
		m_text_tilemap->draw(screen, bitmap, cliprect, 0, 8);

	if (sprites)
		draw_sprites(screen, bitmap, cliprect);

#if 0
	popmessage  ("scra - %08x %08x %08x\nscrb - %08x %08x %08x\nscrc - %08x %08x %08x",
			m_scr_videoregs[0][0] & 0xffffffff, // yyyyxxxx
			m_scr_videoregs[0][1], // ??? more scrolling?
			m_scr_videoregs[0][2], // 08 - 0b

			m_scr_videoregs[1][0] & 0xffffffff, // 00 - 03
			m_scr_videoregs[1][1], // 04 - 07
			m_scr_videoregs[1][2], // 08 - 0b

			m_scr_videoregs[2][0] & 0xffffffff, // 00 - 03
			m_scr_videoregs[2][1], // 04 - 07
			m_scr_videoregs[2][2]);// 08 - 0b
#endif
	return 0;
}

void macrossp_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		// looks like sprites are *two* frames ahead, like nmk16
		memcpy(m_spriteram_old2.get(), m_spriteram_old.get(), m_spriteram.bytes());
		memcpy(m_spriteram_old.get(), m_spriteram, m_spriteram.bytes());
	}
}


/*** VARIOUS READ / WRITE HANDLERS *******************************************/

uint32_t macrossp_state::soundstatus_r()
{
	//  logerror("%08x read soundstatus\n", m_maincpu->pc());

	// bit 1 is sound status
	// bit 0 unknown - it is expected to toggle, vblank?

	if (!machine().side_effects_disabled())
		m_snd_toggle ^= 1;

	return (m_sndpending ? 2 : 0) | m_snd_toggle;
}

void macrossp_state::soundcmd_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_16_31)
	{
		//logerror("%08x write soundcmd %08x (%08x)\n",m_maincpu->pc(),data,mem_mask);
		m_soundlatch->write(data >> 16);
		m_sndpending = true;
		m_audiocpu->set_input_line(2, HOLD_LINE);
		// spin for a while to let the sound CPU read the command
		m_maincpu->spin_until_time(attotime::from_usec(50));
	}
}

uint16_t macrossp_state::soundcmd_r()
{
	//  logerror("%06x read soundcmd\n",m_audiocpu->pc());
	if (!machine().side_effects_disabled())
		m_sndpending = false;
	return m_soundlatch->read();
}

void macrossp_state::palette_fade_w(uint16_t data)
{
	// 0xff is written a few times on startup
	if (data >> 8 != 0xff)
	{
		// range seems to be 40 (brightest) to 252 (darkest)
		uint8_t const fade = ((data >> 8) - 40) / 212.0 * 255.0;
		m_screen->set_brightness(0xff - fade);
	}
}

/*** MEMORY MAPS *************************************************************/

void macrossp_state::main_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x800000, 0x802fff).ram().share(m_spriteram);
	// SCR A Layer
	map(0x900000, 0x903fff).ram().w(FUNC(macrossp_state::scr_videoram_w<0>)).share(m_scr_videoram[0]);
	map(0x904200, 0x9043ff).ram().share(m_scr_linezoom[0]); // W/O?
	map(0x905000, 0x90500b).ram().share(m_scr_videoregs[0]); // W/O?
	// SCR B Layer
	map(0x908000, 0x90bfff).ram().w(FUNC(macrossp_state::scr_videoram_w<1>)).share(m_scr_videoram[1]);
	map(0x90c200, 0x90c3ff).ram().share(m_scr_linezoom[1]); // W/O?
	map(0x90d000, 0x90d00b).ram().share(m_scr_videoregs[1]); // W/O?
	// SCR C Layer
	map(0x910000, 0x913fff).ram().w(FUNC(macrossp_state::scr_videoram_w<2>)).share(m_scr_videoram[2]);
	map(0x914200, 0x9143ff).ram().share(m_scr_linezoom[2]);// W/O?
	map(0x915000, 0x91500b).ram().share(m_scr_videoregs[2]); // W/O?
	// Text Layer
	map(0x918000, 0x91bfff).ram().w(FUNC(macrossp_state::text_videoram_w)).share(m_text_videoram);
	map(0x91c200, 0x91c3ff).ram().share(m_text_linezoom); // W/O?
	map(0x91d000, 0x91d00b).ram().share(m_text_videoregs); // W/O?

	map(0xa00000, 0xa03fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");

	map(0xb00000, 0xb00003).portr("INPUTS");
	map(0xb00004, 0xb00007).r(FUNC(macrossp_state::soundstatus_r)).nopw(); // irq related?
	map(0xb00008, 0xb0000b).nopw();    // irq related?
	map(0xb0000c, 0xb0000f).portr("DSW").nopw();
	map(0xb00012, 0xb00013).w(FUNC(macrossp_state::palette_fade_w));
	map(0xb00020, 0xb00023).nopw();

	map(0xc00000, 0xc00003).w(FUNC(macrossp_state::soundcmd_w));

	map(0xf00000, 0xf1ffff).ram().share(m_mainram); // Main Ram
//  map(0xfe0000, 0xfe0003).noprw();
}

void macrossp_state::sound_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x207fff).ram();
	map(0x400000, 0x40007f).rw("ensoniq", FUNC(es5506_device::read), FUNC(es5506_device::write)).umask16(0x00ff);
	map(0x600000, 0x600001).r(FUNC(macrossp_state::soundcmd_r));
}

void macrossp_state::es5506_bank1_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("ensoniq.0", 0x400000);
}

void macrossp_state::es5506_bank3_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("ensoniq.2", 0x400000);
}

/*** INPUT PORTS *************************************************************/

static INPUT_PORTS_START( macrossp )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED ) // Unknown use
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0000ffc0, IP_ACTIVE_LOW, IPT_UNUSED ) // Unknown use
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED ) // Unknown use, but not dipswitches
	PORT_DIPNAME( 0x000f0000, 0x000f0000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(          0x00020000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(          0x00050000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x00080000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x00040000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(          0x00010000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(          0x000f0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0x00030000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(          0x00070000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(          0x000e0000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(          0x00060000, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(          0x000d0000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(          0x000c0000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(          0x000b0000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(          0x000a0000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(          0x00090000, "1 Coins/7 Credits" )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Free_Play) )
	PORT_DIPNAME( 0x00f00000, 0x00f00000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(          0x00200000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(          0x00500000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x00800000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(          0x00400000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(          0x00100000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(          0x00f00000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0x00300000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(          0x00700000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(          0x00e00000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(          0x00600000, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(          0x00d00000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(          0x00c00000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(          0x00b00000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(          0x00a00000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(          0x00900000, "1 Coins/7 Credits" )
	PORT_DIPNAME( 0x03000000, 0x03000000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(          0x02000000, DEF_STR( Easy ) )
	PORT_DIPSETTING(          0x03000000, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x01000000, DEF_STR( Hard ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c000000, 0x0c000000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(          0x00000000, "2" )
	PORT_DIPSETTING(          0x0c000000, "3" )
	PORT_DIPSETTING(          0x08000000, "4" )
	PORT_DIPSETTING(          0x04000000, "5" )
	PORT_DIPNAME( 0x10000000, 0x10000000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x10000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x20000000, 0x20000000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:6")  // See above for dips listing.... also in Quiz game's test screens
	PORT_DIPSETTING(          0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x40000000, 0x00000000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:7")  // See title page for difference :-)  The Manual shows this as UNUSED
	PORT_DIPSETTING(          0x40000000, DEF_STR( Japanese ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( English ) )
	PORT_SERVICE_DIPLOC(  0x80000000, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( quizmoon )
	PORT_INCLUDE(macrossp)

	PORT_MODIFY("INPUTS")
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Test ) )
	PORT_DIPSETTING(          0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Tilt ) )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )

	PORT_MODIFY("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x40000000, 0x40000000, "SW2:7" ) // no Language dipswitch for this game
INPUT_PORTS_END

/*** GFX DECODE **************************************************************/

static GFXDECODE_START( gfx_macrossp )
	GFXDECODE_ENTRY( "sprites",  0, gfx_16x16x8_raw,        0x000, 0x20 ) // 8bpp but 6bpp granularity
	GFXDECODE_ENTRY( "bgtiles0", 0, gfx_16x16x8_raw,        0x800, 0x20 ) // 8bpp but 6bpp granularity
	GFXDECODE_ENTRY( "bgtiles1", 0, gfx_16x16x8_raw,        0x800, 0x20 ) // 8bpp but 6bpp granularity
	GFXDECODE_ENTRY( "bgtiles2", 0, gfx_16x16x8_raw,        0x800, 0x20 ) // 8bpp but 6bpp granularity
	GFXDECODE_ENTRY( "fgtiles",  0, gfx_16x16x4_packed_msb, 0x800, 0x80 )
GFXDECODE_END

/*** MACHINE DRIVER **********************************************************/

void macrossp_state::irqhandler(int state)
{
	logerror("ES5506 irq %d\n", state);

	/* IRQ lines 1 & 4 on the sound 68000 are definitely triggered by the ES5506,
	but I haven't noticed the ES5506 ever assert the line - maybe only used when developing the game? */
	//  m_audiocpu->set_input_line(1, state ? ASSERT_LINE : CLEAR_LINE);
}

void macrossp_state::machine_start()
{
	save_item(NAME(m_sndpending));
	save_item(NAME(m_snd_toggle));
}

void macrossp_state::machine_reset()
{
	m_sndpending = false;
	m_snd_toggle = 0;
}

void macrossp_state::macrossp(machine_config &config)
{
	// basic machine hardware
	M68EC020(config, m_maincpu, 50_MHz_XTAL / 2);   // 25 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &macrossp_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(macrossp_state::irq3_line_hold)); // there are others ...

	M68000(config, m_audiocpu, 32_MHz_XTAL / 2);    // 16 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &macrossp_state::sound_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*16, 16*16);
	m_screen->set_visarea(0*16, 24*16-1, 0*16, 15*16-1);
	m_screen->set_screen_update(FUNC(macrossp_state::screen_update));
	m_screen->screen_vblank().set(FUNC(macrossp_state::screen_vblank));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_macrossp);
	PALETTE(config, m_palette).set_format(palette_device::RGBx_888, 4096);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_16(config, m_soundlatch);

	es5506_device &ensoniq(ES5506(config, "ensoniq", 32_MHz_XTAL / 2));    // 16 MHz
	ensoniq.set_region0("ensoniq.0");
	ensoniq.set_addrmap(1, &macrossp_state::es5506_bank1_map);
	ensoniq.set_channels(1);
	ensoniq.irq_cb().set(FUNC(macrossp_state::irqhandler));
	ensoniq.add_route(0, "lspeaker", 0.1);
	ensoniq.add_route(1, "rspeaker", 0.1);
}

void macrossp_state::quizmoon(machine_config &config)
{
	macrossp(config);
	m_screen->set_visarea(0, 24*16-1, 0*8, 14*16-1);

	es5506_device *ensoniq = subdevice<es5506_device>("ensoniq");
	ensoniq->set_region2("ensoniq.2");
	ensoniq->set_addrmap(3, &macrossp_state::es5506_bank3_map);
}


/*** ROM LOADING *************************************************************/

ROM_START( macrossp )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "bp964a-c.u1", 0x000003, 0x080000, CRC(39da35e7) SHA1(022c902b0adf21090c650ce68e4b8b42498e1be6) )
	ROM_LOAD32_BYTE( "bp964a-c.u2", 0x000002, 0x080000, CRC(86d0ca6a) SHA1(8cc1b4a83cbba8b07e1343c5c20e2590d3ce471a) )
	ROM_LOAD32_BYTE( "bp964a-c.u3", 0x000001, 0x080000, CRC(fb895a7b) SHA1(547e5d3d43e503a15573748ab49a44e5569db1d7) )
	ROM_LOAD32_BYTE( "bp964a-c.u4", 0x000000, 0x080000, CRC(8c8b966c) SHA1(881fa2b7aefc3ea916924b715d2cd0ceaffe2d92) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "bp964a.u20", 0x000001, 0x080000, CRC(12960cbb) SHA1(7182c4b36849a5d34ddf388bf5f4485ed360fe84) )
	ROM_LOAD16_BYTE( "bp964a.u21", 0x000000, 0x080000, CRC(87bdd2fc) SHA1(c33f087ebca6e98db195404788ca8e0cc6663622) )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "bp964a.u49", 0x000000, 0x020000, CRC(ad203f76) SHA1(3eb86eeeb020349dfd88ebc8b4fc9579d1cc50fb) )  // 'BIOS'

	ROM_REGION( 0x1000000, "sprites", 0 ) // sprites - 16x16x8
	ROM_LOAD32_BYTE( "bp964a.u9",  0x000003, 0x400000, CRC(bd51a70d) SHA1(3447ae9d368e4e33df2d4e2848b4fd5aa0fc6840) )
	ROM_LOAD32_BYTE( "bp964a.u10", 0x000002, 0x400000, CRC(ab84bba7) SHA1(d30876b2e45c4b78cda27d3c648100e60f739d9c) )
	ROM_LOAD32_BYTE( "bp964a.u11", 0x000001, 0x400000, CRC(b9ae1d0b) SHA1(bc541a8bd622c99cf5065b3a793f0b5f6420ac64) )
	ROM_LOAD32_BYTE( "bp964a.u12", 0x000000, 0x400000, CRC(8dda1052) SHA1(c374335e98859ae98ac392a7cdb44f15b4e1c23a) )

	ROM_REGION( 0x800000, "bgtiles0", 0 ) // backgrounds - 16x16x8
	ROM_LOAD( "bp964a.u13", 0x000000, 0x400000, CRC(f4d3c5bf) SHA1(82522d276a6d49148da8a4fb11846a039429bcf8) )
	ROM_LOAD( "bp964a.u14", 0x400000, 0x400000, CRC(4f2dd1b2) SHA1(30a2c9fb26bca8bb27fbc5637878f99e7f6ad8f4) )

	ROM_REGION( 0x800000, "bgtiles1", 0 ) // backgrounds - 16x16x8
	ROM_LOAD( "bp964a.u15", 0x000000, 0x400000, CRC(5b97a870) SHA1(16f3921649b28ecb6d628871214f972333bbeca4) )
	ROM_LOAD( "bp964a.u16", 0x400000, 0x400000, CRC(c8a0cd64) SHA1(2a30a4d4ec3f94631783eb08c62003b116bb2ee3) )

	ROM_REGION( 0x800000, "bgtiles2", 0 ) // backgrounds - 16x16x8
	ROM_LOAD( "bp964a.u17", 0x000000, 0x400000, CRC(f2470876) SHA1(e683208432f71f3cc19ced245fa5b8a82466d19b) )
	ROM_LOAD( "bp964a.u18", 0x400000, 0x400000, CRC(52ef21f3) SHA1(08fb1969ad0ffd0c5bf11d3d5448a26112d562b0) )

	ROM_REGION( 0x400000, "fgtiles", 0 ) // foreground - 16x16x4
	ROM_LOAD( "bp964a.u19", 0x000000, 0x080000, CRC(19c7acd9) SHA1(b7631e74f359c5570c44addf46c3e96c80adc6c3) )

	ROM_REGION16_BE( 0x800000, "ensoniq.0", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "bp964a.u24", 0x000000, 0x400000, CRC(93f90336) SHA1(75daa2f8cedc732cf5ef98254f61748c94b94aea) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "u8.u8",     0x0000, 0x0117, CRC(99bd3cc1) SHA1(b0d3ac93cb5d2857cf9c184c7a2b4afa0211d588) ) // unprotected GAL16V8B
	ROM_LOAD( "u9.u9",     0x0200, 0x0117, CRC(480f4860) SHA1(7ff3723122cbdda5c0dcea5167508e9a7437f551) ) // unprotected GAL16V8B
	ROM_LOAD( "u200.u200", 0x0400, 0x0117, CRC(9343ad76) SHA1(a2adf2510011078ee9fae65acdd2daa0ffab48f6) ) // unprotected GAL16V8B
ROM_END

ROM_START( quizmoon )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "u1.bin",  0x000003, 0x020000, CRC(ea404553) SHA1(123bb8e399a5b54c43f4eb41d3e5f52c6947900f) )
	ROM_LOAD32_BYTE( "u2.bin",  0x000002, 0x020000, CRC(024eedff) SHA1(cbaa2b71980a2686e582331616dc36e34ecd9e67) )
	ROM_LOAD32_BYTE( "u3.bin",  0x000001, 0x020000, CRC(545b1d17) SHA1(f1b15260942482857c48b574ada1e2a3b728f395) )
	ROM_LOAD32_BYTE( "u4.bin",  0x000000, 0x020000, CRC(60b3d18c) SHA1(230342a084938fdbd2b4da23df2054391eab165b) )
	ROM_LOAD32_BYTE( "u5.bin",  0x200003, 0x080000, CRC(4cc65f5e) SHA1(eebad4c1bf761f08cacbf8c75e7f7bd421ee65ca) )
	ROM_LOAD32_BYTE( "u6.bin",  0x200002, 0x080000, CRC(d84b7c6c) SHA1(ba6ab34fb5c61aa1a97159b7aa3d89e978fb0538) )
	ROM_LOAD32_BYTE( "u7.bin",  0x200001, 0x080000, CRC(656b2125) SHA1(ac3874e71ec0aa4e77ac0d556e4572606ce673c7) )
	ROM_LOAD32_BYTE( "u8.bin",  0x200000, 0x080000, CRC(944df309) SHA1(ee85f6dbfe970b63943d01d9f8b491717a4d5a71) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "u20.bin", 0x000001, 0x020000, CRC(d7ad1ffb) SHA1(9d375285628b32296c93456a00bc005a3f40ce38) )
	ROM_LOAD16_BYTE( "u21.bin", 0x000000, 0x020000, CRC(6fc625c6) SHA1(542bc025cf0e37686eae5d6c80bc5e047d6389fd) )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "u49.bin", 0x000000, 0x020000, CRC(1590ad81) SHA1(04fb8119d9eafc6d2a921700dfb11e9c8b705c88) )  // 'BIOS'

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD32_BYTE( "u9.bin",  0x0000003, 0x0400000, CRC(aaaf2ca9) SHA1(b9e59590daf4cdee4b1deeb6d4ecc80eb12a2e18) )
	ROM_LOAD32_BYTE( "u10.bin", 0x0000002, 0x0400000, CRC(f0349691) SHA1(623a680ad164d407be0af585a15540f0dca995a4) )
	ROM_LOAD32_BYTE( "u11.bin", 0x0000001, 0x0400000, CRC(893ab178) SHA1(ba68b9a3e81af4c2565715504ada35c7da3f135f) )
	ROM_LOAD32_BYTE( "u12.bin", 0x0000000, 0x0400000, CRC(39b731b8) SHA1(2bf1d083fc6d8058a0d26b29714945e8be0e2c79) )

	ROM_REGION( 0x400000, "bgtiles0", 0 )
	ROM_LOAD( "u13.bin", 0x0000000, 0x0400000, CRC(3dcbb041) SHA1(fcff67113707fcf14d49538551724490498c0909) )

	ROM_REGION( 0x400000, "bgtiles1", 0 )
	ROM_LOAD( "u15.bin", 0x0000000, 0x0400000, CRC(b84224f0) SHA1(7163aec2cc118111b2c5d8deb61133d762a5d74c) )

	ROM_REGION( 0x0200000, "bgtiles2", 0 )
	ROM_LOAD( "u17.bin", 0x0000000, 0x0200000, CRC(ff93c949) SHA1(13917d73a6cb70d03d0335bd816bf6b094758d0b) )

	ROM_REGION( 0x400000, "fgtiles", ROMREGION_ERASE00 )
	// nothing on this game?

	ROM_REGION16_BE( 0x800000, "ensoniq.0", 0 )
	ROM_LOAD16_BYTE( "u26.bin", 0x0000000, 0x0400000, CRC(6c8f30d4) SHA1(7e215589e4a52cbce7f2bb31b333f874a9f83d00) )
	ROM_LOAD16_BYTE( "u24.bin", 0x0000001, 0x0400000, CRC(5b12d0b1) SHA1(c5ddff2053148a1da0710a10f48689bf5c736ae4) )

	ROM_REGION16_BE( 0x800000, "ensoniq.2", 0 )
	ROM_LOAD16_BYTE( "u27.bin", 0x0000000, 0x0400000, CRC(bd75d165) SHA1(2da770d15c812cbfdb4e3048d320071edffccfa1) )
	ROM_LOAD16_BYTE( "u25.bin", 0x0000001, 0x0400000, CRC(3b9689bc) SHA1(0857c3d3e9810f9468f7c17f8b795825c55a9f08) )
ROM_END



void macrossp_state::macrossp_speedup_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
/*
PC :00018104 018104: addq.w  #1, $f1015a.l
PC :0001810A 01810A: cmp.w   $f10140.l, D0
PC :00018110 018110: beq     18104
*/

	COMBINE_DATA(&m_mainram[0x10158 / 4]);
	if (m_maincpu->pc() == 0x001810A) m_maincpu->spin_until_interrupt();
}

void macrossp_state::quizmoon_speedup_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_mainram[0x00020 / 4]);
	if (m_maincpu->pc() == 0x1cc) m_maincpu->spin_until_interrupt();
}

void macrossp_state::init_macrossp()
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xf10158, 0xf1015b, write32s_delegate(*this, FUNC(macrossp_state::macrossp_speedup_w)));
}

void macrossp_state::init_quizmoon()
{
#if 0
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xf00020, 0xf00023, write32s_delegate(*this, FUNC(macrossp_state::quizmoon_speedup_w)));
#endif
}

} // anonymous namespace


GAME( 1996, macrossp, 0, macrossp, macrossp, macrossp_state, init_macrossp, ROT270, "MOSS / Banpresto", "Macross Plus", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1997, quizmoon, 0, quizmoon, quizmoon, macrossp_state, init_quizmoon, ROT0,   "Banpresto", "Quiz Bishoujo Senshi Sailor Moon - Chiryoku Tairyoku Toki no Un", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
