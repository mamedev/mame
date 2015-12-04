// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/********************************************************************************************************************************

    Sharp MZ-2500 (c) 1985 Sharp Corporation

    driver by Angelo Salese

    TODO:
    - Kanji text is cutted in half when font_size is 1 / interlace is disabled, different ROM used? (check Back to the Future);
    - Implement external ROM hook-up;
    - Add remaining missing peripherals, SIO, HDD and w1300a network;
    - FDC loading without the IPLPRO doesn't work at all, why?
    - reverse / blanking tvram attributes;
    - clean-ups! ^^'

    per-game/program specific TODO:
    - Dust Box vol. 1-3: they die with text garbage, might be bad dumps;
    - Dust Box vol. 4: window effect transition is bugged;
    - Dust Box vol. n: three items returns "purple" text, presumably HW failures (DFJustin: joystick "digital", mouse "not installed", HDD "not installed");
    - LayDock: hangs at title screen due of a PIT bug (timer irq dies for whatever reason);
    - Moon Child: needs mixed 3+3bpp tvram supported, kludged for now (not a real test case);
    - Moon Child: window masking doesn't mask bottom part of the screen?
    - Moon Child: appears to be a network / system link game, obviously doesn't work with current MAME / MESS framework;
    - Marchen Veil I: doesn't load if you try to run it directly, it does if you load another game first (for example Mappy) then do a soft reset;
    - Mugen no Shinzou II - The Prince of Darkness: dies on IPLPRO loading, presumably a wd17xx core bug;
    - Multiplan: random hangs/crashes after you set the RTC, sometimes it loads properly;
    - Murder Club: has lots of CG artifacts, FDC issue?
    - Orrbit 3: floppy issue makes it to throw a game over as soon as you start a game;
    - Penguin Kun Wars: has a bug with window effects ("Push space or trigger" msg on the bottom"), needs investigation;
    - Sound Gal Music Editor: wants a "master disk", that apparently isn't available;
    - Yukar K2 (normal version): moans about something, DFJustin: "please put the system disk back to normal", disk write-protected?

    memory map:
    0x00000-0x3ffff Work RAM
    0x40000-0x5ffff CG RAM
    0x60000-0x67fff "Read modify write" area (related to the CG RAM) (0x30-0x33)
    0x68000-0x6ffff IPL ROM (0x34-0x37)
    0x70000-0x71fff TVRAM (0x38)
    0x72000-0x73fff Kanji ROM / PCG RAM (banked) (0x39)
    0x74000-0x75fff Dictionary ROM (banked) (0x3a)
    0x76000-0x77fff NOP (0x3b)
    0x78000-0x7ffff Phone ROM (0x3c-0x3f)

********************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"
#include "machine/i8255.h"
#include "machine/wd_fdc.h"
#include "machine/pit8253.h"
#include "sound/2203intf.h"
#include "sound/beep.h"
#include "machine/rp5c15.h"
#include "softlist.h"

//#include "imagedev/cassette.h"
#include "imagedev/flopdrv.h"

#define RP5C15_TAG      "rp5c15"

class mz2500_state : public driver_device
{
public:
	mz2500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rtc(*this, RP5C15_TAG),
		m_pit(*this, "pit"),
		m_beeper(*this, "beeper"),
		m_gfxdecode(*this, "gfxdecode"),
		m_fdc(*this, "mb8877a"),
		m_floppy0(*this, "mb8877a:0"),
		m_floppy1(*this, "mb8877a:1"),
		m_floppy2(*this, "mb8877a:2"),
		m_floppy3(*this, "mb8877a:3"),
		m_floppy(nullptr),
		m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<rp5c15_device> m_rtc;
	required_device<pit8253_device> m_pit;
	required_device<beep_device> m_beeper;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<mb8877_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;

	floppy_image_device *m_floppy;

	UINT8 *m_main_ram;
	UINT8 *m_ipl_rom;
	UINT8 *m_kanji_rom;
	UINT8 *m_kanji2_rom;
	UINT8 *m_pcg_ram;
	UINT8 *m_emm_ram;
	UINT8 *m_dic_rom;
	UINT8 *m_phone_rom;
	UINT8 *m_iplpro_rom;

	UINT8 m_bank_val[8];
	UINT8 m_bank_addr;
	UINT8 m_irq_sel;
	UINT8 m_irq_vector[4];
	UINT8 m_irq_mask[4];
	UINT8 m_irq_pending[4];
	UINT8 m_kanji_bank;
	UINT8 m_dic_bank;
	UINT8 m_fdc_reverse;
	UINT8 m_key_mux;
	UINT8 m_monitor_type;
	UINT8 m_text_reg[0x100];
	UINT8 m_text_reg_index;
	UINT8 m_text_col_size;
	UINT8 m_text_font_reg;
	UINT8 m_pal_select;
	UINT16 m_cg_vs;
	UINT16 m_cg_ve;
	UINT16 m_cg_hs;
	UINT16 m_cg_he;
	INT16 m_tv_vs;
	INT16 m_tv_ve;
	INT16 m_tv_hs;
	INT16 m_tv_he;
	UINT8 m_cg_latch[4];
	UINT8 m_cg_reg_index;
	UINT8 m_cg_reg[0x20];
	UINT8 m_clut16[0x10];
	UINT16 m_clut256[0x100];
	UINT8 m_cg_mask;
	int m_scr_x_size;
	int m_scr_y_size;
	UINT8 m_cg_clear_flag;
	UINT32 m_rom_index;
	UINT8 m_hrom_index;
	UINT8 m_lrom_index;
	struct { UINT8 r,g,b; } m_pal[16];
	UINT8 m_joy_mode;
	UINT16 m_kanji_index;
	UINT32 m_emm_offset;
	UINT8 m_old_portc;
	UINT8 m_prev_col_val;
	UINT8 m_pio_latchb;
	UINT8 m_ym_porta;
	UINT8 m_screen_enable;
	DECLARE_READ8_MEMBER(bank0_r);
	DECLARE_READ8_MEMBER(bank1_r);
	DECLARE_READ8_MEMBER(bank2_r);
	DECLARE_READ8_MEMBER(bank3_r);
	DECLARE_READ8_MEMBER(bank4_r);
	DECLARE_READ8_MEMBER(bank5_r);
	DECLARE_READ8_MEMBER(bank6_r);
	DECLARE_READ8_MEMBER(bank7_r);
	DECLARE_WRITE8_MEMBER(bank0_w);
	DECLARE_WRITE8_MEMBER(bank1_w);
	DECLARE_WRITE8_MEMBER(bank2_w);
	DECLARE_WRITE8_MEMBER(bank3_w);
	DECLARE_WRITE8_MEMBER(bank4_w);
	DECLARE_WRITE8_MEMBER(bank5_w);
	DECLARE_WRITE8_MEMBER(bank6_w);
	DECLARE_WRITE8_MEMBER(bank7_w);
	DECLARE_READ8_MEMBER(mz2500_bank_addr_r);
	DECLARE_WRITE8_MEMBER(mz2500_bank_addr_w);
	DECLARE_READ8_MEMBER(mz2500_bank_data_r);
	DECLARE_WRITE8_MEMBER(mz2500_bank_data_w);
	DECLARE_WRITE8_MEMBER(mz2500_kanji_bank_w);
	DECLARE_WRITE8_MEMBER(mz2500_dictionary_bank_w);
	DECLARE_READ8_MEMBER(mz2500_crtc_hvblank_r);
	DECLARE_WRITE8_MEMBER(mz2500_tv_crtc_w);
	DECLARE_WRITE8_MEMBER(mz2500_irq_sel_w);
	DECLARE_WRITE8_MEMBER(mz2500_irq_data_w);
	DECLARE_READ8_MEMBER(mz2500_rom_r);
	DECLARE_WRITE8_MEMBER(mz2500_rom_w);
	DECLARE_WRITE8_MEMBER(palette4096_io_w);
	DECLARE_READ8_MEMBER(mz2500_bplane_latch_r);
	DECLARE_READ8_MEMBER(mz2500_rplane_latch_r);
	DECLARE_READ8_MEMBER(mz2500_gplane_latch_r);
	DECLARE_READ8_MEMBER(mz2500_iplane_latch_r);
	DECLARE_WRITE8_MEMBER(mz2500_cg_addr_w);
	DECLARE_WRITE8_MEMBER(mz2500_cg_data_w);
	DECLARE_WRITE8_MEMBER(timer_w);
	DECLARE_READ8_MEMBER(mz2500_joystick_r);
	DECLARE_WRITE8_MEMBER(mz2500_joystick_w);
	DECLARE_READ8_MEMBER(mz2500_kanji_r);
	DECLARE_WRITE8_MEMBER(mz2500_kanji_w);
	DECLARE_READ8_MEMBER(rp5c15_8_r);
	DECLARE_WRITE8_MEMBER(rp5c15_8_w);
	DECLARE_READ8_MEMBER(mz2500_emm_data_r);
	DECLARE_WRITE8_MEMBER(mz2500_emm_addr_w);
	DECLARE_WRITE8_MEMBER(mz2500_emm_data_w);
	UINT8 mz2500_cg_latch_compare();
	UINT8 mz2500_ram_read(UINT16 offset, UINT8 bank_num);
	void mz2500_ram_write(UINT16 offset, UINT8 data, UINT8 bank_num);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(mz2500);
	UINT32 screen_update_mz2500(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(mz2500_vbl);

	DECLARE_READ8_MEMBER(fdc_r);
	DECLARE_WRITE8_MEMBER(fdc_w);
	DECLARE_WRITE8_MEMBER(floppy_select_w);
	DECLARE_WRITE8_MEMBER(floppy_side_w);

	DECLARE_READ8_MEMBER(mz2500_porta_r);
	DECLARE_READ8_MEMBER(mz2500_portb_r);
	DECLARE_READ8_MEMBER(mz2500_portc_r);
	DECLARE_WRITE8_MEMBER(mz2500_porta_w);
	DECLARE_WRITE8_MEMBER(mz2500_portb_w);
	DECLARE_WRITE8_MEMBER(mz2500_portc_w);
	DECLARE_WRITE8_MEMBER(mz2500_pio1_porta_w);
	DECLARE_READ8_MEMBER(mz2500_pio1_porta_r);
	DECLARE_READ8_MEMBER(mz2500_pio1_portb_r);
	DECLARE_READ8_MEMBER(opn_porta_r);
	DECLARE_WRITE8_MEMBER(opn_porta_w);
	DECLARE_WRITE_LINE_MEMBER(pit8253_clk0_irq);
	DECLARE_WRITE_LINE_MEMBER(mz2500_rtc_alarm_irq);
	IRQ_CALLBACK_MEMBER( mz2500_irq_ack );

	void draw_80x25(bitmap_ind16 &bitmap,const rectangle &cliprect,UINT16 map_addr);
	void draw_40x25(bitmap_ind16 &bitmap,const rectangle &cliprect,int plane,UINT16 map_addr);
	void draw_cg4_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int pri);
	void draw_cg16_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int plane,int x_size,int pri);
	void draw_cg256_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int plane,int pri);
	void draw_tv_screen(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void draw_cg_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int pri);

	void mz2500_draw_pixel(bitmap_ind16 &bitmap,int x,int y,UINT16  pen,UINT8 width,UINT8 height);
	void mz2500_reconfigure_screen();
	UINT8 pal_256_param(int index, int param);
	void mz2500_reset(mz2500_state *state, UINT8 type);
	required_device<palette_device> m_palette;
};


/* machine stuff */

#define WRAM_RESET 0
#define IPL_RESET 1

static const UINT8 bank_reset_val[2][8] =
{
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 },
	{ 0x34, 0x35, 0x36, 0x37, 0x04, 0x05, 0x06, 0x07 }
};

/* video stuff*/

void mz2500_state::video_start()
{
}

/*
[0] xxxx xxxx tile
[1] ---- -xxx color offset
[1] ---- x--- PCG combine mode
[1] --xx ---- PCG select
[1] xx-- ---- reverse / blink attributes
[2] --xx xxxx tile bank (for kanji ROMs)
[2] xx-- ---- kanji select
*/

/* helper function, to draw stuff without getting crazy with height / width conditions :) */
void mz2500_state::mz2500_draw_pixel(bitmap_ind16 &bitmap,int x,int y,UINT16  pen,UINT8 width,UINT8 height)
{
	if(width && height)
	{
		bitmap.pix16(y*2+0, x*2+0) = m_palette->pen(pen);
		bitmap.pix16(y*2+0, x*2+1) = m_palette->pen(pen);
		bitmap.pix16(y*2+1, x*2+0) = m_palette->pen(pen);
		bitmap.pix16(y*2+1, x*2+1) = m_palette->pen(pen);
	}
	else if(width)
	{
		bitmap.pix16(y, x*2+0) = m_palette->pen(pen);
		bitmap.pix16(y, x*2+1) = m_palette->pen(pen);
	}
	else if(height)
	{
		bitmap.pix16(y*2+0, x) = m_palette->pen(pen);
		bitmap.pix16(y*2+1, x) = m_palette->pen(pen);
	}
	else
		bitmap.pix16(y, x) = m_palette->pen(pen);
}

void mz2500_state::draw_80x25(bitmap_ind16 &bitmap,const rectangle &cliprect,UINT16 map_addr)
{
	UINT8 *vram = m_main_ram; // TODO
	int x,y,count,xi,yi;
	UINT8 *gfx_data;
	UINT8 y_step;
	UINT8 s_y;
	UINT8 y_height;

	count = (map_addr & 0x7ff);

	y_step = (m_text_font_reg) ? 1 : 2;
	y_height = (m_text_reg[0] & 0x10) ? 10 : 8;
	s_y = m_text_reg[9] & 0xf;

	for (y=0;y<26*y_step;y+=y_step)
	{
		for (x=0;x<80;x++)
		{
			int tile = vram[0x70000+count+0x0000] & 0xff;
			int attr = vram[0x70000+count+0x0800];
			int tile_bank = vram[0x70000+count+0x1000] & 0x3f;
			int gfx_sel = (attr & 0x38) | (vram[0x70000+count+0x1000] & 0xc0);
			int color = attr & 7;
			int inv_col = (attr & 0x40) >> 6;

			if(gfx_sel & 8) // Xevious, PCG 8 colors have priority above kanji roms
				gfx_data = m_pcg_ram;
			else if(gfx_sel == 0x80)
			{
				gfx_data = m_kanji_rom;
				tile|= tile_bank << 8;
				if(y_step == 2)
					tile &= 0x3ffe;
			}
			else if(gfx_sel == 0xc0)
			{
				gfx_data = m_kanji_rom;
				tile|= (tile_bank << 8);
				if(y_step == 2)
					tile &= 0x3ffe;
				tile|=0x4000;
			}
			else
			{
				gfx_data = m_pcg_ram;
			}

			for(yi=0;yi<8*y_step;yi++)
			{
				for(xi=0;xi<8;xi++)
				{
					UINT8 pen_bit[3],pen;
					int res_x,res_y;

					res_x = x*8+xi;
					res_y = y*y_height+yi-s_y;

					/* check TV window boundaries */
					if(res_x < m_tv_hs || res_x >= m_tv_he || res_y < m_tv_vs || res_y >= m_tv_ve)
						continue;

					if(gfx_sel & 0x8)
					{
						pen_bit[0] = ((gfx_data[tile*8+yi+0x1800]>>(7-xi)) & 1) ? (4+8) : 0; //G
						pen_bit[1] = ((gfx_data[tile*8+yi+0x1000]>>(7-xi)) & 1) ? (2+8) : 0; //R
						pen_bit[2] = ((gfx_data[tile*8+yi+0x0800]>>(7-xi)) & 1) ? (1+8) : 0; //B

						pen = (pen_bit[0]|pen_bit[1]|pen_bit[2]);

						//if(inv_col) { pen ^= 7; } breaks Mappy
					}
					else
						pen = (((gfx_data[tile*8+yi+((gfx_sel & 0x30)<<7)]>>(7-xi)) & 1) ^ inv_col) ? (color+8) : 0;

					if(pen)
					{
						if((res_y) >= 0 && (res_y) < 200*y_step)
							mz2500_draw_pixel(bitmap,res_x,res_y,pen+(m_pal_select ? 0x00 : 0x10),0,0);
					}
				}
			}

			count++;
			count&=0x7ff;
		}
	}
}

void mz2500_state::draw_40x25(bitmap_ind16 &bitmap,const rectangle &cliprect,int plane,UINT16 map_addr)
{
	UINT8 *vram = m_main_ram; // TODO
	int x,y,count,xi,yi;
	UINT8 *gfx_data;
	UINT8 y_step;
	UINT8 s_y;
	UINT8 y_height;

	count = (((plane * 0x400) + map_addr) & 0x7ff);

	y_step = (m_text_font_reg) ? 1 : 2;
	y_height = (m_text_reg[0] & 0x10) ? 10 : 8;
	s_y = m_text_reg[9] & 0xf;

	for (y=0;y<26*y_step;y+=y_step)
	{
		for (x=0;x<40;x++)
		{
			int tile = vram[0x70000+count+0x0000] & 0xff;
			int attr = vram[0x70000+count+0x0800];
			int tile_bank = vram[0x70000+count+0x1000] & 0x3f;
			int gfx_sel = (attr & 0x38) | (vram[0x70000+count+0x1000] & 0xc0);
			//int gfx_num;
			int color = attr & 7;
			int inv_col = (attr & 0x40) >> 6;

			if(gfx_sel & 8) // Xevious, PCG 8 colors have priority above kanji roms
				gfx_data = m_pcg_ram;
			else if(gfx_sel == 0x80)
			{
				gfx_data = m_kanji_rom;
				tile|= tile_bank << 8;
				if(y_step == 2)
					tile &= 0x3ffe;
			}
			else if(gfx_sel == 0xc0)
			{
				gfx_data = m_kanji_rom;
				tile|= (tile_bank << 8);
				if(y_step == 2)
					tile &= 0x3ffe;
				tile|=0x4000;
			}
			else
			{
				gfx_data = m_pcg_ram;
			}

			for(yi=0;yi<8*y_step;yi++)
			{
				for(xi=0;xi<8;xi++)
				{
					UINT8 pen_bit[3],pen;
					int res_x,res_y;

					res_x = x*8+xi;
					res_y = y*y_height+yi-s_y;

					/* check TV window boundaries */
					if(res_x < m_tv_hs || res_x >= m_tv_he || res_y < m_tv_vs || res_y >= m_tv_ve)
						continue;

					if(gfx_sel & 0x8)
					{
						pen_bit[0] = ((gfx_data[tile*8+yi+0x1800]>>(7-xi)) & 1) ? (4+8) : 0; //G
						pen_bit[1] = ((gfx_data[tile*8+yi+0x1000]>>(7-xi)) & 1) ? (2+8) : 0; //R
						pen_bit[2] = ((gfx_data[tile*8+yi+0x0800]>>(7-xi)) & 1) ? (1+8) : 0; //B

						pen = (pen_bit[0]|pen_bit[1]|pen_bit[2]);

						//if(inv_col) { pen ^= 7; } breaks Mappy
					}
					else
						pen = (((gfx_data[tile*8+yi+((gfx_sel & 0x30)<<7)]>>(7-xi)) & 1) ^ inv_col) ? (color+8) : 0;

					if(pen)
					{
						if((res_y) >= 0 && (res_y) < 200*y_step)
							mz2500_draw_pixel(bitmap,res_x,res_y,pen+(m_pal_select ? 0x00 : 0x10),m_scr_x_size == 640,0);
					}
				}
			}

			count++;
			count&=0x7ff;
		}
	}
}

void mz2500_state::draw_cg4_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int pri)
{
	UINT32 count;
	UINT8 *vram = m_main_ram; // TODO
	UINT8 pen,pen_bit[2];
	int x,y,xi,pen_i;
	int res_x,res_y;

	count = 0x40000;

	for(y=0;y<400;y++)
	{
		for(x=0;x<640;x+=8)
		{
			for(xi=0;xi<8;xi++)
			{
				res_x = x+xi;
				res_y = y;

				/* check window boundaries */
				//if(res_x < m_cg_hs || res_x >= m_cg_he || res_y < m_cg_vs || res_y >= m_cg_ve)
				//  continue;

				/* TODO: very preliminary, just Yukar K2 uses this so far*/
				pen_bit[0] = (vram[count + 0x00000]>>(xi)) & 1 ? 7 : 0; // B
				pen_bit[1] = (vram[count + 0x0c000]>>(xi)) & 1 ? 7 : 0; // R

				pen = 0;
				for(pen_i=0;pen_i<2;pen_i++)
					pen |= pen_bit[pen_i];

				{
					//if(pri == ((m_clut256[pen] & 0x100) >> 8))
					mz2500_draw_pixel(bitmap,res_x,res_y,pen,0,0);
				}
			}
			count++;
		}
	}
}

void mz2500_state::draw_cg16_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int plane,int x_size,int pri)
{
	UINT32 count;
	UINT8 *vram = m_main_ram; //TODO
	UINT8 pen,pen_bit[4];
	int x,y,xi,pen_i;
	UINT32 wa_reg;
	UINT8 s_x;
	UINT8 base_mask;
	int res_x,res_y;
	UINT8 cg_interlace;
	UINT8 pen_mask;

	base_mask = (x_size == 640) ? 0x3f : 0x1f;

	count = (m_cg_reg[0x10]) | ((m_cg_reg[0x11] & base_mask) << 8);
	wa_reg = (m_cg_reg[0x12]) | ((m_cg_reg[0x13] & base_mask) << 8);
	/* TODO: layer 2 scrolling */
	s_x = (m_cg_reg[0x0f] & 0xf);
	cg_interlace = m_text_font_reg ? 1 : 2;
	pen_mask = (m_cg_reg[0x18] >> ((plane & 1) * 4)) & 0x0f;

//  popmessage("%d %d %d %d",m_cg_hs,m_cg_he,m_cg_vs,m_cg_ve);

	for(y=0;y<200;y++)
	{
		for(x=0;x<x_size;x+=8)
		{
			for(xi=0;xi<8;xi++)
			{
				res_x = x+xi+s_x;
				res_y = y;

				/* check window boundaries */
				if(res_x < m_cg_hs || res_x >= m_cg_he || res_y < m_cg_vs || res_y >= m_cg_ve)
					continue;

				pen_bit[0] = (vram[count+0x40000+((plane & 1) * 0x2000)+(((plane & 2)>>1) * 0x10000)]>>(xi)) & 1 ? (pen_mask & 0x01) : 0; //B
				pen_bit[1] = (vram[count+0x44000+((plane & 1) * 0x2000)+(((plane & 2)>>1) * 0x10000)]>>(xi)) & 1 ? (pen_mask & 0x02) : 0; //R
				pen_bit[2] = (vram[count+0x48000+((plane & 1) * 0x2000)+(((plane & 2)>>1) * 0x10000)]>>(xi)) & 1 ? (pen_mask & 0x04) : 0; //G
				pen_bit[3] = (vram[count+0x4c000+((plane & 1) * 0x2000)+(((plane & 2)>>1) * 0x10000)]>>(xi)) & 1 ? (pen_mask & 0x08) : 0; //I

				pen = 0;
				for(pen_i=0;pen_i<4;pen_i++)
					pen |= pen_bit[pen_i];

				if(pri == ((m_clut16[pen] & 0x10) >> 4) && m_clut16[pen] != 0x00 && pen_mask) //correct?
					mz2500_draw_pixel(bitmap,res_x,res_y,(m_clut16[pen] & 0x0f)+0x10,(x_size == 320 && m_scr_x_size == 640),cg_interlace == 2);
			}
			count++;
			count&=((base_mask<<8) | 0xff);
			if(count > wa_reg)
				count = 0;
		}
	}
}

void mz2500_state::draw_cg256_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int plane,int pri)
{
	UINT32 count;
	UINT8 *vram = m_main_ram;
	UINT8 pen,pen_bit[8];
	int x,y,xi,pen_i;
	UINT32 wa_reg;
	UINT8 s_x;
	UINT8 base_mask;
	int res_x,res_y;
	UINT8 cg_interlace;

	base_mask = 0x3f; //no x_size == 640

	count = (m_cg_reg[0x10]) | ((m_cg_reg[0x11] & base_mask) << 8);
	wa_reg = (m_cg_reg[0x12]) | ((m_cg_reg[0x13] & base_mask) << 8);
	/* TODO: layer 2 scrolling */
	s_x = (m_cg_reg[0x0f] & 0xf);
	cg_interlace = m_text_font_reg ? 1 : 2;

	for(y=0;y<200;y++)
	{
		for(x=0;x<320;x+=8)
		{
			for(xi=0;xi<8;xi++)
			{
				res_x = x+xi+s_x;
				res_y = y;

				/* check window boundaries */
				if(res_x < m_cg_hs || res_x >= m_cg_he || res_y < m_cg_vs || res_y >= m_cg_ve)
					continue;

				pen_bit[0] = (vram[count + 0x40000 + (((plane & 2)>>1) * 0x10000) + 0x2000]>>(xi)) & 1 ? (m_cg_reg[0x18] & 0x10) : 0; // B1
				pen_bit[1] = (vram[count + 0x40000 + (((plane & 2)>>1) * 0x10000) + 0x0000]>>(xi)) & 1 ? (m_cg_reg[0x18] & 0x01) : 0; // B0
				pen_bit[2] = (vram[count + 0x40000 + (((plane & 2)>>1) * 0x10000) + 0x6000]>>(xi)) & 1 ? (m_cg_reg[0x18] & 0x20) : 0; // R1
				pen_bit[3] = (vram[count + 0x40000 + (((plane & 2)>>1) * 0x10000) + 0x4000]>>(xi)) & 1 ? (m_cg_reg[0x18] & 0x02) : 0; // R0
				pen_bit[4] = (vram[count + 0x40000 + (((plane & 2)>>1) * 0x10000) + 0xa000]>>(xi)) & 1 ? (m_cg_reg[0x18] & 0x40) : 0; // G1
				pen_bit[5] = (vram[count + 0x40000 + (((plane & 2)>>1) * 0x10000) + 0x8000]>>(xi)) & 1 ? (m_cg_reg[0x18] & 0x04) : 0; // G0
				pen_bit[6] = (vram[count + 0x40000 + (((plane & 2)>>1) * 0x10000) + 0xe000]>>(xi)) & 1 ? (m_cg_reg[0x18] & 0x80) : 0; // I1
				pen_bit[7] = (vram[count + 0x40000 + (((plane & 2)>>1) * 0x10000) + 0xc000]>>(xi)) & 1 ? (m_cg_reg[0x18] & 0x08) : 0; // I0

				pen = 0;
				for(pen_i=0;pen_i<8;pen_i++)
					pen |= pen_bit[pen_i];

				if(pri == ((m_clut256[pen] & 0x100) >> 8))
					mz2500_draw_pixel(bitmap,res_x,res_y,(m_clut256[pen] & 0xff)+0x100,m_scr_x_size == 640,cg_interlace == 2);
			}
			count++;
			count&=((base_mask<<8) | 0xff);
			if(count > wa_reg)
				count = 0;
		}
	}
}

void mz2500_state::draw_tv_screen(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	UINT16 base_addr;

	base_addr = m_text_reg[1] | ((m_text_reg[2] & 0x7) << 8);

//  popmessage("%02x",m_clut16[0]);
//  popmessage("%d %d %d %d",m_tv_hs,(m_tv_he),m_tv_vs,(m_tv_ve));

	if(m_text_col_size)
		draw_80x25(bitmap,cliprect,base_addr);
	else
	{
		int tv_mode;

		tv_mode = m_text_reg[0] >> 2;

		switch(tv_mode & 3)
		{
			case 0: //mixed 6bpp mode, TODO
				draw_40x25(bitmap,cliprect,0,base_addr);
				break;
			case 1:
				draw_40x25(bitmap,cliprect,0,base_addr);
				break;
			case 2:
				draw_40x25(bitmap,cliprect,1,base_addr);
				break;
			case 3:
				draw_40x25(bitmap,cliprect,1,base_addr);
				draw_40x25(bitmap,cliprect,0,base_addr);
				break;
			//default: popmessage("%02x %02x %02x",tv_mode & 3,m_text_reg[1],m_text_reg[2]); break;
		}
	}
}

void mz2500_state::draw_cg_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int pri)
{
	//popmessage("%02x %02x",m_cg_reg[0x0e],m_cg_reg[0x18]);

	switch(m_cg_reg[0x0e])
	{
		case 0x00:
			break;
		case 0x03:
			draw_cg4_screen(bitmap,cliprect,0);
			break;
		case 0x14:
			draw_cg16_screen(bitmap,cliprect,0,320,pri);
			draw_cg16_screen(bitmap,cliprect,1,320,pri);
			break;
		case 0x15:
			draw_cg16_screen(bitmap,cliprect,1,320,pri);
			draw_cg16_screen(bitmap,cliprect,0,320,pri);
			break;
		case 0x17:
			draw_cg16_screen(bitmap,cliprect,0,640,pri);
			break;
		case 0x1d:
			draw_cg256_screen(bitmap,cliprect,0,pri);
			break;
		case 0x97:
			draw_cg16_screen(bitmap,cliprect,2,640,pri);
			break;
		default:
			popmessage("Unsupported CG mode %02x, contact MESS dev",m_cg_reg[0x0e]);
			break;
	}
}

UINT32 mz2500_state::screen_update_mz2500(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->pen(0), cliprect); //TODO: correct?

	if(m_screen_enable)
		return 0;

	draw_cg_screen(bitmap,cliprect,0);
	draw_tv_screen(bitmap,cliprect);
	draw_cg_screen(bitmap,cliprect,1);
	//  popmessage("%02x (%02x %02x) (%02x %02x) (%02x %02x) (%02x %02x)",m_cg_reg[0x0f],m_cg_reg[0x10],m_cg_reg[0x11],m_cg_reg[0x12],m_cg_reg[0x13],m_cg_reg[0x14],m_cg_reg[0x15],m_cg_reg[0x16],m_cg_reg[0x17]);
	//  popmessage("%02x",m_text_reg[0x0f]);


	return 0;
}

void mz2500_state::mz2500_reconfigure_screen()
{
	rectangle visarea;

	if((m_cg_reg[0x0e] & 0x1f) == 0x17 || (m_cg_reg[0x0e] & 0x1f) == 0x03 || m_text_col_size)
		m_scr_x_size = 640;
	else
		m_scr_x_size = 320;

	if((m_cg_reg[0x0e] & 0x1f) == 0x03)
		m_scr_y_size = 400;
	else
		m_scr_y_size = 200 * ((m_text_font_reg) ? 1 : 2);

	visarea.set(0, m_scr_x_size - 1, 0, m_scr_y_size - 1);

	//popmessage("%d %d %d %d %02x",vs,ve,hs,he,m_cg_reg[0x0e]);

	machine().first_screen()->configure(720, 480, visarea, machine().first_screen()->frame_period().attoseconds());

	/* calculate CG window parameters here */
	m_cg_vs = (m_cg_reg[0x08]) | ((m_cg_reg[0x09]<<8) & 1);
	m_cg_ve = (m_cg_reg[0x0a]) | ((m_cg_reg[0x0b]<<8) & 1);
	m_cg_hs = ((m_cg_reg[0x0c] & 0x7f)*8);
	m_cg_he = ((m_cg_reg[0x0d] & 0x7f)*8);

	if(m_scr_x_size == 320)
	{
		m_cg_hs /= 2;
		m_cg_he /= 2;
	}

	/* calculate TV window parameters here */
	{
		int x_offs,y_offs;

		m_monitor_type = ((m_text_reg[0x0f] & 0x08) >> 3);

		switch((m_monitor_type|m_text_col_size<<1) & 3)
		{
			default:
			case 0: x_offs = 64; break;
			case 1: x_offs = 80; break;
			case 2: x_offs = 72; break;
			case 3: x_offs = 88; break;
		}
		//printf("%d %d %d\n",x_offs,(m_text_reg[7] & 0x7f) * 8,(m_text_reg[8] & 0x7f)* 8);

		y_offs = (m_monitor_type) ? 76 : 34;

		m_tv_hs = ((m_text_reg[7] & 0x7f)*8) - x_offs;
		m_tv_he = ((m_text_reg[8] & 0x7f)*8) - x_offs;
		m_tv_vs = (m_text_reg[3]*2) - y_offs;
		m_tv_ve = (m_text_reg[5]*2) - y_offs;

		if(m_scr_x_size == 320)
		{
			m_tv_hs /= 2;
			m_tv_he /= 2;
		}

		if(m_scr_y_size == 200)
		{
			m_tv_vs /= 2;
			m_tv_ve /= 2;
		}
	}
}

UINT8 mz2500_state::mz2500_cg_latch_compare()
{
	UINT8 compare_val = m_cg_reg[0x07] & 0xf;
	UINT8 pix_val;
	UINT8 res;
	UINT16 i;
	res = 0;

	for(i=1;i<0x100;i<<=1)
	{
		pix_val = ((m_cg_latch[0] & i) ? 1 : 0) | ((m_cg_latch[1] & i) ? 2 : 0) | ((m_cg_latch[2] & i) ? 4 : 0) | ((m_cg_latch[3] & i) ? 8 : 0);
		if(pix_val == compare_val)
			res|=i;
	}

	return res;
}

UINT8 mz2500_state::mz2500_ram_read(UINT16 offset, UINT8 bank_num)
{
	UINT8 *ram = m_main_ram; // TODO
	UINT8 cur_bank = m_bank_val[bank_num];

	switch(cur_bank)
	{
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		{
			// READ MODIFY WRITE
			if(m_cg_reg[0x0e] == 0x3)
			{
				// ...
			}
			else
			{
				int plane;
				m_cg_latch[0] = ram[offset+((cur_bank & 3)*0x2000)+0x40000]; //B
				m_cg_latch[1] = ram[offset+((cur_bank & 3)*0x2000)+0x44000]; //R
				m_cg_latch[2] = ram[offset+((cur_bank & 3)*0x2000)+0x48000]; //G
				m_cg_latch[3] = ram[offset+((cur_bank & 3)*0x2000)+0x4c000]; //I
				plane = m_cg_reg[0x07] & 3;

				if(m_cg_reg[0x07] & 0x10)
					return mz2500_cg_latch_compare();
				else
					return m_cg_latch[plane];
			}
		}
		break;
		case 0x39:
		{
			if(m_kanji_bank & 0x80) //kanji ROM
				return m_kanji_rom[(offset & 0x7ff)+((m_kanji_bank & 0x7f)*0x800)];
			else //PCG RAM
				return m_pcg_ram[offset];
		}
		case 0x3a:
		{
			return m_dic_rom[(offset & 0x1fff) + ((m_dic_bank & 0x1f)*0x2000)];
		}
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
		{
			return m_phone_rom[offset+(cur_bank & 3)*0x2000];
		}
		default: return ram[offset+cur_bank*0x2000];
	}

	// never executed
	return 0xff;
}

void mz2500_state::mz2500_ram_write(UINT16 offset, UINT8 data, UINT8 bank_num)
{
	UINT8 *ram = m_main_ram; // TODO
	UINT8 cur_bank = m_bank_val[bank_num];

//  if(cur_bank >= 0x30 && cur_bank <= 0x33)
//      printf("CG REG = %02x %02x %02x %02x | offset = %04x | data = %02x\n",m_cg_reg[0],m_cg_reg[1],m_cg_reg[2],m_cg_reg[3],offset,data);

	switch(cur_bank)
	{
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		{
			// READ MODIFY WRITE
			if(m_cg_reg[0x0e] == 0x3)
			{
				// ...
			}
			else
			{
				if((m_cg_reg[0x05] & 0xc0) == 0x00) //replace
				{
					if(m_cg_reg[5] & 1) //B
					{
						ram[offset+((cur_bank & 3)*0x2000)+0x40000] &= ~m_cg_reg[6];
						ram[offset+((cur_bank & 3)*0x2000)+0x40000] |= (m_cg_reg[4] & 1) ? (data & m_cg_reg[0] & m_cg_reg[6]) : 0;
					}
					if(m_cg_reg[5] & 2) //R
					{
						ram[offset+((cur_bank & 3)*0x2000)+0x44000] &= ~m_cg_reg[6];
						ram[offset+((cur_bank & 3)*0x2000)+0x44000] |= (m_cg_reg[4] & 2) ? (data & m_cg_reg[1] & m_cg_reg[6]) : 0;
					}
					if(m_cg_reg[5] & 4) //G
					{
						ram[offset+((cur_bank & 3)*0x2000)+0x48000] &= ~m_cg_reg[6];
						ram[offset+((cur_bank & 3)*0x2000)+0x48000] |= (m_cg_reg[4] & 4) ? (data & m_cg_reg[2] & m_cg_reg[6]) : 0;
					}
					if(m_cg_reg[5] & 8) //I
					{
						ram[offset+((cur_bank & 3)*0x2000)+0x4c000] &= ~m_cg_reg[6];
						ram[offset+((cur_bank & 3)*0x2000)+0x4c000] |= (m_cg_reg[4] & 8) ? (data & m_cg_reg[3] & m_cg_reg[6]) : 0;
					}
				}
				else if((m_cg_reg[0x05] & 0xc0) == 0x40) //pset
				{
					if(m_cg_reg[5] & 1) //B
					{
						ram[offset+((cur_bank & 3)*0x2000)+0x40000] &= ~data;
						ram[offset+((cur_bank & 3)*0x2000)+0x40000] |= (m_cg_reg[4] & 1) ? (data & m_cg_reg[0]) : 0;
					}
					if(m_cg_reg[5] & 2) //R
					{
						ram[offset+((cur_bank & 3)*0x2000)+0x44000] &= ~data;
						ram[offset+((cur_bank & 3)*0x2000)+0x44000] |= (m_cg_reg[4] & 2) ? (data & m_cg_reg[1]) : 0;
					}
					if(m_cg_reg[5] & 4) //G
					{
						ram[offset+((cur_bank & 3)*0x2000)+0x48000] &= ~data;
						ram[offset+((cur_bank & 3)*0x2000)+0x48000] |= (m_cg_reg[4] & 4) ? (data & m_cg_reg[2]) : 0;
					}
					if(m_cg_reg[5] & 8) //I
					{
						ram[offset+((cur_bank & 3)*0x2000)+0x4c000] &= ~data;
						ram[offset+((cur_bank & 3)*0x2000)+0x4c000] |= (m_cg_reg[4] & 8) ? (data & m_cg_reg[3]) : 0;
					}
				}
			}
			break;
		}
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		{
			// IPL ROM, WRITENOP
			//printf("%04x %02x\n",offset+bank_num*0x2000,data);
			break;
		}
		case 0x38:
		{
			// TVRAM
			ram[offset+cur_bank*0x2000] = data;
			break;
		}
		case 0x39:
		{
			ram[offset+cur_bank*0x2000] = data;
			if(m_kanji_bank & 0x80) //kanji ROM
			{
				//NOP
			}
			else //PCG RAM
			{
				m_pcg_ram[offset] = data;
				if((offset & 0x1800) == 0x0000)
					m_gfxdecode->gfx(3)->mark_dirty((offset) >> 3);
				else
					m_gfxdecode->gfx(4)->mark_dirty((offset & 0x7ff) >> 3);
			}
			break;
		}
		case 0x3a:
		{
			// DIC ROM, WRITENOP
			break;
		}
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
		{
			// PHONE ROM, WRITENOP
			break;
		}
		default: ram[offset+cur_bank*0x2000] = data; break;
	}
}

READ8_MEMBER(mz2500_state::bank0_r){ return mz2500_ram_read(offset, 0); }
READ8_MEMBER(mz2500_state::bank1_r){ return mz2500_ram_read(offset, 1); }
READ8_MEMBER(mz2500_state::bank2_r){ return mz2500_ram_read(offset, 2); }
READ8_MEMBER(mz2500_state::bank3_r){ return mz2500_ram_read(offset, 3); }
READ8_MEMBER(mz2500_state::bank4_r){ return mz2500_ram_read(offset, 4); }
READ8_MEMBER(mz2500_state::bank5_r){ return mz2500_ram_read(offset, 5); }
READ8_MEMBER(mz2500_state::bank6_r){ return mz2500_ram_read(offset, 6); }
READ8_MEMBER(mz2500_state::bank7_r){ return mz2500_ram_read(offset, 7); }
WRITE8_MEMBER(mz2500_state::bank0_w){ mz2500_ram_write(offset, data, 0); }
WRITE8_MEMBER(mz2500_state::bank1_w){ mz2500_ram_write(offset, data, 1); }
WRITE8_MEMBER(mz2500_state::bank2_w){ mz2500_ram_write(offset, data, 2); }
WRITE8_MEMBER(mz2500_state::bank3_w){ mz2500_ram_write(offset, data, 3); }
WRITE8_MEMBER(mz2500_state::bank4_w){ mz2500_ram_write(offset, data, 4); }
WRITE8_MEMBER(mz2500_state::bank5_w){ mz2500_ram_write(offset, data, 5); }
WRITE8_MEMBER(mz2500_state::bank6_w){ mz2500_ram_write(offset, data, 6); }
WRITE8_MEMBER(mz2500_state::bank7_w){ mz2500_ram_write(offset, data, 7); }


READ8_MEMBER(mz2500_state::mz2500_bank_addr_r)
{
	return m_bank_addr;
}

WRITE8_MEMBER(mz2500_state::mz2500_bank_addr_w)
{
//  printf("%02x\n",data);
	m_bank_addr = data & 7;
}

READ8_MEMBER(mz2500_state::mz2500_bank_data_r)
{
	UINT8 res;

	res = m_bank_val[m_bank_addr];

	m_bank_addr++;
	m_bank_addr&=7;

	return res;
}

WRITE8_MEMBER(mz2500_state::mz2500_bank_data_w)
{
//  static const char *const bank_name[] = { "bank0", "bank1", "bank2", "bank3", "bank4", "bank5", "bank6", "bank7" };

	m_bank_val[m_bank_addr] = data & 0x3f;

//  if((data*2) >= 0x70)
//  printf("%s %02x\n",bank_name[m_bank_addr],m_bank_val[m_bank_addr]*2);

//  membank(bank_name[m_bank_addr])->set_base(&m_main_ram[m_bank_val[m_bank_addr]*0x2000]);

	m_bank_addr++;
	m_bank_addr&=7;
}

WRITE8_MEMBER(mz2500_state::mz2500_kanji_bank_w)
{
	m_kanji_bank = data;
}

WRITE8_MEMBER(mz2500_state::mz2500_dictionary_bank_w)
{
	m_dic_bank = data;
}

/* 0xf4 - 0xf7 all returns vblank / hblank states */
READ8_MEMBER(mz2500_state::mz2500_crtc_hvblank_r)
{
	UINT8 vblank_bit, hblank_bit;

	vblank_bit = machine().first_screen()->vblank() ? 0 : 1;
	hblank_bit = machine().first_screen()->hblank() ? 0 : 2;

	return vblank_bit | hblank_bit;
}

/*
TVRAM / CRTC registers

[0x00] ---x ---- line height (0) 16 (1) 20
[0x00] ---- xx-- 40 column mode (0) 64 colors (1) screen 1 (2) screen 2 (3) screen 1 + screen 2
[0x00] ---- --x- (related to the transparent pen)
[0x01] xxxx xxxx TV map offset low address value
[0x02] ---- -xxx TV map offset high address value
[0x03] xxxx xxxx CRTC vertical start register

[0x05] xxxx xxxx CRTC vertical end register

[0x07] -xxx xxxx CRTC horizontal start register
[0x08] -xxx xxxx CRTC horizontal end register
[0x09] ---- xxxx vertical scrolling shift position
[0x0a] --GG RRBB 256 color mode
[0x0b] -r-- ---- Back plane red gradient
[0x0b] ---- -b-- Back plane blue gradient
[0x0b] ---- ---i Back plane i gradient
[0x0c] ---- ---g Back plane green gradient

[0x0f] ---- x--- sets monitor type interlace / progressive
*/

UINT8 mz2500_state::pal_256_param(int index, int param)
{
	UINT8 val = 0;

	switch(param & 3)
	{
		case 0: val = index & 0x80 ? 1 : 0; break;
		case 1: val = index & 0x08 ? 1 : 0; break;
		case 2: val = 1; break;
		case 3: val = 0; break;
	}

	return val;
}

WRITE8_MEMBER(mz2500_state::mz2500_tv_crtc_w)
{
	switch(offset)
	{
		case 0: m_text_reg_index = data; break;
		case 1:
			m_text_reg[m_text_reg_index] = data;

			#if 0
			//printf("[%02x] <- %02x\n",m_text_reg_index,data);
			popmessage("(%02x %02x) (%02x %02x %02x %02x) (%02x %02x %02x) (%02x %02x %02x %02x)"
			,m_text_reg[0] & ~0x1e,m_text_reg[3]
			,m_text_reg[4],m_text_reg[5],m_text_reg[6],m_text_reg[7]
			,m_text_reg[8],m_text_reg[10],m_text_reg[11]
			,m_text_reg[12],m_text_reg[13],m_text_reg[14],m_text_reg[15]);

			#endif
			//popmessage("%d %02x %d %02x %d %d",m_text_reg[3],m_text_reg[4],m_text_reg[5],m_text_reg[6],m_text_reg[7]*8,m_text_reg[8]*8);

			mz2500_reconfigure_screen();

			if(m_text_reg_index == 0x0a) // set 256 color palette
			{
				int i,r,g,b;
				UINT8 b_param,r_param,g_param;

				b_param = (data & 0x03) >> 0;
				r_param = (data & 0x0c) >> 2;
				g_param = (data & 0x30) >> 4;

				for(i = 0;i < 0x100;i++)
				{
					int bit0,bit1,bit2;

					bit0 = pal_256_param(i,b_param) ? 1 : 0;
					bit1 = i & 0x01 ? 2 : 0;
					bit2 = i & 0x10 ? 4 : 0;
					b = bit0|bit1|bit2;
					bit0 = pal_256_param(i,r_param) ? 1 : 0;
					bit1 = i & 0x02 ? 2 : 0;
					bit2 = i & 0x20 ? 4 : 0;
					r = bit0|bit1|bit2;
					bit0 = pal_256_param(i,g_param) ? 1 : 0;
					bit1 = i & 0x04 ? 2 : 0;
					bit2 = i & 0x40 ? 4 : 0;
					g = bit0|bit1|bit2;

					m_palette->set_pen_color(i+0x100,pal3bit(r),pal3bit(g),pal3bit(b));
				}
			}
			if(m_text_reg_index >= 0x80 && m_text_reg_index <= 0x8f) //Bitmap 16 clut registers
			{
				/*
				---x ---- priority
				---- xxxx clut number
				*/
				m_clut16[m_text_reg_index & 0xf] = data & 0x1f;
				//printf("%02x -> [%02x]\n",m_text_reg[m_text_reg_index],m_text_reg_index);

				{
					int i;

					for(i=0;i<0x10;i++)
					{
						m_clut256[(m_text_reg_index & 0xf) | (i << 4)] = (((data & 0x1f) << 4) | i);
					}
				}
			}
			break;
		case 2: /* CG Mask reg (priority mixer) */
			m_cg_mask = data;
			break;
		case 3:
			/* Font size reg */
			m_text_font_reg = data & 1;
			mz2500_reconfigure_screen();
			break;
	}
}

WRITE8_MEMBER(mz2500_state::mz2500_irq_sel_w)
{
	m_irq_sel = data;
	//printf("%02x\n",m_irq_sel);
	// activeness is trusted, see Tower of Druaga
	m_irq_mask[0] = (data & 0x08); //CRTC
	m_irq_mask[1] = (data & 0x04); //i8253
	m_irq_mask[2] = (data & 0x02); //printer
	m_irq_mask[3] = (data & 0x01); //RP5c15
}

WRITE8_MEMBER(mz2500_state::mz2500_irq_data_w)
{
	if(m_irq_sel & 0x80)
		m_irq_vector[0] = data; //CRTC
	if(m_irq_sel & 0x40)
		m_irq_vector[1] = data; //i8253
	if(m_irq_sel & 0x20)
		m_irq_vector[2] = data; //printer
	if(m_irq_sel & 0x10)
		m_irq_vector[3] = data; //RP5c15

//  popmessage("%02x %02x %02x %02x",m_irq_vector[0],m_irq_vector[1],m_irq_vector[2],m_irq_vector[3]);
}

WRITE8_MEMBER(mz2500_state::floppy_select_w)
{
	switch ((data & 0x03) ^ m_fdc_reverse)
	{
	case 0: m_floppy = m_floppy0->get_device(); break;
	case 1: m_floppy = m_floppy1->get_device(); break;
	case 2: m_floppy = m_floppy2->get_device(); break;
	case 3: m_floppy = m_floppy3->get_device(); break;
	}

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
		m_floppy->mon_w(!BIT(data, 7));
}

WRITE8_MEMBER(mz2500_state::floppy_side_w)
{
	if (m_floppy)
		m_floppy->ss_w(BIT(data, 0));
}


static ADDRESS_MAP_START(mz2500_map, AS_PROGRAM, 8, mz2500_state )
	AM_RANGE(0x0000, 0x1fff) AM_READWRITE(bank0_r,bank0_w)
	AM_RANGE(0x2000, 0x3fff) AM_READWRITE(bank1_r,bank1_w)
	AM_RANGE(0x4000, 0x5fff) AM_READWRITE(bank2_r,bank2_w)
	AM_RANGE(0x6000, 0x7fff) AM_READWRITE(bank3_r,bank3_w)
	AM_RANGE(0x8000, 0x9fff) AM_READWRITE(bank4_r,bank4_w)
	AM_RANGE(0xa000, 0xbfff) AM_READWRITE(bank5_r,bank5_w)
	AM_RANGE(0xc000, 0xdfff) AM_READWRITE(bank6_r,bank6_w)
	AM_RANGE(0xe000, 0xffff) AM_READWRITE(bank7_r,bank7_w)
ADDRESS_MAP_END


READ8_MEMBER(mz2500_state::mz2500_rom_r)
{
	m_lrom_index = (m_maincpu->state_int(Z80_B));

	m_rom_index = (m_rom_index & 0xffff00) | (m_lrom_index & 0xff);

	return m_iplpro_rom[m_rom_index];
}

WRITE8_MEMBER(mz2500_state::mz2500_rom_w)
{
	m_hrom_index = (m_maincpu->state_int(Z80_B));

	m_rom_index = (data << 8) | (m_rom_index & 0x0000ff) | ((m_hrom_index & 0xff)<<16);
	//printf("%02x\n",data);
}

/* sets 16 color entries out of 4096 possible combinations */
WRITE8_MEMBER(mz2500_state::palette4096_io_w)
{
	UINT8 pal_index;
	UINT8 pal_entry;

	pal_index = m_maincpu->state_int(Z80_B);
	pal_entry = (pal_index & 0x1e) >> 1;

	if(pal_index & 1)
		m_pal[pal_entry].g = (data & 0x0f);
	else
	{
		m_pal[pal_entry].r = (data & 0xf0) >> 4;
		m_pal[pal_entry].b = data & 0x0f;
	}

	m_palette->set_pen_color(pal_entry+0x10, pal4bit(m_pal[pal_entry].r), pal4bit(m_pal[pal_entry].g), pal4bit(m_pal[pal_entry].b));
}

READ8_MEMBER(mz2500_state::fdc_r)
{
	return m_fdc->read(space, offset) ^ 0xff;
}

WRITE8_MEMBER(mz2500_state::fdc_w)
{
	m_fdc->write(space, offset, data ^ 0xff);
}

READ8_MEMBER(mz2500_state::mz2500_bplane_latch_r)
{
	if(m_cg_reg[7] & 0x10)
		return mz2500_cg_latch_compare();
	else
		return m_cg_latch[0];
}


READ8_MEMBER(mz2500_state::mz2500_rplane_latch_r)
{
	if(m_cg_reg[0x07] & 0x10)
	{
		UINT8 vblank_bit;

		vblank_bit = machine().first_screen()->vblank() ? 0 : 0x80 | m_cg_clear_flag;

		return vblank_bit;
	}
	else
		return m_cg_latch[1];
}

READ8_MEMBER(mz2500_state::mz2500_gplane_latch_r)
{
	return m_cg_latch[2];
}

READ8_MEMBER(mz2500_state::mz2500_iplane_latch_r)
{
	return m_cg_latch[3];
}

/*
"GDE" CRTC registers

0x00: CG B - Replace / Pset register data mask
0x01: CG R - Replace / Pset register data mask
0x02: CG G - Replace / Pset register data mask
0x03: CG I - Replace / Pset register data mask
0x04: CG Replace / Pset active pixel
0x05: CG Replace / Pset mode register
0x06: CG Replace data register
0x07: compare CG buffer register
0x08: CG window vertical start lo reg
0x09: CG window vertical start hi reg (upper 1 bit)
0x0a: CG window vertical end lo reg
0x0b: CG window vertical end hi reg (upper 1 bit)
0x0c: CG window horizontal start reg (7 bits, val x 8)
0x0d: CG window horizontal end reg (7 bits, val x 8)
0x0e: CG mode
0x0f: vertical scroll shift (---- xxxx)
0x10: CG map base lo reg
0x11: CG map base hi reg
0x12: CG layer 0 end address lo reg
0x13: CG layer 0 end address hi reg
0x14: CG layer 1 start address lo reg
0x15: CG layer 1 start address hi reg
0x16: CG layer 1 y pixel start lo reg
0x17: CG layer 1 y pixel start hi reg
0x18: CG color masking
*/

WRITE8_MEMBER(mz2500_state::mz2500_cg_addr_w)
{
	m_cg_reg_index = data;
}

WRITE8_MEMBER(mz2500_state::mz2500_cg_data_w)
{
	m_cg_reg[m_cg_reg_index & 0x1f] = data;

	if((m_cg_reg_index & 0x1f) == 0x08) //accessing VS LO reg clears VS HI reg
		m_cg_reg[0x09] = 0;

	if((m_cg_reg_index & 0x1f) == 0x0a) //accessing VE LO reg clears VE HI reg
		m_cg_reg[0x0b] = 0;

	if((m_cg_reg_index & 0x1f) == 0x05 && (m_cg_reg[0x05] & 0xc0) == 0x80) //clear bitmap buffer
	{
		UINT32 i;
		UINT8 *vram = m_main_ram; // TODO
		UINT32 layer_bank;

		layer_bank = (m_cg_reg[0x0e] & 0x80) ? 0x10000 : 0x00000;

		/* TODO: this isn't yet 100% accurate */
		if(m_cg_reg[0x05] & 1)
		{
			for(i=0;i<0x4000;i++)
				vram[i+0x40000+layer_bank] = 0x00; //clear B
		}
		if(m_cg_reg[0x05] & 2)
		{
			for(i=0;i<0x4000;i++)
				vram[i+0x44000+layer_bank] = 0x00; //clear R
		}
		if(m_cg_reg[0x05] & 4)
		{
			for(i=0;i<0x4000;i++)
				vram[i+0x48000+layer_bank] = 0x00; //clear G
		}
		if(m_cg_reg[0x05] & 8)
		{
			for(i=0;i<0x4000;i++)
				vram[i+0x4c000+layer_bank] = 0x00; //clear I
		}
		m_cg_clear_flag = 1;
	}

	{
		mz2500_reconfigure_screen();
	}

	if(m_cg_reg_index & 0x80) //enable auto-inc
		m_cg_reg_index = (m_cg_reg_index & 0xfc) | ((m_cg_reg_index + 1) & 0x03);
}

WRITE8_MEMBER(mz2500_state::timer_w)
{
	m_pit->write_gate0(1);
	m_pit->write_gate1(1);
	m_pit->write_gate0(0);
	m_pit->write_gate1(0);
	m_pit->write_gate0(1);
	m_pit->write_gate1(1);
}


READ8_MEMBER(mz2500_state::mz2500_joystick_r)
{
	UINT8 res,dir_en,in_r;

	res = 0xff;
	in_r = ~ioport(m_joy_mode & 0x40 ? "JOY_2P" : "JOY_1P")->read();

	if(m_joy_mode & 0x40)
	{
		if(!(m_joy_mode & 0x04)) res &= ~0x20;
		if(!(m_joy_mode & 0x08)) res &= ~0x10;
		dir_en = (m_joy_mode & 0x20) ? 0 : 1;
	}
	else
	{
		if(!(m_joy_mode & 0x01)) res &= ~0x20;
		if(!(m_joy_mode & 0x02)) res &= ~0x10;
		dir_en = (m_joy_mode & 0x10) ? 0 : 1;
	}

	if(dir_en)
		res &= (~((in_r) & 0x0f));

	res &= (~((in_r) & 0x30));

	return res;
}

WRITE8_MEMBER(mz2500_state::mz2500_joystick_w)
{
	m_joy_mode = data;
}


READ8_MEMBER(mz2500_state::mz2500_kanji_r)
{
	printf("Read from kanji 2 ROM\n");

	return m_kanji2_rom[(m_kanji_index << 1) | (offset & 1)];
}

WRITE8_MEMBER(mz2500_state::mz2500_kanji_w)
{
	(offset & 1) ? (m_kanji_index = (data << 8) | (m_kanji_index & 0xff)) : (m_kanji_index = (data & 0xff) | (m_kanji_index & 0xff00));
}

READ8_MEMBER(mz2500_state::rp5c15_8_r)
{
	UINT8 rtc_index = (m_maincpu->state_int(Z80_B));

	return m_rtc->read(space, rtc_index);
}

WRITE8_MEMBER(mz2500_state::rp5c15_8_w)
{
	UINT8 rtc_index = (m_maincpu->state_int(Z80_B));

	m_rtc->write(space, rtc_index, data);
}


READ8_MEMBER(mz2500_state::mz2500_emm_data_r)
{
	UINT8 emm_lo_index;

	emm_lo_index = (m_maincpu->state_int(Z80_B));

	m_emm_offset = (m_emm_offset & 0xffff00) | (emm_lo_index & 0xff);

	if(m_emm_offset < 0x100000) //emm max size
		return m_emm_ram[m_emm_offset];

	return 0xff;
}

WRITE8_MEMBER(mz2500_state::mz2500_emm_addr_w)
{
	UINT8 emm_hi_index;

	emm_hi_index = (m_maincpu->state_int(Z80_B));

	m_emm_offset = ((emm_hi_index & 0xff) << 16) | ((data & 0xff) << 8) | (m_emm_offset & 0xff);
}

WRITE8_MEMBER(mz2500_state::mz2500_emm_data_w)
{
	UINT8 emm_lo_index;

	emm_lo_index = (m_maincpu->state_int(Z80_B));

	m_emm_offset = (m_emm_offset & 0xffff00) | (emm_lo_index & 0xff);

	if(m_emm_offset < 0x100000) //emm max size
		m_emm_ram[m_emm_offset] = data;
}

static ADDRESS_MAP_START(mz2500_io, AS_IO, 8, mz2500_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE(0x60, 0x63) AM_WRITE(w3100a_w)
//  AM_RANGE(0x63, 0x63) AM_READ(w3100a_r)
//  AM_RANGE(0x98, 0x99) ADPCM, unknown type, custom?
	AM_RANGE(0xa0, 0xa3) AM_DEVREADWRITE("z80sio",z80sio0_device, ba_cd_r, ba_cd_w)
//  AM_RANGE(0xa4, 0xa5) AM_READWRITE(sasi_r, sasi_w)
	AM_RANGE(0xa8, 0xa8) AM_WRITE(mz2500_rom_w)
	AM_RANGE(0xa9, 0xa9) AM_READ(mz2500_rom_r)
	AM_RANGE(0xac, 0xac) AM_WRITE(mz2500_emm_addr_w)
	AM_RANGE(0xad, 0xad) AM_READ(mz2500_emm_data_r) AM_WRITE(mz2500_emm_data_w)
	AM_RANGE(0xae, 0xae) AM_WRITE(palette4096_io_w)
//  AM_RANGE(0xb0, 0xb3) AM_READWRITE(sio_r,sio_w)
	AM_RANGE(0xb4, 0xb4) AM_READWRITE(mz2500_bank_addr_r,mz2500_bank_addr_w)
	AM_RANGE(0xb5, 0xb5) AM_READWRITE(mz2500_bank_data_r,mz2500_bank_data_w)
	AM_RANGE(0xb7, 0xb7) AM_WRITENOP
	AM_RANGE(0xb8, 0xb9) AM_READWRITE(mz2500_kanji_r,mz2500_kanji_w)
	AM_RANGE(0xbc, 0xbc) AM_READ(mz2500_bplane_latch_r) AM_WRITE(mz2500_cg_addr_w)
	AM_RANGE(0xbd, 0xbd) AM_READ(mz2500_rplane_latch_r) AM_WRITE(mz2500_cg_data_w)
	AM_RANGE(0xbe, 0xbe) AM_READ(mz2500_gplane_latch_r)
	AM_RANGE(0xbf, 0xbf) AM_READ(mz2500_iplane_latch_r)
	AM_RANGE(0xc6, 0xc6) AM_WRITE(mz2500_irq_sel_w)
	AM_RANGE(0xc7, 0xc7) AM_WRITE(mz2500_irq_data_w)
	AM_RANGE(0xc8, 0xc9) AM_DEVREADWRITE("ym", ym2203_device, read, write)
//  AM_RANGE(0xca, 0xca) AM_READWRITE(voice_r,voice_w)
	AM_RANGE(0xcc, 0xcc) AM_READWRITE(rp5c15_8_r, rp5c15_8_w)
	AM_RANGE(0xce, 0xce) AM_WRITE(mz2500_dictionary_bank_w)
	AM_RANGE(0xcf, 0xcf) AM_WRITE(mz2500_kanji_bank_w)
	AM_RANGE(0xd8, 0xdb) AM_READWRITE(fdc_r, fdc_w)
	AM_RANGE(0xdc, 0xdc) AM_WRITE(floppy_select_w)
	AM_RANGE(0xdd, 0xdd) AM_WRITE(floppy_side_w)
	AM_RANGE(0xde, 0xde) AM_WRITENOP
	AM_RANGE(0xe0, 0xe3) AM_DEVREADWRITE("i8255_0", i8255_device, read, write)
	AM_RANGE(0xe4, 0xe7) AM_DEVREADWRITE("pit", pit8253_device, read, write)
	AM_RANGE(0xe8, 0xeb) AM_DEVREADWRITE("z80pio_1", z80pio_device, read_alt, write_alt)
	AM_RANGE(0xef, 0xef) AM_READWRITE(mz2500_joystick_r,mz2500_joystick_w)
	AM_RANGE(0xf0, 0xf3) AM_WRITE(timer_w)
	AM_RANGE(0xf4, 0xf7) AM_READ(mz2500_crtc_hvblank_r) AM_WRITE(mz2500_tv_crtc_w)
//  AM_RANGE(0xf8, 0xf9) AM_READWRITE(extrom_r,extrom_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( mz2500 )
	PORT_START("KEY0")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8)

	PORT_START("KEY1")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F9") PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F10") PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("8 (PAD)") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("9 (PAD)") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(", (PAD)") //PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(". (PAD)") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("+ (PAD)") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("- (PAD)") PORT_CODE(KEYCODE_MINUS_PAD)

	PORT_START("KEY2")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("0 (PAD)") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("1 (PAD)") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("2 (PAD)") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("3 (PAD)") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("4 (PAD)") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("5 (PAD)") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("6 (PAD)") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("7 (PAD)") PORT_CODE(KEYCODE_7_PAD)

	PORT_START("KEY3")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(27)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("BREAK") //PORT_CODE(KEYCODE_ESC)

	PORT_START("KEY4")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("/") //PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("KEY5")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("KEY6")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("KEY7")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("^")
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) //Yen symbol
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("_")
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(".")
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(",")

	PORT_START("KEY8")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("KEY9")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(":")
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(";")
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("@")
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("[")
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("KEYA")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("]")
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("COPY")
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("CLR")
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("INST")
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("BACKSPACE") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("ESC")
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("* (PAD)") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("/ (PAD)") PORT_CODE(KEYCODE_SLASH_PAD)

	PORT_START("KEYB")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("GRPH")
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("SLOCK")
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("KANA")
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("CTRL")
	PORT_BIT(0xe0,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("KEYC")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("KJ1")
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("KJ2")
	PORT_BIT(0xfc,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("KEYD")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("LOGO KEY")
	PORT_BIT(0xfe,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("UNUSED")
	PORT_BIT(0xff,IP_ACTIVE_LOW,IPT_UNUSED )

	/* this enables HD-loader */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "IPLPRO" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x30, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Monitor Interlace" ) //not all games support this
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("JOY_1P")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOY_2P")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void mz2500_state::mz2500_reset(mz2500_state *state, UINT8 type)
{
	int i;

	for(i=0;i<8;i++)
		m_bank_val[i] = bank_reset_val[type][i];
}

static const gfx_layout mz2500_pcg_layout_1bpp =
{
	8, 8,
	0x100,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8 * 8
};

static const gfx_layout mz2500_pcg_layout_3bpp =
{
	8, 8,
	0x100,
	3,
	{ 0x1800*8, 0x1000*8, 0x800*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8 * 8
};

void mz2500_state::machine_start()
{
	/* TODO: main RAM actually needs to be splitted */
	m_main_ram = auto_alloc_array_clear(machine(), UINT8, 0x80000);
	m_pcg_ram = auto_alloc_array_clear(machine(), UINT8, 0x2000);
	m_ipl_rom = memregion("ipl")->base();
	m_kanji_rom = memregion("kanji")->base();
	m_kanji2_rom = memregion("kanji2")->base();
	m_emm_ram = auto_alloc_array_clear(machine(), UINT8, 0x100000);
	m_dic_rom = memregion("dictionary")->base();
	m_phone_rom = memregion("phone")->base();
	m_iplpro_rom = memregion("iplpro")->base();

	save_pointer(NAME(m_main_ram), 0x80000);
	save_pointer(NAME(m_pcg_ram), 0x2000);
	save_pointer(NAME(m_emm_ram), 0x100000);

	/* TODO: gfx[4] crashes as per now */
	m_gfxdecode->set_gfx(3, global_alloc(gfx_element(m_palette, mz2500_pcg_layout_1bpp, (UINT8 *)m_pcg_ram, 0, 0x10, 0)));
	m_gfxdecode->set_gfx(4, global_alloc(gfx_element(m_palette, mz2500_pcg_layout_3bpp, (UINT8 *)m_pcg_ram, 0, 4, 0)));
}

void mz2500_state::machine_reset()
{
	UINT32 i;

	mz2500_reset(this, IPL_RESET);

	//m_irq_vector[0] = 0xef; /* RST 28h - vblank */

	m_text_col_size = 0;
	m_text_font_reg = 0;

	/* copy IPL to its natural bank ROM/RAM position */
	for(i=0;i<0x8000;i++)
	{
		//m_main_ram[i] = IPL[i];
		m_main_ram[i+0x68000] = m_ipl_rom[i];
	}

	/* clear CG RAM */
	for(i=0;i<0x20000;i++)
		m_main_ram[i+0x40000] = 0x00;

	/* disable IRQ */
	for(i=0;i<4;i++)
	{
		m_irq_mask[i] = 0;
		m_irq_pending[i] = 0;
	}
	m_kanji_bank = 0;

	m_cg_clear_flag = 0;

	m_beeper->set_frequency(4096);
	m_beeper->set_state(0);

//  m_monitor_type = ioport("DSW1")->read() & 0x40 ? 1 : 0;
}

static const gfx_layout mz2500_cg_layout =
{
	8, 8,       /* 8 x 8 graphics */
	RGN_FRAC(1,1),      /* 512 codes */
	1,      /* 1 bit per pixel */
	{ 0 },      /* no bitplanes */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8 * 8       /* code takes 8 times 8 bits */
};

/* gfx1 is mostly 16x16, but there are some 8x8 characters */
static const gfx_layout mz2500_8_layout =
{
	8, 8,       /* 8 x 8 graphics */
	1920,       /* 1920 codes */
	1,      /* 1 bit per pixel */
	{ 0 },      /* no bitplanes */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8 * 8       /* code takes 8 times 8 bits */
};

static const gfx_layout mz2500_16_layout =
{
	16, 16,     /* 16 x 16 graphics */
	RGN_FRAC(1,1),      /* 8192 codes */
	1,      /* 1 bit per pixel */
	{ 0 },      /* no bitplanes */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 128, 129, 130, 131, 132, 133, 134, 135 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16 * 16     /* code takes 16 times 16 bits */
};

/* these are just for viewer sake, actually they aren't used in drawing routines */
static GFXDECODE_START( mz2500 )
	GFXDECODE_ENTRY("kanji", 0, mz2500_cg_layout, 0, 256)
	GFXDECODE_ENTRY("kanji", 0x4400, mz2500_8_layout, 0, 256)
	GFXDECODE_ENTRY("kanji", 0, mz2500_16_layout, 0, 256)
//  GFXDECODE_ENTRY("pcg", 0, mz2500_pcg_layout_1bpp, 0, 0x10)
//  GFXDECODE_ENTRY("pcg", 0, mz2500_pcg_layout_3bpp, 0, 4)
GFXDECODE_END

INTERRUPT_GEN_MEMBER(mz2500_state::mz2500_vbl)
{
	if(m_irq_mask[0])
	{
		m_irq_pending[0] = 1;
		m_maincpu->set_input_line(0, ASSERT_LINE);
	}
	m_cg_clear_flag = 0;
}

IRQ_CALLBACK_MEMBER(mz2500_state::mz2500_irq_ack)
{
	int i;
	for(i=0;i<4;i++)
	{
		if(m_irq_mask[i] && m_irq_pending[i])
		{
			m_irq_pending[i] = 0;
			m_maincpu->set_input_line(0, CLEAR_LINE);
			return m_irq_vector[i];
		}
	}
	return 0;
}

READ8_MEMBER(mz2500_state::mz2500_porta_r)
{
	logerror("PPI PORTA R\n");

	return 0xff;
}

READ8_MEMBER(mz2500_state::mz2500_portb_r)
{
	UINT8 vblank_bit;

	vblank_bit = machine().first_screen()->vblank() ? 0 : 1; //Guess: NOBO wants this bit to be high/low

	return 0xfe | vblank_bit;
}

READ8_MEMBER(mz2500_state::mz2500_portc_r)
{
	logerror("PPI PORTC R\n");

	return 0xff;
}

WRITE8_MEMBER(mz2500_state::mz2500_porta_w)
{
	logerror("PPI PORTA W %02x\n",data);
}

WRITE8_MEMBER(mz2500_state::mz2500_portb_w)
{
	logerror("PPI PORTB W %02x\n",data);
}

WRITE8_MEMBER(mz2500_state::mz2500_portc_w)
{
	/*
	---- x--- 0->1 transition = IPL reset
	---- -x-- beeper state
	---- --x- 0->1 transition = Work RAM reset
	---- ---x screen mask
	*/

	/* work RAM reset */
	if((m_old_portc & 0x02) == 0x00 && (data & 0x02))
	{
		mz2500_reset(this, WRAM_RESET);
		/* correct? */
		m_maincpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
	}

	/* bit 2 is speaker */

	/* IPL reset */
	if((m_old_portc & 0x08) == 0x00 && (data & 0x08))
		mz2500_reset(this, IPL_RESET);

	m_old_portc = data;

	m_beeper->set_state(data & 0x04);

	m_screen_enable = data & 1;

	if(data & ~0x0f)
		logerror("PPI PORTC W %02x\n",data & ~0x0f);
}

WRITE8_MEMBER(mz2500_state::mz2500_pio1_porta_w)
{
//  printf("%02x\n",data);

	if(m_prev_col_val != ((data & 0x20) >> 5))
	{
		m_text_col_size = ((data & 0x20) >> 5);
		m_prev_col_val = m_text_col_size;
		mz2500_reconfigure_screen();
	}
	m_key_mux = data & 0x1f;
}


READ8_MEMBER(mz2500_state::mz2500_pio1_porta_r)
{
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3",
											"KEY4", "KEY5", "KEY6", "KEY7",
											"KEY8", "KEY9", "KEYA", "KEYB",
											"KEYC", "KEYD", "UNUSED", "UNUSED" };

	if(((m_key_mux & 0x10) == 0x00) || ((m_key_mux & 0x0f) == 0x0f)) //status read
	{
		int res,i;

		res = 0xff;
		for(i=0;i<0xe;i++)
			res &= ioport(keynames[i])->read();

		m_pio_latchb = res;

		return res;
	}

	m_pio_latchb = ioport(keynames[m_key_mux & 0xf])->read();

	return ioport(keynames[m_key_mux & 0xf])->read();
}

#if 0
READ8_MEMBER(mz2500_state::mz2500_pio1_portb_r)
{
	return m_pio_latchb;
}
#endif

READ8_MEMBER(mz2500_state::opn_porta_r)
{
	return m_ym_porta;
}

WRITE8_MEMBER(mz2500_state::opn_porta_w)
{
	/*
	---- x--- mouse select
	---- -x-- palette bit (16/4096 colors)
	---- --x- floppy reverse bit (controls wd17xx bits in command registers)
	*/

	m_fdc_reverse = data & 2;
	m_pal_select = (data & 4) ? 1 : 0;

	m_ym_porta = data;
}

PALETTE_INIT_MEMBER(mz2500_state, mz2500)
{
	int i;

	for(i=0;i<0x200;i++)
		palette.set_pen_color(i,pal1bit(0),pal1bit(0),pal1bit(0));

	/* set up 8 colors (PCG) */
	for(i=0;i<8;i++)
		m_palette->set_pen_color(i+8,pal1bit((i & 2)>>1),pal1bit((i & 4)>>2),pal1bit((i & 1)>>0));

	/* set up 16 colors (PCG / CG) */

	/* set up 256 colors (CG) */
	{
		int r,g,b;

		for(i = 0;i < 0x100;i++)
		{
			int bit0,bit1,bit2;

			bit0 = pal_256_param(i,0) ? 1 : 0;
			bit1 = i & 0x01 ? 2 : 0;
			bit2 = i & 0x10 ? 4 : 0;
			b = bit0|bit1|bit2;
			bit0 = pal_256_param(i,0) ? 1 : 0;
			bit1 = i & 0x02 ? 2 : 0;
			bit2 = i & 0x20 ? 4 : 0;
			r = bit0|bit1|bit2;
			bit0 = pal_256_param(i,0) ? 1 : 0;
			bit1 = i & 0x04 ? 2 : 0;
			bit2 = i & 0x40 ? 4 : 0;
			g = bit0|bit1|bit2;

			m_palette->set_pen_color(i+0x100,pal3bit(r),pal3bit(g),pal3bit(b));
		}
	}
}

/* PIT8253 Interface */

WRITE_LINE_MEMBER(mz2500_state::pit8253_clk0_irq)
{
	if(m_irq_mask[1] && state & 1)
	{
		m_irq_pending[1] = 1;
		m_maincpu->set_input_line(0, ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER(mz2500_state::mz2500_rtc_alarm_irq)
{
	/* TODO: doesn't work yet */
//  if(m_irq_mask[3] && state & 1)
//      m_maincpu->set_input_line_and_vector(0, HOLD_LINE,drvm_irq_vector[3]);
}


static SLOT_INTERFACE_START( mz2500_floppies )
	SLOT_INTERFACE("dd", FLOPPY_35_DD)
SLOT_INTERFACE_END


static MACHINE_CONFIG_START( mz2500, mz2500_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 6000000)
	MCFG_CPU_PROGRAM_MAP(mz2500_map)
	MCFG_CPU_IO_MAP(mz2500_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mz2500_state,  mz2500_vbl)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(mz2500_state,mz2500_irq_ack)

	MCFG_DEVICE_ADD("i8255_0", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(mz2500_state, mz2500_porta_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(mz2500_state, mz2500_porta_w))
	MCFG_I8255_IN_PORTB_CB(READ8(mz2500_state, mz2500_portb_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(mz2500_state, mz2500_portb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(mz2500_state, mz2500_portc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(mz2500_state, mz2500_portc_w))

	MCFG_DEVICE_ADD("z80pio_1", Z80PIO, 6000000)
	MCFG_Z80PIO_IN_PA_CB(READ8(mz2500_state, mz2500_pio1_porta_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(mz2500_state, mz2500_pio1_porta_w))
	MCFG_Z80PIO_IN_PB_CB(READ8(mz2500_state, mz2500_pio1_porta_r))

	MCFG_Z80SIO0_ADD("z80sio", 6000000, 0, 0, 0, 0)

	MCFG_DEVICE_ADD(RP5C15_TAG, RP5C15, XTAL_32_768kHz)
	MCFG_RP5C15_OUT_ALARM_CB(WRITELINE(mz2500_state, mz2500_rtc_alarm_irq))

	MCFG_DEVICE_ADD("pit", PIT8253, 0)
	MCFG_PIT8253_CLK0(31250)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(mz2500_state, pit8253_clk0_irq))
	// TODO: is this really right?
	MCFG_PIT8253_CLK1(0)
	MCFG_PIT8253_CLK2(16) //CH2, used by Super MZ demo / The Black Onyx and a few others (TODO: timing of this)
	MCFG_PIT8253_OUT2_HANDLER(DEVWRITELINE("pit", pit8253_device, write_clk1))

	MCFG_MB8877_ADD("mb8877a", XTAL_1MHz)

	MCFG_FLOPPY_DRIVE_ADD("mb8877a:0", mz2500_floppies, "dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("mb8877a:1", mz2500_floppies, "dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("mb8877a:2", mz2500_floppies, "dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("mb8877a:3", mz2500_floppies, "dd", floppy_image_device::default_floppy_formats)

	MCFG_SOFTWARE_LIST_ADD("flop_list", "mz2500")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_21_4772MHz, 640+108, 0, 640, 480, 0, 200) //unknown clock / divider
	MCFG_SCREEN_UPDATE_DRIVER(mz2500_state, screen_update_mz2500)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_INIT_OWNER(mz2500_state, mz2500)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mz2500)


	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym", YM2203, 2000000) //unknown clock / divider
	MCFG_AY8910_PORT_A_READ_CB(READ8(mz2500_state, opn_porta_r))  // read A
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW1"))   // read B
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(mz2500_state, opn_porta_w))  // write A
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 0.25)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)

	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.50)
MACHINE_CONFIG_END



/* ROM definition */
ROM_START( mz2500 )
	ROM_REGION( 0x08000, "ipl", 0 )
	ROM_LOAD( "ipl.rom", 0x00000, 0x8000, CRC(7a659f20) SHA1(ccb3cfdf461feea9db8d8d3a8815f7e345d274f7) )

	/* this is probably an hand made ROM, will be removed in the end ...*/
	ROM_REGION( 0x1000, "cgrom", 0 )
	ROM_LOAD( "cg.rom", 0x0000, 0x0800, CRC(a082326f) SHA1(dfa1a797b2159838d078650801c7291fa746ad81) )

	ROM_REGION( 0x40000, "kanji", 0 )
	ROM_LOAD( "kanji.rom", 0x0000, 0x40000, CRC(dd426767) SHA1(cc8fae0cd1736bc11c110e1c84d3f620c5e35b80) )

	ROM_REGION( 0x20000, "kanji2", 0 )
	ROM_LOAD( "kanji2.rom", 0x0000, 0x20000, CRC(eaaf20c9) SHA1(771c4d559b5241390215edee798f3bce169d418c) )

	ROM_REGION( 0x40000, "dictionary", 0 )
	ROM_LOAD( "dict.rom", 0x00000, 0x40000, CRC(aa957c2b) SHA1(19a5ba85055f048a84ed4e8d471aaff70fcf0374) )

	ROM_REGION( 0x8000, "iplpro", ROMREGION_ERASEFF )
	ROM_LOAD( "sasi.rom", 0x00000, 0x8000, CRC(a7bf39ce) SHA1(3f4a237fc4f34bac6fe2bbda4ce4d16d42400081) )

	ROM_REGION( 0x8000, "phone", ROMREGION_ERASEFF )
	ROM_LOAD( "phone.rom", 0x00000, 0x4000, CRC(8e49e4dc) SHA1(2589f0c95028037a41ca32a8fd799c5f085dab51) )
ROM_END

ROM_START( mz2520 )
	ROM_REGION( 0x08000, "ipl", 0 )
	ROM_LOAD( "ipl2520.rom", 0x00000, 0x8000, CRC(0a126eb2) SHA1(faf71236b3ad82d30184adea951d43d10ced663d) )

	/* this is probably an hand made ROM, will be removed in the end ...*/
	ROM_REGION( 0x1000, "cgrom", 0 )
	ROM_LOAD( "cg.rom", 0x0000, 0x0800, CRC(a082326f) SHA1(dfa1a797b2159838d078650801c7291fa746ad81) )

	ROM_REGION( 0x40000, "kanji", 0 )
	ROM_LOAD( "kanji.rom", 0x0000, 0x40000, CRC(dd426767) SHA1(cc8fae0cd1736bc11c110e1c84d3f620c5e35b80) )

	ROM_REGION( 0x20000, "kanji2", 0 )
	ROM_LOAD( "kanji2.rom", 0x0000, 0x20000, CRC(eaaf20c9) SHA1(771c4d559b5241390215edee798f3bce169d418c) )

	ROM_REGION( 0x40000, "dictionary", 0 )
	ROM_LOAD( "dict.rom", 0x00000, 0x40000, CRC(aa957c2b) SHA1(19a5ba85055f048a84ed4e8d471aaff70fcf0374) )

	ROM_REGION( 0x8000, "iplpro", ROMREGION_ERASEFF )
	ROM_LOAD( "sasi.rom", 0x00000, 0x8000, CRC(a7bf39ce) SHA1(3f4a237fc4f34bac6fe2bbda4ce4d16d42400081) )

	ROM_REGION( 0x8000, "phone", ROMREGION_ERASEFF )
	ROM_LOAD( "phone.rom", 0x00000, 0x4000, CRC(8e49e4dc) SHA1(2589f0c95028037a41ca32a8fd799c5f085dab51) )
ROM_END

/* Driver */

COMP( 1985, mz2500,   0,             0,      mz2500,   mz2500, driver_device,        0,      "Sharp",     "MZ-2500", MACHINE_IMPERFECT_GRAPHICS )
COMP( 1985, mz2520,   mz2500,        0,      mz2500,   mz2500, driver_device,        0,      "Sharp",     "MZ-2520", MACHINE_IMPERFECT_GRAPHICS ) // looks a stripped down version of the regular MZ-2500, with only two floppies drives and no cassette interface
