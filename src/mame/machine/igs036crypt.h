// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef _IGS036CRYPT_H_
#define _IGS036CRYPT_H_

#include "emu.h"

class igs036_decryptor
{
public:
	igs036_decryptor(const UINT16* game_key);
	void decrypter_rom(memory_region* region);
	UINT16 decrypt(UINT16 cipherword, int word_address)const;
	UINT16 deobfuscate(UINT16 cipherword, int word_address)const;

private:
	const UINT16* key;

	static int (*rot_enabling[16][4])(int);
	static int (*rot_direction[4][8])(int);
	static const UINT16 triggers[16][2];

	int rotation(int address)const;
	UINT16 rol(UINT16 num, int shift)const;
	int rot_enabled(int address, const int* group)const ;
	int rot_group(int address, const int* group)const;
};

extern const UINT16  orleg2_key[0x100];
extern const UINT16  m312cn_key[0x100];
extern const UINT16 cjddzsp_key[0x100];
extern const UINT16   cjdh2_key[0x100];
extern const UINT16    kov3_key[0x100];
extern const UINT16    kov2_key[0x100];
extern const UINT16  ddpdoj_key[0x100];

#endif
