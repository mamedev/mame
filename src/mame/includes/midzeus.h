/*************************************************************************

    Driver for Midway Zeus games

**************************************************************************/

#define MIDZEUS_VIDEO_CLOCK		66000000


/*----------- defined in video/midzeus.c -----------*/

extern UINT32 *zeusbase;

VIDEO_START( midzeus );
VIDEO_UPDATE( midzeus );

READ32_HANDLER( zeus_r );
WRITE32_HANDLER( zeus_w );

VIDEO_START( midzeus2 );
VIDEO_UPDATE( midzeus2 );

READ32_HANDLER( zeus2_r );
WRITE32_HANDLER( zeus2_w );

