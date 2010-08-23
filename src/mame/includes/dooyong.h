/*----------- defined in video/dooyong.c -----------*/

extern UINT8 *dooyong_txvideoram;
extern UINT8 *paletteram_flytiger;

WRITE8_HANDLER( dooyong_bgscroll8_w );
WRITE8_HANDLER( dooyong_fgscroll8_w );
WRITE8_HANDLER( dooyong_fg2scroll8_w );

WRITE16_HANDLER( dooyong_bgscroll16_w );
WRITE16_HANDLER( dooyong_bg2scroll16_w );
WRITE16_HANDLER( dooyong_fgscroll16_w );
WRITE16_HANDLER( dooyong_fg2scroll16_w );

WRITE8_HANDLER( dooyong_txvideoram8_w );

WRITE8_HANDLER( lastday_ctrl_w );
WRITE8_HANDLER( pollux_ctrl_w );
WRITE8_HANDLER( primella_ctrl_w );
WRITE8_HANDLER( paletteram_flytiger_w );
WRITE8_HANDLER( flytiger_ctrl_w );
WRITE16_HANDLER( rshark_ctrl_w );

VIDEO_UPDATE( lastday );
VIDEO_UPDATE( gulfstrm );
VIDEO_UPDATE( pollux );
VIDEO_UPDATE( bluehawk );
VIDEO_UPDATE( flytiger );
VIDEO_UPDATE( primella );
VIDEO_UPDATE( rshark );
VIDEO_UPDATE( popbingo );

VIDEO_START( lastday );
VIDEO_START( gulfstrm );
VIDEO_START( pollux );
VIDEO_START( bluehawk );
VIDEO_START( flytiger );
VIDEO_START( primella );
VIDEO_START( rshark );
VIDEO_START( popbingo );

VIDEO_EOF( dooyong );
VIDEO_EOF( rshark );
