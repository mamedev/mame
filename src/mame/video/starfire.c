/***************************************************************************

    Star Fire video system

***************************************************************************/

#include "emu.h"
#include "includes/starfire.h"

static TIMER_CALLBACK( starfire_scanline_callback );

/*************************************
 *
 *  Initialize the video system
 *
 *************************************/

VIDEO_START( starfire )
{
	starfire_state *state = machine.driver_data<starfire_state>();

	machine.primary_screen->register_screen_bitmap(state->m_starfire_screen);
	state->m_scanline_timer = machine.scheduler().timer_alloc(FUNC(starfire_scanline_callback));
	state->m_scanline_timer->adjust(machine.primary_screen->time_until_pos(STARFIRE_VBEND), STARFIRE_VBEND);

    /* register for state saving */
	state->save_item(NAME(state->m_starfire_vidctrl));
	state->save_item(NAME(state->m_starfire_vidctrl1));
	state->save_item(NAME(state->m_starfire_color));
	state->save_item(NAME(state->m_starfire_colors));
}


/*************************************
 *
 *  Color RAM read/writes
 *
 *************************************/

WRITE8_MEMBER(starfire_state::starfire_colorram_w)
{

    /* handle writes to the pseudo-color RAM */
	if ((offset & 0xe0) == 0)
	{
		int palette_index = (offset & 0x1f) | ((offset & 0x200) >> 4);

		/* set RAM regardless */
		int cl = (m_starfire_vidctrl1 & 0x80) ? m_starfire_color : (data & 0x1f);
		int cr = (data >> 5) | ((offset & 0x100) >> 5);
        cr |= (m_starfire_vidctrl1 & 0x80) ? (m_starfire_color & 0x10) : (data & 0x10);

		m_starfire_colorram[offset & ~0x100] = cl;
        m_starfire_colorram[offset |  0x100] = cr;

		m_starfire_color = cl;

		/* don't modify the palette unless the TRANS bit is set */
		if (m_starfire_vidctrl1 & 0x40)
		{
			m_starfire_colors[palette_index] = ((cl & 0x3) << 7) | ((cr & 0xf) << 3) | ((cl & 0x1c) >> 2);
		}
	}

	/* handle writes to the rest of color RAM */
	else
	{
		/* set RAM based on CDRM */
		m_starfire_colorram[offset] = (m_starfire_vidctrl1 & 0x80) ? m_starfire_color : (data & 0x1f);
		m_starfire_color = (m_starfire_vidctrl1 & 0x80) ? m_starfire_color : (data & 0x1f);
	}
}

READ8_MEMBER(starfire_state::starfire_colorram_r)
{

	/* handle writes to the pseudo-color RAM, which also happen on reads */
	if ((offset & 0xe0) == 0)
	{
		int palette_index = (offset & 0x1f) | ((offset & 0x200) >> 4);
        int cl = m_starfire_colorram[offset & ~0x100];
        int cr = m_starfire_colorram[offset |  0x100];

		/* don't modify the palette unless the TRANS bit is set */
		if (m_starfire_vidctrl1 & 0x40)
		{
			m_starfire_colors[palette_index] = ((cl & 0x3) << 7) | ((cr & 0xf) << 3) | ((cl & 0x1c) >> 2);
		}

		return cl | ((cr & 0x7) << 5);
	}

	return m_starfire_colorram[offset];
}

/*************************************
 *
 *  Video RAM read/writes
 *
 *************************************/

WRITE8_MEMBER(starfire_state::starfire_videoram_w)
{
	int sh, lr, dm, ds, mask, d0, dalu;
	int offset1 = offset & 0x1fff;
	int offset2 = (offset + 0x100) & 0x1fff;

	/* PROT */
	if (!(offset & 0xe0) && !(m_starfire_vidctrl1 & 0x20))
		return;

	/* selector 6A */
	if (offset & 0x2000)
	{
		sh = (m_starfire_vidctrl >> 1) & 0x07;
		lr = m_starfire_vidctrl & 0x01;
	}
	else
	{
		sh = (m_starfire_vidctrl >> 5) & 0x07;
		lr = (m_starfire_vidctrl >> 4) & 0x01;
	}

	/* mirror bits 5B/5C/5D/5E */
	dm = data;
	if (lr)
		dm = ((dm & 0x01) << 7) | ((dm & 0x02) << 5) | ((dm & 0x04) << 3) | ((dm & 0x08) << 1) |
		     ((dm & 0x10) >> 1) | ((dm & 0x20) >> 3) | ((dm & 0x40) >> 5) | ((dm & 0x80) >> 7);

	/* shifters 6D/6E */
	ds = (dm << 8) >> sh;
	mask = 0xff00 >> sh;

	/* ROLL */
	if ((offset & 0x1f00) == 0x1f00)
	{
		if (m_starfire_vidctrl1 & 0x10)
			mask &= 0x00ff;
		else
			mask &= 0xff00;
	}

	/* ALU 8B/8D */
	d0 = (m_starfire_videoram[offset1] << 8) | m_starfire_videoram[offset2];
	dalu = d0 & ~mask;
	d0 &= mask;
	ds &= mask;
	switch (~m_starfire_vidctrl1 & 15)
	{
		case 0:		dalu |= ds ^ mask;				break;
		case 1:		dalu |= (ds | d0) ^ mask;		break;
		case 2:		dalu |= (ds ^ mask) & d0;		break;
		case 3:		dalu |= 0;						break;
		case 4:		dalu |= (ds & d0) ^ mask;		break;
		case 5:		dalu |= d0 ^ mask;				break;
		case 6:		dalu |= ds ^ d0;				break;
		case 7:		dalu |= ds & (d0 ^ mask);		break;
		case 8:		dalu |= (ds ^ mask) | d0;		break;
		case 9:		dalu |= (ds ^ d0) ^ mask;		break;
		case 10:	dalu |= d0;						break;
		case 11:	dalu |= ds & d0;				break;
		case 12:	dalu |= mask;					break;
		case 13:	dalu |= ds | (d0 ^ mask);		break;
		case 14:	dalu |= ds | d0;				break;
		case 15:	dalu |= ds;						break;
	}

	/* final output */
	m_starfire_videoram[offset1] = dalu >> 8;
	m_starfire_videoram[offset2] = dalu;

	/* color output */
	if (!(offset & 0x2000) && !(m_starfire_vidctrl1 & 0x80))
	{
		if (mask & 0xff00)
			m_starfire_colorram[offset1] = m_starfire_color;
		if (mask & 0x00ff)
			m_starfire_colorram[offset2] = m_starfire_color;
	}
}

READ8_MEMBER(starfire_state::starfire_videoram_r)
{
	int sh, mask, d0;
	int offset1 = offset & 0x1fff;
	int offset2 = (offset + 0x100) & 0x1fff;

	/* selector 6A */
	if (offset & 0x2000)
		sh = (m_starfire_vidctrl >> 1) & 0x07;
	else
		sh = (m_starfire_vidctrl >> 5) & 0x07;

	/* shifters 6D/6E */
	mask = 0xff00 >> sh;

	/* ROLL */
	if ((offset & 0x1f00) == 0x1f00)
	{
		if (m_starfire_vidctrl1 & 0x10)
			mask &= 0x00ff;
		else
			mask &= 0xff00;
	}

	/* munge the results */
	d0 = (m_starfire_videoram[offset1] & (mask >> 8)) | (m_starfire_videoram[offset2] & mask);
	d0 = (d0 << sh) | (d0 >> (8 - sh));
	return d0 & 0xff;
}



/*************************************
 *
 *  Standard screen refresh callback
 *
 *************************************/

static void get_pens(running_machine &machine, pen_t *pens)
{
	offs_t offs;
    starfire_state *state = machine.driver_data<starfire_state>();

	for (offs = 0; offs < STARFIRE_NUM_PENS; offs++)
	{
		UINT16 color = state->m_starfire_colors[offs];

		pens[offs] = MAKE_RGB(pal3bit(color >> 6), pal3bit(color >> 3), pal3bit(color >> 0));
	}
}

static TIMER_CALLBACK( starfire_scanline_callback )
{
    starfire_state *state = machine.driver_data<starfire_state>();
    pen_t pens[STARFIRE_NUM_PENS];
	int y = param;

	get_pens(machine, pens);

	UINT8 *pix = &state->m_starfire_videoram[y];
	UINT8 *col = &state->m_starfire_colorram[y];

	for (int x = 0; x < 256; x += 8)
	{
		int data = pix[0];
		int color = col[0];

		state->m_starfire_screen.pix32(y, x + 0) = pens[color | ((data >> 2) & 0x20)];
		state->m_starfire_screen.pix32(y, x + 1) = pens[color | ((data >> 1) & 0x20)];
		state->m_starfire_screen.pix32(y, x + 2) = pens[color | ((data >> 0) & 0x20)];
		state->m_starfire_screen.pix32(y, x + 3) = pens[color | ((data << 1) & 0x20)];
		state->m_starfire_screen.pix32(y, x + 4) = pens[color | ((data << 2) & 0x20)];
		state->m_starfire_screen.pix32(y, x + 5) = pens[color | ((data << 3) & 0x20)];
		state->m_starfire_screen.pix32(y, x + 6) = pens[color | ((data << 4) & 0x20)];
		state->m_starfire_screen.pix32(y, x + 7) = pens[color | ((data << 5) & 0x20)];

		pix += 256;
		col += 256;
	}

	y++;
	if (y >= STARFIRE_VBSTART) y = STARFIRE_VBEND;
	state->m_scanline_timer->adjust(machine.primary_screen->time_until_pos(y), y);
}

SCREEN_UPDATE_RGB32( starfire )
{
	starfire_state *state = screen.machine().driver_data<starfire_state>();
    copybitmap(bitmap, state->m_starfire_screen, 0, 0, 0, 0, cliprect);

	return 0;
}
