/******************************************************************************************

Seibu Custom "CRT Controller" emulation

by Angelo Salese,based off the old D-Con video HW part by Bryan McPhail

The (likely) custom Seibu CRTC is used at least in the following games:
Raiden (probably the first game to use it)
*Sengoku Mahjong
*Good e Jong
*Tottemo de Jong
Blood Bros.
Sky Smasher
D-Con
SD Gundam Psycho Salamander no Kyoui
(all games in legionna.c)
(all games in raiden2.c)

The name "Seibu CRTC" is actually an agglomerate of all the Seibu Customs that are on the
single boards, most if not all of them are shared video chips in the aforementioned games.

TODO:
- Dynamic Resolution Change (xx10 register?)
- Dynamic Paging register probably incorrect,needs further investigation;
- Merge the aforementioned games and clean-up the code in these drivers;
- Merge the gfxdecode too?
- Fully understand remaining video registers;
- Are the xx80/xxc0 in some games (D-Con/Sengoku Mahjong) another bunch of registers?
- Investigate about the real name(s) of this Seibu "CRT Controller",and if it is really
  custom.

===========================================================================================

List of default vregs (title screen):

*Sengoku Mahjong:
8000:  000F 0013 009F 00BF 00FA 000F 00FA 00FF
8010:  007D 0006 0000 0002 0000 0000 0000 0000
8020:  0000 0000 0004 0000 0000 0000 0040 01FF
8030:  003E 01FF 003F 01FF 0040 0001 0034 0035
8040:  0000 A8A8 0003 1C37 0001 0000 0000 0000

*Tottemo de Jong
8000:  000F 000F 009F 00BF 00FA 000F 00FA 00FF
8010:  0076 0006 0000 0002 0000 0002 0006 0000
8020:  0000 0000 0000 0000 0000 0000 01C0 01FF
8030:  003E 01FF 003F 01FF 00C0 01FF 0034 003F
8040:  0000 A8A8 0003 1830 0001 0000 0000 0000

*Good e Jong
8040:  000F 000F 009F 00BF 00FA 000F 00FA 00FF
8050:  0076 0006 0000 0002 0000 0002 0006 0000
8060:  0000 00FA 0000 0000 0000 0000 01C0 01FF
8070:  003E 01FF 003F 01FF 00C0 01FF 0034 003F
8080:  0000 0000 0000 0000 0000 0000 0000 0000

*******************************************************************************************/

#include "emu.h"
#include "includes/sei_crtc.h"

static tilemap_t *sc0_tilemap,*sc2_tilemap,*sc1_tilemap,*sc3_tilemap_0,*sc3_tilemap_1;
UINT16 *seibucrtc_sc0vram,*seibucrtc_sc1vram,*seibucrtc_sc2vram,*seibucrtc_sc3vram;
UINT16 *seibucrtc_vregs;
UINT16 seibucrtc_sc0bank;

/*******************************
*
* Macros for the video registers
*
*******************************/

/*******************************
* 0x1a - Layer Dynamic Paging?
*******************************/
#define SEIBU_CRTC_DYN_PAGING	(seibucrtc_vregs[0x001a/2])
#define SEIBU_CRTC_SC3_PAGE_SEL (SEIBU_CRTC_DYN_PAGING & 0x0002)

/*******************************
* 0x1c - Layer Enable
*******************************/
#define SEIBU_CRTC_LAYER_EN 	(seibucrtc_vregs[0x001c/2])
#define SEIBU_CRTC_ENABLE_SC0   (!(SEIBU_CRTC_LAYER_EN & 0x0001))
#define SEIBU_CRTC_ENABLE_SC2   (!(SEIBU_CRTC_LAYER_EN & 0x0002))
#define SEIBU_CRTC_ENABLE_SC1   (!(SEIBU_CRTC_LAYER_EN & 0x0004))
#define SEIBU_CRTC_ENABLE_SC3   (!(SEIBU_CRTC_LAYER_EN & 0x0008))
#define SEIBU_CRTC_ENABLE_SPR   (!(SEIBU_CRTC_LAYER_EN & 0x0010))

/************************************
* 0x20 - Screen 0 (BG) scroll x
************************************/
#define SEIBU_CRTC_SC0_SX	(seibucrtc_vregs[0x0020/2])

/************************************
* 0x22 - Screen 0 (BG) scroll y
************************************/
#define SEIBU_CRTC_SC0_SY	(seibucrtc_vregs[0x0022/2])

/************************************
* 0x24 - Screen 1 (FG) scroll x
************************************/
#define SEIBU_CRTC_SC1_SX	(seibucrtc_vregs[0x0024/2])

/************************************
* 0x26 - Screen 1 (FG) scroll y
************************************/
#define SEIBU_CRTC_SC1_SY	(seibucrtc_vregs[0x0026/2])

/************************************
* 0x28 - Screen 2 (MD) scroll x
************************************/
#define SEIBU_CRTC_SC2_SX	(seibucrtc_vregs[0x0028/2])

/************************************
* 0x2a - Screen 2 (MD) scroll y
************************************/
#define SEIBU_CRTC_SC2_SY	(seibucrtc_vregs[0x002a/2])

/************************************
* 0x2c - Fix screen scroll x (global)
************************************/
#define SEIBU_CRTC_FIX_SX	(seibucrtc_vregs[0x002c/2])

/************************************
* 0x2e - Fix screen scroll y (global)
************************************/
#define SEIBU_CRTC_FIX_SY	(seibucrtc_vregs[0x002e/2])


/*******************************
*
* Write RAM accesses
*
*******************************/

WRITE16_HANDLER( seibucrtc_sc0vram_w )
{
	COMBINE_DATA(&seibucrtc_sc0vram[offset]);
	sc0_tilemap->mark_tile_dirty(offset);
}

WRITE16_HANDLER( seibucrtc_sc2vram_w )
{
	COMBINE_DATA(&seibucrtc_sc2vram[offset]);
	sc2_tilemap->mark_tile_dirty(offset);
}

WRITE16_HANDLER( seibucrtc_sc1vram_w )
{
	COMBINE_DATA(&seibucrtc_sc1vram[offset]);
	sc1_tilemap->mark_tile_dirty(offset);
}

WRITE16_HANDLER( seibucrtc_sc3vram_w )
{
	COMBINE_DATA(&seibucrtc_sc3vram[offset]);
	sc3_tilemap_0->mark_tile_dirty(offset);
	sc3_tilemap_1->mark_tile_dirty(offset);
}

WRITE16_HANDLER( seibucrtc_vregs_w )
{
	COMBINE_DATA(&seibucrtc_vregs[offset]);
}

/* Actually external from the CRTC */
void seibucrtc_sc0bank_w(UINT16 data)
{
	seibucrtc_sc0bank = data & 1;
	sc0_tilemap->mark_all_dirty();
}


/*******************************
*
* Tilemap info accesses
*
*******************************/

static TILE_GET_INFO( seibucrtc_sc0_tile_info )
{
	int tile = seibucrtc_sc0vram[tile_index] & 0xfff;
	int color = (seibucrtc_sc0vram[tile_index] >> 12) & 0x0f;
	tile+=(seibucrtc_sc0bank<<12);
	SET_TILE_INFO(1, tile, color, 0);
}

static TILE_GET_INFO( seibucrtc_sc2_tile_info )
{
	int tile = seibucrtc_sc2vram[tile_index] & 0xfff;
	int color = (seibucrtc_sc2vram[tile_index] >> 12) & 0x0f;
	SET_TILE_INFO(2, tile, color, 0);
}

static TILE_GET_INFO( seibucrtc_sc1_tile_info )
{
	int tile = seibucrtc_sc1vram[tile_index] & 0xfff;
	int color = (seibucrtc_sc1vram[tile_index] >> 12) & 0x0f;
	SET_TILE_INFO(3, tile, color, 0);
}

static TILE_GET_INFO( seibucrtc_sc3_tile_info )
{
	int tile = seibucrtc_sc3vram[tile_index] & 0xfff;
	int color = (seibucrtc_sc3vram[tile_index] >> 12) & 0x0f;
	SET_TILE_INFO(4, tile, color, 0);
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect,int pri)
{
	UINT16 *spriteram16 = reinterpret_cast<UINT16 *>(machine.memory().shared("spriteram")->ptr());
	int offs,fx,fy,x,y,color,sprite;
	int dx,dy,ax,ay;

	for (offs = 0x400-4;offs >= 0;offs -= 4)
	{
		if ((spriteram16[offs+0]&0x8000)!=0x8000) continue;
		sprite = spriteram16[offs+1];
		if ((sprite>>14)!=pri) continue;
		sprite &= 0x1fff;

		y = spriteram16[offs+3];
		x = spriteram16[offs+2];

		if (x&0x8000) x=0-(0x200-(x&0x1ff));
		else x&=0x1ff;
		if (y&0x8000) y=0-(0x200-(y&0x1ff));
		else y&=0x1ff;

		color = spriteram16[offs+0]&0x3f;
		fx = spriteram16[offs+0]&0x4000;
		fy = spriteram16[offs+0]&0x2000;
		dy=((spriteram16[offs+0]&0x0380)>>7)+1;
		dx=((spriteram16[offs+0]&0x1c00)>>10)+1;

		for (ax=0; ax<dx; ax++)
			for (ay=0; ay<dy; ay++) {
				if (!fx)
					drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
						sprite++,
						color,fx,fy,x+ax*16,y+ay*16,15);
				else
					drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
						sprite++,
						color,fx,fy,x+(dx-1-ax)*16,y+ay*16,15);
			}
	}
}

/***********************************
*
* VIDEO_START/VIDEO_UPDATE functions
*
***********************************/

VIDEO_START( seibu_crtc )
{
	sc0_tilemap = tilemap_create(machine, seibucrtc_sc0_tile_info,tilemap_scan_rows,16,16,32,32);
	sc2_tilemap = tilemap_create(machine, seibucrtc_sc2_tile_info,tilemap_scan_rows,16,16,32,32);
	sc1_tilemap = tilemap_create(machine, seibucrtc_sc1_tile_info,tilemap_scan_rows,16,16,32,32);
	sc3_tilemap_0 = tilemap_create(machine, seibucrtc_sc3_tile_info,tilemap_scan_rows, 8, 8,32,32);
	sc3_tilemap_1 = tilemap_create(machine, seibucrtc_sc3_tile_info,tilemap_scan_rows, 8, 8,64,32);

	sc2_tilemap->set_transparent_pen(15);
	sc1_tilemap->set_transparent_pen(15);
	sc3_tilemap_0->set_transparent_pen(15);
	sc3_tilemap_1->set_transparent_pen(15);

	seibucrtc_sc0bank = 0;
}

SCREEN_UPDATE_IND16( seibu_crtc )
{
	bitmap.fill(screen.machine().pens[0x7ff], cliprect); //black pen

	sc0_tilemap->set_scrollx(0, (SEIBU_CRTC_SC0_SX + SEIBU_CRTC_FIX_SX+64) & 0x1ff );
	sc0_tilemap->set_scrolly(0, (SEIBU_CRTC_SC0_SY + SEIBU_CRTC_FIX_SY+1) & 0x1ff );
	sc2_tilemap->set_scrollx(0, (SEIBU_CRTC_SC2_SX + SEIBU_CRTC_FIX_SX+64) & 0x1ff );
	sc2_tilemap->set_scrolly(0, (SEIBU_CRTC_SC2_SY + SEIBU_CRTC_FIX_SY+1) & 0x1ff );
	sc1_tilemap->set_scrollx(0, (SEIBU_CRTC_SC1_SX + SEIBU_CRTC_FIX_SX+64) & 0x1ff );
	sc1_tilemap->set_scrolly(0, (SEIBU_CRTC_SC1_SY + SEIBU_CRTC_FIX_SY+1) & 0x1ff );
	(SEIBU_CRTC_SC3_PAGE_SEL ? sc3_tilemap_0 : sc3_tilemap_1)->set_scrollx(0, (SEIBU_CRTC_FIX_SX+64) & 0x1ff );
	(SEIBU_CRTC_SC3_PAGE_SEL ? sc3_tilemap_0 : sc3_tilemap_1)->set_scrolly(0, (SEIBU_CRTC_FIX_SY+1) & 0x1ff );

	if(SEIBU_CRTC_ENABLE_SC0) { sc0_tilemap->draw(bitmap, cliprect, 0,0); }
	if(SEIBU_CRTC_ENABLE_SPR) { draw_sprites(screen.machine(), bitmap,cliprect, 2); }
	if(SEIBU_CRTC_ENABLE_SC2) { sc2_tilemap->draw(bitmap, cliprect, 0,0); }
	if(SEIBU_CRTC_ENABLE_SPR) { draw_sprites(screen.machine(), bitmap,cliprect, 1); }
	if(SEIBU_CRTC_ENABLE_SC1) { sc1_tilemap->draw(bitmap, cliprect, 0,0); }
	if(SEIBU_CRTC_ENABLE_SPR) { draw_sprites(screen.machine(), bitmap,cliprect, 0); }
	if(SEIBU_CRTC_ENABLE_SC3) { (SEIBU_CRTC_SC3_PAGE_SEL ? sc3_tilemap_0 : sc3_tilemap_1)->draw(bitmap, cliprect, 0,0); }
	if(SEIBU_CRTC_ENABLE_SPR) { draw_sprites(screen.machine(), bitmap,cliprect, 3); }

	return 0;
}
