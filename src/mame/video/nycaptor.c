/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/
#include "driver.h"
#include "includes/nycaptor.h"

#define NYCAPTOR_DEBUG	0

#if NYCAPTOR_DEBUG
static  int nycaptor_mask=0;
#endif

static tilemap *bg_tilemap;
static int char_bank,palette_bank,gfxctrl;

UINT8 *nycaptor_scrlram;

static UINT8 *nycaptor_spriteram;

/*
 298 (e298) - spot (0-3) , 299 (e299) - lives
 spot number isn't set to 0 in main menu ; lives - yes
 sprites in main menu req priority 'type' 0
*/
static int nycaptor_spot(void)
{
	if(nyc_gametype==0 || nyc_gametype==2)
		return nycaptor_sharedram[0x299]?nycaptor_sharedram[0x298]:0;
	else
		return 0;
}

WRITE8_HANDLER(nycaptor_spriteram_w)
{
	nycaptor_spriteram[offset]=data;
}

READ8_HANDLER(nycaptor_spriteram_r)
{
	return nycaptor_spriteram[offset];
}

static TILE_GET_INFO( get_tile_info )
{
	int pal;
	tileinfo->category = (videoram[tile_index*2 + 1] & 0x30)>>4;
	pal=videoram[tile_index*2+1]&0x0f;
  tileinfo->group=0;
  if((!nycaptor_spot())&&(pal==6))tileinfo->group=1;
	if(((nycaptor_spot()==3)&&(pal==8))||((nycaptor_spot()==1)&&(pal==0xc)))tileinfo->group=2;
	if((nycaptor_spot()==1)&&(tileinfo->category==2))tileinfo->group=3;
#if NYCAPTOR_DEBUG
  if(nycaptor_mask&(1<<tileinfo->category))
  {
    if(nycaptor_spot())pal=0xe;else pal=4;
  }
#endif

	SET_TILE_INFO(
			0,
			videoram[tile_index*2] + ((videoram[tile_index*2+1] & 0xc0) << 2) +0x400 * char_bank,
			pal,0
			);
}


VIDEO_START( nycaptor )
{
  nycaptor_spriteram = auto_alloc_array(machine, UINT8, 160);
  bg_tilemap = tilemap_create( machine, get_tile_info,tilemap_scan_rows,8,8,32,32 );

  tilemap_set_transmask(bg_tilemap,0,0xf800,0x7ff); //split 0
  tilemap_set_transmask(bg_tilemap,1,0xfe00,0x01ff);//split 1
  tilemap_set_transmask(bg_tilemap,2,0xfffc,0x0003);//split 2
  tilemap_set_transmask(bg_tilemap,3,0xfff0,0x000f);//split 3

	paletteram = auto_alloc_array(machine, UINT8, 0x200);
	paletteram_2 = auto_alloc_array(machine, UINT8, 0x200);
	tilemap_set_scroll_cols(bg_tilemap,32);
}

WRITE8_HANDLER( nycaptor_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset>>1);
}

READ8_HANDLER( nycaptor_videoram_r )
{
	return videoram[offset];
}

WRITE8_HANDLER( nycaptor_palette_w )
{
	if(nyc_gametype==2) //colt
		return;

	if (offset & 0x100)
		paletteram_xxxxBBBBGGGGRRRR_split2_w(space, (offset & 0xff) + (palette_bank << 8),data);
	else
		paletteram_xxxxBBBBGGGGRRRR_split1_w(space, (offset & 0xff) + (palette_bank << 8),data);
}

READ8_HANDLER( nycaptor_palette_r )
{
	if (offset & 0x100)
		return paletteram_2[ (offset & 0xff) + (palette_bank << 8) ];
	else
		return paletteram  [ (offset & 0xff) + (palette_bank << 8) ];
}

WRITE8_HANDLER( nycaptor_gfxctrl_w )
{
	if (gfxctrl == data)
		return;
	gfxctrl = data;

	if(char_bank != ((data & 0x18) >> 3))
	{
		char_bank=((data & 0x18) >> 3);
		tilemap_mark_all_tiles_dirty( bg_tilemap );
	}
	palette_bank = (data & 0x20) >> 5;

}

READ8_HANDLER( nycaptor_gfxctrl_r )
{
		return 	gfxctrl;
}

READ8_HANDLER( nycaptor_scrlram_r )
{
	return nycaptor_scrlram[offset];
}

WRITE8_HANDLER( nycaptor_scrlram_w )
{
	nycaptor_scrlram[offset] = data;
	tilemap_set_scrolly(bg_tilemap, offset, data );
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect,int pri)
{
	int i;
	for (i=0;i<0x20;i++)
	{
		int pr = nycaptor_spriteram[0x9f-i];
		int offs = (pr & 0x1f) * 4;
		{
			int code,sx,sy,flipx,flipy,pal,priori;
			code = nycaptor_spriteram[offs+2] + ((nycaptor_spriteram[offs+1] & 0x10) << 4);//1 bit wolny = 0x20
			pal=nycaptor_spriteram[offs+1] & 0x0f;
			sx = nycaptor_spriteram[offs+3];
			sy = 240-nycaptor_spriteram[offs+0];
			priori=(pr&0xe0)>>5;
      if(priori==pri)
      {
#if NYCAPTOR_DEBUG
      if(nycaptor_mask&(1<<(pri+4)))pal=0xd;
#endif
			flipx = ((nycaptor_spriteram[offs+1]&0x40)>>6);
			flipy = ((nycaptor_spriteram[offs+1]&0x80)>>7);

			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					code,
					pal,
					flipx,flipy,
					sx,sy,15);

			if(nycaptor_spriteram[offs+3]>240)
			{
				sx = (nycaptor_spriteram[offs+3]-256);
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
        				code,
				        pal,
				        flipx,flipy,
					      sx,sy,15);
					}
				}
		}
	}
}





#if NYCAPTOR_DEBUG
/*
 Keys :
   q/w/e/r - bg priority display select
   a/s/d/f/g/h/j/k - sprite priority display select
   z - clear
   x - no bg/sprite pri.
*/

#define mKEY_MASK(x,y) if (input_code_pressed_once(x)){nycaptor_mask|=y;tilemap_mark_all_tiles_dirty( bg_tilemap );}

static void nycaptor_setmask(void)
{
  mKEY_MASK(KEYCODE_Q,1); /* bg */
  mKEY_MASK(KEYCODE_W,2);
  mKEY_MASK(KEYCODE_E,4);
  mKEY_MASK(KEYCODE_R,8);

  mKEY_MASK(KEYCODE_A,0x10); /* sprites */
  mKEY_MASK(KEYCODE_S,0x20);
  mKEY_MASK(KEYCODE_D,0x40);
  mKEY_MASK(KEYCODE_F,0x80);
  mKEY_MASK(KEYCODE_G,0x100);
  mKEY_MASK(KEYCODE_H,0x200);
  mKEY_MASK(KEYCODE_J,0x400);
  mKEY_MASK(KEYCODE_K,0x800);

  if (input_code_pressed_once(KEYCODE_Z)){nycaptor_mask=0;tilemap_mark_all_tiles_dirty( bg_tilemap );} /* disable */
  if (input_code_pressed_once(KEYCODE_X)){nycaptor_mask|=0x1000;tilemap_mark_all_tiles_dirty( bg_tilemap );} /* no layers */
}
#endif

VIDEO_UPDATE( nycaptor )
{
#if NYCAPTOR_DEBUG
  nycaptor_setmask();
  if(nycaptor_mask&0x1000)
  {
     	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|3,0);
     	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|3,0);
     	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|2,0);
     	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|2,0);
     	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|1,0);
     	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|1,0);
     	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|0,0);
     	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|0,0);
     	draw_sprites(screen->machine, bitmap,cliprect,0);
     	draw_sprites(screen->machine, bitmap,cliprect,1);
     	draw_sprites(screen->machine, bitmap,cliprect,2);
     	draw_sprites(screen->machine, bitmap,cliprect,3);
     	draw_sprites(screen->machine, bitmap,cliprect,4);
     	draw_sprites(screen->machine, bitmap,cliprect,5);
     	draw_sprites(screen->machine, bitmap,cliprect,6);
     	draw_sprites(screen->machine, bitmap,cliprect,7);
  }
 else
#endif
 switch (nycaptor_spot()&3)
 {
  case 0:
  	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|3,0);
    draw_sprites(screen->machine, bitmap,cliprect,6);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|3,0);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|2,0);
	  tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|2,0);
   	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|1,0);
    draw_sprites(screen->machine, bitmap,cliprect,3);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|1,0);
    draw_sprites(screen->machine, bitmap,cliprect,0);
    draw_sprites(screen->machine, bitmap,cliprect,2);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|0,0);
    draw_sprites(screen->machine, bitmap,cliprect,1);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|0,0);
  break;

  case 1:
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|3,0);
    draw_sprites(screen->machine, bitmap,cliprect,3);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|3,0);
    draw_sprites(screen->machine, bitmap,cliprect,2);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|2,0);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|1,0);
    draw_sprites(screen->machine, bitmap,cliprect,1);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|1,0);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|2,0);
    draw_sprites(screen->machine, bitmap,cliprect,0);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|0,0);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|0,0);
  break;

  case 2:
   	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|3,0);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|3,0);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|1,0);
    draw_sprites(screen->machine, bitmap,cliprect,1);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|1,0);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|2,0);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|2,0);
    draw_sprites(screen->machine, bitmap,cliprect,0);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|0,0);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|0,0);
  break;

  case 3:
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|1,0);
    draw_sprites(screen->machine, bitmap,cliprect,1);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|1,0);
    draw_sprites(screen->machine, bitmap,cliprect,0);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1|0,0);
    tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0|0,0);
  break;
 }
	return 0;
}

