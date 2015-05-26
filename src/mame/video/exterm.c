// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn,Zsolt Vasvari,Aaron Giles
/***************************************************************************

    Gottlieb Exterminator hardware

***************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "includes/exterm.h"


/*************************************
 *
 *  Palette setup
 *
 *************************************/

PALETTE_INIT_MEMBER(exterm_state, exterm)
{
	int i;

	/* initialize 555 RGB lookup */
	for (i = 0; i < 32768; i++)
		palette.set_pen_color(i + 0x800, pal5bit(i >> 10), pal5bit(i >> 5), pal5bit(i >> 0));
}



/*************************************
 *
 *  Master shift register
 *
 *************************************/

TMS340X0_TO_SHIFTREG_CB_MEMBER(exterm_state::to_shiftreg_master)
{
	memcpy(shiftreg, &m_master_videoram[TOWORD(address)], 256 * sizeof(UINT16));
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(exterm_state::from_shiftreg_master)
{
	memcpy(&m_master_videoram[TOWORD(address)], shiftreg, 256 * sizeof(UINT16));
}


TMS340X0_TO_SHIFTREG_CB_MEMBER(exterm_state::to_shiftreg_slave)
{
	memcpy(shiftreg, &m_slave_videoram[TOWORD(address)], 256 * 2 * sizeof(UINT8));
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(exterm_state::from_shiftreg_slave)
{
	memcpy(&m_slave_videoram[TOWORD(address)], shiftreg, 256 * 2 * sizeof(UINT8));
}



/*************************************
 *
 *  Main video refresh
 *
 *************************************/

TMS340X0_SCANLINE_IND16_CB_MEMBER(exterm_state::scanline_update)
{
	UINT16 *bgsrc = &m_master_videoram[(params->rowaddr << 8) & 0xff00];
	UINT16 *fgsrc = NULL;
	UINT16 *dest = &bitmap.pix16(scanline);
	tms34010_display_params fgparams;
	int coladdr = params->coladdr;
	int fgcoladdr = 0;
	int x;

	/* get parameters for the slave CPU */
	m_slave->get_display_params(&fgparams);

	/* compute info about the slave vram */
	if (fgparams.enabled && scanline >= fgparams.veblnk && scanline < fgparams.vsblnk && fgparams.heblnk < fgparams.hsblnk)
	{
		fgsrc = &m_slave_videoram[((fgparams.rowaddr << 8) + (fgparams.yoffset << 7)) & 0xff80];
		fgcoladdr = (fgparams.coladdr >> 1);
	}

	/* copy the non-blanked portions of this scanline */
	for (x = params->heblnk; x < params->hsblnk; x += 2)
	{
		UINT16 bgdata, fgdata = 0;

		if (fgsrc != NULL)
			fgdata = fgsrc[fgcoladdr++ & 0x7f];

		bgdata = bgsrc[coladdr++ & 0xff];
		if ((bgdata & 0xe000) == 0xe000)
			dest[x + 0] = bgdata & 0x7ff;
		else if ((fgdata & 0x00ff) != 0)
			dest[x + 0] = fgdata & 0x00ff;
		else
			dest[x + 0] = (bgdata & 0x8000) ? (bgdata & 0x7ff) : (bgdata + 0x800);

		bgdata = bgsrc[coladdr++ & 0xff];
		if ((bgdata & 0xe000) == 0xe000)
			dest[x + 1] = bgdata & 0x7ff;
		else if ((fgdata & 0xff00) != 0)
			dest[x + 1] = fgdata >> 8;
		else
			dest[x + 1] = (bgdata & 0x8000) ? (bgdata & 0x7ff) : (bgdata + 0x800);
	}
}
