/*----------- defined in video/tagteam.c -----------*/

extern UINT8 *tagteam_videoram;
extern UINT8 *tagteam_colorram;

WRITE8_HANDLER( tagteam_videoram_w );
WRITE8_HANDLER( tagteam_colorram_w );
READ8_HANDLER( tagteam_mirrorvideoram_r );
WRITE8_HANDLER( tagteam_mirrorvideoram_w );
READ8_HANDLER( tagteam_mirrorcolorram_r );
WRITE8_HANDLER( tagteam_mirrorcolorram_w );
WRITE8_HANDLER( tagteam_control_w );
WRITE8_HANDLER( tagteam_flipscreen_w );

PALETTE_INIT( tagteam );
VIDEO_START( tagteam );
VIDEO_UPDATE( tagteam );
