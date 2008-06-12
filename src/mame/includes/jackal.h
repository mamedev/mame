/*----------- defined in machine/jackal.c -----------*/

MACHINE_RESET( jackal );

READ8_HANDLER( jackal_zram_r );
READ8_HANDLER( jackal_voram_r );
READ8_HANDLER( jackal_spriteram_r );

WRITE8_HANDLER( jackal_rambank_w );
WRITE8_HANDLER( jackal_zram_w );
WRITE8_HANDLER( jackal_voram_w );
WRITE8_HANDLER( jackal_spriteram_w );


/*----------- defined in video/jackal.c -----------*/

extern UINT8 *jackal_videoctrl;

void jackal_mark_tile_dirty(int offset);
PALETTE_INIT( jackal );
VIDEO_START( jackal );
VIDEO_UPDATE( jackal );
