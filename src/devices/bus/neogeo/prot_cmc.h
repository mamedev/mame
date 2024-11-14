// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#ifndef MAME_BUS_NEOGEO_PROT_CMC_H
#define MAME_BUS_NEOGEO_PROT_CMC_H

#pragma once


DECLARE_DEVICE_TYPE(NG_CMC_PROT, cmc_prot_device)

// cmc42
#define KOF99_GFX_KEY (0x00)
#define GAROU_GFX_KEY (0x06)
#define MSLUG3_GFX_KEY (0xad)
#define ZUPAPA_GFX_KEY (0xbd)
#define GANRYU_GFX_KEY (0x07)
#define S1945P_GFX_KEY (0x05)
#define PREISLE2_GFX_KEY (0x9f)
#define BANGBEAD_GFX_KEY (0xf8)
#define NITD_GFX_KEY (0xff)
#define SENGOKU3_GFX_KEY (0xfe)

// cmc50
#define KOF2000_GFX_KEY (0x00)
#define KOF2001_GFX_KEY (0x1e)
#define MSLUG4_GFX_KEY (0x31)
#define ROTD_GFX_KEY (0x3f)
#define PNYAA_GFX_KEY (0x2e)
#define KOF2002_GFX_KEY (0xec)
#define MATRIM_GFX_KEY (0x6a)
#define SAMSHO5_GFX_KEY (0x0f)
#define SAMSHO5SP_GFX_KEY (0x0d)
#define MSLUG5_GFX_KEY (0x19)
#define SVC_GFX_KEY (0x57)
#define KOF2003_GFX_KEY (0x9d)
#define JOCKEYGP_GFX_KEY (0xac)


class cmc_prot_device :  public device_t
{
public:
	// construction/destruction
	cmc_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void decrypt(uint8_t *r0, uint8_t *r1,
		uint8_t c0, uint8_t c1,
		const uint8_t *table0hi,
		const uint8_t *table0lo,
		const uint8_t *table1,
		int base,
		int invert);

	void gfx_decrypt(uint8_t* rom, uint32_t rom_size, int extra_xor);
	void kof99_neogeo_gfx_decrypt(uint8_t* rom, uint32_t rom_size, uint8_t* fixed, uint32_t fixed_size, int extra_xor);
	void kof2000_neogeo_gfx_decrypt(uint8_t* rom, uint32_t rom_size, uint8_t* fixed, uint32_t fixed_size, int extra_xor);
	void cmc42_gfx_decrypt(uint8_t* rom, uint32_t rom_size, int extra_xor);
	void cmc50_gfx_decrypt(uint8_t* rom, uint32_t rom_size, int extra_xor);

	void sfix_decrypt(uint8_t* rom, uint32_t rom_size, uint8_t* fixed, uint32_t fixed_size);

	int m1_address_scramble(int address, uint16_t key);
	void cmc50_m1_decrypt(uint8_t* romcrypt, uint32_t romcrypt_size, uint8_t* romaudio, uint32_t romaudio_size);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	const uint8_t *type0_t03;
	const uint8_t *type0_t12;
	const uint8_t *type1_t03;
	const uint8_t *type1_t12;
	const uint8_t *address_8_15_xor1;
	const uint8_t *address_8_15_xor2;
	const uint8_t *address_16_23_xor1;
	const uint8_t *address_16_23_xor2;
	const uint8_t *address_0_7_xor;
};

#endif // MAME_BUS_NEOGEO_PROT_CMC_H
