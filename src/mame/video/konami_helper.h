/* Konami video helper functions */

#pragma once
#ifndef __KONAMI_HELPER_H__
#define __KONAMI_HELPER_H__

#define NORMAL_PLANE_ORDER 0x0123
#define REVERSE_PLANE_ORDER 0x3210
#define GRADIUS3_PLANE_ORDER 0x1111
#define TASMAN_PLANE_ORDER 0x1616

enum
{
	KONAMI_ROM_DEINTERLEAVE_NONE = 0,
	KONAMI_ROM_DEINTERLEAVE_2
};

/* helper function to join two 16-bit ROMs and form a 32-bit data stream */
void konamid_rom_deinterleave_2(running_machine &machine, const char *mem_region);


void konami_decode_gfx(running_machine &machine, gfxdecode_device * gfxdecode, palette_device &palette, int gfx_index, UINT8 *data, UINT32 total, const gfx_layout *layout, int bpp);
void konami_deinterleave_gfx(running_machine &machine, const char *gfx_memory_region, int deinterleave);

/* helper function to sort three tile layers by priority order */
void konami_sortlayers3(int *layer, int *pri);
/* helper function to sort four tile layers by priority order */
void konami_sortlayers4(int *layer, int *pri);
/* helper function to sort five tile layers by priority order */
void konami_sortlayers5(int *layer, int *pri);

#endif
