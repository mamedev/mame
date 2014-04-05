/* Konami Video Helper functions */

#include "emu.h"
#include "konami_helper.h"

/*
    This recursive function doesn't use additional memory
    (it could be easily converted into an iterative one).
    It's called shuffle because it mimics the shuffling of a deck of cards.
*/
static void konami_shuffle_16(UINT16 *buf,int len)
{
	int i;
	UINT16 t;

	if (len == 2) return;

	if (len % 4) fatalerror("shuffle() - not modulo 4\n");   /* must not happen */

	len /= 2;

	for (i = 0; i < len / 2; i++)
	{
		t = buf[len / 2 + i];
		buf[len / 2 + i] = buf[len + i];
		buf[len + i] = t;
	}

	konami_shuffle_16(buf,len);
	konami_shuffle_16(buf + len,len);
}

static void konami_shuffle_8(UINT8 *buf,int len)
{
	int i;
	UINT8 t;

	if (len == 2) return;

	if (len % 4) fatalerror("shuffle() - not modulo 4\n");  /* must not happen */

	len /= 2;

	for (i = 0; i < len / 2; i++)
	{
		t = buf[len / 2 + i];
		buf[len / 2 + i] = buf[len + i];
		buf[len + i] = t;
	}

	konami_shuffle_8(buf,len);
	konami_shuffle_8(buf + len,len);
}


/* helper function to join two 16-bit ROMs and form a 32-bit data stream */
void konamid_rom_deinterleave_2(running_machine &machine, const char *mem_region)
{
	konami_shuffle_16((UINT16 *)machine.root_device().memregion(mem_region)->base(),machine.root_device().memregion(mem_region)->bytes()/2);
}

/* hacked version of rom_deinterleave_2_half for Lethal Enforcers */
void konamid_rom_deinterleave_2_half(running_machine &machine, const char *mem_region)
{
	UINT8 *rgn = machine.root_device().memregion(mem_region)->base();

	konami_shuffle_16((UINT16 *)rgn,machine.root_device().memregion(mem_region)->bytes()/4);
	konami_shuffle_16((UINT16 *)(rgn+machine.root_device().memregion(mem_region)->bytes()/2),machine.root_device().memregion(mem_region)->bytes()/4);
}

/* helper function to join four 16-bit ROMs and form a 64-bit data stream */
void konamid_rom_deinterleave_4(running_machine &machine, const char *mem_region)
{
	konamid_rom_deinterleave_2(machine, mem_region);
	konamid_rom_deinterleave_2(machine, mem_region);
}


void konami_decode_gfx(running_machine &machine, gfxdecode_device * gfxdecode, palette_device &palette, int gfx_index, UINT8 *data, UINT32 total, const gfx_layout *layout, int bpp)
{
	gfx_layout gl;

	memcpy(&gl, layout, sizeof(gl));
	gl.total = total;
	gfxdecode->set_gfx(gfx_index, global_alloc(gfx_element(&palette, gl, data, 0, palette.entries() >> bpp, 0)));
}


void konami_deinterleave_gfx(running_machine &machine, const char *gfx_memory_region, int deinterleave)
{
	switch (deinterleave)
	{
	case KONAMI_ROM_DEINTERLEAVE_NONE:
		break;
	case KONAMI_ROM_DEINTERLEAVE_2:
		konamid_rom_deinterleave_2(machine, gfx_memory_region);
		break;
	case KONAMI_ROM_DEINTERLEAVE_2_HALF:
		konamid_rom_deinterleave_2_half(machine, gfx_memory_region);
		break;
	case KONAMI_ROM_DEINTERLEAVE_4:
		konamid_rom_deinterleave_4(machine, gfx_memory_region);
		break;
	case KONAMI_ROM_SHUFFLE8:
		konami_shuffle_8(machine.root_device().memregion(gfx_memory_region)->base(), machine.root_device().memregion(gfx_memory_region)->bytes());
		break;
	}
}

/* useful function to sort three tile layers by priority order */
void konami_sortlayers3( int *layer, int *pri )
{
#define SWAP(a,b) \
	if (pri[a] < pri[b]) \
	{ \
		int t; \
		t = pri[a]; pri[a] = pri[b]; pri[b] = t; \
		t = layer[a]; layer[a] = layer[b]; layer[b] = t; \
	}

	SWAP(0,1)
	SWAP(0,2)
	SWAP(1,2)
#undef  SWAP
}

/* useful function to sort four tile layers by priority order */
void konami_sortlayers4( int *layer, int *pri )
{
#define SWAP(a,b) \
	if (pri[a] <= pri[b]) \
	{ \
		int t; \
		t = pri[a]; pri[a] = pri[b]; pri[b] = t; \
		t = layer[a]; layer[a] = layer[b]; layer[b] = t; \
	}

	SWAP(0, 1)
	SWAP(0, 2)
	SWAP(0, 3)
	SWAP(1, 2)
	SWAP(1, 3)
	SWAP(2, 3)
#undef  SWAP
}

/* useful function to sort five tile layers by priority order */
void konami_sortlayers5( int *layer, int *pri )
{
#define SWAP(a,b) \
	if (pri[a] <= pri[b]) \
	{ \
		int t; \
		t = pri[a]; pri[a] = pri[b]; pri[b] = t; \
		t = layer[a]; layer[a] = layer[b]; layer[b] = t; \
	}

	SWAP(0, 1)
	SWAP(0, 2)
	SWAP(0, 3)
	SWAP(0, 4)
	SWAP(1, 2)
	SWAP(1, 3)
	SWAP(1, 4)
	SWAP(2, 3)
	SWAP(2, 4)
	SWAP(3, 4)
#undef  SWAP
}
