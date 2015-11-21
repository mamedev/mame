// license:BSD-3-Clause
// copyright-holders:Luca Elia, Olivier Galibert
/***************************************************************************

                      -= IGS011 (Blitter) Based Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)
            code decrypted by   Olivier Galibert


CPU     :   68000
Sound   :   M6295 + Optional FM or ICS2115
Custom  :   IGS011 (blitter, protection)
            IGS012 (protection, optional)
            IGS003 (8255, protection)
NVRAM   :   Battery for main RAM

---------------------------------------------------------------------------
Year + Game                PCB        Sound         Chips
---------------------------------------------------------------------------
95 Da Ban Cheng            NO-T0084-1 M6295         IGS011 8255
95 Long Hu Bang V033C      NO-T0093   M6295         IGS011 8255
95 Long Hu Bang V035C      ?
95 Mj Ryukobou             NO-T0094   M6295         IGS011 8255
95 Dragon World V010C      NO-0105-4  M6295 YM3812  IGS011 IGS003
95 Dragon World V011H      ?
95 Dragon World V020J      ?          M6295 YM3812  IGS011 IGS003  IGS012
95 Dragon World V021J      ?
95 Dragon World V021O      NO-0105-1  M6295 YM3812  IGS011 IGS003  IGS012
95 Dragon World V030O      NO-0105-1  M6295 YM3812  IGS011 IGS003
97 Dragon World V040O      NO-0105-5  M6295 YM3812  IGS011 IGS003c
96 Virtua Bowling V101XCM  NO-0101-1  ICS2115       IGS011 IGS003e IGS012
96 Virtua Bowling V100JCM  NO-0101-?  ICS2115       IGS011 IGS003e IGS012
96 Long Hu Bang II V185H   NO-0115    M6295 YM2413  IGS011 8255
96 Wan Li Chang Cheng      ?
96 Xing Yen Man Guan       ?
98 Mj Nenrikishu SP V250J  NO-0115-5  M6295 YM2413  IGS011 8255
---------------------------------------------------------------------------

To do:

- Implement the I/O part of IGS003 as an 8255
- IGS003 parametric bitswap protection in nkishusp (instead of patching the roms)
- Interrupt controller at 838000 or a38000 (there's a preliminary implementation for lhb)
- A few graphical bugs

- vbowl, vbowlj: trackball support.
  Wrong colors in "Game Over" screen.

- lhb: in the copyright screen the '5' in '1995' is drawn by the cpu on layer 5,
  but with wrong colors (since the top nibble of the affected pixels is left to 0xf)
  (drgnwrld is like this too, maybe hacked, or a cheap year replacement by IGS)

- dbc: in the title screen the '5' in '1995' is drawn by the cpu with wrong colors.
  (see above comment)
  Also the background palette is wrong since the fade routine is called with wrong
  parameters, but in this case the PCB does the same.

Notes:

- In most games, keep test button pressed during boot for another test mode

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/2413intf.h"
#include "sound/3812intf.h"
#include "sound/ics2115.h"
#include "machine/nvram.h"


struct blitter_t
{
	UINT16  x, y, w, h,
			gfx_lo, gfx_hi,
			depth,
			pen,
			flags;
};

class igs011_state : public driver_device
{
public:
	igs011_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_priority_ram(*this, "priority_ram"),
		m_vbowl_trackball(*this, "vbowl_trackball"),
		m_generic_paletteram_16(*this, "paletteram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<okim6295_device> m_oki;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<UINT16> m_priority_ram;
	optional_shared_ptr<UINT16> m_vbowl_trackball;
	required_shared_ptr<UINT16> m_generic_paletteram_16;

	UINT8 *m_layer[8];
	UINT16 m_priority;
	UINT8 m_lhb2_pen_hi;
	UINT16 m_igs_dips_sel;
	UINT16 m_igs_input_sel;
	UINT16 m_igs_hopper;
	UINT8 m_prot1;
	UINT8 m_prot1_swap;
	UINT32 m_prot1_addr;
	UINT8 m_prot2;
	UINT8 m_igs012_prot;
	UINT8 m_igs012_prot_swap;
	UINT8 m_igs012_prot_mode;
	UINT16 m_igs003_reg[2];
	UINT16 m_lhb_irq_enable;
	blitter_t m_blitter;

	UINT16 m_igs003_prot_hold;
	UINT8 m_igs003_prot_x;
	UINT8 m_igs003_prot_y;
	UINT8 m_igs003_prot_z;
	UINT8 m_igs003_prot_h1;
	UINT8 m_igs003_prot_h2;

	DECLARE_WRITE16_MEMBER(igs011_priority_w);
	DECLARE_READ16_MEMBER(igs011_layers_r);
	DECLARE_WRITE16_MEMBER(igs011_layers_w);
	DECLARE_WRITE16_MEMBER(igs011_palette);
	DECLARE_WRITE16_MEMBER(igs011_blit_x_w);
	DECLARE_WRITE16_MEMBER(igs011_blit_y_w);
	DECLARE_WRITE16_MEMBER(igs011_blit_gfx_lo_w);
	DECLARE_WRITE16_MEMBER(igs011_blit_gfx_hi_w);
	DECLARE_WRITE16_MEMBER(igs011_blit_w_w);
	DECLARE_WRITE16_MEMBER(igs011_blit_h_w);
	DECLARE_WRITE16_MEMBER(igs011_blit_depth_w);
	DECLARE_WRITE16_MEMBER(igs011_blit_pen_w);
	DECLARE_WRITE16_MEMBER(igs011_blit_flags_w);
	DECLARE_WRITE16_MEMBER(igs_dips_w);
	DECLARE_READ16_MEMBER(igs_3_dips_r);
	DECLARE_READ16_MEMBER(igs_4_dips_r);
	DECLARE_READ16_MEMBER(igs_5_dips_r);
	DECLARE_WRITE16_MEMBER(igs011_prot1_w);
	DECLARE_READ16_MEMBER(igs011_prot1_r);
	DECLARE_WRITE16_MEMBER(igs011_prot_addr_w);
	DECLARE_READ16_MEMBER(igs011_prot_fake_r);
	DECLARE_WRITE16_MEMBER(igs011_prot2_reset_w);
	DECLARE_READ16_MEMBER(igs011_prot2_reset_r);
	DECLARE_WRITE16_MEMBER(igs011_prot2_inc_w);
	DECLARE_WRITE16_MEMBER(igs011_prot2_dec_w);
	DECLARE_WRITE16_MEMBER(drgnwrld_igs011_prot2_swap_w);
	DECLARE_WRITE16_MEMBER(lhb_igs011_prot2_swap_w);
	DECLARE_WRITE16_MEMBER(wlcc_igs011_prot2_swap_w);
	DECLARE_WRITE16_MEMBER(vbowl_igs011_prot2_swap_w);
	DECLARE_READ16_MEMBER(drgnwrldv21_igs011_prot2_r);
	DECLARE_READ16_MEMBER(drgnwrldv20j_igs011_prot2_r);
	DECLARE_READ16_MEMBER(lhb_igs011_prot2_r);
	DECLARE_READ16_MEMBER(dbc_igs011_prot2_r);
	DECLARE_READ16_MEMBER(ryukobou_igs011_prot2_r);
	DECLARE_READ16_MEMBER(lhb2_igs011_prot2_r);
	DECLARE_READ16_MEMBER(vbowl_igs011_prot2_r);
	DECLARE_WRITE16_MEMBER(igs012_prot_reset_w);
	DECLARE_READ16_MEMBER(igs012_prot_fake_r);
	DECLARE_WRITE16_MEMBER(igs012_prot_mode_w);
	DECLARE_WRITE16_MEMBER(igs012_prot_inc_w);
	DECLARE_WRITE16_MEMBER(igs012_prot_dec_inc_w);
	DECLARE_WRITE16_MEMBER(igs012_prot_dec_copy_w);
	DECLARE_WRITE16_MEMBER(igs012_prot_copy_w);
	DECLARE_WRITE16_MEMBER(igs012_prot_swap_w);
	DECLARE_READ16_MEMBER(igs012_prot_r);
	DECLARE_WRITE16_MEMBER(drgnwrld_igs003_w);
	DECLARE_READ16_MEMBER(drgnwrld_igs003_r);
	DECLARE_WRITE16_MEMBER(lhb_inputs_w);
	DECLARE_READ16_MEMBER(lhb_inputs_r);
	DECLARE_WRITE16_MEMBER(lhb2_igs003_w);
	DECLARE_READ16_MEMBER(lhb2_igs003_r);
	DECLARE_WRITE16_MEMBER(wlcc_igs003_w);
	DECLARE_READ16_MEMBER(wlcc_igs003_r);
	DECLARE_WRITE16_MEMBER(xymg_igs003_w);
	DECLARE_READ16_MEMBER(xymg_igs003_r);
	DECLARE_WRITE16_MEMBER(vbowl_igs003_w);
	DECLARE_READ16_MEMBER(vbowl_igs003_r);
	DECLARE_WRITE16_MEMBER(lhb_irq_enable_w);
	DECLARE_READ16_MEMBER(vbowl_unk_r);
	DECLARE_WRITE16_MEMBER(vbowl_pen_hi_w);
	DECLARE_WRITE16_MEMBER(vbowl_link_0_w);
	DECLARE_WRITE16_MEMBER(vbowl_link_1_w);
	DECLARE_WRITE16_MEMBER(vbowl_link_2_w);
	DECLARE_WRITE16_MEMBER(vbowl_link_3_w);
	UINT16 igs_dips_r(int NUM);
	DECLARE_CUSTOM_INPUT_MEMBER(igs_hopper_r);
	DECLARE_WRITE16_MEMBER(lhb_okibank_w);
	DECLARE_READ16_MEMBER(ics2115_word_r);
	DECLARE_WRITE16_MEMBER(ics2115_word_w);
	DECLARE_WRITE_LINE_MEMBER(sound_irq);
	DECLARE_DRIVER_INIT(lhbv33c);
	DECLARE_DRIVER_INIT(drgnwrldv21j);
	DECLARE_DRIVER_INIT(wlcc);
	DECLARE_DRIVER_INIT(nkishusp);
	DECLARE_DRIVER_INIT(drgnwrldv21);
	DECLARE_DRIVER_INIT(dbc);
	DECLARE_DRIVER_INIT(lhb);
	DECLARE_DRIVER_INIT(drgnwrld);
	DECLARE_DRIVER_INIT(drgnwrldv30);
	DECLARE_DRIVER_INIT(drgnwrldv11h);
	DECLARE_DRIVER_INIT(lhb2);
	DECLARE_DRIVER_INIT(xymg);
	DECLARE_DRIVER_INIT(drgnwrldv10c);
	DECLARE_DRIVER_INIT(drgnwrldv20j);
	DECLARE_DRIVER_INIT(vbowl);
	DECLARE_DRIVER_INIT(vbowlj);
	DECLARE_DRIVER_INIT(ryukobou);
	TIMER_DEVICE_CALLBACK_MEMBER(lev5_timer_irq_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(lhb_timer_irq_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(lev3_timer_irq_cb);
	virtual void machine_start();
	virtual void video_start();
	UINT32 screen_update_igs011(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_vbowl(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(lhb_vblank_irq);
	void wlcc_decrypt();
	void lhb_decrypt();
	void drgnwrld_type3_decrypt();
	void drgnwrld_type2_decrypt();
	void drgnwrld_type1_decrypt();
	void lhb2_decrypt();
	void nkishusp_decrypt();
	void vbowlj_decrypt();
	void dbc_decrypt();
	void ryukobou_decrypt();
	void lhb2_decrypt_gfx();
	void drgnwrld_gfx_decrypt();
	void prot_mem_range_set();
};


#define LOG_BLITTER 0

/***************************************************************************

    Video

    There are 8 non scrolling layers as big as the screen (512 x 256).
    Each layer has 256 colors and its own palette.

    There are 8 priority codes with RAM associated to each (8 x 256 values).
    For each screen position, to determine which pixel to display, the video
    chip associates a bit to the opacity of that pixel for each layer
    (1 = transparent) to form an address into the selected priority RAM.
    The value at that address (0-7) is the topmost layer.

***************************************************************************/





WRITE16_MEMBER(igs011_state::igs011_priority_w)
{
	COMBINE_DATA(&m_priority);

//  logerror("%06x: priority = %02x\n", space.device().safe_pc(), m_priority);

	if (data & ~0x7)
		logerror("%06x: warning, unknown bits written to priority = %02x\n", space.device().safe_pc(), m_priority);
}


void igs011_state::video_start()
{
	for (int i = 0; i < 8; i++)
	{
		m_layer[i] = auto_alloc_array(machine(), UINT8, 512 * 256);
		save_pointer(NAME(m_layer[i]), 512 * 256, i);
	}

	m_lhb2_pen_hi = 0;

	save_item(NAME(m_priority));
	save_item(NAME(m_lhb2_pen_hi));
	save_item(NAME(m_blitter.x));
	save_item(NAME(m_blitter.y));
	save_item(NAME(m_blitter.w));
	save_item(NAME(m_blitter.h));
	save_item(NAME(m_blitter.gfx_lo));
	save_item(NAME(m_blitter.gfx_hi));
	save_item(NAME(m_blitter.depth));
	save_item(NAME(m_blitter.pen));
	save_item(NAME(m_blitter.flags));
}

UINT32 igs011_state::screen_update_igs011(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
#ifdef MAME_DEBUG
	int layer_enable = -1;
#endif

	int x,y,l,scr_addr,pri_addr;
	UINT16 *pri_ram;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int mask = 0;
		if (machine().input().code_pressed(KEYCODE_Q))  mask |= 0x01;
		if (machine().input().code_pressed(KEYCODE_W))  mask |= 0x02;
		if (machine().input().code_pressed(KEYCODE_E))  mask |= 0x04;
		if (machine().input().code_pressed(KEYCODE_R))  mask |= 0x08;
		if (machine().input().code_pressed(KEYCODE_A))  mask |= 0x10;
		if (machine().input().code_pressed(KEYCODE_S))  mask |= 0x20;
		if (machine().input().code_pressed(KEYCODE_D))  mask |= 0x40;
		if (machine().input().code_pressed(KEYCODE_F))  mask |= 0x80;
		if (mask)   layer_enable &= mask;
	}
#endif

	pri_ram = &m_priority_ram[(m_priority & 7) * 512/2];

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			scr_addr = x + y * 512;
			pri_addr = 0xff;

			for (l = 0; l < 8; l++)
			{
				if (    (m_layer[l][scr_addr] != 0xff)
#ifdef MAME_DEBUG
						&& (layer_enable & (1 << l))
#endif
					)
					pri_addr &= ~(1 << l);
			}


			l   =   pri_ram[pri_addr] & 7;

#ifdef MAME_DEBUG
			if ((layer_enable != -1) && (pri_addr == 0xff))
				bitmap.pix16(y, x) = m_palette->black_pen();
			else
#endif
				bitmap.pix16(y, x) = m_layer[l][scr_addr] | (l << 8);
		}
	}
	return 0;
}

/***************************************************************************

    In addition to the blitter, the CPU can also read from and write to
    the framebuffers for the 8 layers, seen as 0x100000 bytes in memory.
    The first half contains layers 0-3. Layers 4-7 are in the other half.

    The layers are interleaved:

    - bytes 0x00000-0x00003 contain the 1st pixel of layer 0,1,2,3
    - bytes 0x00004-0x00007 contain the 2nd pixel of layer 0,1,2,3
    ...
    - bytes 0x80000-0x80003 contain the 1st pixel of layer 4,5,6,7
    - bytes 0x80004-0x80007 contain the 2nd pixel of layer 4,5,6,7

    and so on.

***************************************************************************/

READ16_MEMBER(igs011_state::igs011_layers_r)
{
	int layer0 = ((offset & (0x80000/2)) ? 4 : 0) + ((offset & 1) ? 0 : 2);

	UINT8 *l0 = m_layer[layer0];
	UINT8 *l1 = m_layer[layer0+1];

	offset >>= 1;
	offset &= 0x1ffff;

	return (l0[offset] << 8) | l1[offset];
}

WRITE16_MEMBER(igs011_state::igs011_layers_w)
{
	UINT16 word;

	int layer0 = ((offset & (0x80000/2)) ? 4 : 0) + ((offset & 1) ? 0 : 2);

	UINT8 *l0 = m_layer[layer0];
	UINT8 *l1 = m_layer[layer0+1];

	offset >>= 1;
	offset &= 0x1ffff;

	word = (l0[offset] << 8) | l1[offset];
	COMBINE_DATA(&word);
	l0[offset] = word >> 8;
	l1[offset] = word;
}

/***************************************************************************

    Palette (r5g5b5)

    offset + 0x000: xRRRRRGG
    offset + 0x800: GGGBBBBB

***************************************************************************/

WRITE16_MEMBER(igs011_state::igs011_palette)
{
	int rgb;

	COMBINE_DATA(&m_generic_paletteram_16[offset]);

	rgb = (m_generic_paletteram_16[offset & 0x7ff] & 0xff) | ((m_generic_paletteram_16[offset | 0x800] & 0xff) << 8);
	m_palette->set_pen_color(offset & 0x7ff,pal5bit(rgb >> 0),pal5bit(rgb >> 5),pal5bit(rgb >> 10));
}

/***************************************************************************

    Blitter

***************************************************************************/


WRITE16_MEMBER(igs011_state::igs011_blit_x_w)
{
	struct blitter_t &blitter = m_blitter;
	COMBINE_DATA(&blitter.x);
}

WRITE16_MEMBER(igs011_state::igs011_blit_y_w)
{
	struct blitter_t &blitter = m_blitter;
	COMBINE_DATA(&blitter.y);
}

WRITE16_MEMBER(igs011_state::igs011_blit_gfx_lo_w)
{
	struct blitter_t &blitter = m_blitter;
	COMBINE_DATA(&blitter.gfx_lo);
}

WRITE16_MEMBER(igs011_state::igs011_blit_gfx_hi_w)
{
	struct blitter_t &blitter = m_blitter;
	COMBINE_DATA(&blitter.gfx_hi);
}

WRITE16_MEMBER(igs011_state::igs011_blit_w_w)
{
	struct blitter_t &blitter = m_blitter;
	COMBINE_DATA(&blitter.w);
}

WRITE16_MEMBER(igs011_state::igs011_blit_h_w)
{
	struct blitter_t &blitter = m_blitter;
	COMBINE_DATA(&blitter.h);
}

WRITE16_MEMBER(igs011_state::igs011_blit_depth_w)
{
	struct blitter_t &blitter = m_blitter;
	COMBINE_DATA(&blitter.depth);
}

WRITE16_MEMBER(igs011_state::igs011_blit_pen_w)
{
	struct blitter_t &blitter = m_blitter;
	COMBINE_DATA(&blitter.pen);
}


WRITE16_MEMBER(igs011_state::igs011_blit_flags_w)
{
	struct blitter_t &blitter = m_blitter;
	int x, xstart, xend, xinc, flipx;
	int y, ystart, yend, yinc, flipy;
	int depth4, clear, opaque, z;
	UINT8 trans_pen, clear_pen, pen_hi, *dest;
	UINT8 pen = 0;

	UINT8 *gfx      =   memregion("blitter")->base();
	UINT8 *gfx2     =   memregion("blitter_hi")->base();
	int gfx_size    =   memregion("blitter")->bytes();
	int gfx2_size   =   memregion("blitter_hi")->bytes();

	const rectangle &clip = m_screen->visible_area();

	COMBINE_DATA(&blitter.flags);

#if LOG_BLITTER
	logerror("%06x: blit x %03x, y %03x, w %03x, h %03x, gfx %03x%04x, depth %02x, pen %02x, flags %03x\n", space.device().safe_pc(),
					blitter.x,blitter.y,blitter.w,blitter.h,blitter.gfx_hi,blitter.gfx_lo,blitter.depth,blitter.pen,blitter.flags);
#endif

	dest    =   m_layer[   blitter.flags & 0x0007   ];
	opaque  =            !(blitter.flags & 0x0008);
	clear   =              blitter.flags & 0x0010;
	flipx   =              blitter.flags & 0x0020;
	flipy   =              blitter.flags & 0x0040;
	if                  (!(blitter.flags & 0x0400))
		return;

	pen_hi  =   (m_lhb2_pen_hi & 0x07) << 5;

	// pixel address
	z       =   blitter.gfx_lo  + (blitter.gfx_hi << 16);

	// what were they smoking???
	depth4  =   !((blitter.flags & 0x7) < (4 - (blitter.depth & 0x7))) ||
				(z & 0x800000);     // see lhb2

	z &= 0x7fffff;

	if (depth4)
	{
		z   *=  2;
		if (gfx2 && (blitter.gfx_hi & 0x80))    trans_pen = 0x1f;   // lhb2
		else                                    trans_pen = 0x0f;

		clear_pen = blitter.pen | 0xf0;
	}
	else
	{
		if (gfx2)   trans_pen = 0x1f;   // vbowl
		else        trans_pen = 0xff;

		clear_pen = blitter.pen;
	}

	xstart = (blitter.x & 0x1ff) - (blitter.x & 0x200);
	ystart = (blitter.y & 0x0ff) - (blitter.y & 0x100);

	if (flipx)  { xend = xstart - (blitter.w & 0x1ff) - 1;  xinc = -1; }
	else        { xend = xstart + (blitter.w & 0x1ff) + 1;  xinc =  1; }

	if (flipy)  { yend = ystart - (blitter.h & 0x0ff) - 1;  yinc = -1; }
	else        { yend = ystart + (blitter.h & 0x0ff) + 1;  yinc =  1; }

	for (y = ystart; y != yend; y += yinc)
	{
		for (x = xstart; x != xend; x += xinc)
		{
			// fetch the pixel
			if (!clear)
			{
				if (depth4)     pen = (gfx[(z/2)%gfx_size] >> ((z&1)?4:0)) & 0x0f;
				else            pen = gfx[z%gfx_size];

				if ( gfx2 )
				{
					pen &= 0x0f;
					if ( gfx2[(z/8)%gfx2_size] & (1 << (z & 7)) )
						pen |= 0x10;
				}
			}

			// plot it
			if (clip.contains(x, y))
			{
				if      (clear)             dest[x + y * 512] = clear_pen;
				else if (pen != trans_pen)  dest[x + y * 512] = pen | pen_hi;
				else if (opaque)            dest[x + y * 512] = 0xff;
			}

			z++;
		}
	}

	#ifdef MAME_DEBUG
#if 1
	if (machine().input().code_pressed(KEYCODE_Z))
	{   char buf[20];
		sprintf(buf, "%02X%02X",blitter.depth,blitter.flags&0xff);
//      ui_draw_text(buf, blitter.x, blitter.y);    // crashes mame!
	}
#endif
	#endif
}

/***************************************************************************

    Common functions

***************************************************************************/

void igs011_state::machine_start()
{
	save_item(NAME(m_igs_dips_sel));
	save_item(NAME(m_igs_input_sel));
	save_item(NAME(m_igs_hopper));
	save_item(NAME(m_prot1));
	save_item(NAME(m_prot1_swap));
	save_item(NAME(m_prot1_addr));
	save_item(NAME(m_prot2));
	save_item(NAME(m_igs012_prot));
	save_item(NAME(m_igs012_prot_swap));
	save_item(NAME(m_igs012_prot_mode));
	save_item(NAME(m_igs003_reg));
	save_item(NAME(m_lhb_irq_enable));
	save_item(NAME(m_igs003_prot_hold));
	save_item(NAME(m_igs003_prot_x));
	save_item(NAME(m_igs003_prot_y));
	save_item(NAME(m_igs003_prot_z));
	save_item(NAME(m_igs003_prot_h1));
	save_item(NAME(m_igs003_prot_h2));
}

// Inputs


CUSTOM_INPUT_MEMBER(igs011_state::igs_hopper_r)
{
	return (m_igs_hopper && ((m_screen->frame_number()/5)&1)) ? 0x0000 : 0x0001;
}

WRITE16_MEMBER(igs011_state::igs_dips_w)
{
	COMBINE_DATA(&m_igs_dips_sel);
}

UINT16 igs011_state::igs_dips_r(int NUM)
{
	int i;
	UINT16 ret=0;
	static const char *const dipnames[] = { "DSW1", "DSW2", "DSW3", "DSW4", "DSW5" };

	for (i = 0; i < NUM; i++)
		if ((~m_igs_dips_sel) & (1 << i) )
			ret = ioport(dipnames[i])->read();

	// 0x0100 is blitter busy
	return  (ret & 0xff) | 0x0000;
}

// Games have 3 to 5 dips
READ16_MEMBER(igs011_state::igs_3_dips_r){ return igs_dips_r(3); }
READ16_MEMBER(igs011_state::igs_4_dips_r){ return igs_dips_r(4); }
READ16_MEMBER(igs011_state::igs_5_dips_r){ return igs_dips_r(5); }

/***************************************************************************

    Code Decryption

***************************************************************************/

void igs011_state::wlcc_decrypt()
{
	int i;
	UINT16 *src = (UINT16 *) (memregion("maincpu")->base());

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		if ((i & 0x2000) == 0x0000 || (i & 0x0004) == 0x0000 || (i & 0x0090) == 0x0000)
			x ^= 0x0004;
		if ((i & 0x0100) == 0x0100 || (i & 0x0040) == 0x0040 || (i & 0x0012) == 0x0012)
			x ^= 0x0020;
		if ((i & 0x2400) == 0x0000 || (i & 0x4100) == 0x4100 || ((i & 0x2000) == 0x2000 && (i & 0x0c00) != 0x0000))
			x ^= 0x0200;
		if ((x & 0x0024) == 0x0004 || (x & 0x0024) == 0x0020)
			x ^= 0x0024;
		src[i] = x;
	}
}


void igs011_state::lhb_decrypt()
{
	int i;
	UINT16 *src = (UINT16 *) (memregion("maincpu")->base());

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		if ((i & 0x1100) != 0x0100)
			x ^= 0x0200;

		if ((i & 0x0150) != 0x0000 && (i & 0x0152) != 0x0010)
			x ^= 0x0004;

		if ((i & 0x2084) != 0x2084 && (i & 0x2094) != 0x2014)
			x ^= 0x0020;

		src[i] = x;
	}
}


void igs011_state::drgnwrld_type3_decrypt()
{
	int i;
	UINT16 *src = (UINT16 *) (memregion("maincpu")->base());

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		if ((i & 0x2000) == 0x0000 || (i & 0x0004) == 0x0000 || (i & 0x0090) == 0x0000)
			x ^= 0x0004;

		if ((i & 0x0100) == 0x0100 || (i & 0x0040) == 0x0040 || (i & 0x0012) == 0x0012)
			x ^= 0x0020;

		if ((((i & 0x1000) == 0x1000) ^ ((i & 0x0100) == 0x0100))
			|| (i & 0x0880) == 0x0800 || (i & 0x0240) == 0x0240)
				x ^= 0x0200;

		if ((x & 0x0024) == 0x0004 || (x & 0x0024) == 0x0020)
			x ^= 0x0024;

		src[i] = x;
	}
}

void igs011_state::drgnwrld_type2_decrypt()
{
	int i;
	UINT16 *src = (UINT16 *) (memregion("maincpu")->base());

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		if(((i & 0x000090) == 0x000000) || ((i & 0x002004) != 0x002004))
			x ^= 0x0004;

		if((((i & 0x000050) == 0x000000) || ((i & 0x000142) != 0x000000)) && ((i & 0x000150) != 0x000000))
			x ^= 0x0020;

		if(((i & 0x004280) == 0x004000) || ((i & 0x004080) == 0x000000))
			x ^= 0x0200;

		if((i & 0x0011a0) != 0x001000)
			x ^= 0x0200;

		if((i & 0x000180) == 0x000100)
			x ^= 0x0200;

		if((x & 0x0024) == 0x0020 || (x & 0x0024) == 0x0004)
			x ^= 0x0024;

		src[i] = x;
	}
}

void igs011_state::drgnwrld_type1_decrypt()
{
	int i;
	UINT16 *src = (UINT16 *) (memregion("maincpu")->base());

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		if ((i & 0x2000) == 0x0000 || (i & 0x0004) == 0x0000 || (i & 0x0090) == 0x0000)
			x ^= 0x0004;

		if ((i & 0x0100) == 0x0100 || (i & 0x0040) == 0x0040 || (i & 0x0012) == 0x0012)
			x ^= 0x0020;
/*
        if ((((i & 0x1000) == 0x1000) ^ ((i & 0x0100) == 0x0100))
            || (i & 0x0880) == 0x0800 || (i & 0x0240) == 0x0240)
                x ^= 0x0200;
*/
		if ((x & 0x0024) == 0x0004 || (x & 0x0024) == 0x0020)
			x ^= 0x0024;

		src[i] = x;
	}
}


void igs011_state::lhb2_decrypt()
{
	int i,j;
	int rom_size = 0x80000;
	UINT16 *src = (UINT16 *) (memregion("maincpu")->base());
	std::vector<UINT16> result_data(rom_size/2);

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		if ((i & 0x0054) != 0x0000 && (i & 0x0056) != 0x0010)
			x ^= 0x0004;

		if ((i & 0x0204) == 0x0000)
			x ^= 0x0008;

		if ((i & 0x3080) != 0x3080 && (i & 0x3090) != 0x3010)
			x ^= 0x0020;

		j = BITSWAP24(i, 23,22,21,20,19,18,17,16,15,14,13, 8, 11,10, 9, 2, 7,6,5,4,3, 12, 1,0);

		result_data[j] = x;
	}

	memcpy(src,&result_data[0],rom_size);
}


// xor similar to ryukobou (both sets are Japan), address scrambling from lhb2
void igs011_state::nkishusp_decrypt()
{
	int i,j;
	int rom_size = 0x80000;
	UINT16 *src = (UINT16 *) (memregion("maincpu")->base());
	std::vector<UINT16> result_data(rom_size/2);

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		// lhb2 address scrambling
		j = BITSWAP24(i, 23,22,21,20,19,18,17,16,15,14,13, 8, 11,10, 9, 2, 7,6,5,4,3, 12, 1,0);

		// ryukobou xor:

//      if ( (j & 0x00100) && (j & 0x00400) )
//          x ^= 0x0200;

		if ( !(j & 0x00004) || !(j & 0x02000) || (!(j & 0x00080) && !(j & 0x00010) ) )
			x ^= 0x0020;

		if ( (j & 0x00100) || (j & 0x00040) || ( (j & 0x00010)&&(j & 0x00002) ) )
			x ^= 0x00004;

		// additional xor:

		if ( !(j & 0x4000) && (j & 0x1000) && (j & 0x00200) )
			x ^= 0x0008;

		result_data[j] = x;
	}

	memcpy(src,&result_data[0],rom_size);
}


void igs011_state::vbowlj_decrypt()
{
	int i;
	UINT16 *src = (UINT16 *) (memregion("maincpu")->base());

	int rom_size = 0x80000;

	for(i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		if((i & 0x4100) == 0x0100)
			x ^= 0x0200;

		if((i & 0x4000) == 0x4000 && (i & 0x0300) != 0x0100)
			x ^= 0x0200;

		if((i & 0x5700) == 0x5100)
			x ^= 0x0200;

		if((i & 0x5500) == 0x1000)
			x ^= 0x0200;

		if((i & 0x0140) != 0x0000 || (i & 0x0012) == 0x0012)
			x ^= 0x0004;

		if((i & 0x2004) != 0x2004 || (i & 0x0090) == 0x0000)
			x ^= 0x0020;

		src[i] = x;
	}
}


void igs011_state::dbc_decrypt()
{
	int i;
	UINT16 *src = (UINT16 *) (memregion("maincpu")->base());

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		if( i & 0x1000/2 )
		{
			if( ~i & 0x400/2 )
				x ^= 0x0200;
		}

		if( i & 0x4000/2 )
		{
			if( i & 0x100/2 )
			{
				if( ~i & 0x08/2 )
					x ^= 0x0020;
			}
			else
			{
				if( ~i & 0x28/2 )
					x ^= 0x0020;
			}
		}
		else
		{
			x ^= 0x0020;
		}

		if( i & 0x200/2 )
		{
			x ^= 0x0004;
		}
		else
		{
			if( (i & 0x80/2) == 0x80/2 || (i & 0x24/2) == 0x24/2 )
				x ^= 0x0004;
		}

		src[i] = x;
	}
}


void igs011_state::ryukobou_decrypt()
{
	int i;
	UINT16 *src = (UINT16 *) memregion("maincpu")->base();
	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		if ( (i & 0x00100) && (i & 0x00400) )
			x ^= 0x0200;

		if ( !(i & 0x00004) || !(i & 0x02000) || (!(i & 0x00080) && !(i & 0x00010) ) )
			x ^= 0x0020;

		if ( (i & 0x00100) || (i & 0x00040) || ( (i & 0x00010)&&(i & 0x00002) ) )
			x ^= 0x00004;

		src[i] = x;
	}
}


/***************************************************************************

    Gfx Decryption

***************************************************************************/


void igs011_state::lhb2_decrypt_gfx()
{
	int i;
	unsigned rom_size = 0x200000;
	UINT8 *src = (UINT8 *) (memregion("blitter")->base());
	dynamic_buffer result_data(rom_size);

	for (i=0; i<rom_size; i++)
		result_data[i] = src[BITSWAP24(i, 23,22,21,20, 19, 17,16,15, 13,12, 10,9,8,7,6,5,4, 2,1, 3, 11, 14, 18, 0)];

	memcpy(src,&result_data[0],rom_size);
}

void igs011_state::drgnwrld_gfx_decrypt()
{
	int i;
	unsigned rom_size = 0x400000;
	UINT8 *src = (UINT8 *) (memregion("blitter")->base());
	dynamic_buffer result_data(rom_size);

	for (i=0; i<rom_size; i++)
		result_data[i] = src[BITSWAP24(i, 23,22,21,20,19,18,17,16,15, 12, 13, 14, 11,10,9,8,7,6,5,4,3,2,1,0)];

	memcpy(src,&result_data[0],rom_size);
}



/***************************************************************************

    IGS011 Protection

    Protection 1 ("ASIC11 CHECK PORT ERROR")

    The chip holds an internal value, a buffered value and an address base register.
    The address base register determines where the protection device is mapped in memory
    (0x00000-0xffff0), has itself a fixed address, and writes to it reset the state.
    The internal and buffered value are manipulated by issuing commands, where
    each command is assigned a specific offset, and is triggered by writing a specific
    byte value to that offset:

    Offs.   R/W     Result
    0         W     COPY: copy buffer to value
    2         W     INC:  increment value
    4         W     DEC:  decrement value
    6         W     SWAP: write bitswap1(value) to buffer
    8       R       READ: read bitswap2(value). Only 2 bits are checked (bitmask 0x24).

    Protection 2 ("CHECK PORT ERROR")

    This is probably not part of the IGS011 nor the IGS012, but a game specific protection
    similar to the above.

    The chip holds an internal value. It is manipulated by issuing commands,
    where each command is assigned a specific address range (fixed per game), and is
    triggered by writing a specific byte value to that range. Possible commands:

    - INC:   increment value
    - DEC:   decrement value
    - SWAP:  value = bitswap1(value). Bitswap1 is game specific.
    - RESET: value = 0

    The protection value is read from an additional address range:
    - READ:  read bitswap2(value). Only 1 bit is checked. Bitswap2 is game specific.

***************************************************************************/



WRITE16_MEMBER(igs011_state::igs011_prot1_w)
{
	offset *= 2;

	switch (offset)
	{
		case 0: // COPY
			if (ACCESSING_BITS_8_15 && (data & 0xff00) == 0x3300)
			{
				m_prot1 = m_prot1_swap;
				return;
			}
			break;

		case 2: // INC
			if (ACCESSING_BITS_8_15 && (data & 0xff00) == 0xff00)
			{
				m_prot1++;
				return;
			}
			break;

		case 4: // DEC
			if (ACCESSING_BITS_8_15 && (data & 0xff00) == 0xaa00)
			{
				m_prot1--;
				return;
			}
			break;

		case 6: // SWAP
			if (ACCESSING_BITS_8_15 && (data & 0xff00) == 0x5500)
			{
				// b1 . (b2|b3) . b2 . (b0&b3)
				UINT8 x = m_prot1;
				m_prot1_swap = (BIT(x,1)<<3) | ((BIT(x,2)|BIT(x,3))<<2) | (BIT(x,2)<<1) | (BIT(x,0)&BIT(x,3));
				return;
			}
			break;
	}

	logerror("%s: warning, unknown igs011_prot1_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}
READ16_MEMBER(igs011_state::igs011_prot1_r)
{
	// !(b1&b2) . 0 . 0 . (b0^b3) . 0 . 0
	UINT8 x = m_prot1;
	return (((BIT(x,1)&BIT(x,2))^1)<<5) | ((BIT(x,0)^BIT(x,3))<<2);
}


WRITE16_MEMBER(igs011_state::igs011_prot_addr_w)
{
	m_prot1 = 0x00;
	m_prot1_swap = 0x00;

//  m_prot2 = 0x00;

	address_space &sp = m_maincpu->space(AS_PROGRAM);
	UINT8 *rom = memregion("maincpu")->base();

	// Plug previous address range with ROM access
	sp.install_rom(m_prot1_addr + 0, m_prot1_addr + 9, rom + m_prot1_addr);

	m_prot1_addr = (data << 4) ^ 0x8340;

	prot_mem_range_set();
}

void igs011_state::prot_mem_range_set()
{
	address_space &sp = m_maincpu->space(AS_PROGRAM);

	// Add protection memory range
	sp.install_write_handler(m_prot1_addr + 0, m_prot1_addr + 7, write16_delegate(FUNC(igs011_state::igs011_prot1_w), this));
	sp.install_read_handler (m_prot1_addr + 8, m_prot1_addr + 9, read16_delegate(FUNC(igs011_state::igs011_prot1_r), this));
}
/*
READ16_MEMBER(igs011_state::igs011_prot_fake_r)
{
    switch (offset)
    {
        case 0: return m_prot1;
        case 1: return m_prot1_swap;
        case 2: return m_prot2;
    }
    return 0;
}
*/






// Prot2

// drgnwrld (33)
WRITE16_MEMBER(igs011_state::igs011_prot2_reset_w)
{
	m_prot2 = 0x00;
}

// wlcc
READ16_MEMBER(igs011_state::igs011_prot2_reset_r)
{
	m_prot2 = 0x00;
	return 0;
}



// lhb2 (55), lhb/dbc/ryukobou (33)
WRITE16_MEMBER(igs011_state::igs011_prot2_inc_w)
{
//  if ( (ACCESSING_BITS_8_15 && (data & 0xff00) == 0x5500) || ((ACCESSING_BITS_0_7 && (data & 0x00ff) == 0x0055)) )
	{
		m_prot2++;
	}
//  else
//      logerror("%s: warning, unknown igs011_prot2_inc_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}

// vbowl (33)
WRITE16_MEMBER(igs011_state::igs011_prot2_dec_w)
{
//  if ( (ACCESSING_BITS_8_15 && (data & 0xff00) == 0x3300) || ((ACCESSING_BITS_0_7 && (data & 0x00ff) == 0x0033)) )
	{
		m_prot2--;
	}
//  else
//      logerror("%s: warning, unknown igs011_prot2_dec_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}



WRITE16_MEMBER(igs011_state::drgnwrld_igs011_prot2_swap_w)
{
	offset *= 2;

//  if ( (ACCESSING_BITS_8_15 && (data & 0xff00) == 0x3300) || ((ACCESSING_BITS_0_7 && (data & 0x00ff) == 0x0033)) )
	{
		// (b3&b0) . b2 . (b0|b1) . (b2^!b4) . (!b1^b3)
		UINT8 x = m_prot2;
		m_prot2 = ((BIT(x,3)&BIT(x,0))<<4) | (BIT(x,2)<<3) | ((BIT(x,0)|BIT(x,1))<<2) | ((BIT(x,2)^BIT(x,4)^1)<<1) | (BIT(x,1)^1^BIT(x,3));
	}
//  else
//      logerror("%s: warning, unknown igs011_prot2_swap_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}

// lhb, xymg, lhb2
WRITE16_MEMBER(igs011_state::lhb_igs011_prot2_swap_w)
{
	offset *= 2;

//  if ( (ACCESSING_BITS_8_15 && (data & 0xff00) == 0x3300) || ((ACCESSING_BITS_0_7 && (data & 0x00ff) == 0x0033)) )
	{
		// (!b0|b1) . b2 . (b0&b1)
		UINT8 x = m_prot2;
		m_prot2 = (((BIT(x,0)^1)|BIT(x,1))<<2) | (BIT(x,2)<<1) | (BIT(x,0)&BIT(x,1));
	}
//  else
//      logerror("%s: warning, unknown igs011_prot2_swap_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}

// wlcc
WRITE16_MEMBER(igs011_state::wlcc_igs011_prot2_swap_w)
{
	offset *= 2;

//  if ( (ACCESSING_BITS_8_15 && (data & 0xff00) == 0x3300) || ((ACCESSING_BITS_0_7 && (data & 0x00ff) == 0x0033)) )
	{
		// (b3 ^ b2) . (b2 ^ b1) . (b1 ^ b0) . !(b4 ^ b0) . !(b4 ^ b3)
		UINT8 x = m_prot2;
		m_prot2 = ((BIT(x,3)^BIT(x,2))<<4) | ((BIT(x,2)^BIT(x,1))<<3) | ((BIT(x,1)^BIT(x,0))<<2) | ((BIT(x,4)^BIT(x,0)^1)<<1) | (BIT(x,4)^BIT(x,3)^1);
	}
//  else
//      logerror("%s: warning, unknown igs011_prot2_swap_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}

// vbowl
WRITE16_MEMBER(igs011_state::vbowl_igs011_prot2_swap_w)
{
	offset *= 2;

//  if ( (ACCESSING_BITS_8_15 && (data & 0xff00) == 0x3300) || ((ACCESSING_BITS_0_7 && (data & 0x00ff) == 0x0033)) )
	{
		// (b3 ^ b2) . (b2 ^ b1) . (b1 ^ b0) . (b4 ^ b0) . (b4 ^ b3)
		UINT8 x = m_prot2;
		m_prot2 = ((BIT(x,3)^BIT(x,2))<<4) | ((BIT(x,2)^BIT(x,1))<<3) | ((BIT(x,1)^BIT(x,0))<<2) | ((BIT(x,4)^BIT(x,0))<<1) | (BIT(x,4)^BIT(x,3));
	}
//  else
//      logerror("%s: warning, unknown igs011_prot2_swap_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}



// drgnwrld
READ16_MEMBER(igs011_state::drgnwrldv21_igs011_prot2_r)
{
	// b9 = (!b4) | (!b0 & b2) | (!(b3 ^ b1) & !(!(b4 & b0) | b2))
	UINT8 x = m_prot2;
	UINT8 b9 = (BIT(x,4)^1) | ((BIT(x,0)^1) & BIT(x,2)) | ( (BIT(x,3)^BIT(x,1)^1) & ((((BIT(x,4)^1) & BIT(x,0)) | BIT(x,2))^1) );
	return (b9 << 9);
}
READ16_MEMBER(igs011_state::drgnwrldv20j_igs011_prot2_r)
{
	// b9 = (!b4 | !b0) | !(b3 | b1) | !(b2 & b0)
	UINT8 x = m_prot2;
	UINT8 b9 = ((BIT(x,4)^1) | (BIT(x,0)^1)) | ((BIT(x,3) | BIT(x,1))^1) | ((BIT(x,2) & BIT(x,0))^1);
	return (b9 << 9);
}

// lhb, xymg
READ16_MEMBER(igs011_state::lhb_igs011_prot2_r)
{
	// b9 = !b2 | (b1 & b0)
	UINT8 x = m_prot2;
	UINT8 b9 = (BIT(x,2)^1) | (BIT(x,1) & BIT(x,0));
	return (b9 << 9);
}

// dbc
READ16_MEMBER(igs011_state::dbc_igs011_prot2_r)
{
	// b9 = !b1 | (!b0 & b2)
	UINT8 x = m_prot2;
	UINT8 b9 = (BIT(x,1)^1) | ((BIT(x,0)^1) & BIT(x,2));
	return (b9 << 9);
}

// ryukobou
READ16_MEMBER(igs011_state::ryukobou_igs011_prot2_r)
{
	// b9 = (!b1 | b2) & b0
	UINT8 x = m_prot2;
	UINT8 b9 = ((BIT(x,1)^1) | BIT(x,2)) & BIT(x,0);
	return (b9 << 9);
}

// lhb2
READ16_MEMBER(igs011_state::lhb2_igs011_prot2_r)
{
	// b3 = !b2 | !b1 | b0
	UINT8 x = m_prot2;
	UINT8 b3 = (BIT(x,2)^1) | (BIT(x,1)^1) | BIT(x,0);
	return (b3 << 3);
}

// vbowl
READ16_MEMBER(igs011_state::vbowl_igs011_prot2_r)
{
	UINT8 x = m_prot2;
	UINT8 b9 = ((BIT(x,4)^1) & (BIT(x,3)^1)) | ((BIT(x,2) & BIT(x,1))^1) | ((BIT(x,4) | BIT(x,0))^1);
	return (b9 << 9);
}

/***************************************************************************

    IGS012 Protection ("ASIC12 CHECK PORT ERROR")

    The chip holds an internal value, a buffered value and a mode.
    These are manipulated by issuing commands, where each command is assigned
    a specific address range, and is triggered by writing a specific byte value
    to that range. Possible commands:

    - INC:   increment value
    - DEC:   decrement value
    - SWAP:  write bitswap1(value) to buffer
    - COPY:  copy buffer to value
    - MODE:  toggle mode (toggles address ranges to write/read and byte values to write)
    - RESET: value = 0, mode = 0

    The protection value is read from an additional address range:
    - READ: read bitswap2(value). Only 2 bits are checked.

***************************************************************************/


WRITE16_MEMBER(igs011_state::igs012_prot_reset_w)
{
	m_igs012_prot = 0x00;
	m_igs012_prot_swap = 0x00;

	m_igs012_prot_mode = 0;
}
/*
READ16_MEMBER(igs011_state::igs012_prot_fake_r)
{
    switch (offset)
    {
        case 0: return m_igs012_prot;
        case 1: return m_igs012_prot_swap;
        case 2: return m_igs012_prot_mode;
    }
    return 0;
}
*/

// Macro that checks whether the current mode and data byte written match the arguments
#define MODE_AND_DATA(_MODE,_DATA)  (m_igs012_prot_mode == (_MODE) && ( (ACCESSING_BITS_8_15 && (data & 0xff00) == ((_DATA)<<8)) || (ACCESSING_BITS_0_7 && ((data & 0x00ff) == (_DATA))) ) )

WRITE16_MEMBER(igs011_state::igs012_prot_mode_w)
{
	if ( MODE_AND_DATA(0, 0xcc) || MODE_AND_DATA(1, 0xdd) )
	{
		m_igs012_prot_mode = m_igs012_prot_mode ^ 1;
	}
	else
		logerror("%s: warning, unknown igs012_prot_mode_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_igs012_prot_mode);
}

WRITE16_MEMBER(igs011_state::igs012_prot_inc_w)
{
	if ( MODE_AND_DATA(0, 0xff) )
	{
		m_igs012_prot = (m_igs012_prot + 1) & 0x1f;
	}
	else
		logerror("%s: warning, unknown igs012_prot_inc_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_igs012_prot_mode);
}

WRITE16_MEMBER(igs011_state::igs012_prot_dec_inc_w)
{
	if ( MODE_AND_DATA(0, 0xaa) )
	{
		m_igs012_prot = (m_igs012_prot - 1) & 0x1f;
	}
	else if ( MODE_AND_DATA(1, 0xfa) )
	{
		m_igs012_prot = (m_igs012_prot + 1) & 0x1f;
	}
	else
		logerror("%s: warning, unknown igs012_prot_dec_inc_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_igs012_prot_mode);
}

WRITE16_MEMBER(igs011_state::igs012_prot_dec_copy_w)
{
	if ( MODE_AND_DATA(0, 0x33) )
	{
		m_igs012_prot = m_igs012_prot_swap;
	}
	else if ( MODE_AND_DATA(1, 0x5a) )
	{
		m_igs012_prot = (m_igs012_prot - 1) & 0x1f;
	}
	else
		logerror("%s: warning, unknown igs012_prot_dec_copy_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_igs012_prot_mode);
}

WRITE16_MEMBER(igs011_state::igs012_prot_copy_w)
{
	if ( MODE_AND_DATA(1, 0x22) )
	{
		m_igs012_prot = m_igs012_prot_swap;
	}
	else
		logerror("%s: warning, unknown igs012_prot_copy_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_igs012_prot_mode);
}

WRITE16_MEMBER(igs011_state::igs012_prot_swap_w)
{
	if ( MODE_AND_DATA(0, 0x55) || MODE_AND_DATA(1, 0xa5) )
	{
		// !(3 | 1)..(2 & 1)..(3 ^ 0)..(!2)
		UINT8 x = m_igs012_prot;
		m_igs012_prot_swap = (((BIT(x,3)|BIT(x,1))^1)<<3) | ((BIT(x,2)&BIT(x,1))<<2) | ((BIT(x,3)^BIT(x,0))<<1) | (BIT(x,2)^1);
	}
	else
		logerror("%s: warning, unknown igs012_prot_swap_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_igs012_prot_mode);
}

READ16_MEMBER(igs011_state::igs012_prot_r)
{
	// FIXME: mode 0 and mode 1 are mapped to different memory ranges
	UINT8 x = m_igs012_prot;

	UINT8 b1 = (BIT(x,3) | BIT(x,1))^1;
	UINT8 b0 = BIT(x,3) ^ BIT(x,0);

	return (b1 << 1) | (b0 << 0);
}

/***************************************************************************

    IGS003 (8255 I/O + Protection)

***************************************************************************/


WRITE16_MEMBER(igs011_state::drgnwrld_igs003_w)
{
	COMBINE_DATA(&m_igs003_reg[offset]);

	if (offset == 0)
		return;

	switch(m_igs003_reg[0])
	{
		case 0x00:
			if (ACCESSING_BITS_0_7)
				coin_counter_w(machine(), 0,data & 2);

			if (data & ~0x2)
				logerror("%06x: warning, unknown bits written in coin counter = %02x\n", space.device().safe_pc(), data);

			break;

//      case 0x01:
		// 0,1,4 written

//      case 0x03:
//      case 0x04:
//      case 0x05:

		default:
//          popmessage("igs003 %x <- %04x",m_igs003_reg[0],data);
			logerror("%06x: warning, writing to igs003_reg %02x = %02x\n", space.device().safe_pc(), m_igs003_reg[0], data);
	}
}
READ16_MEMBER(igs011_state::drgnwrld_igs003_r)
{
	switch(m_igs003_reg[0])
	{
		case 0x00:  return ioport("IN0")->read();
		case 0x01:  return ioport("IN1")->read();
		case 0x02:  return ioport("IN2")->read();

		case 0x20:  return 0x49;
		case 0x21:  return 0x47;
		case 0x22:  return 0x53;

		case 0x24:  return 0x41;
		case 0x25:  return 0x41;
		case 0x26:  return 0x7f;
		case 0x27:  return 0x41;
		case 0x28:  return 0x41;

		case 0x2a:  return 0x3e;
		case 0x2b:  return 0x41;
		case 0x2c:  return 0x49;
		case 0x2d:  return 0xf9;
		case 0x2e:  return 0x0a;

		case 0x30:  return 0x26;
		case 0x31:  return 0x49;
		case 0x32:  return 0x49;
		case 0x33:  return 0x49;
		case 0x34:  return 0x32;

		default:
			logerror("%06x: warning, reading with igs003_reg = %02x\n", space.device().safe_pc(), m_igs003_reg[0]);
	}

	return 0;
}



WRITE16_MEMBER(igs011_state::lhb_inputs_w)
{
	COMBINE_DATA(&m_igs_input_sel);

	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(machine(), 0,    data & 0x20 );
		//  coin out        data & 0x40
		m_igs_hopper        =   data & 0x80;
	}

	if ( m_igs_input_sel & (~0xff) )
		logerror("%06x: warning, unknown bits written in igs_input_sel = %02x\n", space.device().safe_pc(), m_igs_input_sel);

//  popmessage("sel2 %02x",m_igs_input_sel&~0x1f);
}
READ16_MEMBER(igs011_state::lhb_inputs_r)
{
	switch(offset)
	{
		case 0:     return m_igs_input_sel;

		case 1:
			if (~m_igs_input_sel & 0x01)    return ioport("KEY0")->read();
			if (~m_igs_input_sel & 0x02)    return ioport("KEY1")->read();
			if (~m_igs_input_sel & 0x04)    return ioport("KEY2")->read();
			if (~m_igs_input_sel & 0x08)    return ioport("KEY3")->read();
			if (~m_igs_input_sel & 0x10)    return ioport("KEY4")->read();

			logerror("%06x: warning, reading with igs_input_sel = %02x\n", space.device().safe_pc(), m_igs_input_sel);
			break;
	}
	return 0;
}



WRITE16_MEMBER(igs011_state::lhb2_igs003_w)
{
	COMBINE_DATA(&m_igs003_reg[offset]);

	if (offset == 0)
		return;

	switch(m_igs003_reg[0])
	{
		case 0x00:
			COMBINE_DATA(&m_igs_input_sel);

			if (ACCESSING_BITS_0_7)
			{
				coin_counter_w(machine(), 0,    data & 0x20);
				//  coin out        data & 0x40
				m_igs_hopper        =   data & 0x80;
			}

			if ( m_igs_input_sel & ~0x7f )
				logerror("%06x: warning, unknown bits written in igs_input_sel = %02x\n", space.device().safe_pc(), m_igs_input_sel);

//          popmessage("sel2 %02x",m_igs_input_sel&~0x1f);
			break;

		case 0x02:
			if (ACCESSING_BITS_0_7)
			{
				m_lhb2_pen_hi = data & 0x07;

				m_oki->set_bank_base((data & 0x08) ? 0x40000 : 0);
			}

			if ( m_lhb2_pen_hi & ~0xf )
				logerror("%06x: warning, unknown bits written in lhb2_pen_hi = %02x\n", space.device().safe_pc(), m_lhb2_pen_hi);

//          popmessage("oki %02x",m_lhb2_pen_hi & 0x08);
			break;

		case 0x40:
			m_igs003_prot_h2 = m_igs003_prot_h1;
			m_igs003_prot_h1 = data;
		break;

		case 0x41: // data written here is the same as reg 40
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
		break;

		case 0x48:
			m_igs003_prot_x = 0; // not 100% sure about these
			if(!(m_igs003_prot_h2 & 0x0a)) m_igs003_prot_x |= 0x08; // $100de6 - not used?
			if(!(m_igs003_prot_h2 & 0x90)) m_igs003_prot_x |= 0x04; // $100de7 - not used?
			if(!(m_igs003_prot_h1 & 0x02)) m_igs003_prot_x |= 0x02; // $100de8
			if(!(m_igs003_prot_h1 & 0x80)) m_igs003_prot_x |= 0x01; // $100de9
		break;

		case 0x50: // reset?
			m_igs003_prot_hold = 0;
		break;

		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
			{
				UINT16 old;

				m_igs003_prot_y = m_igs003_reg[0] & 0x07;
				m_igs003_prot_z = data;

				old = m_igs003_prot_hold;

				m_igs003_prot_hold <<= 1;
				m_igs003_prot_hold |= BIT(old, 15); // rotate
				m_igs003_prot_hold ^= 0x2bad;
				m_igs003_prot_hold ^= BIT(m_igs003_prot_z, m_igs003_prot_y);
				m_igs003_prot_hold ^= BIT(old, 12);
				m_igs003_prot_hold ^= BIT(old,  8);
				m_igs003_prot_hold ^= BIT(old,  3);
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 0) <<  4;
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 1) <<  6;
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 2) << 10;
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 3) << 12;
			}
		break;

		default:
			logerror("%06x: warning, writing to igs003_reg %02x = %02x\n", space.device().safe_pc(), m_igs003_reg[0], data);
	}
}
READ16_MEMBER(igs011_state::lhb2_igs003_r)
{
	switch(m_igs003_reg[0])
	{
		case 0x01:
			if (~m_igs_input_sel & 0x01)    return ioport("KEY0")->read();
			if (~m_igs_input_sel & 0x02)    return ioport("KEY1")->read();
			if (~m_igs_input_sel & 0x04)    return ioport("KEY2")->read();
			if (~m_igs_input_sel & 0x08)    return ioport("KEY3")->read();
			if (~m_igs_input_sel & 0x10)    return ioport("KEY4")->read();
			/* fall through */
		default:
			logerror("%06x: warning, reading with igs003_reg = %02x\n", space.device().safe_pc(), m_igs003_reg[0]);
			break;

		case 0x03:
			return BITSWAP16(m_igs003_prot_hold, 14,11,8,6,4,3,1,0, 5,2,9,7,10,13,12,15) & 0xff;

		// Protection:
		// 0544FE: 20 21 22 24 25 26 27 28 2A 2B 2C 2D 2E 30 31 32 33 34
		// 0544EC: 49 47 53 41 41 7F 41 41 3E 41 49 F9 0A 26 49 49 49 32

		case 0x20:  return 0x49;
		case 0x21:  return 0x47;
		case 0x22:  return 0x53;

		case 0x24:  return 0x41;
		case 0x25:  return 0x41;
		case 0x26:  return 0x7f;
		case 0x27:  return 0x41;
		case 0x28:  return 0x41;

		case 0x2a:  return 0x3e;
		case 0x2b:  return 0x41;
		case 0x2c:  return 0x49;
		case 0x2d:  return 0xf9;
		case 0x2e:  return 0x0a;

		case 0x30:  return 0x26;
		case 0x31:  return 0x49;
		case 0x32:  return 0x49;
		case 0x33:  return 0x49;
		case 0x34:  return 0x32;
	}

	return 0;
}



WRITE16_MEMBER(igs011_state::wlcc_igs003_w)
{
	COMBINE_DATA(&m_igs003_reg[offset]);

	if (offset == 0)
		return;

	switch(m_igs003_reg[0])
	{
		case 0x02:
			if (ACCESSING_BITS_0_7)
			{
				coin_counter_w(machine(), 0,    data & 0x01);
				//  coin out        data & 0x02

				m_oki->set_bank_base((data & 0x10) ? 0x40000 : 0);
				m_igs_hopper        =   data & 0x20;
			}

			if (data & ~0x33)
				logerror("%06x: warning, unknown bits written in coin counter = %02x\n", space.device().safe_pc(), data);

//          popmessage("coin %02x",data);
			break;

		default:
			logerror("%06x: warning, writing to igs003_reg %02x = %02x\n", space.device().safe_pc(), m_igs003_reg[0], data);
	}
}
READ16_MEMBER(igs011_state::wlcc_igs003_r)
{
	switch(m_igs003_reg[0])
	{
		case 0x00:  return ioport("IN0")->read();

		case 0x20:  return 0x49;
		case 0x21:  return 0x47;
		case 0x22:  return 0x53;

		case 0x24:  return 0x41;
		case 0x25:  return 0x41;
		case 0x26:  return 0x7f;
		case 0x27:  return 0x41;
		case 0x28:  return 0x41;

		case 0x2a:  return 0x3e;
		case 0x2b:  return 0x41;
		case 0x2c:  return 0x49;
		case 0x2d:  return 0xf9;
		case 0x2e:  return 0x0a;

		case 0x30:  return 0x26;
		case 0x31:  return 0x49;
		case 0x32:  return 0x49;
		case 0x33:  return 0x49;
		case 0x34:  return 0x32;

		default:
			logerror("%06x: warning, reading with igs003_reg = %02x\n", space.device().safe_pc(), m_igs003_reg[0]);
	}

	return 0;
}



WRITE16_MEMBER(igs011_state::xymg_igs003_w)
{
	COMBINE_DATA(&m_igs003_reg[offset]);

	if (offset == 0)
		return;

	switch(m_igs003_reg[0])
	{
		case 0x01:
			COMBINE_DATA(&m_igs_input_sel);

			if (ACCESSING_BITS_0_7)
			{
				coin_counter_w(machine(), 0,    data & 0x20);
				//  coin out        data & 0x40
				m_igs_hopper        =   data & 0x80;
			}

			if ( m_igs_input_sel & 0x40 )
				logerror("%06x: warning, unknown bits written in igs_input_sel = %02x\n", space.device().safe_pc(), m_igs_input_sel);

//          popmessage("sel2 %02x",m_igs_input_sel&~0x1f);
			break;

		default:
			logerror("%06x: warning, writing to igs003_reg %02x = %02x\n", space.device().safe_pc(), m_igs003_reg[0], data);
	}
}
READ16_MEMBER(igs011_state::xymg_igs003_r)
{
	switch(m_igs003_reg[0])
	{
		case 0x00:  return ioport("COIN")->read();

		case 0x02:
			if (~m_igs_input_sel & 0x01)    return ioport("KEY0")->read();
			if (~m_igs_input_sel & 0x02)    return ioport("KEY1")->read();
			if (~m_igs_input_sel & 0x04)    return ioport("KEY2")->read();
			if (~m_igs_input_sel & 0x08)    return ioport("KEY3")->read();
			if (~m_igs_input_sel & 0x10)    return ioport("KEY4")->read();
			/* fall through */

		case 0x20:  return 0x49;
		case 0x21:  return 0x47;
		case 0x22:  return 0x53;

		case 0x24:  return 0x41;
		case 0x25:  return 0x41;
		case 0x26:  return 0x7f;
		case 0x27:  return 0x41;
		case 0x28:  return 0x41;

		case 0x2a:  return 0x3e;
		case 0x2b:  return 0x41;
		case 0x2c:  return 0x49;
		case 0x2d:  return 0xf9;
		case 0x2e:  return 0x0a;

		case 0x30:  return 0x26;
		case 0x31:  return 0x49;
		case 0x32:  return 0x49;
		case 0x33:  return 0x49;
		case 0x34:  return 0x32;

		default:
			logerror("%06x: warning, reading with igs003_reg = %02x\n", space.device().safe_pc(), m_igs003_reg[0]);
			break;
	}

	return 0;
}



WRITE16_MEMBER(igs011_state::vbowl_igs003_w)
{
	COMBINE_DATA(&m_igs003_reg[offset]);

	if (offset == 0)
		return;

	switch(m_igs003_reg[0])
	{
		case 0x02:
			if (ACCESSING_BITS_0_7)
			{
				coin_counter_w(machine(), 0, data & 1);
				coin_counter_w(machine(), 1, data & 2);
			}

			if (data & ~0x3)
				logerror("%06x: warning, unknown bits written in coin counter = %02x\n", space.device().safe_pc(), data);

			break;

		case 0x40:
			m_igs003_prot_h2 = m_igs003_prot_h1;
			m_igs003_prot_h1 = data;
		break;

		case 0x41: // repeat data written at $40
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
		break;

		case 0x48:
			m_igs003_prot_x = 0; // not 100% sure about these
			if(!(m_igs003_prot_h2 & 0x0a)) m_igs003_prot_x |= 0x08;
			if(!(m_igs003_prot_h2 & 0x90)) m_igs003_prot_x |= 0x04;
			if(!(m_igs003_prot_h1 & 0x02)) m_igs003_prot_x |= 0x02;
			if(!(m_igs003_prot_h1 & 0x80)) m_igs003_prot_x |= 0x01;
		break;

		case 0x50: // reset?
			m_igs003_prot_hold = 0;
		break;

		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
			{
				UINT16 old;

				m_igs003_prot_y = m_igs003_reg[0] & 0x07;
				m_igs003_prot_z = data;

				old = m_igs003_prot_hold;

				m_igs003_prot_hold <<= 1;
				m_igs003_prot_hold ^= BIT(old, 15); // rotate
				m_igs003_prot_hold ^= 0x2bad;
				m_igs003_prot_hold ^= BIT(old,  5);
				m_igs003_prot_hold ^= BIT(old,  8);
				m_igs003_prot_hold ^= BIT(old, 10);
				m_igs003_prot_hold ^= BIT(m_igs003_prot_z, m_igs003_prot_y);
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 0) <<  4;
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 1) <<  6;
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 2) << 10;
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 3) << 12;
			}
		break;

		default:
//          popmessage("igs003 %x <- %04x",m_igs003_reg[0],data);
			logerror("%06x: warning, writing to igs003_reg %02x = %02x\n", space.device().safe_pc(), m_igs003_reg[0], data);
	}
}
READ16_MEMBER(igs011_state::vbowl_igs003_r)
{
	switch(m_igs003_reg[0])
	{
		case 0x00:  return ioport("IN0")->read();
		case 0x01:  return ioport("IN1")->read();

		case 0x03:
			return BITSWAP16(m_igs003_prot_hold, 14,11,8,6,4,3,1,0, 5,2,9,7,10,13,12,15) & 0xff;

		case 0x20:  return 0x49;
		case 0x21:  return 0x47;
		case 0x22:  return 0x53;

		case 0x24:  return 0x41;
		case 0x25:  return 0x41;
		case 0x26:  return 0x7f;
		case 0x27:  return 0x41;
		case 0x28:  return 0x41;

		case 0x2a:  return 0x3e;
		case 0x2b:  return 0x41;
		case 0x2c:  return 0x49;
		case 0x2d:  return 0xf9;
		case 0x2e:  return 0x0a;

		case 0x30:  return 0x26;
		case 0x31:  return 0x49;
		case 0x32:  return 0x49;
		case 0x33:  return 0x49;
		case 0x34:  return 0x32;

		default:
			logerror("%06x: warning, reading with igs003_reg = %02x\n", space.device().safe_pc(), m_igs003_reg[0]);
	}

	return 0;
}



/***************************************************************************

    Driver Inits (Decryption, Protection Patches)

***************************************************************************/

// V0400O
DRIVER_INIT_MEMBER(igs011_state,drgnwrld)
{
//  UINT16 *rom = (UINT16 *) memregion("maincpu")->base();

	drgnwrld_type1_decrypt();
	drgnwrld_gfx_decrypt();
/*
    // PROTECTION CHECKS
    rom[0x032ee/2]  =   0x606c;     // 0032EE: 676C        beq 335c     (ASIC11 CHECK PORT ERROR 3)
    rom[0x23d5e/2]  =   0x606c;     // 023D5E: 676C        beq 23dcc    (CHECK PORT ERROR 1)
    rom[0x23fd0/2]  =   0x606c;     // 023FD0: 676C        beq 2403e    (CHECK PORT ERROR 2)
    rom[0x24170/2]  =   0x606c;     // 024170: 676C        beq 241de    (CHECK PORT ERROR 3)
    rom[0x24348/2]  =   0x606c;     // 024348: 676C        beq 243b6    (ASIC11 CHECK PORT ERROR 4)
    rom[0x2454e/2]  =   0x606c;     // 02454E: 676C        beq 245bc    (ASIC11 CHECK PORT ERROR 3)
    rom[0x246cc/2]  =   0x606c;     // 0246CC: 676C        beq 2473a    (ASIC11 CHECK PORT ERROR 2)
    rom[0x24922/2]  =   0x606c;     // 024922: 676C        beq 24990    (ASIC11 CHECK PORT ERROR 1)

    rom[0x24b66/2]  =   0x606c;     // 024B66: 676C        beq 24bd4    (ASIC12 CHECK PORT ERROR 4)
    rom[0x24de2/2]  =   0x606c;     // 024DE2: 676C        beq 24e50    (ASIC12 CHECK PORT ERROR 3)
    rom[0x2502a/2]  =   0x606c;     // 02502A: 676C        beq 25098    (ASIC12 CHECK PORT ERROR 2)
    rom[0x25556/2]  =   0x6000;     // 025556: 6700 E584   beq 23adc    (ASIC12 CHECK PORT ERROR 1)

    rom[0x2a16c/2]  =   0x606c;     // 02A16C: 676C        beq 2a1da    (ASIC11 CHECK PORT ERROR 2)
*/
}

DRIVER_INIT_MEMBER(igs011_state,drgnwrldv30)
{
//  UINT16 *rom = (UINT16 *) memregion("maincpu")->base();

	drgnwrld_type1_decrypt();
	drgnwrld_gfx_decrypt();
/*
    // PROTECTION CHECKS
    rom[0x032ee/2]  =   0x606c;     // 0032EE: 676C        beq 335c     (ASIC11 CHECK PORT ERROR 3)
    rom[0x23d5e/2]  =   0x606c;     // 023D5E: 676C        beq 23dcc    (CHECK PORT ERROR 1)
    rom[0x23fd0/2]  =   0x606c;     // 023FD0: 676C        beq 2403e    (CHECK PORT ERROR 2)
    rom[0x24170/2]  =   0x606c;     // 024170: 676C        beq 241de    (CHECK PORT ERROR 3)
    rom[0x24348/2]  =   0x606c;     // 024348: 676C        beq 243b6    (ASIC11 CHECK PORT ERROR 4)
    rom[0x2454e/2]  =   0x606c;     // 02454E: 676C        beq 245bc    (ASIC11 CHECK PORT ERROR 3)
    rom[0x246cc/2]  =   0x606c;     // 0246CC: 676C        beq 2473a    (ASIC11 CHECK PORT ERROR 2)
    rom[0x24922/2]  =   0x606c;     // 024922: 676C        beq 24990    (ASIC11 CHECK PORT ERROR 1)
    rom[0x24b66/2]  =   0x606c;     // 024B66: 676C        beq 24bd4    (ASIC12 CHECK PORT ERROR 4)
    rom[0x24de2/2]  =   0x606c;     // 024DE2: 676C        beq 24e50    (ASIC12 CHECK PORT ERROR 3)
    rom[0x2502a/2]  =   0x606c;     // 02502A: 676C        beq 25098    (ASIC12 CHECK PORT ERROR 2)
    rom[0x25556/2]  =   0x6000;     // 025556: 6700 E584   beq 23adc    (ASIC12 CHECK PORT ERROR 1)
    // different from drgnwrld:
    rom[0x2a162/2]  =   0x606c;     // 02A162: 676C        beq 2a1d0    (ASIC11 CHECK PORT ERROR 2)
*/
}

DRIVER_INIT_MEMBER(igs011_state,drgnwrldv21)
{
//  UINT16 *rom = (UINT16 *) memregion("maincpu")->base();

	drgnwrld_type2_decrypt();
	drgnwrld_gfx_decrypt();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xd4c0, 0xd4ff, read16_delegate(FUNC(igs011_state::drgnwrldv21_igs011_prot2_r), this));
/*
    // PROTECTION CHECKS
    // bp 32ee; bp 11ca8; bp 23d5e; bp 23fd0; bp 24170; bp 24348; bp 2454e; bp 246cc; bp 24922; bp 24b66; bp 24de2; bp 2502a; bp 25556; bp 269de; bp 2766a; bp 2a830
    rom[0x032ee/2]  =   0x606c;     // 0032EE: 676C        beq 335c     (ASIC11 CHECK PORT ERROR 3)
    rom[0x11ca8/2]  =   0x606c;     // 011CA8: 676C        beq 11d16    (CHECK PORT ERROR 1)
    rom[0x23d5e/2]  =   0x606c;     // 023D5E: 676C        beq 23dcc    (CHECK PORT ERROR 1)
    rom[0x23fd0/2]  =   0x606c;     // 023FD0: 676C        beq 2403e    (CHECK PORT ERROR 2)
    rom[0x24170/2]  =   0x606c;     // 024170: 676C        beq 241de    (CHECK PORT ERROR 3)
    rom[0x24348/2]  =   0x606c;     // 024348: 676C        beq 243b6    (ASIC11 CHECK PORT ERROR 4)
    rom[0x2454e/2]  =   0x606c;     // 02454E: 676C        beq 245bc    (ASIC11 CHECK PORT ERROR 3)
    rom[0x246cc/2]  =   0x606c;     // 0246CC: 676C        beq 2473a    (ASIC11 CHECK PORT ERROR 2)
    rom[0x24922/2]  =   0x606c;     // 024922: 676C        beq 24990    (ASIC11 CHECK PORT ERROR 1)
    rom[0x24b66/2]  =   0x606c;     // 024B66: 676C        beq 24bd4    (ASIC12 CHECK PORT ERROR 4)
    rom[0x24de2/2]  =   0x606c;     // 024DE2: 676C        beq 24e50    (ASIC12 CHECK PORT ERROR 3)
    rom[0x2502a/2]  =   0x606c;     // 02502A: 676C        beq 25098    (ASIC12 CHECK PORT ERROR 2)
    rom[0x25556/2]  =   0x6000;     // 025556: 6700 E584   beq 23adc    (ASIC12 CHECK PORT ERROR 1)
    rom[0x269de/2]  =   0x606c;     // 0269DE: 676C        beq 26a4c    (ASIC12 CHECK PORT ERROR 1)
    rom[0x2766a/2]  =   0x606c;     // 02766A: 676C        beq 276d8    (CHECK PORT ERROR 3)
    rom[0x2a830/2]  =   0x606c;     // 02A830: 676C        beq 2a89e    (ASIC11 CHECK PORT ERROR 2)
*/
}

DRIVER_INIT_MEMBER(igs011_state,drgnwrldv21j)
{
//  UINT16 *rom = (UINT16 *) memregion("maincpu")->base();

	drgnwrld_type3_decrypt();
	drgnwrld_gfx_decrypt();
/*
    // PROTECTION CHECKS
    rom[0x033d2/2]  =   0x606c;     // 0033D2: 676C        beq 3440     (ASIC11 CHECK PORT ERROR 3)
    rom[0x11c74/2]  =   0x606c;     // 011C74: 676C        beq 11ce2    (CHECK PORT ERROR 1)
    rom[0x23d2a/2]  =   0x606c;     // 023D2A: 676C        beq 23d98
    rom[0x23f68/2]  =   0x606c;     // 023F68: 676C        beq 23fd6
    rom[0x240d4/2]  =   0x606c;     // 0240D4: 676C        beq 24142    (CHECK PORT ERROR 3)
    rom[0x242ac/2]  =   0x606c;     // 0242AC: 676C        beq 2431a
    rom[0x244b2/2]  =   0x606c;     // 0244B2: 676C        beq 24520
    rom[0x24630/2]  =   0x606c;     // 024630: 676C        beq 2469e
    rom[0x24886/2]  =   0x606c;     // 024886: 676C        beq 248f4
    rom[0x24aca/2]  =   0x606c;     // 024ACA: 676C        beq 24b38
    rom[0x24d46/2]  =   0x606c;     // 024D46: 676C        beq 24db4
    rom[0x24f8e/2]  =   0x606c;     // 024F8E: 676C        beq 24ffc
    rom[0x254ba/2]  =   0x6000;     // 0254BA: 6700 E620   beq 23adc    (ASIC12 CHECK PORT ERROR 1)
    rom[0x26a52/2]  =   0x606c;     // 026A52: 676C        beq 26ac0    (ASIC12 CHECK PORT ERROR 1)
    rom[0x276aa/2]  =   0x606c;     // 0276AA: 676C        beq 27718    (CHECK PORT ERROR 3)
    rom[0x2a870/2]  =   0x606c;     // 02A870: 676C        beq 2a8de    (ASIC11 CHECK PORT ERROR 2)
*/
}

DRIVER_INIT_MEMBER(igs011_state,drgnwrldv20j)
{
//  UINT16 *rom = (UINT16 *) memregion("maincpu")->base();

	drgnwrld_type3_decrypt();
	drgnwrld_gfx_decrypt();
/*
    // PROTECTION CHECKS
    // bp 33d2; bp 11c74; bp 23d2a; bp 23f68; bp 240d4; bp 242ac; bp 244b2; bp 24630; bp 24886; bp 24aca; bp 24d46; bp 24f8e; bp 254ba; bp 26a52; bp 276a0; bp 2a86e
    rom[0x033d2/2]  =   0x606c;     // 0033D2: 676C        beq 3440     (ASIC11 CHECK PORT ERROR 3)
    rom[0x11c74/2]  =   0x606c;     // 011C74: 676C        beq 11ce2    (CHECK PORT ERROR 1)
    rom[0x23d2a/2]  =   0x606c;     // 023D2A: 676C        beq 23d98
    rom[0x23f68/2]  =   0x606c;     // 023F68: 676C        beq 23fd6
    rom[0x240d4/2]  =   0x606c;     // 0240D4: 676C        beq 24142    (CHECK PORT ERROR 3)
    rom[0x242ac/2]  =   0x606c;     // 0242AC: 676C        beq 2431a
    rom[0x244b2/2]  =   0x606c;     // 0244B2: 676C        beq 24520
    rom[0x24630/2]  =   0x606c;     // 024630: 676C        beq 2469e
    rom[0x24886/2]  =   0x606c;     // 024886: 676C        beq 248f4
    rom[0x24aca/2]  =   0x606c;     // 024ACA: 676C        beq 24b38
    rom[0x24d46/2]  =   0x606c;     // 024D46: 676C        beq 24db4
    rom[0x24f8e/2]  =   0x606c;     // 024F8E: 676C        beq 24ffc
    rom[0x254ba/2]  =   0x6000;     // 0254BA: 6700 E620   beq 23adc    (ASIC12 CHECK PORT ERROR 1)
    rom[0x26a52/2]  =   0x606c;     // 026A52: 676C        beq 26ac0    (ASIC12 CHECK PORT ERROR 1)
    // different from drgnwrldv21j:
    rom[0x276a0/2]  =   0x606c;     // 0276A0: 676C        beq 2770e    (CHECK PORT ERROR 3)
    rom[0x2a86e/2]  =   0x606c;     // 02A86E: 676C        beq 2a8dc    (ASIC11 CHECK PORT ERROR 2)
*/
}

DRIVER_INIT_MEMBER(igs011_state,drgnwrldv11h)
{
	drgnwrld_type1_decrypt();
	drgnwrld_gfx_decrypt();

	// PROTECTION CHECKS
	// the protection checks are already patched out like we do!
}

DRIVER_INIT_MEMBER(igs011_state,drgnwrldv10c)
{
//  UINT16 *rom = (UINT16 *) memregion("maincpu")->base();

	drgnwrld_type1_decrypt();
	drgnwrld_gfx_decrypt();
/*
    // PROTECTION CHECKS
    // bp 33d2; bp 23d0e; bp 23f58; bp 240d0; bp 242a8; bp 244ae; bp 2462c; bp 24882; bp 24ac6; bp 24d42; bp 24f8a; bp 254b6; bp 2a23a
    rom[0x033d2/2]  =   0x606c;     // 0033D2: 676C        beq 3440     (ASIC11 CHECK PORT ERROR 3)
    rom[0x23d0e/2]  =   0x606c;     // 023D0E: 676C        beq 23d7c    (CHECK PORT ERROR 1)
    rom[0x23f58/2]  =   0x606c;     // 023F58: 676C        beq 23fc6    (CHECK PORT ERROR 2)
    rom[0x240d0/2]  =   0x606c;     // 0240D0: 676C        beq 2413e    (CHECK PORT ERROR 3)
    rom[0x242a8/2]  =   0x606c;     // 0242A8: 676C        beq 24316    (ASIC11 CHECK PORT ERROR 4)
    rom[0x244ae/2]  =   0x606c;     // 0244AE: 676C        beq 2451c    (ASIC11 CHECK PORT ERROR 3)
    rom[0x2462c/2]  =   0x606c;     // 02462C: 676C        beq 2469a    (ASIC11 CHECK PORT ERROR 2)
    rom[0x24882/2]  =   0x606c;     // 024882: 676C        beq 248f0    (ASIC11 CHECK PORT ERROR 1)
    rom[0x24ac6/2]  =   0x606c;     // 024AC6: 676C        beq 24b34    (ASIC12 CHECK PORT ERROR 4)
    rom[0x24d42/2]  =   0x606c;     // 024D42: 676C        beq 24db0    (ASIC12 CHECK PORT ERROR 3)
    rom[0x24f8a/2]  =   0x606c;     // 024F8A: 676C        beq 24ff8    (ASIC12 CHECK PORT ERROR 2)
    rom[0x254b6/2]  =   0x6000;     // 0254B6: 6700 E5FC   beq 23ab4    (ASIC12 CHECK PORT ERROR 1)
    rom[0x2a23a/2]  =   0x606c;     // 02A23A: 676C        beq 2a2a8    (ASIC11 CHECK PORT ERROR 2)
*/
}


DRIVER_INIT_MEMBER(igs011_state,lhb)
{
//  UINT16 *rom = (UINT16 *) memregion("maincpu")->base();

	lhb_decrypt();

	// PROTECTION CHECKS
//  rom[0x2eef6/2]  =   0x4e75;     // 02EEF6: 4E56 FE00    link A6, #-$200  (fills palette with pink otherwise)
}

DRIVER_INIT_MEMBER(igs011_state,lhbv33c)
{
//  UINT16 *rom = (UINT16 *) memregion("maincpu")->base();

	lhb_decrypt();

	// PROTECTION CHECKS
//  rom[0x2e988/2]  =   0x4e75;     // 02E988: 4E56 FE00    link A6, #-$200  (fills palette with pink otherwise)
}

DRIVER_INIT_MEMBER(igs011_state,dbc)
{
//  UINT16 *rom = (UINT16 *) memregion("maincpu")->base();

	dbc_decrypt();

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x10600, 0x107ff, read16_delegate(FUNC(igs011_state::dbc_igs011_prot2_r), this));
/*
    // PROTECTION CHECKS
    rom[0x04c42/2]  =   0x602e;     // 004C42: 6604         bne 4c48  (rom test error otherwise)

    rom[0x08694/2]  =   0x6008;     // 008694: 6408         bcc 869e  (fills screen with characters otherwise)
    rom[0x0a05e/2]  =   0x4e71;     // 00A05E: 6408         bcc a068  (fills screen with characters otherwise)
    rom[0x0bec2/2]  =   0x6008;     // 00BEC2: 6408         bcc becc  (fills screen with characters otherwise)
    rom[0x0c0d4/2]  =   0x600a;     // 00C0D4: 640A         bcc c0e0  (wrong game state otherwise)
    rom[0x0c0f0/2]  =   0x4e71;     // 00C0F0: 6408         bcc c0fa  (wrong palette otherwise)
    rom[0x0e292/2]  =   0x6008;     // 00E292: 6408         bcc e29c  (fills screen with characters otherwise)
    rom[0x11b42/2]  =   0x6008;     // 011B42: 6408         bcc 11b4c (wrong game state otherwise)
    rom[0x11b5c/2]  =   0x4e71;     // 011B5C: 6408         bcc 11b66 (wrong palette otherwise)
    rom[0x170ae/2]  =   0x4e71;     // 0170AE: 6408         bcc 170b8 (fills screen with characters otherwise)
    rom[0x1842a/2]  =   0x6024;     // 01842A: 6724         beq 18450 (ASIC11 ERROR otherwise)
    rom[0x18538/2]  =   0x6008;     // 018538: 6408         bcc 18542 (wrong game state otherwise)
    rom[0x18552/2]  =   0x4e71;     // 018552: 6408         bcc 1855c (wrong palette otherwise)
    rom[0x18c0e/2]  =   0x6006;     // 018C0E: 6406         bcc 18c16 (fills screen with characters otherwise)
    rom[0x1923e/2]  =   0x4e71;     // 01923E: 6408         bcc 19248 (fills screen with characters otherwise)
*/
	// Fix for the palette fade on title screen
//  rom[0x19E90/2]  =   0x00ff;
}

DRIVER_INIT_MEMBER(igs011_state,ryukobou)
{
//  UINT16 *rom = (UINT16 *) memregion("maincpu")->base();

	ryukobou_decrypt();

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x10600, 0x107ff, read16_delegate(FUNC(igs011_state::ryukobou_igs011_prot2_r), this));

	// PROTECTION CHECKS
//  rom[0x2df68/2]  =   0x4e75;     // 02DF68: 4E56 FE00    link A6, #-$200  (fills palette with pink otherwise)
}


DRIVER_INIT_MEMBER(igs011_state,xymg)
{
//  UINT16 *rom = (UINT16 *) memregion("maincpu")->base();

	lhb_decrypt();
/*
    // PROTECTION CHECKS
    rom[0x00502/2]  =   0x6006;     // 000502: 6050         bra 554
    rom[0x0fc1c/2]  =   0x6036;     // 00FC1C: 6736         beq fc54  (fills palette with red otherwise)
    rom[0x1232a/2]  =   0x6036;     // 01232A: 6736         beq 12362 (fills palette with red otherwise)
    rom[0x18244/2]  =   0x6036;     // 018244: 6736         beq 1827c (fills palette with red otherwise)
    rom[0x1e15e/2]  =   0x6036;     // 01E15E: 6736         beq 1e196 (fills palette with red otherwise)
    rom[0x22286/2]  =   0x6000;     // 022286: 6700 02D2    beq 2255a (fills palette with green otherwise)
    rom[0x298ce/2]  =   0x6036;     // 0298CE: 6736         beq 29906 (fills palette with red otherwise)
    rom[0x2e07c/2]  =   0x6036;     // 02E07C: 6736         beq 2e0b4 (fills palette with red otherwise)
    rom[0x38f1c/2]  =   0x6000;     // 038F1C: 6700 071C    beq 3963a (ASIC11 ERROR 1)
    rom[0x390e8/2]  =   0x6000;     // 0390E8: 6700 0550    beq 3963a (ASIC11 ERROR 2)
    rom[0x3933a/2]  =   0x6000;     // 03933A: 6700 02FE    beq 3963a (ASIC11 ERROR 3)
    rom[0x3955c/2]  =   0x6000;     // 03955C: 6700 00DC    beq 3963a (ASIC11 ERROR 4)
    rom[0x397f4/2]  =   0x6000;     // 0397F4: 6700 02C0    beq 39ab6 (fills palette with green otherwise)
    rom[0x39976/2]  =   0x6000;     // 039976: 6700 013E    beq 39ab6 (fills palette with green otherwise)
    rom[0x39a7e/2]  =   0x6036;     // 039A7E: 6736         beq 39ab6 (fills palette with green otherwise)
    rom[0x4342c/2]  =   0x4e75;     // 04342C: 4E56 0000    link A6, #$0
    rom[0x49966/2]  =   0x6036;     // 049966: 6736         beq 4999e (fills palette with blue otherwise)
    rom[0x58140/2]  =   0x6036;     // 058140: 6736         beq 58178 (fills palette with red otherwise)
    rom[0x5e05a/2]  =   0x6036;     // 05E05A: 6736         beq 5e092 (fills palette with red otherwise)
    rom[0x5ebf0/2]  =   0x6000;     // 05EBF0: 6700 0208    beq 5edfa (fills palette with red otherwise)
    rom[0x5edc2/2]  =   0x6036;     // 05EDC2: 6736         beq 5edfa (fills palette with green otherwise)
    rom[0x5f71c/2]  =   0x6000;     // 05F71C: 6700 01F2    beq 5f910 (fills palette with green otherwise)
    rom[0x5f8d8/2]  =   0x6036;     // 05F8D8: 6736         beq 5f910 (fills palette with red otherwise)
    rom[0x64836/2]  =   0x6036;     // 064836: 6736         beq 6486e (fills palette with red otherwise)
*/
}

DRIVER_INIT_MEMBER(igs011_state,wlcc)
{
//  UINT16 *rom = (UINT16 *) memregion("maincpu")->base();

	wlcc_decrypt();
/*
    // PROTECTION CHECKS
    rom[0x16b96/2]  =   0x6000;     // 016B96: 6700 02FE    beq 16e96  (fills palette with red otherwise)
    rom[0x16e5e/2]  =   0x6036;     // 016E5E: 6736         beq 16e96  (fills palette with green otherwise)
    rom[0x17852/2]  =   0x6000;     // 017852: 6700 01F2    beq 17a46  (fills palette with green otherwise)
    rom[0x17a0e/2]  =   0x6036;     // 017A0E: 6736         beq 17a46  (fills palette with red otherwise)
    rom[0x23636/2]  =   0x6036;     // 023636: 6736         beq 2366e  (fills palette with red otherwise)
    rom[0x2b1e6/2]  =   0x6000;     // 02B1E6: 6700 0218    beq 2b400  (fills palette with green otherwise)
    rom[0x2f9f2/2]  =   0x6000;     // 02F9F2: 6700 04CA    beq 2febe  (fills palette with green otherwise)
    rom[0x2fb2e/2]  =   0x6000;     // 02FB2E: 6700 038E    beq 2febe  (fills palette with red otherwise)
    rom[0x2fcf2/2]  =   0x6000;     // 02FCF2: 6700 01CA    beq 2febe  (fills palette with red otherwise)
    rom[0x2fe86/2]  =   0x6036;     // 02FE86: 6736         beq 2febe  (fills palette with red otherwise)
    rom[0x3016e/2]  =   0x6000;     // 03016E: 6700 03F6    beq 30566  (fills palette with green otherwise)
    rom[0x303c8/2]  =   0x6000;     // 0303C8: 6700 019C    beq 30566  (fills palette with green otherwise)
    rom[0x3052e/2]  =   0x6036;     // 03052E: 6736         beq 30566  (fills palette with green otherwise)
*/
}


DRIVER_INIT_MEMBER(igs011_state,lhb2)
{
//  UINT16 *rom = (UINT16 *) memregion("maincpu")->base();

	lhb2_decrypt();
	lhb2_decrypt_gfx();
/*
    // PROTECTION CHECKS
    rom[0x034f4/2]  =   0x4e71;     // 0034F4: 660E    bne 3504   (rom test, fills palette with white otherwise)
    rom[0x03502/2]  =   0x6032;     // 003502: 6732    beq 3536   (rom test, fills palette with white otherwise)
    rom[0x1afea/2]  =   0x6034;     // 01AFEA: 6734    beq 1b020  (fills palette with black otherwise)
    rom[0x24b8a/2]  =   0x6036;     // 024B8A: 6736    beq 24bc2  (fills palette with green otherwise)
    rom[0x29ef8/2]  =   0x6036;     // 029EF8: 6736    beq 29f30  (fills palette with red otherwise)
    rom[0x2e69c/2]  =   0x6036;     // 02E69C: 6736    beq 2e6d4  (fills palette with green otherwise)
    rom[0x2fe96/2]  =   0x6036;     // 02FE96: 6736    beq 2fece  (fills palette with red otherwise)
    rom[0x325da/2]  =   0x6036;     // 0325DA: 6736    beq 32612  (fills palette with green otherwise)
    rom[0x3d80a/2]  =   0x6034;     // 03D80A: 6734    beq 3d840  (fills palette with black otherwise)
    rom[0x3ed80/2]  =   0x6036;     // 03ED80: 6736    beq 3edb8  (fills palette with red otherwise)
    rom[0x41d72/2]  =   0x6034;     // 041D72: 6734    beq 41da8  (fills palette with black otherwise)
    rom[0x44834/2]  =   0x6034;     // 044834: 6734    beq 4486a  (fills palette with black otherwise)
*/
}

DRIVER_INIT_MEMBER(igs011_state,vbowl)
{
	UINT16 *rom = (UINT16 *) memregion("maincpu")->base();
	UINT8  *gfx = (UINT8 *)  memregion("blitter")->base();
	int i;

	vbowlj_decrypt();

	for (i = 0x400000-1; i >= 0; i--)
	{
		gfx[i * 2 + 1] = (gfx[i] & 0xf0) >> 4;
		gfx[i * 2 + 0] = (gfx[i] & 0x0f) >> 0;
	}

	// Patch the bad dump so that it doesn't reboot at the end of a game (the patched value is from vbowlj)
	rom[0x080e0/2] = 0xe549;    // 0080E0: 0449 dc.w $0449; ILLEGAL
/*
    // PROTECTION CHECKS
    rom[0x03764/2] = 0x4e75;    // 003764: 4E56 0000 link    A6, #$0
    rom[0x173ee/2] = 0x600c;    // 0173EE: 670C      beq     $173fc
    rom[0x1e6e6/2] = 0x600c;    // 01E6E6: 670C      beq     $1e6f4
    rom[0x1f7ce/2] = 0x600c;    // 01F7CE: 670C      beq     $1f7dc
*/
	machine().save().register_postload(save_prepost_delegate(FUNC(igs011_state::prot_mem_range_set), this));
}


DRIVER_INIT_MEMBER(igs011_state,vbowlj)
{
//  UINT16 *rom = (UINT16 *) memregion("maincpu")->base();
	UINT8  *gfx = (UINT8 *)  memregion("blitter")->base();
	int i;

	vbowlj_decrypt();

	for (i = 0x400000-1; i >= 0; i--)
	{
		gfx[i * 2 + 1] = (gfx[i] & 0xf0) >> 4;
		gfx[i * 2 + 0] = (gfx[i] & 0x0f) >> 0;
	}
/*
    // PROTECTION CHECKS
    rom[0x37b4/2] = 0x4e75;     // 0037B4: 4E56 0000 link    A6, #$0
    rom[0x17720/2] = 0x600c;    // 017720: 670C      beq     1772e
    rom[0x1e6e6/2] = 0x600c;    // 01E6E6: 670C      beq     $1e6f4
    rom[0x1f7c8/2] = 0x600c;    // 01F7C8: 670C      beq     1f7d6
*/
	machine().save().register_postload(save_prepost_delegate(FUNC(igs011_state::prot_mem_range_set), this));
}


DRIVER_INIT_MEMBER(igs011_state,nkishusp)
{
	UINT16 *rom = (UINT16 *) memregion("maincpu")->base();

	nkishusp_decrypt();
	lhb2_decrypt_gfx();

	// PROTECTION CHECKS (similar to lhb2)

	rom[0x03624/2]  =   0x6042;     // 003624: 660E      bne     $3634 (ROM test)

	rom[0x1a9d2/2]  =   0x6034;     // 01A9D2: 6734      beq     $1aa08
	rom[0x26306/2]  =   0x6036;     // 026306: 6736      beq     $2633e
	rom[0x29190/2]  =   0x6038;     // 029190: 6E38      bgt     $291ca (system error)
	rom[0x2b82a/2]  =   0x6036;     // 02B82A: 6736      beq     $2b862
	rom[0x2ff20/2]  =   0x6036;     // 02FF20: 6736      beq     $2ff58
	rom[0x3151c/2]  =   0x6036;     // 03151C: 6736      beq     $31554
	rom[0x33dfc/2]  =   0x6036;     // 033DFC: 6736      beq     $33e34
	rom[0x3460e/2]  =   0x6038;     // 03460E: 6E38      bgt     $34648 (system error)
	rom[0x3f09e/2]  =   0x6034;     // 03F09E: 6734      beq     $3f0d4
	rom[0x406a8/2]  =   0x6036;     // 0406A8: 6736      beq     $406e0
	rom[0x4376a/2]  =   0x6034;     // 04376A: 6734      beq     $437a0
	rom[0x462d6/2]  =   0x6034;     // 0462D6: 6734      beq     $4630c
	rom[0x471ec/2]  =   0x6036;     // 0471EC: 6E36      bgt     $47224 (system error)
	rom[0x49c46/2]  =   0x6000;     // 049C46: 6700 0444 beq     $4a08c
	rom[0x4a2b6/2]  =   0x6036;     // 04A2B6: 6736      beq     $4a2ee
	rom[0x4c67a/2]  =   0x6038;     // 04C67A: 6E38      bgt     $4c6b4 (system error)
}


/***************************************************************************

    Memory Maps

***************************************************************************/

static ADDRESS_MAP_START( drgnwrld, AS_PROGRAM, 16, igs011_state )
//  drgnwrld: IGS011 protection dynamically mapped at 1dd7x
//  AM_RANGE( 0x01dd70, 0x01dd77 ) AM_WRITE(igs011_prot1_w )
//  AM_RANGE( 0x01dd78, 0x01dd79 ) AM_READ ( igs011_prot1_r )

	AM_RANGE( 0x000000, 0x07ffff ) AM_ROM
	AM_RANGE( 0x100000, 0x103fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x200000, 0x200fff ) AM_RAM AM_SHARE("priority_ram")
	AM_RANGE( 0x400000, 0x401fff ) AM_RAM_WRITE(igs011_palette ) AM_SHARE("paletteram")
	AM_RANGE( 0x500000, 0x500001 ) AM_READ_PORT( "COIN" )
	AM_RANGE( 0x600000, 0x600001 ) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff )
	AM_RANGE( 0x700000, 0x700003 ) AM_DEVWRITE8("ymsnd", ym3812_device, write, 0x00ff)

	AM_RANGE( 0x800000, 0x800003 ) AM_WRITE(drgnwrld_igs003_w )
	AM_RANGE( 0x800002, 0x800003 ) AM_READ(drgnwrld_igs003_r )

	AM_RANGE( 0xa20000, 0xa20001 ) AM_WRITE(igs011_priority_w )
	AM_RANGE( 0xa40000, 0xa40001 ) AM_WRITE(igs_dips_w )

	AM_RANGE( 0xa50000, 0xa50001 ) AM_WRITE(igs011_prot_addr_w )
//  AM_RANGE( 0xa50000, 0xa50005 ) AM_READ(igs011_prot_fake_r )

	AM_RANGE( 0xa58000, 0xa58001 ) AM_WRITE(igs011_blit_x_w )
	AM_RANGE( 0xa58800, 0xa58801 ) AM_WRITE(igs011_blit_y_w )
	AM_RANGE( 0xa59000, 0xa59001 ) AM_WRITE(igs011_blit_w_w )
	AM_RANGE( 0xa59800, 0xa59801 ) AM_WRITE(igs011_blit_h_w )
	AM_RANGE( 0xa5a000, 0xa5a001 ) AM_WRITE(igs011_blit_gfx_lo_w )
	AM_RANGE( 0xa5a800, 0xa5a801 ) AM_WRITE(igs011_blit_gfx_hi_w )
	AM_RANGE( 0xa5b000, 0xa5b001 ) AM_WRITE(igs011_blit_flags_w )
	AM_RANGE( 0xa5b800, 0xa5b801 ) AM_WRITE(igs011_blit_pen_w )
	AM_RANGE( 0xa5c000, 0xa5c001 ) AM_WRITE(igs011_blit_depth_w )
	AM_RANGE( 0xa88000, 0xa88001 ) AM_READ(igs_3_dips_r )
ADDRESS_MAP_END

static ADDRESS_MAP_START( drgnwrld_igs012, AS_PROGRAM, 16, igs011_state )
	// IGS012
	AM_RANGE( 0x001600, 0x00160f ) AM_WRITE(igs012_prot_swap_w      )   AM_MIRROR(0x01c000) // swap (a5 / 55)
	AM_RANGE( 0x001610, 0x00161f ) AM_READ(igs012_prot_r            )   AM_MIRROR(0x01c000) // read (mode 0)
	AM_RANGE( 0x001620, 0x00162f ) AM_WRITE(igs012_prot_dec_inc_w   )   AM_MIRROR(0x01c000) // dec  (aa), inc  (fa)
	AM_RANGE( 0x001630, 0x00163f ) AM_WRITE(igs012_prot_inc_w       )   AM_MIRROR(0x01c000) // inc  (ff)
	AM_RANGE( 0x001640, 0x00164f ) AM_WRITE(igs012_prot_copy_w      )   AM_MIRROR(0x01c000) // copy (22)
	AM_RANGE( 0x001650, 0x00165f ) AM_WRITE(igs012_prot_dec_copy_w  )   AM_MIRROR(0x01c000) // dec  (5a), copy (33)
	AM_RANGE( 0x001660, 0x00166f ) AM_READ(igs012_prot_r            )   AM_MIRROR(0x01c000) // read (mode 1)
	AM_RANGE( 0x001670, 0x00167f ) AM_WRITE(igs012_prot_mode_w      )   AM_MIRROR(0x01c000) // mode (cc / dd)

	AM_RANGE( 0x00d400, 0x00d43f ) AM_WRITE(igs011_prot2_dec_w              )   // dec   (33)
	AM_RANGE( 0x00d440, 0x00d47f ) AM_WRITE(drgnwrld_igs011_prot2_swap_w    )   // swap  (33)
	AM_RANGE( 0x00d480, 0x00d4bf ) AM_WRITE(igs011_prot2_reset_w            )   // reset (33)
	AM_RANGE( 0x00d4c0, 0x00d4ff ) AM_READ(drgnwrldv20j_igs011_prot2_r  )   // read

	AM_RANGE( 0x902000, 0x902fff ) AM_WRITE(igs012_prot_reset_w )   // reset?
//  AM_RANGE( 0x902000, 0x902005 ) AM_WRITE(igs012_prot_fake_r )

	AM_IMPORT_FROM(drgnwrld)
ADDRESS_MAP_END



// Only values 0 and 7 are written (1 bit per irq source?)
WRITE16_MEMBER(igs011_state::lhb_irq_enable_w)
{
	COMBINE_DATA( &m_lhb_irq_enable );
}

WRITE16_MEMBER(igs011_state::lhb_okibank_w)
{
	if (ACCESSING_BITS_8_15)
	{
		m_oki->set_bank_base((data & 0x200) ? 0x40000 : 0);
	}

	if ( data & (~0x200) )
		logerror("%s: warning, unknown bits written in oki bank = %02x\n", machine().describe_context(), data);

//  popmessage("oki %04x",data);
}

static ADDRESS_MAP_START( lhb, AS_PROGRAM, 16, igs011_state )
//  lhb: IGS011 protection dynamically mapped at 834x
//  AM_RANGE( 0x008340, 0x008347 ) AM_WRITE(igs011_prot1_w )
//  AM_RANGE( 0x008348, 0x008349 ) AM_READ ( igs011_prot1_r )

	AM_RANGE( 0x010000, 0x010001 ) AM_WRITE(lhb_okibank_w )

	AM_RANGE( 0x010200, 0x0103ff ) AM_WRITE(igs011_prot2_inc_w          )
	AM_RANGE( 0x010400, 0x0105ff ) AM_WRITE(lhb_igs011_prot2_swap_w )
	AM_RANGE( 0x010600, 0x0107ff ) AM_READ(lhb_igs011_prot2_r           )
	// no reset

	AM_RANGE( 0x000000, 0x07ffff ) AM_ROM
	AM_RANGE( 0x100000, 0x103fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x200000, 0x200fff ) AM_RAM AM_SHARE("priority_ram")
	AM_RANGE( 0x300000, 0x3fffff ) AM_READWRITE(igs011_layers_r, igs011_layers_w )
	AM_RANGE( 0x400000, 0x401fff ) AM_RAM_WRITE(igs011_palette ) AM_SHARE("paletteram")
	AM_RANGE( 0x600000, 0x600001 ) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff )
	AM_RANGE( 0x700000, 0x700001 ) AM_READ_PORT( "COIN" )
	AM_RANGE( 0x700002, 0x700005 ) AM_READ(lhb_inputs_r )
	AM_RANGE( 0x700002, 0x700003 ) AM_WRITE(lhb_inputs_w )
	AM_RANGE( 0x820000, 0x820001 ) AM_WRITE(igs011_priority_w )
	AM_RANGE( 0x838000, 0x838001 ) AM_WRITE(lhb_irq_enable_w )
	AM_RANGE( 0x840000, 0x840001 ) AM_WRITE(igs_dips_w )

	AM_RANGE( 0x850000, 0x850001 ) AM_WRITE(igs011_prot_addr_w )
//  AM_RANGE( 0x850000, 0x850005 ) AM_WRITE(igs011_prot_fake_r )

	AM_RANGE( 0x858000, 0x858001 ) AM_WRITE(igs011_blit_x_w )
	AM_RANGE( 0x858800, 0x858801 ) AM_WRITE(igs011_blit_y_w )
	AM_RANGE( 0x859000, 0x859001 ) AM_WRITE(igs011_blit_w_w )
	AM_RANGE( 0x859800, 0x859801 ) AM_WRITE(igs011_blit_h_w )
	AM_RANGE( 0x85a000, 0x85a001 ) AM_WRITE(igs011_blit_gfx_lo_w )
	AM_RANGE( 0x85a800, 0x85a801 ) AM_WRITE(igs011_blit_gfx_hi_w )
	AM_RANGE( 0x85b000, 0x85b001 ) AM_WRITE(igs011_blit_flags_w )
	AM_RANGE( 0x85b800, 0x85b801 ) AM_WRITE(igs011_blit_pen_w )
	AM_RANGE( 0x85c000, 0x85c001 ) AM_WRITE(igs011_blit_depth_w )
	AM_RANGE( 0x888000, 0x888001 ) AM_READ(igs_5_dips_r )
ADDRESS_MAP_END

static ADDRESS_MAP_START( xymg, AS_PROGRAM, 16, igs011_state )
//  xymg: IGS011 protection dynamically mapped at 834x
//  AM_RANGE( 0x008340, 0x008347 ) AM_WRITE(igs011_prot1_w )
//  AM_RANGE( 0x008348, 0x008349 ) AM_READ ( igs011_prot1_r )

	AM_RANGE( 0x010000, 0x010001 ) AM_WRITE(lhb_okibank_w )

	AM_RANGE( 0x010200, 0x0103ff ) AM_WRITE(igs011_prot2_inc_w          )   // inc  (33)
	AM_RANGE( 0x010400, 0x0105ff ) AM_WRITE(lhb_igs011_prot2_swap_w     )   // swap (33)
	AM_RANGE( 0x010600, 0x0107ff ) AM_READ(lhb_igs011_prot2_r           )   // read
	// no reset

	AM_RANGE( 0x000000, 0x07ffff ) AM_ROM
	AM_RANGE( 0x100000, 0x103fff ) AM_RAM
	AM_RANGE( 0x1f0000, 0x1f3fff ) AM_RAM AM_SHARE("nvram") // extra ram
	AM_RANGE( 0x200000, 0x200fff ) AM_RAM AM_SHARE("priority_ram")
	AM_RANGE( 0x300000, 0x3fffff ) AM_READWRITE(igs011_layers_r, igs011_layers_w )
	AM_RANGE( 0x400000, 0x401fff ) AM_RAM_WRITE(igs011_palette ) AM_SHARE("paletteram")
	AM_RANGE( 0x600000, 0x600001 ) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff )
	AM_RANGE( 0x700000, 0x700003 ) AM_WRITE(xymg_igs003_w )
	AM_RANGE( 0x700002, 0x700003 ) AM_READ(xymg_igs003_r )
	AM_RANGE( 0x820000, 0x820001 ) AM_WRITE(igs011_priority_w )
	AM_RANGE( 0x840000, 0x840001 ) AM_WRITE(igs_dips_w )

	AM_RANGE( 0x850000, 0x850001 ) AM_WRITE(igs011_prot_addr_w )
//  AM_RANGE( 0x850000, 0x850005 ) AM_WRITE(igs011_prot_fake_r )

	AM_RANGE( 0x858000, 0x858001 ) AM_WRITE(igs011_blit_x_w )
	AM_RANGE( 0x858800, 0x858801 ) AM_WRITE(igs011_blit_y_w )
	AM_RANGE( 0x859000, 0x859001 ) AM_WRITE(igs011_blit_w_w )
	AM_RANGE( 0x859800, 0x859801 ) AM_WRITE(igs011_blit_h_w )
	AM_RANGE( 0x85a000, 0x85a001 ) AM_WRITE(igs011_blit_gfx_lo_w )
	AM_RANGE( 0x85a800, 0x85a801 ) AM_WRITE(igs011_blit_gfx_hi_w )
	AM_RANGE( 0x85b000, 0x85b001 ) AM_WRITE(igs011_blit_flags_w )
	AM_RANGE( 0x85b800, 0x85b801 ) AM_WRITE(igs011_blit_pen_w )
	AM_RANGE( 0x85c000, 0x85c001 ) AM_WRITE(igs011_blit_depth_w )
	AM_RANGE( 0x888000, 0x888001 ) AM_READ(igs_3_dips_r )
ADDRESS_MAP_END

static ADDRESS_MAP_START( wlcc, AS_PROGRAM, 16, igs011_state )
//  wlcc: IGS011 protection dynamically mapped at 834x
//  AM_RANGE( 0x008340, 0x008347 ) AM_WRITE(igs011_prot1_w )
//  AM_RANGE( 0x008348, 0x008349 ) AM_READ(igs011_prot1_r )

	AM_RANGE( 0x518000, 0x5181ff ) AM_WRITE(igs011_prot2_inc_w          )   // inc   (33)
	AM_RANGE( 0x518200, 0x5183ff ) AM_WRITE(wlcc_igs011_prot2_swap_w    )   // swap  (33)
	AM_RANGE( 0x518800, 0x5189ff ) AM_READ(igs011_prot2_reset_r         )   // reset
	AM_RANGE( 0x519000, 0x5195ff ) AM_READ(lhb_igs011_prot2_r           )   // read

	AM_RANGE( 0x000000, 0x07ffff ) AM_ROM
	AM_RANGE( 0x100000, 0x103fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x200000, 0x200fff ) AM_RAM AM_SHARE("priority_ram")
	AM_RANGE( 0x300000, 0x3fffff ) AM_READWRITE(igs011_layers_r, igs011_layers_w )
	AM_RANGE( 0x400000, 0x401fff ) AM_RAM_WRITE(igs011_palette ) AM_SHARE("paletteram")
	AM_RANGE( 0x520000, 0x520001 ) AM_READ_PORT( "COIN" )
	AM_RANGE( 0x600000, 0x600001 ) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff )
	AM_RANGE( 0x800000, 0x800003 ) AM_WRITE(wlcc_igs003_w )
	AM_RANGE( 0x800002, 0x800003 ) AM_READ(wlcc_igs003_r )
	AM_RANGE( 0xa20000, 0xa20001 ) AM_WRITE(igs011_priority_w )
	AM_RANGE( 0xa40000, 0xa40001 ) AM_WRITE(igs_dips_w )

	AM_RANGE( 0xa50000, 0xa50001 ) AM_WRITE(igs011_prot_addr_w )
//  AM_RANGE( 0xa50000, 0xa50005 ) AM_READ(igs011_prot_fake_r )

	AM_RANGE( 0xa58000, 0xa58001 ) AM_WRITE(igs011_blit_x_w )
	AM_RANGE( 0xa58800, 0xa58801 ) AM_WRITE(igs011_blit_y_w )
	AM_RANGE( 0xa59000, 0xa59001 ) AM_WRITE(igs011_blit_w_w )
	AM_RANGE( 0xa59800, 0xa59801 ) AM_WRITE(igs011_blit_h_w )
	AM_RANGE( 0xa5a000, 0xa5a001 ) AM_WRITE(igs011_blit_gfx_lo_w )
	AM_RANGE( 0xa5a800, 0xa5a801 ) AM_WRITE(igs011_blit_gfx_hi_w )
	AM_RANGE( 0xa5b000, 0xa5b001 ) AM_WRITE(igs011_blit_flags_w )
	AM_RANGE( 0xa5b800, 0xa5b801 ) AM_WRITE(igs011_blit_pen_w )
	AM_RANGE( 0xa5c000, 0xa5c001 ) AM_WRITE(igs011_blit_depth_w )
	AM_RANGE( 0xa88000, 0xa88001 ) AM_READ(igs_4_dips_r )
ADDRESS_MAP_END



static ADDRESS_MAP_START( lhb2, AS_PROGRAM, 16, igs011_state )
//  lhb2: IGS011 protection dynamically mapped at 1ff8x
//  AM_RANGE( 0x01ff80, 0x01ff87 ) AM_WRITE(igs011_prot1_w )
//  AM_RANGE( 0x01ff88, 0x01ff89 ) AM_READ ( igs011_prot1_r )

	AM_RANGE( 0x020000, 0x0201ff ) AM_WRITE(igs011_prot2_inc_w          )   // inc   (55)
	AM_RANGE( 0x020200, 0x0203ff ) AM_WRITE(lhb_igs011_prot2_swap_w     )   // swap  (33)
	AM_RANGE( 0x020400, 0x0205ff ) AM_READ(lhb2_igs011_prot2_r          )   // read
	AM_RANGE( 0x020600, 0x0207ff ) AM_WRITE(igs011_prot2_reset_w        )   // reset (55)

	AM_RANGE( 0x000000, 0x07ffff ) AM_ROM
	AM_RANGE( 0x100000, 0x103fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x200000, 0x200001 ) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff )
	AM_RANGE( 0x204000, 0x204003 ) AM_DEVWRITE8("ymsnd", ym2413_device, write, 0x00ff)
	AM_RANGE( 0x208000, 0x208003 ) AM_WRITE(lhb2_igs003_w )
	AM_RANGE( 0x208002, 0x208003 ) AM_READ(lhb2_igs003_r )
	AM_RANGE( 0x20c000, 0x20cfff ) AM_RAM AM_SHARE("priority_ram")
	AM_RANGE( 0x210000, 0x211fff ) AM_RAM_WRITE(igs011_palette ) AM_SHARE("paletteram")
	AM_RANGE( 0x214000, 0x214001 ) AM_READ_PORT( "COIN" )
	AM_RANGE( 0x300000, 0x3fffff ) AM_READWRITE(igs011_layers_r, igs011_layers_w )
	AM_RANGE( 0xa20000, 0xa20001 ) AM_WRITE(igs011_priority_w )
	AM_RANGE( 0xa40000, 0xa40001 ) AM_WRITE(igs_dips_w )

	AM_RANGE( 0xa50000, 0xa50001 ) AM_WRITE(igs011_prot_addr_w )
//  AM_RANGE( 0xa50000, 0xa50005 ) AM_READ(igs011_prot_fake_r )

	AM_RANGE( 0xa58000, 0xa58001 ) AM_WRITE(igs011_blit_x_w )
	AM_RANGE( 0xa58800, 0xa58801 ) AM_WRITE(igs011_blit_y_w )
	AM_RANGE( 0xa59000, 0xa59001 ) AM_WRITE(igs011_blit_w_w )
	AM_RANGE( 0xa59800, 0xa59801 ) AM_WRITE(igs011_blit_h_w )
	AM_RANGE( 0xa5a000, 0xa5a001 ) AM_WRITE(igs011_blit_gfx_lo_w )
	AM_RANGE( 0xa5a800, 0xa5a801 ) AM_WRITE(igs011_blit_gfx_hi_w )
	AM_RANGE( 0xa5b000, 0xa5b001 ) AM_WRITE(igs011_blit_flags_w )
	AM_RANGE( 0xa5b800, 0xa5b801 ) AM_WRITE(igs011_blit_pen_w )
	AM_RANGE( 0xa5c000, 0xa5c001 ) AM_WRITE(igs011_blit_depth_w )
	AM_RANGE( 0xa88000, 0xa88001 ) AM_READ(igs_3_dips_r )
ADDRESS_MAP_END



static ADDRESS_MAP_START( nkishusp, AS_PROGRAM, 16, igs011_state )
//  nkishusp: IGS011 protection dynamically mapped at 1ff8x
//  AM_RANGE( 0x01ff80, 0x01ff87 ) AM_WRITE(igs011_prot1_w )
//  AM_RANGE( 0x01ff88, 0x01ff89 ) AM_READ ( igs011_prot1_r )

	// to be done:
	AM_RANGE( 0x023000, 0x0231ff ) AM_WRITE(igs011_prot2_inc_w          )   // inc   (55)
	AM_RANGE( 0x023200, 0x0233ff ) AM_WRITE(lhb_igs011_prot2_swap_w     )   // swap  (33)
	AM_RANGE( 0x023400, 0x0235ff ) AM_READ(lhb2_igs011_prot2_r          )   // read
	AM_RANGE( 0x023600, 0x0237ff ) AM_WRITE(igs011_prot2_reset_w        )   // reset (55)

	AM_RANGE( 0x000000, 0x07ffff ) AM_ROM
	AM_RANGE( 0x100000, 0x103fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x200000, 0x200001 ) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff )
	AM_RANGE( 0x204000, 0x204003 ) AM_DEVWRITE8("ymsnd", ym2413_device, write, 0x00ff)
	AM_RANGE( 0x208000, 0x208003 ) AM_WRITE(lhb2_igs003_w )
	AM_RANGE( 0x208002, 0x208003 ) AM_READ(lhb2_igs003_r )
	AM_RANGE( 0x20c000, 0x20cfff ) AM_RAM AM_SHARE("priority_ram")
	AM_RANGE( 0x210000, 0x211fff ) AM_RAM_WRITE(igs011_palette ) AM_SHARE("paletteram")
	AM_RANGE( 0x214000, 0x214001 ) AM_READ_PORT( "COIN" )
	AM_RANGE( 0x300000, 0x3fffff ) AM_READWRITE(igs011_layers_r, igs011_layers_w )
	AM_RANGE( 0xa20000, 0xa20001 ) AM_WRITE(igs011_priority_w )
	AM_RANGE( 0xa38000, 0xa38001 ) AM_WRITE(lhb_irq_enable_w )
	AM_RANGE( 0xa40000, 0xa40001 ) AM_WRITE(igs_dips_w )

	AM_RANGE( 0xa50000, 0xa50001 ) AM_WRITE(igs011_prot_addr_w )
//  AM_RANGE( 0xa50000, 0xa50005 ) AM_READ(igs011_prot_fake_r )

	AM_RANGE( 0xa58000, 0xa58001 ) AM_WRITE(igs011_blit_x_w )
	AM_RANGE( 0xa58800, 0xa58801 ) AM_WRITE(igs011_blit_y_w )
	AM_RANGE( 0xa59000, 0xa59001 ) AM_WRITE(igs011_blit_w_w )
	AM_RANGE( 0xa59800, 0xa59801 ) AM_WRITE(igs011_blit_h_w )
	AM_RANGE( 0xa5a000, 0xa5a001 ) AM_WRITE(igs011_blit_gfx_lo_w )
	AM_RANGE( 0xa5a800, 0xa5a801 ) AM_WRITE(igs011_blit_gfx_hi_w )
	AM_RANGE( 0xa5b000, 0xa5b001 ) AM_WRITE(igs011_blit_flags_w )
	AM_RANGE( 0xa5b800, 0xa5b801 ) AM_WRITE(igs011_blit_pen_w )
	AM_RANGE( 0xa5c000, 0xa5c001 ) AM_WRITE(igs011_blit_depth_w )
	AM_RANGE( 0xa88000, 0xa88001 ) AM_READ(igs_3_dips_r )
ADDRESS_MAP_END



/* trap15's note:
 * TODO: change this horrible device-> chain to be proper.
 */
READ16_MEMBER(igs011_state::ics2115_word_r)
{
	ics2115_device* ics2115 = machine().device<ics2115_device>("ics");
	switch(offset)
	{
		case 0: return ics2115->read(space, (offs_t)0);
		case 1: return ics2115->read(space, (offs_t)1);
		case 2: return (ics2115->read(space, (offs_t)3) << 8) | ics2115->read(space, (offs_t)2);
	}
	return 0xff;
}

WRITE16_MEMBER(igs011_state::ics2115_word_w)
{
	ics2115_device* ics2115 = machine().device<ics2115_device>("ics");
	switch(offset)
	{
		case 1:
			if (ACCESSING_BITS_0_7)     ics2115->write(space, 1,data);
			break;
		case 2:
			if (ACCESSING_BITS_0_7)     ics2115->write(space, 2,data);
			if (ACCESSING_BITS_8_15)    ics2115->write(space, 3,data>>8);
			break;
	}
}

READ16_MEMBER(igs011_state::vbowl_unk_r)
{
	return 0xffff;
}

void igs011_state::screen_eof_vbowl(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		m_vbowl_trackball[0] = m_vbowl_trackball[1];
		m_vbowl_trackball[1] = (ioport("AN1")->read() << 8) | ioport("AN0")->read();
	}
}

WRITE16_MEMBER(igs011_state::vbowl_pen_hi_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_lhb2_pen_hi = data & 0x07;
	}

	if (data & ~0x7)
		logerror("%06x: warning, unknown bits written to pen_hi = %04x\n", space.device().safe_pc(), m_priority);
}

WRITE16_MEMBER(igs011_state::vbowl_link_0_w){ }
WRITE16_MEMBER(igs011_state::vbowl_link_1_w){ }
WRITE16_MEMBER(igs011_state::vbowl_link_2_w){ }
WRITE16_MEMBER(igs011_state::vbowl_link_3_w){ }

static ADDRESS_MAP_START( vbowl, AS_PROGRAM, 16, igs011_state )
//  vbowl: IGS011 protection dynamically mapped at 834x
//  AM_RANGE( 0x008340, 0x008347 ) AM_WRITE(igs011_prot1_w )
//  AM_RANGE( 0x008348, 0x008349 ) AM_READ(igs011_prot1_r )

	// IGS012
	AM_RANGE( 0x001600, 0x00160f ) AM_WRITE(igs012_prot_swap_w      )   AM_MIRROR(0x01c000) // swap (a5 / 55)
	AM_RANGE( 0x001610, 0x00161f ) AM_READ(igs012_prot_r            )   AM_MIRROR(0x01c000) // read (mode 0)
	AM_RANGE( 0x001620, 0x00162f ) AM_WRITE(igs012_prot_dec_inc_w   )   AM_MIRROR(0x01c000) // dec  (aa), inc  (fa)
	AM_RANGE( 0x001630, 0x00163f ) AM_WRITE(igs012_prot_inc_w       )   AM_MIRROR(0x01c000) // inc  (ff)
	AM_RANGE( 0x001640, 0x00164f ) AM_WRITE(igs012_prot_copy_w      )   AM_MIRROR(0x01c000) // copy (22)
	AM_RANGE( 0x001650, 0x00165f ) AM_WRITE(igs012_prot_dec_copy_w  )   AM_MIRROR(0x01c000) // dec  (5a), copy (33)
	AM_RANGE( 0x001660, 0x00166f ) AM_READ(igs012_prot_r            )   AM_MIRROR(0x01c000) // read (mode 1)
	AM_RANGE( 0x001670, 0x00167f ) AM_WRITE(igs012_prot_mode_w      )   AM_MIRROR(0x01c000) // mode (cc / dd)

	AM_RANGE( 0x00d400, 0x00d43f ) AM_WRITE(igs011_prot2_dec_w              )   // dec   (33)
	AM_RANGE( 0x00d440, 0x00d47f ) AM_WRITE(drgnwrld_igs011_prot2_swap_w    )   // swap  (33)
	AM_RANGE( 0x00d480, 0x00d4bf ) AM_WRITE(igs011_prot2_reset_w            )   // reset (33)
	AM_RANGE( 0x00d4c0, 0x00d4ff ) AM_READ(drgnwrldv20j_igs011_prot2_r      )   // read

	AM_RANGE( 0x50f000, 0x50f1ff ) AM_WRITE(igs011_prot2_dec_w          )   // dec   (33)
	AM_RANGE( 0x50f200, 0x50f3ff ) AM_WRITE(vbowl_igs011_prot2_swap_w   )   // swap  (33)
	AM_RANGE( 0x50f400, 0x50f5ff ) AM_WRITE(igs011_prot2_reset_w        )   // reset (33)
	AM_RANGE( 0x50f600, 0x50f7ff ) AM_READ(vbowl_igs011_prot2_r         )   // read

	AM_RANGE( 0x902000, 0x902fff ) AM_WRITE(igs012_prot_reset_w )   // reset?
//  AM_RANGE( 0x902000, 0x902005 ) AM_WRITE(igs012_prot_fake_r )

	AM_RANGE( 0x000000, 0x07ffff ) AM_ROM
	AM_RANGE( 0x100000, 0x103fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x200000, 0x200fff ) AM_RAM AM_SHARE("priority_ram")
	AM_RANGE( 0x300000, 0x3fffff ) AM_READWRITE(igs011_layers_r, igs011_layers_w )
	AM_RANGE( 0x400000, 0x401fff ) AM_RAM_WRITE(igs011_palette ) AM_SHARE("paletteram")
	AM_RANGE( 0x520000, 0x520001 ) AM_READ_PORT( "COIN" )
	AM_RANGE( 0x600000, 0x600007 ) AM_READWRITE(ics2115_word_r, ics2115_word_w )
	AM_RANGE( 0x700000, 0x700003 ) AM_RAM AM_SHARE("vbowl_trackball")
	AM_RANGE( 0x700004, 0x700005 ) AM_WRITE(vbowl_pen_hi_w )
	AM_RANGE( 0x800000, 0x800003 ) AM_WRITE(vbowl_igs003_w )
	AM_RANGE( 0x800002, 0x800003 ) AM_READ(vbowl_igs003_r )

	AM_RANGE( 0xa00000, 0xa00001 ) AM_WRITE(vbowl_link_0_w )
	AM_RANGE( 0xa08000, 0xa08001 ) AM_WRITE(vbowl_link_1_w )
	AM_RANGE( 0xa10000, 0xa10001 ) AM_WRITE(vbowl_link_2_w )
	AM_RANGE( 0xa18000, 0xa18001 ) AM_WRITE(vbowl_link_3_w )

	AM_RANGE( 0xa20000, 0xa20001 ) AM_WRITE(igs011_priority_w )
	AM_RANGE( 0xa40000, 0xa40001 ) AM_WRITE(igs_dips_w )

	AM_RANGE( 0xa48000, 0xa48001 ) AM_WRITE(igs011_prot_addr_w )
//  AM_RANGE( 0xa48000, 0xa48005 ) AM_WRITE(igs011_prot_fake_r )

	AM_RANGE( 0xa58000, 0xa58001 ) AM_WRITE(igs011_blit_x_w )
	AM_RANGE( 0xa58800, 0xa58801 ) AM_WRITE(igs011_blit_y_w )
	AM_RANGE( 0xa59000, 0xa59001 ) AM_WRITE(igs011_blit_w_w )
	AM_RANGE( 0xa59800, 0xa59801 ) AM_WRITE(igs011_blit_h_w )
	AM_RANGE( 0xa5a000, 0xa5a001 ) AM_WRITE(igs011_blit_gfx_lo_w )
	AM_RANGE( 0xa5a800, 0xa5a801 ) AM_WRITE(igs011_blit_gfx_hi_w )
	AM_RANGE( 0xa5b000, 0xa5b001 ) AM_WRITE(igs011_blit_flags_w )
	AM_RANGE( 0xa5b800, 0xa5b801 ) AM_WRITE(igs011_blit_pen_w )
	AM_RANGE( 0xa5c000, 0xa5c001 ) AM_WRITE(igs011_blit_depth_w )

	AM_RANGE( 0xa80000, 0xa80001 ) AM_READ(vbowl_unk_r )
	AM_RANGE( 0xa88000, 0xa88001 ) AM_READ(igs_4_dips_r )
	AM_RANGE( 0xa90000, 0xa90001 ) AM_READ(vbowl_unk_r )
	AM_RANGE( 0xa98000, 0xa98001 ) AM_READ(vbowl_unk_r )
ADDRESS_MAP_END



/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( drgnwrld )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal  ) )  // 513
	PORT_DIPSETTING(    0x10, DEF_STR( Hard    ) )  // 627
	PORT_DIPSETTING(    0x08, DEF_STR( Harder  ) )  // 741
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )  // 855
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Nudity" )        // "Open Girl" in test mode
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Background" )
	PORT_DIPSETTING(    0x02, "Girl" )
	PORT_DIPSETTING(    0x00, "Landscape" )     // broken backgrounds with Nudity on (PCB does the same)
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Bang Turtle?" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Send Boom?" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPNAME( 0x80, 0x80, "Test?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )   // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )   // used?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 )   // used?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )    // press in girl test to pause, button 3 advances
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( drgnwrldc )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal  ) )  // 513
	PORT_DIPSETTING(    0x10, DEF_STR( Hard    ) )  // 627
	PORT_DIPSETTING(    0x08, DEF_STR( Harder  ) )  // 741
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )  // 855
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Nudity" )        // "Open Girl" in test mode
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Sex Question" )  // "background" in test mode
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Background" )    // "sex question" in test mode
	PORT_DIPSETTING(    0x04, "Girl" )
	PORT_DIPSETTING(    0x00, "Landscape" )     // broken backgrounds with Nudity on (PCB does the same)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Tiles" )
	PORT_DIPSETTING(    0x10, "Mahjong" )
	PORT_DIPSETTING(    0x00, "Symbols" )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x40, "Bang Turtle?" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )   // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )   // used?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 )   // used?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )    // press in girl test to pause, button 3 advances
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( drgnwrldj )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal  ) )  // 513
	PORT_DIPSETTING(    0x10, DEF_STR( Hard    ) )  // 627
	PORT_DIPSETTING(    0x08, DEF_STR( Harder  ) )  // 741
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )  // 855
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Background" )
	PORT_DIPSETTING(    0x01, "Girl" )
	PORT_DIPSETTING(    0x00, "Landscape" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Tiles" )
	PORT_DIPSETTING(    0x04, "Mahjong" )
	PORT_DIPSETTING(    0x00, "Symbols" )
	PORT_DIPNAME( 0x08, 0x08, "Send Boom?" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPNAME( 0x80, 0x80, "Test?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )   // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )   // used?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 )   // used?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )    // press in girl test to pause, button 3 advances
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( lhb2 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x02, "Pay Out (%)" )
	PORT_DIPSETTING(    0x07, "50" )
	PORT_DIPSETTING(    0x06, "54" )
	PORT_DIPSETTING(    0x05, "58" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x03, "66" )
	PORT_DIPSETTING(    0x02, "70" )
	PORT_DIPSETTING(    0x01, "74" )
	PORT_DIPSETTING(    0x00, "78" )
	PORT_DIPNAME( 0x08, 0x00, "Odds Rate" )
	PORT_DIPSETTING(    0x00, "1,2,3,4,5,6,7,8" )
	PORT_DIPSETTING(    0x08, "1,2,3,5,8,15,30,50" )
	PORT_DIPNAME( 0x10, 0x00, "Max Bet" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPNAME( 0x60, 0x60, "Min Bet" )
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )  // Only when bit 4 = 1
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x04, "Credits Per Note" )  // Only when bit 4 = 0
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x08, 0x08, "Max Note Credits" )
	PORT_DIPSETTING(    0x08, "100" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x10, 0x10, "Money Type" )    // Decides whether to use bits 0&1 or bit 2
	PORT_DIPSETTING(    0x10, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x20, 0x20, "Pay Out Type" )
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x03, "500" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0c, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
//  PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x70, "1 : 1" )
	PORT_DIPSETTING(    0x60, "1 : 2" )
	PORT_DIPSETTING(    0x50, "1 : 5" )
	PORT_DIPSETTING(    0x40, "1 : 6" )
	PORT_DIPSETTING(    0x30, "1 : 7" )
	PORT_DIPSETTING(    0x20, "1 : 8" )
	PORT_DIPSETTING(    0x10, "1 : 9" )
	PORT_DIPSETTING(    0x00, "1 : 10" )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1    )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )   // data clear
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )   // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, igs011_state,igs_hopper_r, (void *)0) // hopper switch
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )   // stats
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER    ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O) // clear coin
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    // ? set to 0 both
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    // ? and you can't start a game

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( nkishusp )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x02, "Pay Out (%)" )
	PORT_DIPSETTING(    0x07, "74" )
	PORT_DIPSETTING(    0x06, "77" )
	PORT_DIPSETTING(    0x05, "80" )
	PORT_DIPSETTING(    0x04, "83" )
	PORT_DIPSETTING(    0x03, "86" )
	PORT_DIPSETTING(    0x02, "89" )
	PORT_DIPSETTING(    0x01, "92" )
	PORT_DIPSETTING(    0x00, "95" )
	PORT_DIPNAME( 0x08, 0x00, "Odds Rate" )
	PORT_DIPSETTING(    0x00, "1,2,3,4,5,6,7,8" )
	PORT_DIPSETTING(    0x08, "1,2,3,5,8,15,30,50" )
	PORT_DIPNAME( 0x10, 0x00, "Max Bet" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPNAME( 0x60, 0x60, "Min Bet" )
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x00, "Credit Timer" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )  // Only when bit 3 = 1
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x04, "Credits Per Note" )  // Only when bit 3 = 0
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x08, 0x08, "Money Type" )    // Decides whether to use bits 0&1 or bit 2
	PORT_DIPSETTING(    0x08, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x10, 0x10, "Auto Play" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Undress Girl" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Credit Limit" )
	PORT_DIPSETTING(    0x03, "500" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x70, "1 : 1" )
	PORT_DIPSETTING(    0x60, "1 : 2" )
	PORT_DIPSETTING(    0x50, "1 : 5" )
	PORT_DIPSETTING(    0x40, "1 : 6" )
	PORT_DIPSETTING(    0x30, "1 : 7" )
	PORT_DIPSETTING(    0x20, "1 : 8" )
	PORT_DIPSETTING(    0x10, "1 : 9" )
	PORT_DIPSETTING(    0x00, "1 : 10" )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1    )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )   // data clear
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )   // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, igs011_state,igs_hopper_r, (void *)0) // hopper switch
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )   // stats
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER    ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O) // clear coin
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    // ? set to 0 both
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    // ? and you can't start a game

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( wlcc )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x03, "1000" )
	PORT_DIPSETTING(    0x02, "1500" )
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Bet" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNKNOWN( 0x10, 0x10 )       // shown in test mode
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x40, "Hide Title" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Credits Per Note" )
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Max Note Credits" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x00, "9999" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" )
	PORT_DIPSETTING(    0x20, "Coins" ) // use bits 0-1
	PORT_DIPSETTING(    0x00, "Notes" ) // use bits 2-3
	PORT_DIPNAME( 0x40, 0x00, "Pay Out Type" )
	PORT_DIPSETTING(    0x00, "Coins" )
	PORT_DIPSETTING(    0x40, "Notes" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1     )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2     )
	PORT_SERVICE_NO_TOGGLE( 0x04,   IP_ACTIVE_LOW ) // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE2  ) // shown in test mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_OTHER     ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)   // clear coin
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL   ) PORT_CUSTOM_MEMBER(DEVICE_SELF, igs011_state,igs_hopper_r, (void *)0)   // hopper switch

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )    // bet
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )
INPUT_PORTS_END


static INPUT_PORTS_START( lhb )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out (%)" )
	PORT_DIPSETTING(    0x0f, "96" )
	PORT_DIPSETTING(    0x0e, "93" )
	PORT_DIPSETTING(    0x0d, "90" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0b, "84" )
	PORT_DIPSETTING(    0x0a, "81" )
	PORT_DIPSETTING(    0x09, "78" )
	PORT_DIPSETTING(    0x08, "75" )
	PORT_DIPSETTING(    0x07, "71" )
	PORT_DIPSETTING(    0x06, "68" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x03, "59" )
	PORT_DIPSETTING(    0x02, "56" )
	PORT_DIPSETTING(    0x01, "53" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x30, 0x30, "YAKUMAN Point" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xc0, 0x80, "Max Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Bet" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x70, 0x70, "DAI MANGUAN Cycle" )
	PORT_DIPSETTING(    0x70, "300" )
//  PORT_DIPSETTING(    0x60, "300" )
//  PORT_DIPSETTING(    0x50, "300" )
//  PORT_DIPSETTING(    0x40, "300" )
//  PORT_DIPSETTING(    0x30, "300" )
//  PORT_DIPSETTING(    0x20, "300" )
//  PORT_DIPSETTING(    0x10, "300" )
//  PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPNAME( 0x80, 0x80, "DAI MANGUAN Times" )
	PORT_DIPSETTING(    0x80, "2" )
//  PORT_DIPSETTING(    0x00, "2" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Max Credit" )
	PORT_DIPSETTING(    0x03, "1000" )
	PORT_DIPSETTING(    0x02, "2000" )
	PORT_DIPSETTING(    0x01, "5000" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Note" )
	PORT_DIPSETTING(    0x0c, "1000" )
	PORT_DIPSETTING(    0x08, "2000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x10, 0x10, "CPU Strength" )
	PORT_DIPSETTING(    0x10, "Strong" )
	PORT_DIPSETTING(    0x00, "Weak" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" )
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0xc0, 0xc0, "DONDEN Times" )
	PORT_DIPSETTING(    0xc0, "0" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "8" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "In Game Music" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, "Girls" )
	PORT_DIPSETTING(    0x0c, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, "Dressed" )
	PORT_DIPSETTING(    0x04, "Underwear" )
	PORT_DIPSETTING(    0x00, "Nude" )
	PORT_DIPNAME( 0x10, 0x10, "Note Rate" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x20, 0x20, "Pay Out" )
	PORT_DIPSETTING(    0x20, "Score" )
	PORT_DIPSETTING(    0x00, "Coin" )
	PORT_DIPNAME( 0x40, 0x40, "Coin In" )
	PORT_DIPSETTING(    0x40, "Credit" )
	PORT_DIPSETTING(    0x00, "Score" )
	PORT_DIPNAME( 0x80, 0x80, "Last Chance" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, "In-Game Bet" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, igs011_state,igs_hopper_r, (void *)0) // hopper switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 )   // system reset
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )   // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )   // stats
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1    ) PORT_IMPULSE(5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER    ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O) // clear coins
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER    ) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD)   // shown in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2)   // shown in test mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( vbowl )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Special Picture" ) /* Sexy Interlude pics */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Open Picture" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Controls ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Joystick ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Trackball ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy   ) )   // 5
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )   // 7
	PORT_DIPSETTING(    0x01, DEF_STR( Medium ) )   // 9
	PORT_DIPSETTING(    0x00, DEF_STR( Hard   ) )   // 11
	PORT_DIPNAME( 0x04, 0x04, "Spares To Win (Frames 1-5)" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x18, 0x18, "Points To Win (Frames 6-10)" )
	PORT_DIPSETTING(    0x18, "160" )
	PORT_DIPSETTING(    0x10, "170" )
	PORT_DIPSETTING(    0x08, "180" )
	PORT_DIPSETTING(    0x00, "190" )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Cabinet ID" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Linked Cabinets" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW4")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("AN0")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("AN1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( vbowlj )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Special Picture" ) /* Sexy Interlude pics */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Controls ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Joystick ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Trackball ) )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy   ) )   // 5
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )   // 7
	PORT_DIPSETTING(    0x01, DEF_STR( Medium ) )   // 9
	PORT_DIPSETTING(    0x00, DEF_STR( Hard   ) )   // 11
	PORT_DIPNAME( 0x04, 0x04, "Spares To Win (Frames 1-5)" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x18, 0x18, "Points To Win (Frames 6-10)" )
	PORT_DIPSETTING(    0x18, "160" )
	PORT_DIPSETTING(    0x10, "170" )
	PORT_DIPSETTING(    0x08, "180" )
	PORT_DIPSETTING(    0x00, "190" )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Cabinet ID" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Linked Cabinets" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW4")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("AN0")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("AN1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( xymg )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Credits Per Note" )
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Max Note Credits" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x00, "9999" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" )
	PORT_DIPSETTING(    0x20, "Coins" ) // use bits 0-1
	PORT_DIPSETTING(    0x00, "Notes" ) // use bits 2-3
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x03, "1000" )
	PORT_DIPSETTING(    0x02, "1500" )
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Bet" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  // shown in test mode
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, igs011_state,igs_hopper_r, (void *)0) // hopper switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )   // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )   // stats
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER    ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O) // clear coin
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/***************************************************************************

    Machine Drivers

***************************************************************************/

// for debugging

#if 0
static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 4, 0, 12,  8, 20,16, 28,24 },
	{ STEP8(0,8*4) },
	8*8*4
};
static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 4, 0, 12, 8, 20,16, 28,24,
		36,32, 44,40, 52,48, 60,56 },
	{ STEP16(0,16*4) },
	16*16*4
};
static const gfx_layout layout_8x8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	{ STEP8(0,8*8) },
	8*8*8
};
static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	{ STEP16(0,16*8) },
	16*16*8
};
static const gfx_layout layout_16x16x1 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(15,-1) },
	{ STEP16(0,16*1) },
	16*16*1
};

static GFXDECODE_START( igs011 )
	GFXDECODE_ENTRY( "blitter", 0, layout_8x8x4,   0, 0x80 )
	GFXDECODE_ENTRY( "blitter", 0, layout_16x16x4, 0, 0x80 )
	GFXDECODE_ENTRY( "blitter", 0, layout_8x8x8,   0, 0x08 )
	GFXDECODE_ENTRY( "blitter", 0, layout_16x16x8, 0, 0x08 )
GFXDECODE_END
static GFXDECODE_START( igs011_hi )
	GFXDECODE_ENTRY( "blitter", 0, layout_8x8x4,   0, 0x80 )
	GFXDECODE_ENTRY( "blitter", 0, layout_16x16x4, 0, 0x80 )
	GFXDECODE_ENTRY( "blitter", 0, layout_8x8x8,   0, 0x08 )
	GFXDECODE_ENTRY( "blitter", 0, layout_16x16x8, 0, 0x08 )
	GFXDECODE_ENTRY( "blitter_hi", 0, layout_16x16x1, 0, 0x80 )
GFXDECODE_END
#endif

static MACHINE_CONFIG_START( igs011_base, igs011_state )
	MCFG_CPU_ADD("maincpu",M68000, XTAL_22MHz/3)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(igs011_state, screen_update_igs011)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x800)
//  MCFG_GFXDECODE_ADD("gfxdecode", "palette", igs011)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", XTAL_22MHz/21, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


TIMER_DEVICE_CALLBACK_MEMBER( igs011_state::lev5_timer_irq_cb )
{
	m_maincpu->set_input_line(5, HOLD_LINE);
}

static MACHINE_CONFIG_DERIVED( drgnwrld, igs011_base )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(drgnwrld)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", igs011_state, irq6_line_hold)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_irq", igs011_state, lev5_timer_irq_cb, attotime::from_hz(240)) // lev5 frequency drives the music tempo

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( drgnwrld_igs012, drgnwrld )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(drgnwrld_igs012)
MACHINE_CONFIG_END



INTERRUPT_GEN_MEMBER(igs011_state::lhb_vblank_irq)
{
	if (!m_lhb_irq_enable)
		return;

	m_maincpu->set_input_line(6, HOLD_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER( igs011_state::lhb_timer_irq_cb )
{
	if (!m_lhb_irq_enable)
		return;

	m_maincpu->set_input_line(5, HOLD_LINE);
}

static MACHINE_CONFIG_DERIVED( lhb, igs011_base )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(lhb)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", igs011_state, lhb_vblank_irq)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_irq", igs011_state, lhb_timer_irq_cb, attotime::from_hz(240)) // lev5 frequency drives the music tempo
	// irq 3 points to an apparently unneeded routine
MACHINE_CONFIG_END



TIMER_DEVICE_CALLBACK_MEMBER( igs011_state::lev3_timer_irq_cb )
{
	m_maincpu->set_input_line(3, HOLD_LINE);
}


static MACHINE_CONFIG_DERIVED( wlcc, igs011_base )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(wlcc)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", igs011_state, irq6_line_hold)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_irq", igs011_state, lev3_timer_irq_cb, attotime::from_hz(240)) // lev3 frequency drives the music tempo
MACHINE_CONFIG_END



static MACHINE_CONFIG_DERIVED( xymg, igs011_base )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(xymg)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", igs011_state, irq6_line_hold)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_irq", igs011_state, lev3_timer_irq_cb, attotime::from_hz(240)) // lev3 frequency drives the music tempo
MACHINE_CONFIG_END



static MACHINE_CONFIG_DERIVED( lhb2, igs011_base )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(lhb2)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", igs011_state, irq6_line_hold)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_irq", igs011_state, lev5_timer_irq_cb, attotime::from_hz(240)) // lev5 frequency drives the music tempo

//  MCFG_GFXDECODE_ADD("gfxdecode", "palette", igs011_hi)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.0)
MACHINE_CONFIG_END



static MACHINE_CONFIG_DERIVED( nkishusp, igs011_base )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(nkishusp)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", igs011_state, irq6_line_hold)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_irq", igs011_state, lev3_timer_irq_cb, attotime::from_hz(240)) // lev3 frequency drives the music tempo

	// VSync 60.0052Hz, HSync 15.620kHz

//  MCFG_GFXDECODE_ADD("gfxdecode", "palette", igs011_hi)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.0)
MACHINE_CONFIG_END



WRITE_LINE_MEMBER(igs011_state::sound_irq)
{
//   m_maincpu->set_input_line(3, state);
}

static MACHINE_CONFIG_DERIVED( vbowl, igs011_base )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(vbowl)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", igs011_state, irq6_line_hold)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_irq", igs011_state, lev3_timer_irq_cb, attotime::from_hz(240)) // lev3 frequency drives the music tempo
	// irq 5 points to a debug function (all routines are clearly patched out)
	// irq 4 points to an apparently unneeded routine

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VBLANK_DRIVER(igs011_state, screen_eof_vbowl)
//  MCFG_GFXDECODE_ADD("gfxdecode", "palette", igs011_hi)

	MCFG_DEVICE_REMOVE("oki")
	MCFG_ICS2115_ADD("ics", 0)
	MCFG_ICS2115_IRQ_CB(WRITELINE(igs011_state, sound_irq))
//  MCFG_SOUND_ADD("ics", ICS2115, 0)
//  MCFG_SOUND_CONFIG(vbowl_ics2115_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 5.0)
MACHINE_CONFIG_END



/***************************************************************************

    ROMs Loading

***************************************************************************/

/***************************************************************************

Dragon World (World, V040O)
(C) 1997 IGS / ALTA

Chips:
  1x 68000 (main)
  1x AC0A26 (equivalent to OKI M6295)(sound)
  1x 6564L (equivalent to YM3812)(sound)
  1x custom IGS003c (marked on PCB as 8255)
  1x oscillator 22.0000MHz (main)
  1x oscillator 3.579545MHz (sound)
  1x custom IGS011 (FPGA?)

ROMs:
  1x MX27C4096 (u3)(main) (dumped)
  1x custom IGSD0301 (mask rom) (not dumped yet)
  1x NEC D27C2001D (IGSS0302)(sound) (not dumped yet)

Notes:
  1x JAMMA edge connector
  1x trimmer (volume)
  3x 8 switches dips
  PCB serial number is: 0105-5

***************************************************************************/

ROM_START( drgnwrld )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "chinadr-v0400.u3", 0x00000, 0x80000, CRC(a6daa2b8) SHA1(0cbfd001c1fd82a6385453d1c2a808add67746af) )

	ROM_REGION( 0x400000, "blitter", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )
ROM_END

/***************************************************************************

Dragon World (World, V030O)
(C) 1995 IGS

Chips:
  1x MC68HC000P10           U2 (main)
  1x CUSTOM IGS011          U24
  1x oscillator 22.000MHz   U25
  1x oscillator 3.579545MHz X1

ROMs:
  1x MX27C4096              U3 (main)

Notes:
  1x JAMMA edge connector
  1x trimmer (volume)
  3x 8 switches dips
  PCB serial number is: 0105-1

***************************************************************************/

ROM_START( drgnwrldv30 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "chinadr-v0300.u3", 0x00000, 0x80000, CRC(5ac243e5) SHA1(50cccff0307239187ac2b65331ad2bcc666f8033) )

	ROM_REGION( 0x400000, "blitter", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )
ROM_END

/***************************************************************************

Dragon World (World, V021O)
(C) 1995 IGS

Chips:
    1x MC68HC000P10             u2      16/32-bit Microprocessor - main
    1x AR17961-AP0642           u41     4-Channel Mixing ADCPM Voice Synthesis LSI - sound
    1x 6564L                    u40     FM Operator Type-L II (OPL II) - sound
    1x LS138S                   u42     sound
    1x LM7805CV                 u38     sound
    1x UPC1242H                 u46     sound
    1x IGS003                   u10     Programmable Peripheral Interface
    1x oscillator   22.0000MHz  u25
    1x oscillator   3.579545MHz x1

ROMs:
    1x  M27C4096                u3      (main) dumped
    1x  custom IGSD0301         u39     not dumped yet
    x   NEC D27C2001D           u43     (sound) dumped

RAMs:
    2x  UM6264                  u4,u5
    2x  CXK5863AP               u31,u32

PLDs:
    1x  custom IGS011 (FPGA?)   u24     not dumped
    1x  custom IGS012 (FPGA?)   u1      not dumped
    2x  PAL16L8A                u17,u18 read protected
    2x  ATF22V10B               u15,u45 read protected
    1x  ATV750                  u16     read protected

***************************************************************************/

ROM_START( drgnwrldv21 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "china-dr-v-0210.u3", 0x00000, 0x80000, CRC(60c2b018) SHA1(58563e3ccb51bd9d8362aa17c23743bb5a593c3b) )

	ROM_REGION( 0x400000, "blitter", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "china-dr-sp.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )
ROM_END

/***************************************************************************

Chuugokuryuu (china dragon jpn ver.)
(c)IGS
Distributed by ALTA

MAIN CPU   : 68000
I/O        : IGS003 (=8255)
SOUND ?    : 6564L  (=OPL?)  , AR17961 (=M6295?)
CRTC ?     : IGS011
SOUND CPU? : IGSD0301 (DIP 42P)
OSC        : 22Mhz , 3.579545Mhz
DIPSW      : 8bitx 3 (SW3 is not used)
OTHER      : IGS012

MAIN PRG   : "CHINA DRAGON U020J" (japan)
SOUND PRG? : "CHINA DRAGON SP"
SOUND DATA?: "CHINA DRAGON U44"

***************************************************************************/

ROM_START( drgnwrldv20j )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "china_jp.v20", 0x00000, 0x80000, CRC(9e018d1a) SHA1(fe14e6344434cabf43685e50fd49c90f05f565be) )

	ROM_REGION( 0x420000, "blitter", 0 )
	// igs-d0301.u39 wasn't in this set
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )
	ROM_LOAD( "china.u44",     0x400000, 0x020000, CRC(10549746) SHA1(aebd83796679c85b43ad514b2771897f94e61294) ) // 1xxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) ) // original label: "sp"
ROM_END

ROM_START( drgnwrldv21j )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "v-021j", 0x00000, 0x80000, CRC(2f87f6e4) SHA1(d43065b078fdd9605c121988ad3092dce6cf0bf1) )

	ROM_REGION( 0x420000, "blitter", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )
	ROM_LOAD( "cg",            0x400000, 0x020000, CRC(2dda0be3) SHA1(587b7cab747d4336515c98eb3365341bb6c7e5e4) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) ) // original label: "sp"
ROM_END

/***************************************************************************

    Dong Fang Zhi Zhu (Hong Kong version of Zhong Guo Long, V011H)

***************************************************************************/

ROM_START( drgnwrldv11h )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "c_drgn_hk.u3", 0x00000, 0x80000, CRC(182037ce) SHA1(141b698777533e57493e588d2526523d4bd3e17d) )

	ROM_REGION( 0x400000, "blitter", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )
ROM_END

/***************************************************************************

Zhong Guo Long (China, V010C)
(C) 1995 IGS

Chips:
  CPU 1x MC68HC000P10 (main)
  1x AR17961-AP0642 (equivalent to OKI M6295)(sound)
  1x 6564L (equivalent to YM3812)(sound)
  1x LS138S (sound)
  1x LM7805CV (sound)
  1x UPC1242H (sound)
  1x custom IGS003 (marked on PCB as 8255)
  1x oscillator 22.0000MHz (main)
  1x oscillator 3.579545MHz (sound)
  1x custom IGS011 (FPGA?)

ROMs:
  1x maskrom 256x16 IGSD0303 (u3)(main)
  1x maskrom 2Mx16 UM23V32000 (IGSD0301)(u39)(gfx)
  1x empty socket for 27C040 (u44)
  1x maskrom NEC D27C2001D (IGSS0302)(u43)(sound)
  2x PAL16L8ACN (u17,u18)(read protected)
  2x PALATF22V10B (u15,u45)
  1x empty space for additional PALATV750 (u16)

Notes:
  1x JAMMA edge connector
  1x trimmer (volume)
  3x 8x2 switches DIP

The PCB is perfectly working, empty spaces and empty sockets are clearly intended to be empty.
25/07/2007 f205v Corrado Tomaselli Gnoppi

***************************************************************************/

ROM_START( drgnwrldv10c )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "igs-d0303.u3", 0x00000, 0x80000, CRC(3b3c29bb) SHA1(77b7e58104314303985c283cce3aec40bd7b9334) )

	ROM_REGION( 0x400000, "blitter", 0 )
	//ROM_LOAD( "igs-0301.u39", 0x000000, 0x400000, CRC(655ab941) SHA1(4bbefb27e8971446998508969661042c5111bc72) ) // bad dump
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )

	ROM_REGION( 0x40000, "plds", 0 )
	ROM_LOAD( "ccdu15.u15", 0x000, 0x2e5, CRC(a15fce69) SHA1(3e38d75c7263bfb36aebdbbd55ebbdd7ca601633) )
	//ROM_LOAD( "ccdu17.u17.bad.dump", 0x000, 0x104, CRC(e9cd78fb) SHA1(557d3e7ef3b25c1338b24722cac91bca788c02b8) )
	//ROM_LOAD( "ccdu18.u18.bad.dump", 0x000, 0x104, CRC(e9cd78fb) SHA1(557d3e7ef3b25c1338b24722cac91bca788c02b8) )
	ROM_LOAD( "ccdu45.u45", 0x000, 0x2e5, CRC(a15fce69) SHA1(3e38d75c7263bfb36aebdbbd55ebbdd7ca601633) )
ROM_END

/***************************************************************************

    Wan Li Chang Cheng (The Great Wall)

    Other files in the zip:

     5.942    16-6126.G16
    14.488    U3-9911.G22
    14.488    U4-82E6.G22
    14.488    U5-6C5E.G22

***************************************************************************/

ROM_START( wlcc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "wlcc4096.rom", 0x00000, 0x80000, CRC(3b16729f) SHA1(4ef4e5cbd6ccc65775e36c2c8b459bc1767d6574) )
	ROM_CONTINUE        (                 0x00000, 0x80000 ) // 1ST+2ND IDENTICAL

	ROM_REGION( 0x280000, "blitter", 0 )
	ROM_LOAD( "m0201-ig.160", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )
	ROM_LOAD( "wlcc.gfx",     0x200000, 0x080000, CRC(1f7ad299) SHA1(ab0a8fb31906519b9352ba172def48456e8d565c) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "040-c3c2.snd", 0x00000, 0x80000, CRC(220949aa) SHA1(1e0dba168a0687d32aaaed42714ae24358f4a3e7) ) // 2 banks
	ROM_CONTINUE(             0x00000, 0x80000 ) // 1ST+2ND IDENTICAL
ROM_END

/***************************************************************************

    Long Hu Bang (V035C)

    Other files in the zip:

     5.938    16V8.jed
    14.464    LHB-U33.jed
    14.488    LHB-U34.jed
    14.488    LHB-U35.jed

***************************************************************************/

ROM_START( lhb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	// identical to LHB-4096
	ROM_LOAD16_WORD_SWAP( "v305j-409", 0x00000, 0x80000, CRC(701de8ef) SHA1(4a77160f642f4de02fa6fbacf595b75c0d4a505d) )

	ROM_REGION( 0x200000, "blitter", 0 )
	ROM_LOAD( "m0201-ig.160", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )

	ROM_REGION( 0x80000, "oki", 0 )
	// identical to 040-c3c2.snd
	ROM_LOAD( "m0202.snd", 0x00000, 0x80000, CRC(220949aa) SHA1(1e0dba168a0687d32aaaed42714ae24358f4a3e7) ) // 2 banks
	ROM_CONTINUE(          0x00000, 0x80000 ) // 1ST+2ND IDENTICAL
ROM_END

/***************************************************************************

Long Hu Bang (V033C)

PCB Layout
----------

IGS PCB NO-T0093
|---------------------------------------|
|uPD1242H     VOL       DSW5            |
|  IGS_M0202                            |
|             AR17961   DSW4            |
|                             CY7C185   |
|                       DSW3            |
|      8255             DSW2  CY7C185   |
|                       DSW1            |
|1                                      |
|8                           DIP32      |
|W             |-------|                |
|A             |       |     IGS_M0201  |
|Y  BATTERY    |IGS011 |                |
|              |       |          PAL   |
|              |-------|                |
|                         TC524256      |
|    MAJ_V-033C                         |
|                         TC524256      |
|1      6264                            |
|0                        TC524256      |
|W      6264    22.285MHz               |
|A         PAL            TC524256      |
|Y         PAL                          |
| SPDT_SW  PAL      68000               |
|---------------------------------------|
Notes:
      Uses common 10-way/18-way Mahjong pinout
      TC524256 - Toshiba TC524256BZ-80 256k x4 Dual Port VRAM (ZIP28)
      CY7C185  - Cypress CY7C185-20PC 8k x8 SRAM (DIP28)
      6264     - UT6264PC-70LL 8k x8 SRAM (DIP28)
      IGS011   - Custom IGS IC (QFP160)
      AR17961  - == OkiM6295 (QFP44)
      DIP32    - Empty socket, maybe a ROM missing, maybe not used?

      ROMs -
            MAJ_V-033C - Main Program (27C4096)
            IGS_M0201  - Graphics (16M maskROM)
            IGS_M0202  - OKI samples (4M maskROM)

***************************************************************************/

ROM_START( lhbv33c )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "maj_v-033c.u30", 0x00000, 0x80000, CRC(02a0b716) SHA1(cd0ee32ea69f66768196b0e9b4df0fae3af84ed3) )

	ROM_REGION( 0x200000, "blitter", 0 )
	ROM_LOAD( "igs_m0201.u15", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )

	ROM_REGION( 0x80000, "oki", 0 )
	// identical to 040-c3c2.snd
	ROM_LOAD( "igs_m0202.u39", 0x00000, 0x80000, CRC(106ac5f7) SHA1(5796a880c3424e3d2251b2223a0e594957afecaf) ) // 2 banks

ROM_END

/***************************************************************************

Da Ban Cheng

PCB Layout
----------

IGS PCB NO-T0084-1
|---------------------------------------|
|uPD1242H     VOL       DSW5            |
|  IGS_M0202                            |
|             AR17961   DSW4            |
|                             CY7C185   |
|                       DSW3            |
|      8255             DSW2  CY7C185   |
|                       DSW1            |
|1                                      |
|8                           MAJ-H_CG   |
|W    PAL      |-------|                |
|A             |       |     IGS_M0201  |
|Y    PAL      |IGS011 |                |
|              |       |          PAL   |
|     PAL      |-------|                |
|                         TC524256      |
|     6264                              |
|                         TC524256      |
|1    6264                              |
|0           22.0994MHz   TC524256      |
|W    MAJ-H_V027H                       |
|A                        TC524256      |
|Y         BATTERY                      |
| SPDT_SW           68000               |
|---------------------------------------|
Notes:
      Uses common 10-way/18-way Mahjong pinout
      TC524256 - Toshiba TC524256BZ-80 256k x4 Dual Port VRAM (ZIP28)
      CY7C185  - Cypress CY7C185-20PC 8k x8 SRAM (DIP28)
      6264     - UT6264PC-70LL 8k x8 SRAM (DIP28)
      IGS011   - Custom IGS IC (QFP160)
      AR17961  - == OkiM6295 (QFP44)

      ROMs -
            MAJ-H_V027H- Main Program (27C4096)
            IGS_M0201  - Graphics (16M maskROM)
            IGS_M0202  - OKI samples (4M maskROM)
            MAJ-H_CG   - Graphics (27c4001 EPROM)

***************************************************************************/

ROM_START( dbc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "maj-h_v027h.u30", 0x00000, 0x80000, CRC(5d5ccd5b) SHA1(7a1223923f9a5825fd919ae9a36912284e705382) )

	ROM_REGION( 0x280000, "blitter", 0 )
	ROM_LOAD( "igs_m0201.u15", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )
	ROM_LOAD( "maj-h_cg.u8",   0x200000, 0x080000, CRC(ee45cc46) SHA1(ed011f758a02026222994aaea0677a4e9580fbda) )   // 1xxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igs_m0202.u39", 0x00000, 0x80000, CRC(106ac5f7) SHA1(5796a880c3424e3d2251b2223a0e594957afecaf) ) // 2 banks
ROM_END

/***************************************************************************

Mahjong Ryukobou
Alta, 1995

PCB Layout
----------

IGS PCB NO-T0094
|------------------------------------|
|UPC1242  VOL  LM7805  DSW5          |
|   MAJ-J_SP           DSW4          |
|  TD62003     M6295   DSW3 CY7C185  |
|        8255          DSW2 CY7C185  |
|M                     DSW1 MAJ-J_CG |
|A                                   |
|H              IGS011      IGS_M0201|
|J                               PAL |
|O                                   |
|N  MAJ_V030J     22MHz              |
|G    6264              TC524256     |
|     6264              TC524256     |
|     PAL               TC524256     |
|     PAL      68000P10 TC524256     |
|     PAL                            |
|------------------------------------|
Notes:
      68000 - clock 7.33333MHz [22/3]
      M6295 - clock 1.04762MHz [22/21]
      VSync - 60.0078Hz
      HSync - 15.620kHz

***************************************************************************/

ROM_START( ryukobou )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "maj_v030j.u30", 0x000000, 0x80000, CRC(186f2b4e) SHA1(380a3d94722d9f5fa5ec206ed0af6dbb8dd81715) )

	ROM_REGION( 0x280000, "blitter", 0 )
	ROM_LOAD( "igs_m0201.u15", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )
	ROM_LOAD( "maj-j_cg.u8",   0x200000, 0x080000, CRC(3c8de5d1) SHA1(51e43bfe7b5157112ded9a34b7032f73b7ffad11) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "maj-j_sp.u39", 0x000000, 0x80000, CRC(82f670f3) SHA1(d10dd67e40aee0e1c4f4b2c1217ce0935cb86406) )
ROM_END

/***************************************************************************

Long Hu Bang II
IGS, 1996

PCB Layout
----------

IGS PCB NO-0115
|---------------------------------------------|
|                  M6295  IGSS0503.U38        |
|  UM3567  3.57945MHz                         |
|                          DSW3               |
|                          DSW2     PAL       |
| IGSM0502.U5              DSW1    6264       |
| IGSM0501.U7     PAL              6264       |
|                 PAL                         |
|                 PAL            IGS011       |
|                 PAL                         |
|                 PAL                         |
|                                             |
|   MC68HC000P10          22MHz  TC524258AZ-10|
|           6264         8255    TC524258AZ-10|
|    BATT   6264   MAJ2V185H.U29 TC524258AZ-10|
|                                TC524258AZ-10|
|---------------------------------------------|

Notes:
        68k clock: 7.3333MHz (i.e. 22/3)
      M6295 clock: 1.0476MHz (i.e. 22/21) \
         M6295 SS: HIGH                   / Therefore sampling freq = 7.936363636kHz (i.e. 1047600 / 132)
           UM3567: Compatible with YM2413, clock = 3.57945MHz
            HSync: 15.78kHz
            VSync: 60Hz

***************************************************************************/

ROM_START( lhb2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "maj2v185h.u29", 0x00000, 0x80000, CRC(2572d59a) SHA1(1d5362e209dadf8b21c10d1351d4bb038bfcaaef) )

	ROM_REGION( 0x200000, "blitter", 0 )
	ROM_LOAD( "igsm0501.u7", 0x00000, 0x200000, CRC(1c952bd6) SHA1(a6b6f1cdfb29647e81c032ffe59c94f1a10ceaf8) )

	ROM_REGION( 0x80000, "blitter_hi", 0 ) // high order bit of graphics (5th bit)
	/* these are identical ..seems ok as igs number is same, only ic changed */
	ROM_LOAD( "igsm0502.u4", 0x00000, 0x80000, CRC(5d73ae99) SHA1(7283aa3d6b15ceb95db80756892be46eb997ef15) )
	ROM_LOAD( "igsm0502.u5", 0x00000, 0x80000, CRC(5d73ae99) SHA1(7283aa3d6b15ceb95db80756892be46eb997ef15) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igss0503.u38", 0x00000, 0x80000, CRC(c9609c9c) SHA1(f036e682b792033409966e84292a69275eaa05e5) )  // 2 banks
ROM_END

/***************************************************************************

Mahjong Nenrikishu SP (V250J)
IGS/Alta, 1998

PCB Layout
----------

IGS PCB NO-0115-5
|----------------------------------|
|UPC1242H VOL M6295     SP         |
|   U3567 LM7805                   |
|   CG   3.579545MHz   DSW3        |
|  M0502               DSW2   PAL  |
|M                     DSW1  6264  |
|A M0501                     6264  |
|H              PAL        IGS011  |
|J              PAL                |
|O              PAL                |
|N              PAL                |
|G                                 |
|                   22MHz M5M442256|
|     68000P16            M5M442256|
|RESET    6264     8255   M5M442256|
| BATT    6264  V250J     M5M442256|
|----------------------------------|
Notes:
      68000 - Clock 7.3333MHz [22/2]
      U3567 - YM2413, clock 3.579545MHz
      M6295 - Clock 1.0476 [22/21]. pin 7 high
      VSync - 60.0052Hz
      HSync - 15.620kHz

***************************************************************************/

ROM_START( nkishusp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "v250j.u29", 0x00000, 0x80000, CRC(500cb919) SHA1(76eed80b59c43e8cc1e258056cfe08b33a651852) )

	ROM_REGION( 0x800000, "blitter", ROMREGION_ERASEFF )
	ROM_LOAD( "m0501.u7", 0x000000, 0x200000, CRC(1c952bd6) SHA1(a6b6f1cdfb29647e81c032ffe59c94f1a10ceaf8) ) // Identical to igsm0501.u7
	// empty
	ROM_LOAD( "cg.u4",    0x400000, 0x080000, CRC(fe60f485) SHA1(d75e5f7a187161137a7f7b54d495d1cb3e1802a4) )
	// empty

	ROM_REGION( 0x200000, "blitter_hi", ROMREGION_ERASEFF ) // high order bit of graphics (5th bit)
	ROM_LOAD( "m0502.u6", 0x00000, 0x80000, CRC(5d73ae99) SHA1(7283aa3d6b15ceb95db80756892be46eb997ef15) ) // Identical to igsm0502.u4, igsm0502.u5
	// empty

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "sp.u38", 0x00000, 0x80000, CRC(d80e28e2) SHA1(c03441686e770227db6a2a41922fbb4284710571) )
ROM_END

/***************************************************************************

Virtua Bowling by IGS

PCB # 0101

U45  (27c240) is probably program
next to 68000 processor
U68,U69 probably images   (27c800 - mask)
U67, U66 sound and ????  (27c040)

ASIC chip used

SMD - custom chip IGS 011      F5XD  174
SMD - custom --near sound section - unknown -- i.d. rubbed off
SMD - custom  -- near inputs and 68000  IGS012    9441EK001

XTL near sound 33.868mhz
XTL near 68000  22.0000mhz

there are 4 banks of 8 dip switches

***************************************************************************/

ROM_START( vbowl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "bowlingv101xcm.u45", 0x00000, 0x80000, BAD_DUMP CRC(ab8e3f1f) SHA1(69159e22559d6a26fe2afafd770aa640c192ba4b) )

	ROM_REGION( 0x400000 * 2, "blitter", 0)
	ROM_LOAD( "vrbowlng.u69", 0x000000, 0x400000, CRC(b0d339e8) SHA1(a26a5e0202a78e8cdc562b10d64e14eadfa4e115) )
	// extra space to expand every 4 bits to 8

	ROM_REGION( 0x100000, "blitter_hi", ROMREGION_INVERT )
	ROM_LOAD( "vrbowlng.u68", 0x000000, 0x100000, CRC(b0ce27e7) SHA1(6d3ef97edd606f384b1e05b152fbea12714887b7) )

	ROM_REGION( 0x400000, "ics", 0 )
	ROM_LOAD( "vrbowlng.u67", 0x00000, 0x80000, CRC(53000936) SHA1(e50c6216f559a9248c095bdfae05c3be4be79ff3) )  // 8 bit signed mono & u-law
	ROM_LOAD( "vrbowlng.u66", 0x80000, 0x80000, CRC(f62cf8ed) SHA1(c53e47e2c619ed974ad40ee4aaa4a35147ea8311) )  // 8 bit signed mono
	ROM_COPY( "ics", 0, 0x100000,0x100000)
	ROM_COPY( "ics", 0, 0x200000,0x100000)
	ROM_COPY( "ics", 0, 0x300000,0x100000)
ROM_END

ROM_START( vbowlj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "vrbowlng.u45", 0x00000, 0x80000, CRC(091c19c1) SHA1(5a7bfbee357122e9061b38dfe988c3853b0984b0) ) // second half all 00

	ROM_REGION( 0x400000 * 2, "blitter", 0)
	ROM_LOAD( "vrbowlng.u69", 0x000000, 0x400000, CRC(b0d339e8) SHA1(a26a5e0202a78e8cdc562b10d64e14eadfa4e115) )
	// extra space to expand every 4 bits to 8

	ROM_REGION( 0x100000, "blitter_hi", ROMREGION_INVERT )
	ROM_LOAD( "vrbowlng.u68", 0x000000, 0x100000, CRC(b0ce27e7) SHA1(6d3ef97edd606f384b1e05b152fbea12714887b7) )

	ROM_REGION( 0x400000, "ics", 0 )
	ROM_LOAD( "vrbowlng.u67", 0x00000, 0x80000, CRC(53000936) SHA1(e50c6216f559a9248c095bdfae05c3be4be79ff3) )  // 8 bit signed mono & u-law
	ROM_LOAD( "vrbowlng.u66", 0x80000, 0x80000, CRC(f62cf8ed) SHA1(c53e47e2c619ed974ad40ee4aaa4a35147ea8311) )  // 8 bit signed mono
	ROM_COPY( "ics", 0, 0x100000,0x100000)
	ROM_COPY( "ics", 0, 0x200000,0x100000)
	ROM_COPY( "ics", 0, 0x300000,0x100000)
ROM_END

/***************************************************************************

    Xing Yen Man Guan

    Other files in the zip:

    14.484 U33-82E6.jed
    14.484 U34-1.jed
    14.484 U35-7068.jed

***************************************************************************/

ROM_START( xymg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "u30-ebac.rom", 0x00000, 0x80000, CRC(7d272b6f) SHA1(15fd1be23cabdc77b747541f5cd9fed6b08be4ad) )

	ROM_REGION( 0x280000, "blitter", 0 )
	ROM_LOAD( "m0201-ig.160", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )
	ROM_LOAD( "ygxy-u8.rom",  0x200000, 0x080000, CRC(56a2706f) SHA1(98bf4b3153eef53dd449e2538b4b7ff2cc2fe6fa) )

	ROM_REGION( 0x80000, "oki", 0 )
	// identical to 040-c3c2.snd
	ROM_LOAD( "m0202.snd", 0x00000, 0x80000, CRC(220949aa) SHA1(1e0dba168a0687d32aaaed42714ae24358f4a3e7) ) // 2 banks
	ROM_CONTINUE(          0x00000, 0x80000 ) // 1ST+2ND IDENTICAL
ROM_END


/***************************************************************************

    Game Drivers

***************************************************************************/

GAME( 1997, drgnwrld,     0,        drgnwrld,        drgnwrld, igs011_state,  drgnwrld,     ROT0, "IGS",        "Dragon World (World, V040O)",          MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv30,  drgnwrld, drgnwrld,        drgnwrld, igs011_state,  drgnwrldv30,  ROT0, "IGS",        "Dragon World (World, V030O)",          MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv21,  drgnwrld, drgnwrld_igs012, drgnwrld, igs011_state,  drgnwrldv21,  ROT0, "IGS",        "Dragon World (World, V021O)",          MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv21j, drgnwrld, drgnwrld_igs012, drgnwrldj, igs011_state, drgnwrldv21j, ROT0, "IGS / Alta", "Zhong Guo Long (Japan, V021J)",        MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv20j, drgnwrld, drgnwrld_igs012, drgnwrldj, igs011_state, drgnwrldv20j, ROT0, "IGS / Alta", "Zhong Guo Long (Japan, V020J)",        MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv10c, drgnwrld, drgnwrld,        drgnwrldc, igs011_state, drgnwrldv10c, ROT0, "IGS",        "Zhong Guo Long (China, V010C)",        MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv11h, drgnwrld, drgnwrld,        drgnwrldc, igs011_state, drgnwrldv11h, ROT0, "IGS",        "Dong Fang Zhi Zhu (Hong Kong, V011H)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, lhb,          0,        lhb,             lhb, igs011_state,       lhb,          ROT0, "IGS",        "Long Hu Bang (China, V035C)",          MACHINE_SUPPORTS_SAVE )
GAME( 1995, lhbv33c,      lhb,      lhb,             lhb, igs011_state,       lhbv33c,      ROT0, "IGS",        "Long Hu Bang (China, V033C)",          MACHINE_SUPPORTS_SAVE )
GAME( 1995, dbc,          lhb,      lhb,             lhb, igs011_state,       dbc,          ROT0, "IGS",        "Da Ban Cheng (Hong Kong, V027H)",      MACHINE_SUPPORTS_SAVE )
GAME( 1995, ryukobou,     lhb,      lhb,             lhb, igs011_state,       ryukobou,     ROT0, "IGS / Alta", "Mahjong Ryukobou (Japan, V030J)",      MACHINE_SUPPORTS_SAVE )
GAME( 1996, lhb2,         0,        lhb2,            lhb2, igs011_state,      lhb2,         ROT0, "IGS",        "Long Hu Bang II (Hong Kong, V185H)",   MACHINE_SUPPORTS_SAVE )
GAME( 1996, xymg,         0,        xymg,            xymg, igs011_state,      xymg,         ROT0, "IGS",        "Xing Yun Man Guan (China, V651C)",     MACHINE_SUPPORTS_SAVE )
GAME( 1996, wlcc,         xymg,     wlcc,            wlcc, igs011_state,      wlcc,         ROT0, "IGS",        "Wan Li Chang Cheng (China, V638C)",    MACHINE_SUPPORTS_SAVE )
GAME( 1996, vbowl,        0,        vbowl,           vbowl, igs011_state,     vbowl,        ROT0, "IGS",        "Virtua Bowling (World, V101XCM)",      MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
GAME( 1996, vbowlj,       vbowl,    vbowl,           vbowlj, igs011_state,    vbowlj,       ROT0, "IGS / Alta", "Virtua Bowling (Japan, V100JCM)",      MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
GAME( 1998, nkishusp,     lhb2,     nkishusp,        nkishusp, igs011_state,  nkishusp,     ROT0, "IGS / Alta", "Mahjong Nenrikishu SP (Japan, V250J)", MACHINE_SUPPORTS_SAVE )
