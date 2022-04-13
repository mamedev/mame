// license:BSD-3-Clause
// copyright-holders:Ville Linde, Nicola Salmoria

// TODO: modernize code

u32 partial_carry_sum32(u32 add1,u32 add2,u32 carry_mask);
u32 partial_carry_sum24(u32 add1,u32 add2,u32 carry_mask);

void seibuspi_sprite_decrypt(u8 *src, int romsize);
void seibuspi_rise10_sprite_decrypt(u8 *rom, int romsize);
void seibuspi_rise11_sprite_decrypt_rfjet(u8 *rom, int romsize);
void seibuspi_rise11_sprite_decrypt_feversoc(u8 *rom, int romsize);
