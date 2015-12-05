// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood


#pragma once

#ifndef __CMC_PROT__
#define __CMC_PROT__

extern const device_type CMC_PROT;

#define MCFG_CMC_PROT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, CMC_PROT, 0)

// cmc42
#define KOF99_GFX_KEY  (0x00)
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
	cmc_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void decrypt(UINT8 *r0, UINT8 *r1,
		UINT8 c0, UINT8 c1,
		const UINT8 *table0hi,
		const UINT8 *table0lo,
		const UINT8 *table1,
		int base,
		int invert);

	void neogeo_gfx_decrypt(UINT8* rom, UINT32 rom_size, int extra_xor);
	void neogeo_sfix_decrypt(UINT8* rom, UINT32 rom_size, UINT8* fixed, UINT32 fixed_size);
	void kof99_neogeo_gfx_decrypt(UINT8* rom, UINT32 rom_size, UINT8* fixed, UINT32 fixed_size, int extra_xor);
	void kof2000_neogeo_gfx_decrypt(UINT8* rom, UINT32 rom_size, UINT8* fixed, UINT32 fixed_size, int extra_xor);
	void cmc42_neogeo_gfx_decrypt(UINT8* rom, UINT32 rom_size, UINT8* fixed, UINT32 fixed_size, int extra_xor);
	void cmc50_neogeo_gfx_decrypt(UINT8* rom, UINT32 rom_size, UINT8* fixed, UINT32 fixed_size, int extra_xor);

	UINT16 generate_cs16(UINT8 *rom, int size);
	int m1_address_scramble(int address, UINT16 key);
	void neogeo_cmc50_m1_decrypt(UINT8* romcrypt, UINT32 romcrypt_size, UINT8* romaudio, UINT32 romaudio_size);

protected:
	virtual void device_start();
	virtual void device_reset();

	const UINT8 *type0_t03;
	const UINT8 *type0_t12;
	const UINT8 *type1_t03;
	const UINT8 *type1_t12;
	const UINT8 *address_8_15_xor1;
	const UINT8 *address_8_15_xor2;
	const UINT8 *address_16_23_xor1;
	const UINT8 *address_16_23_xor2;
	const UINT8 *address_0_7_xor;
};

#endif
