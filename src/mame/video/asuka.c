#include "driver.h"
#include "video/taitoic.h"

/**************************************************************
                 SPRITE READ AND WRITE HANDLERS
**************************************************************/

WRITE16_HANDLER( asuka_spritectrl_w )
{
	const device_config *pc090oj = devtag_get_device(space->machine, "pc090oj");

	/* Bits 2-5 are color bank; in asuka games bit 0 is global priority */
	pc090oj_set_sprite_ctrl(pc090oj, ((data & 0x3c) >> 2) | ((data & 0x1) << 15));
}


/**************************************************************
                        SCREEN REFRESH
**************************************************************/

VIDEO_UPDATE( asuka )
{
	const device_config *tc0100scn = devtag_get_device(screen->machine, "tc0100scn");
	const device_config *pc090oj = devtag_get_device(screen->machine, "pc090oj");
	UINT8 layer[3];

	tc0100scn_tilemap_update(tc0100scn);

	layer[0] = tc0100scn_bottomlayer(tc0100scn);
	layer[1] = layer[0]^1;
	layer[2] = 2;

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	/* Ensure screen blanked even when bottom layer not drawn due to disable bit */
	bitmap_fill(bitmap, cliprect, 0);

	tc0100scn_tilemap_draw(tc0100scn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	tc0100scn_tilemap_draw(tc0100scn, bitmap, cliprect, layer[1], 0, 2);
	tc0100scn_tilemap_draw(tc0100scn, bitmap, cliprect, layer[2], 0, 4);

	/* Sprites may be over or under top bg layer */
	pc090oj_draw_sprites(pc090oj, bitmap, cliprect, 2);
	return 0;
}


VIDEO_UPDATE( bonzeadv )
{
	const device_config *tc0100scn = devtag_get_device(screen->machine, "tc0100scn");
	const device_config *pc090oj = devtag_get_device(screen->machine, "pc090oj");
	UINT8 layer[3];

	tc0100scn_tilemap_update(tc0100scn);

	layer[0] = tc0100scn_bottomlayer(tc0100scn);
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	/* Ensure screen blanked even when bottom layer not drawn due to disable bit */
	bitmap_fill(bitmap, cliprect, 0);

	tc0100scn_tilemap_draw(tc0100scn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	tc0100scn_tilemap_draw(tc0100scn, bitmap, cliprect, layer[1], 0, 2);
	tc0100scn_tilemap_draw(tc0100scn, bitmap, cliprect, layer[2], 0, 4);

	/* Sprites are always over both bg layers */
	pc090oj_draw_sprites(pc090oj, bitmap, cliprect, 0);
	return 0;
}
