#include "driver.h"
#include "video/konamiic.h"
#include "includes/simpsons.h"

static int bg_colorbase,sprite_colorbase,layer_colorbase[3];
UINT8 *simpsons_xtraram;
static int layerpri[3];


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

static void tile_callback(int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*code |= ((*color & 0x3f) << 8) | (bank << 14);
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

static void sprite_callback(int *code,int *color,int *priority_mask)
{
	int pri = (*color & 0x0f80) >> 6;	/* ??????? */
	if (pri <= layerpri[2])								*priority_mask = 0;
	else if (pri > layerpri[2] && pri <= layerpri[1])	*priority_mask = 0xf0;
	else if (pri > layerpri[1] && pri <= layerpri[0])	*priority_mask = 0xf0|0xcc;
	else 												*priority_mask = 0xf0|0xcc|0xaa;

	*color = sprite_colorbase + (*color & 0x001f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( simpsons )
{
	K053251_vh_start(machine);

	K052109_vh_start(machine,"gfx1",NORMAL_PLANE_ORDER,tile_callback);
	K053247_vh_start(machine,"gfx2",53,23,NORMAL_PLANE_ORDER,sprite_callback);
}

/***************************************************************************

  Extra video banking

***************************************************************************/

static READ8_HANDLER( simpsons_K052109_r )
{
	return K052109_r(space,offset + 0x2000);
}

static WRITE8_HANDLER( simpsons_K052109_w )
{
	K052109_w(space,offset + 0x2000,data);
}

static READ8_HANDLER( simpsons_K053247_r )
{
	int offs;

	if (offset < 0x1000)
	{
		offs = offset >> 1;

		if (offset & 1)
			return(space->machine->generic.spriteram.u16[offs] & 0xff);
		else
			return(space->machine->generic.spriteram.u16[offs] >> 8);
	}
	else return simpsons_xtraram[offset - 0x1000];
}

static WRITE8_HANDLER( simpsons_K053247_w )
{
	int offs;

	if (offset < 0x1000)
	{
		UINT16 *spriteram16 = space->machine->generic.spriteram.u16;
		offs = offset >> 1;

		if (offset & 1)
			spriteram16[offs] = (spriteram16[offs] & 0xff00) | data;
		else
			spriteram16[offs] = (spriteram16[offs] & 0x00ff) | (data<<8);
	}
	else simpsons_xtraram[offset - 0x1000] = data;
}

void simpsons_video_banking(running_machine *machine, int bank)
{
	if (bank & 1)
	{
		memory_install_readwrite8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0000, 0x0fff, 0, 0, (read8_space_func)SMH_BANK(5), paletteram_xBBBBBGGGGGRRRRR_be_w);
		memory_set_bankptr(machine, 5, machine->generic.paletteram.v);
	}
	else
		memory_install_readwrite8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0000, 0x0fff, 0, 0, K052109_r, K052109_w);

	if (bank & 2)
		memory_install_readwrite8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x2000, 0x3fff, 0, 0, simpsons_K053247_r, simpsons_K053247_w);
	else
		memory_install_readwrite8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x2000, 0x3fff, 0, 0, simpsons_K052109_r, simpsons_K052109_w);
}



/***************************************************************************

  Display refresh

***************************************************************************/

/* useful function to sort the three tile layers by priority order */
static void sortlayers(int *layer,int *pri)
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
}

VIDEO_UPDATE( simpsons )
{
	int layer[3];


	bg_colorbase       = K053251_get_palette_index(K053251_CI0);
	sprite_colorbase   = K053251_get_palette_index(K053251_CI1);
	layer_colorbase[0] = K053251_get_palette_index(K053251_CI2);
	layer_colorbase[1] = K053251_get_palette_index(K053251_CI3);
	layer_colorbase[2] = K053251_get_palette_index(K053251_CI4);

	K052109_tilemap_update();

	layer[0] = 0;
	layerpri[0] = K053251_get_priority(K053251_CI2);
	layer[1] = 1;
	layerpri[1] = K053251_get_priority(K053251_CI3);
	layer[2] = 2;
	layerpri[2] = K053251_get_priority(K053251_CI4);

	sortlayers(layer,layerpri);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,16 * bg_colorbase);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[layer[0]],0,1);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[layer[1]],0,2);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[layer[2]],0,4);

	K053247_sprites_draw(screen->machine, bitmap,cliprect);
	return 0;
}
