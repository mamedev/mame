// license:BSD-3-Clause
// copyright-holders:Ville Linde, Nicola Salmoria
#ifndef MAME_SEIBU_SEIBUSPI_M_H
#define MAME_SEIBU_SEIBUSPI_M_H

#pragma once

// TODO: modernize code

void seibuspi_sprite_decrypt(u8 *src, int romsize);
void seibuspi_rise10_sprite_decrypt(u8 *rom, int romsize);
void seibuspi_rise11_sprite_decrypt_rfjet(u8 *rom, int romsize);
void seibuspi_rise11_sprite_decrypt_feversoc(u8 *rom, int romsize);

#endif // MAME_SEIBU_SEIBUSPI_M_H
