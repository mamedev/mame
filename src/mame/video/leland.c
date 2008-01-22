/***************************************************************************

    Cinemat/Leland driver

    Leland video hardware

***************************************************************************/

#include "driver.h"
#include "leland.h"
#include "sound/dac.h"


/* constants */
#define VRAM_SIZE	0x10000
#define QRAM_SIZE	0x10000

#define VIDEO_WIDTH  0x28
#define VIDEO_HEIGHT 0x1e


/* debugging */
#define LOG_COMM	0



struct vram_state_data
{
	UINT16	addr;
	UINT8	latch[2];
};


/* video RAM */
UINT8 *ataxx_qram;
UINT8 leland_last_scanline_int;
static mame_bitmap *fgbitmap;
static UINT8 *leland_video_ram;

/* video RAM bitmap drawing */
static struct vram_state_data vram_state[2];
static UINT8 sync_next_write;

/* partial screen updating */
static int next_update_scanline;

/* scroll background registers */
static UINT16 xscroll;
static UINT16 yscroll;
static UINT8 gfxbank;



/*************************************
 *
 *  Start video hardware
 *
 *************************************/

VIDEO_START( leland )
{
	/* allocate memory */
    leland_video_ram = auto_malloc(VRAM_SIZE);
    fgbitmap = auto_bitmap_alloc(VIDEO_WIDTH * 8, VIDEO_HEIGHT * 8, machine->screen[0].format);

	/* reset videoram */
    memset(leland_video_ram, 0, VRAM_SIZE);
}


VIDEO_START( ataxx )
{
	/* first do the standard stuff */
	VIDEO_START_CALL(leland);

	/* allocate memory */
	ataxx_qram = auto_malloc(QRAM_SIZE);

	/* reset QRAM */
	memset(ataxx_qram, 0, QRAM_SIZE);
}



/*************************************
 *
 *  Scrolling and banking
 *
 *************************************/

WRITE8_HANDLER( leland_gfx_port_w )
{
	/* adjust the proper scroll value */
    switch (offset)
    {
    	case -1:
    		if (gfxbank != data)
    		{
				video_screen_update_partial(0, leland_last_scanline_int);
	    		gfxbank = data;
			}
    		break;

		case 0:
			if ((xscroll & 0xff) != data)
			{
				video_screen_update_partial(0, leland_last_scanline_int);
				xscroll = (xscroll & 0xff00) | (data & 0x00ff);
			}
			break;

		case 1:
			if ((xscroll >> 8) != data)
			{
				video_screen_update_partial(0, leland_last_scanline_int);
				xscroll = (xscroll & 0x00ff) | ((data << 8) & 0xff00);
			}
			break;

		case 2:
			if ((yscroll & 0xff) != data)
			{
				video_screen_update_partial(0, leland_last_scanline_int);
				yscroll = (yscroll & 0xff00) | (data & 0x00ff);
			}
			break;

		case 3:
			if ((yscroll >> 8) != data)
			{
				video_screen_update_partial(0, leland_last_scanline_int);
				yscroll = (yscroll & 0x00ff) | ((data << 8) & 0xff00);
			}
			break;
	}
}



/*************************************
 *
 *  Video address setting
 *
 *************************************/

static void leland_video_addr_w(int offset, int data, int num)
{
	struct vram_state_data *state = vram_state + num;

	if (!offset)
		state->addr = (state->addr & 0xfe00) | ((data << 1) & 0x01fe);
	else
		state->addr = ((data << 9) & 0xfe00) | (state->addr & 0x01fe);

	if (num == 0)
		sync_next_write = (state->addr >= 0xf000);
}



/*************************************
 *
 *  Flush data from VRAM into our copy
 *
 *************************************/

static void update_for_scanline(int scanline)
{
	int i, j;

	/* skip if we're behind the times */
	if (scanline <= next_update_scanline)
		return;

	/* update all scanlines */
	for (i = next_update_scanline; i < scanline; i++)
		if (i < VIDEO_HEIGHT * 8)
		{
			UINT8 scandata[VIDEO_WIDTH * 8];
			UINT8 *dst = scandata;
			UINT8 *src = &leland_video_ram[i * 256];

			for (j = 0; j < VIDEO_WIDTH * 8 / 2; j++)
			{
				UINT8 pix = *src++;
				*dst++ = pix >> 4;
				*dst++ = pix & 15;
			}
			draw_scanline8(fgbitmap, 0, i, VIDEO_WIDTH * 8, scandata, NULL, -1);
		}

	/* also update the DACs */
	if (scanline >= VIDEO_HEIGHT * 8)
		scanline = 256;
	for (i = next_update_scanline; i < scanline; i++)
	{
		if (!(leland_dac_control & 0x01))
			leland_dac_update(0, leland_video_ram[i * 256 + 160]);
		if (!(leland_dac_control & 0x02))
			leland_dac_update(1, leland_video_ram[i * 256 + 161]);
	}

	/* set the new last update */
	next_update_scanline = scanline;
}



/*************************************
 *
 *  Common video RAM read
 *
 *************************************/

static int leland_vram_port_r(int offset, int num)
{
	struct vram_state_data *state = vram_state + num;
	int addr = state->addr;
	int inc = (offset >> 2) & 2;
    int ret;

    switch (offset & 7)
    {
        case 3:	/* read hi/lo (alternating) */
        	ret = leland_video_ram[addr];
        	addr += inc & (addr << 1);
        	addr ^= 1;
            break;

        case 5:	/* read hi */
		    ret = leland_video_ram[addr | 1];
		    addr += inc;
            break;

        case 6:	/* read lo */
		    ret = leland_video_ram[addr & ~1];
		    addr += inc;
            break;

        default:
            logerror("CPU #%d %04x Warning: Unknown video port %02x read (address=%04x)\n",
                        cpu_getactivecpu(),activecpu_get_pc(), offset, addr);
            ret = 0;
            break;
    }
    state->addr = addr;

	if (LOG_COMM && addr >= 0xf000)
		logerror("%04X:%s comm read %04X = %02X\n", activecpu_get_previouspc(), num ? "slave" : "master", addr, ret);

    return ret;
}



/*************************************
 *
 *  Common video RAM write
 *
 *************************************/

static void leland_vram_port_w(int offset, int data, int num)
{
	struct vram_state_data *state = vram_state + num;
	int addr = state->addr;
	int inc = (offset >> 2) & 2;
	int trans = (offset >> 4) & num;

	/* if we're writing "behind the beam", make sure we've cached what was there */
	if (addr < 0xf000)
	{
		int cur_scanline = video_screen_get_vpos(0);
		int mod_scanline = addr / 256;

		if (cur_scanline != next_update_scanline && mod_scanline < cur_scanline)
			update_for_scanline(cur_scanline);
	}

	if (LOG_COMM && addr >= 0xf000)
		logerror("%04X:%s comm write %04X = %02X\n", activecpu_get_previouspc(), num ? "slave" : "master", addr, data);

	/* based on the low 3 bits of the offset, update the destination */
    switch (offset & 7)
    {
        case 1:	/* write hi = data, lo = latch */
        	leland_video_ram[addr & ~1] = state->latch[0];
        	leland_video_ram[addr |  1] = data;
        	addr += inc;
        	break;

        case 2:	/* write hi = latch, lo = data */
        	leland_video_ram[addr & ~1] = data;
        	leland_video_ram[addr |  1] = state->latch[1];
        	addr += inc;
        	break;

        case 3:	/* write hi/lo = data (alternating) */
        	if (trans)
        	{
        		if (!(data & 0xf0)) data |= leland_video_ram[addr] & 0xf0;
        		if (!(data & 0x0f)) data |= leland_video_ram[addr] & 0x0f;
        	}
       		leland_video_ram[addr] = data;
        	addr += inc & (addr << 1);
        	addr ^= 1;
            break;

        case 5:	/* write hi = data */
        	state->latch[1] = data;
        	if (trans)
        	{
        		if (!(data & 0xf0)) data |= leland_video_ram[addr | 1] & 0xf0;
        		if (!(data & 0x0f)) data |= leland_video_ram[addr | 1] & 0x0f;
        	}
		    leland_video_ram[addr | 1] = data;
		    addr += inc;
            break;

        case 6:	/* write lo = data */
        	state->latch[0] = data;
        	if (trans)
        	{
        		if (!(data & 0xf0)) data |= leland_video_ram[addr & ~1] & 0xf0;
        		if (!(data & 0x0f)) data |= leland_video_ram[addr & ~1] & 0x0f;
        	}
		    leland_video_ram[addr & ~1] = data;
		    addr += inc;
            break;

        default:
            logerror("CPU #%d %04x Warning: Unknown video port write (address=%04x value=%02x)\n",
                        cpu_getactivecpu(),activecpu_get_pc(), offset, addr);
            break;
    }

    /* update the address and plane */
    state->addr = addr;
}



/*************************************
 *
 *  Master video RAM read/write
 *
 *************************************/

WRITE8_HANDLER( leland_master_video_addr_w )
{
    leland_video_addr_w(offset, data, 0);
}


static TIMER_CALLBACK( leland_delayed_mvram_w )
{
	int num = (param >> 16) & 1;
	int offset = (param >> 8) & 0xff;
	int data = param & 0xff;
	leland_vram_port_w(offset, data, num);
}


WRITE8_HANDLER( leland_mvram_port_w )
{
	if (sync_next_write)
	{
		timer_call_after_resynch(NULL, 0x00000 | (offset << 8) | data, leland_delayed_mvram_w);
		sync_next_write = 0;
	}
	else
	    leland_vram_port_w(offset, data, 0);
}


READ8_HANDLER( leland_mvram_port_r )
{
    return leland_vram_port_r(offset, 0);
}



/*************************************
 *
 *  Slave video RAM read/write
 *
 *************************************/

WRITE8_HANDLER( leland_slave_video_addr_w )
{
    leland_video_addr_w(offset, data, 1);
}


WRITE8_HANDLER( leland_svram_port_w )
{
    leland_vram_port_w(offset, data, 1);
}


READ8_HANDLER( leland_svram_port_r )
{
    return leland_vram_port_r(offset, 1);
}



/*************************************
 *
 *  Ataxx master video RAM read/write
 *
 *************************************/

WRITE8_HANDLER( ataxx_mvram_port_w )
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	if (sync_next_write)
	{
		timer_call_after_resynch(NULL, 0x00000 | (offset << 8) | data, leland_delayed_mvram_w);
		sync_next_write = 0;
	}
	else
		leland_vram_port_w(offset, data, 0);
}


WRITE8_HANDLER( ataxx_svram_port_w )
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	leland_vram_port_w(offset, data, 1);
}



/*************************************
 *
 *  Ataxx slave video RAM read/write
 *
 *************************************/

READ8_HANDLER( ataxx_mvram_port_r )
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
    return leland_vram_port_r(offset, 0);
}


READ8_HANDLER( ataxx_svram_port_r )
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
    return leland_vram_port_r(offset, 1);
}



/*************************************
 *
 *  End-of-frame routine
 *
 *************************************/

static TIMER_CALLBACK( scanline_reset )
{
	/* flush the remaining scanlines */
	next_update_scanline = 0;

	/* turn off the DACs at the start of the frame */
	leland_dac_control = 3;
}


VIDEO_EOF( leland )
{
	/* update anything remaining */
	update_for_scanline(VIDEO_HEIGHT * 8);

	/* set a timer to go off at the top of the frame */
	timer_set(video_screen_get_time_until_pos(0, 0, 0), NULL, 0, scanline_reset);
}



/*************************************
 *
 *  ROM-based refresh routine
 *
 *************************************/

VIDEO_UPDATE( leland )
{
	const UINT8 *background_prom = memory_region(REGION_USER1);
	const gfx_element *gfx = machine->gfx[0];
	int char_bank = ((gfxbank >> 4) & 0x03) * 0x0400;
	int prom_bank = ((gfxbank >> 3) & 0x01) * 0x2000;
	int xcoarse = xscroll / 8;
	int ycoarse = yscroll / 8;
	int xfine = xscroll % 8;
	int yfine = yscroll % 8;
	int x, y;

	/* update anything remaining */
	update_for_scanline(cliprect->max_y);

	/* draw what's visible to the main bitmap */
	for (y = cliprect->min_y / 8; y < cliprect->max_y / 8 + 2; y++)
	{
		int ysum = ycoarse + y;
		for (x = 0; x < VIDEO_WIDTH + 1; x++)
		{
			int xsum = xcoarse + x;
			int offs = ((xsum << 0) & 0x000ff) |
			           ((ysum << 8) & 0x01f00) |
			           prom_bank |
			           ((ysum << 9) & 0x1c000);
			int code = background_prom[offs] |
			           ((ysum << 2) & 0x300) |
			           char_bank;
			int color = (code >> 5) & 7;

			/* draw to the bitmap */
			drawgfx(bitmap, gfx,
					code, 8 * color, 0, 0,
					8 * x - xfine, 8 * y - yfine,
					cliprect, TRANSPARENCY_NONE_RAW, 0);
		}
	}

	/* Merge the two bitmaps together */
	copybitmap(bitmap, fgbitmap, 0, 0, 0, 0, cliprect, TRANSPARENCY_BLEND, 6);
	return 0;
}



/*************************************
 *
 *  RAM-based refresh routine
 *
 *************************************/

VIDEO_UPDATE( ataxx )
{
	const gfx_element *gfx = machine->gfx[0];
	int xcoarse = xscroll / 8;
	int ycoarse = yscroll / 8;
	int xfine = xscroll % 8;
	int yfine = yscroll % 8;
	int x, y;

	/* update anything remaining */
	update_for_scanline(cliprect->max_y);

	/* draw what's visible to the main bitmap */
	for (y = cliprect->min_y / 8; y < cliprect->max_y / 8 + 2; y++)
	{
		int ysum = ycoarse + y;
		for (x = 0; x < VIDEO_WIDTH + 1; x++)
		{
			int xsum = xcoarse + x;
			int offs = ((ysum & 0x40) << 9) + ((ysum & 0x3f) << 8) + (xsum & 0xff);
			int code = ataxx_qram[offs] | ((ataxx_qram[offs + 0x4000] & 0x7f) << 8);

			/* draw to the bitmap */
			drawgfx(bitmap, gfx,
					code, 0, 0, 0,
					8 * x - xfine, 8 * y - yfine,
					cliprect, TRANSPARENCY_NONE_RAW, 0);
		}
	}

	/* Merge the two bitmaps together */
	copybitmap(bitmap, fgbitmap, 0, 0, 0, 0, cliprect, TRANSPARENCY_BLEND, 6);
	return 0;
}
