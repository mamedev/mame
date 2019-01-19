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
	cmc_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void decrypt(u8 *r0, u8 *r1,
		u8 c0, u8 c1,
		const u8 *table0hi,
		const u8 *table0lo,
		const u8 *table1,
		int base,
		int invert);

	void gfx_decrypt(u8* rom, u32 rom_size, int extra_xor);
	void kof99_neogeo_gfx_decrypt(u8* rom, u32 rom_size, u8* fixed, u32 fixed_size, int extra_xor);
	void kof2000_neogeo_gfx_decrypt(u8* rom, u32 rom_size, u8* fixed, u32 fixed_size, int extra_xor);
	void cmc42_gfx_decrypt(u8* rom, u32 rom_size, int extra_xor);
	void cmc50_gfx_decrypt(u8* rom, u32 rom_size, int extra_xor);

	void sfix_decrypt(u8* rom, u32 rom_size, u8* fixed, u32 fixed_size);

	u16 generate_cs16(u8 *rom, int size);
	int m1_address_scramble(int address, u16 key);
	void cmc50_m1_decrypt(u8* romcrypt, u32 romcrypt_size, u8* romaudio, u32 romaudio_size);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	const u8 *type0_t03;
	const u8 *type0_t12;
	const u8 *type1_t03;
	const u8 *type1_t12;
	const u8 *address_8_15_xor1;
	const u8 *address_8_15_xor2;
	const u8 *address_16_23_xor1;
	const u8 *address_16_23_xor2;
	const u8 *address_0_7_xor;
};

#endif // MAME_BUS_NEOGEO_PROT_CMC_H
