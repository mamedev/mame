// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    JPM IMPACT with Video hardware

****************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "jpmimpct.h"

/*************************************
 *
 *  VRAM shift register callbacks
 *
 *************************************/

TMS340X0_TO_SHIFTREG_CB_MEMBER(jpmimpct_video_state::to_shiftreg)
{
	memcpy(shiftreg, &m_vram[address >> 4], 512 * sizeof(uint16_t));
}

TMS340X0_FROM_SHIFTREG_CB_MEMBER(jpmimpct_video_state::from_shiftreg)
{
	memcpy(&m_vram[address >> 4], shiftreg, 512 * sizeof(uint16_t));
}


/*************************************
 *
 *  Main video refresh
 *
 *************************************/

TMS340X0_SCANLINE_RGB32_CB_MEMBER(jpmimpct_video_state::scanline_update)
{
	uint16_t const *const vram = &m_vram[(params->rowaddr << 8) & 0x3ff00];
	uint32_t *const dest = &bitmap.pix(scanline);
	int coladdr = params->coladdr;

	for (int x = params->heblnk; x < params->hsblnk; x += 2)
	{
		uint16_t const pixels = vram[coladdr++ & 0xff];
		dest[x + 0] = m_ramdac->palette_lookup(pixels & 0xff);
		dest[x + 1] = m_ramdac->palette_lookup(pixels >> 8);
	}
}
