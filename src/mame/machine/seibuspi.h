// license:BSD-3-Clause
// copyright-holders:Ville Linde, Nicola Salmoria

// TODO: modernize code (spisprit.c too)

UINT32 partial_carry_sum32(UINT32 add1,UINT32 add2,UINT32 carry_mask);

void seibuspi_text_decrypt(UINT8 *rom);
void seibuspi_bg_decrypt(UINT8 *rom, int size);

void seibuspi_rise10_text_decrypt(UINT8 *rom);
void seibuspi_rise10_bg_decrypt(UINT8 *rom, int size);
void seibuspi_rise10_sprite_decrypt(UINT8 *rom, int romsize);

void seibuspi_rise11_text_decrypt(UINT8 *rom);
void seibuspi_rise11_bg_decrypt(UINT8 *rom, int size);
void seibuspi_rise11_sprite_decrypt_rfjet(UINT8 *rom, int romsize);
void seibuspi_rise11_sprite_decrypt_feversoc(UINT8 *rom, int romsize);
