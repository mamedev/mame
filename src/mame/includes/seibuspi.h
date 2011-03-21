/*----------- defined in drivers/seibuspi.c -----------*/

extern UINT32 *spimainram;


/*----------- defined in machine/spisprit.c -----------*/

void seibuspi_sprite_decrypt(UINT8 *src, int romsize);


/*----------- defined in video/seibuspi.c -----------*/

extern UINT32 *spi_scrollram;

VIDEO_START( spi );
SCREEN_UPDATE( spi );

VIDEO_START( sys386f2 );
SCREEN_UPDATE( sys386f2 );

READ32_HANDLER( spi_layer_bank_r );
WRITE32_HANDLER( spi_layer_bank_w );
WRITE32_HANDLER( spi_layer_enable_w );

void rf2_set_layer_banks(int banks);

WRITE32_HANDLER( tilemap_dma_start_w );
WRITE32_HANDLER( palette_dma_start_w );
WRITE32_HANDLER( video_dma_length_w );
WRITE32_HANDLER( video_dma_address_w );
WRITE32_HANDLER( sprite_dma_start_w );
