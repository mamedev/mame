/***************************************************************************

    Gottlieb Exterminator hardware

***************************************************************************/

#include "driver.h"
#include "cpu/tms34010/tms34010.h"
#include "exterm.h"


UINT16 *exterm_master_videoram, *exterm_slave_videoram;



/*************************************
 *
 *  Palette setup
 *
 *************************************/

PALETTE_INIT( exterm )
{
	int i;

	/* initialize 555 RGB lookup */
	for (i = 0; i < 32768; i++)
		palette_set_color_rgb(machine, i + 0x800, pal5bit(i >> 10), pal5bit(i >> 5), pal5bit(i >> 0));
}



/*************************************
 *
 *  Master shift register
 *
 *************************************/

void exterm_to_shiftreg_master(UINT32 address, UINT16 *shiftreg)
{
	memcpy(shiftreg, &exterm_master_videoram[TOWORD(address)], 256 * sizeof(UINT16));
}


void exterm_from_shiftreg_master(UINT32 address, UINT16 *shiftreg)
{
	memcpy(&exterm_master_videoram[TOWORD(address)], shiftreg, 256 * sizeof(UINT16));
}


void exterm_to_shiftreg_slave(UINT32 address, UINT16 *shiftreg)
{
	memcpy(shiftreg, &exterm_slave_videoram[TOWORD(address)], 256 * 2 * sizeof(UINT8));
}


void exterm_from_shiftreg_slave(UINT32 address, UINT16 *shiftreg)
{
	memcpy(&exterm_slave_videoram[TOWORD(address)], shiftreg, 256 * 2 * sizeof(UINT8));
}



/*************************************
 *
 *  Main video refresh
 *
 *************************************/

void exterm_scanline_update(running_machine *machine, int screen, mame_bitmap *bitmap, int scanline, const tms34010_display_params *params)
{
	UINT16 *bgsrc = &exterm_master_videoram[(params->rowaddr << 8) & 0xff00];
	UINT16 *fgsrc = NULL;
	UINT16 *dest = BITMAP_ADDR16(bitmap, scanline, 0);
	tms34010_display_params fgparams;
	int coladdr = params->coladdr;
	int fgcoladdr = 0;
	int x;

	/* get parameters for the slave CPU */
	tms34010_get_display_params(1, &fgparams);

	/* compute info about the slave vram */
	if (fgparams.enabled && scanline >= fgparams.veblnk && scanline < fgparams.vsblnk && fgparams.heblnk < fgparams.hsblnk)
	{
		fgsrc = &exterm_slave_videoram[((fgparams.rowaddr << 8) + (fgparams.yoffset << 7)) & 0xff80];
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
