/***************************************************************************

  machine/stvcd.h - Sega Saturn and ST-V CD-ROM handling

  By R. Belmont

***************************************************************************/

#ifndef __STVCD_H__
#define __STVCD_H__

void stvcd_reset(void);

READ32_HANDLER( stvcd_r );
WRITE32_HANDLER( stvcd_w );

#endif
