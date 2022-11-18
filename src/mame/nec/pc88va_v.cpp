// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "pc88va.h"



void pc88va_state::video_start()
{
	m_kanjiram = std::make_unique<uint8_t[]>(0x4000);
	m_gfxdecode->gfx(2)->set_source(m_kanjiram.get());
	m_gfxdecode->gfx(3)->set_source(m_kanjiram.get());

	save_item(NAME(m_screen_ctrl_reg));
	save_item(NAME(m_text_transpen));
	save_pointer(NAME(m_video_pri_reg), 2);
}

uint32_t pc88va_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t pri, cur_pri_lv;
	uint32_t screen_pri;
	bitmap.fill(0, cliprect);

	if(m_tsp.disp_on == 0) // don't bother if we are under DSPOFF command
		return 0;

	/*
	m_video_pri_reg[0]
	xxxx ---- ---- ---- priority 3
	---- xxxx ---- ---- priority 2
	---- ---- xxxx ---- priority 1
	---- ---- ---- xxxx priority 0
	m_video_pri_reg[1]
	---- ---- xxxx ---- priority 5
	---- ---- ---- xxxx priority 4

	Note that orthogonality level is actually REVERSED than the level number it indicates, so we have to play a little with the data for an easier usage ...
	*/

	// TODO: this should be calculated only in video_pri_w for cache
	screen_pri = (m_video_pri_reg[1] & 0x00f0) >> 4; // priority 5
	screen_pri|= (m_video_pri_reg[1] & 0x000f) << 4; // priority 4
	screen_pri|= (m_video_pri_reg[0] & 0xf000) >> 4; // priority 3
	screen_pri|= (m_video_pri_reg[0] & 0x0f00) << 4; // priority 2
	screen_pri|= (m_video_pri_reg[0] & 0x00f0) << 12; // priority 1
	screen_pri|= (m_video_pri_reg[0] & 0x000f) << 20; // priority 0

	//popmessage("%08x", screen_pri);

	for(pri = 0; pri < 6; pri++)
	{
		cur_pri_lv = (screen_pri >> (pri * 4)) & 0xf;

		if(cur_pri_lv & 8) // enable layer
		{
			if(pri <= 1) // (direct color mode, priority 5 and 4)
			{
				switch(cur_pri_lv & 3)
				{
					// 8 = graphic 0
					case 0:
						draw_raw_gfx(bitmap, cliprect);
						break;
					// 9 = graphic 1
					case 1:
						popmessage("Unimplemented raw GFX 1");
						break;
					default:
						popmessage("Undocumented pri level %d", cur_pri_lv);
				}
			}
			else
			{
				switch(cur_pri_lv & 3) // (palette color mode)
				{
					case 0: draw_text(bitmap,cliprect); break;
					case 1: if(m_tsp.spr_on) { draw_sprites(bitmap,cliprect); } break;
					// TODO: understand where pal bases come from
					/* A = graphic 0 */ 
					case 2: draw_indexed_gfx(bitmap, cliprect, 0x00000, 0x10); break;
					/* B = graphic 1 */ 
					case 3: draw_indexed_gfx(bitmap, cliprect, 0x20000, 0x00); break;
				}
			}
		}
	}

	return 0;
}

/****************************************
 * Drawing fns
 ***************************************/

void pc88va_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t const *const tvram = m_tvram;

	int offs = m_tsp.spr_offset;
	for(int i=0;i<(0x100);i+=(8))
	{
		int spr_count;

		int ysize = (tvram[(offs + i + 0) / 2] & 0xfc00) >> 10;
		int sw = (tvram[(offs + i + 0) / 2] & 0x200) >> 9;
		int yp = (tvram[(offs + i + 0) / 2] & 0x1ff);
		int xsize = (tvram[(offs + i + 2) / 2] & 0xf800) >> 11;
		int md = (tvram[(offs + i + 2) / 2] & 0x400) >> 10;
		int xp = (tvram[(offs + i + 2) / 2] & 0x3ff);
		int spda = (tvram[(offs + i + 4) / 2] & 0xffff);
		int fg_col = (tvram[(offs + i + 6) / 2] & 0xf0) >> 4;
		int bc = (tvram[(offs + i + 6) / 2] & 0x08) >> 3;

		if(!sw)
			continue;

		if(yp & 0x100)
		{
			yp &= 0xff;
			yp = 0x100 - yp;
		}

		if(0) // uhm, makes more sense without the sign?
		if(xp & 0x200)
		{
			xp &= 0x1ff;
			xp = 0x200 - xp;
		}

		if(md) // 1bpp mode
		{
			xsize = (xsize + 1) * 32;
			ysize = (ysize + 1) * 4;

			if(!(spda & 0x8000)) // correct?
				spda *= 2;

			spr_count = 0;

			for(int y_i=0;y_i<ysize;y_i++)
			{
				for(int x_i=0;x_i<xsize;x_i+=16)
				{
					for(int x_s=0;x_s<16;x_s++)
					{
						int pen = (bitswap<16>(tvram[(spda+spr_count) / 2],7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8) >> (15-x_s)) & 1;

						pen = pen & 1 ? fg_col : (bc) ? 8 : -1;

						if(pen != -1) //transparent pen
							bitmap.pix(yp+y_i, xp+x_i+(x_s)) = m_palette->pen(pen);
					}
					spr_count+=2;
				}
			}
		}
		else
		{
			// 4bpp mode
			// TODO: xsize may be doubled, cfr. pc88vad
			xsize = (xsize + 1) * 8;
			ysize = (ysize + 1) * 4;

			// TODO: verify me up
			if(!(spda & 0x8000))
				spda *= 2;

			spr_count = 0;

			for(int y_i = 0; y_i < ysize; y_i++)
			{
				for(int x_i = 0; x_i < xsize; x_i += 4)
				{
					for(int x_s = 0; x_s < 4; x_s ++)
					{
						int pen = (bitswap<16>(tvram[(spda+spr_count) / 2],7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8)) >> (12 - (x_s * 4)) & 0xf;

						// TODO: bc
						//if(bc != -1) //transparent pen
						//if (pen != 0 && pen != m_text_transpen)
						if (pen != 0)
							bitmap.pix(yp+y_i, xp+x_i+(x_s)) = m_palette->pen(pen);
					}
					spr_count+=2;
				}
			}
		}
	}
}

/* TODO: this is either a result of an hand-crafted ROM or the JIS stuff is really attribute related ... */
uint32_t pc88va_state::calc_kanji_rom_addr(uint8_t jis1,uint8_t jis2,int x,int y)
{
	if(jis1 < 0x30)
		return ((jis2 & 0x60) << 8) + ((jis1 & 0x07) << 10) + ((jis2 & 0x1f) << 5);
	else if(jis1 >= 0x30 && jis1 < 0x3f)
		return ((jis2 & 0x60) << 10) + ((jis1 & 0x0f) << 10) + ((jis2 & 0x1f) << 5);
	else if(jis1 >= 0x40 && jis1 < 0x50)
		return 0x4000 + ((jis2 & 0x60) << 10) + ((jis1 & 0x0f) << 10) + ((jis2 & 0x1f) << 5);
	else if(x == 0 && y == 0 && jis1 != 0 && jis2 != 0) // debug stuff, to be nuked in the end
		printf("%02x %02x\n",jis1, jis2);

	return 0;
}

/*
 * [+00] Frame buffer start address (VSA)
 * [+02] ---- ---- ---- -xxx Upper VSA
 * [+04] ---- -xxx xxxx xxxx Frame buffer height (VH)
 * [+06] <reserved>
 * [+08] ---- --xx xxxx xxxx Frame buffer width (VW), in bytes
 * [+0a] xxxx ---- ---- ---- background color
 *       ---- xxxx ---- ---- foreground color
 *       ---- ---- ---x xxxx display mode
 * [+0c] Raster address offset
 * [+0e] <reserved>
 * [+10] Split screen start address (RSA)
 * [+12] <reserved> / Upper RSA
 * [+14] Split screen height (RH)
 * [+16] Split screen width (RW)
 * [+18] Split screen vertical start position (RYP)
 * [+1a] Split screen horizontal start position (RXP)
 * [+1c] <reserved>
 * [+1e] <reserved>
 */
void pc88va_state::draw_text(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t const *const tvram = m_tvram;
	uint16_t const *const fb_regs = &tvram[m_tsp.tvram_vreg_offset / 2];
	// TODO: PCG select won't work with this arrangement
	uint8_t const *const kanji = memregion("kanji")->base();

	u32 vsa = (fb_regs[0 / 2]); // | (fb_regs[2 / 2] & 7) << 16;

	if (fb_regs[2 / 2])
		popmessage("Upper VSA enabled!");

	const u8 attr_mode = fb_regs[0xa / 2] & 0x1f;
	// TODO: check this out, diverges with documentation
	const u8 screen_fg_col = (fb_regs[0xa / 2] & 0xf000) >> 12;
	const u8 screen_bg_col = (fb_regs[0xa / 2] & 0x0f00) >> 8;

	// TODO: how even vh/vw can run have all these bytes?
	const u8 vh = (fb_regs[4 / 2] & 0x7ff);
	const u16 vw = (fb_regs[8 / 2] & 0x3ff) / 2;

	for(int y = 0; y < vh; y++)
	{
		for(int x = 0; x < vw; x++)
		{
			uint16_t attr = (tvram[vsa+(m_tsp.attr_offset/2)] & 0x00ff);

			uint8_t fg_col, bg_col, secret, reverse;
			//uint8_t blink,dwidc,dwid,uline,hline;
			fg_col = bg_col = reverse = secret = 0; //blink = dwidc = dwid = uline = hline = 0;

			// TODO: convert to functional programming
			switch(attr_mode)
			{
				/*
				xxxx ---- foreground color
				---- xxxx background color
				*/
				case 0:
					fg_col = (attr & 0xf0) >> 4;
					bg_col = (attr & 0x0f) >> 0;
					break;
				/*
				xxxx ---- foreground color
				---- x--- horizontal line
				---- -x-- reverse
				---- --x- blink
				---- ---x secret (hide text)
				background color is defined by screen control table values
				*/
				case 1:
					fg_col = (attr & 0xf0) >> 4;
					bg_col = screen_bg_col;
					//hline = (attr & 0x08) >> 3;
					reverse = (attr & 0x04) >> 2;
					//blink = (attr & 0x02) >> 1;
					secret = (attr & 0x01) >> 0;
					break;
				/*
				x--- ---- dwidc
				-x-- ---- dwid
				--x- ---- uline
				---x ---- hline
				---- -x-- reverse
				---- --x- blink
				---- ---x secret (hide text)
				background and foreground colors are defined by screen control table values
				*/
				case 2:
					fg_col = screen_fg_col;
					bg_col = screen_bg_col;
					//dwidc = (attr & 0x80) >> 7;
					//dwid = (attr & 0x40) >> 6;
					//uline = (attr & 0x20) >> 5;
					//hline = (attr & 0x10) >> 4;
					reverse = (attr & 0x04) >> 2;
					//blink = (attr & 0x02) >> 1;
					secret = (attr & 0x01) >> 0;
					break;
				/*
				---- x--- mixes between mode 0 and 2

				xxxx 1--- foreground color
				---- 1xxx background color
				2)
				x--- 0--- dwidc
				-x-- 0--- dwid
				--x- 0--- uline
				---x 0--- hline
				---- 0x-- reverse
				---- 0-x- blink
				---- 0--x secret (hide text)
				background and foreground colors are defined by screen control table values
				*/
				case 3:
					{
						if(attr & 0x8)
						{
							fg_col = (attr & 0xf0) >> 4;
							bg_col = (attr & 0x07) >> 0;
						}
						else
						{
							fg_col = screen_fg_col;
							bg_col = screen_bg_col;
							//dwidc = (attr & 0x80) >> 7;
							//dwid = (attr & 0x40) >> 6;
							//uline = (attr & 0x20) >> 5;
							//hline = (attr & 0x10) >> 4;
							reverse = (attr & 0x04) >> 2;
							//blink = (attr & 0x02) >> 1;
							secret = (attr & 0x01) >> 0;
						}
					}
					break;
				/*
				x--- ---- blink
				-xxx ---- background color
				---- xxxx foreground color
				*/
				case 4:
					fg_col = (attr & 0x0f) >> 0;
					bg_col = (attr & 0x70) >> 4;
					//blink = (attr & 0x80) >> 7;
					break;
				/*
				x--- ---- blink
				-xxx ---- background color
				---- xxxx foreground color
				hline is enabled if foreground color is 1 or 9
				*/
				case 5:
					fg_col = (attr & 0x0f) >> 0;
					bg_col = (attr & 0x70) >> 4;
					//blink = (attr & 0x80) >> 7;
					//if((fg_col & 7) == 1)
						//hline = 1;
					break;
				default:
					popmessage("Illegal text tilemap attribute mode %02x",attr_mode);
					return;
			}
			
			// TODO: more functional programming
			if ((tvram[vsa] & 0xff00) == 0) // ANK
			{
				
				u32 tile_num = ((tvram[vsa] & 0xff)  * 16) | 0x40000;

				for(int yi = 0; yi < 16; yi++)
				{
					for(int xi = 0; xi < 8; xi++)
					{
						int res_x = x * 8 + xi;
						int res_y = y * 16 + yi;

						if(!cliprect.contains(res_x, res_y))
							continue;

						int pen = kanji[yi + tile_num] >> (7-xi) & 1;

						if(reverse)
							pen = pen & 1 ? bg_col : fg_col;
						else
							pen = pen & 1 ? fg_col : bg_col;

						if(secret) { pen = 0; } //hide text

						if(pen != 0 && pen != m_text_transpen)
							bitmap.pix(res_y, res_x) = m_palette->pen(pen);
					}
				}
			}
			else // kanji
			{
				uint8_t jis1 = (tvram[vsa] & 0x7f) + 0x20;
				uint8_t jis2 = (tvram[vsa] & 0x7f00) >> 8;
				uint16_t lr_half_gfx = ((tvram[vsa] & 0x8000) >> 15);

				uint32_t tile_num = calc_kanji_rom_addr(jis1, jis2, x, y);

				for(int yi = 0; yi < 16; yi++)
				{
					for(int xi = 0; xi < 8; xi++)
					{
						int res_x = x * 8 + xi;
						int res_y = y * 16 + yi;

						if(!cliprect.contains(res_x, res_y))
							continue;

						int pen = kanji[((yi*2)+lr_half_gfx)+tile_num] >> (7-xi) & 1;

						if(reverse)
							pen = pen & 1 ? bg_col : fg_col;
						else
							pen = pen & 1 ? fg_col : bg_col;

						if(secret) { pen = 0; } //hide text

						// TODO: transpen not right, cfr. rogueall
						// (sets 1, wants 7)
						if(pen != 0 && pen != m_text_transpen)
							bitmap.pix(res_y, res_x) = m_palette->pen(pen);
					}
				}
			}


			vsa ++;
			vsa &=0xffff;
		}
	}
}

// TODO: $102 determine mode set (including using this or indexed modes)
void pc88va_state::draw_raw_gfx(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u16 *gvram = m_gvram;

	int count = 0;

	// TODO: cliprect
	for(int y = 0; y < 400; y+= 2)
	{
		for(int x = 0; x < 640; x++)
		{
			uint16_t color = gvram[count];

			for (int yi = 0; yi < 2; yi ++)
			{
				int res_y = y + yi;
				if(cliprect.contains(x, res_y))
				{
					u8 b = pal5bit((color & 0x001f));
					u8 r = pal5bit((color & 0x03e0) >> 5);
					u8 g = pal6bit((color & 0xfc00) >> 10);
					bitmap.pix(res_y, x) = (b) | (g << 8) | (r << 16);
				}
			}

			count ++;
		}
	}
}


void pc88va_state::draw_indexed_gfx(bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 start_offset, u8 pal_base)
{
	uint8_t *gvram = (uint8_t *)m_gvram.target();

	for(int y = cliprect.min_x; y <= cliprect.max_y; y++)
	{
		// TODO: famista wants pitch width = 656, where it comes from?
		const u32 line_offset = ((y * 640) / 2) + start_offset;
		for(int x = cliprect.min_x; x <= cliprect.max_x; x+=2)
		{
			u16 x_char = (x >> 1);
			u32 bitmap_offset = line_offset + x_char;
			
			uint32_t color = (gvram[bitmap_offset] & 0xf0) >> 4;

			if(color && cliprect.contains(x, y))
				bitmap.pix(y, x) = m_palette->pen(color + pal_base);
			
			color = (gvram[bitmap_offset] & 0x0f) >> 0;

			if(color && cliprect.contains(x + 1, y))
				bitmap.pix(y, x + 1) = m_palette->pen(color + pal_base);
		}
	}
}

/****************************************
 * IDP - NEC μPD72022
 ***************************************/

/*
 * x--- ---- LP   Light-pen signal detection (with VA use failure)
 * -x-- ---- VB   Vertical elimination period
 * --x- ---- SC   Sprite control (sprite over/collision)
 * ---x ---- ER   Error occurrence
 * ---- x--- In the midst of execution of EMEN emulation development
 * ---- -x-- In the midst of BUSY command execution
 * ---- --x- OBF output data buffer full
 * ---- ---x IBF input data buffer full (command/parameter commonness)
 */
uint8_t pc88va_state::idp_status_r()
{

	return 0x00;
}


#define SYNC   0x10
#define DSPON  0x12
#define DSPOFF 0x13
#define DSPDEF 0x14
#define CURDEF 0x15
#define ACTSCR 0x16
#define CURS   0x1e
#define EMUL   0x8c
#define EXIT   0x88
#define SPRON  0x82
#define SPROFF 0x83
#define SPRSW  0x85
#define SPROV  0x81

void pc88va_state::idp_command_w(uint8_t data)
{
	switch(data)
	{
		/* 0x10 - SYNC: sets CRTC values */
		case SYNC:   m_cmd = SYNC;  m_buf_size = 14; m_buf_index = 0; break;

		/* 0x12 - DSPON: set DiSPlay ON and set up tvram table vreg */
		case DSPON:  m_cmd = DSPON; m_buf_size = 3;  m_buf_index = 0; break;

		/* 0x13 - DSPOFF: set DiSPlay OFF */
		case DSPOFF: m_cmd = DSPOFF; m_tsp.disp_on = 0; break;

		/* 0x14 - DSPDEF: set DiSPlay DEFinitions */
		case DSPDEF: m_cmd = DSPDEF; m_buf_size = 6; m_buf_index = 0; break;

		/* 0x15 - CURDEF: set CURsor DEFinition */
		case CURDEF: m_cmd = CURDEF; m_buf_size = 1; m_buf_index = 0; break;

		/* 0x16 - ACTSCR: ??? */
		case ACTSCR: m_cmd = ACTSCR; m_buf_size = 1; m_buf_index = 0; break;

		/* 0x15 - CURS: set CURSor position */
		case CURS:   m_cmd = CURS;   m_buf_size = 4; m_buf_index = 0; break;

		/* 0x8c - EMUL: set 3301 EMULation */
		case EMUL:   m_cmd = EMUL;   m_buf_size = 4; m_buf_index = 0; break;

		/* 0x88 - EXIT: ??? */
		case EXIT:   m_cmd = EXIT; break;

		/* 0x82 - SPRON: set SPRite ON */
		case SPRON:  m_cmd = SPRON;  m_buf_size = 3; m_buf_index = 0; break;

		/* 0x83 - SPROFF: set SPRite OFF */
		case SPROFF: m_cmd = SPROFF; m_tsp.spr_on = 0; break;

		/* 0x85 - SPRSW: ??? */
		case SPRSW:  m_cmd = SPRSW;  m_buf_size = 1; m_buf_index = 0; break;

		/* 0x81 - SPROV: set SPRite OVerflow information */
		/*
		-x-- ---- Sprite Over flag
		--x- ---- Sprite Collision flag
		---x xxxx First sprite that caused Sprite Over event
		*/
		case SPROV:  m_cmd = SPROV; /* TODO: where it returns the info? */ break;

		// TODO: 0x89 - mask command

		default: 
			m_cmd = 0x00; 
			printf("PC=%05x: Unknown IDP %02x cmd set\n",m_maincpu->pc(),data);
			break;
	}
}

// TODO: checkout this one
void pc88va_state::tsp_sprite_enable(uint32_t spr_offset, uint16_t sw_bit)
{
	uint32_t target_offset = (spr_offset & 0xffff)/2;
//  address_space &space = m_maincpu->space(AS_PROGRAM);

//  space.write_word(spr_offset, space.read_word(spr_offset) & ~0x200);
//  space.write_word(spr_offset, space.read_word(spr_offset) | (sw_bit & 0x200));
	m_tvram[target_offset] = (m_tvram[target_offset] & ~0x200) | (sw_bit & 0x200);
}

/* TODO: very preliminary, needs something showable first */
void pc88va_state::execute_sync_cmd()
{
	/*
	    ???? ???? [0] - unknown
	    ???? ???? [1] - unknown
	    --xx xxxx [2] - h blank start
	    --xx xxxx [3] - h border start
	    xxxx xxxx [4] - h visible area
	    --xx xxxx [5] - h border end
	    --xx xxxx [6] - h blank end
	    --xx xxxx [7] - h sync
	    --xx xxxx [8] - v blank start
	    --xx xxxx [9] - v border start
	    xxxx xxxx [A] - v visible area
	    -x-- ---- [B] - v visible area (bit 9)
	    --xx xxxx [C] - v border end
	    --xx xxxx [D] - v blank end
	    --xx xxxx [E] - v sync
	*/
	rectangle visarea;
	attoseconds_t refresh;
	uint16_t x_vis_area,y_vis_area;

	//printf("V blank start: %d\n",(sync_cmd[0x8]));
	//printf("V border start: %d\n",(sync_cmd[0x9]));
	//printf("V Visible Area: %d\n",(sync_cmd[0xa])|((sync_cmd[0xb] & 0x40)<<2));
	//printf("V border end: %d\n",(sync_cmd[0xc]));
	//printf("V blank end: %d\n",(sync_cmd[0xd]));

	x_vis_area = (m_buf_ram[4] + 1) * 4;
	y_vis_area = (m_buf_ram[0xa]) | ((m_buf_ram[0xb] & 0x40) << 2);

	visarea.set(0, x_vis_area - 1, 0, y_vis_area - 1);

	//if(y_vis_area == 400)
	//  refresh = HZ_TO_ATTOSECONDS(24800) * x_vis_area * y_vis_area; //24.8 KHz
	//else
	//  refresh = HZ_TO_ATTOSECONDS(15730) * x_vis_area * y_vis_area; //15.73 KHz

	refresh = HZ_TO_ATTOSECONDS(60);

	m_screen->configure(848, 448, visarea, refresh);
}

void pc88va_state::execute_dspon_cmd()
{
	/*
	[0] text table offset (hi word)
	[1] unknown
	[2] unknown
	*/
	m_tsp.tvram_vreg_offset = m_buf_ram[0] << 8;
	m_tsp.disp_on = 1;
}

void pc88va_state::execute_dspdef_cmd()
{
	/*
	[0] attr offset (lo word)
	[1] attr offset (hi word)
	[2] pitch (character code interval x 16, i.e. 0x20 = 2 bytes
	[3] line height
	[4] hline vertical position
	[5] blink number
	*/
	m_tsp.attr_offset = m_buf_ram[0] | m_buf_ram[1] << 8;
	m_tsp.pitch = (m_buf_ram[2] & 0xf0) >> 4;
	m_tsp.line_height = m_buf_ram[3] + 1;
	m_tsp.h_line_pos = m_buf_ram[4];
	m_tsp.blink = (m_buf_ram[5] & 0xf8) >> 3;
}

void pc88va_state::execute_curdef_cmd()
{
	/*
	xxxx x--- [0] Sprite Cursor number (sprite RAM entry)
	---- --x- [0] show cursor bit (actively modifies the spriteram entry)
	---- ---x [0] Blink Enable
	*/

	/* TODO: needs basic sprite emulation */
	m_tsp.curn = (m_buf_ram[0] & 0xf8);
	m_tsp.curn_blink = (m_buf_ram[0] & 1);

	tsp_sprite_enable(m_tsp.spr_offset + m_tsp.curn, (m_buf_ram[0] & 2) << 8);
}

void pc88va_state::execute_actscr_cmd()
{
	/*
	This command assigns a strip where the cursor is located.
	xxxx xxxx [0] strip ID * 32 (???)
	*/

	/* TODO: no idea about this command */
	//printf("ACTSCR: %02x\n",m_buf_ram[0]);
}

void pc88va_state::execute_curs_cmd()
{
	/*
	[0] Cursor Position Y (lo word)
	[1] Cursor Position Y (hi word)
	[2] Cursor Position X (lo word)
	[3] Cursor Position X (hi word)
	*/

	m_tsp.cur_pos_y = m_buf_ram[0] | m_buf_ram[1] << 8;
	m_tsp.cur_pos_x = m_buf_ram[2] | m_buf_ram[3] << 8;
}

void pc88va_state::execute_emul_cmd()
{
	/*
	[0] Emulate target strip ID x 32
	[1] The number of chars
	[2] The number of attributes
	[3] The number of lines
	*/

	// TODO: this starts 3301 video emulation
	//popmessage("Warning: TSP executes EMUL command, contact MESSdev");
}

void pc88va_state::execute_spron_cmd()
{
	/*
	[0] Sprite Table Offset (hi word)
	[1] (unknown / reserved)
	xxxx x--- [2] HSPN: Maximum number of sprites in one raster (num + 1) for Sprite Over
	---- --x- [2] MG: all sprites are 2x zoomed vertically when 1
	---- ---x [2] GR: 1 to enable the group collision detection
	*/
	m_tsp.spr_offset = m_buf_ram[0] << 8;
	m_tsp.spr_on = 1;
	printf("SPR TABLE %02x %02x %02x\n",m_buf_ram[0],m_buf_ram[1],m_buf_ram[2]);
}

void pc88va_state::execute_sprsw_cmd()
{
	/*
	Toggle an individual sprite in the sprite ram entry
	[0] xxxx x--- target sprite number
	[0] ---- --x- sprite off/on switch
	*/

	tsp_sprite_enable(m_tsp.spr_offset + (m_buf_ram[0] & 0xf8), (m_buf_ram[0] & 2) << 8);
}

void pc88va_state::idp_param_w(uint8_t data)
{
	if(m_cmd == DSPOFF || m_cmd == EXIT || m_cmd == SPROFF || m_cmd == SPROV) // no param commands
		return;

	m_buf_ram[m_buf_index] = data;
	m_buf_index++;

	if(m_buf_index >= m_buf_size)
	{
		m_buf_index = 0;
		switch(m_cmd)
		{
			case SYNC:      execute_sync_cmd();    break;
			case DSPON:     execute_dspon_cmd();   break;
			case DSPDEF:    execute_dspdef_cmd(); break;
			case CURDEF:    execute_curdef_cmd(); break;
			case ACTSCR:    execute_actscr_cmd(); break;
			case CURS:      execute_curs_cmd();   break;
			case EMUL:      execute_emul_cmd();   break;
			case SPRON:     execute_spron_cmd();  break;
			case SPRSW:     execute_sprsw_cmd();   break;

			default:
				//printf("%02x\n",data);
				break;
		}
	}
}

/****************************************
 * I/O handlers
 ***************************************/

/*
 * x--- ---- ---- ---- GDEN0 graphics display enable
 * -x-- ---- ---- ---- GVM superimpose if 1?
 * --x- ---- ---- ---- XVSP video signal output mode (0) inhibit scan signals
 * ---x ---- ---- ---- SYNCEN horizontal sync output (0) inhibit hsync
 * TODO: confirm following two
 * ---- x--- ---- ---- YMMD GVRAM mode (0) screen 0 only (1) screens 0 and 1
 * ---- -x-- ---- ---- DM gfx display mode (0) multiplane (1) single plane -> cfr. $153 GMSP
 * ---- ---- xx-- ---- RSM CRT raster scan mode
 * ---- ---- 0x-- ---- Non-interlace mode 0/1
 * ---- ---- 1x-- ---- Interlace mode 0/1
 * ---- ---- --x- ---- GDEN1 gfx display circuit reset (0) reset
 * ---- ---- ---x ---- SYNCM sync signal mode (0) internal (1) external
 * ---- ---- ---- --xx VW vertical resolution
 * ---- ---- ---- --00 400 lines
 * ---- ---- ---- --01 408 lines
 * ---- ---- ---- --10 200 lines
 * ---- ---- ---- --11 204 lines
 */
void pc88va_state::screen_ctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_screen_ctrl_reg);
}

u16 pc88va_state::screen_ctrl_r()
{
	return m_screen_ctrl_reg;
}

void pc88va_state::palette_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int r,g,b;
	COMBINE_DATA(&m_palram[offset]);

	b = (m_palram[offset] & 0x001e) >> 1;
	r = (m_palram[offset] & 0x03c0) >> 6;
	g = (m_palram[offset] & 0x7800) >> 11;

	m_palette->set_pen_color(offset,pal4bit(r),pal4bit(g),pal4bit(b));
}

void pc88va_state::video_pri_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video_pri_reg[offset]);
}

void pc88va_state::text_transpen_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_text_transpen);
	if (m_text_transpen & 0xfff0)
		popmessage("text transpen > 15 (%04x)", m_text_transpen);
}
