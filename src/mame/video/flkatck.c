/***************************************************************************

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "video/konamiic.h"

static tilemap *k007121_tilemap[2];
static rectangle k007121_clip[2];

UINT8 *k007121_ram;

int flkatck_irq_enabled;

static int k007121_flip_screen = 0;

/***************************************************************************

  Callbacks for the K007121

***************************************************************************/

static TILE_GET_INFO( get_tile_info_A )
{
	int attr = k007121_ram[tile_index];
	int code = k007121_ram[tile_index+0x400];
	int bit0 = (K007121_ctrlram[0][0x05] >> 0) & 0x03;
	int bit1 = (K007121_ctrlram[0][0x05] >> 2) & 0x03;
	int bit2 = (K007121_ctrlram[0][0x05] >> 4) & 0x03;
	int bit3 = (K007121_ctrlram[0][0x05] >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0+2)) & 0x02) |
			((attr >> (bit1+1)) & 0x04) |
			((attr >> (bit2  )) & 0x08) |
			((attr >> (bit3-1)) & 0x10) |
			((K007121_ctrlram[0][0x03] & 0x01) << 5);
	int mask = (K007121_ctrlram[0][0x04] & 0xf0) >> 4;

	bank = (bank & ~(mask << 1)) | ((K007121_ctrlram[0][0x04] & mask) << 1);

	if ((attr == 0x0d) && (!(K007121_ctrlram[0][0])) && (!(K007121_ctrlram[0][2])))
		bank = 0;	/*  this allows the game to print text
                    in all banks selected by the k007121 */

	SET_TILE_INFO(
			0,
			code + 256*bank,
			(attr & 0x0f) + 16,
			(attr & 0x20) ? TILE_FLIPY : 0);
}

static TILE_GET_INFO( get_tile_info_B )
{
	int attr = k007121_ram[tile_index+0x800];
	int code = k007121_ram[tile_index+0xc00];

	SET_TILE_INFO(
			0,
			code,
			(attr & 0x0f) + 16,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( flkatck )
{
	k007121_tilemap[0] = tilemap_create(get_tile_info_A,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);
	k007121_tilemap[1] = tilemap_create(get_tile_info_B,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);

	k007121_clip[0] = machine->screen[0].visarea;
	k007121_clip[0].min_x += 40;

	k007121_clip[1] = machine->screen[0].visarea;
	k007121_clip[1].max_x = 39;
	k007121_clip[1].min_x = 0;
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( flkatck_k007121_w )
{
	k007121_ram[offset] = data;
	if (offset < 0x1000)	/* tiles */
	{
		if (offset & 0x800)	/* score */
			tilemap_mark_tile_dirty(k007121_tilemap[1],offset & 0x3ff);
		else
			tilemap_mark_tile_dirty(k007121_tilemap[0],offset & 0x3ff);
	}
}

WRITE8_HANDLER( flkatck_k007121_regs_w )
{
	switch (offset)
	{
		case 0x04:	/* ROM bank select */
			if (data != K007121_ctrlram[0][0x04])
				tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
			break;

		case 0x07:	/* flip screen + IRQ control */
			k007121_flip_screen = data & 0x08;
			tilemap_set_flip(ALL_TILEMAPS, k007121_flip_screen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			flkatck_irq_enabled = data & 0x02;
			break;
	}

	K007121_ctrl_0_w(offset,data);
}


/***************************************************************************

    Display Refresh

***************************************************************************/

/***************************************************************************

    Flack Attack sprites. Each sprite has 16 bytes!:


***************************************************************************/

VIDEO_UPDATE( flkatck )
{
	rectangle final_clip[2];

#if 0
popmessage("%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x  %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x",
	K007121_ctrlram[0][0x00],K007121_ctrlram[0][0x01],K007121_ctrlram[0][0x02],K007121_ctrlram[0][0x03],K007121_ctrlram[0][0x04],K007121_ctrlram[0][0x05],K007121_ctrlram[0][0x06],K007121_ctrlram[0][0x07],
	K007121_ctrlram[1][0x00],K007121_ctrlram[1][0x01],K007121_ctrlram[1][0x02],K007121_ctrlram[1][0x03],K007121_ctrlram[1][0x04],K007121_ctrlram[1][0x05],K007121_ctrlram[1][0x06],K007121_ctrlram[1][0x07]);
#endif

	/* set scroll registers */
	tilemap_set_scrollx(k007121_tilemap[0],0,K007121_ctrlram[0][0x00] - 40);
	tilemap_set_scrolly(k007121_tilemap[0],0,K007121_ctrlram[0][0x02]);

	/* compute clipping */
	final_clip[0] = k007121_clip[0];
	final_clip[1] = k007121_clip[1];
	sect_rect(&final_clip[0], cliprect);
	sect_rect(&final_clip[1], cliprect);

	/* draw the graphics */
	tilemap_draw(bitmap,&final_clip[0],k007121_tilemap[0],0,0);
	K007121_sprites_draw(machine,0,bitmap,cliprect,&k007121_ram[0x1000],0,40,0,-1);
	tilemap_draw(bitmap,&final_clip[1],k007121_tilemap[1],0,0);
	return 0;
}
