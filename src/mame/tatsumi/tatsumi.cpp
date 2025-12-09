// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Angelo Salese
/***************************************************************************

    This is a common base for apache3.cpp, roundup5.cpp and cyclwarr.cpp

    Gray Out (from 1987) is likely a similar setup
    http://www.tatsu-mi.co.jp/game/trace/index.html

    Incredibly complex hardware!  These are all different boards, but share
    a similar sprite chip (TZB215 on Apache 3, TZB315 on others).  Other
    graphics (road, sky, bg/fg layers) all differ between games.

    TODO:
    - Sprite rotation
    - Split these games into individual drivers, write new devices for video routines.
    - Dip switches
    - Deviceify HD6445 (superset of 6845)
    - Various other things..

***************************************************************************/

#include "emu.h"
#include "tatsumi.h"

#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "machine/adc0808.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "screen.h"
#include "speaker.h"

void tatsumi_state::hd6445_crt_w(offs_t offset, uint8_t data)
{
	if (offset==0)
		m_hd6445_address = data & 0x3f;
	if (offset==1)
	{
		m_hd6445_reg[m_hd6445_address] = data;

		static char const *const regnames[40] =
		{
			"Horizontal Total Characters", "Horizontal Displayed Characters", "Horizontal Sync Position",   "Sync Width",
			"Vertical Total Rows",         "Vertical Total Adjust",           "Vertical Displayed Rows",    "Vertical Sync Position",
			"Interlace Mode and Skew",     "Max Raster Address",              "Cursor 1 Start",             "Cursor 1 End",
			"Screen 1 Start Address (H)",  "Screen 1 Start Address (L)",      "Cursor 1 Address (H)",       "Cursor 1 Address (L)",
			"Light Pen (H) (RO)",          "Light Pen (L) (RO)",              "Screen 2 Start Position",    "Screen 2 Start Address (H)",
			"Screen 2 Start Address (L)",  "Screen 3 Start Position",         "Screen 3 Start Address (H)", "Screen 3 Start Address (L)",
			"Screen 4 Start Position",     "Screen 4 Start Address (H)",      "Screen 4 Start Address (L)", "Vertical Sync Position Adjust",
			"Light Pen Raster (RO)",       "Smooth Scrolling",                "Control 1",                  "Control 2",
			"Control 3",                   "Memory Width Offset",             "Cursor 2 Start",             "Cursor 2 End",
			"Cursor 2 Address (H)",        "Cursor 2 Address (L)",            "Cursor 1 Width",             "Cursor 2 Width"
		};

		if(m_hd6445_address < 40)
			logerror("HD6445: register %s [%02x R%d] set %02x\n",regnames[m_hd6445_address],m_hd6445_address,m_hd6445_address,data);
		else
			logerror("HD6445: illegal register access [%02x R%d] set %02x\n",m_hd6445_address,m_hd6445_address,data);
	}
}

uint16_t tatsumi_state::tatsumi_sprite_control_r(offs_t offset)
{
	return m_sprite_control_ram[offset];
}

void tatsumi_state::tatsumi_sprite_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sprite_control_ram[offset]);

	/* 0xe0 is bank switch, others unknown */
//  if ((offset==0xe0 && data&0xefff) || offset!=0xe0)
//      logerror("%s:  Tatsumi TZB215 sprite control %04x %08x\n", m_maincpu->pc(), offset, data);
}

// apply shadowing to underlying layers
// TODO: it might mix up with the lower palette bank instead (color bank 0x1400?)
void tatsumi_state::apply_shadow_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &shadow_bitmap, uint8_t xor_output)
{
	for(int y=cliprect.min_y;y<cliprect.max_y;y++)
	{
		for(int x=cliprect.min_x;x<cliprect.max_x;x++)
		{
			uint8_t shadow = shadow_bitmap.pix(y, x);
			// xor_output is enabled during Chen boss fight (where shadows have more brightness than everything else)
			// TODO: transition before fighting him should also black out all the background tilemaps too!?
			//       (more evidence that we need to mix with color bank 0x1400 instead of doing true RGB mixing).
			if(shadow ^ xor_output)
			{
				rgb_t shadow_pen = bitmap.pix(y, x);
				bitmap.pix(y, x) = rgb_t(shadow_pen.r() >> 1,shadow_pen.g() >> 1, shadow_pen.b() >> 1);
			}
		}
	}
}

void tatsumi_state::text_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_tx_layer->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(tatsumi_state::get_text_tile_info)
{
	int tile = m_videoram[tile_index];
	tileinfo.set(0, tile & 0xfff, tile >> 12, 0);
}

void tatsumi_state::tatsumi_reset()
{
	m_last_control = 0;
	m_control_word = 0;

	save_item(NAME(m_last_control));
	save_item(NAME(m_control_word));
}

uint16_t tatsumi_state::tatsumi_v30_68000_r(offs_t offset)
{
	const uint16_t* rom=(uint16_t*)m_subregion->base();

//logerror("%s:68000_r(%04X),cw=%04X\n", m_maincpu->pc(), offset*2, m_control_word);
	/* Read from 68k RAM */
	if ((m_control_word&0x1f)==0x18)
	{
#if 0
		// hack to make roundup 5 boot
		// doesn't seem necessary anymore, left for reference
		if (m_maincpu->pc()==0xec575)
		{
			uint8_t *dst = m_mainregion->base();
			dst[BYTE_XOR_LE(0xec57a)]=0x46;
			dst[BYTE_XOR_LE(0xec57b)]=0x46;

			dst[BYTE_XOR_LE(0xfc520)]=0x46; //code that stops cpu after coin counter goes mad..
			dst[BYTE_XOR_LE(0xfc521)]=0x46;
			dst[BYTE_XOR_LE(0xfc522)]=0x46;
			dst[BYTE_XOR_LE(0xfc523)]=0x46;
			dst[BYTE_XOR_LE(0xfc524)]=0x46;
			dst[BYTE_XOR_LE(0xfc525)]=0x46;
		}
#endif

		return m_sharedram[offset & 0x1fff];
	}

	/* Read from 68k ROM */
	offset+=(m_control_word&0x7)*0x8000;

	return rom[offset];
}

void tatsumi_state::tatsumi_v30_68000_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if ((m_control_word&0x1f)!=0x18)
		logerror("68k write in bank %05x\n",m_control_word);

	COMBINE_DATA(&m_sharedram[offset]);
}

/******************************************************************************/

INTERRUPT_GEN_MEMBER(tatsumi_state::v30_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0xc8/4);   /* V30 - VBL */
}



/***************************************************************************/



/***************************************************************************/


void tatsumi_state::init_tatsumi()
{
	tatsumi_reset();
}

/***************************************************************************/
