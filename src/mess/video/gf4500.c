/*

    NVIDIA GoForce 4500

    (c) 2010 Tim Schuerewegen

*/

#include "emu.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine &machine, int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt);
		vsprintf( buf, s_fmt, v);
		va_end( v);
		logerror( "%s: %s", machine.describe_context( ), buf);
	}
}

#define BIT(x,n) (((x)>>(n))&1)
#define BITS(x,m,n) (((x)>>(n))&(((UINT32)1<<((m)-(n)+1))-1))

#define GF4500_FRAMEBUF_OFFSET 0x20000

static struct {
	UINT32 *data;
	int screen_x;
	int screen_y;
	int screen_x_max;
	int screen_y_max;
	int screen_x_min;
	int screen_y_min;
	bitmap_rgb32 bitmap;
} gf4500;

static void gf4500_init( running_machine &machine)
{
	gf4500.data = auto_alloc_array( machine, UINT32, 0x140000 / 4);
	gf4500.screen_x = gf4500.screen_y = 0;
	gf4500.screen_x_max = gf4500.screen_y_max = gf4500.screen_x_min = gf4500.screen_y_min = 0;
	machine.primary_screen->register_screen_bitmap(gf4500.bitmap);
}

static void gf4500_vram_write16( UINT16 data)
{
	if ((gf4500.screen_x < gf4500.screen_x_max) && (gf4500.screen_y < gf4500.screen_y_max))
	{
		UINT16 *vram = (UINT16 *)((UINT8 *)gf4500.data + GF4500_FRAMEBUF_OFFSET + (((gf4500.screen_y_min + gf4500.screen_y) * (320 + 1)) + (gf4500.screen_x_min + gf4500.screen_x)) * 2);
		*vram = data;
		gf4500.screen_x++;
	}
}

static rgb_t gf4500_get_color_16( UINT16 data)
{
	UINT8 r, g, b;
	r = BITS( data, 15, 11) << 3;
	g = BITS( data, 10, 5) << 2;
	b = BITS( data, 4, 0) << 3;
	return MAKE_RGB( r, g, b);
}

static void gf4500_render_screen( running_machine &machine, bitmap_rgb32 &bitmap)
{
	UINT16 *vram = (UINT16 *)(gf4500.data + GF4500_FRAMEBUF_OFFSET / 4);
	int x, y;
	for (y = 0; y < 240; y++)
	{
		UINT32 *scanline = &bitmap.pix32(y);
		for (x = 0; x < 320; x++)
		{
			*scanline++ = gf4500_get_color_16( *vram++);
		}
		vram += 1;
	}
}

READ32_HANDLER( gf4500_r )
{
	UINT32 data = gf4500.data[offset];
	switch (offset)
	{
		case 0x4C / 4 :
		{
			data = 0x00145000;
		}
		break;
	}
	if ((offset < (GF4500_FRAMEBUF_OFFSET / 4)) || (offset >= ((GF4500_FRAMEBUF_OFFSET + (321 * 240 * 2)) / 4)))
	{
		verboselog( space->machine(), 9, "(GFO) %08X -> %08X\n", 0x34000000 + (offset << 2), data);
	}
	return data;
}

WRITE32_HANDLER( gf4500_w )
{
	COMBINE_DATA(&gf4500.data[offset]);
	if ((offset < (GF4500_FRAMEBUF_OFFSET / 4)) || (offset >= ((GF4500_FRAMEBUF_OFFSET + (321 * 240 * 2)) / 4)))
	{
		verboselog( space->machine(), 9, "(GFO) %08X <- %08X\n", 0x34000000 + (offset << 2), data);
	}
	switch (offset)
	{
		case 0x300 / 4 :
		{
			gf4500.screen_x = gf4500.screen_y = 0;
		}
		break;
		case 0x304 / 4 :
		{
			gf4500.screen_x_max = (data >>  0) & 0xFFFF;
			gf4500.screen_y_max = (data >> 16) & 0xFFFF;
			if (gf4500.screen_x_max & 1) gf4500.screen_x_min++;
			//if (screen_y_max & 1) screen_y_min++;
		}
		break;
		case 0x308 / 4 :
		{
			gf4500.screen_x_min = (data >>  0) & 0xFFFF;
			gf4500.screen_y_min = (data >> 16) & 0xFFFF;
			if (gf4500.screen_x_min & 1) gf4500.screen_x_min--;
			//if (screen_y_min & 1) screen_y_min--;
		}
		break;
	}
	if ((offset >= (0x200 / 4)) && (offset < (0x280 / 4)))
	{

// 'maincpu' (02996998): (GFO) 34000304 <- 00F00140
// 'maincpu' (029969A8): (GFO) 34000308 <- 00000000
// 'maincpu' (029969B4): (GFO) 34000324 <- 00000000
// 'maincpu' (029969C4): (GFO) 34000328 <- 40000282
// 'maincpu' (029969D4): (GFO) 34000300 <- 001022CC
//
// 'maincpu' (01DCC55C): (GFO) 34000024 -> 00000000
// 'maincpu' (02996A24): (GFO) 34000200 <- AE9FAE9F
//
// 'maincpu' (02996A24): (GFO) 3400027C <- AE9FAE9F
//
// 'maincpu' (01DCC55C): (GFO) 34000024 -> 00000000
// 'maincpu' (02996A24): (GFO) 34000200 <- AE9FAE9F
// ...
// 'maincpu' (02996A24): (GFO) 3400027C <- AE9FAE9F

		gf4500_vram_write16( (data >>  0) & 0xFFFF);
		gf4500_vram_write16( (data >> 16) & 0xFFFF);
		if (gf4500.screen_x >= gf4500.screen_x_max)
		{
			gf4500.screen_x = 0;
			gf4500.screen_y++;
		}
	}
}

VIDEO_START( gf4500 )
{
	gf4500_init( machine);
}

SCREEN_UPDATE_RGB32( gf4500 )
{
	gf4500_render_screen( screen.machine(), gf4500.bitmap);
	copybitmap(bitmap, gf4500.bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}
