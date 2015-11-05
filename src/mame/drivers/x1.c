// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Barry Rodewald
/************************************************************************************************

    Sharp X1 (c) 1983 Sharp Corporation

    driver by Angelo Salese & Barry Rodewald,
    special thanks to Dirk Best for various wd17xx fixes

    TODO:
    - Rewrite keyboard input hook-up and decap/dump the keyboard MCU if possible;
    - Fix the 0xe80/0xe83 kanji ROM readback;
    - x1turbo keyboard inputs are currently broken, use x1turbo40 for now;
    - Hook-up remaining .tap image formats;
    - Implement APSS tape commands;
    - Sort out / redump the BIOS gfx roms;
    - X1Turbo: Implement SIO.
    - Implement true 400 lines mode (i.e. Chatnoir no Mahjong v2.1, Casablanca)
    - Implement SASI HDD interface;
    - clean-ups!
    - There are various unclear video things, these are:
        - Implement the remaining scrn regs;
        - Implement the new features of the x1turboz, namely the 4096 color feature amongst other things
        - (anything else?)
    - Driver Configuration switches:
        - OPN for X1
        - EMM, and hook-up for X1 too
        - RAM size for EMM
        - specific x1turboz features?

    per-game/program specific TODO:
    - CZ8FB02 / CZ8FB03: doesn't load at all, they are 2hd floppies apparently;
    - Chack'n Pop: game is too fast, presumably missing wait states;
    - Dragon Buster: it crashed to me once with a obj flag hang;
    - The Goonies (x1 only): goes offsync with the PCG beam positions;
    - Graphtol: sets up x1turboz paletteram, graphic garbage due of it;
    - Gyajiko2: hangs when it's supposed to load the character selection screen, FDC bug?
    - Hydlide 3: can't get the user disk to work properly, could be a bad dump;
    - Lupin the 3rd: don't know neither how to "data load" nor how to "create a character" ... does the game hangs?
    - Might & Magic: uses 0xe80-3 kanji ports, should be a good test case for that;
    - "newtype": trips a z80dma assert, worked around for now;
    - Saziri: doesn't re-initialize the tilemap attribute vram when you start a play, making it to have missing colors if you don't start a play in time;
    - Suikoden: shows a JP message error (DFJustin: "Problem with the disk device !! Please set a floppy disk properly and press the return key. Retrying.")
    - Super Billiards (X1 Pack 14): has a slight PCG timing bug, that happens randomly;
    - Trivia-Q: dunno what to do on the selection screen, missing inputs?
    - Turbo Alpha: has z80dma / fdc bugs, doesn't show the presentation properly and then hangs;
    - Will 2: doesn't load, fdc issue presumably (note: it's a x1turbo game ONLY);
    - X1F Demo ("New X1 Demo"): needs partial updates, but they doesn't cope well with current video system;
    - Ys 2: crashes after the disclaimer screen;
    - Ys 3: missing user disk, to hack it (and play with x1turboz features): bp 81ca,pc += 2
    - Ys 3: never uploads a valid 4096 palette, probably related to the fact that we don't have an user disk

    Notes:
    - An interesting feature of the Sharp X-1 is the extended i/o bank. When the ppi port c bit 5
      does a 1->0 transition, any write to the i/o space accesses 2 or 3 banks gradients of the bitmap RAM
      with a single write (generally used for layer clearances and bitmap-style sprites).
      Any i/o read disables this extended bitmap ram.
    - I/O port $700 bit 7 of X1 Turbo is a sound (dip-)switch / jumper setting. I don't know yet what is for,
      but King's Knight needs it to be active otherwise it refuses to boot.
    - ROM format is:
      0x00 ROM id (must be 0x01)
      0x01 - 0x0e ROM header
      0xff16 - 0xff17 start-up vector
      In theory, you can convert your tape / floppy games into ROM format easily, provided that you know what's the pinout of the
      cartridge slot and it doesn't exceed 64k (0x10000) of size.
    - Gruppe: shows a random bitmap graphic then returns "program load error" ... it wants that the floppy has write protection enabled (!) (btanb)
    - Maidum: you need to load BOTH disk with write protection disabled, otherwise it refuses to run. (btanb)
    - Marvelous: needs write protection disabled (btanb)
    - Chack'n Pop: to load this game, do a files command on the "Jodan Dos" prompt then move the cursor up at the "Chack'n Pop" file.
      Substitute bin with load and press enter. Finally, do a run once that it loaded correctly.
    - Faeries Residence: to load this game, put a basic v2.0 in drive 0, then execute a NEWON command. Load game disks into drive 0 and 1 then
      type run"START" (case sensitive)
    - POPLEMON: same as above, but you need to type run"POP"

=================================================================================================

    X1 (CZ-800C) - November, 1982
     * CPU: z80A @ 4MHz, 80C49 x 2 (one for key scan, the other for TV & Cas Ctrl)
     * ROM: IPL (4KB) + chargen (2KB)
     * RAM: Main memory (64KB) + VRAM (4KB) + RAM for PCG (6KB) + GRAM (48KB, Option)
     * Text Mode: 80x25 or 40x25
     * Graphic Mode: 640x200 or 320x200, 8 colors
     * Sound: PSG 8 octave
     * I/O Ports: Centronic ports, 2 Joystick ports, Cassette port (2700 baud)

    X1C (CZ-801C) - October, 1983
     * same but only 48KB GRAM

    X1D (CZ-802C) - October, 1983
     * same as X1C but with a 3" floppy drive (notice: 3" not 3" 1/2!!)

    X1Cs (CZ-803C) - June, 1984
     * two expansion I/O ports

    X1Ck (CZ-804C) - June, 1984
     * same as X1Cs
     * ROM: IPL (4KB) + chargen (2KB) + Kanji 1st level

    X1F Model 10 (CZ-811C) - July, 1985
     * Re-designed
     * ROM: IPL (4KB) + chargen (2KB)

    X1F Model 20 (CZ-812C) - July, 1985
     * Re-designed (same as Model 10)
     * ROM: IPL (4KB) + chargen (2KB) + Kanji
     * Built Tape drive plus a 5" floppy drive was available

    X1G Model 10 (CZ-820C) - July, 1986
     * Re-designed again
     * ROM: IPL (4KB) + chargen (2KB)

    X1G Model 30 (CZ-822C) - July, 1986
     * Re-designed again (same as Model 10)
     * ROM: IPL (4KB) + chargen (2KB) + Kanji
     * Built Tape drive plus a 5" floppy drive was available

    X1twin (CZ-830C) - December, 1986
     * Re-designed again (same as Model 10)
     * ROM: IPL (4KB) + chargen (2KB) + Kanji
     * Built Tape drive plus a 5" floppy drive was available
     * It contains a PC-Engine

    =============  X1 Turbo series  =============

    X1turbo Model 30 (CZ-852C) - October, 1984
     * CPU: z80A @ 4MHz, 80C49 x 2 (one for key scan, the other for TV & Cas Ctrl)
     * ROM: IPL (32KB) + chargen (8KB) + Kanji (128KB)
     * RAM: Main memory (64KB) + VRAM (6KB) + RAM for PCG (6KB) + GRAM (96KB)
     * Text Mode: 80xCh or 40xCh with Ch = 10, 12, 20, 25 (same for Japanese display)
     * Graphic Mode: 640x200 or 320x200, 8 colors
     * Sound: PSG 8 octave
     * I/O Ports: Centronic ports, 2 Joystick ports, built-in Cassette interface,
        2 Floppy drive for 5" disks, two expansion I/O ports

    X1turbo Model 20 (CZ-851C) - October, 1984
     * same as Model 30, but only 1 Floppy drive is included

    X1turbo Model 10 (CZ-850C) - October, 1984
     * same as Model 30, but Floppy drive is optional and GRAM is 48KB (it can
        be expanded to 96KB however)

    X1turbo Model 40 (CZ-862C) - July, 1985
     * same as Model 30, but uses tv screen (you could watch television with this)

    X1turboII (CZ-856C) - November, 1985
     * same as Model 30, but restyled, cheaper and sold with utility software

    X1turboIII (CZ-870C) - November, 1986
     * with two High Density Floppy driver

    X1turboZ (CZ-880C) - December, 1986
     * CPU: z80A @ 4MHz, 80C49 x 2 (one for key scan, the other for TV & Cas Ctrl)
     * ROM: IPL (32KB) + chargen (8KB) + Kanji 1st & 2nd level
     * RAM: Main memory (64KB) + VRAM (6KB) + RAM for PCG (6KB) + GRAM (96KB)
     * Text Mode: 80xCh or 40xCh with Ch = 10, 12, 20, 25 (same for Japanese display)
     * Graphic Mode: 640x200 or 320x200, 8 colors [in compatibility mode],
        640x400, 8 colors (out of 4096); 320x400, 64 colors (out of 4096);
        320x200, 4096 colors [in multimode],
     * Sound: PSG 8 octave + FM 8 octave
     * I/O Ports: Centronic ports, 2 Joystick ports, built-in Cassette interface,
        2 Floppy drive for HD 5" disks, two expansion I/O ports

    X1turboZII (CZ-881C) - December, 1987
     * same as turboZ, but added 64KB expansion RAM

    X1turboZIII (CZ-888C) - December, 1988
     * same as turboZII, but no more built-in cassette drive

    Please refer to http://www2s.biglobe.ne.jp/~ITTO/x1/x1menu.html for
    more info

    BASIC has to be loaded from external media (tape or disk), the
    computer only has an Initial Program Loader (IPL)

=================================================================================================

    x1turbo specs (courtesy of Yasuhiro Ogawa):

    upper board: Z80A-CPU
                 Z80A-DMA
                 Z80A-SIO(O)
                 Z80A-CTC
                 uPD8255AC
                 LH5357(28pin mask ROM. for IPL?)
                 YM2149F
                 16.000MHz(X1)

    lower board: IX0526CE(HN61364) (28pin mask ROM. for ANK font?)
                 MB83256x4 (Kanji ROMs)
                 HD46505SP (VDP)
                 M80C49-277 (MCU)
                 uPD8255AC
                 uPD1990 (RTC) + battery
                 6.000MHz(X2)
                 42.9545MHz(X3)

    FDD I/O board: MB8877A (FDC)
                   MB4107 (VFO)

    RAM banks:
    upper board: MB8265A-15 x8 (main memory)
    lower board: MB8416A-12 x3 (VRAM)
                 MB8416A-15 x3 (PCG RAM)
                 MB81416-10 x12 (GRAM)

************************************************************************************************/

#include "includes/x1.h"
#include "formats/2d_dsk.h"
#include "softlist.h"

#define MAIN_CLOCK XTAL_16MHz
#define VDP_CLOCK  XTAL_42_9545MHz
#define MCU_CLOCK  XTAL_6MHz




/*************************************
 *
 *  Video Functions
 *
 *************************************/

VIDEO_START_MEMBER(x1_state,x1)
{
	m_avram = auto_alloc_array_clear(machine(), UINT8, 0x800);
	m_tvram = auto_alloc_array_clear(machine(), UINT8, 0x800);
	m_kvram = auto_alloc_array_clear(machine(), UINT8, 0x800);
	m_gfx_bitmap_ram = auto_alloc_array_clear(machine(), UINT8, 0xc000*2);
	m_pal_4096 = auto_alloc_array_clear(machine(), UINT8, 0x1000*3);
}

void x1_state::x1_draw_pixel(bitmap_rgb32 &bitmap,int y,int x,UINT16 pen,UINT8 width,UINT8 height)
{
	if(!machine().first_screen()->visible_area().contains(x, y))
		return;

	if(width && height)
	{
		bitmap.pix32(y+0+m_ystart, x+0+m_xstart) = m_palette->pen(pen);
		bitmap.pix32(y+0+m_ystart, x+1+m_xstart) = m_palette->pen(pen);
		bitmap.pix32(y+1+m_ystart, x+0+m_xstart) = m_palette->pen(pen);
		bitmap.pix32(y+1+m_ystart, x+1+m_xstart) = m_palette->pen(pen);
	}
	else if(width)
	{
		bitmap.pix32(y+m_ystart, x+0+m_xstart) = m_palette->pen(pen);
		bitmap.pix32(y+m_ystart, x+1+m_xstart) = m_palette->pen(pen);
	}
	else if(height)
	{
		bitmap.pix32(y+0+m_ystart, x+m_xstart) = m_palette->pen(pen);
		bitmap.pix32(y+1+m_ystart, x+m_xstart) = m_palette->pen(pen);
	}
	else
		bitmap.pix32(y+m_ystart, x+m_xstart) = m_palette->pen(pen);
}

#define mc6845_h_char_total     (m_crtc_vreg[0])
#define mc6845_h_display        (m_crtc_vreg[1])
#define mc6845_h_sync_pos       (m_crtc_vreg[2])
#define mc6845_sync_width       (m_crtc_vreg[3])
#define mc6845_v_char_total     (m_crtc_vreg[4])
#define mc6845_v_total_adj      (m_crtc_vreg[5])
#define mc6845_v_display        (m_crtc_vreg[6])
#define mc6845_v_sync_pos       (m_crtc_vreg[7])
#define mc6845_mode_ctrl        (m_crtc_vreg[8])
#define mc6845_tile_height      (m_crtc_vreg[9]+1)
#define mc6845_cursor_y_start   (m_crtc_vreg[0x0a])
#define mc6845_cursor_y_end     (m_crtc_vreg[0x0b])
#define mc6845_start_addr       (((m_crtc_vreg[0x0c]<<8) & 0x3f00) | (m_crtc_vreg[0x0d] & 0xff))
#define mc6845_cursor_addr      (((m_crtc_vreg[0x0e]<<8) & 0x3f00) | (m_crtc_vreg[0x0f] & 0xff))
#define mc6845_light_pen_addr   (((m_crtc_vreg[0x10]<<8) & 0x3f00) | (m_crtc_vreg[0x11] & 0xff))
#define mc6845_update_addr      (((m_crtc_vreg[0x12]<<8) & 0x3f00) | (m_crtc_vreg[0x13] & 0xff))


/* adjust tile index when we are under double height condition */
UINT8 x1_state::check_prev_height(int x,int y,int x_size)
{
	UINT8 prev_tile = m_tvram[(x+((y-1)*x_size)+mc6845_start_addr) & 0x7ff];
	UINT8 cur_tile = m_tvram[(x+(y*x_size)+mc6845_start_addr) & 0x7ff];
	UINT8 prev_attr = m_avram[(x+((y-1)*x_size)+mc6845_start_addr) & 0x7ff];
	UINT8 cur_attr = m_avram[(x+(y*x_size)+mc6845_start_addr) & 0x7ff];

	if(prev_tile == cur_tile && prev_attr == cur_attr)
		return 8;

	return 0;
}

/* Exoa II - Warroid: if double height isn't enabled on the first tile of the line then double height is disabled on everything else. */
UINT8 x1_state::check_line_valid_height(int y,int x_size,int height)
{
	UINT8 line_attr = m_avram[(0+(y*x_size)+mc6845_start_addr) & 0x7ff];

	if((line_attr & 0x40) == 0)
		return 0;

	return height;
}

void x1_state::draw_fgtilemap(bitmap_rgb32 &bitmap,const rectangle &cliprect)
{
	/*
	    attribute table:
	    x--- ---- double width
	    -x-- ---- double height
	    --x- ---- PCG select
	    ---x ---- color blinking
	    ---- x--- reverse color
	    ---- -xxx color pen

	    x--- ---- select Kanji ROM
	    -x-- ---- Kanji side (0=left, 1=right)
	    --x- ---- Underline
	    ---x ---- Kanji ROM select (0=level 1, 1=level 2) (TODO: implement this)
	    ---- xxxx Kanji upper 4 bits
	*/

	int y,x,res_x,res_y;
	UINT32 tile_offset;
	UINT8 x_size,y_size;

	x_size = mc6845_h_display;
	y_size = mc6845_v_display;

	if(x_size == 0 || y_size == 0)
		return; //don't bother if screen is off

	if(x_size != 80 && x_size != 40 && y_size != 25)
		popmessage("%d %d",x_size,y_size);

	for (y=0;y<y_size;y++)
	{
		for (x=0;x<x_size;x++)
		{
			int tile = m_tvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff];
			int color = m_avram[((x+y*x_size)+mc6845_start_addr) & 0x7ff] & 0x1f;
			int width = BIT(m_avram[((x+y*x_size)+mc6845_start_addr) & 0x7ff], 7);
			int height = BIT(m_avram[((x+y*x_size)+mc6845_start_addr) & 0x7ff], 6);
			int pcg_bank = BIT(m_avram[((x+y*x_size)+mc6845_start_addr) & 0x7ff], 5);
			UINT8 *gfx_data = pcg_bank ? m_pcg_ram : m_cg_rom; //machine.root_device().memregion(pcg_bank ? "pcg" : "cgrom")->base();
			int knj_enable = 0;
			int knj_side = 0;
			int knj_bank = 0;
			int knj_uline = 0;
			if(m_is_turbo)
			{
				knj_enable = BIT(m_kvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff], 7);
				knj_side = BIT(m_kvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff], 6);
				knj_uline = BIT(m_kvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff], 5);
				//knj_lv2 = BIT(m_kvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff], 4);
				knj_bank = m_kvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff] & 0x0f;
				if(knj_enable)
				{
					gfx_data = m_kanji_rom;
					tile = ((tile + (knj_bank << 8)) << 1) + (knj_side & 1);
				}
			}

			{
				int pen[3],pen_mask,pcg_pen,xi,yi,dy;

				pen_mask = color & 7;

				dy = 0;

				height = check_line_valid_height(y,x_size,height);

				if(height && y)
					dy = check_prev_height(x,y,x_size);

				/* guess: assume that Kanji VRAM doesn't double the vertical size */
				if(knj_enable) { height = 0; }

				for(yi=0;yi<mc6845_tile_height;yi++)
				{
					for(xi=0;xi<8;xi++)
					{
						if(knj_enable) //kanji select
						{
							tile_offset  = tile * 16;
							tile_offset += (yi+dy*(m_scrn_reg.v400_mode+1)) >> (height+m_scrn_reg.v400_mode);
							pen[0] = gfx_data[tile_offset+0x0000]>>(7-xi) & (pen_mask & 1)>>0;
							pen[1] = gfx_data[tile_offset+0x0000]>>(7-xi) & (pen_mask & 2)>>1;
							pen[2] = gfx_data[tile_offset+0x0000]>>(7-xi) & (pen_mask & 4)>>2;

							if(yi == mc6845_tile_height-1 && knj_uline) //underlined attribute
							{
								pen[0] = (pen_mask & 1)>>0;
								pen[1] = (pen_mask & 2)>>1;
								pen[2] = (pen_mask & 4)>>2;
							}

							if((yi >= 16 && m_scrn_reg.v400_mode == 0) || (yi >= 32 && m_scrn_reg.v400_mode == 1))
								pen[0] = pen[1] = pen[2] = 0;
						}
						else if(pcg_bank) // PCG
						{
							tile_offset  = tile * 8;
							tile_offset += (yi+dy*(m_scrn_reg.v400_mode+1)) >> (height+m_scrn_reg.v400_mode);

							pen[0] = gfx_data[tile_offset+0x0000]>>(7-xi) & (pen_mask & 1)>>0;
							pen[1] = gfx_data[tile_offset+0x0800]>>(7-xi) & (pen_mask & 2)>>1;
							pen[2] = gfx_data[tile_offset+0x1000]>>(7-xi) & (pen_mask & 4)>>2;

							if((yi >= 8 && m_scrn_reg.v400_mode == 0) || (yi >= 16 && m_scrn_reg.v400_mode == 1))
								pen[0] = pen[1] = pen[2] = 0;
						}
						else
						{
							tile_offset  = tile * (8*(m_scrn_reg.ank_sel+1));
							tile_offset += (yi+dy*(m_scrn_reg.v400_mode+1)) >> (height+m_scrn_reg.v400_mode);

							pen[0] = gfx_data[tile_offset+m_scrn_reg.ank_sel*0x0800]>>(7-xi) & (pen_mask & 1)>>0;
							pen[1] = gfx_data[tile_offset+m_scrn_reg.ank_sel*0x0800]>>(7-xi) & (pen_mask & 2)>>1;
							pen[2] = gfx_data[tile_offset+m_scrn_reg.ank_sel*0x0800]>>(7-xi) & (pen_mask & 4)>>2;

							if(m_scrn_reg.ank_sel)
							{
								if((yi >= 16 && m_scrn_reg.v400_mode == 0) || (yi >= 32 && m_scrn_reg.v400_mode == 1))
									pen[0] = pen[1] = pen[2] = 0;
							}
							else
							{
								if((yi >=  8 && m_scrn_reg.v400_mode == 0) || (yi >= 16 && m_scrn_reg.v400_mode == 1))
									pen[0] = pen[1] = pen[2] = 0;
							}
						}

						pcg_pen = pen[2]<<2|pen[1]<<1|pen[0]<<0;

						if(color & 0x10 && machine().first_screen()->frame_number() & 0x10) //reverse flickering
							pcg_pen^=7;

						if(pcg_pen == 0 && (!(color & 8)))
							continue;

						if(color & 8) //revert the used color pen
							pcg_pen^=7;

						if((m_scrn_reg.blackclip & 8) && (color == (m_scrn_reg.blackclip & 7)))
							pcg_pen = 0; // clip the pen to black

						res_x = x*8+xi*(width+1);
						res_y = y*(mc6845_tile_height)+yi;

						if(res_y < cliprect.min_y || res_y > cliprect.max_y) // partial update, TODO: optimize
							continue;

						x1_draw_pixel(bitmap,res_y,res_x,pcg_pen,width,0);
					}
				}
			}

			if(width) //skip next char if we are under double width condition
				x++;
		}
	}
}

/*
 * Priority Mixer Calculation (pri)
 *
 * If pri is 0xff then the bitmap entirely covers the tilemap, if it's 0x00 then
 * the tilemap priority is entirely above the bitmap. Any other value mixes the
 * bitmap and the tilemap priorities based on the pen value, bit 0 = entry 0 <-> bit 7 = entry 7
 * of the bitmap.
 *
 */
int x1_state::priority_mixer_pri(int color)
{
	int pri_i,pri_mask_calc;

	pri_i = 0;
	pri_mask_calc = 1;

	while(pri_i < 7)
	{
		if((color & 7) == pri_i)
			break;

		pri_i++;
		pri_mask_calc<<=1;
	}

	return pri_mask_calc;
}

void x1_state::draw_gfxbitmap(bitmap_rgb32 &bitmap,const rectangle &cliprect, int plane,int pri)
{
	int xi,yi,x,y;
	int pen_r,pen_g,pen_b,color;
	int pri_mask_val;
	UINT8 x_size,y_size;
	int gfx_offset;

	x_size = mc6845_h_display;
	y_size = mc6845_v_display;

	if(x_size == 0 || y_size == 0)
		return; //don't bother if screen is off

	if(x_size != 80 && x_size != 40 && y_size != 25)
		popmessage("%d %d",x_size,y_size);

	//popmessage("%04x %02x",mc6845_start_addr,mc6845_tile_height);

	for (y=0;y<y_size;y++)
	{
		for(x=0;x<x_size;x++)
		{
			for(yi=0;yi<(mc6845_tile_height);yi++)
			{
				for(xi=0;xi<8;xi++)
				{
					gfx_offset = ((x+(y*x_size)) + mc6845_start_addr) & 0x7ff;
					gfx_offset+= ((yi >> m_scrn_reg.v400_mode) * 0x800) & 0x3fff;
					pen_b = (m_gfx_bitmap_ram[gfx_offset+0x0000+plane*0xc000]>>(7-xi)) & 1;
					pen_r = (m_gfx_bitmap_ram[gfx_offset+0x4000+plane*0xc000]>>(7-xi)) & 1;
					pen_g = (m_gfx_bitmap_ram[gfx_offset+0x8000+plane*0xc000]>>(7-xi)) & 1;

					color =  (pen_g<<2 | pen_r<<1 | pen_b<<0) | 8;

					pri_mask_val = priority_mixer_pri(color);
					if(pri_mask_val & pri) continue;

					if((color == 8 && m_scrn_reg.blackclip & 0x10) || (color == 9 && m_scrn_reg.blackclip & 0x20)) // bitmap color clip to black conditions
						color = 0;

					if(y*(mc6845_tile_height)+yi < cliprect.min_y || y*(mc6845_tile_height)+yi > cliprect.max_y) // partial update TODO: optimize
						continue;

					x1_draw_pixel(bitmap,y*(mc6845_tile_height)+yi,x*8+xi,color,0,0);
				}
			}
		}
	}
}

UINT32 x1_state::screen_update_x1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t(0xff,0x00,0x00,0x00), cliprect);

	/* TODO: correct calculation thru mc6845 regs */
	m_xstart = ((mc6845_h_char_total - mc6845_h_sync_pos) * 8) / 2;
	m_ystart = ((mc6845_v_char_total - mc6845_v_sync_pos) * 8) / 2;

//  popmessage("%d %d %d %d",mc6845_h_sync_pos,mc6845_v_sync_pos,mc6845_h_char_total,mc6845_v_char_total);

	draw_gfxbitmap(bitmap,cliprect,m_scrn_reg.disp_bank,m_scrn_reg.pri);
	draw_fgtilemap(bitmap,cliprect);
	draw_gfxbitmap(bitmap,cliprect,m_scrn_reg.disp_bank,m_scrn_reg.pri^0xff);

	return 0;
}


/*************************************
 *
 *  Keyboard MCU simulation
 *
 *************************************/


UINT16 x1_state::check_keyboard_press()
{
	static const char *const portnames[3] = { "key1","key2","key3" };
	int i,port_i,scancode;
	UINT8 keymod = ioport("key_modifiers")->read() & 0x1f;
	UINT32 pad = ioport("tenkey")->read();
	UINT32 f_key = ioport("f_keys")->read();
	scancode = 0;

	for(port_i=0;port_i<3;port_i++)
	{
		for(i=0;i<32;i++)
		{
			if((ioport(portnames[port_i])->read()>>i) & 1)
			{
				//key_flag = 1;
				if(keymod & 0x02)  // shift not pressed
				{
					if(scancode >= 0x41 && scancode < 0x5a)
						scancode += 0x20;  // lowercase
				}
				else
				{
					if(scancode >= 0x31 && scancode < 0x3a)
						scancode -= 0x10;
					if(scancode == 0x30)
					{
						scancode = 0x3d;
					}
				}
				if((keymod & 0x10) == 0) // graph on
					scancode |= 0x80;

				return scancode;
			}
			scancode++;
		}
	}

	// check numpad
	scancode = 0x30;
	for(i=0;i<10;i++)
	{
		if((pad >> i) & 0x01)
		{
			return scancode | 0x100;
		}
		scancode++;
	}

	// check function keys
	scancode = 0x71;
	for(i=0;i<5;i++)
	{
		if((f_key >> i) & 0x01)
		{
			return (scancode + ((keymod & 0x02) ? 0 : 5)) | 0x100;
		}
		scancode++;
	}

	return 0;
}

UINT8 x1_state::check_keyboard_shift()
{
	UINT8 val = 0xe0;
	/*
	all of those are active low
	x--- ---- TEN: Numpad, Function key, special input key
	-x-- ---- KIN: Valid key
	--x- ---- REP: Key repeat
	---x ---- GRAPH key ON
	---- x--- CAPS lock ON
	---- -x-- KANA lock ON
	---- --x- SHIFT ON
	---- ---x CTRL ON
	*/

	val |= ioport("key_modifiers")->read() & 0x1f;

	if(check_keyboard_press() != 0)
		val &= ~0x40;

	if(check_keyboard_press() & 0x100) //function keys
		val &= ~0x80;

	return val;
}

UINT8 x1_state::get_game_key(UINT8 port)
{
	// key status returned by sub CPU function 0xE3.
	// in order from bit 7 to 0:
	// port 0: Q,W,E,A,D,Z,X,C
	// port 1: numpad 7,4,1,8,2,9,6,3
	// port 2: ESC,1,[-],[+],[*],TAB,SPC,RET ([] = numpad)
	// bits are active high
	UINT8 ret = 0;

	if (port == 0)
	{
		UINT32 key3 = ioport("key3")->read();
		if(key3 & 0x00020000) ret |= 0x80;  // Q
		if(key3 & 0x00800000) ret |= 0x40;  // W
		if(key3 & 0x00000020) ret |= 0x20;  // E
		if(key3 & 0x00000002) ret |= 0x10;  // A
		if(key3 & 0x00000010) ret |= 0x08;  // D
		if(key3 & 0x04000000) ret |= 0x04;  // Z
		if(key3 & 0x01000000) ret |= 0x02;  // X
		if(key3 & 0x00000008) ret |= 0x01;  // C
	}
	else
	if (port == 1)
	{
		UINT32 pad = ioport("tenkey")->read();
		if(pad & 0x00000080) ret |= 0x80;  // Tenkey 7
		if(pad & 0x00000010) ret |= 0x40;  // Tenkey 4
		if(pad & 0x00000002) ret |= 0x20;  // Tenkey 1
		if(pad & 0x00000100) ret |= 0x10;  // Tenkey 8
		if(pad & 0x00000004) ret |= 0x08;  // Tenkey 2
		if(pad & 0x00000200) ret |= 0x04;  // Tenkey 9
		if(pad & 0x00000040) ret |= 0x02;  // Tenkey 6
		if(pad & 0x00000008) ret |= 0x01;  // Tenkey 3
	}
	else
	if (port == 2)
	{
		UINT32 key1 = ioport("key1")->read();
		UINT32 key2 = ioport("key2")->read();
		UINT32 pad = ioport("tenkey")->read();
		if(key1 & 0x08000000) ret |= 0x80;  // ESC
		if(key2 & 0x00020000) ret |= 0x40;  // 1
		if(pad & 0x00000400) ret |= 0x20;  // Tenkey -
		if(pad & 0x00000800) ret |= 0x10;  // Tenkey +
		if(pad & 0x00001000) ret |= 0x08;  // Tenkey *
		if(key1 & 0x00000200) ret |= 0x04;  // TAB
		if(key2 & 0x00000001) ret |= 0x02;  // SPC
		if(key1 & 0x00002000) ret |= 0x01;  // RET
	}

	return ret;
}

READ8_MEMBER( x1_state::x1_sub_io_r )
{
	UINT8 ret,bus_res;

	/* Looks like that the HW retains the latest data putted on the bus here, behaviour confirmed by Rally-X */
	if(m_sub_obf)
	{
		bus_res = m_sub_val[m_key_i];
		/* FIXME: likely to be different here. */
		m_key_i++;
		if(m_key_i >= 2)
			m_key_i = 0;

		return bus_res;
	}

#if 0
	if(key_flag == 1)
	{
		key_flag = 0;
		return 0x82; //TODO: this is for shift/ctrl/kana lock etc.
	}
#endif

	m_sub_cmd_length--;
	m_sub_obf = (m_sub_cmd_length) ? 0x00 : 0x20;

	ret = m_sub_val[m_sub_val_ptr];

	m_sub_val_ptr++;
	if(m_sub_cmd_length <= 0)
		m_sub_val_ptr = 0;

	return ret;
}

void x1_state::cmt_command( UINT8 cmd )
{
	// CMT deck control command (E9 xx)
	// E9 00 - Eject
	// E9 01 - Stop
	// E9 02 - Play
	// E9 03 - Fast Forward
	// E9 04 - Rewind
	// E9 05 - APSS Fast Forward
	// E9 06 - APSS Rewind
	// E9 0A - Record
	/*
	APSS is a Sharp invention and stands for Automatic Program Search System, it scans the tape for silent parts that are bigger than 4 seconds.
	It's basically used for audio tapes in order to jump over the next/previous "track".
	*/
	m_cmt_current_cmd = cmd;

	if(m_cassette->get_image() == NULL) //avoid a crash if a disk game tries to access this
		return;

	switch(cmd)
	{
		case 0x01:  // Stop
			m_cassette->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
			m_cassette->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
			m_cmt_test = 1;
			popmessage("CMT: Stop");
			break;
		case 0x02:  // Play
			m_cassette->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
			m_cassette->change_state(CASSETTE_PLAY,CASSETTE_MASK_UISTATE);
			popmessage("CMT: Play");
			break;
		case 0x03:  // Fast Forward
			m_cassette->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
			m_cassette->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
			popmessage("CMT: Fast Forward");
			break;
		case 0x04:  // Rewind
			m_cassette->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
			m_cassette->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
			popmessage("CMT: Rewind");
			break;
		case 0x05:  // APSS Fast Forward
			m_cassette->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
			m_cassette->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
			popmessage("CMT: APSS Fast Forward");
			break;
		case 0x06:  // APSS Rewind
			m_cassette->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
			m_cassette->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
			popmessage("CMT: APSS Rewind");
			break;
		case 0x0a:  // Record
			m_cassette->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
			m_cassette->change_state(CASSETTE_RECORD,CASSETTE_MASK_UISTATE);
			popmessage("CMT: Record");
			break;
		default:
			logerror("Unimplemented or invalid CMT command (0x%02x)\n",cmd);
	}
	logerror("CMT: Command 0xe9-0x%02x received.\n",cmd);
}

TIMER_DEVICE_CALLBACK_MEMBER(x1_state::x1_cmt_wind_timer)
{
	if(m_cassette->get_image() == NULL) //avoid a crash if a disk game tries to access this
		return;

	switch(m_cmt_current_cmd)
	{
		case 0x03:
		case 0x05:  // Fast Forwarding tape
			m_cassette->seek(1,SEEK_CUR);
			if(m_cassette->get_position() >= m_cassette->get_length())  // at end?
				cmt_command(0x01);  // Stop tape
			break;
		case 0x04:
		case 0x06:  // Rewinding tape
			m_cassette->seek(-1,SEEK_CUR);
			if(m_cassette->get_position() <= 0) // at beginning?
				cmt_command(0x01);  // Stop tape
			break;
	}
}

WRITE8_MEMBER( x1_state::x1_sub_io_w )
{
	/* sub-routine at $10e sends to these sub-routines when a keyboard input is triggered:
	 $17a -> floppy
	 $094 -> ROM
	 $0c0 -> timer
	 $052 -> cmt
	 $0f5 -> reload sub-routine? */

	if(m_sub_cmd == 0xe4)
	{
		m_key_irq_vector = data;
		logerror("Key vector set to 0x%02x\n",data);
		data = 0;
	}

	if(m_sub_cmd == 0xe9)
	{
		cmt_command(data);
		data = 0;
	}

	if((data & 0xf0) == 0xd0) //reads here tv recording timer data. (Timer set (0xd0) / Timer readout (0xd8))
	{
		/*
		    xx-- ---- mode
		    --xx xxxx interval
		*/
		m_sub_val[0] = 0;
		/*
		    xxxx xxxx command code:
		    00 timer disabled
		    01 TV command
		    10 interrupt
		    11 Cassette deck
		*/
		m_sub_val[1] = 0;
		/*
		    ---x xxxx minute
		*/
		m_sub_val[2] = 0;
		/*
		    ---- xxxx hour
		*/
		m_sub_val[3] = 0;
		/*
		    xxxx ---- month
		    ---- -xxx day of the week
		*/
		m_sub_val[4] = 0;
		/*
		    --xx xxxx day
		*/
		m_sub_val[5] = 0;
		m_sub_cmd_length = 6;
	}

	switch(data)
	{
		case 0xe3: //game key obtaining
			m_sub_cmd_length = 3;
			m_sub_val[0] = get_game_key(0);
			m_sub_val[1] = get_game_key(1);
			m_sub_val[2] = get_game_key(2);
			break;
		case 0xe4: //irq vector setting
			break;
		//case 0xe5: //timer irq clear
		//  break;
		case 0xe6: //key data readout
			m_sub_val[0] = check_keyboard_shift() & 0xff;
			m_sub_val[1] = check_keyboard_press() & 0xff;
			m_sub_cmd_length = 2;
			break;
//      case 0xe7: // TV Control
//          break;
		case 0xe8: // TV Control read-out
			m_sub_val[0] = m_sub_cmd;
			m_sub_cmd_length = 1;
			break;
		case 0xe9: // CMT Control
			break;
		case 0xea:  // CMT Control status
			m_sub_val[0] = m_cmt_current_cmd;
			m_sub_cmd_length = 1;
			logerror("CMT: Command 0xEA received, returning 0x%02x.\n",m_sub_val[0]);
			break;
		case 0xeb:  // CMT Tape status
					// bit 0 = tape end (0=end of tape)
					// bit 1 = tape inserted
					// bit 2 = record status (1=OK, 0=write protect)
			m_sub_val[0] = 0x05;
			if(m_cassette->get_image() != NULL)
				m_sub_val[0] |= 0x02;
			m_sub_cmd_length = 1;
			logerror("CMT: Command 0xEB received, returning 0x%02x.\n",m_sub_val[0]);
			break;
//      case 0xec: //set date
//          break;
		case 0xed: //get date
			m_sub_val[0] = m_rtc.day;
			m_sub_val[1] = (m_rtc.month<<4) | (m_rtc.wday & 0xf);
			m_sub_val[2] = m_rtc.year;
			m_sub_cmd_length = 3;
			break;
//      case 0xee: //set time
//          break;
		case 0xef: //get time
			m_sub_val[0] = m_rtc.hour;
			m_sub_val[1] = m_rtc.min;
			m_sub_val[2] = m_rtc.sec;
			m_sub_cmd_length = 3;
			break;
	}

	m_sub_cmd = data;

	m_sub_obf = (m_sub_cmd_length) ? 0x00 : 0x20;

	if(data != 0xe6)
		logerror("SUB: Command byte 0x%02x\n",data);
}

/*************************************
 *
 *  ROM Image / Banking Handling
 *
 *************************************/


READ8_MEMBER( x1_state::x1_rom_r )
{
//  printf("%06x\n",m_rom_index[0]<<16|m_rom_index[1]<<8|m_rom_index[2]<<0);
	if (m_cart->exists())
		return m_cart->read_rom(space, (m_rom_index[0] << 16) | (m_rom_index[1] << 8) | (m_rom_index[2] << 0));
	else
		return 0;
}

WRITE8_MEMBER( x1_state::x1_rom_w )
{
	m_rom_index[offset] = data;
}

WRITE8_MEMBER( x1_state::x1_rom_bank_0_w )
{
	m_ram_bank = 0x10;
}

WRITE8_MEMBER( x1_state::x1_rom_bank_1_w )
{
	m_ram_bank = 0x00;
}

/*************************************
 *
 *  MB8877A FDC (wd17XX compatible)
 *
 *************************************/

READ8_MEMBER( x1_state::x1_fdc_r )
{
	//UINT8 ret = 0;

	switch(offset+0xff8)
	{
		case 0x0ff8:
			return m_fdc->status_r(space, offset);
		case 0x0ff9:
			return m_fdc->track_r(space, offset);
		case 0x0ffa:
			return m_fdc->sector_r(space, offset);
		case 0x0ffb:
			return m_fdc->data_r(space, offset);
		case 0x0ffc:
			printf("FDC: read FM type\n");
			return 0xff;
		case 0x0ffd:
			printf("FDC: read MFM type\n");
			return 0xff;
		case 0x0ffe:
			printf("FDC: read 1.6M type\n");
			return 0xff;
		case 0x0fff:
			printf("FDC: switching between 500k/1M\n");
			return 0xff;
	}

	return 0x00;
}

WRITE8_MEMBER( x1_state::x1_fdc_w )
{
	floppy_image_device *floppy = NULL;

	switch(offset+0xff8)
	{
		case 0x0ff8:
			m_fdc->cmd_w(space, offset,data);
			break;
		case 0x0ff9:
			m_fdc->track_w(space, offset,data);
			break;
		case 0x0ffa:
			m_fdc->sector_w(space, offset,data);
			break;
		case 0x0ffb:
			m_fdc->data_w(space, offset,data);
			break;

		case 0x0ffc:
			switch (data & 0x03)
			{
			case 0: floppy = m_floppy0->get_device(); break;
			case 1: floppy = m_floppy1->get_device(); break;
			case 2: floppy = m_floppy2->get_device(); break;
			case 3: floppy = m_floppy3->get_device(); break;
			}

			m_fdc->set_floppy(floppy);

			if (floppy)
			{
				floppy->ss_w(BIT(data, 4));
				floppy->mon_w(!BIT(data, 7));
			}
			break;

		case 0x0ffd:
		case 0x0ffe:
		case 0x0fff:
			logerror("FDC: undefined write to %04x = %02x\n",offset+0xff8,data);
			break;
	}
}

WRITE_LINE_MEMBER(x1_state::fdc_drq_w)
{
	m_dma->rdy_w(state ^ 1);
}

/*************************************
 *
 *  Programmable Character Generator
 *
 *************************************/

UINT16 x1_state::check_pcg_addr()
{
	if(m_avram[0x7ff] & 0x20) return 0x7ff;
	if(m_avram[0x3ff] & 0x20) return 0x3ff;
	if(m_avram[0x5ff] & 0x20) return 0x5ff;
	if(m_avram[0x1ff] & 0x20) return 0x1ff;

	return 0x3ff;
}

UINT16 x1_state::check_chr_addr()
{
	if(!(m_avram[0x7ff] & 0x20)) return 0x7ff;
	if(!(m_avram[0x3ff] & 0x20)) return 0x3ff;
	if(!(m_avram[0x5ff] & 0x20)) return 0x5ff;
	if(!(m_avram[0x1ff] & 0x20)) return 0x1ff;

	return 0x3ff;
}

UINT16 x1_state::get_pcg_addr( UINT16 width, UINT8 y_char_size )
{
	int hbeam = machine().first_screen()->hpos() >> 3;
	int vbeam = machine().first_screen()->vpos() / y_char_size;
	UINT16 pcg_offset = ((hbeam + vbeam*width) + (((m_crtc_vreg[0x0c]<<8) & 0x3f00) | (m_crtc_vreg[0x0d] & 0xff))) & 0x7ff;

	//printf("%08x %d %d %d %d\n",(hbeam+vbeam*width),hbeam,vbeam,machine.first_screen()->vpos() & 7,width);

	return pcg_offset;
}

READ8_MEMBER( x1_state::x1_pcg_r )
{
	int addr;
	int pcg_offset;
	UINT8 res;
	UINT8 *gfx_data;

	addr = (offset & 0x300) >> 8;

	if(addr == 0 && m_scrn_reg.pcg_mode) // Kanji ROM read, X1Turbo only
	{
		gfx_data = m_kanji_rom;
		pcg_offset = (m_tvram[check_chr_addr()]+(m_kvram[check_chr_addr()]<<8)) & 0xfff;
		pcg_offset*=0x20;
		pcg_offset+=(offset & 0x0f);
		pcg_offset+=(m_kvram[check_chr_addr()] & 0x40) >> 2; //left-right check

		res = gfx_data[pcg_offset];
	}
	else
	{
		UINT8 y_char_size;

		/* addr == 0 reads from the ANK rom */
		gfx_data = addr == 0 ? m_cg_rom : m_pcg_ram;
		y_char_size = ((m_crtc_vreg[9]+1) > 8) ? 8 : m_crtc_vreg[9]+1;
		if(y_char_size == 0) { y_char_size = 1; }
		pcg_offset = m_tvram[get_pcg_addr(m_crtc_vreg[1], y_char_size)]*8;
		pcg_offset+= machine().first_screen()->vpos() & (y_char_size-1);
		if(addr) { pcg_offset+= ((addr-1)*0x800); }
		res = gfx_data[pcg_offset];
	}

	return res;
}

WRITE8_MEMBER( x1_state::x1_pcg_w )
{
	int addr,pcg_offset;

	addr = (offset & 0x300) >> 8;

	if(addr == 0)
	{
		/* NOP */
		logerror("Warning: write to the ANK area! %04x %02x\n",offset,data);
	}
	else
	{
		if(m_scrn_reg.pcg_mode) // Hi-Speed Mode, X1Turbo only
		{
			pcg_offset = m_tvram[check_pcg_addr()]*8;
			pcg_offset+= (offset & 0xe) >> 1;
			pcg_offset+=((addr-1)*0x800);
			m_pcg_ram[pcg_offset] = data;

			pcg_offset &= 0x7ff;

			m_gfxdecode->gfx(3)->mark_dirty(pcg_offset >> 3);
		}
		else // Compatible Mode
		{
			UINT8 y_char_size;

			/* TODO: Brain Breaker doesn't work with this arrangement in high resolution mode, check out why */
			y_char_size = (m_crtc_vreg[9]+1) > 8 ? (m_crtc_vreg[9]+1)-8 : m_crtc_vreg[9]+1;
			if(y_char_size == 0) { y_char_size = 1; }
			pcg_offset = m_tvram[get_pcg_addr(m_crtc_vreg[1], y_char_size)]*8;
			pcg_offset+= machine().first_screen()->vpos() & (y_char_size-1);
			pcg_offset+= ((addr-1)*0x800);

			m_pcg_ram[pcg_offset] = data;

			pcg_offset &= 0x7ff;

			m_gfxdecode->gfx(3)->mark_dirty(pcg_offset >> 3);
		}
	}
}

/*************************************
 *
 *  Other Video-related functions
 *
 *************************************/

/* for bitmap mode */
void x1_state::set_current_palette()
{
	UINT8 addr,r,g,b;

	for(addr=0;addr<8;addr++)
	{
		r = ((m_x_r)>>(addr)) & 1;
		g = ((m_x_g)>>(addr)) & 1;
		b = ((m_x_b)>>(addr)) & 1;

		m_palette->set_pen_color(addr|8, pal1bit(r), pal1bit(g), pal1bit(b));
	}
}

WRITE8_MEMBER( x1_state::x1turboz_4096_palette_w )
{
	UINT32 pal_entry;
	UINT8 r,g,b;

	pal_entry = ((offset & 0xff) << 4) | ((data & 0xf0) >> 4);

	m_pal_4096[pal_entry+((offset & 0x300)<<4)] = data & 0xf;

	r = m_pal_4096[pal_entry+(1<<12)];
	g = m_pal_4096[pal_entry+(2<<12)];
	b = m_pal_4096[pal_entry+(0<<12)];

	m_palette->set_pen_color(pal_entry+16, pal3bit(r), pal3bit(g), pal3bit(b));
}

/* Note: docs claims that reading the palette ports makes the value to change somehow in X1 mode ...
         In 4096 color mode, it's used for reading the value back. */
WRITE8_MEMBER( x1_state::x1_pal_r_w )
{
	if(m_turbo_reg.pal & 0x80) //AEN bit, Turbo Z
	{
		if(m_turbo_reg.gfx_pal & 0x80) //APEN bit
			x1turboz_4096_palette_w(space,offset & 0x3ff,data);
	}
	else //compatible mode
	{
		m_x_r = data;
		set_current_palette();
		//if(m_old_vpos != machine().first_screen()->vpos())
		//{
		//  machine().first_screen()->update_partial(machine().first_screen()->vpos());
		//  m_old_vpos = machine().first_screen()->vpos();
		//}
	}
}

WRITE8_MEMBER( x1_state::x1_pal_g_w )
{
	if(m_turbo_reg.pal & 0x80) //AEN bit, Turbo Z
	{
		if(m_turbo_reg.gfx_pal & 0x80) //APEN bit
			x1turboz_4096_palette_w(space,offset & 0x3ff,data);
	}
	else
	{
		m_x_g = data;
		set_current_palette();
		//if(m_old_vpos != machine().first_screen()->vpos())
		//{
			machine().first_screen()->update_partial(machine().first_screen()->vpos());
		//  m_old_vpos = machine().first_screen()->vpos();
		//}
	}
}

WRITE8_MEMBER( x1_state::x1_pal_b_w )
{
	if(m_turbo_reg.pal & 0x80) //AEN bit, Turbo Z
	{
		if(m_turbo_reg.gfx_pal & 0x80) //APEN bit
			x1turboz_4096_palette_w(space,offset & 0x3ff,data);
	}
	else
	{
		m_x_b = data;
		set_current_palette();
		//if(m_old_vpos != machine().first_screen()->vpos())
		//{
		//  machine().first_screen()->update_partial(machine().first_screen()->vpos());
		//  m_old_vpos = machine().first_screen()->vpos();
		//}
	}
}

WRITE8_MEMBER( x1_state::x1_ex_gfxram_w )
{
	UINT8 ex_mask;

	if     (                    offset <= 0x3fff)   { ex_mask = 7; }
	else if(offset >= 0x4000 && offset <= 0x7fff)   { ex_mask = 6; }
	else if(offset >= 0x8000 && offset <= 0xbfff)   { ex_mask = 5; }
	else                                            { ex_mask = 3; }

	if(ex_mask & 1) { m_gfx_bitmap_ram[(offset & 0x3fff)+0x0000+(m_scrn_reg.gfx_bank*0xc000)] = data; }
	if(ex_mask & 2) { m_gfx_bitmap_ram[(offset & 0x3fff)+0x4000+(m_scrn_reg.gfx_bank*0xc000)] = data; }
	if(ex_mask & 4) { m_gfx_bitmap_ram[(offset & 0x3fff)+0x8000+(m_scrn_reg.gfx_bank*0xc000)] = data; }
}

/*
    SCRN flags

    d0(01) = 0:low resolution (15KHz) 1: high resolution (24KHz)
    d1(02) = 0:1 raster / pixel       1:2 raster / pixel
    d2(04) = 0:8 rasters / CHR        1:16 rasters / CHR
    d3(08) = 0:bank 0                 0:bank 1    <- display
    d4(10) = 0:bank 0                 0:bank 1    <- access
    d5(20) = 0:compatibility          1:high speed  <- define PCG mode
    d6(40) = 0:8-raster graphics      1:16-raster graphics
    d7(80) = 0:don't display          1:display  <- underline (when 1, graphics are not displayed)
*/
WRITE8_MEMBER( x1_state::x1_scrn_w )
{
	m_scrn_reg.pcg_mode = BIT(data, 5);
	m_scrn_reg.gfx_bank = BIT(data, 4);
	m_scrn_reg.disp_bank = BIT(data, 3);
	m_scrn_reg.ank_sel = BIT(data, 2);
	m_scrn_reg.v400_mode = ((data & 0x03) == 3) ? 1 : 0;

	if(data & 0x80)
		printf("SCRN = %02x\n",data & 0x80);
	if((data & 0x03) == 1)
		printf("SCRN sets true 400 lines mode\n");
}

WRITE8_MEMBER( x1_state::x1_pri_w )
{
	m_scrn_reg.pri = data;
//  printf("PRI = %02x\n",data);
}

WRITE8_MEMBER( x1_state::x1_6845_w )
{
	if(offset == 0)
	{
		m_crtc_index = data & 31;
		machine().device<mc6845_device>("crtc")->address_w(space, offset, data);
	}
	else
	{
		m_crtc_vreg[m_crtc_index] = data;
		machine().device<mc6845_device>("crtc")->register_w(space, offset, data);
	}
}

READ8_MEMBER( x1_state::x1turboz_blackclip_r )
{
	/*  TODO: this returns only on x1turboz */
	return m_scrn_reg.blackclip;
}

WRITE8_MEMBER( x1_state::x1turbo_blackclip_w )
{
	/*
	-x-- ---- replace blanking duration with black
	--x- ---- replace bitmap palette 1 with black
	---x ---- replace bitmap palette 0 with black
	---- x--- enable text blackclip
	---- -xxx palette color number for text black
	*/
	m_scrn_reg.blackclip = data;
	if(data & 0x40)
		printf("Blackclip data access %02x\n",data);
}

READ8_MEMBER( x1_state::x1turbo_pal_r )
{
	return m_turbo_reg.pal;
}

READ8_MEMBER( x1_state::x1turbo_txpal_r )
{
	return m_turbo_reg.txt_pal[offset];
}

READ8_MEMBER( x1_state::x1turbo_txdisp_r )
{
	return m_turbo_reg.txt_disp;
}

READ8_MEMBER( x1_state::x1turbo_gfxpal_r )
{
	return m_turbo_reg.gfx_pal;
}

WRITE8_MEMBER( x1_state::x1turbo_pal_w )
{
	printf("TURBO PAL %02x\n",data);
	m_turbo_reg.pal = data;
}

WRITE8_MEMBER( x1_state::x1turbo_txpal_w )
{
	int r,g,b;

	printf("TURBO TEXT PAL %02x %02x\n",data,offset);
	m_turbo_reg.txt_pal[offset] = data;

	if(m_turbo_reg.pal & 0x80)
	{
		r = (data & 0x0c) >> 2;
		g = (data & 0x30) >> 4;
		b = (data & 0x03) >> 0;

		m_palette->set_pen_color(offset, pal2bit(r), pal2bit(g), pal2bit(b));
	}
}

WRITE8_MEMBER( x1_state::x1turbo_txdisp_w )
{
	printf("TURBO TEXT DISPLAY %02x\n",data);
	m_turbo_reg.txt_disp = data;
}

WRITE8_MEMBER( x1_state::x1turbo_gfxpal_w )
{
	printf("TURBO GFX PAL %02x\n",data);
	m_turbo_reg.gfx_pal = data;
}


/*
 *  FIXME: bit-wise this doesn't make any sense, I guess that it uses the lv 2 kanji roms
 *         Test cases for this port so far are Hyper Olympics '84 disk version and Might & Magic.
 */
UINT16 x1_state::jis_convert(int kanji_addr)
{
	if(kanji_addr >= 0x0e00 && kanji_addr <= 0x0e9f) { kanji_addr -= 0x0e00; kanji_addr &= 0x0ff; return ((0x0e0) + (kanji_addr >> 3)) << 4; } // numbers
	if(kanji_addr >= 0x0f00 && kanji_addr <= 0x109f) { kanji_addr -= 0x0f00; kanji_addr &= 0x1ff; return ((0x4c0) + (kanji_addr >> 3)) << 4; } // lower case chars
	if(kanji_addr >= 0x1100 && kanji_addr <= 0x129f) { kanji_addr -= 0x1100; kanji_addr &= 0x1ff; return ((0x2c0) + (kanji_addr >> 3)) << 4; } // upper case chars
	if(kanji_addr >= 0x0100 && kanji_addr <= 0x01ff) { kanji_addr -= 0x0100; kanji_addr &= 0x0ff; return ((0x040) + (kanji_addr >> 3)) << 4; } // grammar symbols
	if(kanji_addr >= 0x0500 && kanji_addr <= 0x06ff) { kanji_addr -= 0x0500; kanji_addr &= 0x1ff; return ((0x240) + (kanji_addr >> 3)) << 4; } // math symbols
	if(kanji_addr >= 0x0300 && kanji_addr <= 0x04ff) { kanji_addr -= 0x0300; kanji_addr &= 0x1ff; return ((0x440) + (kanji_addr >> 3)) << 4; } // parentesis

	if(kanji_addr != 0x0720 && kanji_addr != 0x0730)
		printf("%08x\n",kanji_addr);

	return 0x0000;
}

READ8_MEMBER( x1_state::x1_kanji_r )
{
	UINT8 res;

	res = m_kanji_rom[jis_convert(m_kanji_addr & 0xfff0)+(offset*0x10)+(m_kanji_addr & 0xf)];

	if(offset == 1)
		m_kanji_addr_latch++;

	return res;
}

WRITE8_MEMBER( x1_state::x1_kanji_w )
{
//  if(offset < 2)

	switch(offset)
	{
		case 0: m_kanji_addr_latch = (data & 0xff)|(m_kanji_addr_latch&0xff00); break;
		case 1: m_kanji_addr_latch = (data<<8)|(m_kanji_addr_latch&0x00ff);
			//if(m_kanji_addr_latch != 0x720 && m_kanji_addr_latch != 0x730)
			//  printf("%08x\n",m_kanji_addr_latch);
			break;
		case 2:
		{
			/* 0 -> selects Expanded EEPROM */
			/* 1 -> selects Kanji ROM */
			/* 0 -> 1 -> latches Kanji ROM data */

			if(((m_kanji_eksel & 1) == 0) && ((data & 1) == 1))
			{
				m_kanji_addr = (m_kanji_addr_latch);
				//m_kanji_addr &= 0x3fff; //<- temp kludge until the rom is redumped.
				//printf("%08x\n",m_kanji_addr);
				//m_kanji_addr+= m_kanji_count;
			}
			m_kanji_eksel = data & 1;
		}
		break;
	}
}

READ8_MEMBER( x1_state::x1_emm_r )
{
	UINT8 res;

	if(offset & ~3)
	{
		printf("Warning: read EMM BASIC area [%02x]\n",offset & 0xff);
		return 0xff;
	}

	if(offset != 3)
		printf("Warning: read EMM address [%02x]\n",offset);

	res = 0xff;

	if(offset == 3)
	{
		res = m_emm_ram[m_emm_addr];
		m_emm_addr++;
	}

	return res;
}

WRITE8_MEMBER( x1_state::x1_emm_w )
{
	if(offset & ~3)
	{
		printf("Warning: write EMM BASIC area [%02x] %02x\n",offset & 0xff,data);
		return;
	}

	switch(offset)
	{
		case 0: m_emm_addr = (m_emm_addr & 0xffff00) | (data & 0xff); break;
		case 1: m_emm_addr = (m_emm_addr & 0xff00ff) | (data << 8);   break;
		case 2: m_emm_addr = (m_emm_addr & 0x00ffff) | (data << 16);  break; //TODO: this has a max size limit, check exactly how much
		case 3:
			m_emm_ram[m_emm_addr] = data;
			m_emm_addr++;
			break;
	}
}

/*
    CZ-141SF, CZ-127MF, X1turboZII, X1turboZ3 boards
*/
READ8_MEMBER( x1_state::x1turbo_bank_r )
{
//  printf("BANK access read\n");
	return m_ex_bank & 0x3f;
}

WRITE8_MEMBER( x1_state::x1turbo_bank_w )
{
	//UINT8 *RAM = memregion("x1_cpu")->base();
	/*
	--x- ---- BML5: latch bit (doesn't have any real function)
	---x ---- BMCS: select bank RAM, active low
	---- xxxx BMNO: Bank memory ID
	*/

	m_ex_bank = data & 0x3f;
//  printf("BANK access write %02x\n",data);
}

/* TODO: waitstate penalties */
READ8_MEMBER( x1_state::x1_mem_r )
{
	if((offset & 0x8000) == 0 && (m_ram_bank == 0))
	{
		return m_ipl_rom[offset]; //ROM
	}

	return m_work_ram[offset]; //RAM
}

WRITE8_MEMBER( x1_state::x1_mem_w )
{
	m_work_ram[offset] = data; //RAM
}

READ8_MEMBER( x1_state::x1turbo_mem_r )
{
	if((m_ex_bank & 0x10) == 0)
		return m_work_ram[offset+((m_ex_bank & 0xf)*0x10000)];

	return x1_mem_r(space,offset);
}

WRITE8_MEMBER( x1_state::x1turbo_mem_w )
{
	if((m_ex_bank & 0x10) == 0)
		m_work_ram[offset+((m_ex_bank & 0xf)*0x10000)] = data; //RAM
	else
		x1_mem_w(space,offset,data);
}

/*************************************
 *
 *  Memory maps
 *
 *************************************/

READ8_MEMBER( x1_state::x1_io_r )
{
	m_io_bank_mode = 0; //any read disables the extended mode.

	if(offset == 0x0e03)                            { return x1_rom_r(space, 0); }
	// TODO: user could install ym2151 on plain X1 too
	//0x700, 0x701
	//if(offset >= 0x0704 && offset <= 0x0707)      { return z80ctc_r(m_ctc, offset-0x0704); }
	else if(offset >= 0x0ff8 && offset <= 0x0fff)   { return x1_fdc_r(space, offset-0xff8); }
	else if(offset >= 0x1400 && offset <= 0x17ff)   { return x1_pcg_r(space, offset-0x1400); }
	else if(offset >= 0x1900 && offset <= 0x19ff)   { return x1_sub_io_r(space, 0); }
	else if(offset >= 0x1a00 && offset <= 0x1aff)   { return machine().device<i8255_device>("ppi8255_0")->read(space, (offset-0x1a00) & 3); }
	else if(offset >= 0x1b00 && offset <= 0x1bff)   { return machine().device<ay8910_device>("ay")->data_r(space, 0); }
//  else if(offset >= 0x1f80 && offset <= 0x1f8f)   { return z80dma_r(machine().device("dma"), 0); }
//  else if(offset >= 0x1f90 && offset <= 0x1f91)   { return z80sio_c_r(machine().device("sio"), (offset-0x1f90) & 1); }
//  else if(offset >= 0x1f92 && offset <= 0x1f93)   { return z80sio_d_r(machine().device("sio"), (offset-0x1f92) & 1); }
	else if(offset >= 0x1fa0 && offset <= 0x1fa3)   { return m_ctc->read(space,offset-0x1fa0); }
	else if(offset >= 0x1fa8 && offset <= 0x1fab)   { return m_ctc->read(space,offset-0x1fa8); }
//  else if(offset >= 0x1fd0 && offset <= 0x1fdf)   { return x1_scrn_r(space,offset-0x1fd0); }
//  else if(offset == 0x1fe0)                       { return x1turboz_blackclip_r(space,0); }
	else if(offset >= 0x2000 && offset <= 0x2fff)   { return m_avram[offset & 0x7ff]; }
	else if(offset >= 0x3000 && offset <= 0x3fff)   { return m_tvram[offset & 0x7ff]; } // Ys checks if it's a x1/x1turbo machine by checking if this area is a mirror
	else if(offset >= 0x4000 && offset <= 0xffff)   { return m_gfx_bitmap_ram[offset-0x4000+(m_scrn_reg.gfx_bank*0xc000)]; }
	else
	{
		//logerror("(PC=%06x) Read i/o address %04x\n",space.device().safe_pc(),offset);
	}
	return 0xff;
}

WRITE8_MEMBER( x1_state::x1_io_w )
{
	if(m_io_bank_mode == 1)                     { x1_ex_gfxram_w(space, offset, data); }
	// TODO: user could install ym2151 on plain X1 too
	//0x700, 0x701
//  else if(offset >= 0x0704 && offset <= 0x0707)   { z80ctc_w(m_ctc, offset-0x0704,data); }
//  else if(offset >= 0x0c00 && offset <= 0x0cff)   { x1_rs232c_w(machine(), 0, data); }
	else if(offset >= 0x0e00 && offset <= 0x0e02)   { x1_rom_w(space, offset-0xe00,data); }
//  else if(offset >= 0x0e80 && offset <= 0x0e82)   { x1_kanji_w(machine(), offset-0xe80,data); }
	else if(offset >= 0x0ff8 && offset <= 0x0fff)   { x1_fdc_w(space, offset-0xff8,data); }
	else if(offset >= 0x1000 && offset <= 0x10ff)   { x1_pal_b_w(space, 0,data); }
	else if(offset >= 0x1100 && offset <= 0x11ff)   { x1_pal_r_w(space, 0,data); }
	else if(offset >= 0x1200 && offset <= 0x12ff)   { x1_pal_g_w(space, 0,data); }
	else if(offset >= 0x1300 && offset <= 0x13ff)   { x1_pri_w(space, 0,data); }
	else if(offset >= 0x1400 && offset <= 0x17ff)   { x1_pcg_w(space, offset-0x1400,data); }
	else if(offset == 0x1800 || offset == 0x1801)   { x1_6845_w(space, offset-0x1800, data); }
	else if(offset >= 0x1900 && offset <= 0x19ff)   { x1_sub_io_w(space, 0,data); }
	else if(offset >= 0x1a00 && offset <= 0x1aff)   { machine().device<i8255_device>("ppi8255_0")->write(space, (offset-0x1a00) & 3,data); }
	else if(offset >= 0x1b00 && offset <= 0x1bff)   { machine().device<ay8910_device>("ay")->data_w(space, 0,data); }
	else if(offset >= 0x1c00 && offset <= 0x1cff)   { machine().device<ay8910_device>("ay")->address_w(space, 0,data); }
	else if(offset >= 0x1d00 && offset <= 0x1dff)   { x1_rom_bank_1_w(space,0,data); }
	else if(offset >= 0x1e00 && offset <= 0x1eff)   { x1_rom_bank_0_w(space,0,data); }
//  else if(offset >= 0x1f80 && offset <= 0x1f8f)   { z80dma_w(machine().device("dma"), 0,data); }
//  else if(offset >= 0x1f90 && offset <= 0x1f91)   { z80sio_c_w(machine().device("sio"), (offset-0x1f90) & 1,data); }
//  else if(offset >= 0x1f92 && offset <= 0x1f93)   { z80sio_d_w(machine().device("sio"), (offset-0x1f92) & 1,data); }
	else if(offset >= 0x1fa0 && offset <= 0x1fa3)   { m_ctc->write(space,offset-0x1fa0,data); }
	else if(offset >= 0x1fa8 && offset <= 0x1fab)   { m_ctc->write(space,offset-0x1fa8,data); }
//  else if(offset == 0x1fb0)                       { x1turbo_pal_w(space,0,data); }
//  else if(offset >= 0x1fb9 && offset <= 0x1fbf)   { x1turbo_txpal_w(space,offset-0x1fb9,data); }
//  else if(offset == 0x1fc0)                       { x1turbo_txdisp_w(space,0,data); }
//  else if(offset == 0x1fc5)                       { x1turbo_gfxpal_w(space,0,data); }
//  else if(offset >= 0x1fd0 && offset <= 0x1fdf)   { x1_scrn_w(space,0,data); }
//  else if(offset == 0x1fe0)                       { x1turbo_blackclip_w(space,0,data); }
	else if(offset >= 0x2000 && offset <= 0x2fff)   { m_avram[offset & 0x7ff] = data; }
	else if(offset >= 0x3000 && offset <= 0x3fff)   { m_tvram[offset & 0x7ff] = data; }
	else if(offset >= 0x4000 && offset <= 0xffff)   { m_gfx_bitmap_ram[offset-0x4000+(m_scrn_reg.gfx_bank*0xc000)] = data; }
	else
	{
		//logerror("(PC=%06x) Write %02x at i/o address %04x\n",space.device().safe_pc(),data,offset);
	}
}

/* TODO: I should actually simplify this, by just overwriting X1 Turbo specifics here, and call plain X1 functions otherwise */
READ8_MEMBER( x1_state::x1turbo_io_r )
{
	m_io_bank_mode = 0; //any read disables the extended mode.

	// a * at the end states devices used on plain X1 too
	if(offset == 0x0700)                            { return (machine().device<ym2151_device>("ym")->read(space, offset-0x0700) & 0x7f) | (ioport("SOUND_SW")->read() & 0x80); }
	else if(offset == 0x0701)                       { return machine().device<ym2151_device>("ym")->read(space, offset-0x0700); }
	//0x704 is FM sound detection port on X1 turboZ
	else if(offset >= 0x0704 && offset <= 0x0707)   { return m_ctc->read(space,offset-0x0704); }
	else if(offset == 0x0801)                       { printf("Color image board read\n"); return 0xff; } // *
	else if(offset == 0x0803)                       { printf("Color image board 2 read\n"); return 0xff; } // *
	else if(offset >= 0x0a00 && offset <= 0x0a07)   { printf("Stereoscopic board read %04x\n",offset); return 0xff; } // *
	else if(offset == 0x0b00)                       { return x1turbo_bank_r(space,0); }
	else if(offset >= 0x0c00 && offset <= 0x0cff)   { printf("RS-232C read %04x\n",offset); return 0; } // *
	else if(offset >= 0x0d00 && offset <= 0x0dff)   { return x1_emm_r(space,offset & 0xff); } // *
	else if(offset == 0x0e03)                       { return x1_rom_r(space, 0); }
	else if(offset >= 0x0e80 && offset <= 0x0e81)   { return x1_kanji_r(space, offset-0xe80); }
	else if(offset >= 0x0fd0 && offset <= 0x0fd3)   { /* printf("SASI HDD read %04x\n",offset); */ return 0xff; } // *
	else if(offset >= 0x0fe8 && offset <= 0x0fef)   { printf("8-inch FD read %04x\n",offset); return 0xff; } // *
	else if(offset >= 0x0ff8 && offset <= 0x0fff)   { return x1_fdc_r(space, offset-0xff8); }
	else if(offset >= 0x1400 && offset <= 0x17ff)   { return x1_pcg_r(space, offset-0x1400); }
	else if(offset >= 0x1900 && offset <= 0x19ff)   { return x1_sub_io_r(space, 0); }
	else if(offset >= 0x1a00 && offset <= 0x1aff)   { return machine().device<i8255_device>("ppi8255_0")->read(space, (offset-0x1a00) & 3); }
	else if(offset >= 0x1b00 && offset <= 0x1bff)   { return machine().device<ay8910_device>("ay")->data_r(space, 0); }
	else if(offset >= 0x1f80 && offset <= 0x1f8f)   { return m_dma->read(space, 0); }
	else if(offset >= 0x1f90 && offset <= 0x1f93)   { return machine().device<z80sio0_device>("sio")->ba_cd_r(space, (offset-0x1f90) & 3); }
	else if(offset >= 0x1f98 && offset <= 0x1f9f)   { printf("Extended SIO/CTC read %04x\n",offset); return 0xff; }
	else if(offset >= 0x1fa0 && offset <= 0x1fa3)   { return m_ctc->read(space,offset-0x1fa0); }
	else if(offset >= 0x1fa8 && offset <= 0x1fab)   { return m_ctc->read(space,offset-0x1fa8); }
	else if(offset == 0x1fb0)                       { return x1turbo_pal_r(space,0); } // Z only!
	else if(offset >= 0x1fb8 && offset <= 0x1fbf)   { return x1turbo_txpal_r(space,offset-0x1fb8); } //Z only!
	else if(offset == 0x1fc0)                       { return x1turbo_txdisp_r(space,0); } // Z only!
	else if(offset == 0x1fc5)                       { return x1turbo_gfxpal_r(space,0); } // Z only!
//  else if(offset >= 0x1fd0 && offset <= 0x1fdf)   { return x1_scrn_r(space,offset-0x1fd0); } //Z only
	else if(offset == 0x1fe0)                       { return x1turboz_blackclip_r(space,0); }
	else if(offset == 0x1ff0)                       { return ioport("X1TURBO_DSW")->read(); }
	else if(offset >= 0x2000 && offset <= 0x2fff)   { return m_avram[offset & 0x7ff]; }
	else if(offset >= 0x3000 && offset <= 0x37ff)   { return m_tvram[offset & 0x7ff]; }
	else if(offset >= 0x3800 && offset <= 0x3fff)   { return m_kvram[offset & 0x7ff]; }
	else if(offset >= 0x4000 && offset <= 0xffff)   { return m_gfx_bitmap_ram[offset-0x4000+(m_scrn_reg.gfx_bank*0xc000)]; }
	else
	{
		//logerror("(PC=%06x) Read i/o address %04x\n",space.device().safe_pc(),offset);
	}
	return 0xff;
}

WRITE8_MEMBER( x1_state::x1turbo_io_w )
{
	// a * at the end states devices used on plain X1 too
	if(m_io_bank_mode == 1)                    { x1_ex_gfxram_w(space, offset, data); }
	else if(offset == 0x0700 || offset == 0x0701)   { machine().device<ym2151_device>("ym")->write(space, offset-0x0700,data); }
	//0x704 is FM sound detection port on X1 turboZ
	else if(offset >= 0x0704 && offset <= 0x0707)   { m_ctc->write(space,offset-0x0704,data); }
	else if(offset == 0x0800)                       { printf("Color image board write %02x\n",data); } // *
	else if(offset == 0x0802)                       { printf("Color image board 2 write %02x\n",data); } // *
	else if(offset >= 0x0a00 && offset <= 0x0a07)   { printf("Stereoscopic board write %04x %02x\n",offset,data); } // *
	else if(offset == 0x0b00)                       { x1turbo_bank_w(space,0,data); }
	else if(offset >= 0x0c00 && offset <= 0x0cff)   { printf("RS-232C write %04x %02x\n",offset,data); } // *
	else if(offset >= 0x0d00 && offset <= 0x0dff)   { x1_emm_w(space,offset & 0xff,data); } // *
	else if(offset >= 0x0e00 && offset <= 0x0e02)   { x1_rom_w(space, offset-0xe00,data); }
	else if(offset >= 0x0e80 && offset <= 0x0e83)   { x1_kanji_w(space, offset-0xe80,data); }
	else if(offset >= 0x0fd0 && offset <= 0x0fd3)   { printf("SASI HDD write %04x %02x\n",offset,data); } // *
	else if(offset >= 0x0fe8 && offset <= 0x0fef)   { printf("8-inch FD write %04x %02x\n",offset,data); } // *
	else if(offset >= 0x0ff8 && offset <= 0x0fff)   { x1_fdc_w(space, offset-0xff8,data); }
	else if(offset >= 0x1000 && offset <= 0x10ff)   { x1_pal_b_w(space, offset & 0x3ff,data); }
	else if(offset >= 0x1100 && offset <= 0x11ff)   { x1_pal_r_w(space, offset & 0x3ff,data); }
	else if(offset >= 0x1200 && offset <= 0x12ff)   { x1_pal_g_w(space, offset & 0x3ff,data); }
	else if(offset >= 0x1300 && offset <= 0x13ff)   { x1_pri_w(space, 0,data); }
	else if(offset >= 0x1400 && offset <= 0x17ff)   { x1_pcg_w(space, offset-0x1400,data); }
	else if(offset == 0x1800 || offset == 0x1801)   { x1_6845_w(space, offset-0x1800, data); }
	else if(offset >= 0x1900 && offset <= 0x19ff)   { x1_sub_io_w(space, 0,data); }
	else if(offset >= 0x1a00 && offset <= 0x1aff)   { machine().device<i8255_device>("ppi8255_0")->write(space, (offset-0x1a00) & 3,data); }
	else if(offset >= 0x1b00 && offset <= 0x1bff)   { machine().device<ay8910_device>("ay")->data_w(space, 0,data); }
	else if(offset >= 0x1c00 && offset <= 0x1cff)   { machine().device<ay8910_device>("ay")->address_w(space, 0,data); }
	else if(offset >= 0x1d00 && offset <= 0x1dff)   { x1_rom_bank_1_w(space,0,data); }
	else if(offset >= 0x1e00 && offset <= 0x1eff)   { x1_rom_bank_0_w(space,0,data); }
	else if(offset >= 0x1f80 && offset <= 0x1f8f)   { m_dma->write(space, 0,data); }
	else if(offset >= 0x1f90 && offset <= 0x1f93)   { machine().device<z80sio0_device>("sio")->ba_cd_w(space, (offset-0x1f90) & 3,data); }
	else if(offset >= 0x1f98 && offset <= 0x1f9f)   { printf("Extended SIO/CTC write %04x %02x\n",offset,data); }
	else if(offset >= 0x1fa0 && offset <= 0x1fa3)   { m_ctc->write(space,offset-0x1fa0,data); }
	else if(offset >= 0x1fa8 && offset <= 0x1fab)   { m_ctc->write(space,offset-0x1fa8,data); }
	else if(offset == 0x1fb0)                       { x1turbo_pal_w(space,0,data); } // Z only!
	else if(offset >= 0x1fb8 && offset <= 0x1fbf)   { x1turbo_txpal_w(space,offset-0x1fb8,data); } //Z only!
	else if(offset == 0x1fc0)                       { x1turbo_txdisp_w(space,0,data); } //Z only!
	else if(offset == 0x1fc1)                       { printf("Z image capturing access %02x\n",data); } // Z only!
	else if(offset == 0x1fc2)                       { printf("Z mosaic effect access %02x\n",data); } // Z only!
	else if(offset == 0x1fc3)                       { printf("Z Chroma key access %02x\n",data); } // Z only!
	else if(offset == 0x1fc4)                       { printf("Z Extra scroll config access %02x\n",data); } // Z only!
	else if(offset == 0x1fc5)                       { x1turbo_gfxpal_w(space,0,data); } // Z only!
	else if(offset >= 0x1fd0 && offset <= 0x1fdf)   { x1_scrn_w(space,0,data); }
	else if(offset == 0x1fe0)                       { x1turbo_blackclip_w(space,0,data); }
	else if(offset >= 0x2000 && offset <= 0x2fff)   { m_avram[offset & 0x7ff] = data; }
	else if(offset >= 0x3000 && offset <= 0x37ff)   { m_tvram[offset & 0x7ff] = data; }
	else if(offset >= 0x3800 && offset <= 0x3fff)   { m_kvram[offset & 0x7ff] = data; }
	else if(offset >= 0x4000 && offset <= 0xffff)   { m_gfx_bitmap_ram[offset-0x4000+(m_scrn_reg.gfx_bank*0xc000)] = data; }
	else
	{
		//logerror("(PC=%06x) Write %02x at i/o address %04x\n",space.device().safe_pc(),data,offset);
	}
}

static ADDRESS_MAP_START( x1_mem, AS_PROGRAM, 8, x1_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(x1_mem_r,x1_mem_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( x1turbo_mem, AS_PROGRAM, 8, x1_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(x1turbo_mem_r,x1turbo_mem_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( x1_io, AS_IO, 8, x1_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(x1_io_r, x1_io_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( x1turbo_io, AS_IO, 8, x1_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(x1turbo_io_r, x1turbo_io_w)
ADDRESS_MAP_END

/*************************************
 *
 *  PPI8255
 *
 *************************************/

READ8_MEMBER( x1_state::x1_porta_r )
{
	printf("PPI Port A read\n");
	return 0xff;
}

/* this port is system related */
READ8_MEMBER( x1_state::x1_portb_r )
{
	//printf("PPI Port B read\n");
	/*
	x--- ---- "v disp"
	-x-- ---- "sub cpu ibf"
	--x- ---- "sub cpu obf"
	---x ---- ROM/RAM flag (0=ROM, 1=RAM)
	---- x--- "busy" <- allow printer data output
	---- -x-- "v sync"
	---- --x- "cmt read"
	---- ---x "cmt test" (active low) <- actually this is "Sub CPU detected BREAK"
	*/
	UINT8 res = 0;
	int vblank_line = m_crtc_vreg[6] * (m_crtc_vreg[9]+1);
	int vsync_line = m_crtc_vreg[7] * (m_crtc_vreg[9]+1);
	m_vdisp = (machine().first_screen()->vpos() < vblank_line) ? 0x80 : 0x00;
	m_vsync = (machine().first_screen()->vpos() < vsync_line) ? 0x00 : 0x04;

//  popmessage("%d",vsync_line);
//  popmessage("%d",vblank_line);

	res = m_ram_bank | m_sub_obf | m_vsync | m_vdisp;

	if(m_cassette->input() > 0.03)
		res |= 0x02;

//  if(cassette_get_state(m_cassette) & CASSETTE_MOTOR_DISABLED)
//      res &= ~0x02;  // is zero if not playing

	// CMT test bit is set low when the CMT Stop command is issued, and becomes
	// high again when this bit is read.
	res |= 0x01;
	if(m_cmt_test != 0)
	{
		m_cmt_test = 0;
		res &= ~0x01;
	}

	return res;
}

/* I/O system port */
READ8_MEMBER( x1_state::x1_portc_r )
{
	//printf("PPI Port C read\n");
	/*
	x--- ---- Printer port output
	-x-- ---- 320 mode (r/w), divider for the pixel clock
	--x- ---- i/o mode (r/w)
	---x ---- smooth scroll enabled (?)
	---- ---x cassette output data
	*/
	return (m_io_sys & 0x9f) | m_hres_320 | ~m_io_switch;
}

WRITE8_MEMBER( x1_state::x1_porta_w )
{
	//printf("PPI Port A write %02x\n",data);
}

WRITE8_MEMBER( x1_state::x1_portb_w )
{
	//printf("PPI Port B write %02x\n",data);
}

WRITE8_MEMBER( x1_state::x1_portc_w )
{
	m_hres_320 = data & 0x40;

	/* set up the pixel clock according to the above divider */
	m_crtc->set_clock(VDP_CLOCK/((m_hres_320) ? 48 : 24));

	if(!BIT(data, 5) && BIT(m_io_switch, 5))
		m_io_bank_mode = 1;

	m_io_switch = data & 0x20;
	m_io_sys = data & 0xff;

	m_cassette->output(BIT(data, 0) ? +1.0 : -1.0);
}

READ8_MEMBER(x1_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(x1_state::memory_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}

READ8_MEMBER(x1_state::io_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(x1_state::io_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.write_byte(offset, data);
}

/*************************************
 *
 *  Inputs
 *
 *************************************/

INPUT_CHANGED_MEMBER(x1_state::ipl_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);

	m_ram_bank = 0x00;
	if(m_is_turbo) { m_ex_bank = 0x10; }
	//anything else?
}

/* Apparently most games doesn't support this (not even the Konami ones!), one that does is...177 :o */
INPUT_CHANGED_MEMBER(x1_state::nmi_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_PORTS_START( x1 )
	PORT_START("FP_SYS") //front panel buttons, hard-wired with the soft reset/NMI lines
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, x1_state, ipl_reset,0) PORT_NAME("IPL reset")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, x1_state, nmi_reset,0) PORT_NAME("NMI reset")

	PORT_START("SOUND_SW") //FIXME: this is X1Turbo specific
	PORT_DIPNAME( 0x80, 0x80, "OPM Sound Setting?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IOSYS") //TODO: implement front-panel DIP-SW here
	PORT_DIPNAME( 0x01, 0x01, "IOSYS" )
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
	PORT_DIPNAME( 0x80, 0x80, "Sound Setting?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("key1") //0x00-0x1f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_UNUSED) //0x00 null
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-4") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-5") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-6") /*PORT_CODE(KEYCODE_5) PORT_CHAR('5')*/
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-7") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-4") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-5") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(27)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-7") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-8") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-4") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-5") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-6") /*PORT_CODE(KEYCODE_5) PORT_CHAR('5')*/
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-7") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-8") /*PORT_CODE(KEYCODE_5) PORT_CHAR('5')*/
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)

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
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2a *
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2b +
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2c ,
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2e .
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2f /

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
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';')
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

	PORT_START("f_keys")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)

	PORT_START("tenkey")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey -") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey +") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey *") PORT_CODE(KEYCODE_ASTERISK)
	// TODO: add other numpad keys

	PORT_START("key_modifiers")
	PORT_BIT(0x00000001,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x00000002,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x00000004,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("KANA") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x00000008,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("CAPS") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x00000010,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("GRPH") PORT_CODE(KEYCODE_LALT)

#if 0
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey *") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey /") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey +") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey -") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey =")
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xEF\xBF\xA5")

	PORT_START("key3")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey ,")
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey Enter") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey .") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("INS") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("EL") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CLS") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DUP") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF6") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF7") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF8") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF9") PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF10") PORT_CODE(KEYCODE_F10)

#endif
INPUT_PORTS_END

INPUT_PORTS_START( x1turbo )
	PORT_INCLUDE( x1 )

	PORT_START("X1TURBO_DSW")
	PORT_DIPNAME( 0x01, 0x01, "Interlace mode" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x00, "Default Auto-boot Device" ) // this selects what kind of device is loaded at start-up
	PORT_DIPSETTING(    0x00, "5/3-inch 2D" )
	PORT_DIPSETTING(    0x02, "5/3-inch 2DD" )
	PORT_DIPSETTING(    0x04, "5/3-inch 2HD" )
	PORT_DIPSETTING(    0x06, "5/3-inch 2DD (IBM)" )
	PORT_DIPSETTING(    0x08, "8-inch 2D256" )
	PORT_DIPSETTING(    0x0a, "8-inch 2D256 (IBM)" )
	PORT_DIPSETTING(    0x0c, "8-inch 1S128 (IBM)" )
	PORT_DIPSETTING(    0x0e, "SASI HDD" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) //this is a port conditional of some sort ...
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/*************************************
 *
 *  GFX decoding
 *
 *************************************/

static const gfx_layout x1_chars_8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout x1_chars_8x16 =
{
	8,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	8*16
};

static const gfx_layout x1_chars_16x16 =
{
	8,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};

/* decoded for debugging purpose, this will be nuked in the end... */
static GFXDECODE_START( x1 )
	GFXDECODE_ENTRY( "cgrom",   0x00000, x1_chars_8x8,    0, 1 )
	GFXDECODE_ENTRY( "font",    0x00000, x1_chars_8x16,   0, 1 )
	GFXDECODE_ENTRY( "kanji",   0x00000, x1_chars_16x16,  0, 1 )
//  GFXDECODE_ENTRY( "pcg",     0x00000, x1_pcg_8x8,      0, 1 )
GFXDECODE_END

static const z80_daisy_config x1_daisy[] =
{
	{ "x1kb" },
	{ "ctc" },
	{ NULL }
};

static const z80_daisy_config x1turbo_daisy[] =
{
	{ "x1kb" },
	{ "ctc" },
	{ "dma" },
	{ "sio" },
	{ NULL }
};

/*************************************
 *
 *  Machine Functions
 *
 *************************************/

#ifdef UNUSED_FUNCTION
IRQ_CALLBACK_MEMBER(x1_state::x1_irq_callback)
{
	if(m_ctc_irq_flag != 0)
	{
		m_ctc_irq_flag = 0;
		if(m_key_irq_flag == 0)  // if no other devices are pulling the IRQ line high
			device.execute().set_input_line(0, CLEAR_LINE);
		return m_irq_vector;
	}
	if(m_key_irq_flag != 0)
	{
		m_key_irq_flag = 0;
		if(m_ctc_irq_flag == 0)  // if no other devices are pulling the IRQ line high
			device.execute().set_input_line(0, CLEAR_LINE);
		return m_key_irq_vector;
	}
	return m_irq_vector;
}
#endif

TIMER_DEVICE_CALLBACK_MEMBER(x1_state::x1_keyboard_callback)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT32 key1 = ioport("key1")->read();
	UINT32 key2 = ioport("key2")->read();
	UINT32 key3 = ioport("key3")->read();
	UINT32 key4 = ioport("tenkey")->read();
	UINT32 f_key = ioport("f_keys")->read();

	if(m_key_irq_vector)
	{
		//if(key1 == 0 && key2 == 0 && key3 == 0 && key4 == 0 && f_key == 0)
		//  return;

		if((key1 != m_old_key1) || (key2 != m_old_key2) || (key3 != m_old_key3) || (key4 != m_old_key4) || (f_key != m_old_fkey))
		{
			// generate keyboard IRQ
			x1_sub_io_w(space,0,0xe6);
			m_irq_vector = m_key_irq_vector;
			m_key_irq_flag = 1;
			m_maincpu->set_input_line(0,ASSERT_LINE);
			m_old_key1 = key1;
			m_old_key2 = key2;
			m_old_key3 = key3;
			m_old_key4 = key4;
			m_old_fkey = f_key;
		}
	}
}

TIMER_CALLBACK_MEMBER(x1_state::x1_rtc_increment)
{
	static const UINT8 dpm[12] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };

	m_rtc.sec++;

	if((m_rtc.sec & 0x0f) >= 0x0a)              { m_rtc.sec+=0x10; m_rtc.sec&=0xf0; }
	if((m_rtc.sec & 0xf0) >= 0x60)              { m_rtc.min++; m_rtc.sec = 0; }
	if((m_rtc.min & 0x0f) >= 0x0a)              { m_rtc.min+=0x10; m_rtc.min&=0xf0; }
	if((m_rtc.min & 0xf0) >= 0x60)              { m_rtc.hour++; m_rtc.min = 0; }
	if((m_rtc.hour & 0x0f) >= 0x0a)             { m_rtc.hour+=0x10; m_rtc.hour&=0xf0; }
	if((m_rtc.hour & 0xff) >= 0x24)             { m_rtc.day++; m_rtc.wday++; m_rtc.hour = 0; }
	if((m_rtc.wday & 0x0f) >= 0x07)             { m_rtc.wday = 0; }
	if((m_rtc.day & 0x0f) >= 0x0a)              { m_rtc.day+=0x10; m_rtc.day&=0xf0; }
	/* FIXME: very crude leap year support (i.e. it treats the RTC to be with a 2000-2099 timeline), dunno how the real x1 supports this,
	   maybe it just have a 1980-1999 timeline since year 0x00 shows as a XX on display */
	if(((m_rtc.year % 4) == 0) && m_rtc.month == 2)
	{
		if((m_rtc.day & 0xff) >= dpm[m_rtc.month-1]+1+1)
			{ m_rtc.month++; m_rtc.day = 0x01; }
	}
	else if((m_rtc.day & 0xff) >= dpm[m_rtc.month-1]+1){ m_rtc.month++; m_rtc.day = 0x01; }
	if(m_rtc.month > 12)                            { m_rtc.year++;  m_rtc.month = 0x01; }
	if((m_rtc.year & 0x0f) >= 0x0a)             { m_rtc.year+=0x10; m_rtc.year&=0xf0; }
	if((m_rtc.year & 0xf0) >= 0xa0)             { m_rtc.year = 0; } //roll over
}

MACHINE_RESET_MEMBER(x1_state,x1)
{
	//UINT8 *ROM = memregion("x1_cpu")->base();
	int i;

	memset(m_gfx_bitmap_ram,0x00,0xc000*2);

	for(i=0;i<0x1800;i++)
	{
		m_pcg_ram[i] = 0;
		m_gfxdecode->gfx(3)->mark_dirty(i >> 3);
	}

	m_is_turbo = 0;

	m_io_bank_mode = 0;

	//m_x1_cpu->set_irq_acknowledge_callback(device_irq_acknowledge_delegate(FUNC(x1_state::x1_irq_callback),this));

	m_cmt_current_cmd = 0;
	m_cmt_test = 0;
	m_cassette->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);

	m_key_irq_flag = m_ctc_irq_flag = 0;
	m_sub_cmd = 0;
	m_key_irq_vector = 0;
	m_sub_cmd_length = 0;
	m_sub_val[0] = 0;
	m_sub_val[1] = 0;
	m_sub_val[2] = 0;
	m_sub_val[3] = 0;
	m_sub_val[4] = 0;
	m_sub_obf = (m_sub_cmd_length) ? 0x00 : 0x20;

	m_rtc_timer->adjust(attotime::zero, 0, attotime::from_seconds(1));

	/* Reinitialize palette here if there's a soft reset for the Turbo PAL stuff*/
	for(i=0;i<0x10;i++)
		m_palette->set_pen_color(i, pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0));

	m_ram_bank = 0;
//  m_old_vpos = -1;
}

MACHINE_RESET_MEMBER(x1_state,x1turbo)
{
	MACHINE_RESET_CALL_MEMBER( x1 );
	m_is_turbo = 1;
	m_ex_bank = 0x10;

	m_scrn_reg.blackclip = 0;
}

static const gfx_layout x1_pcg_8x8 =
{
	8,8,
	0x100,
	3,
	{ 0x1000*8,0x800*8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

MACHINE_START_MEMBER(x1_state,x1)
{
	/* set up RTC */
	{
		system_time systime;
		machine().base_datetime(systime);

		m_rtc.day = ((systime.local_time.mday / 10)<<4) | ((systime.local_time.mday % 10) & 0xf);
		m_rtc.month = ((systime.local_time.month+1));
		m_rtc.wday = ((systime.local_time.weekday % 10) & 0xf);
		m_rtc.year = (((systime.local_time.year % 100)/10)<<4) | ((systime.local_time.year % 10) & 0xf);
		m_rtc.hour = ((systime.local_time.hour / 10)<<4) | ((systime.local_time.hour % 10) & 0xf);
		m_rtc.min = ((systime.local_time.minute / 10)<<4) | ((systime.local_time.minute % 10) & 0xf);
		m_rtc.sec = ((systime.local_time.second / 10)<<4) | ((systime.local_time.second % 10) & 0xf);

		m_rtc_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(x1_state::x1_rtc_increment),this));
	}

	m_ipl_rom = memregion("ipl")->base();
	m_work_ram = auto_alloc_array_clear(machine(), UINT8, 0x10000*0x10);
	m_emm_ram = auto_alloc_array_clear(machine(), UINT8, 0x1000000);
	m_pcg_ram = auto_alloc_array_clear(machine(), UINT8, 0x1800);
	m_cg_rom = memregion("cgrom")->base();
	m_kanji_rom = memregion("kanji")->base();

	save_pointer(NAME(m_work_ram), 0x10000*0x10);
	save_pointer(NAME(m_emm_ram), 0x1000000);
	save_pointer(NAME(m_pcg_ram), 0x1800);

	m_gfxdecode->set_gfx(3, global_alloc(gfx_element(m_palette, x1_pcg_8x8, (UINT8 *)m_pcg_ram, 0, 1, 0)));
}

PALETTE_INIT_MEMBER(x1_state,x1)
{
	int i;

	for(i=0;i<(0x10+0x1000);i++)
		palette.set_pen_color(i,rgb_t(0x00,0x00,0x00));
}

FLOPPY_FORMATS_MEMBER( x1_state::floppy_formats )
	FLOPPY_2D_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( x1_floppies )
	SLOT_INTERFACE("dd", FLOPPY_525_DD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( x1, x1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("x1_cpu", Z80, MAIN_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(x1_mem)
	MCFG_CPU_IO_MAP(x1_io)
	MCFG_CPU_CONFIG(x1_daisy)

	MCFG_DEVICE_ADD("ctc", Z80CTC, MAIN_CLOCK/4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("x1_cpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE("ctc", z80ctc_device, trg3))
	MCFG_Z80CTC_ZC1_CB(DEVWRITELINE("ctc", z80ctc_device, trg1))
	MCFG_Z80CTC_ZC2_CB(DEVWRITELINE("ctc", z80ctc_device, trg2))

	MCFG_DEVICE_ADD("x1kb", X1_KEYBOARD, 0)

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(x1_state, x1_porta_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(x1_state, x1_porta_w))
	MCFG_I8255_IN_PORTB_CB(READ8(x1_state, x1_portb_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(x1_state, x1_portb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(x1_state, x1_portc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(x1_state, x1_portc_w))

	MCFG_MACHINE_START_OVERRIDE(x1_state,x1)
	MCFG_MACHINE_RESET_OVERRIDE(x1_state,x1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(x1_state, screen_update_x1)

	MCFG_MC6845_ADD("crtc", H46505, "screen", (VDP_CLOCK/48)) //unknown divider
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(8)

	MCFG_PALETTE_ADD("palette", 0x10+0x1000)
	MCFG_PALETTE_INIT_OWNER(x1_state,x1)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", x1)

	MCFG_VIDEO_START_OVERRIDE(x1_state,x1)

	MCFG_MB8877_ADD("fdc", MAIN_CLOCK / 16)

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", x1_floppies, "dd", x1_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", x1_floppies, "dd", x1_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:2", x1_floppies, "dd", x1_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:3", x1_floppies, "dd", x1_state::floppy_formats)

	MCFG_SOFTWARE_LIST_ADD("flop_list","x1_flop")

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "x1_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	/* TODO:is the AY mono or stereo? Also volume balance isn't right. */
	MCFG_SOUND_ADD("ay", AY8910, MAIN_CLOCK/8)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("P1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("P2"))
	MCFG_SOUND_ROUTE(0, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "lspeaker",  0.5)
	MCFG_SOUND_ROUTE(2, "rspeaker", 0.5)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.10)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(x1_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)
	MCFG_CASSETTE_INTERFACE("x1_cass")

	MCFG_SOFTWARE_LIST_ADD("cass_list","x1_cass")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard_timer", x1_state, x1_keyboard_callback, attotime::from_hz(250))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("cmt_wind_timer", x1_state, x1_cmt_wind_timer, attotime::from_hz(16))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( x1turbo, x1 )
	MCFG_CPU_MODIFY("x1_cpu")
	MCFG_CPU_PROGRAM_MAP(x1turbo_mem)
	MCFG_CPU_IO_MAP(x1turbo_io)
	MCFG_CPU_CONFIG(x1turbo_daisy)
	MCFG_MACHINE_RESET_OVERRIDE(x1_state,x1turbo)

	MCFG_Z80SIO0_ADD("sio", MAIN_CLOCK/4 , 0, 0, 0, 0)
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("x1_cpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("dma", Z80DMA, MAIN_CLOCK/4)
	MCFG_Z80DMA_OUT_BUSREQ_CB(INPUTLINE("x1_cpu", INPUT_LINE_HALT))
	MCFG_Z80DMA_OUT_INT_CB(INPUTLINE("x1_cpu", INPUT_LINE_IRQ0))
	MCFG_Z80DMA_IN_MREQ_CB(READ8(x1_state, memory_read_byte))
	MCFG_Z80DMA_OUT_MREQ_CB(WRITE8(x1_state, memory_write_byte))
	MCFG_Z80DMA_IN_IORQ_CB(READ8(x1_state, io_read_byte))
	MCFG_Z80DMA_OUT_IORQ_CB(WRITE8(x1_state, io_write_byte))

	MCFG_DEVICE_MODIFY("fdc")
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(x1_state, fdc_drq_w))

	MCFG_YM2151_ADD("ym", MAIN_CLOCK/8) //option board
	MCFG_SOUND_ROUTE(0, "lspeaker",  0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker",  0.50)
MACHINE_CONFIG_END

/*************************************
 *
 * ROM definitions
 *
 *************************************/

ROM_START( x1 )
	ROM_REGION( 0x8000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.x1", 0x0000, 0x1000, CRC(7b28d9de) SHA1(c4db9a6e99873808c8022afd1c50fef556a8b44d) )

	ROM_REGION(0x1000, "mcu", ROMREGION_ERASEFF) //MCU for the Keyboard, "sub cpu"
	ROM_LOAD( "80c48", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION(0x2000, "font", 0) //TODO: this contains 8x16 charset only, maybe it's possible that it derivates a 8x8 charset by skipping gfx lines?
	ROM_LOAD( "ank.fnt", 0x0000, 0x2000, BAD_DUMP CRC(19689fbd) SHA1(0d4e072cd6195a24a1a9b68f1d37500caa60e599) )

	ROM_REGION(0x1800, "cgrom", 0)
	ROM_LOAD("fnt0808.x1",  0x00000, 0x00800, CRC(e3995a57) SHA1(1c1a0d8c9f4c446ccd7470516b215ddca5052fb2) )
	ROM_COPY("font",    0x1000, 0x00800, 0x1000 )

	ROM_REGION(0x20000, "kanji", ROMREGION_ERASEFF)

	ROM_REGION(0x20000, "raw_kanji", ROMREGION_ERASEFF)
ROM_END

ROM_START( x1turbo )
	ROM_REGION( 0x8000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.x1t", 0x0000, 0x8000, CRC(2e8b767c) SHA1(44620f57a25f0bcac2b57ca2b0f1ebad3bf305d3) )

	ROM_REGION(0x1000, "mcu", ROMREGION_ERASEFF) //MCU for the Keyboard, "sub cpu"
	ROM_LOAD( "80c48", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION(0x2000, "font", 0) //TODO: this contains 8x16 charset only, maybe it's possible that it derivates a 8x8 charset by skipping gfx lines?
	ROM_LOAD( "ank.fnt", 0x0000, 0x2000, CRC(19689fbd) SHA1(0d4e072cd6195a24a1a9b68f1d37500caa60e599) )

	ROM_REGION(0x4800, "cgrom", 0)
	ROM_LOAD("fnt0808_turbo.x1", 0x00000, 0x00800, CRC(84a47530) SHA1(06c0995adc7a6609d4272417fe3570ca43bd0454) )
	ROM_COPY("font",             0x01000, 0x00800, 0x1000 )

	ROM_REGION(0x20000, "kanji", ROMREGION_ERASEFF)

	ROM_REGION(0x20000, "raw_kanji", ROMREGION_ERASEFF)
	ROM_LOAD("kanji4.rom", 0x00000, 0x8000, CRC(3e39de89) SHA1(d3fd24892bb1948c4697dedf5ff065ff3eaf7562) )
	ROM_LOAD("kanji2.rom", 0x08000, 0x8000, CRC(e710628a) SHA1(103bbe459dc8da27a9400aa45b385255c18fcc75) )
	ROM_LOAD("kanji3.rom", 0x10000, 0x8000, CRC(8cae13ae) SHA1(273f3329c70b332f6a49a3a95e906bbfe3e9f0a1) )
	ROM_LOAD("kanji1.rom", 0x18000, 0x8000, CRC(5874f70b) SHA1(dad7ada1b70c45f1e9db11db273ef7b385ef4f17) )
ROM_END

ROM_START( x1turbo40 )
	ROM_REGION( 0x8000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.bin", 0x0000, 0x8000, CRC(112f80a2) SHA1(646cc3fb5d2d24ff4caa5167b0892a4196e9f843) )

	ROM_REGION(0x1000, "mcu", ROMREGION_ERASEFF) //MCU for the Keyboard, "sub cpu"
	ROM_LOAD( "80c48", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION(0x2000, "font", 0) //TODO: this contains 8x16 charset only, maybe it's possible that it derivates a 8x8 charset by skipping gfx lines?
	ROM_LOAD( "ank.fnt", 0x0000, 0x2000, CRC(19689fbd) SHA1(0d4e072cd6195a24a1a9b68f1d37500caa60e599) )

	ROM_REGION(0x4800, "cgrom", 0)
	ROM_LOAD("fnt0808_turbo.x1",0x00000, 0x0800, CRC(84a47530) SHA1(06c0995adc7a6609d4272417fe3570ca43bd0454) )
	ROM_COPY("font",            0x01000, 0x0800, 0x1000 )

	ROM_REGION(0x20000, "kanji", ROMREGION_ERASEFF)

	ROM_REGION(0x20000, "raw_kanji", ROMREGION_ERASEFF)
	ROM_LOAD("kanji4.rom", 0x00000, 0x8000, CRC(3e39de89) SHA1(d3fd24892bb1948c4697dedf5ff065ff3eaf7562) )
	ROM_LOAD("kanji2.rom", 0x08000, 0x8000, CRC(e710628a) SHA1(103bbe459dc8da27a9400aa45b385255c18fcc75) )
	ROM_LOAD("kanji3.rom", 0x10000, 0x8000, CRC(8cae13ae) SHA1(273f3329c70b332f6a49a3a95e906bbfe3e9f0a1) )
	ROM_LOAD("kanji1.rom", 0x18000, 0x8000, CRC(5874f70b) SHA1(dad7ada1b70c45f1e9db11db273ef7b385ef4f17) )
ROM_END


/* Convert the ROM interleaving into something usable by the write handlers */
DRIVER_INIT_MEMBER(x1_state,x1_kanji)
{
	UINT32 i,j,k,l;
	UINT8 *kanji = memregion("kanji")->base();
	UINT8 *raw_kanji = memregion("raw_kanji")->base();

	k = 0;
	for(l=0;l<2;l++)
	{
		for(j=l*16;j<(l*16)+0x10000;j+=32)
		{
			for(i=0;i<16;i++)
			{
				kanji[j+i] = raw_kanji[k];
				kanji[j+i+0x10000] = raw_kanji[0x10000+k];
				k++;
			}
		}
	}
}


/*    YEAR  NAME       PARENT  COMPAT   MACHINE  INPUT       INIT      COMPANY    FULLNAME      FLAGS */
COMP( 1982, x1,        0,      0,       x1,      x1, driver_device,         0,        "Sharp", "X1 (CZ-800C)", 0 )
// x1twin in x1twin.c
COMP( 1984, x1turbo,   x1,     0,       x1turbo, x1turbo, x1_state,    x1_kanji, "Sharp", "X1 Turbo (CZ-850C)", MACHINE_NOT_WORKING ) //model 10
COMP( 1985, x1turbo40, x1,     0,       x1turbo, x1turbo, x1_state,    x1_kanji, "Sharp", "X1 Turbo (CZ-862C)", 0 ) //model 40
//COMP( 1986, x1turboz, x1,     0,       x1turbo, x1turbo, x1_state,    x1_kanji, "Sharp", "X1 TurboZ", MACHINE_NOT_WORKING )
