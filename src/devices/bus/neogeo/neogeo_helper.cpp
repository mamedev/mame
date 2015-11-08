// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood

#include "emu.h"
#include "neogeo_helper.h"

static UINT32 get_region_mask(UINT8* rgn, UINT32 rgn_size)
{
	UINT32 mask;
	UINT32 len;
	UINT32 bit;

	mask = 0xffffffff;

	len = rgn_size;

	for (bit = 0x80000000; bit != 0; bit >>= 1)
	{
		if ((len * 2 - 1) & bit)
			break;

		mask >>= 1;
	}

	return mask;
}

UINT32 neogeohelper_optimize_sprite_data(std::vector<UINT8> &spritegfx, UINT8* region_sprites, UINT32 region_sprites_size)
{
	/* convert the sprite graphics data into a format that
	   allows faster blitting */
	UINT8 *src;
	UINT8 *dest;

	UINT32 mask = get_region_mask(region_sprites, region_sprites_size);

	spritegfx.resize(mask + 1);
	UINT32 spritegfx_address_mask = mask;

	src = region_sprites;
	dest = &spritegfx[0];

	for (unsigned i = 0; i < region_sprites_size; i += 0x80, src += 0x80)
	{
		for (unsigned y = 0; y < 0x10; y++)
		{
			for (unsigned x = 0; x < 8; x++)
			{
				*(dest++) = (((src[0x43 | (y << 2)] >> x) & 0x01) << 3) |
							(((src[0x41 | (y << 2)] >> x) & 0x01) << 2) |
							(((src[0x42 | (y << 2)] >> x) & 0x01) << 1) |
							(((src[0x40 | (y << 2)] >> x) & 0x01) << 0);
			}

			for (unsigned x = 0; x < 8; x++)
			{
				*(dest++) = (((src[0x03 | (y << 2)] >> x) & 0x01) << 3) |
							(((src[0x01 | (y << 2)] >> x) & 0x01) << 2) |
							(((src[0x02 | (y << 2)] >> x) & 0x01) << 1) |
							(((src[0x00 | (y << 2)] >> x) & 0x01) << 0);
			}
		}
	}

	return spritegfx_address_mask;
}
