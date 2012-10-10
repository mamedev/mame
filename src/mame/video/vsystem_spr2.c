// Video System Sprites (type 2)
// todo:
//  move various vsystem sprite functions here
//  unify common ones + convert to device

//  this is probably the VS 8904/8905 combo

// Spinal Breakers
// Power Spikes
// Karate Blazers
// Turbo Force
// Aero Fighters (older hardware types)
// World Beach Championships 97 (modern Power Spikes bootleg, not original hw)
// Welltris
// Formula 1 Grand Prix (1)
// Pipe Dream


#include "emu.h"
#include "vsystem_spr2.h"


const device_type VSYSTEM_SPR2 = &device_creator<vsystem_spr2_device>;

vsystem_spr2_device::vsystem_spr2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VSYSTEM_SPR2, "vsystem_spr2_device", tag, owner, clock)
{
}



void vsystem_spr2_device::device_start()
{

}

void vsystem_spr2_device::device_reset()
{

}




template<class _BitmapClass>
void vsystem_spr2_device::turbofrc_draw_sprites_common( UINT16* spriteram3,  int spriteram3_bytes, UINT16* spriteram1, int spriteram1_bytes, UINT16* spriteram2, int spriteram2_bytes, int sprite_gfx, int spritepalettebank, running_machine &machine, _BitmapClass &bitmap, const rectangle &cliprect, int chip, int chip_disabled_pri )
{
	int attr_start, base, first;
	base = chip * 0x0200;
	first = 4 * spriteram3[0x1fe + base];

	for (attr_start = base + 0x0200 - 8; attr_start >= first + base; attr_start -= 4)
	{
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color, pri;
// some other drivers still use this wrong table, they have to be upgraded
//      int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		if (!(spriteram3[attr_start + 2] & 0x0080))
			continue;

		pri = spriteram3[attr_start + 2] & 0x0010;

		if ( chip_disabled_pri & !pri)
			continue;

		if ((!chip_disabled_pri) & (pri >> 4))
			continue;

		ox = spriteram3[attr_start + 1] & 0x01ff;
		xsize = (spriteram3[attr_start + 2] & 0x0700) >> 8;
		zoomx = (spriteram3[attr_start + 1] & 0xf000) >> 12;
		oy = spriteram3[attr_start + 0] & 0x01ff;
		ysize = (spriteram3[attr_start + 2] & 0x7000) >> 12;
		zoomy = (spriteram3[attr_start + 0] & 0xf000) >> 12;
		flipx = spriteram3[attr_start + 2] & 0x0800;
		flipy = spriteram3[attr_start + 2] & 0x8000;
		color = (spriteram3[attr_start + 2] & 0x000f) + 16 * spritepalettebank;

		map_start = spriteram3[attr_start + 3];

// aerofgt has this adjustment, but doing it here would break turbo force title screen
//      ox += (xsize*zoomx+2)/4;
//      oy += (ysize*zoomy+2)/4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		for (y = 0; y <= ysize; y++)
		{
			int sx, sy;

			if (flipy)
				sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else
				sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx)
					sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else
					sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

				if (chip == 0)
					code = spriteram1[map_start % (spriteram1_bytes/2)];
				else
					code = spriteram2[map_start % (spriteram2_bytes/2)];

				pdrawgfxzoom_transpen(bitmap,cliprect,machine.gfx[sprite_gfx + chip],
							 code,
							 color,
							 flipx,flipy,
							 sx,sy,
							 zoomx << 11, zoomy << 11,
							 machine.priority_bitmap,pri ? 0 : 2,15);
				map_start++;
			}

			if (xsize == 2) map_start += 1;
			if (xsize == 4) map_start += 3;
			if (xsize == 5) map_start += 2;
			if (xsize == 6) map_start += 1;
		}
	}
}

void vsystem_spr2_device::turbofrc_draw_sprites( UINT16* spriteram3,  int spriteram3_bytes, UINT16* spriteram1, int spriteram1_bytes, UINT16* spriteram2, int spriteram2_bytes, int sprite_gfx, int spritepalettebank, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int chip, int chip_disabled_pri )
{ turbofrc_draw_sprites_common( spriteram3, spriteram3_bytes, spriteram1, spriteram1_bytes, spriteram2, spriteram2_bytes, sprite_gfx, spritepalettebank, machine, bitmap, cliprect, chip, chip_disabled_pri ); }

void vsystem_spr2_device::turbofrc_draw_sprites( UINT16* spriteram3,  int spriteram3_bytes, UINT16* spriteram1, int spriteram1_bytes, UINT16* spriteram2, int spriteram2_bytes, int sprite_gfx, int spritepalettebank, running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int chip, int chip_disabled_pri )
{ turbofrc_draw_sprites_common( spriteram3, spriteram3_bytes, spriteram1, spriteram1_bytes, spriteram2, spriteram2_bytes, sprite_gfx, spritepalettebank, machine, bitmap, cliprect, chip, chip_disabled_pri ); }


template<class _BitmapClass>
void vsystem_spr2_device::spinlbrk_draw_sprites_common( UINT16* spriteram3,  int spriteram3_bytes, UINT16* spriteram1, int spriteram1_bytes, UINT16* spriteram2, int spriteram2_bytes, int sprite_gfx, int spritepalettebank, running_machine &machine, _BitmapClass &bitmap, const rectangle &cliprect, int chip, int chip_disabled_pri )
{
	int attr_start, base, first;
	base = chip * 0x0200;
	first = 4 * spriteram3[0x1fe + base];

	for (attr_start = base + 0x0200-8; attr_start >= first + base; attr_start -= 4)
	{
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color, pri;
// some other drivers still use this wrong table, they have to be upgraded
//      int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		if (!(spriteram3[attr_start + 2] & 0x0080))
			continue;

		pri = spriteram3[attr_start + 2] & 0x0010;

		if ( chip_disabled_pri & !pri)
			continue;
		if ((!chip_disabled_pri) & (pri >> 4))
			continue;

		ox = spriteram3[attr_start + 1] & 0x01ff;
		xsize = (spriteram3[attr_start + 2] & 0x0700) >> 8;
		zoomx = (spriteram3[attr_start + 1] & 0xf000) >> 12;
		oy = spriteram3[attr_start + 0] & 0x01ff;
		ysize = (spriteram3[attr_start + 2] & 0x7000) >> 12;
		zoomy = (spriteram3[attr_start + 0] & 0xf000) >> 12;
		flipx = spriteram3[attr_start + 2] & 0x0800;
		flipy = spriteram3[attr_start + 2] & 0x8000;
		color = (spriteram3[attr_start + 2] & 0x000f) + 16 * spritepalettebank;

		map_start = spriteram3[attr_start + 3];

// aerofgt has this adjustment, but doing it here would break turbo force title screen
//      ox += (xsize*zoomx+2)/4;
//      oy += (ysize*zoomy+2)/4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		for (y = 0; y <= ysize; y++)
		{
			int sx, sy;

			if (flipy)
				sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else
				sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx)
					sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else
					sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

				if (chip == 0)
					code = spriteram1[map_start % (spriteram1_bytes/2)];
				else
					code = spriteram2[map_start % (spriteram2_bytes/2)];

				pdrawgfxzoom_transpen(bitmap,cliprect,machine.gfx[sprite_gfx + chip],
							 code,
							 color,
							 flipx,flipy,
							 sx,sy,
							 zoomx << 11, zoomy << 11,
							 machine.priority_bitmap,pri ? 2 : 0,15);
				map_start++;
			}

			if (xsize == 2) map_start += 1;
			if (xsize == 4) map_start += 3;
			if (xsize == 5) map_start += 2;
			if (xsize == 6) map_start += 1;
		}
	}
}

void vsystem_spr2_device::spinlbrk_draw_sprites( UINT16* spriteram3,  int spriteram3_bytes, UINT16* spriteram1, int spriteram1_bytes, UINT16* spriteram2, int spriteram2_bytes, int sprite_gfx, int spritepalettebank, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int chip, int chip_disabled_pri )
{ spinlbrk_draw_sprites_common( spriteram3, spriteram3_bytes, spriteram1, spriteram1_bytes, spriteram2, spriteram2_bytes, sprite_gfx, spritepalettebank, machine, bitmap, cliprect, chip, chip_disabled_pri ); }

void vsystem_spr2_device::spinlbrk_draw_sprites( UINT16* spriteram3,  int spriteram3_bytes, UINT16* spriteram1, int spriteram1_bytes, UINT16* spriteram2, int spriteram2_bytes, int sprite_gfx, int spritepalettebank, running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int chip, int chip_disabled_pri )
{ spinlbrk_draw_sprites_common( spriteram3, spriteram3_bytes, spriteram1, spriteram1_bytes, spriteram2, spriteram2_bytes, sprite_gfx, spritepalettebank, machine, bitmap, cliprect, chip, chip_disabled_pri ); }



// like above but no secondary indirection?
void vsystem_spr2_device::welltris_draw_sprites( UINT16* spriteram, int spritepalettebank, running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	static const UINT8 zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };
	int offs;
	const rectangle &visarea = machine.primary_screen->visible_area();

	/* draw the sprites */
	for (offs = 0; offs < 0x200 - 4; offs += 4) {
		int data0 = spriteram[offs + 0];
		int data1 = spriteram[offs + 1];
		int data2 = spriteram[offs + 2];
		int data3 = spriteram[offs + 3];
		int code = data3 & 0x1fff;
		int color = (data2 & 0x0f) + (0x10 * spritepalettebank);
		int y = (data0 & 0x1ff) + 1;
		int x = (data1 & 0x1ff) + 6;
		int yzoom = (data0 >> 12) & 15;
		int xzoom = (data1 >> 12) & 15;
		int zoomed = (xzoom | yzoom);
		int ytiles = ((data2 >> 12) & 7) + 1;
		int xtiles = ((data2 >>  8) & 7) + 1;
		int yflip = (data2 >> 15) & 1;
		int xflip = (data2 >> 11) & 1;
		int xt, yt;

		if (!(spriteram[offs + 2] & 0x0080)) continue;

		/* compute the zoom factor -- stolen from aerofgt.c */
		xzoom = 16 - zoomtable[xzoom] / 8;
		yzoom = 16 - zoomtable[yzoom] / 8;

		/* wrap around */
		if (x > visarea.max_x) x -= 0x200;
		if (y > visarea.max_y) y -= 0x200;

		/* normal case */
		if (!xflip && !yflip) {
			for (yt = 0; yt < ytiles; yt++) {
				for (xt = 0; xt < xtiles; xt++, code++) {
					if (!zoomed)
						drawgfx_transpen(bitmap, cliprect, machine.gfx[1], code, color, 0, 0,
								x + xt * 16, y + yt * 16, 15);
					else
						drawgfxzoom_transpen(bitmap, cliprect, machine.gfx[1], code, color, 0, 0,
								x + xt * xzoom, y + yt * yzoom,
								0x1000 * xzoom, 0x1000 * yzoom, 15);
				}
				if (xtiles == 3) code += 1;
				if (xtiles == 5) code += 3;
				if (xtiles == 6) code += 2;
				if (xtiles == 7) code += 1;
			}
		}

		/* xflipped case */
		else if (xflip && !yflip) {
			for (yt = 0; yt < ytiles; yt++) {
				for (xt = 0; xt < xtiles; xt++, code++) {
					if (!zoomed)
						drawgfx_transpen(bitmap, cliprect, machine.gfx[1], code, color, 1, 0,
								x + (xtiles - 1 - xt) * 16, y + yt * 16, 15);
					else
						drawgfxzoom_transpen(bitmap, cliprect, machine.gfx[1], code, color, 1, 0,
								x + (xtiles - 1 - xt) * xzoom, y + yt * yzoom,
								0x1000 * xzoom, 0x1000 * yzoom, 15);
				}
				if (xtiles == 3) code += 1;
				if (xtiles == 5) code += 3;
				if (xtiles == 6) code += 2;
				if (xtiles == 7) code += 1;
			}
		}

		/* yflipped case */
		else if (!xflip && yflip) {
			for (yt = 0; yt < ytiles; yt++) {
				for (xt = 0; xt < xtiles; xt++, code++) {
					if (!zoomed)
						drawgfx_transpen(bitmap, cliprect, machine.gfx[1], code, color, 0, 1,
								x + xt * 16, y + (ytiles - 1 - yt) * 16, 15);
					else
						drawgfxzoom_transpen(bitmap, cliprect, machine.gfx[1], code, color, 0, 1,
								x + xt * xzoom, y + (ytiles - 1 - yt) * yzoom,
								0x1000 * xzoom, 0x1000 * yzoom, 15);
				}
				if (xtiles == 3) code += 1;
				if (xtiles == 5) code += 3;
				if (xtiles == 6) code += 2;
				if (xtiles == 7) code += 1;
			}
		}

		/* x & yflipped case */
		else {
			for (yt = 0; yt < ytiles; yt++) {
				for (xt = 0; xt < xtiles; xt++, code++) {
					if (!zoomed)
						drawgfx_transpen(bitmap, cliprect, machine.gfx[1], code, color, 1, 1,
								x + (xtiles - 1 - xt) * 16, y + (ytiles - 1 - yt) * 16, 15);
					else
						drawgfxzoom_transpen(bitmap, cliprect, machine.gfx[1], code, color, 1, 1,
								x + (xtiles - 1 - xt) * xzoom, y + (ytiles - 1 - yt) * yzoom,
								0x1000 * xzoom, 0x1000 * yzoom, 15);
				}
				if (xtiles == 3) code += 1;
				if (xtiles == 5) code += 3;
				if (xtiles == 6) code += 2;
				if (xtiles == 7) code += 1;
			}
		}
	}
}



void vsystem_spr2_device::f1gp_draw_sprites( UINT16* spr1vram, UINT16* spr2vram, UINT16* spr1cgram, int spr1cgram_bytes, UINT16* spr2cgram, int spr2cgram_bytes, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int chip, int primask )
{
	int attr_start, first;
	UINT16 *spram = chip ? spr2vram : spr1vram;

	first = 4 * spram[0x1fe];

	for (attr_start = 0x0200 - 8; attr_start >= first; attr_start -= 4)
	{
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color/*, pri*/;
		/* table hand made by looking at the ship explosion in attract mode */
		/* it's almost a logarithmic scale but not exactly */
		static const int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		if (!(spram[attr_start + 2] & 0x0080)) continue;

		ox = spram[attr_start + 1] & 0x01ff;
		xsize = (spram[attr_start + 2] & 0x0700) >> 8;
		zoomx = (spram[attr_start + 1] & 0xf000) >> 12;
		oy = spram[attr_start + 0] & 0x01ff;
		ysize = (spram[attr_start + 2] & 0x7000) >> 12;
		zoomy = (spram[attr_start + 0] & 0xf000) >> 12;
		flipx = spram[attr_start + 2] & 0x0800;
		flipy = spram[attr_start + 2] & 0x8000;
		color = (spram[attr_start + 2] & 0x000f);// + 16 * spritepalettebank;
		//pri = spram[attr_start + 2] & 0x0010;
		map_start = spram[attr_start + 3];

		zoomx = 16 - zoomtable[zoomx] / 8;
		zoomy = 16 - zoomtable[zoomy] / 8;

		for (y = 0; y <= ysize; y++)
		{
			int sx, sy;

			if (flipy) sy = ((oy + zoomy * (ysize - y) + 16) & 0x1ff) - 16;
			else sy = ((oy + zoomy * y + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx) sx = ((ox + zoomx * (xsize - x) + 16) & 0x1ff) - 16;
				else sx = ((ox + zoomx * x + 16) & 0x1ff) - 16;

				if (chip == 0)
					code = spr1cgram[map_start % (spr1cgram_bytes / 2)];
				else
					code = spr2cgram[map_start % (spr2cgram_bytes / 2)];

				pdrawgfxzoom_transpen(bitmap,cliprect,machine.gfx[1 + chip],
						code,
						color,
						flipx,flipy,
						sx,sy,
						0x1000 * zoomx,0x1000 * zoomy,
						machine.priority_bitmap,
//                      pri ? 0 : 0x2);
						primask,15);
				map_start++;
			}

			if (xsize == 2) map_start += 1;
			if (xsize == 4) map_start += 3;
			if (xsize == 5) map_start += 2;
			if (xsize == 6) map_start += 1;
		}
	}
}

// the same but for an 8-bit system..
void vsystem_spr2_device::draw_sprites_pipedrm( UINT8* spriteram, int spriteram_bytes, int flipscreen, screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int draw_priority )
{
	static const UINT8 zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };
	const rectangle &visarea = screen.visible_area();
//	UINT8 *spriteram = spriteram;
	int offs;

	/* draw the sprites */
	for (offs = 0; offs < spriteram_bytes; offs += 8)
	{
		int data2 = spriteram[offs + 4] | (spriteram[offs + 5] << 8);
		int priority = (data2 >> 4) & 1;

		/* turns out the sprites are the same as in aerofgt.c */
		if ((data2 & 0x80) && priority == draw_priority)
		{
			int data0 = spriteram[offs + 0] | (spriteram[offs + 1] << 8);
			int data1 = spriteram[offs + 2] | (spriteram[offs + 3] << 8);
			int data3 = spriteram[offs + 6] | (spriteram[offs + 7] << 8);
			int code = data3 & 0xfff;
			int color = data2 & 0x0f;
			int y = (data0 & 0x1ff) - 6;
			int x = (data1 & 0x1ff) - 13;
			int yzoom = (data0 >> 12) & 15;
			int xzoom = (data1 >> 12) & 15;
			int zoomed = (xzoom | yzoom);
			int ytiles = ((data2 >> 12) & 7) + 1;
			int xtiles = ((data2 >> 8) & 7) + 1;
			int yflip = (data2 >> 15) & 1;
			int xflip = (data2 >> 11) & 1;
			int xt, yt;

			/* compute the zoom factor -- stolen from aerofgt.c */
			xzoom = 16 - zoomtable[xzoom] / 8;
			yzoom = 16 - zoomtable[yzoom] / 8;

			/* wrap around */
			if (x > visarea.max_x)
				x -= 0x200;
			if (y > visarea.max_y)
				y -= 0x200;

			/* flip ? */
			if (flipscreen)
			{
				y = visarea.max_y - y - 16 * ytiles - 4;
				x = visarea.max_x - x - 16 * xtiles - 24;
				xflip=!xflip;
				yflip=!yflip;
			}

			/* normal case */
			if (!xflip && !yflip)
			{
				for (yt = 0; yt < ytiles; yt++)
					for (xt = 0; xt < xtiles; xt++, code++)
						if (!zoomed)
							drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[2], code, color, 0, 0,
									x + xt * 16, y + yt * 16, 15);
						else
							drawgfxzoom_transpen(bitmap, cliprect, screen.machine().gfx[2], code, color, 0, 0,
									x + xt * xzoom, y + yt * yzoom,
									0x1000 * xzoom, 0x1000 * yzoom, 15);
			}

			/* xflipped case */
			else if (xflip && !yflip)
			{
				for (yt = 0; yt < ytiles; yt++)
					for (xt = 0; xt < xtiles; xt++, code++)
						if (!zoomed)
							drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[2], code, color, 1, 0,
									x + (xtiles - 1 - xt) * 16, y + yt * 16, 15);
						else
							drawgfxzoom_transpen(bitmap, cliprect, screen.machine().gfx[2], code, color, 1, 0,
									x + (xtiles - 1 - xt) * xzoom, y + yt * yzoom,
									0x1000 * xzoom, 0x1000 * yzoom, 15);
			}

			/* yflipped case */
			else if (!xflip && yflip)
			{
				for (yt = 0; yt < ytiles; yt++)
					for (xt = 0; xt < xtiles; xt++, code++)
						if (!zoomed)
							drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[2], code, color, 0, 1,
									x + xt * 16, y + (ytiles - 1 - yt) * 16, 15);
						else
							drawgfxzoom_transpen(bitmap, cliprect, screen.machine().gfx[2], code, color, 0, 1,
									x + xt * xzoom, y + (ytiles - 1 - yt) * yzoom,
									0x1000 * xzoom, 0x1000 * yzoom, 15);
			}

			/* x & yflipped case */
			else
			{
				for (yt = 0; yt < ytiles; yt++)
					for (xt = 0; xt < xtiles; xt++, code++)
						if (!zoomed)
							drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[2], code, color, 1, 1,
									x + (xtiles - 1 - xt) * 16, y + (ytiles - 1 - yt) * 16, 15);
						else
							drawgfxzoom_transpen(bitmap, cliprect, screen.machine().gfx[2], code, color, 1, 1,
									x + (xtiles - 1 - xt) * xzoom, y + (ytiles - 1 - yt) * yzoom,
									0x1000 * xzoom, 0x1000 * yzoom, 15);
			}
		}
	}
}
