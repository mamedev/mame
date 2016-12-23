// license:BSD-3-Clause
// copyright-holders:Ville Linde, Nicola Salmoria

// TODO: modernize code

uint32_t partial_carry_sum32(uint32_t add1,uint32_t add2,uint32_t carry_mask);
uint32_t partial_carry_sum24(uint32_t add1,uint32_t add2,uint32_t carry_mask);

void seibuspi_sprite_decrypt(uint8_t *src, int romsize);
void seibuspi_rise10_sprite_decrypt(uint8_t *rom, int romsize);
void seibuspi_rise11_sprite_decrypt_rfjet(uint8_t *rom, int romsize);
void seibuspi_rise11_sprite_decrypt_feversoc(uint8_t *rom, int romsize);
