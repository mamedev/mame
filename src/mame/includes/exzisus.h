/*----------- defined in video/exzisus.c -----------*/

extern UINT8 *exzisus_videoram0;
extern UINT8 *exzisus_videoram1;
extern UINT8 *exzisus_objectram0;
extern UINT8 *exzisus_objectram1;
extern size_t  exzisus_objectram_size0;
extern size_t  exzisus_objectram_size1;

READ8_HANDLER( exzisus_videoram_0_r );
READ8_HANDLER( exzisus_videoram_1_r );
READ8_HANDLER( exzisus_objectram_0_r );
READ8_HANDLER( exzisus_objectram_1_r );
WRITE8_HANDLER( exzisus_videoram_0_w );
WRITE8_HANDLER( exzisus_videoram_1_w );
WRITE8_HANDLER( exzisus_objectram_0_w );
WRITE8_HANDLER( exzisus_objectram_1_w );

VIDEO_UPDATE( exzisus );


