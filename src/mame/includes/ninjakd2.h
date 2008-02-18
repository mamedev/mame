/*----------- defined in video/ninjakd2.c -----------*/

extern WRITE8_HANDLER( ninjakd2_bgvideoram_w );
extern WRITE8_HANDLER( ninjakd2_fgvideoram_w );
extern WRITE8_HANDLER( ninjakd2_bg_ctrl_w );
extern WRITE8_HANDLER( ninjakd2_sprite_overdraw_w );

extern READ8_HANDLER( robokid_bg0_videoram_r );
extern READ8_HANDLER( robokid_bg1_videoram_r );
extern READ8_HANDLER( robokid_bg2_videoram_r );
extern WRITE8_HANDLER( robokid_bg0_videoram_w );
extern WRITE8_HANDLER( robokid_bg1_videoram_w );
extern WRITE8_HANDLER( robokid_bg2_videoram_w );
extern WRITE8_HANDLER( robokid_bg0_ctrl_w );
extern WRITE8_HANDLER( robokid_bg1_ctrl_w );
extern WRITE8_HANDLER( robokid_bg2_ctrl_w );
extern WRITE8_HANDLER( robokid_bg0_bank_w );
extern WRITE8_HANDLER( robokid_bg1_bank_w );
extern WRITE8_HANDLER( robokid_bg2_bank_w );

extern VIDEO_START( ninjakd2 );
extern VIDEO_START( mnight );
extern VIDEO_START( robokid );
extern VIDEO_START( omegaf );
extern VIDEO_UPDATE( ninjakd2 );
extern VIDEO_UPDATE( robokid );
extern VIDEO_UPDATE( omegaf );
extern VIDEO_EOF( ninjakd2 );

extern UINT8* ninjakd2_bg_videoram;
extern UINT8* ninjakd2_fg_videoram;
