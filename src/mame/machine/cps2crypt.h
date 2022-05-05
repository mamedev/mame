// license:BSD-3-Clause
// copyright-holders:Paul Leaman, Andreas Naive, Nicola Salmoria,Charles MacDonald
/******************************************************************************
CPS-2 Encryption

******************************************************************************/

#ifndef MAME_MACHINE_CPS2CRYPT_H
#define MAME_MACHINE_CPS2CRYPT_H

#pragma once

void cps2_decrypt(running_machine &machine, uint16_t *rom, uint16_t *dec, int length, const uint32_t *master_key, uint32_t lower_limit, uint32_t upper_limit);

#endif // MAME_MACHINE_CPS2CRYPT_H
