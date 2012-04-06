/***************************************************************************

    Cinemat/Leland driver

    Leland video hardware

***************************************************************************/

#include "emu.h"
#include "includes/leland.h"


/* constants */
#define VRAM_SIZE		(0x10000)
#define QRAM_SIZE		(0x10000)

#define VIDEO_WIDTH 	(320)


/* debugging */
#define LOG_COMM	0


/*************************************
 *
 *  Scanline callback
 *
 *************************************/

static TIMER_CALLBACK( scanline_callback )
{
	leland_state *state = machine.driver_data<leland_state>();
	device_t *audio = machine.device("custom");
	int scanline = param;

	/* update the DACs */
	if (!(state->m_dac_control & 0x01))
		leland_dac_update(audio, 0, state->m_video_ram[(state->m_last_scanline) * 256 + 160]);

	if (!(state->m_dac_control & 0x02))
		leland_dac_update(audio, 1, state->m_video_ram[(state->m_last_scanline) * 256 + 161]);

	state->m_last_scanline = scanline;

	scanline = (scanline+1) % 256;

	/* come back at the next appropriate scanline */
	state->m_scanline_timer->adjust(machine.primary_screen->time_until_pos(scanline), scanline);
}


/*************************************
 *
 *  Start video hardware
 *
 *************************************/

static VIDEO_START( leland )
{
	leland_state *state = machine.driver_data<leland_state>();
	/* allocate memory */
	state->m_video_ram = auto_alloc_array_clear(machine, UINT8, VRAM_SIZE);

	/* scanline timer */
	state->m_scanline_timer = machine.scheduler().timer_alloc(FUNC(scanline_callback));
	state->m_scanline_timer->adjust(machine.primary_screen->time_until_pos(0));

}


static VIDEO_START( ataxx )
{
	leland_state *state = machine.driver_data<leland_state>();
	/* first do the standard stuff */
	VIDEO_START_CALL(leland);

	/* allocate memory */
	state->m_ataxx_qram = auto_alloc_array_clear(machine, UINT8, QRAM_SIZE);
}



/*************************************
 *
 *  Scrolling and banking
 *
 *************************************/

WRITE8_MEMBER(leland_state::leland_scroll_w)
{
	int scanline = machine().primary_screen->vpos();
	if (scanline > 0)
		machine().primary_screen->update_partial(scanline - 1);

	/* adjust the proper scroll value */
	switch (offset)
	{
		case 0:
			m_xscroll = (m_xscroll & 0xff00) | (data & 0x00ff);
			break;

		case 1:
			m_xscroll = (m_xscroll & 0x00ff) | ((data << 8) & 0xff00);
			break;

		case 2:
			m_yscroll = (m_yscroll & 0xff00) | (data & 0x00ff);
			break;

		case 3:
			m_yscroll = (m_yscroll & 0x00ff) | ((data << 8) & 0xff00);
			break;

		default:
			fatalerror("Unexpected leland_gfx_port_w");
			break;
	}
}


WRITE8_DEVICE_HANDLER( leland_gfx_port_w )
{
	leland_state *state = device->machine().driver_data<leland_state>();
	device->machine().primary_screen->update_partial(device->machine().primary_screen->vpos());
	state->m_gfxbank = data;
}



/*************************************
 *
 *  Video address setting
 *
 *************************************/

static void leland_video_addr_w(address_space *space, int offset, int data, int num)
{
	leland_state *drvstate = space->machine().driver_data<leland_state>();
	struct vram_state_data *state = drvstate->m_vram_state + num;

	if (!offset)
		state->m_addr = (state->m_addr & 0xfe00) | ((data << 1) & 0x01fe);
	else
		state->m_addr = ((data << 9) & 0xfe00) | (state->m_addr & 0x01fe);
}



/*************************************
 *
 *  Common video RAM read
 *
 *************************************/

static int leland_vram_port_r(address_space *space, int offset, int num)
{
	leland_state *drvstate = space->machine().driver_data<leland_state>();
	struct vram_state_data *state = drvstate->m_vram_state + num;
	int addr = state->m_addr;
	int inc = (offset >> 2) & 2;
	int ret;

	switch (offset & 7)
	{
		case 3:	/* read hi/lo (alternating) */
			ret = drvstate->m_video_ram[addr];
			addr += inc & (addr << 1);
			addr ^= 1;
			break;

		case 5:	/* read hi */
			ret = drvstate->m_video_ram[addr | 1];
			addr += inc;
			break;

		case 6:	/* read lo */
			ret = drvstate->m_video_ram[addr & ~1];
			addr += inc;
			break;

		default:
			logerror("%s: Warning: Unknown video port %02x read (address=%04x)\n",
						space->machine().describe_context(), offset, addr);
			ret = 0;
			break;
	}
	state->m_addr = addr;

	if (LOG_COMM && addr >= 0xf000)
		logerror("%s:%s comm read %04X = %02X\n", space->machine().describe_context(), num ? "slave" : "master", addr, ret);

	return ret;
}



/*************************************
 *
 *  Common video RAM write
 *
 *************************************/

static void leland_vram_port_w(address_space *space, int offset, int data, int num)
{
	leland_state *drvstate = space->machine().driver_data<leland_state>();
	UINT8 *video_ram = drvstate->m_video_ram;
	struct vram_state_data *state = drvstate->m_vram_state + num;
	int addr = state->m_addr;
	int inc = (offset >> 2) & 2;
	int trans = (offset >> 4) & num;

	/* don't fully understand why this is needed.  Isn't the
       video RAM just one big RAM? */
	int scanline = space->machine().primary_screen->vpos();
	if (scanline > 0)
		space->machine().primary_screen->update_partial(scanline - 1);

	if (LOG_COMM && addr >= 0xf000)
		logerror("%s:%s comm write %04X = %02X\n", space->machine().describe_context(), num ? "slave" : "master", addr, data);

	/* based on the low 3 bits of the offset, update the destination */
	switch (offset & 7)
	{
		case 1:	/* write hi = data, lo = latch */
			video_ram[addr & ~1] = state->m_latch[0];
			video_ram[addr |  1] = data;
			addr += inc;
			break;

		case 2:	/* write hi = latch, lo = data */
			video_ram[addr & ~1] = data;
			video_ram[addr |  1] = state->m_latch[1];
			addr += inc;
			break;

		case 3:	/* write hi/lo = data (alternating) */
			if (trans)
			{
				if (!(data & 0xf0)) data |= video_ram[addr] & 0xf0;
				if (!(data & 0x0f)) data |= video_ram[addr] & 0x0f;
			}
			video_ram[addr] = data;
			addr += inc & (addr << 1);
			addr ^= 1;
			break;

		case 5:	/* write hi = data */
			state->m_latch[1] = data;
			if (trans)
			{
				if (!(data & 0xf0)) data |= video_ram[addr | 1] & 0xf0;
				if (!(data & 0x0f)) data |= video_ram[addr | 1] & 0x0f;
			}
			video_ram[addr | 1] = data;
			addr += inc;
			break;

		case 6:	/* write lo = data */
			state->m_latch[0] = data;
			if (trans)
			{
				if (!(data & 0xf0)) data |= video_ram[addr & ~1] & 0xf0;
				if (!(data & 0x0f)) data |= video_ram[addr & ~1] & 0x0f;
			}
			video_ram[addr & ~1] = data;
			addr += inc;
			break;

		default:
			logerror("%s:Warning: Unknown video port write (address=%04x value=%02x)\n",
						space->machine().describe_context(), offset, addr);
			break;
	}

	/* update the address and plane */
	state->m_addr = addr;
}



/*************************************
 *
 *  Master video RAM read/write
 *
 *************************************/

WRITE8_MEMBER(leland_state::leland_master_video_addr_w)
{
	leland_video_addr_w(&space, offset, data, 0);
}


static TIMER_CALLBACK( leland_delayed_mvram_w )
{
	address_space *space = machine.device("master")->memory().space(AS_PROGRAM);

	int num = (param >> 16) & 1;
	int offset = (param >> 8) & 0xff;
	int data = param & 0xff;
	leland_vram_port_w(space, offset, data, num);
}


WRITE8_MEMBER(leland_state::leland_mvram_port_w)
{
	machine().scheduler().synchronize(FUNC(leland_delayed_mvram_w), 0x00000 | (offset << 8) | data);
}


READ8_MEMBER(leland_state::leland_mvram_port_r)
{
	return leland_vram_port_r(&space, offset, 0);
}



/*************************************
 *
 *  Slave video RAM read/write
 *
 *************************************/

WRITE8_MEMBER(leland_state::leland_slave_video_addr_w)
{
	leland_video_addr_w(&space, offset, data, 1);
}


WRITE8_MEMBER(leland_state::leland_svram_port_w)
{
	leland_vram_port_w(&space, offset, data, 1);
}


READ8_MEMBER(leland_state::leland_svram_port_r)
{
	return leland_vram_port_r(&space, offset, 1);
}



/*************************************
 *
 *  Ataxx master video RAM read/write
 *
 *************************************/

WRITE8_MEMBER(leland_state::ataxx_mvram_port_w)
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	machine().scheduler().synchronize(FUNC(leland_delayed_mvram_w), 0x00000 | (offset << 8) | data);
}


WRITE8_MEMBER(leland_state::ataxx_svram_port_w)
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	leland_vram_port_w(&space, offset, data, 1);
}



/*************************************
 *
 *  Ataxx slave video RAM read/write
 *
 *************************************/

READ8_MEMBER(leland_state::ataxx_mvram_port_r)
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	return leland_vram_port_r(&space, offset, 0);
}


READ8_MEMBER(leland_state::ataxx_svram_port_r)
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	return leland_vram_port_r(&space, offset, 1);
}



/*************************************
 *
 *  ROM-based refresh routine
 *
 *************************************/

static SCREEN_UPDATE_IND16( leland )
{
	leland_state *state = screen.machine().driver_data<leland_state>();
	int y;

	const UINT8 *bg_prom = screen.machine().region("user1")->base();
	const UINT8 *bg_gfx = screen.machine().region("gfx1")->base();
	offs_t bg_gfx_bank_page_size = screen.machine().region("gfx1")->bytes() / 3;
	offs_t char_bank = (((state->m_gfxbank >> 4) & 0x03) * 0x2000) & (bg_gfx_bank_page_size - 1);
	offs_t prom_bank = ((state->m_gfxbank >> 3) & 0x01) * 0x2000;

	/* for each scanline in the visible region */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int x;
		UINT8 fg_data = 0;

		UINT16 *dst = &bitmap.pix16(y);
		UINT8 *fg_src = &state->m_video_ram[y << 8];

		/* for each pixel on the scanline */
		for (x = 0; x < VIDEO_WIDTH; x++)
		{
			/* compute the effective scrolled pixel coordinates */
			UINT16 sx = (x + state->m_xscroll) & 0x07ff;
			UINT16 sy = (y + state->m_yscroll) & 0x07ff;

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

	}

	return 0;
}



/*************************************
 *
 *  RAM-based refresh routine
 *
 *************************************/

static SCREEN_UPDATE_IND16( ataxx )
{
	leland_state *state = screen.machine().driver_data<leland_state>();
	int y;

	const UINT8 *bg_gfx = screen.machine().region("gfx1")->base();
	offs_t bg_gfx_bank_page_size = screen.machine().region("gfx1")->bytes() / 6;
	offs_t bg_gfx_offs_mask = bg_gfx_bank_page_size - 1;

	/* for each scanline in the visible region */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int x;
		UINT8 fg_data = 0;

		UINT16 *dst = &bitmap.pix16(y);
		UINT8 *fg_src = &state->m_video_ram[y << 8];

		/* for each pixel on the scanline */
		for (x = 0; x < VIDEO_WIDTH; x++)
		{
			/* compute the effective scrolled pixel coordinates */
			UINT16 sx = (x + state->m_xscroll) & 0x07ff;
			UINT16 sy = (y + state->m_yscroll) & 0x07ff;

			/* get the byte address this background pixel comes from */
			offs_t qram_offs = (sx >> 3) |
							   ((sy << 5) & 0x3f00) |
							   ((sy << 6) & 0x8000);

			offs_t bg_gfx_offs = ((sy & 0x07) |
								  (state->m_ataxx_qram[qram_offs] << 3) |
								  ((state->m_ataxx_qram[0x4000 | qram_offs] & 0x7f) << 11)) & bg_gfx_offs_mask;

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
	}

	return 0;
}



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( leland_video )

	MCFG_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_VIDEO_START(leland)

	MCFG_PALETTE_LENGTH(1024)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_STATIC(leland)
MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED( ataxx_video, leland_video )
	MCFG_VIDEO_START(ataxx)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_STATIC(ataxx)
MACHINE_CONFIG_END
