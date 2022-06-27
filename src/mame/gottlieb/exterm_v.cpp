// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn,Zsolt Vasvari,Aaron Giles
/***************************************************************************

    Gottlieb Exterminator hardware

***************************************************************************/

#include "emu.h"
#include "exterm.h"


/*************************************
 *
 *  Palette setup
 *
 *************************************/

void exterm_state::exterm_palette(palette_device &palette) const
{
	// initialize 555 RGB lookup
	for (int i = 0; i < 32768; i++)
		palette.set_pen_color(i + 0x800, pal5bit(i >> 10), pal5bit(i >> 5), pal5bit(i >> 0));
}



/*************************************
 *
 *  Master shift register
 *
 *************************************/

TMS340X0_TO_SHIFTREG_CB_MEMBER(exterm_state::to_shiftreg_master)
{
	memcpy(shiftreg, &m_master_videoram[address >> 4], 256 * sizeof(uint16_t));
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(exterm_state::from_shiftreg_master)
{
	memcpy(&m_master_videoram[address >> 4], shiftreg, 256 * sizeof(uint16_t));
}


TMS340X0_TO_SHIFTREG_CB_MEMBER(exterm_state::to_shiftreg_slave)
{
	memcpy(shiftreg, &m_slave_videoram[address >> 4], 256 * 2 * sizeof(uint8_t));
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(exterm_state::from_shiftreg_slave)
{
	memcpy(&m_slave_videoram[address >> 4], shiftreg, 256 * 2 * sizeof(uint8_t));
}



/*************************************
 *
 *  Main video refresh
 *
 *************************************/

TMS340X0_SCANLINE_IND16_CB_MEMBER(exterm_state::scanline_update)
{
	uint16_t *const bgsrc = &m_master_videoram[(params->rowaddr << 8) & 0xff00];
	uint16_t *const dest = &bitmap.pix(scanline);
	tms340x0_device::display_params fgparams;
	int coladdr = params->coladdr;
	int fgcoladdr = 0;

	/* get parameters for the slave CPU */
	m_slave->get_display_params(&fgparams);

	/* compute info about the slave vram */
	uint16_t *fgsrc = nullptr;
	if (fgparams.enabled && scanline >= fgparams.veblnk && scanline < fgparams.vsblnk && fgparams.heblnk < fgparams.hsblnk)
	{
		fgsrc = &m_slave_videoram[((fgparams.rowaddr << 8) + (fgparams.yoffset << 7)) & 0xff80];
		fgcoladdr = (fgparams.coladdr >> 1);
	}

	/* copy the non-blanked portions of this scanline */
	for (int x = params->heblnk; x < params->hsblnk; x += 2)
	{
		uint16_t bgdata, fgdata = 0;

		if (fgsrc != nullptr)
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
