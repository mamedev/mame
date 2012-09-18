/*******************************************************************************************************************************************************

    PC-6001 series (c) 1981 NEC

    driver by Angelo Salese

    TODO:
    - Hook up tape loading, images that are floating around the net are already
      ADC'ed, so they should be easy to implement (but not exactly faithful)
    - cassette handling requires a decap of the MCU. It could be possible to
      do some tight synch between the master CPU and a code simulation,but I think
      it's not worth the effort...
    - Identify and hook-up the FDC device, apparently PC-6001 and PC-6601 doesn't even use the same thing;
    - PC-6601: mon r-0 type games doesn't seem to work at all on this version?
    - PC-6001SR: get it to boot, also implement MK-2 compatibility mode (it changes the memory map to behave like the older versions)
    - Currently rewriting the video part without the MC6847 for two reasons:
        A) the later models have a custom video chip in the place of the MC6847,
           so this implementation will be used in the end.
        B) It's easier to me to see what the attribute vram does since I don't
           have any docs atm.

    TODO (game specific):
    - (several AX* games, namely Galaxy Mission Part 1/2 and others): inputs doesn't work
    - AX6 - Demo: When AY-based speech talks, other emus emulates the screen drawing to be a solid green (plain PC-6001) or solid white (Mk2 version),
                  but, according to an original video reference that I've seen, that screen should actually some kind of weird garbage on it ...
    - AX6 - Powered Knight: doesn't work too well, according to the asm code it asks the player to press either 'B' or 'C' then a number but
                            nothing is shown on screen, other emus behaves the same, bad dump?
    - Dawn Patrol (cart): presumably too slow;
    (Mk2 mode 5 games)
    - 3D Golf Simulation Super Version: gameplay / inputs looks broken
    - American Truck: Screen is offset at the loading screen, loading bug?
    - Castle Excellent: copyright text drawing is quite bogus, scans text in vertical instead of horizontal?
    - Dezeni Land (ALL versions) / Hurry Fox 1/2: asks you to "load something", can't do it with current cassette kludge, also, for Dezeni Land(s) keyboard irqs
                                                  doesn't seem to work too well with halt opcode execution?
    - Dezeni Land 1/4: dies after loading of main program
    - Dezeni Land 2: dies at the "load something" screen with presumably wrong stack opcodes
    - (MyCom BASIC games with multiple files): most of them refuses to run ... how to load them?
    - Grobda: when "get ready" speech plays, screen should be full white but instead it's all black, same issue as AX-6 Demo?
    - Pac-Man / Tiny Xevious 2: gameplay is too fast
    - Salad no Kunino Tomato-Hime: can't start a play
    - Space Harrier: inputs doesn't work properly
    - The Black Onyx: dies when it attempts to save the character, that obviously means saving on the tape
    - Yakyukyo / Punchball Mario: waits for an irq, check which one;

========================================================================================================================================================

    PC-6001 (1981-09):

     * CPU: Z80A @ 4 MHz
     * ROM: 16KB + 4KB (chargen) - no kanji
     * RAM: 16KB, it can be expanded to 32KB
     * Text Mode: 32x16 and 2 colors
     * Graphic Modes: 64x48 (9 colors), 128x192 (4 colors), 256x192 (2 colors)
     * Sound: BEEP + PSG - Optional Voice Synth Cart
     * Keyboard: JIS Keyboard with 5 function keys, control key, TAB key,
            HOME/CLR key, INS key, DEL key, GRAPH key, Japanese syllabary
            key, page key, STOP key, and cursor key (4 directions)
     * 1 cartslot, optional floppy drive, optional serial 232 port, 2
            joystick ports


    PC-6001 mkII (1983-07):

     * CPU: Z80A @ 4 MHz
     * ROM: 32KB + 16KB (chargen) + 32KB (kanji) + 16KB (Voice Synth)
     * RAM: 64KB
     * Text Mode: same as PC-6001 with N60-BASIC; 40x20 and 15 colors with
            N60M-BASIC
     * Graphic Modes: same as PC-6001 with N60-BASIC; 80x40 (15 colors),
            160x200 (15 colors), 320x200 (4 colors) with N60M-BASIC
     * Sound: BEEP + PSG
     * Keyboard: JIS Keyboard with 5 function keys, control key, TAB key,
            HOME/CLR key, INS key, DEL key, CAPS key, GRAPH key, Japanese
            syllabary key, page key, mode key, STOP key, and cursor key (4
            directions)
     * 1 cartslot, floppy drive, optional serial 232 port, 2 joystick ports


    PC-6001 mkIISR (1984-12):

     * CPU: Z80A @ 3.58 MHz
     * ROM: 64KB + 16KB (chargen) + 32KB (kanji) + 32KB (Voice Synth)
     * RAM: 64KB
     * Text Mode: same as PC-6001/PC-6001mkII with N60-BASIC; 40x20, 40x25,
            80x20, 80x25 and 15 colors with N66SR-BASIC
     * Graphic Modes: same as PC-6001/PC-6001mkII with N60-BASIC; 80x40 (15 colors),
            320x200 (15 colors), 640x200 (15 colors) with N66SR-BASIC
     * Sound: BEEP + PSG + FM
     * Keyboard: JIS Keyboard with 5 function keys, control key, TAB key,
            HOME/CLR key, INS key, DEL key, CAPS key, GRAPH key, Japanese
            syllabary key, page key, mode key, STOP key, and cursor key (4
            directions)
     * 1 cartslot, floppy drive, optional serial 232 port, 2 joystick ports


    info from http://www.geocities.jp/retro_zzz/machines/nec/6001/spc60.html

==================================================================================

PC-6001 irq table:
irq vector 0x00: writes 0x00 to [$fa19]                                                     ;(unused)
irq vector 0x02: (A = 0, B = 0) tests ppi port c, does something with ay ports (plus more?) ;keyboard data ready, no kanji lock, no caps lock
irq vector 0x04:                                                                            ;uart irq
irq vector 0x06: operates with $fa28, $fa2e, $fd1b                                          ;timer irq
irq vector 0x08: tests ppi port c, puts port A to $fa1d,puts 0x02 to [$fa19]                ;tape data ready
irq vector 0x0a: writes 0x00 to [$fa19]                                                     ;(unused)
irq vector 0x0c: writes 0x00 to [$fa19]                                                     ;(unused)
irq vector 0x0e: same as 2, (A = 0x03, B = 0x00)                                            ;keyboard data ready, unknown type
irq vector 0x10: same as 2, (A = 0x03, B = 0x00)                                            ;(unused)
irq vector 0x12: writes 0x10 to [$fa19]                                                     ;end of tape reached
irq vector 0x14: same as 2, (A = 0x00, B = 0x01)                                            ;kanji lock enabled
irq vector 0x16: tests ppi port c, writes the result to $feca.                              ;joystick
irq vector 0x18:                                                                            ;TVR (?)
irq vector 0x1a:                                                                            ;Date
irq vector 0x1c:                                                                            ;(unused)
irq vector 0x1e:                                                                            ;(unused)
irq vector 0x20:                                                                            ;voice
irq vector 0x22:                                                                            ;VRTC (?)
irq vector 0x24:                                                                            ;(unused)
irq vector 0x26:                                                                            ;(unused)


*******************************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "video/mc6847.h"
#include "sound/ay8910.h"
#include "sound/wave.h"

#include "imagedev/cassette.h"
#include "imagedev/cartslot.h"
#include "formats/p6001_cas.h"


class pc6001_state : public driver_device
{
public:
	pc6001_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_ppi(*this, "ppi8255")
	,
		m_ram(*this, "ram"){ }

	required_device<i8255_device> m_ppi;

	optional_shared_ptr<UINT8> m_ram;
	UINT8 *m_video_ram;
	UINT8 m_irq_vector;
	UINT8 m_cas_switch;
	UINT8 m_sys_latch;
	UINT8 m_timer_irq_mask;
	UINT32 m_cas_offset;
	UINT32 m_cas_maxsize;
	emu_timer *m_timer_irq_timer;
	UINT16 m_timer_hz_div;
	UINT8 m_ex_vram_bank;
	UINT8 m_bgcol_bank;
	UINT8 m_exgfx_text_mode;
	UINT8 m_exgfx_bitmap_mode;
	UINT8 m_exgfx_2bpp_mode;
	UINT8 m_bank_r0;
	UINT8 m_bank_r1;
	UINT8 m_gfx_bank_on;
	UINT8 m_bank_w;
	UINT8 m_bank_opt;
	UINT8 m_timer_irq_mask2;
	UINT8 m_timer_irq_vector;
	UINT32 m_cgrom_bank_addr;
	UINT8 m_sr_video_mode;
	UINT8 m_port_c_8255;
	UINT8 m_cur_keycode;
	UINT8 m_sr_bank_r[8];
	UINT8 m_sr_bank_w[8];
	UINT8 m_kludge;
	UINT32 m_old_key1;
	UINT32 m_old_key2;
	UINT32 m_old_key3;
	DECLARE_WRITE8_MEMBER(pc6001_system_latch_w);
	DECLARE_READ8_MEMBER(nec_ppi8255_r);
	DECLARE_WRITE8_MEMBER(nec_ppi8255_w);
	DECLARE_WRITE8_MEMBER(pc6001m2_bank_r0_w);
	DECLARE_WRITE8_MEMBER(pc6001m2_bank_r1_w);
	DECLARE_WRITE8_MEMBER(pc6001m2_bank_w0_w);
	DECLARE_WRITE8_MEMBER(pc6001m2_opt_bank_w);
	DECLARE_WRITE8_MEMBER(work_ram0_w);
	DECLARE_WRITE8_MEMBER(work_ram1_w);
	DECLARE_WRITE8_MEMBER(work_ram2_w);
	DECLARE_WRITE8_MEMBER(work_ram3_w);
	DECLARE_WRITE8_MEMBER(work_ram4_w);
	DECLARE_WRITE8_MEMBER(work_ram5_w);
	DECLARE_WRITE8_MEMBER(work_ram6_w);
	DECLARE_WRITE8_MEMBER(work_ram7_w);
	DECLARE_WRITE8_MEMBER(necmk2_ppi8255_w);
	DECLARE_WRITE8_MEMBER(pc6001m2_system_latch_w);
	DECLARE_WRITE8_MEMBER(pc6001m2_vram_bank_w);
	DECLARE_WRITE8_MEMBER(pc6001m2_col_bank_w);
	DECLARE_READ8_MEMBER(upd7752_reg_r);
	DECLARE_WRITE8_MEMBER(upd7752_reg_w);
	DECLARE_WRITE8_MEMBER(pc6001m2_0xf3_w);
	DECLARE_WRITE8_MEMBER(pc6001m2_timer_adj_w);
	DECLARE_WRITE8_MEMBER(pc6001m2_timer_irqv_w);
	DECLARE_READ8_MEMBER(pc6001m2_bank_r0_r);
	DECLARE_READ8_MEMBER(pc6001m2_bank_r1_r);
	DECLARE_READ8_MEMBER(pc6001m2_bank_w0_r);
	DECLARE_READ8_MEMBER(pc6601_fdc_r);
	DECLARE_WRITE8_MEMBER(pc6601_fdc_w);
	DECLARE_READ8_MEMBER(pc6001sr_bank_rn_r);
	DECLARE_WRITE8_MEMBER(pc6001sr_bank_rn_w);
	DECLARE_READ8_MEMBER(pc6001sr_bank_wn_r);
	DECLARE_WRITE8_MEMBER(pc6001sr_bank_wn_w);
	DECLARE_WRITE8_MEMBER(sr_work_ram0_w);
	DECLARE_WRITE8_MEMBER(sr_work_ram1_w);
	DECLARE_WRITE8_MEMBER(sr_work_ram2_w);
	DECLARE_WRITE8_MEMBER(sr_work_ram3_w);
	DECLARE_WRITE8_MEMBER(sr_work_ram4_w);
	DECLARE_WRITE8_MEMBER(sr_work_ram5_w);
	DECLARE_WRITE8_MEMBER(sr_work_ram6_w);
	DECLARE_WRITE8_MEMBER(sr_work_ram7_w);
	DECLARE_WRITE8_MEMBER(pc6001sr_mode_w);
	DECLARE_WRITE8_MEMBER(pc6001sr_vram_bank_w);
	DECLARE_WRITE8_MEMBER(pc6001sr_system_latch_w);
	DECLARE_WRITE8_MEMBER(necsr_ppi8255_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	DECLARE_MACHINE_RESET(pc6001m2);
	DECLARE_PALETTE_INIT(pc6001m2);
	DECLARE_MACHINE_RESET(pc6001sr);
	UINT32 screen_update_pc6001(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_pc6001m2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_pc6001sr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/* PC6001mk2 specific */
/* PC6001SR specific */

#define IRQ_LOG (0)

void pc6001_state::video_start()
{
	#if 0
	m6847_config cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.type = M6847_VERSION_M6847T1_NTSC;
	cfg.get_attributes = pc6001_get_attributes;
	cfg.get_video_ram = pc6001_get_video_ram;
	cfg.get_char_rom = pc6001_get_char_rom;
	m6847_init(machine(), &cfg);
	#endif
	m_video_ram = auto_alloc_array(machine(), UINT8, 0x4000);
}

/* this is known as gfx mode 4 */
static void draw_gfx_mode4(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect,int attr)
{
	pc6001_state *state = machine.driver_data<pc6001_state>();
	int x,y,xi;
	int fgcol,color;
	int col_setting;
	static const UINT8 pen_gattr[4][4] = {
		{ 0, 1, 6, 2 }, //Red / Blue
		{ 0, 6, 1, 2 }, //Blue / Red
		{ 0, 5, 2, 2 }, //Pink / Green
		{ 0, 2, 5, 2 }, //Green / Pink
	};
	static const UINT8 pen_wattr[4][4] = {
		{ 0, 1, 6, 7 }, //Red / Blue
		{ 0, 6, 1, 7 }, //Blue / Red
		{ 0, 5, 2, 7 }, //Pink / Green
		{ 0, 2, 5, 7 }, //Green / Pink
	};
	col_setting = machine.root_device().ioport("MODE4_DSW")->read() & 7;

	if((attr & 0x0c) != 0x0c)
		popmessage("Mode 4 vram attr != 0x0c, contact MESSdev");

	for(y=0;y<192;y++)
	{
		for(x=0;x<32;x++)
		{
			int tile = state->m_video_ram[(x+(y*32))+0x200];

			if(col_setting == 0x00) //monochrome
			{
				for(xi=0;xi<8;xi++)
				{
					fgcol = (attr & 2) ? 7 : 2;

					color = ((tile)>>(7-xi) & 1) ? fgcol : 0;

					bitmap.pix16((y+24), (x*8+xi)+32) = machine.pens[color];
				}
			}
			else
			{
				for(xi=0;xi<4;xi++)
				{
					fgcol = ((tile)>>(6-(xi*2)) & 3);

					color = (attr & 2) ? (pen_wattr[col_setting-1][fgcol]) : (pen_gattr[col_setting-1][fgcol]);

					bitmap.pix16((y+24), ((x*8+xi*2)+0)+32) = machine.pens[color];
					bitmap.pix16((y+24), ((x*8+xi*2)+1)+32) = machine.pens[color];
				}
			}
		}
	}
}

static void draw_bitmap_2bpp(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect, int attr)
{
	pc6001_state *state = machine.driver_data<pc6001_state>();
	int color,x,y,xi,yi;

	int shrink_x = 2*4;
	int shrink_y = (attr & 8) ? 1 : 2;
	int w = (shrink_x == 8) ? 32 : 16;
	int col_bank = ((attr & 2)<<1);

	if(attr & 4)
	{
		for(y=0;y<(192/shrink_y);y++)
		{
			for(x=0;x<w;x++)
			{
				int tile = state->m_video_ram[(x+(y*32))+0x200];

				for(yi=0;yi<shrink_y;yi++)
				{
					for(xi=0;xi<shrink_x;xi++)
					{
						int i;
						i = (shrink_x == 8) ? (xi & 0x06) : (xi & 0x0c)>>1;
						color = ((tile >> i) & 3)+8;
						color+= col_bank;

						bitmap.pix16(((y*shrink_y+yi)+24), (x*shrink_x+((shrink_x-1)-xi))+32) = machine.pens[color];
					}
				}
			}
		}
	}
	else /* TODO: clean this up */
	{
		for(y=0;y<(192/shrink_y);y+=3)
		{
			for(x=0;x<w;x++)
			{
				int tile = state->m_video_ram[(x+((y/3)*32))+0x200];

				for(yi=0;yi<shrink_y;yi++)
				{
					for(xi=0;xi<shrink_x;xi++)
					{
						int i;
						i = (shrink_x == 8) ? (xi & 0x06) : (xi & 0x0c)>>1;
						color = ((tile >> i) & 3)+8;
						color+= col_bank;

						bitmap.pix16((((y+0)*shrink_y+yi)+24), (x*shrink_x+((shrink_x-1)-xi))+32) = machine.pens[color];
						bitmap.pix16((((y+1)*shrink_y+yi)+24), (x*shrink_x+((shrink_x-1)-xi))+32) = machine.pens[color];
						bitmap.pix16((((y+2)*shrink_y+yi)+24), (x*shrink_x+((shrink_x-1)-xi))+32) = machine.pens[color];
					}
				}
			}
		}
	}
}

static void draw_tile_3bpp(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect,int x,int y,int tile,int attr)
{
	int color,pen,xi,yi;

	if(attr & 0x10) //2x2 squares on a single cell
		pen = (tile & 0x70)>>4;
	else //2x3
		pen = (tile & 0xc0) >> 6 | (attr & 2)<<1;

	for(yi=0;yi<12;yi++)
	{
		for(xi=0;xi<8;xi++)
		{
			int i;
			i = (xi & 4)>>2; //x-axis
			if(attr & 0x10) //2x2
			{
				i+= (yi >= 6) ? 2 : 0; //y-axis
			}
			else //2x3
			{
				i+= (yi & 4)>>1; //y-axis 1
				i+= (yi & 8)>>1; //y-axis 2
			}

			color = ((tile >> i) & 1) ? pen+8 : 0;

			bitmap.pix16(((y*12+(11-yi))+24), (x*8+(7-xi))+32) = machine.pens[color];
		}
	}
}

static void draw_tile_text(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect,int x,int y,int tile,int attr,int has_mc6847)
{
	int xi,yi,pen,fgcol,color;
	UINT8 *gfx_data = machine.root_device().memregion("gfx1")->base();

	for(yi=0;yi<12;yi++)
	{
		for(xi=0;xi<8;xi++)
		{
			pen = gfx_data[(tile*0x10)+yi]>>(7-xi) & 1;

			if(has_mc6847)
			{
				fgcol = (attr & 2) ? 0x12 : 0x10;

				if(attr & 1)
					color = pen ? (fgcol+0) : (fgcol+1);
				else
					color = pen ? (fgcol+1) : (fgcol+0);

			}
			else
			{
				fgcol = (attr & 2) ? 2 : 7;

				if(attr & 1)
					color = pen ? 0 : fgcol;
				else
					color = pen ? fgcol : 0;
			}

			bitmap.pix16(((y*12+yi)+24), (x*8+xi)+32) = machine.pens[color];
		}
	}
}

static void draw_border(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect,int attr,int has_mc6847)
{
	int x,y,color;

	for(y=0;y<240;y++)
	{
		for(x=0;x<320;x++)
		{
			if(!has_mc6847) //mk2 border color is always black
				color = 0;
			else if((attr & 0x90) == 0x80) //2bpp
				color = ((attr & 2)<<1) + 8;
			else if((attr & 0x90) == 0x90) //1bpp
				color = (attr & 2) ? 7 : 2;
			else
				color = 0; //FIXME: other modes not yet checked

			bitmap.pix16(y, x) = machine.pens[color];
		}
	}
}

static void pc6001_screen_draw(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect, int has_mc6847)
{
	pc6001_state *state = machine.driver_data<pc6001_state>();
	int x,y;
	int tile,attr;

	attr = state->m_video_ram[0];

	draw_border(machine,bitmap,cliprect,attr,has_mc6847);

	if(attr & 0x80) // gfx mode
	{
		if(attr & 0x10) // 256x192x1 mode (FIXME: might be a different trigger)
		{
			draw_gfx_mode4(machine,bitmap,cliprect,attr);
		}
		else // 128x192x2 mode
		{
			draw_bitmap_2bpp(machine,bitmap,cliprect,attr);
		}
	}
	else // text mode
	{
		for(y=0;y<16;y++)
		{
			for(x=0;x<32;x++)
			{
				tile = state->m_video_ram[(x+(y*32))+0x200];
				attr = state->m_video_ram[(x+(y*32)) & 0x1ff];

				if(attr & 0x40)
				{
					draw_tile_3bpp(machine,bitmap,cliprect,x,y,tile,attr);
				}
				else
				{
					draw_tile_text(machine,bitmap,cliprect,x,y,tile,attr,has_mc6847);
				}
			}
		}
	}
}

UINT32 pc6001_state::screen_update_pc6001(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	pc6001_screen_draw(machine(),bitmap,cliprect,1);

	return 0;
}

UINT32 pc6001_state::screen_update_pc6001m2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,tile,attr;

	/* note: bitmap mode have priority over everything else, check American Truck */
	if(m_exgfx_bitmap_mode)
	{
		int count,color,i;

		count = 0;

		for(y=0;y<200;y++)
		{
			for(x=0;x<160;x+=4)
			{
				for(i=0;i<4;i++)
				{
					int pen[2];
#if 0
					/* palette reference: */
					static const UINT8 pal_num[] = { 0x00, 0x04, 0x01, 0x05,
						0x02, 0x06, 0x03, 0x07,
						0x08, 0x0c, 0x09, 0x0d,
						0x0a, 0x0e, 0x0b, 0x0f };

					color |= pal_num[(pen[0] & 3) | ((pen[1] & 3) << 2)];
#endif

					pen[0] = m_video_ram[count+0x0000] >> (6-i*2) & 3;
					pen[1] = m_video_ram[count+0x2000] >> (6-i*2) & 3;

					color = 0x10;
					color |= ((pen[0] & 1) << 2);
					color |= ((pen[0] & 2) >> 1);
					color |= ((pen[1] & 1) << 1);
					color |= ((pen[1] & 2) << 2);

					if (cliprect.contains((x+i)*2+0, y))
						bitmap.pix16(y, (x+i)*2+0) = machine().pens[color];
					if (cliprect.contains((x+i)*2+1, y))
						bitmap.pix16(y, (x+i)*2+1) = machine().pens[color];
				}

				count++;
			}
		}
	}
	else if(m_exgfx_2bpp_mode)
	{
		int count,color,i;

		count = 0;

		for(y=0;y<200;y++)
		{
			for(x=0;x<320;x+=8)
			{
				for(i=0;i<8;i++)
				{
					int pen[2];
#if 0
					/* palette reference: */
					static const UINT8 pal_num[] = { 0x00, 0x04, 0x01, 0x05 };

					color |= pal_num[(pen[0] & 1) | ((pen[1] & 1) << 1)];
#endif

					pen[0] = m_video_ram[count+0x0000] >> (7-i) & 1;
					pen[1] = m_video_ram[count+0x2000] >> (7-i) & 1;

					if(m_bgcol_bank & 4) //PC-6001 emulation mode
					{
						color = 0x08;
						color |= (pen[0]) | (pen[1]<<1);
						color |= (m_bgcol_bank & 1) << 2;
					}
					else //Mk-2 mode
					{
						color = 0x10;
						color |= ((pen[0] & 1) << 2);
						color |= ((pen[1] & 1) >> 0);
						color |= ((m_bgcol_bank & 1) << 1);
						color |= ((m_bgcol_bank & 2) << 2);
					}

					if (cliprect.contains(x+i, y))
						bitmap.pix16(y, (x+i)) = machine().pens[color];
				}

				count++;
			}
		}

	}
	else if(m_exgfx_text_mode)
	{
		int xi,yi,pen,fgcol,bgcol,color;
		UINT8 *gfx_data = machine().root_device().memregion("gfx1")->base();

		for(y=0;y<20;y++)
		{
			for(x=0;x<40;x++)
			{
				/*
                exgfx attr format:
                x--- ---- rom bank select
                -xxx ---- bg color
                ---- xxxx fg color
                Note that the exgfx banks a different gfx ROM
                */
				tile = m_video_ram[(x+(y*40))+0x400] + 0x200;
				attr = m_video_ram[(x+(y*40)) & 0x3ff];
				tile+= ((attr & 0x80) << 1);

				for(yi=0;yi<12;yi++)
				{
					for(xi=0;xi<8;xi++)
					{
						pen = gfx_data[(tile*0x10)+yi]>>(7-xi) & 1;

						fgcol = (attr & 0x0f) + 0x10;
						bgcol = ((attr & 0x70) >> 4) + 0x10 + ((m_bgcol_bank & 2) << 2);

						color = pen ? fgcol : bgcol;

						if (cliprect.contains(x*8+xi, y*12+yi))
							bitmap.pix16(((y*12+yi)), (x*8+xi)) = machine().pens[color];
					}
				}
			}
		}
	}
	else
	{
		attr = m_video_ram[0];
		pc6001_screen_draw(machine(),bitmap,cliprect,0);
	}

	return 0;
}

UINT32 pc6001_state::screen_update_pc6001sr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,tile,attr;
	int xi,yi,pen,fgcol,bgcol,color;
	UINT8 *gfx_data = memregion("gfx1")->base();


	if(m_sr_video_mode & 8) // text mode
	{
		for(y=0;y<20;y++)
		{
			for(x=0;x<40;x++)
			{
				tile = m_video_ram[(x+(y*40))*2+0];
				attr = m_video_ram[(x+(y*40))*2+1];
				tile+= ((attr & 0x80) << 1);

				for(yi=0;yi<12;yi++)
				{
					for(xi=0;xi<8;xi++)
					{
						pen = gfx_data[(tile*0x10)+yi]>>(7-xi) & 1;

						fgcol = (attr & 0x0f) + 0x10;
						bgcol = ((attr & 0x70) >> 4) + 0x10 + ((m_bgcol_bank & 2) << 2);

						color = pen ? fgcol : bgcol;

						if (cliprect.contains(x*8+xi, y*12+yi))
							bitmap.pix16(((y*12+yi)), (x*8+xi)) = machine().pens[color];
					}
				}
			}
		}
	}
	else //4bpp bitmap mode (TODO)
	{
		int count;

		count = 0;

		for(y=0;y<200;y+=2)
		{
			for(x=0;x<320;x+=4)
			{
				color = m_video_ram[count] & 0x0f;

				if (cliprect.contains(x+0, y+0))
					bitmap.pix16((y+0), (x+0)) = machine().pens[color+0x10];

				color = (m_video_ram[count] & 0xf0) >> 4;

				if (cliprect.contains(x+1, y+0))
					bitmap.pix16((y+0), (x+1)) = machine().pens[color+0x10];

				color = m_video_ram[count+1] & 0x0f;

				if (cliprect.contains(x+2, y+0))
					bitmap.pix16((y+0), (x+2)) = machine().pens[color+0x10];

				color = (m_video_ram[count+1] & 0xf0) >> 4;

				if (cliprect.contains(x+3, y+0))
					bitmap.pix16((y+0), (x+3)) = machine().pens[color+0x10];

				color = m_video_ram[count+2] & 0x0f;

				if (cliprect.contains(x+0, y+1))
					bitmap.pix16((y+1), (x+0)) = machine().pens[color+0x10];

				color = (m_video_ram[count+2] & 0xf0) >> 4;

				if (cliprect.contains(x+1, y+1))
					bitmap.pix16((y+1), (x+1)) = machine().pens[color+0x10];

				color = m_video_ram[count+3] & 0x0f;

				if (cliprect.contains(x+2, y+1))
					bitmap.pix16((y+1), (x+2)) = machine().pens[color+0x10];

				color = (m_video_ram[count+3] & 0xf0) >> 4;

				if (cliprect.contains(x+3, y+1))
					bitmap.pix16((y+1), (x+3)) = machine().pens[color+0x10];


				count+=4;
			}
		}
	}

	return 0;
}

WRITE8_MEMBER(pc6001_state::pc6001_system_latch_w)
{
	static const UINT16 startaddr[] = {0xC000, 0xE000, 0x8000, 0xA000 };

	m_video_ram =  m_ram + startaddr[(data >> 1) & 0x03] - 0x8000;

	if((!(m_sys_latch & 8)) && data & 0x8) //PLAY tape cmd
	{
		m_cas_switch = 1;
		//machine().device(CASSETTE_TAG )->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
		//machine().device(CASSETTE_TAG )->change_state(CASSETTE_PLAY,CASSETTE_MASK_UISTATE);
	}
	if((m_sys_latch & 8) && ((data & 0x8) == 0)) //STOP tape cmd
	{
		m_cas_switch = 0;
		//machine().device(CASSETTE_TAG )->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
		//machine().device(CASSETTE_TAG )->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
		//m_irq_vector = 0x00;
		//machine().device("maincpu")->execute().set_input_line(0, ASSERT_LINE);
	}

	m_sys_latch = data;
	m_timer_irq_mask = data & 1;
	//printf("%02x\n",data);
}

#if 0
static ATTR_CONST UINT8 pc6001_get_attributes(running_machine &machine, UINT8 c,int scanline, int pos)
{
	pc6001_state *state = machine.driver_data<pc6001_state>();
	UINT8 result = 0x00;
	UINT8 val = state->m_video_ram [(scanline / 12) * 0x20 + pos];

	if (val & 0x01) {
		result |= M6847_INV;
	}
	if (val & 0x40)
		result |= M6847_AG | M6847_GM1; //TODO

	result |= M6847_INTEXT; // always use external ROM
	return result;
}

static const UINT8 *pc6001_get_video_ram(running_machine &machine, int scanline)
{
	pc6001_state *state = machine.driver_data<pc6001_state>();
	return state->m_video_ram +0x0200+ (scanline / 12) * 0x20;
}

static UINT8 pc6001_get_char_rom(running_machine &machine, UINT8 ch, int line)
{
	UINT8 *gfx = machine.root_device().memregion("gfx1")->base();
	return gfx[ch*16+line];
}
#endif


READ8_MEMBER(pc6001_state::nec_ppi8255_r)
{
	if (offset==2)
		return m_port_c_8255;
	else if(offset==0)
	{
		UINT8 res;
		res = m_cur_keycode;
		//m_cur_keycode = 0;
		return res;
	}

	return m_ppi->read(space, offset);
}

WRITE8_MEMBER(pc6001_state::nec_ppi8255_w)
{
	if (offset==3)
	{
		if(data & 1)
			m_port_c_8255 |=   1<<((data>>1)&0x07);
		else
			m_port_c_8255 &= ~(1<<((data>>1)&0x07));

		switch(data) {
        	case 0x08: m_port_c_8255 |= 0x88; break;
        	case 0x09: m_port_c_8255 &= 0xf7; break;
        	case 0x0c: m_port_c_8255 |= 0x28; break;
        	case 0x0d: m_port_c_8255 &= 0xf7; break;
        	default: break;
		}

		m_port_c_8255 |= 0xa8;

		{
			UINT8 *gfx_data = memregion("gfx1")->base();
			UINT8 *ext_rom = memregion("cart_img")->base();

			//printf("%02x\n",data);

			if((data & 0x0f) == 0x05)
				membank("bank1")->set_base(&ext_rom[0x2000]);
			if((data & 0x0f) == 0x04)
				membank("bank1")->set_base(&gfx_data[0]);
		}
	}
	m_ppi->write(space,offset,data);
}

static ADDRESS_MAP_START(pc6001_map, AS_PROGRAM, 8, pc6001_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_WRITENOP
	AM_RANGE(0x4000, 0x5fff) AM_ROM AM_REGION("cart_img",0)
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc6001_io , AS_IO, 8, pc6001_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x80) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE(0x81, 0x81) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE(0x90, 0x93) AM_MIRROR(0x0c) AM_READWRITE(nec_ppi8255_r, nec_ppi8255_w)
	AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x0c) AM_DEVWRITE_LEGACY("ay8910", ay8910_address_w)
	AM_RANGE(0xa1, 0xa1) AM_MIRROR(0x0c) AM_DEVWRITE_LEGACY("ay8910", ay8910_data_w)
	AM_RANGE(0xa2, 0xa2) AM_MIRROR(0x0c) AM_DEVREAD_LEGACY("ay8910", ay8910_r)
	AM_RANGE(0xa3, 0xa3) AM_MIRROR(0x0c) AM_WRITENOP
	AM_RANGE(0xb0, 0xb0) AM_MIRROR(0x0f) AM_WRITE(pc6001_system_latch_w)
	AM_RANGE(0xd0, 0xd3) AM_MIRROR(0x0c) AM_NOP // disk device
ADDRESS_MAP_END

/*
    ROM_REGION( 0x28000, "maincpu", ROMREGION_ERASEFF )
    ROM_LOAD( "basicrom.62", 0x10000, 0x8000, CRC(950ac401) SHA1(fbf195ba74a3b0f80b5a756befc96c61c2094182) )
    ROM_LOAD( "voicerom.62", 0x18000, 0x4000, CRC(49b4f917) SHA1(1a2d18f52ef19dc93da3d65f19d3abbd585628af) )
    ROM_LOAD( "cgrom60.62",  0x1c000, 0x2000, CRC(81eb5d95) SHA1(53d8ae9599306ff23bf95208d2f6cc8fed3fc39f) )
    ROM_LOAD( "cgrom60m.62", 0x1e000, 0x2000, CRC(3ce48c33) SHA1(f3b6c63e83a17d80dde63c6e4d86adbc26f84f79) )
    ROM_LOAD( "kanjirom.62", 0x20000, 0x8000, CRC(20c8f3eb) SHA1(4c9f30f0a2ebbe70aa8e697f94eac74d8241cadd) )
*/

#define BASICROM(_v_) \
	0x10000+0x2000*_v_ \

#define VOICEROM(_v_) \
	0x18000+0x2000*_v_ \

#define TVROM(_v_) \
	0x1c000+0x2000*_v_ \

#define KANJIROM(_v_) \
	0x20000+0x2000*_v_ \

#define WRAM(_v_) \
	0x28000+0x2000*_v_ \

#define EXWRAM(_v_) \
	0x38000+0x2000*_v_ \

#define EXROM(_v_) \
	0x48000+0x2000*_v_ \

#define INVALID(_v_) \
	0x4c000+0x2000*_v_ \

/* FIXME: some comments aren't right */
static const UINT32 banksw_table_r0[0x10*4][4] = {
	/* 0 */
	{ INVALID(0),	INVALID(0),		INVALID(0),     INVALID(0)  },	//0x00: <invalid setting>
	{ BASICROM(0),	BASICROM(1),	BASICROM(2),	BASICROM(3) },	//0x01: basic rom 0 & 1 / basic rom 2 & 3
	{ TVROM(0),		TVROM(1),		VOICEROM(0),	VOICEROM(1) },	//0x02: tv rom 0 & 1 / voice rom 0 & 1
	{ EXROM(1),		EXROM(1),		EXROM(1),		EXROM(1)	},	//0x03: ex rom 1 & 1 / ex rom 1 & 1
	{ EXROM(0),		EXROM(0),		EXROM(0),		EXROM(0)	},	//0x04: ex rom 0 & 0 / ex rom 0 & 0
	{ TVROM(1),		BASICROM(1),	VOICEROM(0),	BASICROM(3) },	//0x05: tv rom 1 & basic rom 1 / voice rom 0 & basic 3
	{ BASICROM(0),	TVROM(2),		BASICROM(2),	VOICEROM(1) },	//0x06: basic rom 0 & tv rom 2 / basic rom 2 & voice 1
	{ EXROM(0),		EXROM(1),		EXROM(0),		EXROM(1)	},	//0x07: ex rom 0 & ex rom 1 / ex rom 0 & ex rom 1
	{ EXROM(1),		EXROM(0),		EXROM(1),		EXROM(0)	},	//0x08: ex rom 1 & ex rom 0 / ex rom 1 & ex rom 0
	{ EXROM(1),		BASICROM(1),	EXROM(1),		BASICROM(3) },	//0x09: ex rom 1 & basic rom 1 / ex rom 1 & basic 3
	{ BASICROM(0),  EXROM(1),		BASICROM(2),	EXROM(1)	},	//0x0a: basic rom 0 & ex rom 1 / basic rom 2 & ex rom 1
	{ EXROM(0),		TVROM(2),		EXROM(0),		VOICEROM(1) },	//0x0b: ex rom 0 & tv rom 2 / ex rom 0 & voice 1
	{ TVROM(1),		EXROM(0),		VOICEROM(0),	EXROM(0)	},	//0x0c: tv rom 1 & ex rom 0 / voice rom 0 & ex rom 0
	{ WRAM(0),		WRAM(1),		WRAM(2),		WRAM(3)		},	//0x0d: ram 0 & 1 / ram 2 & 3
	{ EXWRAM(0),	EXWRAM(1),		EXWRAM(2),		EXWRAM(3)	},	//0x0e: exram 0 & 1 / exram 2 & 3
	{ INVALID(0),	INVALID(0),		INVALID(0),		INVALID(0)  },	//0x0f: <invalid setting>
	/* 1 */
	{ INVALID(0),	INVALID(0),		INVALID(0),     INVALID(0)  },	//0x00: <invalid setting>
	{ BASICROM(0),	BASICROM(1),	BASICROM(2),	BASICROM(3) },	//0x01: basic rom 0 & 1 / basic rom 2 & 3
	{ KANJIROM(0),	KANJIROM(1),	KANJIROM(0),	KANJIROM(1) },	//0x02: tv rom 0 & 1 / voice rom 0 & 1
	{ EXROM(1),		EXROM(1),		EXROM(1),		EXROM(1)	},	//0x03: ex rom 1 & 1 / ex rom 1 & 1
	{ EXROM(0),		EXROM(0),		EXROM(0),		EXROM(0)	},	//0x04: ex rom 0 & 0 / ex rom 0 & 0
	{ KANJIROM(0),	BASICROM(1),	KANJIROM(0),	BASICROM(3) },	//0x05: tv rom 1 & basic rom 1 / voice rom 0 & basic 3
	{ BASICROM(0),	KANJIROM(1),	BASICROM(2),	KANJIROM(1) },	//0x06: basic rom 0 & tv rom 2 / basic rom 2 & voice 1
	{ EXROM(0),		EXROM(1),		EXROM(0),		EXROM(1)	},	//0x07: ex rom 0 & ex rom 1 / ex rom 0 & ex rom 1
	{ EXROM(1),		EXROM(0),		EXROM(1),		EXROM(0)	},	//0x08: ex rom 1 & ex rom 0 / ex rom 1 & ex rom 0
	{ EXROM(1),		BASICROM(1),	EXROM(1),		BASICROM(3) },	//0x09: ex rom 1 & basic rom 1 / ex rom 1 & basic 3
	{ BASICROM(0),  EXROM(1),		BASICROM(2),	EXROM(1)	},	//0x0a: basic rom 0 & ex rom 1 / basic rom 2 & ex rom 1
	{ EXROM(0),		KANJIROM(1),	EXROM(0),		KANJIROM(1) },	//0x0b: ex rom 0 & tv rom 2 / ex rom 0 & voice 1
	{ KANJIROM(0),	EXROM(0),		KANJIROM(0),	EXROM(0)	},	//0x0c: tv rom 1 & ex rom 0 / voice rom 0 & ex rom 0
	{ WRAM(0),		WRAM(1),		WRAM(2),		WRAM(3)		},	//0x0d: ram 0 & 1 / ram 2 & 3
	{ EXWRAM(0),	EXWRAM(1),		EXWRAM(2),		EXWRAM(3)	},	//0x0e: exram 0 & 1 / exram 2 & 3
	{ INVALID(0),	INVALID(0),		INVALID(0),		INVALID(0)  },	//0x0f: <invalid setting>
	/* 2 */
	{ INVALID(0),	INVALID(0),		INVALID(0),     INVALID(0)  },	//0x00: <invalid setting>
	{ BASICROM(0),	BASICROM(1),	BASICROM(2),	BASICROM(3) },	//0x01: basic rom 0 & 1 / basic rom 2 & 3
	{ TVROM(0),		TVROM(1),		VOICEROM(0),	VOICEROM(1) },	//0x02: tv rom 0 & 1 / voice rom 0 & 1
	{ EXROM(1),		EXROM(1),		EXROM(1),		EXROM(1)	},	//0x03: ex rom 1 & 1 / ex rom 1 & 1
	{ EXROM(0),		EXROM(0),		EXROM(0),		EXROM(0)	},	//0x04: ex rom 0 & 0 / ex rom 0 & 0
	{ TVROM(1),		BASICROM(1),	VOICEROM(0),	BASICROM(3) },	//0x05: tv rom 1 & basic rom 1 / voice rom 0 & basic 3
	{ BASICROM(0),	TVROM(2),		BASICROM(2),	VOICEROM(1) },	//0x06: basic rom 0 & tv rom 2 / basic rom 2 & voice 1
	{ EXROM(0),		EXROM(1),		EXROM(0),		EXROM(1)	},	//0x07: ex rom 0 & ex rom 1 / ex rom 0 & ex rom 1
	{ EXROM(1),		EXROM(0),		EXROM(1),		EXROM(0)	},	//0x08: ex rom 1 & ex rom 0 / ex rom 1 & ex rom 0
	{ EXROM(1),		BASICROM(1),	EXROM(1),		BASICROM(3) },	//0x09: ex rom 1 & basic rom 1 / ex rom 1 & basic 3
	{ BASICROM(0),  EXROM(1),		BASICROM(2),	EXROM(1)	},	//0x0a: basic rom 0 & ex rom 1 / basic rom 2 & ex rom 1
	{ EXROM(0),		TVROM(2),		EXROM(0),		VOICEROM(1) },	//0x0b: ex rom 0 & tv rom 2 / ex rom 0 & voice 1
	{ TVROM(1),		EXROM(0),		VOICEROM(0),	EXROM(0)	},	//0x0c: tv rom 1 & ex rom 0 / voice rom 0 & ex rom 0
	{ WRAM(0),		WRAM(1),		WRAM(2),		WRAM(3)		},	//0x0d: ram 0 & 1 / ram 2 & 3
	{ EXWRAM(0),	EXWRAM(1),		EXWRAM(2),		EXWRAM(3)	},	//0x0e: exram 0 & 1 / exram 2 & 3
	{ INVALID(0),	INVALID(0),		INVALID(0),		INVALID(0)  },	//0x0f: <invalid setting>
	/* 3 */
	{ INVALID(0),	INVALID(0),		INVALID(0),     INVALID(0)  },	//0x00: <invalid setting>
	{ BASICROM(0),	BASICROM(1),	BASICROM(2),	BASICROM(3) },	//0x01: basic rom 0 & 1 / basic rom 2 & 3
	{ KANJIROM(2),	KANJIROM(3),	KANJIROM(2),	KANJIROM(3) },	//0x02: tv rom 0 & 1 / voice rom 0 & 1
	{ EXROM(1),		EXROM(1),		EXROM(1),		EXROM(1)	},	//0x03: ex rom 1 & 1 / ex rom 1 & 1
	{ EXROM(0),		EXROM(0),		EXROM(0),		EXROM(0)	},	//0x04: ex rom 0 & 0 / ex rom 0 & 0
	{ KANJIROM(2),	BASICROM(1),	KANJIROM(2),	BASICROM(3) },	//0x05: tv rom 1 & basic rom 1 / voice rom 0 & basic 3
	{ BASICROM(0),	KANJIROM(3),	BASICROM(2),	KANJIROM(3) },	//0x06: basic rom 0 & tv rom 2 / basic rom 2 & voice 1
	{ EXROM(0),		EXROM(1),		EXROM(0),		EXROM(1)	},	//0x07: ex rom 0 & ex rom 1 / ex rom 0 & ex rom 1
	{ EXROM(1),		EXROM(0),		EXROM(1),		EXROM(0)	},	//0x08: ex rom 1 & ex rom 0 / ex rom 1 & ex rom 0
	{ EXROM(1),		BASICROM(1),	EXROM(1),		BASICROM(3) },	//0x09: ex rom 1 & basic rom 1 / ex rom 1 & basic 3
	{ BASICROM(0),  EXROM(1),		BASICROM(2),	EXROM(1)	},	//0x0a: basic rom 0 & ex rom 1 / basic rom 2 & ex rom 1
	{ EXROM(0),		KANJIROM(3),	EXROM(0),		KANJIROM(3) },	//0x0b: ex rom 0 & tv rom 2 / ex rom 0 & voice 1
	{ KANJIROM(2),	EXROM(0),		KANJIROM(2),	EXROM(0)	},	//0x0c: tv rom 1 & ex rom 0 / voice rom 0 & ex rom 0
	{ WRAM(0),		WRAM(1),		WRAM(2),		WRAM(3)		},	//0x0d: ram 0 & 1 / ram 2 & 3
	{ EXWRAM(0),	EXWRAM(1),		EXWRAM(2),		EXWRAM(3)	},	//0x0e: exram 0 & 1 / exram 2 & 3
	{ INVALID(0),	INVALID(0),		INVALID(0),		INVALID(0)  }	//0x0f: <invalid setting>
};

static const UINT32 banksw_table_r1[0x10*4][4] = {
	/* 0 */
	{ INVALID(0),	INVALID(0),		INVALID(0),     INVALID(0)  },	//0x00: <invalid setting>
	{ BASICROM(0),	BASICROM(1),	BASICROM(2),	BASICROM(3) },	//0x01: basic rom 0 & 1 / basic rom 2 & 3
	{ VOICEROM(0),	VOICEROM(1),	VOICEROM(0),	VOICEROM(1) },	//0x02: voice rom 0 & 1 / voice rom 0 & 1
	{ EXROM(1),		EXROM(1),		EXROM(1),		EXROM(1)	},	//0x03: ex rom 1 & 1 / ex rom 1 & 1
	{ EXROM(0),		EXROM(0),		EXROM(0),		EXROM(0)	},	//0x04: ex rom 0 & 0 / ex rom 0 & 0
	{ VOICEROM(0),	BASICROM(1),	VOICEROM(0),	BASICROM(3) },	//0x05: voice rom 0 & basic rom 1 / voice rom 0 & basic 3
	{ BASICROM(0),	VOICEROM(1),	BASICROM(2),	VOICEROM(1) },	//0x06: basic rom 0 & voice rom 1 / basic rom 2 & voice 1
	{ EXROM(0),		EXROM(1),		EXROM(0),		EXROM(1)	},	//0x07: ex rom 0 & ex rom 1 / ex rom 0 & ex rom 1
	{ EXROM(1),		EXROM(0),		EXROM(1),		EXROM(0)	},	//0x08: ex rom 1 & ex rom 0 / ex rom 1 & ex rom 0
	{ EXROM(1),		BASICROM(1),	EXROM(1),		BASICROM(3) },	//0x09: ex rom 1 & basic rom 1 / ex rom 1 & basic 3
	{ BASICROM(0),  EXROM(1),		BASICROM(2),	EXROM(1)	},	//0x0a: basic rom 0 & ex rom 1 / basic rom 2 & ex rom 1
	{ EXROM(0),		VOICEROM(1),	EXROM(0),		VOICEROM(1) },	//0x0b: ex rom 0 & voice rom 1 / ex rom 0 & voice 1
	{ VOICEROM(0),	EXROM(0),		VOICEROM(0),	EXROM(0)	},	//0x0c: voice rom 1 & ex rom 0 / voice rom 0 & ex rom 0
	{ WRAM(4),		WRAM(5),		WRAM(6),		WRAM(7)		},	//0x0d: ram 4 & 5 / ram 6 & 7
	{ EXWRAM(4),	EXWRAM(5),		EXWRAM(6),		EXWRAM(7)	},	//0x0e: exram 4 & 5 / exram 6 & 7
	{ INVALID(0),	INVALID(0),		INVALID(0),		INVALID(0)  },	//0x0f: <invalid setting>
	/* 1 */
	{ INVALID(0),	INVALID(0),		INVALID(0),     INVALID(0)  },	//0x00: <invalid setting>
	{ BASICROM(0),	BASICROM(1),	BASICROM(2),	BASICROM(3) },	//0x01: basic rom 0 & 1 / basic rom 2 & 3
	{ KANJIROM(0),	KANJIROM(1),	KANJIROM(0),	KANJIROM(1) },	//0x02: kanji rom 0 & 1 / kanji rom 0 & 1
	{ EXROM(1),		EXROM(1),		EXROM(1),		EXROM(1)	},	//0x03: ex rom 1 & 1 / ex rom 1 & 1
	{ EXROM(0),		EXROM(0),		EXROM(0),		EXROM(0)	},	//0x04: ex rom 0 & 0 / ex rom 0 & 0
	{ KANJIROM(0),	BASICROM(1),	KANJIROM(0),	BASICROM(3) },	//0x05: voice rom 0 & basic rom 1 / voice rom 0 & basic 3
	{ BASICROM(0),	KANJIROM(1),	BASICROM(2),	KANJIROM(1) },	//0x06: basic rom 0 & voice rom 1 / basic rom 2 & voice 1
	{ EXROM(0),		EXROM(1),		EXROM(0),		EXROM(1)	},	//0x07: ex rom 0 & ex rom 1 / ex rom 0 & ex rom 1
	{ EXROM(1),		EXROM(0),		EXROM(1),		EXROM(0)	},	//0x08: ex rom 1 & ex rom 0 / ex rom 1 & ex rom 0
	{ EXROM(1),		BASICROM(1),	EXROM(1),		BASICROM(3) },	//0x09: ex rom 1 & basic rom 1 / ex rom 1 & basic 3
	{ BASICROM(0),  EXROM(1),		BASICROM(2),	EXROM(1)	},	//0x0a: basic rom 0 & ex rom 1 / basic rom 2 & ex rom 1
	{ EXROM(0),		KANJIROM(1),	EXROM(0),		KANJIROM(1) },	//0x0b: ex rom 0 & voice rom 1 / ex rom 0 & voice 1
	{ KANJIROM(0),	EXROM(0),		KANJIROM(0),	EXROM(0)	},	//0x0c: voice rom 1 & ex rom 0 / voice rom 0 & ex rom 0
	{ WRAM(4),		WRAM(5),		WRAM(6),		WRAM(7)		},	//0x0d: ram 4 & 5 / ram 6 & 7
	{ EXWRAM(4),	EXWRAM(5),		EXWRAM(6),		EXWRAM(7)	},	//0x0e: exram 4 & 5 / exram 6 & 7
	{ INVALID(0),	INVALID(0),		INVALID(0),		INVALID(0)  },	//0x0f: <invalid setting>
	/* 2 */
	{ INVALID(0),	INVALID(0),		INVALID(0),     INVALID(0)  },	//0x00: <invalid setting>
	{ BASICROM(0),	BASICROM(1),	BASICROM(2),	BASICROM(3) },	//0x01: basic rom 0 & 1 / basic rom 2 & 3
	{ VOICEROM(0),	VOICEROM(1),	VOICEROM(0),	VOICEROM(1) },	//0x02: voice rom 0 & 1 / voice rom 0 & 1
	{ EXROM(1),		EXROM(1),		EXROM(1),		EXROM(1)	},	//0x03: ex rom 1 & 1 / ex rom 1 & 1
	{ EXROM(0),		EXROM(0),		EXROM(0),		EXROM(0)	},	//0x04: ex rom 0 & 0 / ex rom 0 & 0
	{ VOICEROM(0),	BASICROM(1),	VOICEROM(0),	BASICROM(3) },	//0x05: voice rom 0 & basic rom 1 / voice rom 0 & basic 3
	{ BASICROM(0),	VOICEROM(1),	BASICROM(2),	VOICEROM(1) },	//0x06: basic rom 0 & voice rom 1 / basic rom 2 & voice 1
	{ EXROM(0),		EXROM(1),		EXROM(0),		EXROM(1)	},	//0x07: ex rom 0 & ex rom 1 / ex rom 0 & ex rom 1
	{ EXROM(1),		EXROM(0),		EXROM(1),		EXROM(0)	},	//0x08: ex rom 1 & ex rom 0 / ex rom 1 & ex rom 0
	{ EXROM(1),		BASICROM(1),	EXROM(1),		BASICROM(3) },	//0x09: ex rom 1 & basic rom 1 / ex rom 1 & basic 3
	{ BASICROM(0),  EXROM(1),		BASICROM(2),	EXROM(1)	},	//0x0a: basic rom 0 & ex rom 1 / basic rom 2 & ex rom 1
	{ EXROM(0),		VOICEROM(1),	EXROM(0),		VOICEROM(1) },	//0x0b: ex rom 0 & voice rom 1 / ex rom 0 & voice 1
	{ VOICEROM(0),	EXROM(0),		VOICEROM(0),	EXROM(0)	},	//0x0c: voice rom 1 & ex rom 0 / voice rom 0 & ex rom 0
	{ WRAM(4),		WRAM(5),		WRAM(6),		WRAM(7)		},	//0x0d: ram 4 & 5 / ram 6 & 7
	{ EXWRAM(4),	EXWRAM(5),		EXWRAM(6),		EXWRAM(7)	},	//0x0e: exram 4 & 5 / exram 6 & 7
	{ INVALID(0),	INVALID(0),		INVALID(0),		INVALID(0)  },	//0x0f: <invalid setting>
	/* 3 */
	{ INVALID(0),	INVALID(0),		INVALID(0),     INVALID(0)  },	//0x00: <invalid setting>
	{ BASICROM(0),	BASICROM(1),	BASICROM(2),	BASICROM(3) },	//0x01: basic rom 0 & 1 / basic rom 2 & 3
	{ KANJIROM(2),	KANJIROM(3),	KANJIROM(2),	KANJIROM(3) },	//0x02: kanji rom 0 & 1 / kanji rom 0 & 1
	{ EXROM(1),		EXROM(1),		EXROM(1),		EXROM(1)	},	//0x03: ex rom 1 & 1 / ex rom 1 & 1
	{ EXROM(0),		EXROM(0),		EXROM(0),		EXROM(0)	},	//0x04: ex rom 0 & 0 / ex rom 0 & 0
	{ KANJIROM(2),	BASICROM(1),	KANJIROM(2),	BASICROM(3) },	//0x05: voice rom 0 & basic rom 1 / voice rom 0 & basic 3
	{ BASICROM(0),	KANJIROM(3),	BASICROM(2),	KANJIROM(3) },	//0x06: basic rom 0 & voice rom 1 / basic rom 2 & voice 1
	{ EXROM(0),		EXROM(1),		EXROM(0),		EXROM(1)	},	//0x07: ex rom 0 & ex rom 1 / ex rom 0 & ex rom 1
	{ EXROM(1),		EXROM(0),		EXROM(1),		EXROM(0)	},	//0x08: ex rom 1 & ex rom 0 / ex rom 1 & ex rom 0
	{ EXROM(1),		BASICROM(1),	EXROM(1),		BASICROM(3) },	//0x09: ex rom 1 & basic rom 1 / ex rom 1 & basic 3
	{ BASICROM(0),  EXROM(1),		BASICROM(2),	EXROM(1)	},	//0x0a: basic rom 0 & ex rom 1 / basic rom 2 & ex rom 1
	{ EXROM(0),		KANJIROM(3),	EXROM(0),		KANJIROM(3) },	//0x0b: ex rom 0 & voice rom 1 / ex rom 0 & voice 1
	{ KANJIROM(2),	EXROM(0),		KANJIROM(2),	EXROM(0)	},	//0x0c: voice rom 1 & ex rom 0 / voice rom 0 & ex rom 0
	{ WRAM(4),		WRAM(5),		WRAM(6),		WRAM(7)		},	//0x0d: ram 4 & 5 / ram 6 & 7
	{ EXWRAM(4),	EXWRAM(5),		EXWRAM(6),		EXWRAM(7)	},	//0x0e: exram 4 & 5 / exram 6 & 7
	{ INVALID(0),	INVALID(0),		INVALID(0),		INVALID(0)  }	//0x0f: <invalid setting>
};

WRITE8_MEMBER(pc6001_state::pc6001m2_bank_r0_w)
{
	UINT8 *ROM = memregion("maincpu")->base();
	UINT8 *gfx_data = memregion("gfx1")->base();

//  bankaddress = 0x10000 + (0x4000 * ((data & 0x40)>>6));
//  membank(1)->set_base(&ROM[bankaddress]);

	m_bank_r0 = data;

//  printf("%02x BANK | %02x\n",data,m_bank_opt);
	membank("bank1")->set_base(&ROM[banksw_table_r0[(data & 0xf)+(m_bank_opt*0x10)][0]]);
	membank("bank2")->set_base(&ROM[banksw_table_r0[(data & 0xf)+(m_bank_opt*0x10)][1]]);
	membank("bank3")->set_base(&ROM[banksw_table_r0[((data & 0xf0)>>4)+(m_bank_opt*0x10)][2]]);
	if(!m_gfx_bank_on)
		membank("bank4")->set_base(&ROM[banksw_table_r0[((m_bank_r0 & 0xf0)>>4)+(m_bank_opt*0x10)][3]]);
	else
		membank("bank4")->set_base(&gfx_data[m_cgrom_bank_addr]);
}

WRITE8_MEMBER(pc6001_state::pc6001m2_bank_r1_w)
{
	UINT8 *ROM = memregion("maincpu")->base();

//  bankaddress = 0x10000 + (0x4000 * ((data & 0x40)>>6));
//  membank(1)->set_base(&ROM[bankaddress]);

	m_bank_r1 = data;

//  printf("%02x BANK\n",data);
	membank("bank5")->set_base(&ROM[banksw_table_r1[(data & 0xf)+(m_bank_opt*0x10)][0]]);
	membank("bank6")->set_base(&ROM[banksw_table_r1[(data & 0xf)+(m_bank_opt*0x10)][1]]);
	membank("bank7")->set_base(&ROM[banksw_table_r1[((data & 0xf0)>>4)+(m_bank_opt*0x10)][2]]);
	membank("bank8")->set_base(&ROM[banksw_table_r1[((data & 0xf0)>>4)+(m_bank_opt*0x10)][3]]);
}

WRITE8_MEMBER(pc6001_state::pc6001m2_bank_w0_w)
{
	m_bank_w = data;
}

WRITE8_MEMBER(pc6001_state::pc6001m2_opt_bank_w)
{
	UINT8 *ROM = memregion("maincpu")->base();
	UINT8 *gfx_data = memregion("gfx1")->base();

	/*
    0 - TVROM / VOICE ROM
    1 - KANJI ROM bank 0
    2 - KANJI ROM bank 1
    3 - TVROM / VOICE ROM
    */
	m_bank_opt = data & 3;

	membank("bank1")->set_base(&ROM[banksw_table_r0[(m_bank_r0 & 0xf)+(m_bank_opt*0x10)][0]]);
	membank("bank2")->set_base(&ROM[banksw_table_r0[(m_bank_r0 & 0xf)+(m_bank_opt*0x10)][1]]);
	membank("bank3")->set_base(&ROM[banksw_table_r0[((m_bank_r0 & 0xf0)>>4)+(m_bank_opt*0x10)][2]]);
	if(!m_gfx_bank_on)
		membank("bank4")->set_base(&ROM[banksw_table_r0[((m_bank_r0 & 0xf0)>>4)+(m_bank_opt*0x10)][3]]);
	else
		membank("bank4")->set_base(&gfx_data[m_cgrom_bank_addr]);
	membank("bank4")->set_base(&ROM[banksw_table_r0[((m_bank_r0 & 0xf0)>>4)+(m_bank_opt*0x10)][3]]);
	membank("bank5")->set_base(&ROM[banksw_table_r1[(m_bank_r1 & 0xf)+(m_bank_opt*0x10)][0]]);
	membank("bank6")->set_base(&ROM[banksw_table_r1[(m_bank_r1 & 0xf)+(m_bank_opt*0x10)][1]]);
	membank("bank7")->set_base(&ROM[banksw_table_r1[((m_bank_r1 & 0xf0)>>4)+(m_bank_opt*0x10)][2]]);
	membank("bank8")->set_base(&ROM[banksw_table_r1[((m_bank_r1 & 0xf0)>>4)+(m_bank_opt*0x10)][3]]);

}

WRITE8_MEMBER(pc6001_state::work_ram0_w)
{
	UINT8 *ROM = memregion("maincpu")->base();
	ROM[offset+((m_bank_w & 0x01) ? WRAM(0) : EXWRAM(0))] = data;
}

WRITE8_MEMBER(pc6001_state::work_ram1_w)
{
	UINT8 *ROM = memregion("maincpu")->base();
	ROM[offset+((m_bank_w & 0x01) ? WRAM(1) : EXWRAM(1))] = data;
}

WRITE8_MEMBER(pc6001_state::work_ram2_w)
{
	UINT8 *ROM = memregion("maincpu")->base();
	ROM[offset+((m_bank_w & 0x04) ? WRAM(2) : EXWRAM(2))] = data;
}

WRITE8_MEMBER(pc6001_state::work_ram3_w)
{
	UINT8 *ROM = memregion("maincpu")->base();
	ROM[offset+((m_bank_w & 0x04) ? WRAM(3) : EXWRAM(3))] = data;
}

WRITE8_MEMBER(pc6001_state::work_ram4_w)
{
	UINT8 *ROM = memregion("maincpu")->base();
	ROM[offset+((m_bank_w & 0x10) ? WRAM(4) : EXWRAM(4))] = data;
}

WRITE8_MEMBER(pc6001_state::work_ram5_w)
{
	UINT8 *ROM = memregion("maincpu")->base();
	ROM[offset+((m_bank_w & 0x10) ? WRAM(5) : EXWRAM(5))] = data;
}

WRITE8_MEMBER(pc6001_state::work_ram6_w)
{
	UINT8 *ROM = memregion("maincpu")->base();
	ROM[offset+((m_bank_w & 0x40) ? WRAM(6) : EXWRAM(6))] = data;
}

WRITE8_MEMBER(pc6001_state::work_ram7_w)
{
	UINT8 *ROM = memregion("maincpu")->base();
	ROM[offset+((m_bank_w & 0x40) ? WRAM(7) : EXWRAM(7))] = data;
}


WRITE8_MEMBER(pc6001_state::necmk2_ppi8255_w)
{
	if (offset==3)
	{
		if(data & 1)
			m_port_c_8255 |=   1<<((data>>1)&0x07);
		else
			m_port_c_8255 &= ~(1<<((data>>1)&0x07));

		switch(data) {
        	case 0x08: m_port_c_8255 |= 0x88; break;
        	case 0x09: m_port_c_8255 &= 0xf7; break;
        	case 0x0c: m_port_c_8255 |= 0x28; break;
        	case 0x0d: m_port_c_8255 &= 0xf7; break;
        	default: break;
		}

		m_port_c_8255 |= 0xa8;

		{
			UINT8 *ROM = memregion("maincpu")->base();
			UINT8 *gfx_data = memregion("gfx1")->base();

			//printf("%02x\n",data);

			if((data & 0x0f) == 0x05)
			{
				m_gfx_bank_on = 0;
				membank("bank4")->set_base(&ROM[banksw_table_r0[((m_bank_r0 & 0xf0)>>4)+(m_bank_opt*0x10)][3]]);
			}
			if((data & 0x0f) == 0x04)
			{
				m_gfx_bank_on = 1;
				membank("bank4")->set_base(&gfx_data[m_cgrom_bank_addr]);
			}
		}
	}
	m_ppi->write(space,offset,data);
}

static void vram_bank_change(running_machine &machine,UINT8 vram_bank)
{
	pc6001_state *state = machine.driver_data<pc6001_state>();
	UINT8 *work_ram = state->memregion("maincpu")->base();

//  popmessage("%02x",vram_bank);

	switch(vram_bank & 0x66)
	{
		case 0x00: state->m_video_ram = work_ram + 0x8000 + 0x28000; break; //4 color mode
		case 0x02: state->m_video_ram = work_ram + 0xc000 + 0x28000; break;
		case 0x04: state->m_video_ram = work_ram + 0x8000 + 0x28000; break;
		case 0x06: state->m_video_ram = work_ram + 0xc000 + 0x28000; break;
		case 0x20: state->m_video_ram = work_ram + 0xc000 + 0x28000; break; //4 color mode
		case 0x22: state->m_video_ram = work_ram + 0xe000 + 0x28000; break;
		case 0x24: state->m_video_ram = work_ram + 0xc000 + 0x28000; break;
		case 0x26: state->m_video_ram = work_ram + 0xe000 + 0x28000; break;
		case 0x40: state->m_video_ram = work_ram + 0x0000 + 0x28000; break; //4 color mode
		case 0x42: state->m_video_ram = work_ram + 0x8000 + 0x28000; break;
		case 0x44: state->m_video_ram = work_ram + 0x0000 + 0x28000; break;
		case 0x46: state->m_video_ram = work_ram + 0x8000 + 0x28000; break;
		case 0x60: state->m_video_ram = work_ram + 0x4000 + 0x28000; break; //4 color mode
		case 0x62: state->m_video_ram = work_ram + 0xa000 + 0x28000; break;
		case 0x64: state->m_video_ram = work_ram + 0x4000 + 0x28000; break;
		case 0x66: state->m_video_ram = work_ram + 0xa000 + 0x28000; break;
	}
}

WRITE8_MEMBER(pc6001_state::pc6001m2_system_latch_w)
{
	if((!(m_sys_latch & 8)) && data & 0x8) //PLAY tape cmd
	{
		m_cas_switch = 1;
		//machine().device(CASSETTE_TAG )->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
		//machine().device(CASSETTE_TAG )->change_state(CASSETTE_PLAY,CASSETTE_MASK_UISTATE);
	}
	if((m_sys_latch & 8) && ((data & 0x8) == 0)) //STOP tape cmd
	{
		m_cas_switch = 0;
		//machine().device(CASSETTE_TAG )->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
		//machine().device(CASSETTE_TAG )->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
		//m_irq_vector = 0x00;
		//machine().device("maincpu")->execute().set_input_line(0, ASSERT_LINE);
	}

	m_sys_latch = data;

	m_timer_irq_mask = data & 1;
	vram_bank_change(machine(),(m_ex_vram_bank & 0x06) | ((m_sys_latch & 0x06) << 4));

	//printf("%02x B0\n",data);
}


WRITE8_MEMBER(pc6001_state::pc6001m2_vram_bank_w)
{
	//static const UINT32 startaddr[] = {WRAM(6), WRAM(6), WRAM(0), WRAM(4) };

	m_ex_vram_bank = data;
	vram_bank_change(machine(),(m_ex_vram_bank & 0x06) | ((m_sys_latch & 0x06) << 4));

	m_exgfx_text_mode = ((data & 2) == 0);
	m_cgrom_bank_addr = (data & 2) ? 0x0000 : 0x2000;

	m_exgfx_bitmap_mode = (data & 8);
	m_exgfx_2bpp_mode = ((data & 6) == 0);

	{
		/* Apparently bitmap modes changes the screen res to 320 x 200 */
		{
			rectangle visarea = machine().primary_screen->visible_area();
			int y_height;

			y_height = (m_exgfx_bitmap_mode || m_exgfx_2bpp_mode) ? 200 : 240;

			visarea.set(0, (320) - 1, 0, (y_height) - 1);

			machine().primary_screen->configure(320, 240, visarea, machine().primary_screen->frame_period().attoseconds);
		}
	}

//  popmessage("%02x",data);

//  m_video_ram = work_ram + startaddr[(data >> 1) & 0x03];
}

WRITE8_MEMBER(pc6001_state::pc6001m2_col_bank_w)
{
	m_bgcol_bank = (data & 7);
}

/* voice synth is a NEC uPD7752 sound chip (currently unemulated) */
READ8_MEMBER(pc6001_state::upd7752_reg_r)
{
	switch(offset & 3)
	{
		//[0x00]: status register
		//x--- ---- BSY busy status (1) processing (0) stopped
		//-x-- ---- REQ audio parameter (1) input request (0) prohibited (???)
		//--x- ---- ~INT / EXT message data (1) Outside (0) Inside
		//---x ---- ERR error flag
		case 0x00: return 0x60;
		//[0x02]: port 0xe2 latch?
		case 0x02: return 0xff;
		//[0x03]: port 0xe3 latch?
		case 0x03: return 0xff;
	}
	return 0xff;
}

WRITE8_MEMBER(pc6001_state::upd7752_reg_w)
{
	switch(offset & 3)
	{
		// [0x00]: audio parameter transfer

		// [0x02]: mode set
		// ---- -x-- Frame periodic analysis (0) 10 ms / frame (1) 20 ms / frame
		// ---- --xx Utterance (tempo?) speed
		//        00 : NORMAL SPEED
		//        01 : SLOW SPEED
		//        10 : FAST SPEED
		//        11 : Setting prohibited

		// case 0x02:

		// case 0x03: command set
	}
}

WRITE8_MEMBER(pc6001_state::pc6001m2_0xf3_w)
{
	/*
    x--- ---- M1 (?) wait setting
    -x-- ---- ROM wait setting
    --x- ---- RAM wait setting
    ---x ---- custom irq 2 address output
    ---- x--- custom irq 1 address output
    ---- -x-- timer irq mask 2 (mirror?)
    ---- --x- custom irq 2 mask
    ---- ---x custom irq 1 mask
    */
	m_timer_irq_mask2 = data & 4;
}

static TIMER_CALLBACK(audio_callback)
{
	pc6001_state *state = machine.driver_data<pc6001_state>();
	if(state->m_cas_switch == 0 && ((state->m_timer_irq_mask == 0) || (state->m_timer_irq_mask2 == 0)))
	{
		if(IRQ_LOG) printf("Timer IRQ called %02x\n",state->m_timer_irq_vector);
		state->m_irq_vector = state->m_timer_irq_vector;
		machine.device("maincpu")->execute().set_input_line(0, ASSERT_LINE);
	}
}


WRITE8_MEMBER(pc6001_state::pc6001m2_timer_adj_w)
{
	m_timer_hz_div = data;
	attotime period = attotime::from_hz((487.5*4)/(m_timer_hz_div+1));
	m_timer_irq_timer->adjust(period,  0, period);
}

WRITE8_MEMBER(pc6001_state::pc6001m2_timer_irqv_w)
{
	m_timer_irq_vector = data;
}

READ8_MEMBER(pc6001_state::pc6001m2_bank_r0_r)
{
	return m_bank_r0;
}

READ8_MEMBER(pc6001_state::pc6001m2_bank_r1_r)
{
	return m_bank_r1;
}

READ8_MEMBER(pc6001_state::pc6001m2_bank_w0_r)
{
	return m_bank_w;
}

static ADDRESS_MAP_START(pc6001m2_map, AS_PROGRAM, 8, pc6001_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROMBANK("bank1") AM_WRITE(work_ram0_w)
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank2") AM_WRITE(work_ram1_w)
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank3") AM_WRITE(work_ram2_w)
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank4") AM_WRITE(work_ram3_w)
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank5") AM_WRITE(work_ram4_w)
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank6") AM_WRITE(work_ram5_w)
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("bank7") AM_WRITE(work_ram6_w)
	AM_RANGE(0xe000, 0xffff) AM_ROMBANK("bank8") AM_WRITE(work_ram7_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc6001m2_io , AS_IO, 8, pc6001_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x80) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE(0x81, 0x81) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)

	AM_RANGE(0x90, 0x93) AM_MIRROR(0x0c) AM_READWRITE(nec_ppi8255_r, necmk2_ppi8255_w)

	AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x0c) AM_DEVWRITE_LEGACY("ay8910", ay8910_address_w)
	AM_RANGE(0xa1, 0xa1) AM_MIRROR(0x0c) AM_DEVWRITE_LEGACY("ay8910", ay8910_data_w)
	AM_RANGE(0xa2, 0xa2) AM_MIRROR(0x0c) AM_DEVREAD_LEGACY("ay8910", ay8910_r)
	AM_RANGE(0xa3, 0xa3) AM_MIRROR(0x0c) AM_NOP

	AM_RANGE(0xb0, 0xb0) AM_MIRROR(0x0f) AM_WRITE(pc6001m2_system_latch_w)

	AM_RANGE(0xc0, 0xc0) AM_WRITE(pc6001m2_col_bank_w)
	AM_RANGE(0xc1, 0xc1) AM_WRITE(pc6001m2_vram_bank_w)
	AM_RANGE(0xc2, 0xc2) AM_WRITE(pc6001m2_opt_bank_w)

	AM_RANGE(0xd0, 0xd3) AM_MIRROR(0x0c) AM_NOP // disk device

	AM_RANGE(0xe0, 0xe3) AM_MIRROR(0x0c) AM_READWRITE(upd7752_reg_r,upd7752_reg_w)

	AM_RANGE(0xf0, 0xf0) AM_READWRITE(pc6001m2_bank_r0_r,pc6001m2_bank_r0_w)
	AM_RANGE(0xf1, 0xf1) AM_READWRITE(pc6001m2_bank_r1_r,pc6001m2_bank_r1_w)
	AM_RANGE(0xf2, 0xf2) AM_READWRITE(pc6001m2_bank_w0_r,pc6001m2_bank_w0_w)
	AM_RANGE(0xf3, 0xf3) AM_WRITE(pc6001m2_0xf3_w)
//  AM_RANGE(0xf4
//  AM_RANGE(0xf5
	AM_RANGE(0xf6, 0xf6) AM_WRITE(pc6001m2_timer_adj_w)
	AM_RANGE(0xf7, 0xf7) AM_WRITE(pc6001m2_timer_irqv_w)
ADDRESS_MAP_END

/* disk device placeholder (TODO: identify & hook-up this) */
READ8_MEMBER(pc6001_state::pc6601_fdc_r)
{
	return machine().rand();
}

WRITE8_MEMBER(pc6001_state::pc6601_fdc_w)
{
}

static ADDRESS_MAP_START( pc6601_io , AS_IO, 8, pc6001_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x80) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE(0x81, 0x81) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)

	AM_RANGE(0x90, 0x93) AM_MIRROR(0x0c) AM_READWRITE(nec_ppi8255_r, necmk2_ppi8255_w)

	AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x0c) AM_DEVWRITE_LEGACY("ay8910", ay8910_address_w)
	AM_RANGE(0xa1, 0xa1) AM_MIRROR(0x0c) AM_DEVWRITE_LEGACY("ay8910", ay8910_data_w)
	AM_RANGE(0xa2, 0xa2) AM_MIRROR(0x0c) AM_DEVREAD_LEGACY("ay8910", ay8910_r)
	AM_RANGE(0xa3, 0xa3) AM_MIRROR(0x0c) AM_NOP

	AM_RANGE(0xb0, 0xb0) AM_WRITE(pc6001m2_system_latch_w)
	/* these are disk related */
//  AM_RANGE(0xb1
//  AM_RANGE(0xb2
//  AM_RANGE(0xb3

	AM_RANGE(0xc0, 0xc0) AM_WRITE(pc6001m2_col_bank_w)
	AM_RANGE(0xc1, 0xc1) AM_WRITE(pc6001m2_vram_bank_w)
	AM_RANGE(0xc2, 0xc2) AM_WRITE(pc6001m2_opt_bank_w)

	AM_RANGE(0xd0, 0xdf) AM_READWRITE(pc6601_fdc_r,pc6601_fdc_w) // disk device

	AM_RANGE(0xe0, 0xe3) AM_MIRROR(0x0c) AM_READWRITE(upd7752_reg_r,upd7752_reg_w)

	AM_RANGE(0xf0, 0xf0) AM_READWRITE(pc6001m2_bank_r0_r,pc6001m2_bank_r0_w)
	AM_RANGE(0xf1, 0xf1) AM_READWRITE(pc6001m2_bank_r1_r,pc6001m2_bank_r1_w)
	AM_RANGE(0xf2, 0xf2) AM_READWRITE(pc6001m2_bank_w0_r,pc6001m2_bank_w0_w)
	AM_RANGE(0xf3, 0xf3) AM_WRITE(pc6001m2_0xf3_w)
//  AM_RANGE(0xf4
//  AM_RANGE(0xf5
	AM_RANGE(0xf6, 0xf6) AM_WRITE(pc6001m2_timer_adj_w)
	AM_RANGE(0xf7, 0xf7) AM_WRITE(pc6001m2_timer_irqv_w)
ADDRESS_MAP_END

/* PC-6001 SR */

#define SR_SYSROM_1(_v_) \
	0x10000+(0x1000*_v_) \

#define SR_SYSROM_2(_v_) \
	0x20000+(0x1000*_v_) \

#define SR_CGROM1(_v_) \
	0x30000+(0x1000*_v_) \

#define SR_EXROM0(_v_) \
	0x40000+(0x1000*_v_) \

#define SR_EXROM1(_v_) \
	0x50000+(0x1000*_v_) \

#define SR_EXRAM0(_v_) \
	0x60000+(0x1000*_v_) \

#define SR_WRAM0(_v_) \
	0x70000+(0x1000*_v_) \

#define SR_NULL(_v_) \
	0x80000+(0x1000*_v_) \

READ8_MEMBER(pc6001_state::pc6001sr_bank_rn_r)
{
	return m_sr_bank_r[offset];
}

WRITE8_MEMBER(pc6001_state::pc6001sr_bank_rn_w)
{
	static const char *const bank_name[8] = { "bank1","bank2","bank3", "bank4", "bank5", "bank6", "bank7", "bank8" };
	UINT8 *ROM = memregion("maincpu")->base();
	UINT8 bank_num;

	m_sr_bank_r[offset] = data;
	bank_num = data & 0x0f;

	switch(data & 0xf0)
	{
		case 0xf0: membank(bank_name[offset])->set_base(&ROM[SR_SYSROM_1(bank_num)]); break;
		case 0xe0: membank(bank_name[offset])->set_base(&ROM[SR_SYSROM_2(bank_num)]); break;
		case 0xd0: membank(bank_name[offset])->set_base(&ROM[SR_CGROM1(bank_num)]); break;
		case 0xc0: membank(bank_name[offset])->set_base(&ROM[SR_EXROM0(bank_num)]); break;
		case 0xb0: membank(bank_name[offset])->set_base(&ROM[SR_EXROM1(bank_num)]); break;
		case 0x20: membank(bank_name[offset])->set_base(&ROM[SR_EXRAM0(bank_num)]); break;
		case 0x00: membank(bank_name[offset])->set_base(&ROM[SR_WRAM0(bank_num)]); break;
		default:   membank(bank_name[offset])->set_base(&ROM[SR_NULL(bank_num)]); break;
	}
}

READ8_MEMBER(pc6001_state::pc6001sr_bank_wn_r)
{
	return m_sr_bank_w[offset];
}

WRITE8_MEMBER(pc6001_state::pc6001sr_bank_wn_w)
{
	m_sr_bank_w[offset] = data;
}

#define SR_WRAM_BANK_W(_v_) \
{ \
	UINT8 *ROM = memregion("maincpu")->base(); \
	UINT8 bank_num; \
	bank_num = m_sr_bank_w[_v_] & 0x0f; \
	if((m_sr_bank_w[_v_] & 0xf0) != 0x20) \
		ROM[offset+(SR_WRAM0(bank_num))] = data; \
	else \
		ROM[offset+(SR_EXRAM0(bank_num))] = data; \
} \

WRITE8_MEMBER(pc6001_state::sr_work_ram0_w){ SR_WRAM_BANK_W(0); }
WRITE8_MEMBER(pc6001_state::sr_work_ram1_w){ SR_WRAM_BANK_W(1); }
WRITE8_MEMBER(pc6001_state::sr_work_ram2_w){ SR_WRAM_BANK_W(2); }
WRITE8_MEMBER(pc6001_state::sr_work_ram3_w){ SR_WRAM_BANK_W(3); }
WRITE8_MEMBER(pc6001_state::sr_work_ram4_w){ SR_WRAM_BANK_W(4); }
WRITE8_MEMBER(pc6001_state::sr_work_ram5_w){ SR_WRAM_BANK_W(5); }
WRITE8_MEMBER(pc6001_state::sr_work_ram6_w){ SR_WRAM_BANK_W(6); }
WRITE8_MEMBER(pc6001_state::sr_work_ram7_w){ SR_WRAM_BANK_W(7); }

WRITE8_MEMBER(pc6001_state::pc6001sr_mode_w)
{
	m_sr_video_mode = data;

	if(data & 1)
		assert("PC-6001SR in Mk-2 compatibility mode not yet supported!\n");
}

WRITE8_MEMBER(pc6001_state::pc6001sr_vram_bank_w)
{
	UINT8 *work_ram = memregion("maincpu")->base();

	m_video_ram = work_ram + 0x70000 + ((data & 0x0f)*0x1000);
}

WRITE8_MEMBER(pc6001_state::pc6001sr_system_latch_w)
{
	if((!(m_sys_latch & 8)) && data & 0x8) //PLAY tape cmd
	{
		m_cas_switch = 1;
		//machine().device(CASSETTE_TAG )->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
		//machine().device(CASSETTE_TAG )->change_state(CASSETTE_PLAY,CASSETTE_MASK_UISTATE);
	}
	if((m_sys_latch & 8) && ((data & 0x8) == 0)) //STOP tape cmd
	{
		m_cas_switch = 0;
		//machine().device(CASSETTE_TAG )->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
		//machine().device(CASSETTE_TAG )->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
		//m_irq_vector = 0x00;
		//machine().device("maincpu")->execute().set_input_line(0, ASSERT_LINE);
	}

	m_sys_latch = data;

	m_timer_irq_mask = data & 1;
	//vram_bank_change(machine(),(m_ex_vram_bank & 0x06) | ((m_sys_latch & 0x06) << 4));

	//printf("%02x B0\n",data);
}

WRITE8_MEMBER(pc6001_state::necsr_ppi8255_w)
{
	if (offset==3)
	{
		if(data & 1)
			m_port_c_8255 |=   1<<((data>>1)&0x07);
		else
			m_port_c_8255 &= ~(1<<((data>>1)&0x07));

		switch(data) {
        	case 0x08: m_port_c_8255 |= 0x88; break;
        	case 0x09: m_port_c_8255 &= 0xf7; break;
        	case 0x0c: m_port_c_8255 |= 0x28; break;
        	case 0x0d: m_port_c_8255 &= 0xf7; break;
        	default: break;
		}

		m_port_c_8255 |= 0xa8;

		if(0)
		{
			UINT8 *gfx_data = memregion("gfx1")->base();
			UINT8 *ext_rom = memregion("cart_img")->base();

			//printf("%02x\n",data);

			if((data & 0x0f) == 0x05)
				membank("bank1")->set_base(&ext_rom[0x2000]);
			if((data & 0x0f) == 0x04)
				membank("bank1")->set_base(&gfx_data[0]);
		}
	}
	m_ppi->write(space,offset,data);
}


static ADDRESS_MAP_START(pc6001sr_map, AS_PROGRAM, 8, pc6001_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROMBANK("bank1") AM_WRITE(sr_work_ram0_w)
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank2") AM_WRITE(sr_work_ram1_w)
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank3") AM_WRITE(sr_work_ram2_w)
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank4") AM_WRITE(sr_work_ram3_w)
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank5") AM_WRITE(sr_work_ram4_w)
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank6") AM_WRITE(sr_work_ram5_w)
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("bank7") AM_WRITE(sr_work_ram6_w)
	AM_RANGE(0xe000, 0xffff) AM_ROMBANK("bank8") AM_WRITE(sr_work_ram7_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc6001sr_io , AS_IO, 8, pc6001_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x60, 0x67) AM_READWRITE(pc6001sr_bank_rn_r,pc6001sr_bank_rn_w)
	AM_RANGE(0x68, 0x6f) AM_READWRITE(pc6001sr_bank_wn_r,pc6001sr_bank_wn_w)
	AM_RANGE(0x80, 0x80) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE(0x81, 0x81) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)

	AM_RANGE(0x90, 0x93) AM_MIRROR(0x0c) AM_READWRITE(nec_ppi8255_r, necsr_ppi8255_w)

	AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x0c) AM_DEVWRITE_LEGACY("ay8910", ay8910_address_w)
	AM_RANGE(0xa1, 0xa1) AM_MIRROR(0x0c) AM_DEVWRITE_LEGACY("ay8910", ay8910_data_w)
	AM_RANGE(0xa2, 0xa2) AM_MIRROR(0x0c) AM_DEVREAD_LEGACY("ay8910", ay8910_r)
	AM_RANGE(0xa3, 0xa3) AM_MIRROR(0x0c) AM_NOP

	AM_RANGE(0xb0, 0xb0) AM_WRITE(pc6001sr_system_latch_w)
	/* these are disk related */
//  AM_RANGE(0xb1
//  AM_RANGE(0xb2
//  AM_RANGE(0xb3

//  AM_RANGE(0xc0, 0xc0) AM_WRITE(pc6001m2_col_bank_w)
//  AM_RANGE(0xc1, 0xc1) AM_WRITE(pc6001m2_vram_bank_w)
//  AM_RANGE(0xc2, 0xc2) AM_WRITE(pc6001m2_opt_bank_w)

	AM_RANGE(0xc8, 0xc8) AM_WRITE(pc6001sr_mode_w)
	AM_RANGE(0xc9, 0xc9) AM_WRITE(pc6001sr_vram_bank_w)

	AM_RANGE(0xd0, 0xdf) AM_READWRITE(pc6601_fdc_r,pc6601_fdc_w) // disk device

	AM_RANGE(0xe0, 0xe3) AM_MIRROR(0x0c) AM_READWRITE(upd7752_reg_r,upd7752_reg_w)

//  AM_RANGE(0xf0, 0xf0) AM_READWRITE(pc6001m2_bank_r0_r,pc6001m2_bank_r0_w)
//  AM_RANGE(0xf1, 0xf1) AM_READWRITE(pc6001m2_bank_r1_r,pc6001m2_bank_r1_w)
//  AM_RANGE(0xf2, 0xf2) AM_READWRITE(pc6001m2_bank_w0_r,pc6001m2_bank_w0_w)
	AM_RANGE(0xf3, 0xf3) AM_WRITE(pc6001m2_0xf3_w)
//  AM_RANGE(0xf4
//  AM_RANGE(0xf5
	AM_RANGE(0xf6, 0xf6) AM_WRITE(pc6001m2_timer_adj_w)
	AM_RANGE(0xf7, 0xf7) AM_WRITE(pc6001m2_timer_irqv_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( pc6001 )
	PORT_START("MODE4_DSW") //TODO: is this really a DSW? bit arrangement is also unknown if so.
	PORT_DIPNAME( 0x07, 0x00, "Mode 4 GFX colors" )
	PORT_DIPSETTING(    0x00, "Monochrome" )
	PORT_DIPSETTING(    0x01, "Red/Blue" )
	PORT_DIPSETTING(    0x02, "Blue/Red" )
	PORT_DIPSETTING(    0x03, "Pink/Green" )
	PORT_DIPSETTING(    0x04, "Green/Pink" )
	//5-6-7 is presumably invalid

	/* TODO: these two are unchecked */
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("key1") //0x00-0x1f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_UNUSED) //0x00 null
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_UNUSED) //0x01 soh
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_UNUSED) //0x02 stx
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_UNUSED) //0x03 etx
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_UNUSED) //0x04 etx
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_UNUSED) //0x05 eot
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_UNUSED) //0x06 enq
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_UNUSED) //0x07 ack
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0a
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0b lf
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0c vt
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(27)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0e cr
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0f so

	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x10 si
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x11 dle
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x12 dc1
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x13 dc2
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x14 dc3
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x15 dc4
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x16 nak
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x17 syn
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x18 etb
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x19 cancel
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x1a em
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x1b sub
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(27)
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x1d fs
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x1e gs
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x1f us

	PORT_START("key2") //0x20-0x3f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_UNUSED) //0x21 !
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_UNUSED) //0x22 "
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_UNUSED) //0x23 #
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_UNUSED) //0x24 $
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_UNUSED) //0x25 %
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_UNUSED) //0x26 &
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_UNUSED) //0x27 '
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_UNUSED) //0x28 (
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_UNUSED) //0x29 )
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('+')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') //0x2c
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') //0x2e
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') //0x2f

	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3a :
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3b ;
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3c <
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3d =
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3e >
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3f ?

	PORT_START("key3") //0x40-0x5f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']')
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^')
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("_")

	PORT_START("key_modifiers")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("KANA") PORT_CODE(KEYCODE_RCONTROL) PORT_TOGGLE
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CAPS") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("GRPH") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("STOP") PORT_CODE(KEYCODE_ESC)
INPUT_PORTS_END

static UINT8 check_joy_press(running_machine &machine);

static INTERRUPT_GEN( pc6001_interrupt )
{
	pc6001_state *state = device->machine().driver_data<pc6001_state>();
	state->m_cur_keycode = check_joy_press(device->machine());
	if(IRQ_LOG) printf("Stick IRQ called 0x16\n");
	state->m_irq_vector = 0x16;
	device->execute().set_input_line(0, ASSERT_LINE);
}

static INTERRUPT_GEN( pc6001sr_interrupt )
{
	pc6001_state *state = device->machine().driver_data<pc6001_state>();
	state->m_kludge^= 1;

	state->m_cur_keycode = check_joy_press(device->machine());
	if(IRQ_LOG) printf("VRTC IRQ called 0x16\n");
	state->m_irq_vector = (state->m_kludge) ? 0x22 : 0x16;
	device->execute().set_input_line(0, ASSERT_LINE);
}

static IRQ_CALLBACK ( pc6001_irq_callback )
{
	pc6001_state *state = device->machine().driver_data<pc6001_state>();
	device->execute().set_input_line(0, CLEAR_LINE);
	return state->m_irq_vector;
}

static READ8_DEVICE_HANDLER (pc6001_8255_porta_r )
{
	return 0;
}

static WRITE8_DEVICE_HANDLER (pc6001_8255_porta_w )
{
//  if(data != 0x06)
//      printf("pc6001_8255_porta_w %02x\n",data);
}

static READ8_DEVICE_HANDLER (pc6001_8255_portb_r )
{
	return 0;
}

static WRITE8_DEVICE_HANDLER (pc6001_8255_portb_w )
{
	//printf("pc6001_8255_portb_w %02x\n",data);
}

static WRITE8_DEVICE_HANDLER (pc6001_8255_portc_w )
{
	//printf("pc6001_8255_portc_w %02x\n",data);
}

static READ8_DEVICE_HANDLER (pc6001_8255_portc_r )
{
	return 0x88;
}



static I8255_INTERFACE( pc6001_ppi8255_interface )
{
	DEVCB_HANDLER(pc6001_8255_porta_r),
	DEVCB_HANDLER(pc6001_8255_porta_w),
	DEVCB_HANDLER(pc6001_8255_portb_r),
	DEVCB_HANDLER(pc6001_8255_portb_w),
	DEVCB_HANDLER(pc6001_8255_portc_r),
	DEVCB_HANDLER(pc6001_8255_portc_w)
};

static const i8251_interface pc6001_usart_interface=
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


static const ay8910_interface pc6001_ay_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("P1"),
	DEVCB_INPUT_PORT("P2"),
	DEVCB_NULL,
	DEVCB_NULL
};

static UINT8 check_keyboard_press(running_machine &machine)
{
	static const char *const portnames[3] = { "key1","key2","key3" };
	int i,port_i,scancode;
	UINT8 shift_pressed,caps_lock;
	scancode = 0;

	shift_pressed = (machine.root_device().ioport("key_modifiers")->read() & 2)>>1;
	caps_lock = (machine.root_device().ioport("key_modifiers")->read() & 8)>>3;

	for(port_i=0;port_i<3;port_i++)
	{
		for(i=0;i<32;i++)
		{
			if((machine.root_device().ioport(portnames[port_i])->read()>>i) & 1)
			{
				if((shift_pressed != caps_lock) && scancode >= 0x41 && scancode <= 0x5f)
					scancode+=0x20;

				if(shift_pressed && scancode >= 0x31 && scancode <= 0x39)
					scancode-=0x10;

				if(shift_pressed && scancode == 0x30) // '0' / '='
					scancode = 0x3d;

				if(shift_pressed && scancode == 0x2c) // ',' / ';'
					scancode = 0x3b;

				if(shift_pressed && scancode == 0x2f) // '/' / '?'
					scancode = 0x3f;

				if(shift_pressed && scancode == 0x2e) // '.' / ':'
					scancode = 0x3a;

				return scancode;
			}
			scancode++;
		}
	}

	return 0;
}

static UINT8 check_joy_press(running_machine &machine)
{
	UINT8 p1_key = machine.root_device().ioport("P1")->read() ^ 0xff;
	UINT8 shift_key = machine.root_device().ioport("key_modifiers")->read() & 0x02;
	UINT8 space_key = machine.root_device().ioport("key2")->read() & 0x01;
	UINT8 joy_press;

		/*
            PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
            PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
            PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
            PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
            PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
            PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
            PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
            PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
        */

	joy_press = 0;

	switch(p1_key & 0xf)
	{
		case 0x01: joy_press = 0x04; break; //up
		case 0x02: joy_press = 0x08; break; //down
		case 0x04: joy_press = 0x20; break;
		case 0x05: joy_press = 0x24; break; //up-left
		case 0x06: joy_press = 0x28; break; //down-left
		case 0x08: joy_press = 0x10; break; //right
		case 0x09: joy_press = 0x14; break; //up-right
		case 0x0a: joy_press = 0x18; break; //down-right
	}

	if(p1_key & 0x10 || space_key) { joy_press |= 0x80; } //button 1 (space)
	if(p1_key & 0x20 || shift_key) { joy_press |= 0x01; } //button 2 (shift)

	return joy_press;
}

static TIMER_DEVICE_CALLBACK(cassette_callback)
{
	pc6001_state *state = timer.machine().driver_data<pc6001_state>();
	if(state->m_cas_switch == 1)
	{
		#if 0
		static UINT8 cas_data_i = 0x80,cas_data_poll;
		//state->m_cur_keycode = gfx_data[state->m_cas_offset++];
		if((timer.machine().device<cassette_image_device>(CASSETTE_TAG))->input() > 0.03)
			cas_data_poll|= cas_data_i;
		else
			cas_data_poll&=~cas_data_i;
		if(cas_data_i == 1)
		{
			state->m_cur_keycode = cas_data_poll;
			cas_data_i = 0x80;
			/* data ready, poll irq */
			state->m_irq_vector = 0x08;
			timer.machine().device("maincpu")->execute().set_input_line(0, ASSERT_LINE);
		}
		else
			cas_data_i>>=1;
		#else
			UINT8 *cas_data = timer.machine().root_device().memregion("cas")->base();

			state->m_cur_keycode = cas_data[state->m_cas_offset++];
			popmessage("%04x %04x",state->m_cas_offset,state->m_cas_maxsize);
			if(state->m_cas_offset > state->m_cas_maxsize)
			{
				state->m_cas_offset = 0;
				state->m_cas_switch = 0;
				if(IRQ_LOG) printf("Tape-E IRQ 0x12\n");
				state->m_irq_vector = 0x12;
				timer.machine().device("maincpu")->execute().set_input_line(0, ASSERT_LINE);
			}
			else
			{
				if(IRQ_LOG) printf("Tape-D IRQ 0x08\n");
				state->m_irq_vector = 0x08;
				timer.machine().device("maincpu")->execute().set_input_line(0, ASSERT_LINE);
			}
		#endif
	}
}

static TIMER_DEVICE_CALLBACK(keyboard_callback)
{
	pc6001_state *state = timer.machine().driver_data<pc6001_state>();
	address_space &space = *timer.machine().device("maincpu")->memory().space(AS_PROGRAM);
	UINT32 key1 = timer.machine().root_device().ioport("key1")->read();
	UINT32 key2 = timer.machine().root_device().ioport("key2")->read();
	UINT32 key3 = timer.machine().root_device().ioport("key3")->read();
//  UINT8 p1_key = timer.machine().root_device().ioport("P1")->read();

	if(state->m_cas_switch == 0)
	{
		if((key1 != state->m_old_key1) || (key2 != state->m_old_key2) || (key3 != state->m_old_key3))
		{
			state->m_cur_keycode = check_keyboard_press(space.machine());
			if(IRQ_LOG) printf("KEY IRQ 0x02\n");
			state->m_irq_vector = 0x02;
			timer.machine().device("maincpu")->execute().set_input_line(0, ASSERT_LINE);
			state->m_old_key1 = key1;
			state->m_old_key2 = key2;
			state->m_old_key3 = key3;
		}
		#if 0
		else /* joypad polling */
		{
			state->m_cur_keycode = check_joy_press(space.machine());
			if(state->m_cur_keycode)
			{
				state->m_irq_vector = 0x16;
				timer.machine().device("maincpu")->execute().set_input_line(0, ASSERT_LINE);
			}
		}
		#endif
	}
}

void pc6001_state::machine_start()
{
	m_timer_hz_div = 3;
	{
		attotime period = attotime::from_hz((487.5*4)/(m_timer_hz_div+1));
		m_timer_irq_timer = machine().scheduler().timer_alloc(FUNC(audio_callback));
		m_timer_irq_timer->adjust(period,  0, period);
	}
}

void pc6001_state::machine_reset()
{
	UINT8 *work_ram = memregion("maincpu")->base();

	m_video_ram =  work_ram + 0xc000;

	m_port_c_8255=0;

	machine().device("maincpu")->execute().set_irq_acknowledge_callback(pc6001_irq_callback);
	m_cas_switch = 0;
	m_cas_offset = 0;
	m_timer_irq_mask = 1;
	m_timer_irq_mask2 = 1;
	m_timer_irq_vector = 0x06; // actually vector is fixed in plain PC-6001
	m_timer_hz_div = 3;
}

MACHINE_RESET_MEMBER(pc6001_state,pc6001m2)
{
	UINT8 *work_ram = machine().root_device().memregion("maincpu")->base();

	m_video_ram = work_ram + 0xc000 + 0x28000;

	m_port_c_8255=0;

	machine().device("maincpu")->execute().set_irq_acknowledge_callback(pc6001_irq_callback);
	m_cas_switch = 0;
	m_cas_offset = 0;

	/* set default bankswitch */
	{
		UINT8 *ROM = memregion("maincpu")->base();
		m_bank_r0 = 0x71;
		membank("bank1")->set_base(&ROM[BASICROM(0)]);
		membank("bank2")->set_base(&ROM[BASICROM(1)]);
		membank("bank3")->set_base(&ROM[EXROM(0)]);
		membank("bank4")->set_base(&ROM[EXROM(1)]);
		m_bank_r1 = 0xdd;
		membank("bank5")->set_base(&ROM[WRAM(4)]);
		membank("bank6")->set_base(&ROM[WRAM(5)]);
		membank("bank7")->set_base(&ROM[WRAM(6)]);
		membank("bank8")->set_base(&ROM[WRAM(7)]);
		m_bank_opt = 0x02; //tv rom
		m_bank_w = 0x50; //enable write to work ram 4,5,6,7
		m_gfx_bank_on = 0;
	}

	m_timer_irq_mask = 1;
	m_timer_irq_mask2 = 1;
	m_timer_irq_vector = 0x06;
}

MACHINE_RESET_MEMBER(pc6001_state,pc6001sr)
{
	UINT8 *work_ram = machine().root_device().memregion("maincpu")->base();

	m_video_ram = work_ram + 0x70000;

	m_port_c_8255=0;

	machine().device("maincpu")->execute().set_irq_acknowledge_callback(pc6001_irq_callback);
	m_cas_switch = 0;
	m_cas_offset = 0;

	/* set default bankswitch */
	{
		UINT8 *ROM = memregion("maincpu")->base();
		m_sr_bank_r[0] = 0xf8; membank("bank1")->set_base(&ROM[SR_SYSROM_1(0x08)]);
		m_sr_bank_r[1] = 0xfa; membank("bank2")->set_base(&ROM[SR_SYSROM_1(0x0a)]);
		m_sr_bank_r[2] = 0xb0; membank("bank3")->set_base(&ROM[SR_EXROM1(0x00)]);
		m_sr_bank_r[3] = 0xc0; membank("bank4")->set_base(&ROM[SR_EXROM0(0x00)]);
		m_sr_bank_r[4] = 0x08; membank("bank5")->set_base(&ROM[SR_WRAM0(0x08)]);
		m_sr_bank_r[5] = 0x0a; membank("bank6")->set_base(&ROM[SR_WRAM0(0x0a)]);
		m_sr_bank_r[6] = 0x0c; membank("bank7")->set_base(&ROM[SR_WRAM0(0x0c)]);
		m_sr_bank_r[7] = 0x0e; membank("bank8")->set_base(&ROM[SR_WRAM0(0x0e)]);
//      m_bank_opt = 0x02; //tv rom

		/* enable default work RAM writes */
		m_sr_bank_w[0] = 0x00;
		m_sr_bank_w[1] = 0x02;
		m_sr_bank_w[2] = 0x04;
		m_sr_bank_w[3] = 0x06;
		m_sr_bank_w[4] = 0x08;
		m_sr_bank_w[5] = 0x0a;
		m_sr_bank_w[6] = 0x0c;
		m_sr_bank_w[7] = 0x0e;

		m_gfx_bank_on = 0;
	}

	m_timer_irq_mask = 1;
	m_timer_irq_mask2 = 1;
	m_timer_irq_vector = 0x06;
}

static const rgb_t defcolors[] =
{
	MAKE_RGB(0x07, 0xff, 0x00), /* GREEN */
	MAKE_RGB(0xff, 0xff, 0x00),	/* YELLOW */
	MAKE_RGB(0x3b, 0x08, 0xff), /* BLUE */
	MAKE_RGB(0xcc, 0x00, 0x3b), /* RED */
	MAKE_RGB(0xff, 0xff, 0xff),	/* BUFF */
	MAKE_RGB(0x07, 0xe3, 0x99),	/* CYAN */
	MAKE_RGB(0xff, 0x1c, 0xff),	/* MAGENTA */
	MAKE_RGB(0xff, 0x81, 0x00),	/* ORANGE */

	/* MC6847 specific */
	MAKE_RGB(0x00, 0x7c, 0x00),	/* ALPHANUMERIC DARK GREEN */
	MAKE_RGB(0x07, 0xff, 0x00),	/* ALPHANUMERIC BRIGHT GREEN */
	MAKE_RGB(0x91, 0x00, 0x00),	/* ALPHANUMERIC DARK ORANGE */
	MAKE_RGB(0xff, 0x81, 0x00)	/* ALPHANUMERIC BRIGHT ORANGE */
};

static const rgb_t mk2_defcolors[] =
{
	MAKE_RGB(0x00, 0x00, 0x00),	/* BLACK */
	MAKE_RGB(0xff, 0xaf, 0x00),	/* ORANGE */
	MAKE_RGB(0x00, 0xff, 0xaf),	/* tone of GREEN */
	MAKE_RGB(0xaf, 0xff, 0x00),	/* tone of GREEN */
	MAKE_RGB(0xaf, 0x00, 0xff),	/* VIOLET */
	MAKE_RGB(0xff, 0x00, 0xaf),	/* SCARLET */
	MAKE_RGB(0x00, 0xaf, 0xff),	/* LIGHT BLUE */
	MAKE_RGB(0xaf, 0xaf, 0xaf),	/* GRAY */
	MAKE_RGB(0x00, 0x00, 0x00),	/* BLACK */
	MAKE_RGB(0xff, 0x00, 0x00),	/* RED */
	MAKE_RGB(0x00, 0xff, 0x00),	/* GREEN */
	MAKE_RGB(0xff, 0xff, 0x00),	/* YELLOW */
	MAKE_RGB(0x00, 0x00, 0xff),	/* BLUE */
	MAKE_RGB(0xff, 0x00, 0xff),	/* PINK */
	MAKE_RGB(0x00, 0xff, 0xff),	/* CYAN */
	MAKE_RGB(0xff, 0xff, 0xff)	/* WHITE */
};

void pc6001_state::palette_init()
{
	int i;

	for(i=0;i<8+4;i++)
		palette_set_color(machine(), i+8,defcolors[i]);
}

PALETTE_INIT_MEMBER(pc6001_state,pc6001m2)
{
	int i;

	for(i=0;i<8;i++)
		palette_set_color(machine(), i+8,defcolors[i]);

	for(i=0x10;i<0x20;i++)
		palette_set_color(machine(), i,mk2_defcolors[i-0x10]);
}

static const cassette_interface pc6001_cassette_interface =
{
	pc6001_cassette_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED),
	NULL,
	NULL
};

static DEVICE_IMAGE_LOAD( pc6001_cass )
{
	pc6001_state *state = image.device().machine().driver_data<pc6001_state>();
	UINT8 *cas = state->memregion("cas")->base();
	UINT32 size;

	size = image.length();
	if (image.fread( cas, size) != size)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unable to fully read from file");
		return IMAGE_INIT_FAIL;
	}

	state->m_cas_maxsize = size;

	return IMAGE_INIT_PASS;
}

static const gfx_layout char_layout =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};


static const gfx_layout kanji_layout =
{
	16, 16,
	RGN_FRAC(1,2),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
	0+RGN_FRAC(1,2), 1+RGN_FRAC(1,2), 2+RGN_FRAC(1,2), 3+RGN_FRAC(1,2), 4+RGN_FRAC(1,2), 5+RGN_FRAC(1,2), 6+RGN_FRAC(1,2), 7+RGN_FRAC(1,2)  },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};

static GFXDECODE_START( pc6001m2 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, char_layout, 2, 1 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, kanji_layout, 2, 1 )
GFXDECODE_END

#define PC6001_MAIN_CLOCK 7987200

static MACHINE_CONFIG_START( pc6001, pc6001_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, PC6001_MAIN_CLOCK / 2) // ~4 Mhz
	MCFG_CPU_PROGRAM_MAP(pc6001_map)
	MCFG_CPU_IO_MAP(pc6001_io)
	MCFG_CPU_VBLANK_INT("screen", pc6001_interrupt)

//  MCFG_CPU_ADD("subcpu", I8049, 7987200)


	MCFG_GFXDECODE(pc6001m2)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DRIVER(pc6001_state, screen_update_pc6001)
//  MCFG_SCREEN_REFRESH_RATE(M6847_NTSC_FRAMES_PER_SECOND)
//  MCFG_SCREEN_UPDATE_STATIC(m6847)
	MCFG_SCREEN_SIZE(320, 25+192+26)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)


	MCFG_PALETTE_LENGTH(16+4)

	MCFG_I8255_ADD( "ppi8255", pc6001_ppi8255_interface )
	/* uart */
	MCFG_I8251_ADD("uart", pc6001_usart_interface)

	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin")
	MCFG_CARTSLOT_NOT_MANDATORY

//  MCFG_CASSETTE_ADD(CASSETTE_TAG,pc6001_cassette_interface)
	MCFG_CARTSLOT_ADD(CASSETTE_TAG)
	MCFG_CARTSLOT_EXTENSION_LIST("cas,p6")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("pc6001_cass")
	MCFG_CARTSLOT_LOAD(pc6001_cass)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8910", AY8910, PC6001_MAIN_CLOCK/4)
	MCFG_SOUND_CONFIG(pc6001_ay_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
//  MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* TODO: accurate timing on this */
	MCFG_TIMER_ADD_PERIODIC("keyboard_timer", keyboard_callback, attotime::from_hz(250))
	MCFG_TIMER_ADD_PERIODIC("cassette_timer", cassette_callback, attotime::from_hz(1200/12)) //1200 bauds / (1 start bit -> 8 data bits -> 3 stop bits)
MACHINE_CONFIG_END



static MACHINE_CONFIG_DERIVED( pc6001m2, pc6001 )

	MCFG_MACHINE_RESET_OVERRIDE(pc6001_state,pc6001m2)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(pc6001_state, screen_update_pc6001m2)
	MCFG_PALETTE_LENGTH(16+16)
	MCFG_PALETTE_INIT_OVERRIDE(pc6001_state,pc6001m2)

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(pc6001m2_map)
	MCFG_CPU_IO_MAP(pc6001m2_io)

	MCFG_GFXDECODE(pc6001m2)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pc6601, pc6001m2 )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", Z80, PC6001_MAIN_CLOCK / 2)
	MCFG_CPU_PROGRAM_MAP(pc6001m2_map)
	MCFG_CPU_IO_MAP(pc6601_io)
	MCFG_CPU_VBLANK_INT("screen", pc6001_interrupt)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pc6001sr, pc6001m2 )

	MCFG_MACHINE_RESET_OVERRIDE(pc6001_state,pc6001sr)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(pc6001_state, screen_update_pc6001sr)

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", Z80, XTAL_3_579545MHz) //*Yes*, PC-6001 SR Z80 CPU is actually slower than older models
	MCFG_CPU_PROGRAM_MAP(pc6001sr_map)
	MCFG_CPU_IO_MAP(pc6001sr_io)
	MCFG_CPU_VBLANK_INT("screen", pc6001sr_interrupt)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pc6001 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "basicrom.60", 0x0000, 0x4000, CRC(54c03109) SHA1(c622fefda3cdc2b87a270138f24c05828b5c41d2) )

	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "i8049", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "cgrom60.60", 0x0000, 0x1000, CRC(b0142d32) SHA1(9570495b10af5b1785802681be94b0ea216a1e26) )
	ROM_RELOAD(             0x1000, 0x1000 )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "cas", ROMREGION_ERASEFF )

	ROM_REGION( 0x4000, "cart_img", ROMREGION_ERASE00 )
	ROM_CART_LOAD("cart", 0x0000, 0x4000, ROM_OPTIONAL | ROM_MIRROR)
ROM_END

ROM_START( pc6001a )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "basicrom.60a", 0x0000, 0x4000, CRC(fa8e88d9) SHA1(c82e30050a837e5c8ffec3e0c8e3702447ffd69c) )

	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "i8049", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "cgrom60.60a", 0x0000, 0x1000, CRC(49c21d08) SHA1(9454d6e2066abcbd051bad9a29a5ca27b12ec897) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "cas", ROMREGION_ERASEFF )

	ROM_REGION( 0x4000, "cart_img", ROMREGION_ERASE00 )
	ROM_CART_LOAD("cart", 0x0000, 0x4000, ROM_OPTIONAL | ROM_MIRROR)
ROM_END

ROM_START( pc6001mk2 )
	ROM_REGION( 0x50000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "basicrom.62", 0x10000, 0x8000, CRC(950ac401) SHA1(fbf195ba74a3b0f80b5a756befc96c61c2094182) )
	ROM_LOAD( "voicerom.62", 0x18000, 0x4000, CRC(49b4f917) SHA1(1a2d18f52ef19dc93da3d65f19d3abbd585628af) )
	ROM_LOAD( "cgrom60.62",  0x1c000, 0x2000, CRC(81eb5d95) SHA1(53d8ae9599306ff23bf95208d2f6cc8fed3fc39f) )
	ROM_LOAD( "cgrom60m.62", 0x1e000, 0x2000, CRC(3ce48c33) SHA1(f3b6c63e83a17d80dde63c6e4d86adbc26f84f79) )
	ROM_LOAD( "kanjirom.62", 0x20000, 0x8000, CRC(20c8f3eb) SHA1(4c9f30f0a2ebbe70aa8e697f94eac74d8241cadd) )
	// work ram              0x28000,0x10000
	// extended work ram     0x38000,0x10000
	ROM_CART_LOAD("cart",    0x48000, 0x4000, ROM_OPTIONAL | ROM_MIRROR)
	// <invalid>             0x4c000, 0x4000

	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "i8049", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_COPY( "maincpu", 0x1c000, 0x00000, 0x4000 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "maincpu", 0x20000, 0x00000, 0x8000 )

	ROM_REGION( 0x20000, "cas", ROMREGION_ERASEFF )

	ROM_REGION( 0x4000, "cart_img", ROMREGION_ERASE00 )
	ROM_COPY( "maincpu", 0x48000, 0x0000, 0x4000 )
ROM_END

ROM_START( pc6601 )	/* Variant of pc6001m2 */
	ROM_REGION( 0x50000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "basicrom.66", 0x10000, 0x8000, CRC(c0b01772) SHA1(9240bb6b97fe06f5f07b5d65541c4d2f8758cc2a) )
	ROM_LOAD( "voicerom.66", 0x18000, 0x4000, CRC(91d078c1) SHA1(6a93bd7723ef67f461394530a9feee57c8caf7b7) )
	ROM_LOAD( "cgrom60.66",  0x1c000, 0x2000, CRC(d2434f29) SHA1(a56d76f5cbdbcdb8759abe601eab68f01b0a8fe8) )
	ROM_LOAD( "cgrom66.66",  0x1e000, 0x2000, CRC(3ce48c33) SHA1(f3b6c63e83a17d80dde63c6e4d86adbc26f84f79) )
	ROM_LOAD( "kanjirom.66", 0x20000, 0x8000, CRC(20c8f3eb) SHA1(4c9f30f0a2ebbe70aa8e697f94eac74d8241cadd) )
	ROM_CART_LOAD("cart",    0x48000, 0x4000, ROM_OPTIONAL | ROM_MIRROR)

	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "i8049", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_COPY( "maincpu", 0x1c000, 0x00000, 0x4000 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "maincpu", 0x20000, 0x00000, 0x8000 )

	ROM_REGION( 0x20000, "cas", ROMREGION_ERASEFF )

	ROM_REGION( 0x4000, "cart_img", ROMREGION_ERASE00 )
	ROM_COPY( "maincpu", 0x48000, 0x0000, 0x4000 )
ROM_END

ROM_START( pc6001sr )
	ROM_REGION( 0x90000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "systemrom1.64", 0x10000, 0x10000, CRC(b6fc2db2) SHA1(dd48b1eee60aa34780f153359f5da7f590f8dff4) )
	ROM_LOAD( "systemrom2.64", 0x20000, 0x10000, CRC(55a62a1d) SHA1(3a19855d290fd4ac04e6066fe4a80ecd81dc8dd7) )
//  cgrom 1                    0x30000, 0x10000
//  exrom 0                    0x40000, 0x10000
//  exrom 1                    0x50000, 0x10000
//  exram 0                    0x60000, 0x10000
//  work ram 0                 0x70000, 0x10000
//  <invalid>                  0x80000, 0x10000

	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "i8049", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "cgrom68.64", 0x0000, 0x4000, CRC(73bc3256) SHA1(5f80d62a95331dc39b2fb448a380fd10083947eb) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "maincpu", 0x28000, 0x00000, 0x8000 )

	ROM_REGION( 0x20000, "cas", ROMREGION_ERASEFF )

	ROM_REGION( 0x4000, "cart_img", ROMREGION_ERASE00 )
	ROM_COPY( "maincpu", 0x48000, 0x0000, 0x4000 )
ROM_END

/*    YEAR  NAME      PARENT   COMPAT MACHINE   INPUT     INIT    COMPANY  FULLNAME          FLAGS */
COMP( 1981, pc6001,   0,       0,     pc6001,   pc6001, driver_device,   0,      "Nippon Electronic Company",   "PC-6001 (Japan)",    GAME_NOT_WORKING )
COMP( 1981, pc6001a,  pc6001,  0,     pc6001,   pc6001, driver_device,   0,      "Nippon Electronic Company",   "PC-6001A (US)",      GAME_NOT_WORKING ) // This version is also known as the NEC Trek
COMP( 1983, pc6001mk2,pc6001,  0,     pc6001m2, pc6001, driver_device,   0,      "Nippon Electronic Company",   "PC-6001mkII (Japan)",   GAME_NOT_WORKING )
COMP( 1983, pc6601,   pc6001,  0,     pc6601,   pc6001, driver_device,   0,      "Nippon Electronic Company",   "PC-6601 (Japan)",       GAME_NOT_WORKING )
COMP( 1984, pc6001sr, pc6001,  0,     pc6001sr, pc6001, driver_device,   0,      "Nippon Electronic Company",   "PC-6001mkIISR (Japan)", GAME_NOT_WORKING )
