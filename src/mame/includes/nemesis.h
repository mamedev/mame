/*----------- defined in drivers/nemesis.c -----------*/

extern int nemesis_irq_on;
extern int nemesis_irq2_on;
extern UINT16 hcrash_selected_ip;

/*----------- defined in video/nemesis.c -----------*/

extern UINT16 *nemesis_videoram1;
extern UINT16 *nemesis_videoram2;
extern UINT16 *nemesis_colorram1;
extern UINT16 *nemesis_colorram2;
extern UINT16 *nemesis_characterram;
extern size_t nemesis_characterram_size;
extern UINT16 *nemesis_xscroll1, *nemesis_xscroll2;
extern UINT16 *nemesis_yscroll1, *nemesis_yscroll2;

WRITE16_HANDLER( nemesis_videoram1_word_w );
WRITE16_HANDLER( nemesis_videoram2_word_w );
WRITE16_HANDLER( nemesis_colorram1_word_w );
WRITE16_HANDLER( nemesis_colorram2_word_w );
WRITE16_HANDLER( nemesis_characterram_word_w );
VIDEO_START( nemesis );
VIDEO_UPDATE( nemesis );

WRITE16_HANDLER( nemesis_gfx_flipx_word_w );
WRITE16_HANDLER( nemesis_gfx_flipy_word_w );
WRITE16_HANDLER( salamand_control_port_word_w );
WRITE16_HANDLER( salamander_palette_word_w );
WRITE16_HANDLER( nemesis_palette_word_w );
