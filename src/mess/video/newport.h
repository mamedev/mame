/*
    SGI "Newport" graphics board used in the Indy and some Indigo2s

*/

#ifndef _VIDHRDW_NEWPORT_
#define _VIDHRDW_NEWPORT_

extern VIDEO_START( newport );
extern SCREEN_UPDATE_RGB32( newport );

READ32_HANDLER( newport_rex3_r );
WRITE32_HANDLER( newport_rex3_w );

#endif
