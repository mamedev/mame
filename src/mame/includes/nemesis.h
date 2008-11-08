/*----------- defined in video/nemesis.c -----------*/

extern UINT16 *nemesis_videoram1b;
extern UINT16 *nemesis_videoram1f;
extern UINT16 *nemesis_videoram2b;
extern UINT16 *nemesis_videoram2f;
extern UINT16 *nemesis_characterram;
extern UINT16 *nemesis_xscroll1,*nemesis_xscroll2, *nemesis_yscroll;
extern size_t nemesis_characterram_size;
extern UINT16 *nemesis_yscroll1, *nemesis_yscroll2;

WRITE16_HANDLER( nemesis_videoram1b_word_w );
WRITE16_HANDLER( nemesis_videoram1f_word_w );
WRITE16_HANDLER( nemesis_videoram2b_word_w );
WRITE16_HANDLER( nemesis_videoram2f_word_w );
WRITE16_HANDLER( nemesis_characterram_word_w );
VIDEO_UPDATE( nemesis );
VIDEO_START( nemesis );
VIDEO_UPDATE( salamand );

WRITE16_HANDLER( nemesis_gfx_flipx_w );
WRITE16_HANDLER( nemesis_gfx_flipy_w );
WRITE16_HANDLER( salamander_palette_word_w );
WRITE16_HANDLER( nemesis_palette_word_w );
