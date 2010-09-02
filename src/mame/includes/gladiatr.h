/*----------- defined in video/gladiatr.c -----------*/

extern UINT8 *gladiatr_videoram, *gladiatr_colorram,*gladiatr_textram;

WRITE8_HANDLER( gladiatr_videoram_w );
WRITE8_HANDLER( gladiatr_colorram_w );
WRITE8_HANDLER( gladiatr_textram_w );
WRITE8_HANDLER( gladiatr_paletteram_w );
WRITE8_HANDLER( ppking_video_registers_w );
WRITE8_HANDLER( gladiatr_video_registers_w );
WRITE8_HANDLER( gladiatr_spritebuffer_w );
WRITE8_HANDLER( gladiatr_spritebank_w );
VIDEO_START( ppking );
VIDEO_UPDATE( ppking );
VIDEO_START( gladiatr );
VIDEO_UPDATE( gladiatr );
