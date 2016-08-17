// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Cinemat/Leland driver

    Leland video hardware

***************************************************************************/

#include "emu.h"
#include "includes/leland.h"


/* constants */
#define VRAM_SIZE       (0x10000)
#define QRAM_SIZE       (0x10000)

#define VIDEO_WIDTH     (320)


/* debugging */
#define LOG_COMM    0


/*************************************
 *
 *  Scanline callback
 *
 *************************************/

TIMER_CALLBACK_MEMBER(leland_state::scanline_callback)
{
	int scanline = param;

	/* update the DACs */
	if (!(m_dac_control & 0x01))
		m_dac0->write_unsigned8(m_video_ram[(m_last_scanline) * 256 + 160]);

	if (!(m_dac_control & 0x02))
		m_dac1->write_unsigned8(m_video_ram[(m_last_scanline) * 256 + 161]);

	m_last_scanline = scanline;

	scanline = (scanline+1) % 256;

	/* come back at the next appropriate scanline */
	m_scanline_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


/*************************************
 *
 *  Start video hardware
 *
 *************************************/

VIDEO_START_MEMBER(leland_state,leland)
{
	/* allocate memory */
	m_video_ram = make_unique_clear<UINT8[]>(VRAM_SIZE);

	/* scanline timer */
	m_scanline_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(leland_state::scanline_callback),this));
	m_scanline_timer->adjust(m_screen->time_until_pos(0));
}

VIDEO_START_MEMBER(leland_state,ataxx)
{
	/* first do the standard stuff */
	m_video_ram = make_unique_clear<UINT8[]>(VRAM_SIZE);

	/* allocate memory */
	m_ataxx_qram = make_unique_clear<UINT8[]>(QRAM_SIZE);
}



/*************************************
 *
 *  Scrolling and banking
 *
 *************************************/

WRITE8_MEMBER(leland_state::leland_scroll_w)
{
	int scanline = m_screen->vpos();
	if (scanline > 0)
		m_screen->update_partial(scanline - 1);

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
			fatalerror("Unexpected leland_gfx_port_w\n");
	}
}


WRITE8_MEMBER(leland_state::leland_gfx_port_w)
{
	int scanline = m_screen->vpos();
	if (scanline > 0)
		m_screen->update_partial(scanline - 1);

	m_gfxbank = data;
}



/*************************************
 *
 *  Video address setting
 *
 *************************************/

void leland_state::leland_video_addr_w(address_space &space, int offset, int data, int num)
{
	struct vram_state_data *state = m_vram_state + num;

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

int leland_state::leland_vram_port_r(address_space &space, int offset, int num)
{
	struct vram_state_data *state = m_vram_state + num;
	int addr = state->m_addr;
	int inc = (offset >> 2) & 2;
	int ret;

	switch (offset & 7)
	{
		case 3: /* read hi/lo (alternating) */
			ret = m_video_ram[addr];
			addr += inc & (addr << 1);
			addr ^= 1;
			break;

		case 5: /* read hi */
			ret = m_video_ram[addr | 1];
			addr += inc;
			break;

		case 6: /* read lo */
			ret = m_video_ram[addr & ~1];
			addr += inc;
			break;

		default:
			logerror("%s: Warning: Unknown video port %02x read (address=%04x)\n",
						space.machine().describe_context(), offset, addr);
			ret = 0;
			break;
	}
	state->m_addr = addr;

	if (LOG_COMM && addr >= 0xf000)
		logerror("%s:%s comm read %04X = %02X\n", space.machine().describe_context(), num ? "slave" : "master", addr, ret);

	return ret;
}



/*************************************
 *
 *  Common video RAM write
 *
 *************************************/

void leland_state::leland_vram_port_w(address_space &space, int offset, int data, int num)
{
	UINT8 *video_ram = m_video_ram.get();
	struct vram_state_data *state = m_vram_state + num;
	int addr = state->m_addr;
	int inc = (offset >> 2) & 2;
	int trans = (offset >> 4) & num;

	/* don't fully understand why this is needed.  Isn't the
	   video RAM just one big RAM? */
	int scanline = m_screen->vpos();
	if (scanline > 0)
		m_screen->update_partial(scanline - 1);

	if (LOG_COMM && addr >= 0xf000)
		logerror("%s:%s comm write %04X = %02X\n", space.machine().describe_context(), num ? "slave" : "master", addr, data);

	/* based on the low 3 bits of the offset, update the destination */
	switch (offset & 7)
	{
		case 1: /* write hi = data, lo = latch */
			video_ram[addr & ~1] = state->m_latch[0];
			video_ram[addr |  1] = data;
			addr += inc;
			break;

		case 2: /* write hi = latch, lo = data */
			video_ram[addr & ~1] = data;
			video_ram[addr |  1] = state->m_latch[1];
			addr += inc;
			break;

		case 3: /* write hi/lo = data (alternating) */
			if (trans)
			{
				if (!(data & 0xf0)) data |= video_ram[addr] & 0xf0;
				if (!(data & 0x0f)) data |= video_ram[addr] & 0x0f;
			}
			video_ram[addr] = data;
			addr += inc & (addr << 1);
			addr ^= 1;
			break;

		case 5: /* write hi = data */
			state->m_latch[1] = data;
			if (trans)
			{
				if (!(data & 0xf0)) data |= video_ram[addr | 1] & 0xf0;
				if (!(data & 0x0f)) data |= video_ram[addr | 1] & 0x0f;
			}
			video_ram[addr | 1] = data;
			addr += inc;
			break;

		case 6: /* write lo = data */
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
						space.machine().describe_context(), offset, addr);
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
	leland_video_addr_w(space, offset, data, 0);
}


TIMER_CALLBACK_MEMBER(leland_state::leland_delayed_mvram_w)
{
	address_space &space = m_master->space(AS_PROGRAM);

	int num = (param >> 16) & 1;
	int offset = (param >> 8) & 0xff;
	int data = param & 0xff;
	leland_vram_port_w(space, offset, data, num);
}


WRITE8_MEMBER(leland_state::leland_mvram_port_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(leland_state::leland_delayed_mvram_w),this), 0x00000 | (offset << 8) | data);
}


READ8_MEMBER(leland_state::leland_mvram_port_r)
{
	return leland_vram_port_r(space, offset, 0);
}



/*************************************
 *
 *  Slave video RAM read/write
 *
 *************************************/

WRITE8_MEMBER(leland_state::leland_slave_video_addr_w)
{
	leland_video_addr_w(space, offset, data, 1);
}


WRITE8_MEMBER(leland_state::leland_svram_port_w)
{
	leland_vram_port_w(space, offset, data, 1);
}


READ8_MEMBER(leland_state::leland_svram_port_r)
{
	return leland_vram_port_r(space, offset, 1);
}



/*************************************
 *
 *  Ataxx master video RAM read/write
 *
 *************************************/

WRITE8_MEMBER(leland_state::ataxx_mvram_port_w)
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(leland_state::leland_delayed_mvram_w),this), 0x00000 | (offset << 8) | data);
}


WRITE8_MEMBER(leland_state::ataxx_svram_port_w)
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	leland_vram_port_w(space, offset, data, 1);
}



/*************************************
 *
 *  Ataxx slave video RAM read/write
 *
 *************************************/

READ8_MEMBER(leland_state::ataxx_mvram_port_r)
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	return leland_vram_port_r(space, offset, 0);
}


READ8_MEMBER(leland_state::ataxx_svram_port_r)
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	return leland_vram_port_r(space, offset, 1);
}



/*************************************
 *
 *  ROM-based refresh routine
 *
 *************************************/

UINT32 leland_state::screen_update_leland(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const UINT8 *bg_prom = memregion("user1")->base();
	const UINT8 *bg_gfx = memregion("gfx1")->base();
	offs_t bg_gfx_bank_page_size = memregion("gfx1")->bytes() / 3;
	offs_t char_bank = (((m_gfxbank >> 4) & 0x03) * 0x2000) & (bg_gfx_bank_page_size - 1);
	offs_t prom_bank = ((m_gfxbank >> 3) & 0x01) * 0x2000;

	/* for each scanline in the visible region */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT8 fg_data = 0;

		UINT16 *dst = &bitmap.pix16(y);
		UINT8 *fg_src = &m_video_ram[y << 8];

		/* for each pixel on the scanline */
		for (int x = 0; x < VIDEO_WIDTH; x++)
		{
			/* compute the effective scrolled pixel coordinates */
			UINT16 sx = (x + m_xscroll) & 0x07ff;
			UINT16 sy = (y + m_yscroll) & 0x07ff;

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
			pen_t pen = (((bg_gfx[bg_gfx_offs + (2 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 7) |    /* d0 */
						(((bg_gfx[bg_gfx_offs + (1 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 6) |    /* d1 */
						(((bg_gfx[bg_gfx_offs + (0 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 5) |    /* d2 */
						((bg_prom[bg_prom_offs] & 0xe0) >> 2);                                                  /* d3-d5 */

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

UINT32 leland_state::screen_update_ataxx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const UINT8 *bg_gfx = memregion("gfx1")->base();
	offs_t bg_gfx_bank_page_size = memregion("gfx1")->bytes() / 6;
	offs_t bg_gfx_offs_mask = bg_gfx_bank_page_size - 1;

	/* for each scanline in the visible region */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT8 fg_data = 0;

		UINT16 *dst = &bitmap.pix16(y);
		UINT8 *fg_src = &m_video_ram[y << 8];

		/* for each pixel on the scanline */
		for (int x = 0; x < VIDEO_WIDTH; x++)
		{
			/* compute the effective scrolled pixel coordinates */
			UINT16 sx = (x + m_xscroll) & 0x07ff;
			UINT16 sy = (y + m_yscroll) & 0x07ff;

			/* get the byte address this background pixel comes from */
			offs_t qram_offs = (sx >> 3) |
								((sy << 5) & 0x3f00) |
								((sy << 6) & 0x8000);

			offs_t bg_gfx_offs = ((sy & 0x07) |
									(m_ataxx_qram[qram_offs] << 3) |
									((m_ataxx_qram[0x4000 | qram_offs] & 0x7f) << 11)) & bg_gfx_offs_mask;

			/* build the pen, background is d0-d5 */
			pen_t pen = (((bg_gfx[bg_gfx_offs + (0 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 7) |    /* d0 */
						(((bg_gfx[bg_gfx_offs + (1 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 6) |    /* d1 */
						(((bg_gfx[bg_gfx_offs + (2 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 5) |    /* d2 */
						(((bg_gfx[bg_gfx_offs + (3 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 4) |    /* d3 */
						(((bg_gfx[bg_gfx_offs + (4 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 3) |    /* d4 */
						(((bg_gfx[bg_gfx_offs + (5 * bg_gfx_bank_page_size)] << (sx & 0x07)) & 0x80) >> 2);     /* d5 */

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

	MCFG_VIDEO_START_OVERRIDE(leland_state,leland)

	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(BBGGGRRR)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DRIVER(leland_state, screen_update_leland)
	MCFG_SCREEN_PALETTE("palette")
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( ataxx_video, leland_video )
	MCFG_VIDEO_START_OVERRIDE(leland_state,ataxx)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(leland_state, screen_update_ataxx)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)
MACHINE_CONFIG_END
