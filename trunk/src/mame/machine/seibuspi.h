void seibuspi_text_decrypt(UINT8 *rom);
void seibuspi_bg_decrypt(UINT8 *rom, int size);

void seibuspi_rise10_text_decrypt(UINT8 *rom);
void seibuspi_rise10_bg_decrypt(UINT8 *rom, int size);
void seibuspi_rise10_sprite_decrypt(UINT8 *rom, int romsize);

void seibuspi_rise11_text_decrypt(UINT8 *rom);
void seibuspi_rise11_bg_decrypt(UINT8 *rom, int size);
void seibuspi_rise11_sprite_decrypt_rfjet(UINT8 *rom, int romsize);
void seibuspi_rise11_sprite_decrypt_feversoc(UINT8 *rom, int romsize);
