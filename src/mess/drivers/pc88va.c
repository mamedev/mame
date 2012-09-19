/********************************************************************************************

    PC-88VA (c) 1987 NEC

    A follow up of the regular PC-8801. It can also run PC-8801 software in compatible mode

    preliminary driver by Angelo Salese
    Special thanks to Fujix for his documentation translation help

    TODO:
    - Does this system have one or two CPUs? I'm prone to think that the V30 does all the job
      and then enters into z80 compatible mode for PC-8801 emulation.
    - What exact kind of garbage happens if you try to enable both direct and palette color
      modes to a graphic layer?
    - What is exactly supposed to be a "bus slot"?
    - fdc "intelligent mode" has 0x7f as irq vector ... 0x7f is ld a,a and it IS NOT correctly
      hooked up by the current z80 core

********************************************************************************************/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "debug/debugcpu.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/upd765.h"
#include "sound/2203intf.h"
#include "formats/basicdsk.h"

struct tsp_t
{
	UINT16 tvram_vreg_offset;
	UINT16 attr_offset;
	UINT16 spr_offset;
	UINT8 disp_on;
	UINT8 spr_on;
	UINT8 pitch;
	UINT8 line_height;
	UINT8 h_line_pos;
	UINT8 blink;
	UINT16 cur_pos_x,cur_pos_y;
	UINT8 curn;
	UINT8 curn_blink;
};


class pc88va_state : public driver_device
{
public:
	pc88va_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_palram(*this, "palram"){ }

	required_shared_ptr<UINT16> m_palram;
	UINT16 m_bank_reg;
	UINT16 m_screen_ctrl_reg;
	UINT8 m_timer3_io_reg;
	emu_timer *m_t3_mouse_timer;
	tsp_t m_tsp;
	UINT16 m_video_pri_reg[2];
	UINT8 m_backupram_wp;
	UINT8 m_cmd;
	UINT8 m_buf_size;
	UINT8 m_buf_index;
	UINT8 m_buf_ram[16];
	UINT8 m_portc_test;

	/* floppy state */
	UINT8 m_i8255_0_pc;
	UINT8 m_i8255_1_pc;
	UINT8 m_fdc_mode;
	UINT8 m_fdc_irq_opcode;
	DECLARE_READ16_MEMBER(sys_mem_r);
	DECLARE_WRITE16_MEMBER(sys_mem_w);
	DECLARE_READ8_MEMBER(idp_status_r);
	DECLARE_WRITE8_MEMBER(idp_command_w);
	DECLARE_WRITE8_MEMBER(idp_param_w);
	DECLARE_WRITE16_MEMBER(palette_ram_w);
	DECLARE_READ16_MEMBER(sys_port4_r);
	DECLARE_READ16_MEMBER(bios_bank_r);
	DECLARE_WRITE16_MEMBER(bios_bank_w);
	DECLARE_READ8_MEMBER(rom_bank_r);
	DECLARE_READ8_MEMBER(key_r);
	DECLARE_WRITE16_MEMBER(backupram_wp_1_w);
	DECLARE_WRITE16_MEMBER(backupram_wp_0_w);
	DECLARE_READ8_MEMBER(hdd_status_r);
	DECLARE_WRITE8_MEMBER(upd765_mc_w);
	DECLARE_READ8_MEMBER(upd765_tc_r);
	DECLARE_READ8_MEMBER(pc88va_fdc_r);
	DECLARE_WRITE8_MEMBER(pc88va_fdc_w);
	DECLARE_READ16_MEMBER(sysop_r);
	DECLARE_READ16_MEMBER(screen_ctrl_r);
	DECLARE_WRITE16_MEMBER(screen_ctrl_w);
	DECLARE_WRITE8_MEMBER(timer3_ctrl_reg_w);
	DECLARE_WRITE16_MEMBER(video_pri_w);
	DECLARE_READ8_MEMBER(backupram_dsw_r);
	DECLARE_WRITE8_MEMBER(sys_port1_w);
	DECLARE_WRITE8_MEMBER(fdc_irq_vector_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_pc88va(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(pc88va_vrtc_irq);
};



void pc88va_state::video_start()
{

}

static void draw_sprites(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pc88va_state *state = machine.driver_data<pc88va_state>();
	UINT16 *tvram = (UINT16 *)(*state->memregion("tvram"));
	int offs,i;

	offs = state->m_tsp.spr_offset;
	for(i=0;i<(0x100);i+=(8))
	{
		int xp,yp,sw,md,xsize,ysize,spda,fg_col,bc;
		int x_i,y_i,x_s;
		int spr_count;
		int pen;

		ysize = (tvram[(offs + i + 0) / 2] & 0xfc00) >> 10;
		sw = (tvram[(offs + i + 0) / 2] & 0x200) >> 9;
		yp = (tvram[(offs + i + 0) / 2] & 0x1ff);
		xsize = (tvram[(offs + i + 2) / 2] & 0xf800) >> 11;
		md = (tvram[(offs + i + 2) / 2] & 0x400) >> 10;
		xp = (tvram[(offs + i + 2) / 2] & 0x3ff);
		spda = (tvram[(offs + i + 4) / 2] & 0xffff);
		fg_col = (tvram[(offs + i + 6) / 2] & 0xf0) >> 4;
		bc = (tvram[(offs + i + 6) / 2] & 0x08) >> 3;

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

			for(y_i=0;y_i<ysize;y_i++)
			{
				for(x_i=0;x_i<xsize;x_i+=16)
				{
					for(x_s=0;x_s<16;x_s++)
					{
						pen = (BITSWAP16(tvram[(spda+spr_count) / 2],7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8) >> (15-x_s)) & 1;

						pen = pen & 1 ? fg_col : (bc) ? 8 : -1;

						if(pen != -1) //transparent pen
							bitmap.pix32(yp+y_i, xp+x_i+(x_s)) = machine.pens[pen];
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

			for(y_i=0;y_i<ysize;y_i++)
			{
				for(x_i=0;x_i<xsize;x_i+=2)
				{
					for(x_s=0;x_s<2;x_s++)
					{
						pen = (BITSWAP16(tvram[(spda+spr_count) / 2],7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8)) >> (16-(x_s*8)) & 0xf;

						//if(bc != -1) //transparent pen
						bitmap.pix32(yp+y_i, xp+x_i+(x_s)) = machine.pens[pen];
					}
					spr_count+=2;
				}
			}
		}
	}
}

/* TODO: this is either a result of an hand-crafted ROM or the JIS stuff is really attribute related ... */
static UINT32 calc_kanji_rom_addr(UINT8 jis1,UINT8 jis2,int x,int y)
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

static void draw_text(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pc88va_state *state = machine.driver_data<pc88va_state>();
	UINT8 *tvram = machine.root_device().memregion("tvram")->base();
	UINT8 *kanji = state->memregion("kanji")->base();
	int xi,yi;
	int x,y;
	int res_x,res_y;
	UINT16 lr_half_gfx;
	UINT8 jis1,jis2;
	UINT32 count;
	UINT32 tile_num;
	UINT16 attr;
	UINT8 attr_mode;
	UINT8 fg_col,bg_col,secret,reverse;
	//UINT8 blink,dwidc,dwid,uline,hline;
	UINT8 screen_fg_col,screen_bg_col;

	count = (tvram[state->m_tsp.tvram_vreg_offset+0] | tvram[state->m_tsp.tvram_vreg_offset+1] << 8);

	attr_mode = tvram[state->m_tsp.tvram_vreg_offset+0xa] & 0x1f;
	/* Note: bug in docs has the following two reversed */
	screen_fg_col = (tvram[state->m_tsp.tvram_vreg_offset+0xb] & 0xf0) >> 4;
	screen_bg_col = tvram[state->m_tsp.tvram_vreg_offset+0xb] & 0x0f;

	for(y=0;y<13;y++)
	{
		for(x=0;x<80;x++)
		{
			jis1 = (tvram[count+0] & 0x7f) + 0x20;
			jis2 = tvram[count+1] & 0x7f;
			lr_half_gfx = ((tvram[count+1] & 0x80) >> 7);

			tile_num = calc_kanji_rom_addr(jis1,jis2,x,y);

			attr = (tvram[count+state->m_tsp.attr_offset] & 0x00ff);

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

			for(yi=0;yi<16;yi++)
			{
				for(xi=0;xi<8;xi++)
				{
					int pen;

					res_x = x*8+xi;
					res_y = y*16+yi;

					if(!cliprect.contains(res_x, res_y))
						continue;

					pen = kanji[((yi*2)+lr_half_gfx)+tile_num] >> (7-xi) & 1;

					if(reverse)
						pen = pen & 1 ? bg_col : fg_col;
					else
						pen = pen & 1 ? fg_col : bg_col;

					if(secret) { pen = 0; } //hide text

					if(pen != -1) //transparent
						bitmap.pix32(res_y, res_x) = machine.pens[pen];
				}
			}

			count+=2;
			count&=0xffff;
		}
	}
}

UINT32 pc88va_state::screen_update_pc88va(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 pri,cur_pri_lv;
	UINT32 screen_pri;
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
					case 0: draw_text(machine(),bitmap,cliprect); break;
					case 1: if(m_tsp.spr_on) { draw_sprites(machine(),bitmap,cliprect); } break;
					case 2: /* A = graphic 0 */ break;
					case 3: /* B = graphic 1 */ break;
				}
			}
		}
	}

	return 0;
}

READ16_MEMBER(pc88va_state::sys_mem_r)
{
	switch((m_bank_reg & 0xf00) >> 8)
	{
		case 0: // select bus slot
			return 0xffff;
		case 1: // TVRAM
		{
			UINT16 *tvram = (UINT16 *)(*machine().root_device().memregion("tvram"));

			if(((offset*2) & 0x30000) == 0)
				return tvram[offset];

			return 0xffff;
		}
		case 4:
		{
			UINT16 *gvram = (UINT16 *)(*machine().root_device().memregion("gvram"));

			return gvram[offset];
		}
		case 8: // kanji ROM
		case 9:
		{
			UINT16 *knj_ram = (UINT16 *)(*machine().root_device().memregion("kanji"));
			UINT32 knj_offset;

			knj_offset = (offset + (((m_bank_reg & 0x100) >> 8)*0x20000));

			/* 0x00000 - 0x3ffff Kanji ROM 1*/
			/* 0x40000 - 0x4ffff Kanji ROM 2*/
			/* 0x50000 - 0x53fff Backup RAM */
			/* anything else is a NOP (I presume?) */

			return knj_ram[knj_offset];
		}
		break;
		case 0xc: // Dictionary ROM
		case 0xd:
		{
			UINT16 *dic_rom = (UINT16 *)(*machine().root_device().memregion("dictionary"));
			UINT32 dic_offset;

			dic_offset = (offset + (((m_bank_reg & 0x100) >> 8)*0x20000));

			return dic_rom[dic_offset];
		}
	}

	return 0xffff;
}

WRITE16_MEMBER(pc88va_state::sys_mem_w)
{
	switch((m_bank_reg & 0xf00) >> 8)
	{
		case 0: // select bus slot
			break;
		case 1: // TVRAM
		{
			UINT16 *tvram = (UINT16 *)(*machine().root_device().memregion("tvram"));

			if(((offset*2) & 0x30000) == 0)
				COMBINE_DATA(&tvram[offset]);
		}
		break;
		case 4: // TVRAM
		{
			UINT16 *gvram = (UINT16 *)(*machine().root_device().memregion("gvram"));

			COMBINE_DATA(&gvram[offset]);
		}
		break;
		case 8: // kanji ROM, backup RAM at 0xb0000 - 0xb3fff
		case 9:
		{
			UINT16 *knj_ram = (UINT16 *)(*machine().root_device().memregion("kanji"));
			UINT32 knj_offset;

			knj_offset = ((offset) + (((m_bank_reg & 0x100) >> 8)*0x20000));

			if(knj_offset >= 0x50000/2 && knj_offset <= 0x53fff/2) // TODO: there's an area that can be write protected
			{
				COMBINE_DATA(&knj_ram[knj_offset]);
				machine().gfx[0]->mark_dirty((knj_offset * 2) / 8);
				machine().gfx[1]->mark_dirty((knj_offset * 2) / 32);
			}
		}
		break;
		case 0xc: // Dictionary ROM
		case 0xd:
		{
			// NOP?
		}
		break;
	}
}

static ADDRESS_MAP_START( pc88va_map, AS_PROGRAM, 16, pc88va_state )
	AM_RANGE(0x00000, 0x7ffff) AM_RAM
//  AM_RANGE(0x80000, 0x9ffff) AM_RAM // EMM
	AM_RANGE(0xa0000, 0xdffff) AM_READWRITE(sys_mem_r,sys_mem_w)
	AM_RANGE(0xe0000, 0xeffff) AM_ROMBANK("rom00_bank")
	AM_RANGE(0xf0000, 0xfffff) AM_ROMBANK("rom10_bank")
ADDRESS_MAP_END

READ8_MEMBER(pc88va_state::idp_status_r)
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

WRITE8_MEMBER(pc88va_state::idp_command_w)
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
		default:   m_cmd = 0x00; printf("PC=%05x: Unknown IDP %02x cmd set\n",space.device().safe_pc(),data); break;
	}
}

static void tsp_sprite_enable(running_machine &machine, UINT32 spr_offset, UINT8 sw_bit)
{
	address_space &space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	space.write_word(spr_offset, space.read_word(spr_offset) & ~0x200);
	space.write_word(spr_offset, space.read_word(spr_offset) | (sw_bit & 0x200));
}

/* TODO: very preliminary, needs something showable first */
static void execute_sync_cmd(running_machine &machine)
{
	pc88va_state *state = machine.driver_data<pc88va_state>();
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
	UINT16 x_vis_area,y_vis_area;

	//printf("V blank start: %d\n",(sync_cmd[0x8]));
	//printf("V border start: %d\n",(sync_cmd[0x9]));
	//printf("V Visible Area: %d\n",(sync_cmd[0xa])|((sync_cmd[0xb] & 0x40)<<2));
	//printf("V border end: %d\n",(sync_cmd[0xc]));
	//printf("V blank end: %d\n",(sync_cmd[0xd]));

	x_vis_area = state->m_buf_ram[4] * 4;
	y_vis_area = (state->m_buf_ram[0xa])|((state->m_buf_ram[0xb] & 0x40)<<2);

	visarea.set(0, x_vis_area - 1, 0, y_vis_area - 1);

	//if(y_vis_area == 400)
	//  refresh = HZ_TO_ATTOSECONDS(24800) * x_vis_area * y_vis_area; //24.8 KHz
	//else
	//  refresh = HZ_TO_ATTOSECONDS(15730) * x_vis_area * y_vis_area; //15.73 KHz

	refresh = HZ_TO_ATTOSECONDS(60);

	machine.primary_screen->configure(640, 480, visarea, refresh);
}

static void execute_dspon_cmd(running_machine &machine)
{
	pc88va_state *state = machine.driver_data<pc88va_state>();
	/*
    [0] text table offset (hi word)
    [1] unknown
    [2] unknown
    */
	state->m_tsp.tvram_vreg_offset = state->m_buf_ram[0] << 8;
	state->m_tsp.disp_on = 1;
}

static void execute_dspdef_cmd(running_machine &machine)
{
	pc88va_state *state = machine.driver_data<pc88va_state>();
	/*
    [0] attr offset (lo word)
    [1] attr offset (hi word)
    [2] pitch (character code interval x 16, i.e. 0x20 = 2 bytes
    [3] line height
    [4] hline vertical position
    [5] blink number
    */
	state->m_tsp.attr_offset = state->m_buf_ram[0] | state->m_buf_ram[1] << 8;
	state->m_tsp.pitch = (state->m_buf_ram[2] & 0xf0) >> 4;
	state->m_tsp.line_height = state->m_buf_ram[3] + 1;
	state->m_tsp.h_line_pos = state->m_buf_ram[4];
	state->m_tsp.blink = (state->m_buf_ram[5] & 0xf8) >> 3;
}

static void execute_curdef_cmd(running_machine &machine)
{
	pc88va_state *state = machine.driver_data<pc88va_state>();
	/*
    xxxx x--- [0] Sprite Cursor number (sprite RAM entry)
    ---- --x- [0] show cursor bit (actively modifies the spriteram entry)
    ---- ---x [0] Blink Enable
    */

	/* TODO: needs basic sprite emulation */
	state->m_tsp.curn = (state->m_buf_ram[0] & 0xf8);
	state->m_tsp.curn_blink = (state->m_buf_ram[0] & 1);

	tsp_sprite_enable(machine, 0xa0000 + state->m_tsp.spr_offset + state->m_tsp.curn, (state->m_buf_ram[0] & 2) << 8);
}

static void execute_actscr_cmd(running_machine &machine)
{
	//pc88va_state *state = machine.driver_data<pc88va_state>();
	/*
    This command assigns a strip where the cursor is located.
    xxxx xxxx [0] strip ID * 32 (???)
    */

	/* TODO: no idea about this command */
	//printf("ACTSCR: %02x\n",state->m_buf_ram[0]);
}

static void execute_curs_cmd(running_machine &machine)
{
	pc88va_state *state = machine.driver_data<pc88va_state>();
	/*
    [0] Cursor Position Y (lo word)
    [1] Cursor Position Y (hi word)
    [2] Cursor Position X (lo word)
    [3] Cursor Position X (hi word)
    */

	state->m_tsp.cur_pos_y = state->m_buf_ram[0] | state->m_buf_ram[1] << 8;
	state->m_tsp.cur_pos_x = state->m_buf_ram[2] | state->m_buf_ram[3] << 8;
}

static void execute_emul_cmd(running_machine &machine)
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

static void execute_spron_cmd(running_machine &machine)
{
	pc88va_state *state = machine.driver_data<pc88va_state>();
	/*
    [0] Sprite Table Offset (hi word)
    [1] (unknown / reserved)
    xxxx x--- [2] HSPN: Maximum number of sprites in one raster (num + 1) for Sprite Over
    ---- --x- [2] MG: all sprites are 2x zoomed vertically when 1
    ---- ---x [2] GR: 1 to enable the group collision detection
    */
	state->m_tsp.spr_offset = state->m_buf_ram[0] << 8;
	state->m_tsp.spr_on = 1;
	printf("SPR TABLE %02x %02x %02x\n",state->m_buf_ram[0],state->m_buf_ram[1],state->m_buf_ram[2]);
}

static void execute_sprsw_cmd(running_machine &machine)
{
	pc88va_state *state = machine.driver_data<pc88va_state>();
	/*
    Toggle an individual sprite in the sprite ram entry
    [0] xxxx x--- target sprite number
    [0] ---- --x- sprite off/on switch
    */

	tsp_sprite_enable(machine, 0xa0000 + state->m_tsp.spr_offset + (state->m_buf_ram[0] & 0xf8), (state->m_buf_ram[0] & 2) << 8);
}

WRITE8_MEMBER(pc88va_state::idp_param_w)
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
			case SYNC:		execute_sync_cmd(machine());	break;
			case DSPON: 	execute_dspon_cmd(machine());	break;
			case DSPDEF:	execute_dspdef_cmd(machine()); break;
			case CURDEF:	execute_curdef_cmd(machine()); break;
			case ACTSCR:	execute_actscr_cmd(machine()); break;
			case CURS:		execute_curs_cmd(machine());   break;
			case EMUL:		execute_emul_cmd(machine());   break;
			case SPRON:		execute_spron_cmd(machine());  break;
			case SPRSW:		execute_sprsw_cmd(machine());	break;

			default:
				//printf("%02x\n",data);
				break;
		}
	}
}

WRITE16_MEMBER(pc88va_state::palette_ram_w)
{
	int r,g,b;
	COMBINE_DATA(&m_palram[offset]);

	b = (m_palram[offset] & 0x001e) >> 1;
	r = (m_palram[offset] & 0x03c0) >> 6;
	g = (m_palram[offset] & 0x7800) >> 11;

	palette_set_color_rgb(machine(),offset,pal4bit(r),pal4bit(g),pal4bit(b));
}

READ16_MEMBER(pc88va_state::sys_port4_r)
{
	UINT8 vrtc,sw1;
	vrtc = (machine().primary_screen->vpos() < 200) ? 0x20 : 0x00; // vblank

	sw1 = (ioport("DSW")->read() & 1) ? 2 : 0;

	return vrtc | sw1 | 0xc0;
}

READ16_MEMBER(pc88va_state::bios_bank_r)
{
	return m_bank_reg;
}

WRITE16_MEMBER(pc88va_state::bios_bank_w)
{
	/*
    -x-- ---- ---- ---- SMM (compatibility mode)
    ---x ---- ---- ---- GMSP (VRAM drawing Mode)
    ---- xxxx ---- ---- SMBC (0xa0000 - 0xdffff RAM bank)
    ---- ---- xxxx ---- RBC1 (0xf0000 - 0xfffff ROM bank)
    ---- ---- ---- xxxx RBC0 (0xe0000 - 0xeffff ROM bank)
    */
	if ((mem_mask&0xffff) == 0xffff)
		m_bank_reg = data;
	else if ((mem_mask & 0xffff) == 0xff00)
		m_bank_reg = (data & 0xff00) | (m_bank_reg & 0x00ff);
	else if ((mem_mask & 0xffff) == 0x00ff)
		m_bank_reg = (data & 0x00ff) | (m_bank_reg & 0xff00);


	/* RBC1 */
	{
		UINT8 *ROM10 = memregion("rom10")->base();

		if((m_bank_reg & 0xe0) == 0x00)
			membank("rom10_bank")->set_base(&ROM10[(m_bank_reg & 0x10) ? 0x10000 : 0x00000]);
	}

	/* RBC0 */
	{
		UINT8 *ROM00 = memregion("rom00")->base();

		membank("rom00_bank")->set_base(&ROM00[(m_bank_reg & 0xf)*0x10000]); // TODO: docs says that only 0 - 5 are used, dunno why ...
	}
}

READ8_MEMBER(pc88va_state::rom_bank_r)
{
	return 0xff; // bit 7 low is va91 rom bank status
}

READ8_MEMBER(pc88va_state::key_r)
{
	// note row C bit 2 does something at POST ... some kind of test mode?
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3",
	                                        "KEY4", "KEY5", "KEY6", "KEY7",
	                                        "KEY8", "KEY9", "KEYA", "KEYB",
	                                        "KEYC", "KEYD", "KEYE", "KEYF" };

	return ioport(keynames[offset])->read();
}

WRITE16_MEMBER(pc88va_state::backupram_wp_1_w)
{
	m_backupram_wp = 1;
}

WRITE16_MEMBER(pc88va_state::backupram_wp_0_w)
{
	m_backupram_wp = 0;
}

READ8_MEMBER(pc88va_state::hdd_status_r)
{
	return 0x20;
}

WRITE8_MEMBER(pc88va_state::upd765_mc_w)
{
	floppy_mon_w(floppy_get_device(machine(), 0), (data & 1) ? CLEAR_LINE : ASSERT_LINE);
	floppy_mon_w(floppy_get_device(machine(), 1), (data & 2) ? CLEAR_LINE : ASSERT_LINE);
	floppy_drive_set_ready_state(floppy_get_device(machine(), 0), (data & 1), 0);
	floppy_drive_set_ready_state(floppy_get_device(machine(), 1), (data & 2), 0);
}

static TIMER_CALLBACK( pc8801fd_upd765_tc_to_zero )
{
//  pc88va_state *state = machine.driver_data<pc88va_state>();

	upd765_tc_w(machine.device("upd765"), 0);
}

READ8_MEMBER(pc88va_state::upd765_tc_r)
{

	upd765_tc_w(machine().device("upd765"), 1);
	machine().scheduler().timer_set(attotime::from_usec(500), FUNC(pc8801fd_upd765_tc_to_zero));
	return 0;
}

READ8_MEMBER(pc88va_state::pc88va_fdc_r)
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
		case 0x08: return upd765_status_r(machine().device("upd765"), space, 0);
		case 0x0a: return upd765_data_r(machine().device("upd765"), space, 0);
	}

	return 0xff;
}

WRITE8_MEMBER(pc88va_state::pc88va_fdc_w)
{
	printf("%08x %02x\n",offset,data);
	switch(offset*2)
	{
		/*
        ---- ---x MODE: FDC op mode (0) Intelligent (1) DMA
        */
		case 0x00: // FDC mode register
			m_fdc_mode = data & 1;
			machine().device("fdccpu")->execute().set_input_line(INPUT_LINE_HALT, (m_fdc_mode) ? ASSERT_LINE : CLEAR_LINE);
			break;
		/*
        --x- ---- CLK: FDC clock selection (0) 4.8MHz (1) 8 MHz
        ---x ---- DS1: Prohibition of the drive selection of FDC (0) Permission (1) Prohibition
        ---- xx-- TD1/TD0: Drive 1/0 track density (0) 48 TPI (1) 96 TPI
        ---- --xx RV1/RV0: Drive 1/0 mode selection (0) 2D and 2DD mode (1) 2HD mode
        */
		case 0x02: // FDC control port 0
			break;
		/*
        ---- x--- PCM: ?
        ---- --xx M1/M0: Drive 1/0 motor control (0) Motor OFF (1) Motor ON
        */
		case 0x04: upd765_mc_w(space,0,data); break;
		/*
        x--- ---- FDCRST: FDC Reset
        -xx- ---- FDCFRY FRYCEN: FDC force ready control
        ---x ---- DMAE: DMA Enable (0) Prohibit DMA (1) Enable DMA
        ---- -x-- XTMASK: FDC timer IRQ mask (0) Disable (1) Enable
        ---- ---x TTRG: FDC timer trigger (0) FDC timer clearing (1) FDC timer start
        */
		case 0x06:
			printf("%02x\n",data);
			break; // FDC control port 2
		case 0x08: break; // UPD765 status
		case 0x0a: upd765_data_w(machine().device("upd765"), space, 0,data); break;
	}
}


READ16_MEMBER(pc88va_state::sysop_r)
{
	UINT8 sys_op;

	sys_op = ioport("SYSOP_SW")->read() & 3;

	return 0xfffc | sys_op; // docs says all the other bits are high
}

READ16_MEMBER(pc88va_state::screen_ctrl_r)
{
	return m_screen_ctrl_reg;
}

WRITE16_MEMBER(pc88va_state::screen_ctrl_w)
{
	m_screen_ctrl_reg = data;
}

static TIMER_CALLBACK( t3_mouse_callback )
{
	pc88va_state *state = machine.driver_data<pc88va_state>();
	if(state->m_timer3_io_reg & 0x80)
	{
		pic8259_ir5_w(machine.device("pic8259_slave"), 1);
		state->m_t3_mouse_timer->adjust(attotime::from_hz(120 >> (state->m_timer3_io_reg & 3)));
	}
}

WRITE8_MEMBER(pc88va_state::timer3_ctrl_reg_w)
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
		pic8259_ir5_w(machine().device("pic8259_slave"), 0);
		m_t3_mouse_timer->adjust(attotime::never);
	}
}

WRITE16_MEMBER(pc88va_state::video_pri_w)
{
	COMBINE_DATA(&m_video_pri_reg[offset]);
}

READ8_MEMBER(pc88va_state::backupram_dsw_r)
{
	UINT16 *knj_ram = (UINT16 *)(*machine().root_device().memregion("kanji"));

	if(offset == 0)
		return knj_ram[(0x50000 + 0x1fc2) / 2] & 0xff;

	return knj_ram[(0x50000 + 0x1fc6) / 2] & 0xff;
}

WRITE8_MEMBER(pc88va_state::sys_port1_w)
{
	// ...
}

static ADDRESS_MAP_START( pc88va_io_map, AS_IO, 16, pc88va_state )
	AM_RANGE(0x0000, 0x000f) AM_READ8(key_r,0xffff) // Keyboard ROW reading
//  AM_RANGE(0x0010, 0x0010) Printer / Calendar Clock Interface
	AM_RANGE(0x0020, 0x0021) AM_NOP // RS-232C
	AM_RANGE(0x0030, 0x0031) AM_READWRITE8(backupram_dsw_r,sys_port1_w,0xffff) // 0x30 (R) DSW1 (W) Text Control Port 0 / 0x31 (R) DSW2 (W) System Port 1
//  AM_RANGE(0x0032, 0x0032) (R) ? (W) System Port 2
//  AM_RANGE(0x0034, 0x0034) GVRAM Control Port 1
//  AM_RANGE(0x0035, 0x0035) GVRAM Control Port 2
	AM_RANGE(0x0040, 0x0041) AM_READ(sys_port4_r) // (R) System Port 4 (W) System port 3 (strobe port)
	AM_RANGE(0x0044, 0x0045) AM_MIRROR(0x0002) AM_DEVREADWRITE8_LEGACY("ym", ym2203_r,ym2203_w,0xffff)
//  AM_RANGE(0x005c, 0x005c) (R) GVRAM status
//  AM_RANGE(0x005c, 0x005f) (W) GVRAM selection
//  AM_RANGE(0x0070, 0x0070) ? (*)
//  AM_RANGE(0x0071, 0x0071) Expansion ROM select (*)
//  AM_RANGE(0x0078, 0x0078) Memory offset increment (*)
//  AM_RANGE(0x0080, 0x0081) HDD related
	AM_RANGE(0x0082, 0x0083) AM_READ8(hdd_status_r,0x00ff)// HDD control, byte access 7-0
//  AM_RANGE(0x00bc, 0x00bf) d8255 1
//  AM_RANGE(0x00e2, 0x00e3) Expansion RAM selection (*)
//  AM_RANGE(0x00e4, 0x00e4) 8214 IRQ control (*)
//  AM_RANGE(0x00e6, 0x00e6) 8214 IRQ mask (*)
//  AM_RANGE(0x00e8, 0x00e9) ? (*)
//  AM_RANGE(0x00ec, 0x00ed) ? (*)
	AM_RANGE(0x00fc, 0x00ff) AM_DEVREADWRITE8("d8255_2", i8255_device, read, write, 0xffff) // d8255 2, FDD

	AM_RANGE(0x0100, 0x0101) AM_READWRITE(screen_ctrl_r,screen_ctrl_w) // Screen Control Register
//  AM_RANGE(0x0102, 0x0103) Graphic Screen Control Register
	AM_RANGE(0x0106, 0x0109) AM_WRITE(video_pri_w) // Palette Control Register (priority) / Direct Color Control Register (priority)
//  AM_RANGE(0x010a, 0x010b) Picture Mask Mode Register
//  AM_RANGE(0x010c, 0x010d) Color Palette Mode Register
//  AM_RANGE(0x010e, 0x010f) Backdrop Color Register
//  AM_RANGE(0x0110, 0x0111) Color Code/Plain Mask Register
//  AM_RANGE(0x0124, 0x0125) ? (related to Transparent Color of Graphic Screen 0)
//  AM_RANGE(0x0126, 0x0127) ? (related to Transparent Color of Graphic Screen 1)
//  AM_RANGE(0x012e, 0x012f) ? (related to Transparent Color of Text/Sprite)
//  AM_RANGE(0x0130, 0x0137) Picture Mask Parameter
	AM_RANGE(0x0142, 0x0143) AM_READWRITE8(idp_status_r,idp_command_w,0x00ff) //Text Controller (IDP) - (R) Status (W) command
	AM_RANGE(0x0146, 0x0147) AM_WRITE8(idp_param_w,0x00ff) //Text Controller (IDP) - (R/W) Parameter
//  AM_RANGE(0x0148, 0x0149) Text control port 1
//  AM_RANGE(0x014c, 0x014f) ? CG Port
	AM_RANGE(0x0150, 0x0151) AM_READ(sysop_r) // System Operational Mode
	AM_RANGE(0x0152, 0x0153) AM_READWRITE(bios_bank_r,bios_bank_w) // Memory Map Register
//  AM_RANGE(0x0154, 0x0155) Refresh Register (wait states)
	AM_RANGE(0x0156, 0x0157) AM_READ8(rom_bank_r,0x00ff) // ROM bank status
//  AM_RANGE(0x0158, 0x0159) Interruption Mode Modification
//  AM_RANGE(0x015c, 0x015f) NMI mask port (strobe port)
//  AM_RANGE(0x0160, 0x016f) DMA Controller
	AM_RANGE(0x0184, 0x0187) AM_DEVREADWRITE8_LEGACY("pic8259_slave", pic8259_r, pic8259_w, 0x00ff)
	AM_RANGE(0x0188, 0x018b) AM_DEVREADWRITE8_LEGACY("pic8259_master", pic8259_r, pic8259_w, 0x00ff) // ICU, also controls 8214 emulation
//  AM_RANGE(0x0190, 0x0191) System Port 5
//  AM_RANGE(0x0196, 0x0197) Keyboard sub CPU command port
	AM_RANGE(0x0198, 0x0199) AM_WRITE(backupram_wp_1_w) //Backup RAM write inhibit
	AM_RANGE(0x019a, 0x019b) AM_WRITE(backupram_wp_0_w) //Backup RAM write permission
	AM_RANGE(0x01a0, 0x01a7) AM_DEVREADWRITE8_LEGACY("pit8253", pit8253_r, pit8253_w, 0x00ff)// vTCU (timer counter unit)
	AM_RANGE(0x01a8, 0x01a9) AM_WRITE8(timer3_ctrl_reg_w,0x00ff) // General-purpose timer 3 control port
	AM_RANGE(0x01b0, 0x01bb) AM_READWRITE8(pc88va_fdc_r,pc88va_fdc_w,0x00ff)// FDC related (765)
//  AM_RANGE(0x01c0, 0x01c1) ?
	AM_RANGE(0x01c6, 0x01c7) AM_WRITENOP // ???
	AM_RANGE(0x01c8, 0x01cf) AM_DEVREADWRITE8("d8255_3", i8255_device, read, write,0xff00) //i8255 3 (byte access)
//  AM_RANGE(0x01d0, 0x01d1) Expansion RAM bank selection
	AM_RANGE(0x0200, 0x021f) AM_RAM // Frame buffer 0 control parameter
	AM_RANGE(0x0220, 0x023f) AM_RAM // Frame buffer 1 control parameter
	AM_RANGE(0x0240, 0x025f) AM_RAM // Frame buffer 2 control parameter
	AM_RANGE(0x0260, 0x027f) AM_RAM // Frame buffer 3 control parameter
	AM_RANGE(0x0300, 0x033f) AM_RAM_WRITE(palette_ram_w) AM_SHARE("palram") // Palette RAM (xBBBBxRRRRxGGGG format)

//  AM_RANGE(0x0500, 0x05ff) GVRAM
//  AM_RANGE(0x1000, 0xfeff) user area (???)
	AM_RANGE(0xff00, 0xffff) AM_NOP // CPU internal use
ADDRESS_MAP_END
// (*) are specific N88 V1 / V2 ports

/* FDC subsytem CPU */
static ADDRESS_MAP_START( pc88va_z80_map, AS_PROGRAM, 8, pc88va_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_RAM
ADDRESS_MAP_END

WRITE8_MEMBER(pc88va_state::fdc_irq_vector_w)
{
	m_fdc_irq_opcode = data;
}

static ADDRESS_MAP_START( pc88va_z80_io_map, AS_IO, 8, pc88va_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf0, 0xf0) AM_WRITE(fdc_irq_vector_w) // Interrupt Opcode Port
//  AM_RANGE(0xf4, 0xf4) // Drive Control Port
	AM_RANGE(0xf8, 0xf8) AM_READWRITE(upd765_tc_r,upd765_mc_w) // (R) Terminal Count Port (W) Motor Control Port
	AM_RANGE(0xfa, 0xfa) AM_DEVREAD_LEGACY("upd765", upd765_status_r )
	AM_RANGE(0xfb, 0xfb) AM_DEVREADWRITE_LEGACY("upd765", upd765_data_r, upd765_data_w )
	AM_RANGE(0xfc, 0xff) AM_DEVREADWRITE("d8255_2s", i8255_device, read, write)
ADDRESS_MAP_END


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
	PORT_DIPSETTING(    0x02, "N88 V1 Mode" )
	PORT_DIPSETTING(    0x01, "N88 V2 Mode" )
//  PORT_DIPSETTING(    0x03, "???" )
INPUT_PORTS_END

static const gfx_layout pc88va_chars_8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout pc88va_chars_16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	16*16
};

/* decoded for debugging purpose, this will be nuked in the end... */
static GFXDECODE_START( pc88va )
	GFXDECODE_ENTRY( "kanji",   0x00000, pc88va_chars_8x8,    0, 1 )
	GFXDECODE_ENTRY( "kanji",   0x00000, pc88va_chars_16x16,  0, 1 )
GFXDECODE_END

static READ8_DEVICE_HANDLER( cpu_8255_c_r )
{
	pc88va_state *state = device->machine().driver_data<pc88va_state>();

	return state->m_i8255_1_pc >> 4;
}

static WRITE8_DEVICE_HANDLER( cpu_8255_c_w )
{
	pc88va_state *state = device->machine().driver_data<pc88va_state>();

	state->m_i8255_0_pc = data;
}

static I8255A_INTERFACE( master_fdd_intf )
{
	DEVCB_DEVICE_MEMBER("d8255_2s", i8255_device, pb_r),	// Port A read
	DEVCB_NULL,							// Port A write
	DEVCB_DEVICE_MEMBER("d8255_2s", i8255_device, pa_r), // Port B read
	DEVCB_NULL,							// Port B write
	DEVCB_HANDLER(cpu_8255_c_r),		// Port C read
	DEVCB_HANDLER(cpu_8255_c_w)			// Port C write
};

static READ8_DEVICE_HANDLER( fdc_8255_c_r )
{
	pc88va_state *state = device->machine().driver_data<pc88va_state>();

	return state->m_i8255_0_pc >> 4;
}

static WRITE8_DEVICE_HANDLER( fdc_8255_c_w )
{
	pc88va_state *state = device->machine().driver_data<pc88va_state>();

	state->m_i8255_1_pc = data;
}

static I8255A_INTERFACE( slave_fdd_intf )
{
	DEVCB_DEVICE_MEMBER("d8255_2", i8255_device, pb_r),	// Port A read
	DEVCB_NULL,							// Port A write
	DEVCB_DEVICE_MEMBER("d8255_2", i8255_device, pa_r), // Port B read
	DEVCB_NULL,							// Port B write
	DEVCB_HANDLER(fdc_8255_c_r),		// Port C read
	DEVCB_HANDLER(fdc_8255_c_w)			// Port C write
};

static READ8_DEVICE_HANDLER( r232_ctrl_porta_r )
{
	UINT8 sw5, sw4, sw3, sw2,speed_sw;

	speed_sw = (device->machine().root_device().ioport("SPEED_SW")->read() & 1) ? 0x20 : 0x00;
	sw5 = (device->machine().root_device().ioport("DSW")->read() & 0x10);
	sw4 = (device->machine().root_device().ioport("DSW")->read() & 0x08);
	sw3 = (device->machine().root_device().ioport("DSW")->read() & 0x04);
	sw2 = (device->machine().root_device().ioport("DSW")->read() & 0x02);

	return 0xc1 | sw5 | sw4 | sw3 | sw2 | speed_sw;
}

static READ8_DEVICE_HANDLER( r232_ctrl_portb_r )
{
	UINT8 xsw1;

	xsw1 = (device->machine().root_device().ioport("DSW")->read() & 1) ? 0 : 8;

	return 0xf7 | xsw1;
}

static READ8_DEVICE_HANDLER( r232_ctrl_portc_r )
{
	return 0xff;
}

static WRITE8_DEVICE_HANDLER( r232_ctrl_porta_w )
{
	// ...
}

static WRITE8_DEVICE_HANDLER( r232_ctrl_portb_w )
{
	// ...
}

static WRITE8_DEVICE_HANDLER( r232_ctrl_portc_w )
{
	// ...
}

static I8255_INTERFACE( r232c_ctrl_intf )
{
	DEVCB_HANDLER(r232_ctrl_porta_r),						/* Port A read */
	DEVCB_HANDLER(r232_ctrl_porta_w),						/* Port A write */
	DEVCB_HANDLER(r232_ctrl_portb_r),						/* Port B read */
	DEVCB_HANDLER(r232_ctrl_portb_w),						/* Port B write */
	DEVCB_HANDLER(r232_ctrl_portc_r),						/* Port C read */
	DEVCB_HANDLER(r232_ctrl_portc_w)						/* Port C write */
};

static IRQ_CALLBACK(pc88va_irq_callback)
{
	return pic8259_acknowledge( device->machine().device( "pic8259_master" ) );
}

static WRITE_LINE_DEVICE_HANDLER( pc88va_pic_irq )
{
	device->machine().device("maincpu")->execute().set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
//  logerror("PIC#1: set IRQ line to %i\n",interrupt);
}

static READ8_DEVICE_HANDLER( get_slave_ack )
{
	if (offset==7) { // IRQ = 7
		return pic8259_acknowledge(device->machine().device( "pic8259_slave"));
	}
	return 0x00;
}

static const struct pic8259_interface pc88va_pic8259_master_config =
{
	DEVCB_LINE(pc88va_pic_irq),
	DEVCB_LINE_VCC,
	DEVCB_HANDLER(get_slave_ack)
};

static const struct pic8259_interface pc88va_pic8259_slave_config =
{
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir7_w),
	DEVCB_LINE_GND,
	DEVCB_NULL
};

void pc88va_state::machine_start()
{
	machine().device("maincpu")->execute().set_irq_acknowledge_callback(pc88va_irq_callback);

	m_t3_mouse_timer = machine().scheduler().timer_alloc(FUNC(t3_mouse_callback));
	m_t3_mouse_timer->adjust(attotime::never);
}

void pc88va_state::machine_reset()
{
	UINT8 *ROM00 = machine().root_device().memregion("rom00")->base();
	UINT8 *ROM10 = memregion("rom10")->base();

	membank("rom10_bank")->set_base(&ROM10[0x00000]);
	membank("rom00_bank")->set_base(&ROM00[0x00000]);

	m_bank_reg = 0x4100;
	m_backupram_wp = 1;

	/* default palette */
	{
		UINT8 i;
		for(i=0;i<32;i++)
			palette_set_color_rgb(machine(),i,pal1bit((i & 2) >> 1),pal1bit((i & 4) >> 2),pal1bit(i & 1));
	}

	m_tsp.tvram_vreg_offset = 0;

	m_fdc_mode = 0;
	m_fdc_irq_opcode = 0x00; //0x7f ld a,a !

	machine().device("fdccpu")->execute().set_input_line_vector(0, 0);
}

INTERRUPT_GEN_MEMBER(pc88va_state::pc88va_vrtc_irq)
{
	pic8259_ir2_w(machine().device("pic8259_master"), 1);
}

/* Not sure if parameters are correct for pc88va (copied from x68k) */
static LEGACY_FLOPPY_OPTIONS_START( pc88va )
LEGACY_FLOPPY_OPTION( img2d, "xdf,hdm,2hd", "XDF disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
					 HEADS([2])
					 TRACKS([77])
					 SECTORS([8])
					 SECTOR_LENGTH([1024])
					 FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface pc88va_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(pc88va),
	"floppy_5_25",
	NULL
};


static WRITE_LINE_DEVICE_HANDLER( pc88va_pit_out0_changed )
{
	pic8259_ir0_w(device->machine().device("pic8259_master"), 1);
}

static const struct pit8253_config pc88va_pit8253_config =
{
	{
		{
			/* general purpose timer 1 */
			8000000,
			DEVCB_NULL,
			DEVCB_LINE(pc88va_pit_out0_changed)
		},
		{
			/* BEEP frequency setting */
			8000000,
			DEVCB_NULL,
			DEVCB_NULL
		},
		{
			/* RS232C baud rate setting  */
			8000000,
			DEVCB_NULL,
			DEVCB_NULL
		}
	}
};


static WRITE_LINE_DEVICE_HANDLER(pc88va_upd765_interrupt)
{
	pc88va_state *drvstate = device->machine().driver_data<pc88va_state>();
	if(drvstate->m_fdc_mode)
		pic8259_ir3_w(device->machine().device( "pic8259_slave"), state);
	else
		device->machine().device("fdccpu")->execute().set_input_line(0, HOLD_LINE);
};

static const struct upd765_interface pc88va_upd765_interface =
{
	DEVCB_LINE(pc88va_upd765_interrupt),
	DEVCB_NULL, //DRQ, TODO
	NULL,
	UPD765_RDY_PIN_CONNECTED,
	{FLOPPY_0, FLOPPY_1, NULL, NULL}
};

static const ym2203_interface pc88va_ym2203_intf =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_NULL,
		DEVCB_NULL,
		DEVCB_NULL,
		DEVCB_NULL
	},
	DEVCB_NULL
};

static MACHINE_CONFIG_START( pc88va, pc88va_state )

	MCFG_CPU_ADD("maincpu", V30, 8000000)        /* 8 MHz */
	MCFG_CPU_PROGRAM_MAP(pc88va_map)
	MCFG_CPU_IO_MAP(pc88va_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pc88va_state, pc88va_vrtc_irq)

	MCFG_CPU_ADD("fdccpu", Z80, 8000000)        /* 8 MHz */
	MCFG_CPU_PROGRAM_MAP(pc88va_z80_map)
	MCFG_CPU_IO_MAP(pc88va_z80_io_map)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(pc88va_state, screen_update_pc88va)

	MCFG_PALETTE_LENGTH(32)
//  MCFG_PALETTE_INIT_OVERRIDE(pc88va_state, pc8801 )
	MCFG_GFXDECODE( pc88va )



	MCFG_I8255_ADD( "d8255_2", master_fdd_intf )
	MCFG_I8255_ADD( "d8255_3", r232c_ctrl_intf )

	MCFG_I8255_ADD( "d8255_2s", slave_fdd_intf )

	MCFG_PIC8259_ADD( "pic8259_master", pc88va_pic8259_master_config )
	MCFG_PIC8259_ADD( "pic8259_slave", pc88va_pic8259_slave_config )

	MCFG_UPD765A_ADD("upd765", pc88va_upd765_interface)
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(pc88va_floppy_interface)
	MCFG_SOFTWARE_LIST_ADD("disk_list","pc88va")

    MCFG_PIT8253_ADD("pit8253",pc88va_pit8253_config)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ym", YM2203, 3993600) //unknown clock / divider
	MCFG_SOUND_CONFIG(pc88va_ym2203_intf)
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 0.25)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)
MACHINE_CONFIG_END


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

	ROM_REGION( 0x80000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "vafont_va2.rom", 0x00000, 0x50000, BAD_DUMP CRC(b40d34e4) SHA1(a0227d1fbc2da5db4b46d8d2c7e7a9ac2d91379f) ) // should be splitted

	ROM_REGION( 0x80000, "dictionary", 0 )
	ROM_LOAD( "vadic_va2.rom", 0x00000, 0x80000, CRC(a6108f4d) SHA1(3665db538598abb45d9dfe636423e6728a812b12) )

	ROM_REGION( 0x10000, "tvram", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "gvram", ROMREGION_ERASE00 )
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

	ROM_REGION( 0x80000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "vafont.rom", 0x0000, 0x50000, BAD_DUMP CRC(faf7c466) SHA1(196b3d5b7407cb4f286ffe5c1e34ebb1f6905a8c)) // should be splitted

	ROM_REGION( 0x80000, "dictionary", 0 )
	ROM_LOAD( "vadic.rom",  0x0000, 0x80000, CRC(f913c605) SHA1(5ba1f3578d0aaacdaf7194a80e6d520c81ae55fb))

	ROM_REGION( 0x10000, "tvram", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "gvram", ROMREGION_ERASE00 )
ROM_END




COMP( 1987, pc88va,         0,		0,     pc88va,   pc88va, driver_device,  0,    "Nippon Electronic Company",  "PC-88VA", GAME_NOT_WORKING | GAME_NO_SOUND)
COMP( 1988, pc88va2,        pc88va, 0,     pc88va,   pc88va, driver_device,  0,    "Nippon Electronic Company",  "PC-88VA2", GAME_NOT_WORKING | GAME_NO_SOUND )
//COMP( 1988, pc88va3,      pc88va, 0,     pc88va,   pc88va, driver_device,  0,    "Nippon Electronic Company",  "PC-88VA3", GAME_NOT_WORKING )
