// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood, Angelo Salese, Roberto Fresca
#ifndef MAME_SUBSINO_SUBSINO_CRYPT_H
#define MAME_SUBSINO_SUBSINO_CRYPT_H

#pragma once


extern const uint8_t crsbingo_xors[8];
extern const uint8_t sharkpy_xors [8];
extern const uint8_t tisubb_xors  [8];
extern const uint8_t victor5_xors [8];
extern const uint8_t victor21_xors[8];

void crsbingo_bitswaps(uint8_t *decrypt, int i);
void sharkpy_bitswaps (uint8_t *decrypt, int i);
void tisubb_bitswaps  (uint8_t *decrypt, int i);
void victor5_bitswaps (uint8_t *decrypt, int i);
void victor21_bitswaps(uint8_t *decrypt, int i);

void subsino_decrypt(uint8_t *region, void (*bitswaps)(uint8_t *decrypt, int i), const uint8_t *xors, int size);

#endif // MAME_SUBSINO_SUBSINO_CRYPT_H
