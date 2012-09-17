/***************************************************************************

  machine/stvcd.h - Sega Saturn and ST-V CD-ROM handling

  By R. Belmont

***************************************************************************/

#ifndef __STVCD_H__
#define __STVCD_H__

void stvcd_reset(running_machine& machine);
void stvcd_exit(running_machine& machine);

TIMER_DEVICE_CALLBACK( stv_sector_cb );
TIMER_DEVICE_CALLBACK( stv_sh1_sim );

DECLARE_READ32_HANDLER( stvcd_r );
DECLARE_WRITE32_HANDLER( stvcd_w );

void stvcd_set_tray_open(running_machine &machine);
void stvcd_set_tray_close(running_machine &machine);

#endif
