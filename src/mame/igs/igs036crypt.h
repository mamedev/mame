// license:BSD-3-Clause
// copyright-holders: Andreas Naive, David Haywood

#ifndef MAME_IGS_IGS036CRYPT_H
#define MAME_IGS_IGS036CRYPT_H

#pragma once


class igs036_decryptor
{
public:
	igs036_decryptor(const uint8_t *game_key);
	void decrypter_rom(uint16_t *rom, int size, int offset);
	uint16_t decrypt(uint16_t cipherword, int word_address) const;
	uint16_t deobfuscate(uint16_t cipherword, int word_address) const;

private:
	const uint8_t* key;

	static int (*rot_enabling[16][4])(int);
	static int (*rot_direction[4][8])(int);
	static const uint32_t triggers[16][2];

	int rotation(int address) const;
	uint16_t rol(uint16_t num, int shift) const;
	int rot_enabled(int address, const int *group) const;
	int rot_group(int address, const int *group) const;
};

extern const uint8_t m312cn_key[0x100];
extern const uint8_t cjddzsp_key[0x100];
extern const uint8_t cjdh2_key[0x100];
extern const uint8_t bubucar_key[0x100];
extern const uint8_t qhzb_key[0x100];
extern const uint8_t lhtb_key[0x100];
extern const uint8_t lhzb3in1_key[0x100];
extern const uint8_t jxry_key[0x100];
extern const uint8_t dahuaxy_key[0x100];
extern const uint8_t slqzsp_key[0x100];
extern const uint8_t sydh_key[0x100];
extern const uint8_t xyddz_key[0x100];
extern const uint8_t cjbq_key[0x100];
extern const uint8_t cjgdy_key[0x100];
extern const uint8_t xydn_key[0x100];
extern const uint8_t mjzhizun_key[0x100];
extern const uint8_t lhfy_key[0x100];
extern const uint8_t lhzbgqb_key[0x100];
extern const uint8_t mghammer_key[0x100];
extern const uint8_t mghammer_io_key[0x100];
extern const uint8_t zjddz_key[0x100];
extern const uint8_t hyleyuan_key[0x100];
extern const uint8_t hlddz_key[0x100];

#endif // MAME_IGS_IGS036CRYPT_H
