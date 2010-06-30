/***************************************************************************

  machine/stvcd.h - Sega Saturn and ST-V CD-ROM handling

  By R. Belmont

***************************************************************************/

#ifndef __STVCD_H__
#define __STVCD_H__

void stvcd_reset(running_machine* machine);
void stvcd_exit(running_machine& machine);

TIMER_DEVICE_CALLBACK( stv_sector_cb );

READ32_HANDLER( stvcd_r );
WRITE32_HANDLER( stvcd_w );

#endif
