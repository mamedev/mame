/***************************************************************************

    Cinemat/Leland driver

    Leland video hardware

***************************************************************************/

#include "driver.h"
#include "leland.h"


/* constants */
#define VRAM_SIZE		(0x10000)
#define QRAM_SIZE		(0x10000)

#define VIDEO_WIDTH  	(320)


/* debugging */
#define LOG_COMM	0



struct vram_state_data
{
	UINT16	addr;
	UINT8	latch[2];
};


/* video RAM */
UINT8 *ataxx_qram;
static UINT8 *leland_video_ram;

/* video RAM bitmap drawing */
static struct vram_state_data vram_state[2];

/* scroll background registers */
static UINT16 xscroll;
static UINT16 yscroll;
static UINT8 gfxbank;



/*************************************
 *
 *  Start video hardware
 *
 *************************************/

static VIDEO_START( leland )
{
	/* allocate memory */
	leland_video_ram = auto_malloc(VRAM_SIZE);

	/* reset videoram */
	memset(leland_video_ram, 0, VRAM_SIZE);
}


static VIDEO_START( ataxx )
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

WRITE8_HANDLER( leland_scroll_w )
{
	int scanline = video_screen_get_vpos(machine->primary_screen);
	if (scanline > 0)
		video_screen_update_partial(machine->primary_screen, scanline - 1);

	/* adjust the proper scroll value */
	switch (offset)
	{
		case 0:
			xscroll = (xscroll & 0xff00) | (data & 0x00ff);
			break;

		case 1:
			xscroll = (xscroll & 0x00ff) | ((data << 8) & 0xff00);
			break;

		case 2:
			yscroll = (yscroll & 0xff00) | (data & 0x00ff);
			break;

		case 3:
			yscroll = (yscroll & 0x00ff) | ((data << 8) & 0xff00);
			break;

		default:
			fatalerror("Unexpected leland_gfx_port_w");
			break;
	}
}


WRITE8_HANDLER( leland_gfx_port_w )
{
	video_screen_update_partial(machine->primary_screen, video_screen_get_vpos(machine->primary_screen));
	gfxbank = data;
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

static void leland_vram_port_w(running_machine *machine, int offset, int data, int num)
{
	struct vram_state_data *state = vram_state + num;
	int addr = state->addr;
	int inc = (offset >> 2) & 2;
	int trans = (offset >> 4) & num;

	/* don't fully understand why this is needed.  Isn't the
       video RAM just one big RAM? */
	int scanline = video_screen_get_vpos(machine->primary_screen);
	if (scanline > 0)
		video_screen_update_partial(machine->primary_screen, scanline - 1);

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
	leland_vram_port_w(machine, offset, data, num);
}


WRITE8_HANDLER( leland_mvram_port_w )
{
	timer_call_after_resynch(NULL, 0x00000 | (offset << 8) | data, leland_delayed_mvram_w);
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
	leland_vram_port_w(machine, offset, data, 1);
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
	timer_call_after_resynch(NULL, 0x00000 | (offset << 8) | data, leland_delayed_mvram_w);
}


WRITE8_HANDLER( ataxx_svram_port_w )
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	leland_vram_port_w(machine, offset, data, 1);
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
 *  Resets the DAC
 *
 *************************************/

static TIMER_CALLBACK( dac_reset )
{
	/* turn off the DACs at the start of the frame */
	leland_dac_control = 3;
}



/*************************************
 *
 *  ROM-based refresh routine
 *
 *************************************/

static VIDEO_UPDATE( leland )
{
	int y;

	const UINT8 *bg_prom = memory_region(REGION_USER1);
	const UINT8 *bg_gfx = memory_region(REGION_GFX1);
	offs_t bg_gfx_bank_page_size = memory_region_length(REGION_GFX1) / 3;
	offs_t char_bank = (((gfxbank >> 4) & 0x03) * 0x2000) & (bg_gfx_bank_page_size - 1);
	offs_t prom_bank = ((gfxbank >> 3) & 0x01) * 0x2000;

	/* for each scanline in the visible region */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		int x;
		UINT8 fg_data = 0;

		UINT16 *dst = BITMAP_ADDR16(bitmap, y, 0);
		UINT8 *fg_src = &leland_video_ram[y << 8];

		/* for each pixel on the scanline */
		for (x = 0; x < VIDEO_WIDTH; x++)
		{
			/* compute the effective scrolled pixel coordinates */
			UINT16 sx = (x + xscroll) & 0x07ff;
			UINT16 sy = (y + yscroll) & 0x07ff;

			/* get the byte address this background pixel comes from */
			offs_t bg_prom_offs = (sx >> 3) |
								  ((sy << 5) & 0x01f00) |
								  prom_bank |
								  ((sy << 6) & 0x1c000);

			offs_t bg_gfx_offs = (sy & 0x07) |
								 (bg_prom[bg_prom_offs] << 3) |
								 ((sy << 2) & 0x1800) |
								 char_bank;

			/* build the pen, background is d0-d5 */
			pen_t pen = (((bg_gfx[bg_gfx_offs + (2 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 7) |	/* d0 */
						(((bg_gfx[bg_gfx_offs + (1 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 6) |	/* d1 */
						(((bg_gfx[bg_gfx_offs + (0 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 5) |	/* d2 */
						((bg_prom[bg_prom_offs] & 0xe0) >> 2);													/* d3-d5 */

			/* foreground is d6-d9 */
			if (x & 0x01)
				pen = pen | ((fg_data & 0x0f) << 6);
			else
			{
				fg_data = *fg_src++;
				pen = pen | ((fg_data & 0xf0) << 2);
			}

			*dst++ = pen;
		}

		/* also update the DACs */
		if (!(leland_dac_control & 0x01))
			leland_dac_update(0, leland_video_ram[y * 256 + 160]);

		if (!(leland_dac_control & 0x02))
			leland_dac_update(1, leland_video_ram[y * 256 + 161]);
	}

	/* set a timer to go off at the top of the frame */
	if (cliprect->max_y == video_screen_get_visible_area(screen)->max_y)
		timer_set(video_screen_get_time_until_pos(screen, 0, 0), NULL, 0, dac_reset);

	return 0;
}



/*************************************
 *
 *  RAM-based refresh routine
 *
 *************************************/

static VIDEO_UPDATE( ataxx )
{
	int y;

	const UINT8 *bg_gfx = memory_region(REGION_GFX1);
	offs_t bg_gfx_bank_page_size = memory_region_length(REGION_GFX1) / 6;
	offs_t bg_gfx_offs_mask = bg_gfx_bank_page_size - 1;

	/* for each scanline in the visible region */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		int x;
		UINT8 fg_data = 0;

		UINT16 *dst = BITMAP_ADDR16(bitmap, y, 0);
		UINT8 *fg_src = &leland_video_ram[y << 8];

		/* for each pixel on the scanline */
		for (x = 0; x < VIDEO_WIDTH; x++)
		{
			/* compute the effective scrolled pixel coordinates */
			UINT16 sx = (x + xscroll) & 0x07ff;
			UINT16 sy = (y + yscroll) & 0x07ff;

			/* get the byte address this background pixel comes from */
			offs_t qram_offs = (sx >> 3) |
							   ((sy << 5) & 0x3f00) |
							   ((sy << 6) & 0x8000);

			offs_t bg_gfx_offs = ((sy & 0x07) |
								  (ataxx_qram[qram_offs] << 3) |
								  ((ataxx_qram[0x4000 | qram_offs] & 0x7f) << 11)) & bg_gfx_offs_mask;

			/* build the pen, background is d0-d5 */
			pen_t pen = (((bg_gfx[bg_gfx_offs + (0 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 7) |	/* d0 */
						(((bg_gfx[bg_gfx_offs + (1 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 6) |	/* d1 */
						(((bg_gfx[bg_gfx_offs + (2 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 5) |	/* d2 */
						(((bg_gfx[bg_gfx_offs + (3 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 4) |	/* d3 */
						(((bg_gfx[bg_gfx_offs + (4 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 3) |	/* d4 */
						(((bg_gfx[bg_gfx_offs + (5 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 2);		/* d5 */

			/* foreground is d6-d9 */
			if (x & 0x01)
				pen = pen | ((fg_data & 0x0f) << 6);
			else
			{
				fg_data = *fg_src++;
				pen = pen | ((fg_data & 0xf0) << 2);
			}

			*dst++ = pen;
		}

		/* also update the DACs */
		if (!(leland_dac_control & 0x01))
			leland_dac_update(0, leland_video_ram[y * 256 + 160]);

		if (!(leland_dac_control & 0x02))
			leland_dac_update(1, leland_video_ram[y * 256 + 161]);
	}

	/* set a timer to go off at the top of the frame */
	if (cliprect->max_y == video_screen_get_visible_area(screen)->max_y)
		timer_set(video_screen_get_time_until_pos(screen, 0, 0), NULL, 0, dac_reset);

	return 0;
}



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

MACHINE_DRIVER_START( leland_video )

	MDRV_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MDRV_VIDEO_START(leland)
	MDRV_VIDEO_UPDATE(leland)

	MDRV_PALETTE_LENGTH(1024)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)
	MDRV_SCREEN_REFRESH_RATE(60)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( ataxx_video )

	MDRV_IMPORT_FROM(leland_video)

	MDRV_VIDEO_START(ataxx)
	MDRV_VIDEO_UPDATE(ataxx)
MACHINE_DRIVER_END
