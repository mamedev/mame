#include "emu.h"
#include "includes/eolith.h"

int eolith_buffer = 0;
static UINT32 *vram;



WRITE32_HANDLER( eolith_vram_w )
{
	UINT32 *dest = &vram[offset+(0x40000/4)*eolith_buffer];

	if (mem_mask == 0xffffffff)
	{
		if (~data & 0x80000000)
			*dest = (*dest & 0x0000ffff) | (data & 0xffff0000);

		if (~data & 0x00008000)
			*dest = (*dest & 0xffff0000) | (data & 0x0000ffff);
	}
	else if (((mem_mask == 0xffff0000) && (~data & 0x80000000)) ||
	    	 ((mem_mask == 0x0000ffff) && (~data & 0x00008000)))
		COMBINE_DATA(dest);
}


READ32_HANDLER( eolith_vram_r )
{
	return vram[offset+(0x40000/4)*eolith_buffer];
}

VIDEO_START( eolith )
{
	vram = auto_alloc_array(machine, UINT32, 0x40000*2/4);
}

VIDEO_UPDATE( eolith )
{
	int y;

	for (y = 0; y < 240; y++)
	{
		int x;
		UINT32 *src = &vram[(eolith_buffer ? 0 : 0x10000) | (y * (336 / 2))];
		UINT16 *dest = BITMAP_ADDR16(bitmap, y, 0);

		for (x = 0; x < 320; x += 2)
		{
			dest[0] = (*src >> 16) & 0x7fff;
			dest[1] = (*src >>  0) & 0x7fff;

			src++;
			dest += 2;
		}
	}

	return 0;
}
