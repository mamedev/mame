/***************************************************************************

   Desert Assault Video emulation - Bryan McPhail, mish@tendril.co.uk

  I'm not sure if one of the alpha blending effects is correct (mode 0x8000,
  the usual mode 0x4000 should be correct).  It may be some kind of orthogonal
  priority effect where it should cut a hole in other higher priority sprites
  to reveal a non-alpha'd hole, or alpha against a further back tilemap.
  (is this the helicopter shadow at the end of lv.1 ?)

  Also, some priorities are still a little questionable.


****************************************************************************/

#include "emu.h"
#include "video/deco16ic.h"
#include "includes/dassault.h"
#include "video/decocomn.h"
#include "video/decospr.h"

/******************************************************************************/

void dassault_state::video_start()
{
	machine().device<decospr_device>("spritegen1")->alloc_sprite_bitmap();
	machine().device<decospr_device>("spritegen2")->alloc_sprite_bitmap();
}

static void mixdassaultlayer(running_machine &machine, bitmap_rgb32 &bitmap, bitmap_ind16* sprite_bitmap, const rectangle &cliprect, UINT16 pri, UINT16 primask, UINT16 penbase, UINT8 alpha)
{
	int y, x;
	const pen_t *paldata = machine.pens;

	UINT16* srcline;
	UINT32* dstline;

	for (y=cliprect.min_y;y<=cliprect.max_y;y++)
	{
		srcline=&sprite_bitmap->pix16(y,0);
		dstline=&bitmap.pix32(y,0);

		for (x=cliprect.min_x;x<=cliprect.max_x;x++)
		{
			UINT16 pix = srcline[x];

			if ((pix & primask) != pri)
				continue;

			if (pix&0xf)
			{
				UINT16 pen = pix&0x1ff;
				if (pix & 0x800) pen += 0x200;

				if (alpha!=0xff)
				{
					if (pix&0x600)
					{
						UINT32 base = dstline[x];
						dstline[x] = alpha_blend_r32(base, paldata[pen+penbase], alpha);
					}
					else
					{
						dstline[x] = paldata[pen+penbase];
					}
				}
				else
				{
					dstline[x] = paldata[pen+penbase];
				}
			}
		}
	}
}

/* are the priorities 100% correct? they're the same as they were before conversion to DECO52 sprite device, but if (for example) you walk to the side of the crates in the first part of the game you appear over them... */
SCREEN_UPDATE_RGB32( dassault )
{
	dassault_state *state = screen.machine().driver_data<dassault_state>();
	address_space &space = screen.machine().driver_data()->generic_space();
	UINT16 flip = deco16ic_pf_control_r(state->m_deco_tilegen1, space, 0, 0xffff);
	UINT16 priority = decocomn_priority_r(state->m_decocomn, space, 0, 0xffff);

	screen.machine().device<decospr_device>("spritegen2")->draw_sprites(bitmap, cliprect, state->m_spriteram2->buffer(), 0x400, false);
	screen.machine().device<decospr_device>("spritegen1")->draw_sprites(bitmap, cliprect, state->m_spriteram->buffer(), 0x400, false);
	bitmap_ind16* sprite_bitmap1 = &screen.machine().device<decospr_device>("spritegen1")->get_sprite_temp_bitmap();
	bitmap_ind16* sprite_bitmap2 = &screen.machine().device<decospr_device>("spritegen2")->get_sprite_temp_bitmap();

	/* Update tilemaps */
	state->flip_screen_set(BIT(flip, 7));
	deco16ic_pf_update(state->m_deco_tilegen1, 0, state->m_pf2_rowscroll);
	deco16ic_pf_update(state->m_deco_tilegen2, 0, state->m_pf4_rowscroll);

	/* Draw playfields/update priority bitmap */
	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(screen.machine().pens[3072], cliprect);
	deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	/* The middle playfields can be swapped priority-wise */
	if ((priority & 3) == 0)
	{
		mixdassaultlayer(screen.machine(), bitmap, sprite_bitmap1, cliprect,  0x0600, 0x0600,  0x400, 0xff); // 1
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 2); // 2
		mixdassaultlayer(screen.machine(), bitmap, sprite_bitmap1, cliprect,  0x0400, 0x0600,  0x400, 0xff); // 8
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 16); // 16
		mixdassaultlayer(screen.machine(), bitmap, sprite_bitmap1, cliprect,  0x0200, 0x0600,  0x400, 0xff); // 32
		mixdassaultlayer(screen.machine(), bitmap, sprite_bitmap2, cliprect,  0x0000, 0x0000,  0x800, 0x80); // 64?
		mixdassaultlayer(screen.machine(), bitmap, sprite_bitmap1, cliprect,  0x0000, 0x0600,  0x400, 0xff); // 128

	}
	else if ((priority & 3) == 1)
	{
		mixdassaultlayer(screen.machine(), bitmap, sprite_bitmap1, cliprect,  0x0600, 0x0600,  0x400, 0xff); // 1
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 2); // 2
		mixdassaultlayer(screen.machine(), bitmap, sprite_bitmap1, cliprect,  0x0400, 0x0600,  0x400, 0xff); // 8
		mixdassaultlayer(screen.machine(), bitmap, sprite_bitmap2, cliprect,  0x0000, 0x0000,  0x800, 0x80); // 16?
		mixdassaultlayer(screen.machine(), bitmap, sprite_bitmap1, cliprect,  0x0200, 0x0600,  0x400, 0xff); // 32
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 64); // 64
		mixdassaultlayer(screen.machine(), bitmap, sprite_bitmap1, cliprect,  0x0000, 0x0600,  0x400, 0xff); // 128
	}
	else if ((priority & 3) == 3)
	{
		mixdassaultlayer(screen.machine(), bitmap, sprite_bitmap1, cliprect,  0x0600, 0x0600,  0x400, 0xff); // 1
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 2); // 2
		mixdassaultlayer(screen.machine(), bitmap, sprite_bitmap1, cliprect,  0x0400, 0x0600,  0x400, 0xff); // 8
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 16); // 16
		mixdassaultlayer(screen.machine(), bitmap, sprite_bitmap1, cliprect,  0x0200, 0x0600,  0x400, 0xff); // 32
		mixdassaultlayer(screen.machine(), bitmap, sprite_bitmap2, cliprect,  0x0000, 0x0000,  0x800, 0x80); // 64?
		mixdassaultlayer(screen.machine(), bitmap, sprite_bitmap1, cliprect,  0x0000, 0x0600,  0x400, 0xff); // 128
	}
	else
	{
		/* Unused */
	}

	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	return 0;
}
