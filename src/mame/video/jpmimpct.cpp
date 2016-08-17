// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    JPM IMPACT with Video hardware

****************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "includes/jpmimpct.h"


/*************************************
 *
 *  Brooktree Bt477 RAMDAC
 *
 *************************************/


/*
 *  0 0 0    Address register (RAM write mode)
 *  0 0 1    Color palette RAMs
 *  0 1 0    Pixel read mask register
 *  0 1 1    Address register (RAM read mode)
 *  1 0 0    Address register (overlay write mode)
 *  1 1 1    Address register (overlay read mode)
 *  1 0 1    Overlay register
 *  1 1 0    Command register
 */

WRITE16_MEMBER(jpmimpct_state::jpmimpct_bt477_w)
{
	UINT8 val = data & 0xff;

	switch (offset)
	{
		case 0x0:
		{
			m_bt477.address = val;
			m_bt477.addr_cnt = 0;
			break;
		}
		case 0x1:
		{
			UINT8 *addr_cnt = &m_bt477.addr_cnt;
			rgb_t *color = &m_bt477.color;

			color[*addr_cnt] = val;

			if (++*addr_cnt == 3)
			{
				m_palette->set_pen_color(m_bt477.address, rgb_t(color[0], color[1], color[2]));
				*addr_cnt = 0;

				/* Address register increments */
				m_bt477.address++;
			}
			break;
		}
		case 0x2:
		{
			m_bt477.pixmask = val;
			break;
		}
		case 0x6:
		{
			m_bt477.command = val;
			break;
		}
		default:
		{
			popmessage("Bt477: Unhandled write access (offset:%x, data:%x)", offset, val);
		}
	}
}

READ16_MEMBER(jpmimpct_state::jpmimpct_bt477_r)
{
	popmessage("Bt477: Unhandled read access (offset:%x)", offset);
	return 0;
}


/*************************************
 *
 *  VRAM shift register callbacks
 *
 *************************************/

TMS340X0_TO_SHIFTREG_CB_MEMBER(jpmimpct_state::to_shiftreg)
{
	memcpy(shiftreg, &m_vram[TOWORD(address)], 512 * sizeof(UINT16));
}

TMS340X0_FROM_SHIFTREG_CB_MEMBER(jpmimpct_state::from_shiftreg)
{
	memcpy(&m_vram[TOWORD(address)], shiftreg, 512 * sizeof(UINT16));
}


/*************************************
 *
 *  Main video refresh
 *
 *************************************/

TMS340X0_SCANLINE_RGB32_CB_MEMBER(jpmimpct_state::scanline_update)
{
	UINT16 *vram = &m_vram[(params->rowaddr << 8) & 0x3ff00];
	UINT32 *dest = &bitmap.pix32(scanline);
	int coladdr = params->coladdr;
	int x;

	for (x = params->heblnk; x < params->hsblnk; x += 2)
	{
		UINT16 pixels = vram[coladdr++ & 0xff];
		dest[x + 0] = m_palette->pen(pixels & 0xff);
		dest[x + 1] = m_palette->pen(pixels >> 8);
	}
}


/*************************************
 *
 *  Video emulation start
 *
 *************************************/

VIDEO_START_MEMBER(jpmimpct_state,jpmimpct)
{
	memset(&m_bt477, 0, sizeof(m_bt477));

	save_item(NAME(m_bt477.address));
	save_item(NAME(m_bt477.addr_cnt));
	save_item(NAME(m_bt477.pixmask));
	save_item(NAME(m_bt477.command));
	save_item(NAME(m_bt477.color));
}
