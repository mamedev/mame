/*----------- defined in drivers/seibuspi.c -----------*/

extern UINT32 *spimainram;


/*----------- defined in machine/spisprit.c -----------*/

void seibuspi_sprite_decrypt(UINT8 *src, int romsize);


/*----------- defined in machine/seibuspi.c -----------*/

void seibuspi_text_decrypt(UINT8 *rom);
void seibuspi_bg_decrypt(UINT8 *rom, int size);

void seibuspi_rise10_text_decrypt(UINT8 *rom);
void seibuspi_rise10_bg_decrypt(UINT8 *rom, int size);
void seibuspi_rise10_sprite_decrypt(UINT8 *rom, int romsize);

void seibuspi_rise11_text_decrypt(UINT8 *rom);
void seibuspi_rise11_bg_decrypt(UINT8 *rom, int size);
void seibuspi_rise11_sprite_decrypt_rfjet(UINT8 *rom, int romsize);
void seibuspi_rise11_sprite_decrypt_feversoc(UINT8 *rom, int romsize);


/*----------- defined in video/seibuspi.c -----------*/

extern UINT32 *spi_scrollram;

VIDEO_START( spi );
VIDEO_UPDATE( spi );

READ32_HANDLER( spi_layer_bank_r );
WRITE32_HANDLER( spi_layer_bank_w );
WRITE32_HANDLER( spi_layer_enable_w );

void rf2_set_layer_banks(int banks);

WRITE32_HANDLER( tilemap_dma_start_w );
WRITE32_HANDLER( palette_dma_start_w );
WRITE32_HANDLER( video_dma_length_w );
WRITE32_HANDLER( video_dma_address_w );
WRITE32_HANDLER( sprite_dma_start_w );
