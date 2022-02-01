// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/********************************************************************************************

    PC-88VA (c) 1987 NEC

    A follow up of the regular PC-8801. It can also run PC-8801 software in compatible mode

    preliminary driver by Angelo Salese
    Special thanks to Fujix for his documentation translation help

    TODO:
    - What exact kind of garbage happens if you try to enable both direct and palette color
      modes to a graphic layer?
    - unemulated upd71071 demand mode;
    - What is exactly supposed to be a "bus slot"? Does it have an official name?
    - fdc "intelligent mode" has 0x7f as irq vector ... 0x7f is ld a,a and it IS NOT correctly
      hooked up by the current z80 core
    - PC-88VA stock version has two bogus opcodes. One is at 0xf0b15, another at 0xf0b31.
      Making a patch for the latter makes the system to jump into a "DIP-Switch" display.
      bp f0b31,pc=0xf0b36,g
      Update: it never reaches latter with V30->V50 CPU switch fix;
    - Fix floppy motor hook-up (floppy believes to be always in even if empty drive);
    - Support for PC8801 compatible mode & PC80S31K (floppy interface);

********************************************************************************************/

#include "emu.h"
#include "includes/pc88va.h"

#include "softlist_dev.h"


// TODO: verify clocks
#define MASTER_CLOCK    XTAL(8'000'000) // may be XTAL(31'948'800) / 4? (based on PC-8801 and PC-9801)
#define FM_CLOCK        (XTAL(31'948'800) / 8) // 3993600


void pc88va_state::video_start()
{
	m_kanjiram = std::make_unique<uint8_t[]>(0x4000);
	m_gfxdecode->gfx(2)->set_source(m_kanjiram.get());
	m_gfxdecode->gfx(3)->set_source(m_kanjiram.get());
}

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
		else // 4bpp mode (UNTESTED)
		{
			xsize = (xsize + 1) * 8;
			ysize = (ysize + 1) * 4;

			if(!(spda & 0x8000)) // correct?
				spda *= 2;

			spr_count = 0;

			for(int y_i=0;y_i<ysize;y_i++)
			{
				for(int x_i=0;x_i<xsize;x_i+=2)
				{
					for(int x_s=0;x_s<2;x_s++)
					{
						int pen = (bitswap<16>(tvram[(spda+spr_count) / 2],7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8)) >> (16-(x_s*8)) & 0xf;

						//if(bc != -1) //transparent pen
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
	else if(x == 0 && y == 0 && jis1 != 0) // debug stuff, to be nuked in the end
		printf("%02x\n",jis1);

	return 0;
}

void pc88va_state::draw_text(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t const *const tvram = m_tvram;
	// TODO: PCG select won't work with this arrangement
	uint8_t const *const kanji = memregion("kanji")->base();

	uint32_t count = tvram[m_tsp.tvram_vreg_offset/2];

	uint8_t attr_mode = tvram[(m_tsp.tvram_vreg_offset+0xa) / 2] & 0x1f;
	/* Note: bug in docs has the following two reversed */
	uint8_t screen_fg_col = (tvram[(m_tsp.tvram_vreg_offset+0xa) / 2] & 0xf000) >> 12;
	uint8_t screen_bg_col = (tvram[(m_tsp.tvram_vreg_offset+0xa) / 2] & 0x0f00) >> 8;

	for(int y=0;y<13;y++)
	{
		for(int x=0;x<80;x++)
		{
			uint8_t jis1 = (tvram[count] & 0x7f) + 0x20;
			uint8_t jis2 = (tvram[count] & 0x7f00) >> 8;
			uint16_t lr_half_gfx = ((tvram[count] & 0x8000) >> 15);

			uint32_t tile_num = calc_kanji_rom_addr(jis1,jis2,x,y);

			uint16_t attr = (tvram[count+(m_tsp.attr_offset/2)] & 0x00ff);

			uint8_t fg_col,bg_col,secret,reverse;
			//uint8_t blink,dwidc,dwid,uline,hline;
			fg_col = bg_col = reverse = secret = 0; //blink = dwidc = dwid = uline = hline = 0;

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
					popmessage("Illegal text tilemap attribute mode %02x, contact MESSdev",attr_mode);
					return;
			}

			for(int yi=0;yi<16;yi++)
			{
				for(int xi=0;xi<8;xi++)
				{
					int res_x = x*8+xi;
					int res_y = y*16+yi;

					if(!cliprect.contains(res_x, res_y))
						continue;

					int pen = kanji[((yi*2)+lr_half_gfx)+tile_num] >> (7-xi) & 1;

					if(reverse)
						pen = pen & 1 ? bg_col : fg_col;
					else
						pen = pen & 1 ? fg_col : bg_col;

					if(secret) { pen = 0; } //hide text

					if(pen != -1) //transparent
						bitmap.pix(res_y, res_x) = m_palette->pen(pen);
				}
			}

			count++;
			count&=0xffff;
		}
	}
}

uint32_t pc88va_state::screen_update_pc88va(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t pri,cur_pri_lv;
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

	screen_pri = (m_video_pri_reg[1] & 0x00f0) >> 4; // priority 5
	screen_pri|= (m_video_pri_reg[1] & 0x000f) << 4; // priority 4
	screen_pri|= (m_video_pri_reg[0] & 0xf000) >> 4; // priority 3
	screen_pri|= (m_video_pri_reg[0] & 0x0f00) << 4; // priority 2
	screen_pri|= (m_video_pri_reg[0] & 0x00f0) << 12; // priority 1
	screen_pri|= (m_video_pri_reg[0] & 0x000f) << 20; // priority 0

	for(pri=0;pri<6;pri++)
	{
		cur_pri_lv = (screen_pri >> (pri*4)) & 0xf;

		if(cur_pri_lv & 8) // enable layer
		{
			if(pri <= 1) // (direct color mode, priority 5 and 4)
			{
				// 8 = graphic 0
				// 9 = graphic 1
			}
			else
			{
				switch(cur_pri_lv & 3) // (palette color mode)
				{
					case 0: draw_text(bitmap,cliprect); break;
					case 1: if(m_tsp.spr_on) { draw_sprites(bitmap,cliprect); } break;
					case 2: /* A = graphic 0 */ break;
					case 3: /* B = graphic 1 */ break;
				}
			}
		}
	}

	return 0;
}

void pc88va_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_PC8801FD_UPD765_TC_TO_ZERO:
		pc8801fd_upd765_tc_to_zero(param);
		break;
	case TIMER_T3_MOUSE_CALLBACK:
		t3_mouse_callback(param);
		break;
	case TIMER_PC88VA_FDC_TIMER:
		pc88va_fdc_timer(param);
		break;
	case TIMER_PC88VA_FDC_MOTOR_START_0:
		pc88va_fdc_motor_start_0(param);
		break;
	case TIMER_PC88VA_FDC_MOTOR_START_1:
		pc88va_fdc_motor_start_1(param);
		break;
	default:
		throw emu_fatalerror("Unknown id in pc88va_state::device_timer");
	}
}


void pc88va_state::pc88va_map(address_map &map)
{
	map(0x00000, 0x7ffff).ram();
//  map(0x80000, 0x9ffff).ram(); // EMM
	map(0xa0000, 0xdffff).m(m_sysbank, FUNC(address_map_bank_device::amap16));
	map(0xe0000, 0xeffff).bankr("rom00_bank");
	map(0xf0000, 0xfffff).bankr("rom10_bank");
}

/* 0x00000 - 0x3ffff Kanji ROM 1*/
/* 0x40000 - 0x4ffff Kanji ROM 2*/
/* 0x50000 - 0x53fff Backup RAM */
/* above that is a NOP presumably */
uint8_t pc88va_state::kanji_ram_r(offs_t offset)
{
	return m_kanjiram[offset];
}

void pc88va_state::kanji_ram_w(offs_t offset, uint8_t data)
{
	// TODO: there's an area that can be write protected
	m_kanjiram[offset] = data;
	m_gfxdecode->gfx(2)->mark_dirty(offset / 8);
	m_gfxdecode->gfx(3)->mark_dirty(offset / 32);
}


void pc88va_state::sysbank_map(address_map &map)
{
	// 0 select bus slot (?)
	// 1 tvram
	map(0x040000, 0x04ffff).ram().share("tvram");
	// 4 gvram
	map(0x100000, 0x13ffff).ram().share("gvram");
	// 8-9 kanji
	map(0x200000, 0x23ffff).rom().region("kanji", 0x00000);
	map(0x240000, 0x24ffff).rom().region("kanji", 0x40000);
	map(0x250000, 0x253fff).rw(FUNC(pc88va_state::kanji_ram_r),FUNC(pc88va_state::kanji_ram_w));
	// c-d dictionary
	map(0x300000, 0x37ffff).rom().region("dictionary", 0);
}

/* IDP = NEC uPD72022 */
uint8_t pc88va_state::idp_status_r()
{
/*
    x--- ---- LP   Light-pen signal detection (with VA use failure)
    -x-- ---- VB   Vertical elimination period
    --x- ---- SC   Sprite control (sprite over/collision)
    ---x ---- ER   Error occurrence
    ---- x--- In the midst of execution of EMEN emulation development
    ---- -x-- In the midst of BUSY command execution
    ---- --x- OBF output data buffer full
    ---- ---x IBF input data buffer full (command/parameter commonness)
*/
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

		/* TODO: 0x89 shouldn't trigger, should be one of the above commands */
		/* Update: actually 0x89 is mask command */
		default:   m_cmd = 0x00; printf("PC=%05x: Unknown IDP %02x cmd set\n",m_maincpu->pc(),data); break;
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

	x_vis_area = m_buf_ram[4] * 4;
	y_vis_area = (m_buf_ram[0xa])|((m_buf_ram[0xb] & 0x40)<<2);

	visarea.set(0, x_vis_area - 1, 0, y_vis_area - 1);

	//if(y_vis_area == 400)
	//  refresh = HZ_TO_ATTOSECONDS(24800) * x_vis_area * y_vis_area; //24.8 KHz
	//else
	//  refresh = HZ_TO_ATTOSECONDS(15730) * x_vis_area * y_vis_area; //15.73 KHz

	refresh = HZ_TO_ATTOSECONDS(60);

	m_screen->configure(640, 480, visarea, refresh);
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

void pc88va_state::palette_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int r,g,b;
	COMBINE_DATA(&m_palram[offset]);

	b = (m_palram[offset] & 0x001e) >> 1;
	r = (m_palram[offset] & 0x03c0) >> 6;
	g = (m_palram[offset] & 0x7800) >> 11;

	m_palette->set_pen_color(offset,pal4bit(r),pal4bit(g),pal4bit(b));
}

uint16_t pc88va_state::sys_port4_r()
{
	uint8_t vrtc,sw1;
	vrtc = (m_screen->vpos() < 200) ? 0x20 : 0x00; // vblank

	sw1 = (ioport("DSW")->read() & 1) ? 2 : 0;

	return vrtc | sw1 | 0xc0;
}

uint16_t pc88va_state::bios_bank_r()
{
	return m_bank_reg;
}

void pc88va_state::bios_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/*
	-x-- ---- ---- ---- SMM (compatibility mode)
	---x ---- ---- ---- GMSP (VRAM drawing mode)
	---- xxxx ---- ---- SMBC (0xa0000 - 0xdffff RAM bank)
	---- ---- xxxx ---- RBC1 (0xf0000 - 0xfffff ROM bank)
	---- ---- ---- xxxx RBC0 (0xe0000 - 0xeffff ROM bank)
	*/
	COMBINE_DATA(&m_bank_reg);

	/* SMBC */
	m_sysbank->set_bank((m_bank_reg & 0xf00) >> 8);

	/* RBC1 */
	{
		uint8_t *ROM10 = memregion("rom10")->base();

		if((m_bank_reg & 0xe0) == 0x00)
			membank("rom10_bank")->set_base(&ROM10[(m_bank_reg & 0x10) ? 0x10000 : 0x00000]);
	}

	/* RBC0 */
	{
		uint8_t *ROM00 = memregion("rom00")->base();

		membank("rom00_bank")->set_base(&ROM00[(m_bank_reg & 0xf)*0x10000]); // TODO: docs says that only 0 - 5 are used, dunno why ...
	}
}

uint8_t pc88va_state::rom_bank_r()
{
	return 0xff; // bit 7 low is va91 rom bank status
}

uint8_t pc88va_state::key_r(offs_t offset)
{
	// note row D bit 2 does something at POST ... some kind of test mode?
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3",
											"KEY4", "KEY5", "KEY6", "KEY7",
											"KEY8", "KEY9", "KEYA", "KEYB",
											"KEYC", "KEYD", "KEYE", "KEYF" };

	return ioport(keynames[offset])->read();
}

void pc88va_state::backupram_wp_1_w(uint16_t data)
{
	m_backupram_wp = 1;
}

void pc88va_state::backupram_wp_0_w(uint16_t data)
{
	m_backupram_wp = 0;
}

uint8_t pc88va_state::hdd_status_r()
{
	return 0x20;
}

uint8_t pc88va_state::pc88va_fdc_r(offs_t offset)
{
	printf("%08x\n",offset);

	switch(offset*2)
	{
		case 0x00: return 0; // FDC mode register
		case 0x02: return 0; // FDC control port 0
		case 0x04: return 0; // FDC control port 1
		/* ---x ---- RDY: (0) Busy (1) Ready */
		case 0x06: // FDC control port 2
			return 0;
	}

	return 0xff;
}

TIMER_CALLBACK_MEMBER(pc88va_state::pc88va_fdc_timer)
{
	if(m_fdc_ctrl_2 & 4) // XTMASK
	{
		m_pic2->ir3_w(0);
		m_pic2->ir3_w(1);
	}
}

TIMER_CALLBACK_MEMBER(pc88va_state::pc88va_fdc_motor_start_0)
{
	m_fdd[0]->get_device()->mon_w(0);
	m_fdc_motor_status[0] = 1;
}

TIMER_CALLBACK_MEMBER(pc88va_state::pc88va_fdc_motor_start_1)
{
	m_fdd[1]->get_device()->mon_w(0);
	m_fdc_motor_status[1] = 1;
}

void pc88va_state::pc88va_fdc_update_ready(floppy_image_device *, int)
{
	bool ready = m_fdc_ctrl_2 & 0x40;

	floppy_image_device *floppy;
	floppy = m_fdd[0]->get_device();
	if(floppy && ready)
		ready = floppy->ready_r();
	floppy = m_fdd[1]->get_device();
	if(floppy && ready)
		ready = floppy->ready_r();

	m_fdc->ready_w(ready);
}

void pc88va_state::pc88va_fdc_w(offs_t offset, uint8_t data)
{
	printf("%08x %02x\n",offset<<1,data);
	switch(offset<<1)
	{
		/*
		---- ---x MODE: FDC op mode (0) Intelligent (1) DMA
		*/
		case 0x00: // FDC mode register
			m_fdc_mode = data & 1;
			#if TEST_SUBFDC
			m_fdccpu->set_input_line(INPUT_LINE_HALT, (m_fdc_mode) ? ASSERT_LINE : CLEAR_LINE);
			#endif
			break;
		/*
		--x- ---- CLK: FDC clock selection (0) 4.8MHz (1) 8 MHz
		---x ---- DS1: Prohibition of the drive selection of FDC (0) Permission (1) Prohibition
		---- xx-- TD1/TD0: Drive 1/0 track density (0) 48 TPI (1) 96 TPI
		---- --xx RV1/RV0: Drive 1/0 mode selection (0) 2D and 2DD mode (1) 2HD mode
		*/
		case 0x02: // FDC control port 0
			m_fdd[0]->get_device()->set_rpm(data & 0x01 ? 360 : 300);
			m_fdd[1]->get_device()->set_rpm(data & 0x02 ? 360 : 300);

			m_fdc->set_rate(data & 0x20 ? 500000 : 250000);
			break;
		/*
		---- x--- PCM: ?
		---- --xx M1/M0: Drive 1/0 motor control (0) NOP (1) Change motor status
		*/
		case 0x04:
			if(data & 1)
			{
				m_fdd[0]->get_device()->mon_w(1);
				if(m_fdc_motor_status[0] == 0)
					timer_set(attotime::from_msec(505), TIMER_PC88VA_FDC_MOTOR_START_0);
				else
					m_fdc_motor_status[0] = 0;
			}

			if(data & 2)
			{
				m_fdd[1]->get_device()->mon_w(1);
				if(m_fdc_motor_status[1] == 0)
					timer_set(attotime::from_msec(505), TIMER_PC88VA_FDC_MOTOR_START_1);
				else
					m_fdc_motor_status[1] = 0;
			}

			break;
		/*
		x--- ---- FDCRST: FDC Reset
		-xx- ---- FDCFRY FRYCEN: FDC force ready control
		---x ---- DMAE: DMA Enable (0) Prohibit DMA (1) Enable DMA
		---- -x-- XTMASK: FDC timer IRQ mask (0) Disable (1) Enable
		---- ---x TTRG: FDC timer trigger (0) FDC timer clearing (1) FDC timer start
		*/
		case 0x06:
			//printf("%02x\n",data);
			if(data & 1)
				timer_set(attotime::from_msec(100), TIMER_PC88VA_FDC_TIMER);

			if((m_fdc_ctrl_2 & 0x10) != (data & 0x10))
				m_dmac->dreq2_w(1);

			if(data & 0x80) // correct?
				m_fdc->reset();

			m_fdc_ctrl_2 = data;

			pc88va_fdc_update_ready(nullptr, 0);

			break; // FDC control port 2
	}
}


uint16_t pc88va_state::sysop_r()
{
	uint8_t sys_op;

	sys_op = ioport("SYSOP_SW")->read() & 3;

	return 0xfffc | sys_op; // docs says all the other bits are high
}

uint16_t pc88va_state::screen_ctrl_r()
{
	return m_screen_ctrl_reg;
}

void pc88va_state::screen_ctrl_w(uint16_t data)
{
	m_screen_ctrl_reg = data;
}

TIMER_CALLBACK_MEMBER(pc88va_state::t3_mouse_callback)
{
	if(m_timer3_io_reg & 0x80)
	{
		m_pic2->ir5_w(0);
		m_pic2->ir5_w(1);
		m_t3_mouse_timer->adjust(attotime::from_hz(120 >> (m_timer3_io_reg & 3)));
	}
}

void pc88va_state::timer3_ctrl_reg_w(uint8_t data)
{
	/*
	x--- ---- MINTEN (TCU irq enable)
	---- --xx general purpose timer 3 interval (120, 60, 30, 15)
	*/
	m_timer3_io_reg = data;

	if(data & 0x80)
		m_t3_mouse_timer->adjust(attotime::from_hz(120 >> (m_timer3_io_reg & 3)));
	else
	{
		m_pic2->ir5_w(0);
		m_t3_mouse_timer->adjust(attotime::never);
	}
}

void pc88va_state::video_pri_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video_pri_reg[offset]);
}

uint8_t pc88va_state::backupram_dsw_r(offs_t offset)
{
	if(offset == 0)
		return m_kanjiram[0x1fc2 / 2] & 0xff;

	return m_kanjiram[0x1fc6 / 2] & 0xff;
}

void pc88va_state::sys_port1_w(uint8_t data)
{
	// ...
}

#if !TEST_SUBFDC
uint8_t pc88va_state::no_subfdc_r()
{
	return machine().rand();
}
#endif

void pc88va_state::pc88va_io_map(address_map &map)
{
	map(0x0000, 0x000f).r(FUNC(pc88va_state::key_r)); // Keyboard ROW reading
//  map(0x0010, 0x0010) Printer / Calendar Clock Interface
	map(0x0020, 0x0021).noprw(); // RS-232C
	map(0x0030, 0x0031).rw(FUNC(pc88va_state::backupram_dsw_r), FUNC(pc88va_state::sys_port1_w)); // 0x30 (R) DSW1 (W) Text Control Port 0 / 0x31 (R) DSW2 (W) System Port 1
//  map(0x0032, 0x0032) (R) ? (W) System Port 2
//  map(0x0034, 0x0034) GVRAM Control Port 1
//  map(0x0035, 0x0035) GVRAM Control Port 2
	map(0x0040, 0x0041).r(FUNC(pc88va_state::sys_port4_r)); // (R) System Port 4 (W) System port 3 (strobe port)
	map(0x0044, 0x0045).mirror(0x0002).rw("ym", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
//  map(0x005c, 0x005c) (R) GVRAM status
//  map(0x005c, 0x005f) (W) GVRAM selection
//  map(0x0070, 0x0070) ? (*)
//  map(0x0071, 0x0071) Expansion ROM select (*)
//  map(0x0078, 0x0078) Memory offset increment (*)
//  map(0x0080, 0x0081) HDD related
	map(0x0082, 0x0082).r(FUNC(pc88va_state::hdd_status_r));// HDD control, byte access 7-0
//  map(0x00bc, 0x00bf) d8255 1
//  map(0x00e2, 0x00e3) Expansion RAM selection (*)
//  map(0x00e4, 0x00e4) 8214 IRQ control (*)
//  map(0x00e6, 0x00e6) 8214 IRQ mask (*)
//  map(0x00e8, 0x00e9) ? (*)
//  map(0x00ec, 0x00ed) ? (*)
	#if TEST_SUBFDC
	map(0x00fc, 0x00ff).rw("d8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write)); // d8255 2, FDD
	#else
	map(0x00fc, 0x00ff).r(FUNC(pc88va_state::no_subfdc_r)).nopw();
	#endif

	map(0x0100, 0x0101).rw(FUNC(pc88va_state::screen_ctrl_r), FUNC(pc88va_state::screen_ctrl_w)); // Screen Control Register
//  map(0x0102, 0x0103) Graphic Screen Control Register
	map(0x0106, 0x0109).w(FUNC(pc88va_state::video_pri_w)); // Palette Control Register (priority) / Direct Color Control Register (priority)
//  map(0x010a, 0x010b) Picture Mask Mode Register
//  map(0x010c, 0x010d) Color Palette Mode Register
//  map(0x010e, 0x010f) Backdrop Color Register
//  map(0x0110, 0x0111) Color Code/Plain Mask Register
//  map(0x0124, 0x0125) ? (related to Transparent Color of Graphic Screen 0)
//  map(0x0126, 0x0127) ? (related to Transparent Color of Graphic Screen 1)
//  map(0x012e, 0x012f) ? (related to Transparent Color of Text/Sprite)
//  map(0x0130, 0x0137) Picture Mask Parameter
	map(0x0142, 0x0142).rw(FUNC(pc88va_state::idp_status_r), FUNC(pc88va_state::idp_command_w)); //Text Controller (IDP) - (R) Status (W) command
	map(0x0146, 0x0146).w(FUNC(pc88va_state::idp_param_w)); //Text Controller (IDP) - (R/W) Parameter
//  map(0x0148, 0x0149) Text control port 1
//  map(0x014c, 0x014f) ? CG Port
	map(0x0150, 0x0151).r(FUNC(pc88va_state::sysop_r)); // System Operational Mode
	map(0x0152, 0x0153).rw(FUNC(pc88va_state::bios_bank_r), FUNC(pc88va_state::bios_bank_w)); // Memory Map Register
//  map(0x0154, 0x0155) Refresh Register (wait states)
	map(0x0156, 0x0156).r(FUNC(pc88va_state::rom_bank_r)); // ROM bank status
//  map(0x0158, 0x0159) Interruption Mode Modification
//  map(0x015c, 0x015f) NMI mask port (strobe port)
	map(0x0160, 0x016f).rw(m_dmac, FUNC(am9517a_device::read), FUNC(am9517a_device::write)); // DMA Controller
	map(0x0184, 0x0187).rw("pic8259_slave", FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x0188, 0x018b).rw("pic8259_master", FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff); // ICU, also controls 8214 emulation
//  map(0x0190, 0x0191) System Port 5
//  map(0x0196, 0x0197) Keyboard sub CPU command port
	map(0x0198, 0x0199).w(FUNC(pc88va_state::backupram_wp_1_w)); //Backup RAM write inhibit
	map(0x019a, 0x019b).w(FUNC(pc88va_state::backupram_wp_0_w)); //Backup RAM write permission
	map(0x01a0, 0x01a7).rw("pit8253", FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);// vTCU (timer counter unit)
	map(0x01a8, 0x01a8).w(FUNC(pc88va_state::timer3_ctrl_reg_w)); // General-purpose timer 3 control port
	map(0x01b0, 0x01b7).rw(FUNC(pc88va_state::pc88va_fdc_r), FUNC(pc88va_state::pc88va_fdc_w)).umask16(0x00ff);// FDC related (765)
	map(0x01b8, 0x01bb).m(m_fdc, FUNC(upd765a_device::map)).umask16(0x00ff);
//  map(0x01c0, 0x01c1) ?
	map(0x01c6, 0x01c7).nopw(); // ???
	map(0x01c8, 0x01cf).rw("d8255_3", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0xff00); //i8255 3 (byte access)
//  map(0x01d0, 0x01d1) Expansion RAM bank selection
	map(0x0200, 0x021f).ram(); // Frame buffer 0 control parameter
	map(0x0220, 0x023f).ram(); // Frame buffer 1 control parameter
	map(0x0240, 0x025f).ram(); // Frame buffer 2 control parameter
	map(0x0260, 0x027f).ram(); // Frame buffer 3 control parameter
	map(0x0300, 0x033f).ram().w(FUNC(pc88va_state::palette_ram_w)).share("palram"); // Palette RAM (xBBBBxRRRRxGGGG format)

//  map(0x0500, 0x05ff) GVRAM
//  map(0x1000, 0xfeff) user area (???)
	map(0xff00, 0xffff).noprw(); // CPU internal use
}
// (*) are specific N88 V1 / V2 ports

TIMER_CALLBACK_MEMBER(pc88va_state::pc8801fd_upd765_tc_to_zero)
{
	m_fdc->tc_w(false);
}

/* FDC subsytem CPU */
#if TEST_SUBFDC
void pc88va_state::pc88va_z80_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x7fff).ram();
}

uint8_t pc88va_state::upd765_tc_r()
{
	m_fdc->tc_w(true);
	timer_set(attotime::from_usec(50), TIMER_PC8801FD_UPD765_TC_TO_ZERO);
	return 0;
}

void pc88va_state::fdc_irq_vector_w(uint8_t data)
{
	m_fdc_irq_opcode = data;
}

void pc88va_state::upd765_mc_w(uint8_t data)
{
	m_fdd[0]->get_device()->mon_w(!(data & 1));
	m_fdd[1]->get_device()->mon_w(!(data & 2));
}

void pc88va_state::pc88va_z80_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xf0, 0xf0).w(FUNC(pc88va_state::fdc_irq_vector_w)); // Interrupt Opcode Port
//  map(0xf4, 0xf4) // Drive Control Port
	map(0xf8, 0xf8).rw(FUNC(pc88va_state::upd765_tc_r), FUNC(pc88va_state::upd765_mc_w)); // (R) Terminal Count Port (W) Motor Control Port
	map(0xfa, 0xfb).m(m_fdc, FUNC(upd765a_device::map));
	map(0xfc, 0xff).rw("d8255_2s", FUNC(i8255_device::read), FUNC(i8255_device::write));
}
#endif

/* TODO: active low or active high? */
static INPUT_PORTS_START( pc88va )
	PORT_START("KEY0")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("0 (PAD)") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("1 (PAD)") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("2 (PAD)") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("3 (PAD)") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("4 (PAD)") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("5 (PAD)") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("6 (PAD)") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("7 (PAD)") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("KEY1")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("8 (PAD)") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("9 (PAD)") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("* (PAD)") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("+ (PAD)") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("= (PAD)") // PORT_CODE(KEYCODE_EQUAL_PAD) PORT_CHAR(UCHAR_MAMEKEY(EQUAL_PAD))
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(", (PAD)") // PORT_CODE(KEYCODE_EQUAL_PAD) PORT_CHAR(UCHAR_MAMEKEY(EQUAL_PAD))
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(". (PAD)") // PORT_CODE(KEYCODE_EQUAL_PAD) PORT_CHAR(UCHAR_MAMEKEY(EQUAL_PAD))
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("RETURN (PAD)") //PORT_CODE(KEYCODE_RETURN_PAD) PORT_CHAR(UCHAR_MAMEKEY(RETURN_PAD))

	PORT_START("KEY2")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("@") // PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("KEY3")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("KEY4")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("KEY5")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("[")
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("\xC2\xA5") /* Yen */
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("]")
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("^")
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("-")

	PORT_START("KEY6")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("0")  PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("1 !")  PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("3 #")  PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("4 $")  PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("5 %")  PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("6 &")  PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("7 '")  PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("KEY7")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("8 (")  PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("9 )")  PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(": *")  //PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("; +")  //PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(", <")  //PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(". >")  //PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("/ ?")  //PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("/ ?")  //PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("KEY8")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("CLR") // PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Up")  PORT_CODE(KEYCODE_UP)  /* Up */
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)  /* Right */
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("INSDEL")
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("GRPH")
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("KANA") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)

	PORT_START("KEY9")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("STOP") // PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F1 (mirror)") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F2 (mirror)") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F3 (mirror)") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F4 (mirror)") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F5 (mirror)") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("SPACES") // PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)

	PORT_START("KEYA")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)  /* Down */
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)  /* Left */
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("HELP") // PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("COPY") // PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("- (mirror)") // PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("/") // PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("KEYB")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD)  PORT_NAME("Roll Up")  PORT_CODE(KEYCODE_PGUP)  /* Roll Up */
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD)  PORT_NAME("Roll Down")  PORT_CODE(KEYCODE_PGDN)  /* Roll Down */
	PORT_BIT(0xfc,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("KEYC")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("INS") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)

	PORT_START("KEYD")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F9") PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F10") PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) // Conversion?
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) // Decision?
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)

	/* TODO: I don't understand the meaning of several of these */
	PORT_START("KEYE")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD)
	PORT_BIT(0xc0,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("KEYF")
	PORT_BIT(0xff,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "CRT Mode" )
	PORT_DIPSETTING(    0x01, "15.7 KHz" )
	PORT_DIPSETTING(    0x00, "24.8 KHz" )
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

	PORT_START("SPEED_SW")
	PORT_DIPNAME( 0x01, 0x01, "Speed Mode" )
	PORT_DIPSETTING(    0x01, "H Mode" )
	PORT_DIPSETTING(    0x00, "S Mode" )

	PORT_START("SYSOP_SW")
	PORT_DIPNAME( 0x03, 0x01, "System Operational Mode" )
//  PORT_DIPSETTING(    0x00, "Reserved" )
	PORT_DIPSETTING(    0x01, "N88 V2 Mode" )
	PORT_DIPSETTING(    0x02, "N88 V1 Mode" )
//  PORT_DIPSETTING(    0x03, "???" )
INPUT_PORTS_END

static const gfx_layout pc88va_chars_8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout pc88va_chars_16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

// same as above but with static size
static const gfx_layout pc88va_kanji_8x8 =
{
	8,8,
	0x4000/8,
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout pc88va_kanji_16x16 =
{
	16,16,
	0x4000/32,
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

// TODO: decoded for debugging purpose, this will be nuked in the end ...
static GFXDECODE_START( gfx_pc88va )
	GFXDECODE_ENTRY( "kanji",   0x00000, pc88va_chars_8x8,    0, 1 )
	GFXDECODE_ENTRY( "kanji",   0x00000, pc88va_chars_16x16,  0, 1 )
	GFXDECODE_ENTRY( nullptr,   0x00000, pc88va_kanji_8x8,    0, 1 )
	GFXDECODE_ENTRY( nullptr,   0x00000, pc88va_kanji_16x16,  0, 1 )
GFXDECODE_END

uint8_t pc88va_state::cpu_8255_c_r()
{
	return m_i8255_1_pc >> 4;
}

void pc88va_state::cpu_8255_c_w(uint8_t data)
{
	m_i8255_0_pc = data;
}

uint8_t pc88va_state::fdc_8255_c_r()
{
	return m_i8255_0_pc >> 4;
}

void pc88va_state::fdc_8255_c_w(uint8_t data)
{
	m_i8255_1_pc = data;
}

uint8_t pc88va_state::r232_ctrl_porta_r()
{
	uint8_t sw5, sw4, sw3, sw2,speed_sw;

	speed_sw = (ioport("SPEED_SW")->read() & 1) ? 0x20 : 0x00;
	sw5 = (ioport("DSW")->read() & 0x10);
	sw4 = (ioport("DSW")->read() & 0x08);
	sw3 = (ioport("DSW")->read() & 0x04);
	sw2 = (ioport("DSW")->read() & 0x02);

	return 0xc1 | sw5 | sw4 | sw3 | sw2 | speed_sw;
}

uint8_t pc88va_state::r232_ctrl_portb_r()
{
	uint8_t xsw1;

	xsw1 = (ioport("DSW")->read() & 1) ? 0 : 8;

	return 0xf7 | xsw1;
}

uint8_t pc88va_state::r232_ctrl_portc_r()
{
	return 0xff;
}

void pc88va_state::r232_ctrl_porta_w(uint8_t data)
{
	// ...
}

void pc88va_state::r232_ctrl_portb_w(uint8_t data)
{
	// ...
}

void pc88va_state::r232_ctrl_portc_w(uint8_t data)
{
	// ...
}

uint8_t pc88va_state::get_slave_ack(offs_t offset)
{
	if (offset==7) { // IRQ = 7
		return m_pic2->acknowledge();
	}
	return 0x00;
}

void pc88va_state::machine_start()
{
	m_t3_mouse_timer = timer_alloc(TIMER_T3_MOUSE_CALLBACK);
	m_t3_mouse_timer->adjust(attotime::never);
	floppy_image_device *floppy;
	floppy = m_fdd[0]->get_device();
	if(floppy)
		floppy->setup_ready_cb(floppy_image_device::ready_cb(&pc88va_state::pc88va_fdc_update_ready, this));

	floppy = m_fdd[1]->get_device();
	if(floppy)
		floppy->setup_ready_cb(floppy_image_device::ready_cb(&pc88va_state::pc88va_fdc_update_ready, this));

	m_fdd[0]->get_device()->set_rpm(300);
	m_fdd[1]->get_device()->set_rpm(300);
	m_fdc->set_rate(250000);

}

void pc88va_state::machine_reset()
{
	uint8_t *ROM00 = memregion("rom00")->base();
	uint8_t *ROM10 = memregion("rom10")->base();

	membank("rom10_bank")->set_base(&ROM10[0x00000]);
	membank("rom00_bank")->set_base(&ROM00[0x00000]);

	m_bank_reg = 0x4100;
	m_sysbank->set_bank(1);
	m_backupram_wp = 1;

	/* default palette */
	{
		uint8_t i;
		for(i=0;i<32;i++)
			m_palette->set_pen_color(i,pal1bit((i & 2) >> 1),pal1bit((i & 4) >> 2),pal1bit(i & 1));
	}

	m_tsp.tvram_vreg_offset = 0;

	m_fdc_mode = 0;
	m_fdc_irq_opcode = 0x00; //0x7f ld a,a !

	#if TEST_SUBFDC
	m_fdccpu->set_input_line_vector(0, 0); // Z80
	#endif

	m_fdc_motor_status[0] = 0;
	m_fdc_motor_status[1] = 0;
}

INTERRUPT_GEN_MEMBER(pc88va_state::pc88va_vrtc_irq)
{
	m_pic1->ir2_w(0);
	m_pic1->ir2_w(1);
}

WRITE_LINE_MEMBER(pc88va_state::pc88va_pit_out0_changed)
{
	if(state)
	{
		m_pic1->ir0_w(0);
		m_pic1->ir0_w(1);
	}
}

WRITE_LINE_MEMBER( pc88va_state::fdc_drq )
{
	printf("%02x DRQ\n",state);
	m_dmac->dreq2_w(state);
}

WRITE_LINE_MEMBER( pc88va_state::fdc_irq )
{
	if(m_fdc_mode && state)
	{
		//printf("%d\n",state);
		m_pic2->ir3_w(0);
		m_pic2->ir3_w(1);
	}
	#if TEST_SUBFDC
	else
		m_fdccpu->set_input_line(0, HOLD_LINE);
	#endif
}

WRITE_LINE_MEMBER(pc88va_state::pc88va_hlda_w)
{
//  m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	m_dmac->hack_w(state);

//  printf("%02x HLDA\n",state);
}

WRITE_LINE_MEMBER( pc88va_state::pc88va_tc_w )
{
	/* floppy terminal count */
	m_fdc->tc_w(state);

//  printf("TC %02x\n",state);
}


uint8_t pc88va_state::fdc_dma_r()
{
	printf("R DMA\n");
	return m_fdc->dma_r();
}

void pc88va_state::fdc_dma_w(uint8_t data)
{
	printf("W DMA %08x\n",data);
	m_fdc->dma_w(data);
}

void pc88va_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_XDF_FORMAT);
}

static void pc88va_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
}

uint8_t pc88va_state::dma_memr_cb(offs_t offset)
{
	printf("%08x\n",offset);
	return 0;
}

void pc88va_state::dma_memw_cb(offs_t offset, uint8_t data)
{
	printf("%08x %02x\n",offset,data);
}


void pc88va_state::pc88va(machine_config &config)
{
	V50(config, m_maincpu, MASTER_CLOCK); // μPD9002, aka V30 + μPD70008AC (for PC8801 compatibility mode)
	m_maincpu->set_addrmap(AS_PROGRAM, &pc88va_state::pc88va_map);
	m_maincpu->set_addrmap(AS_IO, &pc88va_state::pc88va_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(pc88va_state::pc88va_vrtc_irq));
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

#if TEST_SUBFDC
	z80_device &fdccpu(Z80(config, "fdccpu", 8000000));        /* 8 MHz */
	fdccpu.set_addrmap(AS_PROGRAM, &pc88va_state::pc88va_z80_map);
	fdccpu.set_addrmap(AS_IO, &pc88va_state::pc88va_z80_io_map);

	config.m_perfect_cpu_quantum = subtag("maincpu");
#endif

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 640-1, 0, 200-1);
	m_screen->set_screen_update(FUNC(pc88va_state::screen_update_pc88va));

	PALETTE(config, m_palette).set_entries(32);
//  m_palette->set_init(FUNC(pc88va_state::pc8801));
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pc88va);

	i8255_device &d8255_2(I8255(config, "d8255_2"));
	d8255_2.in_pa_callback().set("d8255_2s", FUNC(i8255_device::pb_r));
	d8255_2.in_pb_callback().set("d8255_2s", FUNC(i8255_device::pa_r));
	d8255_2.in_pc_callback().set(FUNC(pc88va_state::cpu_8255_c_r));
	d8255_2.out_pc_callback().set(FUNC(pc88va_state::cpu_8255_c_w));

	i8255_device &d8255_3(I8255(config, "d8255_3"));
	d8255_3.in_pa_callback().set(FUNC(pc88va_state::r232_ctrl_porta_r));
	d8255_3.out_pa_callback().set(FUNC(pc88va_state::r232_ctrl_porta_w));
	d8255_3.in_pb_callback().set(FUNC(pc88va_state::r232_ctrl_portb_r));
	d8255_3.out_pb_callback().set(FUNC(pc88va_state::r232_ctrl_portb_w));
	d8255_3.in_pc_callback().set(FUNC(pc88va_state::r232_ctrl_portc_r));
	d8255_3.out_pc_callback().set(FUNC(pc88va_state::r232_ctrl_portc_w));

	i8255_device &d8255_2s(I8255(config, "d8255_2s"));
	d8255_2s.in_pa_callback().set("d8255_2", FUNC(i8255_device::pb_r));
	d8255_2s.in_pb_callback().set("d8255_2", FUNC(i8255_device::pa_r));
	d8255_2s.in_pc_callback().set(FUNC(pc88va_state::fdc_8255_c_r));
	d8255_2s.out_pc_callback().set(FUNC(pc88va_state::fdc_8255_c_w));

	PIC8259(config, m_pic1, 0);
	m_pic1->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic1->in_sp_callback().set_constant(1);
	m_pic1->read_slave_ack_callback().set(FUNC(pc88va_state::get_slave_ack));

	PIC8259(config, m_pic2, 0);
	m_pic2->out_int_callback().set(m_pic1, FUNC(pic8259_device::ir7_w));
	m_pic2->in_sp_callback().set_constant(0);

	AM9517A(config, m_dmac, MASTER_CLOCK); // ch2 is FDC, ch0/3 are "user". ch1 is unused
	m_dmac->out_hreq_callback().set(FUNC(pc88va_state::pc88va_hlda_w));
	m_dmac->out_eop_callback().set(FUNC(pc88va_state::pc88va_tc_w));
	m_dmac->in_ior_callback<2>().set(FUNC(pc88va_state::fdc_dma_r));
	m_dmac->out_iow_callback<2>().set(FUNC(pc88va_state::fdc_dma_w));
	m_dmac->in_memr_callback().set(FUNC(pc88va_state::dma_memr_cb));
	m_dmac->out_memw_callback().set(FUNC(pc88va_state::dma_memw_cb));

	UPD765A(config, m_fdc, 8000000, false, true);
	m_fdc->intrq_wr_callback().set(FUNC(pc88va_state::fdc_irq));
	m_fdc->drq_wr_callback().set(FUNC(pc88va_state::fdc_drq));
	FLOPPY_CONNECTOR(config, m_fdd[0], pc88va_floppies, "525hd", pc88va_state::floppy_formats);
	FLOPPY_CONNECTOR(config, m_fdd[1], pc88va_floppies, "525hd", pc88va_state::floppy_formats);
	SOFTWARE_LIST(config, "disk_list").set_original("pc88va");

	pit8253_device &pit8253(PIT8253(config, "pit8253", 0));
	pit8253.set_clk<0>(MASTER_CLOCK); /* general purpose timer 1 */
	pit8253.out_handler<0>().set(FUNC(pc88va_state::pc88va_pit_out0_changed));
	pit8253.set_clk<1>(MASTER_CLOCK); /* BEEP frequency setting */
	pit8253.set_clk<2>(MASTER_CLOCK); /* RS232C baud rate setting */

	ADDRESS_MAP_BANK(config, "sysbank").set_map(&pc88va_state::sysbank_map).set_options(ENDIANNESS_LITTLE, 16, 18+4, 0x40000);

	SPEAKER(config, "mono").front_center();
	ym2203_device &ym(YM2203(config, "ym", FM_CLOCK)); //unknown clock / divider
	ym.add_route(0, "mono", 0.25);
	ym.add_route(1, "mono", 0.25);
	ym.add_route(2, "mono", 0.50);
	ym.add_route(3, "mono", 0.50);
}


ROM_START( pc88va2 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x100000, "fdccpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vasubsys.rom", 0x0000, 0x2000, CRC(08962850) SHA1(a9375aa480f85e1422a0e1385acb0ea170c5c2e0) )

	ROM_REGION( 0x100000, "rom00", ROMREGION_ERASEFF ) // 0xe0000 - 0xeffff
	ROM_LOAD( "varom00_va2.rom",   0x00000, 0x80000, CRC(98c9959a) SHA1(bcaea28c58816602ca1e8290f534360f1ca03fe8) )
	ROM_LOAD( "varom08_va2.rom",   0x80000, 0x20000, CRC(eef6d4a0) SHA1(47e5f89f8b0ce18ff8d5d7b7aef8ca0a2a8e3345) )

	ROM_REGION( 0x20000, "rom10", 0 ) // 0xf0000 - 0xfffff
	ROM_LOAD( "varom1_va2.rom",    0x00000, 0x20000, CRC(7e767f00) SHA1(dd4f4521bfbb068f15ab3bcdb8d47c7d82b9d1d4) )

	/* No idea of the proper size: it has never been dumped */
	ROM_REGION( 0x2000, "audiocpu", 0)
	ROM_LOAD( "soundbios.rom", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION16_LE( 0x80000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "vafont_va2.rom", 0x00000, 0x50000, BAD_DUMP CRC(b40d34e4) SHA1(a0227d1fbc2da5db4b46d8d2c7e7a9ac2d91379f) ) // should be splitted

	ROM_REGION16_LE( 0x80000, "dictionary", 0 )
	ROM_LOAD( "vadic_va2.rom", 0x00000, 0x80000, CRC(a6108f4d) SHA1(3665db538598abb45d9dfe636423e6728a812b12) )
ROM_END

ROM_START( pc88va )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x100000, "fdccpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vasubsys.rom", 0x0000, 0x2000, CRC(08962850) SHA1(a9375aa480f85e1422a0e1385acb0ea170c5c2e0) )

	ROM_REGION( 0x100000, "rom00", ROMREGION_ERASEFF ) // 0xe0000 - 0xeffff
	ROM_LOAD( "varom00.rom", 0x00000, 0x80000, CRC(8a853b00) SHA1(1266ba969959ff25433ecc900a2caced26ef1a9e))
	ROM_LOAD( "varom08.rom", 0x80000, 0x20000, CRC(154803cc) SHA1(7e6591cd465cbb35d6d3446c5a83b46d30fafe95))

	ROM_REGION( 0x20000, "rom10", 0 ) // 0xf0000 - 0xfffff
	ROM_LOAD( "varom1.rom", 0x00000, 0x20000, CRC(0783b16a) SHA1(54536dc03238b4668c8bb76337efade001ec7826))

	/* No idea of the proper size: it has never been dumped */
	ROM_REGION( 0x2000, "audiocpu", 0)
	ROM_LOAD( "soundbios.rom", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION16_LE( 0x80000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "vafont.rom", 0x0000, 0x50000, BAD_DUMP CRC(faf7c466) SHA1(196b3d5b7407cb4f286ffe5c1e34ebb1f6905a8c)) // should be splitted

	ROM_REGION16_LE( 0x80000, "dictionary", 0 )
	ROM_LOAD( "vadic.rom",  0x0000, 0x80000, CRC(f913c605) SHA1(5ba1f3578d0aaacdaf7194a80e6d520c81ae55fb))
ROM_END




COMP( 1987, pc88va,  0,      0, pc88va, pc88va, pc88va_state, empty_init, "NEC", "PC-88VA",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1988, pc88va2, pc88va, 0, pc88va, pc88va, pc88va_state, empty_init, "NEC", "PC-88VA2", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
//COMP( 1988, pc88va3, pc88va, 0, pc88va, pc88va, pc88va_state, empty_init, "NEC", "PC-88VA3", MACHINE_NOT_WORKING )
