// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood, Angelo Salese, Roberto Fresca
#ifndef __MACHINE_SUBSINO__
#define __MACHINE_SUBSINO__

#include "emu.h"

extern const UINT8 crsbingo_xors[8];
extern const UINT8 sharkpy_xors [8];
extern const UINT8 victor5_xors [8];
extern const UINT8 victor21_xors[8];

void crsbingo_bitswaps(UINT8 *decrypt, int i);
void sharkpy_bitswaps (UINT8 *decrypt, int i);
void victor5_bitswaps (UINT8 *decrypt, int i);
void victor21_bitswaps(UINT8 *decrypt, int i);

void subsino_decrypt(running_machine& machine, void (*bitswaps)(UINT8 *decrypt, int i), const UINT8 *xors, int size);

#endif
