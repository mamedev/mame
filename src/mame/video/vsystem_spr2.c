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
	m_newtilecb =  vsystem_tile2_indirection_delegate(FUNC(vsystem_spr2_device::tile_callback_noindirect), this);
}

void vsystem_spr2_device::set_tile_indirect_cb(device_t &device,vsystem_tile2_indirection_delegate newtilecb)
{
	vsystem_spr2_device &dev = downcast<vsystem_spr2_device &>(device);
	dev.m_newtilecb = newtilecb;
}

UINT32 vsystem_spr2_device::tile_callback_noindirect(UINT32 tile)
{
	return tile;
}


void vsystem_spr2_device::device_start()
{
	// bind our handler
	m_newtilecb.bind_relative_to(*owner());
}

void vsystem_spr2_device::device_reset()
{

}


int vsystem_spr2_device::get_sprite_attributes(UINT16* ram)
{
	// sprite is disabled
	if (!(ram[2] & 0x0080))
		return 0;

	curr_sprite.oy    =  (ram[0] & 0x01ff);
	curr_sprite.zoomy =  (ram[0] & 0xf000) >> 12;

	curr_sprite.ox =     (ram[1] & 0x01ff);
	curr_sprite.zoomx =  (ram[1] & 0xf000) >> 12;

	curr_sprite.xsize =  (ram[2] & 0x0700) >> 8;
	curr_sprite.flipx =  (ram[2] & 0x0800);

	curr_sprite.ysize =  (ram[2] & 0x7000) >> 12;
	curr_sprite.flipy =  (ram[2] & 0x8000);

	curr_sprite.color =  (ram[2] & 0x000f);
	curr_sprite.pri =    (ram[2] & 0x0010);

	curr_sprite.map =    (ram[3]);

	return 1;
}

void vsystem_spr2_device::handle_xsize_map_inc(void)
{
	if (curr_sprite.xsize == 2) curr_sprite.map += 1;
	if (curr_sprite.xsize == 4) curr_sprite.map += 3;
	if (curr_sprite.xsize == 5) curr_sprite.map += 2;
	if (curr_sprite.xsize == 6) curr_sprite.map += 1;
}

template<class _BitmapClass>
void vsystem_spr2_device::turbofrc_draw_sprites_common( UINT16* spriteram3,  int spriteram3_bytes, int sprite_gfx, int spritepalettebank, running_machine &machine, _BitmapClass &bitmap, const rectangle &cliprect, int chip_disabled_pri )
{
	int attr_start, base, first;
	base = 0;//chip * 0x0200;
	first = 4 * spriteram3[0x1fe + base];

	for (attr_start = base + 0x0200 - 8; attr_start >= first + base; attr_start -= 4)
	{
		int x, y;
// some other drivers still use this wrong table, they have to be upgraded
//      int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };


		if (!get_sprite_attributes(&spriteram3[attr_start]))
			continue;


		if ( chip_disabled_pri & !curr_sprite.pri)
			continue;

		if ((!chip_disabled_pri) & (curr_sprite.pri >> 4))
			continue;



		curr_sprite.color += 16 * spritepalettebank;

// aerofgt has this adjustment, but doing it here would break turbo force title screen
//      curr_sprite.ox += (curr_sprite.xsize*curr_sprite.zoomx+2)/4;
//      curr_sprite.oy += (curr_sprite.ysize*curr_sprite.zoomy+2)/4;

		curr_sprite.zoomx = 32 - curr_sprite.zoomx;
		curr_sprite.zoomy = 32 - curr_sprite.zoomy;

		for (y = 0; y <= curr_sprite.ysize; y++)
		{
			int sx, sy;

			if (curr_sprite.flipy)
				sy = ((curr_sprite.oy + curr_sprite.zoomy * (curr_sprite.ysize - y)/2 + 16) & 0x1ff) - 16;
			else
				sy = ((curr_sprite.oy + curr_sprite.zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0; x <= curr_sprite.xsize; x++)
			{
				int curr;

				if (curr_sprite.flipx)
					sx = ((curr_sprite.ox + curr_sprite.zoomx * (curr_sprite.xsize - x) / 2 + 16) & 0x1ff) - 16;
				else
					sx = ((curr_sprite.ox + curr_sprite.zoomx * x / 2 + 16) & 0x1ff) - 16;

				curr = m_newtilecb(curr_sprite.map++);

				pdrawgfxzoom_transpen(bitmap,cliprect,machine.gfx[sprite_gfx],
							 curr,
							 curr_sprite.color,
							 curr_sprite.flipx,curr_sprite.flipy,
							 sx,sy,
							 curr_sprite.zoomx << 11, curr_sprite.zoomy << 11,
							 machine.priority_bitmap,curr_sprite.pri ? 0 : 2,15);
			}
			handle_xsize_map_inc();
		}
	}
}

void vsystem_spr2_device::turbofrc_draw_sprites( UINT16* spriteram3,  int spriteram3_bytes, int sprite_gfx, int spritepalettebank, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int chip_disabled_pri )
{ turbofrc_draw_sprites_common( spriteram3, spriteram3_bytes, sprite_gfx, spritepalettebank, machine, bitmap, cliprect, chip_disabled_pri ); }

void vsystem_spr2_device::turbofrc_draw_sprites( UINT16* spriteram3,  int spriteram3_bytes, int sprite_gfx, int spritepalettebank, running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int chip_disabled_pri )
{ turbofrc_draw_sprites_common( spriteram3, spriteram3_bytes, sprite_gfx, spritepalettebank, machine, bitmap, cliprect, chip_disabled_pri ); }


template<class _BitmapClass>
void vsystem_spr2_device::spinlbrk_draw_sprites_common( UINT16* spriteram3,  int spriteram3_bytes, int sprite_gfx, int spritepalettebank, running_machine &machine, _BitmapClass &bitmap, const rectangle &cliprect, int chip_disabled_pri )
{
	int attr_start, base, first;
	base = 0;//chip * 0x0200;
	first = 4 * spriteram3[0x1fe + base];

	for (attr_start = base + 0x0200-8; attr_start >= first + base; attr_start -= 4)
	{
		int x, y;
// some other drivers still use this wrong table, they have to be upgraded
//      int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };


		if (!get_sprite_attributes(&spriteram3[attr_start]))
			continue;


		if ( chip_disabled_pri & !curr_sprite.pri)
			continue;
		if ((!chip_disabled_pri) & (curr_sprite.pri >> 4))
			continue;


		curr_sprite.color += 16 * spritepalettebank;

// aerofgt has this adjustment, but doing it here would break turbo force title screen
//      curr_sprite.ox += (curr_sprite.xsize*curr_sprite.zoomx+2)/4;
//      curr_sprite.oy += (curr_sprite.ysize*curr_sprite.zoomy+2)/4;

		curr_sprite.zoomx = 32 - curr_sprite.zoomx;
		curr_sprite.zoomy = 32 - curr_sprite.zoomy;

		for (y = 0; y <= curr_sprite.ysize; y++)
		{
			int sx, sy;

			if (curr_sprite.flipy)
				sy = ((curr_sprite.oy + curr_sprite.zoomy * (curr_sprite.ysize - y)/2 + 16) & 0x1ff) - 16;
			else
				sy = ((curr_sprite.oy + curr_sprite.zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0; x <= curr_sprite.xsize; x++)
			{
				int curr;

				if (curr_sprite.flipx)
					sx = ((curr_sprite.ox + curr_sprite.zoomx * (curr_sprite.xsize - x) / 2 + 16) & 0x1ff) - 16;
				else
					sx = ((curr_sprite.ox + curr_sprite.zoomx * x / 2 + 16) & 0x1ff) - 16;

				curr = m_newtilecb(curr_sprite.map++);

				pdrawgfxzoom_transpen(bitmap,cliprect,machine.gfx[sprite_gfx],
							 curr,
							 curr_sprite.color,
							 curr_sprite.flipx,curr_sprite.flipy,
							 sx,sy,
							 curr_sprite.zoomx << 11, curr_sprite.zoomy << 11,
							 machine.priority_bitmap,curr_sprite.pri ? 2 : 0,15);
			}
			handle_xsize_map_inc();
		}
	}
}

void vsystem_spr2_device::spinlbrk_draw_sprites( UINT16* spriteram3,  int spriteram3_bytes, int sprite_gfx, int spritepalettebank, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int chip_disabled_pri )
{ spinlbrk_draw_sprites_common( spriteram3, spriteram3_bytes, sprite_gfx, spritepalettebank, machine, bitmap, cliprect, chip_disabled_pri ); }

void vsystem_spr2_device::spinlbrk_draw_sprites( UINT16* spriteram3,  int spriteram3_bytes, int sprite_gfx, int spritepalettebank, running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int chip_disabled_pri )
{ spinlbrk_draw_sprites_common( spriteram3, spriteram3_bytes, sprite_gfx, spritepalettebank, machine, bitmap, cliprect, chip_disabled_pri ); }




void vsystem_spr2_device::welltris_draw_sprites( UINT16* spriteram, int spritepalettebank, running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	static const UINT8 zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };
	int offs;
	const rectangle &visarea = machine.primary_screen->visible_area();

	/* draw the sprites */
	for (offs = 0; offs < 0x200 - 4; offs += 4)
	{
		if (!get_sprite_attributes(&spriteram[offs]))
			continue;

		curr_sprite.map&=0x1fff;
		curr_sprite.color += (0x10 * spritepalettebank);

		int xt, yt;


		/* compute the zoom factor -- stolen from aerofgt.c */
		curr_sprite.zoomx = 16 - zoomtable[curr_sprite.zoomx] / 8;
		curr_sprite.zoomy = 16 - zoomtable[curr_sprite.zoomy] / 8;

		/* wrap around */
		if (curr_sprite.ox > visarea.max_x) curr_sprite.ox -= 0x200;
		if (curr_sprite.oy > visarea.max_y) curr_sprite.oy -= 0x200;

		/* normal case */
		if (!curr_sprite.flipx && !curr_sprite.flipy) {
			for (yt = 0; yt <= curr_sprite.ysize; yt++) {
				for (xt = 0; xt <= curr_sprite.xsize; xt++)
				{
					int curr = m_newtilecb(curr_sprite.map++);

					drawgfxzoom_transpen(bitmap, cliprect, machine.gfx[1], curr, curr_sprite.color, 0, 0,
							curr_sprite.ox + xt * curr_sprite.zoomx, curr_sprite.oy + yt * curr_sprite.zoomy,
							0x1000 * curr_sprite.zoomx, 0x1000 * curr_sprite.zoomy, 15);
				}
				handle_xsize_map_inc();
			}
		}

		/* curr_sprite.flipxped case */
		else if (curr_sprite.flipx && !curr_sprite.flipy) {
			for (yt = 0; yt <= curr_sprite.ysize; yt++) {
				for (xt = 0; xt <= curr_sprite.xsize; xt++)
				{
					int curr = m_newtilecb(curr_sprite.map++);
	
					drawgfxzoom_transpen(bitmap, cliprect, machine.gfx[1], curr, curr_sprite.color, 1, 0,
							curr_sprite.ox + (curr_sprite.xsize - xt) * curr_sprite.zoomx, curr_sprite.oy + yt * curr_sprite.zoomy,
							0x1000 * curr_sprite.zoomx, 0x1000 * curr_sprite.zoomy, 15);
				}
				handle_xsize_map_inc();
			}
		}

		/* curr_sprite.flipyped case */
		else if (!curr_sprite.flipx && curr_sprite.flipy) {
			for (yt = 0; yt <= curr_sprite.ysize; yt++) {
				for (xt = 0; xt <= curr_sprite.xsize; xt++)
				{
					int curr = m_newtilecb(curr_sprite.map++);

					drawgfxzoom_transpen(bitmap, cliprect, machine.gfx[1], curr, curr_sprite.color, 0, 1,
							curr_sprite.ox + xt * curr_sprite.zoomx, curr_sprite.oy + (curr_sprite.ysize - yt) * curr_sprite.zoomy,
							0x1000 * curr_sprite.zoomx, 0x1000 * curr_sprite.zoomy, 15);
				}
				handle_xsize_map_inc();
			}
		}

		/* x & curr_sprite.flipyped case */
		else {
			for (yt = 0; yt <= curr_sprite.ysize; yt++) {
				for (xt = 0; xt <= curr_sprite.xsize; xt++)
				{
					int curr = m_newtilecb(curr_sprite.map++);

					drawgfxzoom_transpen(bitmap, cliprect, machine.gfx[1], curr, curr_sprite.color, 1, 1,
							curr_sprite.ox + (curr_sprite.xsize - xt) * curr_sprite.zoomx, curr_sprite.oy + (curr_sprite.ysize - yt) * curr_sprite.zoomy,
							0x1000 * curr_sprite.zoomx, 0x1000 * curr_sprite.zoomy, 15);
				}
				handle_xsize_map_inc();
			}
		}
	}
}



void vsystem_spr2_device::f1gp_draw_sprites( int gfxrgn, UINT16* sprvram, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int primask )
{
	int attr_start, first;
	UINT16 *spram = sprvram;

	first = 4 * spram[0x1fe];

	for (attr_start = 0x0200 - 8; attr_start >= first; attr_start -= 4)
	{
		int x, y;
		/* table hand made by looking at the ship explosion in attract mode */
		/* it's almost a logarithmic scale but not exactly */
		static const int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };


		if (!get_sprite_attributes(&spram[attr_start]))
			continue;

		curr_sprite.zoomx = 16 - zoomtable[curr_sprite.zoomx] / 8;
		curr_sprite.zoomy = 16 - zoomtable[curr_sprite.zoomy] / 8;

		for (y = 0; y <= curr_sprite.ysize; y++)
		{
			int sx, sy;

			if (curr_sprite.flipy) sy = ((curr_sprite.oy + curr_sprite.zoomy * (curr_sprite.ysize - y) + 16) & 0x1ff) - 16;
			else sy = ((curr_sprite.oy + curr_sprite.zoomy * y + 16) & 0x1ff) - 16;

			for (x = 0; x <= curr_sprite.xsize; x++)
			{
				int curr;

				if (curr_sprite.flipx) sx = ((curr_sprite.ox + curr_sprite.zoomx * (curr_sprite.xsize - x) + 16) & 0x1ff) - 16;
				else sx = ((curr_sprite.ox + curr_sprite.zoomx * x + 16) & 0x1ff) - 16;

				curr = m_newtilecb(curr_sprite.map++);

				pdrawgfxzoom_transpen(bitmap,cliprect,machine.gfx[gfxrgn],
						curr,
						curr_sprite.color,
						curr_sprite.flipx,curr_sprite.flipy,
						sx,sy,
						0x1000 * curr_sprite.zoomx,0x1000 * curr_sprite.zoomy,
						machine.priority_bitmap,
//                      pri ? 0 : 0x2);
						primask,15);
			}
			handle_xsize_map_inc();
		}
	}
}

// the same but for an 8-bit system..
void vsystem_spr2_device::draw_sprites_pipedrm( UINT8* spriteram, int spriteram_bytes, int flipscreen, screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int draw_priority )
{
	static const UINT8 zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };
	const rectangle &visarea = screen.visible_area();
//  UINT8 *spriteram = spriteram;
	int offs;

	/* draw the sprites */
	for (offs = 0; offs < spriteram_bytes; offs += 8)
	{
		int data2 = spriteram[offs + 4] | (spriteram[offs + 5] << 8);
		int priority = (data2 >> 4) & 1;

		/* turns out the sprites are the same as in aerofgt.c */
		if (priority == draw_priority)
		{
			if (!get_sprite_attributes((UINT16*)&spriteram[offs]))
				continue;

			curr_sprite.oy -= 6;
			curr_sprite.ox -= 13;



			int xt, yt;



			/* compute the zoom factor -- stolen from aerofgt.c */
			curr_sprite.zoomx = 16 - zoomtable[curr_sprite.zoomx] / 8;
			curr_sprite.zoomy = 16 - zoomtable[curr_sprite.zoomy] / 8;

			/* wrap around */
			if (curr_sprite.ox > visarea.max_x)
				curr_sprite.ox -= 0x200;
			if (curr_sprite.oy > visarea.max_y)
				curr_sprite.oy -= 0x200;

			/* flip ? */
			if (flipscreen)
			{
				curr_sprite.oy = visarea.max_y - curr_sprite.oy - 16 * curr_sprite.ysize - 4;
				curr_sprite.ox = visarea.max_x - curr_sprite.ox - 16 * curr_sprite.xsize - 24;
				curr_sprite.flipx=!curr_sprite.flipx;
				curr_sprite.flipy=!curr_sprite.flipy;
			}

			/* normal case */
			if (!curr_sprite.flipx && !curr_sprite.flipy)
			{
				for (yt = 0; yt <= curr_sprite.ysize; yt++)
				{
					for (xt = 0; xt <= curr_sprite.xsize; xt++)
					{
						int curr = m_newtilecb(curr_sprite.map++);

						drawgfxzoom_transpen(bitmap, cliprect, screen.machine().gfx[2], curr, curr_sprite.color, 0, 0,
								curr_sprite.ox + xt * curr_sprite.zoomx, curr_sprite.oy + yt * curr_sprite.zoomy,
								0x1000 * curr_sprite.zoomx, 0x1000 * curr_sprite.zoomy, 15);
					}
					handle_xsize_map_inc();
				}
			}

			/* curr_sprite.flipxped case */
			else if (curr_sprite.flipx && !curr_sprite.flipy)
			{
				for (yt = 0; yt <= curr_sprite.ysize; yt++)
				{
					for (xt = 0; xt <= curr_sprite.xsize; xt++)
					{
						int curr = m_newtilecb(curr_sprite.map++);

						drawgfxzoom_transpen(bitmap, cliprect, screen.machine().gfx[2], curr, curr_sprite.color, 1, 0,
								curr_sprite.ox + (curr_sprite.xsize - xt) * curr_sprite.zoomx, curr_sprite.oy + yt * curr_sprite.zoomy,
								0x1000 * curr_sprite.zoomx, 0x1000 * curr_sprite.zoomy, 15);
					}
					handle_xsize_map_inc();
				}
			}

			/* curr_sprite.flipyped case */
			else if (!curr_sprite.flipx && curr_sprite.flipy)
			{
				for (yt = 0; yt <= curr_sprite.ysize; yt++)
				{
					for (xt = 0; xt <= curr_sprite.xsize; xt++)
					{
						int curr = m_newtilecb(curr_sprite.map++);

						drawgfxzoom_transpen(bitmap, cliprect, screen.machine().gfx[2], curr, curr_sprite.color, 0, 1,
								curr_sprite.ox + xt * curr_sprite.zoomx, curr_sprite.oy + (curr_sprite.ysize - yt) * curr_sprite.zoomy,
								0x1000 * curr_sprite.zoomx, 0x1000 * curr_sprite.zoomy, 15);
					}
					handle_xsize_map_inc();
				}
			}

			/* x & curr_sprite.flipyped case */
			else
			{
				for (yt = 0; yt <= curr_sprite.ysize; yt++)
				{
					for (xt = 0; xt <= curr_sprite.xsize; xt++)
					{
						int curr = m_newtilecb(curr_sprite.map++);

						drawgfxzoom_transpen(bitmap, cliprect, screen.machine().gfx[2], curr, curr_sprite.color, 1, 1,
								curr_sprite.ox + (curr_sprite.xsize - xt) * curr_sprite.zoomx, curr_sprite.oy + (curr_sprite.ysize - yt) * curr_sprite.zoomy,
								0x1000 * curr_sprite.zoomx, 0x1000 * curr_sprite.zoomy, 15);
					}
					handle_xsize_map_inc();
				}
			}
		}
	}
}
