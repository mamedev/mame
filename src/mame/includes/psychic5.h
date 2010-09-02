/*----------- defined in video/psychic5.c -----------*/

WRITE8_HANDLER( psychic5_paged_ram_w );
WRITE8_HANDLER( psychic5_vram_page_select_w );
WRITE8_HANDLER( psychic5_title_screen_w );

READ8_HANDLER( psychic5_paged_ram_r );
READ8_HANDLER( psychic5_vram_page_select_r );

VIDEO_START( psychic5 );
VIDEO_RESET( psychic5 );
VIDEO_UPDATE( psychic5 );

WRITE8_HANDLER( bombsa_paged_ram_w );
WRITE8_HANDLER( bombsa_unknown_w );

VIDEO_START( bombsa );
VIDEO_RESET( bombsa );
VIDEO_UPDATE( bombsa );
