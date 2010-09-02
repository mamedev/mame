/*----------- defined in video/quizpani.c -----------*/

extern UINT16 *quizpani_bg_videoram, *quizpani_txt_videoram;
extern UINT16 *quizpani_scrollreg;

WRITE16_HANDLER( quizpani_bg_videoram_w );
WRITE16_HANDLER( quizpani_txt_videoram_w );
WRITE16_HANDLER( quizpani_tilesbank_w );

VIDEO_START( quizpani );
VIDEO_UPDATE( quizpani );
